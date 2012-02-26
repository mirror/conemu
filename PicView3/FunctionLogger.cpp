
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
#include "PictureView_Lang.h"
#include "FunctionLogger.h"

CFunctionLogger::~CFunctionLogger()
{
	WriteLog(sInfo);
}
CFunctionLogger::CFunctionLogger(LPCWSTR asFunc)
{
	sInfo[0] = L'~';
	lstrcpyn(sInfo+1, asFunc, MAX_PATH);
	WriteLog(sInfo+1);
}
void CFunctionLogger::FunctionLogger(LPCWSTR asFunc)
{
	WriteLog(asFunc);
}
CFunctionLogger::CFunctionLogger(LPCWSTR asFuncFormat, int nArg1)
{
	sInfo[0] = 0;
	if (!*g_Plugin.sLogFile)
		return;

	sInfo[0] = L'~'; sInfo[1] = 0;
	wsprintf(sInfo+1, asFuncFormat, nArg1);
	WriteLog(sInfo+1);
}
void CFunctionLogger::FunctionLogger(LPCWSTR asFuncFormat, int nArg1)
{
	if (!*g_Plugin.sLogFile)
		return;

	wchar_t sInfo[MAX_PATH];
	wsprintf(sInfo, asFuncFormat, nArg1);
	WriteLog(sInfo);
}
CFunctionLogger::CFunctionLogger(LPCWSTR asFuncFormat, LPCWSTR asArg1)
{
	sInfo[0] = 0;
	if (!*g_Plugin.sLogFile)
		return;

	sInfo[0] = L'~'; sInfo[1] = 0;	
	if ((lstrlen(asFuncFormat) + lstrlen(asArg1)) <= MAX_PATH) {
		wsprintf(sInfo+1, asFuncFormat, asArg1);
	} else {
		lstrcpyn(sInfo+1, asFuncFormat, MAX_PATH);
	}
	WriteLog(sInfo+1);
}
void CFunctionLogger::FunctionLogger(LPCWSTR asFuncFormat, LPCWSTR asArg1)
{
	if (!*g_Plugin.sLogFile)
		return;

	wchar_t sInfo[MAX_PATH];
	if ((lstrlen(asFuncFormat) + lstrlen(asArg1)) <= MAX_PATH) {
		wsprintf(sInfo, asFuncFormat, asArg1);
	} else {
		lstrcpyn(sInfo, asFuncFormat, MAX_PATH);
	}
	WriteLog(sInfo);
}
void CFunctionLogger::WriteLog(LPCWSTR pszText)
{
	if (!*g_Plugin.sLogFile || !pszText || !*pszText)
		return;

	{
		const wchar_t *szThread = L"[Unkn] ";
		DWORD nID = GetCurrentThreadId();
		if (nID == gnDisplayThreadId)
			szThread = L"[Disp] ";
		else if (nID == gnMainThreadId)
			szThread = L"[Main] ";
		else if (nID == gnDecoderThreadId)
			szThread = L"[Dec.] ";
		DWORD nWrite = 0;
		wchar_t szTemp[2048];
		lstrcpy(szTemp, szThread);
		lstrcpyn(szTemp+7, pszText, 2000); nWrite += lstrlen(szTemp);
		szTemp[nWrite++] = L'\r'; szTemp[nWrite++] = L'\n'; szTemp[nWrite] = 0;

		HANDLE hFile = CreateFile(g_Plugin.sLogFile, GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
		
		if (hFile != INVALID_HANDLE_VALUE) {
			SetFilePointer(hFile, 0,0, FILE_END);
			WriteFile(hFile, szTemp, nWrite*2, &nWrite, 0);
			#ifdef _DEBUG
			OutputDebugString(szTemp);
			#endif
			//WriteFile(hFile, szThread, wcslen(szThread)*2, &nWrite, 0);
			//WriteFile(hFile, pszText, wcslen(pszText)*2, &nWrite, 0);
			//WriteFile(hFile, L"\r\n", 4, &nWrite, 0);
			CloseHandle(hFile);
		}
	}
}
