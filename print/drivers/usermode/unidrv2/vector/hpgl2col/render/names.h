/*++

Copyright (c) 1999-2001  Microsoft Corporation
All rights reserved.

Module Name:

    names.h

Abstract:

    Contains the definitions for the hook function names and IDs.

Environment:

    Windows NT Unidrv driver

Revision History:

    04/07/97 -zhanw-
        Created it.

--*/

#ifndef NAMES_H
#define NAMES_H

#ifdef __cplusplus
extern "C" {
#endif

////////////////////////////////////////////////////////////////////////////////
// OEM COM function interface

CONST CHAR pstrGetInfo[]                = "GetInfo";
CONST CHAR pstrDevMode[]                = "DevMode";
CONST CHAR pstrPublishDriverInterface[] = "PublishDriverInterface";
CONST CHAR pstrGetImplementedMethod[]   = "GetImplementedMethod";
CONST CHAR pstrCommonUIProp[]           = "PropCommonUIProp";
CONST CHAR pstrDocumentPropertySheets[] = "DocumentPropertySheets";
CONST CHAR pstrDevicePropertySheets[]   = "SheetsDevicePropertySheets";
CONST CHAR pstrDevQueryPrintEx[]        = "DevQueryPrintEx";
CONST CHAR pstrDeviceCapabilities[]     = "DeviceCapabilities";
CONST CHAR pstrUpgradePrinter[]         = "UpgradePrinter";
CONST CHAR pstrPrinterEvent[]           = "PrinterEvent";
CONST CHAR pstrDriverEvent[]            = "DriverEvent";
CONST CHAR pstrQueryColorProfile[]      = "QueryColorProfile";
CONST CHAR pstrUpgradeRegistry[]        = "UpgradeRegistry";
CONST CHAR pstrFontInstallerDlgProc[]   = "FontInstallerDlgProc";
CONST CHAR pstrGetDriverSetting[]       = "GetDriverSetting";
CONST CHAR pstrUpgradeRegistrySetting[] = "UpgradeRegistrySetting";
CONST CHAR pstrUpdateUISetting[]        = "UpdateUISetting";
CONST CHAR pstrEnableDriver[]           = "EnableDriver";
CONST CHAR pstrDisableDriver[]          = "DiableDriver";
CONST CHAR pstrEnablePDEV[]             = "EnablePDEV";
CONST CHAR pstrDisablePDEV[]            = "DisablePDEV";
CONST CHAR pstrResetPDEV[]              = "ResetPDEV";
CONST CHAR pstrDrvGetDriverSetting[]    = "DrvGetDriverSetting";
CONST CHAR pstrDrvWriteSpoolBuf[]       = "DrvWriteSpoolBuf";
CONST CHAR pstrDriverDMS[]              = "DriverDMS";
CONST CHAR pstrCommandCallback[]        = "CommandCallback";
CONST CHAR pstrImageProcessing[]        = "ImageProcessing";
CONST CHAR pstrFilterGraphics[]         = "FilterGraphics";
CONST CHAR pstrCompression[]            = "Compression";
CONST CHAR pstrHalftonePattern[]        = "HalftonePattern";
CONST CHAR pstrMemoryUsage[]            = "MemoryUsage";
CONST CHAR pstrDownloadFontHeader[]     = "DownloadFontHeader";
CONST CHAR pstrDownloadCharGlyph[]      = "DownloadCharGlyph";
CONST CHAR pstrTTDownloadMethod[]       = "TTDownloadMethod";
CONST CHAR pstrOutputCharStr[]          = "OutputCharStr";
CONST CHAR pstrSendFontCmd[]            = "SendFontCmd";
CONST CHAR pstrTextOutAsBitmap[]        = "TextOutAsBitmap";
CONST CHAR pstrGetDDIHooks[]            = "GetDDIHooks";
CONST CHAR pstrTTYGetInfo[]             = "TTYGetInfo";

#ifdef __cplusplus
}
#endif

#endif
