

#define _FAR_NO_NAMELESS_UNIONS
#include <unicode/plugin.hpp>
#include <CRT/crt.hpp>
#include <stdio.h>

#define IDENTIFY_EDITOR_BY_FILENAME
  // Because identify by EditorID doesn't work - I don't know the ID of my editor
#define OVERWRITE_DIALOG_COMPLEX

#define LOG_MESSAGE(mode,...) { FILE * ltpHandle = fopen("ltp.log", mode); fprintf(ltpHandle, __VA_ARGS__); fclose(ltpHandle); }
#define SHOW_MESSAGE(...) { const TCHAR *MsgItems[] = { L"Debug", __VA_ARGS__ }; Info.Message(Info.ModuleNumber, FMSG_MB_OK, NULL, MsgItems, ArraySize(MsgItems), 1); }

#ifdef __GNUC__
#define _i64(num) num##ll
#define _ui64(num) num##ull
#else
#define _i64(num) num##i64
#define _ui64(num) num##ui64
#endif

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

WARNING("USING NM==MAX_PATH")
#define NM MAX_PATH

/****************************************************************************
 * These are needed to suppress GCC startup code generation
 ****************************************************************************/
#if defined(__GNUC__)
#ifdef __cplusplus
extern "C"{
#endif
  BOOL WINAPI DllMainCRTStartup(HANDLE hDll, DWORD dwReason, LPVOID lpReserved);
#ifdef __cplusplus
};
#endif

BOOL WINAPI DllMainCRTStartup(HANDLE hDll, DWORD dwReason, LPVOID lpReserved)
{
  (void) lpReserved;
  (void) dwReason;
  (void) hDll;
  return TRUE;
}
#endif

/****************************************************************************
 * Constants for .lng file strings extraction
 ****************************************************************************/
enum CompareLng {
  MNoLngStringDefined = -1,

  MOK,
  MCancel,

  MCompare,

  MCmpTitle,
  MProcessBox,
  MProcessSubfolders,
  MUseMaxScanDepth,
  MProcessSelected,
  MCompareBox,
  MCompareTime,
  MCompareLowPrecision,
  MCompareIgnoreTimeZone,
  MCompareSize,
  MCompareContents,
  MCompareContentsIgnore,
  MCompareIgnoreNewLines,
  MCompareIgnoreWhitespace,
  MMessageWhenNoDiff,

  MFilePanelsRequired,

  MComparing,
  MComparingWith,

  MComparingFiles,

  MNoDiffTitle,
  MNoDiffBody,

  MNoMemTitle,
  MNoMemBody,

  MOldFARTitle,
  MOldFARBody,

  MEscTitle,
  MEscBody,
  
  MShowReport,
  MSynchronizeDirs,
  
  MCopying,
  MCopyingTo,
  
  MWarning,
  MLeftFileAlreadyExists,
  MRightFileAlreadyExists,
  MLeftFileReadonly,
  MRightFileReadonly,
  MNew,
  MExisting,
  MOverwrite,
  MOverwriteAll,
  MSkip,
  MSkipAll,
  
  MFailedToOverwriteFile,
  
	MCopyRightToLeft,
  MOverwriteRightToLeft,
	MCopyLeftToRight,
  MOverwriteLeftToRight,
  
  MReportFileTitle,

  MOpenErrorTitle,
  MOpenErrorBody,
};

/****************************************************************************
 * Plugin settings
 ****************************************************************************/
struct Options {
  int ProcessSubfolders,
      UseMaxScanDepth,
      MaxScanDepth,
      ProcessSelected,
      ProcessHidden,
      CompareTime,
      LowPrecisionTime,
      IgnorePossibleTimeZoneDifferences,
      CompareSize,
      CompareContents,
      CompareContentsIgnore,
      IgnoreWhitespace,
      IgnoreNewLines,
      MessageWhenNoDiff,
      ShowReport;
} Opt;

struct SyncOptions {
	int CopyLeft,
		CopyRight,
		AskOverwriteLeft,
		AskOverwriteRight,
		AskReadonlyLeft, SkipReadonlyLeft,
		AskReadonlyRight, SkipReadonlyRight;
};

struct PendingSync {
	PendingSync * next;
#ifndef IDENTIFY_EDITOR_BY_FILENAME
	int EditorID;
#endif
	TCHAR LeftDir[NM];
	TCHAR RightDir[NM];
	TCHAR ReportFileName[NM];
};

static struct PendingSync * psEditorsQueue = NULL;

/****************************************************************************
 * The copies of FAR structures
 ****************************************************************************/
static struct PluginStartupInfo Info;
static struct FarStandardFunctions FSF;
static TCHAR  *PluginRootKey = NULL;

static void WFD2FFD(WIN32_FIND_DATA &wfd, FAR_FIND_DATA &ffd)
{
  ffd.dwFileAttributes = wfd.dwFileAttributes;
  ffd.ftCreationTime   = wfd.ftCreationTime;
  ffd.ftLastAccessTime = wfd.ftLastAccessTime;
  ffd.ftLastWriteTime  = wfd.ftLastWriteTime;
  ffd.nFileSize        = ((__int64)wfd.nFileSizeHigh << 32) | wfd.nFileSizeLow;
  ffd.nPackSize               = 0;
  ffd.lpwszFileName          = wcsdup(wfd.cFileName);
  ffd.lpwszAlternateFileName = wcsdup(wfd.cAlternateFileName);
}

struct OwnPanelInfo
{
  int PanelType;
  int Plugin;
  PluginPanelItem *PanelItems;
  int ItemsNumber;
  PluginPanelItem *SelectedItems;
  int SelectedItemsNumber;
  wchar_t *lpwszCurDir;
  DWORD Flags;
};
/****************************************************************************
 * FAR service function wrapper: obtain a string from .lng-file
 ****************************************************************************/
static const TCHAR *GetMsg(int CompareLng)
{
  const TCHAR *res=Info.GetMsg(Info.ModuleNumber, CompareLng);
  return res;
}


static int iTruncLen;

/****************************************************************************
 * Truncate long filename beginning (or append short one)
 * for proper display in comparison message
 ****************************************************************************/
static void TrunCopy(TCHAR *cpDest, const TCHAR *cpSrc, const TCHAR cFiller = L' ')
{
  int iLen = lstrlen(FSF.TruncStr(lstrcpy(cpDest, cpSrc), iTruncLen));

  if (iLen < iTruncLen)
  {
    _tmemset(&cpDest[iLen], cFiller, iTruncLen - iLen);
    cpDest[iTruncLen] = L'\0';
  }
}

static bool bStart;
static bool bOpenFail;

/****************************************************************************
 * Display two files comparison message
 ****************************************************************************/
static void ShowMessage(const TCHAR *Name1, const TCHAR *Name2, __int64 Progress, __int64 Max, CompareLng msg1 = MComparing, CompareLng msg2 = MComparingWith)
{
  static DWORD dwTicks;
  DWORD dwNewTicks = GetTickCount();
  if (dwNewTicks - dwTicks < 500)
    return;
  dwTicks = dwNewTicks;

  TCHAR TruncName1[NM], TruncName2[NM], ProgressBar[NM];
  TrunCopy(TruncName1, Name1);
  TrunCopy(TruncName2, Name2);

  __int64 iProgress;
  if (Max <= 0)
    iProgress = 0;
  else if (Progress < Max)
    iProgress = iTruncLen * Progress / Max;
  else
    iProgress = iTruncLen;
  ProgressBar[iTruncLen] = L'\0';
  _tmemset(&ProgressBar[0], 0x2588, (size_t)iProgress);
  _tmemset(&ProgressBar[iProgress], 0x2591, (size_t)(iTruncLen - iProgress));

  const TCHAR *MsgItems[] = {
    GetMsg(MCmpTitle),
    GetMsg(msg1),
    TruncName1,
    GetMsg(msg2),
    TruncName2,
    ProgressBar
  };
  Info.Message(Info.ModuleNumber, bStart ? FMSG_LEFTALIGN :
               FMSG_LEFTALIGN|FMSG_KEEPBACKGROUND,
               NULL, MsgItems, ArraySize(MsgItems), 0);
  bStart = false;
}


/****************************************************************************
 * Dialog handler for ShowDialog
 ****************************************************************************/
LONG_PTR WINAPI ShowDialogProc(HANDLE hDlg, int Msg, int Param1, LONG_PTR Param2)
{
  static int CompareContents,
             CompareContentsIgnore,
             ProcessSubfolders,
             CompareTime;

  switch (Msg)
  {
    case DN_INITDIALOG:
      CompareContents = ((DWORD)Info.SendDlgMessage(hDlg, DM_GETDLGDATA, 0, 0)) & 0x000000FF;
      CompareContentsIgnore = CompareContents + 1;
      ProcessSubfolders = (((DWORD)Info.SendDlgMessage(hDlg, DM_GETDLGDATA, 0, 0)) >> 8) & 0x000000FF;
      CompareTime = ((DWORD)Info.SendDlgMessage(hDlg, DM_GETDLGDATA, 0, 0)) >> 16;
      break;

    case DN_BTNCLICK:
      if (Param1 == CompareTime || Param1 == ProcessSubfolders || Param1 == CompareContents || Param1 == CompareContentsIgnore)
      {
        if (Param2)
        {
          Info.SendDlgMessage(hDlg, DM_ENABLE, Param1+1, TRUE);
          if (!(Param1 == CompareContents && !Info.SendDlgMessage(hDlg, DM_GETCHECK, CompareContentsIgnore, 0)))
          {
            Info.SendDlgMessage(hDlg, DM_ENABLE, Param1+2, TRUE);
            if (Param1 == CompareContents)
              Info.SendDlgMessage(hDlg, DM_ENABLE, Param1+3, TRUE);
          }
        }
        else
        {
          Info.SendDlgMessage(hDlg, DM_ENABLE, Param1+1, FALSE);
          Info.SendDlgMessage(hDlg, DM_ENABLE, Param1+2, FALSE);
          if (Param1 == CompareContents)
            Info.SendDlgMessage(hDlg, DM_ENABLE, Param1+3, FALSE);
        }
      }
      break;
  }

  return Info.DefDlgProc(hDlg, Msg, Param1, Param2);
}

/****************************************************************************
 * Read settings from registry, show options dialog,
 * fill in Opt structure, save new settings if needed,
 * return true if user has pressed OK
 ****************************************************************************/
static bool ShowDialog(bool bPluginPanels, bool bSelectionPresent)
{
  static struct InitDialogItem {
    unsigned char Type;
    unsigned char X1, Y1, X2, Y2;
    int           Data;
    int           DefaultRegValue;
    const TCHAR   *SelectedRegValue;
    unsigned int  Flags;
    int          *StoreTo;
  } InitItems[] = {
    /* 0*/ { DI_DOUBLEBOX,    3,  1, 62, 21, MCmpTitle,                0, NULL,                                 0, NULL },
    /* 1*/ { DI_TEXT,         5,  2,  0,  0, MProcessBox,              0, NULL,                                 0, NULL },
    /* 2*/ { DI_CHECKBOX,     5,  3,  0,  0, MProcessSubfolders,       0, L"ProcessSubfolders",                 0, &Opt.ProcessSubfolders },
    /* 3*/ { DI_CHECKBOX,     9,  4,  0,  0, MUseMaxScanDepth,         0, L"UseMaxScanDepth",                   0, &Opt.UseMaxScanDepth },
    /* 4*/ { DI_FIXEDIT,      0,  4,  4,  0, MNoLngStringDefined,     99, L"MaxScanDepth",                      DIF_MASKEDIT, &Opt.MaxScanDepth },
    /* 5*/ { DI_CHECKBOX,     5,  5,  0,  0, MProcessSelected,         0, L"ProcessSelected",                   0, &Opt.ProcessSelected },
    /* 6*/ { DI_TEXT,         0,  6,  0,  0, MNoLngStringDefined,      0, NULL,                                 DIF_SEPARATOR, NULL },
    /* 7*/ { DI_TEXT,         5,  7,  0,  0, MCompareBox,              0, NULL,                                 0, NULL },
    /* 8*/ { DI_CHECKBOX,     5,  8,  0,  0, MCompareTime,             1, L"CompareTime",                       0, &Opt.CompareTime },
    /* 9*/ { DI_CHECKBOX,     9,  9,  0,  0, MCompareLowPrecision,     1, L"LowPrecisionTime",                  0, &Opt.LowPrecisionTime },
    /*10*/ { DI_CHECKBOX,     9, 10,  0,  0, MCompareIgnoreTimeZone,   1, L"IgnorePossibleTimeZoneDifferences", 0, &Opt.IgnorePossibleTimeZoneDifferences },
    /*11*/ { DI_CHECKBOX,     5, 11,  0,  0, MCompareSize,             1, L"CompareSize",                       0, &Opt.CompareSize },
    /*12*/ { DI_CHECKBOX,     5, 12,  0,  0, MCompareContents,         0, L"CompareContents",                   0, &Opt.CompareContents },
    /*13*/ { DI_CHECKBOX,     9, 13,  0,  0, MCompareContentsIgnore,   0, L"CompareContentsIgnore",             0, &Opt.CompareContentsIgnore },
    /*14*/ { DI_RADIOBUTTON, 13, 14,  0,  0, MCompareIgnoreNewLines,   1, L"IgnoreNewLines",                    DIF_GROUP, &Opt.IgnoreNewLines },
    /*15*/ { DI_RADIOBUTTON, 13, 15,  0,  0, MCompareIgnoreWhitespace, 0, L"IgnoreWhitespace",                  0, &Opt.IgnoreWhitespace },
    /*16*/ { DI_TEXT,         0, 16,  0,  0, MNoLngStringDefined,      0, NULL,                                 DIF_SEPARATOR, NULL },
    /*17*/ { DI_CHECKBOX,     5, 17,  0,  0, MShowReport,              1, L"ShowReport",                        0, &Opt.ShowReport },
    /*18*/ { DI_CHECKBOX,     5, 18,  0,  0, MMessageWhenNoDiff,       0, L"MessageWhenNoDiff",                 0, &Opt.MessageWhenNoDiff },
    /*19*/ { DI_TEXT,         0, 19,  0,  0, MNoLngStringDefined,      0, NULL,                                 DIF_SEPARATOR, NULL },
    /*20*/ { DI_BUTTON,       0, 20,  0,  0, MOK,                      0, NULL,                                 DIF_CENTERGROUP, NULL },
    /*21*/ { DI_BUTTON,       0, 20,  0,  0, MCancel,                  0, NULL,                                 DIF_CENTERGROUP, NULL }
  };
  struct FarDialogItem DialogItems[ArraySize(InitItems)];
  TCHAR Mask[] = L"99999";
  wchar_t tmpnum[ArraySize(InitItems)][32];

  memset(DialogItems,0,sizeof(DialogItems));

  HKEY hKey;
  if (!PluginRootKey ||
       RegOpenKeyEx(HKEY_CURRENT_USER, PluginRootKey, 0, KEY_QUERY_VALUE, &hKey) != ERROR_SUCCESS)
    hKey = 0;

  size_t DlgData=0;
  bool bNoFocus = true;
  size_t i;
  for (i = 0; i < ArraySize(InitItems); i++)
  {
    DWORD dwRegValue;
    DWORD dwSize                  = sizeof(DWORD);
    DialogItems[i].Type           = InitItems[i].Type;
    DialogItems[i].X1             = InitItems[i].X1;
    DialogItems[i].Y1             = InitItems[i].Y1;
    DialogItems[i].X2             = InitItems[i].X2;
    DialogItems[i].Y2             = InitItems[i].Y2;
    DialogItems[i].Focus          = FALSE;
    DialogItems[i].Flags          = InitItems[i].Flags;
    DialogItems[i].PtrData = (InitItems[i].Data == MNoLngStringDefined)
            ? L"" : GetMsg(InitItems[i].Data);

    dwRegValue = (hKey && InitItems[i].SelectedRegValue &&
                  RegQueryValueEx(hKey, InitItems[i].SelectedRegValue, NULL,
                  NULL, (LPBYTE)&dwRegValue, &dwSize)
                  == ERROR_SUCCESS ) ? dwRegValue : InitItems[i].DefaultRegValue;
    if (DialogItems[i].Type == DI_CHECKBOX || DialogItems[i].Type == DI_RADIOBUTTON)
      DialogItems[i].Param.Selected = dwRegValue;
    else if (DialogItems[i].Type == DI_FIXEDIT)
    {
      DialogItems[i].PtrData = tmpnum[i];
      FSF.itoa(dwRegValue, (TCHAR *)DialogItems[i].PtrData, 10);
      DialogItems[i].Param.Mask = Mask;
      DialogItems[i].X1 = DialogItems[i-1].X1 + lstrlen(DialogItems[i-1].PtrData)
                          - (_tcschr(DialogItems[i-1].PtrData, L'&')?1:0) + 5;
      DialogItems[i].X2 += DialogItems[i].X1;
    }

    switch (InitItems[i].Data)
    {
      case MCompareContents:
        DlgData += i;
        if (bPluginPanels)
        {
          DialogItems[i].Flags |= DIF_DISABLE;
          DialogItems[i].Param.Selected = 0;
        }
        if (!DialogItems[i].Param.Selected)
        {
          InitItems[i+1].Flags |= DIF_DISABLE;
          InitItems[i+2].Flags |= DIF_DISABLE;
          InitItems[i+3].Flags |= DIF_DISABLE;
        }
        else
        {
          InitItems[i+1].Flags &= ~DIF_DISABLE;
          InitItems[i+2].Flags &= ~DIF_DISABLE;
          InitItems[i+3].Flags &= ~DIF_DISABLE;
        }
        break;
      case MCompareContentsIgnore:
        if (!DialogItems[i].Param.Selected || DialogItems[i].Flags & DIF_DISABLE)
        {
          InitItems[i+1].Flags |= DIF_DISABLE;
          InitItems[i+2].Flags |= DIF_DISABLE;
        }
        else
        {
          InitItems[i+1].Flags &= ~DIF_DISABLE;
          InitItems[i+2].Flags &= ~DIF_DISABLE;
        }
        break;
      case MCompareIgnoreWhitespace:
        if (DialogItems[i].Param.Selected == DialogItems[i-1].Param.Selected)
        {
          DialogItems[i-1].Param.Selected = 1;
          DialogItems[i].Param.Selected = 0;
        }
        break;
      case MProcessSubfolders:
        DlgData += i<<8;
        if (bPluginPanels)
        {
          DialogItems[i].Flags |= DIF_DISABLE;
          DialogItems[i].Param.Selected = 0;
        }
        if (!DialogItems[i].Param.Selected)
        {
          InitItems[i+1].Flags |= DIF_DISABLE;
          InitItems[i+2].Flags |= DIF_DISABLE;
        }
        else
        {
          InitItems[i+1].Flags &= ~DIF_DISABLE;
          InitItems[i+2].Flags &= ~DIF_DISABLE;
        }
        break;
      case MProcessSelected:
        if (!bSelectionPresent)
        {
          DialogItems[i].Flags |= DIF_DISABLE;
          DialogItems[i].Param.Selected = 0;
        }
        break;
      case MCompareTime:
        DlgData += i<<16;
        if (!DialogItems[i].Param.Selected)
        {
          InitItems[i+1].Flags |= DIF_DISABLE;
          InitItems[i+2].Flags |= DIF_DISABLE;
        }
        else
        {
          InitItems[i+1].Flags &= ~DIF_DISABLE;
          InitItems[i+2].Flags &= ~DIF_DISABLE;
        }
        break;
      case MOK:
        DialogItems[i].DefaultButton = 1;
        break;
    }

    if (bNoFocus && DialogItems[i].Type == DI_CHECKBOX && !(DialogItems[i].Flags & DIF_DISABLE))
    {
      DialogItems[i].Focus = TRUE;
      bNoFocus = false;
    }
  }

  if (hKey)
    RegCloseKey(hKey);

  HANDLE hDlg = Info.DialogInit(Info.ModuleNumber, -1, -1, 66, 23, L"Contents",
                               DialogItems, ArraySize(DialogItems), 0, 0,
                               ShowDialogProc, DlgData);
  if (hDlg == INVALID_HANDLE_VALUE)
    return false;

  int ExitCode = Info.DialogRun(hDlg);
  if (ExitCode != ArraySize(InitItems) - 2) {
    Info.DialogFree(hDlg);
    return false;
  }
  for (i = 0; i < ArraySize(InitItems); i++)
    if (InitItems[i].StoreTo)
      if (InitItems[i].Type == DI_CHECKBOX || InitItems[i].Type == DI_RADIOBUTTON)
        *InitItems[i].StoreTo = (DWORD)Info.SendDlgMessage(hDlg,DM_GETCHECK,i,0);
      else if (InitItems[i].Type == DI_FIXEDIT)
        *InitItems[i].StoreTo = FSF.atoi((const TCHAR *)Info.SendDlgMessage(hDlg,DM_GETCONSTTEXTPTR,i,0));

  DWORD dwDisposition;
  if (PluginRootKey &&
       RegCreateKeyEx(HKEY_CURRENT_USER, PluginRootKey, 0, NULL, REG_OPTION_NON_VOLATILE,
       KEY_ALL_ACCESS, NULL, &hKey, &dwDisposition) == ERROR_SUCCESS)
  {
    for (i = 0; i < ArraySize(InitItems); i++)
      if ((int)Info.SendDlgMessage(hDlg,DM_ENABLE,i,-1) && InitItems[i].SelectedRegValue)
      {
        DWORD dwValue = *InitItems[i].StoreTo;
        RegSetValueEx(hKey, InitItems[i].SelectedRegValue, 0, REG_DWORD, (BYTE *)&dwValue, sizeof(dwValue));
      }
    RegCloseKey(hKey);
  }

  if (bPluginPanels)
  {
    Opt.ProcessSubfolders = FALSE;
    Opt.CompareContents = FALSE;
  }
  Opt.ProcessHidden = (Info.AdvControl(Info.ModuleNumber, ACTL_GETPANELSETTINGS, NULL) &
                        FPS_SHOWHIDDENANDSYSTEMFILES) != 0;

  Info.DialogFree(hDlg);
  return true;
}

static bool bBrokenByEsc;
static HANDLE hConInp = INVALID_HANDLE_VALUE;

/****************************************************************************
 * Check for Esc keypress. Return true if user has pressed Esc
 ****************************************************************************/
static bool CheckForEsc(void)
{
  if (hConInp == INVALID_HANDLE_VALUE)
    return false;

  static DWORD dwTicks;
  DWORD dwNewTicks = GetTickCount();
  if (dwNewTicks - dwTicks < 500)
    return false;
  dwTicks = dwNewTicks;

  INPUT_RECORD rec;
  DWORD ReadCount;
  while (PeekConsoleInput(hConInp, &rec, 1, &ReadCount) && ReadCount)
  {
    ReadConsoleInput(hConInp, &rec, 1, &ReadCount);
    if ( rec.EventType == KEY_EVENT && rec.Event.KeyEvent.wVirtualKeyCode == VK_ESCAPE &&
         rec.Event.KeyEvent.bKeyDown )
      // Optional Esc interrupt acknowledgement
    {
      if ( Info.AdvControl(Info.ModuleNumber, ACTL_GETCONFIRMATIONS, NULL) & FCS_INTERRUPTOPERATION )
      {
        const TCHAR *MsgItems[] = {
          GetMsg(MEscTitle),
          GetMsg(MEscBody),
          GetMsg(MOK),
          GetMsg(MCancel)
        };
        if ( !Info.Message(Info.ModuleNumber, FMSG_WARNING, NULL,
                           MsgItems, ArraySize(MsgItems), 2) )
          return bBrokenByEsc = true;
      }
      else
        return bBrokenByEsc = true;
    }
  }

  return false;
}

/****************************************************************************
 * Build full file path from path and filename
 ****************************************************************************/
static TCHAR *BuildFullFilename(const TCHAR *cpDir, const TCHAR *cpFileName)
{
  static TCHAR cName[MAX_PATH*2];
  FSF.AddEndSlash(lstrcpy(cName, cpDir));

  return lstrcat(cName, cpFileName);
}

struct FileIndex {
  PluginPanelItem **ppi;
  int iCount;
};

/****************************************************************************
 * Compare filenames in two PluginPanelItem structures
 * for qsort()
 ****************************************************************************/
static int __cdecl PICompare(const void *el1, const void *el2)
{
  const PluginPanelItem *ppi1 = *(const PluginPanelItem **)el1, *ppi2 = *(const PluginPanelItem **)el2;

  if (ppi1->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
  {
    if (!(ppi2->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
      return -1;
  }
  else
  {
    if (ppi2->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      return 1;
  }

  return -FSF.LStricmp(ppi1->FindData.lpwszFileName, ppi2->FindData.lpwszFileName);
}

/****************************************************************************
 * Build sorted file list for quick comparison
 ****************************************************************************/
static bool BuildPanelIndex(const OwnPanelInfo *pInfo, struct FileIndex *pIndex
                            ,HANDLE Filter
                           )
{
  bool bProcessSelected;
  pIndex->ppi = NULL;
  pIndex->iCount = ( bProcessSelected = (Opt.ProcessSelected && pInfo->SelectedItemsNumber &&
                     (pInfo->SelectedItems[0].Flags & PPIF_SELECTED)) ) ? pInfo->SelectedItemsNumber :
                     pInfo->ItemsNumber;
  if (!pIndex->iCount)
    return true;
  if (!(pIndex->ppi = (PluginPanelItem **)malloc(pIndex->iCount * sizeof(pIndex->ppi[0]))))
    return false;
  int j = 0;
  for (int i = pInfo->ItemsNumber - 1; i >= 0 && j < pIndex->iCount; i--)
    if ( (Opt.ProcessSubfolders ||
         !(pInfo->PanelItems[i].FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) &&
         (!bProcessSelected || (pInfo->PanelItems[i].Flags & PPIF_SELECTED)) &&
         lstrcmp(pInfo->PanelItems[i].FindData.lpwszFileName, L"..") &&
         lstrcmp(pInfo->PanelItems[i].FindData.lpwszFileName, L".") )
      {
        if (!Info.FileFilterControl(Filter,FFCTL_ISFILEINFILTER,0,(LONG_PTR)&pInfo->PanelItems[i].FindData))
          continue;
        pIndex->ppi[j++] = &pInfo->PanelItems[i];
      }
  if ((pIndex->iCount = j) != 0)
    FSF.qsort(pIndex->ppi, j, sizeof(pIndex->ppi[0]), PICompare);
  else
  {
    free(pIndex->ppi);
    pIndex->ppi = NULL;
    pIndex->iCount = 0;
  }

  return true;
}

/****************************************************************************
 * Free memory
 ****************************************************************************/
static void FreePanelIndex(struct FileIndex *pIndex)
{
  if (pIndex->ppi)
    free(pIndex->ppi);
  pIndex->ppi = NULL;
  pIndex->iCount = 0;
}

/****************************************************************************
 * Replace service function Info.GetDirList(). Returns
 * file list in Dir folder only, without subfolders.
 ****************************************************************************/
static int GetDirList(OwnPanelInfo *PInfo, const TCHAR *Dir)
{
  TCHAR cPathMask[MAX_PATH];
  WIN32_FIND_DATA wfdFindData;
  HANDLE hFind;
  struct PluginPanelItem **pPanelItem = &PInfo->PanelItems;
  int *pItemsNumber = &PInfo->ItemsNumber;
  {
    size_t dirLen = lstrlen(Dir);
    if( dirLen > ArraySize(cPathMask) - 3 ) //need space to add "\\*"
      return FALSE;
  }
  PInfo->lpwszCurDir = wcsdup(Dir);
  *pPanelItem = NULL;
  *pItemsNumber = 0;

  if ( (hFind = FindFirstFile(lstrcat(lstrcpy(cPathMask, Dir), L"\\*"), &wfdFindData)) ==
       INVALID_HANDLE_VALUE )
    return TRUE;

  int iRet = TRUE;
  do
  {
    if (!lstrcmp(wfdFindData.cFileName, L".") || !lstrcmp(wfdFindData.cFileName, L".."))
      continue;
    if ((wfdFindData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) && !Opt.ProcessHidden)
      continue;
    struct PluginPanelItem *pPPI;
    if (!(pPPI = (struct PluginPanelItem *)realloc(*pPanelItem, (*pItemsNumber + 1) * sizeof(*pPPI))))
    {
      iRet = FALSE;
      break;
    }
    *pPanelItem = pPPI;
    WFD2FFD(wfdFindData,(*pPanelItem)[(*pItemsNumber)++].FindData);
  } while (FindNextFile(hFind, &wfdFindData));
  FindClose(hFind);
  return iRet;
}

/****************************************************************************
 * Replace service function Info.FreeDirList().
 ****************************************************************************/
static void FreeDirList(OwnPanelInfo *AInfo)
{
  if (AInfo->PanelItems) {
    for ( int i = 0; i < AInfo->ItemsNumber; i++ ) {
      free((void*)AInfo->PanelItems[i].FindData.lpwszAlternateFileName);
      free((void*)AInfo->PanelItems[i].FindData.lpwszFileName);
    }
    free(AInfo->lpwszCurDir);
  }
}

static bool CompareDirs(const OwnPanelInfo *AInfo, const OwnPanelInfo *PInfo, bool bCompareAll, int ScanDepth, 
                        FILE * fpResults, int APanelRootDirLength, bool bActiveIsLeft );
static DWORD bufSize;
static HANDLE AFilter, PFilter;
static TCHAR *ABuf, *PBuf;

bool isnewline(int c)
{
  return (c == '\r' || c == '\n');
}

static TCHAR strMissingLeft[] = L"<<==";
static TCHAR strMissingRight[] = L"==>>";
static TCHAR strNewerLeft[] = L"--->";
static TCHAR strNewerRight[] = L"<---";
static TCHAR strCantDecide[] = L"<-->";
       
static void WriteFileComparisonResultToFile( 
                                             FILE * fpResults, 
                                             const TCHAR *CurDir, int RootDirLength, 
                                             const TCHAR *FileName, bool bDirectories,
                                             FILETIME *ATime, FILETIME *PTime,
                                             __int64 ASize, __int64 PSize,
                                             bool bActiveIsLeft
                                             )
{
    if (fpResults == 0 || fpResults == INVALID_HANDLE_VALUE)
		return;
    // Directories zajimaji(?) me just in case that differs completely with
    // on one page directory is on the other is not - otherwise they are 
    // automatically applied to files or. are irrelevant.
	TCHAR szDirection[5];
	// Hard to move to the left: If the file is inactive and active is the left,
	// or if the file is not a passive and active is the true
	if ((!ATime && bActiveIsLeft) || (!PTime && !bActiveIsLeft))
      lstrcpy(szDirection, strMissingLeft);
    // Hard to move right: If the file is inactive and active is true, or if the file is not a passive and active is the left
	else if ((!ATime && !bActiveIsLeft) || (!PTime && bActiveIsLeft))
      lstrcpy(szDirection, strMissingRight);
    else {
      // Where Directories gets here, so it is not interesting to me - that means that within the
      // directories are differences, but if they do matter, so as to differences in expression file
      if (bDirectories)
        return;
      bool bAktivniNovejsi = false;
      bool bStejneStare = false;
      if (ATime->dwHighDateTime > PTime->dwHighDateTime)
        bAktivniNovejsi = true;
  	  else if (ATime->dwHighDateTime < PTime->dwHighDateTime)
        bAktivniNovejsi = false;
	  else if (ATime->dwLowDateTime > PTime->dwLowDateTime)
	    bAktivniNovejsi = true;
	  else if (ATime->dwLowDateTime < PTime->dwLowDateTime)
	    bAktivniNovejsi = false;
      else
        bStejneStare = true;
      // Can not decide on the direction
      if (bStejneStare)
        lstrcpy(szDirection, strCantDecide);
      // Mecca moves to the right: If the active set of recent and active is the left, or if the newer set of passive and active is the true
      else if ((bAktivniNovejsi && bActiveIsLeft) || (!bAktivniNovejsi && !bActiveIsLeft))
        lstrcpy(szDirection, strNewerLeft);
      // Mecca to move to the left: When the active set of recent and active is true, or if the newer set of passive and active is the left
      else if ((bAktivniNovejsi && !bActiveIsLeft) || (!bAktivniNovejsi && bActiveIsLeft))
        lstrcpy(szDirection, strNewerRight);
      // This would never be
      else
        lstrcpy(szDirection, L"????");
    }
	TCHAR szASize[128], szPSize[128];
	FSF.itoa64(ASize, szASize, 10);
	FSF.itoa64(PSize, szPSize, 10);
	if (CurDir[RootDirLength])
	  if (bDirectories) 
  	    fwprintf(fpResults, L"%s\t%s\\%s\\*\n", szDirection, &CurDir[RootDirLength+1], FileName);
      else
	    fwprintf(fpResults, L"%s\t%s\\%s\n", szDirection, &CurDir[RootDirLength+1], FileName);
	else
	  if (bDirectories)
  	    fwprintf(fpResults, L"%s\t%s\\*\n", szDirection, FileName);
	  else
	    fwprintf(fpResults, L"%s\t%s\n", szDirection, FileName);
}

static int FileExists(const TCHAR * Filename, __int64 * pSize = NULL, DWORD * pAttrs = NULL) {
	int result = 0;
	HANDLE h = CreateFile(Filename, 0, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (h != INVALID_HANDLE_VALUE) {
		if (pSize != NULL) {
			DWORD hp;
			*pSize = GetFileSize(h, &hp);
			*pSize = (*pSize&0xffffffff) | ((__int64)hp << 32);
			if ((*pSize&0xffffffff) == 0xffffffff && GetLastError != NO_ERROR)
				*pSize = -1;
		}
		if (pAttrs != NULL) {
			DWORD nAttrs = GetFileAttributes(Filename);
			*pAttrs = (nAttrs == -1) ? 0 : nAttrs;
		}
		CloseHandle(h);
		result = 1;
	}
	return result;
}

struct SynchronizeFileCopyCallbackData {
	TCHAR * srcFileName;
	TCHAR * destFileName;
};

DWORD WINAPI SynchronizeFileCopyCallback(
		LARGE_INTEGER TotalFileSize,
    LARGE_INTEGER TotalBytesTransferred,
    LARGE_INTEGER StreamSize,
    LARGE_INTEGER StreamBytesTransferred,
    DWORD dwStreamNumber,
    DWORD dwCallbackReason,
    HANDLE hSourceFile,
    HANDLE hDestinationFile,
    LPVOID lpData)
{
	__int64 progress, max;
	progress = TotalBytesTransferred.QuadPart;
	max = TotalFileSize.QuadPart;
	if (lpData) {
		struct SynchronizeFileCopyCallbackData * data = (SynchronizeFileCopyCallbackData *)lpData;
		ShowMessage(data->srcFileName, data->destFileName, progress, max, MCopying, MCopyingTo);
	} else {
		ShowMessage(L"", L"", progress, max);
	}
	if (CheckForEsc())
		return PROGRESS_CANCEL;
	else
		return PROGRESS_CONTINUE;
}

static TCHAR * FormatFileSize(const TCHAR * filename) {
	static TCHAR result[65];
	__int64 sz;
	lstrcpy(result, L"");
	if (FileExists(filename, &sz)) {
		if (sz >= 0) {
#ifdef FORMAT_INT_BY_LOCALE
			TCHAR num[65];
			FSF.itoa64(sz, num, 10);
			if (!GetNumberFormat(LOCALE_USER_DEFAULT, 0, num, NULL, result, sizeof(result)))
				lstrcpy(result, L"");
#else
			FSF.itoa64(sz, result, 10);
#endif
		}
	}
	return result;
}

static TCHAR * FormatFileDateTime(const TCHAR * filename) {
	static TCHAR result[65];
	TCHAR cDate[32], cTime[32];
	HANDLE handle;
	FILETIME fileTime;
	SYSTEMTIME systemTime;
	lstrcpy(result, L"");
	handle = CreateFile(filename, 0, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (handle != INVALID_HANDLE_VALUE) {
		if (GetFileTime(handle, NULL, NULL, &fileTime)) {
			if (FileTimeToSystemTime(&fileTime, &systemTime)) {
				if (GetDateFormat(LOCALE_USER_DEFAULT, 0, &systemTime, L"dd.MM.yyyy", cDate, ArraySize(cDate))) {
					if (GetTimeFormat(LOCALE_USER_DEFAULT, 0, &systemTime, L"HH:mm:ss", cTime, ArraySize(cTime))) {
						wsprintf(result, L"%s %s", cDate, cTime);
					}
				}
			}
		}
		CloseHandle(handle);
	}
	return result;
}

enum QueryOverwriteFileResult {
	QOF_ABORT,
	QOF_OVERWRITE,
	QOF_OVERWRITEALL,
	QOF_SKIP,
	QOF_SKIPALL
};
enum QueryOverwriteType {
	QOT_EXISTING,
	QOT_READONLY,
};

static QueryOverwriteFileResult QueryOverwriteFile(const int OverwriteType, const int MovingLeft, const TCHAR * strSourceFilename, const TCHAR * strDestinationFilename, const TCHAR * DisplayFilename = NULL)
{
	TCHAR cNew[67], cExisting[67], cFilename[NM];
	wcsncpy(cFilename, (DisplayFilename ? DisplayFilename : strDestinationFilename), ArraySize(cFilename));
	cFilename[ArraySize(cFilename)-1] = 0;
	FSF.TruncPathStr(cFilename, 66);
	wsprintf(cNew, L"%-30.30s %15.15s %19.19s", GetMsg(MNew), FormatFileSize(strSourceFilename), FormatFileDateTime(strSourceFilename));
	wsprintf(cExisting, L"%-30.30s %15.15s %19.19s", GetMsg(MExisting), FormatFileSize(strDestinationFilename), FormatFileDateTime(strDestinationFilename));
#ifdef OVERWRITE_DIALOG_COMPLEX
	struct InitDialogItem {
	  int Type;
	  int X1;
	  int Y1;
	  int X2;
	  int Y2;
	  int Focus;
	  DWORD_PTR Selected;
	  unsigned int Flags;
	  int DefaultButton;
	  const TCHAR *Data;
	} InitItems[] = {
		//       Type       x1  y1  x2  y2 Foc Sel Flg Def Txt
    /*  0 */ DI_DOUBLEBOX,  3,  1, 72,  9, 0, 0, 0,                 0, GetMsg(MWarning),
    /*  1 */ DI_TEXT,       5,  2, 70,  2, 0, 0, DIF_CENTERTEXT,    0,
			(OverwriteType == QOT_EXISTING) ?
				GetMsg(MovingLeft ? MLeftFileAlreadyExists : MRightFileAlreadyExists) :
				GetMsg(MovingLeft ? MLeftFileReadonly : MRightFileReadonly),
    /*  2 */ DI_TEXT,       5,  3, 70,  2, 0, 0, DIF_SHOWAMPERSAND, 0, cFilename,
    /*  3 */ DI_TEXT,       4,  4, 71,  3, 0, 0, DIF_SEPARATOR,     0, L"",
    /*  4 */ DI_TEXT,       5,  5, 70,  4, 0, 0, DIF_SHOWAMPERSAND, 0, cNew,
    /*  5 */ DI_TEXT,       5,  6, 70,  5, 0, 0, DIF_SHOWAMPERSAND, 0, cExisting,
    /*  6 */ DI_TEXT,       4,  7, 71,  6, 0, 0, DIF_SEPARATOR,     0, L"",
    /*  7 */ DI_BUTTON,     0,  8,  0,  8, 1, 0, DIF_CENTERGROUP,   0, GetMsg(MOverwrite),
    /*  8 */ DI_BUTTON,     0,  8,  0,  8, 0, 0, DIF_CENTERGROUP,   0, GetMsg(MOverwriteAll),
    /*  9 */ DI_BUTTON,     0,  8,  0,  8, 0, 0, DIF_CENTERGROUP,   0, GetMsg(MSkip),
    /* 10 */ DI_BUTTON,     0,  8,  0,  8, 0, 0, DIF_CENTERGROUP,   0, GetMsg(MSkipAll),
    /* 11 */ DI_BUTTON,     0,  8,  0,  8, 0, 0, DIF_CENTERGROUP,   0, GetMsg(MCancel)
  };
  enum DialogResults {
		DR_OVERWRITE = 7,
		DR_OVERWRITE_ALL = 8,
		DR_SKIP = 9,
		DR_SKIP_ALL = 10,
	};
  struct FarDialogItem DialogItems[ArraySize(InitItems)];
  struct InitDialogItem *ii = InitItems;
  struct FarDialogItem *di = DialogItems;
  for (int i = 0; i < ArraySize(InitItems); i++, ii++, di++) {
		di->Type = ii->Type;
		di->X1 = ii->X1;
		di->Y1 = ii->Y1;
		di->X2 = ii->X2;
		di->Y2 = ii->Y2;
		di->Focus = ii->Focus;
		di->Param.Selected = ii->Selected;
		di->Flags = ii->Flags;
		di->DefaultButton = ii->DefaultButton;
		di->PtrData = ii->Data;
	}
  HANDLE hDlg = Info.DialogInit(Info.ModuleNumber, -1, -1, 76, 11, NULL, 
	                          DialogItems, ArraySize(DialogItems), 0, FDLG_WARNING, 
	                          NULL, 0);
  int ExitCode;
  if (hDlg == INVALID_HANDLE_VALUE) {
    ExitCode = DR_SKIP_ALL;
  } else {
    ExitCode = Info.DialogRun(hDlg);
    Info.DialogFree(hDlg);
  }
#else
  const char * MessageLines[] = {
		GetMsg(MWarning),
		GetMsg(MovingLeft ? MLeftFileAlreadyExists : MRightFileAlreadyExists),
		cFilename,
		"",
		cNew,
		cExisting,
		"",
		GetMsg(MOverwrite),
		GetMsg(MOverwriteAll),
		GetMsg(MSkip),
		GetMsg(MSkipAll),
		GetMsg(MCancel)
	};
  enum DialogResults {
		DR_OVERWRITE = 0,
		DR_OVERWRITE_ALL = 1,
		DR_SKIP = 2,
		DR_SKIP_ALL = 3,
	};
  int ExitCode = Info.Message(Info.ModuleNumber, FMSG_WARNING|FMSG_LEFTALIGN, NULL, MessageLines, ArraySize(MessageLines)), 5);
#endif
  QueryOverwriteFileResult result = QOF_ABORT;
  switch (ExitCode) {
		case DR_OVERWRITE:
			result = QOF_OVERWRITE;
			break;
		case DR_OVERWRITE_ALL:
			result = QOF_OVERWRITEALL;
			break;
		case DR_SKIP:
			result = QOF_SKIP;
			break;
		case DR_SKIP_ALL:
			result = QOF_SKIPALL;
			break;
	}
	return result;
}

static int SynchronizeFile(const TCHAR * srcFileName, const TCHAR * destFileName, struct SyncOptions * options, int direction, const TCHAR * displayFileName = NULL)
{
	int ok = 1;
	__int64 progress, max;
	progress = 0;
	max = 1;
	if (srcFileName && destFileName && FileExists(srcFileName, &max)) {
		// Show what with it actually copies
		ShowMessage(srcFileName, destFileName, progress, max, MCopying, MCopyingTo);
		// If the target file exists, so maybe I'll have to ask you to overwrite
		int doCopy = 1, doAttrs = 0; DWORD nDestAttrs = 0;
		if (FileExists(destFileName, NULL, &nDestAttrs))
		{
			// Overwrite confirmation
			if (((direction < 0) && options->AskOverwriteLeft) || ((direction > 0) && options->AskOverwriteRight))
			{
				switch (QueryOverwriteFile(QOT_EXISTING, direction < 0, srcFileName, destFileName, displayFileName)) {
					case QOF_OVERWRITE:
						doCopy = 1;
						break;
					case QOF_OVERWRITEALL:
						doCopy = 1;
						if (direction < 0)
							options->AskOverwriteLeft = 0;
						else
							options->AskOverwriteRight = 0;
						break;
					case QOF_SKIP:
						doCopy = 0;
						break;
					case QOF_SKIPALL:
						doCopy = 0;
						if (direction < 0)
							options->CopyLeft = 0;
						else
							options->CopyRight = 0;
						break;
					default:
						doCopy = 0;
						ok = 0;
						break;
				}
			}
			// ReadOnly overwrite confirmation
			if ((nDestAttrs & FILE_ATTRIBUTE_READONLY))
			{
				if (((direction < 0) && options->SkipReadonlyLeft) || ((direction > 0) && options->SkipReadonlyRight))
				{
					doCopy = 0;
				}
				else if (((direction < 0) && options->AskReadonlyLeft) || ((direction > 0) && options->AskOverwriteRight))
				{
					switch (QueryOverwriteFile(QOT_READONLY, direction < 0, srcFileName, destFileName, displayFileName)) {
						case QOF_OVERWRITE:
							doCopy = 1;
							break;
						case QOF_OVERWRITEALL:
							doCopy = 1;
							if (direction < 0)
								options->AskReadonlyLeft = 0;
							else
								options->AskReadonlyRight = 0;
							break;
						case QOF_SKIP:
							doCopy = 0;
							break;
						case QOF_SKIPALL:
							doCopy = 0;
							if (direction < 0)
								options->SkipReadonlyLeft = 0;
							else
								options->SkipReadonlyRight = 0;
							break;
						default:
							doCopy = 0;
							ok = 0;
							break;
					}
				}
				if (doCopy)
					SetFileAttributes(destFileName, nDestAttrs & ~(FILE_ATTRIBUTE_READONLY));
			}
		}
		if (doCopy) {
			struct SynchronizeFileCopyCallbackData copyData;
			copyData.srcFileName = (TCHAR *)srcFileName;
			copyData.destFileName = (TCHAR *)destFileName;
			if (!CopyFileEx(srcFileName, destFileName, SynchronizeFileCopyCallback, &copyData, NULL, 0)) {
				DWORD dwErr = GetLastError();
				const TCHAR * MessageLines[] = {
					GetMsg(MWarning),
					GetMsg(MFailedToOverwriteFile),
					destFileName
				};
				if (bBrokenByEsc || 
					Info.Message(Info.ModuleNumber, FMSG_WARNING|FMSG_MB_OKCANCEL, NULL, MessageLines, ArraySize(MessageLines), 0)==1) 
				{
					ok = 0;
				}
			}
		}
	}
	return ok;
}

static int SynchronizeDirectory(const TCHAR * srcDirName, const TCHAR * destDirName, struct SyncOptions * options, int direction)
{
	// The input source and expect Target directory without a final slash
	// If the target directory does not exist, create it
//SHOW_MESSAGE("Synchronize directory", srcDirName, destDirName);
	WIN32_FIND_DATA fnd;
	HANDLE handle = FindFirstFile(destDirName, &fnd);
	if (handle == INVALID_HANDLE_VALUE) {
		if (!CreateDirectory(destDirName, NULL))
			return 0;
	} else {
		FindClose(handle);
		if (!(fnd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			return 0;
	}
	// Now I will recursively go through all the files in the directory
	int result = 1;
	TCHAR WildCard[NM];
	_sntprintf(WildCard, ArraySize(WildCard), L"%s\\*.*", srcDirName);
	if (handle = FindFirstFile(WildCard, &fnd)) {
		do {
			TCHAR NewSrc[NM], NewDest[NM];
			_sntprintf(NewSrc, ArraySize(NewSrc), L"%s\\%s", srcDirName, fnd.cFileName);
			_sntprintf(NewDest, ArraySize(NewDest), L"%s\\%s", destDirName, fnd.cFileName);
//SHOW_MESSAGE("Synchronize directory LOOP", "Wildcard", WildCard, "Found", fnd.cFileName, "New names", NewSrc, NewDest);
			if (fnd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				if ((_wcsicmp(fnd.cFileName, L".") != 0) && (_wcsicmp(fnd.cFileName, L"..") != 0)) {
					if (!SynchronizeDirectory(NewSrc, NewDest, options, direction)) {
						result = 0;
					}
				}
			} else {
				if (!SynchronizeFile(NewSrc, NewDest, options, direction)) {
					result = 0;
				}
			}
		} while (result && FindNextFile(handle, &fnd));
		FindClose(handle);	
	}
	return result;
}

static void Synchronize(const TCHAR * LeftPath, const TCHAR * RightPath, const TCHAR * ReportFileName, struct SyncOptions * options)
{
  HANDLE hScreen = Info.SaveScreen(0, 0, -1, -1);
  hConInp = CreateFile(L"CONIN$", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
  FILE * fpResults = _wfopen(ReportFileName, L"r,ccs=UNICODE");
  if (lstrlen(LeftPath) && lstrlen(RightPath) && fpResults) {
		TCHAR ReportLine[2*NM];
		int ok = 1;
		bStart = true;
		while (ok && fgetws(ReportLine, ArraySize(ReportLine), fpResults)) {
			int rlLen = lstrlen(ReportLine);
			if (ReportLine[rlLen-1] = L'\n')
				ReportLine[rlLen-1] = 0;
			// I will try to synchronize the given set of this line
			int dir = 0;
			int len = 0;
			if (wcsncmp(ReportLine, strMissingLeft, len = lstrlen(strMissingLeft)) == 0)
				dir = -1;
			else if (wcsncmp(ReportLine, strMissingRight, len = lstrlen(strMissingRight)) == 0)
			  dir = 1;
			else if (wcsncmp(ReportLine, strNewerRight, len = lstrlen(strNewerRight)) == 0)
				dir = -1;
			else if (wcsncmp(ReportLine, strNewerLeft, len = lstrlen(strNewerLeft)) == 0)
				dir = 1;
			if (dir != 0) {
				if (((dir < 0) && options->CopyLeft) || ((dir > 0) && options->CopyRight)) {
					TCHAR * p = &(ReportLine[len]);
					while (*p && ((*p == ' ') || (*p == '\t')))
						p++;
					if (*p) {
						//Special case can occur if the line ends in "\\*" - This indicates that
						//to that directory on the second panel do not exist and the user can copy
						//set cell. Otherwise, it copies only one file.
						len = lstrlen(p);
						int directory = (p[len-1] == '*');
						if (directory) {
							p[len-1] = 0;
							if ((len > 2) && (p[len-2] == '\\'))
								p[len-2] = 0;
						}
						TCHAR srcFileName[NM+1], destFileName[NM+1];
						wcsncpy(srcFileName, BuildFullFilename((dir < 0) ? RightPath : LeftPath, p), ArraySize(srcFileName));
						wcsncpy(destFileName, BuildFullFilename((dir < 0) ? LeftPath : RightPath, p), ArraySize(destFileName));
						if (directory) {
							if (!SynchronizeDirectory(srcFileName, destFileName, options, dir))
								ok = 0;
						} else {
							if (!SynchronizeFile(srcFileName, destFileName, options, dir, p))
								ok = 0;
						}
					}
				}
			}
		}
	if (fpResults)
		fclose(fpResults);
	}
  DeleteFile(ReportFileName);
  CloseHandle(hConInp);
  Info.RestoreScreen(hScreen);
}

static int GetSyncOptions(struct SyncOptions * options, const TCHAR * ReportFileName) {
	options->CopyLeft = 0;
	options->CopyRight = 0;
	options->AskOverwriteLeft = ( Info.AdvControl(Info.ModuleNumber, ACTL_GETCONFIRMATIONS, NULL) & FCS_COPYOVERWRITE );
	options->AskOverwriteRight = ( Info.AdvControl(Info.ModuleNumber, ACTL_GETCONFIRMATIONS, NULL) & FCS_COPYOVERWRITE );
	options->AskReadonlyLeft = options->AskReadonlyRight = 1;
	options->SkipReadonlyLeft = options->SkipReadonlyRight = 0;
  FILE * fpResults = _wfopen(ReportFileName, L"r,ccs=UNICODE");
  if (fpResults) {
		TCHAR ReportLine[2*NM];
		while ((!(options->CopyLeft && options->CopyRight)) && fgetws(ReportLine, ArraySize(ReportLine), fpResults)) {
			if (wcsncmp(ReportLine, strMissingLeft, lstrlen(strMissingLeft)) == 0)
				options->CopyLeft = 1;
			else if (wcsncmp(ReportLine, strMissingRight, lstrlen(strMissingRight)) == 0)
				options->CopyRight = 1;
			else if (wcsncmp(ReportLine, strNewerRight, lstrlen(strNewerRight)) == 0)
				options->CopyLeft = 1;
			else if (wcsncmp(ReportLine, strNewerLeft, lstrlen(strNewerLeft)) == 0)
				options->CopyRight = 1;
		}
		fclose(fpResults);
	}
	if (options->CopyLeft || options->CopyRight) {
		// Are there any files in sync. So you make a dialogue, where I ask what I do.
		struct InitDialogItem {
		  int Type;
		  int X1;
		  int Y1;
		  int X2;
		  int Y2;
		  int Focus;
		  DWORD_PTR Selected;
		  unsigned int Flags;
		  int DefaultButton;
		  const TCHAR *Data;
		} InitItems[] = {
			//   Type           x1  y1 x2   y2 Foc Sel                        Flg                                    Def Txt
	    /*  0 */ DI_DOUBLEBOX,  3,  1, 42,  8, 0, 0,                          0,                                     0, GetMsg(MCmpTitle),
	    /*  1 */ DI_CHECKBOX,   5,  2, 40,  2, 1, options->CopyLeft,          options->CopyLeft  ? 0 : DIF_DISABLE,  0, GetMsg(MCopyRightToLeft),
	    /*  2 */ DI_CHECKBOX,   8,  3, 40,  3, 0, options->AskOverwriteLeft,  options->CopyLeft  ? 0 : DIF_DISABLE,  0, GetMsg(MOverwriteRightToLeft),
	    /*  3 */ DI_CHECKBOX,   5,  4, 40,  4, 0, options->CopyRight,         options->CopyRight ? 0 : DIF_DISABLE,  0, GetMsg(MCopyLeftToRight),
	    /*  4 */ DI_CHECKBOX,   8,  5, 40,  5, 0, options->AskOverwriteRight, options->CopyRight ? 0 : DIF_DISABLE,  0, GetMsg(MOverwriteLeftToRight),
	    /*  5 */ DI_TEXT,       4,  6, 41,  6, 0, 0,                          DIF_SEPARATOR,                         0, L"",
	    /*  6 */ DI_BUTTON,     0,  7,  0,  7, 0, 0,                          DIF_CENTERGROUP,                       1, GetMsg(MOK),
	    /*  7 */ DI_BUTTON,     0,  7,  0,  7, 0, 0,                          DIF_CENTERGROUP,                       0, GetMsg(MCancel)
	  };
	  enum DialogResults {
			DR_CB_COPYLEFT = 1,
			DR_CB_OVERWRITELEFT = 2,
			DR_CB_COPYRIGHT = 3,
			DR_CB_OVERWRITERIGHT = 4,
			DR_OK = 6,
		};
	  struct FarDialogItem DialogItems[ArraySize(InitItems)];
	  struct InitDialogItem *ii = InitItems;
	  struct FarDialogItem *di = DialogItems;
	  for (int i = 0; i < ArraySize(InitItems); i++, ii++, di++) {
			di->Type = ii->Type;
			di->X1 = ii->X1;
			di->Y1 = ii->Y1;
			di->X2 = ii->X2;
			di->Y2 = ii->Y2;
			di->Focus = ii->Focus;
			di->Param.Selected = ii->Selected;
			di->Flags = ii->Flags;
			di->DefaultButton = ii->DefaultButton;
			di->PtrData = ii->Data;
		}


	  HANDLE hDlg = Info.DialogInit(Info.ModuleNumber, -1, -1, 46, 10, NULL, DialogItems, ArraySize(DialogItems), 0, 0, NULL, 0);
        if (hDlg == INVALID_HANDLE_VALUE)
          return 0;

        int ExitCode = Info.DialogRun(hDlg);
        Info.DialogFree(hDlg);

		if (ExitCode == DR_OK) {
			options->CopyLeft = DialogItems[DR_CB_COPYLEFT].Param.Selected;
			options->AskOverwriteLeft = DialogItems[DR_CB_OVERWRITELEFT].Param.Selected;
			options->CopyRight = DialogItems[DR_CB_COPYRIGHT].Param.Selected;
			options->AskOverwriteRight = DialogItems[DR_CB_OVERWRITERIGHT].Param.Selected;
			return 1;
		}
	}
	return 0;
}


/****************************************************************************
 * Compare attribuites and stuff for elements with same names (files of
 * subfolders).
 * Return true if they match.
 ****************************************************************************/
static bool CompareFiles( const FAR_FIND_DATA *AData, const FAR_FIND_DATA *PData,
                          const TCHAR *ACurDir, const TCHAR *PCurDir, int ScanDepth,
						  FILE * fpResults, int APanelRootDirLength, bool bActiveIsLeft )
{
  if (AData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
  {
    // Here we compare 2 subfolders
    if (Opt.ProcessSubfolders)
    {
      if (Opt.UseMaxScanDepth && Opt.MaxScanDepth<ScanDepth+1)
        return true;
      // Make file lists in subfolders
      OwnPanelInfo AInfo, PInfo;
      memset(&AInfo, 0, sizeof(AInfo));
      memset(&PInfo, 0, sizeof(PInfo));
      bool bEqual;
      if(   !GetDirList(&AInfo, BuildFullFilename(ACurDir, AData->lpwszFileName))
         || !GetDirList(&PInfo, BuildFullFilename(PCurDir, PData->lpwszFileName)))
      {
        //bBrokenByEsc = true; // User interrupt or read error
        bEqual = false; // Stop comparison
      }
      else
        bEqual = CompareDirs(&AInfo, &PInfo, true, ScanDepth+1, fpResults, APanelRootDirLength, bActiveIsLeft);
      FreeDirList(&AInfo);
      FreeDirList(&PInfo);
      return bEqual;
    }
  }
  else
  {
    // Here we compare 2 files
    if (Opt.CompareSize)
      if (AData->nFileSize != PData->nFileSize)
        return false;
    if (Opt.CompareTime)
    {
      if (Opt.LowPrecisionTime || Opt.IgnorePossibleTimeZoneDifferences)
      {
        union {
          __int64 num;
          struct
          {
            DWORD lo;
            DWORD hi;
          } hilo;
        } Precision, Difference, TimeDelta, temp;

        Precision.hilo.hi = 0;
        Precision.hilo.lo = Opt.LowPrecisionTime ? 20000000 : 0; //2s or 0s
        Difference.num = _i64(9000000000); //15m

        if (AData->ftLastWriteTime.dwHighDateTime > PData->ftLastWriteTime.dwHighDateTime)
        {
          TimeDelta.hilo.hi = AData->ftLastWriteTime.dwHighDateTime - PData->ftLastWriteTime.dwHighDateTime;
          TimeDelta.hilo.lo = AData->ftLastWriteTime.dwLowDateTime - PData->ftLastWriteTime.dwLowDateTime;
          if (TimeDelta.hilo.lo > AData->ftLastWriteTime.dwLowDateTime)
            --TimeDelta.hilo.hi;
        }
        else
        {
          if (AData->ftLastWriteTime.dwHighDateTime == PData->ftLastWriteTime.dwHighDateTime)
          {
            TimeDelta.hilo.hi = 0;
            TimeDelta.hilo.lo = max(PData->ftLastWriteTime.dwLowDateTime,AData->ftLastWriteTime.dwLowDateTime)-
                                min(PData->ftLastWriteTime.dwLowDateTime,AData->ftLastWriteTime.dwLowDateTime);
          }
          else
          {
            TimeDelta.hilo.hi = PData->ftLastWriteTime.dwHighDateTime - AData->ftLastWriteTime.dwHighDateTime;
            TimeDelta.hilo.lo = PData->ftLastWriteTime.dwLowDateTime - AData->ftLastWriteTime.dwLowDateTime;
            if (TimeDelta.hilo.lo > PData->ftLastWriteTime.dwLowDateTime)
              --TimeDelta.hilo.hi;
          }
        }

        //ignore differences which are equal or less than 26 hours.
        if (Opt.IgnorePossibleTimeZoneDifferences)
        {
          int counter=0;
          while (TimeDelta.hilo.hi > Difference.hilo.hi && counter<=26*4)
          {
            temp.hilo.lo = TimeDelta.hilo.lo - Difference.hilo.lo;
            temp.hilo.hi = TimeDelta.hilo.hi - Difference.hilo.hi;
            if (temp.hilo.lo > TimeDelta.hilo.lo)
              --temp.hilo.hi;
            TimeDelta.hilo.lo = temp.hilo.lo;
            TimeDelta.hilo.hi = temp.hilo.hi;
            ++counter;
          }
          if (counter<=26*4 && TimeDelta.hilo.hi == Difference.hilo.hi)
          {
            TimeDelta.hilo.hi = 0;
            TimeDelta.hilo.lo = max(TimeDelta.hilo.lo,Difference.hilo.lo) - min(TimeDelta.hilo.lo,Difference.hilo.lo);
          }
        }

        if (Precision.hilo.hi < TimeDelta.hilo.hi ||
            (Precision.hilo.hi == TimeDelta.hilo.hi && Precision.hilo.lo < TimeDelta.hilo.lo))
          return false;
      }
      else if (AData->ftLastWriteTime.dwLowDateTime != PData->ftLastWriteTime.dwLowDateTime ||
               AData->ftLastWriteTime.dwHighDateTime != PData->ftLastWriteTime.dwHighDateTime)
        return false;
    }
    if (Opt.CompareContents)
    {
      HANDLE hFileA, hFileP;
      TCHAR cpFileA[MAX_PATH], cpFileP[MAX_PATH];
      __int64 Progress, Max;
      Progress = 0;
	  Max = AData->nFileSize;
      ShowMessage(lstrcpy(cpFileA, BuildFullFilename(ACurDir, AData->lpwszFileName)),
                  lstrcpy(cpFileP, BuildFullFilename(PCurDir, PData->lpwszFileName)),
                  Progress, Max);
      if ((hFileA = CreateFile(cpFileA, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                               OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL)) == INVALID_HANDLE_VALUE)
      {
        bOpenFail = true;
        return false;
      }
      if ((hFileP = CreateFile(cpFileP, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                               OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL)) == INVALID_HANDLE_VALUE)
      {
        CloseHandle(hFileA);
        bOpenFail = true;
        return false;
      }

      bool bEqual = true;
      DWORD ReadSizeA, ReadSizeP;
      if (!Opt.CompareContentsIgnore)
      {
        do
        {
          ShowMessage(cpFileA, cpFileP, Progress, Max);
          if (   CheckForEsc()
              || !ReadFile(hFileA, ABuf, bufSize, &ReadSizeA, NULL)
              || !ReadFile(hFileP, PBuf, bufSize, &ReadSizeP, NULL)
              || ReadSizeA != ReadSizeP)
          {
            bEqual = false;
            break;
          }
          if (!ReadSizeA)
            break;
          if (memcmp(ABuf, PBuf, ReadSizeA))
          {
            bEqual = false;
            break;
          }
          Progress += ReadSizeA;
        } while (ReadSizeA == bufSize);
      }
      else
      {
        ReadSizeA=1;
        ReadSizeP=1;
        TCHAR *PtrA=ABuf+ReadSizeA, *PtrP=PBuf+ReadSizeP;
        bool bExpectNewLineA = false;
        bool bExpectNewLineP = false;
        while (true)
        {
          while (PtrA >= ABuf+ReadSizeA && ReadSizeA)
          {
            if (CheckForEsc() || !ReadFile(hFileA, ABuf, bufSize, &ReadSizeA, NULL))
            {
              bEqual = false;
              break;
            }
            PtrA=ABuf;
          }

          if (!bEqual)
            break;

          while (PtrP >= PBuf+ReadSizeP && ReadSizeP)
          {
            if (CheckForEsc() || !ReadFile(hFileP, PBuf, bufSize, &ReadSizeP, NULL))
            {
              bEqual = false;
              break;
            }
            PtrP=PBuf;
          }

          if (!bEqual || (!ReadSizeP && !ReadSizeA))
            break;

          if (Opt.IgnoreWhitespace)
          {
            while (PtrA < ABuf+ReadSizeA && PtrP < PBuf+ReadSizeP && !isspace(*PtrA) && !isspace(*PtrP))
            {
              if (*PtrA != *PtrP)
              {
                bEqual = false;
                break;
              }
              ++PtrA;
              ++PtrP;
            }

            if (!bEqual)
              break;

            while (PtrA < ABuf+ReadSizeA && isspace(*PtrA))
              ++PtrA;

            while (PtrP < PBuf+ReadSizeP && isspace(*PtrP))
              ++PtrP;
          }
          else
          {
            if (bExpectNewLineA)
            {
              bExpectNewLineA = false;
              if (PtrA < ABuf+ReadSizeA && *PtrA == '\n')
                ++PtrA;
            }

            if (bExpectNewLineP)
            {
              bExpectNewLineP = false;
              if (PtrP < PBuf+ReadSizeP && *PtrP == '\n')
                ++PtrP;
            }

            while (PtrA < ABuf+ReadSizeA && PtrP < PBuf+ReadSizeP && !isnewline(*PtrA) && !isnewline(*PtrP))
            {
              if (*PtrA != *PtrP)
              {
                bEqual = false;
                break;
              }
              ++PtrA;
              ++PtrP;
            }

            if (!bEqual)
              break;

            if (PtrA < ABuf+ReadSizeA && PtrP < PBuf+ReadSizeP && (!isnewline(*PtrA) || !isnewline(*PtrP)))
            {
              bEqual = false;
              break;
            }

            if (PtrA < ABuf+ReadSizeA && PtrP < PBuf+ReadSizeP)
            {
              if (*PtrA == '\r')
                bExpectNewLineA = true;

              if (*PtrP == '\r')
                bExpectNewLineP = true;

              ++PtrA;
              ++PtrP;
            }
          }

          if (PtrA < ABuf+ReadSizeA && !ReadSizeP)
          {
            bEqual = false;
            break;
          }

          if (PtrP < PBuf+ReadSizeP && !ReadSizeA)
          {
            bEqual = false;
            break;
          }
        }
      }

      CloseHandle(hFileA);
      CloseHandle(hFileP);
      return bEqual;
    }
  }
  return true;
}

/****************************************************************************
 * Compare two subfolders described in structures AInfo и PInfo.
 * Return true if they are equal.
 * bCompareAll defines whether all files are compared and PPIF_SELECTED is set
 * (bCompareAll == true)
 * or just false is returned at first mismatch (bCompareAll == false).
 ****************************************************************************/
static bool CompareDirs(const OwnPanelInfo *AInfo, const OwnPanelInfo *PInfo, bool bCompareAll, int ScanDepth, 
                        FILE * fpResults, int APanelRootDirLength, bool bActiveIsLeft )
{
  // Build file indices for quick comparison
  struct FileIndex sfiA, sfiP;
  TCHAR DirA[MAX_PATH], DirP[MAX_PATH];
  __int64 Progress, Max;
  Progress = 0;
  Max = 1;
  ShowMessage(lstrcpy(DirA, BuildFullFilename(AInfo->lpwszCurDir, L"*")),
              lstrcpy(DirP, BuildFullFilename(PInfo->lpwszCurDir, L"*")),
              Progress, Max);
  if (!BuildPanelIndex(AInfo, &sfiA, AFilter) || !BuildPanelIndex(PInfo, &sfiP, PFilter))
  {
    const TCHAR *MsgItems[] = {
      GetMsg(MNoMemTitle),
      GetMsg(MNoMemBody),
      GetMsg(MOK)
    };
    Info.Message(Info.ModuleNumber, FMSG_WARNING, NULL,
                 MsgItems, ArraySize(MsgItems), 1);
    bBrokenByEsc = true;
    FreePanelIndex(&sfiA);
    FreePanelIndex(&sfiP);
    return true;
  }
  bool bDifferenceNotFound = true;
  int i = sfiA.iCount - 1, j = sfiP.iCount - 1;
  while (i >= 0 && j >= 0 && (bDifferenceNotFound || bCompareAll) && !bBrokenByEsc)
  {
    const int iMaxCounter = 256;
    static int iCounter = iMaxCounter;
    if (!--iCounter)
    {
      iCounter = iMaxCounter;
      if (CheckForEsc())
        break;
    }
    switch (PICompare(&sfiA.ppi[i], &sfiP.ppi[j]))
    {
      case 0: // Names have matched - check the rest
        if (CompareFiles(&sfiA.ppi[i]->FindData, &sfiP.ppi[j]->FindData, AInfo->lpwszCurDir, PInfo->lpwszCurDir, ScanDepth, fpResults, APanelRootDirLength, bActiveIsLeft))
        { // The rest has matched too
          sfiA.ppi[i--]->Flags &= ~PPIF_SELECTED;
          sfiP.ppi[j--]->Flags &= ~PPIF_SELECTED;
        }
        else
        {
          // Files are different
          __int64 ASize, PSize;
          ASize = sfiA.ppi[i]->FindData.nFileSize;
          PSize = sfiP.ppi[j]->FindData.nFileSize;
          WriteFileComparisonResultToFile(
              fpResults, 
			  AInfo->lpwszCurDir, APanelRootDirLength,
			  sfiA.ppi[i]->FindData.lpwszFileName, (sfiA.ppi[i]->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && (sfiP.ppi[j]->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY),
              &sfiA.ppi[i]->FindData.ftLastWriteTime, &sfiP.ppi[j]->FindData.ftLastWriteTime,
              ASize, PSize,
              bActiveIsLeft
              );
          bDifferenceNotFound = false;
          sfiA.ppi[i--]->Flags |= PPIF_SELECTED;
          sfiP.ppi[j--]->Flags |= PPIF_SELECTED;
        }
        break;
      case 1: // Element sfiA.ppi[i] has no elements with the same name in sfiP.ppi
        // Passive file does not exist
        __int64 ASize;
        ASize = sfiA.ppi[i]->FindData.nFileSize;
        WriteFileComparisonResultToFile(
            fpResults, 
            AInfo->lpwszCurDir, APanelRootDirLength,
            sfiA.ppi[i]->FindData.lpwszFileName, (sfiA.ppi[i]->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)!=0,
            &sfiA.ppi[i]->FindData.ftLastWriteTime, 0,
            ASize, -1,
            bActiveIsLeft
            );
        bDifferenceNotFound = false;
        sfiA.ppi[i--]->Flags |= PPIF_SELECTED;
        break;
      case -1: // Element sfiP.ppi[j] has no elements with the same name in sfiA.ppi
        // Active file does not exist
        __int64 PSize;
        PSize = sfiP.ppi[j]->FindData.nFileSize;
        WriteFileComparisonResultToFile(
            fpResults, 
            AInfo->lpwszCurDir, APanelRootDirLength,
            sfiP.ppi[j]->FindData.lpwszFileName, (sfiP.ppi[j]->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)!=0,
            0, &sfiP.ppi[j]->FindData.ftLastWriteTime,
            -1, PSize,
            bActiveIsLeft
            );
        bDifferenceNotFound = false;
        sfiP.ppi[j--]->Flags |= PPIF_SELECTED;
        break;
    }
  }
  if (!bBrokenByEsc)
  {
    // The comparison is over. Select skipped elements in arrays
    if (i >= 0)
    {
      bDifferenceNotFound = false;
      if (bCompareAll)
        for (; i >= 0; i--) {
          // Pasivni soubor neexistuje
          __int64 ASize;
          ASize = sfiA.ppi[i]->FindData.nFileSize;
          WriteFileComparisonResultToFile(
              fpResults, 
              AInfo->lpwszCurDir, APanelRootDirLength,
              sfiA.ppi[i]->FindData.lpwszFileName, (sfiA.ppi[i]->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)!=0,
              &sfiA.ppi[i]->FindData.ftLastWriteTime, 0,
              ASize, -1,
              bActiveIsLeft
              );
          sfiA.ppi[i]->Flags |= PPIF_SELECTED;
        }
    }
    if (j >= 0)
    {
      bDifferenceNotFound = false;
      if (bCompareAll)
        for (; j >= 0; j--) {
          // Active file does not exist
          __int64 PSize;
          PSize = sfiP.ppi[j]->FindData.nFileSize;
          WriteFileComparisonResultToFile(
              fpResults, 
              AInfo->lpwszCurDir, APanelRootDirLength,
              sfiP.ppi[j]->FindData.lpwszFileName, (sfiP.ppi[j]->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)!=0,
              0, &sfiP.ppi[j]->FindData.ftLastWriteTime,
              -1, PSize,
              bActiveIsLeft
              );
          sfiP.ppi[j]->Flags |= PPIF_SELECTED;
        }
    }
  }
  FreePanelIndex(&sfiA);
  FreePanelIndex(&sfiP);
  return bDifferenceNotFound;
}

/****************************************************************************
 ***************************** Exported functions ***************************
 ****************************************************************************/

static bool bOldFAR = false;

/****************************************************************************
 * This plugin's function is called by FAR at first place
 ****************************************************************************/
void WINAPI EXP_NAME(SetStartupInfo)(const struct PluginStartupInfo *Info)
{
  const TCHAR *cpPlugRegKey = L"\\AdvCompare";
  ::Info = *Info;

  if (Info->StructSize >= (int)sizeof(struct PluginStartupInfo))
    FSF = *Info->FSF;
  else
    bOldFAR = true;

  if (PluginRootKey)
  {
    free(PluginRootKey);
    PluginRootKey = NULL;
  }

  if ((PluginRootKey = (TCHAR*)malloc(sizeof(TCHAR)*(lstrlen(Info->RootKey) + lstrlen(cpPlugRegKey) + 1))) != NULL)
  {
    lstrcpy(PluginRootKey, Info->RootKey);
    lstrcat(PluginRootKey, cpPlugRegKey);
  }
  else
  {
    const TCHAR *MsgItems[] = {
      GetMsg(MNoMemTitle),
      GetMsg(MNoMemBody),
      GetMsg(MOK)
    };
    ::Info.Message(::Info.ModuleNumber, FMSG_WARNING, NULL,
                   MsgItems, ArraySize(MsgItems), 1);
  }
}

/****************************************************************************
 * This plugin's function is called by FAR at second place
 ****************************************************************************/
void WINAPI EXP_NAME(GetPluginInfo)(struct PluginInfo *Info)
{
  static const TCHAR *PluginMenuStrings[1];

  Info->StructSize              = (int)sizeof(*Info);
  Info->Flags                   = 0;
  Info->DiskMenuStrings         = NULL;
  PluginMenuStrings[0]          = GetMsg(MCompare);
  Info->PluginMenuStrings       = PluginMenuStrings;
  Info->PluginMenuStringsNumber = ArraySize(PluginMenuStrings);
  Info->PluginConfigStrings     = NULL;
  Info->CommandPrefix           = NULL;
}

void GetPanelItem(HANDLE hPlugin,int Command,int Param1,PluginPanelItem* Param2)
{
	PluginPanelItem* item=(PluginPanelItem*)malloc(Info.Control(hPlugin,Command,Param1,0));
	if(item)
	{
		Info.Control(hPlugin,Command,Param1,(LONG_PTR)item);
		*Param2=*item;
		Param2->FindData.lpwszFileName=wcsdup(item->FindData.lpwszFileName);
		Param2->FindData.lpwszAlternateFileName=wcsdup(item->FindData.lpwszAlternateFileName);
		Param2->Description=NULL;
		Param2->Owner=NULL;
		Param2->CustomColumnData=NULL;
		Param2->UserData=0;
		free(item);
	}
}

void FreePanelItems(OwnPanelInfo &AInfo,OwnPanelInfo &PInfo)
{
  for(int i=0;i<AInfo.ItemsNumber;i++)
  {
    free((void*)AInfo.PanelItems[i].FindData.lpwszFileName);
    free((void*)AInfo.PanelItems[i].FindData.lpwszAlternateFileName);
  }
  for(int i=0;i<AInfo.SelectedItemsNumber;i++)
  {
    free((void*)AInfo.SelectedItems[i].FindData.lpwszFileName);
    free((void*)AInfo.SelectedItems[i].FindData.lpwszAlternateFileName);
  }
  delete[] AInfo.PanelItems;
  delete[] AInfo.SelectedItems;

  for(int i=0;i<PInfo.ItemsNumber;i++)
  {
    free((void*)PInfo.PanelItems[i].FindData.lpwszFileName);
    free((void*)PInfo.PanelItems[i].FindData.lpwszAlternateFileName);
  }
  for(int i=0;i<PInfo.SelectedItemsNumber;i++)
  {
    free((void*)PInfo.SelectedItems[i].FindData.lpwszFileName);
    free((void*)PInfo.SelectedItems[i].FindData.lpwszAlternateFileName);
  }
  delete[] PInfo.PanelItems;
  delete[] PInfo.SelectedItems;
}


/****************************************************************************
 * The main plugin's function. FAR calls it when user invokes the plugin.
 ****************************************************************************/
HANDLE WINAPI EXP_NAME(OpenPlugin)(int OpenFrom, INT_PTR Item)
{
  // If FAR version is too old...
  if (bOldFAR)
  {
    const TCHAR *MsgItems[] = {
      GetMsg(MOldFARTitle),
      GetMsg(MOldFARBody),
      GetMsg(MOK)
    };
    Info.Message(Info.ModuleNumber, FMSG_WARNING, NULL,
                 MsgItems, ArraySize(MsgItems), 1);

    return INVALID_HANDLE_VALUE;
  }

  OwnPanelInfo AInfo, PInfo;

  memset(&AInfo,0,sizeof(OwnPanelInfo));
  memset(&PInfo,0,sizeof(OwnPanelInfo));
  PanelInfo AI,PI;
  Info.Control(PANEL_ACTIVE, FCTL_GETPANELINFO,0,(LONG_PTR)&AI);
  Info.Control(PANEL_PASSIVE, FCTL_GETPANELINFO,0,(LONG_PTR)&PI);

  AInfo.PanelType=AI.PanelType;
  AInfo.Plugin=AI.Plugin;
  AInfo.ItemsNumber=AI.ItemsNumber;
  AInfo.SelectedItemsNumber=AI.SelectedItemsNumber;
  AInfo.Flags=AI.Flags;

  int Size=Info.Control(PANEL_ACTIVE, FCTL_GETPANELDIR,0,NULL);
  AInfo.lpwszCurDir=new wchar_t[Size];
  Info.Control(PANEL_ACTIVE, FCTL_GETPANELDIR,Size,(LONG_PTR)AInfo.lpwszCurDir);

  if(AInfo.ItemsNumber)
  {
    AInfo.PanelItems=new PluginPanelItem[AInfo.ItemsNumber];
    for(int i=0;i<AInfo.ItemsNumber;i++)
      GetPanelItem(PANEL_ACTIVE, FCTL_GETPANELITEM,i,&AInfo.PanelItems[i]);
  }
  else
    AInfo.PanelItems=NULL;

  if(AInfo.SelectedItemsNumber)
  {
    AInfo.SelectedItems=new PluginPanelItem[AInfo.SelectedItemsNumber];
    for(int i=0;i<AInfo.SelectedItemsNumber;i++)
      GetPanelItem(PANEL_ACTIVE, FCTL_GETSELECTEDPANELITEM,i,&AInfo.SelectedItems[i]);
  }
  else
    AInfo.SelectedItems=NULL;

  PInfo.PanelType=PI.PanelType;
  PInfo.Plugin=PI.Plugin;
  PInfo.ItemsNumber=PI.ItemsNumber;
  PInfo.SelectedItemsNumber=PI.SelectedItemsNumber;
  PInfo.Flags=PI.Flags;

  Size=Info.Control(PANEL_PASSIVE, FCTL_GETPANELDIR,0,NULL);
  PInfo.lpwszCurDir=new wchar_t[Size];
  Info.Control(PANEL_PASSIVE, FCTL_GETPANELDIR,Size,(LONG_PTR)PInfo.lpwszCurDir);

  if(PInfo.ItemsNumber)
  {
    PInfo.PanelItems=new PluginPanelItem[PInfo.ItemsNumber];
    for(int i=0;i<PInfo.ItemsNumber;i++)
      GetPanelItem(PANEL_PASSIVE, FCTL_GETPANELITEM,i,&PInfo.PanelItems[i]);
  }
  else
    PInfo.PanelItems=NULL;

  if(PInfo.SelectedItemsNumber)
  {
    PInfo.SelectedItems=new PluginPanelItem[PInfo.SelectedItemsNumber];
    for(int i=0;i<PInfo.SelectedItemsNumber;i++)
      GetPanelItem(PANEL_PASSIVE, FCTL_GETSELECTEDPANELITEM,i,&PInfo.SelectedItems[i]);
  }
  else
    PInfo.SelectedItems=NULL;

  // In case of non-file panels...
  if (AInfo.PanelType != PTYPE_FILEPANEL || PInfo.PanelType != PTYPE_FILEPANEL)
  {
    const TCHAR *MsgItems[] = {
      GetMsg(MCmpTitle),
      GetMsg(MFilePanelsRequired),
      GetMsg(MOK)
    };
    Info.Message(Info.ModuleNumber, FMSG_WARNING, NULL,
                 MsgItems, ArraySize(MsgItems), 1);
    FreePanelItems(AInfo,PInfo);
    return INVALID_HANDLE_VALUE;
  }

  // Decide whether active panel is left or right
  bool bActiveIsLeft = true;
  if (AInfo.Flags & PFLAGS_PANELLEFT)
    bActiveIsLeft = true;
  else if (PInfo.Flags & PFLAGS_PANELLEFT)
    bActiveIsLeft = false;

  // If the dialog cannot be displayed...
  if ( !ShowDialog(AInfo.Plugin || PInfo.Plugin,
                  (AInfo.SelectedItemsNumber && (AInfo.SelectedItems[0].Flags & PPIF_SELECTED)) ||
                  (PInfo.SelectedItemsNumber && (PInfo.SelectedItems[0].Flags & PPIF_SELECTED))) )
  {
	FreePanelItems(AInfo,PInfo);
    return INVALID_HANDLE_VALUE;
  }

  // Open console input for Esc checking...
  HANDLE hScreen = Info.SaveScreen(0, 0, -1, -1);
  hConInp = CreateFile(L"CONIN$", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

  // Calculate optimal compare dialog width...
  HANDLE hConOut = CreateFile(L"CONOUT$", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
  CONSOLE_SCREEN_BUFFER_INFO csbiNfo;
  if (GetConsoleScreenBufferInfo(hConOut, &csbiNfo))
  {
    if ((iTruncLen = csbiNfo.dwSize.X - 20) > NM - 2)
      iTruncLen = NM - 2;
    else if (iTruncLen < 0)
      iTruncLen = csbiNfo.dwSize.X - csbiNfo.dwSize.X / 4;
  }
  else
    iTruncLen = 60;
  CloseHandle(hConOut);

  // At comparison time FAR console title is changed...
  TCHAR cConsoleTitle[MAX_PATH], cBuffer[MAX_PATH];
  DWORD dwTitleSaved = GetConsoleTitle(cConsoleTitle, ArraySize(cConsoleTitle));
    lstrcpy(cBuffer, GetMsg(MComparingFiles));
  SetConsoleTitle(cBuffer);

  // Read comparison buffer size from the registry...
  HKEY hKey;
  if ( !PluginRootKey || RegOpenKeyEx(HKEY_CURRENT_USER, PluginRootKey, 0, KEY_QUERY_VALUE, &hKey) !=
       ERROR_SUCCESS )
    hKey = 0;
  DWORD dwSize = sizeof(DWORD);
  bufSize = ( hKey && (RegQueryValueEx(hKey, L"CompareBufferSize", NULL, NULL, (LPBYTE)&bufSize,
              &dwSize) == ERROR_SUCCESS) && (bufSize > 32767) ) ? bufSize : 32768;
  if (hKey)
    RegCloseKey(hKey);
  ABuf = (TCHAR*)malloc(bufSize*sizeof(TCHAR));
  PBuf = (TCHAR*)malloc(bufSize*sizeof(TCHAR));

  bBrokenByEsc = false;
  bStart = true;
  bOpenFail = false;
  bool bDifferenceNotFound = false;
  
  AFilter = INVALID_HANDLE_VALUE;
  PFilter = INVALID_HANDLE_VALUE;

  Info.FileFilterControl(PANEL_ACTIVE,  FFCTL_CREATEFILEFILTER, FFT_PANEL, (LONG_PTR)&AFilter);
  Info.FileFilterControl(PANEL_PASSIVE, FFCTL_CREATEFILEFILTER, FFT_PANEL, (LONG_PTR)&PFilter);

  Info.FileFilterControl(AFilter, FFCTL_STARTINGTOFILTER, 0, 0);
  Info.FileFilterControl(PFilter, FFCTL_STARTINGTOFILTER, 0, 0);

  TCHAR cResultsFileName[MAX_PATH] = L"";
  // char *cLeft = NULL, *cRight = NULL;
  TCHAR cLeft[NM] = L"", cRight[NM] = L"";
  FILE * fpResults = 0; 
  WARNING("Не проверяется, реально ли сформировано имя файла");
  if (FSF.MkTemp(cResultsFileName, sizeof(cResultsFileName)/sizeof(cResultsFileName[0]), NULL)) {
  	fpResults = _wfopen(cResultsFileName, L"w, ccs=UNICODE");
  	WARNING("Не проверяется код возврата");
  	if (bActiveIsLeft) {
			lstrcpy(cLeft, AInfo.lpwszCurDir);
			lstrcpy(cRight, PInfo.lpwszCurDir);
      //cLeft = AInfo.CurDir;
      //cRight = PInfo.CurDir;
    } else {
			lstrcpy(cLeft, PInfo.lpwszCurDir);
			lstrcpy(cRight, AInfo.lpwszCurDir);
      //cLeft = PInfo.CurDir;
      //cRight = AInfo.CurDir;
    }
	const TCHAR* psz = GetMsg(MReportFileTitle);
  	fwprintf(fpResults, psz, cLeft, cRight);
  }
  
  // now we can compare objects on panels...
  if (ABuf && PBuf
      && AFilter != INVALID_HANDLE_VALUE && PFilter != INVALID_HANDLE_VALUE
       )
  {
    int RootDirLength = wcslen(AInfo.lpwszCurDir);
    if ((RootDirLength > 0) && (AInfo.lpwszCurDir[RootDirLength-1] == L'\\'))
       RootDirLength--;
    bDifferenceNotFound = CompareDirs(&AInfo, &PInfo, true, 0, fpResults, RootDirLength, bActiveIsLeft);
  }

  Info.FileFilterControl(AFilter, FFCTL_FREEFILEFILTER, 0, 0);
  Info.FileFilterControl(PFilter, FFCTL_FREEFILEFILTER, 0, 0);

  if (fpResults)
    fclose(fpResults);
    
  free (ABuf);
  free (PBuf);
  CloseHandle(hConInp);
  Info.RestoreScreen(hScreen);

  // Select files and redraw panels. Show message if necessary...
  if (!bBrokenByEsc)
  {
    Info.Control(PANEL_ACTIVE,FCTL_BEGINSELECTION,0,NULL);
    for(int i=0;i<AInfo.ItemsNumber;i++)
    {
      Info.Control(PANEL_ACTIVE, FCTL_SETSELECTION,i,AInfo.PanelItems[i].Flags&PPIF_SELECTED);
    }
    Info.Control(PANEL_ACTIVE,FCTL_ENDSELECTION,0,NULL);

    Info.Control(PANEL_PASSIVE,FCTL_BEGINSELECTION,0,NULL);
    for(int i=0;i<PInfo.ItemsNumber;i++)
    {
      Info.Control(PANEL_PASSIVE, FCTL_SETSELECTION,i,PInfo.PanelItems[i].Flags&PPIF_SELECTED);
    }
    Info.Control(PANEL_PASSIVE,FCTL_ENDSELECTION,0,NULL);

    Info.Control(PANEL_ACTIVE, FCTL_REDRAWPANEL,0,NULL);
    Info.Control(PANEL_PASSIVE, FCTL_REDRAWPANEL,0,NULL);

    if(bOpenFail)
    {
      const TCHAR *MsgItems[] = {
        GetMsg(MOpenErrorTitle),
        GetMsg(MOpenErrorBody),
        GetMsg(MOK),
      };
      Info.Message(Info.ModuleNumber, FMSG_WARNING, NULL, MsgItems, ArraySize(MsgItems), 1);
    }
    if (bDifferenceNotFound && Opt.MessageWhenNoDiff)
    {
      const TCHAR *MsgItems[] = {
        GetMsg(MNoDiffTitle),
        GetMsg(MNoDiffBody),
        GetMsg(MOK)
      };
      Info.Message(Info.ModuleNumber, 0, NULL,
                   MsgItems, ArraySize(MsgItems), 1);
    }
  }
  // I show differences in the editor
  if (!bDifferenceNotFound && cResultsFileName[0]) {
    if (Opt.ShowReport) {
		//If the report shows, so I show a modeless editor and possible synchronization
		//I will be doing at the moment of shut the editor.
		//Synchronization will only do nepluginovych panel just in case I
		//manage to do the editor
		if (Info.Editor(cResultsFileName, 
                      GetMsg(MCmpTitle), 
                      0, 0, -1, -1, 
      	              EF_NONMODAL | EF_IMMEDIATERETURN | EF_DELETEONLYFILEONCLOSE | EF_DISABLEHISTORY,
                      0, 1, CP_UNICODE) == EEC_MODIFIED) {
        EditorInfo edInfo;
        if (!AInfo.Plugin && !PInfo.Plugin && Info.EditorControl(ECTL_GETINFO, &edInfo)) {
          // I will set a record for this editor, so I can retrieve it later
          struct PendingSync * psItem;
          if (psItem = (struct PendingSync *)malloc(sizeof(*psItem))) {
            psItem->next = psEditorsQueue;
#ifndef IDENTIFY_EDITOR_BY_FILENAME
            Info.AdvControl(Info.ModuleNumber, ACTL_COMMIT, 0);
            psItem->EditorID = edInfo.EditorID;
#endif
            lstrcpy(psItem->LeftDir, cLeft);
            lstrcpy(psItem->RightDir, cRight);
            lstrcpy(psItem->ReportFileName, cResultsFileName);
            psEditorsQueue = psItem;
          }
        }
      }
    } else {
      // Otherwise I can see straight to the query parameters and perform a synchronization
      struct SyncOptions opt;
      if (GetSyncOptions(&opt, cResultsFileName)) {
        Synchronize(cLeft, cRight, cResultsFileName, &opt);
      }
    }
  }
  // Restore FAR console title...
  if (dwTitleSaved)
    SetConsoleTitle(cConsoleTitle);
  FreePanelItems(AInfo,PInfo);
  return INVALID_HANDLE_VALUE;
}

/****************************************************************************
 * This function is called by FAR before the plugin is unloaded
 ****************************************************************************/
void WINAPI EXP_NAME(ExitFAR)(void)
{
  if (PluginRootKey)
  {
    free(PluginRootKey);
    PluginRootKey = NULL;
  }
  struct PendingSync * q;
  while (psEditorsQueue != NULL) {
		q = psEditorsQueue->next;
		free(psEditorsQueue);
		psEditorsQueue = q;
	}
}

//Editor ox as modeless, which is indeed good, but I need him, then in
//good time to call. This is a good time event response EE_CLOSE
//editor, whom the discrepancy report viewing.
int WINAPI EXP_NAME(ProcessEditorEvent)(int Event, void *Param)
{
	if (Event == EE_CLOSE) {
		// I'll see if closing the editor are among the mine
		struct PendingSync ** owner = &psEditorsQueue;
		struct PendingSync * q = psEditorsQueue;
#ifdef IDENTIFY_EDITOR_BY_FILENAME
		TCHAR edFileName[NM];
		if ((q != NULL) && Info.EditorControl(ECTL_GETFILENAME, edFileName))	{
			while ((q != NULL) && (wcsicmp(edFileName, q->ReportFileName) != 0)) {
				owner = &(q->next);
				q = q->next;
			}
		} else
			q = NULL;
#else
		while ((q != NULL) && (q->EditorID != (int) Param)) {
			owner = &(q->next);
			q = q->next;
		}
#endif
		// Yes, the floor (because otherwise it ran to the end of q would be NULL)
		if (q != NULL) {
			// This means that the need to call synchronize
	  		struct SyncOptions opt;
			if (GetSyncOptions(&opt, q->ReportFileName)) {
				Synchronize(q->LeftDir, q->RightDir, q->ReportFileName, &opt);
			}
			// And then the editor to delete the queue
			*owner = q->next;
			free(q);
		}
	}
	return 0;
}

