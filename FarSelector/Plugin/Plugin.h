#pragma once

#define SZ_PLUGIN_PIPE _T("\\\\.\\pipe\\FarSelectorPipe.%u")

#define PIPEBUFSIZE 4096

#define SafeCloseHandle(h) { if ((h)!=NULL) { HANDLE hh = (h); (h) = NULL; if (hh!=INVALID_HANDLE_VALUE) CloseHandle(hh); } }

typedef struct tag_SelectorStr {
	DWORD   nAllSize;
	DWORD   nCmd;       // 1 - получить инфу, 2 - передать параметры в открытую копию ФАР
	BOOL    bActiveFar; // плагин возвращает TRUE, если его ФАР активен (активная вкладка в ConEmu, или верхняя видимая консоль)
	DWORD   hWnd;       // HWND или консольного окна, или главного окна ConEmu
	// Далее - данные
	DWORD   nDataLen; // количество wchar_t
	wchar_t szData[66000];
} SELECTORSTR;
