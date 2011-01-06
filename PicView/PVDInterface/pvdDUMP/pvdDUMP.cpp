#include <windows.h>
#ifndef WIN64
#include <crtdbg.h>
#else
#define _ASSERTE(a)
#endif
#include "../PictureViewPlugin.h"

typedef __int32 i32;
typedef __int64 i64;
typedef unsigned __int8 u8;
typedef unsigned __int16 u16;
typedef DWORD u32;


// Некритичные ошибки ( < 0x7FFFFFFF )
// При подборе декодера в PicView нужно возвращать именно их ( < 0x7FFFFFFF )
// для тех файлов (форматов), которые неизвестны данному декодеру.
// PicView не будет отображать эти ошибки пользователю.
#define PDE_NOT_DUMP_HEADER           0x1001
#define PDE_TOO_SMALL_FILE           0x1002

// Далее идут критичные ошибки. Информация об ошибке будет показана
// пользователю, если PicView не сможет открыть файл никаким декодером
#define PDE_OPEN_FILE_FAILED         0x80000001
#define PDE_FILE_MAPPING_FAILED      0x80000002
#define PDE_INVALID_CONTEXT          0x80000003
#define PDE_TOO_LARGE_FILE           0x80000004
#define PDE_OLD_PICVIEW              0x80000005

BOOL __stdcall pvdTranslateError2(DWORD nErrNumber, wchar_t *pszErrInfo, int nBufLen)
{
	if (!pszErrInfo || nBufLen<=0)
		return FALSE;

	switch (nErrNumber)
	{
	case PDE_OPEN_FILE_FAILED:
		lstrcpynW(pszErrInfo, L"Open file failed", nBufLen); break;
	case PDE_FILE_MAPPING_FAILED:
		lstrcpynW(pszErrInfo, L"File mapping failed (not enough memory)", nBufLen); break;
	case PDE_INVALID_CONTEXT:
		lstrcpynW(pszErrInfo, L"Invalid context", nBufLen); break;
	case PDE_NOT_DUMP_HEADER:
		lstrcpynW(pszErrInfo, L"Not PicViewDump header", nBufLen); break;
	case PDE_TOO_LARGE_FILE:
		lstrcpynW(pszErrInfo, L"File is too large", nBufLen); break;
	case PDE_TOO_SMALL_FILE:
		lstrcpynW(pszErrInfo, L"File is too smal", nBufLen); break;
	case PDE_OLD_PICVIEW:	
		lstrcpyn(pszErrInfo, L"Old PicView version, exiting", nBufLen); break;
	default:
		return FALSE;
	}
	return TRUE;
}

typedef struct tag_DumpData
{
	DWORD nWidth, nHeight;
	int nPitch;
	DWORD nBPP;
	BYTE Data[1];
} DumpData;

bool CheckDumpHeader(const BYTE *pBuf, UINT32 lBuf)
{
	LPDWORD pdw = (LPDWORD)pBuf;
	bool bOk = lBuf >= 0x20 && (pdw[0]>=10 && pdw[0]<=4000) && (pdw[1]>=10 && pdw[1]<=4000)
		&& (pdw[3]>=1 && pdw[3]<=32) && pdw[2];
	if (bOk) {
		int nPitch = ((int*)pBuf)[2];
		if (nPitch<0) nPitch = -nPitch;
		int nCalcPitch = (pdw[0] * pdw[3] / 8);
		int nDelta = (nPitch - nCalcPitch);
		if (nDelta<0) nDelta = -nDelta;
		if (nDelta>100) bOk = false;
	}
	return bOk;
}


void __stdcall pvdPluginInfo2(pvdInfoPlugin2 *pPluginInfo)
{
	_ASSERTE(pPluginInfo->cbSize >= sizeof(pvdInfoPlugin2));
	pPluginInfo->Priority = 1;
	pPluginInfo->pName = L"Dumps";
	pPluginInfo->pVersion = L"2.0";
	pPluginInfo->pComments = L"PictureView debug plugin";
	pPluginInfo->Flags = PVD_IP_DECODE;
}


UINT32 __stdcall pvdInit2(pvdInitPlugin2* pInit)
{
	_ASSERTE(pInit->cbSize >= sizeof(pvdInitPlugin2));
	if (pInit->cbSize < sizeof(pvdInitPlugin2)) {
		pInit->nErrNumber = PDE_OLD_PICVIEW;
		return 0;
	}

	return PVD_UNICODE_INTERFACE_VERSION;
}

void __stdcall pvdGetFormats2(void *pContext, pvdFormats2* pFormats)
{
	_ASSERTE(pFormats->cbSize >= sizeof(pvdFormats2));

	// pContext в данном случае не используется, т.к. все тривиально
	// Он может пригодиться другим декодерам, если им требуется сканирование
	// своих библиотек для определения поддерживаемых форматов

	pFormats->pActive = L"dump0,dump1,dump2";
	pFormats->pInactive = pFormats->pForbidden = L"";
}

BOOL __stdcall pvdFileOpen2(void *pContext, const wchar_t *pFileName, INT64 lFileSize, const BYTE *pBuf, UINT32 lBuf, pvdInfoImage2 *pImageInfo)
{
	DumpData *pSrc = (DumpData*)pBuf;

	_ASSERTE(pImageInfo->cbSize >= sizeof(pvdInfoImage2));
	//BUGBUG: вообще-то PicView 1.41 не всегда передает данные (или их часть?) через pBuf. Он может быть и пустым.
	// PicView 2 всегда передает через pBuf хотя бы часть файла


	if (!CheckDumpHeader(pBuf, lBuf)) {
		pImageInfo->nErrNumber = PDE_NOT_DUMP_HEADER;
		return FALSE;
	}

	DWORD nSize = (pSrc->nPitch>0 ? pSrc->nPitch : -pSrc->nPitch)*pSrc->nHeight + sizeof(DumpData);
	if (nSize < lBuf) nSize = lBuf;
	DumpData *pCopy = (DumpData*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, nSize);
	memmove(pCopy, pBuf, lBuf);

	pImageInfo->nPages = (pSrc->nBPP==32) ? 2 : 1;
	pImageInfo->Flags = 0;
	pImageInfo->pFormatName = L"Dump";
	pImageInfo->pCompression = NULL;
	pImageInfo->pComments = NULL;
	pImageInfo->pImageContext = pCopy; // Контекст файла в этом примере не используется
	return TRUE;
}

BOOL __stdcall pvdPageInfo2(void *pContext, void *pImageContext, pvdInfoPage2 *pPageInfo)
{
	_ASSERTE(pPageInfo->cbSize >= sizeof(pvdInfoPage2));

	if (pPageInfo->iPage==0 || pPageInfo->iPage==1)
	{
		DumpData *pSrc = (DumpData*)pImageContext;
		pPageInfo->lWidth = pSrc->nWidth;
		pPageInfo->lHeight = pSrc->nHeight;
		pPageInfo->nBPP = pSrc->nBPP;
		return TRUE;
	}
	return FALSE;
}

BOOL __stdcall pvdPageDecode2(void *pContext, void *pImageContext, pvdInfoDecode2 *pDecodeInfo, pvdDecodeCallback2 DecodeCallback, void *pDecodeCallbackContext)
{
	_ASSERTE(pDecodeInfo->cbSize >= sizeof(pvdInfoDecode2));

	if (pDecodeInfo->iPage==0 || pDecodeInfo->iPage==1)
	{
		DumpData *pSrc = (DumpData*)pImageContext;
		pDecodeInfo->lWidth = pSrc->nWidth;
		pDecodeInfo->lHeight = pSrc->nHeight;
		pDecodeInfo->nBPP = pSrc->nBPP;
		pDecodeInfo->pImage = pSrc->Data;
		pDecodeInfo->lImagePitch = pSrc->nPitch;
		pDecodeInfo->ColorModel = (pDecodeInfo->iPage==0 || pSrc->nBPP!=32) ? PVD_CM_BGR : PVD_CM_BGRA;
		pDecodeInfo->Flags = (pDecodeInfo->iPage==1 && pSrc->nBPP==32) ? PVD_IDF_ALPHA : 0;
		//if (pSrc->nBPP <= 8) {
		//	// Нужна палитра...
		//}
		return TRUE;
	}
	return FALSE;
}

void __stdcall pvdPageFree2(void *pContext, void *pImageContext, pvdInfoDecode2 *pDecodeInfo)
{
	_ASSERTE(pDecodeInfo->cbSize >= sizeof(pvdInfoDecode2));
}

void __stdcall pvdFileClose2(void *pContext, void *pImageContext)
{
	HeapFree(GetProcessHeap(), 0, pImageContext);
}

void __stdcall pvdExit2(void *pContext)
{
}
