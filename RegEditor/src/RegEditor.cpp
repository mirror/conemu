//DEBUG:
//F:\VCProject\FarPlugin\#FAR180\far.exe
//C:\Temp\1000-very-long-path\F1..567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890\F2..567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890

#define PRINT_FAR_VERSION
//#define SHOW_INVALID_OP_ON_DIR


#include "header.h"
#include "RE_CmdLine.h"

HINSTANCE g_hInstance;
struct PluginStartupInfo psi;
struct FarStandardFunctions FSF;
HANDLE ghHeap = NULL;
DWORD gnFarVersion = 0;


#if defined(__GNUC__)
BOOL APIENTRY DllMain( HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved )
{
	switch (ul_reason_for_call) {
		case DLL_PROCESS_ATTACH:
			break;
	}
	return TRUE;
}
#endif

#ifdef _UNICODE
#define OPEN_ANALYSE 9
struct AnalyseData
{
	int StructSize;
	const wchar_t *lpwszFileName;
	const unsigned char *pBuffer;
	DWORD dwBufferSize;
	int OpMode;
};
struct AnalyseData *gpLastAnalyse = NULL;
#endif

#if defined(__GNUC__)
#ifdef __cplusplus
extern "C"{
#endif
  BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved);
  void WINAPI SetStartupInfoW(const PluginStartupInfo *aInfo);
  void WINAPI ExitFARW(void);
  HANDLE WINAPI OpenPluginW(int OpenFrom,INT_PTR Item);
  int WINAPI GetMinFarVersionW(void);
  void WINAPI GetPluginInfoW(struct PluginInfo *pi);
  int WINAPI _export GetFindDataW(HANDLE hPlugin,struct PluginPanelItem **pPanelItem,int *pItemsNumber,int OpMode);
  void WINAPI _export FreeFindDataW(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber);
  void WINAPI _export GetOpenPluginInfoW(HANDLE hPlugin,struct OpenPluginInfo *Info);
  void WINAPI _export ClosePluginW(HANDLE hPlugin);
  int WINAPI _export SetDirectoryW ( HANDLE hPlugin, LPCTSTR Dir, int OpMode );
  int WINAPI _export ProcessEventW(HANDLE hPlugin,int Event,void *Param);
  #ifdef _UNICODE
  int WINAPI _export ProcessSynchroEventW(int Event, void *Param);
  int WINAPI _export ProcessEditorEventW(int Event, void *Param);
  int WINAPI _export AnalyseW(const AnalyseData *pData);
  #endif
  int WINAPI _export ProcessKeyW(HANDLE hPlugin,int Key,unsigned int ControlState);
#ifdef __cplusplus
};
#endif

BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved)
{
  return DllMain(hDll, dwReason,lpReserved);
}
#endif

#ifdef CRTSTARTUP
extern "C"{
BOOL WINAPI _DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved)
{
  return TRUE;
};
};
#endif

int WINAPI _export GetMinFarVersionW(void)
{
	int nVer =
	#ifdef _UNICODE
	// 1607 - только там поправлено падение фара при FCTL_SETNUMERICSORT
	MAKEFARVERSION(FARMANAGERVERSION_MAJOR,FARMANAGERVERSION_MINOR,max(FARMANAGERVERSION_BUILD,1607))
	#else
	FARMANAGERVERSION
	#endif
	;
	return nVer;
}

void WINAPI _export GetPluginInfoW(struct PluginInfo *pi)
{
	if (!cfg) {
		_ASSERTE(cfg!=NULL);
		return;
	}
	
	MCHKHEAP;

	static TCHAR szMenu[MAX_PATH];
	lstrcpy(szMenu, GetMsg(REPluginName)); // "RegEditor"
    static TCHAR *pszMenu[1];
    pszMenu[0] = szMenu;
    static int nDiskNumber[1];
    nDiskNumber[0] = cfg->cDiskMenuHotkey[0] - _T('0');

	pi->StructSize = sizeof(struct PluginInfo);
	pi->Flags = cfg->bAddToPluginsMenu ? 0 : PF_DISABLEPANELS;
	pi->DiskMenuStringsNumber = cfg->bAddToDisksMenu ? 1 : 0;
	pi->DiskMenuStrings = cfg->bAddToDisksMenu ? pszMenu : NULL;
#ifdef _UNICODE
	// Пока оставим, т.к. плагин может работать и в "старых" сборках 2.0
	if (HIWORD(gnFarVersion) < 1692)
		pi->DiskMenuNumbers = cfg->bAddToDisksMenu ? nDiskNumber : NULL;
	else
		pi->DiskMenuNumbers = NULL;
#else
	pi->DiskMenuNumbers = cfg->bAddToDisksMenu ? nDiskNumber : NULL;
#endif
	pi->PluginMenuStrings = pszMenu;
	pi->PluginMenuStringsNumber = 1;
	pi->PluginConfigStrings = pszMenu;
	pi->PluginConfigStringsNumber = 1;
	pi->CommandPrefix = cfg->sCommandPrefix;
	pi->Reserved = REGEDIT_MAGIC; //'RgEd';

	MCHKHEAP;
}

void WINAPI _export SetStartupInfoW(const PluginStartupInfo *aInfo)
{
	_ASSERTE(MAX_REGKEY_NAME <= MAX_PATH);

#ifdef _DEBUG
	BOOL bDump = FALSE;
#endif

	if (ghHeap == NULL)
	{
		#ifdef MVALIDATE_POINTERS
			ghHeap = HeapCreate(0, 1<<20, 0);
		#else
			ghHeap = HeapCreate(HEAP_GENERATE_EXCEPTIONS, 1<<20, 0);
		#endif

		#ifdef _DEBUG
		if (bDump) xf_dump();
		#endif
	}

	MCHKHEAP;
	g_hInstance = GetModuleHandle(aInfo->ModuleName);
	::psi = *aInfo;
	::FSF = *aInfo->FSF;
	::psi.FSF = &::FSF;

	psi.AdvControl(psi.ModuleNumber, ACTL_GETFARVERSION, &gnFarVersion);
	_ASSERTE(gnFarVersion!=0);
	/*
	HIWORD:         = номер билда   (FAR 1.70.387 = 0x0183)
	LOWORD:  HIBYTE = major version (FAR 1.70.387 = 0x01)
			 LOBYTE = minor version (FAR 1.70.387 = 0x46)
	*/

	// Создаст (если надо) cfg и загрузит настройки
	RegConfig::Init(psi.RootKey, aInfo->ModuleName);
	// Для регистрации плагинов
	if (!gpPluginList)
	{
		gpPluginList = new REPluginList();
	}
	MCHKHEAP;

	#ifdef _DEBUG
	if (bDump) xf_dump();
	#endif
}

void   WINAPI _export ExitFARW(void)
{
	#ifdef _UNICODE
	SafeFree(gpLastAnalyse);
	#endif

	MCHKHEAP;
	SafeDelete(cfg);
	SafeDelete(gpPluginList);
	MCHKHEAP;

	if (ghHeap) {
		#ifdef TRACK_MEMORY_ALLOCATIONS
		xf_dump();
		#endif

		HANDLE h = ghHeap;
		ghHeap = NULL;
		HeapDestroy(h);		
	}
}


int WINAPI _export SetDirectoryW ( HANDLE hPlugin, LPCTSTR Dir, int OpMode )
{
	CPluginActivator a(hPlugin,OpMode);
	REPlugin* pPlugin = (REPlugin*)hPlugin;
	if (!pPlugin)
	{
		_ASSERTE(pPlugin);
		InvalidOp();
		return FALSE;
	}
	// Сразу отсечем некорректные аргументы
	if (!Dir || !*Dir)
	{
		_ASSERTE(Dir && *Dir);
		InvalidOp();
		return FALSE;
	}

	MCHKHEAP;
	
	BOOL lbSilence = (OpMode & (OPM_FIND|OPM_SILENT)) != 0;
	int  nRc = 0;

	// Закрытие плагина (нужно только в сетевом режиме - в других Фар закрывает плагин сам)
	if (pPlugin->m_Key.eType == RE_WINAPI && pPlugin->mb_RemoteMode)
	{
		if ((Dir[0] ==_T('\\') && Dir[1] == 0) || (Dir[0] ==_T('.') && Dir[1] ==_T('.') && Dir[2] == 0))
		{
			if (pPlugin->m_Key.mh_Root == NULL && !(pPlugin->m_Key.mpsz_Key && *pPlugin->m_Key.mpsz_Key))
			{
				psi.Control(hPlugin, FCTL_CLOSEPLUGIN, F757NA 0);
				return TRUE;
			}
		}
	}


	// Чтобы обработать "странные" папки, которые нельзя просто отобразить в фар
	// (например "..") нужно проверить, стоит ли курсор на
	if (_tcschr(Dir, _T('\\')) == NULL)
	{
		const PluginPanelItem* ppi = NULL;
		RegItem* p = pPlugin->GetCurrentItem(&ppi);
		if (p && ppi && FILENAMEPTR(ppi->FindData))
		{
			if (lstrcmpi(FILENAMEPTR(ppi->FindData), Dir) == 0)
			{
				p->pFolder->AddRef();
				nRc = pPlugin->SetDirectory(p, lbSilence);
				p->pFolder->Release();
				goto fin;
			}
		}
	}


	// Для удобства - сделаем копию памяти
	wchar_t* pwszDir = lstrdup_w(Dir);
	bool lbMayBeRoot = false;
	if (*pwszDir != L'\\')
	{
		wchar_t* pwszSlash = wcschr(pwszDir, L'\\');
		if (pwszSlash) *pwszSlash = 0;
		HKEY hkTest = NULL;
		lbMayBeRoot = StringKeyToHKey(pwszDir, &hkTest);
		if (pwszSlash) *pwszSlash = L'\\';
	}
	// Это может быть строка, скопированная из *.reg
	// "[HKEY_CURRENT_USER\Software\Far\Users]"
	DebracketRegPath(pwszDir);
	nRc = pPlugin->SetDirectory(pwszDir, lbSilence);
	SafeFree(pwszDir);

	MCHKHEAP;

	// IMHO глюк фара - переход из TempPanel дает путь без "\\"
	if (!nRc && lbMayBeRoot)
	{
		//int nCurDirLen = lstrlen(pPlugin->m_Key.mpsz_Dir);
		int nSetDirLen = lstrlen(Dir);
		//if (nSetDirLen
		MCHKHEAP;
		pwszDir = (wchar_t*)malloc((nSetDirLen+3)*2);
		*pwszDir = L'\\';
		lstrcpy_t(pwszDir+1, nSetDirLen+2, Dir);
		MCHKHEAP;
		nRc = pPlugin->SetDirectory(pwszDir, lbSilence);
		MCHKHEAP;
		SafeFree(pwszDir);
	}


	//#ifdef _UNICODE
	//	nRc = pPlugin->SetDirectory(Dir, lbSilence);
	//#else
	//	int nLen = lstrlenA(Dir)+1;
	//	wchar_t* pwszDir = (wchar_t*)malloc(nLen*2);
	//	lstrcpy_t(pwszDir, nLen, Dir);
	//	nRc = pPlugin->SetDirectory(pwszDir, lbSilence);
	//	SafeFree(pwszDir);
	//#endif
	

fin:

	#ifdef SHOW_INVALID_OP_ON_DIR
		if (!nRc && !(gnOpMode & (OPM_FIND|OPM_SILENT))) {
			REPlugin::MessageFmt(REM_SetDirectoryFailed, Dir);
		}
	#endif
	return nRc;
}



#ifdef SHOW_INVALID_OP_ON_DIR
	#define InvalidCmd() REPlugin::MessageFmt(REM_OpenPluginPrefixFailed, (TCHAR*)Item); InvalidOp()
#else
	#define InvalidCmd()
#endif

HANDLE WINAPI _export OpenPluginW(int OpenFrom,INT_PTR Item)
{
#ifdef _DEBUG
	BOOL bDump = FALSE;
	if (bDump) xf_dump();
#endif

	#ifdef _UNICODE
	// После AnalyseW вызывается OpenPluginW, а не OpenFilePluginW
	if (OpenFrom == OPEN_ANALYSE)
	{
		HANDLE hPlugin = OpenFilePluginW(gpLastAnalyse->lpwszFileName, gpLastAnalyse->pBuffer, gpLastAnalyse->dwBufferSize, 0/*OpMode*/);
		SafeFree(gpLastAnalyse);
		if (hPlugin == (HANDLE)-2)
			hPlugin = INVALID_HANDLE_VALUE;
		return hPlugin;
	}
	SafeFree(gpLastAnalyse);
	#endif


	REPlugin* pPlugin = NULL;

	MCHKHEAP;

	pPlugin = new REPlugin();
	if (!pPlugin) return INVALID_HANDLE_VALUE;
	
	CPluginActivator a((HANDLE)pPlugin,0);
	
	bool lbDirSet = false;
	if (((OpenFrom & 0xFF) == OPEN_COMMANDLINE) && Item)
	{
		LPCTSTR pszCommand = (LPCTSTR)Item;

		OpenPluginArg Args;
		MCHKHEAP;
		switch (Args.ParseCommandLine(pszCommand))
		{
		case aBrowseLocal:
			{
				pPlugin->ChDir(L"\\", TRUE);
				if (Args.wsBrowseLocalKey && *Args.wsBrowseLocalKey)
				{
					pPlugin->SetDirectory(Args.wsBrowseLocalKey, FALSE);
					lbDirSet = true;
				}
			} break;
		case aBrowseRemote:
			{
				if (!pPlugin->ConnectRemote(Args.wsBrowseRemoteServer, Args.wsBrowseRemoteLogin, Args.wsBrowseRemotePassword))
				{
					SafeDelete(pPlugin);
					gpActivePlugin = NULL;
					InvalidCmd();
					return INVALID_HANDLE_VALUE;
				}
				if (Args.wsBrowseRemoteKey && *Args.wsBrowseRemoteKey)
				{
					pPlugin->SetDirectory(L"\\", FALSE);
					pPlugin->SetDirectory(Args.wsBrowseRemoteKey, FALSE);
				}
				lbDirSet = true;
			} break;
		case aBrowseFileReg:
			{
				//gpProgress = new REProgress(GetMsg(RELoadingRegFileTitle), TRUE); -- создает сам плагин!
				BOOL lbLoadRc = pPlugin->LoadRegFile(Args.wsBrowseFileName, FALSE, FALSE, FALSE);
				//SafeDelete(gpProgress);
				if (!lbLoadRc)
				{
					SafeDelete(pPlugin);
					gpActivePlugin = NULL;
					InvalidCmd();
					return INVALID_HANDLE_VALUE;
				}
				lbDirSet = true;
			} break;
		case aBrowseFileHive:
			{
				BOOL lbLoadRc = pPlugin->LoadHiveFile(Args.wsBrowseFileName, FALSE, FALSE);
				if (!lbLoadRc)
				{
					SafeDelete(pPlugin);
					gpActivePlugin = NULL;
					InvalidCmd();
					return INVALID_HANDLE_VALUE;
				}
				lbDirSet = true;
			} break;
		case aExportKeysValues:
			{
				// Экспортировать и сразу выйти
				//pPlugin->Export(nExportFormat, ppszExportKeysOrValues, nExportKeysOrValuesCount, pszExportDestFile);
				InvalidOp();
				SafeDelete(pPlugin);
				gpActivePlugin = NULL;
				return INVALID_HANDLE_VALUE;
			} break;
		case aImportRegFile:
			{
				// Импортировать и сразу выйти
				if (cfg->bConfirmImport)
				{
					int nConfirm = REPlugin::MessageFmt(REM_ImportConfirm, Args.pszSourceFile, 0, _T("ImportKey"), FMSG_MB_YESNO, 0);
					if (nConfirm != 0)
						return INVALID_HANDLE_VALUE;
				}

				MFileReg fileReg;
				MRegistryWinApi winReg;

				gpProgress = new REProgress(GetMsg(REImportDlgTitle), TRUE);
				LONG hRc = MFileReg::LoadRegFile(Args.pszSourceFile, FALSE, &winReg, FALSE, &fileReg);
				SafeDelete(gpProgress);
				if (hRc == 0)
				{
					if (cfg->bShowImportResult)
						REPlugin::MessageFmt(REM_ImportSuccess, Args.pszSourceFile, 0, NULL, FMSG_MB_OK);
				}
				else
				{
					REPlugin::MessageFmt(REM_ImportFailed, Args.pszSourceFile);
				}

				// В любом случае, панель - не открываем
				SafeDelete(pPlugin);
				gpActivePlugin = NULL;
				return INVALID_HANDLE_VALUE;
			} break;
		case aMountHive:
			{
				LONG hRc = MFileHive::GlobalMountHive(Args.pszMountHiveFilePathName, Args.hRootMountKey, Args.pszMountHiveSubKey);
				if (hRc != 0)
					REPlugin::MessageFmt(REM_Mount_Fail, Args.pszUnmountHiveKey, hRc, _T("MountKey"));
				// В любом случае, панель - не открываем
				SafeDelete(pPlugin);
				gpActivePlugin = NULL;
				return INVALID_HANDLE_VALUE;
			} break;
		case aUnmountHive:
			{
				if (1 == REPlugin::MessageFmt(REM_Unmount_Confirm, Args.pszUnmountHiveKey, 0, _T("UnmountKey"), FMSG_WARNING, 2))
				{
					LONG hRc = MFileHive::GlobalUnmountHive(Args.hRootUnmountKey, Args.pszUnmountHiveSubKey);
					if (hRc != 0)
						REPlugin::MessageFmt(REM_Unmount_Fail, Args.pszUnmountHiveKey, hRc, _T("UnmountKey"));
				}
				// В любом случае, панель - не открываем
				SafeDelete(pPlugin);
				gpActivePlugin = NULL;
				return INVALID_HANDLE_VALUE;
			} break;
		default:
			// Ошибка разбора уже должна быть показана
			SafeDelete(pPlugin);
			gpActivePlugin = NULL;
			InvalidCmd();
			return INVALID_HANDLE_VALUE;
		}

		////TODO: Сделать нормальный разбор ком.строки
		//if (pszCommand[0]==_T('f') && pszCommand[1]==_T('i') && pszCommand[2]==_T('l')
		//	&& pszCommand[3]==_T('e') && pszCommand[4]==_T(':'))
		//{
		//	if (!pPlugin->LoadRegFile(pszCommand+5, FALSE)) {
		//		delete pPlugin;
		//		return INVALID_HANDLE_VALUE;
		//	}
		//	lbDirSet = true;
		//	
		//} else {
		//	pPlugin->ChDir(L"\\", TRUE);
		//	if (Item && *(LPCTSTR)Item) {
		//		SetDirectoryW((HANDLE)pPlugin, (LPCTSTR)Item, 0);
		//		lbDirSet = true;
		//	}
		//}
	}

	MCHKHEAP;
	
	if (!lbDirSet && cfg->pszLastRegPath && cfg->pszLastRegPath[0]) {
		#ifdef _UNICODE
			SetDirectoryW((HANDLE)pPlugin, cfg->pszLastRegPath, 0);
		#else
			int nLen = lstrlenW(cfg->pszLastRegPath)+1;
			char* pszA = (char*)malloc(nLen);
			lstrcpy_t(pszA, nLen, cfg->pszLastRegPath);
			SetDirectoryW((HANDLE)pPlugin, pszA, 0);
		#endif
		lbDirSet = true;
	}

	MCHKHEAP;

	return (HANDLE)pPlugin;
}

BOOL IsFileSupported(const TCHAR *Name, const unsigned char *Data, int DataSize, int &nFormat)
{
	if (!Name || !Data || DataSize < 4)
		return FALSE;

	nFormat = -1; // 0 - REG4, 1 - REG5, 2 - HIVE

	if (!DetectFileFormat(Data, DataSize, &nFormat, NULL, NULL))
		return FALSE;
	
	LPCTSTR pszExt = PointToExt(Name);
	// Во избежание ложных попыток открытий файла как Hive
	// требуем, чтобы у файла не было расширения
	if (nFormat == 2 && pszExt && cfg->bBrowseRegHives != 1)
		return FALSE;

	// Проверяем настройки
	if ((nFormat == 0 || nFormat == 1) && cfg->bBrowseRegFiles)
	{
		// Разрешено
	}
	else if (nFormat == 2 && cfg->bBrowseRegHives)
	{
		// Разрешено
	}
	else
	{
		// Запрещено настройками
		return FALSE;
	}
	
	return TRUE;
}

#ifdef _UNICODE
int WINAPI _export AnalyseW(const AnalyseData *pData)
{
	int nFormat = -1;

	SafeFree(gpLastAnalyse);

	if (!IsFileSupported(pData->lpwszFileName, pData->pBuffer, pData->dwBufferSize, nFormat))
		return 0;

	// Запомнить
	int nAllSize = (int)sizeof(*gpLastAnalyse) + pData->dwBufferSize;
	gpLastAnalyse = (struct AnalyseData*)malloc(nAllSize);
	*gpLastAnalyse = *pData;
	gpLastAnalyse->pBuffer = ((const unsigned char*)gpLastAnalyse) + sizeof(*gpLastAnalyse);
	memmove((void*)gpLastAnalyse->pBuffer, pData->pBuffer, pData->dwBufferSize);

	return TRUE;
}
#endif

HANDLE WINAPI _export OpenFilePluginW(const TCHAR *Name,const unsigned char *Data,int DataSize
	#ifdef _UNICODE
	,int OpMode
	#endif
	)
{
	if (!Name || !Data || DataSize < 4)
		return INVALID_HANDLE_VALUE;

	#ifdef _UNICODE
	// После AnalyseW вызывается OpenPluginW, а не OpenFilePluginW
	//SafeFree(gpLastAnalyse); -- пока нельзя. Он освобождается в OpenPluginW
	#endif

	int nFormat = -1; // 0 - REG4, 1 - REG5, 2 - HIVE
	BOOL lbSilence = FALSE;
	#ifdef _UNICODE
	lbSilence = ((OpMode & OPM_SILENT) == OPM_SILENT);
	#endif
	
	if (!IsFileSupported(Name, Data, DataSize, nFormat))
		return INVALID_HANDLE_VALUE;



	REPlugin* pPlugin = new REPlugin();
	if (!pPlugin) return INVALID_HANDLE_VALUE;
	CPluginActivator a((HANDLE)pPlugin
		#ifdef _UNICODE
		,OpMode
		#else
		,0
		#endif
		);
	
	// Сюда приходит полный путь к файлу, поэтому дублируем только в Ansi версии
	#ifdef _UNICODE
	LPCWSTR pszName = Name;
	#else
	wchar_t* pszName = lstrdup_w(Name);
	#endif

	//gnOpMode |= OPM_DISABLE_NONMODAL_EDIT;

	//gpProgress = new REProgress(GetMsg(RELoadingRegFileTitle), TRUE); -- низя, pPlugin->LoadRegFile сам прогресс создает
	//TODO: Сделать abDelayLoading (точнее сделать загрузку файла по требованию, а не при попытке открытия)
	BOOL lbLoadRc = FALSE;
	if (nFormat == 0 || nFormat == 1)
		lbLoadRc = pPlugin->LoadRegFile(pszName, lbSilence, FALSE/*abDelayLoading*/, !lbSilence);
	else
		lbLoadRc = pPlugin->LoadHiveFile(pszName, lbSilence, FALSE/*abDelayLoading*/);
	//SafeDelete(gpProgress);

	gnOpMode = 0;
	
	//WARNING!!!
	//TODO: После добавления AnalyzeW не наколоться с возвратом в FAR чтобы 
	//TODO: он не должен выполнять ShellAssociation

	#ifndef _UNICODE
	SafeFree(pszName);
	#endif
	if (!lbLoadRc)
	{
		SafeDelete(pPlugin);
		gpActivePlugin = NULL;
		// Иначе пойдет запуск RegEdit.exe по shell association
		return (HANDLE)-2;
	}
	
	return (HANDLE)pPlugin;
}

int WINAPI _export PutFilesW(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int Move,
	#ifdef _UNICODE
	const wchar_t *SrcPath,
	#endif
	int OpMode)
{
	CPluginActivator a(hPlugin,OpMode);
	REPlugin* pPlugin = (REPlugin*)hPlugin;
	REPlugin* pOpposite = gpPluginList->GetOpposite(pPlugin);
	
	if (pOpposite == NULL)
	{
		// Если с другой стороны - файловая панель
		REPlugin* pLoader = new REPlugin();
		for (int i = 0; i < ItemsNumber; i++)
		{
			wchar_t* pszFullPath = ExpandPath(FILENAMEPTR(PanelItem[i].FindData));
			if (!pszFullPath)
				continue;
			wchar_t* pszUnc = MakeUNCPath(pszFullPath);
			if (!pszUnc)
			{
				pszUnc = pszFullPath; pszFullPath = NULL;
			}

			if (!(PanelItem[i].FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				int nFormat = -1;
				if (DetectFileFormat(pszUnc, &nFormat, NULL, NULL))
				{
					if (nFormat == 0 || nFormat == 1)
					{
						// Просто импорт *.reg файла в реестр
						BOOL lbLoadRc = pLoader->LoadRegFile(pszFullPath, FALSE, FALSE, FALSE);
						if (lbLoadRc)
						{
							pLoader->ShowRegMenu(true, pPlugin->Worker());
						}
					}
				}
			}
			else
			{
				//TODO: Обработать папки как ключи?
			}
			free(pszUnc);
		}
		delete pLoader;
	}
	else
	{
		psi.Message(psi.ModuleNumber, FMSG_WARNING|FMSG_MB_OK|FMSG_ALLINONE, NULL, 
			(const TCHAR * const *)_T("RegEditor\nImporing of *.reg not implemented yet, sorry."), 2,0);
	}

	pPlugin->SetForceReload();

	return 2; // Ok, фар курсор двигать не будет
}

// Если функция выполнила свои действия успешно, то верните TRUE. В противном случае - FALSE.
int WINAPI DeleteFilesW(HANDLE hPlugin, struct PluginPanelItem *PanelItem, int ItemsNumber, int OpMode)
{
	CPluginActivator a(hPlugin,OpMode);
	// Выполняем удаление, подтверждение запрашивает класс плагина
	REPlugin* pPlugin = (REPlugin*)hPlugin;
	pPlugin->CheckItemsLoaded();
	return pPlugin->DeleteItems(PanelItem, ItemsNumber);
}

// В случае успеха возвращаемое значение должно быть равно 1.
// В случае провала возвращается 0.
// Если функция была прервана пользователем, то должно возвращаться -1. 
int WINAPI MakeDirectoryW(HANDLE hPlugin, 
	#ifdef _UNICODE
	const wchar_t **Name
	#else
	char *Name/*[NM]*/
	#endif
	,int OpMode)
{
	CPluginActivator a(hPlugin,OpMode);
	const TCHAR* pszCreatedName = NULL;
	REPlugin* pPlugin = (REPlugin*)hPlugin;
	pPlugin->CheckItemsLoaded();
	int liRc = -1;
#ifdef _UNICODE
	// Name
	//Указатель на буфер, содержащий имя каталога, передаётся плагином FAR Manager'у. Если флаг OPM_SILENT параметра OpMode не установлен, вы можете позволить пользователю изменить его, но в этом случае Name должен указывать на буфер плагина, содержащий новый каталог. Буфер должны быть валиден после возвращения из функции. 
	liRc = pPlugin->CreateSubkey(*Name, &pszCreatedName, OpMode);
	if (liRc == 1 && pszCreatedName && *pszCreatedName)
		*Name = pszCreatedName;
#else
	// Name
	//Имя каталога. Если флаг OpMode OPM_SILENT не установлен, вы можете позволить пользователю изменить его, но в этом случае новое имя должно быть скопировано в Name (максимум NM байт).
	liRc = pPlugin->CreateSubkey(Name, &pszCreatedName, OpMode);
	if (liRc == 1 && pszCreatedName && *pszCreatedName)
		lstrcpyn(Name, pszCreatedName, NM);
#endif
	// В случае успеха возвращаемое значение должно быть равно 1. В случае провала возвращается 0. Если функция была прервана пользователем, то должно возвращаться -1. 
	return liRc;
}

// ******************************
int WINAPI _export GetFilesW(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int Move,
	#ifdef _UNICODE
	const wchar_t **DestPath
	#else
	char *DestPath
	#endif
	,int OpMode)
{
	REPlugin* pPlugin = (REPlugin*)hPlugin;

	pPlugin->CheckItemsLoaded();

	if ((OpMode & OPM_FIND) == OPM_FIND)
	{
		pPlugin->mb_FindInContents = 1;

		RegItem* pItem = (RegItem*)PanelItem[0].UserData;
		if (pItem && pItem->nMagic == REGEDIT_MAGIC && pItem->ptrData)
		{
			BOOL lbRc = FALSE;
			wchar_t *pwszDestPath = NULL;
			BOOL lbUnicode = FALSE;
			#ifdef _UNICODE
			pwszDestPath = (wchar_t*)*DestPath;
			lbUnicode = TRUE;
			#else
			pwszDestPath = lstrdup_w(DestPath);
			#endif

			int nPrev = gnOpMode;
			gnOpMode = OpMode;

			MFileTxt file;
			if (file.FileCreate((LPCWSTR)pwszDestPath, pItem->pszName ? pItem->pszName : REGEDIT_DEFAULTNAME,
					L""/*asExtension*/, lbUnicode, FALSE/*abConfirmOverwrite*/))
			{
				lbRc = file.FileWriteBuffered(pItem->ptrData, pItem->nDataSize);
				file.FileClose();
			}

			gnOpMode = nPrev;

			#ifndef _UNICODE
			SafeFree(pwszDestPath);
			#endif

			return lbRc ? 1 : 0;
		}
	}

	CPluginActivator a(hPlugin,OpMode);
	BOOL lbRc = FALSE;
	
	//TODO: Если на обеих панелях открыт наш плагин - обойти экспорт в ФС
	
	//TODO: Вернуть измененный путь?
	#ifdef _UNICODE
	lbRc = pPlugin->ExportItems(PanelItem, ItemsNumber, Move, *DestPath, OpMode);
	#else
	lbRc = pPlugin->ExportItems(PanelItem, ItemsNumber, Move, DestPath, OpMode);
	#endif
	return lbRc;
}

// *************************
int WINAPI _export GetFindDataW(HANDLE hPlugin,struct PluginPanelItem **pPanelItem,int *pItemsNumber,int OpMode)
{
	CPluginActivator a(hPlugin,OpMode);
	REPlugin* pPlugin = (REPlugin*)hPlugin;
	MCHKHEAP;

	BOOL lbSilence = (OpMode & (OPM_FIND)) != 0;

	if (pPlugin->mb_ShowRegFileMenu)
	{
		if (lbSilence || cfg->bBrowseRegFiles != 1)
		{
			pPlugin->mb_ShowRegFileMenu = FALSE;
		}
		else
		{
			// Сюда мы попадаем, если юзер нажал Enter (или CtrlPgDn) на *.reg файле
			// Предложить на выбор: войти или импортировать
			MRegistryWinApi winReg;
			if (pPlugin->ShowRegMenu(false, &winReg) == -1)
				return FALSE;
		}
	}

	// Выполнит перечтение ключа только при необходимости!
	if (!pPlugin->LoadItems(lbSilence, OpMode))
		return FALSE;
		
	if (!pPlugin->mp_Items)
	{
		_ASSERTE(pPlugin->mp_Items!=NULL);
	}
	else
	{
		pPlugin->mp_Items->AddRef();
		*pItemsNumber = pPlugin->mp_Items->mn_ItemCount;
		*pPanelItem = pPlugin->mp_Items->mp_PluginItems;
	}

	MCHKHEAP;

	return TRUE;
}

void WINAPI _export FreeFindDataW(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber)
{
	CPluginActivator a(hPlugin,OPM_SILENT);
	REPlugin* pPlugin = (REPlugin*)hPlugin;
	// Плагин сам разберется, нужно ли освобождать память, или она закеширована
	pPlugin->FreeFindData(PanelItem, ItemsNumber);
	return;
}

void WINAPI _export GetOpenPluginInfoW(HANDLE hPlugin,struct OpenPluginInfo *Info)
{
	//CPluginActivator a(hPlugin,0); -- иначе все сбивается, если на обеих панелях открыт Reg2
	
	MCHKHEAP;
	REPlugin* pPlugin = (REPlugin*)hPlugin;
	_ASSERTE(pPlugin);
	_ASSERTE(pPlugin->m_Key.mpsz_Dir && pPlugin->m_Key.mpsz_Title);
	pPlugin->CheckItemsLoaded();
	Info->CurDir = pPlugin->m_Key.mpsz_Dir ? pPlugin->m_Key.mpsz_Dir : _T("");
	//Info->PanelTitle = pPlugin->m_Key.mpsz_Title ? pPlugin->m_Key.mpsz_Title : _T("");
	Info->PanelTitle = pPlugin->GetPanelTitle();
	Info->HostFile = pPlugin->mpsz_HostFile;
	Info->Flags = /*OPIF_USEFILTER|OPIF_USESORTGROUPS|*/OPIF_USEHIGHLIGHTING|OPIF_ADDDOTS;
	Info->Format = cfg->sCommandPrefix;
	Info->ShortcutData = NULL;
	Info->StartSortMode = 0;
	if (pPlugin->mb_SortingSelfDisabled)
	{
		if ((!cfg->bUnsortLargeKeys || (cfg->nLargeKeyCount == 0)) ||
			(pPlugin->mp_Items && (pPlugin->mp_Items->mn_ItemCount < cfg->nLargeKeyCount)))
		{
			// Вернуть сортировку
			pPlugin->ChangeFarSorting(false);			
		}
	}
	MCHKHEAP;

	static KeyBarTitles kbt = {{NULL}}; static TCHAR sNil[2];
	// Комбинации с F1
	kbt.ShiftTitles[0] = GetMsg(REBarShiftF1);
	// Комбинации с F2
	kbt.ShiftTitles[1] = (pPlugin->m_Key.eType == RE_REGFILE) ? GetMsg(REBarShiftF2) : (pPlugin->m_Key.eType == RE_WINAPI) ? GetMsg(REBarShiftF2Virt) : sNil; // сохранить reg
	// Комбинации с F3
	kbt.Titles[2] = GetMsg(REBarF3);
	//kbt.AltTitles[2] = GetMsg(REBarAltF3);
	kbt.AltShiftTitles[2] = GetMsg(REBarAltShiftF3);
	kbt.ShiftTitles[2] = (pPlugin->m_Key.eType == RE_WINAPI) ? GetMsg(REBarShiftF3) : sNil; // переключить x32/x64
	// Комбинации с F4
	kbt.Titles[3] = GetMsg(REBarF4);
	kbt.AltTitles[3] = GetMsg(REBarAltF4);
	kbt.AltShiftTitles[3] = GetMsg(REBarAltShiftF4);
	kbt.CtrlAltTitles[3] = GetMsg(REBarCtrlShiftF4);
	kbt.ShiftTitles[3] = GetMsg(REBarShiftF4);
	// Комбинации с F5
	kbt.Titles[4] = GetMsg(REBarF5);
	kbt.ShiftTitles[4] = GetMsg(REBarShiftF5);
	// Комбинации с F6
	kbt.Titles[5] = GetMsg(REBarF5);
	kbt.ShiftTitles[5] = GetMsg(REBarShiftF6);
	// Комбинации с F7
	kbt.Titles[6] = GetMsg(REBarF7);
	kbt.AltShiftTitles[6] = GetMsg(REBarAltShiftF7);
	kbt.ShiftTitles[6] = GetMsg(REBarShiftF7);
	// Fin
	Info->KeyBar = &kbt;
}

void WINAPI _export ClosePluginW(HANDLE hPlugin)
{
	//CPluginActivator a(hPlugin,0); -- не надо, а то деструктор активатора пытается в pPlugin смотреть
	REPlugin* pPlugin = (REPlugin*)hPlugin;
	gpActivePlugin = pPlugin;
	pPlugin->PreClosePlugin(); // предложить сохранить .reg файл
	MCHKHEAP;
	SafeDelete(pPlugin);
	MCHKHEAP;
	gpActivePlugin = NULL;
}


int WINAPI _export ConfigureW(int ItemNumber)
{
	MCHKHEAP;
	if (!cfg) {
		_ASSERTE(cfg!=NULL);
		return FALSE;
	}
	// Возвращает TRUE, если нужно обновить панели
	int nRc = cfg->Configure();
	MCHKHEAP;
	return nRc;
}

int WINAPI _export ProcessEventW(HANDLE hPlugin, int Event, void *Param)
{
	if (Event == FE_IDLE) {
		CPluginActivator a(hPlugin,0);
		REPlugin* pPlugin = (REPlugin*)hPlugin;
		pPlugin->CheckItemsLoaded();
		pPlugin->OnIdle();
	}
	return FALSE;
}

#ifdef _UNICODE
int WINAPI ProcessSynchroEventW(int Event, void *Param)
{
	if (Event == SE_COMMONSYNCHRO && Param)
	{
		SynchroArg* pArg = (SynchroArg*)Param;
		if (pArg->nEvent == REGEDIT_SYNCHRO_DESC_FINISHED)
		{
			_ASSERTE(pArg->pPlugin!=NULL);
			if (gpPluginList->IsValid(pArg->pPlugin))
			{
				CPluginActivator a((HANDLE)pArg->pPlugin,0);
				pArg->pPlugin->CheckItemsLoaded();
				pArg->pPlugin->OnIdle();
			}
		}
	}
	return 0;
}
#endif

int WINAPI CompareW(HANDLE hPlugin, const struct PluginPanelItem *Item1, const struct PluginPanelItem *Item2, unsigned int Mode)
{
	// для скорости - не ставим: CPluginActivator a(hPlugin,0);
	REPlugin* pPlugin = (REPlugin*)hPlugin;
	return pPlugin->Compare(Item1, Item2, Mode);
}

int WINAPI ProcessEditorEventW(int Event, void *Param)
{
	// Если редактируется Sequence макроса - попытаемся его проверить :)
	if (gpActivePlugin)
	{
		if (Event == EE_GOTFOCUS)
		{
			if (MFileTxt::bBadMszDoubleZero)
			{
				MFileTxt::bBadMszDoubleZero = FALSE;
				REPlugin::Message(REM_BadDoubleZero);
				// Финт ушами, чтобы установить в редакторе статус "Изменен"
				psi.EditorControl(ECTL_INSERTTEXT,_T(" \x8"));
			}
		}
		#ifdef _UNICODE
		else if (gpActivePlugin->mb_InMacroEdit && Event == EE_SAVE)
		{
			EditorInfo ei = {0};
			int nLen = psi.EditorControl(ECTL_GETINFO, &ei);
			if (ei.TotalLines > 0 && ei.CodePage != 1201) // Reverse unicode - не поддерживаем
			{
				//wchar_t* pszFilePathName = NULL;
				//nLen = psi.EditorControl(ECTL_GETFILENAME, NULL);
				//if (nLen > 0 && (pszFilePathName = (wchar_t*)calloc(nLen+1,2)))
				//{
				//	nLen = psi.EditorControl(ECTL_GETFILENAME, pszFilePathName);
				//	LPCWSTR pszFileName = PointToName(pszFilePathName);
				//	//TODO: Хорошо бы еще и на родительский ключ смотреть,
				//	//TODO: ибо для Vars/Common имена НЕ "Sequence", а проверить бы хотелось
				//	if (pszFileName && lstrcmpiW(pszFileName, L"Sequence") == 0)
				//	{
						// Проверяем. Сначала нужно получить сам текст макроса.
						wchar_t *pszMacro = NULL, *psz = NULL;
						int *pnStart = (int*)calloc(ei.TotalLines,sizeof(int));
						size_t nMaxLen = 0, nAllLen = 0; int nMaxLineLen = 2048;
						EditorGetString egs;
						egs.StringNumber = 0;
						psi.EditorControl(ECTL_GETSTRING,&egs);
						if ((egs.StringLength*2+3) > nMaxLineLen)
							nMaxLineLen = egs.StringLength*2+3;
						nMaxLen = nMaxLineLen * ei.TotalLines;
						pszMacro = (wchar_t*)calloc(nMaxLen,2);
						if (!pszMacro)
						{
							InvalidOp(); // не хватило памяти
						} else {
							psz = pszMacro;
							int nLine = 0;
							for (egs.StringNumber = 0; egs.StringNumber < ei.TotalLines; egs.StringNumber++)
							{
								// первая строка (==0) уже получена
								if (egs.StringNumber) psi.EditorControl(ECTL_GETSTRING,&egs);
								// Памяти хватает?
								if ((nAllLen + egs.StringLength + 2) >= nMaxLen)
								{
									// Нужно выделить еще памяти
									nMaxLen = nMaxLen + min(16384,nMaxLen);
									wchar_t* pszNew = (wchar_t*)calloc(nMaxLen,2);
									if (!pszNew)
									{
										psz = NULL; break;
									}
								}
								// добавляем
								if (ei.CodePage == 1200)
								{
									wmemmove(psz, egs.StringText, egs.StringLength);								
								} else {
								}
								pnStart[nLine++] = nAllLen;
								psz += egs.StringLength; *(psz++) = L'\n'; *psz = 0;
								nAllLen += (egs.StringLength + 1);
							}
							// Проверяем, хватило ли памяти, все ли считано
							if (psz)
							{
								// Можно проверять макрос
								ActlKeyMacro mcr = {MCMD_CHECKMACRO};
								mcr.Param.PlainText.SequenceText = pszMacro;
								mcr.Param.PlainText.Flags = KSFLAGS_SILENTCHECK;
								
								psi.AdvControl(psi.ModuleNumber, ACTL_KEYMACRO, &mcr);
								
								//if (!psi.AdvControl(psi.ModuleNumber, ACTL_KEYMACRO, &mcr))
								//{
								//	REPlugin::Message(REM_MPEC_INTPARSERERROR);
								//}							
								//else
								if (mcr.Param.MacroResult.ErrCode == MPEC_INTPARSERERROR || mcr.Param.MacroResult.ErrCode > MPEC_CONTINUE_OTL)
								{
									REPlugin::Message(REM_MPEC_INTPARSERERROR);
								}
								else
								if (mcr.Param.MacroResult.ErrCode != 0)
								{
									// Ошибка!
									EditorSetPosition esp = {mcr.Param.MacroResult.ErrPos.Y, 0, -1, -1, -1, -1};
									psi.EditorControl(ECTL_SETPOSITION, &esp);
									int nMsgId = REM_MPEC_UNRECOGNIZED_KEYWORD + (mcr.Param.MacroResult.ErrCode - MPEC_UNRECOGNIZED_KEYWORD);
									LPCWSTR pszMsgFormat = GetMsg(nMsgId);
									if (pszMsgFormat)
									{
										int nLine = mcr.Param.MacroResult.ErrPos.Y, nCol = mcr.Param.MacroResult.ErrPos.X;
										//if (nLine > 0 && nLine < ei.TotalLines)
										//{
										//	if (nCol >= pnStart[nLine])
										//		nCol -= pnStart[nLine];
										//}
										//for (int i = ei.TotalLines-1; i > 0; i--)
										//{
										//	if (pnStart[i] <= 
										//}
										nLen = mcr.Param.MacroResult.ErrSrc ? lstrlen(mcr.Param.MacroResult.ErrSrc) : 0;
										nLen += lstrlen(pszMsgFormat) + 64;
										psz = (wchar_t*)calloc(nLen, 2);
										if (wcsstr(pszMsgFormat, L"%s"))
											wsprintfW(psz, pszMsgFormat, mcr.Param.MacroResult.ErrSrc ? mcr.Param.MacroResult.ErrSrc : L"", nLine+1, nCol+1);
										else
											wsprintfW(psz, pszMsgFormat, nLine+1, nCol+1);
										REPlugin::Message(psz);
										SafeFree(psz);
									} else {
										REPlugin::Message(REM_MPEC_INTPARSERERROR);
									}
								}
							}
						}
						SafeFree(pszMacro);
						SafeFree(pnStart);
				//	}
				//	SafeFree(pszFilePathName);
				//}
			}
		}
		#endif
	}
	return 0;
}


// FALSE для дальнейшей обработки FAR.
// TRUE, если плагин самостоятельно обработал клавишу. 
int WINAPI _export ProcessKeyW(HANDLE hPlugin,int Key,unsigned int ControlState)
{
	CPluginActivator a(hPlugin,0);

	if ((Key & 0xFF) == VK_F1)
	{
		REPlugin* pPlugin = (REPlugin*)hPlugin;
		pPlugin->CheckItemsLoaded();
		// ShiftF1 - Отобразить и перейти на закладку
		if (ControlState == PKF_SHIFT)
		{
			pPlugin->ShowBookmarks();
			return TRUE;
		}
	} // if ((Key & 0xFF) == VK_F1)
	if ((Key & 0xFF) == VK_F2)
	{
		REPlugin* pPlugin = (REPlugin*)hPlugin;
		pPlugin->CheckItemsLoaded();
		// ShiftF2 - Save opened *.reg file
		if (ControlState == PKF_SHIFT)
		{
			if (pPlugin->m_Key.eType == RE_REGFILE)
			{
				pPlugin->SaveRegFile();
			}
			#ifdef _DEBUG
			else if (pPlugin->m_Key.eType == RE_WINAPI)
			{
				cfg->bVirtualize = !cfg->bVirtualize;
				pPlugin->SetForceReload();
				pPlugin->UpdatePanel(false);
				pPlugin->RedrawPanel();
			}
			#endif
			return TRUE;
		}
	} // if ((Key & 0xFF) == VK_F2)
	else if (((Key & 0xFF) == VK_F3) || ((Key & 0xFF) == VK_CLEAR))
	{
		REPlugin* pPlugin = (REPlugin*)hPlugin;
		pPlugin->CheckItemsLoaded();
		// F3 - авто редактирование (DWORD/String)
		if (ControlState == 0 /*|| ControlState == PKF_ALT*/)
		{
			pPlugin->EditItem(true, true, true);
			return TRUE;
		}
		// AltShiftF3 - редактирование в *.reg ВЫДЕЛЕННЫХ элементов (значений или ключей)
		else if (ControlState == (PKF_ALT|PKF_SHIFT))
		{
			pPlugin->EditItem(false, true, true);
			return TRUE;
		}
		// CtrlShiftF3 - открыть во встроенном просмотрщике RAW данные (Unicode only)
		else if (ControlState == (PKF_CONTROL|PKF_SHIFT))
		{
			pPlugin->EditItem(true/*abOnlyCurrent*/, false/*abForceExport*/, true/*abViewOnly*/, true/*abRawData*/);
			return TRUE;
		}
		// ShiftF3 - переключение [32]<-->[64] битный реестр
		else if (ControlState == PKF_SHIFT)
		{
			// Кнопку перехватываем всегда, а вот опцию дергаем только в x64 OS
			//if (cfg->is64bitOs)
			{
				//cfg->bWow64on32 = !cfg->bWow64on32;
				if (cfg->bWow64on32 == 0) {
					cfg->bWow64on32 = 1;
				} else if (cfg->bWow64on32 == 1) {
					cfg->bWow64on32 = 2;
				} else {
					cfg->bWow64on32 = 0;
				}
				pPlugin->SetForceReload();
				pPlugin->UpdatePanel(false);
				pPlugin->RedrawPanel();
			}
			return TRUE;
		}
	} // if ((Key & 0xFF) == VK_F3)
	else if ((Key & 0xFF) == VK_F4)
	{
		REPlugin* pPlugin = (REPlugin*)hPlugin;
		pPlugin->CheckItemsLoaded();
		// F4 - авто редактирование (DWORD/String)
		if (ControlState == 0)
		{
			pPlugin->EditItem(true, false, false);
			return TRUE;
		}
		// AltF4 - редактирование в *.reg текущего элемента
		else if (ControlState == PKF_ALT)
		{
			pPlugin->EditItem(true, true, false);
			return TRUE;
		}
		// AltShiftF4 - редактирование в *.reg ВЫДЕЛЕННЫХ элементов (значений или ключей)
		else if (ControlState == (PKF_ALT|PKF_SHIFT))
		{
			pPlugin->EditItem(false, true, false);
			return TRUE;
		}
		// CtrlShiftF4 - открыть во встроенном редакторе RAW данные (Unicode only)
		else if (ControlState == (PKF_CONTROL|PKF_SHIFT))
		{
			pPlugin->EditItem(true/*abOnlyCurrent*/, false/*abForceExport*/, false/*abViewOnly*/, true/*abRawData*/);
			return TRUE;
		}
		// ShiftF4 - создать новое значение
		else if (ControlState == PKF_SHIFT)
		{
			pPlugin->NewItem();
			return TRUE;
		}
		// CtrlAltF4 - права
		else if (ControlState == (PKF_CONTROL|PKF_ALT))
		{
			MCHKHEAP;
			pPlugin->EditKeyPermissions();
			MCHKHEAP;
			return TRUE;
		}
	} // if ((Key & 0xFF) == VK_F4)
	else if ((Key & 0xFF) == VK_F5)
	{
		// ShiftF5 - Сделать копию ключа реестра или значения (аналогично ShiftF6)
		if (ControlState == PKF_SHIFT)
		{
			REPlugin* pPlugin = (REPlugin*)hPlugin;
			pPlugin->CheckItemsLoaded();
			pPlugin->RenameOrCopyItem(TRUE/*abCopyOnly*/);
			return TRUE;
		}
	} // if ((Key & 0xFF) == VK_F5)
	else if ((Key & 0xFF) == VK_F6)
	{
		// ShiftF6 - переименовать значение или ключ
		if (ControlState == PKF_SHIFT)
		{
			REPlugin* pPlugin = (REPlugin*)hPlugin;
			pPlugin->CheckItemsLoaded();
			pPlugin->RenameOrCopyItem(FALSE/*abCopyOnly*/);
			return TRUE;
		}
	} // if ((Key & 0xFF) == VK_F6)
	else if ((Key & 0xFF) == VK_F7)
	{
		// AltShiftF7 - Подключиться к удаленной машине
		if (ControlState == (PKF_ALT|PKF_SHIFT))
		{
			REPlugin* pPlugin = (REPlugin*)hPlugin;
			pPlugin->ConnectRemote();
			return TRUE;
		}
		// ShiftF7
		else if (ControlState == PKF_SHIFT)
		{
			REPlugin* pPlugin = (REPlugin*)hPlugin;
			cfg->bShowKeysAsDirs = !cfg->bShowKeysAsDirs;
			pPlugin->SetForceReload();
			pPlugin->UpdatePanel(false);
			pPlugin->RedrawPanel();
			return TRUE;
		}
	} // if ((Key & 0xFF) == VK_F7)
	//else if ((Key & 0xFF) == 'A')
	//{
	//	// CtrlA - Права на ключ (пока только просмотр)
	//	if (ControlState == PKF_CONTROL)
	//	{
	//		REPlugin* pPlugin = (REPlugin*)hPlugin;
	//		MCHKHEAP;
	//		pPlugin->EditKeyPermissions();
	//		MCHKHEAP;
	//		return TRUE;
	//	}
	//} // if ((Key & 0xFF) == 'A')
	else if ((Key & 0xFF) == 'J')
	{
		// CtrlJ - Открыть текущий ключ в RegEdit.exe, аналог ShiftEnter
		if (ControlState == PKF_CONTROL)
		{
			REPlugin* pPlugin = (REPlugin*)hPlugin;
			pPlugin->CheckItemsLoaded();
			pPlugin->JumpToRegedit();
			return TRUE; // Обработали
		}
	} // if ((Key & 0xFF) == 'J')
	else if ((Key & 0xFF) == 'R')
	{
		// CtrlR - сбросить кеш текущего ключа
		if (ControlState == PKF_CONTROL)
		{
			REPlugin* pPlugin = (REPlugin*)hPlugin;
			MCHKHEAP;
			pPlugin->SetForceReload();
			MCHKHEAP;
			return FALSE; //  пусть Фар перечитает сам
		}
	} // if ((Key & 0xFF) == 'R')
	else if ((Key & 0xFF) == 'Z')
	{
		// CtrlZ - редактировать в стиле "Diz"
		if (ControlState == PKF_CONTROL)
		{
			REPlugin* pPlugin = (REPlugin*)hPlugin;
			pPlugin->CheckItemsLoaded();
			pPlugin->EditDescription();
			return TRUE; // Обработали
		}
	} // if ((Key & 0xFF) == 'Z')
	else if (Key == VK_RETURN)
	{
		// ShiftEnter - Открыть текущий ключ в RegEdit.exe, аналог CtrlJ
		if (ControlState == PKF_SHIFT)
		{
			REPlugin* pPlugin = (REPlugin*)hPlugin;
			pPlugin->CheckItemsLoaded();
			pPlugin->JumpToRegedit();
			return TRUE; // Обработали
		}
	} // if ((Key & 0xFF) == VK_RETURN)
	
	// Это условие стоит особняком, т.к. проверяется не просто кнопка - а еще и варианты...
	// Вобщем, чтобы не наколоться с Else...
	if (!cfg->bShowKeysAsDirs // лучше заход в папку обрабатывать самим - это гарантирует корректную обработку "левых" имен
		&& ((((Key /*& 0xFF*/) == VK_RETURN) && ControlState == 0)
			|| (((Key & 0xFF) == VK_NEXT) && ControlState == PKF_CONTROL)))
	{
		REPlugin* pPlugin = (REPlugin*)hPlugin;
		pPlugin->CheckItemsLoaded();
		RegItem* p = pPlugin->GetCurrentItem();
		if (p && p->nValueType == REG__KEY && p->pszName)
		{
			_ASSERTE(wcschr(p->pszName,L'\\') == NULL);
			if (pPlugin->SetDirectory(p, FALSE))
			{
				pPlugin->SetForceReload();
				pPlugin->UpdatePanel(false);
				pPlugin->RedrawPanel((RegItem*)1);
			}
			return TRUE;
		}
	} // Enter или CtrlPgDn в режиме "Ключи как файлы"
	
	return FALSE; // для дальнейшей обработки FAR.
}
