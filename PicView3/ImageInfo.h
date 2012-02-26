
#pragma once

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

#include "PVDInterface/PictureViewPlugin.h"

struct ImageInfo
{
	bool  bPageInfoLoaded;

	uint  lWidth, lHeight, nBPP;
	WARNING("Убрать отсюда декодированные размеры? Тут должна быть информация об исходном файле");
	uint  lDecodedWidth, lDecodedHeight, nDecodedBPP;
	DWORD lOpenTime, lStartOpenTime;
	DWORD lTimeOpen, lTimeDecode, lTimeTransfer, lTimePaint;
	BOOL  bTimePaint;
	uint  nPages; // pages count
	uint  nPage; // 0 based
	uint  Animation;
	uint  lFrameTime;

	wchar_t DecoderName[0x80], FormatName[0x80], Compression[0x80], Comments[0x100];
	
	ImageInfo()
	{
		memset(this, 0, sizeof(ImageInfo));
	}

	void InitDecoder(CModuleInfo *pDecoder);
	void Assign(const pvdInfoImage2& InfoImage);
	void Assign(const pvdInfoPage2& InfoPage);
	void Assign(const pvdInfoDecode2& DecodeInfo, const wchar_t* pNamePart = NULL);
	void UpdatePageCount(uint anNewPages);
	bool IsPageLoaded(uint anPage);
};
