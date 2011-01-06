
#include "PictureView.h"
#include "headers/farkeys.hpp"
#include "PictureView_Dlg.h"
	
MPicViewDlg::MPicViewDlg(LPCTSTR asTitle, int anWidth, int anHeight, LPCTSTR asHelpTopic)
{
	mh_Dlg = NULL;
	mps_HelpTopic = asHelpTopic;
	nNextId = 0; DialogWidth = anWidth; DialogHeight = anHeight; nMaxY = nMaxX = 0;
	
	addFarDialogItem(
		DI_DOUBLEBOX, 3, 1, DialogWidth + 6, DialogHeight + 8, 0, 0, 0, 0, asTitle);
}

MPicViewDlg::~MPicViewDlg()
{
	_ASSERTE(mh_Dlg==NULL); // Уже должен быть вызван DialogFree
}

int MPicViewDlg::addFarDialogItem(int Type, int X1, int Y1, int X2, int Y2, int Focus,
					 DWORD_PTR Selected, DWORD Flags, int DefaultButton,
					 LPCTSTR PtrData /*= NULL*/, size_t MaxLen /*= 0*/)
{
	if (nNextId >= MAX_CONFIG_ITEMS || nNextId < 0) {
		_ASSERTE(nNextId<MAX_CONFIG_ITEMS);
		return -1;
	}

	int nId = nNextId++;
	FarDialogItem* pItems = DialogItems+nId;

	if (nId > 0) {
		if (max(X1,X2) > nMaxX)
			nMaxX = max(X1,X2);
		if (max(Y1,Y2) > nMaxY)
			nMaxY = max(Y1,Y2);
	}

	pItems->Type = Type;
	pItems->X1 = X1;
	pItems->Y1 = Y1;
	pItems->X2 = X2;
	pItems->Y2 = Y2;
	pItems->Focus = Focus;
	pItems->Param.Reserved = Selected; // Тут может быть DWORD_PTR - указатель на память!
	pItems->Flags = Flags;
	pItems->DefaultButton = DefaultButton;
	pItems->PtrData = PtrData;
	pItems->MaxLen = MaxLen;

	return nId;
};

LONG_PTR MPicViewDlg::ConfigDlgProc(HANDLE hDlg, int Msg, int Param1, LONG_PTR Param2)
{
	return g_StartupInfo.DefDlgProc(hDlg, Msg, Param1, Param2);
}

LONG_PTR MPicViewDlg::StaticDlgProc(HANDLE hDlg, int Msg, int Param1, LONG_PTR Param2)
{
	MPicViewDlg *pDlg = NULL;
	
	if (Msg == DN_INITDIALOG) {
		//g_StartupInfo.SendDlgMessage(hDlg, DM_SETDLGDATA, 0, Param2);
		pDlg = (MPicViewDlg*)Param2;
	} else {
		pDlg = (MPicViewDlg*)g_StartupInfo.SendDlgMessage(hDlg, DM_GETDLGDATA, 0, 0);
	}
	_ASSERTE(pDlg);

	return pDlg->ConfigDlgProc(hDlg, Msg, Param1, Param2);
}

HANDLE MPicViewDlg::DialogInit()
{
	// Если он НЕ был освобожден - сначала сделает это
	DialogFree();

	DialogHeight = nMaxY + 3;
	DialogItems[0].Y2 = nMaxY + 1;

	mh_Dlg = g_StartupInfo.DialogInit(g_StartupInfo.ModuleNumber, -1, -1,
		DialogWidth + 10, DialogHeight, mps_HelpTopic,
		DialogItems, nNextId, 0, 0, StaticDlgProc, (LONG_PTR)this);
	if (mh_Dlg == INVALID_HANDLE_VALUE)
		mh_Dlg = NULL;
	return mh_Dlg;
}

void MPicViewDlg::DialogFree()
{
	if (mh_Dlg != NULL && mh_Dlg != INVALID_HANDLE_VALUE)
		g_StartupInfo.DialogFree(mh_Dlg);
	mh_Dlg = NULL;
}

int MPicViewDlg::NextId()
{
	return nNextId;
}

void MPicViewDlg::Reset()
{
	nMaxY = nMaxX = 0;
	mh_Dlg = NULL;
	memset(DialogItems+1,0,sizeof(DialogItems)-sizeof(DialogItems[0]));
	nNextId = 1;
}
