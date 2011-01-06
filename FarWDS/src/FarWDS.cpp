
#include <windows.h>
#include <tchar.h>
#ifdef _DEBUG
	#include <crtdbg.h>
#else
	#define _ASSERT(x)
	#define _ASSERTE(x)
#endif
#ifdef _UNICODE
#include "pluginW.hpp"
#else
#include "pluginA.hpp"
#endif
#include "FarWDS_Lang.h"

#undef _tcslen
#ifdef _UNICODE
	#define _tcslen lstrlenW
	#define _tcsscanf swscanf
	#define SETMENUTEXT(itm,txt) itm.Text = txt;
	#define F757NA 0,
	#define _GetCheck(i) (int)psi.SendDlgMessage(hDlg,DM_GETCHECK,i,0)
	#define GetDataPtr(i) ((const TCHAR *)psi.SendDlgMessage(hDlg,DM_GETCONSTTEXTPTR,i,0))
	#define SETTEXT(itm,txt) itm.PtrData = txt
	#define SETTEXTPRINT(itm,fmt,arg) wsprintf(pszBuf, fmt, arg); SETTEXT(itm,pszBuf); pszBuf+=lstrlen(pszBuf)+2;
	#define _tcstoi _wtoi
#else
	#define _tcslen lstrlenA
	#define _tcsscanf sscanf
	#define SETMENUTEXT(itm,txt) lstrcpy(itm.Text, txt);
	#define F757NA
	#define _GetCheck(i) items[i].Selected
	#define GetDataPtr(i) items[i].Data
	#define SETTEXT(itm,txt) lstrcpy(itm.Data, txt)
	#define SETTEXTPRINT(itm,fmt,arg) wsprintf(itm.Data, fmt, arg)
	#define _tcstoi atoi
#endif

#ifdef _DEBUG
#define MCHKHEAP _ASSERT(_CrtCheckMemory());
#else
#define MCHKHEAP
#endif

#include <searchapi.h>
#include <atlbase.h>
#include <atldbcli.h>
#include <strsafe.h>
#include <propsys.h>
#include <propkey.h>
#include <stdlib.h>
#include <atlbase.h>
#include <atlstr.h>
#include <atlcom.h>
#include <comdef.h>
#include <atldbcli.h>
#include <strsafe.h>
#include <WinInet.h>
#include <Ntquery.h>
#include <Filter.h>
#include <Filterr.h>

#pragma comment(lib,"Ntquery")

#include "FarHelper.h"

struct PluginStartupInfo psi;
struct FarStandardFunctions FSFW;

LONG GetString(LPCWSTR asKey, LPCWSTR asName, wchar_t* pszValue, DWORD cCount)
{
	HKEY hk; LONG nRegRc; DWORD nSize;
	
	pszValue[0] = 0;

	nRegRc = RegOpenKeyExW(HKEY_CLASSES_ROOT, asKey, 0, KEY_READ, &hk);
	if (!nRegRc) {
		nSize = (cCount-1)*sizeof(*pszValue);
		nRegRc = RegQueryValueExW(hk, NULL, NULL, NULL, (LPBYTE)pszValue, &nSize);
		if (nRegRc) {
			*pszValue = 0;
		} else {
			pszValue[nSize/sizeof(pszValue[0])] = 0;
		}
		RegCloseKey(hk);
	}
	
	return nRegRc;
}

TCHAR* lstrdup(LPCTSTR asText)
{
	int nLen = asText ? lstrlen(asText) : 0;
	TCHAR* psz = (TCHAR*)malloc((nLen+1)*sizeof(TCHAR));
	if (nLen)
		lstrcpy(psz, asText);
	else
		psz[0] = 0;
	return psz;
}

void lstrcpy_t(TCHAR* pszDst, int cMaxSize, const wchar_t* pszSrc)
{
	if (cMaxSize<1) return;
#ifdef _UNICODE
	lstrcpyn(pszDst, pszSrc, cMaxSize);
#else
	WideCharToMultiByte(CP_OEMCP, 0, pszSrc, -1, pszDst, cMaxSize, 0,0);
#endif
}

#ifndef _UNICODE
void lstrcpy_t(wchar_t* pszDst, int cMaxSize, const char* pszSrc)
{
	MultiByteToWideChar(CP_OEMCP, 0, pszSrc, -1, pszDst, cMaxSize);
}
#endif

TCHAR *GetMsg(int MsgId) {
	TCHAR* psz = (TCHAR*)psi.GetMsg(psi.ModuleNumber, MsgId);
	_ASSERTE(psz);
    return psz;
}

bool Write(LPCTSTR pszOutName, HANDLE hFile, LPVOID ptrData, DWORD nSize)
{
	DWORD nWritten;
	if (!WriteFile(hFile, ptrData, nSize, &nWritten, NULL) || nSize != nWritten) {
		DWORD dwErr = GetLastError();
		TCHAR szError[MAX_PATH*6]; wsprintf(szError, GetMsg(WDSErrCantWriteFile), pszOutName, dwErr);
		psi.Message(psi.ModuleNumber,FMSG_ALLINONE|FMSG_WARNING|FMSG_MB_OK,NULL,
			(const TCHAR* const*)szError,1,0);
		return false;
	}
	return true;
}

#define WriteText(s) Write(pszOutName, hFile, s, lstrlenW(s)*2)

bool ExtractDocText(const TCHAR* pszFileName, TCHAR *pszOutName/*[MAX_PATH*3]*/, BOOL abSilence)
{
	bool lbFileExists = false;
	IFilter *pFilter = NULL;
	IUnknown *pUnk = NULL;
	HRESULT hr = S_OK;
	ULONG dwFlags = 0;
	wchar_t *pwszFileName;
	wchar_t szText[8192];
	wchar_t szRN[3] = L"\r\n";
	HANDLE hFile = NULL;
	//DWORD nWritten, nWrite;
	PROPVARIANT *pv = NULL;
	
	CScreenRestore scr(GetMsg(WDSStatusExtracting), GetMsg(WDSPluginName));

	#ifdef _UNICODE
		pwszFileName = (wchar_t*)pszFileName;
	#else
		wchar_t szWideFileName[MAX_PATH*2];
		MultiByteToWideChar(CP_OEMCP, 0, pszFileName, -1, szWideFileName, MAX_PATH*2);
		pwszFileName = szWideFileName;
	#endif
	
	hr = CoInitialize(NULL);

	MCHKHEAP;
	hr = LoadIFilter(pwszFileName, NULL, (void**)&pUnk);
	MCHKHEAP;

	if (SUCCEEDED(hr)) {
		hr = pUnk->QueryInterface(__uuidof(IFilter), (void**)&pFilter);
		pUnk->Release(); pUnk = NULL;
	}
	
	if (FAILED(hr) || !pFilter) {
		goto wrap;
	}
	
	pszOutName[0] = 0;
	FSFW.MkTemp(pszOutName,
		#ifdef _UNICODE
			MAX_PATH*2,
		#endif
		_T("FWDS"));
	CreateDirectory(pszOutName, NULL);
	lstrcat(pszOutName, _T("\\"));
	const TCHAR* pszNamePart = _tcsrchr(pszFileName, _T('\\'));
	TCHAR *pszEnd = pszOutName+lstrlen(pszOutName);
	lstrcpyn(pszEnd, pszNamePart ? (pszNamePart+1) : pszFileName, (MAX_PATH-5));
	lstrcat(pszOutName, _T(".txt"));
	
	hFile = CreateFile(pszOutName, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		DWORD dwErr = GetLastError();
		TCHAR szError[MAX_PATH*6]; wsprintf(szError, GetMsg(WDSErrCantCreateFile), pszOutName, dwErr);
		psi.Message(psi.ModuleNumber,FMSG_ALLINONE|FMSG_WARNING|FMSG_MB_OK,NULL,
			(const TCHAR* const*)szError,1,0);
		goto wrap;
	} else {
		lbFileExists = true;
		// Write BOM
		WORD nBOM = 0xFEFF;
		if (!Write(pszOutName, hFile, &nBOM, 2))
			goto wrap;
	}


	if (SUCCEEDED(hr)) {
		hr = pFilter->Init(
			IFILTER_INIT_CANON_PARAGRAPHS|IFILTER_INIT_HARD_LINE_BREAKS|
			IFILTER_INIT_CANON_HYPHENS|IFILTER_INIT_CANON_SPACES|
			IFILTER_INIT_APPLY_INDEX_ATTRIBUTES|IFILTER_INIT_APPLY_CRAWL_ATTRIBUTES|
			IFILTER_INIT_APPLY_OTHER_ATTRIBUTES|IFILTER_INIT_INDEXING_ONLY,
			0, NULL, &dwFlags);
		MCHKHEAP;
	}

	if (SUCCEEDED(hr)) {
		STAT_CHUNK chk = {0};

		hr = pFilter->GetChunk(&chk);
		//while (hr == S_OK)
		while (hr != FILTER_E_END_OF_CHUNKS)
		{
			if (hr == FILTER_E_EMBEDDING_UNAVAILABLE || hr == FILTER_E_LINK_UNAVAILABLE) {
				hr = pFilter->GetChunk(&chk);
				continue;
			} else if (hr != S_OK) {
				break;
			}

			ULONG nRead = 8192;
			PROPVARIANT *pv = NULL;
			MCHKHEAP;
			if (chk.flags & CHUNK_TEXT) {
				ULONG nLen = ARRAYSIZE(szText)-1;
				hr = pFilter->GetText(&nLen, szText);
				BOOL lbExists = FALSE, lbWasRn = FALSE;
				while (hr == S_OK || hr == FILTER_S_LAST_TEXT) {
					if (nLen) {
						szText[nLen] = 0; lbExists = TRUE;
						lbWasRn = (szText[nLen-1] == '\r' || szText[nLen-1] == '\n');
						if (!WriteText(szText))
							goto wrap;
					}

					if (hr == FILTER_S_LAST_TEXT) break;
					nLen = ARRAYSIZE(szText)-1;
					hr = pFilter->GetText(&nLen, szText);
				}
				if (lbExists && !lbWasRn) {
					if (!WriteText(L"\r\n"))
						goto wrap;
				}
				
			} else if (chk.flags & CHUNK_VALUE) {
				MCHKHEAP;
				hr = pFilter->GetValue(&pv);
				if (SUCCEEDED(hr) && pv) {
					BOOL lbWasRn = TRUE;
					ULONG nLen = 0;
					switch (pv->vt) {
						case VT_BSTR:
							nLen = lstrlenW(pv->bstrVal);
							if (nLen) {
								lbWasRn = (pv->bstrVal[nLen-1] == L'\r' || pv->bstrVal[nLen-1] == L'\n');
								if (!WriteText(pv->bstrVal))
									goto wrap;
							}
							break;
						case VT_LPWSTR:
							nLen = lstrlenW(pv->pwszVal);
							if (nLen) {
								lbWasRn = (pv->pwszVal[nLen-1] == L'\r' || pv->pwszVal[nLen-1] == L'\n');
								if (!WriteText(pv->pwszVal))
									goto wrap;
							}
							break;
						case VT_LPSTR:
							nLen = lstrlenA(pv->pszVal);
							if (nLen) {
								lbWasRn = (pv->pszVal[nLen-1] == '\r' || pv->pszVal[nLen-1] == '\n');
								if (nLen < ARRAYSIZE(szText)) {
									MultiByteToWideChar(CP_ACP, 0, pv->pszVal, nLen, szText, nLen);
									szText[nLen] = 0;
									if (!WriteText(szText))
										goto wrap;
								} else {
									wchar_t* pszWide = (wchar_t*)malloc((nLen+1)*2);
									MultiByteToWideChar(CP_ACP, 0, pv->pszVal, nLen, pszWide, nLen);
									pszWide[nLen] = 0;
									bool lbWritten = WriteText(pszWide);
									free(pszWide);
									if (!lbWritten)
										goto wrap;
								}
							}
							break;
					}
					if (!lbWasRn) {
						if (!WriteText(L"\r\n"))
							goto wrap;
					}
					// Очистить
					PropVariantClear(pv);
					CoTaskMemFree(pv);
					pv = NULL;
				}
			}

			MCHKHEAP;
			hr = pFilter->GetChunk(&chk);
			MCHKHEAP;
		}
	}

wrap:
	if (pv) {
		PropVariantClear(pv);
		CoTaskMemFree(pv);
	}
	if (hFile && hFile != INVALID_HANDLE_VALUE) CloseHandle(hFile);
	if (pUnk) pUnk->Release();
	if (pFilter) pFilter->Release();
	CoUninitialize();
	
	if (!lbFileExists && !abSilence) {
		TCHAR* pszFull = NULL;
		TCHAR* lpMsgBuf=NULL;
		if (FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, 
			(DWORD)hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL ))
		{
			while ((pszFull = _tcsstr(lpMsgBuf, _T("\r\n"))) != NULL) {
				pszFull[0] = _T(' '); pszFull[1] = _T('\n');
			}

			#ifndef _UNICODE
			CharToOemBuff(lpMsgBuf, lpMsgBuf, lstrlen(lpMsgBuf));
			#endif
		}
		const TCHAR* pszFormat = GetMsg(WDSErrCantFilter);
		int nLen = (lpMsgBuf ? lstrlen(lpMsgBuf) : 0) + lstrlen(pszFormat) + lstrlen(pszFileName) + 128;
		pszFull = (TCHAR*)malloc(nLen*sizeof(TCHAR));
		wsprintf(pszFull, pszFormat, pszFileName, (DWORD)hr, lpMsgBuf ? lpMsgBuf : _T(""));
		if (lpMsgBuf) LocalFree(lpMsgBuf);
		
		psi.Message(psi.ModuleNumber,FMSG_ALLINONE|FMSG_WARNING|FMSG_MB_OK,NULL,
			(const TCHAR* const*)pszFull,1,0);

		free(pszFull);
	}

	return lbFileExists;
}

TCHAR* GetSelectedFile(BOOL abAllowFolder=FALSE)
{
	TCHAR* pszFileName = NULL;
	PanelInfo pi = {0};
	BOOL lbPanelOk =
	#ifdef _UNICODE
		psi.Control(INVALID_HANDLE_VALUE, FCTL_GETPANELINFO, 0, (LONG_PTR)&pi)
	#else
		psi.Control(INVALID_HANDLE_VALUE, FCTL_GETPANELINFO, &pi)
	#endif
		;

	if (lbPanelOk && pi.ItemsNumber > 0)
	{
		
		#ifdef _UNICODE
		PluginPanelItem *ppi = NULL;
		if (size_t len = psi.Control(INVALID_HANDLE_VALUE, FCTL_GETPANELITEM, pi.CurrentItem, 0))
		{
			if (ppi = (PluginPanelItem*)malloc(len))
			{
				if (psi.Control(INVALID_HANDLE_VALUE, FCTL_GETPANELITEM, pi.CurrentItem, (LONG_PTR)ppi)
					&& (abAllowFolder || !(ppi->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)))
				{
					//pszFileName = lstrdup(ppi->FindData.lpwszFileName);
					int nLen = (int)FSFW.ConvertPath(CPM_FULL, ppi->FindData.lpwszFileName, NULL, 0);
					if (nLen > 0) {
						pszFileName = (wchar_t*)calloc((nLen+1),2);
						FSFW.ConvertPath(CPM_FULL, ppi->FindData.lpwszFileName, pszFileName, nLen);
					}
				}
				free(ppi);
			}
		}
		#else
		if (!(pi.PanelItems[pi.CurrentItem].FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
			//pszFileName = lstrdup(pi.PanelItems[pi.CurrentItem].FindData.cFileName);
			pszFileName = (char*)calloc(MAX_PATH*2,1);
			char* pszPart;
			GetFullPathName(pi.PanelItems[pi.CurrentItem].FindData.cFileName, MAX_PATH*2, pszFileName, &pszPart);
		}
		#endif
	}
	
	return pszFileName;
}

bool OpenDocumentSource(LPCTSTR pszFile, BOOL abEditor, BOOL abSilence)
{
	TCHAR szTempFileName[MAX_PATH*3];
	TCHAR *pszBuf = NULL;
	if (!pszFile) {
		pszBuf = GetSelectedFile();
		if (!pszBuf)
			return false;
		pszFile = pszBuf;
	}
	
	// TODO: Если pszFile не полный путь - развернуть функцией фара
	
	bool lbDocOk = ExtractDocText(pszFile, szTempFileName, FALSE);
	if (lbDocOk) {
		if (abEditor) {
			psi.Editor(szTempFileName, NULL, 0,0,-1,-1,
				EF_NONMODAL|EF_IMMEDIATERETURN|EF_DELETEONCLOSE|EF_ENABLE_F6, 0, 1
					#ifdef _UNICODE
				,1200
				#endif
			);
		} else {
			psi.Viewer(szTempFileName, NULL, 0,0,-1,-1,
				VF_NONMODAL|VF_IMMEDIATERETURN|VF_DELETEONCLOSE|VF_ENABLE_F6
				#ifdef _UNICODE
				,1200
				#endif
			);
		}
	}
	
	if (pszBuf) free(pszBuf);
	
	return lbDocOk;
}

bool CheckForEsc(int nMsg = WDSConfirmStop)
{
	bool EC=false;
	INPUT_RECORD rec;
	static HANDLE hConInp=GetStdHandle(STD_INPUT_HANDLE);
	DWORD ReadCount;
	while (1)
	{
		PeekConsoleInput(hConInp,&rec,1,&ReadCount);
		if (ReadCount==0) break;
		ReadConsoleInput(hConInp,&rec,1,&ReadCount);
		if (rec.EventType==KEY_EVENT)
			if (rec.Event.KeyEvent.wVirtualKeyCode==VK_ESCAPE &&
				rec.Event.KeyEvent.bKeyDown) EC=true;
	}

	if (EC && nMsg) {
		// "Windows Search\nStop search?"
		TCHAR *pszMsg = GetMsg(nMsg);
		int nBtn = psi.Message(psi.ModuleNumber,FMSG_ALLINONE|FMSG_WARNING|FMSG_MB_YESNO,NULL,
			(const TCHAR* const*)pszMsg, 1, 0);
		if (nBtn!=0)
			EC = false;
	}
	return(EC);
}


class WDSStatus;
WDSStatus* gpWDSStatus = NULL;
class WDSStatus
{
public:
	TCHAR  szVersion[32];
	TCHAR  szCatalog[MAX_PATH];
	TCHAR  szUrl[INTERNET_MAX_URL_LENGTH], szUrlLabel[128];
	LONG   nIndexed;
	LONG   nQueuedInc, nQueuedNotify, nQueuedHigh;
	DWORD  nStatus, nPauseReason;
	TCHAR szStatus[255], szPauseReason[255];
	TCHAR szVersionLabel[255], szCatalogName[MAX_PATH+128];
	TCHAR szInIndex[128]; //, szQueued[255];
	TCHAR szSpeedEstim[128];
	struct {
		DWORD nTick, nCount;
	} CountArray[20];
	DWORD nCountArray; int nDocSpeed;
public:
	WDSStatus()
	{
		lstrcpy(szVersion, _T("???")); lstrcpy(szCatalog, _T("SystemIndex"));
		szUrl[0] = 0; nIndexed = nQueuedInc = nQueuedNotify = nQueuedHigh = nStatus = nPauseReason = 0;
		szStatus[0] = szPauseReason[0] = szInIndex[0] = szSpeedEstim[0] = szVersionLabel[0] = szCatalogName[0] = 0;
		nCountArray = nDocSpeed = 0;
	};
	~WDSStatus()
	{
	};
	void ReloadStatus()
	{
		// Create an instance of the search manager
		ISearchManager *pSearchManager;
		HRESULT hr = CoCreateInstance(__uuidof(CSearchManager), NULL, CLSCTX_LOCAL_SERVER, IID_PPV_ARGS(&pSearchManager));
		if (SUCCEEDED(hr))
		{
			LPWSTR pszText = NULL;

			hr = pSearchManager->GetIndexerVersionStr(&pszText);
			if (SUCCEEDED(hr) && pszText) {
				lstrcpy_t(szVersion, ARRAYSIZE(szVersion), pszText);
				CoTaskMemFree(pszText);
			}

			// Get the catalog manager from the search manager
			ISearchCatalogManager *pSearchCatalogManager;
			hr = pSearchManager->GetCatalog(L"SystemIndex", &pSearchCatalogManager);
			if (SUCCEEDED(hr))
			{
				CatalogStatus Status;
				CatalogPausedReason PausedReason;
				hr = pSearchCatalogManager->GetCatalogStatus(&Status, &PausedReason);
				if (SUCCEEDED(hr)) {
					nStatus = (DWORD)Status; nPauseReason = (DWORD)PausedReason;
				} else {
					nStatus = -1; nPauseReason = -1;
				}

				LONG lCount = 0, lInc = 0, lNotif = 0, lHigh = 0;
				hr = pSearchCatalogManager->NumberOfItemsToIndex(&lInc, &lNotif, &lHigh);
				if (SUCCEEDED(hr)) {
					nQueuedInc = lInc;
					nQueuedNotify = lNotif;
					nQueuedHigh = lHigh;
				} else {
					nQueuedInc = nQueuedNotify = nQueuedHigh = -1;
				}
				
				hr = pSearchCatalogManager->NumberOfItems(&lCount);
				if (SUCCEEDED(hr)) {
					nIndexed = lCount;
				} else {
					nIndexed = -1;
				}

				hr = pSearchCatalogManager->URLBeingIndexed(&pszText);
				if (SUCCEEDED(hr) && pszText) {
					lstrcpy_t(szUrl, ARRAYSIZE(szUrl), pszText);
					CoTaskMemFree(pszText);
				}

				pSearchCatalogManager->Release();
			}

			pSearchManager->Release();
		}
		
		TCHAR szEstim[64] = {0}, szSpeed[64] = {0};
		if (nQueuedInc || nQueuedNotify || nQueuedHigh) {
			if (nCountArray > 0) {
				memmove(CountArray+1, CountArray, (ARRAYSIZE(CountArray)-1)*sizeof(CountArray[0]));
			}
			CountArray[0].nTick = GetTickCount();
			CountArray[0].nCount = nIndexed;
			if (nCountArray < ARRAYSIZE(CountArray)) nCountArray++;
		
			if (nCountArray > 1) {
				int nDocCount = CountArray[0].nCount - CountArray[nCountArray-1].nCount;
				int nSeconds = (CountArray[0].nTick - CountArray[nCountArray-1].nTick) / 1000;
				if (nDocCount > 0 && nSeconds > 0) {
					nDocSpeed = nDocCount * 60 / nSeconds;
					wsprintf(szSpeed, _T("%i"), nDocSpeed);
					nSeconds = (UINT)(((__int64)nSeconds) * (nQueuedInc + nQueuedNotify + nQueuedHigh) / nDocCount);
					int nHours = nSeconds / 3600;
					nSeconds = nSeconds % 3600;
					int nMinutes = nSeconds / 60;
					nSeconds = nSeconds % 60;
					wsprintf(szEstim, _T("%i:%02i:%02i"), nHours, nMinutes, nSeconds);
				}
			}
		} else {
			nCountArray = 0;
		}
		
		//wsprintf(szVersionLabel, _T("Windows Search:      %-16s    Catalog name: %s"), szVersion, szCatalog);
		wsprintf(szVersionLabel, GetMsg(WDSLblEngineVer), _T("Windows Search"), szVersion);
		wsprintf(szCatalogName, GetMsg(WDSLblIndex), szCatalog);
		//wsprintf(szInIndex,      _T("Documents indexed:   %-8i   Queued:  %-8i   High priority:  %i"), nIndexed, (nQueuedInc+nQueuedNotify), nQueuedHigh);
		wsprintf(szInIndex,      GetMsg(WDSLblDocuments), nIndexed, (nQueuedInc+nQueuedNotify), nQueuedHigh);
		//wsprintf(szSpeedEstim,   _T("Speed (doc/min):     %-8s     Left:  %s"), szSpeed, szEstim);
		wsprintf(szSpeedEstim,   GetMsg(WDSLblSpeed), szSpeed, szEstim);

		//wsprintf(szInIndex,      _T("Documents indexed:   %-8i%s%s"), nIndexed, szSpeed, szEstim);
		//wsprintf(szQueued,       _T("Incremental queue:   %-8i   Notifications:    %-4i High priority: %i"), nQueuedInc, nQueuedNotify, nQueuedHigh);
		
		TCHAR* pszSlash = _tcsrchr(szUrl, _T('/'));
		lstrcpy(szUrlLabel,  GetMsg(WDSLblLastUrl)/*    _T("Last indexed URL:    ")*/);
		if (pszSlash) {
			int nLen = lstrlen(szUrlLabel); lstrcpyn(szUrlLabel+nLen, pszSlash+1, ARRAYSIZE(szUrlLabel)-nLen-2);
			pszSlash[1] = 0;
		}
		
		lstrcpy(szStatus, GetMsg(WDSLblStatus) /*_T("Catalog status:      ")*/);
		szPauseReason[0] = 0;
		switch (nStatus) {
			case CATALOG_STATUS_IDLE:
			lstrcat(szStatus, GetMsg(WDS_CATALOG_STATUS_IDLE));
			lstrcpy(szPauseReason, _T(""));
			break;

			case CATALOG_STATUS_PAUSED:
			lstrcat(szStatus, GetMsg(WDS_CATALOG_STATUS_PAUSED));
			szPauseReason[0] = 0;
			switch (nPauseReason) {
				case CATALOG_PAUSED_REASON_NONE:
				lstrcpy(szPauseReason, GetMsg(WDS_CATALOG_PAUSED_REASON_NONE));
				break;

				case CATALOG_PAUSED_REASON_HIGH_IO:
				lstrcpy(szPauseReason, GetMsg(WDS_CATALOG_PAUSED_REASON_HIGH_IO));
				break;

				case CATALOG_PAUSED_REASON_HIGH_CPU:
				lstrcpy(szPauseReason, GetMsg(WDS_CATALOG_PAUSED_REASON_HIGH_CPU));
				break;

				case CATALOG_PAUSED_REASON_HIGH_NTF_RATE:
				lstrcpy(szPauseReason, GetMsg(WDS_CATALOG_PAUSED_REASON_HIGH_NTF_RATE));
				break;

				case CATALOG_PAUSED_REASON_LOW_BATTERY:
				lstrcpy(szPauseReason, GetMsg(WDS_CATALOG_PAUSED_REASON_LOW_BATTERY));
				break;

				case CATALOG_PAUSED_REASON_LOW_MEMORY:
				lstrcpy(szPauseReason, GetMsg(WDS_CATALOG_PAUSED_REASON_LOW_MEMORY));
				break;

				case CATALOG_PAUSED_REASON_LOW_DISK:
				lstrcpy(szPauseReason, GetMsg(WDS_CATALOG_PAUSED_REASON_LOW_DISK));
				break;

				case CATALOG_PAUSED_REASON_DELAYED_RECOVERY:
				lstrcpy(szPauseReason, GetMsg(WDS_CATALOG_PAUSED_REASON_DELAYED_RECOVERY));
				break;

				case CATALOG_PAUSED_REASON_USER_ACTIVE:
				lstrcpy(szPauseReason, GetMsg(WDS_CATALOG_PAUSED_REASON_USER_ACTIVE));
				break;

				case CATALOG_PAUSED_REASON_EXTERNAL:
				lstrcpy(szPauseReason, GetMsg(WDS_CATALOG_PAUSED_REASON_EXTERNAL));
				break;

				case CATALOG_PAUSED_REASON_UPGRADING:
				lstrcpy(szPauseReason, GetMsg(WDS_CATALOG_PAUSED_REASON_UPGRADING));
				break;
			};
			break;

			case CATALOG_STATUS_RECOVERING:
			lstrcat(szStatus, GetMsg(WDS_CATALOG_STATUS_RECOVERING));
			lstrcpy(szPauseReason, _T(""));
			break;

			case CATALOG_STATUS_FULL_CRAWL:
			lstrcat(szStatus, GetMsg(WDS_CATALOG_STATUS_FULL_CRAWL));
			lstrcpy(szPauseReason, GetMsg(WDS_CATALOG_STATUS_FULL_CRAWL2));
			break;

			case CATALOG_STATUS_INCREMENTAL_CRAWL:
			lstrcat(szStatus, GetMsg(WDS_CATALOG_STATUS_INCREMENTAL_CRAWL));
			lstrcpy(szPauseReason, GetMsg(WDS_CATALOG_STATUS_FULL_CRAWL2));
			break;

			case CATALOG_STATUS_PROCESSING_NOTIFICATIONS:
			lstrcat(szStatus, GetMsg(WDS_CATALOG_STATUS_PROCESSING_NOTIFICATIONS));
			lstrcpy(szPauseReason, GetMsg(WDS_CATALOG_STATUS_PROCESSING_NOTIFICATIONS2));
			break;

			case CATALOG_STATUS_SHUTTING_DOWN:
			lstrcat(szStatus, GetMsg(WDS_CATALOG_STATUS_SHUTTING_DOWN));
			lstrcpy(szPauseReason, GetMsg(WDS_CATALOG_STATUS_SHUTTING_DOWN2));
			break;
		}
	};
	void ShowStatusDlg()
	{
		HANDLE hScreen = psi.SaveScreen(0,0,-1,-1);
		
		ReloadStatus();
		
		const TCHAR *MsgItems[] = {
			GetMsg(WDSPluginName),
			szVersionLabel,
			szCatalogName,
			_T(""),
			szInIndex,
			szSpeedEstim,
			_T(""),
			szUrlLabel,
			szUrl,
			_T(""),
			szStatus,
			szPauseReason,
			};
		DWORD nLastTick = GetTickCount();
		psi.Message(psi.ModuleNumber,FMSG_LEFTALIGN,NULL,MsgItems,ARRAYSIZE(MsgItems),0);
			
		while (true)
		{
			Sleep(10);
			if (CheckForEsc(0))
				break;

			DWORD nCurTick = GetTickCount();
			if ((nCurTick - nLastTick) > 5000)
			{
				ReloadStatus();
				if (hScreen) psi.RestoreScreen(hScreen);
				hScreen = psi.SaveScreen(0,0,-1,-1);
				psi.Message(psi.ModuleNumber,FMSG_LEFTALIGN,NULL,MsgItems,ARRAYSIZE(MsgItems),0);
				nLastTick = GetTickCount();
			}
		}
		
		if (hScreen) psi.RestoreScreen(hScreen);
	};
};


struct WDSPluginItem
{
	DWORD         dwFileAttributes;
	FILETIME      ftCreationTime, ftLastAccessTime, ftLastWriteTime;
	TCHAR*        szName;
	LARGE_INTEGER nFileSize;
	const TCHAR*  pszNamePart;
};

TCHAR  gszMask[MAX_PATH];
TCHAR *gpszSearch = NULL, *gpszAnyField = NULL;
//BOOL   gbStrictContents = TRUE;

// class which is returned for each result in the query 
class CItem
{
private:
	WCHAR      _szUrl[INTERNET_MAX_URL_LENGTH];
	ULONGLONG  _Size;
	VARIANT    _Created, _Modified, _Accessed;
	DWORD      _Attrs;
public:
	PCWSTR GetUrl() { return _szUrl; };
	ULONGLONG GetSize() { return _Size; };
	FILETIME GetCreated() { SYSTEMTIME st={0}; VariantTimeToSystemTime(_Created.date, &st); FILETIME ft={0}; SystemTimeToFileTime(&st,&ft); return ft; };
	FILETIME GetModified() { SYSTEMTIME st={0}; VariantTimeToSystemTime(_Modified.date, &st); FILETIME ft={0}; SystemTimeToFileTime(&st,&ft); return ft; };
	FILETIME GetAccessed() { SYSTEMTIME st={0}; VariantTimeToSystemTime(_Accessed.date, &st); FILETIME ft={0}; SystemTimeToFileTime(&st,&ft); return ft; };
	DWORD GetAttributes() { return _Attrs; };

	BEGIN_COLUMN_MAP(CItem)
		COLUMN_ENTRY(1, _szUrl)
		COLUMN_ENTRY(2, _Size)
		COLUMN_ENTRY(3, _Attrs)
		COLUMN_ENTRY(4, _Created)
		COLUMN_ENTRY(5, _Modified)
		COLUMN_ENTRY(6, _Accessed)
	END_COLUMN_MAP()
};

struct WDSPlugin
{
	DWORD nMagic; // reserved
	int   nCount, nMaxCount;
	struct WDSPluginItem* pItems;
	
	// Search params
	TCHAR  szMask[MAX_PATH];
	TCHAR *pszSearch, *pszAnyField;
	BOOL   bStrictContents;
	
	// Service vars
	CScreenRestore* pScr;
	TCHAR szStatSearch[128], szStatFound[128];
	HRESULT hr;
	TCHAR szErrFunc[128], szErrAddInfo[64];
	PanelMode PanelModes[10];
	const TCHAR* sPanelTitles[1];
	
	WDSPlugin() {
		memset(this, 0, sizeof(struct WDSPlugin));
		lstrcpy(szMask, gszMask);
		pszSearch = lstrdup(gpszSearch ? gpszSearch : _T(""));
		pszAnyField = lstrdup(gpszAnyField ? gpszAnyField : _T(""));
		//bStrictContents = gbStrictContents;
		memset(PanelModes, 0, sizeof(PanelModes));
		sPanelTitles[0] = GetMsg(WDSColTitle);
		PanelModes[0].ColumnTypes = _T("C0");
		PanelModes[0].ColumnWidths = _T("0");
		PanelModes[0].ColumnTitles = (TCHAR**)sPanelTitles;
		PanelModes[0].DetailedStatus = TRUE;
	};
	~WDSPlugin() {
		if (pItems) {
			for (int i = 0; i < nCount; i++)
				if (pItems[i].szName) free(pItems[i].szName);
			free(pItems);
		}
		if (pszSearch) free(pszSearch);
		if (pszAnyField) free(pszAnyField);
	};

	BOOL AddItem(LPCWSTR asURL, ULONGLONG nFileSize, FILETIME ftCreated, FILETIME ftModified, FILETIME ftAccessed, DWORD nFileAttrs)
	{
		if (wcsncmp(asURL, L"file:", 5))
			return TRUE; // не интересует
		asURL += 5;
		while (*asURL == L'/') asURL++;
		int nLen = (int)wcslen(asURL);
		if (nLen < 3)
			return TRUE; // invalid

		if (!nCount || !pItems || nCount >= nMaxCount) {
			int nNewCount = nCount + 1024;
			struct WDSPluginItem* pNew = (struct WDSPluginItem*)malloc(sizeof(struct WDSPluginItem)*nNewCount);
			if (!pNew) {
				_ASSERTE(pNew);
				return FALSE;
			}
			if (nCount) {
				memmove(pNew, pItems, sizeof(struct WDSPluginItem)*nCount);
				free(pItems);
			}
			pItems = pNew;
		}

		//FILETIME ft; SYSTEMTIME st; GetSystemTime(&st); SystemTimeToFileTime(&st, &ft);
		pItems[nCount].dwFileAttributes = nFileAttrs; //FILE_ATTRIBUTE_NORMAL;
		pItems[nCount].ftCreationTime = ftCreated;
		pItems[nCount].ftLastWriteTime = ftModified;
		pItems[nCount].ftLastAccessTime = ftAccessed;
		pItems[nCount].nFileSize.QuadPart = nFileSize;

		pItems[nCount].szName = (TCHAR*)malloc((nLen+1)*sizeof(TCHAR));
		#ifdef _UNICODE
		wcscpy_s(pItems[nCount].szName, nLen+1, asURL);
		#else
		WideCharToMultiByte(CP_OEMCP, 0, asURL, -1, pItems[nCount].szName, nLen+1, 0,0);
		#endif
		TCHAR *pszSlash = pItems[nCount].szName;
		while ((pszSlash = _tcschr(pszSlash, _T('/'))))
			*pszSlash = _T('\\');
		pItems[nCount].pszNamePart = _tcsrchr(pItems[nCount].szName, _T('\\'));
		if (pItems[nCount].pszNamePart) pItems[nCount].pszNamePart++; else pItems[nCount].pszNamePart = pItems[nCount].szName;
		// Succeeded
		nCount++;
		return TRUE;
	};
	
	static LONG_PTR WINAPI ParamsDlgProc(HANDLE hDlg, int Msg, int Param1, LONG_PTR Param2)
	{
		//if (Msg == DN_BTNCLICK) {
		//	if (Param1 == FPCDVideoExtsReset) {
		//		psi.SendDlgMessage(hDlg, DM_SETTEXTPTR, FPCDVideoExtsData, (LONG_PTR)VIDEO_EXTS);
		//		psi.SendDlgMessage(hDlg, DM_SETFOCUS, FPCDVideoExtsData, 0);
		//		return TRUE;
		//	} else if (Param1 == FPCDAudioExtsReset) {
		//		psi.SendDlgMessage(hDlg, DM_SETTEXTPTR, FPCDAudioExtsData, (LONG_PTR)AUDIO_EXTS);
		//		psi.SendDlgMessage(hDlg, DM_SETFOCUS, FPCDAudioExtsData, 0);
		//		return TRUE;
		//	} else if (Param1 == FPCDPictureExtsReset) {
		//		psi.SendDlgMessage(hDlg, DM_SETTEXTPTR, FPCDPictureExtsData, (LONG_PTR)PICTURE_EXTS);
		//		psi.SendDlgMessage(hDlg, DM_SETFOCUS, FPCDPictureExtsData, 0);
		//		return TRUE;
		//	}
		//}
		return psi.DefDlgProc(hDlg, Msg, Param1, Param2);
	}
	// Открыть диалог настроек
	BOOL GetParams()
	{
		BOOL lbOk = FALSE;
	    int height = 14;

		
		enum {
			WDSD_Title = 0,
			WDSD_FileMaskLabel,
			WDSD_FileMask,
			WDSD_SearchTextLabel,
			WDSD_SearchText,
			WDSD_SearchAnyLabel,
			WDSD_SearchAny,
			WDSD_Search,
			WDSD_Cancel,
		};

	    FarDialogItem items[] = {
	        // Common options
	        {DI_DOUBLEBOX,  3,  1,  51, height - 2},        //WDSD_Title

	        {DI_TEXT,       5,  3, 0,  0},        //WDSD_FileMaskLabel
	        {DI_EDIT,       5,  4, 49, 0, true, (DWORD_PTR)_T("Masks"), DIF_HISTORY},   //WDSD_FileMask

	        {DI_TEXT,       5,  5, 0,  0,},        //WDSD_SearchTextLabel
	        {DI_EDIT,       5,  6, 49, 0, false, (DWORD_PTR)_T("SearchText"), DIF_HISTORY}, //WDSD_SearchText
	        {DI_TEXT,       5,  7, 0,  0,},        //WDSD_SearchAnyLabel
	        {DI_EDIT,       5,  8, 49, 0, false, (DWORD_PTR)_T("SearchText"), DIF_HISTORY}, //WDSD_SearchAny

			//{DI_CHECKBOX,   5,  7,  0,  0, true},    //WDSD_StrictContents

	        {DI_BUTTON,     0, 10, 0,  0, false, false, DIF_CENTERGROUP, true,},     //WDSD_Search
	        {DI_BUTTON,     0, 10, 0,  0, false, false, DIF_CENTERGROUP, false,},    //WDSD_Cancel
	    };
	    
	    SETTEXT(items[WDSD_Title], GetMsg(WDSPluginName));
	    SETTEXT(items[WDSD_FileMaskLabel], GetMsg(WDSDlgMasks));
	    SETTEXT(items[WDSD_SearchTextLabel], GetMsg(WDSDlgText));
	    SETTEXT(items[WDSD_SearchAnyLabel], GetMsg(WDSDlgTextAnyField));
	    //SETTEXT(items[WDSD_StrictContents], GetMsg(WDSDlgStrictContents));
	    SETTEXT(items[WDSD_Search], GetMsg(WDSDlgBtnFind));
	    SETTEXT(items[WDSD_Cancel], GetMsg(WDSDlgBtnCancel));

		//items[WDSD_StrictContents].Selected = bStrictContents;

	    SETTEXT(items[WDSD_FileMask], szMask);
	    SETTEXT(items[WDSD_SearchText], pszSearch);
	    SETTEXT(items[WDSD_SearchAny], pszAnyField);
	    

	    int dialog_res = 0;

		#ifdef _UNICODE
		HANDLE hDlg = psi.DialogInit ( psi.ModuleNumber, -1, -1, 55, height,
			L"SearchDlg"/*HelpTopic*/, items, ARRAYSIZE(items), 0, 0/*Flags*/, WDSPlugin::ParamsDlgProc, 0/*DlgProcParam*/ );
		#endif


	    #ifndef _UNICODE
	    //dialog_res = psi.Dialog(psi.ModuleNumber, -1, -1, 55, height, _T("Configure"), items, NUM_ITEMS(items));
		dialog_res = psi.DialogEx(psi.ModuleNumber,-1,-1,55,height, "SearchDlg"/*HelpTopic*/, items, ARRAYSIZE(items), 0, 0,
			WDSPlugin::ParamsDlgProc, NULL);
	    #else
	    dialog_res = psi.DialogRun ( hDlg );
	    #endif

	    if (dialog_res != -1 && dialog_res != WDSD_Cancel)
	    {
	    	//gbStrictContents = bStrictContents = _GetCheck(WDSD_StrictContents);
	    	
	    	lstrcpyn(szMask, GetDataPtr(WDSD_FileMask), ARRAYSIZE(szMask));
	    	lstrcpy(gszMask, szMask);

	    	//pszSearch, pszAnyField.
	    	TCHAR** ppsz[] = {&pszSearch, &pszAnyField};
	    	TCHAR** gppsz[] = {&gpszSearch, &gpszAnyField};
	    	int nID[] = {WDSD_SearchText, WDSD_SearchAny};
	    	for (int k = 0; k < ARRAYSIZE(nID); k++)
	    	{
		    	const TCHAR* pszText = GetDataPtr(nID[k]);
		    	// пропускаем недопустимые символы.
				while (*pszText == _T(' ') || *pszText == _T('\'') || /**pszText == _T('\"') ||*/ *pszText == _T('.')
						|| *pszText == _T('\t'))
					pszText++;
		    	if (lstrlen(pszText) > lstrlen(*(ppsz[k]))) {
		    		free(*(ppsz[k])); *(ppsz[k]) = (TCHAR*)malloc((lstrlen(pszText)+1)*sizeof(TCHAR));
		    	}
					
		    	lstrcpy(*(ppsz[k]), pszText);
		    	
				TCHAR* pszColon = *(ppsz[k]);
				while (*pszColon) {
					if (*pszColon == _T('\'')) *pszColon = _T(' ');
					/*else if (*pszColon == _T('\"')) *pszColon = _T(' ');*/
					else if (*pszColon == _T('.')) *pszColon = _T(' ');
					else if (*pszColon == _T('\\')) *pszColon = _T(' ');
					pszColon++;
				}
				while (pszColon > *(ppsz[k]) && *(pszColon-1) == _T(' ')) {
					pszColon--;
					*pszColon = 0;
				}
				
		    	if (lstrlen(*(ppsz[k])) > lstrlen(*(gppsz[k]))) {
		    		free(*(gppsz[k])); *(gppsz[k]) = (TCHAR*)malloc((lstrlen(*(ppsz[k]))+1)*sizeof(TCHAR));
		    	}
		    	lstrcpy(*(gppsz[k]), *(ppsz[k]));
	    	}
	    	
	    	lbOk = TRUE;
	    }
	    #ifdef _UNICODE
	    psi.DialogFree ( hDlg );
	    #endif
	    
	    return lbOk;
	};

	HRESULT DoSearch()
	{
		hr = S_OK;
		szErrFunc[0] = szErrAddInfo[0] = 0;
		
		hr = CoInitialize(NULL);

		try
		{
			CString strQuery; BOOL lbWhere = FALSE;
			LCID lcidContentLocaleParam = GetUserDefaultLCID();
			wchar_t szLCID[16]; wsprintfW(szLCID, L"%i", (DWORD)lcidContentLocaleParam);
			strQuery = L"select System.ItemURL, System.Size, System.FileAttributes, System.DateCreated, System.DateModified, System.DateAccessed from SystemIndex ";
			if (*szMask && _tcscmp(szMask, _T("*.*")) && _tcscmp(szMask, _T("*.")) && _tcscmp(szMask, _T("*")))
			{
				TCHAR  szConv[MAX_PATH];
				if (_tcschr(szMask, _T('*')) || _tcschr(szMask, _T('?'))) {
					// Для использования "LIKE" нужно заменить '*'->'%' и '?'->'_'
					TCHAR *pszSrc = szMask; TCHAR *pszDst = szConv;
					while (*pszSrc) {
						switch (*pszSrc) {
						case _T('*'): *pszDst = _T('%'); break; // для WDS '%' это аналог '*'
						case _T('?'): *pszDst = _T('_'); break; // для WDS '_' это аналог '?'
						case _T('\''): *pszDst = _T('_'); break; // в маске не допустимо
						case _T('\"'): pszSrc++; continue; // Invalid? в маске не допустимо
						default: *pszDst = *pszSrc;
						}
						pszSrc++; pszDst++;
					}
					*pszDst = 0;
					// Добавляем
					if (lbWhere) strQuery += L" AND "; else strQuery += L" WHERE ";
					strQuery += L"(\"System.FileName\" LIKE '";
					#ifndef _UNICODE
					OemToCharBuff(szConv, szConv, lstrlen(szConv));
					#endif
					strQuery += szConv;
					strQuery += L"')";
				} else {
					if (lbWhere) strQuery += L" AND "; else strQuery += L" WHERE ";
					strQuery += L"(\"System.FileName\" = '";
					#ifdef _UNICODE
					strQuery += szMask;
					#else
					OemToCharBuff(szMask, szConv, lstrlen(szMask)+1);
					strQuery += szConv;
					#endif
					strQuery += L"')";
				}
				lbWhere = TRUE;
			}
			TCHAR** ppsz[] = {&pszSearch, &pszAnyField};
			for (int k = 0; k < ARRAYSIZE(ppsz); k++)
			{
				if (**(ppsz[k])) {
					if (lbWhere) strQuery += L" AND "; else strQuery += L" WHERE ";
					// некоторые операторы идут самостоятельно
					// CONTAINS('FORMSOF(INFLECTIONAL,"run")')
					bool bAddQuot = (_tcschr(*(ppsz[k]), _T('\"'))==NULL && _tcschr(*(ppsz[k]), _T('~'))==NULL);
					//if (bStrictContents)
					if (k == 0)
						strQuery += L"CONTAINS(System.Search.Contents,'";
					else
						strQuery += L"CONTAINS(*,'";
					if (bAddQuot) strQuery += L"\"";
					#ifdef _UNICODE
					strQuery += *(ppsz[k]);
					#else
					int nLen = lstrlen(*(ppsz[k]))+1;
					char* pszAnsi = (char*)malloc(nLen);
					OemToCharBuff(*(ppsz[k]), pszAnsi, nLen);
					strQuery += pszAnsi;
					free(pszAnsi);
					#endif
					if (bAddQuot) strQuery += L"\"";
					strQuery += L"',";
					strQuery += szLCID;
					strQuery += L")";
					lbWhere = TRUE;
				}
			}

			// set the connection string
			CDataSource cDataSource;
			lstrcpy(szErrFunc, _T("cDataSource.OpenFromInitializationString"));
			hr = cDataSource.OpenFromInitializationString(L"provider=Search.CollatorDSO.1;EXTENDED PROPERTIES=\"Application=Windows\"");
			if (SUCCEEDED(hr))
			{
				CSession cSession;
				lstrcpy(szErrFunc, _T("cSession.Open"));
				hr = cSession.Open(cDataSource);
				if (SUCCEEDED(hr))
				{
					CCommand<CAccessor<CItem>, CRowset> cItems;
					// execute the SQL, get back the recordset
					pScr->Message(szStatSearch, TRUE); // "Executing search..."
					lstrcpy(szErrFunc, _T("cItems.Open"));
					hr = cItems.Open(cSession, strQuery);
					if (SUCCEEDED(hr))
					{
						
						TCHAR szMsg[200+MAX_PATH]; DWORD nLastTick = GetTickCount(), nCurTick;
						// move to first item
						lstrcpy(szErrFunc, _T("cItems.MoveFirst"));
						for (hr = cItems.MoveFirst(); hr == S_OK; hr = cItems.MoveNext())
						{
							AddItem(cItems.GetUrl(), cItems.GetSize(), cItems.GetCreated(), cItems.GetModified(), cItems.GetAccessed(), cItems.GetAttributes());
							
							nCurTick = GetTickCount();
							if (nCount == 1 || (nCurTick - nLastTick) >= 500) {
								nLastTick = nCurTick;
								wsprintf(szMsg, szStatFound, nCount);
								pScr->Message(szMsg, nCount<=1);
								lstrcpy(szErrFunc, _T("cItems.MoveNext"));
								
								if (CheckForEsc(WDSConfirmStop)) {
									break;
								}
							}
						}

						// if we hit the end of the result set it's a normal S_OK 
						if (hr == DB_S_ENDOFROWSET)
							hr = S_OK;
					}
				}
			}
		}
		catch (CAtlException e)
		{
			hr = e.m_hr;
			lstrcpy(szErrAddInfo, _T("ATL Threw an exception"));
		}
		catch (...)
		{
			hr = E_FAIL;
			lstrcpy(szErrAddInfo, _T("Unknown exception"));
		}
		
		CoUninitialize();
		return hr;
	}

	HANDLE Execute()
	{
		// Открыть диалог настроек
		if (!GetParams()) {
			delete this;
			return INVALID_HANDLE_VALUE;
		}
		
		lstrcpyn(szStatSearch, GetMsg(WDSStatusSearch), ARRAYSIZE(szStatSearch));
		lstrcpyn(szStatFound, GetMsg(WDSStatusDocsFound), ARRAYSIZE(szStatFound));
		
		pScr = new CScreenRestore(GetMsg(WDSStatusSearch), GetMsg(WDSPluginName));
		
		// Выполнить поиск
		DoSearch();
		
		if (FAILED(hr)) {
			//HRESULT hr;
			//TCHAR szErrFunc[128], szErrAddInfo[64];
			TCHAR* pszFull = NULL;
			TCHAR* lpMsgBuf=NULL;
			if (FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, 
				(DWORD)hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL ))
			{
				while ((pszFull = _tcsstr(lpMsgBuf, _T("\r\n"))) != NULL) {
					pszFull[0] = _T(' '); pszFull[1] = _T('\n');
				}

				#ifndef _UNICODE
				CharToOemBuff(lpMsgBuf, lpMsgBuf, lstrlen(lpMsgBuf));
				#endif
			}
			const TCHAR* pszFormat = GetMsg(WDSErrMessage);
			int nLen = (lpMsgBuf ? lstrlen(lpMsgBuf) : 0) + lstrlen(pszFormat) + lstrlen(szErrFunc) + lstrlen(szErrAddInfo) + 128;
			pszFull = (TCHAR*)malloc(nLen*sizeof(TCHAR));
			wsprintf(pszFull, pszFormat, szErrFunc, (DWORD)hr, szErrAddInfo, lpMsgBuf ? lpMsgBuf : _T(""));
			if (lpMsgBuf) LocalFree(lpMsgBuf);
			
			psi.Message(psi.ModuleNumber,FMSG_ALLINONE|FMSG_WARNING|FMSG_MB_OK,NULL,
				(const TCHAR* const*)pszFull,1,0);

			free(pszFull);
		}

		delete pScr;

		// Возвращаем результат, pItems будут выведены в панель после GetFindData
		return (HANDLE)this;
	};
	
	static void Configure()
	{
		//ls_WinDir = gnv_App.inv_Platform.of_getsystemdirectory ()
		//if Right(ls_WinDir,1)<>'\' then ls_WinDir += '\'
		//ls_Command = '"' + ls_WinDir + "rundll32.exe~" " + "shell32.dll,Control_RunDLL ~"srchadmin.dll~""
		TCHAR szWinDir[MAX_PATH+32] = {0};
		int nLen = GetSystemDirectory(szWinDir, MAX_PATH);
		if (nLen>1) {
			if (szWinDir[nLen-1] != _T('\\')) {
				szWinDir[nLen++] = _T('\\');
				szWinDir[nLen] = 0;
			}
		}
		lstrcat(szWinDir, _T("rundll32.exe"));
		TCHAR szParms[128]; lstrcpy(szParms, _T(" shell32.dll,Control_RunDLL \"srchadmin.dll\""));
		UINT nRc = (UINT)ShellExecute(NULL, _T("open"), szWinDir, szParms, NULL, SW_SHOWNORMAL);
		if (nRc <= 32) {
			const TCHAR* pszFormat = GetMsg(WDSErrCantExecute);
			int nLen = lstrlen(pszFormat) + lstrlen(szWinDir) + lstrlen(szParms) + 128;
			TCHAR* pszFull = (TCHAR*)malloc(nLen*sizeof(TCHAR));
			#ifndef _UNICODE
			CharToOemBuff(szWinDir, szWinDir, lstrlen(szWinDir));
			#endif
			wsprintf(pszFull, pszFormat, szWinDir, szParms, nRc);
			psi.Message(psi.ModuleNumber,FMSG_ALLINONE|FMSG_WARNING|FMSG_MB_OK,NULL,
				(const TCHAR* const*)pszFull,1,0);
		}
	};
	
	static void CheckFolderFileStatus()
	{
		TCHAR* pszBuf = GetSelectedFile(TRUE);
		if (!pszBuf)
			return;
			
		DWORD  dwAttrs = GetFileAttributes(pszBuf);
		BOOL   bFolderIndexed = FALSE;
		if (dwAttrs != -1) {
			HRESULT hr = CoInitialize(NULL);
			int nLen = lstrlen(pszBuf);
			wchar_t* pszURL = (wchar_t*)malloc((nLen+10)*2);
			lstrcpyW(pszURL, L"file:///");
			lstrcpy_t(pszURL+8, nLen+1, pszBuf);
			wchar_t* p = pszURL+lstrlenW(pszURL)-1;
			if ((dwAttrs & FILE_ATTRIBUTE_DIRECTORY) == 0) {
				p = wcsrchr(pszURL, L'\\');
				if (p) {
					p[1] = 0; p++;
				}
			} else {
				p = wcsrchr(pszURL, L'\\');
			}
			if (!p)
			{
				_ASSERTE(p!=NULL);
			}
			
			try
			{
				// Crawl scope manager for that catalog
				ISearchCrawlScopeManager *pSearchCrawlScopeManager = NULL;
				ISearchCatalogManager *pCatalogManager = NULL;
				ISearchManager *pSearchManager = NULL;
				hr = CoCreateInstance(__uuidof(CSearchManager), NULL, CLSCTX_SERVER, IID_PPV_ARGS(&pSearchManager));
				if (SUCCEEDED(hr))
				{
					hr = pSearchManager->GetCatalog(L"SystemIndex", &pCatalogManager);
					pSearchManager->Release(); pSearchManager = NULL;
				}
				if (SUCCEEDED(hr))
				{
					// Crawl scope manager for that catalog
					hr = pCatalogManager->GetCrawlScopeManager(&pSearchCrawlScopeManager);
					pCatalogManager->Release(); pCatalogManager = NULL;
				}
				if (SUCCEEDED(hr))
				{
					// Default scope rules provide an initial set of scope rules, 
					// and they are also used as a fallback set if user-defined rules are reverted.
					// User scope rules always take precedence over default scope rules. 
					BOOL bIncluded;
					hr = pSearchCrawlScopeManager->IncludedInCrawlScope(pszURL, &bIncluded);
					if (SUCCEEDED(hr) && bIncluded)
						bFolderIndexed = TRUE;
					pSearchCrawlScopeManager->Release(); pSearchCrawlScopeManager = NULL;
				}
			}
			catch (CAtlException e)
			{
				hr = e.m_hr;
				//lstrcpy(szErrAddInfo, _T("ATL Threw an exception"));
			}
			catch (...)
			{
				hr = E_FAIL;
				//lstrcpy(szErrAddInfo, _T("Unknown exception"));
			}
			
			CoUninitialize();
			free(pszURL);
		}
		
		const TCHAR *MsgItems[] = {
			GetMsg(WDSPluginName),
			GetMsg((dwAttrs == -1) ? WDSItemNotFound : ((dwAttrs & FILE_ATTRIBUTE_NOT_CONTENT_INDEXED)==0) ? WDSAttibEnabled : WDSAttibDisabled),
			(dwAttrs == -1) ? _T("") : GetMsg(bFolderIndexed ? WDSFolderIndexing : WDSFolderNotIndexing),
			GetMsg(WDSDlgBtnClose),
			(dwAttrs == -1) ? _T("") : GetMsg(bFolderIndexed ? WDSDlgBtnDelFromIndex : WDSDlgBtnAddToIndex),
			};
		bool bNoAddButton = (dwAttrs == -1 || (!bFolderIndexed && (dwAttrs & FILE_ATTRIBUTE_NOT_CONTENT_INDEXED)));
		// пока - кнопку не добавляем
		bNoAddButton = true;
		int nNumItems = ARRAYSIZE(MsgItems) - (bNoAddButton ? 1 : 0);
		int nBtns = bNoAddButton ? 1 : 2;
		
		psi.Message(psi.ModuleNumber,FMSG_LEFTALIGN,NULL,MsgItems,nNumItems,nBtns);
		
		free(pszBuf);
	};
};



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
  HANDLE WINAPI OpenPluginW(int OpenFrom,INT_PTR Item);
  int WINAPI GetMinFarVersionW(void);
  void WINAPI GetPluginInfoW(struct PluginInfo *pi);
  int WINAPI _export GetFindDataW(HANDLE hPlugin,struct PluginPanelItem **pPanelItem,int *pItemsNumber,int OpMode);
  void WINAPI _export FreeFindDataW(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber);
  void WINAPI _export GetOpenPluginInfoW(HANDLE hPlugin,struct OpenPluginInfo *Info);
  void WINAPI _export ClosePluginW(HANDLE hPlugin);
  int WINAPI _export SetDirectoryW ( HANDLE hPlugin, LPCTSTR Dir, int OpMode );
  int WINAPI _export ProcessEventW(HANDLE hPlugin,int Event,void *Param);
  int WINAPI _export ProcessKeyW(HANDLE hPlugin,int Key,unsigned int ControlState);
#ifdef __cplusplus
};
#endif

BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved)
{
  return DllMain(hDll, dwReason,lpReserved);
}
#endif

#ifdef CRTSTARTUP
extern "C"{
BOOL WINAPI _DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved)
{
  return TRUE;
};
};
#endif

int WINAPI _export GetMinFarVersionW(void)
{
	return FARMANAGERVERSION;
}

void WINAPI _export GetPluginInfoW(struct PluginInfo *pi)
{
	static TCHAR szMenu[MAX_PATH];
	lstrcpy(szMenu, GetMsg(WDSPluginName)); // "Windows Search"
    static TCHAR *pszMenu[1];
    pszMenu[0] = szMenu;

	pi->StructSize = sizeof(struct PluginInfo);
	pi->Flags = PF_VIEWER; //|PF_EDITOR;
	pi->DiskMenuStrings = NULL;
	pi->DiskMenuNumbers = 0;
	pi->PluginMenuStrings = pszMenu;
	pi->PluginMenuStringsNumber = 1;
	pi->PluginConfigStrings = NULL;
	pi->PluginConfigStringsNumber = 0;
	pi->CommandPrefix = _T("FWDS");
	//pi->Reserved = 'WdSr';
	MCHKHEAP
}

void WINAPI _export SetStartupInfoW(const PluginStartupInfo *aInfo)
{
	::psi = *aInfo;
	::FSFW = *aInfo->FSF;
	::psi.FSF = &::FSFW;
	
	lstrcpy(gszMask, _T("*.*"));
	if (!gpszSearch) gpszSearch = lstrdup(_T("")); else *gpszSearch = 0;
	if (!gpszAnyField) gpszAnyField = lstrdup(_T("")); else *gpszAnyField = 0;
}

void   WINAPI _export ExitFARW(void)
{
	if (gpszSearch) { free(gpszSearch); gpszSearch = NULL; }
	if (gpszAnyField) { free(gpszAnyField); gpszAnyField = NULL; }
}

HANDLE WINAPI _export OpenPluginW(int OpenFrom,INT_PTR Item)
{
    if ((OpenFrom & 0xFF) == OPEN_COMMANDLINE) {
    	TCHAR* pszCmd = (TCHAR*)Item;
    	BOOL lbEditor = FALSE;
    	if (_tcsnicmp(pszCmd, _T("view:"), 5) == 0) {
    		lbEditor = FALSE; pszCmd += 5;
    	} else if (_tcsnicmp(pszCmd, _T("edit:"), 5) == 0) {
    		lbEditor = TRUE; pszCmd += 5;
    	}

		TCHAR* pszFileName = NULL;
		#ifdef _UNICODE
		int nLen = (int)FSFW.ConvertPath(CPM_FULL, pszCmd, NULL, 0);
		if (nLen > 0) {
			pszFileName = (wchar_t*)calloc((nLen+1),2);
			FSFW.ConvertPath(CPM_FULL, pszCmd, pszFileName, nLen);
		}
		#else
		pszFileName = (char*)calloc(MAX_PATH*2,1);
		char* pszPart;
		GetFullPathName(pszCmd, MAX_PATH*2, pszFileName, &pszPart);
		#endif

    	OpenDocumentSource(pszFileName, lbEditor, FALSE/*abSilence*/);

		free(pszFileName);
		return INVALID_HANDLE_VALUE;
	}

	if ((OpenFrom & 0xFF) == OPEN_VIEWER || (OpenFrom & 0xFF) == OPEN_EDITOR) {
		LPCTSTR pszFileName = NULL;
		ViewerInfo vi = {sizeof(ViewerInfo)};
		if ((OpenFrom & 0xFF) == OPEN_VIEWER) {
			if (psi.ViewerControl(VCTL_GETINFO, &vi))
				pszFileName = vi.FileName;
		} else {
			//TODO: вызов из редактора
		}
		if (!pszFileName)
			return INVALID_HANDLE_VALUE;
			
		enum {
			WDSD_MenuViewDocText,
			WDSD_MenuEditDocText,
			WDSD_MenuLast,
		};
	    FarMenuItem items[WDSD_MenuLast] = {0};

	    SETMENUTEXT(items[WDSD_MenuViewDocText], GetMsg(WDSMenuViewDocText));
	    SETMENUTEXT(items[WDSD_MenuEditDocText], GetMsg(WDSMenuEditDocText));

	    int menu_res = psi.Menu(psi.ModuleNumber, -1, -1, 0, FMENU_WRAPMODE, GetMsg(WDSPluginName), NULL,
	                               NULL/*help*/, NULL, NULL, items, ARRAYSIZE(items));
	    if (menu_res != WDSD_MenuViewDocText && menu_res != WDSD_MenuEditDocText)
	    	return INVALID_HANDLE_VALUE;
	    	
	    OpenDocumentSource(pszFileName, (menu_res == WDSD_MenuEditDocText), FALSE/*abSilence*/);

		return INVALID_HANDLE_VALUE;
		
		//TCHAR szTempFileName[MAX_PATH*3];
		//bool lbDocOk = ExtractDocText(pszFileName, szTempFileName, FALSE);
		//if (lbDocOk) {
		//	
		//	if (menu_res == WDSD_MenuViewDocText) {
		//		psi.Viewer(szTempFileName, NULL, 0,0,-1,-1,
		//			VF_NONMODAL|VF_IMMEDIATERETURN|VF_DELETEONCLOSE|VF_ENABLE_F6
		//			#ifdef _UNICODE
		//			,1200
		//			#endif
		//		);
		//	} else {
		//		psi.Editor(szTempFileName, NULL, 0,0,-1,-1,
		//			EF_NONMODAL|EF_IMMEDIATERETURN|EF_DELETEONCLOSE|EF_ENABLE_F6, 0, 1
  		//			#ifdef _UNICODE
		//			,1200
		//			#endif
		//		);
		//	}
		//}
		
	} else {

		enum {
			WDSD_MenuSearch = 0,
			WDSD_MenuViewSeparator,
			WDSD_MenuViewDocText,
			WDSD_MenuEditDocText,
			WDSD_MenuConfigureSeparator,
			WDSD_MenuConfigure,
			WDSD_MenuFolderProps,
			WDSD_MenuSearchStatus,
			WDSD_MenuLast,
		};
	    FarMenuItem items[WDSD_MenuLast] = {0};

	    SETMENUTEXT(items[WDSD_MenuSearch], GetMsg(WDSMenuSearch));
	    items[WDSD_MenuViewSeparator].Separator = 1;
	    SETMENUTEXT(items[WDSD_MenuViewDocText], GetMsg(WDSMenuViewDocText));
	    SETMENUTEXT(items[WDSD_MenuEditDocText], GetMsg(WDSMenuEditDocText));
	    items[WDSD_MenuConfigureSeparator].Separator = 1;
	    SETMENUTEXT(items[WDSD_MenuConfigure], GetMsg(WDSMenuConfigure));
	    SETMENUTEXT(items[WDSD_MenuFolderProps], GetMsg(WDSMenuFolderProps));
	    SETMENUTEXT(items[WDSD_MenuSearchStatus], GetMsg(WDSMenuSearchStatus));


	    int menu_res = psi.Menu(psi.ModuleNumber, -1, -1, 0, FMENU_WRAPMODE, GetMsg(WDSPluginName), NULL,
	                               NULL/*help*/, NULL, NULL, items, ARRAYSIZE(items));
	    switch (menu_res){
	    	case WDSD_MenuSearch:
	    	{
				WDSPlugin* pPlugin = NULL;
				pPlugin = new WDSPlugin();
				if (!pPlugin) return INVALID_HANDLE_VALUE;
				return pPlugin->Execute();
	    	}
	        case WDSD_MenuViewDocText:
	        case WDSD_MenuEditDocText:
	        	OpenDocumentSource(NULL, menu_res==WDSD_MenuEditDocText, FALSE/*abSilence*/);
	        	return INVALID_HANDLE_VALUE;
	        case WDSD_MenuConfigure:
	        	WDSPlugin::Configure();
	        	return INVALID_HANDLE_VALUE;
	        case WDSD_MenuFolderProps:
				WDSPlugin::CheckFolderFileStatus();
	        	return INVALID_HANDLE_VALUE;
	        case WDSD_MenuSearchStatus:
	        	{
					WDSStatus *pSearch = new WDSStatus();
					pSearch->ShowStatusDlg();
					delete pSearch;
				}
	        	return INVALID_HANDLE_VALUE;
	    }
    }
    
	return INVALID_HANDLE_VALUE;
}

int WINAPI _export GetFindDataW(HANDLE hPlugin,struct PluginPanelItem **pPanelItem,int *pItemsNumber,int OpMode)
{
	WDSPlugin* pPlugin = (WDSPlugin*)hPlugin;
	int nCount = 0, i;
	//LARGE_INTEGER ll;
	PluginPanelItem* ppi = NULL;

	const TCHAR * pszFormat = GetMsg(WDSStatusSendToPanel);
	TCHAR szMsg[128]; wsprintf(szMsg, pszFormat, 0, pPlugin->nCount);
	DWORD nLastTick = GetTickCount(), nCurTick;
	CScreenRestore scr(szMsg, GetMsg(WDSPluginName));

	*pItemsNumber = nCount = pPlugin->nCount;
	ppi = (PluginPanelItem*)calloc(nCount, sizeof(PluginPanelItem));
	*pPanelItem = ppi;

	MCHKHEAP

	for (i=0; i<nCount; i++) {
		ppi[i].FindData.dwFileAttributes = pPlugin->pItems[i].dwFileAttributes;
		ppi[i].FindData.ftCreationTime = pPlugin->pItems[i].ftCreationTime;
		ppi[i].FindData.ftLastAccessTime = pPlugin->pItems[i].ftLastAccessTime;
		ppi[i].FindData.ftLastWriteTime = pPlugin->pItems[i].ftLastWriteTime;
#ifdef _UNICODE
		ppi[i].FindData.lpwszFileName = pPlugin->pItems[i].szName;
		ppi[i].FindData.nFileSize = pPlugin->pItems[i].nFileSize.QuadPart;
#else
		lstrcpyn(ppi[i].FindData.cFileName, pPlugin->pItems[i].szName, ARRAYSIZE(ppi[i].FindData.cFileName));
		ppi[i].FindData.nFileSizeLow  = pPlugin->pItems[i].nFileSize.LowPart;
		ppi[i].FindData.nFileSizeHigh = pPlugin->pItems[i].nFileSize.HighPart;
#endif
		ppi[i].CustomColumnNumber = 1;
		ppi[i].CustomColumnData = (TCHAR**)&(pPlugin->pItems[i].pszNamePart);

		nCurTick = GetTickCount();
		if ((nCurTick - nLastTick) >= 500) {
			nLastTick = nCurTick;
			wsprintf(szMsg, pszFormat, i+1, nCount);
			scr.Message(szMsg, FALSE);

			if (CheckForEsc(WDSConfirmStopSend)) {
				break;
			}
		}
	}
	MCHKHEAP;

	wsprintf(szMsg, pszFormat, i+1, nCount);
	scr.Message(szMsg, FALSE);

	return TRUE;
}

void WINAPI _export FreeFindDataW(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber)
{
	if (PanelItem)
		free(PanelItem);
	return;
}

void WINAPI _export GetOpenPluginInfoW(HANDLE hPlugin,struct OpenPluginInfo *Info)
{
	WDSPlugin* pPlugin = (WDSPlugin*)hPlugin;
	Info->CurDir = _T("");
	Info->HostFile = NULL;
	Info->PanelTitle = psi.GetMsg(psi.ModuleNumber, WDSPluginName);
	Info->Flags = OPIF_USEFILTER|OPIF_USESORTGROUPS|OPIF_USEHIGHLIGHTING|OPIF_ADDDOTS|OPIF_REALNAMES;
	static KeyBarTitles kbt = {{NULL}};
	kbt.ShiftTitles[2] = GetMsg(WDSKeyView); kbt.ShiftTitles[3] = GetMsg(WDSKeyEdit);
	Info->KeyBar = &kbt;
	// Режим панели
	Info->StartPanelMode = _T('0');
	Info->PanelModesNumber = 1;
	Info->PanelModesArray = pPlugin->PanelModes;
	MCHKHEAP
}

void WINAPI _export ClosePluginW(HANDLE hPlugin)
{
	WDSPlugin* pPlugin = (WDSPlugin*)hPlugin;
	if (pPlugin) {
		delete pPlugin;
	}
}



int WINAPI _export ProcessKeyW(HANDLE hPlugin,int Key,unsigned int ControlState)
{
	if (((Key & 0xFF) == VK_F3 || (Key & 0xFF) == VK_F4) && ControlState == PKF_SHIFT) {
		OpenDocumentSource(NULL, (Key & 0xFF) == VK_F4, FALSE/*abSilence*/);
		return TRUE;
	} else if ((Key & 0xFF) == VK_F9 && ControlState == (PKF_SHIFT|PKF_ALT)) {
		WDSPlugin::Configure();
		return TRUE;
	}
	return FALSE;
}
