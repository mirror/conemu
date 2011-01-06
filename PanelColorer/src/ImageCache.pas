(*

Panel Colorer Plugin v 0.94

Author: Igor Afanasyev <igor.afanasyev (at) gmail (dot) com>

License: GPL v.3 (see license.txt)

*)

unit ImageCache;

interface

uses
  Classes,
  GR32;

type
  TImageCache = class(TObject)
    FList: TStringList;
  private
    function LoadAndPrepareImage(const Bmp32: TBitmap32; FileName: String): Boolean;
  public
    constructor Create;
    destructor Destroy; override;
    function GetBmp(FileName: String) : TBitmap32;
  end;

implementation

{ TImageCache }

//------------------------------------------------------------------------------
constructor TImageCache.Create;
//------------------------------------------------------------------------------
begin
  FList := TStringList.Create;
end;

//------------------------------------------------------------------------------
destructor TImageCache.Destroy;
//------------------------------------------------------------------------------
var
  i : Integer;
begin
  for i := 0 to FList.Count - 1 do
    TBitmap32(FList.Objects[i]).Free;

  FList.Free;
  inherited;
end;

// image should be a black/white or grayscale image *with no alpha*;
// alpha channel will be generated automatically based on red channel
// intensity; white will become transparent and black will become opaque
//------------------------------------------------------------------------------
function TImageCache.LoadAndPrepareImage(const Bmp32: TBitmap32; FileName: String{; Color : TColor32}): Boolean;
//------------------------------------------------------------------------------
var
  x, y : Integer;
  p : PColor32Array;
begin
  Result := true;
  try
    Bmp32.LoadFromFile(FileName); // this might throw an exception
    Bmp32.ResetAlpha;
    for y := 0 to Bmp32.Height - 1 do
    begin
      p := Bmp32.ScanLine[y];
      for x := 0 to Bmp32.Width - 1 do
        p[x] := SetAlpha(clBlack32{Color}, 255 - RedComponent(p[x]));
    end;
    Bmp32.DrawMode := dmBlend;
  except
    Result := false;
  end;
end;

//------------------------------------------------------------------------------
function TImageCache.GetBmp(FileName: String): TBitmap32;
//------------------------------------------------------------------------------
var
  i: Integer;
begin

  i := FList.IndexOf(FileName);
  if (i >= 0) then
  begin
    //TODO: reload image if file changed
    Result := TBitmap32(FList.Objects[i]);
    Exit;
  end;

  // image not found in cache, load

 Result := TBitmap32.Create;
  if LoadAndPrepareImage(Result, FileName) then
    FList.AddObject(FileName, Result)
  else
  begin
    Result.Free;
    Result := nil;
  end;
end;

end.
