
#include "PictureView.h"
#include "PictureView_Lang.h"
#include "PVDManager.h"
#include "PVDInterface/BltHelper.h"
#include <ShObjIdl.h>
#include "headers/farcolor.hpp"

extern RECT GetDisplayRect();


ImageInfo::ImageInfo()
{
	Decoder = NULL; pDraw = NULL; Display = NULL;

	//FileNamePrefix[0] = 0; // \\?\ ...
	//FileNameData[0] = 0;

	bSelected = false;
	lWidth = lHeight = nBPP = lDecodedWidth = lDecodedHeight = nDecodedBPP = 0;
	//OpenTimes[0] = 0;
	lOpenTime = lStartOpenTime = 0;
	lTimeOpen = lTimeDecode = lTimeTransfer = lTimePaint = 0;
	bTimePaint = FALSE;
	nPanelItemRaw = nPages = nPage = Animation = lFrameTime = 0;

	DecoderName[0] = 0; FormatName[0] = 0; Compression[0] = 0; Comments[0] = 0;

	Decoder = new PVDManager(this);
	_ASSERTE(Decoder);

	//dds = new DirectDrawSurface;
	
	memset(&InfoPage, 0, sizeof(InfoPage));
}

ImageInfo::~ImageInfo()
{
	if (Decoder) {
		delete Decoder;
		Decoder = NULL;
	}

	if (pDraw) {
		//delete dds;
		//dds = NULL;
		DisplayClose();
	}
}

//wchar_t* ImageInfo::GetFileName()
//{
//	if (!FileNameData[0]) return FileNameData;
//	FileNamePrefix[0] = L'\\'; FileNamePrefix[1] = L'\\';
//	FileNamePrefix[2] = L'?';  FileNamePrefix[3] = L'\\';
//	return FileNamePrefix;
//}

//bool ImageInfo::Open()
//{
//	return false;
//}

void ImageInfo::CheckPages(UINT32 anNewPages)
{
	if (anNewPages) {
		if (nPages != anNewPages) {
			if (anNewPages > nPage) {
				nPages = anNewPages;
			} else {
				_ASSERTE(anNewPages > nPage);
			}
		}
	}
}

uint ImageInfo::PanelItemRaw()
{
	return nPanelItemRaw;
}

void ImageInfo::SetPanelItemRaw(uint nRaw)
{
	TODO("При переходах может быть -1 и g_Plugin.FarPanelInfo.ItemsNumber. В этом случае они потом еще будут меняться");
	if (nPanelItemRaw != nRaw) {
		nPanelItemRaw = nRaw;
		nPage = 0; // сброс номера страницы
	}
}

TODO("Убрать аргумент piSubDecoder");
bool ImageInfo::ImageOpen(const wchar_t *pFileName, const unsigned char *buf, int lBuf)
{
	_ASSERTE(Decoder);
	_ASSERTE(pFileName && *pFileName);
	
	g_Plugin.InitCMYK(FALSE); // Дождаться его завершения

	// -- все равно дальше Map открывается	
	//// Это может произойти, если открытие идет из вьювера
	//unsigned char tempBuf[128];
	////if (!buf || !lBuf)
	//{
	//	buf = NULL;
	//	// Зачем это чтение? возможно для проверки, доступен ли файл?
	//	HANDLE hFile = CreateFileW(pFileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	//	if (hFile == INVALID_HANDLE_VALUE)
	//		hFile = CreateFileW(pFileName, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
	//	if (hFile != INVALID_HANDLE_VALUE) {
	//		DWORD dwRead = 0;
	//		if (ReadFile(hFile, tempBuf, sizeof(tempBuf), &dwRead, 0)) {
	//			buf = tempBuf; lBuf = dwRead;
	//		}
	//		CloseHandle(hFile);
	//	}
	//	if (!buf || !lBuf)
	//		return false;
	//}
	

	i64 lFileSize;
	if (g_Plugin.MapFileName[0])
		pFileName = g_Plugin.MapFileName;

	// По идее уже проверяли, но на всякий случай
	if ((g_Plugin.FlagsWork & FW_FORCE_DECODE) == 0) {
		if (g_Plugin.IsExtensionIgnored(pFileName))
			return false;
	}
	
	FileMap Map(pFileName);
	lFileSize = Map.lSize;
	if (g_Plugin.MapFileName[0])
		if (Map.MapView())
		{
			buf = Map.pMapping;
			lBuf = (int)Min<i64>(Map.lSize, ~0u);
			//if (lBuf) -- IN_APL этого не пережил
			//	lFileSize = 0; // все данные уже в памяти
		}
		else
		{
			buf = NULL;
			lBuf = 0;
		}

	bool result = false;
	//const wchar_t *p;
	//if (p = wcsrchr(pFileName, '.'))
	//	p++;

	//deletex(this->Decoder);
	//PVDManager *pPVDDecoder = NULL;


	//// Если расширение указано в "необрабатываемых"
	//TO DO("Перенести вверх, а еще лучше вынести в вызывающую функцию");
	//if (g_Plugin.sIgnoredExt[0]) {
	//	if (!p || !*p) {
	//		// Для пропускания файлов без расширений - задать в списке точку
	//		if (ExtensionMatch(g_Plugin.sIgnoredExt, L"."))
	//			return false;
	//	} else if (ExtensionMatch(g_Plugin.sIgnoredExt, p)) {
	//		return false;
	//	}
	//}

	TODO("Перенести в начало функции");
	this->lStartOpenTime = timeGetTime();



	//Decoder->mi_SubDecoder = *piSubDecoder;
	
lReOpen2:
	//pPVDDecoder->SetAllowedPriority(7);
	memset(&InfoPage, 0, sizeof(InfoPage)); // !!! cbSize не заполнять
	result = Decoder->Open(pFileName, lFileSize, buf, lBuf, InfoPage);

	//*piSubDecoder = Decoder->mi_SubDecoder;
	if (!result)
		return false;

	if (g_Plugin.FlagsWork & FW_FIRST_CALL) 
	{
		g_Plugin.bAutoPagingChanged = 0;
		if (!(g_Plugin.FlagsWork & FW_QUICK_VIEW) && g_Plugin.bAutoPagingSet)
		{
			BOOL bChanged = FALSE;
			SHORT CurState = GetKeyState(g_Plugin.nAutoPagingVK);
			if (!Animation && nPages>1) {
				// Включить ScrollLock (PgUp/PgDn листают страницы)
				if ((CurState & 1) == 0)
					bChanged = TRUE;
			} else {
				// вЫключить ScrollLock (PgUp/PgDn листают страницы)
				if ((CurState & 1) != 0)
					bChanged = TRUE;
			}
			if (bChanged) {
				g_Plugin.bAutoPagingChanged = ((CurState & 1) == 0) ? 1 : 2;
				keybd_event((BYTE)g_Plugin.nAutoPagingVK, (BYTE)MapVirtualKey(g_Plugin.nAutoPagingVK, 0), KEYEVENTF_EXTENDEDKEY, 0);
				keybd_event((BYTE)g_Plugin.nAutoPagingVK, (BYTE)MapVirtualKey(g_Plugin.nAutoPagingVK, 0), KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
			}
		}
	}
		
	if (result && !g_Plugin.hThread)
	{
		result = CreateDisplayThread();
		if (result) {
			TODO("А здесь в чем смысл?");
			SetPanelItemRaw(PANEL_ITEM_UNAVAILABLE);
		}
	}

	if (result) {
		TODO("Хорошо бы разрулить ImageDecode и запуск нити Display. Собственно декодирование можно начать еще ДО создания DX и прочего");
		if (!ImageDecode()) {
			WARNING("Если ImageDecode обламывается - то мы зацикливаемся, т.к. ImageOpen выбирает тот же декодер!");
			WARNING("В любом случае нужно оптимизировать окрытие? Не разрушать pPVDDecoder и его локальную переменную bProcessed");
			uint nDecoders = (uint)PVDManager::Decoders.size();
			if (nDecoders <= 1)
				return false;

			if (g_Plugin.FlagsWork & FW_TERMINATE)
				return false; // Плагин закрывается
			//if (!g_Plugin.hWnd)
			//	return false; 

			Decoder->mi_SubDecoder++;
			if (Decoder->mi_SubDecoder >= nDecoders)
				Decoder->mi_SubDecoder = 0;

			goto lReOpen2;
		} else {
			Decoder->ResetProcessed();
		}
	}
	
	return result;
}

bool ImageInfo::ImageDecode(void)
{
	//this->iPage = 0;
	const uint nPages = this->nPages;
	//_ASSERTE(nPage < nPages);
	// Допустимая ситуация. Открыли djvu через djvu.pvd
	// Переключились на другой декодер (AltPgDn), согласился Shell.pvd, а он умеет только
	// превьюшку - то есть только одну страницу.
	if (nPage >= nPages) 
		nPage = 0;
	
	// InfoPage - мог быть уже получен, тогда декодер не будет повторно дергать pvdPageInfo
	bool result = Decoder->Decode(this, &pDraw, false, InfoPage);
	// Количество страниц (если оно изменилось) обновляется через CheckPages(...)
	// Этот AI убираем. Пусть PVD_IIF_ANIMATED выставляет сам PVD
	//if (nPages != this->nPages) // buggy GFL with animated GIF
	//	this->Animation = 1;
	// Пока вообще не закрываем
	//TODO("Когда потребуется уточненное декодирование возможно придется не закрывать и с одной страницей");
	//if (!result || this->nPages < 2)
	//{
	//	this->Decoder->Close();
	//	//deletex(this->Decoder);
	//}

	if (!result)
	{
		Decoder->Close();
	} else {
		if (this == g_Plugin.Image[0]) {
			// Иначе в GDI что-то не перерисовывается
			TODO("Не помогло. тут (this != g_Plugin.Image[0])");
			InvalidateRect(g_Plugin.hWnd, NULL, FALSE);
		}

		if (g_Plugin.FlagsWork & FW_JUMP_DISABLED) {
		
			this->SetPanelItemRaw(PANEL_ITEM_UNAVAILABLE);
			
		} else {
		
			if (g_Plugin.FarPanelInfo.ItemsNumber == -1)
			{
				g_StartupInfo.Control(INVALID_HANDLE_VALUE, FCTL_GETPANELINFO, 0, (LONG_PTR)&g_Plugin.FarPanelInfo);
				g_Plugin.nPanelItems = g_Plugin.FarPanelInfo.ItemsNumber;
				this->SetPanelItemRaw(g_Plugin.FarPanelInfo.CurrentItem);
				this->bSelected = false;
				g_Plugin.nPanelFolders = 0;
				//2009-09-05. Из nPanelItems нужно вычесть количество папок (в том числе и "..")
				if (g_Plugin.nPanelItems > 0) {
					uint nMaxSize = 0x200;
					PluginPanelItem* item = (PluginPanelItem*)calloc(nMaxSize,1);
					if (item) {
						uint i, nSize;
						for (i = 0; i < g_Plugin.nPanelItems; i++) {
							nSize = g_StartupInfo.Control(INVALID_HANDLE_VALUE, FCTL_GETPANELITEM, i, NULL);
							if (!nSize) break;
							if (nSize > nMaxSize) {
								nMaxSize = nSize + 0x100;
								free(item);
								item = (PluginPanelItem*)calloc(nMaxSize,1);
								if (!item) break;
							}
							
							g_StartupInfo.Control(INVALID_HANDLE_VALUE, FCTL_GETPANELITEM, i, (LONG_PTR)item);
							if ((item->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
								g_Plugin.nPanelFolders ++;
							} else {
								break; // дошли до первого файла, считаем, что папки кончились!
							}
						}

						// Теперь нужно получить информацию о выделении файла 
						i = this->PanelItemRaw();
						nSize = g_StartupInfo.Control(INVALID_HANDLE_VALUE, FCTL_GETPANELITEM, i, NULL);
						if (nSize) {
							if (nSize > nMaxSize) {
								nMaxSize = nSize;
								free(item);
								item = (PluginPanelItem*)calloc(nMaxSize,1);
							}
							if (item) {
								g_StartupInfo.Control(INVALID_HANDLE_VALUE, FCTL_GETPANELITEM, i, (LONG_PTR)item);
								this->bSelected = (item->Flags & PPIF_SELECTED) == PPIF_SELECTED;
							}
						}
					}
					if (item) { free(item); item = NULL; }
				}
			}
		}
	}
	return result;
}



void ImageInfo::DisplayClose()
{
	if (pDraw) {
		// Сбрасываем сразу, чтобы нить отрисовки знала - ничего нет...
		LPVOID lpDraw = pDraw;
		pDraw = NULL;
		
		if (Display && Display->pPlugin) {
			Display->pPlugin->DisplayClose2(lpDraw);
		}
	}
}

SIZE ImageInfo::GetDefaultScaleSize()
{
	RECT ParentRect = GetDisplayRect(); //{0, 0, 1280, 960};
	
	//const вызывает инициализацию переменных ДО входа в функцию (по возможности). что нам не подходит - хочется видеть последовательность действий!
	int lScreenWidth = ParentRect.right - ParentRect.left;
	int lScreenHeight = ParentRect.bottom - ParentRect.top;

	RECT ImgRect = {0,0,lScreenWidth,lScreenHeight};
	
	// Тут используем НЕ this->lWidth & this->lHeight
	// а InfoPage.lWidth & InfoPage.lHeight. Т.к. именно они
	// будут содержать актуальную информацию о декодируемой СТРАНИЦЕ многостраничного документа
	
	u32 Zoom;
	if (g_Plugin.ZoomAuto || g_Plugin.FlagsWork & FW_QUICK_VIEW)
	{
		//const вызывает инициализацию переменных ДО входа в функцию (по возможности). что нам не подходит - хочется видеть последовательность действий!
		u32 ZoomW = MulDivU32(lScreenWidth, 0x10000, InfoPage.lWidth /*dds->m_lWorkWidth*/);
		u32 ZoomH = MulDivU32(lScreenHeight, 0x10000, InfoPage.lHeight /*dds->m_lWorkHeight*/);
		Zoom = (g_Plugin.ZoomAuto == ZA_FIT || g_Plugin.FlagsWork & FW_QUICK_VIEW) ? Min(ZoomW, ZoomH) : Max(ZoomW, ZoomH);
		if (!Zoom)
			Zoom = 1;
		if (g_Plugin.bAutoZoomMin && Zoom < g_Plugin.AutoZoomMin)
			Zoom = g_Plugin.AutoZoomMin;
		if (g_Plugin.bAutoZoomMax && Zoom > g_Plugin.AutoZoomMax)
			Zoom = g_Plugin.AutoZoomMax;
	}
	else
	{
		Zoom = g_Plugin.AbsoluteZoom ? g_Plugin.AbsoluteZoom : g_Plugin.Zoom;
	}

	
	//const вызывает инициализацию переменных ДО входа в функцию (по возможности). что нам не подходит - хочется видеть последовательность действий!
	int lWidth = MulDivU32R(InfoPage.lWidth, Zoom, 0x10000);
	int lHeight = MulDivU32R(InfoPage.lHeight, Zoom, 0x10000);
	
	SIZE sz = {lWidth,lHeight};
	
	return sz;
}








PluginData::PluginData()
{
	bInitialized = false;
	sLogFile[0] = 0;
	
	pszPluginTitle = L"Picture View 2";

	bCMYKinitialized = bCMYKstarted = false;
	//hCMYKthread = 
	hGDIPlus = NULL; nCMYK_ErrorNumber = nCMYK_LastError = 0;
	nCMYKparts = 0; pCMYKpalette = 0; nCMYKsize = 0;

	pTaskBar = NULL;

	hWnd = hParentWnd = hFarWnd = hConEmuWnd = hDesktopWnd = NULL;
	hConEmuCtrlPressed = hConEmuShiftPressed = NULL;
	ZeroMemory(&cci,sizeof(cci));
	//hInput = hOutput = 
	hThread = NULL;
	nBackColor = nQVBackColor = 0;
	ZeroMemory(&ViewPanelT, sizeof(ViewPanelT));
	ZeroMemory(&ViewPanelG, sizeof(ViewPanelG));

	FarVersion = 0;

	ZeroMemory(&ViewCenter, sizeof(ViewCenter));
	ZeroMemory(&DragBase, sizeof(DragBase));
	bDragging = bScrolling = bZoomming = bCorrectMousePos = bMouseHided = bIgnoreMouseMove = bTrayOnTopDisabled = bFullScreen = false;
	Zoom = AbsoluteZoom = ZoomAuto = 0;

	//MapFileNamePrefix[0] = MapFileNameData[0] = 0;

	FlagsDisplay = FlagsWork = 0;
	SelectionChanged = false;

	// вроде так: Image[0] - текущий, [1] и [2] буферы кеширования
	// ImageX указатель на тот Image[jj], который сейчас декодируется
	ZeroMemory(Image,sizeof(Image)); ImageX = NULL;

	ZeroMemory(&FarPanelInfo, sizeof(FarPanelInfo));
	nPanelItems = nPanelFolders = 0;

	bHookArc = bHookQuickView = bHookView = bHookEdit = false;
	lstrcpy(sHookPrefix, DEFAULT_PREFIX);
	sIgnoredExt[0] = 0;
	bTrayDisable = bFullScreenStartup = bLoopJump = bFreePosition = bFullDisplayReInit = bMarkBySpace = false;
	bAutoPaging = true; bAutoPagingSet = false; nAutoPagingVK = VK_SCROLL; bAutoPagingChanged = 0;
	uCMYK2RGB = PVD_CMYK2RGB_PRECISE;
	bCachingRP = bCachingVP = bAutoZoom = bAutoZoomMin = bAutoZoomMax = false;
	AutoZoomMin = AutoZoomMax = 0; bKeepZoomAndPosBetweenFiles = true; nKeepPanCorner = 0;
	bSmoothScrolling = bSmoothZooming = false;
	SmoothScrollingStep = SmoothZoomingStep = MouseZoomMode = 0; // 0 - as keyboard; 1 - to screen center; 2 - hold position

	hDisplayEvent = hWorkEvent = hSynchroDone = NULL;
	bUncachedJump = false;

	bTitleSaved = false;
	TitleSave[0] = 0;
}

PluginData::~PluginData()
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
}

u32 PluginData::BackColor()
{
	if (FlagsWork & FW_QUICK_VIEW)
		return nQVBackColor;
	else
		return nBackColor;
}

void PluginData::SaveTitle()
{
	if (bTitleSaved)
		return;
	bTitleSaved = TRUE;
	MCHKHEAP;
	GetConsoleTitleW(TitleSave, sizeofarray(TitleSave));
	MCHKHEAP;
}

void PluginData::RestoreTitle()
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

//wchar_t* PluginData::GetMapFileName()
//{
//	if (!MapFileNameData[0]) return MapFileNameData;
//	MapFileNamePrefix[0] = L'\\'; MapFileNamePrefix[1] = L'\\';
//	MapFileNamePrefix[2] = L'?';  MapFileNamePrefix[3] = L'\\';
//	return MapFileNamePrefix;
//}

bool PluginData::InitHooks()
{
	if (bHookInitialized)
		return true;
	bHookInitialized = true;

	MCHKHEAP;
	
	gnMainThreadId = GetCurrentThreadId();
	
	g_Plugin.FarVersion = (DWORD)g_StartupInfo.AdvControl(g_StartupInfo.ModuleNumber, ACTL_GETFARVERSION, NULL);
	g_Plugin.hDesktopWnd = GetDesktopWindow();

	//2009-10-04 динамическая длина
	g_SelfPath = _wcsdup(g_StartupInfo.ModuleName);
	wchar_t *p = wcsrchr(g_SelfPath, '\\');
	_ASSERTE(p);
	if (p) p[1] = 0; else g_SelfPath[0] = 0;

	// Сформируем наш ключ реестра
	_ASSERTE(g_StartupInfo.RootKey && *g_StartupInfo.RootKey);
	g_RootKey = ConcatPath(g_StartupInfo.RootKey, L"PictureView2");
		

	//ZeroMemory(&g_Plugin, sizeof(g_Plugin));

	bHookArc = true;
	bHookQuickView = true;
	bHookView = false; bHookCtrlShiftF3 = false;
	bHookEdit = false; bHookCtrlShiftF4 = false;
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

bool PluginData::InitPlugin()
{
	if (hConEmuCtrlPressed) { CloseHandle(hConEmuCtrlPressed); hConEmuCtrlPressed = NULL; }
	if (hConEmuShiftPressed) { CloseHandle(hConEmuShiftPressed); hConEmuShiftPressed = NULL; }
	
	if (bInitialized)
		return true;
	bInitialized = true;
	
	pszPluginTitle = GetMsg(MIPluginName);
	
	_ASSERTE(gnMainThreadId!=0);

	MCHKHEAP;
	
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
	UINT PanelTextColor = (UINT)g_StartupInfo.AdvControl(g_StartupInfo.ModuleNumber, ACTL_GETCOLOR, (void*)COL_PANELTEXT);
	DWORD nDefConColors[] = 	{ // Default color scheme (Windows standard)
		0x00000000, 0x00800000, 0x00008000, 0x00808000, 0x00000080, 0x00800080, 0x00008080, 0x00c0c0c0, 
		0x00808080, 0x00ff0000, 0x0000ff00, 0x00ffff00, 0x000000ff, 0x00ff00ff, 0x0000ffff, 0x00ffffff};
	PanelTextColor = (PanelTextColor & 0xF0) >> 4;
	g_Plugin.nQVBackColor = nDefConColors[PanelTextColor];
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

	TODO("По хорошему бы заменить все GetStdHandle(STD_INPUT_HANDLE/STD_OUTPUT_HANDLE) на явные CreateFile");
	//g_Plugin.hInput = GetStdHandle(STD_INPUT_HANDLE);
	//g_Plugin.hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	g_Plugin.bFullScreen = g_Plugin.bFullScreenStartup;
	g_Plugin.Zoom = g_Plugin.AbsoluteZoom = 0x10000; // initial 100%
	g_Plugin.ZoomAuto = g_Plugin.bAutoZoom ? ZA_FIT : ZA_NONE;
	g_Plugin.ZoomAutoManual = false; //101129
	g_Plugin.hWnd = NULL;
	g_Plugin.Image[0] = new ImageInfo();
	g_Plugin.Image[1] = new ImageInfo();
	g_Plugin.Image[2] = new ImageInfo();
	//g_Plugin.dds = new DirectDrawSurface;
	//g_Plugin.dds1 = new DirectDrawSurface(g_Plugin.dds);
	//g_Plugin.dds2 = new DirectDrawSurface(g_Plugin.dds);
	//_ASSERTE(sizeof(g_Plugin.Image[0]->FileNameData) == sizeof(g_Plugin.MapFileNameData));
	//g_Plugin.MapFileName = NULL;
	//g_Plugin.MapFileNameData[0] = 0;
	g_Plugin.hThread = NULL;
	g_Plugin.bUncachedJump = false;

	g_Plugin.hDisplayEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	g_Plugin.hWorkEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	g_Plugin.hSynchroDone = CreateEvent(NULL, FALSE, FALSE, NULL);

	g_Plugin.FarPanelInfo.ItemsNumber = -1;
	g_Plugin.ImageX = g_Plugin.Image[0];
	//g_Plugin.ddsx = g_Plugin.dds;

	PVDManager::LoadPlugins2();


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

bool PluginData::InitCMYK(BOOL bForceLoad)
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

		g_StartupInfo.Message(g_StartupInfo.ModuleNumber, FMSG_WARNING|FMSG_MB_OK, 
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
	//PVDManager decoder(NULL); // сразу, чтобы переменная со строкой статуса не разрушилась до Message
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
	//					PVDManager::DecodeCallback2, &decoder))
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
	//	g_StartupInfo.Message(g_StartupInfo.ModuleNumber, FMSG_WARNING|FMSG_MB_OK, 
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

void PluginData::CreateDefaultPalettes()
{
	CreateDefaultPalette8bpp(pal8);
	CreateDefaultPalette8bppG(pal8G);
}

void PluginData::CreateDefaultPalette8bpp(RGBQUAD* table)
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

void PluginData::CreateDefaultPalette8bppG(RGBQUAD* table)
{
	DWORD* pTable = (DWORD*)table;
	for (UINT i = 0; i<=255; i++) {
		pTable[i] = i | (i << 8) | (i << 16);
	}
}

UINT32* PluginData::GetPalette(UINT nBPP, pvdColorModel ColorModel)
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


bool PluginData::IsExtensionIgnored(const wchar_t *pFileName)
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





CFunctionLogger::~CFunctionLogger()
{
	WriteLog(sInfo);
}
CFunctionLogger::CFunctionLogger(LPCWSTR asFunc)
{
	sInfo[0] = L'~';
	lstrcpyn(sInfo+1, asFunc, MAX_PATH);
	WriteLog(sInfo+1);
}
void CFunctionLogger::FunctionLogger(LPCWSTR asFunc)
{
	WriteLog(asFunc);
}
CFunctionLogger::CFunctionLogger(LPCWSTR asFuncFormat, int nArg1)
{
	sInfo[0] = 0;
	if (!*g_Plugin.sLogFile)
		return;

	sInfo[0] = L'~'; sInfo[1] = 0;
	wsprintf(sInfo+1, asFuncFormat, nArg1);
	WriteLog(sInfo+1);
}
void CFunctionLogger::FunctionLogger(LPCWSTR asFuncFormat, int nArg1)
{
	if (!*g_Plugin.sLogFile)
		return;

	wchar_t sInfo[MAX_PATH];
	wsprintf(sInfo, asFuncFormat, nArg1);
	WriteLog(sInfo);
}
CFunctionLogger::CFunctionLogger(LPCWSTR asFuncFormat, LPCWSTR asArg1)
{
	sInfo[0] = 0;
	if (!*g_Plugin.sLogFile)
		return;

	sInfo[0] = L'~'; sInfo[1] = 0;	
	if ((lstrlen(asFuncFormat) + lstrlen(asArg1)) <= MAX_PATH) {
		wsprintf(sInfo+1, asFuncFormat, asArg1);
	} else {
		lstrcpyn(sInfo+1, asFuncFormat, MAX_PATH);
	}
	WriteLog(sInfo+1);
}
void CFunctionLogger::FunctionLogger(LPCWSTR asFuncFormat, LPCWSTR asArg1)
{
	if (!*g_Plugin.sLogFile)
		return;

	wchar_t sInfo[MAX_PATH];
	if ((lstrlen(asFuncFormat) + lstrlen(asArg1)) <= MAX_PATH) {
		wsprintf(sInfo, asFuncFormat, asArg1);
	} else {
		lstrcpyn(sInfo, asFuncFormat, MAX_PATH);
	}
	WriteLog(sInfo);
}
void CFunctionLogger::WriteLog(LPCWSTR pszText)
{
	if (!*g_Plugin.sLogFile || !pszText || !*pszText)
		return;

	{
		const wchar_t *szThread = L"[Unkn] ";
		DWORD nID = GetCurrentThreadId();
		if (nID == gnDisplayThreadId)
			szThread = L"[Disp] ";
		if (nID == gnMainThreadId)
			szThread = L"[Main] ";
		DWORD nWrite = 0;
		wchar_t szTemp[2048];
		lstrcpy(szTemp, szThread);
		lstrcpyn(szTemp+7, pszText, 2000); nWrite += lstrlen(szTemp);
		szTemp[nWrite++] = L'\r'; szTemp[nWrite++] = L'\n'; szTemp[nWrite] = 0;

		HANDLE hFile = CreateFile(g_Plugin.sLogFile, GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
		
		if (hFile != INVALID_HANDLE_VALUE) {
			SetFilePointer(hFile, 0,0, FILE_END);
			WriteFile(hFile, szTemp, nWrite*2, &nWrite, 0);
			#ifdef _DEBUG
			OutputDebugString(szTemp);
			#endif
			//WriteFile(hFile, szThread, wcslen(szThread)*2, &nWrite, 0);
			//WriteFile(hFile, pszText, wcslen(pszText)*2, &nWrite, 0);
			//WriteFile(hFile, L"\r\n", 4, &nWrite, 0);
			CloseHandle(hFile);
		}
	}
}
