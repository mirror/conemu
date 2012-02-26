
#pragma once

/**************************************************************************
Copyright (c) 2009-2010 Maximus5
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


// Файл описывает следюущие классы

// Базовый класс работы с модулем, содержит виртуальные функции работы с интерфесами
// (перекрываются в потомках версий 1 и 2)
// Также содержит некоторые параметры модуля "по умолчанию" (расширения, приоритет,...)
class CPVDModuleBase;

// Полная информация о модуле, настройки расширений из реестра (отличаются от "умолчаний" модуля)
// Также содержит указатель на интерфейс соответствующей версии (вызов виртуальных функций)
class CModuleInfo;

// Используется в диалоге настройки субплагинов (расширения, приоритет, параметры)
class CPVDModuleConfig;
// Класс диалога настройки модуля
class CPVDModuleConfigDlg;
// Класс диалога описания модуля (название, версия, автор, расширения "по умолчанию")
class CPVDDecoderAboutDlg;


#include "PictureView_Dlg.h"


class CPVDModuleBase
{
public:
	HMODULE hPlugin;
	CModuleInfo* pData;
	const wchar_t *pModulePath;  // Полный путь к субплагину (pvd файл)
	const wchar_t *pRegPath;     // Полный путь к ключу субплагина в реестре (настройки, расширения, приоритет, и т.п.)
	wchar_t *pRegSettings;       // Полный путь к ключу субплагина с его личными настройками (подключ pRegPath)
	DWORD nDefPriority;
	wchar_t *pDefActive;   // Список поддерживаемых расширений через запятую.
                           // Это расширения, которые модуль умеет "хорошо" открывать.
	                       // Здесь допускается указание "*" означающее, что
	                       // субплагин является универсальным.
                           // Если при распознавании ни один из субплагинов не подошел по расширению -
                           // PicView все равно попытается открыть файл субплагином, если расширение
                           // не указано в списке его игнорируемых.
    wchar_t *pDefInactive; // Список неактивных расширений через запятую.
                           // Здесь указываются расширения, которые модуль может открыть
                           // "в принципе", но возможно, с проблемами.
    wchar_t *pDefForbidden;// Список игнорируемых расширений через запятую.
                           // Для файлов с указанными расширениями субплагин не
                           // будет вызываться вообще. Укажите "." для игнорирования
	                       // файлов без расширений.
	UINT   nVersion;
	UINT32 nFlags;         // Возможные флаги PVD_IP_xxx (что получено из pvdPluginInfo)
	UINT32 nCurrentFlags;  // erased unsupported flags from nFlags
	wchar_t *pName;        // имя субплагина
	wchar_t *pVersion;     // версия субплагина
	wchar_t *pComments;    // комментарии о субплагине: для чего предназначен, кто автор субплагина, ...
	bool bPrepared;        // параметры плагина загружены (возможно - только из реестра)
	bool bInitialized;     // pvd библиотека загружена и функции инициализированы
	bool bDisplayInitialized;
	wchar_t szLastError[512];
	//
	bool bCancelled;
public:
	CPVDModuleBase(CModuleInfo* apData);
	virtual ~CPVDModuleBase();
	bool InitPrepare(FILETIME ftRegModified, BOOL abPluginChanged);
	bool LoadFormatsFromReg(FILETIME ftRegModified, BOOL abForceWriteReg);
	virtual bool LoadFormatsFromPvd() = 0;
	void SaveFormatsToReg(FILETIME *pftRegModified=NULL);
	virtual bool InitPlugin(bool bAllowResort=false) = 0; // Загрузить, инициализировать, получить список форматов
	wchar_t* LoadRegValue(HKEY hkey, LPCWSTR asName) const;
	bool IsAllowed(const wchar_t* asExt, bool abAllowAsterisk, bool abAllowInactive, bool abAssumeActive=false);
	void SetStatus(LPCWSTR asStatus, BOOL abSaveInReg=FALSE);
	void SetException(LPCWSTR asFunction);
public:
	virtual BOOL FileOpen2(const wchar_t *pFileName, INT64 lFileSize, const BYTE *pBuf, UINT32 lBuf, pvdInfoImage2 *pImageInfo) = 0;
	virtual BOOL PageInfo2(void *pImageContext, pvdInfoPage2 *pPageInfo) = 0;
	virtual BOOL PageDecode2(void *pImageContext, pvdInfoDecode2 *pDecodeInfo, pvdDecodeCallback2 DecodeCallback, void *pDecodeCallbackContext) = 0;
	virtual void PageFree2(void *pImageContext, pvdInfoDecode2 *pDecodeInfo) = 0;
	virtual void FileClose2(void *pImageContext) = 0;
	virtual BOOL TranslateError2(DWORD nErrNumber, wchar_t *pszErrInfo, int nBufLen);
	virtual void ReloadConfig2() {};
	virtual void Cancel2() {};
	BOOL DisplayCheck();
	BOOL DisplayAttach();
	BOOL DisplayDetach();
	virtual BOOL DisplayInit2(pvdInfoDisplayInit2* pDisplayInit) { return FALSE; };
protected:
	virtual BOOL DisplayAttach2(HWND hWnd, BOOL bAttach) { return FALSE; };
	BOOL LoadExtensionsFromReg(HKEY hkey, LPCWSTR asName, wchar_t** rpszExt);
public:
	virtual BOOL DisplayCreate2(pvdInfoDisplayCreate2* pDisplayCreate) { return FALSE; };
	virtual BOOL DisplayPaint2(void* pDisplayContext, pvdInfoDisplayPaint2* pDisplayPaint) { return FALSE; };
	virtual void DisplayClose2(void* pDisplayContext) {};
	virtual void DisplayExit2() {};
protected:
	virtual bool PluginInfo2() = 0;
	virtual BOOL Init2() = 0;
	virtual void Exit2() {};
	void* mp_Context;
	BOOL  mb_DisplayAttached;
};

class CPVDModuleConfig;

class CPVDModuleConfigDlg : public MPicViewDlg
{
protected:
	CPVDModuleConfig* mp_Config;
	int iActiveExt, iInactiveExt, iForbiddenExt, iResetExt, iOk, iCancel;
	int /*iPName, iPVer, iPComm,*/ iPLog, iReload, iUnload, iAbout /*, iPFlags*/;
	FarList ModuleFlags; FarListItem ModuleFlagItems[30]; wchar_t szFlagsLeft[20];
public:
	CPVDModuleConfigDlg(CPVDModuleConfig* pConfig);
	~CPVDModuleConfigDlg();
	virtual LONG_PTR ConfigDlgProc(HANDLE hDlg, int Msg, int Param1, LONG_PTR Param2);
	bool Run();
protected:
	virtual void Reset();
	// Параметры модуля, Загружаются из реестра, из подключа модуля
	typedef struct tag_PvdItem {
		wchar_t* pszType;
		wchar_t szName[MAX_PATH];
		const wchar_t* pszLabel;
		wchar_t* pszData;
		DWORD nData;
		bool bData;
		DWORD nLen, nType;
		int nID, nCount; // nCount используется для радио-кнопок
	} PvdItem;
	PvdItem m_PvdItems[MAX_CONFIG_PVDITEMS];
	int mn_PvdItems;
	int LoadItems();
	void FreeItems();
	wchar_t ms_Title[128];
	wchar_t* Title(const wchar_t* asModule);
	BOOL LoadExtensionsFromDlg(int nID, wchar_t** rpszExt);
};

class CPVDDecoderAboutDlg : public MPicViewDlg
{
protected:
	CPVDModuleConfig* mp_Config;
	int iOk, iCancel;
	int iActiveExt, iInactiveExt, iForbiddenExt, iPriority, iReloadExt;
	wchar_t szPriority[32];
	int iPName, iPVer, iPComm, iPLog, iPFlags;
	FarList ModuleFlags; FarListItem ModuleFlagItems[30]; wchar_t szFlagsLeft[20];
public:
	CPVDDecoderAboutDlg(CPVDModuleConfig* pConfig);
	~CPVDDecoderAboutDlg();
	virtual LONG_PTR ConfigDlgProc(HANDLE hDlg, int Msg, int Param1, LONG_PTR Param2);
	bool Run();
protected:
	virtual void Reset();
	wchar_t ms_Title[128];
	wchar_t* Title(const wchar_t* asModule);
};

#include "RefRelease.h"

TODO("Все что использует CModuleInfo прошерстить на предмет RefRelease");

class CModuleInfo : public CRefRelease
{
public:
	BOOL bPrepared; // Выставляется в TRUE после проверки версии и пр.
	FILETIME ftModified; // Дата последнего изменения файла
	DWORD nVersion;
	DWORD nFlags; // Флаги, которые отдал плагин (некоторые PicView может сбросить)
	DWORD nCurrentFlags; // Текущие флаги плагина (например PicView сбрасывает флаг DISPLAY если DX запущен в терминале)
	DWORD nActiveFlags; // Пока только PVD_IP_DISPLAY как указатель на умолчательный модуль вывода
	wchar_t *pModulePath;  // Полный путь к субплагину (pvd файл)
	const wchar_t *pModuleFileName;
	wchar_t *pRegPath;     // Полный путь к ключу субплагина в реестре (настройки, расширения, приоритет, и т.п.)
	HMODULE hPlugin;       // May be NULL
	DWORD nPriority;
	wchar_t *pActive;      // Список поддерживаемых расширений через запятую.
                           // Это расширения, которые модуль умеет "хорошо" открывать.
	                       // Здесь допускается указание "*" означающее, что
	                       // субплагин является универсальным.
                           // Если при распознавании ни один из субплагинов не подошел по расширению -
                           // PicView все равно попытается открыть файл субплагином, если расширение
                           // не указано в списке его игнорируемых.
    wchar_t *pInactive;    // Список неактивных расширений через запятую.
                           // Здесь указываются расширения, которые модуль может открыть
                           // "в принципе", но возможно, с проблемами.
    wchar_t *pForbidden;   // Список игнорируемых расширений через запятую.
                           // Для файлов с указанными расширениями субплагин не
                           // будет вызываться вообще. Укажите "." для игнорирования
	                       // файлов без расширений.
	CPVDModuleBase *pPlugin;
	// Переменная используется при подборе декодера
	//bool bProcessed;
	wchar_t szStatus[0x1000];

	CModuleInfo(LPCSTR asFrom);
protected:
	virtual ~CModuleInfo();
public:

	//
	void SetStatus(LPCWSTR asStatus, BOOL abSaveInReg=FALSE);
	DWORD Priority();
	void SetPriority(DWORD n);
	//
	void Unload();
	bool Load(bool abPluginChanged); // изменилась LastWriteDate
};

// Используется в диалоге настройки субплагинов (расширения, приоритет, параметры)
class CPVDModuleConfig
{
public:
	//CPVDModuleBase* pPlugin;
	CModuleInfo* pData;
	wchar_t* pTitle;
	DWORD nPriority;
	wchar_t* pActive;
	wchar_t* pInactive;
	wchar_t* pForbidden;
	
	CPVDModuleConfig(/*CPVDModuleBase* p,*/ CModuleInfo* apData);
	~CPVDModuleConfig();

	//void Init(CPVDModuleBase* p, CModuleInfo* apData);
	void CreateTitle();
	void Free();
	bool Configure();
	void About();
};
