
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

#include "PictureView.h"
#include "ConEmuSupport.h"
#include <Tlhelp32.h>

const wchar_t *GetMsg(int MsgId)
{
	return g_StartupInfo.GetMsg(PluginNumber, MsgId);
}

void RegKeyRead(HKEY RegKey, const wchar_t* const Name, bool *const Param, const bool Default)
{
	u32 len = 4, val;
	*Param = RegQueryValueExW(RegKey, Name, NULL, NULL, (LPBYTE)&val, (LPDWORD)&len) ? Default : val;
}

void RegKeyRead(HKEY RegKey, const wchar_t* const Name, u32 *const Param, const u32 Default)
{
	u32 len = 4;
	if (RegQueryValueExW(RegKey, Name, NULL, NULL, (LPBYTE)Param, (LPDWORD)&len))
		*Param = Default;
}

LONG RegKeyWrite(HKEY RegKey, const wchar_t* const Name, const u32 Param)
{
	return RegSetValueExW(RegKey, Name, 0, REG_DWORD, (const BYTE*) &Param, 4);
}

// Выделить память и слепить полный путь к "Dir\Name".
// Dir не обязан содержать на конце слэш
// Если abReverseName==true - то при наличии в Name обратных слэшей - они заменяются на прямые (для реестра)
wchar_t* ConcatPath(const wchar_t* Dir, const wchar_t* Name, bool abReverseName/*=false*/)
{
	_ASSERTE(Dir && Name); _ASSERTE(*Dir && *Name);
	size_t nDirLen = lstrlen(Dir);
	size_t nNameLen = lstrlen(Name);

	wchar_t *pszFull = (wchar_t*)calloc(nDirLen+nNameLen+2,sizeof(wchar_t)); // + '\0' + '\\'
	if (!pszFull) {
		_ASSERTE(pszFull);
		return NULL;
	}

	lstrcpy(pszFull, Dir);
	if (pszFull[nDirLen-1] != L'\\') {
		pszFull[nDirLen++] = L'\\';
	}
	lstrcpy(pszFull+nDirLen, Name);

	if (abReverseName) {
		wchar_t* psz = wcschr(pszFull+nDirLen, L'\\');
		while (psz) {
			*psz = '/';
			psz = wcschr(psz+1, L'\\');
		}
	}

	return pszFull;
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
	HWND hRoot = NULL, hConEmu = NULL, hConWnd = NULL;
	BOOL lbGui = GetConEmuHwnd(hConEmu, hRoot, hConWnd);

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
	if (lbGui)
	{
		TCHAR szName[64]; DWORD dwPID = GetCurrentProcessId();
		wsprintf(szName, CEKEYEVENT_CTRL, dwPID);
		g_Plugin.hConEmuCtrlPressed = OpenEvent(SYNCHRONIZE, FALSE, szName);
		wsprintf(szName, CEKEYEVENT_SHIFT, dwPID);
		g_Plugin.hConEmuShiftPressed = OpenEvent(SYNCHRONIZE, FALSE, szName);
	}
	//else
	//{
	//	//g_Plugin.hConEmuWnd = NULL;
	//	hNewFarWnd = (HWND)g_StartupInfo.AdvControl(PluginNumber, ACTL_GETFARHWND, NULL);
	//	//const HWND hAncestor = GetAncestor(g_Plugin.hFarWnd, GA_PARENT);
	//	//if (hAncestor != g_Plugin.hDesktopWnd) -- пережитки старых conemu. убито
	//	//	g_Plugin.hFarWnd = hAncestor;
	//}
	////}

	g_Plugin.hFarWnd = hConWnd;
	g_Plugin.hConEmuWnd = hConEmu;

	return (g_Plugin.hConEmuWnd!=NULL);
}

bool ExtensionMatch(LPTSTR asExtList, LPCTSTR asExt)
{
	if (!asExtList || !asExt) return false;
	if (!*asExtList || !*asExt) return false;

	while (*asExtList) {
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

bool CheckFarMacroText(LPCWSTR apszMacro)
{
	int ErrCode = 0;
	
#ifdef FAR_UNICODE
	MacroSendMacroText mcr = {sizeof(mcr)};
	mcr.SequenceText = apszMacro;
	mcr.Flags = KMFLAGS_SILENTCHECK;
	if (g_StartupInfo.MacroControl(PluginNumber, MCTL_SENDSTRING, MSSC_CHECK, &mcr) == FALSE)
	{
		//size_t iRcSize = psi.MacroControl(MCTLARG0, MCTL_GETLASTERROR, 0, NULL);
		//Result = (MacroParseResult*)calloc(iRcSize,0);
		//if (Result)
		//{
		//	Result->StructSize = sizeof(*Result);
		//	psi.MacroControl(MCTLARG0, MCTL_GETLASTERROR, iRcSize, Result);
		//}
		ErrCode = -1;
	}
#else
	ActlKeyMacro mcr = {MCMD_CHECKMACRO};
	mcr.Param.PlainText.SequenceText = apszMacro;
	//mcr.Param.PlainText.Flags = KSFLAGS_SILENTCHECK;
	g_StartupInfo.AdvControl(psi.ModuleNumber, ACTL_KEYMACRO, &mcr);
	ErrCode = mcr.Param.MacroResult.ErrCode;
#endif
	
	if (ErrCode != 0)
	{
		return false;
	}
	
	return true;
}

bool PostMacro(LPCWSTR apszMacro, BOOL abDisableRedraw)
{
	INT_PTR iRc = 0;

#ifdef FAR_UNICODE

	//MacroSendMacroText mcr = {sizeof(MacroSendMacroText)};
	//mcr.Flags = abDisableRedraw ? KMFLAGS_DISABLEOUTPUT : 0;
	//mcr.SequenceText = apszMacro;
	//g_StartupInfo.MacroControl(PluginNumber, MCTL_SENDSTRING, 0, &mcr);

	MacroSendMacroText mcr = {sizeof(mcr)};
	mcr.SequenceText = apszMacro;
	mcr.Flags = abDisableRedraw ? KMFLAGS_DISABLEOUTPUT : 0;
	iRc = g_StartupInfo.MacroControl(PluginNumber, MCTL_SENDSTRING, MSSC_CHECK, &mcr);
	
#else

	ActlKeyMacro mcr;
	mcr.Command = MCMD_POSTMACROSTRING;
	mcr.Param.PlainText.Flags = abDisableRedraw ? KSFLAGS_DISABLEOUTPUT : 0;
	mcr.Param.PlainText.SequenceText = apszMacro;
	iRc = g_StartupInfo.AdvControl(PluginNumber, ACTL_KEYMACRO, (void*)&mcr);

#endif

	return (iRc != 0);
}

// Если имя не содержит полный путь - сначала сделать FCTL_GETPANELDIR
// и слепить из него и pFileName полный путь к файлу!
//bool PutFilePathName(wchar_t* pFullName, const wchar_t* pFileName, bool abNeedUnquote=false)
//extern bool PutFilePathName1154(CUnicodeFileName* pFullName, const wchar_t* pFileName, void* fsf, bool abIsFolder);
//extern bool PutFilePathName1017(CUnicodeFileName* aFullName, const wchar_t* pFileName, bool abIsFolder);
bool PutFilePathName1154(CUnicodeFileName* pFullName, const wchar_t* pFileName, void* fsf, bool abIsFolder)
{
	if (!pFileName || !*pFileName || !pFullName) {
		_ASSERTE(pFileName && *pFileName);
		return false;
	}

	bool result = false;
	_ASSERTE(pFileName[0] != L'"');
	
	int nLen = (int)((FarStandardFunctions*)fsf)->ConvertPath(CPM_FULL, pFileName, NULL, 0);
	if (nLen > 0) {
		((FarStandardFunctions*)fsf)->ConvertPath(CPM_FULL, pFileName, pFullName->GetBuffer(nLen+2), nLen);
		result = pFullName->ReleaseBuffer(abIsFolder);
	}
	
	return result;
}

bool PutFilePathName(CUnicodeFileName* pFullName, const wchar_t* pFileName, bool abIsFolder)
{
	// Ахтунг: использовать GetFullPathNameW для преобразования относительного пути в абсолютный после 1145 билда нельзя, используйте ConvertPath(CPM_FULL, ...).
	WARNING("Проверить, что будет с функцией, если на вход придет pFileName=\\\?\\UNC\\Server\\Share\\File.exe");

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
	//	result = PutFilePathName1017(pFullName, pFileName, abIsFolder);
	//} else if ((HIWORD(nFarVersion)) >= 1148) {
	result = PutFilePathName1154(pFullName, pFileName, &g_FSF, abIsFolder);
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
	//	if (size_t len = g_StartupInfo.PanelControl(INVALID_HANDLE_VALUE, FCTL_GETPANELDIR, 0, 0))
	//		if (pCurDir = (wchar_t*)malloc(len*2+2))
	//			if (!g_StartupInfo.PanelControl(INVALID_HANDLE_VALUE, FCTL_GETPANELDIR, len, (LONG_PTR)pCurDir))
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

// Плагин может быть вызван в первый раз из фоновой нити (диалог поиска при поиске в архивах)
// Поэтому простой "gnMainThreadId = GetCurrentThreadId();" не прокатит. Нужно искать первую нить процесса!
DWORD GetMainThreadId()
{
	DWORD nThreadID = 0;
	DWORD nProcID = GetCurrentProcessId();
	HANDLE h = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (h != INVALID_HANDLE_VALUE)
	{
		THREADENTRY32 ti = {sizeof(THREADENTRY32)};
		if (Thread32First(h, &ti))
		{
			do {
				// Нужно найти ПЕРВУЮ нить процесса
				if (ti.th32OwnerProcessID == nProcID) {
					nThreadID = ti.th32ThreadID;
					break;
				}
			} while (Thread32Next(h, &ti));
		}
		CloseHandle(h);
	}

	// Нехорошо. Должна быть найдена. Вернем хоть что-то (текущую нить)
	if (!nThreadID) {
		_ASSERTE(nThreadID!=0);
		nThreadID = GetCurrentThreadId();
	}
	return nThreadID;
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

//int MyCrtDbgReportW(int _ReportType, const wchar_t * _Filename, int _LineNumber, const wchar_t * _ModuleName, const wchar_t * _Format)
//{
//	return _CrtDbgReportW(_ReportType, _Filename, _LineNumber, _ModuleName, _Format);
//}
#endif



WARNING("Проверить эту функцию");
void ExitViewerEditor(void)
{
	if ((g_Plugin.FlagsWork & FW_VE_HOOK) && !(g_Plugin.FlagsWork & FW_QUICK_VIEW))
	{
		PostMacro(L"Esc", TRUE);
		//DWORD Command = KEY_ESC;
		//KeySequence ks = {KSFLAGS_DISABLEOUTPUT, 1, &Command};
		//g_StartupInfo.AdvControl(PluginNumber, ACTL_POSTKEYSEQUENCE, &ks);
	}

	g_Plugin.FlagsWork &= ~FW_VE_HOOK;
}


int GetFarKey(LONG_PTR Param2)
{
	int Key = 0;

	#ifdef FAR_UNICODE
	if (Param2)
	{
		INPUT_RECORD* r = (INPUT_RECORD*)Param2;
		if (r->EventType == KEY_EVENT)
		{
			switch (r->Event.KeyEvent.wVirtualKeyCode)
			{
			case '1': case '2': case '3': case '4': case '5':
				Key = r->Event.KeyEvent.wVirtualKeyCode; break;
			case VK_F3:
				Key = KEY_F3; break;
			case VK_F4:
				Key = KEY_F4; break;
			case VK_UP:
				Key = KEY_UP; break;
			case VK_DOWN:
				Key = KEY_DOWN; break;
			}
			if (Key)
			{
				if (r->Event.KeyEvent.dwControlKeyState & LEFT_ALT_PRESSED)
					Key |= KEY_ALT;
				if (r->Event.KeyEvent.dwControlKeyState & RIGHT_ALT_PRESSED)
					Key |= KEY_RALT;
				if (r->Event.KeyEvent.dwControlKeyState & LEFT_CTRL_PRESSED)
					Key |= KEY_CTRL;
				if (r->Event.KeyEvent.dwControlKeyState & RIGHT_CTRL_PRESSED)
					Key |= KEY_RCTRL;
				if (r->Event.KeyEvent.dwControlKeyState & SHIFT_PRESSED)
					Key |= KEY_SHIFT;
			}
		}
	}
	#else
		Key = (int)Param2;
	#endif

	return Key;
}
