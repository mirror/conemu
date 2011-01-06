
#include <windows.h>
#include "../PVD2Helper.h"
#include "../PictureViewPlugin.h"

typedef int i32;
typedef unsigned char u8;
typedef unsigned short u16;
typedef DWORD u32;

HMODULE ghModule = NULL;

// ‘ункци€ требуетс€ только дл€ заполнени€ переменной ghModule
// ≈сли плагин содержит свою точку входа - дл€ использовани€ PVD1Helper
// ему необходимо заполн€ть ghModule самосто€тельно
//BOOL WINAPI DllMain( HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved )
//{
//	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
//		ghModule = (HMODULE)hModule;
//	return TRUE;
//}

pvdInitPlugin2 ip = {0};

#define PWE_NO_EXTENSION         0x1001
#define PWE_UNKNOWN_EXTENSION    0x1002
#define PWE_INVALID_CONTEXT      0x80000000
#define PWE_NOT_ENOUGH_MEMORY    0x80000001
#define PWE_RASTER_NOT_ALLOWED   0x80000002
#define PWE_NOT_VALID_METAFILE   0x80000003
#define PWE_INVALID_RECT         0x80000004
#define PWE_OLD_PICVIEW          0x80000005
#define PWE_WIN32_ERROR          0x80001000



DWORD gnLastWin32Error = 0;

BOOL __stdcall pvdTranslateError2(DWORD nErrNumber, wchar_t *pszErrInfo, int nBufLen)
{
	if (!pszErrInfo || nBufLen<=0)
		return FALSE;

	switch (nErrNumber)
	{
	case PWE_WIN32_ERROR:
		FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS, 0, 
			(DWORD)gnLastWin32Error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			pszErrInfo, nBufLen, NULL);
		break;
	case PWE_INVALID_CONTEXT:
		lstrcpyn(pszErrInfo, L"Invalid image context", nBufLen); break;
	case PWE_NO_EXTENSION:
		lstrcpyn(pszErrInfo, L"File have no extension", nBufLen); break;
	case PWE_UNKNOWN_EXTENSION:
		lstrcpyn(pszErrInfo, L"Unknown file extension", nBufLen); break;
	case PWE_NOT_ENOUGH_MEMORY:
		lstrcpyn(pszErrInfo, L"Not enough memory", nBufLen); break;
	case PWE_RASTER_NOT_ALLOWED:
		lstrcpyn(pszErrInfo, L"Raster operations are not allowed", nBufLen); break;
	case PWE_NOT_VALID_METAFILE:
		lstrcpyn(pszErrInfo, L"Not valid metafile", nBufLen); break;
	case PWE_INVALID_RECT:
		lstrcpyn(pszErrInfo, L"Invalid paint rect", nBufLen); break;
	case PWE_OLD_PICVIEW:	
		lstrcpyn(pszErrInfo, L"Old PicView version, exiting", nBufLen); break;
	default:
		return FALSE;
	}
	return TRUE;
}

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


UINT32 __stdcall pvdInit2(pvdInitPlugin2* pInit)
{
	_ASSERTE(pInit->cbSize == sizeof(pvdInitPlugin2));
	if (pInit->cbSize < sizeof(pvdInitPlugin2)) {
		pInit->nErrNumber = PWE_OLD_PICVIEW;
		return 0;
	}

	memset(&ip,0,sizeof(ip));
	memmove(&ip, pInit, min(sizeof(ip),pInit->cbSize));
	ghModule = ip.hModule;

	return PVD_UNICODE_INTERFACE_VERSION;
}

void __stdcall pvdGetFormats2(void *pContext, pvdFormats2* pFormats)
{
	_ASSERTE(pFormats->cbSize >= sizeof(pvdFormats2));

	pFormats->pActive = L"EMF,WMF";
	pFormats->pInactive = pFormats->pForbidden = L"";
}

void __stdcall pvdExit2(void *pContext)
{
}

void __stdcall pvdPluginInfo2(pvdInfoPlugin2 *pPluginInfo)
{
	_ASSERTE(pPluginInfo->cbSize >= sizeof(pvdInfoPlugin2));
	pPluginInfo->Priority = 0x100;
	pPluginInfo->pName = L"WMF";
	pPluginInfo->pVersion = GetVersion(pPluginInfo->hModule);
	pPluginInfo->pComments = L"Copyright © 2009 Maximus5";
	pPluginInfo->Flags = PVD_IP_DECODE|PVD_IP_DISPLAY|PVD_IP_PRIVATE;
}

const wchar_t* GetExtension(const wchar_t *pFileName)
{
	const wchar_t *p = NULL;
	const wchar_t *pS = wcsrchr(pFileName, L'\\');
	if (!pS) pS = pFileName;
	if (((p = wcsrchr(pFileName, L'.')) != NULL) && (p >= pS))
		p++;
	else
		p = NULL;
	return p;
}

BOOL __stdcall pvdFileDetect2(void *pContext, const wchar_t *pFileName, INT64 lFileSize, const BYTE *pBuf, UINT32 lBuf, pvdInfoImage2 *pImageInfo)
{
	const wchar_t *p = GetExtension(pFileName);
	if (!p) {
		pImageInfo->nErrNumber = PWE_NO_EXTENSION;
		return FALSE;
	}

	if (lstrcmpi(p, L"WMF") && lstrcmpi(p, L"EMF")) {
		pImageInfo->nErrNumber = PWE_UNKNOWN_EXTENSION;
		return FALSE;
	}

	pImageInfo->Flags = 0; //PVD_IIF_FILE_REQUIRED;
	return TRUE;
}

typedef struct tag_WmfContext
{
	int nRefCount;
	HENHMETAFILE h;
	SIZE PreferredSize;
	METAFILEPICT mfp;

	PAINTSTRUCT ps;
	SIZE CurrentSize;
	HDC hCompDC; // используетс€ при отрисовке
	HBITMAP hOldBitmap, hCompBitmap; // используетс€ при отрисовке


	void AddRef() {
		nRefCount++;
	};
	void DeleteComp() {
		if (hCompDC) {
			SelectObject(hCompDC, hOldBitmap);
			DeleteObject(hCompBitmap); hCompBitmap = NULL;
			DeleteDC(hCompDC); hCompDC = NULL;
		}
		CurrentSize.cx = 0; CurrentSize.cy = 0;
	};
	HDC CreateComp(HDC hdc, int w, int h) {
		DeleteComp();
		hCompDC = CreateCompatibleDC(hdc);
		if (hCompDC) {
			hCompBitmap = CreateCompatibleBitmap(hdc, w, h);
			if (hCompBitmap) {
				hOldBitmap = (HBITMAP)SelectObject(hCompDC, hCompBitmap);
				CurrentSize.cx = w; CurrentSize.cy = h;
			} else {
				DeleteDC(hCompDC); hCompDC = NULL;
			}
		}
		return hCompDC;
	};
	void Release() {
		nRefCount--;
		_ASSERTE(nRefCount >= 0);
		if (nRefCount <= 0) {
			DeleteComp();
			if (h) {
				DeleteEnhMetaFile(h); h = NULL;
			}
			FREE(this);
		}
	};
} WmfContext;

// Aldus Placeable Header ===================================================
// Since we are a 32bit app, we have to be sure this structure compiles to
// be identical to a 16 bit app's version. To do this, we use the #pragma
// to adjust packing, we use a WORD for the hmf handle, and a SMALL_RECT
// for the bbox rectangle.
#pragma pack( push )
#pragma pack( 2 )
typedef struct
{
	DWORD		dwKey;
	WORD		hmf;
	SMALL_RECT	bbox;
	WORD		wInch;
	DWORD		dwReserved;
	WORD		wCheckSum;
} APMHEADER, *PAPMHEADER;
#pragma pack( pop )
// ==========================================================================

BOOL __stdcall pvdFileOpen2(void *pContext, const wchar_t *pFileName, INT64 lFileSize, const BYTE *pBuf, UINT32 lBuf, pvdInfoImage2 *pImageInfo)
{
	_ASSERTE(pImageInfo->cbSize >= sizeof(pvdInfoImage2));

	WmfContext *pw = (WmfContext*)CALLOC(sizeof(WmfContext));
	if (!pw) {
		pImageInfo->nErrNumber = PWE_NOT_ENOUGH_MEMORY;
		return FALSE;
	}
	//pw->h = GetEnhMetaFile ( pFileName+4 );

	const wchar_t *p = GetExtension(pFileName);
	if (!p) {
		pImageInfo->nErrNumber = PWE_NO_EXTENSION;
		return FALSE;
	}

	pImageInfo->nErrNumber = PWE_WIN32_ERROR; // заранее

	//if (pImageInfo->PreferredSize.cx && pImageInfo->PreferredSize.cy) {
	//	pw->PreferredSize = pImageInfo->PreferredSize;
	//} else {
	pw->PreferredSize.cx = 1000; pw->PreferredSize.cy = 1000;
	//}

	// First try to read it as an enhanced metafile
	// If it works, simply return the handle
	pw->h = SetEnhMetaFileBits(lBuf, pBuf);
	gnLastWin32Error = GetLastError();

	if (!pw->h) {
		pw->h = GetEnhMetaFile( pFileName );
		gnLastWin32Error = GetLastError();
	}

	if (!pw->h) {
		HMETAFILE		hOld;
		DWORD			dwSize;
		LPBYTE			pBits;
		METAFILEPICT	mp;
		HDC				hDC;

		if( (hOld = GetMetaFile( pFileName )) != NULL )
		{
			// Ok, it is a 16bit windows metafile
			// How big are the bits?
			if( (dwSize = GetMetaFileBitsEx( hOld, 0, NULL )) == 0 )
			{
				gnLastWin32Error = GetLastError();
				DeleteMetaFile( hOld );
				//MessageBox( hWndParent, "Failed to Get MetaFile Bits Size", "Error Reading MetaFile", MB_OK );
			} else {
				// Allocate that much memory
				if( (pBits = (LPBYTE)CALLOC( dwSize )) == NULL )
				{
					pImageInfo->nErrNumber = PWE_NOT_ENOUGH_MEMORY;
					gnLastWin32Error = GetLastError();
					DeleteMetaFile( hOld );
					//MessageBox( hWndParent, "Failed to Allocate Memory for Metafile Bits", "Error Reading MetaFile", MB_OK );
					//return NULL;
				} else {
					// Get the metafile bits
					if( GetMetaFileBitsEx( hOld, dwSize, pBits ) == 0 )
					{
						gnLastWin32Error = GetLastError();
						FREE( pBits );
						DeleteMetaFile( hOld );
						//MessageBox( hWndParent, "Failed to Get MetaFile Bits", "Error Reading MetaFile", MB_OK );
						//return NULL;
					} else {
						// Fill out a METAFILEPICT structure
						mp.mm = MM_ANISOTROPIC;
						mp.xExt = 1000;
						mp.yExt = 1000;
						mp.hMF = NULL;
						// Get a reference DC
						hDC = GetDC( NULL );
						// Make an enhanced metafile from the windows metafile
						pw->h = SetWinMetaFileBits( dwSize, pBits, hDC, &mp );
						gnLastWin32Error = GetLastError();
						// Clean up
						ReleaseDC( NULL, hDC );
						DeleteMetaFile( hOld );
						FREE( pBits );
					}
				}
			}
		}
	}

	if (!pw->h) {
		DWORD			dwSize = lBuf;
		LPBYTE			pBits = (LPBYTE)pBuf;
		METAFILEPICT	mp;
		HDC				hDC;

		// Is it a placeable metafile? (check the key)
		if( ((PAPMHEADER)pBits)->dwKey != 0x9ac6cdd7l )
		{
			// Not a metafile that we know how to recognise - bail out
			//MessageBox( hWndParent, "Not a Valid Metafile", szFileName, MB_OK );
			//return NULL;
			pImageInfo->nErrNumber = PWE_NOT_VALID_METAFILE;
		} else {
			// Ok, its a placeable metafile
			// Fill out a METAFILEPICT structure
			mp.mm = MM_ANISOTROPIC;
			mp.xExt = ((PAPMHEADER)pBits)->bbox.Right - ((PAPMHEADER)pBits)->bbox.Left;
			mp.xExt = ( mp.xExt * 2540l ) / (DWORD)(((PAPMHEADER)pBits)->wInch);
			mp.yExt = ((PAPMHEADER)pBits)->bbox.Bottom - ((PAPMHEADER)pBits)->bbox.Top;
			mp.yExt = ( mp.yExt * 2540l ) / (DWORD)(((PAPMHEADER)pBits)->wInch);
			mp.hMF = NULL;
			// Get a reference DC
			hDC = GetDC( NULL );
			// Create an enhanced metafile from the bits
			pw->h = SetWinMetaFileBits( dwSize, &(pBits[sizeof(APMHEADER)]), hDC, &mp );
			gnLastWin32Error = GetLastError();
			// Clean up
			ReleaseDC( NULL, hDC );
			//free( pBits );
			//if( hTemp == NULL )
			//	MessageBox( hWndParent, "Failed to Create MetaFile from Bits", "Error Reading MetaFile", MB_OK );
			//return hTemp;
		}
	}

	//if (pw->h) {
	//	//pw->hCompDC = CreateCompatibleDC(NULL);
	//	pw->mfp.mm = MM_ISOTROPIC;
	//	pw->mfp.xExt = pw->PreferredSize.cx;
	//	pw->mfp.yExt = pw->PreferredSize.cy;
	//	pw->h = (HENHMETAFILE)SetMetaFileBitsEx (lBuf, pBuf);
	//	gnLastWin32Error = GetLastError();
	//	pw->h = SetWinMetaFileBits(lBuf, pBuf, NULL, NULL); //pw->hCompDC, &pw->mfp);
	//	gnLastWin32Error = GetLastError();
	//}

	if (!pw->h) {
		//gnLastWin32Error = GetLastError(); -- уже
		FREE(pw);
		return FALSE;
	}

	pw->AddRef();
	_ASSERTE(pw->nRefCount == 1);

	ENHMETAHEADER	emh = {0};
	DWORD			PixelsX, PixelsY, MMX, MMY;
	emh.nSize = sizeof(ENHMETAHEADER);
	if( GetEnhMetaFileHeader( pw->h, sizeof( ENHMETAHEADER ), &emh ) )
	{
		// Get the characteristics of the output device
		HDC hDC = GetDC(NULL);
		PixelsX = GetDeviceCaps( hDC, HORZRES );
		PixelsY = GetDeviceCaps( hDC, VERTRES );
		MMX = GetDeviceCaps( hDC, HORZSIZE ) * 100;
		MMY = GetDeviceCaps( hDC, VERTSIZE ) * 100;
		ReleaseDC(NULL, hDC);

		// Calculate the rect in which to draw the metafile based on the
		// intended size and the current output device resolution
		// Remember that the intended size is given in 0.01mm units, so
		// convert those to device units on the target device
		//pw->PreferredSize.cx = (int)((float)(emh.rclFrame.right - emh.rclFrame.left) * PixelsX / (MMX));
		//pw->PreferredSize.cy = (int)((float)(emh.rclFrame.bottom - emh.rclFrame.top) * PixelsY / (MMY));
		pw->PreferredSize.cx = ip.MulDivI32((emh.rclFrame.right - emh.rclFrame.left), PixelsX, MMX);
		pw->PreferredSize.cy = ip.MulDivI32((emh.rclFrame.bottom - emh.rclFrame.top), PixelsY, MMY);
		_ASSERTE(pw->PreferredSize.cx>0 && pw->PreferredSize.cy>0);
		if (pw->PreferredSize.cx < 0) pw->PreferredSize.cx = -pw->PreferredSize.cx;
		if (pw->PreferredSize.cy < 0) pw->PreferredSize.cy = -pw->PreferredSize.cy;
	}

	pImageInfo->pImageContext = pw;
	pImageInfo->nPages = 1;
	pImageInfo->Flags = 0;
	pImageInfo->pFormatName = L"WMF";
	pImageInfo->pCompression = NULL;
	pImageInfo->pComments = NULL;

	return TRUE;
}

BOOL __stdcall pvdPageInfo2(void *pContext, void *pImageContext, pvdInfoPage2 *pPageInfo)
{
	if (!pImageContext) {
		pPageInfo->nErrNumber = PWE_INVALID_CONTEXT;
		return FALSE;
	}
	_ASSERTE(pPageInfo->cbSize >= sizeof(pvdInfoPage2));

	WmfContext* pImage = (WmfContext*)pImageContext;

	pPageInfo->lWidth = pImage->PreferredSize.cx;
	pPageInfo->lHeight = pImage->PreferredSize.cy;
	pPageInfo->nBPP = 24;
	return TRUE;
}

BOOL __stdcall pvdPageDecode2(void *pContext, void *pImageContext, pvdInfoDecode2 *pDecodeInfo, 
							  pvdDecodeCallback2 DecodeCallback, void *pDecodeCallbackContext)
{
	if (!pImageContext) {
		pDecodeInfo->nErrNumber = PWE_INVALID_CONTEXT;
		return FALSE;
	}
	if (pDecodeInfo->Flags & PVD_IDF_COMPAT_MODE) {
		pDecodeInfo->nErrNumber = PWE_RASTER_NOT_ALLOWED;
		return FALSE;
	}

	_ASSERTE(pDecodeInfo->cbSize >= sizeof(pvdInfoDecode2));

	WmfContext* pImage = (WmfContext*)pImageContext;

	pImage->nRefCount ++;
	pDecodeInfo->Flags = PVD_IDF_PRIVATE_DISPLAY;
	pDecodeInfo->lParam = (LPARAM)pImageContext;
	pDecodeInfo->nBPP = 24;
	pDecodeInfo->lWidth = pImage->PreferredSize.cx;
	pDecodeInfo->lHeight = pImage->PreferredSize.cy;
	pDecodeInfo->ColorModel = PVD_CM_PRIVATE;

	return TRUE;
}

void __stdcall pvdPageFree2(void *pContext, void *pImageContext, pvdInfoDecode2 *pDecodeInfo)
{
	_ASSERTE(pDecodeInfo->cbSize >= sizeof(pvdInfoDecode2));
	WmfContext* pImage = (WmfContext*)pImageContext;
	pImage->Release();
}

void __stdcall pvdFileClose2(void *pContext, void *pImageContext)
{
	if (pImageContext) {
		WmfContext* pImage = (WmfContext*)pImageContext;
		pImage->Release();
	}
}

//void __stdcall pvdReloadConfig2(void *pContext)
//{
//}



// »нициализаци€ контекста диспле€. »спользуетс€ тот pContext, который был получен в pvdInit2
BOOL __stdcall pvdDisplayInit2(void *pContext, pvdInfoDisplayInit2* pDisplayInit)
{
	return TRUE;
}

BOOL __stdcall pvdDisplayAttach2(void *pContext, pvdInfoDisplayAttach2* pDisplayAttach)
{
	return TRUE;
}


// —оздать контекст дл€ отображени€ картинки в pContext (перенос декодированных данных в видеопам€ть)
BOOL __stdcall pvdDisplayCreate2(void *pContext, pvdInfoDisplayCreate2* pDisplayCreate)
{
	_ASSERTE(pDisplayCreate && pDisplayCreate->pImage && pDisplayCreate->pImage->lParam);

	WmfContext* pImage = (WmfContext*)pDisplayCreate->pImage->lParam;
	pImage->AddRef();

	pDisplayCreate->pDisplayContext = pImage;

	return TRUE;
}

BOOL ColorFill(HDC hdc, RECT ImgRect, DWORD nBackColor)
{
	HBRUSH hBr = CreateSolidBrush(nBackColor);

	FillRect(hdc, &ImgRect, hBr);

	DeleteObject(hBr);

	return TRUE;
}

BOOL __stdcall pvdDisplayPaint2(void *pContext, void* pDisplayContext, pvdInfoDisplayPaint2* pDisplayPaint)
{
	_ASSERTE(pDisplayPaint->cbSize >= sizeof(pvdInfoDisplayPaint2));
	_ASSERTE(pDisplayContext);

	BOOL lbRc = FALSE;
	WmfContext* p = (WmfContext*)pDisplayContext;

	switch (pDisplayPaint->Operation)
	{
	case PVD_IDP_BEGIN:
		{
			//if (!BeginPaint(pDisplayPaint->hWnd, &p->ps)) {
			if (!(p->ps.hdc = GetDC(pDisplayPaint->hWnd))) {
				pDisplayPaint->nErrNumber = PWE_WIN32_ERROR;
				gnLastWin32Error = GetLastError();
			} else {
				lbRc = TRUE;
			}
		} break;
	case PVD_IDP_COMMIT:
		{
			//EndPaint(pDisplayPaint->hWnd, &p->ps);
			ReleaseDC(pDisplayPaint->hWnd, p->ps.hdc);
			lbRc = TRUE;
		} break;
	case PVD_IDP_COLORFILL:
		{
			ColorFill(p->ps.hdc, pDisplayPaint->DisplayRect, pDisplayPaint->nBackColor);
			lbRc = TRUE;
		} break;
	case PVD_IDP_PAINT:
		{
			
			if (pDisplayPaint->DisplayRect.right == pDisplayPaint->DisplayRect.left
				|| pDisplayPaint->DisplayRect.bottom == pDisplayPaint->DisplayRect.top
				|| pDisplayPaint->ImageRect.right == pDisplayPaint->ImageRect.left
				|| pDisplayPaint->ImageRect.bottom == pDisplayPaint->ImageRect.top
				)
			{
				ColorFill(p->ps.hdc, pDisplayPaint->DisplayRect, pDisplayPaint->nBackColor);
				lbRc = FALSE;
				pDisplayPaint->nErrNumber = PWE_INVALID_RECT;
			} else {

				SIZE FullSize = p->PreferredSize;
				POINT StartPoint = {0,0};

				if (pDisplayPaint->ImageRect.left || pDisplayPaint->ImageRect.top
					|| ((pDisplayPaint->DisplayRect.right - pDisplayPaint->DisplayRect.left) != FullSize.cx)
					|| ((pDisplayPaint->DisplayRect.bottom - pDisplayPaint->DisplayRect.top) != FullSize.cy))
				{
					FullSize.cx = (pDisplayPaint->DisplayRect.right - pDisplayPaint->DisplayRect.left)
						* p->PreferredSize.cx / (pDisplayPaint->ImageRect.right - pDisplayPaint->ImageRect.left);
					FullSize.cy = (pDisplayPaint->DisplayRect.bottom - pDisplayPaint->DisplayRect.top)
						* p->PreferredSize.cy / (pDisplayPaint->ImageRect.bottom - pDisplayPaint->ImageRect.top);
					_ASSERTE(FullSize.cx && FullSize.cy);
					StartPoint.x = pDisplayPaint->ImageRect.left * FullSize.cx / p->PreferredSize.cx;
					StartPoint.y = pDisplayPaint->ImageRect.top * FullSize.cy / p->PreferredSize.cy;
				}

				if (FullSize.cx != p->CurrentSize.cx || FullSize.cy != p->CurrentSize.cy) {
					if (!p->CreateComp(p->ps.hdc, FullSize.cx, FullSize.cy)) {
						pDisplayPaint->nErrNumber = PWE_WIN32_ERROR;
						gnLastWin32Error = GetLastError();
						lbRc = FALSE;
					} else {
						RECT rcFill = {0,0,FullSize.cx,FullSize.cy};

						ColorFill(p->hCompDC, rcFill, pDisplayPaint->nBackColor);

						// To stop this function, an application can call the CancelDC function from another thread to terminate the operation. In this case, the function returns FALSE.
						lbRc = PlayEnhMetaFile(p->hCompDC, p->h, &rcFill);
						if (!lbRc) {
							pDisplayPaint->nErrNumber = PWE_WIN32_ERROR;
							gnLastWin32Error = GetLastError();
						}
					}
				}

				if (p->hCompDC) {
					lbRc = BitBlt(p->ps.hdc, pDisplayPaint->DisplayRect.left, pDisplayPaint->DisplayRect.top,
						pDisplayPaint->DisplayRect.right - pDisplayPaint->DisplayRect.left,
						pDisplayPaint->DisplayRect.bottom - pDisplayPaint->DisplayRect.top,
						p->hCompDC, StartPoint.x, StartPoint.y, SRCCOPY);
					if (!lbRc) {
						pDisplayPaint->nErrNumber = PWE_WIN32_ERROR;
						gnLastWin32Error = GetLastError();
					}
				}
			}
		}
	}

	return lbRc;
}

// «акрыть контекст дл€ отображени€ картинки (освободить видеопам€ть)
void __stdcall pvdDisplayClose2(void *pContext, void* pDisplayContext)
{
	_ASSERTE(pDisplayContext);
	WmfContext* p = (WmfContext*)pDisplayContext;
	p->Release();
}

// «акрыть модуль вывода (освобождение интерфейсов DX, отцепитьс€ от окна)
void __stdcall pvdDisplayExit2(void *pContext)
{
}
