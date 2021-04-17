    /***************************************************************************
        Name      :     MODEM.C
        Comment   :     Various modem dialog & support functions, specific
                                to COM connected modems. For a modem on the bus
                                everything below & including this file is replaced
                                by the modem driver.

                Copyright (c) Microsoft Corp. 1991, 1992, 1993

        Revision Log
        Num   Date      Name     Description
        --- -------- ---------- -----------------------------------------------
        101     06/04/92        arulm   Modif to SUPPORT to provide a replaceable interface
                                                        and to use new FCom functions.
***************************************************************************/
#define USE_DEBUG_CONTEXT   DEBUG_CONTEXT_T30_CLASS1

#include "prep.h"

#include "mmsystem.h"
#include "modemint.h"
#include "fcomint.h"
#include "fdebug.h"

#include "efaxcb.h"

#define DEFINE_MDMCMDS
#include "mdmcmds.h"

///RSL
#include "glbproto.h"

#include "psslog.h"
#define FILE_ID     FILE_ID_MODEM

void    InitMonitorLogging(PThrdGlbl pTG);


#       pragma message("Compiling with ADAPTIVE_ANSWER")
USHORT iModemGetAdaptiveResp(PThrdGlbl pTG);
#define uMULTILINE_SAVEENTIRE   0x1234 // +++ HACK passed in as fMultiLine
                                                   //  in iiModemDialog to get it so save
                                                   //  entire buffer in FComModem.bEntireReply.

// Need to have these in descending order so that we'll
// Sync at teh highest common speed with auto-bauding modems!
static UWORD rguwSpeeds[] = {57600,19200, 19200, 9600, 2400, 1200, 300, 0};
// static UWORD rguwSpeeds[] = {19200, 2400, 9600, 1200, 300, 0};
// static UWORD rguwSpeeds[] = {2400, 19200, 9600, 1200, 300, 0};

SWORD HayesSyncSpeed(PThrdGlbl pTG, CBPSTR cbszCommand, UWORD uwLen)
{
    /* Internal routine to synchronize with the modem's speed.  Tries to
       get a response from the modem by trying the speeds in rglSpeeds
       in order (terminated by a 0).  If fTryCurrent is nonzero, checks for
       a response before trying to reset the speeds.

       Returns the speed it found, 0 if they're in sync upon entry (only
       checked if fTryCurrent!=0), or -1 if it couldn't sync.
    */
    // short i;
    short ilWhich = -1;

    DEBUG_FUNCTION_NAME(("HayesSyncSpeed"));
    rguwSpeeds[0] = pTG->CurrentSerialSpeed;

    if ( rguwSpeeds[0] == rguwSpeeds[1]) 
    {
        ilWhich++;
    }

    for(;;)
    {
        DebugPrintEx(   DEBUG_MSG,
                        "Trying: ilWhich=%d  speed=%d", 
                        ilWhich,
                        rguwSpeeds[ilWhich]);

        if(iSyncModemDialog(pTG, (LPSTR)cbszCommand, uwLen, cbszOK))
        {
            DebugPrintEx(   DEBUG_MSG,
                            "Succeeded in Syncing at Speed = %d (il=%d)",
                            rguwSpeeds[ilWhich], 
                            ilWhich);

            return (ilWhich>=0 ? rguwSpeeds[ilWhich] : 0);
        }

        /* failed.  try next speed. */
        if (rguwSpeeds[++ilWhich]==0)
        {
            // Tried all speeds. No response
            DebugPrintEx(   DEBUG_ERR,
                            "Cannot Sync with Modem on Command %s", 
                            (LPSTR)cbszCommand);
            return -1;
        }
        if(!FComSetBaudRate(pTG, rguwSpeeds[ilWhich]))
            return -1;
    }
}

SWORD iModemSync(PThrdGlbl pTG)
{
    // The command used here must be guaranteed to be harmless,
    // side-effect free & non-dstructive. i.e. we can issue it
    // at any point in command mode without chnageing the state
    // of teh modem or disrupting anything.
    // ATZ does not qualify. AT does, I think.....

    return HayesSyncSpeed(pTG, cbszAT, sizeof(cbszAT)-1);
}


SWORD iModemReset(PThrdGlbl pTG, CBPSTR szCmd)
{
    SWORD swRet;

    if (szCmd == NULL) 
    {
        return -1;
    }

    if((swRet = HayesSyncSpeed(pTG, szCmd, (UWORD) _fstrlen(szCmd))) < 0)
    {
        return swRet;
    }
    else
    {
        // ATZ may result in a change in the state/baud rate of the modem
        // (eg. Thought board drops to 2400), therefore we must Sync up
        // again because this function is really a Reset&Sync function.

        // instead of syncing up on AT and then doing ATE0, just
        // sync up on ATE0 directly

        if(iModemSync(pTG) < 0)
                return -1;

        /////////////////////
        // the above idea does not work with Sharad's PP9600FXMT
        // somehow I end up sending it ATATE0 and it answers the phone
        // In other cases, the ATE0 simply has no effect (because the AT&F
        // thing above got confused and teh ATE0 ended up just aborting
        // some previous command) and on ATA I get the ATA echoed,
        // get confused (because multi-line is FALSE) & send ATA again
        // which aborts the whole thing....
        //

        return 0;
    }
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define ATV1                "ATV1"
#define AT                  "AT"
#define cr                  "\r"
#define cbszZero            "0"

USHORT
T30ModemInit(PThrdGlbl pTG)
{
    USHORT uLen, uRet;

    /*** Inits (or re-inits) the COM port, Syncs up with Modem (at whatever,
             speed), gets modem capabilities, puts it into CLASS0, syncs again,
              flushes buffers and returns TRUE on success FALSE on failure
    ***/

    DEBUG_FUNCTION_NAME(("T30ModemInit"));

    PSSLogEntry(PSS_MSG, 0, "Modem initialization");

    // Save the profile ID and key string.
    pTG->FComModem.dwProfileID = DEF_BASEKEY;

    uLen = min(_fstrlen(pTG->lpszPermanentLineID), sizeof(pTG->FComModem.rgchKey)-1);
    _fmemcpy(pTG->FComModem.rgchKey, pTG->lpszPermanentLineID, uLen);
    pTG->FComModem.rgchKey[uLen] = 0;

    if (!uLen)
    {
        DebugPrintEx(   DEBUG_ERR,
                        "Bad param: ProfileID=0x%lx; Key=%s",
                        (unsigned long) pTG->FComModem.dwProfileID,
                        (LPSTR) pTG->FComModem.rgchKey);
        return INIT_INTERNAL_ERROR;
    }

    InitMonitorLogging(pTG);

    //
    // Get the modem info before talking to h/w.
    //

    if(uRet = iModemGetCmdTab(  pTG, 
                                &pTG->FComModem.CurrCmdTab, 
                                &pTG->FComModem.CurrMdmCaps))
    {
        goto error;
    }

    // use MultiLine because we may get asynchronous RING responses
    // at arbitrary times when on-hook

    if(pTG->FComModem.CurrCmdTab.szSetup && (uLen=(USHORT)_fstrlen(pTG->FComModem.CurrCmdTab.szSetup)))
    {
        if(OfflineDialog2(pTG, (LPSTR)pTG->FComModem.CurrCmdTab.szSetup, uLen, cbszOK, cbszERROR) != 1)
        {
            DebugPrintEx(   DEBUG_ERR,
                            "Error in SETUP string: %s", 
                            (LPSTR)pTG->FComModem.CurrCmdTab.szSetup);
            // SETUP is usually the defaults?? So do nothing if this fails
            // uRet = INIT_MODEMERROR;
            // goto error;
        }
    }

    switch (pTG->dwSpeakerMode) 
    {
        case MDMSPKR_OFF:
            pTG->NCUParams.SpeakerControl = 0;
            break;

        case MDMSPKR_DIAL:
            pTG->NCUParams.SpeakerControl = 1;
            break;

        case MDMSPKR_ON:
            pTG->NCUParams.SpeakerControl = 2;
            break;

        default:
            pTG->NCUParams.SpeakerControl = 0;
            break;
    }

    switch (pTG->dwSpeakerVolume) 
    {
        case MDMVOL_LOW:
            pTG->NCUParams.SpeakerVolume = 0;
            break;

        case MDMVOL_MEDIUM:
            pTG->NCUParams.SpeakerVolume = 2;
            break;

        case MDMVOL_HIGH:
            pTG->NCUParams.SpeakerVolume = 3;
            break;

        default:
            pTG->NCUParams.SpeakerVolume = 0;
            break;
    }


    pTG->NCUParams.DialBlind      = 4;  //X4

    // need to do this every time after a Reset/AT&F
    if(! iModemSetNCUParams(    pTG, 
                                -1,
                                pTG->NCUParams.SpeakerControl,
                                pTG->NCUParams.SpeakerVolume,
                                pTG->NCUParams.DialBlind,
                                pTG->NCUParams.SpeakerRing))
    {
        DebugPrintEx(DEBUG_WRN,"Can't Set NCU params - Ignoring that");
    }

    // Why is this here??
    FComFlush(pTG);

    pTG->FComStatus.fModemInit = TRUE;
    uRet = INIT_OK;
    goto end;

error:
    FComClose(pTG);
    pTG->FComStatus.fModemInit = FALSE;
    // fall through...
end:
    return uRet;
}


LPCMDTAB iModemGetCmdTabPtr(PThrdGlbl pTG)
{
    return (pTG->FComStatus.fModemInit) ? &pTG->FComModem.CurrCmdTab: NULL;
}


#define PARAMSBUFSIZE   60
#define fDETECT_DIALTONE 1
#define fDETECT_BUSYTONE 2

BOOL iModemSetNCUParams
(
    PThrdGlbl pTG, 
    int comma, 
    int speaker,
    int volume, 
    int fBlind, 
    int fRingAloud
)
{

    char bBuf[PARAMSBUFSIZE];
    USHORT uLen;

    DEBUG_FUNCTION_NAME(("iModemSetNCUParams"));

    _fstrcpy(bBuf, cbszJustAT);
    uLen = sizeof(cbszJustAT)-1;

    // +++ If we want to split this into dial-tone & busy-tone we
    //         Do it here...
    if ( (fBlind >= 0) && (pTG->ModemKeyCreationId == MODEMKEY_FROM_NOTHING) )
    {
        UINT u=0;
        switch(fBlind)
        {
        case 0:
                break;
        case fDETECT_DIALTONE:
                u=2;
                break;
        case fDETECT_BUSYTONE:
                u=3;
                break;
        default:
                u=4;
                break;
        }
        uLen += (USHORT)wsprintf(bBuf+uLen, cbszXn, u);
    }

    if(comma >= 0)
    {
        if(comma > 255)
        {
            comma = 255;
        }
        uLen += (USHORT)wsprintf(bBuf+uLen, cbszS8, comma);
    }
    if(speaker >= 0)
    {
        if(speaker > 2)
        {
            speaker = 2;
        }
        uLen += (USHORT)wsprintf(bBuf+uLen, cbszMn, speaker);
    }
    if(volume >= 0)
    {
        if(volume > 3)
        {
            volume = 3;
        }
        uLen += (USHORT)wsprintf(bBuf+uLen, cbszLn, volume);
    }

    // do something with RingAloud

    bBuf[uLen++] = '\r';
    bBuf[uLen] = 0;

    // use MultiLine because we may get asynchronous RING responses
    // at arbitrary times when on-hook
    if(OfflineDialog2(pTG, (LPSTR)bBuf, uLen, cbszOK, cbszERROR) != 1)
    {
        DebugPrintEx(DEBUG_ERR,"Can't Set NCU params");
        return FALSE;
    }
    return TRUE;
}

UWORD GetCap(PThrdGlbl pTG, CBPSTR cbpstrSend, UWORD uwLen)
{
    UWORD uRet1=0, uRet2=0, uRet3=0;

    DEBUG_FUNCTION_NAME(("GetCap"));
    // We call GetCapAux twice and if they don't match we
    // call it a 3rd time and arbitrate. Provided it doesn't
    // fail the first time.
    if (!(uRet1=GetCapAux(pTG, cbpstrSend, uwLen))) 
        goto end;

    uRet2=GetCapAux(pTG, cbpstrSend, uwLen);
    if (uRet1!=uRet2)
    {
        DebugPrintEx(   DEBUG_WRN,
                        "2nd getcaps return differs 1=%u,2=%u",
                        (unsigned)uRet1,
                        (unsigned)uRet2);

        uRet3=GetCapAux(pTG, cbpstrSend, uwLen);
        if (uRet1==uRet2 || uRet1==uRet3) 
        {
            goto end;
        }
        else if (uRet2==uRet3)
        {
            uRet1=uRet2; 
            goto end;
        }
        else
        {
            DebugPrintEx(   DEBUG_ERR,
                            "all 2 getcaps differ! 1=%u,2=%u, 3=%u",
                            (unsigned) uRet1, (unsigned) uRet2,
                            (unsigned) uRet3);
        }
    }

end: 
    return uRet1;

}

UWORD GetCapAux(PThrdGlbl pTG, CBPSTR cbpstrSend, UWORD uwLen)
{
    NPSTR sz;
    BYTE  speed, high;
    UWORD i, code;
    USHORT  retry;
    USHORT  uRet;

    DEBUG_FUNCTION_NAME(("GetCapAux"));
    retry = 0;
restart:
    retry++;
    if(retry > 2)
            return 0;

    DebugPrintEx(DEBUG_MSG,"Want Caps for (%s)", (LPSTR)cbpstrSend);

    pTG->fMegaHertzHack = TRUE;
    uRet = OfflineDialog2(pTG, (LPSTR)cbpstrSend, uwLen, cbszOK, cbszERROR);
    pTG->fMegaHertzHack=FALSE;

    // sometimes we don't get the OK so try to parse what we got anyway
    DebugPrintEx(DEBUG_MSG,"LastLine = (%s)",(LPSTR)(&(pTG->FComModem.bLastReply)));

    if(uRet == 2)
            goto restart;

    if(_fstrlen((LPSTR)pTG->FComModem.bLastReply) == 0)
            goto restart;

    speed = 0;
    high = 0;
    for(i=0, sz=pTG->FComModem.bLastReply, code=0; i<REPLYBUFSIZE && sz[i]; i++)
    {
            if(sz[i] >= '0' &&  sz[i] <= '9')
            {
                    code = code*10 + (sz[i] - '0');
                    continue;
            }
            // reached a non-numeric char
            // if its teh first after a code, need to process the code.

            switch(code)
            {
            case 0:  continue;      // not the first char after a code
            case 3:  break;
            case 24: break;
            case 48: speed |= V27; break;
            case 72:
            case 96: speed |= V29; break;
            case 73:
            case 97:
            case 121:
            case 145: speed |= V33; break;  // long-train codes
            case 74:
            case 98:
            case 122:
            case 146: speed |= V17; break;  // short-train codes

            //case 92:
            //case 93:      break;
            // case 120: // not legal
            // case 144: // not legal
            default:
                            DebugPrintEx(   DEBUG_WRN,
                                            "Ignoring unknown Modulation code = %d",
                                            code);
                            code=0;
                            break;
            }
            if(code > high)
                    high=(BYTE)code;

            // reset code counter after processing the baud rate code
            code = 0;
    }

    if(speed == 0)
    {
        // got garbage in response to query
        DebugPrintEx(   DEBUG_MSG,
                        "Can't get Caps for (%s) = 0x%04x  Highest=%d", 
                        (LPSTR)cbpstrSend, 
                        speed, 
                        high);
        return 0;
    }

    if(speed == 0x0F) 
        speed = V27_V29_V33_V17;

    DebugPrintEx(   DEBUG_MSG,
                    "Got Caps for (%s) = 0x%04x  Highest=%d", 
                    (LPSTR)cbpstrSend, 
                    speed, 
                    high);

    return MAKEWORD(speed, high);   // speed==low byte
}

BOOL iModemGetCaps
(
    PThrdGlbl pTG, 
    LPMODEMCAPS lpMdmCaps, 
    DWORD dwSpeed, 
    LPSTR lpszReset,
    LPDWORD lpdwGot
)
{
    /** Modem must be synced up and in normal (non-fax) mode.
            Queries available classes,
            HDLC & Data receive and transmit speeds. Returns
            TRUE if Modem is Class1 or Class2, FALSE if not fax modem
            or other error. Sets the fields in the ET30INST struct **/
    // lpszReset, if nonempty, will be used to reset the modem after
    // the FCLASS=? command see comment about US Robotics Sportster below...

    UWORD   i, uwRet;
    BYTE    speed;
    BOOL    err;
    NPSTR   sz;
    USHORT  retry, uResp;

    DEBUG_FUNCTION_NAME(("iModemGetCaps"));
    if (!*lpdwGot) 
    {
        _fmemset(lpMdmCaps, 0, sizeof(MODEMCAPS));
    }

    if (*lpdwGot & fGOTCAP_CLASSES) 
        goto GotClasses;

    for(retry=0; retry<2; retry++)
    {
        pTG->fMegaHertzHack = TRUE;
        uResp = OfflineDialog2(pTG, (LPSTR)cbszQUERY_CLASS, sizeof(cbszQUERY_CLASS)-1, cbszOK, cbszERROR);
        pTG->fMegaHertzHack=FALSE;
        if(uResp != 2)
                break;
    }

    // sometimes we don't get the OK so try to parse what we got anyway
    DebugPrintEx(   DEBUG_MSG, 
                    "LastLine = (%s)", 
                    (LPSTR)(&(pTG->FComModem.bLastReply)));


    lpMdmCaps->uClasses = 0;
    for(i=0, sz=pTG->FComModem.bLastReply; i<REPLYBUFSIZE && sz[i]; i++)
    {
        UINT uDig=0, uDec=(UINT)-1;

        // This code will accept 1.x as class1, 2 as class2 and 2.x as class2.0
        // Also, it will not detect class 1 in 2.1 or class2 in 1.2 etc.
        // (JDecuir newest class2.0 is labeled class2.1, and he talks
        //  of class 1.0...)
        if(sz[i] >= '0' && sz[i] <= '9')
        {
            uDig = sz[i]- '0';
            if (sz[i+1]=='.')
            {
                i++;
                if(sz[i+1] >= '0' && sz[i+1] <= '9')
                {
                    uDec = sz[i] - '0';
                    i++;
                }
            }
        }
        if(uDig==1) 
        {
            lpMdmCaps->uClasses |= FAXCLASS1;
        }
        if(uDig==2) 
        {
            if (uDec==((UINT)-1)) 
            {
                lpMdmCaps->uClasses |= FAXCLASS2;
            }
            else
            {
                lpMdmCaps->uClasses |= FAXCLASS2_0;
            }
        }
    }
    *lpdwGot |= fGOTCAP_CLASSES;

GotClasses:

    if(!lpMdmCaps->uClasses)
    {
        DebugPrintEx(DEBUG_ERR,"Not a fax modem or unsupported fax class");
        *lpdwGot &= ~(fGOTCAP_CLASSES|fGOTCAP_SENDSPEEDS|fGOTCAP_RECVSPEEDS);
        return FALSE;
    }

    if(!(lpMdmCaps->uClasses & FAXCLASS1)) 
        return TRUE;

///////////////// rest is for Class1 only //////////////////////////

    if(lpszReset && *lpszReset && iModemReset(pTG, lpszReset) < 0) 
        return FALSE;
    //////////

    if(!iiModemGoClass(pTG, 1, dwSpeed)) 
        goto NotClass1;

    err = FALSE;
    if (!(*lpdwGot & fGOTCAP_SENDSPEEDS))
    {
        uwRet = GetCap( pTG, cbszQUERY_FTM, sizeof(cbszQUERY_FTM)-1);
        err = (err || uwRet==0);
        speed = LOBYTE(uwRet);
        lpMdmCaps->uSendSpeeds = speed;
        *lpdwGot |= fGOTCAP_SENDSPEEDS;
    }
    if (!(*lpdwGot & fGOTCAP_RECVSPEEDS))
    {
        uwRet = GetCap(pTG, cbszQUERY_FRM, sizeof(cbszQUERY_FRM)-1);
        err = (err || uwRet==0);
        speed = LOBYTE(uwRet);
        lpMdmCaps->uRecvSpeeds = speed;
        *lpdwGot |= fGOTCAP_RECVSPEEDS;
    }

    if(!iiModemGoClass(pTG, 0, dwSpeed))
        err = TRUE;

    if(err)
    {
        DebugPrintEx(DEBUG_ERR,"Cannot get capabilities");
        goto NotClass1;
    }

    DebugPrintEx(DEBUG_MSG,"Got Caps");
    return TRUE;

NotClass1:
    // Reported Class1 but failed AT+FCLASS=1 or one of the Cap queries
    // GVC9624Vbis does this. See bug#1016
    // FIX: Just zap out the Class1 bit. If any other class supported
    // then return TRUE, else FALSE

    lpMdmCaps->uClasses &= (~FAXCLASS1);    // make the Class1 bit==0
    if(lpMdmCaps->uClasses)
    {
        return TRUE;
    }
    else
    {
        *lpdwGot &= ~(fGOTCAP_CLASSES|fGOTCAP_SENDSPEEDS|fGOTCAP_RECVSPEEDS);
        return FALSE;
    }
}

BOOL iModemGoClass(PThrdGlbl pTG, USHORT uClass)
{
    return iiModemGoClass(pTG, uClass, pTG->FComModem.CurrCmdTab.dwSerialSpeed);
}


BOOL iiModemGoClass(PThrdGlbl pTG, USHORT uClass, DWORD dwSpeed)
{
    int i;
    USHORT uBaud;
    
    DEBUG_FUNCTION_NAME(("iiModemGoClass"));

    for(i=0; i<3; i++)
    {
        // UDS V.3257 modem needs this time, because if we send it a
        // command too quickly after the previous response, it ignores
        // it or gets garbage
        Sleep(100);
        FComFlush(pTG);
        PSSLogEntry(PSS_MSG, 2, "send: \"%s\"", rgcbpstrGO_CLASS[uClass]);
        if(!FComDirectSyncWriteFast(pTG, (LPB)rgcbpstrGO_CLASS[uClass], uLenGO_CLASS[uClass]))
            goto error;
        // wait 500ms. Give modem enough time to get into Class1 mode
        // otherwise the AT we send may abort the transition
        Sleep(500);

        if(dwSpeed)
        {
            USHORT usSpeed  = (USHORT) dwSpeed;
            uBaud = usSpeed;
        }
        else if (pTG->SerialSpeedInit) 
        {
           uBaud = pTG->SerialSpeedInit;
        }
        else 
        {
           uBaud = 57600;
        }

        // RSL don't do hard-coded 2400 for class0.

        FComSetBaudRate(pTG, uBaud);

        FComFlush(pTG);
        if(iModemSync(pTG) >= 0)
        {
            return TRUE;
        }
    }
error:
    // no point -- and we'll smash our settings
    // iModemReset();
    // error is already set to ERR_NO_RESPONSE inside HayesSync()
    DebugPrintEx(DEBUG_ERR,"Cant go to Class %d", uClass);
    return FALSE;
}

BOOL iModemClose(PThrdGlbl pTG)
{
    USHORT uLen;
    BOOL fRet=FALSE;

    DEBUG_FUNCTION_NAME(("iModemClose"));

    if(!pTG->FComStatus.fModemInit)
        return TRUE;


    /** Hangs up the phone if it is off hook, closes the COM port
            and returns. If hangup fails then port is also left open. **/


    if(!iModemHangup(pTG))
        goto lNext;


    if (pTG->Comm.fEnableHandoff &&  pTG->Comm.fDataCall) 
    {
        goto lNext;
    }

    if(pTG->FComModem.CurrCmdTab.szExit && (uLen=(USHORT)_fstrlen(pTG->FComModem.CurrCmdTab.szExit)))
    {
        if(OfflineDialog2(pTG, (LPSTR)pTG->FComModem.CurrCmdTab.szExit, uLen, cbszOK, cbszERROR) != 1)
        {
            DebugPrintEx(   DEBUG_ERR,
                            "Error in EXIT string: %s", 
                            (LPSTR)pTG->FComModem.CurrCmdTab.szExit);
        }
    }

lNext:
    if(FComClose(pTG))
    {
        pTG->FComStatus.fModemInit = FALSE;
        fRet=TRUE;
    }

    return fRet;
}

BOOL iModemHangup(PThrdGlbl pTG)
{
    BOOL fRet=FALSE;

    DEBUG_FUNCTION_NAME(("iModemHangup"));

    if(!pTG->FComStatus.fOffHook) 
    {
        DebugPrintEx(   DEBUG_WRN,
                        "The modem is already on-hook!!!! return without doing nothing");
        return TRUE;
    }

    // Note: iModemHangup is called by NCULink in ddi.c.
    // Rather than do adaptive-answer-specific code in ddi.c as well,
    // we simply ignore the hangup command in the following case...

    if (pTG->Comm.fEnableHandoff &&  pTG->Comm.fDataCall)
    {
        DebugPrintEx(DEBUG_WRN,"IGNORING Hangup of datamodem call");
            return TRUE;
    }

    PSSLogEntry(PSS_MSG, 1, "Hanging up");

    // FComDTR(FALSE);              // Lower DTR to hangup in ModemHangup
                                            // Need to have &D2 in init string for this.

    // Do this twice. There is a bizarre case where you drop DTR,
    // then go into Dialog, flush, send ATH0, then the modem gives
    // you an OK for the DTR, and you take it as one for the ATH0
    // maybe that's ok....if this gets too slow, skip this.
    HayesSyncSpeed(pTG, cbszHANGUP, sizeof(cbszHANGUP)-1);

    if(HayesSyncSpeed(pTG, cbszHANGUP, sizeof(cbszHANGUP)-1) < 0)
    {
        FComDTR(pTG, FALSE);         // Lower DTR on stubborn hangups in ModemHangup
        Sleep(1000);    // pause 1 second
        FComDTR(pTG, TRUE);          // raise it again. Some modems return to cmd state
                                                // only when this is raised again

        if(iModemReset(pTG, pTG->FComModem.CurrCmdTab.szReset) < 0)
            goto error;
        if(HayesSyncSpeed(pTG, cbszHANGUP, sizeof(cbszHANGUP)-1) < 0)
            goto error;
    }
    pTG->FComStatus.fOffHook = FALSE;

    if(!iiModemGoClass(pTG, 0, pTG->FComModem.CurrCmdTab.dwSerialSpeed))
        goto end;
            // Can also ignore this return value. Just for tidier cleanup

    // Avoid! we'll smash our settings
    // iModemReset();
    fRet=TRUE;
    goto end;

error:
    FComDTR(pTG, TRUE);          // raise it again
    // fall through...

end:
    return fRet;
}

/*++
Routine Description:
    Print the dial command to PSS log, hiding the actual number, like this: "ATDT ####"
Arguments:
    pTG
    lpszFormat - format for sprintf, usually #defined as "ATD%c %s"
    chMod - Dial mode ('T' or 'P')
    iLen - Length of number
Return Value:
    None    
 --*/

void LogDialCommand(PThrdGlbl pTG, LPSTR lpszFormat, char chMod, int iLen)
{
    BYTE    bBufHideDest[DIALBUFSIZE] = {'\0'};
    int i;

    sprintf(bBufHideDest, lpszFormat, chMod, TEXT(""));
    if (_tcslen(bBufHideDest)+iLen > DIALBUFSIZE-1)
    {   // Not enough room - don't log!
        return;
    }
    for (i=0; i<iLen; i++)
    {
        strcat(bBufHideDest, "#");
    }
    PSSLogEntry(PSS_MSG, 2, "send: \"%s\"", bBufHideDest);
}


USHORT iModemDial(PThrdGlbl pTG, LPSTR lpszDial)
{
    ULONG   ulTimeout;
    USHORT  uRet, uLen, uDialStringLen;
    BYTE    bBuf[DIALBUFSIZE];
    CBPSTR  cbpstr;
    char    chMod = pTG->NCUParams.chDialModifier;
    DWORD   dwDialTime = 0;
    char    KeyName[200];
    HKEY    hKey;
    char    BlindDialString[200];
    char    RegBlindDialString[200];
    long    lRet;
    DWORD   dwSize;
    DWORD   dwType;

    DEBUG_FUNCTION_NAME(("iModemDial"));
    
    pTG->FComStatus.fOffHook = TRUE;     // Has to be here. Can get an error return
                                         // below even after connecting
                                         // and we want to hangup after that!!
    pTG->Comm.fDataCall=FALSE;

    //
    // check "Modems->Properties->Connection->Wait for dial tone" setting before dialing
    // to correctly set ATX to possibly blind dial
    //
    if (pTG->fBlindDial) 
    {
       // create default string
       sprintf(BlindDialString, "ATX3\r");
       
       // need to check Unimodem Settings\Blind_On key. 
       sprintf(KeyName, "%s\\Settings", pTG->lpszUnimodemKey);

       lRet = RegOpenKeyEx(
                       HKEY_LOCAL_MACHINE,
                       KeyName,
                       0,
                       KEY_READ,
                       &hKey);
   
       if (lRet != ERROR_SUCCESS) 
       {
          DebugPrintEx(DEBUG_ERR, "Can't read Unimodem Settings key %s", KeyName);
       }
       else 
       {
          dwSize = sizeof(RegBlindDialString); 

          lRet = RegQueryValueEx(
                     hKey,
                     "Blind_On",
                     0,
                     &dwType,
                     RegBlindDialString,
                     &dwSize);

          RegCloseKey(hKey);

          if (lRet != ERROR_SUCCESS) 
          {
              DebugPrintEx( DEBUG_ERR, 
                            "Can't read Unimodem key\\Settings\\Blind_On value");
          }
          else if (RegBlindDialString) 
          {
             sprintf(BlindDialString, "AT%s\r", RegBlindDialString);
          }
       }
    }

    if(!iiModemGoClass(pTG, 1, pTG->FComModem.CurrCmdTab.dwSerialSpeed))
    {
        uRet = CONNECT_ERROR;
        goto error;
    }

    //
    // blind dial set here if requested by user
    //
    if (pTG->fBlindDial && BlindDialString) 
    {
       uLen = (USHORT)strlen(BlindDialString);
       if(OfflineDialog2(pTG, BlindDialString, uLen, cbszOK, cbszERROR) != 1)
       {
           DebugPrintEx(    DEBUG_ERR,
                            "Error in BLIND DIAL string: %s", 
                            BlindDialString);
       }
    }

    if(pTG->FComModem.CurrCmdTab.szPreDial && (uLen=(USHORT)_fstrlen(pTG->FComModem.CurrCmdTab.szPreDial)))
    {
        if(OfflineDialog2(pTG, (LPSTR)pTG->FComModem.CurrCmdTab.szPreDial, uLen, cbszOK, cbszERROR) != 1)
        {
            DebugPrintEx(   DEBUG_ERR,
                            "Error in PREDIAL string: %s", 
                            (LPSTR)pTG->FComModem.CurrCmdTab.szPreDial);
        }
    }

    cbpstr = cbszDIAL;

    // If the dial string already has a T or P prefix, we use that
    // instead.
    {
        char c=0;
        while((c=*lpszDial) && c==' ')
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

    // in mdmcmds.h you can find this line: cbszDIAL = "ATD%c %s\r"
    uLen = (USHORT)wsprintf(bBuf, cbpstr, chMod, (LPSTR)lpszDial);

    // Need to set an approriate timeout here. A minimum of 15secs is too short
    // (experiment calling machines within a PABX), plus one has to give extra
    // time for machines that pick up after 2 or 4 rings and also for long distance
    // calls. I take a minumum of 30secs and add 3secs for each digits over 7
    // (unless it's pulse dial in which case I add 8secs/digit).
    // (I'm assuming that a long-distance call will take a minimum of 8 digits
    // anywhere in ths world!). Fax machines I've tested wait about 30secs
    // independent of everything.

    uDialStringLen = (USHORT)_fstrlen(lpszDial);

    ulTimeout = DIAL_TIMEOUT;
    if(uDialStringLen > 7)
    {
            ulTimeout += ((chMod=='p' || chMod=='P')?8000:3000)
                                     * (uDialStringLen - 7);
    }


    pTG->FComStatus.fInDial = TRUE;
    // look for MultiLine, just in case we get echo or garbage.
    // Nothing lost, since on failure of this we can't do anything

    // uRet = iiModemDialog((LPB)bBuf, uLen, ulTimeout, TRUE, 1, TRUE,
    //                                       cbszCONNECT, cbszBUSY, cbszNOANSWER,
    //                                       cbszNODIALTONE, cbszERROR, (CBPSTR)NULL);
    // Send seperately & use iiModemDialog only for the response

    // all this just to send the ATDT
    FComFlushOutput(pTG);
    Sleep(200);     // 100 is not too long for this IMPORTANT one!
    FComFlushInput(pTG);

    LogDialCommand(pTG, cbszDIAL, chMod, uDialStringLen);

    FComDirectAsyncWrite(pTG, bBuf, uLen);
    // now try to get a response
    dwDialTime = GetTickCount();
    uRet = iiModemDialog(   pTG, 
                            0, 
                            0, 
                            ulTimeout, 
                            TRUE, 
                            1, 
                            TRUE,
                            cbszCONNECT, 
                            cbszBUSY, 
                            cbszNOANSWER,
                            cbszNODIALTONE, 
                            cbszERROR, 
                            cbszBLACKLISTED,
                            cbszDELAYED,
                            cbszNOCARRIER, 
                            (CBPSTR)NULL);

    pTG->FComStatus.fInDial = FALSE;
    DebugPrintEx(DEBUG_MSG,"ModemDial -- got %d response from Dialog", uRet);


#if !((CONNECT_TIMEOUT==0) && (CONNECT_OK==1) && (CONNECT_BUSY==2) && (CONNECT_NOANSWER == 3) && (CONNECT_NODIALTONE==4) && (CONNECT_ERROR==5) && (CONNECT_BLACKLISTED==6) && (CONNECT_DELAYED==7))
#error CONNECT defines not correct ERROR, OK, BUSY, NOANSWER, NODIALTONE == CONNECT_ERROR, CONNECT_OK, CONNECT_BUSY, CONNECT_NOANSWER, CONNECT_NODIALTONE
#else
#pragma message("verified CONNECT defines")
#endif


    switch(uRet)
    {
    case CONNECT_TIMEOUT:
        pTG->fFatalErrorWasSignaled = 1;
        SignalStatusChange(pTG, FS_NO_ANSWER);
        PSSLogEntry(PSS_ERR, 1, "Response - timeout");
        break;

    case CONNECT_OK:
        PSSLogEntry(PSS_MSG, 1, "Response - CONNECT");
        pTG->fReceivedHDLCflags = TRUE;
        break;

    case CONNECT_BUSY:
        pTG->fFatalErrorWasSignaled = 1;
        SignalStatusChange(pTG, FS_BUSY);
        PSSLogEntry(PSS_ERR, 1, "Response - BUSY");
        break;

    case CONNECT_NOANSWER:
        pTG->fFatalErrorWasSignaled = 1;
        SignalStatusChange(pTG, FS_NO_ANSWER);
        PSSLogEntry(PSS_ERR, 1, "Response - NO ANSWER");
        break;

    case CONNECT_NODIALTONE:
        pTG->fFatalErrorWasSignaled = 1;
        SignalStatusChange(pTG, FS_NO_DIAL_TONE);
        PSSLogEntry(PSS_ERR, 1, "Response - NO DIALTONE");
        break;

    case CONNECT_ERROR:
        pTG->fFatalErrorWasSignaled = 1;
        SignalStatusChange(pTG, FS_NO_ANSWER);
        PSSLogEntry(PSS_ERR, 1, "Response - ERROR");
        break;

    case CONNECT_BLACKLISTED:
        pTG->fFatalErrorWasSignaled = 1;
        SignalStatusChange(pTG, FS_CALL_BLACKLISTED);
        PSSLogEntry(PSS_ERR, 1, "Response - BLACKLISTED");
        break;

    case CONNECT_DELAYED:
        pTG->fFatalErrorWasSignaled = 1;
        SignalStatusChange(pTG, FS_CALL_DELAYED);
        PSSLogEntry(PSS_ERR, 1, "Response - DELAYED");
        break;

    case 8: {
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

    }

    if(uRet == CONNECT_OK)
    {
            goto done;
    }
    else
    {
            if(uRet == CONNECT_TIMEOUT)     
            {
                pTG->fFatalErrorWasSignaled = 1;
                SignalStatusChange(pTG, FS_NO_ANSWER);

                uRet = CONNECT_NOANSWER;
                    // call it a no answer
            }

            goto error;
    }

error:
    if(!iModemHangup(pTG))
    {
        // at this point in teh production version we
        // need to call some OS reboot function!!
        DebugPrintEx(DEBUG_ERR,"Can't Hangup after DIALFAIL");
        uRet = CONNECT_ERROR;
    }
    // fall through
done:
    return uRet;
}


USHORT   iModemAnswer(PThrdGlbl pTG)
{
    CBPSTR  cbpstr;
    USHORT  uLen, uRet;
    char    Command[400];
    int     i;

    DEBUG_FUNCTION_NAME(("iModemAnswer"));

    pTG->FComStatus.fOffHook=TRUE;       // Has to be here. Can screwup after answering
                                                            // but before CONNECT and we want to hangup
                                                            // after that!!
    pTG->Comm.fDataCall=FALSE;

    //
    // below is Adaptive Answer handling. 
    // It is separate because all the commands are defined via INF
    //

    if (pTG->AdaptiveAnswerEnable) 
    {
       for (i=0; i< (int) pTG->AnswerCommandNum; i++) 
       {
          strcpy  (Command, pTG->AnswerCommand[i] );

          if (i == (int) pTG->AnswerCommandNum - 1) 
          {
             // last command-answer
             FComFlushOutput(pTG);
             Sleep(200);     // 100 is not too long for this IMPORTANT one!
             FComFlushInput(pTG);

             PSSLogEntry(PSS_MSG, 2, "send: \"%s\"", Command);
             FComDirectAsyncWrite(pTG, (LPSTR) Command, (USHORT) strlen(Command) );

             pTG->FComStatus.fInAnswer = TRUE;
             
             break;

          }


          if( (uRet = OfflineDialog2(pTG, (LPSTR) Command, (USHORT) strlen(Command), cbszOK, cbszERROR) ) != 1)    
          {
              DebugPrintEx(DEBUG_ERR, "Answer %d=%s FAILED", i, Command);
          }
          else 
          {
              DebugPrintEx(DEBUG_MSG, "Answer %d=%s rets OK", i, Command);
          }
       }

       uRet=iModemGetAdaptiveResp(pTG);
       pTG->FComStatus.fInAnswer=FALSE;
       if (uRet==CONNECT_OK) 
           goto done;
       else          
           goto error;
    }

    //
    // assuming FAX call since can't determine that anyway...
    //
    else
    {
            // 5/95 JosephJ:Elliot Bug#3421 -- we issue the AT+FCLASS=1 command
            //      twice so that if one gets zapped by a RING the other will
            //          be OK.
            if (pTG->FComModem.CurrCmdTab.dwFlags&fMDMSP_ANS_GOCLASS_TWICE)
                    iiModemGoClass(pTG, 1, pTG->FComModem.CurrCmdTab.dwSerialSpeed);
            if(!iiModemGoClass(pTG, 1, pTG->FComModem.CurrCmdTab.dwSerialSpeed))
            {
                    uRet = CONNECT_ERROR;
                    goto error;
            }
    }

    if(pTG->FComModem.CurrCmdTab.szPreAnswer && (uLen=(USHORT)_fstrlen(pTG->FComModem.CurrCmdTab.szPreAnswer)))
    {
            if(OfflineDialog2(pTG, (LPSTR)pTG->FComModem.CurrCmdTab.szPreAnswer, uLen, cbszOK, cbszERROR) != 1)
            {
                    DebugPrintEx(   DEBUG_WRN,
                                    "Error on PREANSWER string: %s", 
                                    (LPSTR)pTG->FComModem.CurrCmdTab.szPreAnswer);
            }
    }



#define ANSWER_TIMEOUT 40000                            // Random Timeout
// Need to wait reasonably long, so that we don't give up too easily

    cbpstr = cbszANSWER;
    uLen = sizeof(cbszANSWER)-1;

    pTG->FComStatus.fInAnswer = TRUE;

    // if(!iModemDialog((LPSTR)cbpstr, uLen, ANSWER_TIMEOUT, cbszCONNECT))
    // look for MultiLine, just in case we get echo or garbage.
    // Nothing lost, since on failure of this we can't do anything

    // if(!iiModemDialog((LPB)cbpstr, uLen, ANSWER_TIMEOUT, TRUE, 1, TRUE,
    //                                       cbszCONNECT, (CBPSTR)NULL))
    // Send seperately & use iiModemDialog only for the response

    // all this just to send the ATA


    FComFlushOutput(pTG);
    Sleep(200);     // 100 is not too long for this IMPORTANT one!
    FComFlushInput(pTG);
    PSSLogEntry(PSS_MSG, 2, "send: \"%s\"", cbpstr);
    FComDirectAsyncWrite(pTG, cbpstr, uLen);

    // this is used to complete a whole IO operation (presumably a short one)
    // when this flag is set, the IO won't be disturbed by the abort event
    // this flag should NOT be set for long periods of time since abort
    // is disabled while it is set.
    pTG->fStallAbortRequest = TRUE;
    // now try to get a response
    
    if(!iiModemDialog(pTG, 0, 0, ANSWER_TIMEOUT, TRUE, 1, TRUE, cbszCONNECT, (CBPSTR)NULL))
    {
        pTG->FComStatus.fInAnswer = FALSE;
        PSSLogEntry(PSS_ERR, 1, "Response - ERROR");

        // try to hangup and sync with modem. This should work
        // even if phone is not really off hook
        uRet = CONNECT_ERROR;
        goto error;
    }
    else
    {
        pTG->FComStatus.fInAnswer = FALSE;
        PSSLogEntry(PSS_MSG, 1, "Response - CONNECT");

        uRet = CONNECT_OK;
        goto done;
    }

error:

    if (pTG->Comm.fEnableHandoff && uRet==CONNECT_WRONGMODE_DATAMODEM)
    {
        // We won't hangup.
        // We deliberately leave pTG->FComStatus.fOffHook to TRUE, because
        // it is off hook.
        goto done;
    }

    if(!iModemHangup(pTG))
    {
        // at this point in teh production version we need to
        // call some OS reboot function!!
        DebugPrintEx(DEBUG_ERR,"Can't Hangup after ANSWERFAIL");
        uRet = CONNECT_ERROR;
    }
    // fall through

done:
    return uRet;

}


int my_strcmp(LPSTR sz1, LPSTR sz2)
{

   if ( (sz1 == NULL) || (sz2 == NULL) ) 
   {
       return FALSE;
   }

   if ( strcmp(sz1, sz2) == 0 ) 
   {
      return TRUE;
   }

   return FALSE;

}


BOOL fHasNumerals(PThrdGlbl pTG, LPSTR sz)
{
        int i;

        if (sz == NULL) 
        {
            return FALSE;
        }

        for(i=0; sz[i]; i++)
        {
                if(sz[i] >= '0' && sz[i] <= '9')
                        return TRUE;
        }
        return FALSE;
}


#define DIALOGRETRYMIN  600
#define SECONDLINE_TIMEOUT      500
#define ABORT_TIMEOUT    250
#ifdef DEBUG
#       define DEFMONVAL 1
#else   //!DEBUG
#       define DEFMONVAL 0
#endif  //!DEBUG
#define szMONITOREXISTINGFILESIZE "MonitorMaxOldSizeKB"
#define szMONITORDIR                      "MonitorDir"


UWORD far iiModemDialog
(   PThrdGlbl pTG, 
    LPSTR szSend, 
    UWORD uwLen, 
    ULONG ulTimeout,
    BOOL fMultiLine, 
    UWORD uwRepeatCount, 
    BOOL fPause,
    CBPSTR cbpstrWant1, 
    CBPSTR cbpstrWant2,
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
    CBPSTR  rgcbszWant[10];
    va_list ap;
    LPTO    lpto, lptoRead, lpto0;
    BOOL    fGotFirstLine, fFirstSend;
    ULONG   ulLeft;
    UINT    uPos=0;

    DEBUG_FUNCTION_NAME(("iiModemDialog"));
    pTG->FComModem.bEntireReply[0]=0;

    // ensure that we'll abort in FComm only on fresh calls to NCUAbort
    // protecting ourselves against this var being randomly left set.
    // Note we check this variable _just_ before calling ModemDialog
    // in NCUDial and NCUAnswer & assuming atomicity between then and here
    // we'll never miss an abort in a Dial/Answer

    // extract the (variable length) list of acceptable responses.
    // each is a CBSZ, code based 2 byte ptr

    // first response always present
    rgcbszWant[1] = cbpstrWant1;

    if((rgcbszWant[2] = cbpstrWant2) != NULL)
    {
        // if more than one response
        va_start(ap, cbpstrWant2);
        for(j=3; j<10; j++)
        {
                if((rgcbszWant[j] = va_arg(ap, CBPSTR)) == NULL)
                        break;
        }
        uwWantCount = j-1;
        va_end(ap);
    }
    else
    {
        uwWantCount = 1;
    }

    if(szSend)
    {
        DebugPrintEx(   DEBUG_MSG, 
                        "Dialog: Send (%s) len=%d WantCount=%d time=%ld rep=%d", 
                        (LPSTR)szSend,
                        uwLen, 
                        uwWantCount, 
                        ulTimeout, 
                        uwRepeatCount);
    }
    else
    {
        DebugPrintEx(   DEBUG_MSG, 
                        "Response: WantCount=%d time=%ld rep=%d",
                        uwWantCount, 
                        ulTimeout, 
                        uwRepeatCount);
    }
    for(j=1; j<=uwWantCount; j++)
    {
        DebugPrintEx(DEBUG_MSG,"Want %s",(LPSTR)(rgcbszWant[j]));
    }

    lpto = &(pTG->FComModem.toDialog);
    lpto0 = &(pTG->FComModem.toZero);
    pTG->FComStatus.fInDialog = TRUE;

    // Try the dialog upto uwRepeatCount times
    for(uwRet=0, i=0; i<uwRepeatCount; i++)
    {
        startTimeOut(pTG, lpto, ulTimeout);
        fFirstSend = TRUE;
        do
        {
            if(szSend)
            {
                if(!fFirstSend)
                {
                    ulLeft = leftTimeOut(pTG, lpto);
                    if(ulLeft <= DIALOGRETRYMIN)
                    {
                        DebugPrintEx(DEBUG_MSG,"ulLeft=%ul too low",ulLeft);
                        break;
                    }
                    else
                    {
                        DebugPrintEx(DEBUG_MSG,"ulLeft=%ul OK",ulLeft);
                    }
                }
                fFirstSend = FALSE;

                // If a command is supplied, write it out, flushing input
                // first to get rid of spurious input.

        /*** SyncWrite calls Drain here which we should not need **
         *** as we are immediately waiting for a response *********
         **********************************************************
                if(!FComDirectSyncWrite(szSend, uwLen))
         **********************************************************/

                if(fPause)
                        Sleep(40);      // 100 is too long

                // FComFlushInput();
                FComFlush(pTG);            // Need to flush output too? Maybe...
                // there's nowhere else to flush/loosen up teh output

                // The flush has to be as late in the game as possible,
                // because if teh previous command got confused & accepted
                // a response to an earlier command or something, then
                // it's response may still be in transit (this happened
                // on Sharad's PP9600FXMT), so the later we do this the
                // better. So we send the entire command w/o teh \r,
                // wait for it to drain, then Flush again (input only
                // this time) then send the CR

				///////// Potential Major source of failures ////////
				// DirectSyncWrite calls Drain which calls DllSleep if everything
				// is not drained, so we could end up waiting for 1 time slice
				// which is at least 50ms and looks like it can be much higher on
				// some machines. This was screwing up our AT+FTM=96 is some cases
				// FIX: Enter Crit section here exit after this is done
				//////////////////////////////////////////////////////

                PSSLogEntry(PSS_MSG, 2, "send: \"%s\"", szSend);
                
                if(!FComDirectSyncWriteFast(pTG, szSend, (UWORD)(uwLen-1)))
                {
                    // Need to check that we are sending only ASCII or pre-stuffed data here
                    DebugPrintEx(DEBUG_ERR,"Modem Dialog Sync Write timed Out");
                    uwRet = 0;
                    goto error;
                    // If Write fails, fail & return immediately.
                    // SetMyError() will already have been called.
                }
                // output has drained. Now flush input
                FComFlushInput(pTG);
                // and then send the CR
                if(!FComDirectAsyncWrite(pTG, "\r", 1))
                {
                    DebugPrintEx(DEBUG_ERR,"Modem Dialog Write timed Out on CR");
                    uwRet = 0;
                    goto error;
                }
            }

            // Try to get a response until timeout or bad response
            pTG->FComModem.bLastReply[0] = 0;
            fGotFirstLine=FALSE;

            for(lptoRead=lpto;;startTimeOut(pTG, lpto0, SECONDLINE_TIMEOUT), lptoRead=lpto0)
            {
                    // get a CR-LF terminated line
                    // for the first line use macro timeout, for multi-line
                    // responses use 0 timeout.
retry:
                    swNumRead = FComFilterReadLine(pTG, bReply, REPLYBUFSIZE-1, lptoRead);
                    DebugPrintEx(DEBUG_MSG,"FComFilterReadLine returns %d",swNumRead);
                    if(swNumRead == 2 && bReply[0] == '\r' && bReply[1] == '\n')
                            goto retry;             // blank line -- throw away & get another

                    // Fix Bug#1226. Elsa Microlink returns this garbage line in
                    // response to AT+FCLASS=?, followed by the real reply. Since
                    // we only look at the first line, we see only this garbage line
                    // and we never see the real reply (0, 1, 2, 2.0)
                    if(swNumRead==3 && bReply[0]==0x13 && bReply[1]=='\r' && bReply[2]=='\n')
                            goto retry;

                    // Fix Elliot bug#3619 -- German modem TE3801 sends us
                    // \r\r\nOK\r\n -- so we treat \r\r\n as blank line.
                    if(swNumRead==3 && bReply[0]=='\r' && bReply[1]=='\r' && bReply[2]=='\n')
                            goto retry;

                    if(swNumRead == 0)      // timeout
                    {
                        if(fGotFirstLine)
                        {
                            // for MegaHertz, which returns no OK after
                            // capabilities queries
                            if(pTG->fMegaHertzHack)
                            {
                                if(fHasNumerals(pTG, pTG->FComModem.bLastReply))
                                {
                                    uwRet = 1;
                                    goto end;
                                }
                            }
                            break;
                        }
                        else
                        {
                            goto timeout;
                        }
                    }
                    if(swNumRead < 0)       // error-but lets see what we got anyway
                            swNumRead = (-swNumRead);

                    fGotFirstLine=TRUE;


                    //
                    // +++ HACK:
                    // We add everything upto the first NULL of each
                    // line of reply to bEntireReply, for the specific
                    // case of fMultiLine==uMULTILINE_SAVEENTIRE
                    // This is so we save things like:
                    // \r\nDATA\r\n\r\nCONNECT 12000\r\n
                    //
                    if(pTG->Comm.fEnableHandoff && fMultiLine==uMULTILINE_SAVEENTIRE
                            && uPos<sizeof(pTG->FComModem.bEntireReply))
                    {
                        UINT cb;
                        bReply[REPLYBUFSIZE-1]=0;
                        cb = _fstrlen(bReply);
                        if ((cb+1)> (sizeof(pTG->FComModem.bEntireReply)-uPos))
                        {
                            DebugPrintEx(DEBUG_WRN, "bEntireReply: out of space");
                            cb=sizeof(pTG->FComModem.bEntireReply)-uPos;
                            if (cb) cb--;
                        }
                        _fmemcpy((LPB)pTG->FComModem.bEntireReply+uPos, (LPB)bReply, cb);
                        uPos+=cb;
                        pTG->FComModem.bEntireReply[uPos]=0;
                    }

                    PSSLogEntry(PSS_MSG, 2, "recv:     \"%s\"", bReply);

                    for(bReply[REPLYBUFSIZE-1]=0, j=1; j<=uwWantCount; j++)
                    {
                        if(rgcbszWant[j] && (strstr(bReply, rgcbszWant[j]) != NULL))
                        {
                            uwRet = j;
                            goto end;
                        }
                    }


                    if(!fMultiLine)
                            break;
                    // Got something unknown
                    // Retry command and response until timeout

                    // We reach here it IFF we got a non blank reply, but it wasn't what
                    // we wanted. Squirrel teh first line away somewhere so that we can
                    // retrieve is later. We use this hack to get multi-line informational
                    // responses to things like +FTH=? Very important to ensure that
                    // blank-line replies don't get recorded here. (They may override
                    // the preceding line that we need!).

                    if( (pTG->FComModem.bLastReply[0] == 0) ||
                        ( ! _fstrcmp(pTG->FComModem.bLastReply, cbszRING) ) ) 
                    {
                                // copy only if _first_ response line
                            _fmemcpy((LPB)pTG->FComModem.bLastReply, (LPB)bReply, REPLYBUFSIZE);
                    }
                    // copies whole of bReply which includes zero-termination put
                    // there by FComFilterReadLine
                    DebugPrintEx(   DEBUG_MSG,
                                    "Saved line (%s)", 
                                    (LPSTR)(&(pTG->FComModem.bLastReply)));
            }
        }
        while(checkTimeOut(pTG, lpto));

        if(fGotFirstLine)
                continue;

        DebugPrintEx(DEBUG_WRN,"Weird!! got timeout in iiModemDialog loop");
timeout:
        PSSLogEntryStrings(PSS_WRN, 2, &rgcbszWant[1], uwWantCount, 
                "failed to receive expected response: ");
        // Need to send anychar to abort the previous command.
        // use random 120ms timeout -- too short. upped to 250
        // send \rAT\r
        // no need for pause--we just timed out!!

        PSSLogEntry(PSS_MSG, 2, "send: \"AT\"");
        FComFlush(pTG); // flush first--don't wnat some old garbage result
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
        if (bReply[0])
        {
            PSSLogEntry(PSS_MSG, 2, "recv:     \"%s\"", bReply);
        }
        if(bReply[0] && strstr(bReply, cbszOK)==NULL)
            DebugPrintEx(   DEBUG_ERR,
                            "Anykey abort reply not OK. Got <<%s>>", 
                            (LPSTR)bReply);

        // Need Flush here, because \rAT\r will often get us
        // a cr-lf-OK-cr-lf-cr-lfOK-cr-lf response. If we send
        // just a \r, sometimes we may get nothing

        // FComFlushInput();
        FComFlush(pTG);
    }

error:
    DebugPrintEx(   DEBUG_WRN,
                    "(%s) --> (%d)(%s, etc) Failed", 
                    (LPSTR)(szSend?szSend:"null"), 
                    uwWantCount, 
                    (LPSTR)rgcbszWant[1]);

    pTG->FComStatus.fInDialog = 0;
    return 0;

end:

    DebugPrintEx(DEBUG_MSG,"GOT IT %d (%s)", uwRet, (LPSTR)(rgcbszWant[uwRet]));
    pTG->FComStatus.fInDialog = 0;
    return uwRet;
}


void InitMonitorLogging(PThrdGlbl pTG)
{
    DEBUG_FUNCTION_NAME(("InitMonitorLogging"));
    pTG->Comm.fEnableHandoff=1;
    if (pTG->Comm.fEnableHandoff)
    {
        DebugPrintEx(DEBUG_WRN,"ADAPTIVE ANSWER ENABLED");
    }
}

// RSL was 60 000
#define AA_ANSWER_TIMEOUT       40000

USHORT iModemGetAdaptiveResp(PThrdGlbl pTG)
{
    USHORT                uRet=CONNECT_ERROR;
    BOOL                  fGotOK=FALSE;
    BOOL                  fGotData=FALSE;
    BOOL                  fGotFax=FALSE;
    LONG                  lRet;
    char                  Command[400];


    DEBUG_FUNCTION_NAME(("iModemGetAdaptiveResp"));

    pTG->Comm.fDataCall = FALSE;
    //
    // handle Adaptive Answer
    // should get FAX/DATA response
    //
    switch( iiModemDialog(  pTG, 
                            0, 
                            0, 
                            AA_ANSWER_TIMEOUT, 
                            uMULTILINE_SAVEENTIRE,
                            1, 
                            TRUE,
                            pTG->ModemResponseFaxDetect,
                            pTG->ModemResponseDataDetect,
                            cbszCONNECT,
                            cbszOK,
                            (CBPSTR)NULL)) 
    {

         case 1:
              fGotFax = 1;
              DebugPrintEx(DEBUG_MSG,"AdaptiveAnswer: got FAX response");
              break;

         case 2:
             fGotData = 1;
             DebugPrintEx(DEBUG_MSG,"AdaptiveAnswer: got DATA response");
             break;

         case 3:
             DebugPrintEx(DEBUG_ERR,"AnswerPhone: Can't get CONNECT before FAX/DATA");
             pTG->Comm.fDataCall = FALSE;
             uRet = CONNECT_ERROR;
             goto end;

         case 4:
             DebugPrintEx(DEBUG_ERR,"AnswerPhone: Can't get OK before FAX/DATA");
             pTG->Comm.fDataCall = FALSE;
             uRet = CONNECT_ERROR;
             goto end;

         default:
         case 0:   
            DebugPrintEx(DEBUG_ERR,"AnswerPhone: Can't get default before FAX/DATA");
            pTG->Comm.fDataCall = FALSE;
            uRet = CONNECT_ERROR;
            goto end;
    }

    // here we may have to change the serial speed and send some cmds (such as ATO-go online)

    if (fGotFax) 
    {
       if (pTG->SerialSpeedFaxDetect) 
       {
          FComSetBaudRate(pTG, pTG->SerialSpeedFaxDetect);
       }

       if (pTG->HostCommandFaxDetect)  
       {
          strcpy (Command, pTG->HostCommandFaxDetect );

          FComFlushOutput(pTG);
          FComDirectAsyncWrite(pTG, (LPSTR) Command, (USHORT) strlen(Command) );
       }

    }
    else if (fGotData) 
    {
       if (pTG->SerialSpeedDataDetect) 
       {
          FComSetBaudRate(pTG, pTG->SerialSpeedDataDetect);
       }

       if (pTG->HostCommandDataDetect)    
       {
          strcpy (Command, pTG->HostCommandDataDetect );

          FComFlushOutput(pTG);
          FComDirectAsyncWrite(pTG, (LPSTR) Command, (USHORT) strlen(Command) );
       }
    }
    else 
    {
       DebugPrintEx(DEBUG_ERR,"AnswerPhone: LOGICAL PGM ERROR");
       pTG->Comm.fDataCall = FALSE;
       uRet = CONNECT_ERROR;
       goto end;
    }


    // wait for connect now.

    switch( iiModemDialog(  pTG, 
                            0, 
                            0, 
                            AA_ANSWER_TIMEOUT, 
                            uMULTILINE_SAVEENTIRE,
                            1, 
                            TRUE,
                            (fGotFax) ? pTG->ModemResponseFaxConnect : pTG->ModemResponseDataConnect,
                            cbszCONNECT,
                            cbszOK,
                            (CBPSTR)NULL)) 
    {

         case 1:
              if (fGotFax) 
              {
                 uRet=CONNECT_OK;
                 goto end;
              }
              else 
              {
                 goto lDetectDataCall;
              }

         case 2:
            if (fGotFax) 
            {
               uRet=CONNECT_OK;
               goto end;
            }
            else 
            {
               goto lDetectDataCall;
            }

         case 3:
             DebugPrintEx(DEBUG_ERR,"AnswerPhone: Can't get OK after FAX/DATA");
             pTG->Comm.fDataCall = FALSE;
             uRet = CONNECT_ERROR;
             goto end;

         default:
         case 0:
            DebugPrintEx(DEBUG_ERR,"AnswerPhone: Can't get default after FAX/DATA");
            pTG->Comm.fDataCall = FALSE;
            uRet = CONNECT_ERROR;
            goto end;
    }



lDetectDataCall:
    // Now we've got to fake out modem and fcom into thinking that
    // the phone is off hook when in fact it isn't.
    pTG->Comm.fDataCall = TRUE;
    uRet = CONNECT_WRONGMODE_DATAMODEM;
    //
    // New TAPI: Have to switch out of passtrough before handing off the call
    //

    DebugPrintEx(DEBUG_MSG,"AdaptiveAnswer: lineSetCallParams called");

    if (!itapi_async_setup(pTG)) 
    {
        DebugPrintEx(DEBUG_ERR,"AdaptiveAnswer: itapi_async_setup failed");

        pTG->Comm.fDataCall = FALSE;
        uRet = CONNECT_ERROR;
        goto end;
    }

    lRet = lineSetCallParams(pTG->CallHandle,
                             LINEBEARERMODE_VOICE,
                             0,
                             0xffffffff,
                             NULL);

    if (lRet < 0) 
    {
        DebugPrintEx(DEBUG_ERR, "AdaptiveAnswer: lineSetCallParams failed");

        pTG->fFatalErrorWasSignaled = 1;
        SignalStatusChange(pTG, FS_FATAL_ERROR);

        pTG->Comm.fDataCall = FALSE;
        uRet = CONNECT_ERROR;
        goto end;
    }
    else 
    {
         DebugPrintEx(  DEBUG_MSG,
                        "AdaptiveAnswer: lineSetCallParams returns ID %ld", 
                        (long) lRet);
    }

    if(!itapi_async_wait(pTG, (DWORD)lRet, (LPDWORD)&lRet, NULL, ASYNC_TIMEOUT)) 
    {
        DebugPrintEx(DEBUG_ERR, "AdaptiveAnswer: itapi_async_wait failed");
        pTG->fFatalErrorWasSignaled = 1;
        SignalStatusChange(pTG, FS_FATAL_ERROR);

        pTG->Comm.fDataCall = FALSE;
        uRet = CONNECT_ERROR;
        goto end;
    }

    pTG->fFatalErrorWasSignaled = 1;
    SignalStatusChange(pTG, FS_NOT_FAX_CALL);

end:
    return uRet;

}
