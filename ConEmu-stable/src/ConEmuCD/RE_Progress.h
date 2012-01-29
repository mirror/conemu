
#pragma once

/*

	Отображение прогресса операций

*/

class CScreenRestore
{
public:
	void Restore() {};
	void Save() {};
	void Message(const TCHAR* aszMsg, BOOL abForce=TRUE) {};
	void RefreshMessage() {};

	CScreenRestore(const TCHAR* aszMsg, const TCHAR* aszTitle=NULL) {};
	~CScreenRestore() {};
};

class REProgress : public CScreenRestore
{
	REProgress(const TCHAR* aszTitle, BOOL abGraphic = FALSE, const TCHAR* aszFileInfo = NULL) : CScreenRestore(NULL, NULL) {};
	REProgress(const TCHAR* aszMsg, const TCHAR* aszTitle) : CScreenRestore(NULL, NULL) {};
	~REProgress() {};
public:
	BOOL IncAll(size_t anAddAllCount = 1) { return TRUE; };
	BOOL Step(size_t anProcessed = 1) { return TRUE; };
	BOOL SetAll(unsigned __int64 anAllCount) { return TRUE; };
	BOOL SetStep(unsigned __int64 anAllStep, BOOL abForce = FALSE, LPCWSTR asFileName = NULL) { return TRUE; };
	void ShowDuration() {};
	BOOL CheckForEsc(BOOL abForceRead = FALSE) { return FALSE; };
};

extern REProgress *gpProgress;
