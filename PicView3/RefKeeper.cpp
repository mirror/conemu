
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

CRefKeeper* gp_RefKeeper = NULL;

CRefKeeper::CRefKeeper()
{
	InitializeCriticalSection(&mcs_Items);
	mn_MaxSize = 0;
	mp_Items = NULL;
}

CRefKeeper::~CRefKeeper()
{
	int idxUnreleased = -1;
	EnterCriticalSection(&mcs_Items);
	if (mp_Items)
	{
		for (int i = 0; i < mn_MaxSize; i++)
		{
			if (mp_Items[i].bUsed)
			{
				idxUnreleased = i;
				TODO("Может быть стоит освободить объекты, или сообщить пользователю об ошибке");
			}
		}
	}
	_ASSERTE(idxUnreleased == -1);
	LeaveCriticalSection(&mcs_Items);
	DeleteCriticalSection(&mcs_Items);
}

void CRefKeeper::Register(CRefRelease* apRef, LPCTSTR asDesc)
{
	EnterCriticalSection(&mcs_Items);
	
	int idx = -1;
	
	if (mp_Items)
	{
		for (int i = 0; i < mn_MaxSize; i++)
		{
			if (!mp_Items[i].bUsed)
			{
				idx = i; break;
			}
		}
	}
	
	if (idx == -1)
	{
		struct RegKeeperItem *pNew = (struct RegKeeperItem*)calloc(mn_MaxSize+64, sizeof(struct RegKeeperItem));
		if (mp_Items && mn_MaxSize)
			memmove(pNew, mp_Items, mn_MaxSize*sizeof(struct RegKeeperItem));
		SafeFree(mp_Items);
		mp_Items = pNew;
		idx = mn_MaxSize;
		mn_MaxSize += 64;
	}

	mp_Items[idx].bUsed = true;
	mp_Items[idx].pRef = apRef;
	lstrcpyn(mp_Items[idx].sDesc, asDesc, sizeofarray(mp_Items[idx].sDesc));
	
	LeaveCriticalSection(&mcs_Items);
}

void CRefKeeper::Unregister(CRefRelease* apRef)
{
	EnterCriticalSection(&mcs_Items);
	if (mp_Items)
	{
		for (int i = 0; mp_Items && i < mn_MaxSize; i++)
		{
			if (mp_Items[i].bUsed && mp_Items[i].pRef == apRef)
				mp_Items[i].bUsed = false;
		}
	}
	LeaveCriticalSection(&mcs_Items);
}
