
#include "header.h"
// Helpers
#include "TokenHelper.h"

#pragma comment(lib, "version.lib")

RegConfig *cfg = NULL;
extern DWORD gnFarVersion;

typedef BOOL (WINAPI* IsWow64Process_t)(HANDLE hProcess, PBOOL Wow64Process);

static LONG GetString(HKEY hk, LPCWSTR asName, wchar_t* pszValue, DWORD cCount)
{
	LONG nRegRc; DWORD nSize;
	wchar_t sValue[128] = {0};
	_ASSERTE(cCount<=128);
	
	nSize = sizeof(sValue)-2;
	nRegRc = RegQueryValueExW(hk, asName, NULL, NULL, (LPBYTE)sValue, &nSize);
	if (nRegRc == 0) {
		lstrcpynW(pszValue, sValue, cCount);
	} else {
		if (cfg) cfg->bSomeValuesMissed = TRUE;
	}
	MCHKHEAP;

	return nRegRc;
}

#ifndef _UNICODE
static LONG GetString(HKEY hk, LPCWSTR asName, char* pszValue, DWORD cCount)
{
	LONG nRegRc; DWORD nSize;
	wchar_t sValue[128] = {0};
	_ASSERTE(cCount<=128);
	
	nSize = sizeof(sValue)-2;
	nRegRc = RegQueryValueExW(hk, asName, NULL, NULL, (LPBYTE)sValue, &nSize);
	if (nRegRc == 0) {
		lstrcpy_t(pszValue, cCount, sValue);
	} else {
		if (cfg) cfg->bSomeValuesMissed = TRUE;
	}
	MCHKHEAP;

	return nRegRc;
}
#endif

static LONG GetString(HKEY hk, LPCWSTR asName, wchar_t** pszValue)
{
	LONG nRegRc; DWORD nSize = 0;
	
	nRegRc = RegQueryValueExW(hk, asName, NULL, NULL, NULL, &nSize);
	if (nRegRc == 0) {
		_ASSERTE(*pszValue == NULL);
		*pszValue = (wchar_t*)calloc(nSize+2,1);
		nRegRc = RegQueryValueExW(hk, asName, NULL, NULL, (LPBYTE)*pszValue, &nSize);
	} else {
		if (cfg) cfg->bSomeValuesMissed = TRUE;
	}
	MCHKHEAP;
	
	return nRegRc;
}

static LONG SetString(HKEY hk, LPCWSTR asName, const wchar_t* pszValue)
{
	LONG nRegRc; DWORD nSize;
	
	nSize = (lstrlenW(pszValue)+1)*2;
	nRegRc = RegSetValueExW(hk, asName, 0, REG_SZ, (LPBYTE)pszValue, nSize);
	
	return nRegRc;
}

#ifndef _UNICODE
static LONG SetString(HKEY hk, LPCTSTR asName, const char* pszValue)
{
	LONG nRegRc; DWORD nSize;
	
	nSize = (lstrlenA(pszValue)+1);
	nRegRc = RegSetValueExA(hk, asName, 0, REG_SZ, (LPBYTE)pszValue, nSize);
	
	return nRegRc;
}
#endif

static LONG GetDWORD(HKEY hk, LPCWSTR asName, DWORD* pnValue)
{
	LONG nRegRc; DWORD nSize, nValue = 0;

	nSize = sizeof(nValue);
	nRegRc = RegQueryValueExW(hk, asName, NULL, NULL, (LPBYTE)&nValue, &nSize);
	if (nRegRc == 0) {
		*pnValue = nValue;
	} else {
		if (cfg) cfg->bSomeValuesMissed = TRUE;
	}
	
	return nRegRc;
}

static LONG SetDWORD(HKEY hk, LPCWSTR asName, DWORD nValue)
{
	LONG nRegRc; DWORD nSize;

	nSize = sizeof(nValue);
	nRegRc = RegSetValueExW(hk, asName, 0, REG_DWORD, (LPBYTE)&nValue, nSize);
	
	return nRegRc;
}

static LONG GetBool(HKEY hk, LPCWSTR asName, BOOL* pbValue)
{
	LONG nRegRc; DWORD nSize; BYTE nValue = 0;

	nSize = sizeof(nValue);
	nRegRc = RegQueryValueExW(hk, asName, NULL, NULL, (LPBYTE)&nValue, &nSize);
	if (nRegRc == 0) {
		*pbValue = (nValue != 0);
	} else {
		if (cfg) cfg->bSomeValuesMissed = TRUE;
	}
	
	return nRegRc;
}

static LONG SetBool(HKEY hk, LPCWSTR asName, BOOL bValue)
{
	LONG nRegRc; DWORD nSize;
	BYTE b = (BYTE)(bValue & 0xFF);

	nSize = sizeof(b);
	nRegRc = RegSetValueExW(hk, asName, 0, REG_BINARY, (LPBYTE)&b, nSize);
	
	return nRegRc;
}

static LONG GetByte(HKEY hk, LPCWSTR asName, BOOL* pbValue)
{
	LONG nRegRc; DWORD nSize; BYTE nValue = 0;

	nSize = sizeof(nValue);
	nRegRc = RegQueryValueExW(hk, asName, NULL, NULL, (LPBYTE)&nValue, &nSize);
	if (nRegRc == 0) {
		*pbValue = nValue;
	} else {
		if (cfg) cfg->bSomeValuesMissed = TRUE;
	}

	return nRegRc;
}

static LONG SetByte(HKEY hk, LPCWSTR asName, BOOL bValue)
{
	LONG nRegRc; DWORD nSize;
	BYTE b = (BYTE)(bValue & 0xFF);

	nSize = sizeof(b);
	nRegRc = RegSetValueExW(hk, asName, 0, REG_BINARY, (LPBYTE)&b, nSize);

	return nRegRc;
}

void RegConfig::LoadVersionInfo(LPCTSTR asModuleName)
{
	BOOL lbRc=FALSE;
	WCHAR ErrText[512]; ErrText[0] = 0; //DWORD dwErr = 0;
	sVersionInfo[0] = 0;

	DWORD nFarVer = 0;
	psi.AdvControl(psi.ModuleNumber, ACTL_GETFARVERSION, (void*)&nFarVer);
	wsprintf(sVersionInfo, _T("Far %i.%i.%i, RegEdit "), 
		(UINT)HIBYTE(LOWORD(nFarVer)), (UINT)LOBYTE(LOWORD(nFarVer)), (UINT)HIWORD(nFarVer));

	DWORD dwRsrvd = 0;
	DWORD dwSize = GetFileVersionInfoSize(asModuleName, &dwRsrvd);
	if (dwSize>0) {
		void *pVerData = calloc(dwSize, 1);
		if (pVerData) {
			VS_FIXEDFILEINFO *lvs = NULL;
			UINT nLen = sizeof(lvs);
			if (GetFileVersionInfo(asModuleName, 0, dwSize, pVerData)) {
				TCHAR szSlash[3]; lstrcpy(szSlash, _T("\\"));
				if (VerQueryValue ((void*)pVerData, szSlash, (void**)&lvs, &nLen)) {
					wsprintf(sVersionInfo+lstrlen(sVersionInfo),
						_T("%i.%i.%i.%i"),
						HIWORD(lvs->dwFileVersionMS), LOWORD(lvs->dwFileVersionMS),
						HIWORD(lvs->dwFileVersionLS), LOWORD(lvs->dwFileVersionLS)
					);
					lbRc = TRUE;
				}
			}
			SafeFree(pVerData);
		}
	}

	if (!lbRc)
		lstrcat(sVersionInfo, _T("<Unknown>"));

	lstrcat(sVersionInfo, 
		#ifdef _WIN64
			_T(" x64")
		#else
			_T(" x86")
		#endif
		#ifdef _UNICODE
			_T("W")
		#else
			_T("A")
		#endif
		);
}

void RegConfig::Init(LPCTSTR asRootKey, LPCTSTR asModuleName)
{
	if (cfg) return;
	cfg = new RegConfig(asRootKey, asModuleName);
	cfg->LoadConfiguration();
}

RegConfig::RegConfig(LPCTSTR asRootKey, LPCTSTR asModuleName)
{
	size_t nLen = _tcslen(asRootKey);
	pszPluginKey = (TCHAR*)malloc((nLen+16)*sizeof(TCHAR));
	lstrcpy(pszPluginKey, asRootKey);
	lstrcat(pszPluginKey, _T("\\RegEditor"));

	sVersionInfo[0] = 0;
	LoadVersionInfo(asModuleName);

	mp_Token = NULL; mb_TokenWarnedOnce = FALSE; mn_TokenRef = 0; mb_RestoreAquired = FALSE;

	// Значения по умолчанию!	
	bWow64on32 = 2; // 2-Auto. 1-Отображать 64-битный реестр, 0-32-битный
	bVirtualize = FALSE;
	lstrcpy(sCommandPrefix, _T("reg2"));
	lstrcpy(sRegTitlePrefix, _T("REG2"));
	lstrcpy(sRegTitleDirty, _T(" (*)"));
	bAddToDisksMenu = FALSE;
	cDiskMenuHotkey[0] = 0; cDiskMenuHotkey[1] = 0;
	bAddToPluginsMenu = TRUE;
	bBrowseRegFiles = TRUE;
	bBrowseRegHives = 1;
	bSkipAccessDeniedMessage = FALSE; // 1-не показывать ошибку, 0-показывать ошибку
	bUseBackupRestore = TRUE; // пытаться повысить привилегии до BackupRestore при ошибках доступа
	bSkipPrivilegesDeniedMessage = TRUE;
	bShowKeysAsDirs = TRUE;
	bUnsortLargeKeys = TRUE; nLargeKeyCount = 1000; nLargeKeyTimeout = 300;
	#ifdef _UNICODE
	bCreateUnicodeFiles = TRUE;
	#else
	bCreateUnicodeFiles = FALSE;
	#endif
	bEscapeRNonExporting = TRUE;
	bCheckMacroSequences = bCheckMacrosInVars = FALSE;
	nAnsiCodePage = 1251;
	bEditBinaryAsText = TRUE;
	bRefreshChanges = TRUE; // 0-только по CtrlR, 1-автоматически
	bLoadDescriptions = TRUE;
	bExportDefaultValueFirst = FALSE;
	nRefreshKeyTimeout = 1000;
	nRefreshSubkeyTimeout = 10000;
	pszLastRegPath = NULL; nMaxRegPath = 0;
	bConfirmImport = TRUE; bShowImportResult = TRUE;
	
	bSomeValuesMissed = FALSE;
	
	bUseInternalBookmarks = TRUE;
	szBookmarksValueName[0] = 0;
	cchBookmarksMaxCount = cchBookmarksLen = 0;
	pszBookmarks = NULL;
	
	#ifdef _WIN64
	is64bitOs = TRUE; isWow64process = FALSE;
	#else
	// Проверяем, где мы запущены
	isWow64process = FALSE;
	HMODULE hKernel = GetModuleHandle(_T("kernel32.dll"));
	if (hKernel) {
		IsWow64Process_t IsWow64Process_f = (IsWow64Process_t)GetProcAddress(hKernel, "IsWow64Process");
		if (IsWow64Process_f) {
			BOOL bWow64 = FALSE;
			if (IsWow64Process_f(GetCurrentProcess(), &bWow64) && bWow64) {
				isWow64process = TRUE;
			}
		}
	}
	is64bitOs = isWow64process;
	#endif
}

RegConfig::~RegConfig()
{
	SafeFree(pszPluginKey);
	SafeFree(pszLastRegPath);
	nMaxRegPath = 0;
	if (mp_Token)
	{
		mn_TokenRef = 0;
		BackupPrivilegesRelease();
	}
	SafeFree(pszBookmarks);
}

void RegConfig::LoadConfiguration()
{
	HKEY hk;
	bSomeValuesMissed = FALSE;
	if (0 != RegOpenKeyEx(HKEY_CURRENT_USER, pszPluginKey, 0, KEY_READ, &hk))
	{
		// Если ключа не было - сразу создать в реестре значения по умолчанию!
		bSomeValuesMissed = TRUE;
		
	} else {
		GetByte(hk, L"Wow64on32", &bWow64on32); // Отображать 64-битный реестр в FAR.x32
		GetByte(hk, L"Virtualize", &bVirtualize); // Отображать виртуализированные значения реестра
		GetString(hk, L"CommandPrefix", sCommandPrefix, countof(sCommandPrefix)); // _T("reg2")
		GetString(hk, L"RegTitlePrefix", sRegTitlePrefix, countof(sRegTitlePrefix)); // _T("REG2")
		GetString(hk, L"RegTitleDirty", sRegTitleDirty, countof(sRegTitleDirty)); // _T(" (*)")
		GetBool(hk, L"AddToDisksMenu", &bAddToDisksMenu);
		GetString(hk, L"DiskMenuHotkey", cDiskMenuHotkey, countof(cDiskMenuHotkey));
		GetBool(hk, L"AddToPluginsMenu", &bAddToPluginsMenu);
		GetBool(hk, L"BrowseRegFiles", &bBrowseRegFiles);
		GetByte(hk, L"BrowseRegHives", &bBrowseRegHives);
		GetBool(hk, L"SkipAccessDeniedMessage", &bSkipAccessDeniedMessage);
		GetBool(hk, L"UseBackupRestore", &bUseBackupRestore);
		GetBool(hk, L"SkipPrivilegesDeniedMessage", &bSkipPrivilegesDeniedMessage);
		GetBool(hk, L"ShowKeysAsDirs", &bShowKeysAsDirs);
		GetBool(hk, L"UnsortLargeKeys", &bUnsortLargeKeys);
		GetDWORD(hk, L"LargeKeyCount", &nLargeKeyCount);
		GetDWORD(hk, L"LargeKeyTimeout", &nLargeKeyTimeout);
		GetBool(hk, L"CreateUnicodeFiles", &bCreateUnicodeFiles);
		GetDWORD(hk, L"AnsiCodePage", &nAnsiCodePage); if (nAnsiCodePage == CP_UTF7 || nAnsiCodePage == CP_UTF8) nAnsiCodePage = 1251;
		GetBool(hk, L"EscapeRNonExporting", &bEscapeRNonExporting);
		GetBool(hk, L"CheckMacroSequences", &bCheckMacroSequences);
		GetBool(hk, L"CheckMacrosInVars", &bCheckMacrosInVars);
		GetBool(hk, L"EditBinaryAsText", &bEditBinaryAsText);
		GetBool(hk, L"RefreshChanges", &bRefreshChanges);
		GetBool(hk, L"LoadDescriptions", &bLoadDescriptions);
		GetBool(hk, L"ExportDefaultValueFirst", &bExportDefaultValueFirst);
		GetBool(hk, L"ConfirmImport", &bConfirmImport);
		GetBool(hk, L"ShowImportResult", &bShowImportResult);
		GetDWORD(hk, L"RefreshKeyTimeout", &nRefreshKeyTimeout);
		GetDWORD(hk, L"RefreshSubkeyTimeout", &nRefreshSubkeyTimeout);
		GetString(hk, L"LastRegPath", &pszLastRegPath);
		nMaxRegPath = (pszLastRegPath == 0) ? 0 : (lstrlenW(pszLastRegPath)+1);
		
		RegCloseKey(hk);
	}
	
	// Если ключа не было, или в плагине появились новые параметры - сразу сохранить наши текущие настройки (умолчания)
	if (bSomeValuesMissed) {
		SaveConfiguration();
	}
}

void RegConfig::SaveConfiguration()
{
	// Некоторые проверки
	if (nAnsiCodePage == CP_UTF7 || nAnsiCodePage == CP_UTF8) nAnsiCodePage = 1251;
	
	// Сохраняем
	HKEY hk; DWORD dwDisp = 0;
	bSomeValuesMissed = FALSE;
	if (0 == RegCreateKeyEx(HKEY_CURRENT_USER, pszPluginKey, 0, NULL, 0, KEY_ALL_ACCESS, NULL, &hk, &dwDisp))
	{
		SetByte(hk, L"Wow64on32", bWow64on32); // Отображать 64-битный реестр в FAR.x32
		SetByte(hk, L"Virtualize", bVirtualize);// отображать виртуализированные значения реестра
		SetString(hk, _T("CommandPrefix"), sCommandPrefix); // _T("reg2")
		SetString(hk, _T("RegTitlePrefix"), sRegTitlePrefix); // _T("REG2")
		SetString(hk, _T("RegTitleDirty"), sRegTitleDirty); // _T(" (*)")
		SetBool(hk, L"AddToDisksMenu", bAddToDisksMenu);
		SetString(hk, _T("DiskMenuHotkey"), cDiskMenuHotkey); _ASSERTE(cDiskMenuHotkey[1] == 0);
		SetBool(hk, L"AddToPluginsMenu", bAddToPluginsMenu);
		SetBool(hk, L"BrowseRegFiles", bBrowseRegFiles);
		SetByte(hk, L"BrowseRegHives", bBrowseRegHives);
		SetBool(hk, L"SkipAccessDeniedMessage", bSkipAccessDeniedMessage);
		SetBool(hk, L"UseBackupRestore", bUseBackupRestore);
		SetBool(hk, L"SkipPrivilegesDeniedMessage", bSkipPrivilegesDeniedMessage);
		SetBool(hk, L"ShowKeysAsDirs", bShowKeysAsDirs);
		SetBool(hk, L"UnsortLargeKeys", bUnsortLargeKeys);
		SetDWORD(hk, L"LargeKeyCount", nLargeKeyCount);
		SetDWORD(hk, L"LargeKeyTimeout", nLargeKeyTimeout);
		SetBool(hk, L"CreateUnicodeFiles", bCreateUnicodeFiles);
		SetDWORD(hk, L"AnsiCodePage", nAnsiCodePage);
		SetBool(hk, L"EscapeRNonExporting", bEscapeRNonExporting);
		SetBool(hk, L"CheckMacroSequences", bCheckMacroSequences);
		SetBool(hk, L"CheckMacrosInVars", bCheckMacrosInVars);
		SetBool(hk, L"EditBinaryAsText", bEditBinaryAsText);
		SetBool(hk, L"RefreshChanges", bRefreshChanges);
		SetBool(hk, L"LoadDescriptions", bLoadDescriptions);
		SetBool(hk, L"ExportDefaultValueFirst", bExportDefaultValueFirst);
		SetBool(hk, L"ConfirmImport", bConfirmImport);
		SetBool(hk, L"ShowImportResult", bShowImportResult);
		SetDWORD(hk, L"RefreshKeyTimeout", nRefreshKeyTimeout);
		SetDWORD(hk, L"RefreshSubkeyTimeout", nRefreshSubkeyTimeout);
		if (pszLastRegPath != NULL)
			SetString(hk, L"LastRegPath", pszLastRegPath);
		
		RegCloseKey(hk);
	}
}

BOOL RegConfig::Configure()
{
	//TODO: Открыть диалог с настройками
	PluginDialogBuilder Config(psi, REConfigTitle, _T("Configuration"));
	
	FarDialogItem *p1, *p2;
	
	Config.StartColumns();
	Config.AddCheckbox(REAddToPluginsMenu, &bAddToPluginsMenu);
	Config.ColumnBreak();
	Config.AddCheckbox(REAddToDisksMenu, &bAddToDisksMenu);
	Config.EndColumns();
	bool lbNeedDiskNumbers = true;
#ifdef _UNICODE
	if (HIWORD(gnFarVersion) >= 1692)
		lbNeedDiskNumbers = false;
#endif
	if (lbNeedDiskNumbers)
	{
		p1 = Config.AddText(REDisksMenuHotkey);
		p2 = Config.AddEditField(cDiskMenuHotkey, 2, 1); p2->X2 = p2->X1; p2->Type = DI_FIXEDIT;
		Config.MoveItemAfter(p1,p2);
	}
	
	Config.AddSeparator(REPrefixSeparator);
	Config.StartColumns();
	p1 = Config.AddText(REPrefixLabel); p2 = Config.AddEditField(sCommandPrefix, 9, 10);
	Config.MoveItemAfter(p1,p2);
	Config.ColumnBreak();
	p1 = Config.AddText(REPanelPrefixLabel); p2 = Config.AddEditField(sRegTitlePrefix, 9, 10);
	Config.MoveItemAfter(p1,p2);
	Config.EndColumns();

	Config.AddSeparator(REWow6432);
	Config.StartColumns();
	Config.AddCheckbox(REWow64on32, &bWow64on32)->Flags |= DIF_3STATE;
	Config.ColumnBreak();
	Config.AddCheckbox(REWow64process, &is64bitOs)->Flags |= DIF_DISABLE;
	Config.EndColumns();

	Config.AddSeparator();

	Config.AddCheckbox(RESkipAccessDeniedMessage, &bSkipAccessDeniedMessage);
	Config.StartColumns();
	Config.AddCheckbox(REUseBackupRestore, &bUseBackupRestore);
	Config.ColumnBreak();
	Config.AddCheckbox(RESkipPrivilegesDeniedMessage, &bSkipPrivilegesDeniedMessage);
	Config.EndColumns();
	
	Config.StartColumns();
	BOOL lbBrowseRegFiles = (bBrowseRegFiles == 1) ? 2 : (bBrowseRegFiles == 2) ? 1 : 0;
	Config.AddCheckbox(REBrowseRegFiles, &lbBrowseRegFiles)->Flags |= DIF_3STATE;
	Config.AddCheckbox(REShowKeysAsDirs, &bShowKeysAsDirs);
	Config.ColumnBreak();
	Config.AddCheckbox(REBrowseHives, &bBrowseRegHives)->Flags |= DIF_3STATE;
	Config.AddCheckbox(REExportDefaultValueFirst, &bExportDefaultValueFirst);
	Config.EndColumns();
	Config.StartColumns();
	Config.AddCheckbox(RECreateUnicodeFiles, &bCreateUnicodeFiles);
	Config.ColumnBreak();
	p1 = Config.AddText(REAnsiCpLabel); p2 = Config.AddIntEditField((int*)&nAnsiCodePage, 6);
	Config.MoveItemAfter(p1,p2);
	Config.EndColumns();
	Config.StartColumns();
	Config.AddCheckbox(REEscapeRNonExporting, &bEscapeRNonExporting);
	Config.ColumnBreak();
	Config.AddCheckbox(REEditBinaryAsText, &bEditBinaryAsText);
	Config.EndColumns();
	Config.StartColumns();
	Config.AddCheckbox(RECheckMacroSeq, &bCheckMacroSequences);
	Config.ColumnBreak();
	Config.AddCheckbox(RECheckMacroVar, &bCheckMacrosInVars);
	Config.EndColumns();

	// Speed up browsing of large keys (HKCR\\CLSID, and so on)
	Config.AddSeparator(RESpeedUpLargeKeysSeparator);
	Config.AddCheckbox(REUnsortLargeKeys, &bUnsortLargeKeys);
	Config.SlideItemUp(Config.AddIntEditField((int*)&nLargeKeyCount, 7));
	Config.AddCheckbox(RELoadDescriptions, &bLoadDescriptions);
	Config.SlideItemUp(Config.AddIntEditField((int*)&nLargeKeyTimeout, 7));

	// 
	Config.AddSeparator(REAutoRefreshSeparator);
	Config.AddCheckbox(REAutoRefreshChanges, &bRefreshChanges);
	Config.StartColumns();
	p1 = Config.AddText(RERefreshKeyTimeoutLabel); p2 = Config.AddIntEditField((int*)&nRefreshKeyTimeout, 6);
	Config.MoveItemAfter(p1,p2);
	Config.ColumnBreak();
	p1 = Config.AddText(RERefreshSubkeyTimeoutLabel); p2 = Config.AddIntEditField((int*)&nRefreshSubkeyTimeout, 6);
	Config.MoveItemAfter(p1,p2);
	Config.EndColumns();



	Config.AddOKCancel(REBtnOK, REBtnCancel);

	if (Config.ShowDialog())
	{
		if (nAnsiCodePage == CP_UTF8 || nAnsiCodePage == CP_UTF7)
		{
			InvalidOp(); nAnsiCodePage = 1251;
		}
		bBrowseRegFiles = (lbBrowseRegFiles == 1) ? 2 : (lbBrowseRegFiles == 2) ? 1 : 0;
		// сохраним в реестр
		SaveConfiguration();
		// Обновить заголовки
		gpPluginList->UpdateAllTitles();
	}
	
	// Вернуть TRUE, если нужно обновить панели
	return FALSE;
}

DWORD RegConfig::samDesired()
{
	switch (bWow64on32)
	{
	case 0:
		return KEY_WOW64_32KEY;
	case 1:
		return KEY_WOW64_64KEY;
	default:
		return 0;
	}
	//if (is64bitOs) {
	//	if (bWow64on32)
	//		return KEY_WOW64_64KEY;
	//	else
	//		return KEY_WOW64_32KEY;
	//} else {
	//	return 0;
	//}
}

const TCHAR* RegConfig::bitSuffix()
{
	switch (bWow64on32)
	{
	case 0:
		return GetMsg(bVirtualize ? REPanelx32VirtLabel : REPanelx32Label);
	case 1:
		return GetMsg(bVirtualize ? REPanelx64VirtLabel : REPanelx64Label);
	default:
		return bVirtualize ? GetMsg(REPanelVirtLabel) : _T("");
	}
	//if (is64bitOs) {
	//	if (bWow64on32)
	//		return GetMsg(REPanelx64Label);
	//	else
	//		return GetMsg(REPanelx32Label);
	//} else {
	//	return _T("");
	//}
}

#ifdef _UNICODE
UINT RegConfig::EditorCodePage()
{
	UINT cp = bCreateUnicodeFiles ? 1200 : (nAnsiCodePage ? nAnsiCodePage : CP_AUTODETECT);
	return cp;
}
#endif

UINT RegConfig::TitleAddLen()
{
	// Довесок до пути, отображается в заголовке панели
	// + "REG[x64]:" + prefix + HKEY_xxx + " (*)"
	return 128;
}

void RegConfig::SetLastRegPath(HKEY ahKey, LPCWSTR asSubKey)
{
	MCHKHEAP;
	int nLen = lstrlenW(asSubKey);
	if (!pszLastRegPath || (nLen + 40) >= nMaxRegPath) {
		SafeFree(pszLastRegPath);
		nMaxRegPath = nLen + MAX_PATH; // с запасом
		pszLastRegPath = (wchar_t*)malloc(nMaxRegPath*2);
	}
	pszLastRegPath[0] = 0;
	MCHKHEAP;
	if (ahKey) {
		if (!HKeyToStringKey(ahKey, pszLastRegPath, 40)) {
			// Неизвестный ключ!
			_ASSERTE(ahKey==HKEY_CLASSES_ROOT);
		} else if (asSubKey && *asSubKey) {
			lstrcatW(pszLastRegPath, L"\\");
			lstrcatW(pszLastRegPath, asSubKey);
		}
	} else {
		_ASSERTE(asSubKey == NULL || *asSubKey == 0);
	}
	MCHKHEAP;
	// Записать в реестр
	HKEY hk;
	if (0 == RegCreateKeyEx(HKEY_CURRENT_USER, pszPluginKey, 0, NULL, 0, KEY_ALL_ACCESS, NULL, &hk, NULL))
	{
		_ASSERTE(sizeof(*pszLastRegPath)==2);
		DWORD nSize = (lstrlenW(pszLastRegPath)+1)*2;
		RegSetValueExW(hk, L"LastRegPath", 0, REG_SZ, (LPBYTE)pszLastRegPath, nSize);
		RegCloseKey(hk);
	}
	MCHKHEAP;
}

//// Для заголовка панели
//LPCTSTR RegConfig::GetActivePrivileges()
//{
//	if (!mp_Token)
//		return _T("");
//	return GetMsg(mb_RestoreAquired ? REPanelBackupRestorePrefix : REPanelBackupPrefix); 
//}

BOOL RegConfig::BackupPrivilegesAcuire(BOOL abRequireRestore)
{
	if (mp_Token && abRequireRestore && !mb_RestoreAquired)
	{
		CAdjustProcessToken *p = mp_Token; mp_Token = NULL;
		SafeDelete(p);
		mn_TokenRef = 0; mb_RestoreAquired = FALSE;
	}
	if (mn_TokenRef == 0 || !mp_Token)
	{
		mp_Token = new CAdjustProcessToken();
		LPCTSTR pszPriv[] = {SE_BACKUP_NAME, SE_RESTORE_NAME};
		int iRc = mp_Token->Enable(pszPriv, abRequireRestore ? 2 : 1);
		if (iRc != 1)
		{
			if (!mb_TokenWarnedOnce && !bSkipPrivilegesDeniedMessage) {
				mb_TokenWarnedOnce = TRUE;
				psi.Message(psi.ModuleNumber, FMSG_WARNING|FMSG_ERRORTYPE|FMSG_MB_OK|FMSG_ALLINONE, NULL, 
					(const TCHAR * const *)GetMsg(REM_CantAjustPrivileges), 1, 0);
			}
			SafeDelete(mp_Token);
			mn_TokenRef = 0; mb_RestoreAquired = FALSE;
		} else {
			// Если удалось поднять хотя бы одну привилегию
			mn_TokenRef = 1;
			mb_RestoreAquired = abRequireRestore;
		}
	} else {
		mn_TokenRef++;
	}
	return (mn_TokenRef > 0 && mp_Token);
}

void RegConfig::BackupPrivilegesRelease()
{
	if (mn_TokenRef > 0)
		mn_TokenRef --;

	if (mn_TokenRef <= 0)
	{
		mn_TokenRef = 0;
		if (mp_Token)
		{
			CAdjustProcessToken *p = mp_Token; mp_Token = NULL;
			SafeDelete(p);
		}
	}
}

// pKey нужен для того чтобы выбрать имя значения, в котором будут храниться линки
void RegConfig::BookmarksGet(RegPath* pKey, BOOL bRemote, TCHAR** ppszBookmarks, DWORD* pnBookmarksCount)
{
	if (bUseInternalBookmarks)
		BookmarksLoadInt(pKey, bRemote);
	else
		BookmarksLoadExt(pKey, bRemote);

	if (ppszBookmarks)
	{
		if (!cchBookmarksLen || !pszBookmarks)
		{
			*ppszBookmarks = (TCHAR*)calloc(2,sizeof(TCHAR));
			if (pnBookmarksCount) *pnBookmarksCount = 0;
		}
		else
		{
			_ASSERTE(pszBookmarks);
			*ppszBookmarks = (TCHAR*)calloc(cchBookmarksLen,sizeof(TCHAR));
			#ifdef _UNICODE
			wmemmove(*ppszBookmarks, pszBookmarks, cchBookmarksLen);
			#else
			WideCharToMultiByte(nAnsiCodePage, 0, pszBookmarks, cchBookmarksLen, *ppszBookmarks, cchBookmarksLen, 0,0);
			#endif
			if (pnBookmarksCount)
				BookmarksLen(pnBookmarksCount);
		}
	}
}
void RegConfig::BookmarksReset(RegPath* pKey, BOOL bRemote)
{
	if (!bUseInternalBookmarks /*|| pKey->eType != RE_WINAPI*/)
		return;

	SafeFree(pszBookmarks);
	cchBookmarksMaxCount = 2048;
	pszBookmarks = (wchar_t*)calloc(cchBookmarksMaxCount,2);
	cchBookmarksLen = 1; // '\0'

	if (pKey->eType == RE_WINAPI)
	{
		#ifdef _UNICODE
		BookmarkInsertInt(-1, L"HKEY_CURRENT_USER\\Software\\Far2", FALSE);
		#else
		BookmarkInsertInt(-1, L"HKEY_CURRENT_USER\\Software\\Far", FALSE);
		#endif
		BookmarkInsertInt(-1, L"HKEY_LOCAL_MACHINE\\Software\\Far", FALSE);
		BookmarkInsertInt(-1, L"HKEY_CLASSES_ROOT", FALSE);
		BookmarkInsertInt(-1, L"HKEY_CLASSES_ROOT\\CLSID", FALSE);
		BookmarkInsertInt(-1, L"HKEY_CLASSES_ROOT\\TypeLib", FALSE);
		BookmarkInsertInt(-1, L"HKEY_CLASSES_ROOT\\Interface", FALSE);
		
		if (bRemote)
		{
			BookmarkInsertInt(-1, L"", FALSE); // Separator
			BookmarkInsertInt(-1, L"HKLM\\Software\\Microsoft\\OLAP Server", FALSE);
			BookmarkInsertInt(-1, L"HKLM\\Software\\Microsoft\\Windows NT\\CurrentVersion", FALSE);
			BookmarkInsertInt(-1, L"HKLM\\Software\\Microsoft\\Windows NT\\CurrentVersion\\Perflib", FALSE);
			BookmarkInsertInt(-1, L"HKLM\\Software\\Microsoft\\Windows NT\\CurrentVersion\\Print", FALSE);
			BookmarkInsertInt(-1, L"HKLM\\Software\\Microsoft\\Windows NT\\CurrentVersion\\Windows", FALSE);
			BookmarkInsertInt(-1, L"HKLM\\System\\CurrentControlSet\\Control\\ContentIndex", FALSE);
			BookmarkInsertInt(-1, L"HKLM\\System\\CurrentControlSet\\Control\\Print\\Printers", FALSE);
			BookmarkInsertInt(-1, L"HKLM\\System\\CurrentControlSet\\Control\\ProductOptions", FALSE);
			BookmarkInsertInt(-1, L"HKLM\\System\\CurrentControlSet\\Control\\Server Applications", FALSE);
			BookmarkInsertInt(-1, L"HKLM\\System\\CurrentControlSet\\Control\\Terminal Server", FALSE);
			BookmarkInsertInt(-1, L"HKLM\\System\\CurrentControlSet\\Control\\Terminal Server\\DefaultUserConfiguration", FALSE);
			BookmarkInsertInt(-1, L"HKLM\\System\\CurrentControlSet\\Control\\Terminal Server\\UserConfig", FALSE);
			BookmarkInsertInt(-1, L"HKLM\\System\\CurrentControlSet\\Services\\CertSvc", FALSE);
			BookmarkInsertInt(-1, L"HKLM\\System\\CurrentControlSet\\Services\\Eventlog", FALSE);
			BookmarkInsertInt(-1, L"HKLM\\System\\CurrentControlSet\\Services\\SysmonLog", FALSE);
			BookmarkInsertInt(-1, L"HKLM\\System\\CurrentControlSet\\Services\\Wins", FALSE);
		}
	
		//#ifdef _UNICODE
		//lstrcpyW(psz, L"HKEY_CURRENT_USER\\Software\\Far2"); psz += lstrlenW(psz)+1;
		//#else
		//lstrcpyW(psz, L"HKEY_CURRENT_USER\\Software\\Far"); psz += lstrlenW(psz)+1;
		//#endif
		//lstrcpyW(psz, L"HKEY_LOCAL_MACHINE\\Software\\Far"); psz += lstrlenW(psz)+1;
		//lstrcpyW(psz, L"HKEY_CLASSES_ROOT"); psz += lstrlenW(psz)+1;
		//lstrcpyW(psz, L"HKEY_CLASSES_ROOT\\CLSID"); psz += lstrlenW(psz)+1;
		//lstrcpyW(psz, L"HKEY_CLASSES_ROOT\\TypeLib"); psz += lstrlenW(psz)+1;
		//lstrcpyW(psz, L"HKEY_CLASSES_ROOT\\Interface"); psz += lstrlenW(psz)+1;
		/*
		HKLM\Software\Microsoft\OLAP Server
		HKLM\Software\Microsoft\Windows NT\CurrentVersion
		HKLM\Software\Microsoft\Windows NT\CurrentVersion\Perflib
		HKLM\Software\Microsoft\Windows NT\CurrentVersion\Print
		HKLM\Software\Microsoft\Windows NT\CurrentVersion\Windows
		HKLM\System\CurrentControlSet\Control\ContentIndex
		HKLM\System\CurrentControlSet\Control\Print\Printers
		HKLM\System\CurrentControlSet\Control\ProductOptions
		HKLM\System\CurrentControlSet\Control\Server Applications
		HKLM\System\CurrentControlSet\Control\Terminal Server
		HKLM\System\CurrentControlSet\Control\Terminal Server\DefaultUserConfiguration
		HKLM\System\CurrentControlSet\Control\Terminal Server\UserConfig
		HKLM\System\CurrentControlSet\Services\CertSvc
		HKLM\System\CurrentControlSet\Services\Eventlog
		HKLM\System\CurrentControlSet\Services\SysmonLog
		HKLM\System\CurrentControlSet\Services\Wins
		*/
	}

	//cchBookmarksLen = (psz - pszBookmarks + 1);

	BookmarksSaveInt();
}
void RegConfig::BookmarksLoadInt(RegPath* pKey, BOOL bRemote)
{
	LONG nRegRc;
	HKEY hk;
	BOOL bNoBookmarks = FALSE;
	
	SafeFree(pszBookmarks);
	
	switch (pKey->eType)
	{
		case RE_WINAPI:
			lstrcpyW(szBookmarksValueName, bRemote ? L"RegBookmarksRemote" : L"RegBookmarksLocal"); break;
		case RE_REGFILE:
			lstrcpyW(szBookmarksValueName, L"RegBookmarksFile"); break;
		case RE_HIVE:
			lstrcpyW(szBookmarksValueName, L"RegBookmarksHive"); break;
		default:
			InvalidOp();
			return;
	}

	cchBookmarksLen = 0;
	if (0 != RegOpenKeyEx(HKEY_CURRENT_USER, pszPluginKey, 0, KEY_READ, &hk))
	{
		// Если ключа не было - сразу создать в реестре значения по умолчанию!
		bNoBookmarks = TRUE;
		
	}
	else
	{
		DWORD nSize = 0;
		nRegRc = RegQueryValueExW(hk, szBookmarksValueName, NULL, NULL, NULL, &nSize);
		if (nRegRc == 0)
		{
			_ASSERTE(pszBookmarks == NULL);
			wchar_t* pszNew = (wchar_t*)calloc(nSize+4,1);
			nRegRc = RegQueryValueExW(hk, szBookmarksValueName, NULL, NULL, (LPBYTE)pszNew, &nSize);
			if (nRegRc == 0)
			{
				BookmarkParse(pszNew, nSize/2);
				SafeFree(pszNew);
				//SafeFree(pszBookmarks);
				//pszBookmarks = (wchar_t*)calloc(nSize*2+4,1);
				//wchar_t *pszSrc = pszNew, *pszDst = pszBookmarks, *pszEnd = (;
				////while (
				////pszBookmarks = pszNew;
				//cchBookmarksMaxCount = ((nSize>>1) + 512)*2;
				//cchBookmarksLen = nSize>>1;
				//while (cchBookmarksLen > 2 && pszBookmarks[cchBookmarksLen-1] == 0 && pszBookmarks[cchBookmarksLen-2] == 0)
				//	cchBookmarksLen--;
				//// Должно быть два закрывающих нуля!
				//if (cchBookmarksLen < 2)
				//{
				//	cchBookmarksLen = 1; pszBookmarks[0] = pszBookmarks[1] = 0;
				//}
				//else
				//{
				//	if (cchBookmarksLen<1 || pszBookmarks[cchBookmarksLen-1] != 0)
				//		pszBookmarks[cchBookmarksLen++] = 0;
				//	if (cchBookmarksLen<2 || pszBookmarks[cchBookmarksLen-2] != 0)
				//		pszBookmarks[cchBookmarksLen++] = 0;
				//}
			}
		}
		else
		{
			bNoBookmarks = TRUE;
		}
		
		_ASSERTE(cchBookmarksLen < 1 || pszBookmarks[cchBookmarksLen-1] == 0);
		_ASSERTE(cchBookmarksLen < 2 || pszBookmarks[cchBookmarksLen-2] == 0);

		RegCloseKey(hk);
	}
	
	// Если закладок в реестре еще не было - создать умолчания и сохранить
	if (bNoBookmarks)
	{
		BookmarksReset(pKey, bRemote);
	}
}
void RegConfig::BookmarkParse(LPCWSTR pszNew, DWORD cchNew)
{
	DWORD cchReq = (cchNew*2)+4;
	if (!pszBookmarks || (cchBookmarksMaxCount < cchReq))
	{
		SafeFree(pszBookmarks);
		cchBookmarksMaxCount = cchReq + 512;
		pszBookmarks = (wchar_t*)calloc(cchBookmarksMaxCount,2);
	}
	
	LPCWSTR  pszSrc = pszNew;
	LPCWSTR  pszEnd = pszNew + cchNew - 1;
	wchar_t* pszDst = pszBookmarks;
	while ((pszEnd > pszSrc) && (*pszEnd == 0))
		pszEnd--;
	bool lbWasSeparator = false;
	while (pszSrc < pszEnd)
	{
		if (*pszSrc == 0)
		{
			if (!lbWasSeparator)
			{
				*(pszDst++) = L'-'; *(pszDst++) = 0; lbWasSeparator = true;
			}
			pszSrc++;
		}
		else
		{
			LPCWSTR pszNextSrc = wcspbrk(pszSrc, L"\r\n");
			if (!pszNextSrc)
				pszNextSrc = pszSrc + lstrlenW(pszSrc);
			
			size_t nLen = (pszNextSrc - pszSrc);
			if (nLen)
			{
				wmemmove(pszDst, pszSrc, nLen);
				pszDst[nLen] = 0;
				lbWasSeparator = (*pszSrc == L'-' && *(pszSrc+1) == 0);
				pszDst += nLen+1;
			}

			if (*pszNextSrc == 0)
				pszNextSrc++;
			if (*pszNextSrc == L'\r' || *pszNextSrc == L'\n')
			{
				while (*pszNextSrc == 0 || *pszNextSrc == L'\n' || *pszNextSrc == L'\r')
					pszNextSrc++;
				if (!lbWasSeparator && (pszSrc < pszEnd))
				{
					*(pszDst++) = L'-'; *(pszDst++) = 0; lbWasSeparator = true;
				}
			}
			pszSrc = pszNextSrc;
		}
	}	

	cchBookmarksLen = (DWORD)(pszDst - pszBookmarks);
	if (cchBookmarksLen == 0)
	{
		_ASSERTE(cchBookmarksLen > 0);
		cchBookmarksLen = 1;
		pszBookmarks[0] = pszBookmarks[1] = 0;
	}
	else
	{
		if (cchBookmarksLen<1 || pszBookmarks[cchBookmarksLen-1] != 0)
			pszBookmarks[cchBookmarksLen++] = 0;
		if (cchBookmarksLen<2 || pszBookmarks[cchBookmarksLen-2] != 0)
			pszBookmarks[cchBookmarksLen++] = 0;
	}
}
void RegConfig::BookmarkInsert(int nPos, RegPath* pKey)
{
	if (bUseInternalBookmarks)
		BookmarkInsertInt(nPos, pKey);
	else
		BookmarkInsertExt(nPos, pKey);
}
void RegConfig::BookmarkInsertInt(int nPos, RegPath* pKey, BOOL abSaveImmediate /*= TRUE*/)
{
	if (szBookmarksValueName[0] == 0 || !pszBookmarks)
	{
		_ASSERTE(szBookmarksValueName[0] && pszBookmarks);
		return;
	}

	// Сформировать полный путь с "HKEY_xxx"
	wchar_t* pszNewKey = NULL;
	int nKeyLen = pKey->mpsz_Key ? lstrlenW(pKey->mpsz_Key) : 0;
	pszNewKey = (wchar_t*)calloc(nKeyLen+40,2);
	if (pKey->mh_Root) {
		if (!pKey->IsKeyPredefined(pKey->mh_Root)) {
			pszNewKey[0] = 0;
		} else if (!HKeyToStringKey(pKey->mh_Root, pszNewKey , 40)) {
			// Неизвестный ключ!
			InvalidOp();
			SafeFree(pszNewKey);
			return;
		}
		MCHKHEAP;
	}
	if (*pKey->mpsz_Key) {
		lstrcatW(pszNewKey, L"\\");
		lstrcpynW(pszNewKey+lstrlenW(pszNewKey), pKey->mpsz_Key, nKeyLen+1);
	}
	BookmarkInsertInt(nPos, pszNewKey, abSaveImmediate);
	SafeFree(pszNewKey);
}
void RegConfig::BookmarkInsertInt(int nPos, LPCWSTR pszNewKey, BOOL abSaveImmediate /*= TRUE*/)
{
	int nKeyLen = lstrlenW(pszNewKey);

	wchar_t* pszNewBookmarks = NULL;
	if (cchBookmarksMaxCount <= (cchBookmarksLen + nKeyLen + 2))
	{
		cchBookmarksMaxCount = (cchBookmarksLen + nKeyLen + 512);
		pszNewBookmarks = (wchar_t*)calloc(cchBookmarksMaxCount, 2);
	}
	else
	{
		pszNewBookmarks = pszBookmarks;
	}

	//TODO: Вставить в pszBookmarks ПЕРЕД строкой nPos новую ссылку. Если эта ссылка уже есть - изменений не будет?
	wchar_t* psz = pszNewBookmarks;
	wchar_t* pszEnd = pszBookmarks + cchBookmarksLen - 1;
	if (nPos == -1)
	{
		psz = pszEnd;
	} else {
		while (psz < pszEnd && nPos > 0)
		{
			nPos--; psz += lstrlenW(psz)+1;
		}
	}
	if (pszNewBookmarks != pszBookmarks && psz != pszNewBookmarks)
	{
		memmove(pszNewBookmarks, pszBookmarks, (psz - pszNewBookmarks)*2);
	}
	if ((nPos != -1) && (psz < pszEnd))
	{
		memmove(psz+nKeyLen+1, pszBookmarks + (psz - pszNewBookmarks), (cchBookmarksLen - (psz - pszNewBookmarks))*2);
	}
	memmove(psz, pszNewKey, (nKeyLen+1)*2);

	// Запомнить изменения в Instance
	if (pszNewBookmarks != pszBookmarks)
	{
		SafeFree(pszBookmarks);
		pszBookmarks = pszNewBookmarks;
	}
	cchBookmarksLen += nKeyLen+1;
	// сразу сохранить в реестр
	if (abSaveImmediate)
		BookmarksSaveInt();
}
void RegConfig::BookmarkDelete(int nPos)
{
	if (bUseInternalBookmarks)
		BookmarkDeleteInt(nPos);
	else
		BookmarkDeleteExt(nPos);
}
void RegConfig::BookmarkDeleteInt(int nPos)
{
	if (szBookmarksValueName[0] == 0 || !pszBookmarks || nPos < 0)
	{
		_ASSERTE(szBookmarksValueName[0] && pszBookmarks && nPos >= 0);
		return;
	}
	
	// удаление пункта nPos
	wchar_t* psz = pszBookmarks;
	wchar_t* pszEnd = pszBookmarks + cchBookmarksLen - 1;
	while (psz < pszEnd && nPos > 0)
	{
		nPos--; psz += lstrlenW(psz)+1;
	}
	if (nPos == 0 && psz < pszEnd)
	{
		// Сдвинуть все остальное (ПОСЛЕ ТЕКУЩЕГО) вперед
		wchar_t* pszNext = psz + lstrlenW(psz) + 1;
		if (pszNext < pszEnd)
		{
			size_t nDelLen = pszNext - psz;
			if (nDelLen > cchBookmarksLen)
			{
				InvalidOp();
			}
			else
			{
				memmove(psz, pszNext, (cchBookmarksLen - (pszNext - pszBookmarks))*2);
				cchBookmarksLen -= (DWORD)nDelLen;
				_ASSERTE(cchBookmarksLen >= 2);
				_ASSERTE(cchBookmarksLen < 1 || pszBookmarks[cchBookmarksLen-1] == 0);
				_ASSERTE(cchBookmarksLen < 2 || pszBookmarks[cchBookmarksLen-2] == 0);
			}
		}
		else
		{
			// Удален послдений элемент. просто уменьшить длину
			cchBookmarksLen = (DWORD)(psz - pszBookmarks + 1);
			_ASSERTE(cchBookmarksLen >= 1);
			if (cchBookmarksLen > 0) pszBookmarks[cchBookmarksLen-1] = 0;
			if (cchBookmarksLen > 1) pszBookmarks[cchBookmarksLen-2] = 0;
			while (cchBookmarksLen >= 3
				&& pszBookmarks[cchBookmarksLen-1] == 0
				&& pszBookmarks[cchBookmarksLen-2] == 0
				&& pszBookmarks[cchBookmarksLen-3] == 0)
			{
				cchBookmarksLen--;
			}
		}
		_ASSERTE(cchBookmarksLen < 1 || pszBookmarks[cchBookmarksLen-1] == 0);
		_ASSERTE(cchBookmarksLen < 2 || pszBookmarks[cchBookmarksLen-2] == 0);
	}

	BookmarksSaveInt();
}
void RegConfig::BookmarksEdit()
{
	if (!bUseInternalBookmarks)
		return;

	if (szBookmarksValueName[0] == 0)
	{
		_ASSERTE(szBookmarksValueName[0]);
		return;
	}
	
	MFileTxt file;
	BOOL lbFarUnicode =
	#ifdef _UNICODE
		TRUE
	#else
		FALSE
	#endif
		;
	if (!file.FileCreateTemp(szBookmarksValueName, L".txt", lbFarUnicode))
	{
		InvalidOp();
		return;
	}
	if (pszBookmarks)
	{
		if (!file.FileWriteMSZ(pszBookmarks, BookmarksLen(NULL)))
		{
			InvalidOp();
			file.FileDelete();
			return;
		}
	}
	file.FileClose();
	
	
	TCHAR sTitle[64]; lstrcpy_t(sTitle, countof(sTitle), szBookmarksValueName);
	int nEdtRc = psi.Editor(file.GetShowFilePathName(), sTitle, 0,0,-1,-1,
		EF_DISABLEHISTORY/*EF_NONMODAL|EF_IMMEDIATERETURN|EF_DELETEONCLOSE|EF_ENABLE_F6*/,
		#ifdef _UNICODE
		1, 1, 1200
		#else
		0, 1
		#endif
	);
	if (nEdtRc == EEC_MODIFIED)
	{
		wchar_t* pszNew = NULL; DWORD cbNewSize = 0;
		if (MFileTxt::LoadTextMSZ(file.GetFilePathName(), lbFarUnicode, &pszNew, &cbNewSize) == 0)
		{
			BookmarkParse(pszNew, cbNewSize/2);
			SafeFree(pszNew);
			//if (cchBookmarksMaxCount*2 < cbNewSize)
			//{
			//	SafeFree(pszBookmarks);
			//	cchBookmarksMaxCount = (cbNewSize >> 1) + 512;
			//	pszBookmarks = (wchar_t*)calloc(cchBookmarksMaxCount,2);
			//}
			//if (pszBookmarks)
			//	memmove(pszBookmarks, pszNew, cbNewSize);
			//SafeFree(pszNew);
			//cchBookmarksLen = (cbNewSize >> 1);
			//if (cchBookmarksLen == 0)
			//{
			//	_ASSERTE(cchBookmarksLen > 0);
			//	cchBookmarksLen = 1;
			//	pszBookmarks[0] = pszBookmarks[1] = 0;
			//}
			//else
			//{
			//	if (cchBookmarksLen<1 || pszBookmarks[cchBookmarksLen-1] != 0)
			//		pszBookmarks[cchBookmarksLen++] = 0;
			//	if (cchBookmarksLen<2 || pszBookmarks[cchBookmarksLen-2] != 0)
			//		pszBookmarks[cchBookmarksLen++] = 0;
			//}
		}
		_ASSERTE(bUseInternalBookmarks);
		BookmarksSaveInt();
	}

	// ВРЕМЕННЫЙ файл больше не нужен!
	file.FileDelete();
}
void RegConfig::BookmarksSaveInt()
{
	if (szBookmarksValueName[0] == 0 || !pszBookmarks)
	{
		_ASSERTE(szBookmarksValueName[0] && pszBookmarks);
		return;
	}
	
	// Найти конец (длину)
	DWORD nSize = cchBookmarksLen; //BookmarksLen(NULL);
	_ASSERTE((nSize+1) < cchBookmarksMaxCount);
	if (nSize > 1 && (nSize + 1) < cchBookmarksMaxCount)
	{
		_ASSERTE(nSize < 1 || pszBookmarks[nSize-1] == 0);
		_ASSERTE(nSize < 2 || pszBookmarks[nSize-2] == 0);
		//if (pszBookmarks[nSize-1] != 0)
		//{
		//	pszBookmarks[nSize+1] = 0;
		//	nSize++;
		//}
	}
	
	// Сохраняем
	HKEY hk; DWORD dwDisp = 0;
	bSomeValuesMissed = FALSE;
	if (0 == RegCreateKeyEx(HKEY_CURRENT_USER, pszPluginKey, 0, NULL, 0, KEY_ALL_ACCESS, NULL, &hk, &dwDisp))
	{
		RegSetValueExW(hk, szBookmarksValueName, 0, REG_MULTI_SZ, (LPBYTE)pszBookmarks, nSize*2);
		RegCloseKey(hk);
	}
}
DWORD RegConfig::BookmarksLen(DWORD* pnBookmarksCount)
{
	if (!pszBookmarks)
	{
		_ASSERTE(pszBookmarks!=NULL);
		if (pnBookmarksCount) *pnBookmarksCount = 0;
		return 0;
	}
	if (cchBookmarksLen <= 2)
	{
		_ASSERTE(pszBookmarks[0] == 0 && pszBookmarks[1] == 0);
		if (pnBookmarksCount)
			*pnBookmarksCount = 0;
		return 1;
	}
	wchar_t* psz = pszBookmarks;
	wchar_t* pszEnd = pszBookmarks + cchBookmarksLen - 1;
	DWORD nCount = 0;
	while (psz < pszEnd)
	{
		if (!*psz)
		{
			// Старый разделитель?
			psz++;
		}
		else
		{
			int l = lstrlenW(psz);
			if (psz[l-1] == _T('\n'))
				nCount ++; // Новый разделитель
			psz += l+1;
		}
		nCount ++;
	}
#ifdef _DEBUG
	DWORD nLen = (DWORD)(psz - pszBookmarks + 1);
	_ASSERTE(nLen == cchBookmarksLen);
#endif
	if (pnBookmarksCount)
		*pnBookmarksCount = nCount;
	return cchBookmarksLen;
}
// Поддержка Favorites из RegEdit.exe
void RegConfig::BookmarksLoadExt(RegPath* pKey, BOOL bRemote)
{
	SafeFree(pszBookmarks);
	HKEY hk = 0;
	if (!::RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Applets\\Regedit\\Favorites",
		0, KEY_READ, &hk))
	{
		DWORD nValues = 0, nMaxNameLen = 0, nMaxDataLen = 0;
		if (!::RegQueryInfoKeyW(hk, NULL, NULL, NULL, NULL, NULL, NULL, &nValues, &nMaxNameLen, &nMaxDataLen, NULL, NULL))
		{
			if (nValues)
			{
				cchBookmarksMaxCount = (nValues + 1) * (nMaxDataLen+1) + 2;
				pszBookmarks = (wchar_t*)calloc(cchBookmarksMaxCount,2);
				wchar_t* psz = pszBookmarks;
				wchar_t* pszBuf = (wchar_t*)calloc(nMaxDataLen+2,1);
				wchar_t* pszName = (wchar_t*)calloc(nMaxNameLen+1,2);
				for (DWORD i = 0;; i++)
				{
					DWORD nNameLen = nMaxNameLen+1; DWORD nDataLen = nMaxDataLen+2;
					if (::RegEnumValueW(hk, i, pszName, &nNameLen, NULL, NULL, (LPBYTE)pszBuf, &nDataLen))
						break;
					wchar_t* pszSlash = wcschr(pszBuf, L'\\');
					if (!pszSlash || !*(pszSlash+1)) continue;
					lstrcpyW(psz, pszSlash+1); psz += lstrlenW(psz) + 1;
				}
				SafeFree(pszBuf);
				SafeFree(pszName);
				cchBookmarksLen = (psz - pszBookmarks + 1);
			}
		}
		::RegCloseKey(hk);
	}
}
void RegConfig::BookmarkInsertExt(int nPos, RegPath* pKey)
{
}
void RegConfig::BookmarkDeleteExt(int nPos)
{
}
