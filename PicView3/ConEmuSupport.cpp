
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


#include <windows.h>
#include <TCHAR.H>
#include "ConEmuSupport.h"

typedef HWND (WINAPI *GetFarHWND2_t)(BOOL abConEmuOnly);
GetFarHWND2_t ConEmuGetFarHWND2 = NULL;
typedef HWND (WINAPI *GetRealConsoleWindow_t)();
GetRealConsoleWindow_t GetRealConsoleWindow = NULL;
HMODULE ghConEmuDll = NULL;
HMODULE ghConEmuHkDll = NULL;

BOOL GetConEmuHwnd(HWND& hConEmu, HWND& hConEmuRoot, HWND& hConWnd)
{
	//Поскольку плагин conemu может быть выгружен - проверяем его всегда
	//if (!ConEmuGetFarHWND2)
	{
		#ifdef _WIN64
		ghConEmuDll = GetModuleHandle(_T("conemu.x64.dll"));
		if (!ghConEmuDll)
			ghConEmuDll = GetModuleHandle(_T("conemu.dll"));
		#else
		ghConEmuDll = GetModuleHandle(_T("conemu.dll"));
		#endif

		if (ghConEmuDll)
			ConEmuGetFarHWND2 = (GetFarHWND2_t)GetProcAddress(ghConEmuDll, "GetFarHWND2");
		
		#ifdef _WIN64
		ghConEmuHkDll = GetModuleHandle(_T("ConEmuHk64.dll"));
		if (!ghConEmuHkDll)
			ghConEmuHkDll = GetModuleHandle(_T("ConEmuHk.dll"));
		#else
		ghConEmuHkDll = GetModuleHandle(_T("ConEmuHk.dll"));
		#endif
		
		if (ghConEmuHkDll)
			GetRealConsoleWindow = (GetRealConsoleWindow_t)GetProcAddress(ghConEmuHkDll, "GetRealConsoleWindow");
	}
	
	if (ConEmuGetFarHWND2)
	{
		hConEmu = ConEmuGetFarHWND2(1);
		hConEmuRoot = ConEmuGetFarHWND2(2);
		
		if (GetRealConsoleWindow)
			hConWnd = GetRealConsoleWindow();
		else
			hConWnd = GetConsoleWindow();
		
		if (hConEmu && hConEmu != hConWnd)
		{
			// Старая версия ConEmu?
			if (hConEmuRoot == hConEmu)
				hConEmuRoot = GetParent(hConEmu);
		}
	}
	else
	{
		hConEmu = hConEmuRoot = NULL;
		hConWnd = GetConsoleWindow();
	}
	return (hConEmu != NULL);
}
