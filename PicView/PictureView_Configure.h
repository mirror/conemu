/**************************************************************************
Copyright (c) 2009 Skakov Pavel
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



#define MAX_PAGE_COUNT   10
#define MASK_COUNT 3

class MPicViewConfig;
//MPicViewConfig* gpCurConfig = NULL;


class MPicViewConfig : public MPicViewDlg {
public:
	int nPageStart[MAX_PAGE_COUNT];
	BOOL bPageActivated[MAX_PAGE_COUNT];

	int nNextPage, nActivePage;
	int iHookArc, iHookQuickView, iHookView, iHookEdit, iPrefix; //, iUsePVD; //, iUseGFL, iUseGDIPlus;
	int iHookCtrlShiftF3, iHookCtrlShiftF4;
	//int iUsePVDExt, iUseGFLExt, iUseGDIPlusExt, 
	int iPvdDecoders, iIgnoredExt, iIgnoredExtBtn, iPvdDisplay;
	int iCachingRP, iCachingVP, iTrayDisable, iFullScreenStartup, iLoopJump, iMarkBySpace;
	int iAutoPaging, iAutoPagingVK, iAutoPagingSet;
	int iLogList;
	int iFreePosition, iFullDirectDrawInit, iCMYK2RGBstart;
	int iAZGroup, iKeepCorner, iKSZGroup, iMZGroup, iTitleTemplate, iChooseColor, iChooseQVColor;
	int iTitleTemplReset;
	int iQTempl1, iQTempl1Reset, iQTempl2, iQTempl2Reset, iQTempl3, iQTempl3Reset;
	int iOk, iCancel;
	int iMain, iDecoders, iAdvanced, iOSD, iLOG, iLastTabBtn;
	bool bDecoderPriorityChanged;
	
	wchar_t tbuf[0x800], *pbuf;
	
	COLORREF CustomColors[0x10];
	CHOOSECOLOR chc;
	
	FarList ModulesList;
	int nModulesCount, nDisplayCount, nCornerCount;
	PVDDecoderConfig** pModules;
	FarList PagingKeys, DisplayList, CornerList;
	ModuleData **ppDisplays;
	

public:
	MPicViewConfig();

	~MPicViewConfig();
	
protected:
	void Exchange ( HANDLE hDlg, int Old, int New );
	bool CanExchange ( int Old, int New );
	void UpdateModulesHotkeys(HANDLE hDlg);
	
	
	void ActivateMain(HANDLE hDlg);
	void ActivateDecoders(HANDLE hDlg);
	void ActivateAdvanced(HANDLE hDlg);
	void ActivateOSD(HANDLE hDlg);
	void ActivateLOG(HANDLE hDlg);
	
	virtual LONG_PTR ConfigDlgProc(HANDLE hDlg, int Msg, int Param1, LONG_PTR Param2);
	

protected:

protected:
	int InitMain() // Page1
	;
	
	int InitDecoders() // Page2
	;
	
	int InitAdvanced() // Page3
	;
	
	int InitOSD() // Page4
	;
	
	int InitLOG() // Page5
	;
	

	void Init()
	;
	
	BOOL ApplyMain()
	;
	
	BOOL ApplyDecoders()
	;
	
	BOOL ApplyAdvanced()
	;	
	
	BOOL ApplyOSD()
	;	
	
	BOOL ApplyLOG()
	;	
	
	BOOL Apply()
	;
	
public:
	int Run()
	;
	
	void UpdateTitleFor(PVDDecoderConfig* pConfig);
};

extern MPicViewConfig* gpCurConfig;
