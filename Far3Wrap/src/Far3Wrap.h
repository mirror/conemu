
#pragma once

struct WrapPluginInfo;

typedef void (WINAPI* _SetStartupInfoWrap)(struct WrapPluginInfo* wpi, PluginStartupInfo *Info);
typedef void (WINAPI* _GetGlobalInfoWrap)(struct WrapPluginInfo* wpi, GlobalInfo *Info);
typedef void (WINAPI* _GetPluginInfoWrap)(struct WrapPluginInfo* wpi, PluginInfo *Info);
typedef HANDLE (WINAPI* _OpenWrap)(struct WrapPluginInfo* wpi, const OpenInfo *Info);
typedef int (WINAPI* _AnalyseWrap)(struct WrapPluginInfo* wpi, const AnalyseInfo *Info);
typedef void (WINAPI* _ClosePanelWrap)(struct WrapPluginInfo* wpi, HANDLE hPanel);
typedef int (WINAPI* _CompareWrap)(struct WrapPluginInfo* wpi, const CompareInfo *Info);
typedef int (WINAPI* _ConfigureWrap)(struct WrapPluginInfo* wpi, const GUID* Guid);
typedef int (WINAPI* _DeleteFilesWrap)(struct WrapPluginInfo* wpi, const DeleteFilesInfo *Info);
typedef void (WINAPI* _ExitFARWrap)(struct WrapPluginInfo* wpi);
typedef void (WINAPI* _FreeVirtualFindDataWrap)(struct WrapPluginInfo* wpi, const FreeFindDataInfo *Info);
typedef int (WINAPI* _GetFilesWrap)(struct WrapPluginInfo* wpi, GetFilesInfo *Info);
typedef int (WINAPI* _GetFindDataWrap)(struct WrapPluginInfo* wpi, GetFindDataInfo *Info);
typedef void (WINAPI* _FreeFindDataWrap)(struct WrapPluginInfo* wpi, const FreeFindDataInfo *Info);
typedef void (WINAPI* _GetOpenPanelInfoWrap)(struct WrapPluginInfo* wpi, OpenPanelInfo *Info);
typedef int (WINAPI* _GetVirtualFindDataWrap)(struct WrapPluginInfo* wpi, GetVirtualFindDataInfo *Info);
typedef int (WINAPI* _MakeDirectoryWrap)(struct WrapPluginInfo* wpi, MakeDirectoryInfo *Info);
typedef int (WINAPI* _ProcessDialogEventWrap)(struct WrapPluginInfo* wpi, int Event,void *Param);
typedef int (WINAPI* _ProcessEditorEventWrap)(struct WrapPluginInfo* wpi, int Event,void *Param);
typedef int (WINAPI* _ProcessEditorInputWrap)(struct WrapPluginInfo* wpi, const ProcessEditorInputInfo *Rec);
typedef int (WINAPI* _ProcessEventWrap)(struct WrapPluginInfo* wpi, HANDLE hPanel,int Event,void *Param);
typedef int (WINAPI* _ProcessHostFileWrap)(struct WrapPluginInfo* wpi, const ProcessHostFileInfo *Info);
typedef int (WINAPI* _ProcessPanelInputWrap)(struct WrapPluginInfo* wpi, HANDLE hPanel,const struct ProcessPanelInputInfo *Rec);
typedef int (WINAPI* _ProcessConsoleInputWrap)(struct WrapPluginInfo* wpi, struct ProcessConsoleInputInfo *Info);
typedef int (WINAPI* _ProcessSynchroEventWrap)(struct WrapPluginInfo* wpi, int Event,void *Param);
typedef int (WINAPI* _ProcessViewerEventWrap)(struct WrapPluginInfo* wpi, int Event,void *Param);
typedef int (WINAPI* _PutFilesWrap)(struct WrapPluginInfo* wpi, const PutFilesInfo *Info);
typedef int (WINAPI* _SetDirectoryWrap)(struct WrapPluginInfo* wpi, const SetDirectoryInfo *Info);
typedef int (WINAPI* _SetFindListWrap)(struct WrapPluginInfo* wpi, const SetFindListInfo *Info);
typedef int (WINAPI* _GetCustomDataWrap)(struct WrapPluginInfo* wpi, const wchar_t *FilePath, wchar_t **CustomData);
typedef void (WINAPI* _FreeCustomDataWrap)(struct WrapPluginInfo* wpi, wchar_t *CustomData);


typedef LONG_PTR (WINAPI* _FarApiDefDlgProcWrap)(WrapPluginInfo* wpi, HANDLE hDlg, int Msg, int Param1, LONG_PTR Param2);
typedef LONG_PTR (WINAPI* _FarApiSendDlgMessageWrap)(WrapPluginInfo* wpi, HANDLE hDlg, int Msg, int Param1, LONG_PTR Param2);
typedef BOOL (WINAPI* _FarApiShowHelpWrap)(WrapPluginInfo* wpi, const wchar_t *ModuleName, const wchar_t *Topic, DWORD Flags);
typedef HANDLE (WINAPI* _FarApiSaveScreenWrap)(WrapPluginInfo* wpi, int X1, int Y1, int X2, int Y2);
typedef void (WINAPI* _FarApiRestoreScreenWrap)(WrapPluginInfo* wpi, HANDLE hScreen);
typedef void (WINAPI* _FarApiTextWrap)(WrapPluginInfo* wpi, int X, int Y, int Color, const wchar_t *Str);
typedef int (WINAPI* _FarApiEditorWrap)(WrapPluginInfo* wpi, const wchar_t *FileName, const wchar_t *Title, int X1, int Y1, int X2, int Y2, DWORD Flags, int StartLine, int StartChar, UINT CodePage);
typedef int (WINAPI* _FarApiViewerWrap)(WrapPluginInfo* wpi, const wchar_t *FileName, const wchar_t *Title, int X1, int Y1, int X2, int Y2, DWORD Flags, UINT CodePage);
typedef int (WINAPI* _FarApiMenuWrap)(WrapPluginInfo* wpi, INT_PTR PluginNumber, int X, int Y, int MaxHeight, DWORD Flags, const wchar_t* Title, const wchar_t* Bottom, const wchar_t* HelpTopic, const int* BreakKeys, int* BreakCode, const struct Far2::FarMenuItem *Item, int ItemsNumber);
typedef int (WINAPI* _FarApiMessageWrap)(WrapPluginInfo* wpi, INT_PTR PluginNumber, DWORD Flags, const wchar_t *HelpTopic, const wchar_t * const *Items, int ItemsNumber, int ButtonsNumber);
typedef HANDLE (WINAPI* _FarApiDialogInitWrap)(WrapPluginInfo* wpi, INT_PTR PluginNumber, int X1, int Y1, int X2, int Y2, const wchar_t* HelpTopic, struct Far2::FarDialogItem *Item, unsigned int ItemsNumber, DWORD Reserved, DWORD Flags, Far2::FARWINDOWPROC DlgProc, LONG_PTR Param);
typedef int (WINAPI* _FarApiDialogRunWrap)(WrapPluginInfo* wpi, HANDLE hDlg);
typedef void (WINAPI* _FarApiDialogFreeWrap)(WrapPluginInfo* wpi, HANDLE hDlg);
typedef int (WINAPI* _FarApiControlWrap)(WrapPluginInfo* wpi, HANDLE hPlugin, int Command, int Param1, LONG_PTR Param2);
typedef int (WINAPI* _FarApiGetDirListWrap)(WrapPluginInfo* wpi, const wchar_t *Dir, struct Far2::FAR_FIND_DATA **pPanelItem, int *pItemsNumber);
typedef void (WINAPI* _FarApiFreeDirListWrap)(WrapPluginInfo* wpi, struct Far2::FAR_FIND_DATA *PanelItem, int nItemsNumber);
typedef int (WINAPI* _FarApiGetPluginDirListWrap)(WrapPluginInfo* wpi, INT_PTR PluginNumber, HANDLE hPlugin, const wchar_t *Dir, struct Far2::PluginPanelItem **pPanelItem, int *pItemsNumber);
typedef void (WINAPI* _FarApiFreePluginDirListWrap)(WrapPluginInfo* wpi, struct Far2::PluginPanelItem *PanelItem, int nItemsNumber);
typedef int (WINAPI* _FarApiCmpNameWrap)(WrapPluginInfo* wpi, const wchar_t *Pattern, const wchar_t *String, int SkipPath);
typedef LPCWSTR (WINAPI* _FarApiGetMsgWrap)(WrapPluginInfo* wpi, INT_PTR PluginNumber, int MsgId);
typedef INT_PTR (WINAPI* _FarApiAdvControlWrap)(WrapPluginInfo* wpi, INT_PTR ModuleNumber, int Command, void *Param);
typedef int (WINAPI* _FarApiViewerControlWrap)(WrapPluginInfo* wpi, int Command, void *Param);
typedef int (WINAPI* _FarApiEditorControlWrap)(WrapPluginInfo* wpi, int Command, void *Param);
typedef int (WINAPI* _FarApiInputBoxWrap)(WrapPluginInfo* wpi, const wchar_t *Title, const wchar_t *SubTitle, const wchar_t *HistoryName, const wchar_t *SrcText, wchar_t *DestText, int   DestLength, const wchar_t *HelpTopic, DWORD Flags);
typedef int (WINAPI* _FarApiPluginsControlWrap)(WrapPluginInfo* wpi, HANDLE hHandle, int Command, int Param1, LONG_PTR Param2);
typedef int (WINAPI* _FarApiFileFilterControlWrap)(WrapPluginInfo* wpi, HANDLE hHandle, int Command, int Param1, LONG_PTR Param2);
typedef int (WINAPI* _FarApiRegExpControlWrap)(WrapPluginInfo* wpi, HANDLE hHandle, int Command, LONG_PTR Param);
typedef int (WINAPI* _FarStdGetFileOwnerWrap)(WrapPluginInfo* wpi, const wchar_t *Computer,const wchar_t *Name,wchar_t *Owner,int Size);
typedef int (WINAPI* _FarStdGetPathRootWrap)(WrapPluginInfo* wpi, const wchar_t *Path,wchar_t *Root, int DestSize);
typedef wchar_t* (WINAPI* _FarStdXlat)(WrapPluginInfo* wpi, wchar_t *Line,int StartPos,int EndPos,DWORD Flags);
typedef void (WINAPI* _FarStdRecursiveSearchWrap)(WrapPluginInfo* wpi, const wchar_t *InitDir,const wchar_t *Mask,Far2::FRSUSERFUNC Func,DWORD Flags,void *Param);
typedef int (WINAPI* _FarStdMkTempWrap)(WrapPluginInfo* wpi, wchar_t *Dest, DWORD size, const wchar_t *Prefix);
typedef int (WINAPI* _FarStdProcessNameWrap)(WrapPluginInfo* wpi, const wchar_t *param1, wchar_t *param2, DWORD size, DWORD flags);
typedef int (WINAPI* _FarStdMkLinkWrap)(WrapPluginInfo* wpi, const wchar_t *Src,const wchar_t *Dest,DWORD Flags);
typedef int (WINAPI* _FarConvertPathWrap)(WrapPluginInfo* wpi, enum Far2::CONVERTPATHMODES Mode, const wchar_t *Src, wchar_t *Dest, int DestSize);
typedef int (WINAPI* _FarGetReparsePointInfoWrap)(WrapPluginInfo* wpi, const wchar_t *Src, wchar_t *Dest,int DestSize);
typedef DWORD (WINAPI* _FarGetCurrentDirectoryWrap)(WrapPluginInfo* wpi, DWORD Size,wchar_t* Buffer);


struct Far3WrapFunctions
{
	// Non modifiable block begin
	// These members must not be changed in future versions
	// [In]
	size_t StructSize;
	HMODULE hLoader; // Loader.dll
	HMODULE hFar3Wrap; // Far3Wrap.dll
	wchar_t* ErrorInfo; // In-pointer, Out-error text
	int ErrorInfoMax; // max size of ErrorInfo in wchar_t
	int Far3Build; // Version (build number) of Far3 plugin.hpp
	// [Out]
	struct WrapPluginInfo* wpi;
	const GUID* PluginGuid;
	// Non modifiable block end

	// [In] Wrapper Far Api functions
	Far2::FARAPIADVCONTROL FarApiAdvControl;
	Far2::FARAPICMPNAME FarApiCmpName;
	Far2::FARAPICONTROL FarApiControl;
	Far2::FARAPIDEFDLGPROC FarApiDefDlgProc;
	Far2::FARAPIDIALOGFREE FarApiDialogFree;
	Far2::FARAPIDIALOGINIT FarApiDialogInit;
	Far2::FARAPIDIALOGRUN FarApiDialogRun;
	Far2::FARAPIEDITOR FarApiEditor;
	Far2::FARAPIEDITORCONTROL FarApiEditorControl;
	Far2::FARAPIFILEFILTERCONTROL FarApiFileFilterControl;
	Far2::FARAPIFREEDIRLIST FarApiFreeDirList;
	Far2::FARAPIFREEPLUGINDIRLIST FarApiFreePluginDirList;
	Far2::FARAPIGETDIRLIST FarApiGetDirList;
	Far2::FARAPIGETMSG FarApiGetMsg;
	Far2::FARAPIGETPLUGINDIRLIST FarApiGetPluginDirList;
	Far2::FARAPIINPUTBOX FarApiInputBox;
	Far2::FARAPIMENU FarApiMenu;
	Far2::FARAPIMESSAGE FarApiMessage;
	Far2::FARAPIPLUGINSCONTROL FarApiPluginsControl;
	Far2::FARAPIREGEXPCONTROL FarApiRegExpControl;
	Far2::FARAPIRESTORESCREEN FarApiRestoreScreen;
	Far2::FARAPISAVESCREEN FarApiSaveScreen;
	Far2::FARAPISENDDLGMESSAGE FarApiSendDlgMessage;
	Far2::FARAPISHOWHELP FarApiShowHelp;
	Far2::FARAPITEXT FarApiText;
	Far2::FARAPIVIEWER FarApiViewer;
	Far2::FARAPIVIEWERCONTROL FarApiViewerControl;
	Far2::FARCONVERTPATH FarConvertPath;
	Far2::FARGETCURRENTDIRECTORY FarGetCurrentDirectory;
	Far2::FARGETREPARSEPOINTINFO FarGetReparsePointInfo;
	Far2::FARSTDGETFILEOWNER FarStdGetFileOwner;
	Far2::FARSTDGETPATHROOT FarStdGetPathRoot;
	Far2::FARSTDMKLINK FarStdMkLink;
	Far2::FARSTDMKTEMP FarStdMkTemp;
	Far2::FARSTDPROCESSNAME FarStdProcessName;
	Far2::FARSTDRECURSIVESEARCH FarStdRecursiveSearch;
	Far2::FARSTDXLAT FarStdXlat;

	// [Out] Wrapper exported functions 
	_SetStartupInfoWrap SetStartupInfoWrap;
	_GetGlobalInfoWrap GetGlobalInfoWrap;
	_GetPluginInfoWrap GetPluginInfoWrap;
	_OpenWrap OpenWrap;
	_AnalyseWrap AnalyseWrap;
	_ClosePanelWrap ClosePanelWrap;
	_CompareWrap CompareWrap;
	_ConfigureWrap ConfigureWrap;
	_DeleteFilesWrap DeleteFilesWrap;
	_ExitFARWrap ExitFARWrap;
	_FreeVirtualFindDataWrap FreeVirtualFindDataWrap;
	_GetFilesWrap GetFilesWrap;
	_GetFindDataWrap GetFindDataWrap;
	_FreeFindDataWrap FreeFindDataWrap;
	_GetOpenPanelInfoWrap GetOpenPanelInfoWrap;
	_GetVirtualFindDataWrap GetVirtualFindDataWrap;
	_MakeDirectoryWrap MakeDirectoryWrap;
	_ProcessDialogEventWrap ProcessDialogEventWrap;
	_ProcessEditorEventWrap ProcessEditorEventWrap;
	_ProcessEditorInputWrap ProcessEditorInputWrap;
	_ProcessEventWrap ProcessEventWrap;
	_ProcessHostFileWrap ProcessHostFileWrap;
	_ProcessPanelInputWrap ProcessPanelInputWrap;
	_ProcessConsoleInputWrap ProcessConsoleInputWrap;
	_ProcessSynchroEventWrap ProcessSynchroEventWrap;
	_ProcessViewerEventWrap ProcessViewerEventWrap;
	_PutFilesWrap PutFilesWrap;
	_SetDirectoryWrap SetDirectoryWrap;
	_SetFindListWrap SetFindListWrap;
	_GetCustomDataWrap GetCustomDataWrap;
	_FreeCustomDataWrap FreeCustomDataWrap;

	// [Out] Wrapper Far Api functions
	_FarApiDefDlgProcWrap FarApiDefDlgProcWrap;
	_FarApiSendDlgMessageWrap FarApiSendDlgMessageWrap;
	_FarApiShowHelpWrap FarApiShowHelpWrap;
	_FarApiSaveScreenWrap FarApiSaveScreenWrap;
	_FarApiRestoreScreenWrap FarApiRestoreScreenWrap;
	_FarApiTextWrap FarApiTextWrap;
	_FarApiEditorWrap FarApiEditorWrap;
	_FarApiViewerWrap FarApiViewerWrap;
	_FarApiMenuWrap FarApiMenuWrap;
	_FarApiMessageWrap FarApiMessageWrap;
	_FarApiDialogInitWrap FarApiDialogInitWrap;
	_FarApiDialogRunWrap FarApiDialogRunWrap;
	_FarApiDialogFreeWrap FarApiDialogFreeWrap;
	_FarApiControlWrap FarApiControlWrap;
	_FarApiGetDirListWrap FarApiGetDirListWrap;
	_FarApiFreeDirListWrap FarApiFreeDirListWrap;
	_FarApiGetPluginDirListWrap FarApiGetPluginDirListWrap;
	_FarApiFreePluginDirListWrap FarApiFreePluginDirListWrap;
	_FarApiCmpNameWrap FarApiCmpNameWrap;
	_FarApiGetMsgWrap FarApiGetMsgWrap;
	_FarApiAdvControlWrap FarApiAdvControlWrap;
	_FarApiViewerControlWrap FarApiViewerControlWrap;
	_FarApiEditorControlWrap FarApiEditorControlWrap;
	_FarApiInputBoxWrap FarApiInputBoxWrap;
	_FarApiPluginsControlWrap FarApiPluginsControlWrap;
	_FarApiFileFilterControlWrap FarApiFileFilterControlWrap;
	_FarApiRegExpControlWrap FarApiRegExpControlWrap;
	_FarStdGetFileOwnerWrap FarStdGetFileOwnerWrap;
	_FarStdGetPathRootWrap FarStdGetPathRootWrap;
	_FarStdXlat FarStdXlatWrap;
	_FarStdRecursiveSearchWrap FarStdRecursiveSearchWrap;
	_FarStdMkTempWrap FarStdMkTempWrap;
	_FarStdProcessNameWrap FarStdProcessNameWrap;
	_FarStdMkLinkWrap FarStdMkLinkWrap;
	_FarConvertPathWrap FarConvertPathWrap;
	_FarGetReparsePointInfoWrap FarGetReparsePointInfoWrap;
	_FarGetCurrentDirectoryWrap FarGetCurrentDirectoryWrap;
};

// Returns 0 on success
typedef int (WINAPI* _InitPlugin)(struct Far3WrapFunctions *Info);
