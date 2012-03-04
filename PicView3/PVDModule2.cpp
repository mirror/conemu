
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
#include "PVDModule2.h"
#include "PictureView_Lang.h"
//#include <shlwapi.h>


/* ********************* */
/*  Interface Version 2  */
/* ********************* */
CPVDModuleVer2::CPVDModuleVer2(CModuleInfo* apData)
	: CPVDModuleBase(apData)
{
	fInit2 = NULL;
	fExit2 = NULL;
	fPluginInfo2 = NULL;
	fFileOpen2 = NULL;
	fPageInfo2 = NULL;
	fPageDecode2 = NULL;
	fPageFree2 = NULL;
	fFileClose2 = NULL;
	//
	fTranslateError2 = NULL;
	fGetFormats2 = NULL;
	fReloadConfig2 = NULL;
	//
	fCancel2 = NULL;
	//
	fDisplayInit2 = NULL;
	fDisplayCreate2 = NULL;
	fDisplayPaint2 = NULL;
	fDisplayClose2 = NULL;
	fDisplayExit2 = NULL;
	//
	//mp_Context = NULL;
	nVersion = 2;
	nFlags = nCurrentFlags = 0;
}

CPVDModuleVer2::~CPVDModuleVer2()
{	MCHKHEAP;

	if (bDisplayInitialized) {
		DisplayExit2();
		_ASSERTE(!bDisplayInitialized);
	}

	if (bInitialized) {
		Exit2();
		_ASSERTE(!bInitialized);
	}

	MCHKHEAP;
}

bool CPVDModuleVer2::LoadFormatsFromPvd()
{
	if (!bInitialized) if (!InitPlugin()) return false;
	
	FUNCLOGGER(L"CPVDModuleVer2::LoadFormatsFromPvd()");

	MCHKHEAP;

	if (fGetFormats2) {
		BOOL lbRc = FALSE;
		pvdFormats2 pf = {sizeof(pvdFormats2)};

		TRY {
			fGetFormats2(mp_Context, &pf);
			OutputDebugString(L"\n\npvdGetFormats2 succeeded\npActive is: ");
			OutputDebugString(pf.pActive ? pf.pActive : L"<NULL>");
			OutputDebugString(L"\npInactive is: ");
			OutputDebugString(pf.pInactive ? pf.pInactive : L"<NULL>");
			OutputDebugString(L"\npForbidden is: ");
			OutputDebugString(pf.pForbidden ? pf.pForbidden : L"<NULL>");
			OutputDebugString(L"\n\n");
			//#ifdef _DEBUG
			//if (pf.pSup ported && !lstrcmp(pf.pSup ported, L"*") && !lstrcmpi(pName, L"GDIPlus")) {
			//	OutputDebugString(L"GDIPlus sup ported forced to: BMP,DIB,EMF,WMF,JPG,JPE,JPEG,PNG,GIF,TIF,TIFF,EXIF,ICO\n");
			//	pf.pSup ported = L"BMP,DIB,EMF,WMF,JPG,JPE,JPEG,PNG,GIF,TIF,TIFF,EXIF,ICO";
			//}
			//#endif
			lbRc = TRUE;
		} CATCH {
			SetException(L"pvdGetFormats2");
		}

		TRY {
			if (pDefActive) free(pDefActive);
			pDefActive = _wcsdup(pf.pActive ? pf.pActive : L"<NULL>");
			if (pDefInactive) free(pDefInactive);
			pDefInactive = _wcsdup(pf.pInactive ? pf.pInactive : L"");
			if (pDefForbidden) free(pDefForbidden);
			pDefForbidden = _wcsdup(pf.pForbidden ? pf.pForbidden : L"");
		} CATCH {
			SetException(L"_wcsdup(extension)");
			pDefActive = pDefInactive = pDefForbidden = NULL;
		}
	}

	MCHKHEAP;

	wchar_t** ppExt[] = {&pDefActive,&pDefInactive,&pDefForbidden};

	for (UINT i=0; i<sizeofarray(ppExt); i++) {
		if (!*(ppExt[i])) *(ppExt[i]) = _wcsdup(L"");

		if (**(ppExt[i])) {
			CharUpperBuffW(*(ppExt[i]), lstrlen(*(ppExt[i])));
			apiSortExtensions(*(ppExt[i]));
		}
	}

	
	//if (!pDefInactive) pDefInactive = _wcsdup(L"");
	//if (!pDefForbidden) pDefForbidden = _wcsdup(L"");
	//
	//if (*pDefSup ported) {
	//	CharUpperBuffW(pDefSup ported, lstrlen(pDefSup ported));
	//	apiSortExtensions(wchar_t* pszExtensions)
	//}
	//CharUpperBuffW(pDefIgnored, lstrlen(pDefIgnored));

	MCHKHEAP;

	return true;
}

// Загрузить, инициализировать, получить список форматов
bool CPVDModuleVer2::InitPlugin(bool bAllowResort/*=false*/)
{
	if (bInitialized) return true;

	bool result = false;
	
	FUNCLOGGER(L"CPVDModuleVer2::InitPlugin()");

	if (!hPlugin) {
		TRY {
			hPlugin = LoadLibrary(pModulePath);
			if (!hPlugin)
				SetStatus(GetMsg(MICantLoadLibrary),TRUE);
		} CATCH {
			SetException(L"LoadLibrary");
			hPlugin = NULL;
		}
		if (!hPlugin) {
			return false;
		}
	}

	fInit2 = (pvdInit2_t)GetProcAddress(hPlugin, "pvdInit2");
	fExit2 = (pvdExit2_t)GetProcAddress(hPlugin, "pvdExit2");
	fPluginInfo2 = (pvdPluginInfo2_t)GetProcAddress(hPlugin, "pvdPluginInfo2");
	fFileOpen2 = (pvdFileOpen2_t)GetProcAddress(hPlugin, "pvdFileOpen2");
	fPageInfo2 = (pvdPageInfo2_t)GetProcAddress(hPlugin, "pvdPageInfo2");
	fPageDecode2 = (pvdPageDecode2_t)GetProcAddress(hPlugin, "pvdPageDecode2");
	fPageFree2 = (pvdPageFree2_t)GetProcAddress(hPlugin, "pvdPageFree2");
	fFileClose2 = (pvdFileClose2_t)GetProcAddress(hPlugin, "pvdFileClose2");
	fTranslateError2 = (pvdTranslateError2_t)GetProcAddress(hPlugin, "pvdTranslateError2");
	fGetFormats2 = (pvdGetFormats2_t)GetProcAddress(hPlugin, "pvdGetFormats2");
	fReloadConfig2 = (pvdReloadConfig2_t)GetProcAddress(hPlugin, "pvdReloadConfig2");
	
	fCancel2 = (pvdCancel2_t)GetProcAddress(hPlugin, "pvdReloadConfig2");
	//
	fDisplayInit2 = (pvdDisplayInit2_t)GetProcAddress(hPlugin, "pvdDisplayInit2");
	fDisplayAttach2 = (pvdDisplayAttach2_t)GetProcAddress(hPlugin, "pvdDisplayAttach2");
	fDisplayCreate2 = (pvdDisplayCreate2_t)GetProcAddress(hPlugin, "pvdDisplayCreate2");
	fDisplayPaint2 = (pvdDisplayPaint2_t)GetProcAddress(hPlugin, "pvdDisplayPaint2");
	fDisplayClose2 = (pvdDisplayClose2_t)GetProcAddress(hPlugin, "pvdDisplayClose2");
	fDisplayExit2 = (pvdDisplayExit2_t)GetProcAddress(hPlugin, "pvdDisplayExit2");
	
	bool bNeedExit = false;
	
	if (!fInit2 || !fExit2 || !fPluginInfo2) {
		wchar_t szMsg[128];
		wsprintf(szMsg, L"Required plugin function (%s) not found",
			!fInit2 ? L"pvdInit2" :
			!fExit2 ? L"pvdExit2" :
			!fPluginInfo2 ? L"pvdPluginInfo2" :	L"<Unlisted>" );
		SetStatus(szMsg,TRUE);
	} else {

		if (!Init2()) {
			//SetStatus(L"pvdInit2 failed");
			bNeedExit = false;
			Exit2();
		}
		else
		{
			bNeedExit = true;
			
			if (PluginInfo2()) {
				result = true;
				bool bOneAtLeast = false, bSomeFlagsDropped = false;
				
				if (result && (nFlags & PVD_IP_DECODE)) {
					// Интерфейс декодера
					if (!fFileOpen2 || !fPageInfo2 || !fPageDecode2 || !fPageFree2 || !fFileClose2 || !fGetFormats2)
					{
						nCurrentFlags &= ~PVD_IP_DECODE; bSomeFlagsDropped = true;
						result = bOneAtLeast;
						wchar_t szMsg[128];
						wsprintf(szMsg, L"Required plugin function (%s) not found",
							!fFileOpen2 ? L"pvdFileOpen2" :
							!fPageInfo2 ? L"pvdPageInfo2" :
							!fPageDecode2 ? L"pvdPageDecode2" :
							!fPageFree2 ? L"pvdPageFree2" :
							!fFileClose2 ? L"pvdFileClose2" :
							!fTranslateError2 ? L"pvdTranslateError2" :
							!fGetFormats2 ? L"pvdGetFormats2" : L"<Unlisted>" );
						SetStatus(szMsg,TRUE);
					} else {
						bOneAtLeast = true;
					}
				}
				if (result && (nFlags & PVD_IP_TRANSFORM)) {
					//TODO: PVD_IP_TRANSFORM
				}
				if (result && (nFlags & PVD_IP_DISPLAY)) {
					// Интерфейс дисплея
					if (!fDisplayInit2 || !fDisplayCreate2 || !fDisplayPaint2 || !fDisplayClose2 || !fDisplayExit2 || !fDisplayAttach2)
					{
						nCurrentFlags &= ~PVD_IP_DISPLAY; bSomeFlagsDropped = true;
						result = bOneAtLeast;
						wchar_t szMsg[128];
						wsprintf(szMsg, L"Required plugin function (%s) not found",
							!fDisplayInit2 ? L"pvdDisplayInit2" :
							!fDisplayCreate2 ? L"pvdDisplayCreate2" :
							!fDisplayPaint2 ? L"pvdDisplayPaint2" :
							!fDisplayClose2 ? L"pvdDisplayClose2" :
							!fDisplayExit2 ? L"pvdDisplayExit2"  :	
							!fDisplayAttach2 ? L"pvdDisplayAttach2" : L"<Unlisted>" );
						SetStatus(szMsg,TRUE);
					} else {
						bOneAtLeast = true;
					}
				}
				if (result && (nFlags & PVD_IP_PROCESSING)) {
					//TODO: PVD_IP_PROCESSING
				}
			
				if (result) {
					pData->nCurrentFlags = nCurrentFlags;
					bInitialized = true;
					bNeedExit = false;
				}

				if (bSomeFlagsDropped && bAllowResort) {
					HKEY hkey = NULL; DWORD dwDisp = 0;
					if (!RegCreateKeyEx(HKEY_CURRENT_USER, pRegPath, 0, 0, 0, KEY_ALL_ACCESS, 0, &hkey, &dwDisp)) {
						RegSetValueEx(hkey, L"PluginFlags", 0, REG_DWORD, (LPBYTE)&nFlags, sizeof(nFlags));
						RegSetValueEx(hkey, L"PluginCurrentFlags", 0, REG_DWORD, (LPBYTE)&nCurrentFlags, sizeof(nCurrentFlags));
						RegCloseKey(hkey); hkey = NULL;
					}

					CPVDManager::SortPlugins2();
				}
			}
		}
	
	
	}

	if (!result) {
		if (bNeedExit) {
			bNeedExit = false;
			Exit2();
		}
		
		TRY {
			FreeLibrary(hPlugin);
		} CATCH {
			SetException(L"FreeLibrary");
		}
		hPlugin = NULL;
	}

	return result;
}

bool CPVDModuleVer2::PluginInfo2()
{
	_ASSERTE(hPlugin && fPluginInfo2);
	pvdInfoPlugin2 pip = {sizeof(pvdInfoPlugin2)};
	bool lbRc = true;
	
	FUNCLOGGER(L"CPVDModuleVer2::PluginInfo2()");

	pip.hModule = hPlugin;

	TRY {
		fPluginInfo2(&pip);
	} CATCH {
		lbRc = false;
		SetException(L"pvdPluginInfo2");
	}

	if (lbRc) {
		// Заглушка для in_Apl. не возвращает нужные флаги
		if (pip.pName) {
			if (lstrcmpi(pip.pName, L"IN_APL")==0)
				pip.Flags |= PVD_IP_DECODE;
		}
	
		_ASSERTE(pip.Flags);
		nFlags = nCurrentFlags = pip.Flags;
		SafeFree(pName);
		SafeFree(pVersion);
		SafeFree(pComments);
		
		TRY {
			pName = _wcsdup(pip.pName ? pip.pName : L"");
			pVersion = _wcsdup(pip.pVersion ? pip.pVersion : L"");
			pComments = _wcsdup(pip.pComments ? pip.pComments : L"");
		} CATCH {
			lbRc = false;
			SetException(L"_wcsdup(name,version,comments)");
			pName = _wcsdup(L"Invalid");
			pVersion = _wcsdup(L"Invalid");
			pComments = _wcsdup(L"Invalid");
		}
		if (!nDefPriority) {
			HKEY hkey = 0; DWORD dwDisp;
			if (!RegCreateKeyEx(HKEY_CURRENT_USER, pRegPath, 0, 0, 0, KEY_ALL_ACCESS, 0, &hkey, &dwDisp)) {
				RegSetValueEx(hkey, L"DefaultPriority", 0, REG_DWORD, (LPBYTE)&pip.Priority, sizeof(pip.Priority));
				if (!pData->nPriority) {
					DWORD nSize = 4;
					if (RegQueryValueEx(hkey, L"Priority", 0, 0, (LPBYTE)&pData->nPriority, &nSize)
						|| !pData->nPriority)
					{
						// Если приоритет юзером ни разу не задавался - тут еще 0. Запишем "умолчательный"
						pData->nPriority = pip.Priority;
						RegSetValueEx(hkey, L"Priority", 0, REG_DWORD, (LPBYTE)&pip.Priority, sizeof(pip.Priority));
					}
				}
				RegCloseKey(hkey); hkey = NULL;
			}
		}
		nDefPriority = max(1,pip.Priority);
		
		#ifdef _DEBUG
		wchar_t szDbg[128]; wsprintf(szDbg, L"PluginInfo> Priority:0x%04X, Flags:0x%04X", pip.Priority, nFlags);
		CPVDManager::AddLogString(pName, szDbg);
		#endif
	}
	
	return lbRc;
}

BOOL CPVDModuleVer2::TranslateError2(DWORD nErrNumber, wchar_t *pszErrInfo, int nBufLen)
{
	BOOL result = FALSE;
	if (nBufLen < 20) {
		_ASSERTE(nBufLen >= 20);
		return FALSE;
	}

	// Некоторые нерадивые субплагины не возвращают nErrNumber но вываливаются по ошибке
	if (hPlugin && fTranslateError2 && nErrNumber) {
		TRY {
			pszErrInfo[0] = 0;
			result = fTranslateError2(nErrNumber, pszErrInfo, nBufLen);
			if (!pszErrInfo[0])
				CPVDModuleBase::TranslateError2(nErrNumber, pszErrInfo, nBufLen);
			else
				SetStatus(pszErrInfo);
		} CATCH {
			SetException(L"pvdTranslateError2");
			result = FALSE;
		}
		return result;
	}

	return CPVDModuleBase::TranslateError2(nErrNumber, pszErrInfo, nBufLen);
}

// 0-информация, 1-предупреждение, 2-ошибка
void CPVDModuleVer2::apiMessageLog(void *apCallbackContext, const wchar_t* asMessage, UINT32 anSeverity)
{
	OutputDebugString(asMessage);
	CPVDModuleBase* pSubPlugin = (CPVDModuleBase*)apCallbackContext;
	if (pSubPlugin && asMessage) {
		TRY {
			// !!! Тут именно Plugins, т.к. pvdInit2 вызывается один раз на модуль, независимо от его типа
			for (UINT i=0; i<CPVDManager::Plugins.size(); i++) {
				if (pSubPlugin == CPVDManager::Plugins[i]->pPlugin) {
					pSubPlugin->SetStatus(asMessage);
					break;
				}
			}
		} CATCH {
			OutputDebugString(L"\n!!! Exception in apiMessageLog !!!\n");
			CPVDManager::AddLogString(L"Unknown", L"!!! Exception in apiMessageLog !!!");
		}
	}
}

// asExtList может содержать '*' (тогда всегда TRUE) или '.' (TRUE если asExt пусто)
BOOL CPVDModuleVer2::apiExtensionMatch(wchar_t* asExtList, const wchar_t* asExt)
{
	if (!asExtList || !asExt) return FALSE;
	if (!*asExtList) return FALSE;

	if (!asExt || !*asExt) asExt = L".";
	if (/*wcschr(asExtList,L'*') ||*/ ExtensionMatch(asExtList, asExt))
		return TRUE;
	return FALSE;
}

BOOL CPVDModuleVer2::apiCallSehed(pvdCallSehedProc2 CalledProc, LONG_PTR Param1, LONG_PTR Param2, LONG_PTR* Result)
{
	BOOL lbRc = FALSE;
	LONG_PTR rc = 0;
	TRY {
		rc = CalledProc(Param1, Param2);
		if (Result) *Result = rc;
		lbRc = TRUE;
	} CATCH {
		lbRc = FALSE;
	}
	return lbRc;
}

struct WC8
{
	wchar_t s[8];
	int nLen;
};

int CPVDModuleVer2::apiSortExtensions(wchar_t* pszExtensions)
{
	if (!pszExtensions)
		return -1;
		
	wchar_t szExt[10];
	DWORD nAllLen = 0, nLen = 0, nCount = 0;
	std::vector<WC8> lExts;
	std::vector<WC8>::iterator i, j, liMin;
	DWORD nMaxLen = lstrlen(pszExtensions);
	wchar_t wch, *pszExt = pszExtensions, *pszCom, *pszEnd = pszExtensions+nMaxLen;

	while (pszExt < pszEnd)
	{
		pszCom = pszExt;
		while ((wch = *pszCom) && wch != L',' && wch != L';') pszCom++;
		*pszCom = 0;
		if (*pszExt == L'.' && pszExt[1]) pszExt++;
		nLen = pszCom - pszExt;
	
		if (nLen >= 1 && nLen <= 7)
		{
			CharUpperBuff(pszExt, nLen);
			WC8 e; lstrcpy(e.s, pszExt); e.nLen = nLen;
			lExts.push_back(e);
		}
		
		pszExt = pszCom + 1;
	}
	
	if (lExts.empty())
	{
		*pszExtensions = 0;
		return 0;
	}
		
	for (i=lExts.begin(); i!=lExts.end() && (i+1)!=lExts.end(); i++)
	{
		liMin = i;
		for (j=i+1; j!=lExts.end(); j++)
		{
			if (lstrcmpW(liMin->s, j->s) > 0)
				liMin = j;
		}
		if (liMin!=i)
		{
			WC8 t;
			t = *liMin;
			*liMin = *i;
			*i = t;
			//lstrcpy(szExt, liMin->s);
			//lstrcpy(liMin->s, i->s);
			//lstrcpy(i->s, szExt);
		}
	}

	i = lExts.begin();
	pszExt = pszExtensions;
	szExt[0] = 0;
	while (i!=lExts.end())
	{
		if (szExt[0] && !lstrcmpW(szExt, i->s))
		{
			i++; continue; // дубли пропускаем
		}
		lstrcpy(szExt, i->s);
		nLen = i->nLen;
		i++;

		if (nMaxLen >= (nAllLen + nLen))
		{
			wmemmove(pszExtensions+nAllLen, szExt, nLen);
			nAllLen += nLen;
			pszExtensions[nAllLen++] = L',';
			nCount ++;
		}
	}

	if (!nAllLen || !nCount)
	{
		*pszExtensions = 0;
		return 0;
	}
		
	pszExtensions[nAllLen-1] = 0;

	return nCount;
}


int CPVDModuleVer2::apiMulDivI32(int a, int b, int c)  // a * (__int64)b / c;
{
	return ((__int64)a) * b / c;
}

UINT CPVDModuleVer2::apiMulDivU32(UINT a, UINT b, UINT c)
{
	return MulDivU32(a, b, c);
}
	
UINT CPVDModuleVer2::apiMulDivU32R(UINT a, UINT b, UINT c)
{
	return MulDivU32R(a, b, c);
}

int CPVDModuleVer2::apiMulDivIU32R(int a, UINT b, UINT c)
{
	return MulDivIU32R(a, b, c);
}


BOOL CPVDModuleVer2::Init2()
{
	FUNCLOGGER(L"CPVDModuleVer2::Init2()");
	
	if (hPlugin && fInit2) {
		// В плагин нужно передавать ПОДветку реестра. например .../pvd.bmp/Settings, чтобы случайно имена ключен не пересеклись
		pvdInitPlugin2 pip = {sizeof(pvdInitPlugin2)};
		pip.nMaxVersion = PVD_UNICODE_INTERFACE_VERSION;
		pip.pRegKey = pRegSettings;
		pip.pCallbackContext = this;
		pip.MessageLog = apiMessageLog;
		pip.ExtensionMatch = apiExtensionMatch;
		pip.hModule = hPlugin;
		pip.CallSehed = apiCallSehed;
		pip.SortExtensions = apiSortExtensions;
		pip.MulDivI32 = apiMulDivI32;
		pip.MulDivU32 = apiMulDivU32;
		pip.MulDivU32R = apiMulDivU32R;
		pip.MulDivIU32R = apiMulDivIU32R;

		UINT nRc = 0;

		TRY {
			nRc = fInit2(&pip);
			if (nRc != PVD_UNICODE_INTERFACE_VERSION) {
				if (!pip.nErrNumber)
					SetStatus(L"Interface version is not supported",TRUE);
				else if (!TranslateError2(pip.nErrNumber, szLastError, sizeofarray(szLastError)))
					SetStatus(L"pvdInit2 returns an error",TRUE);
			}
		} CATCH {
			SetException(L"pvdInit2");
			nRc = 0;
		}

		if (nRc == PVD_UNICODE_INTERFACE_VERSION) {
			mp_Context = pip.pContext;
			return TRUE;
		}
	} else SetStatus(L"CPVDModuleVer2::Init2 skipped, plugins was not loaded",TRUE);
	return FALSE;
}

void CPVDModuleVer2::Exit2()
{
	if (hPlugin && fExit2) {
		TRY {
			fExit2(mp_Context);
			bInitialized = false;
		} CATCH {
			SetException(L"pvdExit2");
		}
	}
}

BOOL CPVDModuleVer2::FileOpen2(const wchar_t *pFileName, INT64 lFileSize, const BYTE *pBuf, UINT32 lBuf, pvdInfoImage2 *pImageInfo)
{
	if (!lFileSize && !lBuf) return FALSE;
	if (!bInitialized) if (!InitPlugin(true)) return FALSE;
	
	FUNCLOGGER(L"CPVDModuleVer2::FileOpen()");

	BOOL result = FALSE;
	_ASSERTE(pImageInfo->cbSize == sizeof(*pImageInfo));
	if (hPlugin && fFileOpen2) {
		TRY {
			result = fFileOpen2(mp_Context, pFileName, lFileSize, pBuf, lBuf, pImageInfo);
			if (!result)
				TranslateError2(pImageInfo->nErrNumber, szLastError, sizeofarray(szLastError));
		} CATCH {
			SetException(L"pvdFileOpen2");
			result = FALSE;
		}
	}
	return result;
}

BOOL CPVDModuleVer2::PageInfo2(void *pImageContext, pvdInfoPage2 *pPageInfo)
{
	BOOL result = FALSE;
	_ASSERTE(pPageInfo->cbSize == sizeof(*pPageInfo));
	
	FUNCLOGGER(L"CPVDModuleVer2::PageInfo2()");
	
	pPageInfo->nPages = 0; // если плагин установит новое значение, значит количество страниц было скорректировано
	if (hPlugin && fPageInfo2) {
		//pPageInfo->iPage = iPage; -- поле должно быть выставлено
		TRY {
			result = fPageInfo2(mp_Context, pImageContext, pPageInfo);
			if (!result)
				TranslateError2(pPageInfo->nErrNumber, szLastError, sizeofarray(szLastError));
		} CATCH {
			SetException(L"pvdPageInfo2");
			result = FALSE;
		}
	}
	return result;
}

BOOL CPVDModuleVer2::PageDecode2(void *pImageContext, pvdInfoDecode2 *pDecodeInfo, pvdDecodeCallback2 DecodeCallback, void *pDecodeCallbackContext)
{
	BOOL result = FALSE;
	_ASSERTE(pDecodeInfo->cbSize == sizeof(*pDecodeInfo));
	
	FUNCLOGGER(L"CPVDModuleVer2::PageDecode2()");
	
	pDecodeInfo->nPages = 0; // если плагин установит новое значение, значит количество страниц было скорректировано
	if (hPlugin && fPageDecode2) {
		TRY {
			result = fPageDecode2(mp_Context, pImageContext, pDecodeInfo, DecodeCallback, pDecodeCallbackContext);
			if (!result)
				TranslateError2(pDecodeInfo->nErrNumber, szLastError, sizeofarray(szLastError));
			else {
				_ASSERTE(pDecodeInfo->lWidth && pDecodeInfo->lHeight);
			}
		} CATCH {
			SetException(L"pvdPageDecode2");
			result = FALSE;
		}
	}
	return result;
}

void CPVDModuleVer2::PageFree2(void *pImageContext, pvdInfoDecode2 *pDecodeInfo)
{
	_ASSERTE(pDecodeInfo->cbSize == sizeof(*pDecodeInfo));
	
	FUNCLOGGER(L"CPVDModuleVer2::PageFree2()");
	
	if (hPlugin && fPageFree2) {
		TRY {
			//TODO("когда починит - вернуть нормальную обработку");
			//if (!wcscmp(pName, L"IN_APL"))
			//	((pvdPageFree2Old_t)fPageFree2)(mp_Context, pImageContext, pDecodeInfo->iPage, pDecodeInfo);
			//else
			fPageFree2(mp_Context, pImageContext, pDecodeInfo);
		} CATCH {
			SetException(L"pvdPageFree2");
		}
	}
}

void CPVDModuleVer2::FileClose2(void *pImageContext)
{
	FUNCLOGGER(L"CPVDModuleVer2::FileClose2()");
	
	if (hPlugin && fFileClose2) {
		TRY {
			fFileClose2(mp_Context, pImageContext);
		} CATCH {
			SetException(L"pvdFileClose2");
		}
	}
}

void CPVDModuleVer2::ReloadConfig2()
{	// Если плагин еще не был загружен - то и дергаться не нужно - настройки он считает сам
	if (hPlugin && fReloadConfig2) {
		TRY {
			fReloadConfig2(mp_Context);
		} CATCH {
			SetException(L"pvdReloadConfig2");
		}
	}
}

void CPVDModuleVer2::Cancel2()
{
	_ASSERTE(this);
	if (!this) return;
	if (hPlugin && fCancel2) {
		TRY {
			fCancel2(mp_Context);
		} CATCH {
			SetException(L"pvdCancel2");
		}
	}
}

BOOL CPVDModuleVer2::DisplayInit2(pvdInfoDisplayInit2* pDisplayInit)
{
	_ASSERTE(this);
	if (!this) return FALSE;
	BOOL result = FALSE;
	_ASSERTE(pDisplayInit->cbSize == sizeof(*pDisplayInit));
	
	FUNCLOGGER(L"CPVDModuleVer2::DisplayInit2()");

	_ASSERTE(!bDisplayInitialized);
	if (bDisplayInitialized)
		return TRUE;

	if (hPlugin && fDisplayInit2) {
		TRY {
			result = fDisplayInit2(mp_Context, pDisplayInit);
			if (!result)
				TranslateError2(pDisplayInit->nErrNumber, szLastError, sizeofarray(szLastError));
			else
				bDisplayInitialized = true;
		} CATCH {
			SetException(L"pvdDisplayInit2");
			result = FALSE;
		}
	}
	return result;
}

BOOL CPVDModuleVer2::DisplayAttach2(HWND hWnd, BOOL bAttach)
{
	_ASSERTE(this);
	if (!this) return FALSE;
	BOOL result = FALSE;
	pvdInfoDisplayAttach2 att = {sizeof(pvdInfoDisplayAttach2)};
	if (hPlugin && fDisplayAttach2) {
		att.hWnd = hWnd;
		att.bAttach = bAttach;

		TRY {
			result = fDisplayAttach2(mp_Context, &att);
			if (!result)
				TranslateError2(att.nErrNumber, szLastError, sizeofarray(szLastError));
		} CATCH {
			SetException(L"pvdDisplayAttach2");
			result = FALSE;
		}
	}
	return result;
}

BOOL CPVDModuleVer2::DisplayCreate2(pvdInfoDisplayCreate2* pDisplayCreate)
{
	_ASSERTE(this);
	if (!this) return FALSE;
	
	FUNCLOGGER(L"CPVDModuleVer2::DisplayCreate2()");
	
	BOOL result = FALSE;
	_ASSERTE(pDisplayCreate->cbSize == sizeof(*pDisplayCreate));
	if (hPlugin && fDisplayCreate2) {
		TRY {
			_ASSERTE(pDisplayCreate->pImage->lWidth>0 && pDisplayCreate->pImage->lHeight>0);
			result = fDisplayCreate2(mp_Context, pDisplayCreate);
			if (!result)
				TranslateError2(pDisplayCreate->nErrNumber, szLastError, sizeofarray(szLastError));
		} CATCH {
			SetException(L"pvdDisplayCreate2");
			result = FALSE;
		}
	}
	return result;
}

BOOL CPVDModuleVer2::DisplayPaint2(void* pDisplayContext, pvdInfoDisplayPaint2* pDisplayPaint)
{
	_ASSERTE(this);
	if (!this) return FALSE;
	BOOL result = FALSE;
	_ASSERTE(pDisplayPaint->cbSize == sizeof(*pDisplayPaint));
	if (hPlugin && fDisplayPaint2) {
		TRY {
			result = fDisplayPaint2(mp_Context, pDisplayContext, pDisplayPaint);
			if (!result)
				TranslateError2(pDisplayPaint->nErrNumber, szLastError, sizeofarray(szLastError));
		} CATCH {
			SetException(L"pvdDisplayPaint2");
			result = FALSE;
		}
	}
	return result;
}

void CPVDModuleVer2::DisplayClose2(void* pDisplayContext)
{
	_ASSERTE(this);
	if (!this) return;
	if (hPlugin && fDisplayClose2) {
		if (!bDisplayInitialized) {
			SetStatus(L"!!! DisplayClose2 was called, but display was not initialized");
		} else if (!pDisplayContext) {
			SetStatus(L"!!! DisplayClose2 was called with NULL context");
		} else {
			TRY {
				fDisplayClose2(mp_Context, pDisplayContext);
			} CATCH {
				SetException(L"pvdDisplayClose2");
			}
		}
	}
}

void CPVDModuleVer2::DisplayExit2()
{
	_ASSERTE(this);
	if (!this) return;
	_ASSERTE(bDisplayInitialized);
	if (hPlugin && fDisplayExit2) {
		TRY {
			fDisplayExit2(mp_Context);
			bDisplayInitialized = FALSE;
		} CATCH {
			SetException(L"pvdDisplayExit2");
		}
	}
}
