
#pragma once

/**************************************************************************
Copyright (c) 2009 Skakov Pavel
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

#include "PictureView.h"
#include "PVDInterface/PictureViewPlugin.h"
#include <vector>
#include <map>
#include "PictureView_Dlg.h"

#include "PVDModuleBase.h"
#include "PVDModule1.h"
#include "PVDModule2.h"

#include "DecodeParams.h"


#define PVD_IP_SUPPORTEDTYPES (PVD_IP_DECODE|PVD_IP_DISPLAY)

#define MAX_WHOLELOG_LINES 100

class CImage;
class CDecoderHandle;
class CDisplayHandle;
class CDecodeItem;

extern LPCSTR szPVDManager;

struct DecodeQueueItem
{
	// Статус данного элемента очереди: eItemEmpty/eItemFailed/eItemRequest/eItemDecoding/eItemReady
	DecodingStatus Status;
	
	//// Только для сравнения в функциях типа GetDecodedImage, IsImageReady
	//CImage* pImage;

	//// Файл
	//LPCWSTR pFullFileName;

	//// Индекс файла на панели
	//int      nRawPanelItem;

	// Параметры декодирования
	DecodeParams Params;

	// Кеширование содержимого. К сожалению, некоторым декодерам нужен чистый файл и с буфером
	// они не работают. Это например GFL & GhostScript, GDI+, DjVu?.
	LPBYTE pBuf;
	DWORD  lBuf;
	

	//// Декодированные и перемещенные в модуль дисплея данные
	//CModuleInfo *Display;
	//LPVOID pDraw;

	// Event, выставляется когда элемент "готов"
	HANDLE hReady;
};

#ifdef _DEBUG
	#define DECODER_QUEUE_SIZE 3 // чтобы в условиях стресса потестить - должен убрать из очереди неприоритетные
#else
	#define DECODER_QUEUE_SIZE 128
#endif


struct PVDHandleInfo
{
	BOOL   Used;
	DWORD  LastUsed;
	size_t MemoryUsed;
	CRefRelease* Handle;
};


// Общий класс, через который идут обращения к СУБдекодерам
// Экземпляр класса создается в каждом CImage, поэтому все контексты и параметры храним здесь
// В static переменных - данные относящиеся к модулям в общем
class CPVDManager
{
public:
	CPVDManager();
	~CPVDManager();

	static bool bCancelDecoding;
	static std::vector<CModuleInfo*> Plugins, Decoders, Displays;
	static bool CPVDManager::PluginCompare(CModuleInfo* p1, CModuleInfo* p2);
	static CModuleInfo* GetNextDecoder(const CModuleInfo* apDecoder, bool abForward);
	static CModuleInfo* pDefaultDisplay; // Что выбрано в настройке плагина - поле "Display module"
	static CModuleInfo* pActiveDisplay;  // А это то, через что идет вывод СЕЙЧАС (может быть NULL)
	static std::vector<wchar_t*> sWholeLog;
	static bool bWholeLogChanged;
	static void AddLogString(const wchar_t* asModule, const wchar_t* asMessage);

	static void LoadPlugins2();
	static void ScanFolder(const wchar_t* asFrom);
	static CModuleInfo* LoadPlugin(const wchar_t* asFrom, WIN32_FIND_DATAW& fnd, bool abShowErrors);
	static CModuleInfo* FindPlugin(const wchar_t* asFile, bool abShowErrors);
	static void SortPlugins2();
	static void UnloadPlugins2();
	static bool CreateVersion1(CModuleInfo* plug, bool lbForceWriteReg);
	static bool CreateVersion2(CModuleInfo* plug, bool lbForceWriteReg);
	static bool IsSupportedExtension(const wchar_t *pFileName, LPVOID pFileData=NULL, size_t nFileDataSize=0);
	
	static bool DisplayAttach();
	static bool DisplayDetach(BOOL abErase=TRUE);
	static void DisplayExit();
	static void DisplayErase();

	static void DecoderExit();

	void OnTerminate();
	
	//const wchar_t* GetName();

	static BOOL __stdcall DecodeCallback2(void *pDecodeCallbackContext2, UINT32 iStep, UINT32 nSteps,
										  pvdInfoDecodeStep2* pImagePart);


	void OnFirstImageOpened(CImage *apImage);

	
	// Поддержка очереди декодирования
protected:
	// По окончании декодирования должен проверять, текущее это изображение,
	// и если да - передернуть Invalidate, чтобы пошло улучшенное отображение
	static DWORD CALLBACK DecodingThread(LPVOID pParam);
	BOOL DecodingThreadStep();
	HANDLE mh_Thread, mh_Request, mh_Alive; DWORD mn_ThreadId; DWORD mn_AliveTick;
	bool CheckDecodingThread();
	
	// 128 - это даже много. в реале в очередь будем помещено максимум (6..9):
	// фрейм текущего изображения + 2 его следующих фрейма
	// следующее изображение (первый фрейм)
	// предыдущее изображение (первый фрейм)
	// + если декодированный фрейм из анимированной картинки - в очередь будет помещен следующий фрейм
	DecodeQueueItem m_DecoderQueue[DECODER_QUEUE_SIZE];
	std::vector<CDecoderHandle*> m_DecoderRelease;
	CRITICAL_SECTION csDecoderQueue;
	
public:
	// Возможно нужно еще что-то для определения сможем ли мы вообще открыть файл.

	// Функция должна повысить приоритет декодирования этой страницы
	// до максимального и дождаться окончания декодирования
	CImage* GetDecodedImage(DecodeParams *apParams);
	
	// Поместить в очередь декодирования
	BOOL RequestDecodedImage(DecodeParams *apParams);

	// Поместить в очередь на декодирование
	// а) следующий/предыдущий фрейм текущего изображения и
	// б) следующую/предыдущую картинку
	void RequestPreCache(DecodeParams *apParams);
	
	// Остановить декодирование изображения и (или) удалить его из очереди
	// Это например когда был запрошен рендер с улучшением (после зума), но 
	// пользователь опять начал зумить - тогда масштаб, запрошенный в предыдущий
	// раз становится неактуальным, и декодирование в этом масштабе нужно отменить
	// Ну или КАК МИНИМУМ сбросить флаг eRenderActivateOnReady
	//void CancelImageDecoding(CImage* apImage, DecodeParams aParams);
	
	// Может потребоваться при начале PAN изображения. По идее в этот
	// момент нужно переключиться (... ?)
	bool IsImageReady(DecodeParams *apParams);

	// 1) Выполнить перенос данных из контекста декодера в контекст дисплея
	// 2) Если нужно - передернуть окно отрисовки
	bool OnItemReady(CDecodeItem* apItem);
	
	void CloseDecoder(CDecodeItem* apItem);
	void CloseDisplay(CDecodeItem* apItem);
	
public:
	PVDHandleInfo* OnDecoderHandleCreated(CRefRelease* pHandle);
	PVDHandleInfo* OnDisplayHandleCreated(CRefRelease* pHandle);
	void OnDecoderHandleClosed(CRefRelease* pHandle);
	void OnDisplayHandleClosed(CRefRelease* pHandle);
	
	// Вызывается из любого потока для освобождения старых хэндлов
	// nRequiredMem указывается необходимый размер освобождаемой памяти
	void ReleaseUnusedHandles(size_t nRequiredMem = 0);
	
	// Вернуть примерный объем занимаемой памяти различными хэндлами
	size_t TotalMemoryAmountDecoder();
	size_t TotalMemoryAmountDisplay();
	
protected:
	CRITICAL_SECTION mcs_HandleManager;
	std::map<CRefRelease*,PVDHandleInfo*> m_HandleManager;

protected:
	// Если очередь заполнена - нужно убрать из очереди элемент с наименьшим приоритетом
	// Блокируется через csDecoderQueue. Возвращает индекс в m_DecoderQueue.
	int AppendDecoderQueue(DecodeParams *apParams);
	
	// Вернуть элемент "панели" с учетом, что в apParams может быть "относительный" индекс
	CImage* GetImageFromParam(DecodeParams *apParams);
	
	//
	bool ImageOpen(CImage* apImage, DecodeParams *apParams);
	bool Open(CImage* apImage, CDecoderHandle** ppFile, const wchar_t *pFileName, i64 lFileSize, const u8 *pBuffer, uint lBuffer, DecodeParams *apParams);
	bool OpenWith(CImage* apImage, CDecoderHandle** ppFile, CModuleInfo *pDecoder, const wchar_t *pFileName, i64 lFileSize, const u8 *pBuffer, uint lBuffer);
	void CloseData(CDecodeItem* apItem);
	//bool Decode(CImage *apImage, CDecoderHandle* pFile, CDisplayHandle** ppDraw, bool abResetTick);
	bool DecodePixels(CDecodeItem* apItem, bool abResetTick);
	bool PixelsToDisplay(CDecodeItem* apItem);
	//void ResetProcessed(CImage* apImage);
};

extern CPVDManager g_Manager;
