// Test.cpp : Defines the entry point for the console application.
//

#ifndef _WIN32_WINNT            // Specifies that the minimum required platform is Windows Vista.
#define _WIN32_WINNT 0x0600     // Change this to the appropriate value to target other versions of Windows.
#endif

#include <stdio.h>
#include <tchar.h>
#include <conio.h>
#include <windows.h>
#include "../../common/pluginW1900.hpp"

#define DEFINE_CONSOLE_EXPORTS
#include "../ConEmuDw.h"


int _tmain(int argc, _TCHAR* argv[])
{
	int c;
	HMODULE h = LoadLibrary(_T("ConEmuDw.dll"));
	if (!h || (h == INVALID_HANDLE_VALUE))
	{
		printf("Can't load library: ConEmuDw.dll, ErrCode=0x%08X\n", GetLastError());
		return 100;
	}
	GetColorDialog_t GetColorDialog = (GetColorDialog_t)GetProcAddress(h, "GetColorDialog");
	GetTextAttributes_t GetTextAttributes = (GetTextAttributes_t)GetProcAddress(h, "GetTextAttributes");
	SetTextAttributes_t SetTextAttributes = (SetTextAttributes_t)GetProcAddress(h, "SetTextAttributes");
	ClearExtraRegions_t ClearExtraRegions = (ClearExtraRegions_t)GetProcAddress(h, "ClearExtraRegions");
	ReadOutput_t ReadOutput = (ReadOutput_t)GetProcAddress(h, "ReadOutput");
	WriteOutput_t WriteOutput = (WriteOutput_t)GetProcAddress(h, "WriteOutput");
	Commit_t CommitOutput = (Commit_t)GetProcAddress(h, "CommitOutput");
	if (!GetColorDialog || !GetTextAttributes || !SetTextAttributes || !CommitOutput || !WriteOutput || !ReadOutput)
	{
		printf("Export not found in ConEmuDw.dll\n");
		return 100;
	}

	printf(
		"'1' or '2' for color dialog,\n"
		"'3' change attrs, '4' fill truecolor\n"
		"'5' write text 4bit, '6' write truecolor text\n"
		//"'7' truecolor animate\n"
		"Esc - Exit");
	FarColor c1 = {FCF_FG_4BIT, 7}, c2 = {FCF_FG_4BIT|FCF_BG_4BIT, 0, 7};
	while ((c = _getch()) != 27)
	{
		CONSOLE_SCREEN_BUFFER_INFO csbi = {};
		HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
		GetConsoleScreenBufferInfo(h, &csbi);
	
		switch (c)
		{
		case '1':
		case '2':
			if (GetColorDialog((c=='1') ? &c1 : &c2, (c=='1'), (c=='1')))
			{
				int nWidth = 25, nHeight = 10;
				COORD BufSize = {nWidth,nHeight};
				FAR_CHAR_INFO* p = (FAR_CHAR_INFO*)calloc(BufSize.X*BufSize.Y,sizeof(*p));
				COORD BufCoord = {0,0};
				SMALL_RECT rcWrite = {10+csbi.srWindow.Left,12+csbi.srWindow.Top};
				rcWrite.Right = rcWrite.Left+nWidth-1;
				rcWrite.Bottom = rcWrite.Top+nHeight-1;
				FarColor clr = (c=='1') ? c1 : c2;

				LPCWSTR pszText = L"Font Test Font Test Font Test Font Test Font Test Font Test Font Test Font Test Font Test Font Test Font Test ";
				for (int Y = BufCoord.Y; Y <= (BufCoord.Y + nHeight - 1); Y++)
				{
					for (int X = BufCoord.X; X <= (BufCoord.X + nWidth - 1); X++)
					{
						FAR_CHAR_INFO* pp = p+X+Y*BufSize.X;
						pp->Char = pszText[X - BufCoord.X];
						pp->Attributes = clr;
					}
				}
				WriteOutput(p, BufSize, BufCoord, &rcWrite);
				free(p);
				CommitOutput();
			}
			break;
		case '3':
		case '4':
			{
				CONSOLE_SCREEN_BUFFER_INFO csbi = {};
				HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
				if (GetConsoleScreenBufferInfo(h, &csbi))
				{
					int nBufWidth  = csbi.srWindow.Right - csbi.srWindow.Left + 1;
					int nBufHeight = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
					int nBufCount  = nBufWidth*nBufHeight;
					FarColor* p = (FarColor*)calloc(nBufCount, sizeof(*p));
					if (c == '3')
					{
						if (GetTextAttributes(p))
						{
							for (int i = 0; i < nBufCount; i++)
							{
								p[i].Flags = FCF_BG_4BIT|(p[i].Flags&FCF_FG_4BIT);
								p[i].BackgroundColor = ((p[i].BackgroundColor+1) & 0xF);
							}
							SetTextAttributes(p);
							CommitOutput();
						}
					}
					else
					{
						FarColor* pp = p;
						for (int y = csbi.srWindow.Top; y <= csbi.srWindow.Bottom; y++)
						{
							for (int x = csbi.srWindow.Left; x <= csbi.srWindow.Right; x++, pp++)
							{
								pp->Flags = FCF_FG_4BIT;
								pp->ForegroundColor = 7;
								pp->BackgroundColor = RGB(0,0,((csbi.srWindow.Right - x) * 255 / nBufWidth));
							}
						}
						SetTextAttributes(p);
						CommitOutput();
					}
					free(p);
				}
			}
			break;
		case '5':
		case '6':
			{
				//BOOL ReadOutput(FAR_CHAR_INFO* Buffer, COORD BufferSize, COORD BufferCoord, SMALL_RECT* ReadRegion);
				//BOOL WriteOutput(const FAR_CHAR_INFO* Buffer, COORD BufferSize, COORD BufferCoord, SMALL_RECT* WriteRegion);
				COORD BufSize = {99,100};
				FAR_CHAR_INFO* p = (FAR_CHAR_INFO*)calloc(BufSize.X*BufSize.Y,sizeof(*p));
				COORD BufCoord = {30,20};
				int nWidth = 25, nHeight = 10;
				SMALL_RECT rcWrite = {1+csbi.srWindow.Left,8+csbi.srWindow.Left};
				rcWrite.Right = rcWrite.Left+nWidth-1;
				rcWrite.Bottom = rcWrite.Top+nHeight-1;

				LPCWSTR pszText4 = L"4bit Text 4bit Text 4bit Text 4bit Text 4bit Text 4bit Text 4bit Text 4bit Text";
				LPCWSTR pszText24 = L"24bit Text 24bit Text 24bit Text 24bit Text 24bit Text 24bit Text 24bit Text 24bit Text";
				LPCWSTR pszText = (c == '5') ? pszText4 : pszText24;
				for (int Y = BufCoord.Y; Y <= (BufCoord.Y + nHeight - 1); Y++)
				{
					for (int X = BufCoord.X; X <= (BufCoord.X + nWidth - 1); X++)
					{
						FAR_CHAR_INFO* pp = p+X+Y*BufSize.X;
						pp->Char = pszText[X - BufCoord.X];
						if (c == '5')
						{
							pp->Attributes.Flags = FCF_FG_4BIT|FCF_BG_4BIT;
							pp->Attributes.ForegroundColor = 7;
							pp->Attributes.BackgroundColor = (Y - BufCoord.Y) % 16;
						}
						else
						{
							switch ((Y - BufCoord.Y) % 4)
							{
							case 1:
								pp->Attributes.Flags = FCF_FG_BOLD; break;
							case 2:
								pp->Attributes.Flags = FCF_FG_ITALIC; break;
							case 3:
								pp->Attributes.Flags = FCF_FG_UNDERLINE; break;
							default:
								pp->Attributes.Flags = 0;
							}
							pp->Attributes.ForegroundColor = RGB(0,((X - BufCoord.X) * 255 / nWidth),0);
							pp->Attributes.BackgroundColor = RGB(0,0,((BufCoord.X + nWidth - X) * 255 / nWidth));
						}
					}
				}
				WriteOutput(p, BufSize, BufCoord, &rcWrite);
				free(p);
				CommitOutput();
			}
			break;
		}
	}
	printf("\n");


	// Убрать TRUE-color
	{
		CONSOLE_SCREEN_BUFFER_INFO csbi = {};
		HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
		if (GetConsoleScreenBufferInfo(h, &csbi))
		{
			int nBufWidth  = csbi.srWindow.Right - csbi.srWindow.Left + 1;
			int nBufHeight = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
			int nBufCount  = nBufWidth*nBufHeight;
			FarColor* p = (FarColor*)calloc(nBufCount, sizeof(*p));
			FarColor* pp = p;
			for (int y = csbi.srWindow.Top; y <= csbi.srWindow.Bottom; y++)
			{
				for (int x = csbi.srWindow.Left; x <= csbi.srWindow.Right; x++, pp++)
				{
					pp->Flags = FCF_FG_4BIT|FCF_BG_4BIT;
					pp->ForegroundColor = 7;
					pp->BackgroundColor = 0;
				}
			}
			SetTextAttributes(p);
			free(p);
			CommitOutput();
		}
	}

	return 0;
}

