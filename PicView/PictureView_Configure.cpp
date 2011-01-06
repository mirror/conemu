/**************************************************************************
Copyright (c) 2009 Skakov Pavel
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

#include "PictureView.h"
#include "headers/farkeys.hpp"
#include "PVDManager.h"
#include "PictureView_Lang.h"
#include "PictureView_Dlg.h"
#include "PictureView_Configure.h"

#define SETTEXT(item, text) lstrcpy(pbuf, text); item.PtrData = pbuf; pbuf += item.MaxLen = wcslen(pbuf) + 1
#define PRINTTEXT(item, format, text) g_FSF.sprintf(pbuf, format, text); item.PtrData = pbuf; pbuf += item.MaxLen = wcslen(pbuf) + 1
#define GetCheck(Index) DlgItem_GetCheck(g_StartupInfo, mh_Dlg, Index)

#ifdef _DEBUG
#define DBGOUTCFG(x) OutputDebugString(x)
#else
#define DBGOUTCFG(x)
#endif


class MPicViewConfig;
MPicViewConfig* gpCurConfig = NULL;


MPicViewConfig::MPicViewConfig()
	: MPicViewDlg(g_Plugin.pszPluginTitle, 55, 17, L"Settings")
{
	//gpCurConfig = this;
	
	iHookArc = 0; iHookQuickView = 0; iHookView = 0; iHookEdit = 0; iPrefix = 0;
	memset(&ModulesList, 0, sizeof(ModulesList));
	memset(&DisplayList, 0, sizeof(DisplayList));
	memset(&PagingKeys,  0, sizeof(PagingKeys));
	iCachingRP = 0; iCachingVP = 0; iTrayDisable = 0; iFullScreenStartup = 0;
	iLoopJump = 0; iMarkBySpace = 0; iAutoPaging = 0; iAutoPagingVK = 0; iAutoPagingSet = 0;
	iFreePosition = 0; iFullDirectDrawInit = 0; memset(&PagingKeys, 0, sizeof(PagingKeys));
	iOk = 0; iCancel = 0; iMain = 0; iDecoders = 0; iAdvanced = 0; iOSD = 0; iLOG = 0; iLastTabBtn = 0;
	iTitleTemplReset = iQTempl1 = iQTempl1Reset = iQTempl2 = iQTempl2Reset = iQTempl3 = iQTempl3Reset = 0;
	bDecoderPriorityChanged = false;
	nCornerCount = 0;
};

MPicViewConfig::~MPicViewConfig()
{
	//_ASSERTE(gpCurConfig == this);
	
	//if (gpCurConfig == this)
	//	gpCurConfig = NULL;
};

bool MPicViewConfig::CanExchange ( int Old, int New )
{
	if (!pModules)
		return false;
		
	if (!(pModules[New]->pData->nCurrentFlags & PVD_IP_SUPPORTEDTYPES)
		|| !(pModules[Old]->pData->nCurrentFlags & PVD_IP_SUPPORTEDTYPES)
		|| (pModules[New]->pData->nCurrentFlags & PVD_IP_DECODE)!=(pModules[Old]->pData->nCurrentFlags & PVD_IP_DECODE)
		/*|| !pModules[New]->pData->pPlugin
		|| !pModules[Old]->pData->pPlugin*/)
		return false;
		
	return true;
}

void MPicViewConfig::Exchange ( HANDLE hDlg, int Old, int New )
{
	_ASSERTE(Old>=0 && Old<nModulesCount);
	_ASSERTE(New>=0 && New<nModulesCount);
	FarListUpdate flu = {Old};
	PVDDecoderConfig *p = NULL;
	
	// Можно ли двигать приоритеры
	if (!CanExchange ( Old, New ))
		return;
	
	bDecoderPriorityChanged = true;

	g_StartupInfo.SendDlgMessage(hDlg, DM_ENABLEREDRAW, 0, 0);

	flu.Index = Old;
	flu.Item.Text = pModules[New]->pTitle;
	g_StartupInfo.SendDlgMessage(hDlg, DM_LISTUPDATE, iPvdDecoders, (LONG_PTR)&flu);

	flu.Index = New;
	flu.Item.Flags = LIF_SELECTED;
	flu.Item.Text = pModules[Old]->pTitle;
	g_StartupInfo.SendDlgMessage(hDlg, DM_LISTUPDATE, iPvdDecoders, (LONG_PTR)&flu);

	p = pModules[Old];
	pModules[Old] = pModules[New];
	pModules[New] = p;

	g_StartupInfo.SendDlgMessage(hDlg, DM_ENABLEREDRAW, 1, 0);
}

void MPicViewConfig::ActivateMain(HANDLE hDlg)
{
}
void MPicViewConfig::ActivateDecoders(HANDLE hDlg)
{
	g_StartupInfo.SendDlgMessage(hDlg, DM_SETFOCUS, iPvdDecoders, 0);
}
void MPicViewConfig::ActivateAdvanced(HANDLE hDlg)
{
}
void MPicViewConfig::ActivateOSD(HANDLE hDlg)
{
}
void MPicViewConfig::ActivateLOG(HANDLE hDlg)
{
	FarListInfo li = {0};
	g_StartupInfo.SendDlgMessage(hDlg, DM_LISTINFO, iLogList, (LONG_PTR)&li);
	LONG_PTR nLogCount = PVDManager::sWholeLog.size();
	int nCurCount = li.ItemsNumber;
	if (PVDManager::bWholeLogChanged || nCurCount != nLogCount)
	{
		int n = 0;
		FarListInsert fli = {0};
		std::vector<wchar_t*>::reverse_iterator iwc = PVDManager::sWholeLog.rbegin();
		while (nCurCount < nLogCount && iwc != PVDManager::sWholeLog.rend()) {
			fli.Index = (n++);
			fli.Item.Text = *iwc;
			g_StartupInfo.SendDlgMessage(hDlg, DM_LISTINSERT, iLogList, (LONG_PTR)&fli);
			nCurCount++; iwc++;
		}
		
		FarListUpdate flu = {0};
		while (n < nCurCount && iwc != PVDManager::sWholeLog.rend()) {
			flu.Index = (n++);
			flu.Item.Text = *iwc;
			g_StartupInfo.SendDlgMessage(hDlg, DM_LISTUPDATE, iLogList, (LONG_PTR)&flu);
			iwc++;
		}
	}
	
	g_StartupInfo.SendDlgMessage(hDlg, DM_SETFOCUS, iLogList, 0);
}

void MPicViewConfig::UpdateModulesHotkeys(HANDLE hDlg)
{
	static wchar_t szModulesBottom[64] = {0};
	
	if (pModules) {
		//lstrcpy(szModulesBottom, L"F4, Ctrl-Up, Ctrl-Down");
		
		int pos = (int)g_StartupInfo.SendDlgMessage(hDlg, DM_LISTGETCURPOS, iPvdDecoders, 0);
		
		if (pos >= 0 && pos < nModulesCount) {
			lstrcpy(szModulesBottom, L"F3, F4");

			if (pos > 0) {
				if (CanExchange ( pos, pos-1 )) {
					if (szModulesBottom[0]) lstrcat(szModulesBottom, L", ");
					lstrcat(szModulesBottom, L"Ctrl-Up");
				}
			}
			if (pos < (nModulesCount - 1)) {
				if (CanExchange ( pos, pos+1 )) {
					if (szModulesBottom[0]) lstrcat(szModulesBottom, L", ");
					lstrcat(szModulesBottom, L"Ctrl-Down");
				}
			}
		}
	}
	
	FarListTitles titles;
	titles.Title = GetMsg(MIPVDDecoderList);
	titles.TitleLen = lstrlen(titles.Title);
	titles.Bottom = szModulesBottom; //GetMsg(MIPVDDecoderKeys);
	titles.BottomLen = lstrlen(titles.Bottom);
	g_StartupInfo.SendDlgMessage(hDlg, DM_LISTSETTITLES, iPvdDecoders, (LONG_PTR)&titles);
}

LONG_PTR MPicViewConfig::ConfigDlgProc(HANDLE hDlg, int Msg, int Param1, LONG_PTR Param2)
{
	//_ASSERTE(gpCurConfig);

	_ASSERTE(mh_Dlg == hDlg);

	//#ifdef _DEBUG	
	//wchar_t szDbg[128]; wsprintfW(szDbg, L"DlgMsg(%i, %i, %i)\n", Msg, Param1, Param2);
	//OutputDebugString(szDbg);
	//#endif

	if (Msg == DN_INITDIALOG) {
		if ((g_Plugin.FlagsWork & FW_SHOW_MODULES) != 0) {
			FarListPos pos = {0}; pos.TopPos = -1;
			for (int i=0; i<nModulesCount; i++) {
				if (pModules[i]->pData == g_Plugin.Image[0]->Decoder->mp_Data) {
					pos.SelectPos = i; break;
				}
			}
			g_StartupInfo.SendDlgMessage(hDlg, DM_LISTSETCURPOS, iPvdDecoders, (LONG_PTR)&pos);
		}

		UpdateModulesHotkeys(hDlg);
		
		//{
		//	FarListPos pos = {0}; pos.TopPos = -1;
		//	pos.SelectPos = (g_Plugin.nAutoPagingVK == VK_SCROLL) ? 0 : ((g_Plugin.nAutoPagingVK == VK_CAPITAL) ? 1 : 2);
		//	g_StartupInfo.SendDlgMessage(hDlg, DM_LISTSETCURPOS, iAutoPagingVK, (LONG_PTR)&pos);
		//}
		return TRUE;
	}
	
    if (Msg==DN_BTNCLICK && Param1>=iMain && Param1<=iLastTabBtn) {
    	int nNewPage = Param1 - iMain;
    	if (nNewPage == nActivePage)
    		return TRUE; // Она уже активирована
    
    	g_StartupInfo.SendDlgMessage(hDlg, DM_ENABLEREDRAW, FALSE, 0);
    	for (int j=0; j<nNextPage; j++) {
    		int nShow = (j == nNewPage) ? 1 : 0;
	    	for (int i=nPageStart[j]; i<nPageStart[j+1]; i++)
				g_StartupInfo.SendDlgMessage(hDlg, DM_SHOWITEM, i, nShow);
        }
        
        if (Param1 == iMain)
        	ActivateMain(hDlg);
        else if (Param1 == iDecoders)
        	ActivateDecoders(hDlg);
        else if (Param1 == iAdvanced)
        	ActivateAdvanced(hDlg);
        else if (Param1 == iOSD)
        	ActivateOSD(hDlg);
        else if (Param1 == iLOG)
        	ActivateLOG(hDlg);

        g_StartupInfo.SendDlgMessage(hDlg, DM_ENABLEREDRAW, TRUE, 0);
        
        nActivePage = nNewPage;
        bPageActivated[nNewPage] = TRUE;
        
        return TRUE; // Обработали
    }
    
    if (Msg==DN_BTNCLICK && Param1==iChooseColor) {
		g_StartupInfo.SendDlgMessage(hDlg, DM_GETTEXTPTR, iChooseColor - 1, (LONG_PTR)pbuf);
		chc.rgbResult = wcstoul(pbuf + 2, NULL, 0x10);
		
		if (ChooseColor(&chc)) {
			wchar_t *psz = (wchar_t*)DialogItems[iChooseColor - 1].PtrData;
			g_FSF.sprintf(psz, L"%06X", chc.rgbResult);
			g_StartupInfo.SendDlgMessage(hDlg, DM_SETTEXTPTR, iChooseColor - 1, (LONG_PTR)psz);
		}

    	return TRUE;
    }
	if (Msg==DN_BTNCLICK && Param1==iChooseQVColor) {
		g_StartupInfo.SendDlgMessage(hDlg, DM_GETTEXTPTR, iChooseQVColor - 1, (LONG_PTR)pbuf);
		chc.rgbResult = wcstoul(pbuf + 2, NULL, 0x10);

		if (ChooseColor(&chc)) {
			wchar_t *psz = (wchar_t*)DialogItems[iChooseQVColor - 1].PtrData;
			g_FSF.sprintf(psz, L"%06X", chc.rgbResult);
			g_StartupInfo.SendDlgMessage(hDlg, DM_SETTEXTPTR, iChooseQVColor - 1, (LONG_PTR)psz);
		}

		return TRUE;
	}

	//if (Msg==DN_BTNCLICK && 
	//	(Param1==iUsePVDExt || Param1==iUseGFLExt || Param1==iUseGDIPlusExt))
	//{
	//	BOOL lbCheck = (BOOL)GetCheck(Param1);
	//	g_StartupInfo.SendDlgMessage(hDlg, DM_ENABLE, Param1+1, lbCheck);
	//	return TRUE;
	//}

	if (Msg==DN_HELP) {
		static wchar_t *HelpTopics[]={L"Settings",L"Modules",L"AdvSettings",L"OSD",L"LOG"};
		if (nActivePage>=0 && nActivePage<sizeofarray(HelpTopics))
			return (LONG_PTR)(HelpTopics[nActivePage]);
		else
			return NULL;
	}

	if (Msg==DN_BTNCLICK && Param1==iIgnoredExtBtn) {
		g_StartupInfo.SendDlgMessage(hDlg, DM_SETTEXTPTR, iIgnoredExt, (LONG_PTR)DEFAULT_INGORED_EXT);
		g_StartupInfo.SendDlgMessage(hDlg, DM_SETFOCUS, iIgnoredExt, 0);
		return TRUE;
	}
	
	if (Msg==DN_BTNCLICK && Param1==iTitleTemplReset) {
		g_StartupInfo.SendDlgMessage(hDlg, DM_SETTEXTPTR, iTitleTemplReset-1, (LONG_PTR)g_DefaultTitleTemplate);
		g_StartupInfo.SendDlgMessage(hDlg, DM_SETFOCUS, iTitleTemplReset-1, 0);
		return TRUE;
	}

	if (Msg==DN_BTNCLICK && Param1==iQTempl1Reset) {
		g_StartupInfo.SendDlgMessage(hDlg, DM_SETTEXTPTR, iQTempl1, (LONG_PTR)g_DefaultQViewTemplate1);
		g_StartupInfo.SendDlgMessage(hDlg, DM_SETFOCUS, iQTempl1, 0);
		return TRUE;
	}
	if (Msg==DN_BTNCLICK && Param1==iQTempl2Reset) {
		g_StartupInfo.SendDlgMessage(hDlg, DM_SETTEXTPTR, iQTempl2, (LONG_PTR)g_DefaultQViewTemplate2);
		g_StartupInfo.SendDlgMessage(hDlg, DM_SETFOCUS, iQTempl2, 0);
		return TRUE;
	}
	if (Msg==DN_BTNCLICK && Param1==iQTempl3Reset) {
		g_StartupInfo.SendDlgMessage(hDlg, DM_SETTEXTPTR, iQTempl3, (LONG_PTR)g_DefaultQViewTemplate3);
		g_StartupInfo.SendDlgMessage(hDlg, DM_SETFOCUS, iQTempl3, 0);
		return TRUE;
	}

	if (Param1 == iPvdDecoders)
	{
		#ifdef _DEBUG
		//WCHAR szDbg[255]; wsprintfW(szDbg, L"List message: 0x%08X (%i)\n", Msg, Msg); OutputDebugString(szDbg);
		#endif
		
		if (Msg == DM_CLOSE)
			return TRUE;

		BOOL lbDblClick = FALSE;
		if (Msg == DN_MOUSECLICK) {
			UpdateModulesHotkeys(hDlg);
			
			MOUSE_EVENT_RECORD* pEvent = (MOUSE_EVENT_RECORD*)Param2;
			lbDblClick = (pEvent->dwEventFlags & DOUBLE_CLICK) == DOUBLE_CLICK;
			if (!lbDblClick) {
				g_StartupInfo.DefDlgProc(hDlg, Msg, Param1, Param2);
				return TRUE;
			}
		}
		if (lbDblClick || (Msg == DN_KEY && Param2 == KEY_F4))
		{
			TODO("Настройка параметров декодера");
			int pos = (int)g_StartupInfo.SendDlgMessage(hDlg, DM_LISTGETCURPOS, iPvdDecoders, 0);
			if (pos >= 0) {
				if (pos >= 0 && pos < nModulesCount) {
					PVDDecoderConfig *p = pModules[pos];
					if (p->Configure()) {
						FarListUpdate flu = {pos};
						flu.Item.Flags = LIF_SELECTED;
						flu.Item.Text = p->pTitle;
						g_StartupInfo.SendDlgMessage(hDlg, DM_LISTUPDATE, iPvdDecoders, (LONG_PTR)&flu);
						
						// Если двигали приоритеты декодеров, то при OK в настройке декодера
						if (bDecoderPriorityChanged) {
							ApplyDecoders(); // Сразу сохранить в реестр
							bDecoderPriorityChanged = false;
						}
					}
				}
			}
			return TRUE;
		}
		if (Msg == DN_KEY && Param2 == KEY_F3)
		{
			LONG_PTR pos = g_StartupInfo.SendDlgMessage(hDlg, DM_LISTGETCURPOS, iPvdDecoders, 0);
			if (pos >= 0) {
				if (pos >= 0 && pos < nModulesCount) {
					PVDDecoderConfig *p = pModules[pos];
					p->About();
				}
			}
			return TRUE;
		}
		if (Msg == DN_KEY && ((Param2 & (KEY_CTRL|KEY_RCTRL)) 
			&& ( ((Param2&KEY_END_SKEY) == KEY_UP) || ((Param2&KEY_END_SKEY) == KEY_DOWN))))
		{
			// Изменение приоритета 'на лету'
			int pos = (int)g_StartupInfo.SendDlgMessage(hDlg, DM_LISTGETCURPOS, iPvdDecoders, 0);
			bool lbMoveUp = ((Param2&KEY_END_SKEY) == KEY_UP);
			if (lbMoveUp && pos > 0) {
				Exchange ( hDlg, pos, pos-1 );
				//ApplyDecoders(); // порядок сохранять в реестр только после OK
			} else if (!lbMoveUp && pos < (nModulesCount - 1)) {
				Exchange ( hDlg, pos, pos+1 );
				//ApplyDecoders(); // порядок сохранять в реестр только после OK
			}
			
			UpdateModulesHotkeys(hDlg);
			
			return TRUE;
		}
		
		if (Msg == DN_LISTCHANGE)
		{
			UpdateModulesHotkeys(hDlg);
		}
		
		if (Msg == DN_KEY && ((Param2 & (KEY_ALT|KEY_RALT))==(Param2 & KEY_CTRLMASK))
			&& ((Param2&KEY_END_SKEY)>=(DWORD)'1') && ((Param2&KEY_END_SKEY)<=(DWORD)'5')
		   )
		{
			// Вывести фокус из контрола, иначе хоткеи не отрабатывают
			g_StartupInfo.SendDlgMessage(hDlg, DM_SETFOCUS, iDecoders, 0);
		}
	}

	if (Param1 == iLogList)
	{
		if (Msg == DN_MOUSECLICK)
			return TRUE;
			
		if (Msg == DN_KEY && ((Param2 & (KEY_ALT|KEY_RALT))==(Param2 & KEY_CTRLMASK))
			&& ((Param2&KEY_END_SKEY)>=(DWORD)'1') && ((Param2&KEY_END_SKEY)<=(DWORD)'5')
		   )
		{
			// Вывести фокус из контрола, иначе хоткеи не отрабатывают
			g_StartupInfo.SendDlgMessage(hDlg, DM_SETFOCUS, iLOG, 0);
		}
	}

	if (Msg == DN_BTNCLICK && (Param1 == (iCMYK2RGBstart+1) || Param1 == (iCMYK2RGBstart+2)))
	{
		if (!g_Plugin.InitCMYK(TRUE)) {
			g_StartupInfo.SendDlgMessage(hDlg, DM_SETCHECK, iCMYK2RGBstart, BSTATE_CHECKED);
			g_StartupInfo.SendDlgMessage(hDlg, DM_SETFOCUS, iCMYK2RGBstart, 0);
			g_StartupInfo.SendDlgMessage(hDlg, DM_ENABLE, iCMYK2RGBstart+1, 0);
			g_StartupInfo.SendDlgMessage(hDlg, DM_ENABLE, iCMYK2RGBstart+2, 0);
			return TRUE;
		}
	}

	return g_StartupInfo.DefDlgProc(hDlg, Msg, Param1, Param2);
};

int MPicViewConfig::InitMain() // Page1
{
	int Y = 2, X;
	LPCWSTR pszText = NULL;
	
	pszText = GetMsg(MIHook);
	addFarDialogItem(
		DI_TEXT,      5, Y, 0, 0, 0, 0, 0, 0, pszText);
	X = 7+lstrlen(pszText);
	iPrefix = addFarDialogItem(
		DI_FIXEDIT, X,  Y++, X + 6, 0, 0, 0, 0, 0, g_Plugin.sHookPrefix);
	iHookArc = addFarDialogItem(
		DI_CHECKBOX,  6, Y, 0, 0, 0, g_Plugin.bHookArc, 0, 0, GetMsg(MIHookArc));
	iHookQuickView = addFarDialogItem(
		DI_CHECKBOX,  6, Y+1, 0, 0, 0, g_Plugin.bHookQuickView, 0, 0, GetMsg(MIHookQuickView));
	LPCWSTR pszView = GetMsg(MIHookView);
	LPCWSTR pszEdit = GetMsg(MIHookEdit);
	int nMaxLen = max(lstrlenW(pszView),lstrlenW(pszEdit))+4;
	iHookView = addFarDialogItem(
		DI_CHECKBOX, 27, Y, 0, 0, 0, g_Plugin.bHookView, 0, 0, pszView);
	iHookCtrlShiftF3 = addFarDialogItem(
		DI_CHECKBOX, 27+nMaxLen, Y, 0, 0, 0, g_Plugin.bHookCtrlShiftF3, 0, 0, GetMsg(MIHookCtrlShiftF3));
	iHookEdit = addFarDialogItem(
		DI_CHECKBOX, 27, Y+1, 0, 0, 0, g_Plugin.bHookEdit, 0, 0, pszEdit);
	iHookCtrlShiftF4 = addFarDialogItem(
		DI_CHECKBOX, 27+nMaxLen, Y+1, 0, 0, 0, g_Plugin.bHookCtrlShiftF4, 0, 0, GetMsg(MIHookCtrlShiftF4));
	Y += 3;
	
	addFarDialogItem(
		DI_TEXT,     5,  Y++, 0, 0, 0, 0, 0, 0, GetMsg(MIAutoCaching));
	pszText = GetMsg(MIAutoCachingRP);
	iCachingRP = addFarDialogItem(
		DI_CHECKBOX, 6,  Y, 0, 0, 0, g_Plugin.bCachingRP, 0, 0, pszText);
	iCachingVP = addFarDialogItem(
		DI_CHECKBOX, 12 + lstrlen(pszText),  Y++, 0, 0, 0, g_Plugin.bCachingVP, 0, 0, GetMsg(MIAutoCachingVP));
	Y++;

	addFarDialogItem(
		DI_TEXT,     5, Y++, 0, 0, 0, 0, 0, 0, GetMsg(MIMiscSettings));
	iTrayDisable = addFarDialogItem(
		DI_CHECKBOX, 6, Y++, 0, 0, 0, g_Plugin.bTrayDisable, 0, 0, GetMsg(MITrayDisable));
	iFullScreenStartup = addFarDialogItem(
		DI_CHECKBOX, 6, Y++, 0, 0, 0, g_Plugin.bFullScreenStartup, 0, 0, GetMsg(MIFullscreen));
	iLoopJump = addFarDialogItem(
		DI_CHECKBOX, 6, Y++, 0, 0, 0, g_Plugin.bLoopJump, 0, 0, GetMsg(MILoopJump));
	iMarkBySpace = addFarDialogItem(
		DI_CHECKBOX, 6, Y++, 0, 0, 0, g_Plugin.bMarkBySpace, 0, 0, GetMsg(MIMarkBySpace));
	iFreePosition = addFarDialogItem(
		DI_CHECKBOX, 6, Y++, 0, 0, 0, g_Plugin.bFreePosition, 0, 0, GetMsg(MIFreePosition));
	iFullDirectDrawInit = addFarDialogItem(
		DI_CHECKBOX, 6, Y++, 0, 0, 0, g_Plugin.bFullDisplayReInit, 0, 0, GetMsg(MIFullDirectDrawInit));
	X = 6; pszText = GetMsg(MIAutoPagingKey);
	iAutoPaging = addFarDialogItem(
		DI_CHECKBOX, 6, Y, 0, 0, 0, g_Plugin.bAutoPaging, 0, 0, pszText);
	X += lstrlen(pszText)+5;
	iAutoPagingVK = addFarDialogItem(
		DI_COMBOBOX, X, Y, X+10, Y, 0, 0, DIF_DROPDOWNLIST, 0, 0);
	PagingKeys.ItemsNumber = 3;
	PagingKeys.Items = (FarListItem*)calloc(3*sizeof(FarListItem),1);
	PagingKeys.Items[0].Text = L"ScrollLock";
	PagingKeys.Items[1].Text = L"CapsLock";
	PagingKeys.Items[2].Text = L"NumLock";
	PagingKeys.Items[(g_Plugin.nAutoPagingVK == VK_SCROLL) ? 0 : ((g_Plugin.nAutoPagingVK == VK_CAPITAL) ? 1 : 2)].Flags = LIF_SELECTED;
	DialogItems[iAutoPagingVK].Param.ListItems = &PagingKeys;
	X += 13;
	iAutoPagingSet = addFarDialogItem(
		DI_CHECKBOX, X, Y, 0, 0, 0, g_Plugin.bAutoPagingSet, 0, 0, GetMsg(MIAutoPagingSet));
	Y += 2;

	addFarDialogItem(
		DI_TEXT,        5, Y++, 0, 0, 0, 0, 0, 0, GetMsg(MICMYKtoRGBprocessing));
	pszText = GetMsg(MICMYKtoRGBFast);
	X = 6;
	iCMYK2RGBstart = addFarDialogItem(
		DI_RADIOBUTTON, X, Y, 0, 0, 0, g_Plugin.uCMYK2RGB == PVD_CMYK2RGB_FAST, DIF_GROUP, 0, pszText);
	X += lstrlen(pszText)+5;
	pszText = GetMsg(MICMYKtoRGBApproximate);
	addFarDialogItem(
		DI_RADIOBUTTON, X, Y, 0, 0, 0, g_Plugin.uCMYK2RGB == PVD_CMYK2RGB_APPROX, 
		(!g_Plugin.bCMYKinitialized || g_Plugin.pCMYKpalette) ? 0 : DIF_DISABLE, 0, pszText);
	X += lstrlen(pszText)+5;
	pszText = GetMsg(MICMYKtoRGBPrecise);
	addFarDialogItem(
		DI_RADIOBUTTON, X, Y, 0, 0, 0, g_Plugin.uCMYK2RGB == PVD_CMYK2RGB_PRECISE, 
		(!g_Plugin.bCMYKinitialized || g_Plugin.pCMYKpalette) ? 0 : DIF_DISABLE, 0, pszText);
	Y++;
		
	return Y;
};

int MPicViewConfig::InitDecoders() // Page2
{
	int Y = 2;
	LPCTSTR pszBtn = NULL;
	
	//addFarDialogItem(
	//	DI_TEXT,      5, Y++, 0, 0, 0, 0, 0, 0, GetMsg(MIPrivilegedDecoder));
	//Y++;

	iPvdDecoders = addFarDialogItem(
		DI_LISTBOX,   5, Y, DialogWidth + 4, Y+13, 0, 0, 0, 0, GetMsg(MIPVDDecoderList));
	nModulesCount = (int)PVDManager::Plugins.size();
	if (nModulesCount > 0) {
		ModulesList.ItemsNumber = nModulesCount;
		ModulesList.Items = (FarListItem*)calloc(nModulesCount*sizeof(FarListItem),1);

		pModules = (PVDDecoderConfig**)calloc(sizeof(LPVOID),nModulesCount);
		for (int i = 0; i < nModulesCount; i++) {
			pModules[i] = new PVDDecoderConfig(/*PVDManager::Plugins[i]->pPlugin,*/ PVDManager::Plugins[i]);
			ModulesList.Items[i].Text = pModules[i]->pTitle;
		}
	} else {
		pModules = NULL;
	}
	DialogItems[iPvdDecoders].Param.ListItems = &ModulesList;
	Y+=14;
	
	addFarDialogItem(
		DI_TEXT, 5, Y++, 0, 0, 0, 0, DIF_BOXCOLOR | DIF_SEPARATOR, 0);
	pszBtn = GetMsg(MIPVDActiveDisplayModule);
	addFarDialogItem(
		DI_TEXT, 5, Y, 0, 0, 0, 0, 0, 0, pszBtn);
	nDisplayCount = (int)PVDManager::Displays.size();
	if (nDisplayCount>0) {
		DisplayList.ItemsNumber = nDisplayCount;
		DisplayList.Items = (FarListItem*)calloc(nDisplayCount*sizeof(FarListItem),1);
		ppDisplays = (ModuleData**)calloc(nDisplayCount,sizeof(ModuleData*));
		for (int i = 0; i < nDisplayCount; i++) {
			ModuleData *pDisp = PVDManager::Displays[i];
			ppDisplays[i] = pDisp;
			DisplayList.Items[i].Text = pDisp->pPlugin ? pDisp->pPlugin->pName : pDisp->pModuleFileName;
			DisplayList.Items[i].Flags = 0;
			if (!(pDisp->nCurrentFlags & PVD_IP_DISPLAY)
				|| (pDisp->nCurrentFlags & PVD_IP_PRIVATE))
				DisplayList.Items[i].Flags |= LIF_GRAYED;
			else if (pDisp->nActiveFlags & PVD_IP_DISPLAY)
				DisplayList.Items[i].Flags |= LIF_SELECTED;
		}
	} else {
		ppDisplays = NULL;
	}
	iPvdDisplay = addFarDialogItem(
		DI_COMBOBOX, 6 + lstrlen(pszBtn), Y, DialogWidth + 4, Y, 0, 0, 
		DIF_DROPDOWNLIST /*| (g_Plugin.hWnd ? DIF_DISABLE : 0)*/, 0, 0);
	DialogItems[iPvdDisplay].Param.ListItems = &DisplayList;
	Y++;

	addFarDialogItem(
		DI_TEXT, 5, Y++, 0, 0, 0, 0, DIF_BOXCOLOR | DIF_SEPARATOR, 0, GetMsg(MIIgnoredExtensions));
	pszBtn = GetMsg(MIIgnoredExtensionsBtn);
	int nLen = (int)_tcslen(pszBtn)+6;
	iIgnoredExt = addFarDialogItem(
		DI_EDIT,     5, Y, DialogWidth - nLen + 5, 0, 0, 0, 0, 0, 
		g_Plugin.sIgnoredExt, sizeofarray(g_Plugin.sIgnoredExt));
	iIgnoredExtBtn = addFarDialogItem(
		DI_BUTTON,   DialogWidth - nLen + 7, Y++, 0, 0, 0, 0, 0, 0, pszBtn);

	//Y++;
	return Y;
};

int MPicViewConfig::InitAdvanced() // Page3
{
	int Y = 2;

	TCHAR *pszNumberMask = pbuf; lstrcpy(pszNumberMask, L"#####"); pbuf += lstrlen(pbuf)+1;
	TCHAR *pszColorMask = pbuf;  lstrcpy(pszColorMask, L"0xXXXXXX"); pbuf += lstrlen(pbuf)+1;
	
	iAZGroup = addFarDialogItem(
		DI_TEXT,     5,  Y++, 0, 0, 0, 0, 0, 0, GetMsg(MIAutoZoomSettings));
	addFarDialogItem(
		DI_CHECKBOX, 6,  Y++, 0, 0, 0, g_Plugin.bAutoZoom, 0, 0, GetMsg(MIAutoZoomOnImageLoad));
	addFarDialogItem(
		DI_CHECKBOX, 6,  Y, 0, 0, 0, g_Plugin.bAutoZoomMin, 0, 0, GetMsg(MIAutoZoomLimitMin));
	addFarDialogItem(
		DI_FIXEDIT, 35,  Y, 35 + 4, 0, 0, (DWORD_PTR)pszNumberMask, DIF_MASKEDIT, 0);
	addFarDialogItem(
		DI_TEXT,    41,  Y++, 0, 0, 0, 0, 0, 0, L"%");
	addFarDialogItem(
		DI_CHECKBOX, 6,  Y, 0, 0, 0, g_Plugin.bAutoZoomMax, 0, 0, GetMsg(MIAutoZoomLimitMax));
	addFarDialogItem(
		DI_FIXEDIT, 35,  Y, 35 + 4, 0, 0, (DWORD_PTR)pszNumberMask, DIF_MASKEDIT, 0);
	addFarDialogItem(
		DI_TEXT,    41,  Y++, 0, 0, 0, 0, 0, 0, L"%");
	addFarDialogItem(
		DI_CHECKBOX, 6,  Y++, 0, 0, 0, g_Plugin.bKeepZoomAndPosBetweenFiles, 0, 0, GetMsg(MIKeepZoomBetweenFiles));
	iKeepCorner = addFarDialogItem(
		DI_COMBOBOX, 10, Y, 40, Y, 0, 0, DIF_DROPDOWNLIST|DIF_LISTNOAMPERSAND, 0, 0);
	{
		nCornerCount = 3; // От центра, от краев, только масштаб
		CornerList.ItemsNumber = nCornerCount;
		CornerList.Items = (FarListItem*)calloc(nCornerCount*sizeof(FarListItem),1);
		for (int i = 0; i < nCornerCount; i++)
		{
			CornerList.Items[i].Text = GetMsg(MIKeepPosCenter+i);
			CornerList.Items[i].Flags = (g_Plugin.nKeepPanCorner == i) ? LIF_SELECTED : 0;
		}
		DialogItems[iKeepCorner].Param.ListItems = &CornerList;
	}
	Y+=2;
		
	PRINTTEXT(DialogItems[iAZGroup + 3], L"%5i", MulDivU32R(g_Plugin.AutoZoomMin, 100, 0x10000));
	PRINTTEXT(DialogItems[iAZGroup + 6], L"%5i", MulDivU32R(g_Plugin.AutoZoomMax, 100, 0x10000));
		

	iKSZGroup = addFarDialogItem(
		DI_TEXT,     5,  Y++, 0, 0, 0, 0, 0, 0, GetMsg(MIKeyboardScrollingAndZooming));
	addFarDialogItem(
		DI_CHECKBOX, 6,  Y, 0, 0, 0, g_Plugin.bSmoothScrolling, 0, 0, GetMsg(MISmoothScrolling));
	addFarDialogItem(
		DI_FIXEDIT, 35,  Y++, 35 + 4, 0, 0, (DWORD_PTR)pszNumberMask, DIF_MASKEDIT, 0);
	addFarDialogItem(
		DI_CHECKBOX, 6,  Y, 0, 0, 0, g_Plugin.bSmoothZooming, 0, 0, GetMsg(MISmoothZooming));
	addFarDialogItem(
		DI_FIXEDIT, 35,  Y++, 35 + 4, 0, 0, (DWORD_PTR)pszNumberMask, DIF_MASKEDIT, 0);
	Y++;

	PRINTTEXT(DialogItems[iKSZGroup + 2], L"%5i", g_Plugin.SmoothScrollingStep);
	PRINTTEXT(DialogItems[iKSZGroup + 4], L"%5i", g_Plugin.SmoothZoomingStep);
		
	iMZGroup = addFarDialogItem(
		DI_TEXT,        5, Y++, 0, 0, 0, 0, 0, 0, GetMsg(MIMouseZoomMode));
	addFarDialogItem(
		DI_RADIOBUTTON, 6, Y++, 0, 0, 0, g_Plugin.MouseZoomMode == 0, DIF_GROUP, 0, GetMsg(MIMouseZoomSimple));
	addFarDialogItem(
		DI_RADIOBUTTON, 6, Y++, 0, 0, 0, g_Plugin.MouseZoomMode == 1, 0, 0, GetMsg(MIMouseZoomCenter));
	addFarDialogItem(
		DI_RADIOBUTTON, 6, Y++, 0, 0, 0, g_Plugin.MouseZoomMode == 2, 0, 0, GetMsg(MIMouseZoomHoldPos));
	Y++;

	LPCTSTR pszText = GetMsg(MIBackgroundColor);
	LPCTSTR pszBtn = GetMsg(MISelect);
	int nX = 5;
	addFarDialogItem(DI_TEXT, nX, Y, 0, 0, 0, 0, 0, 0, pszText);
	nX += lstrlen(pszText);
	addFarDialogItem(DI_FIXEDIT, nX, Y, nX + 7, 0, 0, (DWORD_PTR)pszColorMask, DIF_MASKEDIT, 0);
	nX += 9;
	pszText = GetMsg(MIBackgroundQView);
	iChooseColor = addFarDialogItem(DI_BUTTON, nX, Y, 0, 0, 0, 0, 0, 0, pszBtn);
	nX += lstrlen(pszBtn)+6;
	addFarDialogItem(DI_TEXT, nX, Y, 0, 0, 0, 0, 0, 0, pszText);
	nX += lstrlen(pszText);
	addFarDialogItem(DI_FIXEDIT, nX, Y, nX + 7, 0, 0, (DWORD_PTR)pszColorMask, DIF_MASKEDIT, 0);
	nX += 9;
	iChooseQVColor = addFarDialogItem(DI_BUTTON, nX, Y++, 0, 0, 0, 0, 0, 0, pszBtn);
	Y++;

	PRINTTEXT(DialogItems[iChooseColor - 1], L"%06X", g_Plugin.nBackColor & 0xFFFFFF);
	_ASSERTE(g_Plugin.nQVBackColor > 0xFFFF);
	PRINTTEXT(DialogItems[iChooseQVColor - 1], L"%06X", g_Plugin.nQVBackColor & 0xFFFFFF);



	return Y;
};

int MPicViewConfig::InitOSD() // Page4
{
	int Y = 2;
	LPCTSTR pszBtn = GetMsg(MITitleTemplateReset);
	int nBtnLen = (int)_tcslen(pszBtn)+6;
	LPCTSTR pszText1 = GetMsg(MIQView1);
	LPCTSTR pszText2 = GetMsg(MIQView2);
	LPCTSTR pszText3 = GetMsg(MIQView3);
	int n, nTextLen = lstrlen(pszText1);
	if ((n = lstrlen(pszText2)) > nTextLen) nTextLen = n;
	if ((n = lstrlen(pszText3)) > nTextLen) nTextLen = n;
	nTextLen++;

	Y+=2;
	
	// Шаблон заголовка
	addFarDialogItem(DI_TEXT, 5, Y++, 0, 0, 0, 0, DIF_BOXCOLOR | DIF_SEPARATOR, 0, GetMsg(MITitleTemplate));
	iTitleTemplate = addFarDialogItem(DI_EDIT, 5, Y, DialogWidth + 5 - nBtnLen, 0, 0, 0, 0, 0, g_TitleTemplate);
	iTitleTemplReset = addFarDialogItem(DI_BUTTON, DialogWidth - nBtnLen + 7, Y, 0, 0, 0, 0, 0, 0, pszBtn);
	Y+=2;

	addFarDialogItem(DI_TEXT, 5, Y++, 0, 0, 0, 0, DIF_BOXCOLOR | DIF_SEPARATOR, 0, GetMsg(MIQViewTemplate));
	// Шаблоны 3-х строк информации для QView
	addFarDialogItem(DI_TEXT, 5, Y, 0, 0, 0, 0, 0, 0, pszText1);
	iQTempl1 = addFarDialogItem(DI_EDIT, 5+nTextLen, Y, DialogWidth + 5 - nBtnLen, 0, 0, 0, 0, 0, g_QViewTemplate1);
	iQTempl1Reset = addFarDialogItem(DI_BUTTON, DialogWidth - nBtnLen + 7, Y, 0, 0, 0, 0, 0, 0, pszBtn);
	Y++;
	addFarDialogItem(DI_TEXT, 5, Y, 0, 0, 0, 0, 0, 0, pszText2);
	iQTempl2 = addFarDialogItem(DI_EDIT, 5+nTextLen, Y, DialogWidth + 5 - nBtnLen, 0, 0, 0, 0, 0, g_QViewTemplate2);
	iQTempl2Reset = addFarDialogItem(DI_BUTTON, DialogWidth - nBtnLen + 7, Y, 0, 0, 0, 0, 0, 0, pszBtn);
	Y++;
	addFarDialogItem(DI_TEXT, 5, Y, 0, 0, 0, 0, 0, 0, pszText3);
	iQTempl3 = addFarDialogItem(DI_EDIT, 5+nTextLen, Y, DialogWidth + 5 - nBtnLen, 0, 0, 0, 0, 0, g_QViewTemplate3);
	iQTempl3Reset = addFarDialogItem(DI_BUTTON, DialogWidth - nBtnLen + 7, Y, 0, 0, 0, 0, 0, 0, pszBtn);
	Y++;
	addFarDialogItem(DI_TEXT, 5, Y++, 0, 0, 0, 0, DIF_BOXCOLOR | DIF_SEPARATOR, 0);
		
	return Y;
};

int MPicViewConfig::InitLOG() // Page5
{
	int Y = 2;
	
	//addFarDialogItem(
	//	DI_TEXT,      5, Y++, 0, 0, 0, 0, 0, 0, L"LOG (TODO)");
	iLogList = addFarDialogItem(
		DI_LISTBOX,   4, Y, DialogWidth + 5, Y+17, 0, 0, DIF_LISTNOBOX, 0, NULL/*GetMsg(MIPVDDecoderList)*/);
	//nModulesCount = PVDManager::Plugins.size();
	//if (nModulesCount > 0) {
	//	ModulesList.ItemsNumber = nModulesCount;
	//	ModulesList.Items = (FarListItem*)calloc(nModulesCount*sizeof(FarListItem),1);
	//
	//	pModules = (PVDDecoderConfig**)calloc(sizeof(LPVOID),nModulesCount);
	//	for (int i = 0; i < nModulesCount; i++) {
	//		pModules[i] = new PVDDecoderConfig(/*PVDManager::Plugins[i]->pPlugin,*/ PVDManager::Plugins[i]);
	//		ModulesList.Items[i].Text = pModules[i]->pTitle;
	//	}
	//} else {
	//	pModules = NULL;
	//}
	//DialogItems[iPvdDecoders].Param.ListItems = &ModulesList;
	Y+=17;

	return Y;
};

void MPicViewConfig::Init()
{
	memset(nPageStart, 0, sizeof(nPageStart));
	//memset(DialogItems, 0, sizeof(DialogItems));
	memset(bPageActivated, 0, sizeof(bPageActivated));
	memset(CustomColors, 0, sizeof(CustomColors));
	memset(&chc, 0, sizeof(chc));
	chc.lStructSize = sizeof(CHOOSECOLOR);
	chc.hwndOwner = g_Plugin.hConEmuWnd ? g_Plugin.hConEmuWnd : g_Plugin.hFarWnd;
	//chc.rgbResult = g_Plugin.BackColor;
	chc.lpCustColors = CustomColors;
	chc.Flags = CC_RGBINIT | CC_FULLOPEN | CC_ANYCOLOR;
	
	tbuf[0] = 0; pbuf = tbuf;
	
	//NextId() = 0;
	nNextPage = 0;
	//DialogWidth = 45; DialogHeight = 17;

	int Y = 1, pgY;
		
	addFarDialogItem(
		DI_DOUBLEBOX, 3, Y++, DialogWidth + 6, DialogHeight + 4, 0, 0, 0, 0, g_Plugin.pszPluginTitle);


	// Главные
	nPageStart[nNextPage++] = NextId();
	pgY = InitMain();
	if (pgY > Y) Y = pgY;

	// Decoders
	nPageStart[nNextPage++] = NextId();
	pgY = InitDecoders();
	if (pgY > Y) Y = pgY;
	
	// AdvSettings
	nPageStart[nNextPage++] = NextId();
	pgY = InitAdvanced();
	if (pgY > Y) Y = pgY;
	
	// OSD
	nPageStart[nNextPage++] = NextId();
	pgY = InitOSD();
	if (pgY > Y) Y = pgY;
	
	// LOG
	nPageStart[nNextPage++] = NextId();
	pgY = InitLOG();
	if (pgY > Y) Y = pgY;
	
	// g_Plugin.FlagsWord |= FW_CONFIGDECODER;
	
	// Подготовка страничек
	nActivePage = (g_Plugin.FlagsWork & FW_SHOW_MODULES) ? 1 : 0;
	nPageStart[nNextPage] = NextId();
	bPageActivated[nActivePage] = TRUE;
	//for (int i=nPageStart[1]; i<nPageStart[nNextPage]; i++)
	//	DialogItems[i].Flags |= DIF_HIDDEN;
	for (int j=0; j<nNextPage; j++) {
		if (j == nActivePage) continue;
    	for (int i=nPageStart[j]; i<nPageStart[j+1]; i++)
			DialogItems[i].Flags |= DIF_HIDDEN;
    }


	// Общие кнопки
	Y ++;
	addFarDialogItem(
		DI_TEXT, 5, Y, 0, 0, 0, 0, DIF_BOXCOLOR | DIF_SEPARATOR, 0);
	iMain = addFarDialogItem(
		DI_BUTTON, 0, Y, 0, 0, 0, 0, DIF_CENTERGROUP, 0, GetMsg(MIMain));
	iDecoders = addFarDialogItem(
		DI_BUTTON, 0, Y, 0, 0, 0, 0, DIF_CENTERGROUP, 0, GetMsg(MIDecoders));
	iAdvanced = addFarDialogItem(
		DI_BUTTON, 0, Y, 0, 0, 0, 0, DIF_CENTERGROUP, 0, GetMsg(MIAdvanced));
	iOSD = addFarDialogItem(
		DI_BUTTON, 0, Y, 0, 0, 0, 0, DIF_CENTERGROUP, 0, GetMsg(MIOSD));
	iLOG = addFarDialogItem(
		DI_BUTTON, 0, Y, 0, 0, 0, 0, DIF_CENTERGROUP, 0, GetMsg(MILOG));
	iLastTabBtn = iLOG;
	Y++;
	iOk = addFarDialogItem(
		DI_BUTTON, 0, Y, 0, 0, 0, 0, DIF_CENTERGROUP, 1, GetMsg(MIOK));
	iCancel = addFarDialogItem(
		DI_BUTTON, 0, Y, 0, 0, 0, 0, DIF_CENTERGROUP, 0, GetMsg(MICancel));

	//DialogItems[0].Y2 = Y + 1;
	//DialogHeight = Y + 3;
};

BOOL MPicViewConfig::ApplyMain()
{
	g_StartupInfo.SendDlgMessage(mh_Dlg, DM_GETTEXTPTR, iPrefix, (LONG_PTR)g_Plugin.sHookPrefix);
	g_Plugin.bHookArc            = GetCheck(iHookArc);
	g_Plugin.bHookQuickView      = GetCheck(iHookQuickView);
	g_Plugin.bHookView           = GetCheck(iHookView);
	g_Plugin.bHookCtrlShiftF3    = GetCheck(iHookCtrlShiftF3);
	g_Plugin.bHookEdit           = GetCheck(iHookEdit);
	g_Plugin.bHookCtrlShiftF4    = GetCheck(iHookCtrlShiftF4);
	g_Plugin.bCachingRP          = GetCheck(iCachingRP);
	g_Plugin.bCachingVP          = GetCheck(iCachingVP);
	g_Plugin.bTrayDisable        = GetCheck(iTrayDisable);
	g_Plugin.bFullScreenStartup  = GetCheck(iFullScreenStartup);
	g_Plugin.bLoopJump           = GetCheck(iLoopJump);
	g_Plugin.bMarkBySpace        = GetCheck(iMarkBySpace);
	g_Plugin.bFreePosition       = GetCheck(iFreePosition);
	g_Plugin.bFullDisplayReInit = GetCheck(iFullDirectDrawInit);
	g_Plugin.bAutoPaging         = GetCheck(iAutoPaging);
	switch (g_StartupInfo.SendDlgMessage(mh_Dlg, DM_LISTGETCURPOS, iAutoPagingVK, 0)) {
	case 1:  g_Plugin.nAutoPagingVK = VK_CAPITAL; break;
	case 2:  g_Plugin.nAutoPagingVK = VK_NUMLOCK; break;
	default: g_Plugin.nAutoPagingVK = VK_SCROLL;
	}
	g_Plugin.bAutoPagingSet      = GetCheck(iAutoPagingSet);
	
	g_Plugin.uCMYK2RGB = GetCheck(iCMYK2RGBstart + 0) ? PVD_CMYK2RGB_FAST : GetCheck(iCMYK2RGBstart + 1) ? PVD_CMYK2RGB_APPROX : PVD_CMYK2RGB_PRECISE;
	
	HKEY RegKey;
	if (!RegCreateKeyExW(HKEY_CURRENT_USER, g_RootKey, 0, NULL, 0, KEY_WRITE, NULL, &RegKey, NULL))
	{
		RegSetValueExW(RegKey, L"Prefix", 0, REG_SZ, (const BYTE*) g_Plugin.sHookPrefix, (int)wcslen(g_Plugin.sHookPrefix)*2 + 2);
		RegKeyWrite(RegKey, L"HookArc", g_Plugin.bHookArc);
		RegKeyWrite(RegKey, L"HookArc", g_Plugin.bHookArc);
		RegKeyWrite(RegKey, L"HookQuickView", g_Plugin.bHookQuickView);
		RegKeyWrite(RegKey, L"HookView", g_Plugin.bHookView);
		RegKeyWrite(RegKey, L"HookCtrlShiftF3", g_Plugin.bHookCtrlShiftF3);
		RegKeyWrite(RegKey, L"HookEdit", g_Plugin.bHookEdit);
		RegKeyWrite(RegKey, L"HookCtrlShiftF4", g_Plugin.bHookCtrlShiftF4);
		RegKeyWrite(RegKey, L"AutoCachingRP", g_Plugin.bCachingRP);
		RegKeyWrite(RegKey, L"AutoCachingVP", g_Plugin.bCachingVP);
		RegKeyWrite(RegKey, L"TrayDisable", g_Plugin.bTrayDisable);
		RegKeyWrite(RegKey, L"FullScreenStartup", g_Plugin.bFullScreenStartup);
		RegKeyWrite(RegKey, L"LoopJump", g_Plugin.bLoopJump);
		RegKeyWrite(RegKey, L"MarkBySpace", g_Plugin.bMarkBySpace);
		RegKeyWrite(RegKey, L"FreePosition", g_Plugin.bFreePosition);
		RegKeyWrite(RegKey, L"FullDirectDrawInit", g_Plugin.bFullDisplayReInit);
		RegKeyWrite(RegKey, L"AutoPaging", g_Plugin.bAutoPaging);
		RegKeyWrite(RegKey, L"AutoPagingKey", g_Plugin.nAutoPagingVK);
		RegKeyWrite(RegKey, L"AutoPagingSet", g_Plugin.bAutoPagingSet);
		RegKeyWrite(RegKey, L"CMYK2RGB", g_Plugin.uCMYK2RGB);
		RegCloseKey(RegKey);
	}
	
	return TRUE;
};

BOOL MPicViewConfig::ApplyDecoders()
{
	int n = nModulesCount * 16;
	for (int i=0; i<nModulesCount; i++, n-=16) {
		pModules[i]->pData->SetPriority(n);
	}
	
	if (nDisplayCount>0) {
		LONG_PTR i, pos = g_StartupInfo.SendDlgMessage(mh_Dlg, DM_LISTGETCURPOS, iPvdDisplay, 0);
		if (pos >= 0 && pos < nDisplayCount) {
			PVDManager::pDefaultDisplay = NULL;
			for (i=0; i<nDisplayCount; i++) {
				if (i == pos) {
					ppDisplays[i]->nActiveFlags |= PVD_IP_DISPLAY;
					PVDManager::pDefaultDisplay = ppDisplays[i];
				} else {
					ppDisplays[i]->nActiveFlags &= ~PVD_IP_DISPLAY;
				}
			}
		}
	}
	PVDManager::SortPlugins2();
	
	for (int i=0; i<nModulesCount; i++) {
		n = pModules[i]->pData->Priority();
		// Тут расширения вообще не трогать - они сохраняются в самом диалоге настройки декодера персонально
		HKEY hkey = NULL; DWORD dwDisp = 0;
		if (!RegCreateKeyEx(HKEY_CURRENT_USER, pModules[i]->pData->pRegPath, 0, 0, 0, KEY_ALL_ACCESS, 0, &hkey, &dwDisp)) {
			DWORD nDataSize = 0;
			RegSetValueEx(hkey, L"Priority", 0, REG_DWORD, (LPBYTE)&n, sizeof(n));
			RegSetValueEx(hkey, L"PluginActiveFlags", 0, REG_DWORD, (LPBYTE)&(pModules[i]->pData->nActiveFlags), sizeof(pModules[i]->pData->nActiveFlags));
			RegCloseKey(hkey); hkey = NULL;
		}
	}

	g_StartupInfo.SendDlgMessage(mh_Dlg, DM_GETTEXTPTR, iIgnoredExt, (LONG_PTR)g_Plugin.sIgnoredExt);

	HKEY RegKey;
	if (!RegCreateKeyExW(HKEY_CURRENT_USER, g_RootKey, 0, NULL, 0, KEY_WRITE, NULL, &RegKey, NULL))
	{
		RegSetValueExW(RegKey, L"IgnoredExtList", 0, REG_SZ, (const BYTE*) g_Plugin.sIgnoredExt, (int)wcslen(g_Plugin.sIgnoredExt)*2 + 2);
	
		RegCloseKey(RegKey);
	}
	
	return TRUE;
};

BOOL MPicViewConfig::ApplyAdvanced()
{
	g_StartupInfo.SendDlgMessage(mh_Dlg, DM_GETTEXTPTR, iChooseColor - 1, (LONG_PTR)pbuf);
	g_Plugin.nBackColor = wcstoul(pbuf + 2, NULL, 0x10);
	g_StartupInfo.SendDlgMessage(mh_Dlg, DM_GETTEXTPTR, iChooseQVColor - 1, (LONG_PTR)pbuf);
	g_Plugin.nQVBackColor = wcstoul(pbuf + 2, NULL, 0x10);
	_ASSERTE(g_Plugin.nQVBackColor > 0xFFFF);

	g_Plugin.bAutoZoom    = GetCheck(iAZGroup + 1);
	g_Plugin.bAutoZoomMin = GetCheck(iAZGroup + 2);
	g_Plugin.bAutoZoomMax = GetCheck(iAZGroup + 5);
	g_StartupInfo.SendDlgMessage(mh_Dlg, DM_GETTEXTPTR, iAZGroup + 3, (LONG_PTR)pbuf);
	g_Plugin.AutoZoomMin = MulDivU32R(Max(_wtoi(pbuf), 0), 0x10000, 100);
	g_StartupInfo.SendDlgMessage(mh_Dlg, DM_GETTEXTPTR, iAZGroup + 6, (LONG_PTR)pbuf);
	g_Plugin.AutoZoomMax = MulDivU32R(Max(_wtoi(pbuf), 0), 0x10000, 100);
	if (!g_Plugin.AutoZoomMin)
		g_Plugin.AutoZoomMin = 0x10000;
	if (!g_Plugin.AutoZoomMax)
		g_Plugin.AutoZoomMax = 0x10000;
	if (g_Plugin.AutoZoomMin > g_Plugin.AutoZoomMax)
		g_Plugin.AutoZoomMin = g_Plugin.AutoZoomMax = (g_Plugin.AutoZoomMin + g_Plugin.AutoZoomMax) / 2;
	g_Plugin.bKeepZoomAndPosBetweenFiles = GetCheck(iAZGroup + 8);
	g_Plugin.nKeepPanCorner = g_StartupInfo.SendDlgMessage(mh_Dlg, DM_LISTGETCURPOS, iKeepCorner, 0);
	if (g_Plugin.nKeepPanCorner > 20)
	{
		_ASSERTE(g_Plugin.nKeepPanCorner <= 20);
		g_Plugin.nKeepPanCorner = 0;
	}

	g_Plugin.bSmoothScrolling = GetCheck(iKSZGroup + 1);
	g_Plugin.bSmoothZooming   = GetCheck(iKSZGroup + 3);
	g_StartupInfo.SendDlgMessage(mh_Dlg, DM_GETTEXTPTR, iKSZGroup + 2, (LONG_PTR)pbuf);
	g_Plugin.SmoothScrollingStep = Min<uint>(_wtoi(pbuf), 10000);
	g_StartupInfo.SendDlgMessage(mh_Dlg, DM_GETTEXTPTR, iKSZGroup + 4, (LONG_PTR)pbuf);
	g_Plugin.SmoothZoomingStep   = Min<uint>(_wtoi(pbuf), 10000);
	if (!g_Plugin.SmoothScrollingStep)
		g_Plugin.SmoothScrollingStep = 20;
	if (!g_Plugin.SmoothZoomingStep)
		g_Plugin.SmoothZoomingStep = 33;

	g_Plugin.MouseZoomMode = GetCheck(iMZGroup + 1) ? 0 : GetCheck(iMZGroup + 2) ? 1 : 2;


	HKEY RegKey;
	if (!RegCreateKeyExW(HKEY_CURRENT_USER, g_RootKey, 0, NULL, 0, KEY_WRITE, NULL, &RegKey, NULL))
	{
		RegKeyWrite(RegKey, L"AutoZoom", g_Plugin.bAutoZoom);
		RegKeyWrite(RegKey, L"AutoZoomMinFlag", g_Plugin.bAutoZoomMin);
		RegKeyWrite(RegKey, L"AutoZoomMin", g_Plugin.AutoZoomMin);
		RegKeyWrite(RegKey, L"AutoZoomMaxFlag", g_Plugin.bAutoZoomMax);
		RegKeyWrite(RegKey, L"KeepZoomBetweenFiles", g_Plugin.bKeepZoomAndPosBetweenFiles);
		RegKeyWrite(RegKey, L"KeepPanCorner", g_Plugin.nKeepPanCorner);
		RegKeyWrite(RegKey, L"AutoZoomMax", g_Plugin.AutoZoomMax);
		RegKeyWrite(RegKey, L"SmoothScrolling", g_Plugin.bSmoothScrolling);
		RegKeyWrite(RegKey, L"SmoothScrollingStep", g_Plugin.SmoothScrollingStep);
		RegKeyWrite(RegKey, L"SmoothZooming", g_Plugin.bSmoothZooming);
		RegKeyWrite(RegKey, L"SmoothZoomingStep", g_Plugin.SmoothZoomingStep);
		RegKeyWrite(RegKey, L"MouseZoomMode", g_Plugin.MouseZoomMode);
		RegKeyWrite(RegKey, L"BackgroundColor", g_Plugin.nBackColor);
		_ASSERTE(g_Plugin.nQVBackColor > 0xFFFF);
		RegKeyWrite(RegKey, L"BackgroundQVColor", g_Plugin.nQVBackColor);
		RegCloseKey(RegKey);
	}
	return TRUE;
}	

BOOL MPicViewConfig::ApplyOSD()
{
	g_StartupInfo.SendDlgMessage(mh_Dlg, DM_GETTEXTPTR, iTitleTemplate, (LONG_PTR)g_TitleTemplate);
	if (!*g_TitleTemplate)
		lstrcpy(g_TitleTemplate, g_DefaultTitleTemplate);

	g_StartupInfo.SendDlgMessage(mh_Dlg, DM_GETTEXTPTR, iQTempl1, (LONG_PTR)g_QViewTemplate1);
	if (!*g_QViewTemplate1)
		lstrcpy(g_QViewTemplate1, g_DefaultQViewTemplate1);
	g_StartupInfo.SendDlgMessage(mh_Dlg, DM_GETTEXTPTR, iQTempl2, (LONG_PTR)g_QViewTemplate2);
	if (!*g_QViewTemplate2)
		lstrcpy(g_QViewTemplate2, g_DefaultQViewTemplate2);
	g_StartupInfo.SendDlgMessage(mh_Dlg, DM_GETTEXTPTR, iQTempl3, (LONG_PTR)g_QViewTemplate3);
	if (!*g_QViewTemplate3)
		lstrcpy(g_QViewTemplate3, g_DefaultQViewTemplate3);

	HKEY RegKey;
	if (!RegCreateKeyExW(HKEY_CURRENT_USER, g_RootKey, 0, NULL, 0, KEY_WRITE, NULL, &RegKey, NULL))
	{
		RegSetValueExW(RegKey, L"TitleTemplate", 0, REG_SZ, (const BYTE*) g_TitleTemplate, (int)wcslen(g_TitleTemplate)*2 + 2);
		RegSetValueExW(RegKey, L"QViewTemplate1", 0, REG_SZ, (const BYTE*) g_QViewTemplate1, (int)wcslen(g_QViewTemplate1)*2 + 2);
		RegSetValueExW(RegKey, L"QViewTemplate2", 0, REG_SZ, (const BYTE*) g_QViewTemplate2, (int)wcslen(g_QViewTemplate2)*2 + 2);
		RegSetValueExW(RegKey, L"QViewTemplate3", 0, REG_SZ, (const BYTE*) g_QViewTemplate3, (int)wcslen(g_QViewTemplate3)*2 + 2);
		RegCloseKey(RegKey);
	}

	return TRUE;
}	

BOOL MPicViewConfig::ApplyLOG()
{
	return TRUE;
}	

BOOL MPicViewConfig::Apply()
{
	if (bPageActivated[0])
		if (!ApplyMain())
			return FALSE;
	if (bPageActivated[1])
		if (!ApplyDecoders())
			return FALSE;
	if (bPageActivated[2])
		if (!ApplyAdvanced())
			return FALSE;
	if (bPageActivated[3])
		if (!ApplyOSD())
			return FALSE;
	if (bPageActivated[4])
		if (!ApplyLOG())
			return FALSE;
	return TRUE;
};

void MPicViewConfig::UpdateTitleFor(PVDDecoderConfig* pConfig)
{
	if (!gpCurConfig || !this)
		return;
		
	for (int pos = 0; pos < nModulesCount; pos++)
	{
		if (pModules[pos] == pConfig) {
			FarListUpdate flu = {pos};
			flu.Item.Flags = LIF_SELECTED;
			flu.Item.Text = pConfig->pTitle;
			g_StartupInfo.SendDlgMessage(mh_Dlg, DM_LISTUPDATE, iPvdDecoders, (LONG_PTR)&flu);
			break;
		}
	}
}

int MPicViewConfig::Run()
{
	Init();
	
	BOOL lbRc = FALSE;
	for (;;)
	{
		if (!DialogInit())
			break; // ошибка выделения памяти?
		
		const int ExitCode = g_StartupInfo.DialogRun(mh_Dlg);
		if (ExitCode == iOk)
		{
			lbRc = Apply();
			if (!lbRc) {
				DialogFree();
				DialogItems[iOk].Focus = 1;
				DialogItems[iAdvanced].Focus = 0;
				continue;
			}
		}
		break;
	}
	DialogFree();
	
	if (ModulesList.Items) {
		free(ModulesList.Items);
		ModulesList.Items = NULL;
	}
	if (DisplayList.Items) {
		free(DisplayList.Items);
		DisplayList.Items = NULL;
	}
	if (ppDisplays) {
		free(ppDisplays);
		ppDisplays = NULL;
	}
	if (pModules) {
		for (int i=0; i<nModulesCount; i++) {
			//pModules[i]->Free();
			delete pModules[i];
			pModules[i] = NULL;
		}
		free(pModules); pModules = NULL;
	}
	if (PagingKeys.Items) {
		free(PagingKeys.Items);
		PagingKeys.Items = NULL;
	}
	if (CornerList.Items) {
		free(CornerList.Items);
		CornerList.Items = NULL;
	}
	
	return lbRc;
};
	
	
	

int WINAPI ConfigureW(int ItemNumber)
{
	if (!gnMainThreadId)
		gnMainThreadId = GetCurrentThreadId();
		
	FUNCLOGGER(L"ConfigureW");

	DBGOUTCFG(L"ConfigureW...");

	g_Plugin.InitPlugin();
	DBGOUTCFG(L"InitPlugin done...");

	g_Plugin.InitCMYK(FALSE); // Дождаться его завершения, чтобы случайно не войти в клинч с другим модулем

	int nRc = 0;
	gpCurConfig = new MPicViewConfig();
	DBGOUTCFG(L"MPicViewConfig done...");
	if (gpCurConfig) {
		nRc = gpCurConfig->Run();
		DBGOUTCFG(L"gpCurConfig->Run done...");
		
		delete gpCurConfig;
		gpCurConfig = NULL;
	}

	DBGOUTCFG(L"ConfigureW done...\n");
	
	return nRc;
}
