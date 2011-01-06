
#include <windows.h>
#include <ShlObj.h>
#define DEFINE_OPTIONS
#include "../../Options.h"
#include "../PictureViewPlugin.h"
#include "../PVD2Helper.h"

typedef int i32;
typedef unsigned char u8;
typedef unsigned short u16;
typedef DWORD u32;
//#include <vector>


extern HMODULE ghModule; // = NULL;
BOOL gbCancelAll = FALSE;

// Функция требуется только для заполнения переменной ghModule
// Если плагин содержит свою точку входа - для использования PVD1Helper
// ему необходимо заполнять ghModule самостоятельно
//BOOL WINAPI DllMain( HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved )
//{
//	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
//		ghModule = (HMODULE)hModule;
//	return TRUE;
//}

#define DEFAULT_RENDER_SIZE 400

pvdInitPlugin2 ip = {0};

#define PSE_INVALID_FRAME        0x1001
#define PSE_EXCEPTION            0x80001002
#define PSE_FILE_NOT_FOUND       0x80001003
#define PSE_NOT_ENOUGH_MEMORY    0x80001004
#define PSE_INVALID_CONTEXT      0x80001005
#define PSE_WIN32_ERROR          0x80001007
#define PSE_UNKNOWN_COLORSPACE   0x80001008
#define PSE_INVALID_PAGEDATA     0x8000100A
#define PSE_UNKNOWN_FORMAT       0x8000100B
#define PSE_INVALID_FILE_NAME    0x8000100C
#define PSE_UNC_NOT_SUPPORTED    0x8000100D
#define PSE_NO_DESKTOP_FOLDER    0x8000100E
#define PSE_GETDIBITS_FAILED     0x8000100F
#define PSE_BITBLT_FAILED        0x80001010
#define PSE_OLD_PICVIEW          0x80001011

HRESULT ghLastWin32Error = 0;
wchar_t* gsExtensions = NULL;
bool gbAutoDetectAlphaChannel = true;

BOOL __stdcall pvdTranslateError2(DWORD nErrNumber, wchar_t *pszErrInfo, int nBufLen)
{
	if (!pszErrInfo || nBufLen<=0)
		return FALSE;

	switch (nErrNumber)
	{
	case PSE_WIN32_ERROR:
		FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS, 0, 
			(DWORD)ghLastWin32Error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			pszErrInfo, nBufLen, NULL);
		break;
	case PSE_UNC_NOT_SUPPORTED:
		lstrcpynW(pszErrInfo, L"UNC paths are not supported", nBufLen); break;
	case PSE_NO_DESKTOP_FOLDER:
		lstrcpynW(pszErrInfo, L"Desktop folder not found", nBufLen); break;
	case PSE_UNKNOWN_COLORSPACE:
		lstrcpynW(pszErrInfo, L"Unsupported output colorspace", nBufLen); break;
	case PSE_EXCEPTION:
		lstrcpynW(pszErrInfo, L"Exception occurred", nBufLen); break;
	case PSE_FILE_NOT_FOUND:
		lstrcpynW(pszErrInfo, L"File not found", nBufLen); break;
	case PSE_NOT_ENOUGH_MEMORY:
		lstrcpynW(pszErrInfo, L"Not enough memory", nBufLen); break;
	case PSE_INVALID_CONTEXT:
		lstrcpynW(pszErrInfo, L"Invalid context", nBufLen); break;
	case PSE_INVALID_PAGEDATA:
		lstrcpynW(pszErrInfo, L"Invalid pDisplayCreate->pImage->lParam", nBufLen); break;
	case PSE_UNKNOWN_FORMAT:
		lstrcpynW(pszErrInfo, L"Unsupported RAW format, codec is not available", nBufLen); break;
	case PSE_GETDIBITS_FAILED:
		lstrcpynW(pszErrInfo, L"GetDIBits failed", nBufLen); break;
	case PSE_BITBLT_FAILED:
		lstrcpynW(pszErrInfo, L"BitBlt failed", nBufLen); break;
	case PSE_OLD_PICVIEW:
		lstrcpyn(pszErrInfo, L"Old PicView version, exiting", nBufLen); break;
	default:
		return FALSE;
	}
	return TRUE;
}

#define DllGetFunction(hModule, FunctionName) FunctionName = (FunctionName##_t)GetProcAddress(hModule, #FunctionName)
//#define MALLOC(n) HeapAlloc(GetProcessHeap(), 0, n)
#define CALLOC(n) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, n)
#define FREE(p) HeapFree(GetProcessHeap(), 0, p)
#define STRING2(x) #x
#define STRING(x) STRING2(x)
#define FILE_LINE __FILE__ "(" STRING(__LINE__) "): "
#ifdef HIDE_TODO
#define TODO(s) 
#define WARNING(s) 
#else
#define TODO(s) __pragma(message (FILE_LINE "TODO: " s))
#define WARNING(s) __pragma(message (FILE_LINE "warning: " s))
#endif
#define PRAGMA_ERROR(s) __pragma(message (FILE_LINE "error: " s))

template <typename T>
inline void SafeRelease(T *&p)
{
	if (NULL != p)
	{
		p->Release();
		p = NULL;
	}
}


/*
const IID IID_IExtractImage = {0xBB2E617C,0x0920,0x11d1,{0x9A,0x0B,0x00,0xC0,0x4F,0xC2,0xD6,0xC1}};
*/

void __stdcall pvdReloadConfig2(void *pContext)
{
	if (!ip.pRegKey) return;
	
	PVDSettings set(ip.pRegKey);
	bool bDefault = true;
	set.GetParam(L"DetectAlphaChannel", L"bool;Autodetect alpha channel",
		REG_BINARY, &bDefault, &gbAutoDetectAlphaChannel, sizeof(gbAutoDetectAlphaChannel));
}

UINT32 __stdcall pvdInit2(pvdInitPlugin2* pInit)
{
	_ASSERTE(pInit->cbSize == sizeof(pvdInitPlugin2));
	if (pInit->cbSize < sizeof(pvdInitPlugin2)) {
		pInit->nErrNumber = PSE_OLD_PICVIEW;
		return 0;
	}

	memset(&ip,0,sizeof(ip));
	memmove(&ip, pInit, min(sizeof(ip),pInit->cbSize));
	ghModule = ip.hModule;
	
	pvdReloadConfig2(NULL);

	ghLastWin32Error = CoInitialize(NULL);
	if (FAILED(ghLastWin32Error)) {
		pInit->nErrNumber = PSE_WIN32_ERROR;
		return 0;
	}

	// Для работы с Shell'ом необходим "Desktop folder"
	// В некоторых WinPE он может быть не создан изначально
	LPITEMIDLIST pDesktopID = NULL;
	wchar_t szDesktopPath[MAX_PATH+1];
	ghLastWin32Error = SHGetFolderPath ( NULL, CSIDL_DESKTOP, NULL, SHGFP_TYPE_CURRENT, szDesktopPath );
	if (ghLastWin32Error == S_OK) {
		DWORD dwAttrs = GetFileAttributes(szDesktopPath);
		if (dwAttrs == (DWORD)-1) {
			// Папка отсутсвует

		} else if ((dwAttrs & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY) {
			ghLastWin32Error = SHGetFolderLocation ( NULL, CSIDL_DESKTOP, NULL, 0, &pDesktopID );
			if (ghLastWin32Error != S_OK && ip.MessageLog) {
				wsprintf(szDesktopPath, L"SHGetFolderLocation()=0x%08X", (DWORD)ghLastWin32Error);
				ip.MessageLog(ip.pCallbackContext, szDesktopPath, 2);
			}

		}
	}
	if (pDesktopID == NULL) {
		pInit->nErrNumber = PSE_NO_DESKTOP_FOLDER;
		CoUninitialize();
		return 0;
	}

	if (pDesktopID) { CoTaskMemFree(pDesktopID); pDesktopID = NULL; }

	return PVD_UNICODE_INTERFACE_VERSION;
}

// обычно возвращает следущие расширения
// 3FR,APS,ARW,ASCX,ASP,BAT,BIN,BKF,BMP,BSC,CAB,CDA,CGM,CMD,COM,CR2,CRW,DBG,DCR,DCT,DIB,DIC,DLL,
// DL_,DNG,DOC,DOT,DVD,EMF,EPS,EXE,EXP,EX_,EYB,FFF,FND,FNT,FOLDER,FON,GHI,GIF,GZ,HHC,HQX,HTA,HTW,
// HTX,ICM,ICO,IDB,IDQ,ILK,IMC,INF,INI,INV,INX,IN_,IVF,JBF,JFIF,JPE,JPEG,JPG,JS,KDC,LATEX,LIB,LOG,
// M14,MDB,MDI,MMF,MOVIE,MRW,MSG,MV,MYDOCS,NCB,NEF,NRW,OBJ,OC_,ORF,PCH,PDB,PDF,PDS,PEF,PIC,PMA,PMC,
// PML,PMR,PNG,POT,PPT,PSD,RAF,RAW,REG,RES,RLE,RPC,RSP,RTF,RW2,RWL,SBR,SC2,SIT,SR2,SRF,SR_,STM,SYM,
// SY_,TAR,TGA,TGZ,TIF,TIFF,TLB,TPIC,TSP,TTC,TTF,URL,VBS,VBX,WDP,WLL,WLT,WMF,WMP,WMZ,WSZ,WTX,X3F,
// XIX,XLB,XLC,XLS,XLT,XPS,Z,Z96,ZIP,SPFILE
//void GetFormatsByPersistentHandler(void *pContext, pvdFormats2* pFormats)
//{
//	_ASSERTE(pFormats->cbSize >= sizeof(pvdFormats2));
//
//	/* вроде бы наличие этого позволяет смотреть Thumbs
//	[HKEY_CLASSES_ROOT\.jpg]
//	"Content Type"="image/jpeg"
//	[HKEY_CLASSES_ROOT\.jpg\PersistentHandler]
//	@="{098f2470-bae0-11cd-b579-08002b30bfeb}"
//	*/
//
//
//	wchar_t szExt[MAX_PATH], szInfo[MAX_PATH];
//	DWORD dwIndex = 0, nAllLen = 0, nLen = 0, nMaxLen = 4096, nData;
//	HKEY hk = 0;
//	if (!gsExtensions) {
//		gsExtensions = (wchar_t*)CALLOC((nMaxLen+1)*2);
//		_ASSERTE(gsExtensions);
//		if (!gsExtensions) return;
//	} else {
//		gsExtensions[0] = 0;
//	}
//	//lstrcpy(gsExtensions, L"<DIR>"); nAllLen = lstrlen(gsExtensions);
//	while (!RegEnumKeyEx(HKEY_CLASSES_ROOT, dwIndex++, szExt, &(nLen = 64), 0, 0, 0, 0))
//	{
//		if (nLen <= 1 || nLen > 7) continue;
//
//		BOOL lbImageType = FALSE;
//		if (RegOpenKeyEx(HKEY_CLASSES_ROOT, szExt, 0, KEY_READ, &hk))
//			continue; // Ошибка доступа
//		if (!RegQueryValueEx(hk, L"Content Type", 0, 0, (LPBYTE)szInfo, &(nData = sizeof(szInfo)))) {
//			lbImageType = wcsncmp(szInfo, L"image/", 6) == 0;
//			if (!wcsncmp(szInfo, L"video/", 6) || !wcsncmp(szInfo, L"audio/", 6) || !wcsncmp(szInfo, L"text/", 5)) {
//				RegCloseKey(hk); continue; // Видео и Аудио в PicView НЕ обрабатываем, чтобы не подраться с MMView
//			}
//		}
//		if (!lbImageType) {
//			if (!RegQueryValueEx(hk, L"Generic", 0, 0, (LPBYTE)szInfo, &(nData = sizeof(szInfo)))) {
//				if (!wcsncmp(szInfo, L"system", 6) || !wcsncmp(szInfo, L"video", 5) || !wcsncmp(szInfo, L"audio", 5) || !wcsncmp(szInfo, L"text", 4)) {
//					RegCloseKey(hk); continue; // Видео и Аудио в PicView НЕ обрабатываем, чтобы не подраться с MMView
//				}
//			}
//			if (!RegQueryValueEx(hk, L"PerceivedType", 0, 0, (LPBYTE)szInfo, &(nData = sizeof(szInfo)))) {
//				if (!wcsncmp(szInfo, L"system", 6) || !wcsncmp(szInfo, L"video", 5) || !wcsncmp(szInfo, L"audio", 5) || !wcsncmp(szInfo, L"text", 4)) {
//					RegCloseKey(hk); continue; // Видео и Аудио в PicView НЕ обрабатываем, чтобы не подраться с MMView
//				}
//			}
//		}
//		RegCloseKey(hk);
//
//		if (!lbImageType) {
//			lstrcat(szExt, L"\\PersistentHandler");
//			if (RegOpenKeyEx(HKEY_CLASSES_ROOT, szExt, 0, KEY_READ, &hk))
//				continue; // Отсутствует "PersistentHandler"
//			RegCloseKey(hk);
//			szExt[nLen] = 0;
//		}
//
//
//		if (nMaxLen >= (nAllLen + nLen + 1)) {
//			if (nAllLen) gsExtensions[nAllLen++] = L',';
//			lstrcpy(gsExtensions+nAllLen, szExt+1);
//			nAllLen += nLen - 1;
//		}
//	}
//
//	CharUpperBuff(gsExtensions, nAllLen);
//	pFormats->pSupported = gsExtensions;
//	pFormats->pIgnored = L"";
//}

typedef struct {
	wchar_t s[8];
} WC8;

// Возвращает следующее
// 3FR,ARW,CR2,CRW,DCR,DNG,FFF,KDC,LNK,MRW,NEF,NRW,ORF,PEF,PFI,PFS,PSD,RAF,RAW,RW2,RWL,SR2,SRF,TGA,TPIC,WDP,X3F
void GetFormatsByIExtractImage(void *pContext, pvdFormats2* pFormats)
{
	_ASSERTE(pFormats->cbSize >= sizeof(pvdFormats2));

	/* вроде бы наличие этого позволяет смотреть Thumbs
	[HKEY_CLASSES_ROOT\.jpg]
	"Content Type"="image/jpeg"
	[HKEY_CLASSES_ROOT\.jpg\PersistentHandler]
	@="{098f2470-bae0-11cd-b579-08002b30bfeb}"
	*/


	wchar_t szExt[MAX_PATH], szInfo[MAX_PATH], szName[MAX_PATH];
	DWORD dwIndex = 0, nAllLen = 0, nLen = 0, nMaxLen = 4096, nData;
	HKEY hk = 0;
	if (!gsExtensions) {
		gsExtensions = (wchar_t*)CALLOC((nMaxLen+1)*2);
		_ASSERTE(gsExtensions);
		if (!gsExtensions) return;
	} else {
		gsExtensions[0] = 0;
	}
	//lstrcpy(gsExtensions, L"<DIR>"); nAllLen = lstrlen(gsExtensions);
	// 3FR,ARW,CR2,CRW,DCR,DNG,FFF,KDC,LNK,MRW,NEF,NRW,ORF,PEF,PFI,PFS,PSD,RAF,RAW,RW2,RWL,SR2,SRF,TGA,TPIC,WDP,X3F
	// Список расширений, которые рендерятся, но не находятся через реестр
	lstrcpy(gsExtensions, L"BMP,GIF,JPE,JPG,JPEG,PNG,TIF,TIFF,EMF,WMF,PDF,"); nAllLen = lstrlen(gsExtensions);
	//lstrcpy(gsExtensions, L","); nAllLen = lstrlen(gsExtensions);
	//std::vector<WC8> lExts;
	//std::vector<WC8>::iterator i, j, liMin;
	
	DWORD nDataSize = 65536;
	wchar_t* pszData = (wchar_t*)CALLOC((nDataSize+1)*2);
	wchar_t* pszCur = pszData;

	while (!RegEnumKeyEx(HKEY_CLASSES_ROOT, dwIndex++, szExt, &(nLen = 64), 0, 0, 0, 0)) {
		if (nLen <= 1 || nLen > 7 || szExt[0] != L'.') continue;
		CharUpperBuff(szExt, nLen);
		//WC8 e; lstrcpy(e.s, szExt);
		//lExts.push_back(e);
		if (nMaxLen > ((pszCur - pszData) + nLen)) {
			lstrcpyW(pszCur, szExt);
			pszCur += nLen+1;
		}
	}
	wchar_t* pszEnd = pszCur;

	//for (i=lExts.begin(); i!=lExts.end() && (i+1)!=lExts.end(); i++) {
	//	liMin = i;
	//	for (j=i+1; j!=lExts.end(); j++) {
	//		if (lstrcmpW(liMin->s, j->s) > 0)
	//			liMin = j;
	//	}
	//	if (liMin!=i) {
	//		lstrcpy(szExt, liMin->s);
	//		lstrcpy(liMin->s, i->s);
	//		lstrcpy(i->s, szExt);
	//	}
	//}

	//i = lExts.begin();
	//while (i!=lExts.end())
	pszCur = pszData;
	while (pszCur < pszEnd)
	{
		//lstrcpy(szExt, i->s);
		lstrcpy(szExt, pszCur);
		//i++;
		nLen = lstrlen(szExt);
		pszCur += nLen+1;

		BOOL lbImageType = FALSE, lbCanHandle = FALSE;
		if (RegOpenKeyEx(HKEY_CLASSES_ROOT, szExt, 0, KEY_READ, &hk))
			continue; // Ошибка доступа
		if (!RegQueryValueEx(hk, L"Content Type", 0, 0, (LPBYTE)szInfo, &(nData = sizeof(szInfo)))) {
			lbImageType = wcsncmp(szInfo, L"image/", 6) == 0;
			if (!wcsncmp(szInfo, L"video/", 6) || !wcsncmp(szInfo, L"audio/", 6) || !wcsncmp(szInfo, L"text/", 5)) {
				RegCloseKey(hk); continue; // Видео и Аудио в PicView НЕ обрабатываем, чтобы не подраться с MMView
			}
		}
		if (RegQueryValueEx(hk, NULL, 0, 0, (LPBYTE)szName, &(nData = 128))) {
			RegCloseKey(hk); continue;
		}
		if (!lbImageType) {
			if (!RegQueryValueEx(hk, L"Generic", 0, 0, (LPBYTE)szInfo, &(nData = sizeof(szInfo)))) {
				if (!wcsncmp(szInfo, L"system", 6) || !wcsncmp(szInfo, L"video", 5) || !wcsncmp(szInfo, L"audio", 5) || !wcsncmp(szInfo, L"text", 4)) {
					RegCloseKey(hk); continue; // Видео и Аудио в PicView НЕ обрабатываем, чтобы не подраться с MMView
				}
			}
			if (!RegQueryValueEx(hk, L"PerceivedType", 0, 0, (LPBYTE)szInfo, &(nData = sizeof(szInfo)))) {
				if (!wcsncmp(szInfo, L"system", 6) || !wcsncmp(szInfo, L"video", 5) || !wcsncmp(szInfo, L"audio", 5) || !wcsncmp(szInfo, L"text", 4)) {
					RegCloseKey(hk); continue; // Видео и Аудио в PicView НЕ обрабатываем, чтобы не подраться с MMView
				}
			}
		}
		RegCloseKey(hk);

		lstrcat(szName, L"\\shellex\\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}");
		if (!RegOpenKeyEx(HKEY_CLASSES_ROOT, szName, 0, KEY_READ, &hk)) {
			lbCanHandle = TRUE;
			RegCloseKey(hk);
		} else {
			lstrcpy(szName, szExt);
			lstrcat(szName, L"\\shellex\\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}");
			if (!RegOpenKeyEx(HKEY_CLASSES_ROOT, szName, 0, KEY_READ, &hk)) {
				lbCanHandle = TRUE;
				RegCloseKey(hk);
			}
		}

		if (!lbCanHandle)
			continue;

		//if (!lbImageType) {
		//	lstrcat(szExt, L"\\PersistentHandler");
		//	if (RegOpenKeyEx(HKEY_CLASSES_ROOT, szExt, 0, KEY_READ, &hk))
		//		continue; // Отсутствует "PersistentHandler"
		//	RegCloseKey(hk);
		//	szExt[nLen] = 0;
		//}


		if (nMaxLen >= (nAllLen + nLen + 1)) {
			//szExt[0] = L','; szExt[nLen] = L',';
			//CharUpperBuff(szExt, nLen);
			//if (wcsstr(gsExtensions, szExt))
			//	continue; // уже добавлено

			szExt[nLen] = 0;
			lstrcpy(gsExtensions+nAllLen, szExt+1);
			nAllLen += nLen - 1;
			gsExtensions[nAllLen++] = L',';
			gsExtensions[nAllLen] = 0;
		}
	}
	
	FREE(pszData);

	gsExtensions[nAllLen-1] = 0;
	
	ip.SortExtensions(gsExtensions);

	pFormats->pActive = gsExtensions;
}

void __stdcall pvdGetFormats2(void *pContext, pvdFormats2* pFormats)
{
	_ASSERTE(pFormats->cbSize >= sizeof(pvdFormats2));
	
	/* вроде бы наличие этого позволяет смотреть Thumbs
[HKEY_CLASSES_ROOT\.jpg]
"Content Type"="image/jpeg"
[HKEY_CLASSES_ROOT\.jpg\PersistentHandler]
@="{098f2470-bae0-11cd-b579-08002b30bfeb}"
	*/

	//GetFormatsByPersistentHandler(pContext, pFormats);
	GetFormatsByIExtractImage(pContext, pFormats);
	pFormats->pInactive = pFormats->pForbidden = L"";
}

void __stdcall pvdExit2(void *pContext)
{
	CoUninitialize();
	if (gsExtensions) { FREE(gsExtensions); gsExtensions = NULL; }
}

void __stdcall pvdPluginInfo2(pvdInfoPlugin2 *pPluginInfo)
{
	_ASSERTE(pPluginInfo->cbSize >= sizeof(pvdInfoPlugin2));
	pPluginInfo->Priority = 1;
	pPluginInfo->pName = L"ShellThumb";
	pPluginInfo->pVersion = GetVersion(pPluginInfo->hModule);
	pPluginInfo->pComments = L"Copyright © 2009 Maximus5";
	pPluginInfo->Flags = PVD_IP_DECODE|PVD_IP_FOLDER;
}


BOOL __stdcall pvdFileOpen2(void *pContext, const wchar_t *pFileName, INT64 lFileSize, const BYTE *pBuf, UINT32 lBuf, pvdInfoImage2 *pImageInfo)
{
	_ASSERTE(pImageInfo->cbSize >= sizeof(pvdInfoImage2));

	gbCancelAll = FALSE;
	pImageInfo->nErrNumber = 0;

	// Пока UNC не обрабатываем
	if (pFileName[0] == L'\\' && pFileName[1] == L'\\' && (pFileName[2] == L'.' || pFileName[2] == L'?') && pFileName[3] == L'\\') {
		pFileName += 4;
		if (pFileName[0] == L'U' && pFileName[1] == L'N' && pFileName[2] == L'C' && pFileName[3] == L'\\') {
			pImageInfo->nErrNumber = PSE_UNC_NOT_SUPPORTED;
			return FALSE;
		}
	}
	
	int nLen = lstrlen(pFileName);
	wchar_t *pszSourceFile = /*_wcsdup(pFileName)*/ (wchar_t*)CALLOC((nLen+1)*2), *pszSlash = NULL;
	if (!pszSourceFile) {
		_ASSERTE(pszSourceFile!=NULL);
		pImageInfo->nErrNumber = PSE_NOT_ENOUGH_MEMORY;
		return FALSE;
	}
	lstrcpy(pszSourceFile, pFileName);
	pszSlash = wcsrchr(pszSourceFile, '\\');
	if (!pszSlash || ((pszSlash - pszSourceFile) < 2)) {
		pImageInfo->nErrNumber = PSE_INVALID_FILE_NAME;
		FREE(pszSourceFile);
		return FALSE;
	}
	
	
    IShellFolder *pFolder=NULL, *pFile=NULL;
    IExtractImage *pEI=NULL;
    LPITEMIDLIST pIdl = NULL;
    wchar_t wchPrev = L'\\';
    ULONG nEaten = 0;
    DWORD dwAttr = 0;
    
    ghLastWin32Error = S_OK;

    
    if (SUCCEEDED(ghLastWin32Error)) {
    	ghLastWin32Error = SHGetDesktopFolder(&pFolder);
	}

	TODO("Если pFileName папка - наверное можно просто ParseDisplayName, без BindToObject");

	if (SUCCEEDED(ghLastWin32Error)) {
		if (*(pszSlash-1) == L':') pszSlash++; // для корня диска нужно слэш оставить
	    wchPrev = *pszSlash; *pszSlash = 0;
	    
	    ghLastWin32Error = pFolder->ParseDisplayName(NULL, NULL, pszSourceFile, &nEaten, &pIdl, &dwAttr);
	    
		*pszSlash = wchPrev;
	}

	if (SUCCEEDED(ghLastWin32Error)) {
	    ghLastWin32Error = pFolder->BindToObject(pIdl, NULL, IID_IShellFolder, (void**)&pFile);
	    SafeRelease(pFolder);
	    if (pIdl) { CoTaskMemFree(pIdl); pIdl = NULL; }
    }

	if (SUCCEEDED(ghLastWin32Error)) {
	    if (wchPrev=='\\') pszSlash ++;
	    ghLastWin32Error = pFile->ParseDisplayName(NULL, NULL, pszSlash, &nEaten, &pIdl, &dwAttr);
	}	    
	    
	if (SUCCEEDED(ghLastWin32Error)) {
	    ghLastWin32Error = pFile->GetUIObjectOf(NULL, 1, (LPCITEMIDLIST*)&pIdl, IID_IExtractImage, NULL, (void**)&pEI);
	    // Если возвращает "Файл не найден" - значит файл не содержит превьюшки!
	    if (pIdl) { CoTaskMemFree(pIdl); pIdl = NULL; }
	}

	if (SUCCEEDED(ghLastWin32Error)) {
		// Пытаемся дернуть картинку сразу. Большое разрешение все-равно получить
		// не удастся, а иногда Shell дает IID_IExtractImage, но отказывается отдать Bitmap.

		BOOL lbRc = FALSE;
		WCHAR wsPathBuffer[512];
		SIZE size;
		DWORD dwPrior = 0, dwFlags = 0;
		DWORD nBitDepth = 32;
		HBITMAP hBmp = NULL, hOldBmp = NULL;


		wsPathBuffer[0] = 0;
		size.cx = DEFAULT_RENDER_SIZE;
		size.cy = DEFAULT_RENDER_SIZE;
		// Смысла в очень большом разрешении все-равно нет. Рендерится обычно только thumbnail (160x160).

		dwFlags = IEIFLAG_SCREEN|IEIFLAG_QUALITY; // IEIFLAG_ASPECT
		ghLastWin32Error = pEI->GetLocation(wsPathBuffer, 512, &dwPrior, &size, nBitDepth, &dwFlags);
		// Ошибка 0x8000000a (E_PENDING) теоретически может возвращаться, если pEI запустил извлечение превьюшки
		// в Background thread. И теоретически, он должен поддерживать интерфейс IID_IRunnableTask для его 
		// остановки и проверки статуса.
		// Эту ошибку могут возвращать Adobe, SolidEdge (jt), может еще кто...

		// На путь (wsPathBuffer) ориентироваться нельзя (в SE его нет). Вообще непонятно зачем он нужен...
		if (ghLastWin32Error==E_PENDING) {
			IRunnableTask* pRun = NULL;
			HRESULT hr = S_OK;
			ULONG lRunState = 0;
			hr = pEI->QueryInterface(IID_IRunnableTask, (void**)&pRun);
			// А вот не экспортит SE этот интерфейс
			if (SUCCEEDED(hr) && pRun) {
				while (!gbCancelAll) {
					lRunState = pRun->IsRunning();
					if (lRunState == IRTIR_TASK_FINISHED)
						break;
					Sleep(10);
				}
				if (gbCancelAll)
					pRun->Kill(0);

				pRun->Release();
				pRun = NULL;
			}
			ghLastWin32Error = S_OK;
		}
	}
    
    
    SafeRelease(pFolder);
    SafeRelease(pFile);
	if (pIdl) { CoTaskMemFree(pIdl); pIdl = NULL; }
	FREE(pszSourceFile); pszSourceFile = NULL;
	
	if (SUCCEEDED(ghLastWin32Error)) {
		pImageInfo->pImageContext = pEI;
		pImageInfo->nPages = 1;
		pImageInfo->Flags = 0;
		pImageInfo->pFormatName = pImageInfo->pCompression = pImageInfo->pComments = NULL;

		return TRUE;

	}

    SafeRelease(pEI);
	
	if (!pImageInfo->nErrNumber)
		pImageInfo->nErrNumber = PSE_WIN32_ERROR;
	return FALSE;
}

BOOL __stdcall pvdPageInfo2(void *pContext, void *pImageContext, pvdInfoPage2 *pPageInfo)
{
	if (!pImageContext) {
		pPageInfo->nErrNumber = PSE_INVALID_CONTEXT;
		return FALSE;
	}
	_ASSERTE(pPageInfo->cbSize >= sizeof(pvdInfoPage2));
	
	pPageInfo->lWidth = pPageInfo->lHeight = DEFAULT_RENDER_SIZE;
	pPageInfo->nBPP = 32;
	return TRUE;
}

#ifdef _DEBUG
	#define SAFEDELETEDC(d) if (d) { DeleteDC(d); d = NULL; }
	#define SAFERELEASEDC(w,d) if (d) { ReleaseDC(w,d); d = NULL; }
	#define SAFEDELETEOBJECT(o) if (o) { DeleteObject(o); o = NULL; }
#else
	#define SAFEDELETEDC(d) if (d) DeleteDC(d);
	#define SAFERELEASEDC(w,d) if (d) ReleaseDC(w,d);
	#define SAFEDELETEOBJECT(o) if (o) DeleteObject(o);
#endif

BOOL __stdcall pvdPageDecode2(void *pContext, void *pImageContext, pvdInfoDecode2 *pDecodeInfo, 
							  pvdDecodeCallback2 DecodeCallback, void *pDecodeCallbackContext)
{
	if (!pImageContext) {
		pDecodeInfo->nErrNumber = PSE_INVALID_CONTEXT;
		return FALSE;
	}

	_ASSERTE(pDecodeInfo->cbSize >= sizeof(pvdInfoDecode2));
	
	IExtractImage *pEI = (IExtractImage*)pImageContext;
	ghLastWin32Error = S_OK;

	BOOL lbRc = FALSE;
	//WCHAR wsPathBuffer[512];
	SIZE size;
	//DWORD dwPrior = 0, dwFlags = 0;
	DWORD nBitDepth = 32;
	HBITMAP hBmp = NULL, hOldBmp = NULL;
	
	// -- GetLocation уже выполнен в pvdOpen

	//wsPathBuffer[0] = 0;
	//size.cx = DEFAULT_RENDER_SIZE;
	//size.cy = DEFAULT_RENDER_SIZE;
	//// Смысла в очень большом разрешении все-равно нет. Рендерится обычно только thumbnail (160x160).
	////if (pDecodeInfo->lWidth && pDecodeInfo->lHeight) {
	////	size.cx = pDecodeInfo->lWidth;
	////	size.cy = pDecodeInfo->lHeight;
	////} else {
	////	size.cx = DEFAULT_RENDER_SIZE;
	////	size.cy = DEFAULT_RENDER_SIZE;
	////	HMONITOR hMon = MonitorFromWindow(GetConsoleWindow(), MONITOR_DEFAULTTOPRIMARY);
	////	MONITORINFO mi = {sizeof(MONITORINFO)};
	////	if (GetMonitorInfo(hMon, &mi)
	////		&& (mi.rcWork.right - mi.rcWork.left)>0 
	////		&& (mi.rcWork.bottom - mi.rcWork.top)>0)
	////	{
	////		size.cx = mi.rcWork.right - mi.rcWork.left;
	////		size.cy = mi.rcWork.bottom - mi.rcWork.top;
	////	}
	////}
 //   dwFlags = IEIFLAG_SCREEN|IEIFLAG_QUALITY; // IEIFLAG_ASPECT
 //   ghLastWin32Error = pEI->GetLocation(wsPathBuffer, 512, &dwPrior, &size, nBitDepth, &dwFlags);
	//// Adobe. Возвращает ошибку 0x8000000a - The data necessary to complete this operation is not yet available.
	//if (ghLastWin32Error==0x8000000a && wsPathBuffer[0]/* если путь все-таки вернули, считаем что ОК */) ghLastWin32Error = S_OK;

	//if (SUCCEEDED(ghLastWin32Error))
	//{
	ghLastWin32Error = pEI->Extract(&hBmp);
	if (!hBmp) ghLastWin32Error = (HRESULT)-1;
    //}

	if (SUCCEEDED(ghLastWin32Error) && hBmp) {
		BITMAPCOREHEADER *bch = (BITMAPCOREHEADER*)GlobalLock((HGLOBAL)hBmp);
		if (bch) GlobalUnlock((HGLOBAL)hBmp);

		
		HDC hScreenDC = GetDC(NULL);
		_ASSERTE(hScreenDC);
		_ASSERTE(pDecodeInfo->pImage == NULL);
		pDecodeInfo->pImage = NULL;

		HDC hCompDc2 = CreateCompatibleDC(hScreenDC);
		HBITMAP hOld2 = (HBITMAP)SelectObject(hCompDc2, hBmp);
		
		BOOL lbCanGetBits = FALSE, lbHasAlpha = FALSE, lbHasNoAlpha = FALSE;
		BITMAPINFO* pbi = (BITMAPINFO*)CALLOC(sizeof(BITMAPINFO)+255*sizeof(RGBQUAD));
		pbi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		int iDibRc = GetDIBits(hCompDc2, hBmp, 0, 0, NULL, pbi, DIB_RGB_COLORS);
		if (iDibRc && pbi->bmiHeader.biWidth && pbi->bmiHeader.biHeight) {
			size.cx = pbi->bmiHeader.biWidth;
			size.cy = pbi->bmiHeader.biHeight;
			lbCanGetBits = (pbi->bmiHeader.biBitCount == 32);
			_ASSERTE(nBitDepth==32);
		}

		pDecodeInfo->lWidth = pDecodeInfo->lSrcWidth = size.cx;
		pDecodeInfo->lHeight = pDecodeInfo->lSrcHeight = size.cy;
		pDecodeInfo->nBPP = nBitDepth;
		
		// ПОКА с выравниванием не заморачиваемся, т.к. размер фиксирован 400x400
		pDecodeInfo->lImagePitch = pDecodeInfo->lWidth * 4;
		pDecodeInfo->pImage = (LPBYTE)CALLOC(pDecodeInfo->lImagePitch * pDecodeInfo->lHeight);
		if (!pDecodeInfo->pImage) {
			pDecodeInfo->nErrNumber = PSE_NOT_ENOUGH_MEMORY;
			
		} else {
		
			if (lbCanGetBits) {
				// Чтобы вернуть прозрачность - сначала пробуем напролом получить биты
				iDibRc = GetDIBits(hCompDc2, hBmp, 0, size.cy, pDecodeInfo->pImage, pbi, DIB_RGB_COLORS);
				lbRc = iDibRc!=0;
				if (!lbRc) {
					pDecodeInfo->nErrNumber = PSE_GETDIBITS_FAILED;
				} else if (gbAutoDetectAlphaChannel) {
					//PRAGMA_ERROR("нужно просматривать все полученные биты на предмет наличия в них альфа канала и ставить PVD_IDF_ALPHA");
					LPBYTE pSrc = pDecodeInfo->pImage;
					DWORD lAbsSrcPitch = pDecodeInfo->lImagePitch, A;
					for (DWORD y = size.cy; y-- && !(lbHasAlpha && lbHasNoAlpha); pSrc += lAbsSrcPitch) {
						for (int x = 0; x<size.cx; x++) {
							A = (((DWORD*)pSrc)[x]) & 0xFF000000;
							if (A) lbHasAlpha = TRUE; else lbHasNoAlpha = TRUE;
						}
					}
				}
			} else {
				HDC hCompDc1 = CreateCompatibleDC(hScreenDC);

				BITMAPINFOHEADER bmi = {sizeof(BITMAPINFOHEADER)};
				bmi.biWidth = size.cx;
				bmi.biHeight = size.cy;
				bmi.biPlanes = 1;
				bmi.biBitCount = 32;
				bmi.biCompression = BI_RGB;

				LPBYTE pBits = NULL;
				HBITMAP hDIB = CreateDIBSection(hScreenDC, (BITMAPINFO*)&bmi, DIB_RGB_COLORS, (void**)&pBits, NULL, 0);
				_ASSERTE(hDIB);

				HBITMAP hOld1 = (HBITMAP)SelectObject(hCompDc1, hDIB);

				//if (pDecodeInfo->cbSize >= sizeof(pvdInfoDecode2)) {
				//	HBRUSH hBackBr = CreateSolidBrush(pDecodeInfo->nBackgroundColor);
				//	RECT rc = {0,0,size.cx,size.cy};
				//	FillRect(hCompDc1, &rc, hBackBr);
				//	DeleteObject(hBackBr);
				//}

				// Не прокатывает. PNG и GIF "прозрачными" не рендерятся. Получается белый фон. Ну и ладно
				//BLENDFUNCTION bf = {AC_SRC_OVER,0,255,AC_SRC_ALPHA};
				//lbRc = AlphaBlend(hCompDc1, 0,0,size.cx,size.cy, hCompDc2, 0,0, size.cx,size.cy, bf);
				//if (!lbRc)
				lbRc = BitBlt(hCompDc1, 0,0,size.cx,size.cy, hCompDc2, 0,0, SRCCOPY);
		
				if (!lbRc) {
					pDecodeInfo->nErrNumber = PSE_BITBLT_FAILED;
				} else {
					memmove(pDecodeInfo->pImage, pBits, pDecodeInfo->lImagePitch * pDecodeInfo->lHeight);
				}

				if (hCompDc1 && hOld1)
					SelectObject(hCompDc1, hOld1);
				SAFEDELETEOBJECT(hDIB);
				SAFEDELETEDC(hCompDc1);
			}
			
			if (lbRc) {
				// В DIB строки идут снизу вверх
				pDecodeInfo->lImagePitch = - pDecodeInfo->lImagePitch;
				
				pDecodeInfo->ColorModel = (lbHasAlpha && lbHasNoAlpha) ? PVD_CM_BGRA : PVD_CM_BGR;
				pDecodeInfo->Flags = (lbHasAlpha && lbHasNoAlpha) ? PVD_IDF_ALPHA : 0;
			}
		}

		if (hCompDc2 && hOld2)
			SelectObject(hCompDc2, hOld2);
		SAFEDELETEDC(hCompDc2);
		SAFEDELETEOBJECT(hBmp);
		SAFERELEASEDC(NULL, hScreenDC);
	}

	if (!lbRc)
		pDecodeInfo->nErrNumber = PSE_WIN32_ERROR;

	return lbRc;
}

void __stdcall pvdPageFree2(void *pContext, void *pImageContext, pvdInfoDecode2 *pDecodeInfo)
{
	_ASSERTE(pDecodeInfo->cbSize >= sizeof(pvdInfoDecode2));
	if (pDecodeInfo->pImage)
		FREE(pDecodeInfo->pImage);
}

void __stdcall pvdFileClose2(void *pContext, void *pImageContext)
{
	if (pImageContext) {
		IExtractImage *pEI = (IExtractImage*)pImageContext;
		SafeRelease(pEI);
	}
}

// Затребована остановка всех операций контекста (декодирование, преобразование, и пр.)
void __stdcall pvdCancel2(void *pContext)
{
	gbCancelAll = TRUE;
}
