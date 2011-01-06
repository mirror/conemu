
#pragma once

class DirectDrawSurface;


enum {
	eRenderFull = 1,
	eRenderScale = 2, // под размер экрана
	eRenderRectScale = 4, // декодирование только области изображения
} DecodingFlags;

enum {
	eCurrentImageCurrentPage, // highest
	eCurrentImageNextPage,
	eNextImage,
} DecodingPriority;


typedef struct tag_DecodeItem
{
	ImageInfo* pImage; // Только для сравнения в функциях типа GetDecodedImage, IsImageReady
	wchar_t* pFullFileName; // КОПИЯ памяти. Разрушать должен сам DecoderCache
	int nFileIndex; // Индекс файла на панели. может быть -1 если панели не соответствует
	
	// Кеширование содержимого. К сожалению, некоторым декодерам нужен чистый файл и с буфером
	// они не работают. Это например GFL & GhostScript.
	LPBYTE pBuf;
	DWORD  lBuf;
	
	DecodingFlags Flags;
	DecodingPriority Priority;
	
	RECT rcSource;
	SIZE szTarget;
	
	LPVOID pOutputContext; // потом
	DirectDrawSurface* dds; // а это сейчас
} DecodeItem;

class DecoderCache
{
public:
	
public:
	// По окончании декодирования должен проверять, текущее это изображение,
	// и если да - передернуть Invalidate, чтобы пошло улучшенное отображение
	DWORD CALLBACK DecodingThread(LPVOID pParam);
	
public:
	// Возможно нужно еще что-то для определения сможем ли мы вообще открыть файл.

	// Функция должна повысить приоритет декодирования этой страницы
	// до максимального и дождаться окончания декодирования
	BOOL GetDecodedImage(int nFileIndex, UINT nPage, DecodingFlags, pvdInfoDecode2* ...);
	// pFullFileName это если открывается изображение не имеющее соответствия файлу на панели
	BOOL GetDecodedImage(const wchar_t* pFullFileName, UINT nPage, DecodingFlags, pvdInfoDecode2* ...);
	
	// Поместить в очередь декодирования
	BOOL RequestDecodedImage(int nFileIndex, UINT nPage, DecodingPriority, DecodingFlags, );
	// pFullFileName это если открывается изображение не имеющее соответствия файлу на панели
	BOOL RequestDecodedImage(const wchar_t* pFullFileName, UINT nPage, DecodingPriority, DecodingFlags, );

	// Может потребоваться при начале драга изображения. По идее в этот
	// момент нужно переключиться 	
	BOOL IsImageReady(Image*, DecodingFlags, RECT,...);
};
