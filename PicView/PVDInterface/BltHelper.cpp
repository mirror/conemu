
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

#include <windows.h>
#include "PictureViewPlugin.h"
#include "BltHelper.h"

#ifdef _DEBUG
#ifndef WIN64
#include <crtdbg.h>
#else
#define _ASSERTE(a)
#endif
#else
	#ifndef _ASSERTE
		#define _ASSERTE(a)
	#endif
#endif

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




DWORD BltHelper::nCMYKparts = 0;
DWORD* BltHelper::pCMYKpalette = NULL;
DWORD BltHelper::nCMYKsize = 0;
DWORD BltHelper::uCMYK2RGB = 0; // PVD_CMYK2RGB_*;

DWORD BltHelper::nChessMateColor1 = 0xFFFFFF;
DWORD BltHelper::nChessMateColor2 = 0xCCCCCC;
DWORD BltHelper::nChessMateSize = 8;



DWORD BltHelper::CMYK2BGR_FAST(DWORD Src)
{
	if (!Src) // In CMYK images, pure white is generated when all four components have values of 0%.
		return 0xFFFFFF;
		
	DWORD C = (Src & 0xFF);
	DWORD M = (Src & 0xFF00) >> 8;
	DWORD Y = (Src & 0xFF0000) >> 16;
	DWORD K = (Src & 0xFF000000) >> 24;
	DWORD R,G,B;

	R = ((C + K) < 255) ? (255 - (C + K)) : 0;
	G = ((M + K) < 255) ? (255 - (M + K)) : 0;
	B = ((Y + K) < 255) ? (255 - (Y + K)) : 0;
	
	return (B) | (G << 8) | (R << 16);
}

DWORD BltHelper::CMYK2BGR_APPROX(DWORD Src)
{
	if (!Src) // In CMYK images, pure white is generated when all four components have values of 0%.
		return 0xFFFFFF;

	_ASSERTE(pCMYKpalette && nCMYKsize);

	DWORD C = (Src & 0xFF);
	DWORD C0 = C >> 4;
	DWORD M = (Src & 0xFF00) >> 8;
	DWORD M0 = M >> 4;
	DWORD Y = (Src & 0xFF0000) >> 16;
	DWORD Y0 = Y >> 4;
	DWORD K = (Src & 0xFF000000) >> 24;
	DWORD K0 = K >> 4;

	DWORD N = (C0 << 12) | (M0 << 8) | (Y0 << 4) | (K0);
	if (N >= nCMYKsize) {
		_ASSERTE(N < nCMYKsize);
		return 0;
	}

	return pCMYKpalette[N];
}

DWORD BltHelper::CMYK2BGR_PRECISE(DWORD Src)
{
	if (!Src) // In CMYK images, pure white is generated when all four components have values of 0%.
		return 0xFFFFFF;

	_ASSERTE(pCMYKpalette && nCMYKsize);

	DWORD C = (Src & 0xFF);
	DWORD C0 = C >> 4;
	DWORD M = (Src & 0xFF00) >> 8;
	DWORD M0 = M >> 4;
	DWORD Y = (Src & 0xFF0000) >> 16;
	DWORD Y0 = Y >> 4;
	DWORD K = (Src & 0xFF000000) >> 24;
	DWORD K0 = K >> 4;

	DWORD N = (C0 << 12) | (M0 << 8) | (Y0 << 4) | (K0);
	if (N >= nCMYKsize) {
		_ASSERTE(N < nCMYKsize);
		return 0;
	}

	DWORD RGB32 = pCMYKpalette[N];

	bool bCMYKpreciese = true;
	// Если хотят "точное" отображение цветов
	if (bCMYKpreciese) {
		DWORD C_ = (C0 << 4);
		DWORD C1 = (C0 == 0xF) ? 0xF : ((C_ == C) ? C0 : (C0+1));
		DWORD M_ = (M0 << 4);
		DWORD M1 = (M0 == 0xF) ? 0xF : ((M_ == M) ? M0 : (M0+1));
		DWORD Y_ = (Y0 << 4);
		DWORD Y1 = (Y0 == 0xF) ? 0xF : ((Y_ == Y) ? Y0 : (Y0+1));
		DWORD K_ = (K0 << 4);
		DWORD K1 = (K0 == 0xF) ? 0xF : ((K_ == K) ? K0 : (K0+1));

		N = (C1 << 12) | (M1 << 8) | (Y1 << 4) | (K1);
		if (N >= nCMYKsize) {
			_ASSERTE(N < nCMYKsize);
			return RGB32;
		}

		DWORD RGB32_1 = pCMYKpalette[N];

		_ASSERTE(nCMYKparts == 17);

		int R = RED_FROM_BGRA(RGB32);
			if (C > C_) {
				int R1 = RED_FROM_BGRA(RGB32_1);
				R += (R1 - R) * (int)(C - C_) / 17;
				if (R<0) R=0; else if (R>255) R=255;
			}
		int G = GREEN_FROM_BGRA(RGB32);
			if (M > M_) {
				int G1 = GREEN_FROM_BGRA(RGB32_1);
				G += (G1 - G) * (int)(M - M_) / 17;
				if (G<0) G=0; else if (G>255) G=255;
			}
		int B = BLUE_FROM_BGRA(RGB32);
			if (Y > Y_) {
				int B1 = BLUE_FROM_BGRA(RGB32_1);
				B += (B1 - B) * (int)(Y - Y_) / 17;
				if (B<0) B=0; else if (B>255) B=255;
			}

		RGB32 = 0xFF000000 | (((DWORD)R) << 16) | (((DWORD)G) << 8) | (((DWORD)B));
	}

	return RGB32;
}

// Нифига не работает. зависимости R/G/B меняются при различных C/M/Y/K
//DWORD __inline BltHelper::CMYK2BGR_FUNC(DWORD Src)
//{
//	if (!Src) // In CMYK images, pure white is generated when all four components have values of 0%.
//		return 0xFFFFFF;
//		
//	// For example, a bright red might contain 2% cyan, 93% magenta, 90% yellow, and 0% black
//		
//	register DWORD C0 = (Src & 0xFF), C = C0 << 8;
//	register DWORD M0 = (Src & 0xFF00) >> 8, M = (Src & 0xFF00);
//	register DWORD Y0 = (Src & 0xFF0000) >> 16, Y = (Src & 0xFF0000) >> 8;
//	register DWORD K0 = (Src & 0xFF000000) >> 24, K = (Src & 0xFF000000) >> 16;
//	register int R = 255, G = 255, B = 255;
//
//	// Функция от C примерно такая
//	if (C0) {
//		if (C0 < 115)
//			R -= C / 230;
//		else if (C0 < 166)
//			R -= (C / 165) - 50;
//		else if (C0 < 179)
//			R -= (C / 101) - 200;
//		else
//			R = 0;
//
//		if (C0 < 140)
//			G -= C / 680;
//		else
//			G -= (C >> 10) + 18;
//
//		B -= (C >> 12);
//	}
//
//	if (M0) {
//		R -= (M / 3350);
//
//		if (M0 < 179)   G -= M / 325;      		else
//		if (M0 < 166)   G -= (M >> 8) - 38; 	else
//		if (M0 != 255)  G -= (M / 110) - 340;
//		else            G = 0;
//
//		if (M0 < 140)	B -= M / 530;
//		else			B -= (M / 620) -10;
//	}
//
//	if (Y0) {
//		// R от Y не зависит
//
//		G -= (Y / 4600);
//
//		if (Y0 < 166)	B -= Y / 320;			else
//		if (Y0 < 230)	B -= (Y / 200) + 80;	else
//		if (Y0 < 245)	B -= (Y / 80) + 522;
//		else			B = 0;
//	}
//
//	if (K0) {
//		//R -= (K / 297);
//		if (!K0)		;						else
//		if (K0 < 77 )	R -= (K / 300) - 3;		else
//		if (K0 < 217)	R -= (K / 328) - 8;
//		else			R -= (K / 230) + 64;
//
//		//G -= (K / 292);
//		if (!K0)		;						else
//		if (K0 < 77)	G -= (K / 315) - 3;		else
//		if (K0 < 204)	G -= (K / 328) - 5;
//		else			G -= (K / 220) + 74;
//
//		//B -= (K / 293);
//		if (!K0)		;						else
//		if (K0 < 115)	B -= (K / 330) - 3;		else
//		if (K0 < 217)	B -= (K / 310) + 4;
//		else			B -= (K / 200) + 103;
//	}
//
//	//R = ((C + K) < 255) ? (255 - (C + K)) : 0;
//	//G = ((M + K) < 255) ? (255 - (M + K)) : 0;
//	//B = ((Y + K) < 255) ? (255 - (Y + K)) : 0;
//
//	if (R < 0) R = 0; else if (R > 255) R = 255;
//	if (G < 0) G = 0; else if (G > 255) G = 255;
//	if (B < 0) B = 0; else if (B > 255) B = 255;
//	
//	return ((DWORD)B) | (((DWORD)G) << 8) | (((DWORD)R) << 16);
//}


BYTE __inline BltHelper::BlendFunction(DWORD Src, DWORD Back, DWORD Alpha)
{
	DWORD nRc = Src * Alpha >> 8;
	if (Back)
		nRc += Back - (Back * Alpha >> 8);
	return (nRc > 255) ? 255 : (BYTE)nRc;
}

void BltHelper::ApplyBlendFunction32(BYTE* pDst, size_t cbSize, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent)
{
	// Глючит на прозрачности (alpha канал)
	if ((idfFlags & PVD_IDF_ALPHA) == PVD_IDF_ALPHA) {
		RGBQUAD *pStart = (RGBQUAD*)pDst, *pEnd = (RGBQUAD*)(pDst + cbSize);
		//RGBQUAD Back; *((DWORD*)&Back) = nBackground;
		DWORD nAlpha = 0, nValue = 0, nNewValue = 0;
		RGBQUAD Dst;
		DWORD nCount = cbSize >> 2;
		//#define RED(x) ((x & 0xFF))
		//#define GREEN(x) ((x & 0xFF00) >> 8)
		//#define BLUE(x) ((x & 0xFF0000) >> 16)
		//#define ALPHA(x) (x & 0xFF)
		DWORD nBackR = RED_FROM_BGRA(nBackground); //Back.rgbRed;
		DWORD nBackG = GREEN_FROM_BGRA(nBackground); //Back.rgbGreen;
		DWORD nBackB = BLUE_FROM_BGRA(nBackground); //Back.rgbBlue;
		//nBackground = (nBackB) | (nBackG << 8) | (nBackR << 16) /*| 0xFF00000000*/;
		
		// Попробовать пооптимизировать - сколько времени через
		// поля структуры, а сколько через маски и сдвиги

		//nBackground |= 0xFF000000;
		// Старший байт вообще не используем
		//nBackground &= 0xFFFFFF; -- МЛАДШИЙ а не старший байт это альфа канал

		/*while (nCount--) {
			nValue = *pStart;
			nAlpha = ALPHA(nValue);
			if (!nAlpha) {
				nNewValue = nBackground;
			} else if (nAlpha != 0xFF) {
				nNewValue = 0xFF;
				/ *nNewValue |= BlendFunction(RED(nValue), nBackR, nAlpha) << 24;
				nNewValue |= BlendFunction(GREEN(nValue), nBackR, nAlpha) << 16;
				nNewValue |= BlendFunction(BLUE(nValue), nBackR, nAlpha) << 8;* /
				nNewValue |= nValue;
			}
			*(pStart++) = nNewValue;
		}*/

		//nCount = 0;
		
		for (pStart = (RGBQUAD*)pDst; pStart < pEnd; pStart++) {
		//for (DWORD n = 0; n < cbSize; n++) {
			nAlpha = pStart->rgbReserved; //0;//ALPHA(*pStart);
			//nAlpha = (*((DWORD*)pStart)) & 0xFF000000;
			if (!nAlpha) {
				*((DWORD*)pStart) = nBackground;
			} else if (nAlpha != 0xFF) {
			//} else if (nAlpha != 0xFF000000) {
				//nAlpha = nAlpha >> 24;
				// Тут тоже попробовать через сдвиги а не через поля структуры
				Dst.rgbBlue = BlendFunction(pStart->rgbBlue, nBackB, nAlpha);
				Dst.rgbGreen = BlendFunction(pStart->rgbGreen, nBackG, nAlpha);
				Dst.rgbRed = BlendFunction(pStart->rgbRed, nBackR, nAlpha);
				//Dst.rgbReserved = 0xFF; -- не требуется
				*pStart = Dst;
			}
		}
	} else if ((idfFlags & PVD_IDF_TRANSPARENT) == PVD_IDF_TRANSPARENT) {
		DWORD *pStart = (DWORD*)pDst, *pEnd = (DWORD*)(pDst + cbSize);
		for (; pStart < pEnd; pStart++) {
			if (*pStart == nTransparent)
				*pStart = nBackground;
		}
	}
	//else {
		// т.к. при создании Surface мы не заказывали альфа канал - содержимое старшего
		// байта просто игнорируется
		////DWORD *pEnd = (DWORD*)(pDst + cbSize);
		////for (DWORD *pStart = (DWORD*)pDst; pStart < pEnd; pStart++) *pStart |= 0xFF000000;
	//}
}


bool BltHelper::Blit32_CMYK(BYTE*& pDst, BYTE*& pSrc, const DWORD iSrcPoint, DWORD nPoints, DWORD lHeight, DWORD lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette)
{
	pSrc += iSrcPoint*4;

	if (!pCMYKpalette || !nCMYKsize || uCMYK2RGB == PVD_CMYK2RGB_FAST) {
		for (DWORD j = lHeight; j--; pSrc += lAbsSrcPitch, pDst += lDstPitch) {
			for (DWORD i = nPoints; i--;)
				((DWORD*)pDst)[i] = CMYK2BGR_FAST(((DWORD*)pSrc)[i]);
		}
	} else if (uCMYK2RGB == PVD_CMYK2RGB_PRECISE) {
		for (DWORD j = lHeight; j--; pSrc += lAbsSrcPitch, pDst += lDstPitch) {
			for (DWORD i = nPoints; i--;)
				((DWORD*)pDst)[i] = CMYK2BGR_PRECISE(((DWORD*)pSrc)[i]);
		}
	} else {
		for (DWORD j = lHeight; j--; pSrc += lAbsSrcPitch, pDst += lDstPitch) {
			for (DWORD i = nPoints; i--;)
				((DWORD*)pDst)[i] = CMYK2BGR_APPROX(((DWORD*)pSrc)[i]);
		}
	}

	return true;
}

bool BltHelper::Blit32_BGRA(BYTE*& pDst, BYTE*& pSrc, const DWORD iSrcPoint, DWORD nPoints, DWORD lHeight, DWORD lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette)
{
	pSrc += iSrcPoint*4;
	size_t cbSize = lHeight * lAbsSrcPitch;
	
	memcpy(pDst, pSrc, cbSize);

	return true;
}
bool BltHelper::Blit32_BGRA_AT(BYTE*& pDst, BYTE*& pSrc, const DWORD iSrcPoint, DWORD nPoints, DWORD lHeight, DWORD lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette)
{
	pSrc += iSrcPoint*4;
	size_t cbSize = lHeight * lAbsSrcPitch;
	
	memcpy(pDst, pSrc, cbSize);

	ApplyBlendFunction32(pDst, cbSize, nBackground, idfFlags, nTransparent);

	return true;
}
bool BltHelper::Blit32_BGRA_VP(BYTE*& pDst, BYTE*& pSrc, const DWORD iSrcPoint, DWORD nPoints, DWORD lHeight, DWORD lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette)
{
	pSrc += iSrcPoint*4;
	size_t cbSize = nPoints * 4;
	
	for (DWORD j = lHeight; j--; pSrc += lAbsSrcPitch, pDst += lDstPitch) {
		memcpy(pDst, pSrc, cbSize);
		ApplyBlendFunction32(pDst, cbSize, nBackground, idfFlags, nTransparent);
	}
	
	return true;
}
bool BltHelper::Blit32_BGRA_AT_VP(BYTE*& pDst, BYTE*& pSrc, const DWORD iSrcPoint, DWORD nPoints, DWORD lHeight, DWORD lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette)
{
	pSrc += iSrcPoint*4;
	size_t cbSize = nPoints * 4;
	
	for (DWORD j = lHeight; j--; pSrc += lAbsSrcPitch, pDst += lDstPitch) {
		memcpy(pDst, pSrc, cbSize);

		ApplyBlendFunction32(pDst, cbSize, nBackground, idfFlags, nTransparent);
	}
	
	return true;
}

bool BltHelper::Blit24_BGR_T(BYTE*& pDst, BYTE*& pSrc, const DWORD iSrcPoint, DWORD nPoints, DWORD lHeight, DWORD lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette)
{
	DWORD NewColor;
	pSrc += iSrcPoint*3;
	for (DWORD j = lHeight; j--; pSrc += lAbsSrcPitch, pDst += lDstPitch)
	{
		UINT nStep = 0;
		DWORD nLeft = 0, nCur = 0;
		DWORD i = 0;
		DWORD* p = (DWORD*)pSrc;
		DWORD* pp = (DWORD*)pDst;

		while (i < nPoints) {
			nCur = *p;
			switch (nStep) {
			case 0:
				NewColor = (nCur & 0xFFFFFF);
				*pp = (NewColor != nTransparent) ? NewColor : nBackground;
				nLeft = (nCur & 0xFF000000) >> 24;
				nStep++;
				break;
			case 1:
				NewColor = nLeft | ((nCur & 0xFFFF) << 8);
				*pp = (NewColor != nTransparent) ? NewColor : nBackground;
				nLeft = (nCur & 0xFFFF0000) >> 16;
				nStep++;
				break;
			case 2:
				NewColor = nLeft | ((nCur & 0xFF) << 16);
				*pp = (NewColor != nTransparent) ? NewColor : nBackground;
				nLeft = (nCur & 0xFFFFFF00) >> 8;
				i++; pp++;
				if (i < nPoints) {
					*pp = (nLeft != nTransparent) ? nLeft : nBackground;
					nLeft = 0;
				}
				nStep = 0;
				break;
			}
			i++; p++; pp++;
		}										
	}
	return true;
}
bool BltHelper::Blit24_BGR_24(BYTE*& pDst, BYTE*& pSrc, const DWORD iSrcPoint, DWORD nPoints, DWORD lHeight, DWORD lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette)
{
	pSrc += iSrcPoint*3;
	size_t cbSize = lHeight * lAbsSrcPitch;
	
	memcpy(pDst, pSrc, cbSize);

	return true;
}
bool BltHelper::Blit24_BGR(BYTE*& pDst, BYTE*& pSrc, const DWORD iSrcPoint, DWORD nPoints, DWORD lHeight, DWORD lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette)
{
	pSrc += iSrcPoint*3;
	for (DWORD j = lHeight; j--; pSrc += lAbsSrcPitch, pDst += lDstPitch)
	{
		UINT nStep = 0;
		DWORD nLeft = 0, nCur = 0;
		DWORD i = 0;
		DWORD* p = (DWORD*)pSrc;
		DWORD* pp = (DWORD*)pDst;

		while (i < nPoints) {
			nCur = *p;
			switch (nStep) {
			case 0:
				*pp = (nCur & 0xFFFFFF);
				nLeft = (nCur & 0xFF000000) >> 24;
				nStep++;
				break;
			case 1:
				*pp = nLeft | ((nCur & 0xFFFF) << 8);
				nLeft = (nCur & 0xFFFF0000) >> 16;
				nStep++;
				break;
			case 2:
				*pp = nLeft | ((nCur & 0xFF) << 16);
				nLeft = (nCur & 0xFFFFFF00) >> 8;
				i++; pp++;
				if (i < nPoints) {
					*pp = nLeft;
					nLeft = 0;
				}
				nStep = 0;
				break;
			}
			i++; p++; pp++;
		}										
	}
	return true;
}

bool BltHelper::Blit16_BGR_T(BYTE*& pDst, BYTE*& pSrc, const DWORD iSrcPoint, DWORD nPoints, DWORD lHeight, DWORD lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette)
{
	COLORREF NewColor;
	pSrc += iSrcPoint*2;
	for (DWORD j = lHeight; j--; pSrc += lAbsSrcPitch, pDst += lDstPitch)
	{
		for (DWORD i = nPoints; i--;)
		{
			// реально используется 15бит, требуется конвертирование в 24бит
			const DWORD t = ((unsigned short*)pSrc)[i], 
				t2 = (t & 0x7C00) << 9 | (t & 0x03E0) << 6 | (t & 0x001F) << 3;
			NewColor = t2 | t2 >> 5 & 0x00070707;
			if (NewColor == nTransparent)
				((DWORD*)pDst)[i] = nBackground;
			else
				((DWORD*)pDst)[i] = NewColor;
		}
	}
	return true;
}
bool BltHelper::Blit16_BGR(BYTE*& pDst, BYTE*& pSrc, const DWORD iSrcPoint, DWORD nPoints, DWORD lHeight, DWORD lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette)
{
	pSrc += iSrcPoint*2;
	for (DWORD j = lHeight; j--; pSrc += lAbsSrcPitch, pDst += lDstPitch)
	{
		for (DWORD i = nPoints; i--;)
		{
			// реально используется 15бит, требуется конвертирование в 24бит
			const DWORD t = ((unsigned short*)pSrc)[i], 
				t2 = (t & 0x7C00) << 9 | (t & 0x03E0) << 6 | (t & 0x001F) << 3;
			((DWORD*)pDst)[i] = t2 | t2 >> 5 & 0x00070707;
		}
	}
	return true;
}

TODO("Во всех функциях с палитрой проверять макс индекс, или что лучше, делать копию палитры и догонять ее до максимального размера черным непрозрачным цветом");
bool BltHelper::Blit8_PAL(BYTE*& pDst, BYTE*& pSrc, const DWORD iSrcPoint, DWORD nPoints, DWORD lHeight, DWORD lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette)
{
	//UINT NewColor;
	pSrc += iSrcPoint;
	for (DWORD j = lHeight; j--; pSrc += lAbsSrcPitch, pDst += lDstPitch)
	{
		for (DWORD i = nPoints; i--;)
			((DWORD*)pDst)[i] = ((DWORD*)pPalette)[pSrc[i]];
	}
	return true;
}
bool BltHelper::Blit8_PAL_A(BYTE*& pDst, BYTE*& pSrc, const DWORD iSrcPoint, DWORD nPoints, DWORD lHeight, DWORD lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette)
{
	//UINT NewColor;
	pSrc += iSrcPoint;
	for (DWORD j = lHeight; j--; pSrc += lAbsSrcPitch, pDst += lDstPitch)
	{
		for (DWORD i = nPoints; i--;)
			((DWORD*)pDst)[i] = ((DWORD*)pPalette)[pSrc[i]];
		// Alpha post processing
		ApplyBlendFunction32(pDst, nPoints*sizeof(DWORD), nBackground, idfFlags, nTransparent);
	}
	return true;
}
bool BltHelper::Blit8_PAL_T(BYTE*& pDst, BYTE*& pSrc, const DWORD iSrcPoint, DWORD nPoints, DWORD lHeight, DWORD lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette)
{
	UINT NewColor;
	pSrc += iSrcPoint;
	for (DWORD j = lHeight; j--; pSrc += lAbsSrcPitch, pDst += lDstPitch)
	{
		for (DWORD i = nPoints; i--;) {
			NewColor = ((DWORD*)pPalette)[pSrc[i]];
			((DWORD*)pDst)[i] = (NewColor == nTransparent) ? nBackground : NewColor;
		}
	}
	return true;
}
bool BltHelper::Blit8_PAL_TI(BYTE*& pDst, BYTE*& pSrc, const DWORD iSrcPoint, DWORD nPoints, DWORD lHeight, DWORD lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette)
{
	UINT NewColor;
	pSrc += iSrcPoint;
	for (DWORD j = lHeight; j--; pSrc += lAbsSrcPitch, pDst += lDstPitch)
	{
		for (DWORD i = nPoints; i--;) {
			NewColor = pSrc[i];
			((DWORD*)pDst)[i] = (NewColor != nTransparent) ? ((DWORD*)pPalette)[NewColor] : nBackground;
		}
	}
	return true;
}


bool BltHelper::Blit4_PAL(BYTE*& pDst, BYTE*& pSrc, const DWORD iSrcPoint, DWORD nPoints, DWORD lHeight, DWORD lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette)
{
	TODO("Во многих случаях можно сэкономить на (iSrcPoint + i)/?, если iSrcPoint==0 - просто сразу сделать ветки кода");
	//UINT NewColor;
	for (DWORD j = lHeight; j--; pSrc += lAbsSrcPitch, pDst += lDstPitch)
	{
		for (DWORD i = nPoints; i--;) {
			TODO("Можно бы на один байт pSrc обработать сразу две точки в pDst");
			((DWORD*)pDst)[i] = ((DWORD*)pPalette)[
				(iSrcPoint + i) & 1 ? pSrc[(iSrcPoint + i)/2] & 0x0F 
					: pSrc[(iSrcPoint + i)/2] >> 4];
		}
	}
	return true;
}
bool BltHelper::Blit4_PAL_A(BYTE*& pDst, BYTE*& pSrc, const DWORD iSrcPoint, DWORD nPoints, DWORD lHeight, DWORD lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette)
{
	TODO("Во многих случаях можно сэкономить на (iSrcPoint + i)/?, если iSrcPoint==0 - просто сразу сделать ветки кода");
	//UINT NewColor;
	for (DWORD j = lHeight; j--; pSrc += lAbsSrcPitch, pDst += lDstPitch)
	{
		for (DWORD i = nPoints; i--;) {
			TODO("Можно бы на один байт pSrc обработать сразу две точки в pDst");
			((DWORD*)pDst)[i] = ((DWORD*)pPalette)[
				(iSrcPoint + i) & 1 ? pSrc[(iSrcPoint + i)/2] & 0x0F 
					: pSrc[(iSrcPoint + i)/2] >> 4];
		}
		ApplyBlendFunction32(pDst, nPoints*sizeof(DWORD), nBackground, idfFlags, nTransparent);
	}
	return true;
}
bool BltHelper::Blit4_PAL_T(BYTE*& pDst, BYTE*& pSrc, const DWORD iSrcPoint, DWORD nPoints, DWORD lHeight, DWORD lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette)
{
	TODO("Во многих случаях можно сэкономить на (iSrcPoint + i)/?, если iSrcPoint==0 - просто сразу сделать ветки кода");
	UINT NewColor;
	for (DWORD j = lHeight; j--; pSrc += lAbsSrcPitch, pDst += lDstPitch)
	{
		for (DWORD i = nPoints; i--;) {
			TODO("Можно бы на один байт pSrc обработать сразу две точки в pDst");
			NewColor = ((DWORD*)pPalette)[
				(iSrcPoint + i) & 1 ? pSrc[(iSrcPoint + i)/2] & 0x0F 
					: pSrc[(iSrcPoint + i)/2] >> 4];
			if (NewColor == nTransparent) NewColor = nBackground;
			((DWORD*)pDst)[i] = NewColor;
		}
	}
	return true;
}
bool BltHelper::Blit4_PAL_TI(BYTE*& pDst, BYTE*& pSrc, const DWORD iSrcPoint, DWORD nPoints, DWORD lHeight, DWORD lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette)
{
	TODO("Во многих случаях можно сэкономить на (iSrcPoint + i)/?, если iSrcPoint==0 - просто сразу сделать ветки кода");
	UINT NewColor;
	for (DWORD j = lHeight; j--; pSrc += lAbsSrcPitch, pDst += lDstPitch)
	{
		for (DWORD i = nPoints; i--;) {
			TODO("Можно бы на один байт pSrc обработать сразу две точки в pDst");
			NewColor = (iSrcPoint + i) & 1 ? pSrc[(iSrcPoint + i)/2] & 0x0F 
				: pSrc[(iSrcPoint + i)/2] >> 4;
			((DWORD*)pDst)[i] = (NewColor != nTransparent) ? ((DWORD*)pPalette)[NewColor] : nBackground;
		}
	}
	return true;
}


bool BltHelper::Blit2_PAL(BYTE*& pDst, BYTE*& pSrc, const DWORD iSrcPoint, DWORD nPoints, DWORD lHeight, DWORD lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette)
{
	TODO("Во многих случаях можно сэкономить на (iSrcPoint + i)/?, если iSrcPoint==0 - просто сразу сделать ветки кода");
	for (DWORD j = lHeight; j--; pSrc += lAbsSrcPitch, pDst += lDstPitch)
	{
		for (DWORD i = nPoints; i--;)
			((DWORD*)pDst)[i] = ((DWORD*)pPalette)[pSrc[(iSrcPoint + i)/4] >> ((3 - ((iSrcPoint + i) & 3)) & 3)*2];
	}
	return true;
}
bool BltHelper::Blit2_PAL_A(BYTE*& pDst, BYTE*& pSrc, const DWORD iSrcPoint, DWORD nPoints, DWORD lHeight, DWORD lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette)
{
	TODO("Во многих случаях можно сэкономить на (iSrcPoint + i)/?, если iSrcPoint==0 - просто сразу сделать ветки кода");
	for (DWORD j = lHeight; j--; pSrc += lAbsSrcPitch, pDst += lDstPitch)
	{
		for (DWORD i = nPoints; i--;)
			((DWORD*)pDst)[i] = ((DWORD*)pPalette)[pSrc[(iSrcPoint + i)/4] >> ((3 - ((iSrcPoint + i) & 3)) & 3)*2];
		ApplyBlendFunction32(pDst, nPoints*sizeof(DWORD), nBackground, idfFlags, nTransparent);
	}
	return true;
}

bool BltHelper::Blit1_PAL(BYTE*& pDst, BYTE*& pSrc, const DWORD iSrcPoint, DWORD nPoints, DWORD lHeight, DWORD lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette)
{
	TODO("Во многих случаях можно сэкономить на (iSrcPoint + i)/?, если iSrcPoint==0 - просто сразу сделать ветки кода");
	for (DWORD j = lHeight; j--; pSrc += lAbsSrcPitch, pDst += lDstPitch)
	{
		for (DWORD i = nPoints; i--;)
			((DWORD*)pDst)[i] = ((DWORD*)pPalette)[pSrc[(iSrcPoint + i)/8] >> (7 - ((iSrcPoint + i) & 7)) & 1];
	}
	return true;
}
bool BltHelper::Blit1_PAL_A(BYTE*& pDst, BYTE*& pSrc, const DWORD iSrcPoint, DWORD nPoints, DWORD lHeight, DWORD lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette)
{
	TODO("Во многих случаях можно сэкономить на (iSrcPoint + i)/?, если iSrcPoint==0 - просто сразу сделать ветки кода");
	for (DWORD j = lHeight; j--; pSrc += lAbsSrcPitch, pDst += lDstPitch)
	{
		for (DWORD i = nPoints; i--;)
			((DWORD*)pDst)[i] = ((DWORD*)pPalette)[pSrc[(iSrcPoint + i)/8] >> (7 - ((iSrcPoint + i) & 7)) & 1];
		// Alpha post processing
		ApplyBlendFunction32(pDst, nPoints*sizeof(DWORD), nBackground, idfFlags, nTransparent);
	}
	return true;
}





BltHelper::BlitFunction_t BltHelper::BlitChoose(
			DWORD nDstBPP, DWORD nBPP, DWORD lAbsSrcPitch, int lDstPitch, BOOL lbAlpha, BOOL lbTransColor, BOOL lbTransIndex,
			bool &result, bool &bMayContinue, const void *&pPalette, pvdColorModel ColorModel)
{
	BlitFunction_t BlitFunction = NULL;
	
						switch (nBPP)
						{
							case 0x20: // 32bit BGR/BGRA (альфа канал может отсутствовать)
								_ASSERTE(nDstBPP==32);
								if (ColorModel == PVD_CM_CMYK) {
									BlitFunction = Blit32_CMYK;
								} else
								if (lAbsSrcPitch == lDstPitch) {
									if (lbAlpha || lbTransColor)
										BlitFunction = Blit32_BGRA_AT;
									else
										BlitFunction = Blit32_BGRA;
								} else {
									if (lbAlpha || lbTransColor)
										BlitFunction = Blit32_BGRA_AT_VP;
									else
										BlitFunction = Blit32_BGRA_VP;
								}
								break;
							case 0x18: // 24bit, BGR
								_ASSERTE(nDstBPP==32 || nDstBPP==24);
								if (lbAlpha) {
									OutputDebugString(L"Alpha is not supported in 24 bit\n");
									result = bMayContinue = false;
								} else if (nDstBPP==24) {
									BlitFunction = Blit24_BGR_24;
								} else if (lbTransColor) {
									BlitFunction = Blit24_BGR_T;
								} else {
									BlitFunction = Blit24_BGR;
								}
								break;
							case 0x10: // 16bit
								_ASSERTE(nDstBPP==32);
								if (lbAlpha) {
									OutputDebugString(L"Alpha is not supported in 16 bit\n");
									result = bMayContinue = false;
								} else if (lbTransColor) {
									BlitFunction = Blit16_BGR_T;
								} else {
									BlitFunction = Blit16_BGR;
								}
								break;
							case 0x08:
								_ASSERTE(nDstBPP==32);
								if (!pPalette) {
									//_ASSERTE(pPalette);
									OutputDebugString(L"\n!!! Palette is not defined !!!\n");
									result = bMayContinue = false;
								} else
								if (lbAlpha) {
									BlitFunction = Blit8_PAL_A;
								} else if (lbTransColor) {
									BlitFunction = Blit8_PAL_T;
								} else if (lbTransIndex) {
									BlitFunction = Blit8_PAL_TI;
								} else {
									BlitFunction = Blit8_PAL;
								}
								break;
							case 0x04:
								_ASSERTE(nDstBPP==32);
								if (!pPalette) {
									//_ASSERTE(pPalette);
									OutputDebugString(L"\n!!! Palette is not defined !!!\n");
									result = bMayContinue = false;
								} else
								if (lbAlpha) {
									BlitFunction = Blit4_PAL_A;
								} else if (lbTransColor) {
									BlitFunction = Blit4_PAL_T;
								} else if (lbTransIndex) {
									BlitFunction = Blit4_PAL_TI;
								} else {
									BlitFunction = Blit4_PAL;
								}
								break;
							case 0x02:
								_ASSERTE(nDstBPP==32);
								if (!pPalette) {
									//_ASSERTE(pPalette);
									OutputDebugString(L"\n!!! Palette is not defined !!!\n");
									result = bMayContinue = false;
								} else
								if (lbAlpha) {
									BlitFunction = Blit2_PAL_A;
								} else {
									TODO("Не хватает Blit2_PAL_T & Blit2_PAL_TI");
									BlitFunction = Blit2_PAL;
								}
								break;
							case 0x01:
								_ASSERTE(nDstBPP==32);
								if (!pPalette) {
									//_ASSERTE(pPalette);
									OutputDebugString(L"\n!!! Palette is not defined !!!\n");
									result = bMayContinue = false;
								} else
								if (lbAlpha) {
									BlitFunction = Blit1_PAL_A;
								} else {
									TODO("Не хватает Blit1_PAL_T & Blit1_PAL_TI");
									BlitFunction = Blit1_PAL;
								}
								break;
							default:
							{
								wchar_t szMsg[128];
								wsprintf(szMsg, L"Unsupported bit depth = %i\n", nBPP);
								OutputDebugString(szMsg);
								result = bMayContinue = false;
							}
						}
						
	return BlitFunction;
}

