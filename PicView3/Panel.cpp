
/**************************************************************************
Copyright (c) 2011 Maximus5
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


#include "PictureView.h"
#include "PictureView_Lang.h"
#include "PictureView_FileName.h"
#include "Panel.h"
#include "Image.h"
#include "PVDManager.h"
#include "headers/FarHelper.h"

CPicViewPanel g_Panel;
LPCSTR szPicViewPanel = "CPicViewPanel";

bool PutFilePathName(CUnicodeFileName* pFullName, const wchar_t* pFileName, bool abIsFolder);

static struct PluginPanelItem sInvalidItem = {{0}};
const struct PluginPanelItem* CPicViewPanel::Invalid = &sInvalidItem;

#define PREV_RAW_IDX 0
#define NEXT_RAW_IDX 1

CPicViewPanel::CPicViewPanel()
{
#ifdef MHEAP_DEFINED
	xf_initialize();
#endif
	// Work mode
	mb_PanelExists = mb_PluginMode = mb_RealFilesMode = false;
	// Items
	mn_ItemsCount = mn_ReadyItemsCount = 0;
	mn_ActiveRawIndex = mn_DecodeRawIndex = -1;
	mn_LastOpenedRawIndex[PREV_RAW_IDX] = mn_LastOpenedRawIndex[NEXT_RAW_IDX] = -1;
	mpvi_Items = NULL;
	// Panel (or directory)
	memset(&m_Panel, 0, sizeof(m_Panel));
	ms_PanelDir = NULL;
	ms_ExtractMacroText = NULL;
	mn_LoadItemFromPlugin = -1;
	mh_LoadItemFromPlugin = NULL;
}

void CPicViewPanel::EraseTempDir()
{
	if (mpvi_Items)
	{
		for (int i = 0; i < mn_ItemsCount; i++)
		{
			if (mpvi_Items[i].sTempFile)
			{
				if (mpvi_Items[i].bTempCopy)
					DeleteFile(*mpvi_Items[i].sTempFile);
				delete mpvi_Items[i].sTempFile;
				mpvi_Items[i].sTempFile = NULL;
			}
		}
	}
	
	if (!ms_TempDir.IsEmpty())
	{
		TODO("Проверить, на слеш в конце ругаться не будет?");
		if (RemoveDirectory((LPCWSTR)ms_TempDir))
			ms_TempDir = NULL;
	}
}

void CPicViewPanel::FreeAllItems()
{
	// Сначала - почистим Temp
	EraseTempDir();
	
	if (mpvi_Items)
	{
		for (int i = 0; i < mn_ItemsCount; i++) {
			SafeFree(mpvi_Items[i].pItem);
			if (mpvi_Items[i].sFile)
				delete mpvi_Items[i].sFile;
			//if (mpvi_Items[i].sTempFile) -- это уже
			//{
			//	if (mpvi_Items[i].bTempCopy)
			//		DeleteFile(*mpvi_Items[i].sTempFile);
			//	delete mpvi_Items[i].sTempFile;
			//}
			if (mpvi_Items[i].pImage)
			{
				if (mpvi_Items[i].pImage->Release(szPicViewPanel))
				{
					_ASSERTE(mpvi_Items[i].pImage==NULL);
					// Звать Release(), пока не освободится
					TODO("Теоретически, элемент может быть занят другими нитями, требуется, чтобы FreeAllItems вызывался в самом конце, перед выходом из плагина");
					while (mpvi_Items[i].pImage->Release(NULL))
						;
				}
				mpvi_Items[i].pImage = NULL;
			}
		}
	}
	SafeFree(mpvi_Items);
	mn_ItemsCount = mn_ReadyItemsCount = 0;
	SafeFree(ms_ExtractMacroText);
	
	if (mh_LoadItemFromPlugin)
	{
		CloseHandle(mh_LoadItemFromPlugin);
		mh_LoadItemFromPlugin = NULL;
	}
}

CPicViewPanel::~CPicViewPanel()
{
	FreeAllItems();
}

void CPicViewPanel::ShowError(const wchar_t* pszMsg)
{
	_ASSERTE(pszMsg!=NULL);
	if (!pszMsg) pszMsg = L"";
	
	if (GetCurrentThreadId() == gnMainThreadId)
	{
		const wchar_t* szLines[3];
		szLines[0] = g_Plugin.pszPluginTitle;
		szLines[1] = pszMsg;
		szLines[2] = L"Source: CPicViewPanel";
		g_StartupInfo.Message(PluginNumberMsg, FMSG_WARNING|FMSG_MB_OK,
			NULL, szLines, 3, 0);
	} else {
		MessageBox(NULL, pszMsg, g_Plugin.pszPluginTitle, MB_SETFOREGROUND|MB_SYSTEMMODAL|MB_OK|MB_ICONSTOP);
	}
}

bool CPicViewPanel::IsInitialized()
{
	return (mpvi_Items != NULL);
}

// OpenFrom
//		0 - НЕявный вызов из панелей (перехват "входа в файл" по Enter/CtrlPgDn БЕЗ использования ассоциаций)
//		OPEN_BY_USER - ЯВНЫЙ вызов из панелей. Это может быть префикс или F11. Возможно, из архива!
//		OPEN_VIEWER - пойман VE_READ, сюда можно попадать только для первой картинки серии
//		(OPEN_VIEWER|OPEN_BY_USER) - F11 из вьювера
//		OPEN_EDITOR - пойман EE_READ, сюда можно попадать только для первой картинки серии
//		(OPEN_EDITOR|OPEN_BY_USER) - F11 из редактора
// asFile
//		Однозначтно заполняется только если (OpenFrom == 0)
//		Иначе - может быть пуст. Функция сама должна получить имя файла.
bool CPicViewPanel::Initialize(int OpenFrom, LPCWSTR asFile)
{
	/*
	enum OPENPLUGIN_OPENFROM{
	OPEN_DISKMENU     = 0,
	OPEN_PLUGINSMENU  = 1,
	OPEN_FINDLIST     = 2,
	OPEN_SHORTCUT     = 3,
	OPEN_COMMANDLINE  = 4,
	OPEN_EDITOR       = 5,
	OPEN_VIEWER       = 6,
	OPEN_FILEPANEL    = 7,
	OPEN_DIALOG       = 8,
	OPEN_ANALYSE      = 9,
	OPEN_FROMMACRO    = 0x10000,
	};
	*/
	WARNING("Добавить инициализацию из View/Edit/QView");
	WARNING("Учесть, что если View - это может быть ПАНЕЛЬ QView");

	MCHKHEAP;

	ms_PanelDir = NULL;
	//ms_TempDir = NULL; -- нельзя. папка могла быть создана в предыдущий запуск плагина
	//ms_PanelFile = NULL;
	_ASSERTE(mpvi_Items == NULL);
	memset(&m_Panel, 0, sizeof(m_Panel));
	mn_LoadItemFromPlugin = -1;
	
	SafeFree(ms_ExtractMacroText);

//#ifdef _DEBUG
//	ms_PanelFile.Validate();
//#endif

	CUnicodeFileName lsOpenFile, lsPanelFile;
	//LPCWSTR pszPanelFileName = NULL;
	int iCurrent = -1;

	if (asFile && *asFile)
	{
		if (wcschr((LPCWSTR)asFile, L'\\') == NULL)
		{
			ShowError(L"asFile does not contains path in CPicViewPanel::Initialize");
			return false;
		}
		TODO("Для папок тут слеш на конце уже гарантирован?");
		lsOpenFile = asFile;
	}

	WARNING("Если плаг открыт из панели поиска через ViewHook - необходимо заблокировать переходы и извлечение из архивов!");

	if (!g_StartupInfo.PanelControl(INVALID_HANDLE_VALUE, FCTL_CHECKPANELSEXIST, 0, NULL))
	{
		// Панелей вообще нет - это может быть только активация из
		// редактора/вьювера, когда фар запущен как 'far.exe /e <file>'
		_ASSERTE(asFile && *asFile);
		mb_PanelExists = false;
		mb_PluginMode = false;
		mb_RealFilesMode = true; // инициализация из файловой системы
		memset(&m_Panel, 0, sizeof(m_Panel));
		//ms_PanelFile = asFile; -- lsOpenFile - уже
	}
	else
	{
		mb_PanelExists = true;

		memset(&m_Panel, 0, sizeof(m_Panel));
		#ifdef FAR_UNICODE
		m_Panel.StructSize = sizeof(m_Panel);
		#endif

		if (!g_StartupInfo.PanelControl(INVALID_HANDLE_VALUE, FCTL_GETPANELINFO, 0, (DLG_LPARAM)&m_Panel))
		{
			ShowError(L"FCTL_GETPANELINFO failed");
			return false;
		}

		mb_PluginMode = IsPluginPanel(m_Panel);
		mb_RealFilesMode = IsPluginPanel(m_Panel) || IsRealFileNames(m_Panel);
		iCurrent = m_Panel.CurrentItem; // сразу запомнить
		
		if (m_Panel.ItemsNumber > 0)
		{
			PluginPanelItem* ppi = LoadPanelItem(m_Panel.CurrentItem);
			if (ppi)
			{
				//#ifdef _DEBUG
				//ms_PanelFile.Validate();
				//#endif

				bool lbFileOk = false;
				if (mb_RealFilesMode)
				{
					lbFileOk = PutFilePathName(&lsPanelFile, PanelItemFileNamePtr(*ppi), (PanelItemAttributes(*ppi) & FILE_ATTRIBUTE_DIRECTORY) != 0);
				}
				else if ((PanelItemAttributes(*ppi) & FILE_ATTRIBUTE_DIRECTORY) != 0)
				{
					// Вызов для папок в плагинах - не допускается
					return false;
				}
				else
				{
					// Для плагиновых панелей - интересует только имя файла
					int nLen = lstrlen(PanelItemFileNamePtr(*ppi));
					if (nLen > 0)
					{
						LPCWSTR pszName = ItemPointToName(PanelItemFileNamePtr(*ppi));
						if (pszName)
						{
							lstrcpy(lsPanelFile.GetBuffer(nLen+1), pszName);
							lbFileOk = true;
						}
					}
				}

				//#ifdef _DEBUG
				//ms_PanelFile.Validate();
				//#endif

				free(ppi);
				if (!lbFileOk)
				{
					ShowError(L"PutFilePathName failed in CPicViewPanel::Initialize");
					return false;
				}
				//if (!asFile || !*asFile)
				//	lsOpenFile = (LPCWSTR)ms_PanelFile;
			} else {
				// Ошибка уже показана в LoadPanelItem
				return false;
			}
		} else {
			ShowError(L"Panel is empty in CPicViewPanel::Initialize");
			return false;
		}
	}
	
	// смотреть на (pi.Flags & PFLAGS_REALNAMES)
	if (mb_PanelExists && mb_RealFilesMode)
	{
		#if FARMANAGERVERSION_BUILD>=2343
		FarPanelDirectory fpd = {sizeof(fpd)};
		if (size_t dir = g_StartupInfo.PanelControl(INVALID_HANDLE_VALUE, FCTL_GETPANELDIRECTORY, sizeof(fpd), &fpd))
		{
			ms_PanelDir.Assign(fpd.Name, true);
		}
		#else
		if (size_t len = g_StartupInfo.PanelControl(INVALID_HANDLE_VALUE, FCTL_GETPANELDIR, 0, 0))
		{
			if (wchar_t* pCurDir = (wchar_t*)malloc(len*2+4))
			{
				if (!g_StartupInfo.PanelControl(INVALID_HANDLE_VALUE, FCTL_GETPANELDIR, (int)len, (DLG_LPARAM)pCurDir))
				{
					*pCurDir = 0;
					mb_PanelExists = FALSE;
				} else {
					len = lstrlen(pCurDir);
					// Для строгости - ms_PanelDir должен содержать EndSlash
					if (len && pCurDir[len-1] != L'\\')
					{
						pCurDir[len++] = L'\\'; pCurDir[len] = 0;
					}
				}
				ms_PanelDir = pCurDir;
				free(pCurDir);
			}
			else
			{
				ShowError(L"Can't allocate pCurDir");
				return false;
			}
		}
		#endif
		else
		{
			_ASSERTE("GetPanelDirectory failed" && 0);
			mb_PanelExists = false;
		}
	}
	else
	{
		_ASSERTE(mb_PluginMode || !mb_PanelExists);
		ms_PanelDir = L""; // плагиновая панель, текущая папка не интересует
	}

	if (mb_PanelExists && asFile && *asFile)
	{
		bool bPanelOk = true;
		
		// Теперь проверяем, имеет ли панель отношение к открываемому файлу
		if (mb_RealFilesMode)
		{
			// Открываемый файл должен совпадать с текущим файлом на панели
			if (lstrcmpiW((LPCWSTR)lsOpenFile, (LPCWSTR)lsPanelFile) != 0)
			{
				// Текущий файл на панели не совпадает с открываемым файлом!
				// Но может быть совпадает папка, а файл просто не текущий под курсором?
				if (!lsOpenFile.CompareDir((LPCWSTR)ms_PanelDir))
				{
					bPanelOk = false;
				}
			}
		} else {
			// Для плагиновых панелей - сравниваем только имя файла (т.к. asFile - путь в %TEMP%)
			bPanelOk = false;
			LPCWSTR pszName1 = ItemPointToName((LPCWSTR)lsOpenFile);
			LPCWSTR pszName2 = ItemPointToName((LPCWSTR)lsPanelFile);
			if (pszName1 && pszName2)
			{
				bPanelOk = (lstrcmpiW(pszName1, pszName2) == 0);
			}
		}
		
		if (!bPanelOk)
		{
			mb_PanelExists = false;
			mb_PluginMode = false;
			mb_RealFilesMode = true; // инициализация из файловой системы
		}
	}

	if (!mb_PanelExists)
	{
		ms_PanelDir = (LPCWSTR)lsOpenFile;
		wchar_t* pszBuf = ms_PanelDir.GetBuffer(0);
		wchar_t* pszSlash = (wchar_t*)ItemPointToName(pszBuf);
		if (!pszSlash)
		{
			_ASSERTE(pszSlash != NULL);
			return false;
		}
		*pszSlash = 0; // оставляем только родительскую папку
	}

	// В любом случае - активный файл в плагине - тот, для которого вызвали PicView (он может не быть текущим на панели)
	//ms_PanelFile = lsOpenFile;

	m_Panel.CurrentItem = -1; // Индекс должен совпадать с открываемым файлом!
	mn_ActiveRawIndex = mn_DecodeRawIndex = -1;
	mn_LastOpenedRawIndex[PREV_RAW_IDX] = mn_LastOpenedRawIndex[NEXT_RAW_IDX] = -1;
	
	mn_ReadyItemsCount = 0;
	_ASSERTE(mn_ItemsCount==0 && mpvi_Items==NULL);
	mn_ItemsCount = 0;

	// Теперь, наконец, загружаем список файлов из панели или из ФС
	if (mb_PanelExists)
	{
		if (!InitListFromPanel())
			return false;
	} else {
		if (!InitListFromFileSystem())
			return false;
	}
	
	_ASSERTE(mn_ActiveRawIndex == -1); // еще не должен инициализироваться
	
	if (asFile && *asFile)
	{
		_ASSERTE(mn_ActiveRawIndex == -1);
		LPCWSTR pszOpenName = ItemPointToName((LPCWSTR)lsOpenFile);
		if (!pszOpenName)
		{
			_ASSERTE(pszOpenName);
			return false;
		}
		int nCmp = 0;
	
		// Сначала сравним с iCurrent
		if (mb_PanelExists && iCurrent >= 0 && iCurrent < mn_ItemsCount && mpvi_Items[iCurrent].pItem)
		{
			if (mb_RealFilesMode)
				nCmp = lstrcmpiW((LPCWSTR)*mpvi_Items[iCurrent].sFile, (LPCWSTR)lsOpenFile);
			else
				nCmp = lstrcmpiW(PanelItemFileNamePtr(*mpvi_Items[iCurrent].pItem), pszOpenName);
			if (nCmp == 0)
			{
				// Активный
				mpvi_Items[iCurrent].bPicDetected = TRUE;
				mn_ActiveRawIndex = m_Panel.CurrentItem = iCurrent;
			}
		}
		// Если не совпадает - перебрать все элементы
		if (mn_ActiveRawIndex == -1)
		{
			for (int i = 0; i < mn_ItemsCount; i++)
			{
				if (!mpvi_Items[i].pItem || !mpvi_Items[i].sFile)
					continue;
				// Сравниваем
				if (mb_RealFilesMode)
					nCmp = lstrcmpiW((LPCWSTR)*mpvi_Items[i].sFile, (LPCWSTR)lsOpenFile);
				else
					nCmp = lstrcmpiW(PanelItemFileNamePtr(*mpvi_Items[i].pItem), pszOpenName);
				if (nCmp == 0)
				{
					// Активный
					mpvi_Items[i].bPicDetected = TRUE;
					mn_ActiveRawIndex = m_Panel.CurrentItem = i;
					break;
				}
			}
		}
	} else {
		// Открываемый файл не передали, т.е. открываем текущий на панели!
		if (!mb_PanelExists
			|| (iCurrent < 0 || iCurrent >= mn_ItemsCount)
			|| (!mpvi_Items[iCurrent].pItem))
		{
			_ASSERTE(mb_PanelExists);
			_ASSERTE(iCurrent >= 0 && iCurrent < mn_ItemsCount);
			_ASSERTE(mpvi_Items[iCurrent].pItem != NULL);
			return false;
		}
		// Активный
		mpvi_Items[iCurrent].bPicDetected = TRUE;
		mn_ActiveRawIndex = m_Panel.CurrentItem = iCurrent;
	}
	
	if (mn_ActiveRawIndex < 0)
	{
		_ASSERTE(mn_ActiveRawIndex >= 0);
		return false;
	}

	if (!mb_RealFilesMode && asFile && *asFile)
	{
		mpvi_Items[mn_ActiveRawIndex].sTempFile = new CUnicodeFileName;
		mpvi_Items[mn_ActiveRawIndex].sTempFile->Assign((LPCWSTR)lsOpenFile, false);
	}
	
	// Если мы в плагине - нужно загрузить текст макроса ms_ExtractMacroText
	if (!mb_RealFilesMode)
	{
		SafeFree(ms_ExtractMacroText);
		if (!InitExtractMacro())
		{
			// Ошибка в макросе. Уже показана.
			return false;
		}
		// Если не удалось загрузить текст макроса (ms_ExtractMacroText == NULL)
		// блокируем возможность листания - это ниже, в RescanSupported
	}

	
	//// Сначала сравним с iCurrent
	//LPCWSTR pszFileNameOnly = ItemPointToName(
	//if (iCurrent >= 0 && iCurrent < mn_ItemsCount)
	//{
	//	if (mpvi_Items[iCurrent].pItem)
	//	{
	//		if (lstrcmpiW((LPCWSTR)ms_PanelFile, (LPCWSTR)*(mpvi_Items[mn_ItemsCount].sFile)) == 0)
	//		{
	//			mpvi_Items[mn_ItemsCount].bPicDetected = TRUE;
	//			mn_ActiveRawIndex = m_Panel.CurrentItem = mn_ItemsCount;
	//		}
	//		else
	//		{
	//		}
	//	}
	//}
	
	//if (m_Panel.CurrentItem == -1 && lstrcmpiW((LPCWSTR)ms_PanelFile, (LPCWSTR)*(mpvi_Items[mn_ItemsCount].sFile)) == 0)
	//{
	//	mpvi_Items[mn_ItemsCount].bPicDetected = TRUE;
	//	mn_ActiveRawIndex = m_Panel.CurrentItem = mn_ItemsCount;
	//}
	
	
	_ASSERTE(mn_ActiveRawIndex == m_Panel.CurrentItem);
	//if (m_Panel.CurrentItem == -1)
	//{
	//	_ASSERTE(m_Panel.CurrentItem > 0);
	//	FILETIME ft = {0,0};
	//	AddItem((LPCWSTR)ms_PanelFile, 0, 0, ft);
	//	if (mn_ItemsCount == 0)
	//	{
	//		_ASSERTE(mn_ItemsCount > 0);
	//		return false;
	//	}
	//	mn_ActiveRawIndex = m_Panel.CurrentItem = (mn_ItemsCount - 1);
	//}
	//if (mpvi_Items[m_Panel.CurrentItem].bPicDetected)
	//{
	//	_ASSERTE(mpvi_Items[m_Panel.CurrentItem].bPicDetected);
	//	mpvi_Items[m_Panel.CurrentItem].bPicDetected = TRUE;
	//}
	//_ASSERTE(mn_ActiveRawIndex == m_Panel.CurrentItem);
	
	
	if (ms_TempDir.IsEmpty())
	{
		wchar_t sTemp[MAX_PATH*2+2]; sTemp[0] = 0;
		g_StartupInfo.FSF->MkTemp(sTemp, sizeofarray(sTemp)-2, L"PiC");
		if (sTemp[0] && CreateDirectory(sTemp, NULL))
		{
			int nLen = lstrlen(sTemp);
			if (sTemp[nLen-1] != L'\\')
			{
				sTemp[nLen++] = L'\\'; sTemp[nLen] = 0;
			}
			ms_TempDir = sTemp;
		}
	} else {
		CreateDirectory(ms_TempDir, NULL); // на всякий случай
	}

	if (!mb_RealFilesMode)
	{
		if (ms_TempDir.IsEmpty())
		{
			SafeFree(ms_ExtractMacroText); // Листание - запретить. Не удалось создать %Temp%.
		}
		else if (ms_ExtractMacroText && asFile && *asFile)
		{
			// Копируем в %TEMP% сразу, ибо декодеры могут завязаться на файл,
			// а фар его может удалить, при перелистывании на следущий файл архива
			if (!CacheFile(mn_ActiveRawIndex, asFile))
			{
				ms_TempDir = NULL;
				SafeFree(ms_ExtractMacroText); // Листание - запретить. Не удалось скопировать в %Temp%.
			}
		}
	}
	

	// Проверить, какие элементы мы можем попытаться открыть	
	RescanSupported();

	CImage* pImage = GetImage(mn_ActiveRawIndex);
	if (!pImage)
		return false;
	//pImage->FileName = (LPCWSTR)ms_PanelFile;
	//g_Plugin.MapFileName = (LPCWSTR)pImage->FileName;











	// Final. returns true after dump.
	
	#ifdef _DEBUG
	// Сделать Dump элементов, которые загрузили
	wchar_t szInfo[1024];
	wsprintf(szInfo, L"PanelExists=%i, PluginMode=%i, RealFilesMode=%i\nItemsCount=%i, ReadyItemsCount=%i, ActiveRawIndex=%i\n",
		(int)mb_PanelExists, (int)mb_PluginMode, (int)mb_RealFilesMode, mn_ItemsCount, mn_ReadyItemsCount, mn_ActiveRawIndex);
	OutputDebugString(szInfo);
	OutputDebugString(L"CurDir:   "); OutputDebugString((LPCWSTR)ms_PanelDir); OutputDebugString(L"\n");
	OutputDebugString(L"CurFile:  "); OutputDebugString((LPCWSTR)lsPanelFile); OutputDebugString(L"\n");
	OutputDebugString(L"OpenFile: "); OutputDebugString((LPCWSTR)lsOpenFile); OutputDebugString(L"\n");
	OutputDebugString(L"------------------ list of items ---------------\n");
	for (int i = 0; i < mn_ItemsCount; i++)
	{
		wsprintf(szInfo, L"%c%03i: %s%s%s", (i == mn_ActiveRawIndex) ? L'*' : L' ', i,
			mpvi_Items[i].bPicDetected ? L"<Ok> " : L"",
			mpvi_Items[i].bPicDisabled ? L"<Disabled> " : L"",
			(mpvi_Items[i].pItem ? ((PanelItemAttributes(*mpvi_Items[i].pItem) & FILE_ATTRIBUTE_DIRECTORY) ? L"<Dir> " : L"") : L""),
			0);
		OutputDebugString(szInfo);
		OutputDebugString(mpvi_Items[i].pItem ? PanelItemFileNamePtr(*mpvi_Items[i].pItem) : L"<NULL>");
		OutputDebugString(L" : ");
		OutputDebugString(mpvi_Items[i].sFile ? ((LPCWSTR)*mpvi_Items[i].sFile) : L"<NULL>");
		OutputDebugString(L"\n");
	}
	OutputDebugString(L"------------------ end of list -----------------\n");
	#endif

	MCHKHEAP;

	return true;
}

// OpenFrom
//		Должен быть равен OPEN_VIEWER - пойман VE_READ, извлечен следующий файл из архива по CtrlShiftF3
// asFile
//		Должен быть пуст. Функция сама должна получить имя файла.
bool CPicViewPanel::CatchNewFile(int OpenFrom, LPCWSTR asFile)
{
	_ASSERTE(FALSE);
	WARNING("Доделать. Это был извлечен следующий файл из архива через CtrlShiftF3");
	return false;
}

bool CPicViewPanel::InitListFromPanel()
{
	_ASSERTE(m_Panel.ItemsNumber > 0);
	mn_ItemsCount = m_Panel.ItemsNumber;
	// Выделяем +1 элемент, ибо вдруг открываемый файл почему-то не попадет в список?
	mpvi_Items = (PicViewItem*)calloc(mn_ItemsCount+1, sizeof(PicViewItem));

	//mn_ReadyItemsCount = 0;

	for (int i = 0; i < mn_ItemsCount; i++)
	{
		if (!(mpvi_Items[i].pItem = LoadPanelItem(i)))
			continue;
		// Проверить папки и ".."
		bool lbIsFolder = (PanelItemAttributes(*mpvi_Items[i].pItem) & FILE_ATTRIBUTE_DIRECTORY) != 0;
		if (lbIsFolder)
		{
			if (lstrcmp(L"..", PanelItemFileNamePtr(*mpvi_Items[i].pItem)) == 0)
			{
				// Для удобства, пометим имя "пустым"
				((wchar_t*)PanelItemFileNamePtr(*mpvi_Items[i].pItem))[0] = 0;
			}
		}

		mpvi_Items[i].nRawIndex = i;

		mpvi_Items[i].sFile = new CUnicodeFileName;
		if (PanelItemFileNamePtr(*mpvi_Items[i].pItem)[0])
		{
			if (!PutFilePathName(mpvi_Items[i].sFile, PanelItemFileNamePtr(*mpvi_Items[i].pItem), lbIsFolder))
			{
				_ASSERTE(FALSE);
				delete mpvi_Items[i].sFile; mpvi_Items[i].sFile = NULL;
				SafeFree(mpvi_Items[i].pItem);
				continue;
			}
		} else {
			// на элементе ".." можно бы показать текущую папку, если поддерживается показ папок
			mpvi_Items[i].sFile->Assign((LPCWSTR)ms_PanelDir, TRUE);
		}
		
		LPCWSTR pszName = ItemPointToName((LPCWSTR)*mpvi_Items[i].sFile);
		if (!pszName)
		{
			SafeFree(mpvi_Items[i].pItem);
			continue;
		} else {
			PanelItemFileNamePtr(*mpvi_Items[i].pItem) = (wchar_t*)pszName;
		}

		//if (m_Panel.CurrentItem == -1 && lstrcmpiW((LPCWSTR)ms_PanelFile, (LPCWSTR)*(mpvi_Items[i].sFile)) == 0)
		//{
		//	mpvi_Items[i].bPicDetected = TRUE;
		//	mn_ActiveRawIndex = m_Panel.CurrentItem = i;
		//}
		
		

		//// Для облегчения отбражения индекса элемента? --> RescanSupported()
		//mpvi_Items[i].nDisplayItemIndex = mn_ReadyItemsCount++;
	}
	TODO("Выставить mpb_PicDetected[i]=TRUE для всех файлов с тем же расширением, что и у ms_PanelFile?");

	return true;
}

bool CPicViewPanel::InitListFromFileSystem()
{
	int nDirLen = lstrlenW((LPCWSTR)ms_PanelDir);
	_ASSERTE(nDirLen && ((LPCWSTR)ms_PanelDir)[nDirLen-1] == L'\\');
	wchar_t* pszPath = (wchar_t*)calloc(nDirLen+MAX_PATH+1, sizeof(wchar_t));
	lstrcpy(pszPath, (LPCWSTR)ms_PanelDir);
	lstrcpy(pszPath+nDirLen, L"*.*");

	_ASSERTE(mn_ItemsCount == 0);
	mn_ItemsCount = 0;

	int nMaxCount = 2048;

	mpvi_Items = (PicViewItem*)calloc(nMaxCount, sizeof(PicViewItem));

	// Запускаем поиск
	WIN32_FIND_DATA fnd = {0};
	HANDLE hFind = FindFirstFile(pszPath, &fnd);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do {
			// Папки - пропускаем, а уж ".." и подавно
			if ((fnd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
			{
				if (fnd.cFileName[0] == L'.')
				{
					if (fnd.cFileName[1] == 0)
						continue;
					else if (fnd.cFileName[1] == L'.' && fnd.cFileName[2] == 0)
						fnd.cFileName[0] = 0;
				}
			}
			lstrcpy(pszPath+nDirLen, fnd.cFileName);

			// Именно +1, т.к. если открываемый файл не будет найден - его принудительно добавим в конец списка!
			if ((mn_ItemsCount+1) >= nMaxCount)
			{
				// Требуется увеличить количество элементов
				_ASSERTE(mn_ItemsCount < nMaxCount);
				nMaxCount += 2048;
				PicViewItem* pItems = (PicViewItem*)calloc(nMaxCount, sizeof(PicViewItem));
				memmove(pItems, mpvi_Items, mn_ItemsCount*sizeof(PicViewItem));
				SafeFree(mpvi_Items);
				mpvi_Items = pItems;
			}

			AddItem(pszPath, fnd.dwFileAttributes, fnd.nFileSizeLow + (((ULONGLONG)fnd.nFileSizeHigh) << 32), fnd.ftLastWriteTime);
			
		} while (FindNextFile(hFind, &fnd));
		FindClose(hFind);

		TODO("Выставить mpb_PicDetected[i]=TRUE для всех файлов с тем же расширением, что и у ms_PanelFile?");
	}
	
	SortItems();

	return true;
}

//// Когда панели нет
//bool CPicViewPanel::RetrieveAllDirFiles()
//{
//	return false;
//}

void CPicViewPanel::SetActiveRawIndex(int aiRawIndex)
{
	if (aiRawIndex < 0 || aiRawIndex >= mn_ItemsCount)
	{
		_ASSERTE(aiRawIndex >= 0 && aiRawIndex < mn_ItemsCount);
	}
	else if (mpvi_Items[aiRawIndex].pItem == NULL)
	{
		_ASSERTE(mpvi_Items[aiRawIndex].pItem != NULL);
	}
	else if (!GetItem(aiRawIndex))
	{
		_ASSERTE(FALSE);
	}
	else if (mn_ActiveRawIndex != aiRawIndex)
	{
		int iFreeUnused = -1;
		// Если мы попали сюда - уже должен быть реальный файл
		_ASSERTE(mb_RealFilesMode || (mpvi_Items[aiRawIndex].bTempCopy && mpvi_Items[aiRawIndex].sTempFile));

		DecodeParams parms;	
		parms.Flags = eRenderFirstAvailable;
		parms.nRawIndex = aiRawIndex;
		// parms.iDirection = abNext ? 1 : -1;
		parms.Flags |= eRenderRelativeIndex;
		

		// Запомнить (для удобства) индексы показанных файлов
		if (aiRawIndex == mn_LastOpenedRawIndex[PREV_RAW_IDX])
		{
			iFreeUnused = mn_LastOpenedRawIndex[PREV_RAW_IDX];
			mn_LastOpenedRawIndex[PREV_RAW_IDX] = -1;
			mn_LastOpenedRawIndex[NEXT_RAW_IDX] = mn_ActiveRawIndex;
			parms.iDirection = -1;
		}
		else if (aiRawIndex == mn_LastOpenedRawIndex[NEXT_RAW_IDX])
		{
			iFreeUnused = mn_LastOpenedRawIndex[NEXT_RAW_IDX];
			mn_LastOpenedRawIndex[NEXT_RAW_IDX] = -1;
			mn_LastOpenedRawIndex[PREV_RAW_IDX] = mn_ActiveRawIndex;
			parms.iDirection = 1;
		}

		// Запомнить новую позицию
		mn_ActiveRawIndex = m_Panel.CurrentItem = aiRawIndex;

		// Проверка индексов
		if (mn_LastOpenedRawIndex[NEXT_RAW_IDX] == aiRawIndex)
		{
			_ASSERTE(mn_LastOpenedRawIndex[NEXT_RAW_IDX] != aiRawIndex);
			mn_LastOpenedRawIndex[NEXT_RAW_IDX] = -1;
		}
		if (mn_LastOpenedRawIndex[PREV_RAW_IDX] == aiRawIndex)
		{
			_ASSERTE(mn_LastOpenedRawIndex[PREV_RAW_IDX] != aiRawIndex);
			mn_LastOpenedRawIndex[PREV_RAW_IDX] = -1;
		}

		// Освобождение дескрипторов устаревшего изображения
		if (iFreeUnused != -1)
			FreeUnusedImage(iFreeUnused);
			
		if (((g_Panel.IsRealNames()) ? g_Plugin.bCachingRP : g_Plugin.bCachingVP))
		{
			if (parms.iDirection)
				g_Manager.RequestDecodedImage(&parms);
		}
	}
}

bool CPicViewPanel::IsMarkAllowed()
{
	if (!mb_PanelExists)
		return false;
	return true;
}

bool CPicViewPanel::IsFileMarked()
{
	int nCurrent = GetActiveRawIdx();
	if (nCurrent < 0 || nCurrent >= mn_ItemsCount || mpvi_Items[nCurrent].pItem == NULL)
		return false;
	return ((mpvi_Items[nCurrent].pItem->Flags & PPIF_SELECTED) != 0);
}

bool CPicViewPanel::MarkUnmarkFile(enum CPicViewPanel::MarkAction action)
{
	//WaitLoadingFinished();

	if (!mb_PanelExists)
		return false;

	if (GetCurrentThreadId() == gnMainThreadId)
	{
		int  iRawIndex = GetActiveRawIdx();
		BOOL bNewSelected;

		PluginPanelItem *ppi = (PluginPanelItem*)GetPanelItem(iRawIndex);
		if (!ppi)
			return false;

		if (action == ema_Mark)
			bNewSelected = TRUE;
		else if (action == ema_Unmark)
			bNewSelected = FALSE;
		else if (action == ema_Switch)
			bNewSelected = (ppi->Flags & PPIF_SELECTED) != PPIF_SELECTED;

		// чтобы при выходе из плагина можно было обновить панель, показав выделенные вверху
		g_Plugin.SelectionChanged = true; TODO("Переделать в флаг");

		g_StartupInfo.PanelControl(PANEL_ACTIVE, FCTL_BEGINSELECTION, 0, NULL);
		g_StartupInfo.PanelControl(PANEL_ACTIVE, FCTL_SETSELECTION, iRawIndex, (FAR_PARAM)bNewSelected);
		g_StartupInfo.PanelControl(PANEL_ACTIVE, FCTL_ENDSELECTION, 0, NULL);

		// Обновить флаги в наших внутренних структурах
		//g_Plugin.Image[0]->bSelected = bNewSelected;
		if (bNewSelected)
			ppi->Flags |= PPIF_SELECTED;
		else
			ppi->Flags &= ~PPIF_SELECTED;

		TODO("Может быть Redraw выполнять сразу не надо?");
		g_StartupInfo.PanelControl(INVALID_HANDLE_VALUE, FCTL_REDRAWPANEL, 0, 0);
		
		TitleRepaint();
		TODO("Это наверное не здесь нужно выполнять, а там, откуда фукнция вызывается?");
		if (g_Plugin.hSynchroDone)
			SetEvent(g_Plugin.hSynchroDone);
		g_Plugin.FlagsWork &= ~(FW_MARK_FILE | FW_UNMARK_FILE);
		return true;
		
	}
	else
	{
		_ASSERTE(FALSE);
	}
	
	return false;
}

//// Когда панели не было
//bool CPicViewPanel::UpdatePanelDir()
//{
//	return false;
//}

// Закрыть все дескрипторы дисплеев
void CPicViewPanel::CloseImageHandles()
{
	for (int i = 0; i < mn_ItemsCount; i++)
	{
		if (mpvi_Items[i].pImage)
		{
			mpvi_Items[i].pImage->Close();
		}
	}
}

// Закрыть только дескрипторы дисплеев
void CPicViewPanel::CloseDisplayHandles()
{
	for (int i = 0; i < mn_ItemsCount; i++)
	{
		if (mpvi_Items[i].pImage)
			mpvi_Items[i].pImage->CloseDisplay();
	}
}

// Вызывается при закрытии плагина - полная остановка всего
void CPicViewPanel::OnTerminate()
{
	TODO("Остановить нить копирования/извлечения файлов");

//#ifdef _DEBUG
//	ms_PanelFile.Validate();
//#endif

	MCHKHEAP;
	CloseImageHandles();
	MCHKHEAP;

	// GFL портит консоль при просмотре WMF
	#ifdef FAR_UNICODE
	g_StartupInfo.AdvControl(PluginNumber, ACTL_REDRAWALL, NULL, NULL);
	#else
	g_StartupInfo.AdvControl(PluginNumber, ACTL_REDRAWALL, NULL);
	#endif

	// Подвинуть курсор на панели, обновить, если была смена выделения (чтобы выделенные по настройке вверх прыгнули)
	g_Panel.RedrawOnExit();

	g_Panel.CheckAllClosed();
	
	g_Panel.FreeAllItems();
}

// Когда панели нет
void CPicViewPanel::SortItems()
{
	int iMaxIndex = mn_ItemsCount;
	for (int i = 0; i < (iMaxIndex-1); i++)
	{
		int nMin = i; //, nMax = -1;
		for (int j = (i + 1); j < iMaxIndex; j++)
		{
			if (CompareItems(mpvi_Items[nMin].pItem, mpvi_Items[j].pItem)>0)
				nMin = j;
		}
		if (nMin != i)
		{
			PicViewItem t;
			t = mpvi_Items[i];
			mpvi_Items[i] = mpvi_Items[nMin];
			mpvi_Items[nMin] = t;
			if (mn_ActiveRawIndex == i)
				mn_ActiveRawIndex = m_Panel.CurrentItem = nMin;
			else if (mn_ActiveRawIndex == nMin)
				mn_ActiveRawIndex = m_Panel.CurrentItem = i;
		}
	}
}

// Когда панели нет
// -1 если Item1 < Item2
//  0 если Item1 == Item2
//  1 если Item1 > Item2
int CPicViewPanel::CompareItems(PluginPanelItem* pItem1, PluginPanelItem* pItem2)
{
	if (pItem1 && pItem2)
	{
		bool bFolder1 = (PanelItemAttributes(*pItem1) & FILE_ATTRIBUTE_DIRECTORY) != 0;
		bool bFolder2 = (PanelItemAttributes(*pItem2) & FILE_ATTRIBUTE_DIRECTORY) != 0;
		if (bFolder1 != bFolder2)
		{
			return bFolder1 ? -1 : 1;
		}
		int nCmp = lstrcmpi(PanelItemFileNamePtr(*pItem1), PanelItemFileNamePtr(*pItem2));
		return (nCmp < 0) ? -1 : ((nCmp > 0) ? 1 : 0);
	}
	
	if (pItem1)
		return -1;
	else if (pItem2)
		return 1;
	else
		return 0;
}

//const wchar_t* CPicViewPanel::GetCurrentItemName()
//{
//	// ppi->FindData.lpwszFileName
//	return (const wchar_t*)ms_PanelFile;
//}

// индекс в mp_Items, 0-based
int CPicViewPanel::GetActiveRawIdx()
{
	// индекс в mp_Items
	_ASSERTE(mn_ActiveRawIndex == m_Panel.CurrentItem);
	return mn_ActiveRawIndex;
}

// индекс для отображения в заголовке и в OSD, 1-based!
int CPicViewPanel::GetActiveDisplayIdx()
{
	_ASSERTE(mn_ActiveRawIndex == m_Panel.CurrentItem);
	if (mn_ActiveRawIndex < 0 || mn_ActiveRawIndex >= mn_ItemsCount)
	{
		_ASSERTE(mn_ActiveRawIndex >= 0 && mn_ActiveRawIndex < mn_ItemsCount);
		return 0;
	}
	return mpvi_Items[mn_ActiveRawIndex].nDisplayItemIndex;
}

const wchar_t* CPicViewPanel::GetCurrentDir()
{
	return (const wchar_t*)ms_PanelDir;
}

PluginPanelItem* CPicViewPanel::LoadPanelItem(int aiPanelItemIndex)
{
	if (!mb_PanelExists)
	{
		_ASSERTE(mb_PanelExists);
		ShowError(L"mb_PanelExists == false in CPicViewPanel::LoadPanelItem");
		return NULL;
	}

	if (aiPanelItemIndex < 0 || aiPanelItemIndex >= (INT_PTR)m_Panel.ItemsNumber)
	{
		wchar_t szErrInfo[256];
		wsprintfW(szErrInfo, L"aiPanelItemIndex (%i) out of bounds (0..%i) in CPicViewPanel::LoadPanelItem",
			aiPanelItemIndex, m_Panel.ItemsNumber);
		ShowError(szErrInfo);
		return NULL;
	}


	if (size_t len = g_StartupInfo.PanelControl(INVALID_HANDLE_VALUE, FCTL_GETPANELITEM, aiPanelItemIndex, 0))
	{
		if (PluginPanelItem* ppi = (PluginPanelItem*)malloc(len))
		{
			size_t rc;
			#ifdef FAR_UNICODE
			FarGetPluginPanelItem get={len, ppi};
			rc = g_StartupInfo.PanelControl(INVALID_HANDLE_VALUE, FCTL_GETPANELITEM, aiPanelItemIndex, (FAR_PARAM)&get);
			#else
			rc = g_StartupInfo.PanelControl(INVALID_HANDLE_VALUE, FCTL_GETPANELITEM, aiPanelItemIndex, (FAR_PARAM)ppi);
			#endif
			if (rc)
			{
				return ppi;
			}
			else
			{
				free(ppi);
				wchar_t szErrInfo[256];
				wsprintfW(szErrInfo, L"FCTL_GETPANELITEM(%i) failed in CPicViewPanel::LoadPanelItem");
				ShowError(szErrInfo);
				return NULL;
			}
		}
		else
		{
			wchar_t szErrInfo[256];
			wsprintfW(szErrInfo, L"Can't allocate %u bytes in CPicViewPanel::LoadPanelItem", (DWORD)len);
			ShowError(szErrInfo);
			return NULL;
		}
	}

	wchar_t szErrInfo[256];
	wsprintfW(szErrInfo, L"FCTL_GETPANELITEM(%i).size failed in CPicViewPanel::LoadPanelItem", aiPanelItemIndex);
	ShowError(szErrInfo);
	return NULL;
}

//void CPicViewPanel::WaitLoadingFinished()
//{
//	TODO("Если список файлов загружается из ФС, что может быть медленно - дождаться окончания загрузки");
//}

// подерживаются ли элементы с таким расширением?
bool CPicViewPanel::IsItemSupported(int aiRawIndex)
{
	if (aiRawIndex < 0 || aiRawIndex >= mn_ItemsCount)
	{
		_ASSERTE(aiRawIndex >= 0 && aiRawIndex < mn_ItemsCount);
		return false;
	}

	if (mpvi_Items[aiRawIndex].pItem == NULL || mpvi_Items[aiRawIndex].bPicDisabled)
	{
		_ASSERTE(mpvi_Items[aiRawIndex].bPicDetected == FALSE);
		return false;
	}

	//
	//if (mpvi_Items[aiRawIndex].pItem->FindData.lpwszFileName == NULL
	//	|| *mpvi_Items[aiRawIndex].pItem->FindData.lpwszFileName == 0)
	//{
	//	return false;
	//}

	// Если этот элемент уже был успешно открыт - возвращаем true
	if (mpvi_Items[aiRawIndex].bPicDetected)
		return true;

	if (!mb_RealFilesMode && !ms_ExtractMacroText)
	{
		mpvi_Items[aiRawIndex].bPicDisabled = TRUE;
		return false;
	}

	// Папки можно "смотреть" только при включенной настройке
	if ((PanelItemAttributes(*mpvi_Items[aiRawIndex].pItem) & FILE_ATTRIBUTE_DIRECTORY) != 0)
	{
		// На плагиновых панелях - просмотр папок запрещен
		if ((mb_PluginMode && !mb_RealFilesMode) || !g_Plugin.bViewFolders)
		{
			mpvi_Items[aiRawIndex].bPicDisabled = TRUE;
			return false;
		} else {
			return true;
		}
	}

	if (!g_Manager.IsSupportedExtension((LPCWSTR)*mpvi_Items[aiRawIndex].sFile))
	{
		mpvi_Items[aiRawIndex].bPicDisabled = TRUE;
		//if (mn_ReadyItemsCount>1)
		//{
		//	mn_ReadyItemsCount--;
		//} else {
		//	_ASSERTE(mn_ReadyItemsCount>1);
		//}
		return false;
	}
	
	return true;
}

// Вызывается сразу, из CPicViewPanel::Initialize
void CPicViewPanel::RescanSupported()
{
	mn_ReadyItemsCount = 0;
	for (int i = 0; i < mn_ItemsCount; i++)
	{
		if (mpvi_Items[i].pItem == NULL)
		{
			mpvi_Items[i].nDisplayItemIndex = 0;
			mpvi_Items[i].bPicDisabled = TRUE;
			continue;
		}

		mpvi_Items[i].bPicDisabled = FALSE;
		if (IsItemSupported(i))
		{
			// Этот элемент может быть удастся открыть (подошло расширение, или это точка входа)
			mpvi_Items[i].nDisplayItemIndex = ++mn_ReadyItemsCount;
			_ASSERTE(mpvi_Items[i].bPicDisabled == FALSE);
		} else {
			_ASSERTE(mpvi_Items[i].bPicDisabled == TRUE);
			mpvi_Items[i].bPicDisabled = TRUE;
		}
	}
}

int CPicViewPanel::GetFirstItemRawIndex()
{
	int nCurrent = 0;
	while (nCurrent < mn_ItemsCount)
	{
		// Функция проверит допустимость, и извлечет файл из архива, если требуется
		if (GetItem(nCurrent))
			return nCurrent;
		nCurrent++;
	}
	_ASSERTE(FALSE);
	return -1;
}

int CPicViewPanel::GetLastItemRawIndex()
{
	int nCurrent = mn_ItemsCount - 1;
	while (nCurrent >= 0)
	{
		// Функция проверит допустимость, и извлечет файл из архива, если требуется
		if (GetItem(nCurrent))
			return nCurrent;
		nCurrent--;
	}
	_ASSERTE(FALSE);
	return -1;
}

// Вернуть следующий RawIndex для файла. Видимо, возвращать нужно только обрабатываемые файлы!
int CPicViewPanel::GetNextItemRawIdx(int anStep /*= 1*/, int aiFromRawIndex /*= -1*/)
{
	int nCurrent = (aiFromRawIndex == -1) ? GetActiveRawIdx() : aiFromRawIndex;
	if (!anStep)
		return nCurrent;
	int nLoopCount = 0;
	if (anStep > 0)
	{
		while (anStep > 0)
		{
			nCurrent++;
			if (nCurrent >= mn_ItemsCount)
			{
				// кончились. перейти в начало?
				if (g_Plugin.bLoopJump && !nLoopCount)
				{
					nLoopCount++; nCurrent = -1;
					continue;
				}
				return -1;
			}
			if (mpvi_Items[nCurrent].pItem && !mpvi_Items[nCurrent].bPicDisabled)
				anStep--;
		}
	} else {
		while (anStep < 0)
		{
			nCurrent--;
			if (nCurrent < 0)
			{
				// кончились. перейти в конец?
				if (g_Plugin.bLoopJump && !nLoopCount)
				{
					nLoopCount++; nCurrent = mn_ItemsCount;
					continue;
				}
				return -1;
			}
			if (mpvi_Items[nCurrent].pItem && !mpvi_Items[nCurrent].bPicDisabled)
				anStep++;
		}
	}
	return nCurrent;
}

int CPicViewPanel::GetReadyItemsCount()
{
	return mn_ReadyItemsCount;
}

// Для упрощения заполнения pItem из файловой системы
bool CPicViewPanel::AddItem(LPCWSTR asFilePathName, DWORD dwFileAttributes, ULONGLONG nFileSize, FILETIME ftLastWriteTime)
{
	//// Для облегчения отбражения индекса элемента? -> RescanSupported()
	//mpvi_Items[mn_ItemsCount].nDisplayItemIndex = mn_ReadyItemsCount+1;
	
	mpvi_Items[mn_ItemsCount].nRawIndex = mn_ItemsCount;

	mpvi_Items[mn_ItemsCount].sFile = new CUnicodeFileName;
	mpvi_Items[mn_ItemsCount].sFile->Assign(asFilePathName,
			((dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0));

	// Найти только имя. Учесть, что для папок - на конце ставится '\\'
	LPCWSTR pszName = ItemPointToName((LPCWSTR)*(mpvi_Items[mn_ItemsCount].sFile));
	if (!pszName)
	{
		mpvi_Items[mn_ItemsCount].pItem = NULL;
	} else {
		mpvi_Items[mn_ItemsCount].pItem = (PluginPanelItem*)calloc(1,sizeof(PluginPanelItem));
		PanelItemFileNamePtr(*mpvi_Items[mn_ItemsCount].pItem) = (wchar_t*)pszName;
		PanelItemAttributes(*mpvi_Items[mn_ItemsCount].pItem) = dwFileAttributes;
		PanelItemCreation(*mpvi_Items[mn_ItemsCount].pItem) = ftLastWriteTime;
		PanelItemAccess(*mpvi_Items[mn_ItemsCount].pItem) = ftLastWriteTime;
		PanelItemWrite(*mpvi_Items[mn_ItemsCount].pItem) = ftLastWriteTime;
		PanelItemChange(*mpvi_Items[mn_ItemsCount].pItem) = ftLastWriteTime;
		PanelItemFileSize(*mpvi_Items[mn_ItemsCount].pItem) = nFileSize; // fnd.nFileSizeLow + (((ULONGLONG)fnd.nFileSizeHigh) << 32);
	}
	
	//if (m_Panel.CurrentItem == -1 && lstrcmpiW((LPCWSTR)ms_PanelFile, (LPCWSTR)*(mpvi_Items[mn_ItemsCount].sFile)) == 0)
	//{
	//	mpvi_Items[mn_ItemsCount].bPicDetected = TRUE;
	//	mn_ActiveRawIndex = m_Panel.CurrentItem = mn_ItemsCount;
	//}

	// Increment counts
	mn_ItemsCount++;
	//mn_ReadyItemsCount++;

	return true;
}

// Работаем с реальной ФС, или только через плагин
// В TempPanel, например, хоть это и плагин, но работаем через ФС
bool CPicViewPanel::IsRealNames()
{
	if (!mb_PluginMode)
		return true;
	if (mb_RealFilesMode)
		return true;
	return false;
}

bool CPicViewPanel::CacheFile(int aiRawIndex, LPCWSTR asSrcFile)
{
	if (ms_TempDir.IsEmpty())
	{
		return false;
	}
	if (aiRawIndex < 0 || aiRawIndex >= mn_ItemsCount)
	{
		_ASSERTE(aiRawIndex>=0 && aiRawIndex < mn_ItemsCount);
		return false;
	}
	
	if (!mpvi_Items[aiRawIndex].pItem)
	{
		_ASSERTE(mpvi_Items[aiRawIndex].pItem);
		return false;
	}
	if (mpvi_Items[aiRawIndex].bTempCopy && mpvi_Items[aiRawIndex].sTempFile && !mpvi_Items[aiRawIndex].sTempFile->IsEmpty())
	{
		// Этот файл - уже кеширован!
		_ASSERTE(mpvi_Items[aiRawIndex].bTempCopy == FALSE);
		return true;
	}
	
	LPCWSTR pszName = ItemPointToName(asSrcFile);
	LPCWSTR pszExt = pszName ? wcsrchr(pszName, L'.') : NULL;
	if (!pszExt || !*pszExt) pszExt = L"";

	CUnicodeFileName sTemp;
	wchar_t* pszBuf = sTemp.GetBuffer(lstrlen((LPCWSTR)ms_TempDir)+MAX_PATH+12);
	wsprintf(pszBuf, L"%s%08u%s", (LPCWSTR)ms_TempDir, aiRawIndex, pszExt);
	
	CUnicodeFileName sSource;
	sSource = asSrcFile;
	
	if (!CopyFile((LPCWSTR)sSource, (LPCWSTR)sTemp, FALSE))
	{
		_ASSERTE(FALSE);
		//RequestTerminate();
		return false;
	}

	if (mpvi_Items[aiRawIndex].sTempFile == NULL)
		mpvi_Items[aiRawIndex].sTempFile = new CUnicodeFileName;

	mpvi_Items[aiRawIndex].sTempFile->Assign((LPCWSTR)sTemp, false);
	mpvi_Items[aiRawIndex].bTempCopy = TRUE;

	return true;
}

bool CPicViewPanel::LoadItemFromPlugin(int aiRawIndex)
{
	if (GetCurrentThreadId() == gnMainThreadId)
	{
		// В главной нити вызов функции не допускается
		_ASSERTE(GetCurrentThreadId() != gnMainThreadId);
		return false;
	}
	if (mb_RealFilesMode)
	{
		// Для "реальных имен" функция вызываться не должна
		_ASSERTE(mb_RealFilesMode == FALSE);
		return true;
	}
	
	if (mpvi_Items[aiRawIndex].bTempCopy
		&& mpvi_Items[aiRawIndex].sTempFile
		&& !mpvi_Items[aiRawIndex].sTempFile->IsEmpty())
	{
		// Файл Уже извлечен - сюда попасть не должны были
		_ASSERTE(mpvi_Items[aiRawIndex].bTempCopy == FALSE);
		return true;
	}
	
	if (!ms_ExtractMacroText || !*ms_ExtractMacroText)
	{
		// Макроса нет - извлечение заблокировано, ошибку не показываем
		return false;
	}
	
	if ((g_Plugin.FlagsWork & FW_IN_RETRIEVE))
	{
		_ASSERTE((g_Plugin.FlagsWork & FW_IN_RETRIEVE) == 0);
		return false;
	}
	
	mn_LoadItemFromPlugin = aiRawIndex;
	if (!mh_LoadItemFromPlugin)
		mh_LoadItemFromPlugin = CreateEvent(NULL, FALSE, FALSE, NULL);
	else
		ResetEvent(mh_LoadItemFromPlugin);
	
	if (!ExecuteInMainThread(FW_RETRIEVE_FILE, 0))
	{
		mn_LoadItemFromPlugin = -1;
		return false;
	}
	if (mn_LoadItemFromPlugin == -1)
	{
		// Запуститься не удалось
		return false;
	}
	
	// Это ушел только запрос на макрос. Нужно должаться его результата...
	
	DWORD nWait = WAIT_TIMEOUT;
	while (nWait != WAIT_OBJECT_0)
	{
		nWait = WaitForSingleObject(mh_LoadItemFromPlugin, 100);
		if ((g_Plugin.FlagsWork & FW_TERMINATE))
		{
			// Выход из плагина!
			return false;
		}
	}
	
	bool lbSuccess = false;
	
	if (mpvi_Items && mpvi_Items[aiRawIndex].bTempCopy
		&& mpvi_Items[aiRawIndex].sTempFile && !mpvi_Items[aiRawIndex].sTempFile->IsEmpty())
	{
		lbSuccess = true;
	}

	return lbSuccess;
}

bool CPicViewPanel::StartItemExtraction()
{
	// Должно выполняться ТОЛЬКО в главной нити!
	if (GetCurrentThreadId() != gnMainThreadId)
	{
		_ASSERTE(GetCurrentThreadId() == gnMainThreadId);
		mn_LoadItemFromPlugin = -1;
		return false;
	}

	if ((g_Plugin.FlagsWork & FW_IN_RETRIEVE))
	{
		_ASSERTE((g_Plugin.FlagsWork & FW_IN_RETRIEVE) == 0);
		mn_LoadItemFromPlugin = -1;
		return false;
	}
	
	if (mn_LoadItemFromPlugin == -1)
	{
		_ASSERTE(mn_LoadItemFromPlugin != -1);
		return false;
	}

	
	INT_PTR liRc;
	#ifdef FAR_UNICODE
	liRc = g_StartupInfo.MacroControl(PluginNumber, MCTL_GETSTATE, 0, 0);
	#else
	ActlKeyMacro mcr = {MCMD_GETSTATE};
	liRc = g_StartupInfo.AdvControl(PluginNumber, ACTL_KEYMACRO, &mcr);
	#endif
	if (liRc != MACROSTATE_NOMACRO)
	{
		// Сейчас УЖЕ выполняется макрос. Нельзя.
		_ASSERTE(liRc == MACROSTATE_NOMACRO);
		mn_LoadItemFromPlugin = -1;
		return false;
	}

	wchar_t sIndex[32]; wsprintf(sIndex, L"%u", mn_LoadItemFromPlugin+1); // 1-based!
	SetEnvironmentVariable(L"PicViewAction", L"RETRIEVE");
	SetEnvironmentVariable(L"PicViewArgument", sIndex);


	if (!ms_ExtractMacroText || !*ms_ExtractMacroText)
	{
		_ASSERTE(ms_ExtractMacroText && *ms_ExtractMacroText);
		mn_LoadItemFromPlugin = -1;
		return false;
	}

	// Сложный макрос по позиционированию на файл и открытию его во вьювере
	// Макрос хранится в файле "Extract.macro" в каталоге плагина
	// "Параметры" передаются через переменные окружения "PicViewAction" и "PicViewArgument"
	if (!PostMacro(ms_ExtractMacroText, FALSE))
	{
		mn_LoadItemFromPlugin = -1;
		return false;
	}

	//mcr.Command = MCMD_POSTMACROSTRING;
	//mcr.Param.PlainText.SequenceText = apszMacro;
	//mcr.Param.PlainText.Flags = KSFLAGS_DISABLEOUTPUT | KSFLAGS_NOSENDKEYSTOPLUGINS;

	//#ifdef FAR_UNICODE
	//liRc = g_StartupInfo.AdvControl(PluginNumber, ACTL_KEYMACRO, 0, &mcr);
	//#else
	//liRc = g_StartupInfo.AdvControl(PluginNumber, ACTL_KEYMACRO, &mcr);
	//#endif
	//if (!liRc)
	//{
	//	mn_LoadItemFromPlugin = -1;
	//	return false;
	//}

	return true;
}

// Если pImage еще не создан создать
CImage* CPicViewPanel::CreateImageData(int aiRawIndex)
{
	if (aiRawIndex < 0 || aiRawIndex >= mn_ItemsCount)
	{
		_ASSERTE(aiRawIndex >= 0 && aiRawIndex < mn_ItemsCount);
		return NULL;
	}
	if (mpvi_Items[aiRawIndex].pItem == NULL)
	{
		_ASSERTE(mpvi_Items[aiRawIndex].pItem != NULL);
		return NULL;
	}

	LPCWSTR pszFileName = GetItemFile(aiRawIndex);
	if (!pszFileName)
		return NULL;

	if (mpvi_Items[aiRawIndex].pImage == NULL)
	{
		mpvi_Items[aiRawIndex].pImage = new CImage(szPicViewPanel, pszFileName, aiRawIndex);
		
		if (mpvi_Items[aiRawIndex].pImage)
		{
			if (mpvi_Items[aiRawIndex].bTempCopy && !mpvi_Items[aiRawIndex].sTempFile)
			{
				_ASSERTE(mpvi_Items[aiRawIndex].bTempCopy && mpvi_Items[aiRawIndex].sTempFile);
				WARNING("Заменить Release на какой-нибудь Close, или предварительно позакрывать все...");
				if (mpvi_Items[aiRawIndex].pImage->Release(szPicViewPanel))
				{
					_ASSERTE("pImage was not Released" && 0);
					while (mpvi_Items[aiRawIndex].pImage->Release(NULL))
						;
				}
				mpvi_Items[aiRawIndex].pImage = NULL;
				return NULL;
			}
			
			mpvi_Items[aiRawIndex].pImage->pszFileNameOnly = ItemPointToName(PanelItemFileNamePtr(*mpvi_Items[aiRawIndex].pItem));

			// -- низя. правильное имя файла "достает" GetItemFile
			//mpvi_Items[aiRawIndex].pImage->FileName =
			//	mpvi_Items[aiRawIndex].bTempCopy ? (LPCWSTR)(*mpvi_Items[aiRawIndex].sTempFile) : (LPCWSTR)(*mpvi_Items[aiRawIndex].sFile);

			//// Подчистить кеш
			//FreeUnusedImageData();
		}
	}

	return mpvi_Items[aiRawIndex].pImage;
}

void CPicViewPanel::FreeUnusedImage(int aiRawIndex)
{
	if (aiRawIndex < 0 || aiRawIndex >= mn_ItemsCount)
	{
		_ASSERTE(aiRawIndex >= 0 && aiRawIndex < mn_ItemsCount);
		return;
	}
	if (aiRawIndex == mn_ActiveRawIndex)
	{
		_ASSERTE(aiRawIndex != mn_ActiveRawIndex);
		return;
	}

	if (mpvi_Items[aiRawIndex].pImage)
		mpvi_Items[aiRawIndex].pImage->Close();
}

// Сделано с таким расчетом, что в Items могут быть и папки, а в них
// на конце принудительно проставлен слеш!
LPCWSTR CPicViewPanel::ItemPointToName(LPCWSTR apszFull)
{
	if (!apszFull)
		return NULL;
	if (!*apszFull)
		return NULL;

	LPCWSTR pszSlash = wcsrchr(apszFull, L'\\');
	if (!pszSlash)
	{
		pszSlash = apszFull;
	}
	else if (pszSlash[1])
	{
		pszSlash++;
	}
	else
	{
		_ASSERTE(pszSlash && pszSlash[0] == L'\\' && pszSlash[1] == 0);
		while (pszSlash > apszFull && *(pszSlash-1) != L'\\')
			pszSlash--;
	}

	return pszSlash;
}

// ЭТА фукнция не должна извлекать файлы из архива!!!
const PicViewItem* CPicViewPanel::GetItem(int aiRawIndex /*= -1*/)
{
	// В этом случае (единственном) не выполняется LoadItemFromPlugin(), даже если файл не был загружен
	if (aiRawIndex == -1)
	{
		aiRawIndex = GetActiveRawIdx();
		if (aiRawIndex >= 0 && aiRawIndex < mn_ItemsCount)
			return (mpvi_Items+aiRawIndex);
		_ASSERTE(aiRawIndex >= 0 && aiRawIndex < mn_ItemsCount);
		return NULL;
	}

	if (aiRawIndex < 0 || aiRawIndex >= mn_ItemsCount)
	{
		_ASSERTE(aiRawIndex >= 0 && aiRawIndex < mn_ItemsCount);
		return NULL;
	}

	if (mpvi_Items[aiRawIndex].pItem == NULL || mpvi_Items[aiRawIndex].bPicDisabled)
	{
		return NULL;
	}

	// Нельзя. Должно вызываться в GetImage, иначе зациклится
	//if (!CreateImageData(aiRawIndex))
	//	return NULL;
	
	if (mpvi_Items[aiRawIndex].nRawIndex != aiRawIndex)
	{
		_ASSERTE(mpvi_Items[aiRawIndex].nRawIndex == aiRawIndex);
		mpvi_Items[aiRawIndex].nRawIndex = aiRawIndex;
	}

	return (mpvi_Items+aiRawIndex);
}

// Вернуть полный путь к реальному (возможно, скопированному в TEMP) файлу
LPCWSTR CPicViewPanel::GetItemFile(int aiRawIndex /*= -1*/)
{
	const PicViewItem* pItem = GetItem(aiRawIndex);
	if (!pItem)
		return NULL;

	// Если файл "виртуальный" - нужно его извлечь из архива
	if (!mb_RealFilesMode && mpvi_Items[aiRawIndex].bTempCopy == FALSE && mpvi_Items[aiRawIndex].sTempFile == NULL)
	{
		if (!LoadItemFromPlugin(aiRawIndex))
		{
			mpvi_Items[aiRawIndex].bPicDisabled = TRUE;
			return NULL;
		}
		if (/*mpvi_Items[aiRawIndex].bTempCopy == FALSE || */mpvi_Items[aiRawIndex].sTempFile == NULL)
		{
			_ASSERTE(mpvi_Items[aiRawIndex].bTempCopy && mpvi_Items[aiRawIndex].sTempFile);
			mpvi_Items[aiRawIndex].bPicDisabled = TRUE;
			return NULL;
		}
	}

	if (mpvi_Items[aiRawIndex].sTempFile)
		return (LPCWSTR)*mpvi_Items[aiRawIndex].sTempFile;
	else
		return (LPCWSTR)*mpvi_Items[aiRawIndex].sFile;
}

TODO("По хорошему, тут нужно переделать на CImagePtr&");
CImage* CPicViewPanel::GetImage(int aiRawIndex /*= -1*/)
{
	// (aiRawIndex!=-1) - возвращает текущий элемент
	const PicViewItem* pItem = GetItem(aiRawIndex); // НЕ ВЫЗЫВАЕТ CreateImageData
	if (!pItem)
		return NULL;
	if (pItem->pImage == NULL)
	{
		aiRawIndex = pItem->nRawIndex;
		_ASSERTE(aiRawIndex == (int)(pItem - mpvi_Items));
		if (!CreateImageData(aiRawIndex))
			return NULL;
	}
	_ASSERTE(pItem->pImage!=NULL);
	return pItem->pImage;
}

//LPVOID CPicViewPanel::GetImageDraw(int aiRawIndex /*= -1*/)
//{
//	CImage* pImage = GetImage(aiRawIndex);
//	if (!pImage)
//		return NULL;
//	return pImage->pDraw;
//}

const PluginPanelItem* CPicViewPanel::GetPanelItem(int aiRawIndex /*= -1*/)
{
	//WaitLoadingFinished();

	if (aiRawIndex < 0)
		aiRawIndex = GetActiveRawIdx();

	if (aiRawIndex >= mn_ItemsCount)
	{
		//ShowError(L"aiRawIndex out of bounds in CPicViewPanel::GetPanelItem");
		wchar_t szErrInfo[256];
		wsprintfW(szErrInfo, L"aiRawIndex (%i) out of bounds (0..%i) in CPicViewPanel::GetPanelItem",
			aiRawIndex, m_Panel.ItemsNumber);
		ShowError(szErrInfo);
		return NULL;
	}
	if (mpvi_Items == NULL || mpvi_Items[aiRawIndex].pItem == NULL)
	{
		_ASSERTE(mpvi_Items && mpvi_Items[aiRawIndex].pItem);
		wchar_t szErrInfo[256];
		wsprintfW(szErrInfo, L"(mp_Items == NULL || mp_Items[%i] == NULL) in CPicViewPanel::GetPanelItem", aiRawIndex);
		ShowError(szErrInfo);
		return NULL;
	}
	return mpvi_Items[aiRawIndex].pItem;
}

void CPicViewPanel::CheckAllClosed()
{
	if (!mpvi_Items)
		return;

	MCHKHEAP;
	for (int i = 0; i < mn_ItemsCount; i++)
	{
		if (mpvi_Items[i].pImage)
		{
			//_ASSERTE(mpvi_Items[i].pImage->mp_ImageContext == NULL);
			_ASSERTE(mpvi_Items[i].pImage->mp_File == NULL);
			mpvi_Items[i].pImage->CheckPagesClosed();
		}
	}
	MCHKHEAP;
}

// Вызывается в главной нити, перед самым выходом из плагина
void CPicViewPanel::RedrawOnExit()
{
	if (mb_PanelExists)
	{
		if (mn_ActiveRawIndex >= 0)
		{
			PanelRedrawInfo pri = {mn_ActiveRawIndex, m_Panel.TopPanelItem};
			g_StartupInfo.PanelControl(PANEL_ACTIVE, FCTL_REDRAWPANEL, 0, (DLG_LPARAM)&pri);
		}

		if (g_Plugin.SelectionChanged)
		{
			PostMacro(L"CtrlR", FALSE);
			//ActlKeyMacro km = {MCMD_POSTMACROSTRING};
			//km.Param.PlainText.SequenceText = L"CtrlR";
			//#ifdef FAR_UNICODE
			//g_StartupInfo.AdvControl(PluginNumber, ACTL_KEYMACRO, 0, &km);
			//#else
			//g_StartupInfo.AdvControl(PluginNumber, ACTL_KEYMACRO, &km);
			//#endif
		}
	}

	g_Plugin.SelectionChanged = false;
}

void CPicViewPanel::FreeUnusedDisplayData(int anForceLevel /*= 0*/)
{
	TODO("Освободить неиспользуемые дескрипторы дисплея");
	// Если (anForceLevel > 0) это значит, что при попытке перенести данные
	// очередного изображения модулю дисплея не хватило памяти!
	// При (anForceLevel == 1) - требуется освободить кеш всех элементов, кроме активного
	// При (anForceLevel == 2) - освободить вообще все (в том числе все ранее открытые дескрипторы активного изображения)
}

void CPicViewPanel::SetItemDisabled(int aiRawIndex)
{
	PicViewItem* pItem = (PicViewItem*)GetItem(aiRawIndex);
	if (pItem && !pItem->bPicDisabled)
	{
		pItem->bPicDisabled = TRUE;
		pItem->nDisplayItemIndex = 0;
		if (mn_ReadyItemsCount > 0)
			mn_ReadyItemsCount --;
			
		_ASSERTE(pItem->nRawIndex == (int)(pItem - mpvi_Items));
		
		for (int i = (pItem->nRawIndex + 1); i < mn_ItemsCount; i++)
		{
			if (mpvi_Items[i].nDisplayItemIndex)
				mpvi_Items[i].nDisplayItemIndex --;
		}	
	}
}

bool CPicViewPanel::InitExtractMacro()
{
	bool lbRc = true; // false возвращает ТОЛЬКО при ошибках в макросе!
	
	SafeFree(ms_ExtractMacroText);
	//ms_ExtractMacroText
	int nLen = lstrlen(g_StartupInfo.ModuleName)+32;
	wchar_t* pszMacroFile = (wchar_t*)malloc(nLen*sizeof(wchar_t));
	lstrcpy(pszMacroFile, g_StartupInfo.ModuleName);
	wchar_t* pszSlash = wcsrchr(pszMacroFile, L'\\');
	if (pszSlash) pszSlash++; else pszSlash = pszMacroFile;
	lstrcpy(pszSlash, L"Extract.macro");
	HANDLE hFile = CreateFile(pszMacroFile, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		DWORD nDataSize = GetFileSize(hFile, NULL);
		if (nDataSize && nDataSize != INVALID_FILE_SIZE)
		{
			LPBYTE ptrData = (LPBYTE)malloc(nDataSize);
			if (ReadFile(hFile, ptrData, nDataSize, &nDataSize, NULL))
			{
				if (ptrData[0] == 0xFF && ptrData[1] == 0xFE)
				{
					//CP1200 with BOM
					ms_ExtractMacroText = (wchar_t*)calloc(nDataSize+4,1);
					memmove(ms_ExtractMacroText, ptrData+2, nDataSize);
					
					// Можно сразу проверить текст макроса
					if (!CheckFarMacroText(ms_ExtractMacroText))
					{
						SafeFree(ms_ExtractMacroText);
						lbRc = false;
					}
				}
			}
			SafeFree(ptrData);
		}
		CloseHandle(hFile);
	}
	
	return lbRc;
}

bool CPicViewPanel::OnFileExtractedToViewer()
{
	_ASSERTE((g_Plugin.FlagsWork & FW_IN_RETRIEVE));
	if (mn_LoadItemFromPlugin == -1)
	{
		_ASSERTE(mn_LoadItemFromPlugin != -1);
		return false;
	}
	
	INT_PTR iViewRc;
	ViewerInfo vi = {sizeof(vi)};
	#if FARMANAGERVERSION_BUILD>1851
	iViewRc = g_StartupInfo.ViewerControl(-1, VCTL_GETINFO, 0, &vi);
	#else
	iViewRc = g_StartupInfo.ViewerControl(VCTL_GETINFO, &vi);
	#endif
	if (!iViewRc)
	{
		_ASSERTE(FALSE);
		return false;
	}
	
	bool lbRc = CacheFile(mn_LoadItemFromPlugin, vi.FileName);
	
	// Чтобы нить извлечения среагировала
	SetEvent(mh_LoadItemFromPlugin);
	
	return lbRc;
}
