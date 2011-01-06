
#include "PictureView.h"
#include "PictureView_Lang.h"
#include "PictureView_FileName.h"
#include "PictureView_Panel.h"

/*
	// Work mode
	bool mb_PanelExists;
	// Items
	int mn_MaxItemsCount;
	struct PluginPanelItem* mp_Items;
	int* mn_MaxItemSize;
	// Panel (or directory)
	struct PanelInfo *mp_Panel;
	wchar_t* ms_PanelDir;
*/

static struct PluginPanelItem sInvalidItem = {{0}};
const struct PluginPanelItem* PicViewPanel::Invalid = &sInvalidItem;

PicViewPanel::PicViewPanel()
{
	// Work mode
	mb_PanelExists = false;
	// Items
	mn_MaxItemsCount = 0;
	mp_Items = NULL;
	mn_MaxItemSize = NULL;
	// Panel (or directory)
	memset(&m_Panel, 0, sizeof(m_Panel));
	ms_PanelDir = NULL;
}

PicViewPanel::~PicViewPanel()
{
	for (int i = 0; i < mn_MaxItemsCount; i++) {
		SAFEFREE(mp_Items[i]);
	}
	SAFEFREE(mp_Items);
}

void PicViewPanel::ShowError(const wchar_t* pszMsg)
{
	_ASSERTE(pszMsg!=NULL);
	if (!pszMsg) pszMsg = L"";
	
	if (GetCurrentThreadId() == gnMainThreadId) {
		const wchar_t* szLines[3];
		szLines[0] = g_Plugin.pszPluginTitle;
		szLines[1] = pszMsg;
		szLines[2] = L"Source: PicViewPanel";
		g_StartupInfo.Message(g_StartupInfo.ModuleNumber, FMSG_WARNING|FMSG_MB_OK,
			NULL, szLines, 3, 0);
	} else {
		MessageBox(NULL, pszMsg, g_Plugin.pszPluginTitle, MB_SETFOREGROUND|MB_SYSTEMMODAL|MB_OK|MB_ICONSTOP);
	}
}

bool PicViewPanel::LoadPanelInfo()
{
	ms_PanelDir = NULL;
	ms_PanelFile = NULL;
	memset(&m_Panel, 0, sizeof(m_Panel));
	
	if (!g_StartupInfo.Control(INVALID_HANDLE_VALUE, FCTL_GETPANELINFO, 0, (LONG_PTR)&m_Panel)) {
		ShowError(L"FCTL_GETPANELINFO failed");
		return false;
	}
	
	if (m_Panel.ItemsNumber > 0)
	{
		if (size_t len = g_StartupInfo.Control(INVALID_HANDLE_VALUE, FCTL_GETPANELITEM, m_Panel.CurrentItem, 0))
		{
			if (PluginPanelItem* ppi = (PluginPanelItem*)malloc(len))
			{
				if (g_StartupInfo.Control(INVALID_HANDLE_VALUE, FCTL_GETPANELITEM, m_Panel.CurrentItem, (LONG_PTR)ppi))
				{
					ms_PanelFile = ppi->FindData.lpwszFileName;
				} else {
					ShowError(L"FCTL_GETPANELITEM(CurrentItem) failed");
					return false;
				}
				free(ppi);
			} else {
				ShowError(L"Can't allocate PluginPanelItem");
				return false;
			}
		} else {
			ShowError(L"FCTL_GETPANELITEM(CurrentItem).size failed");
			return false;
		}
	}
	
	// смотреть на (pi.Flags & PFLAGS_REALNAMES)
	if (size_t len = g_StartupInfo.Control(INVALID_HANDLE_VALUE, FCTL_GETPANELDIR, 0, 0))
	{
		if (wchar_t* pCurDir = (wchar_t*)malloc(len*2))
		{
			if (!g_StartupInfo.Control(INVALID_HANDLE_VALUE, FCTL_GETPANELDIR, (int)len, (LONG_PTR)pCurDir))
			{
				*pCurDir = 0;
			}
			ms_PanelDir = pCurDir;
			free(pCurDir);
		} else {
			ShowError(L"Can't allocate pCurDir");
			return false;
		}
	}
	
	return true;
}

bool PicViewPanel::InitListFromPanel()
{
	return false;
}

bool PicViewPanel::InitListWithFile(UnicodeFileName* asFile)
{
	return false;
}

// Когда панели нет
bool PicViewPanel::RetrieveAllDirFiles()
{
	return false;
}

bool PicViewPanel::MarkUnmarkFile(int aiRawIndex, enum PicViewPanel::MarkAction action)
{
	if (GetCurrentThreadId() == gnMainThreadId) {
		if (action == ema_Mark)
			g_Plugin.Image[0]->bSelected = true;
		else if (action == ema_Unmark)
			g_Plugin.Image[0]->bSelected = false;
		else if (action == ema_Switch)
			g_Plugin.Image[0]->bSelected = !g_Plugin.Image[0]->bSelected;

		// чтобы при выходе из плагина можно было обновить панель, показав выделенные вверху
		g_Plugin.SelectionChanged = true; TODO("Переделать в флаг");

		g_StartupInfo.Control(INVALID_HANDLE_VALUE, FCTL_SETSELECTION,
			g_Plugin.Image[0]->PanelItemRaw(), g_Plugin.Image[0]->bSelected);

		g_StartupInfo.Control(INVALID_HANDLE_VALUE, FCTL_REDRAWPANEL, 0, 0);
		
		TitleRepaint();
		SetEvent(g_Plugin.hSynchroDone);
		g_Plugin.FlagsWork &= ~(FW_MARK_FILE | FW_UNMARK_FILE);
		return true;
		
	} else {
		_ASSERT(FALSE);
	}
	
	return false;
}

// Когда панели не было
bool PicViewPanel::UpdatePanelDir()
{
	return false;
}

// Когда панели нет
void PicViewPanel::SortFolderFiles()
{
}

const wchar_t* PicViewPanel::GetCurrentItemName()
{
	// ppi->FindData.lpwszFileName
	return (const wchar_t*)ms_PanelFile;
}

int PicViewPanel::GetCurrentItemRawIdx()
{
	// индекс в mp_Items
	return m_Panel.CurrentItem;
}

const wchar_t* PicViewPanel::GetCurrentDir()
{
	return (const wchar_t*)ms_PanelDir;
}
