(*

Panel Colorer Plugin v 0.94

Author: Igor Afanasyev <igor.afanasyev (at) gmail (dot) com>

License: GPL v.3 (see license.txt)

*)

unit DiskInfoCache;

interface

uses
  Windows,
  Classes,
  SysUtils,
  PluginW;

type
  TDiskInfo = record
    lpSectorsPerCluster,
    lpBytesPerSector,
    lpNumberOfFreeClusters,
    lpTotalNumberOfClusters: DWORD;
    lpDriveType: Cardinal;
    TimeStamp: TDateTime;
  end;
  PDiskInfo = ^TDiskInfo;

  TPathRoot = record
    TimeStamp: TDateTime;
    BufSize: Cardinal;
    PBuf: PWideChar;
  end;
  PPathRoot = ^TPathRoot;

  TDiskInfoCache = class(TObject)
    FDiskInfoList: TStringList;
    FPathRootList: TStringList;
  public
    constructor Create;
    destructor Destroy; override;
    procedure GetDiskInfo(RootPath: PWideChar;
      var lpSectorsPerCluster, lpBytesPerSector, lpNumberOfFreeClusters,
      lpTotalNumberOfClusters, lpDriveType: DWORD);
    procedure GetPathRoot(const FSF: TFarStandardFunctions; Path: String;
      var lpPathRoot: PWideChar);
  end;

const
  DISKINFO_CACHE_TIME = 1 / 24 / 60 / 60; // 1 second
  PATHROOT_CACHE_TIME = 1; // 1 day

implementation

{ TDiskInfoCache }

//------------------------------------------------------------------------------
constructor TDiskInfoCache.Create;
//------------------------------------------------------------------------------
begin
  FDiskInfoList := TStringList.Create;
  FPathRootList := TStringList.Create;
end;

//------------------------------------------------------------------------------
destructor TDiskInfoCache.Destroy;
//------------------------------------------------------------------------------
var
  i : Integer;
  ppr : PPathRoot;
begin
  for i := 0 to FPathRootList.Count - 1 do
  begin
    ppr := PPathRoot(FPathRootList.Objects[i]);
    FreeMem(ppr.PBuf, ppr.BufSize);
    FreeMem(ppr, SizeOf(TPathRoot));
  end;
  FPathRootList.Free;

  for i := 0 to FDiskInfoList.Count - 1 do
    FreeMem(Pointer(FDiskInfoList.Objects[i]), SizeOf(TDiskInfo));
  FDiskInfoList.Free;
  inherited;
end;

//------------------------------------------------------------------------------
procedure TDiskInfoCache.GetDiskInfo(RootPath: PWideChar;
  var lpSectorsPerCluster, lpBytesPerSector, lpNumberOfFreeClusters,
  lpTotalNumberOfClusters, lpDriveType: DWORD);
//------------------------------------------------------------------------------
var
  i: Integer;
  pdi: PDiskInfo;
begin
  i := FDiskInfoList.IndexOf(RootPath);
  if (i >= 0) then
  begin
    pdi := PDiskInfo(FDiskInfoList.Objects[i]);
    if ((Now - pdi.TimeStamp) < DISKINFO_CACHE_TIME) then
    begin
      lpSectorsPerCluster := pdi.lpSectorsPerCluster;
      lpBytesPerSector := pdi.lpBytesPerSector;
      lpNumberOfFreeClusters := pdi.lpNumberOfFreeClusters;
      lpTotalNumberOfClusters := pdi.lpTotalNumberOfClusters;
      lpDriveType := pdi.lpDriveType;
      Exit;
    end;
  end else
  begin
    GetMem(pdi, SizeOf(TDiskInfo));
    ZeroMemory(pdi, SizeOf(TDiskInfo));
    FDiskInfoList.AddObject(RootPath, TObject(pdi));
  end;

  GetDiskFreeSpace(PWideChar(RootPath), pdi.lpSectorsPerCluster,
    pdi.lpBytesPerSector, pdi.lpNumberOfFreeClusters, pdi.lpTotalNumberOfClusters);

  pdi.lpDriveType := GetDriveType(PWideChar(RootPath));

  pdi.TimeStamp := Now;

  lpSectorsPerCluster := pdi.lpSectorsPerCluster;
  lpBytesPerSector := pdi.lpBytesPerSector;
  lpNumberOfFreeClusters := pdi.lpNumberOfFreeClusters;
  lpTotalNumberOfClusters := pdi.lpTotalNumberOfClusters;
  lpDriveType := pdi.lpDriveType;
end;

//------------------------------------------------------------------------------
procedure TDiskInfoCache.GetPathRoot(const FSF: TFarStandardFunctions;
  Path: String; var lpPathRoot: PWideChar);
//------------------------------------------------------------------------------
var
  buf: array[0..MAX_PATH] of WideChar;
  bufSize: Integer;
  i: Integer;
  ppr: PPathRoot;
begin
  i := FPathRootList.IndexOf(Path);
  if (i >= 0) then
  begin
    ppr := PPathRoot(FPathRootList.Objects[i]);
    if ((Now - ppr.TimeStamp) < PATHROOT_CACHE_TIME) then
    begin
      lpPathRoot := ppr.PBuf;
      Exit;
    end;
  end else
  begin
    GetMem(ppr, SizeOf(TPathRoot));
    ZeroMemory(ppr, SizeOf(TPathRoot));
    FPathRootList.AddObject(Path, TObject(ppr));
  end;
  bufSize := FSF.GetPathRoot(PWideChar(Path), buf, SizeOf(buf));
  bufSize := bufSize * SizeOf(WideChar); // symbols to bytes
  if (bufSize <> ppr.BufSize) then
  begin
    if (ppr.PBuf <> nil) then
      FreeMem(ppr.PBuf, ppr.BufSize);
    GetMem(ppr.PBuf, bufSize);
    ppr.BufSize := bufSize;
  end;

  Move(buf, ppr.PBuf^, bufSize);

  ppr.TimeStamp := Now;

  lpPathRoot := ppr.PBuf;
end;

end.
