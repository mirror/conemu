
#pragma once

class CScreenRestore;

CScreenRestore *gpAction = NULL;

extern struct PluginStartupInfo psi;

class CScreenRestore
{
protected:
	HANDLE hScreen;
	const TCHAR *pszTitle, *pszMsg;
	CScreenRestore *pLastAction;
public:
	void Restore() {
		if (hScreen) {
			psi.RestoreScreen(hScreen);
			hScreen = NULL;
		}
	}
	void Save() {
		if (hScreen) Restore();
		hScreen = psi.SaveScreen(0,0,-1,-1);
	}
	void Message(const TCHAR* aszMsg, BOOL abForce=TRUE)
	{
		if (abForce || !hScreen) {
			if (hScreen) Restore();
			Save();
		}

		_ASSERTE(aszMsg!=NULL);
		pszMsg = aszMsg;
		
		const TCHAR *MsgItems[]={pszTitle,aszMsg};
		psi.Message(psi.ModuleNumber,0,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),0);
	}
protected:
	void RefreshMessage() {
		if (pszMsg) Message(pszMsg, TRUE);
	}
public:
	CScreenRestore(const TCHAR* aszMsg, const TCHAR* aszTitle=NULL) : hScreen(NULL)
	{
		_ASSERTE(aszMsg!=NULL);
		pLastAction = gpAction; //nLastTick = -1;
		pszTitle = aszTitle ? aszTitle : _T("");

		if (aszMsg == NULL) {
			Save();
		} else {
			Message(aszMsg, TRUE);			
		}

		gpAction = this;
	};
	~CScreenRestore()
	{
		Restore();
		if (gpAction == this) {
			if (pLastAction != this) {
				gpAction = pLastAction;
				if (gpAction)
					gpAction->RefreshMessage();
			} else {
				_ASSERTE(pLastAction!=this);
				gpAction = NULL;
			}
		} else {
			_ASSERTE(gpAction == this);
		}
	};
};
