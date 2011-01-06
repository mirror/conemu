(*******************************************************************************
                        P R I M A R Y     L I C E N S E
********************************************************************************

BESEN is copyrighted free software by Benjamin Rosseaux <benjamin@rosseaux.com>.
You can redistribute it and/or modify it under either the terms of the AGPLv3
(see COPYING.txt file), or the conditions below:

  1. You may make and give away verbatim copies of the source form of this
     software without restriction, provided that you duplicate all of the
     original copyright notices and associated disclaimers.

  2. You may modify your copy of this software in any way, provided that
     you do at least ONE of the following:

       a) place your modifications in the Public Domain or otherwise
          make them freely available, such as by posting said
	        modifications to Usenet or an equivalent medium, or by allowing
	        the author to include your modifications in this software.

       b) use the modified software only within your corporation or
          organization.

       c) make other distribution arrangements with the author.

  3. You may distribute this software in object code or executable
     form, provided that you do at least ONE of the following:

       a) distribute the executables and library files of this software,
	        together with instructions (in the manual page or equivalent)
	        on where to get the original distribution.

       b) accompany the distribution with the machine-readable source of
      	  this software.

       c) make other distribution arrangements with the author.

  4. You are permitted to link this software with other software, to embed this
     software in your own software, or to build stand-alone binary or bytecode
     versions of applications that include this software, provided that you do
     at least ONE of the following:

       a) place the other software, if it is our own, in the Public Domain
          or otherwise make them together with machine-readable sources
          freely available, such as by posting said modifications to
          Usenet or an equivalent medium.

       b) use the other software, which includes this software, only within
          your corporation or organization and don't make it accessible or
          distribute it to the public / 3rd parties.

       c) make other distribution arrangements with the author.

  5. The scripts and library files supplied as input to or produced as
     output from the software do not automatically fall under the
     copyright of the software, but belong to whomever generated them,
     and may be sold commercially, and may be aggregated with this
     software.

  6. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
     "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
     LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
     FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
     COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
     INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
     BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
     OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
     AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
     OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
     THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
     DAMAGE.

********************************************************************************
                        S E C O N D A R Y    L I C E N S E
********************************************************************************

    BESEN - A ECMAScript Fifth Edition Object Pascal Implementation
    Copyright (C) 2009-2010  Benjamin 'BeRo' Rosseaux

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be usefufl,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*******************************************************************************)
unit BESENRandomGenerator;
{$i BESEN.inc}

interface

uses {$ifdef windows}Windows,MMSystem,{$endif}{$ifdef unix}dl,BaseUnix,Unix,
     UnixType,{$endif}SysUtils,Classes,Math,BESENConstants,BESENTypes,
     BESENObject,BESENValue,BESENCollectorObject;

type TBESENRandomGeneratorTable=array[0..BESEN_CMWCRND_SIZE-1] of longword;

     TBESENRandomGenerator=class(TBESENCollectorObject)
      public
       Table:TBESENRandomGeneratorTable;
       Position:longword;
       Carry:longword;
       constructor Create(AInstance:TObject); overload; override;
       destructor Destroy; override;
       procedure Reinitialize;
       function Get:longword;
       function GetNumber:TBESENNumber;
     end;

implementation

uses BESEN;

constructor TBESENRandomGenerator.Create(AInstance:TObject);
begin
 inherited Create(AInstance);
 Reinitialize;
end;

destructor TBESENRandomGenerator.Destroy;
begin
 inherited Destroy;
end;

{$ifdef win32}
type HCRYPTPROV=DWORD;

const PROV_RSA_FULL=1;
      CRYPT_VERIFYCONTEXT=$F0000000;

function CryptAcquireContext(var phProv:HCRYPTPROV;pszContainer:PAnsiChar;pszProvider:PAnsiChar;dwProvType:DWORD; dwFlags:DWORD):BOOL; stdcall; external advapi32 name 'CryptAcquireContextA';
function CryptReleaseContext(hProv: HCRYPTPROV; dwFlags: DWORD): BOOL;  stdcall; external advapi32 name 'CryptReleaseContext';
function CryptGenRandom(hProv: HCRYPTPROV; dwLen: DWORD; pbBuffer: Pointer): BOOL; stdcall; external advapi32 name 'CryptGenRandom';

function CoCreateGuid(var guid: TGUID): HResult; stdcall; external 'ole32.dll';
{$endif}

procedure TBESENRandomGenerator.Reinitialize;
const N=25;
      m=7;
      s=7;
      t=15;
      a=longword($8ebfd028);
      b=longword($2b5b2500);
      c=longword($db8b0000);
var LRG,LFSR,k,y,Seed1,Seed2:longword;
    i,j:integer;
    x:array[0..N-1] of longword;
    UnixTimeInMilliSeconds:int64;
{$ifdef unix}
    f:file of longword;
    ura,urb:longword;
{$else}
{$ifdef win32}
    lpc,lpf:int64;
    pp,p:pwidechar;
    st:TBESENANSISTRING;
{$endif}
{$endif}
{$ifdef win32}
 function GenerateRandomBytes(var Buffer;Bytes:Cardinal):boolean;
 var CryptProv:HCRYPTPROV;
 begin
  try
   if not CryptAcquireContext(CryptProv,nil,nil,PROV_RSA_FULL,CRYPT_VERIFYCONTEXT) then begin
    result:=false;
    exit;
   end;
   FillChar(Buffer,Bytes,#0);
   result:=CryptGenRandom(CryptProv,Bytes,@Buffer);
   CryptReleaseContext(CryptProv,0);
  except
   result:=false;
  end;
 end;
 function GetRandomGUIDGarbage:TBESENANSISTRING;
 var g:TGUID;
 begin
  CoCreateGUID(g);
  SetLength(result,sizeof(TGUID));
  move(g,result[1],sizeof(TGUID));
 end;
{$endif}
begin
 UnixTimeInMilliSeconds:=round((SysUtils.Now-25569.0)*86400000.0);
 Seed1:=longword(UnixTimeInMilliSeconds and $ffffffff)+longword(UnixTimeInMilliSeconds shr 32);
 Seed2:=longword(UnixTimeInMilliSeconds shr 32) xor not longword(UnixTimeInMilliSeconds and $ffffffff);
{$ifdef unix}
 ura:=0;
 urb:=0;
 AssignFile(f,'/dev/urandom');
 {$i-}System.reset(f,1);{$i+}
 if ioresult=0 then begin
  System.read(f,ura);
  System.read(f,urb);
  CloseFile(f);
 end else begin
  AssignFile(f,'/dev/random');
  {$i-}System.reset(f,1);{$i+}
  if ioresult=0 then begin
   System.read(f,ura);
   System.read(f,urb);
   CloseFile(f);
  end;
 end;
 Seed1:=Seed1 xor ura;
 Seed2:=Seed2 xor urb;
{$else}
{$ifdef win32}
 QueryPerformanceCounter(lpc);
 QueryPerformanceFrequency(lpf);
 inc(Seed1,timeGetTime+GetCurrentProcessId);
 dec(Seed2,GetTickCount-GetCurrentThreadId);
 inc(Seed1,paramcount);
 Seed1:=Seed1 xor (lpc shr 32);
 Seed2:=Seed2 xor lpc;
 Seed1:=(Seed1*lpc)+(Seed2*(lpf-Seed1));
 Seed2:=(Seed2*lpc)+(Seed1*(lpf-Seed2));
 pp:=GetEnvironmentStringsW;
 if assigned(pp) then begin
  p:=pp;
  while assigned(p) and (p^<>#0) do begin
   while assigned(p) and (p^<>#0) do begin
    inc(Seed1,(Seed2*word(p^)));
    Seed1:=(Seed1*1664525)+1013904223;
    Seed2:=Seed2 xor (Seed1*word(p^));
    Seed2:=Seed2 xor (Seed2 shl 13);
    Seed2:=Seed2 xor (Seed2 shr 17);
    Seed2:=Seed2 xor (Seed2 shl 5);
    inc(p);
   end;
   inc(p);
  end;
  FreeEnvironmentStringsW(pointer(p));
 end;
 pp:=pointer(GetCommandLineW);
 if assigned(pp) then begin
  p:=pp;
  while assigned(p) and (p^<>#0) do begin
   inc(Seed1,(Seed2*word(p^)));
   Seed1:=(Seed1*1664525)+1013904223;
   Seed2:=Seed2 xor (Seed1*word(p^));
   Seed2:=Seed2 xor (Seed2 shl 13);
   Seed2:=Seed2 xor (Seed2 shr 17);
   Seed2:=Seed2 xor (Seed2 shl 5);
   inc(p);
  end;
 end;
 SetLength(st,4096);
 if GenerateRandomBytes(st[1],length(st)) then begin
  for i:=1 to length(st) do begin
   inc(Seed1,(Seed2*byte(st[i])));
   Seed1:=(Seed1*1664525)+1013904223;
   Seed2:=Seed2 xor (Seed1*byte(st[i]));
   Seed2:=Seed2 xor (Seed2 shl 13);
   Seed2:=Seed2 xor (Seed2 shr 17);
   Seed2:=Seed2 xor (Seed2 shl 5);
  end;
 end;
 st:=GetRandomGUIDGarbage;
 for i:=1 to length(st) do begin
  inc(Seed1,(Seed2*byte(st[i])));
  Seed1:=(Seed1*1664525)+1013904223;
  Seed2:=Seed2 xor (Seed1*byte(st[i]));
  Seed2:=Seed2 xor (Seed2 shl 13);
  Seed2:=Seed2 xor (Seed2 shr 17);
  Seed2:=Seed2 xor (Seed2 shl 5);
 end;
 SetLength(st,0);
{$endif}
{$endif}
 LRG:=not Seed1;
 LFSR:=Seed2;
 for i:=0 to N-1 do begin
  LRG:=(LRG*1664525)+1013904223;
  LFSR:=LFSR xor (LFSR shl 13);
  LFSR:=LFSR xor (LFSR shr 17);
  LFSR:=LFSR xor (LFSR shl 5);
  x[i]:=LRG xor not LFSR;
 end;
 k:=N-1;
 LRG:=Seed1;
 LFSR:=not Seed2;
 for i:=0 to BESEN_CMWCRND_MASK do begin
  LRG:=(LRG*1664525)+1013904223;
  LFSR:=LFSR xor (LFSR shl 13);
  LFSR:=LFSR xor (LFSR shr 17);
  LFSR:=LFSR xor (LFSR shl 5);
  inc(k);
  if k>=N then begin
   for j:=0 to (N-m)-1 do begin
    x[j]:=x[j+m] xor (x[j] shr 1) xor ((x[j] and 1)*a);
   end;
   for j:=(N-m) to N-1 do begin
    x[j]:=x[j+m-N] xor (x[j] shr 1) xor ((x[j] and 1)*a);
   end;
   k:=0;
  end;
  y:=x[k] xor ((x[k] shl s) and b);
  y:=y xor ((y shl t) and c);
  Table[i]:=(LRG+LFSR) xor y;
 end;
 Position:=(x[LFSR and $f] xor Table[LRG and BESEN_CMWCRND_MASK]) and BESEN_CMWCRND_MASK;
end;

function TBESENRandomGenerator.Get:longword;
var t:{$ifdef fpc}qword{$else}int64{$endif};
    x:longword;
begin
 Position:=(Position+1) and BESEN_CMWCRND_MASK;
 t:=(BESEN_CMWCRND_A*Table[Position])+Carry;
 Carry:=t shr 32;
 x:=t+Carry;
 if x<Carry then begin
  inc(x);
  inc(Carry);
 end;
 result:=BESEN_CMWCRND_M-x;
 Table[Position]:=result;
end;

function TBESENRandomGenerator.GetNumber:TBESENNumber;
const f:TBESENNumber=1.0/int64($100000000);
var i:int64;
begin
 i:=Get;
 result:=i*f;
end;

end.
