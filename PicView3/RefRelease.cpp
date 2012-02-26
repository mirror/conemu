
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
#include "RefRelease.h"
#include "RefKeeper.h"

LPCSTR szFromConstructor = "Constructor";

CRefRelease::CRefRelease(LPCSTR asFrom)
{
	mn_RefCount = 1;
	#ifdef _DEBUG
	InitializeCriticalSection(&csFrom);
	#endif
	ms_CreatedFrom = asFrom ? asFrom : szFromConstructor;
	AddFrom(ms_CreatedFrom);

	_ASSERTE(gp_RefKeeper!=NULL);
	TODO("Хорошо бы это наружу вынести, чтобы внятные имена объектам дать");
	gp_RefKeeper->Register(this, L"");
}

CRefRelease::~CRefRelease()
{
	_ASSERTE(mn_RefCount==0);

	_ASSERTE(gp_RefKeeper!=NULL);
	gp_RefKeeper->Unregister(this);

	#ifdef _DEBUG
	DeleteCriticalSection(&csFrom);
	#endif
}

void CRefRelease::AddRef(LPCSTR asFrom)
{
	if (!this)
	{
		_ASSERTE(this!=NULL);
		return;
	}
	InterlockedIncrement(&mn_RefCount);

	AddFrom(asFrom);
}

int CRefRelease::Release(LPCSTR asFrom)
{
	if (!this)
		return 0;
	
	InterlockedDecrement(&mn_RefCount);

	RemoveFrom(asFrom);

	_ASSERTE(mn_RefCount>=0);
	if (mn_RefCount <= 0)
	{
		delete this;
		return 0;
	}
	return mn_RefCount;
}

void CRefRelease::AddFrom(LPCSTR asFrom)
{
	#ifdef _DEBUG
	EnterCriticalSection(&csFrom);
	From.push_back(asFrom);
	LeaveCriticalSection(&csFrom);
	#endif
}

void CRefRelease::RemoveFrom(LPCSTR asFrom)
{
	#ifdef _DEBUG
	EnterCriticalSection(&csFrom);
	std::vector<LPCSTR>::iterator i;
	bool bRemoved = false;
	for (i = From.begin(); i != From.end(); ++i)
	{
		if ((*i) == asFrom)
		{
			From.erase(i);
			bRemoved = true;
			break;
		}
	}
	_ASSERTE(bRemoved);
	LeaveCriticalSection(&csFrom);
	#endif
}
