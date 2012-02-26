
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


#include "PictureView.h"
#include "PictureView_FileName.h"

CUnicodeFileName::CUnicodeFileName()
{
	mb_IsUNC = false; mn_BufferSize = 0; ms_Buffer = NULL;
}

CUnicodeFileName::~CUnicodeFileName()
{
	if (ms_Buffer)
	{
		free(ms_Buffer);
		ms_Buffer = NULL;
	}
}

bool CUnicodeFileName::IsEmpty()
{
	if (!this) return true;
	if (!ms_Buffer) return true;
	if (!*ms_Buffer) return true;
	return false;
}

// asFileName может содержать или НЕ содержать префикс
// если abIsFolder - на конце принудительно ставится '\\'
const wchar_t* CUnicodeFileName::Assign(const wchar_t* asFileName, bool abIsFolder /*= false*/)
{
	//WARNING!!! asFileName инкрементить нельзя - он ниже сравнивается в ms_Buffers

	bool lbIsFull = false, lbIsNetwork = false;
	size_t nLen = 0;
	if (asFileName && *asFileName)
	{
		nLen = lstrlen(asFileName);
		if (asFileName[0] == L'\\' && asFileName[1] == L'\\')
		{
			if ((asFileName[2] == L'?' || asFileName[2] == L'.') && asFileName[3] == L'\\')
			{
				lbIsFull = true;
			}
			else
			{
				lbIsNetwork = true;
			}
		}
	}
	else
	{
		if (ms_Buffer)
			ms_Buffer[0] = 0;
		mb_IsUNC = false;

		#ifdef MHEAP_DEFINED
		xf_validate(ms_Buffer);
		#endif

		return ms_Buffer ? ms_Buffer : L"";
	}
	// Присвоение своего же буфера
	if (lbIsFull && asFileName == ms_Buffer)
	{
		#ifdef MHEAP_DEFINED
		xf_validate(ms_Buffer);
		#endif
		return ms_Buffer;
	}
	_ASSERTE(asFileName != ms_Buffer);
	// "\\?\" X:\...
	// "\\?\UNC\" server\share\...
	nLen += 12;
	if (!ms_Buffer || !mn_BufferSize || mn_BufferSize <= nLen)
	{
		GetBuffer(nLen);
		//if (ms_Buffer) free(ms_Buffer);
		//ms_Buffer = (wchar_t*)calloc(nLen,sizeof(wchar_t));
		_ASSERTE(ms_Buffer);
	}
#ifdef MHEAP_DEFINED
	xf_validate(ms_Buffer);
#endif
	if (asFileName && *asFileName)
	{
		if (lbIsFull)
		{
			lstrcpy(ms_Buffer, asFileName);
		}
		else
		{
			lstrcpy(ms_Buffer, L"\\\\?\\");
			if (lbIsNetwork)
			{
				lstrcpy(ms_Buffer+4, L"UNC\\");
				lstrcpy(ms_Buffer+8, asFileName+2);
			}
			else
			{
				lstrcpy(ms_Buffer+4, asFileName);
			}
		}

		if (abIsFolder)
		{
			nLen = lstrlen(ms_Buffer); // точно получить длину
			if (nLen && ms_Buffer[nLen-1] != L'\\')
			{
				ms_Buffer[nLen++] = L'\\';
				ms_Buffer[nLen] = 0;
			}
		}
	}
	else if (ms_Buffer)
	{
		ms_Buffer[0] = 0; // иначе - сброс!
	}

	#ifdef MHEAP_DEFINED
	xf_validate(ms_Buffer);
	#endif

	return ms_Buffer;
}


wchar_t* CUnicodeFileName::GetBuffer(int nLen0) // Выделить буфер. nLen0 должен включать завершающий '\0'
{
	if (!ms_Buffer || (nLen0 > (int)mn_BufferSize))
	{
		if (ms_Buffer) free(ms_Buffer);
		mn_BufferSize = nLen0;
		ms_Buffer = (wchar_t*)malloc((nLen0+1)*sizeof(wchar_t));
		_ASSERTE(ms_Buffer);
		ms_Buffer[0] = 0; // очистить
	}
	return ms_Buffer;
}

bool CUnicodeFileName::ReleaseBuffer(bool abIsFolder) // Проверить буфер и если он содержит НЕ UNC - поправить
{
	if (!ms_Buffer || !*ms_Buffer)
		return false;

#ifdef MHEAP_DEFINED
	xf_validate(ms_Buffer);
#endif
		
	bool lbIsFull = false, lbIsNetwork = false;

	if (ms_Buffer[0] == L'\\' && ms_Buffer[1] == L'\\')
	{
		if ((ms_Buffer[2] == L'?' || ms_Buffer[2] == L'.') && ms_Buffer[3] == L'\\')
		{
			lbIsFull = true;
		}
		else
		{
			lbIsNetwork = true;
		}
	}
	
	// Если он не UNC - сконвертируем через Assign
	if (!lbIsFull)
	{
		wchar_t* psz = ms_Buffer; ms_Buffer = NULL; mn_BufferSize = 0;
		Assign ( psz, abIsFolder );
		free(psz);
	}
	
	return (ms_Buffer != NULL);
}

bool CUnicodeFileName::CompareDir(const wchar_t* asDir)
{
	if (!asDir || !*asDir) return false;
	if (!ms_Buffer || !*ms_Buffer) return false;
	// откинуть префиксы
	SkipPrefix(&asDir);
	const wchar_t* psz = ms_Buffer;
	SkipPrefix(&psz);
	// после откидывания префиксов строка могла обнулиться
	if (!asDir || !*asDir) return false;
	if (!psz || !*psz) return false;
	//
	size_t nLen = lstrlen(asDir);
	TODO("Check CompareDir");
	if (_wcsnicmp(asDir, psz, nLen))
		return false;
	// Если нет слеша - слеш должен быть в файле
	if (asDir[nLen-1] != L'\\' && psz[nLen] != L'\\')
		return false;
	return true;
}


CUnicodeFileName& CUnicodeFileName::operator=(const wchar_t* as)
{
    Assign(as);
    return *this;
}
CUnicodeFileName& CUnicodeFileName::operator=(CUnicodeFileName& as)
{
	Assign((LPCWSTR)as);
	return *this;
}
CUnicodeFileName::operator const wchar_t* () const
{
	if (!this) return L"";
	if (!ms_Buffer) return L"";
	return ms_Buffer;
}

// static, helper
// переместить указатель на начало данных, данные не меняются
// то есть пропустить "\\?\", "\\?\UNC" и т.п.
// для путей с диском - останется реальный путь типа "C:\..."
// для сетевых - "\Server\Share\...", то есть без двойного слеша в начале
void CUnicodeFileName::SkipPrefix(const wchar_t** rsPath)
{
	bool lbIsFull = false, lbIsNetwork = false;
	const wchar_t* psz = *rsPath;

	if (psz[0] == L'\\' && psz[1] == L'\\')
	{
		if ((psz[2] == L'?' || psz[2] == L'.') && psz[3] == L'\\')
		{
			psz += 4;
			if (psz[0] == L'U' && psz[1] == L'N' && psz[2] == L'C' && psz[3] == L'\\')
			{
				psz += 3;
			}
		}
		else
		{
			psz ++; // а от сетевой шары отрезать первый слеш
		}
	}
	
	*rsPath = psz;
}

void CUnicodeFileName::Validate()
{
#ifdef MHEAP_DEFINED
	if (ms_Buffer)
		xf_validate(ms_Buffer);
#endif
}
