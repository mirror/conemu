
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


class CImage;
class CModuleInfo;
struct DecodeParams;

#include "PVDInterface/PictureViewPlugin.h"
#include "ImageInfo.h"
#include "DecodeParams.h"
#include "RefRelease.h"

// ”наследован от RefRelease, т.к. ссылка на объект может использоватьс€
// в разных нит€х, например в очереди декодировани€
class CDecoderHandle : public CRefRelease
{
protected:
	CImage* mp_Image;  // RefRelease
	bool mb_FileOpened;
	
public:
	//uint mi_SubDecoder;
	
protected:
	LPVOID mp_FileContext;
	CModuleInfo* mp_DecoderModule; // RefRelease

public:
	ImageInfo Info;
	DecodeParams Params;

#ifdef _DEBUG
	CUnicodeFileName FileName;
#endif
	
public:
	CDecoderHandle(LPCSTR asFrom, CImage* apImage);
	void MoveTo(CDecoderHandle *apDst);
	void Assign(CModuleInfo *apDecoder, LPVOID apFileContext, pvdInfoImage2* apInfo = NULL);
	void Close();
	LPVOID Context();
	CModuleInfo* Decoder();
	bool IsReady();
	void RequestRelease();
protected:
	virtual ~CDecoderHandle();
};
