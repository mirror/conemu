#pragma once

/**************************************************************************
Copyright (c) 2009 Skakov Pavel
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

#define _FAR_NO_NAMELESS_UNIONS

#include "headers/plugin.hpp"
#include <TCHAR.h>

typedef unsigned int uint;
typedef unsigned __int8 u8;
typedef DWORD u32;

#include "PVDInterface/PictureViewPlugin.h"

#define STRING2(x) #x
#define STRING(x) STRING2(x)
#define FILE_LINE __FILE__ "(" STRING(__LINE__) "): "
#ifdef HIDE_TODO
#define TODO(s) 
#define WARNING(s) 
#else
#define TODO(s) __pragma(message (FILE_LINE "TODO: " s))
#define WARNING(s) __pragma(message (FILE_LINE "warning: " s))
#endif
#define PRAGMA_ERROR(s) __pragma(message (FILE_LINE "error: " s))

#ifdef _DEBUG
	#include <crtdbg.h>

	int MyCrtDbgReportW(int _ReportType, const wchar_t * _Filename, int _LineNumber, const wchar_t * _ModuleName, const wchar_t * _Format);

	#undef _ASSERT_EXPR
	#undef _ASSERTE
	#define _ASSERT_EXPR(expr, msg) \
		(void) ((!!(expr)) || \
		(1 != MyCrtDbgReportW(_CRT_ASSERT, _CRT_WIDE(__FILE__), __LINE__, NULL, msg)) || \
		(_CrtDbgBreak(), 0))
	#define _ASSERTE(expr)  _ASSERT_EXPR((expr), _CRT_WIDE(#expr))

#else
	#ifndef _ASSERTE
		#define _ASSERTE(a)
	#endif
#endif

#undef _ASSERT
#define _ASSERT _ASSERTE



#pragma warning(disable: 4800) // forcing value to bool 'true' or 'false'

#ifndef sizeofarray
	#define sizeofarray(array) (sizeof(array)/sizeof(*array))
#endif
#define DllGetFunction(hModule, FunctionName) FunctionName = (FunctionName##_t)GetProcAddress(hModule, #FunctionName)
#ifdef _DEBUG
	uint MulDivU32(long a, long b, long c);// (uint)((unsigned long long)(a)*(b)/(c))
	uint MulDivU32R(long a, long b, long c);// (uint)(((unsigned long long)(a)*(b) + (c)/2)/(c))
	int  MulDivIU32R(long a, long b, long c);// (int)(((long long)(a)*(b) + (c)/2)/(c))
#else
	#define MulDivU32(a, b, c) (uint)((unsigned long long)(a)*(b)/(c))
	#define MulDivU32R(a, b, c) (uint)(((unsigned long long)(a)*(b) + (c)/2)/(c))
	#define MulDivIU32R(a, b, c) (int)(((long long)(a)*(b) + (c)/2)/(c))
#endif

#define SAFEFREE(p) if ((p)!=NULL) { free(p); (p) = NULL; }

//#ifdef _DEBUG
//	//#define TRY if (TRUE)
//	#define TRY try
//	//#define CATCH else
//	#define CATCH catch(...)
//#else
#define TRY __try
#define CATCH __except( EXCEPTION_EXECUTE_HANDLER )
//#endif


typedef __int16 i16;
typedef __int32 i32;
typedef __int64 i64;
typedef unsigned __int8 u8;
typedef unsigned __int16 u16;

enum MsgIds;
class ImageInfo;
class PluginData;


enum EFlagsDisplay
{
	FD_TITLE_REPAINT = 1,
	FD_JUMP          = 2,
	FD_JUMP_NEXT     = 4,
	FD_REFRESH       = 8,
	FD_HOME_END      = 0x10,
	//FD_CONFIG        = 0x20,
	FD_DISLPAYED     = 0x40,
	//FD_CONFIGDECODER = 0x80,
	FD_REQ_REFRESH   = 0x200,
};
enum EFlagsWork
{
	FW_JUMP_DISABLED = 1, // disable jumping and selection change
	FW_PLUGIN_CALL   = 2, // Плагин должен поймать (Ctrl-PgDn) - извлечение файла с плагиновой панели
	FW_VE_HOOK       = 4, // exit from opened viewer/editor on success open
	FW_QUICK_VIEW    = 8, // quick view mode
	FW_TERMINATE     = 0x10,   // Завершение декодирования, закрытие плагина
	//FW_FIRSTIMAGE    = 0x20,   // Первый файл серии
	FW_SHOW_HELP     = 0x40,   // Показать Help в основной нити
	FW_SHOW_CONFIG   = 0x80,   // Показать окно конфига в основной нити
	FW_SHOW_MODULES  = 0x100,  // Показать вкладку модулей в основной нити
	FW_PREVDECODER   = 0x200,
	FW_NEXTDECODER   = 0x400,
	FW_FORCE_DECODE  = 0x800,  // Усиленный подбор декодера (явный вызов на первом файле серии, если это НЕ перехват первого входа в архив)
	FW_MARK_FILE     = 0x1000,
	FW_UNMARK_FILE   = 0x2000,
	FW_TITLEREPAINT  = 0x4000,
	FW_TITLEREPAINTD = 0x8000,
	FW_QVIEWREPAINT  = 0x10000,
	FW_FIRST_CALL    = 0x20000,  // Явный вызов на первом файле серии
};
enum EFlagsResult
{
	FE_PROCEEDED         =  0, // Успешно обработан. При необходимости вернуть в FAR (HANDLE)-2
	FE_SKIP_ERROR        =  1, // Не открыли, но ошибку показывать не нужно
	FE_EXCEPTION         =  2, // Исключение. Ошибка уже показана
	FE_UNKNOWN_EXTENSION =  3, // Неподдерживаемое расширение
	FE_OPEN_FAILED       =  4, // Ошибка открытия
	FE_NO_PANEL_ITEM     =  5, // 
};
enum EZoomAutoFlags
{
	ZA_NONE = 0,  // Не подгонять размер
	ZA_FIT  = 1,  // Поместить картинку в окне
	ZA_FILL = 2,  // Заполнить окно картинкой (картинка обрезается)
};

#include "PictureView_Class.h"

template <class T> const T& Min(const T &a, const T &b) {return a < b ? a : b;}
template <class T> const T& Max(const T &a, const T &b) {return a > b ? a : b;}
template <class T> void Swap(T &a, T &b) {const T t = a; a = b; b = t;}

extern const wchar_t g_WndClassName[];
extern const wchar_t g_DefaultTitleTemplate[];
extern const wchar_t g_DefaultQViewTemplate1[], g_DefaultQViewTemplate2[], g_DefaultQViewTemplate3[];
extern wchar_t *g_SelfPath; // Содержит полный путь к папке, в которой лежит 0PictureView.dll (на конце - слэш)
extern wchar_t *g_RootKey; // Содержит полный путь к ключу реестра. Обычно это "Software\\Far2\\Plugins\\PictureView"

extern PluginStartupInfo g_StartupInfo;
extern FarStandardFunctions g_FSF;
extern HINSTANCE g_hInstance;
extern PluginData g_Plugin;
extern wchar_t g_TitleTemplate[0x200];
extern wchar_t g_QViewTemplate1[0x200];
extern wchar_t g_QViewTemplate2[0x200];
extern wchar_t g_QViewTemplate3[0x200];

extern DWORD gnMainThreadId, gnDisplayThreadId, gnDecoderThreadId;

const wchar_t *GetMsg(int MsgId);
void RegKeyRead(HKEY RegKey, const wchar_t* const Name, bool *const Param, const bool Default = false);
void RegKeyRead(HKEY RegKey, const wchar_t* const Name, u32 *const Param, const u32 Default = 0);
LONG RegKeyWrite(HKEY RegKey, const wchar_t* const Name, const u32 Param);

LRESULT CALLBACK WndProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);
DWORD WINAPI DisplayThreadProc(void *pParameter = NULL);
void TitleRepaint(bool bDecoding = false);
void QViewRepaint(bool bDecoding = false);
int WINAPI ConfigureW(int ItemNumber);
bool WaitDisplayEvent();
bool CreateDisplayThread();
void HideConsoleCursor(void);
void AutoHideConsoleCursor(void);
u32 CheckAbsoluteZoom();
u32 CheckRelativeZoom();
UINT ProcessConsoleInputs(); // Возвращает количество событий, оставшихся в буфере консоли

wchar_t* ConcatPath(const wchar_t* Dir, const wchar_t* Name, bool abReverseName=false);
bool ExtensionMatch(LPTSTR asExtList, LPCTSTR asExt);
const wchar_t* GetExtension(const wchar_t* pFileName);

bool EscapePressed();

DWORD WINAPI CMYK_ThreadProc(void*);


#define DEFAULT_INGORED_EXT L"7z,aif,aifc,aiff,ape,arj,asf,au,avi,bat,bbs,bz2,bzip2,cab,cmd,deb,dll,dmg,exe,flv,gz,gzip,hfs,ion,iso,iso,jar,lha,lnk,lzh,lzma,lzma86,m1v,m2v,mid,midi,mkv,mp2,mp3,mp4,mpe,mpeg,mpeg4,mpg,msi,ogg,rmi,rpm,snd,swm,tar,taz,tbz,tbz2,tgz,tpz,txt,vob,wav,wim,wm,wma,wmd,wmv,wv,xar,z,zip"

#define PANEL_ITEM_UNAVAILABLE (-2)

#define DMSG_KEYBOARD (WM_APP)
#define DMSG_NEXTFILEPAGE (WM_APP + 1)
#define DMSG_ZOOMBYMOUSE (WM_APP + 2)
#define DMSG_FULLSCREENSWITCH (WM_APP + 3)
#define DMSG_REFRESH (WM_APP + 4)
#define DMSG_KEYUP (WM_APP + 5)
//#define DMSG_SHOWWINDOW (WM_APP + 6)
//#define DMSG_SHOWWINDOW_KEY 0x0101

#define PICVIEW_ENVVAL L"PicView2:"


#ifdef _DEBUG
    int __stdcall _MDEBUG_TRAP(LPCSTR asFile, int anLine);
    extern int MDEBUG_CHK;
    extern char gsz_MDEBUG_TRAP_MSG_APPEND[2000];
    #define MDEBUG_TRAP1(S1) {lstrcpyA(gsz_MDEBUG_TRAP_MSG_APPEND,(S1));_MDEBUG_TRAP(__FILE__,__LINE__);}
    #include <crtdbg.h>
    #define MCHKHEAP { MDEBUG_CHK=_CrtCheckMemory(); if (!MDEBUG_CHK) { MDEBUG_TRAP1("_CrtCheckMemory failed"); } }
    //#define MCHKHEAP
#else
    #define MCHKHEAP
#endif

#define CEKEYEVENT_CTRL     _T("ConEmuCtrlPressed.%u")
#define CEKEYEVENT_SHIFT    _T("ConEmuShiftPressed.%u")
#define TH_ENVVAR_NAME      _T("FarThumbnails")
#define TH_ENVVAR_SCROLL    _T("Scrolling")
