
/*
Copyright (c) 2009-2010 Maximus5
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
*/

#include <windows.h>
#include "EMUPIC.h"
#include "ShObjIdl_Part.h"
const CLSID CLSID_TaskbarList = {0x56FDF344, 0xFD6D, 0x11d0, {0x95, 0x8A, 0x00, 0x60, 0x97, 0xC9, 0xA0, 0x90}};
const IID IID_ITaskbarList3 = {0xea1afb91, 0x9e28, 0x4b86, {0x90, 0xe9, 0x9e, 0x9f, 0x8a, 0x5e, 0xef, 0xaf}};
const IID IID_ITaskbarList2 = {0x602D4995, 0xB13A, 0x429b, {0xA6, 0x6E, 0x19, 0x35, 0xE4, 0x4F, 0x43, 0x17}};
//extern "C" {
//#include "ConEmuCheck.h"
//}

// === Globals ===
extern HINSTANCE ghInstance;
static BOOL gbInitialized=FALSE;
static HWND ghConEmu=NULL, ghConChild=NULL, ghConsole=NULL, ghLastPicWnd=NULL;
#ifdef _DEBUG
static WNDPROC gfnPicViewProc = NULL;
#endif
//static HMODULE ghConEmuDll=NULL;
static RECT grcShift;
static BOOL gbTerminalMode=FALSE;
static ITaskbarList2 *gpTaskBar2 = NULL;
static const char cFarPicClass[] = "FarPictureViewControlClass";

#ifdef _DEBUG
DWORD gnSlideShowElapse = 2500;
static bool gbPicViewSlideShow = FALSE;
static DWORD dwLastSlideShowTick = 0;
#endif

// === Forwards ===
void InitConEmu();
BOOL GetConEmuShift(LPRECT lprcShift);
#ifdef _DEBUG
LRESULT CALLBACK MyWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Defines
#define SLIDE_TIMER_ID 1999
#define SLIDE_TIMER_RESOLUTION 250
#define isPressed(inp) ((GetKeyState(inp) & 0x8000) == 0x8000)
#endif


BOOL __stdcall OnGetClientRect(HWND hwnd,LPRECT lpRect)
{
    if (hwnd && hwnd==ghConEmu && IsWindow(ghConEmu) && IsWindow(ghConChild))
    {
        hwnd = ghConChild;
    }
    return GetClientRect(hwnd, lpRect);
}

int __stdcall OnMapWindowPoints(HWND hwndFrom,HWND hwndTo,LPPOINT lpPoints,UINT cPoints)
{
    if (hwndFrom==ghConEmu && !hwndTo && GetConEmuShift(&grcShift))
    {
        UINT i=0;
        for (i=0; i<cPoints; i++)
        {
            lpPoints[i].x += grcShift.left;
            lpPoints[i].y += grcShift.top;
        }
    }
    return MapWindowPoints(hwndFrom,hwndTo,lpPoints,cPoints);
}


HWND __stdcall OnCreateWindowExA(DWORD dwExStyle,LPCSTR lpClassName,LPCSTR lpWindowName,DWORD dwStyle,int x,int y,int nWidth,int nHeight,HWND hWndParent,HMENU hMenu,HINSTANCE hInstance,LPVOID lpParam)
{
    HWND result = NULL;
    BOOL lbOurWnd = FALSE;
    if (gbTerminalMode) {
	    return NULL;
    }

	if (lpClassName && lstrcmpA(lpClassName,cFarPicClass)==0) {
		lbOurWnd = TRUE;
		if (GetConEmuShift(&grcShift))
		{
			x += grcShift.left;
			y += grcShift.top;
		}
    }
    
    result = CreateWindowExA(dwExStyle,lpClassName,lpWindowName,dwStyle,x,y,nWidth,nHeight,hWndParent,hMenu,hInstance,lpParam);
    if (lbOurWnd)
        ghLastPicWnd = result;
    return result;
}

BOOL __stdcall OnMoveWindow(HWND hWnd,int X,int Y,int nWidth,int nHeight,BOOL bRepaint)
{
    BOOL result = FALSE;

    // В FullScreen хорошо бы не двигать окошко, а то заголовок окна виден, но как его отсечь?
	if (hWnd==ghLastPicWnd)
	{
		RECT rcClient; GetClientRect(ghConEmu, &rcClient);
		if (nWidth<=rcClient.right && nHeight<=rcClient.bottom && GetConEmuShift(&grcShift))
		{
			#ifdef _DEBUG
			DWORD dwStyle = GetWindowLongA(hWnd, GWL_STYLE);
			#endif
			X += grcShift.left;
			Y += grcShift.top;
		}
    }
    result = MoveWindow(hWnd,X,Y,nWidth,nHeight,TRUE/*bRepaint*/);
    return result;
}

BOOL __stdcall OnShowWindow(HWND hWnd,int nCmdShow)
{
	char szClassName[128];
	if (GetClassNameA(hWnd, szClassName, 128) && lstrcmpiA(szClassName,"Shell_TrayWnd")==0) {
		static bool bInitialized = false;
		if (!bInitialized) {
	        HRESULT hr = S_OK;
	        hr = OleInitialize (NULL); // как бы попробовать включать Ole только во время драга. кажется что из-за него глючит переключалка языка
			if (!gpTaskBar2) {
				// В PostCreate это выполняется дольше всего. По идее мешать не должно,
				// т.к. серверная нить уже запущена.
				hr = CoCreateInstance(CLSID_TaskbarList,NULL,CLSCTX_INPROC_SERVER,IID_ITaskbarList2,(void**)&gpTaskBar2);
				if (hr == S_OK && gpTaskBar2) {
					hr = gpTaskBar2->HrInit();
				}
				if (hr != S_OK && gpTaskBar2) {
					if (gpTaskBar2) gpTaskBar2->Release();
					gpTaskBar2 = NULL;
				}
			}
		}

		if (gpTaskBar2) {
			if (ghLastPicWnd) {
				gpTaskBar2->MarkFullscreenWindow(ghLastPicWnd, (nCmdShow == SW_HIDE));
			}
		}

		return TRUE;
	}
	return ShowWindow(hWnd, nCmdShow);
}

#ifdef _DEBUG
typedef BOOL (__stdcall* FReadConsoleInputExW)(DWORD a1,DWORD a2,DWORD a3,DWORD a4,DWORD a5);
FReadConsoleInputExW ReadConsoleInputExW = NULL;
//BOOL __stdcall OnReadConsoleInputW(HANDLE hConsoleInput, PINPUT_RECORD lpBuffer, DWORD nLength, LPDWORD lpNumberOfEventsRead)
BOOL __stdcall OnReadConsoleInputExW(HANDLE hConsoleInput, PINPUT_RECORD lpBuffer, DWORD nLength, LPDWORD lpNumberOfEventsRead, DWORD dwReserved) //(DWORD a1,DWORD a2,DWORD a3,DWORD a4,DWORD a5)
{
	//BOOL lbRc = ReadConsoleInputExW(a1,a2,a3,a4,a5);

	INPUT_RECORD r[2];
	DWORD nRead = 0;

	BOOL lbRc = ReadConsoleInputW(hConsoleInput, r, 1, &nRead);
	
	if (lbRc && ghLastPicWnd && nRead) {
		if (r->EventType == KEY_EVENT) {
			if (r->Event.KeyEvent.wVirtualKeyCode == VK_PAUSE)
			{
				nRead = 0; r->EventType = 0; // в PicView не пускать
				
				if (!r->Event.KeyEvent.bKeyDown) {
	                gbPicViewSlideShow = !gbPicViewSlideShow;
	                if (gbPicViewSlideShow) {
	                    if (gnSlideShowElapse<=500) gnSlideShowElapse=500;
	                    dwLastSlideShowTick = GetTickCount() - gnSlideShowElapse;
	                    SetTimer(ghLastPicWnd, SLIDE_TIMER_ID, SLIDE_TIMER_RESOLUTION, NULL);
	                } else {
	                	KillTimer(ghLastPicWnd, SLIDE_TIMER_ID);
	                }
				}
				
			} else if (gbPicViewSlideShow) {
				if (r->Event.KeyEvent.wVirtualKeyCode==0xbd/* -_ */ 
						|| r->Event.KeyEvent.wVirtualKeyCode==0xbb/* =+ */)
				{
					nRead = 0; r->EventType = 0; // в PicView не пускать
					
	                if (r->Event.KeyEvent.bKeyDown) {
	                    if (r->Event.KeyEvent.wVirtualKeyCode == 0xbb)
	                        gnSlideShowElapse = (DWORD)(1.2 * gnSlideShowElapse);
	                    else {
	                        gnSlideShowElapse = (DWORD)(gnSlideShowElapse / 1.2);
	                        if (gnSlideShowElapse<=500) gnSlideShowElapse=500;
	                    }
	                }
	                
				} else if (r->Event.KeyEvent.wVirtualKeyCode != VK_NEXT) {
					gbPicViewSlideShow = false; // отмена слайдшоу
					KillTimer(ghLastPicWnd, SLIDE_TIMER_ID);
				}
	        }
	    }
	}

	if (!nRead) {
		lbRc = FALSE;
	} else {
		if (!lpBuffer || !nLength || !lpNumberOfEventsRead) {
			lbRc = FALSE; //_ASSERTE?
		} else {
			if (IsBadWritePtr(lpBuffer, sizeof(INPUT_RECORD)) || IsBadWritePtr(lpNumberOfEventsRead,sizeof(DWORD))) {
				lbRc = FALSE; //_ASSERTE?
			} else {
				// OK
				*lpNumberOfEventsRead = nRead;
				*lpBuffer = *r;
			}
		}
	}
	
	return lbRc;
}

ATOM __stdcall OnRegisterClassA(CONST WNDCLASSA *lpWndClass)
{
	WNDCLASSA wc = *lpWndClass;
	
	if (lstrcmpA(wc.lpszClassName,cFarPicClass)==0) {
		gfnPicViewProc = wc.lpfnWndProc;
		wc.lpfnWndProc = MyWndProc;
		//wc.hInstance = ghInstance;
	}
	
	ATOM h = RegisterClassA(&wc);
	return h;
}

#endif








BOOL GetConEmuShift(LPRECT lprcShift)
{
    BOOL result = FALSE;
    
    if (ghConEmu && IsWindow(ghConEmu) && IsWindow(ghConChild))
    {
        RECT rcMain, rcChild;
        //char szDebug[255];
        GetClientRect(ghConEmu, &rcMain);
        GetClientRect(ghConChild, &rcChild);
        //sprintf(szDebug, "(%i-%i)x(%i-%i)\r\n",rcChild.left,rcChild.top,rcChild.right,rcChild.bottom);
        MapWindowPoints(ghConChild, ghConEmu, (LPPOINT)&rcChild, 2);
        //sprintf(szDebug+strlen(szDebug), "(%i-%i)x(%i-%i)",rcChild.left,rcChild.top,rcChild.right,rcChild.bottom);
        //MessageBoxA(0,   szDebug,"ConEmuDebug",0);
    
        if (rcChild.left || rcChild.top ||
            (rcChild.right!=rcMain.right) || (rcChild.bottom!=rcMain.bottom))
        {
            lprcShift->left = rcChild.left;
            lprcShift->top = rcChild.top;
            lprcShift->right = rcMain.right - rcChild.right;
            lprcShift->bottom = rcMain.bottom - rcChild.bottom;

            result = TRUE;
        }
    }
    
    return result;
}

void InitConEmu(HMODULE ahPicView, HWND ahConEmu)
{
    //HMODULE hUser32=NULL;
    wchar_t szClass[64];
    //int nChk = 0;
    
    gbInitialized=TRUE;
    
    // Check Terminal mode
    szClass[0] = 0;
    if (GetEnvironmentVariableW(L"TERM", szClass, 63)) {
	    gbTerminalMode = TRUE;
        //lstrcpy(gsTermMsg, _T("PictureView wrapper\nPicture viewing is not available\nin terminal mode ("));
        //lstrcat(gsTermMsg, szVarValue);
        //lstrcat(gsTermMsg, _T(")"));
    }

    // Real console window handle
    ghConsole = GetConsoleWindow();

#ifdef _DEBUG    
    if (!ReadConsoleInputExW) {
    	ReadConsoleInputExW = (FReadConsoleInputExW)GetProcAddress(GetModuleHandle("Kernel32.dll"),"ReadConsoleInputExW");
    }
#endif
    

    //nChk = ConEmuCheck ( &ghConChild );
    //if (nChk!=0 || !ghConChild)
	//    return;
	ghConChild = ahConEmu;
    
    ghConEmu = GetAncestor(ghConChild, GA_PARENT);
    if (!ghConEmu) return;
    GetClassNameW(ghConEmu, szClass, 63);
    if (lstrcmpW(szClass, VirtualConsoleClassMain)!=0) {
        // видимо старый ConEmu? не будем обрабатывать
        //if (!gbTerminalMode)
	    //    MessageBoxA(ghConChild, "ConEmu old version detected!\r\nPlease upgrade!", "PictureView wrapper", MB_ICONSTOP);
        ghConEmu = NULL;
        return;
    }
    
    // Все, инициализация завершена
}

#ifdef _DEBUG
LRESULT CALLBACK MyWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	//TODO: Перехват таймеров
	if (uMsg == WM_TIMER) {
		if (wParam == SLIDE_TIMER_ID) {
		
	        if (gbPicViewSlideShow) {
	            DWORD dwTicks = GetTickCount();
	            DWORD dwElapse = dwTicks - dwLastSlideShowTick;
	            if (dwElapse > gnSlideShowElapse)
	            {
	                if (IsWindow(ghLastPicWnd)) {
	                    //
	                    gbPicViewSlideShow = false;
	                    
	                    SendMessage(ghConsole, WM_KEYDOWN, VK_NEXT, 0x01510001);
	                    SendMessage(ghConsole, WM_KEYUP, VK_NEXT, 0xc1510001);
	                    
	                    dwLastSlideShowTick = GetTickCount();
	                    gbPicViewSlideShow = true;
	                } else {
	                    ghLastPicWnd = NULL;
	                    gbPicViewSlideShow = false;
	                }
	            }
	        }
		
			return 0;
		}
	}

	//if (uMsg == WM_KEYDOWN || uMsg == WM_KEYUP) {
	//	if (wParam == VK_PAUSE)
	//	{
	//		if (uMsg == WM_KEYUP) {
	//			gbPicViewSlideShow = !gbPicViewSlideShow;
	//			if (gbPicViewSlideShow) {
	//				if (gnSlideShowElapse<=500) gnSlideShowElapse=500;
	//				dwLastSlideShowTick = GetTickCount() - gnSlideShowElapse;
	//				SetTimer(ghLastPicWnd, SLIDE_TIMER_ID, SLIDE_TIMER_RESOLUTION, NULL);
	//			} else {
	//				KillTimer(ghLastPicWnd, SLIDE_TIMER_ID);
	//			}
	//		}

	//		return 0;

	//	} else if (gbPicViewSlideShow) {
	//		if (wParam==0xbd/* -_ */ || wParam==0xbb/* =+ */)
	//		{
	//			if (uMsg == WM_KEYDOWN) {
	//				if (wParam == 0xbb)
	//					gnSlideShowElapse = (DWORD)(1.2 * gnSlideShowElapse);
	//				else {
	//					gnSlideShowElapse = (DWORD)(gnSlideShowElapse / 1.2);
	//					if (gnSlideShowElapse<=500) gnSlideShowElapse=500;
	//				}
	//			}

	//			return 0;

	//		}
	//		else
	//		{
	//			gbPicViewSlideShow = false; // отмена слайдшоу
	//			KillTimer(ghLastPicWnd, SLIDE_TIMER_ID);
	//		}
	//	}
	//}



	// Оригинальная процедура
	if (gfnPicViewProc)
		return gfnPicViewProc(hwnd, uMsg, wParam, lParam);
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
#endif


/* Используются как extern в ConEmuCheck.cpp */
LPVOID _calloc(size_t nCount,size_t nSize) {
	return calloc(nCount,nSize);
}
LPVOID _malloc(size_t nCount) {
	return malloc(nCount);
}
void   _free(LPVOID ptr) {
	free(ptr);
}
