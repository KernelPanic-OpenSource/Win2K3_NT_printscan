
/***************************************************************************
 Name     :     DIS.C
 Comment  :     Collection if DIS/DCS/DTC and CSI/TSI/CIG mangling routines.
                        They manipulate the DIS struct whose members correspond to the bits
                        of the T30 DIS/DCS/DTC frames.

        Copyright (c) 1993 Microsoft Corp.

 Revision Log
 Date     Name  Description
 -------- ----- ---------------------------------------------------------
***************************************************************************/
#define USE_DEBUG_CONTEXT   DEBUG_CONTEXT_T30_MAIN

#include "prep.h"


#include "protocol.h"

///RSL
#include "glbproto.h"

USHORT SetupDISorDCSorDTC
(
    PThrdGlbl pTG, 
    NPDIS npdis, 
    NPBCFAX npbcFax, 
    NPLLPARAMS npll
)
{
    // return length of DIS

    USHORT  uLen;

    DEBUG_FUNCTION_NAME(_T("SetupDISorDCSorDTC"));

    DebugPrintEx(   DEBUG_MSG,
                    "baud=0x%02x min=0x%02x res=0x%02x code=0x%02x"
                    " wide=0x%02x len=0x%02x",
                    npll->Baud, npll->MinScan, 
                    (WORD)npbcFax->AwRes, npbcFax->Encoding, 
                    npbcFax->PageWidth, npbcFax->PageLength);

    _fmemset(npdis, 0, sizeof(DIS));

    // npdis->G1stuff  = 0;
    // npdis->G2stuff = 0;
    npdis->G3Rx = 1;  // always ON for DIS & DCS. Indicates T.4 recv Cap/Mode
    npdis->G3Tx = (BYTE) (npbcFax->fPublicPoll);
    // This must be 0 for a DCS frame. The Omnifax G77 and GT choke on it!

    npdis->Baud = npll->Baud;

    npdis->MR_2D    = ((npbcFax->Encoding & MR_DATA) != 0);

    npdis->MMR      = ((npbcFax->Encoding & MMR_DATA) != 0);
    // npdis->MR_2D                 = 0;
    // npdis->MMR                   = 0;

    npdis->PageWidth                = (BYTE) (npbcFax->PageWidth);
    npdis->PageLength               = (BYTE) (npbcFax->PageLength);
    npdis->MinScanCode      = npll->MinScan;

    // npdis->Uncompressed = npdis->ELM = 0;

	// we do not support ECM
    npdis->ECM = 0;
    npdis->SmallFrame = 0;


    if (npbcFax->PageWidth > WIDTH_MAX)
    {
            npdis->WidthInvalid = TRUE;
            npdis->Width2 = (npbcFax->PageWidth>>WIDTH_SHIFT);
    }

    // doesn't hold for SendParams (why??)
    npdis->Res_300            = 0;  // RSL ((npbcFax->AwRes & AWRES_300_300) != 0);
    npdis->Res8x15            = ((npbcFax->AwRes & AWRES_mm080_154) != 0);


    if (! pTG->SrcHiRes) 
    {
        npdis->ResFine_200 = 0;
    }
    else 
    {
        npdis->ResFine_200      =  ((npbcFax->AwRes & (AWRES_mm080_077|AWRES_200_200)) != 0);
    }

    npdis->Res16x15_400     = ((npbcFax->AwRes & (AWRES_mm160_154|AWRES_400_400)) != 0);
    npdis->ResInchBased = ((npbcFax->AwRes & (AWRES_200_200|AWRES_400_400)) != 0);
    npdis->ResMetricBased = ((npbcFax->AwRes & AWRES_mm160_154) ||
                                                    ((npbcFax->AwRes & AWRES_mm080_077) && npdis->ResInchBased));

    npdis->MinScanSuperHalf = ((npll->MinScan & MINSCAN_SUPER_HALF) != 0);

    npdis->Extend24 = npdis->Extend32 = npdis->Extend40 = 0;
    uLen = 3;

    DebugPrintEx(DEBUG_MSG,"DIS len = %d", uLen);
    return uLen;
}


void ParseDISorDCSorDTC
(
    PThrdGlbl pTG, 
    NPDIS npDIS, 
    NPBCFAX npbcFax, 
    NPLLPARAMS npll, 
    BOOL fParams
)
{
///////////////////////////////////////////////////////////////
// Prepare to get trash (upto 2 bytes) at end of every frame //
///////////////////////////////////////////////////////////////

    LPB npb, npbLim;

    DEBUG_FUNCTION_NAME(_T("ParseDISorDCSorDTC"));
// first make sure DIS is clean. We may have picked up some trailing CRCs
// trailing-trash removed by reading the Extend bits

    npb = npbLim = (LPB)npDIS;
    npbLim += sizeof(DIS);
    for(npb+=2; npb<npbLim && (*npb & 0x80); npb++);
    // exits when npb points past end of structure or
    // points to the first byte with the high bit NOT set
    // i.e. the last VALID byte

    for(npb++; npb<npbLim; npb++)
        *npb = 0;
        // starting with the byte AFTER the last valid byte, until end
        // of the structure, zap all bytes to zero

// parse high level params into NPI

    memset(npbcFax, 0, sizeof(BCFAX));

    npbcFax->AwRes = 0;
    npbcFax->Encoding = 0;

    // Resolution
    if(npDIS->Res8x15)
        npbcFax->AwRes |= AWRES_mm080_154;

    if(npDIS->Res_300)
        npbcFax->AwRes |= AWRES_300_300;

    if(npDIS->ResInchBased)
    {
        if(npDIS->ResFine_200)
            npbcFax->AwRes |= AWRES_200_200;

        if(npDIS->Res16x15_400)
            npbcFax->AwRes |= AWRES_400_400;
    }
    if(npDIS->ResMetricBased || !npDIS->ResInchBased)
    {
        if(npDIS->ResFine_200)
            npbcFax->AwRes |= AWRES_mm080_077;

        if(npDIS->Res16x15_400)
            npbcFax->AwRes |= AWRES_mm160_154;
    }

    // Encoding (MMR only if ECM also supported)
    if(npDIS->MR_2D)
        npbcFax->Encoding |= MR_DATA;

    if(npDIS->MMR && npDIS->ECM)
        npbcFax->Encoding |= MMR_DATA;

    if(!fParams)
    {
        // setting up capabilities -- add the "always present" caps
        npbcFax->AwRes |= AWRES_mm080_038;
        npbcFax->Encoding |= MH_DATA;
    }
    else
    {
        // setting up params -- set the defaults if none otehr specified
        if(!npbcFax->AwRes)
            npbcFax->AwRes = AWRES_mm080_038;

        if(!npbcFax->Encoding)
            npbcFax->Encoding = MH_DATA;

        // if both MR & MMR are set (this happens with Ricoh's fax simulator!)
        // then set just MMR. MH doesnt have an explicit bit, so we set MH
        // here only if nothing else is set. So the only multiple-setting case
        // (for encodings) that we can encounter is (MR|MMR). BUG#6950
        if(npbcFax->Encoding == (MR_DATA|MMR_DATA))
                npbcFax->Encoding = MMR_DATA;
    }

    // PageWidth and Length
    npbcFax->PageWidth      = npDIS->PageWidth;

    // IFAX Bug#8152: Hack for interpreting invalid value (1,1) as
    // A3, because some fax machines do that. This is
    // as per Note 7 of Table 2/T.30 of ITU-T.30 (1992, page 40).
#define WIDTH_ILLEGAL_A3 0x3
    if (!fParams && npbcFax->PageWidth==WIDTH_ILLEGAL_A3)
    {
        npbcFax->PageWidth=WIDTH_A3;
    }
    npbcFax->PageLength     = npDIS->PageLength;

    // has G3 file available for polling
    npbcFax->fPublicPoll = npDIS->G3Tx;


// Now low level params LLPARAMS
// Baudrate and MinScan. That's it!

    npll->Baud = npDIS->Baud;
    npll->MinScan = npDIS->MinScanCode;
    if(npDIS->MinScanSuperHalf)
            npll->MinScan |= MINSCAN_SUPER_HALF;


    DebugPrintEx(   DEBUG_MSG,
                        "baud=0x%02x min=0x%02x res=0x%02x code=0x%02x"
                        " wide=0x%02x len=0x%02x",
                        npll->Baud, npll->MinScan, 
                        (WORD)npbcFax->AwRes, npbcFax->Encoding, 
                        npbcFax->PageWidth, npbcFax->PageLength);
}


/* Converts the code for a speed to the speed in BPS */

UWORD CodeToBPS[16] =
{
/* V27_2400     0 */    2400,
/* V29_9600     1 */    9600,
/* V27_4800     2 */    4800,
/* V29_7200     3 */    7200,
/* V33_14400    4 */    14400,
                        0,
/* V33_12000    6 */    12000,
                        0,
/* V17_14400    8 */    14400,
/* V17_9600     9 */    9600,
/* V17_12000    10 */   12000,
/* V17_7200     11 */   7200,
                        0,
                        0,
                        0,
                        0
};


#define msBAD   255

/* Converts a DCS min-scan field code into millisecs */
BYTE msPerLine[8] = { 20, 5, 10, msBAD, 40, msBAD, msBAD, 0 };


USHORT MinScanToBytesPerLine(PThrdGlbl pTG, BYTE MinScan, BYTE Baud)
{
    USHORT uStuff;
    BYTE ms;

    uStuff = CodeToBPS[Baud];
    ms = msPerLine[MinScan];
    uStuff /= 100;          // StuffBytes = (BPS * ms)/8000
    uStuff *= ms;           // take care not to use longs
    uStuff /= 80;           // or overflow WORD or lose precision
    uStuff += 1;            // Rough fix for truncation problems

    return uStuff;
}

#define ms40    4
#define ms20    0
#define ms10    2
#define ms5     1
#define ms0     7

/* first index is a DIS min-scan capability. 2nd is 0 for normal
 * 1 for fine (1/2) and 2 for super-fine (if 1/2 yet again).
 * Output is the Code to stick in the DCS.
 */

BYTE MinScanTab[8][3] =
{
        ms20,   ms20,   ms10,
        ms5,    ms5,    ms5,
        ms10,   ms10,   ms5,
        ms20,   ms10,   ms5,
        ms40,   ms40,   ms20,
        ms40,   ms20,   ms10,
        ms10,   ms5,    ms5,
        ms0,    ms0,    ms0
};


#define V_ILLEGAL       255

#define V27_2400        0
#define V27_4800        2
#define V29_9600        1
#define V29_7200        3
#define V33_14400       4
#define V33_12000       6
#define V17_14400       8
#define V17_12000       10
#define V17_9600        9
#define V17_7200        11

#define V27_SLOW                        0
#define V27_ONLY                        2
#define V29_ONLY                        1
#define V33_ONLY                        4
#define V17_ONLY                        8
#define V27_V29                         3
#define V27_V29_V33                     7
#define V27_V29_V33_V17         11
#define V_ALL                           15

/* Converts a capability into the best speed it offers.
 * index will usually be the & of both DIS's Baud rate fields
 * (both having first been "adjusted", i.e. 11 changed to 15)
 * Output is the Code to stick in the DCS.
 */

BYTE BaudNegTab[16] =
{
/* V27_SLOW                     0 --> 0 */      V27_2400,
/* V29_ONLY                     1 --> 1 */      V29_9600,
/* V27_ONLY                     2 --> 2 */      V27_4800,
/* V27_V29                      3 --> 1 */      V29_9600,
                                                V_ILLEGAL,
                                                V_ILLEGAL,
                                                V_ILLEGAL,
/* V27_V29_V33                  7 --> 4 */      V33_14400,
                                                V_ILLEGAL,
                                                V_ILLEGAL,
                                                V_ILLEGAL,
/* V27_V29_V33_V17              11 --> 8 */     V17_14400,
                                                V_ILLEGAL,
                                                V_ILLEGAL,
                                                V_ILLEGAL,
/* V_ALL                        15 --> 8 */     V17_14400
};

/***************************************************************************
        Name      :     NegotiateLowLevelParams
        Purpose   :     Takes a received DIS and optionally MS NSF,
                                our HW caps and

                                picks highest common baud rate, ECM is picked if both have it
                                ECM frame size is 256 unless remote DIS has that bit set to 1
                                or we want small frames (compiled-in option).

                                The MinScan time is set to the max.
                                                        (i.e. highest/slowest) of both.

                                Fill results in Negot section of npProt

        CalledFrom:     NegotiateLowLevelParams is called by the sender when a DIS
                                and/or NSF is received.
***************************************************************************/


void NegotiateLowLevelParams
(
    PThrdGlbl pTG, 
    NPLLPARAMS npllRecv, 
    NPLLPARAMS npllSend,
    DWORD AwRes, 
    USHORT uEnc, 
    NPLLPARAMS npllNegot
)
{
    USHORT  Baud, Baud1, Baud2;
    USHORT  MinScanCode, col;

    DEBUG_FUNCTION_NAME(_T("NegotiateLowLevelParams"));
    ////// negotiate Baudrate, ECM, ECM frame size, and MinScan. That's it!

    Baud1 = npllRecv->Baud;
    Baud2 = npllSend->Baud;
    if(Baud1 == 11) 
        Baud1=15;

    if(Baud2 == 11)
        Baud2=15;

    Baud = Baud1 & Baud2;
    npllNegot->Baud = BaudNegTab[Baud];
    if (npllNegot->Baud == V_ILLEGAL)
    {
        // this is a case in which the remote side sent us an invalid param
        // as the input supported baud rate and modulation.
        // since we haven't sent out the DCS yet, we'll try to go with V.29 9600
        // and send this DCS, if the remote side does not support this
        // let it disconnect, anyways it violated the protocol.
        npllNegot->Baud = V29_9600;
        DebugPrintEx(DEBUG_ERR,"Remote side violates protocol (%d), default to V.29 9600",npllRecv->Baud);
    }
    // there is always some common baud rate (i.e. at least 2400 mandatory)


    /* Minimum Scan line times. Only Receiver's pref matters.
     * Use teh table above to convert from Receiver's DIS to the
     * required DCS. Col 1 is used if vertical res. is normal (100dpi)
     * Col2 if VR is 200 or 300dpi, (OR if VR is 400dpi, but Bit 46
     * is not set), and Col3 is used if VR is 400dpi *and* Bit 46
     * (MinScanSuperHalf) is set
     */

    {
        MinScanCode = (npllRecv->MinScan & 0x07);       // low 3 bits

        if(AwRes & (AWRES_mm080_154|AWRES_mm160_154|AWRES_400_400))
        {
            if(npllRecv->MinScan & MINSCAN_SUPER_HALF)
                    col = 2;
            else
                    col = 1;
        }
        // T30 says scan time for 300dpi & 200dpi is the same
        else if(AwRes & (AWRES_300_300|AWRES_mm080_077|AWRES_200_200))
            col = 1;
        else
            col = 0;

        npllNegot->MinScan = MinScanTab[MinScanCode][col];
    }

    DebugPrintEx(   DEBUG_MSG,
                    "baud=0x%02x min=0x%02x",
                    npllNegot->Baud, npllNegot->MinScan);
}


USHORT GetReversedFIFs
(
	IN PThrdGlbl pTG, 
	IN LPCSTR lpstrSource, 
	OUT LPSTR lpstrDest, 
	IN UINT cch
)
{
    /** Both args always 20 bytes long. Throws away leading & trailing
            blanks, then copies what's left *reversed* into lpstr[].
            Terminates with 0.
    **/

    int i, j, k;

    DEBUG_FUNCTION_NAME(_T("GetReversedFIFs"));

	if (strlen(lpstrSource)>=cch)
	{
		DebugPrintEx(DEBUG_ERR,"Output buffer is too short to create reverse FIF");
		return 0;
	}
	
	if (strlen(lpstrSource)>IDFIFSIZE)
	{
		DebugPrintEx(DEBUG_ERR,"Source buffer is too long to create reverse FIF");
		return 0;
	}

    for(k=0; k<IDFIFSIZE && lpstrSource[k]==' '; k++);    // k==first nonblank or 20
            
    for(j=IDFIFSIZE-1; j>=k && lpstrSource[j]==' '; j--); // j==last nonblank or -1
                
    i = 0;

    for( ; i<IDFIFSIZE && j>=k; i++, j--)
            lpstrDest[i] = lpstrSource[j];

    lpstrDest[i] = 0;

    DebugPrintEx(   DEBUG_MSG,
                    "Got<%s> produced<%s>",
                    (LPSTR)lpstrSource, (LPSTR)lpstrDest);

    return (USHORT)i;
}

void CreateStupidReversedFIFs(PThrdGlbl pTG, LPSTR lpstr1, LPSTR lpstr2)
{
    /** Both args always 20 bytes long. Copies LPSTR *reversed* into
            the end of lpstr1[], then pads rest with blank.
            Terminates with a 0
    **/

    int i, j;

    for(i=0, j=IDFIFSIZE-1; lpstr2[i] && j>=0; i++, j--)
            lpstr1[j] = lpstr2[i];

    if(j>=0)
        _fmemset(lpstr1, ' ', j+1);

    lpstr1[IDFIFSIZE] = 0;

}


/* Converts a the code for a speed to the code fro the next
 * best (lower) speed. order is
   (V17: 144-120-96-72-V27_2400) (V33: 144 120 V29: 96 72 V27: 48 24)
 */
// NOTE: FRANCE defines the fallback sequence to go from V17_7200 to V27_4800




BYTE DropBaudTab[16] =
{
/* V27_2400     --> X           0 --> X */      V_ILLEGAL,
/* V29_9600     --> V29_7200    1 --> 3 */      V29_7200,
/* V27_4800     --> V27_2400    2 --> 0 */      V27_2400,
/* V29_7200     --> V27_4800    3 --> 2 */      V27_4800,
/* V33_14400 -> V33_12000       4 --> 6 */      V33_12000,
                                                V_ILLEGAL,
/* V33_12000 -> V29_9600        6 --> 1 */      V29_9600,
                                                V_ILLEGAL,
/* V17_14400 -> V17_12000       8 -> 10 */      V17_12000,
/* V17_9600 --> V17_7200        9 -> 11 */      V17_7200,
/* V17_12000 -> V17_9600        10 -> 9 */      V17_9600,
/* V17_7200 --> V29_9600        11 -> 1
                         or V29_7200    11 -> 3
USE THIS---> or V27_4800        11 -> 2
                         or V27_2400    11 -> 0 */      V27_4800,
                                                        V_ILLEGAL,
                                                        V_ILLEGAL,
                                                        V_ILLEGAL,
                                                        V_ILLEGAL
};

BOOL DropSendSpeed(PThrdGlbl pTG)
{
    USHORT  uSpeed;
    UWORD   uBps;

    DEBUG_FUNCTION_NAME(_T("DropSendSpeed"));
    if ((pTG->ProtInst.llNegot.Baud==V_ILLEGAL) || (pTG->ProtInst.llNegot.Baud>15))
    {
        DebugPrintEx(   DEBUG_ERR,
                        "Illegal input speed to DropSendSpeed %d",
                        pTG->ProtInst.llNegot.Baud);
        return FALSE;
    }

    uSpeed = DropBaudTab[pTG->ProtInst.llNegot.Baud];
    uBps = CodeToBPS[uSpeed];
    // enforce LowestSendSpeed
    if  (   (uSpeed == V_ILLEGAL) ||
            (   (pTG->ProtInst.LowestSendSpeed <= 14400) &&
                (uBps < pTG->ProtInst.LowestSendSpeed)
            ) 
        )
    {
        DebugPrintEx(   DEBUG_MSG,
                        "Can't drop (0x%02x)",
                        pTG->ProtInst.llNegot.Baud);
        return FALSE;
        // speed remains same as before if lowest speed
        // return FALSE to hangup
    }
    else
    {
        DebugPrintEx(DEBUG_MSG,"Now at 0x%02x", uSpeed);
        pTG->ProtInst.llNegot.Baud = (BYTE) uSpeed;
        return TRUE;
    }
}

void EnforceMaxSpeed(PThrdGlbl pTG)
{
    DEBUG_FUNCTION_NAME(_T("EnforceMaxSpeed"));

    // enforce HighestSendSpeed setting
    if( pTG->ProtInst.HighestSendSpeed && pTG->ProtInst.HighestSendSpeed >= 2400 &&
            pTG->ProtInst.HighestSendSpeed >= pTG->ProtInst.LowestSendSpeed)
    {
        DebugPrintEx(   DEBUG_MSG,
                        "MaxSend=%d. Baud=%x BPS=%d, dropping",
                        pTG->ProtInst.HighestSendSpeed, 
                        pTG->ProtInst.llNegot.Baud, 
                        CodeToBPS[pTG->ProtInst.llNegot.Baud]);

        while(CodeToBPS[pTG->ProtInst.llNegot.Baud] > pTG->ProtInst.HighestSendSpeed)
        {
            if(!DropSendSpeed(pTG))
            {
                break;
            }
        }
        DebugPrintEx(   DEBUG_MSG,
                        "MaxSend=%d. Baud=%x BPS=%d",
                        pTG->ProtInst.HighestSendSpeed, 
                        pTG->ProtInst.llNegot.Baud, 
                        CodeToBPS[pTG->ProtInst.llNegot.Baud]);
    }
}

USHORT CopyFrame(PThrdGlbl pTG, LPBYTE lpbDst, LPFR lpfr, USHORT uSize)
{
///////////////////////////////////////////////////////////////
// Prepare to get trash (upto 2 bytes) at end of every frame //
///////////////////////////////////////////////////////////////
    USHORT uDstLen;

    uDstLen = min(uSize, lpfr->cb);
    _fmemset(lpbDst, 0, uSize);
    _fmemcpy(lpbDst, lpfr->fif, uDstLen);
    return uDstLen;
}

void CopyRevIDFrame
(
	IN PThrdGlbl pTG, 
	OUT LPBYTE lpbDst, 
	IN LPFR lpfr,
	IN UINT cb
)
{
///////////////////////////////////////////////////////////////
// Prepare to get trash (upto 2 bytes) at end of every frame //
///////////////////////////////////////////////////////////////

    USHORT  uDstLen;
    char    szTemp[IDFIFSIZE+2];

    DEBUG_FUNCTION_NAME(_T("CopyRevIDFrame"));

    uDstLen = min(IDFIFSIZE, lpfr->cb);
    _fmemset(szTemp, ' ', IDFIFSIZE);       // fill spaces (reqd by GetReverse)
    _fmemcpy(szTemp, lpfr->fif, uDstLen);
    szTemp[IDFIFSIZE] = 0;  // zero-terminate

    GetReversedFIFs(pTG, szTemp, lpbDst, cb);

    if(uDstLen!=IDFIFSIZE)
        DebugPrintEx(DEBUG_ERR, "Bad ID frame" );
}

// This function check whether the parameters in recvdDCS are valids by checking the 
// DCS we got given the DIS we send to transmitter
BOOL AreDCSParametersOKforDIS(LPDIS sendDIS, LPDIS recvdDCS)
{
    // FYI: The DCS is save in pTG->ProtInst->RemoteDCS
    // While we save our DIS in pTG->ProtInst->LocalDIS
    // This will solve bug #4677: "Fax: T.30: service does not receive simple 1 page fax, using ECM"
    if (sendDIS->ECM != recvdDCS->ECM)
    {
        return FALSE;
    }
    switch(sendDIS->PageWidth)
    {
        case 0: // This means: ONLY 1728 dots
                if (recvdDCS->PageWidth != 0) 
                {
                    return FALSE;
                }
                break;
        case 1: // This means: 1728 or 2048
                if ((recvdDCS->PageWidth != 0) && (recvdDCS->PageWidth != 1))
                {
                    return FALSE;
                }
                break;
                

        case 2: // This means: 1728 or 2048 or 2432
        case 3: // This is wrong but we interpret it like 2: All the standard widths
                break;

        default:// There is no other option (PageWidth is two only bits)
                return FALSE;
    }
    return TRUE;    
}





