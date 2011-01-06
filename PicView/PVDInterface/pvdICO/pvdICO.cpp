#include <windows.h>
#include <shellapi.h>
//#include <vector>
#include <GdiPlus.h>
#include "../PictureViewPlugin.h"
#include "../PVD2Helper.h"
#include "../MStream.h"

// Некритичные ошибки ( < 0x7FFFFFFF )
// При подборе декодера в PicView нужно возвращать именно их ( < 0x7FFFFFFF )
// для тех файлов (форматов), которые неизвестны данному декодеру.
// PicView не будет отображать эти ошибки пользователю.
#define PIE_NOT_ICO_FILE             0x1001

// Далее идут критичные ошибки. Информация об ошибке будет показана
// пользователю, если PicView не сможет открыть файл никаким декодером
#define PIE_NO_IMAGES                0x80000001
#define PIE_BUFFER_EMPTY             0x80000002
#define PIE_WIN32_ERROR              0x80000003
#define PIE_TOO_LARGE_FILE           0x80000004
#define PIE_NOT_ENOUGH_MEMORY        0x80000005
#define PIE_INVALID_CONTEXT          0x80000006
#define PIE_INVALID_NUMBER           0x80000007
#define PIE_JUMP_OUT_OF_FILE         0x80000008
#define PIE_INVALID_BPP              0x80000009
#define PIE_UNSUPPORTED_BPP          0x8000000A
#define PIE_OLD_PICVIEW              0x8000000B
#define PIE_BUFFER_NOT_FULL          0x8000000C
DWORD gnLastWin32Error = 0;
pvdInitPlugin2 ip = {0};

#define PVD_MAX_ICON_SIZE 0x100000 /*1Mb*/

#define STRING2(x) #x
#define STRING(x) STRING2(x)
#define FILE_LINE __FILE__ "(" STRING(__LINE__) "): "
#ifdef HIDE_TODO
#define TODO(s) 
#define WARNING(s) 
#else
#define TODO(s) __pragma(message (FILE_LINE "TODO: " s))
#define WARNING(s) __pragma(message (FILE_LINE "warning: " s))
#endif
#define PRAGMA_ERROR(s) __pragma(message (FILE_LINE "error: " s))


BOOL __stdcall pvdTranslateError2(DWORD nErrNumber, wchar_t *pszErrInfo, int nBufLen)
{
	if (!pszErrInfo || nBufLen<=0)
		return FALSE;

	switch (nErrNumber)
	{
	case PIE_NOT_ICO_FILE:
		lstrcpynW(pszErrInfo, L"Unrecognized format (not ICO file)", nBufLen); break;
	case PIE_BUFFER_EMPTY:
		lstrcpynW(pszErrInfo, L"pvdFileOpen2 failed, because empty buffer", nBufLen); break;
	case PIE_NO_IMAGES:
		lstrcpynW(pszErrInfo, L"File does not contains icons", nBufLen); break;
	case PIE_WIN32_ERROR:
		FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS, 0, 
			gnLastWin32Error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			pszErrInfo, nBufLen, NULL);
		break;
	case PIE_OLD_PICVIEW:
		lstrcpyn(pszErrInfo, L"Old PicView version, exiting", nBufLen); break;
	case PIE_BUFFER_NOT_FULL:
		lstrcpyn(pszErrInfo, L"pBuf does not contains whole file", nBufLen); break;
	default:
		return FALSE;
	}
	return TRUE;
}

void __stdcall pvdPluginInfo2(pvdInfoPlugin2 *pPluginInfo)
{
	_ASSERTE(pPluginInfo->cbSize >= sizeof(pvdInfoPlugin2));
	pPluginInfo->Priority = 0x1000;
	pPluginInfo->pName = L"ICO";
	pPluginInfo->pVersion = GetVersion(pPluginInfo->hModule);
	pPluginInfo->pComments = L"Copyright © 2009 Maximus5";
	pPluginInfo->Flags = PVD_IP_DECODE;
}

bool gbCoInitialized = false, gbTokenInitialized = false;
ULONG_PTR gdiplusToken = 0;

UINT32 __stdcall pvdInit2(pvdInitPlugin2* pInit)
{
	_ASSERTE(pInit->cbSize >= sizeof(pvdInitPlugin2));
	if (pInit->cbSize < sizeof(pvdInitPlugin2)) {
		pInit->nErrNumber = PIE_OLD_PICVIEW;
		return 0;
	}

	memset(&ip,0,sizeof(ip));
	memmove(&ip, pInit, min(sizeof(ip),pInit->cbSize));
	ghModule = ip.hModule;
	pInit->pContext = NULL;
	pInit->nErrNumber = 0;

	HRESULT hrCoInitialized = CoInitialize(NULL);
	gbCoInitialized = SUCCEEDED(hrCoInitialized);

	#ifndef _NO_EXEPTION_
	try {
	#endif
		Gdiplus::GdiplusStartupInput gdiplusStartupInput;
		Gdiplus::Status lRc = GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
		if (lRc != Gdiplus::Ok) {
			Gdiplus::GdiplusShutdown(gdiplusToken); gbTokenInitialized = false;
			if (pInit->MessageLog)
				pInit->MessageLog(NULL, L"ICO: GDI+ initialization failed", 1);
		} else {
			gbTokenInitialized = true;
		}
	#ifndef _NO_EXEPTION_
	} catch(...) {
		gbTokenInitialized = false;
	}
	#endif

	return PVD_UNICODE_INTERFACE_VERSION;
}

void __stdcall pvdGetFormats2(void *pContext, pvdFormats2* pFormats)
{
	_ASSERTE(pFormats->cbSize >= sizeof(pvdFormats2));

	// pContext в данном случае не используется, т.к. все тривиально
	// Он может пригодиться другим декодерам, если им требуется сканирование
	// своих библиотек для определения поддерживаемых форматов

	pFormats->pActive = L"cur,ico";
	pFormats->pInactive = L"."; // Файлы без расширений - приходят из ресурсов PE файлов
	pFormats->pForbidden = L"";
}

#pragma pack( push )
#pragma pack( 1 )

typedef struct
{
    BYTE        bWidth;          // Width, in pixels, of the image
    BYTE        bHeight;         // Height, in pixels, of the image
    BYTE        bColorCount;     // Number of colors in image (0 if >=8bpp)
    BYTE        bReserved;       // Reserved ( must be 0)
    WORD        wPlanes;         // Color Planes
    WORD        wBitCount;       // Bits per pixel
    DWORD       dwBytesInRes;    // How many bytes in this resource?
    DWORD       dwImageOffset;   // Where in the file is this image?
} ICONDIRENTRY, *LPICONDIRENTRY;

typedef struct
{
    WORD           idReserved;   // Reserved (must be 0)
    WORD           idType;       // Resource Type (1 for icons, 2 for cursors)
    WORD           idCount;      // How many images?
    ICONDIRENTRY   idEntries[1]; // An entry for each image (idCount of 'em)
} ICONDIR, *LPICONDIR;

typedef struct
{
	DWORD          nDataSize;
	ICONDIR        Icon;
} ICONCONTEXT, *LPICONCONTEXT;

/*typedef struct 
{
	MStream *pStrm;
	Gdiplus::Bitmap *pBmp;
} GDIPLUSICON, *LPGDIPLUSICON;*/

//typedef struct
//{
//   BITMAPINFOHEADER   icHeader;      // DIB header
//   RGBQUAD         icColors[1];   // Color table
//   BYTE            icXOR[1];      // DIB bits for XOR mask
//   BYTE            icAND[1];      // DIB bits for AND mask
//} ICONIMAGE, *LPICONIMAGE;

// DLL and EXE Files
// Icons can also be stored in .DLL and .EXE files. The structures used to store icon images in .EXE and .DLL files differ only slightly from those used in .ICO files.
// Analogous to the ICONDIR data in the ICO file is the RT_GROUP_ICON resource. In fact, one RT_GROUP_ICON resource is created for each ICO file bound to the EXE or DLL with the resource compiler/linker. The RT_GROUP_ICON resource is simply a GRPICONDIR structure:
// #pragmas are used here to insure that the structure's
// packing in memory matches the packing of the EXE or DLL.
typedef struct
{
   BYTE   bWidth;               // Width, in pixels, of the image
   BYTE   bHeight;              // Height, in pixels, of the image
   BYTE   bColorCount;          // Number of colors in image (0 if >=8bpp)
   BYTE   bReserved;            // Reserved
   WORD   wPlanes;              // Color Planes
   WORD   wBitCount;            // Bits per pixel
   DWORD   dwBytesInRes;         // how many bytes in this resource?
   WORD   nID;                  // the ID
} GRPICONDIRENTRY, *LPGRPICONDIRENTRY;

typedef struct 
{
   WORD            idReserved;   // Reserved (must be 0)
   WORD            idType;       // Resource type (1 for icons, 2 for cursors)
   WORD            idCount;      // How many images?
   GRPICONDIRENTRY   idEntries[1]; // The entries for each image
} GRPICONDIR, *LPGRPICONDIR;
#pragma pack( pop )

/*

// Load the DLL/EXE without executing its code
hLib = LoadLibraryEx( szFileName, NULL, LOAD_LIBRARY_AS_DATAFILE );
// Find the group resource which lists its images
hRsrc = FindResource( hLib, MAKEINTRESOURCE( nId ), RT_GROUP_ICON );
// Load and Lock to get a pointer to a GRPICONDIR
hGlobal = LoadResource( hLib, hRsrc );
lpGrpIconDir = LockResource( hGlobal );
// Using an ID from the group, Find, Load and Lock the RT_ICON
hRsrc = FindResource( hLib, MAKEINTRESOURCE( lpGrpIconDir->idEntries[0].nID ),
                      RT_ICON );
hGlobal = LoadResource( hLib, hRsrc );
lpIconImage = LockResource( hGlobal );
// Here, lpIconImage points to an ICONIMAGE structure

*/

BOOL __stdcall pvdFileOpen2(void *pContext, const wchar_t *pFileName, INT64 lFileSize, const BYTE *pBuf, UINT32 lBuf, pvdInfoImage2 *pImageInfo)
{
	_ASSERTE(pImageInfo->cbSize >= sizeof(pvdInfoImage2));
	_ASSERTE(pBuf && lBuf);

	// При открытии ресурсов - pBuf это BITMAPINFOHEADER
	// Соответственно, при просмотре ресурса - один файл это строго одна иконка,
	// информацию нужно грузить строго из BITMAPINFOHEADER
	// Тут может быть и PNG, но это нас уже не интересует. PNG может быть открыт любым декодером...

	LPICONCONTEXT pIcon = NULL;
	LPICONDIR pTest = (LPICONDIR)pBuf;
	LPBITMAPINFOHEADER pBmp = (LPBITMAPINFOHEADER)pBuf;

	if (!pBuf || !lBuf || (lFileSize && lFileSize != (__int64)lBuf)) {
		pImageInfo->nErrNumber = PIE_BUFFER_NOT_FULL;
		return FALSE;
	}

	if (lBuf > sizeof(ICONDIR) && pTest->idReserved==0 && (pTest->idType==1 /* ICON */ || pTest->idType == 2 /* CURSOR */)) {
		if (lFileSize > PVD_MAX_ICON_SIZE) {
			pImageInfo->nErrNumber = PIE_TOO_LARGE_FILE;
			return FALSE;
		}
		if (pTest->idCount == 0) {
			pImageInfo->nErrNumber = PIE_NO_IMAGES;
			return FALSE;
		}

		// Делаем копию буфера
		pIcon = (LPICONCONTEXT)malloc(lBuf+sizeof(DWORD));
		if (!pIcon) {
			pImageInfo->nErrNumber = PIE_NOT_ENOUGH_MEMORY;
			return FALSE;
		}
		pIcon->nDataSize = lBuf;
		memmove(&(pIcon->Icon), pBuf, lBuf);

	} else {
		// Test BITMAPINFOHEADER
		if (lBuf > sizeof(BITMAPINFOHEADER) && pBmp->biSize == sizeof(BITMAPINFOHEADER)
			&& (pBmp->biWidth && pBmp->biWidth < 256)
			&& (pBmp->biHeight == (pBmp->biWidth * 2)))
		{
			// Делаем копию буфера, но предваряем его заголовком иконки
			DWORD nSize = lBuf + sizeof(ICONDIR);
			pIcon = (LPICONCONTEXT)malloc(nSize+sizeof(DWORD));
			if (!pIcon) {
				pImageInfo->nErrNumber = PIE_NOT_ENOUGH_MEMORY;
				return FALSE;
			}
			pIcon->nDataSize = nSize;
			pIcon->Icon.idReserved = 0;
			pIcon->Icon.idType = 1;
			pIcon->Icon.idCount = 1;
			pIcon->Icon.idEntries[0].bWidth = (BYTE)pBmp->biWidth;
			pIcon->Icon.idEntries[0].bHeight = (BYTE)pBmp->biWidth;
			pIcon->Icon.idEntries[0].bColorCount = (pBmp->biBitCount >= 8) ? 0 : (1 << pBmp->biBitCount);
			pIcon->Icon.idEntries[0].bReserved = 0;
			pIcon->Icon.idEntries[0].wPlanes = pBmp->biPlanes;
			pIcon->Icon.idEntries[0].wBitCount = pBmp->biBitCount;
			pIcon->Icon.idEntries[0].dwBytesInRes = lBuf;
			pIcon->Icon.idEntries[0].dwImageOffset = sizeof(ICONDIR);
			memmove(&(pIcon->Icon.idEntries[1]), pBuf, lBuf);

		} else {
			pImageInfo->nErrNumber = PIE_NOT_ICO_FILE;
			return FALSE;
		}
	}


	WARNING("Хорошо бы возвращать умолчательный индекс отображаемой иконки, если первая НЕ содержит изображения вообще");
	// file_view_hc.ico - первый фрейм вообще пустой (полностью прозрачный), а второй содержит картинку


	if (pIcon->nDataSize < (sizeof(ICONDIR) + (pIcon->Icon.idCount-1)*sizeof(ICONDIRENTRY))) {
		pImageInfo->nErrNumber = PIE_JUMP_OUT_OF_FILE;
		free(pIcon);
		return FALSE;
	}

	pImageInfo->pImageContext = pIcon;

	pImageInfo->nPages = pIcon->Icon.idCount;
	pImageInfo->Flags = 0;
	pImageInfo->pFormatName = pIcon->Icon.idType!=2 ? L"ICO" : L"CUR";
	pImageInfo->pCompression = NULL;
	pImageInfo->pComments = NULL;
	pImageInfo->pImageContext = pIcon;
	return TRUE;
}

DWORD LoadPageInfo(LPICONCONTEXT pIcon, int iPage, UINT32& nBPP, UINT32& nWidth, UINT32& nHeight, UINT& nColors, const wchar_t** ppFormat)
{
	if (iPage >= (int)pIcon->Icon.idCount || iPage < 0)
		return PIE_INVALID_NUMBER;


	nWidth = pIcon->Icon.idEntries[iPage].bWidth;
		if (nWidth == 0) nWidth = 256;
	nHeight = pIcon->Icon.idEntries[iPage].bHeight;
		if (nHeight == 0) nHeight = 256;

	nBPP = 0; nColors = pIcon->Icon.idEntries[iPage].bColorCount;
	switch (pIcon->Icon.idEntries[iPage].bColorCount)
	{
	case 1: _ASSERTE(pIcon->Icon.idEntries[iPage].bColorCount!=1); break;
	case 2: nBPP = 1; break;
	case 4: nBPP = 2; break;
	case 8: nBPP = 3; break;
	case 16: nBPP = 4; break;
	case 32: nBPP = 5; break;
	case 64: nBPP = 6; break;
	case 128: nBPP = 7; break;
	default:
		_ASSERTE(pIcon->Icon.idEntries[iPage].bColorCount == 0);
		nBPP = pIcon->Icon.idEntries[iPage].wBitCount * pIcon->Icon.idEntries[iPage].wPlanes;
		//if (nBPP == 0)			nBPP = 8;
		//nColors = 1 << nBPP;
	}
	LPICONDIRENTRY pImage = pIcon->Icon.idEntries + iPage;
	LPBYTE pDataStart = ((LPBYTE)&(pIcon->Icon));
	LPBYTE pImageStart = (pDataStart + pImage->dwImageOffset);
	if (nBPP <= 256) {
		if (*((DWORD*)pImageStart) == 0x474e5089) {
			//PNG Mark
			//_ASSERTE(nColors == 0);
			nBPP = 32; nColors = 0;
			if (ppFormat) *ppFormat = L"ICO.PNG";
		} else if (*((DWORD*)pImageStart) == sizeof(BITMAPINFOHEADER)) {
			BITMAPINFOHEADER *pDIB = (BITMAPINFOHEADER*)pImageStart;
			//if (nBPP == 0) {
			if (pDIB->biPlanes && pDIB->biBitCount) {
				nBPP = pDIB->biPlanes * pDIB->biBitCount;
			}
			_ASSERTE(nBPP == (pDIB->biPlanes * pDIB->biBitCount));
			if (nBPP <= 8)
				nColors = 1 << nBPP;
			else
				nColors = 0;
			if (ppFormat) *ppFormat = L"ICO.DIB";
		} else {
			_ASSERTE(*((DWORD*)pImageStart) == sizeof(BITMAPINFOHEADER));
		}
	}
	_ASSERTE(nBPP);
	if (!nBPP)
		return PIE_INVALID_BPP;

	return NO_ERROR;
}

BOOL __stdcall pvdPageInfo2(void *pContext, void *pImageContext, pvdInfoPage2 *pPageInfo)
{
	_ASSERTE(pPageInfo->cbSize >= sizeof(pvdInfoPage2));
	_ASSERTE(pImageContext);
	if (!pImageContext) {
		pPageInfo->nErrNumber = PIE_INVALID_CONTEXT;
		return FALSE;
	}

	LPICONCONTEXT pIcon = (LPICONCONTEXT)pImageContext;

	UINT nColors = 0;
	if ((pPageInfo->nErrNumber = LoadPageInfo(pIcon, pPageInfo->iPage, pPageInfo->nBPP, 
		pPageInfo->lWidth, pPageInfo->lHeight, nColors, &pPageInfo->pFormatName)) != NO_ERROR)
		return FALSE;

	return TRUE;
}

LPBYTE Decode1BPP(UINT nWidth, UINT nHeight, RGBQUAD* pPAL, LPBYTE pXOR, LPBYTE pAND, INT32& lDstStride)
{
	UINT lXorStride = (((nWidth>>3)+3)>>2)<<2; // ((nWidth + 7) >> 3);
	UINT lAndStride = lXorStride; //((nWidth + 3) >> 2) << 1;
	lDstStride = ((nWidth + 3) >> 2) << 4;
	LPBYTE pData = (LPBYTE)calloc(lDstStride,nHeight); // Делаем 32битное изображение

	//UINT nPoints = nWidth * nHeight;
	//UINT nStride = nWidth << 2; // Это приемник - 32бит на точку

	//LPBYTE pXorSrc, pAndSrc, pDst = pData + nStride * (nHeight - 1);

	////for (UINT i = nPoints; i--;) {
	//UINT i = nPoints - 1, n;
	//for (UINT Y = 0; Y < nHeight; Y++) {
	//	pDst = pData + nStride * Y;
	//	for (UINT X = nWidth; X--; i--) {
	//		n = pAND[i>>1];
	//		((UINT*)pDst)[X] = ((UINT*)pPAL)[ (i & 1) ? (n & 0x0F) : (n >> 4) ];
	//	}
	//}	

	LPBYTE pXorSrc = pXOR; LPBYTE pAndSrc = pAND; LPBYTE pDst = pData + (nHeight - 1) * lDstStride;
	for (UINT j = nHeight; j--; pXorSrc += lXorStride, pAndSrc += lAndStride, pDst -= lDstStride)
	{
		_ASSERTE(pDst >= pData);
		for (UINT i = nWidth; i--;) {
			if ((pXorSrc[i/8] >> (7 - ((i) & 7)) & 1) == 0)
			{
				if ((pAndSrc[i/8] >> (7 - ((i) & 7)) & 1) != 0)
					((UINT*)pDst)[i] = 0xFFFFFFFF;
				else
					((UINT*)pDst)[i] = 0xFF000000;
			}
			//#ifdef _DEBUG
			//else {
			//	((UINT*)pDst)[i] = 0xFFFF0000;
			//}
			//#endif
		}
	}

	return pData;
}

LPBYTE Decode4BPP(UINT nWidth, UINT nHeight, RGBQUAD* pPAL, LPBYTE pXOR, LPBYTE pAND, INT32& lDstStride)
{
	UINT lXorStride = (((nWidth>>3)+3)>>2)<<2; // ((nWidth + 7) >> 3);
	UINT lAndStride = (((nWidth>>1)+3)>>2)<<2; //((nWidth + 3) >> 2) << 1;
	lDstStride = ((nWidth + 3) >> 2) << 4;
	LPBYTE pData = (LPBYTE)calloc(lDstStride,nHeight); // Делаем 32битное изображение

	//UINT nPoints = nWidth * nHeight;
	//UINT nStride = nWidth << 2; // Это приемник - 32бит на точку

	//LPBYTE pXorSrc, pAndSrc, pDst = pData + nStride * (nHeight - 1);

	////for (UINT i = nPoints; i--;) {
	//UINT i = nPoints - 1, n;
	//for (UINT Y = 0; Y < nHeight; Y++) {
	//	pDst = pData + nStride * Y;
	//	for (UINT X = nWidth; X--; i--) {
	//		n = pAND[i>>1];
	//		((UINT*)pDst)[X] = ((UINT*)pPAL)[ (i & 1) ? (n & 0x0F) : (n >> 4) ];
	//	}
	//}	

	LPBYTE pXorSrc = pXOR; LPBYTE pAndSrc = pAND; LPBYTE pDst = pData + (nHeight - 1) * lDstStride;
	UINT n;
	for (UINT j = nHeight; j--; pXorSrc += lXorStride, pAndSrc += lAndStride, pDst -= lDstStride)
	{
		_ASSERTE(pDst >= pData);
		for (UINT i = nWidth; i--;) {
			if ((pXorSrc[i/8] >> (7 - ((i) & 7)) & 1) == 0)
			{
				n = pAndSrc[i>>1];
				((UINT*)pDst)[i] = 0xFF000000 | ((UINT*)pPAL)[ (i & 1) ? (n & 0x0F) : (n >> 4) ];
			}
			//#ifdef _DEBUG
			//else {
			//	((UINT*)pDst)[i] = 0xFFFF0000;
			//}
			//#endif
		}
	}

	return pData;
}

LPBYTE Decode8BPP(UINT& nWidth, UINT nHeight, RGBQUAD* pPAL, LPBYTE pXOR, LPBYTE pAND, INT32& lDstStride)
{
	UINT lXorStride = ((nWidth+31)>>5)<<2;
	UINT lAndStride = (((nWidth<<3)+31)>>5)<<2;
	lDstStride = (((nWidth<<5)+31)>>5)<<2;
	LPBYTE pData = (LPBYTE)calloc(lDstStride,nHeight); // Делаем 32битное изображение

	LPBYTE pXorSrc = pXOR; LPBYTE pAndSrc = pAND; LPBYTE pDst = pData + (nHeight - 1) * lDstStride;
	for (UINT j = nHeight; j--; pXorSrc += lXorStride, pAndSrc += lAndStride, pDst -= lDstStride)
	{
		_ASSERTE(pDst >= pData);
		for (UINT i = nWidth; i--;) {
			if ((pXorSrc[i/8] >> (7 - ((i) & 7)) & 1) == 0)
			//if (i != j)
			{
				((UINT*)pDst)[i] = 0xFF000000 | ((UINT*)pPAL)[ pAndSrc[i] ];
			}
			//#ifdef _DEBUG
			//else {
			//	((UINT*)pDst)[i] = 0xFFFF0000;
			//}
			//#endif
		}
	}

	return pData;
}

LPBYTE Decode24BPP(UINT& nWidth, UINT nHeight, LPBYTE pXOR, LPBYTE pAND, INT32& lDstStride)
{
	UINT lXorStride = ((nWidth+31)>>5)<<2;
	UINT lAndStride = (((nWidth*24)+31)>>5)<<2;
	lDstStride = (((nWidth<<5)+31)>>5)<<2;
	LPBYTE pData = (LPBYTE)calloc(lDstStride,nHeight); // Делаем 32битное изображение

	LPBYTE pXorSrc = pXOR; LPBYTE pAndSrc = pAND; LPBYTE pDst = pData + (nHeight - 1) * lDstStride;
	for (UINT j = nHeight; j--; pXorSrc += lXorStride, pAndSrc += lAndStride, pDst -= lDstStride)
	{
		_ASSERTE(pDst >= pData);
		////memmove(pDst, pAndSrc, lAndStride);
		//for (UINT i = nWidth; i--;) {
		//	if ((pXorSrc[i/8] >> (7 - ((i) & 7)) & 1) == 0)
		//	//if (i != j)
		//	{
		//		((UINT*)pDst)[i] = /*0xFF000000 |*/ ((UINT*)pAndSrc)[i];
		//	}
		//	//#ifdef _DEBUG
		//	//else {
		//	//	((UINT*)pDst)[i] = 0xFFFF0000;
		//	//}
		//	//#endif
		//}

		UINT nStep = 0;
		DWORD nLeft = 0, nCur = 0;
		UINT i = 0;
		DWORD* p = (DWORD*)pAndSrc;

		while (i < nWidth) {
			nCur = *p;
			switch (nStep) {
			case 0:
				if ((pXorSrc[i/8] >> (7 - ((i) & 7)) & 1) == 0)
					((UINT*)pDst)[i] = 0xFF000000 | (nCur & 0xFFFFFF);
				nLeft = (nCur & 0xFF000000) >> 24;
				nStep++;
				break;
			case 1:
				if ((pXorSrc[i/8] >> (7 - ((i) & 7)) & 1) == 0)
					((UINT*)pDst)[i] = 0xFF000000 | nLeft | ((nCur & 0xFFFF) << 8);
				nLeft = (nCur & 0xFFFF0000) >> 16;
				nStep++;
				break;
			case 2:
				if ((pXorSrc[i/8] >> (7 - ((i) & 7)) & 1) == 0)
					((UINT*)pDst)[i] = 0xFF000000 | nLeft | ((nCur & 0xFF) << 16);
				nLeft = (nCur & 0xFFFFFF00) >> 8;
				i++;
				if (i < nWidth) {
					if ((pXorSrc[i/8] >> (7 - ((i) & 7)) & 1) == 0)
						((UINT*)pDst)[i] = 0xFF000000 | nLeft;
					nLeft = 0;
				}
				nStep = 0;
				break;
			}
			i++; p++;
		}										
	}

	return pData;
}

LPBYTE Decode32BPP(UINT& nWidth, UINT nHeight, LPBYTE pXOR, LPBYTE pAND, INT32& lDstStride)
{
	UINT lXorStride = ((nWidth+31)>>5)<<2;
	UINT lAndStride = (((nWidth<<5)+31)>>5)<<2;
	lDstStride = lAndStride;
	LPBYTE pData = (LPBYTE)calloc(lDstStride,nHeight); // Делаем 32битное изображение

	LPBYTE pXorSrc = pXOR; LPBYTE pAndSrc = pAND; LPBYTE pDst = pData + (nHeight - 1) * lDstStride;
	for (UINT j = nHeight; j--; pXorSrc += lXorStride, pAndSrc += lAndStride, pDst -= lDstStride)
	{
		_ASSERTE(pDst >= pData);
		//memmove(pDst, pAndSrc, lAndStride);
		for (UINT i = nWidth; i--;) {
			if ((pXorSrc[i/8] >> (7 - ((i) & 7)) & 1) == 0)
			//if (i != j)
			{
				((UINT*)pDst)[i] = /*0xFF000000 |*/ ((UINT*)pAndSrc)[i];
			}
			//#ifdef _DEBUG
			//else {
			//	((UINT*)pDst)[i] = 0xFFFF0000;
			//}
			//#endif
		}
	}

	return pData;
}

LPBYTE DecodeDummy(UINT nWidth, UINT nHeight, UINT32& nBPP)
{
	DWORD nSize = nWidth*nHeight*4;
	LPBYTE pData = (LPBYTE)malloc(nSize); // Делаем 32битное изображение
	nBPP = 32;
	memset(pData, 0xFF, nSize);
	return pData;
}

BOOL __stdcall pvdPageDecode2(void *pContext, void *pImageContext, pvdInfoDecode2 *pDecodeInfo, pvdDecodeCallback2 DecodeCallback, void *pDecodeCallbackContext)
{
	_ASSERTE(pDecodeInfo->cbSize >= sizeof(pvdInfoDecode2));
	if (!pImageContext) {
		pDecodeInfo->nErrNumber = PIE_INVALID_CONTEXT;
		return FALSE;
	}

	LPICONCONTEXT pIcon = (LPICONCONTEXT)pImageContext;

	UINT32 nIconBPP = 0, nColors = 0;

	if ((pDecodeInfo->nErrNumber = LoadPageInfo(pIcon, pDecodeInfo->iPage, nIconBPP, 
		pDecodeInfo->lWidth, pDecodeInfo->lHeight, nColors, NULL)) != NO_ERROR)
		return FALSE;

	pDecodeInfo->Flags = 0; // PVD_IDF_READONLY не нужен, т.к память под pImage выделяется специально
	pDecodeInfo->nBPP = 32;
	pDecodeInfo->ColorModel = PVD_CM_BGRA;

	LPICONDIRENTRY pImage = pIcon->Icon.idEntries + pDecodeInfo->iPage;
	LPBYTE pDataStart = ((LPBYTE)&(pIcon->Icon));
	LPBYTE pImageStart = (pDataStart + pImage->dwImageOffset);
	if (*((DWORD*)pImageStart) == 0x474e5089) {
		//PNG Mark
		TODO("Хорошо бы в заголовке показать PNG");
		BOOL lbPngLoaded = FALSE;
		#ifndef _NO_EXEPTION_
		try {
		#endif
			if (gbTokenInitialized) {
				MStream strm;
				strm.SetData(pImageStart, pImage->dwBytesInRes);
				Gdiplus::BitmapData data;
				Gdiplus::Bitmap *pBmp = Gdiplus::Bitmap::FromStream (&strm, FALSE);
				if (pBmp) {
					Gdiplus::PixelFormat pf = pBmp->GetPixelFormat();
					Gdiplus::Rect rc(0,0,pDecodeInfo->lWidth,pDecodeInfo->lHeight);
					if (!pBmp->LockBits(&rc, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &data)) {
						_ASSERTE(data.PixelFormat == PixelFormat32bppARGB);
						pDecodeInfo->lWidth = data.Width;
						pDecodeInfo->lHeight = data.Height;
						pDecodeInfo->nBPP = 32;
						_ASSERTE(data.Scan0 && data.Stride);
						pDecodeInfo->pImage = (LPBYTE)malloc(data.Height*((data.Stride<0) ? (-data.Stride) : data.Stride));
						if (pDecodeInfo->pImage) {
							if (data.Stride < 0) {
								pDecodeInfo->lImagePitch = -data.Stride;
								LPBYTE pSrc = (LPBYTE)data.Scan0;
								LPBYTE pDst = pDecodeInfo->pImage;
								for (UINT i=0; i<data.Height; i++, pDst-=data.Stride, pSrc+=data.Stride)
									memmove(pDst, pSrc, -data.Stride);
							} else {
								pDecodeInfo->lImagePitch = data.Stride;
								memmove(pDecodeInfo->pImage, data.Scan0, data.Height*data.Stride);
							}
							pDecodeInfo->Flags |= PVD_IDF_ALPHA;
							lbPngLoaded = TRUE;
						}
						pBmp->UnlockBits(&data);
					}
					delete pBmp;
				}
			}
		#ifndef _NO_EXEPTION_
		} catch(...) {
			lbPngLoaded = FALSE;
		}
		#endif

		if (!lbPngLoaded) // пустой белый квадрат
			pDecodeInfo->pImage = DecodeDummy ( pDecodeInfo->lWidth,pDecodeInfo->lHeight,pDecodeInfo->nBPP );

	} else if (*((DWORD*)pImageStart) == sizeof(BITMAPINFOHEADER)) {
		BITMAPINFOHEADER *pDIB = (BITMAPINFOHEADER*)pImageStart;
		// Icons are stored in funky format where height is doubled - account for it
		_ASSERTE(pDIB->biWidth==pDecodeInfo->lWidth && (pDecodeInfo->lHeight*2)==pDIB->biHeight);
		_ASSERTE(pDIB->biSize == sizeof(BITMAPINFOHEADER));
		RGBQUAD *pPAL = (RGBQUAD*)(((LPBYTE)pDIB) + pDIB->biSize);
		if (nColors != pDIB->biClrUsed && pDIB->biClrUsed)
			nColors = pDIB->biClrUsed;
		LPBYTE   pAND = ((LPBYTE)pPAL) + sizeof(RGBQUAD)*nColors;
		LPBYTE   pXOR = ((LPBYTE)pAND) + pDecodeInfo->lHeight
			* ((((pDecodeInfo->lWidth * nIconBPP) + 31 )>>5)<<2);

		_ASSERTE(pImage->dwBytesInRes >= (((pDecodeInfo->lWidth * nIconBPP) >> 3) * pDecodeInfo->lHeight));

		if (nIconBPP == 1) {
			pDecodeInfo->pImage = Decode1BPP ( pDecodeInfo->lWidth,pDecodeInfo->lHeight,pPAL,pXOR,pAND,pDecodeInfo->lImagePitch );
			pDecodeInfo->nBPP = 32;
			pDecodeInfo->Flags |= PVD_IDF_ALPHA;
			TODO("Хорошо бы в заголовке показать RLE");

		} else if (nIconBPP == 4) {
			pDecodeInfo->pImage = Decode4BPP ( pDecodeInfo->lWidth,pDecodeInfo->lHeight,pPAL,pXOR,pAND,pDecodeInfo->lImagePitch );
			pDecodeInfo->nBPP = 32;
			pDecodeInfo->Flags |= PVD_IDF_ALPHA;
			TODO("Хорошо бы в заголовке показать RLE");

		} else if (nIconBPP == 8) {
			pDecodeInfo->pImage = Decode8BPP ( pDecodeInfo->lWidth,pDecodeInfo->lHeight,pPAL,pXOR,pAND,pDecodeInfo->lImagePitch );
			pDecodeInfo->nBPP = 32;
			pDecodeInfo->Flags |= PVD_IDF_ALPHA;
			TODO("Хорошо бы в заголовке показать RLE");

		} else if (nIconBPP == 24) {
			pDecodeInfo->pImage = Decode24BPP(pDecodeInfo->lWidth,pDecodeInfo->lHeight,pXOR,pAND,pDecodeInfo->lImagePitch);
			pDecodeInfo->nBPP = 32;
			pDecodeInfo->Flags |= PVD_IDF_ALPHA;

		} else if (nIconBPP == 32) {
			pDecodeInfo->pImage = Decode32BPP(pDecodeInfo->lWidth,pDecodeInfo->lHeight,pXOR,pAND,pDecodeInfo->lImagePitch);
			pDecodeInfo->nBPP = 32;
			pDecodeInfo->Flags |= PVD_IDF_ALPHA;

		} else {
			//pDecodeInfo->nErrNumber = PIE_UNSUPPORTED_BPP;
			//return FALSE;
			pDecodeInfo->pImage = DecodeDummy ( pDecodeInfo->lWidth,pDecodeInfo->lHeight,pDecodeInfo->nBPP );
		}
	} else {
		// Unknown image type
		pDecodeInfo->pImage = DecodeDummy ( pDecodeInfo->lWidth,pDecodeInfo->lHeight,pDecodeInfo->nBPP );
	}

	pDecodeInfo->pPalette = NULL;
	pDecodeInfo->nColorsUsed = 0;
	//pDecodeInfo->lImagePitch = (pDecodeInfo->lWidth * pDecodeInfo->nBPP) >> 3;

	return TRUE;
}

void __stdcall pvdPageFree2(void *pContext, void *pImageContext, pvdInfoDecode2 *pDecodeInfo)
{
	_ASSERTE(pDecodeInfo->cbSize >= sizeof(pvdInfoDecode2));
	if (pDecodeInfo->pImage)
		free(pDecodeInfo->pImage);
}

void __stdcall pvdFileClose2(void *pContext, void *pImageContext)
{
	LPICONCONTEXT pIcon = (LPICONCONTEXT)pImageContext;
	if (pIcon) free(pIcon);
}

void __stdcall pvdExit2(void *pContext)
{
	#ifndef _NO_EXEPTION_
	try{
	#endif
		if (gbTokenInitialized) {
			Gdiplus::GdiplusShutdown(gdiplusToken);
			gbTokenInitialized = false;
		}
	#ifndef _NO_EXEPTION_
	} catch(...) {  }
	#endif

	if (gbCoInitialized) {
		gbCoInitialized = FALSE;
		CoUninitialize();
	}
}
