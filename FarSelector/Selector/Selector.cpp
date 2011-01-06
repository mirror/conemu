
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


// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <commctrl.h>
#include <Tlhelp32.h>
#include <crtdbg.h>

#include "Selector.h"
#include "../Plugin/Plugin.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
FARINSTANCE *gFars = NULL;
int gnFarsCount = 0, gnFarsMaxCount = 0, gnDefaultFarIdx = -1;
LPTSTR gpszCmdLine = NULL;
HICON ghIcon = NULL;

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	
	gpszCmdLine = lpCmdLine;

	// Perform application initialization:
	FindInstances ();
	//if (!FindInstances ())
	//{
	//	//TODO: Start new far instance
	//	MessageBox(0, lpCmdLine, _T("FarSelector"), MB_OK|MB_SETFOREGROUND|MB_SYSTEMMODAL);
	//	return FALSE;
	//}

	// Если открытых (активных) фаров нет - сразу запустить новый
	if (!gnFarsCount) {
		StartNewInstance(NULL);
		return 0;
	}

	BOOL lbAutoSelect = FALSE;
	HKEY hk = NULL;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\FAR2\\Plugins\\FarSelector", 0, KEY_READ, &hk) == 0)
	{
		BYTE  nData = 0;
		DWORD dwSize = 1;
		if (RegQueryValueEx(hk, L"Autoselect", NULL, NULL, &nData, &dwSize) == 0)
		{
			lbAutoSelect = nData!=0;
		}
		RegCloseKey(hk);
	}
	if (lbAutoSelect)
	{
		if (CallInstance((gnDefaultFarIdx == -1) ? 0 : gnDefaultFarIdx))
		{
			return 0; // Выполнили
		}
	}
#ifdef _DEBUG
	else
	{
		//MessageBox(0, _T("Autoselect is off"), _T("FarSelector"), MB_OK|MB_SETFOREGROUND|MB_SYSTEMMODAL);
	}
#endif


	INITCOMMONCONTROLSEX iic = {sizeof(INITCOMMONCONTROLSEX)};
	iic.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&iic);
	
	ghIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SELECTOR));
	
	if (-1 == DialogBox(hInst, MAKEINTRESOURCE(IDD_FARSELECT), NULL, SelectionProc)) {
		MessageBox(0, _T("DialogBox(IDD_FARSELECT) failed!"), _T("FarSelector"), MB_OK|MB_SETFOREGROUND|MB_SYSTEMMODAL);
	}

	return 0;
}



//DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);

// Message handler for about box.
//INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
//{
//	UNREFERENCED_PARAMETER(lParam);
//	switch (message)
//	{
//	case WM_INITDIALOG:
//		return (INT_PTR)TRUE;
//
//	case WM_COMMAND:
//		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
//		{
//			EndDialog(hDlg, LOWORD(wParam));
//			return (INT_PTR)TRUE;
//		}
//		break;
//	}
//	return (INT_PTR)FALSE;
//}

void AddProcess(DWORD dwPID)
{
	if (!gnFarsMaxCount || !gFars) {
		gnFarsMaxCount = 100; gnFarsCount = 0;
		gFars = (FARINSTANCE*)calloc(gnFarsMaxCount, sizeof(FARINSTANCE));
		if (!gFars)
			return;
	}
	
	if (gnFarsCount == gnFarsMaxCount) {
		gnFarsMaxCount += 100;
		FARINSTANCE* pNewFars = (FARINSTANCE*)calloc(gnFarsMaxCount, sizeof(FARINSTANCE));
		if (!pNewFars)
			return;
		memmove(pNewFars, gFars, gnFarsCount*sizeof(FARINSTANCE));
	}
	
	// Получить информацию о dwPID (через пайп)
	wchar_t szPipe[64];
	SELECTORSTR ssOut = {0};
	wsprintf(szPipe, SZ_PLUGIN_PIPE, dwPID);
	SELECTORSTR ssIn;
	ssIn.nAllSize = 2*sizeof(DWORD);
	ssIn.nCmd = 1;
	DWORD dwOutSize = sizeof(ssOut);
	if (!CallNamedPipe(szPipe, &ssIn, ssIn.nAllSize, &ssOut, dwOutSize, &dwOutSize, 500))
		return; // Недоступен
	if (!dwOutSize)
		return; // Ошибка, недоступен
	//dwOutSize = *((DWORD*)szData);
	//if (!dwOutSize)
	//	return; // Ошибка, недоступен
	wchar_t* psz = ssOut.szData;
	
	// И если FAR может принять команду - добавить его в список
	// Не могут - запущенные с ключами "/e" "/v", или неактивные (в консоли выполняется команда)
	gFars[gnFarsCount].dwPID = dwPID;
	gFars[gnFarsCount].hWnd = (HWND)ssOut.hWnd;
	gFars[gnFarsCount].bActiveFar = ssOut.bActiveFar;
	gFars[gnFarsCount].pszLeft = _wcsdup(psz);
	psz += lstrlen(psz)+1;
	gFars[gnFarsCount].pszRight = _wcsdup(psz);

	if (gnDefaultFarIdx == -1 && ssOut.bActiveFar) {
		gnDefaultFarIdx = gnFarsCount;
	}

	gnFarsCount++;
}

BOOL FindInstances()
{
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnap != INVALID_HANDLE_VALUE)
	{
		PROCESSENTRY32 proc = {sizeof(PROCESSENTRY32)};
		if (Process32First(hSnap, &proc)) {
			do {
				if (lstrcmpi(proc.szExeFile, L"far.exe") == 0) {
					AddProcess(proc.th32ProcessID);
				}
			} while (Process32Next(hSnap, &proc));
		}
		CloseHandle(hSnap);
	}
	return (gnFarsCount>0);
}

BOOL FileExists(LPCWSTR asFile)
{
	DWORD dwAttr = GetFileAttributes(asFile);
	if (dwAttr == (DWORD)-1)
		return FALSE;
	if ((dwAttr & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
		return FALSE;
	return TRUE; // OK, это файл
}


typedef BOOL (WINAPI* IsWow64Process_t)(HANDLE hProcess, PBOOL Wow64Process);

// pbIsWow64Process <- TRUE, если 32битный процесс запущен в 64битной ОС
BOOL IsWindows64(BOOL *pbIsWow64Process = NULL)
{
	BOOL is64bitOs = FALSE, isWow64process = FALSE;
	#ifdef WIN64
	is64bitOs = TRUE; isWow64process = FALSE;
	#else
	// Проверяем, где мы запущены
	isWow64process = FALSE;
	HMODULE hKernel = GetModuleHandleW(L"kernel32.dll");
	if (hKernel)
	{
		IsWow64Process_t IsWow64Process_f = (IsWow64Process_t)GetProcAddress(hKernel, "IsWow64Process");
		if (IsWow64Process_f)
		{
			BOOL bWow64 = FALSE;
			if (IsWow64Process_f(GetCurrentProcess(), &bWow64) && bWow64)
			{
				isWow64process = TRUE;
			}
		}
	}
	is64bitOs = isWow64process;
	#endif
	if (pbIsWow64Process)
		*pbIsWow64Process = isWow64process;
	return is64bitOs;
}


INT_PTR CALLBACK SelectionProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		{
			// Иконка для TaskBar & AltTab
			SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)ghIcon);
			
			// Центрирование на мониторе, в котором мышка
			RECT rect;
			GetWindowRect(hDlg, &rect);
			POINT ptCur; GetCursorPos(&ptCur);

			BOOL lbCentered = FALSE;
			HMONITOR hMon = MonitorFromPoint(ptCur, MONITOR_DEFAULTTOPRIMARY);
			if (hMon) {
				MONITORINFO mi; mi.cbSize = sizeof(mi);
				if (GetMonitorInfo(hMon, &mi)) {
					lbCentered = TRUE;
					MoveWindow(hDlg, 
						(mi.rcWork.left+mi.rcWork.right-rect.right+rect.left)/2,
						(mi.rcWork.top+mi.rcWork.bottom-rect.bottom+rect.top)/2,
						rect.right - rect.left, rect.bottom - rect.top, false);
				}
			}

			if (!lbCentered)
				MoveWindow(hDlg, GetSystemMetrics(SM_CXSCREEN)/2 - (rect.right - rect.left)/2, GetSystemMetrics(SM_CYSCREEN)/2 - (rect.bottom - rect.top)/2, rect.right - rect.left, rect.bottom - rect.top, false);
		
			wchar_t szPID[16];
			HWND hList = GetDlgItem(hDlg, IDC_FARLIST);
			
			ListView_SetExtendedListViewStyleEx(hList, LVS_EX_FULLROWSELECT|LVS_EX_LABELTIP, LVS_EX_FULLROWSELECT|LVS_EX_LABELTIP);
			
			LVCOLUMN lvc = {LVCF_WIDTH|LVCF_TEXT};
			lvc.cx = 50; lvc.pszText = _T("PID");
			ListView_InsertColumn(hList, 0, &lvc);
			lvc.mask |= LVCF_SUBITEM;
			lvc.cx = 225; lvc.pszText = _T("Left"); lvc.iSubItem = 1;
			ListView_InsertColumn(hList, 1, &lvc);
			lvc.cx = 225; lvc.pszText = _T("Right"); lvc.iSubItem = 2;
			ListView_InsertColumn(hList, 2, &lvc);
			
			LVITEM lvi = {LVIF_TEXT|LVIF_PARAM};
			lvi.pszText = szPID;

			//LVSETINFOTIP lst = {sizeof(LVSETINFOTIP)};
			int iSetCurSel = (gnDefaultFarIdx!=-1) ? gnDefaultFarIdx : 0;
			
			for (int i=0; i<gnFarsCount; i++) {
				wsprintf(szPID, _T("%u"), gFars[i].dwPID);
				lvi.iItem = i; lvi.lParam = i;
				ListView_InsertItem(hList, &lvi);
				if (gFars[i].pszLeft)
					ListView_SetItemText(hList, i, 1, gFars[i].pszLeft);
				if (gFars[i].pszRight)
					ListView_SetItemText(hList, i, 2, gFars[i].pszRight);
			}
			
			lvi.pszText = _T("New"); lvi.lParam = -1;
			lvi.iItem = gnFarsCount; // последним - добавим "New far instance" :)
			ListView_InsertItem(hList, &lvi);

			ListView_SetItemState(hList, iSetCurSel, LVIS_FOCUSED|LVIS_SELECTED, LVIS_FOCUSED|LVIS_SELECTED);
			
			if (gpszCmdLine)
				SetDlgItemText(hDlg, IDC_NEWCMDLINE, gpszCmdLine);
				
			SetFocus(hList);
		}
		return (INT_PTR)FALSE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			// Processing IDC_AUTOSELECT
			if (SendDlgItemMessage(hDlg, IDC_AUTOSELECT, BM_GETCHECK, 0, 0) == BST_CHECKED)
			{
				HKEY hk = NULL;
				DWORD dwDisp = 0;
				if (RegCreateKeyEx(HKEY_CURRENT_USER, L"Software\\FAR2\\Plugins\\FarSelector", 0, 0, 0, KEY_ALL_ACCESS,
						NULL, &hk, &dwDisp) == 0)
				{
					BYTE  nData = 1; DWORD dwSize = 1;
					RegSetValueEx(hk, L"Autoselect", 0, REG_BINARY, &nData, dwSize);
					RegCloseKey(hk);
				}
			}

			HWND hList = GetDlgItem(hDlg, IDC_FARLIST);
			int iSel = ListView_GetNextItem(hList, -1, LVNI_SELECTED);
			if (iSel >= 0)
			{
				// Учтем, что в будущем может появиться сортировка,
				// так что получим реальный индекс в gFars
				LVITEM lvi = {LVIF_PARAM};
				lvi.iItem = iSel;
				if (!ListView_GetItem(hList, &lvi))
					iSel = -1;
				else
					iSel = (int)lvi.lParam;
			
				// Если выбрали существующий ФАР
				if (gnFarsCount && iSel < gnFarsCount)
				{
					if (!CallInstance(iSel))
						iSel = -1; // Если не удалось передать в существующий - откроем новый
				}
				
				if (iSel == -1)
				{
					// Запуск НОВОГО фара
					StartNewInstance(hDlg);
				}
			}
			else
			{
				_ASSERTE(iSel>=0);
			}
		}

		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	case WM_NOTIFY:
		{
			LPNMITEMACTIVATE lp = (LPNMITEMACTIVATE)lParam;
			if (lp->hdr.code == NM_DBLCLK)
				PostMessage(hDlg, WM_COMMAND, IDOK, 0);
		}
		break;
	}
	return (INT_PTR)FALSE;
}

bool IsPluginPrefixPath(const wchar_t *Path) //Max:
{
	if (Path[0] == L'\\')
		return false;
	const wchar_t* pC = wcschr(Path, L':');
	if (!pC)
		return false;
	if ((pC - Path) == 1) {
		wchar_t d = (Path[0] & ~0x20);
		if (d >= L'C' && d <= 'Z')
			return false; // буква диска, а не префикс
	}
	const wchar_t* pS = wcschr(Path, L'\\');
	if (pS && pS < pC)
		return false;
	return true;
}

int NextArg(wchar_t** asCmdLine, wchar_t** rsArg/*[32768]*/)
{
    LPWSTR psCmdLine = *asCmdLine, pch = NULL;
    wchar_t ch = *psCmdLine;
    size_t nArgLen = 0;
    
    while (ch == L' ' || ch == L'\t' || ch == L'\r' || ch == L'\n') ch = *(++psCmdLine);
    if (ch == 0) return -1;

    // аргумент начинается с "
    if (ch == L'"') {
        psCmdLine++;
        pch = wcschr(psCmdLine, L'"');
        if (!pch) return -1;
        while (pch[1] == L'"') {
            pch += 2;
            pch = wcschr(pch, L'"');
            if (!pch) return -1;
        }
        // Теперь в pch ссылка на последнюю "
    } else {
        // До конца строки или до первого пробела
        //pch = wcschr(psCmdLine, L' ');
        // 09.06.2009 Maks - обломался на: cmd /c" echo Y "
        pch = psCmdLine;
        while (*pch && *pch!=L' ' && *pch!=L'"') pch++;
        //if (!pch) pch = psCmdLine + lstrlenW(psCmdLine); // до конца строки
    }
    
    nArgLen = pch - psCmdLine;
    if (nArgLen > 32767) return -1;

    // Вернуть аргумент
    //memcpy(*rsArg, psCmdLine, nArgLen*sizeof(wchar_t));
    //(*rsArg)[nArgLen] = 0;
    //(*rsArg) = (*rsArg)+nArgLen+1;
    ch = psCmdLine[nArgLen]; psCmdLine[nArgLen] = 0;
    
    // Учтем, что могут передать только имя файла (без полного пути)
	if (IsPluginPrefixPath(psCmdLine)) {
		lstrcpy((*rsArg), psCmdLine);
	} else {
		wchar_t* psFilePart;
		if (!GetFullPathName(psCmdLine, 32768, *rsArg, &psFilePart))
    		return -1; // не смогла...
	}
    (*rsArg) = (*rsArg)+lstrlen(*rsArg)+1;

    psCmdLine[nArgLen] = ch;
    psCmdLine = pch;
    
    // Finalize
    ch = *psCmdLine; // может указывать на закрывающую кавычку
    if (ch == L'"') ch = *(++psCmdLine);
    while (ch == L' ' || ch == L'\t' || ch == L'\r' || ch == L'\n') ch = *(++psCmdLine);
    *asCmdLine = psCmdLine;
    
    return 0;
}

void StartNewInstance(HWND hParent)
{
	// Запуск НОВОГО фара
	int nMaxLen = lstrlen(gpszCmdLine)+MAX_PATH+32;
	wchar_t *pszExe = (wchar_t*)malloc(nMaxLen*2);
	pszExe[0] = L'"';
	GetModuleFileName(NULL, pszExe+1, MAX_PATH);
	wchar_t* pszSl = wcsrchr(pszExe, L'\\'); if (pszSl) pszSl++; else pszSl = pszExe+1;
	// Если в каталоге с фаром лежит ConEmu - работать через него
	bool lbConEmu = false;
	if (IsWindows64())
	{
		if (!lbConEmu)
		{
			lstrcpy(pszSl, L"ConEmu\\ConEmu64.exe");
			if (FileExists(pszExe+1))
				lbConEmu = true;
		}
		if (!lbConEmu)
		{
			lstrcpy(pszSl, L"ConEmu64.exe");
			if (FileExists(pszExe+1))
				lbConEmu = true;
		}
	}
	if (!lbConEmu)
	{
		lstrcpy(pszSl, L"ConEmu\\ConEmu.exe");
		if (FileExists(pszExe+1))
			lbConEmu = true;
	}
	if (!lbConEmu)
	{
		lstrcpy(pszSl, L"ConEmu.exe");
		if (FileExists(pszExe+1))
			lbConEmu = true;
	}

	if (lbConEmu)
	{
		lstrcat(pszSl, L"\" /single /cmd ");
		int nLen = lstrlen(pszSl);
		int nLen2 = pszSl-pszExe;
		lstrcpyn(pszSl+nLen, pszExe, nLen2+1);
		pszSl += nLen+nLen2;
	}
	lstrcpy(pszSl, L"far.exe\" ");

	if (gpszCmdLine && *gpszCmdLine)
		lstrcat(pszExe, gpszCmdLine);

	STARTUPINFO si = {sizeof(si)};
	PROCESS_INFORMATION pi = {0};
	BOOL lb = CreateProcess(NULL, pszExe, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS,
		NULL, NULL, &si, &pi);
	if (lb) {
		CloseHandle(pi.hProcess); CloseHandle(pi.hThread);
	} else {
		DWORD dwErr = GetLastError();
		wchar_t *pszErrInfo = (wchar_t*)malloc((nMaxLen+128)*2);
		wsprintf(pszErrInfo, L"Can't create process! ErrCode=0x%08X\n", dwErr); lstrcat(pszErrInfo, pszExe);
		MessageBox(hParent, pszErrInfo, _T("FarSelector"), MB_OK|MB_ICONSTOP);
		free(pszErrInfo);
	}
	free(pszExe);
}

BOOL CallInstance(int iSel)
{
	HWND hNewWnd = (HWND)gFars[iSel].hWnd;
	wchar_t szPipe[64], szData[512]; szData[511] = 0;
	wsprintf(szPipe, SZ_PLUGIN_PIPE, gFars[iSel].dwPID);
	BOOL lbRc = FALSE;
	SELECTORSTR ss, ssOut;
	ss.nCmd = 2;
	ss.nAllSize = (DWORD)(((LPBYTE)&(ss.szData)) - ((LPBYTE)&ss));
	DWORD dwOutSize = 0;
	//
	LPTSTR pszCmd = gpszCmdLine, pszNext = NULL;
	wchar_t* pszData = ss.szData;
	*pszData = 0;
	if (NextArg(&pszCmd, &pszData) == 0)
	{
		if (NextArg(&pszCmd, &pszData) != 0)
		{
			// Один аргумент допустим, только если первый НЕ "/e..." и не "/v"
			if (ss.szData[0] == L'/')
			{
				#ifdef _DEBUG
				MessageBox(0, _T("Invalid arguments"), _T("FarSelector"), MB_OK|MB_SETFOREGROUND|MB_SYSTEMMODAL);
				#endif
				
				lbRc = FALSE;
				goto fail;
			}
		}
		*(pszData++) = 0; *(pszData++) = 0; // чтобы ASCIIZZ точно был
	}
	if (pszData == ss.szData)
	{
		*(pszData++) = 0; *(pszData++) = 0; // чтобы ASCIIZZ точно был
	}

	if (pszData > ss.szData)
	{
		ss.nDataLen = pszData - ss.szData;
		ss.nAllSize += ss.nDataLen*sizeof(wchar_t);

		if (hNewWnd)
		{
			//TODO: Активация вкладки ConEmu!!!

			if (!IsWindowVisible(hNewWnd))
			{
				// Свернутое в трей ?
				ShowWindow(hNewWnd, SW_SHOWNORMAL);
			}
			SetForegroundWindow(hNewWnd);
		}

		BOOL lbWaitPipe = WaitNamedPipe(szPipe, 2000);
		DWORD dwWaitErr = 0;
		if (!lbWaitPipe)
		{
			dwWaitErr = GetLastError();
			//#ifdef _DEBUG
			//wchar_t szDbg[512];
			//wsprintf(szDbg, L"WaitNamedPipe('%s', 2000) failed,\nErrCode=0x%08X\n", szPipe, dwErr);
			//MessageBox(0, szDbg, _T("FarSelector"), MB_OK|MB_SETFOREGROUND|MB_SYSTEMMODAL);
			//#endif
		}

		if (CallNamedPipe(szPipe, &ss, ss.nAllSize, &ssOut, sizeof(ssOut), &dwOutSize, 500))
		{
			lbRc = TRUE;
		}
#ifdef _DEBUG
		else
		{
			wchar_t szDbg[1024]; szDbg[0] = 0;
			DWORD dwErr = GetLastError();
			wsprintf(szDbg, L"WaitNamedPipe('%s', 2000) failed\nErrCode=0x%08X\n\n", szPipe, dwWaitErr);
			wsprintf(szDbg+lstrlenW(szDbg), L"CallNamedPipe(%s) failed\nErrCode=0x%08X", szPipe, dwErr);
			MessageBox(0, szDbg, _T("FarSelector"), MB_OK|MB_SETFOREGROUND|MB_SYSTEMMODAL);
		}
#endif
	}

fail:
	return lbRc;
	//if (!lbRc)
	//	iSel = -1; // Если не удалось передать в существующий - откроем новый
}
