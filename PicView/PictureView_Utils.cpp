/**************************************************************************
Copyright (c) 2009 Skakov Pavel
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

const wchar_t *GetMsg(int MsgId)
{
	return g_StartupInfo.GetMsg(g_StartupInfo.ModuleNumber, MsgId);
}

void RegKeyRead(HKEY RegKey, const wchar_t* const Name, bool *const Param, const bool Default)
{
	u32 len = 4, val;
	*Param = RegQueryValueExW(RegKey, Name, NULL, NULL, (LPBYTE)&val, (LPDWORD)&len) ? Default : val;
}

void RegKeyRead(HKEY RegKey, const wchar_t* const Name, u32 *const Param, const u32 Default)
{
	u32 len = 4;
	if (RegQueryValueExW(RegKey, Name, NULL, NULL, (LPBYTE)Param, (LPDWORD)&len))
		*Param = Default;
}

LONG RegKeyWrite(HKEY RegKey, const wchar_t* const Name, const u32 Param)
{
	return RegSetValueExW(RegKey, Name, 0, REG_DWORD, (const BYTE*) &Param, 4);
}

// Выделить память и слепить полный путь к "Dir\Name".
// Dir не обязан содержать на конце слэш
// Если abReverseName==true - то при наличии в Name обратных слэшей - они заменяются на прямые (для реестра)
wchar_t* ConcatPath(const wchar_t* Dir, const wchar_t* Name, bool abReverseName/*=false*/)
{
	_ASSERTE(Dir && Name); _ASSERTE(*Dir && *Name);
	size_t nDirLen = lstrlen(Dir);
	size_t nNameLen = lstrlen(Name);

	wchar_t *pszFull = (wchar_t*)calloc(nDirLen+nNameLen+2,sizeof(wchar_t)); // + '\0' + '\\'
	if (!pszFull) {
		_ASSERTE(pszFull);
		return NULL;
	}

	lstrcpy(pszFull, Dir);
	if (pszFull[nDirLen-1] != L'\\') {
		pszFull[nDirLen++] = L'\\';
	}
	lstrcpy(pszFull+nDirLen, Name);

	if (abReverseName) {
		wchar_t* psz = wcschr(pszFull+nDirLen, L'\\');
		while (psz) {
			*psz = '/';
			psz = wcschr(psz+1, L'\\');
		}
	}

	return pszFull;
}
