<?xml version="1.0" encoding="UTF-16"?>
<!DOCTYPE DCARRIER SYSTEM "mantis.dtd" [
<!-- RegKey registry types (map to REG_SZ etc) -->
<!ENTITY RegTypeNone                        "0" >
<!ENTITY RegTypeSz                          "1" >
<!ENTITY RegTypeExpandSz                    "2" >
<!ENTITY RegTypeBinary                      "3" >
<!ENTITY RegTypeDword                       "4" >
<!ENTITY RegTypeDwordBigEndian              "5" >
<!ENTITY RegTypeLink                        "6" >
<!ENTITY RegTypeMultiSz                     "7" >
<!ENTITY RegTypeResourceList                "8" >
<!ENTITY RegTypeFullResourceDescriptor      "9" >
<!ENTITY RegTypeResourceRequirementsList    "10" >
<!ENTITY RegTypeQword                       "11" >

<!-- RegKey registry operations -->
<!ENTITY RegOpWrite               "1" >
<!ENTITY RegOpDelete              "2" >
<!ENTITY RegOpEdit                "3" >

<!-- RegKey registry conditionals -->
<!ENTITY RegCondAlways            "1" >
<!ENTITY RegCondIfExists          "2" >
<!ENTITY RegCondIfNotExists       "3" >

<!-- RawDep dependency types -->
<!ENTITY RawDepNone               "None" >
<!ENTITY RawDepCLSID              "CLSID" >
<!ENTITY RawDepFile               "File" >
<!ENTITY RawDepRegKey             "RegKey" >
<!ENTITY RawDepRegValue           "RegValue" >
<!ENTITY RawDepRegPath            "RegPath" >
]>
<DCARRIER CarrierRevision="1">
	<TOOLINFO ToolName="iCat"><![CDATA[<?xml version="1.0"?>
<!DOCTYPE TOOL SYSTEM "file://mess/icat/tool.dtd">
<TOOL>
	<CREATED><NAME>iCat</NAME><VSGUID>{7e14ebbb-dd3e-46f3-a3cc-e0c6119a5018}</VSGUID><VERSION>1.0.0.230</VERSION><BUILD>230</BUILD></CREATED><LASTSAVED><NAME>iCat</NAME><VSGUID>{97b86ee0-259c-479f-bc46-6cea7ef4be4d}</VSGUID><VERSION>1.0.0.452</VERSION><BUILD>452</BUILD><DATE>7/17/2001</DATE></LASTSAVED></TOOL>
]]></TOOLINFO><COMPONENT ComponentVSGUID="{E89D514D-1118-4939-81F9-A8A8EF2B19F4}" ComponentVIGUID="{C7E6742F-3BB1-4A1F-B43B-C1005637E586}" PlatformGUID="{B784E719-C196-4DDB-B358-D9254426C38D}" RepositoryVSGUID="{8E0BE9ED-7649-47F3-810B-232D36C430B4}" Revision="4" Visibility="1000" MultiInstance="False" Released="1" Editable="True" HTMLFinal="False"><HELPCONTEXT src="S:\nt\printscan\ui\embedded\_wia_ui_component.htm">&lt;!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 3.2 Final//EN"&gt;
&lt;HTML DIR="LTR"&gt;&lt;HEAD&gt;
&lt;META HTTP-EQUIV="Content-Type" Content="text/html; charset=Windows-1252"&gt;
&lt;TITLE&gt;WIA_UI Component&lt;/TITLE&gt;
&lt;style type="text/css"&gt;@import url(td.css);&lt;/style&gt;&lt;/HEAD&gt;
&lt;BODY TOPMARGIN="0"&gt;
&lt;H1&gt;&lt;A NAME="_wia_ui_component"&gt;&lt;/A&gt;&lt;SUP&gt;&lt;/SUP&gt;WIA_UI Component&lt;/H1&gt;

&lt;P&gt;Windows Image Acquisition (WIA) provides both an application programming interface (API) and a device driver interface (DDI). It supports still digital cameras and scanners. It also allows the retrieval of still images from IEEE 1394-based digital video (DV) camcorders and universal serial bus-based (USB) Web cameras. There are two WIA components: WIA_Core and WIA_UI.&lt;/P&gt;

&lt;P&gt;The WIA_UI component includes user interface (UI) wizards for interfacing with scanners and still digital cameras: Explorer UI components, common dialogs, and a photo-printing wizard.&lt;/P&gt;

&lt;P&gt;The still camera dialog allows the user to view thumbnails, preview full-sized pictures, view picture information and delete pictures. The scanner and camera wizards allow users to easily retrieve images from all supported devices. Depending on the device type, the wizard presents the user with device-specific features.&lt;/P&gt;

&lt;P&gt;WIA includes a scripting model, which presents most WIA device features to a scripting based application.&lt;/P&gt;

&lt;P&gt;An application writer has the following four choices for communicating with WIA devices: 

&lt;UL type="disc"&gt;
	&lt;LI&gt;Use the WIA high-level interfaces, which are tied to the WIA common dialogs. This provides a minimal-effort approach.&lt;/li&gt;

	&lt;LI&gt;Program directly to the WIA interfaces. This adds flexibility to implement custom solutions and UI-less mode support for devices.&lt;/li&gt;

	&lt;LI&gt;Use the TWAIN compatibility layer, which exposes WIA devices as TWAIN devices. The compatibility layer is limited to UI-mode only and single-image transfer.&lt;/li&gt;

	&lt;LI&gt;Support the common file open and file insert dialog. This dialog is not supported for scanners, and it is only supported with video cameras after frames have been captured.&lt;/li&gt;
&lt;/UL&gt;

&lt;P&gt;Other imaging features associated with WIA include My Pictures Folder and My Pictures Screensaver. &lt;/P&gt;

&lt;H1&gt;Component Configuration&lt;/H1&gt;

&lt;P&gt;There are no configuration requirements for this component.&lt;/P&gt;

&lt;H1&gt;For More Information &lt;/H1&gt;

&lt;P&gt;Fore more information on WIA, see this &lt;A HREF="http://www.microsoft.com/hwdev/wia"&gt;Microsoft Web site&lt;/A&gt;.&lt;/P&gt;

&lt;/BODY&gt;
&lt;/HTML&gt;
</HELPCONTEXT><DISPLAYNAME>WIA UI</DISPLAYNAME><VERSION>1.1</VERSION><DESCRIPTION>Windows Image Acquisition UI Components</DESCRIPTION><COPYRIGHT>2000 Microsoft Corp.</COPYRIGHT><VENDOR>Microsoft Corp.</VENDOR><OWNERS>ricktu</OWNERS><AUTHORS>vlads;ricktu</AUTHORS><DATECREATED>7/26/2000</DATECREATED><DATEREVISED>7/17/2001</DATEREVISED><RESOURCE Name="File:&quot;%11%&quot;,&quot;wiashext.dll&quot;" ResTypeVSGUID="{E66B49F6-4A35-4246-87E8-5C1A468315B5}" BuildTypeMask="819"><PROPERTY Name="DstPath" Format="String">%11%</PROPERTY><PROPERTY Name="DstName" Format="String">wiashext.dll</PROPERTY><PROPERTY Name="SrcFileSize" Format="Integer">0</PROPERTY><PROPERTY Name="NoExpand" Format="Boolean">0</PROPERTY><DISPLAYNAME>Shell Extension</DISPLAYNAME><DESCRIPTION>Allows WIA devices to be a part of shell namepspace</DESCRIPTION></RESOURCE><RESOURCE Name="File:&quot;%11%&quot;,&quot;wiaacmgr.exe&quot;" ResTypeVSGUID="{E66B49F6-4A35-4246-87E8-5C1A468315B5}" BuildTypeMask="819"><PROPERTY Name="DstPath" Format="String">%11%</PROPERTY><PROPERTY Name="DstName" Format="String">wiaacmgr.exe</PROPERTY><PROPERTY Name="SrcFileSize" Format="Integer">0</PROPERTY><PROPERTY Name="NoExpand" Format="Boolean">0</PROPERTY><DISPLAYNAME>Acquisition Manager</DISPLAYNAME><DESCRIPTION>Wizard which steps users through image acquistion</DESCRIPTION></RESOURCE><RESOURCE Name="File:&quot;%11%&quot;,&quot;camocx.dll&quot;" ResTypeVSGUID="{E66B49F6-4A35-4246-87E8-5C1A468315B5}" BuildTypeMask="819"><PROPERTY Name="DstPath" Format="String">%11%</PROPERTY><PROPERTY Name="DstName" Format="String">camocx.dll</PROPERTY><PROPERTY Name="SrcFileSize" Format="Integer">0</PROPERTY><PROPERTY Name="NoExpand" Format="Boolean">0</PROPERTY><DISPLAYNAME>Camera OCX for webview</DISPLAYNAME><DESCRIPTION>Allows webview script to interact with WIA devices</DESCRIPTION></RESOURCE><RESOURCE Name="File:&quot;%11%&quot;,&quot;wiadefui.dll&quot;" ResTypeVSGUID="{E66B49F6-4A35-4246-87E8-5C1A468315B5}" BuildTypeMask="819"><PROPERTY Name="DstPath" Format="String">%11%</PROPERTY><PROPERTY Name="DstName" Format="String">wiadefui.dll</PROPERTY><PROPERTY Name="SrcFileSize" Format="Integer">0</PROPERTY><PROPERTY Name="NoExpand" Format="Boolean">0</PROPERTY><DISPLAYNAME>WIA Common Dialogs</DISPLAYNAME><DESCRIPTION>Common Dialogs for WIA apps</DESCRIPTION></RESOURCE><RESOURCE Name="File:&quot;%11%&quot;,&quot;ssmypics.scr&quot;" ResTypeVSGUID="{E66B49F6-4A35-4246-87E8-5C1A468315B5}" BuildTypeMask="819"><PROPERTY Name="DstPath" Format="String">%11%</PROPERTY><PROPERTY Name="DstName" Format="String">ssmypics.scr</PROPERTY><PROPERTY Name="SrcFileSize" Format="Integer">0</PROPERTY><PROPERTY Name="NoExpand" Format="Boolean">0</PROPERTY><DISPLAYNAME>My Pictures Screen Saver</DISPLAYNAME><DESCRIPTION>Screen saver that shows images from My Pictures folder</DESCRIPTION></RESOURCE><RESOURCE ResTypeVSGUID="{322D2CA9-219E-4380-989B-12E8A830DFFA}" BuildTypeMask="819" Name="FBRegDLL(819):&quot;%11%\wiashext.dll&quot;" Localize="0"><PROPERTY Name="Arguments" Format="String"></PROPERTY><PROPERTY Name="DLLEntryPoint" Format="String"></PROPERTY><PROPERTY Name="DLLInstall" Format="Boolean">0</PROPERTY><PROPERTY Name="DLLRegister" Format="Boolean">True</PROPERTY><PROPERTY Name="FilePath" Format="String">%11%\wiashext.dll</PROPERTY><PROPERTY Name="Flags" Format="Integer">0</PROPERTY><PROPERTY Name="Timeout" Format="Integer">0</PROPERTY><DISPLAYNAME>register WIASHEXT.DLL</DISPLAYNAME><DESCRIPTION>Call DLLRegisterServer on wiashext.dll</DESCRIPTION></RESOURCE><RESOURCE ResTypeVSGUID="{322D2CA9-219E-4380-989B-12E8A830DFFA}" BuildTypeMask="819" Name="FBRegDLL(819):&quot;%11%\camocx.dll&quot;" Localize="0"><PROPERTY Name="Arguments" Format="String"></PROPERTY><PROPERTY Name="DLLEntryPoint" Format="String"></PROPERTY><PROPERTY Name="DLLInstall" Format="Boolean">0</PROPERTY><PROPERTY Name="DLLRegister" Format="Boolean">True</PROPERTY><PROPERTY Name="FilePath" Format="String">%11%\camocx.dll</PROPERTY><PROPERTY Name="Flags" Format="Integer">0</PROPERTY><PROPERTY Name="Timeout" Format="Integer">0</PROPERTY><DISPLAYNAME>register CAMOCX.DLL</DISPLAYNAME><DESCRIPTION>Call DLLRegister on camocx.dll</DESCRIPTION></RESOURCE><RESOURCE ResTypeVSGUID="{322D2CA9-219E-4380-989B-12E8A830DFFA}" BuildTypeMask="819" Name="FBRegDLL(819):&quot;%11%\wiadefui.dll&quot;" Localize="0"><PROPERTY Name="Arguments" Format="String"></PROPERTY><PROPERTY Name="DLLEntryPoint" Format="String"></PROPERTY><PROPERTY Name="DLLInstall" Format="Boolean">0</PROPERTY><PROPERTY Name="DLLRegister" Format="Boolean">True</PROPERTY><PROPERTY Name="FilePath" Format="String">%11%\wiadefui.dll</PROPERTY><PROPERTY Name="Flags" Format="Integer">0</PROPERTY><PROPERTY Name="Timeout" Format="Integer">0</PROPERTY><DISPLAYNAME>register WIADEFUI.DLL</DISPLAYNAME><DESCRIPTION>Call DLLRegister on wiadefui.dll</DESCRIPTION></RESOURCE><RESOURCE ResTypeVSGUID="{E66B49F6-4A35-4246-87E8-5C1A468315B5}" BuildTypeMask="819" Name="File(819):&quot;%11%&quot;,&quot;wiavideo.dll&quot;"><PROPERTY Name="DstPath" Format="String">%11%</PROPERTY><PROPERTY Name="DstName" Format="String">wiavideo.dll</PROPERTY><PROPERTY Name="NoExpand" Format="Boolean">0</PROPERTY><DISPLAYNAME>WIA Video driver UI component</DISPLAYNAME><DESCRIPTION>WIA Video driver UI component</DESCRIPTION></RESOURCE><RESOURCE ResTypeVSGUID="{E66B49F6-4A35-4246-87E8-5C1A468315B5}" BuildTypeMask="819" Name="File(819):&quot;%11%&quot;,&quot;photowiz.dll&quot;"><PROPERTY Name="DstPath" Format="String">%11%</PROPERTY><PROPERTY Name="DstName" Format="String">photowiz.dll</PROPERTY><PROPERTY Name="NoExpand" Format="Boolean">0</PROPERTY><DISPLAYNAME>Photo Printing Wizard</DISPLAYNAME><DESCRIPTION>Prints out image files in photographic sizes</DESCRIPTION></RESOURCE><RESOURCE ResTypeVSGUID="{322D2CA9-219E-4380-989B-12E8A830DFFA}" BuildTypeMask="819" Name="FBRegDLL(819):&quot;%11%\photowiz.dll&quot;" Localize="0"><PROPERTY Name="Arguments" Format="String"></PROPERTY><PROPERTY Name="DLLEntryPoint" Format="String"></PROPERTY><PROPERTY Name="DLLInstall" Format="Boolean">0</PROPERTY><PROPERTY Name="DLLRegister" Format="Boolean">-1</PROPERTY><PROPERTY Name="FilePath" Format="String">%11%\photowiz.dll</PROPERTY><PROPERTY Name="Flags" Format="Integer">0</PROPERTY><PROPERTY Name="Timeout" Format="Integer">0</PROPERTY><DISPLAYNAME>register PHOTOWIZ.DLL</DISPLAYNAME><DESCRIPTION>Call DLLRegsiter on photowiz.dll</DESCRIPTION></RESOURCE><RESOURCE ResTypeVSGUID="{322D2CA9-219E-4380-989B-12E8A830DFFA}" BuildTypeMask="819" Name="FBRegDLL(819):&quot;%11%\wiavideo.dll&quot;" Localize="1"><PROPERTY Name="Arguments" Format="String"></PROPERTY><PROPERTY Name="DLLEntryPoint" Format="String"></PROPERTY><PROPERTY Name="DLLInstall" Format="Boolean">0</PROPERTY><PROPERTY Name="DLLRegister" Format="Boolean">-1</PROPERTY><PROPERTY Name="FilePath" Format="String">%11%\wiavideo.dll</PROPERTY><PROPERTY Name="Flags" Format="Integer">0</PROPERTY><PROPERTY Name="Timeout" Format="Integer">0</PROPERTY><DISPLAYNAME>register WIAVIDEO.DLL</DISPLAYNAME><DESCRIPTION>Call DLLRegister on wiavideo.dll</DESCRIPTION></RESOURCE><RESOURCE ResTypeVSGUID="{90D8E195-E710-4AF6-B667-B1805FFC9B8F}" BuildTypeMask="819" Name="RawDep(819):&quot;File&quot;,&quot;msvcrt.dll&quot;"><PROPERTY Name="RawType" Format="String">File</PROPERTY><PROPERTY Name="Value" Format="String">msvcrt.dll</PROPERTY></RESOURCE><RESOURCE ResTypeVSGUID="{90D8E195-E710-4AF6-B667-B1805FFC9B8F}" BuildTypeMask="819" Name="RawDep(819):&quot;File&quot;,&quot;ADVAPI32.dll&quot;"><PROPERTY Name="RawType" Format="String">File</PROPERTY><PROPERTY Name="Value" Format="String">ADVAPI32.dll</PROPERTY></RESOURCE><RESOURCE ResTypeVSGUID="{90D8E195-E710-4AF6-B667-B1805FFC9B8F}" BuildTypeMask="819" Name="RawDep(819):&quot;File&quot;,&quot;GDI32.dll&quot;"><PROPERTY Name="RawType" Format="String">File</PROPERTY><PROPERTY Name="Value" Format="String">GDI32.dll</PROPERTY></RESOURCE><RESOURCE ResTypeVSGUID="{90D8E195-E710-4AF6-B667-B1805FFC9B8F}" BuildTypeMask="819" Name="RawDep(819):&quot;File&quot;,&quot;gdiplus.dll&quot;"><PROPERTY Name="RawType" Format="String">File</PROPERTY><PROPERTY Name="Value" Format="String">gdiplus.dll</PROPERTY></RESOURCE><RESOURCE ResTypeVSGUID="{90D8E195-E710-4AF6-B667-B1805FFC9B8F}" BuildTypeMask="819" Name="RawDep(819):&quot;File&quot;,&quot;KERNEL32.dll&quot;"><PROPERTY Name="RawType" Format="String">File</PROPERTY><PROPERTY Name="Value" Format="String">KERNEL32.dll</PROPERTY></RESOURCE><RESOURCE ResTypeVSGUID="{90D8E195-E710-4AF6-B667-B1805FFC9B8F}" BuildTypeMask="819" Name="RawDep(819):&quot;File&quot;,&quot;ole32.dll&quot;"><PROPERTY Name="RawType" Format="String">File</PROPERTY><PROPERTY Name="Value" Format="String">ole32.dll</PROPERTY></RESOURCE><RESOURCE ResTypeVSGUID="{90D8E195-E710-4AF6-B667-B1805FFC9B8F}" BuildTypeMask="819" Name="RawDep(819):&quot;File&quot;,&quot;OLEAUT32.dll&quot;"><PROPERTY Name="RawType" Format="String">File</PROPERTY><PROPERTY Name="Value" Format="String">OLEAUT32.dll</PROPERTY></RESOURCE><RESOURCE ResTypeVSGUID="{90D8E195-E710-4AF6-B667-B1805FFC9B8F}" BuildTypeMask="819" Name="RawDep(819):&quot;File&quot;,&quot;SHELL32.dll&quot;"><PROPERTY Name="RawType" Format="String">File</PROPERTY><PROPERTY Name="Value" Format="String">SHELL32.dll</PROPERTY></RESOURCE><RESOURCE ResTypeVSGUID="{90D8E195-E710-4AF6-B667-B1805FFC9B8F}" BuildTypeMask="819" Name="RawDep(819):&quot;File&quot;,&quot;SHLWAPI.dll&quot;"><PROPERTY Name="RawType" Format="String">File</PROPERTY><PROPERTY Name="Value" Format="String">SHLWAPI.dll</PROPERTY></RESOURCE><RESOURCE ResTypeVSGUID="{90D8E195-E710-4AF6-B667-B1805FFC9B8F}" BuildTypeMask="819" Name="RawDep(819):&quot;File&quot;,&quot;USER32.dll&quot;"><PROPERTY Name="RawType" Format="String">File</PROPERTY><PROPERTY Name="Value" Format="String">USER32.dll</PROPERTY></RESOURCE><RESOURCE ResTypeVSGUID="{90D8E195-E710-4AF6-B667-B1805FFC9B8F}" BuildTypeMask="819" Name="RawDep(819):&quot;File&quot;,&quot;WININET.dll&quot;"><PROPERTY Name="RawType" Format="String">File</PROPERTY><PROPERTY Name="Value" Format="String">WININET.dll</PROPERTY></RESOURCE><RESOURCE ResTypeVSGUID="{90D8E195-E710-4AF6-B667-B1805FFC9B8F}" BuildTypeMask="819" Name="RawDep(819):&quot;File&quot;,&quot;SETUPAPI.dll&quot;"><PROPERTY Name="RawType" Format="String">File</PROPERTY><PROPERTY Name="Value" Format="String">SETUPAPI.dll</PROPERTY></RESOURCE><RESOURCE ResTypeVSGUID="{90D8E195-E710-4AF6-B667-B1805FFC9B8F}" BuildTypeMask="819" Name="RawDep(819):&quot;File&quot;,&quot;STI.dll&quot;"><PROPERTY Name="RawType" Format="String">File</PROPERTY><PROPERTY Name="Value" Format="String">STI.dll</PROPERTY></RESOURCE><RESOURCE ResTypeVSGUID="{90D8E195-E710-4AF6-B667-B1805FFC9B8F}" BuildTypeMask="819" Name="RawDep(819):&quot;File&quot;,&quot;WINMM.dll&quot;"><PROPERTY Name="RawType" Format="String">File</PROPERTY><PROPERTY Name="Value" Format="String">WINMM.dll</PROPERTY></RESOURCE><RESOURCE ResTypeVSGUID="{90D8E195-E710-4AF6-B667-B1805FFC9B8F}" BuildTypeMask="819" Name="RawDep(819):&quot;File&quot;,&quot;MSIMG32.dll&quot;"><PROPERTY Name="RawType" Format="String">File</PROPERTY><PROPERTY Name="Value" Format="String">MSIMG32.dll</PROPERTY></RESOURCE><RESOURCE ResTypeVSGUID="{90D8E195-E710-4AF6-B667-B1805FFC9B8F}" BuildTypeMask="819" Name="RawDep(819):&quot;File&quot;,&quot;COMCTL32.dll&quot;"><PROPERTY Name="RawType" Format="String">File</PROPERTY><PROPERTY Name="Value" Format="String">COMCTL32.dll</PROPERTY></RESOURCE><RESOURCE ResTypeVSGUID="{90D8E195-E710-4AF6-B667-B1805FFC9B8F}" BuildTypeMask="819" Name="RawDep(819):&quot;File&quot;,&quot;ntdll.dll&quot;"><PROPERTY Name="RawType" Format="String">File</PROPERTY><PROPERTY Name="Value" Format="String">ntdll.dll</PROPERTY></RESOURCE><RESOURCE ResTypeVSGUID="{90D8E195-E710-4AF6-B667-B1805FFC9B8F}" BuildTypeMask="819" Name="RawDep(819):&quot;File&quot;,&quot;WINSPOOL.DRV&quot;"><PROPERTY Name="RawType" Format="String">File</PROPERTY><PROPERTY Name="Value" Format="String">WINSPOOL.DRV</PROPERTY></RESOURCE><RESOURCE ResTypeVSGUID="{90D8E195-E710-4AF6-B667-B1805FFC9B8F}" BuildTypeMask="819" Name="RawDep(819):&quot;File&quot;,&quot;urlmon.dll&quot;"><PROPERTY Name="RawType" Format="String">File</PROPERTY><PROPERTY Name="Value" Format="String">urlmon.dll</PROPERTY></RESOURCE><GROUPMEMBER GroupVSGUID="{DE577686-9566-11D4-8E84-00B0D03D27C6}"/></COMPONENT></DCARRIER>
