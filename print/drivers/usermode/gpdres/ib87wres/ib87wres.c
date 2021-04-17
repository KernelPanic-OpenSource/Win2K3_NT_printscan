
/*++

Copyright (c) 1996-1999  Microsoft Corporation

Module Name:

    ib587res.c

Abstract:

    Implementation of GPD command callback for "test.gpd":
        OEMCommandCallback

Environment:

    Windows NT Unidrv driver

--*/

#include "pdev.h"

#include <windows.h>
#include <stdio.h>

#include <strsafe.h>

HANDLE	RevertToPrinterSelf( VOID );
BOOL	ImpersonatePrinterClient( HANDLE );

/*--------------------------------------------------------------------------*/
/*                             G L O B A L  V A L U E                         */
/*--------------------------------------------------------------------------*/

//Command strings

//
const BYTE CMD_BEGIN_DOC_1[] = {0x1B,0x7E,0xB0,0x00,0x12,0x01} ;
const BYTE CMD_BEGIN_DOC_2[] = {0x01,0x01,0x00} ;
const BYTE CMD_BEGIN_DOC_3[] = {0x02,0x02,0xFF,0xFF} ;
const BYTE CMD_BEGIN_DOC_4[] = {0x03,0x02,0xFF,0xFF} ;
const BYTE CMD_BEGIN_DOC_5[] = {0x04,0x04,0xFF,0xFF,0xFF,0xFF} ;
// ISSUE-2002/3/18-takashim - Not sure why CMD_BEGIN_PAGE[] was defined as 2 bytes.
//const BYTE CMD_BEGIN_PAGE[]    = {0xD4, 0x00} ;
const BYTE CMD_BEGIN_PAGE[]    = {0xD4} ;
const BYTE CMD_END_JOB[] = {0x1B,0x7E,0xB0,0x00,0x04,0x01,0x01,0x01,0x01} ;
const BYTE CMD_END_PAGE[] = {0x20};

//SetPac
#define CMD_SETPAC pOEM->SetPac

#define CMD_SETPAC_FRONT_TRAY_PAPER_SIZE    4
#define CMD_SETPAC_INPUT_BIN                5
#define CMD_SETPAC_RESOLUTION               12
#define CMD_SETPAC_1ST_CASSETTE_PAPER_SIZE  14
#define CMD_SETPAC_2ND_CASSETTE_PAPER_SIZE  15
#define CMD_SETPAC_PAGE_LENGTH              19 // 4 bytes
#define CMD_SETPAC_TONER_SAVE_MODE          25
#define CMD_SETPAC_CUSTOM_DOTS_PER_LINE     26 // 2 bytes
#define CMD_SETPAC_CUSTOM_LINES_PER_PAGE    28 // 2 bytes

const BYTE CMD_SETPAC_TMPL[
        CMD_SETPAC_SIZE]    ={ 0xD7,
                       0x01,
                       0xD0,
                       0x1D, //CommandLength
                       0x00, //Front tray paper size
                       0x00, //Input-bin
                       0x01,
                       0x04, //Bit-assign1
                       0xD9, //Bit-assign2
                       0x04, //EET
                       0x02, //PrintDensity
                       0x01, 
                       0x00, //Resolution
                       0x01,
                       0x00, //1st cassette paper size
                       0x00, //2nd cassette paper size
                       0x0F, //Time to Power Save
                       0x00, 
                       0x01, //Compression Mode
                       0x00,0x00,0x00,0x00, //PageLength
                       0x07, 
                       0x00, //Number of Copies
                       0x00, //Toner save mode
                       0x00,0x00, //Dot per line for custom size
                       0x00,0x00, //Data lines per page for custom size
                       0x00} ;

const BYTE Mask[]={0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01} ;

const POINTL phySize300[] = {
//    Width,Height Physical paper size for 300dpi
    {3416,4872},    //A3
    {2392,3416},    //A4
    {1672,2392},    //A5
    {2944,4208},    //B4
    {2056,2944},    //B5
    {1088,1656},    //PostCard
    {2456,3208},    //Letter
    {2456,4112},    //Legal
    {0000,0000},    //user define
};
const POINTL phySize600[] = {
//    Width,Height   Physical paper size for 600dpi
    {6832,9736},    //A3
    {4776,6832},    //A4
    {3336,4776},    //A5
    {5888,8416},    //B4
    {4112,5888},    //B5
    {2176,3312},    //PostCard
    {4912,6416},    //Letter
    {4912,8216},    //Legal
    {0000,0000},    //user define
};


/******************* FUNCTIONS *************************/
BOOL MyDeleteFile(PDEVOBJ pdevobj, LPSB lpsb) ;
BOOL InitSpoolBuffer(LPSB lpsb) ;
BOOL MyCreateFile(PDEVOBJ pdevobj, LPSB lpsb) ;
BOOL MySpool(PDEVOBJ pdevobj, LPSB lpsb, PBYTE pBuf, DWORD dwLen) ;
BOOL SpoolOut(PDEVOBJ pdevobj, LPSB lpsb) ;
BOOL MyEndDoc(PDEVOBJ pdevobj) ;
BOOL WriteFileForP_Paper(PDEVOBJ pdevobj, PBYTE pBuf, DWORD dwLen) ;
BOOL WriteFileForL_Paper(PDEVOBJ pdevobj, PBYTE pBuf, DWORD dwLen) ;
BOOL FillPageRestData(PDEVOBJ pdevobj) ;
BOOL SpoolWhiteData(PDEVOBJ pdevobj, DWORD dwWhiteLen, BOOL fComp) ;
BOOL SendPageData(PDEVOBJ pdevobj, PBYTE pSrcImage, DWORD dwLen) ;
BOOL SpoolOutChangedData(PDEVOBJ pdevobj, LPSB lpsb) ;
WORD GetPrintableArea(WORD physSize, INT iRes) ;
BOOL AllocTempBuffer(PIBMPDEV pOEM, DWORD dwNewBufLen) ;
BOOL MyEndPage(PDEVOBJ pdevobj) ;
BOOL MyStartDoc(PDEVOBJ pdevobj) ;

BOOL SpoolOutCompStart(PSOCOMP pSoc);
BOOL SpoolOutCompEnd(PSOCOMP pSoc, PDEVOBJ pdevobj, LPSB psb);
BOOL SpoolOutComp(PSOCOMP pSoc, PDEVOBJ pdevobj, LPSB psb,
    PBYTE pjBuf, DWORD dwLen);

/*****************************************************************************/
/*                                                                             */
/*    Module:         IB587RES.DLL                                              *
/*                                                                             */
/*    Function:        OEMEnablePDEV                                             */
/*                                                                             */
/*    Syntax:         PDEVOEM APIENTRY OEMEnablePDEV(                          */
/*                                        PDEVOBJ         pdevobj,             */
/*                                        PWSTR            pPrinterName,         */
/*                                        ULONG            cPatterns,             */
/*                                        HSURF           *phsurfPatterns,      */
/*                                        ULONG            cjGdiInfo,             */
/*                                        GDIINFO        *pGdiInfo,             */
/*                                        ULONG            cjDevInfo,             */
/*                                        DEVINFO        *pDevInfo,             */
/*                                        DRVENABLEDATA  *pded)                 */
/*                                                                             */
/*    Description:    Allocate buffer of private data to pdevobj                 */
/*                                                                             */
/*****************************************************************************/
PDEVOEM APIENTRY
OEMEnablePDEV(
    PDEVOBJ         pdevobj,
    PWSTR            pPrinterName,
    ULONG            cPatterns,
    HSURF           *phsurfPatterns,
    ULONG            cjGdiInfo,
    GDIINFO        *pGdiInfo,
    ULONG            cjDevInfo,
    DEVINFO        *pDevInfo,
    DRVENABLEDATA  *pded)
{
    PIBMPDEV    pOEM;

    if (!VALID_PDEVOBJ(pdevobj))
    {
        return NULL;
    }

    if(!pdevobj->pdevOEM)
    {
        if(!(pdevobj->pdevOEM = MemAlloc(sizeof(IBMPDEV))))
        {
            //DBGPRINT(DBG_WARNING, (ERRORTEXT("OEMEnablePDEV:Memory alloc failed.\n")));
            return NULL;
        }
    }

    pOEM = (PIBMPDEV)(pdevobj->pdevOEM);

    // Setup pdev specific conrol block fields
    ZeroMemory(pOEM, sizeof(IBMPDEV));
    CopyMemory(pOEM->SetPac, CMD_SETPAC_TMPL, sizeof(CMD_SETPAC_TMPL));

    return pdevobj->pdevOEM;
}


/*****************************************************************************/
/*                                                                             */
/*    Module:         IB587RES.DLL                                              */
/*                                                                             */
/*    Function:        OEMDisablePDEV                                             */
/*                                                                             */
/*    Description:    Free buffer of private data                              */
/*                                                                             */
/*****************************************************************************/
VOID APIENTRY
OEMDisablePDEV(
    PDEVOBJ     pdevobj)
{
    if (!VALID_PDEVOBJ(pdevobj))
    {
        return;
    }

    if(pdevobj->pdevOEM)
    {
        if (((PIBMPDEV)(pdevobj->pdevOEM))->pTempImage) {
            MemFree(((PIBMPDEV)(pdevobj->pdevOEM))->pTempImage);
            ((PIBMPDEV)(pdevobj->pdevOEM))->pTempImage = 0;
        }

        MemFree(pdevobj->pdevOEM);
        pdevobj->pdevOEM = NULL;
    }
    return;
}

BOOL APIENTRY OEMResetPDEV(
    PDEVOBJ pdevobjOld,
    PDEVOBJ pdevobjNew)
{
    PIBMPDEV pOEMOld, pOEMNew;
    PBYTE pTemp;
    DWORD dwTemp;

    if (!VALID_PDEVOBJ(pdevobjOld)
            || !VALID_PDEVOBJ(pdevobjNew))
    {
        return FALSE;
    }

    pOEMOld = (PIBMPDEV)pdevobjOld->pdevOEM;
    pOEMNew = (PIBMPDEV)pdevobjNew->pdevOEM;

    if (pOEMOld != NULL && pOEMNew != NULL) {

        // Save pointer and length
        pTemp = pOEMNew->pTempImage;
        dwTemp = pOEMNew->dwTempBufLen;

        *pOEMNew = *pOEMOld;

        // Restore..
        pOEMNew->pTempImage = pTemp;
        pOEMNew->dwTempBufLen = dwTemp;
    }

    return TRUE;
}

/*****************************************************************************/
/*                                                                             */
/*    Module:    OEMFilterGraphics                                             */
/*                                                                             */
/*    Function:                                                                 */
/*                                                                             */
/*    Syntax:    BOOL APIENTRY OEMFilterGraphics(PDEVOBJ, PBYTE, DWORD)         */
/*                                                                             */
/*    Input:       pdevobj       address of PDEVICE structure                      */
/*               pBuf        points to buffer of graphics data                 */
/*               dwLen       length of buffer in bytes                         */
/*                                                                             */
/*    Output:    BOOL                                                          */
/*                                                                             */
/*    Notice:    nFunction and Escape numbers are the same                     */
/*                                                                             */
/*****************************************************************************/
BOOL
APIENTRY
OEMFilterGraphics(
    PDEVOBJ pdevobj,
    PBYTE pBuf,
    DWORD dwLen)
{
    PIBMPDEV    pOEM;
    BOOL bRet;

    if (!VALID_PDEVOBJ(pdevobj))
    {
        return FALSE;
    }

    pOEM = (PIBMPDEV)pdevobj->pdevOEM;

    bRet = TRUE;

    if(pOEM->fChangeDirection) {
        bRet = WriteFileForL_Paper(pdevobj, pBuf, dwLen);

    }else{
        bRet = WriteFileForP_Paper(pdevobj, pBuf, dwLen);
    }

    return bRet;
}

/*****************************************************************************/
/*                                                                             */
/*    Module:    OEMCommandCallback                                             */
/*                                                                             */
/*    Function:                                                                 */
/*                                                                             */
/*    Syntax:    INT APIENTRY OEMCommandCallback(PDEVOBJ,DWORD,DWORD,PDWORD)     */
/*                                                                             */
/*    Input:       pdevobj                                                         */
/*               dwCmdCbID                                                     */
/*               dwCount                                                         */
/*               pdwParams                                                     */
/*                                                                             */
/*    Output:    INT                                                             */
/*                                                                             */
/*    Notice:                                                                  */
/*                                                                             */
/*****************************************************************************/
INT APIENTRY
OEMCommandCallback(
    PDEVOBJ pdevobj,    // Points to private data required by the Unidriver.dll
    DWORD    dwCmdCbID,    // Callback ID
    DWORD    dwCount,    // Counts of command parameter
    PDWORD    pdwParams ) // points to values of command params
{
    PIBMPDEV       pOEM;
    WORD            wPhysWidth;
    WORD            wPhysHeight;
    WORD            wDataLen ;
    WORD            wLines ;
    WORD            wNumOfByte ;
    POINTL            ptlUserDefSize;

    BYTE            byOutput[64];
    DWORD            dwNeeded;
    DWORD            dwOptionsReturned;
    INT iRet;

    if (!VALID_PDEVOBJ(pdevobj))
    {
        return FALSE;
    }

    pOEM = (PIBMPDEV)pdevobj->pdevOEM;

    iRet = 0;

    switch(dwCmdCbID)
    {

        case PAGECONTROL_BEGIN_DOC:
            if (!InitSpoolBuffer(&(pOEM->sb)))
                goto fail;

            if (!MyCreateFile(pdevobj, &(pOEM->sb)))
                goto fail;
            if (!MyCreateFile(pdevobj, &(pOEM->sbcomp)))
                goto fail;

            pOEM->fChangeDirection = FALSE ;
            pOEM->sPageNum = 0 ;

            if (!MyStartDoc(pdevobj))
                goto fail;

            break;

        case PAGECONTROL_BEGIN_PAGE:
            pOEM->dwCurCursorY = 0 ;
            pOEM->dwOffset = 0 ;
            pOEM->sPageNum ++ ;

            if(pOEM->fChangeDirection == FALSE) {
                if (!MySpool(pdevobj, &(pOEM->sb),
                            (PBYTE)CMD_BEGIN_PAGE, sizeof(CMD_BEGIN_PAGE)))
                    goto fail;
                if (!SpoolOutCompStart(&pOEM->Soc))
                    goto fail;
            }

            break;

        case PAGECONTROL_END_PAGE:
            if(pOEM->fChangeDirection == FALSE){
                if (!FillPageRestData(pdevobj))
                    goto fail;
                if (!SpoolOutCompEnd(&pOEM->Soc, pdevobj, &pOEM->sb))
                    goto fail;
                if (!MySpool(pdevobj,&(pOEM->sb),
                    (PBYTE)CMD_END_PAGE, sizeof(CMD_END_PAGE)))
                    goto fail;
            }else{
               if (!SpoolOutChangedData(pdevobj, &(pOEM->sbcomp)))
                   goto fail;
            }

            if (!MyEndPage(pdevobj))
                goto fail;
            
            break;

        case PAGECONTROL_ABORT_DOC:
        case PAGECONTROL_END_DOC:
                       
            if (!MyEndDoc(pdevobj))
                goto fail;
            break;

        case RESOLUTION_300:
            pOEM->ulHorzRes = 300;
            pOEM->ulVertRes = 300;

            CMD_SETPAC[CMD_SETPAC_RESOLUTION] = 0x02 ;
            
            if (pOEM->sPaperSize < PHYS_PAPER_BASE
                    || pOEM->sPaperSize > PHYS_PAPER_MAX)
            {
                goto fail;
            }
            else if( pOEM->sPaperSize == PHYS_PAPER_UNFIXED){
                pOEM->szlPhysSize.cx = PARAM(pdwParams, 0);
                pOEM->szlPhysSize.cy = PARAM(pdwParams, 1);

                ptlUserDefSize.x = GetPrintableArea((WORD)pOEM->szlPhysSize.cx, RESOLUTION_300);
                ptlUserDefSize.y = GetPrintableArea((WORD)pOEM->szlPhysSize.cy, RESOLUTION_300);
                pOEM->ptlLogSize = ptlUserDefSize;

                CMD_SETPAC[CMD_SETPAC_CUSTOM_DOTS_PER_LINE] = LOBYTE((WORD)(ptlUserDefSize.x));
                CMD_SETPAC[CMD_SETPAC_CUSTOM_DOTS_PER_LINE + 1] = HIBYTE((WORD)(ptlUserDefSize.x));
                CMD_SETPAC[CMD_SETPAC_CUSTOM_LINES_PER_PAGE] = LOBYTE((WORD)(ptlUserDefSize.y));
                CMD_SETPAC[CMD_SETPAC_CUSTOM_LINES_PER_PAGE + 1] = HIBYTE((WORD)(ptlUserDefSize.y));
            }
            else
            {
                pOEM->ptlLogSize = phySize300[
                        pOEM->sPaperSize-PHYS_PAPER_BASE];
            }

            break;

        case RESOLUTION_600:
            pOEM->ulHorzRes = 600;
            pOEM->ulVertRes = 600;
            pOEM->ptlLogSize = phySize600[pOEM->sPaperSize-50];
            CMD_SETPAC[CMD_SETPAC_RESOLUTION] = 0x20 ;

            if (pOEM->sPaperSize < PHYS_PAPER_BASE
                    || pOEM->sPaperSize > PHYS_PAPER_MAX)
            {
                goto fail;
            }
            else if( pOEM->sPaperSize == PHYS_PAPER_UNFIXED){
                pOEM->szlPhysSize.cx = PARAM(pdwParams, 0);
                pOEM->szlPhysSize.cy = PARAM(pdwParams, 1);

                ptlUserDefSize.x = GetPrintableArea((WORD)pOEM->szlPhysSize.cx, RESOLUTION_600);
                ptlUserDefSize.y = GetPrintableArea((WORD)pOEM->szlPhysSize.cy, RESOLUTION_600);
                pOEM->ptlLogSize = ptlUserDefSize;

                CMD_SETPAC[CMD_SETPAC_CUSTOM_DOTS_PER_LINE] = LOBYTE((WORD)(ptlUserDefSize.x));
                CMD_SETPAC[CMD_SETPAC_CUSTOM_DOTS_PER_LINE + 1] = HIBYTE((WORD)(ptlUserDefSize.x));
                CMD_SETPAC[CMD_SETPAC_CUSTOM_LINES_PER_PAGE] = LOBYTE((WORD)(ptlUserDefSize.y));
                CMD_SETPAC[CMD_SETPAC_CUSTOM_LINES_PER_PAGE + 1] = HIBYTE((WORD)(ptlUserDefSize.y));
            }
            else {
                pOEM->ptlLogSize = phySize600[
                         pOEM->sPaperSize-PHYS_PAPER_BASE];
            }
            break;

        case SEND_BLOCK_DATA:
            wNumOfByte = (WORD)PARAM(pdwParams, 0);

            pOEM->wImgHeight = (WORD)PARAM(pdwParams, 1);
            pOEM->wImgWidth = (WORD)PARAM(pdwParams, 2);
            break;

        case ORIENTATION_PORTRAIT:                   // 28
        case ORIENTATION_LANDSCAPE:                 // 29
             switch(pOEM->sPaperSize){
                case PHYS_PAPER_A3 :
                case PHYS_PAPER_B4 :
                case PHYS_PAPER_LEGAL :
                case PHYS_PAPER_POSTCARD :
                    pOEM->fChangeDirection = FALSE ;
                    pOEM->fComp = TRUE ;
                    break;

                case PHYS_PAPER_A4 :
                case PHYS_PAPER_A5 :
                case PHYS_PAPER_B5 :
                case PHYS_PAPER_LETTER :
                    pOEM->fChangeDirection = TRUE ;
                    pOEM->fComp = FALSE ;
                    break;

                case PHYS_PAPER_UNFIXED :           /* Paper is not rotated in UNFIXED case */
                    pOEM->fChangeDirection = FALSE ;
                    pOEM->fComp = TRUE ;
                    break;
            }
            break;

        case PHYS_PAPER_A3:                 // 50
             pOEM->sPaperSize = PHYS_PAPER_A3 ;
             CMD_SETPAC[CMD_SETPAC_FRONT_TRAY_PAPER_SIZE] = 0x04 ;
             CMD_SETPAC[CMD_SETPAC_1ST_CASSETTE_PAPER_SIZE] = 0x04 ;
             CMD_SETPAC[CMD_SETPAC_2ND_CASSETTE_PAPER_SIZE] = 0x04 ;
             break ;
        case PHYS_PAPER_A4:                 // 51
             pOEM->sPaperSize = PHYS_PAPER_A4 ;
             CMD_SETPAC[CMD_SETPAC_FRONT_TRAY_PAPER_SIZE] = 0x83 ;
             CMD_SETPAC[CMD_SETPAC_1ST_CASSETTE_PAPER_SIZE] = 0x83 ;
             CMD_SETPAC[CMD_SETPAC_2ND_CASSETTE_PAPER_SIZE] = 0x83 ;
             break ;
        case PHYS_PAPER_B4:                 // 54
             pOEM->sPaperSize = PHYS_PAPER_B4 ;
             CMD_SETPAC[CMD_SETPAC_FRONT_TRAY_PAPER_SIZE] = 0x07 ;
             CMD_SETPAC[CMD_SETPAC_1ST_CASSETTE_PAPER_SIZE] = 0x07 ;
             CMD_SETPAC[CMD_SETPAC_2ND_CASSETTE_PAPER_SIZE] = 0x07 ;
             break ;
        case PHYS_PAPER_LETTER:             // 57
             pOEM->sPaperSize = PHYS_PAPER_LETTER ;
             CMD_SETPAC[CMD_SETPAC_FRONT_TRAY_PAPER_SIZE] = 0x90 ;
             CMD_SETPAC[CMD_SETPAC_1ST_CASSETTE_PAPER_SIZE] = 0x90 ;
             CMD_SETPAC[CMD_SETPAC_2ND_CASSETTE_PAPER_SIZE] = 0x90 ;
             break ;
        case PHYS_PAPER_LEGAL:                // 58
             pOEM->sPaperSize = PHYS_PAPER_LEGAL ;
             CMD_SETPAC[CMD_SETPAC_FRONT_TRAY_PAPER_SIZE] = 0x11 ;
             CMD_SETPAC[CMD_SETPAC_1ST_CASSETTE_PAPER_SIZE] = 0x11 ;
             CMD_SETPAC[CMD_SETPAC_2ND_CASSETTE_PAPER_SIZE] = 0x11 ;
             break ;

        case PHYS_PAPER_B5:                 // 55
             pOEM->sPaperSize = PHYS_PAPER_B5 ;
             CMD_SETPAC[CMD_SETPAC_FRONT_TRAY_PAPER_SIZE] = 0x86 ;
             CMD_SETPAC[CMD_SETPAC_1ST_CASSETTE_PAPER_SIZE] = 0x86 ;
             CMD_SETPAC[CMD_SETPAC_2ND_CASSETTE_PAPER_SIZE] = 0x86 ;
             break ;
        case PHYS_PAPER_A5:                 // 52
             pOEM->sPaperSize = PHYS_PAPER_A5 ;
             CMD_SETPAC[CMD_SETPAC_FRONT_TRAY_PAPER_SIZE] = 0x82 ;
             CMD_SETPAC[CMD_SETPAC_1ST_CASSETTE_PAPER_SIZE] = 0x82 ;
             CMD_SETPAC[CMD_SETPAC_2ND_CASSETTE_PAPER_SIZE] = 0x82 ;
             break ;

        case PHYS_PAPER_POSTCARD:            // 59
             pOEM->sPaperSize = PHYS_PAPER_POSTCARD ;
             CMD_SETPAC[CMD_SETPAC_FRONT_TRAY_PAPER_SIZE] = 0x17 ;
             CMD_SETPAC[CMD_SETPAC_1ST_CASSETTE_PAPER_SIZE] = 0x17 ;
             CMD_SETPAC[CMD_SETPAC_2ND_CASSETTE_PAPER_SIZE] = 0x17 ;
             break ;

        case PHYS_PAPER_UNFIXED:            // 60
             pOEM->sPaperSize = PHYS_PAPER_UNFIXED ;
             CMD_SETPAC[CMD_SETPAC_FRONT_TRAY_PAPER_SIZE] = 0x3F ;
             CMD_SETPAC[CMD_SETPAC_1ST_CASSETTE_PAPER_SIZE] = 0x3F ;
             CMD_SETPAC[CMD_SETPAC_2ND_CASSETTE_PAPER_SIZE] = 0x3F ;

             break ;

        case PAPER_SRC_FTRAY:
             CMD_SETPAC[CMD_SETPAC_INPUT_BIN] = 0x01 ;
             break ;

        case PAPER_SRC_CAS1:
            CMD_SETPAC[CMD_SETPAC_INPUT_BIN] = 0x02 ;
            break;

        case PAPER_SRC_CAS2:
            CMD_SETPAC[CMD_SETPAC_INPUT_BIN] = 0x04 ;
            break;

        case PAPER_SRC_AUTO:
            CMD_SETPAC[CMD_SETPAC_INPUT_BIN] = 0x04 ;
            break;


        case TONER_SAVE_MEDIUM:                // 100
            CMD_SETPAC[CMD_SETPAC_TONER_SAVE_MODE] = 0x02 ;
            break;

        case TONER_SAVE_DARK:                // 101
            CMD_SETPAC[CMD_SETPAC_TONER_SAVE_MODE] = 0x04 ;
            break;

        case TONER_SAVE_LIGHT:                // 102
            CMD_SETPAC[CMD_SETPAC_TONER_SAVE_MODE] = 0x01 ;
            break;


        case PAGECONTROL_MULTI_COPIES:
            CMD_SETPAC[24] = (BYTE)PARAM(pdwParams, 0);
            pOEM->sCopyNum = (BYTE)PARAM(pdwParams, 0);
            break;
        
        case Y_REL_MOVE :

            if (0 == pOEM->ulHorzRes)
                goto fail;

            pOEM->dwYmove=(WORD)*pdwParams/(MASTERUNIT/(WORD)pOEM->ulHorzRes);

// ISSUE-2002/3/18-takashim - Faking Unidrv here?
// iRet = 0; below is intentional:  Retuning dwYmove will cause
// incorrect outputs.
// DestYRel in GPD means the coordinate relative to the current
// cursor position.  Here, minidriver is always returning 0 (no move)
// to Unidrv, so it is always absolute coordinate (relative to
// the origin)?

            if(pOEM->dwCurCursorY < pOEM->dwYmove){
                pOEM->dwYmove -= pOEM->dwCurCursorY ;
            }else{
                pOEM->dwYmove = 0 ;
            }

//            iRet = pOEM->dwYmove;
            iRet = 0;

            break;
            

        default:
            break;
    }
    return iRet;

fail:
    return -1;
}


/*****************************************************************************/
/*                                                                             */
/*    Module:    GetPrintableArea                                              */
/*                                                                             */
/*    Function:  Calculate PrintableArea for user defined paper                 */
/*                                                                             */
/*    Syntax:    WORD GetPrintableArea(WORD physSize, INT iRes)                 */
/*                                                                             */
/*    Input:       physSize                                                      */
/*               iRes                                                          */
/*                                                                             */
/*    Output:    WORD                                                          */
/*                                                                             */
/*    Notice:                                                                  */
/*                                                                             */
/*****************************************************************************/
WORD GetPrintableArea(WORD physSize, INT iRes)
{
    DWORD dwArea ;
    DWORD dwPhysSizeMMx10 = physSize * 254 / MASTERUNIT;

    /* Unit of phySize is MASTERUNIT(=1200) */

    if(iRes == RESOLUTION_300){
        dwArea = (((WORD)(( ( (DWORD)(dwPhysSizeMMx10*300/25.4) -
                            2*( (DWORD)(4*300*10/25.4) ) ) / 10 +7)/8))) * 8;
    }else{
        dwArea = (((WORD)(( ( (DWORD)(dwPhysSizeMMx10*600/25.4) -
                            2*( (DWORD)(4*600*10/25.4) ) ) / 10 +7)/8))) * 8;
    }

    return (WORD)dwArea ;
}

// NOTICE-2002/3/18/-takashim - Comment
// // #94193: shold create temp. file on spooler directory.
//

/*++

Routine Description:

  This function comes up with a name for a spool file that we should be
  able to write to.

  Note: The file name returned has already been created.

Arguments:

  hPrinter - handle to the printer that we want a spool file for.

  ppwchSpoolFileName: pointer that will receive an allocated buffer
                      containing the file name to spool to.  CALLER
                      MUST FREE.  Use LocalFree().


Return Value:

  TRUE if everything goes as expected.
  FALSE if anything goes wrong.

--*/

BOOL
GetSpoolFileName(
  IN HANDLE hPrinter,
  IN OUT PWCHAR pwchSpoolPath
)
{
  PBYTE         pBuffer = NULL;
  DWORD         dwAllocSize;
  DWORD         dwNeeded = 0;
  DWORD         dwRetval;
  HANDLE        hToken=NULL;

  hToken = RevertToPrinterSelf();

  //
  //  In order to find out where the spooler's directory is, we add
  //  call GetPrinterData with DefaultSpoolDirectory.
  //

  dwAllocSize = ( MAX_PATH ) * sizeof (WCHAR);

  for (;;)
  {
    pBuffer = LocalAlloc( LMEM_FIXED, dwAllocSize );

    if ( pBuffer == NULL )
    {
      ERR((DLLTEXT("LocalAlloc faild, %d\n"), GetLastError()));
      goto Failure;
    }

    if ( GetPrinterData( hPrinter,
                         SPLREG_DEFAULT_SPOOL_DIRECTORY,
                         NULL,
                         pBuffer,
                         dwAllocSize,
                         &dwNeeded ) == ERROR_SUCCESS )
    {
      break;
    }

    if ( ( dwNeeded < dwAllocSize ) ||( GetLastError() != ERROR_MORE_DATA ))
    {
      ERR((DLLTEXT("GetPrinterData failed in a non-understood way.\n")));
      goto Failure;
    }

    //
    // Free the current buffer and increase the size that we try to allocate
    // next time around.
    //

    LocalFree( pBuffer );

    dwAllocSize = dwNeeded;
  }

// FUTURE-2002/3/18-takashim - Temp file path restricted to ANSI.
// According to the SDK document, the pathname handled by GetTempFileName
// must be consist of ANSI characters.  What happens with double-byte
// characters, etc??

  if( !GetTempFileName( (LPWSTR)pBuffer, TEMP_NAME_PREFIX, 0, pwchSpoolPath ))
  {
      goto Failure;
  }

  //
  //  At this point, the spool file name should be done.  Free the structure
  //  we used to get the spooler temp dir and return.
  //

  LocalFree( pBuffer );

  if (NULL != hToken) {
      if (!ImpersonatePrinterClient(hToken))
      {
        // failure..
        return FALSE;
      }
  }

  return( TRUE );

Failure:

  //
  //  Clean up and fail.
  //
  if ( pBuffer != NULL )
  {
    LocalFree( pBuffer );
  }

  if (hToken != NULL)
  {
      (void)ImpersonatePrinterClient(hToken);
  }
  return ( FALSE );
}

//SPLBUF is used for control temp files.
//This printer need the number of bytes of whole page data.
BOOL InitSpoolBuffer(LPSB lpsb)
{
    lpsb->dwWrite = 0 ;
    lpsb->TempName[0] = __TEXT('\0') ;
    lpsb->hFile = INVALID_HANDLE_VALUE ;

    return TRUE;
}

BOOL MyCreateFile(PDEVOBJ pdevobj, LPSB lpsb)
{
    HANDLE hToken = NULL;
    BOOL bRet = FALSE;

    if (!GetSpoolFileName(pdevobj->hPrinter, lpsb->TempName)) {
        //DBGPRINT(DBG_WARNING, ("GetSpoolFileName failed.\n"));
        goto fail;
    }

    hToken = RevertToPrinterSelf();

    lpsb->hFile = CreateFile((LPCTSTR)lpsb->TempName,
                     (GENERIC_READ | GENERIC_WRITE), 
                     0,                             
                     NULL,                            
                     CREATE_ALWAYS,                 
                     FILE_ATTRIBUTE_NORMAL,         
                     NULL) ;

    if(lpsb->hFile == INVALID_HANDLE_VALUE)
    {
        //DBGPRINT(DBG_WARNING, ("Tmp file cannot create.\n"));
        DeleteFile(lpsb->TempName);
        lpsb->TempName[0] = __TEXT('\0') ;
        goto fail;
    }
    bRet = TRUE;

fail:
    if (hToken) (void)ImpersonatePrinterClient(hToken);
    return bRet ;
}

BOOL MyDeleteFile(PDEVOBJ pdevobj, LPSB lpsb)
{    
    HANDLE hToken = NULL;
    BOOL bRet = FALSE;

    if(lpsb->hFile != INVALID_HANDLE_VALUE){

        if (0 == CloseHandle(lpsb->hFile)) {
            //DBGPRINT(DBG_WARNING, ("CloseHandle error %d\n"));
            goto fail;
        }
        lpsb->hFile = INVALID_HANDLE_VALUE ;
        hToken = RevertToPrinterSelf();
        if (0 == DeleteFile(lpsb->TempName)) {
            //DBGPRINT(DBG_WARNING, ("DeleteName error %d\n",GetLastError()));
            goto fail;
        }
        lpsb->TempName[0] = __TEXT('\0');

    }
    bRet = TRUE;

fail:
    if (hToken) (void)ImpersonatePrinterClient(hToken);
    return bRet;
}

//Spool page data to temp file
BOOL MySpool
    (PDEVOBJ pdevobj,
     LPSB  lpsb,
     PBYTE pBuf,
     DWORD dwLen)
{
    DWORD dwTemp, dwTemp2;
    BYTE *pTemp;

    if (lpsb->hFile != INVALID_HANDLE_VALUE) {

        pTemp = pBuf;
        dwTemp = dwLen;
        while (dwTemp > 0) {

            if (0 == WriteFile(lpsb->hFile,
                               pTemp,
                               dwTemp,
                               &dwTemp2,
                               NULL)
                    || dwTemp2 > dwTemp) {

                ERR((DLLTEXT("WriteFile error in CacheData %d.\n"),
                    GetLastError()));
                return FALSE;
            }
            pTemp += dwTemp2;
            dwTemp -= dwTemp2;
            lpsb->dwWrite += dwTemp2 ;
        }
        return TRUE;
    }
    else {
        return WRITESPOOLBUF(pdevobj, pBuf, dwLen);
    }
}

//Dump out temp file to printer
BOOL
SpoolOut(PDEVOBJ pdevobj, LPSB lpsb)
{
 
   DWORD dwSize, dwTemp, dwTemp2;
   HANDLE hFile;

    BYTE  Buf[SPOOL_OUT_BUF_SIZE];

    hFile = lpsb->hFile ;
    dwSize = lpsb->dwWrite ;

    VERBOSE(("dwSize=%ld\n", dwSize));

    if (0L != SetFilePointer(hFile, 0L, NULL, FILE_BEGIN)) {

        ERR((DLLTEXT("SetFilePointer failed %d\n"),
            GetLastError()));
        return FALSE;
    }

    for ( ; dwSize > 0; dwSize -= dwTemp2) {

        dwTemp = ((SPOOL_OUT_BUF_SIZE < dwSize)
            ? SPOOL_OUT_BUF_SIZE : dwSize);

        if (0 == ReadFile(hFile, Buf, dwTemp, &dwTemp2, NULL)
                || dwTemp2 > dwTemp) {
            ERR((DLLTEXT("ReadFile error in SendCachedData.\n")));
            return FALSE;
        }

        if (dwTemp2 > 0) {
            if (!WRITESPOOLBUF(pdevobj, Buf, dwTemp2))
                return FALSE;
        }
    }

    return TRUE;
}

BOOL MyStartDoc(PDEVOBJ pdevobj)
{
    return
    WRITESPOOLBUF(pdevobj, (PBYTE)CMD_BEGIN_DOC_1, sizeof(CMD_BEGIN_DOC_1)) &&
    WRITESPOOLBUF(pdevobj, (PBYTE)CMD_BEGIN_DOC_2, sizeof(CMD_BEGIN_DOC_2)) &&
    WRITESPOOLBUF(pdevobj, (PBYTE)CMD_BEGIN_DOC_3, sizeof(CMD_BEGIN_DOC_3)) &&
    WRITESPOOLBUF(pdevobj, (PBYTE)CMD_BEGIN_DOC_4, sizeof(CMD_BEGIN_DOC_4)) &&
    WRITESPOOLBUF(pdevobj, (PBYTE)CMD_BEGIN_DOC_5, sizeof(CMD_BEGIN_DOC_5));
}

BOOL MyEndPage(PDEVOBJ pdevobj)
{
    PIBMPDEV    pOEM;
    LPSB        lpsb, lpsbco ;
    DWORD        dwPageLen ;
    WORD        wTmph, wTmpl ;

    pOEM = (PIBMPDEV)pdevobj->pdevOEM;
    lpsb = &(pOEM->sb) ;
    lpsbco = &(pOEM->sbcomp) ;

    if(pOEM->fChangeDirection == FALSE) {
        dwPageLen = lpsb->dwWrite;
    }
    else {
        dwPageLen = lpsbco->dwWrite ;
    }

// NOTICE-2002/3/18-takashim - What is this?

    dwPageLen -= 3 ; //End page Command Len

    VERBOSE(("MyEndPage - dwPageLen=%ld\n",
        dwPageLen));

    wTmpl = LOWORD(dwPageLen) ;
    wTmph = HIWORD(dwPageLen) ;
    
    CMD_SETPAC[CMD_SETPAC_PAGE_LENGTH] = LOBYTE(wTmpl);
    CMD_SETPAC[CMD_SETPAC_PAGE_LENGTH + 1] = HIBYTE(wTmpl);
    CMD_SETPAC[CMD_SETPAC_PAGE_LENGTH + 2] = LOBYTE(wTmph);
    CMD_SETPAC[CMD_SETPAC_PAGE_LENGTH + 3] = HIBYTE(wTmph);

    if (!WRITESPOOLBUF(pdevobj, CMD_SETPAC, sizeof(CMD_SETPAC)))
        return FALSE;

    if(pOEM->fChangeDirection == FALSE){
        if (!SpoolOut(pdevobj, lpsb))
            return FALSE;
    }else{
        if (!SpoolOut(pdevobj, lpsbco))
            return FALSE;
    }

    //InitFiles
    lpsbco->dwWrite = 0 ;
    lpsb->dwWrite = 0 ;

    if(INVALID_SET_FILE_POINTER==SetFilePointer(lpsb->hFile,0,NULL,FILE_BEGIN)){
        ERR((DLLTEXT("SetFilePointer failed %d\n"),
             GetLastError()));
        return FALSE;
    }

    if(INVALID_SET_FILE_POINTER==SetFilePointer(lpsbco->hFile,0,NULL,FILE_BEGIN)){
        ERR((DLLTEXT("SetFilePointer failed %d\n"),
             GetLastError()));
        return FALSE;
    }

    return TRUE;
}

BOOL MyEndDoc(PDEVOBJ pdevobj)
{
    PIBMPDEV    pOEM;
    LPSB        lpsb, lpsbco ;
    WORD        wTmph, wTmpl ;
    DWORD        dwPageLen ;
    SHORT        i ;
    LPPD        lppdTemp ;
    BOOL        bRet = FALSE;

    pOEM = (PIBMPDEV)pdevobj->pdevOEM;
    lpsb = &(pOEM->sb) ;
    lpsbco = &(pOEM->sbcomp) ;


    if (!WRITESPOOLBUF(pdevobj, (PBYTE)CMD_END_JOB, sizeof(CMD_END_JOB)))
        goto fail;

    if (!MyDeleteFile(pdevobj, lpsb))
        goto fail;
    if (!MyDeleteFile(pdevobj, lpsbco))
        goto fail;
    bRet = TRUE;

fail:
    return bRet;
}

BOOL WriteFileForP_Paper(PDEVOBJ pdevobj, PBYTE pBuf, DWORD dwLen)
{
    PIBMPDEV           pOEM;
    ULONG                ulHorzPixel;
    WORD                wCompLen;
    DWORD                dwWhiteLen ;
    DWORD                dwTmp ;

    pOEM = (PIBMPDEV)pdevobj->pdevOEM;

    if (pOEM->dwYmove > 0) {

        dwWhiteLen = pOEM->wImgWidth * pOEM->dwYmove;

        if (!SpoolWhiteData(pdevobj, dwWhiteLen, TRUE))
            return FALSE;

        pOEM->dwCurCursorY += pOEM->dwYmove;
        pOEM->dwYmove = 0;
    }

    pOEM->dwCurCursorY += dwLen/pOEM->wImgWidth - 1 ;

    return SendPageData(pdevobj, pBuf, dwLen) ;
}

BOOL WriteFileForL_Paper(PDEVOBJ pdevobj, PBYTE pBuf, DWORD dwLen)
{
    PIBMPDEV           pOEM;
    ULONG                ulHorzPixel;
    WORD                wCompLen;
    DWORD                dwWhiteLen ;

    DWORD i, j;
    DWORD dwHeight, dwWidth;
    PBYTE pTemp;

    pOEM = (PIBMPDEV)pdevobj->pdevOEM;

    if (pOEM->dwYmove > 0) {

        dwWhiteLen = pOEM->wImgWidth * pOEM->dwYmove;

        if (!SpoolWhiteData(pdevobj, dwWhiteLen, FALSE))
            return FALSE;

        pOEM->dwCurCursorY += pOEM->dwYmove;
        pOEM->dwYmove = 0;
    }

    pOEM->dwCurCursorY += dwLen/pOEM->wImgWidth - 1 ;
    pOEM->dwOffset ++ ;

    return MySpool(pdevobj, &(pOEM->sb), pBuf,dwLen);
}


//fill page blanks.
BOOL FillPageRestData(PDEVOBJ pdevobj)
{

    PIBMPDEV    pOEM ;
    DWORD        dwRestHigh ;
    DWORD        dwWhiteLen ;

    pOEM = (PIBMPDEV)pdevobj->pdevOEM;
    
    dwRestHigh = pOEM->ptlLogSize.y - pOEM->dwCurCursorY ;

    if(dwRestHigh <= 0)
        return TRUE;

    dwWhiteLen = pOEM->ptlLogSize.x * dwRestHigh;

    return SpoolWhiteData(pdevobj, dwWhiteLen, pOEM->fComp);
}

//not white data
BOOL SendPageData(PDEVOBJ pdevobj, PBYTE pSrcImage, DWORD dwLen)
{
    PIBMPDEV           pOEM;

    pOEM = (PIBMPDEV)pdevobj->pdevOEM;

    return SpoolOutComp(&pOEM->Soc, pdevobj, &pOEM->sb, pSrcImage, dwLen);
}

BOOL SpoolWhiteData(PDEVOBJ pdevobj, DWORD dwWhiteLen, BOOL fComp)
{

    PIBMPDEV    pOEM;
    PBYTE        pWhite ;
    WORD        wCompLen ;
    DWORD        dwTempLen ;

    pOEM = (PIBMPDEV)pdevobj->pdevOEM;

    if(dwWhiteLen == 0)
        return TRUE;

    if(dwWhiteLen > MAXIMGSIZE){
        dwTempLen = MAXIMGSIZE ;
    }else{
        dwTempLen = dwWhiteLen ;
    }

    if (!AllocTempBuffer(pOEM, dwTempLen))
        return FALSE;
    pWhite = pOEM->pTempImage;

    ZeroMemory(pWhite, dwTempLen);

    if(fComp == TRUE)
    {
        DWORD dwTemp;

        while (0 < dwWhiteLen) {

            if (MAXIMGSIZE <= dwWhiteLen)
                dwTemp = MAXIMGSIZE;
            else
                dwTemp = dwWhiteLen;

            if (!SpoolOutComp(&pOEM->Soc, pdevobj, &pOEM->sb, pWhite, dwTemp))
                return FALSE;
            dwWhiteLen -= dwTemp;
        }

    }
    else{
        if(dwWhiteLen > MAXIMGSIZE){
            while(dwWhiteLen > MAXIMGSIZE){
                if (!MySpool(pdevobj, &pOEM->sb, pWhite, MAXIMGSIZE))
                    return FALSE;

                dwWhiteLen -= MAXIMGSIZE ;
            }
        }

        if(dwWhiteLen > 0){
            if (!MySpool(pdevobj, &pOEM->sb, pWhite, dwWhiteLen))
                return FALSE;

        }
    }

    return TRUE;
}

BOOL SpoolOutChangedData(PDEVOBJ pdevobj, LPSB lpsb)
{
    PIBMPDEV    pOEM;
    POINTL        ptlDataPos ;
    DWORD        dwFilePos, dwTemp;
    HANDLE        hFile ;
    PBYTE        pSaveFileData ;
    PBYTE        pTemp;
    PBYTE        pTransBuf ;
    DWORD        X, Y;
    DWORD        dwFirstPos ;
    INT h, i, j, k;

    POINTL ptlBand;
    PBYTE pSrc, pDst, pSrcSave;
    DWORD dwBandY, dwImageY, dwImageX;
    BOOL bBlank, bZero;
    BOOL bRet = FALSE;

    pOEM = (PIBMPDEV)pdevobj->pdevOEM;
    hFile = pOEM->sb.hFile ;

    // band size in pixels
    ptlBand.x = pOEM->ptlLogSize.y;
    ptlBand.y = TRANS_BAND_Y_SIZE;

    //ファイルの読み込み開始位置を計算
    // Calculate read start positoin in the file
    ptlDataPos.x = 0 ;
    ptlDataPos.y = pOEM->dwCurCursorY + pOEM->dwOffset ;

    //縦方向、１行のした部分余白
    // Y direction, blank area beneth.
    dwImageX = ((ptlDataPos.y + 7) / 8) * 8;

    // Buffer for loading file data (scan lines for a band)
    pSaveFileData = (PBYTE)MemAlloc((ptlBand.y / 8) * dwImageX);
    if (NULL == pSaveFileData) {
        ERR(("Failed to allocate memory.\n"));
        return FALSE;
    }

    // Buffer for transpositions (one scan line)
    pTransBuf = (PBYTE)MemAlloc((ptlBand.x / 8));
    if (NULL == pTransBuf) {
        ERR(("Failed to allocate memory.\n"));
// #441444: PREFIX: reference NULL pointer.
        goto out;
    }

    //ファイルの読み込み位置指定。
    // Specify read start positoin in the file
    dwFirstPos = pOEM->wImgWidth - 1;

    if (!MySpool(pdevobj,&(pOEM->sbcomp),
                (PBYTE)CMD_BEGIN_PAGE, sizeof(CMD_BEGIN_PAGE)))
        goto out;
    if (!SpoolOutCompStart(&pOEM->Soc))
        goto out;

    dwImageY = pOEM->wImgWidth;

    bBlank = FALSE;
    bZero = FALSE;
    for (X = 0; X < (DWORD)pOEM->ptlLogSize.x / 8; X += dwBandY) {

        //余白部分を先にセット。読み飛ばす。
        // Set blank area, then read skip.
        dwBandY = ptlBand.y / 8;
        if (dwBandY > (DWORD)pOEM->ptlLogSize.x / 8 - X)
            dwBandY = (DWORD)pOEM->ptlLogSize.x / 8 - X;

        // White scanlines.  Currently the trailing ones only,
        // desired to be udpated to include others.

        if (X >= dwImageY) {
            bBlank = TRUE;
        }

        // Output white scanline.
        if (bBlank) {

            if (!bZero) {
                ZeroMemory(pTransBuf, (ptlBand.x / 8));
                bZero = TRUE;
            }

            for (i = 0; i < (INT)dwBandY * 8; i++) {
                if (!SpoolOutComp(&pOEM->Soc, pdevobj, &pOEM->sbcomp,
                    (PBYTE)pTransBuf, (ptlBand.x / 8)))
                    goto out;
            }
            continue;
        }

        // Non-white scanlines.

        pTemp = pSaveFileData ;
        dwFilePos = pOEM->wImgWidth - X - dwBandY;

        //縦方向１行をファイルから読み取る。
        // Read vertial one line from the file.

        for (Y = 0; Y < dwImageX; pTemp += dwBandY, Y++) {

            if (Y >= (DWORD)ptlDataPos.y) {
                ZeroMemory(pTemp, dwBandY);
                continue;
            }

            if(INVALID_SET_FILE_POINTER==SetFilePointer(hFile,dwFilePos,NULL,FILE_BEGIN)){
                 ERR((DLLTEXT("SetFilePointer failed %d\n"),
                     GetLastError()));
// #441442: PREFIX: leaking memory.
                    // return;
                    goto out;
            }

            if (0 == ReadFile(hFile, pTemp, dwBandY, &dwTemp, NULL)
                    || dwTemp > dwBandY) {
                 ERR(("Faild reading data from file. (%d)\n",
                     GetLastError()));
// #441442: PREFIX: leaking memory.
                // return;
                goto out;
            }

            dwFilePos += pOEM->wImgWidth;

        }//End of Y loop

        // Transposition and output dwBandY * 8 scan lines
        for (h = 0; h < (INT)dwBandY; h++) {

            //VERBOSE(("> %d/%d\n", h, dwBandY));

            pSrcSave = pSaveFileData + dwBandY - 1 - h;

            // Transposition and output eight scan lines
            for (j = 0; j < 8; j++) {

                pSrc = pSrcSave;
                pDst = pTransBuf;
                ZeroMemory(pDst, (ptlBand.x / 8));

                // Transposition one scan line

                for (i = 0; i < (INT)(dwImageX / 8); i++){

                    for (k = 0; k < 8; k++) {

                        if (0 != (*pSrc & Mask[7 - j])) {
                            *pDst |= Mask[k];
                        }
                        pSrc += dwBandY;
                    }
                    pDst++;
                }

                // Output one scan line
                if (!SpoolOutComp(&pOEM->Soc, pdevobj, &pOEM->sbcomp,
                    (PBYTE)pTransBuf, (ptlBand.x / 8)))
                    goto out;
            }
        }

    }

    // Mark end of image
    if (!SpoolOutCompEnd(&pOEM->Soc, pdevobj, &pOEM->sbcomp))
        goto out;
    if (!MySpool(pdevobj, &(pOEM->sbcomp),
        (PBYTE)CMD_END_PAGE, sizeof(CMD_END_PAGE)))
        goto out;

    bRet = TRUE;

// #441442: PREFIX: leaking memory.
out:
    if (NULL != pSaveFileData){
        MemFree(pSaveFileData);
    }
    if(NULL != pTransBuf){
        MemFree(pTransBuf);
    }

    return bRet;
}

BOOL
AllocTempBuffer(
    PIBMPDEV pOEM,
    DWORD dwNewBufLen)
{
   if (NULL == pOEM->pTempImage ||
        dwNewBufLen > pOEM->dwTempBufLen) {

        if (NULL != pOEM->pTempImage) {
            MemFree(pOEM->pTempImage);
        }
        pOEM->pTempImage = (PBYTE)MemAlloc(dwNewBufLen);
        if (NULL == pOEM->pTempImage) {
            WARNING(("Failed to allocate memory. (%d)\n",
                GetLastError()));

            pOEM->dwTempBufLen = 0;
            return FALSE;
        }
        pOEM->dwTempBufLen = dwNewBufLen;
    }
    return TRUE;
}

BOOL
SpoolOutCompStart(
    PSOCOMP pSoc)
{
    pSoc->iNRCnt = 0;
    pSoc->iRCnt = 0;
    pSoc->iPrv = -1;

    return TRUE;
}

BOOL
SpoolOutCompEnd(
    PSOCOMP pSoc,
    PDEVOBJ pdevobj,
    LPSB psb)
{
    BYTE jTemp;

    if (0 < pSoc->iNRCnt) {
        jTemp = ((BYTE)pSoc->iNRCnt) - 1;
        if (!MySpool(pdevobj, psb, &jTemp, 1))
            return FALSE;
        if (!MySpool(pdevobj, psb, pSoc->pjNRBuf, pSoc->iNRCnt))
            return FALSE;
        pSoc->iNRCnt = 0;
    }

    if (0 < pSoc->iRCnt) {
        jTemp = (0 - (BYTE)pSoc->iRCnt) + 1;
        if (!MySpool(pdevobj, psb, &jTemp, 1))
            return FALSE;
        if (!MySpool(pdevobj, psb, &pSoc->iPrv, 1))
            return FALSE;
        pSoc->iRCnt = 0;
    }

    return TRUE;
}

BOOL
SpoolOutComp(
    PSOCOMP pSoc,
    PDEVOBJ pdevobj,
    LPSB psb,
    PBYTE pjBuf,
    DWORD dwLen)
{
    BYTE jCur, jTemp;

    while (0 < dwLen--) {

        jCur = *pjBuf++;

        if (pSoc->iPrv == jCur) {

            if (0 < pSoc->iNRCnt) {
                if (1 < pSoc->iNRCnt) {
                    jTemp = ((BYTE)pSoc->iNRCnt - 1) - 1;
                    if (!MySpool(pdevobj, psb, &jTemp, 1))
                        return FALSE;
                    if (!MySpool(pdevobj, psb, pSoc->pjNRBuf, pSoc->iNRCnt - 1))
                        return FALSE;
                }
                pSoc->iNRCnt = 0;
                pSoc->iRCnt = 1;
            }

            pSoc->iRCnt++;

            if (RPEAK == pSoc->iRCnt) {
                jTemp = (0 - (BYTE)pSoc->iRCnt) + 1;
                if (!MySpool(pdevobj, psb, &jTemp, 1))
                    return FALSE;
                if (!MySpool(pdevobj, psb, &jCur, 1))
                    return FALSE;
                pSoc->iRCnt = 0;
            }
        }
        else {

            if (0 < pSoc->iRCnt) {
                jTemp = (0 - (BYTE)pSoc->iRCnt) + 1;
                if (!MySpool(pdevobj, psb, &jTemp, 1))
                    return FALSE;
                if (!MySpool(pdevobj, psb, &pSoc->iPrv, 1))
                    return FALSE;
                pSoc->iRCnt = 0;
            }

            pSoc->pjNRBuf[pSoc->iNRCnt++] = jCur;

            if (NRPEAK == pSoc->iNRCnt) {
                jTemp = ((BYTE)pSoc->iNRCnt) - 1;
                if (!MySpool(pdevobj, psb, &jTemp, 1))
                    return FALSE;
                if (!MySpool(pdevobj, psb, pSoc->pjNRBuf, pSoc->iNRCnt))
                    return FALSE;
                pSoc->iNRCnt = 0;
            }
        }
        pSoc->iPrv = jCur;
    }

    return TRUE;
}

/*++

Routine Name

    ImpersonationToken

Routine Description:

    This routine checks if a token is a primary token or an impersonation 
    token.    
    
Arguments:

    hToken - impersonation token or primary token of the process
    
Return Value:

    TRUE, if the token is an impersonation token
    FALSE, otherwise.
    
--*/
BOOL
ImpersonationToken(
    IN HANDLE hToken
    )
{
    BOOL       bRet = TRUE;
    TOKEN_TYPE eTokenType;
    DWORD      cbNeeded;
    DWORD      LastError;

    //
    // Preserve the last error. Some callers of ImpersonatePrinterClient (which
    // calls ImpersonationToken) rely on the fact that ImpersonatePrinterClient
    // does not alter the last error.
    //
    LastError = GetLastError();
        
    //
    // Get the token type from the thread token.  The token comes 
    // from RevertToPrinterSelf. An impersonation token cannot be 
    // queried, because RevertToPRinterSelf doesn't open it with 
    // TOKEN_QUERY access. That's why we assume that hToken is
    // an impersonation token by default
    //
    if (GetTokenInformation(hToken,
                            TokenType,
                            &eTokenType,
                            sizeof(eTokenType),
                            &cbNeeded))
    {
        bRet = eTokenType == TokenImpersonation;
    }        
    
    SetLastError(LastError);

    return bRet;
}

/*++

Routine Name

    RevertToPrinterSelf

Routine Description:

    This routine will revert to the local system. It returns the token that
    ImpersonatePrinterClient then uses to imersonate the client again. If the
    current thread doesn't impersonate, then the function merely returns the
    primary token of the process. (instead of returning NULL) Thus we honor
    a request for reverting to printer self, even if the thread is not impersonating.
    
Arguments:

    None.
    
Return Value:

    NULL, if the function failed
    HANDLE to token, otherwise.
    
--*/
HANDLE
RevertToPrinterSelf(
    VOID
    )
{
    HANDLE   NewToken, OldToken, cToken;
    BOOL	 Status;

    NewToken = NULL;

    Status = OpenThreadToken(GetCurrentThread(),
							 TOKEN_IMPERSONATE,
							 TRUE,
							 &OldToken);
    if (Status) 
    {
        //
        // We are currently impersonating
        //
		cToken = GetCurrentThread();
        Status = SetThreadToken(&cToken,
								NewToken);       
		if (!Status) {
			return NULL;
		}
    }
	else if (GetLastError() == ERROR_NO_TOKEN)
    {
        //
        // We are not impersonating
        //
        Status = OpenProcessToken(GetCurrentProcess(),
								  TOKEN_QUERY,
								  &OldToken);

		if (!Status) {
			return NULL;
		}
    }
    
    return OldToken;
}

/*++

Routine Name

    ImpersonatePrinterClient

Routine Description:

    This routine attempts to set the passed in hToken as the token for the
    current thread. If hToken is not an impersonation token, then the routine
    will simply close the token.
    
Arguments:

    hToken - impersonation token or primary token of the process
    
Return Value:

    TRUE, if the function succeeds in setting hToken
    FALSE, otherwise.
    
--*/
BOOL
ImpersonatePrinterClient(
    HANDLE  hToken)
{
    BOOL	Status;
	HANDLE	cToken;

    //
    // Check if we have an impersonation token
    //
    if (ImpersonationToken(hToken)) 
    {
		cToken = GetCurrentThread();
        Status = SetThreadToken(&cToken,
								hToken);       

        if (!Status) 
        {
            return FALSE;
        }
    }

    CloseHandle(hToken);

    return TRUE;
}
