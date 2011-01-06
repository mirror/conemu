
#include <windows.h>
#include <TCHAR.H>
#include "MultiDef.h"
#include "MultiGlobal.h"

#ifdef _DEBUG
#define NO_TOP_MOST
#endif

const TCHAR g_WndClassName[] = _T("FarMultiViewControlClass");
bool gbFullScreenNow = false;
bool gbChildWindowMode = false;
extern bool gbVideoExtMatch, gbAudioExtMatch, gbPictureExtMatch, gbVideoFullWindow;
DWORD gnSkipSetFocus = 0;
RECT gRcParent = {0,0,0,0};
extern LONGLONG SecondSeeking;

// Local forward definitions
LRESULT CALLBACK WindowProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);
DWORD WINAPI MViewWindowThread(void *pParameter);
void SwitchFullScreen(bool abFullScreen);
void GetRangedRect(RECT *pRangedRect, const COORD* coord, const bool* VideoFullWindow);
//extern void SetTimePosition(LONGLONG newtime, int type);
extern bool IsKeyPressed(WORD vk);

#ifdef _DEBUG
HWND ghLastFore=NULL, ghLastFocus=NULL;
void LogFore(LPCSTR asInfo)
{
	char szClass[128], szDbg[255];
	HWND h = GetForegroundWindow();
	bool lb = false;
	if (h!=ghLastFore) {
		ghLastFore = h; lb = true;
		if (!h) szClass[0]=0; else GetClassNameA(h, szClass, NUM_ITEMS(szClass));
		wsprintfA(szDbg, "Foreground changed to: 0x%08X {%s} ", (DWORD)h, szClass);
		if (h==RootWnd || h==ghPopup)
			OutputDebugStringA(szDbg);
		else
			OutputDebugStringA(szDbg);
	}
	h = GetFocus();
	if (h!=ghLastFocus) {
		ghLastFocus = h; lb = true;
		if (!h) szClass[0]=0; else GetClassNameA(h, szClass, NUM_ITEMS(szClass));
		wsprintfA(szDbg, "Focus changed to: 0x%08X {%s} ", (DWORD)h, szClass);
		OutputDebugStringA(szDbg);
	}
	if (lb) {
		if (asInfo) OutputDebugStringA(asInfo);
		OutputDebugStringA("\n");
	}
}
#else
#define LogFore(asInfo)
#endif


// Functions
RECT FillWindowRectGet()
{
	RECT rect;
	if (QView) {
		rect = DCRect;
	} else {
		GetClientRect(FarWindow, &rect);
	}
	//if (ConEmuWnd == NULL)
	if (gbChildWindowMode) {
		// Готовимся к тому, что "окно отрисовки" станет невидимым
		if (ConEmuWnd != RootWnd)
			MapWindowPoints(ConEmuWnd, RootWnd, (LPPOINT)&rect, 2);
	} else {
		MapWindowPoints(FarWindow, NULL, (LPPOINT)&rect, 2);
	}
	return rect;
}

bool FillWindowCreate(bool abModal)
{
	LogFore("FillWindowCreate.begin");

	if (ghPopup && IsWindow(ghPopup))
		return true; // уже создано

	_ASSERTE(ghPopup==NULL);

	static ATOM hClass = NULL;
	if (!hClass) {
		const WNDCLASS wc = {CS_OWNDC | CS_DBLCLKS, WindowProc, 0, 32, g_hInstance,
			NULL, LoadCursor(NULL, IDC_ARROW), NULL/*(HBRUSH)COLOR_BACKGROUND*/, NULL,
			g_WndClassName};
		hClass = RegisterClass(&wc);
	}

	gbFullScreenNow = false;
	gnSkipSetFocus = 0;

    HANDLE hEvent[2] = {CreateEvent(0,0,0,0),0};
    DWORD  dwTID = 0;
    hEvent[1] = CreateThread(0,0,MViewWindowThread,hEvent[0],0,&dwTID);
    // Если нить удалось создать - подождем, пока она запустится
    if (hEvent[1]) {
    	DWORD nWait = WaitForMultipleObjects(2, hEvent, FALSE, INFINITE);
    	if (nWait) {
    		// Значит нить аварийно завершилась?
		}
		CloseHandle(hEvent[1]); hEvent[1] = 0;
    }
    CloseHandle(hEvent[0]); hEvent[0] = 0;
	gbPopupWasHidden = FALSE;

	TODO("Если станет модальным - нужно ставить 2");
	if (ghPopup) {
		SetWindowLongPtr(ghPopup, 0, abModal ? 0x200 : 0x100);
	}

	LogFore("FillWindowCreate.ok");

	return (ghPopup != NULL);
}

void FillWindowClose()
{
	if (ghPopup && IsWindow(ghPopup))
		SendMessage(ghPopup, WM_CLOSE, 0,0);
	gbPopupWasHidden = FALSE;
	gbFullScreenNow = false;
}

void FillWindowShow(BOOL abShow)
{
	if (ghPopup && IsWindow(ghPopup)) {
		if (IsWindowVisible(ghPopup) != abShow)
			ShowWindow(ghPopup, abShow ? SW_SHOW : SW_HIDE);
	}
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 0;
	switch (messg)
	{
	case WM_CREATE:
		memset(&gRcParent, 0, sizeof(gRcParent));
		SetTimer(hWnd, 101, 100, 0);
		break;
	case WM_TIMER:
	{
		// Если окно FAR двигалось
		RECT rcFarRect; GetWindowRect(FarWindow, &rcFarRect);
		if (gRcParent.left != rcFarRect.left || gRcParent.top != rcFarRect.top
			|| gRcParent.right != rcFarRect.right || gRcParent.bottom != rcFarRect.bottom)
		{
			gRcParent = rcFarRect;
			// Нужно подвинуть окошко
			RECT rect = FillWindowRectGet();
			// После первого же MoveWindow наш popup оказывается ПОД фаром :( Если не ставить WS_EX_TOPMOST
			//MoveWindow(hWnd, rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top, 1);
			SetWindowPos(ghPopup, HWND_TOP, rect.left,rect.top,
				rect.right-rect.left,rect.bottom-rect.top, 0);
		}
		//if (ConEmuWnd == NULL && !gbFullScreenNow)
		if (!gbChildWindowMode && !gbFullScreenNow)
		{
			// Т.к. "скывающее" окно у нас TOPMOST - его нужно прятать, при переключении в другое приложение
			static HWND hLastFore = NULL;
			HWND hFore = GetForegroundWindow();

			if ((hFore == ghPopup)) {
				if ((gnSkipSetFocus == 0)
					|| (gnSkipSetFocus != 1 && ((GetTickCount() - gnSkipSetFocus) >= 1000)))
				{
					SetForegroundWindow(RootWnd);
					gnSkipSetFocus = 1;
					hFore = GetForegroundWindow();
				}
			}

			if (hFore && hFore != hLastFore) {
				DWORD dwPID = 0;
				GetWindowThreadProcessId(hFore, &dwPID);
				if (dwPID != GetCurrentProcessId() && hFore != RootWnd) {
					// В фокусе окно чужого процесса и это НЕ conemu
					if (IsWindowVisible(ghPopup)) {
						FillWindowShow(FALSE);
					}
				} else {
					if (!gbPopupWasHidden && !IsWindowVisible(ghPopup)) {
						FillWindowShow(TRUE);
						hFore = GetForegroundWindow();
						if (hFore != RootWnd)
							SetForegroundWindow(RootWnd);
					}
				}
				hLastFore = hFore;
			}
		}
		break;
	}
	case WM_KEYUP:
	{
		if (wParam == VK_ESCAPE) {
			if (gbFullScreenNow) {
				gbFullScreenNow = false;
				gnSkipSetFocus = 0;
				SwitchFullScreen(false);
				SetForegroundWindow(RootWnd);
			}
		}
		break;
	}
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	{
		if (gbFullScreenNow && FarVideo) {
			// В FullScreen не работают макросы. Сделать хотя-бы прокрутку с паузой
			switch(wParam) {
			case 'F':
				{
					if (gbFullScreenNow) {
						gbFullScreenNow = false;
						gnSkipSetFocus = 0;
						SwitchFullScreen(false);
						SetForegroundWindow(RootWnd);
					}
				} break;
			case VK_LEFT:
			case VK_PRIOR:
				FarVideo->JumpRelative(false, (wParam == VK_PRIOR) || IsKeyPressed(VK_CONTROL));
				break;
			case VK_RIGHT:
			case VK_NEXT:
				FarVideo->JumpRelative(true, (wParam == VK_NEXT) || IsKeyPressed(VK_CONTROL));
				break;
			case VK_HOME:
				FarVideo->JumpHome();
				break;
			case VK_END:
				FarVideo->JumpEnd();
				break;
			case VK_SPACE:
				FarVideo->PauseResume();
				break;
			case 'M':
				FarVideo->SetVolume(volume_mute);
				break;
			case VK_UP:
				FarVideo->SetVolume(volume_up);
				break;
			case VK_DOWN:
				FarVideo->SetVolume(volume_down);
				break;
			}
		}
		break;
	}
	case WM_SETFOCUS:
		//if (!gbFullScreenNow)
		if ((gnSkipSetFocus == 0)
			|| (gnSkipSetFocus != 1 && ((GetTickCount() - gnSkipSetFocus) >= 1000)))
		{
			SetForegroundWindow(RootWnd);
			gnSkipSetFocus = 1;
		}
		//SetFocus(RootWnd);
		break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps = {0};
		//SetWindowPos(ghPopup, FarWindow, 0,0,0,0, SWP_NOSIZE|SWP_NOMOVE);
		HDC hdc = BeginPaint(hWnd, &ps);
		RECT rcWindow; GetClientRect(hWnd, &rcWindow);
		HBRUSH hbr = CreateSolidBrush(gnBackColor);
		FillRect(hdc, &rcWindow, hbr);
		if (DibBuffer) {
			GetRangedRect(&RangedRect, 0, &gbVideoFullWindow);

			StretchDIBits( hdc, 
				RangedRect.left, RangedRect.top, 
				RangedRect.right, RangedRect.bottom, // на самом деле тут width & height
				0, 0, RangedRect.right, RangedRect.bottom, DibBuffer,
				(BITMAPINFO *)&BmpHeader, DIB_RGB_COLORS, SRCCOPY );
		}
		DeleteObject(hbr);
		EndPaint(hWnd, &ps);
		break;
	}
	case WM_ERASEBKGND:
		result = 1;
		break;
	case WM_RBUTTONUP:
		TODO("Закрыть вьювер");
	case WM_RBUTTONDOWN:
		break;
	case WM_LBUTTONDOWN:
	{
		gnSkipSetFocus = GetTickCount();
		break;
	}
	case WM_LBUTTONDBLCLK:
	{
		if (!gbPictureExtMatch)
		{
			gbFullScreenNow = !gbFullScreenNow;
			gnSkipSetFocus = gbFullScreenNow ? 1 : 0;
			//if (ConEmuWnd == NULL)
			if (!gbChildWindowMode)
			{
				DWORD_PTR dwExStyle = GetWindowLongPtr(ghPopup, GWL_EXSTYLE);
				if (gbFullScreenNow)
					dwExStyle &= ~WS_EX_NOACTIVATE;
				else
					dwExStyle |= WS_EX_NOACTIVATE;
				SetWindowLongPtr(ghPopup, GWL_EXSTYLE, dwExStyle);
				if (gbFullScreenNow) {
					// Тут мы должны по идее найти окно с видео
					HWND hVideo = FindWindowEx(ghPopup, NULL, NULL, NULL);
					if (!hVideo) hVideo = ghPopup;
					SetForegroundWindow(ghPopup);
					SetFocus(hVideo);
				}
			}
			SwitchFullScreen(gbFullScreenNow);
			if (!gbFullScreenNow)
				SetForegroundWindow(RootWnd);
		}
		break;
	}
	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;
	case WM_DESTROY:
		for (MSG lpMsg; PeekMessage(&lpMsg, NULL, 0, 0, PM_REMOVE););
		PostQuitMessage(0);
		ghPopup = NULL;
		break;
	default:
		result = DefWindowProc(hWnd, messg, wParam, lParam);
	}

	return result;
}

DWORD WINAPI MViewWindowThread(void *pParameter)
{
	HANDLE hEvent = (HANDLE)pParameter;
	if (!hEvent)
		return 100;

    RECT rect = FillWindowRectGet();

	LogFore("MViewWindowThread started");

    _ASSERTE(ghPopup==NULL);

	HWND hParent = RootWnd; //FarWindow;
	gbChildWindowMode = false;
	if (ConEmuWnd != NULL) {
		LRESULT lRc = 0;
		DWORD_PTR dwRc = 0;
		//WARNING: Это потенциальная проблема. Если нить родительского окна (conemu)
		// находится в ожидании - мы повиснем на CreateWindowEx
		lRc = SendMessageTimeout ( hParent, WM_NULL, 0,0, SMTO_ABORTIFHUNG|SMTO_BLOCK, 300, &dwRc);
		//If the function fails or times out, the return value is zero
		if (lRc != 0) {
			gbChildWindowMode = true; // ConEmu не заблокирован, создавать окошко как дочернее можно
		} else {
			hParent = NULL;
		}
	}

	// WS_EX_TOPMOST обязателен. На некоторых машинах окно оказывается ПОД ghPopup
	DWORD nExStyle = WS_EX_TOOLWINDOW;
	DWORD nStyle = WS_CLIPSIBLINGS | WS_CLIPCHILDREN /*| WS_VISIBLE*/;
	//if (ConEmuWnd == NULL) {
	if (!gbChildWindowMode) {
		nExStyle |= 
			#ifndef NO_TOP_MOST
			WS_EX_TOPMOST | 
			#endif
			WS_EX_NOACTIVATE;
		nStyle |= WS_POPUP;
	} else {
		nStyle |= WS_CHILD;
	}
	//WARNING: Это потенциальная проблема. Если нить родительского окна (conemu)
	// находится в ожидании - мы повиснем :( Для этого выше делается SendMessageTimeout
    ghPopup = CreateWindowEx(nExStyle, g_WndClassName, _T("PictureView"),
    	nStyle, 
    	rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top,
    	hParent, NULL, g_hInstance, NULL);
    if (!ghPopup)
    	return 101;
	LogFore("CreateWindowEx");
	//SetWindowPos(ghPopup, HWND_TOP, 0,0,0,0, SWP_NOSIZE|SWP_NOMOVE);
	TODO("Пока только в ConEmu, потому что любой чих в консоли затирает наше окошко");
	//if (ConEmuWnd) {
	if (gbChildWindowMode) {
		SetForegroundWindow(RootWnd);
		SetParent(ghPopup, RootWnd); //ConEmuWnd);
		LogFore("SetParent");
		// Нужно подвинуть окошко
		RECT rect = FillWindowRectGet();
		MoveWindow(ghPopup, rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top, 1);
		SetForegroundWindow(RootWnd);
		LogFore("SetForegroundWindow.1");
	}
	ShowWindow(ghPopup, SW_SHOWNA);
	UpdateWindow(ghPopup);
	LogFore("UpdateWindow");
	SetForegroundWindow(RootWnd);
	LogFore("SetForegroundWindow.2");
    
	MSG Msg;
	SetEvent(hEvent);
	while (GetMessage(&Msg, ghPopup,0,0)) {
		if (Msg.message == WM_QUIT)
			break;
		#ifdef _DEBUG
		char szDbg[128], szName[64];
		switch (Msg.message) {
			case WM_TIMER: strcpy(szName, "WM_TIMER"); break;
			case WM_SETFOCUS: strcpy(szName, "WM_SETFOCUS"); break;
			case WM_KILLFOCUS: strcpy(szName, "WM_KILLFOCUS"); break;
			case WM_CREATE: strcpy(szName, "WM_CREATE"); break;
			default: wsprintfA(szName, "0x%X", (DWORD)Msg.message);
		}
		wsprintfA(szDbg, "MSG(%s,0x%X,0x%X)", szName, Msg.wParam, Msg.lParam);
		LogFore(szDbg);
		#endif
		DispatchMessage(&Msg);
	}

	_ASSERTE(ghPopup==NULL);
    return 0;
}
