
#pragma once

/**************************************************************************
Copyright (c) 2011 Maximus5
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


#ifdef _DEBUG
	#include <crtdbg.h>
#else
	#ifndef _ASSERTE
		#define _ASSERTE(a)
	#endif
#endif

class CUnicodeFileName {
public:
	CUnicodeFileName();
	~CUnicodeFileName();
	

	// asFileName может содержать или НЕ содержать префикс
	// если abIsFolder - на конце принудительно ставится '\\'
	const wchar_t* Assign(const wchar_t* asFileName, bool abIsFolder = false);

	
	wchar_t* GetBuffer(int nLen0); // Выделить буфер. nLen0 должен включать завершающий '\0'
	
	bool ReleaseBuffer(bool abIsFolder); // Проверить буфер и если он содержит НЕ UNC - поправить
	
	bool CompareDir(const wchar_t* asDir);
	
	CUnicodeFileName& operator=(const wchar_t* as);
	CUnicodeFileName& operator=(CUnicodeFileName& as);
	operator const wchar_t* () const;

	static void SkipPrefix(const wchar_t** rsPath);
	
	bool IsEmpty();

	void Validate();

private:
	bool mb_IsUNC;
	size_t mn_BufferSize;
	wchar_t* ms_Buffer;
};
