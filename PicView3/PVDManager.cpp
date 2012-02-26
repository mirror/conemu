
/**************************************************************************
Copyright (c) 2009 Skakov Pavel
Copyright (c) 2010 Maimus5
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

/*
Должен уметь обрабатывать 

Вызов декодирования элемента (он уже может быть декодирован, preload, тогда сразу вернуть)
FD_JUMP, FD_JUMP_NEXT, FD_HOME_END (переделать на более удобные мнемонически)

Повторное декодирование.
FD_REFRESH
Возможны варианты
1. смена декодера
2. повышение качества изображения после зума

*/



#include "PVDManager.h"
#include "Image.h"
#include "ImageInfo.h"
#include "DecoderHandle.h"
#include "PictureView_Lang.h"
#include "DecodeItem.h"
#include "DecoderHandle.h"
#include "DisplayHandle.h"
#include "PVDModuleBase.h"
#include "FileMap.h"


CPVDManager g_Manager;
LPCSTR szPVDManager = "CPVDManager";

std::vector<CModuleInfo*> CPVDManager::Plugins;
std::vector<CModuleInfo*> CPVDManager::Decoders;
std::vector<CModuleInfo*> CPVDManager::Displays;
bool CPVDManager::bCancelDecoding = false;
CModuleInfo* CPVDManager::pDefaultDisplay = NULL; // Что выбрано в настройке плагина - поле "Display module"
CModuleInfo* CPVDManager::pActiveDisplay = NULL;  // А это то, через что идет вывод СЕЙЧАС (может быть NULL)
std::vector<wchar_t*> CPVDManager::sWholeLog;
bool CPVDManager::bWholeLogChanged = false;

CPVDManager::CPVDManager()
{
#ifdef MHEAP_DEFINED
	xf_initialize();
#endif
	memset(&m_DecoderQueue, 0, sizeof(m_DecoderQueue));
	InitializeCriticalSection(&csDecoderQueue);
	mh_Thread = NULL; mn_ThreadId = 0; mn_AliveTick = 0;
	mh_Request = mh_Alive = NULL;
}

CPVDManager::~CPVDManager()
{
	DeleteCriticalSection(&csDecoderQueue);
}

// Вернуть true, если p2 должен быть выше p1
bool CPVDManager::PluginCompare(CModuleInfo* p1, CModuleInfo* p2)
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

void CPVDManager::LoadPlugins2()
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

void CPVDManager::ScanFolder(const wchar_t* asFrom)
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

CModuleInfo* CPVDManager::FindPlugin(const wchar_t* asFile, bool abShowErrors)
{
	if (!asFile || !*asFile) {
		return NULL;
	}
	
	int nLen = lstrlen(asFile);
	std::vector<CModuleInfo*>::iterator iter;
	
	// Сначала по полному пути
	if (wcschr(asFile, L'\\')) {
		if (_wcsnicmp(g_SelfPath, asFile, lstrlen(g_SelfPath))) {
			// Начало пути не совпадает с корневым к плагину
			return NULL;
		}
		
		// Сравниваем полный путь к плагину
		for (iter = Plugins.begin(); iter != Plugins.end();iter++) {
			CModuleInfo* plug = *iter;
			
			if (!lstrcmpi(plug->pModulePath, asFile))
				return plug; // Нашли
		}
	} else {
		// Пытаемся найти просто по имени
		for (iter = Plugins.begin(); iter != Plugins.end();iter++) {
			CModuleInfo* plug = *iter;
			
			if (!lstrcmpi(plug->pModuleFileName, asFile))
				return plug;
		}
	}
	
	return NULL; // не нашли
}

CModuleInfo* CPVDManager::LoadPlugin(const wchar_t* asFrom, WIN32_FIND_DATAW& fnd, bool abShowErrors)
{
	int nFileNameLen = lstrlen(fnd.cFileName);
	if (nFileNameLen <= 4)
		return NULL;
	if (lstrcmpi(fnd.cFileName+nFileNameLen-4, L".pvd"))
		return NULL; // НЕ '*.pvd' файл

	CModuleInfo *plug = new CModuleInfo(szPVDManager);
	_ASSERTE(plug!=NULL);
	//memset(plug, 0, sizeof(CModuleInfo));
	plug->pModulePath = ConcatPath(asFrom, fnd.cFileName);
	plug->pModuleFileName = wcsrchr(plug->pModulePath, L'\\');
	if (plug->pModuleFileName) plug->pModuleFileName++; else plug->pModuleFileName = plug->pModulePath;
	plug->pRegPath = ConcatPath(g_RootKey, fnd.cFileName);
	plug->ftModified = fnd.ftLastWriteTime;

	// Загрузить из реестра версию, проверить дату изменения
	HKEY hkey = NULL; DWORD dwDisp = 0;
	bool lbPluginChanged = true;
	if (RegCreateKeyEx(HKEY_CURRENT_USER, plug->pRegPath, 0, 0, 0, KEY_ALL_ACCESS, 0, &hkey, &dwDisp)) {
		_ASSERTE(FALSE);
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

	plug->hPlugin = NULL; // Выгрузкой библиотеки занимается CPVDModuleBase
	Plugins.push_back(plug);
	
	return plug;
}

bool CPVDManager::CreateVersion1(CModuleInfo* plug, bool lbForceWriteReg)
{
	_ASSERTE(plug->pPlugin == NULL);
	
	plug->nVersion = 1;
	plug->bPrepared = FALSE;

	plug->pPlugin = new CPVDModuleVer1(plug);

	if (!plug->pPlugin) {
		_ASSERTE(plug->pPlugin!=NULL);
	} else {
		plug->nPriority = plug->pPlugin->nDefPriority = 0;
		//_ASSERTE(!plug->pActive && !plug->pInactive && !plug->pForbidden);
		// Расширения могли остаться от предыдущего раза (unload:, reload:)
		SafeFree(plug->pActive); SafeFree(plug->pInactive); SafeFree(plug->pForbidden);

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

bool CPVDManager::CreateVersion2(CModuleInfo* plug, bool lbForceWriteReg)
{
	_ASSERTE(plug->pPlugin == NULL);
	
	plug->nVersion = 2;
	plug->bPrepared = FALSE;

	plug->pPlugin = new CPVDModuleVer2(plug);

	if (!plug->pPlugin) {
		_ASSERTE(plug->pPlugin!=NULL);
	} else {
		plug->nPriority = plug->pPlugin->nDefPriority = 0;
		//_ASSERTE(!plug->pActive && !plug->pInactive && !plug->pForbidden);
		// Расширения могли остаться от предыдущего раза (unload:, reload:)
		SafeFree(plug->pActive); SafeFree(plug->pInactive); SafeFree(plug->pForbidden);

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

bool CPVDManager::IsSupportedExtension(const wchar_t *pFileName)
{
	WARNING("Добавить обработку папок! Их тоже можно 'смотреть' через Shell например");

	CModuleInfo* p = NULL;
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

void CPVDManager::SortPlugins2()
{
	std::vector<CModuleInfo*>::iterator i, j, liMin;
    bool liCmp, lbChanged = false;
    CModuleInfo *tmp, *pFind, *pNewDefaultDisplay;
    for (i=Plugins.begin(); i!=Plugins.end() && (i+1)!=Plugins.end(); i++)
	{
        liMin = i;
        for (j=i+1; j!=Plugins.end(); j++)
		{
            liCmp = PluginCompare(*liMin, *j);
            if (liCmp)
                liMin = j;
        }
        if (liMin!=i)
		{
			lbChanged = true;
			tmp = *liMin;
			*liMin = *i;
			*i = tmp;
        }
    }

	if (lbChanged)
	{
		UINT n = (int)Plugins.size() * 16;
		
		for (i=Plugins.begin(); i!=Plugins.end(); i++, n-=16)
		{
			CModuleInfo* p = *i;
			if (p->Priority() != n)
			{
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
	
	
    for (i=Decoders.begin(); i!=Decoders.end(); i=Decoders.erase(i))
	{
    	tmp = *i;
    	tmp->Release(szPVDManager);
    }
    pFind = pDefaultDisplay;
	for (i=Displays.begin(); i!=Displays.end(); i=Displays.erase(i))
	{
		tmp = *i;
		if (pFind && pFind == tmp) pFind = NULL;
		tmp->Release(szPVDManager);
	}

	pNewDefaultDisplay = NULL;
	BOOL bTerminalSession = GetSystemMetrics(SM_REMOTESESSION); // TRUE - для "Terminal Services client session"
	#ifdef _DEBUG
	//bTerminalSession = TRUE;
	#endif

	WARNING("Выбор допустимого дисплея. В терминальных сессиях недопустимо использование DX");
    for (i=Plugins.begin(); i!=Plugins.end(); i++)
	{
        tmp = *i;
		bool bKnown = false;
		if ((tmp->nCurrentFlags & PVD_IP_DECODE) == PVD_IP_DECODE)
		{
			tmp->AddRef(szPVDManager);
			Decoders.push_back(tmp);
			bKnown = true;
		} 
		if ((tmp->nCurrentFlags & PVD_IP_DISPLAY) == PVD_IP_DISPLAY
			|| (tmp->pPlugin && (tmp->pPlugin->nFlags & PVD_IP_DISPLAY) == PVD_IP_DISPLAY))
		{
			tmp->AddRef(szPVDManager);
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
void CPVDManager::UnloadPlugins2()
{
	std::vector<CModuleInfo*>::iterator iter = Plugins.begin();
	while (iter != Plugins.end())
	{
		CModuleInfo* plug = *iter;
		plug->Release(szPVDManager);
		iter = Plugins.erase(iter);
	}
	iter = Decoders.begin();
	while (iter != Decoders.end())
	{
		CModuleInfo* plug = *iter;
		plug->Release(szPVDManager);
		iter = Decoders.erase(iter);
	}
	iter = Displays.begin();
	while (iter != Displays.end())
	{
		CModuleInfo* plug = *iter;
		plug->Release(szPVDManager);
		iter = Displays.erase(iter);
	}
	//
	std::vector<wchar_t*>::iterator iwc = sWholeLog.begin();
	while (iwc != sWholeLog.end())
	{
		wchar_t* psz = *iwc;
		free(psz);
		iwc = sWholeLog.erase(iwc);
	}
}

void CPVDManager::AddLogString(const wchar_t* asModule, const wchar_t* asMessage)
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

	MCHKHEAP;
	
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

	MCHKHEAP;
}

//// После успешного открытия и декодирования нужно сборосить флаги декодеров, которыми
//// нюхали открываемый файл. Это требуется для того, чтобы при следующем открытии
//// другого (или даже этого же) файла была выполнена проверка всеми декодерами.
//// При следующей проверке ПЕРВЫМ будет испробовал декодер, успешно отработавший
//// в последний раз.
//void CPVDManager::ResetProcessed(C_Image* apImage)
//{
//	MCHKHEAP;
//	if (apImage->mpb_Processed) {
//		int nCount = (int)Decoders.size();
//		_ASSERTE(nCount>0);
//		if (apImage->mn_ProcessedSize != nCount) {
//			free(apImage->mpb_Processed); apImage->mpb_Processed = NULL; apImage->mn_ProcessedSize = 0;
//		} else {
//			memset(apImage->mpb_Processed, 0, nCount*sizeof(bool));
//		}
//	}
//	MCHKHEAP;
//}

bool CPVDManager::OpenWith(
		CImage* apImage,
		CDecoderHandle** ppFile,
		CModuleInfo *pDecoder,
		const wchar_t *pFileName, i64 lFileSize,
		const u8 *pBuffer, uint lBuffer)
		// pvdInfoPage2 &InfoPage теперь в CImage
{
	if (!pDecoder->pPlugin) {
		_ASSERTE(pDecoder->pPlugin);
		return false;
	}
	
	TODO("Разрулить ситуацию, когда существущий дескриптор одного декодера используется для отрисовки");
	// а приходит запрос на декодирование другим декодером. Тогда нужно создать новый apImage->pTempInfo,
	// после окончания декодирования в нити дисплея нужно сменить активный на только что декодированный
	// учесть, что освобождение дескриптора декодера нужно выполнять в нити ДЕКОДЕРА
	ImageInfo* pInfo = &apImage->Info;

	if (!*ppFile)
	{
		*ppFile = new CDecoderHandle(szPVDManager, apImage);
	}
	else
	{
		_ASSERTE((*ppFile)->Context() == NULL);
		(*ppFile)->Close();
	}

	bool result = false;
	LPVOID pFileContext = NULL;
	pvdInfoImage2 InfoImage = {sizeof(InfoImage)};
	pvdInfoPage2  InfoPage = {sizeof(pvdInfoPage2)};
	
	MCHKHEAP;

	wchar_t szDbg[MAX_PATH*3], *pNamePart;
	int nCur = 0, nSize = sizeofarray(szDbg);
	lstrcpyn(szDbg+nCur, pDecoder->pPlugin->pName, 128); nCur += lstrlen(szDbg+nCur);
	lstrcpy(szDbg+nCur, L": Opening \""); nCur += lstrlen(szDbg+nCur);
	pNamePart = pFileName ? wcsrchr(pFileName, L'\\') : L"<NULL>";
	pNamePart = pNamePart ? (pNamePart+1) : (pFileName ? pFileName : L"<NULL>");
	lstrcpyn(szDbg+nCur, pNamePart, MAX_PATH+1); nCur += lstrlen(szDbg+nCur);
	lstrcpy(szDbg+nCur, L"\"… ");
	//wnsprintf(szDbg, sizeofarray(szDbg), L"%s: Opening image %s...", pDecoder->pPlugin->pName, pFileName ? pFileName : L"<NULL>");
	OutputDebugString(szDbg);

	//IN_APL такого не пережил
	//if (lFileSize == (int)lBuffer)
	//	lFileSize = 0;
	
	// Установим имя декодера сразу, чтобы показать его в заголовке окна
	//lstrcpyn(/*g_Plugin.ImageX*/ apImage->DecoderName, pDecoder->pPlugin->pName, sizeofarray(/*g_Plugin.ImageX*/ apImage->DecoderName));
	///*g_Plugin.ImageX*/ apImage->lWidth = /*g_Plugin.ImageX*/ apImage->lHeight = /*g_Plugin.ImageX*/ apImage->nBPP = 0;
	///*g_Plugin.ImageX*/ apImage->nPages = 0; /*g_Plugin.ImageX*/ apImage->Animation = false;
	pInfo->InitDecoder(pDecoder);
	
	//if (mp_Image && mp_Image == /*g_Plugin.ImageX*/ apImage && /*g_Plugin.ImageX*/ apImage == g_Plugin.Image[0])
	if (apImage->IsActive())
	{
		//TitleRepaint(true);
		//ExecuteInMainThread(FW_TITLEREPAINTD,0);
		WARNING("Иногда нити пересекаются, и в заголовке отрисовывается старый файл, а не тот, который уже на экране!");
		g_Plugin.FlagsWork |= FW_TITLEREPAINTD;
	} else {
		TODO("OSD для декодируемого изображения");
	}
		
	// Сбросим
	pDecoder->pPlugin->szLastError[0] = 0;

	//if (apImage->mb_ImageOpened || apImage->mp_ImageContext) {
	//	_ASSERTE(!apImage->mb_ImageOpened && !apImage->mp_ImageContext);
	//}

	if (pDecoder->pPlugin->FileOpen2(pFileName, lFileSize, pBuffer, lBuffer, &InfoImage))
	{
		MCHKHEAP;
		
		//apImage->mb_ImageOpened = true;
		//apImage->mp_ImageContext = InfoImage.pImageContext;
		(*ppFile)->Assign(pDecoder, InfoImage.pImageContext, &InfoImage);
		
		pInfo->Assign(InfoImage);
		///*g_Plugin.ImageX*/ apImage->nPages = InfoImage.nPages;
		///*g_Plugin.ImageX*/ apImage->Animation = InfoImage.Flags & PVD_IIF_ANIMATED/* 1 */;
		///*g_Plugin.ImageX*/ apImage->FileName = pFileName;
		//if (!wcsncmp(pFileName,L"\\\\?\\",4)) pFileName+=4;
		//lstrcpynW(/*g_Plugin.ImageX*/ apImage->FileNameData, pFileName, sizeofarray(/*g_Plugin.ImageX*/ apImage->FileNameData));
		//memset(&apImage->Info, 0, sizeof(apImage->Info));
		//apImage->Info.cbSize = sizeof(apImage->Info);
		//apImage->Info.iPage = 0;
		
		InfoPage.iPage = 0;
		if (pDecoder->pPlugin->PageInfo2(InfoImage.pImageContext, &InfoPage))
		{
			MCHKHEAP;
			pInfo->Assign(InfoPage);
			
			(*ppFile)->Info.Assign(InfoPage);
			
			///*g_Plugin.ImageX*/ apImage->lWidth = apImage->Info.lWidth;
			///*g_Plugin.ImageX*/ apImage->lHeight = apImage->Info.lHeight;
			///*g_Plugin.ImageX*/ apImage->nBPP = apImage->Info.nBPP;
			//if (apImage->Info.pFormatName)
			//	lstrcpyn(/*g_Plugin.ImageX*/ apImage->FormatName, apImage->Info.pFormatName, sizeofarray(/*g_Plugin.ImageX*/ apImage->FormatName));
			//else if (InfoImage.pFormatName)
			//	lstrcpyn(/*g_Plugin.ImageX*/ apImage->FormatName, InfoImage.pFormatName, sizeofarray(/*g_Plugin.ImageX*/ apImage->FormatName));
			//else
			//	* /*g_Plugin.ImageX*/ apImage->FormatName = 0;
			//if (apImage->Info.pCompression)
			//	lstrcpyn(/*g_Plugin.ImageX*/ apImage->Compression, apImage->Info.pCompression, sizeofarray(/*g_Plugin.ImageX*/ apImage->Compression));
			//else if (InfoImage.pCompression)
			//	lstrcpyn(/*g_Plugin.ImageX*/ apImage->Compression, InfoImage.pCompression, sizeofarray(/*g_Plugin.ImageX*/ apImage->Compression));
			//else
			//	* /*g_Plugin.ImageX*/ apImage->Compression = 0;
			//if (InfoImage.pComments)
			//	lstrcpyn(/*g_Plugin.ImageX*/ apImage->Comments, InfoImage.pComments, sizeofarray(/*g_Plugin.ImageX*/ apImage->Comments));
			//else
			//	* /*g_Plugin.ImageX*/ apImage->Comments = 0;
			result = true;
		}
	}
	
	MCHKHEAP;

	OutputDebugString(result ? L"Succeeded\n" : L"Failed!!!\n");
	if (!result)
	{
		if (pDecoder->pPlugin->szLastError[0]) {
			int nLen = lstrlen(szDbg);
			if ((nLen+10) < sizeofarray(szDbg)) {
				lstrcpyn(szDbg+nLen, pDecoder->pPlugin->szLastError, sizeofarray(szDbg)-nLen-1);
			}
		} else {
			lstrcat(szDbg, L"Failed!!! ");
		}
		pDecoder->pPlugin->SetStatus(szDbg);
		#ifdef _DEBUG
		if ((GetKeyState(VK_CAPITAL) & 1) == 1) {
			Sleep(1000);
		}
		#endif

		(*ppFile)->Close();
	}
	MCHKHEAP;
	return result;
}

bool CPVDManager::Open(
		CImage* apImage,
		CDecoderHandle** ppFile,
		const wchar_t *pFileName,
		i64 lFileSize, const u8 *pBuffer, uint lBuffer,
		DecodeParams *apParams)
		// pvdInfoPage2 &InfoPage теперь в CImage
{
	if (Decoders.empty())
		return false;

	bool result = false;
	//int i, s, nFrom, nTo, nCount, c; // строго со знаком!
	int s;
	
	//nCount = (int)Decoders.size();
	_ASSERTE(pFileName);

	TODO("Чтение файла может быть все-таки выполнять в память только при необходимости?");
	// Чтобы не дергать те файлы, которые точно не проходят по расширениям
    // Да и возможно, что это будет лучше по времени, чем делать FileMapping?

	DWORD nNextDecoder = (apParams->Flags & (eRenderNextDecoder|eRenderPrevDecoder));
	
	//int iFromDecoder = (nNextDecoder != 0) ? apImage->mi_DecoderIndex : 0;
	//CModuleInfo* pFromDecoder = apImage->GetDecoder();

	//if (iFromDecoder < 0 || iFromDecoder >= (int)Decoders.size())
	//{
	//	_ASSERTE(apImage->mi_DecoderIndex < (int)Decoders.size());
	//	iFromDecoder = 0;
	//}

	// Чтобы не "нюхать" файл несколько раз одним декодером
	// создается массив (apImage->mpb_Processed) декодеров, которыми пробовали открыть файл
	//apImage->CheckProcessedSize(nCount);
	
	//if (!apImage->mpb_Processed) {
	//	apImage->mn_ProcessedSize = nCount;
	//	apImage->mpb_Processed = (bool*)calloc(nCount,sizeof(bool));
	//}

	/* -- а вот это делать нельзя (memset...), иначе мы зациклимся, если модуль вывода вернет ошибку
	else if (!nNextDecoder || !mps_FileName) { // добавил !nNextDecoder
		memset(mpb_Processed, 0, nCount*sizeof(bool));
	} else if (lstrcmp(pFileName, mps_FileName)) {
		memset(mpb_Processed, 0, nCount*sizeof(bool));
	}
	*/

	//// Запомнить имя обрабатываемого файла
	//bool bNewFile = (mps_FileName == NULL);
	//if (mps_FileName)
	//	bNewFile = lstrcmp(mps_FileName, pFileName)!=0;
	//if (mps_FileName && bNewFile) {
	//	free(mps_FileName); mps_FileName = NULL;
	//}
	//if (bNewFile)
	//	mps_FileName = _wcsdup(pFileName);



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
	CModuleInfo* p = NULL;
	// Ищем в три прохода 
	// это задел на будущее - дать возможность на лету переключить декодер
	// Например AltPgDn / AltPgUp - достаточно iSubDecoder изменить и вызвать повторное определение
	// {{{
	// 1. iSubDecoder .. Count-1
	// 2. 0 .. iSubDecoder-1
	// }}} -- это чтобы "продолжать" перебор декодеров
	// 3. 0 .. Count-1, но по неактивным расширениям
	// 4. 0 .. Count-1, но и на неактивные расширения внимания уже не обращаем
	for (s = 1; !result && s <= 4; s++)
	{
		if (s >= 4 && !(apParams->Flags & eRenderForceDecoder))
			break; // "Усиленный подбор" без учета поддерживаемых расширений ТОЛЬКО на первом файле серии

		CModuleInfo* pFrom = (s == 1) ? apImage->GetDecoder() : NULL;

		//if (!nNextDecoder) // обычный режим. по возможности используем текущий декодер
		//{
		//	c = 1;
		//	switch (s)
		//	{
		//	case 1: nFrom = iFromDecoder; nTo = nCount; break;
		//	case 2: nFrom = 0; nTo = iFromDecoder; break;
		//	case 3: case 4: nFrom = 0; nTo = nCount; break;
		//	}
		//}
		//else if (nNextDecoder & eRenderPrevDecoder)
		//{
		//	c = -1;
		//	switch (s)
		//	{
		//	case 1: nFrom = iFromDecoder-1; nTo = -1; break;
		//	case 2: nFrom = nCount-1; nTo = iFromDecoder; break;
		//	case 3: case 4: nFrom = nCount-1; nTo = -1; break;
		//	}
		//}
		//else if (nNextDecoder & eRenderNextDecoder)
		//{
		//	c = 1;
		//	switch (s)
		//	{
		//	case 1: nFrom = iFromDecoder+1; nTo = nCount; break;
		//	case 2: nFrom = 0; nTo = iFromDecoder+1; break;
		//	case 3: case 4: nFrom = 0; nTo = nCount; break;
		//	}
		//}
		
		// (i>=0 && i<nCount) - на всякий случай, чтобы с диапазонами не ошибиться
		//for (i = nFrom; !result && i != nTo && (i>=0 && i<nCount); i+=c)
		while ((p = GetNextDecoder(pFrom, ((nNextDecoder & eRenderPrevDecoder) == 0))) != NULL)
		{
			pFrom = p; // сразу запомним, для следующего цикла

			if ((g_Plugin.FlagsWork & FW_TERMINATE))
			{
				return false;
			}
		
			TODO("Не получится сделать ReOpen этим же плагином?");
			//if (i < apImage->mn_ProcessedSize && apImage->WasDecoderFailed(i)) continue;
			if (apImage->WasDecoderFailed(p))
				continue;

			//p = Decoders[i];
			if (p->pPlugin == NULL)
			{
				_ASSERTE(p->pPlugin!=NULL); // Хм? Это когда может быть?
				continue;
			}

			//уже не пробуем. Ингорируемые НЕ игнорируем // На шаге 3 если по расширению подобрать плагин не получилось - 
			//// пробуем всеми подряд, причем не взирая на игнорируемые расширения
			//if ((s == 3) && wcschr(p->pPlugin->pIgnored,L'*')) // за исключением '*'
			_ASSERTE(p->pForbidden);
			// Теперь - всегда (запрещенные расширения)
			if (wcschr(p->pForbidden, L'*') || ExtensionMatch(p->pForbidden, pExt))
			{
				// Это расширение строго запрещено для этого декодера
				apImage->SetDecoderFailed(p); // чтобы повторно не обрабатывать
				continue;
			}

			_ASSERTE(p->pInactive && p->pActive);
			// проверка активных/неактивных расширений
			if (p->pPlugin->IsAllowed(
					pExt, true/*abAllowAsterisk*/, (s >= 3)/*abAllowInactive*/, 
					(s == 4)/*abAssumeActive*/))
			{
				//if (i < apImage->mn_ProcessedSize)
				//	apImage->mpb_Processed[i] = true; // чтобы повторно не обрабатывать

				TRY
				{
					if (OpenWith(apImage, ppFile, p, pFileName, lFileSize, pBuffer, lBuffer))
					{
						// Запомним, какие декодером удалось открыть файл (о декодировании речь пока не идет)
						apImage->SetDecoder(p);
						//apImage->mi_DecoderIndex = i;
						result = true;
						break;
					}
					else
					{
						// Запомним, что этим декодером открыть не удалось
						apImage->SetDecoderFailed(p);
					}
				}CATCH{
				}
			}
		}
		if (result)
			break;
	}

	return result;
}

CModuleInfo* CPVDManager::GetNextDecoder(const CModuleInfo* apDecoder, bool abForward)
{
	CModuleInfo* pNext = NULL;

	if (Decoders.empty())
	{
		_ASSERTE(!Decoders.empty());
		return NULL;
	}
	else if (abForward)
	{
		std::vector<CModuleInfo*>::iterator i;
		for (i = Decoders.begin(); i != Decoders.end(); ++i)
		{
			if (!apDecoder)
			{
				pNext = *i;
				break;
			}
			else if ((*i) == apDecoder)
			{
				++i;
				if (i != Decoders.end())
					pNext = *i;
				break;
			}
		}
		// -- должен вернуть NULL, если достигли конца списка
		//if (!pNext)
		//{
		//	i = Decoders.begin();
		//	pNext = *i;
		//}
	}
	else
	{
		std::vector<CModuleInfo*>::reverse_iterator i;
		for (i = Decoders.rbegin(); i != Decoders.rend(); ++i)
		{
			if (!apDecoder)
			{
				pNext = *i;
				break;
			}
			else if ((*i) == apDecoder)
			{
				++i;
				if (i != Decoders.rend())
					pNext = *i;
				break;
			}
		}
		// -- должен вернуть NULL, если достигли начала списка
		//if (!pNext)
		//{
		//	i = Decoders.rbegin();
		//	pNext = *i;
		//}
	}

	//_ASSERTE(pNext!=NULL); -- NULL допустим, конец перебора
	return pNext;
}

// Закрыть декодированные данные (PageFree2)
void CPVDManager::CloseData(CDecodeItem* apItem)
{
	if (!apItem)
	{
		_ASSERTE(apItem!=NULL);
		return;
	}
	
	if (!gnDecoderThreadId || GetCurrentThreadId() == gnDecoderThreadId)
	{
		if (apItem->pFile)
		{
			if (apItem->pFile && apItem->pFile->Decoder() && apItem->pFile->Decoder()->pPlugin)
			{
				apItem->pFile->Decoder()->pPlugin->PageFree2(apItem->pFile->Context(), /*(pvdInfoDecode2*)*/ &apItem->Data);
			}
			else
			{
				_ASSERTE(apItem->pFile && apItem->pFile->Decoder() && apItem->pFile->Decoder()->pPlugin);
			}
			apItem->Data.pImage = NULL;
		}
	}
	else
	{
		_ASSERTE(gnDecoderThreadId && GetCurrentThreadId() == gnDecoderThreadId);
	}
}

// Закрыть дескриптор декодера
void CPVDManager::CloseDecoder(CDecodeItem* apItem)
{
	if (!apItem)
	{
		_ASSERTE(apItem!=NULL);
		return;
	}
	
	if (!gnDecoderThreadId || GetCurrentThreadId() == gnDecoderThreadId)
	{
		// Заодно выполнит Close()
		SafeRelease(apItem->pFile,NULL); // Кто тут должен быть?
	}
	else
	{
		_ASSERTE(gnDecoderThreadId && GetCurrentThreadId() == gnDecoderThreadId);
	}
	//ResetProcessed(); -- низя. иначе циклится при ошибках вывода например
	// возможен вариант, когда результат декодирования не смог показать ни один модуль дисплея,
	// в этом случае необходимо ПРОДОЛЖИТЬ перебор декодеров, пока они не кончатся,
	// или пока не отработает какой-то модуль дисплея.
	//iSubDecoder = 0; // а вот это нужно. По F5 нужно повторно запустить определение декодера
}

// Закрыть дескриптор дисплея
void CPVDManager::CloseDisplay(CDecodeItem* apItem)
{
	if (!apItem)
	{
		_ASSERTE(apItem!=NULL);
		return;
	}
	
	if (!gnDisplayThreadId || GetCurrentThreadId() == gnDisplayThreadId)
	{
		// Заодно выполнит Close()
		SafeRelease(apItem->pDraw,NULL); // Кто тут должен быть?
	}
	else
	{
		_ASSERTE(gnDisplayThreadId && GetCurrentThreadId() == gnDisplayThreadId);
	}
}

BOOL __stdcall CPVDManager::DecodeCallback2(
		void *pDecodeCallbackContext2, UINT32 iStep, UINT32 nSteps, pvdInfoDecodeStep2* pImagePart)
{
	// Возможность отмены декодирования
	if (bCancelDecoding || (g_Plugin.FlagsWork & FW_TERMINATE))
		return FALSE;
	return TRUE;
}

bool CPVDManager::DecodePixels(CDecodeItem* apItem, bool abResetTick)
{
	WARNING("В аргументы необходимо добавить индекс декодируемой страницы. Пока - загружается apImage->Info.nPage");
	WARNING("Разделить функцию на две ==> CPVDManager::Decode  &&  CPVDManager::TransferToDisplay");
	WARNING("CPVDManager::TransferToDisplay Должна вызываться в нити дисплея!");
	bool result = false;

	WARNING("apItem->pFile->Decoder() запомнить в локальной переменной и звать AddRef/Release");

	//if (!CPVDManager::pDefaultDisplay || !CPVDManager::pDefaultDisplay->pPlugin) {
	//	OutputDebugString(L"CPVDManager::Decode==> PVDManager::pDefaultDisplay->pPlugin was not created!");
	//	return false;
	//}
	
	if (!apItem->pImage)
	{
		_ASSERTE(apItem->pImage!=NULL);
		OutputDebugString(L"CPVDManager::DecodePixels==> apItem->pImage was not created!\n");
		return false;
	}
	if (!apItem->pFile)
	{
		_ASSERTE(apItem->pFile!=NULL);
		OutputDebugString(L"CPVDManager::DecodePixels==> apItem->pFile was not created!\n");
		return false;
	}
	_ASSERTE(apItem->pFile && apItem->pFile->Decoder() && apItem->pFile->Decoder()->pPlugin);
	
	MCHKHEAP

	//CModuleInfo* lpDisplay = NULL;

	if (abResetTick) // При декодировании следующей страницы сбросить тик (там сейчас время открытия)
		apItem->pImage->Info.lStartOpenTime = timeGetTime();
	DWORD t0 = apItem->pImage->Info.lStartOpenTime;
	DWORD t1 = timeGetTime(), t2=t1, t3=t1, t4=t1, t6=t1; //, t5=t1, t6=t1;

	wchar_t szLastError[128]; szLastError[0] = 0;
	wchar_t szDbg[MAX_PATH*3], *pNamePart;
	int nCur = 0, nSize = sizeofarray(szDbg);
	lstrcpyn(szDbg+nCur, apItem->pFile->Decoder()->pPlugin->pName, 128); nCur += lstrlen(szDbg+nCur);
	wsprintf(szDbg+nCur, L": Decoding page %i of image \"", apItem->pImage->Info.nPage); nCur += lstrlen(szDbg+nCur);
	const wchar_t* pFileName = (const wchar_t*)apItem->pImage->FileName;
	pNamePart = pFileName ? wcsrchr(pFileName, L'\\') : L"<NULL>";
	pNamePart = pNamePart ? (pNamePart+1) : (pFileName ? pFileName : L"<NULL>");
	lstrcpyn(szDbg+nCur, pNamePart, MAX_PATH); nCur += lstrlen(szDbg+nCur);
	lstrcpy(szDbg+nCur, L"\"… ");
	//wnsprintf(szDbg, sizeofarray(szDbg), L"%s: Decoding page %i of image %s...",
	//	pDecoder->pPlugin->pName, apItem->pImage->nPage, (const wchar_t*)apItem->pImage->FileName);
	OutputDebugString(szDbg);

	// Сбросим
	apItem->pFile->Decoder()->pPlugin->szLastError[0] = 0;
	
	//pvdInfoPage2 InfoPage = {sizeof(pvdInfoPage2)};
	//TRY{
	
	bool bPageInfoOk = false;
	//if (apItem->pImage->Info.cbSize && apItem->pImage->Info.iPage == apItem->pImage->Info.nPage && apItem->pImage->Info.lWidth && apItem->pImage->Info.lHeight)
	if (apItem->pFile->Info.IsPageLoaded(apItem->pImage->Info.nPage))
	{
		bPageInfoOk = true;
	}
	else
	{
		pvdInfoPage2 InfoPage = {sizeof(pvdInfoPage2)};
		InfoPage.iPage = apItem->pImage->Info.nPage;
		bPageInfoOk = apItem->pFile->Decoder()->pPlugin->PageInfo2(apItem->pFile->Context(), &InfoPage);
		if (bPageInfoOk)
		{
			apItem->pImage->Info.Assign(InfoPage);
			//if (apItem->pImage->Info.pFormatName)
			//	lstrcpyn(apItem->pImage->FormatName, apItem->pImage->Info.pFormatName, sizeofarray(apItem->pImage->FormatName));
			//if (apItem->pImage->Info.pCompression)
			//	lstrcpyn(apItem->pImage->Compression, apItem->pImage->Info.pCompression, sizeofarray(apItem->pImage->FormatName));
		}
		if (bPageInfoOk && apItem->pImage->Info.nPages)
			apItem->pImage->CheckPages(apItem->pImage->Info.nPages);
	}

	if (bPageInfoOk)
	{
		t2 = timeGetTime();
		//pvdInfoDecode2 DecodeInfo = {sizeof(pvdInfoDecode2)};
		bool bNeedFree = true;
		//TODO("Попробовать создавать 24 битные поверхности для ускорения их инициализации. это нужно делать после !PageDecode2!");
		//TODO("Поменять местами PageDecode2 и CreateWorkSurface. В некоторых случаях мы вообще не сможем наполнить поверхность декодированным colormodel");
		//WARNING("24 битные поверхности нифига не блитятся в Primary surface");

		if (apItem->Data.cbSize && apItem->Data.pImage)
		{
			_ASSERTE(!apItem->Data.cbSize || !apItem->Data.pImage);
			WARNING("Старые(?) данные нужно освободить");
		}
		apItem->Data.cbSize = sizeof(apItem->Data);

		// Execute
		{
			t3 = timeGetTime();
			apItem->Data.iPage = apItem->pImage->Info.nPage;

			apItem->Data.Flags = (apItem->pFile->Decoder()->pPlugin == CPVDManager::pDefaultDisplay->pPlugin) ? PVD_IDF_ASDISPLAY : 0;
			apItem->Data.nBackgroundColor = g_Plugin.BackColor();

			MCHKHEAP

			SIZE scaledSize = {0,0};
			if (
			    (apItem->pFile->Decoder()->pPlugin->nFlags & (PVD_IP_CANDESCALE|PVD_IP_CANUPSCALE)) &&
				(g_Plugin.ZoomAuto || g_Plugin.AbsoluteZoom || g_Plugin.FlagsWork & FW_QUICK_VIEW)
			   )
			{
				//TODO("Нужно доработать условия");

				SIZE imgSize = {apItem->pImage->Info.lWidth,apItem->pImage->Info.lHeight};
				SIZE renderSize = apItem->pImage->GetDefaultScaleSize();

				//if (g_Plugin.FlagsWork & FW_QUICK_VIEW)
				if (renderSize.cx && renderSize.cy && imgSize.cx && imgSize.cy) {
					BOOL lbCanScale = FALSE;

					if (renderSize.cx < imgSize.cx && renderSize.cy < imgSize.cy)
					{
						lbCanScale = (apItem->pFile->Decoder()->pPlugin->nFlags & PVD_IP_CANDESCALE);
					}
					else if (renderSize.cx > imgSize.cx && renderSize.cy > imgSize.cy)
					{
						lbCanScale = (apItem->pFile->Decoder()->pPlugin->nFlags & PVD_IP_CANUPSCALE);
					}

					if (lbCanScale)
					{
						double dr = (double)renderSize.cx / (double)renderSize.cy;
						double di = (double)imgSize.cx / (double)imgSize.cy;
						if (g_Plugin.ZoomAuto == ZA_FIT) {
							if (di > dr)
							{
								// Вписать по высоте
								scaledSize.cx = (LONG)(renderSize.cy * di);
								scaledSize.cy = renderSize.cy;
							}
							else
							{
								// Вписать по ширине
								scaledSize.cx = renderSize.cx;
								scaledSize.cy = (LONG)(renderSize.cx / di);
							}
						}
						else if (g_Plugin.ZoomAuto == ZA_FILL)
						{
							if (di < dr)
							{
								// Вписать по высоте
								scaledSize.cx = (LONG)(renderSize.cy * di);
								scaledSize.cy = renderSize.cy;
							}
							else
							{
								// Вписать по ширине
								scaledSize.cx = renderSize.cx;
								scaledSize.cy = (LONG)(renderSize.cx / di);
							}
						}
						else if (di > dr)
						{
							// Вписать по высоте
							scaledSize.cx = (LONG)(renderSize.cy * di);
							scaledSize.cy = renderSize.cy;
						}
						else
						{
							// Вписать по ширине
							scaledSize.cx = renderSize.cx;
							scaledSize.cy = (LONG)(renderSize.cx / di);
						}
						apItem->Data.lWidth = scaledSize.cx;
						apItem->Data.lHeight = scaledSize.cy;
					}
				}
			}

			MCHKHEAP

			//_ASSERTE(g_Plugin.hWnd);
			if (!g_Plugin.hWnd || !IsWindow(g_Plugin.hWnd))
			{
				lstrcpy(szLastError, L"PicView window was closed");
				result = false;
			}
			else if (apItem->pFile->Decoder()->pPlugin->PageDecode2(apItem->pFile->Context(),
					/*(pvdInfoDecode2*)*/ &apItem->Data, DecodeCallback2, this))
			{
				MCHKHEAP;

				// Сразу выставим OK
				result = true;

				// Обновить "основной" дескриптор декодера
				if (apItem->pImage->mp_File != apItem->pFile)
				{
					SafeRelease(apItem->pImage->mp_File,NULL); // Кто тут должен быть?
					apItem->pFile->AddRef(szPVDManager);
					apItem->pImage->mp_File = apItem->pFile;
				}

				if (apItem->Data.Flags & PVD_IDF_ALPHA)
				{
					_ASSERTE(apItem->Data.ColorModel == PVD_CM_BGRA);
				}
				
				// Обновить информацию о размере изображения!
				if (apItem->Data.lSrcWidth && apItem->Data.lSrcHeight)
				{
					apItem->pImage->Info.lWidth = apItem->Data.lSrcWidth;
					apItem->pImage->Info.lHeight = apItem->Data.lSrcHeight;
				}
				apItem->pImage->Info.lDecodedWidth = apItem->Data.lWidth;
				apItem->pImage->Info.lDecodedHeight = apItem->Data.lHeight;
				apItem->pImage->Info.nDecodedBPP = apItem->Data.nBPP;
				
				//apItem->pImage->Info.nBPP = apItem->Data.nBPP; -- так не надо. Изображение может быть и 8-битным, а данные декодированы в 32 бита
				//
				apItem->pImage->Info.Assign(apItem->Data, pNamePart);
				//if (apItem->Data.pFormatName)
				//	lstrcpyn(apItem->pImage->FormatName, apItem->Data.pFormatName, sizeofarray(apItem->pImage->FormatName));
				//if (apItem->Data.pCompression)
				//	lstrcpyn(apItem->pImage->Compression, apItem->Data.pCompression, sizeofarray(apItem->pImage->Compression));
				//if (!apItem->pImage->FormatName[0])
				//{
				//	const wchar_t* pszExt = wcsrchr(pNamePart, L'.');
				//	if (pszExt) {
				//		apItem->pImage->FormatName[0] = L'[';
				//		lstrcpyn(apItem->pImage->FormatName+1, pszExt+1, sizeofarray(apItem->pImage->FormatName)-3);
				//		lstrcat(apItem->pImage->FormatName, L"]");
				//	}
				//}
				
				MCHKHEAP

				t4 = timeGetTime();
				if (apItem->Data.nPages)
					apItem->pImage->CheckPages(apItem->Data.nPages);
				//DWORD nFlags = apItem->Data.Flags;
				//if (nFlags < 2) {
				//	//nFlags |= (apItem->Data.nBPP == 32) ? PVD_IDF_BGRA : PVD_IDF_BGR;
				//}
				TODO("Флаги из 2-й версии интерфейса");

				#if 0
				{
					//WARNING("На анимированных и многостраничных документах тут НЕ NULL");
					////_ASSERTE(*ppDrawContext == NULL);
					//if (*ppDrawContext) {
					//	// А тут именно так. На будущее. В CImage может быть несколько DrawContext, чтобы не закрыть нужный...
					//	_ASSERTE(apItem->pImage->Display->pPlugin);
					//	apItem->pImage->Display->pPlugin->DisplayClose2(*ppDrawContext);
					//	*ppDrawContext = NULL;
					//}
					
					//result = dds->Blit(&apItem->Data, g_Plugin.BackColor);
					pvdInfoDisplayCreate2 DsCreate = {sizeof(pvdInfoDisplayCreate2)};
					#ifdef _DEBUG
						if (apItem->Data.ColorModel == PVD_CM_UNKNOWN)
						{
							if (lstrcmpi(apItem->pFile->Decoder()->pModuleFileName, L"GDIPlus.pvd"))
							{
								_ASSERTE(apItem->Data.ColorModel != PVD_CM_UNKNOWN);
							}
						}
					#endif
					// Некоторые декодеры НЕ возвращают палитру для индексированных форматов.
					// Нужно подставить палитру по умолчанию, а чтобы декодер не разрушил НАШУ
					// память - и делаем копию apItem->Data
					pvdInfoDecode2 DecodeInfoCopy = apItem->Data;
					if (DecodeInfoCopy.nBPP<=8 && DecodeInfoCopy.ColorModel!=PVD_CM_PRIVATE)
					{
						if (!DecodeInfoCopy.pPalette)
						{
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
					
					if ((apItem->Data.Flags & PVD_IDF_PRIVATE_DISPLAY) && (apItem->pFile->Decoder()->pPlugin->nCurrentFlags & PVD_IP_DISPLAY))
					{
						lpDisplay = apItem->pImage->mp_DecoderModule;
					}
					else
					{
						lpDisplay = CPVDManager::pDefaultDisplay;
					}
					CModuleInfo* pStartDisplay = lpDisplay;
					std::vector<CModuleInfo*>::iterator iDisp = CPVDManager::Displays.begin();
					BOOL lbFirstCheckDisplay = TRUE;

					// Если g_Plugin.hWnd == 0 - значит юзер уже нажал Esc и все закрывается
					while (g_Plugin.hWnd && iDisp != CPVDManager::Displays.end())
					{
						if (!lpDisplay->pPlugin) {
							_ASSERTE(lpDisplay->pPlugin);
							result = FALSE;
						} else {
							if (result = lpDisplay->pPlugin->DisplayCheck()) {
								MCHKHEAP
								if (!g_Plugin.hWnd) {
									result = false;
									break;
								}
								if (result = lpDisplay->pPlugin->DisplayCreate2(&DsCreate))
									break;
								wchar_t szMsg[128];
								lstrcpy(szMsg, L"Display module skipped: ");
								lstrcpy(szMsg+lstrlen(szMsg), lpDisplay->pPlugin->pName);
								CPVDManager::AddLogString(L"PVDManager", szMsg);
							}
						}

						if ((apItem->Data.Flags & PVD_IDF_PRIVATE_DISPLAY) || CPVDManager::Displays.size()<=1)
							break;

						if (!lbFirstCheckDisplay)
							iDisp++;
						else
							lbFirstCheckDisplay = FALSE;
						// Найти следующий готовый дисплей
						while (iDisp != CPVDManager::Displays.end()
							&& ((pStartDisplay == *iDisp) 
							    || ((*iDisp)->pPlugin == NULL)
							    || ((*iDisp)->nCurrentFlags & PVD_IP_PRIVATE)))
							iDisp++;
						if (iDisp == CPVDManager::Displays.end()) break;

						// Запомнить новый дисплей
						lpDisplay = *iDisp;
						wchar_t szMsg[128];
						lstrcpy(szMsg, L"Switching to display module: ");
						lstrcpy(szMsg+lstrlen(szMsg), lpDisplay->pPlugin->pName);
						CPVDManager::AddLogString(L"PVDManager", szMsg);
					}

					if (result)
					{
						(*ppDraw)->Assign(lpDisplay, DsCreate.pDisplayContext);
						//*ppDrawContext = DsCreate.pDisplayContext;
					}

						//apItem->Data.pImage, apItem->Data.lImagePitch, apItem->Data.nBPP, g_Plugin.BackColor, 
						//nFlags, apItem->Data.nTransparentColor, apItem->Data.pPalette);
				}
				t5 = timeGetTime();
				bNeedFree = false;
				MCHKHEAP
				apItem->pFile->Decoder()->pPlugin->PageFree2(apItem->pFile->Context(), &apItem->Data);
				#endif
			}
		}
		if (!g_Plugin.hWnd || !IsWindow(g_Plugin.hWnd))
		{
			lstrcpy(szLastError, L"PicView window was closed");
			result = false;
		}
		MCHKHEAP
		if (result)
		{
			apItem->pImage->Info.lWidth = apItem->pImage->Info.lWidth;
			apItem->pImage->Info.lHeight = apItem->pImage->Info.lHeight;
			apItem->pImage->Info.nBPP = apItem->pImage->Info.nBPP;
			if (apItem->pImage->Info.Animation && !apItem->pImage->Info.lFrameTime)
				apItem->pImage->Info.lFrameTime = 100; // минимальное умолчательное время фрейма
			apItem->pImage->Info.lFrameTime = apItem->pImage->Info.lFrameTime;
		}
	}
	t6 = timeGetTime();
	apItem->pImage->Info.lTimeOpen = t2-t0; //Открытие файла изображения и получение информации о нем (Open & PageInfo)
		_ASSERTE(apItem->pImage->Info.lTimeOpen>=0); if (apItem->pImage->Info.lTimeOpen<0) apItem->pImage->Info.lTimeOpen = 0;
	apItem->pImage->Info.lTimeDecode = (t4==t1) ? 0 : (t4-t3); // PageDecode
		_ASSERTE(apItem->pImage->Info.lTimeDecode>=0); if (apItem->pImage->Info.lTimeDecode<0) apItem->pImage->Info.lTimeDecode = 0;
	apItem->pImage->Info.lTimeTransfer = 0 ; //((t5==t1) ? 0 : max(0,t5-t4)) + ((t3==t1) ? 0 : max(0,t3-t2)); // CreateWorkSurface+Blit
	//	_ASSERTE(apItem->pImage->Info.lTimeTransfer>=0); if (apItem->pImage->Info.lTimeTransfer<0) apItem->pImage->Info.lTimeTransfer = 0;
	apItem->pImage->Info.lTimePaint = 0;
	apItem->pImage->Info.bTimePaint = FALSE;
	//wsprintf(apItem->pImage->OpenTimes, L"%i+%i+%i", 
	//	t2-t0, //Открытие файла изображения и получение информации о нем (Open & PageInfo)
	//	(t4==t1) ? 0 : t4-t3, // PageDecode
	//	((t5==t1) ? 0 : t5-t4) + ((t3==t1) ? 0 : t3-t2) // CreateWorkSurface+Blit
	//	);
	apItem->pImage->Info.lOpenTime = t6-apItem->pImage->Info.lStartOpenTime;
	//}CATCH{}

	MCHKHEAP
	OutputDebugString(result ? L"Succeeded\n" : L"Failed!!!\n");
	if (result)
	{
		WARNING("Дернуть нить дисплея");
	}
	else
	{
		LPCWSTR pszError = NULL;
		if (szLastError[0])
		{
			pszError = szLastError;
		}
		else if (apItem->pFile->Decoder()->pPlugin->szLastError[0])
		{
			pszError = apItem->pFile->Decoder()->pPlugin->szLastError;
		}
		else
		{
			pszError = L"Failed!!! ";
		}
		if (pszError)
		{
			int nLen = lstrlen(szDbg);
			if ((nLen+10) < sizeofarray(szDbg))
			{
				lstrcpyn(szDbg+nLen, pszError, sizeofarray(szDbg)-nLen-1);
			}
		}
		apItem->pFile->Decoder()->pPlugin->SetStatus(szDbg);
	}

	return result;
}

bool CPVDManager::PixelsToDisplay(CDecodeItem* apItem)
{
	bool result = false;

	if (!apItem || !apItem->pImage || !apItem->pFile)
	{
		_ASSERTE(apItem && apItem->pImage && apItem->pFile);
		OutputDebugString(L"CPVDManager::PixelsToDisplay==> (!apItem || !apItem->pImage || !apItem->pFile)\n");
		return false;
	}

	if (!CPVDManager::pDefaultDisplay || !CPVDManager::pDefaultDisplay->pPlugin)
	{
		_ASSERTE(CPVDManager::pDefaultDisplay && CPVDManager::pDefaultDisplay->pPlugin);
		OutputDebugString(L"CPVDManager::PixelsToDisplay==> PVDManager::pDefaultDisplay->pPlugin was not created!\n");
		return false;
	}

	if (!apItem->pDraw)
	{
		apItem->pDraw = new CDisplayHandle(szPVDManager, apItem->pImage/*, apItem->pFile*/);
	}
	else
	{
		_ASSERTE(apItem->pDraw->Context() == NULL);
		apItem->pDraw->Close(); // закрыть старый контекст дисплея
	}
	
	MCHKHEAP
	
	DWORD t4 = timeGetTime(), t5=t4, t6=t4;
	
	//WARNING("На анимированных и многостраничных документах тут НЕ NULL");
	////_ASSERTE(*ppDrawContext == NULL);
	//if (*ppDrawContext) {
	//	// А тут именно так. На будущее. В CImage может быть несколько DrawContext, чтобы не закрыть нужный...
	//	_ASSERTE(apItem->pImage->Display->pPlugin);
	//	apItem->pImage->Display->pPlugin->DisplayClose2(*ppDrawContext);
	//	*ppDrawContext = NULL;
	//}
	
	//TODO("Попробовать создавать 24 битные поверхности для ускорения их инициализации. это нужно делать после !PageDecode2!");
	//TODO("Поменять местами PageDecode2 и CreateWorkSurface. В некоторых случаях мы вообще не сможем наполнить поверхность декодированным colormodel");
	//WARNING("24 битные поверхности нифига не блитятся в Primary surface");
	
	//result = dds->Blit(&DecodeInfo, g_Plugin.BackColor);
	pvdInfoDisplayCreate2 DsCreate = {sizeof(pvdInfoDisplayCreate2)};
	#ifdef _DEBUG
		if (apItem->Data.ColorModel == PVD_CM_UNKNOWN)
		{
			if (lstrcmpi(apItem->pFile->Decoder()->pModuleFileName, L"GDIPlus.pvd"))
			{
				_ASSERTE(apItem->Data.ColorModel != PVD_CM_UNKNOWN);
			}
		}
	#endif
	// Некоторые декодеры НЕ возвращают палитру для индексированных форматов.
	// Нужно подставить палитру по умолчанию, а чтобы декодер не разрушил НАШУ
	// память - и делаем копию DecodeInfo
	pvdInfoDecode2 DecodeInfoCopy = apItem->Data;
	if (DecodeInfoCopy.nBPP<=8 && DecodeInfoCopy.ColorModel!=PVD_CM_PRIVATE)
	{
		if (!DecodeInfoCopy.pPalette)
		{
			DecodeInfoCopy.pPalette = g_Plugin.GetPalette(DecodeInfoCopy.nBPP, DecodeInfoCopy.ColorModel);
		}
	}
	DsCreate.pImage = &DecodeInfoCopy;
	DsCreate.BackColor = g_Plugin.BackColor();
	DsCreate.pDisplayContext = NULL;
	DsCreate.pFileName = apItem->pImage->pszFileNameOnly;
	//DsCreate.nChessMateColor1 = 0xFFFFFF;
	//DsCreate.nChessMateColor2 = 0xCCCCCC;
	//DsCreate.nChessMateSize = 8;
	
	WARNING("Проверять флаг PVD_IDF_PRIVATE_DISPLAY. Если установлен - вывод возможен только через модуль декодера");
	
	CModuleInfo* lpDisplay = NULL;
	
	if ((apItem->Data.Flags & PVD_IDF_PRIVATE_DISPLAY) && (apItem->pFile->Decoder()->pPlugin->nCurrentFlags & PVD_IP_DISPLAY))
	{
		lpDisplay = apItem->pImage->mp_File->Decoder();
	}
	else
	{
		lpDisplay = CPVDManager::pDefaultDisplay;
	}
	CModuleInfo* pStartDisplay = lpDisplay;
	std::vector<CModuleInfo*>::iterator iDisp = CPVDManager::Displays.begin();
	BOOL lbFirstCheckDisplay = TRUE;

	// Если g_Plugin.hWnd == 0 - значит юзер уже нажал Esc и все закрывается
	while (g_Plugin.hWnd && iDisp != CPVDManager::Displays.end())
	{
		if (!lpDisplay->pPlugin)
		{
			_ASSERTE(lpDisplay->pPlugin);
			result = FALSE;
		}
		else
		{
			if (result = lpDisplay->pPlugin->DisplayCheck())
			{
				MCHKHEAP
				if (!g_Plugin.hWnd)
				{
					result = false;
					break;
				}
				if (result = lpDisplay->pPlugin->DisplayCreate2(&DsCreate))
					break;
				wchar_t szMsg[128];
				lstrcpy(szMsg, L"Display module skipped: ");
				lstrcpy(szMsg+lstrlen(szMsg), lpDisplay->pPlugin->pName);
				CPVDManager::AddLogString(L"PVDManager", szMsg);
			}
		}

		if ((apItem->Data.Flags & PVD_IDF_PRIVATE_DISPLAY) || CPVDManager::Displays.size()<=1)
			break;

		if (!lbFirstCheckDisplay)
			iDisp++;
		else
			lbFirstCheckDisplay = FALSE;
		// Найти следующий готовый дисплей
		while (iDisp != CPVDManager::Displays.end()
			&& ((pStartDisplay == *iDisp) 
			    || ((*iDisp)->pPlugin == NULL)
			    || ((*iDisp)->nCurrentFlags & PVD_IP_PRIVATE)))
			iDisp++;
		if (iDisp == CPVDManager::Displays.end()) break;

		// Запомнить новый дисплей
		lpDisplay = *iDisp;
		wchar_t szMsg[128];
		lstrcpy(szMsg, L"Switching to display module: ");
		lstrcpy(szMsg+lstrlen(szMsg), lpDisplay->pPlugin->pName);
		CPVDManager::AddLogString(L"PVDManager", szMsg);
	}

	if (result)
	{
		apItem->pDraw->Assign(lpDisplay, DsCreate.pDisplayContext);
		//*ppDrawContext = DsCreate.pDisplayContext;
	}
	
	t5 = timeGetTime();

		//DecodeInfo.pImage, DecodeInfo.lImagePitch, DecodeInfo.nBPP, g_Plugin.BackColor, 
		//nFlags, DecodeInfo.nTransparentColor, DecodeInfo.pPalette);
		
	apItem->pImage->Info.lTimeTransfer = max(0,t5-t4); // ((t5==t4) ? 0 : max(0,t5-t4)) + ((t3==t1) ? 0 : max(0,t3-t2)); // CreateWorkSurface+Blit
		_ASSERTE(apItem->pImage->Info.lTimeTransfer>=0); if (apItem->pImage->Info.lTimeTransfer<0) apItem->pImage->Info.lTimeTransfer = 0;

	return result;
}

#if 0
bool CPVDManager::Decode(CImage *apImage, CDecoderHandle* pFile, CDisplayHandle** ppDraw, bool abResetTick)
{
	WARNING("В аргументы необходимо добавить индекс декодируемой страницы. Пока - загружается apImage->Info.nPage");
	WARNING("Разделить функцию на две ==> CPVDManager::Decode  &&  CPVDManager::TransferToDisplay");
	WARNING("CPVDManager::TransferToDisplay Должна вызываться в нити дисплея!");
	bool result = false;

	if (!CPVDManager::pDefaultDisplay || !CPVDManager::pDefaultDisplay->pPlugin)
	{
		OutputDebugString(L"CPVDManager::Decode==> PVDManager::pDefaultDisplay->pPlugin was not created!");
		return false;
	}
	
	if (!pFile)
	{
		OutputDebugString(L"CPVDManager::Decode==> apImage->mp_File was not created!");
		return false;
	}
	_ASSERTE(pFile && pFile->Decoder() && pFile->Decoder()->pPlugin);
	
	if (!*ppDraw)
	{
		*ppDraw = new CDisplayHandle(apImage, pFile);
	}
	else
	{
		_ASSERTE((*ppDraw)->Context() == NULL);
		(*ppDraw)->Close();
	}
	
	MCHKHEAP

	CModuleInfo* lpDisplay = NULL;

	if (abResetTick) // При декодировании следующей страницы сбросить тик (там сейчас время открытия)
		apImage->Info.lStartOpenTime = timeGetTime();
	DWORD t0 = apImage->Info.lStartOpenTime;
	DWORD t1 = timeGetTime(), t2=t1, t3=t1, t4=t1, t5=t1, t6=t1;

	wchar_t szLastError[128]; szLastError[0] = 0;
	wchar_t szDbg[MAX_PATH*3], *pNamePart;
	int nCur = 0, nSize = sizeofarray(szDbg);
	lstrcpyn(szDbg+nCur, pFile->Decoder()->pPlugin->pName, 128); nCur += lstrlen(szDbg+nCur);
	wsprintf(szDbg+nCur, L": Decoding page %i of image \"", apImage->Info.nPage); nCur += lstrlen(szDbg+nCur);
	const wchar_t* pFileName = (const wchar_t*)apImage->FileName;
	pNamePart = pFileName ? wcsrchr(pFileName, L'\\') : L"<NULL>";
	pNamePart = pNamePart ? (pNamePart+1) : (pFileName ? pFileName : L"<NULL>");
	lstrcpyn(szDbg+nCur, pNamePart, MAX_PATH); nCur += lstrlen(szDbg+nCur);
	lstrcpy(szDbg+nCur, L"\"… ");
	//wnsprintf(szDbg, sizeofarray(szDbg), L"%s: Decoding page %i of image %s...",
	//	pDecoder->pPlugin->pName, apImage->nPage, (const wchar_t*)apImage->FileName);
	OutputDebugString(szDbg);

	// Сбросим
	pFile->Decoder()->pPlugin->szLastError[0] = 0;
	
	//pvdInfoPage2 InfoPage = {sizeof(pvdInfoPage2)};
	//TRY{
	
	bool bPageInfoOk = false;
	//if (apImage->Info.cbSize && apImage->Info.iPage == apImage->Info.nPage && apImage->Info.lWidth && apImage->Info.lHeight)
	if (pFile->Info.IsPageLoaded(apImage->Info.nPage))
	{
		bPageInfoOk = true;
	}
	else
	{
		pvdInfoPage2 InfoPage = {sizeof(pvdInfoPage2)};
		InfoPage.iPage = apImage->Info.nPage;
		bPageInfoOk = pFile->Decoder()->pPlugin->PageInfo2(pFile->Context(), &InfoPage);
		if (bPageInfoOk)
		{
			if (apImage->Info.pFormatName)
				lstrcpyn(apImage->FormatName, apImage->Info.pFormatName, sizeofarray(apImage->FormatName));
			if (apImage->Info.pCompression)
				lstrcpyn(apImage->Compression, apImage->Info.pCompression, sizeofarray(apImage->FormatName));
		}
		if (bPageInfoOk && apImage->Info.nPages)
			apImage->CheckPages(apImage->Info.nPages);
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
			DecodeInfo.iPage = apImage->Info.nPage;

			DecodeInfo.Flags = (pFile->Decoder()->pPlugin == CPVDManager::pDefaultDisplay->pPlugin) ? PVD_IDF_ASDISPLAY : 0;
			DecodeInfo.nBackgroundColor = g_Plugin.BackColor();

			MCHKHEAP

			SIZE scaledSize = {0,0};
			if (
			    (pFile->Decoder()->pPlugin->nFlags & (PVD_IP_CANDESCALE|PVD_IP_CANUPSCALE)) &&
				(g_Plugin.ZoomAuto || g_Plugin.AbsoluteZoom || g_Plugin.FlagsWork & FW_QUICK_VIEW)
			   )
			{
				//TODO("Нужно доработать условия");

				SIZE imgSize = {apImage->Info.lWidth,apImage->Info.lHeight};
				SIZE renderSize = apImage->GetDefaultScaleSize();

				//if (g_Plugin.FlagsWork & FW_QUICK_VIEW)
				if (renderSize.cx && renderSize.cy && imgSize.cx && imgSize.cy)
				{
					BOOL lbCanScale = FALSE;

					if (renderSize.cx < imgSize.cx && renderSize.cy < imgSize.cy)
					{
						lbCanScale = (pFile->Decoder()->pPlugin->nFlags & PVD_IP_CANDESCALE);
					}
					else if (renderSize.cx > imgSize.cx && renderSize.cy > imgSize.cy)
					{
						lbCanScale = (pFile->Decoder()->pPlugin->nFlags & PVD_IP_CANUPSCALE);
					}

					if (lbCanScale)
					{
						double dr = (double)renderSize.cx / (double)renderSize.cy;
						double di = (double)imgSize.cx / (double)imgSize.cy;
						if (g_Plugin.ZoomAuto == ZA_FIT)
						{
							if (di > dr)
							{
								// Вписать по высоте
								scaledSize.cx = (LONG)(renderSize.cy * di);
								scaledSize.cy = renderSize.cy;
							}
							else
							{
								// Вписать по ширине
								scaledSize.cx = renderSize.cx;
								scaledSize.cy = (LONG)(renderSize.cx / di);
							}
						}
						else if (g_Plugin.ZoomAuto == ZA_FILL)
						{
							if (di < dr)
							{
								// Вписать по высоте
								scaledSize.cx = (LONG)(renderSize.cy * di);
								scaledSize.cy = renderSize.cy;
							}
							else
							{
								// Вписать по ширине
								scaledSize.cx = renderSize.cx;
								scaledSize.cy = (LONG)(renderSize.cx / di);
							}
						}
						else if (di > dr)
						{
							// Вписать по высоте
							scaledSize.cx = (LONG)(renderSize.cy * di);
							scaledSize.cy = renderSize.cy;
						}
						else
						{
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
			if (pFile->Decoder()->pPlugin->PageDecode2(pFile->Context(), /*(pvdInfoDecode2*)*/ &DecodeInfo, DecodeCallback2, this))
			{
				MCHKHEAP

				if (DecodeInfo.Flags & PVD_IDF_ALPHA)
				{
					_ASSERTE(DecodeInfo.ColorModel == PVD_CM_BGRA);
				}
				
				// Обновить информацию о размере изображения!
				if (DecodeInfo.lSrcWidth && DecodeInfo.lSrcHeight)
				{
					apImage->Info.lWidth = DecodeInfo.lSrcWidth;
					apImage->Info.lHeight = DecodeInfo.lSrcHeight;
				}
				apImage->Info.lDecodedWidth = DecodeInfo.lWidth;
				apImage->Info.lDecodedHeight = DecodeInfo.lHeight;
				apImage->Info.nDecodedBPP = DecodeInfo.nBPP;
				
				//apImage->Info.nBPP = DecodeInfo.nBPP; -- так не надо. Изображение может быть и 8-битным, а данные декодированы в 32 бита
				//
				if (DecodeInfo.pFormatName)
					lstrcpyn(apImage->FormatName, DecodeInfo.pFormatName, sizeofarray(apImage->FormatName));
				if (DecodeInfo.pCompression)
					lstrcpyn(apImage->Compression, DecodeInfo.pCompression, sizeofarray(apImage->Compression));
				if (!apImage->FormatName[0])
				{
					const wchar_t* pszExt = wcsrchr(pNamePart, L'.');
					if (pszExt)
					{
						apImage->FormatName[0] = L'[';
						lstrcpyn(apImage->FormatName+1, pszExt+1, sizeofarray(apImage->FormatName)-3);
						lstrcat(apImage->FormatName, L"]");
					}
				}
				
				MCHKHEAP

				t4 = timeGetTime();
				if (DecodeInfo.nPages)
					apImage->CheckPages(DecodeInfo.nPages);
				//DWORD nFlags = DecodeInfo.Flags;
				//if (nFlags < 2) {
				//	//nFlags |= (DecodeInfo.nBPP == 32) ? PVD_IDF_BGRA : PVD_IDF_BGR;
				//}
				TODO("Флаги из 2-й версии интерфейса");

				{
					//WARNING("На анимированных и многостраничных документах тут НЕ NULL");
					////_ASSERTE(*ppDrawContext == NULL);
					//if (*ppDrawContext) {
					//	// А тут именно так. На будущее. В CImage может быть несколько DrawContext, чтобы не закрыть нужный...
					//	_ASSERTE(apImage->Display->pPlugin);
					//	apImage->Display->pPlugin->DisplayClose2(*ppDrawContext);
					//	*ppDrawContext = NULL;
					//}
					
					//result = dds->Blit(&DecodeInfo, g_Plugin.BackColor);
					pvdInfoDisplayCreate2 DsCreate = {sizeof(pvdInfoDisplayCreate2)};
					#ifdef _DEBUG
						if (DecodeInfo.ColorModel == PVD_CM_UNKNOWN)
						{
							if (lstrcmpi(pFile->Decoder()->pModuleFileName, L"GDIPlus.pvd"))
							{
								_ASSERTE(DecodeInfo.ColorModel != PVD_CM_UNKNOWN);
							}
						}
					#endif
					// Некоторые декодеры НЕ возвращают палитру для индексированных форматов.
					// Нужно подставить палитру по умолчанию, а чтобы декодер не разрушил НАШУ
					// память - и делаем копию DecodeInfo
					pvdInfoDecode2 DecodeInfoCopy = DecodeInfo;
					if (DecodeInfoCopy.nBPP<=8 && DecodeInfoCopy.ColorModel!=PVD_CM_PRIVATE)
					{
						if (!DecodeInfoCopy.pPalette)
						{
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
					
					if ((DecodeInfo.Flags & PVD_IDF_PRIVATE_DISPLAY) && (pFile->Decoder()->pPlugin->nCurrentFlags & PVD_IP_DISPLAY))
					{
						lpDisplay = apImage->mp_DecoderModule;
					}
					else
					{
						lpDisplay = CPVDManager::pDefaultDisplay;
					}
					CModuleInfo* pStartDisplay = lpDisplay;
					std::vector<CModuleInfo*>::iterator iDisp = CPVDManager::Displays.begin();
					BOOL lbFirstCheckDisplay = TRUE;

					// Если g_Plugin.hWnd == 0 - значит юзер уже нажал Esc и все закрывается
					while (g_Plugin.hWnd && iDisp != CPVDManager::Displays.end())
					{
						if (!lpDisplay->pPlugin)
						{
							_ASSERTE(lpDisplay->pPlugin);
							result = FALSE;
						}
						else
						{
							if (result = lpDisplay->pPlugin->DisplayCheck())
							{
								MCHKHEAP
								if (!g_Plugin.hWnd)
								{
									result = false;
									break;
								}
								if (result = lpDisplay->pPlugin->DisplayCreate2(&DsCreate))
									break;
								wchar_t szMsg[128];
								lstrcpy(szMsg, L"Display module skipped: ");
								lstrcpy(szMsg+lstrlen(szMsg), lpDisplay->pPlugin->pName);
								CPVDManager::AddLogString(L"PVDManager", szMsg);
							}
						}

						if ((DecodeInfo.Flags & PVD_IDF_PRIVATE_DISPLAY) || CPVDManager::Displays.size()<=1)
							break;

						if (!lbFirstCheckDisplay)
							iDisp++;
						else
							lbFirstCheckDisplay = FALSE;
						// Найти следующий готовый дисплей
						while (iDisp != CPVDManager::Displays.end()
							&& ((pStartDisplay == *iDisp) 
							    || ((*iDisp)->pPlugin == NULL)
							    || ((*iDisp)->nCurrentFlags & PVD_IP_PRIVATE)))
						{
							iDisp++;
						}
						if (iDisp == CPVDManager::Displays.end())
							break;

						// Запомнить новый дисплей
						lpDisplay = *iDisp;
						wchar_t szMsg[128];
						lstrcpy(szMsg, L"Switching to display module: ");
						lstrcpy(szMsg+lstrlen(szMsg), lpDisplay->pPlugin->pName);
						CPVDManager::AddLogString(L"PVDManager", szMsg);
					}

					if (result)
					{
						(*ppDraw)->Assign(lpDisplay, DsCreate.pDisplayContext);
						//*ppDrawContext = DsCreate.pDisplayContext;
					}

						//DecodeInfo.pImage, DecodeInfo.lImagePitch, DecodeInfo.nBPP, g_Plugin.BackColor, 
						//nFlags, DecodeInfo.nTransparentColor, DecodeInfo.pPalette);
				}
				t5 = timeGetTime();
				bNeedFree = false;
				MCHKHEAP
				pFile->Decoder()->pPlugin->PageFree2(pFile->Context(), &DecodeInfo);
			}
		}
		if (!g_Plugin.hWnd || !IsWindow(g_Plugin.hWnd))
		{
			lstrcpy(szLastError, L"PicView window was closed");
			result = false;
		}
		MCHKHEAP
		if (result)
		{
			apImage->Info.lWidth = apImage->Info.lWidth;
			apImage->Info.lHeight = apImage->Info.lHeight;
			apImage->Info.nBPP = apImage->Info.nBPP;
			if (apImage->Info.Animation && !apImage->Info.lFrameTime)
				apImage->Info.lFrameTime = 100; // минимальное умолчательное время фрейма
			apImage->Info.lFrameTime = apImage->Info.lFrameTime;
		}
	}
	t6 = timeGetTime();
	apImage->Info.lTimeOpen = t2-t0; //Открытие файла изображения и получение информации о нем (Open & PageInfo)
		_ASSERTE(apImage->Info.lTimeOpen>=0); if (apImage->Info.lTimeOpen<0) apImage->Info.lTimeOpen = 0;
	apImage->Info.lTimeDecode = (t4==t1) ? 0 : (t4-t3); // PageDecode
		_ASSERTE(apImage->Info.lTimeDecode>=0); if (apImage->Info.lTimeDecode<0) apImage->Info.lTimeDecode = 0;
	apImage->Info.lTimeTransfer = ((t5==t1) ? 0 : max(0,t5-t4)) + ((t3==t1) ? 0 : max(0,t3-t2)); // CreateWorkSurface+Blit
		_ASSERTE(apImage->Info.lTimeTransfer>=0); if (apImage->Info.lTimeTransfer<0) apImage->Info.lTimeTransfer = 0;
	apImage->Info.lTimePaint = 0;
	apImage->Info.bTimePaint = FALSE;
	//wsprintf(apImage->OpenTimes, L"%i+%i+%i", 
	//	t2-t0, //Открытие файла изображения и получение информации о нем (Open & PageInfo)
	//	(t4==t1) ? 0 : t4-t3, // PageDecode
	//	((t5==t1) ? 0 : t5-t4) + ((t3==t1) ? 0 : t3-t2) // CreateWorkSurface+Blit
	//	);
	apImage->Info.lOpenTime = t6-apImage->Info.lStartOpenTime;
	//}CATCH{}

	MCHKHEAP
	OutputDebugString(result ? L"Succeeded\n" : L"Failed!!!\n");
	if (!result) {
		LPCWSTR pszError = NULL;
		if (szLastError[0])
		{
			pszError = szLastError;
		}
		else if (pFile->Decoder()->pPlugin->szLastError[0])
		{
			pszError = pFile->Decoder()->pPlugin->szLastError;
		}
		else
		{
			pszError = L"Failed!!! ";
		}
		if (pszError)
		{
			int nLen = lstrlen(szDbg);
			if ((nLen+10) < sizeofarray(szDbg))
			{
				lstrcpyn(szDbg+nLen, pszError, sizeofarray(szDbg)-nLen-1);
			}
		}
		pFile->Decoder()->pPlugin->SetStatus(szDbg);
	}

	return result;
}
#endif

bool CPVDManager::DisplayAttach()
{
	if (!CPVDManager::pActiveDisplay)
		return FALSE;
	if (!CPVDManager::pActiveDisplay->pPlugin)
		return FALSE;

	return CPVDManager::pActiveDisplay->pPlugin->DisplayAttach();
}

bool CPVDManager::DisplayDetach(BOOL abErase/*=TRUE*/)
{
	bool lbRc = false;
	if (CPVDManager::pActiveDisplay && CPVDManager::pActiveDisplay->pPlugin)
	{
		 lbRc = CPVDManager::pActiveDisplay->pPlugin->DisplayDetach();
	}
	if (abErase)
		CPVDManager::DisplayErase();
	return lbRc;
}

void CPVDManager::DisplayExit()
{
	_ASSERTE(GetCurrentThreadId() == gnDisplayThreadId || gnDisplayThreadId == 0);

	CPVDManager::DisplayDetach(FALSE);

	std::vector<CModuleInfo*>::iterator i;
	CModuleInfo *tmp;

	// Закрыть дескрипторы дисплея для всех элементов
	g_Panel.CloseDisplayHandles();

	for (i=Displays.begin(); i!=Displays.end(); i++) {
		tmp = *i;
		if (!tmp->pPlugin) continue;
		if (tmp->pPlugin->bDisplayInitialized) {
			tmp->pPlugin->DisplayExit2();
		}
	}

	if (g_Plugin.hWnd)
		DisplayErase();
}

// В максимизированном состоянии, консоль (Win7 по крайней мере)
// не заливает фоновым цветом нижнюю полоску, не занятую символами.
// http://forum.farmanager.com/viewtopic.php?p=63985#p63985
void CPVDManager::DisplayErase()
{
	if (g_Plugin.hConEmuWnd || (g_Plugin.FlagsWork & FW_QUICK_VIEW))
		return;
	if (g_Plugin.hWnd && IsWindow(g_Plugin.hWnd) && IsWindowVisible(g_Plugin.hWnd)) {
		HDC hdc = GetDC(g_Plugin.hWnd);
		RECT rc; GetClientRect(g_Plugin.hWnd, &rc);
		FillRect(hdc, &rc, (HBRUSH)GetStockObject(BLACK_BRUSH));
		ReleaseDC(g_Plugin.hWnd, hdc);
	}
	#ifdef _DEBUG
	else
	{
		_ASSERTE(g_Plugin.hWnd && IsWindow(g_Plugin.hWnd));
	}
	#endif
}

void CPVDManager::DecoderExit()
{
	// Остановить нить декодера, очистить очередь декодирования
	_ASSERTE((g_Plugin.FlagsWork & FW_TERMINATE) == FW_TERMINATE);
	g_Plugin.FlagsWork |= FW_TERMINATE;


	if (gnDecoderThreadId && GetCurrentThreadId() == gnDecoderThreadId)
	{
		// мы в нити декодера, можно закрывать дескрипторы!
		g_Panel.CloseImageHandles();
	}
	else 
	{
		// Закрывать дескрипторы нельзя - декодеры могут свалиться...
		if (g_Manager.mh_Thread)
		{
			_ASSERTE(g_Manager.mh_Request!=NULL);
			SetEvent(g_Manager.mh_Request);
			DWORD nWait = 0;
			if ((nWait = WaitForSingleObject(g_Manager.mh_Thread, 30000)) == WAIT_TIMEOUT)
			{
				TerminateThread(g_Manager.mh_Thread, 100);
			}
			CloseHandle(g_Manager.mh_Thread); g_Manager.mh_Thread = NULL;
		}
	}

	memset(g_Manager.m_DecoderQueue, 0, sizeof(g_Manager.m_DecoderQueue));
}

void CPVDManager::OnTerminate()
{
	// Остановить нить декодера, очистить очередь декодирования
	// (g_Plugin.FlagsWork == 0), если плагин ни разу не активировался
	_ASSERTE(((g_Plugin.FlagsWork & FW_TERMINATE) == FW_TERMINATE) || g_Plugin.FlagsWork == 0);
	g_Plugin.FlagsWork |= FW_TERMINATE;


	//if (GetCurrentThreadId() == g_Manager.mn_ThreadId)
	//	return; // мы и так в нити декодера!


	MCHKHEAP;
	DecoderExit();
	DisplayExit();
	MCHKHEAP;
}

void CPVDManager::OnFirstImageOpened(CImage *apImage)
{
	//if (!(g_Plugin.FlagsWork & FW_FIRST_CALL))
	//{
	//	_ASSERTE((g_Plugin.FlagsWork & FW_FIRST_CALL) == FW_FIRST_CALL);
	//	return;
	//}

	if ((g_Plugin.FlagsWork & FW_TERMINATE))
		return;

	// Если окно отрисовки еще не было показано - теперь можно показать
	// раз уж картинку смогли открыть каким-то декодером
	if (!g_Plugin.hWnd || !IsWindow(g_Plugin.hWnd))
	{
		_ASSERTE(g_Plugin.hWnd && IsWindow(g_Plugin.hWnd));
	}
	else
	{
		if (!IsWindowVisible(g_Plugin.hWnd) && !(g_Plugin.FlagsWork & FW_TERMINATE))
			ShowWindowAsync(g_Plugin.hWnd, SW_SHOWNORMAL);		
	}


	// Для первой картинки серии - в(ы)ключить (если разрешено) модификатор
	// управляющий поведением PgUp/PgDn (листают страницы или файлы)
	g_Plugin.bAutoPagingChanged = 0;
	if (!(g_Plugin.FlagsWork & FW_QUICK_VIEW) && g_Plugin.bAutoPagingSet)
	{
		BOOL bChanged = FALSE;
		SHORT CurState = GetKeyState(g_Plugin.nAutoPagingVK);
		if (!apImage->Info.Animation && apImage->Info.nPages>1) {
			// Включить ScrollLock (PgUp/PgDn листают страницы)
			if ((CurState & 1) == 0)
				bChanged = TRUE;
		} else {
			// вЫключить ScrollLock (PgUp/PgDn листают файлы)
			if ((CurState & 1) != 0)
				bChanged = TRUE;
		}
		if (bChanged) {
			g_Plugin.bAutoPagingChanged = ((CurState & 1) == 0) ? 1 : 2;
			keybd_event((BYTE)g_Plugin.nAutoPagingVK, (BYTE)MapVirtualKey(g_Plugin.nAutoPagingVK, 0), KEYEVENTF_EXTENDEDKEY, 0);
			keybd_event((BYTE)g_Plugin.nAutoPagingVK, (BYTE)MapVirtualKey(g_Plugin.nAutoPagingVK, 0), KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
		}
	}
}


/* ******************************* */
/* Обработка очереди декодирования */
/* ******************************* */

CImage* CPVDManager::GetDecodedImage(DecodeParams *apParams)
{
	CImage* pImage = GetImageFromParam(apParams);
	if (!pImage)
	{
		TODO("Показать ошибку?");
		RequestTerminate();
		return NULL;
	}

	BOOL result = IsImageReady(apParams);

	//WARNING("Может быть, что mp_File был закрыт, а дисплей - закеширован?");
	//if (pImage->mp_File->IsReady())
	//{
	//	CDisplayHandle* pDraw = pImage->GetDrawHandle(apParams);
	//	if (pDraw)
	//	{
	//		WARNING("При смене параметров/декодера/Refresh и т.п. нужно выполнить повторное декодирование");
	//		_ASSERTE(pImage->mp_File->Params.Compare(apParams));

	//		SafeRelease(pDraw);
	//		result = true;
	//	}
	//}
	
	if (!result)
	{
		apParams->Priority = eCurrentImageCurrentPage;
		apParams->Flags |= eRenderWaitOpened;

		result = RequestDecodedImage(apParams);
	}

	return result ? pImage : NULL;
}

int CPVDManager::AppendDecoderQueue(DecodeParams *apParams)
{
	HANDLE hReadyEvent = NULL;

#if 0
	// Не помню, что хотел сказать... не нужно вроде
	// добавлять в список запросов можно из любого потока
	if (GetCurrentThreadId() == mn_ThreadId)
	{
		_ASSERTE(GetCurrentThreadId() != mn_ThreadId);
		RequestTerminate();
		return -1;
	}
#endif

	EnterCriticalSection(&csDecoderQueue);

	int idx = -1;

	// Если такой запрос уже сделан - просто обновить его приоритет/флаги?
	for (int i = 0; i < sizeofarray(m_DecoderQueue); i++)
	{
		if (m_DecoderQueue[i].Status != eItemEmpty && m_DecoderQueue[i].Status != eItemFailed)
		{
			if (m_DecoderQueue[i].Params.Compare(apParams))
			{
				if (m_DecoderQueue[i].Status == eItemRequest)
				{
					idx = i;
					// Можно чуть подправить параметры
					m_DecoderQueue[i].Params.Flags = apParams->Flags;
					m_DecoderQueue[i].Params.Priority = apParams->Priority;
					LeaveCriticalSection(&csDecoderQueue);
					return idx;
				}				 
			}
		}
	}

	for (int s = eItemEmpty; (idx == -1) && (s <= eItemReady); s++)
	{
		for (int i = 0; i < sizeofarray(m_DecoderQueue); i++)
		{
			// Найти первую свободную, или менее приоритетную ячейку
			if (m_DecoderQueue[i].Status == s
				&& (s != eItemRequest
					|| apParams->Priority < m_DecoderQueue[i].Params.Priority))
			{
				idx = i; break;
			}
		}
	}

	if (idx == -1)
	{
		_ASSERTE(idx != -1);
		LeaveCriticalSection(&csDecoderQueue);
		RequestTerminate();
		return -1;
	}

	if (m_DecoderQueue[idx].Status != eItemEmpty)
	{
		TODO("Если нужно - какие-то действия по отмене элемента очереди");
	}

	if (m_DecoderQueue[idx].hReady == NULL)
		m_DecoderQueue[idx].hReady = CreateEvent(NULL, TRUE, FALSE, NULL);

	hReadyEvent = m_DecoderQueue[idx].hReady;
	ResetEvent(hReadyEvent);

	m_DecoderQueue[idx].Status = eItemRequest;

	// 'operator =' присваивает только определенный набор полей!
	m_DecoderQueue[idx].Params = *apParams;
	
	// Остальные, используемые внутри CPVDManager нужно назначить вручную!
	m_DecoderQueue[idx].Params.pResult = apParams->pResult;
	_ASSERTE(m_DecoderQueue[idx].pBuf == NULL);
	m_DecoderQueue[idx].pBuf = NULL;
	m_DecoderQueue[idx].lBuf = 0;

	SetEvent(mh_Request);

	LeaveCriticalSection(&csDecoderQueue);

	return idx;
}

bool CPVDManager::IsImageReady(DecodeParams *apParams)
{
	bool lbReady = false;
	
	// Возможно, что изображение уже декодировано с требуемыми параметрами
	CImage* pImage = GetImageFromParam(apParams);
	if (!pImage)
	{
		_ASSERTE(pImage!=NULL);
	}
	else
	{
		TODO("Проверить, с какими параметрами было декодировано изображение (качество, масштаб, область)!");
		TODO("вообще-то дескриптор дисплея нужно в нити дисплея создавать, поэтому его может еще не быть");
		WARNING("Проверку (pImage->nPage == apParams->nPage) переделать - в pImage будет храниться более одной страницы!");
		WARNING("вообще-то дескриптор дисплея mp_Draw нужно в нити дисплея создавать, поэтому его может еще не быть");

		WARNING("Может быть, что mp_File был закрыт, а дисплей - закеширован?");
		if (pImage->mp_File->IsReady() /*&& pImage->Info.nPage == apParams->nPage*/)
		{
			CDisplayHandlePtr rDraw;
			if (pImage->GetDrawHandle(rDraw, apParams))
			{
				WARNING("При смене параметров/декодера/Refresh и т.п. нужно выполнить повторное декодирование");
				_ASSERTE(pImage->mp_File->Params.Compare(apParams));

				lbReady = true;
			}
		}
	}
	
	return lbReady;
}

CImage* CPVDManager::GetImageFromParam(DecodeParams *apParams)
{
	CImage* pImage = NULL;

	// Если запрос по абсолютному индексу
	if (!(apParams->Flags & eRenderRelativeIndex))
	{
		_ASSERTE(apParams->nRawIndex!=-1); // возвращает текущий элемент, но надо ли?
		
		// Возвращает NULL, если элемент НЕ доступен (или bPicDisabled)!
		pImage = g_Panel.GetImage(apParams->nRawIndex);
		// Должен вернуть. Индекс уже должен соответвовать файлу,
		// который хотя бы по расширению подходит!
		_ASSERTE(pImage != NULL);
	}
	else //if ((apParams->Flags & eRenderRelativeIndex))
	{
		// Запрос относительно элемента (вверх или вниз)
		_ASSERTE((apParams->Flags & eRenderRelativeIndex) == eRenderRelativeIndex);
		_ASSERTE(apParams->iDirection == -1 || apParams->iDirection == 1);
		
		// Возвращает только "доступные" элементы, у которых (bPicDisabled == false)
		int nNextRawIdx = g_Panel.GetNextItemRawIdx(apParams->iDirection, apParams->nFromRawIndex);
		if (nNextRawIdx >= 0)
		{
			if ((pImage = g_Panel.GetImage(nNextRawIdx)) != NULL)
			{
				// Запомнить в ->nRawIndex - номер полученного элемента
				// В ->nFromRawIndex пока остается старое значение (от кого плясали)
				apParams->nRawIndex = nNextRawIdx;
			}
		}
	}
	
	return pImage;
}

BOOL CPVDManager::RequestDecodedImage(DecodeParams *apParams)
{
	TODO("Запрос на освобождение дескриптора");
	_ASSERTE((apParams->Flags & eRenderReleaseDescriptor)==0);

	if (!CheckDecodingThread())
	{
		// RequestTerminate() уже вызван
		return FALSE;
	}
		
	if ((apParams->Flags & eRenderRelativeIndex))
	{
		_ASSERTE(apParams->nRawIndex >= -1);
		_ASSERTE(apParams->nFromRawIndex == -1 || apParams->nFromRawIndex == 0);
		if (apParams->iDirection == 0)
		{
			_ASSERTE(apParams->iDirection == -1 || apParams->iDirection == 1);
			RequestTerminate();
			return FALSE;
		}
		if (apParams->nRawIndex == -1)
			apParams->nRawIndex = g_Panel.GetActiveRawIdx();
		apParams->nFromRawIndex = apParams->nRawIndex;
	}
	else if ((apParams->Flags & eRenderFirstAvailable))
	{
		_ASSERTE(apParams->nRawIndex >= 0);
		if (apParams->nRawIndex == -1)
			apParams->nRawIndex = g_Panel.GetActiveRawIdx();
		apParams->nFromRawIndex = apParams->nRawIndex;
	}
	
	if ((apParams->Flags & (eRenderFirstAvailable|eRenderRelativeIndex)) && apParams->iDirection == 0)
	{
		_ASSERTE((apParams->Flags & eRenderRelativeIndex)==0 || (apParams->iDirection!=0));
		apParams->Flags &= ~(eRenderFirstAvailable|eRenderRelativeIndex);
	}

	// ЭТА функция не должна извлекать файлы из архива!
	CImage* pImage = GetImageFromParam(apParams);
	if (!pImage)
	{
		//_ASSERTE(pImage != NULL);
		//RequestTerminate(); -- реально может быть NULL, когда долистали до последнего элемента, и отключен Loop
		return FALSE;
	}
	
		
	// Возможно, что изображение уже декодировано с требуемыми параметрами
	if (IsImageReady(apParams))
	{
		if (apParams->Flags & eRenderActivateOnReady)
			apParams->OnItemReady();

		return TRUE;
	}
	if (g_Plugin.FlagsWork & FW_TERMINATE)
	{
		// RequestTerminate() уже вызван
		return FALSE;
	}
	
	DecodingStatus result = eItemFailed;
	if ((apParams->Flags & eRenderWaitOpened))
	{
		apParams->pResult = &result;
	}

	int idx = AppendDecoderQueue(apParams);
	if (idx < 0)
	{
		_ASSERTE(idx >= 0);
		// RequestTerminate() уже вызван
		return FALSE;
	}

	BOOL lbRc = TRUE;

	if ((apParams->Flags & eRenderWaitOpened))
	{
		TODO("Предусмотреть возможность отмены ожидания / останова декодирования");
		HANDLE hReady = m_DecoderQueue[idx].hReady;
		DWORD nWait = WaitForSingleObject(hReady, 250);
		while (nWait == WAIT_TIMEOUT)
		{
			if ((g_Plugin.FlagsWork & FW_TERMINATE))
				break;
			nWait = WaitForSingleObject(hReady, 250);
		}
		// Проверяем результат
		lbRc = (nWait == WAIT_OBJECT_0) && (result == eItemReady);
			///*&& (m_DecoderQueue[idx].nRawPanelItem == aiRawIndex)*/
			//&& (m_DecoderQueue[idx].Status == eItemReady)
			//&& (m_DecoderQueue[idx].Params.nPage == apParams->nPage);
		//// И сразу пометить ячейку как не используемую
		//m_DecoderQueue[idx].Status = eItemEmpty;
	}

	return lbRc;
}

DWORD CPVDManager::DecodingThread(LPVOID)
{
	int iRc = 0;
	BOOL lbStepRc = FALSE, lbException = FALSE;
	
	gnDecoderThreadId = GetCurrentThreadId(); // Информационно

	while (!(g_Plugin.FlagsWork & FW_TERMINATE))
	{
		if (g_Manager.mh_Alive) SetEvent(g_Manager.mh_Alive);
		g_Manager.mn_AliveTick = GetTickCount();
		WaitForSingleObject(g_Manager.mh_Request, 100);

		if ((g_Plugin.FlagsWork & FW_TERMINATE))
			break; // завершение плагина

		TRY
		{
			lbStepRc = g_Manager.DecodingThreadStep();
		}
		CATCH
		{
			lbException = TRUE;
		}

		if (lbException)
		{
			iRc = 100;
			CFunctionLogger::FunctionLogger(L"!!! Exception in CPVDManager::DecodingThread");
			RequestTerminate();
			MessageBox(NULL, L"!!! Exception in CPVDManager::DecodingThread\nPlease, restart FAR", L"PictureView2", MB_OK|MB_ICONSTOP|MB_SETFOREGROUND|MB_SYSTEMMODAL);
			break;
		}

		if (!lbStepRc)
		{
			// RequestTerminate уже должен быть вызван
			iRc = 100;
			break;
		}
	}

	// Закрытие декодеров - должно выполняться в ЭТОЙ нити!
	CPVDManager::DecoderExit();
	
	// Флаг того, что нить нормально завершилась
	gnDecoderThreadId = 0;
	
	return iRc;
}

BOOL CPVDManager::DecodingThreadStep()
{
	int idx = -1;
	EnterCriticalSection(&g_Manager.csDecoderQueue);
	
	int iActiveRawIndex = g_Panel.GetActiveRawIdx();

	for (int p = eCurrentImageCurrentPage; (idx == -1) && (p <= eDecodeAny); p++)
	{
		for (int i = 0; i < sizeofarray(m_DecoderQueue); i++)
		{
			if (p <= eCurrentImageNextPage && iActiveRawIndex != m_DecoderQueue[i].Params.nRawIndex)
				continue; // это уже НЕ текущее изображение
			
			if (m_DecoderQueue[i].Status == eItemRequest
				&& (m_DecoderQueue[i].Params.Priority == p || p == eDecodeAny))
			{
				idx = i; break;
			}
		}
	}

	// теперь можно секцию отпустить, и начать собственно декодирование
	LeaveCriticalSection(&csDecoderQueue); 

	// если есть какие-то запросы
	if (idx == -1)
		return TRUE; // продолжаем обработку в нити!

	TODO("Запрос на освобождение дескриптора");
	_ASSERTE((m_DecoderQueue[idx].Params.Flags & eRenderReleaseDescriptor)==0);

	BOOL lbRc = TRUE;
	bool lbDecodeResult = false;

	CImage* pImage = GetImageFromParam(&m_DecoderQueue[idx].Params);
lTryAgain:
	if (!pImage)
	{
		_ASSERTE(pImage != NULL);
		m_DecoderQueue[idx].Status = eItemFailed;
		SetEvent(m_DecoderQueue[idx].hReady);
		RequestTerminate();
		lbRc = FALSE; // завершение нити
	}
	else
	{

		// Первое открытие файла, или нужен полный цикл подбора декодера.
		if (m_DecoderQueue[idx].Params.Flags & eRenderResetDecoder)
		{
			pImage->Close();
			//pImage->ResetProcessed(); ==> ResetDecoderFailed()
			pImage->ResetDecoderFailed();
		}
		else
		{
			// Изображение уже могло быть декодировано с требуемыми параметрами
			if (IsImageReady(&m_DecoderQueue[idx].Params))
				lbDecodeResult = true;
		}

		// Если требуется декодирование (это не было сделано ранее)
		if (!lbDecodeResult)
		{
			CFunctionLogger::FunctionLogger(L"DecodingThread->ImageOpen(%s)",(LPCWSTR)pImage->FileName);
			lbDecodeResult = ImageOpen(pImage, &m_DecoderQueue[idx].Params);
			//CImage* pRc = g_Manager.GetDecodedImage(&m_DecoderQueue[idx].Params);
			//lbDecodeResult = (pRc != NULL);
			CFunctionLogger::FunctionLogger(L"ImageOpen done");

			if (!lbDecodeResult)
				g_Panel.SetItemDisabled(pImage->PanelItemRaw());
			
			if (!lbDecodeResult && (m_DecoderQueue[idx].Params.Flags & eRenderFirstAvailable))
			{
				// Обновить индекс, "от кого плясать"
				m_DecoderQueue[idx].Params.nFromRawIndex = m_DecoderQueue[idx].Params.nRawIndex;
				// Установить флаг "относительно индекса"
				m_DecoderQueue[idx].Params.Flags |= eRenderRelativeIndex;
				// И проверить "направление", если пусто - ставим "вниз"
				if (m_DecoderQueue[idx].Params.iDirection == 0) m_DecoderQueue[idx].Params.iDirection = 1;
				
				// Пробуем получить ссылку на следующий файл (CImage)
				if ((pImage = GetImageFromParam(&m_DecoderQueue[idx].Params)) != NULL)
					goto lTryAgain;

				// Значит не судьба, lbDecodeResult останется false
			}
		}

		TODO("Дернуть нить дисплея, чтобы он (в фоне) перенес данные из декодера в дисплей?");
		TODO("Или лучше, поместить в некий стек, который будет обрабатываться при простое");
		if (lbDecodeResult /*&& (m_DecoderQueue[idx].Params.Flags & eRenderActivateOnReady)*/)
		{
			m_DecoderQueue[idx].Params.OnItemReady();
		}
		else
		{
			SafeRelease(m_DecoderQueue[idx].Params.pDecodeItem,NULL); // Кто тут должен быть?
		}
		
		if (m_DecoderQueue[idx].Params.pResult && !(g_Plugin.FlagsWork & FW_TERMINATE))
			*m_DecoderQueue[idx].Params.pResult = lbDecodeResult ? eItemReady : eItemFailed;

		m_DecoderQueue[idx].Status = lbDecodeResult ? eItemReady : eItemFailed;
		SetEvent(m_DecoderQueue[idx].hReady);

		lbRc = TRUE; // продолжаем обработку в нити!
	}

	return lbRc;
}

bool CPVDManager::CheckDecodingThread()
{
	DWORD nWait;

	if (mh_Thread)
	{
		nWait = WaitForSingleObject(mh_Thread, 0);
		if (nWait == WAIT_OBJECT_0)
		{
			CloseHandle(mh_Thread); // нить была завершена!
			mh_Thread = NULL;
		}
	}

	if (!mh_Request) mh_Request = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (!mh_Alive) mh_Alive = CreateEvent(NULL, TRUE, FALSE, NULL);

	ResetEvent(mh_Alive);

	if (mh_Thread == NULL)
	{
		mh_Thread = CreateThread(NULL, 0, DecodingThread, NULL, 0, &mn_ThreadId);
		if (!mh_Thread)
		{
			_ASSERTE(mh_Thread != NULL || (g_Plugin.FlagsWork & FW_TERMINATE));
			RequestTerminate();
			return false;
		}

		HANDLE hEvents[2] = {mh_Alive,mh_Thread};
		nWait = WaitForMultipleObjects(2, hEvents, FALSE, 10000);
		if (nWait != WAIT_OBJECT_0)
		{
			_ASSERTE(nWait == WAIT_OBJECT_0 || (g_Plugin.FlagsWork & FW_TERMINATE));
			RequestTerminate();
			return false;
		}
	} else {
		TODO("Проверить, не повисла ли нить декодирования");
	}

	return TRUE;
}

bool CPVDManager::ImageOpen(CImage* apImage, DecodeParams *apParams)
{
	_ASSERTE(apImage && !apImage->FileName.IsEmpty() && apParams);

	//// Должен быть уже вызван, но проверим
	//g_Plugin.InitCMYK(FALSE); // Дождаться его завершения

	i64 lFileSize;
	const unsigned char *buf = NULL; int lBuf = 0;
	FileMap Map((LPCWSTR)apImage->FileName);
	lFileSize = Map.lSize;
	if (Map.MapView())
	{
		buf = Map.pMapping;
		lBuf = (int)Min<i64>(Map.lSize, ~0u);
		//if (lBuf) -- IN_APL этого не пережил
		//	lFileSize = 0; // все данные уже в памяти
	}


	bool result = false;

	TODO("Перенести в начало функции");
	apImage->Info.lStartOpenTime = timeGetTime();

	if (apParams->pDecodeItem == NULL)
	{
		apParams->pDecodeItem = new CDecodeItem(szPVDManager, apImage);
		apParams->pDecodeItem->Params = *apParams;
	}

	// Сбрасывать нельзя! Мы сейчас можем находится в вызове AltPgUp например (смена декодера)
	//Decoder->mi_DecoderIndex = *piSubDecoder;
	
lReOpen2:
	//memset(&InfoPage, 0, sizeof(InfoPage)); // !!! cbSize не заполнять
	result = Open(apImage, &apParams->pDecodeItem->pFile, (LPCWSTR)apImage->FileName, lFileSize, buf, lBuf, apParams);

	if (!result || (g_Plugin.FlagsWork & FW_TERMINATE))
	{
		return false;
	}

	// Для первой картинки серии - в(ы)ключить (если разрешено) модификатор
	// управляющий поведением PgUp/PgDn (листают страницы или файлы)
	if (apParams->Flags & eRenderFirstImage)
	{
		OnFirstImageOpened(apImage);
		if ((g_Plugin.FlagsWork & FW_TERMINATE))
			result = false;
	}
		

	if (result)
	{
		// А не уже?
		TODO("Хорошо бы разрулить ImageDecode и запуск нити Display. Собственно декодирование можно начать еще ДО создания DX и прочего");

		//if (!ImageDecode())
		//if (!Decode(apImage, apImage->mp_File, &apImage->mp_Draw, false/*abResetTick*/))
		
		WARNING("Переделать, поддержка страниц");
		//CDecodeItem img(apImage);

		TODO("abResetTick=true - При декодировании следующей страницы сбросить тик (там сейчас время открытия)");
		if (!DecodePixels(apParams->pDecodeItem, false/*abResetTick*/))
		{
			WARNING("Если ImageDecode обламывается - то мы зацикливаемся, т.к. ImageOpen выбирает тот же декодер!");
			WARNING("В любом случае нужно оптимизировать окрытие? Не разрушать pPVDDecoder и его локальную переменную bProcessed");
			uint nDecoders = (uint)CPVDManager::Decoders.size();
			if (nDecoders <= 1)
				return false;

			if (g_Plugin.FlagsWork & FW_TERMINATE)
				return false; // Плагин закрывается

			// Пометить, что не удалось декодировать, или перенести данные в дисплей
			apImage->SetDecoderFailed(apImage->GetDecoder());
			//apImage->SetDecoderFailed(apImage->mi_DecoderIndex);

			//apImage->mi_DecoderIndex++;
			//if (apImage->mi_DecoderIndex >= nDecoders)
			//	apImage->mi_DecoderIndex = 0;

			goto lReOpen2;
		}
		//else
		//{
		//	// Успешное декодирование. Сброс флагов "Декодер проверен для этого изображения".
		// Низя!
		// возможен вариант, когда результат декодирования не смог показать ни один модуль дисплея,
		// в этом случае необходимо ПРОДОЛЖИТЬ перебор декодеров, пока они не кончатся,
		// или пока не отработает какой-то модуль дисплея.
		//	g_Manager.ResetProcessed(this);
		//}
	}
	
	return result;
}

// 1) Выполнить перенос данных из контекста декодера в контекст дисплея
// 2) Если нужно - передернуть окно отрисовки
bool CPVDManager::OnItemReady(CDecodeItem* apItem)
{
	if (GetCurrentThreadId() != gnDisplayThreadId)
	{
		_ASSERTE(GetCurrentThreadId() == gnDisplayThreadId);
		return false;
	}

	if (!apItem || !apItem->pImage)
	{
		_ASSERTE(apItem!=NULL && apItem->pImage!=NULL);
		return false;
	}

	// Собственно перенос данных из декодера в дисплей
	if (PixelsToDisplay(apItem))
	{
		apItem->pImage->StoreDecodedFrame(apItem);
	}

	if (!apItem->pDraw || !apItem->pDraw->IsReady())
	{
		_ASSERTE(apItem->pDraw && apItem->pDraw->IsReady());
		return false;
	}

	if ((apItem->Params.Flags & eRenderFirstImage))
	{
		if (!IsWindowVisible(g_Plugin.hWnd))
			ShowWindow(g_Plugin.hWnd, SW_SHOWNORMAL);
		g_Plugin.FlagsDisplay |= FD_TITLE_REPAINT;
		InvalidateRect(g_Plugin.hWnd, NULL, FALSE);
	}

	return true;
}
