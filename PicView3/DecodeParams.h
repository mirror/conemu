
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


class CDecodeItem;

enum DecodingFlags
{
	// обычный режим
	eRenderFull = 1,
	// под размер экрана
	eRenderScale = 2,
	// декодирование только области изображения
	eRenderRectScale = 4,
	// Первое открытие файла, или нужен полный цикл подбора декодера.
	eRenderResetDecoder = 0x100,
	// Дождаться окончания декодирования
	eRenderWaitOpened = 0x1000,
	// Смена декодера пользователем
	eRenderNextDecoder = 0x10000,
	eRenderPrevDecoder = 0x20000,
	// "Усиленный" подбор декодера (первая картинка серии при ручном вызове?)
	eRenderForceDecoder = 0x40000,
	// Первая картинка серии
	eRenderFirstImage = 0x80000,
	// По окончании декодирование - отобразить на экране что получилось
	eRenderActivateOnReady = 0x100000,
	// при PgUp/PgDn в RawIndex указывается направление (-1, 1), а не индекс 
	eRenderRelativeIndex   = 0x200000,
	// (PgUp/PgDn/Home/End) листать до тех пор, пока не встретится файл, который сможем декодировать и показать
	eRenderFirstAvailable  = 0x400000,
	// ОСВОБОДИТЬ ДЕСКПРИПТОР
	eRenderReleaseDescriptor = 0x800000,
};

enum DecodingPriority
{
	eCurrentImageCurrentPage = 0, // highest
	eCurrentImageNextPage,
	eDecodeNextImage,
	eDecodeAny,
};

enum DecodingStatus
{
	eItemEmpty = 0,
	eItemFailed,
	eItemRequest,
	eItemReady,
	eItemDecoding,
};

struct DecodeParams
{
public:
	// DecodingFlags: eRenderFull|eRenderScale|eRenderRectScale|eRenderWaitOpened
	//                eRenderNextDecoder|eRenderPrevDecoder|eRenderForceDecoder
	//                eRenderFirstImage|eRenderActivateOnReady|eRenderRelativeIndex|eRenderFirstAvailable
	DWORD Flags;

	// Индекс файла на панели (или исходный индекс, при "относительном" декодировании)
	int   nRawIndex;
	// При декодировании следующего/предыдущего файла (eRenderRelativeIndex)
	int   iDirection;
	// Какой фрейм нужно декодировать
	int   nPage;
	
	// приоритет декодирования: eCurrentImageCurrentPage | eCurrentImageNextPage | eNextImage
	enum DecodingPriority Priority;

	// при первом вызове из OpenFilePluginW данные уже могут быть заполнены
	const unsigned char *buf /*= NULL*/;
	int lBuf /*= 0*/;
	
	//TODO
	RECT rcSource;
	SIZE szTarget;
	
	
	// For internal use!
	enum DecodingStatus *pResult;
	int nFromRawIndex;
	CDecodeItem *pDecodeItem;
	

public:	
	// Конструктор, оператор присваивания, методы
	DecodeParams()
	{
		memset(this, 0, sizeof(DecodeParams));
	}

	DecodeParams& operator=(DecodeParams& a);
	void OnItemReady();
	bool Compare(DecodeParams* pOther);
};
