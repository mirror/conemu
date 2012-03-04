
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
#include "PVDManager.h"

LPCSTR szDecoderHandle = "CDecoderHandle";

CDecoderHandle::CDecoderHandle(LPCSTR asFrom, CImage* apImage)
	: CRefRelease(asFrom)
{
	mp_Image = apImage;
	mp_Image->AddRef(szDecoderHandle);
	
	mb_FileOpened = false;
	//mi_SubDecoder = 0;
	mp_FileContext = NULL;
	mp_DecoderModule = NULL;
	
	memset(&Params, 0, sizeof(Params));
}

CDecoderHandle::~CDecoderHandle()
{
	Close();
	
	SafeRelease(mp_DecoderModule,szDecoderHandle);

	SafeRelease(mp_Image,szDecoderHandle);
}

void CDecoderHandle::MoveTo(CDecoderHandle *apDst)
{
	if (this == NULL || apDst == NULL)
	{
		_ASSERTE(this!=NULL);
		_ASSERTE(apDst!=NULL);
		return;
	}
	if (apDst == this)
	{
		//-- this->Release(szPVDManager); // удалить вторую ссылку
		return; // больше ничего делать не нужно
	}
		
	if (!mp_DecoderModule || !mb_FileOpened)
	{
		_ASSERTE(mp_DecoderModule!=NULL);
		_ASSERTE(mb_FileOpened);
		return;
	}

	// Переместить дескриптор в apDst
	apDst->Close();
	apDst->Assign(mp_DecoderModule, mp_FileContext);
	
	apDst->Info = Info;
	apDst->Params = Params;
	
	mb_FileOpened = false;
	mp_FileContext = NULL;
	mp_DecoderModule->Release(szDecoderHandle);
	mp_DecoderModule = NULL;
}

void CDecoderHandle::Assign(CModuleInfo *apDecoder, LPVOID apFileContext, pvdInfoImage2* apInfo /*= NULL*/)
{
	Close();
	
	_ASSERTE(GetCurrentThreadId() == gnDecoderThreadId);
	
	if (apDecoder)
	{
		mb_FileOpened = true;
		mp_DecoderModule = apDecoder;
		mp_DecoderModule->AddRef(szDecoderHandle);
		mp_FileContext = apFileContext;
		if (apInfo)
			Info.Assign(*apInfo);
	}
	else
	{
		_ASSERTE(apDecoder);
	}
}

void CDecoderHandle::Close()
{
	if (mb_FileOpened)
	{
		// Выполнять в потоке декодера!
		if (gnDecoderThreadId && GetCurrentThreadId() != gnDecoderThreadId)
		{
			_ASSERTE(GetCurrentThreadId() == gnDecoderThreadId);
			RequestTerminate();
			return;
		}

		LPVOID pContext = mp_FileContext; mp_FileContext = NULL;
		mb_FileOpened = false;
		
		if (mp_DecoderModule)
		{
			if (mp_DecoderModule->pPlugin)
			{
				// в mp_FileContext - допускается NULL
				mp_DecoderModule->pPlugin->FileClose2(pContext);
			}

			// Здесь - объект больше не нужен
			mp_DecoderModule->Release(szDecoderHandle);
			mp_DecoderModule = NULL;
			
		}
		else
		{
			_ASSERTE(mp_DecoderModule && mp_DecoderModule->pPlugin);
		}
	}
	
	memset(&Info, 0, sizeof(Info));
	memset(&Params, 0, sizeof(Params));
}

LPVOID CDecoderHandle::Context()
{
	if (!this)
	{
		_ASSERTE(this!=NULL);
		return NULL;
	}
	_ASSERTE(mp_FileContext!=NULL);
	return mp_FileContext;
}

CModuleInfo* CDecoderHandle::Decoder()
{
	if (!this)
	{
		_ASSERTE(this!=NULL);
		return NULL;
	}
	_ASSERTE(mp_DecoderModule);
	return mp_DecoderModule;
}

bool CDecoderHandle::IsReady()
{
	if (!this) return false;
	return mb_FileOpened;
}

void CDecoderHandle::RequestRelease()
{
	if ((RefCount() <= 1) && mp_Image)
	{
		mp_Image->DecoderHandleReleased(this);
		SafeRelease(mp_Image,szDecoderHandle);
	}
}
