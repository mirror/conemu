
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

struct Far3WrapFunctions
{
	// Non modifiable block begin
	// These members must not be changed in future versions
	size_t StructSize;
	int Far3Build; // Version (build number) of Far3 plugin.hpp
	wchar_t* ErrorInfo; // In-pointer, Out-error text
	int ErrorInfoMax; // max size of ErrorInfo in wchar_t
	struct WrapPluginInfo* wpi;
	const GUID* PluginGuid; // [Out]
	// Non modifiable block end

	_SetStartupInfoWrap SetStartupInfoW;
	_GetGlobalInfoWrap GetGlobalInfoW;
	_GetPluginInfoWrap GetPluginInfoW;
	_OpenWrap OpenW;
	_AnalyseWrap AnalyseW;
	_ClosePanelWrap ClosePanelW;
	_CompareWrap CompareW;
	_ConfigureWrap ConfigureW;
	_DeleteFilesWrap DeleteFilesW;
	_ExitFARWrap ExitFARW;
	_FreeVirtualFindDataWrap FreeVirtualFindDataW;
	_GetFilesWrap GetFilesW;
	_GetFindDataWrap GetFindDataW;
	_FreeFindDataWrap FreeFindDataW;
	_GetOpenPanelInfoWrap GetOpenPanelInfoW;
	_GetVirtualFindDataWrap GetVirtualFindDataW;
	_MakeDirectoryWrap MakeDirectoryW;
	_ProcessDialogEventWrap ProcessDialogEventW;
	_ProcessEditorEventWrap ProcessEditorEventW;
	_ProcessEditorInputWrap ProcessEditorInputW;
	_ProcessEventWrap ProcessEventW;
	_ProcessHostFileWrap ProcessHostFileW;
	_ProcessPanelInputWrap ProcessPanelInputW;
	_ProcessConsoleInputWrap ProcessConsoleInputW;
	_ProcessSynchroEventWrap ProcessSynchroEventW;
	_ProcessViewerEventWrap ProcessViewerEventW;
	_PutFilesWrap PutFilesW;
	_SetDirectoryWrap SetDirectoryW;
	_SetFindListWrap SetFindListW;
};

// Returns 0 on success
typedef int (WINAPI* _InitPlugin)(HMODULE hModule, struct Far3WrapFunctions *Info);
//typedef void (WINAPI* _ExitPlugin)(HMODULE hModule, struct WrapPluginInfo* wpi);
