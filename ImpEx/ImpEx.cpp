// ImpEx.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include <malloc.h>
#include "common.h"
#include "ConEmuSupport.h"

#define countof(ar) (sizeof(ar)/sizeof((ar)[0]))

//TODO:
//1. В заголовке плагина показывать "ImpEx:<PEFileName> [<CurFolderName>]"
//2. CTRLZ - всплывать свой диалог со строками Owner/Description

bool gbUseMenu = true, gbUseEnter = true, gbUseCtrlPgDn = true, gbLibRegUnreg = true, gbLibLoadUnload = true;
void LoadSettings();
bool IsKeyPressed(WORD vk);
//#define IMPEX_USEMENU TRUE
//#define IMPEX_USEENTER FALSE
//#define IMPEX_USECTRLPGDN TRUE

PluginStartupInfo psi;
FarStandardFunctions fsf;
TCHAR* gsRootKey = NULL;

BOOL gbSilentMode = FALSE;
//BOOL gbAutoMode = FALSE;

HWND hConEmuWnd = NULL;
HANDLE hConEmuCtrlPressed = NULL;

void WINAPI SetStartupInfoW (struct PluginStartupInfo *Info)
{
	psi = *Info;
	fsf = *Info->FSF;
	psi.FSF = &fsf;

	size_t nLen = lstrlen(Info->RootKey);
	gsRootKey = (TCHAR*)calloc((nLen+32),sizeof(TCHAR));
	lstrcpy(gsRootKey, Info->RootKey);
	if (gsRootKey[nLen] != _T('\\'))
		gsRootKey[nLen++] = _T('\\');
	lstrcpy(gsRootKey+nLen, _T("ImpEx"));
}

int WINAPI GetMinFarVersionW ()
{
	return FARMANAGERVERSION;
}

void WINAPI GetPluginInfoW( struct PluginInfo *Info )
{
	LoadSettings();

    static TCHAR *szMenu[1];
    szMenu[0]=_T("ImpEx");

    Info->Flags = 0;
    Info->DiskMenuStrings = NULL;
    Info->DiskMenuNumbers = 0;
    Info->DiskMenuStringsNumber = 0;
    Info->CommandPrefix = _T("ImpEx");
    Info->PluginConfigStrings = NULL; //szMenu;
    Info->PluginConfigStringsNumber = 0; //1;
    Info->PluginMenuStrings = gbUseMenu ? szMenu : NULL;
    Info->PluginMenuStringsNumber = gbUseMenu ? 1 : 0;
}

void WINAPI GetOpenPluginInfoW( HANDLE hPlugin, struct OpenPluginInfo *Info )
{
	((MImpEx*)hPlugin)->getOpenPluginInfo(Info);
}

int WINAPI GetFindDataW(HANDLE hPlugin, struct PluginPanelItem **pPanelItem, int *pItemsNumber, int OpMode )
{
	return ((MImpEx*)hPlugin)->getFindData(pPanelItem,pItemsNumber,OpMode);
}

void WINAPI FreeFindDataW(HANDLE hPlugin, struct PluginPanelItem *PanelItem, int ItemsNumber)
{
	((MImpEx*)hPlugin)->freeFindData(PanelItem, ItemsNumber);
}

void WINAPI ClosePluginW(HANDLE hPlugin)
{
	if (hPlugin) {
		MImpEx *pPlugin = (MImpEx*)hPlugin;
		delete pPlugin;
	}
}

//#ifdef _UNICODE
//int WINAPI GetFilesW( HANDLE hPlugin, struct PluginPanelItem *PanelItem, int ItemsNumber, int Move, const wchar_t **DestPath, int OpMode )
//{
//  static MString lsBuffer;
//  LPWSTR  pszBuffer = lsBuffer.GetBuffer(MAX_PATH+wcslen(*DestPath));
//  wcscpy(pszBuffer, *DestPath);
//  *DestPath = pszBuffer;
//  return ((MImpEx*)hPlugin)->getFiles(PanelItem, ItemsNumber, Move, pszBuffer, OpMode );
//}
//#else
//int WINAPI GetFilesW( HANDLE hPlugin, struct PluginPanelItem *PanelItem, int ItemsNumber, int Move, LPSTR DestPath, int OpMode )
//{
//  return ((MImpEx*)hPlugin)->getFiles(PanelItem, ItemsNumber, Move, DestPath, OpMode );
//}
//#endif

HANDLE WINAPI OpenFilePluginW(
  TCHAR *Name,
  const unsigned char *Data,
  int DataSize
#ifdef _UNICODE
  ,int OpMode
#endif
)
{
	if (!Name || !*Name)
		return INVALID_HANDLE_VALUE; // ShiftF1
	
	_ASSERTE(sizeof(IMAGE_FILE_HEADER) < sizeof(IMAGE_DOS_HEADER));
	if (
		#ifdef _UNICODE
		(OpMode & OPM_FIND) != 0 ||
		#endif
		!Data || DataSize < sizeof(IMAGE_DOS_HEADER)
	   )
	{
		return INVALID_HANDLE_VALUE;
	}

	LPCTSTR pszNameOnly = psi.FSF->PointToName(Name);
	LPCTSTR pszExt = pszNameOnly ? _tcsrchr(pszNameOnly, _T('.')) : NULL;
	if (!pszExt) pszExt = _T("");
	
	bool lbFormat = false;
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)Data;
	PIMAGE_FILE_HEADER pImgFileHdr = (PIMAGE_FILE_HEADER)Data;
	if ( pDosHeader->e_magic == IMAGE_DOS_SIGNATURE )
	{
		lbFormat = true;
	}
	else if ( pDosHeader->e_magic == IMAGE_SEPARATE_DEBUG_SIGNATURE )
	{
		lbFormat = true;
	}
	else if ( IsValidMachineType(pImgFileHdr->Machine, TRUE) )
	{
		if ( 0 == pImgFileHdr->SizeOfOptionalHeader && lstrcmpi(pszExt, _T(".obj")) == 0 )	// 0 optional header
			lbFormat = true;
		else if ( pImgFileHdr->SizeOfOptionalHeader == IMAGE_SIZEOF_ROM_OPTIONAL_HEADER )
			lbFormat = true;
	}
	else if ( 0 == strncmp((char *)Data, IMAGE_ARCHIVE_START, IMAGE_ARCHIVE_START_SIZE) )
	{
		lbFormat = true;
	}

	if (!lbFormat)
		return INVALID_HANDLE_VALUE;

	LoadSettings();
	if (!gbUseEnter && !gbUseCtrlPgDn)
	{
		return INVALID_HANDLE_VALUE;
	}
	else if (!gbUseEnter)
	{
		if (!IsKeyPressed(VK_CONTROL))
			return INVALID_HANDLE_VALUE;
	}


	HANDLE hPlugin = NULL;
	
    TCHAR* pszFull = NULL;
    
    //#ifdef _UNICODE
	//	int nLen = (int)((FarStandardFunctions*)fsf)->ConvertPath(CPM_FULL, Name, NULL, 0);
	//	if (nLen > 0) {
	//		pszFull = (TCHAR*)calloc(nLen,sizeof(TCHAR));
	//		((FarStandardFunctions*)fsf)->ConvertPath(CPM_FULL, Name, pszFull, nLen);
	//	}
	//#endif
	
	#ifdef _UNICODE
	gbSilentMode = (OpMode & (OPM_SILENT | OPM_FIND)) != 0;
	#else
	gbSilentMode = TRUE;
	#endif
	//gbAutoMode = TRUE;
	
	hPlugin = MImpEx::Open(pszFull ? pszFull : Name, false);
	
	//if (pszFull) {
	//	free(pszFull);
	//}
	
	return hPlugin;
}

HANDLE WINAPI OpenPluginW(int OpenFrom,INT_PTR Item)
{
	HANDLE hPlugin = INVALID_HANDLE_VALUE;

	//gbAutoMode = FALSE;
	gbSilentMode = FALSE;
	
    if (OpenFrom==OPEN_PLUGINSMENU) {
        // Плагин пытаются открыть из плагиновского меню
        PanelInfo pi; memset(&pi, 0, sizeof(pi));
        if (psi.Control(INVALID_HANDLE_VALUE, FCTL_GETPANELINFO, F757NA (LONG_PTR)&pi)) {
			
            if (pi.PanelType==PTYPE_FILEPANEL && pi.ItemsNumber>0 /*&& pi.CurrentItem>0*/) {
                //
                BOOL    lbOpenMode = FALSE;
				FAR_FIND_DATA* pData = NULL;
				#ifdef _UNICODE
					PluginPanelItem* item=NULL;
				#endif
				LPCTSTR pszFileName = NULL;
                
                // Если же выделения нет, но курсор стоит на файле
                if (pi.CurrentItem<=0)
                	return INVALID_HANDLE_VALUE;
                	
				#ifdef _UNICODE
					int nSize = psi.Control(INVALID_HANDLE_VALUE, FCTL_GETPANELITEM, pi.CurrentItem, NULL);
					if (!nSize)
						return INVALID_HANDLE_VALUE;
					item = (PluginPanelItem*)calloc(nSize,1);
					psi.Control(INVALID_HANDLE_VALUE, FCTL_GETPANELITEM, pi.CurrentItem, (LONG_PTR)item);
					pData = &item->FindData;
				#else
					pData = &pi.PanelItems[pi.CurrentItem].FindData;
				#endif

				pszFileName = FILENAMEPTR(*pData);
                if (*pszFileName && _tcscmp(pszFileName, _T(".."))!=0) {
                    lbOpenMode = TRUE;
                }

                
                if (lbOpenMode) {
                    int nLen = 0;
                    TCHAR* pszFull = NULL;
                    
                    #ifdef _UNICODE
						nLen = (int)fsf.ConvertPath(CPM_FULL, pszFileName, NULL, 0);
						if (nLen > 0) {
							pszFull = (TCHAR*)calloc(nLen,sizeof(TCHAR));
							fsf.ConvertPath(CPM_FULL, pszFileName, pszFull, nLen);
						}
                    #else
                    	int nDirLen = lstrlen(pi.CurDir);
                    	nLen = nDirLen+3+lstrlen(pszFileName);
                    	pszFull = (TCHAR*)calloc(nLen,sizeof(TCHAR));
                    	lstrcpy(pszFull, pi.CurDir);
                    	if (nDirLen && pszFull[nDirLen-1] != '\\') {
                    		pszFull[nDirLen++] = '\\';
                    		pszFull[nDirLen] = 0;
                    	}
                    	lstrcpy(pszFull+nDirLen, pszFileName);
                    #endif

					if (pszFull) {
                    	hPlugin = MImpEx::Open(pszFull, true);
                    	free(pszFull);
                	}
                }
                
				#ifdef _UNICODE
					SAFEFREE(item);
				#elif FARBUILD>=757
					psi.Control(INVALID_HANDLE_VALUE, FCTL_FREEPANELITEM, 0, (LONG_PTR)&item);
				#endif
            }
        }
    } else if (OpenFrom==OPEN_COMMANDLINE) {
		if (Item) {
			TCHAR *pszTemp = NULL;
			TCHAR *pszFull = NULL;
			LPCTSTR pszName = (LPCTSTR)Item;
			if (*pszName == _T('"')) {
				pszTemp = _tcsdup(pszName+1);
				TCHAR* pszQ = _tcschr(pszTemp, _T('"'));
				if (pszQ) *pszQ = 0;
				pszName = pszTemp;
			}

			#ifdef _UNICODE
				int nLen = (int)fsf.ConvertPath(CPM_FULL, pszName, NULL, 0);
				if (nLen > 0) {
					pszFull = (TCHAR*)calloc(nLen,sizeof(TCHAR));
					fsf.ConvertPath(CPM_FULL, pszName, pszFull, nLen);
					pszName = pszFull;
				}
			#else
				pszFull = (TCHAR*)calloc(MAX_PATH*2,sizeof(TCHAR));
				TCHAR* pszFilePart = NULL;
				if (GetFullPathName(pszName, MAX_PATH*2, pszFull, &pszFilePart))
					pszName = pszFull;								
			#endif

            hPlugin = MImpEx::Open(pszName, true);
			if (pszTemp) free(pszTemp);
			if (pszFull) free(pszFull);
		}
    }
    return hPlugin;
}

int WINAPI SetDirectoryW ( HANDLE hPlugin, LPCTSTR Dir, int OpMode )
{
    // В случае удачного завершения возвращаемое значение должно равняться TRUE. Иначе функция должна возвращать FALSE. 
    return ((MImpEx*)hPlugin)->setDirectory ( Dir, OpMode );
}

int WINAPI ConfigureW(int ItemNumber)
{
	//TODO
    return TRUE;
}

int WINAPI CompareW(HANDLE hPlugin,const struct PluginPanelItem *Item1,const struct PluginPanelItem *Item2,unsigned int Mode)
{
	if (!_tcscmp(FILENAMEPTR(Item1->FindData), DUMP_FILE_NAME))
		return -1;
	else if (!_tcscmp(FILENAMEPTR(Item2->FindData), DUMP_FILE_NAME))
		return 1;
	
	return -2; // использовать внутреннюю сортировку
}

int WINAPI ProcessKeyW(HANDLE hPlugin, int Key, unsigned int ControlState)
{
    return FALSE; // отдать фару
}

void merror(LPCTSTR asFormat, ...)
{
	if (gbSilentMode) return;
	
    va_list args;
    va_start(args,asFormat);
    TCHAR szBuffer[2048]; szBuffer[0] = 0;
    
    lstrcpy(szBuffer, _T("ImpEx\n"));
    
    int nLen = wvsprintf(szBuffer+lstrlen(szBuffer), asFormat, args);
	if (!nLen || !szBuffer[0])
		lstrcpy(szBuffer, _T("<Empty error text>"));
	
	psi.Message(psi.ModuleNumber, FMSG_ALLINONE|FMSG_MB_OK|FMSG_WARNING, 
		NULL, (const TCHAR * const *)szBuffer, 0,0);
}

int WINAPI GetFilesW(
		HANDLE hPlugin, struct PluginPanelItem *PanelItem, int ItemsNumber, int Move,
		#ifdef _UNICODE
			const wchar_t **DestPath, int OpMode
		#else
			LPSTR DestPath, int OpMode
		#endif
		)
{
#ifdef _UNICODE
	static wchar_t szBuffer[MAX_PATH*2];
	wcscpy(szBuffer, *DestPath);
	*DestPath = szBuffer;
	return ((MImpEx*)hPlugin)->getFiles(PanelItem, ItemsNumber, Move, szBuffer, OpMode );
#else
	return ((MImpEx*)hPlugin)->getFiles(PanelItem, ItemsNumber, Move, DestPath, OpMode );
#endif
}

void WINAPI _export ExitFARW(void)
{
	if (gsRootKey)
	{
		free(gsRootKey); gsRootKey = NULL;
	}
	if (hConEmuCtrlPressed)
	{
		CloseHandle(hConEmuCtrlPressed); hConEmuCtrlPressed = NULL;
	}
}

void LoadSettings()
{
	if (!gsRootKey) return;

	bool b;
	HKEY h;
	DWORD nSize, nType;
	if (!RegCreateKeyEx(HKEY_CURRENT_USER, gsRootKey, 0, NULL, 0, KEY_ALL_ACCESS, NULL, &h, NULL))
	{
		// gbUseMenu, gbUseEnter, gbUseCtrlPgDn, gbLibRegUnreg, gbLibLoadUnload;
		struct tag_Settings {
			bool* pbVal; LPCTSTR psName;
		};
		struct tag_Settings Settings[] =
		{
			{&gbUseMenu, _T("UseMenu")},
			{&gbUseEnter, _T("UseEnter")},
			{&gbUseCtrlPgDn, _T("UseCtrlPgDn")},
			{&gbLibRegUnreg, _T("LibRegUnreg")},
			{&gbLibLoadUnload, _T("LibLoadUnload")}
		};
		for (int i = 0; i < countof(Settings); i++)
		{
			if (RegQueryValueEx(h, Settings[i].psName, NULL, &nType, (LPBYTE)&b, &(nSize=1)))
				RegSetValueEx(h, Settings[i].psName, 0, REG_BINARY, (LPBYTE)Settings[i].pbVal, 1);
			else
				*Settings[i].pbVal = b;
		}
		RegCloseKey(h);
	}
}

bool IsKeyPressed(WORD vk)
{
	USHORT st = GetKeyState(vk);
	if ((st & 0x8000))
		return true;

	if (vk == VK_CONTROL)
	{
		// что-то в ConEmu GetKeyState иногда ничего не дает? возвращается 0
		if (hConEmuWnd && !IsWindow(hConEmuWnd))
		{
			hConEmuWnd = NULL;
		}
		if (!hConEmuWnd)
		{
			hConEmuWnd = GetConEmuHwnd();
		}
		if (hConEmuWnd)
		{
			if (!hConEmuCtrlPressed)
			{
				TCHAR szName[64]; DWORD dwPID = GetCurrentProcessId();
				wsprintf(szName, CEKEYEVENT_CTRL, dwPID);
				hConEmuCtrlPressed = OpenEvent(SYNCHRONIZE, FALSE, szName);
			}
			
			if (hConEmuCtrlPressed)
			{
				DWORD dwWait = WAIT_TIMEOUT;
				if (vk == VK_CONTROL)
					dwWait = WaitForSingleObject(hConEmuCtrlPressed, 0);
				return (dwWait == WAIT_OBJECT_0);
			}
		}
	}
	return false;
}
