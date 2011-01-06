
#pragma once

class PVDManager;
class DirectDrawSurface;
class ModuleData;

//#define MAX_PIC_PATH_LEN 0x1000

#define DEFAULT_PREFIX L"pic"

#include "PictureView_FileName.h"
#include "PVDInterface/PictureViewPlugin.h"

interface ITaskbarList2;

class RefRelease
{
private:
	int mn_RefCount;
public:
	RefRelease() {
		mn_RefCount = 1;
	};
	void AddRef() {
		mn_RefCount ++;
	};
	void Release() {
		mn_RefCount--;
		_ASSERTE(mn_RefCount>=0);
		if (mn_RefCount == 0)
			delete this;
	};
protected:
	virtual ~RefRelease() {
		_ASSERTE(mn_RefCount==0);
	};
};

class ImageInfo
{
public:
	ImageInfo();
	~ImageInfo();
	//wchar_t *GetFileName();
	//bool Open();
	bool ImageOpen(const wchar_t *pFileName, const unsigned char *buf, int lBuf);
	void CheckPages(UINT32 anNewPages);
	void DisplayClose();
	SIZE GetDefaultScaleSize();
private:
	bool ImageDecode(void);
public:

	PVDManager *Decoder;
	ModuleData *Display;
	//DirectDrawSurface *dds;
	LPVOID pDraw;
	pvdInfoPage2 InfoPage;

	//wchar_t FileNamePrefix[4]; // \\?\ ...
	UnicodeFileName FileName;

	bool bSelected;
	uint lWidth, lHeight, nBPP;
	uint lDecodedWidth, lDecodedHeight, nDecodedBPP;
	//uint lOpenTime;
	//wchar_t OpenTimes[0x50];
	DWORD lOpenTime, lStartOpenTime;
	DWORD lTimeOpen, lTimeDecode, lTimeTransfer, lTimePaint;
	BOOL  bTimePaint;
	uint nPages, nPage;
	uint Animation;
	uint lFrameTime;

	wchar_t DecoderName[0x80], FormatName[0x80], Compression[0x80], Comments[0x100];

	uint PanelItemRaw();
	void SetPanelItemRaw(uint nRaw);
private:
	uint nPanelItemRaw;
};
typedef ImageInfo* PImageInfo;

class PluginData
{
public:
	PluginData();
	~PluginData();

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


	//DirectDrawSurface *dds, *dds1, *dds2, *ddsx;
	HWND hWnd, // окно отрисовки
		 hParentWnd, // текущее родительское окно для hWnd (это FAR, ConEmu, или Desktop)
		 hFarWnd, // консольное окно, или окно отрисовки в ConEmu
		 hConEmuWnd, // Главное окно conemu, или NULL в чистом FAR
		 hDesktopWnd; // no comment
	HANDLE hConEmuCtrlPressed, hConEmuShiftPressed;
	CONSOLE_CURSOR_INFO cci;
	//HANDLE hInput, hOutput;
	HANDLE hThread;
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
	UnicodeFileName MapFileName;
	//wchar_t *GetMapFileName();

	uint FlagsDisplay;
	uint FlagsWork;
	bool SelectionChanged;

	// вроде так: Image[0] - текущий, [1] и [2] буферы кеширования
	// ImageX указатель на тот Image[jj], который сейчас декодируется
	PImageInfo Image[3];
	PImageInfo ImageX;

	PanelInfo FarPanelInfo;
	uint nPanelItems, nPanelFolders;

	bool bHookArc, bHookQuickView, bHookView, bHookEdit, bHookCtrlShiftF3, bHookCtrlShiftF4;
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

	HANDLE hDisplayEvent, hWorkEvent;
	HANDLE hSynchroDone; // Выставляется в основной нити после выполнения запроса (help, config, ...)
	bool bUncachedJump;

	bool bTitleSaved;
	wchar_t TitleSave[0x1000];

	ITaskbarList2* pTaskBar;
	
	wchar_t sLogFile[MAX_PATH+1];
};

struct FileMap
{
	HANDLE hFile, hMapping;
	i64 lSize;
	BYTE *pMapping;

	FileMap(const wchar_t *pName)
	{
		hFile = hMapping = INVALID_HANDLE_VALUE;
		pMapping = NULL;
		if ((hFile = CreateFileW(pName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) != INVALID_HANDLE_VALUE)
		{
			*(u32*)&lSize = GetFileSize(hFile, (u32*)&lSize + 1);
			if ((u32)lSize == INVALID_FILE_SIZE && GetLastError() != NO_ERROR)
				lSize = 0;
			if (!(hMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL)))
				CloseHandle(hFile), hFile = INVALID_HANDLE_VALUE;
		}
	}
	~FileMap(void)
	{
		if (pMapping)
			UnmapViewOfFile(pMapping), pMapping = NULL;
		if (hMapping != INVALID_HANDLE_VALUE)
			CloseHandle(hMapping), hMapping = INVALID_HANDLE_VALUE;
		if (hFile != INVALID_HANDLE_VALUE)
			CloseHandle(hFile), hFile = INVALID_HANDLE_VALUE;
	}
	BYTE *MapView(void)
	{
		if (hFile == INVALID_HANDLE_VALUE || !hFile)
			return NULL;
		if (hMapping == INVALID_HANDLE_VALUE || !hMapping)
			return NULL;
		return pMapping = (BYTE*)MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0);
	}
};

class CFunctionLogger
{
public:
	~CFunctionLogger();
	CFunctionLogger(LPCWSTR asFunc);
	CFunctionLogger(LPCWSTR asFuncFormat, int nArg1);
	CFunctionLogger(LPCWSTR asFuncFormat, LPCWSTR asArg1);
	static void FunctionLogger(LPCWSTR asFunc);
	static void FunctionLogger(LPCWSTR asFuncFormat, int nArg1);
	static void FunctionLogger(LPCWSTR asFuncFormat, LPCWSTR asArg1);
private:
	wchar_t sInfo[MAX_PATH];
	static void WriteLog(LPCWSTR pszText);
};

#ifdef _DEBUG
	//#define FUNCLOGGER(asFunc) CFunctionLogger flog(asFunc);
	//#define FUNCLOGGERI(asFormat,nArg1) CFunctionLogger flog(asFormat,nArg1);
	//#define FUNCLOGGERS(asFormat,sArg1) CFunctionLogger flog(asFormat,sArg1);
	#define FUNCLOGGER(asFunc) CFunctionLogger::FunctionLogger(asFunc);
	#define FUNCLOGGERI(asFormat,nArg1) CFunctionLogger::FunctionLogger(asFormat,nArg1);
	#define FUNCLOGGERS(asFormat,sArg1) CFunctionLogger::FunctionLogger(asFormat,sArg1);
#else
	#define FUNCLOGGER(asFunc) CFunctionLogger::FunctionLogger(asFunc);
	#define FUNCLOGGERI(asFormat,nArg1) CFunctionLogger::FunctionLogger(asFormat,nArg1);
	#define FUNCLOGGERS(asFormat,sArg1) CFunctionLogger::FunctionLogger(asFormat,sArg1);
#endif
