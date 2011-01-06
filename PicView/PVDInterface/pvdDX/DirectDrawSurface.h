#pragma once

/**************************************************************************
Copyright (c) 2009 Skakov Pavel
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

#include <ddraw.h>
//#include <vector>

// Некритичные ошибки ( < 0x7FFFFFFF )
// При подборе декодера в PicView нужно возвращать именно их ( < 0x7FFFFFFF )
// для тех файлов (форматов), которые неизвестны данному декодеру.
// PicView не будет отображать эти ошибки пользователю.

// Далее идут критичные ошибки. Информация об ошибке будет показана
// пользователю, если PicView не сможет открыть файл никаким декодером
#define PDXE_WIN32_ERROR              0x80000001
#define PDXE_ALREARY_INITIALIZED      0x80000002
#define PDXE_DLL_FAILED               0x80000003
#define PDXE_DIRECTDRAWCREATE_FAILED  0x80000004
#define PDXE_COOPERATIVELEVEL         0x80000005
#define PDXE_CREATE_PRIMARY_SURFACE   0x80000006
#define PDXE_CREATE_BACK_SURFACE      0x80000007
#define PDXE_CREATE_CLIPPER           0x80000008
#define PDXE_SET_CLIPPER              0x80000009
#define PDXE_CLIPPER_SETWND           0x8000000A
#define PDXE_CREATE_WORK_SURFACE      0x8000000B
#define PDXE_NO_DISPLAY_WINDOW        0x8000000C
#define PDXE_INIT2_WAS_NOT_CALLED     0x8000000D
#define PDXE_MEMORY_ALLOCATION_FAILED 0x8000000E
#define PDXE_INVALID_ARGUMENT         0x8000000F
#define PDXE_NOT_ENOUGH_VIDEO_RAM     0x80000010
#define PDXE_OLD_DISPLAYCREATE        0x80000011
#define PDXE_INVALID_PAINTCONTEXT     0x80000012
#define PDXE_OLD_PICVIEW              0x80000013


typedef unsigned int uint;
typedef unsigned __int8 u8;
typedef DWORD u32;
struct pvdInfoDecode2;
class DirectDraw;
class DirectDrawSurface;

extern DirectDraw* gpDD;

class DirectDraw
{
public:
	HWND m_hWnd;
	SIZE msz_LastWndSize;
	BOOL mb_CoInitialized;
	bool mb_DebugDumps;
	bool mb_DebugFills;
	const wchar_t* pszPluginKey;
	
	LPDIRECTDRAW7 m_pDirectDraw;
	LPDIRECTDRAWSURFACE7 m_pPrimarySurface, m_pBackSurface;
	LPDIRECTDRAWCLIPPER m_pClipper;
	
	//std::vector<DirectDrawSurface*> mp_Images;

	typedef HRESULT (WINAPI* FDirectDrawCreate)( GUID FAR *lpGUID, LPDIRECTDRAW FAR *lplpDD, IUnknown FAR *pUnkOuter );
	FDirectDrawCreate fDirectDrawCreate;

	//DWORD nCMYKparts, *pCMYKpalette, nCMYKsize;
	//DWORD uCMYK2RGB; // PVD_CMYK2RGB_*;
	
	DWORD mn_LastError;
	
	BOOL mb_FirstMemCheck;
	DWORD GetAvailVidMem();
	void ReloadConfig();
	
public:
	DirectDraw(const wchar_t* aszPluginKey);
	~DirectDraw();
	
public:
	bool Init(pvdInfoDisplayInit2* pDisplayInit);
	void Free(void);
	bool SetHWnd(HWND hWnd/*, uint iManualStep = 0*/);
	bool CheckBackBuffer();
};

class DirectDrawSurface
{
public:
	DirectDraw* mp_DD;
	wchar_t *ms_FileName, *ms_DumpName;
	
	struct WORK_SRF_DESC
	{
		//ZeroMemory выполняется в CreateWorkSurface
		//WORK_SRF_DESC() { memset(this,0,sizeof(WORK_SRF_DESC)); };
		LPDIRECTDRAWSURFACE7 Surface;
		uint lWidth, lHeight;
		uint lStrideWidth;
		//u8 *pData;
	} *m_pWork;

	uint m_lWorkWidth, m_lWorkHeight;
	uint m_nWorkSurfaces, m_nWidthParts, m_nHeightParts;
	
	DWORD mn_LastError;

	DirectDrawSurface(DirectDraw *pDD, const wchar_t* aszFileName);
	~DirectDrawSurface();

	void DeleteWorkSurface(void);
	bool CreateWorkSurface(uint lWidth, uint lHeight, uint nBPP);
	bool ViewSurface(const RECT *pImgRect, const RECT *pDisplayRect, bool bWaitForVerticalBlank = true);
	//bool Blit(const void *pSource, int lSrcPitch, uint nBPP, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette=NULL);
	bool Blit(struct pvdInfoDecode2 *pDecodeInfo, COLORREF nBackground);

	void Dump(uint nNo, uint nWidth, uint nHeight, int nPitch, uint nBPP, LPCVOID pData, BOOL bMakeCopy=FALSE);

	//static void ApplyBlendFunction32(u8* pDst, size_t cbSize, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent);
	//static BYTE __inline BlendFunction(DWORD Src, DWORD Back, DWORD Alpha);
	//static DWORD __inline CMYK2BGR_APPROX(DWORD Src);
	//static DWORD __inline CMYK2BGR_PRECISE(DWORD Src);
	//static DWORD __inline CMYK2BGR_FAST(DWORD Src);

	//static bool __cdecl Blit32_CMYK(u8*& pDst, u8*& pSrc, const uint iSrcPoint, uint nPoints, uint lHeight, uint lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette);
	//static bool __cdecl Blit32_BGRA(u8*& pDst, u8*& pSrc, const uint iSrcPoint, uint nPoints, uint lHeight, uint lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette);
	//static bool __cdecl Blit32_BGRA_AT(u8*& pDst, u8*& pSrc, const uint iSrcPoint, uint nPoints, uint lHeight, uint lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette);
	//static bool __cdecl Blit32_BGRA_VP(u8*& pDst, u8*& pSrc, const uint iSrcPoint, uint nPoints, uint lHeight, uint lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette);
	//static bool __cdecl Blit32_BGRA_AT_VP(u8*& pDst, u8*& pSrc, const uint iSrcPoint, uint nPoints, uint lHeight, uint lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette);
	//static bool __cdecl Blit24_BGR_T(u8*& pDst, u8*& pSrc, const uint iSrcPoint, uint nPoints, uint lHeight, uint lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette);
	//static bool __cdecl Blit24_BGR(u8*& pDst, u8*& pSrc, const uint iSrcPoint, uint nPoints, uint lHeight, uint lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette);
	//static bool __cdecl Blit24_BGR_24(u8*& pDst, u8*& pSrc, const uint iSrcPoint, uint nPoints, uint lHeight, uint lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette);
	//static bool __cdecl Blit16_BGR_T(u8*& pDst, u8*& pSrc, const uint iSrcPoint, uint nPoints, uint lHeight, uint lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette);
	//static bool __cdecl Blit16_BGR(u8*& pDst, u8*& pSrc, const uint iSrcPoint, uint nPoints, uint lHeight, uint lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette);
	//static bool __cdecl Blit8_PAL(u8*& pDst, u8*& pSrc, const uint iSrcPoint, uint nPoints, uint lHeight, uint lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette);
	//static bool __cdecl Blit8_PAL_A(u8*& pDst, u8*& pSrc, const uint iSrcPoint, uint nPoints, uint lHeight, uint lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette);
	//static bool __cdecl Blit8_PAL_T(u8*& pDst, u8*& pSrc, const uint iSrcPoint, uint nPoints, uint lHeight, uint lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette);
	//static bool __cdecl Blit8_PAL_TI(u8*& pDst, u8*& pSrc, const uint iSrcPoint, uint nPoints, uint lHeight, uint lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette);
	//static bool __cdecl Blit4_PAL(u8*& pDst, u8*& pSrc, const uint iSrcPoint, uint nPoints, uint lHeight, uint lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette);
	//static bool __cdecl Blit4_PAL_A(u8*& pDst, u8*& pSrc, const uint iSrcPoint, uint nPoints, uint lHeight, uint lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette);
	//static bool __cdecl Blit4_PAL_T(u8*& pDst, u8*& pSrc, const uint iSrcPoint, uint nPoints, uint lHeight, uint lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette);
	//static bool __cdecl Blit4_PAL_TI(u8*& pDst, u8*& pSrc, const uint iSrcPoint, uint nPoints, uint lHeight, uint lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette);
	//static bool __cdecl Blit2_PAL(u8*& pDst, u8*& pSrc, const uint iSrcPoint, uint nPoints, uint lHeight, uint lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette);
	//static bool __cdecl Blit2_PAL_A(u8*& pDst, u8*& pSrc, const uint iSrcPoint, uint nPoints, uint lHeight, uint lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette);
	//static bool __cdecl Blit1_PAL(u8*& pDst, u8*& pSrc, const uint iSrcPoint, uint nPoints, uint lHeight, uint lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette);
	//static bool __cdecl Blit1_PAL_A(u8*& pDst, u8*& pSrc, const uint iSrcPoint, uint nPoints, uint lHeight, uint lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette);
	
	//typedef bool (__cdecl *BlitFunction_t)(u8*& pDst, u8*& pSrc, const uint iSrcPoint, uint nPoints, uint lHeight, uint lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette);
	//static BlitFunction_t BlitChoose(uint nDstBPP, uint nBPP, uint lAbsSrcPitch, int lDstPitch, BOOL lbAlpha, BOOL lbTransColor, BOOL lbTransIndex, bool &result, bool &bMayContinue, const void *&pPalette, pvdColorModel ColorModel);
};
