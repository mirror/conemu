
#pragma once

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


struct FileMap
{
	HANDLE hFile, hMapping;
	i64 lSize;
	BYTE *pMapping;

	FileMap(const wchar_t *pName)
	{
		hFile = hMapping = INVALID_HANDLE_VALUE;
		pMapping = NULL;
		if ((hFile = CreateFileW(pName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) != INVALID_HANDLE_VALUE)
		{
			*(u32*)&lSize = GetFileSize(hFile, (u32*)&lSize + 1);
			if ((u32)lSize == INVALID_FILE_SIZE && GetLastError() != NO_ERROR)
				lSize = 0;
			if (!(hMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL)))
				CloseHandle(hFile), hFile = INVALID_HANDLE_VALUE;
		}
	}
	~FileMap(void)
	{
		if (pMapping)
			UnmapViewOfFile(pMapping), pMapping = NULL;
		if (hMapping != INVALID_HANDLE_VALUE)
			CloseHandle(hMapping), hMapping = INVALID_HANDLE_VALUE;
		if (hFile != INVALID_HANDLE_VALUE)
			CloseHandle(hFile), hFile = INVALID_HANDLE_VALUE;
	}
	BYTE *MapView(void)
	{
		if (hFile == INVALID_HANDLE_VALUE || !hFile)
			return NULL;
		if (hMapping == INVALID_HANDLE_VALUE || !hMapping)
			return NULL;
		return pMapping = (BYTE*)MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0);
	}
};
