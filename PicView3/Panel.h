
#pragma once

/**************************************************************************
Copyright (c) 2010 Maximus5
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


// Добавляется к OpenFrom (CPicViewPanel::Initialize) при вызовах из событий VE_READ или EE_READ.
// OpenFrom может быть чисто OPEN_VIEWER или OPEN_EDITOR, если плагин вызван явно через F11 из вьювера/редактора
	#define OPEN_BY_USER 0x80000000

enum PLUGINPANELITEMFLAGS_PV{
  PPIF__WASNOTCHECKED          = 0x0100,
  PPIF__APPROVED               = 0x0200,
  PPIF__INVALID                = 0x0400,
  PPIF__ALLMASK                = 0xFF00,
};

//{
//  PicPanelItem* pPanelItem;
//  Image* pImage; // может быть NULL'ом
//  CPVDModuleBase *pDecoder; // храним указательна декодер, которым смогли открыть файл
//                            // WARNING! при выгрузке декодера нужно шерстить все элементы и сбрасывать
//                            // в NULL если на него кто ссылался
//  int nRefCount; // наверное счетчик будет только один на оба дескриптора (декодер+вывод)
//  LPVOID pDecoderContext;
//  LPVOID pDisplayContext;
//  DWORD nLastUsed; // некий автоинкрементный индекс, по которому опередяются наиболее "старые" элементы при освобождении памяти
//}

class CImage;

struct PicViewItem
{
	BOOL bPicDetected; // если элемент был открыт хотя бы чем-то, или это "точка входа"
	BOOL bPicDisabled; // для всех кроме "точки входа" ставится в TRUE, если расширение не поддерживается
	BOOL bTempCopy;    // файл был скопирован во временную папку (из архива, например, или с медленной сети)
	int  nRawIndex;    // для информации. соответстует индексу этого элемента в mpvi_Items
	int  nDisplayItemIndex; // индекс для отображения в заголовке и в OSD, 1-based!
	wchar_t sFileExt[10];       // расширение файла - нужно для детектирования декодера и создания временного файла
	CUnicodeFileName* sFile;
	CUnicodeFileName* sTempFile; // скопированный в Temp файл из архива или медленного источника (сеть)
	PluginPanelItem* pItem;
	
	// храним указательна декодер, которым смогли открыть файл
	// WARNING! при выгрузке декодера нужно шерстить все элементы
	// и сбрасывать в NULL если на него кто ссылался
	//CPVDModuleBase *pDecoder;   
	

	// количество объектов, которые используют данный файл
	// временный файл (из кеша) нельзя удалять, пока (nFileRefCount>0)
	int nFileRefCount;
	
	// объект, содержащий дескрипторы декодера,
	// дисплея, и информацию об изображении
	CImage *pImage;

	// некий автоинкрементный индекс, по которому опередяются
	// наиболее "старые" элементы при освобождении памяти
	DWORD nLastUsed;
	
	//TODO. Чтение файла с сети/медленных(removable) дисков
	// Смысл в том, чтобы реально считать данные файла, но чтобы это не тормозило работу?
	// Читать бы по 32Mb? Но чтобы можно было отменить чтение, если оно затянется (с медленного ftp?).
	DWORD  nBufferSize; // размер созданного буфера, по идее, равен размеру файла
	DWORD  nBufferRead; // размер считанных данных
	LPBYTE ptrBuffer;   // выделенная память для чтения файла

};

class CPicViewPanel
{
protected:
	// Work mode
	bool mb_PanelExists;
	bool mb_PluginMode;
	bool mb_RealFilesMode;
	// Items
	int mn_ItemsCount, mn_ReadyItemsCount;
	// активное изображение. == m_Panel.CurrentItem
	int mn_ActiveRawIndex;
	// декодируемое в данный момент изображение
	int mn_DecodeRawIndex;
	// для удобства кеширования - уже обработанные (ранее показанные) файлы сверху и снизу от активного
	// [0] - предыдущее изображение, [1] - следующее. Если "-1" - файл в эту стророну от активного не обрабатывался
	int mn_LastOpenedRawIndex[2];

	//struct PluginPanelItem** mp_Items;
	//BOOL *mpb_PicDetected; // для открытого через префикс, или явном вызове плагина, или для уже подтвержденного файла
	//CUnicodeFileName** ms_Files;
	PicViewItem* mpvi_Items;
	// Panel (or directory)
	struct PanelInfo m_Panel;     // FCTL_GETPANELINFO
	CUnicodeFileName ms_PanelDir;  // FCTL_GETPANELDIR
	CUnicodeFileName ms_TempDir;
	//CUnicodeFileName ms_PanelFile; // FCTL_GETPANELITEM(pi.CurrentItem)
public:
	static const struct PluginPanelItem* Invalid;
public:
	CPicViewPanel();
	~CPicViewPanel();
	void FreeAllItems();
	
public:
	enum MarkAction
	{
		ema_Mark = 1,
		ema_Unmark = 2,
		ema_Switch = 3,
	};
	
public:
	bool Initialize(int OpenFrom, LPCWSTR asFile);
	bool CatchNewFile(int OpenFrom, LPCWSTR asFile);
	bool IsInitialized();
	//bool RetrieveAllDirFiles(); // Когда панели нет
	bool MarkUnmarkFile(enum MarkAction action);
	bool IsMarkAllowed();
	bool IsFileMarked();
	bool IsRealNames();
	//bool UpdatePanelDir();
	
	// Закрыть все дескрипторы дисплеев и декодеров
	void CloseImageHandles();

	// Закрыть только дескрипторы дисплеев
	void CloseDisplayHandles();

	// Assertion purposes
	void CheckAllClosed();

	// Вызывается при закрытии плагина - полная остановка всего
	void OnTerminate();
	
	// Вызывается в главной нити, перед самым выходом из плагина
	void RedrawOnExit();

	int  GetActiveRawIdx(); // индекс в mp_Items, 0-based
	int  GetActiveDisplayIdx(); // индекс для отображения в заголовке и в OSD, 1-based!
	void SetActiveRawIndex(int aiRawIndex);
	void SetDecodeRawIndex(int aiRawIndex);
	LPCWSTR GetItemFile(int aiRawIndex = -1); // Вернуть полный путь к реальному (возможно, скопированному в TEMP) файлу
	const PicViewItem* GetItem(int aiRawIndex = -1);
	const PluginPanelItem* GetPanelItem(int aiRawIndex = -1);
	CImage* GetImage(int aiRawIndex = -1);
	//LPVOID GetImageDraw(int aiRawIndex = -1);

	//Информационные функции	
	//const wchar_t* GetCurrentItemName(); // ppi->FindData.lpwszFileName. !!!_БЕЗ_ПУТИ_!!!
	const wchar_t* GetCurrentDir();

	bool IsItemSupported(int aiRawIndex); // подерживаются ли элементы с таким расширением?
	void RescanSupported();
	void SetItemDisabled(int aiRawIndex); // для этого элемента не удалось подобрать декодер
	
	int GetFirstItemRawIndex();
	int GetLastItemRawIndex();
	int GetNextItemRawIdx(int anStep = 1, int aiFromRawIndex = -1);
	int GetReadyItemsCount();
	
	void ShowError(const wchar_t* pszMsg, const wchar_t* pszMsg2=NULL, bool bSysError=false);

	void FreeUnusedDisplayData(int anForceLevel = 0);
	
	LPCWSTR ItemPointToName(LPCWSTR apszFull);
	
	bool StartItemExtraction();
	bool OnFileExtractedToViewer();
	
protected:
	bool InitListFromPanel();
	bool InitListFromFileSystem();
	void SortItems(); // Когда панели нет
	int  CompareItems(PluginPanelItem* pItem1, PluginPanelItem* pItem2);

	wchar_t* ms_ExtractMacroText;
	PluginPanelItem* LoadPanelItem(int aiPanelItemIndex);
	bool CacheFile(int aiRawIndex, LPCWSTR asSrcFile);
	void EraseTempDir();
	bool InitExtractMacro();
	
	//void WaitLoadingFinished();
	bool AddItem(LPCWSTR asFilePathName, DWORD dwFileAttributes, ULONGLONG nFileSize, FILETIME ftLastWriteTime);
	
	int mn_LoadItemFromPlugin;
	HANDLE mh_LoadItemFromPlugin;
	bool LoadItemFromPlugin(int aiRawIndex);
	CImage* CreateImageData(int aiRawIndex);
	void FreeUnusedImage(int aiRawIndex);
};

extern CPicViewPanel g_Panel;
