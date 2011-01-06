
#include <windows.h>
#include "wincodec.h"
#include <windowsx.h>
#include "../PictureViewPlugin.h"
#include "../PVD2Helper.h"
#include "../MStream.h"

typedef __int32 i32;
typedef unsigned __int8 u8;
typedef unsigned __int16 u16;
typedef DWORD u32;

extern HMODULE ghModule;
wchar_t* gsExtensions = NULL;

#define DEFAULT_INTERPOLATION_MODE 1  // WICBitmapInterpolationModeLinear
DWORD gnInterpolationMode = DEFAULT_INTERPOLATION_MODE;

LPCWSTR gpszPluginKey = NULL;

// Функция требуется только для заполнения переменной ghModule
// Если плагин содержит свою точку входа - для использования PVD1Helper
// ему необходимо заполнять ghModule самостоятельно
//BOOL WINAPI DllMain( HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved )
//{
//	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
//		ghModule = (HMODULE)hModule;
//	return TRUE;
//}

pvdInitPlugin2 ip = {0};

#define PWE_INVALID_FRAME        0x1001
#define PWE_EXCEPTION            0x80001002
#define PWE_FILE_NOT_FOUND       0x80001003
#define PWE_NOT_ENOUGH_MEMORY    0x80001004
#define PWE_INVALID_CONTEXT      0x80001005
#define PWE_WIN32_ERROR          0x80001007
#define PWE_UNKNOWN_COLORSPACE   0x80001008
#define PWE_INVALID_PAGEDATA     0x8000100A
#define PWE_UNKNOWN_FORMAT       0x8000100B
#define PWE_OLD_PICVIEW          0x8000100C

HRESULT ghLastWin32Error = 0;
wchar_t gsLastErrFunc[128] = {0};

BOOL __stdcall pvdTranslateError2(DWORD nErrNumber, wchar_t *pszErrInfo, int nBufLen)
{
	if (!pszErrInfo || nBufLen<=0)
		return FALSE;

	switch (nErrNumber)
	{
	case PWE_WIN32_ERROR:
		{
		wsprintf(pszErrInfo, L"Error 0x%08X in function ", (DWORD)ghLastWin32Error);
		int nLen = lstrlen(pszErrInfo);
		nBufLen -= nLen;
		lstrcpyn(pszErrInfo+nLen, gsLastErrFunc, nBufLen);
		}
		//FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS, 0, 
		//	(DWORD)ghLastWin32Error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		//	pszErrInfo, nBufLen, NULL);
		break;
	case PWE_UNKNOWN_COLORSPACE:
		lstrcpynW(pszErrInfo, L"Unsupported output colorspace", nBufLen); break;
	case PWE_EXCEPTION:
		lstrcpynW(pszErrInfo, L"Exception occurred", nBufLen); break;
	case PWE_FILE_NOT_FOUND:
		lstrcpynW(pszErrInfo, L"File not found", nBufLen); break;
	case PWE_NOT_ENOUGH_MEMORY:
		lstrcpynW(pszErrInfo, L"Not enough memory", nBufLen); break;
	case PWE_INVALID_CONTEXT:
		lstrcpynW(pszErrInfo, L"Invalid context", nBufLen); break;
	case PWE_INVALID_PAGEDATA:
		lstrcpynW(pszErrInfo, L"Invalid pDisplayCreate->pImage->lParam", nBufLen); break;
	case PWE_UNKNOWN_FORMAT:
		lstrcpynW(pszErrInfo, L"Unsupported RAW format, codec is not available", nBufLen); break;
	case PWE_OLD_PICVIEW:
		lstrcpyn(pszErrInfo, L"Old PicView version, exiting", nBufLen); break;
	default:
		return FALSE;
	}
	return TRUE;
}

#define DllGetFunction(hModule, FunctionName) FunctionName = (FunctionName##_t)GetProcAddress(hModule, #FunctionName)
//#define MALLOC(n) HeapAlloc(GetProcessHeap(), 0, n)
#define CALLOC(n) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, n)
#define FREE(p) HeapFree(GetProcessHeap(), 0, p)
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

template <typename T>
inline void SafeRelease(T *&p)
{
	if (NULL != p)
	{
		p->Release();
		p = NULL;
	}
}


UINT GetBitsFromGUID(GUID pf, BOOL& rbAlpha)
{
	UINT nBPP = 0;
	#define CHKFMT(N,n,a) if (pf == N) { nBPP = n; rbAlpha = a; } else

	CHKFMT(GUID_WICPixelFormat1bppIndexed, 1, FALSE)
	CHKFMT(GUID_WICPixelFormat2bppIndexed, 2, FALSE)
	CHKFMT(GUID_WICPixelFormat4bppIndexed, 4, FALSE)
	CHKFMT(GUID_WICPixelFormat8bppIndexed, 8, FALSE)
	CHKFMT(GUID_WICPixelFormatBlackWhite, 1, FALSE)
	CHKFMT(GUID_WICPixelFormat2bppGray,   2, FALSE)
	CHKFMT(GUID_WICPixelFormat4bppGray,   4, FALSE)
	CHKFMT(GUID_WICPixelFormat8bppGray,   8, FALSE)
	CHKFMT(GUID_WICPixelFormat8bppAlpha, 8, TRUE)
	CHKFMT(GUID_WICPixelFormat16bppBGR555, 16, FALSE)
	CHKFMT(GUID_WICPixelFormat16bppBGR565, 16, FALSE)
	CHKFMT(GUID_WICPixelFormat16bppBGRA5551, 16, TRUE)
	CHKFMT(GUID_WICPixelFormat16bppGray,   16, FALSE)
	CHKFMT(GUID_WICPixelFormat24bppBGR, 24, FALSE)
	CHKFMT(GUID_WICPixelFormat24bppRGB, 24, FALSE)
	CHKFMT(GUID_WICPixelFormat32bppBGR,   32, FALSE)
	CHKFMT(GUID_WICPixelFormat32bppBGRA,  32, TRUE)
	CHKFMT(GUID_WICPixelFormat32bppPBGRA, 32, TRUE)
	CHKFMT(GUID_WICPixelFormat32bppGrayFloat,  32, FALSE)
	CHKFMT(GUID_WICPixelFormat32bppRGBA, 32, TRUE)
	CHKFMT(GUID_WICPixelFormat32bppPRGBA, 32, TRUE)
	CHKFMT(GUID_WICPixelFormat48bppRGB, 48, FALSE)
	CHKFMT(GUID_WICPixelFormat48bppBGR, 48, FALSE)
	CHKFMT(GUID_WICPixelFormat64bppRGBA,  64, TRUE)
	CHKFMT(GUID_WICPixelFormat64bppBGRA,  64, TRUE)
	CHKFMT(GUID_WICPixelFormat64bppPRGBA, 64, TRUE)
	CHKFMT(GUID_WICPixelFormat64bppPBGRA, 64, TRUE)
	CHKFMT(GUID_WICPixelFormat16bppGrayFixedPoint, 16, FALSE)
	CHKFMT(GUID_WICPixelFormat32bppBGR101010, 32, FALSE)
	CHKFMT(GUID_WICPixelFormat48bppRGBFixedPoint, 48, FALSE)
	CHKFMT(GUID_WICPixelFormat48bppBGRFixedPoint, 48, FALSE)
	CHKFMT(GUID_WICPixelFormat96bppRGBFixedPoint, 96, FALSE)
	CHKFMT(GUID_WICPixelFormat128bppRGBAFloat,  128, TRUE)
	CHKFMT(GUID_WICPixelFormat128bppPRGBAFloat, 128, TRUE)
	CHKFMT(GUID_WICPixelFormat128bppRGBFloat,   128, FALSE)
	CHKFMT(GUID_WICPixelFormat32bppCMYK, 32, FALSE)
	CHKFMT(GUID_WICPixelFormat64bppRGBAFixedPoint, 64, TRUE)
	CHKFMT(GUID_WICPixelFormat64bppBGRAFixedPoint, 64, TRUE)
	CHKFMT(GUID_WICPixelFormat64bppRGBFixedPoint, 64, FALSE)
	CHKFMT(GUID_WICPixelFormat128bppRGBAFixedPoint, 128, TRUE)
	CHKFMT(GUID_WICPixelFormat128bppRGBFixedPoint, 128, FALSE)
	CHKFMT(GUID_WICPixelFormat64bppRGBAHalf, 64, TRUE)
	CHKFMT(GUID_WICPixelFormat64bppRGBHalf, 64, FALSE)
	CHKFMT(GUID_WICPixelFormat48bppRGBHalf, 48, FALSE)
	CHKFMT(GUID_WICPixelFormat32bppRGBE, 32, FALSE)
	CHKFMT(GUID_WICPixelFormat16bppGrayHalf, 16, FALSE)
	CHKFMT(GUID_WICPixelFormat32bppGrayFixedPoint, 32, FALSE)
	CHKFMT(GUID_WICPixelFormat32bppRGBA1010102, 32, TRUE)
	CHKFMT(GUID_WICPixelFormat32bppRGBA1010102XR, 32, TRUE)
	CHKFMT(GUID_WICPixelFormat64bppCMYK, 64, FALSE)
	CHKFMT(GUID_WICPixelFormat24bpp3Channels, 24, FALSE)
	CHKFMT(GUID_WICPixelFormat32bpp4Channels, 32, FALSE)
	CHKFMT(GUID_WICPixelFormat40bpp5Channels, 40, FALSE)
	CHKFMT(GUID_WICPixelFormat48bpp6Channels, 48, FALSE)
	CHKFMT(GUID_WICPixelFormat56bpp7Channels, 56, FALSE)
	CHKFMT(GUID_WICPixelFormat64bpp8Channels, 64, FALSE)
	CHKFMT(GUID_WICPixelFormat48bpp3Channels, 48, FALSE)
	CHKFMT(GUID_WICPixelFormat64bpp4Channels, 64, FALSE)
	CHKFMT(GUID_WICPixelFormat80bpp5Channels, 80, FALSE)
	CHKFMT(GUID_WICPixelFormat96bpp6Channels, 96, FALSE)
	CHKFMT(GUID_WICPixelFormat112bpp7Channels, 112, FALSE)
	CHKFMT(GUID_WICPixelFormat128bpp8Channels, 128, FALSE)
	CHKFMT(GUID_WICPixelFormat40bppCMYKAlpha, 40, TRUE)
	CHKFMT(GUID_WICPixelFormat80bppCMYKAlpha, 80, TRUE)
	CHKFMT(GUID_WICPixelFormat32bpp3ChannelsAlpha, 32, TRUE)
	CHKFMT(GUID_WICPixelFormat40bpp4ChannelsAlpha, 40, TRUE)
	CHKFMT(GUID_WICPixelFormat48bpp5ChannelsAlpha, 48, TRUE)
	CHKFMT(GUID_WICPixelFormat56bpp6ChannelsAlpha, 56, TRUE)
	CHKFMT(GUID_WICPixelFormat64bpp7ChannelsAlpha, 64, TRUE)
	CHKFMT(GUID_WICPixelFormat72bpp8ChannelsAlpha, 72, TRUE)
	CHKFMT(GUID_WICPixelFormat64bpp3ChannelsAlpha, 64, TRUE)
	CHKFMT(GUID_WICPixelFormat80bpp4ChannelsAlpha, 80, TRUE)
	CHKFMT(GUID_WICPixelFormat96bpp5ChannelsAlpha, 96, TRUE)
	CHKFMT(GUID_WICPixelFormat112bpp6ChannelsAlpha, 112, TRUE)
	CHKFMT(GUID_WICPixelFormat128bpp7ChannelsAlpha, 128, TRUE)
	CHKFMT(GUID_WICPixelFormat144bpp8ChannelsAlpha, 144, TRUE)
	{ nBPP = 0; rbAlpha = FALSE; }
	return nBPP;
}


struct WICImage
{
	IWICImagingFactory *pIWICFactory;
	MStream *pStrm;
	HBITMAP m_hDIBBitmap;
	IWICBitmapDecoder *pDecoder;
	UINT nFrames;
	INT  nActiveFrame;
	WICPixelFormatGUID pf;
	UINT nBPP; BOOL bAlpha;
	UINT nWidth, nHeight;
	IWICBitmapSource *pOriginalBitmapSource;

	LPVOID pBitmapData;

	void Close()
	{
		SafeRelease(pStrm);
		SafeRelease(pDecoder);
		SafeRelease(pOriginalBitmapSource);
	};

	HRESULT SelectFrame(UINT nFrame)
	{
		HRESULT hr = S_OK;

		if (nFrame == nActiveFrame)
			return S_OK;

		IWICBitmapFrameDecode *pFrame = NULL;

		SafeRelease(pOriginalBitmapSource);

		hr = pDecoder->GetFrame(nFrame, &pFrame);
		if (SUCCEEDED(hr))
			nActiveFrame = nFrame;
		else
			wsprintf(gsLastErrFunc, L"SelectFrame(%i).GetFrame", nFrame);

		// Retrieve IWICBitmapSource from the frame
		if (SUCCEEDED(hr))
		{
			hr = pFrame->QueryInterface(
				IID_IWICBitmapSource, 
				reinterpret_cast<void **>(&pOriginalBitmapSource));
			if (FAILED(hr)) wsprintf(gsLastErrFunc, L"SelectFrame(%i).IWICBitmapSource", nFrame);
		}

		if (SUCCEEDED(hr))
		{
			hr = pOriginalBitmapSource->GetSize(&nWidth, &nHeight);
			if (FAILED(hr)) wsprintf(gsLastErrFunc, L"SelectFrame(%i).GetSize", nFrame);
		}

		bAlpha = FALSE;
		if (SUCCEEDED(hr))
		{
			hr = pOriginalBitmapSource->GetPixelFormat(&pf);
			if (SUCCEEDED(hr)) {
				nBPP = GetBitsFromGUID(pf, bAlpha);
			} else wsprintf(gsLastErrFunc, L"SelectFrame(%i).GetPixelFormat", nFrame);
		}

		SafeRelease(pFrame);

		if (FAILED(hr))
			nActiveFrame = -1;

		return hr;
	};
};




void __stdcall pvdReloadConfig2(void *pContext)
{
	PVDSettings set(gpszPluginKey);

	DWORD nDefault = DEFAULT_INTERPOLATION_MODE;
	set.GetParam(L"InterpolationMode", L"long;Interpolation quality (0..3)",
		REG_DWORD, &nDefault, &gnInterpolationMode, sizeof(gnInterpolationMode));
	if (gnInterpolationMode > 3) gnInterpolationMode = 3;
}


UINT32 __stdcall pvdInit2(pvdInitPlugin2* pInit)
{
	_ASSERTE(pInit->cbSize >= sizeof(pvdInitPlugin2));
	if (pInit->cbSize < sizeof(pvdInitPlugin2)) {
		pInit->nErrNumber = PWE_OLD_PICVIEW;
		return 0;
	}

	memset(&ip,0,sizeof(ip));
	memmove(&ip, pInit, min(sizeof(ip),pInit->cbSize));
	ghModule = ip.hModule;
	
	gpszPluginKey = pInit->pRegKey;
	pvdReloadConfig2(NULL);

	ghLastWin32Error = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	if (FAILED(ghLastWin32Error)) {
		lstrcpy(gsLastErrFunc, L"CoInitializeEx");
		pInit->nErrNumber = PWE_WIN32_ERROR;
		return 0;
	}

	// Create WIC factory
	IWICImagingFactory *pIWICFactory = NULL;
	ghLastWin32Error = CoCreateInstance(
		CLSID_WICImagingFactory,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS(&pIWICFactory)
		);
	if (FAILED(ghLastWin32Error) || !pIWICFactory) {
		lstrcpy(gsLastErrFunc, L"CoCreateInstance(WICImagingFactory)");
		pInit->nErrNumber = PWE_WIN32_ERROR;
		return 0;
	}

	pInit->pContext = pIWICFactory;

	return PVD_UNICODE_INTERFACE_VERSION;
}

void __stdcall pvdGetFormats2(void *pContext, pvdFormats2* pFormats)
{
	_ASSERTE(pFormats->cbSize >= sizeof(pvdFormats2));

	//pFormats->pSupported = L"BMP,GIF,ICO,JPG,JPE,JPEG,PNG,TIF,TIFF,TGA,TPIC,DNG,RAF,MRW,ORF,RW2,PEF,X3F,CR2,CRW,3PR,FFF,DCR,KDC,RAW,RWL,NEF,NRW,ARW,SR2,SRF";
	//pFormats->pSupported = L"HDP,TGA,TPIC,DNG,RAF,MRW,ORF,RW2,PEF,X3F,CR2,CRW,3PR,FFF,DCR,KDC,RAW,RWL,NEF,NRW,ARW,SR2,SRF";
	//RAF,3FR,FFF,DNG,GIF,PNG,TGA,TPIC,DJVU,DJV,SDJVU,SDJV,CR2,CRW,BMP,DIB,RLE,RW2,NEF,NRW,RAW,RWL,SR2,SRF,ARW,JPEG,JPE,JPG,JFIF,EXIF,MRW,WDP,PEF,DCR,KDC,TIFF,TIF,ICO,ICON,ORF,X3F
	//HDP,TGA,TPIC,DNG,RAF,MRW,ORF,RW2,PEF,X3F,CR2,CRW,3PR,FFF,DCR,KDC,RAW,RWL,NEF,NRW,ARW,SR2,SRF
	//pFormats->pIgnored = L"";

	/* 
	[HKEY_CLASSES_ROOT\CLSID\{7ED96837-96F0-4812-B211-F13C24117ED3}\Instance]

	[HKEY_CLASSES_ROOT\CLSID\{7ED96837-96F0-4812-B211-F13C24117ED3}\Instance\{1285CF5C-6BD6-4412-B23E-32EE4189AC7E}]
	"CLSID"="{1285CF5C-6BD6-4412-B23E-32EE4189AC7E}"
	"FriendlyName"="Fuji Raw Format Decoder (FastPictureViewer WIC Codec Pack)"

	[HKEY_CLASSES_ROOT\CLSID\{7ED96837-96F0-4812-B211-F13C24117ED3}\Instance\{33AB3A51-9CC1-4CA8-88F5-49BC9B970114}]
	"CLSID"="{33AB3A51-9CC1-4CA8-88F5-49BC9B970114}"
	"FriendlyName"="Hasselblad Raw Format Decoder (FastPictureViewer WIC Codec Pack)"

	[HKEY_CLASSES_ROOT\CLSID\{7ED96837-96F0-4812-B211-F13C24117ED3}\Instance\{35BE9A2F-B182-4bba-A609-4F9031BFE761}]
	"CLSID"="{35BE9A2F-B182-4bba-A609-4F9031BFE761}"
	"FriendlyName"="Adobe Dng Format Decoder (FastPictureViewer WIC Codec Pack)"

	[HKEY_CLASSES_ROOT\CLSID\{7ED96837-96F0-4812-B211-F13C24117ED3}\Instance\{381DDA3C-9CE9-4834-A23E-1F98F8FC52BE}]
	"CLSID"="{381DDA3C-9CE9-4834-A23E-1F98F8FC52BE}"

	[HKEY_CLASSES_ROOT\CLSID\{7ED96837-96F0-4812-B211-F13C24117ED3}\Instance\{389EA17B-5078-4CDE-B6EF-25C15175C751}]
	"CLSID"="{389EA17B-5078-4CDE-B6EF-25C15175C751}"

	[HKEY_CLASSES_ROOT\CLSID\{7ED96837-96F0-4812-B211-F13C24117ED3}\Instance\{3B84C2D7-708C-48ef-8ED7-0C5FC0F030C6}]
	"CLSID"="{3B84C2D7-708C-48ef-8ED7-0C5FC0F030C6}"
	"FriendlyName"="TGA Decoder (FastPictureViewer WIC Codec Pack)"

	[HKEY_CLASSES_ROOT\CLSID\{7ED96837-96F0-4812-B211-F13C24117ED3}\Instance\{4A5CA5D3-9330-4D0B-B3CA-F752F0E1F2C1}]
	"CLSID"="{4A5CA5D3-9330-4D0B-B3CA-F752F0E1F2C1}"
	"Friendly Name"="DjVu WIC Decoder"

	[HKEY_CLASSES_ROOT\CLSID\{7ED96837-96F0-4812-B211-F13C24117ED3}\Instance\{4C9966E0-7DAF-4670-A5A4-DE2AFF4CDC31}]
	"CLSID"="{4C9966E0-7DAF-4670-A5A4-DE2AFF4CDC31}"
	"FriendlyName"="Canon Raw Format Decoder (FastPictureViewer WIC Codec Pack)"

	[HKEY_CLASSES_ROOT\CLSID\{7ED96837-96F0-4812-B211-F13C24117ED3}\Instance\{6B462062-7CBF-400D-9FDB-813DD10F2778}]
	"CLSID"="{6B462062-7CBF-400D-9FDB-813DD10F2778}"

	[HKEY_CLASSES_ROOT\CLSID\{7ED96837-96F0-4812-B211-F13C24117ED3}\Instance\{730069ED-7411-487D-9629-0FED4B30A810}]
	"CLSID"="{730069ED-7411-487D-9629-0FED4B30A810}"
	"FriendlyName"="Panasonic Raw Format Decoder (FastPictureViewer WIC Codec Pack)"

	[HKEY_CLASSES_ROOT\CLSID\{7ED96837-96F0-4812-B211-F13C24117ED3}\Instance\{798CB867-4677-420d-8F1B-C2E12A882180}]
	"CLSID"="{798CB867-4677-420d-8F1B-C2E12A882180}"
	"FriendlyName"="Nikon Raw Format Decoder (FastPictureViewer WIC Codec Pack)"

	[HKEY_CLASSES_ROOT\CLSID\{7ED96837-96F0-4812-B211-F13C24117ED3}\Instance\{7E865BF5-4DEA-495C-B690-296869B83753}]
	"CLSID"="{7E865BF5-4DEA-495C-B690-296869B83753}"
	"FriendlyName"="Leica Raw Format Decoder (FastPictureViewer WIC Codec Pack)"

	[HKEY_CLASSES_ROOT\CLSID\{7ED96837-96F0-4812-B211-F13C24117ED3}\Instance\{82E3DCAD-555E-4FA3-938D-74C0AFFE0C67}]
	"CLSID"="{82E3DCAD-555E-4FA3-938D-74C0AFFE0C67}"
	"FriendlyName"="Sony Raw Format Decoder (FastPictureViewer WIC Codec Pack)"

	[HKEY_CLASSES_ROOT\CLSID\{7ED96837-96F0-4812-B211-F13C24117ED3}\Instance\{9456A480-E88B-43EA-9E73-0B2D9B71B1CA}]
	"CLSID"="{9456A480-E88B-43EA-9E73-0B2D9B71B1CA}"

	[HKEY_CLASSES_ROOT\CLSID\{7ED96837-96F0-4812-B211-F13C24117ED3}\Instance\{94E0E6A3-1590-42EE-8D28-ACCA2CE955D8}]
	"CLSID"="{94E0E6A3-1590-42EE-8D28-ACCA2CE955D8}"
	"FriendlyName"="Minolta Raw Format Decoder (FastPictureViewer WIC Codec Pack)"

	[HKEY_CLASSES_ROOT\CLSID\{7ED96837-96F0-4812-B211-F13C24117ED3}\Instance\{A26CEC36-234C-4950-AE16-E34AACE71D0D}]
	"CLSID"="{A26CEC36-234C-4950-AE16-E34AACE71D0D}"

	[HKEY_CLASSES_ROOT\CLSID\{7ED96837-96F0-4812-B211-F13C24117ED3}\Instance\{A4E39C16-C86F-451B-8273-471BF5666870}]
	"CLSID"="{A4E39C16-C86F-451B-8273-471BF5666870}"
	"FriendlyName"="Pentax Raw Format Decoder (FastPictureViewer WIC Codec Pack)"

	[HKEY_CLASSES_ROOT\CLSID\{7ED96837-96F0-4812-B211-F13C24117ED3}\Instance\{ADE56A16-A02B-4020-B537-914ADC2E07B1}]
	"CLSID"="{ADE56A16-A02B-4020-B537-914ADC2E07B1}"
	"FriendlyName"="Kodak Raw Format Decoder (FastPictureViewer WIC Codec Pack)"

	[HKEY_CLASSES_ROOT\CLSID\{7ED96837-96F0-4812-B211-F13C24117ED3}\Instance\{B54E85D9-FE23-499F-8B88-6ACEA713752B}]
	"CLSID"="{B54E85D9-FE23-499F-8B88-6ACEA713752B}"

	[HKEY_CLASSES_ROOT\CLSID\{7ED96837-96F0-4812-B211-F13C24117ED3}\Instance\{C61BFCDF-2E0F-4AAD-A8D7-E06BAFEBCDFE}]
	"CLSID"="{C61BFCDF-2E0F-4AAD-A8D7-E06BAFEBCDFE}"

	[HKEY_CLASSES_ROOT\CLSID\{7ED96837-96F0-4812-B211-F13C24117ED3}\Instance\{DCEFAA85-F357-4859-963F-F1BB14F0829C}]
	"CLSID"="{DCEFAA85-F357-4859-963F-F1BB14F0829C}"
	"FriendlyName"="Olympus Raw Format Decoder (FastPictureViewer WIC Codec Pack)"

	[HKEY_CLASSES_ROOT\CLSID\{7ED96837-96F0-4812-B211-F13C24117ED3}\Instance\{DF13C2F9-509F-4088-A39F-2D4B288B95E1}]
	"CLSID"="{DF13C2F9-509F-4088-A39F-2D4B288B95E1}"
	"FriendlyName"="Sigma X3F Format Decoder (FastPictureViewer WIC Codec Pack)"

	*/

	/*
	[HKEY_CLASSES_ROOT\CLSID\{1285CF5C-6BD6-4412-B23E-32EE4189AC7E}]
	"FileExtensions"=".raf"
	*/

	wchar_t szExt[MAX_PATH], szName[MAX_PATH], szClsId[64];
	DWORD dwIndex = 0, nAllLen = 0, nLen = 0, nMaxLen = 4096, nData;
	HKEY hk = 0, hkr = 0;
	if (!gsExtensions) {
		gsExtensions = (wchar_t*)CALLOC((nMaxLen+1)*2);
		_ASSERTE(gsExtensions);
		if (!gsExtensions) return;
	} else {
		gsExtensions[0] = 0;
	}
	// Список расширений, которые рендерятся, но не находятся через реестр
	lstrcpy(gsExtensions, L",HDP,"); nAllLen = lstrlen(gsExtensions);
	//lstrcpy(gsExtensions, L",HDP,JXR,"); nAllLen = lstrlen(gsExtensions);

	if (!RegOpenKeyEx(HKEY_CLASSES_ROOT, L"CLSID\\{7ED96837-96F0-4812-B211-F13C24117ED3}\\Instance", 0, KEY_READ, &hkr))
	{
		while (!RegEnumKeyEx(hkr, dwIndex++, szClsId, &(nLen = sizeof(szClsId)), 0, 0, 0, 0))
		{
			lstrcpy(szName, L"CLSID\\");
			lstrcat(szName, szClsId);

			if (RegOpenKeyEx(HKEY_CLASSES_ROOT, szName, 0, KEY_READ, &hk))
				continue;

			if (RegQueryValueEx(hk, L"FileExtensions", 0, 0, (LPBYTE)szExt, &(nData = sizeof(szExt)-4)))
			{
				RegCloseKey(hk); continue;
			}
			RegCloseKey(hk);
			nLen = lstrlen(szExt);
			if (!nLen) continue;

			if (nMaxLen >= (nAllLen + nLen + 1)) {
				szExt[nLen] = 0; szExt[nLen+1] = 0;
				CharUpperBuff(szExt, nLen);

				wchar_t* pszExt = szExt;
				while (*pszExt == L'.') pszExt++;
				while (*pszExt)
				{
					BOOL lbExists = FALSE;

					wchar_t* pszComma = wcschr(pszExt, L',');
					if (!pszComma) pszComma = pszExt+lstrlen(pszExt);
					*pszComma = 0;
					szName[0] = L','; lstrcpy(szName+1, pszExt); lstrcat(szName, L",");
					lbExists = (wcsstr(gsExtensions, szName) != NULL);

					if (!lbExists)
					{
						lstrcpy(gsExtensions+nAllLen, pszExt);
						nAllLen += lstrlen(pszExt);
						gsExtensions[nAllLen++] = L',';
						gsExtensions[nAllLen] = 0;
					}

					pszExt = pszComma + 1;
					while (*pszExt == L'.') pszExt++;
				}
			}
		}
	}

	gsExtensions[nAllLen-1] = 0;

	if (ip.SortExtensions)
		ip.SortExtensions(gsExtensions+1);

	pFormats->pActive = gsExtensions+1;
	pFormats->pInactive = L"";
	pFormats->pForbidden = L"GIF";

}

void __stdcall pvdExit2(void *pContext)
{
	if (pContext) {
		IWICImagingFactory *pIWICFactory = (IWICImagingFactory*)pContext;
		SafeRelease(pIWICFactory);
	}
	if (gsExtensions) { FREE(gsExtensions); gsExtensions = NULL; }
}

void __stdcall pvdPluginInfo2(pvdInfoPlugin2 *pPluginInfo)
{
	_ASSERTE(pPluginInfo->cbSize >= sizeof(pvdInfoPlugin2));
	pPluginInfo->Priority = 0x800;
	pPluginInfo->pName = L"WIC";
	pPluginInfo->pVersion = GetVersion(pPluginInfo->hModule);
	pPluginInfo->pComments = L"Copyright © 2009 Maximus5";
	// UPSCALE WIC не умеет - на изображении появляются артефакты
	pPluginInfo->Flags = PVD_IP_DECODE|PVD_IP_CANDESCALE;
}


BOOL __stdcall pvdFileOpen2(void *pContext, const wchar_t *pFileName, INT64 lFileSize, const BYTE *pBuf, UINT32 lBuf, pvdInfoImage2 *pImageInfo)
{
	_ASSERTE(pImageInfo->cbSize >= sizeof(pvdInfoImage2));
	if (!pContext) {
		pImageInfo->nErrNumber = PWE_INVALID_CONTEXT;
		return FALSE;
	}

	pImageInfo->nErrNumber = 0;

	WICImage *pImage = (WICImage*)CALLOC(sizeof(WICImage));
	if (!pImage) {
		pImageInfo->nErrNumber = PWE_NOT_ENOUGH_MEMORY;
		return FALSE;
	}
	pImage->nActiveFrame = -1;
	if (!(pImage->pStrm = new MStream())) {
		pImageInfo->nErrNumber = PWE_NOT_ENOUGH_MEMORY;
		FREE(pImage);
		return FALSE;
	}
	ghLastWin32Error = pImage->pStrm->Write(pBuf, lBuf, NULL);
	if (FAILED(ghLastWin32Error)) {
		lstrcpy(gsLastErrFunc, L"pImage->pStrm->Write()");
		delete pImage->pStrm;
		FREE(pImage);
		pImageInfo->nErrNumber = PWE_WIN32_ERROR;
		return FALSE;
	}
	pImage->pStrm->Seek(0,0,0);

	IWICImagingFactory *pIWICFactory = (IWICImagingFactory*)pContext;
	pImage->pIWICFactory = pIWICFactory;

	
	

	// Create a decoder
	ghLastWin32Error = pIWICFactory->CreateDecoderFromStream(
		pImage->pStrm,                   // Image to be decoded
		NULL,                            // Do not prefer a particular vendor
		WICDecodeMetadataCacheOnDemand,  // Cache metadata when needed
		&pImage->pDecoder                // Pointer to the decoder
		);
	if (FAILED(ghLastWin32Error)) lstrcpy(gsLastErrFunc, L"CreateDecoderFromStream");
	//ghLastWin32Error = pIWICFactory->CreateDecoderFromFilename(
	//	pFileName,                      // Image to be decoded
	//	NULL,                            // Do not prefer a particular vendor
	//	GENERIC_READ,                    // Desired read access to the file
	//	WICDecodeMetadataCacheOnDemand,  // Cache metadata when needed
	//	&pImage->pDecoder                        // Pointer to the decoder
	//	);

	if (SUCCEEDED(ghLastWin32Error))
	{
		ghLastWin32Error = pImage->pDecoder->GetFrameCount(&pImage->nFrames);
		if (FAILED(ghLastWin32Error)) lstrcpy(gsLastErrFunc, L"GetFrameCount");
		// Если вернул 0 - скорее всего формат нераспознан
		if (pImage->nFrames == 0) {
			// для NEF файлов (по крайней мере) возвращает 0
			ghLastWin32Error = E_INVALIDARG;
			pImageInfo->nErrNumber = PWE_UNKNOWN_FORMAT;
		}
	}

	if (SUCCEEDED(ghLastWin32Error)) {
		ghLastWin32Error = pImage->SelectFrame(0);
		pImageInfo->nErrNumber = PWE_WIN32_ERROR;
		//if (FAILED(ghLastWin32Error)) lstrcpy(gsLastErrFunc, L"SelectFrame(0)");
	}

	TODO("Получить GetMetadataQueryReader для EXIF");

	if (SUCCEEDED(ghLastWin32Error)) {
		pImageInfo->pImageContext = pImage;
		pImageInfo->nPages = pImage->nFrames;
		pImageInfo->Flags = PVD_IIF_CAN_REFINE;
		if (pImage->nFrames > 1) {
			const wchar_t* pszExt = wcsrchr(pFileName, L'.');
			if (pszExt) {
				if (lstrcmpi(pszExt, L".gif")==0)
					pImageInfo->Flags |= PVD_IIF_ANIMATED;
			}
		}
		pImageInfo->pFormatName = NULL;
		pImageInfo->pCompression = NULL;
		pImageInfo->pComments = NULL;

		return TRUE;

	}

	pImage->Close();
	FREE(pImage);

	if (!pImageInfo->nErrNumber)
		pImageInfo->nErrNumber = PWE_WIN32_ERROR;
	return FALSE;
}

BOOL __stdcall pvdPageInfo2(void *pContext, void *pImageContext, pvdInfoPage2 *pPageInfo)
{
	if (!pImageContext) {
		pPageInfo->nErrNumber = PWE_INVALID_CONTEXT;
		return FALSE;
	}
	_ASSERTE(pPageInfo->cbSize >= sizeof(pvdInfoPage2));

	WICImage *pImage = (WICImage*)pImageContext;

	if (pImage->nActiveFrame != pPageInfo->iPage) {
		ghLastWin32Error = pImage->SelectFrame(pPageInfo->iPage);
		if (FAILED(ghLastWin32Error)) {
			//lstrcpy(gsLastErrFunc, L"SelectFrame(pPageInfo->iPage)");
			pPageInfo->nErrNumber = PWE_WIN32_ERROR;
			return FALSE;
		}
	}

	pPageInfo->lWidth = pImage->nWidth;
	pPageInfo->lHeight = pImage->nHeight;
	pPageInfo->nBPP = pImage->nBPP;
	return TRUE;
}

BOOL __stdcall pvdPageDecode2(void *pContext, void *pImageContext, pvdInfoDecode2 *pDecodeInfo, 
							  pvdDecodeCallback2 DecodeCallback, void *pDecodeCallbackContext)
{
	if (!pImageContext) {
		pDecodeInfo->nErrNumber = PWE_INVALID_CONTEXT;
		return FALSE;
	}

	_ASSERTE(pDecodeInfo->cbSize >= sizeof(pvdInfoDecode2));

	WICImage* pImage = (WICImage*)pImageContext;
	ghLastWin32Error = pImage->SelectFrame(pDecodeInfo->iPage);
	if (FAILED(ghLastWin32Error)) {
		//lstrcpy(gsLastErrFunc, L"pImage->SelectFrame(pDecodeInfo->iPage)");
		pDecodeInfo->nErrNumber = PWE_WIN32_ERROR;
		return FALSE;
	}

	BOOL lbRc = FALSE;

	HRESULT hr = S_OK;

	UINT lDecodeWidth = pImage->nWidth;
	UINT lDecodeHeight = pImage->nHeight;

	IWICBitmapSource *pToRenderBitmapSource = pImage->pOriginalBitmapSource;
	
	if ((pImage->pf != GUID_WICPixelFormat32bppBGR && pImage->pf != GUID_WICPixelFormat32bppBGRA)
		|| ((pDecodeInfo->lWidth && pDecodeInfo->lHeight)
			&& (pDecodeInfo->lWidth != lDecodeWidth || pDecodeInfo->lHeight != lDecodeHeight))
		)
	{
		IWICFormatConverter *pConverter = NULL;
		IWICBitmapScaler *pScaler = NULL;

		if (pDecodeInfo->lWidth && pDecodeInfo->lHeight) {
			// Descale/Upscale
			hr = ghLastWin32Error = pImage->pIWICFactory->CreateBitmapScaler(&pScaler);
			if (FAILED(ghLastWin32Error)) lstrcpy(gsLastErrFunc, L"CreateBitmapScaler");
			if (SUCCEEDED(hr)) {
			
			    //WICBitmapInterpolationModeNearestNeighbor	= 0,
				//WICBitmapInterpolationModeLinear	= 0x1,
				//WICBitmapInterpolationModeCubic	= 0x2,
				//WICBitmapInterpolationModeFant	= 0x3,
			
				hr = ghLastWin32Error = pScaler->Initialize(
					pImage->pOriginalBitmapSource, 
					pDecodeInfo->lWidth, pDecodeInfo->lHeight, 
					(WICBitmapInterpolationMode)gnInterpolationMode
					);
				if (FAILED(ghLastWin32Error)) lstrcpy(gsLastErrFunc, L"pScaler->Initialize()");
				TODO("Interpolation mode можно вынести в настройку WIC.pvd");
				if (SUCCEEDED(hr)) {
					lDecodeWidth = pDecodeInfo->lWidth;
					lDecodeHeight = pDecodeInfo->lHeight;
				} else {
					SafeRelease(pScaler);
				}
			}
		}

		hr = ghLastWin32Error = pImage->pIWICFactory->CreateFormatConverter(&pConverter);
		if (FAILED(ghLastWin32Error)) lstrcpy(gsLastErrFunc, L"CreateFormatConverter");

		// Format convert to 32bppBGR
		if (SUCCEEDED(ghLastWin32Error))
		{
			WICDouble dbl = {0,0};
			
			hr = ghLastWin32Error = pConverter->Initialize(
				pScaler ? pScaler : pImage->pOriginalBitmapSource,   // Input bitmap to convert
				pImage->bAlpha ? GUID_WICPixelFormat32bppBGRA : GUID_WICPixelFormat32bppBGR, // Destination pixel format
				WICBitmapDitherTypeNone,         // Specified dither patterm
				NULL,                            // Specify a particular palette 
				dbl /* 0.f*/,                    // Alpha threshold
				WICBitmapPaletteTypeCustom       // Palette translation type
				);
			if (FAILED(ghLastWin32Error)) lstrcpy(gsLastErrFunc, L"pConverter->Initialize()");

			// Store the converted bitmap as ppToRenderBitmapSource 
			if (SUCCEEDED(hr))
			{
				hr = ghLastWin32Error = pConverter->QueryInterface(IID_PPV_ARGS(&pToRenderBitmapSource));
				if (FAILED(ghLastWin32Error)) lstrcpy(gsLastErrFunc, L"QueryInterface(pToRenderBitmapSource)");
			}
		}

		SafeRelease(pScaler);
		SafeRelease(pConverter);

		//if (SUCCEEDED(ghLastWin32Error)) {
		//	TODO("Это нехорошо при Descale - оставлять исходник нужно");
		//	SafeRelease(pImage->pOriginalBitmapSource);
		//	pImage->pOriginalBitmapSource = pToRenderBitmapSource;
		//	pToRenderBitmapSource = NULL;
		//	pImage->pf = GUID_WICPixelFormat32bppBGR;
		//}
	}

	if (SUCCEEDED(hr)) {
		pDecodeInfo->lWidth = lDecodeWidth;
		pDecodeInfo->lHeight = lDecodeHeight;
		// lSrcWidth & lSrcHeight - не трогаем, т.к. реальные размеры изображения не менялись
		pDecodeInfo->nBPP = 32;
		pDecodeInfo->Flags = pImage->bAlpha ? PVD_IDF_ALPHA : 0;
		pDecodeInfo->ColorModel = pImage->bAlpha ? PVD_CM_BGRA : PVD_CM_BGR;
		pDecodeInfo->lImagePitch = lDecodeWidth * 4;
		pDecodeInfo->pImage = (LPBYTE)CALLOC(pDecodeInfo->lImagePitch * lDecodeHeight);
		if (!pDecodeInfo->pImage) {
			pDecodeInfo->nErrNumber = PWE_NOT_ENOUGH_MEMORY;
			return FALSE;
		} else {
			TODO("Попробовать перейти на IWICBitmap::Lock");
			hr = ghLastWin32Error = pToRenderBitmapSource->CopyPixels(
				NULL,
				pDecodeInfo->lImagePitch,
				pDecodeInfo->lImagePitch * pDecodeInfo->lHeight, 
				pDecodeInfo->pImage);
			if (FAILED(hr)) {
				lstrcpy(gsLastErrFunc, L"CopyPixels()");
				FREE(pDecodeInfo->pImage);
				pDecodeInfo->nErrNumber = PWE_WIN32_ERROR;
				return FALSE;
			}
			lbRc = TRUE;
		}
	}

	if (pToRenderBitmapSource != pImage->pOriginalBitmapSource)
		SafeRelease(pToRenderBitmapSource);

	return lbRc;
}

void __stdcall pvdPageFree2(void *pContext, void *pImageContext, pvdInfoDecode2 *pDecodeInfo)
{
	_ASSERTE(pDecodeInfo->cbSize >= sizeof(pvdInfoDecode2));
	if (pDecodeInfo->pImage)
		FREE(pDecodeInfo->pImage);
}

void __stdcall pvdFileClose2(void *pContext, void *pImageContext)
{
	if (pImageContext) {
		WICImage *pImage = (WICImage*)pImageContext;
		pImage->Close();
		FREE(pImage);
	}
}
