
#pragma once

extern HINSTANCE g_hInstance;
extern HWND FarWindow, ConEmuWnd, ghPopup, RootWnd;
extern BOOL gbPopupWasHidden;
extern BITMAPINFOHEADER BmpHeader;
extern unsigned char * DibBuffer;
extern OSVERSIONINFO Version;
extern RECT ConsoleRect, DCRect, RangedRect;
extern bool QView;
extern COLORREF gnBackColor;
