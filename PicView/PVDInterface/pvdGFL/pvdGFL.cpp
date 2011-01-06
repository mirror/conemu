#include <windows.h>
#include <vector>
#include "../PictureViewPlugin.h"
#include "../../headers/libgfl.h"
#include "../PVD2Helper.h"

extern HMODULE ghModule;

TODO("Прогресс декодирования GFL_PROGRESS_CALLBACK и отмены GFL_WANTCANCEL_CALLBACK");

#define PGE_INVALID_FRAME        0x1001
#define PGE_DLL_NOT_FOUND        0x80001001
#define PGE_EXCEPTION            0x80001002
#define PGE_FILE_NOT_FOUND       0x80001003
#define PGE_NOT_ENOUGH_MEMORY    0x80001004
#define PGE_INVALID_CONTEXT      0x80001005
#define PGE_UNKNOWN_COLORSPACE   0x80001007
#define PGE_WIN32_ERROR          0x80001008
#define PGE_NO_UNICODE_FUNCTION  0x80001009
#define PGE_OLD_PICVIEW          0x8000100A
#define PGE_GFL_ERROR__BASE             0x80002000
#define PGE_GFL_ERROR_FILE_OPEN         (PGE_GFL_ERROR__BASE+1)
#define PGE_GFL_ERROR_FILE_READ         (PGE_GFL_ERROR__BASE+2)
#define PGE_GFL_ERROR_FILE_CREATE       (PGE_GFL_ERROR__BASE+3)
#define PGE_GFL_ERROR_FILE_WRITE        (PGE_GFL_ERROR__BASE+4)
#define PGE_GFL_ERROR_NO_MEMORY         (PGE_GFL_ERROR__BASE+5)
#define PGE_GFL_ERROR_UNKNOWN_FORMAT    (PGE_GFL_ERROR__BASE+6)
#define PGE_GFL_ERROR_BAD_BITMAP        (PGE_GFL_ERROR__BASE+7)
#define PGE_GFL_ERROR_BAD_FORMAT_INDEX  (PGE_GFL_ERROR__BASE+10)
#define PGE_GFL_ERROR_BAD_PARAMETERS    (PGE_GFL_ERROR__BASE+50)
#define PGE_GFL_UNKNOWN_ERROR           (PGE_GFL_ERROR__BASE+255)

BOOL __stdcall pvdTranslateError2(DWORD nErrNumber, wchar_t *pszErrInfo, int nBufLen)
{
	if (!pszErrInfo || nBufLen<=0)
		return FALSE;

	switch (nErrNumber)
	{
	case PGE_INVALID_FRAME:
		lstrcpynW(pszErrInfo, L"Invalid frame number", nBufLen); break;
	case PGE_DLL_NOT_FOUND:
		lstrcpynW(pszErrInfo, L"libgfl*.dll not found", nBufLen); break;
	case PGE_EXCEPTION:
		lstrcpynW(pszErrInfo, L"Exception occurred", nBufLen); break;
	case PGE_FILE_NOT_FOUND:
		lstrcpynW(pszErrInfo, L"File not found", nBufLen); break;
	case PGE_INVALID_CONTEXT:
		lstrcpynW(pszErrInfo, L"Invalid context", nBufLen); break;
	case PGE_UNKNOWN_COLORSPACE:
		lstrcpynW(pszErrInfo, L"Unknown colorspace", nBufLen); break;
	case PGE_WIN32_ERROR:
		lstrcpynW(pszErrInfo, L"Win32 error", nBufLen); break;
	case PGE_NO_UNICODE_FUNCTION:
		lstrcpynW(pszErrInfo, L"GFL Unicode functions not found", nBufLen); break;
	case PGE_GFL_ERROR_FILE_OPEN:
		lstrcpynW(pszErrInfo, L"GFL_ERROR_FILE_OPEN", nBufLen); break;
	case PGE_GFL_ERROR_FILE_READ:
		lstrcpynW(pszErrInfo, L"GFL_ERROR_FILE_READ", nBufLen); break;
	case PGE_GFL_ERROR_FILE_CREATE:
		lstrcpynW(pszErrInfo, L"GFL_ERROR_FILE_CREATE", nBufLen); break;
	case PGE_GFL_ERROR_FILE_WRITE:
		lstrcpynW(pszErrInfo, L"GFL_ERROR_FILE_WRITE", nBufLen); break;
	case PGE_GFL_ERROR_NO_MEMORY:
		lstrcpynW(pszErrInfo, L"GFL_ERROR_NO_MEMORY", nBufLen); break;
	case PGE_GFL_ERROR_UNKNOWN_FORMAT:
		lstrcpynW(pszErrInfo, L"GFL_ERROR_UNKNOWN_FORMAT", nBufLen); break;
	case PGE_GFL_ERROR_BAD_BITMAP:
		lstrcpynW(pszErrInfo, L"GFL_ERROR_BAD_BITMAP", nBufLen); break;
	case PGE_GFL_ERROR_BAD_FORMAT_INDEX:
		lstrcpynW(pszErrInfo, L"GFL_ERROR_BAD_FORMAT_INDEX", nBufLen); break;
	case PGE_GFL_ERROR_BAD_PARAMETERS:
		lstrcpynW(pszErrInfo, L"GFL_ERROR_BAD_PARAMETERS", nBufLen); break;
	case PGE_GFL_UNKNOWN_ERROR:
		lstrcpynW(pszErrInfo, L"GFL_UNKNOWN_ERROR", nBufLen); break;
	case PGE_OLD_PICVIEW:
		lstrcpyn(pszErrInfo, L"Old PicView version, exiting", nBufLen); break;
	default:
		return FALSE;
	}
	return TRUE;
}

#define DllGetFunction(hModule, FunctionName) FunctionName = (FunctionName##_t)GetProcAddress(hModule, #FunctionName)
#define CALLOC(n) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, n)
#define FREE(p) HeapFree(GetProcessHeap(), 0, p)
#define sizeofarray(array) (sizeof(array)/sizeof(*array))
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

#define calloc PRAGMA_ERROR
#define malloc PRAGMA_ERROR
#define free PRAGMA_ERROR


struct GFLDecoder
{
	HMODULE hGFL;
	HRESULT nErrNumber, nLastError;
	const wchar_t* pszPluginKey;
	wchar_t szAnimatedFormats[512];
	wchar_t szNoAlphaFormats[512];
	pvdInitPlugin2 ip;
	wchar_t* pszSupported;
	bool bCMYK2RGB;

	typedef GFL_ERROR (GFLAPI *gflLibraryInit_t)(void);
	typedef void (GFLAPI *gflLibraryExit_t)(void);
	typedef void (GFLAPI *gflEnableLZW_t)(GFL_BOOL);
	typedef void (GFLAPI *gflGetDefaultLoadParams_t)(GFL_LOAD_PARAMS *params);
	typedef GFL_ERROR (GFLAPI *gflLoadBitmapFromMemory_t)( const GFL_UINT8* data, GFL_UINT32 data_length, GFL_BITMAP** bitmap, const GFL_LOAD_PARAMS* params, GFL_FILE_INFORMATION* info);
	typedef GFL_ERROR (GFLAPI *gflLoadBitmap_t)( const char* filename, GFL_BITMAP** bitmap, const GFL_LOAD_PARAMS* params, GFL_FILE_INFORMATION* info ); 
	typedef GFL_ERROR (GFLAPI *gflLoadBitmapW_t)( const wchar_t* filename, GFL_BITMAP** bitmap, const GFL_LOAD_PARAMS* params, GFL_FILE_INFORMATION* info ); 
	typedef void (GFLAPI *gflFreeBitmap_t)(GFL_BITMAP *bitmap);
	typedef GFL_ERROR (GFLAPI *gflGetFileInformationFromMemory_t)( const GFL_UINT8* data, GFL_UINT32 data_length, GFL_INT32 index, GFL_FILE_INFORMATION* info);
	typedef GFL_ERROR (GFLAPI *gflGetFileInformation_t)( const char* filename, GFL_INT32 index, GFL_FILE_INFORMATION *info );
	typedef GFL_ERROR (GFLAPI *gflGetFileInformationW_t)( const wchar_t* filename, GFL_INT32 index, GFL_FILE_INFORMATION *info );
	typedef void (GFLAPI *gflFreeFileInformation_t)(GFL_FILE_INFORMATION *info); 
	typedef void (GFLAPI *gflSetPluginsPathname_t)(const char *pPath);
	typedef void (GFLAPI *gflSetPluginsPathnameW_t)(const wchar_t *pPath);
	typedef GFL_ERROR (GFLAPI *gflChangeColorDepth_t)(GFL_BITMAP * src, GFL_BITMAP ** dst, GFL_MODE mode, GFL_MODE_PARAMS params);
	typedef GFL_INT32 (GFLAPI *gflGetNumberOfFormat_t)(void);
	typedef GFL_ERROR (GFLAPI *gflGetFormatInformationByIndex_t)(GFL_INT32 index, GFL_FORMAT_INFORMATION* informations);

	gflLibraryInit_t gflLibraryInit;
	gflLibraryExit_t gflLibraryExit;
	gflEnableLZW_t gflEnableLZW;
	gflGetDefaultLoadParams_t gflGetDefaultLoadParams;
	gflLoadBitmapFromMemory_t gflLoadBitmapFromMemory;
	gflLoadBitmap_t gflLoadBitmap;
	gflLoadBitmapW_t gflLoadBitmapW;
	gflFreeBitmap_t gflFreeBitmap;
	gflGetFileInformationFromMemory_t gflGetFileInformationFromMemory;
	gflGetFileInformation_t gflGetFileInformation;
	gflGetFileInformationW_t gflGetFileInformationW;
	gflFreeFileInformation_t gflFreeFileInformation;
	gflSetPluginsPathname_t gflSetPluginsPathname;
	gflSetPluginsPathnameW_t gflSetPluginsPathnameW;
	gflChangeColorDepth_t gflChangeColorDepth;
	gflGetNumberOfFormat_t gflGetNumberOfFormat;
	gflGetFormatInformationByIndex_t gflGetFormatInformationByIndex;

	GFLDecoder() { 
		hGFL = NULL; memset(&ip, 0, sizeof(ip));
		gflLibraryInit = NULL; gflLibraryExit = NULL; gflEnableLZW = NULL;
		gflGetDefaultLoadParams = NULL; gflFreeBitmap = NULL;
		gflFreeFileInformation = NULL;
		gflSetPluginsPathname = NULL; gflSetPluginsPathnameW = NULL;
		szAnimatedFormats[0] = 0; pszSupported = NULL; bCMYK2RGB = false;
		lstrcpy(szNoAlphaFormats, L"PSD");
	};

	void ReloadConfig(PVDSettings* pSet = NULL)
	{
		if (!pszPluginKey || !*pszPluginKey)
			return;

		bool lbDestroy = false;
		if (!pSet) {
			pSet = new PVDSettings(pszPluginKey);
			if (!pSet) return;
			lbDestroy = true;
		}

		pSet->GetStrParam(L"AnimatedFormats", L"string;List of formats with animation (comma separated)",
			L"", szAnimatedFormats, sizeofarray(szAnimatedFormats));
		CharUpperBuff(szAnimatedFormats, lstrlen(szAnimatedFormats));
		pSet->GetStrParam(L"NoAlphaFormats", L"string;Ignore alpha channel in formats (comma separated)",
			L"PSD", szNoAlphaFormats, sizeofarray(szNoAlphaFormats));
		CharUpperBuff(szNoAlphaFormats, lstrlen(szNoAlphaFormats));
		
		bool bDefault = false;
		pSet->GetParam(L"CMYK2RGB", L"bool;Convert CMYK to RGB internally", REG_BINARY, &bDefault, &bCMYK2RGB, sizeof(bCMYK2RGB));

		if (lbDestroy)
			delete pSet;
	}
	
	bool Init(pvdInitPlugin2* pInit)
	{
		bool result = false;
		nErrNumber = 0;
		wchar_t szGflPath[MAX_PATH*3], szGflName[MAX_PATH*2]; szGflPath[0] = 0;
		wchar_t szGflPluginsFolder[MAX_PATH*2]; szGflPluginsFolder[0] = 0;

		pszPluginKey = pInit->pRegKey;
		memmove(&ip, pInit, min(sizeof(ip),pInit->cbSize));

		PVDSettings::GetPluginFolder(szGflPluginsFolder, sizeofarray(szGflPluginsFolder));

		if (pszPluginKey && *pszPluginKey) {
			PVDSettings set(pszPluginKey);

			if (!hGFL) {
				set.GetStrParam(L"GflPathFileName", L"string;File or full path of GFL library",
					L"libgfl311.dll", szGflName, sizeofarray(szGflName));

				PVDSettings::FindFile(szGflName, szGflPath, sizeofarray(szGflPath));
				if (!(hGFL = LoadLibraryW(szGflPath)) && wcscmp(szGflPath, szGflName))
					hGFL = LoadLibraryW(szGflName);
			}

			
			set.GetStrParam(L"GflPluginsFolder", L"string;GFL subplugins folder\\", 
				szGflPluginsFolder, szGflPluginsFolder, sizeofarray(szGflPluginsFolder));

			ReloadConfig(&set);

			//HKEY hKey = 0; DWORD dwDisp = 0;
			//if (!RegCreateKeyExW(HKEY_CURRENT_USER, pszPluginKey, 0, 0, 0, KEY_ALL_ACCESS, 0, &hKey, &dwDisp)) {
			//	DWORD nSize = sizeof(szGflPath);
			//	if (!RegQueryValueExW(hKey, L"GflPathFileName", 0, 0, (LPBYTE)szGflPath, &nSize)) {
			//		lstrcpyW(szGflPath, L"libgfl311.dll"); nSize = (lstrlenW(szGflPath)+1)*sizeof(wchar_t);
			//		RegSetValueExW(hKey, L"GflPathFileName", 0, REG_SZ, (LPBYTE)&szGflPath, nSize);
			//	}
			//	nSize = sizeof(szGflPluginsFolder);
			//	if (!RegQueryValueExW(hKey, L"GflPluginsFolder", 0, 0, (LPBYTE)szGflPluginsFolder, &nSize)) {
			//		szGflPluginsFolder[0] = 0;
			//		if (!hGFL && ghModule)
			//		{
			//			nSize = sizeof(szGflPluginsFolder)/sizeof(szGflPluginsFolder[0]);
			//			DWORD nLen = GetModuleFileNameW(ghModule, szGflPluginsFolder, nSize);
			//			if (nLen && nLen < nSize) {
			//				wchar_t* pszSlash = wcsrchr(szGflPluginsFolder, L'\\');
			//				if (pszSlash) pszSlash[1] = 0; else szGflPluginsFolder[0] = 0;
			//			}
			//		}
			//		nSize = (lstrlenW(szGflPluginsFolder)+1)*sizeof(wchar_t);
			//		RegSetValueExW(hKey, L"GflPluginsFolder", 0, REG_SZ, (LPBYTE)&szGflPluginsFolder, nSize);
			//	}
			//	RegCloseKey(hKey);
			//}
		}

		//if (szGflPath[0] && wcschr(szGflPath, L'\\')) {
		//	// Если в реестре указан путь к libgfl*.dll
		//	hGFL = LoadLibraryW(szGflPath);
		//}
		// Если на предыдущем шаге загрузить не удалось - пробуем в текущей директории
		if (!hGFL && ghModule)
		{
			wchar_t FullPath[MAX_PATH*3+15]; FullPath[0] = 0;
			if (PVDSettings::GetPluginFolder(FullPath, (MAX_PATH*2))) {
				wchar_t* pszSlash = FullPath + lstrlen(FullPath);
				lstrcpy(pszSlash, L"libgfl*.dll");
				WIN32_FIND_DATA fnd = {0};
				HANDLE hFind = FindFirstFile(FullPath, &fnd);
				if (hFind != INVALID_HANDLE_VALUE) {
					do {
						if ((fnd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
							lstrcpy(pszSlash, fnd.cFileName);
							hGFL = LoadLibraryW(FullPath);
							if (hGFL)
								break;
						}
					} while (FindNextFile(hFind, &fnd));
					FindClose(hFind);
				}
			}
		}
		// Если на предыдущем шаге загрузить не удалось - пробуем просто по имени
		if (!hGFL)
			hGFL = LoadLibraryW(L"libgfl311.dll");

		if (!hGFL) {
			nErrNumber = PGE_DLL_NOT_FOUND;

		} else {
			DllGetFunction(hGFL, gflLibraryInit);
			DllGetFunction(hGFL, gflLibraryExit);
			DllGetFunction(hGFL, gflEnableLZW);
			DllGetFunction(hGFL, gflGetDefaultLoadParams);
			DllGetFunction(hGFL, gflLoadBitmapFromMemory);
			DllGetFunction(hGFL, gflLoadBitmap);
			DllGetFunction(hGFL, gflLoadBitmapW);
			DllGetFunction(hGFL, gflFreeBitmap);
			DllGetFunction(hGFL, gflGetFileInformationFromMemory);
			DllGetFunction(hGFL, gflGetFileInformation);
			DllGetFunction(hGFL, gflGetFileInformationW);
			DllGetFunction(hGFL, gflFreeFileInformation);
			DllGetFunction(hGFL, gflSetPluginsPathname);
			DllGetFunction(hGFL, gflSetPluginsPathnameW);
			DllGetFunction(hGFL, gflChangeColorDepth);
			DllGetFunction(hGFL, gflGetNumberOfFormat);
			DllGetFunction(hGFL, gflGetFormatInformationByIndex);

			if (gflLibraryInit && gflLibraryExit && gflGetDefaultLoadParams && gflLoadBitmapFromMemory 
				&& gflFreeBitmap && gflGetFileInformationFromMemory && gflFreeFileInformation)
			{
				try{
					GFL_ERROR status;

					if (gflSetPluginsPathnameW) {
						gflSetPluginsPathnameW(szGflPluginsFolder);
					} else if (gflSetPluginsPathname) { // т.к. выше gflSetPluginsPathnameX необходимым не указан - требуется проверка
						UINT nCP = AreFileApisANSI() ? CP_ACP : CP_OEMCP;
						char szPath[MAX_PATH];
						WideCharToMultiByte(nCP, 0, szGflPluginsFolder, -1, szPath, MAX_PATH, 0,0);
						gflSetPluginsPathname(szPath);
					}
					status = gflLibraryInit();
					if (status == 0) {
						result = TRUE;
					
						if (gflEnableLZW)
							gflEnableLZW(GFL_TRUE);
					}
				}catch(...){ nErrNumber = PGE_EXCEPTION; }
			}
			if (!result)
				FreeLibrary(hGFL);
		}
		if (result)
			pInit->pContext = this;
		return result;
	};
	
	const wchar_t* LoadSupported()
	{
		if (pszSupported) {
			FREE(pszSupported);
			pszSupported = NULL;
		}
			
		if (gflGetNumberOfFormat && gflGetFormatInformationByIndex)
		{
			GFL_INT32 n, nCount, nMaxLen, nLeft, nLen;
			nCount = gflGetNumberOfFormat();
			if (nCount > 0) {
				nMaxLen = 0;
				//nLeft = nMaxLen-1;
				
				//pszSupported = (wchar_t*)CALLOC(nMaxLen*2);
				wchar_t* pszCur = NULL;
				std::vector<wchar_t*> Formats;
				std::vector<wchar_t*>::iterator i, j, liMin;
				bool liCmp;
				wchar_t* pszFmt = NULL;
				
				for (n = 0; n < nCount; n++) {
					GFL_FORMAT_INFORMATION fmt = {0};
					if (gflGetFormatInformationByIndex(n, &fmt)) continue;
					//nLen = strlen(pszFmt);
					for (GFL_UINT32 j = 0; j < fmt.NumberOfExtension; j++) {
						nLen = lstrlenA(fmt.Extension[j])+1;
						pszCur = (wchar_t*)CALLOC(nLen*2);
						MultiByteToWideChar(CP_ACP, 0, fmt.Extension[j], nLen, pszCur, nLen);
						CharUpperBuffW(pszCur, nLen);
						liCmp = false;
						for (i=Formats.begin(); !liCmp && i!=Formats.end(); i++) {
							if (!lstrcmpW(*i, pszCur))
								liCmp = true;
						}
						if (liCmp) {
							FREE(pszCur);
						} else {
							Formats.push_back(pszCur);
							nMaxLen += nLen+1;
						}
					}
					//if (nLen >= MAX_PATH) continue; // слишком длинное расширение?
					//nMaxLen += nLen+1;
					//Formats.push_back(pszFmt);
				}
				
			    for (i=Formats.begin(); i!=Formats.end() && (i+1)!=Formats.end(); i++) {
			        liMin = i;
			        for (j=i+1; j!=Formats.end(); j++) {
			            if (lstrcmpW(*liMin, *j) > 0)
			                liMin = j;
			        }
			        if (liMin!=i) {
						pszFmt = *liMin;
						*liMin = *i;
						*i = pszFmt;
			        }
			    }
			    
			    pszSupported = (wchar_t*)CALLOC((nMaxLen+1)*2);
			    pszCur = pszSupported;
			    nLeft = nMaxLen;
			    for (i=Formats.begin(); i!=Formats.end(); i++) {
			    	pszFmt = *i;
			    	
					if (pszCur != pszSupported) {
						*(pszCur++) = L','; nLeft--;
					}

					nLen = lstrlen(pszFmt);
					_ASSERTE(nLeft>1);
					if (nLen > nLeft) break;
					lstrcpyW(pszCur, pszFmt);
					pszCur += nLen; nLeft -= nLen;
					FREE(pszFmt);
				}
			}
		}
		if (!pszSupported) {
			pszSupported = (wchar_t*)CALLOC(4*2);
			pszSupported[0] = L'?'; pszSupported[1] = L'?'; pszSupported[2] = L'?'; pszSupported[3] = 0;
		}
		return pszSupported;
	};

	void Close()
	{
		if (hGFL) {
			if (gflLibraryExit) {
				try {
					gflLibraryExit();
				}catch(...){}
			}
			FreeLibrary(hGFL);
			hGFL = NULL;
		}
		if (pszSupported) {
			FREE(pszSupported);
			pszSupported = NULL;
		}
	};
};


struct GFLImage
{
	GFLDecoder *gfl;
	LPBYTE pFileBuffer; DWORD nFileBufferSize; BOOL bFileBufferAllocated;
	GFL_FILE_INFORMATION *pFileInformation;
	GFL_LOAD_PARAMS *pLoadParams;
	GFL_BITMAP* pPageI; BOOL bHasAlpha;
	GFL_ERROR rcGFL;
	BOOL bFileMode; wchar_t *pwszFileName; char *pszFileName;
	HRESULT nErrNumber, nLastError;
	//
	UINT lWidth, lHeight, nBPP, nPages, lFrameTime, nActivePage;
	int nTransparentIdx; // только для изображений с палитрой
	bool Animation;
	wchar_t FormatName[0x80], Compression[0x80];
	
	GFLImage() {
		gfl = NULL; 
		pFileInformation = NULL;
		pLoadParams = NULL;
		pPageI = NULL;
		pFileBuffer = NULL; nFileBufferSize = 0; bFileBufferAllocated = FALSE;
		nErrNumber = 0; nLastError = 0;
		lWidth = 0; lHeight = 0; nBPP = 0; nPages = -1; lFrameTime = 0; 
		nActivePage = -1; Animation = false; nTransparentIdx = -1;
		FormatName[0] = 0; bFileMode = NULL;
		pwszFileName = NULL; pszFileName = NULL;
	}

	bool CheckFileInformation(LPCBYTE pBuffer, DWORD lBuffer)
	{
		bool result = false;
		if (pwszFileName) { FREE(pwszFileName); pwszFileName = NULL; }
		if (pszFileName) { FREE(pszFileName); pszFileName = NULL; }
		bFileMode = FALSE;

		if ((pBuffer!=NULL) && (lBuffer)) {
			rcGFL = gfl->gflGetFileInformationFromMemory( pBuffer, lBuffer, -1, pFileInformation ); 
			result = (rcGFL == 0);
			if (result) {
				// BUGGY GFL. Например для анимированных GIF тут всегда 1
				nPages = pFileInformation->NumberOfImages;

				lWidth = pFileInformation->Width;
				lHeight = pFileInformation->Height;
				nBPP = pFileInformation->BitsPerComponent * pFileInformation->ComponentsPerPixel;
				lFrameTime = 0;
				Animation = false;

				int n = MultiByteToWideChar(CP_OEMCP, 0, pFileInformation->FormatName, sizeofarray(pFileInformation->FormatName), FormatName, sizeofarray(FormatName)-1);
				FormatName[n > 0 ? n : 0] = 0;
				n = MultiByteToWideChar(CP_OEMCP, 0, pFileInformation->CompressionDescription, sizeofarray(pFileInformation->CompressionDescription), Compression, sizeofarray(Compression)-1);
				Compression[n > 0 ? n : 0] = 0;

				gfl->gflFreeFileInformation(pFileInformation);
			} else {
				nErrNumber = PGE_GFL_ERROR__BASE + rcGFL;
			}
		}
		return result;
	}

	bool CheckFileInformation(const wchar_t* asFileName)
	{
		bool result = false;
		if (pwszFileName) { FREE(pwszFileName); pwszFileName = NULL; }
		if (pszFileName) { FREE(pszFileName); pszFileName = NULL; }
		bFileMode = FALSE;

		if (gfl->gflGetFileInformationW && gfl->gflLoadBitmapW) {
			pwszFileName = (wchar_t*)CALLOC((lstrlen(asFileName)+1)*2);
			lstrcpy(pwszFileName, asFileName);
			bFileMode = TRUE;
			rcGFL = gfl->gflGetFileInformationW( asFileName, -1, pFileInformation ); 
		} else if (gfl->gflGetFileInformation && gfl->gflLoadBitmap) {
			if (asFileName[0]==L'\\' && asFileName[1]==L'\\' && asFileName[2]==L'?' && asFileName[3]==L'\\')
				asFileName += 4;
			int nLen = lstrlen(asFileName)+1;
			pszFileName = (char*)CALLOC(nLen);
			UINT nCP = AreFileApisANSI() ? CP_ACP : CP_OEMCP;
			WideCharToMultiByte(nCP, 0, asFileName, -1, pszFileName, nLen, 0,0);
			bFileMode = TRUE;
			rcGFL = gfl->gflGetFileInformation( pszFileName, -1, pFileInformation ); 
		} else {
			nErrNumber = PGE_NO_UNICODE_FUNCTION;
			return false;
		}

		result = (rcGFL == 0);
		if (result) {
			// BUGGY GFL. Например для анимированных GIF тут всегда 1
			nPages = pFileInformation->NumberOfImages;

			lWidth = pFileInformation->Width;
			lHeight = pFileInformation->Height;
			nBPP = pFileInformation->BitsPerComponent * pFileInformation->ComponentsPerPixel;
			lFrameTime = 0;
			Animation = false;

			int n = MultiByteToWideChar(CP_OEMCP, 0, pFileInformation->FormatName, sizeofarray(pFileInformation->FormatName), FormatName, sizeofarray(FormatName)-1);
			FormatName[n > 0 ? n : 0] = 0;
			n = MultiByteToWideChar(CP_OEMCP, 0, pFileInformation->CompressionDescription, sizeofarray(pFileInformation->CompressionDescription), Compression, sizeofarray(Compression)-1);
			Compression[n > 0 ? n : 0] = 0;

			gfl->gflFreeFileInformation(pFileInformation);
		} else {
			nErrNumber = PGE_GFL_ERROR__BASE + rcGFL;
		}

		return result;
	}

	bool Open(const wchar_t *pFileName, __int64 lFileSize, const BYTE *pBuffer, DWORD lBuffer)
	{
		_ASSERTE(gfl != NULL);

		bool result = false;
		nErrNumber = 0; nLastError = 0;

		nActivePage = -1; // Страницу нужно активировать
		nPages = -1; // Количество страниц еще не получено
		nTransparentIdx = -1;
		
		pFileInformation = (GFL_FILE_INFORMATION*)CALLOC(sizeof(GFL_FILE_INFORMATION)+100);
		pLoadParams = (GFL_LOAD_PARAMS*)CALLOC(sizeof(GFL_LOAD_PARAMS)+24);
		if (!pFileInformation || !pLoadParams) {
			Close();
			nErrNumber = PGE_NOT_ENOUGH_MEMORY;
			return false;
		}

		//
		if (!lFileSize || lFileSize == (__int64)lBuffer) {
			_ASSERTE(pBuffer && lBuffer);
			result = CheckFileInformation(pBuffer, lBuffer);
			if (result) {
				pFileBuffer = (LPBYTE)CALLOC(lBuffer);
				if (!pFileBuffer) {
					result = false; nErrNumber = PGE_NOT_ENOUGH_MEMORY;
				} else {
					nFileBufferSize = lBuffer; bFileBufferAllocated = true;
					memmove(pFileBuffer, pBuffer, lBuffer);
				}
			} else
				result = CheckFileInformation(pFileName);
		} else {
			_ASSERT(FALSE); // сюда мы попадать не должны - буфер всегда передается
			nFileBufferSize = (DWORD)lFileSize;
			if (nFileBufferSize != lFileSize) {
				result = false; nErrNumber = PGE_NOT_ENOUGH_MEMORY;
			} else {
				pFileBuffer = (LPBYTE)CALLOC(nFileBufferSize);
				if (!pFileBuffer) {
					result = false; nErrNumber = PGE_NOT_ENOUGH_MEMORY;
				} else {
					bFileBufferAllocated = true;
					HANDLE hFile = CreateFileW(pFileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
					if (hFile == INVALID_HANDLE_VALUE) {
						nErrNumber = PGE_WIN32_ERROR; nLastError = GetLastError();
					} else {
						DWORD nRead = 0;
						if (ReadFile(hFile, pFileBuffer, nFileBufferSize, &nRead, 0) && nFileBufferSize == nRead) {
							result = CheckFileInformation(pFileBuffer, nFileBufferSize);
						}
						CloseHandle(hFile);
					}
				}
			}
		}

		if (result) {
			//BUGGY GFL. Не может получить некоторые параметры, если не декодировать первый фрейм
			result = (pPageI = DecodePage(0)) != NULL;
		}
		if (!result) {
			Close();
		}
		return result;
	};
	bool SelectPage(UINT iPage)
	{
		if (iPage == nActivePage)
			return true;
		// nPages == -1 если картинка еще не декодировалась
		if (nPages!=(UINT)-1 && ((int)iPage < 0 || iPage >= nPages)) {
			nErrNumber = PGE_INVALID_FRAME;
			return false;
		}

		_ASSERTE(pLoadParams);
		return true;
	}

	GFL_BITMAP* DecodePage(UINT iPage)
	{
		bool result = false;
		GFL_BITMAP* GFLBitmap = NULL;
		bHasAlpha = FALSE;

		if (iPage == nActivePage && pPageI) {
			// фрейм уже был декодирован для получения параметров
			GFLBitmap = pPageI;
			pPageI = NULL; // более не требуется - отдали
			result = true;
		} else {
			// Смена страницы - чистим кеш
			if (pPageI) {
				gfl->gflFreeBitmap(pPageI);
				pPageI = NULL;
			}

			if (!SelectPage(iPage))
				return false;

			try{
				gfl->gflGetDefaultLoadParams(pLoadParams);
				// GFL_LOAD_ORIGINAL_COLORMODEL - If the colormodel is CMYK, keep it
				if (pFileInformation->ColorModel == GFL_CM_CMYK && !gfl->bCMYK2RGB) {
					pLoadParams->Flags = GFL_LOAD_COMMENT|GFL_LOAD_ORIGINAL_COLORMODEL;
					pLoadParams->ColorModel = GFL_CM_CMYK;
				} else {
					pLoadParams->Flags = GFL_LOAD_COMMENT;
					pLoadParams->ColorModel = GFL_BGRA;
				}
				pLoadParams->LinePadding = 1;
				pLoadParams->ImageWanted = iPage;

				if (bFileMode) {
					if (gfl->gflLoadBitmapW && pwszFileName)
						rcGFL = gfl->gflLoadBitmapW( pwszFileName, &GFLBitmap, 
							pLoadParams, pFileInformation);
					else if (gfl->gflLoadBitmap && pszFileName)
						rcGFL = gfl->gflLoadBitmap( pszFileName, &GFLBitmap, 
							pLoadParams, pFileInformation);
					else {
						nErrNumber = PGE_NO_UNICODE_FUNCTION;
						rcGFL = -1;
					}
				} else {
					rcGFL = gfl->gflLoadBitmapFromMemory( pFileBuffer, nFileBufferSize, &GFLBitmap, 
						pLoadParams, pFileInformation);
				}
				
				if (rcGFL == 0) {
					bHasAlpha = (GFLBitmap->Type & (GFL_BGRA|GFL_RGBA))!=0;

					if (gfl->gflChangeColorDepth
						&& GFLBitmap->Type != GFL_COLORS && GFLBitmap->Type != GFL_GREY
						&& GFLBitmap->Type != GFL_BINARY && GFLBitmap->Type != GFL_BGR
						&& GFLBitmap->Type != GFL_BGRA && GFLBitmap->Type != GFL_CMYK
						)
					{
						rcGFL = gfl->gflChangeColorDepth(GFLBitmap, NULL, 
							(GFLBitmap->Type == GFL_RGBA) ? GFL_MODE_TO_BGRA : GFL_MODE_TO_BGR, 
							//GFL_MODE_TO_BGRA,
							GFL_MODE_NO_DITHER);
						if (!rcGFL) {
							pFileInformation->BitsPerComponent = 8; pFileInformation->ComponentsPerPixel = 4;
						}
					}

					result = true;
					nActivePage = iPage;

					// BUGGY GFL. Например для анимированных GIF gflGetFileInformationFromMemory возвращает 1
					nPages = pFileInformation->NumberOfImages;

					int n = MultiByteToWideChar(CP_OEMCP, 0, pFileInformation->FormatName, sizeofarray(pFileInformation->FormatName), FormatName, sizeofarray(FormatName)-1);
					FormatName[n > 0 ? n : 0] = 0;
					n = MultiByteToWideChar(CP_OEMCP, 0, pFileInformation->CompressionDescription, sizeofarray(pFileInformation->CompressionDescription), Compression, sizeofarray(Compression)-1);
					Compression[n > 0 ? n : 0] = 0;
					
					BOOL lbForceAnimation = FALSE;
					if (gfl->ip.ExtensionMatch && gfl->szAnimatedFormats[0]) {
						if (gfl->ip.ExtensionMatch(gfl->szAnimatedFormats, this->FormatName))
							lbForceAnimation = TRUE;
					}

					lWidth = pFileInformation->Width;
					lHeight = pFileInformation->Height;
					nBPP = pFileInformation->BitsPerComponent * pFileInformation->ComponentsPerPixel;
					// GFL не возвращает флаг анимации так что наверное лучше вообще не анимировать
					// т.к. GIF'ы все равно не работают...
					if (lbForceAnimation && nPages > 1) {
						lFrameTime = 100;
						Animation = true;
					} else {
						lFrameTime = 0; //(nPages > 1) ? 100 : 0; // GFL не возвращает этот параметр
						Animation = false;
					}
					//if (nPages > 1) {
					//	Animation = lstrcmpiW(FormatName, L"ico") != 0;
					//}
					nTransparentIdx = GFLBitmap->TransparentIndex;

					gfl->gflFreeFileInformation(pFileInformation);
				}

			} catch(...) { nErrNumber = PGE_EXCEPTION; }
		}

		return GFLBitmap;
	};
	void Close()
	{
		//!!! nErrNumber просто так не сбрасывать - он уже может быть выставлен
		if (!gfl) return;
		try{
			if (bFileBufferAllocated && pFileBuffer)
				FREE(pFileBuffer);
			bFileBufferAllocated = FALSE; pFileBuffer = NULL; nFileBufferSize = 0;

			if (pPageI) {
				gfl->gflFreeBitmap(pPageI);
				pPageI = NULL;
			}

			if (pFileInformation) {
				FREE(pFileInformation); pFileInformation = NULL;
			}
			if (pLoadParams) {
				FREE(pLoadParams); pLoadParams = NULL;
			}
			if (pwszFileName) {
				FREE(pwszFileName); pwszFileName = NULL;
			}
			if (pszFileName) {
				FREE(pszFileName); pszFileName = NULL;
			}
		} catch(...) { nErrNumber = PGE_EXCEPTION; }
	};
	bool GetPageBits(pvdInfoDecode2 *pDecodeInfo)
	{
		bool result = false;

		#ifdef _DEBUG
			RGBQUAD rgbq  = {0,0,255,255};
		#endif

		GFL_BITMAP* GFLBitmap = DecodePage(pDecodeInfo->iPage);
		if (!GFLBitmap)
			return false;
		
		try{
			UINT32 *ColorMap = (UINT32*)CALLOC(0x100*sizeof(DWORD));
			BOOL lbAllowAlpha = TRUE;

			pDecodeInfo->lParam = (LPARAM)GFLBitmap;
			pDecodeInfo->pPalette = NULL;
			pDecodeInfo->Flags = PVD_IDF_READONLY;
			pDecodeInfo->lWidth = lWidth;
			pDecodeInfo->lHeight = lHeight;
			pDecodeInfo->lImagePitch = GFLBitmap->BytesPerLine; // отрицательные/положительные?
			pDecodeInfo->pImage = GFLBitmap->Data;
			pDecodeInfo->nColorsUsed = 0;

			//if (pFileInformation->ColorModel == GFL_CM_CMYK)
			//	pDecodeInfo->ColorModel = PVD_CM_CMYK;
			//else
			//	pDecodeInfo->ColorModel = PVD_CM_BGRA;

			if (gfl->ip.ExtensionMatch) {
				if (gfl->ip.ExtensionMatch(gfl->szNoAlphaFormats, this->FormatName))
					lbAllowAlpha = FALSE;
			}

#ifdef _DEBUG
			/*DWORD pLine[256][256];
			LPBYTE pSrc = (LPBYTE)GFLBitmap->Data;
			for (UINT y = 0; y<lHeight; y++) {
				memmove(pLine[y], pSrc, min(1024,GFLBitmap->BytesPerLine));
				pSrc += GFLBitmap->BytesPerLine;
			}*/
#endif

			result = true;
			switch (GFLBitmap->Type)
			{
				case GFL_BINARY:
					ColorMap[0] = 0;
					ColorMap[1] = 0xFFFFFF;
					pDecodeInfo->pPalette = ColorMap;
					pDecodeInfo->nBPP = 1;
					pDecodeInfo->ColorModel = PVD_CM_BGR;
					//pDecodeInfo->Flags |= PVD_IDF_PALETTE; -- не требуется
					break;
				case GFL_GREY:
					for (UINT i = 0x100, Color = 0xFFFFFF; i--; Color -= 0x010101)
						ColorMap[i] = Color;
					pDecodeInfo->pPalette = ColorMap;
					pDecodeInfo->nBPP = 8;
					pDecodeInfo->ColorModel = PVD_CM_BGR;
					//pDecodeInfo->Flags |= PVD_IDF_PALETTE; -- не требуется
					if (lbAllowAlpha && nTransparentIdx != -1) {
						pDecodeInfo->nTransparentColor = nTransparentIdx;
						pDecodeInfo->Flags |= PVD_IDF_TRANSPARENT_INDEX;
					}
					break;
				case GFL_COLORS:
					for (UINT i = 0x100; i--;)
						ColorMap[i] = GFLBitmap->ColorMap->Blue[i] | GFLBitmap->ColorMap->Green[i] << 8 | GFLBitmap->ColorMap->Red[i] << 0x10;
					pDecodeInfo->pPalette = ColorMap;
					pDecodeInfo->nBPP = 8;
					pDecodeInfo->ColorModel = PVD_CM_BGR;
					//pDecodeInfo->Flags |= PVD_IDF_PALETTE; -- не требуется
					if (lbAllowAlpha && nTransparentIdx != -1) {
						pDecodeInfo->nTransparentColor = nTransparentIdx;
						pDecodeInfo->Flags |= PVD_IDF_TRANSPARENT_INDEX;
					}
					break;
				case GFL_BGR:
					pDecodeInfo->nBPP = 24;
					pDecodeInfo->ColorModel = PVD_CM_BGR;
					//pDecodeInfo->Flags |= PVD_IDF_BGR;
					break;
				case GFL_CMYK:
					pDecodeInfo->nBPP = 32;
					pDecodeInfo->ColorModel = PVD_CM_CMYK;
					//pDecodeInfo->Flags |= PVD_IDF_BGR;
					break;
				//case GFL_RGB:
				//	pDecodeInfo->nBPP = 24;
				//	pDecodeInfo->Flags |= PVD_IDF_RGB;
				//	break;
				case GFL_BGRA:
					pDecodeInfo->nBPP = 32;
					if (lbAllowAlpha) {
						pDecodeInfo->Flags |= PVD_IDF_ALPHA;
						pDecodeInfo->ColorModel = PVD_CM_BGRA;
					} else {
						pDecodeInfo->ColorModel = PVD_CM_BGR;
					}
					//#ifdef _DEBUG
					//for (int i=0; i < 100; i++) {
					//	((DWORD*)pDecodeInfo->pImage)[i] = *((DWORD*)&rgbq);
					//}
					//#endif
					break;
				//case GFL_ABGR:
				//	pDecodeInfo->nBPP = 32;
				//	pDecodeInfo->Flags |= PVD_IDF_ABGR;
				//	break;
				//case GFL_RGBA:
				//	pDecodeInfo->nBPP = 32;
				//	pDecodeInfo->Flags |= PVD_IDF_RGBA;
				//	break;
				//case GFL_ARGB:
				//	pDecodeInfo->nBPP = 32;
				//	pDecodeInfo->Flags |= PVD_IDF_ARGB;
				//	break;
				default:
					nErrNumber = PGE_UNKNOWN_COLORSPACE;
					result = false;
			}
			if (!pDecodeInfo->pPalette)
				FREE(ColorMap);
		} catch(...) { nErrNumber = PGE_EXCEPTION; }
	
		if (!result && GFLBitmap) {
			gfl->gflFreeBitmap(GFLBitmap);
			pDecodeInfo->lParam = 0;
		}

		return result;
	};
	void FreePageBits(pvdInfoDecode2 *pDecodeInfo)
	{
		if (!pDecodeInfo || !pDecodeInfo->lParam) return;
		try{
			GFL_BITMAP* GFLBitmap = (GFL_BITMAP*)pDecodeInfo->lParam;
			if (GFLBitmap) {
				gfl->gflFreeBitmap(GFLBitmap);
				pDecodeInfo->lParam = 0;
			}
			UINT32 *ColorMap = pDecodeInfo->pPalette;
			if (ColorMap) {
				FREE(ColorMap);
				pDecodeInfo->pPalette = 0;
			}
		} catch(...) { nErrNumber = PGE_EXCEPTION; }
	}
};





UINT32 __stdcall pvdInit2(pvdInitPlugin2* pInit)
{
	_ASSERTE(pInit->cbSize >= sizeof(pvdInitPlugin2));
	if (pInit->cbSize < sizeof(pvdInitPlugin2)) {
		pInit->nErrNumber = PGE_OLD_PICVIEW;
		return 0;
	}

	ghModule = pInit->hModule;

	GFLDecoder *pDecoder = (GFLDecoder*) CALLOC(sizeof(GFLDecoder));
	if (!pDecoder) {
		pInit->nErrNumber = PGE_NOT_ENOUGH_MEMORY;
		return 0;
	}
	if (!pDecoder->Init(pInit)) {
		pInit->nErrNumber = pDecoder->nErrNumber;
		pvdExit2(pDecoder);
		return 0;
	}

	//pInit->pContext = pDecoder; -- Уже вернул pDecoder->Init
	return PVD_UNICODE_INTERFACE_VERSION;
}

void __stdcall pvdGetFormats2(void *pContext, pvdFormats2* pFormats)
{
	_ASSERTE(pFormats->cbSize >= sizeof(pvdFormats2));

	// !!! Анимированные GIF в GFL глючат
	// !!! Прозрачность в иконках (ICO) не поддерживается
	// !!! В некоторых PNG файлах не поддерживается прозрачность
	// !!! При "декодировании" WMF - гадит в консоль
	pFormats->pForbidden = L"EMF,WMF";
	pFormats->pInactive = L"GIF,PNG,ICO";
	pFormats->pActive = L"BMP,DIB,JPG,JPE,JPEG,TIF,TIFF,EXIF";

	if (pContext) {
		pFormats->pActive = ((GFLDecoder*)pContext)->LoadSupported();
	}
	
	//TO DO("Запустить enumerator форматов в GFL");
    /*
GFL_INT32 gflGetNumberOfFormat(
  void
);
const char * gflGetFormatNameByIndex(
  GFL_INT32 index
);
const char * gflGetFormatDescriptionByIndex(
  GFL_INT32 index
);
	*/
}

void __stdcall pvdExit2(void *pContext)
{
	if (pContext) {
		GFLDecoder *pDecoder = (GFLDecoder*)pContext;
		pDecoder->Close();

		FREE(pContext);
	}
}

void __stdcall pvdPluginInfo2(pvdInfoPlugin2 *pPluginInfo)
{
	_ASSERTE(pPluginInfo->cbSize >= sizeof(pvdInfoPlugin2));
	pPluginInfo->Priority = 0x300;
	pPluginInfo->pName = L"GFL";
	pPluginInfo->pVersion = GetVersion(pPluginInfo->hModule);
	pPluginInfo->pComments = L"Copyright © 2009 Maximus5";
	pPluginInfo->Flags = PVD_IP_DECODE;
}

BOOL __stdcall pvdFileOpen2(void *pContext, const wchar_t *pFileName, INT64 lFileSize, const BYTE *pBuf, UINT32 lBuf, pvdInfoImage2 *pImageInfo)
{
	TODO("Может в GFL есть функция проверки формата изображения?? Чтобы не пытаться сразу делать копию памяти и копировать данные");
	_ASSERTE(pImageInfo->cbSize >= sizeof(pvdInfoImage2));
	if (!pContext) {
		pImageInfo->nErrNumber = PGE_INVALID_CONTEXT;
		return FALSE;
	}

	GFLImage *pImage = (GFLImage*)CALLOC(sizeof(GFLImage));
	if (!pImage) {
		pImageInfo->nErrNumber = PGE_NOT_ENOUGH_MEMORY;
		return FALSE;
	}

	pImage->gfl = (GFLDecoder*)pContext;

	if (!pImage->Open(pFileName, lFileSize, pBuf, lBuf)) {
		pImageInfo->nErrNumber = pImage->nErrNumber;
		pvdFileClose2(pContext, pImage);
		return FALSE;
	}

	pImageInfo->pImageContext = pImage;
	pImageInfo->nPages = pImage->nPages;
	pImageInfo->Flags = (pImage->Animation ? PVD_IIF_ANIMATED : 0);
	pImageInfo->pFormatName = pImage->FormatName;
	pImageInfo->pCompression = NULL;
	pImageInfo->pComments = NULL;

	return TRUE;
}

BOOL __stdcall pvdPageInfo2(void *pContext, void *pImageContext, pvdInfoPage2 *pPageInfo)
{
	if (!pImageContext) {
		pPageInfo->nErrNumber = PGE_INVALID_CONTEXT;
		return FALSE;
	}
	_ASSERTE(pPageInfo->cbSize >= sizeof(pvdInfoPage2));

	GFLImage* pImage = (GFLImage*)pImageContext;
	// zero based
	if (pImage->nActivePage != pPageInfo->iPage) {
		// SelectPage недостаточно. Ее нужно сразу декодировать иначе параметры будут неверные
		if ((pImage->pPageI = pImage->DecodePage(pPageInfo->iPage)) == NULL) {
			pPageInfo->nErrNumber = pImage->nErrNumber;
			return FALSE;
		}
	}

	pPageInfo->lWidth = pImage->lWidth;
	_ASSERTE(pImage->lHeight>0);
	pPageInfo->lHeight = pImage->lHeight;
	pPageInfo->nBPP = pImage->nBPP;
	pPageInfo->lFrameTime = pImage->lFrameTime;
	return TRUE;
}

BOOL __stdcall pvdPageDecode2(void *pContext, void *pImageContext, pvdInfoDecode2 *pDecodeInfo, 
							  pvdDecodeCallback2 DecodeCallback, void *pDecodeCallbackContext)
{
	if (!pImageContext) {
		pDecodeInfo->nErrNumber = PGE_INVALID_CONTEXT;
		return FALSE;
	}

	_ASSERTE(pDecodeInfo->cbSize >= sizeof(pvdInfoDecode2));

	GFLImage* pImage = (GFLImage*)pImageContext;
	if (!pImage->SelectPage(pDecodeInfo->iPage))
		return FALSE;

	BOOL lbRc = pImage->GetPageBits(pDecodeInfo);
	if (!lbRc) pDecodeInfo->nErrNumber = pImage->nErrNumber;

	return lbRc;
}

void __stdcall pvdPageFree2(void *pContext, void *pImageContext, pvdInfoDecode2 *pDecodeInfo)
{
	_ASSERTE(pDecodeInfo->cbSize >= sizeof(pvdInfoDecode2));
	GFLImage* pImage = (GFLImage*)pImageContext;
	pImage->FreePageBits(pDecodeInfo);
}

void __stdcall pvdFileClose2(void *pContext, void *pImageContext)
{
	if (pImageContext) {
		((GFLImage*)pImageContext)->Close();
		FREE(pImageContext);
	}
}

void __stdcall pvdReloadConfig2(void *pContext)
{
	GFLDecoder *pDecoder = (GFLDecoder*)pContext;
	if (pDecoder) {
		pDecoder->ReloadConfig();
	}
}
