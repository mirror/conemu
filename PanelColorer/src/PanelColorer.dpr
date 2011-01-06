(*

Panel Colorer Plugin v 0.94

Author: Igor Afanasyev <igor.afanasyev (at) gmail (dot) com>

License: GPL v.3 (see license.txt)

*)

{$AppType Console}

// DEBUG defines (should be all disabled for production)

{.$DEFINE ShowRenderingDebugStrings} // show some panel debug info
{.$DEFINE DumpVarsFile} // when something has changed, dump vars to text file on desktop

library PanelColorer;

uses
  //FastMM глючит при завершении потоков
  //FastMM4 {for faster memory allocation -- should be the first in 'uses' clause},
  Windows {for WinAPI functions and types},
  SysUtils {for Format and ExtractFileDir function},
  Classes {for TThread},
  Graphics {for font styles},
  GR32 {for TBitmap32},
  PNGImage {to support loading PNG images},
  Registry {to work with Registry},
  ShlObj {for GetSpecialFolder},
  PluginW in 'FarAPI\PluginW.pas' {FAR API bindings},
  ImageCache {ImageChache object},
  Config {Configuring plugin via ECMAScript},
  DiskInfoCache {Cache disk information to lower CPU usage},
  ConEmu in 'ConEmu.pas';

//------------------------------------------------------------------------------

type
  (*
  TWatcherThread = class(TThread)
  protected
    procedure Execute; override;
  end;
  *)

  //TPathWideChar = array[0..MAX_PATH] of WideChar;

  TExtendedPanelInfo = record
    WindowType: Integer;
    Info: TPanelInfo;
    Handle: THandle; // either PANEL_ACTIVE or PANEL_PASSIVE;
    Rect: TRect;
    Path: String;
    DriveType: Cardinal;
    Format: String;
    HostFile: String;
    BytesFree,
    BytesTotal : Int64;
    BgColor: TColor32;
    Text: String;
    ImageName : String;
    MeterAvailable : Boolean;
    MeterVisible : Boolean;
    MeterWidth : Integer;
    DebugString : String;
  end;
  PExtendedPanelInfo = ^TextendedPanelInfo;

//------------------------------------------------------------------------------

const
  CONST_MSEC_IN_DAY = 24*60*60*1000;

  CONST_SLEEP_MS = 10; // ms; production value
  //CONST_SLEEP_MS = 500; // ms; debug

  CONST_REG_KEY = '\PanelColorer'; // relative to FAR-provided root key

  CONST_BASE_CONFIG_FILENAME = '\config.js'; // relative to plugin folder
  CONST_USER_CONFIG_FILENAME = '\user.js'; // relative to plugin folder

  WTYPE_BACKGROUND = 0; // extended window type not defined in PluginW.pas

//------------------------------------------------------------------------------

var
  FConfigMenuStrings: array[0..0] of PFarChar;

  FEnabled: Boolean = false;
  FFarInitialized: Boolean = false;
  FCEInitialized: Boolean = false;
  FCEWrongVersion: Boolean = false;
  FRegistered: Boolean = false;
  FOptEnabled: Boolean = false;
  FInFarExit: Boolean = false;

  FDebug: Boolean = false;
  FOptDebug: Boolean = false;

  FARAPI: TPluginStartupInfo;
  FSF: TFarStandardFunctions;

  //FWatcherThread: TWatcherThread = nil;
  FFarHWND: HWND;
  FOutHandle: THandle;
  FConEmuHwnd: HWND;

  FBaseConfig: TScriptConfig;
  FUserConfig: TScriptConfig;

  FDesktopPath: String;
  FPluginFolder: String;
  FConEmuBackgroundImagePath: String;
  {$IFDEF DumpVarsFile}
  FDumpVarsFilePath: String;
  {$ENDIF}
  FLogFilePath: String;

  FBmp32: TBitmap32;
  FImageCache: TImageCache;
  FDiskInfoCache: TDiskInfoCache;
  FTextHeight : Integer;

  FOldBackgroundPanel, FBackgroundPanel,
  FOldLeftPanel, FLeftPanel,
  FOldRightPanel, FRightPanel,
  FOldViewerPanel, FViewerPanel,
  FOldEditorPanel, FEditorPanel: TExtendedPanelInfo;

  FWindowInfo: TWindowInfo;
  FOldWindowType: Integer;
  FOldWindowPos: Integer;

  FOldSymWidth,
  FOldSymHeight: Integer;
  FOldClientRect: TRect;

  FReferencePeekString: String;
  FOldWindowIsHidden: Boolean;

function GetConsoleWindow: HWND; stdcall; external 'kernel32.dll';

procedure DoTheJob; forward;

//------------------------------------------------------------------------------
function GetSpecialFolder(CSIDL: Integer): String;
//------------------------------------------------------------------------------
var
  buf: array[0..MAX_PATH] of WideChar;
begin
  Result := '';
  if SHGetSpecialFolderPathW(0, @buf,
    CSIDL or CSIDL_FLAG_DONT_UNEXPAND or CSIDL_FLAG_DONT_VERIFY, false) then
    Result := PWideChar(@buf);
end;

//------------------------------------------------------------------------------
function GetDesktopFolder: String;
//------------------------------------------------------------------------------
begin
   Result := GetSpecialFolder(CSIDL_DESKTOP);
end;

//------------------------------------------------------------------------------
procedure Log(s: String);
//------------------------------------------------------------------------------
var
  f: TextFile;
begin
  if not FDebug then Exit;

  AssignFile(f, FLogFilePath);

  if FileExists(FLogFilePath) then
    Append(f)
  else
    Rewrite(f);

  WriteLn(f, s);
  Flush(f);
  CloseFile(f);
end;

//------------------------------------------------------------------------------
procedure DoShowStatus(S: String; ColorIndex: Byte = 10);
//------------------------------------------------------------------------------
begin
  FARAPI.Text(1, 1, ColorIndex, PWideChar(s));
end;

//------------------------------------------------------------------------------
procedure ShowStatus(S: String; ColorIndex: Byte = 10);
//------------------------------------------------------------------------------
begin
  Log(Format('[ShowStatus] %s', [S]));
  DoShowStatus(S, ColorIndex);
end;

//------------------------------------------------------------------------------
procedure DebugShowStatus(S: String);
//------------------------------------------------------------------------------
begin
  Log(Format('[DebugShowStatus] %s', [S]));
  if FDebug then DoShowStatus(S);
end;

//------------------------------------------------------------------------------
procedure DebugShowError(S: String);
//------------------------------------------------------------------------------
begin
  Log(Format('[DebugShowError] %s', [S]));
  if FDebug then DoShowStatus(S, 12);
end;

//------------------------------------------------------------------------------
function FarPanelString(AHandle: THandle; ACmd: Integer): String;
//------------------------------------------------------------------------------
var
  vLen: Integer;
begin
  Result := '';
  vLen := FARAPI.Control(AHandle, ACmd, 0, nil);
  if (vLen > 1) then
  begin
    SetLength(Result, vLen - 1);
    FARAPI.Control(AHandle, ACmd, vLen, PFarChar(Result));
  end;
end;

//------------------------------------------------------------------------------
function FarGetWindowRect: TSmallRect;
//------------------------------------------------------------------------------
begin
  FARAPI.AdvControl(FARAPI.ModuleNumber, ACTL_GETFARRECT, @Result);
end;

//------------------------------------------------------------------------------
procedure GetPluginInfoW(var pi: TPluginInfo); stdcall;
//------------------------------------------------------------------------------
begin
  Log('GetPluginInfoW()');

  pi.StructSize := SizeOf(pi);
  pi.Flags := PF_PRELOAD;

  pi.PluginConfigStrings := @FConfigMenuStrings;
  pi.PluginConfigStringsNumber := 1;
end;

//------------------------------------------------------------------------------
procedure LoadSettings;
//------------------------------------------------------------------------------
var
  reg: TRegistry;

  //----------------------------------------------------------------------------
  function readBool(const Name: string; DefValue: Boolean = false): Boolean;
  //----------------------------------------------------------------------------
  begin
    if reg.ValueExists(Name) then
      Result := reg.ReadBool(Name)
    else
      Result := DefValue;
  end;

//------------------------------------------------------------------------------
begin
  Log('LoadSettings()');

  reg := TRegistry.Create;
  try
    reg.RootKey := HKEY_CURRENT_USER;

    if not reg.OpenKey(FARAPI.RootKey + CONST_REG_KEY, false) then
      Exit;

    FEnabled := readBool('Enabled', true); // enable plugin by default
    FDebug := readBool('Debug');
  finally
    reg.Free;
  end;
end;

//------------------------------------------------------------------------------
procedure SaveSettings;
//------------------------------------------------------------------------------
var
  reg: TRegistry;
begin
  reg := TRegistry.Create;
  try
    reg.RootKey := HKEY_CURRENT_USER;

    if not reg.OpenKey(FARAPI.RootKey + CONST_REG_KEY, true) then
      Exit; // should user be warned here?

    reg.WriteBool('Enabled', FEnabled);
    reg.WriteBool('Debug', FDebug);
  finally
    reg.Free;
  end;
end;

//------------------------------------------------------------------------------
function IsUnderConEmu : Boolean;
//------------------------------------------------------------------------------
begin
  Result := (FFarHWND <> 0) and (FConEmuHWND <> 0)
             //and Assigned(FWatcherThread)
             and Assigned(FBmp32);
             //and not FWatcherThread.Terminated;
end;

//------------------------------------------------------------------------------
function UpdateConEmuBackground : Boolean;
//------------------------------------------------------------------------------
var
  memStream : TMemoryStream;
  res : TSetBackgroundResult;
begin
  Result := false;
  if not IsUnderConEmu then Exit;

  if not FEnabled then
  begin
    (*
    ClearBackground(FFarHWND)
    *)
    if FRegistered then begin
      ConEmuUnRegister();
      FRegistered := false;
    end;
  end
  else
  begin
    //TODO: Обработка ошибок!
    if not FRegistered then begin
      ConEmuRegister();
      FRegistered := true;
    end else begin
      ConEmuRedraw();
    end;
    (*
    memStream := TMemoryStream.Create;
    try
      FBmp32.SaveToStream(memStream);
      res := SetBackground(FFarHWND, memStream.Memory, memStream.Size);
      case res of
         esbr_OK:
           Result := true;

         esbr_InvalidArg:
           DebugShowError('ConEmu says invalid argument passed');

         esbr_PluginForbidden:
           ShowStatus('Please enable "Allow plugins" option in ConEmu');

         esbr_ConEmuInShutdown:
           DebugShowError('ConEmu is shutting down');

         esbr_Unexpected:
           DebugShowError('ConEmu reported an unexpected error');

         else
           DebugShowError(Format('ConEmu reported unknown error #%d', [Ord(res)]));
      end;
    finally
      memStream.Free;
    end;
    *)
  end;
end;

//------------------------------------------------------------------------------
procedure UpdateEnabledState;
//------------------------------------------------------------------------------
const
  CRLF = #13#10;
begin
  if not IsUnderConEmu then Exit;

  //FWatcherThread.Suspended := not FEnabled;
  //if (not FEnabled) then // otherwise the bitmap will be updated automatically
  begin
    UpdateConEmuBackground;

    // clear old rendered state
    ZeroMemory(@FOldClientRect, SizeOf(FOldClientRect));
    ZeroMemory(@FOldLeftPanel, SizeOf(FOldLeftPanel));
    ZeroMemory(@FOldRightPanel, SizeOf(FOldRightPanel));
  end;
end;

type
  TDialogItem = (diBorder, diSeparator, diOK, diCancel, diEnabledCheckbox, diDebugCheckbox);

//------------------------------------------------------------------------------
function OptionsDialogHandler(hDlg: THandle; Msg: Integer; Param1: Integer; Param2: LONG_PTR): LONG_PTR; stdcall;
//------------------------------------------------------------------------------
begin
  if (Msg = DN_BTNCLICK) then
  begin
    if (Param1 = Ord(diEnabledCheckbox)) then // 'Enabled' checkbox
    begin
      FOptEnabled := (Param2 > 0);
      Result := 1;
      Exit;
    end;

    if (Param1 = Ord(diDebugCheckbox)) then // 'Debug' checkbox
    begin
      FOptDebug := (Param2 > 0);
      Result := 1;
      Exit;
    end;
  end;

  Result := FARAPI.DefDlgProc(hDlg, Msg, Param1, Param2);
end;

//------------------------------------------------------------------------------
function ConfigureW(Item: integer): Integer; stdcall;
//------------------------------------------------------------------------------
const
  dlgWidth = 29;
  dlgHeight = 10;
  xBodrderMargin = 3;
  yBorderMargin = 1;
var
  dlg: THandle;
  items: array[TDialogItem] of TFarDialogItem;
  code: Integer;
  changed: Boolean;
  eStr: WideString; //TFarStr;
begin
  Result := 1;

  if not FCEInitialized then begin
    //{$ifdef bUnicodeFar}
    // vStr := ATitle + #10 + AMessage;
    //{$else}
    // vStr := StrAnsiToOem(ATitle + #10 + AMessage);
    //{$endif bUnicodeFar}
    if FCEWrongVersion then
      eStr := 'Panel Colorer' + #10 + 'Unsupported ConEmu version'
    else
      eStr := 'Panel Colorer' + #10 + 'ConEmu was not loaded or initialized';
    FARAPI.Message(FARAPI.ModuleNumber, FMSG_ALLINONE or FMSG_MB_OK or FMSG_WARNING, nil, PPCharArray(PFarChar(eStr)), 0, 0);
    Exit;
  end;


  FOptEnabled := FEnabled;
  FOptDebug := FDebug;

  ZeroMemory(@items, SizeOf(items));

  // dialog border
  with items[diBorder] do
  begin
    ItemType := DI_DOUBLEBOX;
    X1 := xBodrderMargin;
    Y1 := yBorderMargin;
    X2 := dlgWidth - xBodrderMargin - 1;
    Y2 := dlgHeight - yBorderMargin - 1;
    PtrData := 'Panel Colorer';
  end;

  // separator
  with items[diSeparator] do
  begin
    ItemType := DI_TEXT;
    Y1 := dlgHeight - yBorderMargin - 3;
    Flags := DIF_SEPARATOR;
  end;

  // 'OK' button
  with items[diOK] do
  begin
    ItemType := DI_BUTTON;
    Y1 := dlgHeight - yBorderMargin - 2;
    Flags := DIF_CENTERGROUP;
    PtrData := 'OK';
    DefaultButton := 1;
  end;

  // 'Cancel' button
  with items[diCancel] do
  begin
    ItemType := DI_BUTTON;
    Y1 := dlgHeight - yBorderMargin - 2;
    Flags := DIF_CENTERGROUP;
    PtrData := 'Cancel';
  end;

  // '[ ] Enabled' checkbox
  with items[diEnabledCheckbox] do
  begin
    ItemType := DI_CHECKBOX;
    X1 := xBodrderMargin + 2;
    Y1 := yBorderMargin + 2;
    X2 := dlgWidth - xBodrderMargin - 2;
    Y2 := 1;
    PtrData := '&Enabled';
    Focus := 1;
    if FOptEnabled then
      Param.Selected := 1;
  end;

  // '[ ] Debug Mode' checkbox
  with items[diDebugCheckbox] do
  begin
    ItemType := DI_CHECKBOX;
    X1 := xBodrderMargin + 2;
    Y1 := yBorderMargin + 3;
    X2 := dlgWidth - xBodrderMargin - 2;
    Y2 := 1;
    PtrData := '&Debug mode';
    if FOptDebug then
      Param.Selected := 1;
  end;

  dlg := FARAPI.DialogInit(
    FARAPI.ModuleNumber, -1, -1, dlgWidth, dlgHeight,
    nil, @items, Ord(High(TDialogItem)) + 1, 0, 0, OptionsDialogHandler, 0);

  code := FARAPI.DialogRun(dlg);

  if (code = Ord(diOK)) then
  begin
    changed := (FEnabled <> FOptEnabled) or (FDebug <> FOptDebug);

    if (FEnabled <> FOptEnabled) then
    begin
      FEnabled := FOptEnabled;
      UpdateEnabledState;
    end;

    if (FDebug <> FOptDebug) then
    begin
      FDebug := FOptDebug;
      if FEnabled then
        DebugShowStatus('Panel Colorer debug mode is ON');
    end;

    if changed then
      SaveSettings;
  end;
end;

//------------------------------------------------------------------------------
procedure SetStartupInfoW(var psi: TPluginStartupInfo); stdcall;
var
  eStr: WideString; //TFarStr;
//------------------------------------------------------------------------------
begin
  FDesktopPath := GetDesktopFolder;
  if (FDesktopPath = '') then FDesktopPath := 'C:';

  FLogFilePath := FDesktopPath + '\PanelColorer_log.txt';
  if FileExists(FLogFilePath) then
    DeleteFile(FLogFilePath);

  Log('SetStartupInfoW()');

  Move(psi, FARAPI, SizeOf(FARAPI));
  FSF := psi.FSF^;
  FFarInitialized := true;

  LoadSettings;

  {$IFDEF DumpVarsFile}
  FDumpVarsFilePath := FDesktopPath + '\PanelColorer_vars.txt';
  if FileExists(FDumpVarsFilePath) then
    DeleteFile(FDumpVarsFilePath);
  {$ENDIF}

  FConfigMenuStrings[0] := 'Panel Colorer';

  FFarHWND := FARAPI.AdvControl(FARAPI.ModuleNumber, ACTL_GETFARHWND, nil);
  FOutHandle := GetStdHandle(STD_OUTPUT_HANDLE);

  Log(Format('FFarHWND = 0x%x', [FFarHWND]));
  Log(Format('FOutHandle = 0x%x', [FOutHandle]));

  FPluginFolder := IncludeTrailingPathDelimiter(ExtractFileDir(psi.ModuleName));

  Log(Format('FPluginFolder = "%s"', [FPluginFolder]));

  FConEmuBackgroundImagePath := FDesktopPath + '\PanelColorer.bmp';

  //TODO: Переделать. Выполнять в OnConEmuLoaded
  FConEmuHWND := GetConEmuHWND(FFarHWND, false);

  Log(Format('FConEmuHWND = 0x%x', [FConEmuHWND]));

  if (FConEmuHWND = 0) then
  begin
    if FEnabled then
      DebugShowError('ConEmu environment not found');
    Exit;
  end;

  FBmp32 := TBitmap32.Create;
  FBmp32.Font.Style := [fsBold];

  FImageCache := TImageCache.Create;

  Log('Image objects created');

  FDiskInfoCache := TDiskInfoCache.Create;

  Log('Disk info cache object created');

  try
    FBaseConfig := TScriptConfig.Create(FPluginFolder + CONST_BASE_CONFIG_FILENAME);
  except
    on E:Exception do
      DebugShowError(Format('Base script compilation error: %s', [E.Message]));
  end;

  try
    FUserConfig := TScriptConfig.Create(FPluginFolder + CONST_USER_CONFIG_FILENAME);
  except
    on E:Exception do
      DebugShowError(Format('User script compilation error: %s', [E.Message]));
  end;

  Log('Scripts loaded');

  FOldWindowType := -1; // set negative windpw type to force backround style to reload once
  FOldWindowPos := 0;
  FOldSymWidth := 0;
  FOldSymHeight := 0;
  FReferencePeekString := '';
  FOldWindowIsHidden := false;

  ZeroMemory(@FOldClientRect, SizeOf(FOldClientRect));
  ZeroMemory(@FOldLeftPanel, SizeOf(FOldLeftPanel));
  ZeroMemory(@FOldRightPanel, SizeOf(FOldRightPanel));

  //FWatcherThread := TWatcherThread.Create(not FEnabled);
  //FWatcherThread.FreeOnTerminate := false;
  if FCEInitialized then begin
    DoTheJob;
  end else if FCEWrongVersion then begin
    eStr := 'Panel Colorer' + #10 + 'Unsupported ConEmu version';
    FARAPI.Message(FARAPI.ModuleNumber, FMSG_ALLINONE or FMSG_MB_OK or FMSG_WARNING, nil, PPCharArray(PFarChar(eStr)), 0, 0);
  end;

  Log('Watcher thread created');
end;

//------------------------------------------------------------------------------
procedure DoTheJob;
//------------------------------------------------------------------------------
const
  PanelReferenceString = '╚';
  PeekStringLength = 10;
  CRLF = #13#10;
var
  POldInfo, PInfo: PExtendedPanelInfo;
  s, peekString: String;
  viewerInfo: TViewerInfo;
  buf: array[0..MAX_PATH] of WideChar;
  debugStr : String;
  windowIsHidden: Boolean;
  cr: TSmallRect;
  clientRect: TRect;
  symWidth, symHeight: Integer;
  force, needFullCheck, f1, f2: Boolean;
  sectorsPerCluster, bytesPerSector, freeClusters, totalClusters : Cardinal;
  {$IFDEF DumpVarsFile}
  f: TextFile;
  {$ENDIF}
  img : TBitmap32;
  pf, pc1, pc2, pc3: Int64;

  {$REGION 'procedure debugStrAdd(S: String);' fold}
  //----------------------------------------------------------------------------
  procedure debugStrAdd(S: String);
  //----------------------------------------------------------------------------
  begin
    debugStr := debugStr + CRLF + S;
  end;
  {$ENDREGION}

  {$REGION 'procedure DumpVarsFile;' fold}
  {$IFDEF DumpVarsFile}
  //----------------------------------------------------------------------------
  procedure DumpVarsFile;
  //----------------------------------------------------------------------------
  var
    oldDebugStr: String;
  begin
    oldDebugStr := debugStr;

    AssignFile(f, FDesktopPath + '\PanelColorer_vars.txt');
    if FileExists(FDesktopPath + '\PanelColorer_vars.txt') then
      Append(f)
    else
      Rewrite(f);
    try
      debugStrAdd(':5:2');

      WriteLn(f);
      WriteLn(f, '----['+FormatDateTime('HH:NN:SS.ZZZ', Now)+']----------------------');
      WriteLn(f, Format('FReferencePeekString="%s"', [FReferencePeekString]));
      WriteLn(f, Format('PeekString="%s"', [peekString]));
      WriteLn(f, Format('Force="%d"', [Ord(force)]));

      WriteLn(f);
      debugStrAdd(':5:2:1');

      with cr do
        WriteLn(f, Format('FAR rect: Left=%d, Top=%d, Right=%d, Bottom=%d', [Left, Top, Right, Bottom]));

      WriteLn(f);

      if (FOldSymWidth <> symWidth) then
        WriteLn(f, Format('SymWidth %d=>%d', [FOldSymWidth, symWidth]));
      if (FOldSymHeight <> symHeight) then
        WriteLn(f, Format('SymHeight %d=>%d', [FOldSymHeight, symHeight]));
      if (FOldClientRect.Right <> clientRect.Right) then
        WriteLn(f, Format('ClientRect.Right %d=>%d', [FOldClientRect.Right, clientRect.Right]));
      if (FOldClientRect.Bottom <> clientRect.Bottom) then
        WriteLn(f, Format('ClientRect.Bottom %d=>%d', [FOldClientRect.Bottom, clientRect.Bottom]));

      WriteLn(f);
      debugStrAdd(':5:2:2');

      if (FOldWindowType <> FWindowInfo.WindowType) then
        WriteLn(f, Format('WindowType %d=>%d', [FOldWindowType, FWindowInfo.WindowType]));
      if (FOldWindowPos <> FWindowInfo.Pos) then
        WriteLn(f, Format('WindowPos %d=>%d', [FOldWindowPos, FWindowInfo.Pos]));

      WriteLn(f);
      debugStrAdd(':5:2:3');

      if (FWindowInfo.WindowType = WTYPE_PANELS) then
      begin
        if (FOldLeftPanel.Info.Visible <> FLeftPanel.Info.Visible) then
          WriteLn(f, Format('LeftPanel.Visible %d=>%d', [FOldLeftPanel.Info.Visible, FLeftPanel.Info.Visible]));
        if (Ord(FOldLeftPanel.MeterAvailable) <> Ord(FLeftPanel.MeterAvailable)) then
          WriteLn(f, Format('LeftPanel.MeterAvailable %d=>%d', [Ord(FOldLeftPanel.MeterAvailable), Ord(FLeftPanel.MeterAvailable)]));
        if (Ord(FOldLeftPanel.MeterVisible) <> Ord(FLeftPanel.MeterVisible)) then
          WriteLn(f, Format('LeftPanel.MeterVisible %d=>%d', [Ord(FOldLeftPanel.MeterVisible), Ord(FLeftPanel.MeterVisible)]));
        if (FOldLeftPanel.MeterWidth <> FLeftPanel.MeterWidth) then
          WriteLn(f, Format('LeftPanel.MeterWidth %d=>%d', [FOldLeftPanel.MeterWidth, FLeftPanel.MeterWidth]));
        if (FOldLeftPanel.Path <> FLeftPanel.Path) then
          WriteLn(f, Format('LeftPanel.Path %s=>%s', [FOldLeftPanel.Path, FLeftPanel.Path]));

        WriteLn(f);
        debugStrAdd(':5:2:4');

        if (FOldRightPanel.Info.Visible <> FRightPanel.Info.Visible) then
          WriteLn(f, Format('RightPanel.Visible %d=>%d', [FOldRightPanel.Info.Visible, FRightPanel.Info.Visible]));
        if (Ord(FOldRightPanel.MeterAvailable) <> Ord(FRightPanel.MeterAvailable)) then
          WriteLn(f, Format('RightPanel.MeterAvailable %d=>%d', [Ord(FOldRightPanel.MeterAvailable), Ord(FRightPanel.MeterAvailable)]));
        if (Ord(FOldRightPanel.MeterVisible) <> Ord(FRightPanel.MeterVisible)) then
          WriteLn(f, Format('RightPanel.MeterVisible %d=>%d', [Ord(FOldRightPanel.MeterVisible), Ord(FRightPanel.MeterVisible)]));
        if (FOldRightPanel.MeterWidth <> FRightPanel.MeterWidth) then
          WriteLn(f, Format('RightPanel.MeterWidth %d=>%d', [FOldRightPanel.MeterWidth, FRightPanel.MeterWidth]));
        if (FOldRightPanel.Path <> FRightPanel.Path) then
          WriteLn(f, Format('RightPanel.Path %s=>%s', [FOldRightPanel.Path, FRightPanel.Path]));
        WriteLn(f);
      end else
      begin
        Assert(Assigned(POldInfo), 'POldInfo is nil');
        Assert(Assigned(PInfo), 'PInfo is nil');

        if (POldInfo^.Text <> PInfo^.Text) then
          WriteLn(f, Format('Panel.Text %s=>%s', [POldInfo^.Text, PInfo^.Text]));
        if (POldInfo^.ImageName <> PInfo^.ImageName) then
          WriteLn(f, Format('Panel.ImageName %s=>%s', [POldInfo^.ImageName, PInfo^.ImageName]));
        if (POldInfo^.BgColor <> PInfo^.BgColor) then
          WriteLn(f, Format('Panel.BgColor %x=>%x', [POldInfo^.BgColor, PInfo^.BgColor]));
        if (POldInfo^.MeterVisible <> PInfo^.MeterVisible) then
          WriteLn(f, Format('Panel.MeterVisible %d=>%d', [Ord(POldInfo^.MeterVisible), Ord(PInfo^.MeterVisible)]));
      end;
      WriteLn(f, Format('Trace: %s', [oldDebugStr]));
      WriteLn(f);

    finally
      CloseFile(f);
    end;
  end;
  {$ENDIF}
  {$ENDREGION}

  {$REGION 'function PeekStringAt(X, Y, Length : Integer): String;' fold}
  //----------------------------------------------------------------------------
  function PeekStringAt(X, Y, Length : Integer): String;
  //----------------------------------------------------------------------------
  const
    bufSize = 500;
  var
    i : Integer;
    charBuf: packed array[0..BufSize-1] of TCharInfo;
    readSize, readCoord: TCoord;
    readRect: TSmallRect;
  begin
    Result := '';
    if (Length > bufSize) then Length := bufSize;

    readSize.X := bufSize;
    readSize.Y := 1;
    readCoord.X := 0;
    readCoord.Y := 0;

    readRect.Left := X;
    readRect.Top := Y;
    readRect.Right := X + Length;
    readRect.Bottom := Y + 1;

    ZeroMemory(@charBuf, SizeOf(charBuf));
    if ReadConsoleOutput(FOutHandle, @charBuf, readSize, readCoord, readRect) then
    begin
      Length := readRect.Right - readRect.Left; // real length
      for i := 0 to Length - 1 do
        Result := Result + charBuf[i].UnicodeChar;
    end;
  end;
  {$ENDREGION}

  {$REGION 'function PanelRectToClient(r: TRect): TRect;' fold}
  //----------------------------------------------------------------------------
  function PanelRectToClient(r: TRect): TRect;
  //----------------------------------------------------------------------------
  begin
    Result.Left := r.Left * symWidth;
    Result.Right := (r.Right + 1) * symWidth;
    Result.Top := r.Top * symHeight;
    Result.Bottom := (r.Bottom + 1) * symHeight;
  end;
  {$ENDREGION}

  {$REGION 'procedure ClearPanelInfo(var P: TExtendedPanelInfo);' fold}
  //----------------------------------------------------------------------------
  procedure ClearPanelInfo(var P: TExtendedPanelInfo);
  //----------------------------------------------------------------------------
  begin
    with P do
    begin
      WindowType := -1;
      ZeroMemory(@Info, SizeOf(Info));
      Handle := 0;
      Path := '';
      DriveType := 0;
      Format := '';
      HostFile := '';
      BytesFree := 0;
      BytesTotal := 0;
      BgColor := 0;
      Text := '';
      ImageName := '';
      MeterAvailable := false;
      MeterVisible := false;
      MeterWidth := 0;
      DebugString := '';
    end;
  end;
  {$ENDREGION}

  {$REGION 'procedure GatherDiskInfo(var P: TExtendedPanelInfo);' fold}
  //----------------------------------------------------------------------------
  procedure GatherDiskInfo(var P: TExtendedPanelInfo);
  //----------------------------------------------------------------------------
  var
    //buf: array[0..MAX_PATH] of WideChar;
    pRoot : PWideChar;
    r: TRect;
  begin
    P.Path := FarPanelString(P.Handle, FCTL_GETPANELDIR);

    if (P.Info.Plugin = 1) then
    begin
      P.Format := FarPanelString(P.Handle, FCTL_GETPANELFORMAT); // FAR 1657+
      P.HostFile := FarPanelString(P.Handle, FCTL_GETPANELHOSTFILE); // FAR 1657+
      P.Path := P.HostFile + P.Path;
    end else
    begin
      FDiskInfoCache.GetPathRoot(FSF, P.Path, pRoot);
      //FSF.GetPathRoot(PWideChar(String(P.Path)), buf, SizeOf(buf));

      FDiskInfoCache.GetDiskInfo(pRoot, sectorsPerCluster, bytesPerSector, freeClusters, totalClusters, P.DriveType);
      P.BytesFree := Int64(sectorsPerCluster) * Int64(bytesPerSector) * Int64(freeClusters);
      P.BytesTotal := Int64(sectorsPerCluster) * Int64(bytesPerSector) * Int64(totalClusters);

      P.MeterAvailable := (P.DriveType <> DRIVE_CDROM) and (P.Info.PanelType <> PTYPE_QVIEWPANEL);
      if P.MeterAvailable then
      begin
        r := PanelRectToClient(P.Info.PanelRect);
        P.MeterWidth := Round((r.Right - r.Left) * ((P.BytesTotal - P.BytesFree) / P.BytesTotal));
      end;
    end;
  end;
  {$ENDREGION}

  {$REGION 'procedure GatherPanelInfo(var P: TExtendedPanelInfo; Clear: Boolean = true; AHandle: THandle = 0);' fold}
  //----------------------------------------------------------------------------
  procedure GatherPanelInfo(var P: TExtendedPanelInfo; AType: Integer;
    Clear: Boolean = true; AHandle: THandle = 0);
  //----------------------------------------------------------------------------
  begin
    if Clear then
      ClearPanelInfo(P);

    P.WindowType := AType;

    if (AType = WTYPE_PANELS) then
    begin
      debugStrAdd(':3:1');

      P.Handle := AHandle;
      FARAPI.Control(AHandle, FCTL_GETPANELINFO, 0, @P.Info);
      if (P.Info.Visible = 1) then
        GatherDiskInfo(P);
      P.Rect := PanelRectToClient(P.Info.PanelRect);
    end else
    begin
      // for all windows panel rect is equal to client rect
      P.Rect := clientRect;
    end;
  end;
  {$ENDREGION}

  {$REGION 'procedure CalculatePanelStyleAgainstConfig(const AConfig: TScriptConfig; var P: TExtendedPanelInfo; ForceLoad: Boolean = false);' fold}
  //----------------------------------------------------------------------------
  procedure CalculatePanelStyleAgainstConfig(const AConfig: TScriptConfig;
    var P: TExtendedPanelInfo; ForceLoad: Boolean = false);
  //----------------------------------------------------------------------------
  var
    pRoot: PWideChar;
  begin
    debugStrAdd(':cps:1');

    if AConfig.FileAge = 0 then Exit; // no script

    // do not reload style for background window unless forced
    if (P.WindowType = WTYPE_BACKGROUND) and not ForceLoad then Exit;

    debugStrAdd(':cps:2');

    try
      // init output values

      AConfig.PutString('text', P.Text);
      AConfig.PutString('image', P.ImageName);
      AConfig.PutNumber('backgroundColor', P.BgColor);
      //AConfig.PutNumber('color', clWhite32);
      //AConfig.PutString('fontFamily', FBmp32.Font.Name);
      //AConfig.PutNumber('fontSize', FBmp32.Font.Size);
      AConfig.PutBoolean('usageMeterVisible', P.MeterVisible);
      //AConfig.PutNumber('usageMeterWidth', P.MeterWidth);

      debugStrAdd(':cps:3');

      // set common input values

      AConfig.PutNumber('windowType', P.WindowType);
      //AConfig.PutNumber('windowPos', P.WindowPos);
      AConfig.PutNumber('symbolWidth', symWidth);
      AConfig.PutNumber('symbolHeight', symHeight);

      debugStrAdd(':cps:4');

      // clean up irrelevant values

      if (P.WindowType <> WTYPE_PANELS) then
      begin
        debugStrAdd(':cps:5');

        AConfig.Delete('panelType');
        AConfig.Delete('isPlugin');
        AConfig.Delete('path');
        AConfig.Delete('volumeRoot');
        AConfig.Delete('driveType');
        AConfig.Delete('panelFormat');
        AConfig.Delete('panelHostFile');
        AConfig.Delete('bytesTotal');
        AConfig.Delete('bytesFree');
      end;

      if (P.WindowType <> WTYPE_VIEWER) then
      begin
        //AConfig.Delete('codepage');
        //AConfig.Delete('viewerHexMode');
      end;

      if (P.WindowType <> WTYPE_EDITOR) then
      begin
        //AConfig.Delete('codepage');
        //AConfig.Delete('editorModified');
        //AConfig.Delete('editorSaved');
        //AConfig.Delete('editorLocked');
      end;

      debugStrAdd(':cps:6');

      // set values

      if (P.WindowType = WTYPE_PANELS) then
      begin
        debugStrAdd(':cps:7');

        FDiskInfoCache.GetPathRoot(FSF, P.Path, pRoot);

        AConfig.PutNumber('panelType', P.Info.PanelType);
        AConfig.PutBoolean('isPlugin', (P.Info.Plugin = 1));
        AConfig.PutString('path', P.Path);
        AConfig.PutString('volumeRoot', pRoot);
        AConfig.PutNumber('driveType', P.DriveType);
        AConfig.PutString('panelFormat', P.Format);
        AConfig.PutString('panelHostFile', P.HostFile);
        AConfig.PutNumber('bytesTotal', P.BytesTotal);
        AConfig.PutNumber('bytesFree', P.BytesFree);
      end;

      if (P.WindowType = WTYPE_VIEWER) then
      begin
        //AConfig.PutString('codepage', ...);
        //AConfig.PutString('viewerHexMode', ...);
      end;

      if (P.WindowType = WTYPE_EDITOR) then
      begin
        //AConfig.PutBoolean('editorModified', ...);
        //AConfig.PutBoolean('editorSaved', ...);
        //AConfig.PutBoolean('editorLocked', ...);
      end;

      // run configuration script

      debugStrAdd(':cps:8');

      AConfig.EvalConfig;

      debugStrAdd(':cps:9');

      // get properties out of the script and back into P variable

      P.Text := AConfig.GetString('text');
      P.ImageName := AConfig.GetString('image');
      P.BgColor := TColor32(Trunc(AConfig.GetNumber('backgroundColor')));
      P.MeterVisible := AConfig.GetBoolean('usageMeterVisible');

      debugStrAdd(':cps:10');
    except
      on E:Exception do DebugShowError(Format('Script %s error: %s',
                                              [AConfig.FFileName, E.Message]));
    end;
  end;
  {$ENDREGION}

  {$REGION 'procedure CalculatePanelStyle(var P: TExtendedPanelInfo; ForceLoad: Boolean = false);' fold}
  //----------------------------------------------------------------------------
  procedure CalculatePanelStyle(var P: TExtendedPanelInfo; ForceLoad: Boolean = false);
  //----------------------------------------------------------------------------
  begin
    // set defaults
    P.MeterVisible := P.MeterAvailable;
    // calculate againts system and user configs
    CalculatePanelStyleAgainstConfig(FBaseConfig, P, ForceLoad);
    CalculatePanelStyleAgainstConfig(FUserConfig, P, ForceLoad);
  end;
  {$ENDREGION}

  {$REGION 'function PanelStyleHasChanged(const OldPanel: TExtendedPanelInfo; var P: TExtendedPanelInfo): Boolean;' fold}
  //----------------------------------------------------------------------------
  function PanelStyleHasChanged(const OldPanel: TExtendedPanelInfo; var P: TExtendedPanelInfo): Boolean;
  //----------------------------------------------------------------------------
  begin
    CalculatePanelStyle(P);

    Result :=
      (OldPanel.BgColor <> P.BgColor) or
      (OldPanel.Text <> P.Text) or
      (OldPanel.ImageName <> P.ImageName) or
      (OldPanel.Info.PanelRect.Left <> P.Info.PanelRect.Left) or
      (OldPanel.Info.PanelRect.Top <> P.Info.PanelRect.Top) or
      (OldPanel.Info.PanelRect.Right <> P.Info.PanelRect.Right) or
      (OldPanel.Info.PanelRect.Bottom <> P.Info.PanelRect.Bottom) or
      (OldPanel.MeterVisible <> P.MeterVisible) or
      (OldPanel.MeterWidth <> P.MeterWidth);
  end;
  {$ENDREGION}

  {$REGION 'procedure RenderPanel(const P: TExtendedPanelInfo);' fold}
  //----------------------------------------------------------------------------
  procedure RenderPanel(const P: TExtendedPanelInfo);
  //----------------------------------------------------------------------------
  var
    r: TRect;
    dst: TRect;
    h, s, l : Single;
    fileName: String;
  begin
    debugStrAdd(':rp:1');
    FBmp32.ClipRect := FBmp32.BoundsRect; // reset clipping rect

    r := P.Rect;

    // fill panel with slightly dimmed background color

    RGBToHSL(P.BgColor, h, s, l);
    l := l * 0.8;
    FBmp32.FillRectTS(r, HSLToRGB(h, s, l));

    debugStrAdd(':rp:2');

    InflateRect(r, -symWidth, -symHeight);
    FBmp32.ClipRect := r;
    try
      // draw image
      if (P.ImageName <> '') then
      begin
        debugStrAdd(':rp:3');
        fileName := FPluginFolder + P.ImageName;
        if FileExists(fileName) then
        begin
          debugStrAdd(':rp:4');
          // TODO: color should be set in config
          img := FImageCache.GetBmp(fileName{, clBlack32});
          if (img = nil) then
          begin
            debugStrAdd(':rp:5');
            DebugShowError(PWideChar(Format('Error loading image "%s"', [fileName])));
          end else
          begin
            debugStrAdd(':rp:6');
            img.MasterAlpha := $44; // TODO: should be in config (as global setting for text/images?)
            dst.Right := r.Right - symWidth * 1;
            dst.Bottom := r.Bottom - symHeight * 3;
            dst.Left := dst.Right - Round(FTextHeight*3 / img.Height * img.Width);
            dst.Top := dst.Bottom - FTextHeight*3;
            OffsetRect(dst, 0, - FTextHeight {- symHeight});
            img.DrawTo(
              FBmp32,
              dst,
              img.BoundsRect
            );
          end;
        end else
        begin
          debugStrAdd(':rp:7');
          DebugShowError(Format('File not found: "%s"', [P.ImageName]));
        end;
      end;

      debugStrAdd(':rp:8');

      // draw text
      if (P.Text <> '') then
      begin
        debugStrAdd(':rp:9');
        if (FBmp32.TextWidth(P.Text) < (r.Right - r.Left - symWidth * 2)) then
          r.Left := r.Right - FBmp32.TextWidth(P.Text) - symWidth * 2;

        FBmp32.RenderText(
          r.Left,
          r.Bottom - FBmp32.TextHeight(P.Text) - symHeight * 3,
          P.Text, 1, $44000000);
      end;
    finally
      debugStrAdd(':rp:10');
      FBmp32.ClipRect := FBmp32.BoundsRect; // reset clipping rect
    end;

    debugStrAdd(':rp:11');

    if (P.MeterVisible) then
    begin
      debugStrAdd(':rp:12');
      // draw usage meter background
      //TODO: Get geometry and colors from the config

      r := P.Rect;
      r.Top := r.Bottom - 3 * symHeight;
      FBmp32.FillRectTS(r, $44000000);

      // draw usage meter foreground
      r.Right := r.Left + P.MeterWidth;
      RGBToHSL(P.BgColor, h, s, l);
      l := l * 1.3;
      if l > 1 then l := 1;
      FBmp32.FillRectTS(r, HSLToRGB(h, s, l));
    end;

    // render non-panel area

    if (P.Rect.Top > 0) then
    begin
      debugStrAdd(':rp:13');
      r := P.Rect;
      r.Bottom := r.Top;
      r.Top := 0;
      FBmp32.FillRectS(r, FBackgroundPanel.BgColor);
    end;

    if (P.Rect.Bottom < clientRect.Bottom) then
    begin
      debugStrAdd(':rp:14');
      r := P.Rect;
      r.Top := r.Bottom;
      r.Bottom := clientRect.Bottom;
      FBmp32.FillRectS(r.Left, r.Top, r.Right, r.Bottom, FBackgroundPanel.BgColor);
    end;

    {$IFDEF ShowRenderingDebugStrings}
    P.DebugString := Format('[force=%d,type=%d,visible=%d] color=#%x, width=%d, text=%s, img=%s', [Ord(Force), Ord(P.Info.PanelType), ord(P.Info.Visible), P.BgColor, P.MeterWidth, P.Text, P.ImageName]) + ' ' + P.DebugString;
    {$ENDIF}

    debugStrAdd(':rp:15');
  end;
  {$ENDREGION}

  {$REGION 'function RenderPanelIfChanged(var FOldPanel, FPanel: TExtendedPanelInfo): Boolean;' fold}
  //----------------------------------------------------------------------------
  function RenderPanelIfChanged(var FOldPanel, FPanel: TExtendedPanelInfo): Boolean;
  //----------------------------------------------------------------------------
  begin
    debugStrAdd(':6');

    Result := false;

    if (FOldWindowType <> FWindowInfo.WindowType) then
    begin
      debugStrAdd(':6:1');

      // cleanup old data to force fresh render on window of any type
      ClearPanelInfo(FOldPanel);

      //FOldSymWidth := 0;
      //FOldSymHeight := 0;
      //ZeroMemory(@FOldClientRect, SizeOf(FOldClientRect));
    end;

    // calculate panel (window) style from the config

    if PanelStyleHasChanged(FOldPanel, FPanel) or force then
    begin
      debugStrAdd(':6:2');

      RenderPanel(FPanel);
      Result := true;
    end;

    debugStrAdd(':6:3');

    FOldPanel := FPanel;
  end;
  {$ENDREGION}

  {$REGION 'procedure DetectBackgroundWindow;' fold}
  //----------------------------------------------------------------------------
  procedure DetectBackgroundWindow;
  //----------------------------------------------------------------------------
  begin
    // based on current window type, determine which strings to peek for

    if (FWindowInfo.WindowType = WTYPE_PANELS) then
    begin
      // if at least one panel is visible, get its top left corner as a reference string
      if (FLeftPanel.Info.Visible = 1) then
        peekString := PeekStringAt(cr.Left + FLeftPanel.Info.PanelRect.Left, cr.Top + FLeftPanel.Info.PanelRect.Bottom, Length(PanelReferenceString))
      else if (FRightPanel.Info.Visible = 1) then
        peekString := PeekStringAt(cr.Left + FRightPanel.Info.PanelRect.Left, cr.Top + FRightPanel.Info.PanelRect.Bottom, Length(PanelReferenceString));
    end else
    begin
      // otherwise just use the top string
      peekString := PeekStringAt(cr.Left, cr.Top, cr.Right - cr.Left{PeekStringLength});
    end;

    debugStrAdd(':4');

    // if window type or window number (pos) changed
    if (FWindowInfo.WindowType <> FOldWindowType) or (FWindowInfo.Pos <> FOldWindowPos) then
    begin
      FOldWindowIsHidden := false;

      if (FWindowInfo.WindowType = WTYPE_PANELS) then
      begin
        FReferencePeekString := PanelReferenceString;
      end else
      if (FWindowInfo.WindowType = WTYPE_VIEWER) then
      begin
        ZeroMemory(@viewerInfo, SizeOf(viewerInfo));
        viewerInfo.StructSize := SizeOf(viewerInfo);
        FARAPI.ViewerControl(VCTL_GETINFO, @viewerInfo);
        s := viewerInfo.FileName;
        FReferencePeekString := Copy(s, Length(s) - PeekStringLength + 1, PeekStringLength);
      end else
      if (FWindowInfo.WindowType = WTYPE_EDITOR) then
      begin
        ZeroMemory(@buf, SizeOf(buf));
        FARAPI.EditorControl(ECTL_GETFILENAME, @buf);
        s := PWideChar(@buf);
        FReferencePeekString := Copy(s, Length(s) - PeekStringLength + 1, PeekStringLength);
      end;
    end;

    // compare peek string with the reference one
    // if strings do not match then treat window as hidden
    //windowIsHidden := (peekString <> FReferencePeekString);
    if (FWindowInfo.WindowType = WTYPE_PANELS) then
      windowIsHidden := (peekString <> FReferencePeekString)
    else
      windowIsHidden := (Pos(FReferencePeekString, peekString) = 0);

    if (windowIsHidden) then
      FWindowInfo.WindowType := WTYPE_BACKGROUND; // special window type
  end;
  {$ENDREGION}

//------------------------------------------------------------------------------
begin // procedure DoTheJob;
  debugStr := '';
  debugStrAdd(':1');

  try
    if not FEnabled or not IsUnderConEmu or not FCEInitialized then Exit;
    //if not FConfig.ScriptCompiled then Exit; we do want to run still, because the config might change

    needFullCheck := false;
    peekString := '';

    POldInfo := nil;
    PInfo := nil;

    if FDebug then
    begin
      QueryPerformanceFrequency(pf);
      QueryPerformanceCounter(pc1);
      pc2 := 0;
    end;

    // get info on currently active window

    ZeroMemory(@FWindowInfo, SizeOf(TWindowInfo));
    FWindowInfo.Pos := -1;
    FARAPI.AdvControl(FARAPI.ModuleNumber, ACTL_GETSHORTWINDOWINFO, @FWindowInfo);

    if (FWindowInfo.Pos = -1) then Exit; // FAR is shutting down (or is showing some dialog?)

    // get console and client rectangles, determine symbol dimensions

    cr := FarGetWindowRect;

    debugStrAdd(':2:1');

    if (FConEmuHWND <> 0) then
    begin
      GetClientRect(FConEmuHWND, clientRect);
    end else
      GetClientRect(FFarHWND, clientRect);

    debugStrAdd(':2:2');

    symWidth := (clientRect.Right - clientRect.Left) div (cr.Right - cr.Left);
    symHeight := (clientRect.Bottom - clientRect.Top) div (cr.Bottom - cr.Top);

    force := (FOldSymWidth <> symWidth) or
             (FOldSymHeight <> symHeight) or
             (FOldClientRect.Right <> clientRect.Right) or
             (FOldClientRect.Bottom <> clientRect.Bottom);

    if (force) then
    begin
      // recalculate font size
      FBmp32.Font.Size := 150;
      while (FBmp32.Font.Size > 30) and
            (FBmp32.TextWidth('XXXXXXXXXXXXXXX') > ((clientRect.Right - clientRect.Left) div 2)) do
      begin
        FBmp32.Font.Size := FBmp32.Font.Size - 5;
      end;
      FTextHeight := FBmp32.TextHeight('X');
    end;

    // nothing to be changed for popup windows (dialogs/help/menu)

    if (FWindowInfo.WindowType <> WTYPE_PANELS) and
       (FWindowInfo.WindowType <> WTYPE_VIEWER) and
       (FWindowInfo.WindowType <> WTYPE_EDITOR) then
      FWindowInfo.WindowType := FOldWindowType;

    debugStrAdd(':3');

    // we need to gather full information about current window and panels
    // as this will be used in DetectBackgroundWindow

    // prepare pointers to records based on window type (except for panels)

    if (FWindowInfo.WindowType = WTYPE_VIEWER) then
    begin
      POldInfo := @FOldViewerPanel;
      PInfo := @FViewerPanel;
    end;

    if (FWindowInfo.WindowType = WTYPE_EDITOR) then
    begin
      POldInfo := @FOldEditorPanel;
      PInfo := @FEditorPanel;
    end;

    // gather information about panels (windows)

    if (FWindowInfo.WindowType = WTYPE_PANELS) then
    begin
      FARAPI.Control(PANEL_ACTIVE, FCTL_GETPANELINFO, 0, @FLeftPanel.Info);
      if ((FLeftPanel.Info.Flags and PFLAGS_PANELLEFT) > 0) then
      begin
        GatherPanelInfo(FLeftPanel, FWindowInfo.WindowType, true, PANEL_ACTIVE);
        GatherPanelInfo(FRightPanel, FWindowInfo.WindowType, true, PANEL_PASSIVE);
      end else
      begin
        GatherPanelInfo(FLeftPanel, FWindowInfo.WindowType, true, PANEL_PASSIVE);
        GatherPanelInfo(FRightPanel, FWindowInfo.WindowType, true, PANEL_ACTIVE);
      end;
    end else
    begin
      Assert(Assigned(POldInfo), 'POldInfo is nil');
      Assert(Assigned(PInfo), 'PInfo is nil');
      GatherPanelInfo(PInfo^, FWindowInfo.WindowType);
    end;

    // see if background window (use console) is visible and
    // adjust FWindowInfo.WindowType accordingly

    DetectBackgroundWindow;

    // if background window was detected, prepare pointers
    // for background window as well

    if (FWindowInfo.WindowType = WTYPE_BACKGROUND) then
    begin
      POldInfo := @FOldBackgroundPanel;
      PInfo := @FBackgroundPanel;
      GatherPanelInfo(PInfo^, WTYPE_BACKGROUND, false); // do not clear info, just update
    end;

    // force redraw if window type has changed

    force := force or (FOldWindowType <> FWindowInfo.WindowType);

    // if window type or number has not changed,
    // do some additional checks to se if we need
    // to recalculate rendering at all

    if (FWindowInfo.WindowType = FOldWindowType) and
       (FWindowInfo.Pos = FOldWindowPos) then
    begin
      if (FWindowInfo.WindowType = WTYPE_PANELS) then
      begin
        // if this is a panels mode, check if its important properties has changed
        // that are likely to affect rendering
        if (FLeftPanel.Info.Visible <> FOldLeftPanel.Info.Visible) or
           (FLeftPanel.Info.PanelType <> FOldLeftPanel.Info.PanelType) or
           (FLeftPanel.Info.PanelRect.Left <> FOldLeftPanel.Info.PanelRect.Left) or
           (FLeftPanel.Info.PanelRect.Top <> FOldLeftPanel.Info.PanelRect.Top) or
           (FLeftPanel.Info.PanelRect.Right <> FOldLeftPanel.Info.PanelRect.Right) or
           (FLeftPanel.Info.PanelRect.Bottom <> FOldLeftPanel.Info.PanelRect.Bottom) or
           (FLeftPanel.Path <> FOldLeftPanel.Path) or
           ((FLeftPanel.Info.Visible = 1) and
             (
               (FLeftPanel.MeterAvailable <> FOldLeftPanel.MeterAvailable) or
               (FLeftPanel.MeterWidth <> FOldLeftPanel.MeterWidth)
             )
           )
        or
           (FRightPanel.Info.Visible <> FOldRightPanel.Info.Visible) or
           (FRightPanel.Info.PanelType <> FOldRightPanel.Info.PanelType) or
           (FRightPanel.Info.PanelRect.Left <> FOldRightPanel.Info.PanelRect.Left) or
           (FRightPanel.Info.PanelRect.Top <> FOldRightPanel.Info.PanelRect.Top) or
           (FRightPanel.Info.PanelRect.Right <> FOldRightPanel.Info.PanelRect.Right) or
           (FRightPanel.Info.PanelRect.Bottom <> FOldRightPanel.Info.PanelRect.Bottom) or
           (FRightPanel.Path <> FOldRightPanel.Path) or
           ((FRightPanel.Info.Visible = 1) and
             (
               (FRightPanel.MeterAvailable <> FOldRightPanel.MeterAvailable) or
               (FRightPanel.MeterWidth <> FOldRightPanel.MeterWidth)
             )
           )
        then
          needFullCheck := true;
      end
    end else
      needFullCheck := true;

    if not (needFullCheck or force) then Exit; // quit early

    // something has changed, and we need to do full recalculation

    debugStrAdd(':5');

    try
      // ask configs to reload if necessary
      if (FBaseConfig.ReloadConfigIfChanged) or
         (FUserConfig.ReloadConfigIfChanged) or
         ((FBaseConfig.ScriptCompiled) and (FOldWindowType = -1)) then
      begin
        // if config changed, reload style for background once
        // as it affects rendering of non-panel area
        CalculatePanelStyle(FBackgroundPanel, true); // force load background style
      end;
    except
      on E:Exception do
      begin
        DebugShowError('Script compilation error: ' + E.Message);
        //raise; otherwise it will flood the exception log
        //Exit; // so that error would be visible
      end;
    end;

    // exit if base script not found or can't be compiled
    if not FBaseConfig.ScriptCompiled then Exit;

    // exit if user script found *and* can't be compiled
    if (FUserConfig.FileAge > 0) and not FUserConfig.ScriptCompiled then Exit;

    // adjust bitmap size

    FBmp32.SetSize(clientRect.Right - clientRect.Left, clientRect.Bottom - clientRect.Top);

    try
      // check if window/panels have changed, and render them if necessary
      // or exit if they did not change

      if (FWindowInfo.WindowType = WTYPE_PANELS) then
      begin
        f1 := RenderPanelIfChanged(FOldLeftPanel, FLeftPanel);
        f2 := RenderPanelIfChanged(FOldRightPanel, FRightPanel);
        if not f1 and not f2 then Exit;
      end else
        if not RenderPanelIfChanged(POldInfo^, PInfo^) then Exit;

      debugStrAdd(':7');

      if FDebug then
        QueryPerformanceCounter(pc2);

      debugStrAdd(':8');

      if UpdateConEmuBackground then
        if FDebug then
        begin
          QueryPerformanceCounter(pc3);

          ShowStatus(Format(
            'Updated at %s [render:%.1fms + xfer:%.1fms = %.1fms]  ',
            [
              TimeToStr(Now),
              1000 * (pc2 - pc1) / pf,
              1000 * (pc3 - pc2) / pf,
              1000 * (pc3 - pc1) / pf
            ]));
        end;

      if FDebug then
      begin
        debugStr := ':8:2';

        try
          FBmp32.SaveToFile(FConEmuBackgroundImagePath);
        except
          // if first write failed, wait a bit and try saving again
          Sleep(50);
          try
            FBmp32.SaveToFile(FConEmuBackgroundImagePath);
          except
            //Beep;
          end;
        end;
      end;

      debugStrAdd(':9');

      {$IFDEF ShowRenderingDebugStrings}
      FARAPI.Text(2, 2, 12, PWideChar(Format('[ok1=%d] %s                          ', [Ord(ok1), FLeftPanel.DebugString])));
      FARAPI.Text(2, 3, 12, PWideChar(Format('[ok2=%d] %s                          ', [Ord(ok2), FRightPanel.DebugString])));
      FARAPI.Text(2, 5, 12, PWideChar(Format('WindowType=%d               ', [FWindowInfo.WindowType])));
      {$ENDIF}

      debugStrAdd(':10');
    finally
      {$IFDEF DumpVarsFile}
      DumpVarsFile;
      {$ENDIF}

      FOldWindowType := FWindowInfo.WindowType;
      FOldWindowPos := FWindowInfo.Pos;
      FOldWindowIsHidden := windowIsHidden;
      FOldSymWidth := symWidth;
      FOldSymHeight := symHeight;
      FOldClientRect := clientRect;
    end;
  except
    on E: Exception do
    begin
      if FDebug then
      begin
        //GetConsoleScreenBufferInfo(FOutHandle, sbi);
        s := '=================================' + CRLF +
             '[' + E.ClassName + ']' + CRLF +
             E.Message + CRLF +
             '=================================' + CRLF +
             debugStr + CRLF +
             //'----------' + CRLF +
             //Format('sbi.dwSize X=%d, Y=%d', [sbi.dwSize.X, sbi.dwSize.Y]) + CRLF +
             //Format('sbi.dwCursorPosition X=%d, Y=%d', [sbi.dwCursorPosition.X, sbi.dwCursorPosition.Y]) + CRLF +
             //Format('sbi.dwMaximumWindowSize X=%d, Y=%d', [sbi.dwMaximumWindowSize.X, sbi.dwMaximumWindowSize.Y]) + CRLF +
             //Format('sbi.srWindow Left=%d, Top=%d, Right=%d, Bottom=%d', [sbi.srWindow.Left, sbi.srWindow.Top, sbi.srWindow.Right, sbi.srWindow.Bottom]) + CRLF +
             //Format('sbi.wAttributes=%d', [sbi.wAttributes]) + CRLF +
             //'----------' + CRLF +
             //Format('FReferencePeekString="%s"', [FReferencePeekString]) + CRLF +
             //Format('PeekString="%s"', [peekString]) + CRLF +
             //Format('windowIsHidden=%d', [Ord(windowIsHidden)]) + CRLF +
             '----------' + CRLF +
             Format('WindowInfo Type=%d', [FWindowInfo.WindowType]) + CRLF +
             Format('Client rect: Left=%d, Top=%d, Right=%d, Bottom=%d', [clientRect.Left, clientRect.Top, clientRect.Right, clientRect.Bottom]) + CRLF +
             Format('FAR rect: Left=%d, Top=%d, Right=%d, Bottom=%d', [cr.Left, cr.Top, cr.Right, cr.Bottom]) + CRLF +
             '=================================' + CRLF;

        Log(s);
        Beep;
      end;
    end;
  end;
end;

//------------------------------------------------------------------------------
function ConEmuSynchroEvent(lParam :INT_PTR): Integer; stdcall;
//------------------------------------------------------------------------------
begin
  Result := 0;
  DoTheJob;
end;

(*
//------------------------------------------------------------------------------
procedure TWatcherThread.Execute;
//------------------------------------------------------------------------------
begin
  while not Terminated and not FInFarExit do
  begin
    //FARAPI.AdvControl(FARAPI.ModuleNumber, ACTL_SYNCHRO, nil);
    if FCEInitialized then
      CEAPI.SyncExecute(CEAPI.hPlugin, @ConEmuSynchroEvent, 0);
    Sleep(CONST_SLEEP_MS);
  end;
  // Какие-то глюки с завершением
  //TerminateThread(Handle, 0);
end;
*)

//------------------------------------------------------------------------------
function BackgroundTimerProc(lParam: INT_PTR) :Integer; stdcall;
//------------------------------------------------------------------------------
begin
  //TODO: Тут нужно оставить только проверку размера и свободного места на дисках!
  DoTheJob;
  Result := 0; // вернуть 1, если нужно обновление. но пока это делает DoTheJob
end;

////------------------------------------------------------------------------------
//function ProcessSynchroEventW(Event: Integer; Param: Pointer): Integer; stdcall;
////------------------------------------------------------------------------------
//begin
//  Result := 0;
//  if (Event <> SE_COMMONSYNCHRO) then Exit;
//  DoTheJob;
//end;

//------------------------------------------------------------------------------
procedure ExitFARW; stdcall;
//------------------------------------------------------------------------------
begin
  FInFarExit := true;
  FEnabled := false;
  (*
  if Assigned(FWatcherThread) then
  begin
    if not FWatcherThread.Terminated then begin
      FWatcherThread.Terminate;
      FWatcherThread.WaitFor;
      //TerminateThread(FWatcherThread.Handle, 0);
      //FWatcherThread.Destroy;
      //FWatcherThread.Terminate;
    end;
    FreeAndNil(FWatcherThread);
  end;
  *)
  if FRegistered then begin
    ConEmuUnRegister();
    FRegistered := false;
  end;
  //UpdateConEmuBackground; // clear background
  FreeAndNil(FDiskInfoCache);
  FreeAndNil(FImageCache);
  FreeAndNil(FBmp32);
  FreeAndNil(FUserConfig);
  FreeAndNil(FBaseConfig);
end;

//------------------------------------------------------------------------------
function DoUpdateBackground(var pBk: TPaintBackgroundArg) :Integer; stdcall;
//------------------------------------------------------------------------------
begin
  DoTheJob;
  if Assigned(FBmp32) then begin
    FBmp32.DrawTo(pBk.hdc, 0,0);
    Result := 1;
  end else begin
    Result := 0;
  end;
end;

//------------------------------------------------------------------------------
procedure OnConEmuLoaded(var csi: TConEmuLoadedArg); stdcall;
//------------------------------------------------------------------------------
begin
  FCEWrongVersion := false;
  if csi.cbSize < SizeOf(CEAPI) then begin
    FCEInitialized := false;
  end else begin
    Move(csi, CEAPI, SizeOf(CEAPI));
    BkCallback := @DoUpdateBackground;
    BkTimerProc := @BackgroundTimerProc;
    if (CEAPI.nBuildNo < 1012160) or (CEAPI.nBuildNo > 9912310) then begin
      FCEInitialized := false;
      FCEWrongVersion := true;
    end else begin
      FCEInitialized := (CEAPI.bLoaded<>0) and (CEAPI.bGuiActive<>0);
    end;
    if FFarInitialized and FCEInitialized then begin
      DoTheJob;
    end;
  end;
end;


//------------------------------------------------------------------------------

exports
  GetPluginInfoW,
  ConfigureW,
  SetStartupInfoW,
  //ProcessSynchroEventW,
  OnConEmuLoaded,
  ExitFARW;

begin
end.
