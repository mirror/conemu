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
unit BESENDTOA;
{$i BESEN.inc}

(****************************************************************
 *
 * The original author of this software is David M. Gay.
 *
 * Copyright (c) 1991, 2000, 2001 by Lucent Technologies.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose without fee is hereby granted, provided that this entire notice
 * is included in all copies of any software which is or includes a copy
 * or modification of this software and in all copies of the supporting
 * documentation for such software.
 *
 * THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTY.  IN PARTICULAR, NEITHER THE AUTHOR NOR LUCENT MAKES ANY
 * REPRESENTATION OR WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY
 * OF THIS SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR PURPOSE.
 *
 ***************************************************************
 *
 * Extensions/Addons/Patches/Modifications from Benjamin Rosseaux are
 * copyrighted: Copyright (C) 2010 by Benjamin Rossseaux.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose without fee is hereby granted, provided that this entire notice
 * is included in all copies of any software which is or includes a copy
 * or modification of this software and in all copies of the supporting
 * documentation for such software.
 *
 * THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTY.  IN PARTICULAR, NEITHER THE AUTHOR NOR LUCENT MAKES ANY
 * REPRESENTATION OR WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY
 * OF THIS SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR PURPOSE.
 *
 ***************************************************************)

(* Please send bug reports to David M. Gay (dmg at acm dot org,
 * with " at " changed at "@" and " dot " changed to ".").  Or due to
 * extensions/addons/patches/modifications in this pascal port to
 * Benjamin Rosseaux (benjamin at rosseaux dot com, with " at " changed
 * at "@" and " dot " changed to ".")
 *)

interface

uses BESENConstants,BESENTypes;

const Round_zero=0;
      Round_near=1;
	    Round_up=2;
	    Round_down=3;

      DTOSTR_STANDARD=0;
      DTOSTR_STANDARD_EXPONENTIAL=1;
      DTOSTR_FIXED=2;
      DTOSTR_EXPONENTIAL=3;
      DTOSTR_PRECISION=4;

type PPUChar=^PUChar;
     PUChar=PAnsiChar;

     PInt=^TInt;
     TInt=smallint;

function strtod(s00:PUChar;se:PPUChar;octal:boolean=false):double;
procedure freedtoa(s:PUChar);
function dtoa(dd:double;mode,ndigits:TInt;decpt,sign:PInt;rve:PPUChar=nil;BiasUp:boolean=false):PUChar;
function dtobase(dd:double;base:TInt):PUChar;
function basetod(s:puchar;base:TInt):double;
function dtostr(dd:double;mode,precision:TInt):ansistring;

implementation

uses Math,SyncObjs;

type PULong=^TULong;
     TULong=longword;

     PULongs=^TULongs;
     TULongs=array[0..65535] of TULong;

     PLong=^TLong;
     TLong=longint;

     TUChar=ansichar;

     PU=^TU;
     TU=packed record
      case boolean of
       false:(d:double);
       true:({$ifdef BIG_ENDIAN}hi,lo{$else}lo,hi{$endif}:TULong);
     end;

     PBCinfo=^TBCinfo;
     TBCinfo=record
      dp0,dp1,dplen,dsign,e0,inexact,nd,nd0,rounding,scale,uflchk:TInt;
     end;

     PBigint=^TBigint;
     TBigint=record
      next:PBigint;
      k,maxwds,sign,wds:TInt;
      x:array[0..3] of TULong;
     end;

     PBigintFreeList=^TBigintFreeList;
     TBigintFreeList=array[0..7] of PBigint;

     PBigintFreeListCounter=^TBigintFreeListCounter;
     TBigintFreeListCounter=array[0..7] of TLong;

const DtoAFPUExceptionMask:TFPUExceptionMask=[exInvalidOp,exDenormalized,exZeroDivide,exOverflow,exUnderflow,exPrecision];
      DtoAFPUPrecisionMode:TFPUPrecisionMode=pmDOUBLE;
      DtoAFPURoundingMode:TFPURoundingMode=rmNEAREST;

      tens:array[0..22] of double=(1e0,1e1,1e2,1e3,1e4,1e5,1e6,1e7,1e8,1e9,1e10,1e11,1e12,1e13,1e14,1e15,1e16,1e17,1e18,1e19,1e20,1e21,1e22);
      bigtens:array[0..4] of double=(1e16,1e32,1e64,1e128,1e256);
      tinytens:array[0..4] of double=(1e-16,1e-32,1e-64,1e-128,{1e-256}9007199254740992.0*9007199254740992.e-256);

      dtoaModes:array[DTOSTR_STANDARD..DTOSTR_PRECISION] of byte=(0,0,3,2,2);

var BigintFreeList:TBigintFreeList;
    BigintFreeListCounter:TBigintFreeListCounter;
    p5s:PBigint;
    hexdig:array[byte] of byte;
    basedig:array[byte] of byte;
    DtoACriticalSection:TCriticalSection;

procedure strcpy(Dst,Src:PUChar);
begin
 while Src^<>#0 do begin
  Dst^:=Src^;
  inc(Dst);
  inc(Src);
 end;
 Dst^:=#0;
end;

function Balloc(k:TInt):PBigint;
var x:TInt;
    i:TLong;
begin
 result:=nil;
 if k<=high(TBigintFreeList) then begin
  DtoACriticalSection.Enter;
  try
  if BigintFreeListCounter[k]>0 then begin
    result:=BigintFreeList[k];
    if assigned(result) then begin
     BigintFreeList[k]:=result^.next;
     dec(BigintFreeListCounter[k]);
    end;
   end;
  finally
   DtoACriticalSection.Leave;
  end;
 end;
 if not assigned(result) then begin
  x:=1 shl k;
  if x<4 then begin
   i:=sizeof(TBigint);
  end else begin
   i:=sizeof(TBigint)+((x-4)*sizeof(TULong));
  end;
  GetMem(result,i);
  FillChar(result^,i,#0);
  result^.k:=k;
  result^.maxwds:=x;
 end;
 result^.next:=nil;
 result^.sign:=0;
 result^.wds:=0;
end;

procedure Bfree(v:PBigint);
var k:TLong;
begin
 if assigned(v) then begin
  k:=v^.k;
  if k<=high(TBigintFreeList) then begin
   DtoACriticalSection.Enter;
   try
    if BigintFreeListCounter[k]<16 then begin
     v^.next:=BigintFreeList[k];
     BigintFreeList[k]:=v;
     inc(BigintFreeListCounter[k]);
    end else begin
     FreeMem(v);
    end;
   finally
    DtoACriticalSection.Leave;
   end;
  end else begin
   FreeMem(v);
  end;
 end;
end;

procedure BBcopy(Dst,Src:PBigint);
{$ifdef UseNoMove}
var i,j,k:integer;
begin
 Dst^.sign:=Src^.sign;
 Dst^.wds:=Src^.wds;
 j:=Src^.wds;
 if j>0 then begin
  k:=0;
  for i:=1 to j shr 2 do begin
   Dst^.x[k+0]:=Src^.x[k+0];
   Dst^.x[k+1]:=Src^.x[k+1];
   Dst^.x[k+2]:=Src^.x[k+2];
   Dst^.x[k+3]:=Src^.x[k+3];
   inc(k,4);
  end;
  for i:=1 to j and 3 do begin
   Dst^.x[k]:=Src^.x[k];
   inc(k);
  end;
 end;
end;
{$else}
begin
 Move(Src^.sign,Dst^.sign,ptruint(ptruint(@Src^.x[Src^.wds])-ptruint(@Src^.sign)));
end;
{$endif}

procedure Bdump(b:PBigint;c:PUChar);
var i:Tint;
begin
 writeln(c,':');
 writeln('    ',b^.sign);
 writeln('    ',b^.wds);
 for i:=0 to b^.wds-1 do begin
  writeln('        ',b^.x[i]);
 end;
 writeln;
end;

function multadd(b:PBigint;m,a:TInt):PBigint;
var i,wds:TInt;
    x:PULong;
    y,xi,z,carry:TULong;
    bl:PBigint;
begin
 wds:=b^.wds;
 x:=@b^.x[0];
 if x^=1316134913 then begin
  if x^=1316134913 then begin
  end;
 end;
 i:=0;
 carry:=a;
 repeat
  xi:=x^;
	y:=((xi and $ffff)*TULong(m))+TULong(carry);
	z:=((xi shr 16)*TULong(m))+(y shr 16);
	carry:=z shr 16;
	x^:=(z shl 16)+(y and $ffff);
  inc(x);
  inc(i);
 until i>=wds;
 if carry<>0 then begin
	if wds>=b.maxwds then begin
   bl:=Balloc(b^.k+1);
   BBcopy(bl,b);
 	 Bfree(b);
	 b:=bl;
  end;
  b^.x[wds]:=carry;
  inc(wds);
	b^.wds:=wds;
 end;
 result:=b;
end;

function s2b(s:PUChar;nd0,nd:TInt;y9:TULong;dplen:TINt):PBigint;
var b:PBigint;
    i,k:TInt;
    x,y:TLong;
begin
 x:=(nd+8) div 9;
 k:=0;
 y:=1;
 while x>y do begin
  y:=y shl 1;
  inc(k);
 end;

 b:=Balloc(k);
 b^.x[0]:=y9;
 b^.wds:=1;

 i:=9;
 if 9<nd0 then begin
  inc(s,9);
	repeat
   b:=multadd(b,10,ord(s^)-ord('0'));
   inc(s);
   inc(i);
  until i>=nd0;
 	inc(s,dplen);
 end else begin
 	inc(s,dplen+9);
 end;
 while i<nd do begin
  b:=multadd(b,10,ord(s^)-ord('0'));
  inc(s);
  inc(i);
 end;
 result:=b;
end;

function hi0bits(x:TULong):TInt;
begin
 if (x and $ffff0000)=0 then begin
  result:=16;
  x:=x shl 16;
 end else begin
  result:=0;
 end;
 if (x and $ff000000)=0 then begin
  inc(result,8);
  x:=x shl 8;
 end;
 if (x and $f0000000)=0 then begin
  inc(result,4);
  x:=x shl 4;
 end;
 if (x and $c0000000)=0 then begin
  inc(result,2);
  x:=x shl 2;
 end;
 if (x and $80000000)=0 then begin
  inc(result);
  if (x and $40000000)=0 then begin
   result:=32;
  end;
 end;
end;

function lo0bits(y:PULong):TInt;
var x:TULong;
begin
 x:=y^;
 if (x and 7)<>0 then begin
  if (x and 1)<>0 then begin
   result:=0;
  end else if (x and 2)<>0 then begin
   y^:=x shr 1;
   result:=1;
  end else begin
   y^:=x shr 2;
   result:=2;
  end;
 end else begin
  if (x and $ffff)=0 then begin
   result:=16;
   x:=x shr 16;
  end else begin
   result:=0;
  end;
  if (x and $ff)=0 then begin
   inc(result,8);
   x:=x shr 8;
  end;
  if (x and $f)=0 then begin
   inc(result,4);
   x:=x shr 4;
  end;
  if (x and $3)=0 then begin
   inc(result,2);
   x:=x shr 2;
  end;
  if (x and 1)=0 then begin
   inc(result);
   x:=x shr 1;
   if (x and 1)=0 then begin
    result:=32;
    exit;
   end;
  end;
  y^:=x;
 end;
end;

function i2b(i:TInt):PBigint;
begin
 result:=BAlloc(1);
 result^.x[0]:=i;
 result^.wds:=1;
end;

function mult(a,b:PBigint):PBigint;
var c:PBigint;
    k,wa,wb,wc:TInt;
    x,xa,xae,xb,xbe,xc,xc0:PULong;
    y,carry,z,z2:TULong;
begin
 if a^.wds<b^.wds then begin
	c:=a;
  a:=b;
	b:=c;
 end;
 k:=a^.k;
 wa:=a^.wds;
 wb:=b^.wds;
 wc:=wa+wb;
 if wc>a^.maxwds then begin
  inc(k);
 end;
 c:=Balloc(k);
 x:=@c^.x[0];
 xa:=@c^.x[wc];
 while ptruint(x)<ptruint(xa) do begin
  x^:=0;
  inc(x);
 end;
 xa:=@a^.x[0];
 xae:=@a^.x[wa];
 xb:=@b^.x[0];
 xbe:=@b^.x[wb];
 xc0:=@c^.x[0];
 while ptruint(xb)<ptruint(xbe) do begin
  y:=xb^ and $ffff;
  if y<>0 then begin
   x:=xa;
	 xc:=xc0;
	 carry:=0;
   repeat
    z:=((x^ and $ffff)*y)+(xc^ and $ffff)+carry;
    carry:=z shr 16;
		z2:=((x^ shr 16)*y)+(xc^ shr 16)+carry;
    inc(x);
    carry:=z2 shr 16;
    xc^:=(z2 shl 16) or (z and $ffff);
    inc(xc);
   until ptruint(x)>=ptruint(xae);
	 xc^:=carry;
  end;
  y:=xb^ shr 16;
  if y<>0 then begin
   x:=xa;
	 xc:=xc0;
	 carry:=0;
   z2:=xc^;
   repeat
    z:=((x^ and $ffff)*y)+(xc^ shr 16)+carry;
    carry:=z shr 16;
    xc^:=(z shl 16) or (z2 and $ffff);
    inc(xc);
		z2:=((x^ shr 16)*y)+(xc^ and $ffff)+carry;
    inc(x);
    carry:=z2 shr 16;
   until ptruint(x)>=ptruint(xae);
	 xc^:=z2;
  end;
  inc(xb);
  inc(xc0);
 end;
 xc:=@c^.x[wc];
 dec(xc);
 while (wc>0) and (xc^=0) do begin
  dec(wc);
  dec(xc);
 end;
 c^.wds:=wc;
 result:=c;
end;

function pow5mult(b:PBigint;k:TInt):PBigint;
const p05:array[0..2] of TInt=(5,25,125);
var b1,p5,p51:PBigint;
    i:TInt;
begin
 i:=k and 3;
 if i<>0 then begin
	b:=multadd(b,p05[i-1],0);
 end;

 k:=k shr 2;
 if k=0 then begin
  result:=b;
  exit;
 end;

 p5:=p5s;
 if not assigned(p5) then begin
  DtoACriticalSection.Enter;
  try
   p5:=p5s;
   if not assigned(p5) then begin
    p5:=i2b(625);
    p5.next:=nil;
    p5s:=p5;
   end;
  finally
   DtoACriticalSection.Leave;
  end;
 end;

 while true do begin
  if (k and 1)<>0 then begin
	 b1:=mult(b,p5);
   Bfree(b);
	 b:=b1;
  end;
  k:=k shr 1;
  if k=0 then begin
   break;
  end;
  p51:=p5^.next;
  if not assigned(p51) then begin
   DtoACriticalSection.Enter;
   try
    p51:=p5^.next;
    if not assigned(p51) then begin
     p51:=mult(p5,p5);
     p51^.next:=nil;
     p5^.next:=p51;
    end;
   finally
    DtoACriticalSection.Leave;
   end;
  end;
  p5:=p51;
 end;
 result:=b;
end;

function lshift(b:PBigint;k:TInt):PBigint;
var i,k1,n,n1:TInt;
	  b1:PBigint;
    z:TULong;
	  x,x1,xe:PULong;
begin
 n:=k shr 5;

 k1:=b^.k;
 n1:=n+b^.wds+1;
 i:=b^.maxwds;
 while n1>i do begin
  inc(k1);
  i:=i shl 1;
 end;
 b1:=Balloc(k1);
 x1:=@b1^.x[0];
 for i:=0 to n-1 do begin
  x1^:=0;
  inc(x1);
 end;
 x:=@b^.x[0];
 xe:=@b^.x[b^.wds];

 k:=k and $1f;
 if k<>0 then begin
 	k1:=32-k;
  z:=0;
  repeat
   x1^:=(x^ shl k) or z;
   z:=x^ shr k1;
   inc(x1);
   inc(x);
  until ptruint(x)>=ptruint(xe);
  x1^:=z;
  if z<>0 then begin
   inc(n1);
  end;
 end else begin
  repeat
   x1^:=x^;
   inc(x1);
   inc(x);
  until ptruint(x)>=ptruint(xe);
 end;
 b1^.wds:=n1-1;
 Bfree(b);
 result:=b1;
end;

function cmp(a,b:PBigint):TInt;
var xa,xa0,xb:PULong;
	  i,j:TInt;
begin
 i:=a^.wds;
 j:=b^.wds;
 dec(i,j);
 if i<>0 then begin
  result:=i;
  exit;
 end;
 xa0:=@a^.x[0];
 xa:=@a^.x[j];
 xb:=@b^.x[j];
 while true do begin
  dec(xa);
  dec(xb);
  if xa^<>xb^ then begin
   if xa^<xb^ then begin
    result:=-1;
   end else begin
    result:=1;
   end;
   exit;
  end;
  if ptruint(xa)<=ptruint(xa0) then begin
   break;
  end;
 end;
 result:=0;
end;

function diff(a,b:PBigint):PBigint;
var c:PBigint;
	  i,wa,wb:TInt;
    xa,xae,xb,xbe,xc:PULong;
    borrow,y,z:TULong;
begin
 i:=cmp(a,b);
 if i=0 then begin
 	c:=Balloc(0);
  c^.wds:=1;
  c^.x[0]:=0;
  result:=c;
  exit;
 end;
 if i<0 then begin
	c:=a;
	a:=b;
	b:=c;
	i:=1;
 end else begin
  i:=0;
 end;

 c:=Balloc(a^.k);
 c^.sign:=i;
 wa:=a^.wds;
 xa:=@a^.x[0];
 xae:=@a^.x[wa];
 wb:=b^.wds;
 xb:=@b^.x[0];
 xbe:=@b^.x[wb];
 xc:=@c^.x[0];
 borrow:=0;

 repeat
	y:=((xa^ and $ffff)-(xb^ and $ffff))-borrow;
	borrow:=(y and $10000) shr 16;
	z:=((xa^ shr 16)-(xb^ shr 16))-borrow;
	borrow:=(z and $10000) shr 16;
  xc^:=(z shl 16) or (y and $ffff);
  inc(xa);
  inc(xb);
  inc(xc);
 until ptruint(xb)>=ptruint(xbe);

 while ptruint(xa)<ptruint(xae) do begin
	y:=(xa^ and $ffff)-borrow;
	borrow:=(y and $10000) shr 16;
	z:=(xa^ shr 16)-borrow;
	borrow:=(z and $10000) shr 16;
  xc^:=(z shl 16) or (y and $ffff);
  inc(xa);
  inc(xc);
 end;

 dec(xc);
 while xc^=0 do begin
  dec(wa);
  dec(xc);
 end;

 c^.wds:=wa;
 result:=c;
end;

function ulp(x:PU):double;
var u:TU;
begin
 u.hi:=(x^.hi and $7ff00000)-((53-1)*$100000);
 u.lo:=0;
 result:=u.d;
end;

function b2d(a:PBigint;e:PInt):double;
var xa,xa0:PULong;
    w,y,z:TULong;
    k:TInt;
    d:TU;
begin
 xa0:=@a^.x[0];
 xa:=@a^.x[a^.wds];
 dec(xa);
 y:=xa^;

 k:=hi0bits(y);
 e^:=32-k;

 if k<11 then begin
  d.hi:=$3ff00000 or (y shr (11-k));
  if ptruint(xa)>ptruint(xa0) then begin
   dec(xa);
   w:=xa^;
  end else begin
   w:=0;
  end;
  d.lo:=(y shl ((32-11)+k)) or (w shr (11-k));
  result:=d.d;
  exit;
 end;

 if ptruint(xa)>ptruint(xa0) then begin
  dec(xa);
  z:=xa^;
 end else begin
  z:=0;
 end;
 dec(k,11);
 if k<>0 then begin
  d.hi:=$3ff00000 or (y shl k) or (z shr (32-k));
  if ptruint(xa)>ptruint(xa0) then begin
   dec(xa);
   y:=xa^;
  end else begin
   y:=0;
  end;
  d.lo:=(z shl k) or (y shr (32-k));
 end else begin
  d.hi:=$3ff00000 or y;
  d.lo:=z;
 end;

 result:=d.d;
end;

function d2b(d:PU;e,bits:PInt):PBigint;
var b:PBigint;
   	de,k,i:TInt;
    x:PULongs;
    y,z:TULong;
begin
 b:=Balloc(1);
 x:=@b^.x[0];

 z:=d^.hi and $fffff;
 d^.hi:=d^.hi and $7fffffff;

 de:=d^.hi shr 20;
 if de<>0 then begin
  z:=z or $100000;
 end;

 y:=d^.lo;
 if y<>0 then begin
  k:=lo0bits(@y);
  if k<>0 then begin
   x^[0]:=y or (z shl (32-k));
   z:=z shr k;
  end else begin
   x^[0]:=y;
  end;
  x^[1]:=z;
  if z<>0 then begin
   i:=2;
  end else begin
   i:=1;
  end;
  b^.wds:=i;
 end else begin
 	k:=lo0bits(@z);
  x^[0]:=z;
  i:=1;
  b^.wds:=i;
  inc(k,32);
 end;

 if de<>0 then begin
  e^:=((de-1023)-(53-1))+k;
  bits^:=53-k;
 end else begin
  e^:=(((de-1023)-(53-1))+1)+k;
  bits^:=(32*i)-hi0bits(x^[i-1]);
 end;

 result:=b;
end;

function ratio(a,b:PBigint):double;
var da,db:TU;
    k,ka,kb:TInt;
begin
 da.d:=b2d(a,@ka);
 db.d:=b2d(b,@kb);

 k:=(ka-kb)+(32*(a^.wds-b^.wds));

 if k>0 then begin
  inc(da.hi,k*$100000);
 end else begin
  k:=-k;
  inc(db.hi,k*$100000);
 end;

 result:=da.d/db.d;
end;

procedure htinit(h,s:PUChar;incr:TInt);
var i,j:TInt;
begin
 i:=0;
 j:=byte(s[i]);
 while j<>0 do begin
	byte(h[j]):=i+incr;
  inc(i);
  j:=byte(s[i]);
 end;
end;

procedure hexdig_init;
begin
 FillChar(hexdig,sizeof(hexdig),#0);
 htinit(pointer(@hexdig),'0123456789',$10);
 htinit(pointer(@hexdig),'abcdef',$10+10);
 htinit(pointer(@hexdig),'ABCDEF',$10+10);
end;

procedure basedig_init;
begin
 FillChar(basedig,sizeof(basedig),#0);
 htinit(pointer(@basedig),'0123456789',$80);
 htinit(pointer(@basedig),'ABCDEFGHIJKLMNOPQRSTUVWXYZ',$80+10);
 htinit(pointer(@basedig),'abcdefghijklmnopqrstuvwxyz',$80+10);
end;

function match(sp:PPUChar;t:PUChar):TInt;
var c,d:TInt;
    s:PUchar;
begin
 s:=sp^;
 d:=byte(t^);
 inc(t);
 while d<>0 do begin
  inc(s);
  c:=byte(s^);
  if (c>=ord('A')) and (c<=ord('Z')) then begin
   inc(c,ord('a')-ord('A'));
  end;
  if c<>d then begin
   result:=0;
   exit;
  end;
  d:=byte(t^);
  inc(t);
 end;
 inc(s);
 sp^:=s;
 result:=1;
end;

procedure hexnan(rvp:PU;sp:PPUChar);
var c:TULong;
    x:array[0..1] of TULong;
    s:PUChar;
    c1,havedig,udx0,xshift:TInt;
begin
 if hexdig[ord('0')]=0 then begin
  hexdig_init;
 end;

 x[0]:=0;
 x[1]:=0;
 havedig:=0;
 xshift:=0;
 udx0:=1;
 s:=sp^;

 c:=byte(s[1]);
 while (c<>0) and (c<=ord(' ')) do begin
  inc(s);
  c:=byte(s[1]);
 end;

 if (s[1]='0') and ((s[2]='x') or (s[2]='X'))  then begin
  inc(s,2);
 end;

 inc(s);
 c:=byte(s^);
 while c<>0 do begin
  c1:=hexdig[c];
  if c1<>0 then begin
   c:=c1 and $f;
  end else begin
   if c<=ord(' ') then begin
    if (udx0<>0) and (havedig<>0) then begin
     udx0:=0;
     xshift:=1;
    end;
    continue;
   end else begin
    repeat
     if c=ord(')') then begin
      sp^:=@s[1];
      break;
     end;
     inc(s);
     c:=byte(s^);
    until c=0;
		break;
   end;
  end;

	havedig:=1;
	if xshift<>0 then begin
   xshift:=0;
	 x[0]:=x[1];
 	 x[1]:=0;
  end;
  if udx0<>0 then begin
 	 x[0]:=(x[0] shl 4) or (x[1] shl 28);
  end;
	x[1]:=(x[1] shl 4) or c;
  inc(s);
  c:=byte(s^);
 end;
 x[0]:=x[0] and $fffff;
 if (x[0]<>0) or (x[1]<>0) then begin
  rvp.hi:=$7ff00000 or x[0];
  rvp.lo:=x[1];
 end;
end;

procedure rshift(b:PBigint;k:TInt);
var x,x1,xe:PULong;
    y:TULong;
    n:TInt;
begin
 x:=@b^.x[0];
 x1:=@b^.x[0];
 n:=k shr 5;
 if n<b^.wds then begin
  xe:=@b^.x[b^.wds];
  inc(x,n);
  k:=k and 31;
	if k<>0 then begin
   n:=32-k;
   y:=x^ shr k;
   inc(x);
   while ptruint(x)<ptruint(xe) do begin
		x1^:=(y or (x^ shl n)) and $ffffffff;
  	y:=x^ shr k;
    inc(x1);
    inc(x);
   end;
   x1^:=y;
   if y<>0 then begin
    inc(x1);
   end;
  end else begin
   while ptruint(x)<ptruint(xe) do begin
		x1^:=x^;
    inc(x1);
    inc(x);
   end;
  end;
 end;
 b^.wds:=(ptruint(x1)-ptruint(@b^.x)) div sizeof(TULong);
 if b^.wds=0 then begin
 	b^.x[0]:=0;
 end;
end;

function any_on(b:PBigint;k:TInt):TULong;
var n,nwds:TInt;
    x,x0:PULong;
    x1,x2:TULong;
begin
 x:=@b^.x[0];
 nwds:=b^.wds;
 n:=k shr 5;
 if n>nwds then begin
  n:=nwds;
 end else begin
  k:=k and 31;
  if (n<nwds) and (k<>0) then begin
   x1:=PULongs(x)^[n];
   x2:=x1;
   x1:=x1 shr k;
   x1:=x1 shl k;
   if x1<>x2 then begin
    result:=1;
    exit;
   end;
  end;
 end;
 x0:=x;
 inc(x,n);
 while ptruint(x)>ptruint(x0) do begin
  dec(x);
  if x^<>0 then begin
   result:=1;
   exit;
  end;
 end;
 result:=0;
end;

function increment(b:PBigint):PBigint;
var x,xe:PULong;
    bl:PBigint;
begin
 x:=@b^.x[0];
 xe:=@b^.x[b^.wds];
 repeat
	if x^<$ffffffff then begin
   inc(x^);
   result:=b;
   exit;
  end;
	x^:=0;
  inc(x);
 until ptruint(x)>=ptruint(xe);
 if b^.wds>=b^.maxwds then begin
 	bl:=Balloc(b^.k+1);
  BBcopy(bl,b);
	Bfree(b);
	b:=bl;
 end;
 b^.x[b^.wds]:=1;
 inc(b^.wds);
 result:=b;
end;

procedure gethex(sp:PPUChar;rvp:PU;rounding,sign:TInt);
label pcheck,ret_tiny,ret_big,retz1,retz,ovfl,ovfl1;
const emax=(($7fe-1023)-53)+1;
	   	emin=((-1022)-53)+1;
var b:PBigint;
    decpt,s0,s,s1:PUChar;
    e,e1:TLong;
    L,lostbits:TULong;
    x:PULong;
    big,denorm,esign,havedig,k,n,nbits,up,zret:TInt;
begin
 if hexdig[ord('0')]=0 then begin
  hexdig_init;
 end;

 denorm:=0;
 up:=0;
 havedig:=0;

 s0:=@sp^[2];
 while s0[havedig]='0' do begin
  inc(havedig);
 end;
 inc(s0,havedig);
 s:=s0;
 decpt:=nil;
 zret:=0;
 e:=0;
 if hexdig[byte(s^)]<>0 then begin
  inc(havedig);
 end else begin
  zret:=1;
  if s^<>'.' then begin
   goto pcheck;
  end;
  inc(s);
  decpt:=s;

  if hexdig[byte(s^)]=0 then begin
   goto pcheck;
  end;

	while s^='0' do begin
   inc(s);
  end;

  if hexdig[byte(s^)]<>0 then begin
   zret:=0;
  end;

	havedig:=1;
  s0:=s;
 end;
 while hexdig[byte(s^)]<>0 do begin
  inc(s);
 end;
 if (s^='.') and not assigned(decpt) then begin
  inc(s);
	decpt:=s;
  while hexdig[byte(s^)]<>0 do begin
   inc(s);
  end;
 end;
 if assigned(decpt) then begin
  e:=-((s-decpt) shl 2);
 end;
 pcheck:
 s1:=s;
 big:=0;
 esign:=0;
 case s^ of
  'p','P':begin
   inc(s);
   case s^ of
    '-':begin
     esign:=1;
     inc(s);
    end;
    '+':begin
     inc(s);
    end;
   end;
   n:=hexdig[byte(s^)];
   if (n=0) or (n>$19) then begin
    s:=s1;
   end else begin
    e1:=n-$10;
    inc(s);
    n:=hexdig[byte(s^)];
    while (n<>0) and (n<=$19) do begin
     if (e1 and $f8000000)<>0 then begin
      big:=1;
     end;
     e1:=((10*e1)+n)-$10;
     inc(s);
     n:=hexdig[byte(s^)];
    end;
    if esign<>0 then begin
     e1:=-e1;
    end;
    inc(e,e1);
   end;
	end;
 end;
 sp^:=s;
 if havedig=0 then begin
	sp^:=@s0[-1];
 end;
 if zret<>0 then begin
  goto retz1;
 end;
 if big<>0 then begin
  if esign<>0 then begin
   case rounding of
    Round_up:begin
     if sign=0 then begin
      goto ret_tiny;
     end;
    end;
    Round_down:begin
     if sign<>0 then begin
      goto ret_tiny;
     end;
    end;
   end;
   goto retz;
   ret_tiny:
   rvp.hi:=0;
   rvp.lo:=1;
   exit;
  end;
  case rounding of
   Round_near:begin
    goto ovfl1;
   end;
   Round_up:begin
    if sign=0 then begin
     goto ovfl1;
    end else begin
     goto ret_big;
    end;
   end;
   Round_down:begin
    if sign<>0 then begin
     goto ovfl1;
    end else begin
     goto ret_big;
    end;
   end;
  end;
  ret_big:
  rvp.hi:=($fffff or ($100000*((308+1023)-1)));
	rvp.lo:=$ffffffff;
	exit;
 end;
 n:=(s1-s0)-1;
 k:=0;
 while n>((1 shl (5-2))-1) do begin
  inc(k);
  n:=n shr 1;
 end;

 b:=Balloc(k);
 x:=@b^.x[0];
 n:=0;
 L:=0;

 while ptruint(s1)>ptruint(s0) do begin
  dec(s1);
  if s1^='.' then begin
   continue;
  end;
	if n=32 then begin
   x^:=L;
   inc(x);
   L:=0;
	 n:=0;
  end;
	L:=L or ((hexdig[byte(s1^)] and $f) shl n);
	inc(n,4);
 end;
 x^:=L;
 inc(x);
 n:=(ptruint(x)-ptruint(@b^.x[0])) div sizeof(TULong);
 b^.wds:=n;
 n:=(32*n)-hi0bits(L);
 nbits:=53;
 lostbits:=0;
 x:=@b^.x[0];
 if n>nbits then begin
  dec(n,nbits);
  if any_on(b,n)<>0 then begin
   lostbits:=1;
	 k:=n-1;
   if (PULongs(x)^[k shr 5] and (1 shl (k and 31)))<>0 then begin
    lostbits:=2;
		if (k>0) and (any_on(b,k)<>0) then begin
     lostbits:=3;
    end;
   end;
  end;
  rshift(b,n);
  inc(e,n);
 end else if n<nbits then begin
  n:=nbits-n;
  b:=lshift(b,n);
  dec(e,n);
  x:=@b^.x[0];
 end;
 if e>1023 then begin
  ovfl:
  Bfree(b);
  ovfl1:
  rvp^.hi:=$7ff00000;
  rvp^.lo:=0;
  exit;
 end;
 denorm:=0;
 if e<emin then begin
  denorm:=1;
  n:=emin-e;
  if n>=nbits then begin
   case rounding of
    Round_near:begin
     if (n=nbits) and ((n<2) or (any_on(b,n-1)<>0)) then begin
      goto ret_tiny;
     end;
    end;
    Round_up:begin
     if sign=0 then begin
      goto ret_tiny;
     end;
    end;
    Round_down:begin
     if sign<>0 then begin
      goto ret_tiny;
     end;
     Bfree(b);
     retz:
     retz1:
     rvp^.d:=0.0;
     exit;
    end;
   end;
  end;
  k:=n-1;
  if lostbits<>0 then begin
   lostbits:=1;
  end else begin
   if k>0 then begin
    lostbits:=any_on(b,k);
   end;
  end;
  if (PULongs(x)^[k shr 5] and (1 shl ( k and 31 )))<>0 then begin
   lostbits:=lostbits or 2;
  end;
  dec(nbits,n);
  rshift(b,n);
  e:=emin;
 end;
 if lostbits<>0 then begin
  up:=0;
  case rounding of
   Round_zero:begin
   end;
   Round_near:begin
    if ((lostbits and 2)<>0) and (((lostbits and 1)<>0) or ((PULongs(x)^[0] and 1)<>0)) then begin
     up:=1;
    end;
   end;
   Round_up:begin
    up:=1-sign;
   end;
   Round_down:begin
    up:=sign;
   end;
  end;
  if up<>0 then begin
   k:=b^.wds;
   b:=increment(b);
   x:=@b^.x[0];
   n:=nbits and 31;
   if denorm<>0 then begin
   end else if ((b^.wds>k) or ((n<>0) and (hi0bits(PULongs(x)^[k-1])<(32-n)))) then begin
    rshift(b,1);
    inc(e);
    if e>1023 then begin
     goto ovfl;
    end;
   end;
  end;
 end;
 if denorm<>0 then begin
  if b^.wds>1 then begin
   rvp^.hi:=b^.x[1] and not $100000;
  end else begin
   rvp^.hi:=0;
  end;
 end else begin
  rvp^.hi:=(b^.x[1] and not $100000) or longword((e+$3ff+52) shl 20);
 end;
 rvp^.lo:=b^.x[0];
 Bfree(b);
end;

procedure getbase(sp:PPUChar;rvp:PU;rounding,sign,base:TInt);
label pcheck,ret_tiny,ret_big,retz1,retz,ovfl,ovfl1;
const emax=(($7fe-1023)-53)+1;
	   	emin=((-1022)-53)+1;
var b:PBigint;
    decpt,s0,s,s1:PUChar;
    e,e1:TLong;
    L,lostbits:TULong;
    x:PULong;
    big,denorm,esign,havedig,k,n,nbits,up,zret:TInt;
begin
 if basedig[ord('0')]=0 then begin
  basedig_init;
 end;

 denorm:=0;
 up:=0;
 havedig:=0;

 s0:=@sp^[0];
 while s0[havedig]='0' do begin
  inc(havedig);
 end;
 inc(s0,havedig);
 s:=s0;
 decpt:=nil;
 zret:=0;
 e:=0;
 if basedig[byte(s^)]<>0 then begin
  inc(havedig);
 end else begin
  zret:=1;
  if s^<>'.' then begin
   goto pcheck;
  end;
  inc(s);
  decpt:=s;

  if basedig[byte(s^)]=0 then begin
   goto pcheck;
  end;

	while s^='0' do begin
   inc(s);
  end;

  if basedig[byte(s^)]<>0 then begin
   zret:=0;
  end;

	havedig:=1;
  s0:=s;
 end;
 while basedig[byte(s^)]<>0 do begin
  inc(s);
 end;
 if (s^='.') and not assigned(decpt) then begin
  inc(s);
	decpt:=s;
  while basedig[byte(s^)]<>0 do begin
   inc(s);
  end;
 end;
 if assigned(decpt) then begin
  e:=-((s-decpt) shl 2);
 end;
 pcheck:
 s1:=s;
 big:=0;
 esign:=0;
 case s^ of
  'p','P':begin
   inc(s);
   case s^ of
    '-':begin
     esign:=1;
     inc(s);
    end;
    '+':begin
     inc(s);
    end;
   end;
   n:=basedig[byte(s^)];
   if (n=0) or (n>$19) then begin
    s:=s1;
   end else begin
    e1:=n-$10;
    inc(s);
    n:=basedig[byte(s^)];
    while (n<>0) and (n<=$19) do begin
     if (e1 and $f8000000)<>0 then begin
      big:=1;
     end;
     e1:=((10*e1)+n)-$10;
     inc(s);
     n:=basedig[byte(s^)];
    end;
    if esign<>0 then begin
     e1:=-e1;
    end;
    inc(e,e1);
   end;
	end;
 end;
 sp^:=s;
 if havedig=0 then begin
	sp^:=@s0[-1];
 end;
 if zret<>0 then begin
  goto retz1;
 end;
 if big<>0 then begin
  if esign<>0 then begin
   case rounding of
    Round_up:begin
     if sign=0 then begin
      goto ret_tiny;
     end;
    end;
    Round_down:begin
     if sign<>0 then begin
      goto ret_tiny;
     end;
    end;
   end;
   goto retz;
   ret_tiny:
   rvp.hi:=0;
   rvp.lo:=1;
   exit;
  end;
  case rounding of
   Round_near:begin
    goto ovfl1;
   end;
   Round_up:begin
    if sign=0 then begin
     goto ovfl1;
    end else begin
     goto ret_big;
    end;
   end;
   Round_down:begin
    if sign<>0 then begin
     goto ovfl1;
    end else begin
     goto ret_big;
    end;
   end;
  end;
  ret_big:
  rvp.hi:=($fffff or ($100000*((308+1023)-1)));
	rvp.lo:=$ffffffff;
	exit;
 end;
 n:=(s1-s0)-1;

 b:=Balloc(1);
 k:=b^.k;
 x:=@b^.x[0];
 b.wds:=1;
 b.x[0]:=0;

 s:=s0;
 while ptruint(s)<ptruint(s1) do begin
  if s^='.' then begin
   inc(s);
   continue;
  end;
  multadd(b,base,basedig[byte(s^)] and $7f);
  inc(s);
 end;
 n:=b^.wds;
 L:=b^.x[0];
 n:=(32*n)-hi0bits(L);
 nbits:=53;
 lostbits:=0;
 x:=@b^.x[0];
 if n>nbits then begin
  dec(n,nbits);
  if any_on(b,n)<>0 then begin
   lostbits:=1;
	 k:=n-1;
   if (PULongs(x)^[k shr 5] and (1 shl (k and 31)))<>0 then begin
    lostbits:=2;
		if (k>0) and (any_on(b,k)<>0) then begin
     lostbits:=3;
    end;
   end;
  end;
  rshift(b,n);
  inc(e,n);
 end else if n<nbits then begin
  n:=nbits-n;
  b:=lshift(b,n);
  dec(e,n);
  x:=@b^.x[0];
 end;
 if e>1023 then begin
  ovfl:
  Bfree(b);
  ovfl1:
  rvp^.hi:=$7ff00000;
  rvp^.lo:=0;
  exit;
 end;
 denorm:=0;
 if e<emin then begin
  denorm:=1;
  n:=emin-e;
  if n>=nbits then begin
   case rounding of
    Round_near:begin
     if (n=nbits) and ((n<2) or (any_on(b,n-1)<>0)) then begin
      goto ret_tiny;
     end;
    end;
    Round_up:begin
     if sign=0 then begin
      goto ret_tiny;
     end;
    end;
    Round_down:begin
     if sign<>0 then begin
      goto ret_tiny;
     end;
     Bfree(b);
     retz:
     retz1:
     rvp^.d:=0.0;
     exit;
    end;
   end;
  end;
  k:=n-1;
  if lostbits<>0 then begin
   lostbits:=1;
  end else begin
   if k>0 then begin
    lostbits:=any_on(b,k);
   end;
  end;
  if (PULongs(x)^[k shr 5] and (1 shl ( k and 31 )))<>0 then begin
   lostbits:=lostbits or 2;
  end;
  dec(nbits,n);
  rshift(b,n);
  e:=emin;
 end;
 if lostbits<>0 then begin
  up:=0;
  case rounding of
   Round_zero:begin
   end;
   Round_near:begin
    if ((lostbits and 2)<>0) and (((lostbits and 1)<>0) or ((PULongs(x)^[0] and 1)<>0)) then begin
     up:=1;
    end;
   end;
   Round_up:begin
    up:=1-sign;
   end;
   Round_down:begin
    up:=sign;
   end;
  end;
  if up<>0 then begin
   k:=b^.wds;
   b:=increment(b);
   x:=@b^.x[0];
   n:=nbits and 31;
   if denorm<>0 then begin
   end else if ((b^.wds>k) or ((n<>0) and (hi0bits(PULongs(x)^[k-1])<(32-n)))) then begin
    rshift(b,1);
    inc(e);
    if e>1023 then begin
     goto ovfl;
    end;
   end;
  end;
 end;
 if denorm<>0 then begin
  if b^.wds>1 then begin
   rvp^.hi:=b^.x[1] and not $100000;
  end else begin
   rvp^.hi:=0;
  end;
 end else begin
  rvp^.hi:=(b^.x[1] and not $100000) or longword((e+$3ff+52) shl 20);
 end;
 rvp^.lo:=b^.x[0];
 Bfree(b);
end;

function basetod(s:puchar;base:TInt):double;
label break2,ret0,ret;
var rv:TU;
    sign:integer;
    OldFPUExceptionMask:TFPUExceptionMask;
    OldFPUPrecisionMode:TFPUPrecisionMode;
    OldFPURoundingMode:TFPURoundingMode;
begin
 sign:=0;
 OldFPUExceptionMask:=GetExceptionMask;
 OldFPUPrecisionMode:=GetPrecisionMode;
 OldFPURoundingMode:=GetRoundMode;
 try
  if OldFPUExceptionMask<>DtoAFPUExceptionMask then begin
   SetExceptionMask(DtoAFPUExceptionMask);
  end;
  if OldFPUPrecisionMode<>DtoAFPUPrecisionMode then begin
   SetPrecisionMode(DtoAFPUPrecisionMode);
  end;
  if OldFPURoundingMode<>DtoAFPURoundingMode then begin
   SetRoundMode(DtoAFPURoundingMode);
  end;
  if (not assigned(s)) or (s^=#0) then begin
   goto ret0;
  end;
  while true do begin
   case s^ of
    '-':begin
     sign:=1;
     if s^<>#0 then begin
      goto break2;
     end;
     goto ret0;
    end;
    '+':begin
     inc(s);
     if s^<>#0 then begin
      goto break2;
     end;
     goto ret0;
    end;
    #0:begin
     goto ret0;
    end;
    #9,#10,#11,#12,#13,#32:begin
    end;
    else begin
     goto break2;
    end;
   end;
   inc(s);
  end;
  break2:
  if s^=#0 then begin
   goto ret0;
  end;
  getbase(@s,@rv,1,sign,base);
  goto ret;
  ret0:
  rv.d:=0;
  ret:
  if sign<>0 then begin
   result:=-rv.d;
  end else begin
   result:=rv.d;
  end;
 finally
  if OldFPUExceptionMask<>DtoAFPUExceptionMask then begin
   SetExceptionMask(OldFPUExceptionMask);
  end;
  if OldFPUPrecisionMode<>DtoAFPUPrecisionMode then begin
   SetPrecisionMode(OldFPUPrecisionMode);
  end;
  if OldFPURoundingMode<>DtoAFPURoundingMode then begin
   SetRoundMode(OldFPURoundingMode);
  end;
 end;
end;

function dshift(b:PBigint;p2:TInt):TInt;
var rv:TInt;
begin
 rv:=hi0bits(b^.x[b^.wds-1])-4;
 if p2>0 then begin
  dec(rv,p2);
 end;
 result:=rv and 31;
end;

function quorem(b,S:PBigint):TInt;
var n:TInt;
  	bx,bxe,sx,sxe:PULong;
    q,borrow,carry,y,ys,si,z,zs:TULong;
begin
 n:=S^.wds;
 if b^.wds<n then begin
  result:=0;
  exit;
 end;
 sx:=@S^.x[0];
 dec(n);
 sxe:=@S^.x[n];
 bx:=@b^.x[0];
 bxe:=@b^.x[n];
 q:=bxe^ div (sxe^+1);

 if q<>0 then begin
	borrow:=0;
	carry:=0;
	repeat
   si:=sx^;
   inc(sx);
   ys:=((si and $ffff)*q)+carry;
   zs:=((si shr 16)*q)+(ys shr 16);
   carry:=zs shr 16;
   y:=((bx^ and $ffff)-(ys and $ffff))-borrow;
   borrow:=(y and $10000) shr 16;
   z:=((bx^ shr 16)-(zs and $ffff))-borrow;
   borrow:=(z and $10000 ) shr 16;
   bx^:=(z shl 16) or (y and $ffff);
   inc(bx);
  until ptruint(sx)>ptruint(sxe);
  if bxe^=0 then begin
 	 bx:=@b^.x[0];
   dec(bxe);
   while (ptruint(bxe)>ptruint(bx)) and (bxe^=0) do begin
    dec(bxe);
    dec(n);
   end;
 	 b^.wds:=n;
  end;
 end;
 if cmp(b,S)>=0 then begin
  inc(q);
	borrow:=0;
	carry:=0;
	bx:=@b^.x[0];
	sx:=@S^.x[0];
  repeat
   si:=sx^;
   inc(sx);
   ys:=(si and $ffff)+carry;
   zs:=(si shr 16)+(ys shr 16);
   carry:=zs shr 16;
   y:=((bx^ and $ffff)-(ys and $ffff))-borrow;
   borrow:=(y and $10000) shr 16;
   z:=((bx^ shr 16)-(zs and $ffff))-borrow;
   borrow:=(z and $10000 ) shr 16;
   bx^:=(z shl 16) or (y and $ffff);
   inc(bx);
  until ptruint(sx)>ptruint(sxe);
  bx:=@b^.x[0];
  bxe:=@b^.x[n];
  if bxe^=0 then begin
   dec(bxe);
   while (ptruint(bxe)>ptruint(bx)) and (bxe^=0) do begin
    dec(bxe);
    dec(n);
   end;
   b^.wds:=n;
  end;
 end;
 result:=q;
end;

function sulp(x:PU;bc:PBCinfo):double;
var u:TU;
    rv:double;
    i:integer;
begin
 rv:=ulp(x);
 i:=((2*53)+1)-((x^.hi and $7ff00000) shr 20);
 if (bc^.scale=0) or (i<=0) then begin
  result:=rv;
  exit;
 end;
 u.hi:=$3ff00000+(i shl 20);
 u.lo:=0;
 result:=rv*u.d;
end;

procedure bigcomp(rv:PU;s0:PUChar;bc:PBCinfo);
label have_i,ret,retlow1,rethi1,odd;
var b,d:PBigint;
    b2,bbits,d2,dd,dig,dsign,i,j,nd,nd0,p2,p5,speccase:TInt;
begin
 dd:=0;
 dsign:=bc^.dsign;
 nd:=bc^.nd;
 nd0:=bc^.nd0;
 p5:=(nd+bc^.e0)-1;
 speccase:=0;
 if rv^.d=0.0 then begin
	b:=i2b(1);
	p2:=((-1022)-53)+1;
	bbits:=1;
  rv^.hi:=(53+2) shl 20;
  i:=0;
	begin
   speccase:=1;
	 dec(p2);
   dsign:=0;
   goto have_i;
  end;
 end else begin
  b:=d2b(rv,@p2,@bbits);
 end;

 dec(p2,bc^.scale);

 i:=53-bbits;
 j:=((53-(-1023))-1)+p2;
 if i>j then begin
	i:=j;
 end;

 begin
  inc(i);
  b:=lshift(b,i);
  b^.x[0]:=b^.x[0] or 1;
 end;

have_i:
 dec(p2,p5+i);
 d:=i2b(1);

 if p5>0 then begin
  d:=pow5mult(d,p5);
 end else begin
  if p5<0 then begin
   b:=pow5mult(b,-p5);
  end;
 end;
 if p2>0 then begin
  b2:=p2;
	d2:=0;
 end else begin
  b2:=0;
	d2:=-p2;
 end;
 i:=dshift(d,d2);
 inc(b2,i);
 if b2>0 then begin
  b:=lshift(b,b2);
 end;
 inc(d2,i);
 if d2>0 then begin
  d:=lshift(d,d2);
 end;

 dig:=quorem(b,d);
 if dig=0 then begin
  b:=multadd(b,10,0);
  dig:=quorem(b,d);
 end;

 i:=0;
 while i<nd0 do begin
  dd:=(ord(s0[i])-ord('0'))-dig;
  inc(i);
  if dd<>0 then begin
 	 goto ret;
  end;
  if (b^.wds=1) and (b^.x[0]=0) then begin
   if i<nd then begin
    dd:=1;
   end;
   goto ret;
  end;
  b:=multadd(b,10,0);
  dig:=quorem(b,d);
 end;
 j:=bc^.dp1;
 while i<nd do begin
  inc(i);
  dd:=(ord(s0[j])-ord('0'))-dig;
  inc(j);
  if dd<>0 then begin
 	 goto ret;
  end;
  if (b^.wds=1) and (b^.x[0]=0) then begin
   if i<nd then begin
    dd:=1;
   end;
   goto ret;
  end;
	b:=multadd(b,10,0);
	dig:=quorem(b,d);
 end;
 if (b^.x[0]<>0) or (b^.wds>1) then begin
	dd:=-1;
 end;
ret:
 Bfree(b);
 Bfree(d);
 if speccase<>0 then begin
  if dd<=0 then begin
   rv^.d:=0.0;
  end;
 end else begin
  if dd<0 then begin
   if dsign=0 then begin
retlow1:
    rv^.d:=rv^.d-sulp(rv,bc);
   end;
  end else begin
   if dd>0 then begin
    if dsign<>0 then begin
rethi1:
     rv^.d:=rv^.d+sulp(rv,bc);
    end;
   end else begin
    j:=((rv^.hi and $7ff00000) shr 20)-TULong(bc^.scale);
    if j<=0 then begin
     i:=1-j;
     if i<=31 then begin
      if (rv^.lo and (1 shl i))<>0 then begin
       goto odd;
      end;
     end else begin
      if (rv^.hi and (1 shl (i-32)))<>0 then begin
       goto odd;
      end;
     end;
    end else begin
     if (rv.lo and 1)<>0 then begin
odd:
      if dsign<>0 then begin
       goto rethi1;
      end;
      goto retlow1;
     end;
    end;
   end;
  end;
 end;
end;

function matchex(a,b:PUChar):boolean;
begin
 if assigned(a) and assigned(b) then begin
  while a^=b^ do begin
   inc(a);
   inc(b);
  end;
  result:=a^<>b^;
 end else begin
  result:=false;
 end;
end;

function strtod(s00:PUChar;se:PPUChar;octal:boolean=false):double;
label break2,ret0,ret,have_dig,dig_done,ovfl,range_err,undfl,drop_down,cont;
var bb2,bb5,bbe,bd2,bd5,bbbits,bs2,c,e,e1,esign,i,j,k,nd,nd0,nf,nz,nz0,nz1,sign,req_bigcomp:TInt;
    s,s0,s1:PUChar;
    aadj,aadj1:double;
    l:TLong;
	  aadj2,adj,rv,rv0:TU;
  	y,z,yy:TULong;
	  bc:TBCInfo;
	  bb,bb1,bd,bd0,bs,delta:PBigint;
    Lsb,Lsb1:TULong;
    OldFPUExceptionMask:TFPUExceptionMask;
    OldFPUPrecisionMode:TFPUPrecisionMode;
    OldFPURoundingMode:TFPURoundingMode;
begin
 OldFPUExceptionMask:=GetExceptionMask;
 OldFPUPrecisionMode:=GetPrecisionMode;
 OldFPURoundingMode:=GetRoundMode;
 try
  if OldFPUExceptionMask<>DtoAFPUExceptionMask then begin
   SetExceptionMask(DtoAFPUExceptionMask);
  end;
  if OldFPUPrecisionMode<>DtoAFPUPrecisionMode then begin
   SetPrecisionMode(DtoAFPUPrecisionMode);
  end;
  if OldFPURoundingMode<>DtoAFPURoundingMode then begin
   SetRoundMode(DtoAFPURoundingMode);
  end;

  req_bigcomp:=0;

  bb:=nil;
  bd:=nil;
  bs:=nil;
  delta:=nil;
  aadj1:=0;

  sign:=0;
  nz0:=0;
  nz1:=0;
  nz:=0;
  bc.dplen:=0;
  bc.uflchk:=0;
  rv.d:=0.0;
  s:=s00;
  while true do begin
   case s^ of
    '-':begin
     inc(s);
     sign:=1;
     if s^<>#0 then begin
      goto break2;
     end;
     goto ret0;
    end;
    '+':begin
     inc(s);
     if s^<>#0 then begin
      goto break2;
     end;
     goto ret0;
    end;
    #0:begin
     goto ret0;
    end;
    #9,#10,#11,#12,#13,#32:begin
    end;
    else begin
     goto break2;
    end;
   end;
   inc(s);
  end;
break2:
  if s^='0' then begin
   case s[1] of
    'x','X':begin
     gethex(@s,@rv,1,sign);
     goto ret;
    end;
    else begin
     if octal then begin
      getbase(@s,@rv,1,sign,8);
      goto ret;
     end;
    end;
   end;

   nz0:=1;
   inc(s);
   while s^='0' do inc(s);
   if s^=#0 then begin
    goto ret;
   end;
  end;
  s0:=s;
  y:=0;
  z:=0;
  nd:=0;
  nf:=0;
  c:=ord(s^);
  while (c>=ord('0')) and (c<=ord('9')) do begin
   if nd<9 then begin
    y:=(10*y)+TULong(c-ord('0'));
   end else if nd<16 then begin
    z:=(10*z)+TULong(c-ord('0'));
   end;
   inc(nd);
   inc(s);
   c:=ord(s^);
  end;
  nd0:=nd;
  bc.dp0:=s-s0;
  bc.dp1:=bc.dp0;
  s1:=s;
  if ptruint(s1)>ptruint(s0) then begin
   dec(s1);
   if s1^='0' then begin
    while true do begin
     inc(nz1);
     if ptruint(s1)>ptruint(s0) then begin
      dec(s1);
      if s1^<>'0' then begin
       break;
      end;
     end else begin
      break;
     end;
    end;
   end;
  end;
  if c=ord('.') then begin
   inc(s);
   c:=ord(s^);
   bc.dp1:=s-s0;
   bc.dplen:=bc.dp1-bc.dp0;
   if nd=0 then begin
    while c=ord('0') do begin
     inc(nz);
     inc(s);
     c:=ord(s^);
    end;
    if (c>=ord('1')) and (c<=ord('9')) then begin
     s0:=s;
     inc(nf,nz);
     nz:=0;
     goto have_dig;
    end;
    goto dig_done;
   end;
   while (c>=ord('0')) and (c<=ord('9')) do begin
have_dig:
    inc(nz);
    dec(c,ord('0'));
    if c<>0 then begin
     inc(nf,nz);
     for i:=1 to nz-1 do begin
      if nd<9 then begin
       inc(nd);
       y:=y*10;
      end else begin
       inc(nd);
       if nd<=(15+1) then begin
        z:=z*10;
       end;
      end;
     end;
     if nd<9 then begin
      inc(nd);
      y:=(y*10)+TULong(c);
     end else begin
      inc(nd);
      if nd<=(15+1) then begin
       z:=(z*10)+TULong(c);
      end;
     end;
     nz:=0;
     nz1:=0;
    end;
    inc(s);
    c:=ord(s^);
   end;
  end;
dig_done:
  e:=0;
  if (c=ord('e')) or (c=ord('E')) then begin
   if (nd=0) and (nz=0) and (nz0=0) then begin
    goto ret0;
   end;
   s00:=s;
   esign:=0;
   inc(s);
   c:=ord(s^);
   case c of
    ord('-'):begin
     esign:=1;
     inc(s);
     c:=ord(s^);
    end;
    ord('+'):begin
     inc(s);
     c:=ord(s^);
    end;
   end;
   if (c>=ord('0')) and (c<=ord('9')) then begin
    while c=ord('0') do begin
     inc(s);
     c:=ord(s^);
    end;
    if (c>=ord('1')) and (c<=ord('9')) then begin
     L:=c-ord('0');
     s1:=s;
     inc(s);
     c:=ord(s^);
     while (c>=ord('0')) and (c<=ord('9')) do begin
      L:=(L*10)+(c-ord('0'));
      inc(s);
      c:=ord(s^);
     end;
     if ((s-s1)>8) or (L>19999) then begin
      e:=19999;
     end	else begin
      e:=L;
     end;
     if esign<>0 then begin
      e:=-e;
     end;
    end else begin
     e:=0;
    end;
   end	else begin
    s:=s00;
   end;
  end;

  if nd=0 then begin
   if (nz=0) and (nz0=0) then begin
    if bc.dplen=0 then begin
     case c of
      ord('i'),ord('I'):begin
       if match(@s,'nf')<>0 then begin
        dec(s);
        if match(@s,'inity')=0 then begin
         inc(s);
        end;
        rv.hi:=$7ff00000;
        rv.lo:=0;
        goto ret;
       end;
      end;
      ord('n'),ord('N'):begin
       if match(@s,'an')<>0 then begin
        rv.hi:=$7ff80000;
        rv.lo:=0;
        if s^='(' then begin
         hexnan(@rv,@s);
        end;
        goto ret;
       end;
      end;
     end;
    end;
ret0:
    s:=s00;
    sign:=0;
   end;
   goto ret;
  end;

  dec(e,nf);
  bc.e0:=e;
  e1:=e;

  if nd0=0 then begin
   nd0:=nd;
  end;
  if nd<(15+1) then begin
   k:=nd;
  end else begin
   k:=15+1;
  end;
  rv.d:=y;
  if k>9 then begin
   rv.d:=(Tens[k-9]*rv.d)+z;
  end;
  bd0:=nil;
  if nd<=15 then begin
   if e=0 then begin
    goto ret;
   end;
   if e>0 then begin
    if e<=22 then begin
     rv.d:=rv.d*tens[e];
     goto ret;
    end;
    i:=15-nd;
    if e<=(22+i) then begin
     dec(e,i);
     rv.d:=(rv.d*tens[i])*tens[e];
     goto ret;
    end;
   end else begin
    if e>=-22 then begin
     rv.d:=rv.d/tens[-e];
     goto ret;
    end;
   end;
  end;
  inc(e1,nd-k);

  bc.scale:=0;
  if e1>0 then begin
   i:=e1 and 15;
   if i<>0 then begin
    rv.d:=rv.d*tens[i];
   end;
   e1:=e1 and not 15;
   if e1<>0 then begin
    if e1>308 then begin
ovfl:
     rv.hi:=$7ff00000;
     rv.lo:=0;
range_err:
     if assigned(bd0) then begin
      Bfree(bb);
      Bfree(bd);
      Bfree(bs);
      Bfree(bd0);
      Bfree(delta);
     end;
     goto ret;
    end;
    e1:=e1 shr 4;
    j:=0;
    while e1>1 do begin
     if (e1 and 1)<>0 then begin
      rv.d:=rv.d*bigtens[j];
     end;
     inc(j);
     e1:=e1 shr 1;
    end;
    dec(rv.hi,53*$100000);
    rv.d:=rv.d*bigtens[j];
    z:=rv.hi and $7ff00000;
    if z>$100000*((1024+1023)-53) then begin
     goto ovfl;
    end;
    if z>$100000*(((1024+1023)-1)-53) then begin
     rv.hi:=$000fffff or ($100000*((1024+1023)-1));
     rv.lo:=$ffffffff;
    end else begin
     inc(rv.hi,53*$100000);
    end;
   end;
  end else begin
   if e1<0 then begin
    e1:=-e1;
    i:=e1 and 15;
    if i<>0 then begin
     rv.d:=rv.d/tens[i];
    end;
    e1:=e1 shr 4;
    if e1<>0 then begin
     if e1>=(1 shl 5) then begin
      goto undfl;
     end;
     if (e1 and $10)<>0 then begin
      bc.scale:=2*53;
     end;
     j:=0;
     while e1>0 do begin
      if (e1 and 1)<>0 then begin
       rv.d:=rv.d*tinytens[j];
      end;
      inc(j);
      e1:=e1 shr 1;
     end;
     j:=((2*53)+1)-((rv.hi and $7ff00000) shr 20);
     if (bc.scale<>0) and (j>0) then begin
      if j>=32 then begin
       if j>54 then begin
        goto undfl;
       end;
       rv.lo:=0;
       if j>=53 then begin
        rv.hi:=(53+2)*$100000;
       end else begin
        rv.hi:=rv.hi and ($ffffffff shl (j-32));
       end;
      end else begin
       rv.lo:=rv.lo and ($ffffffff shl j);
      end;
     end;
     if rv.d=0 then begin
undfl:
      rv.d:=0.0;
      goto range_err;
     end;
    end;
   end;
  end;

  bc.nd:=nd-nz1;
  bc.nd0:=nd0;

  if nd>40 then begin
   i:=18;
   j:=18;
   if i>nd0 then begin
    inc(j,bc.dplen);
   end;
   while true do begin
    dec(j);
    if (j<bc.dp1) and (j>=bc.dp0) then begin
     j:=bc.dp0-1;
    end;
    if s0[j]<>'0' then begin
     break;
    end;
    dec(i);
   end;
   inc(e,nd-i);
   nd:=i;
   if nd0>nd then begin
    nd0:=nd;
   end;
   if nd<9 then begin
    y:=0;
    i:=0;
    while i<nd0 do begin
     y:=(y*10)+TULong(ord(s0[i])-ord('0'));
     inc(i);
    end;
    j:=bc.dp1;
    while i<nd do begin
     y:=(y*10)+TULong(ord(s0[j])-ord('0'));
     inc(j);
     inc(i);
    end;
   end;
  end;

  bd0:=s2b(s0,nd0,nd,y,bc.dplen);

  while true do begin
   bd:=Balloc(bd0^.k);
   BBcopy(bd,bd0);
   bb:=d2b(@rv,@bbe,@bbbits);
   bs:=i2b(1);

   if e>=0 then begin
    bb2:=0;
    bb5:=0;
    bd2:=e;
    bd5:=e;
   end else begin
    bb2:=-e;
    bb5:=-e;
    bd2:=0;
    bd5:=0;
   end;
   if bbe>=0 then begin
    inc(bb2,bbe);
   end else begin
    dec(bd2,bbe);
   end;
   bs2:=bb2;

   Lsb:=1;
   Lsb1:=0;
   j:=bbe-bc.scale;
   i:=(j+bbbits)-1;
   j:=(53+1)-bbbits;
   if i<(-1022) then begin
    i:=(-1022)-i;
    dec(j,i);
    if i<32 then begin
     Lsb:=Lsb shl i;
    end else begin
     if i<52 then begin
      Lsb1:=Lsb shl (i-32);
     end else begin
      Lsb1:=$7ff00000;
     end;
    end;
   end;

   inc(bb2,j);
   inc(bd2,j);

   inc(bd2,bc.scale);

   if bb2<bd2 then begin
    i:=bb2;
   end else begin
    i:=bd2;
   end;
   if i>bs2 then begin
    i:=bs2;
   end;
   if i>0 then begin
    dec(bb2,i);
    dec(bd2,i);
    dec(bs2,i);
   end;
   if bb5>0 then begin
    bs:=pow5mult(bs,bb5);
    bb1:=mult(bs,bb);
    Bfree(bb);
    bb:=bb1;
   end;
   if bb2>0 then begin
    bb:=lshift(bb,bb2);
   end;
   if bd5>0 then begin
    bd:=pow5mult(bd,bd5);
   end;
   if bd2>0 then begin
    bd:=lshift(bd,bd2);
   end;
   if bs2>0 then begin
    bs:=lshift(bs,bs2);
   end;
   delta:=diff(bb,bd);
   bc.dsign:=delta^.sign;
   delta^.sign:=0;
   i:=cmp(delta,bs);
   if (bc.nd>nd) and (i<=0) then begin
    if bc.dsign<>0 then begin
     req_bigcomp:=1;
     break;
    end;
    i:=-1;
   end;
   if i<0 then begin
    if (bc.dsign<>0) or (rv.lo<>0) or ((rv.hi and $fffff)<>0) or ((rv.hi and $7ff00000)<=((2*53)+1)*$100000) then begin
     break;
    end;
    if (delta.x[0]=0) and (delta.wds<=1) then begin
     break;
    end;
    delta:=lshift(delta,1);
    if cmp(delta,bs)>0 then begin
     goto drop_down;
    end;
    break;
   end;
   if i=0 then begin
    if bc.dsign<>0 then begin
     y:=rv.hi and $7ff00000;
     if (bc.scale<>0) and (y<=(2*53)*$100000) then begin
      yy:=$ffffffff and ($ffffffff shl (((2*53)+1)-(y shr 20)));
     end else begin
      yy:=$ffffffff;
     end;
     if ((rv.hi and $fffff)=$fffff) and (rv.hi=yy) then begin
      if (rv.hi=($fffff or ($100000*((1024+1023)-1)))) and (rv.lo=$ffffffff) then begin
       goto ovfl;
      end;
      rv.hi:=(rv.hi and $7ff00000)+$100000;
      rv.lo:=0;
      bc.dsign:=0;
      break;
     end;
    end else begin
     if ((rv.hi and $fffff)=0) and (rv.lo=0) then begin
drop_down:
      if bc.scale<>0 then begin
       L:=rv.Hi and $7ff00000;
       if L<=(((2*53)+1)*$100000) then begin
        if L>((53+2)*$100000) then begin
         break;
        end;
        if bc.nd>nd then begin
         bc.uflchk:=1;
         break;
        end;
        goto undfl;
       end;
      end;
      L:=(rv.hi and $7ff00000)-$100000;
      rv.hi:=L or $fffff;
      rv.lo:=$ffffffff;
      if bc.nd>nd then begin
       goto cont;
      end;
      break;
     end;
    end;
{$ifdef ROUND_BIASED}
    if bc.dsign<>0 then begin
     rv.d:=rv.d+sulp(@rv,@bc);
    end;
{$else}
    if Lsb1<>0 then begin
     if (rv.hi and Lsb1)=0 then begin
      break;
     end;
    end else begin
     if (rv.lo and Lsb)=0 then begin
      break;
     end;
    end;
    if bc.dsign<>0 then begin
     rv.d:=rv.d+sulp(@rv,@bc);
    end else begin
     rv.d:=rv.d-sulp(@rv,@bc);
     if rv.d=0 then begin
      if bc.nd>nd then begin
       bc.uflchk:=1;
       break;
      end;
      goto undfl;
     end;
    end;
    bc.dsign:=1-bc.dsign;
{$endif}
    break;
   end;
   aadj:=ratio(delta,bs);
   if aadj<=2.0 then begin
    if bc.dsign<>0 then begin
     aadj:=1.0;
     aadj1:=1.0;
    end else begin
     if (rv.lo<>0) or ((rv.hi and $fffff)<>0) then begin
      if (rv.lo=1) and (rv.hi=0) then begin
       if bc.nd>nd then begin
        bc.uflchk:=1;
        break;
       end;
       goto undfl;
      end;
      aadj:=1.0;
      aadj1:=-1.0;
     end else begin
      if aadj<(2.0/2) then begin
       aadj:=1.0/2;
      end else begin
       aadj:=aadj*0.5;
      end;
      aadj1:=-aadj;
     end;
    end;
   end else begin
    aadj:=aadj*0.5;
    if bc.dsign<>0 then begin
     aadj1:=aadj;
    end else begin
     aadj1:=-aadj;
    end;
   end;
   y:=rv.hi and $7ff00000;
   if y=($100000*((1024+1023)-1)) then begin
    rv0.d:=rv.d;
    dec(rv.hi,53*$100000);
    adj.d:=aadj1*ulp(@rv);
    rv.d:=rv.d+adj.d;
    if (rv.hi and $7ff00000)>=($100000*((1024+1023)-53)) then begin
     if (rv0.hi=($fffff or ($100000*((1024+1023)-1)))) and (rv0.lo=$ffffffff) then begin
      goto ovfl;
     end;
     rv.hi:=$fffff or ($100000*((1024+1023)-1));
     rv.lo:=$ffffffff;
     goto cont;
    end else begin
     inc(rv.hi,53*$100000);
    end;
   end else begin
    if (bc.scale<>0) and (y<=((2*53)*$100000)) then begin
     if aadj<=$7fffffff then begin
      z:=trunc(aadj);
      if z<=0 then begin
       z:=1;
      end;
      aadj:=z;
      if bc.dsign<>0 then begin
       aadj1:=aadj;
      end else begin
       aadj1:=-aadj;
      end;
     end;
     aadj2.d:=aadj1;
     inc(aadj2.hi,(((2*53)+1)*$100000)-y);
     aadj1:=aadj2.d;
     adj.d:=aadj1*ulp(@rv);
     rv.d:=rv.d+adj.d;
     if rv.d=0 then begin
      if bc.nd>nd then begin
       bc.dsign:=1;
      end;
      break;
     end;
    end else begin
     adj.d:=aadj1*ulp(@rv);
     rv.d:=rv.d+adj.d;
    end;
   end;
   z:=rv.hi and $7ff00000;
   if bc.nd=nd then begin
    if bc.scale=0 then begin
     if y=z then begin
      L:=trunc(aadj);
      aadj:=aadj-L;
      if (bc.dsign<>0) or (rv.lo<>0) or ((rv.hi and $fffff)<>0) then begin
       if (aadj<0.4999999) or (aadj>0.5000001) then begin
        break;
       end;
      end else begin
       if aadj<(0.4999999/2) then begin
        break;
       end;
      end;
     end;
    end;
   end;

cont:
   Bfree(bb);
   Bfree(bd);
   Bfree(bs);
   Bfree(delta);
  end;
  Bfree(bb);
  Bfree(bd);
  Bfree(bs);
  Bfree(bd0);
  Bfree(delta);

  if req_bigcomp<>0 then begin
   bd0:=nil;
   inc(bc.e0,nz1);
   bigcomp(@rv,s0,@bc);
   y:=rv.hi and $7ff00000;
   if y=$7ff00000 then begin
    goto ovfl;
   end;
   if (y=0) and (rv.d=0.0) then begin
    goto undfl;
   end;
  end;

  if bc.scale<>0 then begin
   rv.hi:=$3ff00000-((2*53)*$100000);
   rv.lo:=0;
   rv.d:=rv.d*rv0.d;
  end;

ret:
  if assigned(se) then begin
   se^:=s;
  end;
  if sign<>0 then begin
   result:=-rv.d;
  end else begin
   result:=rv.d;
  end;
 finally
  if OldFPUExceptionMask<>DtoAFPUExceptionMask then begin
   SetExceptionMask(OldFPUExceptionMask);
  end;
  if OldFPUPrecisionMode<>DtoAFPUPrecisionMode then begin
   SetPrecisionMode(OldFPUPrecisionMode);
  end;
  if OldFPURoundingMode<>DtoAFPURoundingMode then begin
   SetRoundMode(OldFPURoundingMode);
  end;
 end;
end;

function rv_alloc(i:TInt):PUChar;
begin
 GetMem(result,i);
 FillChar(result^,i,#0);
end;

function nrv_alloc(s:PUChar;rve:PPUChar;n:TInt):PUChar;
var t:PUChar;
begin
 result:=rv_alloc(n+1);
 t:=result;
 while s^<>#0 do begin
  t^:=s^;
  inc(s);
  inc(t);
 end;
 t^:=#0;
 if assigned(rve) then begin
	rve^:=t;
 end;
end;

procedure freedtoa(s:PUChar);
begin
 FreeMem(s);
end;

function dtoa(dd:double;mode,ndigits:TInt;decpt,sign:PInt;rve:PPUChar=nil;BiasUp:boolean=false):PUChar;
label retf,fast_failed,one_digit,no_digits,ret1,bump_up,ret,round_9_up,accept_dig,roundoff;
var bbits,b2,b5,be,dig,i,ieps,ilim,ilim0,ilim1,j,j1,k,k0,k_check,leftright,m2,m5,s2,s5,spec_case,try_quick,denorm:TInt;
    L:TLong;
    x:TULong;
    b,b1,delta,mlo,mhi,bS:PBigint;
    d2,eps,u:TU;
	  ds:double;
    s,s0:PUChar;
    OldFPUExceptionMask:TFPUExceptionMask;
    OldFPUPrecisionMode:TFPUPrecisionMode;
    OldFPURoundingMode:TFPURoundingMode;
begin
 OldFPUExceptionMask:=GetExceptionMask;
 OldFPUPrecisionMode:=GetPrecisionMode;
 OldFPURoundingMode:=GetRoundMode;
 try
  if OldFPUExceptionMask<>DtoAFPUExceptionMask then begin
   SetExceptionMask(DtoAFPUExceptionMask);
  end;
  if OldFPUPrecisionMode<>DtoAFPUPrecisionMode then begin
   SetPrecisionMode(DtoAFPUPrecisionMode);
  end;
  if OldFPURoundingMode<>DtoAFPURoundingMode then begin
   SetRoundMode(DtoAFPURoundingMode);
  end;

  dig:=0;
  mlo:=nil;

  u.d:=dd;
  if (u.hi and $80000000)<>0 then begin
   sign^:=1;
   u.hi:=u.hi and $7fffffff;
  end else begin
   sign^:=0;
  end;

  if (u.hi and $7ff00000)=$7ff00000 then begin
   decpt^:=9999;
   if ((u.hi and $000fffff) or u.lo)=0 then begin
    result:=nrv_alloc('Infinity',rve,8);
   end else begin
    result:=nrv_alloc('NaN',rve,3);
   end;
   goto retf;
  end;

	if u.d=0 then begin
	 decpt^:=1;
   result:=nrv_alloc('0',rve,1);
   goto retf;
  end;

	b:=d2b(@u,@be,@bbits);

  i:=(u.hi shr 20) and $7ff;
	if i<>0 then begin
   d2:=u;
   d2.hi:=(d2.hi and $fffff) or $3ff00000;
	 dec(i,1023);
 	 denorm:=0;
  end else begin
   i:=(bbits+be)+((1023+(53-1))-1);
   if i>32 then begin
    x:=(u.hi shl (64-i)) or (u.lo shr (i-32));
   end else begin
    x:=u.lo shl (32-i);
   end;
   d2.d:=x;
   dec(d2.hi,$1f00000);
   dec(i,((1023+(53-1))-1)+1);
   denorm:=1;
  end;

	ds:=(((d2.d-1.5)*0.289529654602168)+0.1760912590558)+(i*0.301029995663981);
	k:=trunc(ds);
	if (ds<0.0) and (ds<>k) then begin
   dec(k);
  end;
	k_check:=1;
	if (k>=0) and (k<=22) then begin
   if u.d<tens[k] then begin
    dec(k);
   end;
   k_check:=0;
  end;
	j:=(bbits-i)-1;
  if j>=0 then begin
   b2:=0;
   s2:=j;
  end else begin
   b2:=-j;
   s2:=0;
  end;
  if k>=0 then begin
   b5:=0;
   s5:=k;
   inc(s2,k);
  end else begin
   dec(b2,k);
   b5:=-k;
   s5:=0;
  end;

  if (Mode<0) or (Mode>9) then begin
   Mode:=0;
  end;

	try_quick:=1;

  if mode>5 then begin
   dec(mode,4);
   try_quick:=0;
  end;
  if mode in [2,3] then begin
   leftright:=0;
  end else begin
   leftright:=1;
  end;

	ilim:=-1;
  ilim1:=-1;

  case mode of
   0,1:begin
    i:=18;
    ndigits:=0;
   end;
   2,4:begin
    if ndigits<=0 then begin
     ndigits:=1;
    end;
    i:=nDigits;
    ilim:=i;
    ilim1:=i;
   end;
   3,5:begin
    i:=ndigits+k+1;
    ilim:=i;
    ilim1:=i-1;
    if i<=0 then begin
     i:=1;
    end;
   end;
  end;

  s0:=rv_alloc(i);
	s:=s0;

	if ((ilim>=0) and (ilim<=14)) and (try_quick<>0) then begin
 	 i:=0;
   d2:=u;
   k0:=k;
	 ilim0:=ilim;
	 ieps:=2;
   if k>0 then begin
    ds:=tens[k and $f];
    j:=k shr 4;
    if (j and $10)<>0 then begin
     j:=j and $f;
     u.d:=u.d/bigtens[high(BigTens)];
     inc(ieps);
    end;
    while j<>0 do begin
     if (j and 1)<>0 then begin
      inc(ieps);
      ds:=ds*bigtens[i];
     end;
     j:=j shr 1;
     inc(i);
    end;
    u.d:=u.d/ds;
   end else begin
    j1:=-k;
    if j1<>0 then begin
     u.d:=u.d*tens[j1 and $f];
     j:=j1 shr 4;
     while j<>0 do begin
      if (j and 1)<>0 then begin
       inc(ieps);
       u.d:=u.d*bigtens[i];
      end;
      j:=j shr 1;
      inc(i);
     end;
    end;
   end;
   if (k_check<>0) and ((u.d<1.0) and (ilim>0)) then begin
    if ilim1<=0 then begin
     goto fast_failed;
    end else begin
     ilim:=ilim1;
     dec(k);
     u.d:=u.d*10.0;
     inc(ieps);
    end;
   end;
   eps.d:=(ieps*u.d)+7.0;
   dec(eps.hi,(53-1)*$100000);
   if ilim=0 then begin
    bS:=nil;
    mhi:=nil;
	  u.d:=u.d-5.0;
    if u.d>eps.d then begin
		 goto one_digit;
		end;
    if u.d<(-eps.d) then begin
     goto no_digits;
    end;
		goto fast_failed;
   end;
   if leftright<>0 then begin
    eps.d:=(0.5/tens[ilim-1])-eps.d;
    i:=0;
    while true do begin
		 L:=trunc(u.d);
     u.d:=u.d-L;
     byte(s^):=ord('0')+L;
     inc(s);
     if u.d<eps.d then begin
      goto ret1;
     end;
     if (1.0-u.d)<eps.d then begin
		 	goto bump_up;
     end;
     inc(i);
     if i>=ilim then begin
		  break;
     end;
     eps.d:=eps.d*10.0;
     u.d:=u.d*10.0;
    end;
   end else begin
    eps.d:=eps.d*tens[ilim-1];
    i:=1;
    while true do begin
     L:=trunc(u.d);
     u.d:=u.d-L;
     if u.d=0 then begin
			ilim:=i;
     end;
     byte(s^):=ord('0')+L;
     inc(s);
		 if i=ilim then begin
     	if u.d>(0.5+eps.d) then begin
       goto bump_up;
			end else begin
       if u.d<(0.5-eps.d) then begin
        dec(s);
        while s^='0' do begin
         dec(s);
        end;
        inc(s);
				goto ret1;
       end;
      end;
      break;
     end;
     inc(i);
     u.d:=u.d*10.0;
    end;
   end;
fast_failed:
	 s:=s0;
   u:=d2;
	 k:=k0;
	 ilim:=ilim0;
  end;

  if (be>=0) and (k<=14) then begin
 	 ds:=tens[k];
   if (ndigits<0) and (ilim<=0) then begin
    bS:=nil;
    mhi:=nil;
    if (ilim<0) or (u.d<=(5*ds)) or ((not BiasUp) and (u.d=(5*ds))) then begin
     goto no_digits;
    end;
    goto one_digit;
   end;
   i:=1;
   while true do begin
    L:=trunc(u.d/ds);
    u.d:=u.d-(L*ds);
    byte(s^):=ord('0')+L;
    inc(s);
    if u.d=0 then begin
 		 break;
		end;
    if i=ilim then begin
     u.d:=u.d+u.d;
		 if (u.d>ds) or ((u.d=ds) and ((L and 1)<>0)) then begin
bump_up:
      dec(s);
      while s^='9' do begin
       if s=s0 then begin
        inc(k);
        s^:='0';
        break;
       end;
       dec(s);
      end;
      inc(s^);
      inc(s);
     end;
		 break;
    end;
    inc(i);
    u.d:=u.d*10.0;
   end;
	 goto ret1;
  end;

	m2:=b2;
	m5:=b5;
	mhi:=nil;
  mlo:=nil;
	if leftright<>0 then begin
   if denorm<>0 then begin
		i:=be+(((1023+(53-1))-1)+1);
   end else begin
    i:=(1+53)-bbits;
   end;
   inc(b2,i);
   inc(s2,i);
   mhi:=i2b(1);
  end;
	if (m2>0) and (s2>0) then begin
   if m2<s2 then begin
    i:=m2;
   end else begin
    i:=s2;
   end;
   dec(b2,i);
   dec(m2,i);
   dec(s2,i);
  end;
	if b5>0 then begin
	 if leftright<>0 then begin
    if m5>0 then begin
     mhi:=pow5mult(mhi,m5);
		 b1:=mult(mhi,b);
     Bfree(b);
		 b:=b1;
    end;
    j:=b5-m5;
    if j<>0 then begin
		 b:=pow5mult(b,j);
    end;
   end else begin
		b:=pow5mult(b,b5);
   end;
  end;
	bS:=i2b(1);
	if s5>0 then begin
 	 bS:=pow5mult(bS,s5);
  end;

	spec_case:=0;
	if (mode<2) or (leftright<>0) then begin
   if ((u.hi and ($7ff00000 and not $100000))<>0) and (((u.hi and $fffff) or u.lo)=0) then begin
    inc(b2,1);
    inc(s2,1);
    spec_case:=1;
   end;
  end;

  if s5<>0 then begin
   i:=32-hi0bits(bS.x[bS.wds-1]);
  end else begin
   i:=1;
  end;
  i:=(i+s2) and $1f;
  if i<>0 then begin
   i:=32-i;
  end;

	i:=dshift(bS,s2);
  inc(b2,i);
  inc(m2,i);
  inc(s2,i);
	if b2>0 then begin
   b:=lshift(b,b2);
  end;
	if s2>0 then begin
   bS:=lshift(bS,s2);
  end;
	if k_check<>0 then begin
   if cmp(b,bS)<0 then begin
    dec(k);
		b:=multadd(b,10,0);
    if leftright<>0 then begin
     mhi:=multadd(mhi,10,0);
    end;
    ilim:=ilim1;
   end;
  end;
	if (ilim<=0) and (mode in [3,5]) then begin
   bS:=multadd(bS,5,0);
	 if (ilim<0) or (cmp(b,bS)<=0) or ((not BiasUp) and (i=0)) then begin
no_digits:
		k:=(-1)-ndigits;
    goto ret;
   end;
one_digit:
   s^:='1';
   inc(s);
   inc(k);
	 goto ret;
	end;
	if leftright<>0 then begin
	 if m2>0 then begin
    mhi:=lshift(mhi,m2);
	 end;
   mlo:=mhi;
	 if spec_case<>0 then begin
	 	mhi:=Balloc(mhi^.k);
    BBCopy(mhi,mlo);
	 	mhi:=lshift(mhi,1);
   end;
   i:=1;
   while true do begin
    dig:=quorem(b,bS)+ord('0');
		j:=cmp(b,mlo);
		delta:=diff(bS,mhi);
    if delta^.sign<>0 then begin
     j1:=1;
    end else begin
 		 j1:=cmp(b,delta);
    end;
    Bfree(delta);

{$ifndef ROUND_BIASED}
		if (j1=0) and (mode<>1) and ((u.lo and 1)=0) then begin
		 if dig=ord('9') then begin
      goto round_9_up;
     end;
		 if j>0 then begin
      inc(dig);
     end;
     byte(s^):=dig;
     inc(s);
		 goto ret;
    end;
{$endif}

		if (j<0) or ((j=0) and (mode<>1) {$ifndef ROUND_BIASED}and ((u.lo and 1)=0){$endif}) then begin
     if (b^.x[0]=0) and (b^.wds<=1) then begin
      goto accept_dig;
     end;
     if j1>0 then begin
      b:=lshift(b,1);
			j1:=cmp(b,bS);
      if ((j1>0) or ((j1=0) and (((dig and 1)<>0) or BiasUp))) and (dig=ord('9')) then begin
       inc(dig);
       goto round_9_up;
      end;
      inc(dig);
     end;
accept_dig:
     byte(s^):=dig;
     inc(s);
		 goto ret;
    end;
    if j1>0 then begin
     if dig=ord('9') then begin
round_9_up:
      s^:='9';
      inc(s);
			goto roundoff;
     end;
     byte(s^):=dig+1;
     inc(s);
		 goto ret;
    end;
    byte(s^):=dig;
    inc(s);
		if i=ilim then begin
     break;
    end;
		b:=multadd(b,10,0);
    if mlo=mhi then begin
     mhi:=multadd(mhi,10,0);
     mlo:=mhi;
    end else begin
     mlo:=multadd(mlo,10,0);
     mhi:=multadd(mhi,10,0);
    end;
    inc(i);
   end;
  end else begin
   i:=1;
   while true do begin
    dig:=quorem(b,bS)+ord('0');
    byte(s^):=dig;
    inc(s);
		if (b^.x[0]=0) and (b^.wds<=1) then begin
     goto ret;
    end;
    if i>=ilim then begin
     break;
    end;
		b:=multadd(b,10,0);
    inc(i);
   end;
  end;

  b:=lshift(b,1);
	j:=cmp(b,bS);
	if (j>0) or ((j=0) and (((dig and 1)<>0) or BiasUp)) then begin
roundoff:
   dec(s);
   while s^='9' do begin
    if s=s0 then begin
     inc(k);
     s^:='1';
     inc(s);
     goto ret;
    end;
    dec(s);
   end;
   inc(s^);
   inc(s);
  end else begin
   dec(s);
   while s^='0' do begin
    dec(s);
   end;
   inc(s);
	end;
ret:
	Bfree(bS);
	if assigned(mhi) then begin
   if assigned(mlo) and (mlo<>mhi) then begin
		Bfree(mlo);
	 end;
   Bfree(mhi);
	end;
ret1:

	Bfree(b);
	s^:=#0;
	decpt^:=k+1;
	if assigned(rve) then begin
   rve^:=s;
  end;
  result:=s0;

retf:
 finally
  if OldFPUExceptionMask<>DtoAFPUExceptionMask then begin
   SetExceptionMask(OldFPUExceptionMask);
  end;
  if OldFPUPrecisionMode<>DtoAFPUPrecisionMode then begin
   SetPrecisionMode(OldFPUPrecisionMode);
  end;
  if OldFPURoundingMode<>DtoAFPURoundingMode then begin
   SetRoundMode(OldFPURoundingMode);
  end;
 end;
end;

function divrem(b:PBigint;divisor:TULong):TULong;
var remainder,a,dividend,quotientHi,quotientLo:TULong;
    bx,bp:PLong;
begin
 remainder:=0;
 if b^.wds>0 then begin
  bx:=@b^.x[0];
  bp:=@b^.x[b^.wds];
  repeat
   dec(bp);
   a:=bp^;
   dividend:=(remainder shl 16) or (a shr 16);
   quotientHi:=dividend div divisor;
   remainder:=dividend-(quotientHi*divisor);
   dividend:=(remainder shl 16) or (a and $ffff);
   quotientLo:=dividend div divisor;
   remainder:=dividend-(quotientLo*divisor);
   bp^:=(quotientHi shl 16) or (quotientLo and $ffff);
  until ptruint(bx)>=ptruint(bp);
  if b^.x[b^.wds-1]=0 then begin
   dec(b.wds);
  end;
 end;
 result:=remainder;
end;

function quorem2(b:PBigint;k:TLong):TULong;
var mask:TULong;
    w,n:TLong;
    bx,bxe:PLong;
begin
 result:=0;
 n:=k shr 5;
 k:=k and 31;
 mask:=(1 shl k)-1;
 w:=b.wds-n;
 if w>0 then begin
  bx:=@b^.x[0];
  bxe:=@b^.x[n];
  result:=bxe^ shr k;
  bxe^:=bxe^ and mask;
  if (w=2) and (k<>0) then begin
   result:=result or (PULongs(bxe)^[1] shr (32-k));
  end;
  inc(n);
  while (bxe^=0) and (ptruint(bxe)>=ptruint(bx)) do begin
   dec(n);
   dec(bxe);
  end;
  b^.wds:=n;
 end;
end;

function dtobase(dd:double;base:TInt):PUChar;
label retf,err1,err2;
const MaxBufferSize=2048; // 1078
      Base36:array[0..35] of TUChar='0123456789abcdefghijklmnopqrstuvwxyz';
var d,di,df:TU;
    buffer,p,pInt,q:PUChar;
    digit,n,m:TULong;
    e,bbits:TInt;
    s2,j,j1:TLong;
    done:boolean;
    b,s,mlo,mhi,delta:PBigint;
    ch:TUChar;
    OldFPUExceptionMask:TFPUExceptionMask;
    OldFPUPrecisionMode:TFPUPrecisionMode;
    OldFPURoundingMode:TFPURoundingMode;
begin
 if (base<2) or (base>36) then begin
  result:=nil;
  exit;
 end;

 OldFPUExceptionMask:=GetExceptionMask;
 OldFPUPrecisionMode:=GetPrecisionMode;
 OldFPURoundingMode:=GetRoundMode;
 try
  if OldFPUExceptionMask<>DtoAFPUExceptionMask then begin
   SetExceptionMask(DtoAFPUExceptionMask);
  end;
  if OldFPUPrecisionMode<>DtoAFPUPrecisionMode then begin
   SetPrecisionMode(DtoAFPUPrecisionMode);
  end;
  if OldFPURoundingMode<>DtoAFPURoundingMode then begin
   SetRoundMode(DtoAFPURoundingMode);
  end;

  d.d:=dd;

  b:=nil;
  s:=nil;
  mlo:=nil;
  mhi:=nil;

  GetMem(buffer,MaxBufferSize);
  FillChar(buffer^,MaxBufferSize,#0);
  if assigned(buffer) then begin
   p:=buffer;
   if (d.hi and $80000000)<>0 then begin
    d.hi:=d.hi and $7fffffff;
    p^:='-';
    inc(p);
   end;

   if (d.hi and $7ff00000)=$7ff00000 then begin
    if ((d.hi and $000fffff) or d.lo)=0 then begin
     strcpy(p,'Infinity');
    end else begin
     strcpy(p,'NaN');
    end;
    goto retf;
   end;

   pInt:=p;
   di.d:=system.int(d.d);
   if frac(d.d)<0 then begin
    di.d:=di.d-1;
   end;
   if di.d<4294967295.0 then begin
    n:=int64(trunc(di.d));
    if n=0 then begin
     p^:='0';
     inc(p);
    end else begin
     repeat
      m:=n div TULong(base);
      digit:=n-(m*TULong(base));
      n:=m;
      if digit>35 then begin
       goto err1;
      end;
      p^:=Base36[digit];
      inc(p);
     until n=0;
    end;
   end else begin
    b:=d2b(@di,@e,@bbits);
    b:=lshift(b,e);
    repeat
     digit:=divrem(b,base);
     if digit>35 then begin
err1:
      Bfree(b);
      FreeMem(buffer);
      buffer:=nil;
      goto retf;
err2:
      Bfree(s);
      if mlo<>mhi then begin
       Bfree(mlo);
      end;
      Bfree(mhi);
      goto err1;
     end;
     p^:=Base36[digit];
     inc(p);
    until b^.wds=0;
    Bfree(b);
   end;

   q:=@p[-1];
   while ptruint(q)>ptruint(pInt) do begin
    ch:=pInt^;
    pInt^:=q^;
    inc(pInt);
    q^:=ch;
    dec(q);
   end;

   df.d:=d.d-di.d;
   if df.d<>0 then begin
    p^:='.';
    inc(p);
    b:=d2b(@df,@e,@bbits);
    s2:=-longint(longword((d.hi shr 20) and $7ff));
    if s2=0 then begin
     s2:=-1;
    end;
    inc(s2,1023+53);
    mlo:=i2b(1);
    mhi:=mlo;
    if (((d.hi and $fffff) or d.lo)=0) and ((d.hi and ($7ff00000 and ($7ff00000 shl 1)))<>0) then begin
     inc(s2);
     mhi:=i2b(2);
    end;
    b:=lshift(b,e+s2);
    s:=lshift(i2b(1),s2);
    done:=false;
    repeat
     b:=multadd(b,base,0);
     digit:=quorem2(b,s2);
     if mlo=mhi then begin
      mlo:=multadd(mlo,base,0);
      mhi:=mlo;
     end else begin
      mlo:=multadd(mlo,base,0);
      mhi:=multadd(mhi,base,0);
     end;
     j:=cmp(b,mlo);
     delta:=diff(s,mhi);
     if delta.sign<>0 then begin
      j1:=1;
     end else begin
      j1:=cmp(b,delta);
     end;
     Bfree(delta);
{$ifdef ROUND_BIASED}
     if j<=0 then begin
      if j1>0 then begin
       b:=lshift(b,1);
       j1:=cmp(b,s);
       if j1>0 then begin
        inc(digit);
       end;
      end;
      done:=true;
     end else if j1>0 then begin
      inc(digit);
      done:=true;
     end;
{$else}
     if (j1=0) and ((d.lo and 1)=0) then begin
      if j>0 then begin
       inc(digit);
      end;
      done:=true;
     end else if (j<0) or ((j=0) and ((d.lo and 1)=0)) then begin
      if j1>0 then begin
       b:=lshift(b,1);
       j1:=cmp(b,s);
       if j1>0 then begin
        inc(digit);
       end;
      end;
      done:=true;
     end else if j1>0 then begin
      inc(digit);
      done:=true;
     end;
{$endif}
     if digit>35 then begin
      goto err2;
     end;
     p^:=Base36[digit];
     inc(p);
    until done;
   end;

   Bfree(b);
   Bfree(s);
   if mlo<>mhi then begin
    Bfree(mlo);
   end;
   Bfree(mhi);

   p^:=#0;
  end;

retf:
  result:=buffer;
 finally
  if OldFPUExceptionMask<>DtoAFPUExceptionMask then begin
   SetExceptionMask(OldFPUExceptionMask);
  end;
  if OldFPUPrecisionMode<>DtoAFPUPrecisionMode then begin
   SetPrecisionMode(OldFPUPrecisionMode);
  end;
  if OldFPURoundingMode<>DtoAFPURoundingMode then begin
   SetRoundMode(OldFPURoundingMode);
  end;
 end;
end;

function dtostr(dd:double;mode,precision:TInt):ansistring;
var decPt,sign,nDigits,minNDigits:TInt;
    d:TU;
    Buffer,numBegin,numEnd,p,q:PUChar;
    exponentialNotation:boolean;
    i,j,k,BufferSize,DestBufferSize:integer;
begin
 result:='';
 Buffer:=nil;
 try
  if (mode=DTOSTR_FIXED) and ((dd<=-1e21) or (dd>=1e21)) then begin
   mode:=DTOSTR_STANDARD;
  end;

  d.d:=dd;

  numBegin:=dtoa(d.d,dtoaModes[mode],precision,@decPt,@sign,@numEnd);

  if assigned(numBegin) then begin
   nDigits:=ptruint(numEnd)-ptruint(numBegin);

   if decPt=9999 then begin

    BufferSize:=nDigits+4;
    GetMem(Buffer,BufferSize);
    FillChar(Buffer^,BufferSize,#0);
    Move(numBegin^,Buffer[2],nDigits);
    freedtoa(numBegin);

    numBegin:=@Buffer[2];
    numEnd:=@numBegin[nDigits];
    numEnd^:=#0;

   end else begin

    exponentialNotation:=false;
    minNDigits:=0;
    case Mode of
     DTOSTR_STANDARD:begin
      if (decPt<-5) or (decPt>21) then begin
       exponentialNotation:=true;
      end else begin
       minNDigits:=decPt;
      end;
     end;
     DTOSTR_STANDARD_EXPONENTIAL:begin
      exponentialNotation:=true;
     end;
     DTOSTR_FIXED:begin
      minNDigits:=decPt;
      if precision>=0 then begin
       inc(minNDigits,precision);
      end;
     end;
     DTOSTR_EXPONENTIAL:begin
      if precision<0 then begin
       precision:=0;
      end;
      minNDigits:=precision;
      exponentialNotation:=true;
     end;
     DTOSTR_PRECISION:begin
      if precision<0 then begin
       precision:=0;
      end;
      minNDigits:=precision;
      exponentialNotation:=(decPt<-5) or (decPt>precision);
     end;
    end;

    if nDigits<minNDigits then begin
     DestBufferSize:=minNDigits;
    end else begin
     DestBufferSize:=nDigits;
    end;
    inc(DestBufferSize,4);
    if exponentialNotation then begin
     inc(DestBufferSize,16);
    end else if decPt<>nDigits then begin
     if decPt>0 then begin
      inc(DestBufferSize,decPt);
     end else begin
      inc(DestBufferSize,1-decPt);
     end;
    end;

    BufferSize:=16;
    while BufferSize<=DestBufferSize do begin
     inc(BufferSize,BufferSize);
    end;

    GetMem(Buffer,BufferSize);
    FillChar(Buffer^,BufferSize,#0);
    Move(numBegin^,Buffer[2],nDigits);
    freedtoa(numBegin);

    numBegin:=@Buffer[2];
    numEnd:=@numBegin[nDigits];
    numEnd^:=#0;

    if nDigits<minNDigits then begin
     p:=@numBegin[minNDigits];
     nDigits:=minNDigits;
     repeat
      numEnd^:='0';
      inc(numEnd);
     until ptruint(numEnd)>=ptruint(p);
     numEnd^:=#0;
    end;

    if exponentialNotation then begin

     if nDigits<>1 then begin
      dec(numBegin);
      numBegin[0]:=numBegin[1];
      numBegin[1]:='.';
     end;

     p:=numEnd;
     p^:='e';
     inc(p);

     i:=decPt-1;
     if i=0 then begin
      p^:='0';
      inc(p);

     end else begin
      if i<0 then begin
       i:=-i;
       p^:='-';
       inc(p);
      end;

      j:=i;
      k:=0;
      while i<>0 do begin
       i:=i div 10;
       inc(k);
      end;

      inc(p,k);

      i:=j;
      while i<>0 do begin
       j:=i div 10;
       k:=i-(j*10);
       i:=j;
       dec(p);
       byte(p^):=k+ord('0');
      end;
     end;
    end else if decPt<>nDigits then begin
     if decPt>0 then begin
      dec(numBegin);

      p:=numBegin;
      repeat
       p^:=p[1];
       inc(p);
       dec(decPt);
      until decPt=0;

      p^:='.';
     end else begin
      p:=numEnd;
      inc(numEnd,1-decPt);
      q:=numEnd;
      numEnd^:=#0;
      while p<>numBegin do begin
       dec(q);
       dec(p);
       q^:=p^;
      end;

      p:=@numBegin[1];
      while ptruint(p)<ptruint(q) do begin
       p^:='0';
       inc(p);
      end;
      numBegin^:='.';
      dec(numBegin);
      numBegin^:='0';
     end;
    end;
   end;

   if (sign<>0) and (not ((d.hi=$80000000) and (d.lo=0))) and not (((d.hi and $7ff00000)=$7ff00000) and (((d.hi and $fffff) or d.lo)<>0)) then begin
    dec(numBegin);
    numBegin^:='-';
   end;

   result:=numBegin;
  end;
 finally
  if assigned(Buffer) then begin
   FreeMem(Buffer);
  end;
 end;
end;

procedure InitDTOA;
var p5,p51:PBigint;
    i:TLong;
begin
 DtoACriticalSection:=TCriticalSection.Create;
 FillChar(BigintFreeList,sizeof(TBigintFreeList),#0);
 FillChar(BigintFreeListCounter,sizeof(TBigintFreeListCounter),#0);
 p5:=i2b(625);
 p5s:=p5;
 for i:=1 to 8 do begin
  p51:=mult(p5,p5);
  p5^.next:=p51;
  p5:=p51;
 end;
 hexdig_init;
 basedig_init;
end;

procedure DoneDTOA;
var i:TInt;
    p5,b,nb:PBigint;
begin
 while assigned(p5s) do begin
  p5:=p5s;
  p5s:=p5s^.next;
  FreeMem(p5);
 end;
 for i:=low(TBigintFreeList) to high(TBigintFreeList) do begin
  b:=BigintFreeList[i];
  while assigned(b) do begin
   nb:=b^.next;
   FreeMem(b);
   b:=nb;
  end;
  BigintFreeList[i]:=nil;
 end;
 DtoACriticalSection.Destroy;
end;

procedure InitBESEN;
begin
 InitDTOA;
end;

procedure DoneBESEN;
begin
 DoneDTOA;
end;

initialization
 InitBESEN;
finalization
 DoneBESEN;
end.
