
#include <stdio.h>
#include <windows.h>
#include <TCHAR.H>
#include "MultiDef.h"
#ifndef _UNICODE
    #include "FarSdk/Ascii/plugin.hpp"
    #include "FarSdk/Ascii/farcolor.hpp"
    #include "FarSdk/Ascii/farkeys.hpp"
#else
    #include "FarSdk/Unicode/plugin.hpp"
    #include "FarSdk/Unicode/farcolor.hpp"
    #include "FarSdk/Unicode/farkeys.hpp"
	// чтобы плагин работал и в старых версиях
	#define _ACTL_GETFARRECT 32
#endif
#include "menu.h"
#include "libgfl.h"
#include "gflproc.h"
#include "MultiView.h"
#include "regval.h"
#include "ConEmuSupport.h"

//#define GFL_NAME _T("gfl.dl")
#define GFL_DEFAULT_NAME _T("libgfl311.dll")
#define MIN_GFL_VER "3.11"
//#define FAR_VER 0x0146
//#define FAR_BUILD 0x05020000
#define MMVIEW_ENVVAL _T("MMView:")

#ifdef _UNICODE
    #define _tcsscanf swscanf
    #define SETMENUTEXT(itm,txt) itm.Text = txt;
    #define F757NA 0,
    #define _GetCheck(i) (int)Info.SendDlgMessage(hDlg,DM_GETCHECK,i,0)
    #define GetDataPtr(i) ((const TCHAR *)Info.SendDlgMessage(hDlg,DM_GETCONSTTEXTPTR,i,0))
    #define SETTEXT(itm,txt) itm.PtrData = txt
    #define SETTEXTPRINT(itm,fmt,arg) wsprintf(pszBuf, fmt, arg); SETTEXT(itm,pszBuf); pszBuf+=lstrlen(pszBuf)+2;
    #define _tcstoi _wtoi
#else
    #define _tcsscanf sscanf
    #define SETMENUTEXT(itm,txt) lstrcpy(itm.Text, txt);
    #define F757NA
    #define _GetCheck(i) items[i].Selected
    #define GetDataPtr(i) items[i].Data
    #define SETTEXT(itm,txt) lstrcpy(itm.Data, txt)
    #define SETTEXTPRINT(itm,fmt,arg) wsprintf(itm.Data, fmt, arg)
    #define _tcstoi atoi
#endif

HINSTANCE g_hInstance = NULL;
struct PluginStartupInfo Info = {0};
//bool NewFAR;
bool GflLoaded = false;
bool FirstStart = true;
//DWORD Far_version;
FarVersion gFarVersion;
HMODULE GflHandle;
WORD FillColor;
HWND FarWindow = 0, ConEmuWnd = 0, ghPopup = 0, RootWnd = 0;
HANDLE ghConEmuCtrlPressed = NULL, ghConEmuShiftPressed = NULL;
BOOL gbPopupWasHidden = FALSE;
CHAR_INFO* fillbuffer;
BITMAPINFOHEADER BmpHeader;
unsigned char * DibBuffer = 0;
OSVERSIONINFO Version;
TCHAR PlugKey[1000];

//TCHAR HotKey[2];

bool NotUnloadGfl = false;
bool Bilinear = false;
bool gbVideoFullWindow = false;
bool NoSetStartVolume = false;
bool NoGflWarning = true;
DWORD Delay9x = 0;
TCHAR VideoExts[4096];
TCHAR AudioExts[4096];
TCHAR PictureExts[4096];
bool gbWildCardMatch = false;
bool gbVideoExtMatch = false, gbAudioExtMatch = false, gbPictureExtMatch = false;
bool gbForceShowDetectWarn = false;
TCHAR GflFilePathName[MAX_PATH*2];
bool PlugStarted = false, gbPostActivate = false;
#define DELAY9XDEFLT 100
LONGLONG SecondSeeking = 100000000;
#define SECONDSEEK_DEFLT 10
#define SECOND 10000000
#define VIDEO_EXTS _T("asf;avi;m1v;m2v;mp4;mpe;mpeg4;mpeg;mpg;vob;wm;wmd;wmv;flv")
#define AUDIO_EXTS _T("aif;aifc;aiff;au;mid;midi;mp2;mp3;rmi;snd;wav;wma")
#define PICTURE_EXTS _T("bmp;png;gif;jpg;jpeg;tif;tiff;pcx")
long Volume = 0;
bool gbMute = false;
#define VOLUMESTEP 200
DWORD gnIgnorePicturesLarger = 52428800/* 50Mb */; // Если 0 - пытаемся открыть любой не пустой файл

bool PlugActive = true, AllowCtrlShiftF3 = false;

DWORD VidSeekType = AM_SEEKING_AbsolutePositioning;

RECT ConsoleRect;
RECT DCRect;
bool QView;
RECT RangedRect; // на самом деле это не Rect, а {x,y,width,height}
GFL_BITMAP* PGflBitmap = 0;
HANDLE ghFileHandle = NULL;
HANDLE ghMapHandle = NULL;
LPBYTE gpMapData = NULL;
char *PicReady;

TCHAR Drive[_MAX_DRIVE], Dir[_MAX_DIR], Fname[_MAX_FNAME], Ext[_MAX_EXT];
bool gbFromCmdLine = false;
TCHAR PicName[32768];
COLORREF gnBackColor = 0;

void SeconsToHours(LONGLONG time, int* hour, int* min, int* sec);
LONGLONG VideoJump(int duration, int current);

/*
typedef int (WINAPI *FARAPIVIEWERCONTROL)(
  int Command,
  void *Param
);

FARAPIVIEWERCONTROL ViewerControl = 0;

typedef union {
  __int64 i64;
  struct {
    DWORD LowPart;
    LONG  HighPart;
  } Part;
} FARINT64;

struct ViewerMode{
  int UseDecodeTable;
  int TableNum;
  int AnsiMode;
  int Unicode;
  int Wrap;
  int TypeWrap;
  int Hex;
  DWORD Reserved[4];
};

struct ViewerInfo
{
  int    StructSize;
  int    ViewerID;
  const char *FileName;
  FARINT64 FileSize;
  FARINT64 FilePos;
  int    WindowSizeX;
  int    WindowSizeY;
  DWORD  Options;
  int    TabSize;
  struct ViewerMode CurMode;
  int    LeftPos;
  DWORD  Reserved3;
};
*/

//struct Video 
//{
//    // DirectShow interfaces
//    IGraphBuilder   *pGB;
//    IMediaControl   *pMC;
//    IMediaEventEx   *pME;
//    IVideoWindow    *pVW;
//    IBasicAudio     *pBA;
//    IBasicVideo     *pBV;
//    IMediaSeeking   *pMS;
//    IMediaPosition  *pMP;
//    IVideoFrameStep *pFS;
//    //
//    IBaseFilter     *pIn, *pOutA, *pOutV;
//    IPin            *pPin, *pPinA, *pPinV;
//
//    HANDLE          RefreshThread;
//    bool            Initialized;
//    bool            VideoFullWindow;
//
//	TCHAR sCaptionFormat[MAX_PATH];
//	TCHAR sFileName[MAX_PATH]; // ТОЛЬКО имя файла, для заголовка
//	TCHAR sLastCurPos[32];

void Video::ShowPosition()
{
	if (!this) return;
	int prcnt;
	LONGLONG maxpos, curpos;
	int mhour, mmin, msec, chour, cmin, csec;
	TCHAR szCaption[MAX_PATH*2], szCurPos[32], szMaxPos[32];

	pMS->SetTimeFormat( &TIME_FORMAT_MEDIA_TIME );
	HRESULT hr = pMS->GetPositions(&curpos, &maxpos);
	if (SUCCEEDED(hr)) {
		SeconsToHours(curpos/SECOND, &chour, &cmin, &csec); wsprintf(szCurPos, _T("%i:%02i:%02i"), chour, cmin, csec);
		GUID tf;
		pMS->GetTimeFormat(&tf);

		if (lstrcmp(sLastCurPos, szCurPos)) {
			prcnt = (int)(curpos * 100 / maxpos);
			if (prcnt > 100) {
				prcnt = 100;
			} else if (prcnt < 0) {
				prcnt = 0;
			}
			SeconsToHours(maxpos/SECOND, &mhour, &mmin, &msec); wsprintf(szMaxPos, _T("%i:%02i:%02i"), mhour, mmin, msec);
			wsprintf(szCaption, sCaptionFormat/*_T("{%i} %s - Multimedia Viewer")*/, prcnt, sFileName, szCurPos, szMaxPos);
			SetConsoleTitle(szCaption);
		}
	}
};
void Video::SetTimePosition(LONGLONG newtime, int type)
{
	if (!this) return;
	LONGLONG position = 0;
	SetCaption capt(GetMsg(FarPicSeeking));

	pMS->SetTimeFormat( &TIME_FORMAT_MEDIA_TIME );

	if (type == set_time_rel) {
		pMS->GetCurrentPosition(&position);
	}
	position += newtime;

	// what for???
	//pVW->SetWindowForeground(OAFALSE);

	pMS->SetPositions(&position, VidSeekType, 0, AM_SEEKING_NoPositioning);

	// what for???
	//SetForegroundWindow(RootWnd);
};
void Video::JumpRelative(bool abForward, bool abFast)
{
	if (!this) return;
	SetTimePosition((abForward ? 1 : -1) * SecondSeeking * (abFast ? 10 : 1), set_time_rel);
};
void Video::JumpHome()
{
	if (!this) return;
	SetTimePosition(0, set_time_abs);
};
void Video::JumpEnd()
{
	if (!this) return;
	LONGLONG position = 0;
	pMS->GetStopPosition( &position );
	position -= 60;
	SetTimePosition(position, set_time_abs);
};
void Video::PauseResume()
{
	if (!this) return;
	FILTER_STATE state;
	pMC->GetState( INFINITE, (OAFilterState*)&state );
	//*** pVW->SetWindowForeground(OAFALSE);
	if (state == State_Running){
		pMC->Pause();
	} else {
		pMC->Run();
	}
}
void Video::JumpFree()
{
	if (!this) return;
	LONGLONG maxpos, curpos, position;
	FarVideo->pMS->GetPositions(&curpos, &maxpos);

	// Уже должно быть выполнено в OpenPlugin
	//long lVisible = 0, lPopupVisible = 0;
	//if( FarVideo->pVW && FarVideo->pBV && SUCCEEDED(FarVideo->pVW->get_Visible( &lVisible )) && lVisible) {
	//	FarVideo->pVW->put_Visible(OAFALSE);
	//}
	//if (ghPopup && IsWindowVisible(ghPopup)) {
	//	ShowWindow(ghPopup, SW_HIDE);
	//	lPopupVisible = 1;
	//}

	//FILTER_STATE state;
	//pMC->GetState( INFINITE, (OAFilterState*)&state );
	////pVW->SetWindowForeground(OAFALSE); ???
	//if (state == State_Running) {
	//	pVW->SetWindowForeground(OAFALSE);
	//	pMC->Pause();
	//}

	// pVW->SetWindowForeground(OAFALSE);
	SetForegroundWindow(RootWnd);

	position = VideoJump((int)(maxpos/SECOND), (int)(curpos/SECOND));

	if (maxpos >= position && position != 1){
		SetTimePosition(position, set_time_abs);
	}

	//pVW->SetWindowForeground(OAFALSE);
	SetForegroundWindow(RootWnd);

	//if (state == State_Running) {
	//	pMC->Run();
	//}

	// Вообще-то наверное уже не требуется, т.к. "заливка" идет через Popup окно
	//if (FarVideo->RefreshThread) FillBackground(&ConsoleRect);

	// должно быть выполнено в OpenPlugin
	//if (PlugStarted) {
	//	if (lPopupVisible && ghPopup)
	//		ShowWindow(ghPopup, SW_SHOW);
	//	if (lVisible)
	//		FarVideo->pVW->put_Visible(OATRUE);
	//}
}
void Video::SetVolume(MMViewVolume type)
{
	if (!this) return;
	if (type == volume_up) {
		Volume = Volume < VOLUMESTEP ? 0 : Volume - VOLUMESTEP;
		pBA->put_Volume(-Volume);
		SetParam(REG_VOLUME, Volume);
	} else if (type == volume_down) {
		Volume = Volume > 10000-VOLUMESTEP ? 10000 : Volume + VOLUMESTEP;
		pBA->put_Volume(-Volume);
		SetParam(REG_VOLUME, Volume);
	} else if (type == volume_mute) {
		gbMute = !gbMute;
		pBA->put_Volume(gbMute ? -10000 : -Volume);
	}
}
void Video::FitVideo()
{
	if (!this) return;
	long lVisible = 0;
	if (SUCCEEDED(pVW->get_Visible(&lVisible) && lVisible)) {
		long width, height;
		SetParam(REG_FULLWINDOW, gbVideoFullWindow = VideoFullWindow = !VideoFullWindow);
		pBV->get_VideoHeight(&height);
		pBV->get_VideoWidth(&width);

		COORD size = {(short)width, (short)height};

		GetRangedRect(&RangedRect, &size, &VideoFullWindow);
		pVW->SetWindowPosition( RangedRect.left, RangedRect.top, RangedRect.right, RangedRect.bottom );
		// FarVideo->pVW->put_FullScreenMode(OATRUE);
	}
}
//};

Video* FarVideo = 0;

#define JIF(x) if (FAILED(hr=(x))) { DeleteVideo( video ); return 0;}
#define SAFE_RELEASE(x) { if (x) x->Release(); x = NULL; }
HRESULT hr = 0;

int main()
{
	return 0;
}


bool IsKeyPressed(WORD vk)
{
	USHORT st = GetKeyState(vk);
	if ((st & 0x8000))
		return true;

	// что-то в ConEmu GetKeyState иногда ничего не дает? возвращается 0
	if (ConEmuWnd && ghConEmuCtrlPressed && ghConEmuShiftPressed) {
		DWORD dwWait = WAIT_TIMEOUT;
		if (vk == VK_CONTROL)
			dwWait = WaitForSingleObject(ghConEmuCtrlPressed, 0);
		else if (vk == VK_SHIFT)
			dwWait = WaitForSingleObject(ghConEmuShiftPressed, 0);
		return (dwWait == WAIT_OBJECT_0);
	}

	return false;
}

DWORD WINAPI MViewRefreshThread( LPVOID lpParameter )
{
    Video* video = (Video*) lpParameter;
    if( !video )
        return 0;
	// Переменная сбрасывается в DeleteVideo
    bool* pbInitialized = &video->Initialized;
    while( *pbInitialized )
    {
        Sleep( 100 );
        //video->pBV->SetDefaultDestinationPosition();
		video->ShowPosition();
    }

    return 0;
}

void DeleteVideo(Video* video)
{
    if (!video) return;
    video->Initialized = false;
    if (video->RefreshThread){
    	// что за условие? (!ghPopup) ? заменил на (ghPopup)
		if (ghPopup && FarVideo->pVW) {
			FarVideo->pVW->put_Visible(0); // чтобы не мелькало
			FarVideo->pVW->put_Owner(NULL);
		}
		// По идее, нить RefreshThread зависать не должна.
        WaitForSingleObject( video->RefreshThread, INFINITE );
        CloseHandle (video->RefreshThread);
        video->RefreshThread = NULL;
    }
    SAFE_RELEASE(video->pME);
    SAFE_RELEASE(video->pMS);
    SAFE_RELEASE(video->pMP);
    SAFE_RELEASE(video->pMC);
    SAFE_RELEASE(video->pBA);
    SAFE_RELEASE(video->pBV);
    SAFE_RELEASE(video->pVW);
    SAFE_RELEASE(video->pFS);
    SAFE_RELEASE(video->pPin);
	SAFE_RELEASE(video->pPinA);
	SAFE_RELEASE(video->pPinV);
    SAFE_RELEASE(video->pIn);
	SAFE_RELEASE(video->pOutA);
	SAFE_RELEASE(video->pOutV);
    SAFE_RELEASE(video->pGB);
    free( video );
	FillWindowClose();
}

bool MatchExtention( const TCHAR* name, const TCHAR* Exts )
{
	gbWildCardMatch = false;

    if( !name || !Exts || !*name || !*Exts )
        return false;
        
    if (_tcschr(Exts, _T('*'))) {
    	gbWildCardMatch = true;
        return true;
    }

    TCHAR extention[100];
    int len = lstrlen( name );
    int ext_len = lstrlen( Exts );
    int ext_pos = 0;
	const TCHAR *pszFile = _tcsrchr(name, _T('\\')); if (pszFile) pszFile++; else pszFile = name;
	const TCHAR *pszExt = _tcsrchr(pszFile, _T('.'));

    while( ext_pos < ext_len )
    {
        extention[0] = '.';
        extention[1] = 0;
        int i;
        for( i = ext_pos; Exts[i] && Exts[i] != ';' && i - ext_pos < sizeof( extention ) - 2; i++ )
            extention[i - ext_pos + 1] = Exts[i];
		if ((i == ext_pos) || ((i - ext_pos) == 1 && extention[1] == _T('.'))) {
			if (!pszExt)
				return true;
		} else if (pszExt) {
	        extention[i - ext_pos + 1] = 0;
		    if( !lstrcmpi( extention, pszExt /*name + len - lstrlen( extention )*/ ) )
			    return true;
		}

        ext_pos = i + 1;
    }

    return false;
}

Video *NewVideo(const TCHAR* FileName)
{
    if (!FileName || !*FileName) return 0;

    Video *video = (Video*)calloc(sizeof (*video),1);
    if (!video) return 0;

    /*video->pGB = NULL;
    video->pMC = NULL;
    video->pME = NULL;
    video->pVW = NULL;
    video->pBA = NULL;
    video->pBV = NULL;
    video->pMS = NULL;
    video->pMP = NULL;
    video->pFS = NULL;
    video->RefreshThread = NULL;
    video->Initialized = false;*/
    video->VideoFullWindow = gbVideoFullWindow;

	lstrcpyn(video->sCaptionFormat, GetMsg(FarPicCurPos), MAX_PATH);
	const TCHAR *pszFile = _tcsrchr(FileName, _T('\\')); if (pszFile) pszFile++; else pszFile = FileName;
	lstrcpyn(video->sFileName, pszFile, MAX_PATH);
	video->sLastCurPos[0] = 0;

    WCHAR name[1000] = L"";
    #ifndef _UNICODE
    MultiByteToWideChar(CP_OEMCP, 0, FileName, lstrlen(FileName), name, 999);
    #else
    lstrcpyn(name, FileName, 1000);
    #endif

    // Get the interface for DirectShow's GraphBuilder
    JIF(CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void **)&video->pGB));

    if (SUCCEEDED(hr)) {
        // Have the graph builder construct its the appropriate graph automatically
        //#ifndef _DEBUG
        hr = video->pGB->RenderFile(name, NULL);
        //#else
        //hr = VFW_E_CANNOT_RENDER; //VFW_E_CANNOT_CONNECT;
        //#endif
        // Если отрендерить не удалось - попробуем пошаманить
        if (hr == VFW_E_CANNOT_RENDER)
        {
        	// MPEG Source
        	//@device:sw:{083863F1-70DE-11D0-BD40-00A0C911CE86}\{1365BE7A-C86A-473C-9A41-C0A6E82C9FA3}
        	
        	
            hr = video->pGB->AddSourceFilter(name, 
                //L"{E436EBB5-524F-11CE-9F53-0020AF0BA770}",
                //L"{083863F1-70DE-11d0-BD40-00A0C911CE86}\\Instance\\{E436EBB5-524F-11CE-9F53-0020AF0BA770}",
                L"@device:sw:{083863F1-70DE-11d0-BD40-00A0C911CE86}\\{E436EBB5-524F-11CE-9F53-0020AF0BA770}",
				&video->pIn);
            JIF(hr);
            IEnumPins *pEnum = NULL;
            hr = video->pIn->EnumPins(&pEnum);
            if (SUCCEEDED(hr) && pEnum) {
                ULONG nCount = 0;
                hr = pEnum->Next(1, &video->pPin, &nCount);
                if (SUCCEEDED(hr) && nCount && video->pPin) {
                    hr = video->pGB->Render(video->pPin);

					#ifdef _DEBUG
					if (hr == VFW_E_CANNOT_RENDER) {
						//{79376820-07D0-11CF-A24D-0020AFD79767}
						//hr = CoCreateInstance(CLSID_AudioRendererCategory, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void **)&video->pGB)
						hr = CoCreateInstance(CLSID_DSoundRender, NULL, CLSCTX_INPROC_SERVER,
							IID_IBaseFilter/*IID_IBasicAudio*/,
							(void **)&video->pOutA);
						if (hr == S_OK && video->pOutA) {
							hr = video->pGB->AddFilter(video->pOutA, NULL);
							if (hr == S_OK) {
								hr = video->pOutA->EnumPins(&pEnum);
								if (SUCCEEDED(hr) && pEnum) {
									ULONG nCount = 0;
									hr = pEnum->Next(1, &video->pPinA, &nCount);
									if (SUCCEEDED(hr) && nCount && video->pPinA) {
										hr = video->pGB->Connect(video->pPin, video->pPinA);
									}
									SAFE_RELEASE(pEnum);
								}								
							}
						}
					}
					#endif
                } else 
                    hr = VFW_E_CANNOT_RENDER;
                SAFE_RELEASE(pEnum);
            } else 
                hr = VFW_E_CANNOT_RENDER;
                
            //Если рендер не прошел - можно попробовать прицепить к 
            //L"@device:cm:{E0F158E1-CB04-11D0-BD4E-00A0C911CE86}\\Default DirectSound Device"
        }
        JIF(hr);
 
        // QueryInterface for DirectShow interfaces
        JIF(video->pGB->QueryInterface(IID_IMediaControl, (void **)&video->pMC));
        JIF(video->pGB->QueryInterface(IID_IMediaEventEx, (void **)&video->pME));
        JIF(video->pGB->QueryInterface(IID_IMediaSeeking, (void **)&video->pMS));
        JIF(video->pGB->QueryInterface(IID_IMediaPosition, (void **)&video->pMP));

        // Query for video interfaces, which may not be relevant for audio files
        JIF(video->pGB->QueryInterface(IID_IVideoWindow, (void **)&video->pVW));
        JIF(video->pGB->QueryInterface(IID_IBasicVideo, (void **)&video->pBV));

        // Query for audio interfaces, which may not be relevant for video-only files
        JIF(video->pGB->QueryInterface(IID_IBasicAudio, (void **)&video->pBA));

        return video;
    }
    return 0;
}

void WarnNoMedia()
{
    TCHAR* message[] = {GetMsg(FarPicName), _T(""), GetMsg(FarPicWarnNoMedia), _T(""), _T(""), _T(""), _T(""), _T("")};
    int nCount = 4;
    TCHAR *pszErrInfo = NULL, szRc[32];
    if (hr) {
        switch(hr) {
            case VFW_S_AUDIO_NOT_RENDERED:
				message[nCount++] = _T("Partial success; the audio was not rendered."); break;
            case VFW_S_DUPLICATE_NAME:
				message[nCount++] = _T("Success; the Filter Graph Manager modified");
				message[nCount++] = _T("the filter name to avoid duplication."); break;
            case VFW_S_PARTIAL_RENDER:
				message[nCount++] = _T("Some of the streams in this movie");
				message[nCount++] = _T("are in an unsupported format."); break;
            case VFW_S_VIDEO_NOT_RENDERED:
				message[nCount++] = _T("Partial success; some of the streams");
				message[nCount++] = _T("in thismovie are in an unsupported format."); break;
            case E_ABORT:
				message[nCount++] = _T("Operation aborted."); break;
            case E_FAIL:
				message[nCount++] = _T("Failure."); break;
            case E_INVALIDARG:
				message[nCount++] = _T("Argument is invalid."); break;
            case E_OUTOFMEMORY:
				message[nCount++] = _T("Insufficient memory."); break;
            case E_POINTER:
				message[nCount++] = _T("NULL: pointer argument."); break;
            case VFW_E_CANNOT_CONNECT:
				message[nCount++] = _T("No combination of intermediate filters");
				message[nCount++] = _T("could be found to make the connection."); break;
            case VFW_E_CANNOT_LOAD_SOURCE_FILTER:
				message[nCount++] = _T("The source filter for this file could not be loaded."); break;
            case VFW_E_CANNOT_RENDER:
				message[nCount++] = _T("No combination of filters");
				message[nCount++] = _T("could be found to render the stream."); break;
            case VFW_E_INVALID_FILE_FORMAT:
				message[nCount++] = _T("The file format is invalid."); break;
            case VFW_E_NOT_FOUND:
				message[nCount++] = _T("An object or name was not found."); break;
            case VFW_E_UNKNOWN_FILE_TYPE:
				message[nCount++] = _T("The media type of this file is not recognized."); break;
            case VFW_E_UNSUPPORTED_STREAM:
				message[nCount++] = _T("Cannot play back the file:");
				message[nCount++] = _T("the format is not supported."); break;
            default:
                FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, hr, 
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&pszErrInfo, 0, NULL );
                if (pszErrInfo) {
                    message[nCount++] = pszErrInfo;
					TCHAR* psz = pszErrInfo;
					while (*psz) {
						if (*psz == _T('\r') || *psz == _T('\n') || *psz == _T('\t'))
							*psz = _T(' ');
						psz++;
					}
					if (lstrlen(pszErrInfo)>60) {
						psz = pszErrInfo;
						TCHAR *pszEnd = pszErrInfo+lstrlen(pszErrInfo);
						psz += 40;
						while (psz < pszEnd) {
							if (*psz == _T(' ')) {
								*psz = 0;
								message[nCount++] = psz+1;
								if (nCount>=NUM_ITEMS(message)) break;
								psz += 40;
							}
							psz++;
						}
					}
                } else {
                    wsprintf(szRc, _T("ErrCode=0x%08X"), hr);
                    message[nCount++] = szRc;
                }
        }
		if (nCount<NUM_ITEMS(message))
			nCount++;
    }
    Info.Message( Info.ModuleNumber, FMSG_WARNING | FMSG_MB_OK, 0, message, nCount, 0 );
    if (pszErrInfo) GlobalFree(pszErrInfo);
}

void WarnNoPicture()
{
    TCHAR* message[] = {GetMsg(FarPicName), _T(""), GetMsg(FarPicWarnNoPicture), _T("")};
    int nCount = NUM_ITEMS(message);
    Info.Message( Info.ModuleNumber, FMSG_WARNING | FMSG_MB_OK, 0, message, nCount, 0 );
}

bool DetectMedia( const TCHAR* PicName )
{
	bool lbRc = false;
	gbVideoExtMatch = false; gbAudioExtMatch = false; gbPictureExtMatch = false;

	bool bWarned = false;
	gbVideoExtMatch = MatchExtention(PicName, VideoExts);
	gbAudioExtMatch = MatchExtention(PicName, AudioExts);
	gbPictureExtMatch = MatchExtention(PicName, PictureExts);

	// gbForceShowDetectWarn == true при явном вызове плагина, поэтому пытаемся и без расширений...
    if (gbVideoExtMatch || gbAudioExtMatch || (gbForceShowDetectWarn && !gbPictureExtMatch))
    {
		SetCaption capt(GetMsg(FarPicDetectingMedia));
        ReleaseMedia(); // не было
        // Проверяет, чтобы файл был НЕ пустой
        if (FileSizeSmaller((TCHAR*)PicName, 0)) {
            hr = 0;
			if ((FarVideo = NewVideo(PicName)) != NULL) {
				if (!gbVideoExtMatch && !gbAudioExtMatch)
					gbVideoExtMatch = true;
                lbRc = true;
				goto wrap;
			}
            //else
			// Если сдетектилась НЕ как '*' в списке расширений
			//if (!gbWildCardMatch || gbForceShowDetectWarn)
            //    WarnNoMedia();
        }
    }
    
	// gbForceShowDetectWarn == true при явном вызове плагина, поэтому пытаемся и без расширений...
	if (gbPictureExtMatch || gbForceShowDetectWarn)
    {
		SetCaption capt(GetMsg(FarPicDetectingMedia));
        ReleaseMedia(); // не было
        // Не обрабатывать изображения размером больше 50M
        if (FileSizeSmaller((TCHAR*)PicName, gnIgnorePicturesLarger)) {
            LoadGfl();
            hr = 0;
            if (GflLoaded) {
				if (LoadPicture((TCHAR*)PicName, &PGflBitmap)) {
					lbRc = true;
					goto wrap;
				}
                //else if (!gbWildCardMatch || gbForceShowDetectWarn)
                //    WarnNoPicture();
            }
        }
    }

	if (gbForceShowDetectWarn && (gbVideoExtMatch || gbAudioExtMatch || gbPictureExtMatch))
		WarnNoPicture();

wrap:
	gbForceShowDetectWarn = false; // один раз, до следующего вызова через F11
    return lbRc;
}

bool ShowMedia()
{
	// Чтобы не драться с PicView - ставим EnvVar
	SetEnvironmentVariable(_T("FarPicViewMode"), MMVIEW_ENVVAL _T("Displaying"));

	#ifdef _DEBUG
	OutputDebugString(_T("ShowMedia called\n"));
	#endif
	LogFore("ShowMedia.begin");
    if (PGflBitmap && FarGetFname(0, true) && PreparePicture(PGflBitmap)) {
		if (!ghPopup) {
			_ASSERTE(ghPopup);
			ReleaseMedia();
			PlugStarted = false;
			gbFromCmdLine = false;
			return false;
		}
		InvalidateRect(ghPopup, NULL, TRUE);
		LogFore("ShowMedia.picture.ok");
        PlugStarted = true;
        return true;
    } else if (FarVideo && FarGetFname( 0, true )) {
        long lVisible; HRESULT hrVisible = -1;

		//if (!ghPopup && (!gbAudioExtMatch || gbVideoExtMatch)) {
		//	if (!FillWindowCreate(false))
		//	{   // !!! Не удалось создать "скрывающее" окно
		//		ReleaseMedia();
		//		PlugStarted = false;
		//		gbFromCmdLine = false;
		//		return false;
		//	}
		//	LogFore("ShowMedia.popup.created");
		//}

		if (FarVideo->pVW && FarVideo->pBV)
			hrVisible = FarVideo->pVW->get_Visible(&lVisible);

		//if (gbAudioExtMatch && !gbVideoExtMatch) {
		//	FarVideo->m_Type = Video::eAudio;
		//	FarVideo->pMC->Run();
		//} else
        //if (FarVideo->pVW && FarVideo->pBV && SUCCEEDED(hrVisible = FarVideo->pVW->get_Visible(&lVisible))) {
		if (SUCCEEDED(hrVisible)) {
			FarVideo->m_Type = Video::eVideo;

			if (!ghPopup /*&& (!gbAudioExtMatch || gbVideoExtMatch)*/) {
				if (!FillWindowCreate(false))
				{   // !!! Не удалось создать "скрывающее" окно
					ReleaseMedia();
					PlugStarted = false;
					gbFromCmdLine = false;
					return false;
				}
				LogFore("ShowMedia.popup.created");
			}

            long width;
            long height;
            FarVideo->pBV->get_VideoHeight(&height);
            FarVideo->pBV->get_VideoWidth(&width);

            COORD size = {(short)width, (short)height};
            GetRangedRect(&RangedRect, &size, &FarVideo->VideoFullWindow);

            // Run the graph to play the media file
			FarVideo->pVW->put_Owner( (OAHWND)(ghPopup ? ghPopup : FarWindow) );
			if (ghPopup) // Включить форвардинг мышки и клавиатуры в ghPopup
				FarVideo->pVW->put_MessageDrain((OAHWND)ghPopup);
            FarVideo->pVW->put_WindowStyle(WS_CHILD);
			// попробуем?, чтобы не мигало
			FarVideo->pVW->SetWindowPosition( RangedRect.left, RangedRect.top, RangedRect.right, RangedRect.bottom );
            FarVideo->pVW->put_WindowState(SW_SHOWNA);

            FarVideo->pMS->SetTimeFormat( &TIME_FORMAT_MEDIA_TIME );

            FarVideo->pVW->SetWindowPosition( RangedRect.left, RangedRect.top, RangedRect.right, RangedRect.bottom );
            FarVideo->pVW->SetWindowForeground(OAFALSE);

			// 2010-05-22 раньше это делалось каждые 100мс в MViewRefreshThread
			FarVideo->pBV->SetDefaultDestinationPosition();

            // Вообще-то наверное уже не требуется, т.к. "заливка" идет через Popup окно
            //FillBackground(&ConsoleRect);
			LogFore("ShowMedia.pre.run");
            FarVideo->pMC->Run();
        } else {
        	// Сюда мы попадаем если расширения не подошли ни к audio ни к video?
        	_ASSERTE(hrVisible == E_NOINTERFACE);
        	FarVideo->m_Type = Video::eAudio;
            FarVideo->pMC->Run();
        }
		FarVideo->Initialized = true;

        // Нить, обновляющую в заголовке текущее положение и длительность, теперь запускаем и для звука
        if (FarVideo->RefreshThread) {
        	// Сюда мы попадать не должны.
			DWORD dwWait = WaitForSingleObject(FarVideo->RefreshThread,0);
			_ASSERTE(FarVideo->RefreshThread == NULL);
			if (dwWait == WAIT_OBJECT_0) {
				// Нить завершилась, но не был сброшен ее хэндл?
				CloseHandle(FarVideo->RefreshThread);
				FarVideo->RefreshThread = NULL;
			}
		}
		if (!FarVideo->RefreshThread) {
            FarVideo->RefreshThread = CreateThread( 0, 0, MViewRefreshThread, FarVideo, 0, &FarVideo->RefreshThreadId );
            _ASSERTE(FarVideo->RefreshThread!=NULL);
        }
        
		LogFore("ShowMedia.Run()");
		SetForegroundWindow(RootWnd);
        if (!NoSetStartVolume) //2009-09-01
            FarVideo->pBA->put_Volume(-Volume);
		LogFore("ShowMedia.video.ok");
		PlugStarted = true;
        return true;
    }
    return false;
}

void SwitchFullScreen(bool abFullScreen)
{
	if (!FarVideo) return;
	if (!FarVideo->pVW) return;

	long lFullScreen = 0, lNew = 0, ll;
	HRESULT hr = 0;
	if ((hr = FarVideo->pVW->get_FullScreenMode(&lFullScreen)) == S_OK) {
		lNew = (lFullScreen == OAFALSE) ? OATRUE : OAFALSE;
		hr = FarVideo->pVW->put_FullScreenMode(lNew);
		FarVideo->pVW->get_FullScreenMode(&ll);
	}
}

bool ReleaseMedia()
{
	// Чтобы не драться с PicView - ставим EnvVar
	TCHAR szValue[128];
	if (GetEnvironmentVariable(_T("FarPicViewMode"), szValue, 128)) {
		szValue[7] = 0;
		if (lstrcmp(szValue, MMVIEW_ENVVAL) == 0) {
			SetEnvironmentVariable(_T("FarPicViewMode"), NULL);
		}
	}

    if (FarVideo) {
        DeleteVideo( FarVideo );
        FarVideo = 0;
    }

	if (ghPopup) {
		SendMessage(ghPopup,WM_CLOSE,0,0);
		_ASSERTE(ghPopup==NULL);
	}
    // Сама разберется и буфер сбросит
    ReleasePicture(&PGflBitmap);
    //
	if (DibBuffer) {
		free(DibBuffer); DibBuffer = 0;
	}

    return true;
}

// ----------------------- Code Start ---------------------------------

// --------------------------------------------------------------------
// Поиск и загрузка длл-ки gfl.dll
bool LoadGflLibrary(){
    TCHAR gflpath[MAX_PATH*3];
    TCHAR* pszSlash;

	lstrcpyn(gflpath, ::Info.ModuleName, NUM_ITEMS(gflpath));
	pszSlash = (TCHAR*)_tcsrchr(gflpath, L'\\');
	if (!pszSlash) pszSlash = gflpath; else pszSlash++;

    // Если указан путь в реестре - грузим его
    if (GflFilePathName[0]) {
    	if (_tcschr(GflFilePathName, L'\\')) {
    		// Указан полный путь
    		lstrcpyn(gflpath, GflFilePathName, NUM_ITEMS(gflpath));
    	} else {
    		// Указано только имя файла
    		lstrcpyn(pszSlash, GflFilePathName, (int)NUM_ITEMS(gflpath)-(int)(pszSlash-gflpath));
    	}
    	if ((GflHandle = LoadLibrary(gflpath)) != NULL)
    		return true;
    }

    lstrcpyn(gflpath, ::Info.ModuleName, NUM_ITEMS(gflpath));
	lstrcpyn(pszSlash, GFL_DEFAULT_NAME, (int)NUM_ITEMS(gflpath)-(int)(pszSlash-gflpath));
    
    if ((GflHandle = LoadLibrary(gflpath)) != NULL)
        return true;

    // Пробуем по имени
    if ((GflHandle = LoadLibrary(GFL_DEFAULT_NAME)) != NULL)
        return true;

    return false;
}

// -------------------------------------------------------------------
// Получаем адреса нужных нам функций
bool EnumerateGflFunct(){
    if (!GflLoaded) return (false);
    for (int i = 0; i < NUM_ITEMS(GflProcNames); i++){
        GflProc[i] = (gfl)GetProcAddress(GflHandle, GflProcNames[i]);
        if (GflProc[i] == NULL) {
            return (false);
        }
    }
    return(true);
}

// -------------------------------------------------------------------
// Версия библиотеки должна быть не ниже чем ...
bool TestGflVersion(){
    const char *version;

    version = ((PVgflGetVersionT)GflProc[PVgflGetVersion])();
    return (lstrcmpA(MIN_GFL_VER, version) <= 0);
}

// -------------------------------------------------------------------
// Пытаемся загрузить библиотеку
int LoadGfl(void){
    int exitcode;
    if (GflLoaded) return (PVGflLoadOK);
    
    bool unload = false;

    if (LoadGflLibrary()){
        GflLoaded = true;
        if (EnumerateGflFunct()){
            if (TestGflVersion()) {
                if (((PVgflLibraryInitT)GflProc[PVgflLibraryInit])() == GFL_NO_ERROR) {
                    ((PVgflEnableLZWT)GflProc[PVgflEnableLZW])(GFL_TRUE);
                    exitcode = PVGflLoadOK;
                } else {
                    exitcode = PVGflLoadNotInit;
                    unload = true;
                }
            } else {
                exitcode = PVGflLoadLowVersion;
                unload = true;
            }
        } else {
            unload = true;
            exitcode = PVGflLoadDamaged;
        }
    } else {
        exitcode = PVGflLoadNotLoaded;
    }
    if (unload) {
        UnLoadGfl();
    }
    
    static bool NoWarning = false;
    if (!NoWarning && exitcode != PVGflLoadOK){
        if (!NoGflWarning)
            Info.Message(Info.ModuleNumber, FMSG_ALLINONE|FMSG_MB_OK|FMSG_WARNING, NULL, (const TCHAR * const *)GetMsg(FarPicGflErrorLoad + exitcode - 1), 0,0);
        NoWarning = true;
    }
    GflLoaded = (exitcode == PVGflLoadOK);
    return (exitcode);
}


// -------------------------------------------------------------------
// Выгружаем библиотеку
bool UnLoadGfl(void){
    bool exitcode;

    if (GflLoaded){
        GflProc[PVgflLibraryExit]();
        if (FreeLibrary(GflHandle) != 0){
            GflLoaded = false;
            exitcode = true;
        } else {
            exitcode = false;
        }
    } else {
        exitcode = true;
    }
    return (exitcode);
}


// -------------------------------------------------------------------
// Получаем RECT куда надо пместить картинку
void GetFarRect()
{
    RECT rect;
    CONSOLE_SCREEN_BUFFER_INFO info;
    
    GetClientRect(FarWindow, &rect);
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);

//#ifdef VS90_FLOAT
	// При генерации в VS90 здесь требуется __ftol2_sse, что не позволяет использовать старый runtime
	#define ROUND(x) x=(x>(int)x)?((int)x+1):x

    double dx = (double)(rect.right - rect.left) / (info.srWindow.Right - info.srWindow.Left + 1);
    double dy = (double)(rect.bottom - rect.top) / (info.srWindow.Bottom - info.srWindow.Top + 1);
    ROUND(dx);
    ROUND(dy);

    DCRect.left   = (long)(dx * (ConsoleRect.left - info.srWindow.Left));
    DCRect.right  = (long)(dx * (ConsoleRect.right + 1 - info.srWindow.Left));
    DCRect.top    = (long)(dy * (ConsoleRect.top - info.srWindow.Top));
    DCRect.bottom = (long)(dy * (ConsoleRect.bottom + 1 - info.srWindow.Top));
//#else
//	// Это не прокатывает. Когда фар распахнут на весь экран размер консоли чуть-чуть меньше - пара пикселей не помещается...
//	long dx = (rect.right - rect.left) / (info.srWindow.Right - info.srWindow.Left + 1);
//	long dy = (rect.bottom - rect.top) / (info.srWindow.Bottom - info.srWindow.Top + 1);
//
//    DCRect.left   = (long)(dx * (ConsoleRect.left - info.srWindow.Left));
//    DCRect.right  = (long)(dx * (ConsoleRect.right + 1 - info.srWindow.Left));
//    DCRect.top    = (long)(dy * (ConsoleRect.top - info.srWindow.Top));
//    DCRect.bottom = (long)(dy * (ConsoleRect.bottom + 1 - info.srWindow.Top));
//#endif
}

// Проверяет, чтобы файл был НЕ пустой
// Если (maxsize > 0) - проверяет, чтобы файл не превышал maxsize
bool FileSizeSmaller(TCHAR *name, unsigned long maxsize){
    bool result = false;
    HANDLE h;

    WIN32_FIND_DATA fdata;
    if ((h = FindFirstFile(name, &fdata)) != INVALID_HANDLE_VALUE) {
        FindClose(h);
        if (fdata.nFileSizeLow == 0 && fdata.nFileSizeHigh == 0) {
            result = false; // Не обрабатывать пустые файлы
        } else if (maxsize == 0) {
            result = true; //  просто проверка, чтобы файл был не пустой
        } else {
            result = (fdata.nFileSizeLow < maxsize) && (fdata.nFileSizeHigh == 0);
        }
    }
    return (result);
}

// -------------------------------------------------------------------
// Проверка, можно ли рисовать картинку и если да, получаем координаты
// области, куда будем рисовать
bool FarGetFname(TCHAR *PicName, bool DetectCoordOnly){
    //if (gbFromCmdLine)
    //    return true;

    struct WindowInfo winfo = {0};
    struct PanelInfo panel = {0};

    winfo.Pos = -1;
    #ifdef _UNICODE
    wchar_t szWNameBuffer[0x2000];
    winfo.Name = szWNameBuffer;
    winfo.NameSize = NUM_ITEMS(szWNameBuffer);
    #endif
    Info.AdvControl(Info.ModuleNumber, ACTL_GETWINDOWINFO, (void*)&winfo);

    switch (winfo.Type) { // просмотреть можно только из Viewer'а или QuickView
        case WTYPE_VIEWER : {
            QView = false;
            HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
            CONSOLE_SCREEN_BUFFER_INFO screen;
            GetConsoleScreenBufferInfo( out, &screen );
            //RECT rect = { 0, 1, screen.dwSize.X - 1, screen.dwSize.Y - 2 };
			RECT rect = { 0, 0, screen.dwSize.X - 1, screen.dwSize.Y - 1 };
			#ifdef _UNICODE
			// FAR2 с ключом /w может работать с длинным буфером, а отображается только в видимой области
			if (gFarVersion.dwVerMajor > 2 || (gFarVersion.dwVerMajor == 2 && gFarVersion.dwBuild >= 1573))
			{
				SMALL_RECT rcVisible;
				if (Info.AdvControl(Info.ModuleNumber, _ACTL_GETFARRECT, (void*)&rcVisible)) {
					rect.left = rcVisible.Left; rect.top = rcVisible.Top;
					rect.right = rcVisible.Right; rect.bottom = rcVisible.Bottom;
				}
			}
			#endif
            ConsoleRect = rect;

            FillColor = (WORD)(0xFFFF & Info.AdvControl(Info.ModuleNumber, ACTL_GETCOLOR, (void*)COL_VIEWERTEXT));
            break;
        }
        case WTYPE_PANELS : {
            //int panels[] = {FCTL_GETANOTHERPANELINFO, FCTL_GETPANELINFO};
            bool qview = false;
            for (int i = 0; i < 2 && !qview; i++){
                #ifndef _UNICODE
                Info.Control(INVALID_HANDLE_VALUE, (i==0) ? FCTL_GETANOTHERPANELINFO : FCTL_GETPANELINFO, &panel);
                #else
                Info.Control((i==0) ? PANEL_PASSIVE : PANEL_ACTIVE, FCTL_GETPANELINFO, F757NA (LONG_PTR)&panel);
                #endif
                qview = (panel.PanelType == PTYPE_QVIEWPANEL && panel.Visible);
            }
            if (qview) {
                QView = true;
                FillColor = (WORD)(0xFFFF & Info.AdvControl(Info.ModuleNumber, ACTL_GETCOLOR, (void*)COL_PANELTEXT));

				SMALL_RECT rcVisible = {0,0};
				#ifdef _UNICODE
				// FAR2 с ключом /w может работать с длинным буфером, а отображается только в видимой области
				if (gFarVersion.dwVerMajor > 2 || (gFarVersion.dwVerMajor == 2 && gFarVersion.dwBuild >= 1573))
				{
					if (!Info.AdvControl(Info.ModuleNumber, _ACTL_GETFARRECT, (void*)&rcVisible)) {
						rcVisible.Left = rcVisible.Top = 0;
					}
				}
				#endif
                ConsoleRect.top    = panel.PanelRect.top    + 1 + rcVisible.Top;
                ConsoleRect.bottom = panel.PanelRect.bottom - 3 + rcVisible.Top;
                ConsoleRect.left   = panel.PanelRect.left   + 1 + rcVisible.Left;
                ConsoleRect.right  = panel.PanelRect.right  - 1 + rcVisible.Left;
            } else {
                return (false); // если панели и нет панели QuickView, то активизироваться не надо
            }
            break;
        }
        default : { // если нет, то активизироваться не надо
            return (false);
        }
    }
    if (lstrlen(winfo.Name) > _MAX_PATH) return (false); // если имя файла больше _MAX_PATH, то активизироваться не надо

    if( !DetectCoordOnly && PicName )
        lstrcpy(PicName, winfo.Name);
    FillColor = (FillColor & 0xf0) | ((FillColor & 0xf0) >> 4);
    GetFarRect();
    return (true);
}

// -------------------------------------------------------------------
// Загружаем картинку с помощью gfl
bool LoadPicture(TCHAR * PicName, GFL_BITMAP** ppGflBitmap)
{
    bool exitcode = false;
    GFL_LOAD_PARAMS load_params;

    _ASSERTE(!ghFileHandle && !ghMapHandle && !gpMapData);
    const GFL_UINT8* data = NULL;
    GFL_UINT32 data_length = 0;
    DWORD nSizeLow = 0, nSizeHigh = 0;
    //
    ghFileHandle = CreateFile(PicName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if (ghFileHandle != INVALID_HANDLE_VALUE) {
	    nSizeLow = GetFileSize(ghFileHandle, &nSizeHigh);
	    if ((nSizeHigh == 0) && (nSizeLow != INVALID_FILE_SIZE)) {
			if ((ghMapHandle = CreateFileMapping(ghFileHandle, NULL, PAGE_READONLY, 0, 0, NULL)) != NULL)
				gpMapData = (LPBYTE)MapViewOfFile(ghMapHandle, FILE_MAP_READ, 0, 0, 0);
		}
	}
	if (!gpMapData) {
		ReleasePicture(NULL);
		return false;
	}



    ((PVgflGetDefaultLoadParamsT)GflProc[PVgflGetDefaultLoadParams])(&load_params);
    load_params.Flags |= GFL_LOAD_SKIP_ALPHA; 
    load_params.Origin = GFL_BOTTOM_LEFT; 
    load_params.ColorModel = GFL_BGR; 
    load_params.LinePadding = 4; 

    exitcode = (((PVgflLoadBitmapFromMemoryT)GflProc[PVgflLoadBitmapFromMemory])(
    	(const GFL_UINT8*)gpMapData, nSizeLow, ppGflBitmap, &load_params, NULL) == GFL_NO_ERROR);

    return (exitcode);
}

//void FillBackground(RECT *ConsoleRect){
//	if (ghPopup && IsWindow(ghPopup))
//		return;
//
//    COORD size = {(SHORT)(ConsoleRect->right - ConsoleRect->left + 1), (SHORT)(ConsoleRect->bottom - ConsoleRect->top + 1)};
//    COORD start = {0, 0};
//    SMALL_RECT rect = {(short)ConsoleRect->left, (short)ConsoleRect->top, (short)ConsoleRect->right, (short)ConsoleRect->bottom};
//    
//    fillbuffer = (CHAR_INFO*)malloc(size.X * size.Y * sizeof(*fillbuffer));
//    CHAR_INFO templ;
//    templ.Attributes = FillColor;
//    templ.Char.UnicodeChar = 1;
//    for (int i = 0; i < size.X * size.Y; i++){
//        fillbuffer[i] = templ;
//    }
//    WriteConsoleOutput(GetStdHandle(STD_OUTPUT_HANDLE), fillbuffer, size, start, &rect);
//    free(fillbuffer);
//}

void GetRangedRect(RECT *pRangedRect, const COORD* coord, const bool* VideoFullWindow)
{
    int width = coord ? coord->X : PGflBitmap->Width;
    int height = coord ? coord->Y : PGflBitmap->Height;

    if (VideoFullWindow && *VideoFullWindow) {
        int sx, sy;
        sx = DCRect.right - DCRect.left;
        sy = DCRect.bottom - DCRect.top;
        if (sy/height > sx/width){
            height = height*sx/width;
            width = sx;
        } else {
            width = width*sy/height;
            height = sy;
        }
    }

    float asp_dst = (float)(DCRect.right - DCRect.left) / (float)(DCRect.bottom - DCRect.top);
    float asp_src = (float)width / (float)height;
    
    int dst_w;
    int dst_h;


    if (asp_dst < asp_src){
        dst_w = min(DCRect.right - DCRect.left, width);
		//#ifdef VS90_FLOAT
			dst_h = (int)(((float)dst_w) / asp_src);
		//#else
		//	dst_h = dst_w * (__int64)height / width;
		//#endif
    } else {
        dst_h = min( DCRect.bottom - DCRect.top, height );
		//#ifdef VS90_FLOAT
			dst_w = (int)(asp_src * dst_h);
		//#else
		//	dst_w = width * (__int64)dst_h / height;
		//#endif
    }

    RECT dest = {DCRect.left, DCRect.top, dst_w, dst_h};
    (*pRangedRect) = dest;

    int shift;
    if ((shift = DCRect.right - DCRect.left - pRangedRect->right + 1) > 0 ){
        pRangedRect->left += shift/2;
    }
    if ((shift = DCRect.bottom - DCRect.top - pRangedRect->bottom + 1) > 0){
        pRangedRect->top += shift/2;
    }

    _ASSERTE(ghPopup!=NULL);
    if (ghPopup) {
		POINT pt = {0,0}; ClientToScreen(ghPopup, &pt);
		ScreenToClient(FarWindow, &pt);
		pRangedRect->left -= pt.x;
		pRangedRect->top -= pt.y;
	}
}

void ExtractPicture(GFL_BITMAP *bitmap, BITMAPINFOHEADER * pBmpHeader, unsigned char **data ){
    int i, j, bytes_per_line; 
    unsigned char *ptr_src, *ptr_dst; 
    
    *data = NULL;
    
    memset(pBmpHeader, 0, sizeof(BITMAPINFOHEADER) ); 
    
    pBmpHeader->biSize   = sizeof(BITMAPINFOHEADER);
    pBmpHeader->biWidth  = bitmap->Width; 
    pBmpHeader->biHeight = bitmap->Height; 
    pBmpHeader->biPlanes = 1; 
    
    bytes_per_line = (bitmap->Width * 3 + 3) & -4; 
    pBmpHeader->biClrUsed = 0;
    pBmpHeader->biBitCount = 24; 
    pBmpHeader->biCompression = BI_RGB; 
    pBmpHeader->biSizeImage = bytes_per_line * bitmap->Height; 
    pBmpHeader->biClrImportant = 0; 
    
    *data = (unsigned char*)malloc(pBmpHeader->biSizeImage); 
    _ASSERTE(*data!=NULL);
    if (!*data)
    	return;
    
    if ( bitmap->Type != GFL_BINARY && bitmap->Type != GFL_GREY
        && bitmap->Type != GFL_COLORS && bytes_per_line == (int)bitmap->BytesPerLine ){
        memcpy( *data, bitmap->Data, pBmpHeader->biSizeImage );
    } else {
    
        for ( i = 0; i < bitmap->Height; i++ ){
            ptr_src = bitmap->Data + i * bitmap->BytesPerLine; 
            ptr_dst = *data + i * bytes_per_line; 

            switch (bitmap->Type){
                case GFL_BINARY : {
                    for ( j=0; j<bitmap->Width; j++ ){
/*                      ptr_dst[0] = ptr_dst[1] = ptr_dst[2] = *ptr_src ? 255 : 0;
                        ptr_dst += 3;
                        ptr_src++;*/
                    *ptr_dst = *(ptr_dst + 1) = *(ptr_dst + 2) =
                        (ptr_src[j / 8] & (0x80 >> (j % 8))) ? 255 : 0;
                    ptr_dst += 3;
                    } break;
                }
                case GFL_GREY : {
                    for ( j=0; j<bitmap->Width; j++ ){
                        *ptr_dst++ = *ptr_src; 
                        *ptr_dst++ = *ptr_src; 
                        *ptr_dst++ = *ptr_src++; 
                    } break;
                }
                case GFL_COLORS : {
                    for ( j=0; j<bitmap->Width; j++ ){
                        *ptr_dst++ = bitmap->ColorMap->Blue[ *ptr_src ]; 
                        *ptr_dst++ = bitmap->ColorMap->Green[ *ptr_src ]; 
                        *ptr_dst++ = bitmap->ColorMap->Red[ *ptr_src++ ]; 
                    } break;
                }
                case GFL_RGB : {
                    for ( j=0; j<bitmap->Width; j++ ){
                        *ptr_dst++ = ptr_src[0]; 
                        *ptr_dst++ = ptr_src[1]; 
                        *ptr_dst++ = ptr_src[2]; 
                        ptr_src += bitmap->BytesPerPixel; 
                    } break;
                }
                default : {
                    for (j=0; j<bitmap->Width; j++){
                        *ptr_dst++ = ptr_src[2]; 
                        *ptr_dst++ = ptr_src[1]; 
                        *ptr_dst++ = ptr_src[0]; 
                        ptr_src += bitmap->BytesPerPixel; 
                    }
                }
            }
        }
    }
}

bool PreparePicture(GFL_BITMAP* pGflBitmap){
    bool exitcode = false;
    GFL_BITMAP* pic = NULL;

	if (FillWindowCreate(false)) {
		GetRangedRect(&RangedRect, 0, &gbVideoFullWindow);
	    ((PVgflResizeT)GflProc[PVgflResize])(pGflBitmap, &pic, RangedRect.right, RangedRect.bottom, 
	    	Bilinear ? GFL_RESIZE_BILINEAR : GFL_RESIZE_QUICK, 0);
	    if (pic != NULL) {
	    	_ASSERTE(DibBuffer==NULL);
	        ExtractPicture(pic, &BmpHeader, &DibBuffer);
	        ReleasePicture(&pic);
	        exitcode = (DibBuffer!=NULL);
	    }
	    if (!exitcode) {
	    	FillWindowClose();
	    }
    }

    return (exitcode);
}

void ReleasePicture(GFL_BITMAP** ppGflBitmap)
{
	if (ppGflBitmap && *ppGflBitmap) {
    	((PVgflFreeBitmapT)GflProc[PVgflFreeBitmap])(*ppGflBitmap); *ppGflBitmap = NULL;
    }
    if (gpMapData) {
    	UnmapViewOfFile((void*)gpMapData); gpMapData = NULL;
    }
    if (ghMapHandle) {
    	CloseHandle(ghMapHandle); ghMapHandle = NULL;
    }
    if (ghFileHandle) {
    	CloseHandle(ghFileHandle); ghFileHandle = NULL;
    }
}

bool GetParam(TCHAR* valname, DWORD* value, DWORD deflt){
    HKEY key = NULL;
    DWORD size = sizeof (*value);

    *value = deflt;
    if (RegOpenKeyEx(HKEY_CURRENT_USER, PlugKey, 0, KEY_READ, &key) == ERROR_SUCCESS){
        if (RegQueryValueEx(key, valname, 0, NULL, (unsigned char *)value, &size ) != ERROR_SUCCESS){
            *value = deflt;
        }
        RegCloseKey(key);
    }
    return (true);
}

bool GetStrParam(TCHAR* valname, TCHAR* value, TCHAR* deflt, DWORD size){
    HKEY key = NULL;

    lstrcpy(value, deflt);
    if (RegOpenKeyEx(HKEY_CURRENT_USER, PlugKey, 0, KEY_READ, &key) == ERROR_SUCCESS){
        if (RegQueryValueEx(key, valname, 0, NULL, (unsigned char *)value, &size ) != ERROR_SUCCESS){
            lstrcpy(value, deflt);
        }
        RegCloseKey(key);
    }
    return (true);
}

void SetParam(const TCHAR* keyname, DWORD value){
    HKEY key;
    
    if (RegCreateKey(HKEY_CURRENT_USER, PlugKey, &key ) == ERROR_SUCCESS) {
        RegSetValueEx(key, keyname, 0, REG_DWORD, (unsigned char*)&value, sizeof(value) );
        RegCloseKey( key );
    }
}

void SetStrParam(const TCHAR* keyname, const TCHAR* value){
    HKEY key;
    
    if (RegCreateKey(HKEY_CURRENT_USER, PlugKey, &key ) == ERROR_SUCCESS) {
        int nSize = (lstrlen(value)+1)*sizeof(TCHAR);
        RegSetValueEx(key, keyname, 0, REG_SZ, (unsigned char*)value, nSize );
        RegCloseKey( key );
    }
}

void FindParentHwnd()
{
    ConEmuWnd = FarWindow = GetConEmuHwnd();
	if (ConEmuWnd) {
		RootWnd = GetParent(ConEmuWnd);
		TCHAR szName[64]; DWORD dwPID = GetCurrentProcessId();
		if (!ghConEmuCtrlPressed) {
			wsprintf(szName, CEKEYEVENT_CTRL, dwPID);
			ghConEmuCtrlPressed = OpenEvent(SYNCHRONIZE, FALSE, szName);
		}
		if (!ghConEmuShiftPressed) {
			wsprintf(szName, CEKEYEVENT_SHIFT, dwPID);
			ghConEmuShiftPressed = OpenEvent(SYNCHRONIZE, FALSE, szName);
		}
	} else {
        FarWindow = (HWND)::Info.AdvControl( ::Info.ModuleNumber, ACTL_GETFARHWND, 0);
		RootWnd = FarWindow;
		if (ghConEmuCtrlPressed) { CloseHandle(ghConEmuCtrlPressed); ghConEmuCtrlPressed = NULL; }
		if (ghConEmuShiftPressed) { CloseHandle(ghConEmuShiftPressed); ghConEmuShiftPressed = NULL; }
	}
}

bool PluginInit(){
    if (FirstStart) {
        Version.dwOSVersionInfoSize = sizeof (Version);
        GetVersionEx(&Version);
        if (Version.dwPlatformId == VER_PLATFORM_WIN32_NT){
			FindParentHwnd();
        } else {
            FarWindow = FindWindowExA( FindWindowA( "tty", 0 ), NULL, "ttyGrab", NULL );
			RootWnd = FarWindow;
        }
        FirstStart = false;
    }
    ReadConfig();
    return (FarWindow != NULL);
}

void ReadConfig(){
    DWORD val;
    GetParam(REG_PLUGACTIVE, &val, 1);
    PlugActive = !(val == 0);
	GetParam(REG_ALLOWCTRLSHIFTF3, &val, 0);
	AllowCtrlShiftF3 = !(val == 0);
    GetParam(REG_NOTUNLOADGFL, &val, 0);
    NotUnloadGfl = !(val == 0);
    GetParam(REG_BILINEAR, &val, 0);
    Bilinear = !(val == 0);
    GetParam(REG_VIDSEEK, &val, 0);
    VidSeekType = val ? AM_SEEKING_AbsolutePositioning | AM_SEEKING_SeekToKeyFrame : AM_SEEKING_AbsolutePositioning;

    GetParam(REG_DELAY9X, &Delay9x, DELAY9XDEFLT);
    GetParam(REG_VOLUME, (unsigned long*)&Volume, 0);
    GetParam(REG_VIDEOSEEKINTERVAL, &val, 0);
    SecondSeeking = val ? val * SECOND : SECONDSEEK_DEFLT * SECOND;
    
    GetStrParam(REG_VIDEOEXTS, VideoExts, VIDEO_EXTS, sizeof(VideoExts));
    //if (!VideoExts[0]) lstrcpyn(VideoExts, VIDEO_EXTS, sizeof(VideoExts)/sizeof(TCHAR));
    
    GetStrParam(REG_AUDIOEXTS, AudioExts, AUDIO_EXTS, sizeof(AudioExts));
    //if (!AudioExts[0]) lstrcpyn(AudioExts, AUDIO_EXTS, sizeof(AudioExts)/sizeof(TCHAR));
    
    GetStrParam(REG_PICTUREEXTS, PictureExts, PICTURE_EXTS, sizeof(PictureExts));
    GetParam(REG_FULLWINDOW, &val, 0);
    gbVideoFullWindow = !(val == 0);

    GetParam(REG_NOSTARTVOL, &val, 0);
    NoSetStartVolume = !(val == 0);
    GetParam(REG_NOGFLWARN, &val, 1);
    NoGflWarning = !(val == 0);

    GetStrParam(REG_GFLPATH, GflFilePathName, _T(""), sizeof(GflFilePathName));
	if (_tcschr(GflFilePathName, _T('%'))) {
		TCHAR sPath[MAX_PATH*2];
		DWORD nExp = ExpandEnvironmentStrings(GflFilePathName, sPath, MAX_PATH*2);
		if (nExp && nExp <= MAX_PATH*2)
			_tcscpy(GflFilePathName, sPath);
		else if (nExp && nExp > MAX_PATH*2)
			GflFilePathName[0] = 0; // слишком длинный путь - игнорируем
	}
}

TCHAR *GetMsg(int MsgId) {
    return((TCHAR *)Info.GetMsg(Info.ModuleNumber, MsgId));
}

int WINAPI _export GetMinFarVersion(void){
    return FARMANAGERVERSION;
}

//void WarnHotKey(void){
//    const TCHAR *Msg[] = {
//        GetMsg(FarPicName),
//        _T(""),
//        GetMsg(FarPicWarnHotKeyText1),
//        GetMsg(FarPicWarnHotKeyText2),
//        _T(""),
//    };
//    ::Info.Message(Info.ModuleNumber, FMSG_WARNING|FMSG_MB_OK, NULL,  Msg, NUM_ITEMS(Msg), 0);
//}

//TCHAR GetHotKey(void)
//{
//    TCHAR HotKeyPath[1000];
//    lstrcpy(HotKeyPath, ::Info.RootKey);
//    lstrcpy(HotKeyPath + lstrlen(HotKeyPath) - 7, _T("PluginHotkeys\\"));
//
//    TCHAR ModuleName[MAX_PATH];
//    lstrcpyn(ModuleName, ::Info.ModuleName, MAX_PATH);
//    for (TCHAR *temp = ModuleName; temp[0] != 0; temp++){
//        if (temp[0] == _T('\\')) temp[0] = _T('/');
//    }
//
//    HKEY key = NULL;
//    if (RegOpenKeyEx(HKEY_CURRENT_USER, HotKeyPath, 0, KEY_READ, &key) == ERROR_SUCCESS){
//        DWORD mn = lstrlen(ModuleName);
//
//        DWORD size;
//        TCHAR PlugHotKey[NM];
//        FILETIME time;
//
//        for (DWORD i=0; size = NM, ERROR_SUCCESS == RegEnumKeyEx(key, i, PlugHotKey, &size, NULL, NULL, NULL, &time); i++){
//            if (!::Info.FSF->LStricmp(ModuleName + (mn > size ? mn - size : 0), PlugHotKey)) {
//                i = 0xFFFFFFFE;
//                HKEY k = NULL; DWORD size = sizeof(HotKey);
//                lstrcat(HotKeyPath, PlugHotKey);
//
//                if (RegOpenKeyEx(HKEY_CURRENT_USER, HotKeyPath, 0, KEY_READ, &k) == ERROR_SUCCESS){
//                    if (RegQueryValueEx(k, _T("Hotkey"), 0, NULL, (unsigned char*)HotKey, &size) != ERROR_SUCCESS){
//                        HotKey[0] = 0;
//                    }
//                    RegCloseKey(k);
//                } else {
//                    HotKey[0] = 0;
//                }
//            }
//        }
//        RegCloseKey(key);
//    }
//    return (HotKey[0] != 0);
//}

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD fdwReason, LPVOID lpvReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
		g_hInstance = hInstance;
	return TRUE;
}

void WINAPI _export SetStartupInfo(struct PluginStartupInfo *Info) {
    ::Info=*Info;
    //NewFAR=false;
    if(Info->StructSize >= sizeof(struct PluginStartupInfo)){
        ::Info.AdvControl(::Info.ModuleNumber, ACTL_GETFARVERSION, &gFarVersion.dwRAW);
        //if ( !((Far_version & 0xffff) < FAR_VER ||  (Far_version & 0xffff) == FAR_VER && (Far_version & 0xffff0000) < FAR_BUILD) ) {
        lstrcpy(PlugKey, ::Info.RootKey);
        lstrcat(PlugKey, PLUG_KEY);
        
        if (!IsTerminalMode()) {
        
			// ХотКей для работы плагина больше не требуется - только для макросов, на усмотрение пользователя
            //if (!GetHotKey()) WarnHotKey();

            //NewFAR = true;
            CoInitialize( NULL );

            //ViewerControl = (FARAPIVIEWERCONTROL)(Info->Reserved[1]);
            PluginInit();
            
        }
        //}
    }
}

void WINAPI _export GetPluginInfo(struct PluginInfo *Info) {
    if (!IsTerminalMode())
    {
        static TCHAR *PluginMenuStrings[1];
        PluginMenuStrings[0] = GetMsg(FarPicName);
        
        Info->StructSize=sizeof(*Info);
        Info->PluginMenuStrings = PluginMenuStrings;
        Info->PluginMenuStringsNumber = NUM_ITEMS(PluginMenuStrings);
        Info->PluginConfigStrings = PluginMenuStrings;
        Info->PluginConfigStringsNumber = NUM_ITEMS(PluginMenuStrings);
        Info->CommandPrefix = _T("mmview");
        Info->Flags = PF_VIEWER; //|PF_DISABLEPANELS;
        
        #ifdef _UNICODE
        Info->Reserved = 'MMVW';
        #endif
    }
}

void PostMacros()
{
	//DWORD keys[] = {KEY_F11, HotKey[0], KEY_SHIFT};
	//struct KeySequence keyseq = {0, NUM_ITEMS(keys), keys};
	//
	//if (HotKey[0])
	//	Info.AdvControl(Info.ModuleNumber, ACTL_POSTKEYSEQUENCE, &keyseq);
	ActlKeyMacro macro = {MCMD_POSTMACROSTRING};
	TCHAR szMacro[1024];
	const TCHAR *pszName = GetMsg(FarPicName);
	const TCHAR *pszErr = GetMsg(FarPicPluginNotFound);
	wsprintf(szMacro, _T("F11 $if (menu.select(\"%s\", 2) > 0) Enter $else MsgBox(\"%s\",\"%s\",0x10001) $end"),
		pszName, pszName, pszErr);		
	macro.Param.PlainText.SequenceText = szMacro;
	Info.AdvControl(Info.ModuleNumber, ACTL_KEYMACRO, &macro);
}

void PostActivate()
{
#ifndef _UNICODE
	gbPostActivate = true;
	PostMacros();
#else
	HEVENT hEvent = NULL; //CreatEvent(0,0,0,0); -- это если потребуется дождаться из другой нити
	Info.AdvControl ( Info.ModuleNumber, ACTL_SYNCHRO, (void*)hEvent);
	//WaitForSingleObject(hEvent);
#endif
}

#ifdef _UNICODE
int WINAPI _export ProcessSynchroEventW(int Event,void *Param)
{
	if (Event == SE_COMMONSYNCHRO) {
    	if (!PlugStarted && (PGflBitmap || FarVideo)) { // just draw media
        	ShowMedia();
    	}
	}
	return 0;
}
#endif

void WINAPI _export ExitFar(void) {
    UnLoadGfl();
    if (FarVideo) {
        DeleteVideo( FarVideo );
        FarVideo = 0;
    }
    CoUninitialize();
}

void SeconsToHours(LONGLONG time, int* hour, int* min, int* sec)
{
    *hour = (int)(time / 3600); time -= *hour*3600;
    *min = (int)(time / 60);
    *sec = (int)(time - *min*60);
}

LONGLONG VideoJump(int duration, int current)
{
    TCHAR data[1024], str[1024], help[1023];
    data[0] = 0;
    LONGLONG position = 1;
    int mhour, mmin, msec, chour, cmin, csec;

    SeconsToHours(duration, &mhour, &mmin, &msec);
    SeconsToHours(current,  &chour, &cmin, &csec);
    wsprintf(str, GetMsg(FarPicOpenVidJumpTitle), chour, cmin, csec, mhour, mmin, msec);
    wsprintf(help, _T("<%s>Jump"), Info.ModuleName);
    if (Info.InputBox(str, GetMsg(FarPicOpenVidJumpText), _T("MultimediaViewer_JumpToTime"), data, data, sizeof(data), help, 0)){
        int hour, min, sec;
        TCHAR c;
        if (_tcsscanf(data, _T("%d%*1[:, h]%d%*1[:, m]%d"), &hour, &min, &sec) == 3){
            position = 3600*hour + 60*min + sec;
        } else if (_tcsscanf(data, _T("%d%c%d"), &min, &c, &sec) == 3) {
            switch (c) {
                case _T('h') : {
                    position = 3600*min + 60*sec; break;
                }
                 case _T('m') : {
                    position = 60*min + sec; break;
                }
                 case _T(':') : {
                    position = 60*min + sec; break;
                }
                case _T(' ') : {
                    position = 60*min + sec; break;
                }
                case _T(',') : {
                    position = 60*min + sec; break;
                }
            }
        } else if (_tcsscanf(data, _T("%d%c"), &position, &c) == 2) {
            switch (c) {
                case _T('h') : {
                    position *= 3600; break;
                }
                case _T('m') : {
                    position *= 60; break;
                }
                case _T('s') : break;
                default  : position = 1;
            }
        }
        if (position != 1) {
            position *= SECOND;
        }
    }
    return (position);
}

//enum {
//    set_time_abs,
//    set_time_rel,
//};


bool IsTerminalMode()
{
    static bool TerminalMode = false, TerminalChecked = false;
    if (!TerminalChecked) {
        TCHAR szVarValue[64];
        szVarValue[0] = 0;
        if (GetEnvironmentVariable(_T("TERM"), szVarValue, 63) && szVarValue[0]) {
            TerminalMode = true;
        }
        TerminalChecked = true;
    }
    return TerminalMode;
}

HANDLE WINAPI _export OpenPlugin(int OpenFrom,int Item)
{
    ReadConfig();

	bool lbPostActivate = gbPostActivate; gbPostActivate = false;

    if (IsTerminalMode()) return (INVALID_HANDLE_VALUE);
    if (Info.AdvControl(Info.ModuleNumber, ACTL_CONSOLEMODE, (void*)FAR_CONSOLE_GET_MODE) != FAR_CONSOLE_WINDOWED)
		return (INVALID_HANDLE_VALUE);
    
    if (Version.dwPlatformId == VER_PLATFORM_WIN32_NT){
        FindParentHwnd();
    }

    if (!PlugStarted && !PGflBitmap && !FarVideo) { // && FarGetFname( PicName )) {
        bool lbPicOk = false, lbViewerWindow = false;

        gbForceShowDetectWarn = ((OpenFrom & 0xFF) == OPEN_PLUGINSMENU)
			|| ((OpenFrom & 0xFF) == OPEN_VIEWER/* меню из вьювера */) || ((OpenFrom & 0xFF) == OPEN_COMMANDLINE);

		// открытие через префикс
        if ((OpenFrom & 0xFF) == OPEN_COMMANDLINE) {
            lstrcpyn(PicName, (TCHAR*)Item, NUM_ITEMS(PicName));
			lbPicOk = gbFromCmdLine = gbForceShowDetectWarn = true;
			lbViewerWindow = false;
		} else 
		// Открыт через меню плагина из панелей
		if ((OpenFrom & 0xFF) == OPEN_PLUGINSMENU) {
			PanelInfo pi = {0}; lbPicOk = false;
			BOOL lbPanelOk =
			#ifdef _UNICODE
				Info.Control(INVALID_HANDLE_VALUE, FCTL_GETPANELINFO, 0, (LONG_PTR)&pi)
			#else
				Info.Control(INVALID_HANDLE_VALUE, FCTL_GETPANELINFO, &pi)
			#endif
				;
			if (lbPanelOk)
			{
				if (pi.ItemsNumber > 0)
				{
					#ifdef _UNICODE
					PluginPanelItem *ppi = NULL;
					if (size_t len = Info.Control(INVALID_HANDLE_VALUE, FCTL_GETPANELITEM, pi.CurrentItem, 0))
					{
						if (ppi = (PluginPanelItem*)malloc(len))
						{
							if (Info.Control(INVALID_HANDLE_VALUE, FCTL_GETPANELITEM, pi.CurrentItem, (LONG_PTR)ppi)
								&& !(ppi->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
							{
								lstrcpyn(PicName, ppi->FindData.lpwszFileName, NUM_ITEMS(PicName));
								lbPicOk = true;
							}
							free(ppi);
						}
					}
					#else
					if (!(pi.PanelItems[pi.CurrentItem].FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
						lstrcpyn(PicName, pi.PanelItems[pi.CurrentItem].FindData.cFileName, NUM_ITEMS(PicName));
						lbPicOk = true;
					}
					#endif
					if (lbPicOk) {
						gbFromCmdLine = gbForceShowDetectWarn = true;
						lbViewerWindow = false;
					}
				}
			}
		}

		// Если открыт НЕ в окне вьювера - его нужно запустить
		if (lbPicOk && !lbViewerWindow)
		{
			// Чтобы не драться с PicView - ставим EnvVar
			SetEnvironmentVariable(_T("FarPicViewMode"), MMVIEW_ENVVAL _T("Loading"));
			// Теперь - запуск вьювера
			Info.Viewer(PicName, PicName, 0,0,-1,-1, 
				VF_NONMODAL|VF_IMMEDIATERETURN
				#ifdef _UNICODE
				,CP_AUTODETECT
				#endif
			);
			gbForceShowDetectWarn = false; // на всякий случай сбросим? если DetectMedia не был вызван
            lbPicOk = false; // уже обработано событием вьювера!
        } else {
			_ASSERTE(lbPicOk==false);
            lbPicOk = FarGetFname( PicName );
            gbFromCmdLine = false;
			if (!lbPicOk && gbForceShowDetectWarn) {
				Info.Message(Info.ModuleNumber, FMSG_ALLINONE|FMSG_MB_OK|FMSG_WARNING, NULL, (const TCHAR * const *)GetMsg(FarPicNoViewerWindow), 0,0);
			}
        }
        if (lbPicOk)
		{
			if (!DetectMedia( PicName )) {
				//TCHAR* message[] = {GetMsg(FarPicName), _T(""), GetMsg(FarPicWarnNoMedia), _T("")};
				//Info.Message( Info.ModuleNumber, FMSG_WARNING | FMSG_MB_OK, 0, message, NUM_ITEMS(message), 0 );
			} else {
				ShowMedia();
				PlugStarted = true;
			}
        }
        return INVALID_HANDLE_VALUE; // here we can include dialog for detection media
    } else if (!PlugStarted && (PGflBitmap || FarVideo)) { // just draw media
        ShowMedia();
        //PlugStarted = true; -- ставит ShowMedia при успехе
	} else if (lbPostActivate && (PGflBitmap || FarVideo)) { // уже активирован...
		//
    } else if (PlugStarted && PGflBitmap) {// menu for Picture
        FarMenuItem items[FPODgflLast];
        memset(&items, 0, sizeof(items));
		BOOL lbViaCallPlugin = FALSE;
		#ifdef _UNICODE
		lbViaCallPlugin = ((OpenFrom & OPEN_FROMMACRO) == OPEN_FROMMACRO);
		#endif

        SETMENUTEXT(items[FPODgflFitWindow], GetMsg(FarPicOpenFitWindow));
        SETMENUTEXT(items[FPODgflStop], GetMsg(FarPicOpenVidStop));
        items[FPODgfl1Sep].Separator = 1;
        SETMENUTEXT(items[FPODgflReStart], GetMsg(FarPicOpenReStart));

        bool bPopupVisible = false;
        if (!lbViaCallPlugin && ghPopup && IsWindowVisible(ghPopup)) {
        	ShowWindow(ghPopup, SW_HIDE);
        	bPopupVisible = true;
        }

        int dialog_res = -1;
        #ifdef _UNICODE
        // Открытие через макро "CallPlugin(n)"
        if (lbViaCallPlugin && (Item >= 1 && Item <= 100)) {
        	switch(Item) {
			case MVC_FitWindowSwitch: // = 1,
				dialog_res = FPODgflFitWindow; break;
			case MVC_Stop: // = 9,
				dialog_res = FPODgflStop; break;
			//case MVC_FullScreenSwitch: // = 10,
			//	dialog_res = FPODVid1Sep; break;
			case MVC_Restart: // = 15,
				dialog_res = FPODgflReStart; break;
			default:
            	dialog_res = - 1;
        	}
        } else
        #endif
        dialog_res = Info.Menu(Info.ModuleNumber, -1, -1, 23, 0, GetMsg(FarPicName), NULL,
                                   NULL/*help*/, NULL, NULL, items, NUM_ITEMS(items));
        if (bPopupVisible && dialog_res != FPODgflStop) ShowWindow(ghPopup, SW_SHOWNA);
        switch (dialog_res){
        	case FPODgflStop        :
        	{
                ReleaseMedia();
                PlugStarted = false;
        		break;
        	}
            case FPODgflFitWindow   : 
            	SetParam(REG_FULLWINDOW, gbVideoFullWindow = !gbVideoFullWindow);
            case FPODgflReStart     : 
            	ShowMedia(); 
            	break;
        }
    } else if( PlugStarted && FarVideo ) {  // menu for Video
        long lVisible = 0, lPopupVisible = 0;
		BOOL lbViaCallPlugin = FALSE;
		#ifdef _UNICODE
		lbViaCallPlugin = ((OpenFrom & OPEN_FROMMACRO) == OPEN_FROMMACRO);
		#endif

        if( FarVideo->pVW && FarVideo->pBV && SUCCEEDED( FarVideo->pVW->get_Visible( &lVisible ) ) )
            lVisible = 1;

        if (lVisible) {
            #ifdef _UNICODE
            // Открытие через макро "CallPlugin(n)"
            //if (((OpenFrom & OPEN_FROMMACRO) != OPEN_FROMMACRO) || (Item != (FPODVidJump+1))) 
            #endif
			if (!lbViaCallPlugin || (Item == MVC_JumpToTime/*(FPODVidJump+1)*/))
            {
                FarVideo->pVW->put_Visible(OAFALSE);
            }
        }
        if ((!lbViaCallPlugin || (Item == MVC_JumpToTime/*(FPODVidJump+1)*/)) && ghPopup && IsWindowVisible(ghPopup)) {
        	ShowWindow(ghPopup, SW_HIDE);
        	lPopupVisible = 1;
        }


        int code = 0, result = 0;
        FarMenuItem items[FPODVidLast];
        memset(&items, 0, sizeof(items));
        
        SETMENUTEXT(items[FPODVidFitWindow],    GetMsg(FarPicOpenFitWindow));
        SETMENUTEXT(items[FPODVidForward],      GetMsg(FarPicOpenVidForward));
        SETMENUTEXT(items[FPODVidFastForward],  GetMsg(FarPicOpenVidFastForward));
        SETMENUTEXT(items[FPODVidBack],     GetMsg(FarPicOpenVidBack));
        SETMENUTEXT(items[FPODVidFastBack], GetMsg(FarPicOpenVidFastBack));
        SETMENUTEXT(items[FPODVidHome],     GetMsg(FarPicOpenVidHome));
        SETMENUTEXT(items[FPODVidEnd],          GetMsg(FarPicOpenVidEnd));
        SETMENUTEXT(items[FPODVidPause],        GetMsg(FarPicOpenVidPause));
        SETMENUTEXT(items[FPODVidStop],     GetMsg(FarPicOpenVidStop));
        SETMENUTEXT(items[FPODVidFullScreen],     GetMsg(FarPicOpenFullScreen));
        items[FPODVid1Sep].Separator = 1;
        SETMENUTEXT(items[FPODVolumeUp],        GetMsg(FarPicOpenVolumeUp));
        SETMENUTEXT(items[FPODVolumeDown],      GetMsg(FarPicOpenVolumeDown));
		SETMENUTEXT(items[FPODVolumeMute],      GetMsg(FarPicOpenVolumeMute));
        items[FPODVid2Sep].Separator = 1;
        SETMENUTEXT(items[FPODVidJump],     GetMsg(FarPicOpenVidJump));
        SETMENUTEXT(items[FPODVidReStart],      GetMsg(FarPicOpenReStart));
        
        #ifdef _UNICODE
        // Открытие через макро "CallPlugin(n)"
        if (((OpenFrom & OPEN_FROMMACRO) == OPEN_FROMMACRO) && (Item >= 1 && Item <= 100)) {
        	switch(Item) {
			case MVC_FitWindowSwitch: // = 1,
				result = FPODVidFitWindow; break;
			case MVC_Forward: // = 2,
				result = FPODVidForward; break;
			case MVC_FastForward: // = 3,
				result = FPODVidFastForward; break;
			case MVC_Back: // = 4,
				result = FPODVidBack; break;
			case MVC_FastBack: // = 5,
				result = FPODVidFastBack; break;
			case MVC_Home: // = 6,
				result = FPODVidHome; break;
			case MVC_End: // = 7,
				result = FPODVidEnd; break;
			case MVC_PlayPause: // = 8,
				result = FPODVidPause; break;
			case MVC_Stop: // = 9,
				result = FPODVidStop; break;
			case MVC_FullScreenSwitch: // = 10,
				result = FPODVidFullScreen; break;
			case MVC_VolumeUp: // = 11,
				result = FPODVolumeUp; break;
			case MVC_VolumeDown: // = 12,
				result = FPODVolumeDown; break;
			case MVC_VolumeMute: // = 13,
				result = FPODVolumeMute; break;
			case MVC_JumpToTime: // = 14,
				result = FPODVidJump; break;
			case MVC_Restart: // = 15,
				result = FPODVidReStart; break;
			default:
            	result = - 1;
        	}
        } else
        #endif
        result = Info.Menu( Info.ModuleNumber, -1, -1, 0, FMENU_WRAPMODE, GetMsg(FarPicName), GetMsg(FarPicNavigate),
                                _T("Controls"), 0, &code, items, NUM_ITEMS (items));

        //LONGLONG position = 0;
        //FILTER_STATE state;
        //FarVideo->pMS->SetTimeFormat( &TIME_FORMAT_MEDIA_TIME );
        switch (result) {
            case FPODVidFitWindow : {
				FarVideo->FitVideo();
				//                if (lVisible) {
				//                    long width, height;
				//                    SetParam(REG_FULLWINDOW, VideoFullWindow = FarVideo->VideoFullWindow = !FarVideo->VideoFullWindow);
				//                    FarVideo->pBV->get_VideoHeight(&height);
				//                    FarVideo->pBV->get_VideoWidth(&width);
				//
				//                    COORD size = {(short)width, (short)height};
				//
				//                    GetRangedRect(&RangedRect, &size, &FarVideo->VideoFullWindow);
				//                    FarVideo->pVW->SetWindowPosition( RangedRect.left, RangedRect.top, RangedRect.right, RangedRect.bottom );
				////                  FarVideo->pVW->put_FullScreenMode(OATRUE);
				//                }
                break;
            }
            case FPODVidForward : {
                //SetTimePosition(SecondSeeking, set_time_rel);
				FarVideo->JumpRelative(true,false);
                break;
            }
            case FPODVidFastForward : {
                //SetTimePosition(10*SecondSeeking, set_time_rel);
				FarVideo->JumpRelative(true,true);
                break;
            }
            case FPODVidBack : {
                //SetTimePosition(-SecondSeeking, set_time_rel);
				FarVideo->JumpRelative(false,false);
                break;
            }
            case FPODVidFastBack : {
                //SetTimePosition(-10*SecondSeeking, set_time_rel);
				FarVideo->JumpRelative(false,true);
                break;
            }
            case FPODVidHome : {
                //SetTimePosition(0, set_time_abs);
				FarVideo->JumpHome();
                break;
            }
            case FPODVidEnd : {
                //FarVideo->pMS->GetStopPosition( &position );
                //position -= 60;
                //SetTimePosition(position, set_time_abs);
				FarVideo->JumpEnd();
                break;
            }
            case FPODVidPause : {
                //FarVideo->pMC->GetState( INFINITE, (OAFilterState*)&state );
                //FarVideo->pVW->SetWindowForeground(OAFALSE);
                //if (state == State_Running){
                //    FarVideo->pMC->Pause();
                //} else {
                //    FarVideo->pMC->Run();
                //}
				FarVideo->PauseResume();
                break;
            }
            case FPODVidStop : {
                ReleaseMedia();
                PlugStarted = false;
                break;
            }
            case FPODVidFullScreen: {
            	if (ghPopup && IsWindow(ghPopup))
            		PostMessage(ghPopup, WM_LBUTTONDBLCLK, 0, 0);
            	break;
            }

            case FPODVolumeUp : {
				FarVideo->SetVolume(volume_up);
                //Volume = Volume < VOLUMESTEP ? 0 : Volume - VOLUMESTEP;
                //FarVideo->pBA->put_Volume(-Volume);
                //SetParam(REG_VOLUME, Volume);
                break;
            }

            case FPODVolumeDown : {
				FarVideo->SetVolume(volume_down);
                //Volume = Volume > 10000-VOLUMESTEP ? 10000 : Volume + VOLUMESTEP;
                //FarVideo->pBA->put_Volume(-Volume);
                //SetParam(REG_VOLUME, Volume);
                break;
            }

			case FPODVolumeMute : {
				FarVideo->SetVolume(volume_mute);
				break;
			}

            case FPODVidReStart : {
                ReleaseMedia();
                PlugStarted = false;
                PostMacros();
                break;
            }
            case FPODVidJump : {
            	// Вообще-то наверное уже не требуется, т.к. "заливка" идет через Popup окно
                //if (lVisible && PlugStarted) {
                //    FillBackground(&ConsoleRect);
                //}
                //LONGLONG maxpos, curpos;
                //FarVideo->pMS->GetPositions(&curpos, &maxpos);
                //position = VideoJump((int)(maxpos/SECOND), (int)(curpos/SECOND));
                //if (maxpos >= position && position != 1){
                //    SetTimePosition(position, set_time_abs);
                //}
                //// Вообще-то наверное уже не требуется, т.к. "заливка" идет через Popup окно
                ////if (FarVideo->RefreshThread) FillBackground(&ConsoleRect);
				FarVideo->JumpFree();
                break;
            }
        }
        if (PlugStarted) {
        	if (lPopupVisible && ghPopup)
        		ShowWindow(ghPopup, SW_SHOWNA);
			if (lVisible) {
            	FarVideo->pVW->put_Visible(OATRUE);
				FarVideo->pVW->SetWindowForeground(OAFALSE);
				SetForegroundWindow(RootWnd);
			}
        }
    }
    return INVALID_HANDLE_VALUE;
}

int WINAPI _export ProcessEditorEvent(int nEvent, void* param)
{
	if (nEvent == VE_GOTFOCUS) {
		FillWindowShow(FALSE); gbPopupWasHidden = TRUE;
	}
	return 0;
}

int WINAPI _export ProcessViewerEvent(int nEvent, void* param)
{
    ReadConfig();

    //BUGBUG: может ли VE_READ возникнуть после открытия вьювера?
    if (nEvent == VE_READ || nEvent == VE_CLOSE){
        ReleaseMedia();
        PlugStarted = false;
		if (nEvent == VE_CLOSE)
			gbFromCmdLine = false;
    }

	if (nEvent == VE_KILLFOCUS) {
		FillWindowShow(FALSE); gbPopupWasHidden = TRUE;
	} else if (nEvent == VE_GOTFOCUS) {
		bool lbMediaShown = false;
		if (PlugActive && !PlugStarted) {
			// ShowMedia срабатывает только если раньше было успешно создано PGflBitmap или FarVideo
			// поэтому здесь AllowCtrlShiftF3 можно игнорировать
			lbMediaShown = ShowMedia();
		}
		if (gbPopupWasHidden && lbMediaShown) {
			FillWindowShow(TRUE); gbPopupWasHidden = FALSE;
		}
	}

    if (nEvent == VE_READ && (PlugActive || gbFromCmdLine)){
		gbFromCmdLine = false;
        if (IsTerminalMode()) return 0;
		if (!AllowCtrlShiftF3) {
			if (IsKeyPressed(VK_CONTROL) && IsKeyPressed(VK_SHIFT))
				return 0;
		}
		TCHAR szValue[128];
		if (GetEnvironmentVariable(_T("FarPicViewMode"), szValue, 128)) {
			szValue[7] = 0;
			if (lstrcmp(szValue, MMVIEW_ENVVAL) != 0) {
				// значит просмотр уже начат другим плагином
				return 0;
			}
		}

#ifdef _UNICODE
		TODO("Хорошо бы проверять функцией GetConsoleDisplayMode, но она есть только начиная с XP/2003");
#else
        if (Info.AdvControl(Info.ModuleNumber, ACTL_CONSOLEMODE, (void*)FAR_CONSOLE_GET_MODE) != FAR_CONSOLE_WINDOWED){
            return 0; // if we are in fullscreen mode then exit
        }
#endif

        if (!Info.ViewerControl) // check if VewerControl is not defined then exit
            return 0;

        ViewerInfo vinfo = {sizeof( vinfo )}; // getting info about viewer
        Info.ViewerControl( VCTL_GETINFO, &vinfo );

        //gbFromCmdLine = false;

		// Проверить, а не из QView ли мы запустились?
		QView = false; FarGetFname(0, true);
		if (QView && ConEmuWnd) {
			// в режиме PanelViews нужно избежать излишних активаций при скроллировании!
			TCHAR szEnvVar[128];
			if (GetEnvironmentVariable(TH_ENVVAR_NAME, szEnvVar, NUM_ITEMS(szEnvVar))) {
				if (!lstrcmp(szEnvVar, TH_ENVVAR_SCROLL)) {
					return 0;
				}
			}
		}

		// имя файла у нас однозначно известно
		lstrcpyn( PicName, vinfo.FileName, NUM_ITEMS(PicName) );
		if (DetectMedia(PicName)) {
			//if (gbVideoExtMatch && QView)
			PostActivate(); // без этого видео не запускается во вьювере при листании "Gray+/Gray-"?
			//PostMacros(); -- не нужно, это лишние мелькания экрана, сразу ShowMedia()
			//ShowMedia(); -- ожидаем VE_GOTFOCUS! -- в qview VE_GOTFOCUS не приходит
		}
    } else if (nEvent == VE_CLOSE)
        if (!NotUnloadGfl) UnLoadGfl();
    return (0);
}

LONG_PTR WINAPI MVConfigDlgProc(HANDLE hDlg, int Msg, int Param1, LONG_PTR Param2)
{
	if (Msg == DN_BTNCLICK) {
		if (Param1 == FPCDVideoExtsReset) {
			Info.SendDlgMessage(hDlg, DM_SETTEXTPTR, FPCDVideoExtsData, (LONG_PTR)VIDEO_EXTS);
			Info.SendDlgMessage(hDlg, DM_SETFOCUS, FPCDVideoExtsData, 0);
			return TRUE;
		} else if (Param1 == FPCDAudioExtsReset) {
			Info.SendDlgMessage(hDlg, DM_SETTEXTPTR, FPCDAudioExtsData, (LONG_PTR)AUDIO_EXTS);
			Info.SendDlgMessage(hDlg, DM_SETFOCUS, FPCDAudioExtsData, 0);
			return TRUE;
		} else if (Param1 == FPCDPictureExtsReset) {
			Info.SendDlgMessage(hDlg, DM_SETTEXTPTR, FPCDPictureExtsData, (LONG_PTR)PICTURE_EXTS);
			Info.SendDlgMessage(hDlg, DM_SETFOCUS, FPCDPictureExtsData, 0);
			return TRUE;
		}
	}
	return Info.DefDlgProc(hDlg, Msg, Param1, Param2);
}


int WINAPI Configure(int ItemNumber) {

    PluginInit();
    int height = 25;

	LPCTSTR pszResetBtn = GetMsg(FarPicConfReset);
	int nResetLen = (int)_tcslen(pszResetBtn)+4;

    FarDialogItem items[] = {
        // Common options
        {DI_DOUBLEBOX,  3,  1,  51, height - 2, false,  0,              0,                  0,},        //FPCDTitle

		{DI_CHECKBOX,   5,  3,  0,  0,          true,   false,          0,                  false,},    //FPCDPlugActive
		{DI_CHECKBOX,  35,  3,  0,  0,          true,   false,          0,                  false,},    //FPCDCtrlShiftF3

        {DI_CHECKBOX,   5,  5,  0,  0,          true,   false,          0,                  false,},    //FPCDNotUnloadGfl
        {DI_CHECKBOX,   5,  6,  0,  0,          true,   false,          0,                  false,},    //FPCDBilinear
        {DI_CHECKBOX,   5,  7,  0,  0,          true,   false,          0,                  false,},    //FPCDFullWindow
        {DI_CHECKBOX,   5,  8,  0,  0,          true,   false,          0,                  false,},    //FPCDNoSetStartVolume
        {DI_CHECKBOX,   5,  9,  0,  0,          true,   false,          0,                  false,},    //FPCDNoGflWarning

        {DI_TEXT,       11,10,  0,  0,          0,      0,              0,                  0,},        //FPCDDelay9xText
        {DI_FIXEDIT,    5, 10,  9,  0,          false,  (int)_T("99999"),   DIF_MASKEDIT,   0,},        //FPCDDelay9xData

        // Video options
        {DI_CHECKBOX,   5,  12, 0,  0,          true,   false,          0,                  false,},    //FPCDVidKeyframe

        {DI_TEXT,       9,  13, 0,  0,          0,      0,              0,                  0,},        //FPCDVidSeekIntervalText
        {DI_FIXEDIT,    5,  13, 7,  0,          false,  (int)_T("999"),     DIF_MASKEDIT,   0,},        //FPCDVidSeekIntervalData

        {DI_TEXT,       5,  15, 0,  0,          0,      0,              0,                  0,},        //FPCDVideoExtsText
        {DI_EDIT,       5,  16, 49, 0,          false,  NULL,           DIF_MASKEDIT,       0,},        //FPCDVideoExtsData

        {DI_TEXT,       5,  17, 0,  0,          0,      0,              0,                  0,},        //FPCDAudioExtsText
        {DI_EDIT,       5,  18, 49, 0,          false,  NULL,           DIF_MASKEDIT,       0,},        //FPCDAudioExtsData

        {DI_TEXT,       5,  19, 0,  0,          0,      0,              0,                  0,},        //FPCDPictureExtsText
        {DI_EDIT,       5,  20, 49, 0,          false,  NULL,           DIF_MASKEDIT,       0,},        //FPCDPictureExtsData


		{DI_BUTTON,       50-nResetLen, 15, 0, 0, 0, 0, 0, 0,},        //FPCDVideoExtsReset
		{DI_BUTTON,       50-nResetLen, 17, 0, 0, 0, 0, 0, 0,},        //FPCDAudioExtsReset
		{DI_BUTTON,       50-nResetLen, 19, 0, 0, 0, 0, 0, 0,},        //FPCDPictureExtsReset

        {DI_BUTTON,     0,  22, 0,  0,          true,   true,           DIF_CENTERGROUP,    true,},     //FPCDOK
        {DI_BUTTON,     0,  22, 0,  0,          true,   false,          DIF_CENTERGROUP,    false,},    //FPCDCancel
    };
    
    SETTEXT(items[FPCDTitle], GetMsg(FarPicName));
    SETTEXT(items[FPCDBilinear], GetMsg(FarPicConfBilinear));
    SETTEXT(items[FPCDNotUnloadGfl], GetMsg(FarPicConfNotUnloadGfl));
    SETTEXT(items[FPCDDelay9xText], GetMsg(FarPicConfDelay9xText));

    SETTEXT(items[FPCDVidSeekIntervalText], GetMsg(FarPicConfVidSeekIntervalText));

    SETTEXT(items[FPCDVidKeyframe], GetMsg(FarPicConfVidKeyframe));
    SETTEXT(items[FPCDVideoExtsText], GetMsg(FarPicConfVideoExtsText));
    SETTEXT(items[FPCDAudioExtsText], GetMsg(FarPicConfAudioExtsText));
    SETTEXT(items[FPCDPictureExtsText], GetMsg(FarPicConfPictureExtsText));
    SETTEXT(items[FPCDFullWindow], GetMsg(FarPicConfVideoFullWind));
    SETTEXT(items[FPCDNoSetStartVolume], GetMsg(FarPicConfNotSetStartVolume));
    SETTEXT(items[FPCDNoGflWarning], GetMsg(FarPicNoGflWarn));
	SETTEXT(items[FPCDVideoExtsReset], GetMsg(FarPicConfReset));
	SETTEXT(items[FPCDAudioExtsReset], GetMsg(FarPicConfReset));
	SETTEXT(items[FPCDPictureExtsReset], GetMsg(FarPicConfReset));
    SETTEXT(items[FPCDOK], GetMsg(FarPicOK));
    SETTEXT(items[FPCDCancel], GetMsg(FarPicCancel));
    SETTEXT(items[FPCDPlugActive], GetMsg(FarPicConfPlugActive));
	SETTEXT(items[FPCDCtrlShiftF3], GetMsg(FarPicCtrlShiftF3));

	items[FPCDPlugActive].Selected = PlugActive;
	items[FPCDCtrlShiftF3].Selected = AllowCtrlShiftF3;
    items[FPCDNotUnloadGfl].Selected = NotUnloadGfl;
    items[FPCDBilinear].Selected = Bilinear;
    items[FPCDFullWindow].Selected = gbVideoFullWindow;
    items[FPCDNoSetStartVolume].Selected = NoSetStartVolume;
    items[FPCDNoGflWarning].Selected = NoGflWarning;
    items[FPCDVidKeyframe].Selected = (VidSeekType & AM_SEEKING_SeekToKeyFrame);

    TCHAR szBuf[0x1000], *pszBuf = szBuf;
    SETTEXTPRINT(items[FPCDDelay9xData], _T("%5d"), Delay9x > 99999 ? DELAY9XDEFLT : Delay9x);
    SETTEXTPRINT(items[FPCDVidSeekIntervalData], _T("%3d"), SecondSeeking / SECOND);
    SETTEXT(items[FPCDVideoExtsData], VideoExts);
    SETTEXT(items[FPCDAudioExtsData], AudioExts);
    SETTEXT(items[FPCDPictureExtsData], PictureExts);
    
    if (Version.dwPlatformId == VER_PLATFORM_WIN32_NT){
        items[FPCDDelay9xText].Flags = DIF_HIDDEN;
        items[FPCDDelay9xData].Flags = DIF_HIDDEN;
        items[FPCDTitle].Y2 -= 1; height--;
        for (int i = FPCDDelay9xData + 1; i < FPCDLast; i++){
            items[i].Y1 -= 1;
        }
    }

    int dialog_res = 0;

	#ifdef _UNICODE
	HANDLE hDlg = Info.DialogInit ( Info.ModuleNumber, -1, -1, 55, height,
		_T("Configure"), items, NUM_ITEMS(items), 0, 0/*Flags*/, MVConfigDlgProc, 0/*DlgProcParam*/ );
	#endif


    #ifndef _UNICODE
    //dialog_res = Info.Dialog(Info.ModuleNumber, -1, -1, 55, height, _T("Configure"), items, NUM_ITEMS(items));
	dialog_res = Info.DialogEx(Info.ModuleNumber,-1,-1,55,height, "Configure", items, NUM_ITEMS(items), 0, 0,
		MVConfigDlgProc, NULL);
    #else
    dialog_res = Info.DialogRun ( hDlg );
    #endif

    if (dialog_res != -1 && dialog_res != FPCDCancel)
    {
        SetParam(REG_NOTUNLOADGFL, _GetCheck(FPCDNotUnloadGfl));
        SetParam(REG_BILINEAR, _GetCheck(FPCDBilinear));
        SetParam(REG_FULLWINDOW, _GetCheck(FPCDFullWindow));
        SetParam(REG_NOSTARTVOL, _GetCheck(FPCDNoSetStartVolume));
        SetParam(REG_NOGFLWARN, _GetCheck(FPCDNoGflWarning));
        SetParam(REG_VIDSEEK, _GetCheck(FPCDVidKeyframe));
        SetParam(REG_PLUGACTIVE, _GetCheck(FPCDPlugActive));
		SetParam(REG_ALLOWCTRLSHIFTF3, _GetCheck(FPCDCtrlShiftF3));
        SetStrParam(REG_VIDEOEXTS, GetDataPtr(FPCDVideoExtsData));
        SetStrParam(REG_AUDIOEXTS, GetDataPtr(FPCDAudioExtsData));
        SetStrParam(REG_PICTUREEXTS, GetDataPtr(FPCDPictureExtsData));
        if (Version.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
        {
            SetParam(REG_DELAY9X, _tcstoi(GetDataPtr(FPCDDelay9xData)));
        }
        SetParam(REG_VIDEOSEEKINTERVAL, _tcstoi(GetDataPtr(FPCDVidSeekIntervalData)));

        ReadConfig();

        if (!NotUnloadGfl)
        {
            UnLoadGfl();
        }
    }
    #ifdef _UNICODE
    Info.DialogFree ( hDlg );
    #endif
    
    return(true);
}
