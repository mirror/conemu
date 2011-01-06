
#include <windows.h>
#include "../PictureViewPlugin.h"
#include "../PVD2Helper.h"

#define DJVU_LIB

typedef __int32 i32;
typedef __int64 i64;
typedef unsigned __int8 u8;
typedef unsigned __int16 u16;
typedef DWORD u32;

extern HMODULE ghModule;

LPCWSTR gpszPluginKey = NULL;

pvdInitPlugin2 ip = {0};

#define PDJE_NOT_DJVU_HEADER      0x1001
#define PDJE_FILE_REQUIRED        0x80001001
#define PDJE_EXCEPTION            0x80001002
#define PDJE_FILE_NOT_FOUND       0x80001003
#define PDJE_NOT_ENOUGH_MEMORY    0x80001004
#define PDJE_INVALID_CONTEXT      0x80001005
#define PDJE_UNKNOWN_FORMAT       0x8000100B
#define PDJE_OLD_PICVIEW          0x8000100C

wchar_t gsLastErrFunc[128] = {0};

BOOL __stdcall pvdTranslateError2(DWORD nErrNumber, wchar_t *pszErrInfo, int nBufLen)
{
	if (!pszErrInfo || nBufLen<=0)
		return FALSE;

	switch (nErrNumber)
	{
	case PDJE_NOT_DJVU_HEADER:
		lstrcpynW(pszErrInfo, L"DjVu format tag not found", nBufLen); break;
	case PDJE_FILE_REQUIRED:
		lstrcpynW(pszErrInfo, L"Physical file required for DjvuLibre", nBufLen); break;
	case PDJE_EXCEPTION:
		lstrcpynW(pszErrInfo, L"Exception occurred", nBufLen); break;
	case PDJE_FILE_NOT_FOUND:
		lstrcpynW(pszErrInfo, L"File not found", nBufLen); break;
	case PDJE_NOT_ENOUGH_MEMORY:
		lstrcpynW(pszErrInfo, L"Not enough memory", nBufLen); break;
	case PDJE_INVALID_CONTEXT:
		lstrcpynW(pszErrInfo, L"Invalid context", nBufLen); break;
	case PDJE_UNKNOWN_FORMAT:
		lstrcpynW(pszErrInfo, L"Unsupported RAW format, codec is not available", nBufLen); break;
	case PDJE_OLD_PICVIEW:	
		lstrcpyn(pszErrInfo, L"Old PicView version, exiting", nBufLen); break;
	default:
		return FALSE;
	}
	return TRUE;
}










#define DDJVUAPI extern

// DJVUlibre, Types
typedef void ddjvu_context_t;
typedef void ddjvu_document_t;
typedef void ddjvu_job_t;
typedef void ddjvu_page_t;
typedef enum {
	DDJVU_JOB_NOTSTARTED, /* operation was not even started */
	DDJVU_JOB_STARTED,    /* operation is in progress */
	DDJVU_JOB_OK,         /* operation terminated successfully */
	DDJVU_JOB_FAILED,     /* operation failed because of an error */
	DDJVU_JOB_STOPPED     /* operation was interrupted by user */
} ddjvu_status_t;
typedef enum {
	DDJVU_ERROR,
	DDJVU_INFO,
	DDJVU_NEWSTREAM,
	DDJVU_DOCINFO,
	DDJVU_PAGEINFO,
	DDJVU_RELAYOUT,
	DDJVU_REDISPLAY,
	DDJVU_CHUNK,
	DDJVU_THUMBNAIL,
	DDJVU_PROGRESS,
} ddjvu_message_tag_t;
typedef struct ddjvu_message_any_s {
	ddjvu_message_tag_t   tag;
	ddjvu_context_t      *context;
	ddjvu_document_t     *document;
	ddjvu_page_t         *page;
	ddjvu_job_t          *job;
} ddjvu_message_any_t; 
struct ddjvu_message_error_s {  /* ddjvu_message_t::m_error */
	ddjvu_message_any_t   any;
	const char           *message;
	const char           *function;
	const char           *filename;
	int                   lineno;
}; 
union ddjvu_message_s {
	struct ddjvu_message_any_s        m_any;
	struct ddjvu_message_error_s      m_error;
	//struct ddjvu_message_info_s       m_info;
	//struct ddjvu_message_newstream_s  m_newstream;
	//struct ddjvu_message_docinfo_s    m_docinfo;
	//struct ddjvu_message_pageinfo_s   m_pageinfo;
	//struct ddjvu_message_chunk_s      m_chunk;
	//struct ddjvu_message_relayout_s   m_relayout;
	//struct ddjvu_message_redisplay_s  m_redisplay;
	//struct ddjvu_message_thumbnail_s  m_thumbnail;
	//struct ddjvu_message_progress_s   m_progress;
};
typedef union  ddjvu_message_s    ddjvu_message_t;
typedef struct ddjvu_pageinfo_s {
	int width;                    /* page width (in pixels) */
	int height;                   /* page height (in pixels) */
	int dpi;                      /* page resolution (in dots per inche) */
	int rotation;                 /* initial page orientation */
	int version;                  /* page version */
} ddjvu_pageinfo_t;
typedef enum {
	DDJVU_RENDER_COLOR = 0,       /* color page or stencil */
	DDJVU_RENDER_BLACK,           /* stencil or color page */
	DDJVU_RENDER_COLORONLY,       /* color page or fail */
	DDJVU_RENDER_MASKONLY,        /* stencil or fail */
	DDJVU_RENDER_BACKGROUND,      /* color background layer */
	DDJVU_RENDER_FOREGROUND,      /* color foreground layer */
} ddjvu_render_mode_t;
typedef enum {
	DDJVU_FORMAT_BGR24,           /* truecolor 24 bits in BGR order */
	DDJVU_FORMAT_RGB24,           /* truecolor 24 bits in RGB order */
	DDJVU_FORMAT_RGBMASK16,       /* truecolor 16 bits with masks */
	DDJVU_FORMAT_RGBMASK32,       /* truecolor 32 bits with masks */
	DDJVU_FORMAT_GREY8,           /* greylevel 8 bits */
	DDJVU_FORMAT_PALETTE8,        /* paletized 8 bits (6x6x6 color cube) */
	DDJVU_FORMAT_MSBTOLSB,        /* packed bits, msb on the left */
	DDJVU_FORMAT_LSBTOMSB,        /* packed bits, lsb on the left */
} ddjvu_format_style_t;
typedef void ddjvu_format_t;
struct ddjvu_rect_s {
	int x, y;
	unsigned int w, h;
};
typedef struct ddjvu_rect_s       ddjvu_rect_t;
// functions
#ifndef DJVU_LIB
typedef ddjvu_context_t*(__cdecl* Fddjvu_context_create)(const char *programname);
typedef ddjvu_document_t*(__cdecl *Fddjvu_document_create_by_filename)(ddjvu_context_t *context, const char *filename, int cache);
typedef ddjvu_status_t(__cdecl *Fddjvu_job_status)(ddjvu_job_t *job);
typedef int(__cdecl *Fddjvu_document_get_pagenum)(ddjvu_document_t *document);
typedef ddjvu_message_t*(__cdecl *Fddjvu_message_wait)(ddjvu_context_t *context);
typedef ddjvu_message_t*(__cdecl *Fddjvu_message_peek)(ddjvu_context_t *context);
typedef void(__cdecl *Fddjvu_message_pop)(ddjvu_context_t *context);
typedef void(__cdecl *Fddjvu_job_release)(ddjvu_job_t *job);
typedef void(__cdecl *Fddjvu_context_release)(ddjvu_context_t *context);
typedef ddjvu_status_t(__cdecl *Fddjvu_document_get_pageinfo_imp)(ddjvu_document_t *document, int pageno, ddjvu_pageinfo_t *info, unsigned int infosz );
typedef ddjvu_job_t*(__cdecl *Fddjvu_document_job)(ddjvu_document_t *document);
typedef ddjvu_page_t*(__cdecl *Fddjvu_page_create_by_pageno)(ddjvu_document_t *document, int pageno);
typedef ddjvu_format_t*(__cdecl *Fddjvu_format_create)(ddjvu_format_style_t style, int nargs, unsigned int *args);
typedef void(__cdecl *Fddjvu_format_set_row_order)(ddjvu_format_t *format, int top_to_bottom);
typedef int(__cdecl *Fddjvu_page_render)(ddjvu_page_t *page, const ddjvu_render_mode_t mode, const ddjvu_rect_t *pagerect, const ddjvu_rect_t *renderrect, const ddjvu_format_t *pixelformat, unsigned long rowsize, char *imagebuffer );
typedef void(__cdecl *Fddjvu_format_release)(ddjvu_format_t *format);
typedef ddjvu_job_t*(__cdecl *Fddjvu_page_job)(ddjvu_page_t *page);
#else
#ifdef __cplusplus
extern "C" { 
#endif
DDJVUAPI ddjvu_context_t* __cdecl  ddjvu_context_create(const char *programname);
DDJVUAPI ddjvu_document_t* __cdecl ddjvu_document_create_by_filename(ddjvu_context_t *context, const char *filename, int cache);
DDJVUAPI ddjvu_status_t __cdecl ddjvu_job_status(ddjvu_job_t *job);
DDJVUAPI int __cdecl ddjvu_document_get_pagenum(ddjvu_document_t *document);
DDJVUAPI ddjvu_message_t* __cdecl ddjvu_message_wait(ddjvu_context_t *context);
DDJVUAPI ddjvu_message_t* __cdecl ddjvu_message_peek(ddjvu_context_t *context);
DDJVUAPI void __cdecl ddjvu_message_pop(ddjvu_context_t *context);
DDJVUAPI void __cdecl ddjvu_job_release(ddjvu_job_t *job);
DDJVUAPI void __cdecl ddjvu_context_release(ddjvu_context_t *context);
DDJVUAPI ddjvu_status_t __cdecl ddjvu_document_get_pageinfo_imp(ddjvu_document_t *document, int pageno, ddjvu_pageinfo_t *info, unsigned int infosz );
DDJVUAPI ddjvu_job_t* __cdecl ddjvu_document_job(ddjvu_document_t *document);
DDJVUAPI ddjvu_page_t* __cdecl ddjvu_page_create_by_pageno(ddjvu_document_t *document, int pageno);
DDJVUAPI ddjvu_format_t* __cdecl ddjvu_format_create(ddjvu_format_style_t style, int nargs, unsigned int *args);
DDJVUAPI void __cdecl ddjvu_format_set_row_order(ddjvu_format_t *format, int top_to_bottom);
DDJVUAPI int __cdecl ddjvu_page_render(ddjvu_page_t *page, const ddjvu_render_mode_t mode, const ddjvu_rect_t *pagerect, const ddjvu_rect_t *renderrect, const ddjvu_format_t *pixelformat, unsigned long rowsize, char *imagebuffer );
DDJVUAPI void __cdecl ddjvu_format_release(ddjvu_format_t *format);
DDJVUAPI ddjvu_job_t* __cdecl ddjvu_page_job(ddjvu_page_t *page);


/*
ftol

push          BP
mov           BP, SP
add           SP, 0F4h
fwait
fnstcw        [DI-002h]
fwait
mov           EAX, [DI-002h]
or            AH, 00Ch
mov           [DI-004h], EAX
fldcw         [DI-004h]
fistp         qword ptr [DI-00Ch]
fldcw         [DI-002h]
mov           AX, [DI-00Ch]
mov           DX, [DI-008h]
leave
ret
*/
#ifdef __cplusplus
};
#endif
#endif
// defines
#define ddjvu_document_decoding_status(document) \
	ddjvu_job_status(ddjvu_document_job(document))
#define ddjvu_document_decoding_done(document) \
	(ddjvu_document_decoding_status(document) >= DDJVU_JOB_OK)
#define ddjvu_document_decoding_error(document) \
	(ddjvu_document_decoding_status(document) >= DDJVU_JOB_FAILED)
#define ddjvu_document_release(document) \
	ddjvu_job_release(ddjvu_document_job(document))
#define ddjvu_document_get_pageinfo(d,p,i) \
	ddjvu_document_get_pageinfo_imp(d,p,i,sizeof(ddjvu_pageinfo_t))
#define ddjvu_page_decoding_status(page) \
	ddjvu_job_status(ddjvu_page_job(page))
#define ddjvu_page_decoding_done(page) \
	(ddjvu_page_decoding_status(page) >= DDJVU_JOB_OK)
#define ddjvu_page_decoding_error(page) \
	(ddjvu_page_decoding_status(page) >= DDJVU_JOB_FAILED)
#define ddjvu_page_release(page) \
	ddjvu_job_release(ddjvu_page_job(page))

// DJVU pages are zero based

struct FileMap
{
	// DJVU
	int nPages, nWidth, nHeight, nBPP, nLoadedPageInfo;
	//
	ddjvu_context_t *ctx;
	ddjvu_document_t *doc;
	ddjvu_format_t *fmt;
	size_t nImageSize;

	//
	BOOL handle(int wait)
	{
		const ddjvu_message_t *msg;
		if (!ctx)
			return FALSE;
		if (wait)
			msg = ddjvu_message_wait(ctx);
		while ((msg = ddjvu_message_peek(ctx)))
		{
			switch(msg->m_any.tag)
			{
			case DDJVU_ERROR:
				//fprintf(stderr,"ddjvu: %s\n", msg->m_error.message);
				//if (msg->m_error.filename)
				//	fprintf(stderr,"ddjvu: '%s:%d'\n", 
				//	msg->m_error.filename, msg->m_error.lineno);
				return FALSE;
			default:
				break;
			}
			ddjvu_message_pop(ctx);
		}
		return TRUE;
	}

	BOOL Open(const char *pName)
	{
		nLoadedPageInfo = -1;

		/* Create context and document */
		if (! (ctx = ddjvu_context_create("PictureView2")))
			//die(i18n("Cannot create djvu context."));
			return FALSE;
		// pName - UTF-8
		if (! (doc = ddjvu_document_create_by_filename(ctx, pName, TRUE)))
			//die(i18n("Cannot open djvu document '%s'."), pName);
			return FALSE;

		while (! ddjvu_document_decoding_done(doc)) {
			if (!handle(TRUE))
				return FALSE;
		}

		nPages = ddjvu_document_get_pagenum(doc);

		return TRUE;
	}
	BOOL LoadPageInfo(int nPage)
	{
		if (nPage == nLoadedPageInfo)
			return TRUE;

		ddjvu_status_t r;
		ddjvu_pageinfo_t info;
		while ((r=ddjvu_document_get_pageinfo(doc,nPage,&info))<DDJVU_JOB_OK) {
			if (!handle(TRUE)) {
				nLoadedPageInfo = -1;
				return FALSE;
			}
		}
		if (r>=DDJVU_JOB_FAILED) {
			nLoadedPageInfo = -1;
			return FALSE;
		}
		nWidth = info.width;
		nHeight = info.height;
		nBPP = 24;
		nLoadedPageInfo = nPage;
		return TRUE;		
	}
	BYTE* LoadPage(int nPage, UINT32* nDecodeWidth, UINT32* nDecodeHeight, INT32* nPitch)
	{
		BYTE *image = NULL;

		if (nPage != nLoadedPageInfo)
			if (!LoadPageInfo(nPage))
				return NULL;

		int width = nWidth, height = nHeight;
		ddjvu_page_t *page = NULL;

		// PageNo is zero based!
		if (! (page = ddjvu_page_create_by_pageno(doc, nPage)))
		{
			//die(i18n("Cannot access page %d."), pageno);
			return NULL;
		}

		while (! ddjvu_page_decoding_done(page)) {
			if (!handle(TRUE)) {
				ddjvu_page_release(page);
				return NULL;
			}
		}
		if (ddjvu_page_decoding_error(page)) {
			//die(i18n("Cannot decode page %d."), pageno);
			ddjvu_page_release(page);
			return NULL;
		}

		ddjvu_render_mode_t mode = DDJVU_RENDER_COLOR;
		ddjvu_format_style_t style = DDJVU_FORMAT_BGR24;
		int compression = 1/*COMPRESSION_NONE*/;
		int white = 0xFF;

		if (!fmt) {
			if (! (fmt = ddjvu_format_create(style, 0, 0))) {
				//die(i18n("Cannot determine pixel style for page %d"), pageno);
				ddjvu_page_release(page);
				return NULL;
			}
			ddjvu_format_set_row_order(fmt, 1);
		}

		if (*nDecodeWidth<=0 || *nDecodeHeight<=0) {
			*nDecodeWidth = width;
			*nDecodeHeight = height;
		}

		// Assumed 24 BPP
		int rowsize = (*nDecodeWidth)*3;
		*nPitch = rowsize;
		int nSize = rowsize*(*nDecodeHeight);

		if (! (image = (BYTE*)HeapAlloc(GetProcessHeap(), 0, nSize))) {
			//die(i18n("Cannot allocate image buffer for page %d"), pageno);
			ddjvu_page_release(page);
			return NULL;
		}

		ddjvu_rect_t rect = {0,0,*nDecodeWidth,*nDecodeHeight};
		//ddjvu_rect_t decoderect = {0,0,width,height};

		if (! ddjvu_page_render(page, mode, &rect, &rect, fmt, rowsize, (char*)image))
			memset(image, white, rowsize * (*nDecodeHeight));

		ddjvu_page_release(page);
		return image;
	}
	void Close(void)
	{
		if (fmt) {
			ddjvu_format_release(fmt);
			fmt = NULL;
		}
		if (doc) {
			ddjvu_document_release(doc);
			doc = NULL;
		}
		if (ctx) {
			ddjvu_context_release(ctx);
			ctx = NULL;
		}
	}
};

/*BOOL WINAPI DllMain( HMODULE hModule, DWORD ul_reason_for_call, LPVOID *lpReserved* )
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
		ghModule = hModule;
	return TRUE;
}*/

void __stdcall pvdGetFormats2(void *pContext, pvdFormats2* pFormats)
{
	_ASSERTE(pFormats->cbSize >= sizeof(pvdFormats2));

	pFormats->pActive = L"DJVU,DJV";
	pFormats->pInactive = pFormats->pForbidden = L"";
}

void __stdcall pvdReloadConfig2(void *pContext)
{
	//PVDSettings set(gpszPluginKey);

	//DWORD nDefault = DEFAULT_INTERPOLATION_MODE;
	//set.GetParam(L"InterpolationMode", L"long;Interpolation quality (0..3)",
	//	REG_DWORD, &nDefault, &gnInterpolationMode, sizeof(gnInterpolationMode));
	//if (gnInterpolationMode > 3) gnInterpolationMode = 3;
}


UINT32 __stdcall pvdInit2(pvdInitPlugin2* pInit)
{
	_ASSERTE(pInit->cbSize >= sizeof(pvdInitPlugin2));
	if (pInit->cbSize < sizeof(pvdInitPlugin2)) {
		pInit->nErrNumber = PDJE_OLD_PICVIEW;
		return 0;
	}

	memset(&ip,0,sizeof(ip));
	memmove(&ip, pInit, min(sizeof(ip),pInit->cbSize));
	ghModule = ip.hModule;

	gpszPluginKey = pInit->pRegKey;
	pvdReloadConfig2(NULL);

	return PVD_UNICODE_INTERFACE_VERSION;
}

void __stdcall pvdExit2(void * /*pContext*/)
{
}

void __stdcall pvdPluginInfo2(pvdInfoPlugin2 *pPluginInfo)
{
	_ASSERTE(pPluginInfo->cbSize >= sizeof(pvdInfoPlugin2));
	pPluginInfo->Priority = 0x1000;
	pPluginInfo->pName = L"DJVU";
	pPluginInfo->pVersion = GetVersion(pPluginInfo->hModule);
	pPluginInfo->pComments = L"Copyright © 2009 Maximus5";
	pPluginInfo->Flags = PVD_IP_DECODE|PVD_IP_CANDESCALE|PVD_IP_CANUPSCALE;
}

BOOL __stdcall pvdFileDetect2(void * /*pContext*/, const wchar_t *pFileName, INT64 lFileSize, const BYTE *pBuf, UINT32 lBuf, pvdInfoImage2 *pImageInfo)
{
	if (lFileSize && lBuf >= 0x4 && *(u32*)pBuf == 'T&TA')
	{
		pImageInfo->Flags = PVD_IIF_FILE_REQUIRED;
		pImageInfo->pFormatName = L"DJVU";
		return TRUE;
	}
	return FALSE;
}

BOOL __stdcall pvdFileOpen2(void * /*pContext*/, const wchar_t *pFileName, INT64 lFileSize, const BYTE *pBuf, UINT32 lBuf, pvdInfoImage2 *pImageInfo)
{
	if (!lFileSize)
		pImageInfo->nErrNumber = PDJE_FILE_REQUIRED;
	else if (lBuf < 0x4 || *(u32*)pBuf != 'T&TA')
		pImageInfo->nErrNumber = PDJE_NOT_DJVU_HEADER;
	else
	{
		BOOL lbOpenRc = FALSE;
		FileMap *pFile = (FileMap*) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(FileMap));

		if (!wcsncmp(pFileName, L"\\\\?\\", 4))
			pFileName += 4;
		size_t nLen = lstrlenW(pFileName)+1;
		char* pszUtf8 = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 4*nLen+1);
		if (pFileName[0] == L'U' && pFileName[1] == L'N' && pFileName[2] == L'C' && pFileName[3] == L'\\')
		{
			*pszUtf8 = '\\';
			WideCharToMultiByte(CP_UTF8, 0, pFileName+3, -1, pszUtf8+1, (UINT)4*nLen, 0, 0);
		} else {
			WideCharToMultiByte(CP_UTF8, 0, pFileName, -1, pszUtf8, (UINT)4*nLen, 0, 0);
		}

		lbOpenRc = pFile->Open(pszUtf8);

		HeapFree(GetProcessHeap(), 0, pszUtf8);
		if (!lbOpenRc)
		{
			pFile->Close();
			HeapFree(GetProcessHeap(), 0, pFile);
			return FALSE;
		}

		pImageInfo->nPages = pFile->nPages;
		pImageInfo->Flags = 0;
		pImageInfo->pFormatName = L"DJVU";
		pImageInfo->pCompression = NULL;
		pImageInfo->pComments = NULL;
		pImageInfo->pImageContext = pFile;
		return TRUE;
	}
	return FALSE;
}

BOOL __stdcall pvdPageInfo2(void * /*pContext*/, void *pImageContext, pvdInfoPage2 *pPageInfo)
{
	if (!((FileMap*)pImageContext)->LoadPageInfo(pPageInfo->iPage))
		return FALSE;
	pPageInfo->lWidth = ((FileMap*)pImageContext)->nWidth;
	pPageInfo->lHeight = ((FileMap*)pImageContext)->nHeight;
	pPageInfo->nBPP = ((FileMap*)pImageContext)->nBPP;
	return TRUE;
}

BOOL __stdcall pvdPageDecode2(void * /*pContext*/, void *pImageContext, pvdInfoDecode2 *pDecodeInfo, 
							  pvdDecodeCallback2 DecodeCallback, void *pDecodeCallbackContext)
{
	pDecodeInfo->pImage = ((FileMap*)pImageContext)->LoadPage(pDecodeInfo->iPage, 
		&pDecodeInfo->lWidth, &pDecodeInfo->lHeight, &pDecodeInfo->lImagePitch);
	pDecodeInfo->pPalette = NULL;
	pDecodeInfo->Flags = 0;
	pDecodeInfo->nBPP = ((FileMap*)pImageContext)->nBPP;
	pDecodeInfo->ColorModel = PVD_CM_BGR;
	pDecodeInfo->nColorsUsed = 0;
	return TRUE;
}

void __stdcall pvdPageFree2(void * /*pContext*/, void *pImageContext, pvdInfoDecode2 *pDecodeInfo)
{
	if (pDecodeInfo->pImage) {
		HeapFree(GetProcessHeap(), 0, pDecodeInfo->pImage);
		pDecodeInfo->pImage = NULL;
	}
}

void __stdcall pvdFileClose2(void * /*pContext*/, void *pImageContext)
{
	((FileMap*)pImageContext)->Close();
	HeapFree(GetProcessHeap(), 0, pImageContext);
}
