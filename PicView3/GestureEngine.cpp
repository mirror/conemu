
/**************************************************************************
Copyright (c) 2012 Maximus5
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

#define SHOWDEBUGSTR

#ifdef _DEBUG
	#define USE_DUMPGEST
#else
	#undef USE_DUMPGEST
#endif
#define DEBUGSTRTOUCH(s) OutputDebugStringW(s)

#include "PictureView.h"
#include "GestureEngine.h"
#include "PictureView_Display.h"
#include "Panel.h"

#ifndef GID_PRESSANDTAP
    #define GID_PRESSANDTAP GID_ROLLOVER
#endif

#define LODWORD(ull) ((DWORD)((ULONGLONG)(ull) & 0x00000000ffffffff))
#define HIDWORD(ull) ((DWORD)(ull>>32))

#define MIN_PAN_START 40

CGestures* gp_Gestures = NULL;

#ifdef USE_DUMPGEST
	static wchar_t szDump[512];
#endif

// Default constructor of the class.
CGestures::CGestures()
:   _dwArguments(0), _inRotate(false), _PtCount(0)
{
	_isTabletPC = GetSystemMetrics(SM_TABLETPC);
	OSVERSIONINFO OSVer = {sizeof(OSVer)}; GetVersionEx(&OSVer);
	#define IsWindows7 ((OSVer.dwMajorVersion > 6) || (OSVer.dwMajorVersion == 6 && OSVer.dwMinorVersion > 0))
	_isGestures = IsWindows7;
	if (_isGestures)
	{
		HMODULE hUser = GetModuleHandle(L"user32.dll");
		_GetGestureInfo = (GetGestureInfo_t)GetProcAddress(hUser, "GetGestureInfo");
		_SetGestureConfig = (SetGestureConfig_t)GetProcAddress(hUser, "SetGestureConfig");
		_CloseGestureInfoHandle = (CloseGestureInfoHandle_t)GetProcAddress(hUser, "CloseGestureInfoHandle");
		if (!_GetGestureInfo || !_SetGestureConfig || !_CloseGestureInfoHandle)
			_isGestures = false;
	}
}

// Destructor
CGestures::~CGestures()
{
}

bool CGestures::IsGesturesEnabled()
{
	if (!_isTabletPC || !_isGestures)
		return false;
	// Финт ушами - считаем, что событие от мыши, если мышиный курсор
	// видим на экране. Если НЕ видим - то событие от тачскрина. Актуально
	// для того, чтобы различать правый клик от мышки и от тачскрина.
	CURSORINFO ci = {sizeof(ci)};
	if (!GetCursorInfo(&ci))
		return false;
	// 0 - курсор скрыт, а 2 - похоже недокументировано (тачскрин)
	if (ci.flags == 0 || ci.flags == 2)
		return true;
	_ASSERTE(ci.flags == CURSOR_SHOWING);
	return false;
}

// Проверить, в каких случаях разрешено листать одним пальцем (или только левой кнопкой мышки)
bool CGestures::IsSingleTapPaging()
{
	// Только если картинка полностью вписана, не нужен PAN
	if (g_Plugin.ZoomAuto != ZA_FIT)
		return false;
	if (!IsGesturesEnabled())
		return false;
	return true;
}

// Main function of this class decodes gesture information
// in:
//      hWnd        window handle
//      wParam      message parameter (message-specific)
//      lParam      message parameter (message-specific)
bool CGestures::ProcessGestureMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult)
{
#if 0
	// Не интересует, могут быть обработаны и обычные мышиные события
	if ((uMsg != WM_GESTURENOTIFY) && (uMsg != WM_GESTURE) && (uMsg != WM_TOUCH))
	{
		return false;
	}

	if (!_isGestures)
	{
		_ASSERTE(_isGestures);
		return false;
	}
#endif

	
	switch (uMsg)
	{
	#if 0
		// не приходит?
	case WM_TOUCH:
		{
			// Нас интересует количество точек, связанных с этим сообщением
			_PtCount = LOWORD(wParam);
			
			#ifdef USE_DUMPGEST
				TOUCHINPUT Points[4] = {};
				BOOL bInfo = GetTouchInputInfo((HTOUCHINPUT)lParam, ARRAYSIZE(Points), Points, sizeof(Points[0]));
				if (!bInfo)
				{
					wsprintfW(szDump, 
						L"Touch: Count=%u, GetTouchInputInfo failed, Code=%u \n",
						(UINT)LOWORD(wParam), GetLastError());
				}
				else
				{
					wsprintfW(szDump, 
						L"Touch: Count=%u,", (UINT)LOWORD(wParam));
					if (LOWORD(wParam) == 0)
						lstrcatW(szDump, L" No info");
					else
					{
						for (UINT i = 0; i < LOWORD(wParam); i++)
						{
							wsprintfW(szDump+lstrlenW(szDump), 
								L" {%ix%i ID=%u Flags=x%04X Mask=x%04X [%ix%i]}",
								Points[i].x, Points[i].y, Points[i].dwID, Points[i].dwFlags, Points[i].dwMask,
								Points[i].cxContact, Points[i].cyContact
								);
						}
					}
					lstrcatW(szDump, L"\n");
				}
				DEBUGSTRTOUCH(szDump);
			#endif
			
			// DefWindowProc will close and invalidate touch handle
			lResult = ::DefWindowProc(hWnd, uMsg, wParam, lParam);
		}
		break;
	#endif
	
	case WM_LBUTTONDOWN:
		{
			if (!IsSingleTapPaging())
			{
				_PtCount = 0;
				return false;
			}
			_PtCount = 1;
			GetCursorPos(&_ptFirst);
			ScreenToClient(hWnd, &_ptFirst);

			#ifdef USE_DUMPGEST
			wsprintfW(szDump,
				L"LBtnDown(x%08X {%i,%i}\n",
				(DWORD)hWnd, _ptFirst.x, _ptFirst.y);
			DEBUGSTRTOUCH(szDump);
			#endif
		}
		break;
	case WM_MOUSEMOVE:
	case WM_LBUTTONUP:
		{
			if (_PtCount != 1)
				return false;
			GetCursorPos(&_ptSecond);
			ScreenToClient(hWnd, &_ptSecond);

			#ifdef USE_DUMPGEST
			if (uMsg == WM_LBUTTONUP)
			{
				wsprintfW(szDump,
					L"LBtnUp(x%08X {%i,%i} dx={%i,%i}\n",
					(DWORD)hWnd, _ptFirst.x, _ptFirst.y, _ptSecond.x-_ptFirst.x, _ptSecond.y-_ptFirst.y);
				DEBUGSTRTOUCH(szDump);
			}
			#endif

			if (ProcessMove(hWnd, _ptSecond.x-_ptFirst.x, _ptSecond.y-_ptFirst.y, (uMsg == WM_LBUTTONUP)))
			{
				// We have to copy second point into first one to prepare
				// for the next step of this gesture.
				_ptFirst = _ptSecond;
			}

			if (uMsg == WM_LBUTTONUP)
				_PtCount = 0;
		}
		break;

	case WM_GESTURENOTIFY:
		{
			#ifdef USE_DUMPGEST
				PGESTURENOTIFYSTRUCT p = (PGESTURENOTIFYSTRUCT)lParam;
				wsprintfW(szDump,
					L"GestureNotify(x%08X {%i,%i} ID=%u Flags=x%08X \n",
					(DWORD)p->hwndTarget, p->ptsLocation.x, p->ptsLocation.y, p->dwInstanceID, p->dwFlags);
				DEBUGSTRTOUCH(szDump);
			#endif
		
	        // This is the right place to define the list of gestures that this
	        // application will support. By populating GESTURECONFIG structure 
	        // and calling SetGestureConfig function. We can choose gestures 
	        // that we want to handle in our application. In this app we
	        // decide to handle all gestures.
	        GESTURECONFIG gc[] = {
				{GID_ZOOM, GC_ZOOM},
				{GID_ROTATE, GC_ROTATE},
				{GID_PAN,
					GC_PAN|GC_PAN_WITH_GUTTER|GC_PAN_WITH_INERTIA
					,GC_PAN_WITH_SINGLE_FINGER_VERTICALLY|GC_PAN_WITH_SINGLE_FINGER_HORIZONTALLY
				},
				{GID_PRESSANDTAP, GC_PRESSANDTAP},
				{GID_TWOFINGERTAP, GC_TWOFINGERTAP},
	        };

			DWORD dwErr = 0;
	        BOOL bResult = _SetGestureConfig(hWnd, 0, ARRAYSIZE(gc), gc, sizeof(GESTURECONFIG));                        
	        
	        if (!bResult)
	        {
				dwErr = GetLastError();
	            _ASSERTE("Error in execution of SetGestureConfig" && 0);
	        }

			lResult = ::DefWindowProc(hWnd, WM_GESTURENOTIFY, wParam, lParam);

		} // case WM_GESTURENOTIFY:
		break;

	case WM_GESTURE:
		{
		    // helper variables
		    POINT ptZoomCenter;
		    double k;

		    GESTUREINFO gi = {sizeof(gi)};
		    BOOL bResult = _GetGestureInfo((HGESTUREINFO)lParam, &gi);

		    if (!bResult)
		    {
		        _ASSERT(L"_GetGestureInfo failed!" && 0);
		        return false;
		    }

			#ifdef USE_DUMPGEST
				#define DUMPGEST(tp) \
					wsprintfW(szDump, \
						L"Gesture(x%08X {%i,%i} %s", \
						(DWORD)gi.hwndTarget, gi.ptsLocation.x, gi.ptsLocation.y, \
						tp); \
					if (gi.dwID==GID_PRESSANDTAP) { \
						DWORD h = LODWORD(gi.ullArguments); wsprintfW(szDump+_tcslen(szDump), \
							L" Dist={%i,%i}", (int)(short)LOWORD(h), (int)(short)HIWORD(h)); } \
					if (gi.dwID==GID_ROTATE) { \
						DWORD h = LODWORD(gi.ullArguments); wsprintfW(szDump+_tcslen(szDump), \
							L" %i", (int)LOWORD(h)); } \
					if (gi.dwFlags&GF_BEGIN) lstrcatW(szDump, L" GF_BEGIN"); \
					if (gi.dwFlags&GF_END) lstrcatW(szDump, L" GF_END"); \
					if (gi.dwFlags&GF_INERTIA) { lstrcatW(szDump, L" GF_INERTIA"); \
						DWORD h = HIDWORD(gi.ullArguments); wsprintfW(szDump+_tcslen(szDump), \
							L" {%i,%i}", (int)(short)LOWORD(h), (int)(short)HIWORD(h)); } \
					lstrcatW(szDump, L")\n"); \
					DEBUGSTRTOUCH(szDump)
			#else
				#define DUMPGEST(s)
			#endif

		    switch (gi.dwID)
		    {
		    case GID_BEGIN:
				DUMPGEST(L"GID_BEGIN");
		        break;

		    case GID_END:
				DUMPGEST(L"GID_END");
		        break;
		    
		    case GID_ZOOM:
				DUMPGEST(L"GID_ZOOM");
		        if (gi.dwFlags & GF_BEGIN)
		        {
		            _dwArguments = LODWORD(gi.ullArguments);
		            _ptFirst.x = gi.ptsLocation.x;
		            _ptFirst.y = gi.ptsLocation.y;
		            ScreenToClient(hWnd,&_ptFirst);
				}
				else
				{
		            // We read here the second point of the gesture. This is middle point between 
		            // fingers in this new position.
		            _ptSecond.x = gi.ptsLocation.x;
		            _ptSecond.y = gi.ptsLocation.y;
		            ScreenToClient(hWnd,&_ptSecond);

		            // We have to calculate zoom center point 
		            ptZoomCenter.x = (_ptFirst.x + _ptSecond.x)/2;
		            ptZoomCenter.y = (_ptFirst.y + _ptSecond.y)/2;           
		            
		            // The zoom factor is the ratio between the new and the old distance. 
		            // The new distance between two fingers is stored in gi.ullArguments 
		            // (lower DWORD) and the old distance is stored in _dwArguments.
		            k = (double)(LODWORD(gi.ullArguments))/(double)(_dwArguments);

		            // Now we process zooming in/out of the object
		            ProcessZoom(hWnd, k, ptZoomCenter.x, ptZoomCenter.y);

		            // Now we have to store new information as a starting information 
		            // for the next step in this gesture.
		            _ptFirst = _ptSecond;
		            _dwArguments = LODWORD(gi.ullArguments);
		        }
		        break;
		    
		    case GID_PAN:
				DUMPGEST(L"GID_PAN");
		        if (gi.dwFlags & GF_BEGIN)
		        {
		            _ptFirst.x = gi.ptsLocation.x;
		            _ptFirst.y = gi.ptsLocation.y;
					_ptBegin.x = gi.ptsLocation.x;
					_ptBegin.y = gi.ptsLocation.y;
		            ScreenToClient(hWnd, &_ptFirst);
				}
				else
				{
		            // We read the second point of this gesture. It is a middle point
		            // between fingers in this new position
		            _ptSecond.x = gi.ptsLocation.x;
		            _ptSecond.y = gi.ptsLocation.y;
		            ScreenToClient(hWnd, &_ptSecond);

					//if (!(gi.dwFlags & (GF_END/*|GF_INERTIA*/)))
					{
			            // We apply move operation of the object
				        if (ProcessMove(hWnd, _ptSecond.x-_ptFirst.x, _ptSecond.y-_ptFirst.y, ((gi.dwFlags & GF_END) == GF_END)))
						{
							// We have to copy second point into first one to prepare
							// for the next step of this gesture.
							_ptFirst = _ptSecond;
						}
					}

		        }
		        break;

		    case GID_ROTATE:
				DUMPGEST(L"GID_ROTATE");
		        if (gi.dwFlags & GF_BEGIN)
		        {
					_inRotate = false;
		            _dwArguments = LODWORD(gi.ullArguments); // Запомним начальный угол
				}
				else
				{
		            _ptFirst.x = gi.ptsLocation.x;
		            _ptFirst.y = gi.ptsLocation.y;
		            ScreenToClient(hWnd, &_ptFirst);
					// Пока угол не станет достаточным для смены таба - игнорируем
		            if (ProcessRotate(hWnd, 
							LODWORD(gi.ullArguments) - _dwArguments,
							_ptFirst.x,_ptFirst.y, ((gi.dwFlags & GF_END) == GF_END)))
					{
						_dwArguments = LODWORD(gi.ullArguments);
					}
		        }
		        break;

		    case GID_TWOFINGERTAP:
				DUMPGEST(L"GID_TWOFINGERTAP");
		        _ptFirst.x = gi.ptsLocation.x;
		        _ptFirst.y = gi.ptsLocation.y;
				SetCursorPos(_ptFirst.x, _ptFirst.y);
		        ScreenToClient(hWnd,&_ptFirst);
		        ProcessTwoFingerTap(hWnd, _ptFirst.x, _ptFirst.y, LODWORD(gi.ullArguments));
		        break;

		    case GID_PRESSANDTAP:
				DUMPGEST(L"GID_PRESSANDTAP");
		        if (gi.dwFlags & GF_BEGIN)
		        {
					_ptFirst.x = gi.ptsLocation.x;
					_ptFirst.y = gi.ptsLocation.y;
					ScreenToClient(hWnd,&_ptFirst);
					DWORD nDelta = LODWORD(gi.ullArguments);
					short nDeltaX = (short)LOWORD(nDelta);
					short nDeltaY = (short)HIWORD(nDelta);
		            ProcessPressAndTap(hWnd, _ptFirst.x, _ptFirst.y, nDeltaX, nDeltaY);
		        }
		        break;
			default:
				DUMPGEST(L"GID_<UNKNOWN>");
		    }

		    _CloseGestureInfoHandle((HGESTUREINFO)lParam);
		    
		} // case WM_GESTURE:
		break;

	default:
		return false;
	}
    return true;
}

// This function is called when press and tap gesture is recognized
void CGestures::ProcessPressAndTap(HWND hWnd, const LONG ldx, const LONG ldy, const short nDeltaX, const short nDeltaY)
{
	ExecuteInMainThread(FW_TOGGLEMARK_FILE, FALSE/*bRestoreCursor*/, FALSE);
	return;
}

// This function is invoked when two finger tap gesture is recognized
// ldx/ldy - Indicates the center of the two fingers.
void CGestures::ProcessTwoFingerTap(HWND hWnd, const LONG ldx, const LONG ldy, const ULONG dist)
{
	// Аналог Gray/
	// Переключение: автомасштабирование / масштаб 100%
	OnAutoZoom(hWnd, true);
	return;
}

// This function is called constantly through duration of zoom in/out gesture
void CGestures::ProcessZoom(HWND hWnd, const double dZoomFactor, const LONG lZx, const LONG lZy)
{
	if (dZoomFactor > 0)
	{
	}
	return;
}

// This function is called throughout the duration of the panning/inertia gesture
bool CGestures::ProcessMove(HWND hWnd, const LONG ldx, const LONG ldy, bool bEnd)
{
	bool lbSent = false;
	LONG ux = Abs(ldx);
	LONG uy = Abs(ldy);

#if 0
	// Сюда не придет то, что не нужно
	if (_PtCount == 1)
	{
		if (g_Plugin.ZoomAuto != ZA_FIT)
		{
			// Возможно, здесь стоит делать именно Pan картинки, вместо листания
		}
	}
#endif
	
	if ((ux < MIN_PAN_START) && (uy < MIN_PAN_START))
	{
		if (bEnd)
		{
			#ifdef USE_DUMPGEST
			wsprintf(szDump, L"Pan range too small {%ix%i), ignoring\n", ldx, ldy);
			DEBUGSTRTOUCH(szDump);
			#endif
		}
	}
	else
	{
		if (!bEnd)
		{
			// Самое время запросить следующую картинку/страницу
			
		}
		// Пальцы отпустили
		else //if (bEnd)
		{
			// (wParam & 0x80000000) - Next, иначе - Prev
			if ((ux > uy) && (ux > MIN_PAN_START))
			{
				// Прокрутка по горизонтали - листаем файлы
				OnNextFile(hWnd, 0, (ldx < 0) ? 0x80000000 : 0, 0);
			}
			else if (uy > MIN_PAN_START)
			{
				// Прокрутка по вертикали - листаем страницы
				OnNextPage(hWnd, 0, (ldy < 0) ? 0x80000000 : 0, 0);
			}
			
			lbSent = true; // Запомнить обработанную координату
		}
	}

	//lbSent = true; // Запомнить обработанную координату?
	return lbSent;
}

// This function is called throughout the duration of the rotation gesture
bool CGestures::ProcessRotate(HWND hWnd, const LONG lAngle, const LONG lOx, const LONG lOy, bool bEnd)
{
	bool bProcessed = false;

	if ((((lAngle<0)?-lAngle:lAngle) / 2048) >= 1)
	{
		if (!_inRotate)
		{
			//starting
			_inRotate = true;
		}


		bProcessed = true;
	}

	if (bEnd /*&& _inRotate*/)
	{
		_inRotate = false;
	}

	return bProcessed;
}
