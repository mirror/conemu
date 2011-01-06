
/*
PictureView wrapper  for ConEmu
*/

/*
Copyright (c) 2009-2010 Maximus5
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
#include <TCHAR.H>
#include "PicViewWrapper.h"
#include "EMUPIC.h"
#include "SetHook.h"
#include "common.hpp"
//#include "ConEmuCheck.h"


//extern "C" {
//#include "ConEmuCheck.h"
//}

/* FAR */
PluginStartupInfo psi;
FarStandardFunctions fsf;
FWrapperAdvControl fWrapperAdvControl=NULL;

/* PictureView */
HMODULE hPicViewDll=NULL;
FClosePlugin fClosePlugin=NULL;
FConfigure fConfigure=NULL;
FExitFAR fExitFAR=NULL;
FGetMinFarVersion fGetMinFarVersion=NULL;
FGetPluginInfo fGetPluginInfo=NULL;
FOpenFilePlugin fOpenFilePlugin=NULL;
FOpenPlugin fOpenPlugin=NULL;
FProcessEditorEvent fProcessEditorEvent=NULL;
FProcessViewerEvent fProcessViewerEvent=NULL;
FSetStartupInfo fSetStartupInfo=NULL;

/* Local */
HINSTANCE ghInstance = NULL;
TCHAR gsTermMsg[128];
TCHAR gsLoadMsg[320];

/* ConEmu */
HWND ghConEmu = NULL;
//HMODULE ghConEmuDll=NULL;
//typedef HWND (WINAPI* FGetFarHWND2)(BOOL abConEmuOnly);
//FGetFarHWND2 GetFarHWND2=NULL;
BOOL gbOldConEmu=FALSE;
BOOL gbConEmuChecked=FALSE, gbOldConEmuWarned=FALSE;
BOOL IsOldConEmu();
//typedef HWND (WINAPI* FIsTerminalMode)();
//FIsTerminalMode IsTerminalMode=NULL;


BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        {
            // Init variables
            ghInstance = (HINSTANCE)hModule;
            gsTermMsg[0] = 0; gsLoadMsg[0] = 0;
            memset(&psi, 0, sizeof(psi));
            memset(&fsf, 0, sizeof(fsf));

            // 21.03.2010 Maks - Проверим в SetStartupInfo, чтобы ошибки показать, если что...
            // Инициализировать хэндл КонЕму и проверить "свежесть"
            //IsOldConEmu();
            
            // Check Terminal mode
            TCHAR szVarValue[MAX_PATH];
            szVarValue[0] = 0; gsTermMsg[0] = 0;
            if (GetEnvironmentVariable(_T("TERM"), szVarValue, 63)) {
                lstrcpy(gsTermMsg, _T("PictureView wrapper\nPicture viewing is not available\nin terminal mode ("));
                lstrcat(gsTermMsg, szVarValue);
                lstrcat(gsTermMsg, _T(")"));
            }

            
            // Init PictureView plugin
            if (!hPicViewDll && (gsTermMsg[0] == 0)) {
                TCHAR szModulePath[MAX_PATH+1]; //, *pszSlash;
                int nLen = 0;
                if ((nLen=GetModuleFileName(ghInstance, szModulePath, MAX_PATH))>0)
                {
                    if (szModulePath[nLen-1]!=_T('L') && szModulePath[nLen-1]!=_T('l')) {
	                    if (!gsTermMsg[0]) {
		                    DWORD dwErr;
		                    dwErr = GetLastError();
		                    wsprintfA(gsLoadMsg, "PictureView wrapper\nInvalid file name!\n%s\n*.dll expected", szModulePath);
	                        //MessageBox(ghConEmu, szError, _T("PictureView wrapper"), MB_ICONSTOP);
	                    }
                        return TRUE; // иначе FAR в Windows7 иногда не загружался вообще
                    }
                    szModulePath[nLen-1] = '_';
                    hPicViewDll = LoadLibrary(szModulePath);
                    if (!hPicViewDll) {
	                    if (!gsTermMsg[0]) {
		                    DWORD dwErr;
		                    dwErr = GetLastError();
		                    wsprintfA(gsLoadMsg, "PictureView wrapper\nCan't load library!\n%s\nLastError=0x%08X", szModulePath, dwErr);
	                        //MessageBox(ghConEmu, szError, _T("PictureView wrapper"), MB_ICONSTOP);
	                    }
                        return TRUE; // иначе FAR в Windows7 иногда не загружался вообще
                    }
                }
            }
            if (hPicViewDll) {
	            fClosePlugin=(FClosePlugin)GetProcAddress(hPicViewDll,"ClosePlugin");
	            fConfigure=(FConfigure)GetProcAddress(hPicViewDll,"Configure");
	            fExitFAR=(FExitFAR)GetProcAddress(hPicViewDll,"ExitFAR");
	            fGetMinFarVersion=(FGetMinFarVersion)GetProcAddress(hPicViewDll,"GetMinFarVersion");
	            fGetPluginInfo=(FGetPluginInfo)GetProcAddress(hPicViewDll,"GetPluginInfo");
	            fOpenFilePlugin=(FOpenFilePlugin)GetProcAddress(hPicViewDll,"OpenFilePlugin");
	            fOpenPlugin=(FOpenPlugin)GetProcAddress(hPicViewDll,"OpenPlugin");
	            fProcessEditorEvent=(FProcessEditorEvent)GetProcAddress(hPicViewDll,"ProcessEditorEvent");
	            fProcessViewerEvent=(FProcessViewerEvent)GetProcAddress(hPicViewDll,"ProcessViewerEvent");
	            fSetStartupInfo=(FSetStartupInfo)GetProcAddress(hPicViewDll,"SetStartupInfo");
	        }
        } break;
    case DLL_PROCESS_DETACH:
        {
            if (hPicViewDll) {
                FreeLibrary(hPicViewDll);
                hPicViewDll = NULL;
            }
        } break;
    }
    return TRUE;
}


void WINAPI ClosePlugin(
  HANDLE hPlugin
)
{
    if (hPicViewDll && fClosePlugin)
        fClosePlugin(hPlugin);
}

int WINAPI Configure(
  int ItemNumber
)
{
    if (/*!gbOldConEmu &&*/ hPicViewDll && fConfigure)
        return fConfigure(ItemNumber);
    return FALSE;
}

void WINAPI ExitFAR( void )
{
    if (hPicViewDll && fExitFAR)
        fExitFAR();
}

int WINAPI GetMinFarVersion(void)
{
    if (hPicViewDll && fGetMinFarVersion)
        return fGetMinFarVersion();
    return MAKEFARVERSION(1,71,2470);
}

void WINAPI GetPluginInfo(
  struct PluginInfo *Info
)
{
    if (hPicViewDll && fGetPluginInfo)
        fGetPluginInfo(Info);
}

HANDLE WINAPI OpenFilePlugin(
  char *Name,
  const unsigned char *Data,
  int DataSize
)
{
    HANDLE hRc = INVALID_HANDLE_VALUE;
    if (/*!gbOldConEmu &&*/ hPicViewDll && fOpenFilePlugin) {
        if (gsTermMsg[0]) {
            TCHAR *pszExt = NULL;
            // PicView может использоваться и на Enter/CtrlPgDn, будем ругаться только на "известные" файлы, остальные просто игнорировать
            if (Name && *Name) {
                for (pszExt=Name+lstrlen(Name)-1; pszExt>=Name && *pszExt!=_T('\\'); pszExt--) {
                    if (*pszExt==_T('.')) {
                        pszExt++;
                        if (lstrcmpi(pszExt, _T("jpg")) && lstrcmpi(pszExt, _T("jpeg")) && lstrcmpi(pszExt, _T("jfif")) &&
                            lstrcmpi(pszExt, _T("tif")) && lstrcmpi(pszExt, _T("tiff")) &&
                            lstrcmpi(pszExt, _T("bmp")) && lstrcmpi(pszExt, _T("dib")) &&
                            lstrcmpi(pszExt, _T("wmf")) && lstrcmpi(pszExt, _T("emf")) &&
                            lstrcmpi(pszExt, _T("pcx")) &&
                            lstrcmpi(pszExt, _T("gif")) &&
                            1)
                            pszExt=NULL;
                        break;
                    }
                }

            }
            if (pszExt && psi.Message) {
                psi.Message(psi.ModuleNumber, FMSG_MB_OK|FMSG_ALLINONE, NULL, 
                    (const TCHAR *const *)gsTermMsg, 0, 0);
            }
        } else {
            hRc = fOpenFilePlugin(Name, Data, DataSize);
        }
    }
    return hRc;
}

HANDLE WINAPI OpenPlugin(
  int OpenFrom,
  INT_PTR Item
)
{
    HANDLE hRc = INVALID_HANDLE_VALUE;
    if (/*!gbOldConEmu &&*/ hPicViewDll && fOpenPlugin) {
        if (gsTermMsg[0] && psi.Message)
            psi.Message(psi.ModuleNumber, FMSG_MB_OK|FMSG_ALLINONE, NULL, 
                (const TCHAR *const *)gsTermMsg, 0, 0);
        else
            hRc = fOpenPlugin(OpenFrom, Item);
    }
    return hRc;
}

int WINAPI ProcessEditorEvent(
  int Event,
  void *Param
)
{
    if (/*!gbOldConEmu &&*/ hPicViewDll && fProcessEditorEvent)
        return fProcessEditorEvent(Event, Param);
    return FALSE;
}

int WINAPI ProcessViewerEvent(
  int Event,
  void *Param
)
{
    if (/*!gbOldConEmu &&*/ hPicViewDll && fProcessViewerEvent)
        return fProcessViewerEvent(Event, Param);
    return FALSE;
}

void WINAPI SetStartupInfo(
  const struct PluginStartupInfo *Info
)
{
    ::psi = *Info;
    ::fsf = *(Info->FSF);

    if (gsTermMsg[0] != 0)
    	return; // В терминале мы не работаем

	// При ошибке загрузки собственно плагина
    if (gsLoadMsg[0] && psi.Message) {
        psi.Message(psi.ModuleNumber, FMSG_MB_OK|FMSG_ALLINONE, NULL, 
            (const TCHAR *const *)gsLoadMsg, 0, 0);
        return;
    }

    // Теперь - можно поискать ConEmu
    IsOldConEmu();
    
    //if (IsOldConEmu) {
    //    psi.Message(psi.ModuleNumber, FMSG_MB_OK|FMSG_ALLINONE, NULL, 
    //        (const TCHAR *const *)"PictureView wrapper\nConEmu old version detected!\nPlease upgrade!", 0, 0);
    //}

    fWrapperAdvControl = psi.AdvControl;
    psi.AdvControl = WrapperAdvControl;

    psi.FSF = &fsf;
    
    // Загрузить "SlideShowElapse"=dword:00001000

    if (hPicViewDll && fSetStartupInfo)
        fSetStartupInfo(&psi);
}

INT_PTR WINAPI WrapperAdvControl(int ModuleNumber,int Command,void *Param)
{
    if (!fWrapperAdvControl)
        return 0;

    if (Command==ACTL_GETFARHWND && !IsOldConEmu() && ghConEmu) {
        HWND hFarWnd = (HWND)fWrapperAdvControl(ModuleNumber, ACTL_GETFARHWND, NULL);
        //if (IsWindowVisible(hFarWnd))
        //    return (INT_PTR)hFarWnd;
            
        if (IsWindow(ghConEmu)) {
            return (INT_PTR)ghConEmu;
        } else {
            return (INT_PTR)hFarWnd;
        }
    }

    return fWrapperAdvControl(ModuleNumber, Command, Param);
}

BOOL IsOldConEmu()
{
	// GUI могло свалиться, тогда окошко ConEmu нужно поискать повторно
	if (gbConEmuChecked && ghConEmu) {
		if (!IsWindow(ghConEmu)) {
			gbConEmuChecked = FALSE;
			ghConEmu = NULL;
		}
	}

    if (!gbConEmuChecked) {
	    //int nRc = -1;
	    
	    gbConEmuChecked = TRUE;

	    DWORD dwErr = 0;
	    HWND FarHwnd = GetConsoleWindow();
	    char szMapName[64];
		wsprintfA(szMapName, CECONMAPNAME_A, (DWORD)FarHwnd);

		HANDLE hFileMapping = OpenFileMapping(FILE_MAP_READ, FALSE, szMapName);
		if (hFileMapping) {
			CESERVER_REQ_CONINFO_HDR* pConsoleInfo = 
					(CESERVER_REQ_CONINFO_HDR*)MapViewOfFile(hFileMapping, FILE_MAP_READ,0,0,0);
			if (pConsoleInfo) {
				// Проверяем и ищем ConEmu HWND
				if (((HWND)pConsoleInfo->hConWnd) != FarHwnd) {
					gbOldConEmu = TRUE;
				} else {
					DWORD dwGuiPid = pConsoleInfo->nGuiPID;
					DWORD dwPID;
					HWND hRoot = FindWindowExW(NULL, NULL, VirtualConsoleClassMain, NULL);
					while (hRoot) {
						dwPID = 0; GetWindowThreadProcessId(hRoot, &dwPID);
						if (dwPID == dwGuiPid) {
							ghConEmu = FindWindowExW(hRoot, NULL, VirtualConsoleClass, NULL);
							if (!ghConEmu || IsWindowVisible(ghConEmu)) {
								// Дочернее окно (ViewPort) должен быть невидимым
								gbOldConEmu = TRUE;
							}
							break;
						}
						hRoot = FindWindowExW(NULL, hRoot, VirtualConsoleClassMain, NULL);
					}
				}

				UnmapViewOfFile(pConsoleInfo);
			} else {
				dwErr = GetLastError();
				char szError[255];
				wsprintfA(szError, "PictureView wrapper\nMapViewOfFile() failed. Can't find ConEmu.\nErrorCode=0x%08X\nMapName=", dwErr);
				lstrcatA(szError, szMapName);
				psi.Message(psi.ModuleNumber, FMSG_MB_OK|FMSG_ALLINONE, NULL, 
            		(const TCHAR *const *)szError, 0, 0);
			}
			CloseHandle(hFileMapping);
		} else {
			if (!IsWindowVisible(FarHwnd))
				gbOldConEmu = TRUE;
		}

	    
	    //ghConEmu = NULL;
	    //nRc = ConEmuCheck(&ghConEmu);
		//// 0 -- All OK (ConEmu found, Version OK)
		//// 1 -- NO ConEmu (simple console mode)
		//// 2 -- ConEmu found, but old version
	    //if (ghConEmu && nRc==2)
		//    gbOldConEmu = TRUE;
    }

    if (gbOldConEmu && !gbOldConEmuWarned && psi.Message)
    {
	    gbOldConEmuWarned = TRUE;
        psi.Message(psi.ModuleNumber, FMSG_MB_OK|FMSG_ALLINONE, NULL, 
            (const TCHAR *const *)"PictureView wrapper\nConEmu unsupported version detected!\nPlease upgrade!", 0, 0);
    } else {
    	// Инициализация необходимых хэндлов в EMUPIC.cpp
    	InitConEmu(hPicViewDll, ghConEmu);
        // Теперь ставим хуки
        if (!SetAllHooks( (HMODULE)ghInstance, hPicViewDll )) {
	        psi.Message(psi.ModuleNumber, FMSG_MB_OK|FMSG_ALLINONE, NULL, 
	            (const TCHAR *const *)"PictureView wrapper\nCan't initialize injects in the original plugin!", 0, 0);
        }
    }
    return gbOldConEmu;
}
