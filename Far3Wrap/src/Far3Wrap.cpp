//TODO: При ошибках загрузки плагина не ругаться сразу (MessageBox), а запомнить текст ошибки, 
// и пометить плагин ошибочным в списке плагинов (например, "RQP - Wrapper Failed")

/*
Copyright (c) 2011 Maximus5
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ''AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include <windows.h>
#include <rpc.h>
#include <TCHAR.h>
#include <map>
#include <crtdbg.h>

#define ZeroStruct(s) memset(&s, 0, sizeof(s))

#define LOG_COMMANDS
#ifdef LOG_COMMANDS
	class LogCmd
	{
		public:
		LPCWSTR pszInfo;
		wchar_t szFile[64];
		public:
		void Dump(LPCWSTR asFmt)
		{
			wchar_t szFull[1024];
			SYSTEMTIME st; GetLocalTime(&st); DWORD nTID = GetCurrentThreadId();
			wsprintf(szFull, asFmt, szFile, nTID, st.wHour, st.wMinute, st.wSecond, pszInfo);
			OutputDebugString(szFull);
		};
		LogCmd(LPCWSTR asFunc, LPWSTR asFile)
		{
			pszInfo = asFunc;
			lstrcpyn(szFile, asFile, ARRAYSIZE(szFile));
			Dump(L"%s:T%u(%u:%02u:%02u) %s\n");
		}
		~LogCmd()
		{
			Dump(L"%s:T%u(%u:%02u:%02u) -end- %s\n");
		};
	};
	#define LOG_CMD(f,a1,a2,a3) \
		wchar_t szInfo[512]; wsprintf(szInfo, f, a1,a2,a3); \
		LogCmd llLogCmd(szInfo, wpi ? wpi->File : L"<wpi==NULL>");
	#define LOG_CMD0(f,a1,a2,a3)
#else
	#define LOG_CMD(f,a1,a2,a3)
	#define LOG_CMD0(f,a1,a2,a3)
#endif

#define _FAR_NO_NAMELESS_UNIONS

namespace Far2
{
//int gi1 = 0;
#undef __PLUGIN_HPP__
#include "pluginW.hpp"
#undef __FARKEYS_HPP__
#include "farkeys.hpp"
//int gi2 = 0;
};

//namespace Far3
//{
//int gi1 = 0;
#undef __PLUGIN_HPP__
#include "pluginW3.hpp"
#undef __FARKEYS_HPP__
#include "farkeys3.hpp"
//int gi2 = 0;
//};


#include <malloc.h>
#include <TChar.h>


GUID guid_DefPlugin = { /* 3a446422-cc89-4d74-87f9-a5e0db2c4486 */                                                  
    0x3a446422,                                                                                               
    0xcc89,                                                                                                   
    0x4d74,                                                                                                   
    {0x87, 0xf9, 0xa5, 0xe0, 0xdb, 0x2c, 0x44, 0x86}                                                          
};

GUID guid_DefPluginMenu = { /* d96d46a6-3a38-435c-ad74-a70cfd1719a7 */                                                  
    0xd96d46a6,                                                                                               
    0x3a38,                                                                                                   
    0x435c,                                                                                                   
    {0xad, 0x74, 0xa7, 0x0c, 0xfd, 0x17, 0x19, 0xa7}                                                          
};

GUID guid_DefPluginDiskMenu = { /* 925bb55c-edf6-47f8-aed9-1aa66fbf0d69 */
    0x925bb55c,
    0xedf6,
    0x47f8,
    {0xae, 0xd9, 0x1a, 0xa6, 0x6f, 0xbf, 0x0d, 0x69}
};

GUID guid_DefPluginConfigMenu = { /* 7c0d344e-8030-49f0-81df-f776ef95dde0 */                                                  
    0x7c0d344e,                                                                                               
    0x8030,                                                                                                   
    0x49f0,                                                                                                   
    {0x81, 0xdf, 0xf7, 0x76, 0xef, 0x95, 0xdd, 0xe0}                                                          
};

GUID guid_DefDialogs = { /* c0f2566f-358e-4c81-9947-55cf8bebe103 */
    0xc0f2566f,
    0x358e,
    0x4c81,
    {0x99, 0x47, 0x55, 0xcf, 0x8b, 0xeb, 0xe1, 0x03}
};


#define InvalidOp()

HMODULE ghInstance = NULL;

//#if defined(__GNUC__)
//extern "C"{
//	BOOL   WINAPI DllMain( HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved );
//	int    WINAPI AnalyseW(const AnalyseInfo *Info);
//	void   WINAPI ClosePanelW(HANDLE hPanel);
//	int    WINAPI CompareW(const CompareInfo *Info);
//	int    WINAPI ConfigureW(const GUID* Guid);
//	int    WINAPI DeleteFilesW(const DeleteFilesInfo *Info);
//	void   WINAPI ExitFARW(void);
//	void   WINAPI FreeFindDataW(const FreeFindDataInfo *Info);
//	void   WINAPI FreeVirtualFindDataW(const FreeFindDataInfo *Info);
//	int    WINAPI GetFilesW(GetFilesInfo *Info);
//	int    WINAPI GetFindDataW(GetFindDataInfo *Info);
//	void   WINAPI GetGlobalInfoW(GlobalInfo *Info);
//	void   WINAPI GetOpenPanelInfoW(OpenPanelInfo *Info);
//	void   WINAPI GetPluginInfoW(PluginInfo *Info);
//	int    WINAPI GetVirtualFindDataW(GetVirtualFindDataInfo *Info);
//	int    WINAPI MakeDirectoryW(MakeDirectoryInfo *Info);
//	HANDLE WINAPI OpenW(const OpenInfo *Info);
//	int    WINAPI ProcessDialogEventW(int Event,void *Param);
//	int    WINAPI ProcessEditorEventW(int Event,void *Param);
//	int    WINAPI ProcessEditorInputW(const INPUT_RECORD *Rec);
//	int    WINAPI ProcessEventW(HANDLE hPanel,int Event,void *Param);
//	int    WINAPI ProcessHostFileW(const ProcessHostFileInfo *Info);
//	int    WINAPI ProcessKeyW(HANDLE hPanel,const INPUT_RECORD *Rec);
//	int    WINAPI ProcessSynchroEventW(int Event,void *Param);
//	int    WINAPI ProcessViewerEventW(int Event,void *Param);
//	int    WINAPI PutFilesW(const PutFilesInfo *Info);
//	int    WINAPI SetDirectoryW(const SetDirectoryInfo *Info);
//	int    WINAPI SetFindListW(const SetFindListInfo *Info);
//	void   WINAPI SetStartupInfoW(const PluginStartupInfo *Info);
//};
//#endif

//void f1()
//{
//	gi1 = 0;
//}

BOOL lbPsi2 = FALSE;
PluginStartupInfo psi3;
FarStandardFunctions FSF3;
Far2::PluginStartupInfo psi2;
Far2::FarStandardFunctions FSF2;

struct Far2Dialog;
struct WrapPluginInfo;

struct WrapPluginInfo
{
	HMODULE hDll;
	wchar_t PluginDll[MAX_PATH+1], IniFile[MAX_PATH+1];
	wchar_t File[64];
	wchar_t Title[MAX_PATH+1], Desc[256], Author[256];
	wchar_t RegRoot[1024];
	VersionInfo Version;
	GUID guid_Plugin, guid_Dialogs;
	int nPluginMenu; GUID* guids_PluginMenu;
	int nPluginDisks; GUID* guids_PluginDisks;
	int nPluginConfig; GUID* guids_PluginConfig;
	Far2::PluginInfo Info;
	InfoPanelLine *InfoLines; size_t InfoLinesNumber; // Far3
	PanelMode     *PanelModesArray; size_t PanelModesNumber; // Far3
	KeyBarTitles   KeyBar; // Far3
	KeyBarLabel    KeyBarLabels[7*12];
	Far2::FarList m_ListItems2;
	FarList m_ListItems3;

	AnalyseInfo* pAnalyze;
	//;; 1 - (AnalyzeW [ & OpenW]) -> OpenFilePluginW
	//;; 2 - (AnalyzeW [ & OpenW]) -> OpenFilePluginW & ClosePluginW [& OpenFilePluginW]
	int AnalyzeMode;

	int OldPutFilesParams;

	std::map<PluginPanelItem*,Far2::PluginPanelItem*> MapPanelItems;

	Far2Dialog* LastFar2Dlg;
	std::map<Far2Dialog*,HANDLE> MapDlg_2_3;
	std::map<HANDLE,Far2Dialog*> MapDlg_3_2;


	WrapPluginInfo();
	~WrapPluginInfo();

	void UnloadPlugin();
	void ClearProcAddr();

	KeyBarTitles* KeyBarTitles_2_3(const Far2::KeyBarTitles* KeyBar);


	// Changed functions
	static LONG_PTR WINAPI FarApiDefDlgProc(HANDLE hDlg, int Msg, int Param1, LONG_PTR Param2);
	static LONG_PTR WINAPI FarApiSendDlgMessage(HANDLE hDlg, int Msg, int Param1, LONG_PTR Param2);
	static BOOL WINAPI FarApiShowHelp(const wchar_t *ModuleName, const wchar_t *Topic, DWORD Flags);
	static HANDLE WINAPI FarApiSaveScreen(int X1, int Y1, int X2, int Y2);
	static void WINAPI FarApiRestoreScreen(HANDLE hScreen);
	static void WINAPI FarApiText(int X, int Y, int Color, const wchar_t *Str);
	static int WINAPI FarApiEditor(const wchar_t *FileName, const wchar_t *Title, int X1, int Y1, int X2, int Y2, DWORD Flags, int StartLine, int StartChar, UINT CodePage);
	static int WINAPI FarApiViewer(const wchar_t *FileName, const wchar_t *Title, int X1, int Y1, int X2, int Y2, DWORD Flags, UINT CodePage);
	static int WINAPI FarApiMenu(INT_PTR PluginNumber, int X, int Y, int MaxHeight, DWORD Flags, const wchar_t* Title, const wchar_t* Bottom, const wchar_t* HelpTopic, const int* BreakKeys, int* BreakCode, const struct Far2::FarMenuItem *Item, int ItemsNumber);
	static int WINAPI FarApiMessage(INT_PTR PluginNumber, DWORD Flags, const wchar_t *HelpTopic, const wchar_t * const *Items, int ItemsNumber, int ButtonsNumber);
	static HANDLE WINAPI FarApiDialogInit(INT_PTR PluginNumber, int X1, int Y1, int X2, int Y2, const wchar_t* HelpTopic, struct Far2::FarDialogItem *Item, unsigned int ItemsNumber, DWORD Reserved, DWORD Flags, Far2::FARWINDOWPROC DlgProc, LONG_PTR Param);
	static int WINAPI FarApiDialogRun(HANDLE hDlg);
	static void WINAPI FarApiDialogFree(HANDLE hDlg);
	static int WINAPI FarApiControl(HANDLE hPlugin, int Command, int Param1, LONG_PTR Param2);
	static int WINAPI FarApiGetDirList(const wchar_t *Dir, struct Far2::FAR_FIND_DATA **pPanelItem, int *pItemsNumber);
	static void WINAPI FarApiFreeDirList(struct Far2::FAR_FIND_DATA *PanelItem, int nItemsNumber);
	static int WINAPI FarApiGetPluginDirList(INT_PTR PluginNumber, HANDLE hPlugin, const wchar_t *Dir, struct Far2::PluginPanelItem **pPanelItem, int *pItemsNumber);
	static void WINAPI FarApiFreePluginDirList(struct Far2::PluginPanelItem *PanelItem, int nItemsNumber);
	static int WINAPI FarApiCmpName(const wchar_t *Pattern, const wchar_t *String, int SkipPath);
	static const wchar_t* WINAPI FarApiGetMsg(INT_PTR PluginNumber, int MsgId);
	static INT_PTR WINAPI FarApiAdvControl(INT_PTR ModuleNumber, int Command, void *Param);
	static int WINAPI FarApiViewerControl(int Command, void *Param);
	static int WINAPI FarApiEditorControl(int Command, void *Param);
	static int WINAPI FarApiInputBox(const wchar_t *Title, const wchar_t *SubTitle, const wchar_t *HistoryName, const wchar_t *SrcText, wchar_t *DestText, int   DestLength, const wchar_t *HelpTopic, DWORD Flags);
	static int WINAPI FarApiPluginsControl(HANDLE hHandle, int Command, int Param1, LONG_PTR Param2);
	static int WINAPI FarApiFileFilterControl(HANDLE hHandle, int Command, int Param1, LONG_PTR Param2);
	static int WINAPI FarApiRegExpControl(HANDLE hHandle, int Command, LONG_PTR Param);
	//struct int WINAPIV FARSTDSPRINTF(wchar_t *Buffer,const wchar_t *Format,...);
	//struct int WINAPIV FARSTDSNPRINTF(wchar_t *Buffer,size_t Sizebuf,const wchar_t *Format,...);
	//struct int WINAPIV FARSTDSSCANF(const wchar_t *Buffer, const wchar_t *Format,...);
	//struct void WINAPI FARSTDQSORT(void *base, size_t nelem, size_t width, int (__cdecl *fcmp)(const void *, const void *));
	//struct void WINAPI FARSTDQSORTEX(void *base, size_t nelem, size_t width, int (__cdecl *fcmp)(const void *, const void *,void *userparam),void *userparam);
	//struct void   *WINAPI FARSTDBSEARCH(const void *key, const void *base, size_t nelem, size_t width, int (__cdecl *fcmp)(const void *, const void *));
	static int WINAPI FarStdGetFileOwner(const wchar_t *Computer,const wchar_t *Name,wchar_t *Owner,int Size);
	//struct int WINAPI FARSTDGETNUMBEROFLINKS(const wchar_t *Name);
	//struct int WINAPI FARSTDATOI(const wchar_t *s);
	//struct __int64 WINAPI FARSTDATOI64(const wchar_t *s);
	//struct wchar_t   *WINAPI FARSTDITOA64(__int64 value, wchar_t *string, int radix);
	//struct wchar_t   *WINAPI FARSTDITOA(int value, wchar_t *string, int radix);
	//struct wchar_t   *WINAPI FARSTDLTRIM(wchar_t *Str);
	//struct wchar_t   *WINAPI FARSTDRTRIM(wchar_t *Str);
	//struct wchar_t   *WINAPI FARSTDTRIM(wchar_t *Str);
	//struct wchar_t   *WINAPI FARSTDTRUNCSTR(wchar_t *Str,int MaxLength);
	//struct wchar_t   *WINAPI FARSTDTRUNCPATHSTR(wchar_t *Str,int MaxLength);
	//struct wchar_t   *WINAPI FARSTDQUOTESPACEONLY(wchar_t *Str);
	//struct const wchar_t*WINAPI FARSTDPOINTTONAME(const wchar_t *Path);
	static int WINAPI FarStdGetPathRoot(const wchar_t *Path,wchar_t *Root, int DestSize);
	//struct BOOL WINAPI FARSTDADDENDSLASH(wchar_t *Path);
	//struct int WINAPI FARSTDCOPYTOCLIPBOARD(const wchar_t *Data);
	//struct wchar_t *WINAPI FARSTDPASTEFROMCLIPBOARD(void);
	//struct int WINAPI FARSTDINPUTRECORDTOKEY(const INPUT_RECORD *r);
	//struct int WINAPI FARSTDLOCALISLOWER(wchar_t Ch);
	//struct int WINAPI FARSTDLOCALISUPPER(wchar_t Ch);
	//struct int WINAPI FARSTDLOCALISALPHA(wchar_t Ch);
	//struct int WINAPI FARSTDLOCALISALPHANUM(wchar_t Ch);
	//struct wchar_t WINAPI FARSTDLOCALUPPER(wchar_t LowerChar);
	//struct wchar_t WINAPI FARSTDLOCALLOWER(wchar_t UpperChar);
	//struct void WINAPI FARSTDLOCALUPPERBUF(wchar_t *Buf,int Length);
	//struct void WINAPI FARSTDLOCALLOWERBUF(wchar_t *Buf,int Length);
	//struct void WINAPI FARSTDLOCALSTRUPR(wchar_t *s1);
	//struct void WINAPI FARSTDLOCALSTRLWR(wchar_t *s1);
	//struct int WINAPI FARSTDLOCALSTRICMP(const wchar_t *s1,const wchar_t *s2);
	//struct int WINAPI FARSTDLOCALSTRNICMP(const wchar_t *s1,const wchar_t *s2,int n);
	static wchar_t* WINAPI FarStdXlat(wchar_t *Line,int StartPos,int EndPos,DWORD Flags);
	static void WINAPI FarStdRecursiveSearch(const wchar_t *InitDir,const wchar_t *Mask,Far2::FRSUSERFUNC Func,DWORD Flags,void *Param);
	static int WINAPI FarStdMkTemp(wchar_t *Dest, DWORD size, const wchar_t *Prefix);
	static int WINAPI FarStdProcessName(const wchar_t *param1, wchar_t *param2, DWORD size, DWORD flags);
	static int WINAPI FarStdMkLink(const wchar_t *Src,const wchar_t *Dest,DWORD Flags);
	static int WINAPI FarConvertPath(enum Far2::CONVERTPATHMODES Mode, const wchar_t *Src, wchar_t *Dest, int DestSize);
	static int WINAPI FarGetReparsePointInfo(const wchar_t *Src, wchar_t *Dest,int DestSize);
	static DWORD WINAPI FarGetCurrentDirectory(DWORD Size,wchar_t* Buffer);


	// Exported Function
	typedef int    (WINAPI* _AnalyseW)(const struct AnalyseInfo *Info);
	_AnalyseW AnalyseW;
	typedef void   (WINAPI* _ClosePluginW)(HANDLE hPlugin);
	_ClosePluginW ClosePluginW;
	typedef int    (WINAPI* _CompareW)(HANDLE hPlugin,const Far2::PluginPanelItem *Item1,const Far2::PluginPanelItem *Item2,unsigned int Mode);
	_CompareW CompareW;
	typedef int    (WINAPI* _ConfigureW)(int ItemNumber);
	_ConfigureW ConfigureW;
	typedef int    (WINAPI* _DeleteFilesW)(HANDLE hPlugin,Far2::PluginPanelItem *PanelItem,int ItemsNumber,int OpMode);
	_DeleteFilesW DeleteFilesW;
	typedef void   (WINAPI* _ExitFARW)(void);
	_ExitFARW ExitFARW;
	typedef void   (WINAPI* _FreeFindDataW)(HANDLE hPlugin,Far2::PluginPanelItem *PanelItem,int ItemsNumber);
	_FreeFindDataW FreeFindDataW;
	typedef void   (WINAPI* _FreeVirtualFindDataW)(HANDLE hPlugin,Far2::PluginPanelItem *PanelItem,int ItemsNumber);
	_FreeVirtualFindDataW FreeVirtualFindDataW;
	typedef int    (WINAPI* _GetFilesW)(HANDLE hPlugin,Far2::PluginPanelItem *PanelItem,int ItemsNumber,int Move,const wchar_t **DestPath,int OpMode);
	_GetFilesW GetFilesW;
	typedef int    (WINAPI* _GetFindDataW)(HANDLE hPlugin,Far2::PluginPanelItem **pPanelItem,int *pItemsNumber,int OpMode);
	_GetFindDataW GetFindDataW;
	typedef int    (WINAPI* _GetMinFarVersionW)(void);
	_GetMinFarVersionW GetMinFarVersionW;
	typedef void   (WINAPI* _GetOpenPluginInfoW)(HANDLE hPlugin,Far2::OpenPluginInfo *Info);
	_GetOpenPluginInfoW GetOpenPluginInfoW;
	typedef void   (WINAPI* _GetPluginInfoW)(Far2::PluginInfo *Info);
	_GetPluginInfoW GetPluginInfoW;
	typedef int    (WINAPI* _GetVirtualFindDataW)(HANDLE hPlugin,Far2::PluginPanelItem **pPanelItem,int *pItemsNumber,const wchar_t *Path);
	_GetVirtualFindDataW GetVirtualFindDataW;
	typedef int    (WINAPI* _MakeDirectoryW)(HANDLE hPlugin,const wchar_t **Name,int OpMode);
	_MakeDirectoryW MakeDirectoryW;
	typedef HANDLE (WINAPI* _OpenFilePluginW)(const wchar_t *Name,const unsigned char *Data,int DataSize,int OpMode);
	_OpenFilePluginW OpenFilePluginW;
	typedef HANDLE (WINAPI* _OpenPluginW)(int OpenFrom,INT_PTR Item);
	_OpenPluginW OpenPluginW;
	typedef int    (WINAPI* _ProcessDialogEventW)(int Event,void *Param);
	_ProcessDialogEventW ProcessDialogEventW;
	typedef int    (WINAPI* _ProcessEditorEventW)(int Event,void *Param);
	_ProcessEditorEventW ProcessEditorEventW;
	typedef int    (WINAPI* _ProcessEditorInputW)(const INPUT_RECORD *Rec);
	_ProcessEditorInputW ProcessEditorInputW;
	typedef int    (WINAPI* _ProcessEventW)(HANDLE hPlugin,int Event,void *Param);
	_ProcessEventW ProcessEventW;
	typedef int    (WINAPI* _ProcessHostFileW)(HANDLE hPlugin,Far2::PluginPanelItem *PanelItem,int ItemsNumber,int OpMode);
	_ProcessHostFileW ProcessHostFileW;
	typedef int    (WINAPI* _ProcessKeyW)(HANDLE hPlugin,int Key,unsigned int ControlState);
	_ProcessKeyW ProcessKeyW;
	typedef int    (WINAPI* _ProcessSynchroEventW)(int Event,void *Param);
	_ProcessSynchroEventW ProcessSynchroEventW;
	typedef int    (WINAPI* _ProcessViewerEventW)(int Event,void *Param);
	_ProcessViewerEventW ProcessViewerEventW;
	typedef int    (WINAPI* _PutFilesW)(HANDLE hPlugin,Far2::PluginPanelItem *PanelItem,int ItemsNumber,int Move,const wchar_t *SrcPath,int OpMode);
	typedef int    (WINAPI* _PutFilesOldW)(HANDLE hPlugin,Far2::PluginPanelItem *PanelItem,int ItemsNumber,int Move,int OpMode);
	_PutFilesW PutFilesW;
	typedef int    (WINAPI* _SetDirectoryW)(HANDLE hPlugin,const wchar_t *Dir,int OpMode);
	_SetDirectoryW SetDirectoryW;
	typedef int    (WINAPI* _SetFindListW)(HANDLE hPlugin,const Far2::PluginPanelItem *PanelItem,int ItemsNumber);
	_SetFindListW SetFindListW;
	typedef void   (WINAPI* _SetStartupInfoW)(const Far2::PluginStartupInfo *Info);
	_SetStartupInfoW SetStartupInfoW;
};
WrapPluginInfo* wpi = NULL;

int OpMode_3_2(OPERATION_MODES OpMode3)
{
	int OpMode2 = 0;
	if (OpMode3 & OPM_SILENT)
		OpMode2 |= Far2::OPM_SILENT;
	if (OpMode3 & OPM_FIND)
		OpMode2 |= Far2::OPM_FIND;
	if (OpMode3 & OPM_VIEW)
		OpMode2 |= Far2::OPM_VIEW;
	if (OpMode3 & OPM_EDIT)
		OpMode2 |= Far2::OPM_EDIT;
	if (OpMode3 & OPM_TOPLEVEL)
		OpMode2 |= Far2::OPM_TOPLEVEL;
	if (OpMode3 & OPM_DESCR)
		OpMode2 |= Far2::OPM_DESCR;
	if (OpMode3 & OPM_QUICKVIEW)
		OpMode2 |= Far2::OPM_QUICKVIEW;
	//if (OpMode3 & OPM_PGDN) -- в Far2 отсутствует
	//	OpMode2 |= Far2::OPM_PGDN;
	return OpMode2;
}
//OPERATION_MODES OpMode_2_3(int OpMode2)
//{
//}

int OpenFrom_3_2(OPENFROM OpenFrom3)
{
	int OpenFrom2 = 0;
	
	switch ((OpenFrom3 & OPEN_FROM_MASK))
	{
		case OPEN_LEFTDISKMENU:
			OpenFrom2 |= Far2::OPEN_DISKMENU; break;
		case OPEN_PLUGINSMENU:
			OpenFrom2 |= Far2::OPEN_PLUGINSMENU; break;
		case OPEN_FINDLIST:
			OpenFrom2 |= Far2::OPEN_FINDLIST; break;
		case OPEN_SHORTCUT:
			OpenFrom2 |= Far2::OPEN_SHORTCUT; break;
		case OPEN_COMMANDLINE:
			OpenFrom2 |= Far2::OPEN_COMMANDLINE; break;
		case OPEN_EDITOR:
			OpenFrom2 |= Far2::OPEN_EDITOR; break;
		case OPEN_VIEWER:
			OpenFrom2 |= Far2::OPEN_VIEWER; break;
		case OPEN_FILEPANEL:
			OpenFrom2 |= Far2::OPEN_FILEPANEL; break;
		case OPEN_DIALOG:
			OpenFrom2 |= Far2::OPEN_DIALOG; break;
		case OPEN_ANALYSE:
			OpenFrom2 |= Far2::OPEN_ANALYSE; break;
		//case OPEN_RIGHTDISKMENU: -- в Far2 отсутсвует
	}
	
	if (OpenFrom3 & OPEN_FROMMACRO_MASK)
		OpenFrom2 |= Far2::OPEN_FROMMACRO;
	
	return OpenFrom2;
}

OPENPANELINFO_FLAGS OpenPanelInfoFlags_2_3(DWORD Flags2)
{
	OPENPANELINFO_FLAGS Flags3 = OPIF_NONE;
	
	if (!(Flags2 & Far2::OPIF_USEFILTER)) Flags3 |= OPIF_DISABLEFILTER;
	if (!(Flags2 & Far2::OPIF_USESORTGROUPS)) Flags3 |= OPIF_DISABLESORTGROUPS;
	if (!(Flags2 & Far2::OPIF_USEHIGHLIGHTING)) Flags3 |= OPIF_DISABLEHIGHLIGHTING;
	if ((Flags2 & Far2::OPIF_ADDDOTS)) Flags3 |= OPIF_ADDDOTS;
	if ((Flags2 & Far2::OPIF_RAWSELECTION)) Flags3 |= OPIF_RAWSELECTION;
	if ((Flags2 & Far2::OPIF_REALNAMES)) Flags3 |= OPIF_REALNAMES;
	if ((Flags2 & Far2::OPIF_SHOWNAMESONLY)) Flags3 |= OPIF_SHOWNAMESONLY;
	if ((Flags2 & Far2::OPIF_SHOWRIGHTALIGNNAMES)) Flags3 |= OPIF_SHOWRIGHTALIGNNAMES;
	if ((Flags2 & Far2::OPIF_SHOWPRESERVECASE)) Flags3 |= OPIF_SHOWPRESERVECASE;
	if ((Flags2 & Far2::OPIF_COMPAREFATTIME)) Flags3 |= OPIF_COMPAREFATTIME;
	if ((Flags2 & Far2::OPIF_EXTERNALGET)) Flags3 |= OPIF_EXTERNALGET;
	if ((Flags2 & Far2::OPIF_EXTERNALPUT)) Flags3 |= OPIF_EXTERNALPUT;
	if ((Flags2 & Far2::OPIF_EXTERNALDELETE)) Flags3 |= OPIF_EXTERNALDELETE;
	if ((Flags2 & Far2::OPIF_EXTERNALMKDIR)) Flags3 |= OPIF_EXTERNALMKDIR;
	if ((Flags2 & Far2::OPIF_USEATTRHIGHLIGHTING)) Flags3 |= OPIF_USEATTRHIGHLIGHTING;
	
	return Flags3;
}

Far2::OPENPLUGININFO_SORTMODES SortMode_3_2(OPENPANELINFO_SORTMODES Mode3)
{
	Far2::OPENPLUGININFO_SORTMODES Mode2 = Far2::SM_DEFAULT;
	switch (Mode3)
	{
		case SM_UNSORTED: Mode2 = Far2::SM_UNSORTED; break;
		case SM_NAME: Mode2 = Far2::SM_NAME; break;
		case SM_EXT: Mode2 = Far2::SM_EXT; break;
		case SM_MTIME: Mode2 = Far2::SM_MTIME; break;
		case SM_CTIME: Mode2 = Far2::SM_CTIME; break;
		case SM_ATIME: Mode2 = Far2::SM_ATIME; break;
		case SM_SIZE: Mode2 = Far2::SM_SIZE; break;
		case SM_DESCR: Mode2 = Far2::SM_DESCR; break;
		case SM_OWNER: Mode2 = Far2::SM_OWNER; break;
		case SM_COMPRESSEDSIZE: Mode2 = Far2::SM_COMPRESSEDSIZE; break;
		case SM_NUMLINKS: Mode2 = Far2::SM_NUMLINKS; break;
		case SM_NUMSTREAMS: Mode2 = Far2::SM_NUMSTREAMS; break;
		case SM_STREAMSSIZE: Mode2 = Far2::SM_STREAMSSIZE; break;
		case SM_FULLNAME: Mode2 = Far2::SM_FULLNAME; break;
	}
	return Mode2;
}

OPENPANELINFO_SORTMODES SortMode_2_3(/*Far2::OPENPLUGININFO_SORTMODES*/int Mode2)
{
	OPENPANELINFO_SORTMODES Mode3 = SM_DEFAULT;
	switch (Mode2)
	{
		case Far2::SM_UNSORTED: Mode3 = SM_UNSORTED; break;
		case Far2::SM_NAME: Mode3 = SM_NAME; break;
		case Far2::SM_EXT: Mode3 = SM_EXT; break;
		case Far2::SM_MTIME: Mode3 = SM_MTIME; break;
		case Far2::SM_CTIME: Mode3 = SM_CTIME; break;
		case Far2::SM_ATIME: Mode3 = SM_ATIME; break;
		case Far2::SM_SIZE: Mode3 = SM_SIZE; break;
		case Far2::SM_DESCR: Mode3 = SM_DESCR; break;
		case Far2::SM_OWNER: Mode3 = SM_OWNER; break;
		case Far2::SM_COMPRESSEDSIZE: Mode3 = SM_COMPRESSEDSIZE; break;
		case Far2::SM_NUMLINKS: Mode3 = SM_NUMLINKS; break;
		case Far2::SM_NUMSTREAMS: Mode3 = SM_NUMSTREAMS; break;
		case Far2::SM_STREAMSSIZE: Mode3 = SM_STREAMSSIZE; break;
		case Far2::SM_FULLNAME: Mode3 = SM_FULLNAME; break;
	}
	return Mode3;
}

PLUGINPANELITEMFLAGS PluginPanelItemFlags_2_3(DWORD Flags2)
{
	PLUGINPANELITEMFLAGS Flags3 = PPIF_NONE;
	if (Flags2 & Far2::PPIF_PROCESSDESCR)
		Flags3 |= PPIF_PROCESSDESCR;
	if (Flags2 & Far2::PPIF_USERDATA)
		Flags3 |= PPIF_USERDATA;
	if (Flags2 & Far2::PPIF_SELECTED)
		Flags3 |= PPIF_SELECTED;
	return Flags3;
}

DWORD PluginPanelItemFlags_3_2(PLUGINPANELITEMFLAGS Flags3)
{
	DWORD Flags2 = 0;
	if (Flags3 & PPIF_PROCESSDESCR)
		Flags2 |= Far2::PPIF_PROCESSDESCR;
	if (Flags3 & PPIF_USERDATA)
		Flags2 |= Far2::PPIF_USERDATA;
	if (Flags3 & PPIF_SELECTED)
		Flags2 |= Far2::PPIF_SELECTED;
	return Flags2;
}

void PluginPanelItem_2_3(const Far2::PluginPanelItem* p2, PluginPanelItem* p3)
{
	p3->FileAttributes = p2->FindData.dwFileAttributes;
	p3->CreationTime = p2->FindData.ftCreationTime;
	p3->LastAccessTime = p2->FindData.ftLastAccessTime;
	p3->LastWriteTime = p2->FindData.ftLastWriteTime;
	p3->ChangeTime = p2->FindData.ftLastWriteTime;
	p3->FileSize = p2->FindData.nFileSize;
	p3->PackSize = p2->FindData.nPackSize;
	p3->FileName = p2->FindData.lpwszFileName;
	p3->AlternateFileName = p2->FindData.lpwszAlternateFileName;
	p3->Flags = PluginPanelItemFlags_2_3(p2->Flags);
	p3->NumberOfLinks = p2->NumberOfLinks;
	p3->Description = p2->Description;
	p3->Owner = p2->Owner;
	p3->CustomColumnData = p2->CustomColumnData;
	p3->CustomColumnNumber = p2->CustomColumnNumber;
	p3->UserData = p2->UserData;
	p3->CRC32 = p2->CRC32;
}

PluginPanelItem* PluginPanelItems_2_3(const Far2::PluginPanelItem* pItems, int ItemsNumber)
{
	PluginPanelItem* p3 = NULL;
	if (pItems && ItemsNumber > 0)
	{
		p3 = (PluginPanelItem*)calloc(ItemsNumber, sizeof(*p3));
		if (p3)
		{
			for (int i = 0; i < ItemsNumber; i++)
				PluginPanelItem_2_3(pItems+i, p3+i);
		}
	}
	return p3;
}

void PluginPanelItem_3_2(const PluginPanelItem* p3, Far2::PluginPanelItem* p2)
{
	p2->FindData.dwFileAttributes = p3->FileAttributes;
	p2->FindData.ftCreationTime = p3->CreationTime;
	p2->FindData.ftLastAccessTime = p3->LastAccessTime;
	p2->FindData.ftLastWriteTime = p3->LastWriteTime;
	//p2->FindData.ftLastWriteTime = p3->ChangeTime;
	p2->FindData.nFileSize = p3->FileSize;
	p2->FindData.nPackSize = p3->PackSize;
	p2->FindData.lpwszFileName = p3->FileName;
	p2->FindData.lpwszAlternateFileName = p3->AlternateFileName;
	p2->Flags = PluginPanelItemFlags_3_2(p3->Flags);
	p2->NumberOfLinks = p3->NumberOfLinks;
	p2->Description = p3->Description;
	p2->Owner = p3->Owner;
	p2->CustomColumnData = p3->CustomColumnData;
	p2->CustomColumnNumber = p3->CustomColumnNumber;
	p2->UserData = p3->UserData;
	p2->CRC32 = p3->CRC32;
}

Far2::PluginPanelItem* PluginPanelItems_3_2(const PluginPanelItem* pItems, int ItemsNumber)
{
	Far2::PluginPanelItem* p2 = NULL;
	if (pItems && ItemsNumber > 0)
	{
		p2 = (Far2::PluginPanelItem*)calloc(ItemsNumber, sizeof(*p2));
		if (p2)
		{
			for (int i = 0; i < ItemsNumber; i++)
			{
				PluginPanelItem_3_2(pItems+i, p2+i);
			}
		}
	}
	return p2;
}

//FarKey FarKey_2_3(int Key2)
//{
//	FarKey Key3 = {0};
//	//TODO:
//	return Key3;
//}

DWORD FarKey_3_2(KEY_EVENT_RECORD Key)
{
	int Key2 = 0;
	//TODO:
	return Key2;
}

FARDIALOGITEMTYPES DialogItemTypes_2_3(int ItemType2)
{
	FARDIALOGITEMTYPES ItemType3 = DI_TEXT;
	switch (ItemType2)
	{
	case Far2::DI_TEXT: ItemType3 = DI_TEXT; break;
	case Far2::DI_VTEXT: ItemType3 = DI_VTEXT; break;
	case Far2::DI_SINGLEBOX: ItemType3 = DI_SINGLEBOX; break;
	case Far2::DI_DOUBLEBOX: ItemType3 = DI_DOUBLEBOX; break;
	case Far2::DI_EDIT: ItemType3 = DI_EDIT; break;
	case Far2::DI_PSWEDIT: ItemType3 = DI_PSWEDIT; break;
	case Far2::DI_FIXEDIT: ItemType3 = DI_FIXEDIT; break;
	case Far2::DI_BUTTON: ItemType3 = DI_BUTTON; break;
	case Far2::DI_CHECKBOX: ItemType3 = DI_CHECKBOX; break;
	case Far2::DI_RADIOBUTTON: ItemType3 = DI_RADIOBUTTON; break;
	case Far2::DI_COMBOBOX: ItemType3 = DI_COMBOBOX; break;
	case Far2::DI_LISTBOX: ItemType3 = DI_LISTBOX; break;
	case Far2::DI_USERCONTROL: ItemType3 = DI_USERCONTROL; break;
	}
	return ItemType3;
}

int DialogItemTypes_3_2(FARDIALOGITEMTYPES ItemType3)
{
	int ItemType2 = DI_TEXT;
	switch (ItemType3)
	{
	case DI_TEXT: ItemType2 = Far2::DI_TEXT; break;
	case DI_VTEXT: ItemType2 = Far2::DI_VTEXT; break;
	case DI_SINGLEBOX: ItemType2 = Far2::DI_SINGLEBOX; break;
	case DI_DOUBLEBOX: ItemType2 = Far2::DI_DOUBLEBOX; break;
	case DI_EDIT: ItemType2 = Far2::DI_EDIT; break;
	case DI_PSWEDIT: ItemType2 = Far2::DI_PSWEDIT; break;
	case DI_FIXEDIT: ItemType2 = Far2::DI_FIXEDIT; break;
	case DI_BUTTON: ItemType2 = Far2::DI_BUTTON; break;
	case DI_CHECKBOX: ItemType2 = Far2::DI_CHECKBOX; break;
	case DI_RADIOBUTTON: ItemType2 = Far2::DI_RADIOBUTTON; break;
	case DI_COMBOBOX: ItemType2 = Far2::DI_COMBOBOX; break;
	case DI_LISTBOX: ItemType2 = Far2::DI_LISTBOX; break;
	case DI_USERCONTROL: ItemType2 = Far2::DI_USERCONTROL; break;
	}
	return ItemType2;
}

DWORD FarDialogItemFlags_3_2(FarDialogItemFlags Flags3)
{
	_ASSERTE(Far2::DIF_COLORMASK == DIF_COLORMASK);
	DWORD Flags2 = (DWORD)(Flags3 & DIF_COLORMASK);

	if (Flags3 & DIF_SETCOLOR)
		Flags2 |= Far2::DIF_SETCOLOR;
	if (Flags3 & DIF_BOXCOLOR)
		Flags2 |= Far2::DIF_BOXCOLOR;
	if (Flags3 & DIF_GROUP)
		Flags2 |= Far2::DIF_GROUP;
	if (Flags3 & DIF_LEFTTEXT)
		Flags2 |= Far2::DIF_LEFTTEXT;
	if (Flags3 & DIF_MOVESELECT)
		Flags2 |= Far2::DIF_MOVESELECT;
	if (Flags3 & DIF_SHOWAMPERSAND)
		Flags2 |= Far2::DIF_SHOWAMPERSAND;
	if (Flags3 & DIF_CENTERGROUP)
		Flags2 |= Far2::DIF_CENTERGROUP;
	if (Flags3 & DIF_NOBRACKETS/* == DIF_MANUALADDHISTORY*/)
		Flags2 |= Far2::DIF_NOBRACKETS/* == DIF_MANUALADDHISTORY*/;
	if (Flags3 & DIF_SEPARATOR)
		Flags2 |= Far2::DIF_SEPARATOR;
	if (Flags3 & DIF_SEPARATOR2/* == DIF_EDITOR == DIF_LISTNOAMPERSAND*/)
		Flags2 |= Far2::DIF_SEPARATOR2/* == DIF_EDITOR == DIF_LISTNOAMPERSAND*/;
	if (Flags3 & DIF_LISTNOBOX/* == DIF_HISTORY == DIF_BTNNOCLOSE == DIF_CENTERTEXT*/)
		Flags2 |= Far2::DIF_LISTNOBOX/* == DIF_HISTORY == DIF_BTNNOCLOSE == DIF_CENTERTEXT*/;
	if (Flags3 & DIF_SETSHIELD/* == DIF_EDITEXPAND*/)
		Flags2 |= Far2::DIF_SETSHIELD/* == DIF_EDITEXPAND*/;
	if (Flags3 & DIF_DROPDOWNLIST)
		Flags2 |= Far2::DIF_DROPDOWNLIST;
	if (Flags3 & DIF_USELASTHISTORY)
		Flags2 |= Far2::DIF_USELASTHISTORY;
	if (Flags3 & DIF_MASKEDIT)
		Flags2 |= Far2::DIF_MASKEDIT;
	if (Flags3 & DIF_SELECTONENTRY/* == DIF_3STATE*/)
		Flags2 |= Far2::DIF_SELECTONENTRY/* == DIF_3STATE*/;
	if (Flags3 & DIF_SELECTONENTRY/* == DIF_3STATE*/)
		Flags2 |= Far2::DIF_SELECTONENTRY/* == DIF_3STATE*/;
	if (Flags3 & DIF_NOAUTOCOMPLETE/* == DIF_LISTAUTOHIGHLIGHT*/)
		Flags2 |= Far2::DIF_NOAUTOCOMPLETE/* == DIF_LISTAUTOHIGHLIGHT*/;
	if (Flags3 & DIF_LISTNOCLOSE)
		Flags2 |= Far2::DIF_LISTNOCLOSE;
	if (Flags3 & DIF_HIDDEN)
		Flags2 |= Far2::DIF_HIDDEN;
	if (Flags3 & DIF_READONLY)
		Flags2 |= Far2::DIF_READONLY;
	if (Flags3 & DIF_NOFOCUS)
		Flags2 |= Far2::DIF_NOFOCUS;
	if (Flags3 & DIF_DISABLE)
		Flags2 |= Far2::DIF_DISABLE;
	if (Flags3 & DIF_DISABLE)
		Flags2 |= Far2::DIF_DISABLE;

	return Flags2;
}

FarDialogItemFlags FarDialogItemFlags_2_3(DWORD Flags2)
{
	_ASSERTE(Far2::DIF_COLORMASK == DIF_COLORMASK);
	FarDialogItemFlags Flags3 = (Flags2 & Far2::DIF_COLORMASK);

	if (Flags2 & Far2::DIF_SETCOLOR)
		Flags3 |= DIF_SETCOLOR;
	if (Flags2 & Far2::DIF_BOXCOLOR)
		Flags3 |= DIF_BOXCOLOR;
	if (Flags2 & Far2::DIF_GROUP)
		Flags3 |= DIF_GROUP;
	if (Flags2 & Far2::DIF_LEFTTEXT)
		Flags3 |= DIF_LEFTTEXT;
	if (Flags2 & Far2::DIF_MOVESELECT)
		Flags3 |= DIF_MOVESELECT;
	if (Flags2 & Far2::DIF_SHOWAMPERSAND)
		Flags3 |= DIF_SHOWAMPERSAND;
	if (Flags2 & Far2::DIF_CENTERGROUP)
		Flags3 |= DIF_CENTERGROUP;
	if (Flags2 & Far2::DIF_NOBRACKETS/* == DIF_MANUALADDHISTORY*/)
		Flags3 |= DIF_NOBRACKETS/* == DIF_MANUALADDHISTORY*/;
	if (Flags2 & Far2::DIF_SEPARATOR)
		Flags3 |= DIF_SEPARATOR;
	if (Flags2 & Far2::DIF_SEPARATOR2/* == DIF_EDITOR == DIF_LISTNOAMPERSAND*/)
		Flags3 |= DIF_SEPARATOR2/* == DIF_EDITOR == DIF_LISTNOAMPERSAND*/;
	if (Flags2 & Far2::DIF_LISTNOBOX/* == DIF_HISTORY == DIF_BTNNOCLOSE == DIF_CENTERTEXT*/)
		Flags3 |= DIF_LISTNOBOX/* == DIF_HISTORY == DIF_BTNNOCLOSE == DIF_CENTERTEXT*/;
	if (Flags2 & Far2::DIF_SETSHIELD/* == DIF_EDITEXPAND*/)
		Flags3 |= DIF_SETSHIELD/* == DIF_EDITEXPAND*/;
	if (Flags2 & Far2::DIF_DROPDOWNLIST)
		Flags3 |= DIF_DROPDOWNLIST;
	if (Flags2 & Far2::DIF_USELASTHISTORY)
		Flags3 |= DIF_USELASTHISTORY;
	if (Flags2 & Far2::DIF_MASKEDIT)
		Flags3 |= DIF_MASKEDIT;
	if (Flags2 & Far2::DIF_SELECTONENTRY/* == DIF_3STATE*/)
		Flags3 |= DIF_SELECTONENTRY/* == DIF_3STATE*/;
	if (Flags2 & Far2::DIF_SELECTONENTRY/* == DIF_3STATE*/)
		Flags3 |= DIF_SELECTONENTRY/* == DIF_3STATE*/;
	if (Flags2 & Far2::DIF_NOAUTOCOMPLETE/* == DIF_LISTAUTOHIGHLIGHT*/)
		Flags3 |= DIF_NOAUTOCOMPLETE/* == DIF_LISTAUTOHIGHLIGHT*/;
	if (Flags2 & Far2::DIF_LISTNOCLOSE)
		Flags3 |= DIF_LISTNOCLOSE;
	if (Flags2 & Far2::DIF_HIDDEN)
		Flags3 |= DIF_HIDDEN;
	if (Flags2 & Far2::DIF_READONLY)
		Flags3 |= DIF_READONLY;
	if (Flags2 & Far2::DIF_NOFOCUS)
		Flags3 |= DIF_NOFOCUS;
	if (Flags2 & Far2::DIF_DISABLE)
		Flags3 |= DIF_DISABLE;
	if (Flags2 & Far2::DIF_DISABLE)
		Flags3 |= DIF_DISABLE;

	return Flags3;
}

void FarListItem_2_3(const Far2::FarListItem* p2, FarListItem* p3)
{
	//TODO: конвертация флагов
	p3->Flags = p2->Flags;
	p3->Text = p2->Text;
	p3->Reserved[0] = p2->Reserved[0];
	p3->Reserved[1] = p2->Reserved[1];
	p3->Reserved[2] = p2->Reserved[2];
}

void FarListItem_3_2(const FarListItem* p3, Far2::FarListItem* p2)
{
	//TODO: конвертация флагов
	p2->Flags = p3->Flags;
	p2->Text = p3->Text;
	p2->Reserved[0] = p3->Reserved[0];
	p2->Reserved[1] = p3->Reserved[1];
	p2->Reserved[2] = p3->Reserved[2];
}

void FarDialogItem_2_3(const Far2::FarDialogItem *p2, FarDialogItem *p3, FarList *pList3)
{
	p3->Reserved = 0;

	p3->Type = DialogItemTypes_2_3(p2->Type);
	p3->X1 = p2->X1;
	p3->Y1 = p2->Y1;
	p3->X2 = p2->X2;
	p3->Y2 = p2->Y2;

	p3->Flags = FarDialogItemFlags_2_3(p2->Flags);
	if (p2->DefaultButton)
		p3->Flags |= DIF_DEFAULTBUTTON;
	if (p2->Focus)
		p3->Flags |= DIF_FOCUS;

	p3->Data = p2->PtrData;
	p3->MaxLength = p2->MaxLen;

	if (p3->Type == DI_EDIT)
	{
		p3->History = p2->Param.History;
	}
	else if (p3->Type == DI_FIXEDIT)
	{
		p3->Mask = p2->Param.Mask;
	}
	else if (p3->Type == DI_COMBOBOX || p3->Type == DI_LISTBOX)
	{
		if (pList3->Items)
		{
			free(pList3->Items);
			pList3->Items = NULL;
		}
		if (p2->Param.ListItems != NULL)
		{
			int nItems = p2->Param.ListItems->ItemsNumber;
			pList3->ItemsNumber = nItems;
			if (p2->Param.ListItems > 0)
			{
				pList3->Items = (FarListItem*)calloc(nItems,sizeof(FarListItem));
				const Far2::FarListItem* pi2 = p2->Param.ListItems->Items;
				FarListItem* pi3 = pList3->Items;
				for (int j = 0; j < nItems; j++, pi2++, pi3++)
				{
					FarListItem_2_3(pi2, pi3);
				}
			}
			p3->ListItems = pList3;
		}
	}
	else if (p3->Type == DI_USERCONTROL)
	{
		p3->VBuf = p2->Param.VBuf;
	}
	else
	{
		p3->Reserved = p2->Param.Reserved;
	}
}

void FarDialogItem_3_2(const FarDialogItem *p3, Far2::FarDialogItem *p2, Far2::FarList *pList2)
{
	p2->Param.Reserved = 0;

	p2->Type = DialogItemTypes_3_2(p3->Type);
	p2->X1 = p3->X1;
	p2->Y1 = p3->Y1;
	p2->X2 = p3->X2;
	p2->Y2 = p3->Y2;

	p2->Flags = FarDialogItemFlags_3_2(p3->Flags);
	p2->DefaultButton = (p3->Flags & DIF_DEFAULTBUTTON) == DIF_DEFAULTBUTTON;
	p2->Focus = (p3->Flags & DIF_FOCUS) == DIF_FOCUS;

	p2->PtrData = p3->Data;
	p2->MaxLen = p3->MaxLength;

	if (p3->Type == DI_EDIT)
	{
		p2->Param.History = p3->History;
	}
	else if (p3->Type == DI_FIXEDIT)
	{
		p2->Param.Mask = p3->Mask;
	}
	else if (p3->Type == DI_COMBOBOX || p3->Type == DI_LISTBOX)
	{
		if (pList2->Items)
		{
			free(pList2->Items);
			pList2->Items = NULL;
		}
		if (p3->ListItems != NULL)
		{
			int nItems = p3->ListItems->ItemsNumber;
			pList2->ItemsNumber = nItems;
			if (p3->ListItems > 0)
			{
				pList2->Items = (Far2::FarListItem*)calloc(nItems,sizeof(FarListItem));
				const FarListItem* pi3 = p3->ListItems->Items;
				Far2::FarListItem* pi2 = pList2->Items;
				for (int j = 0; j < nItems; j++, pi2++, pi3++)
				{
					FarListItem_3_2(pi3, pi2);
				}
			}
			p2->Param.ListItems = pList2;
		}
	}
	else if (p3->Type == DI_USERCONTROL)
	{
		p2->Param.VBuf = p3->VBuf;
	}
	else
	{
		p2->Param.Reserved = p3->Reserved;
	}
}


int gnMsg_2 = 0, gnParam1_2 = 0, gnParam1_3 = 0;
FARMESSAGE gnMsg_3 = DM_FIRST;
LONG_PTR gnParam2_2 = 0;
void* gpParam2_3 = NULL;
FarListItem* gpListItems3 = NULL; INT_PTR gnListItemsMax3 = 0;
Far2::FarListItem* gpListItems2 = NULL; UINT_PTR gnListItemsMax2 = 0;

FARMESSAGE FarMessage_2_3(const int Msg2, const int Param1, LONG_PTR& Param2)
{
	FARMESSAGE Msg3 = DM_FIRST;
	if (Msg2 == gnMsg_2 && gnParam1_2 == Param1 && gnParam2_2 == Param2)
		Msg3 = gnMsg_3;
	else if (Msg2 >= Far2::DM_USER)
		Msg3 = (FARMESSAGE)Msg2;
	else switch (Msg2)
	{
		//TODO: Скорее всего потребуется переработка аргументов для некоторых сообщений
		case Far2::DM_FIRST: Msg3 = DM_FIRST; break;
		case Far2::DM_CLOSE: Msg3 = DM_CLOSE; break;
		case Far2::DM_ENABLE: Msg3 = DM_ENABLE; break;
		case Far2::DM_ENABLEREDRAW: Msg3 = DM_ENABLEREDRAW; break;
		case Far2::DM_GETDLGDATA: Msg3 = DM_GETDLGDATA; break;
		case Far2::DM_GETDLGRECT: Msg3 = DM_GETDLGRECT; break;
		case Far2::DM_GETTEXT: Msg3 = DM_GETTEXT; break;
		case Far2::DM_GETTEXTLENGTH: Msg3 = DM_GETTEXTLENGTH; break;
		case Far2::DM_KEY:
			//TODO: Аргументы?
			//Msg3 = (gnMsg_3 == DN_KEY) ? DN_KEY : DM_KEY;
			Msg3 = DM_KEY;
			break;
		case Far2::DM_MOVEDIALOG: Msg3 = DM_MOVEDIALOG; break;
		case Far2::DM_SETDLGDATA: Msg3 = DM_SETDLGDATA; break;
		case Far2::DM_SETFOCUS: Msg3 = DM_SETFOCUS; break;
		case Far2::DM_REDRAW: Msg3 = DM_REDRAW; break;
		//DM_SETREDRAW=DM_REDRAW,
		case Far2::DM_SETTEXT: Msg3 = DM_SETTEXT; break;
		case Far2::DM_SETMAXTEXTLENGTH: Msg3 = DM_SETMAXTEXTLENGTH; break;
		//DM_SETTEXTLENGTH=DM_SETMAXTEXTLENGTH,
		case Far2::DM_SHOWDIALOG: Msg3 = DM_SHOWDIALOG; break;
		case Far2::DM_GETFOCUS: Msg3 = DM_GETFOCUS; break;
		case Far2::DM_GETCURSORPOS: Msg3 = DM_GETCURSORPOS; break;
		case Far2::DM_SETCURSORPOS: Msg3 = DM_SETCURSORPOS; break;
		case Far2::DM_GETTEXTPTR: Msg3 = DM_GETTEXTPTR; break;
		case Far2::DM_SETTEXTPTR: Msg3 = DM_SETTEXTPTR; break;
		case Far2::DM_SHOWITEM: Msg3 = DM_SHOWITEM; break;
		case Far2::DM_ADDHISTORY: Msg3 = DM_ADDHISTORY; break;
		case Far2::DM_GETCHECK: Msg3 = DM_GETCHECK; break;
		case Far2::DM_SETCHECK: Msg3 = DM_SETCHECK; break;
		case Far2::DM_SET3STATE: Msg3 = DM_SET3STATE; break;
		case Far2::DM_LISTSORT: Msg3 = DM_LISTSORT; break;
		case Far2::DM_LISTGETITEM:
			if (!Param2)
			{
				_ASSERTE(Param2!=NULL);
			}
			else
			{
				const Far2::FarListGetItem* p2 = (const Far2::FarListGetItem*)Param2;
				static FarListGetItem p3;
				p3.ItemIndex = p2->ItemIndex;
				FarListItem_2_3(&p2->Item, &p3.Item);
				Msg3 = DM_LISTGETITEM;
			}
			break;
		case Far2::DM_LISTGETCURPOS: //Msg3 = DM_LISTGETCURPOS; break;
		case Far2::DM_LISTSETCURPOS: //Msg3 = DM_LISTSETCURPOS; break;
			if (!Param2)
			{
				_ASSERTE(Param2!=NULL);
			}
			else
			{
				const Far2::FarListPos* p2 = (const Far2::FarListPos*)Param2;
				static FarListPos p3;
				p3.SelectPos = p2->SelectPos;
				p3.TopPos = p2->TopPos;
				switch (Msg2)
				{
				case Far2::DM_LISTGETCURPOS: Msg3 = DM_LISTGETCURPOS; break;
				case Far2::DM_LISTSETCURPOS: Msg3 = DM_LISTSETCURPOS; break;
				}
			}
			break;
		case Far2::DM_LISTDELETE:
			if (!Param2)
			{
				_ASSERTE(Param2!=NULL);
			}
			else
			{
				const Far2::FarListDelete* p2 = (const Far2::FarListDelete*)Param2;
				static FarListDelete p3;
				p3.StartIndex = p2->StartIndex;
				p3.Count = p2->Count;
				Param2 = (LONG_PTR)&p3;
				Msg3 = DM_LISTDELETE;
			}
			break;
		case Far2::DM_LISTADD:
		case Far2::DM_LISTSET:
			if (!Param2)
			{
				_ASSERTE(Param2!=NULL);
			}
			else
			{
				const Far2::FarList* p2 = (const Far2::FarList*)Param2;
				static FarList p3;
				p3.ItemsNumber = p2->ItemsNumber;
				if (!gpListItems3 || (gnListItemsMax3 < p2->ItemsNumber))
				{
					if (gpListItems3)
					{
						free(gpListItems3);
						gpListItems3 = NULL;
					}
					gnListItemsMax3 = p2->ItemsNumber;
					gpListItems3 = (FarListItem*)calloc(p2->ItemsNumber,sizeof(*gpListItems3));
				}
				if (gpListItems3)
				{
					const Far2::FarListItem* pp2 = p2->Items;
					FarListItem* pp3 = gpListItems3;
					for (int i = 0; i < p2->ItemsNumber; i++, pp2++, pp3++)
					{
						FarListItem_2_3(pp2,pp3);
					}
					p3.Items = gpListItems3;
					Param2 = (LONG_PTR)&p3;
					switch (Msg2)
					{
					case Far2::DM_LISTADD: Msg3 = DM_LISTADD; break;
					case Far2::DM_LISTSET: Msg3 = DM_LISTSET; break;
					}
				}
			}
			break;
		case Far2::DM_LISTADDSTR: Msg3 = DM_LISTADDSTR; break;
		case Far2::DM_LISTUPDATE:
			if (!Param2)
			{
				_ASSERTE(Param2!=NULL);
			}
			else
			{
				const Far2::FarListUpdate* p2 = (const Far2::FarListUpdate*)Param2;
				static FarListUpdate p3;
				p3.Index = p2->Index;
				FarListItem_2_3(&p2->Item, &p3.Item);
				Param2 = (LONG_PTR)&p3;
				Msg3 = DM_LISTUPDATE;
			}
			break;
		case Far2::DM_LISTINSERT:
			if (!Param2)
			{
				_ASSERTE(Param2!=NULL);
			}
			else
			{
				const Far2::FarListInsert* p2 = (const Far2::FarListInsert*)Param2;
				static FarListInsert p3;
				p3.Index = p2->Index;
				FarListItem_2_3(&p2->Item, &p3.Item);
				Param2 = (LONG_PTR)&p3;
				Msg3 = DM_LISTINSERT;
			}
			break;
		case Far2::DM_LISTFINDSTRING:
			if (!Param2)
			{
				_ASSERTE(Param2!=NULL);
			}
			else
			{
				const Far2::FarListFind* p2 = (const Far2::FarListFind*)Param2;
				static FarListFind p3;
				p3.StartIndex = p2->StartIndex;
				p3.Pattern = p2->Pattern;
				p3.Flags = p2->Flags;
				p3.Reserved = p2->Reserved;
				Param2 = (LONG_PTR)&p3;
				Msg3 = DM_LISTFINDSTRING;
			}
			break;
		case Far2::DM_LISTINFO:
			if (!Param2)
			{
				_ASSERTE(Param2!=NULL);
			}
			else
			{
				const Far2::FarListInfo* p2 = (const Far2::FarListInfo*)Param2;
				static FarListInfo p3;
				p3.Flags = p2->Flags;
				p3.ItemsNumber = p2->ItemsNumber;
				p3.SelectPos = p2->SelectPos;
				p3.TopPos = p2->TopPos;
				p3.MaxHeight = p2->MaxHeight;
				p3.MaxLength = p2->MaxLength;
				p3.Reserved[0] = p2->Reserved[0];
				p3.Reserved[1] = p2->Reserved[1];
				p3.Reserved[2] = p2->Reserved[2];
				p3.Reserved[3] = p2->Reserved[3];
				p3.Reserved[4] = p2->Reserved[4];
				p3.Reserved[5] = p2->Reserved[5];
				Param2 = (LONG_PTR)&p3;
				Msg3 = DM_LISTINFO;
			}
			break;
		case Far2::DM_LISTGETDATA: Msg3 = DM_LISTGETDATA; break;
		case Far2::DM_LISTSETDATA:
			if (!Param2)
			{
				_ASSERTE(Param2!=NULL);
			}
			else
			{
				const Far2::FarListItemData* p2 = (const Far2::FarListItemData*)Param2;
				static FarListItemData p3;
				p3.Index = p2->Index;
				p3.DataSize = p2->DataSize;
				p3.Data = p2->Data;
				p3.Reserved = p2->Reserved;
				Param2 = (LONG_PTR)&p3;
				Msg3 = DM_LISTSETDATA;
			}
			break;
		case Far2::DM_LISTSETTITLES: //Msg3 = DM_LISTSETTITLES; break;
		case Far2::DM_LISTGETTITLES: //Msg3 = DM_LISTGETTITLES; break;
			if (!Param2)
			{
				_ASSERTE(Param2!=NULL);
			}
			else
			{
				const Far2::FarListTitles* p2 = (const Far2::FarListTitles*)Param2;
				static FarListTitles p3;
				p3.TitleLen = p2->TitleLen;
				p3.Title = p2->Title;
				p3.BottomLen = p2->BottomLen;
				p3.Bottom = p2->Bottom;
				Param2 = (LONG_PTR)&p3;
				switch (Msg2)
				{
				case Far2::DM_LISTSETTITLES: Msg3 = DM_LISTSETTITLES; break;
				case Far2::DM_LISTGETTITLES: Msg3 = DM_LISTGETTITLES; break;
				}
			}
			break;
		case Far2::DM_RESIZEDIALOG: Msg3 = DM_RESIZEDIALOG; break;
		case Far2::DM_SETITEMPOSITION: Msg3 = DM_SETITEMPOSITION; break;
		case Far2::DM_GETDROPDOWNOPENED: Msg3 = DM_GETDROPDOWNOPENED; break;
		case Far2::DM_SETDROPDOWNOPENED: Msg3 = DM_SETDROPDOWNOPENED; break;
		case Far2::DM_SETHISTORY: Msg3 = DM_SETHISTORY; break;
		case Far2::DM_GETITEMPOSITION: Msg3 = DM_GETITEMPOSITION; break;
		case Far2::DM_SETMOUSEEVENTNOTIFY: Msg3 = DM_SETMOUSEEVENTNOTIFY; break;
		case Far2::DM_EDITUNCHANGEDFLAG: Msg3 = DM_EDITUNCHANGEDFLAG; break;
		case Far2::DM_GETITEMDATA: Msg3 = DM_GETITEMDATA; break;
		case Far2::DM_SETITEMDATA: Msg3 = DM_SETITEMDATA; break;
		case Far2::DM_LISTSETMOUSEREACTION: Msg3 = DM_LISTSETMOUSEREACTION; break;
		case Far2::DM_GETCURSORSIZE: Msg3 = DM_GETCURSORSIZE; break;
		case Far2::DM_SETCURSORSIZE: Msg3 = DM_SETCURSORSIZE; break;
		case Far2::DM_LISTGETDATASIZE: Msg3 = DM_LISTGETDATASIZE; break;
		case Far2::DM_GETSELECTION:
			_ASSERTE(sizeof(EditorSelect)==sizeof(Far2::EditorSelect));
			Msg3 = DM_GETSELECTION;
			break;
		case Far2::DM_SETSELECTION:
			_ASSERTE(sizeof(EditorSelect)==sizeof(Far2::EditorSelect));
			Msg3 = DM_SETSELECTION;
			break;
		case Far2::DM_GETEDITPOSITION:
			_ASSERTE(sizeof(EditorSetPosition)==sizeof(Far2::EditorSetPosition));
			Msg3 = DM_GETEDITPOSITION;
			break;
		case Far2::DM_SETEDITPOSITION:
			_ASSERTE(sizeof(EditorSetPosition)==sizeof(Far2::EditorSetPosition));
			Msg3 = DM_SETEDITPOSITION;
			break;
		case Far2::DM_SETCOMBOBOXEVENT: Msg3 = DM_SETCOMBOBOXEVENT; break;
		case Far2::DM_GETCOMBOBOXEVENT: Msg3 = DM_GETCOMBOBOXEVENT; break;
		case Far2::DM_GETCONSTTEXTPTR: Msg3 = DM_GETCONSTTEXTPTR; break;
		case Far2::DM_GETDIALOGINFO: Msg3 = DM_GETDIALOGINFO; break;
		case Far2::DN_FIRST: Msg3 = DN_FIRST; break;
		case Far2::DN_BTNCLICK: Msg3 = DN_BTNCLICK; break;
		case Far2::DN_CTLCOLORDIALOG: Msg3 = DN_CTLCOLORDIALOG; break;
		case Far2::DN_CTLCOLORDLGITEM: Msg3 = DN_CTLCOLORDLGITEM; break;

		case Far2::DN_CTLCOLORDLGLIST:
			if (!Param2)
			{
				_ASSERTE(Param2!=0);
			}
			else
			{
				const Far2::FarListColors* p2 = (const Far2::FarListColors*)Param2;
				static FarListColors p3;
				//TODO: Конвертация флагов?
				p3.Flags = p2->Flags;
				p3.Reserved = p2->Reserved;
				p3.ColorCount = p2->ColorCount;
				p3.Colors = p2->Colors;
				Param2 = (LONG_PTR)&p3;
				Msg3 = DN_CTLCOLORDLGLIST;
			}
			break;

		case Far2::DN_DRAWDIALOG: Msg3 = DN_DRAWDIALOG; break;

		case Far2::DM_GETDLGITEM:
			if (Param2)
			{
				static FarGetDialogItem p3;
				ZeroStruct(p3);
				Param2 = (LONG_PTR)&p3;
			}
			Msg3 = DM_GETDLGITEM;
			break;
		case Far2::DM_GETDLGITEMSHORT:
			if (Param2)
			{
				static FarDialogItem p3;
				ZeroStruct(p3);
				Param2 = (LONG_PTR)&p3;
			}
			Msg3 = DM_GETDLGITEMSHORT;
			break;
		case Far2::DM_SETDLGITEM:
		case Far2::DM_SETDLGITEMSHORT:
		case Far2::DN_EDITCHANGE:
		case Far2::DN_DRAWDLGITEM:
			if (!Param2)
			{
				_ASSERTE(Param2!=0);
			}
			else
			{
				const Far2::FarDialogItem* p2 = (const Far2::FarDialogItem*)Param2;
				static FarDialogItem p3;
				ZeroStruct(p3);
				FarDialogItem_2_3(p2, &p3, &wpi->m_ListItems3);
				Param2 = (LONG_PTR)&p3;
				switch (Msg2)
				{
				//case Far2::DM_GETDLGITEM: Msg3 = DM_GETDLGITEM; break;
				//case Far2::DM_GETDLGITEMSHORT: Msg3 = DM_GETDLGITEMSHORT; break;
				case Far2::DM_SETDLGITEM: Msg3 = DM_SETDLGITEM; break;
				case Far2::DM_SETDLGITEMSHORT: Msg3 = DM_SETDLGITEMSHORT; break;
				case Far2::DN_EDITCHANGE: Msg3 = DN_EDITCHANGE; break;
				case Far2::DN_DRAWDLGITEM: Msg3 = DN_DRAWDLGITEM; break;
				}
			}
			break;

		case Far2::DN_ENTERIDLE: Msg3 = DN_ENTERIDLE; break;
		case Far2::DN_GOTFOCUS: Msg3 = DN_GOTFOCUS; break;
		case Far2::DN_HELP: Msg3 = DN_HELP; break;
		case Far2::DN_HOTKEY: Msg3 = DN_HOTKEY; break;
		case Far2::DN_INITDIALOG: Msg3 = DN_INITDIALOG; break;
		case Far2::DN_KILLFOCUS: Msg3 = DN_KILLFOCUS; break;
		case Far2::DN_LISTCHANGE: Msg3 = DN_LISTCHANGE; break;
		case Far2::DN_MOUSECLICK:
			// DN_KEY и DN_MOUSECLICK объеденены в DN_CONTROLINPUT. 
			// Param2 указывает на INPUT_RECORD. в будущем планируется приход и других событий.
			_ASSERTE(Msg2 != Far2::DN_MOUSECLICK);
			Msg3 = DN_CONTROLINPUT;
			Msg3 = DM_FIRST;
			break;
		case Far2::DN_DRAGGED: Msg3 = DN_DRAGGED; break;
		case Far2::DN_RESIZECONSOLE: Msg3 = DN_RESIZECONSOLE; break;
		case Far2::DN_MOUSEEVENT:
			// DN_MOUSEEVENT переименовано в DN_INPUT. Param2 указывает на INPUT_RECORD. 
			// в будущем планируется приход не только мышиных событий, поэтому настоятельно 
			// рекомендуется проверять EventType.
			_ASSERTE(Msg2 != Far2::DN_MOUSEEVENT);
			Msg3 = DN_INPUT;
			Msg3 = DM_FIRST;
			break;
		case Far2::DN_DRAWDIALOGDONE: Msg3 = DN_DRAWDIALOGDONE; break;
		case Far2::DN_LISTHOTKEY: Msg3 = DN_LISTHOTKEY; break;
		//DN_GETDIALOGINFO=DM_GETDIALOGINFO, -- отсутствует в Far3
		//DN_CLOSE=DM_CLOSE,
		//DN_KEY=DM_KEY, // 3. DM_KEY больше не равна DN_KEY.
	}
	return Msg3;
}

Far2::FarMessagesProc FarMessage_3_2(const int Msg3, const int Param1, void*& Param2)
{
	Far2::FarMessagesProc Msg2 = Far2::DM_FIRST;
	gnMsg_3 = (FARMESSAGE)Msg3;
	gnParam1_2 = gnParam1_3 = Param1;
	gpParam2_3 = Param2;
	if (Msg3 >= DM_USER)
		Msg2 = (Far2::FarMessagesProc)Msg3;
	else switch (Msg3)
	{
		//TODO: Скорее всего потребуется переработка аргументов для некоторых сообщений
		case DN_INPUT:
		case DN_CONTROLINPUT:
			if (!Param2)
			{
				_ASSERTE(Param2!=0);
				Msg2 = Far2::DM_FIRST;
			}
			else
			{
				const INPUT_RECORD* p = (const INPUT_RECORD*)Param2;
				if (p->EventType == MOUSE_EVENT)
				{
					static MOUSE_EVENT_RECORD mer;
					mer = p->Event.MouseEvent;
					Param2 = &mer;
					if (!mer.dwEventFlags)
						Msg2 = Far2::DN_MOUSECLICK;
					else
						Msg2 = Far2::DN_MOUSEEVENT;
				}
				else if (p->EventType == KEY_EVENT)
				{
					Param2 = (void*)FarKey_3_2(p->Event.KeyEvent);
					Msg2 = Far2::DN_KEY;
				}
				else
					Msg2 = Far2::DM_FIRST;
			}
			break;

		case DM_FIRST: Msg2 = Far2::DM_FIRST; break;
		case DM_CLOSE: Msg2 = Far2::DM_CLOSE; break;
		case DM_ENABLE: Msg2 = Far2::DM_ENABLE; break;
		case DM_ENABLEREDRAW: Msg2 = Far2::DM_ENABLEREDRAW; break;
		case DM_GETDLGDATA: Msg2 = Far2::DM_GETDLGDATA; break;
		case DM_GETDLGRECT: Msg2 = Far2::DM_GETDLGRECT; break;
		case DM_GETTEXT: Msg2 = Far2::DM_GETTEXT; break;
		case DM_GETTEXTLENGTH: Msg2 = Far2::DM_GETTEXTLENGTH; break;
		case DM_KEY:
			//TODO: Аргументы?
			Msg2 = Far2::DM_KEY;
			break;
		case DM_MOVEDIALOG: Msg2 = Far2::DM_MOVEDIALOG; break;
		case DM_SETDLGDATA: Msg2 = Far2::DM_SETDLGDATA; break;
		case DM_SETFOCUS: Msg2 = Far2::DM_SETFOCUS; break;
		case DM_REDRAW: Msg2 = Far2::DM_REDRAW; break;
		case DM_SETTEXT: Msg2 = Far2::DM_SETTEXT; break;
		case DM_SETMAXTEXTLENGTH: Msg2 = Far2::DM_SETMAXTEXTLENGTH; break;
		case DM_SHOWDIALOG: Msg2 = Far2::DM_SHOWDIALOG; break;
		case DM_GETFOCUS: Msg2 = Far2::DM_GETFOCUS; break;
		case DM_GETCURSORPOS: Msg2 = Far2::DM_GETCURSORPOS; break;
		case DM_SETCURSORPOS: Msg2 = Far2::DM_SETCURSORPOS; break;
		case DM_GETTEXTPTR: Msg2 = Far2::DM_GETTEXTPTR; break;
		case DM_SETTEXTPTR: Msg2 = Far2::DM_SETTEXTPTR; break;
		case DM_SHOWITEM: Msg2 = Far2::DM_SHOWITEM; break;
		case DM_ADDHISTORY: Msg2 = Far2::DM_ADDHISTORY; break;
		case DM_GETCHECK: Msg2 = Far2::DM_GETCHECK; break;
		case DM_SETCHECK: Msg2 = Far2::DM_SETCHECK; break;
		case DM_SET3STATE: Msg2 = Far2::DM_SET3STATE; break;
		case DM_LISTSORT: Msg2 = Far2::DM_LISTSORT; break;
		case DM_LISTGETITEM:
			if (!Param2)
			{
				_ASSERTE(Param2!=NULL);
			}
			else
			{
				const FarListGetItem* p3 = (const FarListGetItem*)Param2;
				static Far2::FarListGetItem p2;
				p2.ItemIndex = p3->ItemIndex;
				FarListItem_3_2(&p3->Item, &p2.Item);
				Msg2 = Far2::DM_LISTGETITEM;
			}
			break;
		case DM_LISTGETCURPOS: //Msg2 = Far2::DM_LISTGETCURPOS; break;
		case DM_LISTSETCURPOS: //Msg2 = Far2::DM_LISTSETCURPOS; break;
			if (!Param2)
			{
				_ASSERTE(Param2!=NULL);
			}
			else
			{
				const FarListPos* p3 = (const FarListPos*)Param2;
				static Far2::FarListPos p2;
				p2.SelectPos = p3->SelectPos;
				p2.TopPos = p3->TopPos;
				switch (Msg3)
				{
				case DM_LISTGETCURPOS: Msg2 = Far2::DM_LISTGETCURPOS; break;
				case DM_LISTSETCURPOS: Msg2 = Far2::DM_LISTSETCURPOS; break;
				}
			}
			break;
		case DM_LISTDELETE:
			if (!Param2)
			{
				_ASSERTE(Param2!=NULL);
			}
			else
			{
				const FarListDelete* p3 = (const FarListDelete*)Param2;
				static Far2::FarListDelete p2;
				p2.StartIndex = p3->StartIndex;
				p2.Count = p3->Count;
				Param2 = &p2;
				Msg2 = Far2::DM_LISTDELETE;
			}
			break;
		case DM_LISTADD:
		case DM_LISTSET:
			if (!Param2)
			{
				_ASSERTE(Param2!=NULL);
			}
			else
			{
				const FarList* p3 = (const FarList*)Param2;
				static Far2::FarList p2;
				p2.ItemsNumber = p3->ItemsNumber;
				if (!gpListItems2 || (gnListItemsMax2 < p3->ItemsNumber))
				{
					if (gpListItems2)
					{
						free(gpListItems2);
						gpListItems2 = NULL;
					}
					gnListItemsMax2 = p3->ItemsNumber;
					gpListItems2 = (Far2::FarListItem*)calloc(gnListItemsMax2,sizeof(*gpListItems2));
				}
				if (gpListItems2)
				{
					const FarListItem* pp3 = p3->Items;
					Far2::FarListItem* pp2 = gpListItems2;
					for (size_t i = 0; i < p3->ItemsNumber; i++, pp2++, pp3++)
					{
						FarListItem_3_2(pp3, pp2);
					}
					p2.Items = gpListItems2;
					Param2 = &p2;
					switch (Msg3)
					{
					case DM_LISTADD: Msg2 = Far2::DM_LISTADD; break;
					case DM_LISTSET: Msg2 = Far2::DM_LISTSET; break;
					}
				}
			}
			break;
		case DM_LISTADDSTR: Msg2 = Far2::DM_LISTADDSTR; break;
		case DM_LISTUPDATE:
			if (!Param2)
			{
				_ASSERTE(Param2!=NULL);
			}
			else
			{
				const FarListUpdate* p3 = (const FarListUpdate*)Param2;
				static Far2::FarListUpdate p2;
				p2.Index = p3->Index;
				FarListItem_3_2(&p3->Item, &p2.Item);
				Param2 = &p2;
				Msg2 = Far2::DM_LISTUPDATE;
			}
			break;
		case DM_LISTINSERT:
			if (!Param2)
			{
				_ASSERTE(Param2!=NULL);
			}
			else
			{
				const FarListInsert* p3 = (const FarListInsert*)Param2;
				static Far2::FarListInsert p2;
				p2.Index = p3->Index;
				FarListItem_3_2(&p3->Item, &p2.Item);
				Param2 = &p2;
				Msg2 = Far2::DM_LISTINSERT;
			}
			break;
		case DM_LISTFINDSTRING:
			if (!Param2)
			{
				_ASSERTE(Param2!=NULL);
			}
			else
			{
				const FarListFind* p3 = (const FarListFind*)Param2;
				static Far2::FarListFind p2;
				p2.StartIndex = p3->StartIndex;
				p2.Pattern = p3->Pattern;
				//TODO: конвертация флагов
				p2.Flags = p3->Flags;
				p2.Reserved = p3->Reserved;
				Param2 = &p2;
				Msg2 = Far2::DM_LISTFINDSTRING;
			}
			break;
		case DM_LISTINFO:
			if (!Param2)
			{
				_ASSERTE(Param2!=NULL);
			}
			else
			{
				const FarListInfo* p3 = (const FarListInfo*)Param2;
				static Far2::FarListInfo p2;
				//TODO: конвертация флагов
				p2.Flags = p3->Flags;
				p2.ItemsNumber = p3->ItemsNumber;
				p2.SelectPos = p3->SelectPos;
				p2.TopPos = p3->TopPos;
				p2.MaxHeight = p3->MaxHeight;
				p2.MaxLength = p3->MaxLength;
				p2.Reserved[0] = p3->Reserved[0];
				p2.Reserved[1] = p3->Reserved[1];
				p2.Reserved[2] = p3->Reserved[2];
				p2.Reserved[3] = p3->Reserved[3];
				p2.Reserved[4] = p3->Reserved[4];
				p2.Reserved[5] = p3->Reserved[5];
				Param2 = &p2;
				Msg2 = Far2::DM_LISTINFO;
			}
			break;
		case DM_LISTGETDATA: Msg2 = Far2::DM_LISTGETDATA; break;
		case DM_LISTSETDATA:
			if (!Param2)
			{
				_ASSERTE(Param2!=NULL);
			}
			else
			{
				const FarListItemData* p3 = (const FarListItemData*)Param2;
				static Far2::FarListItemData p2;
				p2.Index = p3->Index;
				p2.DataSize = p3->DataSize;
				p2.Data = p3->Data;
				p2.Reserved = p3->Reserved;
				Param2 = &p2;
				Msg2 = Far2::DM_LISTSETDATA;
			}
			break;
		case DM_LISTSETTITLES: //Msg2 = Far2::DM_LISTSETTITLES; break;
		case DM_LISTGETTITLES: //Msg2 = Far2::DM_LISTGETTITLES; break;
			if (!Param2)
			{
				_ASSERTE(Param2!=NULL);
			}
			else
			{
				const FarListTitles* p3 = (const FarListTitles*)Param2;
				static Far2::FarListTitles p2;
				p2.TitleLen = p3->TitleLen;
				p2.Title = p3->Title;
				p2.BottomLen = p3->BottomLen;
				p2.Bottom = p3->Bottom;
				Param2 = &p2;
				switch (Msg2)
				{
				case DM_LISTSETTITLES: Msg2 = Far2::DM_LISTSETTITLES; break;
				case DM_LISTGETTITLES: Msg2 = Far2::DM_LISTGETTITLES; break;
				}
			}
			break;
		case DM_RESIZEDIALOG: Msg2 = Far2::DM_RESIZEDIALOG; break;
		case DM_SETITEMPOSITION: Msg2 = Far2::DM_SETITEMPOSITION; break;
		case DM_GETDROPDOWNOPENED: Msg2 = Far2::DM_GETDROPDOWNOPENED; break;
		case DM_SETDROPDOWNOPENED: Msg2 = Far2::DM_SETDROPDOWNOPENED; break;
		case DM_SETHISTORY: Msg2 = Far2::DM_SETHISTORY; break;
		case DM_GETITEMPOSITION: Msg2 = Far2::DM_GETITEMPOSITION; break;
		case DM_SETMOUSEEVENTNOTIFY: Msg2 = Far2::DM_SETMOUSEEVENTNOTIFY; break;
		case DM_EDITUNCHANGEDFLAG: Msg2 = Far2::DM_EDITUNCHANGEDFLAG; break;
		case DM_GETITEMDATA: Msg2 = Far2::DM_GETITEMDATA; break;
		case DM_SETITEMDATA: Msg2 = Far2::DM_SETITEMDATA; break;
		case DM_LISTSETMOUSEREACTION: Msg2 = Far2::DM_LISTSETMOUSEREACTION; break;
		case DM_GETCURSORSIZE: Msg2 = Far2::DM_GETCURSORSIZE; break;
		case DM_SETCURSORSIZE: Msg2 = Far2::DM_SETCURSORSIZE; break;
		case DM_LISTGETDATASIZE: Msg2 = Far2::DM_LISTGETDATASIZE; break;
		case DM_GETSELECTION:
			_ASSERTE(sizeof(EditorSelect)==sizeof(Far2::EditorSelect));
			Msg2 = Far2::DM_GETSELECTION;
			break;
		case DM_SETSELECTION:
			_ASSERTE(sizeof(EditorSelect)==sizeof(Far2::EditorSelect));
			Msg2 = Far2::DM_SETSELECTION;
			break;
		case DM_GETEDITPOSITION:
			_ASSERTE(sizeof(EditorSetPosition)==sizeof(Far2::EditorSetPosition));
			Msg2 = Far2::DM_GETEDITPOSITION;
			break;
		case DM_SETEDITPOSITION:
			_ASSERTE(sizeof(EditorSetPosition)==sizeof(Far2::EditorSetPosition));
			Msg2 = Far2::DM_SETEDITPOSITION;
			break;
		case DM_SETCOMBOBOXEVENT: Msg2 = Far2::DM_SETCOMBOBOXEVENT; break;
		case DM_GETCOMBOBOXEVENT: Msg2 = Far2::DM_GETCOMBOBOXEVENT; break;
		case DM_GETCONSTTEXTPTR: Msg2 = Far2::DM_GETCONSTTEXTPTR; break;
		case DM_GETDIALOGINFO: Msg2 = Far2::DM_GETDIALOGINFO; break;
		case DN_FIRST: Msg2 = Far2::DN_FIRST; break;
		case DN_BTNCLICK: Msg2 = Far2::DN_BTNCLICK; break;
		case DN_CTLCOLORDIALOG: Msg2 = Far2::DN_CTLCOLORDIALOG; break;
		case DN_CTLCOLORDLGITEM: Msg2 = Far2::DN_CTLCOLORDLGITEM; break;

		case DN_CTLCOLORDLGLIST:
			if (!Param2)
			{
				_ASSERTE(Param2!=0);
			}
			else
			{
				const FarListColors* p3 = (const FarListColors*)Param2;
				static Far2::FarListColors p2;
				//TODO: Конвертация флагов?
				p2.Flags = p3->Flags;
				p2.Reserved = p3->Reserved;
				p2.ColorCount = p3->ColorCount;
				p2.Colors = p3->Colors;
				Param2 = &p2;
				Msg2 = Far2::DN_CTLCOLORDLGLIST;
			}
			break;
		case DN_DRAWDIALOG: Msg2 = Far2::DN_DRAWDIALOG; break;

		case DM_GETDLGITEM:
			_ASSERTE(Msg3!=DM_GETDLGITEM);
			break;
		case DM_GETDLGITEMSHORT:
			_ASSERTE(Msg3!=DM_GETDLGITEMSHORT);
			break;
		case DM_SETDLGITEM:
		case DM_SETDLGITEMSHORT:
		case DN_DRAWDLGITEM:
		case DN_EDITCHANGE:
			if (!Param2)
			{
				_ASSERTE(Param2!=0);
			}
			else
			{
				const FarDialogItem* p3 = (const FarDialogItem*)Param2;
				static Far2::FarDialogItem p2;
				memset(&p2, 0, sizeof(p2));
				FarDialogItem_3_2(p3, &p2, &wpi->m_ListItems2);
				Param2 = &p2;
				switch (Msg3)
				{
				//case DM_GETDLGITEM: Msg2 = Far2::DM_GETDLGITEM; break;
				//case DM_GETDLGITEMSHORT: Msg2 = Far2::DM_GETDLGITEMSHORT; break;
				case DM_SETDLGITEM: Msg2 = Far2::DM_SETDLGITEM; break;
				case DM_SETDLGITEMSHORT: Msg2 = Far2::DM_SETDLGITEMSHORT; break;
				case DN_DRAWDLGITEM: Msg2 = Far2::DN_DRAWDLGITEM; break;
				case DN_EDITCHANGE: Msg2 = Far2::DN_EDITCHANGE; break;
				}
			}
			break;
		
		case DN_ENTERIDLE: Msg2 = Far2::DN_ENTERIDLE; break;
		case DN_GOTFOCUS: Msg2 = Far2::DN_GOTFOCUS; break;
		case DN_HELP: Msg2 = Far2::DN_HELP; break;
		case DN_HOTKEY: Msg2 = Far2::DN_HOTKEY; break;
		case DN_INITDIALOG: Msg2 = Far2::DN_INITDIALOG; break;
		case DN_KILLFOCUS: Msg2 = Far2::DN_KILLFOCUS; break;
		case DN_LISTCHANGE: Msg2 = Far2::DN_LISTCHANGE; break;
		case DN_DRAGGED: Msg2 = Far2::DN_DRAGGED; break;
		case DN_RESIZECONSOLE: Msg2 = Far2::DN_RESIZECONSOLE; break;
		case DN_DRAWDIALOGDONE: Msg2 = Far2::DN_DRAWDIALOGDONE; break;
		case DN_LISTHOTKEY: Msg2 = Far2::DN_LISTHOTKEY; break;
		case DN_CLOSE: Msg2 = Far2::DN_CLOSE; break;
	}

	gnMsg_2 = Msg2;
	gnParam2_2 = (LONG_PTR)Param2;

	return Msg2;
}

void FarMessageParam_2_3(const int Msg2, const int Param1, const void* Param2, void* OrgParam2, LONG_PTR lRc)
{
	if (Param2 == OrgParam2 || !Param2 || !OrgParam2)
	{
		_ASSERTE(Param2 != OrgParam2 && Param2 && OrgParam2);
		return;
	}
	switch (Msg2)
	{
	case Far2::DM_GETDLGITEM:
		{
			_ASSERTE(Msg2!=Far2::DM_GETDLGITEM);
		}
		break;
	case Far2::DM_GETDLGITEMSHORT:
		{
			_ASSERTE(Msg2!=Far2::DM_GETDLGITEMSHORT);
			const Far2::FarDialogItem* p2 = (const Far2::FarDialogItem*)Param2;
			FarDialogItem* p3 = (FarDialogItem*)OrgParam2;
			FarDialogItem_2_3(p2, p3, &wpi->m_ListItems3);
		}
		break;
	case Far2::DM_LISTGETITEM:
		{
			const Far2::FarListGetItem* p2 = (const Far2::FarListGetItem*)Param2;
			FarListGetItem* p3 = (FarListGetItem*)OrgParam2;
			p3->ItemIndex = p2->ItemIndex;
			FarListItem_2_3(&p2->Item, &p3->Item);
		}
		break;
	case Far2::DM_LISTINFO:
		{
			const Far2::FarListInfo* p2 = (const Far2::FarListInfo*)Param2;
			FarListInfo* p3 = (FarListInfo*)OrgParam2;
			p3->Flags = p2->Flags;
			p3->ItemsNumber = p2->ItemsNumber;
			p3->SelectPos = p2->SelectPos;
			p3->TopPos = p2->TopPos;
			p3->MaxHeight = p2->MaxHeight;
			p3->MaxLength = p2->MaxLength;
			p3->Reserved[0] = p2->Reserved[0];
			p3->Reserved[1] = p2->Reserved[1];
			p3->Reserved[2] = p2->Reserved[2];
			p3->Reserved[3] = p2->Reserved[3];
			p3->Reserved[4] = p2->Reserved[4];
			p3->Reserved[5] = p2->Reserved[5];
		}
		break;
	case Far2::DM_LISTGETCURPOS:
		{
			const Far2::FarListPos* p2 = (const Far2::FarListPos*)Param2;
			FarListPos* p3 = (FarListPos*)OrgParam2;
			p3->SelectPos = p2->SelectPos;
			p3->TopPos = p2->TopPos;
		}
		break;
	case Far2::DM_LISTGETTITLES:
		{
			const Far2::FarListTitles* p2 = (const Far2::FarListTitles*)Param2;
			FarListTitles* p3 = (FarListTitles*)OrgParam2;
			p3->TitleLen = p2->TitleLen;
			p3->Title = p2->Title;
			p3->BottomLen = p2->BottomLen;
			p3->Bottom = p2->Bottom;
		}
		break;
	}
}

void FarMessageParam_3_2(HANDLE hDlg3, const int Msg3, const int Param1, const LONG_PTR Param2, LONG_PTR OrgParam2, LONG_PTR& lRc)
{
	if (Param2 == OrgParam2 || !Param2 || !OrgParam2)
	{
		_ASSERTE(Param2 != OrgParam2 && Param2 && OrgParam2);
		return;
	}
	switch (Msg3)
	{
	case DM_GETDLGITEM:
		if (lRc > 0)
		{
			Far2::FarDialogItem* p2 = (Far2::FarDialogItem*)OrgParam2;
			FarGetDialogItem item = {sizeof(lRc)};
			item.Item = (FarDialogItem*)calloc(lRc, 1);
			lRc = psi3.SendDlgMessage(hDlg3, DM_GETDLGITEM, Param1, &item);
			if (lRc > 0)
			{
				FarDialogItem_3_2(item.Item, p2, &wpi->m_ListItems2);
			}
			free(item.Item);
		}
		break;
	case DM_GETDLGITEMSHORT:
		{
			const FarDialogItem* p3 = (const FarDialogItem*)Param2;
			Far2::FarDialogItem* p2 = (Far2::FarDialogItem*)OrgParam2;
			FarDialogItem_3_2(p3, p2, &wpi->m_ListItems2);
		}
		break;
	case DM_LISTGETITEM:
		{
			const FarListGetItem* p3 = (const FarListGetItem*)Param2;
			Far2::FarListGetItem* p2 = (Far2::FarListGetItem*)OrgParam2;
			p2->ItemIndex = p3->ItemIndex;
			FarListItem_3_2(&p3->Item, &p2->Item);
		}
		break;
	case DM_LISTINFO:
		{
			const FarListInfo* p3 = (const FarListInfo*)Param2;
			Far2::FarListInfo* p2 = (Far2::FarListInfo*)OrgParam2;
			//TODO: конвертация флагов
			p2->Flags = p3->Flags;
			p2->ItemsNumber = p3->ItemsNumber;
			p2->SelectPos = p3->SelectPos;
			p2->TopPos = p3->TopPos;
			p2->MaxHeight = p3->MaxHeight;
			p2->MaxLength = p3->MaxLength;
			p2->Reserved[0] = p3->Reserved[0];
			p2->Reserved[1] = p3->Reserved[1];
			p2->Reserved[2] = p3->Reserved[2];
			p2->Reserved[3] = p3->Reserved[3];
			p2->Reserved[4] = p3->Reserved[4];
			p2->Reserved[5] = p3->Reserved[5];
		}
		break;
	case DM_LISTGETCURPOS:
		{
			const FarListPos* p3 = (const FarListPos*)Param2;
			Far2::FarListPos* p2 = (Far2::FarListPos*)OrgParam2;
			p2->SelectPos = p3->SelectPos;
			p2->TopPos = p3->TopPos;
		}
		break;
	case DM_LISTGETTITLES:
		{
			const FarListTitles* p3 = (const FarListTitles*)Param2;
			Far2::FarListTitles* p2 = (Far2::FarListTitles*)OrgParam2;
			p2->TitleLen = p3->TitleLen;
			p2->Title = p3->Title;
			p2->BottomLen = p3->BottomLen;
			p2->Bottom = p3->Bottom;
		}
		break;
	}
};

struct Far2Dialog
{
	// Far3
	HANDLE hDlg3;
	
	// Far2
    int m_X1, m_Y1, m_X2, m_Y2;
    wchar_t *m_HelpTopic;
    Far2::FarDialogItem *m_Items2;
    FarDialogItem *m_Items3;
    FarList *mp_ListInit3;
    UINT m_ItemsNumber;
    DWORD m_Flags;
    Far2::FARWINDOWPROC m_DlgProc;
    LONG_PTR m_Param;
    BOOL m_GuidChecked;
    GUID m_PluginGuid, m_Guid, m_DefGuid;
    
    void FreeDlg();
	static INT_PTR WINAPI Far3DlgProc(HANDLE hDlg, int Msg, int Param1, void* Param2);
    int RunDlg();
    
	Far2Dialog(int X1, int Y1, int X2, int Y2,
	    const wchar_t *HelpTopic, Far2::FarDialogItem *Items, UINT ItemsNumber,
	    DWORD Flags, Far2::FARWINDOWPROC DlgProc, LONG_PTR Param,
	    GUID PluginGuid, GUID DefGuid);
	~Far2Dialog();
};


void WrapPluginInfo::UnloadPlugin()
{
	if (hDll)
	{
		FreeLibrary(hDll);
		hDll = NULL;
	}
	ClearProcAddr();
}

WrapPluginInfo::~WrapPluginInfo()
{
	if (m_ListItems2.Items)
	{
		free(m_ListItems2.Items);
		m_ListItems2.Items = NULL;
	}
	if (m_ListItems3.Items)
	{
		free(m_ListItems3.Items);
		m_ListItems3.Items = NULL;
	}
	if (guids_PluginMenu)
		free(guids_PluginMenu);
	if (guids_PluginDisks)
		free(guids_PluginDisks);
	if (guids_PluginConfig)
		free(guids_PluginConfig);
	if (pAnalyze)
		free(pAnalyze);
	if (InfoLines)
		free(InfoLines);
	if (PanelModesArray)
		free(PanelModesArray);
	_ASSERTE(KeyBar.Labels==KeyBarLabels);
}

void WrapPluginInfo::ClearProcAddr()
{
	AnalyseW = NULL;
	ClosePluginW = NULL;
	CompareW = NULL;
	ConfigureW = NULL;
	DeleteFilesW = NULL;
	ExitFARW = NULL;
	FreeFindDataW = NULL;
	FreeVirtualFindDataW = NULL;
	GetFilesW = NULL;
	GetFindDataW = NULL;
	GetMinFarVersionW = NULL;
	GetOpenPluginInfoW = NULL;
	GetPluginInfoW = NULL;
	GetVirtualFindDataW = NULL;
	MakeDirectoryW = NULL;
	OpenFilePluginW = NULL;
	OpenPluginW = NULL;
	ProcessDialogEventW = NULL;
	ProcessEditorEventW = NULL;
	ProcessEditorInputW = NULL;
	ProcessEventW = NULL;
	ProcessHostFileW = NULL;
	ProcessKeyW = NULL;
	ProcessSynchroEventW = NULL;
	ProcessViewerEventW = NULL;
	PutFilesW = NULL;
	SetDirectoryW = NULL;
	SetFindListW  = NULL;
	SetStartupInfoW = NULL;
}

WrapPluginInfo::WrapPluginInfo()
{
	hDll = NULL;
	PluginDll[0] = IniFile[0] = Title[0] = Desc[0] = Author[0] = RegRoot[0] = File[0] = 0;
	memset(&Version, 0, sizeof(Version));
	memset(&guid_Plugin, 0, sizeof(guid_Plugin));
	memset(&guid_Dialogs, 0, sizeof(guid_Dialogs));
	nPluginMenu = nPluginDisks = nPluginConfig = 0;
	guids_PluginMenu = guids_PluginDisks = guids_PluginConfig = NULL;
	memset(&Info, 0, sizeof(Info));
	InfoLines = NULL; InfoLinesNumber = 0;
	PanelModesArray = NULL; PanelModesNumber = 0;
	KeyBar.CountLabels = 0; KeyBar.Labels = KeyBarLabels;
	LastFar2Dlg = NULL;
	OldPutFilesParams = 0;
	AnalyzeMode = 2;
	ZeroStruct(m_ListItems2); ZeroStruct(m_ListItems3);

	ClearProcAddr();
	pAnalyze = NULL;
};


KeyBarTitles* WrapPluginInfo::KeyBarTitles_2_3(const Far2::KeyBarTitles* KeyBar)
{
	wpi->KeyBar.CountLabels = 0;

	if (KeyBar)
	{
		size_t cnt = 0; // Max count. Real count may be less
		
		struct { wchar_t* const* Titles; DWORD ctrl; } src[] = 
		{
			{KeyBar->Titles, 0},
			{KeyBar->CtrlTitles, LEFT_CTRL_PRESSED},
			{KeyBar->AltTitles, LEFT_ALT_PRESSED},
			{KeyBar->ShiftTitles, SHIFT_PRESSED},
			{KeyBar->CtrlShiftTitles, LEFT_CTRL_PRESSED|SHIFT_PRESSED},
			{KeyBar->AltShiftTitles, LEFT_ALT_PRESSED|SHIFT_PRESSED},
			{KeyBar->CtrlAltTitles, LEFT_CTRL_PRESSED|LEFT_ALT_PRESSED}
			//{KeyBar->CtrlTitles, Far2::PKF_CONTROL/*LEFT_CTRL_PRESSED*/},
			//{KeyBar->AltTitles, Far2::PKF_ALT/*LEFT_ALT_PRESSED*/},
			//{KeyBar->ShiftTitles, Far2::PKF_SHIFT/*SHIFT_PRESSED*/},
			//{KeyBar->CtrlShiftTitles, Far2::PKF_CONTROL/*LEFT_CTRL_PRESSED*/|Far2::PKF_SHIFT/*SHIFT_PRESSED*/},
			//{KeyBar->AltShiftTitles, Far2::PKF_ALT/*LEFT_ALT_PRESSED*/|Far2::PKF_SHIFT/*SHIFT_PRESSED*/},
			//{KeyBar->CtrlAltTitles, Far2::PKF_CONTROL/*LEFT_CTRL_PRESSED*/|Far2::PKF_ALT/*LEFT_ALT_PRESSED*/}
		};
		
		for (int i = 0; i < ARRAYSIZE(src); i++)
		{
			if (src[i].Titles) cnt += 12;
		}
		_ASSERTE(cnt<=ARRAYSIZE(wpi->KeyBarLabels));
		
		wpi->KeyBar.CountLabels = 0;
		
		if (cnt > 0)
		{
			KeyBarLabel *p = wpi->KeyBar.Labels;
			for (int i = 0; i < ARRAYSIZE(src); i++)
			{
				if (!src[i].Titles)
					continue;
				for (int j = 0; j < 12; j++)
				{
					if (!src[i].Titles[j])
						continue;
					p->Key.VirtualKeyCode = VK_F1 + j;
					p->Key.ControlKeyState = src[i].ctrl;
					p->Text = src[i].Titles[j];
					p->LongText = NULL; //src[i].Titles[j];
					p++;
				}
			}
			wpi->KeyBar.CountLabels = p - wpi->KeyBar.Labels;
		}
	}
	return &wpi->KeyBar;
}


// Changed functions
LONG_PTR WINAPI WrapPluginInfo::FarApiDefDlgProc(HANDLE hDlg, int Msg, int Param1, LONG_PTR Param2)
{
	LOG_CMD(L"psi2.DefDlgProc(%i,%i,%i)",Msg,Param1,Param2);
	LONG_PTR lRc = 0;
	HANDLE hDlg3 = wpi->MapDlg_2_3[(Far2Dialog*)hDlg];
	if (hDlg3)
	{
		LONG_PTR OrgParam2 = Param2;
		FARMESSAGE Msg3 = FarMessage_2_3(Msg, Param1, Param2);
		lRc = psi3.DefDlgProc(hDlg3, Msg3, Param1, (void*)Param2);
		if (Param2 && Param2 != OrgParam2)
			FarMessageParam_3_2(hDlg3, Msg, Param1, Param2, OrgParam2, lRc);
	}
	else
	{
		_ASSERTE(hDlg3!=NULL);
	}
	return lRc;
}
LONG_PTR WINAPI WrapPluginInfo::FarApiSendDlgMessage(HANDLE hDlg, int Msg, int Param1, LONG_PTR Param2)
{
	LOG_CMD(L"psi2.SendDlgMessage(%i,%i,%i)",Msg,Param1,Param2);
	LONG_PTR lRc = 0;
	HANDLE hDlg3 = wpi->MapDlg_2_3[(Far2Dialog*)hDlg];
	if (hDlg3)
	{
		LONG_PTR OrgParam2 = Param2;
		FARMESSAGE Msg3 = FarMessage_2_3(Msg, Param1, Param2);
		lRc = psi3.SendDlgMessage(hDlg3, Msg3, Param1, (void*)Param2);
		if (Param2 && Param2 != OrgParam2)
			FarMessageParam_3_2(hDlg3, Msg, Param1, Param2, OrgParam2, lRc);
	}
	else
	{
		_ASSERTE(hDlg3!=NULL);
	}
	return lRc;
}
BOOL WINAPI WrapPluginInfo::FarApiShowHelp(const wchar_t *ModuleName, const wchar_t *Topic, DWORD Flags)
{
	LOG_CMD(L"psi2.ShowHelp",0,0,0);
	FARHELPFLAGS Flags3 = 0
		| ((Flags & Far2::FHELP_NOSHOWERROR) ? FHELP_NOSHOWERROR : 0)
		| ((Flags & Far2::FHELP_FARHELP) ? FHELP_FARHELP : 0)
		| ((Flags & Far2::FHELP_CUSTOMFILE) ? FHELP_CUSTOMFILE : 0)
		| ((Flags & Far2::FHELP_CUSTOMPATH) ? FHELP_CUSTOMPATH : 0)
		| ((Flags & Far2::FHELP_USECONTENTS) ? FHELP_USECONTENTS : 0);
	int iRc = psi3.ShowHelp(ModuleName, Topic, Flags3);
	return iRc;
}
HANDLE WINAPI WrapPluginInfo::FarApiSaveScreen(int X1, int Y1, int X2, int Y2)
{
	LOG_CMD(L"psi2.SaveScreen",0,0,0);
	return psi3.SaveScreen(X1,Y1,X2,Y2);
}
void WINAPI WrapPluginInfo::FarApiRestoreScreen(HANDLE hScreen)
{
	LOG_CMD(L"psi2.RestoreScreen",0,0,0);
	psi3.RestoreScreen(hScreen);
}
void WINAPI WrapPluginInfo::FarApiText(int X, int Y, int Color, const wchar_t *Str)
{
	LOG_CMD(L"psi2.Text",0,0,0);
	psi3.Text(X,Y,Color,Str);
}
int WINAPI WrapPluginInfo::FarApiEditor(const wchar_t *FileName, const wchar_t *Title, int X1, int Y1, int X2, int Y2, DWORD Flags, int StartLine, int StartChar, UINT CodePage)
{
	LOG_CMD(L"psi2.Editor",0,0,0);
	EDITOR_FLAGS Flags3 = 0
		| ((Flags & Far2::EF_NONMODAL) ? EF_NONMODAL : 0)
		| ((Flags & Far2::EF_CREATENEW) ? EF_CREATENEW : 0)
		| ((Flags & Far2::EF_ENABLE_F6) ? EF_ENABLE_F6 : 0)
		| ((Flags & Far2::EF_DISABLEHISTORY) ? EF_DISABLEHISTORY : 0)
		| ((Flags & Far2::EF_DELETEONCLOSE) ? EF_DELETEONCLOSE : 0)
		| ((Flags & Far2::EF_IMMEDIATERETURN) ? EF_IMMEDIATERETURN : 0)
		| ((Flags & Far2::EF_DELETEONLYFILEONCLOSE) ? EF_DELETEONLYFILEONCLOSE : 0);
	int iRc = psi3.Editor(FileName, Title, X1,Y1,X2,Y2, Flags3, StartLine, StartChar, CodePage);
	return iRc;
}
int WINAPI WrapPluginInfo::FarApiViewer(const wchar_t *FileName, const wchar_t *Title, int X1, int Y1, int X2, int Y2, DWORD Flags, UINT CodePage)
{
	LOG_CMD(L"psi2.Viewer",0,0,0);
	VIEWER_FLAGS Flags3 = 0
		| ((Flags & Far2::VF_NONMODAL) ? VF_NONMODAL : 0)
		| ((Flags & Far2::VF_DELETEONCLOSE) ? VF_DELETEONCLOSE : 0)
		| ((Flags & Far2::VF_ENABLE_F6) ? VF_ENABLE_F6 : 0)
		| ((Flags & Far2::VF_DISABLEHISTORY) ? VF_DISABLEHISTORY : 0)
		| ((Flags & Far2::VF_IMMEDIATERETURN) ? VF_IMMEDIATERETURN : 0)
		| ((Flags & Far2::VF_DELETEONLYFILEONCLOSE) ? VF_DELETEONLYFILEONCLOSE : 0);
	int iRc = psi3.Viewer(FileName, Title, X1,Y1,X2,Y2, Flags3, CodePage);
	return iRc;
};
int WINAPI WrapPluginInfo::FarApiMenu(INT_PTR PluginNumber, int X, int Y, int MaxHeight, DWORD Flags, const wchar_t* Title, const wchar_t* Bottom, const wchar_t* HelpTopic, const int* BreakKeys, int* BreakCode, const struct Far2::FarMenuItem *Item, int ItemsNumber)
{
	LOG_CMD(L"psi2.Menu",0,0,0);
	int iRc = -1;
	FarMenuItem *pItems3 = NULL;
	FarKey *pBreak3 = NULL;
	
	if (Item && ItemsNumber > 0)
	{
		pItems3 = (FarMenuItem*)calloc(ItemsNumber, sizeof(*pItems3));
		if (!(Flags & Far2::FMENU_USEEXT))
		{
			const Far2::FarMenuItem* p2 = Item;
			FarMenuItem* p3 = pItems3;
			for (int i = 0; i < ItemsNumber; i++, p2++, p3++)
			{
				p3->Text = p2->Text;
				p3->Flags = 0
					| (p2->Selected ? MIF_SELECTED : 0)
					| (p2->Checked ? MIF_CHECKED : 0)
					| (p2->Separator ? MIF_SEPARATOR : 0);
			}
		}
		else
		{
			const Far2::FarMenuItemEx* p2 = (const Far2::FarMenuItemEx*)Item;
			FarMenuItem* p3 = pItems3;
			for (int i = 0; i < ItemsNumber; i++, p2++, p3++)
			{
				p3->Text = p2->Text;
				p3->Flags = 0
					| ((p2->Flags & Far2::MIF_SELECTED) ? MIF_SELECTED : 0)
					| ((p2->Flags & Far2::MIF_CHECKED) ? MIF_CHECKED : 0)
					| ((p2->Flags & Far2::MIF_SEPARATOR) ? MIF_SEPARATOR : 0)
					| ((p2->Flags & Far2::MIF_DISABLE) ? MIF_DISABLE : 0)
					| ((p2->Flags & Far2::MIF_GRAYED) ? MIF_GRAYED : 0)
					| ((p2->Flags & Far2::MIF_HIDDEN) ? MIF_HIDDEN : 0);
				p3->AccelKey = p2->AccelKey;
				p3->Reserved = p2->Reserved;
				p3->UserData = p2->UserData;
			}
		}
		
		if (BreakKeys)
		{
			int cnt = 0;
			for (int i = 0; BreakKeys[i]; i++)
				cnt++;
			pBreak3 = (FarKey*)calloc(cnt+1, sizeof(*pBreak3));
			for (int i = 0; BreakKeys[i]; i++)
			{
				pBreak3[i].VirtualKeyCode = LOWORD(BreakKeys[i]);
				pBreak3[i].ControlKeyState = 0
					| ((HIWORD(BreakKeys[i]) & Far2::PKF_CONTROL) ? LEFT_CTRL_PRESSED : 0)
					| ((HIWORD(BreakKeys[i]) & Far2::PKF_ALT) ? LEFT_ALT_PRESSED : 0)
					| ((HIWORD(BreakKeys[i]) & Far2::PKF_SHIFT) ? SHIFT_PRESSED : 0);
			}
		}
		
		FARMENUFLAGS Flags3 = 0
			| ((Flags & Far2::FMENU_SHOWAMPERSAND) ? FMENU_SHOWAMPERSAND : 0)
			| ((Flags & Far2::FMENU_WRAPMODE) ? FMENU_WRAPMODE : 0)
			| ((Flags & Far2::FMENU_AUTOHIGHLIGHT) ? FMENU_AUTOHIGHLIGHT : 0)
			| ((Flags & Far2::FMENU_REVERSEAUTOHIGHLIGHT) ? FMENU_REVERSEAUTOHIGHLIGHT : 0)
			| ((Flags & Far2::FMENU_CHANGECONSOLETITLE) ? FMENU_CHANGECONSOLETITLE : 0);
		
		iRc = psi3.Menu(&wpi->guid_Plugin, X, Y, MaxHeight, Flags3,
				Title, Bottom, HelpTopic, pBreak3, BreakCode, pItems3, ItemsNumber);
	}
	
	if (pItems3)
		free(pItems3);
	if (pBreak3)
		free(pBreak3);
	return iRc;
};
int WINAPI WrapPluginInfo::FarApiMessage(INT_PTR PluginNumber, DWORD Flags, const wchar_t *HelpTopic, const wchar_t * const *Items, int ItemsNumber, int ButtonsNumber)
{
	LOG_CMD(L"psi2.Message",0,0,0);
	int iRc = -1;

	DWORD Far3Flags = 0
		| ((Flags & Far2::FMSG_WARNING) ? FMSG_WARNING : 0)
		| ((Flags & Far2::FMSG_ERRORTYPE) ? FMSG_ERRORTYPE : 0)
		| ((Flags & Far2::FMSG_KEEPBACKGROUND) ? FMSG_KEEPBACKGROUND : 0)
		| ((Flags & Far2::FMSG_LEFTALIGN) ? FMSG_LEFTALIGN : 0)
		| ((Flags & Far2::FMSG_ALLINONE) ? FMSG_ALLINONE : 0);
		
	if ((Flags & 0x000F0000) == Far2::FMSG_MB_OK)
		Far3Flags |= FMSG_MB_OK;
	else if ((Flags & 0x000F0000) == Far2::FMSG_MB_OKCANCEL)
		Far3Flags |= FMSG_MB_OKCANCEL;
	else if ((Flags & 0x000F0000) == Far2::FMSG_MB_ABORTRETRYIGNORE)
		Far3Flags |= FMSG_MB_ABORTRETRYIGNORE;
	else if ((Flags & 0x000F0000) == Far2::FMSG_MB_YESNO)
		Far3Flags |= FMSG_MB_YESNO;
	else if ((Flags & 0x000F0000) == Far2::FMSG_MB_YESNOCANCEL)
		Far3Flags |= FMSG_MB_YESNOCANCEL;
	else if ((Flags & 0x000F0000) == Far2::FMSG_MB_RETRYCANCEL)
		Far3Flags |= FMSG_MB_RETRYCANCEL;
	
	iRc = psi3.Message(&wpi->guid_Plugin, Far3Flags, 
				HelpTopic, Items, ItemsNumber, ButtonsNumber);
	return iRc;
};
HANDLE WINAPI WrapPluginInfo::FarApiDialogInit(INT_PTR PluginNumber, int X1, int Y1, int X2, int Y2, const wchar_t* HelpTopic, struct Far2::FarDialogItem *Item, unsigned int ItemsNumber, DWORD Reserved, DWORD Flags, Far2::FARWINDOWPROC DlgProc, LONG_PTR Param)
{
	LOG_CMD(L"psi2.DialogInit",0,0,0);
	Far2Dialog *p = new Far2Dialog(X1, Y1, X2, Y2,
    	HelpTopic, Item, ItemsNumber, Flags, DlgProc, Param,
    	wpi->guid_Plugin, wpi->guid_Dialogs);
	return (HANDLE)p;
};
int WINAPI WrapPluginInfo::FarApiDialogRun(HANDLE hDlg)
{
	LOG_CMD(L"psi2.DialogRun",0,0,0);
	Far2Dialog *p = (Far2Dialog*)hDlg;
	if (!p)
		return -1;
	return p->RunDlg();
}
void WINAPI WrapPluginInfo::FarApiDialogFree(HANDLE hDlg)
{
	LOG_CMD(L"psi2.DialogFree",0,0,0);
	Far2Dialog *p = (Far2Dialog*)hDlg;
	if (!p)
		return;
	delete p;
};
int WINAPI WrapPluginInfo::FarApiControl(HANDLE hPlugin, int Command, int Param1, LONG_PTR Param2)
{
	LOG_CMD(L"psi2.Control(%i,%i,%i)",Command, Param1, Param2);
	//TODO: Конвертация параметров!
	int iRc = 0;
	switch (Command)
	{
	case Far2::FCTL_CLOSEPLUGIN: iRc = psi3.PanelControl(hPlugin, FCTL_CLOSEPANEL, Param1, (void*)Param2); break;
	case Far2::FCTL_GETPANELINFO:
		{
			Far2::PanelInfo* p2 = (Far2::PanelInfo*)Param2;
			PanelInfo p3 = {sizeof(PanelInfo)};
			iRc = psi3.PanelControl(hPlugin, FCTL_GETPANELINFO, Param1, &p3);
			if (iRc)
			{
				memset(p2, 0, sizeof(*p2));
				//TODO: конвертация типа
				p2->PanelType = p3.PanelType;
				p2->Plugin = (p3.Flags & PFLAGS_PLUGIN) == PFLAGS_PLUGIN;
				p2->PanelRect = p3.PanelRect;
				p2->ItemsNumber = p3.ItemsNumber;
				p2->SelectedItemsNumber = p3.SelectedItemsNumber;
				p2->CurrentItem = p3.CurrentItem;
				p2->TopPanelItem = p3.TopPanelItem;
				p2->Visible = (p3.Flags & PFLAGS_VISIBLE) == PFLAGS_VISIBLE;
				p2->Focus = (p3.Flags & PFLAGS_FOCUS) == PFLAGS_FOCUS;
				//TODO: конвертация режима
				p2->ViewMode = p3.ViewMode;
				p2->ShortNames = (p3.Flags & PFLAGS_ALTERNATIVENAMES) == PFLAGS_ALTERNATIVENAMES;
				p2->SortMode = p3.SortMode;
				//TODO: конвертация флагов
				p2->Flags = p3.Flags & 0x1FF;
			}
		}
		break;
	case Far2::FCTL_UPDATEPANEL: iRc = psi3.PanelControl(hPlugin, FCTL_UPDATEPANEL, Param1, (void*)Param2); break;
	case Far2::FCTL_REDRAWPANEL: iRc = psi3.PanelControl(hPlugin, FCTL_REDRAWPANEL, Param1, (void*)Param2); break;
	case Far2::FCTL_GETCMDLINE: iRc = psi3.PanelControl(hPlugin, FCTL_GETCMDLINE, Param1, (void*)Param2); break;
	case Far2::FCTL_SETCMDLINE: iRc = psi3.PanelControl(hPlugin, FCTL_SETCMDLINE, Param1, (void*)Param2); break;
	case Far2::FCTL_SETSELECTION: iRc = psi3.PanelControl(hPlugin, FCTL_SETSELECTION, Param1, (void*)Param2); break;
	case Far2::FCTL_SETVIEWMODE: iRc = psi3.PanelControl(hPlugin, FCTL_SETVIEWMODE, Param1, (void*)Param2); break;
	case Far2::FCTL_INSERTCMDLINE: iRc = psi3.PanelControl(hPlugin, FCTL_INSERTCMDLINE, Param1, (void*)Param2); break;
	case Far2::FCTL_SETUSERSCREEN: iRc = psi3.PanelControl(hPlugin, FCTL_SETUSERSCREEN, Param1, (void*)Param2); break;
	case Far2::FCTL_SETPANELDIR: iRc = psi3.PanelControl(hPlugin, FCTL_SETPANELDIR, Param1, (void*)Param2); break;
	case Far2::FCTL_SETCMDLINEPOS: iRc = psi3.PanelControl(hPlugin, FCTL_SETCMDLINEPOS, Param1, (void*)Param2); break;
	case Far2::FCTL_GETCMDLINEPOS: iRc = psi3.PanelControl(hPlugin, FCTL_GETCMDLINEPOS, Param1, (void*)Param2); break;
	case Far2::FCTL_SETSORTMODE: iRc = psi3.PanelControl(hPlugin, FCTL_SETSORTMODE, Param1, (void*)Param2); break;
	case Far2::FCTL_SETSORTORDER: iRc = psi3.PanelControl(hPlugin, FCTL_SETSORTORDER, Param1, (void*)Param2); break;
	case Far2::FCTL_GETCMDLINESELECTEDTEXT:
		{
			CmdLineSelect sel = {0};
			int iSel = psi3.PanelControl(hPlugin, FCTL_GETCMDLINESELECTION, 0, &sel);
			int nLen = ((sel.SelEnd > sel.SelStart)
					? (sel.SelEnd - sel.SelStart + 1)
					: (sel.SelStart - sel.SelEnd + 1));
			if (Param2 == 0)
			{
				iRc = (nLen+1) * sizeof(wchar_t);
			}
			else
			{
				int nCmdLen = psi3.PanelControl(hPlugin, FCTL_GETCMDLINE, 0, NULL);
				wchar_t* psz = (wchar_t*)calloc(nCmdLen+1,sizeof(wchar_t));
				nCmdLen = psi3.PanelControl(hPlugin, FCTL_GETCMDLINE, nCmdLen, psz);
				lstrcpyn((wchar_t*)Param2, psz+((sel.SelEnd > sel.SelStart) ? sel.SelStart : sel.SelEnd),
					min((nLen+1),Param1));
				iRc = TRUE;
			}
		}
		break;
	case Far2::FCTL_SETCMDLINESELECTION: iRc = psi3.PanelControl(hPlugin, FCTL_SETCMDLINESELECTION, Param1, (void*)Param2); break;
	case Far2::FCTL_GETCMDLINESELECTION: iRc = psi3.PanelControl(hPlugin, FCTL_GETCMDLINESELECTION, Param1, (void*)Param2); break;
	case Far2::FCTL_CHECKPANELSEXIST: iRc = psi3.PanelControl(hPlugin, FCTL_CHECKPANELSEXIST, Param1, (void*)Param2); break;
	case Far2::FCTL_SETNUMERICSORT: iRc = psi3.PanelControl(hPlugin, FCTL_SETNUMERICSORT, Param1, (void*)Param2); break;
	case Far2::FCTL_GETUSERSCREEN: iRc = psi3.PanelControl(hPlugin, FCTL_GETUSERSCREEN, Param1, (void*)Param2); break;
	case Far2::FCTL_ISACTIVEPANEL:
		iRc = psi3.PanelControl(hPlugin, FCTL_ISACTIVEPANEL, Param1, (void*)Param2); break;
	case Far2::FCTL_GETPANELITEM: //iRc = psi3.PanelControl(hPlugin, FCTL_GETPANELITEM, Param1, (void*)Param2); break;
	case Far2::FCTL_GETSELECTEDPANELITEM: //iRc = psi3.PanelControl(hPlugin, FCTL_GETSELECTEDPANELITEM, Param1, (void*)Param2); break;
	case Far2::FCTL_GETCURRENTPANELITEM: //iRc = psi3.PanelControl(hPlugin, FCTL_GETCURRENTPANELITEM, Param1, (void*)Param2); break;
		{
			FILE_CONTROL_COMMANDS Cmd3 = (Command == Far2::FCTL_GETPANELITEM) ? FCTL_GETPANELITEM :
					   (Command == Far2::FCTL_GETSELECTEDPANELITEM) ? FCTL_GETSELECTEDPANELITEM :
					   (Command == Far2::FCTL_GETCURRENTPANELITEM) ? FCTL_GETCURRENTPANELITEM 
					   : (FILE_CONTROL_COMMANDS)0;
			_ASSERTE(Cmd3!=(FILE_CONTROL_COMMANDS)0);
			size_t nItemSize = psi3.PanelControl(hPlugin, Cmd3, Param1, NULL);
			if (Param2 == NULL)
			{
				_ASSERTE(sizeof(PluginPanelItem)>=sizeof(Far2::PluginPanelItem));
				iRc = nItemSize;
			}
			else if (nItemSize)
			{
				Far2::PluginPanelItem* p2 = (Far2::PluginPanelItem*)Param2;
				FarGetPluginPanelItem p3 = {nItemSize};
				p3.Item = (PluginPanelItem*)malloc(nItemSize);
				if (p3.Item)
				{
					iRc = psi3.PanelControl(hPlugin, Cmd3, Param1, &p3);
					if (iRc)
					{
						PluginPanelItem_3_2(p3.Item, p2);
						// Обработка строк
						wchar_t* psz = (wchar_t*)(((LPBYTE)p2)+sizeof(*p2));
						if (p3.Item->FileName)
						{
							p2->FindData.lpwszFileName = psz;
							lstrcpy(psz, p3.Item->FileName);
							psz += lstrlen(psz)+1;
						}
						if (p3.Item->AlternateFileName)
						{
							p2->FindData.lpwszAlternateFileName = psz;
							lstrcpy(psz, p3.Item->AlternateFileName);
							psz += lstrlen(psz)+1;
						}
						if (p3.Item->Description)
						{
							p2->Description = psz;
							lstrcpy(psz, p3.Item->Description);
							psz += lstrlen(psz)+1;
						}
						if (p3.Item->Owner)
						{
							p2->Owner = psz;
							lstrcpy(psz, p3.Item->Owner);
							psz += lstrlen(psz)+1;
						}
						//TODO: CustomColumnData/CustomColumnNumber
						p2->CustomColumnData = NULL;
						p2->CustomColumnNumber = 0;
					}
					free(p3.Item);
				}
			}
		}
		break;
	case Far2::FCTL_GETPANELDIR: iRc = psi3.PanelControl(hPlugin, FCTL_GETPANELDIR, Param1, (void*)Param2); break;
	case Far2::FCTL_GETCOLUMNTYPES: iRc = psi3.PanelControl(hPlugin, FCTL_GETCOLUMNTYPES, Param1, (void*)Param2); break;
	case Far2::FCTL_GETCOLUMNWIDTHS: iRc = psi3.PanelControl(hPlugin, FCTL_GETCOLUMNWIDTHS, Param1, (void*)Param2); break;
	case Far2::FCTL_BEGINSELECTION: iRc = psi3.PanelControl(hPlugin, FCTL_BEGINSELECTION, Param1, (void*)Param2); break;
	case Far2::FCTL_ENDSELECTION: iRc = psi3.PanelControl(hPlugin, FCTL_ENDSELECTION, Param1, (void*)Param2); break;
	case Far2::FCTL_CLEARSELECTION: iRc = psi3.PanelControl(hPlugin, FCTL_CLEARSELECTION, Param1, (void*)Param2); break;
	case Far2::FCTL_SETDIRECTORIESFIRST: iRc = psi3.PanelControl(hPlugin, FCTL_SETDIRECTORIESFIRST, Param1, (void*)Param2); break;
	}
	return iRc;
};
int WINAPI WrapPluginInfo::FarApiGetDirList(const wchar_t *Dir, struct Far2::FAR_FIND_DATA **pPanelItem, int *pItemsNumber)
{
	LOG_CMD(L"psi2.GetDirList",0,0,0);
	//TODO:
	return 0;
};
void WINAPI WrapPluginInfo::FarApiFreeDirList(struct Far2::FAR_FIND_DATA *PanelItem, int nItemsNumber)
{
	LOG_CMD(L"psi2.FreeDirList",0,0,0);
	//TODO:
};
int WINAPI WrapPluginInfo::FarApiGetPluginDirList(INT_PTR PluginNumber, HANDLE hPlugin, const wchar_t *Dir, struct Far2::PluginPanelItem **pPanelItem, int *pItemsNumber)
{
	LOG_CMD(L"psi2.GetPluginDirList",0,0,0);
	//TODO:
	return 0;
};
void WINAPI WrapPluginInfo::FarApiFreePluginDirList(struct Far2::PluginPanelItem *PanelItem, int nItemsNumber)
{
	LOG_CMD(L"psi2.FreePluginDirList",0,0,0);
	//TODO:
};
int WINAPI WrapPluginInfo::FarApiCmpName(const wchar_t *Pattern, const wchar_t *String, int SkipPath)
{
	LOG_CMD(L"psi2.CmdName",0,0,0);
	wchar_t* pszFile = (wchar_t*)(SkipPath ? FSF3.PointToName(String) : String);
	int iRc = FSF3.ProcessName(Pattern, pszFile, 0, PN_CMPNAME);
	return iRc;
};
const wchar_t* WINAPI WrapPluginInfo::FarApiGetMsg(INT_PTR PluginNumber, int MsgId)
{
	LOG_CMD(L"psi2.GetMsg(%i)",MsgId,0,0);
	if (wpi && ((INT_PTR)wpi->hDll) == PluginNumber)
		return psi3.GetMsg(&wpi->guid_Plugin, MsgId);
	return L"";
};
INT_PTR WINAPI WrapPluginInfo::FarApiAdvControl(INT_PTR ModuleNumber, int Command, void *Param)
{
	LOG_CMD(L"psi2.AdvControl(%i)",Command,0,0);
	INT_PTR iRc = 0;
	if (((INT_PTR)wpi->hDll) != ModuleNumber)
	{
		_ASSERTE(((INT_PTR)wpi->hDll) == ModuleNumber);
		iRc = 0;
	}
	else switch (Command)
	{
	case Far2::ACTL_GETFARVERSION:
		{
			VersionInfo vi = {0};
			iRc = psi3.AdvControl(&wpi->guid_Plugin, ACTL_GETFARMANAGERVERSION, 0, &vi);
			if (iRc)
			{
				*((DWORD*)Param) = MAKEFARVERSION2(vi.Major, vi.Minor, vi.Build);
			}
			break;
		}
	case Far2::ACTL_GETSYSWORDDIV: iRc = psi3.AdvControl(&wpi->guid_Plugin, ACTL_GETSYSWORDDIV, 0, Param); break;
	case Far2::ACTL_WAITKEY: iRc = psi3.AdvControl(&wpi->guid_Plugin, ACTL_WAITKEY, 0, Param); break;
	case Far2::ACTL_GETCOLOR: iRc = psi3.AdvControl(&wpi->guid_Plugin, ACTL_GETCOLOR, 0, Param); break;
	case Far2::ACTL_GETARRAYCOLOR: iRc = psi3.AdvControl(&wpi->guid_Plugin, ACTL_GETARRAYCOLOR, 0, Param); break;
	case Far2::ACTL_EJECTMEDIA: iRc = psi3.AdvControl(&wpi->guid_Plugin, ACTL_EJECTMEDIA, 0, Param); break;
	case Far2::ACTL_KEYMACRO:
		{
			if (Param)
			{
				Far2::ActlKeyMacro* p2 = (Far2::ActlKeyMacro*)Param;
				//iRc = psi3.AdvControl(&wpi->guid_Plugin, ACTL_KEYMACRO, Param);
				switch (p2->Command)
				{
				case Far2::MCMD_LOADALL:
					iRc = psi3.MacroControl(INVALID_HANDLE_VALUE, MCTL_LOADALL, 0, 0);
					break;
				case Far2::MCMD_SAVEALL:
					iRc = psi3.MacroControl(INVALID_HANDLE_VALUE, MCTL_LOADALL, 0, 0);
					break;
				case Far2::MCMD_POSTMACROSTRING:
					{
						MacroSendMacroText mcr = {sizeof(MacroSendMacroText)};
						mcr.SequenceText = p2->Param.PlainText.SequenceText;
						mcr.Flags = 0
							| ((p2->Param.PlainText.Flags & Far2::KSFLAGS_DISABLEOUTPUT) ? KMFLAGS_DISABLEOUTPUT : 0)
							| ((p2->Param.PlainText.Flags & Far2::KSFLAGS_NOSENDKEYSTOPLUGINS) ? KMFLAGS_NOSENDKEYSTOPLUGINS : 0)
							| ((p2->Param.PlainText.Flags & Far2::KSFLAGS_REG_MULTI_SZ) ? KMFLAGS_REG_MULTI_SZ : 0)
							| ((p2->Param.PlainText.Flags & Far2::KSFLAGS_SILENTCHECK) ? KMFLAGS_SILENTCHECK : 0);
						psi3.MacroControl(INVALID_HANDLE_VALUE, MCTL_SENDSTRING, 0, &mcr);
					}
					break;
				case Far2::MCMD_CHECKMACRO:
					{
						MacroCheckMacroText mcr = {{sizeof(MacroCheckMacroText)}};
						mcr.Text.SequenceText = p2->Param.PlainText.SequenceText;
						mcr.Text.Flags = 0
							| ((p2->Param.PlainText.Flags & Far2::KSFLAGS_DISABLEOUTPUT) ? KMFLAGS_DISABLEOUTPUT : 0)
							| ((p2->Param.PlainText.Flags & Far2::KSFLAGS_NOSENDKEYSTOPLUGINS) ? KMFLAGS_NOSENDKEYSTOPLUGINS : 0)
							| ((p2->Param.PlainText.Flags & Far2::KSFLAGS_REG_MULTI_SZ) ? KMFLAGS_REG_MULTI_SZ : 0)
							| ((p2->Param.PlainText.Flags & Far2::KSFLAGS_SILENTCHECK) ? KMFLAGS_SILENTCHECK : 0);
						iRc = psi3.MacroControl(INVALID_HANDLE_VALUE, MCTL_SENDSTRING, MSSC_CHECK, &mcr);
						p2->Param.MacroResult.ErrCode = mcr.Result.ErrCode;
						p2->Param.MacroResult.ErrPos = mcr.Result.ErrPos;
						p2->Param.MacroResult.ErrSrc = mcr.Result.ErrSrc;
					}
					break;
				case Far2::MCMD_GETSTATE:
					iRc = psi3.MacroControl(INVALID_HANDLE_VALUE, MCTL_GETSTATE, 0, 0);
					break;
				case 6/*Far2::MCMD_GETAREA*/:
					iRc = psi3.MacroControl(INVALID_HANDLE_VALUE, MCTL_GETAREA, 0, 0);
					break;
				}
			}
			break;
		}
	case Far2::ACTL_POSTKEYSEQUENCE:
		//TODO: Нужно переделать на MCTL_SENDSTRING
		_ASSERTE(Command != Far2::ACTL_POSTKEYSEQUENCE);
		break;
	case Far2::ACTL_GETWINDOWINFO: iRc = psi3.AdvControl(&wpi->guid_Plugin, ACTL_GETWINDOWINFO, 0, Param); break;
	case Far2::ACTL_GETWINDOWCOUNT: iRc = psi3.AdvControl(&wpi->guid_Plugin, ACTL_GETWINDOWCOUNT, 0, Param); break;
	case Far2::ACTL_SETCURRENTWINDOW: iRc = psi3.AdvControl(&wpi->guid_Plugin, ACTL_SETCURRENTWINDOW, 0, Param); break;
	case Far2::ACTL_COMMIT: iRc = psi3.AdvControl(&wpi->guid_Plugin, ACTL_COMMIT, 0, Param); break;
	case Far2::ACTL_GETFARHWND: iRc = psi3.AdvControl(&wpi->guid_Plugin, ACTL_GETFARHWND, 0, Param); break;
	case Far2::ACTL_GETSYSTEMSETTINGS: iRc = psi3.AdvControl(&wpi->guid_Plugin, ACTL_GETSYSTEMSETTINGS, 0, Param); break;
	case Far2::ACTL_GETPANELSETTINGS: iRc = psi3.AdvControl(&wpi->guid_Plugin, ACTL_GETPANELSETTINGS, 0, Param); break;
	case Far2::ACTL_GETINTERFACESETTINGS: iRc = psi3.AdvControl(&wpi->guid_Plugin, ACTL_GETINTERFACESETTINGS, 0, Param); break;
	case Far2::ACTL_GETCONFIRMATIONS: iRc = psi3.AdvControl(&wpi->guid_Plugin, ACTL_GETCONFIRMATIONS, 0, Param); break;
	case Far2::ACTL_GETDESCSETTINGS: iRc = psi3.AdvControl(&wpi->guid_Plugin, ACTL_GETDESCSETTINGS, 0, Param); break;
	case Far2::ACTL_SETARRAYCOLOR: iRc = psi3.AdvControl(&wpi->guid_Plugin, ACTL_SETARRAYCOLOR, 0, Param); break;
	case Far2::ACTL_GETPLUGINMAXREADDATA: iRc = psi3.AdvControl(&wpi->guid_Plugin, ACTL_GETPLUGINMAXREADDATA, 0, Param); break;
	case Far2::ACTL_GETDIALOGSETTINGS: iRc = psi3.AdvControl(&wpi->guid_Plugin, ACTL_GETDIALOGSETTINGS, 0, Param); break;
	case Far2::ACTL_GETSHORTWINDOWINFO:
		{
			Far2::WindowInfo* p2 = (Far2::WindowInfo*)Param;
			if (p2->Pos == -1)
			{
				memset(p2, 0, sizeof(*p2));
				p2->Pos = -1;
				//iRc = psi3.AdvControl(&wpi->guid_Plugin, ACTL_GETSHORTWINDOWINFO, Param);
				INT_PTR nArea = psi3.MacroControl(INVALID_HANDLE_VALUE, MCTL_GETAREA, 0, 0);
				switch(nArea)
				{
					case MACROAREA_SHELL:
					case MACROAREA_INFOPANEL:
					case MACROAREA_QVIEWPANEL:
					case MACROAREA_TREEPANEL:
						p2->Type = WTYPE_PANELS;
					case MACROAREA_VIEWER:
						p2->Type = WTYPE_VIEWER;
					case MACROAREA_EDITOR:
						p2->Type = WTYPE_EDITOR;
					case MACROAREA_DIALOG:
					case MACROAREA_SEARCH:
					case MACROAREA_DISKS:
					case MACROAREA_FINDFOLDER:
					case MACROAREA_AUTOCOMPLETION:
						p2->Type = WTYPE_DIALOG;
					case MACROAREA_HELP:
						p2->Type = WTYPE_HELP;
					case MACROAREA_MAINMENU:
					case MACROAREA_MENU:
					case MACROAREA_USERMENU:
						p2->Type = WTYPE_VMENU;
					//case MACROAREA_OTHER: // Grabber
					//	return -1;
				}
			}
		}
		break;
	case Far2::ACTL_REDRAWALL:
		iRc = psi3.AdvControl(&wpi->guid_Plugin, ACTL_REDRAWALL, 0, Param); break;
	case Far2::ACTL_SYNCHRO:
		iRc = psi3.AdvControl(&wpi->guid_Plugin, ACTL_SYNCHRO, 0, Param); break;
	case Far2::ACTL_SETPROGRESSSTATE: iRc = psi3.AdvControl(&wpi->guid_Plugin, ACTL_SETPROGRESSSTATE, 0, Param); break;
	case Far2::ACTL_SETPROGRESSVALUE: iRc = psi3.AdvControl(&wpi->guid_Plugin, ACTL_SETPROGRESSVALUE, 0, Param); break;
	case Far2::ACTL_QUIT: iRc = psi3.AdvControl(&wpi->guid_Plugin, ACTL_QUIT, 0, Param); break;
	case Far2::ACTL_GETFARRECT:
		iRc = psi3.AdvControl(&wpi->guid_Plugin, ACTL_GETFARRECT, 0, Param); break;
	case Far2::ACTL_GETCURSORPOS: iRc = psi3.AdvControl(&wpi->guid_Plugin, ACTL_GETCURSORPOS, 0, Param); break;
	case Far2::ACTL_SETCURSORPOS: iRc = psi3.AdvControl(&wpi->guid_Plugin, ACTL_SETCURSORPOS, 0, Param); break;
	}
	return iRc;
};
int WINAPI WrapPluginInfo::FarApiViewerControl(int Command, void *Param)
{
	LOG_CMD(L"psi2.ViewerControl",0,0,0);
	int nRc = 0; //psi3.ViewerControl(-1, Command, 0, Param);
	//TODO: Для некоторых функций нужна конвертация аргументов
	switch (Command)
	{
	case Far2::VCTL_GETINFO:
		{
			Far2::ViewerInfo* p2 = (Far2::ViewerInfo*)Param;
			ViewerInfo p3 = {sizeof(ViewerInfo)};
			p3.ViewerID = p2->ViewerID;
			nRc = psi3.ViewerControl(-1, VCTL_GETINFO, 0, &p3);
			p2->ViewerID = p3.ViewerID;
			p2->FileName = p3.FileName;
			p2->FileSize = p3.FileSize;
			p2->FilePos = p3.FilePos;
			p2->WindowSizeX = p3.WindowSizeX;
			p2->WindowSizeY = p3.WindowSizeY;
			p2->Options = p3.Options;
			p2->TabSize = p3.TabSize;
			p2->CurMode.CodePage = p3.CurMode.CodePage;
			p2->CurMode.Wrap = p3.CurMode.Wrap;
			p2->CurMode.WordWrap = p3.CurMode.WordWrap;
			p2->CurMode.Hex = p3.CurMode.Hex;
			p2->LeftPos = p3.LeftPos;
		}
		break;
	case Far2::VCTL_QUIT: nRc = psi3.ViewerControl(-1, VCTL_QUIT, 0, (void*)Param); break;
	case Far2::VCTL_REDRAW: nRc = psi3.ViewerControl(-1, VCTL_REDRAW, 0, (void*)Param); break;
	case Far2::VCTL_SETKEYBAR:
		{
			KeyBarTitles* p3 = (KeyBarTitles*)Param;
			if (Param && (Param != (void*)-1))
			{
				const Far2::KeyBarTitles* p2 = (const Far2::KeyBarTitles*)Param;
				p3 = wpi->KeyBarTitles_2_3(p2);
			}
			nRc = psi3.ViewerControl(-1, VCTL_SETKEYBAR, 0, p3);
		}
		break;
	case Far2::VCTL_SETPOSITION:
		{
			const Far2::ViewerSetPosition* p2 = (const Far2::ViewerSetPosition*)Param;
			//TODO: Конвертация флагов
			ViewerSetPosition p3 = {(VIEWER_SETPOS_FLAGS)p2->Flags};
			_ASSERTE(p3.Flags == (VIEWER_SETPOS_FLAGS)p2->Flags); // добавят cbStructSize?
			p3.StartPos = p2->StartPos;
			p3.LeftPos = p2->LeftPos;
			nRc = psi3.ViewerControl(-1, VCTL_SETPOSITION, 0, &p3);
		}
		break;
	case Far2::VCTL_SELECT:
		_ASSERTE(sizeof(Far2::ViewerSelect)==sizeof(ViewerSelect));
		nRc = psi3.ViewerControl(-1, VCTL_SELECT, 0, (void*)Param);
		break;
	case Far2::VCTL_SETMODE:
		{
			const Far2::ViewerSetMode* p2 = (const Far2::ViewerSetMode*)Param;
			//TODO: Конвертация типа
			ViewerSetMode p3 = {(VIEWER_SETMODE_TYPES)p2->Type};
			_ASSERTE(p3.Type == (VIEWER_SETMODE_TYPES)p2->Type); // добавят cbStructSize?
			p3.wszParam = p2->Param.wszParam; // наиболее толстый из union
			//TODO: Конвертация флагов
			p3.Flags = p2->Flags;
			nRc = psi3.ViewerControl(-1, VCTL_SETMODE, 0, &p3);
		}
		break;
	}
	return nRc;
};
int WINAPI WrapPluginInfo::FarApiEditorControl(int Command, void *Param)
{
	LOG_CMD0(L"psi2.EditorControl(%i)",Command,0,0);
	int nRc = 0; //psi3.EditorControl(-1, Command, 0, Param);
	//TODO: Для некоторых функций нужна конвертация аргументов
	switch (Command)
	{
	case Far2::ECTL_GETSTRING:
		_ASSERTE(sizeof(Far2::EditorGetString)==sizeof(EditorGetString));
		nRc = psi3.EditorControl(-1, ECTL_GETSTRING, 0, (void*)Param);
		break;
	case Far2::ECTL_SETSTRING:
		_ASSERTE(sizeof(Far2::EditorSetString)==sizeof(EditorSetString));
		nRc = psi3.EditorControl(-1, ECTL_SETSTRING, 0, (void*)Param);
		break;
	case Far2::ECTL_INSERTSTRING:
		nRc = psi3.EditorControl(-1, ECTL_INSERTSTRING, 0, (void*)Param);
		break;
	case Far2::ECTL_DELETESTRING:
		nRc = psi3.EditorControl(-1, ECTL_DELETESTRING, 0, (void*)Param);
		break;
	case Far2::ECTL_DELETECHAR:
		nRc = psi3.EditorControl(-1, ECTL_DELETECHAR, 0, (void*)Param);
		break;
	case Far2::ECTL_INSERTTEXT:
		nRc = psi3.EditorControl(-1, ECTL_INSERTTEXT, 0, (void*)Param);
		break;
	case Far2::ECTL_GETINFO:
		{
			Far2::EditorInfo* p2 = (Far2::EditorInfo*)Param;
			EditorInfo p3 = {0};
			p3.EditorID = p2->EditorID;
			nRc = psi3.EditorControl(-1, ECTL_GETINFO, 0, &p3);
			p2->EditorID = p3.EditorID;
			p2->WindowSizeX = p3.WindowSizeX;
			p2->WindowSizeY = p3.WindowSizeY;
			p2->TotalLines = p3.TotalLines;
			p2->CurLine = p3.CurLine;
			p2->CurPos = p3.CurPos;
			p2->CurTabPos = p3.CurTabPos;
			p2->TopScreenLine = p3.TopScreenLine;
			p2->LeftPos = p3.LeftPos;
			p2->Overtype = p3.Overtype;
			p2->BlockType = p3.BlockType;
			p2->BlockStartLine = p3.BlockStartLine;
			p2->Options = p3.Options;
			p2->TabSize = p3.TabSize;
			p2->BookMarkCount = p3.BookMarkCount;
			p2->CurState = p3.CurState;
			p2->CodePage = p3.CodePage;
		}
		break;
	case Far2::ECTL_SETPOSITION:
		_ASSERTE(sizeof(Far2::EditorSetPosition)==sizeof(EditorSetPosition));
		nRc = psi3.EditorControl(-1, ECTL_SETPOSITION, 0, (void*)Param);
		break;
	case Far2::ECTL_SELECT:
		_ASSERTE(sizeof(Far2::EditorSelect)==sizeof(EditorSelect));
		nRc = psi3.EditorControl(-1, ECTL_SELECT, 0, (void*)Param);
		break;
	case Far2::ECTL_REDRAW: nRc = psi3.EditorControl(-1, ECTL_REDRAW, 0, (void*)Param); break;
	case Far2::ECTL_TABTOREAL:
		_ASSERTE(sizeof(Far2::EditorConvertPos)==sizeof(EditorConvertPos));
		nRc = psi3.EditorControl(-1, ECTL_TABTOREAL, 0, (void*)Param);
		break;
	case Far2::ECTL_REALTOTAB:
		_ASSERTE(sizeof(Far2::EditorConvertPos)==sizeof(EditorConvertPos));
		nRc = psi3.EditorControl(-1, ECTL_REALTOTAB, 0, (void*)Param);
		break;
	case Far2::ECTL_EXPANDTABS: nRc = psi3.EditorControl(-1, ECTL_EXPANDTABS, 0, (void*)Param); break;
	case Far2::ECTL_SETTITLE: nRc = psi3.EditorControl(-1, ECTL_SETTITLE, 0, (void*)Param); break;
	case Far2::ECTL_READINPUT: nRc = psi3.EditorControl(-1, ECTL_READINPUT, 0, (void*)Param); break;
	case Far2::ECTL_PROCESSINPUT: nRc = psi3.EditorControl(-1, ECTL_PROCESSINPUT, 0, (void*)Param); break;
	case Far2::ECTL_ADDCOLOR:
		{
			const Far2::EditorColor* p2 = (const Far2::EditorColor*)Param;
			EditorColor p3 = {sizeof(EditorColor)};
			p3.StringNumber = p2->StringNumber;
			p3.ColorItem = p2->ColorItem;
			p3.StartPos = p2->StartPos;
			p3.EndPos = p2->EndPos;
			p3.Color.Flags = FMSG_FG_4BIT|FMSG_BG_4BIT;
			p3.Color.ForegroundColor = p2->Color & 0xF;
			p3.Color.BackgroundColor = (p2->Color & 0xF0) >> 4;
			nRc = psi3.EditorControl(-1, ECTL_ADDCOLOR, 0, &p3);
		}
		break;
	case Far2::ECTL_GETCOLOR:
		{
			Far2::EditorColor* p2 = (Far2::EditorColor*)Param;
			EditorColor p3 = {sizeof(EditorColor)};
			p3.StringNumber = p2->StringNumber;
			p3.ColorItem = p2->ColorItem;
			p3.StartPos = p2->StartPos;
			p3.EndPos = p2->EndPos;
			p3.Color.Flags = FMSG_FG_4BIT|FMSG_BG_4BIT;
			nRc = psi3.EditorControl(-1, ECTL_GETCOLOR, 0, &p3);
			if (nRc)
			{
				_ASSERTE(p3.Color.Flags == (FMSG_FG_4BIT|FMSG_BG_4BIT));
				p2->Color = (p3.Color.ForegroundColor & 0xF) | ((p3.Color.BackgroundColor & 0xF) << 4);
			}
		}
		break;
	case Far2::ECTL_SAVEFILE:
		_ASSERTE(sizeof(Far2::EditorSaveFile)==sizeof(EditorSaveFile));
		nRc = psi3.EditorControl(-1, ECTL_SAVEFILE, 0, (void*)Param);
		break;
	case Far2::ECTL_QUIT: nRc = psi3.EditorControl(-1, ECTL_QUIT, 0, (void*)Param); break;
	case Far2::ECTL_SETKEYBAR:
		{
			KeyBarTitles* p3 = (KeyBarTitles*)Param;
			if (Param && (Param != (void*)-1))
			{
				const Far2::KeyBarTitles* p2 = (const Far2::KeyBarTitles*)Param;
				p3 = wpi->KeyBarTitles_2_3(p2);
			}
			nRc = psi3.EditorControl(-1, ECTL_SETKEYBAR, 0, p3);
		}
		break;
	case Far2::ECTL_PROCESSKEY: nRc = psi3.EditorControl(-1, ECTL_PROCESSKEY, 0, (void*)Param); break;
	case Far2::ECTL_SETPARAM:
		{
			const Far2::EditorSetParameter* p2 = (const Far2::EditorSetParameter*)Param;
			EditorSetParameter p3 = {(EDITOR_SETPARAMETER_TYPES)-1};
			p3.wszParam = p2->Param.wszParam; // наиболее "объемный" из union
			//TODO: Конвертация флагов?
			p3.Flags = p2->Flags;
			p3.Size = p2->Size;
			switch (p2->Type)
			{
			case Far2::ESPT_TABSIZE: p3.Type = ESPT_TABSIZE; break;
			case Far2::ESPT_EXPANDTABS: p3.Type = ESPT_EXPANDTABS; break;
			case Far2::ESPT_AUTOINDENT: p3.Type = ESPT_AUTOINDENT; break;
			case Far2::ESPT_CURSORBEYONDEOL: p3.Type = ESPT_CURSORBEYONDEOL; break;
			case Far2::ESPT_CHARCODEBASE: p3.Type = ESPT_CHARCODEBASE; break;
			case Far2::ESPT_CODEPAGE: p3.Type = ESPT_CODEPAGE; break;
			case Far2::ESPT_SAVEFILEPOSITION: p3.Type = ESPT_SAVEFILEPOSITION; break;
			case Far2::ESPT_LOCKMODE: p3.Type = ESPT_LOCKMODE; break;
			case Far2::ESPT_SETWORDDIV: p3.Type = ESPT_SETWORDDIV; break;
			case Far2::ESPT_GETWORDDIV: p3.Type = ESPT_GETWORDDIV; break;
			case Far2::ESPT_SHOWWHITESPACE: p3.Type = ESPT_SHOWWHITESPACE; break;
			case Far2::ESPT_SETBOM: p3.Type = ESPT_SETBOM; break;
			}
			if (p3.Type != (EDITOR_SETPARAMETER_TYPES)-1)
				nRc = psi3.EditorControl(-1, ECTL_SETPARAM, 0, &p3);
		}
		break;
	case Far2::ECTL_GETBOOKMARKS: nRc = psi3.EditorControl(-1, ECTL_GETBOOKMARKS, 0, (void*)Param); break;
	case Far2::ECTL_TURNOFFMARKINGBLOCK: nRc = psi3.EditorControl(-1, ECTL_TURNOFFMARKINGBLOCK, 0, (void*)Param); break;
	case Far2::ECTL_DELETEBLOCK: nRc = psi3.EditorControl(-1, ECTL_DELETEBLOCK, 0, (void*)Param); break;
	case Far2::ECTL_ADDSTACKBOOKMARK: nRc = psi3.EditorControl(-1, ECTL_ADDSTACKBOOKMARK, 0, (void*)Param); break;
	case Far2::ECTL_PREVSTACKBOOKMARK: nRc = psi3.EditorControl(-1, ECTL_PREVSTACKBOOKMARK, 0, (void*)Param); break;
	case Far2::ECTL_NEXTSTACKBOOKMARK: nRc = psi3.EditorControl(-1, ECTL_NEXTSTACKBOOKMARK, 0, (void*)Param); break;
	case Far2::ECTL_CLEARSTACKBOOKMARKS: nRc = psi3.EditorControl(-1, ECTL_CLEARSTACKBOOKMARKS, 0, (void*)Param); break;
	case Far2::ECTL_DELETESTACKBOOKMARK: nRc = psi3.EditorControl(-1, ECTL_DELETESTACKBOOKMARK, 0, (void*)Param); break;
	case Far2::ECTL_GETSTACKBOOKMARKS: nRc = psi3.EditorControl(-1, ECTL_GETSTACKBOOKMARKS, 0, (void*)Param); break;
	case Far2::ECTL_UNDOREDO:
		{
			const Far2::EditorUndoRedo* p2 = (const Far2::EditorUndoRedo*)Param;
			EditorUndoRedo p3 = {(EDITOR_UNDOREDO_COMMANDS)0};
			//TODO: Конвертация команды
			p3.Command = (EDITOR_UNDOREDO_COMMANDS)p2->Command;
			nRc = psi3.EditorControl(-1, ECTL_UNDOREDO, 0, &p3);
		}
		break;
	case Far2::ECTL_GETFILENAME: nRc = psi3.EditorControl(-1, ECTL_GETFILENAME, 0, (void*)Param); break;
	}
	return nRc;
};
int WINAPI WrapPluginInfo::FarApiInputBox(const wchar_t *Title, const wchar_t *SubTitle, const wchar_t *HistoryName, const wchar_t *SrcText, wchar_t *DestText, int   DestLength, const wchar_t *HelpTopic, DWORD Flags)
{
	LOG_CMD(L"psi2.InputBox",0,0,0);
	//TODO: Флаги
	int nRc = psi3.InputBox(&wpi->guid_Plugin, Title, SubTitle, HistoryName, SrcText, DestText, DestLength, HelpTopic, Flags);
	return nRc;
};
int WINAPI WrapPluginInfo::FarApiPluginsControl(HANDLE hHandle, int Command, int Param1, LONG_PTR Param2)
{
	LOG_CMD(L"psi2.PluginsControl",0,0,0);
	int nRc = 0; //psi3.PluginsControl(hHandle, Command, Param1, Param2);
	switch (Command)
	{
	case Far2::PCTL_LOADPLUGIN:
		nRc = psi3.PluginsControl(hHandle, PCTL_LOADPLUGIN, Param1, (void*)Param2); break;
	case Far2::PCTL_UNLOADPLUGIN:
		nRc = psi3.PluginsControl(hHandle, PCTL_UNLOADPLUGIN, Param1, (void*)Param2); break;
	case 2/*Far2::PCTL_FORCEDLOADPLUGIN*/:
		nRc = psi3.PluginsControl(hHandle, PCTL_FORCEDLOADPLUGIN, Param1, (void*)Param2); break;
	}
	return nRc;
};
int WINAPI WrapPluginInfo::FarApiFileFilterControl(HANDLE hHandle, int Command, int Param1, LONG_PTR Param2)
{
	LOG_CMD(L"psi2.FileFilterControl",0,0,0);
	int nRc = 0; //psi3.FileFilterControl(hHandle, Command, Param1, Param2);
	switch (Command)
	{
	case Far2::FFCTL_CREATEFILEFILTER:
		nRc = psi3.FileFilterControl(hHandle, FFCTL_CREATEFILEFILTER, Param1, (void*)Param2); break;
	case Far2::FFCTL_FREEFILEFILTER:
		nRc = psi3.FileFilterControl(hHandle, FFCTL_FREEFILEFILTER, Param1, (void*)Param2); break;
	case Far2::FFCTL_OPENFILTERSMENU:
		nRc = psi3.FileFilterControl(hHandle, FFCTL_OPENFILTERSMENU, Param1, (void*)Param2); break;
	case Far2::FFCTL_STARTINGTOFILTER:
		nRc = psi3.FileFilterControl(hHandle, FFCTL_STARTINGTOFILTER, Param1, (void*)Param2); break;
	case Far2::FFCTL_ISFILEINFILTER:
		nRc = psi3.FileFilterControl(hHandle, FFCTL_ISFILEINFILTER, Param1, (void*)Param2); break;
	}
	return nRc;
};
int WINAPI WrapPluginInfo::FarApiRegExpControl(HANDLE hHandle, int Command, LONG_PTR Param)
{
	LOG_CMD(L"psi2.RegExpControl",0,0,0);
	int nRc = 0; //psi3.RegExpControl(hHandle, Command, 0, (void*)Param);
	switch (Command)
	{
	case Far2::RECTL_CREATE:
		nRc = psi3.RegExpControl(hHandle, RECTL_CREATE, 0, (void*)Param); break;
	case Far2::RECTL_FREE:
		nRc = psi3.RegExpControl(hHandle, RECTL_FREE, 0, (void*)Param); break;
	case Far2::RECTL_COMPILE:
		nRc = psi3.RegExpControl(hHandle, RECTL_COMPILE, 0, (void*)Param); break;
	case Far2::RECTL_OPTIMIZE:
		nRc = psi3.RegExpControl(hHandle, RECTL_OPTIMIZE, 0, (void*)Param); break;
	case Far2::RECTL_MATCHEX:
		nRc = psi3.RegExpControl(hHandle, RECTL_MATCHEX, 0, (void*)Param); break;
	case Far2::RECTL_SEARCHEX:
		nRc = psi3.RegExpControl(hHandle, RECTL_SEARCHEX, 0, (void*)Param); break;
	case Far2::RECTL_BRACKETSCOUNT:
		nRc = psi3.RegExpControl(hHandle, RECTL_BRACKETSCOUNT, 0, (void*)Param); break;
	}
	return nRc;
};
//struct int WINAPIV FARSTDSPRINTF(wchar_t *Buffer,const wchar_t *Format,...);
//struct int WINAPIV FARSTDSNPRINTF(wchar_t *Buffer,size_t Sizebuf,const wchar_t *Format,...);
//struct int WINAPIV FARSTDSSCANF(const wchar_t *Buffer, const wchar_t *Format,...);
//struct void WINAPI FARSTDQSORT(void *base, size_t nelem, size_t width, int (__cdecl *fcmp)(const void *, const void *));
//struct void WINAPI FARSTDQSORTEX(void *base, size_t nelem, size_t width, int (__cdecl *fcmp)(const void *, const void *,void *userparam),void *userparam);
//struct void   *WINAPI FARSTDBSEARCH(const void *key, const void *base, size_t nelem, size_t width, int (__cdecl *fcmp)(const void *, const void *));
int WINAPI WrapPluginInfo::FarStdGetFileOwner(const wchar_t *Computer,const wchar_t *Name,wchar_t *Owner,int Size)
{
	LOG_CMD(L"fsf2.GetFileOwner",0,0,0);
	__int64 nRc = FSF3.GetFileOwner(Computer, Name, Owner, Size);
	return (int)nRc;
};
//struct int WINAPI FARSTDGETNUMBEROFLINKS(const wchar_t *Name);
//struct int WINAPI FARSTDATOI(const wchar_t *s);
//struct __int64 WINAPI FARSTDATOI64(const wchar_t *s);
//struct wchar_t   *WINAPI FARSTDITOA64(__int64 value, wchar_t *string, int radix);
//struct wchar_t   *WINAPI FARSTDITOA(int value, wchar_t *string, int radix);
//struct wchar_t   *WINAPI FARSTDLTRIM(wchar_t *Str);
//struct wchar_t   *WINAPI FARSTDRTRIM(wchar_t *Str);
//struct wchar_t   *WINAPI FARSTDTRIM(wchar_t *Str);
//struct wchar_t   *WINAPI FARSTDTRUNCSTR(wchar_t *Str,int MaxLength);
//struct wchar_t   *WINAPI FARSTDTRUNCPATHSTR(wchar_t *Str,int MaxLength);
//struct wchar_t   *WINAPI FARSTDQUOTESPACEONLY(wchar_t *Str);
//struct const wchar_t*WINAPI FARSTDPOINTTONAME(const wchar_t *Path);
int WINAPI WrapPluginInfo::FarStdGetPathRoot(const wchar_t *Path,wchar_t *Root, int DestSize)
{
	LOG_CMD(L"fsf2.GetPathRoot",0,0,0);
	__int64 nRc = FSF3.GetPathRoot(Path, Root, DestSize);
	return (int)nRc;
};
//struct BOOL WINAPI FARSTDADDENDSLASH(wchar_t *Path);
//struct int WINAPI FARSTDCOPYTOCLIPBOARD(const wchar_t *Data);
//struct wchar_t *WINAPI FARSTDPASTEFROMCLIPBOARD(void);
//struct int WINAPI FARSTDINPUTRECORDTOKEY(const INPUT_RECORD *r);
//struct int WINAPI FARSTDLOCALISLOWER(wchar_t Ch);
//struct int WINAPI FARSTDLOCALISUPPER(wchar_t Ch);
//struct int WINAPI FARSTDLOCALISALPHA(wchar_t Ch);
//struct int WINAPI FARSTDLOCALISALPHANUM(wchar_t Ch);
//struct wchar_t WINAPI FARSTDLOCALUPPER(wchar_t LowerChar);
//struct wchar_t WINAPI FARSTDLOCALLOWER(wchar_t UpperChar);
//struct void WINAPI FARSTDLOCALUPPERBUF(wchar_t *Buf,int Length);
//struct void WINAPI FARSTDLOCALLOWERBUF(wchar_t *Buf,int Length);
//struct void WINAPI FARSTDLOCALSTRUPR(wchar_t *s1);
//struct void WINAPI FARSTDLOCALSTRLWR(wchar_t *s1);
//struct int WINAPI FARSTDLOCALSTRICMP(const wchar_t *s1,const wchar_t *s2);
//struct int WINAPI FARSTDLOCALSTRNICMP(const wchar_t *s1,const wchar_t *s2,int n);
wchar_t* WINAPI WrapPluginInfo::FarStdXlat(wchar_t *Line,int StartPos,int EndPos,DWORD Flags)
{
	LOG_CMD(L"fsf2.XLat",0,0,0);
	wchar_t* pszRc = FSF3.XLat(Line, StartPos, EndPos, Flags);
	return pszRc;
};
void WINAPI WrapPluginInfo::FarStdRecursiveSearch(const wchar_t *InitDir,const wchar_t *Mask,Far2::FRSUSERFUNC Func,DWORD Flags,void *Param)
{
	LOG_CMD(L"fsf2.RecursiveSearch",0,0,0);
	//FSF3.RecursiveSearch(InitDir, Mask, Func, Flags, Param);
	//TODO:
};
int WINAPI WrapPluginInfo::FarStdMkTemp(wchar_t *Dest, DWORD size, const wchar_t *Prefix)
{
	LOG_CMD(L"fsf2.MkTemp",0,0,0);
	__int64 nRc = FSF3.MkTemp(Dest, size, Prefix);
	return (int)nRc;
};
int WINAPI WrapPluginInfo::FarStdProcessName(const wchar_t *param1, wchar_t *param2, DWORD size, DWORD flags)
{
	LOG_CMD(L"fsf2.ProcessName",0,0,0);
	__int64 nRc = FSF3.ProcessName(param1, param2, size, flags);
	return (int)nRc;
};
int WINAPI WrapPluginInfo::FarStdMkLink(const wchar_t *Src,const wchar_t *Dest,DWORD Flags)
{
	LOG_CMD(L"fsf2.MkLink",0,0,0);
	LINK_TYPE Type3 = (LINK_TYPE)(Flags & 7);
	MKLINK_FLAGS Flags3 = (MKLINK_FLAGS)(Flags & 0x30000);
	BOOL nRc = FSF3.MkLink(Src, Dest, Type3, Flags3);
	return nRc;
};
int WINAPI WrapPluginInfo::FarConvertPath(enum Far2::CONVERTPATHMODES Mode, const wchar_t *Src, wchar_t *Dest, int DestSize)
{
	LOG_CMD(L"fsf2.ConvertPath",0,0,0);
	__int64 nRc = 0;
	switch (Mode)
	{
	case Far2::CPM_FULL:
		nRc = FSF3.ConvertPath(CPM_FULL, Src, Dest, DestSize); break;
	case Far2::CPM_REAL:
		nRc = FSF3.ConvertPath(CPM_REAL, Src, Dest, DestSize); break;
	case Far2::CPM_NATIVE:
		nRc = FSF3.ConvertPath(CPM_NATIVE, Src, Dest, DestSize); break;
	}
	return (int)nRc;
};
int WINAPI WrapPluginInfo::FarGetReparsePointInfo(const wchar_t *Src, wchar_t *Dest,int DestSize)
{
	LOG_CMD(L"fsf2.GetReparsePointInfo",0,0,0);
	__int64 nRc = FSF3.GetReparsePointInfo(Src, Dest, DestSize);
	return (int)nRc;
};
DWORD WINAPI WrapPluginInfo::FarGetCurrentDirectory(DWORD Size,wchar_t* Buffer)
{
	LOG_CMD(L"fsf2.GetCurrentDirectory",0,0,0);
	size_t nRc = FSF3.GetCurrentDirectory(Size, Buffer);
	return (DWORD)nRc;
};

InfoPanelLine* InfoLines_2_3(const Far2::InfoPanelLine *InfoLines, int InfoLinesNumber)
{
	if (wpi && wpi->InfoLines)
	{
		free(wpi->InfoLines);
		wpi->InfoLines = NULL;
	}
	wpi->PanelModesNumber = 0;
	if (InfoLines && InfoLinesNumber > 0)
	{	
		wpi->InfoLines = (InfoPanelLine*)calloc(InfoLinesNumber, sizeof(*wpi->InfoLines));
		for (int i = 0; i < InfoLinesNumber; i++)
		{
			wpi->InfoLines[i].Text = InfoLines[i].Text;
			wpi->InfoLines[i].Data = InfoLines[i].Data;
			wpi->InfoLines[i].Separator = InfoLines[i].Separator;
		}
		wpi->PanelModesNumber = InfoLinesNumber;
	}
	return wpi->InfoLines;
}

PanelMode* PanelModes_2_3(const Far2::PanelMode *PanelModesArray, int PanelModesNumber)
{
	if (wpi && wpi->PanelModesArray)
	{
		free(wpi->PanelModesArray);
		wpi->PanelModesArray = NULL;
	}
	wpi->PanelModesNumber = 0;
	if (PanelModesArray && PanelModesNumber > 0)
	{
		wpi->PanelModesArray = (PanelMode*)calloc(PanelModesNumber, sizeof(*wpi->PanelModesArray));
		for (int i = 0; i < PanelModesNumber; i++)
		{
			wpi->PanelModesArray[i].StructSize = sizeof(*wpi->PanelModesArray);
			wpi->PanelModesArray[i].ColumnTypes = PanelModesArray[i].ColumnTypes;
			wpi->PanelModesArray[i].ColumnWidths = PanelModesArray[i].ColumnWidths;
			wpi->PanelModesArray[i].ColumnTitles = PanelModesArray[i].ColumnTitles;
			wpi->PanelModesArray[i].StatusColumnTypes = PanelModesArray[i].StatusColumnTypes;
			wpi->PanelModesArray[i].StatusColumnWidths = PanelModesArray[i].StatusColumnWidths;
			wpi->PanelModesArray[i].Flags = 
				(PanelModesArray[i].FullScreen ? PMFLAGS_FULLSCREEN : 0) |
				(PanelModesArray[i].DetailedStatus ? PMFLAGS_DETAILEDSTATUS : 0) |
				(PanelModesArray[i].AlignExtensions ? PMFLAGS_ALIGNEXTENSIONS : 0) |
				(PanelModesArray[i].CaseConversion ? PMFLAGS_CASECONVERSION : 0);
		}
		wpi->PanelModesNumber = PanelModesNumber;
	}
	return wpi->PanelModesArray;
}


void LoadPluginInfo()
{
	if (wpi)
		return; // уже
		
	BOOL lbRc = FALSE;
	wchar_t szSelf[MAX_PATH+1]; szSelf[0] = 0;
	wchar_t szIni[MAX_PATH+1], szTemp[2048];
	wpi = new WrapPluginInfo;
	
	if (GetModuleFileName(ghInstance, szSelf, ARRAYSIZE(szSelf)-4))
	{
		lstrcpy(szIni, szSelf);
		wchar_t* pszSlash = wcsrchr(szIni, L'\\');
		if (!pszSlash) pszSlash = szIni;
		wchar_t* pszDot = wcsrchr(pszSlash, L'.');
		if (pszDot)
			*pszDot = 0;
		lstrcat(szIni, L".ini");
		lstrcpy(wpi->IniFile, szIni);

		lstrcpy(wpi->PluginDll, szSelf);
		pszSlash = wcsrchr(wpi->PluginDll, L'\\');
		if (pszSlash) pszSlash++; else pszSlash = wpi->PluginDll;
		if (!GetPrivateProfileString(L"Plugin", L"PluginFile", L"", pszSlash, ARRAYSIZE(wpi->PluginDll)-lstrlen(wpi->PluginDll), szIni))
			wpi->PluginDll[0] = 0;
		else
			lstrcpyn(wpi->File, pszSlash, ARRAYSIZE(wpi->File));
			
		wpi->AnalyzeMode = GetPrivateProfileInt(L"Plugin", L"AnalyzeMode", 2, szIni);
		if (wpi->AnalyzeMode != 1 && wpi->AnalyzeMode != 2)
			wpi->AnalyzeMode = 2;

		wpi->OldPutFilesParams = GetPrivateProfileInt(L"Plugin", L"OldPutFilesParams", 0, szIni);
		if (wpi->OldPutFilesParams != 0 && wpi->OldPutFilesParams != 1)
			wpi->OldPutFilesParams = 0;
		
		if (GetPrivateProfileString(L"Plugin", L"Title", L"Sample Far3 plugin", szTemp, ARRAYSIZE(szTemp), szIni))
			lstrcpyn(wpi->Title, szTemp, ARRAYSIZE(wpi->Title));
		if (GetPrivateProfileString(L"Plugin", L"Description", L"Far2->Far3 plugin wrapper", szTemp, ARRAYSIZE(szTemp), szIni))
			lstrcpyn(wpi->Desc, szTemp, ARRAYSIZE(wpi->Desc));
		if (GetPrivateProfileString(L"Plugin", L"Author", L"Maximus5", szTemp, ARRAYSIZE(szTemp), szIni))
			lstrcpyn(wpi->Author, szTemp, ARRAYSIZE(wpi->Author));
		if (GetPrivateProfileString(L"Plugin", L"RegRoot", L"Software\\Far Manager\\Plugins", szTemp, ARRAYSIZE(szTemp), szIni))
			lstrcpyn(wpi->RegRoot, szTemp, ARRAYSIZE(wpi->RegRoot));
		if (GetPrivateProfileString(L"Plugin", L"Version", L"1.0.0.0", szTemp, ARRAYSIZE(szTemp), szIni))
		{
			//TODO: Обработка версии
		}
		GUID guid;
		if (GetPrivateProfileString(L"Plugin", L"GUID", L"", szTemp, ARRAYSIZE(szTemp), szIni))
		{
			if (UuidFromStringW((RPC_WSTR)szTemp, &guid) == RPC_S_OK)
				wpi->guid_Plugin = guid;
			else
				wpi->guid_Plugin = guid_DefPlugin;
		}
		//if (GetPrivateProfileString(L"Plugin", L"MenuGUID", L"", szTemp, ARRAYSIZE(szTemp), szIni))
		//{
		//	if (UuidFromStringW(szTemp, &guid) == RPC_S_OK)
		//		wpi->guid_PluginMenu = guid;
		//	else
		//		wpi->guid_PluginMenu = ::guid_DefPluginMenu;
		//}
		//if (GetPrivateProfileString(L"Plugin", L"ConfigGUID", L"", szTemp, ARRAYSIZE(szTemp), szIni))
		//{
		//	if (UuidFromStringW(szTemp, &guid) == RPC_S_OK)
		//		wpi->guid_PluginConfigMenu = guid;
		//	else
		//		wpi->guid_PluginConfigMenu = ::guid_DefPluginConfigMenu;
		//}
		if (GetPrivateProfileString(L"Plugin", L"DialogsGUID", L"", szTemp, ARRAYSIZE(szTemp), szIni))
		{
			if (UuidFromStringW((RPC_WSTR)szTemp, &guid) == RPC_S_OK)
				wpi->guid_Dialogs = guid;
			else
				wpi->guid_Dialogs = ::guid_DefDialogs;
		}
		lbRc = TRUE;
	}
	
	if (!lbRc)
	{
		lstrcpyn(wpi->Title, szSelf[0] ? szSelf : L"Far3Wrap", ARRAYSIZE(wpi->Title));
		lstrcpy(wpi->Desc, L"Far2->Far3 plugin wrapper");
		lstrcpy(wpi->Author, L"Maximus5");
		lstrcpy(wpi->RegRoot, L"Software\\Far Manager\\Plugins");
		wpi->guid_Plugin = guid_DefPlugin;
		//wpi->guid_PluginMenu = ::guid_DefPluginMenu;
		//wpi->guid_PluginConfigMenu = ::guid_DefPluginConfigMenu;
		wpi->guid_Dialogs = ::guid_DefDialogs;
	}
}

BOOL LoadPlugin(BOOL abSilent)
{
	if (!wpi || !*wpi->PluginDll)
	{
		return FALSE;
	}
	
	if (wpi && wpi->hDll == NULL)
	{
		wchar_t szOldDir[MAX_PATH] = {0};
		GetCurrentDirectory(ARRAYSIZE(szOldDir), szOldDir);
		wchar_t* pszSlash = wcsrchr(wpi->PluginDll, L'\\');
		if (pszSlash)
		{
			*pszSlash = 0;
			SetCurrentDirectory(wpi->PluginDll);
			*pszSlash = L'\\';
		}
		wpi->hDll = LoadLibrary(wpi->PluginDll);
		DWORD dwErr = GetLastError();
		if (pszSlash)
			SetCurrentDirectory(szOldDir);
		if (wpi && wpi->hDll == NULL)
		{
			//TODO: Обработка ошибок загрузки
			wchar_t szInfo[1024] = {0};
			wsprintf(szInfo, L"Plugin loading failed!\n%s\nErrCode=0x%08X\n", wpi->PluginDll, dwErr);
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS, NULL, dwErr,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), szInfo+lstrlen(szInfo),
				ARRAYSIZE(szInfo)-lstrlen(szInfo) - 1, NULL);
			MessageBox(NULL, szInfo, L"Far3Wrapper", MB_ICONSTOP|MB_SYSTEMMODAL);
		}
		else
		{
			wpi->AnalyseW = (WrapPluginInfo::_AnalyseW)GetProcAddress(wpi->hDll, "AnalyseW");
			wpi->ClosePluginW = (WrapPluginInfo::_ClosePluginW)GetProcAddress(wpi->hDll, "ClosePluginW");
			wpi->CompareW = (WrapPluginInfo::_CompareW)GetProcAddress(wpi->hDll, "CompareW");
			wpi->ConfigureW = (WrapPluginInfo::_ConfigureW)GetProcAddress(wpi->hDll, "ConfigureW");
			wpi->DeleteFilesW = (WrapPluginInfo::_DeleteFilesW)GetProcAddress(wpi->hDll, "DeleteFilesW");
			wpi->ExitFARW = (WrapPluginInfo::_ExitFARW)GetProcAddress(wpi->hDll, "ExitFARW");
			wpi->FreeFindDataW = (WrapPluginInfo::_FreeFindDataW)GetProcAddress(wpi->hDll, "FreeFindDataW");
			wpi->FreeVirtualFindDataW = (WrapPluginInfo::_FreeVirtualFindDataW)GetProcAddress(wpi->hDll, "FreeVirtualFindDataW");
			wpi->GetFilesW = (WrapPluginInfo::_GetFilesW)GetProcAddress(wpi->hDll, "GetFilesW");
			wpi->GetFindDataW = (WrapPluginInfo::_GetFindDataW)GetProcAddress(wpi->hDll, "GetFindDataW");
			wpi->GetMinFarVersionW = (WrapPluginInfo::_GetMinFarVersionW)GetProcAddress(wpi->hDll, "GetMinFarVersionW");
			wpi->GetOpenPluginInfoW = (WrapPluginInfo::_GetOpenPluginInfoW)GetProcAddress(wpi->hDll, "GetOpenPluginInfoW");
			wpi->GetPluginInfoW = (WrapPluginInfo::_GetPluginInfoW)GetProcAddress(wpi->hDll, "GetPluginInfoW");
			wpi->GetVirtualFindDataW = (WrapPluginInfo::_GetVirtualFindDataW)GetProcAddress(wpi->hDll, "GetVirtualFindDataW");
			wpi->MakeDirectoryW = (WrapPluginInfo::_MakeDirectoryW)GetProcAddress(wpi->hDll, "MakeDirectoryW");
			wpi->OpenFilePluginW = (WrapPluginInfo::_OpenFilePluginW)GetProcAddress(wpi->hDll, "OpenFilePluginW");
			wpi->OpenPluginW = (WrapPluginInfo::_OpenPluginW)GetProcAddress(wpi->hDll, "OpenPluginW");
			wpi->ProcessDialogEventW = (WrapPluginInfo::_ProcessDialogEventW)GetProcAddress(wpi->hDll, "ProcessDialogEventW");
			wpi->ProcessEditorEventW = (WrapPluginInfo::_ProcessEditorEventW)GetProcAddress(wpi->hDll, "ProcessEditorEventW");
			wpi->ProcessEditorInputW = (WrapPluginInfo::_ProcessEditorInputW)GetProcAddress(wpi->hDll, "ProcessEditorInputW");
			wpi->ProcessEventW = (WrapPluginInfo::_ProcessEventW)GetProcAddress(wpi->hDll, "ProcessEventW");
			wpi->ProcessHostFileW = (WrapPluginInfo::_ProcessHostFileW)GetProcAddress(wpi->hDll, "ProcessHostFileW");
			wpi->ProcessKeyW = (WrapPluginInfo::_ProcessKeyW)GetProcAddress(wpi->hDll, "ProcessKeyW");
			wpi->ProcessSynchroEventW = (WrapPluginInfo::_ProcessSynchroEventW)GetProcAddress(wpi->hDll, "ProcessSynchroEventW");
			wpi->ProcessViewerEventW = (WrapPluginInfo::_ProcessViewerEventW)GetProcAddress(wpi->hDll, "ProcessViewerEventW");
			wpi->PutFilesW = (WrapPluginInfo::_PutFilesW)GetProcAddress(wpi->hDll, "PutFilesW");
			wpi->SetDirectoryW = (WrapPluginInfo::_SetDirectoryW)GetProcAddress(wpi->hDll, "SetDirectoryW");
			wpi->SetFindListW = (WrapPluginInfo::_SetFindListW)GetProcAddress(wpi->hDll, "SetFindListW");
			wpi->SetStartupInfoW = (WrapPluginInfo::_SetStartupInfoW)GetProcAddress(wpi->hDll, "SetStartupInfoW");
		}
	}
	
	return (wpi->hDll != NULL);
}


BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
                     )
{
    if (ghInstance == NULL)
    {
		ghInstance = (HMODULE)hModule;

		lbPsi2 = FALSE;
		memset(&psi3, 0, sizeof(psi3));
		memset(&psi2, 0, sizeof(psi2));
		memset(&FSF3, 0, sizeof(FSF3));
		memset(&FSF2, 0, sizeof(FSF2));

		wpi = NULL;
		LoadPluginInfo();
    }
    if (ul_reason_for_call == DLL_PROCESS_DETACH)
    {
    	if (wpi)
    	{
    		delete wpi;
    		wpi = NULL;
    	}
    }
    return TRUE;
}


void WINAPI SetStartupInfoW(PluginStartupInfo *Info)
{
	memmove(&psi3, Info, sizeof(psi3));
	memmove(&FSF3, Info->FSF, sizeof(FSF3));
	psi3.FSF = &FSF3;
	
	LoadPluginInfo();
	
	if (!LoadPlugin(TRUE))
		return;

	LOG_CMD(L"SetStartupInfoW",0,0,0);	
	// перенести в FSF2 & psi2
	
	// *** FARSTANDARDFUNCTIONS *** 
	FSF2.StructSize = sizeof(FSF2);
	FSF2.atoi = FSF3.atoi;
	FSF2.atoi64 = FSF3.atoi64;
	FSF2.itoa = FSF3.itoa;
	FSF2.itoa64 = FSF3.itoa64;
	// <C&C++>
	FSF2.sprintf = FSF3.sprintf;
	FSF2.sscanf = FSF3.sscanf;
	// </C&C++>
	FSF2.qsort = FSF3.qsort;
	FSF2.bsearch = FSF3.bsearch;
	FSF2.qsortex = FSF3.qsortex;
	// <C&C++>
	FSF2.snprintf = FSF3.snprintf;
	// </C&C++>
	FSF2.LIsLower = FSF3.LIsLower;
	FSF2.LIsUpper = FSF3.LIsUpper;
	FSF2.LIsAlpha = FSF3.LIsAlpha;
	FSF2.LIsAlphanum = FSF3.LIsAlphanum;
	FSF2.LUpper = FSF3.LUpper;
	FSF2.LLower = FSF3.LLower;
	FSF2.LUpperBuf = FSF3.LUpperBuf;
	FSF2.LLowerBuf = FSF3.LLowerBuf;
	FSF2.LStrupr = FSF3.LStrupr;
	FSF2.LStrlwr = FSF3.LStrlwr;
	FSF2.LStricmp = FSF3.LStricmp;
	FSF2.LStrnicmp = FSF3.LStrnicmp;
	FSF2.Unquote = FSF3.Unquote;
	FSF2.LTrim = FSF3.LTrim;
	FSF2.RTrim = FSF3.RTrim;
	FSF2.Trim = FSF3.Trim;
	FSF2.TruncStr = FSF3.TruncStr;
	FSF2.TruncPathStr = FSF3.TruncPathStr;
	FSF2.QuoteSpaceOnly = FSF3.QuoteSpaceOnly;
	FSF2.PointToName = FSF3.PointToName;
	FSF2.GetPathRoot = WrapPluginInfo::FarStdGetPathRoot;
	FSF2.AddEndSlash = FSF3.AddEndSlash;
	FSF2.CopyToClipboard = FSF3.CopyToClipboard;
	FSF2.PasteFromClipboard = FSF3.PasteFromClipboard;
	FSF2.FarKeyToName = FSF3.FarKeyToName;
	FSF2.FarNameToKey = FSF3.FarNameToKey;
	FSF2.FarInputRecordToKey = FSF3.FarInputRecordToKey;
	FSF2.XLat = WrapPluginInfo::FarStdXlat;
	FSF2.GetFileOwner = WrapPluginInfo::FarStdGetFileOwner;
	FSF2.GetNumberOfLinks = FSF3.GetNumberOfLinks;
	FSF2.FarRecursiveSearch = WrapPluginInfo::FarStdRecursiveSearch;
	FSF2.MkTemp = WrapPluginInfo::FarStdMkTemp;
	FSF2.DeleteBuffer = FSF3.DeleteBuffer;
	FSF2.ProcessName = WrapPluginInfo::FarStdProcessName;
	FSF2.MkLink = WrapPluginInfo::FarStdMkLink;
	FSF2.ConvertPath = WrapPluginInfo::FarConvertPath;
	FSF2.GetReparsePointInfo = WrapPluginInfo::FarGetReparsePointInfo;
	FSF2.GetCurrentDirectory = WrapPluginInfo::FarGetCurrentDirectory;
	
	// *** PluginStartupInfo ***
	psi2.StructSize = sizeof(psi2);
	psi2.ModuleName = wpi->PluginDll;
	psi2.ModuleNumber = (INT_PTR)wpi->hDll;
	psi2.RootKey = wpi->RegRoot;
	psi2.Menu = WrapPluginInfo::FarApiMenu;
	psi2.Message = WrapPluginInfo::FarApiMessage;
	psi2.GetMsg = WrapPluginInfo::FarApiGetMsg;
	psi2.Control = WrapPluginInfo::FarApiControl;
	psi2.SaveScreen = WrapPluginInfo::FarApiSaveScreen;
	psi2.RestoreScreen = WrapPluginInfo::FarApiRestoreScreen;
	psi2.GetDirList = WrapPluginInfo::FarApiGetDirList;
	psi2.GetPluginDirList = WrapPluginInfo::FarApiGetPluginDirList;
	psi2.FreeDirList = WrapPluginInfo::FarApiFreeDirList;
	psi2.FreePluginDirList = WrapPluginInfo::FarApiFreePluginDirList;
	psi2.Viewer = WrapPluginInfo::FarApiViewer;
	psi2.Editor = WrapPluginInfo::FarApiEditor;
	psi2.CmpName = WrapPluginInfo::FarApiCmpName;
	psi2.Text = WrapPluginInfo::FarApiText;
	psi2.EditorControl = WrapPluginInfo::FarApiEditorControl;

	psi2.FSF = &FSF2;

	psi2.ShowHelp = WrapPluginInfo::FarApiShowHelp;
	psi2.AdvControl = WrapPluginInfo::FarApiAdvControl;
	psi2.InputBox = WrapPluginInfo::FarApiInputBox;
	psi2.DialogInit = WrapPluginInfo::FarApiDialogInit;
	psi2.DialogRun = WrapPluginInfo::FarApiDialogRun;
	psi2.DialogFree = WrapPluginInfo::FarApiDialogFree;

	psi2.SendDlgMessage = WrapPluginInfo::FarApiSendDlgMessage;
	psi2.DefDlgProc = WrapPluginInfo::FarApiDefDlgProc;
	//DWORD_PTR              Reserved;
	psi2.ViewerControl = WrapPluginInfo::FarApiViewerControl;
	psi2.PluginsControl = WrapPluginInfo::FarApiPluginsControl;
	psi2.FileFilterControl = WrapPluginInfo::FarApiFileFilterControl;
	psi2.RegExpControl = WrapPluginInfo::FarApiRegExpControl;

	lbPsi2 = TRUE;
	
	if (wpi && wpi->SetStartupInfoW && lbPsi2)
	{
		wpi->SetStartupInfoW(&psi2);
	}
}

void WINAPI GetGlobalInfoW(GlobalInfo *Info)
{
	//LOG_CMD(L"GetGlobalInfoW",0,0,0);
	LoadPluginInfo();
	
	Info->StructSize = sizeof(GlobalInfo);
	Info->MinFarVersion = FARMANAGERVERSION;

	Info->Version = wpi->Version;
	Info->Guid = wpi->guid_Plugin;
	Info->Title = wpi->Title;
	Info->Description = wpi->Desc;
	Info->Author = wpi->Author;
}



void WINAPI GetPluginInfoW(PluginInfo *Info)
{
    memset(Info, 0, sizeof(PluginInfo));
    
	Info->StructSize = sizeof(*Info);

    if (wpi && wpi->GetPluginInfoW)
    {
    	LOG_CMD(L"GetPluginInfoW",0,0,0);
		wpi->Info.StructSize = sizeof(wpi->Info);
    	wpi->GetPluginInfoW(&wpi->Info);
    	
    	if (wpi && wpi->Info.Flags & Far2::PF_PRELOAD)
    		Info->Flags |= PF_PRELOAD;
    	if (wpi && wpi->Info.Flags & Far2::PF_DISABLEPANELS)
    		Info->Flags |= PF_DISABLEPANELS;
    	if (wpi && wpi->Info.Flags & Far2::PF_EDITOR)
    		Info->Flags |= PF_EDITOR;
    	if (wpi && wpi->Info.Flags & Far2::PF_VIEWER)
    		Info->Flags |= PF_VIEWER;
    	if (wpi && wpi->Info.Flags & Far2::PF_FULLCMDLINE)
    		Info->Flags |= PF_FULLCMDLINE;
    	if (wpi && wpi->Info.Flags & Far2::PF_DIALOG)
    		Info->Flags |= PF_DIALOG;

		//if (GetPrivateProfileString(L"Plugin", L"MenuGUID", L"", szTemp, ARRAYSIZE(szTemp), szIni))
		//{
		//	if (
		//		wpi->guid_PluginMenu = guid;
		//	else
		//		wpi->guid_PluginMenu = ::guid_DefPluginMenu;
		//}

		wchar_t szValName[64], szGUID[128]; GUID guid;
		struct { LPCWSTR Fmt; int Count; const wchar_t * const * Strings;
				 PluginMenuItem *Menu; int* GuidCount; GUID** Guids; }
			Menus[] = 
		{
			{L"PluginMenuGUID%i", wpi->Info.PluginMenuStringsNumber, wpi->Info.PluginMenuStrings, 
				&Info->PluginMenu, &wpi->nPluginMenu, &wpi->guids_PluginMenu},
			{L"DiskMenuGUID%i", wpi->Info.DiskMenuStringsNumber, wpi->Info.DiskMenuStrings, 
				&Info->DiskMenu, &wpi->nPluginDisks, &wpi->guids_PluginDisks},
			{L"PluginConfigGUID%i", wpi->Info.PluginConfigStringsNumber, wpi->Info.PluginConfigStrings, 
				&Info->PluginConfig, &wpi->nPluginConfig, &wpi->guids_PluginConfig}
		};
		for (int k = 0; k < ARRAYSIZE(Menus); k++)
		{
			if (Menus[k].Count < 1)
				continue;
			
    		if (*Menus[k].GuidCount < Menus[k].Count || !*Menus[k].Guids)
    		{
    			if (*Menus[k].Guids)
    				free(*Menus[k].Guids);
    			*Menus[k].GuidCount = 0;
    			*Menus[k].Guids = (GUID*)calloc(sizeof(GUID),Menus[k].Count);
    			for (int i = 1; i <= Menus[k].Count; i++)
    			{
    				wsprintf(szValName, Menus[k].Fmt, i);
    				if (!GetPrivateProfileString(L"Plugin", szValName, L"", szGUID, ARRAYSIZE(szGUID), wpi->IniFile))
    					break;
    				if (UuidFromStringW((RPC_WSTR)szGUID, &guid) != RPC_S_OK)
    					break;
    				// OK
    				*(Menus[k].Guids[i-1]) = guid;
    				(*Menus[k].GuidCount)++;
    			}
    			Menus[k].Count = *Menus[k].GuidCount; // могло не на всех гуидов хватить
    		}
    		if (Menus[k].Count > 0)
    		{
				Menus[k].Menu->Guids = *Menus[k].Guids;
				Menus[k].Menu->Strings = Menus[k].Strings;
				Menus[k].Menu->Count = Menus[k].Count;
			}
    	}
		//const wchar_t * const *DiskMenuStrings;
		//int *DiskMenuNumbers;
		//int DiskMenuStringsNumber;
		// -- >struct PluginMenuItem DiskMenu;
		//const wchar_t * const *PluginMenuStrings;
		//int PluginMenuStringsNumber;
		// -->struct PluginMenuItem PluginMenu;
		//const wchar_t * const *PluginConfigStrings;
		//int PluginConfigStringsNumber;
		// -->struct PluginMenuItem PluginConfig;
		
		Info->CommandPrefix = wpi->Info.CommandPrefix;
    }
}

HANDLE WINAPI OpenW(const OpenInfo *Info)
{
	LOG_CMD(L"OpenW(0x%08X)", (DWORD)Info->OpenFrom,0,0);
	HANDLE h = INVALID_HANDLE_VALUE;

	if ((Info->OpenFrom & OPEN_FROM_MASK) == OPEN_ANALYSE)
	{
		if (wpi && wpi->AnalyseW)
		{
			// В принципе, в Far2 была такая функция, так что если плаг ее экспортит - зовем
			h = wpi->OpenPluginW(OpenFrom_3_2(Info->OpenFrom), Info->Data);
			goto trap;
		}
		if (!wpi->pAnalyze || !wpi->OpenFilePluginW)
			goto trap;
		h = wpi->OpenFilePluginW(wpi->pAnalyze->FileName,
				(const unsigned char*)wpi->pAnalyze->Buffer,
				wpi->pAnalyze->BufferSize,
				OpMode_3_2(wpi->pAnalyze->OpMode));
		goto trap;
	}
	else if (wpi && wpi->pAnalyze)
	{
		free(wpi->pAnalyze);
		wpi->pAnalyze = NULL;
	}
	
	if ((Info->OpenFrom & OPEN_FROMMACRO_MASK))
	{
		if (wpi && wpi->Info.Reserved 
			&& (((Info->OpenFrom & OPEN_FROMMACRO_MASK) == OPEN_FROMMACRO)
				|| ((Info->OpenFrom & OPEN_FROMMACRO_MASK) == OPEN_FROMMACROSTRING))
		    && wpi->OpenPluginW)
		{
			h = wpi->OpenPluginW(Far2::OPEN_FROMMACRO, Info->Data);
		}
		goto trap;
	}

	if (wpi && wpi->OpenPluginW)
	{
		switch (Info->OpenFrom & OPEN_FROM_MASK)
		{
		case OPEN_LEFTDISKMENU:
		case OPEN_RIGHTDISKMENU:
			h = wpi->OpenPluginW(Far2::OPEN_DISKMENU, Info->Data); break;
		case OPEN_PLUGINSMENU:
			h = wpi->OpenPluginW(Far2::OPEN_PLUGINSMENU, Info->Data); break;
		case OPEN_FINDLIST:
			h = wpi->OpenPluginW(Far2::OPEN_FINDLIST, Info->Data); break;
		case OPEN_SHORTCUT:
			h = wpi->OpenPluginW(Far2::OPEN_SHORTCUT, Info->Data); break;
		case OPEN_COMMANDLINE:
			h = wpi->OpenPluginW(Far2::OPEN_COMMANDLINE, Info->Data); break;
		case OPEN_EDITOR:
			h = wpi->OpenPluginW(Far2::OPEN_EDITOR, Info->Data); break;
		case OPEN_VIEWER:
			h = wpi->OpenPluginW(Far2::OPEN_VIEWER, Info->Data); break;
		case OPEN_FILEPANEL:
			h = wpi->OpenPluginW(Far2::OPEN_FILEPANEL, Info->Data); break;
		case OPEN_DIALOG:
			h = wpi->OpenPluginW(Far2::OPEN_DIALOG, Info->Data); break;
		}
	}
trap:
	return h;
}

int    WINAPI AnalyseW(const AnalyseInfo *Info)
{
	LOG_CMD(L"AnalyseW",0,0,0);
	if (!wpi)
		return FALSE;
	if (wpi && wpi->AnalyseW)
		return wpi->AnalyseW(Info);
	if (wpi && wpi->pAnalyze)
	{
		free(wpi->pAnalyze);
		wpi->pAnalyze = NULL;
	}
	if (!wpi->OpenFilePluginW)
		return FALSE;
	size_t nNewSize = sizeof(*wpi->pAnalyze) + Info->BufferSize
			+ ((Info->FileName ? lstrlen(Info->FileName) : 0)+1)*sizeof(wchar_t);
	wpi->pAnalyze = (AnalyseInfo*)malloc(nNewSize);
	if (!wpi->pAnalyze)
		return FALSE;
	LPBYTE ptr = (LPBYTE)wpi->pAnalyze;
	memmove(ptr, Info, sizeof(wpi->pAnalyze));
	ptr += sizeof(wpi->pAnalyze);
	if (Info->BufferSize && Info->Buffer)
	{
		memmove(ptr, Info->Buffer, Info->BufferSize);
		wpi->pAnalyze->Buffer = ptr;
		ptr += Info->BufferSize;
	}
	else
	{
		wpi->pAnalyze->Buffer = NULL;
	}
	if (Info->FileName)
	{
		lstrcpy((LPWSTR)ptr, Info->FileName);
		wpi->pAnalyze->FileName = (LPCWSTR)ptr;
	}
	else
		wpi->pAnalyze->FileName = NULL;
	
	HANDLE h = wpi->OpenFilePluginW(wpi->pAnalyze->FileName,
		(const unsigned char*)wpi->pAnalyze->Buffer,
		wpi->pAnalyze->BufferSize,
		OpMode_3_2(Info->OpMode));
	if (h && h != INVALID_HANDLE_VALUE && ((INT_PTR)h) != -2)
	{
		if (wpi && wpi->AnalyzeMode == 1)
		{
			//TODO: Не закрывать. только вопрос - когда его можно будет закрыть, если OpenW НЕ будет вызван?
		}
	
		//TODO: Хорошо бы оптимизнуть, и не открывать файл два раза подряд, но непонятно
		//TODO: в какой момент можно закрыть Far2 плагин, если до OpenW дело не дошло...
		if (wpi && wpi->ClosePluginW)
			wpi->ClosePluginW(h);
		return TRUE;
	}
	return 0;
}
void   WINAPI ClosePanelW(HANDLE hPanel)
{
	LOG_CMD(L"ClosePanelW",0,0,0);
	if (wpi && wpi->ClosePluginW)
		wpi->ClosePluginW(hPanel);
}
int    WINAPI CompareW(const CompareInfo *Info)
{
	LOG_CMD(L"CompareW",0,0,0);
	int iRc = -2;
	if (wpi && wpi->CompareW)
	{
		Far2::PluginPanelItem Item1 = {{0}};
		Far2::PluginPanelItem Item2 = {{0}};
		PluginPanelItem_3_2(Info->Item1, &Item1);
		PluginPanelItem_3_2(Info->Item2, &Item2);
		iRc = wpi->CompareW(Info->hPanel, &Item1, &Item2, SortMode_3_2(Info->Mode));
	}
	return iRc;
}
int    WINAPI ConfigureW(const GUID* Guid)
{
	LOG_CMD(L"ConfigureW",0,0,0);
	int iRc = 0;
	if (wpi && wpi->ConfigureW)
	{
		if (wpi && wpi->nPluginConfig > 0 && wpi->guids_PluginConfig)
		{
			for (int i = 0; i < wpi->nPluginConfig; i++)
			{
				if (memcmp(Guid, &GUID_NULL, sizeof(GUID)) == 0 ||
					memcmp(Guid, wpi->guids_PluginConfig+i, sizeof(GUID)) == 0)
				{
					iRc = wpi->ConfigureW(i);
					break;
				}
			}
		}
	}
	return iRc;
}
int    WINAPI DeleteFilesW(const DeleteFilesInfo *Info)
{
	LOG_CMD(L"DeleteFilesW",0,0,0);
	int iRc = 0;
	if (wpi && wpi->DeleteFilesW)
	{
		Far2::PluginPanelItem *p2 = PluginPanelItems_3_2(Info->PanelItem, Info->ItemsNumber);
		if (p2)
		{
			iRc = wpi->DeleteFilesW(Info->hPanel, p2, Info->ItemsNumber, OpMode_3_2(Info->OpMode));
			free(p2);
		}
	}
	return iRc;
}
void   WINAPI ExitFARW(void)
{
	LOG_CMD(L"ExitFARW",0,0,0);
	if (wpi)
	{
		if (wpi->ExitFARW)
			wpi->ExitFARW();
		wpi->UnloadPlugin();
		delete wpi;
		wpi = NULL;
	}
}
void   WINAPI FreeVirtualFindDataW(const FreeFindDataInfo *Info)
{
	LOG_CMD(L"FreeVirtualFindDataW",0,0,0);
	//TODO:
}
int    WINAPI GetFilesW(GetFilesInfo *Info)
{
	LOG_CMD(L"GetFilesW",0,0,0);
	int iRc = 0;
	if (wpi && wpi->GetFilesW)
	{
		Far2::PluginPanelItem *p2 = PluginPanelItems_3_2(Info->PanelItem, Info->ItemsNumber);
		if (p2)
		{
			iRc = wpi->GetFilesW(Info->hPanel, p2, Info->ItemsNumber, Info->Move, &Info->DestPath, OpMode_3_2(Info->OpMode));
			free(p2);
		}
	}
	return iRc;
}
int    WINAPI GetFindDataW(GetFindDataInfo *Info)
{
	LOG_CMD(L"GetFindDataW",0,0,0);
	//GetFindDataW(HANDLE hPlugin,Far2::PluginPanelItem **pPanelItem,int *pItemsNumber,int OpMode);
	if (!wpi->GetFindDataW)
		return 0;
	Far2::PluginPanelItem *pPanelItem = NULL;
	int ItemsNumber = 0;
	int iRc = wpi->GetFindDataW(Info->hPanel, &pPanelItem, &ItemsNumber, OpMode_3_2(Info->OpMode));
	if (iRc && ItemsNumber > 0)
	{
		Info->PanelItem = PluginPanelItems_2_3(pPanelItem, ItemsNumber);
		Info->ItemsNumber = ItemsNumber;
		wpi->MapPanelItems[Info->PanelItem] = pPanelItem;
		//PluginPanelItem* p3 = Info->PanelItem;
		//Far2::PluginPanelItem* p2 = pPanelItem;
		//for (int i = 0; i < ItemsNumber; i++, p2++, p3++)
		//{
		//	p3->FileAttributes = p2->FindData.dwFileAttributes;
		//	p3->CreationTime = p2->FindData.ftCreationTime;
		//	p3->LastAccessTime = p2->FindData.ftLastAccessTime;
		//	p3->LastWriteTime = p2->FindData.ftLastWriteTime;
		//	p3->ChangeTime = p2->FindData.ftLastWriteTime;
		//	p3->FileSize = p2->FindData.nFileSize;
		//	p3->PackSize = p2->FindData.nPackSize;
		//	p3->FileName = p2->FindData.lpwszFileName;
		//	p3->AlternateFileName = p2->FindData.lpwszAlternateFileName;
		//	p3->Flags = PluginPanelItemFlags_3_2(p2->Flags);
		//	p3->NumberOfLinks = p2->NumberOfLinks;
		//	p3->Description = p2->Description;
		//	p3->Owner = p2->Owner;
		//	p3->CustomColumnData = p2->CustomColumnData;
		//	p3->CustomColumnNumber = p2->CustomColumnNumber;
		//	p3->UserData = p2->UserData;
		//	p3->CRC32 = p2->CRC32;
		//}
	}
	return iRc;
}
void   WINAPI FreeFindDataW(const FreeFindDataInfo *Info)
{
	LOG_CMD(L"FreeFindDataW",0,0,0);
	if (Info->PanelItem)
	{
		if (wpi && wpi->FreeFindDataW)
		{
			Far2::PluginPanelItem *pPanelItem = wpi->MapPanelItems[Info->PanelItem];
			if (pPanelItem)
				wpi->FreeFindDataW(Info->hPanel, pPanelItem, Info->ItemsNumber);
		}
		free(Info->PanelItem);
		
		wpi->MapPanelItems.erase(Info->PanelItem);
		//std::map<PluginPanelItem*,Far2::PluginPanelItem*>::iterator iter;
		//for (iter = wpi->MapPanelItems.begin( ); iter!=wpi->MapPanelItems.end( ); iter++)
		//{
		//	if (iter->first == Info->PanelItem)
		//	{
		//		wpi->MapPanelItems.erase(iter);
		//		break;
		//	}
		//}
	}
}
void   WINAPI GetOpenPanelInfoW(OpenPanelInfo *Info)
{
	LOG_CMD(L"GetOpenPluginInfoW",0,0,0);
	if (wpi && wpi->GetOpenPluginInfoW)
	{
		Far2::OpenPluginInfo ofi = {sizeof(Far2::OpenPluginInfo)};
		wpi->GetOpenPluginInfoW(Info->hPanel, &ofi);
		
		//memset(Info, 0, sizeof(*Info));
		//Info->StructSize = sizeof(*Info);
		//HANDLE hPanel;
		Info->Flags = OpenPanelInfoFlags_2_3(ofi.Flags);
		Info->HostFile = ofi.HostFile;
		Info->CurDir = ofi.CurDir;
		Info->Format = ofi.Format;
		Info->PanelTitle = ofi.PanelTitle;
		Info->InfoLines = InfoLines_2_3(ofi.InfoLines, ofi.InfoLinesNumber);
		Info->InfoLinesNumber = wpi->InfoLinesNumber;
		Info->DescrFiles = ofi.DescrFiles;
		Info->DescrFilesNumber = ofi.DescrFilesNumber;
		Info->PanelModesArray = PanelModes_2_3(ofi.PanelModesArray, ofi.PanelModesNumber);
		Info->PanelModesNumber = wpi->PanelModesNumber;
		Info->StartPanelMode = ofi.StartPanelMode;
		Info->StartSortMode = SortMode_2_3(ofi.StartSortMode);
		Info->StartSortOrder = ofi.StartSortOrder;
		Info->KeyBar = wpi->KeyBarTitles_2_3(ofi.KeyBar);
		Info->ShortcutData = ofi.ShortcutData;
		Info->FreeSize = 0;
	}
}
int    WINAPI GetVirtualFindDataW(GetVirtualFindDataInfo *Info)
{
	LOG_CMD(L"GetVirtualFindDataW",0,0,0);
	//TODO:
	return 0;
}
int    WINAPI MakeDirectoryW(MakeDirectoryInfo *Info)
{
	LOG_CMD(L"MakeDirectoryW",0,0,0);
	int iRc = 0;
	if (wpi && wpi->MakeDirectoryW)
		iRc = wpi->MakeDirectoryW(Info->hPanel, &Info->Name, OpMode_3_2(Info->OpMode));
	return iRc;
}
int    WINAPI ProcessDialogEventW(int Event,void *Param)
{
	LOG_CMD(L"ProcessDialogEventW",0,0,0);
	//TODO:
	return 0;
}
int    WINAPI ProcessEditorEventW(int Event,void *Param)
{
	LOG_CMD0(L"ProcessEditorEventW(%i)",Event,0,0);
	int iRc = 0;
	if (wpi && wpi->ProcessEditorEventW)
		iRc = wpi->ProcessEditorEventW(Event, Param);
	return iRc;
}
int    WINAPI ProcessEditorInputW(const INPUT_RECORD *Rec)
{
	LOG_CMD(L"ProcessEditorInputW",0,0,0);
	int iRc = 0;
	if (wpi && wpi->ProcessEditorInputW)
		iRc = wpi->ProcessEditorInputW(Rec);
	return iRc;
}
int    WINAPI ProcessEventW(HANDLE hPanel,int Event,void *Param)
{
	LOG_CMD0(L"ProcessEventW(%i)",Event,0,0);
	int iRc = 0;
	if (wpi && wpi->ProcessEventW)
		iRc = wpi->ProcessEventW(hPanel, Event, Param);
	return iRc;
}
int    WINAPI ProcessHostFileW(const ProcessHostFileInfo *Info)
{
	LOG_CMD(L"ProcessHostFileW",0,0,0);
	int iRc = 0;
	if (wpi && wpi->ProcessHostFileW)
	{
		Far2::PluginPanelItem *p2 = PluginPanelItems_3_2(Info->PanelItem, Info->ItemsNumber);
		iRc = wpi->ProcessHostFileW(Info->hPanel, p2, Info->ItemsNumber, OpMode_3_2(Info->OpMode));
		if (p2)
			free(p2);
	}
	return iRc;
}
int    WINAPI ProcessKeyW(HANDLE hPanel,const INPUT_RECORD *Rec)
{
	LOG_CMD(L"ProcessKeyW(%s,0x%X,0x%X)",(Rec->EventType==KEY_EVENT?L"Key":L"???"),Rec->Event.KeyEvent.wVirtualKeyCode,Rec->Event.KeyEvent.dwControlKeyState);
	int iRc = 0;
	if (wpi && wpi->ProcessKeyW)
	{
		int Key2 = 0;
		if (Rec->EventType == KEY_EVENT)
		{
			if (!Rec->Event.KeyEvent.uChar.UnicodeChar
				&& !Rec->Event.KeyEvent.dwControlKeyState
				&& !Rec->Event.KeyEvent.wVirtualScanCode
				&& (Rec->Event.KeyEvent.wVirtualKeyCode <= 8))
			{
				Key2 = INTERNAL_KEY_BASE_2+Rec->Event.KeyEvent.wVirtualKeyCode;
			}
			else if (Rec->Event.KeyEvent.uChar.UnicodeChar == 0
				&& Rec->Event.KeyEvent.wVirtualScanCode && Rec->Event.KeyEvent.wVirtualKeyCode
					/*&& (Rec->Event.KeyEvent.wVirtualKeyCode == VK_SHIFT
					|| Rec->Event.KeyEvent.wVirtualKeyCode == VK_CONTROL || Rec->Event.KeyEvent.wVirtualKeyCode == VK_RCONTROL
					|| Rec->Event.KeyEvent.wVirtualKeyCode == VK_MENU || Rec->Event.KeyEvent.wVirtualKeyCode == VK_RMENU)*/)
			{
				switch (Rec->Event.KeyEvent.wVirtualKeyCode)
				{
				case VK_SHIFT: Key2 = KEY_SHIFT; break;
				case VK_CONTROL: Key2 = KEY_CTRL; break;
				case VK_MENU: Key2 = KEY_ALT; break;
				case VK_RCONTROL: Key2 = KEY_RCTRL; break;
				case VK_RMENU: Key2 = KEY_RALT; break;
				default: Key2 = Rec->Event.KeyEvent.wVirtualKeyCode;
				}
				if (Rec->Event.KeyEvent.dwControlKeyState & LEFT_CTRL_PRESSED)
					Key2 |= KEY_CTRL;
				if (Rec->Event.KeyEvent.dwControlKeyState & RIGHT_CTRL_PRESSED)
					Key2 |= KEY_RCTRL;
				if (Rec->Event.KeyEvent.dwControlKeyState & LEFT_ALT_PRESSED)
					Key2 |= KEY_ALT;
				if (Rec->Event.KeyEvent.dwControlKeyState & RIGHT_ALT_PRESSED)
					Key2 |= KEY_RALT;
				if (Rec->Event.KeyEvent.dwControlKeyState & SHIFT_PRESSED)
					Key2 |= KEY_SHIFT;
			}
			else
			{
				// У фара (2018) срывает крышу: http://bugs.farmanager.com/view.php?id=1760
				_ASSERTE(FALSE);
				Key2 = FSF3.FarInputRecordToKey(Rec);
			}
		}
		else
		{
			// У фара (2018) срывает крышу: http://bugs.farmanager.com/view.php?id=1760
			_ASSERTE(FALSE);
			Key2 = FSF3.FarInputRecordToKey(Rec);
		}
		//TODO: Ctrl/Shift/Alt?
		DWORD FShift = Key2 & 0x7F000000; // старший бит используется в других целях!
		DWORD ControlState =
			(FShift & KEY_SHIFT ? Far2::PKF_SHIFT : 0)|
			(FShift & KEY_ALT ? Far2::PKF_ALT : 0)|
			(FShift & KEY_CTRL ? Far2::PKF_CONTROL : 0);

		iRc = wpi->ProcessKeyW(hPanel, Key2 & 0x0003FFFF, ControlState);
	}
	return iRc;
}
int    WINAPI ProcessSynchroEventW(int Event,void *Param)
{
	LOG_CMD(L"ProcessSynchroEventW",0,0,0);
	int iRc = 0;
	if (wpi && wpi->ProcessSynchroEventW)
		iRc = wpi->ProcessSynchroEventW(Event, Param);
	return iRc;
}
int    WINAPI ProcessViewerEventW(int Event,void *Param)
{
	LOG_CMD(L"ProcessViewerEventW",0,0,0);
	int iRc = 0;
	if (wpi && wpi->ProcessViewerEventW)
		iRc = wpi->ProcessViewerEventW(Event, Param);
	return iRc;
}
int    WINAPI PutFilesW(const PutFilesInfo *Info)
{
	LOG_CMD(L"PutFilesW",0,0,0);
	int iRc = 0;
	if (wpi && wpi->PutFilesW)
	{
		Far2::PluginPanelItem *p2 = PluginPanelItems_3_2(Info->PanelItem, Info->ItemsNumber);
		if (p2)
		{
			if (!wpi->OldPutFilesParams)
				iRc = wpi->PutFilesW(Info->hPanel, p2, Info->ItemsNumber, Info->Move, Info->SrcPath, OpMode_3_2(Info->OpMode));
			else
			{
				wchar_t szOldPath[MAX_PATH]; szOldPath[0] = 0;
				if (Info->SrcPath)
				{
					GetCurrentDirectory(ARRAYSIZE(szOldPath), szOldPath);
					// Тут будет облом на "длинных" или "неправильных" путях
					SetCurrentDirectory(Info->SrcPath);
				}
				iRc = ((WrapPluginInfo::_PutFilesOldW)wpi->PutFilesW)(Info->hPanel, p2, Info->ItemsNumber, Info->Move, OpMode_3_2(Info->OpMode));
				if (szOldPath[0])
					SetCurrentDirectory(szOldPath);
			}
			free(p2);
		}
	}
	return iRc;
}
int    WINAPI SetDirectoryW(const SetDirectoryInfo *Info)
{
	int iRc = 0;
	LOG_CMD(L"SetDirectoryW",0,0,0);
	if (wpi && wpi->SetDirectoryW)
		iRc = wpi->SetDirectoryW(Info->hPanel, Info->Dir, OpMode_3_2(Info->OpMode));
	return iRc;
}
int    WINAPI SetFindListW(const SetFindListInfo *Info)
{
	LOG_CMD(L"SetFindListW",0,0,0);
	//TODO:
	return 0;
}

//int EditCtrl(int Cmd, void* Parm)
//{
//	int iRc;
//	iRc = psi.EditorControl(-1, (EDITOR_CONTROL_COMMANDS)Cmd, 0, (INT_PTR)Parm);
//	return iRc;
//}




Far2Dialog::Far2Dialog(int X1, int Y1, int X2, int Y2,
    const wchar_t *HelpTopic, Far2::FarDialogItem *Items, UINT ItemsNumber,
    DWORD Flags, Far2::FARWINDOWPROC DlgProc, LONG_PTR Param,
    GUID PluginGuid, GUID DefGuid)
{
	hDlg3 = NULL; m_Items3 = NULL; mp_ListInit3 = NULL;
	m_PluginGuid = PluginGuid;
	
    m_X1 = X1; m_Y1 = Y1; m_X2 = X2; m_Y2 = Y2;
    m_HelpTopic = HelpTopic ? _wcsdup(HelpTopic) : NULL;
    
    m_ItemsNumber = ItemsNumber;
    m_Items2 = NULL;
    if (ItemsNumber > 0 && Items)
    {
    	m_Items2 = (Far2::FarDialogItem*)calloc(ItemsNumber, sizeof(*m_Items2));
    	memmove(m_Items2, Items, ItemsNumber*sizeof(*m_Items2));
    }
    m_Flags = Flags;
    m_DlgProc = DlgProc;
    m_Param = Param;
    // GUID
    m_GuidChecked = FALSE;
    memset(&m_Guid, 0, sizeof(m_Guid));
    m_DefGuid = DefGuid;
};

Far2Dialog::~Far2Dialog()
{
	// Far3 call
	FreeDlg();
	// Release memory
	if (m_HelpTopic)
	{
		free(m_HelpTopic);
		m_HelpTopic = NULL;
	}
	if (m_Items2)
	{
		free(m_Items2);
		m_Items2 = NULL;
	}
	if (mp_ListInit3)
	{
		for (UINT i = 0; i < m_ItemsNumber; i++)
		{
			if (mp_ListInit3[i].Items)
				free(mp_ListInit3[i].Items);
		}
		free(mp_ListInit3);
	}
	if (m_Items3)
	{
		free(m_Items3);
		m_Items3 = NULL;
	}
};

void Far2Dialog::FreeDlg()
{
	wpi->MapDlg_2_3.erase(this);
	if (hDlg3 != NULL)
	{
		psi3.DialogFree(hDlg3);
		wpi->MapDlg_3_2.erase(hDlg3);
		hDlg3 = NULL;
	}
}

INT_PTR Far2Dialog::Far3DlgProc(HANDLE hDlg, int Msg, int Param1, void* Param2)
{
	Far2Dialog* p = wpi->MapDlg_3_2[hDlg];
	INT_PTR lRc = 0;
	if (p && p->m_DlgProc)
	{
		void* OrgParam2 = Param2;
		Far2::FarMessagesProc Msg2 = FarMessage_3_2(Msg, Param1, Param2);
		lRc = p->m_DlgProc((HANDLE)p, Msg2, Param1, (LONG_PTR)Param2);
		if (Param2 && Param2 != OrgParam2)
			FarMessageParam_2_3(Msg, Param1, Param2, OrgParam2, lRc);
	}
	else
	{
		lRc = psi3.DefDlgProc(hDlg, Msg, Param1, (void*)Param2);
	}
	return lRc;
}

int Far2Dialog::RunDlg()
{
	int iRc = -1;
	
	if (!m_GuidChecked)
	{
		/*
		-- пока не получается. Например, UCharMap зовет DM_GETDLGDATA, а диалога-то еще нет...
		if (m_DlgProc)
		{
			Far2::DialogInfo info = {sizeof(Far2::DialogInfo)};
			if (m_DlgProc((HANDLE)this, Far2::DM_GETDIALOGINFO, 0, (LONG_PTR)&info)
				&& memcmp(&info.Id, &m_Guid, sizeof(m_Guid)))
			{
				m_Guid = info.Id;
				m_GuidChecked = TRUE;
			}
		}
		*/
		if (!m_GuidChecked)
		{
			m_Guid = m_DefGuid;
			m_GuidChecked = TRUE;
		}
	}
	
	if (!m_Items3 && m_Items2 && m_ItemsNumber > 0)
	{
		m_Items3 = (FarDialogItem*)calloc(m_ItemsNumber, sizeof(*m_Items3));
		if (!mp_ListInit3)
			mp_ListInit3 = (FarList*)calloc(m_ItemsNumber,sizeof(FarList));
		Far2::FarDialogItem *p2 = m_Items2;
		FarDialogItem *p3 = m_Items3;
		for (UINT i = 0; i < m_ItemsNumber; i++, p2++, p3++)
		{
			FarDialogItem_2_3(p2, p3, mp_ListInit3+i);
			//p3->Type = DialogItemTypes_2_3(p2->Type);
			//p3->X1 = p2->X1;
			//p3->Y1 = p2->Y1;
			//p3->X2 = p2->X2;
			//p3->Y2 = p2->Y2;

			//p3->Flags = FarDialogItemFlags_2_3(p2->Flags);
			//if (p2->DefaultButton)
			//	p3->Flags |= DIF_DEFAULTBUTTON;
			//if (p2->Focus)
			//	p3->Flags |= DIF_FOCUS;

			//p3->Data = p2->PtrData;
			//p3->MaxLength = p2->MaxLen;

			//if (p3->Type == DI_EDIT)
			//{
			//	p3->History = p2->Param.History;
			//}
			//else if (p3->Type == DI_FIXEDIT)
			//{
			//	p3->Mask = p2->Param.Mask;
			//}
			//else if (p3->Type == DI_COMBOBOX || p3->Type == DI_LISTBOX)
			//{
			//	if (p2->Param.ListItems != NULL)
			//	{
			//		int nItems = p2->Param.ListItems->ItemsNumber;
			//		mp_ListInit3[i].ItemsNumber = nItems;
			//		if (p2->Param.ListItems > 0)
			//		{
			//			mp_ListInit3[i].Items = (FarListItem*)calloc(nItems,sizeof(FarListItem));
			//			const Far2::FarListItem* pi2 = p2->Param.ListItems->Items;
			//			FarListItem* pi3 = mp_ListInit3[i].Items;
			//			for (int j = 0; j < nItems; j++, pi2++, pi3++)
			//			{
			//				FarListItem_2_3(pi2, pi3);
			//			}
			//		}
			//		p3->ListItems = (mp_ListInit3 + i);
			//	}
			//}
			//else if (p3->Type == DI_USERCONTROL)
			//{
			//	p3->VBuf = p2->Param.VBuf;
			//}
			//else
			//{
			//	p3->Reserved = p2->Param.Reserved;
			//}
		}
	}

	if (hDlg3 == NULL)
	{
		wpi->LastFar2Dlg = this;
		FARDIALOGFLAGS Flags3 = 0
			| ((m_Flags & Far2::FDLG_WARNING) ? FDLG_WARNING : 0)
			| ((m_Flags & Far2::FDLG_SMALLDIALOG) ? FDLG_SMALLDIALOG : 0)
			| ((m_Flags & Far2::FDLG_NODRAWSHADOW) ? FDLG_NODRAWSHADOW : 0)
			| ((m_Flags & Far2::FDLG_NODRAWPANEL) ? FDLG_NODRAWPANEL : 0);
		hDlg3 = psi3.DialogInit(&m_PluginGuid, &m_Guid,
			m_X1, m_Y1, m_X2, m_Y2, m_HelpTopic, m_Items3, m_ItemsNumber, 0,
			Flags3, Far3DlgProc, (void*)m_Param);
		if (hDlg3)
		{
			wpi->MapDlg_2_3[this] = hDlg3;
			wpi->MapDlg_3_2[hDlg3] = this;
		}
	}
	
	if (hDlg3 != NULL)
	{
		wpi->LastFar2Dlg = this;
		iRc = psi3.DialogRun(hDlg3);

		// Некоторые параметры нужно перекинуть обратно в версию 2
		if (m_Items3 && m_Items2 && m_ItemsNumber > 0)
		{
			Far2::FarDialogItem *p2 = m_Items2;
			FarDialogItem *p3 = m_Items3;
			for (UINT i = 0; i < m_ItemsNumber; i++, p2++, p3++)
			{
				p2->Param.Reserved = p3->Reserved;
			}

		}

		// Освобождаем - в DialogFree
		//wpi->MapDlg_2_3.erase(this);
		//for (std::map<Far2Dialog*,HANDLE>::iterator i1 = wpi->MapDlg_2_3.begin();
		//	 i1 != wpi->MapDlg_2_3.end(); i++)
		//{
		//	if (i->first == this)
		//	{
		//		wpi->MapDlg_2_3.erase(i1);
		//		break;
		//	}
		//}
		//wpi->MapDlg_3_2.erase(hDlg3);
		//for (std::map<HANDLE,Far2Dialog*>::iterator i2 = wpi->MapDlg_3_2.begin();
		//	 i2 != wpi->MapDlg_3_2.end(); i++)
		//{
		//	if (i->first == hDlg3)
		//	{
		//		wpi->MapDlg_3_2.erase(i2);
		//		break;
		//	}
		//}
	}
	
	wpi->LastFar2Dlg = NULL;
	return iRc;
}

