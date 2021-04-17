/*++

Copyright (C) 1999- Microsoft Corporation

Module Name:

    ptputil.h

Abstract:

    This module declares PTP data manipulating utility functions

Author:

    William Hsieh (williamh) created

Revision History:


--*/

#ifndef PTPUTIL__H_
#define PTPUTIL__H_

//
// Time conversion functions
//
HRESULT PtpTime2SystemTime(CBstr *pptpTime, SYSTEMTIME *pSystemTime);
HRESULT SystemTime2PtpTime(SYSTEMTIME *pSystemTime, CBstr *pptpTime, BOOL bTwoDigitsForMilliseconds);

//
// Functions that dump a PTP structure to the log file
//
VOID    DumpCommand(PTP_COMMAND *pCommand, DWORD NumParams);
VOID    DumpResponse(PTP_RESPONSE *pResponse);
VOID    DumpEvent(PTP_EVENT *pEvent);
VOID    DumpGuid(GUID *pGuid);

//
// Macro for getting size of array
//
#ifndef ARRAYSIZE
#define ARRAYSIZE(x) (sizeof(x) / sizeof(x[0]))
#endif

//
// Class for reading registry entries
//
class CPTPRegistry
{
public:
    CPTPRegistry() :
        m_hKey(NULL)
    {
    }

    ~CPTPRegistry()
    {
        if (m_hKey)
            RegCloseKey(m_hKey);
    }

    HRESULT Open(HKEY hkAncestor, LPCTSTR KeyName, REGSAM Access = KEY_READ);
    HRESULT GetValueStr(LPCTSTR ValueName, TCHAR *string, DWORD *pcbStringBytes);
    HRESULT GetValueDword(LPCTSTR ValueName, DWORD *pValue);
    HRESULT GetValueCodes(LPCTSTR ValueName, CArray16 *pCodeArray);

private:
    HKEY    m_hKey;
};

#endif // PTPUTIL__H_
