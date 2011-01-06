
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

// Version 1 (fixed)
typedef UINT32 (__stdcall *pvdInit_t)(void);
typedef void (__stdcall *pvdExit_t)(void);
typedef void (__stdcall *pvdPluginInfo_t)(pvdInfoPlugin *pPluginInfo);
typedef BOOL (__stdcall *pvdFileOpen_t)(const char *pFileName, INT64 lFileSize, const BYTE *pBuf, UINT32 lBuf, pvdInfoImage *pImageInfo, void **ppContext);
typedef BOOL (__stdcall *pvdPageInfo_t)(void *pContext, UINT32 iPage, pvdInfoPage *pPageInfo);
typedef BOOL (__stdcall *pvdPageDecode_t)(void *pContext, UINT32 iPage, pvdInfoDecode *pDecodeInfo, pvdDecodeCallback DecodeCallback, void *pDecodeCallbackContext);
typedef void (__stdcall *pvdPageFree_t)(void *pContext, pvdInfoDecode *pDecodeInfo);
typedef void (__stdcall *pvdFileClose_t)(void *pContext);


class PVDDecoderVer1 : public PVDDecoderBase {
public:
	PVDDecoderVer1(ModuleData* apData);
	virtual ~PVDDecoderVer1();
	virtual bool LoadFormatsFromPvd();
	virtual bool InitPlugin(bool bAllowResort=false); // Загрузить, инициализировать, получить список форматов
public:
	virtual BOOL FileOpen2(const wchar_t *pFileName, INT64 lFileSize, const BYTE *pBuf, UINT32 lBuf, pvdInfoImage2 *pImageInfo);
	virtual BOOL PageInfo2(void *pImageContext, pvdInfoPage2 *pPageInfo);
	virtual BOOL PageDecode2(void *pImageContext, pvdInfoDecode2 *pDecodeInfo, pvdDecodeCallback2 DecodeCallback, void *pDecodeCallbackContext);
	virtual void PageFree2(void *pImageContext, pvdInfoDecode2 *pDecodeInfo);
	virtual void FileClose2(void *pImageContext);
protected:
	virtual bool PluginInfo2();
	virtual BOOL Init2();
	virtual void Exit2();
protected:
	pvdInit_t fInit;
	pvdExit_t fExit;
	pvdPluginInfo_t fPluginInfo;
	pvdFileOpen_t fFileOpen;
	pvdPageInfo_t fPageInfo;
	pvdPageDecode_t fPageDecode;
	pvdPageFree_t fPageFree;
	pvdFileClose_t fFileClose;
	//pvdInfoPlugin m_PluginInfo;
	static BOOL __stdcall DecodeCallbackWrap(void *pDecodeCallbackContext, UINT32 iStep, UINT32 nSteps);
	typedef struct tag_DecodeCallbackArg {
		pvdDecodeCallback2 DecodeCallback;
		void *pDecodeCallbackContext;
	} DecodeCallbackArg;
	typedef struct tag_ImageContextWrap {
		UINT32 lWidth;             // [OUT] Ширина декодированной области (pImage)
		UINT32 lHeight;            // [OUT] Высота декодированной области (pImage)
		void  *pImageContext;
		wchar_t *pFormatName;  // [OUT] название формата файла
		wchar_t *pCompression; // [OUT] алгоритм сжатия
		wchar_t *pComments;    // [OUT] различные комментарии о файле
	} ImageContextWrap;
	wchar_t* ConvertToWide(const char* psz);
};
