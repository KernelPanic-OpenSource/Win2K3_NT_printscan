/*++

Copyright (c) 1996  Microsoft Corporation

Module Name:

    rx_thrd.c

Abstract:

    This module implements async. MR/MH page decoding in a separate thread.

Author:

    Rafael Lisitsa (RafaelL) 14-Aug-1996

Revision History:

--*/
#define USE_DEBUG_CONTEXT   DEBUG_CONTEXT_T30_MAIN

#include "prep.h"

#include "efaxcb.h"
#include "t30.h"
#include "hdlc.h"
#include "debug.h"

#include "tiff.h"
#include "..\..\..\tiff\src\fasttiff.h"

#include "glbproto.h"
#include "t30gl.h"


// 15 min.
#define WAIT_FOR_NEXT_STRIP_RX_TIMEOUT      900000
#define RET_NEXT_STRIP_RX_TIMEOUT           1

BOOL DecodeFaxPageAsync
(
    PThrdGlbl           pTG,
    DWORD               *RetFlags,
    char                *InFileName
);


DWORD PageAckThread(PThrdGlbl pTG)
{
    DWORD             RetCode = FALSE;
    DWORD             RetFlags = 0;
    DWORD             ThrdDoneRetCode;
    char              InFileName[_MAX_FNAME];

    DEBUG_FUNCTION_NAME(_T("PageAckThread"));
    //
    // Set the appropriate PRTY for this thread
    // I/O threads run at 15. TIFF - at 9...11
    //

    if (! SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_HIGHEST) ) 
    {
        DebugPrintEx(   DEBUG_ERR, 
                        "SetThreadPriority HIGHEST failed le=%x",
                        GetLastError());
        goto error_exit;
    }

    // binary file has fixed name based on lineID; it is created and updated by T.30 RX I/O thread.
    _fmemcpy (InFileName, gT30.TmpDirectory, gT30.dwLengthTmpDirectory);
    _fmemcpy (&InFileName[gT30.dwLengthTmpDirectory], pTG->TiffConvertThreadParams.lpszLineID, 8);
    sprintf  (&InFileName[gT30.dwLengthTmpDirectory+8], ".RX");

    do 
    {
        RetFlags = 0;

        ThrdDoneRetCode = DecodeFaxPageAsync (  pTG,
                                                &RetFlags,
                                                InFileName);

        DebugPrintEx(   DEBUG_MSG,
                        "DecodeFaxPageAsync RetFlags=%d",
                        RetFlags);

        if ( RetFlags == RET_NEXT_STRIP_RX_TIMEOUT ) 
        {
            DebugPrintEx(   DEBUG_MSG,
                            "TimeOut. Trying to delete file%s",
                            InFileName);
            
            if (!DeleteFile(InFileName))
            {
                DebugPrintEx(   DEBUG_ERR, 
                                "Could not delete file %s, le = %x",
                                InFileName, 
                                GetLastError());
            }
            return (FALSE);
        }

        // Signal that we finish process the page.
        if (!SetEvent(pTG->ThrdDoneSignal))
        {
            DebugPrintEx(   DEBUG_ERR, 
                            "SetEvent(0x%lx) returns failure code: %ld",
                            (ULONG_PTR)pTG->ThrdDoneSignal,
                            (long) GetLastError());
            RetCode = FALSE;
            goto error_exit;
        }

    } 
    while (! pTG->ReqTerminate); // Handle the next page

    
    if (!DeleteFile(InFileName))
    {
        DebugPrintEx(   DEBUG_ERR, 
                        "Could not delete file %s, le = %x", 
                        InFileName, 
                        GetLastError());
    }
    
    DebugPrintEx(DEBUG_MSG,"Terminated");

    RetCode = TRUE;

error_exit:

    pTG->AckTerminate = 1;
    pTG->fOkToResetAbortReqEvent = 1;

    if (!SetEvent(pTG->ThrdAckTerminateSignal))
    {
        DebugPrintEx(   DEBUG_ERR, 
                        "SetEvent(0x%lx) returns failure code: %ld",
                        (ULONG_PTR)pTG->ThrdAckTerminateSignal,
                        (long) GetLastError());
        RetCode = FALSE;
    }

    DebugPrintEx(DEBUG_MSG,"PageAckThread EXITs");

    return (RetCode);
}


BOOL DecodeFaxPageAsync
(
    PThrdGlbl           pTG,
    DWORD               *RetFlags,
    char                *InFileName
)
{
    HANDLE              InFileHandle;
    DWORD               AllowedBadFaxLines = gT30.MaxErrorLinesPerPage;
    DWORD               AllowedConsecBadLines = gT30.MaxConsecErrorLinesPerPage;

    LPDWORD             EndPtr;
    LPDWORD             EndBuffer;

    LPDWORD             lpdwResPtr;
    LPDWORD             lpdwRead;
    BYTE                ResBit;
    BOOL                fTestLength;
    BOOL                fError;

    BOOL                fFirstRead;

    LPDWORD             lpBuffer=NULL;
    BOOL                fLastReadBlockSync;   // needs to be sync. fetched, updated by RX I/O thrd.
    DWORD               BytesReuse;
    DWORD               BytesDelta;
    DWORD               BytesToRead;
    DWORD               BytesHaveRead;

    BOOL                f1D;

    DWORD               PageCount;
    DWORD               NumHandles=2;
    HANDLE              HandlesArray[2];
    DWORD               WaitResult;
    BOOL                fRet=TRUE;

    //
    // At Start of Page
    //

    DEBUG_FUNCTION_NAME(_T("DecodeFaxPageAsync"));

    HandlesArray[0] = pTG->AbortReqEvent;
    HandlesArray[1] = pTG->ThrdSignal;

    pTG->fTiffThreadRunning = 0;


    do 
    {
        WaitResult = WaitForMultipleObjects(NumHandles, HandlesArray, FALSE, WAIT_FOR_NEXT_STRIP_RX_TIMEOUT);

        if (WaitResult == WAIT_TIMEOUT) 
        {
            *RetFlags = RET_NEXT_STRIP_RX_TIMEOUT;
            return FALSE;
        }

        if (WaitResult == WAIT_FAILED) 
        {
            DebugPrintEx(   DEBUG_ERR, 
                            "WaitForMultipleObjects FAILED le=%lx",
                            GetLastError());
        }

        if ( pTG->ReqTerminate || ( WaitResult == WAIT_OBJECT_0) )   
        {
            DebugPrintEx(DEBUG_MSG, "wait for next page ABORTED") ;
            pTG->fOkToResetAbortReqEvent = 1;
            return TRUE;
        }

    } 
    while (pTG->fPageIsBad); // pTG->fPageIsBad become FALSE when we call to RECV_STARTPAGE to get new page.
    // The reason we wait for fPageIsBad: If the prev page was bad, we want to wait till clean-up was done.
    
    pTG->fTiffThreadRunning = 1;

    pTG->Lines = 0;
    pTG->BadFaxLines = 0;
    pTG->ConsecBadLines = 0;
    pTG->iResScan = 0;

    PageCount = pTG->PageCount;

    fLastReadBlockSync = pTG->fLastReadBlock;

    DebugPrintEx(   DEBUG_MSG, 
                    "waked up fLastReadBlockSync=%d",
                    fLastReadBlockSync);

    if ( ( InFileHandle = CreateFileA(InFileName, GENERIC_READ, FILE_SHARE_WRITE | FILE_SHARE_READ,
                                       NULL, OPEN_EXISTING, 0, NULL) ) == INVALID_HANDLE_VALUE ) 
    {
        DebugPrintEx(   DEBUG_ERR, 
                        "PAGE COULD NOT open %s",
                        InFileName);

        pTG->fPageIsBad = 1;
        return FALSE;
    }

    fFirstRead = 1;
    pTG->BytesOut = 0;

    lpBuffer = MemAlloc(DECODE_BUFFER_SIZE);
    if (!lpBuffer)
    {
        DebugPrintEx(DEBUG_ERR, "MemAlloc failed");
        pTG->fPageIsBad = 1;
        goto bad_exit;
    }

    // lpBuffer is DWORD aligned
    lpdwResPtr = lpBuffer;
    ResBit = 0;

    EndBuffer = lpBuffer + ( DECODE_BUFFER_SIZE / sizeof(DWORD) );

    //
    // loop thru all blocks
    //
    do 
    {
        //
        // Read the next RAW block prepared by main I/O thread
        //
        DWORD tiffCompression;
        BOOL HiRes;
        
        if (fFirstRead) 
        {
            lpdwRead = lpBuffer;
            BytesReuse = 0;
            BytesToRead = DECODE_BUFFER_SIZE;
            fTestLength = DO_NOT_TEST_LENGTH;
        }
        else 
        {
            BytesReuse = (DWORD)((EndBuffer - lpdwResPtr) * sizeof (DWORD));
            CopyMemory( (char *) lpBuffer, (char *) lpdwResPtr, BytesReuse);
            lpdwRead = lpBuffer + (BytesReuse / sizeof (DWORD) );
            BytesToRead = DECODE_BUFFER_SIZE -  BytesReuse;
            fTestLength = DO_TEST_LENGTH;
        }

        lpdwResPtr = lpBuffer;

        BytesDelta = pTG->BytesIn - pTG->BytesOut;

        if (BytesDelta < DECODE_BUFFER_SIZE) 
        {
            if (! fLastReadBlockSync) 
            {
                DebugPrintEx(   DEBUG_ERR, 
                                "PAGE LOGIC. SYNC. file %s Bytes: IN:%d OUT:%d",
                                InFileName, 
                                pTG->BytesIn, 
                                pTG->BytesOut);

                pTG->fPageIsBad = 1;
                goto bad_exit;
            }
        }

        if (fLastReadBlockSync) 
        {
            if (BytesDelta < BytesToRead) 
            {
                BytesToRead = BytesDelta;
            }
        }

        if (! ReadFile(InFileHandle, lpdwRead, BytesToRead, &BytesHaveRead, NULL ) ) 
        {
            DebugPrintEx(   DEBUG_ERR, 
                            "PAGE COULD NOT READ file %s Bytes: IN:%d"
                            " OUT:%d WANTED:%d LE=%x",
                            InFileName, 
                            pTG->BytesIn, 
                            pTG->BytesOut, 
                            BytesToRead, 
                            GetLastError());

            pTG->fPageIsBad = 1;
            goto bad_exit;
        }

        if (BytesHaveRead != BytesToRead) 
        {
            DebugPrintEx(   DEBUG_ERR, 
                            "PAGE ReadFile count=%d WANTED=%d file %s"
                            " Bytes: IN:%d OUT:%d",
                            BytesHaveRead, 
                            BytesToRead, 
                            InFileName, 
                            pTG->BytesIn, 
                            pTG->BytesOut);

            pTG->fPageIsBad = 1;
            goto bad_exit;
        }


        if ( fLastReadBlockSync && (BytesToRead == BytesDelta) ) 
        {
            EndPtr = lpBuffer + ( (BytesReuse + BytesToRead) / sizeof(DWORD) );
        }
        else 
        {
            //
            // leave 1000*4 = 4000 bytes ahead if not final block to make sure
            // we always have one full line ahead.
            //
            EndPtr = EndBuffer - 1000;
        }

        pTG->BytesOut += BytesToRead;

        DebugPrintEx(   DEBUG_MSG, 
                        "BytesIn=%d Out=%d Read=%d ResBit=%d StartPtr=%lx"
                        " EndPtr=%lx Reuse=%d",
                        pTG->BytesIn, 
                        pTG->BytesOut, 
                        BytesToRead, 
                        ResBit, 
                        lpBuffer, 
                        EndPtr, 
                        BytesReuse);

        //
        // find first EOL
        //

        f1D = 1;

        if (! FindNextEol (lpdwResPtr, ResBit, EndBuffer, &lpdwResPtr, &ResBit, fTestLength, &fError) ) 
        {
            DebugPrintEx(   DEBUG_ERR, 
                            "Couldn't find EOL fTestLength=%d fError=%d",
                            fTestLength, 
                            fError);
            pTG->fPageIsBad = 1;
            goto bad_exit;
        }

        //
        // Scan the next segment
        //
        // if those settings change from one page to the other
        // it has to be inside the loop, beause this thread
        // gets all the pages and then dies
        tiffCompression = pTG->TiffConvertThreadParams.tiffCompression;
        HiRes = pTG->TiffConvertThreadParams.HiRes;
        DebugPrintEx(   DEBUG_MSG,
                        "Calling %s with compression=%d and resolution=%d",
                        (tiffCompression == TIFF_COMPRESSION_MR)?"ScanMrSegment":"ScanMhSegment",
                        tiffCompression,HiRes);
        if (tiffCompression == TIFF_COMPRESSION_MR) 
        {
            pTG->iResScan = ScanMrSegment(&lpdwResPtr,
                                    &ResBit,
                                        EndPtr,
                                        EndBuffer,
                                    &(pTG->Lines),
                                    &(pTG->BadFaxLines),
                                    &(pTG->ConsecBadLines),
                                        AllowedBadFaxLines,
                                        AllowedConsecBadLines,
                                    &f1D,
                                    pTG->TiffInfo.ImageWidth);
        }
        else
        {
            pTG->iResScan = ScanMhSegment(&lpdwResPtr,
                                        &ResBit,
                                        EndPtr,
                                        EndBuffer,
                                    &(pTG->Lines),
                                    &(pTG->BadFaxLines),
                                    &(pTG->ConsecBadLines),
                                        AllowedBadFaxLines,
                                        AllowedConsecBadLines,
                                        pTG->TiffInfo.ImageWidth);
        }

        DebugPrintEx( DEBUG_MSG,
                        "%s returned: ResScan=%d  Lines=%d  " 
                        "BadFaxLines=%d  tAllowedBadFaxLines=%d  "
                        "ConsecBadLines=%d  AllowedConsecBadLines=%d  "
                        "tpImageWidth=%d",
                        (tiffCompression == TIFF_COMPRESSION_MR)?"ScanMrSegment":"ScanMhSegment",
                        pTG->iResScan, 
                        pTG->Lines, 
                        pTG->BadFaxLines, 
                        AllowedBadFaxLines, 
                        pTG->ConsecBadLines, 
                        AllowedConsecBadLines, 
                        pTG->TiffInfo.ImageWidth);

        if (pTG->iResScan == TIFF_SCAN_SUCCESS) 
        {
            goto good_exit;
        }
        else if (pTG->iResScan == TIFF_SCAN_FAILURE)
        {
            DebugPrintEx(   DEBUG_ERR,
                            "ScanSegment returns TIFF_SCAN_FAILURE");
            pTG->fPageIsBad = 1;
            goto bad_exit;
        }
        else if (pTG->iResScan != TIFF_SCAN_SEG_END) 
        {
            DebugPrintEx(   DEBUG_ERR,
                            "ScanSegment returns INVALID %d", 
                            pTG->iResScan);
            pTG->fPageIsBad = 1;
            goto bad_exit;
        }

//lNextBlock:
        // here we make decision as to whether to do the next segment OR to block (not enough data avail).
        if (fLastReadBlockSync && (pTG->BytesOut == pTG->BytesIn) ) 
        {
            //
            // The class 2/2.0 standards say the modem is not supposed to include the RTC in the page data.
            // So, for 2/2.0, finishing the page without finding RTC marks the page as good.
            //
            if ((pTG->ModemClass==MODEM_CLASS2) || (pTG->ModemClass==MODEM_CLASS2_0))
            {
                DebugPrintEx(DEBUG_MSG, "Didn't find RTC, but we're on class 2/2.0, so page is good");
                goto good_exit;
            }

            DebugPrintEx(   DEBUG_ERR, 
                            "Didn't find RTC  Bad=%d ConsecBad=%d Good=%d",
                            pTG->BadFaxLines, 
                            pTG->ConsecBadLines, 
                            pTG->Lines);
            pTG->fPageIsBad = 1;
            goto bad_exit;
        }


        DebugPrintEx(   DEBUG_MSG,
                        "Done with next strip BytesIn=%d Out=%d"
                        " Lines=%d Bad=%d ConsecBad=%d Processed %d bytes \n",
                        pTG->BytesIn, 
                        pTG->BytesOut, 
                        pTG->Lines, 
                        pTG->BadFaxLines, 
                        pTG->ConsecBadLines,
                        (lpdwResPtr - lpBuffer) * sizeof(DWORD));

        fLastReadBlockSync = pTG->fLastReadBlock;

        if ( (pTG->BytesIn - pTG->BytesOut < DECODE_BUFFER_SIZE) && (! fLastReadBlockSync) )   
        {
            DebugPrintEx(DEBUG_MSG,"Waiting for next strip to be avail.");

            pTG->fTiffThreadRunning = 0;

            WaitResult = WaitForMultipleObjects(NumHandles, HandlesArray, FALSE, WAIT_FOR_NEXT_STRIP_RX_TIMEOUT);

            if (WaitResult == WAIT_TIMEOUT) 
            {
                *RetFlags = RET_NEXT_STRIP_RX_TIMEOUT;
                goto bad_exit;
            }

            if (WaitResult == WAIT_FAILED) 
            {
                DebugPrintEx(   DEBUG_ERR, 
                                "WaitForMultipleObjects FAILED le=%lx",
                                GetLastError());
            }

            if ( pTG->ReqTerminate || ( WaitResult == WAIT_OBJECT_0) )   
            {
                DebugPrintEx(DEBUG_MSG,"wait for next page ABORTED") ;
                goto bad_exit;
            }

            pTG->fTiffThreadRunning = 1;

            fLastReadBlockSync = pTG->fLastReadBlock;

            DebugPrintEx(   DEBUG_MSG, 
                            "Waked up with next strip. fLastReadBlockSync=%d"
                            " BytesIn=%d Out=%d",
                            fLastReadBlockSync,  
                            pTG->BytesIn, 
                            pTG->BytesOut);

        }

        fFirstRead = 0;

    } 
    while ( ! pTG->ReqTerminate );

    DebugPrintEx(DEBUG_ERR, "Got Terminate request");
    pTG->fPageIsBad = 1;
    // fall through
    
bad_exit:
    fRet=FALSE;
    goto exit;

good_exit:
    fRet=TRUE;
    // fall through

exit:
    CloseHandle(InFileHandle);
    if (lpBuffer)
    {
        MemFree(lpBuffer);
        lpBuffer=NULL;
    }
    return fRet;

}






