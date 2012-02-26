
/**************************************************************************
Copyright (c) 2009-2010 Maximus5
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

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**************************************************************************/

#include "PVDManager.h"
#include "PVDModuleBase.h"
#include "PVDModule1.h"
#include "PVDModule2.h"
#include "PictureView_Lang.h"
#include "Configure.h"

#ifdef _DEBUG
#define DBGOUTCFG(x) OutputDebugString(x)
#else
#define DBGOUTCFG(x)
#endif


#define POS1 11
#define POS2 (POS1+9)
#define POS3 (POS2+9)
#define POS4 50




CModuleInfo::CModuleInfo(LPCSTR asFrom)
	: CRefRelease(asFrom)
{
	bPrepared = FALSE; // Выставляется в TRUE после проверки версии и пр.
	memset(&ftModified,0,sizeof(ftModified)); // Дата последнего изменения файла
	nVersion = 0;
	pModulePath = 0;  // Полный путь к субплагину (pvd файл)
	pModuleFileName = 0;
	pRegPath = 0;     // Полный путь к ключу субплагина в реестре (настройки, расширения, приоритет, и т.п.)
	hPlugin = 0;       // May be NULL
	nPriority = 0;
	pActive = pInactive = pForbidden = NULL;
	// Собственно интерфейс плагина
	pPlugin = 0;
	szStatus[0] = 0;
	nCurrentFlags = nFlags = nActiveFlags = 0;
}

CModuleInfo::~CModuleInfo()
{
	Unload(); // delete pPlugin;
	//
	if (pRegPath) {
		free(pRegPath); pRegPath = NULL;
	}
	if (pModulePath) {
		free(pModulePath); pModulePath = NULL;
	}
	if (pActive) {
		free(pActive); pActive = NULL;
	}
	if (pInactive) {
		free(pInactive); pInactive = NULL;
	}
	if (pForbidden) {
		free(pForbidden); pForbidden = NULL;
	}
	if (hPlugin) { // В большинстве случаев FreeLibrary выполняет pPlugin. А здесь уже NULL.
		FreeLibrary(hPlugin); hPlugin = NULL;
	}
}

void CModuleInfo::Unload()
{
	_ASSERTE(this!=NULL);
	if (pPlugin) {
		delete pPlugin;
		pPlugin = NULL;
	}
	bPrepared = FALSE;
	SetStatus(GetMsg(MIPluginUnloaded)); // OK
}

bool CModuleInfo::Load(bool abPluginChanged)
{
	_ASSERTE(pPlugin==NULL);
	_ASSERTE(hPlugin==NULL);
	_ASSERTE(pModulePath);

	if (!bPrepared) {
		TRY{
			hPlugin = LoadLibrary(pModulePath);
			if (!hPlugin)
				SetStatus(L"Can't load subplugin library", TRUE);
		}CATCH{
			SetStatus(L"LoadLibrary raised exception", TRUE);
			hPlugin = NULL;
		}

		if (hPlugin) {
			if (GetProcAddress(hPlugin, "pvdInit2")) {
				if (!CPVDManager::CreateVersion2(this, abPluginChanged)) {
					if (GetProcAddress(hPlugin, "pvdInit")) {
						CPVDManager::CreateVersion1(this, abPluginChanged);
					}
				}
			} else if (GetProcAddress(hPlugin, "pvdInit")) {
				CPVDManager::CreateVersion1(this, abPluginChanged);
			} else {
				SetStatus(L"Unknown subplugin version (pvdInit or pvdInit2 not found)", TRUE);
				//Close(); -- Все равно покажем в настройке
				//continue;
			}

			if (!pPlugin && hPlugin) {
				TRY{
					FreeLibrary(hPlugin);
				}CATCH{
				}
				hPlugin = NULL;
			}
		}
		
	} else {
		if (nVersion == 1) {
			CPVDManager::CreateVersion1(this, abPluginChanged);
		} else if (nVersion == 2) {
			CPVDManager::CreateVersion2(this, abPluginChanged);
		}
	}
	
	if (pPlugin)
		SetStatus(L"OK", TRUE); // OK
	
	_ASSERTE(hPlugin == NULL);

	return (pPlugin!=NULL);
}

void CModuleInfo::SetStatus(LPCWSTR asStatus, BOOL abSaveInReg)
{
	const wchar_t* pszName = (pPlugin && pPlugin->pName) ? pPlugin->pName : pModuleFileName;
	CPVDManager::AddLogString(pszName, asStatus);
	
	HKEY hkey = NULL; DWORD dwDisp = 0;
	if (!asStatus) asStatus = L"";
	lstrcpyn(szStatus, asStatus, sizeofarray(szStatus));
	if (abSaveInReg) {
		if (!RegCreateKeyEx(HKEY_CURRENT_USER, pRegPath, 0, 0, 0, KEY_ALL_ACCESS, 0, &hkey, &dwDisp)) {
			RegSetValueEx(hkey, L"Status", 0, REG_SZ, (LPBYTE)asStatus, (lstrlen(asStatus)+1)*2);
			RegCloseKey(hkey); hkey = NULL;
		}
	}
}

DWORD CModuleInfo::Priority()
{
	//if (pPlugin) {
	//	iPriority = pPlugin->iPriority; // "правильным" должен являться хранящийся в pPlugin
	//	return pPlugin->iPriority;
	//}
	return nPriority;
}

void CModuleInfo::SetPriority(DWORD n)
{
#ifdef _DEBUG
	if (nPriority != n) {
		wchar_t szDbg[128];
		wsprintf(szDbg, L"Priority changed from 0x%04X to 0x%04X", Priority(), n);
		SetStatus(szDbg);
	}
#endif

	nPriority = n;
	//if (pPlugin)
	//	pPlugin->iPriority = n;
}



/* ********************** */
/*  Interface Base class  */
/* ********************** */
CPVDModuleBase::CPVDModuleBase(CModuleInfo* apData)
{
	pData = apData;
	pModulePath = apData->pModulePath;
	pRegPath = apData->pRegPath;
	hPlugin = apData->hPlugin;
	apData->hPlugin = NULL;
	pDefActive = pDefInactive = pDefForbidden = NULL;
	nFlags = nCurrentFlags = 0; pName = NULL; pVersion = NULL; pComments = NULL;
	nDefPriority = 0; nVersion = 0;
	pRegSettings = ConcatPath(pRegPath, L"Settings");
	szLastError[0] = 0;
	bCancelled = false;
	mp_Context = NULL;
	mb_DisplayAttached = FALSE;
	bDisplayInitialized = bInitialized = bPrepared = false;
}

// Загрузить приоритет, список форматов и пр.
// Если abForceWriteReg = true - значит файл изменился
// ТРЕБУЕТСЯ загрузить плуг и инициализировать его. Но потом можно загрузить настройки из реестра
bool CPVDModuleBase::InitPrepare(FILETIME ftRegModified, BOOL abPluginChanged)
{
	MCHKHEAP;

	if (abPluginChanged) {
		if (!InitPlugin())
			return false;
	}

	// Т.к. пользователь может перекывать умолчания по расширениям и приоритетам
	// то всегда сначала пытаемся загрузить из реестра - и только в случае
	// неудачи - загружаем pvd файл и обращаемся к его функциям
	if (LoadFormatsFromReg(ftRegModified, abPluginChanged)) {
		bPrepared = true;
		return true;
	}

	if (!abPluginChanged) {
		if (!InitPlugin())
			return false;
	}

	if (!LoadFormatsFromPvd())
		return false;

	SaveFormatsToReg(&ftRegModified);

	bPrepared = true;
	bInitialized = true;
	return true;
}

void CPVDModuleBase::SaveFormatsToReg(FILETIME *pftRegModified)
{
	HKEY hkey = NULL; DWORD dwDisp = 0;
	if (!RegCreateKeyEx(HKEY_CURRENT_USER, pRegPath, 0, 0, 0, KEY_ALL_ACCESS, 0, &hkey, &dwDisp)) {
		DWORD nDataSize = 0;
		// Приоритет сохраняем только явно со страницы модулей диалога настроек
		//RegSetValueEx(hkey, L"Priority", 0, REG_DWORD, (LPBYTE)&iPriority, sizeof(iPriority));
		// Если мы дошли сюда - значит LoadFormatsFromReg обломался, и расширения нужно записать в реестр
		if (nCurrentFlags & PVD_IP_DECODE) {
			if (pDefActive && !pData->pActive)
				RegSetValueEx(hkey, L"ExtensionActive", 0, REG_SZ, (LPBYTE)pDefActive, (wcslen(pDefActive)+1)*2);
			if (pDefInactive && !pData->pInactive)
				RegSetValueEx(hkey, L"ExtensionInactive", 0, REG_SZ, (LPBYTE)pDefInactive, (wcslen(pDefInactive)+1)*2);
			if (pDefForbidden && !pData->pForbidden)
				RegSetValueEx(hkey, L"ExtensionForbidden", 0, REG_SZ, (LPBYTE)pDefForbidden, (wcslen(pDefForbidden)+1)*2);
		}
		RegSetValueEx(hkey, L"PluginFlags", 0, REG_DWORD, (LPBYTE)&nFlags, sizeof(nFlags));
		RegSetValueEx(hkey, L"PluginCurrentFlags", 0, REG_DWORD, (LPBYTE)&nCurrentFlags, sizeof(nCurrentFlags));
		if (pName)
			RegSetValueEx(hkey, L"PluginName", 0, REG_SZ, (LPBYTE)pName, (wcslen(pName)+1)*2);
		if (pVersion)
			RegSetValueEx(hkey, L"PluginVersion", 0, REG_SZ, (LPBYTE)pVersion, (wcslen(pVersion)+1)*2);
		if (pComments)
			RegSetValueEx(hkey, L"PluginComments", 0, REG_SZ, (LPBYTE)pComments, (wcslen(pComments)+1)*2);
		RegSetValueEx(hkey, L"Version", 0, REG_DWORD, (LPBYTE)&nVersion, sizeof(nVersion));
		RegSetValueEx(hkey, L"Modified", 0, REG_BINARY, (LPBYTE)pftRegModified, sizeof(*pftRegModified));
		wchar_t szStatus[4]; lstrcpy(szStatus,L"OK"); SetStatus(szStatus);
		RegSetValueEx(hkey, L"Status", 0, REG_SZ, (LPBYTE)szStatus, (lstrlen(szStatus)+1)*2);
		RegCloseKey(hkey); hkey = NULL;
	}
}

BOOL CPVDModuleBase::DisplayAttach()
{
	if (mb_DisplayAttached)
		return TRUE;
	
	mb_DisplayAttached = DisplayAttach2(g_Plugin.hWnd, TRUE);
	return mb_DisplayAttached;
}

BOOL CPVDModuleBase::DisplayDetach()
{
	if (!mb_DisplayAttached)
		return TRUE;

	BOOL lbRc = DisplayAttach2(g_Plugin.hWnd, FALSE);
	mb_DisplayAttached = FALSE;
	return lbRc;
}

BOOL CPVDModuleBase::DisplayCheck()
{
	if (!bInitialized) {
		if (!InitPlugin(true))
			return FALSE;
	}

	if (!bDisplayInitialized) {
		pvdInfoDisplayInit2 DsInit = {sizeof(pvdInfoDisplayInit2)};
		_ASSERTE(g_Plugin.hWnd);
		DsInit.hWnd = g_Plugin.hWnd;

		DsInit.nCMYKparts = g_Plugin.nCMYKparts;
		DsInit.pCMYKpalette = g_Plugin.pCMYKpalette;
		DsInit.nCMYKsize = g_Plugin.nCMYKsize;
		DsInit.uCMYK2RGB = g_Plugin.uCMYK2RGB;

		if (!DisplayInit2(&DsInit))
			return FALSE;
	}

	return TRUE;
}

CPVDModuleBase::~CPVDModuleBase()
{
	_ASSERTE(!bDisplayInitialized);
	_ASSERTE(!bInitialized);

	if (hPlugin) {
		TRY {
			FreeLibrary(hPlugin); hPlugin = NULL;
		} CATCH {
			SetException(L"FreeLibrary");
		}
	}

	if (pDefActive) {
		free(pDefActive); pDefActive = NULL;
	}
	if (pDefInactive) {
		free(pDefInactive); pDefInactive = NULL;
	}
	if (pDefForbidden) {
		free(pDefForbidden); pDefForbidden = NULL;
	}
	if (pRegSettings) {
		free(pRegSettings); pRegSettings = NULL;
	}
	if (pName) {
		free(pName); pName = NULL;
	}
	if (pVersion) {
		free(pVersion); pVersion = NULL;
	}
	if (pComments) {
		free(pComments); pComments = NULL;
	}
}

wchar_t* CPVDModuleBase::LoadRegValue(HKEY hkey, LPCWSTR asName) const
{
	wchar_t *psz = NULL;
	DWORD nDataSize = 0;
	if (!RegQueryValueEx(hkey, asName, 0, 0, 0, &nDataSize) && nDataSize) {
		psz = (wchar_t*)calloc(nDataSize+2,1);
		if (RegQueryValueEx(hkey, asName, 0, 0, (LPBYTE)psz, &nDataSize)) {
			free(psz); psz = NULL;
		}
	}
	return psz;
}

bool CPVDModuleBase::IsAllowed(const wchar_t* asExt, bool abAllowAsterisk, bool abAllowInactive, bool abAssumeActive/*=false*/)
{
	// плагин может быть вообще отключен
	_ASSERTE(pData && pData->pActive && pData->pInactive);
	if (wcschr(pData->pForbidden,L'*'))
		return false;

	// теперь остальные проверки
	if (!asExt || !*asExt) asExt = L".";

	bool lbActiveMatch = ExtensionMatch(pData->pActive, asExt);
	if (lbActiveMatch)
		return true;

	if (wcschr(pData->pInactive, L'*') || ExtensionMatch(pData->pInactive, asExt)) {
		return abAllowInactive;
	}

	if (abAssumeActive || (abAllowAsterisk && wcschr(pData->pActive,L'*')) || lbActiveMatch)
		return true;

	return false;
}

void CPVDModuleBase::SetStatus(LPCWSTR asStatus, BOOL abSaveInReg)
{
	if (!asStatus) {
		_ASSERTE(asStatus!=NULL);
		asStatus = L"";
	}

	_ASSERTE(pData!=NULL);
	pData->SetStatus(asStatus);

	OutputDebugString(L"\n"); OutputDebugString(asStatus); OutputDebugString(L"\n");

	//DWORD dwDisp = 0; BOOL bKeyCreated = FALSE;
	//if (hkey == NULL) {
	//	if (RegCreateKeyEx(HKEY_CURRENT_USER, pRegPath, 0, 0, 0, KEY_ALL_ACCESS, 0, &hkey, &dwDisp))
	//		return;
	//	bKeyCreated = TRUE;
	//}

	//RegSetValueEx(hkey, L"Status", 0, REG_SZ, (LPBYTE)asStatus, (lstrlen(asStatus)+1)*2);

	//if (bKeyCreated) {
	//	RegCloseKey(hkey); hkey = NULL;
	//}
}

void CPVDModuleBase::SetException(LPCWSTR asFunction)
{
	wchar_t szMsg[128];
	//wnsprintf(szMsg, sizeofarray(szMsg)-2, L"\n!!! %s raised exception", szMsg);
	lstrcpy(szMsg, L"\n!!! ");
	lstrcpyn(szMsg+5, asFunction, 100);
	lstrcat(szMsg, L" raised exception");
	
	// Пометить в реестре последнюю ошибку
	SetStatus(szMsg+1,TRUE);
	
	// И выкинуть в DebugOutput
	lstrcat(szMsg, L"\n");
	OutputDebugString(szMsg);
}

BOOL CPVDModuleBase::LoadExtensionsFromReg(HKEY hkey, LPCWSTR asName, wchar_t** rpszExt)
{
	BOOL result = FALSE;
	wchar_t *pszLoad = NULL;
	if ((pszLoad = LoadRegValue(hkey, asName))!=NULL) {
		if (*pszLoad)
			CharUpperBuffW(pszLoad, lstrlen(pszLoad));
		if (*rpszExt) free(*rpszExt);
		*rpszExt = pszLoad; pszLoad = NULL;
		result = TRUE;
	}
	return result;
}

// abForceWriteReg - обновить в реестре дату изменения и версию интерфейса
bool CPVDModuleBase::LoadFormatsFromReg(FILETIME ftRegModified, BOOL abForceWriteReg)
{
	bool result = false;
	HKEY hkey = NULL; DWORD dwDisp = 0;
	if (!RegCreateKeyEx(HKEY_CURRENT_USER, pRegPath, 0, 0, 0, KEY_ALL_ACCESS, 0, &hkey, &dwDisp)) {
		DWORD nDataSize = 0;
		bool bFlagsChanged = false, bActiveFlagsChanged = false;
		
		result = true; // Всё необходимое загрузили из реестра, LoadLibrary не требуется
		
		if (!nFlags) {
			if (RegQueryValueEx(hkey, L"PluginFlags", 0, 0, (LPBYTE)&nFlags, &(nDataSize=sizeof(nFlags))))
				result = false;
			if (RegQueryValueEx(hkey, L"PluginCurrentFlags", 0, 0, (LPBYTE)&nCurrentFlags, &(nDataSize=sizeof(nCurrentFlags))))
				result = false;
			//else
			//	nCurrentFlags = nFlags;
		} else {
			bFlagsChanged = true;
		}
		
		if (!pData->nActiveFlags) {
			if (RegQueryValueEx(hkey, L"PluginActiveFlags", 0, 0, (LPBYTE)&(pData->nActiveFlags), &(nDataSize=sizeof(pData->nActiveFlags))))
				pData->nActiveFlags = 0;
				//result = false; -- result не трогаем, т.к. эти флаги получаются НЕ из плагина, а настраиваеются пользователем
		} else {
			bActiveFlagsChanged = true;
		}
		
		if (nCurrentFlags & PVD_IP_DECODE) {
			_ASSERTE(!pDefActive && !pDefInactive && !pDefForbidden);
			if (!LoadExtensionsFromReg(hkey, L"ExtensionActive", &(pData->pActive)))
				result = false;
			if (!LoadExtensionsFromReg(hkey, L"ExtensionInactive", &(pData->pInactive)))
				result = false;
			if (!LoadExtensionsFromReg(hkey, L"ExtensionForbidden", &(pData->pForbidden)))
				result = false;
		}

		if (RegQueryValueEx(hkey, L"Priority", 0, 0, (LPBYTE)&pData->nPriority, &(nDataSize=sizeof(pData->nPriority)))) {
			result = false; pData->nPriority = 0;
		}
		if (RegQueryValueEx(hkey, L"DefaultPriority", 0, 0, (LPBYTE)&nDefPriority, &(nDataSize=sizeof(nDefPriority)))) {
			result = false; nDefPriority = 1;
		} else if (!nDefPriority) {
			nDefPriority = 1;
		}
		//if (!nPriority)
		//	nPriority = pData->nPriority;
			
		if (!pName) if ((pName = LoadRegValue(hkey, L"PluginName"))==NULL)
			result = false;

		if (!pVersion) if ((pVersion = LoadRegValue(hkey, L"PluginVersion"))==NULL)
			result = false;

		if (!pComments) if ((pComments = LoadRegValue(hkey, L"PluginComments"))==NULL)
			result = false;



		if (result && abForceWriteReg) {
			RegSetValueEx(hkey, L"Version", 0, REG_DWORD, (LPBYTE)&nVersion, sizeof(nVersion));
			RegSetValueEx(hkey, L"Modified", 0, REG_BINARY, (LPBYTE)&ftRegModified, sizeof(ftRegModified));
			if (bFlagsChanged) {
				RegSetValueEx(hkey, L"PluginFlags", 0, REG_DWORD, (LPBYTE)&nFlags, sizeof(nFlags));
				RegSetValueEx(hkey, L"PluginCurrentFlags", 0, REG_DWORD, (LPBYTE)&nCurrentFlags, sizeof(nCurrentFlags));
			}
			if (bActiveFlagsChanged)
				RegSetValueEx(hkey, L"PluginActiveFlags", 0, REG_DWORD, (LPBYTE)&pData->nActiveFlags, sizeof(pData->nActiveFlags));
			if (pName)
				RegSetValueEx(hkey, L"PluginName", 0, REG_SZ, (LPBYTE)pName, (wcslen(pName)+1)*2);
			if (pVersion)
				RegSetValueEx(hkey, L"PluginVersion", 0, REG_SZ, (LPBYTE)pVersion, (wcslen(pVersion)+1)*2);
			if (pComments)
				RegSetValueEx(hkey, L"PluginComments", 0, REG_SZ, (LPBYTE)pComments, (wcslen(pComments)+1)*2);
		}

		RegCloseKey(hkey); hkey = NULL;
	}
	if (result) {
		pData->nCurrentFlags = nCurrentFlags;
		pData->nFlags = nFlags;
	}
	return result;
}

BOOL CPVDModuleBase::TranslateError2(DWORD nErrNumber, wchar_t *pszErrInfo, int nBufLen)
{
	if (nBufLen < 20)
		return FALSE;
	wsprintf(pszErrInfo, L"ErrCode=0x%08X", nErrNumber);
	SetStatus(pszErrInfo);
	return TRUE;
}


/* ********************* */
/*  Configuration dialog */
/* ********************* */

CPVDModuleConfigDlg::CPVDModuleConfigDlg(CPVDModuleConfig* pConfig)
	: MPicViewDlg(Title(pConfig->pData->pModuleFileName), 60, 13, pConfig->pData->pModuleFileName)
{
	mp_Config = pConfig;
	Reset();
}

wchar_t* CPVDModuleConfigDlg::Title(const wchar_t* asModule)
{
	lstrcpyn(ms_Title, asModule, 63);
	lstrcpyn(ms_Title+lstrlen(ms_Title), GetMsg(MIAddTitleConfig), 64);
	return ms_Title;
}

void CPVDModuleConfigDlg::Reset()
{
	memset(m_PvdItems, 0, sizeof(m_PvdItems));
	mn_PvdItems = 0;
	iActiveExt = iInactiveExt = iForbiddenExt = iResetExt = iOk = iCancel = 0;
	/*iPName = iPVer = iPComm = iPFlags =*/
	iPLog = iReload = iUnload = iAbout = 0;

	MPicViewDlg::Reset();
}

CPVDModuleConfigDlg::~CPVDModuleConfigDlg()
{
	FreeItems();
}

LONG_PTR CPVDModuleConfigDlg::ConfigDlgProc(HANDLE hDlg, int Msg, int Param1, LONG_PTR Param2)
{
	if (Msg == DN_INITDIALOG)
	{
		//EditorSelect es = {BTYPE_NONE};
		//COORD cr = {0,0};
		//LONG_PTR lRc = 0;
		//lRc = g_StartupInfo.SendDlgMessage(mh_Dlg, DM_SETCURSORPOS, iPName, (LONG_PTR)&cr);
		//lRc = g_StartupInfo.SendDlgMessage(mh_Dlg, DM_SETCURSORPOS, iPVer, (LONG_PTR)&cr);
		//lRc = g_StartupInfo.SendDlgMessage(mh_Dlg, DM_SETCURSORPOS, iPComm, (LONG_PTR)&cr);
		//lRc = g_StartupInfo.SendDlgMessage(mh_Dlg, DM_SETSELECTION, iPName, (LONG_PTR)&es);
		//lRc = g_StartupInfo.SendDlgMessage(mh_Dlg, DM_SETSELECTION, iPVer, (LONG_PTR)&es);
		//lRc = g_StartupInfo.SendDlgMessage(mh_Dlg, DM_SETSELECTION, iPComm, (LONG_PTR)&es);
	}
	if (Msg == DN_BTNCLICK && Param1 == iResetExt)
	{
		if (mp_Config->pData->pPlugin)
		{
			if (mp_Config->pData->pPlugin->LoadFormatsFromPvd())
			{
				//mp_Config->pData->pPlugin->SaveFormatsToReg();
				g_StartupInfo.SendDlgMessage(hDlg, DM_SETTEXTPTR, iActiveExt, (DLG_LPARAM)mp_Config->pData->pPlugin->pDefActive);
				g_StartupInfo.SendDlgMessage(hDlg, DM_SETTEXTPTR, iInactiveExt, (DLG_LPARAM)mp_Config->pData->pPlugin->pDefInactive);
				g_StartupInfo.SendDlgMessage(hDlg, DM_SETTEXTPTR, iForbiddenExt, (DLG_LPARAM)mp_Config->pData->pPlugin->pDefForbidden);
			}
			g_StartupInfo.SendDlgMessage(hDlg, DM_SETTEXTPTR, iPLog, (DLG_LPARAM)mp_Config->pData->szStatus);
			g_StartupInfo.SendDlgMessage(hDlg, DM_SETFOCUS, iActiveExt, 0);
		}
		return TRUE;
	}
	if ((Msg == DN_BTNCLICK && Param1 == iAbout)
		#ifdef FAR_UNICODE
			|| (Msg == DN_CONTROLINPUT && GetFarKey(Param2) == KEY_F3)
		#else
			|| (Msg == DN_KEY && Param2 == KEY_F3)
		#endif
		)
	{
		mp_Config->About();
		return TRUE;
	}

	return g_StartupInfo.DefDlgProc(hDlg, Msg, Param1, (DLG_LPARAM)Param2);
}

BOOL CPVDModuleConfigDlg::LoadExtensionsFromDlg(int nID, wchar_t** rpszExt)
{
	BOOL result = TRUE;

	int nLen = (int)g_StartupInfo.SendDlgMessage(mh_Dlg, DM_GETTEXTPTR, nID, 0);

	if (*rpszExt && nLen > lstrlen(*rpszExt)) {
		free(*rpszExt); *rpszExt = NULL;
	}
	if ((*rpszExt) == NULL)
		*rpszExt = (wchar_t*)calloc(nLen+1,sizeof(wchar_t));

	g_StartupInfo.SendDlgMessage(mh_Dlg, DM_GETTEXTPTR, nID, (DLG_LPARAM)*rpszExt);

	if (**rpszExt) {
		CharUpperBuff(*rpszExt, lstrlen(*rpszExt));
		CPVDModuleVer2::apiSortExtensions(*rpszExt);
	}

	return result;
}

bool CPVDModuleConfigDlg::Run()
{
	bool lbRc = false;
lReopen:
	int Y = 2;
	int nVerTextLen = 0;
	LPCTSTR pszText = NULL;

	nVerTextLen = max(lstrlen(GetMsg(MISubpluginVersionI1)),lstrlen(GetMsg(MISubpluginVersionI2)));
	if (nVerTextLen < 20) nVerTextLen = 20;
	
	if (mp_Config->pData->nCurrentFlags & PVD_IP_DECODE)
	{
		addFarDialogItem(
			DI_TEXT,     5, Y++, 0, 0, 0, 0, 0, 0, GetMsg(MISubpluginActiveExtensions));
		iActiveExt = addFarDialogItem(
			DI_EDIT,     5, Y++, DialogWidth+4, 0, 0, 0, 0, 0, mp_Config->pActive);

		addFarDialogItem(
			DI_TEXT,     5, Y, 0, 0, 0, 0, 0, 0, GetMsg(MISubpluginInactiveExtensions));
		iInactiveExt = addFarDialogItem(
			DI_EDIT,     5, Y+1, (DialogWidth/2)-2, 0, 0, 0, 0, 0, mp_Config->pInactive);

		addFarDialogItem(
			DI_TEXT,     DialogWidth/2, Y, 0, 0, 0, 0, 0, 0, GetMsg(MISubpluginForbiddenExtensions));
		iForbiddenExt = addFarDialogItem(
			DI_EDIT,     DialogWidth/2, Y+1, DialogWidth+4, 0, 0, 0, 0, 0, mp_Config->pForbidden);
		Y += 2;

		pszText = GetMsg(MISubpluginResetExt);
		iResetExt = addFarDialogItem(
			DI_BUTTON,   DialogWidth-lstrlen(pszText)+1, Y-4, 0, 0, 0, 0, mp_Config->pData->pPlugin ? 0 : DIF_DISABLE, 0, pszText);
		
		addFarDialogItem(
			DI_TEXT, 5, Y++, 0, 0, 0, 0, DIF_BOXCOLOR | DIF_SEPARATOR, 0, 0/*GetMsg(MIIgnoredExtensions)*/);
	}
	
	/*
	// Имя
	addFarDialogItem(DI_TEXT,     5, Y, 0, 0, 0, 0, 0, 0, GetMsg(MISubpluginName));
	if (mp_Config->pData->pPlugin) pszText = mp_Config->pData->pPlugin->pName; else pszText = mp_Config->pData->pModuleFileName;
	iPName = addFarDialogItem(DI_EDIT,     5, Y+1, DialogWidth-nVerTextLen+3, 0, 0, 0, DIF_READONLY, 0, pszText, 0);
	// Версия (интерфейса и самого плагина)
	if (mp_Config->pData->pPlugin) pszText = GetMsg((mp_Config->pData->pPlugin->nVersion==1) ? MISubpluginVersionI1 : MISubpluginVersionI2); else pszText = GetMsg(MISubpluginVersion);
	addFarDialogItem(DI_TEXT,     DialogWidth-nVerTextLen+5, Y, 0, 0, 0, 0, 0, 0, pszText);
	if (mp_Config->pData->pPlugin) pszText = mp_Config->pData->pPlugin->pVersion; else pszText = L"???";
	iPVer = addFarDialogItem(DI_EDIT,     DialogWidth-nVerTextLen+5, Y+1, DialogWidth+4, 0, 0, 0, DIF_READONLY, 0, pszText, 0);
	Y+=2;
	// Комментарий (информация из субплагина)
	addFarDialogItem(DI_TEXT,     5, Y, 0, 0, 0, 0, 0, 0, GetMsg(MISubpluginComments));
	if (mp_Config->pData->pPlugin) pszText = mp_Config->pData->pPlugin->pComments; else pszText = L"";
	iPComm = addFarDialogItem(DI_EDIT,     5, Y+1, DialogWidth-nVerTextLen+3, 0, 0, 0, DIF_READONLY, 0, pszText, 0);
	// Флаги субплагина
	memset(&ModuleFlags, 0, sizeof(ModuleFlags)); memset(ModuleFlagItems, 0, sizeof(ModuleFlagItems));
	ModuleFlags.Items = ModuleFlagItems;
	DWORD nFlags = mp_Config->pData->pPlugin ? mp_Config->pData->pPlugin->nFlags : mp_Config->pData->nCurrentFlags;
	#define ADDFLAG(n,s) if (nFlags & n) { ModuleFlagItems[ModuleFlags.ItemsNumber++].Text = s; nFlags &= ~n; }
	ADDFLAG(PVD_IP_DECODE,L"Decoder");
	ADDFLAG(PVD_IP_TRANSFORM,L"Transform");
	ADDFLAG(PVD_IP_DISPLAY,L"Display");
	ADDFLAG(PVD_IP_PRIVATE,L"Private display");
	ADDFLAG(PVD_IP_DIRECT,L"Direct to video");
	ADDFLAG(PVD_IP_NOTERMINAL,L"No terminal");
	ADDFLAG(PVD_IP_PROCESSING,L"PostProcessing");
	ADDFLAG(PVD_IP_MULTITHREAD,L"Multithread");
	ADDFLAG(PVD_IP_ALLOWCACHE,L"Allow caching");
	ADDFLAG(PVD_IP_CANREFINE,L"Can refine");
	ADDFLAG(PVD_IP_CANREFINERECT,L"Can refine rect");
	ADDFLAG(PVD_IP_CANREDUCE,L"Can reduce");
	if (nFlags) {
		wsprintf(szFlagsLeft, L"0x%08X", nFlags);
		ModuleFlagItems[ModuleFlags.ItemsNumber++].Text = szFlagsLeft;
	}
	if (ModuleFlags.ItemsNumber) ModuleFlagItems[0].Flags = LIF_SELECTED;
	addFarDialogItem(DI_TEXT,     DialogWidth-nVerTextLen+5, Y, 0, 0, 0, 0, 0, 0, GetMsg(MISubpluginFlags));
	iPFlags = addFarDialogItem(DI_COMBOBOX, DialogWidth-nVerTextLen+5, Y+1, DialogWidth+4, Y, 0, 0, DIF_DROPDOWNLIST, 0, 0);
	DialogItems[iPFlags].Param.ListItems = &ModuleFlags;
	Y+=2;
	// Полный путь к субплагину
	addFarDialogItem(DI_TEXT,     5, Y++, 0, 0, 0, 0, 0, 0, GetMsg(MISubpluginFullPathName));
	pszText = mp_Config->pData->pModulePath;
	addFarDialogItem(DI_EDIT,     5, Y++, DialogWidth+4, 0, 0, 0, DIF_READONLY, 0, pszText, 0);
	*/

	if (LoadItems() > 0) {
		//addFarDialogItem(
		//	DI_TEXT, 5, Y++, 0, 0, 0, 0, DIF_BOXCOLOR | DIF_SEPARATOR, 0, 0/*GetMsg(MIIgnoredExtensions)*/);

		int nMaxHeight = 10;
		for (int i = 0; nMaxHeight > 0 && i < mn_PvdItems; i++) {
			TODO("Обработка 'hex:00=CMYK:01:FastRGB;Decode CMYK images in'");
			if (!lstrcmpi(m_PvdItems[i].pszType, L"bool")) {
				m_PvdItems[i].nID = addFarDialogItem(DI_CHECKBOX, 5, Y++, DialogWidth+4, 0, 0, m_PvdItems[i].bData, 0, 0, m_PvdItems[i].pszLabel);
				nMaxHeight --;
			} else if (!lstrcmpi(m_PvdItems[i].pszType, L"string")) {
				addFarDialogItem(DI_TEXT,     5, Y++, 0, 0, 0, 0, 0, 0, m_PvdItems[i].pszLabel);
				m_PvdItems[i].nID = addFarDialogItem(DI_EDIT,     5, Y++, DialogWidth+4, 0, 0, 0, 0, 0, m_PvdItems[i].pszData);
				nMaxHeight -= 2;
			} else if (!lstrcmpi(m_PvdItems[i].pszType, L"long")) {
				addFarDialogItem(DI_TEXT,     5, Y++, 0, 0, 0, 0, 0, 0, m_PvdItems[i].pszLabel);
				m_PvdItems[i].pszData = (wchar_t*)calloc(12,sizeof(wchar_t));
				wsprintf(m_PvdItems[i].pszData, L"%i", (int)m_PvdItems[i].nData);
				m_PvdItems[i].nID = addFarDialogItem(DI_EDIT,     5, Y++, DialogWidth+4, 0, 0, 0, 0, 0, m_PvdItems[i].pszData);
				nMaxHeight -= 2;
			} else if (!lstrcmpi(m_PvdItems[i].pszType, L"dword")) {
				addFarDialogItem(DI_TEXT,     5, Y++, 0, 0, 0, 0, 0, 0, m_PvdItems[i].pszLabel);
				m_PvdItems[i].pszData = (wchar_t*)calloc(12,sizeof(wchar_t));
				wsprintf(m_PvdItems[i].pszData, L"0x%08X", m_PvdItems[i].nData);
				m_PvdItems[i].nID = addFarDialogItem(DI_EDIT,     5, Y++, DialogWidth+4, 0, 0, 0, 0, 0, m_PvdItems[i].pszData);
				nMaxHeight -= 2;
			}
		}
		//TO DO("добавить кнопку 'Add' - создает новый параметр с запросом имени");
	}

	// Статус субплагина
	addFarDialogItem(DI_TEXT,     5, Y++, 0, 0, 0, 0, DIF_BOXCOLOR | DIF_SEPARATOR, 0, GetMsg(MISubpluginMessages));
	iPLog = addFarDialogItem(DI_EDIT,     5, Y++, DialogWidth+4, 0, 0, 0, DIF_READONLY, 0, mp_Config->pData->szStatus);


	// И последнее - кнопки OK / Cancel
	addFarDialogItem(
		DI_TEXT, 5, Y, 0, 0, 0, 0, DIF_BOXCOLOR | DIF_SEPARATOR, 0, 0/*GetMsg(MIIgnoredExtensions)*/);
	iReload = addFarDialogItem(
		DI_BUTTON, 0, Y, 0, 0, 0, 0, DIF_CENTERGROUP, 0, GetMsg(MISubpluginReload));
	iUnload = addFarDialogItem(
		DI_BUTTON, 0, Y, 0, 0, 0, 0, DIF_CENTERGROUP | (mp_Config->pData->pPlugin ? 0 : DIF_DISABLE), 0, GetMsg(MISubpluginUnload));
	iAbout = addFarDialogItem(
		DI_BUTTON, 0, Y, 0, 0, 0, 0, DIF_CENTERGROUP | (mp_Config->pData->pPlugin ? 0 : DIF_DISABLE), 0, GetMsg(MISubpluginAbout));

	Y++;
	iOk = addFarDialogItem(
		DI_BUTTON, 0, Y, 0, 0, 0, 0, DIF_CENTERGROUP, TRUE, GetMsg(MIOK));
	iCancel = addFarDialogItem(
		DI_BUTTON, 0, Y, 0, 0, 0, 0, DIF_CENTERGROUP, 0, GetMsg(MICancel));

	
	int ExitCode = -1;
	if (DialogInit()) {
		ExitCode = g_StartupInfo.DialogRun(mh_Dlg);
		// По кнопкам Unload/Reload тоже сохраняем
		if (ExitCode == iOk || ExitCode == iReload || ExitCode == iUnload) {
			int nLen = 0;
			if (mp_Config->pData->nCurrentFlags & PVD_IP_DECODE)
			{
				// Активные расширения
				LoadExtensionsFromDlg(iActiveExt, &(mp_Config->pActive));
				// Неактивные расширения
				LoadExtensionsFromDlg(iInactiveExt, &(mp_Config->pInactive));
				// Запрещенные расширения
				LoadExtensionsFromDlg(iForbiddenExt, &(mp_Config->pForbidden));
				//
				if (mp_Config->pTitle) free(mp_Config->pTitle); mp_Config->pTitle = NULL;
				mp_Config->CreateTitle();

				// Сразу сохранить расширения в реестр
				_ASSERTE(mp_Config->pActive && mp_Config->pInactive && mp_Config->pForbidden);
				_ASSERTE(mp_Config->pData);
				if (mp_Config->pData->pActive) free(mp_Config->pData->pActive);
				mp_Config->pData->pActive = _wcsdup(mp_Config->pActive);
				if (mp_Config->pData->pInactive) free(mp_Config->pData->pInactive);
				mp_Config->pData->pInactive = _wcsdup(mp_Config->pInactive);
				if (mp_Config->pData->pForbidden) free(mp_Config->pData->pForbidden);
				mp_Config->pData->pForbidden = _wcsdup(mp_Config->pForbidden);
				
				HKEY hkey = NULL; DWORD dwDisp = 0;
				if (!RegCreateKeyEx(HKEY_CURRENT_USER, mp_Config->pData->pRegPath, 0, 0, 0, KEY_ALL_ACCESS, 0, &hkey, &dwDisp))
				{
					DWORD nDataSize = 0;
					RegSetValueEx(hkey, L"ExtensionActive", 0, REG_SZ, 
						(LPBYTE)mp_Config->pActive, (wcslen(mp_Config->pActive)+1)*2);
					RegSetValueEx(hkey, L"ExtensionInactive", 0, REG_SZ, 
						(LPBYTE)mp_Config->pInactive, (wcslen(mp_Config->pInactive)+1)*2);
					RegSetValueEx(hkey, L"ExtensionForbidden", 0, REG_SZ, 
						(LPBYTE)mp_Config->pForbidden, (wcslen(mp_Config->pForbidden)+1)*2);
					RegCloseKey(hkey); hkey = NULL;
				}
			}

			// Строки внутренней настройки субплагина
			if (mn_PvdItems > 0)
			{
				HKEY hKey = 0, hRoot = 0;
				// Объект плагина (mp_Config->pData->pPlugin) мог быть не создан, если были ошибки инициализации
				if (!RegOpenKeyEx(HKEY_CURRENT_USER, mp_Config->pData->pRegPath, 0, KEY_WRITE, &hRoot)) {
					if (!RegOpenKeyEx(hRoot, L"Settings", 0, KEY_WRITE, &hKey)) {
						for (int i=0; i<mn_PvdItems; i++) {
							if (m_PvdItems[i].nType == REG_SZ || m_PvdItems[i].nType == REG_DWORD
								|| (m_PvdItems[i].nType == REG_BINARY && m_PvdItems[i].nLen == 4))
							{
								nLen = (int)g_StartupInfo.SendDlgMessage(mh_Dlg, DM_GETTEXTPTR, m_PvdItems[i].nID, 0);
								if (nLen >= lstrlen(m_PvdItems[i].pszData)) {
									free(m_PvdItems[i].pszData);
									if (!(m_PvdItems[i].pszData = (wchar_t*)calloc(nLen+1,sizeof(wchar_t)))) break; // ошибка выделения памяти
								}
								g_StartupInfo.SendDlgMessage(mh_Dlg, DM_GETTEXTPTR, m_PvdItems[i].nID, (DLG_LPARAM)m_PvdItems[i].pszData);

								if (m_PvdItems[i].nType == REG_SZ) {
									RegSetValueEx(hKey, m_PvdItems[i].szName, 0, m_PvdItems[i].nType, 
										(LPBYTE)m_PvdItems[i].pszData, (lstrlen(m_PvdItems[i].pszData)+1)*sizeof(wchar_t));
								} else {
									wchar_t* pszEnd = NULL;
									if (m_PvdItems[i].pszData[0]==L'0' && (m_PvdItems[i].pszData[1]==L'x' || m_PvdItems[i].pszData[1]==L'X'))
										m_PvdItems[i].nData = wcstoul(m_PvdItems[i].pszData, &pszEnd, 16);
									else
										m_PvdItems[i].nData = (DWORD)wcstol(m_PvdItems[i].pszData, &pszEnd, 10);
									RegSetValueEx(hKey, m_PvdItems[i].szName, 0, m_PvdItems[i].nType, 
										(LPBYTE)&(m_PvdItems[i].nData), 4);
								}

							} else if (m_PvdItems[i].nType == REG_BINARY && m_PvdItems[i].nLen == 1) {
								TODO("Здесь может быть радиокнопка - тогда нужно по другому, с обработкой nCount");
								m_PvdItems[i].bData = DlgItem_GetCheck(g_StartupInfo, mh_Dlg, m_PvdItems[i].nID);
								RegSetValueEx(hKey, m_PvdItems[i].szName, 0, m_PvdItems[i].nType, 
									(LPBYTE)&(m_PvdItems[i].bData), 1);
							}					
						}
						RegCloseKey(hKey); hKey = 0;
					}
					RegCloseKey(hRoot); hRoot = 0;
				}

				if (mp_Config->pData->pPlugin) { // Объект плагина мог быть не создан, если были ошибки инициализации
					mp_Config->pData->pPlugin->ReloadConfig2();
				}
			}
			
			
		} else {
			// -- по идее, измениться могли только расширения по умолчанию, а текущие трогаться не должны были
			#ifdef _DEBUG
			int nDbg = 1;
			#endif
			// В результате нажатия кнопки "Reset" расширения могли измениться. Их нужно вернуть
			//if (mp_Config->pSupported && mp_Config->pData->pPlugin) {
			//	if (!mp_Config->pData->pPlugin->pSupported || lstrcmp(mp_Config->pData->pPlugin->pSupported, mp_Config->pSupported)) {
			//		if (mp_Config->pData->pPlugin->pSupported) free(mp_Config->pData->pPlugin->pSupported);
			//		mp_Config->pData->pPlugin->pSupported = _wcsdup(mp_Config->pSupported);
			//	}
			//}
			//if (mp_Config->pIgnored && mp_Config->pData->pPlugin) {
			//	if (!mp_Config->pData->pPlugin->pIgnored || lstrcmp(mp_Config->pData->pPlugin->pIgnored, mp_Config->pIgnored)) {
			//		if (mp_Config->pData->pPlugin->pIgnored) free(mp_Config->pData->pPlugin->pIgnored);
			//		mp_Config->pData->pPlugin->pIgnored = _wcsdup(mp_Config->pIgnored);
			//	}
			//}
		}
		DialogFree();
		
		if (ExitCode == iOk || ExitCode == iReload || ExitCode == iUnload)
			lbRc = true; // НЕ сбрасывать, т.к. оно могло уже измениться...
	}
	FreeItems();
	
	// Unload/Reload
	if (ExitCode == iReload || ExitCode == iUnload)
	{
		bool bReload = (ExitCode == iReload);
		
		mp_Config->pData->Unload();
		
		if (bReload) {
			mp_Config->pData->Load(true); // force

			memset(m_PvdItems, 0, sizeof(m_PvdItems));
			mn_PvdItems = 0;
			Reset();
			
			if (mp_Config->pTitle) free(mp_Config->pTitle); mp_Config->pTitle = NULL;
			mp_Config->CreateTitle();
			gpCurConfig->UpdateTitleFor(mp_Config);

			goto lReopen;
		} else {
			if (mp_Config->pTitle) free(mp_Config->pTitle); mp_Config->pTitle = NULL;
			mp_Config->CreateTitle();
			gpCurConfig->UpdateTitleFor(mp_Config);
		}
	}
	
	return lbRc;
}

// Загрузить из реестра параметры модуля (каждая строка содержит описание, тип, и значение параметра)
// Модуль при первой загрузке должен создать в своем подключе реестра параметры по умолчанию
// Пример.
// [HKEY_CURRENT_USER\Software\Far2\Plugins\PictureView2\WIC.pvd\Settings\Description]
// "InterpolationMode"="long;Interpolation quality (0..3)"
// "RotateOnExif"="bool;Rotate on EXIF"
int CPVDModuleConfigDlg::LoadItems()
{
	mn_PvdItems = 0;
	HKEY hKey = 0, hDesc = 0, hPlugin = 0;
	PvdItem pi = {0};
	if (!RegOpenKeyEx(HKEY_CURRENT_USER, mp_Config->pData->pRegPath, 0, KEY_READ, &hPlugin)) {
		if (!RegOpenKeyEx(hPlugin, L"Settings", 0, KEY_READ, &hKey)) {
			if (RegOpenKeyEx(hKey, L"Description", 0, KEY_READ, &hDesc)) hDesc = NULL;

			wchar_t szName[MAX_PATH]; DWORD nNameLen, nLen, nType, nDescLen, nDescType;
			DWORD nIndex = 0; LSTATUS nDescRc = 0;
			while (mn_PvdItems < sizeofarray(m_PvdItems) &&
				!RegEnumValue(hKey, nIndex++, szName, &(nNameLen = sizeofarray(szName)), 0, &nType, 0, &(nLen=0)))
			{
				memset(&pi, 0, sizeof(pi));
				if (hDesc) {
					if (!(nDescRc = RegQueryValueEx(hDesc, szName, 0, &(nDescType=0), 0, &(nDescLen=0)))
						&& nDescType == REG_SZ && nDescLen)
					{
						if (!(pi.pszType = (wchar_t*)calloc(nDescLen,1))) break;
						if (!(nDescRc = RegQueryValueEx(hDesc, szName, 0, 0, (LPBYTE)pi.pszType, &nDescLen))) {
							wchar_t* psz = wcschr(pi.pszType, L';');
							if (!psz) {
								nDescRc = -1; free(pi.pszType); pi.pszType = NULL;
							} else {
								*psz = 0; pi.pszLabel = psz+1;
							}
						} else {
							free(pi.pszType); pi.pszType = NULL;
						}
					}
				}

				if (nType == REG_SZ)
				{
					if (!(pi.pszData = (wchar_t*)calloc(nLen,1))) break;
					if (RegQueryValueEx(hKey, szName, 0, 0, (LPBYTE)pi.pszData, &nLen)) {
						free(pi.pszData); continue;
					}
					pi.nLen = nLen; pi.nType = nType;
					if (!pi.pszType) {
						if (!(pi.pszType = (wchar_t*)calloc(lstrlen(szName)+20,sizeof(wchar_t)))) break;
						lstrcpy(pi.pszType, L"string"); pi.pszLabel=pi.pszType+lstrlen(pi.pszType)+1; lstrcpy((wchar_t*)pi.pszLabel, szName);
					} else {
						if (lstrcmpi(pi.pszType, L"string")) continue;
					}
				} else
				if (nType == REG_DWORD || (nLen == 4 && nType == REG_BINARY))
				{
					if (RegQueryValueEx(hKey, szName, 0, 0, (LPBYTE)&pi.nData, &(nLen=sizeof(pi.nData))))
						continue;
					pi.nLen = nLen; pi.nType = nType;
					if (!pi.pszType) {
						if (!(pi.pszType = (wchar_t*)calloc(lstrlen(szName)+20,sizeof(wchar_t)))) break;
						lstrcpy(pi.pszType, L"dword"); pi.pszLabel=pi.pszType+lstrlen(pi.pszType)+1; lstrcpy((wchar_t*)pi.pszLabel, szName);
					} else {
						if (lstrcmpi(pi.pszType, L"long") && lstrcmpi(pi.pszType, L"dword")) continue;
					}
				} else
				if (nType == REG_BINARY && nLen == 1)
				{
					if (RegQueryValueEx(hKey, szName, 0, 0, (LPBYTE)&pi.bData, &(nLen=sizeof(pi.bData))))
						continue;
					pi.nLen = nLen; pi.nType = nType;
					if (!pi.pszType) {
						if (!(pi.pszType = (wchar_t*)calloc(lstrlen(szName)+20,sizeof(wchar_t)))) break;
						lstrcpy(pi.pszType, L"bool"); pi.pszLabel=pi.pszType+lstrlen(pi.pszType)+1; lstrcpy((wchar_t*)pi.pszLabel, szName);
					} else {
						TODO("Здесь может быть радиокнопка 'hex:00=CMYK:01:FastRGB;Decode CMYK images in'");
						if (lstrcmpi(pi.pszType, L"bool")) continue;
					}
				} else {
					if (pi.pszType) free(pi.pszType); pi.pszType = NULL;
					continue;
				}

				_ASSERTE(pi.pszType);
				lstrcpy(pi.szName, szName);
				m_PvdItems[mn_PvdItems++] = pi;
				memset(&pi, 0, sizeof(pi));
			}
			if (hDesc) RegCloseKey(hDesc); hDesc = NULL;
			RegCloseKey(hKey); hKey = NULL;
		}
		RegCloseKey(hPlugin); hPlugin = NULL;
	}

	return mn_PvdItems;
}

void CPVDModuleConfigDlg::FreeItems()
{
	int nMaxHeight = 10;
	for (int i = 0; i < sizeofarray(m_PvdItems); i++) {
		if (m_PvdItems[i].pszType) { free(m_PvdItems[i].pszType); m_PvdItems[i].pszType = NULL; }
		m_PvdItems[i].pszLabel = NULL;
		if (m_PvdItems[i].pszData) { free(m_PvdItems[i].pszData); m_PvdItems[i].pszData = NULL; }
		m_PvdItems[i].nID = 0;
	}
}



/* ******************* */
/*  Configuration info */
/* ******************* */

CPVDModuleConfig::CPVDModuleConfig(/*CPVDModuleBase* p,*/ CModuleInfo* apData)
{
	MCHKHEAP;
	//pPlugin = p;
	pData = apData;
	nPriority  = pData->Priority();
	pActive = _wcsdup(pData->pActive ? pData->pActive : L"");
	pInactive = _wcsdup(pData->pInactive ? pData->pInactive : L"");
	pForbidden = _wcsdup(pData->pForbidden ? pData->pForbidden : L"");
	CreateTitle();
}

CPVDModuleConfig::~CPVDModuleConfig()
{
	Free();
}

void CPVDModuleConfig::CreateTitle()
{
	MCHKHEAP;
	size_t nLen = 0;
	const wchar_t *pName = pData->pPlugin ? pData->pPlugin->pName : pData->pModuleFileName;
	//if (pData->pPlugin) {
	//	nLen = lstrlen(pData->pPlugin->pName)
	//		+(pSupported ? lstrlen(pSupported) : 0)
	//		+(pIgnored ? lstrlen(pIgnored) : 0)
	//		+POS3;
	//	pName = pData->pPlugin->pName;
	//} else {
	//	pName = wcsrchr(pData->pModulePath, L'\\');
	//	if (pName) pName++; else if (pData->pModulePath) pName = pData->pModulePath; else pName = L"<NULL>";
	//	nLen = lstrlen(pName) + POS3;
	//}
	nLen = POS4 + 10;
	DBGOUTCFG(pName); DBGOUTCFG(L":CreateTitle...");
	pTitle = (wchar_t*)calloc(nLen,sizeof(wchar_t));
	/*
	GDI+.pvd   |      |BMP,DIB,EMF,WMF,JPG,J>
	*/
	lstrcpyn(pTitle, pName, POS1+2);
	nLen = lstrlen(pTitle);
	if (nLen>=POS1) { pTitle[POS1-1] = L'…'; nLen = POS1; }
	while (nLen<POS1) pTitle[nLen++] = L' ';
	pTitle[nLen++] = L'|';
	if (pData->pPlugin) {
		MCHKHEAP;
		if (pData->nCurrentFlags & PVD_IP_DECODE) {
			if (pForbidden) {
				lstrcpyn(pTitle+nLen, pForbidden, POS2-POS1+2);
				nLen = lstrlen(pTitle);
			}
			if (nLen>=POS2) { pTitle[POS2-1] = L'…'; nLen = POS2; }
			while (nLen<POS2) pTitle[nLen++] = L' ';
			pTitle[nLen++] = L'|';

			if (pInactive) {
				lstrcpyn(pTitle+nLen, pInactive, POS3-POS2+2);
				nLen = lstrlen(pTitle);
			}
			if (nLen>=POS3) { pTitle[POS3-1] = L'…'; nLen = POS3; }
			while (nLen<POS3) pTitle[nLen++] = L' ';
			pTitle[nLen++] = L'|';

			MCHKHEAP;
			if (pActive) {
				lstrcpyn(pTitle+nLen, pActive, POS4-POS3+2);
				nLen = lstrlen(pTitle);
			}
			pTitle[nLen] = 0;

			MCHKHEAP;

		} else if (pData->nCurrentFlags & PVD_IP_DISPLAY) {
			MCHKHEAP;
			lstrcpyn(pTitle+nLen, GetMsg(MIDisplayModule), POS4-nLen+1);
		} else if (pData->nCurrentFlags & PVD_IP_PROCESSING) {
			MCHKHEAP;
			lstrcpyn(pTitle+nLen, GetMsg(MIPostprocessingModule), POS4-nLen+1);
		} else {
			MCHKHEAP;
			lstrcpyn(pTitle+nLen, GetMsg(MIUnknownModuleType), POS4-nLen+1);
		}
	} else {
		MCHKHEAP;
		lstrcpyn(pTitle+nLen, GetMsg(MIModuleWasNotLoaded), POS4-nLen+1);
	}
	MCHKHEAP;
	if (lstrlen(pTitle) >= POS4) { pTitle[POS4-1] = L'…'; pTitle[POS4] = 0; }
	MCHKHEAP;
	DBGOUTCFG(L"Done\n");
}

void CPVDModuleConfig::Free()
{
	free(pTitle); pTitle = NULL;
	free(pActive); pActive = NULL;
	free(pInactive); pInactive = NULL;
	free(pForbidden); pForbidden = NULL;
}

bool CPVDModuleConfig::Configure()
{
	MCHKHEAP;

	CPVDModuleConfigDlg dlg(this);
	bool lbRc = dlg.Run();
	dlg.DialogFree();

	MCHKHEAP;
	return lbRc;
}

void CPVDModuleConfig::About()
{
	MCHKHEAP;

	CPVDDecoderAboutDlg dlg(this);
	bool lbRc = dlg.Run();
	dlg.DialogFree();

	MCHKHEAP;
}


/* ************* */
/*  Module About */
/* ************* */

CPVDDecoderAboutDlg::CPVDDecoderAboutDlg(CPVDModuleConfig* pConfig)
: MPicViewDlg(Title(pConfig->pData->pModuleFileName), 65, 13, pConfig->pData->pModuleFileName)
{
	mp_Config = pConfig;
	Reset();
}

wchar_t* CPVDDecoderAboutDlg::Title(const wchar_t* asModule)
{
	lstrcpyn(ms_Title, asModule, 63);
	lstrcpyn(ms_Title+lstrlen(ms_Title), GetMsg(MIAddTitleAbout), 64);
	return ms_Title;
}

void CPVDDecoderAboutDlg::Reset()
{
	iOk = iCancel = iPName = iPVer = iPComm = iPFlags = iPLog = 0;
	iActiveExt = iInactiveExt = iForbiddenExt = iPriority = iReloadExt = 0;
	memset(&ModuleFlags, 0, sizeof(ModuleFlags));
	memset(ModuleFlagItems, 0, sizeof(ModuleFlagItems));
	memset(szFlagsLeft, 0, sizeof(szFlagsLeft));
	memset(szPriority, 0, sizeof(szPriority));

	MPicViewDlg::Reset();
}

CPVDDecoderAboutDlg::~CPVDDecoderAboutDlg()
{
}

LONG_PTR CPVDDecoderAboutDlg::ConfigDlgProc(HANDLE hDlg, int Msg, int Param1, LONG_PTR Param2)
{
	if (Msg == DN_INITDIALOG) {
		//EditorSelect es = {BTYPE_NONE};
		//COORD cr = {0,0};
		//LONG_PTR lRc = 0;
		//lRc = g_StartupInfo.SendDlgMessage(mh_Dlg, DM_SETCURSORPOS, iPName, (LONG_PTR)&cr);
		//lRc = g_StartupInfo.SendDlgMessage(mh_Dlg, DM_SETCURSORPOS, iPVer, (LONG_PTR)&cr);
		//lRc = g_StartupInfo.SendDlgMessage(mh_Dlg, DM_SETCURSORPOS, iPComm, (LONG_PTR)&cr);
		//lRc = g_StartupInfo.SendDlgMessage(mh_Dlg, DM_SETSELECTION, iPName, (LONG_PTR)&es);
		//lRc = g_StartupInfo.SendDlgMessage(mh_Dlg, DM_SETSELECTION, iPVer, (LONG_PTR)&es);
		//lRc = g_StartupInfo.SendDlgMessage(mh_Dlg, DM_SETSELECTION, iPComm, (LONG_PTR)&es);
	}

	if (Msg == DN_BTNCLICK && Param1 == iReloadExt) {
		if (mp_Config->pData->pPlugin) {
			if (mp_Config->pData->pPlugin->LoadFormatsFromPvd()) {
				//mp_Config->pData->pPlugin->SaveFormatsToReg();
				g_StartupInfo.SendDlgMessage(hDlg, DM_SETTEXTPTR, iActiveExt, (DLG_LPARAM)mp_Config->pData->pPlugin->pDefActive);
				g_StartupInfo.SendDlgMessage(hDlg, DM_SETTEXTPTR, iInactiveExt, (DLG_LPARAM)mp_Config->pData->pPlugin->pDefInactive);
				g_StartupInfo.SendDlgMessage(hDlg, DM_SETTEXTPTR, iForbiddenExt, (DLG_LPARAM)mp_Config->pData->pPlugin->pDefForbidden);
			}
			g_StartupInfo.SendDlgMessage(hDlg, DM_SETTEXTPTR, iPLog, (DLG_LPARAM)mp_Config->pData->szStatus);
			g_StartupInfo.SendDlgMessage(hDlg, DM_SETFOCUS, iActiveExt, 0);
		}
		return TRUE;
	}

	return g_StartupInfo.DefDlgProc(hDlg, Msg, Param1, (DLG_LPARAM)Param2);
}

bool CPVDDecoderAboutDlg::Run()
{
	bool lbRc = false;
	int Y = 2;
	int nVerTextLen = 0;
	LPCTSTR pszText = NULL;

	nVerTextLen = max(lstrlen(GetMsg(MISubpluginVersionI1)),lstrlen(GetMsg(MISubpluginVersionI2)));
	if (nVerTextLen < 20) nVerTextLen = 20;

	// Имя
	addFarDialogItem(DI_TEXT,     5, Y, 0, 0, 0, 0, 0, 0, GetMsg(MISubpluginName));
	if (mp_Config->pData->pPlugin) pszText = mp_Config->pData->pPlugin->pName; else pszText = mp_Config->pData->pModuleFileName;
	iPName = addFarDialogItem(DI_EDIT,     5, Y+1, DialogWidth-nVerTextLen+3, 0, 0, 0, DIF_READONLY, 0, pszText, 0);
	// Версия (интерфейса и самого плагина)
	if (mp_Config->pData->pPlugin) pszText = GetMsg((mp_Config->pData->pPlugin->nVersion==1) ? MISubpluginVersionI1 : MISubpluginVersionI2); else pszText = GetMsg(MISubpluginVersion);
	addFarDialogItem(DI_TEXT,     DialogWidth-nVerTextLen+5, Y, 0, 0, 0, 0, 0, 0, pszText);
	if (mp_Config->pData->pPlugin) pszText = mp_Config->pData->pPlugin->pVersion; else pszText = L"???";
	iPVer = addFarDialogItem(DI_EDIT,     DialogWidth-nVerTextLen+5, Y+1, DialogWidth+4, 0, 0, 0, DIF_READONLY, 0, pszText, 0);
	Y+=2;
	// Комментарий (информация из субплагина)
	addFarDialogItem(DI_TEXT,     5, Y, 0, 0, 0, 0, 0, 0, GetMsg(MISubpluginComments));
	if (mp_Config->pData->pPlugin) pszText = mp_Config->pData->pPlugin->pComments; else pszText = L"";
	iPComm = addFarDialogItem(DI_EDIT,     5, Y+1, DialogWidth-nVerTextLen+3, 0, 0, 0, DIF_READONLY, 0, pszText, 0);
	// Флаги субплагина
	memset(&ModuleFlags, 0, sizeof(ModuleFlags)); memset(ModuleFlagItems, 0, sizeof(ModuleFlagItems));
	ModuleFlags.Items = ModuleFlagItems;
	DWORD nFlags = mp_Config->pData->pPlugin ? mp_Config->pData->pPlugin->nFlags : mp_Config->pData->nCurrentFlags;
#define ADDFLAG(n,s) if (nFlags & n) { ModuleFlagItems[ModuleFlags.ItemsNumber++].Text = s; nFlags &= ~n; }
	ADDFLAG(PVD_IP_DECODE,L"Decoder");
	ADDFLAG(PVD_IP_TRANSFORM,L"Transform");
	ADDFLAG(PVD_IP_DISPLAY,L"Display");
	ADDFLAG(PVD_IP_PRIVATE,L"Private display");
	ADDFLAG(PVD_IP_DIRECT,L"Direct to video");
	ADDFLAG(PVD_IP_NOTERMINAL,L"No terminal");
	ADDFLAG(PVD_IP_PROCESSING,L"PostProcessing");
	ADDFLAG(PVD_IP_MULTITHREAD,L"Multithread");
	ADDFLAG(PVD_IP_ALLOWCACHE,L"Allow caching");
	ADDFLAG(PVD_IP_CANUPSCALE,L"Can upscale");
	ADDFLAG(PVD_IP_CANDESCALE,L"Can descale");
	ADDFLAG(PVD_IP_CANREFINERECT,L"Can render rect");
	ADDFLAG(PVD_IP_CANREDUCE,L"Can reduce");
	if (nFlags) {
		wsprintf(szFlagsLeft, L"0x%08X", nFlags);
		ModuleFlagItems[ModuleFlags.ItemsNumber++].Text = szFlagsLeft;
	}
	if (ModuleFlags.ItemsNumber) ModuleFlagItems[0].Flags = LIF_SELECTED;
	addFarDialogItem(DI_TEXT,     DialogWidth-nVerTextLen+5, Y, 0, 0, 0, 0, 0, 0, GetMsg(MISubpluginFlags));
	iPFlags = addFarDialogItem(DI_COMBOBOX, DialogWidth-nVerTextLen+5, Y+1, DialogWidth+4, Y, 0, 0, DIF_DROPDOWNLIST, 0, 0);
#ifdef FAR_UNICODE
	DialogItems[iPFlags].ListItems = &ModuleFlags;
#else
	DialogItems[iPFlags].Param.ListItems = &ModuleFlags;
#endif
	Y+=2;
	// Полный путь к субплагину
	addFarDialogItem(DI_TEXT,     5, Y++, 0, 0, 0, 0, 0, 0, GetMsg(MISubpluginFullPathName));
	pszText = mp_Config->pData->pModulePath;
	addFarDialogItem(DI_EDIT,     5, Y++, DialogWidth+4, 0, 0, 0, DIF_READONLY, 0, pszText, 0);



	if (mp_Config->pData->nCurrentFlags & PVD_IP_DECODE && mp_Config->pData->pPlugin)
	{
		addFarDialogItem(
			DI_TEXT, 5, Y++, 0, 0, 0, 0, DIF_BOXCOLOR | DIF_SEPARATOR, 0, 0/*GetMsg(MIIgnoredExtensions)*/);

		addFarDialogItem(
			DI_TEXT,     5, Y++, 0, 0, 0, 0, 0, 0, GetMsg(MISubpluginActiveExtensions));
		iActiveExt = addFarDialogItem(DI_EDIT, 5, Y++, DialogWidth+4, 0, 0, 0, DIF_READONLY, 0,
			mp_Config->pData->pPlugin->pDefActive ? mp_Config->pData->pPlugin->pDefActive : GetMsg(MIModuleInfoNotLoaded));
		pszText = GetMsg(MISubpluginResetExt);

		addFarDialogItem(
			DI_TEXT,     5, Y, 0, 0, 0, 0, 0, 0, GetMsg(MISubpluginInactiveExtensions));
		iInactiveExt = addFarDialogItem(DI_EDIT, 5, Y+1, (DialogWidth/2)-2, 0, 0, 0, DIF_READONLY, 0,
			mp_Config->pData->pPlugin->pDefInactive ? mp_Config->pData->pPlugin->pDefInactive : GetMsg(MIModuleInfoNotLoaded));

		addFarDialogItem(
			DI_TEXT,     DialogWidth/2, Y, 0, 0, 0, 0, 0, 0, GetMsg(MISubpluginForbiddenExtensions));
		iForbiddenExt = addFarDialogItem(DI_EDIT, DialogWidth/2, Y+1, DialogWidth+4, 0, 0, 0, DIF_READONLY, 0,
			mp_Config->pData->pPlugin->pDefForbidden ? mp_Config->pData->pPlugin->pDefForbidden : GetMsg(MIModuleInfoNotLoaded));
		Y += 2;

		pszText = GetMsg(MISubpluginReloadExt);
		iReloadExt = addFarDialogItem(
			DI_BUTTON,   DialogWidth-lstrlen(pszText)+1, Y-4, 0, 0, 0, 0, mp_Config->pData->pPlugin ? 0 : DIF_DISABLE, 0, pszText);

		addFarDialogItem(
			DI_TEXT,     5, Y++, 0, 0, 0, 0, 0, 0, GetMsg(MISubpluginDefaultPriority));
		wsprintf(szPriority, L"0x%04X", mp_Config->pData->pPlugin->nDefPriority);
		iPriority = addFarDialogItem(
			DI_EDIT,     5, Y++, DialogWidth+4, 0, 0, 0, DIF_READONLY, 0, szPriority);

		/*
		iResetExt = addFarDialogItem(
			DI_BUTTON,   DialogWidth-lstrlen(pszText)+1, Y-4, 0, 0, 0, 0, mp_Config->pData->pPlugin ? 0 : DIF_DISABLE, 0, pszText);
		*/
	}


	// И последнее - кнопки OK / Cancel
	addFarDialogItem(
		DI_TEXT, 5, Y++, 0, 0, 0, 0, DIF_BOXCOLOR | DIF_SEPARATOR, 0, 0/*GetMsg(MIIgnoredExtensions)*/);

	iOk = addFarDialogItem(
		DI_BUTTON, 0, Y, 0, 0, 0, 0, DIF_CENTERGROUP, TRUE, GetMsg(MIOK));
	iCancel = addFarDialogItem(
		DI_BUTTON, 0, Y, 0, 0, 0, 0, DIF_CENTERGROUP, 0, GetMsg(MICancel));


	int ExitCode = -1;
	if (DialogInit()) {
		ExitCode = g_StartupInfo.DialogRun(mh_Dlg);

		DialogFree();

		lbRc = true;
	}

	MCHKHEAP;
	return lbRc;
}


