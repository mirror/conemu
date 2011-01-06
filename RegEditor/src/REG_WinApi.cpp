
#include "header.h"
#include <Aclui.h>

// PrivOjSI.cpp
extern "C"
HRESULT
CreateObjSecurityInfo(DWORD dwFlags,          // e.g. SI_EDIT_ALL | SI_ADVANCED | SI_CONTAINER
                      PSECURITY_DESCRIPTOR *ppSD,        // Program defined structure for objects
                      LPSECURITYINFO *ppObjSI,
                      HANDLE  hToken,         // client token
                      LPCWSTR pszServerName,  // Name of server on which SIDs will be resolved
                      LPCWSTR pszKeyName,
					  HKEY hkRoot, LPCWSTR pszKeyPath, DWORD dwRegFlags);    // This is the only way to name my generic objects


void EditKeyPermissions(HKEY hkRoot, LPCWSTR pszKeyPath)
{
	LONG hRc = 0;
	DWORD cbSecurityDescriptor = 4096;
	PSECURITY_DESCRIPTOR pSecurityDescriptor = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, cbSecurityDescriptor);
	LPSECURITYINFO pSI = NULL;
	HANDLE hToken = NULL;
	HKEY hk = NULL;
	wchar_t szKeyName[512];
	BOOL bTokenAquired = FALSE;
	SECURITY_INFORMATION si = OWNER_SECURITY_INFORMATION|GROUP_SECURITY_INFORMATION|DACL_SECURITY_INFORMATION;

	if (pszKeyPath && *pszKeyPath) {
		hRc = RegOpenKeyExW(hkRoot, pszKeyPath, 0, KEY_READ|cfg->samDesired(), &hk);
		if (hRc == ERROR_ACCESS_DENIED && cfg->bUseBackupRestore)
		{
			// Попытаться повысить привилегии
			bTokenAquired = cfg->BackupPrivilegesAcuire(TRUE);
			if (bTokenAquired)
			{
				hRc = ::RegCreateKeyExW(hkRoot, pszKeyPath, 0, NULL, REG_OPTION_BACKUP_RESTORE, cfg->samDesired(), NULL, &hk, NULL);
				if (hRc != 0)
				{
					cfg->BackupPrivilegesRelease();
					bTokenAquired = FALSE;
				}
			}
		}
		lstrcpynW(szKeyName, PointToName(pszKeyPath), countof(szKeyName));
	} else {
		hk = hkRoot;
		HKeyToStringKey(hkRoot, szKeyName, countof(szKeyName));
	}
	
	OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &hToken);
	
	if (hRc == 0) {
		hRc = RegGetKeySecurity(hk, si, pSecurityDescriptor, &cbSecurityDescriptor);
		if (hRc == ERROR_INSUFFICIENT_BUFFER) {
			LocalFree((HLOCAL)pSecurityDescriptor);
			pSecurityDescriptor = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR,cbSecurityDescriptor);
			hRc = RegGetKeySecurity(hk, si, pSecurityDescriptor, &cbSecurityDescriptor);
		}

		if (hk && hk != hkRoot)
			RegCloseKey(hk);
	}

	if (hRc == 0)
	{
		// If an administrator needs access to the key, the solution is to enable 
		// the SE_TAKE_OWNERSHIP_NAME privilege and open the registry key with WRITE_OWNER access.
		// For more information, see Enabling and Disabling Privileges.
		hRc = CreateObjSecurityInfo(
					SI_READONLY | SI_EDIT_ALL | SI_ADVANCED | SI_NO_ACL_PROTECT | SI_CONTAINER,
					&pSecurityDescriptor, &pSI, hToken,
					L""/* Computer Name */, szKeyName,
					hkRoot, pszKeyPath,
					READ_CONTROL|WRITE_DAC|WRITE_OWNER|cfg->samDesired());
	}
	
	if(hRc == 0)
	{
		//TODO: Можно бы запустить отдельную нить, а в этой мониторить созданный диалог, чтоб его наверх поднять, если он под фаром окажется
		EditSecurity(NULL,pSI);
		pSI->Release(); pSI = NULL;
	}

	if (bTokenAquired)
	{
		cfg->BackupPrivilegesRelease();
		bTokenAquired = FALSE;
	}
	
	if (hToken)
		CloseHandle(hToken);

	if (pSecurityDescriptor)
		LocalFree((HLOCAL)pSecurityDescriptor);	
}


// Ctor
MRegistryWinApi::MRegistryWinApi()
	: MRegistryBase()
{
	eType = RE_WINAPI;
	nPredefined = 0;
	bTokenAquired = FALSE;
	hAcquiredHkey = NULL;
	//hRemoteLogon = NULL;
	
	hAdvApi = LoadLibraryW(L"Advapi32.dll");
	if (hAdvApi)
		_RegDeleteKeyEx = (RegDeleteKeyExW_t)GetProcAddress(hAdvApi, "RegDeleteKeyExW");

	FillPredefined();
}

MRegistryBase* MRegistryWinApi::Duplicate()
{
	MRegistryBase* pDup = new MRegistryWinApi(/*bRemote, sRemoteServer, sRemoteLogin, sRemotePassword*/);
	if (bRemote)
		pDup->ConnectRemote(sRemoteServer, NULL, NULL, NULL);
	//if (hRemoteLogon) {
	//	HANDLE hProc = GetCurrentProcess();
	//	if (!DuplicateHandle(hProc, hRemoteLogon, 
	//			hProc, &((MRegistryWinApi*)pDup)->hRemoteLogon,
	//			0, FALSE, DUPLICATE_SAME_ACCESS))
	//	{
	//		//TODO: Показать ошибку Win32
	//		InvalidOp();
	//		((MRegistryWinApi*)pDup)->hRemoteLogon = NULL;
	//	}
	//}
	return pDup;
}

MRegistryWinApi::~MRegistryWinApi()
{
	ConnectLocal();
	if (hAdvApi)
	{
		FreeLibrary(hAdvApi);
		hAdvApi = NULL;
	}
}

void MRegistryWinApi::FillPredefined()
{
	nPredefined = 0;

	HKEY hk[] = {
		HKEY_CLASSES_ROOT,
		HKEY_CURRENT_USER,
		HKEY_LOCAL_MACHINE,
		HKEY_USERS,
		HKEY_CURRENT_CONFIG,
		HKEY_PERFORMANCE_DATA,
		NULL
	};
	
	// Для RemoteRegistry определены ТОЛЬКО HKEY_LOCAL_MACHINE, HKEY_PERFORMANCE_DATA, HKEY_USERS
	if (bRemote) {
		hk[0] = HKEY_CLASSES_ROOT;
		hk[1] = HKEY_CURRENT_USER;
		hk[2] = HKEY_LOCAL_MACHINE;
		hk[3] = HKEY_USERS;
		hk[4] = 0;
	}
	
	memset(hkPredefined, 0, sizeof(hkPredefined));
	
	for (UINT i = 0; i < countof(hk) && hk[i]; i++) {
		_ASSERTE((((ULONG_PTR)(hk[i])) & HKEY__PREDEFINED_MASK) == HKEY__PREDEFINED_TEST); //  Predefined!
		hkPredefined[i].hKey = hk[i]; hkPredefined[i].hKeyRemote = NULL;
		HKeyToStringKey(hk[i], hkPredefined[i].szKey, countof(hkPredefined[i].szKey));
		hkPredefined[i].bSlow = (hk[i] == HKEY_PERFORMANCE_DATA);
		nPredefined++;
	}
	_ASSERTE(nPredefined<countof(hkPredefined));
}

void MRegistryWinApi::ConnectLocal()
{
	if (bRemote) {
		for (UINT i = 0; i < nPredefined; i++) {
			if (hkPredefined[i].hKeyRemote) {
				// Отключение от реестра другой машины
				::RegCloseKey(hkPredefined[i].hKeyRemote);
				hkPredefined[i].hKeyRemote = NULL;
			}
		}
	}
	if (bTokenAquired) {
		bTokenAquired = FALSE;
		cfg->BackupPrivilegesRelease();
	}
	//if (hRemoteLogon)
	//{
	//	CloseHandle(hRemoteLogon);
	//	hRemoteLogon = NULL;
	//}
	
	MRegistryBase::ConnectLocal();

	FillPredefined();
}

BOOL MRegistryWinApi::ConnectRemote(LPCWSTR asServer, LPCWSTR asLogin /*= NULL*/, LPCWSTR asPassword /*= NULL*/, LPCWSTR asResource /*= NULL*/)
{
	// Отключиться, если было подключение
	ConnectLocal();
	
	if (!MRegistryBase::ConnectRemote(asServer, asLogin, asPassword, asResource))
		return FALSE;
	
	FillPredefined();

	HKEY hKey = NULL;
	LONG lRc = ConnectRegistry(HKEY_LOCAL_MACHINE, &hKey);
	if (lRc != 0)
	{
		if (0!=REPlugin::MessageFmt(REM_RemoteKeyFailed, L"HKEY_LOCAL_MACHINE", lRc, _T("RemoteConnect"), FMSG_WARNING, 2))
		{
			return FALSE;
		}
	}
	
	return TRUE;
}

// Перекрыто, потому что здесь могут быть hRemoteKeys
BOOL MRegistryWinApi::IsPredefined(HKEY hKey)
{
	if (!hKey || (((ULONG_PTR)hKey) & HKEY__PREDEFINED_MASK) != HKEY__PREDEFINED_TEST)
		return FALSE;

	for (UINT i = 0; i < nPredefined; i++) {
		if (hKey == hkPredefined[i].hKey)
			return TRUE;
	}

	_ASSERTE(hKey == hkPredefined[0].hKey);

	return FALSE;
}


LONG MRegistryWinApi::ConnectRegistry(HKEY hKey, PHKEY phkResult)
{
	if (!bRemote) {
		_ASSERTE(bRemote);
		return -1;
	}
	// Сначала все-таки проверим, может уже?
	int nIdx = -1;
	for (UINT i = 0; i < nPredefined; i++)
	{
		if (hKey == hkPredefined[i].hKey) {
			if (hkPredefined[i].hKeyRemote) {
				*phkResult = hkPredefined[i].hKeyRemote;
				return 0; // уже
			}
			nIdx = i; break;
		}
	}
	if (nIdx == -1) {
		_ASSERTE(nIdx>=0);
		return -1; // неизвестный ключ
	}
	
	//BOOL lbNeedReturnLogon = FALSE;
	//if (sRemoteLogin[0] || hRemoteLogon)
	//{
	//	if (!hRemoteLogon)
	//	{
	//		if (!LogonUserW(sRemoteLogin, NULL, sRemotePassword, LOGON32_LOGON_NEW_CREDENTIALS, LOGON32_PROVIDER_DEFAULT, &hRemoteLogon))
	//		{
	//			// Показать ошибку Win32 - REM_LogonFailed
	//			DWORD nErrCode = GetLastError();
	//			REPlugin::MessageFmt(REM_LogonFailed, sRemoteLogin, nErrCode, _T("RemoteConnect"));
	//		}
	//		SecureZeroMemory(sRemotePassword, sizeof(sRemotePassword));
	//	}
	//	if (hRemoteLogon)
	//	{
	//		if (!ImpersonateLoggedOnUser(hRemoteLogon))
	//		{
	//			// Показать ошибку Win32 : REM_ImpersonateFailed
	//			DWORD nErrCode = GetLastError();
	//			REPlugin::MessageFmt(REM_ImpersonateFailed, sRemoteLogin, nErrCode, _T("RemoteConnect"));
	//		} else {
	//			lbNeedReturnLogon = TRUE;
	//		}
	//	}
	//}
	
	LONG lRc = ::RegConnectRegistryW(
			sRemoteServer[0] ? sRemoteServer : NULL, // NULL - для локальной машины
			hKey, &(hkPredefined[nIdx].hKeyRemote));
	
	//if (lbNeedReturnLogon)
	//{
	//	if (!RevertToSelf())
	//	{
	//		// Показать ошибку Win32 : REM_RevertToSelfFailed
	//		DWORD nErrCode = GetLastError();
	//		if (!nErrCode) { SetLastError(-1); }
	//		REPlugin::Message(REM_RevertToSelfFailed, FMSG_ERRORTYPE|FMSG_WARNING|FMSG_MB_OK, 0, _T("RemoteConnect"));
	//	}
	//}

	if (lRc == 0) {
		*phkResult = hkPredefined[nIdx].hKeyRemote;
	}
	return lRc;
}

// Wrappers
LONG MRegistryWinApi::CreateKeyEx(HKEY hKey, LPCWSTR lpSubKey, DWORD Reserved, LPWSTR lpClass, DWORD dwOptions, REGSAM samDesired, LPSECURITY_ATTRIBUTES lpSecurityAttributes, PHKEY phkResult, LPDWORD lpdwDisposition, DWORD *pnKeyFlags, RegKeyOpenRights *apRights /*= NULL*/, LPCWSTR pszComment /*= NULL*/)
{
	LONG lRc = 0;
	samDesired &= ~(KEY_NOTIFY);
	if (hKey == NULL) {
		_ASSERTE(hKey!=NULL);
		lRc = E_INVALIDARG; // Predefined keys не открываем?
	} else {
		if (bRemote && IsPredefined(hKey)) {
			// Если подключение уже есть - возвращает его!
			lRc = ConnectRegistry(hKey, &hKey);
			if (lRc != 0)
				return lRc;
		}
		lRc = ::RegCreateKeyExW(hKey, lpSubKey, Reserved, lpClass, dwOptions, samDesired|cfg->samDesired(), lpSecurityAttributes, phkResult, lpdwDisposition);
		if (!bRemote && lRc == ERROR_ACCESS_DENIED && cfg->bUseBackupRestore && !(apRights && *apRights == eRightsNoAdjust))
		{
			// Попытаться повысить привилегии
			if (!bTokenAquired)
			{
				bTokenAquired = cfg->BackupPrivilegesAcuire(TRUE);
			}
			if (bTokenAquired)
			{
				lRc = ::RegCreateKeyExW(hKey, lpSubKey, Reserved, lpClass, dwOptions|REG_OPTION_BACKUP_RESTORE, samDesired|cfg->samDesired(), lpSecurityAttributes, phkResult, lpdwDisposition);
				if (lRc == 0)
				{
					if (apRights) *apRights = eRightsBackupRestore;
					if (hAcquiredHkey == NULL)
						hAcquiredHkey = *phkResult;
				} else {
					cfg->BackupPrivilegesRelease();
					bTokenAquired = FALSE;
				}
			}
		}
	}
	if (lRc) SetLastError(lRc);
	return lRc;
}

LONG MRegistryWinApi::OpenKeyEx(HKEY hKey, LPCWSTR lpSubKey, DWORD ulOptions, REGSAM samDesired, PHKEY phkResult, DWORD *pnKeyFlags, RegKeyOpenRights *apRights /*= NULL*/)
{
	LONG lRc = 0;
	samDesired &= ~(KEY_NOTIFY);
	if (hKey == NULL) {
		_ASSERTE(hKey!=NULL);
		lRc = E_INVALIDARG; // Predefined keys не открываем?
	} else {
		if (bRemote && IsPredefined(hKey)) {
			// Если подключение уже есть - возвращает его!
			lRc = ConnectRegistry(hKey, &hKey);
			if (lRc != 0)
				return lRc;
			if (!lpSubKey || !*lpSubKey)
			{
				*phkResult = hKey;
				return 0;
			}
		}
		lRc = ::RegOpenKeyExW(hKey, lpSubKey, ulOptions, samDesired|cfg->samDesired(), phkResult);
		if (!bRemote && lRc == ERROR_ACCESS_DENIED && cfg->bUseBackupRestore && !(apRights && *apRights == eRightsNoAdjust))
		{
			// Попытаться повысить привилегии
			if (!bTokenAquired)
			{
				bTokenAquired = cfg->BackupPrivilegesAcuire((samDesired & KEY_WRITE) == KEY_WRITE);
			}
			if (bTokenAquired)
			{
				lRc = ::RegCreateKeyExW(hKey, lpSubKey, 0, NULL, REG_OPTION_BACKUP_RESTORE, samDesired|cfg->samDesired(), NULL, phkResult, NULL);
				if (lRc == 0)
				{
					if (apRights) *apRights = ((samDesired & KEY_WRITE) == KEY_WRITE) ? eRightsBackupRestore : eRightsBackup;
					if (hAcquiredHkey == NULL)
						hAcquiredHkey = *phkResult;
				} else {
					cfg->BackupPrivilegesRelease();
					bTokenAquired = FALSE;
				}
			}
		}
		if (lRc != 0 && ((samDesired & (KEY_SET_VALUE|KEY_CREATE_SUB_KEY)) == 0))
		{
			// Попытаться открыть ключ со пониженными правами
			lRc = ::RegOpenKeyExW(hKey, lpSubKey, ulOptions, KEY_ENUMERATE_SUB_KEYS|cfg->samDesired(), phkResult);
		}
	}
	if (lRc) SetLastError(lRc);
	return lRc;
}

LONG MRegistryWinApi::CloseKey(HKEY hKey)
{
	if (hKey == NULL) {
		_ASSERTE(hKey!=NULL);
		return -1;
	}
	for (UINT i = 0; i < nPredefined; i++) {
		if (hkPredefined[i].hKey == hKey)
			return 0; // Predefined key!
		if (bRemote) {
			if (hkPredefined[i].hKeyRemote == hKey)
				return 0; // Predefined Remote key! они закрываются в деструкторе!
		}
	}
	
	LONG lRc = ::RegCloseKey(hKey);

	if (hAcquiredHkey && hKey == hAcquiredHkey)
	{
		hAcquiredHkey = NULL;
		if (bTokenAquired) {
			bTokenAquired = FALSE;
			cfg->BackupPrivilegesRelease();
		}
	}
	return lRc;
}

LONG MRegistryWinApi::QueryInfoKey(
		HKEY hKey, LPWSTR lpClass, LPDWORD lpcClass, LPDWORD lpReserved,
		LPDWORD lpcSubKeys, LPDWORD lpcMaxSubKeyLen, LPDWORD lpcMaxClassLen,
		LPDWORD lpcValues, LPDWORD lpcMaxValueNameLen, LPDWORD lpcMaxValueLen,
		LPDWORD lpcbSecurityDescriptor, REGFILETIME* lpftLastWriteTime)
{
	if (hKey == NULL) {
		// Собираются смотреть список ключей (HKEY_xxx)
		if (lpClass) lpClass[0] = 0;
		if (lpcSubKeys) *lpcSubKeys = nPredefined;
		if (lpcMaxSubKeyLen) *lpcMaxSubKeyLen = 32;
		if (lpcMaxClassLen) *lpcMaxClassLen = 0;
		if (lpcValues) *lpcValues = 0;
		if (lpcMaxValueNameLen) *lpcMaxValueNameLen = 0;
		if (lpcMaxValueLen) *lpcMaxValueLen = 0;
		if (lpcbSecurityDescriptor) *lpcbSecurityDescriptor = 0;
		if (lpftLastWriteTime) {
			SYSTEMTIME st; GetSystemTime(&st); SystemTimeToFileTime(&st, (PFILETIME)lpftLastWriteTime);
		}
		return 0;
	}

	LONG lRc = 0;	
	if (bRemote && IsPredefined(hKey)) {
		// Если подключение уже есть - возвращает его!
		lRc = ConnectRegistry(hKey, &hKey);
		if (lRc != 0)
			return lRc;
	}
	
	lRc = ::RegQueryInfoKeyW(hKey, lpClass, lpcClass, lpReserved, lpcSubKeys, lpcMaxSubKeyLen, lpcMaxClassLen, lpcValues, lpcMaxValueNameLen, lpcMaxValueLen, lpcbSecurityDescriptor, (PFILETIME)lpftLastWriteTime);
	return lRc;
}

LONG MRegistryWinApi::EnumValue(HKEY hKey, DWORD dwIndex, LPWSTR lpValueName, LPDWORD lpcchValueName, LPDWORD lpReserved, REGTYPE* lpDataType, LPBYTE lpData, LPDWORD lpcbData, BOOL abEnumComments, LPCWSTR* ppszValueComment /*= NULL*/)
{
	if (hKey == NULL) {
		return ERROR_NO_MORE_ITEMS;
	}
	LONG lRc = 0;
	if (bRemote && IsPredefined(hKey)) {
		// Если подключение уже есть - возвращает его!
		lRc = ConnectRegistry(hKey, &hKey);
		if (lRc != 0)
			return lRc;
	}
	DWORD nApiDataType = 0;
	lRc = ::RegEnumValueW(hKey, dwIndex, lpValueName, lpcchValueName, lpReserved, &nApiDataType, lpData, lpcbData);
	if (lpDataType) *lpDataType = nApiDataType;
	return lRc;
}

LONG MRegistryWinApi::EnumKeyEx(HKEY hKey, DWORD dwIndex, LPWSTR lpName, LPDWORD lpcName, LPDWORD lpReserved, LPWSTR lpClass, LPDWORD lpcClass, REGFILETIME* lpftLastWriteTime, DWORD* pnKeyFlags /*= NULL*/, TCHAR* lpDefValue /*= NULL*/, DWORD cchDefValueMax /*= 0*/, LPCWSTR* ppszKeyComment /*= NULL*/)
{
	LONG lRc = 0;

	// Сразу сбросим, то что в API не используется
	if (pnKeyFlags) *pnKeyFlags = 0;
	
	// Здесь чтение описаний ключа не поддерживается
	if (lpDefValue) *lpDefValue = 0;

	if (hKey == NULL) {
		if (dwIndex >= nPredefined)
			return ERROR_NO_MORE_ITEMS;
		if (bRemote || hkPredefined[dwIndex].bSlow) {
			if (lpftLastWriteTime) {
				SYSTEMTIME st; GetSystemTime(&st); SystemTimeToFileTime(&st, (PFILETIME)lpftLastWriteTime);
			}
			if (lpClass) *lpClass = 0;
			if (lpcClass) *lpcClass = 0;
		} else
		if (lpftLastWriteTime || lpClass || lpcClass) {
			lRc = ::RegQueryInfoKeyW(
						hkPredefined[dwIndex].hKey, lpClass, lpcClass,
						NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
						(PFILETIME)lpftLastWriteTime);
		}
		if (lpName) lstrcpynW(lpName, hkPredefined[dwIndex].szKey, *lpcName);
		*lpcName = lstrlenW(hkPredefined[dwIndex].szKey);
	} else {
		if (bRemote && IsPredefined(hKey)) {
			// Если подключение уже есть - возвращает его!
			lRc = ConnectRegistry(hKey, &hKey);
			if (lRc != 0)
				return lRc;
		}
		lRc = ::RegEnumKeyExW(hKey, dwIndex, lpName, lpcName, lpReserved, lpClass, lpcClass, (PFILETIME)lpftLastWriteTime);
	}
	return lRc;
}

LONG MRegistryWinApi::QueryValueEx(HKEY hKey, LPCWSTR lpValueName, LPDWORD lpReserved, REGTYPE* lpDataType, LPBYTE lpData, LPDWORD lpcbData, LPCWSTR* ppszValueComment /*= NULL*/)
{
	if (hKey == NULL)
		return -1;
	LONG lRc = 0;
	if (bRemote && IsPredefined(hKey)) {
		// Если подключение уже есть - возвращает его!
		lRc = ConnectRegistry(hKey, &hKey);
		if (lRc != 0)
			return lRc;
	}
	DWORD nApiDataType = 0;
	lRc = ::RegQueryValueExW(hKey, lpValueName, lpReserved, &nApiDataType, lpData, lpcbData);
	if (lpDataType) *lpDataType = nApiDataType;
	if (ppszValueComment) *ppszValueComment = NULL;
	return lRc;
}

LONG MRegistryWinApi::SetValueEx(HKEY hKey, LPCWSTR lpValueName, DWORD Reserved, REGTYPE nDataType, const BYTE *lpData, DWORD cbData, LPCWSTR pszComment /*= NULL*/)
{
	if (nDataType >= REG__INTERNAL_TYPES)
		return 0; // Через API - не обрабатываем
	if (hKey == NULL)
		return -1;
	LONG lRc = 0;
	if (bRemote && IsPredefined(hKey)) {
		// Если подключение уже есть - возвращает его!
		lRc = ConnectRegistry(hKey, &hKey);
		if (lRc != 0)
			return lRc;
	}
	lRc = ::RegSetValueExW(hKey, lpValueName, Reserved, (DWORD)nDataType, lpData, cbData);
	return lRc;
}


// Service - TRUE, если права были изменены
BOOL MRegistryWinApi::EditKeyPermissions(RegPath *pKey, RegItem* pItem)
{
	if (pKey->eType != RE_WINAPI)
		return FALSE;
	
	if (pItem == NULL && pKey->mh_Root == NULL)
		return FALSE; // самый корень плагина, выделен ".."

	HKEY hkRoot = pKey->mh_Root;
	if (pKey->mh_Root == NULL) {
		if (!StringKeyToHKey(pItem->pszName, &hkRoot)) {
			_ASSERTE(FALSE);
			return FALSE;
		}
	}
	
	if (bRemote /*&& (hkRoot == NULL) || IsPredefined(hkRoot)*/) {
		return FALSE; // Для реестра удаленной машины - права не показывать?
	}
	

	wchar_t* pszKeyPath = NULL;
	int nFullLen = 0, nKeyLen = 0, nSubkeyLen = 0;
	if (pKey->mpsz_Key && *pKey->mpsz_Key)
		nFullLen += (nKeyLen = lstrlenW(pKey->mpsz_Key));
	if (pKey->mh_Root != NULL && pItem && pItem->nValueType == REG__KEY)
		nFullLen += (nSubkeyLen = lstrlenW(pItem->pszName));
	
	pszKeyPath = (wchar_t*)malloc((nFullLen+3)*2);
	pszKeyPath[0] = 0;
	if (nKeyLen > 0) {
		lstrcpyW(pszKeyPath, pKey->mpsz_Key);
		if (nSubkeyLen > 0) {
			pszKeyPath[nKeyLen] = L'\\';
			lstrcpyW(pszKeyPath+nKeyLen+1, pItem->pszName);
		}
	} else if (nSubkeyLen > 0 && pKey->mh_Root != NULL) {
		lstrcpyW(pszKeyPath, pItem->pszName);
	}
	
	::EditKeyPermissions(hkRoot, pszKeyPath);
	
	SafeFree(pszKeyPath);
	
	return FALSE; // пока - FALSE
}

//LONG MRegistryWinApi::NotifyChangeKeyValue(RegFolder *pFolder, HKEY hKey)
//{
//	if (bRemote)
//		return -1; // Для удаленной машины - не поддерживается
//		
//	if (pFolder->mh_ChangeNotify == NULL) {
//		pFolder->mh_ChangeNotify = CreateEvent(NULL, TRUE, FALSE, NULL);
//	} else {
//		ResetEvent(pFolder->mh_ChangeNotify);
//	}
//
//	#ifdef _DEBUG
//	DWORD nWait = WaitForSingleObject(pFolder->mh_ChangeNotify, 0);
//	#endif
//
//	// смотрим Subtree в том случае, если загружаются описания
//	// получается некоторая избыточность (мониторятся все подуровни ключа)
//	// но это лучше, чем мониторить все дочерние ключи одноуровнево
//	LONG lRc = RegNotifyChangeKeyValue(
//					hKey, FALSE/*cfg->bLoadDescriptions*/,  //TRUE - какая-то пурга. ключ постоянно перечитывается?
//					REG_NOTIFY_CHANGE_NAME|REG_NOTIFY_CHANGE_LAST_SET,
//					pFolder->mh_ChangeNotify, TRUE);
//	
//
//	#ifdef _DEBUG
//	nWait = WaitForSingleObject(pFolder->mh_ChangeNotify, 0);
//	#endif
//
//	return lRc;
//}

LONG MRegistryWinApi::DeleteValue(HKEY hKey, LPCWSTR lpValueName)
{
	LONG hRc = RegDeleteValueW(hKey, lpValueName);
	return hRc;
}

LONG MRegistryWinApi::DeleteSubkeyTree(HKEY hKey, LPCWSTR lpSubKey)
{
	if (!lpSubKey || !*lpSubKey)
		return -1;

	LONG hRc = 0;
	HKEY hSubKey = NULL;

	hRc = OpenKeyEx(hKey, lpSubKey, 0, 
		DELETE|KEY_ENUMERATE_SUB_KEYS |cfg->samDesired(), &hSubKey, NULL);
	if (hRc == 0)
	{
		RegFolder subkeys; //memset(&subkeys,0,sizeof(subkeys)); -- больше низя! инициализиуется конструктором!
		//REGFILETIME ft = {{0}};
		RegPath subkey = {RE_UNDEFINED}; subkey.Init(RE_WINAPI, hSubKey); // , L"", ft);
		subkeys.Init(&subkey);
		if (subkeys.LoadKey(NULL, this, eKeysOnly, TRUE, TRUE, FALSE, NULL))
		{
			// Удалить все подключи
			for (UINT i = 0; i < subkeys.mn_ItemCount; i++)
			{
				if (subkeys.mp_Items[i].nValueType == REG__KEY)
				{
					hRc = DeleteSubkeyTree(hSubKey, subkeys.mp_Items[i].pszName);
				}
			}
		}
		subkeys.Release();
		CloseKey(hSubKey);
	}

	hRc = ::RegDeleteKeyW(hKey, lpSubKey);
	return hRc;
}

LONG MRegistryWinApi::ExistValue(HKEY hKey, LPCWSTR lpszKey, LPCWSTR lpValueName, REGTYPE* pnDataType /*= NULL*/, DWORD* pnDataSize /*= NULL*/)
{
	LONG hRc = 0;
	HKEY hSubKey = NULL;
	REGTYPE nDataType = 0;
	DWORD nDataSize = 0;

	//TODO: Заменить KEY_READ на что-то менее требовательное к правам?
	
	// Проверить наличие ключа
	if (lpszKey && *lpszKey)
	{
		hRc = OpenKeyEx(hKey, lpszKey, 0, KEY_READ|cfg->samDesired(), &hSubKey, NULL);
	} else {
		hSubKey = hKey; hRc = 0;
	}
	
	if (hRc == 0) {
		// Проверяем наличие значение, и получаем заодно его тип данных
		hRc = QueryValueEx(hSubKey, lpValueName, NULL, &nDataType, NULL, &nDataSize);
		// Может он ERROR_MORE_DATA может вернуть?
		if (hRc == ERROR_MORE_DATA) hRc = 0;
		if (hRc == 0) {
			if (pnDataType) *pnDataType = nDataType;
			if (pnDataSize) *pnDataSize = nDataSize;
		}

		// Ключ на чтение больше не требуется
		CloseKey(hSubKey);
	}
	
	return hRc;
}

LONG MRegistryWinApi::ExistKey(HKEY hKey, LPCWSTR lpszKey, LPCWSTR lpSubKey)
{
	LONG hRc = 0;
	HKEY hSubKey = NULL;
	HKEY hSubKey2 = NULL;
	//DWORD nDataType = 0;

	//TODO: Заменить KEY_READ на что-то менее требовательное к правам?
	
	// Проверить наличие ключа
	if (lpszKey && *lpszKey)
	{
		hRc = OpenKeyEx(hKey, lpszKey, 0, KEY_READ|cfg->samDesired(), &hSubKey, NULL);
	} else {
		hSubKey = hKey; hRc = 0;
	}
	
	if (hRc == 0) {
		// Проверяем подключ
		if (lpSubKey && *lpSubKey)
		{
			hRc = OpenKeyEx(hSubKey, lpSubKey, 0, KEY_READ|cfg->samDesired(), &hSubKey2, NULL);
			// Подключ на чтение больше не требуется
			if (hRc == 0) CloseKey(hSubKey2);
		}

		// Ключ на чтение больше не требуется
		CloseKey(hSubKey);
	}
	
	return hRc;
}

LONG MRegistryWinApi::SaveKey(HKEY hKey, LPCWSTR lpFile, LPSECURITY_ATTRIBUTES lpSecurityAttributes /*= NULL*/)
{
	LONG hRc = 0;
	
	// The calling process must have the SE_BACKUP_NAME privilege enabled
	hRc = ::RegSaveKeyW(hKey, lpFile, lpSecurityAttributes);
	if (hRc != 0) SetLastError(hRc);
	
	return hRc;
}

LONG MRegistryWinApi::RestoreKey(HKEY hKey, LPCWSTR lpFile, DWORD dwFlags /*= 0*/)
{
	// The calling process must have the SE_RESTORE_NAME and SE_BACKUP_NAME privileges 
	LONG hRc = 0;
	
	hRc = ::RegRestoreKeyW(hKey, lpFile, dwFlags);
	if (hRc != 0) SetLastError(hRc);
	
	return hRc;
}

LONG MRegistryWinApi::GetSubkeyInfo(HKEY hKey, LPCWSTR lpszSubkey, LPTSTR pszDesc, DWORD cchMaxDesc, LPTSTR pszOwner, DWORD cchMaxOwner)
{
	LONG hRc = 0;
	HKEY hSubKey = NULL;
	_ASSERTE(cchMaxDesc == 128);
	
	if (pszDesc)  *pszDesc  = 0;
	if (pszOwner) *pszOwner = 0;
	
	// Не пытаться выполнить AjustTokenPrivileges
	if (0 == (hRc = OpenKeyEx(hKey, lpszSubkey, 0, KEY_READ, &hSubKey, FALSE)))
	{
		wchar_t wsData[4096];
		DWORD   dwDataSize = sizeof(wsData);
		REGTYPE dwValueType = 0;		
		LPBYTE  ptrData = (LPBYTE)wsData;
		DWORD   dwAllocDataSize = sizeof(wsData);
		PSECURITY_DESCRIPTOR pSD = NULL;
		SECURITY_INFORMATION si = OWNER_SECURITY_INFORMATION|GROUP_SECURITY_INFORMATION;
		
		// Считать значение "по умолчанию"
		hRc = QueryValueEx(hSubKey, NULL, NULL, &dwValueType, (LPBYTE)ptrData, &dwDataSize);
		if (hRc == ERROR_MORE_DATA)
		{
			_ASSERTE(dwDataSize > dwAllocDataSize);
			ptrData = (LPBYTE)malloc(dwDataSize);
			dwAllocDataSize = dwDataSize;
			hRc = QueryValueEx(hSubKey, NULL, NULL, &dwValueType, (LPBYTE)ptrData, &dwDataSize);
		}
		else if (hRc == ERROR_FILE_NOT_FOUND)
		{
			hRc = QueryValueEx(hSubKey, L"Description", NULL, &dwValueType, (LPBYTE)ptrData, &dwDataSize);
			if (hRc == ERROR_MORE_DATA)
			{
				_ASSERTE(dwDataSize > dwAllocDataSize);
				ptrData = (LPBYTE)malloc(dwDataSize);
				dwAllocDataSize = dwDataSize;
				hRc = QueryValueEx(hSubKey, L"Description", NULL, &dwValueType, (LPBYTE)ptrData, &dwDataSize);
			}
		}

		if (0 == hRc)
		{
			FormatDataVisual(dwValueType, ptrData, dwDataSize, pszDesc);
		}
		

		pSD = (PSECURITY_DESCRIPTOR)ptrData;
		hRc = RegGetKeySecurity(hSubKey, si, pSD, &dwAllocDataSize);
		if (hRc == ERROR_INSUFFICIENT_BUFFER) {
			if (ptrData != (LPBYTE)wsData)
				SafeFree(ptrData);
			ptrData = (LPBYTE)malloc(dwAllocDataSize);
			pSD = (PSECURITY_DESCRIPTOR)ptrData;
			hRc = RegGetKeySecurity(hSubKey, si, pSD, &dwAllocDataSize);
		}
		if (hRc == 0)
		{
			PSID pOwner = NULL;
			BOOL bOwnerDefaulted = FALSE;
			if (GetSecurityDescriptorOwner(pSD, &pOwner, &bOwnerDefaulted))
			{
				if (IsValidSid(pOwner))
				{
					gpPluginList->GetSIDName(bRemote ? (sRemoteServer+2) : L"", pOwner, pszOwner, cchMaxOwner);
				}
			}
		}

		if (ptrData != (LPBYTE)wsData)
			SafeFree(ptrData);
		
		CloseKey(hSubKey);
	} else {
		// Значения по умолчанию нет, но поскольку это используется только для
		// отображения в колонке C0/DIZ то вернем пустую строку, чтобы плагин
		// не пытался открыть ключ и считать default value самостоятельно
		if (pszDesc) { pszDesc[0] = _T(' '); pszDesc[1] = 0; }
		if (pszOwner) { pszOwner[0] = _T(' '); pszOwner[1] = 0; }
	}
	
	return hRc;
}

LONG MRegistryWinApi::RenameKey(RegPath* apParent, BOOL abCopyOnly, LPCWSTR lpOldSubKey, LPCWSTR lpNewSubKey, BOOL* pbRegChanged)
{
	if (!lpOldSubKey || !lpNewSubKey || !apParent || !*lpOldSubKey || !*lpNewSubKey || !pbRegChanged)
	{
		InvalidOp();
		return E_INVALIDARG;
	}

	LONG hRc = 0;

	if (!cfg->BackupPrivilegesAcuire(TRUE))
	{
		REPlugin::Message(REM_NotEnoughRighsForCopy, FMSG_WARNING|FMSG_ERRORTYPE|FMSG_MB_OK);
		cfg->BackupPrivilegesRelease();
		hRc = E_ACCESSDENIED;
	}
	else
	{
		TCHAR szTemp[MAX_PATH*2+1] = _T("");
		// Сформировать имя временного файла
		FSF.MkTemp(szTemp,
			#ifdef _UNICODE
			MAX_PATH*2,
			#endif
			_T("FREG"));
		_ASSERTE(szTemp[0] != 0);
		wchar_t szTempFile[MAX_PATH*2+1]; lstrcpy_t(szTempFile, countof(szTempFile), szTemp);

		// выполнить RegSaveKey
		HKEY hKey = NULL, hParent = NULL;
		LPSECURITY_ATTRIBUTES lpSecurityAttributes = NULL;
		if ((hRc = OpenKeyEx(apParent->mh_Root, apParent->mpsz_Key, 0, KEY_ALL_ACCESS, &hParent, NULL)) != 0)
			REPlugin::CantOpenKey(apParent, TRUE);
		if (hRc == 0 && (hRc = OpenKeyEx(hParent, lpOldSubKey, 0, KEY_ALL_ACCESS, &hKey, NULL)) != 0)
			REPlugin::CantOpenKey(apParent, lpOldSubKey, TRUE);
		if (hRc == 0 && (hRc = SaveKey(hKey, szTempFile)) != 0)
			REPlugin::CantLoadSaveKey(lpOldSubKey, szTempFile, TRUE/*abSave*/);
		//TODO: Сохранить текущий lpSecurityAttributes
		// Закрыть подключ
		if (hKey != NULL) {
			CloseKey(hKey); hKey = NULL;
		}
		// придется сразу удалить старый ключ, если различие только в регистре
		if (hRc == 0 && lstrcmpiW(lpNewSubKey, lpOldSubKey) == 0 && lstrcmpW(lpNewSubKey, lpOldSubKey) != 0) {
			if ((hRc = DeleteSubkeyTree(hParent, lpOldSubKey)) != 0)
				REPlugin::ValueOperationFailed(apParent, lpOldSubKey, TRUE/*abModify*/);
			else
				*pbRegChanged = TRUE;
		}
		// Удалять ключ с новым именем, если он уже есть, не нужно?
		// WinApi сама чистит его от было мусора (левых ключей/значений)?
		// Создаем ключ с новым именем
		if (hRc == 0 && (hRc = CreateKeyEx(hParent, lpNewSubKey, 0, NULL, 0, KEY_ALL_ACCESS, lpSecurityAttributes, &hKey, NULL, NULL)) != 0)
			REPlugin::CantOpenKey(apParent, lpNewSubKey, TRUE);
		// Загрузить содержимое из файла
		if (hRc == 0) {
			if ((hRc = RestoreKey(hKey, szTempFile)) != 0)
				REPlugin::CantLoadSaveKey(lpOldSubKey, szTemp, FALSE/*abSave*/);
			else
				*pbRegChanged = TRUE;
		}
		// Закрыть подключ
		if (hKey != NULL) {
			CloseKey(hKey); hKey = NULL;
		}
		// Удалить Удалить старое дерево ключа, если оно различается регистроНЕзависимо
		if (!abCopyOnly)
		{
			if (hRc == 0 && lstrcmpiW(lpNewSubKey, lpOldSubKey) != 0) {
				if ((hRc = DeleteSubkeyTree(hParent, lpOldSubKey)) != 0)
					REPlugin::ValueOperationFailed(apParent, lpOldSubKey, TRUE/*abModify*/);
				else
					*pbRegChanged = TRUE;
			}
		}
		// Удалить временный файл
		DeleteFileW(szTempFile);
		//TODO: Восстановить права на сам ключ?
		// Закрыть текущий ключ
		if (hParent)
			CloseKey(hParent);

		// Отпустить привилегию
		cfg->BackupPrivilegesRelease();
	}

	return hRc;
}
