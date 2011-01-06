
/*
    Класс кеширования загруженных ключей
    Общий для всех экземпляров плагина
*/

#pragma once



class RegFolderCache
{
public:
	RegFolderCache();
	~RegFolderCache();
	
	//void AddRef(REPlugin* pPlugin);
	//void Release(REPlugin* pPlugin);
	
	RegFolder* GetFolder(RegPath* apKey, int OpMode);
	RegFolder* FindByPanelItems(struct PluginPanelItem *PanelItem);

	//REPlugin* Plugins[10];
	//void UpdateAllTitles();
	
protected:
	/* ** Instance variables ** */
	int mn_RefCount;
	int mn_ItemCount, mn_MaxItemCount;
	RegFolder **mp_Cache;
	void FreeItems();
};

//extern RegFolderCache *gpCache;
