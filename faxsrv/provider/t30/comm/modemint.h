/***************************************************************************
 Name     :     MODEMINT.H
 Comment  :
 Functions:     (see Prototypes just below)

                Copyright (c) Microsoft Corp. 1991, 1992, 1993

 Revision Log
 Date     Name  Description
 -------- ----- ---------------------------------------------------------
***************************************************************************/

/**---------------------- #define of sizes of things ---------------------

        Frames can be at most 2.55 secs (sent) or 3.45 secs (recvd) long, or
        2.55 * 300/8 = 96 bytes and 132 bytes long respectively

        Dialstrings are limited to 512 bytes (arbitrarily)

        Commands (except dial) are never more than about 10-20 bytes long, so
        we use a buffer of 40 bytes. Replies are never big at all, but we
        might hold a frame in there, so keep it same size as a Framebuffer

        The Dial command is ATDT <string><CR>, so we use 512+10 bytes buffer

---------------------- #define of sizes of things ---------------------**/

#define MAXPHONESIZE    512
#define DIALBUFSIZE     MAXPHONESIZE + 10


#define CR                              0x0d
#define LF                              0x0a
#define DLE                             0x10            // DLE = ^P = 16d = 10h
#define ETX                             0x03

// The following bunch of defines allow us to combine detection
// with pre-read settings (from unimodem, say).

#define fGOTCMD_Reset           (0x1)
#define fGOTCMD_Setup           (0x1<<1)
#define fGOTCMD_PreAnswer       (0x1<<2)
#define fGOTCMD_PreDial         (0x1<<3)
#define fGOTCMD_PreExit         (0x1<<4)

#define fGOTCMDS \
          fGOTCMD_Reset \
        | fGOTCMD_Setup \
        | fGOTCMD_PreAnswer \
        | fGOTCMD_PreDial \
        | fGOTCMD_PreExit

#define fGOTCAP_CLASSES         (0x1<<10)
#define fGOTCAP_SENDSPEEDS      (0x1<<11)
#define fGOTCAP_RECVSPEEDS      (0x1<<12)

#define fGOTCAPS \
          fGOTCAP_CLASSES \
        | fGOTCAP_SENDSPEEDS \
        | fGOTCAP_RECVSPEEDS

#define fGOTPARM_PORTSPEED      (0x1<<20)
#define fGOTPARM_IDCMD          (0x1<<21)
#define fGOTPARM_ID                     (0x1<<22)

#define fGOTPARMS \
          fGOTPARM_PORTSPEED \
        | fGOTPARM_IDCMD \
        | fGOTPARM_ID

#define fGOTIDS \
          fGOTPARM_IDCMD \
        | fGOTPARM_ID

#define fGOTFLAGS (0x1<<23)


// Following structure has stuff which should ideally go into
// MODEMCAPS, but we can't change that at this state (11/94).

extern BOOL                             fMegaHertzHack;



// used for Resync type stuff. RepeatCount = 2
// This has to be multi-line too, because echo could be on and
// we could get the command echoed back instead of response!
// Looks like even 330 is too short for some modems..
// 550 is too short for Sharad's PP9600FXMT & things
// can get really screwed up if this times out, so use
// a nice large value
#define  iSyncModemDialog(pTG, s, l, w)                                                      \
                iiModemDialog(pTG, s, l, 990, TRUE, 2, TRUE, (CBPSTR)w, (CBPSTR)(NULL))

// This version for dealing with possible NON-numeric responses as well...
#define  iSyncModemDialog2(pTG, s, l, w1, w2)                                                        \
                iiModemDialog(pTG, s, l, 990, TRUE, 2, TRUE, (CBPSTR)w1, (CBPSTR)w2, (CBPSTR)(NULL))


// These are used for offline things, so we make them all (a) multiline
// (b) long timeout (c) 2 tries and (d) make sure they all look for ERROR
// as a response, to speed things up

#define OfflineDialog2(pTG, s,l,w1,w2)        iiModemDialog(pTG, s, l, 5000, TRUE, 2, TRUE, (CBPSTR)w1, (CBPSTR)w2, (CBPSTR)(NULL))

#define GOCLASS2_0      3
extern CBSZ cbszOK, cbszERROR;




/****************** begin prototypes from modem.c *****************/
SWORD iModemSync(PThrdGlbl pTG);
SWORD iModemReset(PThrdGlbl pTG, CBPSTR szCmd);
UWORD GetCap(PThrdGlbl pTG, CBPSTR cbpstrSend, UWORD uwLen);
UWORD GetCapAux(PThrdGlbl pTG, CBPSTR cbpstrSend, UWORD uwLen);
BOOL iModemGetCaps(PThrdGlbl pTG, LPMODEMCAPS lpMdmCaps,
                                        DWORD dwSpeed, LPSTR lpszReset, LPDWORD lpdwGot);
BOOL iiModemGoClass(PThrdGlbl pTG, USHORT uClass, DWORD dwSpeed);
/***************** end of prototypes from modem.c *****************/


/****************** begin prototypes from identify.c *****************/
USHORT iModemGetCmdTab(PThrdGlbl pTG, LPCMDTAB lpCmdTab, LPMODEMCAPS lpMdmCaps);
USHORT iModemInstall(PThrdGlbl pTG, BOOL fDontPurge);
USHORT iModemFigureOutCmds(PThrdGlbl pTG, LPCMDTAB lpCmdTab);
USHORT iModemGetWriteCaps(PThrdGlbl pTG);
/***************** end of prototypes from identify.c *****************/
