
#pragma once

#define VS90_FLOAT

bool LoadGflLibrary();
bool EnumerateGflFunct();
bool TestGflVersion();
int LoadGfl(void);
bool UnLoadGfl(void);
void GetFarRect();
bool FarGetFname(TCHAR* FileName, bool DetectCoordOnly = false);
bool LoadPicture(TCHAR* , GFL_BITMAP** );
void FillBackground(RECT *ConsoleRect);
void GetRangedRect(RECT *RangedRect, const COORD* coord = 0, const bool* VideoFullWindow = 0);
void ExtractPicture(GFL_BITMAP *, BITMAPINFOHEADER*, unsigned char **);
bool PreparePicture(GFL_BITMAP*);
void ReleasePicture(GFL_BITMAP**);
void ViewPicture();
bool GetParam(TCHAR*, DWORD*, DWORD);
void SetParam(const TCHAR*, DWORD);
bool PluginInit();
void ReadConfig();
TCHAR *GetMsg(int MsgId);
int WINAPI _export GetMinFarVersion(void);
void WINAPI _export SetStartupInfo(struct PluginStartupInfo*);
void WINAPI _export GetPluginInfo(struct PluginInfo*);
void WINAPI _export ExitFar(void);
HANDLE WINAPI _export OpenPlugin(int, int);
int WINAPI Configure(int);

bool FileSizeSmaller(TCHAR *, unsigned long);
bool IsTerminalMode();
bool ReleaseMedia();

bool FillWindowCreate(bool abModal);
void FillWindowClose();
void FillWindowShow(BOOL abShow);

void FindParentHwnd();


#include "MultiGlobal.h"
