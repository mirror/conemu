
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


#include "PictureView.h"
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
	if (nNextId >= MAX_CONFIG_DLGITEMS || nNextId < 0)
	{
		_ASSERTE(nNextId<MAX_CONFIG_DLGITEMS);
		return -1;
	}

	int nId = nNextId++;
	FarDialogItem* pItems = DialogItems+nId;
	memset(pItems, 0, sizeof(*pItems));

	if (nId > 0)
	{
		if (max(X1,X2) > nMaxX)
			nMaxX = max(X1,X2);
		if (max(Y1,Y2) > nMaxY)
			nMaxY = max(Y1,Y2);
	}

#ifdef FAR_UNICODE
	pItems->Type = (FARDIALOGITEMTYPES)Type;
#else
	pItems->Type = Type;
#endif
	pItems->X1 = X1;
	pItems->Y1 = Y1;
	pItems->X2 = X2;
	pItems->Y2 = Y2;
	pItems->Flags = Flags;
#ifdef FAR_UNICODE
	pItems->Flags |=
		(Focus ? DIF_FOCUS : 0) |
		(DefaultButton ? DIF_DEFAULTBUTTON : 0);
	pItems->Selected = Selected;
	pItems->Data = PtrData;
	pItems->MaxLength = MaxLen; // terminate 0 not included (if == 0 string size is unlimited)
#else
	pItems->Focus = Focus;
	pItems->DefaultButton = DefaultButton;
	pItems->Param.Selected = Selected; // Тут может быть DWORD_PTR - указатель на память!
	pItems->PtrData = PtrData;
	pItems->MaxLen = MaxLen;
#endif

	return nId;
};

LONG_PTR MPicViewDlg::ConfigDlgProc(HANDLE hDlg, int Msg, int Param1, LONG_PTR Param2)
{
	return g_StartupInfo.DefDlgProc(hDlg, Msg, Param1, (DLG_LPARAM)Param2);
}

DLG_RESULT MPicViewDlg::StaticDlgProc(HANDLE hDlg, int Msg, int Param1, DLG_LPARAM Param2)
{
	MPicViewDlg *pDlg = NULL;
	
	if (Msg == DN_INITDIALOG)
	{
		//g_StartupInfo.SendDlgMessage(hDlg, DM_SETDLGDATA, 0, Param2);
		pDlg = (MPicViewDlg*)Param2;
	}
	else
	{
		pDlg = (MPicViewDlg*)g_StartupInfo.SendDlgMessage(hDlg, DM_GETDLGDATA, 0, 0);
	}
	_ASSERTE(pDlg);

	return pDlg->ConfigDlgProc(hDlg, Msg, Param1, (LONG_PTR)Param2);
}

HANDLE MPicViewDlg::DialogInit()
{
	// Если он НЕ был освобожден - сначала сделает это
	DialogFree();

	DialogHeight = nMaxY + 3;
	DialogItems[0].Y2 = nMaxY + 1;

	mh_Dlg = g_StartupInfo.DialogInit(
		#ifdef FAR_UNICODE
		&guid_PicView, &guid_ConfDlg,
		#else
		g_StartupInfo.ModuleNumber,
		#endif
		-1, -1,
		DialogWidth + 10, DialogHeight, mps_HelpTopic,
		DialogItems, nNextId, 0, 0, StaticDlgProc, (DLG_LPARAM)this);
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
