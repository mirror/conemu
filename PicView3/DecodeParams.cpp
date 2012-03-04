
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
#include "DecodeParams.h"
#include "DecodeItem.h"
#include "PVDManager.h"
	
	
// Конструктор, оператор присваивания, методы
//DecodeParams::DecodeParams()
//{
//	memset(this, 0, sizeof(DecodeParams));
//}

//DecodeParams::~DecodeParams()
//{
//	SafeRelease(pDecodeItem);
//}

DecodeParams& DecodeParams::operator=(DecodeParams& a)
{
	// Копируемые поля
	Flags = a.Flags;
	nRawIndex = a.nRawIndex;
	iDirection = a.iDirection;
	nPage = a.nPage;
	Priority = a.Priority;
	rcSource = a.rcSource;
	szTarget = a.szTarget;
	nFromRawIndex = a.nFromRawIndex;

	// НЕ копируемые поля
	buf = NULL;
	lBuf = 0;
	pResult = NULL;

	// Fin
	return *this;
}

void DecodeParams::OnItemReady()
{
	DecodeParams* pParamsCopy = NULL;

	if (!pDecodeItem)
	{
		_ASSERTE(g_Manager.IsImageReady(this));
	}
	else
	{
		_ASSERTE(pDecodeItem->Params.nRawIndex == nRawIndex);
		_ASSERTE(pDecodeItem->Params.Compare(this));
	}

	pParamsCopy = new DecodeParams;
	*pParamsCopy = *this;

	PostMessage(g_Plugin.hWnd, DMSG_IMAGEREADY, (WPARAM)pParamsCopy, (LPARAM)pDecodeItem);
	pDecodeItem = NULL;
}

bool DecodeParams::Compare(DecodeParams* pOther)
{
	_ASSERTE(pOther->nRawIndex!=-1 && nRawIndex!=-1);
	if (/*((pOther->Flags & (eRenderRelativeIndex|eRenderFirstAvailable)) 
			!= (Flags & (eRenderRelativeIndex|eRenderFirstAvailable)))
		||*/
		pOther->nRawIndex != nRawIndex || pOther->nPage != nPage
		/*|| ((Flags & eRenderRelativeIndex) && pOther->iDirection != iDirection)*/)
	{
		return false;
	}
	WARNING("Доделать сравнение параметров (масштаб, качество, ...)");
	return true;
}
