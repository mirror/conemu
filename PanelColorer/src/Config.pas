(*

Panel Colorer Plugin v 0.94
ConEmu console emulator API bindings

Author: Igor Afanasyev <igor.afanasyev (at) gmail (dot) com>

License: GPL v.3 (see license.txt)

*)

unit Config;

interface

uses
  //FastMM4,
  SysUtils,

  BESEN,
  BESENConstants,
  BESENStringUtils,
  BESENObject,
  BESENNumberUtils,
  BESENErrors,
  BESENCharset,
  BESENTypes,
  BESENValue,
  BESENASTNodes;

//------------------------------------------------------------------------------

type
  TScriptConfig = class(TObject)
    FInstance: TBESEN;
    FFileName: String;
    FFileAge: TDateTime;
    FScriptCompiled: Boolean;
  private
    procedure CreateInstanceFromFile(FileName: String);
  public
    constructor Create(FileName: String);
    destructor Destroy; override;
    function Eval(S: AnsiString): TBESENValue;
    function ReloadConfigIfChanged: Boolean;
    procedure EvalConfig;
    function GetBoolean(S: String): Boolean;
    function GetNumber(S: String): Double;
    function GetString(S: String): String;
    function IsDefined(S: String): Boolean;
    procedure PutBoolean(S: String; Value: Boolean);
    procedure PutNumber(S: String; Value: Double);
    procedure PutString(S: String; Value: String);
    procedure Delete(S: String);
    property ScriptCompiled: Boolean read FScriptCompiled;
    property FileAge: TDateTime read FFileAge;
  end;

//------------------------------------------------------------------------------

implementation

//------------------------------------------------------------------------------
constructor TScriptConfig.Create(FileName: String);
//------------------------------------------------------------------------------
begin
  CreateInstanceFromFile(FileName);
end;

//------------------------------------------------------------------------------
destructor TScriptConfig.Destroy;
//------------------------------------------------------------------------------
begin
  FInstance.Free;
  inherited;
end;

//------------------------------------------------------------------------------
procedure TScriptConfig.CreateInstanceFromFile(FileName: String);
//------------------------------------------------------------------------------
begin
  FScriptCompiled := false;
  FFileName := FileName;
  FFileAge := 0;
  SysUtils.FileAge(FileName, FFileAge);

  FInstance := TBESEN.Create(COMPAT_BESEN or COMPAT_JS);
  FInstance.RecursionLimit := 8;

  // this will compile and initialize the script (or give exception)
  // this can thow an exception
  FInstance.Execute(BESENConvertToUTF8(BESENGetFileContent(AnsiString(FFileName))));
  FScriptCompiled := true;
end;

//------------------------------------------------------------------------------
function TScriptConfig.Eval(S: AnsiString): TBESENValue;
//------------------------------------------------------------------------------
begin
  Result.ValueType := bvtUNDEFINED;
  if not FScriptCompiled then Exit;
  Result := FInstance.Eval(S); // this can throw an exception
end;

//------------------------------------------------------------------------------
procedure TScriptConfig.EvalConfig;
//------------------------------------------------------------------------------
begin
  Eval('main()');
end;

//------------------------------------------------------------------------------
function TScriptConfig.GetBoolean(S: String): Boolean;
//------------------------------------------------------------------------------
var
  v: TBESENValue;
begin
  FInstance.ObjectGlobal.Get(S, v);
  Result := FInstance.ToBool(v); // this can throw an exception
end;

//------------------------------------------------------------------------------
function TScriptConfig.GetNumber(S: String): Double;
//------------------------------------------------------------------------------
var
  v: TBESENValue;
begin
  FInstance.ObjectGlobal.Get(S, v);
  Result := FInstance.ToNum(v); // this can throw an exception
end;

//------------------------------------------------------------------------------
function TScriptConfig.GetString(S: String): String;
//------------------------------------------------------------------------------
var
  v: TBESENValue;
begin
  FInstance.ObjectGlobal.Get(S, v);
  Result := FInstance.ToStr(v); // this can throw an exception
end;

//------------------------------------------------------------------------------
function TScriptConfig.IsDefined(S: String): Boolean;
//------------------------------------------------------------------------------
var
  v: TBESENValue;
begin
  FInstance.ObjectGlobal.Get(S, v); // this can throw an exception
  Result := (v.ValueType <> bvtUNDEFINED);
end;

//------------------------------------------------------------------------------
procedure TScriptConfig.PutBoolean(S: String; Value: Boolean);
//------------------------------------------------------------------------------
var
  v: TBESENValue;
begin
  v.ValueType := bvtBOOLEAN;
  v.Bool := Value;
  FInstance.ObjectGlobal.Put(S, v, true); // this can throw an exception
end;

//------------------------------------------------------------------------------
procedure TScriptConfig.PutNumber(S: String; Value: Double);
//------------------------------------------------------------------------------
var
  v: TBESENValue;
begin
  v.ValueType := bvtNUMBER;
  v.Num := Value;
  FInstance.ObjectGlobal.Put(S, v, true); // this can throw an exception
end;

//------------------------------------------------------------------------------
procedure TScriptConfig.PutString(S, Value: String);
//------------------------------------------------------------------------------
var
  v: TBESENValue;
begin
  v.ValueType := bvtSTRING;
  v.Str := Value;
  FInstance.ObjectGlobal.Put(S, v, true); // this can throw an exception
end;

//------------------------------------------------------------------------------
function TScriptConfig.ReloadConfigIfChanged: Boolean;
//------------------------------------------------------------------------------
var
  age: TDateTime;
begin
  age := 0;
  SysUtils.FileAge(FFileName, age);
  Result := (age <> FFileAge);
  if Result then
  begin
    FInstance.Free;
    CreateInstanceFromFile(FFileName);
  end;
end;

//------------------------------------------------------------------------------
procedure TScriptConfig.Delete(S: String);
//------------------------------------------------------------------------------
begin
  FInstance.ObjectGlobal.Delete(S, true); // this can throw an exception
end;

//------------------------------------------------------------------------------

{$IFDEF cpu386}
var
  Old8087CW : Word;

initialization
  Old8087CW := Get8087CW;
  Set8087CW($133f);

finalization
  Set8087CW(Old8087CW);
{$ENDIF}

end.
