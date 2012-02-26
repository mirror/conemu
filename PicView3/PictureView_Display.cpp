
/**************************************************************************
Copyright (c) 2009 Skakov Pavel
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

#define _WIN32_WINNT 0x0500
#include "PictureView.h"
#include <wchar.h>
//#include "headers/farcolor.hpp"
#include "PVDManager.h"
#include "Image.h"
#include "ImageInfo.h"
#include "DecodeItem.h"
#include "DecoderHandle.h"
#include "DisplayHandle.h"
#include <wininet.h>
#include <shlobj.h>
#include <ShObjIdl.h>
//
#include "ConEmuSupport.h"
#include "PictureView_Lang.h"
#include "PictureView_Display.h"

LPCSTR szPictureViewDisplay = "PictureViewDisplay";

#define RESET_VIEW_POS 0x7FFFFFFF

CDisplay g_Display;

CDisplay::CDisplay()
{
}
CDisplay::~CDisplay()
{
}

void CDisplay::OnTerminate()
{
	if (!(g_Plugin.FlagsWork & FW_TERMINATE))
	{
		_ASSERTE(g_Plugin.FlagsWork & FW_TERMINATE);
		g_Plugin.FlagsWork |= FW_TERMINATE;
	}

	if (g_Plugin.hDisplayThread)
	{
		if (WaitForSingleObject(g_Plugin.hDisplayThread, 15000) != WAIT_OBJECT_0)
		{
			_ASSERTE(g_Plugin.hDisplayThread == NULL);
			TerminateThread(g_Plugin.hDisplayThread, 100);
		}
		CloseHandle(g_Plugin.hDisplayThread); g_Plugin.hDisplayThread = NULL;
	}
}

bool CDisplay::CreateDisplayThread()
{
	DWORD nTID = GetCurrentThreadId();

	if (g_Plugin.hDisplayThread)
	{
		_ASSERTE(g_Plugin.hDisplayThread == NULL);
		if (WaitForSingleObject(g_Plugin.hDisplayThread,0) != WAIT_TIMEOUT)
		{
			// Нить завешилась?
			_ASSERTE(nTID == gnMainThreadId);
			CloseHandle(g_Plugin.hDisplayThread);
			g_Plugin.hDisplayThread = NULL;
		}
		else
		{
			return true;
		}
	}

	bool result = false;

	_ASSERTE(g_Plugin.hWnd == NULL);
	g_Plugin.hWnd = NULL;

	g_Plugin.FlagsDisplay = 0;
	ResetEvent(g_Plugin.hDisplayReady);
	ResetEvent(g_Plugin.hDisplayEvent);
	ResetEvent(g_Plugin.hWorkEvent);

	g_Plugin.hDisplayThread = CreateThread(NULL, 0, DisplayThreadProc, NULL, 0, &gnDisplayThreadId);
	if (result = (g_Plugin.hDisplayThread != NULL))
	{
		if (g_Plugin.hSynchroDone)
		{
			CloseHandle(g_Plugin.hSynchroDone); g_Plugin.hSynchroDone = NULL;
		}

		SetThreadPriority(g_Plugin.hDisplayThread, THREAD_PRIORITY_ABOVE_NORMAL);
		//Wait ForSingleObject(g_Plugin.hDisplayEvent, INFINITE);

		// Дождаться окончания создания окна отрисовки или завершения нити
		HANDLE hEvents[2] = {g_Plugin.hDisplayReady, g_Plugin.hDisplayThread};
		CFunctionLogger flog(L"Wait for display thread ready");
		//WaitDisplayEvent();
		DWORD nWait = WaitForMultipleObjects(2, hEvents, FALSE, INFINITE);

		if (nWait == (WAIT_OBJECT_0+1))
		{
			result = false;
			_ASSERTE(g_Plugin.hWnd == NULL);
			g_Plugin.hWnd = NULL;
			CloseHandle(g_Plugin.hDisplayThread);
			g_Plugin.hDisplayThread = NULL;
		}
		else
		{
			result = (g_Plugin.hWnd != NULL);
		}
	}

	if (result && !g_Plugin.hSynchroDone)
	{
		g_Plugin.hSynchroDone = CreateEvent(NULL, FALSE, FALSE, NULL);
	}

	return result;
}


#define KEY_PAN_DELAY 10

const u32 ZoomLevel[/* 27 */] = {
	0x290/*1%*/, 0x400, 0x600, 0x800, 0xC00, 0x1000, 0x1800, 0x2000, 0x3000, 0x4000, 0x6000, 
	0x8000/*50%*/, 0xC000, 0x10000/*100%*/, 0x18000, 0x20000, 0x30000, 0x40000, 0x60000, 0x80000, 
	0xC0000, 0x100000, 0x140000, 0x1E0000, 0x320000, 0x460000, 0x640000/*10000%*/};
const wchar_t g_WndClassName[] = L"FarPictureViewControlClass";

void TrayRestore();
void HideConsoleCursor();
void RestoreConsoleCursor();
void TrayDisable();
LRESULT OnPaint(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);
RECT GetDisplayRect();

bool gbDblClkFullScreen = false;

bool gbGotoPageNo = false;
UINT gnGotoPageNo = 0;

WORD vkSubtract = VK_SUBTRACT;
WORD vkAdd = VK_ADD;


static bool gbLockInputProcessing = false;

#define EX_STYLE_FULLSCREEN    (WS_EX_PALETTEWINDOW | WS_EX_TOPMOST)
//#define EX_STYLE_NOFULLSCREEN  (WS_EX_TOOLWINDOW | WS_EX_TOPMOST) // WS_EX_PALETTEWINDOW
#define EX_STYLE_NOFULLSCREEN  (WS_EX_PALETTEWINDOW | WS_EX_NOACTIVATE)
#define STYLE_FULLSCREEN       (WS_POPUP | WS_VISIBLE)
//#define STYLE_NOFULLSCREEN     (WS_POPUP | WS_VISIBLE)
#define STYLE_NOFULLSCREEN     (WS_CHILDWINDOW | WS_CLIPSIBLINGS /*| WS_VISIBLE*/) // WS_CHILD не проходит. начинает глючить вывод через GDI+

#define TIMER_HIDECURSOR 0
#define TIMER_ANIMATION  1
#define TIMER_REDRAWCON  2
#define TIMER_GOTOPAGE   3

#define TIMER_REDRAWCON_TIMEOUT 3000
#define TIMER_GOTOPAGE_TIMEOUT  2000

DWORD gnRedrawConTimerStart = 0;

LPCSTR szImagePtr = "CImagePtr";

class CImagePtr
{
protected:
	CImage* pImage;
public:
	CImagePtr()
	{
		pImage = g_Panel.GetImage();
		if (!pImage)
		{
			_ASSERTE(pImage != NULL);
			MessageBox(NULL, _T("g_Panel.GetImage() failed!"), _T("PicView2"), MB_ICONSTOP|MB_SETFOREGROUND|MB_SYSTEMMODAL);
		}
		else
		{
			pImage->AddRef(szImagePtr);
		}
	};
	void Assign(CImage* apImage)
	{
		if (pImage != apImage)
		{
			if (pImage)
				pImage->Release(szImagePtr);
			pImage = apImage;
			pImage->AddRef(szImagePtr);
		}
	};
	~CImagePtr()
	{
		if (pImage)
			pImage->Release(szImagePtr);
	};
public:
	CImage* operator->() const
	{
		_ASSERTE(pImage!=NULL);
		return pImage;
	};
	operator CImage*() const
	{
		_ASSERTE(pImage!=NULL);
		return pImage;
	};
	bool IsValid() const
	{
		return (pImage != NULL);
	}
};

void RequestTerminate(int nState /*= 0*/)
{
	g_Plugin.FlagsWork |= FW_TERMINATE;

	if (g_Plugin.hWnd)
	{
		KillTimer(g_Plugin.hWnd, TIMER_ANIMATION);
		KillTimer(g_Plugin.hWnd, TIMER_HIDECURSOR);
		KillTimer(g_Plugin.hWnd, TIMER_REDRAWCON);
		KillTimer(g_Plugin.hWnd, TIMER_GOTOPAGE);
	}

	if (g_Plugin.bMouseHided)
	{
		ShowCursor(TRUE);
		g_Plugin.bMouseHided = false;
	}

	// Это нужно выполнить ДО закрытия окна (зачистка неиспользуемых консолью участков клиентской части окна)
	if (GetCurrentThreadId() == gnDisplayThreadId)
	{
		CPVDManager::DisplayExit();
	}

	//// Остановить все нити, позакрывать все дескрипторы // -- тут нельзя - выполняется в своих нитях!!!
	//g_Manager.OnTerminate();
	//g_Panel.OnTerminate();

	if (nState < 1)
	{
		PostMessage(g_Plugin.hWnd, WM_DESTROY, 0, 0);
	} else if (nState > 1) {
		if (g_Plugin.hWnd && IsWindow(g_Plugin.hWnd))
			DestroyWindow(g_Plugin.hWnd);
	}
}

BOOL __stdcall DecodeCallback(void *pDecodeCallbackContext, UINT32 iStep, UINT32 nSteps)
{
	WARNING("GetAsyncKeyState нужно заменять на статус. иначе это потенциальный баг - она может вернуть не то, что ожидается");
	//return GetAsyncKeyState(VK_ESCAPE) >= 0;
	return !EscapePressed();
}

bool InitImageDisplay(bool bResetViewPos = true)
{
	CFunctionLogger flog(L"InitImageDisplay(%i)", (int)bResetViewPos);

	CImagePtr Image;
	if (!Image.IsValid())
	{
		RequestTerminate();
		return false;
	}

	if (Image->Info.Animation)
	{
		if (Image->Info.nPage /*&& g_Manager*/)
		{
			// При переходе на анимированную картинку - начинать анимацию строго с 0-го фрейма

			DecodeParams next;
			next.Priority = eCurrentImageCurrentPage; // наивысший приоритет
			next.nPage = 0; // 0-фрейм
			next.nRawIndex = Image->PanelItemRaw();
			
			// RequestDecodedImage - помещает запрос в очередь декодирования,
			// GetDecodedImage - дожидается результата декодирования
			CImage* pImage = g_Manager.GetDecodedImage(&next);
			if (!pImage)
			{
				WARNING("Что делать в таком случае?");
				RequestTerminate();
				return false;
			}

			// Обновить, если дескриптор сменился
			Image.Assign(pImage);

			//Image->Info.nPage = 0;
			//g_Manager.Decode(Image, &Image->mp_Draw, true);
		}
		Image->Info.Animation = 1;
		SetTimer(g_Plugin.hWnd, TIMER_ANIMATION, Image->Info.lFrameTime, NULL);
	}

	// По абсолютному зуму получить относительный g_Plugin.Zoom, который считается для декодированных размеров
	CheckRelativeZoom();

	if (bResetViewPos)
	{
		g_Plugin.ViewCenter.x = g_Plugin.ViewCenter.y = RESET_VIEW_POS/*0x7FFFFFFF*/;
		if (!g_Plugin.bKeepZoomAndPosBetweenFiles)
		{
			g_Plugin.ZoomAutoManual = false; //101129
			if ((g_Plugin.ZoomAuto==0) != (g_Plugin.bAutoZoom==false))
				g_Plugin.ZoomAuto = g_Plugin.bAutoZoom ? ZA_FIT : ZA_NONE;
			if (!g_Plugin.ZoomAuto)
				g_Plugin.Zoom = g_Plugin.AbsoluteZoom = 0x10000; // initial 100%
		}
	}

	gbGotoPageNo = false;
	gnGotoPageNo = 0;

	// Это делаем в кноце
	InvalidateRect(g_Plugin.hWnd, NULL, FALSE);
	g_Plugin.FlagsDisplay |= FD_TITLE_REPAINT;

	return true;
}


BOOL ExecuteInMainThread(DWORD nCmd, BOOL bRestoreCursor, BOOL bChangeParent)
{
	if (!g_Plugin.hSynchroDone)
		return FALSE;

	_ASSERTE(GetCurrentThreadId() == gnDisplayThreadId);
	
	static bool bAlreadyInCall = false;
	if (bAlreadyInCall) {
		CFunctionLogger::FunctionLogger(L"Trying to ExecuteInMainThread(%i) before previous call finished!",nCmd);
		return FALSE;
	}
	bAlreadyInCall = true;

	CFunctionLogger::FunctionLogger(L"ExecuteInMainThread(%i)", nCmd);
	
	if (bChangeParent) {
		CPVDManager::DisplayDetach();
		ShowWindow(g_Plugin.hWnd, SW_HIDE);
		CFunctionLogger::FunctionLogger(L"ExecuteInMainThread.SetParent(DesktopWnd)");
		SetParent(g_Plugin.hWnd, g_Plugin.hDesktopWnd);
		CFunctionLogger::FunctionLogger(L"SetParent done");
		TrayRestore();
	}

	if (bRestoreCursor)
		RestoreConsoleCursor();

	TODO("Это нужно будет закрыть в CriticalSection, чтобы запрос не пересекся с нитью декодера");
	
	_ASSERTE((g_Plugin.FlagsWork & nCmd) == 0);


	BOOL lbResult = ExecuteInMainThread(nCmd, 1 /*звать DefWindowProc*/);

	
	//ResetEvent(g_Plugin.hSynchroDone);
	//
	//g_Plugin.FlagsWork |= nCmd;
	//
	//bool lbBreak = false;
	//while (!lbBreak && WaitForSingleObject(g_Plugin.hSynchroDone, 10) != WAIT_OBJECT_0)
	//{
	//	if ((g_Plugin.FlagsWork & nCmd) == 0) {
	//		// Событие может быть выставлено уже после отрабатывания WaitForSingleObject как TIMEOUT
	//		//_ASSERTE((g_Plugin.FlagsWork & nCmd) != 0);
	//		break;
	//	}
	//	WARNING("EscapePressed не катит. Нажатие Esc в субдиалогах настройки и картинка опять показана");
	//	//if (EscapePressed()) // защита от зависания
	//	//	g_Plugin.FlagsWork |= FW_TERMINATE;

	//	if (g_Plugin.FlagsWork & FW_TERMINATE) {
	//		g_Plugin.FlagsWork &= ~nCmd;
	//		break;
	//	}

	//	MSG  lpMsg;
	//	while (PeekMessage(&lpMsg, NULL, 0, 0, PM_REMOVE))
	//	{
	//		// По идее, этого произойти не должно? Разве что идет закрытие фара?
	//		if (lpMsg.message == WM_QUIT || lpMsg.message == WM_DESTROY) {
	//			// вернуть обратно в очередь
	//			PostMessage(lpMsg.hwnd, lpMsg.message, lpMsg.wParam, lpMsg.lParam);
	//			// и выйти
	//			lbBreak = true; break;
	//		}

	//		__try {
	//			DefWindowProc(lpMsg.hwnd, lpMsg.message, lpMsg.wParam, lpMsg.lParam);
	//		}__except(EXCEPTION_EXECUTE_HANDLER){
	//			CFunctionLogger::FunctionLogger(L"!!! Exception in ExecuteInMainThread.DefWindowProc(%i)",lpMsg.message);
	//		}
	//		
	//		// Если команда завершилась - прекращаем обработку (точнее пропуск) сообщений
	//		if (WaitForSingleObject(g_Plugin.hSynchroDone, 0) == WAIT_OBJECT_0) {
	//			lbBreak = true; break;
	//		}
	//	}
	//}
	////_ASSERTE((g_Plugin.FlagsWork & nCmd) == 0);
	//g_Plugin.FlagsWork &= ~nCmd;


	
	if (g_Plugin.FlagsDisplay & FD_REQ_REFRESH) {
		g_Plugin.FlagsDisplay &= ~FD_REQ_REFRESH;
		PostMessage(g_Plugin.hWnd, DMSG_REFRESH, 0, 0);
	}
	
	if (bRestoreCursor)
		HideConsoleCursor();

	if (bChangeParent)
	{
		if (g_Plugin.hParentWnd != g_Plugin.hFarWnd
			&& g_Plugin.hParentWnd != g_Plugin.hConEmuWnd
			)
		{
			TrayDisable();
		}

		CFunctionLogger::FunctionLogger(L"ExecuteInMainThread.SetParent(ParentWnd)");
		SetParent(g_Plugin.hWnd, g_Plugin.hParentWnd);
		CFunctionLogger::FunctionLogger(L"SetParent done");
		ShowWindow(g_Plugin.hWnd, SW_SHOW);
		g_Plugin.FlagsDisplay |= FD_TITLE_REPAINT;
		InvalidateRect(g_Plugin.hWnd, NULL, FALSE);
	}
	
	CFunctionLogger::FunctionLogger(L"~ExecuteInMainThread(%i)", nCmd);

	bAlreadyInCall = false;
	return lbResult;
}

static bool CursorWasHidden = false;

void HideConsoleCursor(void)
{
	CFunctionLogger flog(L"HideConsoleCursor");
	const CONSOLE_CURSOR_INFO cci_hide = {1, FALSE};
	if (!CursorWasHidden) {
		GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE)/*g_Plugin.hOutput*/, &g_Plugin.cci);
		CursorWasHidden = true;
	}
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE)/*g_Plugin.hOutput*/, &cci_hide);
}

void AutoHideConsoleCursor(void)
{
	CFunctionLogger flog(L"AutoHideConsoleCursor");
	CONSOLE_CURSOR_INFO cci_cur = {1, FALSE};
	GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE)/*g_Plugin.hOutput*/, &cci_cur);
	if (cci_cur.bVisible || cci_cur.dwSize > 1) {
		HideConsoleCursor();
		if (g_Plugin.hWnd && IsWindow(g_Plugin.hWnd))
			InvalidateRect(g_Plugin.hWnd, NULL, FALSE);
	}
}

void RestoreConsoleCursor(void)
{
	CFunctionLogger flog(L"RestoreConsoleCursor");
	CursorWasHidden = false;
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE)/*g_Plugin.hOutput*/, &g_Plugin.cci);
}

WARNING("-> ITaskbarList2::MarkFullscreenWindow");

void TrayDisable(void)
{
	// В XP и выше - ничего делать не нужно
	if (g_Plugin.pTaskBar) {
		//g_Plugin.pTaskBar->MarkFullscreenWindow(g_Plugin.hWnd, TRUE);
		return;
	}

	if (!g_Plugin.bTrayDisable)
		return;

	CFunctionLogger flog(L"TrayDisable");

	const HWND hTrayWnd = FindWindowW(L"Shell_TrayWnd", NULL);
	const LONG TrayStyle = GetWindowLong(hTrayWnd, GWL_STYLE);
	if (TrayStyle & WS_VISIBLE)
	{
		ShowWindow(hTrayWnd, SW_HIDE);
		g_Plugin.bTrayOnTopDisabled = true;
	}
}

void TrayRestore()
{
	CFunctionLogger flog(L"TrayRestore");

	if (g_Plugin.pTaskBar) {
		//g_Plugin.pTaskBar->MarkFullscreenWindow(g_Plugin.hWnd, FALSE);
		g_Plugin.pTaskBar->DeleteTab(g_Plugin.hWnd);
		return;
	}

	// В XP и выше - больше ничего делать не нужно
	if (!g_Plugin.bTrayOnTopDisabled)
		return;
	const HWND hTrayWnd = FindWindowW(L"Shell_TrayWnd", NULL);
	ShowWindow(hTrayWnd, SW_SHOWNA);
	g_Plugin.bTrayOnTopDisabled = false;
}

// g_Plugin.Zoom - считается относительно декодированных размеров, прикинем абсолютный зум
u32  CheckAbsoluteZoom()
{
	//g_Plugin.AbsoluteZoom = g_Plugin.Zoom; -- сразу не ставим, чтобы не сбить ненароком

	CImagePtr Image;

	// Если есть декодированные размеры текущего (на экране) изображения
	if (Image.IsValid()
		&& Image->Info.lDecodedWidth && Image->Info.lDecodedHeight
		&& Image->Info.lWidth && Image->Info.lHeight)
	{
		u32 NewZoom = 0;
		
		// Прикидываем абсолютный зум по большей стороне
		if (Image->Info.lDecodedWidth > Image->Info.lDecodedHeight)
			NewZoom = MulDivU32R(g_Plugin.Zoom, Image->Info.lDecodedWidth, Image->Info.lWidth);
		else
			NewZoom = MulDivU32R(g_Plugin.Zoom, Image->Info.lDecodedHeight, Image->Info.lHeight);
			
		g_Plugin.AbsoluteZoom = Max<u32>(
				*ZoomLevel /*min 1%*/, 
				Min<u32>(
					ZoomLevel[sizeofarray(ZoomLevel) - 1] /*max 10000%*/, 
					NewZoom ));
	}
	
	if (!g_Plugin.AbsoluteZoom && g_Plugin.Zoom)
		g_Plugin.AbsoluteZoom = g_Plugin.Zoom;

	return g_Plugin.AbsoluteZoom;
}

// По абсолютному зуму (g_Plugin.AbsoluteZoom) прикинем 
// относительный g_Plugin.Zoom, который считается для декодированных размеров
u32 CheckRelativeZoom()
{
	//g_Plugin.AbsoluteZoom = g_Plugin.Zoom; -- сразу не ставим, чтобы не сбить ненароком

	CImagePtr Image;


	// Если есть декодированные размеры текущего (на экране) изображения
	if (Image.IsValid()
		&& g_Plugin.AbsoluteZoom
		&& Image->Info.lDecodedWidth && Image->Info.lDecodedHeight
		&& Image->Info.lWidth && Image->Info.lHeight)
	{
		u32 NewZoom = 0;

		// Прикидываем абсолютный зум по большей стороне
		if (Image->Info.lDecodedWidth > Image->Info.lDecodedHeight)
			NewZoom = MulDivU32R(g_Plugin.AbsoluteZoom, Image->Info.lWidth, Image->Info.lDecodedWidth);
		else
			NewZoom = MulDivU32R(g_Plugin.AbsoluteZoom, Image->Info.lHeight, Image->Info.lDecodedHeight);

		g_Plugin.Zoom = Max<u32>(
			*ZoomLevel /*min 1%*/, 
			Min<u32>(
			ZoomLevel[sizeofarray(ZoomLevel) - 1] /*max 10000%*/, 
			NewZoom ));
	}

	return g_Plugin.Zoom;
}

LRESULT OnZoom100(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
	g_Plugin.ZoomAutoManual = true; //101129
	// Масштаб 100%
	if (lParam & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED)) {
		g_Plugin.AbsoluteZoom = 0x10000;
		g_Plugin.ZoomAuto = ZA_NONE; // 100%
		// По абсолютному зуму получить относительный g_Plugin.Zoom, который считается для декодированных размеров
		CheckRelativeZoom();
	} else if (lParam & SHIFT_PRESSED) {
		// Автомасштабирование Fit   Shift-<Gray *>
		g_Plugin.ZoomAuto = ZA_FIT;
	} else if (lParam & (LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED)) {
		// Автомасштабирование Fill  Alt-<Gray *>
		g_Plugin.ZoomAuto = ZA_FILL;
	} else {
		g_Plugin.ViewCenter.x = 0;
		g_Plugin.ViewCenter.y = 0;
		if (g_Plugin.ZoomAuto == ZA_FILL) {
			g_Plugin.AbsoluteZoom = 0x10000;
			g_Plugin.ZoomAuto = ZA_NONE; // 100%
			CheckRelativeZoom();
		} else if (g_Plugin.ZoomAuto == ZA_FIT) {
			g_Plugin.ZoomAuto = ZA_FILL;
		} else {
			g_Plugin.ZoomAuto = ZA_FIT;
		}
	}
	// g_Plugin.Zoom - считается относительно декодированных размеров, прикинем абсолютный зум
	CheckAbsoluteZoom();
	
	//{ -- было
	//	g_Plugin.Zoom = 0x10000;
	//	g_Plugin.ZoomAuto = 0;
	//}
	g_Plugin.FlagsDisplay |= FD_TITLE_REPAINT;
	InvalidateRect(hWnd, NULL, FALSE);
	return TRUE;
}

LRESULT OnConfig(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
	DWORD nCmd = 0;
	if (!(lParam & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED)) && (lParam & (LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED)) && (lParam & SHIFT_PRESSED))
		nCmd = FW_SHOW_CONFIG;
	else
		nCmd = FW_SHOW_CONFIG | FW_SHOW_MODULES;

	ExecuteInMainThread(nCmd, TRUE/*bRestoreCursor*/, TRUE);
	return TRUE;

	//CPVDManager::DisplayDetach();
	//ShowWindow(hWnd, SW_HIDE);
	//SetParent(hWnd, g_Plugin.hDesktopWnd);
	//TrayRestore();
	//RestoreConsoleCursor();
	//if (!(lParam & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED)) && (lParam & (LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED)) && (lParam & SHIFT_PRESSED))
	//	g_Plugin.FlagsDisplay |= FD_CONFIG;
	//else
	//	g_Plugin.FlagsDisplay |= FD_CONFIG|FD_CONFIGDECODER;

	//WARNING("А это что за вечный цикл? Если hDisplayEvent окажется взведен - мы отсюда можем не выбраться");
	//while (WaitForSingleObject(g_Plugin.hDisplayEvent, 0) == WAIT_OBJECT_0) {
	//	SetEvent(g_Plugin.hDisplayEvent); Sleep(1);
	//}
	//
	//SetEvent(g_Plugin.hDisplayEvent);
	//
	//WaitForSingleObject(g_Plugin.hWorkEvent, INFINITE);
	//g_Plugin.FlagsDisplay = g_Plugin.FlagsDisplay & ~(FD_CONFIG|FD_CONFIGDECODER) | FD_TITLE_REPAINT;
	//HideConsoleCursor();
	//if (g_Plugin.hParentWnd != g_Plugin.hFarWnd)
	//	TrayDisable();
	//SetParent(hWnd, g_Plugin.hParentWnd);
	//ShowWindow(hWnd, SW_SHOW);
	//InvalidateRect(hWnd, NULL, FALSE);
	///WndProc(hWnd, WM_PAINT, 0, 0);
	//SetEvent(g_Plugin.hDisplayEvent);
	//return TRUE;
}

LRESULT OnRefresh(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
	CImagePtr Image;
	if (!Image.IsValid())
	{
		RequestTerminate();
		return FALSE;
	}

	g_Plugin.FlagsDisplay |= FD_REFRESH;
	WARNING("А это что за вечный цикл? Если hDisplayEvent окажется взведен - мы отсюда можем не выбраться");
	//while (WaitForSingleObject(g_Plugin.hDisplayEvent, 0) == WAIT_OBJECT_0) {
	//	SetEvent(g_Plugin.hDisplayEvent); Sleep(1);
	//}
	WARNING("!!! от g_Plugin.hDisplayEvent нужно избавляться !!!");
	SetEvent(g_Plugin.hDisplayEvent);
	WARNING("!!! от INFINITE нужно избавляться !!!"); // Если плагин падает, far показывает диалог с TrapLog, а ConEmu виснет, т.к. в нем подвисло окно PicView и он не может отрисоваться, т.к. все заблокировано
	CFunctionLogger::FunctionLogger(L"OnRefresh.WaitForSingleObject(g_Plugin.hWorkEvent, INFINITE)");
	WaitForSingleObject(g_Plugin.hWorkEvent, INFINITE);
	CFunctionLogger::FunctionLogger(L"OnRefresh.WaitForSingleObject(g_Plugin.hWorkEvent, INFINITE) done");
	g_Plugin.FlagsDisplay &= ~FD_REFRESH;
	CDisplayHandlePtr rDraw;
	if (!Image->GetDrawHandle(rDraw))
	{
		_ASSERTE(rDraw.IsValid());
		//PostMessage(hWnd, WM_DESTROY, 0, 0);
		RequestTerminate();
		return TRUE;
	}
	if (!InitImageDisplay(false))
		return FALSE; // уже вызван RequestTerminate
	WndProc(hWnd, WM_PAINT, 0, 0); // ибо _try
	//OnPaint(hWnd, WM_PAINT, 0, 0);
	WARNING("!!! от g_Plugin.hDisplayEvent нужно избавляться !!!");
	SetEvent(g_Plugin.hDisplayEvent);
	return TRUE;
}

LRESULT OnSharpZoomIn(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
	for (uint i = 0; i < sizeofarray(ZoomLevel); i++)
		if (g_Plugin.Zoom < ZoomLevel[i])
		{
			g_Plugin.ViewCenter.x = MulDivIU32R(g_Plugin.ViewCenter.x, ZoomLevel[i], g_Plugin.Zoom);
			g_Plugin.ViewCenter.y = MulDivIU32R(g_Plugin.ViewCenter.y, ZoomLevel[i], g_Plugin.Zoom);
			g_Plugin.Zoom = ZoomLevel[i];
			g_Plugin.ZoomAuto = ZA_NONE;
			g_Plugin.FlagsDisplay |= FD_TITLE_REPAINT;
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		}
	// g_Plugin.Zoom - считается относительно декодированных размеров, прикинем абсолютный зум
	CheckAbsoluteZoom();
	return TRUE;
}

LRESULT OnZoomIn(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
	if (g_Plugin.bSmoothZooming ^ ((lParam & (LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED)) != 0))
		g_Plugin.bZoomming = true;
	else
		OnSharpZoomIn(hWnd, messg, wParam, lParam);
	return TRUE;
}

LRESULT OnSharpZoomOut(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
	for (uint i = sizeofarray(ZoomLevel); i--;)
		if (g_Plugin.Zoom > ZoomLevel[i])
		{
			g_Plugin.ViewCenter.x = MulDivIU32R(g_Plugin.ViewCenter.x, ZoomLevel[i], g_Plugin.Zoom);
			g_Plugin.ViewCenter.y = MulDivIU32R(g_Plugin.ViewCenter.y, ZoomLevel[i], g_Plugin.Zoom);
			g_Plugin.Zoom = ZoomLevel[i];
			g_Plugin.ZoomAuto = ZA_NONE;
			g_Plugin.FlagsDisplay |= FD_TITLE_REPAINT;
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		}
	// g_Plugin.Zoom - считается относительно декодированных размеров, прикинем абсолютный зум
	CheckAbsoluteZoom();
	return TRUE;
}

LRESULT OnZoomOut(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
	if (g_Plugin.bSmoothZooming ^ ((lParam & (LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED)) != 0))
		g_Plugin.bZoomming = true;
	else
		OnSharpZoomOut(hWnd, messg, wParam, lParam);
	return TRUE;
}

LRESULT OnAutoZoom(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
	// Переключение: автомасштабирование / масштаб 100%
	g_Plugin.ViewCenter.x = 0;
	g_Plugin.ViewCenter.y = 0;
	if (!g_Plugin.ZoomAuto)
		g_Plugin.ZoomAuto = ZA_FIT;
	else
	{
		WARNING("0x10000 сработает только на декодированный размер, а не на 'реальный'. Поправить!");
		g_Plugin.AbsoluteZoom = 0x10000;
		g_Plugin.ZoomAuto = ZA_NONE;
		// По абсолютному зуму получить относительный g_Plugin.Zoom, который считается для декодированных размеров
		CheckRelativeZoom();
	}
	g_Plugin.FlagsDisplay |= FD_TITLE_REPAINT;
	InvalidateRect(hWnd, NULL, FALSE);
	return TRUE;
}

LRESULT OnZoomByWheel(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
	if (!g_Plugin.MouseZoomMode)
	{
		if (wParam & 0x80000000) {
			OnSharpZoomOut(hWnd, messg, wParam, lParam);
			return TRUE;
		} else {
			OnSharpZoomIn(hWnd, messg, wParam, lParam);
			return TRUE;
		}
	}
	else
	{
		POINT MPos;
		GetCursorPos(&MPos);
		POINT MPosLocal = MPos;
		MapWindowPoints(NULL, hWnd, &MPosLocal, 1);
		RECT ImgRect;
		GetClientRect(hWnd, &ImgRect);
		const int dx = (ImgRect.right - ImgRect.left)/2 - MPosLocal.x;
		const int dy = (ImgRect.bottom - ImgRect.top)/2 - MPosLocal.y;

		const u32 OldZoom = g_Plugin.Zoom;
		if (wParam & 0x80000000)
		{
			for (uint i = sizeofarray(ZoomLevel); i--;)
				if (g_Plugin.Zoom > ZoomLevel[i])
				{
					g_Plugin.Zoom = ZoomLevel[i];
					#ifdef _DEBUG
					wchar_t szNewZoom[64]; wsprintf(szNewZoom, L"New zoom by wheel is: %i\n", g_Plugin.Zoom);
					OutputDebugString(szNewZoom);
					#endif
					break;
				}
		}
		else
		{
			for (uint i = 0; i < sizeofarray(ZoomLevel); i++)
				if (g_Plugin.Zoom < ZoomLevel[i])
				{
					g_Plugin.Zoom = ZoomLevel[i];
					#ifdef _DEBUG
					wchar_t szNewZoom[64]; wsprintf(szNewZoom, L"New zoom by wheel is: %i\n", g_Plugin.Zoom);
					OutputDebugString(szNewZoom);
					#endif
					break;
				}
		}
		
		// g_Plugin.Zoom - считается относительно декодированных размеров, прикинем абсолютный зум
		CheckAbsoluteZoom();
		
		if (OldZoom != g_Plugin.Zoom)
		{
			g_Plugin.ViewCenter.x = MulDivIU32R(g_Plugin.ViewCenter.x + dx, g_Plugin.Zoom, OldZoom);
			g_Plugin.ViewCenter.y = MulDivIU32R(g_Plugin.ViewCenter.y + dy, g_Plugin.Zoom, OldZoom);
			if (g_Plugin.MouseZoomMode == 1)
			{
				g_Plugin.DragBase.x += dx;
				g_Plugin.DragBase.y += dy;
				SetCursorPos(MPos.x + dx, MPos.y + dy);
			}
			else
			{
				g_Plugin.ViewCenter.x -= dx;
				g_Plugin.ViewCenter.y -= dy;
			}
			g_Plugin.ZoomAuto = ZA_NONE;
			g_Plugin.bCorrectMousePos = true;
			g_Plugin.FlagsDisplay |= FD_TITLE_REPAINT;
			InvalidateRect(hWnd, NULL, FALSE);
		}
	}
	return TRUE;
}


WARNING("Полностью инкапсулировать в g_Panel. пусть он занимается необходимым извлечением файла");
LRESULT RequestNextImage(bool abNext, bool abHomeEnd = false, bool abActivate = true)
{
	DecodeParams parms;
	parms.Flags = eRenderFirstAvailable | (abActivate ? eRenderActivateOnReady : 0);
	
	if (abHomeEnd)
	{
		parms.nRawIndex =
			 abNext
			 	? g_Panel.GetLastItemRawIndex()
			 	: g_Panel.GetFirstItemRawIndex();
		// В какую сторону мотать, если не удастся обработать желаемый элемент
		parms.iDirection = abNext ? -1 : 1;
	} else {
		parms.Flags |= eRenderRelativeIndex;
		parms.nRawIndex = -1;
		parms.iDirection = abNext ? 1 : -1;
	}

	return g_Manager.RequestDecodedImage(&parms);
}

LRESULT OnNextFile(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
	//if (g_Plugin.FlagsWork & FW_JUMP_DISABLED)
	//	return TRUE;
	//if (wParam & 0x80000000)
	//	g_Plugin.FlagsDisplay |= FD_JUMP_NEXT;
	//else
	//	g_Plugin.FlagsDisplay &= ~FD_JUMP_NEXT;
	//g_Plugin.FlagsDisplay |= FD_JUMP;
	
	//OnNextFileJump(hWnd, messg, wParam, lParam);
	RequestNextImage((wParam & 0x80000000)/*abNext*/, false/*abHomeEnd*/, true/*abActivate*/);
	
	return TRUE;
}
LRESULT OnGotoPage(UINT nNewPage, BOOL abForward)
{
	KillTimer(g_Plugin.hWnd, TIMER_GOTOPAGE);

	CImagePtr Image;
	if (!Image.IsValid())
	{
		RequestTerminate();
		return FALSE;
	}

	if (/*g_Manager &&*/ Image->Info.nPages > 1 && Image->Info.nPages > nNewPage)
	{
		DecodeParams next;
		next.Priority = eCurrentImageCurrentPage; // наивысший приоритет
		next.nPage = nNewPage;
		next.nRawIndex = Image->PanelItemRaw();
		
		// RequestDecodedImage - помещает запрос в очередь декодирования,
		// GetDecodedImage - дожидается результата декодирования
		g_Manager.RequestDecodedImage(&next);

		// Остальное должно "само"

		//Image->Info.nPage = nNewPage;
		//g_Manager.Decode(Image, &Image->mp_Draw, true);
		//// При листании на следующую страницу (для неанимированных) - сбросить прокрутку
		WARNING("Проверить, чтобы прокрутка при листании сбрасывалась?");
		//if (abForward && !Image->Info.Animation)
		//	g_Plugin.ViewCenter.x = g_Plugin.ViewCenter.y = RESET_VIEW_POS/*0x7FFFFFFF*/;
		//g_Plugin.FlagsDisplay |= FD_TITLE_REPAINT;
		//InvalidateRect(g_Plugin.hWnd, NULL, FALSE);

		return 1;
	}
	return 0;
}
LRESULT OnGotoPageAddNumber(UINT nAddNo)
{
	// Сначала - прибить
	KillTimer(g_Plugin.hWnd, TIMER_GOTOPAGE);

	CImagePtr Image;
	if (!Image.IsValid())
	{
		RequestTerminate();
		return 0;
	}

	if (Image->Info.Animation == 1)
		return 0; // пока изображение анимируется - запрещено
	if (nAddNo > 9)
		return 0; // некорректная цифра?

	if (!gbGotoPageNo) {
		gbGotoPageNo = true;
		gnGotoPageNo = 0;
	}
	
	gnGotoPageNo = (gnGotoPageNo*10)+nAddNo;
	if (gnGotoPageNo >= Image->Info.nPages) {
		gbGotoPageNo = false; gnGotoPageNo = 0;
		MessageBeep(MB_ICONEXCLAMATION); // Бибикнуть что-ли?
		return 0;
	}

	// Лучше не надо. Юзер по инерции может нажать Enter, что приведет закрытию изображения
	//// Если ввод следующей цифры (любой) приведет к превышению диапазона - прыгаем сразу
	//if ((gnGotoPageNo*10) >= Image->Info.nPages) {
	//	gbGotoPageNo = false;
	//	OnGotoPage(gnGotoPageNo-1, TRUE); // TRUE, чтобы сбросился PAN
	//	gnGotoPageNo = 0;
	//	return 1;
	//}

	SetTimer(g_Plugin.hWnd, TIMER_GOTOPAGE, TIMER_GOTOPAGE_TIMEOUT, NULL);

	return 1;
}
LRESULT OnNextPage(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
	CImagePtr Image;
	if (!Image.IsValid())
	{
		RequestTerminate();
		return FALSE;
	}

	// Анимированный GIF. Отключить таймер
	if (Image->Info.Animation == 1)
	{
		Image->Info.Animation = 2;
		KillTimer(g_Plugin.hWnd, TIMER_ANIMATION);
		g_Plugin.FlagsDisplay |= FD_TITLE_REPAINT;
	}
	
	if (wParam & 0x80000000)
	{
		if (Image->Info.nPages > (Image->Info.nPage + 1))
			OnGotoPage(Image->Info.nPage + 1, TRUE);
		//if (g_Manager && Image->Info.nPages > 1 && Image->Info.nPages > (Image->Info.nPage + 1))
		//{
		//	Image->Info.nPage++;
		//	g_Manager.Decode(Image, &Image->mp_Draw, true, Image->InfoPage);
		//	// При листании на следующую страницу - сбросить прокрутку
		//	if (!Image->Info.Animation)
		//		g_Plugin.ViewCenter.x = g_Plugin.ViewCenter.y = RESET_VIEW_POS/*0x7FFFFFFF*/;
		//	g_Plugin.FlagsDisplay |= FD_TITLE_REPAINT;
		//	InvalidateRect(hWnd, NULL, FALSE);
		//}
	}
	else
	{
		if (Image->Info.nPage)
			OnGotoPage(Image->Info.nPage - 1, FALSE);
		//if (g_Manager && Image->Info.nPages > 1 && Image->Info.nPage)
		//{
		//	Image->Info.nPage--;
		//	g_Manager.Decode(Image, &Image->mp_Draw, true, Image->InfoPage);
		//	g_Plugin.FlagsDisplay |= FD_TITLE_REPAINT;
		//	InvalidateRect(hWnd, NULL, FALSE);
		//}
	}
	return TRUE;
}

LRESULT OnScroll(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
	if (g_Plugin.bSmoothScrolling ^ ((lParam & (LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED)) != 0))
		g_Plugin.bScrolling = true; // SmoothScrolling обрабатывается в самом DisplayThreadProc
	else
	{
		RECT ParentRect;
		// Тут был g_Plugin.hParentWnd. Логичнее - hWnd
		if (GetClientRect(g_Plugin.hWnd, &ParentRect))
		{
			bool bLeft, bRight, bUp, bDown;
			bLeft  = GetAsyncKeyState(VK_LEFT)  < 0 || GetAsyncKeyState(VK_NUMPAD4) < 0;
			bRight = GetAsyncKeyState(VK_RIGHT) < 0 || GetAsyncKeyState(VK_NUMPAD6) < 0;
			bUp    = GetAsyncKeyState(VK_UP)    < 0 || GetAsyncKeyState(VK_NUMPAD8) < 0;
			bDown  = GetAsyncKeyState(VK_DOWN)  < 0 || GetAsyncKeyState(VK_NUMPAD2) < 0;

			if (bRight ^ bLeft)
			{
				const int dx = (ParentRect.right - ParentRect.left) / 0x10;
				if (bRight)
					g_Plugin.ViewCenter.x -= dx;
				else
					g_Plugin.ViewCenter.x += dx;
			}
			if (bUp ^ bDown)
			{
				const int dy = (ParentRect.bottom - ParentRect.top) / 0x10;
				if (bDown)
					g_Plugin.ViewCenter.y -= dy;
				else
					g_Plugin.ViewCenter.y += dy;
			}
			if (bRight ^ bLeft || bUp ^ bDown)
				InvalidateRect(hWnd, NULL, FALSE);
		}
	}
	return TRUE;
}

WARNING("Тоже нафиг не нужно передавать управление в основную нить");
// Только для обновления заголовка - FW_UPDATETITLE
LRESULT OnChangeDecoder(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
	_ASSERTE(GetCurrentThreadId() == gnDisplayThreadId);

	CImagePtr Image;
	if (!Image.IsValid())
	{
		RequestTerminate();
		return FALSE;
	}

	if (!(lParam & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED)) && (lParam & (LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED)) && !(lParam & SHIFT_PRESSED))
	{
		DecodeParams parms;
		parms.nRawIndex = Image->PanelItemRaw();
		parms.nPage = Image->Info.nPage;
		parms.Flags = ((wParam == VK_NEXT) ? eRenderNextDecoder : eRenderPrevDecoder) | eRenderActivateOnReady;
		parms.Priority = eCurrentImageCurrentPage;

		g_Manager.RequestDecodedImage(&parms);

		//g_Plugin.FlagsDisplay |= FD_REFRESH;
		//if (wParam == VK_NEXT)
		//	g_Plugin.FlagsWork |= FW_NEXTDECODER;
		//else
		//	g_Plugin.FlagsWork |= FW_PREVDECODER;
		//WARNING("А это что за вечный цикл? Если hDisplayEvent окажется взведен - мы отсюда можем не выбраться");
		//while (WaitForSingleObject(g_Plugin.hDisplayEvent, 0) == WAIT_OBJECT_0) {
		//	SetEvent(g_Plugin.hDisplayEvent); Sleep(1);
		//}
		//SetEvent(g_Plugin.hDisplayEvent);
		//WARNING("!!! от INFINITE нужно избавляться !!!"); // Если плагин падает, far показывает диалог с TrapLog, а ConEmu виснет, т.к. в нем подвисло окно PicView и он не может отрисоваться, т.к. все заблокировано
		//CFunctionLogger::FunctionLogger(L"OnChangeDecoder.WaitForSingleObject(g_Plugin.hWorkEvent, INFINITE)");
		//WaitForSingleObject(g_Plugin.hWorkEvent, INFINITE);
		//CFunctionLogger::FunctionLogger(L"OnChangeDecoder.WaitForSingleObject(g_Plugin.hWorkEvent, INFINITE) done");
		//g_Plugin.FlagsDisplay &= ~(FD_REFRESH);
		//g_Plugin.FlagsWork &= ~(FW_PREVDECODER|FW_NEXTDECODER);
		//if (!Image->mp_Draw) {
		//	//PostMessage(hWnd, WM_DESTROY, 0, 0);
		//	RequestTerminate();
		//	return TRUE;
		//}
		//if (!InitImageDisplay(false))
		//	return FALSE; // уже вызван RequestTerminate
		//WndProc(hWnd, WM_PAINT, 0, 0); // ибо _try
		////OnPaint(hWnd, WM_PAINT, 0, 0);
		//SetEvent(g_Plugin.hDisplayEvent);
		return TRUE;
	}
	return FALSE;
}

LRESULT OnMark(BOOL abNewSelected)
{
	//if (!(g_Plugin.FlagsWork & FW_JUMP_DISABLED) && Image->PanelItemRaw() != PANEL_ITEM_UNAVAILABLE)
	if (g_Panel.IsMarkAllowed())
	{
		ExecuteInMainThread(abNewSelected ? FW_MARK_FILE : FW_UNMARK_FILE, FALSE/*bRestoreCursor*/, FALSE);
	}
	return TRUE;
}

LRESULT OnFullScreen(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
	if (g_Plugin.FlagsWork & FW_QUICK_VIEW) {
		WARNING("А в quickview глючить не будет?");
		g_Plugin.hParentWnd = g_Plugin.hConEmuWnd ? g_Plugin.hConEmuWnd : g_Plugin.hFarWnd;
		return FALSE;
	}

	if (g_Plugin.bMouseHided == g_Plugin.bFullScreen)
	{
		ShowCursor(g_Plugin.bMouseHided);
		g_Plugin.bMouseHided ^= true;
		g_Plugin.bIgnoreMouseMove = g_Plugin.bMouseHided;
	}
	if (g_Plugin.bFullScreen ^= true)
	{
		OutputDebugString(L"FullScreen -> ON\n");
		TrayDisable();
		g_Plugin.hParentWnd = g_Plugin.hDesktopWnd;
	}
	else
	{
		OutputDebugString(L"FullScreen -> OFF\n");
		TrayRestore();
		g_Plugin.hParentWnd = g_Plugin.hConEmuWnd ? g_Plugin.hConEmuWnd : g_Plugin.hFarWnd;
		g_Plugin.FlagsDisplay |= FD_TITLE_REPAINT;
	}

	if (g_Plugin.bFullScreen)
		SetWindowLongPtr(hWnd, GWL_STYLE, STYLE_FULLSCREEN);
	//if (g_Plugin.bFullScreen) {
	//	SetWindowLongPtr(hWnd, GWL_STYLE, STYLE_FULLSCREEN);
	//} else {
	//	SetWindowLongPtr(hWnd, GWL_EXSTYLE, EX_STYLE_NOFULLSCREEN);
	//}

	CFunctionLogger::FunctionLogger(L"OnFullScreen.SetParent(ParentWnd)");
	SetParent(hWnd, g_Plugin.hParentWnd);
	CFunctionLogger::FunctionLogger(L"SetParent done");

	if (g_Plugin.bFullScreen) {
		if (g_Plugin.pTaskBar) {
			// Уведомим, что окошко должно быть поверх TaskBar'а
			g_Plugin.pTaskBar->MarkFullscreenWindow(g_Plugin.hWnd, TRUE);
			// Удалим кнопку на всякий случай
			g_Plugin.pTaskBar->DeleteTab(g_Plugin.hWnd);
		}

		//SetWindowLongPtr(hWnd, GWL_EXSTYLE, EX_STYLE_FULLSCREEN);
		SetWindowPos(hWnd, HWND_TOP, 0,0,0,0, SWP_NOMOVE|SWP_NOSIZE);
	} else {
		SetWindowLongPtr(hWnd, GWL_STYLE, STYLE_NOFULLSCREEN);
	}

	InvalidateRect(hWnd, NULL, FALSE);
	//WndProc(hWnd, WM_PAINT, 0, 0); // ибо _try

	if (!g_Plugin.bFullScreen && messg != DMSG_KEYBOARD) {
		//SetCapture(hWnd);
		//FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE)/*g_Plugin.hInput*/);
		//Sleep(100); // с секундой вроде бы нормально, но это долго...
		// На Flush может виснуть!
		//FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE)/*g_Plugin.hInput*/);
		//InvalidateRect(hWnd, NULL, FALSE);
		//WndProc(hWnd, WM_PAINT, 0, 0); // ибо _try
		//PostMessage(hWnd, WM_PAINT, 0, 0);
	}

	if (g_Plugin.bFullScreen) {
		SetForegroundWindow(g_Plugin.hWnd);
		SetFocus(g_Plugin.hWnd);
	} else {
		SetForegroundWindow(g_Plugin.hConEmuWnd ? g_Plugin.hConEmuWnd : g_Plugin.hFarWnd);
	}
	return TRUE;
}

// Координаты (актуальноя для QView) и размер окна отрисовки.
// координаты даются относительно родительского окна (FAR/Desktop/ConEmu).
// !! Само окно может быть еще и не создано
RECT GetDisplayRect()
{
	RECT ParentRect = {0, 0, 1280, 960};
	if (g_Plugin.FlagsWork & FW_QUICK_VIEW)
	{
		ParentRect = g_Plugin.ViewPanelG;
	}
	else
	{
		// Как минимум - ParentRect, но если получится - берем координаты текущего монитора
		_ASSERTE(g_Plugin.hFarWnd && IsWindow(g_Plugin.hFarWnd));
		// консольное окно, или окно отрисовки в ConEmu
		GetClientRect(g_Plugin.hFarWnd, &ParentRect);
		if (ParentRect.right == ParentRect.left || ParentRect.bottom == ParentRect.top)
		{
			// глюки при сворачивании/разворачивании консоли
			Sleep(10);
			GetClientRect(g_Plugin.hFarWnd, &ParentRect);
			_ASSERTE(ParentRect.right>ParentRect.left && ParentRect.bottom>ParentRect.top);
		}
		if (g_Plugin.bFullScreen)
		{
			HMONITOR hMon = MonitorFromWindow(g_Plugin.hFarWnd, MONITOR_DEFAULTTONEAREST);
			if (hMon)
			{
				MONITORINFO mi = {sizeof(MONITORINFO)};
				if (GetMonitorInfo(hMon, &mi))
					ParentRect = mi.rcMonitor;
			}
		}
		else if (g_Plugin.hConEmuWnd)
		{
			MapWindowPoints(g_Plugin.hFarWnd, g_Plugin.hConEmuWnd, (LPPOINT)&ParentRect, 2);
		}
	}
	return ParentRect;
}

LRESULT OnPaint(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
	if (!IsWindowVisible(hWnd))
	{
		return 0;
	}

	CImagePtr Image;
	if (!Image.IsValid())
	{
		RequestTerminate();
		return 0;
	}

	if (!IsWindowVisible(hWnd))
	{
		_ASSERT(FALSE);
		return 0;
	}
	if ((g_Plugin.hConEmuWnd && IsIconic(g_Plugin.hConEmuWnd)) || IsIconic(g_Plugin.hFarWnd))
	{
		return 0; // Если окно свернуто - ничего не делать!
	}


	// Warning!!! После этого, return должен быть только в конце функции (pDraw->Release)

	RECT ParentRect = GetDisplayRect(); //{0, 0, 1280, 960};
	//if (g_Plugin.FlagsWork & FW_QUICK_VIEW)
	//	ParentRect = g_Plugin.ViewPanelG;
	//else {
	//	// Как минимум - ParentRect, но если получится - берем координаты текущего монитора
	//	GetClientRect(g_Plugin.hParentWnd, &ParentRect);
	//	if (g_Plugin.bFullScreen) {
	//		HMONITOR hMon = MonitorFromWindow(g_Plugin.hFarWnd, MONITOR_DEFAULTTONEAREST);
	//		if (hMon) {
	//			MONITORINFO mi = {sizeof(MONITORINFO)};
	//			if (GetMonitorInfo(hMon, &mi))
	//				ParentRect = mi.rcMonitor;
	//		}
	//	}
	//}
	
	//const вызывает инициализацию переменных ДО входа в функцию (по возможности). что нам не подходит - хочется видеть последовательность действий!
	int lScreenWidth = ParentRect.right - ParentRect.left;
	int lScreenHeight = ParentRect.bottom - ParentRect.top;

	RECT ImgRect;
	if (GetClientRect(hWnd, &ImgRect))
	{
		// Для QView это некорректно. ImgRect никогда не будет = ParentRect, поэтому MapWindowPoints
		RECT rcTest = ImgRect;
		MapWindowPoints(hWnd, GetParent(hWnd), (LPPOINT)&rcTest, 2);
		if (memcmp(&rcTest, &ParentRect, sizeof(RECT)))
			MoveWindow(hWnd, ParentRect.left, ParentRect.top, lScreenWidth, lScreenHeight, FALSE);
	}
	

	CDisplayHandlePtr rDraw;

	if (!Image->GetDrawHandle(rDraw))
	{
		// По идее, такого возникать не должно, но, в принципе, это допустимая ситуация. Посмотрим...
		//_ASSERTE(pDraw && Image->Info.lDecodedWidth && Image->Info.lDecodedHeight);
		OutputDebugString(L"!!! Recieved WM_PAINT before Image[0] decode finished !!!\n");
		PAINTSTRUCT ps = {NULL};
		// BeginPaint работает с учетом Invalidate, а нам нужно "все
		HDC hdc = GetDC(hWnd); // BeginPaint(hWnd, &ps);
		RECT rc; GetClientRect(hWnd, &rc);
		COLORREF crBack =
			#ifdef _DEBUG
				g_Plugin.BackColor() //RGB(255,0,0) // зальем все красным, чтобы глюк видеть под отладчиком
			#else
				g_Plugin.BackColor()
			#endif
			;
		HBRUSH hBr = CreateSolidBrush(crBack);
		FillRect(hdc, &rc, hBr);
		DeleteObject(hBr);
		//EndPaint(hWnd, &ps);
		ReleaseDC(hWnd, hdc);
		//Sleep(1);
		return 0;
	}

	
	
	u32 Zoom;
	if (g_Plugin.ZoomAuto || g_Plugin.FlagsWork & FW_QUICK_VIEW)
	{
		//const вызывает инициализацию переменных ДО входа в функцию (по возможности). что нам не подходит - хочется видеть последовательность действий!
		u32 ZoomW = MulDivU32(lScreenWidth, 0x10000, Image->Info.lWidth /*dds->m_lWorkWidth*/);
		u32 ZoomH = MulDivU32(lScreenHeight, 0x10000, Image->Info.lHeight /*dds->m_lWorkHeight*/);
		Zoom = (g_Plugin.ZoomAuto == ZA_FIT || g_Plugin.FlagsWork & FW_QUICK_VIEW) ? Min(ZoomW, ZoomH) : Max(ZoomW, ZoomH);
		if (!Zoom)
			Zoom = 1;

		//101129 - По идее, это ручное изменение масштаба, и нужно игнорировать авторазмеры (они только для загрузки картинки)
		if (!g_Plugin.ZoomAutoManual)
		{
			if (g_Plugin.bAutoZoomMin && Zoom < g_Plugin.AutoZoomMin)
			{
				Zoom = g_Plugin.AutoZoomMin; g_Plugin.ZoomAuto = ZA_NONE; //101129
			}
			if (g_Plugin.bAutoZoomMax && Zoom > g_Plugin.AutoZoomMax)
			{
				Zoom = g_Plugin.AutoZoomMax; g_Plugin.ZoomAuto = ZA_NONE; //101129
			}
		}

		g_Plugin.AbsoluteZoom = Zoom;

		if (g_Plugin.AbsoluteZoom != Zoom && !(g_Plugin.FlagsWork & FW_QUICK_VIEW))
		{
			g_Plugin.FlagsDisplay |= FD_TITLE_REPAINT;
		}
		// g_Plugin.Zoom - считается относительно декодированных размеров, прикинем абсолютный зум
		//CheckAbsoluteZoom();

		// По абсолютному зуму получить относительный g_Plugin.Zoom, который считается для декодированных размеров
		Zoom = CheckRelativeZoom();
		
	}
	else
	{
		Zoom = g_Plugin.Zoom;
		
		// Прикинуть Zoom по AbsoluteZoom
		if (g_Plugin.AbsoluteZoom
			&& Image->Info.lDecodedWidth && Image->Info.lDecodedHeight
			&& Image->Info.lWidth && Image->Info.lHeight
			&& (Image->Info.lDecodedWidth != Image->Info.lWidth
			    || Image->Info.lDecodedHeight != Image->Info.lHeight)
			)
		{
			u32 NewZoom = 0;
			
			// Прикидываем абсолютный зум по большей стороне
			if (Image->Info.lDecodedWidth > Image->Info.lDecodedHeight)
				NewZoom = MulDivU32R(g_Plugin.AbsoluteZoom, Image->Info.lWidth, Image->Info.lDecodedWidth);
			else
				NewZoom = MulDivU32R(g_Plugin.AbsoluteZoom, Image->Info.lHeight, Image->Info.lDecodedHeight);
				
			Zoom = Max<u32>(
					*ZoomLevel /*min 1%*/, 
					Min<u32>(
						ZoomLevel[sizeofarray(ZoomLevel) - 1] /*max 10000%*/, 
						NewZoom ));

			g_Plugin.Zoom = Zoom;
		}
		// g_Plugin.Zoom - считается относительно декодированных размеров, прикинем абсолютный зум
		CheckAbsoluteZoom();
	}
	/*
					u32 NewZoom = Max<u32>(
							*ZoomLevel / *min 1%* /, 
							Min<u32>(
								ZoomLevel[sizeofarray(ZoomLevel) - 1] / *max 10000%* /, 
								bZoomIn ? MulDivU32R(g_Plugin.Zoom, 1000 + g_Plugin.SmoothZoomingStep, 1000) 
										: MulDivU32R(g_Plugin.Zoom, 1000, 1000 + g_Plugin.SmoothZoomingStep)));
	*/	
	

	if (g_Plugin.FlagsDisplay & FD_TITLE_REPAINT)
	{
		g_Plugin.FlagsDisplay &= ~FD_TITLE_REPAINT;
		if (!(g_Plugin.FlagsWork & FW_QUICK_VIEW))
		{
			//TitleRepaint();
			ExecuteInMainThread(FW_TITLEREPAINT,FALSE,FALSE);
		}
		else
		{   // В режиме QuickView инф.строка печатается в самой панели QView
			ExecuteInMainThread(FW_QVIEWREPAINT,FALSE,FALSE);
		}
	}
	
	//когда-то тут был const, вызывающий инициализацию переменных ДО входа в функцию (по возможности). что нам не подходит - хочется видеть последовательность действий!

	// По декодированному размеру и текущему зуму (относительному) получить размеры ViewSpace.
	// Этот размер может быть намного больше (или меньше) размера экрана.
	int lWidth = MulDivU32R(Image->Info.lDecodedWidth, Zoom, 0x10000);
	int lHeight = MulDivU32R(Image->Info.lDecodedHeight, Zoom, 0x10000);
	ImgRect.left = (lScreenWidth - lWidth) / 2;
	ImgRect.right = 0; // будет установлен ниже, по скорректированному .left + lWidth
	ImgRect.top = (lScreenHeight - lHeight) / 2;
	ImgRect.bottom = 0; // будет установлен ниже, по скорректированному .left + lWidth

	if (ParentRect.left)
	{
		ParentRect.right -= ParentRect.left;
		ParentRect.left = 0;
	}
	if (ParentRect.top)
	{
		ParentRect.bottom -= ParentRect.top;
		ParentRect.top = 0;
	}

	if (!g_Plugin.bFreePosition || g_Plugin.ViewCenter.x == RESET_VIEW_POS/*0x7FFFFFFF*/)
	{
		POINT ViewCenterOld = g_Plugin.ViewCenter;
		if (g_Plugin.ViewCenter.x > 0 && g_Plugin.ViewCenter.x > ParentRect.left - ImgRect.left)
			g_Plugin.ViewCenter.x = Max<int>(ParentRect.left - ImgRect.left, 0);
		else if (g_Plugin.ViewCenter.x < 0 && g_Plugin.ViewCenter.x < ParentRect.right - ImgRect.left - lWidth)
			g_Plugin.ViewCenter.x = Min<int>(ParentRect.right - ImgRect.left - lWidth, 0);

		if (g_Plugin.ViewCenter.y > 0 && g_Plugin.ViewCenter.y > ParentRect.top - ImgRect.top)
			g_Plugin.ViewCenter.y = Max<int>(ParentRect.top - ImgRect.top, 0);
		else if (g_Plugin.ViewCenter.y < 0 && g_Plugin.ViewCenter.y < ParentRect.bottom - ImgRect.top - lHeight)
			g_Plugin.ViewCenter.y = Min<int>(ParentRect.bottom - ImgRect.top - lHeight, 0);

		if (g_Plugin.bCorrectMousePos)
		{
			g_Plugin.bCorrectMousePos = false;
			if (memcmp(&ViewCenterOld, &g_Plugin.ViewCenter, sizeof(POINT)))
			{
				//const вызывает инициализацию переменных ДО входа в функцию (по возможности). что нам не подходит - хочется видеть последовательность действий!
				int dx = g_Plugin.ViewCenter.x - ViewCenterOld.x;
				int dy = g_Plugin.ViewCenter.y - ViewCenterOld.y;
				g_Plugin.DragBase.x += dx;
				g_Plugin.DragBase.y += dy;
				POINT MPos;
				GetCursorPos(&MPos);
				SetCursorPos(MPos.x + dx, MPos.y + dy);
			}
		}
	}

	ImgRect.left += g_Plugin.ViewCenter.x;
	ImgRect.top += g_Plugin.ViewCenter.y;

	ImgRect.right = ImgRect.left + lWidth;
	ImgRect.bottom = ImgRect.top + lHeight;
	//
	//

	BOOL lbRc = FALSE;
	pvdInfoDisplayPaint2 Paint = {sizeof(pvdInfoDisplayPaint2)};
	Paint.hWnd = g_Plugin.hWnd;
	Paint.hParentWnd = g_Plugin.hParentWnd;
	Paint.nBackColor = g_Plugin.BackColor();
	Paint.ImageRect = ImgRect; // это вроде вообще не нужно
	Paint.nZoom = Zoom;
	Paint.nFlags = (g_Plugin.bZoomming ? PVD_IDPF_ZOOMING : 0)
	             | (g_Plugin.bScrolling ? PVD_IDPF_PANNING : 0);
	TODO("+Флаг шахматки: PVD_IDPF_CHESSBOARD");
	TODO("Убедиться, что при отпускании g_Plugin.bZoomming Post'ом вызывается OnPaint");
	            

	if (CPVDManager::pActiveDisplay && CPVDManager::pActiveDisplay != rDraw->Display())
	{
		if (CPVDManager::pActiveDisplay->pPlugin)
			CPVDManager::pActiveDisplay->pPlugin->DisplayDetach();
		CPVDManager::pActiveDisplay = rDraw->Display();
	}
	
	DWORD t0 = timeGetTime();

	Paint.Operation = PVD_IDP_BEGIN;
	static bool bDisplaySucceeded = false;
	if (rDraw->Display() &&
		rDraw->Display()->pPlugin &&
		rDraw->Display()->pPlugin->DisplayAttach() &&
		(lbRc = rDraw->Display()->pPlugin->DisplayPaint2(rDraw->Context(), &Paint)))
	{
		bDisplaySucceeded = true; // флаг того, что Begin прошел

		// Отрисовать требуемую часть изображения

		uint 
			lOutWidth = ImgRect.right - ImgRect.left, 
			lOutHeight = ImgRect.bottom - ImgRect.top;


		RECT GlobalRect = ImgRect;
		//ImgRect.left -= ParentRect.left;
		//ImgRect.right -= ParentRect.left;
		//ImgRect.top -= ParentRect.top;
		//ImgRect.bottom -= ParentRect.top;
		/*MapWindowPoints(g_Plugin.hParentWnd, NULL, (LPPOINT)&GlobalRect, 2);
		MapWindowPoints(g_Plugin.hParentWnd, NULL, (LPPOINT)&ParentRect, 2);*/

		TODO("Причесать и убрать лишнее");

		RECT *pOutRect = &GlobalRect, *pCropRect = &ParentRect;
		//Image->dds->ViewSurface(&GlobalRect, &ParentRect);
		RECT in = {0, 0, Image->Info.lDecodedWidth, Image->Info.lDecodedHeight};
		RECT out = {
			pOutRect->left, 
			pOutRect->top, 
			pOutRect->left + lOutWidth, 
			pOutRect->top + lOutHeight};
		RECT intersect;
		if (!IntersectRect(&intersect, &out, pCropRect))
		{
			_ASSERT(FALSE); // А должен пересекаться! Это же отображаемая часть картинки

			Paint.Operation = PVD_IDP_COLORFILL;
			Paint.DisplayRect = ParentRect;
			rDraw->Display()->pPlugin->DisplayPaint2(rDraw->Context(), &Paint);

			lbRc = TRUE;
		}
		else
		{
			if (memcmp(&intersect, &out, sizeof(RECT)))
			{
				in.left    = MulDivU32R(intersect.left - out.left, Image->Info.lDecodedWidth, out.right - out.left);
				in.right  -= MulDivU32R(out.right - intersect.right, Image->Info.lDecodedWidth, out.right - out.left);
				in.top     = MulDivU32R(intersect.top - out.top, Image->Info.lDecodedHeight, out.bottom - out.top);
				in.bottom -= MulDivU32R(out.bottom - intersect.bottom, Image->Info.lDecodedHeight, out.bottom - out.top);
				out = intersect;
			}
			ImgRect = out;

			WARNING("На F:\\VCProject\\FarPlugin\\ConEmu\\PictureView.img\\CMYK\\Pure-C=87-M=87-Y=0-K=87.tif проблема - низ и правый край отрисованы с огрехами");


			// Залить фоном части вокруг изображения
			// Заливаем с захватом изображения на 1 пиксел, чтобы края корректно отрисовывались (если там алиасинг будет GDI+ подглюкивает...)
			Paint.Operation = PVD_IDP_COLORFILL;
			RECT rside = ParentRect;
			if (ImgRect.top)
			{
				RECT rect = ParentRect;
				rect.bottom = ImgRect.top;
				Paint.DisplayRect = rect;
				_ASSERTE(Paint.DisplayRect.left!=Paint.DisplayRect.right && Paint.DisplayRect.top!=Paint.DisplayRect.bottom);
				rDraw->Display()->pPlugin->DisplayPaint2(rDraw->Context(), &Paint);
				SubtractRect(&rside, &rside, &rect);
			}
			if (ParentRect.bottom > (ImgRect.bottom))
			{
				RECT rect = ParentRect;
				rect.top = ImgRect.bottom;
				Paint.DisplayRect = rect;
				_ASSERTE(Paint.DisplayRect.left!=Paint.DisplayRect.right && Paint.DisplayRect.top!=Paint.DisplayRect.bottom);
				rDraw->Display()->pPlugin->DisplayPaint2(rDraw->Context(), &Paint);
				SubtractRect(&rside, &rside, &rect);
			}
			if (ImgRect.left)
			{
				RECT rect = rside;
				rect.right = ImgRect.left;
				Paint.DisplayRect = rect;
				_ASSERTE(Paint.DisplayRect.left!=Paint.DisplayRect.right && Paint.DisplayRect.top!=Paint.DisplayRect.bottom);
				rDraw->Display()->pPlugin->DisplayPaint2(rDraw->Context(), &Paint);
			}
			if (rside.right > (ImgRect.right))
			{
				rside.left = ImgRect.right;
				Paint.DisplayRect = rside;
				_ASSERTE(Paint.DisplayRect.left!=Paint.DisplayRect.right && Paint.DisplayRect.top!=Paint.DisplayRect.bottom);
				rDraw->Display()->pPlugin->DisplayPaint2(rDraw->Context(), &Paint);
			}


			// Отрисовка видимой части изображения
			Paint.Operation = PVD_IDP_PAINT;
			Paint.ImageRect = in;
			Paint.DisplayRect = out;
			lbRc = rDraw->Display()->pPlugin->DisplayPaint2(rDraw->Context(), &Paint);

			//OutputDebugString(lbRc ? L"Paint succeeded\n" : L"Paint FAILED!\n");
		}

		// Отрисовать на экране то что получилось
		Paint.Operation = PVD_IDP_COMMIT;
		rDraw->Display()->pPlugin->DisplayPaint2(rDraw->Context(), &Paint);
	}
	else
	{
		//_ASSERT(FALSE); // Почему модуль вывода отказался делать вывод?
		// Возникает после блокировки компьютера
		PAINTSTRUCT ps = {NULL};
		HDC hdc = BeginPaint(hWnd, &ps);
		RECT rc; GetClientRect(hWnd, &rc);
		HBRUSH hBr = CreateSolidBrush(g_Plugin.BackColor());
		FillRect(hdc, &rc, hBr);
		DeleteObject(hBr);
		EndPaint(hWnd, &ps);

		// И обновить картинку
		if (bDisplaySucceeded)
		{
			// Но только один раз, чтобы не зациклиться
			bDisplaySucceeded = false;
			PostMessage(g_Plugin.hWnd, DMSG_REFRESH, 0, 0);
		}
	}
	
	DWORD t1 = timeGetTime();

	// Теперь делаем сами. т.к. в GDI+ теперь BeginPaint / EndPaint не вызывается
	RECT rcClient; GetClientRect(g_Plugin.hWnd, &rcClient);
	ValidateRect(g_Plugin.hWnd, &rcClient);
	
	if (!Image->Info.bTimePaint)
	{
		Image->Info.bTimePaint = TRUE;
		Image->Info.lTimePaint = (t1 - t0);
		Image->Info.lOpenTime += (t1 - t0);
		ExecuteInMainThread((g_Plugin.FlagsWork & FW_QUICK_VIEW) ? FW_QVIEWREPAINT : FW_TITLEREPAINT,FALSE,FALSE);
	}


	//RECT GlobalRect = ImgRect;
	//ImgRect.left -= ParentRect.left;
	//ImgRect.right -= ParentRect.left;
	//ImgRect.top -= ParentRect.top;
	//ImgRect.bottom -= ParentRect.top;
	//MapWindowPoints(g_Plugin.hParentWnd, NULL, (LPPOINT)&GlobalRect, 2);
	//MapWindowPoints(g_Plugin.hParentWnd, NULL, (LPPOINT)&ParentRect, 2);
	//
	//Image->dds->ViewSurface(&GlobalRect, &ParentRect);
	//ValidateRect(hWnd, &ImgRect);

	//PAINTSTRUCT pstruct;
	//if (BeginPaint(hWnd, &pstruct))
	//{
	//	MapWindowPoints(hWnd, NULL, (LPPOINT)&pstruct.rcPaint, 2);
	//	DDBLTFX fx;
	//	ZeroMemory(&fx, sizeof(DDBLTFX));
	//	fx.dwSize = sizeof(DDBLTFX);
	//	fx.dwFillColor = _byteswap_ulong(g_Plugin.BackColor) >> 8;

	//	RECT rside = pstruct.rcPaint;
	//	if (pstruct.rcPaint.top < GlobalRect.top)
	//	{
	//		RECT rect = pstruct.rcPaint;
	//		rect.bottom = GlobalRect.top;
	//		Image->dds->m_pPrimarySurface->Blt(&rect, NULL, NULL, DDBLT_WAIT | DDBLT_COLORFILL, &fx);
	//		SubtractRect(&rside, &rside, &rect);
	//	}
	//	if (pstruct.rcPaint.bottom > GlobalRect.bottom)
	//	{
	//		RECT rect = pstruct.rcPaint;
	//		rect.top = GlobalRect.bottom;
	//		Image->dds->m_pPrimarySurface->Blt(&rect, NULL, NULL, DDBLT_WAIT | DDBLT_COLORFILL, &fx);
	//		SubtractRect(&rside, &rside, &rect);
	//	}
	//	if (rside.left < GlobalRect.left)
	//	{
	//		RECT rect = rside;
	//		rect.right = GlobalRect.left;
	//		Image->dds->m_pPrimarySurface->Blt(&rect, NULL, NULL, DDBLT_WAIT | DDBLT_COLORFILL, &fx);
	//	}
	//	if (rside.right > GlobalRect.right)
	//	{
	//		rside.left = GlobalRect.right;
	//		Image->dds->m_pPrimarySurface->Blt(&rside, NULL, NULL, DDBLT_WAIT | DDBLT_COLORFILL, &fx);
	//	}
	//	EndPaint(hWnd, &pstruct);
	//}

	//pDraw->Release();
	
	Sleep(0);
	return TRUE;
}

LRESULT OnTimer(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
		case TIMER_HIDECURSOR: // Timer for cursor hiding
			if (g_Plugin.hParentWnd != g_Plugin.hFarWnd && g_Plugin.hParentWnd != g_Plugin.hConEmuWnd
				&& !g_Plugin.bMouseHided && !g_Plugin.bDragging)
			{
				ShowCursor(FALSE);
				g_Plugin.bMouseHided = true;
			}
			KillTimer(hWnd, TIMER_HIDECURSOR);
			return TRUE;

		case TIMER_ANIMATION: // Timer for animation
			{
				CImagePtr Image;
				if (!Image.IsValid())
				{
					RequestTerminate();
					return FALSE;
				}

				if (Image->Info.Animation == 1 && Image->Info.nPages > 1 /*&& g_Manager*/)
				{
					KillTimer(hWnd, TIMER_ANIMATION);

					int nNextPage = Image->Info.nPage + 1;
					if (nNextPage >= Image->Info.nPages)
						Image->Info.nPage = 0;
					const uint lOldFrameTime = Image->Info.lFrameTime;
					DWORD nStartTick = GetTickCount();

					DecodeParams next;
					WARNING("Указать бы в флагах, что все анимированные страницы нужно кешировать?");
					next.Priority = eCurrentImageCurrentPage; // наивысший приоритет
					next.nPage = nNextPage;
					next.nRawIndex = Image->PanelItemRaw();
					
					// RequestDecodedImage - помещает запрос в очередь декодирования,
					// GetDecodedImage - дожидается результата декодирования
					CImage* pImage = g_Manager.GetDecodedImage(&next);
					if (pImage)
					{
						// При ошибке - попробовать перейти к 0-фрейму
						if (nNextPage > 0)
						{
							next.nPage = 0;
							pImage = g_Manager.GetDecodedImage(&next);
						}

						if (!pImage)
						{
							// Остановим анимацию
							return TRUE;
						}
					}

					Image.Assign(pImage);

					//g_Manager.Decode(Image, &Image->mp_Draw, true);

					DWORD nDeltaTick = GetTickCount() - nStartTick;
					// Учесть в задержке следующего фрейма время декодирования текущего
					uint lNewFrameTime = Image->Info.lFrameTime;
					lNewFrameTime = (lNewFrameTime > (nDeltaTick+10)) 
						? (lNewFrameTime-nDeltaTick) : 10;

					SetTimer(hWnd, TIMER_ANIMATION, lNewFrameTime, NULL);

					InvalidateRect(hWnd, NULL, FALSE);
				}
				else
				{
					KillTimer(hWnd, TIMER_ANIMATION);
				}
			}
			return TRUE;

		case TIMER_REDRAWCON:
			// После активации консоль гадит - насильно вызывает Paint
			if ((GetTickCount() - gnRedrawConTimerStart) >= TIMER_REDRAWCON_TIMEOUT)
			{
				KillTimer(hWnd, TIMER_REDRAWCON);
			}
			else if (!g_Plugin.hConEmuWnd && (g_Plugin.hFarWnd != GetForegroundWindow()))
			{
				KillTimer(hWnd, TIMER_REDRAWCON);
			}
			else
			{
				InvalidateRect(g_Plugin.hWnd, NULL, FALSE);
				WndProc(g_Plugin.hWnd, WM_PAINT, 0,0); // ибо _try
			}
			return TRUE;

		case TIMER_GOTOPAGE:
			KillTimer(hWnd, TIMER_GOTOPAGE);
			if (gbGotoPageNo) {
				gbGotoPageNo = false;
				OnGotoPage(gnGotoPageNo-1, TRUE); // TRUE, чтобы сбросился PAN
				gnGotoPageNo = 0;
			}
			return TRUE;
	}
	return TRUE;
}

LRESULT OnWallpaper(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
	CImagePtr Image;
	if (!Image.IsValid())
	{
		RequestTerminate();
		return FALSE;
	}

	TODO("MIWallpaperConfirm"); // но так как нехорошо звать функи из другой нити - ресурсы нужно загружать при запуске треда дисплея
	if (MessageBox(NULL, L"Set current image as desktop wallpaper?", L"PictureView", MB_YESNO|MB_ICONQUESTION|MB_SETFOREGROUND)!=IDYES)
		return FALSE;
		

	//HRESULT hr;
	//IActiveDesktop *pActiveDesktop = NULL;

	//hr = CoInitialize(NULL);
	//if (SUCCEEDED(hr)) {

	//	//Create an instance of the Active Desktop
	//	hr = CoCreateInstance(CLSID_ActiveDesktop, NULL, CLSCTX_INPROC_SERVER,
	//						  IID_IActiveDesktop, (void**)&pActiveDesktop);

	//	if (SUCCEEDED(hr) && pActiveDesktop) {
			LPCWSTR pszSrcFile = (LPCWSTR)Image->FileName;
			LPCWSTR pszDot = wcsrchr(pszSrcFile, L'.');
			if (!pszDot || wcschr(pszDot, L'\\'))
				pszDot = L".bmp"; // расширение по умолчанию?
			wchar_t szUserName[MAX_PATH];
			TODO("Кидать в AppData...?");
			lstrcpyW(szUserName, L"\\\\?\\C:\\PictureView");
			lstrcatW(szUserName, pszDot);
			
			if (CopyFile(pszSrcFile, szUserName, FALSE)) {
				pszSrcFile = szUserName;
				CUnicodeFileName::SkipPrefix(&pszSrcFile);
				//hr = pActiveDesktop->SetWallpaper(pszSrcFile, 0);
				WARNING("Windows Server 2003 and Windows XP/2000:  The pvParam parameter cannot specify a .jpg file.");
				// SPIF_SENDCHANGE - низя. повиснет консоль
				SystemParametersInfoW(SPI_SETDESKWALLPAPER, 0, (LPVOID)pszSrcFile, 0);
				TODO("SetWallpaperOptions? Ругнуться при ошибке");
			} else {
				TODO("Ругнуться при ошибке копирования");
			}

			// Call the Release method
			//pActiveDesktop->Release();
		//} else {
		//	TODO("Ругнуться при ошибке интерфейса");
		//}

	//	CoUninitialize();
	//} else {
	//	TODO("Ругнуться при ошибке интерфейса");
	//}
	
	return TRUE;
}

LRESULT OnHelp(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
	ExecuteInMainThread(FW_SHOW_HELP, FALSE/*bRestoreCursor*/, TRUE);
	return TRUE;

//	CPVDManager::DisplayDetach();
//	ShowWindow(hWnd, SW_HIDE);
//	TrayRestore();
//
//#ifndef _DEBUG
//	{
//		wchar_t Title[0x40];
//		lstrcpynW(Title, GetMsg((MsgIds)0), sizeofarray(Title) - 6);
//		lstrcat(Title, L" - Far");
//		SetConsoleTitleW(Title);
//	}
//#endif
//	
//	//g_StartupInfo.ShowHelp(g_StartupInfo.ModuleName, NULL, 0);
//	ResetEvent(g_Plugin.hSynchroDone);
//	g_Plugin.FlagsWork |= FW_SHOW_HELP;
//	
//	bool lbBreak = false;
//	while (!lbBreak && WaitForSingleObject(g_Plugin.hSynchroDone, 10) != WAIT_OBJECT_0)
//	{
//		MSG  lpMsg;
//		while (PeekMessage(&lpMsg, NULL, 0, 0, PM_REMOVE))
//		{
//			if (lpMsg.message == WM_QUIT || lpMsg.message == WM_DESTROY) {
//				// вернуть обратно в очередь
//				PostMessage(lpMsg.hwnd, lpMsg.message, lpMsg.wParam, lpMsg.lParam);
//				// и выйти
//				lbBreak = true; break;
//			}
//			DefWindowProc(lpMsg.hwnd, lpMsg.message, lpMsg.wParam, lpMsg.lParam);
//		}
//	}
//	
//	if (g_Plugin.hParentWnd != g_Plugin.hFarWnd)
//		TrayDisable();
//	ShowWindow(hWnd, SW_SHOW);
//	g_Plugin.FlagsDisplay |= FD_TITLE_REPAINT;
//	InvalidateRect(hWnd, NULL, FALSE);
//	return TRUE;
}

LRESULT OnLButtonDown(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
	if (!(g_Plugin.FlagsWork & FW_QUICK_VIEW))
	{
		g_Plugin.bDragging = true;
		g_Plugin.DragBase.x = (i16)lParam;
		g_Plugin.DragBase.y = (i16)(lParam >> 0x10);
		if (g_Plugin.bMouseHided)
		{
			ShowCursor(TRUE);
			g_Plugin.bMouseHided = false;
		}
	}

	if (!g_Plugin.bFullScreen) {
		//2009-11-01 Убрал
		SetForegroundWindow(g_Plugin.hConEmuWnd ? g_Plugin.hConEmuWnd : g_Plugin.hFarWnd);
		//2009-11-01 Убрал. т.к. ставим WS_CHILD
		SetCapture(hWnd); // need to be foreground window
	}
	
	return TRUE;
}

LRESULT OnLButtonUp(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
	//lLButtonUp:
	if (g_Plugin.FlagsWork & FW_QUICK_VIEW)
		return TRUE;
	ReleaseCapture();
	g_Plugin.bDragging = false;
	if (g_Plugin.hParentWnd != g_Plugin.hFarWnd && g_Plugin.hParentWnd != g_Plugin.hConEmuWnd
		&& !g_Plugin.bMouseHided)
	{
		ShowCursor(FALSE);
		g_Plugin.bMouseHided = true;
	}
	return TRUE;
}

LRESULT OnMouseMove(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
	if (g_Plugin.bDragging)
	{
		//OutputDebugString(L"Dragging with mouse\n");

		if (!(wParam & MK_LBUTTON)) {
			//goto lLButtonUp;
			OnLButtonUp(hWnd, messg, wParam, lParam);
			return TRUE;
		}
		const int dx = (i16)lParam - g_Plugin.DragBase.x;
		const int dy = (i16)(lParam >> 0x10) - g_Plugin.DragBase.y;
		if (dx || dy)
		{
			g_Plugin.ViewCenter.x += dx;
			g_Plugin.ViewCenter.y += dy;
			InvalidateRect(hWnd, NULL, FALSE);
		} else {
			//OutputDebugString(L"dx==dy==0\n");
		}
	}
	else
	{
		if (g_Plugin.hParentWnd != g_Plugin.hFarWnd && g_Plugin.hParentWnd != g_Plugin.hConEmuWnd
			&& (i16)lParam - g_Plugin.DragBase.x && (i16)(lParam >> 0x10) - g_Plugin.DragBase.y)
		{
			if (g_Plugin.bIgnoreMouseMove)
				g_Plugin.bIgnoreMouseMove = false;
			else
			{
				if (g_Plugin.bMouseHided)
				{
					ShowCursor(TRUE);
					g_Plugin.bMouseHided = false;
				}
				SetTimer(hWnd, TIMER_HIDECURSOR, 1000, NULL);
			}
		}
	}
	g_Plugin.DragBase.x = (i16)lParam;
	g_Plugin.DragBase.y = (i16)(lParam >> 0x10);
	
	return TRUE;
}

BOOL IsScrollingPages(LPARAM lParam)
{
	BOOL bScrollPages = 0 != (lParam & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED)); // | LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED));
	// Но g_Plugin.nAutoPagingVK (ScrollLock) действие инвертирует
	if (g_Plugin.bAutoPaging) {
		if ((GetKeyState(g_Plugin.nAutoPagingVK) & 1) == 1)
			bScrollPages = !bScrollPages;
	}
	return bScrollPages;
}

BOOL OnImageReady(HWND hWnd, UINT messg, WPARAM wParam, CDecodeItem* pReady)
{
	//CDecodeItem* pReady = (CDecodeItem*)lParam;
	if (!pReady)
	{
		_ASSERTE(pReady!=NULL);
		RequestTerminate();
		return FALSE;
	}

	bool bActivate = ((pReady->Params.Flags & eRenderActivateOnReady) == eRenderActivateOnReady);

	// Важно, чтобы это выполнялось в потоке дисплея
	// Выполнить перенос данных в контекст дисплея
	if (!g_Manager.OnItemReady(pReady))
		bActivate = false;

	if (bActivate)
	{
		int lOldWidth = 0;
		int lOldHeight = 0;

		// Сначала - получить текущие параметры отображения (чтобы при листании не сбиваться)
		{
			CImagePtr OldImage;
			if (!OldImage.IsValid())
			{
				_ASSERTE(OldImage.IsValid());
				SafeRelease(pReady,szPVDManager);
				RequestTerminate();
				return FALSE;
			}

			lOldWidth = OldImage->Info.lWidth;
			lOldHeight = OldImage->Info.lHeight;
			TODO("Зум, смещение?");
		}

		g_Panel.SetActiveRawIndex((int)wParam);
	
		// Активировали файл, проверим что получилось
		CImagePtr Image;
		if (!Image.IsValid() || (Image->PanelItemRaw() != wParam))
		{
			_ASSERTE(Image->PanelItemRaw() == wParam);
			SafeRelease(pReady,szPVDManager);
			RequestTerminate();
			return FALSE;
		}

		//Image->Info.nPage = (uint)lParam;
		Image->Info = pReady->Info;

		// Заменить дескриптор декодера
		pReady->pFile->MoveTo(Image->mp_File);
		//WARNING("TODO: Выполнить перенос данных в контекст дисплея");

		//pReady->pDraw->MoveTo(Image->mp_Draw);
		// Освободить память под pReady
		SafeRelease(pReady,szPVDManager);

		
		if (!InitImageDisplay(!g_Plugin.bKeepZoomAndPosBetweenFiles))
		{
			SafeRelease(pReady,szPVDManager);
			return FALSE; // уже вызван RequestTerminate
		}

		// Если хотят сохранять масштаб/положение
		uint nCorner = g_Plugin.nKeepPanCorner + MIKeepPosCenter;
		if (g_Plugin.bKeepZoomAndPosBetweenFiles
			&& (g_Plugin.bKeepZoomAndPosBetweenFiles && (nCorner != MIKeepZoomOnly))
			// и ранее положение не было запомнено
			&& (g_Plugin.ViewCenter.x != RESET_VIEW_POS/*0x7FFFFFFF*/ && g_Plugin.ViewCenter.y != RESET_VIEW_POS/*0x7FFFFFFF*/)
			&& g_Plugin.AbsoluteZoom)
		{
			RECT ParentRect = GetDisplayRect();
			int lScreenWidth = ParentRect.right - ParentRect.left;
			int lScreenHeight = ParentRect.bottom - ParentRect.top;

			lOldWidth = MulDivU32R(lOldWidth, g_Plugin.AbsoluteZoom, 0x10000);;
			lOldHeight = MulDivU32R(lOldHeight, g_Plugin.AbsoluteZoom, 0x10000);;
			int lNewWidth = MulDivU32R(Image->Info.lWidth, g_Plugin.AbsoluteZoom, 0x10000);;
			int lNewHeight = MulDivU32R(Image->Info.lHeight, g_Plugin.AbsoluteZoom, 0x10000);;

			if (lNewWidth != lOldWidth || lNewHeight != lOldHeight)
			{
				lOldWidth = (lOldWidth - lScreenWidth) / 2;
				lNewWidth = (lNewWidth - lScreenWidth) / 2;
				lOldHeight = (lOldHeight - lScreenHeight) / 2;
				lNewHeight = (lNewHeight - lScreenHeight) / 2;

				//uint nCorner = g_Plugin.nKeepPanCorner + MIKeepPosCenter;
				if (nCorner == MIKeepPosFromEdges)
				{
					if (lOldWidth && lNewHeight > 0)
						g_Plugin.ViewCenter.x = MulDivIU32R(g_Plugin.ViewCenter.x,lNewWidth,lOldWidth);
					if (lOldHeight && lNewWidth > 0)
						g_Plugin.ViewCenter.y = MulDivIU32R(g_Plugin.ViewCenter.y,lNewHeight,lOldHeight);
				}
			}
		}

		g_Plugin.FlagsDisplay |= FD_TITLE_REPAINT;

		WndProc(hWnd, WM_PAINT, 0, 0); // а не OnPaint, ибо _try

		SetEvent(g_Plugin.hDisplayEvent);
	}

	SafeRelease(pReady,szPVDManager);
	return TRUE;
}

LRESULT WndProcSEH(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 0;
	switch (messg)
	{
	case WM_PAINT:
	{
		OnPaint(hWnd, messg, wParam, lParam);
		break;
	}
	case WM_ERASEBKGND:
		result = 1;
		break;
	case WM_RBUTTONUP:
		PostMessage(hWnd, DMSG_KEYBOARD, VK_ESCAPE, 0);
		SetForegroundWindow(g_Plugin.hConEmuWnd ? g_Plugin.hConEmuWnd : g_Plugin.hFarWnd);
	case WM_RBUTTONDOWN:
		break;
	case WM_MBUTTONDOWN:
		PostMessage(hWnd, DMSG_KEYBOARD, VK_DIVIDE, 0);
		//2009-11-01 Убрал
		//SetForegroundWindow(g_Plugin.hConEmuWnd ? g_Plugin.hConEmuWnd : g_Plugin.hFarWnd);
		break;
	case WM_LBUTTONDBLCLK:
		OutputDebugString(L"\nWM_LBUTTONDBLCLK\n");
		gbDblClkFullScreen = true;
		//if (g_Plugin.bFullScreen) {
		//	gbTurnOffFullScreen = true;
		//	break;
		//}
		//gbTurnOffFullScreen = false;
		//OnFullScreen(hWnd, messg, wParam, lParam);
		break;
	case WM_CAPTURECHANGED:
		SetForegroundWindow(g_Plugin.hConEmuWnd ? g_Plugin.hConEmuWnd : g_Plugin.hFarWnd);
		break;
	case WM_LBUTTONDOWN:
		OutputDebugString(L"\nWM_LBUTTONDOWN\n");
		OnLButtonDown(hWnd, messg, wParam, lParam);
		break;
	case WM_LBUTTONUP:
		OutputDebugString(L"\nWM_LBUTTONUP\n");
		if (gbDblClkFullScreen) {
			gbDblClkFullScreen = false;
			//OnFullScreen(hWnd, messg, wParam, lParam);
			PostMessage(hWnd, DMSG_FULLSCREENSWITCH, 0, 0);
		} else {
			OnLButtonUp(hWnd, messg, wParam, lParam);
		}
		break;
	case WM_MOUSEMOVE:
		OnMouseMove(hWnd, messg, wParam, lParam);
		break;
	case WM_MOUSEWHEEL:
		{
			// Переслать в DMSG_KEYBOARD в формате консольных событий
			LPARAM KeyFlags = 0;
			SHORT ks;
			if ((ks = GetKeyState(VK_CAPITAL)) & 1)
				KeyFlags |= CAPSLOCK_ON;
			if ((ks = GetKeyState(VK_NUMLOCK)) & 1)
				KeyFlags |= NUMLOCK_ON;
			if ((ks = GetKeyState(VK_SCROLL)) & 1)
				KeyFlags |= SCROLLLOCK_ON;

			if ((ks = GetKeyState(VK_LMENU)) & 0x8000)
				KeyFlags |= LEFT_ALT_PRESSED;
			if ((ks = GetKeyState(VK_LCONTROL)) & 0x8000)
				KeyFlags |= LEFT_CTRL_PRESSED;
			if ((ks = GetKeyState(VK_RMENU)) & 0x8000)
				KeyFlags |= RIGHT_ALT_PRESSED;
			if ((ks = GetKeyState(VK_RCONTROL)) & 0x8000)
				KeyFlags |= RIGHT_CTRL_PRESSED;
			if ((ks = GetKeyState(VK_SHIFT)) & 0x8000)
				KeyFlags |= SHIFT_PRESSED;

			// При масштабировании посылаем (WM_APP + 2), чтобы с листанием не пересечься
			if ((wParam & MK_LBUTTON) || (KeyFlags & (LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED)))
				PostMessage(g_Plugin.hWnd, DMSG_ZOOMBYMOUSE/*WM_APP + 2*/, wParam, KeyFlags);
			else
				PostMessage(g_Plugin.hWnd, DMSG_NEXTFILEPAGE/*WM_APP + 1*/, wParam, KeyFlags);
		}
		break;
	case WM_TIMER:
		OnTimer(hWnd, messg, wParam, lParam);
		break;
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		{
			// Переслать в DMSG_KEYBOARD в формате консольных событий
			LPARAM KeyFlags = 0;
			SHORT ks;
			if ((ks = GetKeyState(VK_CAPITAL)) & 1)
				KeyFlags |= CAPSLOCK_ON;
			if ((ks = GetKeyState(VK_NUMLOCK)) & 1)
				KeyFlags |= NUMLOCK_ON;
			if ((ks = GetKeyState(VK_SCROLL)) & 1)
				KeyFlags |= SCROLLLOCK_ON;
				
			if ((ks = GetKeyState(VK_LMENU)) & 0x8000)
				KeyFlags |= LEFT_ALT_PRESSED;
			if ((ks = GetKeyState(VK_LCONTROL)) & 0x8000)
				KeyFlags |= LEFT_CTRL_PRESSED;
			if ((ks = GetKeyState(VK_RMENU)) & 0x8000)
				KeyFlags |= RIGHT_ALT_PRESSED;
			if ((ks = GetKeyState(VK_RCONTROL)) & 0x8000)
				KeyFlags |= RIGHT_CTRL_PRESSED;
			if ((ks = GetKeyState(VK_SHIFT)) & 0x8000)
				KeyFlags |= SHIFT_PRESSED;

			PostMessage(hWnd, DMSG_KEYBOARD, wParam, KeyFlags);
			break;
		}
	case DMSG_KEYUP:
		switch (wParam)
		{
		case VK_ADD: // Gray+
		case VK_OEM_PLUS:
			break;
		case VK_SUBTRACT: // Gray-
		case VK_OEM_MINUS:
			break;
		case VK_RIGHT:
		case VK_LEFT:
		case VK_DOWN:
		case VK_UP:
		case VK_NUMPAD2:
		case VK_NUMPAD4:
		case VK_NUMPAD6:
		case VK_NUMPAD8:
			//OnScroll(hWnd, messg, wParam, lParam);
			break;
		}
		break;
	case DMSG_KEYBOARD:
		switch (wParam)
		{
		case VK_F4:
			if (!(lParam & (LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED)))
				break;
		case VK_ESCAPE:
			// VK_ESCAPE может быть послено из WM_RBUTTONUP: PostMessage(hWnd, DMSG_KEYBOARD, VK_ESCAPE, 0);
		case VK_RETURN:
		case VK_F3:
		case VK_F10:
			// Если вводили номер страницы для прыжка
			if (wParam == VK_RETURN && gbGotoPageNo) {
				gbGotoPageNo = false;
				OnGotoPage(gnGotoPageNo-1, TRUE); // TRUE, чтобы сбросился PAN
				gnGotoPageNo = 0;
				break;
			}
			RequestTerminate();
			//TODO("Остановить все декодеры");
			//g_Plugin.FlagsWork |= FW_TERMINATE;
			////CPVDManager::Display Detach(); // 2010-06-06 -> DisplayExit
			//CPVDManager::DisplayExit();
			//DestroyWindow(hWnd);
			break;
		case VK_F1:
			OnHelp(hWnd, messg, wParam, lParam);
			break;
		case VK_F2:
			if (lParam & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED) && lParam & (LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED) && lParam & SHIFT_PRESSED)
			{
				g_Plugin.bTrayOnTopDisabled = true;
				TrayRestore();
				break;
			}
			break;
		case VK_F9:
			OnConfig(hWnd, messg, wParam, lParam);
			break;
		case VK_F5:
			OnRefresh(hWnd, messg, wParam, lParam);
			break;
		case VK_ADD:      // Gray+
		case VK_OEM_PLUS: // '+' any country
			vkAdd = wParam;
			OnZoomIn(hWnd, messg, wParam, lParam);
			break;
		case VK_SUBTRACT:  // Gray-
		case VK_OEM_MINUS: // '-' any country
			vkSubtract = wParam;
			OnZoomOut(hWnd, messg, wParam, lParam);
			break;
		case VK_MULTIPLY: // Gray*
			// Масштаб 100% / Автомасштабирование по ширине / по высоте
			OnZoom100(hWnd, messg, wParam, lParam);
			break;
		case VK_DIVIDE: // Gray/ 
		case VK_OEM_2:  // '/?' for US - кнопка рядом с правым шифтом
			// Переключение: автомасштабирование / масштаб 100%
			OnAutoZoom(hWnd, messg, wParam, lParam);
			break;
		case VK_RIGHT:
		case VK_LEFT:
		case VK_DOWN:
		case VK_UP:
		case VK_NUMPAD2:
		case VK_NUMPAD4:
		case VK_NUMPAD6:
		case VK_NUMPAD8:
			OnScroll(hWnd, messg, wParam, lParam);
			break;
		case VK_NEXT:
		case VK_PRIOR:
			if (!(lParam & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED)) && (lParam & (LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED)) && !(lParam & SHIFT_PRESSED))
			{
				if (OnChangeDecoder(hWnd, messg, wParam, lParam))
					break;
			}
		case VK_BACK:
		case VK_NUMPAD9:
		case 'B':
		case VK_OEM_4: //'[':
		case VK_SPACE:
		case VK_NUMPAD3:
		case 'N':
		case VK_OEM_6: //']':
			if (wParam == VK_SPACE && g_Plugin.bMarkBySpace) {
				BOOL bScrollPages = IsScrollingPages(lParam);
				if (!bScrollPages)
					OnMark(TRUE/*abNewSelected*/);
			}
				
			if (wParam == VK_NEXT || wParam == VK_SPACE || wParam == VK_NUMPAD3 || wParam == 'N' || wParam == VK_OEM_6/*']'*/) {
				OutputDebugString(L"PgDn pressed, switching to next file\n");
				PostMessage(hWnd, DMSG_NEXTFILEPAGE, 0x80000000, lParam);
			} else {
				OutputDebugString(L"PgDn pressed, switching to prev file\n");
				PostMessage(hWnd, DMSG_NEXTFILEPAGE, 0, lParam);
			}
			break;
		case VK_INSERT:
		case VK_DELETE:
		case VK_NUMPAD0:
		case VK_DECIMAL:
			OnMark(wParam == VK_INSERT || wParam == VK_NUMPAD0/*abNewSelected*/);
			break;
		case VK_CLEAR:
		case VK_NUMPAD5:
		case 'F':
			OutputDebugString(L"FullScreen switching by 'F'\n");
			OnFullScreen(hWnd, messg, wParam, lParam);
			break;
		case VK_HOME:
		case VK_NUMPAD7:
		case 'R':
			{
				BOOL bScrollPages = IsScrollingPages(lParam);
				if (bScrollPages) {
					OnGotoPage(0, TRUE);
				} else {
					//if (g_Plugin.FlagsWork & FW_JUMP_DISABLED)
					//	break;
					//g_Plugin.FlagsDisplay |= FD_JUMP_NEXT | FD_HOME_END;
					//goto lJump;
					//OnNextFileJump(hWnd, messg, wParam, lParam);
					RequestNextImage(false/*abNext*/, true/*abHomeEnd*/, true/*abActivate*/);
				}
				break;
			}
		case VK_END:
		case VK_NUMPAD1:
		case 'E':
			{
				CImagePtr Image;
				if (!Image.IsValid())
				{
					RequestTerminate();
					return FALSE;
				}

				BOOL bScrollPages = IsScrollingPages(lParam);
				if (bScrollPages) {
					OnGotoPage(Image->Info.nPages-1, TRUE);
				} else {
					//if (g_Plugin.FlagsWork & FW_JUMP_DISABLED)
					//	break;
					//g_Plugin.FlagsDisplay = g_Plugin.FlagsDisplay & ~FD_JUMP_NEXT | FD_HOME_END;
					//goto lJump;
					//OnNextFileJump(hWnd, messg, wParam, lParam);
					RequestNextImage(true/*abNext*/, true/*abHomeEnd*/, true/*abActivate*/);
				}
				break;
			}
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			if (!(lParam & (LEFT_CTRL_PRESSED|RIGHT_CTRL_PRESSED|LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED|SHIFT_PRESSED))) {
				OnGotoPageAddNumber(((UINT)wParam) - ((UINT)'0'));
			} else if (SHIFT_PRESSED == (lParam & (LEFT_CTRL_PRESSED|RIGHT_CTRL_PRESSED|LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED|SHIFT_PRESSED))) {
				switch (wParam) {
					case '8': // '*' - аналог Gray*, но в флаге нужно сбросить Shift
						// Масштаб 100% / Автомасштабирование по ширине / по высоте
						OnZoom100(hWnd, messg, wParam, (lParam & ~SHIFT_PRESSED));
						break;
				}
			}
			break;
		case 'W':
			if ((lParam & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED))) {
				OnWallpaper(hWnd, messg, wParam, lParam);
			}
			break;
		#ifdef _DEBUG
		default:
			if (wParam != VK_SHIFT && wParam != VK_CONTROL && wParam != VK_MENU) {
				wParam = wParam;
			}
		#endif
		}
		break;
	case DMSG_NEXTFILEPAGE /*WM_APP + 1*/:
		{
			// По умолчанию PgUp/PgDn листают файлы, а при зажатом Ctrl - страницы
			BOOL bScrollPages = IsScrollingPages(lParam);
			//	0 != (lParam & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED)); // | LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED));
			//// Но g_Plugin.nAutoPagingVK (ScrollLock) действие инвертирует
			//if (g_Plugin.bAutoPaging) {
			//	if ((GetKeyState(g_Plugin.nAutoPagingVK) & 1) == 1)
			//		bScrollPages = !bScrollPages;
			//}

			if (bScrollPages)
				OnNextPage(hWnd, messg, wParam, lParam);
			else
				OnNextFile(hWnd, messg, wParam, lParam);
		}
		break;
	case DMSG_ZOOMBYMOUSE /*WM_APP + 2*/:
		// Масштабирование колесом мышки
		if ((wParam & FROM_LEFT_1ST_BUTTON_PRESSED) || (lParam & (LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED)))
			OnZoomByWheel(hWnd, messg, wParam, lParam);
		break;
	case DMSG_FULLSCREENSWITCH /*WM_APP + 3*/:
		OutputDebugString(L"Post FullScreen switching\n");
		OnFullScreen(hWnd, messg, wParam, lParam);
		break;
	case DMSG_REFRESH /*WM_APP + 4*/:
		OnRefresh(hWnd, messg, wParam, lParam);
		break;
	case DMSG_IMAGEREADY:
		OnImageReady(hWnd, messg, wParam, (CDecodeItem*)lParam);
		break;
	//case DMSG_SHOWWINDOW /*WM_APP + 6*/:
	//	result = 0;
	//	if ((wParam && 0xFFFF) == DMSG_SHOWWINDOW_KEY)
	//		if (ShowWindow(hWnd, ((wParam && 0xFFFF0000) >> 16)))
	//			result = lParam;
	//	break;
	case WM_DESTROY:
		// Это нужно выполнить ДО закрытия окна (зачистка неиспользуемых консолью участков клиентской части окна)
		//CPVDManager::DisplayExit();
		// Позвать завершение, на всякий случай
		RequestTerminate(1);
		//g_Plugin.FlagsWork |= FW_TERMINATE; // завершить все обработки и закрыть нить DisplayThread
		//CPVDManager::DisplayExit();
		////2010-04-30 Уберем, очередь потом почистится в конце DisplayThreadProcSEH
		////for (MSG lpMsg; PeekMessage(&lpMsg, NULL, 0, 0, PM_REMOVE););
		PostQuitMessage(0);
		break;

	default:
		result = DefWindowProc(hWnd, messg, wParam, lParam);
	}

	return result;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lRc = 0;
	bool lbException = false;

	__try {
		lRc = WndProcSEH(hWnd, messg, wParam, lParam);
	}__except( EXCEPTION_EXECUTE_HANDLER ){
		lRc = 0; lbException = true;
	}

	if (lbException)
	{
		CFunctionLogger::FunctionLogger(L"!!! Exception in WndProcSEH");
		wchar_t szInfo[1024];
		wsprintfW(szInfo, L"!!! Exception in WndProcSEH\nMsg=%u, wParam=0x%08X, lParam=0x%08X\nPlease, restart FAR",
			messg, (DWORD)wParam, (DWORD)lParam);
		MessageBox(NULL, szInfo, L"PictureView2", MB_OK|MB_ICONSTOP|MB_SETFOREGROUND|MB_SYSTEMMODAL);
	}

	return lRc;
}

// Возвращает количество событий, оставшихся в буфере консоли
UINT ProcessConsoleInputs()
{
	// Видимость окна проверять нельзя, иначе плаг "повиснет" если окно не удалось отображить
	if (!g_Plugin.hWnd || !IsWindow(g_Plugin.hWnd) /*|| !IsWindowVisible(g_Plugin.hWnd)*/)
	{
		_ASSERTE(g_Plugin.hWnd && IsWindow(g_Plugin.hWnd));
		RequestTerminate();
		return 0;
	}

	// gbLockInputProcessing ?

	CFunctionLogger::FunctionLogger(L"Processing console input.1");
	u32 nEvents = 0;
	HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);

	GetNumberOfConsoleInputEvents(hInput, &nEvents);
	CFunctionLogger::FunctionLogger(L"Processing console input.2");
	if (!(g_Plugin.FlagsWork & FW_QUICK_VIEW))
	{
		if (nEvents)
		{
			INPUT_RECORD ir;
			u32 t;
			CFunctionLogger::FunctionLogger(L"Processing console input.3");
			ReadConsoleInput(hInput, &ir, 1, &t);
			CFunctionLogger::FunctionLogger(L"Processing console input.4");
			if (t)
			{
				if (ir.EventType == KEY_EVENT) {
					if (ir.Event.KeyEvent.bKeyDown) {
						PostMessage(g_Plugin.hWnd, DMSG_KEYBOARD, ir.Event.KeyEvent.wVirtualKeyCode, ir.Event.KeyEvent.dwControlKeyState);
					} else {
						// Пока не используется, на будущее, чтобы при отпускании кнопок зума-скролла передернуть экран
						PostMessage(g_Plugin.hWnd, DMSG_KEYUP, ir.Event.KeyEvent.wVirtualKeyCode, ir.Event.KeyEvent.dwControlKeyState);
					}
				} else if (ir.EventType == MOUSE_EVENT && ir.Event.MouseEvent.dwEventFlags & MOUSE_WHEELED) {
					WPARAM wParam = ir.Event.MouseEvent.dwButtonState;
					LPARAM lParam = ir.Event.MouseEvent.dwControlKeyState | ir.Event.MouseEvent.dwEventFlags << 12;
					// При масштабировании посылаем (WM_APP + 2), чтобы с листанием не пересечься
					if ((wParam & FROM_LEFT_1ST_BUTTON_PRESSED) || (lParam & (LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED)))
						PostMessage(g_Plugin.hWnd, DMSG_ZOOMBYMOUSE/*WM_APP + 2*/, wParam, lParam);
					else
						PostMessage(g_Plugin.hWnd, DMSG_NEXTFILEPAGE/*WM_APP + 1*/, wParam, lParam);
				} 
				else if (ir.EventType == FOCUS_EVENT && ir.Event.FocusEvent.bSetFocus) {
					if (g_Plugin.bFullScreen) {
						SetForegroundWindow(g_Plugin.hWnd);
					}
				//	//Sleep(100); // Консоль будет пытаться отрисоваться
				//	InvalidateRect(g_Plugin.hWnd, NULL, FALSE);
				//	OnPaint(g_Plugin.hWnd, WM_PAINT, 0,0);
				//	gnRedrawConTimerStart = GetTickCount();
				//	SetTimer(g_Plugin.hWnd, TIMER_REDRAWCON, 500, 0);
				}
			}
		}
	}
	else
	{
		if (nEvents)
		{
			INPUT_RECORD ir;
			u32 t;
			CFunctionLogger::FunctionLogger(L"Processing console input.5");
			PeekConsoleInput(hInput, &ir, 1, &t);
			CFunctionLogger::FunctionLogger(L"Processing console input.6");
			if (t && (
				(ir.EventType == KEY_EVENT && ir.Event.KeyEvent.bKeyDown && (ir.Event.KeyEvent.wVirtualKeyCode > 0x12 || ir.Event.KeyEvent.wVirtualKeyCode < 0x10)) || 
				(ir.EventType == MOUSE_EVENT && ir.Event.MouseEvent.dwEventFlags & ~MOUSE_MOVED)
				))
			{
				if (ir.EventType == KEY_EVENT && ir.Event.KeyEvent.wVirtualKeyCode == VK_ESCAPE) {
					// Просто закрыть PicView, но панели не гасить. Достало.
					CFunctionLogger::FunctionLogger(L"Processing console input.Esc.clear");
					// Убрать из буфера Esc Press/release
					ReadConsoleInput(hInput, &ir, 1, &t);
					while (PeekConsoleInput(hInput, &ir, 1, &t) && t>0) {
						ReadConsoleInput(hInput, &ir, 1, &t);
						if (ir.EventType == KEY_EVENT && ir.Event.KeyEvent.wVirtualKeyCode == VK_ESCAPE)
							break;
					}
					CFunctionLogger::FunctionLogger(L"Processing console input.Esc.done");
				}
				TODO("QView: Хорошо бы попробовать не закрывать окно с предыдущей картинкой до окончания открытия новой...");
				RequestTerminate();
				//g_Plugin.FlagsWork |= FW_TERMINATE;
				//PostMessage(g_Plugin.hWnd, WM_DESTROY, 0, 0);
				return -1;
			}
			else if (t)
			{
				CFunctionLogger::FunctionLogger(L"Processing console input.7");
				ReadConsoleInput(hInput, &ir, 1, &t);
				CFunctionLogger::FunctionLogger(L"Processing console input.8");
				/*if (ir.EventType == FOCUS_EVENT && ir.Event.FocusEvent.bSetFocus) { -- это у нас не крутится..
				InvalidateRect(g_Plugin.hWnd, NULL, FALSE);
				}*/
			}
		}
	}
	if (nEvents) nEvents--;
	return nEvents;
}

DWORD WINAPI DisplayThreadProcSEH(void *pParameter)
{
	// ЗДЕСЬ - это делать НЕЛЬЗЯ. Основная нить сейчас ждет, когда будет создано окно дисплея!
	//if (!(g_Plugin.FlagsWork & FW_QUICK_VIEW))
	//	ExecuteInMainThread(FW_TITLEREPAINTD,FALSE,FALSE);

	if (g_Plugin.bFullDisplayReInit)
	{
		//g_Panel.CloseDisplayHandles();
		g_Manager.DisplayExit(); // сам позовет g_Panel.CloseDisplayHandles();
	}
	
	bool result = TRUE;

	//if (result)
	//{
	_ASSERTE(g_Plugin.hFarWnd && g_Plugin.hParentWnd);
	_ASSERTE(g_Plugin.hConEmuWnd==NULL || g_Plugin.hParentWnd!=g_Plugin.hFarWnd);

	HWND hParent = g_Plugin.hParentWnd;

	g_Plugin.hWnd = CreateWindowExW(EX_STYLE_NOFULLSCREEN,
		g_WndClassName, L"PictureView", STYLE_NOFULLSCREEN,
		0, 0, 1/*FarWndRect.right*/, 1/*FarWndRect.bottom*/, 
		hParent, NULL, g_hInstance, NULL);
	result = g_Plugin.hWnd;
	//}


	//SetEvent(g_Plugin.hDisplay Event);
	if (!result)
		return 0;

	SetEvent(g_Plugin.hDisplayReady);

	if (!(g_Plugin.FlagsWork & FW_QUICK_VIEW))
		ExecuteInMainThread(FW_TITLEREPAINTD,FALSE,FALSE);

	WARNING("Но нужно проверить, чтобы все было корректно с заливкой, если основная нить еще не успела");
	// Лучше сразу залить окно цветом фона, чем блокироваться на INFINITE
	//WARNING("!!! от INFINITE нужно избавляться !!!"); // Если плагин падает, far показывает диалог с TrapLog,
	//// а ConEmu виснет, т.к. в нем подвисло окно PicView и он не может отрисоваться, т.к. все заблокировано
	//CFunctionLogger::FunctionLogger(L"DisplayThreadProc.WaitForSingleObject(g_Plugin.hWorkEvent, INFINITE)");
	//WaitForSingleObject(g_Plugin.hWorkEvent, INFINITE);
	//CFunctionLogger::FunctionLogger(L"DisplayThreadProc.WaitForSingleObject(g_Plugin.hWorkEvent, INFINITE) done");

	if (g_Plugin.FlagsWork & FW_TERMINATE)
		goto lExit;
	
	//Image еще не создан, и окно создается невидимым
	//{
	//	CImagePtr Image;
	//	if (!Image.IsValid())
	//		goto lExit;

	//	//if (!Image->dds->m_pWork || !DirectDrawSurface::SetHWnd(NULL, 2))
	//	if (!Image->Display ||
	//		!Image->mp_Draw->Display()->pPlugin ||
	//		!Image->mp_Draw->Display()->pPlugin->DisplayCheck())
	//		goto lExit;
	//}

	g_Plugin.FlagsDisplay |= FD_DISLPAYED;
	g_Plugin.bMouseHided = false;
	g_Plugin.bIgnoreMouseMove = false;
	g_Plugin.bDragging = false;
	g_Plugin.bScrolling = false;
	g_Plugin.bZoomming = false;
	g_Plugin.bCorrectMousePos = false;
	g_Plugin.bTrayOnTopDisabled = false;

	gbLockInputProcessing = false;

	if (g_Plugin.FlagsWork & FW_QUICK_VIEW)
	{
		WARNING("FlushConsoleInputBuffer может виснуть при вызове из нити Display");
		CFunctionLogger::FunctionLogger(L"DisplayThreadProc.FlushConsoleInputBuffer");
		FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE)/*g_Plugin.hInput*/);
		CFunctionLogger::FunctionLogger(L"DisplayThreadProc.SetParent(%s)", g_Plugin.hConEmuWnd ? L"ConEmuWnd" : L"FarWnd");
		SetParent(g_Plugin.hWnd, g_Plugin.hParentWnd = g_Plugin.hConEmuWnd ? g_Plugin.hConEmuWnd : g_Plugin.hFarWnd);
		CFunctionLogger::FunctionLogger(L"DisplayThreadProc.SetForegroundWindow");
		SetForegroundWindow(g_Plugin.hConEmuWnd ? g_Plugin.hConEmuWnd : g_Plugin.hFarWnd);
	}
	else
	{
		HideConsoleCursor();
		g_Plugin.bFullScreen = !g_Plugin.bFullScreen;
		OnFullScreen(g_Plugin.hWnd, WM_LBUTTONDBLCLK, 0, 0);
	}

	if (!InitImageDisplay(true))
		goto lExit;
	WndProc(g_Plugin.hWnd, WM_PAINT, 0, 0); // ибо _try
	//SetEvent(g_Plugin.hDisplay Event);

	//for (bool bWorking = true; bWorking && g_Panel.GetImageDraw() /*dds->m_pWork*/;)
	while (!(g_Plugin.FlagsWork & FW_TERMINATE))
	{
		UINT nEvents = 0;

		static HWND hLastForeground = NULL;
		if (!g_Plugin.hConEmuWnd) { // Только в чистом FAR, в ConEmu проблемы нет
			HWND h = GetForegroundWindow();
			if (h == g_Plugin.hFarWnd && hLastForeground != h) {
				CFunctionLogger fl2(L"Invalidating on FAR got focus was started");
				InvalidateRect(g_Plugin.hWnd, NULL, FALSE);
				WndProc(g_Plugin.hWnd, WM_PAINT, 0,0); // ибо _try
				gnRedrawConTimerStart = GetTickCount();
				SetTimer(g_Plugin.hWnd, TIMER_REDRAWCON, 500, 0);
			}
			hLastForeground = h;
		}

		if (g_Plugin.bFullScreen && !(g_Plugin.FlagsWork & FW_QUICK_VIEW))
		{
			if (const HWND hActiveWnd = GetForegroundWindow())
			{
				WINDOWPLACEMENT wplc = {sizeof(WINDOWPLACEMENT)};
				GetWindowPlacement(g_Plugin.hFarWnd, &wplc);
				if (wplc.showCmd == SW_SHOWMINIMIZED 
					|| (hActiveWnd != g_Plugin.hFarWnd && hActiveWnd != g_Plugin.hConEmuWnd && hActiveWnd != g_Plugin.hWnd))
				{
					if (g_Plugin.hParentWnd != g_Plugin.hFarWnd && g_Plugin.hParentWnd != g_Plugin.hConEmuWnd)
					{
						TrayRestore();
						CFunctionLogger::FunctionLogger(L"DisplayThreadProc.SetParent(%s)", g_Plugin.hConEmuWnd ? L"ConEmuWnd" : L"FarWnd");
						SetParent(g_Plugin.hWnd, g_Plugin.hParentWnd = g_Plugin.hConEmuWnd ? g_Plugin.hConEmuWnd : g_Plugin.hFarWnd);
						CFunctionLogger::FunctionLogger(L"SetParent done");
						if (g_Plugin.bMouseHided)
						{
							ShowCursor(TRUE);
							g_Plugin.bMouseHided = false;
						}
					}
				}
				else
				{
					if (g_Plugin.hParentWnd == g_Plugin.hFarWnd || g_Plugin.hParentWnd == g_Plugin.hConEmuWnd)
					{
						TrayDisable();
						CFunctionLogger::FunctionLogger(L"DisplayThreadProc.SetParent(DesktopWnd)");
						SetParent(g_Plugin.hWnd, g_Plugin.hParentWnd = g_Plugin.hDesktopWnd);
						CFunctionLogger::FunctionLogger(L"SetParent done");
						InvalidateRect(g_Plugin.hWnd, NULL, FALSE);
						if (!g_Plugin.bMouseHided)
						{
							ShowCursor(FALSE);
							g_Plugin.bMouseHided = true;
							g_Plugin.bIgnoreMouseMove = true;
						}
					}
				}
			}
		}

		const uint Time = GetTickCount();
		for (MSG Msg; PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE);)
		{
			if (Msg.message == WM_QUIT)
			{
				goto lExit;
			}
			DispatchMessage(&Msg);
			if (GetTickCount() - Time > 50)
			{
				// сразу пойдем на следующий цикл, без Sleep(10)
				nEvents = 2;
				break;
			}
		}

		//2001-04-30 на выход пойдем сразу? а то вроде зависания какие-то бывают
		if ((g_Plugin.FlagsWork & FW_TERMINATE))
			goto lExit;

		bool bRepaint = false;
		if (g_Plugin.bScrolling)
		{
			const bool bLeft  = GetAsyncKeyState(VK_LEFT)  < 0 || GetAsyncKeyState(VK_NUMPAD4) < 0;
			const bool bRight = GetAsyncKeyState(VK_RIGHT) < 0 || GetAsyncKeyState(VK_NUMPAD6) < 0;
			const bool bUp    = GetAsyncKeyState(VK_UP)    < 0 || GetAsyncKeyState(VK_NUMPAD8) < 0;
			const bool bDown  = GetAsyncKeyState(VK_DOWN)  < 0 || GetAsyncKeyState(VK_NUMPAD2) < 0;

			if (!bLeft && !bRight && !bUp && !bDown) {
				g_Plugin.bScrolling = false;
				// передернуть, чтобы картинка могла отобразиться с антиалиасингом
				InvalidateRect(g_Plugin.hWnd, NULL, FALSE);
			}
			else
			{
				static DWORD nLastPanTick = 0;
				DWORD nCurPanTick = timeGetTime();

				DWORD nTickStep = nCurPanTick - nLastPanTick;
				if (nTickStep > KEY_PAN_DELAY) nTickStep = KEY_PAN_DELAY;
				
				int nCurStep = nTickStep ? (g_Plugin.SmoothScrollingStep * nTickStep / KEY_PAN_DELAY) : 0;

				// Шаг плавного скролла - 20 пикселей (по умолчанию)
				// а дискретного - 6.25% окна отрисовки

				if (nCurStep > 1) {
					nLastPanTick = nCurPanTick;
				
					if (bRight ^ bLeft)
					{
						if (bRight)
							g_Plugin.ViewCenter.x -= nCurStep;
						else
							g_Plugin.ViewCenter.x += nCurStep;
						bRepaint = true;
					}
					if (bUp ^ bDown)
					{
						if (bDown)
							g_Plugin.ViewCenter.y -= nCurStep;
						else
							g_Plugin.ViewCenter.y += nCurStep;
						bRepaint = true;
					}
				}
			}
		}
		if (g_Plugin.bZoomming)
		{
			const bool bZoomIn  = GetAsyncKeyState(vkAdd)  < 0;
			const bool bZoomOut = GetAsyncKeyState(vkSubtract) < 0;

			if (!bZoomIn && !bZoomOut) {
				g_Plugin.bZoomming = false;
				// передернуть, чтобы картинка могла отобразиться с антиалиасингом
				InvalidateRect(g_Plugin.hWnd, NULL, FALSE);
			} else {
				// Если не обе кнопки (Gray+ и Gray-) нажаты
				if (bZoomIn ^ bZoomOut)
				{
					u32 NewZoom = Max<u32>(
							*ZoomLevel /*min 1%*/, 
							Min<u32>(
								ZoomLevel[sizeofarray(ZoomLevel) - 1] /*max 10000%*/, 
								bZoomIn ? MulDivU32R(g_Plugin.Zoom, 1000 + g_Plugin.SmoothZoomingStep, 1000) 
										: MulDivU32R(g_Plugin.Zoom, 1000, 1000 + g_Plugin.SmoothZoomingStep)));
										
					g_Plugin.ViewCenter.x = MulDivIU32R(g_Plugin.ViewCenter.x, NewZoom, g_Plugin.Zoom);
					g_Plugin.ViewCenter.y = MulDivIU32R(g_Plugin.ViewCenter.y, NewZoom, g_Plugin.Zoom);
					g_Plugin.Zoom = NewZoom;
					// g_Plugin.Zoom - считается относительно декодированных размеров, прикинем абсолютный зум
					CheckAbsoluteZoom();
					g_Plugin.ZoomAuto = ZA_NONE;
					g_Plugin.FlagsDisplay |= FD_TITLE_REPAINT;
					bRepaint = true;
				}
			}
		}
		if (bRepaint)
		{
			InvalidateRect(g_Plugin.hWnd, NULL, FALSE);
			WndProc(g_Plugin.hWnd, WM_PAINT, 0, 0); // ибо _try
			//OnPaint(g_Plugin.hWnd, WM_PAINT, 0, 0);
			continue;
		}
		// Если в буфере есть события (клавиатура/мышь) сразу пойдем на следующий цикл, без ожидания
		if (nEvents > 1)
			continue;
		Sleep(10);
	}

	if (!(g_Plugin.FlagsWork & FW_QUICK_VIEW))
		RestoreConsoleCursor();
	if (g_Plugin.bMouseHided)
		ShowCursor(TRUE);

lExit:
	KillTimer(g_Plugin.hWnd, TIMER_HIDECURSOR);
	KillTimer(g_Plugin.hWnd, TIMER_ANIMATION);
	KillTimer(g_Plugin.hWnd, TIMER_REDRAWCON);
	TrayRestore();

	RequestTerminate(2);
	g_Plugin.hWnd = NULL;

	////DirectDrawSurface::SetHWnd(NULL);
	//g_Plugin.FlagsWork |= FW_TERMINATE; // На всякий случай, если еще не выставили
	CPVDManager::DisplayExit(); // !!! Нада. Закрытие дескрипторов дисплея должно выполняться в этой нити !!!
	//if (g_Plugin.hWnd && IsWindow(g_Plugin.hWnd))
	//	DestroyWindow(g_Plugin.hWnd);

	// Очистка очереди, если там что-то осталось...
	// подстрахуемся от зависания (max 255 msgs)
	MSG Msg;
	for (int i = 255; i-->0 && PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE););

	//if (g_Plugin.hDisplayThread)
	//{
	//	SetEvent(g_Plugin.hDisplayEvent);
	//}
	return 0;
}

DWORD WINAPI DisplayThreadProc(void *pParameter)
{
	DWORD nRc = 0;

	__try {
		nRc = DisplayThreadProcSEH(pParameter);
	}__except( EXCEPTION_EXECUTE_HANDLER ){
		nRc = (DWORD)-1;
	}

	if (nRc == (DWORD)-1){
		CFunctionLogger::FunctionLogger(L"!!! Exception in DisplayThreadProcSEH");
		MessageBox(NULL, L"!!! Exception in DisplayThreadProcSEH\nPlease, restart FAR", L"PictureView2", MB_OK|MB_ICONSTOP|MB_SETFOREGROUND|MB_SYSTEMMODAL);
	}

	// Флаг того, что нить нормально завершилась
	gnDisplayThreadId = 0;

	return nRc;
}
