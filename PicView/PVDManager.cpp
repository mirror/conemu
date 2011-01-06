
/**************************************************************************
Copyright (c) 2009 Skakov Pavel
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

#include "PVDManager.h"
#include "PictureView_Lang.h"
//#include <shlwapi.h>

std::vector<ModuleData*> PVDManager::Plugins;
std::vector<ModuleData*> PVDManager::Decoders;
std::vector<ModuleData*> PVDManager::Displays;
bool PVDManager::bCancelDecoding = false;
ModuleData* PVDManager::pDefaultDisplay = NULL; // Что выбрано в настройке плагина - поле "Display module"
ModuleData* PVDManager::pActiveDisplay = NULL;  // А это то, через что идет вывод СЕЙЧАС (может быть NULL)
std::vector<wchar_t*> PVDManager::sWholeLog;
bool PVDManager::bWholeLogChanged = false;

PVDManager::PVDManager(ImageInfo* apImage)
{
	mp_Image = apImage;
	mi_SubDecoder = 0;
	mp_ImageContext = NULL;
	mp_Data = NULL;
	mb_ImageOpened = false;
	mps_FileName = NULL;
	mb_Processed = NULL; mn_ProcessedSize = 0;
}

PVDManager::~PVDManager()
{
	if (mb_ImageOpened)
		Close();
	if (mp_Data) // pData это ссылка на pPlugin, а они освобождаются в PVDManager::UnloadPlugins2
		mp_Data = NULL;
	if (mb_Processed) {
		free(mb_Processed); mb_Processed = NULL; mn_ProcessedSize = 0;
	}
	if (mps_FileName) {
		free(mps_FileName); mps_FileName = NULL;
	}
}

// Вернуть true, если p2 должен быть выше p1
bool PVDManager::PluginCompare(ModuleData* p1, ModuleData* p2)
{
	//-- может быть выгружен, так что не ругаться // Объект плагина создается всегда - иначе вниз
	//if (!p1->pPlugin && p2->pPlugin)
	//	return true; // p1 в самый низ как недоступный
	if (!(p1->nCurrentFlags & PVD_IP_SUPPORTEDTYPES) && (p2->nCurrentFlags & PVD_IP_SUPPORTEDTYPES))
		return true; // p1 в самый низ как неизвестный тип
	if (!(p1->nCurrentFlags & PVD_IP_DECODE) && (p2->nCurrentFlags & PVD_IP_DECODE))
		return true; // p2 вверх как декодер
	if ((p1->nCurrentFlags & PVD_IP_DECODE) && !(p2->nCurrentFlags & PVD_IP_DECODE))
		return false; // p1 вверх как декодер
	// низя. в одном плуге может быть несколько типов
	//if ((p1->nCurrentFlags & PVD_IP_SUPPORTEDTYPES) > (p2->nCurrentFlags & PVD_IP_SUPPORTEDTYPES))
	//	return true; // вверх тот плагин (p2), у которого меньше ИД типа
	if (p1->Priority() < p2->Priority())
		return true; // вверх тот плагин (p2), у которого больше приоритет
	else if (p1->Priority() > p2->Priority())
		return false; // вверх тот плагин (p1), у которого больше приоритет
	if (p1->pModuleFileName && p2->pModuleFileName && lstrcmpi(p1->pModuleFileName, p2->pModuleFileName) > 0)
		return true; // иначе сортируем по имени модуля
	return false;
}

void PVDManager::LoadPlugins2()
{
	MCHKHEAP;

	ScanFolder(g_SelfPath);

	MCHKHEAP;

	SortPlugins2();

	#ifdef _DEBUG
	SortPlugins2();
	#endif

	MCHKHEAP;
}

void PVDManager::ScanFolder(const wchar_t* asFrom)
{
	wchar_t *pSearchPath = ConcatPath(asFrom, L"*");
	if (!pSearchPath) {
		_ASSERTE(pSearchPath);
		return;
	}

	WIN32_FIND_DATAW fnd;
	HANDLE hFind = FindFirstFileW(pSearchPath, &fnd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if ((fnd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY) {
				if (fnd.cFileName[0] == L'.' 
					&& (fnd.cFileName[1] == 0 || (fnd.cFileName[1] == L'.' && fnd.cFileName[2] == 0))
					)
					continue;

				wchar_t *pSubFolder = ConcatPath(asFrom, fnd.cFileName);
				if (pSubFolder) {
					ScanFolder(pSubFolder);
					free(pSubFolder);
				} else {
					_ASSERTE(pSubFolder);
				}
				continue;
			}
			
			LoadPlugin(asFrom, fnd, false/*abShowErrors*/);

		} while (FindNextFileW(hFind, &fnd));
		FindClose(hFind);
	}

	free(pSearchPath);
}

ModuleData* PVDManager::FindPlugin(const wchar_t* asFile, bool abShowErrors)
{
	if (!asFile || !*asFile) {
		return NULL;
	}
	
	int nLen = lstrlen(asFile);
	std::vector<ModuleData*>::iterator iter;
	
	// Сначала по полному пути
	if (wcschr(asFile, L'\\')) {
		if (_wcsnicmp(g_SelfPath, asFile, lstrlen(g_SelfPath))) {
			// Начало пути не совпадает с корневым к плагину
			return NULL;
		}
		
		// Сравниваем полный путь к плагину
		for (iter = Plugins.begin(); iter != Plugins.end();iter++) {
			ModuleData* plug = *iter;
			
			if (!lstrcmpi(plug->pModulePath, asFile))
				return plug; // Нашли
		}
	} else {
		// Пытаемся найти просто по имени
		for (iter = Plugins.begin(); iter != Plugins.end();iter++) {
			ModuleData* plug = *iter;
			
			if (!lstrcmpi(plug->pModuleFileName, asFile))
				return plug;
		}
	}
	
	return NULL; // не нашли
}

ModuleData* PVDManager::LoadPlugin(const wchar_t* asFrom, WIN32_FIND_DATAW& fnd, bool abShowErrors)
{
	int nFileNameLen = lstrlen(fnd.cFileName);
	if (nFileNameLen <= 4)
		return NULL;
	if (lstrcmpi(fnd.cFileName+nFileNameLen-4, L".pvd"))
		return NULL; // НЕ '*.pvd' файл

	ModuleData *plug = new ModuleData;
	_ASSERTE(plug!=NULL);
	//memset(plug, 0, sizeof(ModuleData));
	plug->pModulePath = ConcatPath(asFrom, fnd.cFileName);
	plug->pModuleFileName = wcsrchr(plug->pModulePath, L'\\');
	if (plug->pModuleFileName) plug->pModuleFileName++; else plug->pModuleFileName = plug->pModulePath;
	plug->pRegPath = ConcatPath(g_RootKey, fnd.cFileName);
	plug->ftModified = fnd.ftLastWriteTime;

	// Загрузить из реестра версию, проверить дату изменения
	HKEY hkey = NULL; DWORD dwDisp = 0;
	bool lbPluginChanged = true;
	if (RegCreateKeyEx(HKEY_CURRENT_USER, plug->pRegPath, 0, 0, 0, KEY_ALL_ACCESS, 0, &hkey, &dwDisp)) {
		_ASSERT(FALSE);
	} else {
		DWORD nVerSize = sizeof(DWORD), nVersion = 0, nModSize = sizeof(FILETIME), nType,
			nFlags = 0, nFlagSize = sizeof(DWORD), nCurrentFlags = 0, nCurrentFlagSize = sizeof(DWORD);
		FILETIME ftRegModified;
		if (!RegQueryValueEx(hkey, L"Version", 0, &(nType=REG_DWORD), (LPBYTE)&nVersion, &nVerSize)
			&& !RegQueryValueEx(hkey, L"Modified", 0, &(nType=REG_BINARY), (LPBYTE)&ftRegModified, &nModSize)
			&& !RegQueryValueEx(hkey, L"PluginFlags", 0, 0, (LPBYTE)&nFlags, &nFlagSize)
			&& !RegQueryValueEx(hkey, L"PluginCurrentFlags", 0, 0, (LPBYTE)&nCurrentFlags, &nCurrentFlagSize)
			)
		{
			// Если модуль не изменялся с последней загрузки
			if (ftRegModified.dwHighDateTime == plug->ftModified.dwHighDateTime
				&& ftRegModified.dwLowDateTime == plug->ftModified.dwLowDateTime)
			{
				plug->nVersion = nVersion;
				plug->nCurrentFlags = nCurrentFlags;
				plug->nFlags = nFlags;
				plug->bPrepared = (nFlags & PVD_IP_SUPPORTEDTYPES) != 0;
				lbPluginChanged = false;
			}
		}
		RegCloseKey(hkey); hkey = NULL;
	}

	plug->Load(lbPluginChanged);

	plug->hPlugin = NULL; // Выгрузкой библиотеки занимается PVDDecoderBase
	Plugins.push_back(plug);
	
	return plug;
}

bool PVDManager::CreateVersion1(ModuleData* plug, bool lbForceWriteReg)
{
	_ASSERTE(plug->pPlugin == NULL);
	
	plug->nVersion = 1;
	plug->bPrepared = FALSE;

	plug->pPlugin = new PVDDecoderVer1(plug);

	if (!plug->pPlugin) {
		_ASSERTE(plug->pPlugin!=NULL);
	} else {
		plug->nPriority = plug->pPlugin->nDefPriority = 0;
		//_ASSERTE(!plug->pActive && !plug->pInactive && !plug->pForbidden);
		// Расширения могли остаться от предыдущего раза (unload:, reload:)
		SAFEFREE(plug->pActive); SAFEFREE(plug->pInactive); SAFEFREE(plug->pForbidden);

		if (plug->pPlugin->InitPrepare(plug->ftModified, lbForceWriteReg)) {
			_ASSERTE(plug->pPlugin->nFlags == PVD_IP_DECODE);
			_ASSERTE(plug->nCurrentFlags == PVD_IP_DECODE);
			_ASSERTE(plug->pPlugin->nDefPriority > 0);
			// Если настройки модуля ни разу не сохранялись в реестре - в plug->nPriority может быть 0
			if (!plug->nPriority) plug->nPriority = max(1,plug->pPlugin->nDefPriority);
			_ASSERTE(plug->pPlugin->pDefActive || plug->pActive);
			if (!plug->pActive)
				plug->pActive = _wcsdup(plug->pPlugin->pDefActive ? plug->pPlugin->pDefActive : L"");
			if (!plug->pInactive)
				plug->pInactive = _wcsdup(plug->pPlugin->pDefInactive ? plug->pPlugin->pDefInactive : L"");
			if (!plug->pForbidden)
				plug->pForbidden = _wcsdup(plug->pPlugin->pDefForbidden ? plug->pPlugin->pDefForbidden : L"");
			//plug->nFlags = plug->pPlugin->nFlags;
			//plug->iPriority = plug->pPlugin->iPriority;
			plug->bPrepared = TRUE; // OK
		} else {
			delete plug->pPlugin;
			plug->pPlugin = NULL;
		}
	}

	return plug->bPrepared;
}

bool PVDManager::CreateVersion2(ModuleData* plug, bool lbForceWriteReg)
{
	_ASSERTE(plug->pPlugin == NULL);
	
	plug->nVersion = 2;
	plug->bPrepared = FALSE;

	plug->pPlugin = new PVDDecoderVer2(plug);

	if (!plug->pPlugin) {
		_ASSERTE(plug->pPlugin!=NULL);
	} else {
		plug->nPriority = plug->pPlugin->nDefPriority = 0;
		//_ASSERTE(!plug->pActive && !plug->pInactive && !plug->pForbidden);
		// Расширения могли остаться от предыдущего раза (unload:, reload:)
		SAFEFREE(plug->pActive); SAFEFREE(plug->pInactive); SAFEFREE(plug->pForbidden);

		if (plug->pPlugin->InitPrepare(plug->ftModified, lbForceWriteReg)) {
			_ASSERTE(plug->pPlugin->nFlags != 0);
			_ASSERTE(plug->nCurrentFlags != 0);
			_ASSERTE(plug->pPlugin->nDefPriority > 0);
			// Если настройки модуля ни разу не сохранялись в реестре - в plug->nPriority может быть 0
			if (!plug->nPriority) plug->nPriority = max(1,plug->pPlugin->nDefPriority);
			if (plug->nCurrentFlags & PVD_IP_DECODE) {
				_ASSERTE(plug->pPlugin->pDefActive || plug->pActive);
				if (!plug->pActive)
					plug->pActive = _wcsdup(plug->pPlugin->pDefActive ? plug->pPlugin->pDefActive : L"");
				if (!plug->pInactive)
					plug->pInactive = _wcsdup(plug->pPlugin->pDefInactive ? plug->pPlugin->pDefInactive : L"");
				if (!plug->pForbidden)
					plug->pForbidden = _wcsdup(plug->pPlugin->pDefForbidden ? plug->pPlugin->pDefForbidden : L"");
			}
			//plug->nFlags = plug->pPlugin->nFlags;
			//plug->iPriority = plug->pPlugin->iPriority;
			plug->bPrepared = TRUE; // OK
		} else {
			delete plug->pPlugin;
			plug->pPlugin = NULL;
		}
	}

	return plug->bPrepared;
}

bool PVDManager::IsSupportedExtension(const wchar_t *pFileName)
{
	ModuleData* p = NULL;
	UINT nCount = (UINT)Decoders.size();

	const wchar_t *pExt = NULL;
	const wchar_t *pS = wcsrchr(pFileName, '\\');
	if (!pS) pS = pFileName;
	if (((pExt = wcsrchr(pFileName, '.')) != NULL) && (pExt >= pS))
		pExt++;
	else
		pExt = NULL;

	
	for (UINT i = 0; i < nCount; i++)
	{
		p = Decoders[i];
		if (p->pPlugin == NULL) continue; // Если на этом шаге субплагин не был создан - значит декодирование будет невозможно

		// Запрещенные - запрещены всегда
		_ASSERTE(p->pForbidden);
		if (wcschr(p->pForbidden,L'*'))
			continue; // за исключением '*'

		// На шаге 3 если по расширению подобрать плагин не получилось - 
		// пробуем всеми подряд (активными и НЕ активными)

		// Интересует принципиальная возможность открытия этого файла
		// 2009-12-13 было false/*abAllowAsterisk*/
		if (p->pPlugin->IsAllowed(pExt, true/*abAllowAsterisk*/, true/*abAllowInactive*/))
		{
			return true;
		}
	}

	return false;
}

void PVDManager::SortPlugins2()
{
	std::vector<ModuleData*>::iterator i, j, liMin;
    bool liCmp, lbChanged = false;
    ModuleData *tmp, *pFind, *pNewDefaultDisplay;
    for (i=Plugins.begin(); i!=Plugins.end() && (i+1)!=Plugins.end(); i++) {
        liMin = i;
        for (j=i+1; j!=Plugins.end(); j++) {
            liCmp = PluginCompare(*liMin, *j);
            if (liCmp)
                liMin = j;
        }
        if (liMin!=i) {
			lbChanged = true;
			tmp = *liMin;
			*liMin = *i;
			*i = tmp;
        }
    }

	if (lbChanged) {
		UINT n = (int)Plugins.size() * 16;
		
		for (i=Plugins.begin(); i!=Plugins.end(); i++, n-=16) {
			ModuleData* p = *i;
			if (p->Priority() != n) {
				p->SetPriority(n);

				// Приоритет сохраняем только явно со страницы модулей диалога настроек
				//HKEY hkey = NULL; DWORD dwDisp = 0;
				//if (!RegCreateKeyEx(HKEY_CURRENT_USER, p->pRegPath, 0, 0, 0, KEY_ALL_ACCESS, 0, &hkey, &dwDisp)) {
				//	DWORD nDataSize = 0;
				//	RegSetValueEx(hkey, L"Priority", 0, REG_DWORD, (LPBYTE)&n, sizeof(n));
				//	RegCloseKey(hkey); hkey = NULL;
				//}
			}
		}
		_ASSERTE(n>=0);
	}
	
	
    for (i=Decoders.begin(); i!=Decoders.end(); i=Decoders.erase(i)) {
    	tmp = *i;
    	tmp->Release();
    }
    pFind = pDefaultDisplay;
	for (i=Displays.begin(); i!=Displays.end(); i=Displays.erase(i)) {
		tmp = *i;
		if (pFind && pFind == tmp) pFind = NULL;
		tmp->Release();
	}

	pNewDefaultDisplay = NULL;
	BOOL bTerminalSession = GetSystemMetrics(SM_REMOTESESSION); // TRUE - для "Terminal Services client session"
	#ifdef _DEBUG
	//bTerminalSession = TRUE;
	#endif

	WARNING("Выбор допустимого дисплея. В терминальных сессиях недопустимо использование DX");
    for (i=Plugins.begin(); i!=Plugins.end(); i++) {
        tmp = *i;
		bool bKnown = false;
		if ((tmp->nCurrentFlags & PVD_IP_DECODE) == PVD_IP_DECODE) {
			tmp->AddRef();
			Decoders.push_back(tmp);
			bKnown = true;
		} 
		if ((tmp->nCurrentFlags & PVD_IP_DISPLAY) == PVD_IP_DISPLAY
			|| (tmp->pPlugin && (tmp->pPlugin->nFlags & PVD_IP_DISPLAY) == PVD_IP_DISPLAY))
		{
			tmp->AddRef();
			Displays.push_back(tmp);
			bKnown = true;

			if (bTerminalSession) {
				if (tmp->nCurrentFlags & PVD_IP_NOTERMINAL) {
					tmp->nActiveFlags &= ~PVD_IP_DISPLAY;
					tmp->nCurrentFlags &= ~PVD_IP_DISPLAY;
				}
			}
			
			if ((tmp->nActiveFlags & PVD_IP_DISPLAY) && (tmp->nCurrentFlags & PVD_IP_PRIVATE)) {
				// Private Display - нельзя выбирать как умолчательный модуль вывода
				tmp->nActiveFlags &= ~PVD_IP_DISPLAY;
			} else if ((tmp->nActiveFlags & PVD_IP_DISPLAY) && !pNewDefaultDisplay) {
				if (tmp->pPlugin)
					pNewDefaultDisplay = tmp;
				else
					tmp->nActiveFlags &= ~PVD_IP_DISPLAY; // ошибка, модуль не создан, а флаг установлен
			} else if (pNewDefaultDisplay && (tmp->nActiveFlags & PVD_IP_DISPLAY)) {
				tmp->nActiveFlags &= ~PVD_IP_DISPLAY; // наличие нескольких активных флагов недопустимо
			}
		} 

		if (!bKnown) {
			OutputDebugString(L"Unknown module type: ");
			OutputDebugString(tmp->pModulePath);
			OutputDebugString(L"\n");
		}
	}

	if (!pNewDefaultDisplay && Displays.size()>0) {
		for (i=Displays.begin(); i!=Displays.end(); i++) {
			tmp = *i;
			if (!(tmp->nCurrentFlags & PVD_IP_DISPLAY))
				continue; // Недопустим в текущем режиме
			if ((tmp->nCurrentFlags & PVD_IP_DISPLAY) && (tmp->nCurrentFlags & PVD_IP_PRIVATE))
				continue; // Private Display - нельзя выбирать как умолчательный модуль вывода
			if (tmp->nCurrentFlags & PVD_IP_DIRECT) {
				pNewDefaultDisplay = tmp; break; // Предпочтение - быстрому модулю вывода
			}
		}
		if (!pNewDefaultDisplay) {
			for (i=Displays.begin(); i!=Displays.end(); i++) {
				tmp = *i;
				if (!(tmp->nCurrentFlags & PVD_IP_DISPLAY))
					continue; // Недопустим в текущем режиме
				if ((tmp->nCurrentFlags & PVD_IP_DISPLAY) && (tmp->nCurrentFlags & PVD_IP_PRIVATE))
					continue; // Private Display - нельзя выбирать как умолчательный модуль вывода
				pNewDefaultDisplay = tmp; break; // ну а теперь уже первому доступному...
			}
		}


		if (!pNewDefaultDisplay)
			pNewDefaultDisplay = Displays[0];

		pNewDefaultDisplay->nActiveFlags |= PVD_IP_DISPLAY;
	}
	pDefaultDisplay = pNewDefaultDisplay;
	_ASSERTE(pDefaultDisplay!=NULL);
}

// Вызывается при завершении работы плагина (из ExitFARW)
void PVDManager::UnloadPlugins2()
{
	std::vector<ModuleData*>::iterator iter = Plugins.begin();
	while (iter != Plugins.end()) {
		ModuleData* plug = *iter;
		plug->Release();
		iter = Plugins.erase(iter);
	}
	iter = Decoders.begin();
	while (iter != Decoders.end()) {
		ModuleData* plug = *iter;
		plug->Release();
		iter = Decoders.erase(iter);
	}
	iter = Displays.begin();
	while (iter != Displays.end()) {
		ModuleData* plug = *iter;
		plug->Release();
		iter = Displays.erase(iter);
	}
	//
	std::vector<wchar_t*>::iterator iwc = sWholeLog.begin();
	while (iwc != sWholeLog.end()) {
		wchar_t* psz = *iwc;
		free(psz);
		iwc = sWholeLog.erase(iwc);
	}
}

void PVDManager::AddLogString(const wchar_t* asModule, const wchar_t* asMessage)
{
	if (!asMessage) asMessage = L"<NULL>";
	if (!asModule) asModule = L"";
	
	if (g_Plugin.sLogFile[0] && (*asModule || *asMessage)) {
		HANDLE hFile = CreateFile(g_Plugin.sLogFile, GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
		if (hFile != INVALID_HANDLE_VALUE) {
			DWORD nWrite = 0;
			SetFilePointer(hFile, 0,0, FILE_END);
			if (*asModule) {
				WriteFile(hFile, asModule, lstrlen(asModule)*2, &nWrite, 0);
				WriteFile(hFile, L": ", 4, &nWrite, 0);
			}
			if (*asMessage) WriteFile(hFile, asMessage, lstrlen(asMessage)*2, &nWrite, 0);
			WriteFile(hFile, L"\r\n", 4, &nWrite, 0);
			CloseHandle(hFile);
		}
	}
	
	int nCurCount = (int)sWholeLog.size();
	if (nCurCount >= MAX_WHOLELOG_LINES) {
		std::vector<wchar_t*>::iterator iwc = sWholeLog.begin();
		while ((nCurCount >= MAX_WHOLELOG_LINES) 
			&& (iwc != sWholeLog.end()))
		{
			wchar_t* psz = *iwc;
			free(psz);
			iwc = sWholeLog.erase(iwc);
		}
	}
	
	int nLen = lstrlen(asMessage)+1;
	if (nLen > 1024) nLen = 1024;
	#define LOGMODULEWIDTH 12
	wchar_t* pszNew = (wchar_t*)malloc((nLen+LOGMODULEWIDTH+3)*sizeof(wchar_t));
	if (pszNew) {
		lstrcpyn(pszNew, asModule, LOGMODULEWIDTH+1);
		int nCur = lstrlen(pszNew);
		while (nCur < LOGMODULEWIDTH) pszNew[nCur++] = L' ';
		pszNew[LOGMODULEWIDTH] = L'|';
		lstrcpyn(pszNew+LOGMODULEWIDTH+1, asMessage, nLen);
		
		sWholeLog.push_back(pszNew);
		bWholeLogChanged = true;
	}
}

bool PVDManager::OpenWith(ModuleData *pData, const wchar_t *pFileName, i64 lFileSize, const u8 *pBuffer, uint lBuffer, pvdInfoPage2 &InfoPage)
{
	if (!pData->pPlugin) {
		_ASSERTE(pData->pPlugin);
		return false;
	}

	bool result = false;
	pvdInfoImage2 InfoImage = {sizeof(InfoImage)};
	
	MCHKHEAP;

	wchar_t szDbg[MAX_PATH*3], *pNamePart;
	int nCur = 0, nSize = sizeofarray(szDbg);
	lstrcpyn(szDbg+nCur, pData->pPlugin->pName, 128); nCur += lstrlen(szDbg+nCur);
	lstrcpy(szDbg+nCur, L": Opening \""); nCur += lstrlen(szDbg+nCur);
	pNamePart = pFileName ? wcsrchr(pFileName, L'\\') : L"<NULL>";
	pNamePart = pNamePart ? (pNamePart+1) : (pFileName ? pFileName : L"<NULL>");
	lstrcpyn(szDbg+nCur, pNamePart, MAX_PATH+1); nCur += lstrlen(szDbg+nCur);
	lstrcpy(szDbg+nCur, L"\"… ");
	//wnsprintf(szDbg, sizeofarray(szDbg), L"%s: Opening image %s...", pData->pPlugin->pName, pFileName ? pFileName : L"<NULL>");
	OutputDebugString(szDbg);

	//IN_APL такого не пережил
	//if (lFileSize == (int)lBuffer)
	//	lFileSize = 0;
	
	// Установим имя декодера сразу, чтобы показать его в заголовке окна
	lstrcpyn(g_Plugin.ImageX->DecoderName, pData->pPlugin->pName, sizeofarray(g_Plugin.ImageX->DecoderName));
	g_Plugin.ImageX->lWidth = g_Plugin.ImageX->lHeight = g_Plugin.ImageX->nBPP = 0;
	g_Plugin.ImageX->nPages = 0; g_Plugin.ImageX->Animation = false;
	if (mp_Image && mp_Image == g_Plugin.ImageX && g_Plugin.ImageX == g_Plugin.Image[0])
		TitleRepaint(true);
		
	// Сбросим
	pData->pPlugin->szLastError[0] = 0;

	if (mb_ImageOpened || mp_ImageContext) {
		_ASSERTE(!mb_ImageOpened && !mp_ImageContext);
	}

	if (pData->pPlugin->FileOpen2(pFileName, lFileSize, pBuffer, lBuffer, &InfoImage))
	{
		MCHKHEAP;
		
		mb_ImageOpened = true;
		mp_ImageContext = InfoImage.pImageContext;
		g_Plugin.ImageX->nPages = InfoImage.nPages;
		g_Plugin.ImageX->Animation = InfoImage.Flags & PVD_IIF_ANIMATED/* 1 */;
		g_Plugin.ImageX->FileName = pFileName;
		//if (!wcsncmp(pFileName,L"\\\\?\\",4)) pFileName+=4;
		//lstrcpynW(g_Plugin.ImageX->FileNameData, pFileName, sizeofarray(g_Plugin.ImageX->FileNameData));

		memset(&InfoPage, 0, sizeof(InfoPage));
		InfoPage.cbSize = sizeof(InfoPage);
		InfoPage.iPage = 0;
		if (pData->pPlugin->PageInfo2(mp_ImageContext, &InfoPage))
		{
			MCHKHEAP;
			
			g_Plugin.ImageX->lWidth = InfoPage.lWidth;
			g_Plugin.ImageX->lHeight = InfoPage.lHeight;
			g_Plugin.ImageX->nBPP = InfoPage.nBPP;
			if (InfoPage.pFormatName)
				lstrcpyn(g_Plugin.ImageX->FormatName, InfoPage.pFormatName, sizeofarray(g_Plugin.ImageX->FormatName));
			else if (InfoImage.pFormatName)
				lstrcpyn(g_Plugin.ImageX->FormatName, InfoImage.pFormatName, sizeofarray(g_Plugin.ImageX->FormatName));
			else
				*g_Plugin.ImageX->FormatName = 0;
			if (InfoPage.pCompression)
				lstrcpyn(g_Plugin.ImageX->Compression, InfoPage.pCompression, sizeofarray(g_Plugin.ImageX->Compression));
			else if (InfoImage.pCompression)
				lstrcpyn(g_Plugin.ImageX->Compression, InfoImage.pCompression, sizeofarray(g_Plugin.ImageX->Compression));
			else
				*g_Plugin.ImageX->Compression = 0;
			if (InfoImage.pComments)
				lstrcpyn(g_Plugin.ImageX->Comments, InfoImage.pComments, sizeofarray(g_Plugin.ImageX->Comments));
			else
				*g_Plugin.ImageX->Comments = 0;
			result = true;
		}
	}
	
	MCHKHEAP;

	OutputDebugString(result ? L"Succeeded\n" : L"Failed!!!\n");
	if (!result) {
		if (pData->pPlugin->szLastError[0]) {
			int nLen = lstrlen(szDbg);
			if ((nLen+10) < sizeofarray(szDbg)) {
				lstrcpyn(szDbg+nLen, pData->pPlugin->szLastError, sizeofarray(szDbg)-nLen-1);
			}
		} else {
			lstrcat(szDbg, L"Failed!!! ");
		}
		pData->pPlugin->SetStatus(szDbg);
		#ifdef _DEBUG
		if ((GetKeyState(VK_CAPITAL) & 1) == 1) {
			Sleep(1000);
		}
		#endif

		if (mb_ImageOpened) {
			pData->pPlugin->FileClose2(mp_ImageContext);
			mp_ImageContext = NULL;
			mb_ImageOpened = false;
		}
	}
	MCHKHEAP;
	return result;
}

// После успешного открытия и декодирования нужно сборосить флаги декодеров, которыми
// нюхали открываемый файл. Это требуется для того, чтобы при следующем открытии
// другого (или даже этого же) файла была выполнена проверка всеми декодерами.
// При следующей проверке ПЕРВЫМ будет испробовал декодер, успешно отработавший
// в последний раз.
void PVDManager::ResetProcessed()
{
	MCHKHEAP;
	if (mb_Processed) {
		int nCount = (int)Decoders.size();
		_ASSERTE(nCount>0);
		if (mn_ProcessedSize != nCount) {
			free(mb_Processed); mb_Processed = NULL; mn_ProcessedSize = 0;
		} else {
			memset(mb_Processed, 0, nCount*sizeof(bool));
		}
	}
	MCHKHEAP;
}

bool PVDManager::Open(const wchar_t *pFileName, i64 lFileSize, const u8 *pBuffer, uint lBuffer, pvdInfoPage2 &InfoPage)
{
	if (Decoders.empty())
		return false;

	bool result = false;
	int i, s, nFrom, nTo, nCount, c; // строго со знаком!
	
	nCount = (int)Decoders.size();
	_ASSERTE((uint)mi_SubDecoder < (uint)Decoders.size());
	_ASSERTE(pFileName);

	TODO("Чтение файла может быть все-таки выполнять в память только при необходимости?");
	// Чтобы не дергать те файлы, которые точно не проходят по расширениям
    // Да и возможно, что это будет лучше по времени, чем делать FileMapping?

	DWORD nNextDecoder = (g_Plugin.FlagsWork & (FW_PREVDECODER|FW_NEXTDECODER));

	// Refresh уже обработан в Close()
	if (!nNextDecoder /*&& bNewFile*/) { // убрал bNewFile - мешает
		mi_SubDecoder = 0;
	}

	// Чтобы не "нюхать" файл несколько раз одним декодером
	if (!mb_Processed) {
		mn_ProcessedSize = nCount;
		mb_Processed = (bool*)calloc(nCount,sizeof(bool));
	} 
	/* -- а вот это делать нельзя (memset...), иначе мы зациклимся, если модуль вывода вернет ошибку
	else if (!nNextDecoder || !mps_FileName) { // добавил !nNextDecoder
		memset(mb_Processed, 0, nCount*sizeof(bool));
	} else if (lstrcmp(pFileName, mps_FileName)) {
		memset(mb_Processed, 0, nCount*sizeof(bool));
	}
	*/
	// Запомнить имя обрабатываемого файла
	bool bNewFile = (mps_FileName == NULL);
	if (mps_FileName)
		bNewFile = lstrcmp(mps_FileName, pFileName)!=0;
	if (mps_FileName && bNewFile) {
		free(mps_FileName); mps_FileName = NULL;
	}
	if (bNewFile)
		mps_FileName = _wcsdup(pFileName);



	const wchar_t *pExt = GetExtension(pFileName);
	//	wcsrchr(pFileName, L'\\');
	//if (!pExt) pExt = pFileName;
	//if (pExt = wcsrchr(pFileName, '.'))
	//	pExt++;
	//else
	//	pExt = L".";

	

	//for (; iSubDecoder < nPlugins && !result 
	//	&& pPluginsData[iSubDecoder].PluginInfo.Priority >= LowestAllowedPriority; 
	//	iSubDecoder++)
	ModuleData* p = NULL;
	// Ищем в три прохода 
	// это задел на будущее - дать возможность на лету переключить декодер
	// Например AltPgDn / AltPgUp - достаточно iSubDecoder изменить и вызвать повторное определение
	// 1. iSubDecoder .. Count-1
	// 2. 0 .. iSubDecoder-1
	// 3. 0 .. Count-1, но по неактивным расширениям
	// 4. 0 .. Count-1, но и на неактивные расширения внимания уже не обращаем
	for (s = 1; !result && s <= 4; s++)
	{
		if (s >= 4 && !(g_Plugin.FlagsWork & FW_FORCE_DECODE))
			break; // "Усиленный подбор" без учета поддерживаемых расширений ТОЛЬКО на первом файле серии

		if (!nNextDecoder) // обычный режим. по возможности используем текущий декодер
		{
			c = 1;
			switch (s) {
			case 1: nFrom = mi_SubDecoder; nTo = nCount; break;
			case 2: nFrom = 0; nTo = mi_SubDecoder; break;
			case 3: case 4: nFrom = 0; nTo = nCount; break;
			}
		} else if (nNextDecoder & FW_PREVDECODER) {
			c = -1;
			switch (s) {
			case 1: nFrom = mi_SubDecoder-1; nTo = -1; break;
			case 2: nFrom = nCount-1; nTo = mi_SubDecoder; break;
			case 3: case 4: nFrom = nCount-1; nTo = -1; break;
			}
		} else if (nNextDecoder & FW_NEXTDECODER) {
			c = 1;
			switch (s) {
			case 1: nFrom = mi_SubDecoder+1; nTo = nCount; break;
			case 2: nFrom = 0; nTo = mi_SubDecoder+1; break;
			case 3: case 4: nFrom = 0; nTo = nCount; break;
			}
		}
		
		// (i>=0 && i<nCount) - на всякий случай, чтобы с диапазонами не ошибиться
		for (i = nFrom; !result && i != nTo && (i>=0 && i<nCount); i+=c)
		{
			TODO("Не получится сделать ReOpen этим же плагином?");
			if (i < mn_ProcessedSize && mb_Processed[i]) continue;
			p = Decoders[i];
			if (p->pPlugin == NULL) continue;

			//уже не пробуем. Ингорируемые НЕ игнорируем // На шаге 3 если по расширению подобрать плагин не получилось - 
			//// пробуем всеми подряд, причем не взирая на игнорируемые расширения
			//if ((s == 3) && wcschr(p->pPlugin->pIgnored,L'*')) // за исключением '*'
			_ASSERTE(p->pForbidden);
			// Теперь - всегда
			if (wcschr(p->pForbidden, L'*') || ExtensionMatch(p->pForbidden, pExt)) {
				mb_Processed[i] = true; // чтобы повторно не обрабатывать
				continue;
			}

			_ASSERTE(p->pInactive && p->pActive);
			if (p->pPlugin->IsAllowed(pExt, true/*abAllowAsterisk*/, (s >= 3)/*abAllowInactive*/, (s == 4)/*abAssumeActive*/))
			{
				if (i < mn_ProcessedSize)
					mb_Processed[i] = true; // чтобы повторно не обрабатывать
				TRY{
					if (OpenWith(p, pFileName, lFileSize, pBuffer, lBuffer, InfoPage)) {
						result = true;
						mp_Data = p; mi_SubDecoder = i;
					}
				}CATCH{
				}
			}
		}
	}

	return result;
}

TODO("Требуется коррекция после добавления в Plugins не только декодеров");
uint PVDManager::GetNextDecoder(uint iDecoder, bool abForward)
{
	if (Decoders.size() <= 1)
		return 0;
		
	if (abForward) {
		if ((++iDecoder) >= Decoders.size())
			iDecoder = 0;
	} else {
		if (iDecoder>0)
			iDecoder--;
		else
			iDecoder = Decoders.size()-1;
	}
	
	return iDecoder;
}

void PVDManager::Close(void)
{
	TODO("Переделать. Нужно контекст и плагин передавать аргументом в PVDManager::Close(void)");
	if (mb_ImageOpened) {
		_ASSERTE(mp_Data && mp_Data->pPlugin);
		if (mp_Data) {
			mp_Data->pPlugin->FileClose2(mp_ImageContext);
		}
		mp_ImageContext = NULL; // чтобы не повадно было
		mb_ImageOpened = false;
	}
	//ResetProcessed(); -- низя. иначе циклится при ошибках вывода например
	//iSubDecoder = 0; // а вот это нужно. По F5 нужно повторно запустить определение декодера
}

BOOL __stdcall PVDManager::DecodeCallback2(void *pDecodeCallbackContext2, UINT32 iStep, UINT32 nSteps,
									  pvdInfoDecodeStep2* pImagePart)
{
	// Возможность отмены декодирования
	if (bCancelDecoding || (g_Plugin.FlagsWork & FW_TERMINATE))
		return FALSE;
	return TRUE;
}

bool PVDManager::Decode(ImageInfo *Image, LPVOID* ppDrawContext, bool abResetTick, pvdInfoPage2 &InfoPage)
{
	_ASSERTE(mp_Data && mp_Data->pPlugin);
	bool result = false;

	if (!PVDManager::pDefaultDisplay || !PVDManager::pDefaultDisplay->pPlugin) {
		OutputDebugString(L"PVDManager::pDefaultDisplay->pPlugin was not created!");
		return false;
	}
	
	
	MCHKHEAP


	if (abResetTick) // При декодировании следующей страницы сбросить тик (там сейчас время открытия)
		Image->lStartOpenTime = timeGetTime();
	DWORD t0 = Image->lStartOpenTime;
	DWORD t1 = timeGetTime(), t2=t1, t3=t1, t4=t1, t5=t1, t6=t1;

	wchar_t szLastError[128]; szLastError[0] = 0;
	wchar_t szDbg[MAX_PATH*3], *pNamePart;
	int nCur = 0, nSize = sizeofarray(szDbg);
	lstrcpyn(szDbg+nCur, mp_Data->pPlugin->pName, 128); nCur += lstrlen(szDbg+nCur);
	wsprintf(szDbg+nCur, L": Decoding page %i of image \"", Image->nPage); nCur += lstrlen(szDbg+nCur);
	const wchar_t* pFileName = (const wchar_t*)Image->FileName;
	pNamePart = pFileName ? wcsrchr(pFileName, L'\\') : L"<NULL>";
	pNamePart = pNamePart ? (pNamePart+1) : (pFileName ? pFileName : L"<NULL>");
	lstrcpyn(szDbg+nCur, pNamePart, MAX_PATH); nCur += lstrlen(szDbg+nCur);
	lstrcpy(szDbg+nCur, L"\"… ");
	//wnsprintf(szDbg, sizeofarray(szDbg), L"%s: Decoding page %i of image %s...",
	//	pData->pPlugin->pName, Image->nPage, (const wchar_t*)Image->FileName);
	OutputDebugString(szDbg);

	// Сбросим
	mp_Data->pPlugin->szLastError[0] = 0;
	
	//pvdInfoPage2 InfoPage = {sizeof(pvdInfoPage2)};
	//TRY{
	
	bool bPageInfoOk = false;
	if (InfoPage.cbSize && InfoPage.iPage == Image->nPage && InfoPage.lWidth && InfoPage.lHeight) {
		bPageInfoOk = true;
	} else {
		InfoPage.cbSize = sizeof(InfoPage);
		InfoPage.iPage = Image->nPage;
		bPageInfoOk = mp_Data->pPlugin->PageInfo2(mp_ImageContext, &InfoPage);
		if (bPageInfoOk) {
			if (InfoPage.pFormatName)
				lstrcpyn(Image->FormatName, InfoPage.pFormatName, sizeofarray(Image->FormatName));
			if (InfoPage.pCompression)
				lstrcpyn(Image->Compression, InfoPage.pCompression, sizeofarray(Image->FormatName));
		}
		if (bPageInfoOk && InfoPage.nPages)
			Image->CheckPages(InfoPage.nPages);
	}

	if (bPageInfoOk)
	{
		t2 = timeGetTime();
		pvdInfoDecode2 DecodeInfo = {sizeof(pvdInfoDecode2)};
		bool bNeedFree = true;
		TODO("Попробовать создавать 24 битные поверхности для ускорения их инициализации. это нужно делать после !PageDecode2!");
		TODO("Поменять местами PageDecode2 и CreateWorkSurface. В некоторых случаях мы вообще не сможем наполнить поверхность декодированным colormodel");
		WARNING("24 битные поверхности нифига не блитятся в Primary surface");

		{
			t3 = timeGetTime();
			DecodeInfo.iPage = Image->nPage;

			DecodeInfo.Flags = (mp_Data->pPlugin == PVDManager::pDefaultDisplay->pPlugin) ? PVD_IDF_ASDISPLAY : 0;
			DecodeInfo.nBackgroundColor = g_Plugin.BackColor();

			MCHKHEAP

			SIZE scaledSize = {0,0};
			if (
			    (mp_Data->pPlugin->nFlags & (PVD_IP_CANDESCALE|PVD_IP_CANUPSCALE)) &&
				(g_Plugin.ZoomAuto || g_Plugin.AbsoluteZoom || g_Plugin.FlagsWork & FW_QUICK_VIEW)
			   )
			{
				//TODO("Нужно доработать условия");

				SIZE imgSize = {InfoPage.lWidth,InfoPage.lHeight};
				SIZE renderSize = Image->GetDefaultScaleSize();

				//if (g_Plugin.FlagsWork & FW_QUICK_VIEW)
				if (renderSize.cx && renderSize.cy && imgSize.cx && imgSize.cy) {
					BOOL lbCanScale = FALSE;

					if (renderSize.cx < imgSize.cx && renderSize.cy < imgSize.cy) {
						lbCanScale = (mp_Data->pPlugin->nFlags & PVD_IP_CANDESCALE);
					} else if (renderSize.cx > imgSize.cx && renderSize.cy > imgSize.cy) {
						lbCanScale = (mp_Data->pPlugin->nFlags & PVD_IP_CANUPSCALE);
					}

					if (lbCanScale)
					{
						double dr = (double)renderSize.cx / (double)renderSize.cy;
						double di = (double)imgSize.cx / (double)imgSize.cy;
						if (g_Plugin.ZoomAuto == ZA_FIT) {
							if (di > dr) {
								// Вписать по высоте
								scaledSize.cx = (LONG)(renderSize.cy * di);
								scaledSize.cy = renderSize.cy;
							} else {
								// Вписать по ширине
								scaledSize.cx = renderSize.cx;
								scaledSize.cy = (LONG)(renderSize.cx / di);
							}
						} else if (g_Plugin.ZoomAuto == ZA_FILL) {
							if (di < dr) {
								// Вписать по высоте
								scaledSize.cx = (LONG)(renderSize.cy * di);
								scaledSize.cy = renderSize.cy;
							} else {
								// Вписать по ширине
								scaledSize.cx = renderSize.cx;
								scaledSize.cy = (LONG)(renderSize.cx / di);
							}
						} else if (di > dr) {
							// Вписать по высоте
							scaledSize.cx = (LONG)(renderSize.cy * di);
							scaledSize.cy = renderSize.cy;
						} else {
							// Вписать по ширине
							scaledSize.cx = renderSize.cx;
							scaledSize.cy = (LONG)(renderSize.cx / di);
						}
						DecodeInfo.lWidth = scaledSize.cx;
						DecodeInfo.lHeight = scaledSize.cy;
					}
				}
			}

			MCHKHEAP

			//_ASSERTE(g_Plugin.hWnd);
			if (!g_Plugin.hWnd || !IsWindow(g_Plugin.hWnd)) {
				lstrcpy(szLastError, L"PicView window was closed");
				result = false;
			} else
			if (mp_Data->pPlugin->PageDecode2(mp_ImageContext, &DecodeInfo, DecodeCallback2, this))
			{
				MCHKHEAP

				if (DecodeInfo.Flags & PVD_IDF_ALPHA) {
					_ASSERTE(DecodeInfo.ColorModel == PVD_CM_BGRA);
				}
				
				// Обновить информацию о размере изображения!
				if (DecodeInfo.lSrcWidth && DecodeInfo.lSrcHeight) {
					InfoPage.lWidth = DecodeInfo.lSrcWidth;
					InfoPage.lHeight = DecodeInfo.lSrcHeight;
				}
				Image->lDecodedWidth = DecodeInfo.lWidth;
				Image->lDecodedHeight = DecodeInfo.lHeight;
				Image->nDecodedBPP = DecodeInfo.nBPP;
				
				//InfoPage.nBPP = DecodeInfo.nBPP; -- так не надо. Изображение может быть и 8-битным, а данные декодированы в 32 бита
				//
				if (DecodeInfo.pFormatName)
					lstrcpyn(Image->FormatName, DecodeInfo.pFormatName, sizeofarray(Image->FormatName));
				if (DecodeInfo.pCompression)
					lstrcpyn(Image->Compression, DecodeInfo.pCompression, sizeofarray(Image->Compression));
				if (!Image->FormatName[0]) {
					const wchar_t* pszExt = wcsrchr(pNamePart, L'.');
					if (pszExt) {
						Image->FormatName[0] = L'[';
						lstrcpyn(Image->FormatName+1, pszExt+1, sizeofarray(Image->FormatName)-3);
						lstrcat(Image->FormatName, L"]");
					}
				}
				
				MCHKHEAP

				t4 = timeGetTime();
				if (DecodeInfo.nPages)
					Image->CheckPages(DecodeInfo.nPages);
				//DWORD nFlags = DecodeInfo.Flags;
				//if (nFlags < 2) {
				//	//nFlags |= (DecodeInfo.nBPP == 32) ? PVD_IDF_BGRA : PVD_IDF_BGR;
				//}
				TODO("Флаги из 2-й версии интерфейса");

				{
					WARNING("На анимированных и многостраничных документах тут НЕ NULL");
					//_ASSERTE(*ppDrawContext == NULL);
					if (*ppDrawContext) {
						// А тут именно так. На будущее. В ImageInfo может быть несколько DrawContext, чтобы не закрыть нужный...
						_ASSERTE(Image->Display->pPlugin);
						Image->Display->pPlugin->DisplayClose2(*ppDrawContext);
						*ppDrawContext = NULL;
					}
					//result = dds->Blit(&DecodeInfo, g_Plugin.BackColor);
					pvdInfoDisplayCreate2 DsCreate = {sizeof(pvdInfoDisplayCreate2)};
					#ifdef _DEBUG
						if (DecodeInfo.ColorModel == PVD_CM_UNKNOWN) {
							if (lstrcmpi(mp_Data->pModuleFileName, L"GDIPlus.pvd")) {
								_ASSERTE(DecodeInfo.ColorModel != PVD_CM_UNKNOWN);
							}
						}
					#endif
					// Некоторые декодеры НЕ возвращают палитру для индексированных форматов.
					// Нужно подставить палитру по умолчанию, а чтобы декодер не разрушил НАШУ
					// память - и делаем копию DecodeInfo
					pvdInfoDecode2 DecodeInfoCopy = DecodeInfo;
					if (DecodeInfoCopy.nBPP<=8 && DecodeInfoCopy.ColorModel!=PVD_CM_PRIVATE) {
						if (!DecodeInfoCopy.pPalette) {
							DecodeInfoCopy.pPalette = g_Plugin.GetPalette(DecodeInfoCopy.nBPP, DecodeInfoCopy.ColorModel);
						}
					}
					DsCreate.pImage = &DecodeInfoCopy;
					DsCreate.BackColor = g_Plugin.BackColor();
					DsCreate.pDisplayContext = NULL;
					DsCreate.pFileName = wcsrchr(pFileName, L'\\');
					if (DsCreate.pFileName) DsCreate.pFileName++; else DsCreate.pFileName = pFileName;
					//DsCreate.nChessMateColor1 = 0xFFFFFF;
					//DsCreate.nChessMateColor2 = 0xCCCCCC;
					//DsCreate.nChessMateSize = 8;
					
					WARNING("Проверять флаг PVD_IDF_PRIVATE_DISPLAY. Если установлен - вывод возможен только через модуль декодера");
					
					if ((DecodeInfo.Flags & PVD_IDF_PRIVATE_DISPLAY) && (mp_Data->pPlugin->nCurrentFlags & PVD_IP_DISPLAY)) {
						Image->Display = mp_Data;
					} else {
						Image->Display = PVDManager::pDefaultDisplay;
					}
					ModuleData* pStartDisplay = Image->Display;
					std::vector<ModuleData*>::iterator iDisp = PVDManager::Displays.begin();
					BOOL lbFirstCheckDisplay = TRUE;

					// Если g_Plugin.hWnd == 0 - значит юзер уже нажал Esc и все закрывается
					while (g_Plugin.hWnd && iDisp != PVDManager::Displays.end())
					{
						if (!Image->Display->pPlugin) {
							_ASSERTE(Image->Display->pPlugin);
							result = FALSE;
						} else {
							if (result = Image->Display->pPlugin->DisplayCheck()) {
								MCHKHEAP
								if (!g_Plugin.hWnd) {
									result = false;
									break;
								}
								if (result = Image->Display->pPlugin->DisplayCreate2(&DsCreate))
									break;
								wchar_t szMsg[128];
								lstrcpy(szMsg, L"Display module skipped: ");
								lstrcpy(szMsg+lstrlen(szMsg), Image->Display->pPlugin->pName);
								PVDManager::AddLogString(L"PVDManager", szMsg);
							}
						}

						if ((DecodeInfo.Flags & PVD_IDF_PRIVATE_DISPLAY) || PVDManager::Displays.size()<=1)
							break;

						if (!lbFirstCheckDisplay)
							iDisp++;
						else
							lbFirstCheckDisplay = FALSE;
						// Найти следующий готовый дисплей
						while (iDisp != PVDManager::Displays.end()
							&& ((pStartDisplay == *iDisp) 
							    || ((*iDisp)->pPlugin == NULL)
							    || ((*iDisp)->nCurrentFlags & PVD_IP_PRIVATE)))
							iDisp++;
						if (iDisp == PVDManager::Displays.end()) break;

						// Запомнить новый дисплей
						Image->Display = *iDisp;
						wchar_t szMsg[128];
						lstrcpy(szMsg, L"Switching to display module: ");
						lstrcpy(szMsg+lstrlen(szMsg), Image->Display->pPlugin->pName);
						PVDManager::AddLogString(L"PVDManager", szMsg);
					}

					if (result)
						*ppDrawContext = DsCreate.pDisplayContext;

						//DecodeInfo.pImage, DecodeInfo.lImagePitch, DecodeInfo.nBPP, g_Plugin.BackColor, 
						//nFlags, DecodeInfo.nTransparentColor, DecodeInfo.pPalette);
				}
				t5 = timeGetTime();
				bNeedFree = false;
				MCHKHEAP
				mp_Data->pPlugin->PageFree2(mp_ImageContext, &DecodeInfo);
			}
		}
		if (!g_Plugin.hWnd || !IsWindow(g_Plugin.hWnd)) {
			lstrcpy(szLastError, L"PicView window was closed");
			result = false;
		}
		MCHKHEAP
		if (result)
		{
			Image->lWidth = InfoPage.lWidth;
			Image->lHeight = InfoPage.lHeight;
			Image->nBPP = InfoPage.nBPP;
			if (Image->Animation && !InfoPage.lFrameTime)
				InfoPage.lFrameTime = 100; // минимальное умолчательное время фрейма
			Image->lFrameTime = InfoPage.lFrameTime;
		}
	}
	t6 = timeGetTime();
	Image->lTimeOpen = t2-t0; //Открытие файла изображения и получение информации о нем (Open & PageInfo)
		_ASSERTE(Image->lTimeOpen>=0); if (Image->lTimeOpen<0) Image->lTimeOpen = 0;
	Image->lTimeDecode = (t4==t1) ? 0 : (t4-t3); // PageDecode
		_ASSERTE(Image->lTimeDecode>=0); if (Image->lTimeDecode<0) Image->lTimeDecode = 0;
	Image->lTimeTransfer = ((t5==t1) ? 0 : max(0,t5-t4)) + ((t3==t1) ? 0 : max(0,t3-t2)); // CreateWorkSurface+Blit
		_ASSERTE(Image->lTimeTransfer>=0); if (Image->lTimeTransfer<0) Image->lTimeTransfer = 0;
	Image->lTimePaint = 0;
	Image->bTimePaint = FALSE;
	//wsprintf(Image->OpenTimes, L"%i+%i+%i", 
	//	t2-t0, //Открытие файла изображения и получение информации о нем (Open & PageInfo)
	//	(t4==t1) ? 0 : t4-t3, // PageDecode
	//	((t5==t1) ? 0 : t5-t4) + ((t3==t1) ? 0 : t3-t2) // CreateWorkSurface+Blit
	//	);
	Image->lOpenTime = t6-Image->lStartOpenTime;
	//}CATCH{}

	MCHKHEAP
	OutputDebugString(result ? L"Succeeded\n" : L"Failed!!!\n");
	if (!result) {
		LPCWSTR pszError = NULL;
		if (szLastError[0]) {
			pszError = szLastError;
		} else if (mp_Data->pPlugin->szLastError[0]) {
			pszError = mp_Data->pPlugin->szLastError;
		} else {
			pszError = L"Failed!!! ";
		}
		if (pszError) {
			int nLen = lstrlen(szDbg);
			if ((nLen+10) < sizeofarray(szDbg)) {
				lstrcpyn(szDbg+nLen, pszError, sizeofarray(szDbg)-nLen-1);
			}
		}
		mp_Data->pPlugin->SetStatus(szDbg);
	}

	return result;
}

const wchar_t* PVDManager::GetName()
{
	if (mp_Data) {
		if (mp_Data->pPlugin)
			if (mp_Data->pPlugin->pName && *mp_Data->pPlugin->pName)
				return mp_Data->pPlugin->pName;
		if (mp_Data->pModuleFileName)
			return mp_Data->pModuleFileName;
	}

	return L"Unknown";
}


bool PVDManager::DisplayAttach()
{
	if (!PVDManager::pActiveDisplay)
		return FALSE;
	if (!PVDManager::pActiveDisplay->pPlugin)
		return FALSE;

	return PVDManager::pActiveDisplay->pPlugin->DisplayAttach();
}

bool PVDManager::DisplayDetach()
{
	if (!PVDManager::pActiveDisplay)
		return FALSE;
	if (!PVDManager::pActiveDisplay->pPlugin)
		return FALSE;

	return PVDManager::pActiveDisplay->pPlugin->DisplayDetach();
}

void PVDManager::DisplayExit()
{
	std::vector<ModuleData*>::iterator i;
	ModuleData *tmp;

	
	for (uint i = 3; i--;)
		g_Plugin.Image[i]->DisplayClose();

	for (i=Displays.begin(); i!=Displays.end(); i++) {
		tmp = *i;
		if (!tmp->pPlugin) continue;
		if (tmp->pPlugin->bDisplayInitialized) {
			tmp->pPlugin->DisplayExit2();
		}
	}
}
