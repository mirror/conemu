
#pragma once

/*

	Объект предназначен для работы с текстовыми (Ansi/Unicode) и бинарными файлами.

*/

class MFileTxt : public MRegistryBase
{
protected:
	/* 
	**** унаследованы от MRegistryBase ***
	RegWorkType eType; // с чем мы работаем (WinApi, *.reg, hive)
	bool bRemote;
	wchar_t sRemoteServer[MAX_PATH];
	**************************************
	*/
	HANDLE hFile;
	BOOL   bOneKeyCreated;
	BOOL   bUnicode;
	wchar_t *psTempDirectory;
	wchar_t *psFilePathName;
	TCHAR *psShowFilePathName;
	//BOOL   bKeyWasCreated;
	// Write buffer (cache)
	LPBYTE ptrWriteBuffer; DWORD nWriteBufferLen, nWriteBufferSize;
	// Value export buffer
	LPBYTE pExportBufferData; DWORD cbExportBufferData;
	LPBYTE pExportFormatted;  DWORD cbExportFormatted;
	LPBYTE pExportCPConvert;  DWORD cbExportCPConvert; DWORD nAnsiCP;
	wchar_t* pszExportHexValues;
	// I/O result
	BOOL bLastRc; DWORD nLastErr;
public:
	MFileTxt();
	virtual ~MFileTxt();
	// Text file operations
	BOOL FileCreateTemp(LPCWSTR asDefaultName, LPCWSTR asExtension, BOOL abUnicode);
	BOOL FileCreate(LPCWSTR asPath/* may contain filename */, LPCWSTR asDefaultName, LPCWSTR asExtension, BOOL abUnicode, BOOL abConfirmOverwrite);
	void FileClose();
	void FileDelete();
	BOOL FileWrite(LPCWSTR aszText, int anLen=-1);
	BOOL FileWriteMSZ(LPCWSTR aszText, DWORD anLen);
	BOOL FileWriteValue(LPCWSTR pszValueName, REGTYPE nDataType, const BYTE* pData, DWORD nDataSize, LPCWSTR pszComment);
	BOOL FileWriteBuffered(LPCVOID apData, DWORD nDataSize); // Binary data
	BOOL FileWriteRegHeader(MRegistryBase* pWorker);
	BOOL Flush();
	LPBYTE GetExportBuffer(DWORD cbSize); // --> pExportBufferData
	LPCWSTR GetFilePathName();
	LPCTSTR GetShowFilePathName();
	static LONG LoadText(LPCWSTR asFilePathName, BOOL abUseUnicode, wchar_t** pszText, DWORD* pcbSize);
	static LONG LoadTextMSZ(LPCWSTR asFilePathName, BOOL abUseUnicode, wchar_t** pszText, DWORD* pcbSize);
	static LONG LoadData(LPCWSTR asFilePathName, void** pData, DWORD* pcbSize, size_t ncbMaxSize = -1);
	#ifndef _UNICODE
	static LONG LoadText(LPCSTR asFilePathName, BOOL abUseUnicode, wchar_t** pszText, DWORD* pcbSize);
	static LONG LoadTextMSZ(LPCSTR asFilePathName, BOOL abUseUnicode, wchar_t** pszText, DWORD* pcbSize);
	static LONG LoadData(LPCSTR asFilePathName, void** pData, DWORD* pcbSize);
	#endif
private:
	BOOL FileCreateApi(LPCWSTR asFilePathName, BOOL abUnicode, BOOL abAppendExisting);
	LPBYTE GetFormatBuffer(DWORD cbSize); // --> pExportFormatted
	LPBYTE GetConvertBuffer(DWORD cbSize); // --> pExportCPConvert
public:
	static BOOL bBadMszDoubleZero;
public:
	// ***
	virtual MRegistryBase* Duplicate();
	//virtual LONG NotifyChangeKeyValue(RegFolder *pFolder, HKEY hKey);
	virtual LONG RenameKey(RegPath* apParent, BOOL abCopyOnly, LPCWSTR lpOldSubKey, LPCWSTR lpNewSubKey, BOOL* pbRegChanged);
	// Wrappers
	virtual LONG CreateKeyEx(HKEY hKey, LPCWSTR lpSubKey, DWORD Reserved, LPWSTR lpClass, DWORD dwOptions, REGSAM samDesired, LPSECURITY_ATTRIBUTES lpSecurityAttributes, PHKEY phkResult, LPDWORD lpdwDisposition, DWORD *pnKeyFlags, RegKeyOpenRights *apRights = NULL, LPCWSTR pszComment = NULL);
	virtual LONG OpenKeyEx(HKEY hKey, LPCWSTR lpSubKey, DWORD ulOptions, REGSAM samDesired, PHKEY phkResult, DWORD *pnKeyFlags, RegKeyOpenRights *apRights = NULL);
	virtual LONG CloseKey(HKEY hKey);
	virtual LONG QueryInfoKey(HKEY hKey, LPWSTR lpClass, LPDWORD lpcClass, LPDWORD lpReserved, LPDWORD lpcSubKeys, LPDWORD lpcMaxSubKeyLen, LPDWORD lpcMaxClassLen, LPDWORD lpcValues, LPDWORD lpcMaxValueNameLen, LPDWORD lpcMaxValueLen, LPDWORD lpcbSecurityDescriptor, REGFILETIME* lpftLastWriteTime);
	virtual LONG EnumValue(HKEY hKey, DWORD dwIndex, LPWSTR lpValueName, LPDWORD lpcchValueName, LPDWORD lpReserved, REGTYPE* lpDataType, LPBYTE lpData, LPDWORD lpcbData, BOOL abEnumComments, LPCWSTR* ppszValueComment = NULL);
	virtual LONG EnumKeyEx(HKEY hKey, DWORD dwIndex, LPWSTR lpName, LPDWORD lpcName, LPDWORD lpReserved, LPWSTR lpClass, LPDWORD lpcClass, REGFILETIME* lpftLastWriteTime, DWORD* pnKeyFlags = NULL, TCHAR* lpDefValue = NULL, DWORD cchDefValueMax = 0, LPCWSTR* ppszKeyComment = NULL);
	virtual LONG QueryValueEx(HKEY hKey, LPCWSTR lpValueName, LPDWORD lpReserved, REGTYPE* lpDataType, LPBYTE lpData, LPDWORD lpcbData, LPCWSTR* ppszValueComment = NULL);
	virtual LONG SetValueEx(HKEY hKey, LPCWSTR lpValueName, DWORD Reserved, REGTYPE nDataType, const BYTE *lpData, DWORD cbData, LPCWSTR pszComment = NULL);
	virtual LONG DeleteValue(HKEY hKey, LPCWSTR lpValueName) {return -1;}; // = 0;
	virtual LONG DeleteSubkeyTree(HKEY hKey, LPCWSTR lpSubKey) {return -1;}; // = 0;
	virtual LONG ExistValue(HKEY hKey, LPCWSTR lpszKey, LPCWSTR lpValueName, REGTYPE* pnDataType = NULL, DWORD* pnDataSize = NULL) { return -1;}; // = 0;
	virtual LONG ExistKey(HKEY hKey, LPCWSTR lpszKey, LPCWSTR lpSubKey) { return -1;}; // = 0;
	virtual LONG SaveKey(HKEY hKey, LPCWSTR lpFile, LPSECURITY_ATTRIBUTES lpSecurityAttributes = NULL) { return -1;}; // = 0;
	virtual LONG RestoreKey(HKEY hKey, LPCWSTR lpFile, DWORD dwFlags = 0) { return -1;}; // = 0;
	virtual LONG GetSubkeyInfo(HKEY hKey, LPCWSTR lpszSubkey, LPTSTR pszDesc, DWORD cchMaxDesc, LPTSTR pszOwner, DWORD cchMaxOwner) { return -1;};
};
