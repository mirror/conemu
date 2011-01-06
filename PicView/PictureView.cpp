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

#include "PictureView.h"
#include "PVDManager.h"
#include "headers/farkeys.hpp"
#include "PictureView_Lang.h"
//
#include "ConEmuSupport.h"
#ifdef _DEBUG
	#include <shlwapi.h>
#endif

//#define deletex(pointer) {delete pointer; pointer = NULL;}

HINSTANCE g_hInstance;

PluginStartupInfo g_StartupInfo;
FarStandardFunctions g_FSF;
DWORD nFarVersion = 0;
DWORD gnMainThreadId = 0, gnDisplayThreadId = 0, gnDecoderThreadId = 0;

PluginData g_Plugin;

wchar_t *g_SelfPath = NULL; // Содержит полный путь к папке, в которой лежит 0PictureView.dll (на конце - слэш)
wchar_t *g_RootKey = NULL;  // Содержит полный путь к ключу реестра. Обычно это "Software\\Far2\\Plugins\\PictureView"

// Если имя не содержит полный путь - сначала сделать FCTL_GETPANELDIR
// и слепить из него и pFileName полный путь к файлу!
//bool PutFilePathName(wchar_t* pFullName, const wchar_t* pFileName, bool abNeedUnquote=false)
//extern bool PutFilePathName1154(UnicodeFileName* pFullName, const wchar_t* pFileName, void* fsf);
//extern bool PutFilePathName1017(UnicodeFileName* aFullName, const wchar_t* pFileName);
bool PutFilePathName1154(UnicodeFileName* pFullName, const wchar_t* pFileName, void* fsf)
{
	if (!pFileName || !*pFileName || !pFullName) {
		_ASSERTE(pFileName && *pFileName);
		return false;
	}

	bool result = false;
	_ASSERTE(pFileName[0] != L'"');

	int nLen = (int)((FarStandardFunctions*)fsf)->ConvertPath(CPM_FULL, pFileName, NULL, 0);
	if (nLen > 0) {
		((FarStandardFunctions*)fsf)->ConvertPath(CPM_FULL, pFileName, pFullName->GetBuffer(nLen), nLen);
		result = pFullName->ReleaseBuffer();
	}

	return result;
}

bool PutFilePathName(UnicodeFileName* pFullName, const wchar_t* pFileName)
{
	// Ахтунг: использовать GetFullPathNameW для преобразования относительного пути в абсолютный после 1145 билда нельзя, используйте ConvertPath(CPM_FULL, ...).

	*pFullName = L""; // сразу сбросим, чтобы невалидные имена вообще не обрабатывать
	if (!pFileName || !*pFileName) {
		_ASSERTE(pFileName && *pFileName);
		return false;
	}

	bool result = false;
	int nLen = 0;
	wchar_t* pszUnquote = NULL;
	if (pFileName[0] == L'"') {
		pFileName ++;
		const wchar_t* pszEndQ = wcschr(pFileName, L'"');
		if (!pszEndQ) pszEndQ = pFileName + lstrlen(pFileName);
		nLen = (int)(pszEndQ - pFileName);
		size_t nSize = (nLen + 1)*sizeof(wchar_t);
		pszUnquote = (wchar_t*)malloc(nSize);
		memmove(pszUnquote, pFileName, nSize-2);
		pszUnquote[nLen] = 0;
		pFileName = pszUnquote;
	}
	
	//if ((HIWORD(nFarVersion)) <= 1144) {
	//	result = PutFilePathName1017(pFullName, pFileName);
	//} else if ((HIWORD(nFarVersion)) >= 1148) {
	result = PutFilePathName1154(pFullName, pFileName, &g_FSF);
	//} else {
	//	result = false;
	//}
	
	if (pszUnquote) free(pszUnquote);
	return result;
	

	//pFullName[0] = 0; // сразу сбросим, чтобы невалидные имена вообще не обрабатывать
	//
	//if (!pFileName || !*pFileName) {
	//	_ASSERTE(pFileName && *pFileName);
	//	return false;
	//}
	//
	//bool lbStartQ = false, lbEndQ = false;
	//if (pFileName[0] == L'"') {
	//	pFileName++; lbStartQ = true;
	//}
	//size_t nLen = lstrlenW(pFileName);
	//if (nLen && lbStartQ && pFileName[nLen-1] == L'"') {
	//	lbEndQ = true;
	//}
	//
	//// \\?\... или \\server\...
	//if (pFileName[0]==L'\\' && pFileName[1]==L'\\' /*&& pFileName[2]==L'?' && pFileName[3]==L'\\'*/) {
	//	if (nLen >= MAX_PIC_PATH_LEN)
	//		return false;
	//	lstrcpynW(pFullName, pFileName, MAX_PIC_PATH_LEN);
	//} else
	//// X:\... (а вот если просто X:name - нужно мудрить)
	//if (pFileName[1]==L':' && pFileName[2]==L'\\') {
	//	if (nLen >= MAX_PIC_PATH_LEN)
	//		return false;
	//	lstrcpynW(pFullName, pFileName, MAX_PIC_PATH_LEN);
	//} else {
	//	wchar_t* pCurDir = NULL;
	//	if (size_t len = g_StartupInfo.Control(INVALID_HANDLE_VALUE, FCTL_GETPANELDIR, 0, 0))
	//		if (pCurDir = (wchar_t*)malloc(len*2+2))
	//			if (!g_StartupInfo.Control(INVALID_HANDLE_VALUE, FCTL_GETPANELDIR, len, (LONG_PTR)pCurDir))
	//				*pCurDir = 0;
	//			else if (pCurDir[len-1] != L'\\') { // завершающий слеш нужен
	//				pCurDir[len-1] = L'\\';
	//				pCurDir[len] = 0;
	//			}
	//
	//	_ASSERTE(pCurDir);
	//
	//	if (pFileName[1] == L':') {
	//		wchar_t* pDrive = wcschr(pCurDir, L':');
	//		if (pDrive) {
	//			pDrive--;
	//			if ((pFileName[0] & ~0x20) == (pDrive[0] & ~0x20)) {
	//				// Буква диска (ucase) совпадает
	//				int n = (int)(pDrive-pCurDir+2);
	//				lstrcpynW(pFullName, pCurDir, n);
	//				lstrcpynW(pFullName+n, pFileName+2, MAX_PIC_PATH_LEN-n);
	//			} else {
	//				_ASSERTE((pFileName[0] & ~0x20) == (pDrive[0] & ~0x20));
	//				lstrcpynW(pFullName, pFileName, MAX_PIC_PATH_LEN);
	//			}
	//		} else {
	//			_ASSERTE(pDrive);
	//			lstrcpynW(pFullName, pFileName, MAX_PIC_PATH_LEN);
	//		}
	//	} else {
	//		size_t n = lstrlenW(pCurDir);
	//		if (n>=MAX_PIC_PATH_LEN) {
	//			_ASSERTE(n<MAX_PIC_PATH_LEN);
	//			return false;
	//		}
	//		lstrcpynW(pFullName, pCurDir, MAX_PIC_PATH_LEN);
	//		lstrcpynW(pFullName+n, pFileName, MAX_PIC_PATH_LEN-n);
	//	}
	//	if (pCurDir) free(pCurDir);
	//}
	//
	//if (lbEndQ) {
	//	nLen = lstrlenW(pFullName);
	//	if (nLen && pFullName[nLen-1] == L'"')
	//		pFullName[nLen-1] = 0;
	//}
	//
	//if (abNeedUnquote) TODO("А надо? уже вроде все сделали")
	//	g_FSF.Unquote(pFullName);
	//
	//TODO("GetFullPathName");
	//
	//return true;
}

#ifdef _DEBUG
uint MulDivU32(long a, long b, long c) 
{
	return (uint)((unsigned long long)(a)*(b)/(c));
}
uint MulDivU32R(long a, long b, long c){
	return (uint)(((unsigned long long)(a)*(b) + (c)/2)/(c));
}
int  MulDivIU32R(long a, long b, long c){
	return (int)(((long long)(a)*(b) + (c)/2)/(c));
}
#endif

void TestJumpDisable(const wchar_t *pFileName)
{
	const wchar_t *pPanelFileName = NULL;
	PanelInfo pi;
	PluginPanelItem *ppi = NULL;
	wchar_t *pCurDir = NULL;
	if (g_StartupInfo.Control(INVALID_HANDLE_VALUE, FCTL_GETPANELINFO, 0, (LONG_PTR)&pi))
		if (pi.ItemsNumber > 0)
			if (size_t len = g_StartupInfo.Control(INVALID_HANDLE_VALUE, FCTL_GETPANELITEM, pi.CurrentItem, 0))
				if (ppi = (PluginPanelItem*)malloc(len))
					if (g_StartupInfo.Control(INVALID_HANDLE_VALUE, FCTL_GETPANELITEM, pi.CurrentItem, (LONG_PTR)ppi))
						pPanelFileName = ppi->FindData.lpwszFileName;

	g_Plugin.Image[0]->bSelected = ppi ? ((ppi->Flags & PPIF_SELECTED)!=0) : false;

	TODO("А флаг FW_JUMP_DISABLED сбрасывать предварительно не нужно?");

	if (!*pFileName && pPanelFileName) {
		//lstrcpy(g_Plugin.Image[0]->FileName, pPanelFileName);
		if (!wcsncmp(pPanelFileName,L"\\\\?\\",4)) pPanelFileName+=4;
		if (!PutFilePathName(&g_Plugin.Image[0]->FileName, pPanelFileName))
			g_Plugin.FlagsWork |= FW_JUMP_DISABLED;
	}
	else
	{
		//lstrcpynW(g_Plugin.Image[0]->FileName, pFileName, sizeof(g_Plugin.Image[0]->FileName));
		//UnicodeFileName::SkipPrefix(&pPanelFileName);

		if (!PutFilePathName(&g_Plugin.Image[0]->FileName, pFileName))
			g_Plugin.FlagsWork |= FW_JUMP_DISABLED;
		//g_FSF.Unquote(g_Plugin.Image[0]->FileName);
		else if (!pPanelFileName) // viewer/editor?
			g_Plugin.FlagsWork |= FW_JUMP_DISABLED;
		else
		{
			const wchar_t* pszImgFileName = g_Plugin.Image[0]->FileName;
			UnicodeFileName::SkipPrefix(&pPanelFileName);
			UnicodeFileName::SkipPrefix(&pszImgFileName);

			if (g_FSF.LStricmp(pPanelFileName, pszImgFileName))
			// Если имя под курсором совпадает с именем переданным в плагин ("pic:!.!" ?)
			{
				const wchar_t *const p = g_FSF.PointToName(g_Plugin.Image[0]->FileName);
				if (g_FSF.LStricmp(pPanelFileName, p))
					g_Plugin.FlagsWork |= FW_JUMP_DISABLED;
				else
					if (pi.Flags & PFLAGS_REALNAMES)
					{
						//const int lPath = p - g_Plugin.Image[0]->FileNameData - 1;
						if (size_t len = g_StartupInfo.Control(INVALID_HANDLE_VALUE, FCTL_GETPANELDIR, 0, 0))
							if (pCurDir = (wchar_t*)malloc(len*2))
								if (!g_StartupInfo.Control(INVALID_HANDLE_VALUE, FCTL_GETPANELDIR, len, (LONG_PTR)pCurDir))
									*pCurDir = 0;
						if (!pCurDir || !g_Plugin.Image[0]->FileName.CompareDir(pCurDir))
						{
							g_Plugin.FlagsWork |= FW_JUMP_DISABLED;
						}
					}
			}
		}
	}
	if (pCurDir) free(pCurDir);
	if (ppi) free(ppi);
}

const wchar_t* GetExtension(const wchar_t* pFileName)
{
	const wchar_t *p = NULL;
	const wchar_t *pS = wcsrchr(pFileName, '\\');
	if (!pS)
		pS = pFileName;
	if (((p = wcsrchr(pFileName, '.')) != NULL) && (p >= pS))
		p++;
	else
		p = L".";
	if (!p || !*p)
		p = L".";
	return p;
}

bool ExtensionMatch(LPTSTR asExtList, LPCTSTR asExt)
{
	wchar_t szExtTmp[32];
	// 2010-10-01 - отрезать хвостовые пробелы
	if (asExt && *asExt)
	{
		int nLen = lstrlen(asExt); if (nLen > 32) nLen = 31;
		if (asExt[nLen-1] == L' ')
		{
			while (asExt[nLen-1] == L' ')
				nLen--;
			if (nLen)
			{
				lstrcpyn(szExtTmp, asExt, nLen+1);
				szExtTmp[nLen] = 0;
				asExt = szExtTmp;
			}
			else
			{
				asExt = L".";
			}
		}
	}

	if (!asExtList || !asExt) return false;
	if (!*asExtList || !*asExt) return false;

	while (*asExtList)
	{
		wchar_t* pszNext = wcschr(asExtList, L',');
		if (pszNext) *pszNext = 0;
		bool bEqual = lstrcmpi(asExtList, asExt) == 0;
		if (pszNext) *pszNext = L',';

		if (bEqual) {
			return true;
		} else if (!pszNext) {
			break;
		} else {
			asExtList = pszNext + 1;
		}
	}
	return false;
}


bool SwitchImages(bool bForward)
{
	bool result = false;
	if (bForward)
	{
		if (g_Plugin.Image[2]->pDraw)
		{
			Swap(g_Plugin.Image[0], g_Plugin.Image[1]);
			Swap(g_Plugin.Image[0], g_Plugin.Image[2]);

			g_Plugin.Image[2]->DisplayClose();
			if (g_Plugin.Image[2]->Decoder)
			{
				g_Plugin.Image[2]->Decoder->Close();
				//deletex(g_Plugin.Image[2]->Decoder);
			}
			result = true;
		}
		g_Plugin.Image[2]->SetPanelItemRaw(g_Plugin.Image[0]->PanelItemRaw());
		g_Plugin.ImageX = g_Plugin.Image[2];
		//g_Plugin.ddsx = g_Plugin.Image[2]->dds;
	}
	else
	{
		if (g_Plugin.Image[1]->pDraw)
		{
			Swap(g_Plugin.Image[0], g_Plugin.Image[2]);
			Swap(g_Plugin.Image[0], g_Plugin.Image[1]);

			g_Plugin.Image[1]->DisplayClose();
			if (g_Plugin.Image[1]->Decoder)
			{
				g_Plugin.Image[1]->Decoder->Close();
				//deletex(g_Plugin.Image[1]->Decoder);
			}
			result = true;
		}
		g_Plugin.Image[1]->SetPanelItemRaw(g_Plugin.Image[0]->PanelItemRaw());
		g_Plugin.ImageX = g_Plugin.Image[1];
		//g_Plugin.ddsx = g_Plugin.Image[1]->dds;
	}

	OutputDebugString(L"Invalidating in SwitchImages\n");
	InvalidateRect(g_Plugin.hWnd, NULL, FALSE);
	return result;
}

bool CreateDisplayThread()
{
	DWORD nTID = GetCurrentThreadId();
	
	if (g_Plugin.hThread) {
		_ASSERTE(g_Plugin.hThread == NULL);
		if (WaitForSingleObject(g_Plugin.hThread,0) != WAIT_TIMEOUT) {
			// Нить завешилась?
			_ASSERTE(nTID == gnMainThreadId);
			CloseHandle(g_Plugin.hThread);
			g_Plugin.hThread = NULL;
		} else {
			return true;
		}
	}

	bool result = false;
	
	g_Plugin.FlagsDisplay = 0;
	ResetEvent(g_Plugin.hDisplayEvent);
	ResetEvent(g_Plugin.hWorkEvent);

	g_Plugin.hThread = CreateThread(NULL, 0, DisplayThreadProc, NULL, 0, &gnDisplayThreadId);
	if (result = (g_Plugin.hThread != NULL))
	{
		SetThreadPriority(g_Plugin.hThread, THREAD_PRIORITY_ABOVE_NORMAL);
		//Wait ForSingleObject(g_Plugin.hDisplayEvent, INFINITE);
		
		// Дождаться окончания создания окна отрисовки или завершения нити
		WaitDisplayEvent();
		
		result = (g_Plugin.hWnd != NULL);
	}
		
	return result;
}

bool WaitDisplayEvent()
{
	DWORD dwWait = 0;
	HANDLE hEvents[2] = {g_Plugin.hDisplayEvent, g_Plugin.hThread};
	//Wait ForSingleObject(g_Plugin.hDisplayEvent, INFINITE); -- низя. Если нить грохнулась - мы здесь повиснем

	WARNING("Бесконечное ожидание заменить на цикл, в котором опрашивать консоль. А из Display - убрать ReadConsoleInput");
	// Это нужно для того, чтобы FAR не вис при переключении в другие приложения во время отрисовки диалогов и хелпа
	// НО нужно учесть, что если работает хелп/диалог то консоль читает сам фар?

	while ((dwWait = WaitForMultipleObjects(2, hEvents, FALSE, 0)) == WAIT_TIMEOUT)
	{
		//// Может быть потом тут что-то типа PeekConsole... вставить, во избежание повисания conhost
		//if (g_Plugin.hWnd && !IsWindowVisible(g_Plugin.hWnd)) {
		//	INPUT_RECORD ir;
		//	u32 t;
		//	PeekConsoleInput(GetStdHandle(STD_INPUT_HANDLE)/*g_Plugin.hInput*/, &ir, 1, &t);
		//}
		if (g_Plugin.FlagsWork & FW_SHOW_HELP) {
			// Показать Help в основной нити
			CFunctionLogger flog(L"FW_SHOW_HELP");
			wchar_t Title[0x40];
			lstrcpynW(Title, g_Plugin.pszPluginTitle, sizeofarray(Title) - 6);
			lstrcat(Title, L" - Far");
			// Раньше выполнялось в нити Display, и часто висла
			SetConsoleTitleW(Title);

			g_StartupInfo.ShowHelp(g_StartupInfo.ModuleName, _T("Controls"), 0);
			
			SetEvent(g_Plugin.hSynchroDone);
			g_Plugin.FlagsWork &= ~FW_SHOW_HELP;
			
		} else if (g_Plugin.FlagsWork & (FW_SHOW_CONFIG | FW_SHOW_MODULES)) {
			// Показать окно конфига в основной нити
			CFunctionLogger flog(L"FW_SHOW_CONFIG|FW_SHOW_MODULES");
			
			//TRUE, если настройки были изменены
			const bool reconfig = ConfigureW(0);
			
			if (reconfig)
				g_Plugin.FlagsDisplay |= FD_REQ_REFRESH;
			SetEvent(g_Plugin.hSynchroDone);
			g_Plugin.FlagsWork &= ~(FW_SHOW_CONFIG|FW_SHOW_MODULES);

			//{
			//	//TODO("А в [0]-m DisplayClose не нужен?");
			//	//// Закрыть все хэндлы вывода кроме текущего
			//	//g_Plugin.Image[1]->DisplayClose();
			//	//g_Plugin.Image[2]->DisplayClose();
			//	//for (uint i = 3; --i;)
			//	//	if (g_Plugin.Image[i]->Decoder)
			//	//	{
			//	//		g_Plugin.Image[i]->DisplayClose();
			//	//		g_Plugin.Image[i]->Decoder->Close();
			//	//		//deletex(g_Plugin.Image[i]->Decoder);
			//	//	}
			//	//g_Plugin.Image[1]->iPanelItemRaw = g_Plugin.Image[2]->iPanelItemRaw = g_Plugin.Image[0]->iPanelItemRaw;
			//	//g_Plugin.ImageX = g_Plugin.Image[2];
			//	////g_Plugin.ddsx = g_Plugin.Image[2]->dds;
			//	
			//	
			//	//dwWait = WAIT_OBJECT_0;
			//	break;
			//}
		} else if (g_Plugin.FlagsWork & (FW_MARK_FILE | FW_UNMARK_FILE)) {
			CFunctionLogger flog(L"FW_MARK_FILE | FW_UNMARK_FILE");

			g_Plugin.Image[0]->bSelected = (g_Plugin.FlagsWork & FW_MARK_FILE) == FW_MARK_FILE;
			g_Plugin.SelectionChanged = true; // чтобы при выходе из плагина можно было обновить панель, показав выделенные вверху
			g_StartupInfo.Control(INVALID_HANDLE_VALUE, FCTL_SETSELECTION, g_Plugin.Image[0]->PanelItemRaw(), g_Plugin.Image[0]->bSelected);
			g_StartupInfo.Control(INVALID_HANDLE_VALUE, FCTL_REDRAWPANEL, 0, 0);
			TitleRepaint();
			SetEvent(g_Plugin.hSynchroDone);
			g_Plugin.FlagsWork &= ~(FW_MARK_FILE | FW_UNMARK_FILE);

		} else if (g_Plugin.FlagsWork & (FW_TITLEREPAINT | FW_TITLEREPAINTD)) {
			CFunctionLogger flog(L"FW_TITLEREPAINT | FW_TITLEREPAINTD");

			TitleRepaint((g_Plugin.FlagsWork & FW_TITLEREPAINTD)==FW_TITLEREPAINTD);
			SetEvent(g_Plugin.hSynchroDone);
			g_Plugin.FlagsWork &= ~(FW_TITLEREPAINT | FW_TITLEREPAINTD);

		} else if (g_Plugin.FlagsWork & (FW_QVIEWREPAINT)) {
			CFunctionLogger flog(L"FW_QVIEWREPAINT");

			QViewRepaint();
			SetEvent(g_Plugin.hSynchroDone);
			g_Plugin.FlagsWork &= ~FW_QVIEWREPAINT;

		}

		if (!ProcessConsoleInputs())
			Sleep(10);
		if (g_Plugin.FlagsWork & FW_TERMINATE) {
			while ((dwWait = WaitForMultipleObjects(2, hEvents, FALSE, 10)) == WAIT_TIMEOUT)
				;
			break;
		}
	}

	CFunctionLogger::FunctionLogger(L"WorkDisplayEvent done");
	//_ASSERTE(dwWait!=(WAIT_OBJECT_0+1));
	return (dwWait==WAIT_OBJECT_0);
}

void WorkWait(void)
{
	const bool bCaching = !(g_Plugin.FlagsWork & FW_JUMP_DISABLED) && (g_Plugin.FarPanelInfo.Flags & PFLAGS_REALNAMES ? g_Plugin.bCachingRP : g_Plugin.bCachingVP);
	g_Plugin.ImageX = NULL;
	//g_Plugin.ddsx = NULL;

	if (g_Plugin.bUncachedJump)
	{
		g_Plugin.bUncachedJump = false;
		if (SwitchImages(g_Plugin.FlagsDisplay & FD_JUMP_NEXT))
		{
			if (g_Plugin.FlagsDisplay & FD_HOME_END) // invalidate incorrect cache
				if (g_Plugin.FlagsDisplay & FD_JUMP_NEXT)
				{
					g_Plugin.Image[1]->DisplayClose();
					if (g_Plugin.Image[1]->Decoder)
					{
						g_Plugin.Image[1]->Decoder->Close();
						//deletex(g_Plugin.Image[1]->Decoder);
					}
				}
				else
				{
					g_Plugin.Image[2]->DisplayClose();
					if (g_Plugin.Image[2]->Decoder)
					{
						g_Plugin.Image[2]->Decoder->Close();
						//deletex(g_Plugin.Image[2]->Decoder);
					}
				}
			SetEvent(g_Plugin.hWorkEvent);
			//Wait ForSingleObject(g_Plugin.hDisplayEvent, INFINITE);
			if (!WaitDisplayEvent())
				return;
			if (bCaching)
				return;
		}
		else
		{
			g_Plugin.FlagsDisplay &= ~(FD_JUMP | FD_HOME_END);
			SetEvent(g_Plugin.hWorkEvent);
		}
	}

	for (;;)
	{
		// В некоторых случаях FAR пытается восстановить консольный курсор.
		// Если он видим - гасим и Invalidate
		AutoHideConsoleCursor();

		//Wait ForSingleObject(g_Plugin.hDisplayEvent, INFINITE);
		if (!WaitDisplayEvent())
			return;

		if (g_Plugin.FlagsDisplay & FD_REFRESH)
			return;

		if (g_Plugin.FlagsWork & FW_SHOW_CONFIG)
		{
			_ASSERT(FALSE); // Сюда мы больше попадать не должны?
			_ASSERTE(g_Plugin.FlagsWork & FW_SHOW_CONFIG);
			
			//TRUE, если настройки были изменены
			const bool reconfig = ConfigureW(0);
			
			SetEvent(g_Plugin.hWorkEvent);
			//Wait ForSingleObject(g_Plugin.hDisplayEvent, INFINITE);
			if (!WaitDisplayEvent())
				return;

			if (!reconfig)
				continue;

			TODO("А в [0]-m DisplayClose не нужен?");
			g_Plugin.Image[1]->DisplayClose();
			g_Plugin.Image[2]->DisplayClose();
			for (uint i = 3; --i;)
				if (g_Plugin.Image[i]->Decoder)
				{
					g_Plugin.Image[i]->Decoder->Close();
					//deletex(g_Plugin.Image[i]->Decoder);
				}
			g_Plugin.Image[1]->SetPanelItemRaw(g_Plugin.Image[0]->PanelItemRaw());
			g_Plugin.Image[2]->SetPanelItemRaw(g_Plugin.Image[0]->PanelItemRaw());
			g_Plugin.ImageX = g_Plugin.Image[2];
			//g_Plugin.ddsx = g_Plugin.Image[2]->dds;
		}
		else if (g_Plugin.FlagsDisplay & FD_HOME_END)
		{
			if (g_Plugin.Image[0]->PanelItemRaw() == (g_Plugin.FlagsDisplay & FD_JUMP_NEXT ? 0 : g_Plugin.nPanelItems - 1))
			{
				g_Plugin.FlagsDisplay &= ~FD_HOME_END;
				SetEvent(g_Plugin.hWorkEvent);
				continue;
			}
			_ASSERTE((g_Plugin.FlagsDisplay & FD_REFRESH) == 0);
			if (g_Plugin.FlagsDisplay & FD_JUMP_NEXT)
			{
				g_Plugin.Image[2]->DisplayClose();
				if (g_Plugin.Image[2]->Decoder)
				{
					g_Plugin.Image[2]->Decoder->Close();
					//deletex(g_Plugin.Image[2]->Decoder);
				}
				// Ниже по коду (OpenImagePlugin) индекс будет (++) так что тут будет 0
				g_Plugin.Image[2]->SetPanelItemRaw(-1);
				g_Plugin.ImageX = g_Plugin.Image[2];
				//g_Plugin.ddsx = g_Plugin.Image[2]->dds;
			}
			else
			{
				g_Plugin.Image[1]->DisplayClose();
				if (g_Plugin.Image[1]->Decoder)
				{
					g_Plugin.Image[1]->Decoder->Close();
					//deletex(g_Plugin.Image[1]->Decoder);
				}
				// Ниже по коду (OpenImagePlugin) индекс будет (--) так что тут будет (ItemsNumber-1)
				g_Plugin.Image[1]->SetPanelItemRaw(g_Plugin.FarPanelInfo.ItemsNumber);
				g_Plugin.ImageX = g_Plugin.Image[1];
				//g_Plugin.ddsx = g_Plugin.Image[1]->dds;
			}
			g_Plugin.bUncachedJump = true;
		}
		else if (g_Plugin.FlagsDisplay & FD_JUMP)
		{
			int nIndex = g_Plugin.FlagsDisplay & FD_JUMP_NEXT ? 2 : 1;
			if (g_Plugin.Image[nIndex]->PanelItemRaw() == PANEL_ITEM_UNAVAILABLE)
			{
				g_Plugin.FlagsDisplay &= ~FD_JUMP;
				SetEvent(g_Plugin.hWorkEvent);
				continue;
			}
			else
				if (SwitchImages(g_Plugin.FlagsDisplay & FD_JUMP_NEXT))
				{
					SetEvent(g_Plugin.hWorkEvent);
					//Wait ForSingleObject(g_Plugin.hDisplayEvent, INFINITE);
					if (!WaitDisplayEvent())
						return;
					if (!bCaching)
						continue;
				}
				else
					g_Plugin.bUncachedJump = true;
		}
		else
		{
			g_Plugin.ImageX = g_Plugin.Image[0];
			//g_Plugin.ddsx = g_Plugin.Image[0]->dds;
		}
		break;
	}
}

void ExitViewerEditor(void)
{
	if ((g_Plugin.FlagsWork & FW_VE_HOOK) && !(g_Plugin.FlagsWork & FW_QUICK_VIEW))
	{
		DWORD Command = KEY_ESC;
		KeySequence ks = {KSFLAGS_DISABLEOUTPUT, 1, &Command};
		g_StartupInfo.AdvControl(g_StartupInfo.ModuleNumber, ACTL_POSTKEYSEQUENCE, &ks);
	}

	g_Plugin.FlagsWork &= ~FW_VE_HOOK;
}

EFlagsResult OpenImagePlugin(const wchar_t *pFileName);

void CheckAllClosed()
{
	MCHKHEAP;
	for (uint i = 3; i--;)
		if (g_Plugin.Image[i]->Decoder)
		{
			_ASSERTE(g_Plugin.Image[i]->Decoder->mp_ImageContext == NULL);
		}
	MCHKHEAP;
}

HANDLE WINAPI OpenFilePluginW(const wchar_t *pFileName, const unsigned char *buf, int lBuf, int OpMode)
{
	if (!nFarVersion || ((HIWORD(nFarVersion) > 1144) && (HIWORD(nFarVersion) < 1148)))
		return INVALID_HANDLE_VALUE;
		
	if (!gnMainThreadId)
		gnMainThreadId = GetCurrentThreadId();
		
	g_Plugin.InitHooks();

	// ACTL_CONSOLEMODE фаром более не предоставляется, самому проверять не хочется

	TODO("pFileName всегда должно быть или нет?");
	if (!pFileName || !*pFileName
	    //|| g_StartupInfo.AdvControl(g_StartupInfo.ModuleNumber, ACTL_CONSOLEMODE, (void*)FAR_CONSOLE_GET_MODE) == FAR_CONSOLE_FULLSCREEN 
	    || (!g_Plugin.bHookArc && !g_Plugin.hThread && !(g_Plugin.FlagsWork & (FW_VE_HOOK | FW_PLUGIN_CALL))))
		return INVALID_HANDLE_VALUE;

	// Раз мы в FAR2 - вполне можно опредлить откуда пытается быть вызван вход в файл
	if ((OpMode & (OPM_SILENT|OPM_FIND)) != 0)
		return INVALID_HANDLE_VALUE;

	if (g_Plugin.IsExtensionIgnored(pFileName))
		return INVALID_HANDLE_VALUE;
		
	g_Plugin.InitPlugin();
	
	FUNCLOGGERS(L"OpenFilePluginW(%s)", pFileName);

	// Если это первый вызов серии
	if (!(g_Plugin.FlagsWork & FW_PLUGIN_CALL)) {
		g_Plugin.FlagsWork |= FW_FIRST_CALL; // ставим флаг (влияет на включение кнопки-модификатора)
		g_Plugin.SelectionChanged = false; // Чтобы знать, что менялось выделение
	}

	// Сброс точно ненужных
	g_Plugin.FlagsWork &= ~(FW_TERMINATE|FW_QUICK_VIEW);

	g_Plugin.SaveTitle();

	g_Plugin.MapFileName = pFileName;

	HANDLE hPlugin = INVALID_HANDLE_VALUE;
	EFlagsResult rc;
	
	TRY {
		rc = OpenImagePlugin(g_Plugin.MapFileName);
		CFunctionLogger::FunctionLogger(L"OpenFilePluginW.OpenImagePlugin done");
		if ((g_Plugin.FlagsWork & FW_PLUGIN_CALL) && g_Plugin.hWnd && IsWindow(g_Plugin.hWnd)) {
			TODO("Прыгать по плагиновым панелям получается плохо - при ошибках нить не завершается");
			if (rc == FE_UNKNOWN_EXTENSION) {
				g_Plugin.FlagsWork |= FW_TERMINATE;
				PostMessage(g_Plugin.hWnd, DMSG_KEYBOARD, VK_ESCAPE, 0);
				SetEvent(g_Plugin.hWorkEvent);
				SetEvent(g_Plugin.hSynchroDone);
				WARNING("!!! от INFINITE нужно избавляться !!!"); // Если нить не завершается - она может висеть и ее нужно насильно прибить
				CFunctionLogger::FunctionLogger(L"OpenFilePluginW.WaitForSingleObject(g_Plugin.hThread, INFINITE)");
				WaitForSingleObject(g_Plugin.hThread, INFINITE);
				CFunctionLogger::FunctionLogger(L"OpenFilePluginW.WaitForSingleObject(g_Plugin.hThread, INFINITE) done");
				CloseHandle(g_Plugin.hThread);
				g_Plugin.hThread = NULL;
				ResetEvent(g_Plugin.hWorkEvent);
				ResetEvent(g_Plugin.hDisplayEvent);
			}
		}
		switch (rc) {
			case FE_PROCEEDED:
				// Если (g_Plugin.FlagsWork & FW_PLUGIN_CALL) то в FAR мог быть послан Ctrl-PgDn
				CFunctionLogger::FunctionLogger(L"OpenFilePluginW.FE_PROCEEDED");
				hPlugin = (HANDLE)-2; // Чтобы FAR не пытался открыть файл другим плагином
				break;
			case FE_SKIP_ERROR:
				CFunctionLogger::FunctionLogger(L"OpenFilePluginW.FE_SKIP_ERROR");
				break; // Не открыли, но ошибку показывать не нужно
			case FE_UNKNOWN_EXTENSION:
				// Неизвестное расширение
				//if (g_Plugin.FlagsWork & FW_FIRST_CALL) {
				//	g_StartupInfo.Message(g_StartupInfo.ModuleNumber, FMSG_ALLINONE|FMSG_MB_OK,
				//		NULL, (const wchar_t * const *)GetMsg(MIOpenUnknownExtension), 0, 0);
				//}
				CFunctionLogger::FunctionLogger(L"OpenFilePluginW.FE_UNKNOWN_EXTENSION");
				break;
			case FE_OPEN_FAILED:
				// Ошибка открытия файла
				CFunctionLogger::FunctionLogger(L"OpenFilePluginW.FE_OPEN_FAILED");
				if (g_Plugin.FlagsWork & FW_FIRST_CALL) {
					g_StartupInfo.Message(g_StartupInfo.ModuleNumber, FMSG_ALLINONE|FMSG_MB_OK,
						NULL, (const wchar_t * const *)GetMsg(MIOpenFailedMsg), 0, 0);
				}
				break;
			case FE_NO_PANEL_ITEM:
				// Больше нет элементов на панели?
				CFunctionLogger::FunctionLogger(L"OpenFilePluginW.FE_NO_PANEL_ITEM");
				break;
		}

		if ((g_Plugin.FlagsDisplay & FD_JUMP) && !(g_Plugin.FlagsWork & FW_TERMINATE)) {
			hPlugin = (HANDLE)-2; // Чтобы FAR не пытался открыть файл другим плагином
		}

	} CATCH {
		CFunctionLogger::FunctionLogger(L"OpenFilePluginW.OpenImagePlugin raised an exception");
		g_StartupInfo.Message(g_StartupInfo.ModuleNumber, FMSG_ALLINONE|FMSG_MB_OK,
			NULL,
			(const wchar_t * const *)GetMsg(MIOpenImgPlugException),
			0, 0);
		rc = FE_EXCEPTION;
	}

	if ((g_Plugin.FlagsWork & FW_TERMINATE) && g_Plugin.FlagsWork != FW_TERMINATE) {
		_ASSERTE(g_Plugin.FlagsWork == FW_TERMINATE);
		g_Plugin.FlagsWork = FW_TERMINATE;
	}

	if (g_Plugin.FlagsWork & FW_TERMINATE)
	{
		g_Plugin.RestoreTitle();

		// GFL портит консоль при просмотре WMF
		g_StartupInfo.AdvControl(g_StartupInfo.ModuleNumber, ACTL_REDRAWALL, NULL);

		TODO("Имеет смысл, только если выбрана настройка 'Показывать выделенные файлы вверху'");
		if (g_Plugin.SelectionChanged) {
			ActlKeyMacro km = {MCMD_POSTMACROSTRING};
			km.Param.PlainText.SequenceText = L"CtrlR";
			g_StartupInfo.AdvControl(g_StartupInfo.ModuleNumber, ACTL_KEYMACRO, &km);
			g_Plugin.SelectionChanged = false;
		}

		CheckAllClosed();
	}
	
	return hPlugin;
}

bool EscapePressed()
{
	if (g_Plugin.FlagsWork & FW_TERMINATE)
		return true;

	// был GetAsyncKeyState
	SHORT stat = GetKeyState(VK_ESCAPE);
	if (stat & 0x8000) {
		g_Plugin.FlagsWork |= FW_TERMINATE;
		return true;
	}

	return false;
}

BOOL CheckConEmu()
{
	HWND hRoot = NULL;
	HWND hNewFarWnd = GetConEmuHwnd();

	if (g_Plugin.hConEmuCtrlPressed) { CloseHandle(g_Plugin.hConEmuCtrlPressed); g_Plugin.hConEmuCtrlPressed = NULL; }
	if (g_Plugin.hConEmuShiftPressed) { CloseHandle(g_Plugin.hConEmuShiftPressed); g_Plugin.hConEmuShiftPressed = NULL; }

	if (!g_Plugin.hParentWnd)
		g_Plugin.hParentWnd = g_Plugin.hDesktopWnd;

	//if (!g_Plugin.hFarWnd || !g_Plugin.hParentWnd 
	//	|| (hNewFarWnd && hNewFarWnd != g_Plugin.hFarWnd)
	//	|| (g_Plugin.hConEmuWnd && !IsWindow(g_Plugin.hConEmuWnd))
	//	|| (hNewFarWnd && hNewFarWnd != g_Plugin.hConEmuWnd)
	//	)
	//{
	//	g_Plugin.hParentWnd = g_Plugin.hDesktopWnd;
	if (hNewFarWnd) {
		//g_Plugin.hFarWnd = hNewFarWnd;
		hRoot = GetParent(hNewFarWnd);

		TCHAR szName[64]; DWORD dwPID = GetCurrentProcessId();
		wsprintf(szName, CEKEYEVENT_CTRL, dwPID);
		g_Plugin.hConEmuCtrlPressed = OpenEvent(SYNCHRONIZE, FALSE, szName);
		wsprintf(szName, CEKEYEVENT_SHIFT, dwPID);
		g_Plugin.hConEmuShiftPressed = OpenEvent(SYNCHRONIZE, FALSE, szName);

	} else {
		//g_Plugin.hConEmuWnd = NULL;
		hNewFarWnd = (HWND)g_StartupInfo.AdvControl(g_StartupInfo.ModuleNumber, ACTL_GETFARHWND, NULL);
		//const HWND hAncestor = GetAncestor(g_Plugin.hFarWnd, GA_PARENT);
		//if (hAncestor != g_Plugin.hDesktopWnd) -- пережитки старых conemu. убито
		//	g_Plugin.hFarWnd = hAncestor;
	}
//}

	g_Plugin.hFarWnd = hNewFarWnd;
	g_Plugin.hConEmuWnd = hRoot;

	return (g_Plugin.hConEmuWnd!=NULL);
}

EFlagsResult OpenImagePlugin(const wchar_t *pFileName)
{
	g_Plugin.InitHooks();

	// Уже должно быть выполнено! Да и при вызове через префикс этого лучше не делать
	//if (g_Plugin.IsExtensionIgnored(pFileName))
	//	return (HANDLE)-1;

	g_Plugin.InitPlugin();

	// Для первой картинки серии - установить флаг FW_FIRST_CALL, но не FW_FORCE_DECODE,
	// т.к. сюда мы можем попасть и при перехвате вьювера/редактора, а там FW_FORCE_DECODE не нужен
	// Первую картинку серии определяем по осутствию окна g_Plugin.hWnd и НЕ QView.
	if (g_Plugin.hWnd == NULL && !(g_Plugin.FlagsWork & FW_QUICK_VIEW)) {
		g_Plugin.FlagsWork |= FW_FIRST_CALL;
	}

	// Дополнительная инициализация при вызове для первой картинки серии
	// (т.к. InitPlugin вызывается только один раз за сеанс FAR-а)
	if (g_Plugin.FlagsWork & FW_FIRST_CALL) {
		CheckAllClosed();
		g_Plugin.ImageX = g_Plugin.Image[0];
	}

	MCHKHEAP;

	if ((g_Plugin.FlagsWork & FW_FORCE_DECODE) == 0) {
		if (!PVDManager::IsSupportedExtension(pFileName))
			return FE_UNKNOWN_EXTENSION;
	}

	g_Plugin.FlagsWork &= ~FW_TERMINATE;

	_ASSERTE(g_Plugin.hConEmuCtrlPressed==NULL && g_Plugin.hConEmuShiftPressed==NULL);

	CheckConEmu();
	//HWND hNewFarWnd = GetConEmuHwnd();
	//if (!g_Plugin.hFarWnd || !g_Plugin.hParentWnd 
	//	|| (hNewFarWnd && hNewFarWnd != g_Plugin.hFarWnd)
	//	|| (g_Plugin.hConEmuWnd && !IsWindow(g_Plugin.hConEmuWnd))
	//	|| (hNewFarWnd && !g_Plugin.hConEmuWnd)
	//	)
	//{
	//	g_Plugin.hParentWnd = g_Plugin.hDesktopWnd;
	//	if (hNewFarWnd) {
	//		g_Plugin.hFarWnd = hNewFarWnd;
	//		g_Plugin.hConEmuWnd = GetParent(g_Plugin.hFarWnd);
	//		TCHAR szName[64]; DWORD dwPID = GetCurrentProcessId();
	//		wsprintf(szName, CEKEYEVENT_CTRL, dwPID);
	//		g_Plugin.hConEmuCtrlPressed = OpenEvent(SYNCHRONIZE, FALSE, szName);
	//		wsprintf(szName, CEKEYEVENT_SHIFT, dwPID);
	//		g_Plugin.hConEmuShiftPressed = OpenEvent(SYNCHRONIZE, FALSE, szName);
	//	} else {
	//		g_Plugin.hConEmuWnd = NULL;
	//		g_Plugin.hFarWnd = (HWND)g_StartupInfo.AdvControl(g_StartupInfo.ModuleNumber, ACTL_GETFARHWND, NULL);
	//		//const HWND hAncestor = GetAncestor(g_Plugin.hFarWnd, GA_PARENT);
	//		//if (hAncestor != g_Plugin.hDesktopWnd) -- пережитки старых conemu. убито
	//		//	g_Plugin.hFarWnd = hAncestor;
	//	}
	//}

	TODO("Окно отрисовки в conemu может стать невидимым");
	bool lbVisible = false;
	if (!g_Plugin.hConEmuWnd && (!IsWindow(g_Plugin.hFarWnd) || !IsWindowVisible(g_Plugin.hFarWnd))) {
		g_StartupInfo.Message(g_StartupInfo.ModuleNumber, FMSG_ALLINONE|FMSG_MB_OK,
			NULL,
			(const wchar_t * const *)GetMsg(MIConEmuPluginWarning),
			0, 0);
		return FE_SKIP_ERROR;
	}

	g_Plugin.InitCMYK(FALSE); // Дождаться его завершения

	MCHKHEAP;
	//g_Plugin.FlagsWork |= FW_FIRSTIMAGE;
	
	TODO("???");
	if (g_Plugin.ImageX != g_Plugin.Image[0] 
		&& (g_Plugin.hParentWnd == g_Plugin.hFarWnd || g_Plugin.hParentWnd == g_Plugin.hConEmuWnd))
	{
		TitleRepaint();
		if (g_Plugin.hWnd)
			InvalidateRect(g_Plugin.hWnd, NULL, FALSE);
	}

	MCHKHEAP;

	_ASSERTE(g_Plugin.ImageX);
	if (g_Plugin.ImageX)
		g_Plugin.ImageX->Decoder->ResetProcessed();

	g_Plugin.ZoomAutoManual = false; //101129
	if (g_Plugin.FlagsWork & FW_FIRST_CALL) {
		g_Plugin.ZoomAuto = g_Plugin.bAutoZoom ? ZA_FIT : ZA_NONE;

		if (!g_Plugin.ZoomAuto) {
			g_Plugin.Zoom = g_Plugin.AbsoluteZoom = 0x10000; // initial 100%
		}
	} else if (!g_Plugin.bKeepZoomAndPosBetweenFiles) {
		g_Plugin.ZoomAuto = g_Plugin.bAutoZoom ? ZA_FIT : ZA_NONE;
		WARNING("Почему-то ветки различаются?  if (!g_Plugin.ZoomAuto) {...");
		g_Plugin.Zoom = g_Plugin.AbsoluteZoom = 0x10000; // initial 100%
	}

	//unsigned char tempBuf[128];
	PluginPanelItem *ppi = NULL;
	//uint iSubDecoder = 0;
lReOpen:
//lReOpen2:
	MCHKHEAP;
	if (!g_Plugin.MapFileName.IsEmpty() && pFileName != (const wchar_t*)g_Plugin.MapFileName) {
		pFileName = (const wchar_t*)g_Plugin.MapFileName;
	}
	_ASSERTE(g_Plugin.ImageX!=NULL);
	MCHKHEAP;
	//заменил Image[0] на ImageX - иначе вроде PgDn/PgUp не работает
	CFunctionLogger::FunctionLogger(L"OpenImagePlugin.ImageX->ImageOpen(%s)",pFileName);
	bool result = g_Plugin.ImageX->ImageOpen(pFileName, NULL, 0);
	CFunctionLogger::FunctionLogger(L"ImageOpen done");

	// Сброс флага перехвата CtrlPgDn, и первого файла серии
	// После AltPgDn/Up и отработки в ImageOpen - сбросим флаги (FW_PREVDECODER|FW_NEXTDECODER)
	g_Plugin.FlagsWork &= ~(FW_PREVDECODER|FW_NEXTDECODER);
	if (result)
		g_Plugin.FlagsWork &= ~(FW_PLUGIN_CALL|FW_FIRST_CALL|FW_FORCE_DECODE);
	else if (g_Plugin.FlagsWork & FW_FIRST_CALL) {
		g_Plugin.FlagsWork |= (FW_TERMINATE);
		// при обломе стартовой загрузки изображения нить дисплея ждет
		// WaitForSingleObject(g_Plugin.hWorkEvent, INFINITE);
		//SetEvent(g_Plugin.hWorkEvent); -- и пусть ждет. еще может дисплеи закрывать нужно, а окна уже нет
	}

	if (g_Plugin.FlagsWork & FW_QUICK_VIEW) {
		g_Plugin.RestoreTitle();
	}

	g_Plugin.MapFileName = L"";
	const bool bCaching = result 
		&& !(g_Plugin.FlagsWork & FW_JUMP_DISABLED)
		&& (g_Plugin.FarPanelInfo.Flags & PFLAGS_REALNAMES ? g_Plugin.bCachingRP : g_Plugin.bCachingVP);

lCachingDone:

	WARNING("GetAsyncKeyState нужно заменять на статус. иначе это потенциальный баг - она может вернуть не то, что ожидается");
	const bool bESC = EscapePressed();
		//GetAsyncKeyState(VK_ESCAPE) < 0;
	if (!g_Plugin.hWnd || bESC) // Display thread ended
	{
		g_Plugin.ImageX = g_Plugin.Image[0];
		//g_Plugin.ddsx = g_Plugin.Image[0]->dds;
		result = false;
		if (bESC)
			FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE)/*g_Plugin.hInput*/);
	}

	MCHKHEAP;

	if (result)
	{
		if (g_Plugin.ImageX == g_Plugin.Image[0]) // main image done
		{
			SetEvent(g_Plugin.hWorkEvent);
			//Wait ForSingleObject(g_Plugin.hDisplayEvent, INFINITE);
			if (!WaitDisplayEvent()) {
				_ASSERT(FALSE);
				return FE_OPEN_FAILED;
			}
			if (bCaching && !g_Plugin.Image[2]->pDraw && g_Plugin.Image[2]->PanelItemRaw() != PANEL_ITEM_UNAVAILABLE)
			{
				g_Plugin.Image[2]->SetPanelItemRaw(g_Plugin.Image[0]->PanelItemRaw());
				g_Plugin.ImageX = g_Plugin.Image[2];
				//g_Plugin.ddsx = g_Plugin.Image[2]->dds;
			}
			else if (bCaching && !g_Plugin.Image[1]->pDraw && g_Plugin.Image[1]->PanelItemRaw() != PANEL_ITEM_UNAVAILABLE)
			{
				g_Plugin.Image[1]->SetPanelItemRaw(g_Plugin.Image[0]->PanelItemRaw());
				g_Plugin.ImageX = g_Plugin.Image[1];
				//g_Plugin.ddsx = g_Plugin.Image[1]->dds;
			}
			else
				WorkWait();
		}
		else
		{
			if (WaitForSingleObject(g_Plugin.hDisplayEvent, 0) == WAIT_OBJECT_0)
			{
				SetEvent(g_Plugin.hDisplayEvent);
				WorkWait();
			}
			else
			{
				if (g_Plugin.ImageX == g_Plugin.Image[2]) // forward caching done
				{
					if (bCaching && !g_Plugin.bUncachedJump && !g_Plugin.Image[1]->pDraw && g_Plugin.Image[1]->PanelItemRaw() != PANEL_ITEM_UNAVAILABLE)
					{
						g_Plugin.Image[1]->SetPanelItemRaw(g_Plugin.Image[0]->PanelItemRaw());
						g_Plugin.ImageX = g_Plugin.Image[1];
						//g_Plugin.ddsx = g_Plugin.Image[1]->dds;
					}
					else
						WorkWait();
				}
				else // backward caching done
				{
					if (bCaching && !g_Plugin.bUncachedJump && !g_Plugin.Image[2]->pDraw && g_Plugin.Image[2]->PanelItemRaw() != PANEL_ITEM_UNAVAILABLE)
					{
						g_Plugin.Image[2]->SetPanelItemRaw(g_Plugin.Image[0]->PanelItemRaw());
						g_Plugin.ImageX = g_Plugin.Image[2];
						//g_Plugin.ddsx = g_Plugin.Image[2]->dds;
					}
					else
						WorkWait();
				}
			}
		}
	}

	MCHKHEAP;

	CFunctionLogger::FunctionLogger(L"OpenImagePlugin.1");

	if ((g_Plugin.FlagsWork & FW_TERMINATE) || (g_Plugin.ImageX == g_Plugin.Image[0] && !(g_Plugin.FlagsDisplay & FD_REFRESH))) // no main image
	{
		// Сюда мы по идее должны попадать только при закрытии плагина - окно уже должно быть разрушено
		// Если это только не первый вызов - тогда оно могло еще не успеть разрушиться
		//#ifdef _DEBUG
		//if (!(g_Plugin.FlagsWork & FW_FIRST_CALL)) {
		//	_ASSERTE(g_Plugin.hWnd == NULL); -- низя.
		//}
		//#endif

		TODO("А если какая-то картинка обломается - result не сбросится случайно? Viewer может тогда остаться");
		if (result)
			ExitViewerEditor();
		EFlagsResult ExitCode = FE_SKIP_ERROR;
		if (g_Plugin.hThread && g_Plugin.FlagsDisplay & FD_DISLPAYED) // exit after display
		{
			ExitCode = FE_PROCEEDED;
			if (g_Plugin.Image[0]->PanelItemRaw() != PANEL_ITEM_UNAVAILABLE)
			{
				PanelRedrawInfo pri = {g_Plugin.Image[0]->PanelItemRaw(), g_Plugin.FarPanelInfo.TopPanelItem};
				g_StartupInfo.Control(INVALID_HANDLE_VALUE, FCTL_REDRAWPANEL, 0, (LONG_PTR)&pri);
			}
		}

		CFunctionLogger::FunctionLogger(L"OpenImagePlugin.2");

		g_Plugin.Image[0]->DisplayClose();
		g_Plugin.Image[1]->DisplayClose();
		g_Plugin.Image[2]->DisplayClose();
		for (uint i = 3; i--;)
			if (g_Plugin.Image[i]->Decoder)
			{
				g_Plugin.Image[i]->Decoder->Close();
				//deletex(g_Plugin.Image[i]->Decoder);
			}

		g_Plugin.Image[1]->SetPanelItemRaw(0);
		g_Plugin.Image[2]->SetPanelItemRaw(0);
		g_Plugin.FarPanelInfo.ItemsNumber = -1;

		// Очистить все флаги КРОМЕ ..., иначе мы не узнаем закрываемся мы или нет
		WARNING("Если мы в архиве уже что-то открыли - то при обломе на втором файле перейти к следующему");
		g_Plugin.FlagsWork &= (FW_TERMINATE);

		MCHKHEAP;

		CFunctionLogger::FunctionLogger(L"OpenImagePlugin.3");

		CloseHandle(g_Plugin.hThread);
		g_Plugin.hThread = NULL;

		g_Plugin.bUncachedJump = false;
		if (g_Plugin.hWnd)
			SetEvent(g_Plugin.hWorkEvent);

		MCHKHEAP;

		free(ppi);

		CFunctionLogger::FunctionLogger(L"OpenImagePlugin.4end");

		MCHKHEAP;
		return ExitCode;
	}

	CFunctionLogger::FunctionLogger(L"OpenImagePlugin.5");

	// caching in progress
	if (g_Plugin.FlagsDisplay & FD_REFRESH)
	{
		// Ниже по коду это условие. для чего оно?
		//_ASSERTE(g_Plugin.ImageX != g_Plugin.Image[0]);

		CFunctionLogger::FunctionLogger(L"OpenImagePlugin.6");

		// Тут как-то коряво, но это нужно, чтобы сработал refresh после применения настроек диалога на лету
		////////////////////////////////////////////////
		// !!! g_Plugin.Image[0] тут не обрабатывается
		for (uint i = 3; --i;)
			if (g_Plugin.Image[i]->Decoder)
			{
				g_Plugin.Image[i]->DisplayClose();
				g_Plugin.Image[i]->Decoder->ResetProcessed();
				g_Plugin.Image[i]->Decoder->Close();
				//deletex(g_Plugin.Image[i]->Decoder);
			}
			g_Plugin.Image[1]->SetPanelItemRaw(g_Plugin.Image[0]->PanelItemRaw());
			g_Plugin.Image[2]->SetPanelItemRaw(g_Plugin.Image[0]->PanelItemRaw());
			g_Plugin.ImageX = g_Plugin.Image[0];
		g_Plugin.Image[0]->Decoder->ResetProcessed();
		g_Plugin.FlagsWork |= FW_FORCE_DECODE; // Чтобы пошел "усиленный" подбор декодера
		////////////////////////////////////////////////
		MCHKHEAP;

		if (g_Plugin.ImageX != g_Plugin.Image[0])
		{
			g_Plugin.bUncachedJump = false;
			WorkWait();
		}
		DWORD nNextDecoder = (g_Plugin.FlagsWork & (FW_PREVDECODER|FW_NEXTDECODER));
		if (nNextDecoder)
			g_Plugin.FlagsWork |= FW_FORCE_DECODE; // Чтобы пошел "усиленный" подбор декодера
		g_Plugin.FlagsDisplay &= ~FD_REFRESH; //FW_PREVDECODER|FW_NEXTDECODER - это оставим
		g_Plugin.FlagsWork |= FW_PLUGIN_CALL;
		//g_Plugin.Image[0]->dds->DeleteWorkSurface();
		//g_Plugin.Image[1]->dds->DeleteWorkSurface();
		//g_Plugin.Image[2]->dds->DeleteWorkSurface();
		for (uint i = 3; i--;) {
			g_Plugin.Image[i]->DisplayClose();
			if (g_Plugin.Image[i]->Decoder)
			{
				g_Plugin.Image[i]->Decoder->Close();
				//deletex(g_Plugin.Image[i]->Decoder);
			}
		}

		MCHKHEAP;
			
		/*if (nNextDecoder)
			iSubDecoder = PVDManager::GetNextDecoder(iSubDecoder, ((nNextDecoder & FD_NEXTDECODER) != 0));*/
			
		if (g_Plugin.FlagsWork & FW_JUMP_DISABLED)
		{
			//g_Plugin.MapFileName = g_Plugin.Image[0]->FileName;
			//lstrcpyW(g_Plugin.MapFileNameData, g_Plugin.Image[0]->FileNameData);
			g_Plugin.MapFileName = (const wchar_t*)g_Plugin.Image[0]->FileName;
			goto lReOpen;
		}
	}
	else
	{
		MCHKHEAP;

		CFunctionLogger::FunctionLogger(L"OpenImagePlugin.7");

		do {
			// крутим индекс открываемого элемента на панели
			_ASSERTE(g_Plugin.ImageX!=NULL);
			g_Plugin.ImageX->SetPanelItemRaw( g_Plugin.ImageX->PanelItemRaw() + ((g_Plugin.ImageX == g_Plugin.Image[2]) ? 1 : -1));
			g_Plugin.ImageX->Decoder->ResetProcessed();
			if ((int)g_Plugin.ImageX->PanelItemRaw() < 0 || (int)g_Plugin.ImageX->PanelItemRaw() >= g_Plugin.FarPanelInfo.ItemsNumber)
				if (!g_Plugin.bLoopJump || (int)g_Plugin.Image[0]->PanelItemRaw() < 0 || (int)g_Plugin.Image[0]->PanelItemRaw() > g_Plugin.FarPanelInfo.ItemsNumber - 1)
				{
					g_Plugin.ImageX->SetPanelItemRaw(PANEL_ITEM_UNAVAILABLE);
					result = true;
					goto lCachingDone;
				}
				else
					g_Plugin.ImageX->SetPanelItemRaw(g_Plugin.ImageX == g_Plugin.Image[2] ? 0 : g_Plugin.FarPanelInfo.ItemsNumber - 1);
			PluginPanelItem *p;
			size_t len;
			if (!(len = g_StartupInfo.Control(INVALID_HANDLE_VALUE, FCTL_GETPANELITEM, g_Plugin.ImageX->PanelItemRaw(), 0)) ||
				!(p = (PluginPanelItem*)realloc(ppi, len)) ||
				!(ppi = p, g_StartupInfo.Control(INVALID_HANDLE_VALUE, FCTL_GETPANELITEM, g_Plugin.ImageX->PanelItemRaw(), (LONG_PTR)ppi)))
				{
					g_Plugin.ImageX->SetPanelItemRaw(PANEL_ITEM_UNAVAILABLE);
					result = true;
					goto lCachingDone;
				} else {
					g_Plugin.ImageX->bSelected = (ppi->Flags & PPIF_SELECTED) == PPIF_SELECTED;
				}
		} while (ppi->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);

		if (g_Plugin.ImageX->PanelItemRaw() == g_Plugin.Image[0]->PanelItemRaw()) // found the same image
		{
			g_Plugin.ImageX->SetPanelItemRaw(PANEL_ITEM_UNAVAILABLE);
			result = true;
			goto lCachingDone;
		}

		MCHKHEAP;
	}

	CFunctionLogger::FunctionLogger(L"OpenImagePlugin.8");

	if (g_Plugin.FarPanelInfo.Flags & PFLAGS_REALNAMES)
	{
		if (size_t len = g_StartupInfo.Control(INVALID_HANDLE_VALUE, FCTL_GETPANELITEM, g_Plugin.ImageX->PanelItemRaw(), 0))
		{
			if (PluginPanelItem *p = (PluginPanelItem*)realloc(ppi, len))
			{
				if (ppi = p, g_StartupInfo.Control(INVALID_HANDLE_VALUE, FCTL_GETPANELITEM, g_Plugin.ImageX->PanelItemRaw(), (LONG_PTR)ppi))
				{
					//g_Plugin.MapFileName = ppi->FindData.lpwszFileName;
					PutFilePathName(&g_Plugin.MapFileName, ppi->FindData.lpwszFileName);
					goto lReOpen;
				}
			}
		}
	}

	CFunctionLogger::FunctionLogger(L"OpenImagePlugin.9ExitViewerEditor");

	MCHKHEAP;
	ExitViewerEditor();
	MCHKHEAP;
	PanelRedrawInfo pri = {g_Plugin.ImageX->PanelItemRaw(), g_Plugin.FarPanelInfo.TopPanelItem};
	g_StartupInfo.Control(INVALID_HANDLE_VALUE, FCTL_REDRAWPANEL, 0, (LONG_PTR)&pri);

	CFunctionLogger::FunctionLogger(L"OpenImagePlugin.10");

	// макро не срабатывает - файлы не открываются, долистывается до конца
	//ActlKeyMacro km = {MCMD_POSTMACROSTRING};
	//km.Param.PlainText.SequenceText = L"$Text \"pic:!.!\" Enter";
	//if (!g_StartupInfo.AdvControl(g_StartupInfo.ModuleNumber, ACTL_KEYMACRO, &km))
	DWORD Command = KEY_CTRL | KEY_PGDN;
	KeySequence ks = {KSFLAGS_DISABLEOUTPUT, 1, &Command};
	g_Plugin.FlagsWork |= FW_PLUGIN_CALL; // Добавил на всякий случай, а то сброситься могло
	if (!g_StartupInfo.AdvControl(g_StartupInfo.ModuleNumber, ACTL_POSTKEYSEQUENCE, &ks))
	{
		g_Plugin.ImageX->SetPanelItemRaw(PANEL_ITEM_UNAVAILABLE);
		result = true;
		goto lCachingDone;
	}
	free(ppi);
	MCHKHEAP;
	CFunctionLogger::FunctionLogger(L"OpenImagePlugin.end");
	return FE_PROCEEDED;
}

void WINAPI ClosePluginW(HANDLE hPlugin)
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
	
	if (!lbUnload && !lbLoad)
		return false;
		
	UnicodeFileName szFileName;
	PutFilePathName(&szFileName, Item);
	Item = (const wchar_t*)szFileName;  // это будет полный путь 
	UnicodeFileName::SkipPrefix(&Item); // а это уже без UNC префиксов
	const wchar_t* szLines[10];
	MCHKHEAP;
	
	ModuleData* plug = PVDManager::FindPlugin(Item, true/*abShowErrors*/);
	if (lbUnload) {
		if (plug != NULL) {
			// Сам объект ModuleData не разрушается. Только выгружается pvd-шка
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
				g_StartupInfo.Message(g_StartupInfo.ModuleNumber, FMSG_WARNING|FMSG_MB_OK, 
					NULL, szLines, 3, 0);
			}
			else
			{
				FindClose(hFind);
				// плуг новый - будет создан новый ModuleData*
				wchar_t* psz = _wcsdup(Item);
				wchar_t* pszSlash = wcsrchr(psz, L'\\');
				if (pszSlash) *pszSlash = 0;
				MCHKHEAP;
				plug = PVDManager::LoadPlugin(psz, fnd, true/*abShowErrors*/);
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
				g_StartupInfo.Message(g_StartupInfo.ModuleNumber, FMSG_WARNING|FMSG_MB_OK, 
					NULL, szLines, 4, 0);
			}
			MCHKHEAP;
		}
	}

	// и сразу сортирнуть, т.к. выгружаем/загружаем только один плуг
	PVDManager::SortPlugins2(); // Там же плагины по типам распределяются
	MCHKHEAP;

	return true;
}

HANDLE WINAPI OpenPluginW(int OpenFrom, INT_PTR Item)
{
	if (!nFarVersion || ((HIWORD(nFarVersion) > 1144) && (HIWORD(nFarVersion) < 1148)))
		return INVALID_HANDLE_VALUE;
		
	if (!gnMainThreadId)
		gnMainThreadId = GetCurrentThreadId();

	MCHKHEAP;

	// Это явный вызов плагина. Выполняем полную инициализацию
	g_Plugin.InitPlugin();

	// Явный вызов - ставим флаг (влияет на подбор декодеров, включение кнопки-модификатора)
	g_Plugin.FlagsWork |= FW_FIRST_CALL|FW_FORCE_DECODE;
	g_Plugin.FlagsWork &= ~(FW_TERMINATE|FW_QUICK_VIEW);
	g_Plugin.SelectionChanged = false; // Чтобы знать, что менялось выделение
	
	
	FUNCLOGGERI(L"OpenPluginW(%i)", OpenFrom);


	switch (OpenFrom)
	{
		case OPEN_VIEWER:
		case OPEN_EDITOR:
			{
				struct WindowInfo wi = {-1};
				wchar_t szName[0x1000]; TODO("Сделать динамическим?");
				wi.Name = szName; wi.NameSize =	sizeof(szName)/sizeof(szName[0]);
				if (g_StartupInfo.AdvControl(g_StartupInfo.ModuleNumber, ACTL_GETWINDOWINFO, &wi))
				{
					// -- явный вызов. Расширения игнорируем
					//if (g_Plugin.IsExtensionIgnored(wi.Name))
					//	return INVALID_HANDLE_VALUE;
					//if (!wcsncmp(wi.Name,L"\\\\?\\",4)) wi.Name+=4;
					//lstrcpynW(g_Plugin.Image[0]->FileNameData, wi.Name, sizeofarray(g_Plugin.Image[0]->FileNameData));
					g_Plugin.Image[0]->FileName = wi.Name;
				} else {
					//g_Plugin.Image[0]->FileName = L"";
					return INVALID_HANDLE_VALUE;
				}
				g_Plugin.FlagsWork |= FW_JUMP_DISABLED;
			} break;
		case OPEN_COMMANDLINE:
			if (ProcessPrefix((const wchar_t*)Item)) {
				//load:, unload:, reload:
				g_Plugin.FlagsWork &= ~(FW_FIRST_CALL|FW_FORCE_DECODE);
				return INVALID_HANDLE_VALUE;
			}
			// только вызовом через префикс можно обойти (наверное) IsExtensionIgnored
			TestJumpDisable((const wchar_t*)Item);
			break;
		case OPEN_PLUGINSMENU:
			TestJumpDisable(L"");
	}

	g_Plugin.FlagsWork |= FW_PLUGIN_CALL;
	//g_Plugin.MapFileName = g_Plugin.Image[0]->FileName;
	//lstrcpy(g_Plugin.MapFileNameData, g_Plugin.Image[0]->FileNameData);
	g_Plugin.MapFileName = (const wchar_t*)g_Plugin.Image[0]->FileName;

	g_Plugin.SaveTitle();

	//HANDLE rc = OpenImagePlugin(g_Plugin.MapFileName);
	EFlagsResult rc;
	
	TRY {
		rc = OpenImagePlugin(g_Plugin.MapFileName);
		CFunctionLogger::FunctionLogger(L"OpenPluginW.OpenImagePlugin done");
		switch (rc) {
			case FE_PROCEEDED:
				CFunctionLogger::FunctionLogger(L"OpenPluginW.FE_PROCEEDED");
				break;
			case FE_SKIP_ERROR:
				CFunctionLogger::FunctionLogger(L"OpenPluginW.FE_SKIP_ERROR");
				break; // Не открыли, но ошибку показывать не нужно
			case FE_UNKNOWN_EXTENSION:
				// Неизвестное расширение
				CFunctionLogger::FunctionLogger(L"OpenPluginW.FE_UNKNOWN_EXTENSION");
				if (g_Plugin.FlagsWork & FW_FIRST_CALL) {
					g_StartupInfo.Message(g_StartupInfo.ModuleNumber, FMSG_ALLINONE|FMSG_MB_OK,
						NULL, (const wchar_t * const *)GetMsg(MIOpenUnknownExtension), 0, 0);
				}
				break;
			case FE_OPEN_FAILED:
				// Ошибка открытия файла
				CFunctionLogger::FunctionLogger(L"OpenPluginW.FE_OPEN_FAILED");
				if (g_Plugin.FlagsWork & FW_FIRST_CALL) {
					g_StartupInfo.Message(g_StartupInfo.ModuleNumber, FMSG_ALLINONE|FMSG_MB_OK,
						NULL, (const wchar_t * const *)GetMsg(MIOpenFailedMsg), 0, 0);
				}
				break;
			case FE_NO_PANEL_ITEM:
				// Больше нет элементов на панели?
				CFunctionLogger::FunctionLogger(L"OpenPluginW.FE_NO_PANEL_ITEM");
				break;
		}
	} CATCH {
		CFunctionLogger::FunctionLogger(L"OpenPluginW.OpenImagePlugin raised an exception");
		g_StartupInfo.Message(g_StartupInfo.ModuleNumber, FMSG_ALLINONE|FMSG_MB_OK,
			NULL,
			(const wchar_t * const *)GetMsg(MIOpenImgPlugException),
			0, 0);
	}
	MCHKHEAP;

	if ((g_Plugin.FlagsWork & FW_TERMINATE) && g_Plugin.FlagsWork != FW_TERMINATE) {
		_ASSERTE(g_Plugin.FlagsWork == FW_TERMINATE);
		g_Plugin.FlagsWork = FW_TERMINATE;
	}
	
	if (g_Plugin.FlagsWork & FW_TERMINATE)
	{
		//if (rc == (HANDLE)-2) -- теперь всегда, на всякий
		g_Plugin.RestoreTitle();
		// GFL портит консоль при просмотре WMF
		g_StartupInfo.AdvControl(g_StartupInfo.ModuleNumber, ACTL_REDRAWALL, NULL);

		TODO("Имеет смысл, только если выбрана настройка 'Показывать выделенные файлы вверху'");
		if (g_Plugin.SelectionChanged) {
			ActlKeyMacro km = {MCMD_POSTMACROSTRING};
			km.Param.PlainText.SequenceText = L"CtrlR";
			g_StartupInfo.AdvControl(g_StartupInfo.ModuleNumber, ACTL_KEYMACRO, &km);
			g_Plugin.SelectionChanged = false;
		}

		CheckAllClosed();
	}

	return INVALID_HANDLE_VALUE;
}

bool IsKeyPressed(WORD vk)
{
	USHORT st = GetKeyState(vk);
	if ((st & 0x8000))
		return true;

	// что-то в ConEmu GetKeyState иногда ничего не дает? возвращается 0
	if (g_Plugin.hConEmuWnd && g_Plugin.hConEmuCtrlPressed && g_Plugin.hConEmuShiftPressed) {
		DWORD dwWait = WAIT_TIMEOUT;
		if (vk == VK_CONTROL)
			dwWait = WaitForSingleObject(g_Plugin.hConEmuCtrlPressed, 0);
		else if (vk == VK_SHIFT)
			dwWait = WaitForSingleObject(g_Plugin.hConEmuShiftPressed, 0);
		return (dwWait == WAIT_OBJECT_0);
	}

	return false;
}

int WINAPI ProcessEditorEventW(int Event, void *Param)
{
	g_Plugin.InitHooks();

	if (Event == EE_READ && g_Plugin.bHookEdit)
	{
		// Не перехватывать CtrlShiftF4
		if (!g_Plugin.bHookCtrlShiftF4) {
			if (IsKeyPressed(VK_SHIFT) && IsKeyPressed(VK_CONTROL) /*&& (GetKeyState(VK_F4) & 0x8000)*/) {
				return 0;
			}
		}
	
		EditorInfo ei;
		if (g_StartupInfo.EditorControl(ECTL_GETINFO, &ei))
		{
			wchar_t* pszFileName = NULL;
			int nLen = g_StartupInfo.EditorControl(ECTL_GETFILENAME, NULL);
			if (nLen>0 && (pszFileName = (wchar_t*)calloc(nLen+1,2))!=NULL) {
				g_StartupInfo.EditorControl(ECTL_GETFILENAME, pszFileName);
				
				if (!g_Plugin.IsExtensionIgnored(pszFileName))
				{
					g_Plugin.InitPlugin();
					
					g_Plugin.FlagsWork |= FW_VE_HOOK;
					g_Plugin.FlagsWork &= ~(FW_TERMINATE|FW_QUICK_VIEW);

					g_Plugin.SaveTitle();

					TestJumpDisable(pszFileName/*ei.FileName*/);
					//g_Plugin.MapFileName = g_Plugin.Image[0]->FileName;
					//lstrcpy(g_Plugin.MapFileNameData, g_Plugin.Image[0]->FileNameData);
					g_Plugin.MapFileName = (const wchar_t*)g_Plugin.Image[0]->FileName;

					TODO("Флаги TERMINATED, FIRSTCALL, переменная g_Plugin.SelectionChanged");
					
					//OpenImagePlugin(g_Plugin.MapFileName);
					int rc = 0;
					
					TRY {
						rc = OpenImagePlugin(g_Plugin.MapFileName);
					} CATCH {
						CFunctionLogger::FunctionLogger(L"ProcessEditorEventW.OpenImagePlugin raised an exception");
						g_StartupInfo.Message(g_StartupInfo.ModuleNumber, FMSG_ALLINONE|FMSG_MB_OK,
							NULL,
							(const wchar_t * const *)GetMsg(MIOpenImgPlugException),
							0, 0);
					}
					

					g_Plugin.RestoreTitle();
				}

				free(pszFileName);
			}
		}
	}
	return 0;
}

typedef COORD (WINAPI *GetConsoleFontSize_t)(HANDLE hConsoleOutput,DWORD nFont);
typedef BOOL (WINAPI *GetCurrentConsoleFont_t)(HANDLE hConsoleOutput,BOOL bMaximumWindow,PCONSOLE_FONT_INFO lpConsoleCurrentFont);

int WINAPI ProcessViewerEventW(int Event, void *Param)
{
	g_Plugin.InitHooks();

	if (Event == VE_READ && (g_Plugin.bHookQuickView || g_Plugin.bHookView))
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
	
		ViewerInfo vi = {0};
		vi.StructSize = sizeof(vi);
		if (g_StartupInfo.ViewerControl(VCTL_GETINFO, &vi))
		{
			g_Plugin.FlagsWork |= FW_VE_HOOK;
			// Сначала - сбросим QView! Иначе он может ошибочно остаться от предыдущего QView при отрытии обычного View
			g_Plugin.FlagsWork &= ~(FW_TERMINATE|FW_QUICK_VIEW);

			PanelInfo ppi[2];
			if (g_StartupInfo.Control(PANEL_ACTIVE, FCTL_GETPANELINFO, 0, (LONG_PTR)ppi) && g_StartupInfo.Control(PANEL_PASSIVE, FCTL_GETPANELINFO, 0, (LONG_PTR)(ppi + 1)))
			{
				for (uint i = 2; i--;)
					if (ppi[i].PanelType == PTYPE_QVIEWPANEL && ppi[i].PanelRect.right - ppi[i].PanelRect.left - 1 == vi.WindowSizeX)
					{
						CONSOLE_SCREEN_BUFFER_INFO csbi;
						RECT FarRect;
						//HWND hConEmu = GetConEmuHwnd(), hConEmuRoot = NULL;
						CheckConEmu();
						//HWND hWnd = hConEmu;
						HWND hWnd = g_Plugin.hFarWnd;
						//if (!hWnd) {
						//	const HWND hFarWnd = (HWND)g_StartupInfo.AdvControl(g_StartupInfo.ModuleNumber, ACTL_GETFARHWND, NULL);
						//	hWnd = GetAncestor(hFarWnd, GA_PARENT);
						//	WARNING("Эта проверка сомнительна в условиях многодисплейной конфигурации");
						//	if (hWnd == g_Plugin.hDesktopWnd)
						//		hWnd = hFarWnd;
						//} else {
						//	hConEmuRoot = GetParent(hConEmu);
						//}
						SMALL_RECT FarWorkRect;
						if (!GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE)/*g_Plugin.hOutput*/, &csbi) 
							|| !GetClientRect(hWnd, &FarRect)
							|| !g_StartupInfo.AdvControl(g_StartupInfo.ModuleNumber, ACTL_GETFARRECT, &FarWorkRect))
						{
							g_Plugin.FlagsWork &= ~FW_VE_HOOK;
							break;
						}

						// Поддержка "Far /w"
						csbi.srWindow.Left -= FarWorkRect.Left; csbi.srWindow.Right -= FarWorkRect.Left;
						csbi.srWindow.Top -= FarWorkRect.Top; csbi.srWindow.Bottom -= FarWorkRect.Top;

						if (ppi[i].PanelRect.left >= csbi.srWindow.Right || ppi[i].PanelRect.right <= csbi.srWindow.Left || ppi[i].PanelRect.top >= csbi.srWindow.Bottom || ppi[i].PanelRect.bottom - 2 <= csbi.srWindow.Top) {g_Plugin.FlagsWork &= ~FW_VE_HOOK; break;}
						uint wdx = 0, wdy = 0; // Видимо это задел SEt'а на обработку черной окантовки вокруг консоли, если окно развернуто
						//uint dx1 = (FarRect.right - FarRect.left + (csbi.srWindow.Right - csbi.srWindow.Left)) / (csbi.srWindow.Right - csbi.srWindow.Left + 1);
						uint dx0 = (csbi.srWindow.Right - csbi.srWindow.Left + 1);
						// Когда FAR Maximized - в консоли часть символов может не влезать
						uint dx = (FarRect.right - FarRect.left + dx0/2) / dx0;
						//uint dy1 = (FarRect.bottom - FarRect.top + (csbi.srWindow.Bottom - csbi.srWindow.Top)) / (csbi.srWindow.Bottom - csbi.srWindow.Top + 1);
						uint dy0 = (csbi.srWindow.Bottom - csbi.srWindow.Top + 1);
						uint dy = (FarRect.bottom - FarRect.top + dy0/2) / dy0;
						// Для реальной консоли - попробуем получить ширину и высоту через WinAPI
						if (!g_Plugin.hConEmuWnd) {
							// Эти функции есть в XP и выше
							GetCurrentConsoleFont_t fGetCurrentConsoleFont = (GetCurrentConsoleFont_t)GetProcAddress(GetModuleHandle(_T("kernel32.dll")),"GetCurrentConsoleFont");
							GetConsoleFontSize_t fGetConsoleFontSize = (GetConsoleFontSize_t)GetProcAddress(GetModuleHandle(_T("kernel32.dll")),"GetConsoleFontSize");
							if (fGetCurrentConsoleFont) {
								CONSOLE_FONT_INFO cfi = {0};
								BOOL bMaximized = IsZoomed(g_Plugin.hFarWnd);
								BOOL bFontRC = fGetCurrentConsoleFont(GetStdHandle(STD_OUTPUT_HANDLE),bMaximized,&cfi);
								if (bFontRC) {
									COORD cr = fGetConsoleFontSize(GetStdHandle(STD_OUTPUT_HANDLE), cfi.nFont);
									if (cr.X && cr.Y) {
										//_ASSERTE(dx == cr.X && dy == cr.Y);
										dx = cr.X;
										dy = cr.Y;
									}
								}
							}
						}
						//BUGBUG. Тут потенциально можно нарваться. Если панель будет вообще не видна из-за прокрутки
						g_Plugin.ViewPanelT = ppi[i].PanelRect;
						g_Plugin.ViewPanelG.left   = wdx + (ppi[i].PanelRect.left + 1 - csbi.srWindow.Left) * dx;
						g_Plugin.ViewPanelG.right  = wdx + (ppi[i].PanelRect.right - csbi.srWindow.Left) * dx;
						g_Plugin.ViewPanelG.top    = wdy + (ppi[i].PanelRect.top + 1 - csbi.srWindow.Top) * dy;
						g_Plugin.ViewPanelG.bottom = wdy + (ppi[i].PanelRect.bottom - 2 - csbi.srWindow.Top) * dy;
						g_Plugin.FlagsWork |= FW_QUICK_VIEW | FW_JUMP_DISABLED;
						if (g_Plugin.hConEmuWnd)
							MapWindowPoints(g_Plugin.hFarWnd, g_Plugin.hConEmuWnd, (LPPOINT)&g_Plugin.ViewPanelG, 2);
						break;
					}
			}

			// в режиме PanelViews нужно избежать излишних активаций при скроллировании!
			bool bSkipCauseTH = false;
			if (g_Plugin.hConEmuWnd && (g_Plugin.FlagsWork & FW_QUICK_VIEW))
			{
				TCHAR szEnvVar[128];
				if (GetEnvironmentVariable(TH_ENVVAR_NAME, szEnvVar, ARRAYSIZE(szEnvVar))) {
					if (!lstrcmp(szEnvVar, TH_ENVVAR_SCROLL)) {
						bSkipCauseTH = true;
					}
				}
			}
			
			if ((g_Plugin.FlagsWork & FW_QUICK_VIEW) && bSkipCauseTH)
				g_Plugin.FlagsWork = 0;
			else if (!(g_Plugin.FlagsWork & FW_QUICK_VIEW) && !g_Plugin.bHookCtrlShiftF3 && lbCtrlShiftPressed)
				g_Plugin.FlagsWork = 0;
			else if (!(g_Plugin.FlagsWork & FW_VE_HOOK) || (g_Plugin.FlagsWork & FW_QUICK_VIEW ? !g_Plugin.bHookQuickView : !g_Plugin.bHookView))
				g_Plugin.FlagsWork = 0;
			else if (!vi.FileName[0] || g_Plugin.IsExtensionIgnored(vi.FileName))
				g_Plugin.FlagsWork = 0;
			else
			{
				g_Plugin.InitPlugin();
				
				g_Plugin.SaveTitle();

				if (g_Plugin.FlagsWork & FW_QUICK_VIEW) {
					//g_Plugin.MapFileName = vi.FileName;
					//if (!wcsncmp(vi.FileName,L"\\\\?\\",4)) vi.FileName+=4;
					g_Plugin.MapFileName = vi.FileName;
				}
				else
				{
					TestJumpDisable(vi.FileName);
					//g_Plugin.MapFileName = g_Plugin.Image[0]->FileName;
					//lstrcpy(g_Plugin.MapFileNameData, g_Plugin.Image[0]->FileNameData);
					g_Plugin.MapFileName = (const wchar_t*)g_Plugin.Image[0]->FileName;
					TODO("Флаги TERMINATED, FIRSTCALL, переменная g_Plugin.SelectionChanged");
				}
				
				//OpenImagePlugin(g_Plugin.MapFileName);
				int rc = 0;
				
				TRY {
					rc = OpenImagePlugin(g_Plugin.MapFileName);
				} CATCH {
					CFunctionLogger::FunctionLogger(L"ProcessViewerEventW.OpenImagePlugin raised an exception");
					g_StartupInfo.Message(g_StartupInfo.ModuleNumber, FMSG_ALLINONE|FMSG_MB_OK,
						NULL,
						(const wchar_t * const *)GetMsg(MIOpenImgPlugException),
						0, 0);
					rc = FE_EXCEPTION;
				}

				g_Plugin.RestoreTitle();
			}
		}
	}
	return 0;
}

void WINAPI GetPluginInfoW(struct PluginInfo *Info)
{
	Info->StructSize = sizeof(*Info);
	Info->Flags = PF_VIEWER | PF_EDITOR;

	g_Plugin.InitHooks();

	static const wchar_t *const PluginNameStrings[] = {g_Plugin.pszPluginTitle}; // MIPluginName
	Info->PluginMenuStrings = Info->PluginConfigStrings = PluginNameStrings;
	Info->PluginMenuStringsNumber = Info->PluginConfigStringsNumber = sizeofarray(PluginNameStrings);
	Info->CommandPrefix = g_Plugin.sHookPrefix; //L"pic"; DEFAULT_PREFIX
}

int WINAPI GetMinFarVersionW(void)
{
	return FARMANAGERVERSION;
}

//#ifdef _DEBUG
//void TTT(void*);
//#endif

void WINAPI SetStartupInfoW(const struct PluginStartupInfo *xInfo)
{
	g_StartupInfo = *xInfo;
	g_FSF = *xInfo->FSF;
	g_StartupInfo.FSF = &g_FSF;

	//TTT(&g_FSF);
	
	nFarVersion = 0;
	g_StartupInfo.AdvControl(g_StartupInfo.ModuleNumber, ACTL_GETFARVERSION, &nFarVersion);

	if (!nFarVersion || ((HIWORD(nFarVersion) > 1144) && (HIWORD(nFarVersion) < 1148)))
	{
		g_StartupInfo.Message(g_StartupInfo.ModuleNumber, FMSG_ALLINONE|FMSG_MB_OK,
			NULL,
			(const wchar_t * const *)GetMsg(MIFarBuildError), // L"PictureView2\nThis version of FAR manager is not supported\nPlease update FAR",
			0, 0);
		return;
	}

	// Если имя файла плагина НЕ начинается с '0' - листание может перехватить другой
	// плагин, что вызовет кучу проблем.
	const wchar_t *pszFileName = wcsrchr(xInfo->ModuleName, L'\\');
	if (pszFileName) pszFileName++; else pszFileName = xInfo->ModuleName;
	if (*pszFileName != L'0') {
		g_StartupInfo.Message(g_StartupInfo.ModuleNumber, FMSG_ALLINONE|FMSG_MB_OK,
			NULL,
			(const wchar_t * const *)GetMsg(MIPluginNameError), 
			0, 0);
	}
}

void WINAPI ExitFARW()
{
	MCHKHEAP;

	for (UINT i=0; i<sizeofarray(g_Plugin.Image); i++) {
		if (g_Plugin.Image[i]) {
			delete g_Plugin.Image[i];
			g_Plugin.Image[i] = NULL;
		}
	}
	CloseHandle(g_Plugin.hDisplayEvent);
	CloseHandle(g_Plugin.hWorkEvent);

	MCHKHEAP;

	PVDManager::UnloadPlugins2();

	MCHKHEAP;

	if (g_RootKey) { free(g_RootKey); g_RootKey = NULL; }
	if (g_SelfPath) { free(g_SelfPath); g_SelfPath = NULL; }

	MCHKHEAP;

	UnregisterClass(g_WndClassName, g_hInstance);
}


#ifdef _DEBUG
char gsz_MDEBUG_TRAP_MSG[3000];
char gsz_MDEBUG_TRAP_MSG_APPEND[2000];
HWND gh_MDEBUG_TRAP_PARENT_WND = NULL;
int __stdcall _MDEBUG_TRAP(LPCSTR asFile, int anLine)
{
	//__debugbreak();
	_ASSERT(FALSE);
    wsprintfA(gsz_MDEBUG_TRAP_MSG, "MDEBUG_TRAP\r\n%s(%i)\r\n", asFile, anLine);
    if (gsz_MDEBUG_TRAP_MSG_APPEND[0])
        lstrcatA(gsz_MDEBUG_TRAP_MSG,gsz_MDEBUG_TRAP_MSG_APPEND);
    MessageBoxA(NULL,gsz_MDEBUG_TRAP_MSG,"MDEBUG_TRAP",MB_OK|MB_ICONSTOP);
    return 0;
}
int MDEBUG_CHK = TRUE;

int MyCrtDbgReportW(int _ReportType, const wchar_t * _Filename, int _LineNumber, const wchar_t * _ModuleName, const wchar_t * _Format)
{
	return _CrtDbgReportW(_ReportType, _Filename, _LineNumber, _ModuleName, _Format);
}

#endif


BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD fdwReason, LPVOID lpvReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH) {
		//#ifdef _DEBUG
		//		MessageBox(NULL, L"PicView loaded", L"0pictureview.dll", MB_OK|MB_SYSTEMMODAL);
		//#endif
		g_hInstance = hInstance;
	}
	return TRUE;
}
