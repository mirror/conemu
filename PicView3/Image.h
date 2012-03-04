
#pragma once

/**************************************************************************
Copyright (c) 2011 Maximus5
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


class CPVDManager;
//class DirectDrawSurface;
class CModuleInfo;
class CDecoderHandle;
class CDisplayHandle;
struct DecodeParams;
class CImagePages;
class CDecodeItem;
class CDisplayHandlePtr;

#include "PictureView_FileName.h"
#include "PVDInterface/PictureViewPlugin.h"

#include "RefRelease.h"
#include "ImageInfo.h"

#include <vector>

// Унаследован от RefRelease, т.к. ссылка на объект может использоваться
// в разных нитях, например в очереди декодирования
class CImage : public CRefRelease
{
public:
	CImage(LPCSTR asFrom, const wchar_t *apFileName, int anPanelItemRaw);
	static void OnTerminate();
protected:
	virtual ~CImage();
public:
	//bool ImageOpen(DecodeParams *apParams, const unsigned char *buf /*= NULL*/, int lBuf /*= 0*/);
	void CheckPages(uint anNewPages);
	void Close();
	void CloseImage();
	void CloseDisplay();
	SIZE GetDefaultScaleSize();

#ifdef _DEBUG
	virtual void AddRef(LPCSTR asFrom)
	{
		CRefRelease::AddRef(asFrom);
	};
	virtual int Release(LPCSTR asFrom)
	{
		int i = CRefRelease::Release(asFrom);
		return i;
	};
#endif

private:
	CImagePages* mp_Pages;
//private:
//	bool ImageDecode(void);
public:
	void CheckPagesClosed();

	//CPVDManager *Decoder; -- данные инкапсулированы в CImage

	//int mi_DecoderIndex;
private:
	// Информационно, каким декодером открыли(открывали) в последний раз
	CModuleInfo* mp_Decoder; // RefRelease
public:
	void SetDecoder(CModuleInfo* apDecoder);
	CModuleInfo* GetDecoder();


	// Это Все-таки должно быть в CImage, т.к. даже если файл был декодирован
	// возможен вариант, когда результат декодирования не смог показать ни один модуль дисплея,
	// в этом случае необходимо ПРОДОЛЖИТЬ перебор декодеров, пока они не кончатся,
	// или пока не отработает какой-то модуль дисплея.
	WARNING("Изменена логика mpb_Processed ==> mpb_DecoderFailed");
	// true ставится не ПЕРЕД попыткой открытия декодером,
	// а ПОСЛЕ того, как он обломается, или обломаются все модули дисплея
	// на данных, которые вернул этот декодер.
private:
	//bool* mpb_DecoderFailed; uint mn_DecoderFailedCount; // Какими декодерами уже пытались открыть файл
	static CRITICAL_SECTION mcs_DecoderFailed;
	static bool mb_CSInitialized;
	std::vector<const void*> mp_DecoderFailed;
public:
	//void CheckDecoderFailedSize(); -- не требуется
	bool WasDecoderFailed(CModuleInfo* apDecoder);
	void SetDecoderFailed(CModuleInfo* apDecoder);
	void ResetDecoderFailed();


	WARNING("Сделать Private");
	CDecoderHandle* mp_File; // активный, используется(!) в данный момент в нити дисплея
	void DecoderHandleReleased(CDecoderHandle* apFile);

	WARNING("Сделать Private");
	//CDisplayHandle* mp_Draw; // активный, используется(!) в данный момент в нити дисплея

	CDecoderHandle* GetDecoderHandle(LPCSTR asFrom);
	bool GetDrawHandle(CDisplayHandlePtr& rDraw, DecodeParams *apParams = NULL);

	void StoreDecodedFrame(CDecodeItem* apFrame);

	//bool mb_ImageOpened;
	////wchar_t* mps_FileName;
	//void *mp_ImageContext;
	////TODO("Переименовать mp_Data в mp_DecoderModule, когда то что раньше было Decoder переименуется в g_Manager");
	//CModuleInfo* mp_DecoderModule;

	//TODO("Переименовать Display в mp_DisplayModule");
	//CModuleInfo *Display;
	//LPVOID pDraw;
	//pvdInfoPage2 InfoPage;

	CUnicodeFileName FileName;
	LPCWSTR pszFileNameOnly;

	
	ImageInfo Info;
	//uint lWidth, lHeight, nBPP;
	//uint lDecodedWidth, lDecodedHeight, nDecodedBPP;
	//DWORD lOpenTime, lStartOpenTime;
	//DWORD lTimeOpen, lTimeDecode, lTimeTransfer, lTimePaint;
	//BOOL  bTimePaint;
	//uint nPages, nPage;
	//uint Animation;
	//uint lFrameTime;
	//
	//wchar_t DecoderName[0x80], FormatName[0x80], Compression[0x80], Comments[0x100];

	bool IsValid();
	bool IsActive();
	int PanelItemRaw();
	//void SetPanelItemRaw(uint nRaw);
private:
	int nPanelItemRaw;
};
//typedef CImage* PImageInfo;
