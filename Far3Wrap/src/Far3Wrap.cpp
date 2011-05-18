//TODO: При ошибках загрузки плагина не ругаться сразу (MessageBox), а запомнить текст ошибки, 
// и пометить плагин ошибочным в списке плагинов (например, "RQP - Wrapper Failed")
//TODO: Переделать FarMessage_3_2, чтобы не использовать static переменные
//TODO: GetVirtualFindDataW/FreeVirtualFindDataW
//TODO: SetFindListW? Только если в фаре появится возможность "отключать" экспорты
//TODO: Конвертация CustomColumnData/CustomColumnNumber
//TODO: Объявить MCMD_GETAREA и FARMACROAREA в plguinW.hpp

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
#include <malloc.h>
#include <map>
#include <vector>
#include <crtdbg.h>
#include <TlHelp32.h>
#include "version.h"
#include "resourceW.h"

#define ZeroStruct(s) memset(&s, 0, sizeof(s))
//TODO
#define InvalidOp()


//#define ASSERTSTRUCT(s) { if (sizeof(Far2::s)!=sizeof(s)) AssertStructSize(sizeof(s), sizeof(Far2::s), L#s, __FILE__, __LINE__); }
//#define ASSERTSTRUCTGT(s) { if (sizeof(Far2::s)>sizeof(s)) AssertStructSize(sizeof(s), sizeof(Far2::s), L#s, __FILE__, __LINE__); }
#include "Assert3.h"


// class LogCmd, LOG_CMD, LOG_CMD0
#define LOG_COMMANDS
#include "LogCmd.h"


// Far2 & Far3 plugin API's
#define _FAR_NO_NAMELESS_UNIONS

namespace Far2
{
#undef __PLUGIN_HPP__
#include "pluginW.hpp"
#undef __FARKEYS_HPP__
#include "farkeys.hpp"
};

//namespace Far3
//{
#undef __PLUGIN_HPP__
#include "pluginW3.hpp"
#undef __FARKEYS_HPP__
#include "farkeys3.hpp"
//};

#include "Far3Wrap.h"

HMODULE ghFar3Wrap = NULL;
DWORD gnMainThreadId = 0;



struct WrapPluginInfo;

struct Far2Dialog
{
	// Far3
	HANDLE hDlg3;
	WrapPluginInfo* wpi;
	
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
    
	Far2Dialog(WrapPluginInfo* pwpi,
		int X1, int Y1, int X2, int Y2,
	    const wchar_t *HelpTopic, Far2::FarDialogItem *Items, UINT ItemsNumber,
	    DWORD Flags, Far2::FARWINDOWPROC DlgProc, LONG_PTR Param,
	    GUID PluginGuid, GUID DefGuid);
	~Far2Dialog();
};

std::map<Far2Dialog*,HANDLE> MapDlg_2_3;
std::map<HANDLE,Far2Dialog*> MapDlg_3_2;

typedef unsigned int LoaderFunctions3;
static const LoaderFunctions3
	LF3_None               = 0,
	LF3_Analyse            = 0x0001,
	LF3_CustomData         = 0x0002,
	LF3_Dialog             = 0x0004,
	LF3_Editor             = 0x0008,
	LF3_FindList           = 0x0010,
	LF3_Viewer             = 0x0020
	//LF3_Window             = (LF3_Dialog|LF3_Editor|LF3_Viewer),
	//LF3_Full               = (LF3_Analyse|LF3_Dialog|LF3_Editor|LF3_Viewer|LF3_Window)
	//LF3_FullCustom       = (LF3_Full|LF3_CustomData)
	;

//struct WrapUpdateFunctions
//{
//	LoaderFunctions3 mn_Functions; // set of LoaderFunctions3
//	int nResourceID;
//	wchar_t ms_Loader[MAX_PATH+1], ms_IniFile[MAX_PATH+1];
//};

//std::vector<WrapUpdateFunctions> UpdateFunc;

struct WrapPluginInfo
{
	// Instance variables
	HMODULE mh_Loader; // Loader.dll / Loader64.dll
	HMODULE mh_Dll;    // HMODULE собственно плагина
	LoaderFunctions3 mn_LoaderFunctions, mn_PluginFunctions; // set of LoaderFunctions3
	wchar_t* m_ErrorInfo; // In-pointer, Out-error text (used for Loader)
	int m_ErrorInfoMax; // max size of ErrorInfo in wchar_t (used for Loader)
	wchar_t ms_PluginDll[MAX_PATH+1], ms_IniFile[MAX_PATH+1];
	wchar_t ms_File[64];
	wchar_t ms_Title[MAX_PATH+1], ms_Desc[256], ms_Author[256];
	wchar_t ms_RegRoot[1024];
	VersionInfo m_Version;
	GUID mguid_Plugin, mguid_Dialogs;
	int mn_PluginMenu; GUID* mguids_PluginMenu;
	int mn_PluginDisks; GUID* mguids_PluginDisks;
	int mn_PluginConfig; GUID* mguids_PluginConfig;
	Far2::PluginInfo m_Info;
	InfoPanelLine *m_InfoLines; size_t m_InfoLinesNumber; // Far3
	PanelMode     *m_PanelModesArray; size_t m_PanelModesNumber; // Far3
	KeyBarTitles   m_KeyBar; // Far3
	KeyBarLabel    m_KeyBarLabels[7*12];
	Far2::FarList m_ListItems2;
	FarList m_ListItems3;

	AnalyseInfo* mp_Analyze;
	//;; 1 - (AnalyzeW [ & OpenW]) -> OpenFilePluginW
	//;; 2 - (AnalyzeW [ & OpenW]) -> OpenFilePluginW & ClosePluginW [& OpenFilePluginW]
	int m_AnalyzeMode;

	int m_OldPutFilesParams;

	Far2Dialog* m_LastFar2Dlg;

	std::map<PluginPanelItem*,Far2::PluginPanelItem*> m_MapPanelItems;
	std::map<Far2::FAR_FIND_DATA*,PluginPanelItem*> m_MapDirList;
	std::map<Far2::PluginPanelItem*,PluginPanelItem*> m_MapPlugDirList;

	int gnMsg_2 /*= 0*/, gnParam1_2 /*= 0*/, gnParam1_3 /*= 0*/;
	FARMESSAGE gnMsg_3 /*= DM_FIRST*/;
	FARMESSAGE gnMsgKey_3 /*= DM_FIRST*/, gnMsgClose_3 /*= DM_FIRST*/;
	LONG_PTR gnParam2_2 /*= 0*/;
	void* gpParam2_3 /*= NULL*/;
	FarListItem* gpListItems3 /*= NULL*/; INT_PTR gnListItemsMax3 /*= 0*/;
	Far2::FarListItem* gpListItems2 /*= NULL*/; UINT_PTR gnListItemsMax2 /*= 0*/;
	FarGetDialogItem m_GetDlgItem;

	BOOL lbPsi2 /*= FALSE*/;
	BOOL lbPsi3;
	PluginStartupInfo psi3;
	FarStandardFunctions FSF3;
	Far2::PluginStartupInfo psi2;
	Far2::FarStandardFunctions FSF2;


	Far2::FARAPIADVCONTROL FarApiAdvControlExp;
	Far2::FARAPICMPNAME FarApiCmpNameExp;
	Far2::FARAPICONTROL FarApiControlExp;
	Far2::FARAPIDEFDLGPROC FarApiDefDlgProcExp;
	Far2::FARAPIDIALOGFREE FarApiDialogFreeExp;
	Far2::FARAPIDIALOGINIT FarApiDialogInitExp;
	Far2::FARAPIDIALOGRUN FarApiDialogRunExp;
	Far2::FARAPIEDITOR FarApiEditorExp;
	Far2::FARAPIEDITORCONTROL FarApiEditorControlExp;
	Far2::FARAPIFILEFILTERCONTROL FarApiFileFilterControlExp;
	Far2::FARAPIFREEDIRLIST FarApiFreeDirListExp;
	Far2::FARAPIFREEPLUGINDIRLIST FarApiFreePluginDirListExp;
	Far2::FARAPIGETDIRLIST FarApiGetDirListExp;
	Far2::FARAPIGETMSG FarApiGetMsgExp;
	Far2::FARAPIGETPLUGINDIRLIST FarApiGetPluginDirListExp;
	Far2::FARAPIINPUTBOX FarApiInputBoxExp;
	Far2::FARAPIMENU FarApiMenuExp;
	Far2::FARAPIMESSAGE FarApiMessageExp;
	Far2::FARAPIPLUGINSCONTROL FarApiPluginsControlExp;
	Far2::FARAPIREGEXPCONTROL FarApiRegExpControlExp;
	Far2::FARAPIRESTORESCREEN FarApiRestoreScreenExp;
	Far2::FARAPISAVESCREEN FarApiSaveScreenExp;
	Far2::FARAPISENDDLGMESSAGE FarApiSendDlgMessageExp;
	Far2::FARAPISHOWHELP FarApiShowHelpExp;
	Far2::FARAPITEXT FarApiTextExp;
	Far2::FARAPIVIEWER FarApiViewerExp;
	Far2::FARAPIVIEWERCONTROL FarApiViewerControlExp;
	Far2::FARCONVERTPATH FarConvertPathExp;
	Far2::FARGETCURRENTDIRECTORY FarGetCurrentDirectoryExp;
	Far2::FARGETREPARSEPOINTINFO FarGetReparsePointInfoExp;
	Far2::FARSTDGETFILEOWNER FarStdGetFileOwnerExp;
	Far2::FARSTDGETPATHROOT FarStdGetPathRootExp;
	Far2::FARSTDMKLINK FarStdMkLinkExp;
	Far2::FARSTDMKTEMP FarStdMkTempExp;
	Far2::FARSTDPROCESSNAME FarStdProcessNameExp;
	Far2::FARSTDRECURSIVESEARCH FarStdRecursiveSearchExp;
	Far2::FARSTDXLAT FarStdXlatExp;


/* ******************************** */

	// Ctors
	WrapPluginInfo(Far3WrapFunctions *pInfo2);
	~WrapPluginInfo();

	BOOL LoadPlugin(BOOL abSilent);
	void LoadPluginInfo();
	void CheckPluginExports(LoaderFunctions3 eFunc);

	void UnloadPlugin();
	void ClearProcAddr();

	KeyBarTitles* KeyBarTitles_2_3(const Far2::KeyBarTitles* KeyBar);

	static int OpMode_3_2(OPERATION_MODES OpMode3);
	static int OpenFrom_3_2(OPENFROM OpenFrom3);
	static OPENPANELINFO_FLAGS OpenPanelInfoFlags_2_3(DWORD Flags2);
	static Far2::OPENPLUGININFO_SORTMODES SortMode_3_2(OPENPANELINFO_SORTMODES Mode3);
	static OPENPANELINFO_SORTMODES SortMode_2_3(/*Far2::OPENPLUGININFO_SORTMODES*/int Mode2);
	static PLUGINPANELITEMFLAGS PluginPanelItemFlags_2_3(DWORD Flags2);
	static DWORD PluginPanelItemFlags_3_2(PLUGINPANELITEMFLAGS Flags3);
	static void PluginPanelItem_2_3(const Far2::PluginPanelItem* p2, PluginPanelItem* p3);
	PluginPanelItem* PluginPanelItems_2_3(const Far2::PluginPanelItem* pItems, int ItemsNumber);
	static void PluginPanelItem_3_2(const PluginPanelItem* p3, Far2::PluginPanelItem* p2);
	static void PluginPanelItem_3_2(const PluginPanelItem *p3, Far2::FAR_FIND_DATA* p2);
	static void PluginPanelItem_2_3(const Far2::FAR_FIND_DATA* p2, PluginPanelItem *p3);
	Far2::PluginPanelItem* PluginPanelItems_3_2(const PluginPanelItem* pItems, int ItemsNumber);
	void FarKey_2_3(int Key2, INPUT_RECORD *r);
	DWORD FarKey_3_2(const INPUT_RECORD *Rec);
	static FARDIALOGITEMTYPES DialogItemTypes_2_3(int ItemType2);
	static int DialogItemTypes_3_2(FARDIALOGITEMTYPES ItemType3);
	static DWORD FarDialogItemFlags_3_2(FarDialogItemFlags Flags3);
	static FarDialogItemFlags FarDialogItemFlags_2_3(DWORD Flags2);
	void FarListItem_2_3(const Far2::FarListItem* p2, FarListItem* p3);
	void FarListItem_3_2(const FarListItem* p3, Far2::FarListItem* p2);
	void FarDialogItem_2_3(const Far2::FarDialogItem *p2, FarDialogItem *p3, FarList *pList3);
	void FarDialogItem_3_2(const FarDialogItem *p3, /*size_t nAllocated3,*/ Far2::FarDialogItem *p2, Far2::FarList *pList2);
	LONG_PTR CallDlgProc_2_3(FARAPIDEFDLGPROC DlgProc3, HANDLE hDlg2, const int Msg2, const int Param1, LONG_PTR Param2);
	Far2::FarMessagesProc FarMessage_3_2(const int Msg3, const int Param1, void*& Param2);
	void FarMessageParam_2_3(const int Msg2, const int Param1, const void* Param2, void* OrgParam2, LONG_PTR lRc);
	InfoPanelLine* InfoLines_2_3(const Far2::InfoPanelLine *InfoLines, int InfoLinesNumber);
	PanelMode* PanelModes_2_3(const Far2::PanelMode *PanelModesArray, int PanelModesNumber);
	static LPCWSTR FormatGuid(GUID* guid, wchar_t* tmp, BOOL abQuote = FALSE);


/* ******************************** */

	static FARPROC WINAPI GetProcAddressWrap(struct WrapPluginInfo* wpi, HMODULE hModule, LPCSTR lpProcName);
	       FARPROC        GetProcAddressW3  (HMODULE hModule, LPCSTR lpProcName);
	       
/* ******************************** */

	static void   WINAPI SetStartupInfoWrap(struct WrapPluginInfo* wpi, PluginStartupInfo *Info);
	       void          SetStartupInfoW3  (PluginStartupInfo *Info);
	static void   WINAPI GetGlobalInfoWrap(struct WrapPluginInfo* wpi, GlobalInfo *Info);
	       void          GetGlobalInfoW3  (GlobalInfo *Info);
	static void   WINAPI GetPluginInfoWrap(struct WrapPluginInfo* wpi, PluginInfo *Info);
	       void          GetPluginInfoW3  (PluginInfo *Info);
	static HANDLE WINAPI OpenWrap(struct WrapPluginInfo* wpi, const OpenInfo *Info);
	       HANDLE        OpenW3  (const OpenInfo *Info);
	static int    WINAPI AnalyseWrap(struct WrapPluginInfo* wpi, const AnalyseInfo *Info);
	       int           AnalyseW3  (const AnalyseInfo *Info);
	static void   WINAPI ClosePanelWrap(struct WrapPluginInfo* wpi, HANDLE hPanel);
	       void          ClosePanelW3  (HANDLE hPanel);
	static int    WINAPI CompareWrap(struct WrapPluginInfo* wpi, const CompareInfo *Info);
	       int           CompareW3  (const CompareInfo *Info);
	static int    WINAPI ConfigureWrap(struct WrapPluginInfo* wpi, const GUID* Guid);
	       int           ConfigureW3  (const GUID* Guid);
	static int    WINAPI DeleteFilesWrap(struct WrapPluginInfo* wpi, const DeleteFilesInfo *Info);
	       int           DeleteFilesW3  (const DeleteFilesInfo *Info);
	static void   WINAPI ExitFARWrap(struct WrapPluginInfo* wpi);
	       void          ExitFARW3  (void);
	static void   WINAPI FreeVirtualFindDataWrap(struct WrapPluginInfo* wpi, const FreeFindDataInfo *Info);
	       void          FreeVirtualFindDataW3  (const FreeFindDataInfo *Info);
	static int    WINAPI GetFilesWrap(struct WrapPluginInfo* wpi, GetFilesInfo *Info);
	       int           GetFilesW3  (GetFilesInfo *Info);
	static int    WINAPI GetFindDataWrap(struct WrapPluginInfo* wpi, GetFindDataInfo *Info);
	       int           GetFindDataW3  (GetFindDataInfo *Info);
	static void   WINAPI FreeFindDataWrap(struct WrapPluginInfo* wpi, const FreeFindDataInfo *Info);
	       void          FreeFindDataW3  (const FreeFindDataInfo *Info);
	static void   WINAPI GetOpenPanelInfoWrap(struct WrapPluginInfo* wpi, OpenPanelInfo *Info);
	       void          GetOpenPanelInfoW3  (OpenPanelInfo *Info);
	static int    WINAPI GetVirtualFindDataWrap(struct WrapPluginInfo* wpi, GetVirtualFindDataInfo *Info);
	       int           GetVirtualFindDataW3  (GetVirtualFindDataInfo *Info);
	static int    WINAPI MakeDirectoryWrap(struct WrapPluginInfo* wpi, MakeDirectoryInfo *Info);
	       int           MakeDirectoryW3  (MakeDirectoryInfo *Info);
	static int    WINAPI ProcessDialogEventWrap(struct WrapPluginInfo* wpi, int Event,void *Param);
	       int           ProcessDialogEventW3  (int Event,void *Param);
	static int    WINAPI ProcessEditorEventWrap(struct WrapPluginInfo* wpi, int Event,void *Param);
	       int           ProcessEditorEventW3  (int Event,void *Param);
	static int    WINAPI ProcessEditorInputWrap(struct WrapPluginInfo* wpi, const ProcessEditorInputInfo *Info);
	       int           ProcessEditorInputW3  (const ProcessEditorInputInfo *Info);
	static int    WINAPI ProcessEventWrap(struct WrapPluginInfo* wpi, HANDLE hPanel,int Event,void *Param);
	       int           ProcessEventW3  (HANDLE hPanel,int Event,void *Param);
	static int    WINAPI ProcessHostFileWrap(struct WrapPluginInfo* wpi, const ProcessHostFileInfo *Info);
	       int           ProcessHostFileW3  (const ProcessHostFileInfo *Info);
	static int    WINAPI ProcessPanelInputWrap(struct WrapPluginInfo* wpi, HANDLE hPanel,const struct ProcessPanelInputInfo *Info);
	       int           ProcessPanelInputW3  (HANDLE hPanel,const struct ProcessPanelInputInfo *Info);
	static int    WINAPI ProcessConsoleInputWrap(struct WrapPluginInfo* wpi, ProcessConsoleInputInfo *Info);
	       int           ProcessConsoleInputW3  (ProcessConsoleInputInfo *Info);
	static int    WINAPI ProcessSynchroEventWrap(struct WrapPluginInfo* wpi, int Event,void *Param);
	       int           ProcessSynchroEventW3  (int Event,void *Param);
	static int    WINAPI ProcessViewerEventWrap(struct WrapPluginInfo* wpi, int Event,void *Param);
	       int           ProcessViewerEventW3  (int Event,void *Param);
	static int    WINAPI PutFilesWrap(struct WrapPluginInfo* wpi, const PutFilesInfo *Info);
	       int           PutFilesW3  (const PutFilesInfo *Info);
	static int    WINAPI SetDirectoryWrap(struct WrapPluginInfo* wpi, const SetDirectoryInfo *Info);
	       int           SetDirectoryW3  (const SetDirectoryInfo *Info);
	static int    WINAPI SetFindListWrap(struct WrapPluginInfo* wpi, const SetFindListInfo *Info);
	       int           SetFindListW3  (const SetFindListInfo *Info);
	static int    WINAPI GetCustomDataWrap(struct WrapPluginInfo* wpi, const wchar_t *FilePath, wchar_t **CustomData);
	       int    WINAPI GetCustomDataW3  (const wchar_t *FilePath, wchar_t **CustomData);
	static void   WINAPI FreeCustomDataWrap(struct WrapPluginInfo* wpi, wchar_t *CustomData);
	       void   WINAPI FreeCustomDataW3  (wchar_t *CustomData);


/* ******************************** */

	// Some internal typedefs
	struct RecSearchUserFnArg
	{
		Far2::FRSUSERFUNC UserFn2;
		void *Param2;
	};
	static int WINAPI RecSearchUserFn(const struct PluginPanelItem *FData, const wchar_t *FullName, void *Param);

	// Changed functions
	static LONG_PTR WINAPI FarApiDefDlgProcWrap(WrapPluginInfo* wpi, HANDLE hDlg, int Msg, int Param1, LONG_PTR Param2);
	       LONG_PTR        FarApiDefDlgProc(HANDLE hDlg, int Msg, int Param1, LONG_PTR Param2);
	static LONG_PTR WINAPI FarApiSendDlgMessageWrap(WrapPluginInfo* wpi, HANDLE hDlg, int Msg, int Param1, LONG_PTR Param2);
	       LONG_PTR        FarApiSendDlgMessage(HANDLE hDlg, int Msg, int Param1, LONG_PTR Param2);
	static BOOL WINAPI FarApiShowHelpWrap(WrapPluginInfo* wpi, const wchar_t *ModuleName, const wchar_t *Topic, DWORD Flags);
	       BOOL        FarApiShowHelp(const wchar_t *ModuleName, const wchar_t *Topic, DWORD Flags);
	static HANDLE WINAPI FarApiSaveScreenWrap(WrapPluginInfo* wpi, int X1, int Y1, int X2, int Y2);
	       HANDLE        FarApiSaveScreen(int X1, int Y1, int X2, int Y2);
	static void WINAPI FarApiRestoreScreenWrap(WrapPluginInfo* wpi, HANDLE hScreen);
	       void        FarApiRestoreScreen(HANDLE hScreen);
	static void WINAPI FarApiTextWrap(WrapPluginInfo* wpi, int X, int Y, int Color, const wchar_t *Str);
	       void        FarApiText(int X, int Y, int Color, const wchar_t *Str);
	static int WINAPI FarApiEditorWrap(WrapPluginInfo* wpi, const wchar_t *FileName, const wchar_t *Title, int X1, int Y1, int X2, int Y2, DWORD Flags, int StartLine, int StartChar, UINT CodePage);
	       int        FarApiEditor(const wchar_t *FileName, const wchar_t *Title, int X1, int Y1, int X2, int Y2, DWORD Flags, int StartLine, int StartChar, UINT CodePage);
	static int WINAPI FarApiViewerWrap(WrapPluginInfo* wpi, const wchar_t *FileName, const wchar_t *Title, int X1, int Y1, int X2, int Y2, DWORD Flags, UINT CodePage);
	       int        FarApiViewer(const wchar_t *FileName, const wchar_t *Title, int X1, int Y1, int X2, int Y2, DWORD Flags, UINT CodePage);
	static int WINAPI FarApiMenuWrap(WrapPluginInfo* wpi, INT_PTR PluginNumber, int X, int Y, int MaxHeight, DWORD Flags, const wchar_t* Title, const wchar_t* Bottom, const wchar_t* HelpTopic, const int* BreakKeys, int* BreakCode, const struct Far2::FarMenuItem *Item, int ItemsNumber);
	       int        FarApiMenu(INT_PTR PluginNumber, int X, int Y, int MaxHeight, DWORD Flags, const wchar_t* Title, const wchar_t* Bottom, const wchar_t* HelpTopic, const int* BreakKeys, int* BreakCode, const struct Far2::FarMenuItem *Item, int ItemsNumber);
	static int WINAPI FarApiMessageWrap(WrapPluginInfo* wpi, INT_PTR PluginNumber, DWORD Flags, const wchar_t *HelpTopic, const wchar_t * const *Items, int ItemsNumber, int ButtonsNumber);
	       int        FarApiMessage(INT_PTR PluginNumber, DWORD Flags, const wchar_t *HelpTopic, const wchar_t * const *Items, int ItemsNumber, int ButtonsNumber);
	static HANDLE WINAPI FarApiDialogInitWrap(WrapPluginInfo* wpi, INT_PTR PluginNumber, int X1, int Y1, int X2, int Y2, const wchar_t* HelpTopic, struct Far2::FarDialogItem *Item, unsigned int ItemsNumber, DWORD Reserved, DWORD Flags, Far2::FARWINDOWPROC DlgProc, LONG_PTR Param);
	       HANDLE        FarApiDialogInit(INT_PTR PluginNumber, int X1, int Y1, int X2, int Y2, const wchar_t* HelpTopic, struct Far2::FarDialogItem *Item, unsigned int ItemsNumber, DWORD Reserved, DWORD Flags, Far2::FARWINDOWPROC DlgProc, LONG_PTR Param);
	static int WINAPI FarApiDialogRunWrap(WrapPluginInfo* wpi, HANDLE hDlg);
	       int        FarApiDialogRun(HANDLE hDlg);
	static void WINAPI FarApiDialogFreeWrap(WrapPluginInfo* wpi, HANDLE hDlg);
	       void        FarApiDialogFree(HANDLE hDlg);
	static int WINAPI FarApiControlWrap(WrapPluginInfo* wpi, HANDLE hPlugin, int Command, int Param1, LONG_PTR Param2);
	       int        FarApiControl(HANDLE hPlugin, int Command, int Param1, LONG_PTR Param2);
	static int WINAPI FarApiGetDirListWrap(WrapPluginInfo* wpi, const wchar_t *Dir, struct Far2::FAR_FIND_DATA **pPanelItem, int *pItemsNumber);
	       int        FarApiGetDirList(const wchar_t *Dir, struct Far2::FAR_FIND_DATA **pPanelItem, int *pItemsNumber);
	static void WINAPI FarApiFreeDirListWrap(WrapPluginInfo* wpi, struct Far2::FAR_FIND_DATA *PanelItem, int nItemsNumber);
	       void        FarApiFreeDirList(struct Far2::FAR_FIND_DATA *PanelItem, int nItemsNumber);
	static int WINAPI FarApiGetPluginDirListWrap(WrapPluginInfo* wpi, INT_PTR PluginNumber, HANDLE hPlugin, const wchar_t *Dir, struct Far2::PluginPanelItem **pPanelItem, int *pItemsNumber);
	       int        FarApiGetPluginDirList(INT_PTR PluginNumber, HANDLE hPlugin, const wchar_t *Dir, struct Far2::PluginPanelItem **pPanelItem, int *pItemsNumber);
	static void WINAPI FarApiFreePluginDirListWrap(WrapPluginInfo* wpi, struct Far2::PluginPanelItem *PanelItem, int nItemsNumber);
	       void        FarApiFreePluginDirList(struct Far2::PluginPanelItem *PanelItem, int nItemsNumber);
	static int WINAPI FarApiCmpNameWrap(WrapPluginInfo* wpi, const wchar_t *Pattern, const wchar_t *String, int SkipPath);
	       int        FarApiCmpName(const wchar_t *Pattern, const wchar_t *String, int SkipPath);
	static LPCWSTR WINAPI FarApiGetMsgWrap(WrapPluginInfo* wpi, INT_PTR PluginNumber, int MsgId);
	       LPCWSTR        FarApiGetMsg(INT_PTR PluginNumber, int MsgId);
	static INT_PTR WINAPI FarApiAdvControlWrap(WrapPluginInfo* wpi, INT_PTR ModuleNumber, int Command, void *Param);
	       INT_PTR        FarApiAdvControl(INT_PTR ModuleNumber, int Command, void *Param);
	static int WINAPI FarApiViewerControlWrap(WrapPluginInfo* wpi, int Command, void *Param);
	       int        FarApiViewerControl(int Command, void *Param);
	static int WINAPI FarApiEditorControlWrap(WrapPluginInfo* wpi, int Command, void *Param);
	       int        FarApiEditorControl(int Command, void *Param);
	static int WINAPI FarApiInputBoxWrap(WrapPluginInfo* wpi, const wchar_t *Title, const wchar_t *SubTitle, const wchar_t *HistoryName, const wchar_t *SrcText, wchar_t *DestText, int   DestLength, const wchar_t *HelpTopic, DWORD Flags);
	       int        FarApiInputBox(const wchar_t *Title, const wchar_t *SubTitle, const wchar_t *HistoryName, const wchar_t *SrcText, wchar_t *DestText, int   DestLength, const wchar_t *HelpTopic, DWORD Flags);
	static int WINAPI FarApiPluginsControlWrap(WrapPluginInfo* wpi, HANDLE hHandle, int Command, int Param1, LONG_PTR Param2);
	       int        FarApiPluginsControl(HANDLE hHandle, int Command, int Param1, LONG_PTR Param2);
	static int WINAPI FarApiFileFilterControlWrap(WrapPluginInfo* wpi, HANDLE hHandle, int Command, int Param1, LONG_PTR Param2);
	       int        FarApiFileFilterControl(HANDLE hHandle, int Command, int Param1, LONG_PTR Param2);
	static int WINAPI FarApiRegExpControlWrap(WrapPluginInfo* wpi, HANDLE hHandle, int Command, LONG_PTR Param);
	       int        FarApiRegExpControl(HANDLE hHandle, int Command, LONG_PTR Param);
	static int WINAPI FarStdGetFileOwnerWrap(WrapPluginInfo* wpi, const wchar_t *Computer,const wchar_t *Name,wchar_t *Owner,int Size);
	       int        FarStdGetFileOwner(const wchar_t *Computer,const wchar_t *Name,wchar_t *Owner,int Size);
	static int WINAPI FarStdGetPathRootWrap(WrapPluginInfo* wpi, const wchar_t *Path,wchar_t *Root, int DestSize);
	       int        FarStdGetPathRoot(const wchar_t *Path,wchar_t *Root, int DestSize);
	static wchar_t* WINAPI FarStdXlatWrap(WrapPluginInfo* wpi, wchar_t *Line,int StartPos,int EndPos,DWORD Flags);
	       wchar_t*        FarStdXlatW3(wchar_t *Line,int StartPos,int EndPos,DWORD Flags);
	static void WINAPI FarStdRecursiveSearchWrap(WrapPluginInfo* wpi, const wchar_t *InitDir,const wchar_t *Mask,Far2::FRSUSERFUNC Func,DWORD Flags,void *Param);
	       void        FarStdRecursiveSearch(const wchar_t *InitDir,const wchar_t *Mask,Far2::FRSUSERFUNC Func,DWORD Flags,void *Param);
	static int WINAPI FarStdMkTempWrap(WrapPluginInfo* wpi, wchar_t *Dest, DWORD size, const wchar_t *Prefix);
	       int        FarStdMkTemp(wchar_t *Dest, DWORD size, const wchar_t *Prefix);
	static int WINAPI FarStdProcessNameWrap(WrapPluginInfo* wpi, const wchar_t *param1, wchar_t *param2, DWORD size, DWORD flags);
	       int        FarStdProcessName(const wchar_t *param1, wchar_t *param2, DWORD size, DWORD flags);
	static int WINAPI FarStdMkLinkWrap(WrapPluginInfo* wpi, const wchar_t *Src,const wchar_t *Dest,DWORD Flags);
	       int        FarStdMkLink(const wchar_t *Src,const wchar_t *Dest,DWORD Flags);
	static int WINAPI FarConvertPathWrap(WrapPluginInfo* wpi, enum Far2::CONVERTPATHMODES Mode, const wchar_t *Src, wchar_t *Dest, int DestSize);
	       int        FarConvertPath(enum Far2::CONVERTPATHMODES Mode, const wchar_t *Src, wchar_t *Dest, int DestSize);
	static int WINAPI FarGetReparsePointInfoWrap(WrapPluginInfo* wpi, const wchar_t *Src, wchar_t *Dest,int DestSize);
	       int        FarGetReparsePointInfo(const wchar_t *Src, wchar_t *Dest,int DestSize);
	static DWORD WINAPI FarGetCurrentDirectoryWrap(WrapPluginInfo* wpi, DWORD Size,wchar_t* Buffer);
	       DWORD        FarGetCurrentDirectory(DWORD Size,wchar_t* Buffer);


/* ******************************** */

	// Exported Far2(!) Function
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
	typedef int (WINAPI* _GetCustomDataW)(const wchar_t *FilePath, wchar_t **CustomData);
	_GetCustomDataW GetCustomDataW;
	typedef void (WINAPI* _FreeCustomDataW)(wchar_t *CustomData);
	_FreeCustomDataW FreeCustomDataW;
	// End of Exported Far2(!) Function
};
//WrapPluginInfo* wpi = NULL;


WrapPluginInfo::WrapPluginInfo(Far3WrapFunctions *pInfo2)
{
	mh_Loader = pInfo2->hLoader;
	mh_Dll = NULL;
	mn_PluginFunctions = mn_LoaderFunctions = LF3_None;
	ms_PluginDll[0] = ms_IniFile[0] = ms_Title[0] = ms_Desc[0] = ms_Author[0] = ms_RegRoot[0] = ms_File[0] = 0;
	memset(&m_Version, 0, sizeof(m_Version));
	memset(&mguid_Plugin, 0, sizeof(mguid_Plugin));
	memset(&mguid_Dialogs, 0, sizeof(mguid_Dialogs));
	mn_PluginMenu = mn_PluginDisks = mn_PluginConfig = 0;
	mguids_PluginMenu = mguids_PluginDisks = mguids_PluginConfig = NULL;
	memset(&m_Info, 0, sizeof(m_Info));
	m_InfoLines = NULL; m_InfoLinesNumber = 0;
	m_PanelModesArray = NULL; m_PanelModesNumber = 0;
	m_KeyBar.CountLabels = 0; m_KeyBar.Labels = m_KeyBarLabels;
	m_LastFar2Dlg = NULL;
	m_OldPutFilesParams = 0;
	m_AnalyzeMode = 2;
	ZeroStruct(m_ListItems2); ZeroStruct(m_ListItems3);
	ZeroStruct(m_GetDlgItem);

	gnMsg_2 = 0; gnParam1_2 = 0; gnParam1_3 = 0;
	gnMsg_3 = DM_FIRST;
	gnMsgKey_3 = DM_FIRST; gnMsgClose_3 = DM_FIRST;
	gnParam2_2 = 0;
	gpParam2_3 = NULL;
	gpListItems3 = NULL; gnListItemsMax3 = 0;
	gpListItems2 = NULL; gnListItemsMax2 = 0;

	lbPsi2 = FALSE;
	lbPsi3 = FALSE;
	ZeroStruct(psi3);
	ZeroStruct(FSF3);
	ZeroStruct(psi2);
	ZeroStruct(FSF2);

	ClearProcAddr();
	mp_Analyze = NULL;

	m_ErrorInfo = pInfo2->ErrorInfo;
	m_ErrorInfoMax = pInfo2->ErrorInfoMax;

	#undef SET_FN
	#define SET_FN(n) n##Exp = pInfo2->n
	SET_FN(FarApiAdvControl);
	SET_FN(FarApiCmpName);
	SET_FN(FarApiControl);
	SET_FN(FarApiDefDlgProc);
	SET_FN(FarApiDialogFree);
	SET_FN(FarApiDialogInit);
	SET_FN(FarApiDialogRun);
	SET_FN(FarApiEditor);
	SET_FN(FarApiEditorControl);
	SET_FN(FarApiFileFilterControl);
	SET_FN(FarApiFreeDirList);
	SET_FN(FarApiFreePluginDirList);
	SET_FN(FarApiGetDirList);
	SET_FN(FarApiGetMsg);
	SET_FN(FarApiGetPluginDirList);
	SET_FN(FarApiInputBox);
	SET_FN(FarApiMenu);
	SET_FN(FarApiMessage);
	SET_FN(FarApiPluginsControl);
	SET_FN(FarApiRegExpControl);
	SET_FN(FarApiRestoreScreen);
	SET_FN(FarApiSaveScreen);
	SET_FN(FarApiSendDlgMessage);
	SET_FN(FarApiShowHelp);
	SET_FN(FarApiText);
	SET_FN(FarApiViewer);
	SET_FN(FarApiViewerControl);
	SET_FN(FarConvertPath);
	SET_FN(FarGetCurrentDirectory);
	SET_FN(FarGetReparsePointInfo);
	SET_FN(FarStdGetFileOwner);
	SET_FN(FarStdGetPathRoot);
	SET_FN(FarStdMkLink);
	SET_FN(FarStdMkTemp);
	SET_FN(FarStdProcessName);
	SET_FN(FarStdRecursiveSearch);
	SET_FN(FarStdXlat);
	#undef SET_FN
};

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
	if (mguids_PluginMenu)
		free(mguids_PluginMenu);
	if (mguids_PluginDisks)
		free(mguids_PluginDisks);
	if (mguids_PluginConfig)
		free(mguids_PluginConfig);
	if (mp_Analyze)
		free(mp_Analyze);
	if (m_InfoLines)
		free(m_InfoLines);
	if (m_PanelModesArray)
		free(m_PanelModesArray);
	_ASSERTE(m_KeyBar.Labels==m_KeyBarLabels);
	if (m_GetDlgItem.Item)
		free(m_GetDlgItem.Item);
}

void WrapPluginInfo::LoadPluginInfo()
{
	BOOL lbRc = FALSE;
	wchar_t szSelf[MAX_PATH+1]; szSelf[0] = 0;
	wchar_t szIni[MAX_PATH+1], szTemp[2048];
	
	if (GetModuleFileName(mh_Loader, szSelf, ARRAYSIZE(szSelf)-4))
	{
		GUID tmpGuid = {0}; wchar_t szTmp[64];
		HANDLE hIniFile = NULL;
		BOOL lbNewIniFile = FALSE;
		lstrcpy(szIni, szSelf);
		wchar_t* pszSelfName = wcsrchr(szSelf, L'\\');
		if (pszSelfName) pszSelfName++; else pszSelfName = szSelf;
		wchar_t* pszSlash = wcsrchr(szIni, L'\\');
		wchar_t* pszFilePtr = NULL;
		if (!pszSlash) pszSlash = szIni;
		wchar_t* pszDot = wcsrchr(pszSlash, L'.');
		if (pszDot)
			*pszDot = 0;
		lstrcat(szIni, L".ini");
		lstrcpy(ms_IniFile, szIni);

		
		lstrcpy(ms_PluginDll, szSelf);
		pszSlash = wcsrchr(ms_PluginDll, L'\\');
		//if (pszSlash) pszSlash++; else pszSlash = PluginDll;
		pszFilePtr = pszSlash ? (pszSlash+1) : ms_PluginDll;
		hIniFile = CreateFile(szIni, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
		if (hIniFile == INVALID_HANDLE_VALUE)
		{
			hIniFile = CreateFile(szIni, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			if (hIniFile != INVALID_HANDLE_VALUE)
			{
				CloseHandle(hIniFile);
				// OK, создали, теперь нужно сформировать информацию по старому плагину
				lbNewIniFile = TRUE;
				//if (pszSlash) *pszSlash = 0;
				//FSF3.FarRecursiveSearch(PluginDll, L"*.dll", WrapPluginInfo::SeekForPlugin,
				//	(FRSMODE)0, this);
				//if (pszSlash) *pszSlash = L'\\';
			}
		}
		else
		{
			CloseHandle(hIniFile);
		}

		if (!lbNewIniFile)
		{
			if (!GetPrivateProfileString(L"Plugin", L"PluginFile", L"", pszFilePtr, ARRAYSIZE(ms_PluginDll)-lstrlen(ms_PluginDll), szIni))
			{
				//PluginDll[0] = 0;
				lbNewIniFile = TRUE;
			}
			else
				lstrcpyn(ms_File, pszFilePtr, ARRAYSIZE(ms_File));
		}

		if (lbNewIniFile)
		{
			
			WIN32_FIND_DATA fnd;
			HMODULE hTestDll = NULL;
			FARPROC lpSetStartupInfoW = NULL, lpGetGlobalInfoW = NULL; // просто для информации, звать их нельзя
			HANDLE hFind = NULL;
			for (int i = 0; !hTestDll && i <= 1; i++)
			{
				lstrcpy(pszFilePtr, i ? L"*.dl_" : L"*.dll");
				hFind = FindFirstFile(ms_PluginDll, &fnd);
				if (hFind && (hFind != INVALID_HANDLE_VALUE))
				{
					*pszFilePtr = 0;
					int nLen = ARRAYSIZE(ms_PluginDll)-lstrlen(ms_PluginDll);
					do {
						if ((fnd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
							continue;
						if (lstrcmpi(pszSelfName, fnd.cFileName) == 0)
							continue; // это мы
						lstrcpy(pszFilePtr, fnd.cFileName);
						hTestDll = LoadLibraryEx(ms_PluginDll, NULL, DONT_RESOLVE_DLL_REFERENCES|LOAD_WITH_ALTERED_SEARCH_PATH);
						if (hTestDll)
						{
							lpSetStartupInfoW = GetProcAddress(hTestDll, "SetStartupInfoW");
							lpGetGlobalInfoW = GetProcAddress(hTestDll, "GetGlobalInfoW");
							FreeLibrary(hTestDll);
							if (lpSetStartupInfoW && !lpGetGlobalInfoW)
								break; // Да, это Far2 плагин!
							hTestDll = NULL; // продолжаем поиск
						}
					} while (FindNextFile(hFind, &fnd));
					FindClose(hFind);
				}
			}

			if (hTestDll)
			{
				lstrcpyn(ms_File, pszFilePtr, ARRAYSIZE(ms_File));
				// OK, Инициализируем .ini файл
				DWORD dwErr = 0;
				BOOL lb = WritePrivateProfileString(L"Plugin", L"PluginFile", pszFilePtr, szIni);
				if (!lb)
					dwErr = GetLastError();
				//lb = WritePrivateProfileString(L"Plugin", L"DisabledFunctions", L"0", szIni);
				lb = WritePrivateProfileString(L"Plugin", L"AnalyzeMode", L"2", szIni);
				lb = WritePrivateProfileString(L"Plugin", L"OldPutFilesParams", L"0", szIni);
				lb = WritePrivateProfileString(L"Plugin", L"Title", pszFilePtr, szIni);
				lb = WritePrivateProfileString(L"Plugin", L"Description", pszFilePtr, szIni);
				lb = WritePrivateProfileString(L"Plugin", L"Author", L"<Unknown>", szIni);
				lb = WritePrivateProfileString(L"Plugin", L"RegRoot", L"Software\\Far Manager\\Plugins", szIni);
				lb = WritePrivateProfileString(L"Plugin", L"Version", L"1.0.0.0", szIni);
				// Начальные GUID-ы
				UuidCreate(&tmpGuid);
				lb = WritePrivateProfileString(L"Plugin", L"GUID", FormatGuid(&tmpGuid, szTmp), szIni);
				UuidCreate(&tmpGuid);
				lb = WritePrivateProfileString(L"Plugin", L"DialogsGUID", FormatGuid(&tmpGuid, szTmp), szIni);
			}
			else
			{
				ms_File[0] = 0;
			}
		}

		if (hIniFile && (hIniFile != INVALID_HANDLE_VALUE))
		{
			//if (!GetPrivateProfileString(L"Plugin", L"PluginFile", L"", pszFilePtr, ARRAYSIZE(PluginDll)-lstrlen(PluginDll), szIni))
			//	PluginDll[0] = 0;
			//else
			//	lstrcpyn(File, pszFilePtr, ARRAYSIZE(File));

			//mn_OldAbsentFunctions = GetPrivateProfileInt(L"Plugin", L"DisabledFunctions", 0, szIni);

			m_AnalyzeMode = GetPrivateProfileInt(L"Plugin", L"AnalyzeMode", 2, szIni);
			if (m_AnalyzeMode != 1 && m_AnalyzeMode != 2)
				m_AnalyzeMode = 2;

			m_OldPutFilesParams = GetPrivateProfileInt(L"Plugin", L"OldPutFilesParams", 0, szIni);
			if (m_OldPutFilesParams != 0 && m_OldPutFilesParams != 1)
				m_OldPutFilesParams = 0;
			
			if (GetPrivateProfileString(L"Plugin", L"Title", L"Sample Far3 plugin", szTemp, ARRAYSIZE(szTemp), szIni))
				lstrcpyn(ms_Title, szTemp, ARRAYSIZE(ms_Title));
			if (GetPrivateProfileString(L"Plugin", L"Description", L"Far2->Far3 plugin wrapper", szTemp, ARRAYSIZE(szTemp), szIni))
				lstrcpyn(ms_Desc, szTemp, ARRAYSIZE(ms_Desc));
			if (GetPrivateProfileString(L"Plugin", L"Author", L"Maximus5", szTemp, ARRAYSIZE(szTemp), szIni))
				lstrcpyn(ms_Author, szTemp, ARRAYSIZE(ms_Author));
			if (GetPrivateProfileString(L"Plugin", L"RegRoot", L"Software\\Far Manager\\Plugins", szTemp, ARRAYSIZE(szTemp), szIni))
				lstrcpyn(ms_RegRoot, szTemp, ARRAYSIZE(ms_RegRoot));
			if (GetPrivateProfileString(L"Plugin", L"Version", L"1.0.0.0", szTemp, ARRAYSIZE(szTemp), szIni))
			{
				//TODO: Обработка версии
			}
			GUID guid;
			if (GetPrivateProfileString(L"Plugin", L"GUID", L"", szTemp, ARRAYSIZE(szTemp), szIni))
			{
				if (UuidFromStringW((RPC_WSTR)szTemp, &guid) == RPC_S_OK)
					mguid_Plugin = guid;
				else
				{
					UuidCreate(&mguid_Plugin);
					WritePrivateProfileString(L"Plugin", L"GUID", FormatGuid(&mguid_Plugin, szTmp), szIni);
				}
			}
			if (GetPrivateProfileString(L"Plugin", L"DialogsGUID", L"", szTemp, ARRAYSIZE(szTemp), szIni))
			{
				if (UuidFromStringW((RPC_WSTR)szTemp, &guid) == RPC_S_OK)
					mguid_Dialogs = guid;
				else
				{
					UuidCreate(&mguid_Dialogs);
					WritePrivateProfileString(L"Plugin", L"GUID", FormatGuid(&mguid_Dialogs, szTmp), szIni);
				}
			}
			lbRc = TRUE;
		}
	}
	
	if (!lbRc)
	{
		lstrcpyn(ms_Title, szSelf[0] ? szSelf : L"Far3Wrap", ARRAYSIZE(ms_Title));
		lstrcpy(ms_Desc, L"Far2->Far3 plugin wrapper");
		lstrcpy(ms_Author, L"Maximus5");
		lstrcpy(ms_RegRoot, L"Software\\Far Manager\\Plugins");
		UuidCreate(&mguid_Plugin);
		UuidCreate(&mguid_Dialogs);
		//mguid_Plugin = guid_DefPlugin;
		//guid_PluginMenu = ::guid_DefPluginMenu;
		//guid_PluginConfigMenu = ::guid_DefPluginConfigMenu;
		//guid_Dialogs = ::guid_DefDialogs;
	}
}

void WrapPluginInfo::CheckPluginExports(LoaderFunctions3 eFunc)
{
	bool lbPluginExport = false; // Наличие в Far2 плагине экспортов
	bool lbLoaderExport = false; // Наличие экспорта в Loader.dll
	
	
	switch (eFunc)
	{
	case LF3_Analyse:
		lbPluginExport = (AnalyseW!=NULL)||(OpenFilePluginW!=NULL);
		lbLoaderExport = GetProcAddress(mh_Loader, "AnalyseW")!=NULL;
		break;
	case LF3_CustomData:
		lbPluginExport = (GetCustomDataW!=NULL);
		lbLoaderExport = GetProcAddress(mh_Loader, "GetCustomDataW")!=NULL;
		break;
	case LF3_Dialog:
		lbPluginExport = (ProcessDialogEventW!=NULL);
		lbLoaderExport = GetProcAddress(mh_Loader, "ProcessDialogEventW")!=NULL;
		break;
	case LF3_Editor:
		lbPluginExport = (ProcessEditorEventW!=NULL)||(ProcessEditorInputW!=NULL);
		lbLoaderExport = (GetProcAddress(mh_Loader, "ProcessEditorEventW")!=NULL)
					  && (GetProcAddress(mh_Loader, "ProcessEditorInputW")!=NULL);
		break;
	case LF3_FindList:
		lbPluginExport = (SetFindListW!=NULL);
		lbLoaderExport = (GetProcAddress(mh_Loader, "SetFindListW")!=NULL)
					  && (GetProcAddress(mh_Loader, "AnalyseW")!=NULL); // TempPanel!!
		break;
	case LF3_Viewer:
		lbPluginExport = (ProcessViewerEventW!=NULL);
		lbLoaderExport = GetProcAddress(mh_Loader, "ProcessViewerEventW")!=NULL;
		break;
	//case LF3_Window:
	//	lbPluginExport = (
	//		(((ProcessEditorEventW!=NULL)||(ProcessEditorInputW!=NULL)) ? 1 : 0) +
	//		((ProcessViewerEventW!=NULL) ? 1 : 0) + 
	//		((ProcessDialogEventW!=NULL) ? 1 : 0)) > 1;
	//	lbLoaderExport = 
	//		   (GetProcAddress(mh_Loader, "ProcessEditorEventW")!=NULL)
	//		&& (GetProcAddress(mh_Loader, "ProcessEditorInputW")!=NULL)
	//		&& (GetProcAddress(mh_Loader, "ProcessViewerEventW")!=NULL)
	//		&& (GetProcAddress(mh_Loader, "ProcessDialogEventW")!=NULL);
	//	break;
	default:
		// Неизвестный тип
		_ASSERTE(eFunc == LF3_Viewer);
	}
	
	if (lbPluginExport)
		mn_PluginFunctions |= eFunc;
	
	if (lbLoaderExport)
		mn_LoaderFunctions |= eFunc;
}

BOOL WrapPluginInfo::LoadPlugin(BOOL abSilent)
{
	#ifdef _DEBUG
	void *p1 = NULL, *p2 = NULL;
	#endif

	if (!*ms_PluginDll)
	{
		return FALSE;
	}
	
	if (mh_Dll == NULL)
	{
		//wchar_t szOldDir[MAX_PATH] = {0};
		//GetCurrentDirectory(ARRAYSIZE(szOldDir), szOldDir);
		//wchar_t* pszSlash = wcsrchr(PluginDll, L'\\');
		//if (pszSlash)
		//{
		//	*pszSlash = 0;
		//	SetCurrentDirectory(PluginDll);
		//	*pszSlash = L'\\';
		//}
		DWORD dwErr = 0;
		wchar_t szInfo[1024] = {0};

		if (wcschr(ms_PluginDll, L'*'))
			wsprintf(szInfo, L"Far3Wrap\nPlugin module not found: \n%s", ms_PluginDll);
		else
		{
			if (wcschr(ms_PluginDll, L'\\'))
				mh_Dll = LoadLibraryEx(ms_PluginDll, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
			else
				mh_Dll = LoadLibrary(ms_PluginDll);
			dwErr = GetLastError();
		}
		//if (pszSlash)
		//	SetCurrentDirectory(szOldDir);
		if (mh_Dll == NULL)
		{
			//TODO: Обработка ошибок загрузки
			if (szInfo[0] == 0)
			{
				wsprintf(szInfo, L"Far3Wrap\nPlugin loading failed!\n%s\nErrCode=0x%08X\n", ms_PluginDll, dwErr);
				FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS, NULL, dwErr,
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), szInfo+lstrlen(szInfo),
					ARRAYSIZE(szInfo)-lstrlen(szInfo) - 1, NULL);
			}
			if (abSilent)
			{
				if (m_ErrorInfo && m_ErrorInfoMax > 0)
					lstrcpyn(m_ErrorInfo, szInfo, m_ErrorInfoMax);
			}
			else
			{
				MessageBox(NULL, szInfo, L"Far3Wrapper", MB_ICONSTOP|MB_SYSTEMMODAL);
			}
		}
		else
		{
			AnalyseW = (WrapPluginInfo::_AnalyseW)GetProcAddress(mh_Dll, "AnalyseW");
			ClosePluginW = (WrapPluginInfo::_ClosePluginW)GetProcAddress(mh_Dll, "ClosePluginW");
			CompareW = (WrapPluginInfo::_CompareW)GetProcAddress(mh_Dll, "CompareW");
			ConfigureW = (WrapPluginInfo::_ConfigureW)GetProcAddress(mh_Dll, "ConfigureW");
			DeleteFilesW = (WrapPluginInfo::_DeleteFilesW)GetProcAddress(mh_Dll, "DeleteFilesW");
			ExitFARW = (WrapPluginInfo::_ExitFARW)GetProcAddress(mh_Dll, "ExitFARW");
			FreeFindDataW = (WrapPluginInfo::_FreeFindDataW)GetProcAddress(mh_Dll, "FreeFindDataW");
			FreeVirtualFindDataW = (WrapPluginInfo::_FreeVirtualFindDataW)GetProcAddress(mh_Dll, "FreeVirtualFindDataW");
			GetFilesW = (WrapPluginInfo::_GetFilesW)GetProcAddress(mh_Dll, "GetFilesW");
			GetFindDataW = (WrapPluginInfo::_GetFindDataW)GetProcAddress(mh_Dll, "GetFindDataW");
			GetMinFarVersionW = (WrapPluginInfo::_GetMinFarVersionW)GetProcAddress(mh_Dll, "GetMinFarVersionW");
			GetOpenPluginInfoW = (WrapPluginInfo::_GetOpenPluginInfoW)GetProcAddress(mh_Dll, "GetOpenPluginInfoW");
			GetPluginInfoW = (WrapPluginInfo::_GetPluginInfoW)GetProcAddress(mh_Dll, "GetPluginInfoW");
			GetVirtualFindDataW = (WrapPluginInfo::_GetVirtualFindDataW)GetProcAddress(mh_Dll, "GetVirtualFindDataW");
			MakeDirectoryW = (WrapPluginInfo::_MakeDirectoryW)GetProcAddress(mh_Dll, "MakeDirectoryW");
			OpenFilePluginW = (WrapPluginInfo::_OpenFilePluginW)GetProcAddress(mh_Dll, "OpenFilePluginW");
			OpenPluginW = (WrapPluginInfo::_OpenPluginW)GetProcAddress(mh_Dll, "OpenPluginW");
			ProcessDialogEventW = (WrapPluginInfo::_ProcessDialogEventW)GetProcAddress(mh_Dll, "ProcessDialogEventW");
			ProcessEditorEventW = (WrapPluginInfo::_ProcessEditorEventW)GetProcAddress(mh_Dll, "ProcessEditorEventW");
			ProcessEditorInputW = (WrapPluginInfo::_ProcessEditorInputW)GetProcAddress(mh_Dll, "ProcessEditorInputW");
			ProcessEventW = (WrapPluginInfo::_ProcessEventW)GetProcAddress(mh_Dll, "ProcessEventW");
			ProcessHostFileW = (WrapPluginInfo::_ProcessHostFileW)GetProcAddress(mh_Dll, "ProcessHostFileW");
			ProcessKeyW = (WrapPluginInfo::_ProcessKeyW)GetProcAddress(mh_Dll, "ProcessKeyW");
			ProcessSynchroEventW = (WrapPluginInfo::_ProcessSynchroEventW)GetProcAddress(mh_Dll, "ProcessSynchroEventW");
			ProcessViewerEventW = (WrapPluginInfo::_ProcessViewerEventW)GetProcAddress(mh_Dll, "ProcessViewerEventW");
			PutFilesW = (WrapPluginInfo::_PutFilesW)GetProcAddress(mh_Dll, "PutFilesW");
			SetDirectoryW = (WrapPluginInfo::_SetDirectoryW)GetProcAddress(mh_Dll, "SetDirectoryW");
			SetFindListW = (WrapPluginInfo::_SetFindListW)GetProcAddress(mh_Dll, "SetFindListW");
			SetStartupInfoW = (WrapPluginInfo::_SetStartupInfoW)GetProcAddress(mh_Dll, "SetStartupInfoW");
			GetCustomDataW = (WrapPluginInfo::_GetCustomDataW)GetProcAddress(mh_Dll, "GetCustomDataW");
			FreeCustomDataW = (WrapPluginInfo::_FreeCustomDataW)GetProcAddress(mh_Dll, "FreeCustomDataW");

			//TODO: Если экспортируемые функции из mh_Loader не совпадают с mn_OldAbsentFunctions - обновить (сбросить mn_OldAbsentFunctions в 0)
			mn_PluginFunctions = LF3_None;
			mn_LoaderFunctions = LF3_None; // сформируем на основе реальных экспортов в Loader.dll

			CheckPluginExports(LF3_Analyse);
			CheckPluginExports(LF3_CustomData);
			CheckPluginExports(LF3_Dialog);
			CheckPluginExports(LF3_Editor);
			CheckPluginExports(LF3_FindList);
			CheckPluginExports(LF3_Viewer);
			//CheckPluginExports(LF3_Window);
			
			#if 0	
			CheckPluginExports(ALF3_Analyse);
			CheckPluginExports(ALF3_Open);
			CheckPluginExports(ALF3_Configure);
			CheckPluginExports(ALF3_Compare);
			CheckPluginExports(ALF3_GetFiles);
			CheckPluginExports(ALF3_PutFiles);
			CheckPluginExports(ALF3_FindData);
			CheckPluginExports(ALF3_VirtualFindData);
			CheckPluginExports(ALF3_ProcessHostFile);
			CheckPluginExports(ALF3_ProcessDialogEvent);
			CheckPluginExports(ALF3_ProcessEditorEvent);
			CheckPluginExports(ALF3_ProcessEditorInput);
			CheckPluginExports(ALF3_ProcessViewerEvent);
			CheckPluginExports(ALF3_SetFindList);
			CheckPluginExports(ALF3_CustomData);
			#endif

			#if 0
			// Если нужно в загрузчике снести (или восстановить) какие-то экспорты - самое время поставить его в очередь
			int nLoaderResourceId = 0;
			#ifdef _WIN64
			#define MAKE_LOADER_RC(s) IDR_LOADER64_##s
			#else
			#define MAKE_LOADER_RC(s) IDR_LOADER_##s
			#endif
			// !! LF3_FindList должен идти перед LF3_Analyse !!
			if (mn_LoaderFunctions == LF3_FindList)
			{
				if (mn_PluginFunctions != LF3_FindList)
					nLoaderResourceId = MAKE_LOADER_RC(FINDLIST);
			}
			else if (mn_LoaderFunctions == LF3_Analyse)
			{
				if (mn_PluginFunctions != LF3_Analyse)
					nLoaderResourceId = MAKE_LOADER_RC(ARC);
			}
			else if (mn_LoaderFunctions == LF3_CustomData)
			{
				if (mn_PluginFunctions != LF3_CustomData)
					nLoaderResourceId = MAKE_LOADER_RC(CUSTOM);
			}
			else if (mn_LoaderFunctions == LF3_Window)
			{
				if (mn_PluginFunctions != LF3_Window)
					nLoaderResourceId = MAKE_LOADER_RC(WINDOW);
			}
			else if (mn_LoaderFunctions == LF3_Dialog)
			{
				if (mn_PluginFunctions != LF3_Dialog)
					nLoaderResourceId = MAKE_LOADER_RC(DIALOG);
			}
			else if (mn_LoaderFunctions == LF3_Editor)
			{
				if (mn_PluginFunctions != LF3_Editor)
					nLoaderResourceId = MAKE_LOADER_RC(EDITOR);
			}
			else
			{
				_ASSERTE(mn_LoaderFunctions == LF3_Full);
				if ((mn_PluginFunctions & LF3_Full) != LF3_Full)
					nLoaderResourceId = MAKE_LOADER_RC(FULL);
			};
			if (mn_LoaderFunctions != mn_PluginFunctions && nLoaderResourceId == 0)
			{
				_ASSERTE(mn_LoaderFunctions == mn_PluginFunctions);
			}
			if (mn_LoaderFunctions != mn_PluginFunctions && *ms_PluginDll && nLoaderResourceId)
			{
				// Проверим, может уже помещен в список "на модификацию"
				bool lbExist = false;
				std::vector<WrapUpdateFunctions>::iterator iter;
				for (iter = UpdateFunc.begin(); !lbExist && iter != UpdateFunc.end(); iter++)
				{
					if (lstrcmpi(iter->ms_IniFile, ms_IniFile) == 0)
						lbExist = true;
				}

				if (!lbExist)
				{
					WrapUpdateFunctions upd = {mn_PluginFunctions, nLoaderResourceId};
					if (GetModuleFileName(mh_Loader, upd.ms_Loader, ARRAYSIZE(upd.ms_Loader)))
					{
						lstrcpy(upd.ms_IniFile, ms_IniFile);
						UpdateFunc.push_back(upd);
					}
				}
			}
			#endif

			#if 0
			int nIdx = 0;
			ExportFunc strNull[64] = {{NULL}};
			#undef SET_EXP
			#define SET_EXP_(n,s) if (!n) { strNull[nIdx].Name = s; strNull[nIdx].OldAddress = GetProcAddress(mh_Loader, s); nIdx++; }
			#define SET_EXP(n) SET_EXP_(n,#n)
			SET_EXP(ConfigureW);
			SET_EXP_((AnalyseW||OpenFilePluginW),"AnalyseW");
			SET_EXP_(ProcessKeyW, "ProcessPanelInputW");
			SET_EXP(ProcessDialogEventW);
			SET_EXP(ProcessEditorEventW);
			SET_EXP(ProcessViewerEventW);
			SET_EXP(GetFilesW);
			SET_EXP(PutFilesW);
			SET_EXP(DeleteFilesW);
			SET_EXP(GetFindDataW);
			SET_EXP(FreeFindDataW);
			SET_EXP(GetVirtualFindDataW);
			SET_EXP(FreeVirtualFindDataW);
			SET_EXP(SetDirectoryW);
			SET_EXP(SetFindListW);
			SET_EXP(GetCustomDataW);
			SET_EXP(FreeCustomDataW);
			SET_EXP_((OpenPluginW||OpenFilePluginW), "OpenW");
			#undef SET_EXP
			#ifdef _DEBUG
			p1 = GetProcAddress(mh_Loader, "SetFindListW");
			#endif
			ChangeExports(strNull, mh_Loader);
			#ifdef _DEBUG
			p2 = GetProcAddress(mh_Loader, "SetFindListW");
			#endif
			#endif
		}
	}
	
	return (mh_Dll != NULL);
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
	GetCustomDataW = NULL;
	FreeCustomDataW = NULL;
}

void WrapPluginInfo::UnloadPlugin()
{
	if (mh_Dll)
	{
		FreeLibrary(mh_Dll);
		mh_Dll = NULL;
	}
	ClearProcAddr();
}



LPCWSTR WrapPluginInfo::FormatGuid(GUID* guid, wchar_t* tmp, BOOL abQuote /*= FALSE*/)
{
	wsprintf(tmp, 
		abQuote ?
			L"\"%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X\"" :
			L"%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X"
			,
		guid->Data1, guid->Data2, guid->Data3,
		guid->Data4[0], guid->Data4[1], guid->Data4[2], guid->Data4[3],
		guid->Data4[4], guid->Data4[5], guid->Data4[6], guid->Data4[7]);
	return tmp;
}

InfoPanelLine* WrapPluginInfo::InfoLines_2_3(const Far2::InfoPanelLine *InfoLines, int InfoLinesNumber)
{
	if (m_InfoLines)
	{
		free(m_InfoLines);
		m_InfoLines = NULL;
	}
	m_InfoLinesNumber = 0;
	if (InfoLines && InfoLinesNumber > 0)
	{	
		m_InfoLines = (InfoPanelLine*)calloc(InfoLinesNumber, sizeof(*InfoLines));
		for (int i = 0; i < InfoLinesNumber; i++)
		{
			m_InfoLines[i].Text = InfoLines[i].Text;
			m_InfoLines[i].Data = InfoLines[i].Data;
			m_InfoLines[i].Separator = InfoLines[i].Separator;
		}
		m_InfoLinesNumber = InfoLinesNumber;
	}
	return m_InfoLines;
}

PanelMode* WrapPluginInfo::PanelModes_2_3(const Far2::PanelMode *PanelModesArray, int PanelModesNumber)
{
	if (m_PanelModesArray)
	{
		free(m_PanelModesArray);
		m_PanelModesArray = NULL;
	}
	m_PanelModesNumber = 0;
	if (PanelModesArray && PanelModesNumber > 0)
	{
		m_PanelModesArray = (PanelMode*)calloc(PanelModesNumber, sizeof(*PanelModesArray));
		for (int i = 0; i < PanelModesNumber; i++)
		{
			m_PanelModesArray[i].StructSize = sizeof(*PanelModesArray);
			m_PanelModesArray[i].ColumnTypes = PanelModesArray[i].ColumnTypes;
			m_PanelModesArray[i].ColumnWidths = PanelModesArray[i].ColumnWidths;
			m_PanelModesArray[i].ColumnTitles = PanelModesArray[i].ColumnTitles;
			m_PanelModesArray[i].StatusColumnTypes = PanelModesArray[i].StatusColumnTypes;
			m_PanelModesArray[i].StatusColumnWidths = PanelModesArray[i].StatusColumnWidths;
			m_PanelModesArray[i].Flags = 
				(PanelModesArray[i].FullScreen ? PMFLAGS_FULLSCREEN : 0) |
				(PanelModesArray[i].DetailedStatus ? PMFLAGS_DETAILEDSTATUS : 0) |
				(PanelModesArray[i].AlignExtensions ? PMFLAGS_ALIGNEXTENSIONS : 0) |
				(PanelModesArray[i].CaseConversion ? PMFLAGS_CASECONVERSION : 0);
		}
		m_PanelModesNumber = PanelModesNumber;
	}
	return m_PanelModesArray;
}


int WrapPluginInfo::OpMode_3_2(OPERATION_MODES OpMode3)
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

int WrapPluginInfo::OpenFrom_3_2(OPENFROM OpenFrom3)
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

OPENPANELINFO_FLAGS WrapPluginInfo::OpenPanelInfoFlags_2_3(DWORD Flags2)
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

Far2::OPENPLUGININFO_SORTMODES WrapPluginInfo::SortMode_3_2(OPENPANELINFO_SORTMODES Mode3)
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

OPENPANELINFO_SORTMODES WrapPluginInfo::SortMode_2_3(/*Far2::OPENPLUGININFO_SORTMODES*/int Mode2)
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

PLUGINPANELITEMFLAGS WrapPluginInfo::PluginPanelItemFlags_2_3(DWORD Flags2)
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

DWORD WrapPluginInfo::PluginPanelItemFlags_3_2(PLUGINPANELITEMFLAGS Flags3)
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

void WrapPluginInfo::PluginPanelItem_2_3(const Far2::PluginPanelItem* p2, PluginPanelItem* p3)
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

PluginPanelItem* WrapPluginInfo::PluginPanelItems_2_3(const Far2::PluginPanelItem* pItems, int ItemsNumber)
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

void WrapPluginInfo::PluginPanelItem_3_2(const PluginPanelItem* p3, Far2::PluginPanelItem* p2)
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

void WrapPluginInfo::PluginPanelItem_3_2(const PluginPanelItem *p3, Far2::FAR_FIND_DATA* p2)
{
	p2->dwFileAttributes = p3->FileAttributes;
	p2->ftCreationTime = p3->CreationTime;
	p2->ftLastAccessTime = p3->LastAccessTime;
	p2->ftLastWriteTime = p3->LastWriteTime;
	p2->nFileSize = p3->FileSize;
	p2->nPackSize = p3->PackSize;
	p2->lpwszFileName = p3->FileName;
	p2->lpwszAlternateFileName = p3->AlternateFileName;
}

void WrapPluginInfo::PluginPanelItem_2_3(const Far2::FAR_FIND_DATA* p2, PluginPanelItem *p3)
{
	memset(p3, 0, sizeof(*p3));
	p3->FileAttributes = p2->dwFileAttributes;
	p3->CreationTime = p2->ftCreationTime;
	p3->LastAccessTime = p2->ftLastAccessTime;
	p3->LastWriteTime = p2->ftLastWriteTime;
	p3->ChangeTime = p2->ftLastWriteTime;
	p3->FileSize = p2->nFileSize;
	p3->PackSize = p2->nPackSize;
	p3->FileName = p2->lpwszFileName;
	p3->AlternateFileName = p2->lpwszAlternateFileName;
}

Far2::PluginPanelItem* WrapPluginInfo::PluginPanelItems_3_2(const PluginPanelItem* pItems, int ItemsNumber)
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

void WrapPluginInfo::FarKey_2_3(int Key2, INPUT_RECORD *r)
{
	memset(r, 0, sizeof(INPUT_RECORD));
	FSF3.FarKeyToInputRecord(Key2, r);
}

DWORD WrapPluginInfo::FarKey_3_2(const INPUT_RECORD *Rec)
{
	DWORD Key2 = 0;

#if 1
	// В Far3 build 2026 функу подправили
	Key2 = FSF3.FarInputRecordToKey(Rec);

#else

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
			//TODO: У фара (2018) срывает крышу: http://bugs.farmanager.com/view.php?id=1760
			//_ASSERTE(FALSE);
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
	//DWORD FShift = Key2 & 0x7F000000; // старший бит используется в других целях!
	//ControlState =
	//	(FShift & KEY_SHIFT ? Far2::PKF_SHIFT : 0)|
	//	(FShift & KEY_ALT ? Far2::PKF_ALT : 0)|
	//	(FShift & KEY_CTRL ? Far2::PKF_CONTROL : 0);
#endif

	//TODO: Маскировать нужно?
	return Key2;
}

FARDIALOGITEMTYPES WrapPluginInfo::DialogItemTypes_2_3(int ItemType2)
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

int WrapPluginInfo::DialogItemTypes_3_2(FARDIALOGITEMTYPES ItemType3)
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

DWORD WrapPluginInfo::FarDialogItemFlags_3_2(FarDialogItemFlags Flags3)
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

FarDialogItemFlags WrapPluginInfo::FarDialogItemFlags_2_3(DWORD Flags2)
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

void WrapPluginInfo::FarListItem_2_3(const Far2::FarListItem* p2, FarListItem* p3)
{
	//TODO: конвертация флагов
	p3->Flags = p2->Flags;
	p3->Text = p2->Text;
	p3->Reserved[0] = p2->Reserved[0];
	p3->Reserved[1] = p2->Reserved[1];
	p3->Reserved[2] = p2->Reserved[2];
}

void WrapPluginInfo::FarListItem_3_2(const FarListItem* p3, Far2::FarListItem* p2)
{
	//TODO: конвертация флагов
	p2->Flags = p3->Flags;
	p2->Text = p3->Text;
	p2->Reserved[0] = p3->Reserved[0];
	p2->Reserved[1] = p3->Reserved[1];
	p2->Reserved[2] = p3->Reserved[2];
}

void WrapPluginInfo::FarDialogItem_2_3(const Far2::FarDialogItem *p2, FarDialogItem *p3, FarList *pList3)
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

void WrapPluginInfo::FarDialogItem_3_2(const FarDialogItem *p3, /*size_t nAllocated3,*/ Far2::FarDialogItem *p2, Far2::FarList *pList2)
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

	//#define CpyDlgStr(dst,src) \
	//if (!nAllocated3 || ((LPBYTE)p3->src) < ((LPBYTE)p3) || ((LPBYTE)p3->src) >= (((LPBYTE)p3)+nAllocated3))
	//	p2->dst = p3->src;
	//else
	//{
	//	p2->dst = (wchar_t*)(((LPBYTE)p2) + (((LPBYTE)p3->src) - ((LPBYTE)p3)));
	//	lstrcpy((wchar_t*)p2->dst, p3->src);
	//}

	p2->PtrData = p3->Data;
	//CpyDlgStr(PtrData,Data);
	p2->MaxLen = p3->MaxLength;

	if (p3->Type == DI_EDIT)
	{
		p2->Param.History = p3->History;
		//CpyDlgStr(Param.History, History);
	}
	else if (p3->Type == DI_FIXEDIT)
	{
		p2->Param.Mask = p3->Mask;
		//CpyDlgStr(Param.Mask, Mask);
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


//int gnMsg_2 = 0, gnParam1_2 = 0, gnParam1_3 = 0;
//FARMESSAGE gnMsg_3 = DM_FIRST;
//FARMESSAGE gnMsgKey_3 = DM_FIRST, gnMsgClose_3 = DM_FIRST;
//LONG_PTR gnParam2_2 = 0;
//void* gpParam2_3 = NULL;
//FarListItem* gpListItems3 = NULL; INT_PTR gnListItemsMax3 = 0;
//Far2::FarListItem* gpListItems2 = NULL; UINT_PTR gnListItemsMax2 = 0;

LONG_PTR WrapPluginInfo::CallDlgProc_2_3(FARAPIDEFDLGPROC DlgProc3, HANDLE hDlg2, const int Msg2, const int Param1, LONG_PTR Param2)
{
	if (!hDlg2)
	{
		_ASSERTE(hDlg2!=NULL);
		return 0;
	}
	LONG_PTR lRc = 0;
	HANDLE hDlg3 = MapDlg_2_3[(Far2Dialog*)hDlg2];
	if (!hDlg3) // Может быть NULL, если это диалог НЕ из этого плагина
	{
		hDlg3 = hDlg2;
	}

	FARMESSAGE Msg3 = DM_FIRST;
#ifdef _DEBUG
	Far2::FarMessagesProc Msg2_ = (Far2::FarMessagesProc)Msg2;
#endif
	LONG_PTR OrgParam2 = Param2;

	INPUT_RECORD r;
	FarListGetItem flgi3;
	FarListPos flp3;
	FarListDelete fld3;
	FarList fl3;
	FarListUpdate flu3;
	FarListInsert fli3;
	FarListFind flf3;
	FarListInfo flInfo3;
	FarListItemData flItemData3;
	FarListTitles flTitles3;
	FarListColors flColors3;
	FarGetDialogItem fgdi3;
	FarDialogItem fdi3;
	DialogInfo di3;

	//TODO: Сохранять gnMsg_2/gnMsg_3 только если они <DM_USER!
	if (Msg2 == gnMsg_2 && gnMsg_3 != DM_FIRST
		&& gnParam1_2 == Param1 && gnParam2_2 == Param2)
	{
		Msg3 = gnMsg_3;
		Param2 = (LONG_PTR)gpParam2_3;
	}
	else if (Msg2 >= Far2::DM_USER)
	{
		Msg3 = (FARMESSAGE)Msg2;
	}
	else switch (Msg2)
	{
		//TODO: Скорее всего потребуется переработка аргументов для некоторых сообщений
		case Far2::DM_FIRST:
			_ASSERTE(Msg2!=Far2::DM_FIRST); // это сообщение по идее пересылаться не должно
			Msg3 = DM_FIRST; break;
		case Far2::DM_CLOSE:
			Msg3 = (gnMsgClose_3!=DM_FIRST) ? gnMsgClose_3 : DM_CLOSE;
			_ASSERTE(Msg3 == DM_CLOSE || Msg3 == DN_CLOSE);
			break;
		case Far2::DM_ENABLE:
			Msg3 = DM_ENABLE; break;
		case Far2::DM_ENABLEREDRAW:
			Msg3 = DM_ENABLEREDRAW; break;
		case Far2::DM_GETDLGDATA:
			Msg3 = DM_GETDLGDATA; break;
		case Far2::DM_GETDLGRECT:
			Msg3 = DM_GETDLGRECT; break;
		case Far2::DM_GETTEXT:
			Msg3 = DM_GETTEXT; break;
		case Far2::DM_GETTEXTLENGTH:
			Msg3 = DM_GETTEXTLENGTH; break;
		case Far2::DM_KEY:
			//TODO: Аргументы?
			//Msg3 = (gnMsg_3 == DN_KEY) ? DN_KEY : DM_KEY;
			{
				//static INPUT_RECORD r;
				ZeroStruct(r);
				FarKey_2_3((int)Param2, &r);
				Param2 = (LONG_PTR)&r;
				Msg3 = (gnMsgKey_3 != DM_FIRST) ? gnMsgKey_3 : DM_KEY;
				_ASSERTE(Msg3 == DM_KEY || Msg3 == DN_CONTROLINPUT);
			}
			break;
		case Far2::DM_MOVEDIALOG:
			Msg3 = DM_MOVEDIALOG; break;
		case Far2::DM_SETDLGDATA:
			Msg3 = DM_SETDLGDATA; break;
		case Far2::DM_SETFOCUS:
			Msg3 = DM_SETFOCUS; break;
		case Far2::DM_REDRAW:
			Msg3 = DM_REDRAW; break;
		//DM_SETREDRAW=DM_REDRAW,
		case Far2::DM_SETTEXT:
			Msg3 = DM_SETTEXT; break;
		case Far2::DM_SETMAXTEXTLENGTH:
			Msg3 = DM_SETMAXTEXTLENGTH; break;
		//DM_SETTEXTLENGTH=DM_SETMAXTEXTLENGTH,
		case Far2::DM_SHOWDIALOG:
			Msg3 = DM_SHOWDIALOG; break;
		case Far2::DM_GETFOCUS:
			Msg3 = DM_GETFOCUS; break;
		case Far2::DM_GETCURSORPOS:
			Msg3 = DM_GETCURSORPOS; break;
		case Far2::DM_SETCURSORPOS:
			Msg3 = DM_SETCURSORPOS; break;
		case Far2::DM_GETTEXTPTR:
			Msg3 = DM_GETTEXTPTR; break;
		case Far2::DM_SETTEXTPTR:
			Msg3 = DM_SETTEXTPTR; break;
		case Far2::DM_SHOWITEM:
			Msg3 = DM_SHOWITEM; break;
		case Far2::DM_ADDHISTORY:
			Msg3 = DM_ADDHISTORY; break;
		case Far2::DM_GETCHECK:
			Msg3 = DM_GETCHECK; break;
		case Far2::DM_SETCHECK:
			Msg3 = DM_SETCHECK; break;
		case Far2::DM_SET3STATE:
			Msg3 = DM_SET3STATE; break;
		case Far2::DM_LISTSORT:
			Msg3 = DM_LISTSORT; break;
		case Far2::DM_LISTGETITEM:
			if (!Param2)
			{
				_ASSERTE(Param2!=NULL);
			}
			else
			{
				const Far2::FarListGetItem* p2 = (const Far2::FarListGetItem*)Param2;
				//static FarListGetItem flgi3;
				ZeroStruct(flgi3);
				flgi3.ItemIndex = p2->ItemIndex;
				FarListItem_2_3(&p2->Item, &flgi3.Item);
				Param2 = (LONG_PTR)&flgi3;
				Msg3 = DM_LISTGETITEM;
			}
			break;
		case Far2::DM_LISTGETCURPOS: //Msg3 = DM_LISTGETCURPOS; break;
		case Far2::DM_LISTSETCURPOS: //Msg3 = DM_LISTSETCURPOS; break;
			if (!Param2)
			{
				if (Msg2 == Far2::DM_LISTGETCURPOS)
				{
					Msg3 = DM_LISTGETCURPOS;
				}
				else
				{
					_ASSERTE(Param2!=NULL);
				}
			}
			else
			{
				const Far2::FarListPos* p2 = (const Far2::FarListPos*)Param2;
				//static FarListPos flp3;
				ZeroStruct(flp3);
				flp3.SelectPos = p2->SelectPos;
				flp3.TopPos = p2->TopPos;
				Param2 = (LONG_PTR)&flp3;
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
				//_ASSERTE(Param2!=NULL); -- допустимо. удаление всех элементов
				Msg3 = DM_LISTDELETE;
			}
			else
			{
				const Far2::FarListDelete* p2 = (const Far2::FarListDelete*)Param2;
				//static FarListDelete fld3;
				ZeroStruct(fld3);
				fld3.StartIndex = p2->StartIndex;
				fld3.Count = p2->Count;
				Param2 = (LONG_PTR)&fld3;
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
				//static FarList fl3;
				ZeroStruct(fl3);
				fl3.ItemsNumber = p2->ItemsNumber;
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
					fl3.Items = gpListItems3;
					Param2 = (LONG_PTR)&fl3;
					switch (Msg2)
					{
					case Far2::DM_LISTADD: Msg3 = DM_LISTADD; break;
					case Far2::DM_LISTSET: Msg3 = DM_LISTSET; break;
					}
				}
			}
			break;
		case Far2::DM_LISTADDSTR:
			Msg3 = DM_LISTADDSTR; break;
		case Far2::DM_LISTUPDATE:
			if (!Param2)
			{
				_ASSERTE(Param2!=NULL);
			}
			else
			{
				const Far2::FarListUpdate* p2 = (const Far2::FarListUpdate*)Param2;
				//static FarListUpdate flu3;
				ZeroStruct(flu3);
				flu3.Index = p2->Index;
				FarListItem_2_3(&p2->Item, &flu3.Item);
				Param2 = (LONG_PTR)&flu3;
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
				//static FarListInsert fli3;
				ZeroStruct(fli3);
				fli3.Index = p2->Index;
				FarListItem_2_3(&p2->Item, &fli3.Item);
				Param2 = (LONG_PTR)&fli3;
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
				//static FarListFind flf3;
				ZeroStruct(flf3);
				flf3.StartIndex = p2->StartIndex;
				flf3.Pattern = p2->Pattern;
				flf3.Flags = p2->Flags;
				flf3.Reserved = p2->Reserved;
				Param2 = (LONG_PTR)&flf3;
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
				//static FarListInfo flInfo3;
				ZeroStruct(flInfo3);
				flInfo3.Flags = p2->Flags;
				flInfo3.ItemsNumber = p2->ItemsNumber;
				flInfo3.SelectPos = p2->SelectPos;
				flInfo3.TopPos = p2->TopPos;
				flInfo3.MaxHeight = p2->MaxHeight;
				flInfo3.MaxLength = p2->MaxLength;
				flInfo3.Reserved[0] = p2->Reserved[0];
				flInfo3.Reserved[1] = p2->Reserved[1];
				flInfo3.Reserved[2] = p2->Reserved[2];
				flInfo3.Reserved[3] = p2->Reserved[3];
				flInfo3.Reserved[4] = p2->Reserved[4];
				flInfo3.Reserved[5] = p2->Reserved[5];
				Param2 = (LONG_PTR)&flInfo3;
				Msg3 = DM_LISTINFO;
			}
			break;
		case Far2::DM_LISTGETDATA:
			Msg3 = DM_LISTGETDATA; break;
		case Far2::DM_LISTSETDATA:
			if (!Param2)
			{
				_ASSERTE(Param2!=NULL);
			}
			else
			{
				const Far2::FarListItemData* p2 = (const Far2::FarListItemData*)Param2;
				//static FarListItemData flItemData3;
				ZeroStruct(flItemData3);
				flItemData3.Index = p2->Index;
				flItemData3.DataSize = p2->DataSize;
				flItemData3.Data = p2->Data;
				flItemData3.Reserved = p2->Reserved;
				Param2 = (LONG_PTR)&flItemData3;
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
				//static FarListTitles flTitles3;
				ZeroStruct(flTitles3);
				flTitles3.TitleLen = p2->TitleLen;
				flTitles3.Title = p2->Title;
				flTitles3.BottomLen = p2->BottomLen;
				flTitles3.Bottom = p2->Bottom;
				Param2 = (LONG_PTR)&flTitles3;
				switch (Msg2)
				{
				case Far2::DM_LISTSETTITLES: Msg3 = DM_LISTSETTITLES; break;
				case Far2::DM_LISTGETTITLES: Msg3 = DM_LISTGETTITLES; break;
				}
			}
			break;
		case Far2::DM_RESIZEDIALOG:
			Msg3 = DM_RESIZEDIALOG; break;
		case Far2::DM_SETITEMPOSITION:
			Msg3 = DM_SETITEMPOSITION; break;
		case Far2::DM_GETDROPDOWNOPENED:
			Msg3 = DM_GETDROPDOWNOPENED; break;
		case Far2::DM_SETDROPDOWNOPENED:
			Msg3 = DM_SETDROPDOWNOPENED; break;
		case Far2::DM_SETHISTORY:
			Msg3 = DM_SETHISTORY; break;
		case Far2::DM_GETITEMPOSITION:
			Msg3 = DM_GETITEMPOSITION; break;
		case Far2::DM_SETMOUSEEVENTNOTIFY:
			Msg3 = DM_SETMOUSEEVENTNOTIFY; break;
		case Far2::DM_EDITUNCHANGEDFLAG:
			Msg3 = DM_EDITUNCHANGEDFLAG; break;
		case Far2::DM_GETITEMDATA:
			Msg3 = DM_GETITEMDATA; break;
		case Far2::DM_SETITEMDATA:
			Msg3 = DM_SETITEMDATA; break;
		case Far2::DM_LISTSETMOUSEREACTION:
			Msg3 = DM_LISTSETMOUSEREACTION; break;
		case Far2::DM_GETCURSORSIZE:
			Msg3 = DM_GETCURSORSIZE; break;
		case Far2::DM_SETCURSORSIZE:
			Msg3 = DM_SETCURSORSIZE; break;
		case Far2::DM_LISTGETDATASIZE:
			Msg3 = DM_LISTGETDATASIZE; break;
		case Far2::DM_GETSELECTION:
			ASSERTSTRUCT(EditorSelect);
			Msg3 = DM_GETSELECTION;
			break;
		case Far2::DM_SETSELECTION:
			ASSERTSTRUCT(EditorSelect);
			Msg3 = DM_SETSELECTION;
			break;
		case Far2::DM_GETEDITPOSITION:
			ASSERTSTRUCT(EditorSetPosition);
			Msg3 = DM_GETEDITPOSITION;
			break;
		case Far2::DM_SETEDITPOSITION:
			ASSERTSTRUCT(EditorSetPosition);
			Msg3 = DM_SETEDITPOSITION;
			break;
		case Far2::DM_SETCOMBOBOXEVENT:
			Msg3 = DM_SETCOMBOBOXEVENT; break;
		case Far2::DM_GETCOMBOBOXEVENT:
			Msg3 = DM_GETCOMBOBOXEVENT; break;
		case Far2::DM_GETCONSTTEXTPTR:
			Msg3 = DM_GETCONSTTEXTPTR; break;
		case Far2::DM_GETDIALOGINFO:
			ZeroStruct(di3);
			di3.StructSize = sizeof(di3);
			Param2 = (LONG_PTR)&di3;
			Msg3 = DM_GETDIALOGINFO;
			break;
		case Far2::DN_FIRST:
			Msg3 = DN_FIRST; break;
		case Far2::DN_BTNCLICK:
			Msg3 = DN_BTNCLICK; break;
		case Far2::DN_CTLCOLORDIALOG:
			Msg3 = DN_CTLCOLORDIALOG; break;
		case Far2::DN_CTLCOLORDLGITEM:
			Msg3 = DN_CTLCOLORDLGITEM; break;

		case Far2::DN_CTLCOLORDLGLIST:
			if (!Param2)
			{
				_ASSERTE(Param2!=0);
			}
			else
			{
				const Far2::FarListColors* p2 = (const Far2::FarListColors*)Param2;
				//static FarListColors flColors3;
				ZeroStruct(flColors3);
				//TODO: Конвертация флагов?
				flColors3.Flags = p2->Flags;
				flColors3.Reserved = p2->Reserved;
				flColors3.ColorCount = p2->ColorCount;
				flColors3.Colors = p2->Colors;
				Param2 = (LONG_PTR)&flColors3;
				Msg3 = DN_CTLCOLORDLGLIST;
			}
			break;

		case Far2::DN_DRAWDIALOG:
			Msg3 = DN_DRAWDIALOG; break;

		case Far2::DM_GETDLGITEM:
			if (Param2)
			{
				// Тут мы просто получим размер, реально память выделяется в FarMessageParam_3_2
				//static FarGetDialogItem fgdi3;
				ZeroStruct(fgdi3);
				Param2 = (LONG_PTR)&fgdi3;
			}
			Msg3 = DM_GETDLGITEM;
			break;
		case Far2::DM_GETDLGITEMSHORT:
			if (Param2)
			{
				//static FarDialogItem fdi3;
				ZeroStruct(fdi3);
				Param2 = (LONG_PTR)&fdi3;
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
				//static FarDialogItem p3;
				ZeroStruct(fdi3);
				FarDialogItem_2_3(p2, &fdi3, &m_ListItems3);
				Param2 = (LONG_PTR)&fdi3;
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

		case Far2::DN_ENTERIDLE:
			Msg3 = DN_ENTERIDLE; break;
		case Far2::DN_GOTFOCUS:
			Msg3 = DN_GOTFOCUS; break;
		case Far2::DN_HELP:
			Msg3 = DN_HELP; break;
		case Far2::DN_HOTKEY:
			Msg3 = DN_HOTKEY; break;
		case Far2::DN_INITDIALOG:
			Msg3 = DN_INITDIALOG; break;
		case Far2::DN_KILLFOCUS:
			Msg3 = DN_KILLFOCUS; break;
		case Far2::DN_LISTCHANGE:
			Msg3 = DN_LISTCHANGE; break;
		case Far2::DN_MOUSECLICK:
			// DN_KEY и DN_MOUSECLICK объеденены в DN_CONTROLINPUT. 
			// Param2 указывает на INPUT_RECORD. в будущем планируется приход и других событий.
			_ASSERTE(Msg2 != Far2::DN_MOUSECLICK);
			Msg3 = DN_CONTROLINPUT;
			Msg3 = DM_FIRST;
			break;
		case Far2::DN_DRAGGED:
			Msg3 = DN_DRAGGED; break;
		case Far2::DN_RESIZECONSOLE:
			Msg3 = DN_RESIZECONSOLE; break;
		case Far2::DN_MOUSEEVENT:
			// DN_MOUSEEVENT переименовано в DN_INPUT. Param2 указывает на INPUT_RECORD. 
			// в будущем планируется приход не только мышиных событий, поэтому настоятельно 
			// рекомендуется проверять EventType.
			_ASSERTE(Msg2 != Far2::DN_MOUSEEVENT);
			Msg3 = DN_INPUT;
			Msg3 = DM_FIRST;
			break;
		case Far2::DN_DRAWDIALOGDONE:
			Msg3 = DN_DRAWDIALOGDONE; break;
		case Far2::DN_LISTHOTKEY:
			Msg3 = DN_LISTHOTKEY; break;
		//DN_GETDIALOGINFO=DM_GETDIALOGINFO, -- отсутствует в Far3
		//DN_CLOSE=DM_CLOSE,
		//DN_KEY=DM_KEY, // 3. DM_KEY больше не равна DN_KEY.
		default:
			// Некоторые внутренние события Фар не описаны в Plugin.hpp
			_ASSERTE(Msg2==(DN_FIRST-1) || Msg2==(DN_FIRST-2) || Msg2==(DM_USER-1));
			Msg3 = (FARMESSAGE)Msg2;
	}

	// Собственно вызов
	if (Msg3 == DM_FIRST && Msg2 != Far2::DM_FIRST)
	{
		_ASSERTE(Msg3!=DM_FIRST || Msg2==Far2::DM_FIRST);
		lRc = 0;
	}
	else
	{
		lRc = DlgProc3(hDlg3, Msg3, Param1, (void*)Param2);
		if (Param2 && OrgParam2 && Param2 != OrgParam2)
		{
			//FarMessageParam_3_2(hDlg3, Msg, Param1, Param2, OrgParam2, lRc);
			switch (Msg3)
			{
			case DM_GETDIALOGINFO:
				if (lRc > 0)
				{
					Far2::DialogInfo* p2 = (Far2::DialogInfo*)OrgParam2;
					if (p2 && p2->StructSize >= sizeof(Far2::DialogInfo))
					{
						p2->Id = di3.Id;
					}
				}
				break;
			case DM_GETDLGITEM:
				if (lRc > 0)
				{
					Far2::FarDialogItem* p2 = (Far2::FarDialogItem*)OrgParam2;
					//_ASSERTE(sizeof(Far2::FarDialogItem)>=sizeof(FarDialogItem));
					//FarGetDialogItem item = {lRc};
					if (!m_GetDlgItem.Item || ((INT_PTR)m_GetDlgItem.Size < lRc))
					{
						if (m_GetDlgItem.Item)
							free(m_GetDlgItem.Item);
						m_GetDlgItem.Size = lRc;
						m_GetDlgItem.Item = (FarDialogItem*)calloc(lRc, 1);
					}
					lRc = psi3.SendDlgMessage(hDlg3, DM_GETDLGITEM, Param1, &m_GetDlgItem);
					if (lRc > 0)
					{
						FarDialogItem_3_2(m_GetDlgItem.Item, /*m_GetDlgItem.Size,*/ p2, &m_ListItems2);
					}
					//free(item.Item);
				}
				break;
			case DM_GETDLGITEMSHORT:
				{
					const FarDialogItem* p3 = (const FarDialogItem*)Param2;
					Far2::FarDialogItem* p2 = (Far2::FarDialogItem*)OrgParam2;
					FarDialogItem_3_2(p3, /*0,*/ p2, &m_ListItems2);
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
		}
	}

	return lRc;
}

Far2::FarMessagesProc WrapPluginInfo::FarMessage_3_2(const int Msg3, const int Param1, void*& Param2)
{
	Far2::FarMessagesProc Msg2 = Far2::DM_FIRST;
	//if (Msg3 < DM_USER)
	//{
	//	gnMsg_3 = (FARMESSAGE)Msg3;
	//	gnParam1_2 = gnParam1_3 = Param1;
	//	gpParam2_3 = Param2;
	//}

	if (Msg3 >= DM_USER)
	{
		Msg2 = (Far2::FarMessagesProc)Msg3;
	}
	else switch (Msg3)
	{
		//TODO: Скорее всего потребуется переработка аргументов для некоторых сообщений
		case DM_KEY:
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
					ZeroStruct(mer);
					mer = p->Event.MouseEvent;
					Param2 = &mer;
					if (!mer.dwEventFlags)
						Msg2 = Far2::DN_MOUSECLICK;
					else
						Msg2 = Far2::DN_MOUSEEVENT;
					_ASSERTE(Msg3!=DM_KEY);
				}
				else if (p->EventType == KEY_EVENT)
				{
					Param2 = (void*)FarKey_3_2(p);
					Msg2 = (Msg3 == DM_KEY) ? Far2::DM_KEY : Far2::DN_KEY;
				}
				else
					Msg2 = Far2::DM_FIRST;
			}
			break;

		case DM_FIRST:
			Msg2 = Far2::DM_FIRST; break;
		case DM_CLOSE:
			Msg2 = Far2::DM_CLOSE; break;
		case DM_ENABLE:
			Msg2 = Far2::DM_ENABLE; break;
		case DM_ENABLEREDRAW:
			Msg2 = Far2::DM_ENABLEREDRAW; break;
		case DM_GETDLGDATA:
			Msg2 = Far2::DM_GETDLGDATA; break;
		case DM_GETDLGRECT:
			Msg2 = Far2::DM_GETDLGRECT; break;
		case DM_GETTEXT:
			Msg2 = Far2::DM_GETTEXT; break;
		case DM_GETTEXTLENGTH:
			Msg2 = Far2::DM_GETTEXTLENGTH; break;
		//case DM_KEY:
		//	//TODO: Аргументы?
		//	_ASSERTE(Msg3!=DM_KEY);
		//	Msg2 = Far2::DM_KEY;
		//	break;
		case DM_MOVEDIALOG:
			Msg2 = Far2::DM_MOVEDIALOG; break;
		case DM_SETDLGDATA:
			Msg2 = Far2::DM_SETDLGDATA; break;
		case DM_SETFOCUS:
			Msg2 = Far2::DM_SETFOCUS; break;
		case DM_REDRAW:
			Msg2 = Far2::DM_REDRAW; break;
		case DM_SETTEXT:
			Msg2 = Far2::DM_SETTEXT; break;
		case DM_SETMAXTEXTLENGTH:
			Msg2 = Far2::DM_SETMAXTEXTLENGTH; break;
		case DM_SHOWDIALOG:
			Msg2 = Far2::DM_SHOWDIALOG; break;
		case DM_GETFOCUS:
			Msg2 = Far2::DM_GETFOCUS; break;
		case DM_GETCURSORPOS:
			Msg2 = Far2::DM_GETCURSORPOS; break;
		case DM_SETCURSORPOS:
			Msg2 = Far2::DM_SETCURSORPOS; break;
		case DM_GETTEXTPTR:
			Msg2 = Far2::DM_GETTEXTPTR; break;
		case DM_SETTEXTPTR:
			Msg2 = Far2::DM_SETTEXTPTR; break;
		case DM_SHOWITEM:
			Msg2 = Far2::DM_SHOWITEM; break;
		case DM_ADDHISTORY:
			Msg2 = Far2::DM_ADDHISTORY; break;
		case DM_GETCHECK:
			Msg2 = Far2::DM_GETCHECK; break;
		case DM_SETCHECK:
			Msg2 = Far2::DM_SETCHECK; break;
		case DM_SET3STATE:
			Msg2 = Far2::DM_SET3STATE; break;
		case DM_LISTSORT:
			Msg2 = Far2::DM_LISTSORT; break;
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
				case DM_LISTGETCURPOS:
					Msg2 = Far2::DM_LISTGETCURPOS; break;
				case DM_LISTSETCURPOS:
					Msg2 = Far2::DM_LISTSETCURPOS; break;
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
					case DM_LISTADD:
						Msg2 = Far2::DM_LISTADD; break;
					case DM_LISTSET:
						Msg2 = Far2::DM_LISTSET; break;
					}
				}
			}
			break;
		case DM_LISTADDSTR:
			Msg2 = Far2::DM_LISTADDSTR; break;
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
		case DM_LISTGETDATA:
			Msg2 = Far2::DM_LISTGETDATA; break;
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
				case DM_LISTSETTITLES:
					Msg2 = Far2::DM_LISTSETTITLES; break;
				case DM_LISTGETTITLES:
					Msg2 = Far2::DM_LISTGETTITLES; break;
				}
			}
			break;
		case DM_RESIZEDIALOG:
			Msg2 = Far2::DM_RESIZEDIALOG; break;
		case DM_SETITEMPOSITION:
			Msg2 = Far2::DM_SETITEMPOSITION; break;
		case DM_GETDROPDOWNOPENED:
			Msg2 = Far2::DM_GETDROPDOWNOPENED; break;
		case DM_SETDROPDOWNOPENED:
			Msg2 = Far2::DM_SETDROPDOWNOPENED; break;
		case DM_SETHISTORY:
			Msg2 = Far2::DM_SETHISTORY; break;
		case DM_GETITEMPOSITION:
			Msg2 = Far2::DM_GETITEMPOSITION; break;
		case DM_SETMOUSEEVENTNOTIFY:
			Msg2 = Far2::DM_SETMOUSEEVENTNOTIFY; break;
		case DM_EDITUNCHANGEDFLAG:
			Msg2 = Far2::DM_EDITUNCHANGEDFLAG; break;
		case DM_GETITEMDATA:
			Msg2 = Far2::DM_GETITEMDATA; break;
		case DM_SETITEMDATA:
			Msg2 = Far2::DM_SETITEMDATA; break;
		case DM_LISTSETMOUSEREACTION:
			Msg2 = Far2::DM_LISTSETMOUSEREACTION; break;
		case DM_GETCURSORSIZE:
			Msg2 = Far2::DM_GETCURSORSIZE; break;
		case DM_SETCURSORSIZE:
			Msg2 = Far2::DM_SETCURSORSIZE; break;
		case DM_LISTGETDATASIZE:
			Msg2 = Far2::DM_LISTGETDATASIZE; break;
		case DM_GETSELECTION:
			ASSERTSTRUCT(EditorSelect);
			Msg2 = Far2::DM_GETSELECTION;
			break;
		case DM_SETSELECTION:
			ASSERTSTRUCT(EditorSelect);
			Msg2 = Far2::DM_SETSELECTION;
			break;
		case DM_GETEDITPOSITION:
			ASSERTSTRUCT(EditorSetPosition);
			Msg2 = Far2::DM_GETEDITPOSITION;
			break;
		case DM_SETEDITPOSITION:
			ASSERTSTRUCT(EditorSetPosition);
			Msg2 = Far2::DM_SETEDITPOSITION;
			break;
		case DM_SETCOMBOBOXEVENT:
			Msg2 = Far2::DM_SETCOMBOBOXEVENT; break;
		case DM_GETCOMBOBOXEVENT:
			Msg2 = Far2::DM_GETCOMBOBOXEVENT; break;
		case DM_GETCONSTTEXTPTR:
			Msg2 = Far2::DM_GETCONSTTEXTPTR; break;
		case DM_GETDIALOGINFO:
			Msg2 = Far2::DM_GETDIALOGINFO; break;
		case DN_FIRST:
			Msg2 = Far2::DN_FIRST; break;
		case DN_BTNCLICK:
			Msg2 = Far2::DN_BTNCLICK; break;
		case DN_CTLCOLORDIALOG:
			Msg2 = Far2::DN_CTLCOLORDIALOG; break;
		case DN_CTLCOLORDLGITEM:
			Msg2 = Far2::DN_CTLCOLORDLGITEM; break;

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
		case DN_DRAWDIALOG:
			Msg2 = Far2::DN_DRAWDIALOG; break;

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
				FarDialogItem_3_2(p3, /*0,*/ &p2, &m_ListItems2);
				Param2 = &p2;
				switch (Msg3)
				{
				//case DM_GETDLGITEM:Msg2 = Far2::DM_GETDLGITEM; break;
				//case DM_GETDLGITEMSHORT: Msg2 = Far2::DM_GETDLGITEMSHORT; break;
				case DM_SETDLGITEM: Msg2 = Far2::DM_SETDLGITEM; break;
				case DM_SETDLGITEMSHORT:
					Msg2 = Far2::DM_SETDLGITEMSHORT; break;
				case DN_DRAWDLGITEM:
					Msg2 = Far2::DN_DRAWDLGITEM; break;
				case DN_EDITCHANGE:
					Msg2 = Far2::DN_EDITCHANGE; break;
				}
			}
			break;
		
		case DN_ENTERIDLE:
			Msg2 = Far2::DN_ENTERIDLE; break;
		case DN_GOTFOCUS:
			Msg2 = Far2::DN_GOTFOCUS; break;
		case DN_HELP:
			Msg2 = Far2::DN_HELP; break;
		case DN_HOTKEY:
			Msg2 = Far2::DN_HOTKEY; break;
		case DN_INITDIALOG:
			Msg2 = Far2::DN_INITDIALOG; break;
		case DN_KILLFOCUS:
			Msg2 = Far2::DN_KILLFOCUS; break;
		case DN_LISTCHANGE:
			Msg2 = Far2::DN_LISTCHANGE; break;
		case DN_DRAGGED:
			Msg2 = Far2::DN_DRAGGED; break;
		case DN_RESIZECONSOLE:
			Msg2 = Far2::DN_RESIZECONSOLE; break;
		case DN_DRAWDIALOGDONE:
			Msg2 = Far2::DN_DRAWDIALOGDONE; break;
		case DN_LISTHOTKEY:
			Msg2 = Far2::DN_LISTHOTKEY; break;
		case DN_CLOSE:
			Msg2 = Far2::DN_CLOSE; break;
		default:
			// Некоторые внутренние события Фар не описаны в Plugin.hpp
			_ASSERTE(Msg3==(DN_FIRST-1) || Msg3==(DN_FIRST-2) || Msg3==(DM_USER-1));
			Msg2 = (Far2::FarMessagesProc)Msg3;
	}

	//gnMsg_3 = DM_FIRST;
	//gnMsg_2 = Msg2;
	//gnParam2_2 = (LONG_PTR)Param2;

	return Msg2;
}

void WrapPluginInfo::FarMessageParam_2_3(const int Msg2, const int Param1, const void* Param2, void* OrgParam2, LONG_PTR lRc)
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
			FarDialogItem_2_3(p2, p3, &m_ListItems3);
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

//void FarMessageParam_3_2(HANDLE hDlg3, const int Msg3, const int Param1, const LONG_PTR Param2, LONG_PTR OrgParam2, LONG_PTR& lRc)
//{
//	if (Param2 == OrgParam2 || !Param2 || !OrgParam2)
//	{
//		_ASSERTE(Param2 != OrgParam2 && Param2 && OrgParam2);
//		return;
//	}
//	switch (Msg3)
//	{
//	case DM_GETDLGITEM:
//		if (lRc > 0)
//		{
//			Far2::FarDialogItem* p2 = (Far2::FarDialogItem*)OrgParam2;
//			FarGetDialogItem item = {lRc};
//			item.Item = (FarDialogItem*)calloc(lRc, 1);
//			lRc = psi3.SendDlgMessage(hDlg3, DM_GETDLGITEM, Param1, &item);
//			if (lRc > 0)
//			{
//				FarDialogItem_3_2(item.Item, p2, &m_ListItems2);
//			}
//			free(item.Item);
//		}
//		break;
//	case DM_GETDLGITEMSHORT:
//		{
//			const FarDialogItem* p3 = (const FarDialogItem*)Param2;
//			Far2::FarDialogItem* p2 = (Far2::FarDialogItem*)OrgParam2;
//			FarDialogItem_3_2(p3, p2, &m_ListItems2);
//		}
//		break;
//	case DM_LISTGETITEM:
//		{
//			const FarListGetItem* p3 = (const FarListGetItem*)Param2;
//			Far2::FarListGetItem* p2 = (Far2::FarListGetItem*)OrgParam2;
//			p2->ItemIndex = p3->ItemIndex;
//			FarListItem_3_2(&p3->Item, &p2->Item);
//		}
//		break;
//	case DM_LISTINFO:
//		{
//			const FarListInfo* p3 = (const FarListInfo*)Param2;
//			Far2::FarListInfo* p2 = (Far2::FarListInfo*)OrgParam2;
//			//TODO: конвертация флагов
//			p2->Flags = p3->Flags;
//			p2->ItemsNumber = p3->ItemsNumber;
//			p2->SelectPos = p3->SelectPos;
//			p2->TopPos = p3->TopPos;
//			p2->MaxHeight = p3->MaxHeight;
//			p2->MaxLength = p3->MaxLength;
//			p2->Reserved[0] = p3->Reserved[0];
//			p2->Reserved[1] = p3->Reserved[1];
//			p2->Reserved[2] = p3->Reserved[2];
//			p2->Reserved[3] = p3->Reserved[3];
//			p2->Reserved[4] = p3->Reserved[4];
//			p2->Reserved[5] = p3->Reserved[5];
//		}
//		break;
//	case DM_LISTGETCURPOS:
//		{
//			const FarListPos* p3 = (const FarListPos*)Param2;
//			Far2::FarListPos* p2 = (Far2::FarListPos*)OrgParam2;
//			p2->SelectPos = p3->SelectPos;
//			p2->TopPos = p3->TopPos;
//		}
//		break;
//	case DM_LISTGETTITLES:
//		{
//			const FarListTitles* p3 = (const FarListTitles*)Param2;
//			Far2::FarListTitles* p2 = (Far2::FarListTitles*)OrgParam2;
//			p2->TitleLen = p3->TitleLen;
//			p2->Title = p3->Title;
//			p2->BottomLen = p3->BottomLen;
//			p2->Bottom = p3->Bottom;
//		}
//		break;
//	}
//};





KeyBarTitles* WrapPluginInfo::KeyBarTitles_2_3(const Far2::KeyBarTitles* KeyBar)
{
	m_KeyBar.CountLabels = 0;

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
		_ASSERTE(cnt<=ARRAYSIZE(m_KeyBarLabels));
		
		m_KeyBar.CountLabels = 0;
		
		if (cnt > 0)
		{
			KeyBarLabel *p = m_KeyBar.Labels;
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
			m_KeyBar.CountLabels = p - m_KeyBar.Labels;
		}
	}
	return &m_KeyBar;
}


// Changed functions
LONG_PTR WrapPluginInfo::FarApiDefDlgProc(HANDLE hDlg, int Msg, int Param1, LONG_PTR Param2)
{
	LOG_CMD(L"psi2.DefDlgProc(%i,%i,%i)",Msg,Param1,Param2);
	LONG_PTR lRc = CallDlgProc_2_3(psi3.DefDlgProc, hDlg, Msg, Param1, Param2);
	return lRc;
	//HANDLE hDlg3 = MapDlg_2_3[(Far2Dialog*)hDlg];
	//if (!hDlg3) // Может быть NULL, если это диалог НЕ из этого плагина
	//	hDlg3 = hDlg;
	//if (hDlg3)
	//{
	//	LONG_PTR OrgParam2 = Param2;
	//	FARMESSAGE Msg3 = FarMessage_2_3(Msg, Param1, Param2);
	//	_ASSERTE(Msg3!=DM_FIRST);
	//	lRc = psi3.DefDlgProc(hDlg3, Msg3, Param1, (void*)Param2);
	//	if (Param2 && Param2 != OrgParam2)
	//		FarMessageParam_3_2(hDlg3, Msg, Param1, Param2, OrgParam2, lRc);
	//}
	//else
	//{
	//	_ASSERTE(hDlg3!=NULL);
	//}
	//return lRc;
}
LONG_PTR WrapPluginInfo::FarApiSendDlgMessage(HANDLE hDlg, int Msg, int Param1, LONG_PTR Param2)
{
	LOG_CMD(L"psi2.SendDlgMessage(%i,%i,%i)",Msg,Param1,Param2);
	LONG_PTR lRc = CallDlgProc_2_3(psi3.SendDlgMessage, hDlg, Msg, Param1, Param2);
	return lRc;
	//HANDLE hDlg3 = MapDlg_2_3[(Far2Dialog*)hDlg];
	//if (!hDlg3) // Может быть NULL, если это диалог НЕ из этого плагина
	//	hDlg3 = hDlg;
	//if (hDlg3)
	//{
	//	LONG_PTR OrgParam2 = Param2;
	//	FARMESSAGE Msg3 = FarMessage_2_3(Msg, Param1, Param2);
	//	_ASSERTE(Msg3!=DM_FIRST);
	//	lRc = psi3.SendDlgMessage(hDlg3, Msg3, Param1, (void*)Param2);
	//	if (Param2 && Param2 != OrgParam2)
	//		FarMessageParam_3_2(hDlg3, Msg, Param1, Param2, OrgParam2, lRc);
	//}
	//else
	//{
	//	_ASSERTE(hDlg3!=NULL);
	//}
	//return lRc;
}
BOOL WrapPluginInfo::FarApiShowHelp(const wchar_t *ModuleName, const wchar_t *Topic, DWORD Flags)
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
HANDLE WrapPluginInfo::FarApiSaveScreen(int X1, int Y1, int X2, int Y2)
{
	LOG_CMD(L"psi2.SaveScreen",0,0,0);
	return psi3.SaveScreen(X1,Y1,X2,Y2);
}
void WrapPluginInfo::FarApiRestoreScreen(HANDLE hScreen)
{
	LOG_CMD(L"psi2.RestoreScreen",0,0,0);
	psi3.RestoreScreen(hScreen);
}
void WrapPluginInfo::FarApiText(int X, int Y, int Color, const wchar_t *Str)
{
	LOG_CMD(L"psi2.Text",0,0,0);
	psi3.Text(X,Y,Color,Str);
}
int WrapPluginInfo::FarApiEditor(const wchar_t *FileName, const wchar_t *Title, int X1, int Y1, int X2, int Y2, DWORD Flags, int StartLine, int StartChar, UINT CodePage)
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
int WrapPluginInfo::FarApiViewer(const wchar_t *FileName, const wchar_t *Title, int X1, int Y1, int X2, int Y2, DWORD Flags, UINT CodePage)
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
int WrapPluginInfo::FarApiMenu(INT_PTR PluginNumber, int X, int Y, int MaxHeight, DWORD Flags, const wchar_t* Title, const wchar_t* Bottom, const wchar_t* HelpTopic, const int* BreakKeys, int* BreakCode, const struct Far2::FarMenuItem *Item, int ItemsNumber)
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
		
		iRc = psi3.Menu(&mguid_Plugin, X, Y, MaxHeight, Flags3,
				Title, Bottom, HelpTopic, pBreak3, BreakCode, pItems3, ItemsNumber);
	}
	
	if (pItems3)
		free(pItems3);
	if (pBreak3)
		free(pBreak3);
	return iRc;
};
int WrapPluginInfo::FarApiMessage(INT_PTR PluginNumber, DWORD Flags, const wchar_t *HelpTopic, const wchar_t * const *Items, int ItemsNumber, int ButtonsNumber)
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
	
	iRc = psi3.Message(&mguid_Plugin, Far3Flags, 
				HelpTopic, Items, ItemsNumber, ButtonsNumber);
	return iRc;
};
HANDLE WrapPluginInfo::FarApiDialogInit(INT_PTR PluginNumber, int X1, int Y1, int X2, int Y2, const wchar_t* HelpTopic, struct Far2::FarDialogItem *Item, unsigned int ItemsNumber, DWORD Reserved, DWORD Flags, Far2::FARWINDOWPROC DlgProc, LONG_PTR Param)
{
	LOG_CMD(L"psi2.DialogInit",0,0,0);
	Far2Dialog *p = new Far2Dialog(this, X1, Y1, X2, Y2,
    	HelpTopic, Item, ItemsNumber, Flags, DlgProc, Param,
    	mguid_Plugin, mguid_Dialogs);
	return (HANDLE)p;
};
int WrapPluginInfo::FarApiDialogRun(HANDLE hDlg)
{
	LOG_CMD(L"psi2.DialogRun",0,0,0);
	Far2Dialog *p = (Far2Dialog*)hDlg;
	if (!p)
		return -1;
	return p->RunDlg();
}
void WrapPluginInfo::FarApiDialogFree(HANDLE hDlg)
{
	LOG_CMD(L"psi2.DialogFree",0,0,0);
	Far2Dialog *p = (Far2Dialog*)hDlg;
	if (!p)
		return;
	delete p;
};
int WrapPluginInfo::FarApiControl(HANDLE hPlugin, int Command, int Param1, LONG_PTR Param2)
{
	LOG_CMD(L"psi2.Control(%i,%i,%i)",Command, Param1, Param2);
	//TODO: Конвертация параметров!
	int iRc = 0;
	switch (Command)
	{
	case Far2::FCTL_CLOSEPLUGIN:
		iRc = psi3.PanelControl(hPlugin, FCTL_CLOSEPANEL, Param1, (void*)Param2); break;
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
	case Far2::FCTL_UPDATEPANEL:
		iRc = psi3.PanelControl(hPlugin, FCTL_UPDATEPANEL, Param1, (void*)Param2); break;
	case Far2::FCTL_REDRAWPANEL:
		iRc = psi3.PanelControl(hPlugin, FCTL_REDRAWPANEL, Param1, (void*)Param2); break;
	case Far2::FCTL_GETCMDLINE:
		iRc = psi3.PanelControl(hPlugin, FCTL_GETCMDLINE, Param1, (void*)Param2); break;
	case Far2::FCTL_SETCMDLINE:
		iRc = psi3.PanelControl(hPlugin, FCTL_SETCMDLINE, Param1, (void*)Param2); break;
	case Far2::FCTL_SETSELECTION:
		iRc = psi3.PanelControl(hPlugin, FCTL_SETSELECTION, Param1, (void*)Param2); break;
	case Far2::FCTL_SETVIEWMODE:
		iRc = psi3.PanelControl(hPlugin, FCTL_SETVIEWMODE, Param1, (void*)Param2); break;
	case Far2::FCTL_INSERTCMDLINE:
		iRc = psi3.PanelControl(hPlugin, FCTL_INSERTCMDLINE, Param1, (void*)Param2); break;
	case Far2::FCTL_SETUSERSCREEN:
		iRc = psi3.PanelControl(hPlugin, FCTL_SETUSERSCREEN, Param1, (void*)Param2); break;
	case Far2::FCTL_SETPANELDIR:
		iRc = psi3.PanelControl(hPlugin, FCTL_SETPANELDIR, Param1, (void*)Param2); break;
	case Far2::FCTL_SETCMDLINEPOS:
		iRc = psi3.PanelControl(hPlugin, FCTL_SETCMDLINEPOS, Param1, (void*)Param2); break;
	case Far2::FCTL_GETCMDLINEPOS:
		iRc = psi3.PanelControl(hPlugin, FCTL_GETCMDLINEPOS, Param1, (void*)Param2); break;
	case Far2::FCTL_SETSORTMODE:
		iRc = psi3.PanelControl(hPlugin, FCTL_SETSORTMODE, Param1, (void*)Param2); break;
	case Far2::FCTL_SETSORTORDER:
		iRc = psi3.PanelControl(hPlugin, FCTL_SETSORTORDER, Param1, (void*)Param2); break;
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
	case Far2::FCTL_SETCMDLINESELECTION:
		iRc = psi3.PanelControl(hPlugin, FCTL_SETCMDLINESELECTION, Param1, (void*)Param2); break;
	case Far2::FCTL_GETCMDLINESELECTION:
		iRc = psi3.PanelControl(hPlugin, FCTL_GETCMDLINESELECTION, Param1, (void*)Param2); break;
	case Far2::FCTL_CHECKPANELSEXIST:
		iRc = psi3.PanelControl(hPlugin, FCTL_CHECKPANELSEXIST, Param1, (void*)Param2); break;
	case Far2::FCTL_SETNUMERICSORT:
		iRc = psi3.PanelControl(hPlugin, FCTL_SETNUMERICSORT, Param1, (void*)Param2); break;
	case Far2::FCTL_GETUSERSCREEN:
		iRc = psi3.PanelControl(hPlugin, FCTL_GETUSERSCREEN, Param1, (void*)Param2); break;
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
				ASSERTSTRUCTGT(PluginPanelItem);
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
	case Far2::FCTL_GETPANELDIR:
		iRc = psi3.PanelControl(hPlugin, FCTL_GETPANELDIR, Param1, (void*)Param2); break;
	case Far2::FCTL_GETCOLUMNTYPES:
		iRc = psi3.PanelControl(hPlugin, FCTL_GETCOLUMNTYPES, Param1, (void*)Param2); break;
	case Far2::FCTL_GETCOLUMNWIDTHS:
		iRc = psi3.PanelControl(hPlugin, FCTL_GETCOLUMNWIDTHS, Param1, (void*)Param2); break;
	case Far2::FCTL_BEGINSELECTION:
		iRc = psi3.PanelControl(hPlugin, FCTL_BEGINSELECTION, Param1, (void*)Param2); break;
	case Far2::FCTL_ENDSELECTION:
		iRc = psi3.PanelControl(hPlugin, FCTL_ENDSELECTION, Param1, (void*)Param2); break;
	case Far2::FCTL_CLEARSELECTION:
		iRc = psi3.PanelControl(hPlugin, FCTL_CLEARSELECTION, Param1, (void*)Param2); break;
	case Far2::FCTL_SETDIRECTORIESFIRST:
		iRc = psi3.PanelControl(hPlugin, FCTL_SETDIRECTORIESFIRST, Param1, (void*)Param2); break;
	}
	return iRc;
};
//std::map<Far2::FAR_FIND_DATA*,PluginPanelItem*> m_MapDirList;
//std::map<Far2::PluginPanelItem*,PluginPanelItem*> m_MapPlugDirList;
int WrapPluginInfo::FarApiGetDirList(const wchar_t *Dir, struct Far2::FAR_FIND_DATA **pPanelItem, int *pItemsNumber)
{
	LOG_CMD(L"psi2.GetDirList",0,0,0);
	PluginPanelItem* p3 = NULL;
	int iRc = psi3.GetDirList(Dir, &p3, pItemsNumber);
	if (iRc)
	{
		*pPanelItem = (Far2::FAR_FIND_DATA*)calloc(sizeof(Far2::FAR_FIND_DATA),*pItemsNumber);
		if (!*pPanelItem)
		{
			_ASSERTE(*pPanelItem != NULL);
			psi3.FreeDirList(p3, *pItemsNumber);
			iRc = 0;
		}
		else
		{
			m_MapDirList[*pPanelItem] = p3;
			for (int i = *pItemsNumber; (i--) > 0;)
			{
				PluginPanelItem_3_2(p3+i, (*pPanelItem)+i);
			}
		}
	}
	return iRc;
};
void WrapPluginInfo::FarApiFreeDirList(struct Far2::FAR_FIND_DATA *PanelItem, int nItemsNumber)
{
	LOG_CMD(L"psi2.FreeDirList",0,0,0);
	PluginPanelItem* p3 = m_MapDirList[PanelItem];
	if (p3)
		psi3.FreeDirList(p3, nItemsNumber);
	m_MapDirList.erase(PanelItem);
	free(PanelItem);
};
int WrapPluginInfo::FarApiGetPluginDirList(INT_PTR PluginNumber, HANDLE hPlugin, const wchar_t *Dir, struct Far2::PluginPanelItem **pPanelItem, int *pItemsNumber)
{
	_ASSERTE(PluginNumber == psi2.ModuleNumber);
	LOG_CMD(L"psi2.GetPluginDirList",0,0,0);
	PluginPanelItem* p3 = NULL;
	int iRc = psi3.GetPluginDirList(&mguid_Plugin, hPlugin, Dir, &p3, pItemsNumber);
	if (iRc)
	{
		*pPanelItem = (Far2::PluginPanelItem*)calloc(sizeof(Far2::PluginPanelItem),*pItemsNumber);
		if (!*pPanelItem)
		{
			_ASSERTE(*pPanelItem != NULL);
			psi3.FreeDirList(p3, *pItemsNumber);
			iRc = 0;
		}
		else
		{
			m_MapPlugDirList[*pPanelItem] = p3;
			for (int i = *pItemsNumber; (i--) > 0;)
			{
				PluginPanelItem_3_2(p3+i, (*pPanelItem)+i);
			}
		}
	}
	return iRc;
};
void WrapPluginInfo::FarApiFreePluginDirList(struct Far2::PluginPanelItem *PanelItem, int nItemsNumber)
{
	LOG_CMD(L"psi2.FreePluginDirList",0,0,0);
	PluginPanelItem* p3 = m_MapPlugDirList[PanelItem];
	if (p3)
		psi3.FreePluginDirList(p3, nItemsNumber);
	m_MapPlugDirList.erase(PanelItem);
	free(PanelItem);
};
int WrapPluginInfo::FarApiCmpName(const wchar_t *Pattern, const wchar_t *String, int SkipPath)
{
	LOG_CMD(L"psi2.CmdName",0,0,0);
	wchar_t* pszFile = (wchar_t*)(SkipPath ? FSF3.PointToName(String) : String);
	int iRc = FSF3.ProcessName(Pattern, pszFile, 0, PN_CMPNAME);
	return iRc;
};
LPCWSTR WrapPluginInfo::FarApiGetMsg(INT_PTR PluginNumber, int MsgId)
{
	LOG_CMD(L"psi2.GetMsg(%i)",MsgId,0,0);
	if (((INT_PTR)mh_Dll) == PluginNumber)
		return psi3.GetMsg(&mguid_Plugin, MsgId);
	return L"";
};
INT_PTR WrapPluginInfo::FarApiAdvControl(INT_PTR ModuleNumber, int Command, void *Param)
{
	LOG_CMD(L"psi2.AdvControl(%i)",Command,0,0);
	INT_PTR iRc = 0;
	GUID guid = {0};
	if (((INT_PTR)mh_Dll) == ModuleNumber)
	{
		_ASSERTE(((INT_PTR)mh_Dll) == ModuleNumber);
		guid = mguid_Plugin;
	}
	
	switch (Command)
	{
	case Far2::ACTL_GETFARVERSION:
		{
			VersionInfo vi = {0};
			iRc = psi3.AdvControl(&guid, ACTL_GETFARMANAGERVERSION, 0, &vi);
			if (iRc)
			{
				DWORD ver = MAKEFARVERSION2(vi.Major, vi.Minor, vi.Build);
				if (Param)
					*((DWORD*)Param) = ver;
				iRc = ver;
			}
			break;
		}
	case Far2::ACTL_GETSYSWORDDIV:
		iRc = psi3.AdvControl(&guid, ACTL_GETSYSWORDDIV, 0, Param); break;
	case Far2::ACTL_WAITKEY:
		iRc = psi3.AdvControl(&guid, ACTL_WAITKEY, 0, Param); break;
	case Far2::ACTL_GETCOLOR:
		iRc = psi3.AdvControl(&guid, ACTL_GETCOLOR, (int)(INT_PTR)Param, NULL); break;
	case Far2::ACTL_GETARRAYCOLOR:
		iRc = psi3.AdvControl(&guid, ACTL_GETARRAYCOLOR, 0, Param); break;
	case Far2::ACTL_EJECTMEDIA:
		ASSERTSTRUCT(ActlEjectMedia);
		iRc = psi3.AdvControl(&guid, ACTL_EJECTMEDIA, 0, Param);
		break;
	case Far2::ACTL_KEYMACRO:
		{
			if (Param)
			{
				Far2::ActlKeyMacro* p2 = (Far2::ActlKeyMacro*)Param;
				//iRc = psi3.AdvControl(&guid, ACTL_KEYMACRO, Param);
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
						wchar_t *pszUpper = NULL, *pszChanged = NULL;
						// Плагин может звать сам себя через CallPlugin, 
						// в этом случае нужно заменить старый PluginID (DWORD) на GUID
						if (mcr.SequenceText && *mcr.SequenceText && m_Info.Reserved)
						{
							pszUpper = _wcsdup(p2->Param.PlainText.SequenceText);
							size_t nOrigLen = lstrlen(p2->Param.PlainText.SequenceText);
							CharUpperBuff(pszUpper, lstrlen(pszUpper));
							wchar_t* pszFrom = pszUpper;
							wchar_t* pszCall;
							WCHAR /*szIdDec[32], szIdHex1[32], szIdHex2[32], szId[32],*/ szGuid[64];
							//wsprintf(szIdDec, L"%u", m_Info.Reserved);
							//wsprintf(szIdHex1, L"0X%X", m_Info.Reserved);
							//wsprintf(szIdHex2, L"0X%08X", m_Info.Reserved);
							//FormatGuid(&mguid_Plugin, szGuid, TRUE);
							int nCchAdd = 0, nCchGuidsAdd = 0;
							while ((pszCall = wcsstr(pszFrom, L"CALLPLUGIN")) != NULL)
							{
								pszCall = pszCall + 10; // lstrlen(L"CALLPLUGIN")
								pszFrom = pszCall; // сразу, чтобы не забыть и не зациклиться
								while (*pszCall == L' ' || *pszCall == L'\t') pszCall++;
								if (*pszCall != L'(') continue;
								pszCall++;
								while (*pszCall == L' ' || *pszCall == L'\t') pszCall++;
								if (*pszCall < L'0' || *pszCall > L'9') continue; // допускаются только числа

								szGuid[0] = 0;
								DWORD nID = 0;
								wchar_t* pszEnd = NULL;
								if (pszCall[1] == L'X')
									nID = wcstoul(pszCall+2, &pszEnd, 16);
								else
									nID = wcstoul(pszCall, &pszEnd, 10);
								if (!nID || !pszEnd || (pszEnd <= pszCall))
									continue; // ошибка в ID?

								if (nID == m_Info.Reserved)
									FormatGuid(&mguid_Plugin, szGuid, TRUE);
								else
								{
									LPCWSTR pszGuid = NULL;
									switch (nID)
									{
									case 0x43454D55: // ConEmu
										pszGuid = L"4b675d80-1d4a-4ea9-8436-fdc23f2fc14b"; break;
									case 0x43455568: // ConEmuTh
										pszGuid = L"bd454d48-448e-46cc-909d-b6cf789c2d65"; break;
									// Wrapper!
									case 0x424C434D: // MacroLib
										pszGuid = L"46FC0BF1-CC99-4652-B41D-C7B8705D52AF"; break;
									case 0x436C4678: // ClipFixer
										pszGuid = L"AF0DD773-7193-4F50-9904-AB24673EB42C"; break;
									//const SameFolder = 0x44464D53
									case 0x444E4645: // EditFind
										pszGuid = L"8EF28982-957E-4BCE-AD73-7E67DB443969"; break;
									case 0x44654272: // DeepBrowser
										pszGuid = L"D1778FAF-B6B1-4604-9D9C-CBCF3B15F7A8"; break;
									case 0x466C5470: // FileTypes
										pszGuid = L"32C72FF2-8762-4485-9BAE-2020B9143AA0"; break;
									case 0x4D426C6B: // MBlockEditor
										pszGuid = L"D82D6847-0C7B-4BF4-9A31-B0B929707854"; break;
									case 0x4D4D5657: // MMView
										pszGuid = L"44E0DA00-F361-4ACC-BF8F-DDC7D2E0494F"; break;
									case 0x52674564: // RegEditor
										pszGuid = L"F6A1E51C-1C11-4BD5-ADD2-8677348BC106"; break;
									//const FarHints = 0x544E4948
									case 0x5774654E: // Network
										pszGuid = L"9E724C40-D0B6-4DC3-9F30-CC7AF5292C5B"; break;
									case 0xA91B3F07: // PanelTabs
										pszGuid = L"66D5D731-EE8E-4113-87F3-56883CC321DA"; break;
									}
									if (pszGuid)
									{
										szGuid[0] = L'"'; lstrcpy(szGuid+1, pszGuid); lstrcat(szGuid, L"\"");
									}
								}

								if (szGuid[0] == 0)
									continue; // неизвестный ID, сделать ничего не сможем

								if (nCchAdd < 38 || !pszChanged)
								{
									wchar_t* pszNew = NULL;
									int nLen = 0;
									nCchAdd = 38*10; // 10 гуидов с кавычками
									if (pszChanged == NULL)
									{
										nLen = lstrlen(p2->Param.PlainText.SequenceText);
										pszNew = (wchar_t*)calloc(nLen+nCchAdd+1, sizeof(wchar_t));
										if (!pszNew) { _ASSERTE(pszNew!=NULL); break; }
										lstrcpy(pszNew, p2->Param.PlainText.SequenceText);
									}
									else
									{
										nLen = lstrlen(pszChanged);
										pszNew = (wchar_t*)calloc(nLen+nCchAdd+1, sizeof(wchar_t));
										if (!pszNew) { _ASSERTE(pszNew!=NULL); break; }
										lstrcpy(pszNew, pszChanged);
										free(pszChanged);
									}
									pszChanged = pszNew;
								}

								size_t nPos = pszCall - pszUpper;
								size_t nOrigIdLen = pszEnd - pszCall;
								size_t nNewIdLen = lstrlen(szGuid);
								_ASSERTE(nNewIdLen==38);
								if (nOrigLen < (nOrigIdLen+nPos))
								{
									_ASSERTE(nOrigLen > (nOrigIdLen+nPos));
									break;
								}
								// Освободить место для GUID (размер такой, т.к. память выделена calloc)
								memmove(pszChanged+nNewIdLen+nPos, pszChanged+nOrigIdLen+nPos, (nOrigLen-nOrigIdLen-nPos)*sizeof(wchar_t));
								// положить новый ИД
								memmove(pszChanged+nPos, szGuid, nNewIdLen*sizeof(wchar_t));

								nCchAdd -= nNewIdLen;
								pszFrom = pszCall + nOrigIdLen;

								//// Теперь - скопировать ID в szId
								//for (int i = 0; i < 12 
								//		&& ( (pszCall[i] >= L'0' && pszCall[i] <= L'9')
								//		  || (pszCall[i] >= L'A' && pszCall[i] <= L'F') // был CharUpperBuff
								//		  || (pszCall[i] >= L'X') // HEX
								//		    ) ; i++)
								//{
								//	szId[i] = pszCall[i]; szId[i+1] = 0;
								//}
								////
								//if (lstrcmp(szId, szIdDec) && lstrcmp(szId, szIdHex1) && lstrcmp(szId, szIdHex2))
								//	continue; 
								//if (szGuid[0] == 0)
								//	continue; // неизвестный ID, сделать ничего не сможем
							}

							if (pszChanged != NULL)
								mcr.SequenceText = pszChanged;
						}
						mcr.Flags = 0
							| ((p2->Param.PlainText.Flags & Far2::KSFLAGS_DISABLEOUTPUT) ? KMFLAGS_DISABLEOUTPUT : 0)
							| ((p2->Param.PlainText.Flags & Far2::KSFLAGS_NOSENDKEYSTOPLUGINS) ? KMFLAGS_NOSENDKEYSTOPLUGINS : 0)
							| ((p2->Param.PlainText.Flags & Far2::KSFLAGS_REG_MULTI_SZ) ? KMFLAGS_REG_MULTI_SZ : 0)
							| ((p2->Param.PlainText.Flags & Far2::KSFLAGS_SILENTCHECK) ? KMFLAGS_SILENTCHECK : 0);
						psi3.MacroControl(INVALID_HANDLE_VALUE, MCTL_SENDSTRING, 0, &mcr);
						if (pszUpper)
							free(pszUpper);
						if (pszChanged)
							free(pszChanged);
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
					// В Far2 официально это в plugin api вынесено не было, но пользовались многие
					_ASSERTE(MACROAREA_AUTOCOMPLETION==15 && MACROAREA_SHELL==1);
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
	case Far2::ACTL_GETWINDOWINFO:
		//iRc = psi3.AdvControl(&guid, ACTL_GETWINDOWINFO, 0, Param);
		{
			Far2::WindowInfo* p2 = (Far2::WindowInfo*)Param;
			WindowInfo wi = {sizeof(WindowInfo)};
			wi.Pos = p2->Pos;
			wi.TypeName = p2->TypeName; wi.TypeNameSize = p2->TypeNameSize;
			wi.Name = p2->Name; wi.NameSize = p2->NameSize;
			iRc = psi3.AdvControl(&guid, ACTL_GETWINDOWINFO, 0, &wi);
			if (iRc)
			{
				p2->Pos = wi.Pos;
				p2->Type = wi.Type; //TODO: Конвертация типа?
				p2->Modified = (wi.Flags & WIF_MODIFIED) == WIF_MODIFIED;
				p2->Current = (wi.Flags & WIF_CURRENT) == WIF_CURRENT;
				p2->TypeNameSize = wi.TypeNameSize;
				p2->NameSize = wi.NameSize;
			}
		}
		break;
	case Far2::ACTL_GETWINDOWCOUNT:
		iRc = psi3.AdvControl(&guid, ACTL_GETWINDOWCOUNT, 0, Param); break;
	case Far2::ACTL_SETCURRENTWINDOW:
		iRc = psi3.AdvControl(&guid, ACTL_SETCURRENTWINDOW, 0, Param); break;
	case Far2::ACTL_COMMIT:
		iRc = psi3.AdvControl(&guid, ACTL_COMMIT, 0, Param); break;
	case Far2::ACTL_GETFARHWND:
		iRc = psi3.AdvControl(&guid, ACTL_GETFARHWND, 0, Param); break;
	case Far2::ACTL_GETSYSTEMSETTINGS:
		iRc = psi3.AdvControl(&guid, ACTL_GETSYSTEMSETTINGS, 0, Param); break;
	case Far2::ACTL_GETPANELSETTINGS:
		iRc = psi3.AdvControl(&guid, ACTL_GETPANELSETTINGS, 0, Param); break;
	case Far2::ACTL_GETINTERFACESETTINGS:
		iRc = psi3.AdvControl(&guid, ACTL_GETINTERFACESETTINGS, 0, Param); break;
	case Far2::ACTL_GETCONFIRMATIONS:
		iRc = psi3.AdvControl(&guid, ACTL_GETCONFIRMATIONS, 0, Param); break;
	case Far2::ACTL_GETDESCSETTINGS:
		iRc = psi3.AdvControl(&guid, ACTL_GETDESCSETTINGS, 0, Param); break;
	case Far2::ACTL_SETARRAYCOLOR:
		iRc = psi3.AdvControl(&guid, ACTL_SETARRAYCOLOR, 0, Param); break;
	case Far2::ACTL_GETPLUGINMAXREADDATA:
		iRc = psi3.AdvControl(&guid, ACTL_GETPLUGINMAXREADDATA, 0, Param); break;
	case Far2::ACTL_GETDIALOGSETTINGS:
		iRc = psi3.AdvControl(&guid, ACTL_GETDIALOGSETTINGS, 0, Param); break;
	case Far2::ACTL_GETSHORTWINDOWINFO:
		{
			Far2::WindowInfo* p2 = (Far2::WindowInfo*)Param;
			if (p2->Pos == -1)
			{
				memset(p2, 0, sizeof(*p2));
				p2->Pos = -1;
				//BUGBUG: Поскольку ACTL_GETWINDOWINFO нифига не ThreadSafe - юзаем MCTL_GETAREA
				if (GetCurrentThreadId() != gnMainThreadId)
				{
					//iRc = psi3.AdvControl(&guid, ACTL_GETSHORTWINDOWINFO, Param);
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
				else
				{
					WindowInfo wi = {sizeof(WindowInfo)};
					wi.Pos = p2->Pos;
					iRc = psi3.AdvControl(&guid, ACTL_GETWINDOWINFO, 0, &wi);
					if (iRc)
					{
						p2->Pos = wi.Pos;
						p2->Type = wi.Type; //TODO: Конвертация типа?
						p2->Modified = (wi.Flags & WIF_MODIFIED) == WIF_MODIFIED;
						p2->Current = (wi.Flags & WIF_CURRENT) == WIF_CURRENT;
						p2->TypeNameSize = wi.TypeNameSize;
						p2->NameSize = wi.NameSize;
					}
				}
			}
		}
		break;
	case Far2::ACTL_REDRAWALL:
		iRc = psi3.AdvControl(&guid, ACTL_REDRAWALL, 0, Param); break;
	case Far2::ACTL_SYNCHRO:
		iRc = psi3.AdvControl(&guid, ACTL_SYNCHRO, 0, Param); break;
	case Far2::ACTL_SETPROGRESSSTATE:
		iRc = psi3.AdvControl(&guid, ACTL_SETPROGRESSSTATE, 0, Param); break;
	case Far2::ACTL_SETPROGRESSVALUE:
		iRc = psi3.AdvControl(&guid, ACTL_SETPROGRESSVALUE, 0, Param); break;
	case Far2::ACTL_QUIT:
		iRc = psi3.AdvControl(&guid, ACTL_QUIT, 0, Param); break;
	case Far2::ACTL_GETFARRECT:
		iRc = psi3.AdvControl(&guid, ACTL_GETFARRECT, 0, Param); break;
	case Far2::ACTL_GETCURSORPOS:
		iRc = psi3.AdvControl(&guid, ACTL_GETCURSORPOS, 0, Param); break;
	case Far2::ACTL_SETCURSORPOS:
		iRc = psi3.AdvControl(&guid, ACTL_SETCURSORPOS, 0, Param); break;
	}
	return iRc;
};
int WrapPluginInfo::FarApiViewerControl(int Command, void *Param)
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
	case Far2::VCTL_QUIT:
		nRc = psi3.ViewerControl(-1, VCTL_QUIT, 0, (void*)Param); break;
	case Far2::VCTL_REDRAW:
		nRc = psi3.ViewerControl(-1, VCTL_REDRAW, 0, (void*)Param); break;
	case Far2::VCTL_SETKEYBAR:
		{
			KeyBarTitles* p3 = (KeyBarTitles*)Param;
			if (Param && (Param != (void*)-1))
			{
				const Far2::KeyBarTitles* p2 = (const Far2::KeyBarTitles*)Param;
				p3 = KeyBarTitles_2_3(p2);
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
		ASSERTSTRUCT(ViewerSelect);
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
int WrapPluginInfo::FarApiEditorControl(int Command, void *Param)
{
	LOG_CMD0(L"psi2.EditorControl(%i)",Command,0,0);
	int nRc = 0; //psi3.EditorControl(-1, Command, 0, Param);
	//TODO: Для некоторых функций нужна конвертация аргументов
	switch (Command)
	{
	case Far2::ECTL_GETSTRING:
		ASSERTSTRUCT(EditorGetString);
		nRc = psi3.EditorControl(-1, ECTL_GETSTRING, 0, (void*)Param);
		break;
	case Far2::ECTL_SETSTRING:
		ASSERTSTRUCT(EditorSetString);
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
		ASSERTSTRUCT(EditorSetPosition);
		nRc = psi3.EditorControl(-1, ECTL_SETPOSITION, 0, (void*)Param);
		break;
	case Far2::ECTL_SELECT:
		ASSERTSTRUCT(EditorSelect);
		nRc = psi3.EditorControl(-1, ECTL_SELECT, 0, (void*)Param);
		break;
	case Far2::ECTL_REDRAW:
		nRc = psi3.EditorControl(-1, ECTL_REDRAW, 0, (void*)Param); break;
	case Far2::ECTL_TABTOREAL:
		ASSERTSTRUCT(EditorConvertPos);
		nRc = psi3.EditorControl(-1, ECTL_TABTOREAL, 0, (void*)Param);
		break;
	case Far2::ECTL_REALTOTAB:
		ASSERTSTRUCT(EditorConvertPos);
		nRc = psi3.EditorControl(-1, ECTL_REALTOTAB, 0, (void*)Param);
		break;
	case Far2::ECTL_EXPANDTABS:
		nRc = psi3.EditorControl(-1, ECTL_EXPANDTABS, 0, (void*)Param); break;
	case Far2::ECTL_SETTITLE:
		nRc = psi3.EditorControl(-1, ECTL_SETTITLE, 0, (void*)Param); break;
	case Far2::ECTL_READINPUT:
		nRc = psi3.EditorControl(-1, ECTL_READINPUT, 0, (void*)Param); break;
	case Far2::ECTL_PROCESSINPUT:
		nRc = psi3.EditorControl(-1, ECTL_PROCESSINPUT, 0, (void*)Param); break;
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
		ASSERTSTRUCT(EditorSaveFile);
		nRc = psi3.EditorControl(-1, ECTL_SAVEFILE, 0, (void*)Param);
		break;
	case Far2::ECTL_QUIT:
		nRc = psi3.EditorControl(-1, ECTL_QUIT, 0, (void*)Param); break;
	case Far2::ECTL_SETKEYBAR:
		{
			KeyBarTitles* p3 = (KeyBarTitles*)Param;
			if (Param && (Param != (void*)-1))
			{
				const Far2::KeyBarTitles* p2 = (const Far2::KeyBarTitles*)Param;
				p3 = KeyBarTitles_2_3(p2);
			}
			nRc = psi3.EditorControl(-1, ECTL_SETKEYBAR, 0, p3);
		}
		break;
	case Far2::ECTL_PROCESSKEY:
		nRc = psi3.EditorControl(-1, ECTL_PROCESSKEY, 0, (void*)Param); break;
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
	case Far2::ECTL_GETBOOKMARKS:
		nRc = psi3.EditorControl(-1, ECTL_GETBOOKMARKS, 0, (void*)Param); break;
	case Far2::ECTL_TURNOFFMARKINGBLOCK:
		nRc = psi3.EditorControl(-1, ECTL_TURNOFFMARKINGBLOCK, 0, (void*)Param); break;
	case Far2::ECTL_DELETEBLOCK:
		nRc = psi3.EditorControl(-1, ECTL_DELETEBLOCK, 0, (void*)Param); break;
	case Far2::ECTL_ADDSTACKBOOKMARK:
		nRc = psi3.EditorControl(-1, ECTL_ADDSTACKBOOKMARK, 0, (void*)Param); break;
	case Far2::ECTL_PREVSTACKBOOKMARK:
		nRc = psi3.EditorControl(-1, ECTL_PREVSTACKBOOKMARK, 0, (void*)Param); break;
	case Far2::ECTL_NEXTSTACKBOOKMARK:
		nRc = psi3.EditorControl(-1, ECTL_NEXTSTACKBOOKMARK, 0, (void*)Param); break;
	case Far2::ECTL_CLEARSTACKBOOKMARKS:
		nRc = psi3.EditorControl(-1, ECTL_CLEARSTACKBOOKMARKS, 0, (void*)Param); break;
	case Far2::ECTL_DELETESTACKBOOKMARK:
		nRc = psi3.EditorControl(-1, ECTL_DELETESTACKBOOKMARK, 0, (void*)Param); break;
	case Far2::ECTL_GETSTACKBOOKMARKS:
		nRc = psi3.EditorControl(-1, ECTL_GETSTACKBOOKMARKS, 0, (void*)Param); break;
	case Far2::ECTL_UNDOREDO:
		{
			const Far2::EditorUndoRedo* p2 = (const Far2::EditorUndoRedo*)Param;
			EditorUndoRedo p3 = {(EDITOR_UNDOREDO_COMMANDS)0};
			//TODO: Конвертация команды
			p3.Command = (EDITOR_UNDOREDO_COMMANDS)p2->Command;
			nRc = psi3.EditorControl(-1, ECTL_UNDOREDO, 0, &p3);
		}
		break;
	case Far2::ECTL_GETFILENAME:
		nRc = psi3.EditorControl(-1, ECTL_GETFILENAME, 0, (void*)Param); break;
	}
	return nRc;
};
int WrapPluginInfo::FarApiInputBox(const wchar_t *Title, const wchar_t *SubTitle, const wchar_t *HistoryName, const wchar_t *SrcText, wchar_t *DestText, int   DestLength, const wchar_t *HelpTopic, DWORD Flags)
{
	LOG_CMD(L"psi2.InputBox",0,0,0);
	//TODO: Флаги
	int nRc = psi3.InputBox(&mguid_Plugin, Title, SubTitle, HistoryName, SrcText, DestText, DestLength, HelpTopic, Flags);
	return nRc;
};
int WrapPluginInfo::FarApiPluginsControl(HANDLE hHandle, int Command, int Param1, LONG_PTR Param2)
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
int WrapPluginInfo::FarApiFileFilterControl(HANDLE hHandle, int Command, int Param1, LONG_PTR Param2)
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
		{
			Far2::FAR_FIND_DATA* p2 = (Far2::FAR_FIND_DATA*)Param2;
			PluginPanelItem p3;
			PluginPanelItem_2_3(p2, &p3);
			nRc = psi3.FileFilterControl(hHandle, FFCTL_ISFILEINFILTER, Param1, (void*)&p3);
		}
		break;
	}
	return nRc;
};
int WrapPluginInfo::FarApiRegExpControl(HANDLE hHandle, int Command, LONG_PTR Param)
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
int WrapPluginInfo::FarStdGetFileOwner(const wchar_t *Computer,const wchar_t *Name,wchar_t *Owner,int Size)
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
int WrapPluginInfo::FarStdGetPathRoot(const wchar_t *Path,wchar_t *Root, int DestSize)
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
wchar_t* WrapPluginInfo::FarStdXlatW3(wchar_t *Line,int StartPos,int EndPos,DWORD Flags)
{
	LOG_CMD(L"fsf2.XLat",0,0,0);
	wchar_t* pszRc = FSF3.XLat(Line, StartPos, EndPos, Flags);
	return pszRc;
};
int WrapPluginInfo::RecSearchUserFn(const struct PluginPanelItem *FData, const wchar_t *FullName, void *Param)
{
	RecSearchUserFnArg* p = (RecSearchUserFnArg*)Param;
	Far2::FAR_FIND_DATA ffd;
	PluginPanelItem_3_2(FData, &ffd);
	return p->UserFn2(&ffd, FullName, p->Param2);
}
void WrapPluginInfo::FarStdRecursiveSearch(const wchar_t *InitDir,const wchar_t *Mask,Far2::FRSUSERFUNC Func,DWORD Flags,void *Param)
{
	LOG_CMD(L"fsf2.RecursiveSearch",0,0,0);
	RecSearchUserFnArg arg = {Func, Param};
	FRSMODE Flags3 = (FRSMODE)0;
	if (Flags & Far2::FRS_RETUPDIR)
		Flags3 |= FRS_RETUPDIR;
	if (Flags & Far2::FRS_RECUR)
		Flags3 |= FRS_RECUR;
	if (Flags & Far2::FRS_SCANSYMLINK)
		Flags3 |= FRS_SCANSYMLINK;
	FSF3.FarRecursiveSearch(InitDir, Mask, RecSearchUserFn, Flags3, &arg);
};
int WrapPluginInfo::FarStdMkTemp(wchar_t *Dest, DWORD size, const wchar_t *Prefix)
{
	LOG_CMD(L"fsf2.MkTemp",0,0,0);
	__int64 nRc = FSF3.MkTemp(Dest, size, Prefix);
	return (int)nRc;
};
int WrapPluginInfo::FarStdProcessName(const wchar_t *param1, wchar_t *param2, DWORD size, DWORD flags)
{
	LOG_CMD(L"fsf2.ProcessName",0,0,0);
	__int64 nRc = FSF3.ProcessName(param1, param2, size, flags);
	return (int)nRc;
};
int WrapPluginInfo::FarStdMkLink(const wchar_t *Src,const wchar_t *Dest,DWORD Flags)
{
	LOG_CMD(L"fsf2.MkLink",0,0,0);
	LINK_TYPE Type3 = (LINK_TYPE)(Flags & 7);
	MKLINK_FLAGS Flags3 = (MKLINK_FLAGS)(Flags & 0x30000);
	BOOL nRc = FSF3.MkLink(Src, Dest, Type3, Flags3);
	return nRc;
};
int WrapPluginInfo::FarConvertPath(enum Far2::CONVERTPATHMODES Mode, const wchar_t *Src, wchar_t *Dest, int DestSize)
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
int WrapPluginInfo::FarGetReparsePointInfo(const wchar_t *Src, wchar_t *Dest,int DestSize)
{
	LOG_CMD(L"fsf2.GetReparsePointInfo",0,0,0);
	__int64 nRc = FSF3.GetReparsePointInfo(Src, Dest, DestSize);
	return (int)nRc;
};
DWORD WrapPluginInfo::FarGetCurrentDirectory(DWORD Size,wchar_t* Buffer)
{
	LOG_CMD(L"fsf2.GetCurrentDirectory",0,0,0);
	size_t nRc = FSF3.GetCurrentDirectory(Size, Buffer);
	return (DWORD)nRc;
};












void WrapPluginInfo::SetStartupInfoW3(PluginStartupInfo *Info)
{
	memmove(&psi3, Info, sizeof(psi3));
	memmove(&FSF3, Info->FSF, sizeof(FSF3));
	psi3.FSF = &FSF3;
	lbPsi3 = TRUE;
	
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
	FSF2.GetPathRoot = WrapPluginInfo::FarStdGetPathRootExp;
	FSF2.AddEndSlash = FSF3.AddEndSlash;
	FSF2.CopyToClipboard = FSF3.CopyToClipboard;
	FSF2.PasteFromClipboard = FSF3.PasteFromClipboard;
	FSF2.FarKeyToName = FSF3.FarKeyToName;
	FSF2.FarNameToKey = FSF3.FarNameToKey;
	FSF2.FarInputRecordToKey = FSF3.FarInputRecordToKey;
	FSF2.XLat = WrapPluginInfo::FarStdXlatExp;
	FSF2.GetFileOwner = WrapPluginInfo::FarStdGetFileOwnerExp;
	FSF2.GetNumberOfLinks = FSF3.GetNumberOfLinks;
	FSF2.FarRecursiveSearch = WrapPluginInfo::FarStdRecursiveSearchExp;
	FSF2.MkTemp = WrapPluginInfo::FarStdMkTempExp;
	FSF2.DeleteBuffer = FSF3.DeleteBuffer;
	FSF2.ProcessName = WrapPluginInfo::FarStdProcessNameExp;
	FSF2.MkLink = WrapPluginInfo::FarStdMkLinkExp;
	FSF2.ConvertPath = WrapPluginInfo::FarConvertPathExp;
	FSF2.GetReparsePointInfo = WrapPluginInfo::FarGetReparsePointInfoExp;
	FSF2.GetCurrentDirectory = WrapPluginInfo::FarGetCurrentDirectoryExp;
	
	// *** PluginStartupInfo ***
	psi2.StructSize = sizeof(psi2);
	psi2.ModuleName = ms_PluginDll;
	psi2.ModuleNumber = (INT_PTR)mh_Dll;
	psi2.RootKey = ms_RegRoot;
	psi2.Menu = WrapPluginInfo::FarApiMenuExp;
	psi2.Message = WrapPluginInfo::FarApiMessageExp;
	psi2.GetMsg = WrapPluginInfo::FarApiGetMsgExp;
	psi2.Control = WrapPluginInfo::FarApiControlExp;
	psi2.SaveScreen = WrapPluginInfo::FarApiSaveScreenExp;
	psi2.RestoreScreen = WrapPluginInfo::FarApiRestoreScreenExp;
	psi2.GetDirList = WrapPluginInfo::FarApiGetDirListExp;
	psi2.GetPluginDirList = WrapPluginInfo::FarApiGetPluginDirListExp;
	psi2.FreeDirList = WrapPluginInfo::FarApiFreeDirListExp;
	psi2.FreePluginDirList = WrapPluginInfo::FarApiFreePluginDirListExp;
	psi2.Viewer = WrapPluginInfo::FarApiViewerExp;
	psi2.Editor = WrapPluginInfo::FarApiEditorExp;
	psi2.CmpName = WrapPluginInfo::FarApiCmpNameExp;
	psi2.Text = WrapPluginInfo::FarApiTextExp;
	psi2.EditorControl = WrapPluginInfo::FarApiEditorControlExp;

	psi2.FSF = &FSF2;

	psi2.ShowHelp = WrapPluginInfo::FarApiShowHelpExp;
	psi2.AdvControl = WrapPluginInfo::FarApiAdvControlExp;
	psi2.InputBox = WrapPluginInfo::FarApiInputBoxExp;
	psi2.DialogInit = WrapPluginInfo::FarApiDialogInitExp;
	psi2.DialogRun = WrapPluginInfo::FarApiDialogRunExp;
	psi2.DialogFree = WrapPluginInfo::FarApiDialogFreeExp;

	psi2.SendDlgMessage = WrapPluginInfo::FarApiSendDlgMessageExp;
	psi2.DefDlgProc = WrapPluginInfo::FarApiDefDlgProcExp;
	//DWORD_PTR              Reserved;
	psi2.ViewerControl = WrapPluginInfo::FarApiViewerControlExp;
	psi2.PluginsControl = WrapPluginInfo::FarApiPluginsControlExp;
	psi2.FileFilterControl = WrapPluginInfo::FarApiFileFilterControlExp;
	psi2.RegExpControl = WrapPluginInfo::FarApiRegExpControlExp;

	lbPsi2 = TRUE;
	
	if (SetStartupInfoW && lbPsi2)
	{
		SetStartupInfoW(&psi2);
	}
}

void WrapPluginInfo::GetGlobalInfoW3(GlobalInfo *Info)
{
	//LOG_CMD(L"GetGlobalInfoW",0,0,0);
	LoadPluginInfo();
	
	Info->StructSize = sizeof(GlobalInfo);
	Info->MinFarVersion = FARMANAGERVERSION;

	Info->Version = m_Version;
	Info->Guid = mguid_Plugin;
	Info->Title = ms_Title;
	Info->Description = ms_Desc;
	Info->Author = ms_Author;
}



void WrapPluginInfo::GetPluginInfoW3(PluginInfo *Info)
{
    memset(Info, 0, sizeof(PluginInfo));
    
	Info->StructSize = sizeof(*Info);

	//_ASSERTE(lbPsi2 && lbPsi3);
	OutputDebugString(L"GetPluginInfoW3 called before SetStartupInfoW3\n");

    if (GetPluginInfoW && lbPsi2)
    {
    	LOG_CMD(L"GetPluginInfoW",0,0,0);
		ZeroStruct(m_Info);
		m_Info.StructSize = sizeof(m_Info);
    	GetPluginInfoW(&m_Info);
    	
    	if (m_Info.Flags & Far2::PF_PRELOAD)
    		Info->Flags |= PF_PRELOAD;
    	if (m_Info.Flags & Far2::PF_DISABLEPANELS)
    		Info->Flags |= PF_DISABLEPANELS;
    	if (m_Info.Flags & Far2::PF_EDITOR)
    		Info->Flags |= PF_EDITOR;
    	if (m_Info.Flags & Far2::PF_VIEWER)
    		Info->Flags |= PF_VIEWER;
    	if (m_Info.Flags & Far2::PF_FULLCMDLINE)
    		Info->Flags |= PF_FULLCMDLINE;
    	if (m_Info.Flags & Far2::PF_DIALOG)
    		Info->Flags |= PF_DIALOG;

		//if (GetPrivateProfileString(L"Plugin", L"MenuGUID", L"", szTemp, ARRAYSIZE(szTemp), szIni))
		//{
		//	if (
		//		guid_PluginMenu = guid;
		//	else
		//		guid_PluginMenu = ::guid_DefPluginMenu;
		//}

		wchar_t szValName[64], szGUID[128]; GUID guid;
		struct { LPCWSTR Fmt; int Count; const wchar_t * const * Strings;
				 PluginMenuItem *Menu; int* GuidCount; GUID** Guids; }
			Menus[] = 
		{
			{L"PluginMenuGUID%i", m_Info.PluginMenuStringsNumber, m_Info.PluginMenuStrings, 
				&Info->PluginMenu, &mn_PluginMenu, &mguids_PluginMenu},
			{L"DiskMenuGUID%i", m_Info.DiskMenuStringsNumber, m_Info.DiskMenuStrings, 
				&Info->DiskMenu, &mn_PluginDisks, &mguids_PluginDisks},
			{L"PluginConfigGUID%i", m_Info.PluginConfigStringsNumber, m_Info.PluginConfigStrings, 
				&Info->PluginConfig, &mn_PluginConfig, &mguids_PluginConfig}
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
					BOOL lbGot = FALSE;
					szGUID[0] = 0;
    				lbGot = (GetPrivateProfileString(L"Plugin", szValName, L"", szGUID, ARRAYSIZE(szGUID), ms_IniFile)
						&& (UuidFromStringW((RPC_WSTR)szGUID, &guid) == RPC_S_OK));
    				if (!lbGot)
					{
						RPC_STATUS hr = UuidCreate(&guid);
						if (FAILED(hr))
							break;
						WritePrivateProfileString(L"Plugin", szValName, FormatGuid(&guid, szGUID), ms_IniFile);
					}
    				// OK
    				(*Menus[k].Guids)[i-1] = guid;
    				(*Menus[k].GuidCount)++;
    			}
				if (Menus[k].Count != *Menus[k].GuidCount)
				{
					//TODO: Сказать пользователю про ошибку в настройке
					_ASSERTE(Menus[k].Count == *Menus[k].GuidCount);
    				Menus[k].Count = *Menus[k].GuidCount; // могло не на всех гуидов хватить
				}
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
		
		Info->CommandPrefix = m_Info.CommandPrefix;
    }
}

HANDLE WrapPluginInfo::OpenW3(const OpenInfo *Info)
{
	LOG_CMD(L"OpenW(0x%08X)", (DWORD)Info->OpenFrom,0,0);
	HANDLE h = INVALID_HANDLE_VALUE;

	int nPluginItemNumber = 0;
	if (Info->Guid && memcmp(Info->Guid, &GUID_NULL, sizeof(GUID)) && mguids_PluginMenu)
	{
		for (int i = 0; i < mn_PluginMenu; i++)
		{
			if (memcmp(Info->Guid, mguids_PluginMenu+i, sizeof(GUID)) == 0)
			{
				nPluginItemNumber = i;
				break;
			}
		}
	}

	if ((Info->OpenFrom & OPEN_FROM_MASK) == OPEN_ANALYSE)
	{
		if (AnalyseW)
		{
			// В принципе, в Far2 была такая функция, так что если плаг ее экспортит - зовем
			h = OpenPluginW(OpenFrom_3_2(Info->OpenFrom), Info->Data);
			goto trap;
		}
		if (!mp_Analyze || !OpenFilePluginW)
			goto trap;
		h = OpenFilePluginW(mp_Analyze->FileName,
				(const unsigned char*)mp_Analyze->Buffer,
				mp_Analyze->BufferSize,
				OpMode_3_2(mp_Analyze->OpMode));
		goto trap;
	}
	else if (mp_Analyze)
	{
		free(mp_Analyze);
		mp_Analyze = NULL;
	}
	
	if ((Info->OpenFrom & OPEN_FROMMACRO_MASK))
	{
		if (m_Info.Reserved 
			&& (((Info->OpenFrom & OPEN_FROMMACRO_MASK) == OPEN_FROMMACRO)
				|| ((Info->OpenFrom & OPEN_FROMMACRO_MASK) == OPEN_FROMMACROSTRING)
				|| ((Info->OpenFrom & OPEN_FROMMACRO_MASK) == (OPEN_FROMMACRO|OPEN_FROMMACROSTRING)))
		    && OpenPluginW)
		{
			h = OpenPluginW(Far2::OPEN_FROMMACRO
							/*|(Info->OpenFrom & OPEN_FROMMACROSTRING)?0x20000:0*/,
							Info->Data);
		}
		goto trap;
	}

	if (OpenPluginW)
	{
		switch (Info->OpenFrom & OPEN_FROM_MASK)
		{
		case OPEN_LEFTDISKMENU:
		case OPEN_RIGHTDISKMENU:
			h = OpenPluginW(Far2::OPEN_DISKMENU, Info->Data); break;
		case OPEN_PLUGINSMENU:
			h = OpenPluginW(Far2::OPEN_PLUGINSMENU, Info->Data); break;
		case OPEN_FINDLIST:
			h = OpenPluginW(Far2::OPEN_FINDLIST, Info->Data); break;
		case OPEN_SHORTCUT:
			h = OpenPluginW(Far2::OPEN_SHORTCUT, Info->Data); break;
		case OPEN_COMMANDLINE:
			h = OpenPluginW(Far2::OPEN_COMMANDLINE, Info->Data); break;
		case OPEN_EDITOR:
			h = OpenPluginW(Far2::OPEN_EDITOR, Info->Data); break;
		case OPEN_VIEWER:
			h = OpenPluginW(Far2::OPEN_VIEWER, Info->Data); break;
		case OPEN_FILEPANEL:
			h = OpenPluginW(Far2::OPEN_FILEPANEL, Info->Data); break;
		case OPEN_DIALOG:
			{
				const OpenDlgPluginData* p3 = (OpenDlgPluginData*)Info->Data;
				Far2::OpenDlgPluginData p2 = {nPluginItemNumber};
				if (p3)
				{
					Far2Dialog* p = MapDlg_3_2[p3->hDlg];
					// Может быть NULL, если это диалог НЕ из этого плагина
					p2.hDlg = p ? ((HANDLE)p) : p3->hDlg;
				}
				h = OpenPluginW(Far2::OPEN_DIALOG, (INT_PTR)&p2);
			}
			break;
		}
	}
trap:
	return h;
}

int    WrapPluginInfo::AnalyseW3(const AnalyseInfo *Info)
{
	LOG_CMD(L"AnalyseW",0,0,0);
	if (AnalyseW)
		return AnalyseW(Info);
	if (mp_Analyze)
	{
		free(mp_Analyze);
		mp_Analyze = NULL;
	}
	if (!OpenFilePluginW)
		return FALSE;
	size_t nNewSize = sizeof(*mp_Analyze) + Info->BufferSize
			+ ((Info->FileName ? lstrlen(Info->FileName) : 0)+1)*sizeof(wchar_t);
	mp_Analyze = (AnalyseInfo*)malloc(nNewSize);
	if (!mp_Analyze)
		return FALSE;
	LPBYTE ptr = (LPBYTE)mp_Analyze;
	memmove(ptr, Info, sizeof(*mp_Analyze));
	ptr += sizeof(*mp_Analyze);
	if (Info->BufferSize && Info->Buffer)
	{
		memmove(ptr, Info->Buffer, Info->BufferSize);
		mp_Analyze->Buffer = ptr;
		ptr += Info->BufferSize;
	}
	else
	{
		mp_Analyze->Buffer = NULL;
	}
	if (Info->FileName)
	{
		lstrcpy((LPWSTR)ptr, Info->FileName);
		mp_Analyze->FileName = (LPCWSTR)ptr;
	}
	else
		mp_Analyze->FileName = NULL;
	
	HANDLE h = OpenFilePluginW(mp_Analyze->FileName,
		(const unsigned char*)mp_Analyze->Buffer,
		mp_Analyze->BufferSize,
		OpMode_3_2(Info->OpMode));
	if (h && h != INVALID_HANDLE_VALUE && ((INT_PTR)h) != -2)
	{
		if (m_AnalyzeMode == 1)
		{
			//TODO: Не закрывать. только вопрос - когда его можно будет закрыть, если OpenW НЕ будет вызван?
		}
	
		//TODO: Хорошо бы оптимизнуть, и не открывать файл два раза подряд, но непонятно
		//TODO: в какой момент можно закрыть Far2 плагин, если до OpenW дело не дошло...
		if (ClosePluginW)
			ClosePluginW(h);
		return TRUE;
	}
	if (mp_Analyze)
	{
		free(mp_Analyze);
		mp_Analyze = NULL;
	}
	return 0;
}
void   WrapPluginInfo::ClosePanelW3(HANDLE hPanel)
{
	LOG_CMD(L"ClosePanelW",0,0,0);
	if (ClosePluginW)
		ClosePluginW(hPanel);
}
int    WrapPluginInfo::CompareW3(const CompareInfo *Info)
{
	LOG_CMD(L"CompareW",0,0,0);
	int iRc = -2;
	if (CompareW)
	{
		Far2::PluginPanelItem Item1 = {{0}};
		Far2::PluginPanelItem Item2 = {{0}};
		PluginPanelItem_3_2(Info->Item1, &Item1);
		PluginPanelItem_3_2(Info->Item2, &Item2);
		iRc = CompareW(Info->hPanel, &Item1, &Item2, SortMode_3_2(Info->Mode));
	}
	return iRc;
}
int    WrapPluginInfo::ConfigureW3(const GUID* Guid)
{
	LOG_CMD(L"ConfigureW",0,0,0);
	int iRc = 0;
	if (ConfigureW)
	{
		if (mn_PluginConfig > 0 && mguids_PluginConfig)
		{
			for (int i = 0; i < mn_PluginConfig; i++)
			{
				if (memcmp(Guid, &GUID_NULL, sizeof(GUID)) == 0 ||
					memcmp(Guid, mguids_PluginConfig+i, sizeof(GUID)) == 0)
				{
					iRc = ConfigureW(i);
					break;
				}
			}
		}
	}
	return iRc;
}
int    WrapPluginInfo::DeleteFilesW3(const DeleteFilesInfo *Info)
{
	LOG_CMD(L"DeleteFilesW",0,0,0);
	int iRc = 0;
	if (DeleteFilesW)
	{
		Far2::PluginPanelItem *p2 = PluginPanelItems_3_2(Info->PanelItem, Info->ItemsNumber);
		if (p2)
		{
			iRc = DeleteFilesW(Info->hPanel, p2, Info->ItemsNumber, OpMode_3_2(Info->OpMode));
			free(p2);
		}
	}
	return iRc;
}
void   WrapPluginInfo::ExitFARW3(void)
{
	LOG_CMD(L"ExitFARW",0,0,0);
	if (ExitFARW)
		ExitFARW();
	UnloadPlugin();
	//// Если нужно в загрузчике снести (или восстановить) какие-то экспорты - самое время поставить его в очередь
	//if (mn_OldAbsentFunctions != mn_NewAbsentFunctions && *ms_PluginDll)
	//{
	//	WrapUpdateFunctions upd = {mn_OldAbsentFunctions, mn_NewAbsentFunctions};
	//	if (GetModuleFileName(mh_Loader, upd.ms_Loader, ARRAYSIZE(upd.ms_Loader)))
	//	{
	//		lstrcpy(upd.ms_IniFile, ms_IniFile);
	//		UpdateFunc.push_back(upd);
	//	}
	//}
	delete this;
}
void   WrapPluginInfo::FreeVirtualFindDataW3(const FreeFindDataInfo *Info)
{
	LOG_CMD(L"FreeVirtualFindDataW",0,0,0);
	//TODO:
}
int    WrapPluginInfo::GetFilesW3(GetFilesInfo *Info)
{
	LOG_CMD(L"GetFilesW",0,0,0);
	int iRc = 0;
	if (GetFilesW)
	{
		Far2::PluginPanelItem *p2 = PluginPanelItems_3_2(Info->PanelItem, Info->ItemsNumber);
		if (p2)
		{
			iRc = GetFilesW(Info->hPanel, p2, Info->ItemsNumber, Info->Move, &Info->DestPath, OpMode_3_2(Info->OpMode));
			free(p2);
		}
	}
	return iRc;
}
int    WrapPluginInfo::GetFindDataW3(GetFindDataInfo *Info)
{
	LOG_CMD(L"GetFindDataW",0,0,0);
	//GetFindDataW(HANDLE hPlugin,Far2::PluginPanelItem **pPanelItem,int *pItemsNumber,int OpMode);
	if (!GetFindDataW)
		return 0;
	Far2::PluginPanelItem *pPanelItem = NULL;
	int ItemsNumber = 0;
	int iRc = GetFindDataW(Info->hPanel, &pPanelItem, &ItemsNumber, OpMode_3_2(Info->OpMode));
	if (iRc && ItemsNumber > 0)
	{
		Info->PanelItem = PluginPanelItems_2_3(pPanelItem, ItemsNumber);
		Info->ItemsNumber = ItemsNumber;
		m_MapPanelItems[Info->PanelItem] = pPanelItem;
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
void   WrapPluginInfo::FreeFindDataW3(const FreeFindDataInfo *Info)
{
	LOG_CMD(L"FreeFindDataW",0,0,0);
	if (Info->PanelItem)
	{
		if (FreeFindDataW)
		{
			Far2::PluginPanelItem *pPanelItem = m_MapPanelItems[Info->PanelItem];
			if (pPanelItem)
				FreeFindDataW(Info->hPanel, pPanelItem, Info->ItemsNumber);
		}
		free(Info->PanelItem);
		
		m_MapPanelItems.erase(Info->PanelItem);
		//std::map<PluginPanelItem*,Far2::PluginPanelItem*>::iterator iter;
		//for (iter = MapPanelItems.begin( ); iter!=MapPanelItems.end( ); iter++)
		//{
		//	if (iter->first == Info->PanelItem)
		//	{
		//		MapPanelItems.erase(iter);
		//		break;
		//	}
		//}
	}
}
void   WrapPluginInfo::GetOpenPanelInfoW3(OpenPanelInfo *Info)
{
	LOG_CMD(L"GetOpenPluginInfoW",0,0,0);
	if (GetOpenPluginInfoW)
	{
		Far2::OpenPluginInfo ofi = {sizeof(Far2::OpenPluginInfo)};
		GetOpenPluginInfoW(Info->hPanel, &ofi);
		
		//memset(Info, 0, sizeof(*Info));
		//Info->StructSize = sizeof(*Info);
		//HANDLE hPanel;
		Info->Flags = OpenPanelInfoFlags_2_3(ofi.Flags);
		Info->HostFile = ofi.HostFile;
		Info->CurDir = ofi.CurDir;
		Info->Format = ofi.Format;
		Info->PanelTitle = ofi.PanelTitle;
		Info->InfoLines = InfoLines_2_3(ofi.InfoLines, ofi.InfoLinesNumber);
		Info->InfoLinesNumber = m_InfoLinesNumber;
		Info->DescrFiles = ofi.DescrFiles;
		Info->DescrFilesNumber = ofi.DescrFilesNumber;
		Info->PanelModesArray = PanelModes_2_3(ofi.PanelModesArray, ofi.PanelModesNumber);
		Info->PanelModesNumber = m_PanelModesNumber;
		Info->StartPanelMode = ofi.StartPanelMode;
		Info->StartSortMode = SortMode_2_3(ofi.StartSortMode);
		Info->StartSortOrder = ofi.StartSortOrder;
		Info->KeyBar = KeyBarTitles_2_3(ofi.KeyBar);
		Info->ShortcutData = ofi.ShortcutData;
		Info->FreeSize = 0;
	}
}
int    WrapPluginInfo::GetVirtualFindDataW3(GetVirtualFindDataInfo *Info)
{
	LOG_CMD(L"GetVirtualFindDataW",0,0,0);
	//TODO:
	return 0;
}
int    WrapPluginInfo::MakeDirectoryW3(MakeDirectoryInfo *Info)
{
	LOG_CMD(L"MakeDirectoryW",0,0,0);
	int iRc = 0;
	if (MakeDirectoryW)
		iRc = MakeDirectoryW(Info->hPanel, &Info->Name, OpMode_3_2(Info->OpMode));
	return iRc;
}
int    WrapPluginInfo::ProcessDialogEventW3(int Event,void *Param)
{
	LOG_CMD0(L"ProcessDialogEventW",0,0,0);
	int lRc = 0;
	if (ProcessDialogEventW)
	{
		Far2::DIALOG_EVENTS Event2 = Far2::DE_DLGPROCINIT;
		switch (Event)
		{
		case DE_DLGPROCINIT:
			Event2 = Far2::DE_DLGPROCINIT; break;
		case DE_DEFDLGPROCINIT:
			Event2 = Far2::DE_DEFDLGPROCINIT; break;
		case DE_DLGPROCEND:
			Event2 = Far2::DE_DLGPROCEND; break;
		default:
			return FALSE;
		}
		FarDialogEvent* p3 = (FarDialogEvent*)Param;
		Far2::FarDialogEvent p2 = {p3->hDlg, 0, p3->Param1, (LONG_PTR)p3->Param2, p3->Result};
		p2.Msg = FarMessage_3_2(p3->Msg, p2.Param1, (void*&)p2.Param2);
		if (p2.Msg != DM_FIRST)
		{
			lRc = ProcessDialogEventW(Event2, &p2);
			if (lRc)
				p3->Result = p2.Result;
		}
	}
	return lRc;
}
int    WrapPluginInfo::ProcessEditorEventW3(int Event,void *Param)
{
	LOG_CMD0(L"ProcessEditorEventW(%i)",Event,0,0);
	int iRc = 0;
	if (ProcessEditorEventW)
		iRc = ProcessEditorEventW(Event, Param);
	return iRc;
}
int    WrapPluginInfo::ProcessEditorInputW3(const ProcessEditorInputInfo *Info)
{
	LOG_CMD(L"ProcessEditorInputW",0,0,0);
	int iRc = 0;
	_ASSERTE(Info->StructSize==sizeof(*Info));
	if (ProcessEditorInputW)
		iRc = ProcessEditorInputW(&Info->Rec);
	return iRc;
}
int    WrapPluginInfo::ProcessEventW3(HANDLE hPanel,int Event,void *Param)
{
	LOG_CMD0(L"ProcessEventW(%i)",Event,0,0);
	int iRc = 0;
	if (ProcessEventW)
		iRc = ProcessEventW(hPanel, Event, Param);
	return iRc;
}
int    WrapPluginInfo::ProcessHostFileW3(const ProcessHostFileInfo *Info)
{
	LOG_CMD(L"ProcessHostFileW",0,0,0);
	int iRc = 0;
	if (ProcessHostFileW)
	{
		Far2::PluginPanelItem *p2 = PluginPanelItems_3_2(Info->PanelItem, Info->ItemsNumber);
		iRc = ProcessHostFileW(Info->hPanel, p2, Info->ItemsNumber, OpMode_3_2(Info->OpMode));
		if (p2)
			free(p2);
	}
	return iRc;
}
int    WrapPluginInfo::ProcessPanelInputW3(HANDLE hPanel,const struct ProcessPanelInputInfo *Info)
{
	LOG_CMD(L"ProcessPanelInputW(%s,0x%X,0x%X)",(Info->Rec.EventType==KEY_EVENT?L"Key":L"???"),Info->Rec.Event.KeyEvent.wVirtualKeyCode,Info->Rec.Event.KeyEvent.dwControlKeyState);
	_ASSERTE(Info->StructSize == sizeof(*Info));
	int iRc = 0;
	if (ProcessKeyW)
	{
		int Key2 = FarKey_3_2(&Info->Rec);
		DWORD FShift = Key2 & 0x7F000000; // старший бит используется в других целях!
		DWORD ControlState =
			(FShift & KEY_SHIFT ? Far2::PKF_SHIFT : 0)|
			(FShift & KEY_ALT ? Far2::PKF_ALT : 0)|
			(FShift & KEY_CTRL ? Far2::PKF_CONTROL : 0);
		//DWORD PreProcess = (Info->Flags & PKIF_PREPROCESS) ? Far2::PKF_PREPROCESS : 0;
		iRc = ProcessKeyW(hPanel, (Key2 & 0x0003FFFF) /*| PreProcess*/, ControlState);
	}
	return iRc;
}
int    WrapPluginInfo::ProcessConsoleInputW3(ProcessConsoleInputInfo *Info)
{
	//TODO: А здесь можно бы кинуть в Far2 то что раньше было PKF_PREPROCESS
	return 0;
}
int    WrapPluginInfo::ProcessSynchroEventW3(int Event,void *Param)
{
	LOG_CMD(L"ProcessSynchroEventW",0,0,0);
	int iRc = 0;
	if (ProcessSynchroEventW)
		iRc = ProcessSynchroEventW(Event, Param);
	return iRc;
}
int    WrapPluginInfo::ProcessViewerEventW3(int Event,void *Param)
{
	LOG_CMD(L"ProcessViewerEventW",0,0,0);
	int iRc = 0;
	if (ProcessViewerEventW)
		iRc = ProcessViewerEventW(Event, Param);
	return iRc;
}
int    WrapPluginInfo::PutFilesW3(const PutFilesInfo *Info)
{
	LOG_CMD(L"PutFilesW",0,0,0);
	int iRc = 0;
	if (PutFilesW)
	{
		Far2::PluginPanelItem *p2 = PluginPanelItems_3_2(Info->PanelItem, Info->ItemsNumber);
		if (p2)
		{
			if (!m_OldPutFilesParams)
				iRc = PutFilesW(Info->hPanel, p2, Info->ItemsNumber, Info->Move, Info->SrcPath, OpMode_3_2(Info->OpMode));
			else
			{
				wchar_t szOldPath[MAX_PATH]; szOldPath[0] = 0;
				if (Info->SrcPath)
				{
					GetCurrentDirectory(ARRAYSIZE(szOldPath), szOldPath);
					// Тут будет облом на "длинных" или "неправильных" путях
					SetCurrentDirectory(Info->SrcPath);
				}
				iRc = ((WrapPluginInfo::_PutFilesOldW)PutFilesW)(Info->hPanel, p2, Info->ItemsNumber, Info->Move, OpMode_3_2(Info->OpMode));
				if (szOldPath[0])
					SetCurrentDirectory(szOldPath);
			}
			free(p2);
		}
	}
	return iRc;
}
int    WrapPluginInfo::SetDirectoryW3(const SetDirectoryInfo *Info)
{
	int iRc = 0;
	LOG_CMD(L"SetDirectoryW",0,0,0);
	if (SetDirectoryW)
		iRc = SetDirectoryW(Info->hPanel, Info->Dir, OpMode_3_2(Info->OpMode));
	return iRc;
}
int    WrapPluginInfo::SetFindListW3(const SetFindListInfo *Info)
{
	LOG_CMD(L"SetFindListW(count=%u)",(DWORD)Info->ItemsNumber,0,0);
	int iRc = 0;
	if (SetFindListW)
	{
		Far2::PluginPanelItem *p2 = PluginPanelItems_3_2(Info->PanelItem, Info->ItemsNumber);
		if (p2)
		{
			iRc = SetFindListW(Info->hPanel, p2, Info->ItemsNumber);
			free(p2);
		}
	}
	return iRc;
}
int    WrapPluginInfo::GetCustomDataW3(const wchar_t *FilePath, wchar_t **CustomData)
{
	int iRc = 0;
	if (GetCustomDataW)
		iRc = GetCustomDataW(FilePath, CustomData);
	return iRc;
}
void   WrapPluginInfo::FreeCustomDataW3(wchar_t *CustomData)
{
	if (FreeCustomDataW)
		FreeCustomDataW(CustomData);
}


/* *** */
FARPROC WrapPluginInfo::GetProcAddressWrap(struct WrapPluginInfo* wpi, HMODULE hModule, LPCSTR lpProcName)
{
	return wpi->GetProcAddressW3(hModule, lpProcName);
}
FARPROC WrapPluginInfo::GetProcAddressW3(HMODULE hModule, LPCSTR lpProcName)
{
	if (!lstrcmpA(lpProcName, "AnalyseW"))
		if (!(mn_PluginFunctions & LF3_Analyse))
			return NULL;
	if (!lstrcmpA(lpProcName, "GetCustomDataW") || !lstrcmpA(lpProcName, "FreeCustomDataW"))
		if (!(mn_PluginFunctions & LF3_CustomData))
			return NULL;
	if (!lstrcmpA(lpProcName, "ProcessDialogEventW"))
		if (!(mn_PluginFunctions & LF3_Dialog))
			return NULL;
	if (!lstrcmpA(lpProcName, "ProcessEditorEventW") || !lstrcmpA(lpProcName, "ProcessEditorInputW"))
		if (!(mn_PluginFunctions & LF3_Editor))
			return NULL;
	if (!lstrcmpA(lpProcName, "SetFindListW"))
		if (!(mn_PluginFunctions & LF3_FindList))
			return NULL;
	if (!lstrcmpA(lpProcName, "ProcessViewerEventW"))
		if (!(mn_PluginFunctions & LF3_Viewer))
			return NULL;
	if (!lstrcmpA(lpProcName, "ProcessViewerEventW"))
		if (!(mn_PluginFunctions & LF3_Viewer))
			return NULL;
	
	// OK, можно
	return GetProcAddress(hModule, lpProcName);
}
/* *** */
void WrapPluginInfo::SetStartupInfoWrap(struct WrapPluginInfo* wpi, PluginStartupInfo *Info)
{
	wpi->SetStartupInfoW3(Info);
}
void WrapPluginInfo::GetGlobalInfoWrap(struct WrapPluginInfo* wpi, GlobalInfo *Info)
{
	wpi->GetGlobalInfoW3(Info);
}
void WrapPluginInfo::GetPluginInfoWrap(struct WrapPluginInfo* wpi, PluginInfo *Info)
{
	wpi->GetPluginInfoW3(Info);
}
HANDLE WrapPluginInfo::OpenWrap(struct WrapPluginInfo* wpi, const OpenInfo *Info)
{
	return wpi->OpenW3(Info);
}
int    WrapPluginInfo::AnalyseWrap(struct WrapPluginInfo* wpi, const AnalyseInfo *Info)
{
	return wpi->AnalyseW3(Info);
}
void   WrapPluginInfo::ClosePanelWrap(struct WrapPluginInfo* wpi, HANDLE hPanel)
{
	return wpi->ClosePanelW3(hPanel);
}
int    WrapPluginInfo::CompareWrap(struct WrapPluginInfo* wpi, const CompareInfo *Info)
{
	return wpi->CompareW3(Info);
}
int    WrapPluginInfo::ConfigureWrap(struct WrapPluginInfo* wpi, const GUID* Guid)
{
	return wpi->ConfigureW3(Guid);
}
int    WrapPluginInfo::DeleteFilesWrap(struct WrapPluginInfo* wpi, const DeleteFilesInfo *Info)
{
	return wpi->DeleteFilesW3(Info);
}
void   WrapPluginInfo::ExitFARWrap(struct WrapPluginInfo* wpi)
{
	return wpi->ExitFARW3();
}
void   WrapPluginInfo::FreeVirtualFindDataWrap(struct WrapPluginInfo* wpi, const FreeFindDataInfo *Info)
{
	return wpi->FreeVirtualFindDataW3(Info);
}
int    WrapPluginInfo::GetFilesWrap(struct WrapPluginInfo* wpi, GetFilesInfo *Info)
{
	return wpi->GetFilesW3(Info);
}
int    WrapPluginInfo::GetFindDataWrap(struct WrapPluginInfo* wpi, GetFindDataInfo *Info)
{
	return wpi->GetFindDataW3(Info);
}
void   WrapPluginInfo::FreeFindDataWrap(struct WrapPluginInfo* wpi, const FreeFindDataInfo *Info)
{
	return wpi->FreeFindDataW3(Info);
}
void   WrapPluginInfo::GetOpenPanelInfoWrap(struct WrapPluginInfo* wpi, OpenPanelInfo *Info)
{
	return wpi->GetOpenPanelInfoW3(Info);
}
int    WrapPluginInfo::GetVirtualFindDataWrap(struct WrapPluginInfo* wpi, GetVirtualFindDataInfo *Info)
{
	return wpi->GetVirtualFindDataW3(Info);
}
int    WrapPluginInfo::MakeDirectoryWrap(struct WrapPluginInfo* wpi, MakeDirectoryInfo *Info)
{
	return wpi->MakeDirectoryW3(Info);
}
int    WrapPluginInfo::ProcessDialogEventWrap(struct WrapPluginInfo* wpi, int Event,void *Param)
{
	return wpi->ProcessDialogEventW3(Event, Param);
}
int    WrapPluginInfo::ProcessEditorEventWrap(struct WrapPluginInfo* wpi, int Event,void *Param)
{
	return wpi->ProcessEditorEventW3(Event, Param);
}
int    WrapPluginInfo::ProcessEditorInputWrap(struct WrapPluginInfo* wpi, const ProcessEditorInputInfo *Info)
{
	return wpi->ProcessEditorInputW3(Info);
}
int    WrapPluginInfo::ProcessEventWrap(struct WrapPluginInfo* wpi, HANDLE hPanel,int Event,void *Param)
{
	return wpi->ProcessEventW3(hPanel, Event, Param);
}
int    WrapPluginInfo::ProcessHostFileWrap(struct WrapPluginInfo* wpi, const ProcessHostFileInfo *Info)
{
	return wpi->ProcessHostFileW3(Info);
}
int    WrapPluginInfo::ProcessPanelInputWrap(struct WrapPluginInfo* wpi, HANDLE hPanel,const struct ProcessPanelInputInfo *Info)
{
	return wpi->ProcessPanelInputW3(hPanel, Info);
}
int    WrapPluginInfo::ProcessConsoleInputWrap(struct WrapPluginInfo* wpi, ProcessConsoleInputInfo *Info)
{
	return wpi->ProcessConsoleInputW3(Info);
}
int    WrapPluginInfo::ProcessSynchroEventWrap(struct WrapPluginInfo* wpi, int Event,void *Param)
{
	return wpi->ProcessSynchroEventW3(Event, Param);
}
int    WrapPluginInfo::ProcessViewerEventWrap(struct WrapPluginInfo* wpi, int Event,void *Param)
{
	return wpi->ProcessViewerEventW3(Event, Param);
}
int    WrapPluginInfo::PutFilesWrap(struct WrapPluginInfo* wpi, const PutFilesInfo *Info)
{
	return wpi->PutFilesW3(Info);
}
int    WrapPluginInfo::SetDirectoryWrap(struct WrapPluginInfo* wpi, const SetDirectoryInfo *Info)
{
	return wpi->SetDirectoryW3(Info);
}
int    WrapPluginInfo::SetFindListWrap(struct WrapPluginInfo* wpi, const SetFindListInfo *Info)
{
	return wpi->SetFindListW3(Info);
}
int    WrapPluginInfo::GetCustomDataWrap(struct WrapPluginInfo* wpi, const wchar_t *FilePath, wchar_t **CustomData)
{
	return wpi->GetCustomDataW3(FilePath, CustomData);
}
void   WrapPluginInfo::FreeCustomDataWrap(struct WrapPluginInfo* wpi, wchar_t *CustomData)
{
	return wpi->FreeCustomDataW3(CustomData);
}
/* *** */
/* *** */
/* *** */
LONG_PTR WrapPluginInfo::FarApiDefDlgProcWrap(WrapPluginInfo* wpi, HANDLE hDlg, int Msg, int Param1, LONG_PTR Param2)
{
	return wpi->FarApiDefDlgProc(hDlg, Msg, Param1, Param2);
}
LONG_PTR WrapPluginInfo::FarApiSendDlgMessageWrap(WrapPluginInfo* wpi, HANDLE hDlg, int Msg, int Param1, LONG_PTR Param2)
{
	return wpi->FarApiSendDlgMessage(hDlg, Msg, Param1, Param2);
}
BOOL WrapPluginInfo::FarApiShowHelpWrap(WrapPluginInfo* wpi, const wchar_t *ModuleName, const wchar_t *Topic, DWORD Flags)
{
	return wpi->FarApiShowHelp(ModuleName, Topic, Flags);
}
HANDLE WrapPluginInfo::FarApiSaveScreenWrap(WrapPluginInfo* wpi, int X1, int Y1, int X2, int Y2)
{
	return wpi->FarApiSaveScreen(X1, Y1, X2, Y2);
}
void WrapPluginInfo::FarApiRestoreScreenWrap(WrapPluginInfo* wpi, HANDLE hScreen)
{
	wpi->FarApiRestoreScreen(hScreen);
}
void WrapPluginInfo::FarApiTextWrap(WrapPluginInfo* wpi, int X, int Y, int Color, const wchar_t *Str)
{
	wpi->FarApiText(X, Y, Color, Str);
}
int WrapPluginInfo::FarApiEditorWrap(WrapPluginInfo* wpi, const wchar_t *FileName, const wchar_t *Title, int X1, int Y1, int X2, int Y2, DWORD Flags, int StartLine, int StartChar, UINT CodePage)
{
	return wpi->FarApiEditor(FileName, Title, X1, Y1, X2, Y2, Flags, StartLine, StartChar, CodePage);
}
int WrapPluginInfo::FarApiViewerWrap(WrapPluginInfo* wpi, const wchar_t *FileName, const wchar_t *Title, int X1, int Y1, int X2, int Y2, DWORD Flags, UINT CodePage)
{
	return wpi->FarApiViewer(FileName, Title, X1, Y1, X2, Y2, Flags, CodePage);
}
int WrapPluginInfo::FarApiMenuWrap(WrapPluginInfo* wpi, INT_PTR PluginNumber, int X, int Y, int MaxHeight, DWORD Flags, const wchar_t* Title, const wchar_t* Bottom, const wchar_t* HelpTopic, const int* BreakKeys, int* BreakCode, const struct Far2::FarMenuItem *Item, int ItemsNumber)
{
	return wpi->FarApiMenu(PluginNumber, X, Y, MaxHeight, Flags, Title, Bottom, HelpTopic, BreakKeys, BreakCode, Item, ItemsNumber);
}
int WrapPluginInfo::FarApiMessageWrap(WrapPluginInfo* wpi, INT_PTR PluginNumber, DWORD Flags, const wchar_t *HelpTopic, const wchar_t * const *Items, int ItemsNumber, int ButtonsNumber)
{
	return wpi->FarApiMessage(PluginNumber, Flags, HelpTopic, Items, ItemsNumber, ButtonsNumber);
}
HANDLE WrapPluginInfo::FarApiDialogInitWrap(WrapPluginInfo* wpi, INT_PTR PluginNumber, int X1, int Y1, int X2, int Y2, const wchar_t* HelpTopic, struct Far2::FarDialogItem *Item, unsigned int ItemsNumber, DWORD Reserved, DWORD Flags, Far2::FARWINDOWPROC DlgProc, LONG_PTR Param)
{
	return wpi->FarApiDialogInit(PluginNumber, X1, Y1, X2, Y2, HelpTopic, Item, ItemsNumber, Reserved, Flags, DlgProc, Param);
}
int WrapPluginInfo::FarApiDialogRunWrap(WrapPluginInfo* wpi, HANDLE hDlg)
{
	return wpi->FarApiDialogRun(hDlg);
}
void WrapPluginInfo::FarApiDialogFreeWrap(WrapPluginInfo* wpi, HANDLE hDlg)
{
	wpi->FarApiDialogFree(hDlg);
}
int WrapPluginInfo::FarApiControlWrap(WrapPluginInfo* wpi, HANDLE hPlugin, int Command, int Param1, LONG_PTR Param2)
{
	return wpi->FarApiControl(hPlugin, Command, Param1, Param2);
}
int WrapPluginInfo::FarApiGetDirListWrap(WrapPluginInfo* wpi, const wchar_t *Dir, struct Far2::FAR_FIND_DATA **pPanelItem, int *pItemsNumber)
{
	return wpi->FarApiGetDirList(Dir, pPanelItem, pItemsNumber);
}
void WrapPluginInfo::FarApiFreeDirListWrap(WrapPluginInfo* wpi, struct Far2::FAR_FIND_DATA *PanelItem, int nItemsNumber)
{
	wpi->FarApiFreeDirList(PanelItem, nItemsNumber);
}
int WrapPluginInfo::FarApiGetPluginDirListWrap(WrapPluginInfo* wpi, INT_PTR PluginNumber, HANDLE hPlugin, const wchar_t *Dir, struct Far2::PluginPanelItem **pPanelItem, int *pItemsNumber)
{
	return wpi->FarApiGetPluginDirList(PluginNumber, hPlugin, Dir, pPanelItem, pItemsNumber);
}
void WrapPluginInfo::FarApiFreePluginDirListWrap(WrapPluginInfo* wpi, struct Far2::PluginPanelItem *PanelItem, int nItemsNumber)
{
	wpi->FarApiFreePluginDirList(PanelItem, nItemsNumber);
}
int WrapPluginInfo::FarApiCmpNameWrap(WrapPluginInfo* wpi, const wchar_t *Pattern, const wchar_t *String, int SkipPath)
{
	return wpi->FarApiCmpName(Pattern, String, SkipPath);
}
LPCWSTR WrapPluginInfo::FarApiGetMsgWrap(WrapPluginInfo* wpi, INT_PTR PluginNumber, int MsgId)
{
	return wpi->FarApiGetMsg(PluginNumber, MsgId);
}
INT_PTR WrapPluginInfo::FarApiAdvControlWrap(WrapPluginInfo* wpi, INT_PTR ModuleNumber, int Command, void *Param)
{
	return wpi->FarApiAdvControl(ModuleNumber, Command, Param);
}
int WrapPluginInfo::FarApiViewerControlWrap(WrapPluginInfo* wpi, int Command, void *Param)
{
	return wpi->FarApiViewerControl(Command, Param);
}
int WrapPluginInfo::FarApiEditorControlWrap(WrapPluginInfo* wpi, int Command, void *Param)
{
	return wpi->FarApiEditorControl(Command, Param);
}
int WrapPluginInfo::FarApiInputBoxWrap(WrapPluginInfo* wpi, const wchar_t *Title, const wchar_t *SubTitle, const wchar_t *HistoryName, const wchar_t *SrcText, wchar_t *DestText, int   DestLength, const wchar_t *HelpTopic, DWORD Flags)
{
	return wpi->FarApiInputBox(Title, SubTitle, HistoryName, SrcText, DestText, DestLength, HelpTopic, Flags);
}
int WrapPluginInfo::FarApiPluginsControlWrap(WrapPluginInfo* wpi, HANDLE hHandle, int Command, int Param1, LONG_PTR Param2)
{
	return wpi->FarApiPluginsControl(hHandle, Command, Param1, Param2);
}
int WrapPluginInfo::FarApiFileFilterControlWrap(WrapPluginInfo* wpi, HANDLE hHandle, int Command, int Param1, LONG_PTR Param2)
{
	return wpi->FarApiFileFilterControl(hHandle, Command, Param1, Param2);
}
int WrapPluginInfo::FarApiRegExpControlWrap(WrapPluginInfo* wpi, HANDLE hHandle, int Command, LONG_PTR Param)
{
	return wpi->FarApiRegExpControl(hHandle, Command, Param);
}
int WrapPluginInfo::FarStdGetFileOwnerWrap(WrapPluginInfo* wpi, const wchar_t *Computer,const wchar_t *Name,wchar_t *Owner,int Size)
{
	return wpi->FarStdGetFileOwner(Computer, Name, Owner, Size);
}
int WrapPluginInfo::FarStdGetPathRootWrap(WrapPluginInfo* wpi, const wchar_t *Path,wchar_t *Root, int DestSize)
{
	return wpi->FarStdGetPathRoot(Path, Root, DestSize);
}
wchar_t* WrapPluginInfo::FarStdXlatWrap(WrapPluginInfo* wpi, wchar_t *Line,int StartPos,int EndPos,DWORD Flags)
{
	return wpi->FarStdXlatW3(Line, StartPos, EndPos, Flags);
}
void WrapPluginInfo::FarStdRecursiveSearchWrap(WrapPluginInfo* wpi, const wchar_t *InitDir,const wchar_t *Mask,Far2::FRSUSERFUNC Func,DWORD Flags,void *Param)
{
	wpi->FarStdRecursiveSearch(InitDir, Mask, Func, Flags, Param);
}
int WrapPluginInfo::FarStdMkTempWrap(WrapPluginInfo* wpi, wchar_t *Dest, DWORD size, const wchar_t *Prefix)
{
	return wpi->FarStdMkTemp(Dest, size, Prefix);
}
int WrapPluginInfo::FarStdProcessNameWrap(WrapPluginInfo* wpi, const wchar_t *param1, wchar_t *param2, DWORD size, DWORD flags)
{
	return wpi->FarStdProcessName(param1, param2, size, flags);
}
int WrapPluginInfo::FarStdMkLinkWrap(WrapPluginInfo* wpi, const wchar_t *Src,const wchar_t *Dest,DWORD Flags)
{
	return wpi->FarStdMkLink(Src, Dest, Flags);
}
int WrapPluginInfo::FarConvertPathWrap(WrapPluginInfo* wpi, enum Far2::CONVERTPATHMODES Mode, const wchar_t *Src, wchar_t *Dest, int DestSize)
{
	return wpi->FarConvertPath(Mode, Src, Dest, DestSize);
}
int WrapPluginInfo::FarGetReparsePointInfoWrap(WrapPluginInfo* wpi, const wchar_t *Src, wchar_t *Dest,int DestSize)
{
	return wpi->FarGetReparsePointInfo(Src, Dest, DestSize);
}
DWORD WrapPluginInfo::FarGetCurrentDirectoryWrap(WrapPluginInfo* wpi, DWORD Size,wchar_t* Buffer)
{
	return wpi->FarGetCurrentDirectory(Size, Buffer);
}
/* *** */





Far2Dialog::Far2Dialog(WrapPluginInfo* pwpi,
	int X1, int Y1, int X2, int Y2,
    const wchar_t *HelpTopic, Far2::FarDialogItem *Items, UINT ItemsNumber,
    DWORD Flags, Far2::FARWINDOWPROC DlgProc, LONG_PTR Param,
    GUID PluginGuid, GUID DefGuid)
{
	wpi = pwpi;
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
	MapDlg_2_3.erase(this);
	if (hDlg3 != NULL)
	{
		wpi->psi3.DialogFree(hDlg3);
		MapDlg_3_2.erase(hDlg3);
		hDlg3 = NULL;
	}
}

INT_PTR Far2Dialog::Far3DlgProc(HANDLE hDlg, int Msg, int Param1, void* Param2)
{
	Far2Dialog* p = MapDlg_3_2[hDlg];
	INT_PTR lRc = 0;
	if (p && p->m_DlgProc)
	{
		void* OrgParam2 = Param2;
		Far2::FarMessagesProc Msg2 = p->wpi->FarMessage_3_2(Msg, Param1, Param2);
		_ASSERTE(Msg2!=Far2::DM_FIRST);
		if (Msg > DM_FIRST && Msg < DM_USER && Msg2 != Far2::DM_FIRST)
		{
			if (OrgParam2 && Param2)
			{
				p->wpi->gnMsg_3 = (FARMESSAGE)Msg;
				p->wpi->gnParam1_2 = p->wpi->gnParam1_3 = Param1;
				p->wpi->gpParam2_3 = OrgParam2;
				p->wpi->gnMsg_2 = Msg2;
				p->wpi->gnParam2_2 = (LONG_PTR)Param2;
			}
		}
		else
		{
			p->wpi->gnMsg_3 = DM_FIRST;
		}
		if (Msg == DM_KEY || Msg == DN_CONTROLINPUT)
			p->wpi->gnMsgKey_3 = (FARMESSAGE)Msg;
		if (Msg == DM_CLOSE || Msg == DN_CLOSE)
			p->wpi->gnMsgClose_3 = (FARMESSAGE)Msg;
		lRc = p->m_DlgProc((HANDLE)p, Msg2, Param1, (LONG_PTR)Param2);
		if (Msg == DM_KEY || Msg == DN_CONTROLINPUT)
			p->wpi->gnMsgKey_3 = DM_FIRST;
		if (Msg == DM_CLOSE || Msg == DN_CLOSE)
			p->wpi->gnMsgClose_3 = DM_FIRST;
		if (Param2 && Param2 != OrgParam2)
			p->wpi->FarMessageParam_2_3(Msg, Param1, Param2, OrgParam2, lRc);
	}
	else
	{
		lRc = p->wpi->psi3.DefDlgProc(hDlg, Msg, Param1, (void*)Param2);
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
			wpi->FarDialogItem_2_3(p2, p3, mp_ListInit3+i);
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
		wpi->m_LastFar2Dlg = this;
		FARDIALOGFLAGS Flags3 = 0
			| ((m_Flags & Far2::FDLG_WARNING) ? FDLG_WARNING : 0)
			| ((m_Flags & Far2::FDLG_SMALLDIALOG) ? FDLG_SMALLDIALOG : 0)
			| ((m_Flags & Far2::FDLG_NODRAWSHADOW) ? FDLG_NODRAWSHADOW : 0)
			| ((m_Flags & Far2::FDLG_NODRAWPANEL) ? FDLG_NODRAWPANEL : 0);
		hDlg3 = wpi->psi3.DialogInit(&m_PluginGuid, &m_Guid,
			m_X1, m_Y1, m_X2, m_Y2, m_HelpTopic, m_Items3, m_ItemsNumber, 0,
			Flags3, Far3DlgProc, (void*)m_Param);
		if (hDlg3)
		{
			MapDlg_2_3[this] = hDlg3;
			MapDlg_3_2[hDlg3] = this;
		}
	}
	
	if (hDlg3 != NULL)
	{
		wpi->m_LastFar2Dlg = this;
		iRc = wpi->psi3.DialogRun(hDlg3);

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
		//MapDlg_2_3.erase(this);
		//for (std::map<Far2Dialog*,HANDLE>::iterator i1 = MapDlg_2_3.begin();
		//	 i1 != MapDlg_2_3.end(); i++)
		//{
		//	if (i->first == this)
		//	{
		//		MapDlg_2_3.erase(i1);
		//		break;
		//	}
		//}
		//MapDlg_3_2.erase(hDlg3);
		//for (std::map<HANDLE,Far2Dialog*>::iterator i2 = MapDlg_3_2.begin();
		//	 i2 != MapDlg_3_2.end(); i++)
		//{
		//	if (i->first == hDlg3)
		//	{
		//		MapDlg_3_2.erase(i2);
		//		break;
		//	}
		//}
	}
	
	wpi->m_LastFar2Dlg = NULL;
	return iRc;
}



















typedef FARPROC (WINAPI* GetProcAddressT)(HMODULE hModule, LPCSTR lpProcName);
GetProcAddressT _GetProcAddress = NULL;
FARPROC WINAPI WrapGetProcAddress(HMODULE hModule, LPCSTR lpProcName)
{
	// If this parameter is an ordinal value, it must be in the low-order word;
	// the high-order word must be zero.
	if ( (((DWORD_PTR)lpProcName) & 0xFFFF) != ((DWORD_PTR)lpProcName) )
	{
		GetProcAddressT fn = (GetProcAddressT)GetProcAddress(hModule, "FarWrapGetProcAddress");
		if (fn)
			return fn(hModule, lpProcName);
	}
	
	// Если запрос не обработали - то обычным образом
	if (_GetProcAddress)
		return _GetProcAddress(hModule, lpProcName);
	_ASSERTE(_GetProcAddress!=NULL);
	return GetProcAddress(hModule, lpProcName);
}


bool SetProcAddressHook()
{
	#define GetPtrFromRVA(rva,pNTHeader,imageBase) (PVOID)((imageBase)+(rva))
	
	IMAGE_IMPORT_DESCRIPTOR* Import = 0;
	DWORD Size = 0;
	HMODULE Module = GetModuleHandle(0);

	if (!Module || (Module == INVALID_HANDLE_VALUE))
		return false;

	IMAGE_DOS_HEADER* dos_header = (IMAGE_DOS_HEADER*)Module;
	IMAGE_NT_HEADERS* nt_header = NULL;

	if (dos_header->e_magic == IMAGE_DOS_SIGNATURE /*'ZM'*/)
	{
		nt_header = (IMAGE_NT_HEADERS*)((char*)Module + dos_header->e_lfanew);

		if (nt_header->Signature != 0x004550)
			return false;
		else
		{
			Import = (IMAGE_IMPORT_DESCRIPTOR*)((char*)Module +
			                                    (DWORD)(nt_header->OptionalHeader.
			                                            DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].
			                                            VirtualAddress));
			Size = nt_header->OptionalHeader.
			       DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size;
		}
	}
	else
		return false;

	// if wrong module or no import table
	if (!Import)
		return false;

#ifdef _DEBUG
	PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(nt_header);
#endif
#ifdef _WIN64
	_ASSERTE(sizeof(DWORD_PTR)==8);
#else
	_ASSERTE(sizeof(DWORD_PTR)==4);
#endif
#ifdef _WIN64
#define TOP_SHIFT 60
#else
#define TOP_SHIFT 28
#endif


	bool bHooked = false;
	
	int nCount = Size / sizeof(IMAGE_IMPORT_DESCRIPTOR);
	//_ASSERTE(Size == (nCount * sizeof(IMAGE_IMPORT_DESCRIPTOR))); -- равно быть не обязано
	for(int i = 0; !bHooked && i < nCount; i++)
	{
		if (Import[i].Name == 0)
			break;

		//DebugString( ToTchar( (char*)Module + Import[i].Name ) );
#ifdef _DEBUG
		char* mod_name = (char*)Module + Import[i].Name;
#endif
		DWORD_PTR rvaINT = Import[i].OriginalFirstThunk;
		DWORD_PTR rvaIAT = Import[i].FirstThunk;

		if (rvaINT == 0)      // No Characteristics field?
		{
			// Yes! Gotta have a non-zero FirstThunk field then.
			rvaINT = rvaIAT;

			if (rvaINT == 0)       // No FirstThunk field?  Ooops!!!
			{
				_ASSERTE(rvaINT!=0);
				break;
			}
		}

		PIMAGE_IMPORT_BY_NAME pOrdinalNameO = NULL;
		IMAGE_THUNK_DATA* thunk = (IMAGE_THUNK_DATA*)GetPtrFromRVA(rvaIAT, nt_header, (PBYTE)Module);
		IMAGE_THUNK_DATA* thunkO = (IMAGE_THUNK_DATA*)GetPtrFromRVA(rvaINT, nt_header, (PBYTE)Module);

		if (!thunk ||  !thunkO)
		{
			_ASSERTE(thunk && thunkO);
			continue;
		}

		for (int f = 0; !bHooked && thunk->u1.Function; thunk++, thunkO++, f++)
		{
			const char* pszFuncName = NULL;
			ULONGLONG ordinalO = -1;

			if (thunk->u1.Function != thunkO->u1.Function)
			{
				// Ordinal у нас пока не используется
				if (IMAGE_SNAP_BY_ORDINAL(thunkO->u1.Ordinal))
				{
					//WARNING("Это НЕ ORDINAL, это Hint!!!");
					ordinalO = IMAGE_ORDINAL(thunkO->u1.Ordinal);
					pOrdinalNameO = NULL;
				}

				if (!IMAGE_SNAP_BY_ORDINAL(thunkO->u1.Ordinal))
				{
					pOrdinalNameO = (PIMAGE_IMPORT_BY_NAME)GetPtrFromRVA(thunkO->u1.AddressOfData, nt_header, (PBYTE)Module);
					//WARNING("Множественные вызовы IsBad???Ptr могут глючить");
					BOOL lbValidPtr = !IsBadReadPtr(pOrdinalNameO, sizeof(IMAGE_IMPORT_BY_NAME));
					_ASSERTE(lbValidPtr);

					if (lbValidPtr)
					{
						lbValidPtr = !IsBadStringPtrA((LPCSTR)pOrdinalNameO->Name, 10);
						_ASSERTE(lbValidPtr);

						if (lbValidPtr)
							pszFuncName = (LPCSTR)pOrdinalNameO->Name;
					}
				}
			}
			
			if (!pszFuncName)
				continue; // Если имя импорта определить не удалось - пропускаем
			else if (lstrcmpA(pszFuncName, "GetProcAddress"))
				continue;
			
			
			bHooked = true;
			DWORD old_protect = 0; DWORD dwErr = 0;

			if (thunk->u1.Function == (DWORD_PTR)WrapGetProcAddress)
			{
				// оказалось захучено в другой нити? такого быть не должно
				_ASSERTE(thunk->u1.Function != (DWORD_PTR)WrapGetProcAddress);
			}
			else
			{
				if (!VirtualProtect(&thunk->u1.Function, sizeof(thunk->u1.Function),
				                   PAGE_READWRITE, &old_protect))
				{
					dwErr = GetLastError();
					_ASSERTE(FALSE);
				}
				else
				{
					_GetProcAddress = (GetProcAddressT)thunk->u1.Function;
					thunk->u1.Function = (DWORD_PTR)WrapGetProcAddress;
					VirtualProtect(&thunk->u1.Function, sizeof(DWORD), old_protect, &old_protect);
				}
			}
		}
	}

	return bHooked;
}


// Плагин может быть вызван в первый раз из фоновой нити (диалог поиска при поиске в архивах)
// Поэтому простой "gnMainThreadId = GetCurrentThreadId();" не прокатит. Нужно искать первую нить процесса!
DWORD GetMainThreadId()
{
	DWORD nThreadID = 0;
	DWORD nProcID = GetCurrentProcessId();
	HANDLE h = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (h != INVALID_HANDLE_VALUE)
	{
		THREADENTRY32 ti = {sizeof(THREADENTRY32)};
		if (Thread32First(h, &ti))
		{
			do {
				// Нужно найти ПЕРВУЮ нить процесса
				if (ti.th32OwnerProcessID == nProcID) {
					nThreadID = ti.th32ThreadID;
					break;
				}
			} while (Thread32Next(h, &ti));
		}
		CloseHandle(h);
	}

	// Нехорошо. Должна быть найдена. Вернем хоть что-то (текущую нить)
	if (!nThreadID) {
		_ASSERTE(nThreadID!=0);
		nThreadID = GetCurrentThreadId();
	}
	return nThreadID;
}


BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
                     )
{
    if (ghFar3Wrap == NULL)
    {
		ghFar3Wrap = (HMODULE)hModule;
		gnMainThreadId = GetMainThreadId();
		
		// Хукнуть GetProcAddress, чтобы в фар не отдавать те функции, 
		// которых нет в оригинальном плагине. 
		// Это может создать лишнюю и весьма большую нагрузку.
		bool lbHooked = SetProcAddressHook();
		if (!lbHooked)
		{
			_ASSERTE(lbHooked==true);
		}

		//lbPsi2 = FALSE;
		//memset(&psi3, 0, sizeof(psi3));
		//memset(&psi2, 0, sizeof(psi2));
		//memset(&FSF3, 0, sizeof(FSF3));
		//memset(&FSF2, 0, sizeof(FSF2));

		//wpi = NULL;
		//LoadPluginInfo(); // может сейчас не будем, выполним в SetStartupInfoW?
    }
    if (ul_reason_for_call == DLL_PROCESS_DETACH)
    {
    	#if 0
		//TODO: !!! Если нужно в загрузчике снести (или восстановить) какие-то экспорты - самое время это выполнить
		std::vector<WrapUpdateFunctions>::iterator iter;
		#undef SET_EXP
		#define SET_EXP(v,n) { lstrcpyA(strChange[nIdx].OldName, n); lstrcpyA(strChange[nIdx].NewName, n); if (upd.mn_NewAbsentFunctions & v) strChange[nIdx].NewName[0] = '_'; else strChange[nIdx].OldName[0] = '_'; nIdx++; }
		for (iter = UpdateFunc.begin(); iter != UpdateFunc.end(); iter++)
		{
			WrapUpdateFunctions upd = *iter;
			HRSRC hRc = FindResource(ghFar3Wrap, (LPCTSTR)upd.nResourceID, _T("LOADERS"));
			if (hRc == NULL)
			{
				_ASSERTE(hRc != NULL);
			}
			else
			{
				DWORD nSize = SizeofResource(ghFar3Wrap, hRc);
				HGLOBAL hRes = LoadResource(ghFar3Wrap, hRc);
				if (hRes == NULL && nSize)
				{
					_ASSERTE(hRes != NULL && nSize);
				}
				else
				{
					LPVOID ptrLoader = LockResource(hRes);
					if (ptrLoader == NULL)
					{
						_ASSERTE(ptrLoader!=NULL);
					}
					else
					{
						HANDLE hFile = CreateFile(upd.ms_Loader, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, NULL,
							CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
						if ( hFile == INVALID_HANDLE_VALUE )
							continue;
						DWORD nWrite = 0;
						BOOL lbWrite = WriteFile(hFile, ptrLoader, nSize, &nWrite, NULL);
						_ASSERTE(lbWrite && nWrite == nSize);
						CloseHandle(hFile);
					}
				}
			}
			#if 0
			HANDLE hFile = CreateFile(upd.ms_Loader, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, NULL,
				OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
			if ( hFile == INVALID_HANDLE_VALUE )
				continue;
			HANDLE hFileMapping = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, 0, NULL);
			if (hFileMapping)
			{
				PBYTE pMappedFileBase = (PBYTE)MapViewOfFile(hFileMapping,FILE_MAP_READ|FILE_MAP_WRITE,0,0,0);
				if (pMappedFileBase)
				{
					ExportFunc strChange[64] = {{NULL}};
					int nIdx = 0;
					if ((upd.mn_OldAbsentFunctions & ALF3_Analyse) != (upd.mn_NewAbsentFunctions & ALF3_Analyse))
					{
						SET_EXP(ALF3_Analyse, "AnalyseW");
					}
					if ((upd.mn_OldAbsentFunctions & ALF3_Open) != (upd.mn_NewAbsentFunctions & ALF3_Open))
					{
						SET_EXP(ALF3_Open, "OpenW");
					}
					if ((upd.mn_OldAbsentFunctions & ALF3_Configure) != (upd.mn_NewAbsentFunctions & ALF3_Configure))
					{
						SET_EXP(ALF3_Configure, "ConfigureW");
					}
					if ((upd.mn_OldAbsentFunctions & ALF3_Compare) != (upd.mn_NewAbsentFunctions & ALF3_Compare))
					{
						SET_EXP(ALF3_Compare, "CompareW");
					}
					if ((upd.mn_OldAbsentFunctions & ALF3_GetFiles) != (upd.mn_NewAbsentFunctions & ALF3_GetFiles))
					{
						SET_EXP(ALF3_GetFiles, "GetFilesW");
					}
					if ((upd.mn_OldAbsentFunctions & ALF3_PutFiles) != (upd.mn_NewAbsentFunctions & ALF3_PutFiles))
					{
						SET_EXP(ALF3_PutFiles, "PutFilesW");
					}
					if ((upd.mn_OldAbsentFunctions & ALF3_FindData) != (upd.mn_NewAbsentFunctions & ALF3_FindData))
					{
						SET_EXP(ALF3_FindData, "GetFindDataW");
						SET_EXP(ALF3_FindData, "FreeFindDataW");
					}
					if ((upd.mn_OldAbsentFunctions & ALF3_VirtualFindData) != (upd.mn_NewAbsentFunctions & ALF3_VirtualFindData))
					{
						SET_EXP(ALF3_VirtualFindData, "GetVirtualFindDataW");
						SET_EXP(ALF3_VirtualFindData, "FreeVirtualFindDataW");
					}
					if ((upd.mn_OldAbsentFunctions & ALF3_ProcessHostFile) != (upd.mn_NewAbsentFunctions & ALF3_ProcessHostFile))
					{
						SET_EXP(ALF3_ProcessHostFile, "ProcessHostFileW");
					}
					if ((upd.mn_OldAbsentFunctions & ALF3_ProcessDialogEvent) != (upd.mn_NewAbsentFunctions & ALF3_ProcessDialogEvent))
					{
						SET_EXP(ALF3_ProcessDialogEvent, "ProcessDialogEventW");
					}
					if ((upd.mn_OldAbsentFunctions & ALF3_ProcessEditorEvent) != (upd.mn_NewAbsentFunctions & ALF3_ProcessEditorEvent))
					{
						SET_EXP(ALF3_ProcessEditorEvent, "ProcessEditorEventW");
					}
					if ((upd.mn_OldAbsentFunctions & ALF3_ProcessEditorInput) != (upd.mn_NewAbsentFunctions & ALF3_ProcessEditorInput))
					{
						SET_EXP(ALF3_ProcessEditorInput, "ProcessEditorInputW");
					}
					if ((upd.mn_OldAbsentFunctions & ALF3_ProcessViewerEvent) != (upd.mn_NewAbsentFunctions & ALF3_ProcessViewerEvent))
					{
						SET_EXP(ALF3_ProcessViewerEvent, "ProcessViewerEventW");
					}
					if ((upd.mn_OldAbsentFunctions & ALF3_SetFindList) != (upd.mn_NewAbsentFunctions & ALF3_SetFindList))
					{
						SET_EXP(ALF3_SetFindList, "SetFindListW");
					}
					if ((upd.mn_OldAbsentFunctions & ALF3_CustomData) != (upd.mn_NewAbsentFunctions & ALF3_CustomData))
					{
						SET_EXP(ALF3_CustomData, "GetCustomDataW");
						SET_EXP(ALF3_CustomData, "FreeCustomDataW");
					}
					ChangeExports(strChange, pMappedFileBase);
					wchar_t szNew[32];
					wsprintf(szNew, L"%i", upd.mn_NewAbsentFunctions);
					WritePrivateProfileString(L"Plugin", L"DisabledFunctions", szNew, upd.ms_IniFile);
				}
				CloseHandle(hFileMapping);
			}
			// Что-то не меняется LastWriteDate
			SYSTEMTIME st; GetSystemTime(&st);
			FILETIME ft; SystemTimeToFileTime(&st, &ft);
			SetFileTime(hFile, NULL, NULL, &ft);
			CloseHandle(hFile);
			#endif
		}
		#undef SET_EXP
		#endif
		return TRUE;
    	//if (wpi)
    	//{
    	//	delete wpi;
    	//	wpi = NULL;
    	//}
    }
    return TRUE;
}

int WINAPI InitPlugin(struct Far3WrapFunctions *pInfo2)
{
#ifdef _DEBUG
	_InitPlugin fDbg = InitPlugin; // Для проверки аргументов на этапе компиляции
#endif
	if (!pInfo2)
		return -1;
	if (pInfo2->StructSize != sizeof(*pInfo2))
	{
		_ASSERTE(pInfo2->StructSize == sizeof(*pInfo2));
		if (pInfo2->ErrorInfo && (pInfo2->ErrorInfoMax >= 255 && pInfo2->ErrorInfoMax <= 4096))
			wsprintf(pInfo2->ErrorInfo, L"Far3Wrap\nInitPlugin failed. Invalid value of pInfo2->StructSize\nRequired: %u, received: %u", (DWORD)sizeof(*pInfo2), (DWORD)pInfo2->StructSize);
		return -2;
	}
	if (pInfo2->Far3Build != MVV_3)
	{
		_ASSERTE(pInfo2->Far3Build == MVV_3);
		if (pInfo2->ErrorInfo && (pInfo2->ErrorInfoMax >= 255 && pInfo2->ErrorInfoMax <= 4096))
			wsprintf(pInfo2->ErrorInfo, L"Far3Wrap\nInitPlugin failed. Invalid value of pInfo2->Far3Build\nRequired: %u, received: %u", MVV_3, pInfo2->Far3Build);
		return -3;
	}

	_ASSERTE(ghFar3Wrap==NULL || ghFar3Wrap==pInfo2->hFar3Wrap);
	if (!gnMainThreadId)
		gnMainThreadId = GetMainThreadId();
	ghFar3Wrap = pInfo2->hFar3Wrap;
	
	pInfo2->wpi = new WrapPluginInfo(pInfo2);

	pInfo2->wpi->LoadPluginInfo();
	if (!pInfo2->wpi->LoadPlugin(TRUE/*abSilent*/))
	{
		_ASSERTE(pInfo2->wpi->mh_Dll!=NULL);
		delete pInfo2->wpi;
		pInfo2->wpi = NULL;
		return -4;
	}

	pInfo2->PluginGuid = &pInfo2->wpi->mguid_Plugin;

	// Вернуть в Loader список функций враппера
	#undef SET_FN
	#define SET_FN(n) pInfo2->n = WrapPluginInfo::n;
	SET_FN(GetProcAddressWrap);
	SET_FN(SetStartupInfoWrap);
	SET_FN(GetGlobalInfoWrap);
	SET_FN(GetPluginInfoWrap);
	SET_FN(OpenWrap);
	SET_FN(AnalyseWrap);
	SET_FN(ClosePanelWrap);
	SET_FN(CompareWrap);
	SET_FN(ConfigureWrap);
	SET_FN(DeleteFilesWrap);
	SET_FN(ExitFARWrap);
	SET_FN(FreeVirtualFindDataWrap);
	SET_FN(GetFilesWrap);
	SET_FN(GetFindDataWrap);
	SET_FN(FreeFindDataWrap);
	SET_FN(GetOpenPanelInfoWrap);
	SET_FN(GetVirtualFindDataWrap);
	SET_FN(MakeDirectoryWrap);
	SET_FN(ProcessDialogEventWrap);
	SET_FN(ProcessEditorEventWrap);
	SET_FN(ProcessEditorInputWrap);
	SET_FN(ProcessEventWrap);
	SET_FN(ProcessHostFileWrap);
	SET_FN(ProcessPanelInputWrap);
	SET_FN(ProcessConsoleInputWrap);
	SET_FN(ProcessSynchroEventWrap);
	SET_FN(ProcessViewerEventWrap);
	SET_FN(PutFilesWrap);
	SET_FN(SetDirectoryWrap);
	SET_FN(SetFindListWrap);
	SET_FN(GetCustomDataWrap);
	SET_FN(FreeCustomDataWrap);
	//
	SET_FN(FarApiDefDlgProcWrap);
	SET_FN(FarApiSendDlgMessageWrap);
	SET_FN(FarApiShowHelpWrap);
	SET_FN(FarApiSaveScreenWrap);
	SET_FN(FarApiRestoreScreenWrap);
	SET_FN(FarApiTextWrap);
	SET_FN(FarApiEditorWrap);
	SET_FN(FarApiViewerWrap);
	SET_FN(FarApiMenuWrap);
	SET_FN(FarApiMessageWrap);
	SET_FN(FarApiDialogInitWrap);
	SET_FN(FarApiDialogRunWrap);
	SET_FN(FarApiDialogFreeWrap);
	SET_FN(FarApiControlWrap);
	SET_FN(FarApiGetDirListWrap);
	SET_FN(FarApiFreeDirListWrap);
	SET_FN(FarApiGetPluginDirListWrap);
	SET_FN(FarApiFreePluginDirListWrap);
	SET_FN(FarApiCmpNameWrap);
	SET_FN(FarApiGetMsgWrap);
	SET_FN(FarApiAdvControlWrap);
	SET_FN(FarApiViewerControlWrap);
	SET_FN(FarApiEditorControlWrap);
	SET_FN(FarApiInputBoxWrap);
	SET_FN(FarApiPluginsControlWrap);
	SET_FN(FarApiFileFilterControlWrap);
	SET_FN(FarApiRegExpControlWrap);
	SET_FN(FarStdGetFileOwnerWrap);
	SET_FN(FarStdGetPathRootWrap);
	SET_FN(FarStdXlatWrap);
	SET_FN(FarStdRecursiveSearchWrap);
	SET_FN(FarStdMkTempWrap);
	SET_FN(FarStdProcessNameWrap);
	SET_FN(FarStdMkLinkWrap);
	SET_FN(FarConvertPathWrap);
	SET_FN(FarGetReparsePointInfoWrap);
	SET_FN(FarGetCurrentDirectoryWrap);
	#undef SET_FN

	return 0;
}
