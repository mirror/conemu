
/*
Copyright (c) 2009 ConEmu.Maximus5@gmail.com
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

THIS SOFTWARE IS PROVIDED BY THE AUTHOR 'AS IS' AND ANY EXPRESS OR
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

#include <windows.h>
#include "pluginW.hpp"


#define MAKEFARVERSION(major,minor,build) ( ((major)<<8) | (minor) | ((build)<<16))

struct PluginStartupInfo psi;
struct FarStandardFunctions fsf;


BOOL APIENTRY DllMain( HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved )
{
	return TRUE;
}

#if defined(__GNUC__)
#define CRTSTARTUP
#else
#define DllMainCRTStartup _DllMainCRTStartup
#endif

#if defined(CRTSTARTUP)

#ifdef __cplusplus
extern "C"{
#endif
  BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved);
  #if defined(__GNUC__)
  void WINAPI SetStartupInfoW(struct PluginStartupInfo *aInfo);
  #endif
#ifdef __cplusplus
};
#endif

BOOL WINAPI _DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved)
{
  return DllMain(hDll, dwReason,lpReserved);
}
#endif


// minimal(?) FAR version 2.0 alpha build 1587
int WINAPI _export GetMinFarVersionW(void)
{
	return FARMANAGERVERSION;
}

void WINAPI _export GetPluginInfoW(struct PluginInfo *pi)
{
    static WCHAR *szMenu[1], szMenu1[255];
    szMenu[0] = szMenu1;
    lstrcpyW(szMenu[0], L"Clipboard fixer");

	pi->StructSize = sizeof(struct PluginInfo);
	pi->Flags = PF_EDITOR | PF_DIALOG;
	pi->DiskMenuStrings = NULL;
	pi->DiskMenuNumbers = 0;
	pi->PluginMenuStrings = szMenu;
	pi->PluginMenuStringsNumber = 1;
	pi->PluginConfigStrings = NULL;
	pi->PluginConfigStringsNumber = 0;
	pi->CommandPrefix = NULL;
	pi->Reserved = 0x436C4678;
}

HANDLE WINAPI _export OpenPluginW(int OpenFrom,INT_PTR Item)
{
	if (OpenClipboard(NULL))
	{
		if (CF_TEXT == EnumClipboardFormats(0))
		{
			if (IsClipboardFormatAvailable(CF_UNICODETEXT))
			{
				HANDLE hText = GetClipboardData(CF_TEXT);
				HANDLE hUText = GetClipboardData(CF_UNICODETEXT);
				if (hText && hUText) {
					LPCSTR pszText = (LPCSTR)GlobalLock(hText);
					LPCWSTR pwszText = (LPCWSTR)GlobalLock(hUText);
					if (pszText && pwszText)
					{
						BOOL lbNeedFix = FALSE;
						LPCSTR psz = pszText;
						LPCWSTR pwsz = pwszText;
						while (*psz && *pwsz)
						{
							if (*psz > 127 && *pwsz == *psz)
							{
								lbNeedFix = TRUE;
								break;
							}
							psz++; pwsz++;
						}
						if (lbNeedFix)
						{
							if (pwszText) { GlobalUnlock(hUText); pwszText = NULL; }
							size_t nLen = lstrlenA(pszText);
							if ((hUText = GlobalAlloc(GMEM_MOVEABLE, (nLen+1)*2)) != NULL)
							{
								wchar_t *pwszNew = (wchar_t*)GlobalLock(hUText);
								if (pwszNew)
								{
									MultiByteToWideChar(CP_ACP, 0,
										pszText, nLen+1, pwszNew, nLen+1);
									GlobalUnlock(hUText);
									SetClipboardData(CF_UNICODETEXT, hUText);
								}
								else
								{
									// неудача
									GlobalUnlock(hUText);
									GlobalFree(hUText);
								}
							}
						}
					}
					if (pszText) GlobalUnlock(hText);
					if (pwszText) GlobalUnlock(hUText);
				}
			}
		}
		CloseClipboard();
	}
	return INVALID_HANDLE_VALUE;
}

void WINAPI _export SetStartupInfoW(struct PluginStartupInfo *aInfo)
{
	::psi = *((struct PluginStartupInfo*)aInfo);
	::fsf = *((struct PluginStartupInfo*)aInfo)->FSF;
	::psi.FSF = &::fsf;
}
