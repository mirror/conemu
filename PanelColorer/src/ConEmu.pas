(*

Panel Colorer Plugin v 0.94
ConEmu console emulator API bindings

Author: Igor Afanasyev <igor.afanasyev (at) gmail (dot) com>
        Maximus5 <ConEmu (dot) Maximus5 (at) gmail (dot) com>

License: GPL v.3 (see license.txt)

*)

unit ConEmu;

interface

uses
  Windows,
  SysUtils {for Format function};

type


  // ConEmu respond for CESERVER_REQ_SETBACKGROUND
  TSetBackgroundResult = (
    esbr_OK,               // All OK
    esbr_InvalidArg,       // Invalid *RegisterBackgroundArg
    esbr_PluginForbidden,  // "Allow plugins" unchecked in ConEmu settings ("Main" page)
    esbr_ConEmuInShutdown, // Console is closing. This is not an error, just information
    esbr_Unexpected,       // Unexpected error in ConEmu
	esbr_InvalidArgSize,   // Invalid RegisterBackgroundArg.cbSize
	esbr_InvalidArgProc    // Invalid RegisterBackgroundArg.PaintConEmuBackground
  );


type
  PBkPanelInfo = ^TBkPanelInfo;
  TBkPanelInfo = record
    bVisible: Integer; // Наличие панели
    bFocused: Integer; // В фокусе
    bPlugin:  Integer; // Плагиновая панель
    szCurDir: PWideChar; // Текущая папка на панели
    szFormat: PWideChar; // Доступно только в FAR2
    szHostFile: PWideChar; // Доступно только в FAR2
    rcPanelRect: TRect; // Консольные кооринаты панели. В FAR 2 с ключом /w верх может быть != {0,0}
  end;

  // Основной шрифт в GUI
  PConEmuMainFont = ^TConEmuMainFont;
  TConEmuMainFont = record
    sFontName: array[1..32] of WideChar;
    nFontHeight, nFontWidth, nFontCellWidth: DWORD;
    nFontQuality, nFontCharSet: DWORD;
    Bold, Italic: Integer;
    sBorderFontName: array[1..32] of WideChar;
    nBorderFontWidth: DWORD;
  end;

  PPaintBackgroundArg = ^TPaintBackgroundArg;
  TPaintBackgroundArg = record
  	cbSize              : Integer;

  	// указан при вызове RegisterBackground(rbc_Register)
    lParam: INT_PTR;
    // панели/редактор/вьювер: enum PaintBackgroundPlaces
    Place: Integer;
    // Слой, в котором вызван плагин. По сути, это порядковый номер, 0-based
    // если (nLevel > 0) плагин НЕ ДОЛЖЕН затирать фон целиком.
    dwLevel: DWORD;
    // [Reserved] комбинация из enum PaintBackgroundEvents
    dwEventFlags: DWORD;


    // Основной шрифт в GUI
    MainFont: TConEmuMainFont;
    // Палитра в ConEmu GUI (COLORREF)
    crPalette: array[0..15] of DWORD;
    // Reserved
    dwReserved: array[1..20] of DWORD;


    // DC для отрисовки фона. Изначально (для nLevel==0) DC залит цветом фона crPalette[0]
    hdc: HDC;
    // размер DC в пикселях
    dcSizeX, dcSizeY: Integer;
    // Координаты панелей в DC (base 0x0)
    rcDcLeft, rcDcRight: TRect;

    // Для облегчения жизни плагинам - текущие параметры FAR
    rcConWorkspace: TRect; // Кооринаты рабоче области FAR. В FAR 2 с ключом /w верх может быть != {0,0}
    conCursor: TCoord; // положение курсора, или {-1,-1} если он не видим
    nFarInterfaceSettings: DWORD; // ACTL_GETINTERFACESETTINGS
    nFarPanelSettings: DWORD; // ACTL_GETPANELSETTINGS
    nFarColors: array[0..255] of AnsiChar; // Массив цветов фара

    // Далее идет информация о консоли и FAR.
    bPanelsAllowed: Integer;
    LeftPanel, RightPanel: TBkPanelInfo; // -- subject to change

    // [OUT] Плагин должен указать, какие части консоли он "раскрасил" - enum PaintBackgroundPlaces
    dwDrawnPlaces: DWORD;
end;


  TCEAPI_PaintConEmuBackground = function(var pBk: TPaintBackgroundArg) :Integer; stdcall;

  // Если функция вернет 0 - обновление фона пока не требуется
  TCEAPI_BackgroundTimerProc = function(lParam: INT_PTR) :Integer; stdcall;

const
  rbc_Register   = 1; // Первичная регистрации "фонового" плагина
  rbc_Unregister = 2; // Убрать плагин из списка "фоновых"
  rbc_Redraw     = 3; // Если плагину требуется обновить фон - он зовет эту команду


type
  PRegisterBackgroundArg = ^TRegisterBackgroundArg;
  TRegisterBackgroundArg = record
    cbSize              : Integer;
    Cmd                 : Integer; // RegisterBackgroundCmd
    hPlugin             : INT_PTR; // Instance плагина, содержащего PaintConEmuBackground

    // Для дерегистрации всех калбэков достаточно вызвать {sizeof(RegisterBackgroundArg), rbc_Unregister, hPlugin}

    // Что вызывать для обновления фона.
    // Требуется заполнять только для Cmd==rbc_Register,
    // в остальных случаях команды игнорируются

    // Плагин может зарегистрировать несколько различных пар {PaintConEmuBackground,lParam}
    PaintConEmuBackground: TCEAPI_PaintConEmuBackground; // Собственно калбэк
    lParam: INT_PTR; // lParam будет передан в PaintConEmuBackground

    dwPlaces: Integer; // bitmask of PaintBackgroundPlaces
    dwEventFlags: DWORD; // bitmask of PaintBackgroundEvents

    // 0 - плагин предпочитает отрисовывать фон первым. Чем больше nSuggestedLevel
    // тем позднее может быть вызван плагин при обновлении фона
    nSuggestedLevel: Integer;

    // Необязательный калбэк таймера
    BackgroundTimerProc: TCEAPI_BackgroundTimerProc;
    // Рекомендованная частота опроса (ms)
    nBackgroundTimerElapse: DWORD;
  end; {TPluginStartupInfo}


type
  TCEAPI_SyncExecuteCallback = procedure(lParam :INT_PTR); stdcall;
  TCEAPI_GetFarHWND = function() :INT_PTR; stdcall;
  TCEAPI_GetFarHWND2 = function(abConEmuOnly: Integer) :INT_PTR; stdcall;
  TCEAPI_GetFarVersion = procedure(pfv: INT_PTR); stdcall;
  TCEAPI_IsTerminalMode = function() :Integer; stdcall;
  TCEAPI_IsConsoleActive = function() :Integer; stdcall;
  TCEAPI_RegisterPanelView = function(ppvi: INT_PTR) :Integer; stdcall;
  TCEAPI_RegisterBackground = function(pbk: PRegisterBackgroundArg) :Integer; stdcall;
  TCEAPI_ActivateConsole = function() :Integer; stdcall;
  TCEAPI_SyncExecute = function(hModule: INT_PTR; CallBack: TCEAPI_SyncExecuteCallback; lParam :INT_PTR) :Integer; stdcall;



// Аргумент для функции OnConEmuLoaded
type
  PConEmuLoadedArg = ^TConEmuLoadedArg;
  TConEmuLoadedArg = record
    cbSize              : Integer;
    nBuildNo            : DWORD;
    hConEmu             : INT_PTR;
    hPlugin             : INT_PTR;
    bLoaded             : Integer;
    bGuiActive          : Integer;

    // Сервисные функции
    GetFarHWND          : TCEAPI_GetFarHWND;
    GetFarHWND2         : TCEAPI_GetFarHWND2;
    GetFarVersion       : TCEAPI_GetFarVersion;
    IsTerminalMode      : TCEAPI_IsTerminalMode;
    IsConsoleActive     : TCEAPI_IsConsoleActive;
    RegisterPanelView   : TCEAPI_RegisterPanelView;
    RegisterBackground  : TCEAPI_RegisterBackground;
    ActivateConsole     : TCEAPI_ActivateConsole;
    SyncExecute         : TCEAPI_SyncExecute;
  end; {TPluginStartupInfo}

var
  CEAPI:       TConEmuLoadedArg;
  BkCallback:  TCEAPI_PaintConEmuBackground;
  BkTimerProc: TCEAPI_BackgroundTimerProc;



function GetConEmuHWND(FarHwnd: HWND; abRoot: Boolean): HWND;
function ConEmuRedraw(): Integer;
function ConEmuRegister(): Integer;
function ConEmuUnRegister(): Integer;

implementation


function GetConEmuHWND(FarHwnd: HWND; abRoot: Boolean): HWND;
begin
  if @CEAPI.GetFarHWND2<>nil then
    Result := CEAPI.GetFarHWND2(1)
  else
    Result := 0;
end;

// Trigger background redraw in the main thread
function ConEmuRedraw(): Integer;
var
  arg: TRegisterBackgroundArg;
begin
  ZeroMemory(@arg, SizeOf(arg));
  arg.cbSize := SizeOf(arg);
  arg.Cmd := rbc_Redraw;
  Result := CEAPI.RegisterBackground(@arg);
end;

function ConEmuRegister(): Integer;
var
  arg: TRegisterBackgroundArg;
begin
  ZeroMemory(@arg, SizeOf(arg));
  arg.cbSize := SizeOf(arg);
  arg.Cmd := rbc_Register;
  arg.hPlugin := CEAPI.hPlugin;
  arg.PaintConEmuBackground := BkCallback;
  arg.lParam := 0;
  arg.dwPlaces := 1; //pbp_Panels
  arg.dwEventFlags := 1; // pbe_Common+pbe_PanelDirectory
  arg.nSuggestedLevel := 0; // Самый нижний уровень отрисовки
  arg.BackgroundTimerProc := BkTimerProc;
  arg.nBackgroundTimerElapse := 1000;
  Result := CEAPI.RegisterBackground(@arg);
end;

function ConEmuUnRegister(): Integer;
var
  arg: TRegisterBackgroundArg;
begin
  ZeroMemory(@arg, SizeOf(arg));
  arg.cbSize := SizeOf(arg);
  arg.Cmd := rbc_Unregister;
  arg.hPlugin := CEAPI.hPlugin;
  Result := CEAPI.RegisterBackground(@arg);
end;

end.
