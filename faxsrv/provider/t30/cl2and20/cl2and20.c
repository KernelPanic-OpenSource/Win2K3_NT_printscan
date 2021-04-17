/*++

Copyright (c) 1997  Microsoft Corporation

Module Name:

    class20.c

Abstract:

    This is the main source for Classes 2 and 2.0 fax-modem T.30 driver

Author:
    Source base was originated by Win95 At Work Fax package.
    RafaelL - July 1997 - port to NT

Revision History:

--*/


#define USE_DEBUG_CONTEXT DEBUG_CONTEXT_T30_CLASS2


#include "prep.h"
#include "efaxcb.h"

#include "tiff.h"

#include "glbproto.h"
#include "t30gl.h"
#include "cl2spec.h"

#include "psslog.h"
#define FILE_ID FILE_ID_CL2AND20
#include "pssframe.h"

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

WORD Class2CodeToBPS[16] =
{
/* V27_2400             0 */    2400,
/* V27_4800             1 */    4800,
/* V29_V17_7200         2 */    7200,
/* V29_V17_9600         3 */    9600,
/* V33_V17_12000        4 */    12000,
/* V33_V17_14400        5 */    14400,
                                0,
                                0,
                                0,
                                0,
                                0,
                                0,
                                0,
                                0,
                                0,
                                0
};

#define CLASS2_MAX_CODE_TO_BPS   15

// Speeds for Class2HayesSyncSpeed to try
UWORD rguwClass2Speeds[] = {19200, 9600, 2400, 1200, 0};





BOOL ParseFPTS_SendAck(PThrdGlbl pTG)
{
    BOOL fPageAck = FALSE;
    BOOL fFoundFPTS = FALSE;
    UWORD count;

    DEBUG_FUNCTION_NAME("ParseFPTS_SendAck");

    // Acknowledge that we sent the page
    // Parse the FPTS response and see if the page is good or bad.
    for (count=0; count < pTG->class2_commands.comm_count; ++count)
    {
        switch (pTG->class2_commands.command[count])
        {
        case  CL2DCE_FPTS:
                fFoundFPTS = TRUE;
                switch(pTG->class2_commands.parameters[count][0])
                {
                case 1: DebugPrintEx(DEBUG_MSG,"Found good FPTS");
                        fPageAck = TRUE;
                        // Exar hack!!!
                        // Exar modems give FPTS:1 for good pages and
                        // FPTS:1,2,0,0 for bad. So, look for the 1,2
                        // if this is an EXAR. Otherwise, 1 means good.
                        if (pTG->CurrentMFRSpec.bIsExar)
                        {
                            if (pTG->class2_commands.parameters[count][1] == 2)
                            {
                                DebugPrintEx(DEBUG_ERR,"Nope - really Found bad FPTS");
                                fPageAck = FALSE;
                            }
                        }
                        else
                        {
                            PSSLogEntry(PSS_MSG, 1, "Received MCF");
                        }
                        break;
                case 2: PSSLogEntry(PSS_WRN, 1, "Received RTN");
                        fPageAck = FALSE;
                        break;
                case 3: PSSLogEntry(PSS_WRN, 1, "Received RTP");
                        fPageAck = TRUE;  // After RTP, we send the next page!
                        break;
                case 4: PSSLogEntry(PSS_WRN, 1, "Received PIN");
                        break;
                case 5: PSSLogEntry(PSS_WRN, 1, "Received PIP");
                        break;
                default:PSSLogEntry(PSS_WRN, 1, "Received unknown response");
                }
                break;
        default:
                break;
        }
    }

    if (!fFoundFPTS)
    {
        PSSLogEntry(PSS_WRN, 1, "Didn't receive FPTS");
    }
    return fPageAck;
}


USHORT Class2Dial(PThrdGlbl pTG, LPSTR lpszDial)
{
    UWORD   uwLen,
            uwDialStringLen;
    ULONG   ulTimeout;
    SWORD   swLen;
    USHORT  uRet;
    BYTE    bBuf[DIALBUFSIZE];
    char    chMod = pTG->NCUParams2.chDialModifier;
    DWORD   dwDialTime;

    DEBUG_FUNCTION_NAME("Class2Dial");

    // Send the predial command
    if (pTG->lpCmdTab->szPreDial && (swLen=(SWORD)_fstrlen(pTG->lpCmdTab->szPreDial)))
    {
        if (Class2iModemDialog( pTG,
                                (LPSTR)pTG->lpCmdTab->szPreDial,
                                swLen,
                                10000L,
                                0,
                                TRUE,
                                pTG->cbszCLASS2_OK,
                                pTG->cbszCLASS2_ERROR,
                                (C2PSTR)NULL) != 1)
        {
            DebugPrintEx(DEBUG_WRN,"Error on User's PreDial string: %s", (LPSTR)pTG->lpCmdTab->szPreDial);
        }
    }

    // If the dial string already has a T or P prefix, we use that
    // instead.
    {
        char c = 0;
        while ((c=*lpszDial) && c==' ')
        {
            lpszDial++;
        }
        if (c=='t'|| c=='T' || c=='p'|| c=='P')
        {
            chMod = c;
            lpszDial++;
            while((c=*lpszDial) && c==' ')
            {
                lpszDial++;
            }
        }
    }

    uwLen = (UWORD)wsprintf(bBuf, pTG->cbszCLASS2_DIAL,chMod, (LPSTR)lpszDial);

// Need to set an approriate timeout here. A minimum of 15secs is too short
// (experiment calling machines within a PABX), plus one has to give extra
// time for machines that pick up after 2 or 4 rings and also for long distance
// calls. I take a minumum of 30secs and add 3secs for each digits over 7
// (unless it's pulse dial in which case I add 8secs/digit).
// (I'm assuming that a long-distance call will take a minimum of 8 digits
// anywhere in ths world!). Fax machines I've tested wait about 30secs
// independent of everything.

    uwDialStringLen = (SWORD)_fstrlen(lpszDial);
    ulTimeout = DIAL_TIMEOUT;
    if (uwDialStringLen > 7)
    {
        ulTimeout += ((chMod=='p' || chMod=='P')?8000:3000) * (uwDialStringLen - 7);
    }

    // If user aborts during one of the pre-dial commands (AT+FLID, etc.),
    // that command fails, but Class2xSend continues. Therefore, check for
    // abort here.
    if(pTG->fAbortRequested)
    {
        DebugPrintEx(DEBUG_ERR,"Class2Dial aborting");
        Class2ModemHangup(pTG);
        return CONNECT_ERROR;
    }

    dwDialTime = GetTickCount();
    LogDialCommand(pTG, pTG->cbszCLASS2_DIAL, chMod, strlen(lpszDial));
    uRet = Class2iModemDialog(  pTG,
                                (LPB)bBuf,
                                uwLen,
                                ulTimeout,
                                0,
                                FALSE,
                                pTG->cbszFCON,
                                pTG->cbszCLASS2_BUSY,
                                pTG->cbszCLASS2_NOANSWER,
                                pTG->cbszCLASS2_NODIALTONE,
                                pTG->cbszCLASS2_ERROR,
                                pTG->cbszCLASS2_NOCARRIER,
                                (NPSTR)NULL);

    // If it was "ERROR", try again - maybe the predial command screwed
    // up somehow and left and ERROR hanging around?
    if (uRet == 5)
    {
        LogDialCommand(pTG, pTG->cbszCLASS2_DIAL, chMod, strlen(lpszDial));
        uRet = Class2iModemDialog(  pTG,
                                    (LPB)bBuf,
                                    uwLen,
                                    ulTimeout,
                                    0,
                                    FALSE,
                                    pTG->cbszFCON,
                                    pTG->cbszCLASS2_BUSY,
                                    pTG->cbszCLASS2_NOANSWER,
                                    pTG->cbszCLASS2_NODIALTONE,
                                    pTG->cbszCLASS2_ERROR,
                                    pTG->cbszCLASS2_NOCARRIER,
                                    (NPSTR)NULL);
    }

    switch(uRet)
    {
    case CONNECT_TIMEOUT:       if (pTG->fFoundFHNG)
                                {
                                    PSSLogEntry(PSS_ERR, 1, "Call was disconnected");
                                }
                                else
                                {
                                    PSSLogEntry(PSS_ERR, 1, "Response - timeout");
                                }
                                pTG->fFatalErrorWasSignaled = 1;
                                SignalStatusChange(pTG, FS_NO_ANSWER);
                                break;
    case CONNECT_OK:            PSSLogEntry(PSS_MSG, 1, "Response - +FCON");
                                pTG->fReceivedHDLCflags = TRUE;
                                break;
    case CONNECT_BUSY:          PSSLogEntry(PSS_ERR, 1, "Response - BUSY");
                                pTG->fFatalErrorWasSignaled = 1;
                                SignalStatusChange(pTG, FS_BUSY);
                                break;
    case CONNECT_NOANSWER:      PSSLogEntry(PSS_ERR, 1, "Response - NOANSWER");
                                pTG->fFatalErrorWasSignaled = 1;
                                SignalStatusChange(pTG, FS_NO_ANSWER);
                                break;
    case CONNECT_NODIALTONE:    PSSLogEntry(PSS_ERR, 1, "Response - NODIALTONE");
                                pTG->fFatalErrorWasSignaled = 1;
                                SignalStatusChange(pTG, FS_NO_DIAL_TONE);
                                break;
    case CONNECT_ERROR:         PSSLogEntry(PSS_ERR, 1, "Response - ERROR");
                                pTG->fFatalErrorWasSignaled = 1;
                                SignalStatusChange(pTG, FS_NO_ANSWER);
                                break;
    case 6:                     {
                                    DWORD dwDelta = GetTickCount() - dwDialTime;
                                    PSSLogEntry(PSS_ERR, 1, "Response - NO CARRIER");
                                    if (dwDelta < 5000L)
                                    {
                                        DebugPrintEx(DEBUG_WRN,"Dial: Pretending it's BUSY");
                                        pTG->fFatalErrorWasSignaled = 1;
                                        SignalStatusChange(pTG, FS_BUSY);
                                        uRet = CONNECT_BUSY;
                                    }
                                    else
                                    {
                                        DebugPrintEx(DEBUG_WRN,"Dial: Pretending it's TIMEOUT");
                                        pTG->fFatalErrorWasSignaled = 1;
                                        SignalStatusChange(pTG, FS_NO_ANSWER);
                                        uRet = CONNECT_TIMEOUT;
                                    }
                                }
                                break;

    default:                    uRet = CONNECT_ERROR;
                                break;
    }

    if (uRet != CONNECT_OK)
    {
        if (!Class2ModemHangup(pTG ))
        {
            return CONNECT_ERROR;
        }
    }

    return uRet;
}


USHORT Class2Answer(PThrdGlbl pTG)
{
    USHORT  uRet;
    SWORD   swLen;
    ULONG   ulWaitTime;

    DEBUG_FUNCTION_NAME("Class2Answer");

    // Default time we will wait after getting right number of rings
    // to allow input buffer to flush
    ulWaitTime = 500L;

    // We may still have an "OK" coming from the modem as a result of the
    // ATS1 command. We need to wait a bit and flush it. In most cases we
    // just wait 500 milliseconds. But, if we saw a 000 from the ATS1, we
    // broke immediately out of the loop above, setting ulWaitTime to be
    // the approximate wait we need before answering the phone.

    startTimeOut(pTG, &pTG->toAnswer, ulWaitTime);
    while (checkTimeOut(pTG, &pTG->toAnswer))
    {
    }
    FComFlush(pTG );

    // Send the preanswer command
    if (pTG->lpCmdTab->szPreAnswer && (swLen=(SWORD)_fstrlen(pTG->lpCmdTab->szPreAnswer)))
    {
        if (Class2iModemDialog( pTG,
                                (LPSTR)pTG->lpCmdTab->szPreAnswer,
                                swLen,
                                10000L,
                                0,
                                TRUE,
                                pTG->cbszCLASS2_OK,
                                pTG->cbszCLASS2_ERROR,
                                (C2PSTR)NULL) != 1)
        {
            DebugPrintEx(DEBUG_WRN,"Error on User's PreAnswer str: %s", (LPSTR)pTG->lpCmdTab->szPreAnswer);
        }
    }


#define ANSWER_TIMEOUT 35000
// Need to wait reasonably long, so that we don't give up too easily
// 7/25/95 JosephJ This used to be 15000. Changed to 35000 because
// MWAVE devices needed that timeout. Also, T.30 says that callee
// should try to negotiate for T1 seconds. T1 = 35 +/- 5s.

    /*
    * Send ATA command. The result will be stored in the global
    * variable pTG->lpbResponseBuf2. We will parse that in the Class2Receive
    * routine.
    */

    // Look for ERROR return first and try again if it happened
    if ((uRet = Class2iModemDialog( pTG,
                                    pTG->cbszATA,
                                    (UWORD)(strlen(pTG->cbszATA)),
                                    ANSWER_TIMEOUT,
                                    0,
                                    TRUE,
                                    pTG->cbszCLASS2_OK,
                                    pTG->cbszCLASS2_FHNG,
                                    pTG->cbszCLASS2_ERROR,
                                    (C2PSTR) NULL)) == 3)
    {
        DebugPrintEx(DEBUG_ERR,"ATA returned ERROR on first try");
        // dunno why we try this a 2nd time. But this time if we get ERROR
        // dont exit. The Racal modem (bug#1982) gives ERROR followed by a
        // good response! Cant ignore ERROR the first time otherwise we'll
        // change the ATA--ERROR--ATA(repeat) behaviour which seems to be
        // explicitly desired for some cases. However we dont take any
        // action based on the return value of the 2nd try, so it's safe to
        // ignore ERROR here. Worst case we take longer to timeout.
        uRet = Class2iModemDialog(  pTG,
                                    pTG->cbszATA,
                                    (UWORD)(strlen(pTG->cbszATA)),
                                    ANSWER_TIMEOUT,
                                    0,
                                    TRUE,
                                    pTG->cbszCLASS2_OK,
                                    pTG->cbszCLASS2_FHNG,
                                    (C2PSTR) NULL);
    }

    // Should've used Class2xParse to detect +FHNG. However, the ATA dialogs had the
    // CLASS2_FHNG reply in them before FHNG detection was implemented into
    // Class2xParse. If these answers are removed, a modem that replies "+FHNG" and
    // then nothing (no "OK") would only fail on timeout here. So, in order not to
    // cause a regression on such (hypothetical) modems, I'm keeping the CLASS2_FHNG's.
    if (uRet==2)
    {
        pTG->fFoundFHNG = TRUE;
    }

    if ((uRet != 1) || (pTG->fFoundFHNG)) // a '1' return indicates OK.
    {
        DebugPrintEx(DEBUG_ERR,"Can't get OK after ATA");
        
        PSSLogEntry(PSS_ERR, 1, "Failed to answer - this call was not a fax call");

        pTG->fFatalErrorWasSignaled = 1;
        SignalStatusChangeWithStringId(pTG, FS_RECV_NOT_FAX_CALL, IDS_RECV_NOT_FAX_CALL);

        // try to hangup and sync with modem. This should work
        // even if phone is not really off hook
        if (!Class2ModemHangup(pTG))
        {
            DebugPrintEx(DEBUG_ERR,"Can't Hangup after ANSWERFAIL");
        }
        return CONNECT_ERROR;
    }
    else
    {
        DebugPrintEx(DEBUG_MSG,"ATA Received %s",(LPSTR)(&(pTG->lpbResponseBuf2)));
        pTG->fReceivedHDLCflags = TRUE;
        return CONNECT_OK;
    }
}


SWORD Class2ModemSync(PThrdGlbl pTG)
{
    // The command used here must be guaranteed to be harmless,
    // side-effect free & non-dstructive. i.e. we can issue it
    // at any point in command mode without chnageing the state
    // of teh modem or disrupting anything.
    // ATZ does not qualify. AT does, I think.....
    SWORD ret_value;

    DEBUG_FUNCTION_NAME("Class2ModemSync");

    DebugPrintEx(DEBUG_MSG,"Calling Class2HayesSyncSpeed");
    ret_value = Class2HayesSyncSpeed(   pTG,
                                        pTG->cbszCLASS2_ATTEN,
                                        (UWORD)(strlen(pTG->cbszCLASS2_ATTEN)));
    DebugPrintEx(DEBUG_MSG,"Class2HayesSyncSpeed returned %d ", ret_value);
    return ret_value;
}


BOOL Class2ModemHangup(PThrdGlbl pTG)
{
    DEBUG_FUNCTION_NAME("Class2ModemHangup");

    PSSLogEntry(PSS_MSG, 0, "Phase E - Hang-up");
    PSSLogEntry(PSS_MSG, 1, "Hanging up");
    if (Class2HayesSyncSpeed(   pTG,
                                pTG->cbszCLASS2_HANGUP,
                                (UWORD)(strlen(pTG->cbszCLASS2_HANGUP))) < 0)
    {
        DebugPrintEx(DEBUG_WRN,"Failed once");

        FComDTR(pTG, FALSE);    // Lower DTR on stubborn hangups in ModemHangup
        Sleep(1000); // pause 1 second
        FComDTR(pTG, TRUE);     // raise it again. Some modems return to cmd state
                                // only when this is raised again

        if (Class2HayesSyncSpeed(   pTG,
                                    pTG->cbszCLASS2_HANGUP,
                                    (UWORD)(strlen(pTG->cbszCLASS2_HANGUP))) < 0)
        {
            DebugPrintEx(DEBUG_ERR,"Failed again");
            return FALSE;
        }
    }
    DebugPrintEx(DEBUG_MSG,"HANGUP Completed");

    if (!iModemGoClass(pTG, 0))
    {
        return FALSE;
    }
    // Can also ignore this return value. Just for tidier cleanup
    DebugPrintEx(DEBUG_MSG,"Completed GoClass0");

    // Bug1982: Racal modem, doesnt accept ATA. So we send it a PreAnswer
    // command of ATS0=1, i.r. turning ON autoanswer. And we ignore the
    // ERROR response it gives to the subsequent ATAs. It then answers
    // 'automatically' and gives us all the right responses. On hangup
    // however we need to send an ATS0=0 to turn auto-answer off. The
    // ExitCommand is not sent at all in Class2 and in Class1 it is only
    // sent on releasing the modem, not between calls. So send an S0=0
    // after ATH0. If the modem doesnt like it we ignore the resp anyway.
    Class2iModemDialog( pTG,
                        pTG->cbszCLASS2_CALLDONE,
                        (UWORD)(strlen(pTG->cbszCLASS2_CALLDONE)),
                        LOCALCOMMAND_TIMEOUT,
                        0,
                        TRUE,
                        pTG->cbszCLASS2_OK,
                        pTG->cbszCLASS2_ERROR,
                        (C2PSTR)NULL);
    return TRUE;
}


BOOL Class2ModemAbort(PThrdGlbl pTG)
{
    DEBUG_FUNCTION_NAME("Class2ModemAbort");

    // If modem already reported +FHNG, no need telling it to abort
    if (!pTG->fFoundFHNG)
    {
        // Try to abort modem in reasonable fashion - send the
        // abort command and then send hangup. The abort command
        // should hangup, but I am not convinced it always does!!!
        // It should not hurt to hang up again (I hope).
        // We'll use a long timeout here to let the abort take place.
        if (!Class2iModemDialog(pTG,
                                pTG->cbszCLASS2_ABORT,
                                (UWORD)(strlen(pTG->cbszCLASS2_ABORT)),
                                STARTSENDMODE_TIMEOUT,
                                1,
                                TRUE,
                                pTG->cbszCLASS2_OK,
                                pTG->cbszCLASS2_ERROR,
                                (C2PSTR) NULL))
        {
            //Ignore failure
            DebugPrintEx(DEBUG_ERR,"FK Failed");
        }
    }

    return Class2ModemHangup(pTG );
}



SWORD Class2HayesSyncSpeed(PThrdGlbl pTG, C2PSTR cbszCommand, UWORD uwLen)
{
    /*
    Internal routine to synchronize with the modem's speed.  Tries to
    get a response from the modem by trying the speeds in rglSpeeds
    in order (terminated by a 0).  If fTryCurrent is nonzero, checks for
    a response before trying to reset the speeds.

    Returns the speed it found, 0 if they're in sync upon entry (only
    checked if fTryCurrent!=0), or -1 if it couldn't sync.
    */
    short ilWhich = 0;

    DEBUG_FUNCTION_NAME("Class2HayesSyncSpeed");

    for(;;)
    {
        if (Class2SyncModemDialog(pTG,cbszCommand,uwLen,pTG->cbszCLASS2_OK))
        {
            return (ilWhich>=0 ? rguwClass2Speeds[ilWhich] : 0);
        }

        /* failed.  try next speed. */
        if (rguwClass2Speeds[++ilWhich]==0)
        {
            // Tried all speeds. No response
            DebugPrintEx(DEBUG_ERR,"Cannot Sync with Modem on Command %s", (LPSTR)cbszCommand);
            return -1;
        }
        if (!FComSetBaudRate(pTG,  rguwClass2Speeds[ilWhich]))
        {
            return -1;
        }
    }
}



USHORT  Class2ModemRecvBuf(PThrdGlbl pTG, LPBUFFER far* lplpbf, USHORT uTimeout)
{
    USHORT uRet;

    DEBUG_FUNCTION_NAME("Class2ModemRecvBuf");

    *lplpbf = MyAllocBuf(pTG, MY_BIGBUF_SIZE);
    if (*lplpbf == NULL)
    {
        DebugPrintEx(DEBUG_ERR, "MyAllocBuf failed trying to allocate %d bytes", MY_BIGBUF_SIZE);
        return RECV_OUTOFMEMORY;
    }

    uRet = Class2ModemRecvData( pTG,
                                (*lplpbf)->lpbBegBuf,
                                (*lplpbf)->wLengthBuf,
                                uTimeout,
                                &((*lplpbf)->wLengthData));

    if  (!((*lplpbf)->wLengthData))
    {
        MyFreeBuf(pTG, *lplpbf);
        *lplpbf = NULL;
    }
    else
    {
        // If necessary, bit-reverse...
        if (pTG->CurrentMFRSpec.fSWFBOR && pTG->CurrentMFRSpec.iReceiveBOR==1)
        {
            DebugPrintEx(DEBUG_WRN,"SWFBOR Enabled. bit-reversing data");
            cl2_flip_bytes( (*lplpbf)->lpbBegBuf, ((*lplpbf)->wLengthData));
        }
    }

    return uRet;
}


USHORT Class2ModemRecvData(PThrdGlbl pTG, LPB lpb, USHORT cbMax, USHORT uTimeout, USHORT far* lpcbRecv)
{
    SWORD   swEOF;

    startTimeOut(pTG, &pTG->toRecv, uTimeout);
    // 4th arg must be TRUE for Class2
    *lpcbRecv = FComFilterReadBuf(pTG, lpb, cbMax, &pTG->toRecv, TRUE, &swEOF);

    switch(swEOF)
    {
    case 1:         // Class1 eof
    case -1:        // Class2 eof
                    return RECV_EOF;
    case 0:
                    return RECV_OK;
    default:
                    // fall through
    case -2:
                    return RECV_ERROR;
    case -3:
                    return RECV_TIMEOUT;
    }
}


BOOL  Class2ModemSendMem(PThrdGlbl pTG, LPBYTE lpb, USHORT uCount)
{
    DEBUG_FUNCTION_NAME("Class2ModemSendMem");

    if (!FComFilterAsyncWrite(pTG, lpb, uCount, FILTER_DLEZERO))
    {
        goto error;
    }

    return TRUE;

error:
    DebugPrintEx(DEBUG_ERR,"Failed on AsyncWrite");
    FComOutFilterClose(pTG);
    FComXon(pTG, FALSE);
    return FALSE;
}

/*
output:
0 - timeout
1 - OK
2 - ERROR
*/
DWORD Class2ModemDrain(PThrdGlbl pTG)
{
    DWORD dwResult = 0;
    DEBUG_FUNCTION_NAME("Class2ModemDrain");

    if (!FComDrain(pTG, TRUE, TRUE))
    {
        return FALSE;
    }

    // Must turn XON/XOFF off immediately *after* drain, but before we
    // send the next AT command, since Received frames have 0x13 or
    // even 0x11 in them!! MUST GO AFTER the getOK ---- See BELOW!!!!

    dwResult = Class2iModemDialog(pTG,
                                  NULL,
                                  0,
                                  STARTSENDMODE_TIMEOUT,
                                  0,
                                  TRUE,
                                  pTG->cbszCLASS2_OK,
                                  pTG->cbszCLASS2_ERROR,
                                  (C2PSTR)NULL);
    // Must change FlowControl State *after* getting OK because in Windows
    // this call takes 500 ms & resets chips, blows away data etc.
    // So do this *only* when you *know* both RX & TX are empty.

    // Turn off flow control.
    FComXon(pTG, FALSE);

    // FComDrain doesn't check for abort, and the dialog might succeed because of that
    // Therefore, check for it here.
    if (pTG->fAbortRequested)
    {
        return 0;   // timeout
    }

    return dwResult;
}


#define ABORT_TIMEOUT    250

UWORD Class2iModemDialog
(
    PThrdGlbl pTG,
    LPSTR szSend,
    UWORD uwLen,
    ULONG ulTimeout,
    UWORD uwRepeatCount,
    BOOL fLogSend,
    ...
)
{
/** Takes a command string, and it's lengt writes it out to the modem
    and tries to get one of the allowed responses. It writes the command
        out, waits ulTimeOut millisecs for a response. If it gets one of the
        expected responses it returns immediately.

        If it gets an unexpected/illegal response it tries (without any
        waiting) for subsequent lines to the same response.     When all the
        lines (if > 1) of the response lines are exhausted, if none is among the
        expected responses, it writes the command again and tries again,
        until ulTimeout has expired. Note that if no response is received,
        the command will be written just once.

        The whole above thing will be repeated upto uwRepeatCount times
        if uwRepeatCount is non-zero

<<<<<NOTE:::uwRepeatCount != 0 should not be used except for local sync>>>>>

        It returns when (a) one of the specified responses is received or
        (b) uwRepeatCount tries have failed (each having returned an
        illegal response or having returned no response in ulTimeout
        millsecs) or (c) the command write failed, in which
        case it returns immediately.

        It flushes the modem inque before each Command Write.

        Returns 0 on failure and the 1 based index of the successful
        response on     success.

        This can be used in the following way:-

        for Local Dialogs (AT, AT+FTH=? etc), set ulTimeout to a lowish
        value, of the order of the transmission time of the longest
        possible (erroneous or correct) line of response plus the size
        of the command. eg. at 1200baud we have about 120cps = about
        10ms/char. Therefore a timeout of about 500ms is more than
        adequate, except for really long command lines.

        for Local Sync dialogs, used to sync up with the modem which may
        be in an unsure state, use the same timeout, but also a repeat
        count of 2 or 3.

        for remote-driven dialogs, eg. AT+FRH=xx which returns a CONNECT
        after the flags have been received, and which may incur a delay
        before a response (ATDT is teh same. CONNECT is issued after a
        long delay & anything the DTE sends will abort the process).
        For these cases the caller should supply a long timeout and
        probably a repeatcount of 1, so that the
        routine will timeout after one try but go on issuing teh command
        as long as an error repsonse is received.

        For +FRH etc, the long timeout should be T1 or T2 in teh case of
        CommandRecv and ResponseRecv respectively.

**/
    BYTE bReply[REPLYBUFSIZE];
    UWORD   i, j, uwRet, uwWantCount;
    SWORD   swNumRead;
    C2PSTR  rgcbszWant[10] = {NULL};
    va_list args;
    LPTO    lpto, lptoRead, lpto0;

    DEBUG_FUNCTION_NAME("Class2iModemDialog");
    // extract the (variable length) list of acceptable responses.
    // each is a C2SZ, code based 2 byte ptr

    va_start(args, fLogSend);  // Ansi Defintion

    for(j=1; j<10; j++)
    {
        if((rgcbszWant[j] = va_arg(args, C2PSTR)) == NULL)
        {
            break;
        }
    }
    uwWantCount = j-1;
    va_end(args);

    pTG->lpbResponseBuf2[0] = 0;

    lpto = &pTG->toDialog;
    lpto0 = &pTG->toZero;
    // Try the dialog upto uwRepeatCount times
    for (uwRet=0, i=0; i<=uwRepeatCount; i++)
    {
        startTimeOut(pTG, lpto, ulTimeout);
        do
        {
            if(szSend)
            {
                // If a command is supplied, write it out, flushing input
                // first to get rid of spurious input.

                // FComInputFlush();
                FComFlush(pTG);            // Need to flush output too?

                if (fLogSend)
                {
                    PSSLogEntry(PSS_MSG, 2, "send: \"%s\"", szSend);
                }

                // Need to check that we are sending only ASCII or pre-stuffed data here
                if (!FComDirectSyncWriteFast(pTG,  szSend, uwLen))
                {
                    DebugPrintEx(DEBUG_ERR,"Modem Dialog Write timed Out");
                    uwRet = 0;
                    goto done;
                    // If Write fails, fail & return immediately.
                    // SetMyError() will already have been called.
                }
            }

            for (lptoRead=lpto;;startTimeOut(pTG, lpto0, ulTimeout), lptoRead=lpto0)
            {
                // get a CR-LF terminated line
                // for the first line use macro timeout, for multi-line
                // responses use 0 timeout.

                swNumRead = FComFilterReadLine(pTG, bReply, REPLYBUFSIZE-1, lptoRead);
                if(swNumRead < 0)
                {
                    swNumRead = (-swNumRead);       // error-but lets see what we got anyway
                }
                else if (swNumRead == 0)
                {
                    goto timeout;                          // Timeout -- restart dialog or exit
                }
                if (swNumRead == 2 && bReply[0] == '\r' && bReply[1] == '\n')
                {
                    continue;                                       // blank line -- throw away & get another
                }

                // COPIED THIS FROM DUP FUNCTION IN MODEM.C!!
                // Fix Bug#1226. Elsa Microlink returns this garbage line in
                // response to AT+FDIS?, followed by the real reply. Since
                // we only look at the first line, we see only this garbage line
                // and we never see the real reply
                if (swNumRead==3 && bReply[0]==0x13 && bReply[1]=='\r' && bReply[2]=='\n')
                {
                    continue;
                }

                PSSLogEntry(PSS_MSG, 2, "recv:     \"%s\"", bReply);

                for(bReply[REPLYBUFSIZE-1]=0, j=0; j<=uwWantCount; j++)
                {
                    if( rgcbszWant[j] && (strstr(bReply, rgcbszWant[j]) != NULL))
                    {
                        uwRet = j;
                        // It matched!!!
                        // Save this reply. This is used when checking
                        // ATS1 responses
                        goto end;
                    }
                }
                // go to ulTimeout check. i.e. *don't* set fTimedOut
                // but don't exit either. Retry command and response until
                // timeout

                // We reach here it IFF we got a non blank reply, but it wasn't what
                // we wanted. Squirrel teh first line away somewhere so that we can
                // retrieve is later. We use this hack to get multi-line informational
                // responses to things like +FTH=? Very important to ensure that
                // blank-line replies don't get recorded here. (They may override
                // the preceding line that we need!).

                if ( (strlen(pTG->lpbResponseBuf2) + strlen(bReply) ) < RESPONSE_BUF_SIZE)
                {
                    strncat(pTG->lpbResponseBuf2, bReply, (ARR_SIZE(pTG->lpbResponseBuf2) - strlen(pTG->lpbResponseBuf2) -1));
                }
                else
                {
                    DebugPrintEx(DEBUG_ERR,"Response too long!");
                    uwRet = 0;
                    goto end;
                }

                // If +FHNG was received, an OK should follow.
                if (strstr(bReply, pTG->cbszCLASS2_FHNG) != NULL)
                {
                    UWORD uwAns;

                    // Check whether OK is already on the responses list
                    for (uwAns=1; uwAns<=uwWantCount; uwAns++)
                    {
                        if (strstr(rgcbszWant[uwAns], pTG->cbszCLASS2_OK) != NULL)
                        {
                            break;
                        }
                    }

                    if (uwAns > uwWantCount)
                    {
                        // OK is not on the list, add it as response #0. When the OK
                        // comes, the dialog will terminate with response 0, and the caller
                        // will know that the dialog failed.
                        DebugPrintEx(DEBUG_MSG, "FHNG found during dialog, adding OK to want list");
                        rgcbszWant[0] = pTG->cbszCLASS2_OK;
                    }
                    else
                    {
                        // OK is already on the wanted responses list, don't add anything. The
                        // dialog will succeed with the original OK response, and the caller can
                        // explicitely check pTG->fFoundFHNG
                        DebugPrintEx(DEBUG_MSG, "FHNG found during dialog, OK already on want list");
                    }
                }
            }   // end of loop that reads lines until timeout
            // we come here only on timeout.
        } while (checkTimeOut(pTG, lpto));

timeout:
        PSSLogEntryStrings(PSS_WRN, 2, &rgcbszWant[1], uwWantCount,
                "failed to receive expected response: ");
        // Need to send anychar to abort the previous command. Use "AT"
        PSSLogEntry(PSS_MSG, 2, "send: \"AT\"");
        FComFlush(pTG); // flush first--don't want some old garbage result
        FComDirectSyncWriteFast(pTG, "\rAT", 3);
        FComFlushInput(pTG); // flush input again
        FComDirectAsyncWrite(pTG, "\r", 1);
        startTimeOut(pTG, lpto0, ABORT_TIMEOUT);
        do
        {
            swNumRead = FComFilterReadLine(pTG, bReply, REPLYBUFSIZE-1, lpto0);
        }
        while(swNumRead==2 && bReply[0]=='\r'&& bReply[1]=='\n');
        // While we get a blank line. Get another.
        bReply[REPLYBUFSIZE-1] = 0;
        if (swNumRead != 0)
        {
            PSSLogEntry(PSS_MSG, 2, "recv:     \"%s\"", bReply);
        }
    }  // for i=0..RepeatCount

end:
    if (uwRet == 0)
    {
        DebugPrintEx(   DEBUG_ERR,
                        "(%s) --> (%d)(%s, etc) Failed",
                        (LPSTR)(szSend?szSend:"null"),
                        uwWantCount,
                        (LPSTR)rgcbszWant[1]);
    }
    else
    {
        DebugPrintEx(DEBUG_MSG,"GOT IT %d (%s)", uwRet, (LPSTR)(rgcbszWant[uwRet]));
    }

done:
    // Parse modem notifications into pTG->class2_commands
    if (pTG->ModemClass == MODEM_CLASS2)
    {
       Class2Parse(pTG,  &pTG->class2_commands, pTG->lpbResponseBuf2 );
    }
    else
    {
       Class20Parse(pTG,  &pTG->class2_commands, pTG->lpbResponseBuf2 );
    }

    return uwRet;
}


/* Converts the code for a speed to the speed in BPS */
// These are in the same order as the return values
// for DIS/DCS frames defined in the Class 2 spec in
// Table 8.4


/* Converts a DCS min-scan field code into millisecs */
// One array is for normal (100 dpi) res, the other for high (200 dpi) res...
// The ordering of the arraies is based on the values that
// are defined in filet30.h - THEY ARE NOT THE SAME AS THE VALUES
// RETURNED IN THE DCS FRAME!!!! This is inconsistent with baud rate
// but it is consistent with the Class 1 code...
BYTE msPerLineNormalRes[8] = { 20, 5, 10, 20, 40, 40, 10, 0 };
BYTE msPerLineHighRes[8] =   { 20, 5, 10, 10, 40, 20, 5, 0 };





USHORT Class2MinScanToBytesPerLine(PThrdGlbl pTG, BYTE Minscan, BYTE Baud, BYTE Resolution)
{
    USHORT uStuff;
    BYTE ms;

    DEBUG_FUNCTION_NAME("Class2MinScanToBytesPerLine");

    uStuff = Class2CodeToBPS[Baud];
    if ( Resolution & AWRES_mm080_077)
    {
        ms = msPerLineHighRes[Minscan];
    }
    else
    {
        ms = msPerLineNormalRes[Minscan];
    }
    uStuff /= 100;          // StuffBytes = (BPS * ms)/8000
    uStuff *= ms;           // take care not to use longs
    uStuff /= 80;           // or overflow WORD or lose precision
    uStuff += 1;            // Rough fix for truncation problems

    DebugPrintEx(DEBUG_MSG,"Stuffing %d bytes", uStuff);
    return uStuff;
}

// Convert the SEND_CAPS or SEND_PARAMS BC structure into values used by
// the +FDCC, +FDIS, and +FDT commands



void Class2SetDIS_DCSParams
(
    PThrdGlbl pTG,
    BCTYPE bctype,
    OUT LPUWORD Encoding,
    OUT LPUWORD Resolution,
    OUT LPUWORD PageWidth,
    OUT LPUWORD PageLength,
    OUT LPSTR szID,
	IN UINT cch
)
{
    LPBC lpbc;

    DEBUG_FUNCTION_NAME("Class2SetDIS_DCSParams");

    DebugPrintEx(DEBUG_MSG,"Type = %d", (USHORT)bctype);

    if (bctype == SEND_PARAMS)
    {
        lpbc = (LPBC)&pTG->bcSendParams;
    }
    else
    {
        lpbc = (LPBC)&pTG->bcSendCaps;
    }

    if (pTG->LocalID)
    {
		HRESULT hr = StringCchCopy(szID,cch,pTG->LocalID);
		if (FAILED(hr))
		{
			// we failed, nothing to do other than debug print.
			// the OUT param is null terminated.
			DebugPrintEx(DEBUG_WRN,"StringCchCopy failed (ec=0x%08X)",hr);
		}
    }

    switch(lpbc->Fax.Encoding)
    {
    case MH_DATA:                           *Encoding = 0;
                                            break;
    case MR_DATA:
    case (MR_DATA | MH_DATA):               *Encoding = 1;
                                            break;
    case MMR_DATA:
    case (MMR_DATA | MH_DATA):
    case (MMR_DATA | MR_DATA):
    case (MMR_DATA | MR_DATA | MH_DATA):    *Encoding = 3;
                                            break;

    default:                                DebugPrintEx(DEBUG_ERR,"Bad Encoding type %x",lpbc->Fax.Encoding);
                                            break;
    }

    if ( (lpbc->Fax.AwRes) & AWRES_mm080_077)
    {
        *Resolution = 1;
    }
    else if ( (lpbc->Fax.AwRes) & AWRES_mm080_038)
    {
        *Resolution = 0;
    }
    else
    {
        DebugPrintEx(DEBUG_ERR,"Bad Resolution type %x",lpbc->Fax.AwRes);
    }

    switch (lpbc->Fax.PageWidth & 0x3)
    {
    case WIDTH_A4:      // 1728 pixels
                        *PageWidth = 0;
                        break;
    case WIDTH_B4:      // 2048 pixels
                        *PageWidth = 1;
                        break;
    case WIDTH_A3:      // 2432 pixels
                        *PageWidth = 2;
                        break;
    default:            DebugPrintEx(DEBUG_ERR,"Bad PageWidth type %x", lpbc->Fax.PageWidth);
                        break;
    }

    switch(lpbc->Fax.PageLength)
    {
    case LENGTH_A4:         *PageLength = 0;
                            break;
    case LENGTH_B4:         *PageLength = 1;
                            break;
    case LENGTH_UNLIMITED:  *PageLength = 2;
                            break;
    default:                DebugPrintEx(DEBUG_ERR,"Bad PageLength type %x", lpbc->Fax.PageLength);
                            break;
    }
}


/*++
Routine Description:
    Copies a string, removing leading and trailing spaces

Arguments:
    pszDest         [out]    Pointer to buffer that will accept output string.
    cchDest         [in]     Size of pszDest, in chars
    pszSrc          [in/out] Pointer to source string
                             Warning: This function changes pszSrc.
Return Value:
    Success / Error in HRESULT form
--*/

HRESULT Class2CopyID(LPSTR pszDest, size_t cchDest, LPSTR pszSrc)
{
    int iFirst;
    int iLast;

    for (iFirst=0; pszSrc[iFirst]==' '; iFirst++)
        ;
    for (iLast=strlen(pszSrc)-1; iLast >= iFirst && pszSrc[iLast]==' '; iLast--)
        ;
    pszSrc[iLast+1] = '\0';
    return StringCchCopy(pszDest, cchDest, &(pszSrc[iFirst]));
}


BOOL Class2ResponseAction(PThrdGlbl pTG, LPPCB lpPcb)
{
    USHORT count;
    BOOL   fFoundDIS_DCS;
    HRESULT hr;

    DEBUG_FUNCTION_NAME("Class2ResponseAction");

    fFoundDIS_DCS = FALSE;

    _fmemset(lpPcb, 0, sizeof(PCB));

    // Even though Class2iModemDialog calls Class2XParse, we need to call
    // it again, because main function may change pTG->lpbResponseBuf2.
    if (pTG->ModemClass == MODEM_CLASS2)
    {
       Class2Parse(pTG,  &pTG->class2_commands, pTG->lpbResponseBuf2 );
    }
    else
    {
       Class20Parse(pTG,  &pTG->class2_commands, pTG->lpbResponseBuf2 );
    }


    DebugPrintEx(   DEBUG_MSG,
                    "Number of commands is %d",
                    pTG->class2_commands.comm_count);
    for (count=0; count < pTG->class2_commands.comm_count; ++count)
    {
        switch (pTG->class2_commands.command[count])
        {
            case  CL2DCE_FDIS:
            case  CL2DCE_FDCS:

                    DebugPrintEx(DEBUG_MSG,"Found DCS or DIS");
                    fFoundDIS_DCS = TRUE;
                    //  Assign resolution.
                    if( pTG->class2_commands.parameters[count][0] == 0)
                    {
                        lpPcb->Resolution = AWRES_mm080_038;
                        DebugPrintEx(DEBUG_MSG,"Normal resolution");
                    }
                    else if (pTG->class2_commands.parameters[count][0] & 1 )
                    {
                        // Resolution when reported by a DIS frame indicates
                        // it accepts either fine or normal. When reported
                        // in a DCS, it means the negotiated value is FINE.
                        if (pTG->class2_commands.command[count] == CL2DCE_FDIS)
                        {
                            // we received a DIS
                            lpPcb->Resolution = AWRES_mm080_038 | AWRES_mm080_077;
                            DebugPrintEx(DEBUG_MSG,"Normal & Fine resolution");
                        }
                        else
                        {
                            // we received a DCS
                            lpPcb->Resolution = AWRES_mm080_077;
                            DebugPrintEx(DEBUG_MSG,"Fine resolution");
                        }
                    }
                    else
                    {
                        DebugPrintEx(DEBUG_MSG,"Fall through - Fine resolution");
                        lpPcb->Resolution = AWRES_mm080_077;
                    }

                    //  Assign encoding scheme.
                    if( pTG->class2_commands.parameters[count][4] == 0)
                    {
                        lpPcb->Encoding = MH_DATA;
                        DebugPrintEx(DEBUG_MSG,"MH Encoding");
                    }
                    else if ((pTG->class2_commands.parameters[count][4] == 1) ||
                             (pTG->class2_commands.parameters[count][4] == 2) ||
                             (pTG->class2_commands.parameters[count][4] == 3) )
                    {
                        lpPcb->Encoding = MH_DATA | MR_DATA;
                        DebugPrintEx(DEBUG_MSG,"MR Encoding");
                    }
                    else
                    {
                        DebugPrintEx(DEBUG_ERR,"Failed to assign encoding");
                        return FALSE;
                    }

                    //  Assign page width.
                    if( pTG->class2_commands.parameters[count][2] == 0)
                    {
                        lpPcb->PageWidth = WIDTH_A4;
                        DebugPrintEx(DEBUG_MSG,"A4 Width");
                    }
                    else if (pTG->class2_commands.parameters[count][2] == 1)
                    {
                        lpPcb->PageWidth = WIDTH_B4;
                        DebugPrintEx(DEBUG_MSG,"B4 Width");
                    }
                    else if (pTG->class2_commands.parameters[count][2] == 2)
                    {
                        lpPcb->PageWidth = WIDTH_A3;
                        DebugPrintEx(DEBUG_MSG,"A3 Width");
                    }
                    // We don't support 3 and 4 (A5, A6)
                    // but we'll still allow them and map them to A4
                    // This is for Elliot bug #1252 - it should have
                    // no deleterious effect, since this width field
                    // is not used for anything at the point in where
                    // bug 1252 occurs. FrankFi
                    else if (pTG->class2_commands.parameters[count][2] == 3)
                    {
                        lpPcb->PageWidth = WIDTH_A4;
                        DebugPrintEx(DEBUG_MSG,"A4 Width - we don't support A5");
                    }
                    else if (pTG->class2_commands.parameters[count][2] == 4)
                    {
                        lpPcb->PageWidth = WIDTH_A4;
                        DebugPrintEx(DEBUG_MSG,"A4 Width - we don't support A6");
                    }
                    else
                    {
                        DebugPrintEx(DEBUG_ERR,"Failed to assign width");
                        return FALSE;
                    }

                    //  Assign page length.
                    if( pTG->class2_commands.parameters[count][3] == 0)
                    {
                        lpPcb->PageLength = LENGTH_A4;
                        DebugPrintEx(DEBUG_MSG,"A4 Length");
                    }
                    else if (pTG->class2_commands.parameters[count][3] == 1)
                    {
                        lpPcb->PageLength = LENGTH_B4;
                        DebugPrintEx(DEBUG_MSG,"B4 Length");
                    }
                    else if (pTG->class2_commands.parameters[count][3] == 2)
                    {
                        lpPcb->PageLength = LENGTH_UNLIMITED;
                        DebugPrintEx(DEBUG_MSG,"Unlimited Length");
                    }
                    else
                    {
                        DebugPrintEx(DEBUG_ERR,"Invalid length");
                        // assume it is unlimited! Some modems
                        // screw up on length.
                        lpPcb->PageLength = LENGTH_UNLIMITED;
                    }

                    //  Assign baud rate
                    //  For now, we will use the raw numbers returned in the
                    //  DCS command. Dangerous - should fix later!
                    //  These numbers will be tied to the baud rate array in
                    //  the routine that figures out zero byte stuffing from
                    //  the scan line and baud rate.

                    // Fixed the Hack--added a Baud field
                    lpPcb->Baud = pTG->class2_commands.parameters[count][1];
                    if (lpPcb->Baud > CLASS2_MAX_CODE_TO_BPS)
                    {
                        DebugPrintEx(DEBUG_WRN, "Invalid baud rate %d, will be regarded as %d",
                                lpPcb->Baud, CLASS2_MAX_CODE_TO_BPS);
                        lpPcb->Baud = CLASS2_MAX_CODE_TO_BPS;
                    }

                    //  Assign minimum scan time - the first number
                    //  in the MINSCAN_num_num_num constant
                    //  refers to scan time in ms for 100dpi, the
                    //  second for 200dpi, and the last for 400dpi
                    //  Class 2 does not use the 400dpi number,
                    //  but these variables are shared with Class 1
                    if( pTG->class2_commands.parameters[count][7] == 0)
                    {
                        lpPcb->MinScan = MINSCAN_0_0_0;
                    }
                    else if (pTG->class2_commands.parameters[count][7] == 1)
                    {
                        lpPcb->MinScan = MINSCAN_5_5_5;
                    }
                    else if (pTG->class2_commands.parameters[count][7] == 2)
                    {
                        lpPcb->MinScan = MINSCAN_10_5_5;
                    }
                    else if (pTG->class2_commands.parameters[count][7] == 3)
                    {
                        lpPcb->MinScan = MINSCAN_10_10_10;
                    }
                    else if (pTG->class2_commands.parameters[count][7] == 4)
                    {
                        lpPcb->MinScan = MINSCAN_20_10_10;
                    }
                    else if (pTG->class2_commands.parameters[count][7] == 5)
                    {
                        lpPcb->MinScan = MINSCAN_20_20_20;
                    }
                    else if (pTG->class2_commands.parameters[count][7] == 6)
                    {
                        lpPcb->MinScan = MINSCAN_40_20_20;
                    }
                    else if (pTG->class2_commands.parameters[count][7] == 7)
                    {
                        lpPcb->MinScan = MINSCAN_40_40_40;
                    }

                    break;

            case  CL2DCE_FCSI:
            case  CL2DCE_FTSI:
            		hr = Class2CopyID(lpPcb->szID, ARR_SIZE(lpPcb->szID), pTG->class2_commands.parameters[count]);
            		if (FAILED(hr))
            		{
            			DebugPrintEx(DEBUG_WRN,"Class2CopyID failed (ec=0x%08X)",hr);
            		}

                    // prepare CSID for logging by FaxSvc

                    pTG->RemoteID = AnsiStringToUnicodeString(lpPcb->szID);
                    if (pTG->RemoteID)
                    {
                        pTG->fRemoteIdAvail = 1;
                    }
                    break;
            default:
                    break;
        }
    }

    return fFoundDIS_DCS;
}

USHORT Class2EndPageResponseAction(PThrdGlbl pTG)
{
    USHORT csi_count = 0,count;

    DEBUG_FUNCTION_NAME("Class2EndPageResponseAction");

    for(count=0; count < pTG->class2_commands.comm_count; ++count)
    {
        switch (pTG->class2_commands.command[count])
        {
        case CL2DCE_FET:
            switch (pTG->class2_commands.parameters[count][0])
            {
            case 0:  PSSLogEntry(PSS_MSG, 1, "Received MPS");
                     DebugPrintEx(DEBUG_MSG,"More pages coming");
                     return MORE_PAGES;
            case 1:  PSSLogEntry(PSS_MSG, 1, "Received EOM");
                     DebugPrintEx(DEBUG_MSG,"More pages coming");
                     return MORE_PAGES;
            case 2:  PSSLogEntry(PSS_MSG, 1, "Received EOP");
                     DebugPrintEx(DEBUG_MSG,"No more pages coming");
                     return NO_MORE_PAGES;
            default: PSSLogEntry(PSS_MSG, 1, "Received unknown response");
                     DebugPrintEx(DEBUG_MSG,"No more pages coming");
                     return NO_MORE_PAGES;
            }
            break;
        case CL2DCE_FPTS:
            pTG->FPTSreport = pTG->class2_commands.parameters[count][0];
            DebugPrintEx(DEBUG_MSG, "FPTS returned %d", pTG->FPTSreport);
            break;
        }
    }
    return FALSE;
}


extern DWORD PageWidthInPixelsFromDCS[];

/*++
Routine Description:
    Update pTG->TiffInfo according to lpPcb.

Arguments:
    lpPcb - last received DCS, in PCB format

Return Value:
    TRUE for success, FALSE for failure.
--*/
BOOL Class2UpdateTiffInfo(PThrdGlbl pTG, LPPCB lpPcb)
{
    DEBUG_FUNCTION_NAME("Class2UpdateTiffInfo");
    if (!lpPcb)
    {
        DebugPrintEx(DEBUG_WRN, "lpPcb==NULL");
        return FALSE;
    }
    if (lpPcb->PageWidth >= 4)      // PageWidthInPixelsFromDCS has 4 entries
    {
        DebugPrintEx(DEBUG_WRN, "Unsupported PageWidth %d", lpPcb->PageWidth);
        return FALSE;
    }
    pTG->TiffInfo.ImageWidth = PageWidthInPixelsFromDCS[lpPcb->PageWidth];
    pTG->TiffInfo.YResolution = (lpPcb->Resolution & (AWRES_mm080_077 |  AWRES_200_200)) ?
        TIFFF_RES_Y : TIFFF_RES_Y_DRAFT;
    pTG->TiffInfo.CompressionType = (lpPcb->Encoding & MR_DATA) ? TIFF_COMPRESSION_MR : TIFF_COMPRESSION_MH;
    return TRUE;
}


/*++
Routine Description:
    Checks whether received DCS (in PCB format) is valid. Currently, only verifies width

Arguments:
    lpPcb - last received DCS, in PCB format

Return Value:
    TRUE for valid, FALSE for invalid.
--*/
BOOL Class2IsValidDCS(LPPCB lpPcb)
{
    DEBUG_FUNCTION_NAME("Class2IsValidDCS");
    if (!lpPcb)
    {
        DebugPrintEx(DEBUG_WRN, "lpPcb==NULL");
        return FALSE;
    }
    if (lpPcb->PageWidth != WIDTH_A4)      // We currently support only A4
    {
        DebugPrintEx(DEBUG_WRN, "Unsupported PageWidth %d", lpPcb->PageWidth);
        return FALSE;
    }
    return TRUE;
}


void Class2InitBC(PThrdGlbl pTG, LPBC lpbc, USHORT uSize, BCTYPE bctype)
{
        _fmemset(lpbc, 0, uSize);
        lpbc->bctype = bctype;
        lpbc->wBCSize = sizeof(BC);
        lpbc->wTotalSize = sizeof(BC);

/**
        lpbc->Fax.AwRes = (AWRES_mm080_038 | AWRES_mm080_077 | AWRES_200_200 | AWRES_300_300);
        lpbc->Fax.Encoding   = MH_DATA;    // ENCODE_ALL eventually!
        lpbc->Fax.PageWidth  = WIDTH_A4;
        lpbc->Fax.PageLength = LENGTH_UNLIMITED;
        lpbc->Fax.MinScan    = MINSCAN_0_0_0;
**/

}

void Class2PCBtoBC(PThrdGlbl pTG, LPBC lpbc, USHORT uMaxSize, LPPCB lppcb)
{
    lpbc->Fax.AwRes         = lppcb->Resolution;
    lpbc->Fax.Encoding      = lppcb->Encoding;
    lpbc->Fax.PageWidth     = lppcb->PageWidth;
    lpbc->Fax.PageLength    = lppcb->PageLength;
}

BOOL Class2GetBC(PThrdGlbl pTG, BCTYPE bctype)
{
    USHORT  uLen;
    LPBC    lpbc;

    DEBUG_FUNCTION_NAME("Class2GetBC");

    if(bctype == BC_NONE)
    {
        DebugPrintEx(DEBUG_MSG,"entering, type = BC_NONE");
        Class2InitBC(pTG, (LPBC)&pTG->bcSendCaps, sizeof(pTG->bcSendCaps), SEND_CAPS);
        pTG->bcSendCaps.Fax.AwRes      = (AWRES_mm080_038 | AWRES_mm080_077);
        pTG->bcSendCaps.Fax.Encoding   = MH_DATA;
        pTG->bcSendCaps.Fax.PageWidth  = WIDTH_A4;
        pTG->bcSendCaps.Fax.PageLength = LENGTH_UNLIMITED;
        return TRUE;
    }

    if(!(lpbc = ICommGetBC(pTG, bctype)))
    {
        return FALSE;
    }

    DebugPrintEx(DEBUG_MSG, "Class2GetBC: entering, type = %d\n\r", bctype);
    DebugPrintEx(DEBUG_MSG, "Some params: encoding = %d, res = %d\n\r", lpbc->Fax.Encoding, lpbc->Fax.AwRes);

    // Depending on the type, pick the correct global BC structure

    if (bctype == SEND_CAPS)
    {
        uLen = min(sizeof(pTG->bcSendCaps), lpbc->wTotalSize);
        _fmemcpy(&pTG->bcSendCaps, lpbc, uLen);
        return TRUE;
    }
    else if (bctype == SEND_PARAMS)
    {
        uLen = min(sizeof(pTG->bcSendParams), lpbc->wTotalSize);
        _fmemcpy(&pTG->bcSendParams, lpbc, uLen);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

BOOL Class2SetProtParams(PThrdGlbl pTG, LPPROTPARAMS lp)
{
    DEBUG_FUNCTION_NAME("Class2SetProtParams");

    pTG->ProtParams2 = *lp;

    DebugPrintEx(   DEBUG_MSG,
                    "Set Class2ProtParams: fV17Send=%d fV17Recv=%d uMinScan=%d",
                    " HighestSend=%d LowestSend=%d",
                    pTG->ProtParams2.fEnableV17Send,
                    pTG->ProtParams2.fEnableV17Recv,
                    pTG->ProtParams2.uMinScan,
                    pTG->ProtParams2.HighestSendSpeed,
                    pTG->ProtParams2.LowestSendSpeed);

    return TRUE;
}

void iNCUParamsReset(PThrdGlbl pTG)
{
    _fmemset(&pTG->NCUParams2, 0, sizeof(pTG->NCUParams2));
    pTG->lpCmdTab = 0;

    pTG->NCUParams2.uSize = sizeof(pTG->NCUParams2);
    // These are used to set S regs etc.
    // -1 means leave modem at default
    pTG->NCUParams2.DialPauseTime   = -1;
    // pTG->NCUParams2.PulseMakeBreak  = pTG->NCUParams2.DialBlind         = -1;
    pTG->NCUParams2.DialBlind         = -1;
    pTG->NCUParams2.SpeakerVolume   = pTG->NCUParams2.SpeakerControl    = -1;
    pTG->NCUParams2.SpeakerRing     = -1;

    // used in Dial
    pTG->NCUParams2.chDialModifier  = 'T';
}


BYTE rgbFlip256[256]  = {
        0x0,    0x80,   0x40,   0xc0,   0x20,   0xa0,   0x60,   0xe0,
        0x10,   0x90,   0x50,   0xd0,   0x30,   0xb0,   0x70,   0xf0,
        0x8,    0x88,   0x48,   0xc8,   0x28,   0xa8,   0x68,   0xe8,
        0x18,   0x98,   0x58,   0xd8,   0x38,   0xb8,   0x78,   0xf8,
        0x4,    0x84,   0x44,   0xc4,   0x24,   0xa4,   0x64,   0xe4,
        0x14,   0x94,   0x54,   0xd4,   0x34,   0xb4,   0x74,   0xf4,
        0xc,    0x8c,   0x4c,   0xcc,   0x2c,   0xac,   0x6c,   0xec,
        0x1c,   0x9c,   0x5c,   0xdc,   0x3c,   0xbc,   0x7c,   0xfc,
        0x2,    0x82,   0x42,   0xc2,   0x22,   0xa2,   0x62,   0xe2,
        0x12,   0x92,   0x52,   0xd2,   0x32,   0xb2,   0x72,   0xf2,
        0xa,    0x8a,   0x4a,   0xca,   0x2a,   0xaa,   0x6a,   0xea,
        0x1a,   0x9a,   0x5a,   0xda,   0x3a,   0xba,   0x7a,   0xfa,
        0x6,    0x86,   0x46,   0xc6,   0x26,   0xa6,   0x66,   0xe6,
        0x16,   0x96,   0x56,   0xd6,   0x36,   0xb6,   0x76,   0xf6,
        0xe,    0x8e,   0x4e,   0xce,   0x2e,   0xae,   0x6e,   0xee,
        0x1e,   0x9e,   0x5e,   0xde,   0x3e,   0xbe,   0x7e,   0xfe,
        0x1,    0x81,   0x41,   0xc1,   0x21,   0xa1,   0x61,   0xe1,
        0x11,   0x91,   0x51,   0xd1,   0x31,   0xb1,   0x71,   0xf1,
        0x9,    0x89,   0x49,   0xc9,   0x29,   0xa9,   0x69,   0xe9,
        0x19,   0x99,   0x59,   0xd9,   0x39,   0xb9,   0x79,   0xf9,
        0x5,    0x85,   0x45,   0xc5,   0x25,   0xa5,   0x65,   0xe5,
        0x15,   0x95,   0x55,   0xd5,   0x35,   0xb5,   0x75,   0xf5,
        0xd,    0x8d,   0x4d,   0xcd,   0x2d,   0xad,   0x6d,   0xed,
        0x1d,   0x9d,   0x5d,   0xdd,   0x3d,   0xbd,   0x7d,   0xfd,
        0x3,    0x83,   0x43,   0xc3,   0x23,   0xa3,   0x63,   0xe3,
        0x13,   0x93,   0x53,   0xd3,   0x33,   0xb3,   0x73,   0xf3,
        0xb,    0x8b,   0x4b,   0xcb,   0x2b,   0xab,   0x6b,   0xeb,
        0x1b,   0x9b,   0x5b,   0xdb,   0x3b,   0xbb,   0x7b,   0xfb,
        0x7,    0x87,   0x47,   0xc7,   0x27,   0xa7,   0x67,   0xe7,
        0x17,   0x97,   0x57,   0xd7,   0x37,   0xb7,   0x77,   0xf7,
        0xf,    0x8f,   0x4f,   0xcf,   0x2f,   0xaf,   0x6f,   0xef,
        0x1f,   0x9f,   0x5f,   0xdf,   0x3f,   0xbf,   0x7f,   0xff
};

#define FLIP(index) (lpb[(index)] = rgbFlip256[lpb[(index)]])

void    cl2_flip_bytes(LPB lpb, DWORD dw)
{
        while (dw>8)
        {
                FLIP(0); FLIP(1); FLIP(2); FLIP(3);
                FLIP(4); FLIP(5); FLIP(6); FLIP(7);
                dw-=8;
                lpb+=8;
        }

        while(dw)
        {
                FLIP(0);
                dw--;
                lpb++;
        }
}


// These values are taken from table 20/T.32: "Hangup status codes"
#define FHNG_DCS_NO_RESPONSE            25
#define FHNG_FAIL_TRAIN_AT_LOWEST_SPEED 27
#define FHNG_MPS_NO_RESPONSE            52
#define FHNG_EOP_NO_RESPONSE            54
#define FHNG_EOM_NO_RESPONSE            56

/*++
Routine Description:
    Calls SignalStatusChange with the appropriate StatusId and StringId, according 
    to dwFHNGReason.
--*/
void Class2SignalFatalError(PThrdGlbl pTG)
{
    DEBUG_FUNCTION_NAME("Class2ReportFatalError");

    if (pTG->fFatalErrorWasSignaled)
    {
        return;
    }
    
    pTG->fFatalErrorWasSignaled = TRUE;
    if (pTG->fFoundFHNG)
    {
        switch (pTG->dwFHNGReason)
        {
            case FHNG_DCS_NO_RESPONSE:
                        SignalStatusChangeWithStringId(pTG, FS_NO_RESPONSE, IDS_NO_RESPONSE);
                        break;
            case FHNG_FAIL_TRAIN_AT_LOWEST_SPEED:
                        if (pTG->Operation==T30_TX)
                        {
                            SignalStatusChangeWithStringId(pTG, FS_SEND_BAD_TRAINING, IDS_SEND_BAD_TRAINING);
                        }
                        else
                        {
                            SignalStatusChangeWithStringId(pTG, FS_RECV_BAD_TRAINING, IDS_RECV_BAD_TRAINING);
                        }
                        break;
            case FHNG_MPS_NO_RESPONSE:
            case FHNG_EOP_NO_RESPONSE:
            case FHNG_EOM_NO_RESPONSE:
                        SignalStatusChangeWithStringId(pTG, FS_NO_RESPONSE, IDS_NO_RESPONSE);
                        break;

            default:    SignalStatusChange(pTG, FS_FATAL_ERROR);
        }
    }
    else
    {
        SignalStatusChange(pTG, FS_FATAL_ERROR);
    }
}

