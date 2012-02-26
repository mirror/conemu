
/**************************************************************************
Copyright (c) 2010 Maximus5
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
#include "ImageInfo.h"
#include "PVDModuleBase.h"

//ImageInfo::ImageInfo()
//{
//	bPageInfoLoaded = false;
//
//	lWidth = lHeight = nBPP = 0;
//	lDecodedWidth = lDecodedHeight = nDecodedBPP = 0;
//	lOpenTime = lStartOpenTime = lTimeOpen = lTimeDecode = lTimeTransfer = lTimePaint = 0;
//	bTimePaint = FALSE;
//	nPages = nPage = 0;
//	Animation = 0;
//	lFrameTime = 0;
//
//	DecoderName[0] = FormatName[0] = Compression[0] = Comments[0] = 0;
//}

void ImageInfo::InitDecoder(CModuleInfo *pDecoder)
{
	lstrcpyn(DecoderName, pDecoder->pPlugin->pName, sizeofarray(DecoderName));
	lWidth = lHeight = nBPP = 0;
	nPages = 0; Animation = false;
}

void ImageInfo::Assign(const pvdInfoImage2& InfoImage)
{
	bPageInfoLoaded = false;

	// Инициализируется только в pvdInfoPage2
	lWidth = 0;
	lHeight = 0;
	nBPP = 0;

	// Инициализация количества страниц
	nPage = 0;
	nPages = InfoImage.nPages;
	Animation = InfoImage.Flags & PVD_IIF_ANIMATED/* 1 */;

	// Если есть формат/сжатие/комментарий - запомним
	if (InfoImage.pFormatName)
		lstrcpyn(FormatName, InfoImage.pFormatName, sizeofarray(FormatName));
	else
		*FormatName = 0;

	if (InfoImage.pCompression)
		lstrcpyn(Compression, InfoImage.pCompression, sizeofarray(Compression));
	else
		*Compression = 0;

	if (InfoImage.pComments)
		lstrcpyn(Comments, InfoImage.pComments, sizeofarray(Comments));
	else
		*Comments = 0;
		
	// Сброс остальных полей
	*Comments = 0;
}

void ImageInfo::Assign(const pvdInfoPage2& InfoPage)
{
	bPageInfoLoaded = true;
	
	lWidth = InfoPage.lWidth;
	lHeight = InfoPage.lHeight;
	nBPP = InfoPage.nBPP;

	// Обновляем только если передали, иначе сохраняем то что было в pvdInfoImage2
	if (InfoPage.pFormatName)
		lstrcpyn(FormatName, InfoPage.pFormatName, sizeofarray(FormatName));

	// Обновляем только если передали, иначе сохраняем то что было в pvdInfoImage2
	if (InfoPage.pCompression)
		lstrcpyn(Compression, InfoPage.pCompression, sizeofarray(Compression));
}

void ImageInfo::Assign(const pvdInfoDecode2& DecodeInfo, const wchar_t* pNamePart /*= NULL*/)
{
	if (DecodeInfo.pFormatName)
		lstrcpyn(FormatName, DecodeInfo.pFormatName, sizeofarray(FormatName));
	
	if (DecodeInfo.pCompression)
		lstrcpyn(Compression, DecodeInfo.pCompression, sizeofarray(Compression));
	
	if (!FormatName[0] && pNamePart)
	{
		const wchar_t* pszExt = wcsrchr(pNamePart, L'.');
		if (pszExt)
		{
			FormatName[0] = L'[';
			lstrcpyn(FormatName+1, pszExt+1, sizeofarray(FormatName)-3);
			lstrcat(FormatName, L"]");
		}
	}
}

// В процессе декодирования может появиться большее количество страниц, чем
// было обозначено при первичном обнюхивании файла
void ImageInfo::UpdatePageCount(uint anNewPages)
{
	if (anNewPages)
	{
		if (nPages != anNewPages)
		{
			if (anNewPages > nPage)
			{
				nPages = anNewPages;
			}
			else
			{
				_ASSERTE(anNewPages > nPage);
			}
		}
	}
}

bool ImageInfo::IsPageLoaded(uint anPage)
{
	return (bPageInfoLoaded && (anPage == nPage));
}
