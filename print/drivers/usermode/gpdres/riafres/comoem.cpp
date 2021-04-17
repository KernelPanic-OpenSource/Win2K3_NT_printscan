/*++

Copyright (c) 1996-2002  Microsoft Corp. & Ricoh Co., Ltd. All rights reserved.

FILE:           COMOEM.CPP

Abstract:       Necessary COM class definition to Unidrv OEM rendering plugin module.

Environment:    Windows NT Unidrv5 driver

Revision History:
    04/24/1998 -takashim-
        Written the original sample so that it is more C++.
    02/29/2000 -Masatoshi Kubokura-
        Modified for PCL5e plugin from RPDL code.
    11/13/2001 -Masatoshi Kubokura-
        Last Modified for XP inbox.
    02/26/2002 -Masatoshi Kubokura-
        Include strsafe.h.
    03/29/2002 -Masatoshi Kubokura-
        Check NULL pointer at QueryInterface(), EnablePDEV(), and CommandCallback().

--*/

#define INITGUID // for GUID one-time initialization

#include "pdev.h"
#include "names.h"
#ifndef WINNT_40
#include "strsafe.h"        // @Feb/26/2002
#endif // !WINNT_40

// Globals
static HMODULE g_hModule = NULL ;   // DLL module handle
static long g_cComponents = 0 ;     // Count of active components
static long g_cServerLocks = 0 ;    // Count of locks

//
// IOemCB Definition
//

class IOemCB : public IPrintOemUni
{
public:

    //
    // IUnknown methods
    //

    STDMETHODIMP
    QueryInterface(
        const IID& iid, void** ppv)
    {    
        VERBOSE((DLLTEXT("IOemCB: QueryInterface entry\n")));
// @Mar/29/2002 ->
        if (NULL == ppv)
            return E_FAIL;
// @Mar/29/2002 <-
        if (iid == IID_IUnknown)
        {
            *ppv = static_cast<IUnknown*>(this); 
            VERBOSE((DLLTEXT("IOemCB:Return pointer to IUnknown.\n"))); 
        }
        else if (iid == IID_IPrintOemUni)
        {
            *ppv = static_cast<IPrintOemUni*>(this);
            VERBOSE((DLLTEXT("IOemCB:Return pointer to IPrintOemUni.\n"))); 
        }
        else
        {
            *ppv = NULL ;
            VERBOSE((DLLTEXT("IOemCB:Return NULL.\n"))); 
            return E_NOINTERFACE ;
        }
        reinterpret_cast<IUnknown*>(*ppv)->AddRef();
        return S_OK ;
    }

    STDMETHODIMP_(ULONG)
    AddRef()
    {
        VERBOSE((DLLTEXT("IOemCB::AddRef() entry.\n")));
        return InterlockedIncrement(&m_cRef);
    }

    STDMETHODIMP_(ULONG)
    Release()
    {
        VERBOSE((DLLTEXT("IOemCB::Release() entry.\n")));
        if (InterlockedDecrement(&m_cRef) == 0)
        {
            delete this ;
            return 0 ;
        }
        return m_cRef ;
    }

    //
    // IPrintOemCommon methods
    //

    // Function Name: GetInfo
    // Plug-in: Any
    // Driver: Any
    // Type: Mandatory
    //

    STDMETHODIMP
    GetInfo(
        DWORD dwMode,
        PVOID pBuffer,
        DWORD cbSize,
        PDWORD pcbNeeded)
    {
        VERBOSE((DLLTEXT("IOemCB::GetInfo() entry.\n")));

        if (OEMGetInfo(dwMode, pBuffer, cbSize, pcbNeeded))
            return S_OK;
        else
            return E_FAIL;
    }

    //
    // Function Name: DevMode
    // Plug-in: Rendering module
    // Driver: Any
    // Type: Optional
    //

    STDMETHODIMP
    DevMode(
        DWORD       dwMode,
        POEMDMPARAM pOemDMParam) 
    {
        VERBOSE((DLLTEXT("IOemCB::DevMode() entry.\n")));

        if (OEMDevMode(dwMode, pOemDMParam)) {
            return S_OK;
        }
        else {
            return E_FAIL;
        }
    }

    //
    // IPrintOemEngine methods
    //

    //
    // Function Name: EnableDriver
    // Plug-in: Rendering module
    // Driver: Any
    // Type: Optional
    //

    STDMETHODIMP
    EnableDriver(
        DWORD dwDriverVersion,
        DWORD cbSize,
        PDRVENABLEDATA pded)
    {
        VERBOSE((DLLTEXT("IOemCB::EnableDriver() entry.\n")));
//      return E_NOTIMPL;

        OEMEnableDriver(dwDriverVersion, cbSize, pded);

        // Need to return S_OK so that DisableDriver() will be called, which Releases
        // the reference to the Printer Driver's interface.
        return S_OK;
    }

    //
    // Function Name: DisableDriver
    // Plug-in: Rendering module
    // Driver: Any
    // Type: Optional
    //

    STDMETHODIMP
    DisableDriver(VOID)
    {
        VERBOSE((DLLTEXT("IOemCB::DisaleDriver() entry.\n")));
//      return E_NOTIMPL;
        OEMDisableDriver();
        // Release reference to Printer Driver's interface.
        if (this->pOEMHelp)
        {
            this->pOEMHelp->Release();
            this->pOEMHelp = NULL;
        }
        return S_OK;
    }

    //
    // Function Name: EnablePDEV
    // Plug-in: Rendering module
    // Driver: Any
    // Type: Optional
    //

    STDMETHODIMP
    EnablePDEV(
        PDEVOBJ         pdevobj,
        PWSTR           pPrinterName,
        ULONG           cPatterns,
        HSURF          *phsurfPatterns,
        ULONG           cjGdiInfo,
        GDIINFO        *pGdiInfo,
        ULONG           cjDevInfo,
        DEVINFO        *pDevInfo,
        DRVENABLEDATA  *pded,
        OUT PDEVOEM    *pDevOem)
    {
        VERBOSE((DLLTEXT("IOemCB::EnablePDEV() entry.\n")));
//      return E_NOTIMPL;
        PDEVOEM pTemp;

// @Mar/29/2002 ->
        if (NULL == pDevOem)
            return E_FAIL;
// @Mar/29/2002 <-

        pTemp = OEMEnablePDEV(pdevobj, pPrinterName, cPatterns,  phsurfPatterns,
                              cjGdiInfo, pGdiInfo, cjDevInfo, pDevInfo, pded);

        if (NULL == pTemp)
            return E_FAIL;

        //
        // Save necessary helpder function addresses.
        //
//        ((MINIDEV *)pTemp)->pIntf = this->pOEMHelp;

        *pDevOem = pTemp;
        return S_OK;
    }

    //
    // Function Name: DisablePDEV
    // Plug-in: Rendering module
    // Driver: Any
    // Type: Optional
    //

    STDMETHODIMP
    DisablePDEV(
        PDEVOBJ pdevobj)
    {
        LONG lI;

        VERBOSE((DLLTEXT("IOemCB::DisablePDEV() entry.\n")));
//      return E_NOTIMPL;
        OEMDisablePDEV(pdevobj);
        return S_OK;
    }

    //
    // Function Name: ResetPDEV
    // Plug-in: Rendering module
    // Driver: Any
    // Type: Optional
    //

    STDMETHODIMP
    ResetPDEV(
        PDEVOBJ pdevobjOld,
        PDEVOBJ pdevobjNew)
    {
        VERBOSE((DLLTEXT("IOemCB::ResetPDEV() entry.\n")));
//      return E_NOTIMPL;
        if (OEMResetPDEV(pdevobjOld, pdevobjNew))
            return S_OK;
// @Mar/26/2002 (MS) ->
//         else
//             return S_FALSE;
        else
            return E_FAIL;
// @Mar/26/2002 <-
    }

    //
    // IPrintOemUni methods
    //

    //
    // Function Name: PublishDriverInterface
    // Plug-in: Rendering module
    // Driver: Any
    // Type: Mandatory
    //

    STDMETHODIMP
    PublishDriverInterface(
        IUnknown *pIUnknown)
    {
        VERBOSE((DLLTEXT("IOemCB::PublishDriverInterface() entry.\n")));

        // Need to store pointer to Driver Helper functions, if we already haven't.
        if (this->pOEMHelp == NULL)
//          pIUnknown->AddRef();
        {
            HRESULT hResult;

            // Get Interface to Helper Functions.
            hResult = pIUnknown->QueryInterface(IID_IPrintOemDriverUni, (void**)&(this->pOEMHelp));
            if(!SUCCEEDED(hResult))
            {
                // Make sure that interface pointer reflects interface query failure.
                this->pOEMHelp = NULL;
                return E_FAIL;
            }
        }
//      this->pOEMHelp = reinterpret_cast<IPrintOemDriverUni*>(pIUnknown);
        return S_OK;
    }

    //
    // Function Name: GetImplementationMethod
    // Plug-in: Rendering module
    // Driver: Any
    // Type: Mandatory
    //

    //
    // Needed to be static so that it can be passed
    // to the bsearch() as a pointer to a functin.
    //

    static
    int __cdecl
    iCompNames(
        const void *p1,
        const void *p2) {

        return strcmp(
            *((char **)p1),
            *((char **)p2));
    }

    STDMETHODIMP
    GetImplementedMethod(
        PSTR pMethodName)
    {
        LONG lRet = E_NOTIMPL;
        PSTR pTemp;


        VERBOSE((DLLTEXT("IOemCB::GetImplementedMethod() entry.\n")));

        if (NULL != pMethodName) {

            pTemp = (PSTR)bsearch(
                &pMethodName,
                gMethodsSupported,
                (sizeof (gMethodsSupported) / sizeof (PSTR)),
                sizeof (PSTR),
                iCompNames);

            if (NULL != pTemp)
                lRet = S_OK;
        }

        VERBOSE((DLLTEXT("pMethodName = %s, lRet = %d\n"), pMethodName, lRet));

        return lRet;
    }

    //
    // Function Name: CommandCallback
    // Plug-in: Rendering module
    // Driver: Unidrv
    // Type: Optional
    //

    STDMETHODIMP
    CommandCallback(
        PDEVOBJ pdevobj,
        DWORD dwCallbackID,
        DWORD dwCount,
        PDWORD pdwParams,
        OUT INT *piResult)
    {
        VERBOSE((DLLTEXT("IOemCB::CommandCallback() entry.\n")));
// @Mar/29/2002 ->
        if (NULL == piResult)
            return E_FAIL;
// @Mar/29/2002 <-
        *piResult = OEMCommandCallback(pdevobj, dwCallbackID, dwCount, pdwParams);
        return S_OK;
    }

    //
    // Function Name: ImageProcessing
    // Plug-in: Rendering module
    // Driver: Unidrv
    // Type: Optional
    //

    STDMETHODIMP
    ImageProcessing(
        PDEVOBJ             pdevobj,  
        PBYTE               pSrcBitmap,
        PBITMAPINFOHEADER   pBitmapInfoHeader,
        PBYTE               pColorTable,
        DWORD               dwCallbackID,
        PIPPARAMS           pIPParams,
        OUT PBYTE           *ppbResult)
    {
        VERBOSE((DLLTEXT("IOemCB::ImageProcessing() entry.\n")));
        return E_NOTIMPL;
    }

    //
    // Function Name: FilterGraphics
    // Plug-in: Rendering module
    // Driver: Unidrv
    // Type: Optional
    //

    STDMETHODIMP
    FilterGraphics(
        PDEVOBJ pdevobj,
        PBYTE pBuf,
        DWORD dwLen)
    {
        VERBOSE((DLLTEXT("IOemCB::FilterGraphis() entry.\n")));
        return E_NOTIMPL;
    }

    //
    // Function Name: Compression
    // Plug-in: Rendering module
    // Driver: Unidrv
    // Type: Optional
    //

    STDMETHODIMP
    Compression(
        PDEVOBJ     pdevobj,
        PBYTE       pInBuf,
        PBYTE       pOutBuf,
        DWORD       dwInLen,
        DWORD       dwOutLen,
        OUT INT     *piResult)
    {
        VERBOSE((DLLTEXT("IOemCB::Compression() entry.\n")));
        return E_NOTIMPL;
    }

    //
    // Function Name: HalftonePattern
    // Plug-in: Rendering module
    // Driver: Unidrv
    // Type: Optional
    //

    STDMETHODIMP
    HalftonePattern(
        PDEVOBJ     pdevobj,
        PBYTE       pHTPattern,
        DWORD       dwHTPatternX,
        DWORD       dwHTPatternY,
        DWORD       dwHTNumPatterns,
        DWORD       dwCallbackID,
        PBYTE       pResource,
        DWORD       dwResourceSize)
    {
        VERBOSE((DLLTEXT("IOemCB::HalftonePattern() entry.\n")));
#ifdef WINNT_40     // @Nov/13/2001
        return E_NOTIMPL;
#else  // !WINNT_40
// @Sep/28/2001 ->
        if (OEMHalftonePattern(pdevobj, pHTPattern, dwHTPatternX, dwHTPatternY,
                               dwHTNumPatterns, dwCallbackID, pResource, dwResourceSize))
            return S_OK;
        else
            return E_FAIL;
// @Sep/28/2001 <-
#endif // !WINNT_40
    }

    //
    // Function Name: MemoryUsge
    // Plug-in: Rendering module
    // Driver: Unidrv
    // Type: Optional
    //

    STDMETHODIMP
    MemoryUsage(
        PDEVOBJ         pdevobj,   
        POEMMEMORYUSAGE pMemoryUsage)
    {
        VERBOSE((DLLTEXT("IOemCB::MemoryUsage() entry.\n")));
        return E_NOTIMPL;
    }

    //
    // Function Name: DownloadFontHeader
    // Plug-in: Rendering module
    // Driver: Unidrv
    // Type: Optional
    //

    STDMETHODIMP
    DownloadFontHeader(
        PDEVOBJ     pdevobj,
        PUNIFONTOBJ pUFObj,
        OUT DWORD   *pdwResult) 
    {
        VERBOSE((DLLTEXT("IOemCB::DownloadFontHeader() entry.\n")));
        return E_NOTIMPL;
    }

    //
    // Function Name: DownloadCharGlyph
    // Plug-in: Rendering module
    // Driver: Unidrv
    // Type: Optional
    //

    STDMETHODIMP
    DownloadCharGlyph(
        PDEVOBJ     pdevobj,
        PUNIFONTOBJ pUFObj,
        HGLYPH      hGlyph,
        PDWORD      pdwWidth,
        OUT DWORD   *pdwResult) 
    {
        VERBOSE((DLLTEXT("IOemCB::DownloadCharGlyph() entry.\n")));
        return E_NOTIMPL;
    }

    //
    // Function Name: TTDonwloadMethod
    // Plug-in: Rendering module
    // Driver: Unidrv
    // Type: Optional
    //

    STDMETHODIMP
    TTDownloadMethod(
        PDEVOBJ     pdevobj,
        PUNIFONTOBJ pUFObj,
        OUT DWORD   *pdwResult) 
    {
        VERBOSE((DLLTEXT("IOemCB::TTDownloadMethod() entry.\n")));
        return E_NOTIMPL;
    }

    //
    // Function Name: OutputCharStr
    // Plug-in: Rendering module
    // Driver: Unidrv
    // Type: Optional
    //

    STDMETHODIMP
    OutputCharStr(
        PDEVOBJ     pdevobj,
        PUNIFONTOBJ pUFObj,
        DWORD       dwType,
        DWORD       dwCount,
        PVOID       pGlyph) 
    {
        VERBOSE((DLLTEXT("IOemCB::OutputCharStr() entry.\n")));
        return E_NOTIMPL;
    }

    //
    // Function Name: SendFontCmd
    // Plug-in: Rendering module
    // Driver: Unidrv
    // Type: Optional
    //

    STDMETHODIMP
    SendFontCmd(
        PDEVOBJ      pdevobj,
        PUNIFONTOBJ  pUFObj,
        PFINVOCATION pFInv) 
    {
        VERBOSE((DLLTEXT("IOemCB::SendFontCmd() entry.\n")));
        return E_NOTIMPL;
    }

    //
    // Function Name: DriverDMS
    // Plug-in: Rendering module
    // Driver: Unidrv
    // Type: Optional
    //

    STDMETHODIMP
    DriverDMS(
        PVOID   pDevObj,
        PVOID   pBuffer,
        DWORD   cbSize,
        PDWORD  pcbNeeded)
    {
        VERBOSE((DLLTEXT("IOemCB::DriverDMS() entry.\n")));
        return E_NOTIMPL;
    }

    //
    // Function Name: TextOutputAsBitmap
    // Plug-in: Rendering module
    // Driver: Unidrv
    // Type: Optional
    //

    STDMETHODIMP
    TextOutAsBitmap(
        SURFOBJ    *pso,
        STROBJ     *pstro,
        FONTOBJ    *pfo,
        CLIPOBJ    *pco,
        RECTL      *prclExtra,
        RECTL      *prclOpaque,
        BRUSHOBJ   *pboFore,
        BRUSHOBJ   *pboOpaque,
        POINTL     *pptlOrg,
        MIX         mix)
    {
        VERBOSE((DLLTEXT("IOemCB::TextOutAsBitmap() entry.\n")));
        return E_NOTIMPL;
    }

    //
    // Function Name: TTYGetInfo
    // Plug-in: Rendering module
    // Driver: Unidrv
    // Type: Optional
    //

    STDMETHODIMP
    TTYGetInfo(
        PDEVOBJ     pdevobj,
        DWORD       dwInfoIndex,
        PVOID       pOutputBuf,
        DWORD       dwSize,
        DWORD       *pcbcNeeded)
    {
        VERBOSE((DLLTEXT("IOemCB::TTYGetInfo() entry.\n")));
        return E_NOTIMPL;
    }

    //
    // Constructors
    //

    IOemCB() { m_cRef = 1; pOEMHelp = NULL; };
    ~IOemCB() { };

protected:
    IPrintOemDriverUni* pOEMHelp;
    LONG m_cRef;
};

//
// Class factory definition
//

class IOemCF : public IClassFactory
{
public:
    //
    // IUnknown methods
    //

    STDMETHODIMP
    QueryInterface(const IID& iid, void** ppv)
    {
        if ((iid == IID_IUnknown) || (iid == IID_IClassFactory))
        {
            *ppv = static_cast<IOemCF*>(this);
        }
        else
        {
            *ppv = NULL ;
            return E_NOINTERFACE ;
        }
        reinterpret_cast<IUnknown*>(*ppv)->AddRef();
        return S_OK ;
    }

    STDMETHODIMP_(ULONG)
    AddRef()
    {
        return InterlockedIncrement(&m_cRef);
    }

    STDMETHODIMP_(ULONG)
    Release()
    {
        if (InterlockedDecrement(&m_cRef) == 0)
        {
            delete this ;
            return 0 ;
        }
        return m_cRef ;
    }

    //
    // IClassFactory methods
    //

    STDMETHODIMP
    CreateInstance(
        IUnknown *pUnknownOuter,
        const IID &iid,
        void **ppv)
    {
        //VERBOSE((DLLTEXT("IOemCF::CreateInstance() called\n.")));

        // Cannot aggregate.
        if (NULL != pUnknownOuter) {

            return CLASS_E_NOAGGREGATION;
        }

        // Create component.
        IOemCB* pOemCB = new IOemCB;
        if (NULL == pOemCB) {

            return E_OUTOFMEMORY;
        }

        // Get the requested interface.
        HRESULT hr = pOemCB->QueryInterface(iid, ppv);

        // Release the IUnknown pointer.
        // (If QueryInterface failed, component will delete itself.)
        pOemCB->Release();
        return hr ;
    }

    // LockServer
    STDMETHODIMP
    LockServer(BOOL bLock)
    {
        if (bLock)
        {
            InterlockedIncrement(&g_cServerLocks);
        }
        else
        {
            InterlockedDecrement(&g_cServerLocks);
        }
        return S_OK ;
    }

    //
    // Constructor
    //

    IOemCF(): m_cRef(1) { };
    ~IOemCF() { };

protected:
    LONG m_cRef;
};

//
// Export functions
//

//
// Get class factory
//

STDAPI
DllGetClassObject(
    const CLSID &clsid,
    const IID &iid,
    void **ppv)
{
    //VERBOSE((DLLTEXT("DllGetClassObject:\tCreate class factory.")));

    // Can we create this component?
    if (clsid != CLSID_OEMRENDER)
    {
        return CLASS_E_CLASSNOTAVAILABLE ;
    }

    // Create class factory.
    IOemCF* pFontCF = new IOemCF ;  // Reference count set to 1
                                         // in constructor
    if (pFontCF == NULL)
    {
        return E_OUTOFMEMORY ;
    }

    // Get requested interface.
    HRESULT hr = pFontCF->QueryInterface(iid, ppv);
    pFontCF->Release();

    return hr ;
}

//
//
// Can DLL unload now?
//

STDAPI
DllCanUnloadNow()
{
    if ((g_cComponents == 0) && (g_cServerLocks == 0))
    {
        return S_OK;
    }
    else
    {
// @Mar/26/2002 (MS) ->
//         return E_FAIL;
        return S_FALSE;
// @Mar/26/2002 <-
    }
}
