
#pragma once

#define MAX_CONFIG_ITEMS 100

class MPicViewDlg {
protected:
	HANDLE mh_Dlg;
	LPCTSTR mps_HelpTopic;

	FarDialogItem DialogItems[MAX_CONFIG_ITEMS];

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

	static LONG_PTR WINAPI StaticDlgProc(HANDLE hDlg, int Msg, int Param1, LONG_PTR Param2);

	int NextId();
	virtual void Reset();

public:
	HANDLE DialogInit();
	void DialogFree();
};
