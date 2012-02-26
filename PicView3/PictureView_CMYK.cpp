
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

#include "PictureView.h"
#include "PictureView_Lang.h"
#include "PVDInterface/BltHelper.h"
#include <GdiPlus.h>


DWORD WINAPI CMYK_ThreadProc(void*)
{
	OutputDebugString(L"\n*** Loading CMYK.png...");
	ULONG_PTR gdiplusToken = 0; bool bTokenInitialized = false;
	BOOL bCoInitialized = FALSE;
	wchar_t* pszFileName = NULL; //ConcatPath(g_SelfPath, L"CMYK.png");
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	Gdiplus::Status lRc;
	Gdiplus::GpBitmap *img = NULL;
	UINT lWidth = 0, lHeight = 0, pf = 0, nBPP = 0;
	Gdiplus::BitmapData bmd; bool bBitsLocked = false;
	Gdiplus::GpRect rect(0, 0, 256, 256);

	typedef Gdiplus::Status (WINAPI *GdiplusStartup_t)(OUT ULONG_PTR *token, const Gdiplus::GdiplusStartupInput *input, OUT Gdiplus::GdiplusStartupOutput *output);
	typedef VOID (WINAPI *GdiplusShutdown_t)(ULONG_PTR token);
	typedef Gdiplus::GpStatus (WINGDIPAPI *GdipCreateBitmapFromFile_t)(GDIPCONST WCHAR* filename, Gdiplus::GpBitmap **bitmap);
	typedef Gdiplus::GpStatus (WINGDIPAPI *GdipGetImageWidth_t)(Gdiplus::GpImage *image, UINT *width);
	typedef Gdiplus::GpStatus (WINGDIPAPI *GdipGetImageHeight_t)(Gdiplus::GpImage *image, UINT *height);
	typedef Gdiplus::GpStatus (WINGDIPAPI *GdipGetImagePixelFormat_t)(Gdiplus::GpImage *image, Gdiplus::PixelFormat *format);
	typedef Gdiplus::GpStatus (WINGDIPAPI *GdipBitmapLockBits_t)(Gdiplus::GpBitmap* bitmap, GDIPCONST Gdiplus::GpRect* rect, UINT flags, Gdiplus::PixelFormat format, Gdiplus::BitmapData* lockedBitmapData);
	typedef Gdiplus::GpStatus (WINGDIPAPI *GdipBitmapUnlockBits_t)(Gdiplus::GpBitmap* bitmap, Gdiplus::BitmapData* lockedBitmapData);
	typedef Gdiplus::GpStatus (WINGDIPAPI *GdipDisposeImage_t)(Gdiplus::GpImage *image);

	#define DllGetFunction(hModule, FunctionName) FunctionName = (FunctionName##_t)GetProcAddress(hModule, #FunctionName)
	
	GdiplusStartup_t GdiplusStartup;
	GdiplusShutdown_t GdiplusShutdown;
	GdipCreateBitmapFromFile_t GdipCreateBitmapFromFile;
	GdipGetImageWidth_t GdipGetImageWidth;
	GdipGetImageHeight_t GdipGetImageHeight;
	GdipGetImagePixelFormat_t GdipGetImagePixelFormat;
	GdipBitmapLockBits_t GdipBitmapLockBits;
	GdipBitmapUnlockBits_t GdipBitmapUnlockBits;
	GdipDisposeImage_t GdipDisposeImage;


	//gs_CMYK_ErrorInfo[0] = 0;
	//g_Plugin.nCMYK_ErrorNumber = g_Plugin.nCMYK_LastError = 0;

	HRESULT hr = CoInitialize(NULL);
	if (!(bCoInitialized = SUCCEEDED(hr))) {
		//wsprintf(gs_CMYK_ErrorInfo, L"CoInitialize failed. Code=0x%08X", (DWORD)hr);
		g_Plugin.nCMYK_ErrorNumber = MICMYKCoInitializeFailed;
		g_Plugin.nCMYK_LastError = (DWORD)hr;
		goto wrap;
	}

	g_Plugin.hGDIPlus = LoadLibraryW(L"GdiPlus.dll");
	if (!g_Plugin.hGDIPlus) {
		//dwLastError = GetLastError();
		//wsprintf(gs_CMYK_ErrorInfo, L"LoadLibrary(GdiPlus.dll) failed. Code=0x%08X", (DWORD)hr);
		g_Plugin.nCMYK_ErrorNumber = MICMYKLoadLibraryFailed;
		g_Plugin.nCMYK_LastError = GetLastError();
		goto wrap;
	}

	DllGetFunction(g_Plugin.hGDIPlus, GdiplusStartup);
	DllGetFunction(g_Plugin.hGDIPlus, GdiplusShutdown);
	DllGetFunction(g_Plugin.hGDIPlus, GdipCreateBitmapFromFile);
	DllGetFunction(g_Plugin.hGDIPlus, GdipGetImageWidth);
	DllGetFunction(g_Plugin.hGDIPlus, GdipGetImageHeight);
	DllGetFunction(g_Plugin.hGDIPlus, GdipGetImagePixelFormat);
	DllGetFunction(g_Plugin.hGDIPlus, GdipBitmapLockBits);
	DllGetFunction(g_Plugin.hGDIPlus, GdipBitmapUnlockBits);
	DllGetFunction(g_Plugin.hGDIPlus, GdipDisposeImage);

	if (!GdiplusStartup || !GdiplusShutdown || !GdipCreateBitmapFromFile || !GdipGetImageWidth || !GdipGetImageHeight 
		|| !GdipGetImagePixelFormat || !GdipBitmapLockBits || !GdipBitmapUnlockBits || !GdipDisposeImage)
	{
		//lstrcpy(gs_CMYK_ErrorInfo, L"One of GdiPlus flat api functions not found!");
		g_Plugin.nCMYK_ErrorNumber = MICMYKBadGdiFlat;
		goto wrap;
	}

	bTokenInitialized = Gdiplus::Ok == (lRc = GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL));
	if (!bTokenInitialized) {
		//wsprintf(gs_CMYK_ErrorInfo, L"GdiplusStartup failed. Code=%i", (DWORD)lRc);
		g_Plugin.nCMYK_ErrorNumber = MICMYKGdipStartupFailed;
		g_Plugin.nCMYK_LastError = (DWORD)lRc;
		goto wrap;
	}

	pszFileName = ConcatPath(g_SelfPath, L"CMYK.png");

	if (Gdiplus::Ok != (lRc = GdipCreateBitmapFromFile(pszFileName, &img))) {
		//wsprintf(gs_CMYK_ErrorInfo, L"GdipCreateBitmapFromFile(CMYK.png) failed. Code=%i", (DWORD)lRc);
		g_Plugin.nCMYK_ErrorNumber = MICMYKCreateBitmapFailed;
		g_Plugin.nCMYK_LastError = (DWORD)lRc;
		goto wrap;
	}

	lRc = GdipGetImageWidth(img, &lWidth);
	lRc = GdipGetImageHeight(img, &lHeight);
	lRc = GdipGetImagePixelFormat(img, (Gdiplus::PixelFormat*)&pf);
	nBPP = pf >> 8 & 0xFF;

	if (lWidth != (16*16) || lHeight != (16*16)) {
		g_Plugin.nCMYK_ErrorNumber = MICMYKpaletteInvalidSize;
		goto wrap;
	}
	if (nBPP != 24 && nBPP != 32) {
		g_Plugin.nCMYK_ErrorNumber = MICMYKpaletteInvalidBPP;
		goto wrap;
	}

	rect.Width = lWidth; rect.Height = lHeight;

	if (Gdiplus::Ok != (lRc = GdipBitmapLockBits(img, &rect, Gdiplus::ImageLockModeRead, PixelFormat32bppRGB, &bmd))) {
		g_Plugin.nCMYK_ErrorNumber = MICMYKpaletteInvalidBPP;
		goto wrap;
	}
	bBitsLocked = true;



	g_Plugin.nCMYKparts = 17; // пока фиксировано
	g_Plugin.nCMYKsize = lWidth * lHeight;
	g_Plugin.pCMYKpalette = (DWORD*)calloc(g_Plugin.nCMYKsize,4);

	if (!g_Plugin.pCMYKpalette) {
		g_Plugin.nCMYK_ErrorNumber = MIMemoryAllocationFailed;
		goto wrap;

	} else {

		BYTE* pDst = (BYTE*)g_Plugin.pCMYKpalette;
		BYTE* pSrc = (BYTE*)bmd.Scan0;
		uint lAbsSrcPitch = 0;
		int lDstPitch = lWidth * 4;

		if (bmd.Stride < 0)
		{
			pDst += (int)(lHeight - 1) * lDstPitch;
			lAbsSrcPitch = -bmd.Stride;
			lDstPitch = -lDstPitch;
		} else {
			lAbsSrcPitch = bmd.Stride;
		}

		BltHelper::Blit32_BGRA(pDst, pSrc, 0, lWidth, lHeight, lAbsSrcPitch, lDstPitch, 0, 0, 0, 0);

		OutputDebugString(L"Succeeded.\n");
	}

	// OK

wrap:
	if (bBitsLocked) {
		GdipBitmapUnlockBits(img, &bmd); bBitsLocked = false;
	}
	if (img) {
		GdipDisposeImage(img); img = NULL;
	}
	if (pszFileName) {
		free(pszFileName); pszFileName = NULL;
	}
	if (bTokenInitialized) {
		GdiplusShutdown(gdiplusToken); bTokenInitialized = false;
	}
	//if (hGDIPlus) {
	//	FreeLibrary(hGDIPlus);
	//	hGDIPlus = NULL;
	//}
	//if (bCoInitialized)
	//	CoUninitialize();

	return 0;
}
