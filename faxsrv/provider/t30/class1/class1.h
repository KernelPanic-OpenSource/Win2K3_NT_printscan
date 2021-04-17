/***************************************************************************
 Name     :     CLASS1.H
 Comment  :     Main include file for Windows Comm Class-1 Modem driver

        Copyright (c) Microsoft Corp. 1991 1992 1993

 Revision Log
 Date     Name  Description
 -------- ----- ---------------------------------------------------------
***************************************************************************/



/**---------------------- #define of sizes of things ---------------------

        Frames can be at most 2.55 secs (sent) or 3.45 secs (recvd) long, or
        2.55 * 300/8 = 96 bytes and 132 bytes long respectively

        Dialstrings are limited to 60 bytes (arbitrarily)

        Commands (except dial) are never more than about 10-20 bytes long, so
        we use a buffer of 40 bytes. Replies are never big at all, but we
        might hold a frame in there, so keep it same size as a Framebuffer

        The Dial command is ATDT <string><CR>, so we use 60+10 bytes buffer

---------------------- #define of sizes of things ---------------------**/



/**---------------------- #define of other things ---------------------

        FAX_CLASSn      is used in Modem.FaxClass.

        CHECK_PATTERN is used in the Guard elements.
        ECM_FRAMESIZE in T30.C

---------------------- #define of other things ---------------------**/

#define CR                              0x0d
#define LF                              0x0a
#define DLE                             0x10            // DLE = ^P = 16d = 10h
#define ETX                             0x03

extern BYTE                             bDLEETX[];
extern BYTE                             bDLEETXOK[];


#define EndMode(pTG)         { pTG->Class1Modem.DriverMode = IDLE;      \
                                                pTG->Class1Modem.ModemMode = COMMAND;        \
                                                pTG->Class1Modem.CurMod = 0; }



// from ddi.c
void SendZeros1(PThrdGlbl pTG, USHORT uCount);

BOOL iModemDrain(PThrdGlbl pTG);


#define iModemNoPauseDialog(pTG, s, l, t, w)                 \
                iiModemDialog(pTG, s, l, t, FALSE, 1, FALSE, (CBPSTR)w, (CBPSTR)(NULL))
#define iModemNoPauseDialog2(pTG, s, l, t, w1, w2)   \
                iiModemDialog(pTG, s, l, t, FALSE, 1, FALSE, (CBPSTR)w1, (CBPSTR)w2, (CBPSTR)(NULL))
#define iModemNoPauseDialog3(pTG, s, l, t, w1, w2, w3)       \
                iiModemDialog(pTG, s, l, t, FALSE, 1, FALSE, (CBPSTR)w1, (CBPSTR)w2, (CBPSTR)w3, (CBPSTR)(NULL))
#define iModemPauseDialog(pTG, s, l, t, w)                   \
                iiModemDialog(pTG, s, l, t, FALSE, 1, TRUE, (CBPSTR)w, (CBPSTR)(NULL))
#define iModemResp2(pTG, t, w1, w2)         \
          iiModemDialog(pTG, NULL, 0, t, FALSE, 1, FALSE, (CBPSTR)w1, (CBPSTR)w2, (CBPSTR)(NULL))
#define iModemResp3(pTG,  t, w1, w2, w3)             \
          iiModemDialog(pTG, NULL, 0, t, FALSE, 1, FALSE, (CBPSTR)w1, (CBPSTR)w2, (CBPSTR)w3, (CBPSTR)(NULL))
#define iModemResp4( pTG, t, w1, w2, w3, w4)         \
          iiModemDialog(pTG, NULL, 0, t, FALSE, 1, FALSE, (CBPSTR)w1, (CBPSTR)w2, (CBPSTR)w3, (CBPSTR)w4, (CBPSTR)(NULL))



