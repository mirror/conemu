
/*
Copyright (c) 2009-2012 Maximus5
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
#ifdef _DEBUG
	#include <crtdbg.h>
#else
	#define _ASSERT(x)
	#define _ASSERTE(x)
#endif
#include "version.h"
#include "ResolveDlg.h"

#include <tchar.h>

#if !defined(_UNICODE)
	#include "common/ansi/plugin.hpp"
	#define FARDLGPARM LPARAM
	#define FARDLGRET LONG_PTR
#elif FAR_UNICODE>=2460
	#include "common/far3/pluginW3.hpp"
	#define FARDLGPARM void*
	#define FARDLGRET INT_PTR
#else
	#include "common/unicode/pluginW.hpp"
	#define FARDLGPARM LPARAM
	#define FARDLGRET LONG_PTR
#endif
#include "common/FarHelper.h"

#undef _tcslen
#ifdef _UNICODE
#define _tcslen lstrlenW
#else
#define _tcslen lstrlenA
#endif

#if FAR_UNICODE>=1906
// Plugin GUID
GUID guid_PluginGuid = { /* fc5e35f4-02f8-4dc0-9acc-f6b6c962a51c */
    0xfc5e35f4,
    0x02f8,
    0x4dc0,
    {0x9a, 0xcc, 0xf6, 0xb6, 0xc9, 0x62, 0xa5, 0x1c}
  };
GUID guid_ResolveDlg = { /* 7429e6a7-f190-4f56-a974-14560676d851 */
    0x7429e6a7,
    0xf190,
    0x4f56,
    {0xa9, 0x74, 0x14, 0x56, 0x06, 0x76, 0xd8, 0x51}
  };
GUID guid_Msg1 = { /* 9398c13b-fc89-49c0-8dbb-e06429022eb6 */
    0x9398c13b,
    0xfc89,
    0x49c0,
    {0x8d, 0xbb, 0xe0, 0x64, 0x29, 0x02, 0x2e, 0xb6}
  };
GUID guid_Msg2 = { /* 504f5430-caf6-49c0-8019-b3803af2a63d */
    0x504f5430,
    0xcaf6,
    0x49c0,
    {0x80, 0x19, 0xb3, 0x80, 0x3a, 0xf2, 0xa6, 0x3d}
  };
GUID guid_Msg3 = { /* bbabad5b-d2e9-409c-9ba8-fed021f1e41a */
    0xbbabad5b,
    0xd2e9,
    0x409c,
    {0x9b, 0xa8, 0xfe, 0xd0, 0x21, 0xf1, 0xe4, 0x1a}
  };
GUID guid_Msg4 = { /* a03e507c-7422-45a1-b41b-0100f1544be8 */
    0xa03e507c,
    0x7422,
    0x45a1,
    {0xb4, 0x1b, 0x01, 0x00, 0xf1, 0x54, 0x4b, 0xe8}
  };
GUID guid_Msg5 = { /* 41b0d3de-2a99-4585-a05b-07781304ca17 */
    0x41b0d3de,
    0x2a99,
    0x4585,
    {0xa0, 0x5b, 0x07, 0x78, 0x13, 0x04, 0xca, 0x17}
  };
#endif

struct PluginStartupInfo psi;
struct FarStandardFunctions FSF;

/*
TODO: нужен полноценный вывод результата в поля диалога
TODO: настройка путей include/lib/bin
*/

#if defined(__GNUC__)
BOOL APIENTRY DllMain( HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved )
{
	switch (ul_reason_for_call) {
		case DLL_PROCESS_ATTACH:
			break;
	}
	return TRUE;
}
#endif

#if defined(__GNUC__)
#ifdef __cplusplus
extern "C"{
#endif
  BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved);
  void WINAPI SetStartupInfoW(const PluginStartupInfo *aInfo);
  void WINAPI ExitFARW(void);
  int  WINAPI ConfigureW(int ItemNumber);
  HANDLE WINAPI OpenPluginW(int OpenFrom,INT_PTR Item);
  int WINAPI GetMinFarVersionW(void);
  void WINAPI GetPluginInfoW(struct PluginInfo *pi);
#ifdef __cplusplus
};
#endif

BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved)
{
  return DllMain(hDll, dwReason,lpReserved);
}
#endif

#ifndef FAR_UNICODE
wchar_t* gsRootPath = NULL;
#endif
char* gsPath = NULL;		//("MSVCPATH"));
char* gsIncPath = NULL;	//("IncPath"));
char* gsLibPath = NULL;	//("LibPath"));
char* gsIncs = NULL;		//("Includes"));
char* gsLibs = NULL;		//("Libraries"));

struct SettingString
{
	char** Value;
	LPCSTR TitleA;
	LPCWSTR TitleW;
	LPCSTR DefValue;
	LPCSTR DefValue_WOW;
};
SettingString gs[] =
{
	{&gsPath, "MSVCPATH", L"MSVCPATH", DEFAULT_PATH, DEFAULT_PATH_WOW},
	{&gsIncPath, "IncPath", L"IncPath", DEFAULT_INCPATH, DEFAULT_INCPATH_WOW},
	{&gsLibPath, "LibPath", L"LibPath", DEFAULT_LIBPATH, DEFAULT_LIBPATH_WOW},
	{&gsIncs, "Includes", L"Includes", DEFAULT_INCS},
	{&gsLibs, "Libraries", L"Libraries", DEFAULT_LIBS},
};
#define GS_COUNT ARRAYSIZE(gs)


BOOL IsWindows64(BOOL *pbIsWow64Process/* = NULL */)
{
	typedef BOOL (WINAPI* IsWow64Process_t)(HANDLE hProcess, PBOOL Wow64Process);
	BOOL is64bitOs = FALSE, isWow64process = FALSE;
#ifdef _WIN64
	is64bitOs = TRUE; isWow64process = FALSE;
#else
	// Проверяем, где мы запущены
	isWow64process = FALSE;
	HMODULE hKernel = GetModuleHandleW(L"kernel32.dll");

	if (hKernel)
	{
		IsWow64Process_t IsWow64Process_f = (IsWow64Process_t)GetProcAddress(hKernel, "IsWow64Process");

		if (IsWow64Process_f)
		{
			BOOL bWow64 = FALSE;

			if (IsWow64Process_f(GetCurrentProcess(), &bWow64) && bWow64)
			{
				isWow64process = TRUE;
			}
		}
	}

	is64bitOs = isWow64process;
#endif

	if (pbIsWow64Process)
		*pbIsWow64Process = isWow64process;

	return is64bitOs;
}

void LoadSettings()
{
#if FAR_UNICODE>=2460
	FarSettingsCreate sc = {sizeof(FarSettingsCreate), guid_PluginGuid, INVALID_HANDLE_VALUE};
	if (psi.SettingsControl(INVALID_HANDLE_VALUE, SCTL_CREATE, 0, &sc) != 0)
	{
		for (int i = 0; i < GS_COUNT; i++)
		{
			if (*gs[i].Value) { free(*gs[i].Value); *gs[i].Value = NULL; }

			FarSettingsItem fsi = {0};
			fsi.Name = gs[i].TitleW;
			fsi.Type = FST_STRING;
			if (psi.SettingsControl(sc.Handle, SCTL_GET, 0, &fsi))
			{
				int nLen = lstrlen(fsi.String);
				*gs[i].Value = (char*)calloc(nLen+1,sizeof(**gs[i].Value));
				//TODO: Unicode?
				WideCharToMultiByte(CP_ACP, 0, fsi.String, -1, *gs[i].Value, nLen+1, 0,0);
			}
		}
		psi.SettingsControl(sc.Handle, SCTL_FREE, 0, 0);
	}
#else
	HKEY hKey = NULL;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, gsRootPath, 0, KEY_READ, &hKey)==ERROR_SUCCESS)
	{
		#define QueryValue(sz,valuename) { \
			if (sz) { free(sz); sz = NULL; } \
			if (!RegQueryValueExA(hKey, valuename, NULL, NULL, NULL, &dw)) { \
				sz = (char*)calloc(dw+2,1); \
				if (RegQueryValueExA(hKey, valuename, NULL, NULL, (LPBYTE)sz, &dw)) { \
					free(sz); sz = NULL; \
				} \
			} \
		}

		DWORD dw;
		
		for (int i = 0; i < GS_COUNT; i++)
		{
			QueryValue(*gs[i].Value,gs[i].TitleA);
		}

		RegCloseKey(hKey); hKey = NULL;
	}
#endif

	BOOL lbWin64 = IsWindows64(NULL);
	char szExpanded[1024];

	for (int i = 0; i < GS_COUNT; i++)
	{
		if (!*gs[i].Value || !**gs[i].Value)
		{
			if (*gs[i].Value) free(*gs[i].Value);
			ExpandEnvironmentStringsA((lbWin64&&gs[i].DefValue_WOW)?gs[i].DefValue_WOW:gs[i].DefValue, szExpanded, ARRAYSIZE(szExpanded));
			_ASSERTE(szExpanded[0] != 0);
			*gs[i].Value = _strdup(szExpanded);
		}
	}
}

void SaveSettings()
{
#if FAR_UNICODE>=2460
	FarSettingsCreate sc = {sizeof(FarSettingsCreate), guid_PluginGuid, INVALID_HANDLE_VALUE};
	if (psi.SettingsControl(INVALID_HANDLE_VALUE, SCTL_CREATE, 0, &sc) != 0)
	{
		for (int i = 0; i < GS_COUNT; i++)
		{
			FarSettingsItem fsi = {0};
			fsi.Name = gs[i].TitleW;
			fsi.Type = FST_STRING;
			int nLen = *gs[i].Value ? lstrlenA(*gs[i].Value) : 0;
			wchar_t* psz = (wchar_t*)calloc(nLen+1,sizeof(*psz));
			//TODO: Unicode?
			MultiByteToWideChar(CP_ACP, 0, *gs[i].Value ? *gs[i].Value : "", -1, psz, nLen+1);
			fsi.String = psz;
			psi.SettingsControl(sc.Handle, SCTL_SET, 0, &fsi);
			free(psz);
		}
		psi.SettingsControl(sc.Handle, SCTL_FREE, 0, 0);
	}
#else
	HKEY hKey = NULL;
	if (RegCreateKeyEx(HKEY_CURRENT_USER, gsRootPath, 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL)==ERROR_SUCCESS)
	{
		DWORD dw;
		
		for (int i = 0; i < GS_COUNT; i++)
		{
			if (*gs[i].Value)
			{
				dw = lstrlenA(*gs[i].Value)+1;
				RegSetValueExA(hKey, gs[i].TitleA, 0, REG_SZ, (LPBYTE)*gs[i].Value, dw);
			}
		}

		RegCloseKey(hKey); hKey = NULL;
	}
#endif
}

#if FAR_UNICODE>=2460
void WINAPI GetGlobalInfoW(struct GlobalInfo *Info)
{
	_ASSERTE(Info->StructSize == sizeof(GlobalInfo));
	
	Info->MinFarVersion = FARMANAGERVERSION;
	
	// Build: YYMMDDX (YY - две цифры года, MM - месяц, DD - день, X - 0 и выше-номер подсборки)
	Info->Version = MAKEFARVERSION(MVV_1,MVV_2,MVV_3,((MVV_1 % 100)*100000) + (MVV_2*1000) + (MVV_3*10) + (MVV_4 % 10),VS_RELEASE);
	
	Info->Guid = guid_PluginGuid;
	Info->Title = L"Resolve";
	Info->Description = L"Resolve plugin";
	Info->Author = L"ConEmu.Maximus5@gmail.com";
}
#endif

// minimal(?) FAR version
int WINAPI GetMinFarVersionW(void)
{
#if FAR_UNICODE>=2460
	#define MAKEFARVERSION2(major,minor,build) ( ((major)<<8) | (minor) | ((build)<<16))
	#define FARMANAGERVERSION2 MAKEFARVERSION2(FARMANAGERVERSION_MAJOR,FARMANAGERVERSION_MINOR,FARMANAGERVERSION_BUILD)
#else
	#define FARMANAGERVERSION2 FARMANAGERVERSION
#endif

	return FARMANAGERVERSION2;
}

void WINAPI GetPluginInfoW(struct PluginInfo *pi)
{
    //static TCHAR *szMenu[1];
    //szMenu[0]=_T("Resolve");
	//pi->StructSize = sizeof(struct PluginInfo);
	//pi->Flags = PF_EDITOR | PF_VIEWER;
	//pi->PluginMenuStrings = szMenu;
	//pi->PluginMenuStringsNumber = 1;
	//pi->PluginConfigStrings = szMenu;
	//pi->PluginConfigStringsNumber = 1;
	//pi->CommandPrefix = NULL;
	//pi->Reserved = 0;	

	#if FAR_UNICODE>=1906
	_ASSERTE(pi->StructSize >= sizeof(struct PluginInfo));
	#else
	pi->StructSize = sizeof(struct PluginInfo);
	#endif

	pi->Flags = PF_EDITOR | PF_VIEWER;

	static TCHAR szMenu[MAX_PATH];
	lstrcpy(szMenu, _T("Resolve"));
    static TCHAR *pszMenu[1];
    pszMenu[0] = szMenu;
	
	#if FAR_UNICODE>=1906
		pi->PluginMenu.Guids = &guid_PluginGuid;
		pi->PluginMenu.Strings = pszMenu;
		pi->PluginMenu.Count = 1;
	#else
		pi->PluginMenuStrings = pszMenu;
		pi->PluginMenuStringsNumber = 1;
	#endif
	
	#if FAR_UNICODE>=1906
		pi->PluginConfig.Guids = &guid_PluginGuid;
		pi->PluginConfig.Strings = pszMenu;
		pi->PluginConfig.Count = 1;
	#else
		pi->PluginConfigStrings = pszMenu;
		pi->PluginConfigStringsNumber = 1;
	#endif
}

void WINAPI SetStartupInfoW(const PluginStartupInfo *aInfo)
{
	::psi = *aInfo;
	::FSF = *aInfo->FSF;
	::psi.FSF = &::FSF;

#if !defined(FAR_UNICODE)
	int lRootLen = lstrlen(aInfo->RootKey);
	gsRootPath = (wchar_t*)calloc(lRootLen+32,2);
	lstrcpy(gsRootPath, aInfo->RootKey);
	if (gsRootPath[lRootLen-1] != _T('\\'))
		gsRootPath[lRootLen++] = _T('\\');
	lstrcpy(gsRootPath+lRootLen, _T("CPPResolve"));
#endif
}

#if FAR_UNICODE>=2460
#define ExitArg void*
#else
#define ExitArg void
#endif

void WINAPI ExitFARW(ExitArg)
{
}

void WritePath(HANDLE hFile, LPCSTR pszList)
{
	LPCSTR pszColon;
	DWORD dw;
	while (*pszList == ';') pszList++;
	while (*pszList)
	{
		pszColon = strchr(pszList, ';');
		if (!pszColon) pszColon = pszList + lstrlenA(pszList);
		WriteFile(hFile, pszList, (DWORD)(pszColon-pszList), &dw, NULL);
		WriteFile(hFile, "\r\n", 2, &dw, NULL);
		while (*pszColon == ';') pszColon++;
		pszList = pszColon;
	}
}

void ParseStringSettings(char* pszData)
{
	char *pszSection, *pszSectionEnd;
	char *pszLine, *pszDest, *pszNextLine;
	SettingString* ps = NULL;
	bool lbIsPath;
	while (pszData && *pszData)
	{
		//while (*pszData == ' ' || *pszData == '\t' || *pszData == '\r' || *pszData == '\n')
		//	pszData++;
		pszSection = strchr(pszData, '[');
		if (!pszSection)
			break;
		pszSection++;
		pszSectionEnd = strchr(pszSection, ']');
		if (!pszSectionEnd)
			break;
		*pszSectionEnd = 0; ps = NULL;
		for (int i = 0; i < GS_COUNT; i++)
		{
			if (lstrcmpiA(pszSection, gs[i].TitleA) == 0)
			{
				ps = gs+i;
				lbIsPath = (i < 3); // замена '\r\n' на ';'
				break;
			}
		}
		if (!ps)
		{
			_ASSERTE(ps!=NULL);
			break;
		}

		pszLine = pszSectionEnd+1;
		while (*pszLine == ' ' || *pszLine == '\t' || *pszLine == '\r' || *pszLine == '\n')
			pszLine++;
		*ps->Value = (char*)calloc(lstrlenA(pszLine)+1,1);
		pszDest = *ps->Value;
		if (lbIsPath)
		{
			while (*pszLine)
			{
				if (*pszDest) lstrcatA(pszDest, ";");
				pszNextLine = strpbrk(pszLine, "\r\n");
				if (pszNextLine) *pszNextLine = 0;
				lstrcatA(pszDest, pszLine);
				pszLine = pszNextLine+1;
				while (*pszLine == ' ' || *pszLine == '\t' || *pszLine == '\r' || *pszLine == '\n')
					pszLine++;
				if (*pszLine == '[')
					break;
			}
			pszData = pszLine;
		}
		else
		{
			pszSection = strstr(pszLine, "\n[");
			if (!pszSection)
				lstrcpyA(pszDest, pszLine);
			else
				memmove(pszDest, pszLine, pszSection-pszLine+1);
			pszLine = pszDest + lstrlenA(pszDest);
			while (pszLine >= pszDest && (*(pszLine-1) == '\r' || *(pszLine-1) == '\n'))
			{
				pszLine--; *pszLine = 0;
			}

			// Next
			if (!pszSection)
				break;
			pszData = pszSection+1;
		}
	}
}

int    WINAPI ConfigureW(int ItemNumber)
{
	LoadSettings();
	/*
	QueryValue(gsPath,("MSVCPATH"));
	QueryValue(gsIncPath,("IncPath"));
	QueryValue(gsLibPath,("LibPath"));
	QueryValue(gsIncs,("Includes"));
	QueryValue(gsLibs,("Libraries"));
	*/

	wchar_t szTempPath[MAX_PATH*2];
	FSF.MkTemp(szTempPath,
		#ifdef _UNICODE
			MAX_PATH*2,
		#endif
		_T("CRSV"));

	HANDLE hFile = CreateFile(szTempPath, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		LPCWSTR lsLines[3];
		lsLines[0] = L"Resolve";
		lsLines[1] = L"Can't create temp file";
		lsLines[2] = szTempPath;
		psi.Message(_PluginNumber(guid_Msg1), FMSG_ERRORTYPE|FMSG_WARNING|FMSG_MB_OK, NULL, lsLines, 3, 0);
		return 0;
	}
	DWORD dw;
	for (int i = 0; i < GS_COUNT; i++)
	{
		WriteFile(hFile, "[", 1, &dw, NULL);
		WriteFile(hFile, gs[i].TitleA, lstrlenA(gs[i].TitleA), &dw, NULL);
		WriteFile(hFile, "]\r\n", 3, &dw, NULL);
		if (i < 3)
			WritePath(hFile, *gs[i].Value);
		else
			WriteFile(hFile, *gs[i].Value, lstrlenA(*gs[i].Value), &dw, NULL);
		WriteFile(hFile, "\r\n\r\n", 4, &dw, NULL);
	}
	CloseHandle(hFile);

	int nEdtRc = psi.Editor(szTempPath, NULL, 0,0,-1,-1, EF_DISABLEHISTORY,
		#ifdef _UNICODE
		1, 0, 1251
		#else
		0, 0
		#endif
	);
	
	if (nEdtRc == EEC_MODIFIED)
	{
		hFile = CreateFile(szTempPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			LPCWSTR lsLines[3];
			lsLines[0] = L"Resolve";
			lsLines[1] = L"Can't open temp file";
			lsLines[2] = szTempPath;
			psi.Message(_PluginNumber(guid_Msg2), FMSG_ERRORTYPE|FMSG_WARNING|FMSG_MB_OK, NULL, lsLines, 3, 0);
			return 0;
		}
		for (int i = 0; i < GS_COUNT; i++)
		{
			if (*gs[i].Value) { free(*gs[i].Value); *gs[i].Value = NULL; }
		}
		DWORD nLen = GetFileSize(hFile, NULL);
		if (nLen == 0)
		{
			for (int i = 0; i < GS_COUNT; i++)
			{
				if (*gs[i].Value) free(*gs[i].Value);
				*gs[i].Value = _strdup(gs[i].DefValue);
			}
		}
		else
		{
			char* pszData = (char*)calloc(nLen+2,1);
			ReadFile(hFile, pszData, nLen, &nLen, NULL);
			// Парсинг
			ParseStringSettings(pszData);
			free(pszData);
		}
		CloseHandle(hFile);
		for (int i = 0; i < GS_COUNT; i++)
		{
			if (!*gs[i].Value || !**gs[i].Value)
			{
				if (*gs[i].Value) free(*gs[i].Value);
				*gs[i].Value = _strdup(gs[i].DefValue);
			}
		}

		SaveSettings();
	}

	DeleteFile(szTempPath);
	
	return 0;
}

FARDLGRET WINAPI ResolveDlgProc(
  HANDLE   hDlg,
  int      Msg,
  int      Param1,
  FARDLGPARM Param2
);

CResolveDlg* pResolve = NULL;

wchar_t* SaveEnvVar(LPCWSTR asVarName)
{
	int nEnvLen = 32767; DWORD dwErr;
	wchar_t *pszSave = (wchar_t*)malloc(nEnvLen*2); pszSave[0] = 0;
	if (!GetEnvironmentVariableW(asVarName, pszSave, nEnvLen))
	{
		dwErr = GetLastError();
		if (dwErr == ERROR_ENVVAR_NOT_FOUND)
		{
			free(pszSave); pszSave = NULL;
		}
	}
	return pszSave;
}

#define ResolveBtnId 4
#define ErrLookBtnId 5
#define Copy1BtnId 10 // Value only
#define Copy2BtnId 11 // <type> <name> = <value>
#define Copy3BtnId 12 // value/*name*/
#define ExprId 2
#define TypeId 7
#define ValueId 9
#if FAR_UNICODE>=2460
HANDLE WINAPI OpenW(const struct OpenInfo *Info)
#else
HANDLE WINAPI OpenPluginW(int OpenFrom,INT_PTR Item)
#endif
{
	LoadSettings();

#if FAR_UNICODE>=2460
	FarDialogItem dialog[] = {
	{ DI_DOUBLEBOX,    3,  1, 52, 10, {0}, 0,0,0, L"CPP Resolve" },
	{ DI_TEXT,         5,  3,  0,  0, {0}, 0,0,0, L"&Expr:" },
	{ DI_EDIT,        12,  3, 50,  0, {0}, L"resolve_expr", 0, DIF_HISTORY},
	{ DI_BUTTON,       27, 4,  0,  0, {0}, 0,0,0, L"&Resolve" },
	{ DI_BUTTON,       40, 4,  0,  0, {0}, 0,0,0, L"Err&Look" },
	//
	{ DI_TEXT,         5,  5,  0,  0, {0}, 0,0, DIF_SEPARATOR},
	//
	{ DI_TEXT,         5,  7,  0,  0, {0}, 0,0,0, L"&Type:"},
	{ DI_EDIT,        12,  7, 50,  0, {0}, 0,0, DIF_READONLY, L""},
	{ DI_TEXT,         5,  8,  0,  0, {0}, 0,0,0, L"&Value:"},
	{ DI_EDIT,        12,  8, 50,  0, {0}, 0,0, DIF_READONLY, L""},
	{ DI_BUTTON,       12, 9,  0,  0, {0}, 0,0,0, L"&1 Copy"},
	{ DI_BUTTON,       24, 9,  0,  0, {0}, 0,0,0, L"&2 Copy #"},
	{ DI_BUTTON,       38, 9,  0,  0, {0}, 0,0,0, L"&3 Copy /*"}
	}; //type,x1,y1,x2,y2,{Sel},History,Mask,Flags,Data,MaxLength,UserData
#else
	FarDialogItem dialog[] = {
	{ DI_DOUBLEBOX,    3,  1, 52, 10, 0, {0}, 0, 0, L"CPP Resolve" },
	{ DI_TEXT,         5,  3,  0,  0, 0, {0}, 0, 0, L"&Expr:" },
	{ DI_EDIT,        12,  3, 50,  0, 1, {(int)L"resolve_expr"}, DIF_HISTORY, 0, 0 },
	{ DI_BUTTON,       27, 4,  0,  0, 0, {0}, 0, 0, L"&Resolve" },
	{ DI_BUTTON,       40, 4,  0,  0, 0, {0}, 0, 0, L"Err&Look" },
	//
	{ DI_TEXT,         5,  5,  0,  0, 0, {0}, DIF_SEPARATOR, 0, L"" },
	//
	{ DI_TEXT,         5,  7,  0,  0, 0, {0}, 0, 0, L"&Type:" },
	{ DI_EDIT,        12,  7, 50,  0, 0, {0}, DIF_DISABLE, 0, L"" },
	{ DI_TEXT,         5,  8,  0,  0, 0, {0}, 0, 0, L"&Value:" },
	{ DI_EDIT,        12,  8, 50,  0, 0, {0}, DIF_DISABLE, 0, L"" },
	{ DI_BUTTON,       12, 9,  0,  0, 0, {0}, 0, 0, L"&1 Copy" },
	{ DI_BUTTON,       24, 9,  0,  0, 0, {0}, 0, 0, L"&2 Copy #" },
	{ DI_BUTTON,       38, 9,  0,  0, 0, {0}, 0, 0, L"&3 Copy /*" }
	//{ DI_TEXT,         5,  6,  0,  0, 0, 0, 0, 0, L"&Value:" },
	}; //type,x1,y1,x2,y2,Focus,{Sel},Flags,Def,Data
#endif

	wchar_t *pszSavePath = SaveEnvVar(L"PATH");
	wchar_t *pszSaveInc  = SaveEnvVar(L"include");
	wchar_t *pszSaveLib  = SaveEnvVar(L"lib");

	pResolve = new CResolveDlg();

#if FAR_UNICODE>=2460
	HANDLE hDlg = psi.DialogInit(&guid_PluginGuid, &guid_ResolveDlg, -1, -1, 56, 12, NULL, dialog,
              sizeof(dialog)/sizeof(dialog[0]), 0, 0, ResolveDlgProc, 0);
#else
	HANDLE hDlg = psi.DialogInit(psi.ModuleNumber, -1, -1, 56, 12, NULL, dialog,
              sizeof(dialog)/sizeof(dialog[0]), 0, 0, ResolveDlgProc, 0);
#endif
    psi.DialogRun(hDlg);
    psi.DialogFree(hDlg);

	delete pResolve; pResolve = NULL;

	if (pszSavePath)
	{
		SetEnvironmentVariableW(L"PATH", pszSavePath); free(pszSavePath);
	}
	if (pszSaveInc)
	{
		SetEnvironmentVariableW(L"include", pszSaveInc); free(pszSaveInc);
	}
	if (pszSaveLib)
	{
		SetEnvironmentVariableW(L"lib", pszSaveLib); free(pszSaveLib);
	}
	
	return INVALID_HANDLE_VALUE;
}

void DoErrLookup(HANDLE hDlg)
{
	INT_PTR nLen = 0;
	//_asm int 3;
	nLen = psi.SendDlgMessage(hDlg, DM_GETTEXTLENGTH, ExprId, 0);
	if (nLen>0)
	{
		FarDialogItemData fdid;
		fdid.PtrLength = nLen+1;
		fdid.PtrData = (wchar_t*)calloc(nLen+1,2);
		
		psi.SendDlgMessage(hDlg, DM_GETTEXT, ExprId, (FARDLGPARM)&fdid);

		DWORD nVal = 0;
		WCHAR *psz = fdid.PtrData;
		WCHAR *endptr=NULL;
		while (*psz==L' ' || *psz==L'\t') psz++;
		if (psz[0] == L'0' && (psz[1] == L'x' || psz[1] == L'X'))
			nVal = wcstoul(psz+2, &endptr, 16);
		else
			nVal = wcstoul(psz, &endptr, 10);

		WCHAR* pszFull = NULL;
		WCHAR* lpMsgBuf=NULL;
		if (FormatMessageW( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, nVal, 
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL ))
		{
			while ((pszFull = wcsstr(lpMsgBuf, L"\r\n")) != NULL)
			{
				pszFull[0] = L' '; pszFull[1] = L'\n';
			}

			pszFull = (WCHAR*)calloc(200+lstrlen(lpMsgBuf),2);
			wsprintf(pszFull, L"Resolve\nErrCode=0x%08X (%i)\n\n%s\nOK", nVal, (int)nVal,
				(WCHAR*)lpMsgBuf);
			LocalFree(lpMsgBuf);
		}
		else
		{
			pszFull = (WCHAR*)calloc(200,2);
			wsprintf(pszFull, L"Resolve\nErrCode=0x%08X (%i)\n\nCan't get description\nOK", nVal, (int)nVal);
		}

		psi.Message(_PluginNumber(guid_Msg3), FMSG_ALLINONE|FMSG_WARNING|FMSG_LEFTALIGN, NULL, 
		(const wchar_t * const *)pszFull, 0, 1);

		free(fdid.PtrData);
		free(pszFull);

	}
	else
	{
		// Не введен текст
	}

	psi.SendDlgMessage(hDlg, DM_SETFOCUS, ExprId, 0);
	if (nLen>0)
	{
		EditorSelect ei = {BTYPE_STREAM,0,0,nLen,1};
		psi.SendDlgMessage(hDlg, DM_SETSELECTION, ExprId, (FARDLGPARM)&ei);
	}
}

void DoResolve(HANDLE hDlg)
{
	INT_PTR nLen = 0;
	nLen = psi.SendDlgMessage(hDlg, DM_GETTEXTLENGTH, ExprId, 0);
	if (nLen>0 && nLen<1000)
	{
		WCHAR szText[1024];
		FarDialogItemData fdid;
		fdid.PtrLength = nLen+1;
		fdid.PtrData = szText;

		psi.SendDlgMessage(hDlg, DM_GETTEXT, ExprId, (FARDLGPARM)&fdid);
		psi.SendDlgMessage(hDlg, DM_SETTEXTPTR, TypeId, (FARDLGPARM)_T(""));
		psi.SendDlgMessage(hDlg, DM_SETTEXTPTR, ValueId, (FARDLGPARM)_T(""));
		
		
		//MessageBoxW(GetForegroundWindow(), fdid.PtrData, L"Test", 0);

		LPTSTR lsResult=NULL;
		LPCTSTR lsType=NULL, lsValue=NULL;
		#ifdef _UNICODE
		char szAnsi[1001];
		WideCharToMultiByte(CP_ACP, 0, szText, -1, szAnsi, 1001, 0,0);
		#endif

		psi.SendDlgMessage(hDlg, DM_SHOWDIALOG, FALSE, 0);

		HANDLE hScr = psi.SaveScreen(0,0,-1,-1);
		psiControl(INVALID_HANDLE_VALUE, FCTL_GETUSERSCREEN, 0, NULL);

		TCHAR TempDir[MAX_PATH];
		FSF.MkTemp(TempDir,ARRAYSIZE(TempDir),L"Rsl");

		BOOL lbRc = pResolve->OnResolve(
			TempDir,
			#ifdef _UNICODE
			szAnsi
			#else
			szText
			#endif
		, &lsResult, &lsType, &lsValue);

		psiControl(INVALID_HANDLE_VALUE, FCTL_SETUSERSCREEN, 0, NULL);
		psi.RestoreScreen(hScr);

		psi.SendDlgMessage(hDlg, DM_SHOWDIALOG, TRUE, 0);

		if (lbRc)
		{
			psi.SendDlgMessage(hDlg, DM_SETTEXTPTR, TypeId, (FARDLGPARM)lsType);
			psi.SendDlgMessage(hDlg, DM_SETTEXTPTR, ValueId, (FARDLGPARM)lsValue);
		}
		else if (lsResult)
		{
			psi.Message(_PluginNumber(guid_Msg3), FMSG_ALLINONE|(lbRc ? 0 : FMSG_WARNING)|FMSG_LEFTALIGN|FMSG_MB_OK, NULL, 
				(const TCHAR * const *)lsResult, 0, 0);
		}
			
	}
}

FARDLGRET WINAPI ResolveDlgProc(
  HANDLE   hDlg,
  int      Msg,
  int      Param1,
  FARDLGPARM Param2
)
{
	if (Msg == DN_CLOSE && Param1!=-1)
	{
		INT_PTR nLen = 0;
		nLen = psi.SendDlgMessage(hDlg, DM_GETTEXTLENGTH, ExprId, 0);
		if (nLen>0)
		{
			WCHAR szText[1024];
			FarDialogItemData fdid;
			fdid.PtrLength = nLen+1;
			fdid.PtrData = szText;
			
			psi.SendDlgMessage(hDlg, DM_GETTEXT, ExprId, (FARDLGPARM)&fdid);
			
			DWORD nVal = 0;
			WCHAR *psz = fdid.PtrData;
			WCHAR *endptr=NULL;
			while (*psz==L' ' || *psz==L'\t') psz++;
			if (*psz)
			{
				if (psz[0] == L'0' && (psz[1] == L'x' || psz[1] == L'X'))
				{
					psz += 2;
					nVal = wcstoul(psz, &endptr, 16);
				}
				else
				{
					nVal = wcstoul(psz, &endptr, 10);
				}
				if (endptr && endptr!=psz)
					DoErrLookup(hDlg);
				else
					DoResolve(hDlg);
			}
		}
		return FALSE;
	}
	else if (Msg == DN_BTNCLICK)
	{
		switch (Param1)
		{
		case ResolveBtnId:
			{
				DoResolve(hDlg);
			}
			break;
		case ErrLookBtnId:
			{
				DoErrLookup(hDlg);
			}
			break;
		case Copy1BtnId:
		case Copy2BtnId:
		case Copy3BtnId:
			{
				INT_PTR nName = psi.SendDlgMessage(hDlg, DM_GETTEXTLENGTH, ExprId, 0);
				INT_PTR nType = psi.SendDlgMessage(hDlg, DM_GETTEXTLENGTH, TypeId, 0);
				INT_PTR nVal  = psi.SendDlgMessage(hDlg, DM_GETTEXTLENGTH, ValueId, 0);
				TCHAR *pszName = (TCHAR*)calloc(nName+1,sizeof(TCHAR));
				TCHAR *pszType = (TCHAR*)calloc(nType+1,sizeof(TCHAR));
				TCHAR *pszVal  = (TCHAR*)calloc(nVal+1,sizeof(TCHAR));
				
				psi.SendDlgMessage(hDlg, DM_GETTEXTPTR, ExprId, (FARDLGPARM)pszName);
				psi.SendDlgMessage(hDlg, DM_GETTEXTPTR, TypeId, (FARDLGPARM)pszType);
				psi.SendDlgMessage(hDlg, DM_GETTEXTPTR, ValueId, (FARDLGPARM)pszVal);

				HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE|GMEM_ZEROINIT, (nName+nType+nVal+32)*sizeof(TCHAR));
				TCHAR *pszAll = (TCHAR*)GlobalLock(hMem);

				switch (Param1)
				{
				case Copy1BtnId: // Value only
					lstrcpy(pszAll, pszVal);
					break;
				case Copy2BtnId: // <type> <name> = <value>
					wsprintf(pszAll, _T("%s %s = %s"), pszType, pszName, pszVal);
					break;
				case Copy3BtnId: // value/*name*/
					wsprintf(pszAll, _T("%s/*%s*/"), pszVal, pszName);
					break;
				}

				GlobalUnlock(hMem);
				if (OpenClipboard(NULL))
				{
					EmptyClipboard();
					SetClipboardData(
						#ifdef _UNICODE
						CF_UNICODETEXT
						#else
						CF_TEXT
						#endif
						, hMem);
					CloseClipboard();
				}
				else
				{
					psi.Message(_PluginNumber(guid_Msg5), FMSG_ALLINONE|FMSG_ERRORTYPE|FMSG_WARNING|FMSG_MB_OK, NULL, 
						(const TCHAR * const *)_T("CPP Resolve\nClipboard failed"), 0, 0);
					GlobalFree(hMem);
				}
			}
			break;
		}
		return TRUE;
	}

    return psi.DefDlgProc(hDlg,Msg,Param1,Param2);
	//return FALSE;
}
