#pragma once

/**************************************************************************
Copyright (c) 2009 Maximus5
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


struct BltHelper
{
	static DWORD nCMYKparts, *pCMYKpalette, nCMYKsize, uCMYK2RGB;
	static DWORD nChessMateColor1, nChessMateColor2, nChessMateSize;

	static void ApplyBlendFunction32(BYTE* pDst, size_t cbSize, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent);
	static BYTE __inline BlendFunction(DWORD Src, DWORD Back, DWORD Alpha);
	static DWORD __inline CMYK2BGR_APPROX(DWORD Src);
	static DWORD __inline CMYK2BGR_PRECISE(DWORD Src);
	static DWORD __inline CMYK2BGR_FAST(DWORD Src);

	static bool __cdecl Blit32_CMYK(BYTE*& pDst, BYTE*& pSrc, const DWORD iSrcPoint, DWORD nPoints, DWORD lHeight, DWORD lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette);
	static bool __cdecl Blit32_BGRA(BYTE*& pDst, BYTE*& pSrc, const DWORD iSrcPoint, DWORD nPoints, DWORD lHeight, DWORD lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette);
	static bool __cdecl Blit32_BGRA_AT(BYTE*& pDst, BYTE*& pSrc, const DWORD iSrcPoint, DWORD nPoints, DWORD lHeight, DWORD lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette);
	static bool __cdecl Blit32_BGRA_VP(BYTE*& pDst, BYTE*& pSrc, const DWORD iSrcPoint, DWORD nPoints, DWORD lHeight, DWORD lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette);
	static bool __cdecl Blit32_BGRA_AT_VP(BYTE*& pDst, BYTE*& pSrc, const DWORD iSrcPoint, DWORD nPoints, DWORD lHeight, DWORD lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette);
	static bool __cdecl Blit24_BGR_T(BYTE*& pDst, BYTE*& pSrc, const DWORD iSrcPoint, DWORD nPoints, DWORD lHeight, DWORD lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette);
	static bool __cdecl Blit24_BGR(BYTE*& pDst, BYTE*& pSrc, const DWORD iSrcPoint, DWORD nPoints, DWORD lHeight, DWORD lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette);
	static bool __cdecl Blit24_BGR_24(BYTE*& pDst, BYTE*& pSrc, const DWORD iSrcPoint, DWORD nPoints, DWORD lHeight, DWORD lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette);
	static bool __cdecl Blit16_BGR_T(BYTE*& pDst, BYTE*& pSrc, const DWORD iSrcPoint, DWORD nPoints, DWORD lHeight, DWORD lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette);
	static bool __cdecl Blit16_BGR(BYTE*& pDst, BYTE*& pSrc, const DWORD iSrcPoint, DWORD nPoints, DWORD lHeight, DWORD lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette);
	static bool __cdecl Blit8_PAL(BYTE*& pDst, BYTE*& pSrc, const DWORD iSrcPoint, DWORD nPoints, DWORD lHeight, DWORD lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette);
	static bool __cdecl Blit8_PAL_A(BYTE*& pDst, BYTE*& pSrc, const DWORD iSrcPoint, DWORD nPoints, DWORD lHeight, DWORD lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette);
	static bool __cdecl Blit8_PAL_T(BYTE*& pDst, BYTE*& pSrc, const DWORD iSrcPoint, DWORD nPoints, DWORD lHeight, DWORD lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette);
	static bool __cdecl Blit8_PAL_TI(BYTE*& pDst, BYTE*& pSrc, const DWORD iSrcPoint, DWORD nPoints, DWORD lHeight, DWORD lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette);
	static bool __cdecl Blit4_PAL(BYTE*& pDst, BYTE*& pSrc, const DWORD iSrcPoint, DWORD nPoints, DWORD lHeight, DWORD lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette);
	static bool __cdecl Blit4_PAL_A(BYTE*& pDst, BYTE*& pSrc, const DWORD iSrcPoint, DWORD nPoints, DWORD lHeight, DWORD lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette);
	static bool __cdecl Blit4_PAL_T(BYTE*& pDst, BYTE*& pSrc, const DWORD iSrcPoint, DWORD nPoints, DWORD lHeight, DWORD lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette);
	static bool __cdecl Blit4_PAL_TI(BYTE*& pDst, BYTE*& pSrc, const DWORD iSrcPoint, DWORD nPoints, DWORD lHeight, DWORD lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette);
	static bool __cdecl Blit2_PAL(BYTE*& pDst, BYTE*& pSrc, const DWORD iSrcPoint, DWORD nPoints, DWORD lHeight, DWORD lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette);
	static bool __cdecl Blit2_PAL_A(BYTE*& pDst, BYTE*& pSrc, const DWORD iSrcPoint, DWORD nPoints, DWORD lHeight, DWORD lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette);
	static bool __cdecl Blit1_PAL(BYTE*& pDst, BYTE*& pSrc, const DWORD iSrcPoint, DWORD nPoints, DWORD lHeight, DWORD lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette);
	static bool __cdecl Blit1_PAL_A(BYTE*& pDst, BYTE*& pSrc, const DWORD iSrcPoint, DWORD nPoints, DWORD lHeight, DWORD lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette);
	
	typedef bool (__cdecl *BlitFunction_t)(BYTE*& pDst, BYTE*& pSrc, const DWORD iSrcPoint, DWORD nPoints, DWORD lHeight, DWORD lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette);
	static BlitFunction_t __cdecl BlitChoose(DWORD nDstBPP, DWORD nBPP, DWORD lAbsSrcPitch, int lDstPitch, BOOL lbAlpha, BOOL lbTransColor, BOOL lbTransIndex, bool &result, bool &bMayContinue, const void *&pPalette, pvdColorModel ColorModel);
};
