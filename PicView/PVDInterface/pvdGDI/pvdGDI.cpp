
#include <windows.h>
#include <GdiPlus.h>
#define DEFINE_OPTIONS
#include "../../Options.h"
#include "../PictureViewPlugin.h"
#include "../PVD2Helper.h"
#include "../BltHelper.h"
#include "../MStream.h"

typedef __int32 i32;
typedef __int64 i64;
typedef unsigned __int8 u8;
typedef unsigned __int16 u16;
typedef DWORD u32;

extern HMODULE ghModule;

pvdInitPlugin2 ip = {0};

#define PGE_INVALID_FRAME        0x1001
#define PGE_ERROR_BASE           0x80000000
#define PGE_DLL_NOT_FOUND        0x80001001
#define PGE_EXCEPTION            0x80001002
#define PGE_FILE_NOT_FOUND       0x80001003
#define PGE_NOT_ENOUGH_MEMORY    0x80001004
#define PGE_INVALID_CONTEXT      0x80001005
#define PGE_FUNCTION_NOT_FOUND   0x80001006
#define PGE_WIN32_ERROR          0x80001007
#define PGE_UNKNOWN_COLORSPACE   0x80001008
#define PGE_UNSUPPORTED_PITCH    0x80001009
#define PGE_INVALID_PAGEDATA     0x8000100A
#define PGE_OLD_PICVIEW          0x8000100B

DWORD gnLastWin32Error = 0;

#ifdef _DEBUG
#define DebugString(x) OutputDebugString(x)
#define PaintDebugString(x) //OutputDebugString(x)
#else
#define DebugString(x)
#define PaintDebugString(x)
#endif


BOOL __stdcall pvdTranslateError2(DWORD nErrNumber, wchar_t *pszErrInfo, int nBufLen)
{
	if (!pszErrInfo || nBufLen<=0)
		return FALSE;

	switch (nErrNumber)
	{
	case PGE_WIN32_ERROR:
		FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS, 0, 
			(DWORD)gnLastWin32Error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			pszErrInfo, nBufLen, NULL);
		break;
	case PGE_UNKNOWN_COLORSPACE:
		lstrcpynW(pszErrInfo, L"Unsupported output colorspace or BPP", nBufLen); break;
	case PGE_UNSUPPORTED_PITCH:
		lstrcpynW(pszErrInfo, L"Unsupported output image pitch", nBufLen); break;
	case PGE_INVALID_FRAME:
		lstrcpynW(pszErrInfo, L"Invalid frame number", nBufLen); break;
	case PGE_ERROR_BASE+Gdiplus::GenericError:
		lstrcpynW(pszErrInfo, L"GDI+ error: GenericError", nBufLen); break;
	case PGE_ERROR_BASE+Gdiplus::InvalidParameter:
		lstrcpynW(pszErrInfo, L"GDI+ error: InvalidParameter", nBufLen); break;
	case PGE_ERROR_BASE+Gdiplus::OutOfMemory:
		lstrcpynW(pszErrInfo, L"GDI+ error: OutOfMemory", nBufLen); break;
	case PGE_ERROR_BASE+Gdiplus::ObjectBusy:
		lstrcpynW(pszErrInfo, L"GDI+ error: ObjectBusy", nBufLen); break;
	case PGE_ERROR_BASE+Gdiplus::InsufficientBuffer:
		lstrcpynW(pszErrInfo, L"GDI+ error: InsufficientBuffer", nBufLen); break;
	case PGE_ERROR_BASE+Gdiplus::NotImplemented:
		lstrcpynW(pszErrInfo, L"GDI+ error: NotImplemented", nBufLen); break;
	case PGE_ERROR_BASE+Gdiplus::Win32Error:
		lstrcpynW(pszErrInfo, L"GDI+ error: Win32Error", nBufLen); break;
	case PGE_ERROR_BASE+Gdiplus::WrongState:
		lstrcpynW(pszErrInfo, L"GDI+ error: WrongState", nBufLen); break;
	case PGE_ERROR_BASE+Gdiplus::Aborted:
		lstrcpynW(pszErrInfo, L"GDI+ error: Aborted", nBufLen); break;
	case PGE_ERROR_BASE+Gdiplus::FileNotFound:
		lstrcpynW(pszErrInfo, L"GDI+ error: FileNotFound", nBufLen); break;
	case PGE_ERROR_BASE+Gdiplus::ValueOverflow:
		lstrcpynW(pszErrInfo, L"GDI+ error: ValueOverflow", nBufLen); break;
	case PGE_ERROR_BASE+Gdiplus::AccessDenied:
		lstrcpynW(pszErrInfo, L"GDI+ error: AccessDenied", nBufLen); break;
	case PGE_ERROR_BASE+Gdiplus::UnknownImageFormat:
		lstrcpynW(pszErrInfo, L"GDI+ error: UnknownImageFormat", nBufLen); break;
	case PGE_ERROR_BASE+Gdiplus::FontFamilyNotFound:
		lstrcpynW(pszErrInfo, L"GDI+ error: FontFamilyNotFound", nBufLen); break;
	case PGE_ERROR_BASE+Gdiplus::FontStyleNotFound:
		lstrcpynW(pszErrInfo, L"GDI+ error: FontStyleNotFound", nBufLen); break;
	case PGE_ERROR_BASE+Gdiplus::NotTrueTypeFont:
		lstrcpynW(pszErrInfo, L"GDI+ error: NotTrueTypeFont", nBufLen); break;
	case PGE_ERROR_BASE+Gdiplus::UnsupportedGdiplusVersion:
		lstrcpynW(pszErrInfo, L"GDI+ error: UnsupportedGdiplusVersion", nBufLen); break;
	case PGE_ERROR_BASE+Gdiplus::GdiplusNotInitialized:
		lstrcpynW(pszErrInfo, L"GDI+ error: GdiplusNotInitialized", nBufLen); break;
	case PGE_ERROR_BASE+Gdiplus::PropertyNotFound:
		lstrcpynW(pszErrInfo, L"GDI+ error: PropertyNotFound", nBufLen); break;
	case PGE_ERROR_BASE+Gdiplus::PropertyNotSupported:
		lstrcpynW(pszErrInfo, L"GDI+ error: PropertyNotSupported", nBufLen); break;
	case PGE_DLL_NOT_FOUND:
		lstrcpynW(pszErrInfo, L"GdiPlus.dll not found", nBufLen); break;
	case PGE_EXCEPTION:
		lstrcpynW(pszErrInfo, L"Exception occurred", nBufLen); break;
	case PGE_FILE_NOT_FOUND:
		lstrcpynW(pszErrInfo, L"File not found", nBufLen); break;
	case PGE_NOT_ENOUGH_MEMORY:
		lstrcpynW(pszErrInfo, L"Not enough memory", nBufLen); break;
	case PGE_INVALID_CONTEXT:
		lstrcpynW(pszErrInfo, L"Invalid context", nBufLen); break;
	case PGE_FUNCTION_NOT_FOUND:
		lstrcpynW(pszErrInfo, L"Some gdi+ functions not found", nBufLen); break;
	case PGE_INVALID_PAGEDATA:
		lstrcpynW(pszErrInfo, L"Invalid pDisplayCreate->pImage->lParam", nBufLen); break;
	case PGE_OLD_PICVIEW:
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

struct GDIPlusDecoder
{
	HMODULE hGDIPlus;
	ULONG_PTR gdiplusToken; bool bTokenInitialized;
	HRESULT nErrNumber, nLastError;
	bool bUseICM, bForceSelfDisplay, bCMYK2RGB, bRotateOnExif;
	BOOL bCoInitialized;
	const wchar_t* pszPluginKey;
	BOOL bAsDisplay;
	BOOL bCancelled;

	typedef Gdiplus::Status (WINAPI *GdiplusStartup_t)(OUT ULONG_PTR *token, const Gdiplus::GdiplusStartupInput *input, OUT Gdiplus::GdiplusStartupOutput *output);
	typedef VOID (WINAPI *GdiplusShutdown_t)(ULONG_PTR token);
	typedef Gdiplus::GpStatus (WINGDIPAPI *GdipCreateBitmapFromFile_t)(GDIPCONST WCHAR* filename, Gdiplus::GpBitmap **bitmap);
	typedef Gdiplus::GpStatus (WINGDIPAPI *GdipCreateBitmapFromStream_t)(IStream* stream, Gdiplus::GpBitmap **bitmap);
	typedef Gdiplus::GpStatus (WINGDIPAPI *GdipCreateBitmapFromFileICM_t)(GDIPCONST WCHAR* filename, Gdiplus::GpBitmap **bitmap);
	typedef Gdiplus::GpStatus (WINGDIPAPI *GdipCreateBitmapFromStreamICM_t)(IStream* stream, Gdiplus::GpBitmap **bitmap);
	typedef Gdiplus::GpStatus (WINGDIPAPI *GdipGetImageWidth_t)(Gdiplus::GpImage *image, UINT *width);
	typedef Gdiplus::GpStatus (WINGDIPAPI *GdipGetImageHeight_t)(Gdiplus::GpImage *image, UINT *height);
	typedef Gdiplus::GpStatus (WINGDIPAPI *GdipGetImagePixelFormat_t)(Gdiplus::GpImage *image, Gdiplus::PixelFormat *format);
	typedef Gdiplus::GpStatus (WINGDIPAPI *GdipBitmapLockBits_t)(Gdiplus::GpBitmap* bitmap, GDIPCONST Gdiplus::GpRect* rect, UINT flags, Gdiplus::PixelFormat format, Gdiplus::BitmapData* lockedBitmapData);
	typedef Gdiplus::GpStatus (WINGDIPAPI *GdipBitmapUnlockBits_t)(Gdiplus::GpBitmap* bitmap, Gdiplus::BitmapData* lockedBitmapData);
	typedef Gdiplus::GpStatus (WINGDIPAPI *GdipDisposeImage_t)(Gdiplus::GpImage *image);
	typedef Gdiplus::GpStatus (WINGDIPAPI *GdipImageGetFrameCount_t)(Gdiplus::GpImage *image, GDIPCONST GUID* dimensionID, UINT* count);
	typedef Gdiplus::GpStatus (WINGDIPAPI *GdipImageSelectActiveFrame_t)(Gdiplus::GpImage *image, GDIPCONST GUID* dimensionID, UINT frameIndex);
	typedef Gdiplus::GpStatus (WINGDIPAPI *GdipGetPropertyItemSize_t)(Gdiplus::GpImage *image, PROPID propId, UINT* size);
	typedef Gdiplus::GpStatus (WINGDIPAPI *GdipGetPropertyItem_t)(Gdiplus::GpImage *image, PROPID propId, UINT propSize, Gdiplus::PropertyItem* buffer);
	typedef Gdiplus::GpStatus (WINGDIPAPI *GdipImageRotateFlip_t)(Gdiplus::GpImage *image, Gdiplus::RotateFlipType rfType);
	typedef Gdiplus::GpStatus (WINGDIPAPI *GdipGetImageRawFormat_t)(Gdiplus::GpImage *image, OUT GUID* format);
	typedef Gdiplus::GpStatus (WINGDIPAPI *GdipGetImageFlags_t)(Gdiplus::GpImage *image, UINT *flags);
	typedef Gdiplus::GpStatus (WINGDIPAPI *GdipGetImagePalette_t)(Gdiplus::GpImage *image, Gdiplus::ColorPalette *palette, INT size);
	typedef Gdiplus::GpStatus (WINGDIPAPI *GdipGetImagePaletteSize_t)(Gdiplus::GpImage *image, INT *size);
	typedef Gdiplus::GpStatus (WINGDIPAPI *GdipCreateFromHDC_t)(HDC hdc, Gdiplus::GpGraphics **graphics);
	typedef Gdiplus::GpStatus (WINGDIPAPI *GdipDeleteGraphics_t)(Gdiplus::GpGraphics *graphics);
	typedef Gdiplus::GpStatus (WINGDIPAPI *GdipDrawImageRectRectI_t)(Gdiplus::GpGraphics *graphics, Gdiplus::GpImage *image, INT dstx, INT dsty, INT dstwidth, INT dstheight, INT srcx, INT srcy, INT srcwidth, INT srcheight, Gdiplus::GpUnit srcUnit, const Gdiplus::GpImageAttributes* imageAttributes, Gdiplus::DrawImageAbort callback, VOID * callbackData);
	typedef Gdiplus::GpStatus (WINGDIPAPI *GdipCreateBitmapFromScan0_t)(INT width, INT height, INT stride, Gdiplus::PixelFormat format, BYTE* scan0, Gdiplus::GpBitmap** bitmap);
	typedef Gdiplus::GpStatus (WINGDIPAPI *GdipFillRectangleI_t)(Gdiplus::GpGraphics *graphics, Gdiplus::GpBrush *brush, INT x, INT y, INT width, INT height);
	typedef Gdiplus::GpStatus (WINGDIPAPI *GdipCreateSolidFill_t)(Gdiplus::ARGB color, Gdiplus::GpSolidFill **brush);
	typedef Gdiplus::GpStatus (WINGDIPAPI *GdipDeleteBrush_t)(Gdiplus::GpBrush *brush);
	//typedef Gdiplus::GpStatus (WINGDIPAPI *GdipCloneImage_t)(Gdiplus::GpImage *image, Gdiplus::GpImage **cloneImage);
	typedef Gdiplus::GpStatus (WINGDIPAPI *GdipCloneBitmapAreaI_t)(INT x, INT y, INT width, INT height, Gdiplus::PixelFormat format, Gdiplus::GpBitmap *srcBitmap, Gdiplus::GpBitmap **dstBitmap);
	typedef Gdiplus::GpStatus (WINGDIPAPI *GdipSetImagePalette_t)(Gdiplus::GpImage *image, GDIPCONST Gdiplus::ColorPalette *palette);



	GdiplusStartup_t GdiplusStartup;
	GdiplusShutdown_t GdiplusShutdown;
	GdipCreateBitmapFromFile_t GdipCreateBitmapFromFile;
	GdipCreateBitmapFromStream_t GdipCreateBitmapFromStream;
	GdipCreateBitmapFromFileICM_t GdipCreateBitmapFromFileICM;
	GdipCreateBitmapFromStreamICM_t GdipCreateBitmapFromStreamICM;
	GdipGetImageWidth_t GdipGetImageWidth;
	GdipGetImageHeight_t GdipGetImageHeight;
	GdipGetImagePixelFormat_t GdipGetImagePixelFormat;
	GdipBitmapLockBits_t GdipBitmapLockBits;
	GdipBitmapUnlockBits_t GdipBitmapUnlockBits;
	GdipDisposeImage_t GdipDisposeImage;
	GdipImageGetFrameCount_t GdipImageGetFrameCount;
	GdipImageSelectActiveFrame_t GdipImageSelectActiveFrame;
	GdipGetPropertyItemSize_t GdipGetPropertyItemSize;
	GdipGetPropertyItem_t GdipGetPropertyItem;
	GdipImageRotateFlip_t GdipImageRotateFlip;
	GdipGetImageRawFormat_t GdipGetImageRawFormat;
	GdipGetImageFlags_t GdipGetImageFlags;
	GdipGetImagePalette_t GdipGetImagePalette;
	GdipGetImagePaletteSize_t GdipGetImagePaletteSize;
	GdipCreateFromHDC_t GdipCreateFromHDC;
	GdipDeleteGraphics_t GdipDeleteGraphics;
	GdipDrawImageRectRectI_t GdipDrawImageRectRectI;
	GdipCreateBitmapFromScan0_t GdipCreateBitmapFromScan0;
	GdipFillRectangleI_t GdipFillRectangleI;
	GdipCreateSolidFill_t GdipCreateSolidFill;
	GdipDeleteBrush_t GdipDeleteBrush;
	//GdipCloneImage_t GdipCloneImage;
	GdipCloneBitmapAreaI_t GdipCloneBitmapAreaI;
	GdipSetImagePalette_t GdipSetImagePalette;
	
	GDIPlusDecoder() {
		hGDIPlus = NULL; gdiplusToken = NULL; bTokenInitialized = false;
		nErrNumber = 0; nLastError = 0; bUseICM = false; bCoInitialized = FALSE; bCMYK2RGB = false; bRotateOnExif = true;
		pszPluginKey = NULL; bCancelled = FALSE; bForceSelfDisplay = false;
	}

	void ReloadConfig()
	{
		PVDSettings set(pszPluginKey);
		
		bool bDefault;
		
		bDefault = true;
		set.GetParam(L"RotateOnExif", L"bool;Rotate on EXIF",
			REG_BINARY, &bDefault, &bRotateOnExif, sizeof(bRotateOnExif));
		
		bDefault = false;
		set.GetParam(L"UseICM", L"bool;Use ICM when possible",
			REG_BINARY, &bDefault, &bUseICM, sizeof(bUseICM));
			
		bDefault = false;
		set.GetParam(L"ForceSelfDisplay", L"bool;Forced self display",
			REG_BINARY, &bDefault, &bForceSelfDisplay, sizeof(bForceSelfDisplay));

		bDefault = false;
		// Что-то в GDI+ CMYK изображения всегда загружаются сразу как PixelFormat24bppRGB
		//set.GetParam(L"CMYK2RGB", L"bool;Convert CMYK to RGB internally", REG_BINARY, &bDefault, &bCMYK2RGB, sizeof(bCMYK2RGB));
	}

	bool Init(pvdInitPlugin2* pInit)
	{
		bool result = false;
		nErrNumber = 0;

		pszPluginKey = pInit->pRegKey;

		ReloadConfig();

		HRESULT hrCoInitialized = CoInitialize(NULL);
		bCoInitialized = SUCCEEDED(hrCoInitialized);

		wchar_t FullPath[MAX_PATH*2+15]; FullPath[0] = 0;
		if (ghModule)
		{
			PVDSettings::FindFile(L"GdiPlus.dll", FullPath, sizeofarray(FullPath));
			hGDIPlus = LoadLibraryW(FullPath);
		}
		if (!hGDIPlus)
			hGDIPlus = LoadLibraryW(L"GdiPlus.dll");

		if (!hGDIPlus) {
			nErrNumber = PGE_DLL_NOT_FOUND;

		} else {
			DllGetFunction(hGDIPlus, GdiplusStartup);
			DllGetFunction(hGDIPlus, GdiplusShutdown);
			DllGetFunction(hGDIPlus, GdipCreateBitmapFromFile);
			DllGetFunction(hGDIPlus, GdipCreateBitmapFromFileICM);
			DllGetFunction(hGDIPlus, GdipCreateBitmapFromStream);
			DllGetFunction(hGDIPlus, GdipCreateBitmapFromStreamICM);
			DllGetFunction(hGDIPlus, GdipGetImageWidth);
			DllGetFunction(hGDIPlus, GdipGetImageHeight);
			DllGetFunction(hGDIPlus, GdipGetImagePixelFormat);
			DllGetFunction(hGDIPlus, GdipBitmapLockBits);
			DllGetFunction(hGDIPlus, GdipBitmapUnlockBits);
			DllGetFunction(hGDIPlus, GdipDisposeImage);
			DllGetFunction(hGDIPlus, GdipImageGetFrameCount);
			DllGetFunction(hGDIPlus, GdipImageSelectActiveFrame);
			DllGetFunction(hGDIPlus, GdipGetPropertyItemSize);
			DllGetFunction(hGDIPlus, GdipGetPropertyItem);
			DllGetFunction(hGDIPlus, GdipImageRotateFlip);
			DllGetFunction(hGDIPlus, GdipGetImageRawFormat);
			DllGetFunction(hGDIPlus, GdipGetImageFlags);
			DllGetFunction(hGDIPlus, GdipGetImagePalette);
			DllGetFunction(hGDIPlus, GdipGetImagePaletteSize);
			DllGetFunction(hGDIPlus, GdipCreateFromHDC);
			DllGetFunction(hGDIPlus, GdipDeleteGraphics);
			DllGetFunction(hGDIPlus, GdipDrawImageRectRectI);
			DllGetFunction(hGDIPlus, GdipCreateBitmapFromScan0);
			DllGetFunction(hGDIPlus, GdipFillRectangleI);
			DllGetFunction(hGDIPlus, GdipCreateSolidFill);
			DllGetFunction(hGDIPlus, GdipDeleteBrush);
			DllGetFunction(hGDIPlus, GdipCloneBitmapAreaI);
			DllGetFunction(hGDIPlus, GdipSetImagePalette);

			if (!GdipCreateBitmapFromFileICM && !GdipCreateBitmapFromStreamICM)
				bUseICM = false;

			if (GdiplusStartup && GdiplusShutdown && GdipCreateBitmapFromFile && GdipGetImageWidth && GdipGetImageHeight && GdipGetImagePixelFormat && GdipBitmapLockBits && GdipBitmapUnlockBits && GdipDisposeImage && GdipImageGetFrameCount && GdipImageSelectActiveFrame 
				&& GdipGetPropertyItemSize && GdipGetPropertyItem && GdipGetImageFlags && GdipImageRotateFlip
				&& GdipGetImagePalette && GdipGetImagePaletteSize && GdipCloneBitmapAreaI
				&& GdipCreateFromHDC && GdipDeleteGraphics && GdipDrawImageRectRectI
				&& GdipCreateBitmapFromScan0 && GdipFillRectangleI && GdipCreateSolidFill && GdipDeleteBrush
				&& GdipSetImagePalette)
			{
				__try{
					Gdiplus::GdiplusStartupInput gdiplusStartupInput;
					Gdiplus::Status lRc = GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
					result = (lRc == Gdiplus::Ok);
					if (!result) {
						nLastError = GetLastError();
						GdiplusShutdown(gdiplusToken); bTokenInitialized = false;
						nErrNumber = PGE_ERROR_BASE + (DWORD)lRc;
					} else {
						bTokenInitialized = true;
					}
				}__except( EXCEPTION_EXECUTE_HANDLER ){
					if (ip.MessageLog) ip.MessageLog(ip.pCallbackContext, L"Exception in GdiplusStartup", 2);
					nErrNumber = PGE_EXCEPTION;
				}
			} else {
				nErrNumber = PGE_FUNCTION_NOT_FOUND;
			}
			if (!result)
				FreeLibrary(hGDIPlus);
		}
		if (result)
			pInit->pContext = this;
		return result;
	};

	void Close()
	{
		__try{
			if (bTokenInitialized) {
				GdiplusShutdown(gdiplusToken);
				bTokenInitialized = false;
			}
		} __except( EXCEPTION_EXECUTE_HANDLER ) {
			if (ip.MessageLog) ip.MessageLog(ip.pCallbackContext, L"Exception in GdiplusShutdown", 2);
			nErrNumber = PGE_EXCEPTION;
		}
		if (hGDIPlus) {
			FreeLibrary(hGDIPlus);
			hGDIPlus = NULL;
		}

		if (bCoInitialized) {
			bCoInitialized = FALSE;
			CoUninitialize();
		}
	};
};

#define DEFINE_GUID_(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
	EXTERN_C const GUID DECLSPEC_SELECTANY name \
	= { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }

DEFINE_GUID_(FrameDimensionTime, 0x6aedbd6d,0x3fb5,0x418a,0x83,0xa6,0x7f,0x45,0x22,0x9d,0xc8,0x72);
DEFINE_GUID_(FrameDimensionPage, 0x7462dc86,0x6180,0x4c7e,0x8e,0x3f,0xee,0x73,0x33,0xa7,0xa4,0x83);

struct GDIPlusImage;

struct GDIPlusPage
{
	struct GDIPlusImage *img;
	bool bBitmapData;
	UINT nActivePage;
	Gdiplus::BitmapData  bmd;
};

struct GDIPlusImage
{
#ifdef _DEBUG
	wchar_t szFileName[MAX_PATH];
#endif
	GDIPlusDecoder *gdi;
	Gdiplus::GpBitmap *img;
	int nRefCount;
	Gdiplus::PropertyItem *pFrameTimesProp;
	Gdiplus::PropertyItem *pTransparent;
	MStream *strm;
	HRESULT nErrNumber, nLastError;
	//
	UINT lWidth, lHeight, pf, nBPP, nPages, lFrameTime, nActivePage, nTransparent, nImgFlags;
	bool Animation;
	bool bRotated; // наверное нужно только один раз выполнять
	wchar_t FormatName[0x80];

	void AddRef() {
		_ASSERTE(nRefCount>=0);
		nRefCount ++;
	};

	struct GDIPlusImage* Clone()
	{
		struct GDIPlusImage* newimg = (struct GDIPlusImage*)CALLOC(sizeof(struct GDIPlusImage));
		if (!newimg) {
			nErrNumber = PGE_NOT_ENOUGH_MEMORY;
			return NULL;
		}
#ifdef _DEBUG
		lstrcpyn(newimg->szFileName, szFileName, MAX_PATH);
#endif

		Gdiplus::Status lRc = gdi->GdipCloneBitmapAreaI(0,0,lWidth,lHeight, pf, img, &newimg->img);
		if (lRc) {
			nErrNumber = PGE_ERROR_BASE+(int)lRc;
			FREE(newimg);
			return NULL;
		}

		
		newimg->gdi = gdi;
		//pFrameTimesProp;
		//pTransparent;
		newimg->lWidth = lWidth;
		newimg->lHeight = lHeight;
		newimg->pf = pf;
		newimg->nBPP = nBPP;
		newimg->nPages = 1;
		newimg->lFrameTime = lFrameTime;
		newimg->nActivePage = 0;
		newimg->nTransparent = nTransparent;
		newimg->nImgFlags = nImgFlags;
		newimg->Animation = Animation;

		newimg->AddRef();
		return newimg;
	};

	Gdiplus::GpBitmap* OpenBitmapFromStream(MStream* strm)
	{
		Gdiplus::Status lRc = Gdiplus::Ok;
		Gdiplus::GpBitmap *img = NULL;
		__try {
			if (gdi->bUseICM) {
				if (gdi->GdipCreateBitmapFromStreamICM)
					lRc = gdi->GdipCreateBitmapFromStreamICM(strm, &img);
				else
					gdi->bUseICM = false;
			}
			if (!gdi->bUseICM || !lRc || !img) {
				LARGE_INTEGER dlibMove; dlibMove.QuadPart = 0;
				strm->Seek(dlibMove, STREAM_SEEK_SET, NULL);
				lRc = gdi->GdipCreateBitmapFromStream(strm, &img);
			}
			if (!img) {
				nLastError = GetLastError();
				nErrNumber = PGE_ERROR_BASE + (DWORD)lRc;
			}
		} __except( EXCEPTION_EXECUTE_HANDLER ) {
			if (ip.MessageLog) ip.MessageLog(ip.pCallbackContext, L"Exception in GdipCreateBitmapFromStream", 2);
			nErrNumber = PGE_EXCEPTION;
		}
		return img;
	}
	Gdiplus::GpBitmap* OpenBitmapFromFile(const wchar_t *pFileName)
	{
		Gdiplus::Status lRc = Gdiplus::Ok;
		Gdiplus::GpBitmap *img = NULL;
		__try {
			if (gdi->bUseICM) {
				if (gdi->GdipCreateBitmapFromFileICM)
					lRc = gdi->GdipCreateBitmapFromFileICM(pFileName, &img);
				else
					gdi->bUseICM = false;
			}
			if (!gdi->bUseICM || !lRc || !img) {
				lRc = gdi->GdipCreateBitmapFromFile(pFileName, &img);
			}
			if (!img) {
				nLastError = GetLastError();
				nErrNumber = PGE_ERROR_BASE + (DWORD)lRc;
			}
		} __except( EXCEPTION_EXECUTE_HANDLER ) {
			if (ip.MessageLog) ip.MessageLog(ip.pCallbackContext, L"Exception in GdipCreateBitmapFromFile", 2);
			nErrNumber = PGE_EXCEPTION;
		}
		return img;
	}

	Gdiplus::GpBitmap* OpenBitmap(const wchar_t *pFileName, i64 lFileSize, const u8 *pBuffer, u32 lBuffer)
	{
		_ASSERTE(nRefCount>=0);
		Gdiplus::Status lRc = Gdiplus::Ok;
		Gdiplus::GpBitmap *img = NULL;

		nErrNumber = PGE_FILE_NOT_FOUND;

		#ifdef _DEBUG
		lstrcpyn(szFileName, pFileName, MAX_PATH);
		#endif
		
		// Практика показывает, что для GDI+ предпочтительнее работать с файлами. Меньше глюков.
		if (lFileSize && pFileName && *pFileName) {
			img = OpenBitmapFromFile(pFileName);
			//TODO("Обязательно выставить флажок FILE_REQUIRED");
			if (img)
				return img;
		}

		if (gdi->bCoInitialized && pBuffer && lBuffer && (!lFileSize || (lFileSize == (i64)lBuffer))) {
			strm = new MStream();
			// Если нужно создать копию памяти - делать Write & Seek
			// Если выделять память не нужно (использовать ссылку на pBuffer) - SetData
			// Сейчас PicView не хранит открытый буфер
			nLastError = strm->Write(pBuffer, lBuffer, NULL);
			//strm->SetData(pBuffer, lBuffer);
			// Если вдруг памяти не хватило - попробуем
			if (nLastError == S_OK) {
				LARGE_INTEGER ll; ll.QuadPart = 0;
				strm->Seek(ll, STREAM_SEEK_SET, NULL);
				img = OpenBitmapFromStream(strm);
				if (img)
					return img;
			} else {
				nErrNumber = PGE_ERROR_BASE + (DWORD)Gdiplus::Win32Error;
			}
		}

		return NULL;
	};

	bool Open(const wchar_t *pFileName, i64 lFileSize, const u8 *pBuffer, u32 lBuffer)
	{
		_ASSERTE(nRefCount>=0);
		_ASSERTE(img == NULL);
		_ASSERTE(gdi != NULL);

		bool result = false;
		Gdiplus::Status lRc;

		nActivePage = -1; nTransparent = -1; nImgFlags = 0;
		img = OpenBitmap(pFileName, lFileSize, pBuffer, lBuffer);

		if (!img) {
			//nErrNumber = gdi->nErrNumber; -- ошибка УЖЕ в nErrNumber

		} else {
			lRc = gdi->GdipGetImageWidth(img, &lWidth);
			lRc = gdi->GdipGetImageHeight(img, &lHeight);
			lRc = gdi->GdipGetImagePixelFormat(img, (Gdiplus::PixelFormat*)&pf);
			nBPP = pf >> 8 & 0xFF;

			lRc = gdi->GdipGetImageFlags(img, &nImgFlags);
			
			if ((pf & PixelFormatIndexed) == PixelFormatIndexed) {
				//nTransparent
				UINT lPropSize;
				if (!(lRc = gdi->GdipGetPropertyItemSize(img, 0x5104/*PropertyTagIndexTransparent*/, &lPropSize)))
				{
					pTransparent = (Gdiplus::PropertyItem*)CALLOC(lPropSize);
					if ((lRc = gdi->GdipGetPropertyItem(img, 0x5104/*PropertyTagIndexTransparent*/, lPropSize, pTransparent)))
					{
						FREE(pTransparent);
						pTransparent = NULL;
					}
				}
			}

			Animation = false; nPages = 1;
			if (!(lRc = gdi->GdipImageGetFrameCount(img, &FrameDimensionTime, &nPages)))
				Animation = nPages > 1;
			else
				if ((lRc = gdi->GdipImageGetFrameCount(img, &FrameDimensionPage, &nPages)))
					nPages = 1;

			pFrameTimesProp = NULL;
			if (Animation)
			{
				UINT lPropSize;
				if (!(lRc = gdi->GdipGetPropertyItemSize(img, PropertyTagFrameDelay, &lPropSize)))
					if (pFrameTimesProp = (Gdiplus::PropertyItem*)CALLOC(lPropSize))
						if ((lRc = gdi->GdipGetPropertyItem(img, PropertyTagFrameDelay, lPropSize, pFrameTimesProp)))
						{
							FREE(pFrameTimesProp);
							pFrameTimesProp = NULL;
						}
			}

			FormatName[0] = 0;
			if (gdi->GdipGetImageRawFormat)
			{
				const wchar_t Format[][5] = {L"BMP", L"EMF", L"WMF", L"JPEG", L"PNG", L"GIF", L"TIFF", L"EXIF", L"", L"", L"ICO"};
				GUID gformat;
				if (!(lRc = gdi->GdipGetImageRawFormat(img, &gformat))) {
					// DEFINE_GUID(ImageFormatUndefined, 0xb96b3ca9,0x0728,0x11d3,0x9d,0x7b,0x00,0x00,0xf8,0x1e,0xf3,0x2e);
					// DEFINE_GUID(ImageFormatMemoryBMP, 0xb96b3caa,0x0728,0x11d3,0x9d,0x7b,0x00,0x00,0xf8,0x1e,0xf3,0x2e);

					if (gformat.Data1 >= 0xB96B3CAB && gformat.Data1 <= 0xB96B3CB5) {
						lstrcpy(FormatName, Format[gformat.Data1 - 0xB96B3CAB]);
					}
				}
			}

			result = SelectPage(0);
		}
		return result;
	};
	bool GetExifTagValueAsInt(PROPID pid, int& nValue)
	{
		bool bExists = false;
		Gdiplus::Status lRc;
		
		nValue = 0;
		
		UINT lPropSize;
		if (!(lRc = gdi->GdipGetPropertyItemSize(img, pid, &lPropSize)))
		{
			Gdiplus::PropertyItem* p = (Gdiplus::PropertyItem*)CALLOC(lPropSize);
			if (!(lRc = gdi->GdipGetPropertyItem(img, pid, lPropSize, p)))
			{
				switch (p->type) {
					case PropertyTagTypeByte:
						nValue = *(BYTE*)p->value; bExists = true;
						break;
					case PropertyTagTypeShort:
						nValue = *(short*)p->value; bExists = true;
						break;
					case PropertyTagTypeLong: case PropertyTagTypeSLONG:
						nValue = *(int*)p->value; bExists = true;
						break;
				}
			}
			FREE(p);
		}
		
		return bExists;
	};
	bool SelectPage(UINT iPage)
	{
		/*if (iPage == nActivePage)
			return true;
		if ((int)iPage < 0 || iPage >= nPages)
			return false;*/

		bool result = false;
		Gdiplus::Status lRc;
		__try{
			if ((lRc = gdi->GdipImageGetFrameCount(img, &FrameDimensionTime, &pf)))
				lRc = gdi->GdipImageSelectActiveFrame(img, &FrameDimensionPage, iPage);
			else
				lRc = gdi->GdipImageSelectActiveFrame(img, &FrameDimensionTime, iPage);
			
			if (gdi->bRotateOnExif && !bRotated) {
				bRotated = true;
				int nOrient;
				if (GetExifTagValueAsInt(PropertyTagOrientation, nOrient)) {
					// Теперь - крутим
					Gdiplus::RotateFlipType rft = Gdiplus::RotateNoneFlipNone;
					switch (nOrient) {
				        case 3: rft = Gdiplus::Rotate180FlipNone; break;
				        case 6: rft = Gdiplus::Rotate90FlipNone; break;
				        case 8: rft = Gdiplus::Rotate270FlipNone; break;
				        case 2: rft = Gdiplus::RotateNoneFlipX; break;
				        case 4: rft = Gdiplus::RotateNoneFlipY; break;
				        case 5: rft = Gdiplus::Rotate90FlipX; break;
				        case 7: rft = Gdiplus::Rotate270FlipX; break;
					}
					if (rft) {
						lRc = gdi->GdipImageRotateFlip(img, rft);
						// Размеры обновятся ниже
					}
				}
			}
				
			lRc = gdi->GdipGetImageWidth(img, &lWidth);
			lRc = gdi->GdipGetImageHeight(img, &lHeight);
			lRc = gdi->GdipGetImagePixelFormat(img, (Gdiplus::PixelFormat*)&pf);

			nBPP = pf >> 8 & 0xFF;

			//BUGBUG
			lFrameTime = 0;
			if (pFrameTimesProp) {
				lFrameTime = (pFrameTimesProp->length >= (iPage + 1) * 4)
					? (((u32*)pFrameTimesProp->value)[iPage] * 10) : 100;
				if (lFrameTime == 0)
					lFrameTime = 100;
			}

			nActivePage = iPage;
			result = true;
		} __except( EXCEPTION_EXECUTE_HANDLER ) {
			if (ip.MessageLog) ip.MessageLog(ip.pCallbackContext, L"Exception in GdipImageGetFrameCount...", 2);
			nErrNumber = PGE_EXCEPTION;
		}

		return result;
	};
	void Close()
	{
		if (!gdi) return;
		nRefCount--;
		if (nRefCount > 0)
			return;

		Gdiplus::Status lRc;
		__try{
			if (pFrameTimesProp) {
				//delete[] (u8*)pFrameTimesProp;
				FREE(pFrameTimesProp);
				pFrameTimesProp = NULL;
			}
			if (img) {
				lRc = gdi->GdipDisposeImage(img);
				img = NULL;
			}
			if (strm) {
				delete strm;
				strm = NULL;
			}
		} __except( EXCEPTION_EXECUTE_HANDLER ) {
			if (ip.MessageLog) ip.MessageLog(ip.pCallbackContext, L"Exception in GdipDisposeImage", 2);
			nErrNumber = PGE_EXCEPTION;
		}

		__try {
			FREE(this);
		} __except( EXCEPTION_EXECUTE_HANDLER ) {
			if (ip.MessageLog) ip.MessageLog(ip.pCallbackContext, L"Exception in FREE(this)", 2);
			nErrNumber = PGE_EXCEPTION;
		}
	};
	bool GetPageBits(pvdInfoDecode2 *pDecodeInfo)
	{
		bool result = false;
		
		__try{
			Gdiplus::Status lRc;
			Gdiplus::GpRect rect(0, 0, lWidth, lHeight);

			GDIPlusPage *pPage = (GDIPlusPage*)CALLOC(sizeof(GDIPlusPage));
			pPage->img = this;
			pPage->nActivePage = nActivePage;

			Gdiplus::BitmapData *bmd = pPage ? &(pPage->bmd) : NULL;
			BOOL lbAsDisplay = FALSE;
			if (pDecodeInfo->Flags & PVD_IDF_ASDISPLAY)
				lbAsDisplay = TRUE;
			else if (gdi->bForceSelfDisplay && !(pDecodeInfo->Flags & PVD_IDF_COMPAT_MODE))
				lbAsDisplay = TRUE;
			

			if (!bmd) {
				nErrNumber = PGE_NOT_ENOUGH_MEMORY;
			} else if (lbAsDisplay) {
				// GDI+ используется как дисплей
				result = true;
				pPage->bBitmapData = false;
				BOOL lbHasAlpha = (nImgFlags & Gdiplus::ImageFlagsHasAlpha) == Gdiplus::ImageFlagsHasAlpha;
				pDecodeInfo->pPalette = NULL;
				pDecodeInfo->Flags = PVD_IDF_READONLY | PVD_IDF_PRIVATE_DISPLAY;
				pDecodeInfo->ColorModel = PVD_CM_BGR;
				if (lbHasAlpha) {
					pDecodeInfo->Flags |= PVD_IDF_ALPHA;
					pDecodeInfo->ColorModel = PVD_CM_BGRA;
				}
				pDecodeInfo->lWidth = lWidth;
				pDecodeInfo->lHeight = lHeight;
				pDecodeInfo->lImagePitch = 0;
				pDecodeInfo->pImage = NULL;
				pDecodeInfo->lParam = (LPARAM)pPage;
				
			} else if (SelectPage(nActivePage)) {
				TODO("прозрачные 8-битные GIF отрисовывются на белом фоне");
				BOOL lbHasAlpha = (nImgFlags & Gdiplus::ImageFlagsHasAlpha) == Gdiplus::ImageFlagsHasAlpha;
				pDecodeInfo->pPalette = NULL;
				pDecodeInfo->Flags = PVD_IDF_READONLY;
				pDecodeInfo->ColorModel = PVD_CM_BGR;
				TODO("PixelFormat32bppCMYK ???");
				// К сожалению, GDI+ декодирует CMYK СРАЗУ как YCbCrK, что не дает возможности отдать данные в CMYK
				if (pf == PixelFormat1bppIndexed || pf == PixelFormat4bppIndexed || pf == PixelFormat8bppIndexed)
				{
					INT cbPalette = 0;
					lRc = gdi->GdipGetImagePaletteSize(img, (INT*)&cbPalette);
					if (lRc == Gdiplus::Ok) {
						Gdiplus::ColorPalette* pPalette = (Gdiplus::ColorPalette*)CALLOC(cbPalette);
						lRc = gdi->GdipGetImagePalette(img, pPalette, cbPalette);
						if (lRc == Gdiplus::Ok) {
							_ASSERTE(pPalette->Count);
							pDecodeInfo->pPalette = (UINT32*)CALLOC(pPalette->Count*sizeof(UINT32));
							lbHasAlpha = (pPalette->Flags & Gdiplus::PaletteFlagsHasAlpha) == Gdiplus::PaletteFlagsHasAlpha;
							_ASSERTE(cbPalette > (INT)(pPalette->Count*sizeof(UINT32)));
							memmove(pDecodeInfo->pPalette, pPalette->Entries, pPalette->Count*sizeof(UINT32));
							if (lbHasAlpha) {
								UINT32 n, nIdx = 0, nCount = 0;
								// Найти в палитре прозрачный цвет
								for (n = 0; n < pPalette->Count; n++) {
									if ((pPalette->Entries[n] & 0xFF000000) != 0xFF000000) {
										if (nCount == 0) nIdx = n;
										nCount ++;
									}
								}
								if (nCount == 0) {
									lbHasAlpha = FALSE;
								} else if (nCount == 1 && (pPalette->Entries[nIdx] & 0xFF000000) == 0) {
									lbHasAlpha = FALSE; pDecodeInfo->Flags |= PVD_IDF_TRANSPARENT_INDEX;
									pDecodeInfo->nTransparentColor = nIdx;
								} else if (nCount > 0) {
									lbHasAlpha = TRUE;
								}
							}
							FREE(pPalette);
						}
					}

					lRc = gdi->GdipBitmapLockBits(img, &rect, Gdiplus::ImageLockModeRead, pf, bmd);
					if (pf == PixelFormat1bppIndexed)
						pDecodeInfo->nBPP = 1;
					else if (pf == PixelFormat4bppIndexed)
						pDecodeInfo->nBPP = 4;
					else if (pf == PixelFormat8bppIndexed)
						pDecodeInfo->nBPP = 8;

				} else {
					lRc = gdi->GdipBitmapLockBits(img, &rect, Gdiplus::ImageLockModeRead,
						lbHasAlpha ? PixelFormat32bppARGB : PixelFormat32bppRGB, bmd);
					if (lRc == 0) {
						pDecodeInfo->nBPP = 32;
					} else
					if (lRc /* == NotEnoughMemory */ && pf == PixelFormat24bppRGB) {
						lRc = gdi->GdipBitmapLockBits(img, &rect, Gdiplus::ImageLockModeRead,
							PixelFormat24bppRGB, bmd);
						pDecodeInfo->nBPP = 24;
					}
				}
				if (lRc != Gdiplus::Ok) {
					nLastError = GetLastError();
					nErrNumber = PGE_ERROR_BASE + (DWORD)lRc;
				} else {
					result = true;
					pPage->bBitmapData = true;
					if (lbHasAlpha) {
						pDecodeInfo->Flags |= PVD_IDF_ALPHA;
						pDecodeInfo->ColorModel = PVD_CM_BGRA;
					}
					pDecodeInfo->lWidth = lWidth;
					pDecodeInfo->lHeight = lHeight;
					pDecodeInfo->lImagePitch = bmd->Stride;
					pDecodeInfo->pImage = (LPBYTE)bmd->Scan0;
					pDecodeInfo->lParam = (LPARAM)pPage;
				}
			}
		} __except( EXCEPTION_EXECUTE_HANDLER ) {
			if (ip.MessageLog) ip.MessageLog(ip.pCallbackContext, L"Exception in GetPageBits", 2);
			nErrNumber = PGE_EXCEPTION;
		}

		return result;
	};
	void FreePageBits(pvdInfoDecode2 *pDecodeInfo)
	{
		if (!pDecodeInfo || !pDecodeInfo->lParam) return;
		__try{
			GDIPlusPage *pPage = (GDIPlusPage*)pDecodeInfo->lParam;
			if (pPage->bBitmapData)
				gdi->GdipBitmapUnlockBits(img, &(pPage->bmd));

			FREE(pPage);

			if (pDecodeInfo->pPalette)
				FREE(pDecodeInfo->pPalette);
		} __except( EXCEPTION_EXECUTE_HANDLER ) {
			if (ip.MessageLog) ip.MessageLog(ip.pCallbackContext, L"Exception in FreePageBits", 2);
			nErrNumber = PGE_EXCEPTION;
		}
	}
};





UINT32 __stdcall pvdInit2(pvdInitPlugin2* pInit)
{
	_ASSERTE(pInit->cbSize >= sizeof(pvdInitPlugin2));
	if (pInit->cbSize < sizeof(pvdInitPlugin2)) {
		pInit->nErrNumber = PGE_OLD_PICVIEW;
		return 0;
	}

	memset(&ip,0,sizeof(ip));
	memmove(&ip, pInit, min(sizeof(ip),pInit->cbSize));
	ghModule = ip.hModule;

	GDIPlusDecoder *pDecoder = (GDIPlusDecoder*) CALLOC(sizeof(GDIPlusDecoder));
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

	pFormats->pActive = L"BMP,DIB,EMF,WMF,JPG,JPE,JPEG,PNG,GIF,TIF,TIFF,EXIF";
	pFormats->pInactive = L"ICO";
	pFormats->pForbidden = L"";
}

void __stdcall pvdExit2(void *pContext)
{
	if (pContext) {
		GDIPlusDecoder *pDecoder = (GDIPlusDecoder*)pContext;
		pDecoder->Close();

		FREE(pContext);
	}
}

void __stdcall pvdPluginInfo2(pvdInfoPlugin2 *pPluginInfo)
{
	_ASSERTE(pPluginInfo->cbSize >= sizeof(pvdInfoPlugin2));
	pPluginInfo->Priority = 0x200;
	pPluginInfo->pName = L"GDI+";
	pPluginInfo->pVersion = GetVersion(pPluginInfo->hModule);
	pPluginInfo->pComments = L"Copyright © 2009 Maximus5";
	pPluginInfo->Flags = PVD_IP_DECODE|PVD_IP_DISPLAY;
}

BOOL __stdcall pvdFileOpen2(void *pContext, const wchar_t *pFileName, INT64 lFileSize, const BYTE *pBuf, UINT32 lBuf, pvdInfoImage2 *pImageInfo)
{
	_ASSERTE(pImageInfo->cbSize >= sizeof(pvdInfoImage2));
	if (!pContext) {
		pImageInfo->nErrNumber = PGE_INVALID_CONTEXT;
		return FALSE;
	}

	GDIPlusImage *pImage = (GDIPlusImage*)CALLOC(sizeof(GDIPlusImage));
	if (!pImage) {
		pImageInfo->nErrNumber = PGE_NOT_ENOUGH_MEMORY;
		return FALSE;
	}

	pImage->gdi = (GDIPlusDecoder*)pContext;

	if (!pImage->Open(pFileName, lFileSize, pBuf, lBuf)) {
		pImageInfo->nErrNumber = pImage->nErrNumber;
		pvdFileClose2(pContext, pImage);
		return FALSE;
	}

	pImage->AddRef();

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

	GDIPlusImage* pImage = (GDIPlusImage*)pImageContext;
	// zero based
	if (!pImage->SelectPage(pPageInfo->iPage)) {
		pPageInfo->nErrNumber = pImage->nErrNumber;
		return FALSE;
	}

	pPageInfo->lWidth = pImage->lWidth;
	_ASSERTE(pImage->lHeight>0);
	pPageInfo->lHeight = pImage->lHeight;
	pPageInfo->nBPP = pImage->nBPP;
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

	GDIPlusImage* pImage = (GDIPlusImage*)pImageContext;
	if (!pImage->SelectPage(pDecodeInfo->iPage))
		return FALSE;

	BOOL lbRc = FALSE;
	
	lbRc = pImage->GetPageBits(pDecodeInfo);

	return lbRc;
}

void __stdcall pvdPageFree2(void *pContext, void *pImageContext, pvdInfoDecode2 *pDecodeInfo)
{
	_ASSERTE(pDecodeInfo->cbSize >= sizeof(pvdInfoDecode2));
	GDIPlusImage* pImage = (GDIPlusImage*)pImageContext;
	pImage->FreePageBits(pDecodeInfo);
}

void __stdcall pvdFileClose2(void *pContext, void *pImageContext)
{
	if (pImageContext) {
		((GDIPlusImage*)pImageContext)->Close();
	}
}

void __stdcall pvdReloadConfig2(void *pContext)
{
	GDIPlusDecoder *pDecoder = (GDIPlusDecoder*)pContext;
	if (pDecoder) {
		pDecoder->ReloadConfig();
	}
}



// Инициализация контекста дисплея. Используется тот pContext, который был получен в pvdInit2
BOOL __stdcall pvdDisplayInit2(void *pContext, pvdInfoDisplayInit2* pDisplayInit)
{
	GDIPlusDecoder *pDecoder = (GDIPlusDecoder*)pContext;
	if (pDecoder) {
		pDecoder->bAsDisplay = TRUE;
		#ifdef _DEBUG
		DebugString(L"GDI+ display initialized\n");
		#endif
		return TRUE;
	}
	pDisplayInit->nErrNumber = PGE_INVALID_CONTEXT;
	return FALSE;
}

BOOL __stdcall pvdDisplayAttach2(void *pContext, pvdInfoDisplayAttach2* pDisplayAttach)
{
#ifdef _DEBUG
	wchar_t szDbg[128];
	if (pDisplayAttach->bAttach)
		wsprintf(szDbg, L"Attaching GDI+ display to 0x%08X\n", (DWORD)pDisplayAttach->hWnd);
	else
		lstrcpy(szDbg, L"DEtaching GDI+ display\n");
	DebugString(szDbg);
#endif
	return TRUE;
}

typedef struct tag_GdiDrawBitmap
{
#ifdef _DEBUG
	wchar_t szFileName[MAX_PATH];
	UINT32 iPage;
#endif
	struct GDIPlusImage *img; // исходное изображение
	Gdiplus::GpBitmap* bitmap;
	LPBYTE pData;
	DWORD nLastZoom;
	BOOL  b100decoded;
	Gdiplus::ColorPalette pal;
	DWORD pal_entries[255];
} GdiDrawBitmap;

#define Abs(i) ((i < 0) ? -(i) : (i))

// Создать контекст для отображения картинки в pContext (перенос декодированных данных в видеопамять)
BOOL __stdcall pvdDisplayCreate2(void *pContext, pvdInfoDisplayCreate2* pDisplayCreate)
{
	_ASSERTE(pContext);
	GDIPlusDecoder* gdi = (GDIPlusDecoder*)pContext;

	if (pDisplayCreate->pImage->Flags & PVD_IDF_PRIVATE_DISPLAY)
	{
		GDIPlusPage *pPage = (GDIPlusPage*)pDisplayCreate->pImage->lParam;
		_ASSERTE(pPage);
		if (!pPage) {
			pDisplayCreate->nErrNumber = PGE_INVALID_PAGEDATA;
			return FALSE;
		}
		if (!pPage->bBitmapData) {
			GdiDrawBitmap *p = (GdiDrawBitmap*)CALLOC(sizeof(GdiDrawBitmap));
			if (!p) {
				pDisplayCreate->nErrNumber = PGE_NOT_ENOUGH_MEMORY;
				return FALSE;
			}
			#ifdef _DEBUG
			lstrcpyn(p->szFileName, pPage->img->szFileName, MAX_PATH);
			#endif

			if (pPage->img->nPages == 1) {
				pPage->img->AddRef();

				p->img = pPage->img;
				p->bitmap = p->img->img;
			} else {
				p->img = pPage->img->Clone();
				p->bitmap = p->img->img;
			}

			pDisplayCreate->pDisplayContext = p;
			return TRUE;
		}
	}
	
	//if (pDisplayCreate->pImage->lImagePitch < 0) {
	//	//_ASSERTE(pDisplayCreate->pImage->lImagePitch > 0);
	//	pDisplayCreate->nErrNumber = PGE_UNKNOWN_COLORSPACE;
	//	return FALSE;
	//}

	BOOL bPalette = FALSE;
	Gdiplus::PixelFormat pf = 0;
	if (pDisplayCreate->pImage->nBPP == 24 && pDisplayCreate->pImage->ColorModel == PVD_CM_BGR) {
		pf = PixelFormat24bppRGB;
	} else if (pDisplayCreate->pImage->nBPP == 32 && pDisplayCreate->pImage->ColorModel == PVD_CM_BGR) {
		pf = PixelFormat32bppRGB;
	} else if (pDisplayCreate->pImage->nBPP == 32 && pDisplayCreate->pImage->ColorModel == PVD_CM_BGRA) {
		pf = PixelFormat32bppARGB;
	} else if (pDisplayCreate->pImage->nBPP == 32 && pDisplayCreate->pImage->ColorModel == PVD_CM_CMYK) {
		TODO("CMYK переделать на нашу коррекцию?");
		pf = PixelFormat32bppCMYK;
	} else if (pDisplayCreate->pImage->nBPP == 16 && pDisplayCreate->pImage->ColorModel == PVD_CM_BGR) {
		pf = PixelFormat16bppRGB555;
	} else if (pDisplayCreate->pImage->nBPP == 1 && pDisplayCreate->pImage->pPalette) {
		pf = PixelFormat1bppIndexed; bPalette = TRUE;
	} else if (pDisplayCreate->pImage->nBPP == 4 && pDisplayCreate->pImage->pPalette) {
		pf = PixelFormat4bppIndexed; bPalette = TRUE;
	} else if (pDisplayCreate->pImage->nBPP == 8 && pDisplayCreate->pImage->pPalette) {
		pf = PixelFormat8bppIndexed; bPalette = TRUE;

	} else {
		//_ASSERT(FALSE);
		pDisplayCreate->nErrNumber = PGE_UNKNOWN_COLORSPACE;
		return FALSE;
	}
	//_ASSERTE(pDisplayCreate->pImage->pPalette == NULL);
	//*pDisplayCreate->ppDisplayContext = (LPVOID)1;
	Gdiplus::GpStatus stat;
	WARNING("!!! По возможности не создавать копию изображения, а использовать существующий GDIPlusImage с его img");
	WARNING("!!! Если в img более одного Page - делать Clone");
	WARNING("!!! Если в img более одного Page перед Clone убедиться, что выбрана нужная страница (nActivePage)");
	//Gdiplus::GpBitmap* bitmap = NULL;
	GdiDrawBitmap *p = (GdiDrawBitmap*)CALLOC(sizeof(GdiDrawBitmap));
	if (!p) {
		pDisplayCreate->nErrNumber = PGE_NOT_ENOUGH_MEMORY;
		return FALSE;
	}

	int lImagePitch = pDisplayCreate->pImage->lImagePitch;
	INT lGdiStride = (((pDisplayCreate->pImage->lWidth+31)>>5)<<5)*3;
	if (lGdiStride != Abs(pDisplayCreate->pImage->lImagePitch)) {
		// В расчете на 32 BPP
		lGdiStride = (((pDisplayCreate->pImage->lWidth+31)>>5)<<7);
	}
	_ASSERTE(Abs(pDisplayCreate->pImage->lImagePitch) <= lGdiStride);
	size_t nSrcSize = Abs(lImagePitch) * pDisplayCreate->pImage->lHeight;
	size_t nSize = lGdiStride*pDisplayCreate->pImage->lHeight;
	p->pData = (LPBYTE)CALLOC(nSize);
	if (!p->pData) {
		FREE(p);
		pDisplayCreate->nErrNumber = PGE_NOT_ENOUGH_MEMORY;
		return FALSE;
	}

	LPBYTE pStride0 = p->pData;

	TODO("Хорошо бы сначала попробовать не выполнять наши функции преобразования. Может GDI+ сам справится");
	_ASSERTE(nSize >= nSrcSize);
	if (nSize >= nSrcSize && !bPalette) {
		if (pDisplayCreate->pImage->pPalette && ip.MessageLog)
			ip.MessageLog(ip.pCallbackContext, L"pImage->pPalette will be ignored", 1);

		// декодеру GDI+ исходная память требуется постоянно. придется копировать
		memmove(p->pData, pDisplayCreate->pImage->pImage, nSrcSize);

		if (lImagePitch < 0) {
			pStride0 -= lImagePitch * (pDisplayCreate->pImage->lHeight - 1);
		}

		stat = gdi->GdipCreateBitmapFromScan0(
			pDisplayCreate->pImage->lWidth, pDisplayCreate->pImage->lHeight,
			lImagePitch, pf,
			pStride0, &p->bitmap);
		if (!stat) {
			if (pDisplayCreate->pImage->nBPP <= 8 && pDisplayCreate->pImage->pPalette)
			{
				p->pal.Flags = 0; // PaletteFlagsHasAlpha | PaletteFlagsGrayScale | PaletteFlagsHalftone
				p->pal.Count = 1 << pDisplayCreate->pImage->nBPP;
				if (pDisplayCreate->pImage->nColorsUsed > 0 && pDisplayCreate->pImage->nColorsUsed < p->pal.Count)
					p->pal.Count = pDisplayCreate->pImage->nColorsUsed;
				for (UINT i=0; i<p->pal.Count; i++) {
					p->pal.Entries[i] = pDisplayCreate->pImage->pPalette[i];
				}
				stat = gdi->GdipSetImagePalette(p->bitmap, &p->pal);
			}

			goto lBmpCreated;
		}

		if (ip.MessageLog) {
			wchar_t szMsg[255];
			wsprintf(szMsg, L"GDI+ can't handle image creation (%ix%ix%i:%i). Code=",
				pDisplayCreate->pImage->lWidth, pDisplayCreate->pImage->lHeight,
				pDisplayCreate->pImage->nBPP, (int)pDisplayCreate->pImage->ColorModel);
			pvdTranslateError2(PGE_ERROR_BASE+(int)stat, szMsg+lstrlen(szMsg), 128);
			lstrcat(szMsg, L". Using BlitFunction");

			ip.MessageLog(ip.pCallbackContext, szMsg, 1);
		}
	}

	pStride0 = p->pData;
	/*if (lGdiStride == Abs(pDisplayCreate->pImage->lImagePitch)) {
		if (pDisplayCreate->pImage->pPalette && ip.MessageLog)
			ip.MessageLog(ip.pCallbackContext, L"pImage->pPalette will be ignored", 1);

		memmove(p->pData, pDisplayCreate->pImage->pImage, nSize);
	} else */
	{
		bool lbBltRc = true, lbMayContinue = true;
		BOOL lbAlpha = FALSE, lbTransColor = FALSE, lbTransIndex = FALSE;
		INT lGdiStride2 = (pDisplayCreate->pImage->lImagePitch < 0) ? -lGdiStride : lGdiStride;
		const void* pPalette = pDisplayCreate->pImage->pPalette;
		BltHelper::BlitFunction_t BlitFunction;
		BlitFunction = BltHelper::BlitChoose(32, pDisplayCreate->pImage->nBPP,
			Abs(pDisplayCreate->pImage->lImagePitch), lGdiStride2, lbAlpha, lbTransColor, lbTransIndex,
			lbBltRc, lbMayContinue, pPalette, pDisplayCreate->pImage->ColorModel);
		if (!lbBltRc || !BlitFunction) {
			pDisplayCreate->nErrNumber = PGE_UNSUPPORTED_PITCH;
			FREE(p->pData);
			FREE(p);
			return FALSE;
		}

		if (lImagePitch < 0) {
			pStride0 -= lGdiStride2 * (pDisplayCreate->pImage->lHeight - 1);
		}

		lbBltRc = BlitFunction(
			pStride0, pDisplayCreate->pImage->pImage, 0, pDisplayCreate->pImage->lWidth, pDisplayCreate->pImage->lHeight,
			Abs(pDisplayCreate->pImage->lImagePitch), lGdiStride2, pDisplayCreate->BackColor,
			pDisplayCreate->pImage->Flags, pDisplayCreate->pImage->nTransparentColor, pPalette);
		if (!lbBltRc) {
			pDisplayCreate->nErrNumber = PGE_UNSUPPORTED_PITCH;
			FREE(p->pData);
			FREE(p);
			return FALSE;
		}

		pf = PixelFormat32bppRGB;
		lImagePitch = lGdiStride;
	}

	pStride0 = p->pData;
	if (lImagePitch < 0) {
		pStride0 -= pDisplayCreate->pImage->lImagePitch * (pDisplayCreate->pImage->lHeight - 1);
	}

	

	//PRAGMA_ERROR("Does not work"); // Она НЕ копирует данные, а ссылается на переданные... в итоге отрисовка падает, т.к. память уже освобождена
	stat = gdi->GdipCreateBitmapFromScan0(
		pDisplayCreate->pImage->lWidth, pDisplayCreate->pImage->lHeight,
		lImagePitch, pf,
		pStride0, &p->bitmap);
	if (stat) {
		FREE(p->pData);
		FREE(p);
		pDisplayCreate->nErrNumber = PGE_ERROR_BASE+(int)stat;
		return FALSE;
	}
lBmpCreated:
	pDisplayCreate->pDisplayContext = p;

	#ifdef _DEBUG
	lstrcpyn(p->szFileName, pDisplayCreate->pFileName, sizeofarray(p->szFileName));
	p->iPage = pDisplayCreate->iPage;
	#endif

	return TRUE;
}

BOOL CALLBACK DrawImageAbortCallback(GDIPlusDecoder *pGDI)
{
	if (pGDI)
		return pGDI->bCancelled;
	return FALSE;
}

typedef struct tag_GdiDrawContext
{
	PAINTSTRUCT pstruct;
	HDC hCompDC;
	HBITMAP hCompBMP, hOldBMP;
	RECT rcPaint;
	Gdiplus::GpGraphics *pGr;
} GdiDrawContext;

BOOL ColorFill(GDIPlusDecoder* gdi, GdiDrawContext *p, RECT ImgRect, DWORD nBackColor)
{
	BOOL lbRc = FALSE;
	Gdiplus::GpStatus stat;

	ImgRect.left = max(ImgRect.left-2,0); 
	ImgRect.top = max(ImgRect.top-2,0);
	ImgRect.bottom+=2;
	ImgRect.right+=2;

	Gdiplus::GpSolidFill *pBr = NULL;
	DWORD nBGR = 0xFF000000 | BGRA_FROM_RGBA(nBackColor);

	stat = gdi->GdipCreateSolidFill(nBGR, &pBr);

	if (!stat) {
		stat = gdi->GdipFillRectangleI(p->pGr, pBr, ImgRect.left, ImgRect.top, ImgRect.right-ImgRect.left, ImgRect.bottom-ImgRect.top);
		gdi->GdipDeleteBrush(pBr);
	}

	lbRc = (stat == Gdiplus::Ok);
	return lbRc;
}

// Собственно отрисовка. Функция должна при необходимости выполнять "Stretch"
BOOL __stdcall pvdDisplayPaint2(void *pContext, void* pDisplayContext, pvdInfoDisplayPaint2* pDisplayPaint)
{
	_ASSERTE(pDisplayPaint->cbSize >= sizeof(pvdInfoDisplayPaint2));
	_ASSERTE(pContext);
	_ASSERTE(pDisplayContext);

	GDIPlusDecoder* gdi = (GDIPlusDecoder*)pContext;

	BOOL lbRc = FALSE;
	Gdiplus::GpStatus stat;
	GdiDrawContext *p = (GdiDrawContext*)pDisplayPaint->pDrawContext;
	GdiDrawBitmap* pb = (GdiDrawBitmap*)pDisplayContext;

	#ifdef _DEBUG
	wchar_t szDbg[512];
	#endif


	switch (pDisplayPaint->Operation)
	{
	case PVD_IDP_BEGIN:
		{
			#ifdef _DEBUG
			wsprintf(szDbg, L"PVD_IDP_BEGIN %s:(%i)...", pb->szFileName, pb->iPage);
			PaintDebugString(szDbg);
			#endif

			p = (GdiDrawContext*)CALLOC(sizeof(GdiDrawContext));
			if (!p) {
				pDisplayPaint->nErrNumber = PGE_NOT_ENOUGH_MEMORY;
				return FALSE;
			}
			//if (!BeginPaint(pDisplayPaint->hWnd, &p->pstruct)) {
			if (!(p->pstruct.hdc = GetDC(pDisplayPaint->hWnd))) {
				gnLastWin32Error = GetLastError();
				pDisplayPaint->nErrNumber = PGE_WIN32_ERROR;
				FREE(p);
				return FALSE;
			}

			TODO("Возможно, для ускорения отрисовки вообще не создавать CompatibleDC если картинка НЕ прозрачная");
			p->hCompDC = CreateCompatibleDC(p->pstruct.hdc);
			if (p->hCompDC)
			{
				GetClientRect(pDisplayPaint->hWnd, &p->rcPaint);
				p->hCompBMP = (HBITMAP)CreateCompatibleBitmap(p->pstruct.hdc, p->rcPaint.right, p->rcPaint.bottom);
				p->hOldBMP = (HBITMAP)SelectObject(p->hCompDC, p->hCompBMP);
			}

			stat = gdi->GdipCreateFromHDC(p->hCompDC ? p->hCompDC : p->pstruct.hdc, &p->pGr);
			if (stat) {
				pDisplayPaint->nErrNumber = PGE_ERROR_BASE+(int)stat;
				return FALSE;
			}
			pDisplayPaint->pDrawContext = p;

			if (p->hCompDC) {
				lbRc = ColorFill(gdi, p, p->rcPaint, pDisplayPaint->nBackColor);
			}

			#ifdef _DEBUG
			PaintDebugString(L"OK\n");
			#endif

			lbRc = TRUE;
		} break;
	case PVD_IDP_COMMIT:
		{
			#ifdef _DEBUG
			wsprintf(szDbg, L"PVD_IDP_COMMIT %s:(%i)...", pb->szFileName, pb->iPage);
			PaintDebugString(szDbg);
			#endif

			if (p->hCompDC) {
				lbRc = BitBlt(p->pstruct.hdc, 0,0,p->rcPaint.right,p->rcPaint.bottom,
					p->hCompDC,0,0,SRCCOPY);
				if (!lbRc) {
					gnLastWin32Error = GetLastError();
					pDisplayPaint->nErrNumber = PGE_WIN32_ERROR;
				}
			} else {
				lbRc = TRUE; // все уже отрисовано
			}
			stat = gdi->GdipDeleteGraphics(p->pGr);
			SelectObject(p->hCompDC, p->hOldBMP);
			DeleteObject(p->hCompBMP);
			DeleteDC(p->hCompDC);
			//EndPaint(pDisplayPaint->hWnd, &p->pstruct);
			ReleaseDC(pDisplayPaint->hWnd, p->pstruct.hdc);
			FREE(p);
			pDisplayPaint->pDrawContext = NULL;

			#ifdef _DEBUG
			PaintDebugString(L"OK\n");
			#endif

		} break;
	case PVD_IDP_COLORFILL:
		{
			if (p->hCompDC) {
				lbRc = TRUE; // заливка идет один раз на весь CompatibleDC
			} else {
				lbRc = ColorFill(gdi, p, pDisplayPaint->DisplayRect, pDisplayPaint->nBackColor);
			}
		} break;
	case PVD_IDP_PAINT:
		{
			_ASSERTE(pb!=NULL && pb->bitmap!=(Gdiplus::GpBitmap *)1);
			gdi->bCancelled = FALSE;
			
			#ifdef _DEBUG
			wsprintf(szDbg, L"PVD_IDP_PAINT %s:(%i)\n    {{%i-%i},{%i-%i}} -> {{%i-%i},{%i-%i}} Zoom=%i...",
				pb->szFileName, pb->iPage,
				pDisplayPaint->ImageRect.left, pDisplayPaint->ImageRect.top,
				pDisplayPaint->ImageRect.right, pDisplayPaint->ImageRect.bottom,
				pDisplayPaint->DisplayRect.left, pDisplayPaint->DisplayRect.top,
				pDisplayPaint->DisplayRect.right, pDisplayPaint->DisplayRect.bottom,
				(DWORD)(pDisplayPaint->nZoom/655)
				);
			PaintDebugString(szDbg);
			#endif

			if (pb->nLastZoom && pb->nLastZoom != pDisplayPaint->nZoom && !pb->b100decoded
				/*
				&& !(pDisplayPaint->nFlags & (PVD_IDPF_ZOOMING|PVD_IDPF_PANNING))
				*/
				)
			{
				// Нужно вызвать полное декодирование изображения
				// Lock - нельзя, т.к. начинает тормозить вообще все по черному
				//Gdiplus::GpRect rect(0, 0, pb->img->lWidth, pb->img->lHeight);
				//Gdiplus::BitmapData bmd;
				//stat = gdi->GdipBitmapLockBits(pb->bitmap, &rect, Gdiplus::ImageLockModeRead, pb->img->pf, &bmd);
				//if (!stat) gdi->GdipBitmapUnlockBits(pb->bitmap, &bmd);
				pb->b100decoded = TRUE;
				
				HDC hCompDC = CreateCompatibleDC(p->pstruct.hdc);
				_ASSERTE(hCompDC);
				if (hCompDC)
				{
					DebugString(L"Starning 'decoding' in 100%...");
					Gdiplus::GpGraphics *pGr = NULL;
					int nShowWidth  = min(pDisplayPaint->DisplayRect.right-pDisplayPaint->DisplayRect.left,
					                      pDisplayPaint->ImageRect.right-pDisplayPaint->ImageRect.left);
					int nShowHeight = min(pDisplayPaint->DisplayRect.bottom-pDisplayPaint->DisplayRect.top,
					                      pDisplayPaint->ImageRect.bottom-pDisplayPaint->ImageRect.top);
					HBITMAP hCompBMP = (HBITMAP)CreateCompatibleBitmap(p->pstruct.hdc, nShowWidth, nShowHeight);
					HBITMAP hOldBMP = (HBITMAP)SelectObject(hCompDC, hCompBMP);
					
					stat = gdi->GdipCreateFromHDC(hCompDC, &pGr);
					if (!stat) {
						stat = gdi->GdipDrawImageRectRectI(
							pGr, pb->bitmap,
							0, 0, nShowWidth, nShowHeight,
							pDisplayPaint->ImageRect.left, pDisplayPaint->ImageRect.top,
							nShowWidth, nShowHeight,
							Gdiplus::UnitPixel, NULL, //NULL, NULL);
							(Gdiplus::DrawImageAbort)DrawImageAbortCallback, gdi);
						gdi->GdipDeleteGraphics(pGr);
					}
					
					SelectObject(hCompDC, hOldBMP);
					DeleteObject(hCompBMP);
					DeleteDC(hCompDC);
					
					DebugString(L"Done...\n");
				}

				PaintDebugString(L"Creating p->pGr...");
				stat = gdi->GdipCreateFromHDC(p->hCompDC ? p->hCompDC : p->pstruct.hdc, &p->pGr);
			}
			pb->nLastZoom = pDisplayPaint->nZoom;
			
			PaintDebugString(L"Drawing pb->bitmap in p->pGr...");

			WARNING("Если падает - то следующее обращение к bitmap вызовет ObjectBusy");
			stat = gdi->GdipDrawImageRectRectI(
				p->pGr, pb->bitmap,
				pDisplayPaint->DisplayRect.left, pDisplayPaint->DisplayRect.top,
				pDisplayPaint->DisplayRect.right-pDisplayPaint->DisplayRect.left,
				pDisplayPaint->DisplayRect.bottom-pDisplayPaint->DisplayRect.top,
				pDisplayPaint->ImageRect.left, pDisplayPaint->ImageRect.top,
				pDisplayPaint->ImageRect.right-pDisplayPaint->ImageRect.left,
				pDisplayPaint->ImageRect.bottom-pDisplayPaint->ImageRect.top,
				Gdiplus::UnitPixel, NULL, //NULL, NULL);
				(Gdiplus::DrawImageAbort)DrawImageAbortCallback, gdi);
			lbRc = stat == Gdiplus::Ok;
			if (!lbRc)
				pDisplayPaint->nErrNumber = PGE_ERROR_BASE+(int)stat;
		}
	}

	return lbRc;
}

// Закрыть контекст для отображения картинки (освободить видеопамять)
void __stdcall pvdDisplayClose2(void *pContext, void* pDisplayContext)
{
	_ASSERTE(pContext);
	GDIPlusDecoder* gdi = (GDIPlusDecoder*)pContext;
	if (gdi && pDisplayContext) {
		GdiDrawBitmap* p = (GdiDrawBitmap*)pDisplayContext;

		#ifdef _DEBUG
		wchar_t szDbg[512];
		wsprintf(szDbg, L"GDI+ closing display context for %s:(%i)\n", p->szFileName, p->iPage);
		DebugString(szDbg);
		#endif

		if (p->img) {
			p->img->Close();
		} else {
			if (p->bitmap) {
				gdi->GdipDisposeImage(p->bitmap);
			}
			if (p->pData) {
				FREE(p->pData);
			}
		}
		FREE(p);
	}
}

// Закрыть модуль вывода (освобождение интерфейсов DX, отцепиться от окна)
void __stdcall pvdDisplayExit2(void *pContext)
{
	GDIPlusDecoder *pDecoder = (GDIPlusDecoder*)pContext;
	if (pDecoder)
		pDecoder->bAsDisplay = FALSE;
	#ifdef _DEBUG
	DebugString(L"GDI+ display exited\n");
	#endif
}
