
/*
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
*/


#include <windows.h>
#include <TCHAR.h>
#include <crtdbg.h>
#include "pluginW1007.hpp"
#include "Plugin.h"

//#ifdef _UNICODE
//	#define TEXP(F) F##W
//#else
//	#define TEXP(F) F
//#endif

#define isPressed(inp) ((GetKeyState(inp) & 0x8000) == 0x8000)

PluginStartupInfo psi = {0};
FarStandardFunctions fsf = {0};

HANDLE  ghPipeThread = NULL;
DWORD   gnPipeTID = 0;
HANDLE  ghTerminateEvent = NULL;
HANDLE  ghSynchroEvent = NULL;
wchar_t gszPluginPipe[64] = {0};
SELECTORSTR *gpIn = NULL;
SELECTORSTR *gpOut = NULL;
DWORD   gnCallCommand = 0;
DWORD   gnPluginID = 'FrSl';
TCHAR   gsRootKey[MAX_PATH+32];
const TCHAR gsAllKey[] = _T("Software\\FAR2\\Plugins\\FarSelector");


#ifdef _UNICODE
	#define _tcsscanf swscanf
	#define SETMENUTEXT(itm,txt) itm.Text = txt;
	#define F757NA 0,
	#define _GetCheck(i) (int)psi.SendDlgMessage(hDlg,DM_GETCHECK,i,0)
	#define GetDataPtr(i) ((const TCHAR *)psi.SendDlgMessage(hDlg,DM_GETCONSTTEXTPTR,i,0))
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

#define NUM_ITEMS(X) (sizeof(X)/sizeof(X[0]))

void LoadSettings(BOOL* pbEnabled, BOOL* pbAutoselect, BOOL* pbCtrlPgDn);
void SaveSettings(BOOL bEnabled, BOOL bAutoselect, BOOL bCtrlPgDn);


void ProcessPipeCommand(HANDLE hPipe)
{
    DWORD cbRead = 0, cbWritten = 0, dwErr = 0;
    BOOL fSuccess = FALSE;
    //SELECTORSTR ss = {0};
    DWORD nHdrSize = 2*sizeof(DWORD);

	BOOL bEnabled, bAutoselect, bCtrlPgDn;
	LoadSettings(&bEnabled, &bAutoselect, &bCtrlPgDn);
	if (!bEnabled)
		return; // Плагин отключен

    // Send a message to the pipe server and read the response. 
    fSuccess = ReadFile( hPipe, gpIn, nHdrSize, &cbRead, NULL);
	dwErr = GetLastError();

	if (gpIn->nCmd == 1 || gpIn->nCmd == 2)
	{
		if (gpIn->nCmd == 2)
		{
			_ASSERTE(gpIn->nAllSize <= sizeof(SELECTORSTR));
			int nLen = gpIn->nAllSize - nHdrSize;
			fSuccess = ReadFile( hPipe, ((LPBYTE)gpIn)+nHdrSize, nLen, &cbRead, NULL);
			if (!fSuccess || cbRead != nLen)
			{
				_ASSERTE(fSuccess && cbRead);
				return;
			}
			if (gpIn->nDataLen < 2)
			{
				//_ASSERTE(gpIn->szData[0]!=0);
				_ASSERTE(gpIn->nDataLen >= 2);
				return;
			}
		}
	
		gnCallCommand = gpIn->nCmd;
		//gnCommandResult = 0;
		gpOut->nAllSize = 0;
		
		ResetEvent(ghSynchroEvent);

		//TODO: Для Far 1.7x - просить об активации ConEmu
		// Просим FAR активировать плагин в основной нити
		psi.AdvControl(psi.ModuleNumber,ACTL_SYNCHRO,NULL);

		// Теперь ждем (немножко
		if (WaitForSingleObject(ghSynchroEvent,1000) == WAIT_OBJECT_0)
		{
			gnCallCommand = 0;
			
			// OK, записать в пайп результат
			if (gpOut->nAllSize)
			{
				fSuccess = WriteFile(hPipe, gpOut, gpOut->nAllSize, &cbWritten, NULL);
			}
		}
		// Чтобы не было ошибочных активаций с таймаутом
		gnCallCommand = 0;
		//gnCommandResult = 0;
	}
}

DWORD WINAPI PipeThread(LPVOID apArg)
{ 
    BOOL fConnected = FALSE;
    DWORD dwErr = 0;
    HANDLE hPipe = NULL; 
	
	if (!ghTerminateEvent)
		ghTerminateEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
	if (!ghSynchroEvent)
		ghSynchroEvent = CreateEvent(NULL,TRUE,FALSE,NULL);

	DWORD dwPID = GetCurrentProcessId();
    wsprintf(gszPluginPipe, SZ_PLUGIN_PIPE, dwPID);

    // The main loop creates an instance of the named pipe and 
    // then waits for a client to connect to it. When the client 
    // connects, a thread is created to handle communications 
    // with that client, and the loop is repeated. 
    
    // Пока не затребовано завершение плагина
    do
    {
		fConnected = FALSE; // Новый пайп
        while (!fConnected)
        { 
            _ASSERTE(hPipe == NULL);

            hPipe = CreateNamedPipe( gszPluginPipe, PIPE_ACCESS_DUPLEX, 
				PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, PIPE_UNLIMITED_INSTANCES,
                PIPEBUFSIZE, PIPEBUFSIZE, 0, NULL/*gpNullSecurity*/);

            _ASSERTE(hPipe != INVALID_HANDLE_VALUE);

            if (hPipe == INVALID_HANDLE_VALUE) 
            {
                //DisplayLastError(L"CreateNamedPipe failed"); 
                hPipe = NULL;
                Sleep(50);
                continue;
            }

            // Wait for the client to connect; if it succeeds, 
            // the function returns a nonzero value. If the function
            // returns zero, GetLastError returns ERROR_PIPE_CONNECTED. 

            fConnected = ConnectNamedPipe(hPipe, NULL) ? TRUE : ((dwErr = GetLastError()) == ERROR_PIPE_CONNECTED); 

            // Затребован выход из плагина?
            if (WaitForSingleObject ( ghTerminateEvent, 0 ) == WAIT_OBJECT_0)
            {
                SafeCloseHandle(hPipe);
                goto wrap;
            }

            if (!fConnected)
                SafeCloseHandle(hPipe);
        }

        if (fConnected)
        {
        	ProcessPipeCommand(hPipe);
        	SafeCloseHandle(hPipe);
        }

		if (hPipe)
		{
			if (hPipe != INVALID_HANDLE_VALUE)
			{
				FlushFileBuffers(hPipe); 
				CloseHandle(hPipe);
			}
			hPipe = NULL;
		}
    } // Перейти к открытию нового instance пайпа
    while (WaitForSingleObject ( ghTerminateEvent, 0 ) != WAIT_OBJECT_0);

wrap:
    return 0; 
}

int WINAPI _export GetMinFarVersionW(void)
{
	return FARMANAGERVERSION;
}

void WINAPI _export SetStartupInfoW(const struct PluginStartupInfo *Info)
{
	fsf = *Info->FSF;
	psi = *Info;
	psi.FSF = &fsf;
	
	BOOL bExists = psi.Control(PANEL_ACTIVE, FCTL_CHECKPANELSEXIST, 0, 0);
	if (!bExists)
		return; // Панелей нет, сделать ничего нельзя

	lstrcpyn(gsRootKey, psi.RootKey, MAX_PATH);
	lstrcat(gsRootKey, _T("\\FarSelector"));
	
	if (!gpIn)
		gpIn = (SELECTORSTR*)calloc(sizeof(SELECTORSTR),1);
	if (!gpIn) {
		_ASSERTE(gpIn!=NULL);
		return;
	}
	if (!gpOut)
		gpOut = (SELECTORSTR*)calloc(sizeof(SELECTORSTR),1);
	if (!gpOut) {
		_ASSERTE(gpOut!=NULL);
		return;
	}
	
	_ASSERTE(ghPipeThread == NULL);
	if (ghPipeThread) {
		if (WaitForSingleObject(ghPipeThread,0) == WAIT_OBJECT_0)
			SafeCloseHandle(ghPipeThread); // нить завершилась?
	}
	if (ghPipeThread == NULL) {
		ghPipeThread = CreateThread(NULL, 0, PipeThread, NULL, 0, &gnPipeTID);
	}
}

void WINAPI _export GetPluginInfoW(struct PluginInfo *Info)
{
	Info->Flags = PF_PRELOAD|PF_DISABLEPANELS;
	Info->Reserved = gnPluginID;
	Info->PluginConfigStringsNumber = 1;
	static const wchar_t* PluginConfigStrings[1];
	PluginConfigStrings[0] = psi.GetMsg(psi.ModuleNumber, 0);
	Info->PluginConfigStrings = PluginConfigStrings;
}

// Возвращает (длину строки + '\0')
int GetPanelInfo(HANDLE hPanel, PanelInfo* ppi, wchar_t** psz)
{
	int nLen = 0;
	**psz = 0;
	if (ppi->PanelType == PTYPE_FILEPANEL) {
		wchar_t* pszTmp = *psz;
		if (hPanel == PANEL_ACTIVE) {
			lstrcpy(pszTmp, _T("(*) "));
			pszTmp += lstrlen(pszTmp);
		}
		if (!ppi->Plugin) {
			psi.Control(hPanel, FCTL_GETPANELDIR, 32767, (LONG_PTR)pszTmp);
		} else {
			// нужно получать информацию о плагине
			lstrcpy(pszTmp, _T("[Plugin\\"));
			pszTmp += lstrlen(pszTmp);
			psi.Control(hPanel, FCTL_GETPANELDIR, 32767, (LONG_PTR)pszTmp);
			pszTmp += lstrlen(pszTmp);
			*(pszTmp++) = _T(']'); *pszTmp = 0;
		}
	} else {
		lstrcpy(*psz, 
			(ppi->PanelType == PTYPE_TREEPANEL) ? _T("Tree") :
			(ppi->PanelType == PTYPE_QVIEWPANEL) ? _T("QView") :
			(ppi->PanelType == PTYPE_INFOPANEL) ? _T("Info") :
			_T("Unknown"));
	}
	nLen = lstrlen(*psz)+1;
	*psz += nLen;
	return nLen;
}

typedef BOOL (WINAPI* FIsConsoleActive)();
typedef HWND (WINAPI* FGetFarHWND2)(BOOL abConEmuOnly);
typedef int  (WINAPI* FActivateConsole)();

HANDLE WINAPI _export OpenPluginW(int OpenFrom,INT_PTR Item)
{
	if ((OpenFrom & OPEN_FROMMACRO) == OPEN_FROMMACRO)
	{
		if (Item == 101)
		{
			PanelInfo piActive = {0}, piPassive = {0};
			//TODO: Если активен НЕ модальный Viewer/Editor - таки разрешить?
			if (psi.Control(PANEL_ACTIVE, FCTL_GETPANELINFO, 0, (LONG_PTR)&piActive)
				&& psi.Control(PANEL_PASSIVE, FCTL_GETPANELINFO, 0, (LONG_PTR)&piPassive))
			{
				int nLen = 0;
				wchar_t* psz = gpOut->szData;
				gpOut->nDataLen = 0;

				HMODULE hConEmuDll = GetModuleHandle(_T("conemu.dll"));
				if (!hConEmuDll) hConEmuDll = GetModuleHandle(_T("conemu.x64.dll"));
				FIsConsoleActive IsConsoleActive = NULL;
				FGetFarHWND2 GetFarHWND2 = NULL;
				FActivateConsole ActivateConsole = NULL;
				if (hConEmuDll)
				{
					IsConsoleActive = (FIsConsoleActive)GetProcAddress(hConEmuDll, "IsConsoleActive");
					GetFarHWND2 = (FGetFarHWND2)GetProcAddress(hConEmuDll, "GetFarHWND2");
					ActivateConsole = (FActivateConsole)GetProcAddress(hConEmuDll, "ActivateConsole");
				}

				BOOL lbEnabled = TRUE;
				BOOL lbConEmu = FALSE;
				HWND hCon = GetConsoleWindow();
				wchar_t szWndClass[64];
				//TODO: В принципе, RealConsole может быть и видна, но подцеплена к ConEmu
				if (!IsWindowVisible(hCon))
				{
					lbEnabled = FALSE;
					if (GetFarHWND2)
					{
						// Получить главное окно ConEmu
						HWND hConEmuDC = GetFarHWND2(TRUE);
						if (hConEmuDC)
						{
							lbConEmu = TRUE;
							hCon = GetParent(hConEmuDC);
							gpOut->bActiveFar = IsConsoleActive();
							if (!gpOut->bActiveFar && ActivateConsole == NULL)
							{
								lbEnabled = FALSE; // активировать неактивную консоль не получится
							}
							else
							{
								lbEnabled = TRUE;
								lstrcpy(szWndClass, _T("VirtualConsoleClass"));
							}
						}
					}
				}
				
				if (lbEnabled)
				{
					gpOut->hWnd = (DWORD)hCon;
					if (!lbConEmu) lstrcpy(szWndClass, _T("ConsoleWindowClass"));

					// Теперь определить, является ли hCon верхним окном класса szWndClass
					HWND hFind = FindWindowEx(NULL, NULL, szWndClass, NULL);
					if (hFind == hCon)
					{
						if (!lbConEmu)
							gpOut->bActiveFar = TRUE;
					}
					else
					{
						gpOut->bActiveFar = FALSE;
					}

					if (piActive.Flags & PFLAGS_PANELLEFT)
					{
						nLen = GetPanelInfo(PANEL_ACTIVE, &piActive, &psz);
						gpOut->nDataLen += nLen;
						nLen = GetPanelInfo(PANEL_PASSIVE, &piPassive, &psz);
						gpOut->nDataLen += nLen;
					}
					else
					{
						nLen = GetPanelInfo(PANEL_PASSIVE, &piPassive, &psz);
						gpOut->nDataLen += nLen;
						nLen = GetPanelInfo(PANEL_ACTIVE, &piActive, &psz);
						gpOut->nDataLen += nLen;
					}
					//TODO: Заполнение параметра gpOut->bActiveFar
					gpOut->nAllSize = gpOut->nDataLen*sizeof(wchar_t) // длина строк+"\0"
						+ (DWORD)(((LPBYTE)&(gpOut->szData[0])) - ((LPBYTE)gpOut)); // + заголовок
				} // if (lbEnabled)
			}
		}
	
		SetEvent(ghSynchroEvent);
	}
	else
	{
		_ASSERTE((OpenFrom & OPEN_FROMMACRO) == OPEN_FROMMACRO);
	}
	
	return INVALID_HANDLE_VALUE;
}

void WriteDummyEvent()
{
	//FAR BUGBUG: Макрос не запускается на исполнение, пока мышкой не дернем :(
	//  Это чаще всего проявляется при вызове меню по RClick
	//  Если курсор на другой панели, то RClick сразу по пассивной
	//  не вызывает отрисовку :(
	//if (!mcr.Param.PlainText.Flags) {
	INPUT_RECORD ir[2] = {{MOUSE_EVENT},{MOUSE_EVENT}};
	if (isPressed(VK_CAPITAL))
		ir[0].Event.MouseEvent.dwControlKeyState |= CAPSLOCK_ON;
	if (isPressed(VK_NUMLOCK))
		ir[0].Event.MouseEvent.dwControlKeyState |= NUMLOCK_ON;
	if (isPressed(VK_SCROLL))
		ir[0].Event.MouseEvent.dwControlKeyState |= SCROLLLOCK_ON;
	ir[0].Event.MouseEvent.dwEventFlags = MOUSE_MOVED;
	ir[1].Event.MouseEvent.dwControlKeyState = ir[0].Event.MouseEvent.dwControlKeyState;
	ir[1].Event.MouseEvent.dwEventFlags = MOUSE_MOVED;
	ir[1].Event.MouseEvent.dwMousePosition.X = 1;
	ir[1].Event.MouseEvent.dwMousePosition.Y = 1;

	HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
	DWORD cbWritten = 0;
	#ifdef _DEBUG
	BOOL fSuccess = 
	#endif
	WriteConsoleInput(hIn, ir, 1, &cbWritten);
}

void _tcscpyesc(TCHAR* pszDst, const TCHAR* pszSrc)
{
	TCHAR c;
	while (*pszSrc)
	{
		c = *(pszSrc++);
		if (c == _T('"') || c == _T('\\'))
			*(pszDst++) = _T('\\');
		*(pszDst++) = c;
	}
	*pszDst = 0;
}

BOOL IsFile(wchar_t* pszFull, wchar_t* pszFolder/*[32768]*/, wchar_t* pszFile/*[MAX_PATH+1]*/)
{
	pszFile[0] = 0; pszFolder[0] = 0;
	if (*pszFull == 0) return FALSE;
	
	int nLen = lstrlen(pszFull);
	if (pszFull[nLen-1] == L'\\')
	{
		lstrcpyn(pszFolder, pszFull, 32768);
		return FALSE;
	}
		
	wchar_t* pszSlash = wcsrchr(pszFull, L'\\');
	if (!pszSlash)
	{
		lstrcpy(pszFile, pszFull);
		return TRUE;
	}
	
	DWORD dwAttr = GetFileAttributes(pszFull);
	if ((dwAttr != (DWORD)-1) && ((dwAttr & FILE_ATTRIBUTE_DIRECTORY) == 0))
	{
		// Файл
		lstrcpyn(pszFile, pszSlash+1, MAX_PATH+1);
		size_t nDirLen = pszSlash-pszFull+1;
		if (nDirLen >= 32768) nDirLen = 32767;
		wmemcpy(pszFolder, pszFull, nDirLen);
		pszFolder[nDirLen] = 0;
		return TRUE;
	}
	
	lstrcpyn(pszFolder, pszFull, 32768);
	return FALSE;
}

bool IsPluginPrefixPath(const wchar_t *Path) //Max:
{
	if (Path[0] == L'\\')
		return false;
	const wchar_t* pC = wcschr(Path, L':');
	if (!pC)
		return false;
	if ((pC - Path) == 1)
	{
		wchar_t d = (Path[0] & ~0x20);
		if (d >= L'C' && d <= 'Z')
			return false; // буква диска, а не префикс
	}
	const wchar_t* pS = wcschr(Path, L'\\');
	if (pS && pS < pC)
		return false;
	return true;
}

void AddMacro(wchar_t* &pszTmp, wchar_t* pszFull, wchar_t* szDir, wchar_t* szFile, BOOL abActive, BOOL abCtrlPgDn)
{
	BOOL bPrefix = IsPluginPrefixPath(pszFull);
	
	if (bPrefix)
	{
		lstrcpy(pszTmp, L"$If (CmdLine.ItemCount>0) Esc $End "); pszTmp += lstrlen(pszTmp);
		if (!abActive)
		{
			lstrcpy(pszTmp, L"Tab "); pszTmp += 4;
		}
		lstrcpy(pszTmp, _T("print(\"")); pszTmp += lstrlen(pszTmp);
		_tcscpyesc(pszTmp, pszFull); pszTmp += lstrlen(pszTmp);
		lstrcpy(pszTmp, _T("\") ")); pszTmp += lstrlen(pszTmp);
		lstrcpy(pszTmp, _T("Enter ")); pszTmp += lstrlen(pszTmp);

	} else {
		BOOL bFile = IsFile(pszFull, szDir, szFile);

		if (bFile)
		{
			_ASSERTE(szDir[0]!=0);
			if (szDir[0] != 0)
			{
				wsprintf(pszTmp, _T("panel.SetPath(%i,\""), abActive?0:1); pszTmp += lstrlen(pszTmp);
				_tcscpyesc(pszTmp, szDir); pszTmp += lstrlen(pszTmp);
				lstrcpy(pszTmp, _T("\",\"")); pszTmp += lstrlen(pszTmp);
				_tcscpyesc(pszTmp, szFile); pszTmp += lstrlen(pszTmp);
				lstrcpy(pszTmp, _T("\") ")); pszTmp += lstrlen(pszTmp);
				if (abCtrlPgDn)
				{
					lstrcpy(pszTmp, _T("CtrlPgDn ")); pszTmp += lstrlen(pszTmp);
				}
			}
		}
		else
		{
			wsprintf(pszTmp, _T("panel.SetPath(%i,\""), abActive?0:1); pszTmp += lstrlen(pszTmp);
			_tcscpyesc(pszTmp, szDir); pszTmp += lstrlen(pszTmp);
			lstrcpy(pszTmp, _T("\") ")); pszTmp += lstrlen(pszTmp);
		}
	}
}

int WINAPI _export ProcessSynchroEventW(int Event,void *Param)
{
	if (Event != SE_COMMONSYNCHRO)
		return 0;
		
	switch (gnCallCommand)
	{
	case 1:
		{
			// Получить информацию (через макро, чтобы удостовериться в текущей области)
			ActlKeyMacro mcr;
			mcr.Command = MCMD_POSTMACROSTRING;
			mcr.Param.PlainText.Flags = 0; // По умолчанию - вывод на экран разрешен (KSFLAGS_DISABLEOUTPUT)
			TCHAR szMacro[128];
			wsprintf(szMacro, _T("$IF (Shell) callplugin(0x%08X,101) $Else callplugin(0x%08X,100) $End"), gnPluginID, gnPluginID);
			mcr.Param.PlainText.SequenceText = szMacro;
			psi.AdvControl(psi.ModuleNumber, ACTL_KEYMACRO, (void*)&mcr);
			
			//FAR BUGBUG
			WriteDummyEvent();
		}
		break;
	case 2:
		{
			// Выполнить команду (смена каталогов на панелях, переход к файлу

			BOOL bEnabled, bAutoselect, bCtrlPgDn;
			LoadSettings(&bEnabled, &bAutoselect, &bCtrlPgDn);
			if (!bEnabled)
			{
				gpOut->nAllSize = 2*sizeof(DWORD);
				gpOut->nCmd = 0;
				SetEvent(ghSynchroEvent);
				return 0;
			}

			// Т.к. переход в папку или откртие архива может занять длительное время - ставим event сразу!
			gpOut->nAllSize = 2*sizeof(DWORD);
			gpOut->nCmd = 2;
			SetEvent(ghSynchroEvent);
			
			// Активировать консоль в ConEmu
			HMODULE hConEmuDll = GetModuleHandle(_T("conemu.dll"));
			if (!hConEmuDll) hConEmuDll = GetModuleHandle(_T("conemu.x64.dll"));
			FIsConsoleActive IsConsoleActive = NULL;
			FActivateConsole ActivateConsole = NULL;
			if (hConEmuDll)
			{
				IsConsoleActive = (FIsConsoleActive)GetProcAddress(hConEmuDll, "IsConsoleActive");
				ActivateConsole = (FActivateConsole)GetProcAddress(hConEmuDll, "ActivateConsole");
				if (IsConsoleActive && ActivateConsole)
				{
					if (!IsConsoleActive())
						ActivateConsole();
				}
			}
			
			// Выполнить макру
			ActlKeyMacro mcr;
			mcr.Command = MCMD_POSTMACROSTRING;
			mcr.Param.PlainText.Flags = 0; // По умолчанию - вывод на экран разрешен (KSFLAGS_DISABLEOUTPUT)
			TCHAR* pszMacro = NULL;
			wchar_t* pszActive = gpIn->szData;
			wchar_t* pszPassive = gpIn->szData+wcslen(gpIn->szData)+1;
			if (*pszActive || *pszPassive)
			{
				// готовим память (экранирование " и  \)
				int nAllLen = (lstrlen(pszActive)*2 + lstrlen(pszPassive)*2 + 512);
				pszMacro = (TCHAR*)calloc(nAllLen,sizeof(TCHAR));
				wchar_t szDir[32768], szFile[MAX_PATH+1];
				//BOOL bFile;
				//TODO: обработка /e[x:x] & /v
				// создаем макру
				// panel.SetPath(0,"C:\\Program Files","FAR") -- _tcscpyesc
				// CtrlPgDn
				// panel.SetPath(1,"C:\\Program Files","FAR") -- _tcscpyesc
				// CtrlPgDn
				lstrcpy(pszMacro, _T("$If (Shell) "));
				wchar_t* pszTmp = pszMacro+lstrlen(pszMacro);
				
				// Active panel
				if (*pszActive)
					AddMacro(pszTmp, pszActive, szDir, szFile, TRUE, bCtrlPgDn);
				// Passive panel
				if (*pszPassive)
					AddMacro(pszTmp, pszPassive, szDir, szFile, FALSE, FALSE);
				
				//wsprintf(szMacro, _T("$IF (Shell) callplugin(0x%08X,102) $Else callplugin(0x%08X,200) $End"), gnPluginID, gnPluginID);
				lstrcat(pszTmp, _T(" $End"));
				mcr.Param.PlainText.SequenceText = pszMacro;
				psi.AdvControl(psi.ModuleNumber, ACTL_KEYMACRO, (void*)&mcr);
				
				//FAR BUGBUG. Иначе макрос не выполнится до первого движения мышкой, или еще какого консольного события
				WriteDummyEvent();
				// Fin
				free(pszMacro); pszMacro = NULL;
			}
		}
		break;
	}
	
	//SetEvent(ghSynchroEvent);

	return 0;
}

void WINAPI _export ExitFARW(void)
{
	if (ghPipeThread)
	{
		SetEvent(ghTerminateEvent);
		HANDLE hPipe = CreateFile(gszPluginPipe,GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);
		if (WaitForSingleObject(ghPipeThread, 200)!=WAIT_OBJECT_0)
		{
			TerminateThread(ghPipeThread,1);
		}
		SafeCloseHandle(hPipe);
		SafeCloseHandle(ghPipeThread);
		SafeCloseHandle(ghTerminateEvent);
		SafeCloseHandle(ghSynchroEvent);
	}
	if (gpIn)
	{
		free(gpIn); gpIn = NULL;
	}
	if (gpOut)
	{
		free(gpOut); gpOut = NULL;
	}
}

void LoadSettings(BOOL* pbEnabled, BOOL* pbAutoselect, BOOL* pbCtrlPgDn)
{
	*pbEnabled = TRUE;
	*pbAutoselect = FALSE;
	*pbCtrlPgDn = TRUE;

	HKEY hk = 0;
	BYTE bVal; DWORD nValLen;
	if (0 == RegOpenKeyEx(HKEY_CURRENT_USER, gsAllKey, 0, KEY_READ, &hk))
	{
		if (!RegQueryValueEx(hk, _T("Enabled"), 0, 0, &bVal, &(nValLen=1)))
			*pbEnabled = (bVal!=0);
		if (!RegQueryValueEx(hk, _T("Autoselect"), 0, 0, &bVal, &(nValLen=1)))
			*pbAutoselect = (bVal!=0);
		if (!RegQueryValueEx(hk, _T("CtrlPgDn"), 0, 0, &bVal, &(nValLen=1)))
			*pbCtrlPgDn = (bVal!=0);
		RegCloseKey(hk);
	}

	// Если фар запущен под другим пользователем, или это все-таки FAR1 - ветка другая
	if (lstrcmpi(gsAllKey, gsRootKey))
	{
		if (0 == RegOpenKeyEx(HKEY_CURRENT_USER, gsAllKey, 0, KEY_READ, &hk))
		{
			// Autoselect - только в основной ветке
			if (!RegQueryValueEx(hk, _T("Enabled"), 0, 0, &bVal, &(nValLen=1)))
				*pbEnabled = (bVal!=0);
			if (!RegQueryValueEx(hk, _T("CtrlPgDn"), 0, 0, &bVal, &(nValLen=1)))
				*pbCtrlPgDn = (bVal!=0);
			RegCloseKey(hk);
		}
	}
}

void SaveSettings(BOOL bEnabled, BOOL bAutoselect, BOOL bCtrlPgDn)
{
	HKEY hk = 0;
	BYTE bVal;
	BOOL bOneKey = lstrcmpi(gsAllKey, gsRootKey)==0;

	if (0 == RegCreateKeyEx(HKEY_CURRENT_USER, gsAllKey, 0, 0, 0, KEY_ALL_ACCESS, 0, &hk, 0))
	{
		RegSetValueEx(hk, _T("Autoselect"), 0, REG_BINARY, &(bVal=bAutoselect), 1);

		if (bOneKey)
		{
			RegSetValueEx(hk, _T("Enabled"), 0, REG_BINARY, &(bVal=bEnabled), 1);
			RegSetValueEx(hk, _T("CtrlPgDn"), 0, REG_BINARY, &(bVal=bCtrlPgDn), 1);
		}

		RegCloseKey(hk);
	}

	// Если фар запущен под другим пользователем, или это все-таки FAR1 - ветка другая
	if (!bOneKey)
	{
		if (0 == RegCreateKeyEx(HKEY_CURRENT_USER, gsAllKey, 0, 0, 0, KEY_ALL_ACCESS, 0, &hk, 0))
		{
			// Autoselect - только в основной ветке
			RegSetValueEx(hk, _T("Enabled"), 0, REG_BINARY, &(bVal=bEnabled), 1);
			RegSetValueEx(hk, _T("CtrlPgDn"), 0, REG_BINARY, &(bVal=bCtrlPgDn), 1);
			RegCloseKey(hk);
		}
	}
}

int WINAPI _export ConfigureW(int ItemNumber)
{
	if (!ghPipeThread)
		return FALSE;

	int height = 11;

	FarDialogItem items[] =
	{
		// Common options
		{DI_DOUBLEBOX,  3,  1,  30, height - 2, false,  0,              0,                  0,},        //"FAR Selector"

		{DI_CHECKBOX,   5,  3,  0,  0,          true,   false,          0,                  false,},    //"&Enabled"

		{DI_CHECKBOX,   5,  5,  0,  0,          true,   false,          0,                  false,},    //"&Autoselect"
		{DI_CHECKBOX,   5,  6,  0,  0,          true,   false,          0,                  false,},    //"Ctrl&PgDn for files"

		{DI_BUTTON,     0,  8, 0,  0,          true,   true,           DIF_CENTERGROUP,    true,},     //"&OK"
		{DI_BUTTON,     0,  8, 0,  0,          true,   false,          DIF_CENTERGROUP,    false,},    //"Cancel"
	};

	for (int i=0; i<NUM_ITEMS(items); i++)
	{
		SETTEXT(items[i], psi.GetMsg(psi.ModuleNumber,i));
	}

	BOOL bEnabled = FALSE, bAutoselect = FALSE, bCtrlPgDn = FALSE;
	LoadSettings(&bEnabled, &bAutoselect, &bCtrlPgDn);

	items[1].Selected = bEnabled;
	items[2].Selected = bAutoselect;
	items[3].Selected = bCtrlPgDn;


	int dialog_res = 0;

#ifdef _UNICODE
	HANDLE hDlg = psi.DialogInit ( psi.ModuleNumber, -1, -1, 34, height,
		_T("Configure"), items, NUM_ITEMS(items), 0, 0/*Flags*/, NULL/*MVConfigDlgProc*/, 0/*DlgProcParam*/ );
#endif


#ifndef _UNICODE
	//dialog_res = psi.Dialog(psi.ModuleNumber, -1, -1, 55, height, _T("Configure"), items, NUM_ITEMS(items));
	dialog_res = psi.DialogEx(psi.ModuleNumber,-1,-1,55,height, "Configure", items, NUM_ITEMS(items), 0, 0,
		MVConfigDlgProc, NULL);
#else
	dialog_res = psi.DialogRun ( hDlg );
#endif

	if (dialog_res != -1 && dialog_res != (NUM_ITEMS(items)-1))
	{
		SaveSettings(_GetCheck(1), _GetCheck(2), _GetCheck(3));
	}
#ifdef _UNICODE
	psi.DialogFree ( hDlg );
#endif

	return(true);
}
