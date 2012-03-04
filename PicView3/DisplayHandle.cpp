
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
#include "DisplayHandle.h"
#include "Image.h"
#include "PVDModuleBase.h"


/* ********************************** */

LPCSTR szDisplayHandlePtr = "CDisplayHandlePtr";

CDisplayHandlePtr::CDisplayHandlePtr()
	: pDraw(NULL)
{
};

CDisplayHandlePtr::CDisplayHandlePtr(CDisplayHandle* apDraw)
	: pDraw(NULL)
{
	Assign(apDraw);
};

CDisplayHandlePtr::~CDisplayHandlePtr()
{
	SafeRelease(pDraw,szDisplayHandlePtr);
};

void CDisplayHandlePtr::Assign(CDisplayHandle* apDraw)
{
	if (apDraw != pDraw)
	{
		if (apDraw)
			apDraw->AddRef(szDisplayHandlePtr);

		SafeRelease(pDraw,szDisplayHandlePtr);
		pDraw = apDraw;
	}
}

CDisplayHandle* CDisplayHandlePtr::operator->() const
{
	_ASSERTE(pDraw!=NULL);
	return pDraw;
};

CDisplayHandlePtr::operator CDisplayHandle*() const
{
	_ASSERTE(pDraw!=NULL);
	return pDraw;
};

bool CDisplayHandlePtr::IsValid() const
{
	return (pDraw != NULL);
};



/* ********************************** */

LPCSTR szDisplayHandle = "CDisplayHandle";

CDisplayHandle::CDisplayHandle(LPCSTR asFrom, CImage* apImage/*, CDecoderHandle* apDecoderHandle*/)
	: CRefRelease(asFrom)
{
	mp_Image = apImage;
	mp_Image->AddRef(szDisplayHandle);
	
	#ifdef _DEBUG
	// Для информации
	FileName = (LPCWSTR)apImage->FileName;
	#endif

	//mp_DecoderHandle = apDecoderHandle;
	//mp_DecoderHandle->AddRef();
	
	mb_DisplayOpened = false;
	mp_DisplayModule = NULL;
	mp_DrawContext = NULL;
	
	memset(&Params, 0, sizeof(Params));
}

CDisplayHandle::~CDisplayHandle()
{
	Close();

	//SafeRelease(mp_DecoderHandle);

	SafeRelease(mp_DisplayModule,szDisplayHandle);
	
	SafeRelease(mp_Image,szDisplayHandle);
}

void CDisplayHandle::MoveTo(CDisplayHandle *apDst)
{
	if (this == NULL || apDst == NULL)
	{
		_ASSERTE(this!=NULL);
		_ASSERTE(apDst!=NULL);
		return;
	}
	if (apDst == this)
	{
		this->Release(szDisplayHandle); // удалить вторую ссылку
		return; // больше ничего делать не нужно
	}
		
	if (!mp_DisplayModule || !mb_DisplayOpened)
	{
		_ASSERTE(mp_DisplayModule!=NULL);
		_ASSERTE(mb_DisplayOpened);
		return;
	}

	// Переместить дескриптор в apDst
	apDst->Close();
	apDst->Assign(mp_DisplayModule, mp_DrawContext);
	
	apDst->Info = Info;
	apDst->Params = Params;
	
	mb_DisplayOpened = false;
	mp_DrawContext = NULL;
	mp_DisplayModule->Release(szDisplayHandle);
	mp_DisplayModule = NULL;
}

void CDisplayHandle::Assign(CModuleInfo *apDisplay, LPVOID apDrawContext, ImageInfo* apInfo /*= NULL*/, DecodeParams* apParams /*= NULL*/)
{
	Close();
	
	if (apDisplay)
	{
		mb_DisplayOpened = true;
		mp_DisplayModule = apDisplay;
		mp_DisplayModule->AddRef(szDisplayHandle);
		mp_DrawContext = apDrawContext;
		if (apInfo)
			Info = *apInfo;
		if (apParams)
			Params = *apParams;
	} else {
		_ASSERTE(apDisplay);
	}
}

void CDisplayHandle::Close()
{
	TODO("Хорошо бы проверять, что это выполняется в нити дисплея!");
	if (mb_DisplayOpened)
	{
		LPVOID pDraw = mp_DrawContext;
		mp_DrawContext = NULL;
		mb_DisplayOpened = false;
		
		if (mp_DisplayModule)
		{
			if (mp_DisplayModule->pPlugin)
			{
				// в mp_ImageContext - допускается NULL
				mp_DisplayModule->pPlugin->DisplayClose2(pDraw);
			}

			// Здесь - объект больше не нужен
			mp_DisplayModule->Release(szDisplayHandle);
			mp_DisplayModule = NULL;
			
		} else {
			_ASSERTE(mp_DisplayModule && mp_DisplayModule->pPlugin);
		}
	}
	
	memset(&Info, 0, sizeof(Info));
	memset(&Params, 0, sizeof(Params));
}

LPVOID CDisplayHandle::Context()
{
	if (!this)
	{
		_ASSERTE(this!=NULL);
		return NULL;
	}
	_ASSERTE(mp_DrawContext!=NULL);
	return mp_DrawContext;
}

CModuleInfo* CDisplayHandle::Display()
{
	if (!this)
	{
		_ASSERTE(this!=NULL);
		return NULL;
	}
	_ASSERTE(mp_DisplayModule!=NULL);
	return mp_DisplayModule;
}

bool CDisplayHandle::IsReady()
{
	if (!this)
		return false;

	if (!mb_DisplayOpened || (mp_DisplayModule == NULL))
		return false;
	return true;
}
