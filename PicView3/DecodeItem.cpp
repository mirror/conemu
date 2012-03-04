
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
#include "DecoderHandle.h"
#include "Image.h"
#include "PVDModuleBase.h"
#include "DecodeItem.h"
#include "DisplayHandle.h"
#include "PVDManager.h"

LPCSTR szDecodeItem = "CDecodeItem";

CDecodeItem::CDecodeItem(LPCSTR asFrom, CImage* apImage)
	: CRefRelease(asFrom)
{
	pImage = apImage;
	if (pImage)
		pImage->AddRef(szDecodeItem);

	pFile = apImage->mp_File;
	if (pFile)
		pFile->AddRef(szDecodeItem);

	// Сброс структуры данных декодера (API)
	memset(&Data, 0, sizeof(Data));
	// У структур Info и Params есть конструкторы

	pDraw = NULL; // пока не создан
}

CDecodeItem::~CDecodeItem()
{
	// Перенос контекстов производится в {OnItemReady()/DMSG_IMAGEREADY}

	//if (pImage && pDraw && (pDraw != pImage->mp_Draw))
	//{
	//	_ASSERTE(!pImage->mp_Draw || (pImage->mp_Draw==pDraw));
	//	if (pImage->mp_Draw)
	//		pImage->mp_Draw->Release();
	//	pImage->mp_Draw = pDraw;
	//	pImage->mp_Draw->AddRef();
	//}
	//SafeRelease(pDraw,szPVDManager);
	g_Manager.CloseDisplay(this);
	
	//if (pImage && pFile && (pFile != pImage->mp_File))
	//{
	//	_ASSERTE(pFile == pImage->mp_File); // Хм? Может меняться?
	//	if (pImage->mp_File)
	//		pImage->mp_File->Release();
	//	pImage->mp_File = pFile;
	//	pImage->mp_File->AddRef();
	//}
	//SafeRelease(pFile,szPVDManager);
	g_Manager.CloseDecoder(this);
	
	SafeRelease(pImage,szDecodeItem);

	//SafeFree(mpb_Processed);
	//mn_ProcessedSize = 0;
}

#if 0
bool CDecodeItem::OnItemReady()
{
	if (GetCurrentThreadId() != gnDisplayThreadId)
	{
		_ASSERTE(GetCurrentThreadId() == gnDisplayThreadId);
		return false;
	}

	if (!pImage)
	{
		_ASSERTE(pImage!=NULL);
		return false;
	}

	// Собственно перенос данных из декодера в дисплей
	if (g_Manager.PixelsToDisplay(this))
	{
		pImage->StoreDecodedFrame(this);
	}

	if (!pDraw || !pDraw->IsReady())
	{
		_ASSERTE(pDraw && pDraw->IsReady());
		return false;
	}
	return true;
}
#endif
