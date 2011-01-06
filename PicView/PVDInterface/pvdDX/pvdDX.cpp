
#include <windows.h>
#include <crtdbg.h>
#include <shellapi.h>
//#include <vector>
#include <GdiPlus.h>
#include "../PictureViewPlugin.h"
#include "../PVD2Helper.h"
#include "../BltHelper.h"
#include "DirectDrawSurface.h"


template <class T> const T& Min(const T &a, const T &b) {return a < b ? a : b;}
template <class T> const T& Max(const T &a, const T &b) {return a > b ? a : b;}


extern HRESULT ghLastDXError;
extern wchar_t gsLastErrFunc[128];
pvdInitPlugin2 ip = {0};


HMODULE ghModule = NULL;

// Функция требуется только для заполнения переменной ghModule
// Если плагин содержит свою точку входа - для использования PVD1Helper
// ему необходимо заполнять ghModule самостоятельно
BOOL WINAPI DllMain( HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved )
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
		ghModule = (HMODULE)hModule;
	return TRUE;
}


BOOL __stdcall pvdTranslateError2(DWORD nErrNumber, wchar_t *pszErrInfo, int nBufLen)
{
	if (!pszErrInfo || nBufLen<=0)
		return FALSE;
		
	#define PRINTERR(e) case e: wsprintf(pszErrInfo, L"%s(0x%08X)", L#e, (DWORD)ghLastDXError);	break

	switch (nErrNumber)
	{
	case PDXE_WIN32_ERROR:
		{
		wsprintf(pszErrInfo, L"Error 0x%08X in function ", (DWORD)ghLastDXError);
		int nLen = lstrlen(pszErrInfo);
		nBufLen -= nLen;
		lstrcpyn(pszErrInfo+nLen, gsLastErrFunc, nBufLen);
		}
		//FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS, 0, 
		//	(DWORD)ghLastDXError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		//	pszErrInfo, nBufLen, NULL);
		break;
	PRINTERR(PDXE_ALREARY_INITIALIZED);
	PRINTERR(PDXE_DLL_FAILED);
	PRINTERR(PDXE_DIRECTDRAWCREATE_FAILED);
	PRINTERR(PDXE_COOPERATIVELEVEL);
	PRINTERR(PDXE_CREATE_PRIMARY_SURFACE);
	PRINTERR(PDXE_CREATE_BACK_SURFACE);
	PRINTERR(PDXE_CREATE_CLIPPER);
	PRINTERR(PDXE_SET_CLIPPER);
	PRINTERR(PDXE_CLIPPER_SETWND);
	PRINTERR(PDXE_CREATE_WORK_SURFACE);
	PRINTERR(PDXE_NO_DISPLAY_WINDOW);
	PRINTERR(PDXE_INIT2_WAS_NOT_CALLED);
	PRINTERR(PDXE_MEMORY_ALLOCATION_FAILED);
	PRINTERR(PDXE_INVALID_ARGUMENT);
	PRINTERR(PDXE_NOT_ENOUGH_VIDEO_RAM);
	PRINTERR(PDXE_OLD_DISPLAYCREATE);
	PRINTERR(PDXE_INVALID_PAINTCONTEXT);
	case PDXE_OLD_PICVIEW:
		lstrcpyn(pszErrInfo, L"Old PicView version, exiting", nBufLen); break;
	default:
		return FALSE;
	}
	return TRUE;
}

void __stdcall pvdPluginInfo2(pvdInfoPlugin2 *pPluginInfo)
{
	_ASSERTE(pPluginInfo->cbSize >= sizeof(pvdInfoPlugin2));
	pPluginInfo->Priority = 0;
	pPluginInfo->pName = L"DirectX";
	pPluginInfo->pVersion = GetVersion(pPluginInfo->hModule);
	pPluginInfo->pComments = L"Copyright © 2009 Maximus5";
	pPluginInfo->Flags = PVD_IP_DISPLAY|PVD_IP_NOTERMINAL|PVD_IP_DIRECT;
}

DirectDraw* gpDD = NULL;

UINT32 __stdcall pvdInit2(pvdInitPlugin2* pInit)
{
	_ASSERTE(pInit->cbSize >= sizeof(pvdInitPlugin2));
	if (pInit->cbSize < sizeof(pvdInitPlugin2)) {
		pInit->nErrNumber = PDXE_OLD_PICVIEW;
		return 0;
	}

	_ASSERTE(gpDD == NULL);
	memset(&ip,0,sizeof(ip));
	memmove(&ip, pInit, min(sizeof(ip),pInit->cbSize));
	ghModule = ip.hModule;
	
	if (gpDD == NULL) {
		gpDD = new DirectDraw(ip.pRegKey);
		if (gpDD == NULL) {
			pInit->nErrNumber = PDXE_MEMORY_ALLOCATION_FAILED;
			return 0;
		}
	}

	pInit->pContext = gpDD;
	pInit->nErrNumber = 0;

	return PVD_UNICODE_INTERFACE_VERSION;
}

void __stdcall pvdExit2(void *pContext)
{
	_ASSERTE((void*)gpDD == pContext);
	if (gpDD) {
		delete gpDD;
		gpDD = NULL;
	}
}

// Инициализация контекста дисплея. Используется тот pContext, который был получен в pvdInit2
BOOL __stdcall pvdDisplayInit2(void *pContext, pvdInfoDisplayInit2* pDisplayInit)
{
	_ASSERTE((void*)gpDD == pContext);
	if (!gpDD) {
		pDisplayInit->nErrNumber = PDXE_INIT2_WAS_NOT_CALLED;
		return FALSE;
	}
	
	BOOL lbRc = gpDD->Init(pDisplayInit);
	if (!lbRc)
		pDisplayInit->nErrNumber = gpDD->mn_LastError;

	return lbRc;
}

BOOL __stdcall pvdDisplayAttach2(void *pContext, pvdInfoDisplayAttach2* pDisplayAttach)
{
	BOOL lbRc = FALSE;
	_ASSERTE(pDisplayAttach->cbSize>=sizeof(pvdInfoDisplayAttach2));
	if (pDisplayAttach->bAttach) {
		if (gpDD) {
			lbRc = gpDD->SetHWnd(pDisplayAttach->hWnd);
			if (!lbRc)
				pDisplayAttach->nErrNumber = gpDD->mn_LastError;
		}
	} else {
		if (gpDD)
			gpDD->SetHWnd(NULL);
		lbRc = TRUE;
	}
	return lbRc;
}

// Создать контекст для отображения картинки в pContext (перенос декодированных данных в видеопамять)
BOOL __stdcall pvdDisplayCreate2(void *pContext, pvdInfoDisplayCreate2* pDisplayCreate)
{
	_ASSERTE((void*)gpDD == pContext);
	if (!gpDD) {
		pDisplayCreate->nErrNumber = PDXE_INIT2_WAS_NOT_CALLED;
		return FALSE;
	}
	if (pDisplayCreate->cbSize < sizeof(pvdInfoDisplayCreate2)) {
		pDisplayCreate->nErrNumber = PDXE_OLD_DISPLAYCREATE;
		return FALSE;
	}
	
	DWORD nBackColor = pDisplayCreate->BackColor;
	//BltHelper::nChessMateColor1 = pDisplayCreate->nChessMateColor1;
	//BltHelper::nChessMateColor2 = pDisplayCreate->nChessMateColor2;
	//BltHelper::nChessMateSize = pDisplayCreate->nChessMateSize;
	//if (BltHelper::nChessMateColor1 == BltHelper::nChessMateColor2) {
	//	BltHelper::nChessMateSize = 0;
	//}
	
	
	BOOL lbRc = FALSE;
	DirectDrawSurface* pDDS = new DirectDrawSurface(gpDD, pDisplayCreate->pFileName);
	if (!pDDS) {
		pDisplayCreate->nErrNumber = PDXE_MEMORY_ALLOCATION_FAILED;
		return FALSE;
	}
	
	if (!pDDS->CreateWorkSurface(pDisplayCreate->pImage->lWidth, pDisplayCreate->pImage->lHeight, pDisplayCreate->pImage->nBPP))
		pDisplayCreate->nErrNumber = pDDS->mn_LastError;
	else if (!pDDS->Blit(pDisplayCreate->pImage, nBackColor))
		pDisplayCreate->nErrNumber = pDDS->mn_LastError;
	else
		lbRc = TRUE;
		
	if (!lbRc) {
		delete pDDS;
		pDDS = NULL;
	} else {
		pDisplayCreate->pDisplayContext = pDDS;
	}

	return lbRc;
}

// Собственно отрисовка. Функция должна при необходимости выполнять "Stretch"
BOOL __stdcall pvdDisplayPaint2(void *pContext, void* pDisplayContext, pvdInfoDisplayPaint2* pDisplayPaint)
{
	_ASSERTE((void*)gpDD == pContext);
	if (!gpDD) {
		pDisplayPaint->nErrNumber = PDXE_INIT2_WAS_NOT_CALLED;
		return FALSE;
	}

	_ASSERTE(pDisplayContext!=NULL);
	if (!pDisplayContext) {
		pDisplayPaint->nErrNumber = PDXE_INVALID_PAINTCONTEXT;
		return FALSE;
	}
	DirectDrawSurface* pDDS = (DirectDrawSurface*)pDisplayContext;
	
	BOOL lbRc = FALSE;

	switch (pDisplayPaint->Operation)
	{
	case PVD_IDP_BEGIN:
		{
			lbRc = gpDD->SetHWnd(pDisplayPaint->hWnd);
			if (!lbRc) {
				pDisplayPaint->nErrNumber = gpDD->mn_LastError;
			} else {
				RECT ImgRect = {0,0,gpDD->msz_LastWndSize.cx,gpDD->msz_LastWndSize.cy};

				//MapWindowPoints(pDisplayPaint->hWnd, NULL, (LPPOINT)&ImgRect, 2);
				DDBLTFX fx;
				ZeroMemory(&fx, sizeof(DDBLTFX));
				fx.dwSize = sizeof(DDBLTFX);
				fx.dwFillColor = BGRA_FROM_RGBA(pDisplayPaint->nBackColor);

				ghLastDXError = gpDD->m_pBackSurface->Blt(&ImgRect, NULL, NULL, DDBLT_WAIT | DDBLT_COLORFILL, &fx);
				HRESULT hRestore = -1;
				if (ghLastDXError == DDERR_SURFACELOST)
				{
					// После этой операции нужно перезаполнить поверхности, поэтому возвращаем ошибку
					hRestore = gpDD->m_pDirectDraw->RestoreAllSurfaces();
				}
				if (FAILED(ghLastDXError)) {
					if (ghLastDXError == DDERR_INVALIDRECT)
					{
						wchar_t szMsg[128]; wsprintf(szMsg, L"DDERR_INVALIDRECT({%ix%i}-{%ix%i}", ImgRect.left, ImgRect.top, ImgRect.right, ImgRect.bottom);
						if (ip.MessageLog) ip.MessageLog(ip.pCallbackContext, szMsg, 2);
					}
					lstrcpy(gsLastErrFunc, L"gpDD->m_pBackSurface->Blt(COLORFILL)");
				}

				lbRc = SUCCEEDED(ghLastDXError);
				if (!lbRc)
					pDisplayPaint->nErrNumber = PDXE_WIN32_ERROR;
			}

			//ghLastDXError = gpDD->m_pDirectDraw->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN, NULL);
			//if (FAILED(ghLastDXError)) {
			//	wchar_t szDbg[128]; wprintf(szDbg, L"m_pDirectDraw->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN) = 0x%08X", (DWORD)ghLastDXError);
			//	if (ip.MessageLog) ip.MessageLog(ip.pCallbackContext, szDbg, 2);
			//	//_ASSERTE(SUCCEEDED(ghLastDXError));
			//}

			//pDisplayPaint->pDrawContext = p;
			//lbRc = TRUE;
		} break;
	case PVD_IDP_COMMIT:
		{
			RECT Out = {0,0,gpDD->msz_LastWndSize.cx,gpDD->msz_LastWndSize.cy};
			MapWindowPoints(pDisplayPaint->hWnd, NULL, (LPPOINT)&Out, 2);
			RECT In = {0,0,gpDD->msz_LastWndSize.cx,gpDD->msz_LastWndSize.cy};
			//HRESULT hRet = S_OK;
			DDBLTFX fx = {sizeof(DDBLTFX)};

			ghLastDXError = gpDD->m_pDirectDraw->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN, NULL);
			// Собственно, вывод на экран
			ghLastDXError = gpDD->m_pPrimarySurface->Blt(&Out, gpDD->m_pBackSurface, &In, DDBLT_WAIT, &fx);
			//while (TRUE)
			//{
			//	hRet = gpDD->m_pPrimarySurface->Flip(NULL, 0);
			//	if (hRet == DD_OK)
			//		break;
			//	if (hRet == DDERR_SURFACELOST)
			//	{
			//		hRet = gpDD->m_pPrimarySurface->Restore();
			//		if (hRet != DD_OK)
			//			break;
			//	}
			//	if (hRet != DDERR_WASSTILLDRAWING)
			//		break;
			//}
			//ghLastDXError = hRet;

			RECT rcClient; GetClientRect(pDisplayPaint->hWnd, &rcClient);
			ValidateRect(pDisplayPaint->hWnd, &rcClient);
			//pDisplayPaint->pDrawContext = NULL;
			lbRc = TRUE;
		} break;
	case PVD_IDP_COLORFILL:
		{
			//RECT ImgRect = pDisplayPaint->DisplayRect;

			////MapWindowPoints(pDisplayPaint->hWnd, NULL, (LPPOINT)&ImgRect, 2);
			//DDBLTFX fx;
			//ZeroMemory(&fx, sizeof(DDBLTFX));
			//fx.dwSize = sizeof(DDBLTFX);
			//fx.dwFillColor = BGRA_FROM_RGBA(pDisplayPaint->nBackColor);

			//ghLastDXError = gpDD->m_pBackSurface->Blt(&ImgRect, NULL, NULL, DDBLT_WAIT | DDBLT_COLORFILL, &fx);
			//if (FAILED(ghLastDXError)) {
			//	if (ghLastDXError == DDERR_INVALIDRECT) {
			//		wchar_t szMsg[128]; wsprintf(szMsg, L"DDERR_INVALIDRECT({%ix%i}-{%ix%i}", ImgRect.left, ImgRect.top, ImgRect.right, ImgRect.bottom);
			//		if (ip.MessageLog) ip.MessageLog(ip.pCallbackContext, szMsg, 2);
			//	}
			//	lstrcpy(gsLastErrFunc, L"gpDD->m_pBackSurface->Blt(COLORFILL)");
			//}

			//lbRc = SUCCEEDED(ghLastDXError);
			//if (!lbRc)
			//	pDisplayPaint->nErrNumber = PDXE_WIN32_ERROR;
			lbRc = TRUE;
		} break;
	case PVD_IDP_PAINT:
		{
			RECT GlobalRect = pDisplayPaint->ImageRect;
			RECT ParentRect = pDisplayPaint->DisplayRect;

			//MapWindowPoints(pDisplayPaint->hWnd, NULL, (LPPOINT)&GlobalRect, 2);
			//MapWindowPoints(pDisplayPaint->hWnd, NULL, (LPPOINT)&ParentRect, 2);

			lbRc = pDDS->ViewSurface(&GlobalRect, &ParentRect);
		}
	}

	return lbRc;
}

// Закрыть контекст для отображения картинки (освободить видеопамять)
void __stdcall pvdDisplayClose2(void *pContext, void* pDisplayContext)
{
	_ASSERTE((void*)gpDD == pContext);
	if (!gpDD) {
		return;
	}
	
	_ASSERTE(pDisplayContext!=NULL);
	DirectDrawSurface* pDDS = (DirectDrawSurface*)pDisplayContext;
	if (pDDS) {
		delete pDDS;
	}
}

// Закрыть модуль вывода (освобождение интерфейсов DX, отцепиться от окна)
void __stdcall pvdDisplayExit2(void *pContext)
{
	_ASSERTE((void*)gpDD == pContext);
	if (gpDD) {
		gpDD->Free();
	}
}

void __stdcall pvdReloadConfig2(void *pContext)
{
	_ASSERTE((void*)gpDD == pContext);
	if (gpDD) {
		gpDD->ReloadConfig();
	}
}
