
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
#include <TCHAR.h>
#include <malloc.h>
#include <rpc.h>
#include "Version.h"

// Far2 & Far3 plugin API's
#define _FAR_NO_NAMELESS_UNIONS

namespace Far2
{
#undef __PLUGIN_HPP__
#include "pluginW.hpp"
//#undef __FARKEYS_HPP__
//#include "farkeys.hpp"
};

//namespace Far3
//{
#undef __PLUGIN_HPP__
#if MVV_3<=2102
	#include "pluginW3#2098.hpp"
#elif MVV_3<=2124
	#include "pluginW3#2124.hpp"
#elif MVV_3<=2172
	#include "pluginW3#2163.hpp"
#elif MVV_3<=2188
	#include "pluginW3#2184.hpp"
#elif MVV_3<=2203
	#include "pluginW3#2194.hpp"
#elif MVV_3<=2342
	#include "pluginW3#2342.hpp"
#elif MVV_3<=2375
	#include "pluginW3#2375.hpp"
#elif MVV_3<=2400 // условно, где поломалось - искать лень
	#include "pluginW3#2400.hpp"
#elif MVV_3<=2426
	#include "pluginW3#2426.hpp"
#elif MVV_3<=2457
	#include "pluginW3#2457.hpp"
#elif MVV_3<=2461
	#include "pluginW3#2461.hpp"
#elif MVV_3<=2540
	#include "pluginW3#2540.hpp"
#elif MVV_3<=2566
	#include "pluginW3#2566.hpp"
#else
	#include "pluginW3.hpp"
#endif
//#undef __FARKEYS_HPP__
//#include "farkeys3.hpp"
//};

#include "Far3Wrap.h"
#include "version.h"

HMODULE ghInstance = NULL;
HMODULE ghWrapper = NULL;
Far3WrapFunctions fwf;
PluginStartupInfo psi;
//typedef int (WINAPI* _InitPlugin)(struct Far3WrapFunctions *Info);
_InitPlugin InitPlugin = NULL;
wchar_t* gsErrInfo = NULL;
bool gbFailReported = false;
GUID guidPlugin = {0};
wchar_t* gsModuleFail = NULL;
#define ERRINFO_SIZE 2048
#define ERRORTITLE_SIZE 255

#if MVV_3>=2103
	#define WrapGuids(g) &g, &g
#else
	#define WrapGuids(g) &g
#endif

BOOL gbLoadWrapperCalled = FALSE;
wchar_t gszWrapper[MAX_PATH];
BOOL LoadWrapper(LPCWSTR asModule);

#define DO_NOT_UNLOAD_WRAP

#ifndef _ASSERTE
#define _ASSERTE(x)
#endif

BOOL APIENTRY DllMain( HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved )
{
    if (ghInstance == NULL)
    {
		ghInstance = (HMODULE)hModule;
		memset(&psi, 0, sizeof(psi));
		memset(&fwf, 0, sizeof(fwf));
		gbFailReported = false;
		
		//wchar_t szWrapper[MAX_PATH];
		int nTry = 0;
		DWORD i;
		gsErrInfo = (wchar_t*)calloc(ERRINFO_SIZE,sizeof(wchar_t));
		gsModuleFail = (wchar_t*)calloc(ERRORTITLE_SIZE,sizeof(wchar_t));

		_ASSERTE(ghWrapper==NULL);
		ghWrapper = NULL;
		_ASSERTE(gbLoadWrapperCalled==FALSE);
		gbLoadWrapperCalled = FALSE;

		while (ghWrapper == NULL)
		{
			gszWrapper[0] = 0;
			wchar_t* pszSlash;
			switch (nTry++)
			{
				case 0: // Сначала в папке вместе с "Loader.dll"
					i = GetModuleFileName(ghInstance, gszWrapper, ARRAYSIZE(gszWrapper)-28);
					if (i < 10 || i >= (ARRAYSIZE(gszWrapper)-28))
						continue;
					if (!(pszSlash = wcsrchr(gszWrapper, L'\\'))) continue;
					pszSlash[1] = 0;
					break;
				case 1: // Папка, указанная в %Far3WrapperPath%
					i = GetEnvironmentVariable(L"Far3WrapperPath", gszWrapper, ARRAYSIZE(gszWrapper));
					if (i < 10 || i >= ARRAYSIZE(gszWrapper))
						continue;
					break;
				case 2: // %FARHOME%\Plugins
					i = GetEnvironmentVariable(L"FARHOME", gszWrapper, ARRAYSIZE(gszWrapper)-28);
					if (i < 10 || i >= (ARRAYSIZE(gszWrapper)-28))
						continue;
					if (gszWrapper[i-1] != L'\\')
					{
						gszWrapper[i++] = L'\\'; gszWrapper[i] = 0;
					}
					lstrcat(gszWrapper, L"Plugins");
					break;
				case 3: // %FARHOME%
					i = GetEnvironmentVariable(L"FARHOME", gszWrapper, ARRAYSIZE(gszWrapper)-28);
					if (i < 10 || i >= (ARRAYSIZE(gszWrapper)-28))
						continue;
					break;
				case 4: // аналогично %FARHOME%\Plugins, но вдруг переменную окружения порушили?
					i = GetModuleFileName(NULL, gszWrapper, ARRAYSIZE(gszWrapper)-28);
					if (i < 10 || i >= (ARRAYSIZE(gszWrapper)-28))
						continue;
					if (gszWrapper[i-1] != L'\\')
					{
						gszWrapper[i++] = L'\\'; gszWrapper[i] = 0;
					}
					lstrcat(gszWrapper, L"Plugins");
					break;
				case 5: // аналогично %FARHOME%, но вдруг переменную окружения порушили?
					i = GetModuleFileName(NULL, gszWrapper, ARRAYSIZE(gszWrapper)-28);
					if (i < 10 || i >= (ARRAYSIZE(gszWrapper)-28))
						continue;
					if (!(pszSlash = wcsrchr(gszWrapper, L'\\'))) continue;
					pszSlash[1] = 0;
					break;
				default:
					GetModuleFileName(ghInstance, gszWrapper, ARRAYSIZE(gszWrapper));
					wsprintf(gsErrInfo, L"Far3Wrap\nWrapper module NOT found (Far3Wrap.dll):\n%s", gszWrapper);
					return TRUE; // -- возвращаем всегда TRUE, а то ошибка глупая отображается
			}
			
			if (*gszWrapper)
			{
				i = lstrlen(gszWrapper);
				if (gszWrapper[i-1] != L'\\')
				{
					gszWrapper[i++] = L'\\'; gszWrapper[i] = 0;
				}
				#ifdef _WIN64
				lstrcat(gszWrapper, L"Far3Wrap64.dll");
				#else
				lstrcat(gszWrapper, L"Far3Wrap.dll");
				#endif

				HANDLE h = CreateFile(gszWrapper, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, NULL);
				if (h && h != INVALID_HANDLE_VALUE)
				{
					CloseHandle(h);
					break; // Нашли
				}
				//if (LoadWrapper(szWrapper))
				//	break; // OK
			}
		}
		//if (!ghWrapper) -- возвращаем всегда TRUE, а то ошибка глупая отображается
		//	return FALSE;
    }
    if (ul_reason_for_call == DLL_PROCESS_DETACH)
    {
		// не будем выгружать. Основной код может пытаться подправить экспорты загрузчика
		#ifndef DO_NOT_UNLOAD_WRAP
    	if (ghWrapper)
    	{
    		FreeLibrary(ghWrapper);
    		ghWrapper = NULL;
    	}
		#endif
		if (gsErrInfo)
		{
			free(gsErrInfo);
			gsErrInfo = NULL;
		}
    }
    return TRUE;
}

#if defined(CRTSTARTUP)

#ifdef __cplusplus
extern "C"{
#endif
  BOOL WINAPI _DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved);
#ifdef __cplusplus
};
#endif

BOOL WINAPI _DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved)
{
  return DllMain(hDll, dwReason,lpReserved);
}
#endif

void ReportWrapperFail(bool bForce = false)
{
	if (gbFailReported && !bForce)
		return;
	gbFailReported = true; // чтобы опять не свалиться
	if (gsErrInfo && *gsErrInfo)
	{
		if (psi.Message && guidPlugin.Data1)
			psi.Message(WrapGuids(guidPlugin), FMSG_WARNING|FMSG_ALLINONE|FMSG_MB_OK, NULL,
				(const wchar_t * const *)gsErrInfo, 0,0);
		else
			MessageBox(NULL, gsErrInfo, L"Far3Wrap", MB_ICONSTOP|MB_OK|MB_SYSTEMMODAL);
	}
}

#define CHECKFN(n) if (!fwf.n) { ReportWrapperFail(); return; }
#define CHECKFNRET(n,r) if (!fwf.n) { ReportWrapperFail(); return r; }
#define CHECKFNRETF(n,r) if (!fwf.n) { ReportWrapperFail(true); return r; }


FARPROC WINAPI FarWrapGetProcAddress(HMODULE hModule, LPCSTR lpProcName)
{
	if (!gbLoadWrapperCalled)
		LoadWrapper(gszWrapper);
	if (fwf.GetProcAddressWrap)
		return fwf.GetProcAddressWrap(fwf.wpi, hModule, lpProcName);
	//_ASSERTE(fwf.GetProcAddressWrap!=NULL);
	return GetProcAddress(hModule, lpProcName);
}

int WINAPI GetMinFarVersionW(void)
{
	if (!gbLoadWrapperCalled)
		LoadWrapper(gszWrapper);
	#define MAKEFARVERSION2(major,minor,build) ( ((major)<<8) | (minor) | ((build)<<16))
	return MAKEFARVERSION2(3,0,MVV_3);
}

void WINAPI SetStartupInfoW(PluginStartupInfo *Info)
{
	if (!gbLoadWrapperCalled)
		LoadWrapper(gszWrapper);
	psi = *Info;
	// Ругаться сразу в SetStartupInfoW наверное некошерно?
	if (fwf.SetStartupInfoWrap)
		fwf.SetStartupInfoWrap(fwf.wpi, Info);
}

LPCWSTR GetModuleTitle(LPCWSTR asInfo = NULL, DWORD anError = 0, BOOL abForce = FALSE)
{
	// По идее, должно было быть заполнено в Far3wrap.dll
	if (abForce || !*gsModuleFail)
	{
		wchar_t sModule[MAX_PATH+1] = {0};
		GetModuleFileName(ghInstance, sModule, ARRAYSIZE(sModule));
		//wchar_t* pszSlash = wcsrchr(sModule, L'\\');
		//lstrcpyn(gsModuleFail, pszSlash ? pszSlash : *sModule ? sModule : L"<Far3wrap>", ARRAYSIZE(gsModuleFail));
		int nLen = lstrlen(sModule);
		int nMaxLen = ERRORTITLE_SIZE - (asInfo ? lstrlen(asInfo) : 0) - 20;
		wsprintf(gsModuleFail, 
			!asInfo ? L"<%s>" : !anError ? L"<%s> %s" : L"<%s> %s x%X",
			*sModule
				? ((nLen < ERRORTITLE_SIZE) ? sModule : (sModule + nLen - ERRORTITLE_SIZE + 1) )
				: L"[Far3wrap]",
			asInfo ? asInfo : L"", anError);
	}
	return gsModuleFail;
}

void WINAPI GetGlobalInfoW(GlobalInfo *Info)
{
	if (!gbLoadWrapperCalled)
		LoadWrapper(gszWrapper);
	if (fwf.GetGlobalInfoWrap)
	{
		fwf.GetGlobalInfoWrap(fwf.wpi, Info);
	}
	else
	{
		// Исключительная ситуация, не удалось загрузить Far3wrap.dll
		Info->StructSize = sizeof(GlobalInfo);
		Info->MinFarVersion = FARMANAGERVERSION;
		//static wchar_t sModule[MAX_PATH];
		//GetModuleFileName(ghInstance, sModule, ARRAYSIZE(sModule));
		Info->Version = MAKEFARVERSION(MVV_1,MVV_2,0,MVV_3,VS_RC);
		//TODO: А вот здесь, можно было бы генерить гуид по полному пути файла?
		if (!guidPlugin.Data1)
		{
			UuidCreate(&guidPlugin);
		}
		Info->Guid = guidPlugin;
		Info->Title = GetModuleTitle(); //sModule;
		Info->Description = L"Far2->Far3 wrapper";
		Info->Author = L"Maximus5";
	}
}

void WINAPI GetPluginInfoW(PluginInfo *Info)
{
	if (!gbLoadWrapperCalled)
		LoadWrapper(gszWrapper);
	if (fwf.GetPluginInfoWrap)
	{
		fwf.GetPluginInfoWrap(fwf.wpi, Info);
	}
	else
	{
		// Исключительная ситуация, не удалось загрузить Far3wrap.dll
		Info->StructSize = sizeof(*Info);
		static LPCWSTR sMenu[1];
		//static wchar_t sModule[MAX_PATH];
		//TODO: А вот здесь, можно было бы генерить гуид по полному пути файла?
		if (!guidPlugin.Data1)
			UuidCreate(&guidPlugin);
		//GetModuleFileName(ghInstance, sModule, ARRAYSIZE(sModule));
		sMenu[0] = GetModuleTitle(); //wcsrchr(sModule, L'\\');
		Info->PluginMenu.Count = 1;
		Info->PluginMenu.Guids = &guidPlugin;
		Info->PluginMenu.Strings = sMenu;
	}
}

HANDLE WINAPI OpenW(const OpenInfo *Info)
{
	if (!gbLoadWrapperCalled)
		LoadWrapper(gszWrapper);
	CHECKFNRETF(OpenWrap,INVALID_HANDLE_VALUE);
	return fwf.OpenWrap(fwf.wpi, Info);
}

HANDLE WINAPI AnalyseW(const AnalyseInfo *Info)
{
	if (!gbLoadWrapperCalled)
		LoadWrapper(gszWrapper);
	CHECKFNRET(AnalyseWrap,0);
	return fwf.AnalyseWrap(fwf.wpi, Info);
}

void   WINAPI CloseAnalyseW(const struct CloseAnalyseInfo *Info)
{
	if (!gbLoadWrapperCalled)
		LoadWrapper(gszWrapper);
	CHECKFN(CloseAnalyseWrap);
	fwf.CloseAnalyseWrap(fwf.wpi, Info);
}

void   WINAPI ClosePanelW(const struct ClosePanelInfo *Info)
{
	if (fwf.ClosePanelWrap)
		fwf.ClosePanelWrap(fwf.wpi, Info);
}

int    WINAPI CompareW(const CompareInfo *Info)
{
	if (fwf.CompareWrap)
		return fwf.CompareWrap(fwf.wpi, Info);
	return -2;
}

int    WINAPI ConfigureW(const struct ConfigureInfo *Info)
{
	if (!gbLoadWrapperCalled)
		LoadWrapper(gszWrapper);
	CHECKFNRET(ConfigureWrap,0);
	return fwf.ConfigureWrap(fwf.wpi, Info);
}

int    WINAPI DeleteFilesW(const DeleteFilesInfo *Info)
{
	if (fwf.DeleteFilesWrap)
		return fwf.DeleteFilesWrap(fwf.wpi, Info);
	return 0;
}

void   WINAPI ExitFARW(const struct ExitInfo *Info)
{
	if (fwf.ExitFARWrap)
	{
		_ExitFARWrap ExitFARWrap = fwf.ExitFARWrap;
		WrapPluginInfo* wpi = fwf.wpi;
		// Чтобы не было шансов позвать функции враппера из другой нити или после завершения fwf.ExitFARWrap(fwf.wpi);
		memset(&fwf, 0, sizeof(fwf));
		ExitFARWrap(wpi, Info);
	}
	else
		memset(&fwf, 0, sizeof(fwf));
}

void   WINAPI FreeVirtualFindDataW(const FreeFindDataInfo *Info)
{
	if (fwf.FreeVirtualFindDataWrap)
		fwf.FreeVirtualFindDataWrap(fwf.wpi, Info);
}

int    WINAPI GetFilesW(GetFilesInfo *Info)
{
	if (fwf.GetFilesWrap)
		return fwf.GetFilesWrap(fwf.wpi, Info);
	return 0;
}

int    WINAPI GetFindDataW(GetFindDataInfo *Info)
{
	if (fwf.GetFindDataWrap)
		return fwf.GetFindDataWrap(fwf.wpi, Info);
	return 0;
}

void   WINAPI FreeFindDataW(const FreeFindDataInfo *Info)
{
	if (fwf.FreeFindDataWrap)
		fwf.FreeFindDataWrap(fwf.wpi, Info);
}

void   WINAPI GetOpenPanelInfoW(OpenPanelInfo *Info)
{
	if (fwf.GetOpenPanelInfoWrap)
		fwf.GetOpenPanelInfoWrap(fwf.wpi, Info);
}

int    WINAPI GetVirtualFindDataW(GetVirtualFindDataInfo *Info)
{
	if (fwf.GetVirtualFindDataWrap)
		return fwf.GetVirtualFindDataWrap(fwf.wpi, Info);
	return 0;
}

int    WINAPI MakeDirectoryW(MakeDirectoryInfo *Info)
{
	if (fwf.MakeDirectoryWrap)
		return fwf.MakeDirectoryWrap(fwf.wpi, Info);
	return 0;
}

int    WINAPI ProcessDialogEventW(const struct ProcessDialogEventInfo *Info)
{
	if (!gbLoadWrapperCalled)
		LoadWrapper(gszWrapper);
	if (fwf.ProcessDialogEventWrap)
		return fwf.ProcessDialogEventWrap(fwf.wpi, Info);
	return 0;
}

int    WINAPI ProcessEditorEventW(const struct ProcessEditorEventInfo *Info)
{
	if (!gbLoadWrapperCalled)
		LoadWrapper(gszWrapper);
	if (fwf.ProcessEditorEventWrap)
		return fwf.ProcessEditorEventWrap(fwf.wpi, Info);
	return 0;
}

int    WINAPI ProcessEditorInputW(const ProcessEditorInputInfo *Info)
{
	if (!gbLoadWrapperCalled)
		LoadWrapper(gszWrapper);
	if (fwf.ProcessEditorInputWrap)
		return fwf.ProcessEditorInputWrap(fwf.wpi, Info);
	return 0;
}

int    WINAPI ProcessPanelEventW(const struct ProcessPanelEventInfo *Info)
{
	if (fwf.ProcessPanelEventWrap)
		return fwf.ProcessPanelEventWrap(fwf.wpi, Info);
	return 0;
}

int    WINAPI ProcessHostFileW(const ProcessHostFileInfo *Info)
{
	if (fwf.ProcessHostFileWrap)
		return fwf.ProcessHostFileWrap(fwf.wpi, Info);
	return 0;
}

int    WINAPI ProcessPanelInputW(const struct ProcessPanelInputInfo *Info)
{
	if (fwf.ProcessPanelInputWrap)
		return fwf.ProcessPanelInputWrap(fwf.wpi, Info);
	return 0;
}

int    WINAPI ProcessConsoleInputW(struct ProcessConsoleInputInfo *Info)
{
	if (!gbLoadWrapperCalled)
		LoadWrapper(gszWrapper);
	if (fwf.ProcessConsoleInputWrap)
		return fwf.ProcessConsoleInputWrap(fwf.wpi, Info);
	return 0;
}

int    WINAPI ProcessSynchroEventW(const struct ProcessSynchroEventInfo *Info)
{
	if (!gbLoadWrapperCalled)
		LoadWrapper(gszWrapper);
	if (fwf.ProcessSynchroEventWrap)
		return fwf.ProcessSynchroEventWrap(fwf.wpi, Info);
	return 0;
}

int    WINAPI ProcessViewerEventW(const struct ProcessViewerEventInfo *Info)
{
	if (!gbLoadWrapperCalled)
		LoadWrapper(gszWrapper);
	if (fwf.ProcessViewerEventWrap)
		return fwf.ProcessViewerEventWrap(fwf.wpi, Info);
	return 0;
}

int    WINAPI PutFilesW(const PutFilesInfo *Info)
{
	if (fwf.PutFilesWrap)
		return fwf.PutFilesWrap(fwf.wpi, Info);
	return 0;
}

int    WINAPI SetDirectoryW(const SetDirectoryInfo *Info)
{
	if (fwf.SetDirectoryWrap)
		return fwf.SetDirectoryWrap(fwf.wpi, Info);
	return 0;
}

int    WINAPI SetFindListW(const SetFindListInfo *Info)
{
	if (!gbLoadWrapperCalled)
		LoadWrapper(gszWrapper);
	if (fwf.SetFindListWrap)
		return fwf.SetFindListWrap(fwf.wpi, Info);
	return 0;
}

int WINAPI GetCustomDataW(const wchar_t *FilePath, wchar_t **CustomData)
{
	if (!gbLoadWrapperCalled)
		LoadWrapper(gszWrapper);
	if (fwf.GetCustomDataWrap)
		return fwf.GetCustomDataWrap(fwf.wpi, FilePath, CustomData);
	return 0;
}

void WINAPI FreeCustomDataW(wchar_t *CustomData)
{
	if (fwf.FreeCustomDataWrap)
		fwf.FreeCustomDataWrap(fwf.wpi, CustomData);
}



// Changed functions
LONG_PTR WINAPI FarApiDefDlgProc(HANDLE hDlg, int Msg, int Param1, LONG_PTR Param2)
{
	if (fwf.FarApiDefDlgProcWrap)
		return fwf.FarApiDefDlgProcWrap(fwf.wpi, hDlg,Msg,Param1,Param2);
	return 0;
}
LONG_PTR WINAPI FarApiSendDlgMessage(HANDLE hDlg, int Msg, int Param1, LONG_PTR Param2)
{
	if (fwf.FarApiSendDlgMessageWrap)
		return fwf.FarApiSendDlgMessageWrap(fwf.wpi, hDlg,Msg,Param1,Param2);
	return 0;
}
BOOL WINAPI FarApiShowHelp(const wchar_t *ModuleName, const wchar_t *Topic, DWORD Flags)
{
	if (fwf.FarApiShowHelpWrap)
		return fwf.FarApiShowHelpWrap(fwf.wpi, ModuleName,Topic,Flags);
	return 0;
}
HANDLE WINAPI FarApiSaveScreen(int X1, int Y1, int X2, int Y2)
{
	if (fwf.FarApiSaveScreenWrap)
		return fwf.FarApiSaveScreenWrap(fwf.wpi, X1,Y1,X2,Y2);
	return 0;
}
void WINAPI FarApiRestoreScreen(HANDLE hScreen)
{
	if (fwf.FarApiRestoreScreenWrap)
		fwf.FarApiRestoreScreenWrap(fwf.wpi, hScreen);
}
void WINAPI FarApiText(int X, int Y, int Color, const wchar_t *Str)
{
	if (fwf.FarApiTextWrap)
		fwf.FarApiTextWrap(fwf.wpi, X,Y,Color,Str);
}
int WINAPI FarApiEditor(const wchar_t *FileName, const wchar_t *Title, int X1, int Y1, int X2, int Y2, DWORD Flags, int StartLine, int StartChar, UINT CodePage)
{
	if (fwf.FarApiEditorWrap)
		return fwf.FarApiEditorWrap(fwf.wpi, FileName,Title,X1,Y1,X2,Y2,Flags,StartLine,StartChar,CodePage);
	return 0;
}
int WINAPI FarApiViewer(const wchar_t *FileName, const wchar_t *Title, int X1, int Y1, int X2, int Y2, DWORD Flags, UINT CodePage)
{
	if (fwf.FarApiViewerWrap)
		return fwf.FarApiViewerWrap(fwf.wpi, FileName,Title,X1,Y1,X2,Y2,Flags,CodePage);
	return 0;
}
int WINAPI FarApiMenu(INT_PTR PluginNumber, int X, int Y, int MaxHeight, DWORD Flags, const wchar_t* Title, const wchar_t* Bottom, const wchar_t* HelpTopic, const int* BreakKeys, int* BreakCode, const struct Far2::FarMenuItem *Item, int ItemsNumber)
{
	if (fwf.FarApiMenuWrap)
		return fwf.FarApiMenuWrap(fwf.wpi, PluginNumber,X,Y,MaxHeight,Flags,Title,Bottom,HelpTopic,BreakKeys,BreakCode,Item,ItemsNumber);
	return 0;
}
int WINAPI FarApiMessage(INT_PTR PluginNumber, DWORD Flags, const wchar_t *HelpTopic, const wchar_t * const *Items, int ItemsNumber, int ButtonsNumber)
{
	if (fwf.FarApiMessageWrap)
		return fwf.FarApiMessageWrap(fwf.wpi, PluginNumber,Flags,HelpTopic,Items,ItemsNumber,ButtonsNumber);
	return 0;
}
HANDLE WINAPI FarApiDialogInit(INT_PTR PluginNumber, int X1, int Y1, int X2, int Y2, const wchar_t* HelpTopic, struct Far2::FarDialogItem *Item, unsigned int ItemsNumber, DWORD Reserved, DWORD Flags, Far2::FARWINDOWPROC DlgProc, LONG_PTR Param)
{
	if (fwf.FarApiDialogInitWrap)
		return fwf.FarApiDialogInitWrap(fwf.wpi, PluginNumber,X1,Y1,X2,Y2,HelpTopic,Item,ItemsNumber,Reserved,Flags,DlgProc,Param);
	return 0;
}
int WINAPI FarApiDialogRun(HANDLE hDlg)
{
	if (fwf.FarApiDialogRunWrap)
		return fwf.FarApiDialogRunWrap(fwf.wpi, hDlg);
	return 0;
}
void WINAPI FarApiDialogFree(HANDLE hDlg)
{
	if (fwf.FarApiDialogFreeWrap)
		fwf.FarApiDialogFreeWrap(fwf.wpi, hDlg);
}
int WINAPI FarApiControl(HANDLE hPlugin, int Command, int Param1, LONG_PTR Param2)
{
	if (fwf.FarApiControlWrap)
		return fwf.FarApiControlWrap(fwf.wpi, hPlugin,Command,Param1,Param2);
	return 0;
}
int WINAPI FarApiGetDirList(const wchar_t *Dir, struct Far2::FAR_FIND_DATA **pPanelItem, int *pItemsNumber)
{
	if (fwf.FarApiGetDirListWrap)
		return fwf.FarApiGetDirListWrap(fwf.wpi, Dir,pPanelItem,pItemsNumber);
	return 0;
}
void WINAPI FarApiFreeDirList(struct Far2::FAR_FIND_DATA *PanelItem, int nItemsNumber)
{
	if (fwf.FarApiFreeDirListWrap)
		fwf.FarApiFreeDirListWrap(fwf.wpi, PanelItem,nItemsNumber);
}
int WINAPI FarApiGetPluginDirList(INT_PTR PluginNumber, HANDLE hPlugin, const wchar_t *Dir, struct Far2::PluginPanelItem **pPanelItem, int *pItemsNumber)
{
	if (fwf.FarApiGetPluginDirListWrap)
		return fwf.FarApiGetPluginDirListWrap(fwf.wpi, PluginNumber,hPlugin,Dir,pPanelItem,pItemsNumber);
	return 0;
}
void WINAPI FarApiFreePluginDirList(struct Far2::PluginPanelItem *PanelItem, int nItemsNumber)
{
	if (fwf.FarApiFreePluginDirListWrap)
		fwf.FarApiFreePluginDirListWrap(fwf.wpi, PanelItem,nItemsNumber);
}
int WINAPI FarApiCmpName(const wchar_t *Pattern, const wchar_t *String, int SkipPath)
{
	if (fwf.FarApiCmpNameWrap)
		return fwf.FarApiCmpNameWrap(fwf.wpi, Pattern,String,SkipPath);
	return 0;
}
const wchar_t* WINAPI FarApiGetMsg(INT_PTR PluginNumber, int MsgId)
{
	if (fwf.FarApiGetMsgWrap)
		return fwf.FarApiGetMsgWrap(fwf.wpi, PluginNumber,MsgId);
	return 0;
}
INT_PTR WINAPI FarApiAdvControl(INT_PTR ModuleNumber, int Command, void *Param)
{
	if (fwf.FarApiAdvControlWrap)
		return fwf.FarApiAdvControlWrap(fwf.wpi, ModuleNumber,Command,Param);
	return 0;
}
int WINAPI FarApiViewerControl(int Command, void *Param)
{
	if (fwf.FarApiViewerControlWrap)
		return fwf.FarApiViewerControlWrap(fwf.wpi, Command,Param);
	return 0;
}
int WINAPI FarApiEditorControl(int Command, void *Param)
{
	if (fwf.FarApiEditorControlWrap)
		return fwf.FarApiEditorControlWrap(fwf.wpi, Command,Param);
	return 0;
}
int WINAPI FarApiInputBox(const wchar_t *Title, const wchar_t *SubTitle, const wchar_t *HistoryName, const wchar_t *SrcText, wchar_t *DestText, int   DestLength, const wchar_t *HelpTopic, DWORD Flags)
{
	if (fwf.FarApiInputBoxWrap)
		return fwf.FarApiInputBoxWrap(fwf.wpi, Title,SubTitle,HistoryName,SrcText,DestText,DestLength,HelpTopic,Flags);
	return 0;
}
int WINAPI FarApiPluginsControl(HANDLE hHandle, int Command, int Param1, LONG_PTR Param2)
{
	if (fwf.FarApiPluginsControlWrap)
		return fwf.FarApiPluginsControlWrap(fwf.wpi, hHandle,Command,Param1,Param2);
	return 0;
}
int WINAPI FarApiFileFilterControl(HANDLE hHandle, int Command, int Param1, LONG_PTR Param2)
{
	if (fwf.FarApiFileFilterControlWrap)
		return fwf.FarApiFileFilterControlWrap(fwf.wpi, hHandle,Command,Param1,Param2);
	return 0;
}
int WINAPI FarApiRegExpControl(HANDLE hHandle, int Command, LONG_PTR Param)
{
	if (fwf.FarApiRegExpControlWrap)
		return fwf.FarApiRegExpControlWrap(fwf.wpi, hHandle,Command,Param);
	return 0;
}
int WINAPI FarStdGetFileOwner(const wchar_t *Computer,const wchar_t *Name,wchar_t *Owner,int Size)
{
	if (fwf.FarStdGetFileOwnerWrap)
		return fwf.FarStdGetFileOwnerWrap(fwf.wpi, Computer,Name,Owner,Size);
	return 0;
}
int WINAPI FarStdGetPathRoot(const wchar_t *Path,wchar_t *Root, int DestSize)
{
	if (fwf.FarStdGetPathRootWrap)
		return fwf.FarStdGetPathRootWrap(fwf.wpi, Path,Root,DestSize);
	return 0;
}
wchar_t* WINAPI FarStdXlat(wchar_t *Line,int StartPos,int EndPos,DWORD Flags)
{
	if (fwf.FarStdXlatWrap)
		return fwf.FarStdXlatWrap(fwf.wpi, Line,StartPos,EndPos,Flags);
	return 0;
}
int WINAPI GetNumberOfLinks(const wchar_t *Name)
{
	if (fwf.GetNumberOfLinksWrap)
		return fwf.GetNumberOfLinksWrap(fwf.wpi, Name);
	return 0;
}
void WINAPI FarStdRecursiveSearch(const wchar_t *InitDir,const wchar_t *Mask,Far2::FRSUSERFUNC Func,DWORD Flags,void *Param)
{
	if (fwf.FarStdRecursiveSearchWrap)
		fwf.FarStdRecursiveSearchWrap(fwf.wpi, InitDir,Mask,Func,Flags,Param);
}
int WINAPI FarStdMkTemp(wchar_t *Dest, DWORD size, const wchar_t *Prefix)
{
	if (fwf.FarStdMkTempWrap)
		return fwf.FarStdMkTempWrap(fwf.wpi, Dest,size,Prefix);
	return 0;
}
int WINAPI FarStdProcessName(const wchar_t *param1, wchar_t *param2, DWORD size, DWORD flags)
{
	if (fwf.FarStdProcessNameWrap)
		return fwf.FarStdProcessNameWrap(fwf.wpi, param1,param2,size,flags);
	return 0;
}
int WINAPI FarStdMkLink(const wchar_t *Src,const wchar_t *Dest,DWORD Flags)
{
	if (fwf.FarStdMkLinkWrap)
		return fwf.FarStdMkLinkWrap(fwf.wpi, Src,Dest,Flags);
	return 0;
}
int WINAPI FarConvertPath(enum Far2::CONVERTPATHMODES Mode, const wchar_t *Src, wchar_t *Dest, int DestSize)
{
	if (fwf.FarConvertPathWrap)
		return fwf.FarConvertPathWrap(fwf.wpi, Mode,Src,Dest,DestSize);
	return 0;
}
int WINAPI FarGetReparsePointInfo(const wchar_t *Src, wchar_t *Dest,int DestSize)
{
	if (fwf.FarGetReparsePointInfoWrap)
		return fwf.FarGetReparsePointInfoWrap(fwf.wpi, Src,Dest,DestSize);
	return 0;
}
DWORD WINAPI FarGetCurrentDirectory(DWORD Size,wchar_t* Buffer)
{
	if (fwf.FarGetCurrentDirectoryWrap)
		return fwf.FarGetCurrentDirectoryWrap(fwf.wpi, Size,Buffer);
	return 0;
}
void WINAPI FarStdQSort(void *base, size_t nelem, size_t width, int (__cdecl *fcmp)(const void *, const void *))
{
	if (fwf.FarStdQSortWrap)
		fwf.FarStdQSortWrap(fwf.wpi, base, nelem, width, fcmp);
}
void WINAPI FarStdQSortEx(void *base, size_t nelem, size_t width, int (__cdecl *fcmp)(const void *, const void *,void *userparam),void *userparam)
{
	if (fwf.FarStdQSortExWrap)
		fwf.FarStdQSortExWrap(fwf.wpi, base, nelem, width, fcmp, userparam);
}
void* WINAPI FarStdBSearch(const void *key, const void *base, size_t nelem, size_t width, int (__cdecl *fcmp)(const void *, const void *))
{
	if (fwf.FarStdBSearchWrap)
		return fwf.FarStdBSearchWrap(fwf.wpi, key, base, nelem, width, fcmp);
	return NULL;
}





BOOL LoadWrapper(LPCWSTR asModule)
{
	gbLoadWrapperCalled = TRUE;
	
	BOOL lbRc = FALSE;
	int iWrap = 0;
	wchar_t szDbg[MAX_PATH+128];
	memset(&fwf, 0, sizeof(fwf));
	
	// In params
	fwf.StructSize = sizeof(fwf);
	fwf.hLoader = ghInstance;
	//fwf.hFar3Wrap = ghWrapper; -- это NULL еще
	fwf.ErrorInfo = gsErrInfo;
	fwf.ErrorInfoMax = ERRINFO_SIZE;
	fwf.ErrorTitle = gsModuleFail;
	fwf.ErrorTitleMax = ERRORTITLE_SIZE;
	fwf.Far3Build = MVV_3;
	// In static functions
	fwf.FarApiDefDlgProc = FarApiDefDlgProc;
	fwf.FarApiSendDlgMessage = FarApiSendDlgMessage;
	fwf.FarApiShowHelp = FarApiShowHelp;
	fwf.FarApiSaveScreen = FarApiSaveScreen;
	fwf.FarApiRestoreScreen = FarApiRestoreScreen;
	fwf.FarApiText = FarApiText;
	fwf.FarApiEditor = FarApiEditor;
	fwf.FarApiViewer = FarApiViewer;
	fwf.FarApiMenu = FarApiMenu;
	fwf.FarApiMessage = FarApiMessage;
	fwf.FarApiDialogInit = FarApiDialogInit;
	fwf.FarApiDialogRun = FarApiDialogRun;
	fwf.FarApiDialogFree = FarApiDialogFree;
	fwf.FarApiControl = FarApiControl;
	fwf.FarApiGetDirList = FarApiGetDirList;
	fwf.FarApiFreeDirList = FarApiFreeDirList;
	fwf.FarApiGetPluginDirList = FarApiGetPluginDirList;
	fwf.FarApiFreePluginDirList = FarApiFreePluginDirList;
	fwf.FarApiCmpName = FarApiCmpName;
	fwf.FarApiGetMsg = FarApiGetMsg;
	fwf.FarApiAdvControl = FarApiAdvControl;
	fwf.FarApiViewerControl = FarApiViewerControl;
	fwf.FarApiEditorControl = FarApiEditorControl;
	fwf.FarApiInputBox = FarApiInputBox;
	fwf.FarApiPluginsControl = FarApiPluginsControl;
	fwf.FarApiFileFilterControl = FarApiFileFilterControl;
	fwf.FarApiRegExpControl = FarApiRegExpControl;
	fwf.FarStdGetFileOwner = FarStdGetFileOwner;
	fwf.FarStdGetPathRoot = FarStdGetPathRoot;
	fwf.FarStdXlat = FarStdXlat;
	fwf.GetNumberOfLinks = GetNumberOfLinks;
	fwf.FarStdRecursiveSearch = FarStdRecursiveSearch;
	fwf.FarStdMkTemp = FarStdMkTemp;
	fwf.FarStdProcessName = FarStdProcessName;
	fwf.FarStdMkLink = FarStdMkLink;
	fwf.FarConvertPath = FarConvertPath;
	fwf.FarGetReparsePointInfo = FarGetReparsePointInfo;
	fwf.FarGetCurrentDirectory = FarGetCurrentDirectory;
	fwf.FarStdQSort = FarStdQSort;
	fwf.FarStdQSortEx = FarStdQSortEx;
	fwf.FarStdBSearch = FarStdBSearch;


	// GO!
	
	//#ifdef DO_NOT_UNLOAD_WRAP
	//ghWrapper = GetModuleHandle(asModule);
	//if (!ghWrapper)
	//#endif
	ghWrapper = LoadLibrary(asModule);
	
	if (ghWrapper)
	{
		fwf.hFar3Wrap = ghWrapper;
		InitPlugin = (_InitPlugin)GetProcAddress(ghWrapper, "InitPlugin");
		//ExitPlugin = (_ExitPlugin)GetProcAddress(ghWrapper, "ExitPlugin");
		if (!InitPlugin /*|| !ExitPlugin*/)
		{
			wsprintf(gsErrInfo, L"Far3Wrap\nWrapper module failed (InitPlugin not found): %s\n", asModule);
			OutputDebugString(gsErrInfo);
			GetModuleTitle(L"InitPlugin not found", 0, TRUE);
		}
		else
		{
			lbRc = TRUE; // т.к. основной код враппера удалось загрузить
			gsModuleFail[0] = 0; // сброс на всякий случай
			if (0 != (iWrap = InitPlugin(&fwf)))
			{
				guidPlugin = fwf.PluginGuid;
				wsprintf(szDbg, L"Wrapper module failed (ErrCode=%i): %s\n", iWrap, asModule);
				OutputDebugString(szDbg);
				if (!*gsModuleFail) // если основной код враппера не сформировал заголовок для меню плагинов...
					GetModuleTitle(L"Wrapper failed", iWrap, TRUE);
			}
		}
	}
	else
	{
		GetModuleTitle(L"Wrapper loading failed", GetLastError(), TRUE);
	}
	if (!lbRc && ghWrapper)
	{
		FreeLibrary(ghWrapper);
		ghWrapper = NULL;
		// Чтобы не было вероятности вызвать внешнюю функцию, если длл-ку уже выгрузили
		memset(&fwf, 0, sizeof(fwf));
	}
	return lbRc;
}
