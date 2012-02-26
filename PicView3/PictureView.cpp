
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

#define DEFINE_OPTIONS
#include "PictureView.h"
#include "PVDManager.h"
#include "PictureView_Lang.h"
#include "version.h"
#include "Image.h"
#include "RefKeeper.h"

//
//#include "ConEmuSupport.h"
#ifdef _DEBUG
	#include <shlwapi.h>
#endif
//#include <Tlhelp32.h>

//#define deletex(pointer) {delete pointer; pointer = NULL;}

HINSTANCE g_hInstance;

PluginStartupInfo g_StartupInfo;
FarStandardFunctions g_FSF;
#ifndef FAR_UNICODE
DWORD nFarVersion = 0;
#endif
DWORD gnMainThreadId = 0, gnDisplayThreadId = 0, gnDecoderThreadId = 0;

//CPluginData g_Plugin;
//CPicViewPanel g_Panel;
//CPVDManager g_Manager;

wchar_t *g_SelfPath = NULL; // Содержит полный путь к папке, в которой лежит 0PictureView.dll (на конце - слэш)
wchar_t *g_RootKey = NULL;  // Содержит полный путь к ключу реестра. Обычно это "Software\\Far2\\Plugins\\PictureView"

#ifdef FAR_UNICODE
GUID guid_PicView = { /* 24dd1870-e341-4d60-8d19-f5d06e3f99de */
    0x24dd1870,
    0xe341,
    0x4d60,
    {0x8d, 0x19, 0xf5, 0xd0, 0x6e, 0x3f, 0x99, 0xde}
  };
GUID guid_PlugMenu = { /* 8aa41813-cba7-4615-add5-3c01fb88ae02 */
    0x8aa41813,
    0xcba7,
    0x4615,
    {0xad, 0xd5, 0x3c, 0x01, 0xfb, 0x88, 0xae, 0x02}
  };
GUID guid_ConfMenu = { /* f76b2c10-78c8-4cd2-87b6-07731bb6859e */
    0xf76b2c10,
    0x78c8,
    0x4cd2,
    {0x87, 0xb6, 0x07, 0x73, 0x1b, 0xb6, 0x85, 0x9e}
  };
GUID guid_ConfDlg = { /* 2037b068-64c8-4c68-839d-2400a3e83917 */
    0x2037b068,
    0x64c8,
    0x4c68,
    {0x83, 0x9d, 0x24, 0x00, 0xa3, 0xe8, 0x39, 0x17}
  };
GUID guid_MsgBox = { /* 30648cc6-daa7-49af-b60f-b116392ece52 */
    0x30648cc6,
    0xdaa7,
    0x49af,
    {0xb6, 0x0f, 0xb1, 0x16, 0x39, 0x2e, 0xce, 0x52}
  };
#endif

//void TestJumpDisable(const wchar_t *pFileName)
//{
//	const wchar_t *pPanelFileName = NULL;
//	PanelInfo pi;
//	PluginPanelItem *ppi = NULL;
//	wchar_t *pCurDir = NULL;
//	if (g_StartupInfo.PanelControl(INVALID_HANDLE_VALUE, FCTL_GETPANELINFO, 0, (LONG_PTR)&pi))
//		if (pi.ItemsNumber > 0)
//			if (size_t len = g_StartupInfo.PanelControl(INVALID_HANDLE_VALUE, FCTL_GETPANELITEM, pi.CurrentItem, 0))
//				if (ppi = (PluginPanelItem*)malloc(len))
//					if (g_StartupInfo.PanelControl(INVALID_HANDLE_VALUE, FCTL_GETPANELITEM, pi.CurrentItem, (LONG_PTR)ppi))
//						pPanelFileName = ppi->FindData.lpwszFileName;
//
//	g_Plugin.Image[0]->bSelected = ppi ? ((ppi->Flags & PPIF_SELECTED)!=0) : false;
//
//	TODO("А флаг FW_JUMP_DISABLED сбрасывать предварительно не нужно?");
//
//	if (!*pFileName && pPanelFileName) {
//		//lstrcpy(g_Plugin.Image[0]->FileName, pPanelFileName);
//		if (!wcsncmp(pPanelFileName,L"\\\\?\\",4)) pPanelFileName+=4;
//		if (!PutFilePathName(&g_Plugin.Image[0]->FileName, pPanelFileName))
//			g_Plugin.FlagsWork |= FW_JUMP_DISABLED;
//	}
//	else
//	{
//		//lstrcpynW(g_Plugin.Image[0]->FileName, pFileName, sizeof(g_Plugin.Image[0]->FileName));
//		//CUnicodeFileName::SkipPrefix(&pPanelFileName);
//
//		if (!PutFilePathName(&g_Plugin.Image[0]->FileName, pFileName))
//			g_Plugin.FlagsWork |= FW_JUMP_DISABLED;
//		//g_FSF.Unquote(g_Plugin.Image[0]->FileName);
//		else if (!pPanelFileName) // viewer/editor?
//			g_Plugin.FlagsWork |= FW_JUMP_DISABLED;
//		else
//		{
//			const wchar_t* pszImgFileName = g_Plugin.Image[0]->FileName;
//			CUnicodeFileName::SkipPrefix(&pPanelFileName);
//			CUnicodeFileName::SkipPrefix(&pszImgFileName);
//
//			if (g_FSF.LStricmp(pPanelFileName, pszImgFileName))
//			// Если имя под курсором совпадает с именем переданным в плагин ("pic:!.!" ?)
//			{
//				const wchar_t *const p = g_FSF.PointToName(g_Plugin.Image[0]->FileName);
//				if (g_FSF.LStricmp(pPanelFileName, p))
//					g_Plugin.FlagsWork |= FW_JUMP_DISABLED;
//				else
//					if (pi.Flags & PFLAGS_REALNAMES)
//					{
//						//const int lPath = p - g_Plugin.Image[0]->FileNameData - 1;
//						if (size_t len = g_StartupInfo.PanelControl(INVALID_HANDLE_VALUE, FCTL_GETPANELDIR, 0, 0))
//							if (pCurDir = (wchar_t*)malloc(len*2))
//								if (!g_StartupInfo.PanelControl(INVALID_HANDLE_VALUE, FCTL_GETPANELDIR, len, (LONG_PTR)pCurDir))
//									*pCurDir = 0;
//						if (!pCurDir || !g_Plugin.Image[0]->FileName.CompareDir(pCurDir))
//						{
//							g_Plugin.FlagsWork |= FW_JUMP_DISABLED;
//						}
//					}
//			}
//		}
//	}
//	if (pCurDir) free(pCurDir);
//	if (ppi) free(ppi);
//}

//bool SwitchImages(bool bForward)
//{
//	bool result = false;
//	if (bForward)
//	{
//		if (g_Plugin.Image[2]->pDraw)
//		{
//			Swap(g_Plugin.Image[0], g_Plugin.Image[1]);
//			Swap(g_Plugin.Image[0], g_Plugin.Image[2]);
//
//			g_Plugin.Image[2]->DisplayClose();
//			if (g_Plugin.Image[2]->Decoder)
//			{
//				g_Plugin.Image[2]->Decoder->Close();
//				//deletex(g_Plugin.Image[2]->Decoder);
//			}
//			result = true;
//		}
//		g_Plugin.Image[2]->SetPanelItemRaw(g_Plugin.Image[0]->PanelItemRaw());
//		g_Plugin.ImageX = g_Plugin.Image[2];
//		//g_Plugin.ddsx = g_Plugin.Image[2]->dds;
//	}
//	else
//	{
//		if (g_Plugin.Image[1]->pDraw)
//		{
//			Swap(g_Plugin.Image[0], g_Plugin.Image[2]);
//			Swap(g_Plugin.Image[0], g_Plugin.Image[1]);
//
//			g_Plugin.Image[1]->DisplayClose();
//			if (g_Plugin.Image[1]->Decoder)
//			{
//				g_Plugin.Image[1]->Decoder->Close();
//				//deletex(g_Plugin.Image[1]->Decoder);
//			}
//			result = true;
//		}
//		g_Plugin.Image[1]->SetPanelItemRaw(g_Plugin.Image[0]->PanelItemRaw());
//		g_Plugin.ImageX = g_Plugin.Image[1];
//		//g_Plugin.ddsx = g_Plugin.Image[1]->dds;
//	}
//
//	OutputDebugString(L"Invalidating in SwitchImages\n");
//	InvalidateRect(g_Plugin.hWnd, NULL, FALSE);
//	return result;
//}

//bool WaitDisplayEvent()
//{
//	DWORD dwWait = 0;
//	HANDLE hEvents[2] = {g_Plugin.hDisplayEvent, g_Plugin.hDisplayThread};
//	//Wait ForSingleObject(g_Plugin.hDisplayEvent, INFINITE); -- низя. Если нить грохнулась - мы здесь повиснем
//
//	_ASSERTE(GetCurrentThreadId() == gnMainThreadId);
//
//	// Бесконечное ожидание заменить на цикл, в котором опрашивать консоль. А из Display - убрать ReadConsoleInput
//	// Это нужно для того, чтобы FAR не вис при переключении в другие приложения во время отрисовки диалогов и хелпа
//	// НО нужно учесть, что если работает хелп/диалог то консоль читает сам фар?
//
//	while ((dwWait = WaitForMultipleObjects(2, hEvents, FALSE, 0)) == WAIT_TIMEOUT)
//	{
//		//// Может быть потом тут что-то типа PeekConsole... вставить, во избежание повисания conhost
//		//if (g_Plugin.hWnd && !IsWindowVisible(g_Plugin.hWnd)) {
//		//	INPUT_RECORD ir;
//		//	u32 t;
//		//	PeekConsoleInput(GetStdHandle(STD_INPUT_HANDLE)/*g_Plugin.hInput*/, &ir, 1, &t);
//		//}
//		if (g_Plugin.FlagsWork & FW_SHOW_HELP) {
//			// Показать Help в основной нити
//			CFunctionLogger flog(L"FW_SHOW_HELP");
//			wchar_t Title[0x40];
//			lstrcpynW(Title, g_Plugin.pszPluginTitle, sizeofarray(Title) - 6);
//			lstrcat(Title, L" - Far");
//			// Раньше выполнялось в нити Display, и часто висла
//			SetConsoleTitleW(Title);
//
//			g_StartupInfo.ShowHelp(g_StartupInfo.ModuleName, _T("Controls"), 0);
//			
//			SetEvent(g_Plugin.hSynchroDone);
//			g_Plugin.FlagsWork &= ~FW_SHOW_HELP;
//			
//		} else if (g_Plugin.FlagsWork & (FW_SHOW_CONFIG | FW_SHOW_MODULES)) {
//			// Показать окно конфига в основной нити
//			CFunctionLogger flog(L"FW_SHOW_CONFIG|FW_SHOW_MODULES");
//			
//			//TRUE, если настройки были изменены
//			const bool reconfig = ConfigureW(0);
//			
//			if (reconfig)
//				g_Plugin.FlagsDisplay |= FD_REQ_REFRESH;
//			SetEvent(g_Plugin.hSynchroDone);
//			g_Plugin.FlagsWork &= ~(FW_SHOW_CONFIG|FW_SHOW_MODULES);
//
//		} else if (g_Plugin.FlagsWork & (FW_MARK_FILE | FW_UNMARK_FILE)) {
//			CFunctionLogger flog(L"FW_MARK_FILE | FW_UNMARK_FILE");
//
//			//g_Plugin.Image[0]->bSelected = (g_Plugin.FlagsWork & FW_MARK_FILE) == FW_MARK_FILE;
//			g_Plugin.SelectionChanged = true; // чтобы при выходе из плагина можно было обновить панель, показав выделенные вверху
//			//g_StartupInfo.PanelControl(INVALID_HANDLE_VALUE, FCTL_SETSELECTION, g_Plugin.Image[0]->PanelItemRaw(), g_Plugin.Image[0]->bSelected);
//			//g_StartupInfo.PanelControl(INVALID_HANDLE_VALUE, FCTL_REDRAWPANEL, 0, 0);
//			g_Panel.MarkUnmarkFile( ((g_Plugin.FlagsWork & FW_MARK_FILE) == FW_MARK_FILE)
//				? CPicViewPanel::ema_Mark : CPicViewPanel::ema_Unmark);
//			TitleRepaint();
//			SetEvent(g_Plugin.hSynchroDone);
//			g_Plugin.FlagsWork &= ~(FW_MARK_FILE | FW_UNMARK_FILE);
//
//		} else if (g_Plugin.FlagsWork & (FW_TITLEREPAINT | FW_TITLEREPAINTD)) {
//			CFunctionLogger flog(L"FW_TITLEREPAINT | FW_TITLEREPAINTD");
//
//			TitleRepaint((g_Plugin.FlagsWork & FW_TITLEREPAINTD)==FW_TITLEREPAINTD);
//			SetEvent(g_Plugin.hSynchroDone);
//			g_Plugin.FlagsWork &= ~(FW_TITLEREPAINT | FW_TITLEREPAINTD);
//
//		} else if (g_Plugin.FlagsWork & (FW_QVIEWREPAINT)) {
//			CFunctionLogger flog(L"FW_QVIEWREPAINT");
//
//			QViewRepaint();
//			SetEvent(g_Plugin.hSynchroDone);
//			g_Plugin.FlagsWork &= ~FW_QVIEWREPAINT;
//
//		}
//
//		if (!ProcessConsoleInputs())
//			Sleep(10);
//		if (g_Plugin.FlagsWork & FW_TERMINATE) {
//			while ((dwWait = WaitForMultipleObjects(2, hEvents, FALSE, 10)) == WAIT_TIMEOUT)
//				;
//			break;
//		}
//	}
//
//	CFunctionLogger::FunctionLogger(L"WorkDisplayEvent done");
//	//_ASSERTE(dwWait!=(WAIT_OBJECT_0+1));
//	return (dwWait==WAIT_OBJECT_0);
//}

//void WorkWait(void)
//{
//	const bool bCaching = !(g_Plugin.FlagsWork & FW_JUMP_DISABLED)
//		&& (g_Panel.IsRealNames() ? g_Plugin.bCachingRP : g_Plugin.bCachingVP);
//		//&& (g_Plugin.FarPanelInfo.Flags & PFLAGS_REALNAMES ? g_Plugin.bCachingRP : g_Plugin.bCachingVP);
//	g_Plugin.ImageX = NULL;
//	//g_Plugin.ddsx = NULL;
//
//	if (g_Plugin.bUncachedJump)
//	{
//		g_Plugin.bUncachedJump = false;
//		if (SwitchImages(g_Plugin.FlagsDisplay & FD_JUMP_NEXT))
//		{
//			if (g_Plugin.FlagsDisplay & FD_HOME_END) // invalidate incorrect cache
//				if (g_Plugin.FlagsDisplay & FD_JUMP_NEXT)
//				{
//					g_Plugin.Image[1]->DisplayClose();
//					if (g_Plugin.Image[1]->Decoder)
//					{
//						g_Plugin.Image[1]->Decoder->Close();
//						//deletex(g_Plugin.Image[1]->Decoder);
//					}
//				}
//				else
//				{
//					g_Plugin.Image[2]->DisplayClose();
//					if (g_Plugin.Image[2]->Decoder)
//					{
//						g_Plugin.Image[2]->Decoder->Close();
//						//deletex(g_Plugin.Image[2]->Decoder);
//					}
//				}
//			SetEvent(g_Plugin.hWorkEvent);
//			//Wait ForSingleObject(g_Plugin.hDisplayEvent, INFINITE);
//			if (!WaitDisplayEvent())
//				return;
//			if (bCaching)
//				return;
//		}
//		else
//		{
//			g_Plugin.FlagsDisplay &= ~(FD_JUMP | FD_HOME_END);
//			SetEvent(g_Plugin.hWorkEvent);
//		}
//	}
//
//	for (;;)
//	{
//		// В некоторых случаях FAR пытается восстановить консольный курсор.
//		// Если он видим - гасим и Invalidate
//		AutoHideConsoleCursor();
//
//		//Wait ForSingleObject(g_Plugin.hDisplayEvent, INFINITE);
//		if (!WaitDisplayEvent())
//			return;
//
//		if (g_Plugin.FlagsDisplay & FD_REFRESH)
//			return;
//
//		if (g_Plugin.FlagsWork & FW_SHOW_CONFIG)
//		{
//			_ASSERT(FALSE); // Сюда мы больше попадать не должны?
//			_ASSERTE(g_Plugin.FlagsWork & FW_SHOW_CONFIG);
//			
//			//TRUE, если настройки были изменены
//			const bool reconfig = ConfigureW(0);
//			
//			SetEvent(g_Plugin.hWorkEvent);
//			//Wait ForSingleObject(g_Plugin.hDisplayEvent, INFINITE);
//			if (!WaitDisplayEvent())
//				return;
//
//			if (!reconfig)
//				continue;
//
//			TODO("А в [0]-m DisplayClose не нужен?");
//			g_Plugin.Image[1]->DisplayClose();
//			g_Plugin.Image[2]->DisplayClose();
//			for (uint i = 3; --i;)
//				if (g_Plugin.Image[i]->Decoder)
//				{
//					g_Plugin.Image[i]->Decoder->Close();
//					//deletex(g_Plugin.Image[i]->Decoder);
//				}
//			g_Plugin.Image[1]->SetPanelItemRaw(g_Plugin.Image[0]->PanelItemRaw());
//			g_Plugin.Image[2]->SetPanelItemRaw(g_Plugin.Image[0]->PanelItemRaw());
//			g_Plugin.ImageX = g_Plugin.Image[2];
//			//g_Plugin.ddsx = g_Plugin.Image[2]->dds;
//		}
//		else if (g_Plugin.FlagsDisplay & FD_HOME_END)
//		{
//			if (g_Plugin.Image[0]->PanelItemRaw() == (g_Plugin.FlagsDisplay & FD_JUMP_NEXT ? 0 : g_Plugin.nPanelItems - 1))
//			{
//				g_Plugin.FlagsDisplay &= ~FD_HOME_END;
//				SetEvent(g_Plugin.hWorkEvent);
//				continue;
//			}
//			_ASSERTE((g_Plugin.FlagsDisplay & FD_REFRESH) == 0);
//			if (g_Plugin.FlagsDisplay & FD_JUMP_NEXT)
//			{
//				g_Plugin.Image[2]->DisplayClose();
//				if (g_Plugin.Image[2]->Decoder)
//				{
//					g_Plugin.Image[2]->Decoder->Close();
//					//deletex(g_Plugin.Image[2]->Decoder);
//				}
//				// Ниже по коду (OpenImagePlugin) индекс будет (++) так что тут будет 0
//				g_Plugin.Image[2]->SetPanelItemRaw(-1);
//				g_Plugin.ImageX = g_Plugin.Image[2];
//				//g_Plugin.ddsx = g_Plugin.Image[2]->dds;
//			}
//			else
//			{
//				g_Plugin.Image[1]->DisplayClose();
//				if (g_Plugin.Image[1]->Decoder)
//				{
//					g_Plugin.Image[1]->Decoder->Close();
//					//deletex(g_Plugin.Image[1]->Decoder);
//				}
//				// Ниже по коду (OpenImagePlugin) индекс будет (--) так что тут будет (ItemsNumber-1)
//				g_Plugin.Image[1]->SetPanelItemRaw(g_Plugin.FarPanelInfo.ItemsNumber);
//				g_Plugin.ImageX = g_Plugin.Image[1];
//				//g_Plugin.ddsx = g_Plugin.Image[1]->dds;
//			}
//			g_Plugin.bUncachedJump = true;
//		}
//		else if (g_Plugin.FlagsDisplay & FD_JUMP)
//		{
//			int nIndex = g_Plugin.FlagsDisplay & FD_JUMP_NEXT ? 2 : 1;
//			if (g_Plugin.Image[nIndex]->PanelItemRaw() == PANEL_ITEM_UNAVAILABLE)
//			{
//				g_Plugin.FlagsDisplay &= ~FD_JUMP;
//				SetEvent(g_Plugin.hWorkEvent);
//				continue;
//			}
//			else
//				if (SwitchImages(g_Plugin.FlagsDisplay & FD_JUMP_NEXT))
//				{
//					SetEvent(g_Plugin.hWorkEvent);
//					//Wait ForSingleObject(g_Plugin.hDisplayEvent, INFINITE);
//					if (!WaitDisplayEvent())
//						return;
//					if (!bCaching)
//						continue;
//				}
//				else
//					g_Plugin.bUncachedJump = true;
//		}
//		else
//		{
//			g_Plugin.ImageX = g_Plugin.Image[0];
//			//g_Plugin.ddsx = g_Plugin.Image[0]->dds;
//		}
//		break;
//	}
//}


//EFlagsResult OpenImagePlugin(const wchar_t *pFileName);

#ifdef FAR_UNICODE

struct AnalyseInfo g_LastAnalyse = {};

#if FARMANAGERVERSION_BUILD>=2462
	#define AnalyseRet HANDLE
#else
	#define AnalyseRet int
#endif

AnalyseRet WINAPI AnalyseW(const struct AnalyseInfo *Info)
{
	if ((Info->OpMode & (OPM_SILENT|OPM_FIND|OPM_COMMANDS)) != 0)
	{
		return (AnalyseRet)FALSE;
	}

	g_Plugin.InitHooks();

	if (!Info->FileName || !*Info->FileName || !g_Plugin.bHookArc)
	{
		return (AnalyseRet)FALSE;
	}

	if (g_Plugin.IsExtensionIgnored(Info->FileName))
	{
		return (AnalyseRet)FALSE;
	}

	// Запомнить
	g_LastAnalyse = *Info;
	g_LastAnalyse.Buffer = NULL;

	return (AnalyseRet)TRUE;
}

#else

HANDLE WINAPI OpenFilePluginW(const wchar_t *pFileName, const unsigned char *buf, int lBuf, int OpMode)
{
	if (!nFarVersion || ((HIWORD(nFarVersion) > 1144) && (HIWORD(nFarVersion) < 1148)))
		return INVALID_HANDLE_VALUE;
		
	// Раз мы в FAR2 - вполне можно опредлить откуда пытается быть вызван вход в файл
	if ((OpMode & (OPM_SILENT|OPM_FIND)) != 0)
		return INVALID_HANDLE_VALUE;

	// Не нужно реагировать на ShiftF3?
	BOOL lbShift = IsKeyPressed(VK_SHIFT);
	if (lbShift) {
		BOOL lbF3 = IsKeyPressed(VK_F3);
		if (lbF3) {
			BOOL lbControl = IsKeyPressed(VK_CONTROL);
			BOOL lbAlt = IsKeyPressed(VK_MENU);
			if (!lbControl && !lbAlt)
				return INVALID_HANDLE_VALUE;
		}
	}

	// gnMainThreadId инициализируется в DllMain
	_ASSERTE(gnMainThreadId!=0); // плагин может активироваться НЕ в главной нити
	//if (!gnMainThreadId)
	//	gnMainThreadId = GetMainThreadId();
		
	g_Plugin.InitHooks();

	TODO("pFileName всегда должно быть или нет?");
	if (!pFileName || !*pFileName || !g_Plugin.bHookArc)
		return INVALID_HANDLE_VALUE;
		//|| g_StartupInfo.AdvControl(PluginNumber, ACTL_CONSOLEMODE, (void*)FAR_CONSOLE_GET_MODE) == FAR_CONSOLE_FULLSCREEN 
		//|| (!g_Plugin.bHookArc && !g_Plugin.hDisplayThread && !(g_Plugin.FlagsWork & (FW_VE_HOOK | FW_PLUGIN_CALL))))
		//return INVALID_HANDLE_VALUE;

	if (g_Plugin.IsExtensionIgnored(pFileName))
		return INVALID_HANDLE_VALUE;
		
	g_Plugin.InitPlugin();
	
	FUNCLOGGERS(L"OpenFilePluginW(%s)", pFileName);


	EFlagsResult eprc = EntryPoint(OPEN_BY_USER, pFileName);

	if (eprc == FE_PROCEEDED || eprc == FE_RETRIEVE_FILE)
	{
		return (HANDLE)-2; // Чтобы FAR не пытался открыть файл другим плагином
	}

	return INVALID_HANDLE_VALUE;


	//// Если это первый вызов серии
	//if (!(g_Plugin.FlagsWork & FW_PLUGIN_CALL)) {
	//	g_Plugin.FlagsWork |= FW_FIRST_CALL; // ставим флаг (влияет на включение кнопки-модификатора)
	//	g_Plugin.FlagsWork &= ~(FW_JUMP_DISABLED);
	//	g_Plugin.SelectionChanged = false; // Чтобы знать, что менялось выделение
	//}

	//// Сброс точно ненужных
	//g_Plugin.FlagsWork &= ~(FW_TERMINATE|FW_QUICK_VIEW);

	//g_Plugin.SaveTitle();

	//g_Plugin.MapFileName = pFileName;

	//HANDLE hPlugin = INVALID_HANDLE_VALUE;
	//EFlagsResult rc;
	//
	//TRY {
	//	rc = OpenImagePlugin(g_Plugin.MapFileName);
	//	CFunctionLogger::FunctionLogger(L"OpenFilePluginW.OpenImagePlugin_done");
	//	if ((g_Plugin.FlagsWork & FW_PLUGIN_CALL) && g_Plugin.hWnd && IsWindow(g_Plugin.hWnd)) {
	//		TODO("Прыгать по плагиновым панелям получается плохо - при ошибках нить не завершается");
	//		if (rc == FE_UNKNOWN_EXTENSION) {
	//			g_Plugin.FlagsWork |= FW_TERMINATE;
	//			PostMessage(g_Plugin.hWnd, DMSG_KEYBOARD, VK_ESCAPE, 0);
	//			SetEvent(g_Plugin.hWorkEvent);
	//			SetEvent(g_Plugin.hSynchroDone);
	//			WARNING("!!! от INFINITE нужно избавляться !!!"); // Если нить не завершается - она может висеть и ее нужно насильно прибить
	//			CFunctionLogger::FunctionLogger(L"OpenFilePluginW.WaitForSingleObject(g_Plugin.hDisplayThread, INFINITE)");
	//			WaitForSingleObject(g_Plugin.hDisplayThread, INFINITE);
	//			CFunctionLogger::FunctionLogger(L"OpenFilePluginW.WaitForSingleObject(g_Plugin.hDisplayThread, INFINITE) done");
	//			CloseHandle(g_Plugin.hDisplayThread);
	//			g_Plugin.hDisplayThread = NULL;
	//			ResetEvent(g_Plugin.hWorkEvent);
	//			ResetEvent(g_Plugin.hDisplayEvent);
	//		}
	//	}
	//	switch (rc) {
	//		case FE_PROCEEDED:
	//			// Если (g_Plugin.FlagsWork & FW_PLUGIN_CALL) то в FAR мог быть послан Ctrl-PgDn
	//			CFunctionLogger::FunctionLogger(L"OpenFilePluginW.FE_PROCEEDED");
	//			hPlugin = (HANDLE)-2; // Чтобы FAR не пытался открыть файл другим плагином
	//			break;
	//		case FE_SKIP_ERROR:
	//			CFunctionLogger::FunctionLogger(L"OpenFilePluginW.FE_SKIP_ERROR");
	//			break; // Не открыли, но ошибку показывать не нужно
	//		case FE_UNKNOWN_EXTENSION:
	//			// Неизвестное расширение
	//			//if (g_Plugin.FlagsWork & FW_FIRST_CALL) {
	//			//	g_StartupInfo.Message(PluginNumberMsg, FMSG_ALLINONE|FMSG_MB_OK,
	//			//		NULL, (const wchar_t * const *)GetMsg(MIOpenUnknownExtension), 0, 0);
	//			//}
	//			CFunctionLogger::FunctionLogger(L"OpenFilePluginW.FE_UNKNOWN_EXTENSION");
	//			break;
	//		case FE_OPEN_FAILED:
	//			// Ошибка открытия файла
	//			CFunctionLogger::FunctionLogger(L"OpenFilePluginW.FE_OPEN_FAILED");
	//			if (g_Plugin.FlagsWork & FW_FIRST_CALL) {
	//				g_StartupInfo.Message(PluginNumberMsg, FMSG_ALLINONE|FMSG_MB_OK,
	//					NULL, (const wchar_t * const *)GetMsg(MIOpenFailedMsg), 0, 0);
	//			}
	//			break;
	//		case FE_NO_PANEL_ITEM:
	//			// Больше нет элементов на панели?
	//			CFunctionLogger::FunctionLogger(L"OpenFilePluginW.FE_NO_PANEL_ITEM");
	//			break;
	//	}

	//	if ((g_Plugin.FlagsDisplay & FD_JUMP) && !(g_Plugin.FlagsWork & FW_TERMINATE)) {
	//		hPlugin = (HANDLE)-2; // Чтобы FAR не пытался открыть файл другим плагином
	//	}

	//} CATCH {
	//	CFunctionLogger::FunctionLogger(L"OpenFilePluginW.OpenImagePlugin_raised an exception");
	//	g_StartupInfo.Message(PluginNumberMsg, FMSG_ALLINONE|FMSG_MB_OK,
	//		NULL,
	//		(const wchar_t * const *)GetMsg(MIOpenImgPlugException),
	//		0, 0);
	//	rc = FE_EXCEPTION;
	//}

	//if ((g_Plugin.FlagsWork & FW_TERMINATE) && g_Plugin.FlagsWork != FW_TERMINATE) {
	//	_ASSERTE(g_Plugin.FlagsWork == FW_TERMINATE);
	//	g_Plugin.FlagsWork = FW_TERMINATE;
	//}

	//if (g_Plugin.FlagsWork & FW_TERMINATE)
	//{
	//	g_Plugin.RestoreTitle();

	//	// GFL портит консоль при просмотре WMF
	//	g_StartupInfo.AdvControl(PluginNumber, ACTL_REDRAWALL, NULL);

	//	TODO("Имеет смысл, только если выбрана настройка 'Показывать выделенные файлы вверху'");
	//	if (g_Plugin.SelectionChanged) {
	//		ActlKeyMacro km = {MCMD_POSTMACROSTRING};
	//		km.Param.PlainText.SequenceText = L"CtrlR";
	//		g_StartupInfo.AdvControl(PluginNumber, ACTL_KEYMACRO, &km);
	//		g_Plugin.SelectionChanged = false;
	//	}

	//	g_Panel.CheckAllClosed();
	//}
	//
	//return hPlugin;
}

#endif

//EFlagsResult OpenImagePlugin(const wchar_t *pFileName)
//{
//	g_Plugin.InitHooks();
//
//	// Уже должно быть выполнено! Да и при вызове через префикс этого лучше не делать
//	//if (g_Plugin.IsExtensionIgnored(pFileName))
//	//	return (HANDLE)-1;
//
//	g_Plugin.InitPlugin();
//
//	// Для первой картинки серии - установить флаг FW_FIRST_CALL, но не FW_FORCE_DECODE,
//	// т.к. сюда мы можем попасть и при перехвате вьювера/редактора, а там FW_FORCE_DECODE не нужен
//	// Первую картинку серии определяем по осутствию окна g_Plugin.hWnd и НЕ QView.
//	// Не QView, т.к. там hWnd может закрываться между картинками, а сбрасывать кеш не нужно (если он есть)
//	if (g_Plugin.hWnd == NULL && !(g_Plugin.FlagsWork & FW_QUICK_VIEW)) {
//		g_Plugin.FlagsWork |= FW_FIRST_CALL;
//	}
//
//	// Дополнительная инициализация при вызове для первой картинки серии
//	// (т.к. InitPlugin вызывается только один раз за сеанс FAR-а)
//	if (g_Plugin.FlagsWork & FW_FIRST_CALL) {
//		g_Panel.CheckAllClosed();
//		g_Plugin.ImageX = g_Plugin.Image[0];
//
//		g_Panel.FreeAllItems();
//		g_Panel.Initialize(pFileName);
//	}
//
//	MCHKHEAP;
//
//	if ((g_Plugin.FlagsWork & FW_FORCE_DECODE) == 0) {
//		if (!CPVDManager::IsSupportedExtension(pFileName))
//			return FE_UNKNOWN_EXTENSION;
//	}
//
//	g_Plugin.FlagsWork &= ~FW_TERMINATE;
//
//	_ASSERTE(g_Plugin.hConEmuCtrlPressed==NULL && g_Plugin.hConEmuShiftPressed==NULL);
//
//	CheckConEmu();
//	//HWND hNewFarWnd = GetConEmuHwnd();
//	//if (!g_Plugin.hFarWnd || !g_Plugin.hParentWnd 
//	//	|| (hNewFarWnd && hNewFarWnd != g_Plugin.hFarWnd)
//	//	|| (g_Plugin.hConEmuWnd && !IsWindow(g_Plugin.hConEmuWnd))
//	//	|| (hNewFarWnd && !g_Plugin.hConEmuWnd)
//	//	)
//	//{
//	//	g_Plugin.hParentWnd = g_Plugin.hDesktopWnd;
//	//	if (hNewFarWnd) {
//	//		g_Plugin.hFarWnd = hNewFarWnd;
//	//		g_Plugin.hConEmuWnd = GetParent(g_Plugin.hFarWnd);
//	//		TCHAR szName[64]; DWORD dwPID = GetCurrentProcessId();
//	//		wsprintf(szName, CEKEYEVENT_CTRL, dwPID);
//	//		g_Plugin.hConEmuCtrlPressed = OpenEvent(SYNCHRONIZE, FALSE, szName);
//	//		wsprintf(szName, CEKEYEVENT_SHIFT, dwPID);
//	//		g_Plugin.hConEmuShiftPressed = OpenEvent(SYNCHRONIZE, FALSE, szName);
//	//	} else {
//	//		g_Plugin.hConEmuWnd = NULL;
//	//		g_Plugin.hFarWnd = (HWND)g_StartupInfo.AdvControl(PluginNumber, ACTL_GETFARHWND, NULL);
//	//		//const HWND hAncestor = GetAncestor(g_Plugin.hFarWnd, GA_PARENT);
//	//		//if (hAncestor != g_Plugin.hDesktopWnd) -- пережитки старых conemu. убито
//	//		//	g_Plugin.hFarWnd = hAncestor;
//	//	}
//	//}
//
//	TODO("Окно отрисовки в conemu может стать невидимым");
//	bool lbVisible = false;
//	if (!g_Plugin.hConEmuWnd && (!IsWindow(g_Plugin.hFarWnd) || !IsWindowVisible(g_Plugin.hFarWnd))) {
//		g_StartupInfo.Message(PluginNumberMsg, FMSG_ALLINONE|FMSG_MB_OK,
//			NULL,
//			(const wchar_t * const *)GetMsg(MIConEmuPluginWarning),
//			0, 0);
//		return FE_SKIP_ERROR;
//	}
//
//	g_Plugin.InitCMYK(FALSE); // Дождаться его завершения
//
//	MCHKHEAP;
//	//g_Plugin.FlagsWork |= FW_FIRSTIMAGE;
//	
//	TODO("???");
//	if (g_Plugin.ImageX != g_Plugin.Image[0] 
//		&& (g_Plugin.hParentWnd == g_Plugin.hFarWnd || g_Plugin.hParentWnd == g_Plugin.hConEmuWnd))
//	{
//		TitleRepaint();
//		if (g_Plugin.hWnd)
//			InvalidateRect(g_Plugin.hWnd, NULL, FALSE);
//	}
//
//	MCHKHEAP;
//
//	// ***************************************************************
//
//	_ASSERTE(g_Plugin.ImageX);
//	if (g_Plugin.ImageX)
//		g_Plugin.ImageX->Decoder->ResetProcessed();
//
//	if (g_Plugin.FlagsWork & FW_FIRST_CALL) {
//		g_Plugin.ZoomAuto = g_Plugin.bAutoZoom ? ZA_FIT : ZA_NONE;
//
//		if (!g_Plugin.ZoomAuto) {
//			g_Plugin.Zoom = g_Plugin.AbsoluteZoom = 0x10000; // initial 100%
//		}
//	} else if (!g_Plugin.bKeepZoomAndPosBetweenFiles) {
//		g_Plugin.ZoomAuto = g_Plugin.bAutoZoom ? ZA_FIT : ZA_NONE;
//		WARNING("Почему-то ветки различаются?  if (!g_Plugin.ZoomAuto) {...");
//		g_Plugin.Zoom = g_Plugin.AbsoluteZoom = 0x10000; // initial 100%
//	}
//
//	//unsigned char tempBuf[128];
//	PluginPanelItem *ppi = NULL;
//	//uint iSubDecoder = 0;
//lReOpen:
////lReOpen2:
//	MCHKHEAP;
//	if (!g_Plugin.MapFileName.IsEmpty() && pFileName != (const wchar_t*)g_Plugin.MapFileName) {
//		pFileName = (const wchar_t*)g_Plugin.MapFileName;
//	}
//	_ASSERTE(g_Plugin.ImageX!=NULL);
//	MCHKHEAP;
//	//заменил Image[0] на ImageX - иначе вроде PgDn/PgUp не работает
//	CFunctionLogger::FunctionLogger(L"OpenImagePlugin_ImageX->ImageOpen(%s)",pFileName);
//	bool result = g_Plugin.ImageX->ImageOpen(pFileName, NULL, 0);
//	CFunctionLogger::FunctionLogger(L"ImageOpen done");
//
//	// Сброс флага перехвата CtrlPgDn, и первого файла серии
//	// После AltPgDn/Up и отработки в ImageOpen - сбросим флаги (FW_PREVDECODER|FW_NEXTDECODER)
//	g_Plugin.FlagsWork &= ~(FW_PREVDECODER|FW_NEXTDECODER);
//	if (result)
//		g_Plugin.FlagsWork &= ~(FW_PLUGIN_CALL|FW_FIRST_CALL|FW_FORCE_DECODE);
//	else if (g_Plugin.FlagsWork & FW_FIRST_CALL) {
//		g_Plugin.FlagsWork |= (FW_TERMINATE);
//		// при обломе стартовой загрузки изображения нить дисплея ждет
//		// WaitForSingleObject(g_Plugin.hWorkEvent, INFINITE);
//		//SetEvent(g_Plugin.hWorkEvent); -- и пусть ждет. еще может дисплеи закрывать нужно, а окна уже нет
//	}
//
//	if (g_Plugin.FlagsWork & FW_QUICK_VIEW) {
//		g_Plugin.RestoreTitle();
//	}
//
//	g_Plugin.MapFileName = L"";
//	const bool bCaching = result 
//		&& !(g_Plugin.FlagsWork & FW_JUMP_DISABLED)
//		&& (g_Panel.IsRealNames() ? g_Plugin.bCachingRP : g_Plugin.bCachingVP);
//		//&& (g_Plugin.FarPanelInfo.Flags & PFLAGS_REALNAMES ? g_Plugin.bCachingRP : g_Plugin.bCachingVP);
//
//lCachingDone:
//
//	WARNING("GetAsyncKeyState нужно заменять на статус. иначе это потенциальный баг - она может вернуть не то, что ожидается");
//	const bool bESC = EscapePressed();
//		//GetAsyncKeyState(VK_ESCAPE) < 0;
//	if (!g_Plugin.hWnd || bESC) // Display thread ended
//	{
//		g_Plugin.ImageX = g_Plugin.Image[0];
//		//g_Plugin.ddsx = g_Plugin.Image[0]->dds;
//		result = false;
//		if (bESC)
//			FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE)/*g_Plugin.hInput*/);
//	}
//
//	MCHKHEAP;
//
//	if (result)
//	{
//		if (g_Plugin.ImageX == g_Plugin.Image[0]) // main image done
//		{
//			SetEvent(g_Plugin.hWorkEvent);
//			//Wait ForSingleObject(g_Plugin.hDisplayEvent, INFINITE);
//			if (!WaitDisplayEvent()) {
//				_ASSERT(FALSE);
//				return FE_OPEN_FAILED;
//			}
//			if (bCaching && !g_Plugin.Image[2]->pDraw && g_Plugin.Image[2]->PanelItemRaw() != PANEL_ITEM_UNAVAILABLE)
//			{
//				g_Plugin.Image[2]->SetPanelItemRaw(g_Plugin.Image[0]->PanelItemRaw());
//				g_Plugin.ImageX = g_Plugin.Image[2];
//				//g_Plugin.ddsx = g_Plugin.Image[2]->dds;
//			}
//			else if (bCaching && !g_Plugin.Image[1]->pDraw && g_Plugin.Image[1]->PanelItemRaw() != PANEL_ITEM_UNAVAILABLE)
//			{
//				g_Plugin.Image[1]->SetPanelItemRaw(g_Plugin.Image[0]->PanelItemRaw());
//				g_Plugin.ImageX = g_Plugin.Image[1];
//				//g_Plugin.ddsx = g_Plugin.Image[1]->dds;
//			}
//			else
//				WorkWait();
//		}
//		else
//		{
//			if (WaitForSingleObject(g_Plugin.hDisplayEvent, 0) == WAIT_OBJECT_0)
//			{
//				SetEvent(g_Plugin.hDisplayEvent);
//				WorkWait();
//			}
//			else
//			{
//				if (g_Plugin.ImageX == g_Plugin.Image[2]) // forward caching done
//				{
//					if (bCaching && !g_Plugin.bUncachedJump && !g_Plugin.Image[1]->pDraw && g_Plugin.Image[1]->PanelItemRaw() != PANEL_ITEM_UNAVAILABLE)
//					{
//						g_Plugin.Image[1]->SetPanelItemRaw(g_Plugin.Image[0]->PanelItemRaw());
//						g_Plugin.ImageX = g_Plugin.Image[1];
//						//g_Plugin.ddsx = g_Plugin.Image[1]->dds;
//					}
//					else
//						WorkWait();
//				}
//				else // backward caching done
//				{
//					if (bCaching && !g_Plugin.bUncachedJump && !g_Plugin.Image[2]->pDraw && g_Plugin.Image[2]->PanelItemRaw() != PANEL_ITEM_UNAVAILABLE)
//					{
//						g_Plugin.Image[2]->SetPanelItemRaw(g_Plugin.Image[0]->PanelItemRaw());
//						g_Plugin.ImageX = g_Plugin.Image[2];
//						//g_Plugin.ddsx = g_Plugin.Image[2]->dds;
//					}
//					else
//						WorkWait();
//				}
//			}
//		}
//	}
//
//	MCHKHEAP;
//
//	CFunctionLogger::FunctionLogger(L"OpenImagePlugin_1");
//
//	if ((g_Plugin.FlagsWork & FW_TERMINATE) || (g_Plugin.ImageX == g_Plugin.Image[0] && !(g_Plugin.FlagsDisplay & FD_REFRESH))) // no main image
//	{
//		// Сюда мы по идее должны попадать только при закрытии плагина - окно уже должно быть разрушено
//		// Если это только не первый вызов - тогда оно могло еще не успеть разрушиться
//		//#ifdef _DEBUG
//		//if (!(g_Plugin.FlagsWork & FW_FIRST_CALL)) {
//		//	_ASSERTE(g_Plugin.hWnd == NULL); -- низя.
//		//}
//		//#endif
//
//		TODO("А если какая-то картинка обломается - result не сбросится случайно? Viewer может тогда остаться");
//		if (result)
//			ExitViewerEditor();
//		EFlagsResult ExitCode = FE_SKIP_ERROR;
//		if (g_Plugin.hDisplayThread && g_Plugin.FlagsDisplay & FD_DISLPAYED) // exit after display
//		{
//			ExitCode = FE_PROCEEDED;
//			if (g_Plugin.Image[0]->PanelItemRaw() != PANEL_ITEM_UNAVAILABLE)
//			{
//				PanelRedrawInfo pri = {g_Plugin.Image[0]->PanelItemRaw(), g_Plugin.FarPanelInfo.TopPanelItem};
//				g_StartupInfo.PanelControl(INVALID_HANDLE_VALUE, FCTL_REDRAWPANEL, 0, (LONG_PTR)&pri);
//			}
//		}
//
//		CFunctionLogger::FunctionLogger(L"OpenImagePlugin_2");
//
//		g_Plugin.Image[0]->DisplayClose();
//		g_Plugin.Image[1]->DisplayClose();
//		g_Plugin.Image[2]->DisplayClose();
//		for (uint i = 3; i--;)
//			if (g_Plugin.Image[i]->Decoder)
//			{
//				g_Plugin.Image[i]->Decoder->Close();
//				//deletex(g_Plugin.Image[i]->Decoder);
//			}
//
//		g_Plugin.Image[1]->SetPanelItemRaw(0);
//		g_Plugin.Image[2]->SetPanelItemRaw(0);
//		g_Plugin.FarPanelInfo.ItemsNumber = -1;
//
//		// Очистить все флаги КРОМЕ ..., иначе мы не узнаем закрываемся мы или нет
//		WARNING("Если мы в архиве уже что-то открыли - то при обломе на втором файле перейти к следующему");
//		g_Plugin.FlagsWork &= (FW_TERMINATE);
//
//		MCHKHEAP;
//
//		CFunctionLogger::FunctionLogger(L"OpenImagePlugin_3");
//
//		CloseHandle(g_Plugin.hDisplayThread);
//		g_Plugin.hDisplayThread = NULL;
//
//		g_Plugin.bUncachedJump = false;
//		if (g_Plugin.hWnd)
//			SetEvent(g_Plugin.hWorkEvent);
//
//		MCHKHEAP;
//
//		free(ppi);
//
//		CFunctionLogger::FunctionLogger(L"OpenImagePlugin_4end");
//
//		MCHKHEAP;
//		return ExitCode;
//	}
//
//	CFunctionLogger::FunctionLogger(L"OpenImagePlugin_5");
//
//	// caching in progress
//	if (g_Plugin.FlagsDisplay & FD_REFRESH)
//	{
//		// Ниже по коду это условие. для чего оно?
//		//_ASSERTE(g_Plugin.ImageX != g_Plugin.Image[0]);
//
//		CFunctionLogger::FunctionLogger(L"OpenImagePlugin_6");
//
//		// Тут как-то коряво, но это нужно, чтобы сработал refresh после применения настроек диалога на лету
//		////////////////////////////////////////////////
//		// !!! g_Plugin.Image[0] тут не обрабатывается
//		for (uint i = 3; --i;)
//			if (g_Plugin.Image[i]->Decoder)
//			{
//				g_Plugin.Image[i]->DisplayClose();
//				g_Plugin.Image[i]->Decoder->ResetProcessed();
//				g_Plugin.Image[i]->Decoder->Close();
//				//deletex(g_Plugin.Image[i]->Decoder);
//			}
//			g_Plugin.Image[1]->SetPanelItemRaw(g_Plugin.Image[0]->PanelItemRaw());
//			g_Plugin.Image[2]->SetPanelItemRaw(g_Plugin.Image[0]->PanelItemRaw());
//			g_Plugin.ImageX = g_Plugin.Image[0];
//		g_Plugin.Image[0]->Decoder->ResetProcessed();
//		g_Plugin.FlagsWork |= FW_FORCE_DECODE; // Чтобы пошел "усиленный" подбор декодера
//		////////////////////////////////////////////////
//		MCHKHEAP;
//
//		if (g_Plugin.ImageX != g_Plugin.Image[0])
//		{
//			g_Plugin.bUncachedJump = false;
//			WorkWait();
//		}
//		DWORD nNextDecoder = (g_Plugin.FlagsWork & (FW_PREVDECODER|FW_NEXTDECODER));
//		if (nNextDecoder)
//			g_Plugin.FlagsWork |= FW_FORCE_DECODE; // Чтобы пошел "усиленный" подбор декодера
//		g_Plugin.FlagsDisplay &= ~FD_REFRESH; //FW_PREVDECODER|FW_NEXTDECODER - это оставим
//		g_Plugin.FlagsWork |= FW_PLUGIN_CALL;
//		//g_Plugin.Image[0]->dds->DeleteWorkSurface();
//		//g_Plugin.Image[1]->dds->DeleteWorkSurface();
//		//g_Plugin.Image[2]->dds->DeleteWorkSurface();
//		for (uint i = 3; i--;) {
//			g_Plugin.Image[i]->DisplayClose();
//			if (g_Plugin.Image[i]->Decoder)
//			{
//				g_Plugin.Image[i]->Decoder->Close();
//				//deletex(g_Plugin.Image[i]->Decoder);
//			}
//		}
//
//		MCHKHEAP;
//			
//		/*if (nNextDecoder)
//			iSubDecoder = CPVDManager::GetNextDecoder(iSubDecoder, ((nNextDecoder & FD_NEXTDECODER) != 0));*/
//			
//		if (g_Plugin.FlagsWork & FW_JUMP_DISABLED)
//		{
//			//g_Plugin.MapFileName = g_Plugin.Image[0]->FileName;
//			//lstrcpyW(g_Plugin.MapFileNameData, g_Plugin.Image[0]->FileNameData);
//			g_Plugin.MapFileName = (const wchar_t*)g_Plugin.Image[0]->FileName;
//			goto lReOpen;
//		}
//	}
//	else
//	{
//		MCHKHEAP;
//
//		CFunctionLogger::FunctionLogger(L"OpenImagePlugin_7");
//
//		do {
//			// крутим индекс открываемого элемента на панели
//			_ASSERTE(g_Plugin.ImageX!=NULL);
//			g_Plugin.ImageX->SetPanelItemRaw( g_Plugin.ImageX->PanelItemRaw() + ((g_Plugin.ImageX == g_Plugin.Image[2]) ? 1 : -1));
//			g_Plugin.ImageX->Decoder->ResetProcessed();
//			if ((int)g_Plugin.ImageX->PanelItemRaw() < 0 || (int)g_Plugin.ImageX->PanelItemRaw() >= g_Plugin.FarPanelInfo.ItemsNumber)
//				if (!g_Plugin.bLoopJump || (int)g_Plugin.Image[0]->PanelItemRaw() < 0 || (int)g_Plugin.Image[0]->PanelItemRaw() > g_Plugin.FarPanelInfo.ItemsNumber - 1)
//				{
//					g_Plugin.ImageX->SetPanelItemRaw(PANEL_ITEM_UNAVAILABLE);
//					result = true;
//					goto lCachingDone;
//				}
//				else
//					g_Plugin.ImageX->SetPanelItemRaw(g_Plugin.ImageX == g_Plugin.Image[2] ? 0 : g_Plugin.FarPanelInfo.ItemsNumber - 1);
//			PluginPanelItem *p;
//			size_t len;
//			if (!(len = g_StartupInfo.PanelControl(INVALID_HANDLE_VALUE, FCTL_GETPANELITEM, g_Plugin.ImageX->PanelItemRaw(), 0)) ||
//				!(p = (PluginPanelItem*)realloc(ppi, len)) ||
//				!(ppi = p, g_StartupInfo.PanelControl(INVALID_HANDLE_VALUE, FCTL_GETPANELITEM, g_Plugin.ImageX->PanelItemRaw(), (LONG_PTR)ppi)))
//				{
//					g_Plugin.ImageX->SetPanelItemRaw(PANEL_ITEM_UNAVAILABLE);
//					result = true;
//					goto lCachingDone;
//				} else {
//					g_Plugin.ImageX->bSelected = (ppi->Flags & PPIF_SELECTED) == PPIF_SELECTED;
//				}
//		} while (ppi->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
//
//		if (g_Plugin.ImageX->PanelItemRaw() == g_Plugin.Image[0]->PanelItemRaw()) // found the same image
//		{
//			g_Plugin.ImageX->SetPanelItemRaw(PANEL_ITEM_UNAVAILABLE);
//			result = true;
//			goto lCachingDone;
//		}
//
//		MCHKHEAP;
//	}
//
//	CFunctionLogger::FunctionLogger(L"OpenImagePlugin_8");
//
//	if (g_Plugin.FarPanelInfo.Flags & PFLAGS_REALNAMES)
//	{
//		if (size_t len = g_StartupInfo.PanelControl(INVALID_HANDLE_VALUE, FCTL_GETPANELITEM, g_Plugin.ImageX->PanelItemRaw(), 0))
//		{
//			if (PluginPanelItem *p = (PluginPanelItem*)realloc(ppi, len))
//			{
//				if (ppi = p, g_StartupInfo.PanelControl(INVALID_HANDLE_VALUE, FCTL_GETPANELITEM, g_Plugin.ImageX->PanelItemRaw(), (LONG_PTR)ppi))
//				{
//					//g_Plugin.MapFileName = ppi->FindData.lpwszFileName;
//					PutFilePathName(&g_Plugin.MapFileName, ppi->FindData.lpwszFileName);
//					goto lReOpen;
//				}
//			}
//		}
//	}
//
//	CFunctionLogger::FunctionLogger(L"OpenImagePlugin_9ExitViewerEditor");
//
//	MCHKHEAP;
//	ExitViewerEditor();
//	MCHKHEAP;
//	PanelRedrawInfo pri = {g_Plugin.ImageX->PanelItemRaw(), g_Plugin.FarPanelInfo.TopPanelItem};
//	g_StartupInfo.PanelControl(INVALID_HANDLE_VALUE, FCTL_REDRAWPANEL, 0, (LONG_PTR)&pri);
//
//	CFunctionLogger::FunctionLogger(L"OpenImagePlugin_10");
//
//	// макро не срабатывает - файлы не открываются, долистывается до конца
//	//ActlKeyMacro km = {MCMD_POSTMACROSTRING};
//	//km.Param.PlainText.SequenceText = L"$Text \"pic:!.!\" Enter";
//	//if (!g_StartupInfo.AdvControl(PluginNumber, ACTL_KEYMACRO, &km))
//	DWORD Command = KEY_CTRL | KEY_PGDN;
//	KeySequence ks = {KSFLAGS_DISABLEOUTPUT, 1, &Command};
//	g_Plugin.FlagsWork |= FW_PLUGIN_CALL; // Добавил на всякий случай, а то сброситься могло
//	if (!g_StartupInfo.AdvControl(PluginNumber, ACTL_POSTKEYSEQUENCE, &ks))
//	{
//		g_Plugin.ImageX->SetPanelItemRaw(PANEL_ITEM_UNAVAILABLE);
//		result = true;
//		goto lCachingDone;
//	}
//	free(ppi);
//	MCHKHEAP;
//	CFunctionLogger::FunctionLogger(L"OpenImagePlugin_end");
//	return FE_PROCEEDED;
//}

void WINAPI ClosePluginW(
		#if FAR_UNICODE>=2000
			const struct ClosePanelInfo *Info
		#else
			HANDLE hPlugin
		#endif
	)
{
}

bool ProcessPrefix(const wchar_t* Item)
{
	bool lbUnload = false, lbLoad = false;
	MCHKHEAP;
	if (!_wcsnicmp(Item, L"load:", 5)) {
		lbLoad = true;
		Item += 5;
	} else if (!_wcsnicmp(Item, L"reload:", 7)) {
		lbUnload = true; lbLoad = true;
		Item += 7;
	} else if (!_wcsnicmp(Item, L"unload:", 7)) {
		lbUnload = true;
		Item += 7;
	}
	MCHKHEAP;
	
	if (!lbUnload && !lbLoad || !*Item)
		return false;

	
	//int nLen = lstrlen(Item);
	//bool lbIsFolder = Item[nLen-1] == L'\\';

		
	CUnicodeFileName szFileName;
	PutFilePathName(&szFileName, Item, false);
	Item = (const wchar_t*)szFileName;  // это будет полный путь 
	//if (!lbIsFolder)
	//{
	//	nLen = lstrlen(Item);
	//	_ASSERTE(Item[nLen-1] == L'\\');
	//	Item[nLen-1] = 0;

	//}
	CUnicodeFileName::SkipPrefix(&Item); // а это уже без UNC префиксов
	const wchar_t* szLines[10];
	MCHKHEAP;
	
	CModuleInfo* plug = CPVDManager::FindPlugin(Item, true/*abShowErrors*/);
	if (lbUnload) {
		if (plug != NULL) {
			// Сам объект CModuleInfo не разрушается. Только выгружается pvd-шка
			plug->Unload();
		}
	}
	MCHKHEAP;

	if (lbLoad) {
		if (!plug) {
			// нужно подгрузить
			WIN32_FIND_DATAW fnd;
			HANDLE hFind = FindFirstFile(Item, &fnd);
			if (hFind == INVALID_HANDLE_VALUE
				|| ((fnd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY))
			{
				szLines[0] = g_Plugin.pszPluginTitle;
				szLines[1] = GetMsg(MISubpluginFileNotFound); //L"Subplugin file not found";
				szLines[2] = Item;																
				g_StartupInfo.Message(PluginNumberMsg, FMSG_WARNING|FMSG_MB_OK, 
					NULL, szLines, 3, 0);
			}
			else
			{
				FindClose(hFind);
				// плуг новый - будет создан новый CModuleInfo*
				wchar_t* psz = _wcsdup(Item);
				wchar_t* pszSlash = wcsrchr(psz, L'\\');
				if (pszSlash) *pszSlash = 0;
				MCHKHEAP;
				plug = CPVDManager::LoadPlugin(psz, fnd, true/*abShowErrors*/);
				free(psz);
				MCHKHEAP;
			}
		}
		if (plug && (plug->pPlugin == NULL))
		{
			if (!plug->Load(true)) {
				szLines[0] = g_Plugin.pszPluginTitle;
				szLines[1] = GetMsg(MISubpluginLoadingFailed);
				_ASSERTE(plug->pModulePath);
				szLines[2] = plug->pModulePath ? plug->pModulePath : Item;
				szLines[3] = plug->szStatus;
				g_StartupInfo.Message(PluginNumberMsg, FMSG_WARNING|FMSG_MB_OK, 
					NULL, szLines, 4, 0);
			}
			MCHKHEAP;
		}
	}

	// и сразу сортирнуть, т.к. выгружаем/загружаем только один плуг
	CPVDManager::SortPlugins2(); // Там же плагины по типам распределяются
	MCHKHEAP;

	return true;
}

// OpenW in Far3
HANDLE WINAPI OpenPluginW(
	#ifdef FAR_UNICODE
		const struct OpenInfo *Info
	#else
		int OpenFrom, INT_PTR Item
	#endif
	)
{
	#ifdef FAR_UNICODE
		int OpenFrom = Info->OpenFrom;
		INT_PTR Item = Info->Data;
		// OPEN_ANALYSE???
	#else
	
	if (!nFarVersion || ((HIWORD(nFarVersion) > 1144) && (HIWORD(nFarVersion) < 1148)))
		return INVALID_HANDLE_VALUE;
	
	#endif
		
	// gnMainThreadId инициализируется в DllMain
	_ASSERTE(gnMainThreadId!=0); // плагин может активироваться НЕ в главной нити
	//if (!gnMainThreadId)
	//	gnMainThreadId = GetMainThreadId();

	MCHKHEAP;

	// Это явный вызов плагина. Выполняем полную инициализацию
	g_Plugin.InitPlugin();

	//
	
	#ifdef FAR_UNICODE
	if ((OpenFrom & OPEN_FROM_MASK) == OPEN_ANALYSE)
	{
		#if FARMANAGERVERSION_BUILD>=2462
		OpenAnalyseInfo* p = (OpenAnalyseInfo*)Info->Data;
		_ASSERTE(p && p->StructSize>=sizeof(*p));
		g_LastAnalyse = *(p->Info);
		#endif
		
		EFlagsResult eprc = EntryPoint(OPEN_BY_USER, g_LastAnalyse.FileName);
		//if (eprc == FE_PROCEEDED || eprc == FE_RETRIEVE_FILE)
		//{
		//	//TODO: ?
		//}
		return INVALID_HANDLE_VALUE;
	}
	#endif

	if ((OpenFrom & OPEN_FROM_MASK) == OPEN_COMMANDLINE)
	{
		if (ProcessPrefix((const wchar_t*)Item)) {
			//load:, unload:, reload:
			//g_Plugin.FlagsWork &= ~(FW_FIRST_CALL|FW_FORCE_DECODE);
			return INVALID_HANDLE_VALUE;
		}
	}
	else if (
		#if FARMANAGERVERSION_BUILD>=2458
		OpenFrom == OPEN_FROMMACRO
		#else
		OpenFrom & OPEN_FROMMACRO_MASK
		#endif
		)
	{
		_ASSERTE(OpenFrom == 0);
		WARNING("Доделать!");
		return INVALID_HANDLE_VALUE;
	}

	LPCWSTR apszFile = NULL;
	CUnicodeFileName file;
	if ((OpenFrom & OPEN_FROM_MASK) == OPEN_COMMANDLINE)
	{
		TODO("Проверить, что произойдет с плагином дальше, если Item - папка, без слеша на конце?");
		PutFilePathName(&file, (LPCWSTR)Item, false);
		apszFile = (LPCWSTR)file;
	}

	// Это явный вызов пользвателем
	EFlagsResult eprc = EntryPoint((OpenFrom|OPEN_BY_USER), apszFile);


	return INVALID_HANDLE_VALUE;

	//// Явный вызов - ставим флаг (влияет на подбор декодеров, включение кнопки-модификатора)
	//g_Plugin.FlagsWork |= FW_FIRST_CALL|FW_FORCE_DECODE;
	//g_Plugin.FlagsWork &= ~(FW_TERMINATE|FW_QUICK_VIEW|FW_JUMP_DISABLED);
	//g_Plugin.SelectionChanged = false; // Чтобы знать, что менялось выделение
	//
	//
	//FUNCLOGGERI(L"OpenPluginW(%i)", OpenFrom);


	//switch (OpenFrom)
	//{
	//	case OPEN_VIEWER:
	//	case OPEN_EDITOR:
	//		{
	//			struct WindowInfo wi = {-1};
	//			wchar_t szName[0x1000]; TODO("Сделать динамическим?");
	//			wi.Name = szName; wi.NameSize =	sizeof(szName)/sizeof(szName[0]);
	//			if (g_StartupInfo.AdvControl(PluginNumber, ACTL_GETWINDOWINFO, &wi))
	//			{
	//				// -- явный вызов. Расширения игнорируем
	//				//if (g_Plugin.IsExtensionIgnored(wi.Name))
	//				//	return INVALID_HANDLE_VALUE;
	//				//if (!wcsncmp(wi.Name,L"\\\\?\\",4)) wi.Name+=4;
	//				//lstrcpynW(g_Plugin.Image[0]->FileNameData, wi.Name, sizeofarray(g_Plugin.Image[0]->FileNameData));
	//				if (!g_Panel.Initialize(wi.Name))
	//					return INVALID_HANDLE_VALUE;
	//				//2010-08-01 - уже, в g_Panel.Initialize
	//				//g_Plugin.Image[0]->FileName = wi.Name;
	//			} else {
	//				//g_Plugin.Image[0]->FileName = L"";
	//				return INVALID_HANDLE_VALUE;
	//			}
	//			//g_Plugin.FlagsWork |= FW_JUMP_DISABLED;
	//		} break;
	//	case OPEN_COMMANDLINE:
	//		if (ProcessPrefix((const wchar_t*)Item)) {
	//			//load:, unload:, reload:
	//			g_Plugin.FlagsWork &= ~(FW_FIRST_CALL|FW_FORCE_DECODE);
	//			return INVALID_HANDLE_VALUE;
	//		}
	//		// только вызовом через префикс можно обойти (наверное) IsExtensionIgnored
	//		//TestJumpDisable((const wchar_t*)Item);
	//		if (!g_Panel.Initialize((const wchar_t*)Item))
	//			return INVALID_HANDLE_VALUE;
	//		break;
	//	case OPEN_PLUGINSMENU:
	//		//TestJumpDisable(L"");
	//		if (!g_Panel.Initialize(NULL))
	//			return INVALID_HANDLE_VALUE;
	//}

	//g_Plugin.FlagsWork |= FW_PLUGIN_CALL;
	////g_Plugin.MapFileName = g_Plugin.Image[0]->FileName;
	////lstrcpy(g_Plugin.MapFileNameData, g_Plugin.Image[0]->FileNameData);
	////2010-08-01 - уже, в g_Panel.Initialize
	////g_Plugin.MapFileName = (const wchar_t*)g_Plugin.Image[0]->FileName;

	//g_Plugin.SaveTitle();

	//EFlagsResult rc;
	//
	//TRY {
	//	rc = OpenImagePlugin(g_Plugin.MapFileName);
	//	CFunctionLogger::FunctionLogger(L"OpenPluginW.OpenImagePlugin_done");
	//	switch (rc) {
	//		case FE_PROCEEDED:
	//			CFunctionLogger::FunctionLogger(L"OpenPluginW.FE_PROCEEDED");
	//			break;
	//		case FE_SKIP_ERROR:
	//			CFunctionLogger::FunctionLogger(L"OpenPluginW.FE_SKIP_ERROR");
	//			break; // Не открыли, но ошибку показывать не нужно
	//		case FE_UNKNOWN_EXTENSION:
	//			// Неизвестное расширение
	//			CFunctionLogger::FunctionLogger(L"OpenPluginW.FE_UNKNOWN_EXTENSION");
	//			if (g_Plugin.FlagsWork & FW_FIRST_CALL) {
	//				g_StartupInfo.Message(PluginNumberMsg, FMSG_ALLINONE|FMSG_MB_OK,
	//					NULL, (const wchar_t * const *)GetMsg(MIOpenUnknownExtension), 0, 0);
	//			}
	//			break;
	//		case FE_OPEN_FAILED:
	//			// Ошибка открытия файла
	//			CFunctionLogger::FunctionLogger(L"OpenPluginW.FE_OPEN_FAILED");
	//			if (g_Plugin.FlagsWork & FW_FIRST_CALL) {
	//				g_StartupInfo.Message(PluginNumberMsg, FMSG_ALLINONE|FMSG_MB_OK,
	//					NULL, (const wchar_t * const *)GetMsg(MIOpenFailedMsg), 0, 0);
	//			}
	//			break;
	//		case FE_NO_PANEL_ITEM:
	//			// Больше нет элементов на панели?
	//			CFunctionLogger::FunctionLogger(L"OpenPluginW.FE_NO_PANEL_ITEM");
	//			break;
	//	}
	//} CATCH {
	//	CFunctionLogger::FunctionLogger(L"OpenPluginW.OpenImagePlugin_raised an exception");
	//	g_StartupInfo.Message(PluginNumberMsg, FMSG_ALLINONE|FMSG_MB_OK,
	//		NULL,
	//		(const wchar_t * const *)GetMsg(MIOpenImgPlugException),
	//		0, 0);
	//}
	//MCHKHEAP;

	//if ((g_Plugin.FlagsWork & FW_TERMINATE) && g_Plugin.FlagsWork != FW_TERMINATE) {
	//	_ASSERTE(g_Plugin.FlagsWork == FW_TERMINATE);
	//	g_Plugin.FlagsWork = FW_TERMINATE;
	//}
	//
	//if (g_Plugin.FlagsWork & FW_TERMINATE)
	//{
	//	//if (rc == (HANDLE)-2) -- теперь всегда, на всякий
	//	g_Plugin.RestoreTitle();
	//	// GFL портит консоль при просмотре WMF
	//	g_StartupInfo.AdvControl(PluginNumber, ACTL_REDRAWALL, NULL);

	//	TODO("Имеет смысл, только если выбрана настройка 'Показывать выделенные файлы вверху'");
	//	if (g_Plugin.SelectionChanged) {
	//		ActlKeyMacro km = {MCMD_POSTMACROSTRING};
	//		km.Param.PlainText.SequenceText = L"CtrlR";
	//		g_StartupInfo.AdvControl(PluginNumber, ACTL_KEYMACRO, &km);
	//		g_Plugin.SelectionChanged = false;
	//	}

	//	g_Panel.CheckAllClosed();
	//}

	//return INVALID_HANDLE_VALUE;
}

int WINAPI ProcessEditorEventW(
	#ifdef FAR_UNICODE
		const struct ProcessEditorEventInfo *Info
	#else
		int Event, void *Param
	#endif
	)
{
	#ifdef FAR_UNICODE
	int Event = Info->Event;
	void *Param = Info->Param;
	#endif
	
	g_Plugin.InitHooks();

	if (Event == EE_READ && g_Plugin.bHookEdit)
	{
		// Не перехватывать CtrlShiftF4
		if (!g_Plugin.bHookCtrlShiftF4) {
			if (IsKeyPressed(VK_SHIFT) && IsKeyPressed(VK_CONTROL) /*&& (GetKeyState(VK_F4) & 0x8000)*/) {
				return 0;
			}
		}

		//EditorInfo ei;
		//if (g_StartupInfo.EditorControl(ECTL_GETINFO, &ei))
		//{
		//	wchar_t* pszFileName = NULL;
		//	int nLen = g_StartupInfo.EditorControl(ECTL_GETFILENAME, NULL);
		//	if (nLen>0 && (pszFileName = (wchar_t*)calloc(nLen+1,2))!=NULL) {
		//		g_StartupInfo.EditorControl(ECTL_GETFILENAME, pszFileName);
		//		
		//		if (!g_Plugin.IsExtensionIgnored(pszFileName)
		//			&& g_Panel.Initialize(pszFileName))
		//		{
		//			g_Plugin.InitPlugin();
		//			
		//			g_Plugin.FlagsWork |= FW_VE_HOOK;
		//			g_Plugin.FlagsWork &= ~(FW_TERMINATE|FW_QUICK_VIEW);

		//			g_Plugin.SaveTitle();

		//			//TestJumpDisable(pszFileName/*ei.FileName*/);
		//			//g_Plugin.MapFileName = g_Plugin.Image[0]->FileName;
		//			//lstrcpy(g_Plugin.MapFileNameData, g_Plugin.Image[0]->FileNameData);
		//			//g_Plugin.MapFileName = (const wchar_t*)g_Plugin.Image[0]->FileName;

		//			TODO("Флаги TERMINATED, FIRSTCALL, переменная g_Plugin.SelectionChanged");
		//			
		//			int rc = 0;
		//			
		//			TRY {
		//				rc = OpenImagePlugin(g_Plugin.MapFileName);
		//			} CATCH {
		//				CFunctionLogger::FunctionLogger(L"ProcessEditorEventW.OpenImagePlugin_raised an exception");
		//				g_StartupInfo.Message(PluginNumberMsg, FMSG_ALLINONE|FMSG_MB_OK,
		//					NULL,
		//					(const wchar_t * const *)GetMsg(MIOpenImgPlugException),
		//					0, 0);
		//			}
		//			

		//			g_Plugin.RestoreTitle();
		//		}

		//		free(pszFileName);
		//	}
		//}
	}
	return 0;
}

typedef COORD (WINAPI *GetConsoleFontSize_t)(HANDLE hConsoleOutput,DWORD nFont);
typedef BOOL (WINAPI *GetCurrentConsoleFont_t)(HANDLE hConsoleOutput,BOOL bMaximumWindow,PCONSOLE_FONT_INFO lpConsoleCurrentFont);

int WINAPI ProcessViewerEventW(
	#ifdef FAR_UNICODE
		const struct ProcessViewerEventInfo *Info
	#else
		int Event, void *Param
	#endif
	)
{
	#ifdef FAR_UNICODE
	int Event = Info->Event;
	void *Param = Info->Param;
	#endif
	
	g_Plugin.InitHooks();

	if (Event == VE_READ)
	{
		if (g_Plugin.FlagsWork & FW_IN_RETRIEVE)
		{
			if (!g_Panel.OnFileExtractedToViewer())
			{
				OnFinalTerminate(FE_VIEWER_ERROR);
				return 0;
			}
			g_Plugin.FlagsWork &= ~FW_IN_RETRIEVE;
			
			// Теперь надо бы закрыть вьювер и вернуться в плагин...
			_ASSERTE(FALSE);
			
			// А так мы наверное в макросе останемся
			EFlagsMainProcess nResult = ProcessMainThread();
			if (nResult == FEM_EXIT_PLUGIN)
			{
				OnFinalTerminate(FE_VIEWER_ERROR);
			}
			
			return 0;			
		}
		else if (g_Plugin.bHookQuickView || g_Plugin.bHookView)
		{
			// Не перехватывать CtrlShiftF3
			BOOL lbCtrlShiftPressed = (IsKeyPressed(VK_SHIFT) && IsKeyPressed(VK_CONTROL));
			//if ( /*&& (GetKeyState(VK_F3) & 0x8000)*/) {
			//	return 0;
			//}
			TCHAR szValue[128];
			if (GetEnvironmentVariable(_T("FarPicViewMode"), szValue, 128)) {
				szValue[9] = 0;
				if (lstrcmp(szValue, PICVIEW_ENVVAL) != 0) {
					// значит просмотр уже начат другим плагином
					return 0;
				}
			}
		}
	
		//ViewerInfo vi = {0};
		//vi.StructSize = sizeof(vi);
		//if (g_StartupInfo.ViewerControl(VCTL_GETINFO, &vi))
		//{
		//	g_Plugin.FlagsWork |= FW_VE_HOOK;
		//	// Сначала - сбросим QView! Иначе он может ошибочно остаться от предыдущего QView при отрытии обычного View
		//	g_Plugin.FlagsWork &= ~(FW_TERMINATE|FW_QUICK_VIEW);

		//	TODO("Хорошо бы подстраховаться через ACTL_GETWINDOWINFO");

		//	PanelInfo ppi[2];
		//	// Для определения наличия QView (он может быть как на пассивной, так и на активной панели!
		//	if (g_StartupInfo.PanelControl(PANEL_ACTIVE, FCTL_GETPANELINFO, 0, (LONG_PTR)ppi)
		//		&& g_StartupInfo.PanelControl(PANEL_PASSIVE, FCTL_GETPANELINFO, 0, (LONG_PTR)(ppi + 1)))
		//	{
		//		for (uint i = 2; i--;)
		//			if (ppi[i].PanelType == PTYPE_QVIEWPANEL && ppi[i].PanelRect.right - ppi[i].PanelRect.left - 1 == vi.WindowSizeX)
		//			{
		//				CONSOLE_SCREEN_BUFFER_INFO csbi;
		//				RECT FarRect;
		//				//HWND hConEmu = GetConEmuHwnd(), hConEmuRoot = NULL;
		//				CheckConEmu();
		//				//HWND hWnd = hConEmu;
		//				HWND hWnd = g_Plugin.hFarWnd;
		//				//if (!hWnd) {
		//				//	const HWND hFarWnd = (HWND)g_StartupInfo.AdvControl(PluginNumber, ACTL_GETFARHWND, NULL);
		//				//	hWnd = GetAncestor(hFarWnd, GA_PARENT);
		//				//	WARNING("Эта проверка сомнительна в условиях многодисплейной конфигурации");
		//				//	if (hWnd == g_Plugin.hDesktopWnd)
		//				//		hWnd = hFarWnd;
		//				//} else {
		//				//	hConEmuRoot = GetParent(hConEmu);
		//				//}
		//				if (!GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE)/*g_Plugin.hOutput*/, &csbi) 
		//					|| !GetClientRect(hWnd, &FarRect))
		//				{
		//					g_Plugin.FlagsWork &= ~FW_VE_HOOK;
		//					break;
		//				}
		//				if (ppi[i].PanelRect.left >= csbi.srWindow.Right || ppi[i].PanelRect.right <= csbi.srWindow.Left || ppi[i].PanelRect.top >= csbi.srWindow.Bottom || ppi[i].PanelRect.bottom - 2 <= csbi.srWindow.Top) {g_Plugin.FlagsWork &= ~FW_VE_HOOK; break;}
		//				uint wdx = 0, wdy = 0; // Видимо это задел SEt'а на обработку черной окантовки вокруг консоли, если окно развернуто
		//				//uint dx1 = (FarRect.right - FarRect.left + (csbi.srWindow.Right - csbi.srWindow.Left)) / (csbi.srWindow.Right - csbi.srWindow.Left + 1);
		//				uint dx0 = (csbi.srWindow.Right - csbi.srWindow.Left + 1);
		//				// Когда FAR Maximized - в консоли часть символов может не влезать
		//				uint dx = (FarRect.right - FarRect.left + dx0/2) / dx0;
		//				//uint dy1 = (FarRect.bottom - FarRect.top + (csbi.srWindow.Bottom - csbi.srWindow.Top)) / (csbi.srWindow.Bottom - csbi.srWindow.Top + 1);
		//				uint dy0 = (csbi.srWindow.Bottom - csbi.srWindow.Top + 1);
		//				uint dy = (FarRect.bottom - FarRect.top + dy0/2) / dy0;
		//				// Для реальной консоли - попробуем получить ширину и высоту через WinAPI
		//				if (!g_Plugin.hConEmuWnd) {
		//					// Эти функции есть в XP и выше
		//					GetCurrentConsoleFont_t fGetCurrentConsoleFont = (GetCurrentConsoleFont_t)GetProcAddress(GetModuleHandle(_T("kernel32.dll")),"GetCurrentConsoleFont");
		//					GetConsoleFontSize_t fGetConsoleFontSize = (GetConsoleFontSize_t)GetProcAddress(GetModuleHandle(_T("kernel32.dll")),"GetConsoleFontSize");
		//					if (fGetCurrentConsoleFont) {
		//						CONSOLE_FONT_INFO cfi = {0};
		//						BOOL bMaximized = IsZoomed(g_Plugin.hFarWnd);
		//						BOOL bFontRC = fGetCurrentConsoleFont(GetStdHandle(STD_OUTPUT_HANDLE),bMaximized,&cfi);
		//						if (bFontRC) {
		//							COORD cr = fGetConsoleFontSize(GetStdHandle(STD_OUTPUT_HANDLE), cfi.nFont);
		//							if (cr.X && cr.Y) {
		//								//_ASSERTE(dx == cr.X && dy == cr.Y);
		//								dx = cr.X;
		//								dy = cr.Y;
		//							}
		//						}
		//					}
		//				}
		//				//BUGBUG. Тут потенциально можно нарваться. Если панель будет вообще не видна из-за прокрутки
		//				g_Plugin.ViewPanelT = ppi[i].PanelRect;
		//				g_Plugin.ViewPanelG.left   = wdx + (ppi[i].PanelRect.left + 1 - csbi.srWindow.Left) * dx;
		//				g_Plugin.ViewPanelG.right  = wdx + (ppi[i].PanelRect.right - csbi.srWindow.Left) * dx;
		//				g_Plugin.ViewPanelG.top    = wdy + (ppi[i].PanelRect.top + 1 - csbi.srWindow.Top) * dy;
		//				g_Plugin.ViewPanelG.bottom = wdy + (ppi[i].PanelRect.bottom - 2 - csbi.srWindow.Top) * dy;
		//				g_Plugin.FlagsWork |= FW_QUICK_VIEW | FW_JUMP_DISABLED;
		//				if (g_Plugin.hConEmuWnd)
		//					MapWindowPoints(g_Plugin.hFarWnd, g_Plugin.hConEmuWnd, (LPPOINT)&g_Plugin.ViewPanelG, 2);
		//				break;
		//			}
		//	}

		//	// в режиме PanelViews нужно избежать излишних активаций при скроллировании!
		//	bool bSkipCauseTH = false;
		//	if (g_Plugin.hConEmuWnd && (g_Plugin.FlagsWork & FW_QUICK_VIEW))
		//	{
		//		TCHAR szEnvVar[128];
		//		if (GetEnvironmentVariable(TH_ENVVAR_NAME, szEnvVar, ARRAYSIZE(szEnvVar))) {
		//			if (!lstrcmp(szEnvVar, TH_ENVVAR_SCROLL)) {
		//				bSkipCauseTH = true;
		//			}
		//		}
		//	}
		//	
		//	if ((g_Plugin.FlagsWork & FW_QUICK_VIEW) && bSkipCauseTH)
		//		g_Plugin.FlagsWork = 0;
		//	else if (!(g_Plugin.FlagsWork & FW_QUICK_VIEW) && !g_Plugin.bHookCtrlShiftF3 && lbCtrlShiftPressed)
		//		g_Plugin.FlagsWork = 0;
		//	else if (!(g_Plugin.FlagsWork & FW_VE_HOOK) || (g_Plugin.FlagsWork & FW_QUICK_VIEW ? !g_Plugin.bHookQuickView : !g_Plugin.bHookView))
		//		g_Plugin.FlagsWork = 0;
		//	else if (!vi.FileName[0] || g_Plugin.IsExtensionIgnored(vi.FileName))
		//		g_Plugin.FlagsWork = 0;
		//	else if (!g_Panel.IsInitialized() && !g_Panel.Initialize(vi.FileName))
		//		g_Plugin.FlagsWork = 0;
		//	else
		//	{
		//		g_Plugin.InitPlugin();				

		//		g_Plugin.MapFileName = vi.FileName;
		//		
		//		//if (g_Plugin.FlagsWork & FW_QUICK_VIEW) {
		//		//	//g_Plugin.MapFileName = vi.FileName;
		//		//	//if (!wcsncmp(vi.FileName,L"\\\\?\\",4)) vi.FileName+=4;
		//		//	g_Plugin.MapFileName = vi.FileName;
		//		//}
		//		//else
		//		//{
		//		//	TestJumpDisable(vi.FileName);
		//		//	//g_Plugin.MapFileName = g_Plugin.Image[0]->FileName;
		//		//	//lstrcpy(g_Plugin.MapFileNameData, g_Plugin.Image[0]->FileNameData);
		//		//	g_Plugin.MapFileName = (const wchar_t*)g_Plugin.Image[0]->FileName;					
		//			TODO("Флаги TERMINATED, FIRSTCALL, переменная g_Plugin.SelectionChanged");
		//		//}
		//		
		//		int rc = 0;
		//		
		//		g_Plugin.SaveTitle();

		//		TRY {
		//			rc = OpenImagePlugin(g_Plugin.MapFileName);
		//		} CATCH {
		//			CFunctionLogger::FunctionLogger(L"ProcessViewerEventW.OpenImagePlugin_raised an exception");
		//			g_StartupInfo.Message(PluginNumberMsg, FMSG_ALLINONE|FMSG_MB_OK,
		//				NULL,
		//				(const wchar_t * const *)GetMsg(MIOpenImgPlugException),
		//				0, 0);
		//			rc = FE_EXCEPTION;
		//		}

		//		g_Plugin.RestoreTitle();
		//	}
		//}
	}
	return 0;
}

void WINAPI GetPluginInfoW(struct PluginInfo *Info)
{
#ifdef FAR_UNICODE
	_ASSERTE(Info->StructSize >= sizeof(*Info));
#else
	Info->StructSize = sizeof(*Info);
#endif
	Info->Flags = PF_VIEWER | PF_EDITOR;

	g_Plugin.InitHooks();

	Info->CommandPrefix = g_Plugin.sHookPrefix; //L"pic"; DEFAULT_PREFIX

	static const wchar_t * PluginNameStrings[1];
	PluginNameStrings[0] = GetMsg(MIPluginName);
#ifdef FAR_UNICODE
	Info->PluginMenu.Guids = &guid_PlugMenu;
	Info->PluginMenu.Strings = PluginNameStrings;
	Info->PluginMenu.Count = 1;
	
	Info->PluginConfig.Guids = &guid_ConfMenu;
	Info->PluginConfig.Strings = PluginNameStrings;
	Info->PluginConfig.Count = 1;
#else	
	Info->PluginMenuStrings = Info->PluginConfigStrings = PluginNameStrings;
	Info->PluginMenuStringsNumber = Info->PluginConfigStringsNumber = sizeofarray(PluginNameStrings);
	Info->Reserved = 0x32436950; // PiC2
#endif
}

#ifdef FAR_UNICODE
void WINAPI GetGlobalInfoW(struct GlobalInfo *Info)
{
	memset(Info, 0, sizeof(GlobalInfo));
	Info->StructSize = sizeof(GlobalInfo);
	
	Info->MinFarVersion = /*FARMANAGERVERSION*/
		MAKEFARVERSION(
			FARMANAGERVERSION_MAJOR,
			FARMANAGERVERSION_MINOR,
			FARMANAGERVERSION_REVISION,
			FARMANAGERVERSION_BUILD,
			FARMANAGERVERSION_STAGE);
	
	// Build: YYMMDDX (YY - две цифры года, MM - месяц, DD - день, X - 0 и выше-номер подсборки)
	Info->Version = MAKEFARVERSION(MVV_1,MVV_2,MVV_3,MVV_4,VS_ALPHA);
	
	Info->Guid = guid_PicView;
	Info->Title = L"Picture View 3";
	Info->Description = L"Picture View 3";
	Info->Author = L"ConEmu.Maximus5@gmail.com";
}
#endif

#ifdef FAR_UNICODE
int WINAPI ProcessConsoleInputW(struct ProcessConsoleInputInfo *Info)
{
	TODO("Обработка/перехват консольного ввода");
	return FALSE; // отдать фару
}
#endif

int WINAPI GetMinFarVersionW(void)
{
	int nVer =
	#ifdef FAR_UNICODE
		#define MAKEFARVERSION2(major,minor,build) ( ((major)<<8) | (minor) | ((build)<<16))
		// Чтобы в Far2 не загружался
		MAKEFARVERSION2(FARMANAGERVERSION_MAJOR,FARMANAGERVERSION_MINOR,FARMANAGERVERSION_BUILD)
	#else
		FARMANAGERVERSION
	#endif
	;
	return nVer;
}

void WINAPI SetStartupInfoW(const struct PluginStartupInfo *xInfo)
{
	g_StartupInfo = *xInfo;
	g_FSF = *xInfo->FSF;
	g_StartupInfo.FSF = &g_FSF;

	#ifndef FAR_UNICODE
	nFarVersion = 0;
	g_StartupInfo.AdvControl(PluginNumber, ACTL_GETFARVERSION, &nFarVersion);

	if (!nFarVersion || ((HIWORD(nFarVersion) > 1144) && (HIWORD(nFarVersion) < 1148)))
	{
		g_StartupInfo.Message(PluginNumberMsg, FMSG_ALLINONE|FMSG_MB_OK,
			NULL,
			(const wchar_t * const *)GetMsg(MIFarBuildError), // L"PictureView2\nThis version of FAR manager is not supported\nPlease update FAR",
			0, 0);
		return;
	}
	#endif

	// Если имя файла плагина НЕ начинается с '0' - листание может перехватить другой
	// плагин, что вызовет кучу проблем.
	const wchar_t *pszFileName = wcsrchr(xInfo->ModuleName, L'\\');
	if (pszFileName) pszFileName++; else pszFileName = xInfo->ModuleName;
	if (*pszFileName != L'0') {
		g_StartupInfo.Message(PluginNumberMsg, FMSG_ALLINONE|FMSG_MB_OK,
			NULL,
			(const wchar_t * const *)GetMsg(MIPluginNameError), 
			0, 0);
	}
}

void WINAPI ExitFARW(
	#ifdef FAR_UNICODE
		void*
	#endif
	)
{
	MCHKHEAP;

	g_Panel.OnTerminate();
	g_Manager.OnTerminate();
	CImage::OnTerminate();

	//for (UINT i=0; i<sizeofarray(g_Plugin.Image); i++) {
	//	if (g_Plugin.Image[i]) {
	//		delete g_Plugin.Image[i];
	//		g_Plugin.Image[i] = NULL;
	//	}
	//}
	CloseHandle(g_Plugin.hDisplayEvent); g_Plugin.hDisplayEvent = NULL;
	CloseHandle(g_Plugin.hDisplayReady); g_Plugin.hDisplayReady = NULL;
	CloseHandle(g_Plugin.hWorkEvent);    g_Plugin.hWorkEvent = NULL;
	CloseHandle(g_Plugin.hSynchroDone);  g_Plugin.hSynchroDone = NULL;

	MCHKHEAP;

	CPVDManager::UnloadPlugins2();

	if (gp_RefKeeper)
	{
		delete gp_RefKeeper;
		gp_RefKeeper = NULL;
	}

	MCHKHEAP;

	if (g_RootKey) { free(g_RootKey); g_RootKey = NULL; }
	if (g_SelfPath) { free(g_SelfPath); g_SelfPath = NULL; }
	
	MCHKHEAP;

	UnregisterClass(g_WndClassName, g_hInstance);
}



BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD fdwReason, LPVOID lpvReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH) {
		//#ifdef _DEBUG
		//		MessageBox(NULL, L"PicView loaded", L"0pictureview.dll", MB_OK|MB_SYSTEMMODAL);
		//#endif
		g_hInstance = hInstance;
		gnMainThreadId = GetMainThreadId();

		#ifdef MHEAP_DEFINED
		xf_initialize();
		#endif
	} else if (fdwReason == DLL_PROCESS_DETACH) {
		#ifdef MHEAP_DEFINED
		//xf_done(); -- пока не избавлюсь от at_exit (глобальные статические классы)
		#endif
	}
	return TRUE;
}
