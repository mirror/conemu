
#pragma once

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


class CPVDManager;
//class DirectDrawSurface;
class CModuleInfo;
class CImage;

//#define MAX_PIC_PATH_LEN 0x1000

#define DEFAULT_PREFIX L"pic"

#include "PictureView_FileName.h"
#include "PVDInterface/PictureViewPlugin.h"

interface ITaskbarList2;

class CPluginData
{
public:
	CPluginData();
	~CPluginData();

	bool InitPlugin();
	bool InitHooks();
	bool InitCMYK(BOOL bForceLoad);
	
	bool IsExtensionIgnored(const wchar_t *pFileName);

	void SaveTitle();
	void RestoreTitle();

	bool bInitialized, bHookInitialized;

	bool bCMYKinitialized, bCMYKstarted;
	//HANDLE hCMYKthread; DWORD nCMYKthread; 
	HMODULE hGDIPlus;
	UINT nCMYK_ErrorNumber, nCMYK_LastError;
	DWORD nCMYKparts, *pCMYKpalette, nCMYKsize;

	// Палитры "по умолчанию"
	RGBQUAD pal1[2], pal1G[2], pal2[4], pal2G[4], pal4[16], pal4G[16], pal8[256], pal8G[256];
	void CreateDefaultPalettes();
	void CreateDefaultPalette8bpp(RGBQUAD* table);
	void CreateDefaultPalette8bppG(RGBQUAD* table);
	UINT32* GetPalette(UINT nBPP, pvdColorModel ColorModel);


	// окно отрисовки
	TODO("Заменить на класс CDisplayWnd, учесть что окна может быть ДВА - QView & обычное");
	HWND hWnd;
	// Родительские окна
	HWND hParentWnd, // текущее родительское окно для hWnd (это FAR, ConEmu, или Desktop)
		 hFarWnd, // консольное окно, или окно отрисовки в ConEmu
		 hConEmuWnd, // Главное окно conemu, или NULL в чистом FAR
		 hDesktopWnd; // no comment
	HANDLE hConEmuCtrlPressed, hConEmuShiftPressed;
	CONSOLE_CURSOR_INFO cci;
	//HANDLE hInput, hOutput;
	HANDLE hDisplayThread;
	u32 nBackColor, nQVBackColor;
	u32 BackColor();
	RECT ViewPanelT, ViewPanelG;

	//ITaskbarList2* pTaskBar;

	uint FarVersion;
	const wchar_t* pszPluginTitle; // = GetMsg(MIPluginName)

	// Сдвиг от центра изображения.
	// Это ЭКРАННЫЕ координаты. Сдвиг негативен. То есть отображение
	// правого нижнего угла соответсвует 
	// {-548,-462} для изображения 510x399 и Zoom = 371%
	// при размере области отображения 560x800
	POINT ViewCenter;

	POINT DragBase;
	bool bDragging;
	bool bScrolling;
	bool bZoomming;
	bool bCorrectMousePos;
	bool bMouseHided, bIgnoreMouseMove;
	bool bTrayOnTopDisabled;
	bool bFullScreen;
	u32 Zoom, AbsoluteZoom, ZoomAuto;
	bool ZoomAutoManual; //101129

	//wchar_t MapFileNamePrefix[4]; // \\?\ ...
	//CUnicodeFileName MapFileName;
	//wchar_t *GetMapFileName();

	uint FlagsDisplay;
	uint FlagsWork, FlagsWorkResult;
	bool SelectionChanged;

	// вроде так: Image[0] - текущий, [1] и [2] буферы кеширования
	// ImageX указатель на тот Image[jj], который сейчас декодируется
	//CImage *Image[3];
	//CImage *ImageX;

	//PanelInfo FarPanelInfo;
	//uint nPanelItems, nPanelFolders;

	bool bHookArc, bHookQuickView, bHookView, bHookEdit, bHookCtrlShiftF3, bHookCtrlShiftF4;
	bool bViewFolders; // разрешить просмотр папок как картинок
	WCHAR sHookPrefix[32];
	wchar_t sIgnoredExt[0x1000];
	bool bTrayDisable, bFullScreenStartup, bLoopJump, bFreePosition, bFullDisplayReInit, bMarkBySpace;
	bool bAutoPaging, bAutoPagingSet; DWORD nAutoPagingVK; BYTE bAutoPagingChanged;
	u32 uCMYK2RGB; // PVD_CMYK2RGB_*
	bool bCachingRP, bCachingVP;
	bool bAutoZoom, bAutoZoomMin, bAutoZoomMax, bKeepZoomAndPosBetweenFiles;
	u32 nKeepPanCorner;
	u32 AutoZoomMin, AutoZoomMax;
	bool bSmoothScrolling, bSmoothZooming;
	u32 SmoothScrollingStep, SmoothZoomingStep;
	u32 MouseZoomMode; // 0 - as keyboard; 1 - to screen center; 2 - hold position

	HANDLE hDisplayReady; // Выставляется в DisplayThread после успешного создания окна отображения
	HANDLE hDisplayEvent;
	HANDLE hWorkEvent;
	HANDLE hSynchroDone; // Выставляется в основной нити после выполнения запроса (help, config, ...)
	CRITICAL_SECTION csMainThread, csDecodeThread;
	bool bUncachedJump;

	bool bTitleSaved;
	wchar_t TitleSave[0x1000];

	ITaskbarList2* pTaskBar;
	
	wchar_t sLogFile[MAX_PATH+1];
};

extern CPluginData g_Plugin;
