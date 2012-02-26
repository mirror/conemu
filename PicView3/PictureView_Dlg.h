
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


#define MAX_CONFIG_PVDITEMS 80
#define MAX_CONFIG_DLGITEMS (MAX_CONFIG_PVDITEMS+20)

class MPicViewDlg
{
protected:
	HANDLE mh_Dlg;
	LPCTSTR mps_HelpTopic;

	FarDialogItem DialogItems[MAX_CONFIG_DLGITEMS];

	int DialogWidth, DialogHeight, nMaxY, nMaxX;

private:
	int nNextId;

public:
	MPicViewDlg(LPCTSTR asTitle, int anWidth, int anHeight, LPCTSTR asHelpTopic);

	~MPicViewDlg();
	
protected:

	int addFarDialogItem(int Type, int X1, int Y1, int X2, int Y2, int Focus,
						 DWORD_PTR Selected, DWORD Flags, int DefaultButton,
						 LPCTSTR PtrData = NULL, size_t MaxLen = 0);

	virtual LONG_PTR ConfigDlgProc(HANDLE hDlg, int Msg, int Param1, LONG_PTR Param2);

	static DLG_RESULT WINAPI StaticDlgProc(HANDLE hDlg, int Msg, int Param1, DLG_LPARAM Param2);

	int NextId();
	virtual void Reset();

public:
	HANDLE DialogInit();
	void DialogFree();
};
