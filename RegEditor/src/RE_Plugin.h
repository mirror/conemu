
#pragma once



class REPlugin
{
public:
	// Где мы находимся
	RegPath    m_Key;
	// Что мы считали
	RegFolder *mp_Items;
	// Создать/вернуть объект для работы с реестром/файлом
	MRegistryBase *Worker();
	// Remote
	bool    mb_RemoteMode;
	wchar_t ms_RemoteServer[MAX_PATH+2], ms_RemoteLogin[MAX_PATH]; //, ms_RemotePassword[MAX_PATH];
	// Initial modes
	bool    mb_InitialReverseSort;
	int     mn_InitialSortMode;
	// Флаг, что мы отключили сортировку
	bool    mb_SortingSelfDisabled;
	// Host-файл (*.reg или hive, при открытии с панели)
	TCHAR  *mpsz_HostFile;
	// Устанавливается в TRUE на время редактирования макроса
	BOOL    mb_InMacroEdit;
	// Для CPluginActivator
	int     mn_ActivatedCount;
	// Определить нужна ли выгрузка данных при поиске
	int    mb_FindInContents;

	// При входе в reg-файл показать пользователю список действий (Browse/Import32/Import64)
	BOOL   mb_ShowRegFileMenu;

private:
	MRegistryBase *mp_Worker;
	//RegFolder m_TempItems;
	TCHAR* mpsz_PanelTitle;
	int mn_PanelTitleMax;
	TCHAR* mpsz_PanelPath;
	int mn_PanelPathMax;
	
public:
	REPlugin();
	~REPlugin();
	//void AllocateDirLen(int anAddLen);
	BOOL ChDir(LPCWSTR asKey, BOOL abSilence); // по одной папке
	BOOL SetDirectory(LPCWSTR Dir, BOOL abSilence); // полностью
	BOOL SetDirectory(RegItem* pKey, BOOL abSilence);
	LPCTSTR GetPanelTitle();
	BOOL CheckKeyAvailable(RegPath* apKey, BOOL abSilence = FALSE);
	BOOL LoadRegFile(LPCWSTR asRegFilePathName, BOOL abSilence, BOOL abDelayLoading, BOOL abAllowUserMenu);
	void SaveRegFile(BOOL abInClose = FALSE);
	BOOL LoadHiveFile(LPCWSTR asHiveFilePathName, BOOL abSilence, BOOL abDelayLoading);
	void KeyNotExist(RegPath* pKey, LPCTSTR asSubKey);
	#ifndef _UNICODE
	void KeyNotExist(RegPath* pKey, LPCWSTR asSubKey);
	#endif
	static void KeyNameTooLong(LPCWSTR asSubKey);
	static void CantOpenKey(RegPath* pKey, int abModify); // 0-read, 1-write, -1-delete
	static void CantOpenKey(RegPath* pKey, LPCWSTR asSubKey, BOOL abModify);
	static void CantLoadSaveKey(LPCWSTR asSubKey, LPCTSTR asFile, BOOL abSave);
	#ifndef _UNICODE
	static void CantLoadSaveKey(LPCWSTR asSubKey, LPCWSTR asFile, BOOL abSave);
	#endif
	static void ValueNotExist(RegPath* pKey, LPCWSTR asSubKey);
	static BOOL ValueOperationFailed(RegPath* pKey, LPCWSTR asValueName, BOOL abModify, BOOL abAllowContinue = FALSE);
	static BOOL DeleteFailed(RegPath* pKey, LPCWSTR asName, BOOL abKey, BOOL abAllowContinue); // TRUE - continue
	static void MemoryAllocFailed(size_t nSize, int nMsgID = REM_CantAllocateMemory);
	static int Message(int nMsgID, DWORD nFlags = FMSG_WARNING|FMSG_MB_OK, int nBtnCount = 0, LPCTSTR asHelpTopic = NULL);
	static int Message(LPCTSTR asMsg, DWORD nFlags = FMSG_WARNING|FMSG_MB_OK, int nBtnCount = 0, LPCTSTR asHelpTopic = NULL);
	static int MessageFmt(int nFormatMsgID, LPCWSTR asArgument, DWORD nErrCode = 0, LPCTSTR asHelpTopic = NULL, DWORD nFlags = FMSG_WARNING|FMSG_MB_OK, int nBtnCount = 0);
#ifndef _UNICODE
	static int MessageFmt(int nFormatMsgID, LPCSTR asArgument, DWORD nErrCode = 0, LPCTSTR asHelpTopic = NULL, DWORD nFlags = FMSG_WARNING|FMSG_MB_OK, int nBtnCount = 0);
#endif
	void SetForceReload();
	RegItem* GetCurrentItem(const PluginPanelItem** ppPluginPaneItem = NULL);
	static BOOL ConfirmOverwriteFile(LPCWSTR asTargetFile, BOOL* pbAppendExisting, BOOL* pbUnicode);
	void PreClosePlugin();
	int ShowRegMenu(bool abForceImport, MRegistryBase* apTarget); // вернет -1, если плагин нужно закрыть

	//// static
	//static void InternalInvalidOp(LPCWSTR asFile, int nLine);
	
public:
	void CloseItems();
	//void UpdateTitle();
	BOOL AllowCacheFolder();
	BOOL LoadItems(BOOL abSilence, int OpMode);
	void CheckItemsLoaded();
	void FreeFindData(struct PluginPanelItem *PanelItem,int ItemsNumber);
	void EditKeyPermissions();
	BOOL ConfirmExport(
		BOOL abAllowHives, int* pnExportMode, BOOL* pbHiveAsSubkey, int ItemsNumber,
		LPCTSTR asDestPathOrFile, BOOL abDestFileSpecified, LPCTSTR asNameForLabel,
		wchar_t** ppwszDestPath, wchar_t* sDefaultName/*[MAX_PATH]*/, wchar_t* wsDefaultExt/*[5]*/,
		int nTitleMsgId = REExportDlgTitle, int nExportLabelMsgId = REExportItemLabel /*(ItemsNumber == 1) ? REExportItemLabel : REExportItemsLabel*/,
		int nCancelMsgId = REBtnCancel
		);
	BOOL ExportItems(PluginPanelItem *PanelItem,int ItemsNumber,int Move,const TCHAR *DestPath,int OpMode,const TCHAR** pszDestModified=NULL);
	BOOL ExportItems2Hive(RegFolder* expFolder,BOOL abHiveAsSubkey,int Move,LPCWSTR pwszDestPath,LPCWSTR pwszDefaultName,LPCWSTR pwszDefaultExt, const TCHAR** pszDestModified);
	void EditItem(bool abOnlyCurrent, bool abForceExport, bool abViewOnly, bool abRawData = false);
	BOOL EditNumber(wchar_t** pName, LPVOID pNumber, REGTYPE* pnDataType, DWORD cbSize, BOOL bNewValue = FALSE);
	BOOL EditString(wchar_t* pName, wchar_t** pText, REGTYPE nDataType, DWORD* cbSize, BOOL bNewValue = FALSE);
	int  EditValue(RegFolder* pFolder, RegItem *pItem, REGTYPE nNewValueType = 0);
	void EditValueRaw(RegFolder* pFolder, RegItem *pItem, bool abViewOnly);
	LONG ValueDataGet(RegFolder* pFolder, RegItem *pItem, LPBYTE* ppData, LPDWORD pcbSize, REGTYPE* pnDataType, BOOL abSilence = FALSE);
	LONG ValueDataSet(RegFolder* pFolder, RegItem *pItem, LPBYTE pData, DWORD cbSize, REGTYPE nDataType);
	LONG ValueDataSet(RegFolder* pFolder, LPCWSTR asValueName, LPBYTE pData, DWORD cbSize, REGTYPE nDataType);
	void NewItem();
	void RenameOrCopyItem(BOOL abCopyOnly = FALSE);
	void EditDescription();
	void JumpToRegedit();
	BOOL ConnectRemote(LPCWSTR asServer = NULL, LPCWSTR asLogin = NULL, LPCWSTR asPassword = NULL);
	void ShowBookmarks();
	RegFolder* PrepareExportKey(bool abOnlyCurrent, wchar_t *psDefaultName/*[nMaxDefaultLen]*/, int nMaxDefaultLen);
	RegFolder* PrepareExportKey(PluginPanelItem *PanelItem, int ItemsNumber, wchar_t *psDefaultName/*[nMaxDefaultLen]*/, int nMaxDefaultLen);
	void ChangeFarSorting(bool abFastAccess);
	BOOL RegTypeMsgId2RegType(DWORD nMsgId, REGTYPE* pnRegType);
	// Api wrappers
	void RedrawPanel(RegItem* pNewSel = NULL);
	void UpdatePanel(bool abResetSelection);
	void OnIdle();
	int  Compare(const struct PluginPanelItem *Item1, const struct PluginPanelItem *Item2, unsigned int Mode);
	BOOL DeleteItems(struct PluginPanelItem *PanelItem, int ItemsNumber);
	int  CreateSubkey(const TCHAR* aszSubkey, const TCHAR** pszCreated, int OpMode);
protected:
	BOOL mb_ForceReload;
	//BOOL mb_EnableKeyMonitoring;
	TCHAR ms_CreatedSubkeyBuffer[32768];
	//void FreeItems();
	//BOOL AddItem(LPCWSTR aszName, REGFILETIME aftModified, LPCWSTR asDesc, DWORD adwFileAttributes = FILE_ATTRIBUTE_DIRECTORY, DWORD anValueType = 0, DWORD anValueSize = 0);
	void LoadCurrentSortMode(int *pnSortMode, bool *pbReverseSort);
};

extern REPlugin *gpActivePlugin; // Устанавливается на время вызова из ФАР функций нашего плагина
