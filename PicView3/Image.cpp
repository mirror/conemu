
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


#include "PictureView.h"
#include "PictureView_Lang.h"
#include "PVDManager.h"
#include "Image.h"
#include "ImagePages.h"
#include "FileMap.h"
#include "DecoderHandle.h"
#include "DisplayHandle.h"

extern RECT GetDisplayRect();

CRITICAL_SECTION CImage::mcs_DecoderFailed;
bool CImage::mb_CSInitialized = false;

LPCSTR szImage = "CImage";

CImage::CImage(LPCSTR asFrom, const wchar_t *apFileName, int anPanelItemRaw)
	: CRefRelease(asFrom)
{
	if (!mb_CSInitialized)
	{
		mb_CSInitialized = TRUE;
		InitializeCriticalSection(&mcs_DecoderFailed);
	}

	FileName = apFileName;
	nPanelItemRaw = anPanelItemRaw;
	pszFileNameOnly = g_Panel.ItemPointToName((LPCWSTR)FileName);

	mp_Decoder = NULL;
	mp_File = NULL;
	//mp_Draw = NULL;

	mp_Pages = new CImagePages(this);

#if 0
	mi_DecoderIndex = -1;
	mp_ImageContext = NULL;
	mp_DecoderModule = NULL;
	mb_ImageOpened = false;
	//mps_FileName = NULL;
	mpb_DecoderFailed = NULL; mn_DecoderFailedCount = 0;

	//Decoder = NULL;
	pDraw = NULL; Display = NULL;
#endif

	//FileNamePrefix[0] = 0; // \\?\ ...
	//FileNameData[0] = 0;

	////bSelected = false;
	//lWidth = lHeight = nBPP = lDecodedWidth = lDecodedHeight = nDecodedBPP = 0;
	////OpenTimes[0] = 0;
	//lOpenTime = lStartOpenTime = 0;
	//lTimeOpen = lTimeDecode = lTimeTransfer = lTimePaint = 0;
	//bTimePaint = FALSE;
	//nPages = nPage = Animation = lFrameTime = 0;

	//DecoderName[0] = 0; FormatName[0] = 0; Compression[0] = 0; Comments[0] = 0;

	//Decoder = new CPVDManager(this);
	//_ASSERTE(Decoder);

	//dds = new DirectDrawSurface;
	
	//memset(&InfoPage, 0, sizeof(InfoPage));
}

CImage::~CImage()
{
	// Освободить указатель на декодер
	SetDecoder(NULL);

	// И все, что осталось
	Close();
	
	//_ASSERTE(mp_ImageContext == NULL);
	//_ASSERTE(mb_ImageOpened == FALSE);
	//_ASSERTE(pDraw == NULL);
	//if (mp_DecoderModule) // pData это ссылка на pPlugin, а они освобождаются в CPVDManager::UnloadPlugins2
	//	mp_DecoderModule = NULL;

	//SafeFree(mpb_DecoderFailed); mn_DecoderFailedCount = 0;
}

void CImage::OnTerminate()
{
	if (mb_CSInitialized)
	{
		mb_CSInitialized = false;
		DeleteCriticalSection(&mcs_DecoderFailed);
	}
}

void CImage::Close()
{
	if (!gnDecoderThreadId || GetCurrentThreadId() == gnDecoderThreadId)
	{
		CloseImage();
	}
	
	CloseDisplay();

	// Удалить указатели на страницы
	if (mp_Pages)
	{
		delete mp_Pages;
		mp_Pages = NULL;
	}
}

void CImage::DecoderHandleReleased(CDecoderHandle* apFile)
{
	if (mp_File == apFile)
		mp_File = NULL;
}

void CImage::CloseImage()
{
	SafeRelease(mp_File,szPVDManager);

	// mpb_DecoderFailed чистить не нужно!
	// возможен вариант, когда результат декодирования не смог показать ни один модуль дисплея,
	// в этом случае необходимо ПРОДОЛЖИТЬ перебор декодеров, пока они не кончатся,
	// или пока не отработает какой-то модуль дисплея.
}

void CImage::CloseDisplay()
{
	//SafeRelease(mp_Draw,szImage);
	if (mp_Pages)
		mp_Pages->CloseDisplay();
}

// В процессе декодирования может появиться большее количество страниц, чем
// было обозначено при первичном обнюхивании файла
void CImage::CheckPages(uint anNewPages)
{
	Info.UpdatePageCount(anNewPages);
}

bool CImage::IsValid()
{
	if (!this)
	{
		_ASSERTE(this!=NULL);
		return false;
	}
	TODO("Хорошо бы еще валидацию объекта провести, вдруг некорректную ссылку используют");
	return true;
}

bool CImage::IsActive()
{
	if (!IsValid())
		return false;
	
	return (nPanelItemRaw == g_Panel.GetActiveRawIdx());
}

int CImage::PanelItemRaw()
{
	_ASSERTE(nPanelItemRaw >= 0);
	return nPanelItemRaw;
}

//void CImage::SetPanelItemRaw(uint nRaw)
//{
//	TODO("При переходах может быть -1 и g_Plugin.FarPanelInfo.ItemsNumber. В этом случае они потом еще будут меняться");
//	if (nPanelItemRaw != nRaw) {
//		nPanelItemRaw = nRaw;
//		nPage = 0; // сброс номера страницы
//	}
//}

//TODO("Убрать аргумент piSubDecoder");
//bool CImage::ImageOpen(DecodeParams *apParams, const unsigned char *buf /*= NULL*/, int lBuf /*= 0*/)
//{
//	_ASSERTE(!FileName.IsEmpty());
//
//	// Должен быть уже вызван, но проверим
//	g_Plugin.InitCMYK(FALSE); // Дождаться его завершения
//
//
//	i64 lFileSize;
//	
//	FileMap Map((LPCWSTR)FileName);
//	lFileSize = Map.lSize;
//	if (!buf || !lBuf)
//		if (Map.MapView())
//		{
//			buf = Map.pMapping;
//			lBuf = (int)Min<i64>(Map.lSize, ~0u);
//			//if (lBuf) -- IN_APL этого не пережил
//			//	lFileSize = 0; // все данные уже в памяти
//		}
//		else
//		{
//			buf = NULL;
//			lBuf = 0;
//		}
//
//	bool result = false;
//
//	TODO("Перенести в начало функции");
//	this->lStartOpenTime = timeGetTime();
//
//
//	// Сбрасывать нельзя! Мы сейчас можем находится в вызове AltPgUp например (смена декодера)
//	//Decoder->mi_SubDecoder = *piSubDecoder;
//	
//lReOpen2:
//	memset(&InfoPage, 0, sizeof(InfoPage)); // !!! cbSize не заполнять
//	result = g_Manager.Open(this, (LPCWSTR)FileName, lFileSize, buf, lBuf, apParams);
//
//	if (!result || (g_Plugin.FlagsWork & FW_TERMINATE))
//	{
//		return false;
//	}
//
//	// Для первой картинки серии - в(ы)ключить (если разрешено) модификатор
//	// управляющий поведением PgUp/PgDn (листают страницы или файлы)
//	if (apParams->Flags & eRenderFirstImage)
//	{
//		g_Manager.OnFirstImageOpened(this);
//		if ((g_Plugin.FlagsWork & FW_TERMINATE))
//			result = false;
//	}
//		
//
//	if (result)
//	{
//		TODO("Хорошо бы разрулить ImageDecode и запуск нити Display. Собственно декодирование можно начать еще ДО создания DX и прочего");
//		if (!ImageDecode())
//		{
//			WARNING("Если ImageDecode обламывается - то мы зацикливаемся, т.к. ImageOpen выбирает тот же декодер!");
//			WARNING("В любом случае нужно оптимизировать окрытие? Не разрушать pPVDDecoder и его локальную переменную bProcessed");
//			uint nDecoders = (uint)CPVDManager::Decoders.size();
//			if (nDecoders <= 1)
//				return false;
//
//			if (g_Plugin.FlagsWork & FW_TERMINATE)
//				return false; // Плагин закрывается
//
//			mi_SubDecoder++;
//			if (mi_SubDecoder >= nDecoders)
//				mi_SubDecoder = 0;
//
//			goto lReOpen2;
//		}
//		else
//		{
//			// Успешное декодирование. Сброс флагов "Декодер проверен для этого изображения".
//			g_Manager.ResetProcessed(this);
//		}
//	}
//	
//	return result;
//}

//bool CImage::ImageDecode(void)
//{
//	//this->iPage = 0;
//	const uint nPages = this->nPages;
//	//_ASSERTE(nPage < nPages);
//	// Допустимая ситуация. Открыли djvu через djvu.pvd
//	// Переключились на другой декодер (AltPgDn), согласился Shell.pvd, а он умеет только
//	// превьюшку - то есть только одну страницу.
//	if (nPage >= nPages) 
//		nPage = 0;
//	
//	WARNING("!!! Перенос в дисплей нужно выполнять в нити дисплея !!!");
//		
//	// InfoPage - мог быть уже получен, тогда декодер не будет повторно дергать pvdPageInfo
//	bool result = g_Manager.Decode(this, &pDraw, false);
//	// Количество страниц (если оно изменилось) обновляется через CheckPages(...)
//	// Этот AI убираем. Пусть PVD_IIF_ANIMATED выставляет сам PVD
//	//if (nPages != this->nPages) // buggy GFL with animated GIF
//	//	this->Animation = 1;
//	// Пока вообще не закрываем
//	//TODO("Когда потребуется уточненное декодирование возможно придется не закрывать и с одной страницей");
//	//if (!result || this->nPages < 2)
//	//{
//	//	this->Decoder->Close();
//	//	//deletex(this->Decoder);
//	//}
//
//	if (!result)
//	{
//		g_Manager.Close(this);
//	} else {
//		//if (this == g_Plugin.Image[0])
//		if (IsActive())
//		{
//			// Иначе в GDI что-то не перерисовывается
//			TODO("Не помогло. тут (this != g_Plugin.Image[0])");
//			Invalidate(g_Plugin.hWnd);
//		}
//
//		//if (g_Plugin.FlagsWork & FW_JUMP_DISABLED) {
//		//
//		//	this->SetPanelItemRaw(PANEL_ITEM_UNAVAILABLE);
//		//	
//		//}
//	}
//	
//	return result;
//}




SIZE CImage::GetDefaultScaleSize()
{
	RECT ParentRect = GetDisplayRect(); //{0, 0, 1280, 960};
	
	//const вызывает инициализацию переменных ДО входа в функцию (по возможности). что нам не подходит - хочется видеть последовательность действий!
	int lScreenWidth = ParentRect.right - ParentRect.left;
	int lScreenHeight = ParentRect.bottom - ParentRect.top;

	RECT ImgRect = {0,0,lScreenWidth,lScreenHeight};
	
	// Тут используем НЕ this->lWidth & this->lHeight
	// а InfoPage.lWidth & InfoPage.lHeight. Т.к. именно они
	// будут содержать актуальную информацию о декодируемой СТРАНИЦЕ многостраничного документа
	
	u32 Zoom;
	if (g_Plugin.ZoomAuto || g_Plugin.FlagsWork & FW_QUICK_VIEW)
	{
		//const вызывает инициализацию переменных ДО входа в функцию (по возможности). что нам не подходит - хочется видеть последовательность действий!
		u32 ZoomW = MulDivU32(lScreenWidth, 0x10000, Info.lWidth /*dds->m_lWorkWidth*/);
		u32 ZoomH = MulDivU32(lScreenHeight, 0x10000, Info.lHeight /*dds->m_lWorkHeight*/);
		Zoom = (g_Plugin.ZoomAuto == ZA_FIT || g_Plugin.FlagsWork & FW_QUICK_VIEW) ? Min(ZoomW, ZoomH) : Max(ZoomW, ZoomH);
		if (!Zoom)
			Zoom = 1;
		if (g_Plugin.bAutoZoomMin && Zoom < g_Plugin.AutoZoomMin)
			Zoom = g_Plugin.AutoZoomMin;
		if (g_Plugin.bAutoZoomMax && Zoom > g_Plugin.AutoZoomMax)
			Zoom = g_Plugin.AutoZoomMax;
	}
	else
	{
		Zoom = g_Plugin.AbsoluteZoom ? g_Plugin.AbsoluteZoom : g_Plugin.Zoom;
	}

	
	//const вызывает инициализацию переменных ДО входа в функцию (по возможности). что нам не подходит - хочется видеть последовательность действий!
	int lWidth = MulDivU32R(Info.lWidth, Zoom, 0x10000);
	int lHeight = MulDivU32R(Info.lHeight, Zoom, 0x10000);
	
	SIZE sz = {lWidth,lHeight};
	
	return sz;
}


void CImage::SetDecoder(CModuleInfo* apDecoder)
{
	// mp_Decoder используется только информационно, при переборе декодеров,
	// наряду с информацией WasDecoderFailed/SetDecoderFailed/ResetDecoderFailed

	//_ASSERTE(!apDecoder || (mp_File && (apDecoder == mp_File->Decoder())));

	if (mp_Decoder != apDecoder)
	{
		if (apDecoder)
			apDecoder->AddRef(szImage);

		if (mp_Decoder)
			mp_Decoder->Release(szImage);

		mp_Decoder = apDecoder;
	}
}

CModuleInfo* CImage::GetDecoder()
{
	WARNING("Убрать? Должен быть только в mp_File?");
	return mp_Decoder;
}


// Это Все-таки должно быть в CImage, т.к. даже если файл был декодирован
// возможен вариант, когда результат декодирования не смог показать ни один модуль дисплея,
// в этом случае необходимо ПРОДОЛЖИТЬ перебор декодеров, пока они не кончатся,
// или пока не отработает какой-то модуль дисплея.
// WARNING!!! Изменена логика mpb_DecoderFailed ==> mpb_DecoderFailed
// true ставится не ПЕРЕД попыткой открытия декодером,
// а ПОСЛЕ того, как он обломается, или обломаются все модули дисплея
// на данных, которые вернул этот декодер.
//void CImage::CheckDecoderFailedSize()
//{
//	uint nDecoders = (uint)CPVDManager::Decoders.size();
//	if (!mpb_DecoderFailed || mn_DecoderFailedCount != nDecoders)
//	{
//		SafeFree(mpb_DecoderFailed);
//		mn_DecoderFailedCount = nDecoders;
//		mpb_DecoderFailed = (bool*)calloc(nDecoders,sizeof(*mpb_DecoderFailed));
//	}
//}

bool CImage::WasDecoderFailed(CModuleInfo* apDecoder)
{
	bool lbFailed = false;

	EnterCriticalSection(&mcs_DecoderFailed);
	for (std::vector<const void*>::iterator iter = mp_DecoderFailed.begin(); iter != mp_DecoderFailed.end(); ++iter)
	{
		const void* p = *iter;
		if (p == (const void*)apDecoder)
		{
			lbFailed = true;
			break;
		}
	}
	LeaveCriticalSection(&mcs_DecoderFailed);
	
	return lbFailed;
}

void CImage::SetDecoderFailed(CModuleInfo* apDecoder)
{
	EnterCriticalSection(&mcs_DecoderFailed);
	if (!WasDecoderFailed(apDecoder))
	{
		mp_DecoderFailed.push_back((const void*)apDecoder);
	}
	LeaveCriticalSection(&mcs_DecoderFailed);
}

void CImage::ResetDecoderFailed()
{
	EnterCriticalSection(&mcs_DecoderFailed);
	mp_DecoderFailed.clear();
	LeaveCriticalSection(&mcs_DecoderFailed);
}

// На CDecoderHandle сразу делается AddRef, вызывающая функция должна его Release
CDecoderHandle* CImage::GetDecoderHandle(LPCSTR asFrom)
{
	if (mp_File)
		mp_File->AddRef(asFrom);
	return mp_File;
}

// На CDisplayHandle сразу делается AddRef, вызывающая функция должна его Release
bool CImage::GetDrawHandle(CDisplayHandlePtr& rDraw, DecodeParams *apParams /*= NULL*/)
{
	//if (!Info.lDecodedWidth || !Info.lDecodedHeight)
	//	return NULL; // Еще не декодировано!
	if ((this == NULL) || (mp_Pages == NULL))
	{
		_ASSERTE(this && mp_Pages);
		return false;
	}

	bool bValid = false;

	// Вернуть подходящий дескриптор дисплея для текущего фрейма (допускается NULL если еще не готово)
	if (apParams)
		bValid = mp_Pages->GetDrawHandle(rDraw, apParams->nPage, apParams);
	else
		bValid = mp_Pages->GetDrawHandle(rDraw, Info.nPage);

	return bValid;
}

void CImage::StoreDecodedFrame(CDecodeItem* apFrame)
{
	_ASSERTE(GetCurrentThreadId() == gnDisplayThreadId);
	if ((this == NULL) || (mp_Pages == NULL))
	{
		_ASSERTE(this && mp_Pages);
		return;
	}

	mp_Pages->Store(apFrame);
}

void CImage::CheckPagesClosed()
{
	if (this == NULL)
	{
		_ASSERTE(this);
		return;
	}

	if (mp_Pages)
		mp_Pages->CheckPagesClosed();
}
