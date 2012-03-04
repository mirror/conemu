
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


#include "PictureView.h"
#include "PictureView_Lang.h"
#include "PVDManager.h"
#include "PVDInterface/BltHelper.h"
#include <ShObjIdl.h>
#include "headers/farcolor.hpp"
#include "RefKeeper.h"
#include "GestureEngine.h"

CPluginData g_Plugin;

CPluginData::CPluginData()
{
#ifdef MHEAP_DEFINED
	xf_initialize();
#endif

	bInitialized = false;
	sLogFile[0] = 0;
	
	pszPluginTitle = L"Picture View 3";

	bCMYKinitialized = bCMYKstarted = false;
	//hCMYKthread = 
	hGDIPlus = NULL; nCMYK_ErrorNumber = nCMYK_LastError = 0;
	nCMYKparts = 0; pCMYKpalette = 0; nCMYKsize = 0;

	pTaskBar = NULL;

	hWnd = hParentWnd = hFarWnd = hConEmuWnd = hDesktopWnd = NULL;
	hConEmuCtrlPressed = hConEmuShiftPressed = NULL;
	ZeroMemory(&cci,sizeof(cci));
	//hInput = hOutput = 
	//hThread = NULL;
	nBackColor = nQVBackColor = 0;
	ZeroMemory(&ViewPanelT, sizeof(ViewPanelT));
	ZeroMemory(&ViewPanelG, sizeof(ViewPanelG));

	FarVersion = 0;

	ZeroMemory(&ViewCenter, sizeof(ViewCenter));
	ZeroMemory(&DragBase, sizeof(DragBase));
	bDragging = bScrolling = bZoomming = bCorrectMousePos = bMouseHided = bIgnoreMouseMove = bTrayOnTopDisabled = bFullScreen = false;
	Zoom = AbsoluteZoom = ZoomAuto = 0;

	//MapFileNamePrefix[0] = MapFileNameData[0] = 0;

	FlagsDisplay = FlagsWork = FlagsWorkResult = 0;
	SelectionChanged = false;

	// вроде так: Image[0] - текущий, [1] и [2] буферы кеширования
	// ImageX указатель на тот Image[jj], который сейчас декодируется
	//ZeroMemory(Image,sizeof(Image)); ImageX = NULL;

	//ZeroMemory(&FarPanelInfo, sizeof(FarPanelInfo));
	//nPanelItems = nPanelFolders = 0;

	bHookArc = bHookQuickView = bHookView = bHookEdit = false;
	bViewFolders = false;
	lstrcpy(sHookPrefix, DEFAULT_PREFIX);
	sIgnoredExt[0] = 0;
	bTrayDisable = bFullScreenStartup = bLoopJump = bFreePosition = bFullDisplayReInit = bMarkBySpace = false;
	bAutoPaging = true; bAutoPagingSet = false; nAutoPagingVK = VK_SCROLL; bAutoPagingChanged = 0;
	uCMYK2RGB = PVD_CMYK2RGB_PRECISE;
	bCachingRP = bCachingVP = bAutoZoom = bAutoZoomMin = bAutoZoomMax = false;
	AutoZoomMin = AutoZoomMax = 0; bKeepZoomAndPosBetweenFiles = true; nKeepPanCorner = 0;
	bSmoothScrolling = bSmoothZooming = false;
	SmoothScrollingStep = SmoothZoomingStep = MouseZoomMode = 0; // 0 - as keyboard; 1 - to screen center; 2 - hold position

	hDisplayReady = hDisplayEvent = hWorkEvent = hSynchroDone = NULL;
	bUncachedJump = false;

	InitializeCriticalSection(&g_Plugin.csMainThread);
	InitializeCriticalSection(&g_Plugin.csDecodeThread);

	bTitleSaved = false;
	TitleSave[0] = 0;
}

CPluginData::~CPluginData()
{
	if (pCMYKpalette) {
		free(pCMYKpalette); pCMYKpalette = NULL;
	}
	if (pTaskBar) {
		pTaskBar->Release(); pTaskBar = NULL;
	}
	//if (hCMYKthread) {
	//	if (!WaitForSingleObject(hCMYKthread,100)) {
	//		TerminateThread(hCMYKthread,100);
	//	}
	//	CloseHandle(hCMYKthread); hCMYKthread = NULL;
	//}
	if (hGDIPlus) {
		FreeLibrary(hGDIPlus);
		hGDIPlus = NULL;
		CoUninitialize();
	}
	if (hConEmuCtrlPressed) { CloseHandle(hConEmuCtrlPressed); hConEmuCtrlPressed = NULL; }
	if (hConEmuShiftPressed) { CloseHandle(hConEmuShiftPressed); hConEmuShiftPressed = NULL; }

	DeleteCriticalSection(&g_Plugin.csMainThread);
	DeleteCriticalSection(&g_Plugin.csDecodeThread);
}

u32 CPluginData::BackColor()
{
	if (FlagsWork & FW_QUICK_VIEW)
		return nQVBackColor;
	else
		return nBackColor;
}

void CPluginData::SaveTitle()
{
	if (bTitleSaved)
		return;
	bTitleSaved = TRUE;
	MCHKHEAP;
	GetConsoleTitleW(TitleSave, sizeofarray(TitleSave));
	MCHKHEAP;
}

void CPluginData::RestoreTitle()
{
	MCHKHEAP;

	if (bTitleSaved)
	{
		SetConsoleTitleW(TitleSave);
		bTitleSaved = false;
	}
	
	if (g_Plugin.bAutoPagingChanged)
	{
		SHORT CurState = GetKeyState(g_Plugin.nAutoPagingVK);
		BOOL bChanged = g_Plugin.bAutoPagingChanged != (((CurState & 1) == 0) ? 1 : 2);
		if (bChanged) {
			keybd_event((BYTE)g_Plugin.nAutoPagingVK, (BYTE)MapVirtualKey(g_Plugin.nAutoPagingVK, 0), KEYEVENTF_EXTENDEDKEY, 0);
			keybd_event((BYTE)g_Plugin.nAutoPagingVK, (BYTE)MapVirtualKey(g_Plugin.nAutoPagingVK, 0), KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
		}
		g_Plugin.bAutoPagingChanged = 0;
	}

	MCHKHEAP;
}

//wchar_t* CPluginData::GetMapFileName()
//{
//	if (!MapFileNameData[0]) return MapFileNameData;
//	MapFileNamePrefix[0] = L'\\'; MapFileNamePrefix[1] = L'\\';
//	MapFileNamePrefix[2] = L'?';  MapFileNamePrefix[3] = L'\\';
//	return MapFileNamePrefix;
//}

bool CPluginData::InitHooks()
{
	if (bHookInitialized)
		return true;
	bHookInitialized = true;

	MCHKHEAP;

	// gnMainThreadId инициализируется в DllMain
	_ASSERTE(gnMainThreadId!=0); // плагин может активироваться НЕ в главной нити
	//gnMainThreadId = GetCurrentThreadId();

	#ifndef FAR_UNICODE
	g_Plugin.FarVersion = (DWORD)g_StartupInfo.AdvControl(PluginNumber, ACTL_GETFARVERSION, NULL);
	#endif
	g_Plugin.hDesktopWnd = GetDesktopWindow();

	//2009-10-04 динамическая длина
	g_SelfPath = _wcsdup(g_StartupInfo.ModuleName);
	wchar_t *p = wcsrchr(g_SelfPath, '\\');
	_ASSERTE(p);
	if (p) p[1] = 0; else g_SelfPath[0] = 0;

	// Сформируем наш ключ реестра
	//_ASSERTE(g_StartupInfo.RootKey && *g_StartupInfo.RootKey);
	WARNING("Переделать на Far3 API");
	g_RootKey = ConcatPath(L"Software\\Far Manager\\Plugins", L"PicView3");
		

	//ZeroMemory(&g_Plugin, sizeof(g_Plugin));

	bHookArc = true;
	bHookQuickView = true;
	bHookView = false; bHookCtrlShiftF3 = false;
	bHookEdit = false; bHookCtrlShiftF4 = false;
	bViewFolders = false;
	lstrcpy(sHookPrefix, DEFAULT_PREFIX);
	lstrcpy(sIgnoredExt, DEFAULT_INGORED_EXT);

	HKEY RegKey;
	if (!RegOpenKeyExW(HKEY_CURRENT_USER, g_RootKey, 0, KEY_READ, &RegKey))
	{
		DWORD len;
		
		RegKeyRead(RegKey, L"HookArc", &g_Plugin.bHookArc, true);
		RegKeyRead(RegKey, L"HookQuickView", &g_Plugin.bHookQuickView, true);
		RegKeyRead(RegKey, L"HookView", &g_Plugin.bHookView, false);
		RegKeyRead(RegKey, L"HookCtrlShiftF3", &g_Plugin.bHookCtrlShiftF3, false);
		RegKeyRead(RegKey, L"HookEdit", &g_Plugin.bHookEdit, false);
		RegKeyRead(RegKey, L"HookCtrlShiftF4", &g_Plugin.bHookCtrlShiftF4, false);
		RegKeyRead(RegKey, L"ViewFolders", &g_Plugin.bViewFolders, false);
		if (RegQueryValueExW(RegKey, L"Prefix", NULL, NULL, (LPBYTE)sHookPrefix, &(len = sizeof(sHookPrefix))) || !sHookPrefix[0])
			lstrcpy(sHookPrefix, DEFAULT_PREFIX);
		RegQueryValueExW(RegKey, L"IgnoredExtList", NULL, NULL, (LPBYTE)g_Plugin.sIgnoredExt, &(len = sizeof(g_Plugin.sIgnoredExt)));
		
		if (RegQueryValueExW(RegKey, L"LogFileName", NULL, NULL, (LPBYTE)sLogFile, &(len = sizeof(sLogFile))))
			sLogFile[0] = 0;
		
		RegCloseKey(RegKey);
	}

	MCHKHEAP;
	
	return true;	
}

bool CPluginData::InitPlugin()
{
	if (hConEmuCtrlPressed) { CloseHandle(hConEmuCtrlPressed); hConEmuCtrlPressed = NULL; }
	if (hConEmuShiftPressed) { CloseHandle(hConEmuShiftPressed); hConEmuShiftPressed = NULL; }
	
	if (bInitialized)
		return true;
	bInitialized = true;
	
	pszPluginTitle = GetMsg(MIPluginName);
	
	if (gnMainThreadId == 0)
	{
		_ASSERTE(gnMainThreadId!=0);
		gnMainThreadId = GetMainThreadId();
	}

	MCHKHEAP;

	if (!gp_RefKeeper)
	{
		gp_RefKeeper = new CRefKeeper();
	}
	
	if (!gp_Gestures)
	{
		gp_Gestures = new CGestures;
	}
	
	// Минимальная инициализация - только тип активации плагина
	InitHooks();

	// Палитры по умолчанию (если декодер не вернул палитру для индексированного цвета)
	CreateDefaultPalettes();

	g_Plugin.bCachingRP = true;
	g_Plugin.bCachingVP = false;
	g_Plugin.bAutoZoom = true;
	g_Plugin.bTrayDisable = true;
	g_Plugin.bFullScreenStartup = false;
	g_Plugin.bLoopJump = false;
	g_Plugin.bMarkBySpace = false;
	g_Plugin.bFreePosition = false;
	g_Plugin.bFullDisplayReInit = false;
	g_Plugin.bAutoPaging = true;
	g_Plugin.nAutoPagingVK = VK_SCROLL;
	g_Plugin.bAutoPagingSet = false;
	g_Plugin.uCMYK2RGB = PVD_CMYK2RGB_PRECISE;
	g_Plugin.bAutoZoomMin = false;
	g_Plugin.AutoZoomMin = 0x10000;
	g_Plugin.bAutoZoomMax = false;
	g_Plugin.bKeepZoomAndPosBetweenFiles = true;
	g_Plugin.nKeepPanCorner = 0;
	g_Plugin.AutoZoomMax = 0x10000;
	g_Plugin.bSmoothScrolling = true;
	g_Plugin.SmoothScrollingStep = 20;
	g_Plugin.bSmoothZooming = true;
	g_Plugin.SmoothZoomingStep = 33;
	g_Plugin.MouseZoomMode = 2;
	g_Plugin.nBackColor = 0;
	DWORD nDefConColors[] = 	{ // Default color scheme (Windows standard)
		0x00000000, 0x00800000, 0x00008000, 0x00808000, 0x00000080, 0x00800080, 0x00008080, 0x00c0c0c0, 
		0x00808080, 0x00ff0000, 0x0000ff00, 0x00ffff00, 0x000000ff, 0x00ff00ff, 0x0000ffff, 0x00ffffff};
	#ifdef FAR_UNICODE
		FarColor clr = {};
		g_StartupInfo.AdvControl(PluginNumber, ACTL_GETCOLOR, COL_PANELTEXT, &clr);
		if (clr.Flags & FCF_BG_4BIT)
		{
			_ASSERTE(clr.BackgroundColor<=15);
			g_Plugin.nQVBackColor = nDefConColors[clr.BackgroundColor & 0xF];
		}
		else
		{
			g_Plugin.nQVBackColor = clr.BackgroundColor;
		}
	#else
		UINT PanelTextColor = (UINT)g_StartupInfo.AdvControl(PluginNumber, ACTL_GETCOLOR, (void*)COL_PANELTEXT);
		PanelTextColor = (PanelTextColor & 0xF0) >> 4;
		g_Plugin.nQVBackColor = nDefConColors[PanelTextColor];
	#endif
	lstrcpy(g_TitleTemplate, g_DefaultTitleTemplate);
	lstrcpy(g_QViewTemplate1, g_DefaultQViewTemplate1);
	lstrcpy(g_QViewTemplate2, g_DefaultQViewTemplate2);
	lstrcpy(g_QViewTemplate3, g_DefaultQViewTemplate3);

	HKEY RegKey;
	if (!RegOpenKeyExW(HKEY_CURRENT_USER, g_RootKey, 0, KEY_READ, &RegKey))
	{
		DWORD len;
		
		//RegKeyRead(RegKey, L"HookArc", &g_Plugin.bHookArc, true);
		//RegKeyRead(RegKey, L"HookQuickView", &g_Plugin.bHookQuickView, true);
		//RegKeyRead(RegKey, L"HookView", &g_Plugin.bHookView, false);
		//RegKeyRead(RegKey, L"HookEdit", &g_Plugin.bHookEdit, false);
		//RegQueryValueExW(RegKey, L"IgnoredExtList", NULL, NULL, (LPBYTE)g_Plugin.sIgnoredExt, &(len = sizeof(g_Plugin.sIgnoredExt)));
		RegKeyRead(RegKey, L"AutoCachingRP", &g_Plugin.bCachingRP, true);
		RegKeyRead(RegKey, L"AutoCachingVP", &g_Plugin.bCachingVP, false);
		RegKeyRead(RegKey, L"AutoZoom", &g_Plugin.bAutoZoom, true);
		RegKeyRead(RegKey, L"TrayDisable", &g_Plugin.bTrayDisable, true);
		RegKeyRead(RegKey, L"FullScreenStartup", &g_Plugin.bFullScreenStartup, false);
		RegKeyRead(RegKey, L"LoopJump", &g_Plugin.bLoopJump, false);
		RegKeyRead(RegKey, L"MarkBySpace", &g_Plugin.bMarkBySpace, false);
		RegKeyRead(RegKey, L"FreePosition", &g_Plugin.bFreePosition, false);
		RegKeyRead(RegKey, L"FullDirectDrawInit", &g_Plugin.bFullDisplayReInit, false);
		RegKeyRead(RegKey, L"AutoPaging", &g_Plugin.bAutoPaging, true);
		RegKeyRead(RegKey, L"AutoPagingKey", &g_Plugin.nAutoPagingVK, VK_SCROLL);
		RegKeyRead(RegKey, L"AutoPagingSet", &g_Plugin.bAutoPagingSet, false);
		RegKeyRead(RegKey, L"CMYK2RGB", &g_Plugin.uCMYK2RGB, PVD_CMYK2RGB_PRECISE);
		RegKeyRead(RegKey, L"AutoZoomMinFlag", &g_Plugin.bAutoZoomMin, false);
		RegKeyRead(RegKey, L"AutoZoomMin", &g_Plugin.AutoZoomMin, 0x10000);
		RegKeyRead(RegKey, L"AutoZoomMaxFlag", &g_Plugin.bAutoZoomMax, false);
		RegKeyRead(RegKey, L"KeepZoomBetweenFiles", &g_Plugin.bKeepZoomAndPosBetweenFiles, true);
		RegKeyRead(RegKey, L"KeepPanCorner", &g_Plugin.nKeepPanCorner, 0);
		RegKeyRead(RegKey, L"AutoZoomMax", &g_Plugin.AutoZoomMax, 0x10000);
		RegKeyRead(RegKey, L"SmoothScrolling", &g_Plugin.bSmoothScrolling, true);
		RegKeyRead(RegKey, L"SmoothScrollingStep", &g_Plugin.SmoothScrollingStep, 20);
		RegKeyRead(RegKey, L"SmoothZooming", &g_Plugin.bSmoothZooming, true);
		RegKeyRead(RegKey, L"SmoothZoomingStep", &g_Plugin.SmoothZoomingStep, 33);
		RegKeyRead(RegKey, L"MouseZoomMode", &g_Plugin.MouseZoomMode, 2);
		RegKeyRead(RegKey, L"BackgroundColor", &g_Plugin.nBackColor, g_Plugin.nBackColor);
		RegKeyRead(RegKey, L"BackgroundQVColor", &g_Plugin.nQVBackColor, g_Plugin.nQVBackColor);
		_ASSERTE(g_Plugin.nQVBackColor > 0xFFFF);
		{
			len = sizeof(g_TitleTemplate);
			if (!RegQueryValueExW(RegKey, L"TitleTemplate", NULL, NULL, (LPBYTE)g_TitleTemplate, (LPDWORD)&len) && len && *g_TitleTemplate)
				g_TitleTemplate[len] = 0;
			else
				lstrcpy(g_TitleTemplate, g_DefaultTitleTemplate);

			len = sizeof(g_QViewTemplate1);
			if (!RegQueryValueExW(RegKey, L"QViewTemplate1", NULL, NULL, (LPBYTE)g_QViewTemplate1, (LPDWORD)&len) 
				&& len && *g_QViewTemplate1)
				g_QViewTemplate1[len] = 0;
			else
				lstrcpy(g_QViewTemplate1, g_DefaultQViewTemplate1);

			len = sizeof(g_QViewTemplate2);
			if (!RegQueryValueExW(RegKey, L"QViewTemplate2", NULL, NULL, (LPBYTE)g_QViewTemplate2, (LPDWORD)&len) 
				&& len && *g_QViewTemplate2)
				g_QViewTemplate2[len] = 0;
			else
				lstrcpy(g_QViewTemplate2, g_DefaultQViewTemplate2);

			len = sizeof(g_QViewTemplate3);
			if (!RegQueryValueExW(RegKey, L"QViewTemplate3", NULL, NULL, (LPBYTE)g_QViewTemplate3, (LPDWORD)&len) 
				&& len && *g_QViewTemplate3)
				g_QViewTemplate3[len] = 0;
			else
				lstrcpy(g_QViewTemplate3, g_DefaultQViewTemplate3);
		}
		if (g_Plugin.nAutoPagingVK!=VK_SCROLL && g_Plugin.nAutoPagingVK!=VK_CAPITAL && g_Plugin.nAutoPagingVK!=VK_NUMLOCK)
			g_Plugin.nAutoPagingVK = VK_SCROLL;
		if (!g_Plugin.AutoZoomMin)
			g_Plugin.AutoZoomMin = 0x10000;
		if (!g_Plugin.AutoZoomMax)
			g_Plugin.AutoZoomMax = 0x10000;
		if (g_Plugin.AutoZoomMin > g_Plugin.AutoZoomMax)
			g_Plugin.AutoZoomMin = g_Plugin.AutoZoomMax = (g_Plugin.AutoZoomMin + g_Plugin.AutoZoomMax) / 2;
		if (g_Plugin.MouseZoomMode > 2)
			g_Plugin.MouseZoomMode = 2;
		if (!g_Plugin.SmoothScrollingStep)
			g_Plugin.SmoothScrollingStep = 20;
		if (!g_Plugin.SmoothZoomingStep)
			g_Plugin.SmoothZoomingStep = 33;
		if (g_Plugin.SmoothScrollingStep > 10000)
			g_Plugin.SmoothScrollingStep = 10000;
		if (g_Plugin.SmoothZoomingStep > 10000)
			g_Plugin.SmoothZoomingStep = 10000;
		RegCloseKey(RegKey);
	}

	UnregisterClass(g_WndClassName, g_hInstance); // если вдруг остался зарегистрированным старый класс
	const WNDCLASSW wc = {CS_OWNDC | CS_DBLCLKS, WndProc, 0, 0, g_hInstance, NULL, LoadCursor(NULL, IDC_ARROW), NULL/*(HBRUSH)COLOR_BACKGROUND*/, NULL, g_WndClassName};
	RegisterClassW(&wc);

	// Добавить папку g_SelfPath в список, в котором ищутся загружаемые .dll
	if (const HMODULE hKernel = GetModuleHandleW(L"kernel32.dll"))
		if (void (__stdcall *SetDllDirectory)(LPCWSTR lpPathName) = (void (__stdcall*)(LPCWSTR))GetProcAddress(hKernel, "SetDllDirectoryW"))
			SetDllDirectory(g_SelfPath);

	//TODO("По хорошему бы заменить все GetStdHandle(STD_INPUT_HANDLE/STD_OUTPUT_HANDLE) на явные CreateFile");
	//g_Plugin.hInput = GetStdHandle(STD_INPUT_HANDLE);
	//g_Plugin.hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	g_Plugin.bFullScreen = g_Plugin.bFullScreenStartup;
	g_Plugin.Zoom = g_Plugin.AbsoluteZoom = 0x10000; // initial 100%
	g_Plugin.ZoomAuto = g_Plugin.bAutoZoom ? ZA_FIT : ZA_NONE;
	g_Plugin.ZoomAutoManual = false; //101129
	g_Plugin.hWnd = NULL;
	//g_Plugin.Image[0] = new C Image();
	//g_Plugin.Image[1] = new C Image();
	//g_Plugin.Image[2] = new C Image();
	//g_Plugin.dds = new DirectDrawSurface;
	//g_Plugin.dds1 = new DirectDrawSurface(g_Plugin.dds);
	//g_Plugin.dds2 = new DirectDrawSurface(g_Plugin.dds);
	//_ASSERTE(sizeof(g_Plugin.Image[0]->FileNameData) == sizeof(g_Plugin.MapFileNameData));
	//g_Plugin.MapFileName = NULL;
	//g_Plugin.MapFileNameData[0] = 0;
	g_Plugin.hDisplayThread = NULL;
	g_Plugin.bUncachedJump = false;

	g_Plugin.hDisplayReady = CreateEvent(NULL, FALSE, FALSE, NULL);
	g_Plugin.hDisplayEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	g_Plugin.hWorkEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	_ASSERTE(g_Plugin.hSynchroDone == NULL);
	//g_Plugin.hSynchroDone = CreateEvent(NULL, FALSE, FALSE, NULL);

	//g_Plugin.FarPanelInfo.ItemsNumber = -1;
	//g_Plugin.ImageX = g_Plugin.Image[0];
	//g_Plugin.ddsx = g_Plugin.dds;

	CPVDManager::LoadPlugins2();


	HRESULT hr = S_OK;
	hr = OleInitialize (NULL); // как бы попробовать включать Ole только во время драга. кажется что из-за него глючит переключалка языка
	//CoInitializeEx(NULL, COINIT_MULTITHREADED);


	if (!pTaskBar) {
		hr = CoCreateInstance(CLSID_TaskbarList,NULL,CLSCTX_INPROC_SERVER,IID_ITaskbarList2,(void**)&pTaskBar);
		if (hr == S_OK && pTaskBar) {
			hr = pTaskBar->HrInit();
		}
		if (hr != S_OK && pTaskBar) {
			if (pTaskBar) pTaskBar->Release();
			pTaskBar = NULL;
		}
	}

	MCHKHEAP;

	return true;
}

bool CPluginData::InitCMYK(BOOL bForceLoad)
{
	// Убрал. От InitPlugin мы вроде не зависим
	//if (!bInitialized) // на всякий случай
	//	InitPlugin();

	if (bCMYKinitialized) {
		if (!pCMYKpalette) // На всякий случай - проверим
			uCMYK2RGB = PVD_CMYK2RGB_FAST;
		return (pCMYKpalette!=NULL);
	}

	if (!bForceLoad) {
		// Если насильно грузить не просили - то и не дергаться, если палитра не используется
		if (uCMYK2RGB == PVD_CMYK2RGB_FAST)
			return false;
	}


	CMYK_ThreadProc(NULL);

	//if (!bCMYKstarted) {
	//	bCMYKstarted = true;
	//	_ASSERTE(hCMYKthread == NULL);
	//	if (!hCMYKthread) {
	//		nCMYK_ErrorNumber = nCMYK_LastError = 0;
	//		hCMYKthread = CreateThread(NULL, 0, CMYK_ThreadProc, NULL, 0, &nCMYKthread);
	//		if (!hCMYKthread) {
	//			nCMYK_ErrorNumber = MICMYKStartThreadFailed;
	//			nCMYK_LastError = GetLastError();
	//			goto wrap;
	//		}
	//	}
	//}
	//if (!bReqFinished)
	//	return true; // Требуется только запустить нить, данные пока НЕ нужны
	//WaitForSingleObject(hCMYKthread, INFINITE);

	bCMYKinitialized = true;
	//CloseHandle(hCMYKthread);
	//hCMYKthread = NULL;
	
//wrap:

	if (nCMYK_ErrorNumber) {
		const wchar_t* pszItems[10];
		int nItems = 0;
		wchar_t szFormatted[255];

		if (pCMYKpalette) {
			free(pCMYKpalette); pCMYKpalette = NULL;
		}

		pszItems[nItems++] = g_Plugin.pszPluginTitle;
		pszItems[nItems++] = GetMsg(MICantProcessCMYKpalette);

		wchar_t* pszFileName = ConcatPath(g_SelfPath, L"CMYK.png");
		pszItems[nItems++] = pszFileName ? pszFileName : L"CMYK.png";

		LPCWSTR pszFormat = GetMsg(nCMYK_ErrorNumber);
		if (wcschr(pszFormat, L'%')) {
			wsprintf(szFormatted, pszFormat, nCMYK_LastError);
			pszItems[nItems++] = szFormatted;
		} else {
			pszItems[nItems++] = pszFormat;
		}

		g_StartupInfo.Message(PluginNumberMsg, FMSG_WARNING|FMSG_MB_OK, 
			NULL, pszItems, nItems, 0);

		// Сказать, что палитру использовать нельзя
		uCMYK2RGB = PVD_CMYK2RGB_FAST;

		// Если диалог с ошибкой был закрыт через Esc - дождаться отпускания клавиши
		while (GetKeyState(VK_ESCAPE) & 0x8000) Sleep(1);
		// и очистить буфер ввода - там мог Esc остаться, что приведет к неожиданному закрытию картинки
		FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
		// сбросить флаг для этой функции
		GetAsyncKeyState(VK_ESCAPE); 
	}

	return (pCMYKpalette!=NULL);

	//bCMYKinitialized = true; // повторно не выполнять

	//bool lbRc = false;
	//CPVDManager decoder(NULL); // сразу, чтобы переменная со строкой статуса не разрушилась до Message
	//const wchar_t* pszItems[10];
	//int nItems = 0;

	//pszItems[nItems++] = g_Plugin.pszPluginTitle;
	//pszItems[nItems++] = GetMsg(MICantProcessCMYKpalette);

	//wchar_t* pszFileName = ConcatPath(g_SelfPath, L"CMYK.png");
	//wchar_t  szDecoder[80];
	//pszItems[nItems++] = pszFileName ? pszFileName : L"CMYK.png";
	//if (pszFileName) {
	//	FileMap cmyk(pszFileName);
	//	BYTE* pBuf = cmyk.MapView();
	//	if (pBuf) {
	//		pvdInfoPage2 InfoPage = {sizeof(pvdInfoPage2)};
	//		if (!decoder.Open(pszFileName, cmyk.lSize, pBuf, (uint)cmyk.lSize, InfoPage)) {
	//			pszItems[nItems++] = GetMsg(MICMYKpaletteCantOpen);
	//		} else {
	//			lstrcpyn(szDecoder, GetMsg(MIDecoderColon), 30);
	//			int nLen = lstrlen(szDecoder);
	//			lstrcpyn(szDecoder+nLen, decoder.GetName(), sizeofarray(szDecoder)-nLen-1);
	//			pszItems[nItems++] = szDecoder;
	//			
	//			if (InfoPage.lWidth != (16*16) || InfoPage.lHeight != (16*16)) {
	//				pszItems[nItems++] = GetMsg(MICMYKpaletteInvalidSize);
	//			} else if (InfoPage.nBPP != 24 && InfoPage.nBPP != 32) {
	//				pszItems[nItems++] = GetMsg(MICMYKpaletteInvalidBPP);
	//			} else {
	//				pvdInfoDecode2 DecodeInfo = {sizeof(pvdInfoDecode2)};
	//				DecodeInfo.Flags = PVD_IDF_COMPAT_MODE;
	//				if (!decoder.mp_Data->pPlugin->PageDecode2(
	//					decoder.mp_ImageContext, &DecodeInfo, 
	//					CPVDManager::DecodeCallback2, &decoder))
	//				{
	//					pszItems[nItems++] = decoder.mp_Data->szStatus;
	//				} else {
	//					if (DecodeInfo.lWidth != (16*16) || DecodeInfo.lHeight != (16*16)) {
	//						pszItems[nItems++] = GetMsg(MICMYKpaletteInvalidSize);
	//					} else if (DecodeInfo.nBPP != 24 && DecodeInfo.nBPP != 32) {
	//						pszItems[nItems++] = GetMsg(MICMYKpaletteInvalidBPP);
	//					} else if (DecodeInfo.ColorModel != PVD_CM_BGR) {
	//						pszItems[nItems++] = GetMsg(MICMYKpaletteInvalidColor);
	//					} else {
	//						nCMYKparts = 17; // пока фиксировано
	//						nCMYKsize = DecodeInfo.lWidth*DecodeInfo.lHeight;
	//						pCMYKpalette = (DWORD*)calloc(nCMYKsize,4);
	//						if (!pCMYKpalette) {
	//							pszItems[nItems++] = GetMsg(MIMemoryAllocationFailed);
	//						} else {
	//							BYTE* pDst = (BYTE*)pCMYKpalette;
	//							BYTE* pSrc = (BYTE*)DecodeInfo.pImage;
	//							uint lAbsSrcPitch = 0;
	//							int lDstPitch = DecodeInfo.lWidth * 4;

	//							if (DecodeInfo.lImagePitch < 0)
	//							{
	//								pDst += (int)(DecodeInfo.lHeight - 1) * lDstPitch;
	//								lAbsSrcPitch = -DecodeInfo.lImagePitch;
	//								lDstPitch = -lDstPitch;
	//							} else {
	//								lAbsSrcPitch = DecodeInfo.lImagePitch;
	//							}
	//						
	//							if (DecodeInfo.nBPP == 32) {
	//								lbRc = 
	//									BltHelper::Blit32_BGRA(pDst, pSrc, 0, DecodeInfo.lWidth, DecodeInfo.lHeight, lAbsSrcPitch, lDstPitch, 0, 0, 0, 0);
	//							} else {
	//								lbRc = 
	//									BltHelper::Blit24_BGR(pDst, pSrc, 0, DecodeInfo.lWidth, DecodeInfo.lHeight, lAbsSrcPitch, lDstPitch, 0, 0, 0, 0);
	//							}
	//						}
	//					}
	//				}
	//			}
	//		}
	//	}
	//}

	//if (!lbRc) {
	//	g_StartupInfo.Message(PluginNumberMsg, FMSG_WARNING|FMSG_MB_OK, 
	//			NULL, pszItems, nItems, 0);
	//	if (pCMYKpalette) {
	//		free(pCMYKpalette); pCMYKpalette = NULL;
	//	}

	//	uCMYK2RGB = PVD_CMYK2RGB_FAST;
	//	while (GetKeyState(VK_ESCAPE) & 0x8000)
	//		Sleep(1);
	//	// Очистить буфер ввода - там мог Esc остаться, что приведет к неожиданному закрытию картинки
	//	FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE)/*g_Plugin.hInput*/);
	//	//INPUT_RECORD ir; u32 t = 0;
	//	//while (PeekConsoleInput(g_Plugin.hInput, &ir, 1, &t) && t)
	//	//	ReadConsoleInput(g_Plugin.hInput, &ir, 1, &t);
	//	GetAsyncKeyState(VK_ESCAPE); // сбросить флаг для этой функции
	//}
	//
	//return lbRc;
}

void CPluginData::CreateDefaultPalettes()
{
	CreateDefaultPalette8bpp(pal8);
	CreateDefaultPalette8bppG(pal8G);
}

void CPluginData::CreateDefaultPalette8bpp(RGBQUAD* table)
{
	memset(table, 0, sizeof(*table)*256);
	/*
	Bit   07 06 05 04 03 02 01 00
	Data   R  R  R  G  G  G  B  B
	*/
	for (UINT i = 1; i<255; i++) {
		table[i].rgbBlue  = (i & 0x03) * 85;
		table[i].rgbGreen = ((i & 0x1C) >> 2) * 36;
		table[i].rgbRed   = ((i & 0xE0) >> 5) * 36;
	}
	((DWORD*)table)[255] = 0xFFFFFF;
}

void CPluginData::CreateDefaultPalette8bppG(RGBQUAD* table)
{
	DWORD* pTable = (DWORD*)table;
	for (UINT i = 0; i<=255; i++) {
		pTable[i] = i | (i << 8) | (i << 16);
	}
}

UINT32* CPluginData::GetPalette(UINT nBPP, pvdColorModel ColorModel)
{
	if (nBPP == 1) {
		if (ColorModel == PVD_CM_GRAY)
			return (UINT32*)pal1G;
		else
			return (UINT32*)pal1;
	}
	if (nBPP == 2) {
		if (ColorModel == PVD_CM_GRAY)
			return (UINT32*)pal2G;
		else
			return (UINT32*)pal2;
	}
	if (nBPP <= 4) {
		if (ColorModel == PVD_CM_GRAY)
			return (UINT32*)pal4G;
		else
			return (UINT32*)pal4;
	}

	if (ColorModel == PVD_CM_GRAY)
		return (UINT32*)pal8G;
	else
		return (UINT32*)pal8;
}


bool CPluginData::IsExtensionIgnored(const wchar_t *pFileName)
{
	if (!g_Plugin.sIgnoredExt[0])
		return false;
	
	const wchar_t *p = GetExtension(pFileName);
	//const wchar_t *pS = wcsrchr(pFileName, '\\');
	//if (!pS) pS = pFileName;
	//if (((p = wcsrchr(pFileName, '.')) != NULL) && (p >= pS))
	//	p++;
	//else
	//	p = NULL;

	// Если расширение указано в "необрабатываемых"
	if (g_Plugin.sIgnoredExt[0])
	{
		//if (!p || !*p)
		//{
		//	// Для пропускания файлов без расширений - задать в списке точку
		//	if (ExtensionMatch(g_Plugin.sIgnoredExt, L"."))
		//		return true;
		//} else
		if (ExtensionMatch(g_Plugin.sIgnoredExt, p)) {
			return true;
		}
	}
	
	return false;
}
