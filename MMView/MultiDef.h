
#pragma once

#define NUM_ITEMS(X) (sizeof(X)/sizeof(X[0]))
#define TODO(x)
#define WARNING(x)

#ifdef _DEBUG
#include <crtdbg.h>
#else
#define _ASSERTE(x)
#endif

#ifdef _DEBUG
extern void LogFore(LPCSTR asInfo);
#else
#define LogFore(asInfo)
#endif


#include <dshow.h>

enum {
	set_time_abs,
	set_time_rel,
};

enum MMViewVolume{
	volume_up,
	volume_down,
	volume_mute
};

typedef struct tag_Video 
{
	enum {
		eEmpty = 0,
		ePicture,
		eAudio,
		eVideo
	} m_Type;
	// DirectShow interfaces
	IGraphBuilder   *pGB;
	IMediaControl   *pMC;
	IMediaEventEx   *pME;
	IVideoWindow    *pVW;
	IBasicAudio     *pBA;
	IBasicVideo     *pBV;
	IMediaSeeking   *pMS;
	IMediaPosition  *pMP;
	IVideoFrameStep *pFS;
	//
	IBaseFilter     *pIn, *pOutA, *pOutV;
	IPin            *pPin, *pPinA, *pPinV;

	HANDLE          RefreshThread;
	DWORD           RefreshThreadId;
	bool            Initialized;
	bool            VideoFullWindow;

	TCHAR sCaptionFormat[MAX_PATH];
	TCHAR sFileName[MAX_PATH]; // ТОЛЬКО имя файла, для заголовка
	TCHAR sLastCurPos[32];

	void ShowPosition();
	void SetTimePosition(LONGLONG newtime, int type);
	void JumpRelative(bool abForward, bool abFast);
	void JumpFree(); // с выбором позиции пользователем
	void JumpHome();
	void JumpEnd();
	void PauseResume();
	void SetVolume(MMViewVolume type);
	void FitVideo();
} Video;

extern Video* FarVideo;

class SetCaption
{
protected:
	TCHAR* ms_OldCaption;
public:
	SetCaption(LPCTSTR asNewCaption) {
		ms_OldCaption = (TCHAR*)calloc(4096,sizeof(TCHAR));
		GetConsoleTitle(ms_OldCaption, 4096);
		SetConsoleTitle(asNewCaption);
	};
	~SetCaption() {
		if (ms_OldCaption) {
			SetConsoleTitle(ms_OldCaption);
			free(ms_OldCaption);
		}
	};
};

#define CEKEYEVENT_CTRL     _T("ConEmuCtrlPressed.%u")
#define CEKEYEVENT_SHIFT    _T("ConEmuShiftPressed.%u")
#define TH_ENVVAR_NAME      _T("FarThumbnails")
#define TH_ENVVAR_SCROLL    _T("Scrolling")

#include <PshPack1.h>
typedef struct tag_FarVersion {
	union {
		struct {
			union {
				struct {
					BYTE dwVerMinor;
					BYTE dwVerMajor;
				};
				WORD dwVer;
			};
			WORD dwBuild;
		};
		DWORD dwRAW;
	};
} FarVersion;
#include <PopPack.h>
extern FarVersion gFarVersion;
