
#include "header.h"

/*

	Объект предназначен для работы с текстовыми (Ansi/Unicode) и бинарными файлами.

*/

BOOL MFileTxt::bBadMszDoubleZero = FALSE;

MFileTxt::MFileTxt()
	: MRegistryBase()
{
	eType = RE_REGFILE;

	hFile = NULL; bLastRc = FALSE; nLastErr = 0; bOneKeyCreated = FALSE;
	bUnicode = cfg->bCreateUnicodeFiles;
	psTempDirectory = psFilePathName = NULL;
	psShowFilePathName = NULL;
	// Write buffer (cache)
	ptrWriteBuffer = NULL; nWriteBufferLen = nWriteBufferSize = 0;
	// Value export buffer
	pExportBufferData = pExportFormatted = pExportCPConvert = NULL;
	cbExportBufferData = cbExportFormatted = cbExportCPConvert = 0;
	// Exporting values
	pszExportHexValues = (wchar_t*)malloc(256*4);
	wchar_t sHex[] = L"0123456789abcdef";
	wchar_t *ph = pszExportHexValues;
	for (int i=0; i<16; i++) {
		for (int j=0; j<16; j++) {
			*(ph++) = sHex[i];
			*(ph++) = sHex[j];
		}
	}
	//memset(&TreeRoot, 0, sizeof(TreeRoot));
	
	//pszRootKeys[0] = L"HKEY_CLASSES_ROOT";
	//pszRootKeys[1] = L"HKEY_CURRENT_USER";
	//pszRootKeys[2] = L"HKEY_LOCAL_MACHINE";
	//pszRootKeys[3] = L"HKEY_USERS";
	//pszRootKeys[3] = L"HKEY_PERFORMANCE_DATA";
	//pszRootKeys[5] = L"HKEY_CURRENT_CONFIG";
	//pszRootKeys[6] = L"HKEY_DYN_DATA";
	//pszRootKeys[7] = L"HKEY_CURRENT_USER_LOCAL_SETTINGS";
}

MFileTxt::~MFileTxt()
{
	FileClose(); // Если еще не закрыт
	// Free blocks
	SafeFree(psTempDirectory);
	SafeFree(psFilePathName);
	SafeFree(psShowFilePathName);
	// Write buffer (cache)
	SafeFree(ptrWriteBuffer);
	// Value export buffer
	SafeFree(pExportBufferData);
	SafeFree(pExportFormatted);
	SafeFree(pExportCPConvert);
	SafeFree(pszExportHexValues);
}

#ifndef _UNICODE
/*static*/
// Должен возвращать длину строки + 4 байта (L"\0")
LONG MFileTxt::LoadText(LPCSTR asFilePathName, BOOL abUseUnicode, wchar_t** pszText, DWORD* pcbSize)
{
	wchar_t* pszW = lstrdup_w(asFilePathName);
	LONG lRc = LoadText(pszW, abUseUnicode, pszText, pcbSize);
	SafeFree(pszW);
	return lRc;
}
/*static*/
// Должен возвращать длину строк + 4 байта (L"\0\0")
LONG MFileTxt::LoadTextMSZ(LPCSTR asFilePathName, BOOL abUseUnicode, wchar_t** pszText, DWORD* pcbSize)
{
	wchar_t* pszW = lstrdup_w(asFilePathName);
	LONG lRc = LoadTextMSZ(pszW, abUseUnicode, pszText, pcbSize);
	SafeFree(pszW);
	return lRc;
}
/*static*/
LONG MFileTxt::LoadData(LPCSTR asFilePathName, void** pData, DWORD* pcbSize)
{
	wchar_t* pszW = lstrdup_w(asFilePathName);
	LONG lRc = LoadData(pszW, pData, pcbSize);
	SafeFree(pszW);
	return lRc;
}
#endif

/*static*/
// Должен возвращать длину строки + 4 байта (L"\0")
LONG MFileTxt::LoadText(LPCWSTR asFilePathName, BOOL abUseUnicode, wchar_t** pszText, DWORD* pcbSize)
{
	LONG lLoadRc = -1;
	//HANDLE lhFile = NULL;
	void* pData = NULL; DWORD cbSize = 0;
	
	
	_ASSERTE(pszText && pcbSize);
	_ASSERTE(*pszText == NULL);
	
	lLoadRc = LoadData(asFilePathName, &pData, &cbSize);
	if (lLoadRc != 0)
		return lLoadRc;
		
	//// Если размер менее 2 байт или не выровнен на 2 байта - считаем что текст НЕ юникодный?
	//if (cbSize < 2 || (abUseUnicode && ((cbSize>>1)<<1) != cbSize))
	//{
	//	//TODO: хотя это потенциальная засада? может быть пытаться отдавать наибольшее получившееся
	//	//TODO: количество символов wchar_t?
	//	if (abUseUnicode)
	//	{
	//		//TODO: проверить ветку
	//		*pcbSize = (cbSize*2) + 2; // размер в wchar_t + L'\0'
	//		*pszText = (wchar_t*)malloc(*pcbSize);
	//		_ASSERTE(cfg->nAnsiCodePage!=CP_UTF8 && cfg->nAnsiCodePage!=CP_UTF7);
	//		MultiByteToWideChar(cfg->nAnsiCodePage, 0, (char*)pData, cbSize, *pszText, cbSize);
	//		(*pszText)[cbSize] = 0; // + L'\0'
	//	} else {
	//		//TODO: проверить ветку
	//		*pcbSize = cbSize + 1; // размер в char + L'\0'
	//		*pszText = (wchar_t*)malloc(*pcbSize);
	//		memmove(*pszText, pData, cbSize);
	//		(*((char**)pszText))[cbSize] = 0; // + '\0'
	//	}
	//	SafeFree(pData);
	//	return 0; // OK
	//}
	
	lLoadRc = -1;

	//if (abUseUnicode)
	{
		// Проверить, что есть BOM
		WORD nBOM = 0xFEFF;
		if (abUseUnicode && cbSize >= 2 && (*((WORD*)pData) == nBOM) && (((cbSize>>1)<<1) == cbSize))
		{
			// Считаем, что в файле лежит юникодный текст
			//TODO: хотя это потенциальная засада? может быть пытаться отдавать наибольшее получившееся
			//TODO: количество символов wchar_t?
			cbSize = ((cbSize >> 1) - 1) << 1;
			*pcbSize = cbSize + 2; // размер в BYTE + L'\0' + не отдавать, но зарезервировать еще один L'\0' для MULTI_SZ
			*pszText = (wchar_t*)malloc(cbSize + 4); // MULTI_SZ ( + L"\0\0" )
			// Copy skipping BOM
			if (cbSize > 0)
				memmove(*pszText, ((wchar_t*)pData)+1, cbSize);
			(*pszText)[(cbSize>>1)] = 0; // + L'\0'
			(*pszText)[(cbSize>>1)+1] = 0; // + L'\0'
			lLoadRc = 0;
		}
		else
		{
			//TODO: проверить ветку
			// Считаем, что в файле лежит ansi текст
			*pcbSize = (cbSize*2) + 2; // размер в BYTE + L'\0' + не отдавать, но зарезервировать еще один L'\0' для MULTI_SZ
			*pszText = (wchar_t*)malloc((*pcbSize) + 2);
			_ASSERTE(cfg->nAnsiCodePage!=CP_UTF8 && cfg->nAnsiCodePage!=CP_UTF7);
			MultiByteToWideChar(cfg->nAnsiCodePage, 0, (char*)pData, cbSize, *pszText, cbSize);
			(*pszText)[cbSize] = 0; // + L'\0'
			(*pszText)[cbSize+1] = 0; // + L'\0'
			lLoadRc = 0;
		}
	}
	//else
	//{
	//	//TODO: проверить ветку
	//	// Хотят на выходе получить ANSI, считаем, что файл ANSI?
	//	*pcbSize = cbSize + 1; // размер в wchar_t + L'\0'
	//	*pszText = (wchar_t*)malloc(*pcbSize);
	//	memmove(*pszText, ((wchar_t*)pData)+1, cbSize);
	//	(*((char**)pszText))[cbSize] = 0; // + '\0'
	//	lLoadRc = 0;
	//}
	
	SafeFree(pData);
	return lLoadRc;
}

/*static*/
// Должен возвращать длину строк + 4 байта (L"\0\0")
LONG MFileTxt::LoadTextMSZ(LPCWSTR asFilePathName, BOOL abUseUnicode, wchar_t** pszText, DWORD* pcbSize)
{
	LONG lLoadRc = MFileTxt::LoadText(asFilePathName, abUseUnicode, pszText, pcbSize);
	if (lLoadRc != 0)
		return lLoadRc;

	// Преобразовать переводы строк \r\n в \0
	wchar_t* pszBase = *pszText;
	DWORD nLen = (*pcbSize) >> 1;
	wchar_t* pszEnd = pszBase + nLen;
	wchar_t* pszDst = pszBase;
	wchar_t* pszLnStart = pszBase;
	wchar_t* pszLnNext;
	wchar_t* pszLnEnd;
	while (pszLnStart < pszEnd)
	{
		if (*pszLnStart == 0 && (pszLnStart+1) >= pszEnd)
		{
			break;
		}
		//pszLnEnd = pszLnStart;
		//while (pszLnEnd < pszEnd)
		//{
		//	if (*pszLnEnd == L'\r' && *(pszLnEnd+1) == L'\n') {
		//		*pszLnEnd = 0;
		//		break;
		//	}
		//	pszLnEnd++;
		//}
		pszLnEnd = wcspbrk(pszLnStart, L"\r\n");
		if (!pszLnEnd) {
			pszLnEnd = pszEnd - 1;
			pszLnNext = pszEnd;
		} else {
			if (pszLnEnd[0] == L'\r' && pszLnEnd[1] == L'\n')
				pszLnNext = pszLnEnd + 2;
			else
				pszLnNext = pszLnEnd + 1;
		}

		if (pszDst != pszLnStart) {
			_ASSERTE(pszDst < pszLnStart);
			memmove(pszDst, pszLnStart, (pszLnEnd - pszLnStart)*2);
		}
		//*pszLnEnd = 0;

		pszDst += (pszLnEnd - pszLnStart);
		if (*pszLnNext == L'\r' || *pszLnNext == L'\n')
		{
			*(pszDst++) = L'\n';
		} else {
			*(pszDst++) = 0;
		}
		pszLnStart = pszLnNext;
	}
	_ASSERTE((pszDst) <= pszEnd);
	*(pszDst++) = 0;
	//*(pszDst+1) = 0;
	if (pszDst == pszBase)
		*pcbSize = 2; // пустой файл
	else
		*pcbSize = (pszDst - pszBase )*2;

	return lLoadRc;
}

/*static*/
LONG MFileTxt::LoadData(LPCWSTR asFilePathName, void** pData, DWORD* pcbSize, size_t ncbMaxSize /*= -1*/)
{
	LONG lLoadRc = -1;
	HANDLE lhFile = NULL;
	
	_ASSERTE(pData && pcbSize);
	_ASSERTE(*pData == NULL);

	if (!asFilePathName || !*asFilePathName || !pData || !pcbSize)
	{
		InvalidOp();
		return E_INVALIDARG;
	}

	//#ifdef _UNICODE
	if (asFilePathName[0]==L'\\' && asFilePathName[1]==L'\\' && asFilePathName[2]==L'?' && asFilePathName[3]==L'\\')
	{
		lhFile = CreateFileW( asFilePathName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
		lLoadRc = GetLastError();
	}
	else
	{
		#ifdef _DEBUG
		int nLen = lstrlenW(asFilePathName);
		#endif
		//wchar_t* pszUNC = (wchar_t*)malloc((nLen+32)*2);
		//CopyPath(pszUNC, asFilePathName, nLen+32);
		wchar_t* pszUNC = MakeUNCPath(asFilePathName);
		lhFile = CreateFileW( pszUNC, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
		lLoadRc = GetLastError();
		SafeFree(pszUNC);
	}
	//#else
	//	lhFile = CreateFileA( asFilePathName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	//	lLoadRc = GetLastError();
	//#endif
	if (lhFile == INVALID_HANDLE_VALUE)
	{
		//lLoadRc = GetLastError();
		REPlugin::MessageFmt(REM_CantOpenFileReading, asFilePathName, lLoadRc);
		return (lLoadRc == 0) ? -1 : lLoadRc;
	}
	
	LARGE_INTEGER lSize;
	if (!GetFileSizeEx(lhFile, &lSize))
	{
		lLoadRc = GetLastError();
		CloseHandle(lhFile);
		return (lLoadRc == 0) ? -1 : lLoadRc;
	}
	// Таких больших файлов в случае реестра быть не может
	if (lSize.HighPart)
	{
		CloseHandle(lhFile);
		return ERROR_NOT_ENOUGH_MEMORY;
	}

	if (ncbMaxSize != (size_t)-1 && lSize.LowPart > ncbMaxSize)
		lSize.LowPart = ncbMaxSize;
	
	*pcbSize = lSize.LowPart;
	*pData = (void*)malloc(lSize.LowPart+3); // Страховка для строковых функций в MFileReg
	if (*pData == NULL)
	{
		CloseHandle(lhFile);
		REPlugin::MemoryAllocFailed(lSize.LowPart);
		return ERROR_NOT_ENOUGH_MEMORY;
	}
	
	DWORD nRead = 0;
	if (!ReadFile(lhFile, *pData, lSize.LowPart, &nRead, NULL))
	{
		lLoadRc = GetLastError();
		if (lLoadRc == 0) lLoadRc = -1;
		SafeFree(*pData);
		REPlugin::MessageFmt(REM_CantReadFile, asFilePathName, lLoadRc);
	}
	else
	{
		lLoadRc = 0;
		// Страховка для строковых функций в MFileReg
		((LPBYTE)*pData)[lSize.LowPart] = 0;
		((LPBYTE)*pData)[lSize.LowPart+1] = 0;
		((LPBYTE)*pData)[lSize.LowPart+2] = 0;
	}
	
	CloseHandle(lhFile);
	return lLoadRc;
}


// Text file operations
// asExtension = L".reg", L".txt"
BOOL MFileTxt::FileCreateTemp(LPCWSTR asDefaultName, LPCWSTR asExtension, BOOL abUnicode)
{
	//HANDLE hFile = NULL;
	int nLen;
	TCHAR szTempDir[MAX_PATH*2+1] = _T("");
	wchar_t szOutFileName[MAX_PATH*3+20] = L"";
	
	// На всякий случай, подчистим переменные
	_ASSERTE(psTempDirectory == NULL);
	SafeFree(psTempDirectory);
	_ASSERTE(psFilePathName == NULL);
	SafeFree(psFilePathName);
	_ASSERTE(psShowFilePathName == NULL);
	SafeFree(psShowFilePathName);
	_ASSERTE(hFile == NULL);
	// И закроем файл
	FileClose();
	
	//_ASSERTE(asExtension && asExtension[0] == L'.' && asExtension[1] != 0);
	if (asDefaultName == NULL)
		asDefaultName = REGEDIT_DEFAULTNAME;
	if (!asExtension)
		asExtension = L""; //L".txt"; -- без расширения, чтобы колорер нормально раскраску подхватывал
	
	// Создать временный каталог
	FSF.MkTemp(szTempDir,
		#ifdef _UNICODE
			MAX_PATH*2,
		#endif
		_T("FREG"));
	_ASSERTE(szTempDir[0] != 0);

	// Сразу запомнить (psTempDirectory), что создали временную папку
	psTempDirectory = MakeUNCPath(szTempDir);
	bLastRc = CreateDirectoryW(psTempDirectory, NULL);
	if (!bLastRc)
	{
		nLastErr = GetLastError();
		REPlugin::MessageFmt(REM_CantCreateTempFolder, psTempDirectory, nLastErr);
		SafeFree(psTempDirectory);
		return (bLastRc = FALSE);
	}

	lstrcpynW(szOutFileName, psTempDirectory, MAX_PATH*2+20);
	nLen = lstrlenW(szOutFileName);
	_ASSERTE(szOutFileName[nLen-1] != _T('\\'));
	szOutFileName[nLen++] = _T('\\'); szOutFileName[nLen] = 0;
	CopyFileName(szOutFileName+nLen, MAX_FILE_NAME-5, asDefaultName);
	lstrcpynW(szOutFileName+lstrlenW(szOutFileName), asExtension, 5);
		
	// Создаем файл (имя файла скопирует он сам)
	bLastRc = FileCreateApi(szOutFileName, abUnicode, FALSE/*abAppendExisting*/);
	
	return bLastRc;
}

BOOL MFileTxt::FileCreate(LPCWSTR asPath/*only directory!*/, LPCWSTR asDefaultName, LPCWSTR asExtension, BOOL abUnicode, BOOL abConfirmOverwrite)
{
	int nLen, nFullLen;

	// На всякий случай, подчистим переменные
	_ASSERTE(psTempDirectory == NULL);
	SafeFree(psTempDirectory);
	_ASSERTE(psFilePathName == NULL);
	SafeFree(psFilePathName);
	_ASSERTE(psShowFilePathName == NULL);
	SafeFree(psShowFilePathName);
	_ASSERTE(hFile == NULL);
	// И закроем файл
	FileClose();

	//_ASSERTE(asExtension && asExtension[0] == L'.' && asExtension[1] != 0);
	if (asDefaultName == NULL)
		asDefaultName = REGEDIT_DEFAULTNAME;
	if (!asExtension)
		asExtension = L""; //L".txt"; -- без расширения, чтобы колорер нормально раскраску подхватывал

	if (!asPath || !*asPath) {
		//TODO: Использовать текущую папку фара (получать через API)
		_ASSERTE(asPath && *asPath);
		return FALSE;
	} else {
		//TODO: Развернуть возможные ".\", "..\" и т.п.
		nLen = lstrlenW(asPath);
		//if (asPath[nLen-1] == _T('\\')) nLen--;
	}
	// CopyFileName может корректировать asDefaultName (заменять некорректные символы на #xx)
	nFullLen = nLen + MAX_PATH/*lstrlenW(asDefaultName)*/ + lstrlenW(asExtension) + 20;
	psFilePathName = (wchar_t*)malloc(nFullLen*sizeof(wchar_t));

	nLen = CopyUNCPath(psFilePathName, nFullLen, asPath);
	if (psFilePathName[nLen-1] == L'\\') {
		psFilePathName[nLen--] = 0;
	}

	//TODO: Проверить, может директория уже создана
	bLastRc = CreateDirectoryW(psFilePathName, NULL);
	//if (!bLastRc) {
	//	nLastErr = GetLastError();
	//	return (bLastRc = FALSE);
	//}

	//lstrcpy_t(szOutName+nLen, (MAX_FILE_NAME-5), asDefaultName);
	psFilePathName[nLen++] = L'\\';
	CopyFileName(psFilePathName+nLen, MAX_FILE_NAME-((*asExtension) ? 5 : 1), asDefaultName);
	if (*asExtension)
		lstrcpynW(psFilePathName+lstrlenW(psFilePathName), asExtension, 5);

	// Если abConfirmOverwrite и файл существует - подтверждение на перезапись!
	BOOL lbAppendExisting = FALSE;
	if (abConfirmOverwrite)
	{
		if (!REPlugin::ConfirmOverwriteFile(psFilePathName, &lbAppendExisting, &abUnicode))
		{
			bLastRc = TRUE; nLastErr = 0;
			return FALSE;
		}
	}

	// Создаем файл (имя файла скопирует он сам)
	bLastRc = FileCreateApi(psFilePathName, abUnicode, lbAppendExisting);

	return bLastRc;
}

LPCWSTR MFileTxt::GetFilePathName()
{
	_ASSERTE(psFilePathName != NULL);
	return psFilePathName;
}

LPCTSTR MFileTxt::GetShowFilePathName()
{
	_ASSERTE(psFilePathName != NULL || psShowFilePathName != NULL);
	if (!psShowFilePathName && psFilePathName) {
		psShowFilePathName = UnmakeUNCPath_t(psFilePathName);
	}
	return psShowFilePathName;
}

BOOL MFileTxt::FileCreateApi(LPCWSTR asFilePathName, BOOL abUnicode, BOOL abAppendExisting)
{
	FileClose(); // На всякий случай
	
	MFileTxt::bBadMszDoubleZero = FALSE;
	
	// Сразу!
	bUnicode = abUnicode;
	bOneKeyCreated = FALSE;
	if (asFilePathName != psFilePathName)
	{
		SafeFree(psFilePathName);
		psFilePathName = lstrdup(asFilePathName);
	}
	nAnsiCP = cfg->nAnsiCodePage;

	int nNameLen = lstrlenW(asFilePathName)+1;
	psShowFilePathName = (TCHAR*)malloc(nNameLen*sizeof(TCHAR));

	//if (wcsncmp(asFilePathName, _T("\\\\?\\"), 4))
	if (asFilePathName[0 ]== _T('\\')
		&& asFilePathName[1] == _T('\\')
		&& asFilePathName[2] == _T('?') 
		&& asFilePathName[3] == _T('\\'))
	{
		LPCWSTR pFileName = asFilePathName+4;
		if (pFileName[0] == L'U' && pFileName[1] == L'N' && pFileName[2] == L'C' && pFileName[3] == L'\\')
		{
			*psShowFilePathName = '\\';
			lstrcpy_t(psShowFilePathName+1, nNameLen-1, pFileName+3);
		}
		else
		{
			lstrcpy_t(psShowFilePathName, nNameLen, pFileName);
		}
	}
	else
	{
		lstrcpy_t(psShowFilePathName, nNameLen, asFilePathName);
	}
	MCHKHEAP;
	
	if (!ptrWriteBuffer || nWriteBufferSize < 1048576 /* 1Mb */)
	{
		SafeFree(ptrWriteBuffer);
		nWriteBufferSize = 1048576;
		nWriteBufferLen = 0;
		ptrWriteBuffer = (LPBYTE)malloc(nWriteBufferSize);
	}

	// здесь уже должен быть путь в UNC формате!
	_ASSERTE(asFilePathName[0]==L'\\' && asFilePathName[1]==L'\\' && asFilePathName[2]==L'?' && asFilePathName[3]==L'\\');
	// Создаем файл
	hFile = CreateFileW( asFilePathName, GENERIC_WRITE, FILE_SHARE_READ, NULL, 
						(abAppendExisting ? OPEN_ALWAYS : CREATE_ALWAYS),
						FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		nLastErr = GetLastError();
		if (!(gnOpMode & (OPM_FIND|OPM_SILENT)))
			REPlugin::MessageFmt(REM_CantCreateTempFile, asFilePathName, nLastErr);
		hFile = NULL;
		return (bLastRc = FALSE);
	}
	
	// Если это НЕ дописывание в конец
	if (abUnicode && !abAppendExisting)
	{
		WORD nBOM = 0xFEFF;
		if (!FileWriteBuffered(&nBOM, sizeof(nBOM)))
			return FALSE;
	}

	// Это дописывание в конец
	if (abAppendExisting)
	{
		// Переместить указатель в конец файла
		if (SetFilePointer(hFile, 0, NULL, FILE_END) == INVALID_SET_FILE_POINTER)
			return FALSE;
		// И на всякий случай, вставить перевод строки
		if (abUnicode)
		{
			if (!FileWriteBuffered(L"\r\n", 4))
				return FALSE;
		}
		else
		{
			if (!FileWriteBuffered("\r\n", 2))
				return FALSE;
		}
	}
		
	return (bLastRc = TRUE);
}

void MFileTxt::FileClose()
{
	if (hFile && hFile != INVALID_HANDLE_VALUE) {
		// Если что-то осталось в кеше записи
		if (ptrWriteBuffer && nWriteBufferLen) {
			//FileWriteBuffered(NULL, -1); // сбросит кеш на диск
			Flush(); // сбросит кеш на диск
		}
		// И закрыть дескриптор
		CloseHandle(hFile); hFile = NULL;
	}
	SafeFree(ptrWriteBuffer);
	SafeFree(pExportFormatted);
}

void MFileTxt::FileDelete()
{
	// Если еще не закрыт
	FileClose();

	// Удалить файл
	if (psFilePathName) {
		if (*psFilePathName) {
			bLastRc = DeleteFileW(psFilePathName);
			if (!bLastRc) nLastErr = GetLastError();
		} else {
			bLastRc = FALSE; nLastErr = -1;
		}
		SafeFree(psFilePathName);
	}
	
	// Удалить созданную нами временную папку
	if (psTempDirectory) {
		if (*psTempDirectory) {
			int nLen = lstrlenW(psTempDirectory);
			if (psTempDirectory[nLen-1] == _T('\\')) {
				psTempDirectory[nLen-1] = 0; nLen--;
			}
			if (psTempDirectory[nLen-1] == _T(':')) {
				bLastRc = TRUE;
			} else {
				bLastRc = RemoveDirectoryW(psTempDirectory);
				if (!bLastRc) nLastErr = GetLastError();
			}
		} else {
			bLastRc = FALSE; nLastErr = -1;
		}
		SafeFree(psTempDirectory);
	}
}

BOOL MFileTxt::Flush()
{
	_ASSERTE(ptrWriteBuffer != NULL);
	if (nWriteBufferLen > 0 && ptrWriteBuffer) {
		DWORD nWritten = 0;
		bLastRc = WriteFile(hFile, ptrWriteBuffer, nWriteBufferLen, &nWritten, NULL);
		if (!bLastRc) {
			nLastErr = GetLastError();
			return FALSE;
		}
		nWriteBufferLen = 0;
	}
	return bLastRc;
}

BOOL MFileTxt::FileWriteRegHeader(MRegistryBase* pWorker)
{
	BOOL lbExportRc = TRUE;

	if (bUnicode) {
		// (BOM уже записан в file.FileCreateTemp)
		lbExportRc = FileWrite(L"Windows Registry Editor Version 5.00\r\n");
	} else {
		LPCSTR pszHeader = "REGEDIT4\r\n";
		int nSize = lstrlenA(pszHeader);
		lbExportRc = FileWriteBuffered(pszHeader, nSize);
	}

	if (lbExportRc && pWorker && pWorker->eType == RE_HIVE)
	{
		//lbExportRc = file.FileWriteBuffered(pszHeader, nSize);
		//lbExportRc = file.FileWriteBuffered(pszHeader, nSize);
		wchar_t* pwszHost = UnmakeUNCPath_w(((MFileHive*)pWorker)->GetFilePathName());
		lbExportRc = FileWrite(L"\r\n;RootFile=") &&
			FileWrite(pwszHost) &&
			FileWrite(L"\r\n");
		SafeFree(pwszHost);
	}

	return lbExportRc;
}

BOOL MFileTxt::FileWriteBuffered(LPCVOID apData, DWORD nDataSize)
{
	_ASSERTE(ptrWriteBuffer != NULL);
	DWORD nWritten = 0;
	
	if (!apData)
	{
		// Сбросить на диск
		_ASSERTE(apData == NULL && nDataSize == (DWORD)-1);
		Flush();
		return (bLastRc = TRUE);
	}
	// Если ничего не нужно
	if (nDataSize == 0)
	{
		return (bLastRc = TRUE);
	}
	
	if (ptrWriteBuffer)
	{
		int nLeft = nWriteBufferSize - nWriteBufferLen;
		_ASSERTE(nLeft >= 0);
		
		// Если буфер заполнился (почему? он уже должен был быть сброшен)
		// или размер данных превышает оставшийся свободный блок
		if (nLeft <= 0 || nDataSize >= (DWORD)nLeft)
		{
			// Сбросить текущий буфер на диск
			//if (!FileWriteBuffered(NULL, -1))
			if (!Flush())
				return FALSE;
			_ASSERTE(nWriteBufferLen==0);
			nWriteBufferLen = 0;
			nLeft = nWriteBufferSize;
		}
		
		// Если размер данных превышает (или равен) размеру выделенного буфера - сразу пишем на диск
		if (nDataSize >= nWriteBufferSize)
		{
			_ASSERTE(nWriteBufferLen == 0);
			bLastRc = WriteFile(hFile, apData, nDataSize, &nWritten, NULL);
			if (!bLastRc) nLastErr = GetLastError();
			return bLastRc; // Сразу выходим, уже записали или ошибка
		}

		// Скопировать в буфер и передвинуть указатель
		_ASSERTE(nDataSize <= (DWORD)nLeft && nLeft > 0);
		memmove(ptrWriteBuffer+nWriteBufferLen, apData, nDataSize);
		nWriteBufferLen += nDataSize;
		
		// Если буфер заполнился - сбросить на диск
		if (nWriteBufferLen == nWriteBufferSize)
		{
			//if (!FileWriteBuffered(NULL, -1))
			if (!Flush())
				return FALSE;
			_ASSERTE(nWriteBufferLen==0);
			nWriteBufferLen = 0;
		}
		bLastRc = TRUE;
		
	}
	else
	{
		// Буфер не был создан?
		bLastRc = WriteFile(hFile, apData, nDataSize, &nWritten, NULL);
		if (!bLastRc) nLastErr = GetLastError();
	}
	
	return bLastRc;
}

BOOL MFileTxt::FileWriteMSZ(LPCWSTR aszText, DWORD anLen)
{
	const wchar_t* psz = aszText;
	const wchar_t* pszEnd = aszText + anLen;
	#ifdef _DEBUG
	if (pszEnd > psz)
	{
		_ASSERTE(*(pszEnd-1) == 0);
	}
	#endif
	BOOL lbExportRc = TRUE; // Может быть пустой!
	int nDoubleZero = 0;
	while (psz < pszEnd)
	{
		if (*psz)
		{
			int nLen = lstrlenW(psz);
			if (!(lbExportRc = FileWrite(psz, nLen)))
				break; // ошибка записи
			psz += nLen+1;
			if (nDoubleZero>0) MFileTxt::bBadMszDoubleZero = TRUE;
			nDoubleZero = 0;
		}
		else
		{
			if ((psz+1) >= pszEnd)
				break;
			psz++; // просто записать перевод строки
			nDoubleZero++;
		}
		// Просто "\n" пишется для унификации пустых строк - они в реестр заносятся как "\n"
		if (!(lbExportRc = FileWrite(L"\n", 1)))
			break; // ошибка записи
	}
	if (nDoubleZero>1) MFileTxt::bBadMszDoubleZero = TRUE;
	return lbExportRc;
}

BOOL MFileTxt::FileWrite(LPCWSTR aszText, int anLen/*=-1*/)
{
	if (aszText == NULL)
	{
		_ASSERTE(aszText != NULL);
		nLastErr = -1;
		return (bLastRc = FALSE);
	}
	
	if (anLen == -1)
	{
		anLen = lstrlenW(aszText);
	}
	_ASSERTE(anLen >= 0);
	
	// Если писать нечего
	if (anLen == 0)
	{
		nLastErr = 0;
		return (bLastRc = TRUE);
	}

	if (bUnicode)
	{
		bLastRc = FileWriteBuffered(aszText, anLen*2);
		if (!bLastRc) nLastErr = GetLastError();
	}
	else
	{
		_ASSERTE(anLen < (anLen*3));
		char* pszAscii = (char*)GetConvertBuffer(anLen*3); // Может кто-то UTF-8 захочет?
		int nCvtLen = WideCharToMultiByte(nAnsiCP/*CP_ACP*/, 0, aszText, anLen, pszAscii, anLen*3, 0,0);
		bLastRc = FileWriteBuffered(pszAscii, nCvtLen);
		if (!bLastRc) nLastErr = GetLastError();
	}
	
	return bLastRc;
}

BOOL MFileTxt::FileWriteValue(LPCWSTR pszValueName, REGTYPE nDataType, const BYTE* pData, DWORD nDataSize, LPCWSTR pszComment)
{
	wchar_t* psz = (wchar_t*)GetFormatBuffer(max(0xFFFF,nDataSize));
	
	if (nDataType == REG__KEY)
	{
		_ASSERTE(nDataType!=REG__KEY);
		return FALSE;
	}

	if (nDataType == REG__COMMENT)
	{
		if (!bOneKeyCreated)
		{
			bOneKeyCreated = TRUE;
			if (!FileWrite(L"\r\n", 2))
				return FALSE;
		}

		if (!FileWrite(pszValueName, -1))
			return FALSE;
		//if (pszComment && !FileWrite(pszComment, -1))
		//	return FALSE;
		if (!FileWrite(L"\r\n", 2))
			return FALSE;
		return TRUE;
	}

	if (!bUnicode && nDataSize && (nDataType == REG_EXPAND_SZ || nDataType == REG_MULTI_SZ))
	{
		size_t nLen = (nDataSize>>1);
		_ASSERTE(nLen < (nLen*3));
		char* pszAscii = (char*)GetConvertBuffer(nLen*3+1); // Может кто-то UTF-8 захочет?
		int nCvtLen = WideCharToMultiByte(nAnsiCP/*CP_ACP*/, 0, (LPCWSTR)pData, nLen, pszAscii, nLen*3+1, 0,0);
		_ASSERTE(nCvtLen > 0);
		pData = (LPBYTE)pszAscii; nDataSize = nCvtLen;
	}

	if (pszValueName && *pszValueName)
	{
		*(psz++) = L'\"';
		//lstrcpyW(psz, pszValueName); psz += lstrlenW(pszValueName);
		// escaped
		BOOL bCvtRN = cfg->bEscapeRNonExporting;
		const wchar_t* pszSrc = (wchar_t*)pszValueName;
		wchar_t ch;
		while ((ch = *(pszSrc++)) != 0)
		{
			switch (ch)
			{
			case L'\r':
				if (bCvtRN)
				{
					*(psz++) = L'\\'; *(psz++) = L'r';
				}
				else
				{
					*(psz++) = ch;
				}
				break;
			case L'\n':
				if (bCvtRN)
				{
					*(psz++) = L'\\'; *(psz++) = L'n';
				}
				else
				{
					*(psz++) = ch;
				}
				break;
			//TODO: заменять символ табуляции?
			//case L'\t':
			//	*(psz++) = L'\\'; *(psz++) = L't'; break;
			case L'\"':
				*(psz++) = L'\\'; *(psz++) = L'"'; break;
			case L'\\':
				*(psz++) = L'\\'; *(psz++) = L'\\'; break;
			default:
				*(psz++) = ch;
			}
		}
		*(psz++) = L'\"';
	}
	else
	{
		*(psz++) = L'@';
	}
	*(psz++) = L'=';

	//wchar_t* pszValueStart = psz;

	if (nDataType == REG__DELETE)
	{
		*(psz++) = L'-';
	}
	else if (nDataType == REG_SZ)
	{
		//TODO: Если встречается перевод каретки - можно опционально переформатировать в режиме "hex(1):"
		const wchar_t* pszSrc = (wchar_t*)pData;
		wchar_t ch;
		DWORD nLen = nDataSize>>1;
		if (nLen>0 && pszSrc[nLen-1] == 0) nLen--;
		*(psz++) = L'\"';
		BOOL bEscRN = cfg->bEscapeRNonExporting;
		while (nLen--)
		{
			switch ((ch = *(pszSrc++)))
			{
			case 0:
				*(psz++) = L'\\'; *(psz++) = L'0';
				break;
			case L'\r':
				if (bEscRN)
				{
					*(psz++) = L'\\'; *(psz++) = L'r';
				}
				else
				{
					*(psz++) = ch;
				}
				break;
			case L'\n':
				if (bEscRN)
				{
					*(psz++) = L'\\'; *(psz++) = L'n';
				}
				else
				{
					*(psz++) = ch;
				}
				break;
			//TODO: заменять символ табуляции?
			//case L'\t':
			//	*(psz++) = L'\\'; *(psz++) = L't'; break;
			case L'\"':
				*(psz++) = L'\\'; *(psz++) = L'"'; break;
			case L'\\':
				*(psz++) = L'\\'; *(psz++) = L'\\'; break;
			default:
				*(psz++) = ch;
			}
		}
		*(psz++) = L'\"';

	}
	else if (nDataType == REG_DWORD && nDataSize == 4)
	{
		//TODO: ручками
		wsprintfW(psz, L"dword:%08x", *((DWORD*)pData));
		psz += lstrlenW(psz);
		
	}
	else
	{
		if (nDataType == REG_BINARY)
		{
			*(psz++) = L'h'; *(psz++) = L'e'; *(psz++) = L'x'; *(psz++) = L':';
		}
		else
		{
			wsprintfW(psz, L"hex(%x):", nDataType);
			psz += lstrlenW(psz);
		}
		if (nDataSize)
		{
			wchar_t* ph = pszExportHexValues+(*(pData++))*2;
			*(psz++) = *(ph++);
			*(psz++) = *(ph++);
			wchar_t* pszLineStart = (wchar_t*)pExportFormatted;
			for (UINT n = 1; n < nDataSize; n++)
			{
				ph = pszExportHexValues+(*(pData++))*2;
				*(psz++) = L',';
				if ((psz - pszLineStart) >= 77)
				{
					*(psz++) = L'\\'; *(psz++) = L'\r'; *(psz++) = L'\n'; *(psz++) = L' '; *(psz++) = L' ';
					pszLineStart = psz - 2;
				}
				*(psz++) = *(ph++);
				*(psz++) = *(ph++);
			}
		}
	}
	//*(psz++) = L'\r'; *(psz++) = L'\n';
	
	int nLen = (int)(psz - ((wchar_t*)pExportFormatted));

	if (!FileWrite((LPCWSTR)pExportFormatted, nLen))
		return FALSE;

	if (pszComment && !FileWrite(pszComment, -1))
		return FALSE;

	if (!FileWrite(L"\r\n", 2))
		return FALSE;

	return TRUE;
}

LPBYTE MFileTxt::GetExportBuffer(DWORD cbSize) // --> pExportBufferData
{
	if (!pExportBufferData || cbSize > cbExportBufferData)
	{
		SafeFree(pExportBufferData);
		cbExportBufferData = cbSize;
		pExportBufferData = (LPBYTE)malloc(cbExportBufferData);
	}
	return pExportBufferData;
}

LPBYTE MFileTxt::GetFormatBuffer(DWORD cbSize) // --> pExportFormatted
{
	// буфер должен быть достаточно большим для принятия 
	// отформатированных hex данных и имени значения реестра
	unsigned __int64 tSize = cbSize*16+0x10000;
	if (tSize != (tSize & 0xFFFFFFFF))
	{
		// В принципе, размер значений ограничен только объемом памяти
		// но должны же быть какие-то пределы!
		_ASSERTE(tSize == (tSize & 0xFFFFFFFF));
		return NULL;
	}
	else
	{
		cbSize = (DWORD)tSize;
	}
	
	if (!pExportFormatted || cbSize > cbExportFormatted) {
		SafeFree(pExportFormatted);
		cbExportFormatted = cbSize;
		pExportFormatted = (LPBYTE)malloc(cbExportFormatted);
	}
	return pExportFormatted;
}

LPBYTE MFileTxt::GetConvertBuffer(DWORD cbSize) // --> pExportCPConvert
{
	_ASSERTE(cbSize!=0);
	
	if (!pExportCPConvert || cbSize > cbExportCPConvert)
	{
		SafeFree(pExportCPConvert);
		cbExportCPConvert = cbSize;
		pExportCPConvert = (LPBYTE)malloc(cbExportCPConvert);
	}
	return pExportCPConvert;
}

// ***
MRegistryBase* MFileTxt::Duplicate()
{
	_ASSERTE(FALSE);
	InvalidOp();
	return NULL;
}

//LONG MFileTxt::NotifyChangeKeyValue(RegFolder *pFolder, HKEY hKey)
//{
//	return 0;
//}

// Wrappers
LONG MFileTxt::CreateKeyEx(
		HKEY hKey, LPCWSTR lpSubKey, DWORD Reserved, LPWSTR lpClass, DWORD dwOptions, REGSAM samDesired,
		LPSECURITY_ATTRIBUTES lpSecurityAttributes, PHKEY phkResult, LPDWORD lpdwDisposition, DWORD *pnKeyFlags,
		RegKeyOpenRights *apRights /*= NULL*/, LPCWSTR pszComment /*= NULL*/)
{
	if (!FileWrite(L"\r\n", 2))
		return -1;

	BOOL bKeyDeletion = (dwOptions & REG__OPTION_CREATE_DELETED) == REG__OPTION_CREATE_DELETED;

	bOneKeyCreated = TRUE;

	wchar_t sTemp[64];
	wchar_t* pszTemp = sTemp;
	*(pszTemp++) = L'[';
	if (bKeyDeletion)
		*(pszTemp++) = L'-';
	if (hKey == NULL)
	{
		if (!lpSubKey || !*lpSubKey)
		{
			_ASSERTE(lpSubKey && *lpSubKey);
			return -1;
		}

		*pszTemp = 0;
		if (!FileWrite(sTemp) ||
			!FileWrite(lpSubKey))
			return -1;
	}
	else
	{
		HKeyToStringKey(hKey, pszTemp, 40);

		// Заголовок ключа
		if (!FileWrite(sTemp))
			return -1;

		if (lpSubKey && lpSubKey[0])
		{
			if (!FileWrite(L"\\", 1) ||
				!FileWrite(lpSubKey))
				return -1;
		}
	}
	if (pszComment)
	{
		if (!FileWrite(L"]", 1) ||
			!FileWrite(pszComment) ||
			!FileWrite(L"\r\n", 2))
			return -1;
	}
	else
	{
		if (!FileWrite(L"]\r\n", 3))
			return -1;
	}

	// Что-то отличное от NULL нужно вернуть!
	*phkResult = HKEY__HIVE;

	//bKeyWasCreated = TRUE;
	return 0;
}
LONG MFileTxt::OpenKeyEx(HKEY hKey, LPCWSTR lpSubKey, DWORD ulOptions, REGSAM samDesired, PHKEY phkResult, DWORD *pnKeyFlags, RegKeyOpenRights *apRights /*= NULL*/)
{
	return -1;
}
LONG MFileTxt::CloseKey(HKEY hKey)
{
	return 0;
}
LONG MFileTxt::QueryInfoKey(HKEY hKey, LPWSTR lpClass, LPDWORD lpcClass, LPDWORD lpReserved, LPDWORD lpcSubKeys, LPDWORD lpcMaxSubKeyLen, LPDWORD lpcMaxClassLen, LPDWORD lpcValues, LPDWORD lpcMaxValueNameLen, LPDWORD lpcMaxValueLen, LPDWORD lpcbSecurityDescriptor, REGFILETIME* lpftLastWriteTime)
{
	return -1;
}
LONG MFileTxt::EnumValue(HKEY hKey, DWORD dwIndex, LPWSTR lpValueName, LPDWORD lpcchValueName, LPDWORD lpReserved, REGTYPE* lpDataType, LPBYTE lpData, LPDWORD lpcbData, BOOL abEnumComments, LPCWSTR* ppszValueComment /*= NULL*/)
{
	// pParent->dwValIndex = -1; pParent->pValIndex = -1;
	return -1;
}
LONG MFileTxt::EnumKeyEx(HKEY hKey, DWORD dwIndex, LPWSTR lpName, LPDWORD lpcName, LPDWORD lpReserved, LPWSTR lpClass, LPDWORD lpcClass, REGFILETIME* lpftLastWriteTime, DWORD* pnKeyFlags /*= NULL*/, TCHAR* lpDefValue /*= NULL*/, DWORD cchDefValueMax /*= 0*/, LPCWSTR* ppszKeyComment /*= NULL*/)
{
	// pParent->dwKeyIndex = -1; pParent->pKeyIndex = -1;
	return -1;
}
LONG MFileTxt::QueryValueEx(HKEY hKey, LPCWSTR lpValueName, LPDWORD lpReserved, REGTYPE* lpDataType, LPBYTE lpData, LPDWORD lpcbData, LPCWSTR* ppszValueComment /*= NULL*/)
{
	return -1;
}
LONG MFileTxt::SetValueEx(HKEY hKey, LPCWSTR lpValueName, DWORD Reserved, REGTYPE nDataType, const BYTE *lpData, DWORD cbData, LPCWSTR pszComment /*= NULL*/)
{
	BOOL lbRc = FileWriteValue(lpValueName, nDataType, lpData, cbData, pszComment);
	return (lbRc ? 0 : -1);
}
LONG MFileTxt::RenameKey(RegPath* apParent, BOOL abCopyOnly, LPCWSTR lpOldSubKey, LPCWSTR lpNewSubKey, BOOL* pbRegChanged)
{
	return -1;
}

//int MFileTxt::CopyPath(wchar_t* pszDest, const char* pszSrc, int nMaxChars)
//{
//	bool lbIsFull = false, lbIsNetwork = false;
//	if (pszSrc[0] == '\\' && pszSrc[1] == '\\') {
//		if ((pszSrc[2] == '?' || pszSrc[2] == '.') && pszSrc[3] == '\\') {
//			lbIsFull = true;
//		} else {
//			lbIsNetwork = true;
//		}
//	}
//	// "\\?\" X:\...
//	// "\\?\UNC\" server\share\...
//	if (lbIsFull) {
//		lstrcpy_t(pszDest, nMaxChars, pszSrc);
//	} else {
//		lstrcpyW(pszDest, L"\\\\?\\");
//		if (lbIsNetwork) {
//			lstrcpyW(pszDest+4, L"UNC\\");
//			lstrcpy_t(pszDest+8, nMaxChars-8, pszSrc+2);
//		} else {
//			lstrcpy_t(pszDest+4, nMaxChars-4, pszSrc);
//		}
//	}
//
//	int nLen = lstrlenW(pszDest);
//	return nLen;
//}
//
//int MFileTxt::CopyPath(wchar_t* pszDest, const wchar_t* pszSrc, int nMaxChars)
//{
//	bool lbIsFull = false, lbIsNetwork = false;
//	if (pszSrc[0] == L'\\' && pszSrc[1] == L'\\') {
//		if ((pszSrc[2] == L'?' || pszSrc[2] == L'.') && pszSrc[3] == L'\\') {
//			lbIsFull = true;
//		} else {
//			lbIsNetwork = true;
//		}
//	}
//	// "\\?\" X:\...
//	// "\\?\UNC\" server\share\...
//	if (lbIsFull) {
//		lstrcpyW(pszDest, pszSrc);
//	} else {
//		lstrcpyW(pszDest, L"\\\\?\\");
//		if (lbIsNetwork) {
//			lstrcpyW(pszDest+4, L"UNC\\");
//			lstrcpyW(pszDest+8, pszSrc+2);
//		} else {
//			lstrcpyW(pszDest+4, pszSrc);
//		}
//	}
//
//	int nLen = lstrlenW(pszDest);
//	return nLen;
//}
