
#pragma once

/**************************************************************************
Copyright (c) 2009 Maximus5
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

// Version 2
typedef void (__stdcall *pvdPluginInfo2_t)(pvdInfoPlugin2 *pPluginInfo);
typedef BOOL (__stdcall *pvdTranslateError2_t)(DWORD nErrNumber, wchar_t *pszErrInfo, int nBufLen);
typedef UINT32 (__stdcall *pvdInit2_t)(pvdInitPlugin2* pInit);
typedef void (__stdcall *pvdGetFormats2_t)(void *pContext, pvdFormats2* pFormats);
typedef void (__stdcall *pvdExit2_t)(void *pContext);
typedef BOOL (__stdcall *pvdFileOpen2_t)(void *pContext, const wchar_t *pFileName, INT64 lFileSize, const BYTE *pBuf, UINT32 lBuf, pvdInfoImage2 *pImageInfo);
typedef BOOL (__stdcall *pvdPageInfo2_t)(void *pContext, void *pImageContext, pvdInfoPage2 *pPageInfo);
typedef BOOL (__stdcall *pvdPageDecode2_t)(void *pContext, void *pImageContext, pvdInfoDecode2 *pDecodeInfo, pvdDecodeCallback2 DecodeCallback, void *pDecodeCallbackContext);
typedef void (__stdcall *pvdPageFree2_t)(void *pContext, void *pImageContext, pvdInfoDecode2 *pDecodeInfo);
typedef void (__stdcall *pvdPageFree2Old_t)(void *pContext, void *pImageContext, UINT32 iPage, pvdInfoDecode2 *pDecodeInfo);
typedef void (__stdcall *pvdFileClose2_t)(void *pContext, void *pImageContext);
typedef void (__stdcall *pvdReloadConfig2_t)(void *pContext);
typedef void (__stdcall *pvdCancel2_t)(void *pContext); // отмена всех операций переданного контекста
//
// Инициализация контекста дисплея. Используется тот pContext, который был получен в pvdInit2
typedef BOOL (__stdcall *pvdDisplayInit2_t)(void *pContext, pvdInfoDisplayInit2* pDisplayInit);
//
typedef BOOL (__stdcall *pvdDisplayAttach2_t)(void *pContext, pvdInfoDisplayAttach2* pDisplayAttach);
// Создать контекст для отображения картинки в pContext (перенос декодированных данных в видеопамять)
typedef BOOL (__stdcall *pvdDisplayCreate2_t)(void *pContext, pvdInfoDisplayCreate2* pDisplayCreate);
// Собственно отрисовка. Функция должна при необходимости выполнять "Stretch"
typedef BOOL (__stdcall *pvdDisplayPaint2_t)(void *pContext, void* pDisplayContext, pvdInfoDisplayPaint2* pDisplayPaint);
// Закрыть контекст для отображения картинки (освободить видеопамять)
typedef void (__stdcall *pvdDisplayClose2_t)(void *pContext, void* pDisplayContext);
// Закрыть модуль вывода (освобождение интерфейсов DX, отцепиться от окна)
typedef void (__stdcall *pvdDisplayExit2_t)(void *pContext);

class PVDDecoderVer2 : public PVDDecoderBase {
public:
	PVDDecoderVer2(ModuleData* apData);
	virtual ~PVDDecoderVer2();
	virtual bool LoadFormatsFromPvd();
	virtual bool InitPlugin(bool bAllowResort=false); // Загрузить, инициализировать, получить список форматов
public:
	virtual BOOL TranslateError2(DWORD nErrNumber, wchar_t *pszErrInfo, int nBufLen);
	virtual BOOL FileOpen2(const wchar_t *pFileName, INT64 lFileSize, const BYTE *pBuf, UINT32 lBuf, pvdInfoImage2 *pImageInfo);
	virtual BOOL PageInfo2(void *pImageContext, pvdInfoPage2 *pPageInfo);
	virtual BOOL PageDecode2(void *pImageContext, pvdInfoDecode2 *pDecodeInfo, pvdDecodeCallback2 DecodeCallback, void *pDecodeCallbackContext);
	virtual void PageFree2(void *pImageContext, pvdInfoDecode2 *pDecodeInfo);
	virtual void FileClose2(void *pImageContext);
	virtual void ReloadConfig2();
	virtual void Cancel2();
	virtual BOOL DisplayInit2(pvdInfoDisplayInit2* pDisplayInit);
protected:
	virtual BOOL DisplayAttach2(HWND hWnd, BOOL bAttach);
public:
	virtual BOOL DisplayCreate2(pvdInfoDisplayCreate2* pDisplayCreate);
	virtual BOOL DisplayPaint2(void* pDisplayContext, pvdInfoDisplayPaint2* pDisplayPaint);
	virtual void DisplayClose2(void* pDisplayContext);
	virtual void DisplayExit2();
protected:
	virtual bool PluginInfo2();
	virtual BOOL Init2();
	virtual void Exit2();
protected:
	pvdInit2_t fInit2;
	pvdExit2_t fExit2;
	pvdPluginInfo2_t fPluginInfo2;
	pvdFileOpen2_t fFileOpen2;
	pvdPageInfo2_t fPageInfo2;
	pvdPageDecode2_t fPageDecode2;
	pvdPageFree2_t fPageFree2;
	pvdFileClose2_t fFileClose2;
	//
	pvdTranslateError2_t fTranslateError2;
	pvdGetFormats2_t fGetFormats2;
	pvdReloadConfig2_t fReloadConfig2;
	//
	pvdCancel2_t fCancel2;
	//
	pvdDisplayInit2_t fDisplayInit2;
	pvdDisplayAttach2_t fDisplayAttach2;
	pvdDisplayCreate2_t fDisplayCreate2;
	pvdDisplayPaint2_t fDisplayPaint2;
	pvdDisplayClose2_t fDisplayClose2;
	pvdDisplayExit2_t fDisplayExit2;
	
	//pvdInfoPlugin2 m_PluginInfo;
public: // API Helper functions
	// 0-информация, 1-предупреждение, 2-ошибка
	static void __stdcall apiMessageLog(void *apCallbackContext, const wchar_t* asMessage, UINT32 anSeverity);
	// asExtList может содержать '*' (тогда всегда TRUE) или '.' (TRUE если asExt пусто)
	static BOOL __stdcall apiExtensionMatch(wchar_t* asExtList, const wchar_t* asExt);
	// Просто вызвать 
	static BOOL __stdcall apiCallSehed(pvdCallSehedProc2 CalledProc, LONG_PTR Param1, LONG_PTR Param2, LONG_PTR* Result);
	static int __stdcall apiSortExtensions(wchar_t* pszExtensions);
	static int __stdcall apiMulDivI32(int a, int b, int c);  // a * (__int64)b / c;
	static UINT __stdcall apiMulDivU32(UINT a, UINT b, UINT c);
	static UINT __stdcall apiMulDivU32R(UINT a, UINT b, UINT c);
	static int __stdcall apiMulDivIU32R(int a, UINT b, UINT c);
};
