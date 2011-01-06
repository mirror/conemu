#pragma once

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

#include "PictureView.h"
#include "PVDInterface/PictureViewPlugin.h"
#include <vector>
#include "PictureView_Dlg.h"

#include "PVDDecoderBase.h"
#include "PVDDecoder1.h"
#include "PVDDecoder2.h"


#define PVD_IP_SUPPORTEDTYPES (PVD_IP_DECODE|PVD_IP_DISPLAY)

#define MAX_WHOLELOG_LINES 100

class ImageInfo;

// Общий класс, через который идут обращения к СУБдекодерам
// Экземпляр класса создается в каждом ImageInfo, поэтому все контексты и параметры храним здесь
// В static переменных - данные относящиеся к модулям в общем
class PVDManager
{
public:
	PVDManager(ImageInfo* apImage);
	~PVDManager();

	static bool bCancelDecoding;
	static std::vector<ModuleData*> Plugins, Decoders, Displays;
	static bool PVDManager::PluginCompare(ModuleData* p1, ModuleData* p2);
	static uint GetNextDecoder(uint iDecoder, bool abForward);
	static ModuleData *pDefaultDisplay; // Что выбрано в настройке плагина - поле "Display module"
	static ModuleData *pActiveDisplay;  // А это то, через что идет вывод СЕЙЧАС (может быть NULL)
	static std::vector<wchar_t*> sWholeLog;
	static bool bWholeLogChanged;
	static void AddLogString(const wchar_t* asModule, const wchar_t* asMessage);

	static void LoadPlugins2();
	static void ScanFolder(const wchar_t* asFrom);
	static ModuleData* LoadPlugin(const wchar_t* asFrom, WIN32_FIND_DATAW& fnd, bool abShowErrors);
	static ModuleData* FindPlugin(const wchar_t* asFile, bool abShowErrors);
	static void SortPlugins2();
	static void UnloadPlugins2();
	static bool CreateVersion1(ModuleData* plug, bool lbForceWriteReg);
	static bool CreateVersion2(ModuleData* plug, bool lbForceWriteReg);
	static bool IsSupportedExtension(const wchar_t *pFileName);
	
	static bool DisplayAttach();
	static bool DisplayDetach();
	static void DisplayExit();
	
	const wchar_t* GetName();

	bool Open(const wchar_t *pFileName, i64 lFileSize, const u8 *pBuffer, uint lBuffer, pvdInfoPage2 &InfoPage);
	bool OpenWith(ModuleData *pData, const wchar_t *pFileName, i64 lFileSize, const u8 *pBuffer, uint lBuffer, pvdInfoPage2 &InfoPage);
	void Close(void);
	bool Decode(ImageInfo *Image, LPVOID* ppDrawContext, bool abResetTick, pvdInfoPage2 &InfoPage);

	static BOOL __stdcall DecodeCallback2(void *pDecodeCallbackContext2, UINT32 iStep, UINT32 nSteps,
										  pvdInfoDecodeStep2* pImagePart);

	ImageInfo* mp_Image;
	uint mi_SubDecoder;
	bool mb_ImageOpened;
	bool* mb_Processed; int mn_ProcessedSize; // Какими декодерами уже пытались открыть файл
	wchar_t* mps_FileName;
	void *mp_ImageContext;
	//PVDDecoderBase *pSubPlugin; // Subplugin instance
	ModuleData* mp_Data;

	void ResetProcessed();
};
