
#include <windows.h>
#include <TCHAR.H>
#include "ConEmuSupport.h"

typedef HWND (WINAPI *FGetFarHWND2)(BOOL abConEmuOnly);
FGetFarHWND2 ConEmuGetFarHWND2 = NULL;
HMODULE ghConEmuDll = NULL;

HWND GetConEmuHwnd()
{
	if (!ConEmuGetFarHWND2) {
		if (!ghConEmuDll)
			ghConEmuDll = GetModuleHandle(_T("conemu.dll"));
		if (!ghConEmuDll)
			ghConEmuDll = GetModuleHandle(_T("conemu.x64.dll"));

		if (ghConEmuDll)
			ConEmuGetFarHWND2 = (FGetFarHWND2)GetProcAddress(ghConEmuDll,"GetFarHWND2");
	}
	if (ConEmuGetFarHWND2)
		return ConEmuGetFarHWND2(TRUE);
	return NULL;
}
