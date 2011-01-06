
/*
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
*/


#include <windows.h>
#include <TCHAR.H>
#include "Plugin.hpp"

#ifndef ARRAYSIZE
	#define ARRAYSIZE(array) (sizeof(array)/sizeof(*array))
#endif

/* FAR */
PluginStartupInfo psi;
FarStandardFunctions fsf;

// The structure of an ID3v1.1 tag.
typedef struct tag_ID3_1_1 {
	char head[3]; //TAG
	char title[30];
	char artist[30];
	char album[30];
	char year[4];
	char comment[28];
	BYTE _null;
	BYTE track;
	BYTE genre;
} ID3_1_1;

/*
array(string) id3_genres = ({
  "Blues", // 0
  "Classic Rock",
  "Country",
  "Dance",
  "Disco",
  "Funk",
  "Grunge",
  "Hip-Hop",
  "Jazz",
  "Metal",
  "New Age",
  "Oldies",
  "Other",
  "Pop",
  "R&B",
  "Rap",
  "Reggae",
  "Rock",
  "Techno",
  "Industrial",
  "Alternative",
  "Ska",
  "Death Metal",
  "Pranks",
  "Soundtrack",
  "Euro-Techno",
  "Ambient",
  "Trip-Hop",
  "Vocal",
  "Jazz+Funk",
  "Fusion",
  "Trance",
  "Classical",
  "Instrumental",
  "Acid",
  "House",
  "Game",
  "Sound Clip",
  "Gospel",
  "Noise",
  "AlternRock",
  "Bass",
  "Soul",
  "Punk",
  "Space",
  "Meditative",
  "Instrumental Pop",
  "Instrumental Rock",
  "Ethnic",
  "Gothic",
  "Darkwave",
  "Techno-Industrial",
  "Electronic",
  "Pop-Folk",
  "Eurodance",
  "Dream",
  "Southern Rock",
  "Comedy",
  "Cult",
  "Gangsta",
  "Top 40",
  "Christian Rap",
  "Pop/Funk",
  "Jungle",
  "Native American",
  "Cabaret",
  "New Wave",
  "Psychadelic",
  "Rave",
  "Showtunes",
  "Trailer",
  "Lo-Fi",
  "Tribal",
  "Acid Punk",
  "Acid Jazz",
  "Polka",
  "Retro",
  "Musical",
  "Rock & Roll",
  "Hard Rock", // 79
  "Folk",
  "Folk-Rock",
  "National Folk",
  "Swing",
  "Fast Fusion",
  "Bebob",
  "Latin",
  "Revival",
  "Celtic",
  "Bluegrass",
  "Avantgarde",
  "Gothic Rock",
  "Progressive Rock",
  "Psychedelic Rock",
  "Symphonic Rock",
  "Slow Rock",
  "Big Band",
  "Chorus",
  "Easy Listening",
  "Acoustic",
  "Humour",
  "Speech",
  "Chanson",
  "Opera",
  "Chamber Music",
  "Sonata",
  "Symphony",
  "Booty Bass",
  "Primus",
  "Porn Groove",
  "Satire",
  "Slow Jam",
  "Club",
  "Tango",
  "Samba",
  "Folklore",
  "Ballad",
  "Power Ballad",
  "Rhythmic Soul",
  "Freestyle",
  "Duet",
  "Punk Rock",
  "Drum Solo",
  "A capella",
  "Euro-House",
  "Dance Hall", // 125
  "Goa",
  "Drum & Bass",
  "Club-House",
  "Hardcore",
  "Terror",
  "Indie",
  "BritPop",
  "Negerpunk",
  "Polsk Punk",
  "Beat",
  "Christian",
  "Heavy Metal",
  "Black Metal",
  "Crossover",
  "Contemporary",
  "Christian Rock",
  "Merengue",
  "Salsa",
  "Thrash Metal",
  "Anime",
  "JPop",
  "Synthpop",
});
*/

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
                     )
{
    return TRUE;
}


#if defined(__GNUC__)

extern
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
                     );

#ifdef __cplusplus
extern "C"{
#endif
  BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved);
  int WINAPI GetMinFarVersionW(void);
  void WINAPI SetStartupInfoW(const struct PluginStartupInfo *Info);
  void WINAPI GetPluginInfoW(struct PluginInfo *Info);
  int WINAPI GetCustomDataW(const wchar_t *FilePath, wchar_t **CustomData);
  void WINAPI FreeCustomDataW(wchar_t *CustomData);
#ifdef __cplusplus
};
#endif

BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved)
{
  DllMain(hDll, dwReason, lpReserved);
  return TRUE;
}
#endif


int WINAPI GetMinFarVersionW(void)
{
    return MAKEFARVERSION(2,0,1588);
}

void WINAPI SetStartupInfoW(const struct PluginStartupInfo *Info)
{
    ::psi = *Info;
    ::fsf = *(Info->FSF);
}

void WINAPI GetPluginInfoW(struct PluginInfo *Info)
{
	Info->Flags = PF_PRELOAD;
}

void cpytag(char* &psz, const char* src, int nMax)
{
	char* p = psz;
	for (int i = 0; i < nMax && *src; i++)
		*(psz++) = *(src++);
	*psz = 0;
	while (psz > p && *(psz-1) == ' ')
		*(--psz) = 0;
}

void trimtag(char* psz, int nMax)
{
	int j = 0;
	for (int i = 0; i < nMax && psz[i]; i++) {
		if (psz[i] != ' ')
			j = i+1;
	}
	if (j < nMax && psz[j] == ' ') psz[j] = 0;
}

int WINAPI GetCustomDataW(const wchar_t *FilePath, wchar_t **CustomData)
{
	*CustomData = NULL;

	int nLen = lstrlenW(FilePath);
	if (nLen < 5) return FALSE;

	if (lstrcmpiW(FilePath+nLen-4, L".mp3")) return FALSE;

	HANDLE hFile = CreateFileW(FilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0,0);
	if (hFile == INVALID_HANDLE_VALUE) return FALSE;

	LARGE_INTEGER nSize = {{0}};
	if (!GetFileSizeEx(hFile, &nSize) || nSize.QuadPart <= sizeof(ID3_1_1)) {
		CloseHandle(hFile); return FALSE;
	}

	LARGE_INTEGER nPos, nRet;
	nPos.QuadPart = nSize.QuadPart - sizeof(ID3_1_1);
	if (!SetFilePointerEx(hFile, nPos, &nRet, FILE_BEGIN)) {
		CloseHandle(hFile); return FALSE;
	}

	ID3_1_1 tag;
	DWORD nRead;
	if (!ReadFile(hFile, &tag, sizeof(tag), &nRead, NULL) || nRead!=sizeof(tag)) {
		CloseHandle(hFile); return FALSE;
	}
	CloseHandle(hFile);

	if (tag.head[0] != 'T' || tag.head[1] != 'A' || tag.head[2] != 'G') return FALSE;
	trimtag(tag.title, ARRAYSIZE(tag.title)); trimtag(tag.artist, ARRAYSIZE(tag.artist));
	if (tag.title[0] == 0 && tag.artist[0] == 0) return FALSE;
	
	char szInfo[128], *psz; szInfo[0] = 0;
	psz = szInfo;
	if (tag.title[0])
		cpytag(psz, tag.title, 30);
	if (tag.artist[0]) {
		if (psz != szInfo) {
			*(psz++) = ' '; *(psz++) = '-'; *(psz++) = ' '; *psz = 0;
		}
		cpytag(psz, tag.artist, 30);
	}
	if (psz == szInfo) return FALSE;

	nLen = (psz - szInfo +1);
	*CustomData = (wchar_t*)malloc(nLen*2);
	MultiByteToWideChar(CP_ACP, 0, szInfo, -1, *CustomData, nLen);

	return TRUE;
}

void WINAPI FreeCustomDataW(wchar_t *CustomData)
{
	if (CustomData)
		free(CustomData);
}
