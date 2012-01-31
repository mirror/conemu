
/*
Copyright (c) 2009-2012 Maximus5
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

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ''AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include "Header.h"
#include "ConEmu.h"
#include "VConRelease.h"
#include "VirtualConsole.h"

#define REF_FINALIZE 0x7FFFFFFF

CVConRelease::CVConRelease()
{
	mn_RefCount = 1;
}

CVConRelease::~CVConRelease()
{
	_ASSERTE(mn_RefCount==REF_FINALIZE);
}

void CVConRelease::AddRef()
{
	if (!this)
	{
		_ASSERTE(this!=NULL);
		return;
	}
	InterlockedIncrement(&mn_RefCount);
}

int CVConRelease::Release()
{
	if (!this)
		return 0;
	InterlockedDecrement(&mn_RefCount);
	_ASSERTE(mn_RefCount>=0);
	if (mn_RefCount <= 0)
	{
		mn_RefCount = REF_FINALIZE; // �������������, ����� �� ���� ��������� ������������ delete ��� ������ ������������
		CVirtualConsole* pVCon = (CVirtualConsole*)this;
		delete pVCon;
		return 0;
	}
	return mn_RefCount;
}


CVConGuard::CVConGuard(CVirtualConsole* &apRef)
{
	mp_Ref = apRef;
	if (mp_Ref)
	{
		mp_Ref->AddRef();

		// -- �� ����������. ���������� ��� �� ����� WM_CREATE
		//if (!gpConEmu->isValid(mp_Ref))
		//{
		//	MBoxAssert(gpConEmu->isValid(mp_Ref));
		//	apRef = NULL;
		//}
		//else
		//{
		//	mp_Ref->AddRef();
		//}
	}
}

CVConGuard::~CVConGuard()
{
	if (mp_Ref)
		mp_Ref->Release();
}