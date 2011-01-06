
#pragma once

enum PLUGINPANELITEMFLAGS_PV{
  PPIF__WASNOTCHECKED          = 0x0100,
  PPIF__APPROVED               = 0x0200,
  PPIF__INVALID                = 0x0400,
  PPIF__ALLMASK                = 0xFF00,
};

class PicViewPanel
{
protected:
	// Work mode
	bool mb_PanelExists;
	// Items
	int mn_MaxItemsCount;
	struct PluginPanelItem** mp_Items;
	int* mn_MaxItemSize;
	// Panel (or directory)
	struct PanelInfo m_Panel;     // FCTL_GETPANELINFO
	UnicodeFileName ms_PanelDir;  // FCTL_GETCURRENTDIRECTORY
	UnicodeFileName ms_PanelFile; // FCTL_GETPANELITEM(pi.CurrentItem)
public:
	static const struct PluginPanelItem* Invalid;
public:
	PicViewPanel();
	~PicViewPanel();
	
public:
	enum MarkAction {
		ema_Mark = 1,
		ema_Unmark = 2,
		ema_Switch = 3,
	};
	
public:
	bool LoadPanelInfo();
	bool InitListFromPanel();
	bool InitListWithFile(UnicodeFileName* asFile);
	bool RetrieveAllDirFiles(); // Когда панели нет
	bool MarkUnmarkFile(int aiRawIndex, enum MarkAction action);
	bool UpdatePanelDir();
	const struct PluginPanelItem* GetPanelItem(int aiRawIndex);
	const wchar_t* GetCurrentItemName(); // ppi->FindData.lpwszFileName
	const wchar_t* GetCurrentDir();
	int GetCurrentItemRawIdx(); // индекс в mp_Items
	
	void ShowError(const wchar_t* pszMsg);
	
protected:
	void SortFolderFiles(); // Когда панели нет
};
