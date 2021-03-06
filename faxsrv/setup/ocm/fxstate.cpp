//////////////////////////////////////////////////////////////////////////////
//
// File Name:       fxState.cpp
//
// Abstract:        This provides the state routines used in the FaxOCM
//                  code base.
//
// Environment:     Windows XP / User Mode
//
// Copyright (c) 2000 Microsoft Corporation
//
// Revision History:
//
// Date:        Developer:                Comments:
// -----        ----------                ---------
// 21-Mar-2000  Oren Rosenbloom (orenr)   Created file, cleanup routines
//////////////////////////////////////////////////////////////////////////////
#include "faxocm.h"
#pragma hdrstop

///////////////////////////////////////////////////////////////////////////////////////
//  Function: 
//                  prv_IsXPOrDotNetUpgrade
//
//  Purpose:        
//                  This function searches for FXSAPI.DLL in %system32% and if
//                  it exists the function returns TRUE, indicating that this is an
//                  upgrade from XP or .NET Server.
//  Params:
//                  None
//
//  Return Value:
//                  TRUE - This is a XP or .NET upgrade
//                  FALSE - This is W2K, or a failure occured.
//
//  Author:
//                  Mooly Beery (MoolyB) 26-Dec-2001
///////////////////////////////////////////////////////////////////////////////////////
BOOL prv_IsXPOrDotNetUpgrade()
{
    BOOL            bRet                        = FALSE;
    HANDLE          hFind                       = NULL;
    WIN32_FIND_DATA FindFileData                = {0};
    TCHAR           szSystemDirectory[MAX_PATH] = {0};

    DBG_ENTER(TEXT("prv_IsXPOrDotNetUpgrade"),bRet);

    // get the system directory
    if (GetSystemDirectory(szSystemDirectory,MAX_PATH-_tcslen(FAX_API_MODULE_NAME)-1)==0)
    {
        VERBOSE( SETUP_ERR,TEXT("GetSystemDirectory failed (ec: %ld)"),GetLastError());
        goto exit;
    }

    // append
    _tcscat(szSystemDirectory,_T("\\"));
    _tcscat(szSystemDirectory,FAX_API_MODULE_NAME);

    // search for FXSAPI.DLL in the system folder
    hFind = FindFirstFile(szSystemDirectory, &FindFileData);
    if (hFind==INVALID_HANDLE_VALUE) 
    {
        VERBOSE( DBG_WARNING, 
                 TEXT("FindFirstFile %s failed (ec: %ld)"),
                 szSystemDirectory,
                 GetLastError());
        goto exit;
    }

    // found it.
    bRet = TRUE;
    FindClose(hFind);

exit:
    return bRet;
}

///////////////////////////////////////////////////////////////////////////////////////
//  Function: 
//                  prv_IsRepair
//
//  Purpose:        
//                  This function determines whether this is a 'same build' upgrade
//                  which means the user wants to perform a 'repair' operation.
//                  This is done by comparing the values for SKU and Version we
//					write each setup with current values.
//					A 'repair' is an upgrade from same build to itself without
//					the SKU changing.
//
//  Params:
//                  None
//
//  Return Value:
//                  TRUE - Upgrading from same build to itself and SKU didn't change.
//                  FALSE - Otherwise.
//
//  Author:
//                  Mooly Beery (MoolyB) 06-Jan-2002
///////////////////////////////////////////////////////////////////////////////////////
BOOL prv_IsRepair()
{
    BOOL				bRet                    = FALSE;
	PRODUCT_SKU_TYPE	InstalledProductSKU		= PRODUCT_SKU_UNKNOWN;
	DWORD				InstalledProductBuild  = 0;

    DBG_ENTER(TEXT("prv_IsXPOrDotNetUpgrade"),bRet);

	faxocm_GetProductInfo(&InstalledProductSKU,&InstalledProductBuild);

	if (InstalledProductSKU!=GetProductSKU())
	{
		VERBOSE(DBG_MSG,_T("Different SKU upgrade, this is not repair"));
		return FALSE;
	}

	if (InstalledProductBuild!=GetProductBuild())
	{
		VERBOSE(DBG_MSG,_T("Different build upgrade, this is not repair"));
		return FALSE;
	}

	return TRUE;
}
///////////////////////////////
// fxState_Init
//
// Initialize the state handling
// module for Faxocm.
//
// Params:
//      - void
// Returns:
//      - NO_ERROR on success.
//      - error code otherwise.
//
DWORD fxState_Init(void)
{
    DWORD dwRes = NO_ERROR;
    DBG_ENTER(_T("Init State module"),dwRes);

    return dwRes;
}

///////////////////////////////
// fxState_Term
//
// Terminate the state handling module
// 
// Params:
//      - void.
// Returns:
//      - NO_ERROR on success.
//      - error code otherwise.
//
DWORD fxState_Term(void)
{
    DWORD dwRes = NO_ERROR;
    DBG_ENTER(_T("Term State module"),dwRes);

    return dwRes;
}

///////////////////////////////
// fxState_IsUnattended
//
// Determines if this is an unattended install
// It interprets flags given to us
// by OC Manager.
//
// Params:
//      - void.
// Returns:
//      - TRUE if unattended install.
//      - FALSE if not.
//
BOOL fxState_IsUnattended(void)
{
    DWORDLONG   dwlFlags = 0;

    // get the setup flags.
    dwlFlags = faxocm_GetComponentFlags();

    // if SETOP_BATCH flag is set, then we are in unattended mode.
    return (dwlFlags & SETUPOP_BATCH) ? TRUE : FALSE;
}

///////////////////////////////
// fxState_IsCleanInstall
//
// Determines if this a clean install.
// A clean install is when we are 
// NOT upgrading, and we are not 
// running in stand alone mode (see below for def'n).
//
// Params:
//      - void.
// Returns:
//      - TRUE if clean install.
//      - FALSE if not.
//
BOOL fxState_IsCleanInstall(void)
{
    BOOL        bClean   = FALSE;    

    // a clean install is if we are NOT upgrading AND we are not in
    // stand alone mode.
    if (!fxState_IsUpgrade() && !fxState_IsStandAlone())
    {
        bClean = TRUE;
    }

    return bClean;
}

///////////////////////////////
// fxState_IsStandAlone
//
// Determines if we are running in
// standalone mode or not.  We are
// in this mode if the user started 
// us up via "sysocmgr.exe" found
// in %systemroot%\system32, as opposed
// to via the install setup, or the
// Add/Remove Windows Components.
//
// Params:
//      - void.
// Returns: 
//      - TRUE if we are in stand alone mode.
//      - FALSE if we are not.
//
BOOL fxState_IsStandAlone(void)
{
    DWORDLONG dwlFlags = 0;

    dwlFlags = faxocm_GetComponentFlags();

    return ((dwlFlags & SETUPOP_STANDALONE) ? TRUE : FALSE);
}

///////////////////////////////
// fxState_IsUpgrade
//
// Determines if we are upgrading
// the OS, as opposed to a clean
// installation.
//
// Params:
//      - void.
// Returns:
//      - fxState_UpgradeType_e enumerated
//        type indicating the type of upgrade.
//        (i.e. are we upgrading from Win9X,
//         W2K, etc).
//
fxState_UpgradeType_e fxState_IsUpgrade(void)
{
    fxState_UpgradeType_e   eUpgradeType = FXSTATE_UPGRADE_TYPE_NONE;
    DWORDLONG               dwlFlags     = 0;
	static BOOL				bXpDotNetUpgrade = prv_IsXPOrDotNetUpgrade();
	static BOOL             bIsRepair = prv_IsRepair();

    dwlFlags = faxocm_GetComponentFlags();

    if ((dwlFlags & SETUPOP_WIN31UPGRADE) == SETUPOP_WIN31UPGRADE)
    {
        eUpgradeType = FXSTATE_UPGRADE_TYPE_WIN31;
    }
    else if ((dwlFlags & SETUPOP_WIN95UPGRADE) == SETUPOP_WIN95UPGRADE)
    {
        eUpgradeType = FXSTATE_UPGRADE_TYPE_WIN9X;
    }
    else if ((dwlFlags & SETUPOP_NTUPGRADE) == SETUPOP_NTUPGRADE)
    {
        if (bXpDotNetUpgrade)
        {
			if (bIsRepair)
			{
				eUpgradeType = FXSTATE_UPGRADE_TYPE_REPAIR;
			}
			else
			{
				eUpgradeType = FXSTATE_UPGRADE_TYPE_XP_DOT_NET;
			}
        }
        else
        {
            eUpgradeType = FXSTATE_UPGRADE_TYPE_W2K;
        }
    }

    return eUpgradeType;
}

///////////////////////////////
// fxState_IsOsServerBeingInstalled
//
// Are we installing the Server
// version of the OS, or a workstation
// or personal version.
//
// Params:
//      - void.
// Returns:
//      - TRUE if we are installing a server version.
//      - FALSE if we are not.

BOOL fxState_IsOsServerBeingInstalled(void)
{
    BOOL  bIsServerInstall  = FALSE;
    DWORD dwProductType     = 0;

    dwProductType = faxocm_GetProductType();

    if (dwProductType == PRODUCT_WORKSTATION)
    {
        bIsServerInstall = FALSE;
    }
    else
    {
        bIsServerInstall = TRUE;
    }

    return bIsServerInstall;
}

///////////////////////////////
// fxState_GetInstallType
//
// This function returns one
// of the INF_KEYWORD_INSTALLTYPE_*
// constants found in 
// fxconst.h/fxconst.cpp
//
// Params:
//      - pszCurrentSection - section we are installing from
// Returns:
//      - ptr to one of INF_KEYWORD_INSTALLTYPE_* constants.
//
const TCHAR* fxState_GetInstallType(const TCHAR* pszCurrentSection)
{
    DWORD dwErr                 = NO_ERROR;
    BOOL  bInstall              = TRUE;
    BOOL  bSelectionHasChanged  = FALSE;

    DBG_ENTER(_T("fxState_GetInstallType"),_T("%s"),pszCurrentSection);

    if (pszCurrentSection == NULL)
    {
        ::SetLastError(ERROR_INVALID_PARAMETER);
        return NULL;
    }

    // determine if we are installing or uninstalling
    dwErr = faxocm_HasSelectionStateChanged(pszCurrentSection, 
                                            &bSelectionHasChanged,
                                            &bInstall, 
                                            NULL);

    if (dwErr != NO_ERROR)
    {
        VERBOSE(SETUP_ERR,
                _T("faxocm_HasSelectionStateChanged failed, rc = 0x%lx"),
                dwErr);

        return NULL;
    }

    // we expect the INF to look something like this:
    // [Fax]
    //
    // FaxCleanInstall     = Fax.CleanInstall
    // FaxUpgradeFromWin9x = Fax.UpgradeFromWin9x
    // FaxUninstall        = Fax.Uninstall
    //
    // [Fax.CleanInstall]
    // CopyFiles = ...
    // etc.
    //
    // Thus the goal of this function is to determine if we are
    // clean installing, upgrading, etc., and then get the section
    // name pointed to by one of 'FaxCleanInstall', 'FaxUpgradeFromWin9x'
    // , or 'FaxUninstall'.
    //
    // So for example, if we determined we are clean installing, then
    // this function will find the "FaxCleanInstall" keyword, and then
    // return "Fax.CleanInstall" in the 'pszSectionToProcess' buffer.

    if (bInstall)
    {
        fxState_UpgradeType_e eUpgrade = FXSTATE_UPGRADE_TYPE_NONE;

        if (fxState_IsCleanInstall())
        {
            // we are a clean install of the OS, user is not upgrading from
            // another OS, they are installing a clean version of the OS.
            return INF_KEYWORD_INSTALLTYPE_CLEAN;
        }
        else if (fxState_IsUpgrade())
        {
            // We are installing as an Upgrade to another OS.
            // Determine which OS we are upgrading, then determine
            // the type of install to perform.

            eUpgrade = fxState_IsUpgrade();

            switch (eUpgrade)
            {
                case FXSTATE_UPGRADE_TYPE_NONE:
                    return INF_KEYWORD_INSTALLTYPE_CLEAN;
                break;

                case FXSTATE_UPGRADE_TYPE_WIN9X:
                    return INF_KEYWORD_INSTALLTYPE_UPGFROMWIN9X;
                break;

                case FXSTATE_UPGRADE_TYPE_W2K:
                    return INF_KEYWORD_INSTALLTYPE_UPGFROMWIN2K;
                break;

                case FXSTATE_UPGRADE_TYPE_XP_DOT_NET:
                    return INF_KEYWORD_INSTALLTYPE_UPGFROMXPDOTNET;

				case FXSTATE_UPGRADE_TYPE_REPAIR:
					return INF_KEYWORD_INSTALLTYPE_CLEAN;

                default:
                    VERBOSE(SETUP_ERR, 
                            _T("Failed to get section to process "),
                            _T("for install.  Upgrade Type = %lu"),
                            eUpgrade);
                break;
            }
        }
        else if (fxState_IsStandAlone())
        {
            // we are being run from SysOcMgr.exe.
            // SysOcMgr.exe is either invoked from the command line 
            // (usually as a way to test new OCM components - not really in 
            // the retail world), or it is invoked by the Add/Remove 
            // Windows Components in control panel.  In either case, 
            // treat it as a clean install.

            return INF_KEYWORD_INSTALLTYPE_CLEAN;
        }
    }
    else
    {
        return INF_KEYWORD_INSTALLTYPE_UNINSTALL;
    }

    return NULL;
}


///////////////////////////////
// fxState_DumpSetupState
//
// Dumps to debug the state we
// are running in.
//
// Params:
//      void
// Returns:
//      void
//
//
void fxState_DumpSetupState(void)
{
    DWORD				dwExpectedOCManagerVersion		= 0;
    DWORD				dwCurrentOCManagerVersion		= 0;
    TCHAR				szComponentID[255 + 1]			= {0};
    TCHAR				szSourcePath[_MAX_PATH + 1]		= {0};
    TCHAR				szUnattendFile[_MAX_PATH + 1]	= {0};
	PRODUCT_SKU_TYPE	InstalledProductSKU				= PRODUCT_SKU_UNKNOWN;
	DWORD				InstalledProductBuild			= 0;

    DBG_ENTER(_T("fxState_DumpSetupState"));

    faxocm_GetComponentID(szComponentID, 
                          sizeof(szComponentID) / sizeof(TCHAR));

    faxocm_GetComponentSourcePath(szSourcePath, 
                                  sizeof(szSourcePath) / sizeof(TCHAR));

    faxocm_GetComponentUnattendFile(szUnattendFile, 
                                  sizeof(szUnattendFile) / sizeof(TCHAR));

    faxocm_GetVersionInfo(&dwExpectedOCManagerVersion,
                          &dwCurrentOCManagerVersion);

	faxocm_GetProductInfo(&InstalledProductSKU,&InstalledProductBuild);

    VERBOSE(DBG_MSG,
            _T("IsCleanInstall: '%lu'"), 
            fxState_IsCleanInstall());

    VERBOSE(DBG_MSG,
            _T("IsStandAlone: '%lu'"), 
            fxState_IsStandAlone());

    VERBOSE(DBG_MSG,
            _T("IsUpgrade (0 = No, 1 = Win31, 2 = Win9X, 3 = Win2K, 4 = XP/.NET, 5=Repair: '%lu'"), 
            fxState_IsUpgrade());

	if ((fxState_IsUpgrade()==FXSTATE_UPGRADE_TYPE_XP_DOT_NET) ||
		(fxState_IsUpgrade()==FXSTATE_UPGRADE_TYPE_REPAIR))
	{
		VERBOSE(DBG_MSG,
				_T("Upgrading from Fax build %d"), 
				InstalledProductBuild);

		VERBOSE(DBG_MSG,
				_T("Upgrading from OS SKU %s"), 
				StringFromSKU(InstalledProductSKU));

	}

    VERBOSE(DBG_MSG,
            _T("IsUnattended: '%lu'"), 
            fxState_IsUnattended());

    VERBOSE(DBG_MSG, _T("ComponentID: '%s'"), szComponentID);
    VERBOSE(DBG_MSG, _T("Source Path: '%s'"), szSourcePath);
    VERBOSE(DBG_MSG, _T("Unattend File: '%s'"), szUnattendFile);

    VERBOSE(DBG_MSG,
            _T("Expected OC Manager Version: 0x%lx"),
            dwExpectedOCManagerVersion);

    VERBOSE(DBG_MSG,
            _T("Current OC Manager Version:  0x%lx"),
            dwCurrentOCManagerVersion);

    return;
}