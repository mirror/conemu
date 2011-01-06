/**************************************************************************
Copyright (c) 2009 Skakov Pavel
Copyright (c) 2009 Maximus5
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

#include <windows.h>
#include "../PictureViewPlugin.h"
#include "../PVD2Helper.h"
#include "DirectDrawSurface.h"
#include "../BltHelper.h"

#pragma comment(lib, "strmiids.lib")
//#pragma comment(lib, "ddraw.lib")

#define FreeInterface(pObject) if (pObject) {(pObject)->Release(); (pObject) = NULL;}
#define DDSErrorMsg(errno,fn) { mn_LastError = errno; if (FAILED(ghLastDXError)) lstrcpy(gsLastErrFunc, fn); goto wrap; }

//#define MulDivU32(a, b, c) (uint)((unsigned long long)(a)*(b)/(c))
#define MulDivU32 ip.MulDivU32

HRESULT ghLastDXError = 0;
wchar_t gsLastErrFunc[128] = {0};

extern pvdInitPlugin2 ip;

extern DWORD nChessMateColor1;
extern DWORD nChessMateColor2;
extern DWORD nChessMateSize;

#define DEFINE_GUID_(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
	EXTERN_C const GUID DECLSPEC_SELECTANY name \
	= { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }

DEFINE_GUID_( IID_IDirectDraw2, 0xB3A6F3E0,0x2B43,0x11CF,0xA2,0xDE,0x00,0xAA,0x00,0xB9,0x33,0x56 );
DEFINE_GUID_( IID_IDirectDraw7, 0x15e65ec0,0x3b9c,0x11d2,0xb9,0x2f,0x00,0x60,0x97,0x97,0xea,0x5b );

//int DirectDrawSurface::mn_RefCount = 0;
//HWND DirectDrawSurface::m_hWnd = 0;
//LPDIRECTDRAW DirectDrawSurface::m_pDirectDraw = 0;
//LPDIRECTDRAWSURFACE DirectDrawSurface::m_pPrimarySurface = 0;
//LPDIRECTDRAWCLIPPER DirectDrawSurface::m_pClipper = 0;

DirectDraw::DirectDraw(const wchar_t* aszPluginKey)
{
	m_hWnd = NULL;
	
	m_pDirectDraw = NULL;
	m_pPrimarySurface = NULL;
	m_pBackSurface = NULL;
	m_pClipper = NULL;
	fDirectDrawCreate = NULL;
	mb_CoInitialized = FALSE;
	mb_DebugDumps = mb_DebugFills = false;
	//nCMYKparts = 0;
	//pCMYKpalette = NULL;
	//nCMYKsize = 0;
	
	memset(&msz_LastWndSize, 0, sizeof(msz_LastWndSize));
	
	//std::vector<DirectDrawSurface*> mp_Images;
	
	mn_LastError = 0;

	pszPluginKey = aszPluginKey;
	ReloadConfig();
}

DirectDraw::~DirectDraw()
{
	Free();
}

void DirectDraw::ReloadConfig()
{
	PVDSettings set(pszPluginKey);

	bool bDefault = false;
	set.GetParam(L"DebugDumps", L"bool;Debug: Dump surface data to C:\\",
		REG_BINARY, &bDefault, &mb_DebugDumps, sizeof(mb_DebugDumps));

	bDefault = false;
	set.GetParam(L"DebugFills", L"bool;Debug: Show dummy surface instead image",
		REG_BINARY, &bDefault, &mb_DebugFills, sizeof(mb_DebugFills));
}

bool DirectDraw::Init(pvdInfoDisplayInit2* pDisplayInit)
{
	_ASSERTE(!m_pDirectDraw && !m_pPrimarySurface && !m_pBackSurface && !m_pClipper);
	
	if (m_pDirectDraw && m_pPrimarySurface && m_pBackSurface && m_pClipper) {
		mn_LastError = PDXE_ALREARY_INITIALIZED;
		return false;
	}

	BltHelper::nCMYKparts = pDisplayInit->nCMYKparts;
	BltHelper::pCMYKpalette = pDisplayInit->pCMYKpalette;
	BltHelper::nCMYKsize = pDisplayInit->nCMYKsize;
	BltHelper::uCMYK2RGB = pDisplayInit->uCMYK2RGB;

	bool lbRc = false;

	DDSURFACEDESC2 ddSurfaceDesc;
	//DDSCAPS2       ddscaps;
	LPDIRECTDRAW pDirectDraw = NULL;
	
	ghLastDXError = S_OK;

	if (!mb_CoInitialized) {	
		ghLastDXError = CoInitialize(NULL);
		if (FAILED(ghLastDXError)) lstrcpy(gsLastErrFunc, L"CoInitialize");
		mb_CoInitialized = SUCCEEDED(ghLastDXError);
	}

	if (fDirectDrawCreate == NULL) {
		HMODULE hDDraw = LoadLibrary(L"ddraw.dll");
		if (hDDraw) fDirectDrawCreate = (FDirectDrawCreate)GetProcAddress(hDDraw, "DirectDrawCreate");
	}

	if (fDirectDrawCreate==NULL)
		DDSErrorMsg(PDXE_DLL_FAILED,L"(fDirectDrawCreate==NULL)");
		
	if (FAILED(ghLastDXError = fDirectDrawCreate(NULL, &pDirectDraw, NULL)) || !pDirectDraw)
		DDSErrorMsg(PDXE_DIRECTDRAWCREATE_FAILED,L"DirectDrawCreate");

	if (FAILED(ghLastDXError = pDirectDraw->QueryInterface(IID_IDirectDraw7, (void**)&m_pDirectDraw)) || !m_pDirectDraw)
		DDSErrorMsg(PDXE_DIRECTDRAWCREATE_FAILED,L"QueryInterface(IDD7)");
		
	if (FAILED(ghLastDXError = m_pDirectDraw->SetCooperativeLevel(NULL/*GetDesktopWindow()*/, DDSCL_NORMAL)))
		DDSErrorMsg(PDXE_COOPERATIVELEVEL,L"SetCooperativeLevel");
		
		
	ZeroMemory(&ddSurfaceDesc, sizeof(ddSurfaceDesc));
	ddSurfaceDesc.dwSize = sizeof(ddSurfaceDesc);
	ddSurfaceDesc.dwFlags = DDSD_CAPS; // | DDSD_BACKBUFFERCOUNT; BackBuffer можно только в Exclusive, что нам не подходит
	ddSurfaceDesc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE; // | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
	ddSurfaceDesc.dwBackBufferCount = 1;
		
	if (FAILED(ghLastDXError = m_pDirectDraw->CreateSurface(&ddSurfaceDesc, &m_pPrimarySurface, NULL)))
		DDSErrorMsg(PDXE_CREATE_PRIMARY_SURFACE,L"m_pDirectDraw->CreateSurface");

	//// Get a pointer to the back buffer
	//ZeroMemory(&ddscaps, sizeof(ddscaps));
	//ddscaps.dwCaps = DDSCAPS_BACKBUFFER;
	//if (FAILED(ghLastDXError = m_pPrimarySurface->GetAttachedSurface(&ddscaps, &m_pBackSurface)))
	//	DDSErrorMsg(PDXE_CREATE_BACK_SURFACE,L"m_pPrimarySurface->CreateSurface");
		
	//ZeroMemory(&ddSurfaceDesc, sizeof(ddSurfaceDesc));
	//ddSurfaceDesc.dwSize = sizeof(ddSurfaceDesc);
	//ddSurfaceDesc.dwFlags = DDSD_CAPS;
	//ddSurfaceDesc.ddsCaps.dwCaps = DDSCAPS_VIDEOMEMORY;
	//	
	//if (FAILED(ghLastDXError = m_pDirectDraw->CreateSurface(&ddSurfaceDesc, &m_pBackSurface, NULL)))
	//	DDSErrorMsg(PDXE_CREATE_PRIMARY_SURFACE);

		
	if (FAILED(ghLastDXError = m_pDirectDraw->CreateClipper(NULL, &m_pClipper, NULL)))
		DDSErrorMsg(PDXE_CREATE_CLIPPER,L"m_pDirectDraw->CreateClipper");
		
	if (FAILED(ghLastDXError = m_pPrimarySurface->SetClipper(m_pClipper)))
		DDSErrorMsg(PDXE_SET_CLIPPER,L"m_pPrimarySurface->SetClipper");
		
	if (GetAvailVidMem() == 0)
		goto wrap;
		
	lbRc = true;
wrap:
	if (pDirectDraw) {
		pDirectDraw->Release(); pDirectDraw = NULL;
	}
	if (!lbRc) {
		Free();
	}
	return lbRc;
}

void DirectDraw::Free(void)
{
	//std::vector<DirectDrawSurface*>::iterator i;
	//for (i = mp_Images.begin(); i != mp_Images.end(); i = mp_Images.erase(i)) {
	//	DirectDrawSurface* pdd = *i;
	//	delete pdd;
	//}

	FreeInterface(m_pClipper);
	FreeInterface(m_pBackSurface);
	FreeInterface(m_pPrimarySurface);
	FreeInterface(m_pDirectDraw);
	
	if (mb_CoInitialized) {
		mb_CoInitialized = FALSE;
		CoUninitialize();
	}
}

bool DirectDraw::SetHWnd(HWND hWnd/*, uint iManualStep = 0*/)
{
	// iManualStep == 2 был только в одном месте да и не нужен он похоже
	//if (iManualStep > 1 /*|| m_pParent*/)
	//	return true;
	
	mn_LastError = PDXE_CLIPPER_SETWND;
	
	if (!m_pClipper)
		return false;

	if (m_hWnd != hWnd)
	{
		m_hWnd = hWnd;

		if (FAILED(ghLastDXError = m_pClipper->SetHWnd(0, hWnd))) {
			mn_LastError = PDXE_WIN32_ERROR;
			return false;
		}
			
		mb_FirstMemCheck = (hWnd != NULL);
	}
		
	if (!CheckBackBuffer())
		return false;
	
	return true;
}

DWORD DirectDraw::GetAvailVidMem()
{
	HRESULT hr = S_OK;
	DWORD dwTotalRAM = 0, dwFreeRAM = 0;
	IDirectDraw7* pDD7 = NULL;
	DDSCAPS2 ddsCaps = {0};
	ddsCaps.dwCaps = DDSCAPS_VIDEOMEMORY | DDSCAPS_OFFSCREENPLAIN; // DDSCAPS_LOCALVIDMEM - ничего вроде не меняет. Все равно память на motherboard используется для интегрированных
	
	if (SUCCEEDED(hr = gpDD->m_pDirectDraw->QueryInterface(IID_IDirectDraw7, (void**)&pDD7)) && pDD7) {
		hr = pDD7->GetAvailableVidMem(&ddsCaps, &dwTotalRAM, &dwFreeRAM);
		pDD7->Release(); pDD7 = NULL;
	}

	if (FAILED(hr)) {
		ghLastDXError = hr;
		dwFreeRAM = 8 << 20; // 8 Mb
		if (ip.MessageLog) {
			wchar_t szMsg[128], szErr[64];
			if (hr == DDERR_NODIRECTDRAWHW) {
				lstrcpy(szErr, L"No DirectDraw hardware");
				// Наверное вообще запретим использование DX
				// Это происходит на виртуальных машинах VMWare без установленных VMWareTools
				// В принципе, в этом случае вывод через DX работает, но с отвратительным качеством
				// Поэтому ставим 0
				dwFreeRAM = 0;
			} else {
				wsprintf(szErr, L"hr=0x%X. Assumed to 8M free", (DWORD)hr);
			}
			
			wsprintf(szMsg, L"Can't detect video RAM size. %s", szErr);
			
			if (ip.MessageLog) ip.MessageLog(ip.pCallbackContext, szMsg, 2);
		}
		
	} 
	else
	if (!dwTotalRAM || !dwFreeRAM)
	{
		if (ip.MessageLog) {
			wchar_t szMsg[128];
			wsprintf(szMsg, L"Avail: %i, Total: %i, hr=0x%X, Assumed to 8M free",
				dwFreeRAM, dwTotalRAM, (DWORD)hr );
			if (ip.MessageLog) ip.MessageLog(ip.pCallbackContext, szMsg, 2);
		}
		dwFreeRAM = 8 << 20; // 8 Mb
	} else if (mb_FirstMemCheck) {
		wchar_t szMsg[128], szFree[16], szTotal[16];
		
		if ((dwFreeRAM >> 20) >= 8)
			wsprintf(szFree, L"%iMB", (dwFreeRAM >> 20));
		else if ((dwFreeRAM >> 10) >= 8)
			wsprintf(szFree, L"%iKB", (dwFreeRAM >> 10));
		else
			wsprintf(szFree, L"%iB", dwFreeRAM);
			
		if ((dwTotalRAM >> 20) >= 8)
			wsprintf(szTotal, L"%iMB", (dwTotalRAM >> 20));
		else if ((dwTotalRAM >> 10) >= 8)
			wsprintf(szTotal, L"%iKB", (dwTotalRAM >> 10));
		else
			wsprintf(szTotal, L"%iB", dwTotalRAM);
		
		wsprintf(szMsg, L"Video RAM Total: %s, Free: %s", szTotal, szFree);
		
		if (ip.MessageLog) ip.MessageLog(ip.pCallbackContext, szMsg, 2);
	}
	mb_FirstMemCheck = FALSE;

	//#ifdef _DEBUG
	//// для отладки. принудительное ограничение объема памяти
	//dwFreeRAM = 4 << 20;
	//#endif
	
	return dwFreeRAM;
}


bool DirectDraw::CheckBackBuffer()
{
	if (!m_hWnd) {
		FreeInterface(m_pBackSurface);
		memset(&msz_LastWndSize, 0, sizeof(msz_LastWndSize));
		return true;
	}
	
	RECT rcClient; GetClientRect(m_hWnd, &rcClient);
	if (m_pBackSurface) {
		if (msz_LastWndSize.cx != rcClient.right || msz_LastWndSize.cy != rcClient.bottom) {
			FreeInterface(m_pBackSurface);
		}
	}
	
	if (!m_pBackSurface)
	{
		DDPIXELFORMAT ddpf = {sizeof(DDPIXELFORMAT), DDPF_RGB, 0, 0x20, 0xFF0000, 0xFF00, 0xFF, 0};
		DDSURFACEDESC2 ddsd;
		ZeroMemory(&ddsd, sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_VIDEOMEMORY;
		ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
		ddsd.dwBackBufferCount = 0;
		ddsd.ddpfPixelFormat = ddpf;
		ddsd.dwWidth = rcClient.right;
		ddsd.dwHeight = rcClient.bottom;
		
		if (FAILED(m_pDirectDraw->CreateSurface(&ddsd, &m_pBackSurface, NULL)))
		{
			mn_LastError = PDXE_CREATE_BACK_SURFACE;
			return false;
		}
		
		msz_LastWndSize.cx = rcClient.right;
		msz_LastWndSize.cy = rcClient.bottom;
	}
	
	_ASSERTE(m_pBackSurface);
	return (m_pBackSurface != NULL);
}



DirectDrawSurface::DirectDrawSurface(DirectDraw *pDD, const wchar_t* aszFileName)
{
	mp_DD = pDD;
	m_pWork = NULL;
	m_nWorkSurfaces = 0;
	mn_LastError = 0;
	ms_FileName = ms_DumpName = NULL;

	if (aszFileName && *aszFileName) {
		int nLen = lstrlenW(aszFileName);
		ms_FileName = (wchar_t*)calloc(nLen+1,2);
		lstrcpyW(ms_FileName, aszFileName);

		ms_DumpName = (wchar_t*)calloc(nLen+32,2);
		const wchar_t* psz = wcsrchr(aszFileName, L'\\');
		lstrcpyW(ms_DumpName, L"C:\\"); lstrcatW(ms_DumpName, aszFileName); lstrcatW(ms_DumpName, L".dump0");
	} else {
		ms_DumpName = (wchar_t*)calloc(128,2);
		lstrcpyW(ms_DumpName, L"C:\\PicView.DX.dump0");
	}
}

DirectDrawSurface::~DirectDrawSurface()
{
	DeleteWorkSurface();
	if (ms_DumpName) { free(ms_DumpName); ms_DumpName = NULL; }
	if (ms_FileName) { free(ms_FileName); ms_FileName = NULL; }
}

void DirectDrawSurface::DeleteWorkSurface(void)
{
	if (m_pWork)
	{
		_ASSERTE(gpDD); // Если DX уже закрыт - Release вызовет исключение

		while (gpDD && m_nWorkSurfaces) {
			int n = --m_nWorkSurfaces;
			if (m_pWork[n].Surface) {
				#ifndef _NO_EXEPTION_
				__try {
				#endif
					m_pWork[n].Surface->Release();
				#ifndef _NO_EXEPTION_
				}__except( EXCEPTION_EXECUTE_HANDLER ){
					if (ip.MessageLog)
						ip.MessageLog(ip.pCallbackContext, L"!!! Exception in m_pWork[n].Surface->Release()", 1);
				}
				#endif
				m_pWork[n].Surface = NULL;
			}
		}
	
		delete[] m_pWork;
		m_pWork = NULL;
	}
	_ASSERTE(m_nWorkSurfaces == 0);
}

bool DirectDrawSurface::CreateWorkSurface(uint lWidth, uint lHeight, uint nBPP)
{
	DeleteWorkSurface();
	DDPIXELFORMAT ddpf = {sizeof(DDPIXELFORMAT), DDPF_RGB, 0, 0x20, 0xFF0000, 0xFF00, 0xFF, 0};

	//TO DO("Попробуем создать 24бит поверхность, для jpeg");
	//if (nBPP == 24) {
	//	ddpf.dwRGBBitCount = 24;
	//}

	HRESULT hr = S_OK;
	DWORD dwFreeRAM = 0;
	DDSURFACEDESC2 ddsd;
	ZeroMemory(&ddsd, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	TODO("Если требуемый размер превышает IDirectDraw7::GetAvailableVidMem - выделять в DDSCAPS_SYSTEMMEMORY");
	TODO("Только gpDD->m_pPrimarySurface->Blt после этого нифига не отображает, хотя возвращает S_OK");
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;// | DDSCAPS_SYSTEMMEMORY;
	ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
	ddsd.dwBackBufferCount = 0;
	ddsd.ddpfPixelFormat = ddpf;
	
	dwFreeRAM = gpDD->GetAvailVidMem();

	//PRAGMA_ERROR("Похоже Stride для DX нужно считать так: ((400+63)>>6)<<6");
	// Иначе на НЕКОТОРЫХ видеокартах возникают проблемы

	// 4 байта на точку + еще чуть чуть должно быть больше dwFreeRAM
	if (((lWidth * lHeight) * 4.5) >= dwFreeRAM) {
		if (ip.MessageLog) {
			wchar_t szMsg[128], szFree[16], szReq[16];

			if ((dwFreeRAM >> 20) >= 8)
				wsprintf(szFree, L"%iMB", (dwFreeRAM >> 20));
			else if ((dwFreeRAM >> 10) >= 8)
				wsprintf(szFree, L"%iKB", (dwFreeRAM >> 10));
			else
				wsprintf(szFree, L"%iB", dwFreeRAM);

			DWORD dwReq = (DWORD)((lWidth * (unsigned __int64)lHeight) >> 8);
			if ((dwReq >> 10) >= 8)
				wsprintf(szReq, L"%iMB", (dwReq >> 10));
			else
				wsprintf(szReq, L"%iKB", dwReq);

			wsprintf(szMsg, L"Required video RAM: %s, Free: %s", szReq, szFree );
			if (ip.MessageLog) ip.MessageLog(ip.pCallbackContext, szMsg, 2);
		}
		mn_LastError = PDXE_NOT_ENOUGH_VIDEO_RAM;
		return false;
	}

	m_lWorkWidth = lWidth;
	m_lWorkHeight = lHeight;

	uint MaxTexWidth = 2048;

lSplitTexture:

	m_nWidthParts = lWidth/MaxTexWidth + 1;
	m_nHeightParts = lHeight/2048 + 1;

	if (!(m_pWork = new WORK_SRF_DESC[m_nWidthParts * m_nHeightParts]))
		return false;
	ZeroMemory(m_pWork, sizeof(WORK_SRF_DESC) * m_nWidthParts * m_nHeightParts);

	for (uint h = 0; h < m_nHeightParts; h++)
		for (uint w = 0; w < m_nWidthParts; w++, m_nWorkSurfaces++)
		{
			m_pWork[m_nWorkSurfaces].lWidth = MulDivU32(lWidth, w + 1, m_nWidthParts) - MulDivU32(lWidth, w, m_nWidthParts);
			ddsd.dwWidth = m_pWork[m_nWorkSurfaces].lStrideWidth = ((m_pWork[m_nWorkSurfaces].lWidth+63)>>6)<<6;
			ddsd.dwHeight = m_pWork[m_nWorkSurfaces].lHeight = MulDivU32(lHeight, h + 1, m_nHeightParts) - MulDivU32(lHeight, h, m_nHeightParts);
			//m_pWork[m_nWorkSurfaces].pData = NULL;

			if (FAILED(gpDD->m_pDirectDraw->CreateSurface(&ddsd, &m_pWork[m_nWorkSurfaces].Surface, NULL)))
			{
				if (!m_nWorkSurfaces)
				{
					RECT DesctopRect;
					GetClientRect(GetDesktopWindow(), &DesctopRect);
					if (MaxTexWidth > (uint)DesctopRect.right)
					{
						MaxTexWidth = DesctopRect.right;
						delete[] m_pWork;
						goto lSplitTexture;
					}
				}
				if (!m_nWorkSurfaces) {
					mn_LastError = PDXE_CREATE_WORK_SURFACE;
					return false;
				}
				break;
			}
		}
	return true;
}

bool DirectDrawSurface::ViewSurface(const RECT *pImgRect, const RECT *pDisplayRect, bool bWaitForVerticalBlank)
{
	ghLastDXError = S_OK;
	if (m_nWorkSurfaces && gpDD->m_pBackSurface)
	{
		if (gpDD->m_pBackSurface->IsLost())
			gpDD->m_pBackSurface->Restore();
		for (uint i = 0; i < m_nWorkSurfaces; i++)
			if (m_pWork[i].Surface->IsLost())
				m_pWork[i].Surface->Restore();
		DDBLTFX fx;
		ZeroMemory(&fx, sizeof(DDBLTFX));
		fx.dwSize = sizeof(DDBLTFX);

		//if (bWaitForVerticalBlank) {
		//	ghLastDXError = gpDD->m_pDirectDraw->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN, NULL);
		//	if (FAILED(ghLastDXError)) {
		//		lstrcpy(gsLastErrFunc, L"gpDD->m_pDirectDraw->WaitForVerticalBlank");
		//		wchar_t szDbg[128]; wsprintf(szDbg, L"m_pDirectDraw->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN) = 0x%08X", (DWORD)ghLastDXError);
		//		if (ip.MessageLog) ip.MessageLog(ip.pCallbackContext, szDbg, 2);
		//		//_ASSERTE(SUCCEEDED(ghLastDXError));
		//	}
		//}

		uint lDispWidth = pDisplayRect->right - pDisplayRect->left, lDispHeight = pDisplayRect->bottom - pDisplayRect->top;
		uint lViewWidth = pImgRect->right - pImgRect->left, lViewHeight = pImgRect->bottom - pImgRect->top;
		for (uint h = 0, i = 0; h < m_nHeightParts; h++)
			for (uint w = 0; w < m_nWidthParts && i < m_nWorkSurfaces; w++, i++)
			{
				RECT glb = { // -- координаты поверхности {w-h} в координатах изображения
					MulDivU32(m_lWorkWidth, w, m_nWidthParts), MulDivU32(m_lWorkHeight, h, m_nHeightParts),
					MulDivU32(m_lWorkWidth, w+1, m_nWidthParts), MulDivU32(m_lWorkHeight, h+1, m_nHeightParts)};

				RECT glbint;
				if (!IntersectRect(&glbint, &glb, pImgRect)) // Если не попадают в видимую область изображения
					continue; // пропускаем эту поверхность

				RECT in = {	// Если часть поверхности не попадает на экран - локальные координаты поверхности
					glbint.left - glb.left,
					glbint.top - glb.top,
					glbint.right - glb.left,
					glbint.bottom - glb.top
				};

				RECT out = {
					pDisplayRect->left + MulDivU32((glb.left+in.left-pImgRect->left),lDispWidth,lViewWidth),
					pDisplayRect->top + MulDivU32((glb.top+in.top-pImgRect->top),lDispHeight,lViewHeight),
					pDisplayRect->left + MulDivU32((glb.left+in.right-pImgRect->left),lDispWidth,lViewWidth),
					pDisplayRect->top + MulDivU32((glb.top+in.bottom-pImgRect->top),lDispHeight,lViewHeight)
				};

				// Собственно, вывод на экран
				ghLastDXError = gpDD->m_pBackSurface->Blt(&out, m_pWork[i].Surface, &in, DDBLT_WAIT, &fx);

				if (FAILED(ghLastDXError)) {
					lstrcpy(gsLastErrFunc, L"gpDD->m_pBackSurface->Blt(Surface)");
					wchar_t szDbg[128]; wsprintf(szDbg, L"m_pBackSurface->Blt(m_pWork[i]) = 0x%08X", (DWORD)ghLastDXError);
					if (ip.MessageLog) ip.MessageLog(ip.pCallbackContext, szDbg, 2);
					//_ASSERTE(SUCCEEDED(ghLastDXError));
				}
			}
		return (ghLastDXError == S_OK); // было true;
	}
	return false;
}

//DWORD DirectDrawSurface::CMYK2BGR_FAST(DWORD Src)
//{
//	if (!Src) // In CMYK images, pure white is generated when all four components have values of 0%.
//		return 0xFFFFFF;
//		
//	DWORD C = (Src & 0xFF);
//	DWORD M = (Src & 0xFF00) >> 8;
//	DWORD Y = (Src & 0xFF0000) >> 16;
//	DWORD K = (Src & 0xFF000000) >> 24;
//	DWORD R,G,B;
//
//	R = ((C + K) < 255) ? (255 - (C + K)) : 0;
//	G = ((M + K) < 255) ? (255 - (M + K)) : 0;
//	B = ((Y + K) < 255) ? (255 - (Y + K)) : 0;
//	
//	return (B) | (G << 8) | (R << 16);
//}
//
//DWORD DirectDrawSurface::CMYK2BGR_APPROX(DWORD Src)
//{
//	if (!Src) // In CMYK images, pure white is generated when all four components have values of 0%.
//		return 0xFFFFFF;
//
//	_ASSERTE(gpDD->pCMYKpalette && gpDD->nCMYKsize);
//
//	DWORD C = (Src & 0xFF);
//	DWORD C0 = C >> 4;
//	DWORD M = (Src & 0xFF00) >> 8;
//	DWORD M0 = M >> 4;
//	DWORD Y = (Src & 0xFF0000) >> 16;
//	DWORD Y0 = Y >> 4;
//	DWORD K = (Src & 0xFF000000) >> 24;
//	DWORD K0 = K >> 4;
//
//	DWORD N = (C0 << 12) | (M0 << 8) | (Y0 << 4) | (K0);
//	if (N >= gpDD->nCMYKsize) {
//		_ASSERTE(N < gpDD->nCMYKsize);
//		return 0;
//	}
//
//	return gpDD->pCMYKpalette[N];
//}
//
//DWORD DirectDrawSurface::CMYK2BGR_PRECISE(DWORD Src)
//{
//	if (!Src) // In CMYK images, pure white is generated when all four components have values of 0%.
//		return 0xFFFFFF;
//
//	_ASSERTE(gpDD->pCMYKpalette && gpDD->nCMYKsize);
//
//	DWORD C = (Src & 0xFF);
//	DWORD C0 = C >> 4;
//	DWORD M = (Src & 0xFF00) >> 8;
//	DWORD M0 = M >> 4;
//	DWORD Y = (Src & 0xFF0000) >> 16;
//	DWORD Y0 = Y >> 4;
//	DWORD K = (Src & 0xFF000000) >> 24;
//	DWORD K0 = K >> 4;
//
//	DWORD N = (C0 << 12) | (M0 << 8) | (Y0 << 4) | (K0);
//	if (N >= gpDD->nCMYKsize) {
//		_ASSERTE(N < gpDD->nCMYKsize);
//		return 0;
//	}
//
//	DWORD RGB32 = gpDD->pCMYKpalette[N];
//
//	bool bCMYKpreciese = true;
//	// Если хотят "точное" отображение цветов
//	if (bCMYKpreciese) {
//		DWORD C_ = (C0 << 4);
//		DWORD C1 = (C0 == 0xF) ? 0xF : ((C_ == C) ? C0 : (C0+1));
//		DWORD M_ = (M0 << 4);
//		DWORD M1 = (M0 == 0xF) ? 0xF : ((M_ == M) ? M0 : (M0+1));
//		DWORD Y_ = (Y0 << 4);
//		DWORD Y1 = (Y0 == 0xF) ? 0xF : ((Y_ == Y) ? Y0 : (Y0+1));
//		DWORD K_ = (K0 << 4);
//		DWORD K1 = (K0 == 0xF) ? 0xF : ((K_ == K) ? K0 : (K0+1));
//
//		N = (C1 << 12) | (M1 << 8) | (Y1 << 4) | (K1);
//		if (N >= gpDD->nCMYKsize) {
//			_ASSERTE(N < gpDD->nCMYKsize);
//			return RGB32;
//		}
//
//		DWORD RGB32_1 = gpDD->pCMYKpalette[N];
//
//		_ASSERTE(gpDD->nCMYKparts == 17);
//
//		int R = RED_FROM_BGRA(RGB32);
//			if (C > C_) {
//				int R1 = RED_FROM_BGRA(RGB32_1);
//				R += (R1 - R) * (int)(C - C_) / 17;
//				if (R<0) R=0; else if (R>255) R=255;
//			}
//		int G = GREEN_FROM_BGRA(RGB32);
//			if (M > M_) {
//				int G1 = GREEN_FROM_BGRA(RGB32_1);
//				G += (G1 - G) * (int)(M - M_) / 17;
//				if (G<0) G=0; else if (G>255) G=255;
//			}
//		int B = BLUE_FROM_BGRA(RGB32);
//			if (Y > Y_) {
//				int B1 = BLUE_FROM_BGRA(RGB32_1);
//				B += (B1 - B) * (int)(Y - Y_) / 17;
//				if (B<0) B=0; else if (B>255) B=255;
//			}
//
//		RGB32 = 0xFF000000 | (((DWORD)R) << 16) | (((DWORD)G) << 8) | (((DWORD)B));
//	}
//
//	return RGB32;
//}
//
//// Нифига не работает. зависимости R/G/B меняются при различных C/M/Y/K
////DWORD __inline DirectDrawSurface::CMYK2BGR_FUNC(DWORD Src)
////{
////	if (!Src) // In CMYK images, pure white is generated when all four components have values of 0%.
////		return 0xFFFFFF;
////		
////	// For example, a bright red might contain 2% cyan, 93% magenta, 90% yellow, and 0% black
////		
////	register DWORD C0 = (Src & 0xFF), C = C0 << 8;
////	register DWORD M0 = (Src & 0xFF00) >> 8, M = (Src & 0xFF00);
////	register DWORD Y0 = (Src & 0xFF0000) >> 16, Y = (Src & 0xFF0000) >> 8;
////	register DWORD K0 = (Src & 0xFF000000) >> 24, K = (Src & 0xFF000000) >> 16;
////	register int R = 255, G = 255, B = 255;
////
////	// Функция от C примерно такая
////	if (C0) {
////		if (C0 < 115)
////			R -= C / 230;
////		else if (C0 < 166)
////			R -= (C / 165) - 50;
////		else if (C0 < 179)
////			R -= (C / 101) - 200;
////		else
////			R = 0;
////
////		if (C0 < 140)
////			G -= C / 680;
////		else
////			G -= (C >> 10) + 18;
////
////		B -= (C >> 12);
////	}
////
////	if (M0) {
////		R -= (M / 3350);
////
////		if (M0 < 179)   G -= M / 325;      		else
////		if (M0 < 166)   G -= (M >> 8) - 38; 	else
////		if (M0 != 255)  G -= (M / 110) - 340;
////		else            G = 0;
////
////		if (M0 < 140)	B -= M / 530;
////		else			B -= (M / 620) -10;
////	}
////
////	if (Y0) {
////		// R от Y не зависит
////
////		G -= (Y / 4600);
////
////		if (Y0 < 166)	B -= Y / 320;			else
////		if (Y0 < 230)	B -= (Y / 200) + 80;	else
////		if (Y0 < 245)	B -= (Y / 80) + 522;
////		else			B = 0;
////	}
////
////	if (K0) {
////		//R -= (K / 297);
////		if (!K0)		;						else
////		if (K0 < 77 )	R -= (K / 300) - 3;		else
////		if (K0 < 217)	R -= (K / 328) - 8;
////		else			R -= (K / 230) + 64;
////
////		//G -= (K / 292);
////		if (!K0)		;						else
////		if (K0 < 77)	G -= (K / 315) - 3;		else
////		if (K0 < 204)	G -= (K / 328) - 5;
////		else			G -= (K / 220) + 74;
////
////		//B -= (K / 293);
////		if (!K0)		;						else
////		if (K0 < 115)	B -= (K / 330) - 3;		else
////		if (K0 < 217)	B -= (K / 310) + 4;
////		else			B -= (K / 200) + 103;
////	}
////
////	//R = ((C + K) < 255) ? (255 - (C + K)) : 0;
////	//G = ((M + K) < 255) ? (255 - (M + K)) : 0;
////	//B = ((Y + K) < 255) ? (255 - (Y + K)) : 0;
////
////	if (R < 0) R = 0; else if (R > 255) R = 255;
////	if (G < 0) G = 0; else if (G > 255) G = 255;
////	if (B < 0) B = 0; else if (B > 255) B = 255;
////	
////	return ((DWORD)B) | (((DWORD)G) << 8) | (((DWORD)R) << 16);
////}
//
//
//BYTE __inline DirectDrawSurface::BlendFunction(DWORD Src, DWORD Back, DWORD Alpha)
//{
//	DWORD nRc = Src * Alpha >> 8;
//	if (Back)
//		nRc += Back - (Back * Alpha >> 8);
//	return (nRc > 255) ? 255 : (BYTE)nRc;
//}
//
//void DirectDrawSurface::ApplyBlendFunction32(u8* pDst, size_t cbSize, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent)
//{
//	// Глючит на прозрачности (alpha канал)
//	if ((idfFlags & PVD_IDF_ALPHA) == PVD_IDF_ALPHA) {
//		RGBQUAD *pStart = (RGBQUAD*)pDst, *pEnd = (RGBQUAD*)(pDst + cbSize);
//		//RGBQUAD Back; *((DWORD*)&Back) = nBackground;
//		DWORD nAlpha = 0, nValue = 0, nNewValue = 0;
//		RGBQUAD Dst;
//		DWORD nCount = cbSize >> 2;
//		//#define RED(x) ((x & 0xFF))
//		//#define GREEN(x) ((x & 0xFF00) >> 8)
//		//#define BLUE(x) ((x & 0xFF0000) >> 16)
//		//#define ALPHA(x) (x & 0xFF)
//		DWORD nBackR = RED_FROM_BGRA(nBackground); //Back.rgbRed;
//		DWORD nBackG = GREEN_FROM_BGRA(nBackground); //Back.rgbGreen;
//		DWORD nBackB = BLUE_FROM_BGRA(nBackground); //Back.rgbBlue;
//		//nBackground = (nBackB) | (nBackG << 8) | (nBackR << 16) /*| 0xFF00000000*/;
//		
//		// Попробовать пооптимизировать - сколько времени через
//		// поля структуры, а сколько через маски и сдвиги
//
//		//nBackground |= 0xFF000000;
//		// Старший байт вообще не используем
//		//nBackground &= 0xFFFFFF; -- МЛАДШИЙ а не старший байт это альфа канал
//
//		/*while (nCount--) {
//			nValue = *pStart;
//			nAlpha = ALPHA(nValue);
//			if (!nAlpha) {
//				nNewValue = nBackground;
//			} else if (nAlpha != 0xFF) {
//				nNewValue = 0xFF;
//				/ *nNewValue |= BlendFunction(RED(nValue), nBackR, nAlpha) << 24;
//				nNewValue |= BlendFunction(GREEN(nValue), nBackR, nAlpha) << 16;
//				nNewValue |= BlendFunction(BLUE(nValue), nBackR, nAlpha) << 8;* /
//				nNewValue |= nValue;
//			}
//			*(pStart++) = nNewValue;
//		}*/
//
//		//nCount = 0;
//		
//		for (pStart = (RGBQUAD*)pDst; pStart < pEnd; pStart++) {
//		//for (DWORD n = 0; n < cbSize; n++) {
//			nAlpha = pStart->rgbReserved; //0;//ALPHA(*pStart);
//			//nAlpha = (*((DWORD*)pStart)) & 0xFF000000;
//			if (!nAlpha) {
//				*((DWORD*)pStart) = nBackground;
//			} else if (nAlpha != 0xFF) {
//			//} else if (nAlpha != 0xFF000000) {
//				//nAlpha = nAlpha >> 24;
//				// Тут тоже попробовать через сдвиги а не через поля структуры
//				Dst.rgbBlue = BlendFunction(pStart->rgbBlue, nBackB, nAlpha);
//				Dst.rgbGreen = BlendFunction(pStart->rgbGreen, nBackG, nAlpha);
//				Dst.rgbRed = BlendFunction(pStart->rgbRed, nBackR, nAlpha);
//				//Dst.rgbReserved = 0xFF; -- не требуется
//				*pStart = Dst;
//			}
//		}
//	} else if ((idfFlags & PVD_IDF_TRANSPARENT) == PVD_IDF_TRANSPARENT) {
//		DWORD *pStart = (DWORD*)pDst, *pEnd = (DWORD*)(pDst + cbSize);
//		for (; pStart < pEnd; pStart++) {
//			if (*pStart == nTransparent)
//				*pStart = nBackground;
//		}
//	}
//	//else {
//		// т.к. при создании Surface мы не заказывали альфа канал - содержимое старшего
//		// байта просто игнорируется
//		////DWORD *pEnd = (DWORD*)(pDst + cbSize);
//		////for (DWORD *pStart = (DWORD*)pDst; pStart < pEnd; pStart++) *pStart |= 0xFF000000;
//	//}
//}
//
//
//bool DirectDrawSurface::Blit32_CMYK(u8*& pDst, u8*& pSrc, const uint iSrcPoint, uint nPoints, uint lHeight, uint lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette)
//{
//	pSrc += iSrcPoint*4;
//
//	if (!gpDD->pCMYKpalette || !gpDD->nCMYKsize || gpDD->uCMYK2RGB == PVD_CMYK2RGB_FAST) {
//		for (uint j = lHeight; j--; pSrc += lAbsSrcPitch, pDst += lDstPitch) {
//			for (uint i = nPoints; i--;)
//				((DWORD*)pDst)[i] = CMYK2BGR_FAST(((DWORD*)pSrc)[i]);
//		}
//	} else if (gpDD->uCMYK2RGB == PVD_CMYK2RGB_PRECISE) {
//		for (uint j = lHeight; j--; pSrc += lAbsSrcPitch, pDst += lDstPitch) {
//			for (uint i = nPoints; i--;)
//				((DWORD*)pDst)[i] = CMYK2BGR_PRECISE(((DWORD*)pSrc)[i]);
//		}
//	} else {
//		for (uint j = lHeight; j--; pSrc += lAbsSrcPitch, pDst += lDstPitch) {
//			for (uint i = nPoints; i--;)
//				((DWORD*)pDst)[i] = CMYK2BGR_APPROX(((DWORD*)pSrc)[i]);
//		}
//	}
//
//	return true;
//}
//
//bool DirectDrawSurface::Blit32_BGRA(u8*& pDst, u8*& pSrc, const uint iSrcPoint, uint nPoints, uint lHeight, uint lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette)
//{
//	pSrc += iSrcPoint*4;
//	size_t cbSize = lHeight * lAbsSrcPitch;
//	
//	memcpy(pDst, pSrc, cbSize);
//
//	return true;
//}
//bool DirectDrawSurface::Blit32_BGRA_AT(u8*& pDst, u8*& pSrc, const uint iSrcPoint, uint nPoints, uint lHeight, uint lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette)
//{
//	pSrc += iSrcPoint*4;
//	size_t cbSize = lHeight * lAbsSrcPitch;
//	
//	memcpy(pDst, pSrc, cbSize);
//
//	ApplyBlendFunction32(pDst, cbSize, nBackground, idfFlags, nTransparent);
//
//	return true;
//}
//bool DirectDrawSurface::Blit32_BGRA_VP(u8*& pDst, u8*& pSrc, const uint iSrcPoint, uint nPoints, uint lHeight, uint lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette)
//{
//	pSrc += iSrcPoint*4;
//	size_t cbSize = nPoints * 4;
//	
//	for (uint j = lHeight; j--; pSrc += lAbsSrcPitch, pDst += lDstPitch) {
//		memcpy(pDst, pSrc, cbSize);
//		ApplyBlendFunction32(pDst, cbSize, nBackground, idfFlags, nTransparent);
//	}
//	
//	return true;
//}
//bool DirectDrawSurface::Blit32_BGRA_AT_VP(u8*& pDst, u8*& pSrc, const uint iSrcPoint, uint nPoints, uint lHeight, uint lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette)
//{
//	pSrc += iSrcPoint*4;
//	size_t cbSize = nPoints * 4;
//	
//	for (uint j = lHeight; j--; pSrc += lAbsSrcPitch, pDst += lDstPitch) {
//		memcpy(pDst, pSrc, cbSize);
//
//		ApplyBlendFunction32(pDst, cbSize, nBackground, idfFlags, nTransparent);
//	}
//	
//	return true;
//}
//
//bool DirectDrawSurface::Blit24_BGR_T(u8*& pDst, u8*& pSrc, const uint iSrcPoint, uint nPoints, uint lHeight, uint lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette)
//{
//	DWORD NewColor;
//	pSrc += iSrcPoint*3;
//	for (uint j = lHeight; j--; pSrc += lAbsSrcPitch, pDst += lDstPitch)
//	{
//		UINT nStep = 0;
//		DWORD nLeft = 0, nCur = 0;
//		uint i = 0;
//		DWORD* p = (DWORD*)pSrc;
//		DWORD* pp = (DWORD*)pDst;
//
//		while (i < nPoints) {
//			nCur = *p;
//			switch (nStep) {
//			case 0:
//				NewColor = (nCur & 0xFFFFFF);
//				*pp = (NewColor != nTransparent) ? NewColor : nBackground;
//				nLeft = (nCur & 0xFF000000) >> 24;
//				nStep++;
//				break;
//			case 1:
//				NewColor = nLeft | ((nCur & 0xFFFF) << 8);
//				*pp = (NewColor != nTransparent) ? NewColor : nBackground;
//				nLeft = (nCur & 0xFFFF0000) >> 16;
//				nStep++;
//				break;
//			case 2:
//				NewColor = nLeft | ((nCur & 0xFF) << 16);
//				*pp = (NewColor != nTransparent) ? NewColor : nBackground;
//				nLeft = (nCur & 0xFFFFFF00) >> 8;
//				i++; pp++;
//				if (i < nPoints) {
//					*pp = (nLeft != nTransparent) ? nLeft : nBackground;
//					nLeft = 0;
//				}
//				nStep = 0;
//				break;
//			}
//			i++; p++; pp++;
//		}										
//	}
//	return true;
//}
//bool DirectDrawSurface::Blit24_BGR_24(u8*& pDst, u8*& pSrc, const uint iSrcPoint, uint nPoints, uint lHeight, uint lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette)
//{
//	pSrc += iSrcPoint*3;
//	size_t cbSize = lHeight * lAbsSrcPitch;
//	
//	memcpy(pDst, pSrc, cbSize);
//
//	return true;
//}
//bool DirectDrawSurface::Blit24_BGR(u8*& pDst, u8*& pSrc, const uint iSrcPoint, uint nPoints, uint lHeight, uint lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette)
//{
//	pSrc += iSrcPoint*3;
//	for (uint j = lHeight; j--; pSrc += lAbsSrcPitch, pDst += lDstPitch)
//	{
//		UINT nStep = 0;
//		DWORD nLeft = 0, nCur = 0;
//		uint i = 0;
//		DWORD* p = (DWORD*)pSrc;
//		DWORD* pp = (DWORD*)pDst;
//
//		while (i < nPoints) {
//			nCur = *p;
//			switch (nStep) {
//			case 0:
//				*pp = (nCur & 0xFFFFFF);
//				nLeft = (nCur & 0xFF000000) >> 24;
//				nStep++;
//				break;
//			case 1:
//				*pp = nLeft | ((nCur & 0xFFFF) << 8);
//				nLeft = (nCur & 0xFFFF0000) >> 16;
//				nStep++;
//				break;
//			case 2:
//				*pp = nLeft | ((nCur & 0xFF) << 16);
//				nLeft = (nCur & 0xFFFFFF00) >> 8;
//				i++; pp++;
//				if (i < nPoints) {
//					*pp = nLeft;
//					nLeft = 0;
//				}
//				nStep = 0;
//				break;
//			}
//			i++; p++; pp++;
//		}										
//	}
//	return true;
//}
//
//bool DirectDrawSurface::Blit16_BGR_T(u8*& pDst, u8*& pSrc, const uint iSrcPoint, uint nPoints, uint lHeight, uint lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette)
//{
//	COLORREF NewColor;
//	pSrc += iSrcPoint*2;
//	for (uint j = lHeight; j--; pSrc += lAbsSrcPitch, pDst += lDstPitch)
//	{
//		for (uint i = nPoints; i--;)
//		{
//			// реально используется 15бит, требуется конвертирование в 24бит
//			const uint t = ((unsigned short*)pSrc)[i], 
//				t2 = (t & 0x7C00) << 9 | (t & 0x03E0) << 6 | (t & 0x001F) << 3;
//			NewColor = t2 | t2 >> 5 & 0x00070707;
//			if (NewColor == nTransparent)
//				((u32*)pDst)[i] = nBackground;
//			else
//				((u32*)pDst)[i] = NewColor;
//		}
//	}
//	return true;
//}
//bool DirectDrawSurface::Blit16_BGR(u8*& pDst, u8*& pSrc, const uint iSrcPoint, uint nPoints, uint lHeight, uint lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette)
//{
//	pSrc += iSrcPoint*2;
//	for (uint j = lHeight; j--; pSrc += lAbsSrcPitch, pDst += lDstPitch)
//	{
//		for (uint i = nPoints; i--;)
//		{
//			// реально используется 15бит, требуется конвертирование в 24бит
//			const uint t = ((unsigned short*)pSrc)[i], 
//				t2 = (t & 0x7C00) << 9 | (t & 0x03E0) << 6 | (t & 0x001F) << 3;
//			((u32*)pDst)[i] = t2 | t2 >> 5 & 0x00070707;
//		}
//	}
//	return true;
//}
//
//TODO("Во всех функциях с палитрой проверять макс индекс, или что лучше, делать копию палитры и догонять ее до максимального размера черным непрозрачным цветом");
//bool DirectDrawSurface::Blit8_PAL(u8*& pDst, u8*& pSrc, const uint iSrcPoint, uint nPoints, uint lHeight, uint lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette)
//{
//	//UINT NewColor;
//	pSrc += iSrcPoint;
//	for (uint j = lHeight; j--; pSrc += lAbsSrcPitch, pDst += lDstPitch)
//	{
//		for (uint i = nPoints; i--;)
//			((u32*)pDst)[i] = ((u32*)pPalette)[pSrc[i]];
//	}
//	return true;
//}
//bool DirectDrawSurface::Blit8_PAL_A(u8*& pDst, u8*& pSrc, const uint iSrcPoint, uint nPoints, uint lHeight, uint lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette)
//{
//	//UINT NewColor;
//	pSrc += iSrcPoint;
//	for (uint j = lHeight; j--; pSrc += lAbsSrcPitch, pDst += lDstPitch)
//	{
//		for (uint i = nPoints; i--;)
//			((u32*)pDst)[i] = ((u32*)pPalette)[pSrc[i]];
//		// Alpha post processing
//		ApplyBlendFunction32(pDst, nPoints*sizeof(u32), nBackground, idfFlags, nTransparent);
//	}
//	return true;
//}
//bool DirectDrawSurface::Blit8_PAL_T(u8*& pDst, u8*& pSrc, const uint iSrcPoint, uint nPoints, uint lHeight, uint lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette)
//{
//	UINT NewColor;
//	pSrc += iSrcPoint;
//	for (uint j = lHeight; j--; pSrc += lAbsSrcPitch, pDst += lDstPitch)
//	{
//		for (uint i = nPoints; i--;) {
//			NewColor = ((u32*)pPalette)[pSrc[i]];
//			((u32*)pDst)[i] = (NewColor == nTransparent) ? nBackground : NewColor;
//		}
//	}
//	return true;
//}
//bool DirectDrawSurface::Blit8_PAL_TI(u8*& pDst, u8*& pSrc, const uint iSrcPoint, uint nPoints, uint lHeight, uint lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette)
//{
//	UINT NewColor;
//	pSrc += iSrcPoint;
//	for (uint j = lHeight; j--; pSrc += lAbsSrcPitch, pDst += lDstPitch)
//	{
//		for (uint i = nPoints; i--;) {
//			NewColor = pSrc[i];
//			((u32*)pDst)[i] = (NewColor != nTransparent) ? ((u32*)pPalette)[NewColor] : nBackground;
//		}
//	}
//	return true;
//}
//
//
//bool DirectDrawSurface::Blit4_PAL(u8*& pDst, u8*& pSrc, const uint iSrcPoint, uint nPoints, uint lHeight, uint lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette)
//{
//	TODO("Во многих случаях можно сэкономить на (iSrcPoint + i)/?, если iSrcPoint==0 - просто сразу сделать ветки кода");
//	//UINT NewColor;
//	for (uint j = lHeight; j--; pSrc += lAbsSrcPitch, pDst += lDstPitch)
//	{
//		for (uint i = nPoints; i--;) {
//			TODO("Можно бы на один байт pSrc обработать сразу две точки в pDst");
//			((u32*)pDst)[i] = ((u32*)pPalette)[
//				(iSrcPoint + i) & 1 ? pSrc[(iSrcPoint + i)/2] & 0x0F 
//					: pSrc[(iSrcPoint + i)/2] >> 4];
//		}
//	}
//	return true;
//}
//bool DirectDrawSurface::Blit4_PAL_A(u8*& pDst, u8*& pSrc, const uint iSrcPoint, uint nPoints, uint lHeight, uint lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette)
//{
//	TODO("Во многих случаях можно сэкономить на (iSrcPoint + i)/?, если iSrcPoint==0 - просто сразу сделать ветки кода");
//	//UINT NewColor;
//	for (uint j = lHeight; j--; pSrc += lAbsSrcPitch, pDst += lDstPitch)
//	{
//		for (uint i = nPoints; i--;) {
//			TODO("Можно бы на один байт pSrc обработать сразу две точки в pDst");
//			((u32*)pDst)[i] = ((u32*)pPalette)[
//				(iSrcPoint + i) & 1 ? pSrc[(iSrcPoint + i)/2] & 0x0F 
//					: pSrc[(iSrcPoint + i)/2] >> 4];
//		}
//		ApplyBlendFunction32(pDst, nPoints*sizeof(u32), nBackground, idfFlags, nTransparent);
//	}
//	return true;
//}
//bool DirectDrawSurface::Blit4_PAL_T(u8*& pDst, u8*& pSrc, const uint iSrcPoint, uint nPoints, uint lHeight, uint lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette)
//{
//	TODO("Во многих случаях можно сэкономить на (iSrcPoint + i)/?, если iSrcPoint==0 - просто сразу сделать ветки кода");
//	UINT NewColor;
//	for (uint j = lHeight; j--; pSrc += lAbsSrcPitch, pDst += lDstPitch)
//	{
//		for (uint i = nPoints; i--;) {
//			TODO("Можно бы на один байт pSrc обработать сразу две точки в pDst");
//			NewColor = ((u32*)pPalette)[
//				(iSrcPoint + i) & 1 ? pSrc[(iSrcPoint + i)/2] & 0x0F 
//					: pSrc[(iSrcPoint + i)/2] >> 4];
//			if (NewColor == nTransparent) NewColor = nBackground;
//			((u32*)pDst)[i] = NewColor;
//		}
//	}
//	return true;
//}
//bool DirectDrawSurface::Blit4_PAL_TI(u8*& pDst, u8*& pSrc, const uint iSrcPoint, uint nPoints, uint lHeight, uint lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette)
//{
//	TODO("Во многих случаях можно сэкономить на (iSrcPoint + i)/?, если iSrcPoint==0 - просто сразу сделать ветки кода");
//	UINT NewColor;
//	for (uint j = lHeight; j--; pSrc += lAbsSrcPitch, pDst += lDstPitch)
//	{
//		for (uint i = nPoints; i--;) {
//			TODO("Можно бы на один байт pSrc обработать сразу две точки в pDst");
//			NewColor = (iSrcPoint + i) & 1 ? pSrc[(iSrcPoint + i)/2] & 0x0F 
//				: pSrc[(iSrcPoint + i)/2] >> 4;
//			((u32*)pDst)[i] = (NewColor != nTransparent) ? ((u32*)pPalette)[NewColor] : nBackground;
//		}
//	}
//	return true;
//}
//
//
//bool DirectDrawSurface::Blit2_PAL(u8*& pDst, u8*& pSrc, const uint iSrcPoint, uint nPoints, uint lHeight, uint lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette)
//{
//	TODO("Во многих случаях можно сэкономить на (iSrcPoint + i)/?, если iSrcPoint==0 - просто сразу сделать ветки кода");
//	for (uint j = lHeight; j--; pSrc += lAbsSrcPitch, pDst += lDstPitch)
//	{
//		for (uint i = nPoints; i--;)
//			((u32*)pDst)[i] = ((u32*)pPalette)[pSrc[(iSrcPoint + i)/4] >> ((3 - ((iSrcPoint + i) & 3)) & 3)*2];
//	}
//	return true;
//}
//bool DirectDrawSurface::Blit2_PAL_A(u8*& pDst, u8*& pSrc, const uint iSrcPoint, uint nPoints, uint lHeight, uint lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette)
//{
//	TODO("Во многих случаях можно сэкономить на (iSrcPoint + i)/?, если iSrcPoint==0 - просто сразу сделать ветки кода");
//	for (uint j = lHeight; j--; pSrc += lAbsSrcPitch, pDst += lDstPitch)
//	{
//		for (uint i = nPoints; i--;)
//			((u32*)pDst)[i] = ((u32*)pPalette)[pSrc[(iSrcPoint + i)/4] >> ((3 - ((iSrcPoint + i) & 3)) & 3)*2];
//		ApplyBlendFunction32(pDst, nPoints*sizeof(u32), nBackground, idfFlags, nTransparent);
//	}
//	return true;
//}
//
//bool DirectDrawSurface::Blit1_PAL(u8*& pDst, u8*& pSrc, const uint iSrcPoint, uint nPoints, uint lHeight, uint lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette)
//{
//	TODO("Во многих случаях можно сэкономить на (iSrcPoint + i)/?, если iSrcPoint==0 - просто сразу сделать ветки кода");
//	for (uint j = lHeight; j--; pSrc += lAbsSrcPitch, pDst += lDstPitch)
//	{
//		for (uint i = nPoints; i--;)
//			((u32*)pDst)[i] = ((u32*)pPalette)[pSrc[(iSrcPoint + i)/8] >> (7 - ((iSrcPoint + i) & 7)) & 1];
//	}
//	return true;
//}
//bool DirectDrawSurface::Blit1_PAL_A(u8*& pDst, u8*& pSrc, const uint iSrcPoint, uint nPoints, uint lHeight, uint lAbsSrcPitch, int lDstPitch, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette)
//{
//	TODO("Во многих случаях можно сэкономить на (iSrcPoint + i)/?, если iSrcPoint==0 - просто сразу сделать ветки кода");
//	for (uint j = lHeight; j--; pSrc += lAbsSrcPitch, pDst += lDstPitch)
//	{
//		for (uint i = nPoints; i--;)
//			((u32*)pDst)[i] = ((u32*)pPalette)[pSrc[(iSrcPoint + i)/8] >> (7 - ((iSrcPoint + i) & 7)) & 1];
//		// Alpha post processing
//		ApplyBlendFunction32(pDst, nPoints*sizeof(u32), nBackground, idfFlags, nTransparent);
//	}
//	return true;
//}
//
//
//
//
//
//DirectDrawSurface::BlitFunction_t DirectDrawSurface::BlitChoose(
//			uint nDstBPP, uint nBPP, uint lAbsSrcPitch, int lDstPitch, BOOL lbAlpha, BOOL lbTransColor, BOOL lbTransIndex,
//			bool &result, bool &bMayContinue, const void *&pPalette, pvdColorModel ColorModel)
//{
//	BlitFunction_t BlitFunction = NULL;
//	
//						switch (nBPP)
//						{
//							case 0x20: // 32bit BGR/BGRA (альфа канал может отсутствовать)
//								_ASSERTE(nDstBPP==32);
//								if (ColorModel == PVD_CM_CMYK) {
//									BlitFunction = Blit32_CMYK;
//								} else
//								if (lAbsSrcPitch == lDstPitch) {
//									if (lbAlpha || lbTransColor)
//										BlitFunction = Blit32_BGRA_AT;
//									else
//										BlitFunction = Blit32_BGRA;
//								} else {
//									if (lbAlpha || lbTransColor)
//										BlitFunction = Blit32_BGRA_AT_VP;
//									else
//										BlitFunction = Blit32_BGRA_VP;
//								}
//								break;
//							case 0x18: // 24bit, BGR
//								_ASSERTE(nDstBPP==32 || nDstBPP==24);
//								if (lbAlpha) {
//									OutputDebugString(L"Alpha is not supported in 24 bit\n");
//									result = bMayContinue = false;
//								} else if (nDstBPP==24) {
//									BlitFunction = Blit24_BGR_24;
//								} else if (lbTransColor) {
//									BlitFunction = Blit24_BGR_T;
//								} else {
//									BlitFunction = Blit24_BGR;
//								}
//								break;
//							case 0x10: // 16bit
//								_ASSERTE(nDstBPP==32);
//								if (lbAlpha) {
//									OutputDebugString(L"Alpha is not supported in 16 bit\n");
//									result = bMayContinue = false;
//								} else if (lbTransColor) {
//									BlitFunction = Blit16_BGR_T;
//								} else {
//									BlitFunction = Blit16_BGR;
//								}
//								break;
//							case 0x08:
//								_ASSERTE(nDstBPP==32);
//								if (!pPalette) {
//									//_ASSERTE(pPalette);
//									OutputDebugString(L"\n!!! Palette is not defined !!!\n");
//									result = bMayContinue = false;
//								} else
//								if (lbAlpha) {
//									BlitFunction = Blit8_PAL_A;
//								} else if (lbTransColor) {
//									BlitFunction = Blit8_PAL_T;
//								} else if (lbTransIndex) {
//									BlitFunction = Blit8_PAL_TI;
//								} else {
//									BlitFunction = Blit8_PAL;
//								}
//								break;
//							case 0x04:
//								_ASSERTE(nDstBPP==32);
//								if (!pPalette) {
//									//_ASSERTE(pPalette);
//									OutputDebugString(L"\n!!! Palette is not defined !!!\n");
//									result = bMayContinue = false;
//								} else
//								if (lbAlpha) {
//									BlitFunction = Blit4_PAL_A;
//								} else if (lbTransColor) {
//									BlitFunction = Blit4_PAL_T;
//								} else if (lbTransIndex) {
//									BlitFunction = Blit4_PAL_TI;
//								} else {
//									BlitFunction = Blit4_PAL;
//								}
//								break;
//							case 0x02:
//								_ASSERTE(nDstBPP==32);
//								if (!pPalette) {
//									//_ASSERTE(pPalette);
//									OutputDebugString(L"\n!!! Palette is not defined !!!\n");
//									result = bMayContinue = false;
//								} else
//								if (lbAlpha) {
//									BlitFunction = Blit2_PAL_A;
//								} else {
//									TODO("Не хватает Blit2_PAL_T & Blit2_PAL_TI");
//									BlitFunction = Blit2_PAL;
//								}
//								break;
//							case 0x01:
//								_ASSERTE(nDstBPP==32);
//								if (!pPalette) {
//									//_ASSERTE(pPalette);
//									OutputDebugString(L"\n!!! Palette is not defined !!!\n");
//									result = bMayContinue = false;
//								} else
//								if (lbAlpha) {
//									BlitFunction = Blit1_PAL_A;
//								} else {
//									TODO("Не хватает Blit1_PAL_T & Blit1_PAL_TI");
//									BlitFunction = Blit1_PAL;
//								}
//								break;
//							default:
//							{
//								wchar_t szMsg[128];
//								wsprintf(szMsg, L"Unsupported bit depth = %i\n", nBPP);
//								OutputDebugString(szMsg);
//								result = bMayContinue = false;
//							}
//						}
//						
//	return BlitFunction;
//}



//(const void *pSource, int lSrcPitch, uint nBPP, COLORREF nBackground, DWORD idfFlags, COLORREF nTransparent, const void *pPalette)
bool DirectDrawSurface::Blit(struct pvdInfoDecode2 *pDecodeInfo, COLORREF nBackground)
{
	bool result = false;
	if (!m_nWorkSurfaces)
	{
		if (ip.MessageLog) ip.MessageLog(ip.pCallbackContext, L"Work surfaces was not created! (m_nWorkSurfaces == 0)\n", 2);
	}
	else
	{
		const void *pSource = pDecodeInfo->pImage;
		int lSrcPitch = pDecodeInfo->lImagePitch;
		uint nBPP = pDecodeInfo->nBPP;
		DWORD idfFlags = pDecodeInfo->Flags;
		COLORREF nTransparent = pDecodeInfo->nTransparentColor;
		const void *pPalette = pDecodeInfo->pPalette;

		if (gpDD->mb_DebugDumps) {
			Dump(0, pDecodeInfo->lWidth, pDecodeInfo->lHeight, pDecodeInfo->lImagePitch, pDecodeInfo->nBPP, pDecodeInfo->pImage);
		}

		int lAbsSrcPitch = lSrcPitch, lDstPitch;
		if (lSrcPitch < 0)
		{
			pSource = (u8*)pSource - lSrcPitch * (int)(m_lWorkHeight - 1);
			lAbsSrcPitch = -lAbsSrcPitch;
		}

		DDSURFACEDESC2 desc = {sizeof(DDSURFACEDESC2)};
		DDPIXELFORMAT ddpf = {sizeof(DDPIXELFORMAT), DDPF_RGB, 0, 0x20, 0xFF0000, 0xFF00, 0xFF, 0};
		desc.dwFlags = DDSD_PIXELFORMAT;
		desc.ddpfPixelFormat = ddpf;

		if (gpDD->mb_DebugFills) {
			DDBLTFX fx;
			RECT rcFill;
			ZeroMemory(&fx, sizeof(DDBLTFX));
			fx.dwSize = sizeof(DDBLTFX);
			for (uint h = 0, i = 0; h < m_nHeightParts; h++)
			{
				for (uint w = 0; w < m_nWidthParts && i < m_nWorkSurfaces; w++, i++)
				{
					fx.dwFillColor = 0xFFDDBBAA;
					m_pWork[i].Surface->Blt(NULL, NULL, NULL, DDBLT_WAIT | DDBLT_COLORFILL, &fx);

					if (m_pWork[i].lStrideWidth >= 8 && m_pWork[i].lHeight >= 8)
					{
						uint x1 = m_pWork[i].lWidth / 2;
						uint x2 = m_pWork[i].lWidth - 1;
						uint y1 = m_pWork[i].lHeight / 2;
						uint y2 = m_pWork[i].lHeight - 1;

						//Red
						fx.dwFillColor = 0xFFFF0000;
						rcFill.left = 1; rcFill.top = 1; rcFill.right = x1; rcFill.bottom = y1;
						m_pWork[i].Surface->Blt(&rcFill, NULL, NULL, DDBLT_WAIT | DDBLT_COLORFILL, &fx);

						// Green
						fx.dwFillColor = 0xFF00FF00;
						rcFill.left = x1+1; rcFill.right = x2;
						m_pWork[i].Surface->Blt(&rcFill, NULL, NULL, DDBLT_WAIT | DDBLT_COLORFILL, &fx);

						// Blue
						fx.dwFillColor = 0xFF0000FF;
						rcFill.top = y1+1; rcFill.left = 1; rcFill.right = x1; rcFill.bottom = y2;
						m_pWork[i].Surface->Blt(&rcFill, NULL, NULL, DDBLT_WAIT | DDBLT_COLORFILL, &fx);

						// White
						fx.dwFillColor = 0xFFFFFFFF;
						rcFill.top = y1+1; rcFill.left = x1+1; rcFill.right = x2; rcFill.bottom = y2;
						m_pWork[i].Surface->Blt(&rcFill, NULL, NULL, DDBLT_WAIT | DDBLT_COLORFILL, &fx);
					}
				}
			}

			result = true;

		} else {
			BOOL lbAlpha = ((idfFlags & PVD_IDF_ALPHA) == PVD_IDF_ALPHA), lbTransColor = FALSE, lbTransIndex = FALSE;
			if (!lbAlpha) {
				lbTransColor = (idfFlags & PVD_IDF_TRANSPARENT) == PVD_IDF_TRANSPARENT;
				if (!lbTransColor)
					lbTransIndex = (idfFlags & PVD_IDF_TRANSPARENT_INDEX) == PVD_IDF_TRANSPARENT_INDEX;
			}

			nBackground = BGRA_FROM_RGBA(nBackground);
			
			BltHelper::BlitFunction_t BlitFunction = NULL;

			TODO("Заранее сделать анализ типа изображения и выбрать функцию заполнения (см.выше)");
			TODO("Если тип изображения (например CMYK) не поддерживается - сразу выйти, не делая Lock");

			bool bMayContinue = true;
			bool bFirstLocked = false;
			uint h = 0, i = 0, w = 0;
			int iLockedSurface = -1;
			DWORD nLockFlags = DDLOCK_WAIT | DDLOCK_WRITEONLY;
			
			if (!m_pWork[0].lWidth) {
				TODO("Хорошо бы это выкинуть в статус субплагина вывода");
				OutputDebugString(L"m_pWork[i].lWidth == 0\n");
				bMayContinue = false;
			} else
			if (FAILED(m_pWork[0].Surface->Lock(NULL, &desc, nLockFlags, NULL)))
			{
				TODO("Хорошо бы это выкинуть в статус субплагина вывода");
				OutputDebugString(L"Can't lock surface memory!\n");
				bMayContinue = false;
			} else {
				iLockedSurface = 0;
				bFirstLocked = true;
			}
			
			
			const uint nPoints = m_pWork[i].lWidth;
			lDstPitch = (lSrcPitch < 0) ? -desc.lPitch : desc.lPitch;
				
			BlitFunction = BltHelper::BlitChoose(desc.ddpfPixelFormat.dwRGBBitCount, nBPP, lAbsSrcPitch, lDstPitch, lbAlpha, lbTransColor, lbTransIndex,
				result, bMayContinue, pPalette, pDecodeInfo->ColorModel);
			if (!BlitFunction) {
				result = bMayContinue = false;
			}
			
			#ifndef _NO_EXEPTION_
			__try
			{
			#endif
				for (h = 0, i = 0; bMayContinue && h < m_nHeightParts; h++)
				{
					for (w = 0; bMayContinue && w < m_nWidthParts && i < m_nWorkSurfaces; w++, i++)
					{
						if (!m_pWork[i].lWidth) {
							OutputDebugString(L"m_pWork[i].lWidth == 0\n");
							bMayContinue = false; break;
						}

						if (bFirstLocked) {
							bFirstLocked = false; // первая поверхность уже заблокирована
						} else	
						if (FAILED(m_pWork[i].Surface->Lock(NULL, &desc, nLockFlags, NULL)))
						{
							OutputDebugString(L"Can't lock surface memory!\n");
							bMayContinue = false; break;
						} else {
							iLockedSurface = i;
						}
						
						{
								result = true;
								const uint nPoints = m_pWork[i].lWidth;
								u8 *pDst = (u8*)desc.lpSurface;
								u8 *pSrc = (u8*)pSource + MulDivU32(m_lWorkHeight, h, m_nHeightParts)*lSrcPitch;
								if (lSrcPitch < 0)
								{
									pSrc += (int)(m_pWork[i].lHeight - 1) * lSrcPitch;
									pDst += (int)(m_pWork[i].lHeight - 1) * desc.lPitch;
									desc.lPitch = -desc.lPitch;
								}
								
								// Если desc.lPitch не совпадает с полученным для первого куска - нужно переопределить функцию блита
								if (lDstPitch != desc.lPitch) {
									BlitFunction = BltHelper::BlitChoose(desc.ddpfPixelFormat.dwRGBBitCount, nBPP, lAbsSrcPitch, lDstPitch, lbAlpha, lbTransColor, lbTransIndex,
										result, bMayContinue, pPalette, pDecodeInfo->ColorModel);
								}
								_ASSERTE(BlitFunction!=NULL);
								if (!BlitFunction) {
									result = bMayContinue = false;
								} else {
								
									const uint iSrcPoint = MulDivU32(m_lWorkWidth, w, m_nWidthParts);
									
									// Теперь собственно Blit
									
									bMayContinue = BlitFunction(pDst, pSrc, iSrcPoint, nPoints, m_pWork[i].lHeight,
										lAbsSrcPitch, desc.lPitch, nBackground, idfFlags, nTransparent, pPalette);
										
									// Blit этой поверхности закончен

									if (i == 0 && gpDD->mb_DebugDumps) {
										Dump(1, desc.dwWidth, desc.dwHeight, desc.lPitch, desc.ddpfPixelFormat.dwRGBBitCount, desc.lpSurface, TRUE);
									}
								}
						}
						m_pWork[i].Surface->Unlock(NULL);
						iLockedSurface = -1;
					}
				} // for (h = 0, i = 0; bMayContinue && h < m_nHeightParts; h++)
			#ifndef _NO_EXEPTION_
			}__except( EXCEPTION_EXECUTE_HANDLER ){
				;
			}
			#endif
		
			if (iLockedSurface != -1) {
				#ifndef _NO_EXEPTION_
				__try {
				#endif
					m_pWork[iLockedSurface].Surface->Unlock(NULL);
				#ifndef _NO_EXEPTION_
				}__except( EXCEPTION_EXECUTE_HANDLER ){
					;
				}
				#endif
				iLockedSurface = -1;
			}
		}
	}

	if (result && gpDD->mb_DebugDumps)  {
		DDSURFACEDESC2 desc = {sizeof(DDSURFACEDESC2)};
		//DDPIXELFORMAT ddpf = {sizeof(DDPIXELFORMAT), DDPF_RGB, 0, 0x20, 0xFF0000, 0xFF00, 0xFF, 0};
		//desc.dwFlags = DDSD_PIXELFORMAT;
		//desc.ddpfPixelFormat = ddpf;

		if (SUCCEEDED(m_pWork[0].Surface->Lock(NULL, &desc, DDLOCK_WAIT | DDLOCK_READONLY, NULL))) {
			Dump(2, desc.dwWidth, desc.dwHeight, desc.lPitch, desc.ddpfPixelFormat.dwRGBBitCount, desc.lpSurface, TRUE);
			m_pWork[0].Surface->Unlock(NULL);
		}
	}

	return result;
}

void DirectDrawSurface::Dump(uint nNo, uint nWidth, uint nHeight, int nPitch, uint nBPP, LPCVOID pData, BOOL bMakeCopy/*=FALSE*/)
{
	int nLen = lstrlenW(ms_DumpName);
	if (nLen<=1) return;
	ms_DumpName[nLen-1] = (wchar_t)(nNo + (uint)L'0');

	HANDLE hFile = CreateFile(ms_DumpName, GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile == INVALID_HANDLE_VALUE) {
		wchar_t szDbgMsg[128]; DWORD dwErrNo = GetLastError();
		wsprintf(szDbgMsg, L"Can't create dump file, ErrCode=0x%08X", dwErrNo);
		if (ip.MessageLog) ip.MessageLog(ip.pCallbackContext, szDbgMsg, 2);
	} else {
		DWORD dwWrite = 0;
		WriteFile(hFile, &nWidth, 4, &dwWrite, 0);
		WriteFile(hFile, &nHeight, 4, &dwWrite, 0);
		WriteFile(hFile, &nPitch, 4, &dwWrite, 0);
		WriteFile(hFile, &nBPP, 4, &dwWrite, 0);
		if (nPitch < 0) nPitch = -nPitch;
		DWORD dwSize = nPitch*nHeight, dwErr = 0;
		if (dwSize > 8388608) dwSize = 8388608; // 8 MB
		LPBYTE pCopy = NULL;
		if (bMakeCopy) {
			pCopy = (LPBYTE)malloc(dwSize);
			__try {
				memmove(pCopy, pData, dwSize);
			}__except( EXCEPTION_EXECUTE_HANDLER ){
				if (ip.MessageLog) ip.MessageLog(ip.pCallbackContext, L"Dump buffer is not valid!", 2);
			}
			pData = pCopy;
		}
		if (!WriteFile(hFile, pData, dwSize, &dwWrite, 0) || dwWrite < dwSize) {
			dwErr = GetLastError();
			wchar_t szDbg[128]; wsprintf(szDbg, L"Can't write %i bytes to dump file, ErrCode=0x%08X", dwSize, dwErr);
			if (ip.MessageLog) ip.MessageLog(ip.pCallbackContext, szDbg, 2);
		}
		if (pCopy) { free(pCopy); pCopy = NULL; }
		CloseHandle(hFile);
	}
}
