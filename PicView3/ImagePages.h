
#pragma once

/**************************************************************************
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

#include <vector>

/*

 Кеширование дескрипторов дисплея для одного CImage
  
 WARNING! Вызываться должен только в нити дисплея!

*/

class CImage;

struct ImagePagesItem
{
	//bool bUsed; // true для занятых ячеек
	bool bScreenSize; // при освобождении неиспользуемых - эти по возвожности оставлять
	DWORD nLastAccess; // GetTickCount() - время последнего доступа в этой ячейке
	uint nPage; // для упрощения доступа
	
	// Далее идут собственно данные и дескрипторы
	ImageInfo Info;
	DecodeParams Params;
	CDisplayHandle* pDraw;

	ImagePagesItem();
	~ImagePagesItem();
	void Assign(CDecodeItem* apFrame);
};

class CImagePages
{
protected:
	CRITICAL_SECTION csPages;

	bool mb_Animated;
	//uint mn_StoreSize; // для начала - выделим (mp_Image.nPages * 2) ячеек.
	std::vector<ImagePagesItem*> m_Store;
	
public:
	CImagePages(CImage* apImage);
	~CImagePages();

	void CheckPagesClosed();

	void CloseDisplay();

	void FreeUnused();

	// В apFrame сейчас должен быть как минимум - блок данных полученный из декодера
	// также может быть (но не обязан) дескриптор файла и (уже готовый) дескриптор дисплея
	void Store(CDecodeItem* apFrame);
	
	// Вернуть наиболее подходящий дескриптор дисплея
	// На CDisplayHandle сразу делается AddRef, вызывающая функция должна его Release
	// CDisplayHandle содержит указатель на модуль дисплея и контекст для отрисовки
	bool GetDrawHandle(CDisplayHandlePtr& rDraw, uint anPage, DecodeParams *apParams = NULL);
};
