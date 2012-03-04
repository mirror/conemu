
/**************************************************************************
Copyright (c) 2009 Skakov Pavel
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

#include "PictureView.h"
#include "Image.h"
#include "PVDManager.h"
#include "PictureView_Lang.h"
#include "headers/farcolor.hpp"
#include "DisplayHandle.h"
#include "DecoderHandle.h"
#include "ImageInfo.h"

#define PrintString(pStr) { \
	int nMax = (int)(nBufLen-1-(pd - pTitleText)); \
	if (nMax > 1) { \
		lstrcpyn(pd, (pStr), nMax+1); pd += wcslen(pd); \
	}}
#define PrintNumber(Number) {g_FSF.sprintf(pd, L"%i", (Number)); pd += wcslen(pd);}

wchar_t g_TitleTemplate[0x200];
wchar_t g_QViewTemplate1[0x200], g_QViewTemplate2[0x200], g_QViewTemplate3[0x200];
//const wchar_t g_DefaultTitleTemplate[] = L"\\{S* \\}\\N  \\{t<\\D>\\}  \\W x \\H x \\Bb\\{T   \\Z%\\{I   \\I\\}\\{P   [\\P]\\}   -   PictureView by \\D\\{F : \\F\\}\\{C   [\\C]\\}\\{M   {\\M}\\}   \\Tms\\}";
const wchar_t g_DefaultTitleTemplate[] = L"\\{S* \\}\\NL12\\{t  <\\D>\\}\\{w  \\W•\\H•\\Bb\\}\\{T [\\W0•\\H0]   \\Z%\\{I   \\I\\}\\{P   [\\P]\\}   (\\D\\{F:\\F\\}\\{C:[\\C]\\}:\\R)  \\E=\\Tms\\} - PicView2";
const wchar_t g_DefaultQViewTemplate1[] = L"$ PicView2: \\NL24 ";
const wchar_t g_DefaultQViewTemplate2[] = L" \\W•\\H•\\Bb  \\Z%\\{P  [\\P]\\}  (\\D\\{F:\\F\\}\\{C:[\\C]\\})";
const wchar_t g_DefaultQViewTemplate3[] = L"#\\{T\\E=\\Tms\\}";

LPCSTR szTemplatePrint = "TemplatePrint";

void TemplatePrint(bool bDecoding, wchar_t *pTitleText, UINT nBufLen, const wchar_t* pTemplate)
{
	wchar_t *pd = pTitleText;
	wchar_t sTemp[128];
	const wchar_t *ps = pTemplate;
	_ASSERTE(nBufLen > 0x200);
	UINT nBorder = nBufLen - 0x100;

	CImage* pImage = NULL;
	CDecoderHandle* pFile = NULL;
	CDisplayHandlePtr rDraw;

	const PicViewItem* pItem = g_Panel.GetItem();
	if (pItem && pItem->pImage)
	{
		pImage = pItem->pImage;
		pImage->AddRef(szTemplatePrint);

		pFile = pImage->GetDecoderHandle(szTemplatePrint);
		pImage->GetDrawHandle(rDraw);
	}

    while (*ps)
	{
		if (*ps++ != '\\')
			*pd++ = ps[-1];
		else
		{
			switch (*ps++)
			{
				case 0:	ps--; break;
				case '\\': *pd++ = '\\'; break;
				case '{':
				{
					bool bSkip = true;
					// Маленькие буквы задают отрицание условия аналогичной заглавной буквы. ~0x20 -> UpperCase
					switch (*ps & ~0x20)
					{
						case 0: bSkip = false; ps--; break;
						case 'C': bSkip = !pImage || !*pImage->Info.Compression; break;
						case 'F': bSkip = !pImage || !*pImage->Info.FormatName; break;
						case 'I': bSkip = bDecoding || (g_Panel.GetReadyItemsCount() <= 1); break;
						//case 'I': bSkip = bDecoding || g_Plugin.FlagsWork & FW_JUMP_DISABLED || !g_Plugin.nPanelItems; break;
						case 'M': bSkip = !pImage || !*pImage->Info.Comments; break;
						case 'P': bSkip = !pImage || (pImage->Info.nPages <= 1); break;
						case 'T': bSkip = bDecoding; break;
						case 'S': bSkip = !g_Panel.IsFileMarked(); break;
						case 'W': bSkip = !pImage || (pImage->Info.lWidth <= 0); break;
					}
					// Проверка положительного/отрицательного условия
					if ((bool)(*ps++ & 0x20) ^ bSkip)
					{
						for (uint iLevel = 1; iLevel;)
						{
							for (; *ps && !(*ps == '\\' && (ps[1] == '}' || ps[1] == '{' || ps[1] == '\\')); ps++);
							if (!*ps)
								break;
							if (ps[1] == '}')
								iLevel--;
							else if (ps[1] == '{')
							{
								iLevel++;
								if (ps[2])
									ps++;
							}
							ps += 2;
						}
					}
					break;
				}
				case 'B':
				{
					if (!pImage)
					{
						PrintNumber(0);
					}
					else if (*ps == L'0')
					{
						PrintNumber(pImage->Info.nDecodedBPP); ps++;
					}
					else
					{
						PrintNumber(pImage->Info.nBPP);
					}
					break;
				}
				case 'C': PrintString(pImage ? pImage->Info.Compression : L""); break;
				case 'D': PrintString(pImage ? pImage->Info.DecoderName : L""); break;
				case 'F': PrintString(pImage ? pImage->Info.FormatName  : L""); break;
				case 'H':
				{
					if (!pImage)
					{
						PrintString(L"???")
					}
					else if (*ps == L'0')
					{
						PrintNumber(pImage->Info.lDecodedHeight); ps++;
					}
					else
					{
						PrintNumber(pImage->Info.lHeight);
					}
					break;
				}
				case 'I':
					//if (bDecoding || g_Plugin.FlagsWork & FW_JUMP_DISABLED || !g_Plugin.nPanelItems)
					if (bDecoding || (g_Panel.GetReadyItemsCount() <= 1))
					{
						*(u32*)pd = '?/?';
						pd += 3;
					}
					else
					{
						TODO("Заменить iPanelItemRaw на номер в фильтрованном списке");
						//PrintNumber(g_Plugin.Image[0]->PanelItemRaw() - g_Plugin.nPanelFolders + 1);
						PrintNumber(pItem ? pItem->nDisplayItemIndex : 0);
						*pd++ = '/';
						//TODO("Заменить количество на фильтрованный список");
						//PrintNumber(g_Plugin.nPanelItems-g_Plugin.nPanelFolders);
						PrintNumber(g_Panel.GetReadyItemsCount());
					}
					break;
				case 'M': PrintString(pImage ? pImage->Info.Comments : L""); break;
				case 'N':
				{
					const wchar_t *p = pImage->pszFileNameOnly;
					if (!p) p = L"???";
					if (*ps == L'L') // Ограничение длины имени файла
					{
						ps++;
						wchar_t* pEnd = NULL;
						long nLen = wcstol(ps, &pEnd, 10);
						if (pEnd) ps = pEnd; // Переместить "курсор" на первый символ после числа
						if (nLen > 127) nLen = 127; else
						if (nLen < 12) nLen = 12;
						if (lstrlen(p) > nLen) { // Если текущая длина имени файла превышает - отрезаем
							const wchar_t* pszExt = wcsrchr(p, L'.');
							if (!pszExt) { // Если расширения нет вообще - все просто
								lstrcpyn(sTemp, p, nLen);
								sTemp[nLen-1] = L'…'; sTemp[nLen] = 0;
							} else { // Иначе нужно учесть длину расширения
								pszExt ++; // точку отрежем, вместо нее идет троеточие
								int nExtLen = lstrlen(pszExt); // с точкой
								if (nExtLen > (nLen-2)) { // слишком длинное расширение - игнорируем
									lstrcpyn(sTemp, p, nLen);
									sTemp[nLen-1] = L'…'; sTemp[nLen] = 0;
								} else {
									nLen -= nExtLen;
									lstrcpyn(sTemp, p, nLen);
									sTemp[nLen-1] = L'…'; sTemp[nLen] = 0;
									lstrcat(sTemp, pszExt);
								}
							}
							p = sTemp;
						}
					}
					PrintString(p);
					break;
				}
				case 'P':
				{
					if (pFile && pFile->Decoder()) // значит было декодировано (хотя бы раз)
					{
						if (pImage->Info.Animation != 1 /*&& pImage->Decoder*/)
						{
							PrintNumber(pImage->Info.nPage + 1);
							*pd++ = '/';
						}
						else if (pImage->Info.nPages > 1 /*&& !pImage->Decoder*/)
							*pd++ = '~';
						PrintNumber(pImage->Info.nPages);
					}
					else
					{
						PrintString(L"???");
					}
					break;
				}
				case 'Q':
				{
					if (pImage)
					{
						const wchar_t *pszFull = (const wchar_t*)pImage->FileName;
						CUnicodeFileName::SkipPrefix(&pszFull);
						const wchar_t *p = g_FSF.PointToName(pszFull);
						memcpy(pd, pszFull, (p - pszFull)*sizeof(wchar_t));
						pd += p - pszFull;
					} else {
						PrintString(L"???");
					}
					break;
				}
				case 'R':
				{
					if (rDraw.IsValid() && rDraw->Display() && rDraw->Display()->pPlugin)
					{
						PrintString(rDraw->Display()->pPlugin->pName);
					}
					else
					{
						PrintString(L"???");
					}
					break;
				}
				case 'T':
					if (bDecoding || !pImage)
						*pd++ = '?';
					else
						PrintNumber(pImage->Info.lOpenTime);
					break;
				case 'E':
				{
					if (bDecoding || !pImage) *pd++ = '?'; else {
						wsprintf(sTemp, L"%i+%i+%i+%i",
							pImage->Info.lTimeOpen,
							pImage->Info.lTimeDecode,
							pImage->Info.lTimeTransfer,
							pImage->Info.lTimePaint);
						PrintString(sTemp);
					}
					break;
				}
				case 'W':
				{
					if (!pImage) {
						PrintString(L"???");
					} else if (*ps == L'0') {
						PrintNumber(pImage->Info.lDecodedWidth); ps++;
					} else {
						PrintNumber(pImage->Info.lWidth);
					}
					break;
				}
				case 'Z':
				{
					if (bDecoding)
						*pd++ = '?';
					else
						PrintNumber(MulDivU32(g_Plugin.AbsoluteZoom ? g_Plugin.AbsoluteZoom : g_Plugin.Zoom, 100, 0x10000));
					break;
				}
			}
			if (pd - pTitleText > 0xD00)
				break;
		}
	}
	*pd = 0;

	SafeRelease(pFile,szTemplatePrint);
	//SafeRelease(pDraw);
	SafeRelease(pImage,szTemplatePrint);
}

void TitleRepaint(bool bDecoding)
{
	_ASSERTE(GetCurrentThreadId() == gnMainThreadId);

	wchar_t pTitleText[0xF00];

	TemplatePrint(bDecoding, pTitleText, sizeofarray(pTitleText), g_TitleTemplate);

	SetConsoleTitleW(pTitleText);
}

void QViewRepaint(bool bDecoding)
{
	_ASSERTE(GetCurrentThreadId() == gnMainThreadId);

	wchar_t pTitleText[0xF00]; // По хорошему должна быть динамической
	//const вызывает инициализацию переменных ДО входа в функцию (по возможности). что нам не подходит - хочется видеть последовательность действий!
	#ifdef FAR_UNICODE
	FarColor clr[3];
	FarColor* PanelTextColor = &(clr[0]);
	FarColor* PanelFrameColor = &(clr[1]);
	FarColor* PanelSelectedColor = &(clr[2]);
	g_StartupInfo.AdvControl(PluginNumber, ACTL_GETCOLOR, COL_PANELTEXT, PanelTextColor);
	g_StartupInfo.AdvControl(PluginNumber, ACTL_GETCOLOR, COL_PANELBOX, PanelFrameColor);
	g_StartupInfo.AdvControl(PluginNumber, ACTL_GETCOLOR, COL_PANELSELECTEDINFO, PanelSelectedColor);
	#else
	int PanelTextColor = g_StartupInfo.AdvControl(PluginNumber, ACTL_GETCOLOR, (void*)COL_PANELTEXT);
	int PanelFrameColor = g_StartupInfo.AdvControl(PluginNumber, ACTL_GETCOLOR, (void*)COL_PANELBOX);
	int PanelSelectedColor = g_StartupInfo.AdvControl(PluginNumber, ACTL_GETCOLOR, (void*)COL_PANELSELECTEDINFO);
	#endif
	uint lPanel = Min<uint>(g_Plugin.ViewPanelT.right - g_Plugin.ViewPanelT.left - 1, sizeofarray(pTitleText) - 1);
	uint nLen = 0;

	wmemset(pTitleText, 0x2550, lPanel); // Двойная горизонтальная черта
	pTitleText[lPanel] = 0;
	// Нижняя строка (там остается текст старой панели)
	g_StartupInfo.Text(g_Plugin.ViewPanelT.left + 1, g_Plugin.ViewPanelT.bottom, PanelFrameColor, pTitleText);
	// Верхняя строка (там остается текст старой панели)
	if (lPanel>5) pTitleText[lPanel-5] = 0; // Часы оставить
	g_StartupInfo.Text(g_Plugin.ViewPanelT.left + 1, g_Plugin.ViewPanelT.top, PanelFrameColor, pTitleText);
	LPCTSTR pszQTitle = GetMsg(MIQViewPanelTitle);
	nLen = lstrlen(pszQTitle);
	int X = (g_Plugin.ViewPanelT.right + g_Plugin.ViewPanelT.left + 1 - nLen) / 2;
	g_StartupInfo.Text(X, g_Plugin.ViewPanelT.top, PanelTextColor, pszQTitle);

	wmemset(pTitleText, 0x2500, lPanel); // горизонтальная черта - забить место, где фар пишет название формата из реестра
	pTitleText[lPanel] = 0;
	g_StartupInfo.Text(g_Plugin.ViewPanelT.left + 1, g_Plugin.ViewPanelT.bottom - 2, PanelFrameColor, pTitleText);

	wmemset(pTitleText, L' ', lPanel); // Пред-очистка информационного поля
	pTitleText[lPanel] = 0;
	g_StartupInfo.Text(g_Plugin.ViewPanelT.left + 1, g_Plugin.ViewPanelT.bottom - 1, PanelTextColor, pTitleText);

	// Могла остаться полоса прокрутки
	pTitleText[0] = 0x2551; pTitleText[1] = 0;
	for (int y=g_Plugin.ViewPanelT.top+1; y<(g_Plugin.ViewPanelT.bottom-2); y++)
		g_StartupInfo.Text(g_Plugin.ViewPanelT.right, y, PanelFrameColor, pTitleText);


	nLen = 0;
	// Поехали шаблоны
	for (int i=1; i<=3; i++) {
		pTitleText[0] = 0;
		LPCTSTR pszTempl = (i==1) ? g_QViewTemplate1 : (i==2) ? g_QViewTemplate2 : g_QViewTemplate3;
		TemplatePrint(bDecoding, pTitleText, sizeofarray(pTitleText), 
			(*pszTempl == L'$' || *pszTempl == L'#') ? (pszTempl + 1) : pszTempl );
		nLen = lstrlen(pTitleText);
		if (nLen >= lPanel) pTitleText[lPanel] = 0;
		if (nLen>0) {
			X = g_Plugin.ViewPanelT.left + 1;
			if (*pszTempl == L'$') { // Если просили выровнять по центру
				X = (g_Plugin.ViewPanelT.right + g_Plugin.ViewPanelT.left - 2 - nLen) >> 1;
			} else if (*pszTempl == L'#') { // Если просили выровнять по правому краю
				X = g_Plugin.ViewPanelT.right - 2 - nLen;
			}
			g_StartupInfo.Text(X, g_Plugin.ViewPanelT.bottom - 3 + i, (i==1) ? PanelSelectedColor : PanelTextColor, pTitleText);
		}
	}

	//////const вызывает инициализацию переменных ДО входа в функцию (по возможности). что нам не подходит - хочется видеть последовательность действий!
	//size_t lText = wcslen(pTitleText);
	//if (lText < lPanel) {
	//	wmemset(pTitleText + lText, ' ', lPanel - lText);
	//	if ((lPanel - lText) > 8)
	//		lstrcpy(pTitleText + lPanel - 9, L"PicView2 ");
	//}

	//g_StartupInfo.Text(g_Plugin.ViewPanelT.left + 1, g_Plugin.ViewPanelT.bottom - 1, PanelTextColor, pTitleText);

	// Это нужно выполнять в основной нити!!! Иначе Виснет
	g_StartupInfo.Text(0, 0, 0, NULL);
}
