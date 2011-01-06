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
unit BESENNumberUtils;
{$i BESEN.inc}

interface

uses Math,BESENConstants,BESENTypes,BESENStringUtils,BESENDTOA;

type PBESENDoubleBytes=^TBESENDoubleBytes;
     TBESENDoubleBytes=array[0..sizeof(double)-1] of byte;

     TBESENNumberCodeFlagLookupTable=array[word,boolean] of byte;

     PBESENNumberCodeAbstractRelationalLookupTableItem=^TBESENNumberCodeAbstractRelationalLookupTableItem;
     TBESENNumberCodeAbstractRelationalLookupTableItem=packed record
      IsNotUndefined:bytebool;
      ResultBooleanValue:bytebool;
      DoCompare:bytebool;
      Spacer:bytebool;
     end;

     TBESENNumberCodeAbstractRelationalLookupTable=array[byte] of TBESENNumberCodeAbstractRelationalLookupTableItem;

const BESEN_ROUND_TO_NEAREST=0;
      BESEN_ROUND_TOWARD_ZERO=1;
      BESEN_ROUND_UPWARD=2;
      BESEN_ROUND_DOWNWARD=3;

      BESEN_CHECKNUMBERSTRING_FAIL=-1;
      BESEN_CHECKNUMBERSTRING_EMPTY=0;
      BESEN_CHECKNUMBERSTRING_VALID=1;
      BESEN_CHECKNUMBERSTRING_INFNEG=2;
      BESEN_CHECKNUMBERSTRING_INFPOS=3;
      BESEN_CHECKNUMBERSTRING_NAN=4;

{$ifdef BIG_ENDIAN}
      BESENDoubleNaN:TBESENDoubleBytes=($7f,$ff,$ff,$ff,$ff,$ff,$ff,$ff);
      BESENDoubleInfPos:TBESENDoubleBytes=($7f,$f0,$00,$00,$00,$00,$00,$00);
      BESENDoubleInfNeg:TBESENDoubleBytes=($ff,$f0,$00,$00,$00,$00,$00,$00);
      BESENDoubleMax:TBESENDoubleBytes=($7f,$ef,$ff,$ff,$ff,$ff,$ff,$ff);
      BESENDoubleMin:TBESENDoubleBytes=($00,$00,$00,$00,$00,$00,$00,$01);
{$else}
      BESENDoubleNaN:TBESENDoubleBytes=($ff,$ff,$ff,$ff,$ff,$ff,$ff,$7f);
      BESENDoubleInfPos:TBESENDoubleBytes=($00,$00,$00,$00,$00,$00,$f0,$7f);
      BESENDoubleInfNeg:TBESENDoubleBytes=($00,$00,$00,$00,$00,$00,$f0,$ff);
      BESENDoubleMax:TBESENDoubleBytes=($ff,$ff,$ff,$ff,$ff,$ff,$ef,$7f);
      BESENDoubleMin:TBESENDoubleBytes=($01,$00,$00,$00,$00,$00,$00,$00);
{$endif}
      BESENDoubleZero:TBESENNumber=0.0;
      BESENDoubleOne:TBESENNumber=1.0;

      BESENNumZero:TBESENNumber=0;
      BESENNumOne:TBESENNumber=1;

var BESENNumberCodeFlagLookupTable:TBESENNumberCodeFlagLookupTable;
    BESENNumberCodeAbstractRelationalLookupTable:TBESENNumberCodeAbstractRelationalLookupTable;

function BESENNumberCodeFlags(const AValue:TBESENNumber):byte; {$ifdef caninline}inline;{$endif}
    
function BESENIntLog2(x:longword):longword; {$ifdef cpu386}assembler; register;{$endif}

function BESENToInt(v:TBESENNumber):TBESENINT64;
function BESENToInt32(v:TBESENNumber):TBESENINT32;
function BESENToUInt32(v:TBESENNumber):TBESENUINT32;
function BESENToInt16(v:TBESENNumber):TBESENINT32;
function BESENToUInt16(v:TBESENNumber):TBESENUINT32;

function BESENToIntFast(v:PBESENNumber):TBESENINT64; {$ifdef UseRegister}register;{$endif}{$ifdef caninline}inline;{$endif}
function BESENToInt32Fast(v:PBESENNumber):TBESENINT32; {$ifdef UseRegister}register;{$endif}{$ifdef caninline}inline;{$endif}
function BESENToUInt32Fast(v:PBESENNumber):TBESENUINT32; {$ifdef UseRegister}register;{$endif}{$ifdef caninline}inline;{$endif}
function BESENToInt16Fast(v:PBESENNumber):TBESENINT32; {$ifdef UseRegister}register;{$endif}{$ifdef caninline}inline;{$endif}
function BESENToUInt16Fast(v:PBESENNumber):TBESENUINT32; {$ifdef UseRegister}register;{$endif}{$ifdef caninline}inline;{$endif}

function BESENFloatToStr(const v:TBESENNumber):TBESENString;
function BESENFloatToLocaleStr(const v:TBESENNumber):TBESENString;

function BESENSameValue(const A,B:TBESENNumber):boolean;
function BESENIsNaN(const AValue:TBESENNumber):boolean; {$ifdef caninline}inline;{$endif}
function BESENIsInfinite(const AValue:TBESENNumber):boolean; {$ifdef caninline}inline;{$endif}
function BESENIsFinite(const AValue:TBESENNumber):boolean; {$ifdef caninline}inline;{$endif}
function BESENIsPosInfinite(const AValue:TBESENNumber):boolean; {$ifdef caninline}inline;{$endif}
function BESENIsNegInfinite(const AValue:TBESENNumber):boolean; {$ifdef caninline}inline;{$endif}
function BESENIsPosZero(const AValue:TBESENNumber):boolean; {$ifdef caninline}inline;{$endif}
function BESENIsNegZero(const AValue:TBESENNumber):boolean; {$ifdef caninline}inline;{$endif}
function BESENIsZero(const AValue:TBESENNumber):boolean; {$ifdef caninline}inline;{$endif}
function BESENIsNegative(const AValue:TBESENNumber):boolean; {$ifdef caninline}inline;{$endif}
function BESENIsSameValue(const a,b:TBESENNumber):boolean; {$ifdef caninline}inline;{$endif}
function BESENFloor(FloatValue:TBESENNumber):TBESENNumber; {$ifdef caninline}inline;{$endif}
function BESENCeil(FloatValue:TBESENNumber):TBESENNumber; {$ifdef caninline}inline;{$endif}

function BESENStringToDoubleExact(const StringValue:TBESENString;RoundingMode:integer=BESEN_ROUND_TO_NEAREST):double;
function BESENStringToDoubleFast(const StringValue:TBESENString;RoundingMode:integer=BESEN_ROUND_TO_NEAREST):double;

function BESENStringToNumber(const StringValue:TBESENString;EmptyIsValid:TBESENBoolean=true;AcceptHex:TBESENBoolean=false;OnlyNumber:boolean=false;AcceptHexSign:boolean=true):TBESENNumber;

function BESENNumberToString(const Value:TBESENNumber):TBESENString;

function BESENCheckNumberString(const StringValue:TBESENString;var StartPosition,EndPosition:integer;IsParseFloat,AcceptHex,OnlyNumber,AcceptHexSign:boolean):integer;

function BESENModulo(x,y:double):double;{$ifdef cpu386}stdcall; assembler;{$endif}
function BESENModuloPos(x,y:double):double;

implementation

uses BESENLocale;

procedure InitBESENNumberTables;
var i:word;
    j:boolean;
    c,ca,cb:byte;
    Item:PBESENNumberCodeAbstractRelationalLookupTableItem;
begin
 for i:=low(TBESENNumberCodeFlagLookupTable) to high(TBESENNumberCodeFlagLookupTable) do begin
  for j:=low(boolean) to high(boolean) do begin
   c:=0;
   if j then begin
    if (i and $7ff0)=$7ff0 then begin
     c:=c or bncfNAN;
    end;
   end else begin
    if (i and $7ff0)=$7ff0 then begin
     if (i and $000f)<>0 then begin
      c:=c or bncfNAN;
     end else begin
      c:=c or bncfINFINITE;
     end;
    end else if (i and $7fff)=0 then begin
     c:=c or bncfZERO;
    end;
   end;
   if (i and $8000)<>0 then begin
    c:=c or bncfNEGATIVE;
   end;
   BESENNumberCodeFlagLookupTable[i,j]:=c;
  end;
 end;
 for c:=low(byte) to high(byte) do begin
  Item:=@BESENNumberCodeAbstractRelationalLookupTable[c];
  ca:=c shr 4;
  cb:=c and $f;
  Item^.Spacer:=false;
  if ((ca or cb) and bncfNAN)<>0 then begin
   Item^.IsNotUndefined:=false;
   Item^.ResultBooleanValue:=false;
   Item^.DoCompare:=false;
  end else if (((ca and cb) and bncfZERO)<>0) and (((ca xor cb) and bncfNEGATIVE)<>0) then begin
   Item^.IsNotUndefined:=true;
   Item^.ResultBooleanValue:=false;
   Item^.DoCompare:=false;
  end else if (ca and (bncfINFINITE or bncfNEGATIVE))=bncfINFINITE then begin
   Item^.IsNotUndefined:=true;
   Item^.ResultBooleanValue:=false;
   Item^.DoCompare:=false;
  end else if (cb and (bncfINFINITE or bncfNEGATIVE))=bncfINFINITE then begin
   Item^.IsNotUndefined:=true;
   Item^.ResultBooleanValue:=true;
   Item^.DoCompare:=false;
  end else if (cb and (bncfINFINITE or bncfNEGATIVE))=(bncfINFINITE or bncfNEGATIVE) then begin
   Item^.IsNotUndefined:=true;
   Item^.ResultBooleanValue:=false;
   Item^.DoCompare:=false;
  end else if (ca and (bncfINFINITE or bncfNEGATIVE))=(bncfINFINITE or bncfNEGATIVE) then begin
   Item^.IsNotUndefined:=true;
   Item^.ResultBooleanValue:=true;
   Item^.DoCompare:=false;
  end else begin
   Item^.IsNotUndefined:=true;
   Item^.ResultBooleanValue:=false;
   Item^.DoCompare:=true;
  end;
 end;
end;

function BESENNumberCodeFlags(const AValue:TBESENNumber):byte; {$ifdef caninline}inline;{$endif}
{$ifdef fpc}
var HighPart:longword;
begin
 HighPart:=qword(pointer(@AValue)^) shr 32;
 result:=BESENNumberCodeFlagLookupTable[HighPart shr 16,((HighPart and $ffff) or longword(qword(pointer(@AValue)^) and $ffffffff))<>0];
end;
{$else}
{$ifdef cpu386}
asm
 mov ecx,dword ptr [AValue+4]
 mov edx,ecx
 and edx,$0000ffff
 or edx,dword ptr [AValue]
 setnz dl
 and edx,$7f
 shr ecx,16
 mov al,byte ptr [offset BESENNumberCodeFlagLookupTable+ecx*2+edx]
end;
{$else}
var HighPart:longword;
begin
 HighPart:=int64(pointer(@AValue)^) shr 32;
 result:=BESENNumberCodeFlagLookupTable[HighPart shr 16,((HighPart and $ffff) or longword(int64(pointer(@AValue)^) and $ffffffff))<>0];
end;
{$endif}
{$endif}

function BESENConvertMantissaExponentToDouble(Mantissa:int64;MantissaSize,FractionalDigits,FractionalExponent,Exponent,ExponentSign:integer):double; {$ifdef caninline}inline;{$endif}
const MaxExponentOfTen=308;
      MaxExponentOfTenMinusTwo=MaxExponentOfTen-2;
      PowersOfTen:array[0..8] of TBESENParsingNumberType=(1e1,1e2,1e4,1e8,1e16,1e32,1e64,1e128,1e256);
var FloatValue,ExponentFactor:TBESENParsingNumberType;
    MantissaExponent,PartExponent,Index:integer;
    OldFPUExceptionMask:TFPUExceptionMask;
    OldFPURoundingMode:TFPURoundingMode;
    OldFPUPrecisionMode:TFPUPrecisionMode;
begin
 OldFPUExceptionMask:=GetExceptionMask;
 OldFPURoundingMode:=GetRoundMode;
 OldFPUPrecisionMode:=GetPrecisionMode;
 try
  if OldFPUExceptionMask<>BESENFPUExceptionMask then begin
   SetExceptionMask(BESENFPUExceptionMask);
  end;
  if OldFPURoundingMode<>rmNearest then begin
   SetRoundMode(rmNearest);
  end;
  if OldFPUPrecisionMode<>BESENFPUPrecisionMode then begin
   SetPrecisionMode(BESENFPUPrecisionMode);
  end;

  FloatValue:=Mantissa;

  MantissaExponent:=FractionalDigits+FractionalExponent;
  if MantissaSize>18 then begin
   dec(MantissaExponent,MantissaSize-18);
  end;

  if ExponentSign>0 then begin
   dec(Exponent,MantissaExponent);
  end else begin
   inc(Exponent,MantissaExponent);
  end;
  if Exponent<0 then begin
   ExponentSign:=-ExponentSign;
   Exponent:=-Exponent;
  end;

  while Exponent>0 do begin
   PartExponent:=Exponent;
   if PartExponent>MaxExponentOfTenMinusTwo then begin
    PartExponent:=MaxExponentOfTenMinusTwo;
   end;
   dec(Exponent,PartExponent);

   Index:=0;
   ExponentFactor:=1;
   while (PartExponent<>0) and (Index<length(PowersOfTen)) do begin
    if (PartExponent and 1)<>0 then begin
     ExponentFactor:=ExponentFactor*PowersOfTen[Index];
    end;
    inc(Index);
    PartExponent:=PartExponent shr 1;
   end;

   if ExponentSign>0 then begin
    FloatValue:=FloatValue*ExponentFactor;
   end else begin
    FloatValue:=FloatValue/ExponentFactor;
   end;
  end;
  result:=FloatValue;
 finally
  if OldFPUExceptionMask<>BESENFPUExceptionMask then begin
   SetExceptionMask(OldFPUExceptionMask);
  end;
  if OldFPURoundingMode<>rmNearest then begin
   SetRoundMode(OldFPURoundingMode);
  end;
  if OldFPUPrecisionMode<>BESENFPUPrecisionMode then begin
   SetPrecisionMode(OldFPUPrecisionMode);
  end;
 end;
end;

function BESENStringToDoubleExact(const StringValue:TBESENString;RoundingMode:integer=BESEN_ROUND_TO_NEAREST):double;
type PDoubleCasted=^TDoubleCasted;
     TDoubleCasted=packed record
      case byte of
       0:(Value:double);
       1:({$ifdef BIG_ENDIAN}Hi,Lo{$else}Lo,Hi{$endif}:longword);
     end;
const MantissaWords=12; //6; // 12
      MantissaDigits=52; //28; // 52
      WordTopBit=$8000;
      WordBits=16;
      WordBitShift=4;
      WordBitMask=WordBits-1;
      WordMask=$ffff;
      IEEEFormatBytes=8;
      IEEEFormatBits=IEEEFormatBytes shl 3;
      IEEEFormatExplicit=0;
      IEEEFormatExponent=11;
      IEEEFormatOneMask=WordTopBit shr ((IEEEFormatExponent+IEEEFormatExplicit) and WordBitMask);
      IEEEFormatOnePos=(IEEEFormatExponent+IEEEFormatExplicit) shr WordBitShift;
      IEEEFormatExpMax=1 shl (IEEEFormatExponent-1);
type PWords=^TWords;
     TWords=array[0..MantissaWords] of word;
     PTemp=^TTemp;
     TTemp=array[0..MantissaWords*2] of longword;
     PDigits=^TDigits;
     TDigits=array[0..MantissaDigits] of byte;
var MantissaPosition,Exponent,TenPower,TwoPower,ExtraTwos,Shift,i,p,q,r:integer;
    Bit,Carry:word;
    Negative,ExponentNegative,HasDigits,Started:boolean;
    ResultCasted:PDoubleCasted;
    Temp:PTemp;
    Digits:PDigits;
    MantissaMultiplicator,Mantissa:PWords;
 function MantissaMultiply(vTo,vFrom:PWords):integer;
 var i,j,k:integer;
     v:longword;
     t:PTemp;
 begin
  t:=Temp;
  FillChar(t^,sizeof(TTemp),#0);
  for i:=0 to MantissaWords-1 do begin
   for j:=0 to MantissaWords-1 do begin
    v:=longword(vTo^[i]+0)*longword(vFrom^[j]+0);
    k:=i+j;
    inc(t^[k],v shr WordBits);
    inc(t^[k+1],v and WordMask);
   end;
  end;
  for i:=high(TTemp) downto 1 do begin
   inc(t^[i-1],t^[i] shr WordBits);
   t^[i]:=t^[i] and WordMask;
  end;
  if (t^[0] and WordTopBit)<>0 then begin
   for i:=0 to MantissaWords-1 do begin
    vTo^[i]:=t^[i] and WordMask;
   end;
   result:=0;
  end else begin
   for i:=0 to MantissaWords-1 do begin
    vTo^[i]:=(t^[i] shl 1)+word(ord((t^[i+1] and WordTopBit)<>0));
   end;
   result:=-1;
  end;
 end;
 procedure MantissaShiftRight(var Mantissa:TWords;Shift:integer);
 var Bits,Words,InvBits,Position:integer;
     Carry,Current:longword;
 begin
  Bits:=Shift and WordBitMask;
  Words:=Shift shr WordBitShift;
  InvBits:=WordBits-Bits;
  Position:=high(TWords);
  if Bits=0 then begin
   if Words<>0 then begin
    while Position>=Words do begin
     Mantissa[Position]:=Mantissa[Position-Words];
     dec(Position);
    end;
   end;
  end else begin
   if (high(TWords)-Words)>=0 then begin
    Carry:=Mantissa[high(TWords)-Words] shr Bits;
   end else begin
    Carry:=0;
   end;
   while Position>Words do begin
    Current:=Mantissa[Position-(Words+1)];
    Mantissa[Position]:=(Current shl InvBits) or Carry;
    Carry:=Current shr Bits;
    dec(Position);
   end;
   Mantissa[Position]:=Carry;
   dec(Position);
  end;
  while Position>=0 do begin
   Mantissa[Position]:=0;
   dec(Position);
  end;
 end;
 procedure MantissaSetBit(var Mantissa:TWords;i:integer);
 begin
  Mantissa[i shr WordBitShift]:=Mantissa[i shr WordBitShift] or (WordTopBit shr (i and WordBitMask));
 end;
 function MantissaTestBit(var Mantissa:TWords;i:integer):boolean;
 begin
  result:=(Mantissa[i shr WordBitShift] shr ((not i) and WordBitMask))<>0;
 end;
 function MantissaIsZero(var Mantissa:TWords):boolean;
 var i:integer;
 begin
  result:=true;
  for i:=low(TWords) to High(TWords) do begin
   if Mantissa[i]<>0 then begin
    result:=false;
    break;
   end;
  end;
 end;
 function MantissaRound(Negative:boolean;var Mantissa:TWords;BitPos:integer):boolean;
 var i,p:integer;
     Bit:longword;
  function RoundAbsDown:boolean;
  var j:integer;
  begin
   Mantissa[i]:=Mantissa[i] and not (Bit-1);
   for j:=i+1 to high(TWords) do begin
    Mantissa[j]:=0;
   end;
   result:=false;
  end;
  function RoundAbsUp:boolean;
  var j:integer;
  begin
   Mantissa[i]:=(Mantissa[i] and not (Bit-1))+Bit;
   for j:=i+1 to high(TWords) do begin
    Mantissa[j]:=0;
   end;
   while (i>0) and (Mantissa[i]=0) do begin
    dec(i);
    inc(Mantissa[i]);
   end;
   result:=Mantissa[0]=0;
  end;
  function RoundTowardsInfinity:boolean;
  var j:integer;
      m:longword;
  begin
   m:=Mantissa[i] and ((Bit shl 1)-1);
   for j:=i+1 to high(TWords) do begin
    m:=m or Mantissa[j];
   end;
   if m<>0 then begin
    result:=RoundAbsUp;
   end else begin
    result:=RoundAbsDown;
   end;
  end;
  function RoundNear:boolean;
  var j:integer;
      m:longword;
  begin
   if (Mantissa[i] and Bit)<>0 then begin
    Mantissa[i]:=Mantissa[i] and not Bit;
    m:=Mantissa[i] and ((Bit shl 1)-1);
    for j:=i+1 to high(TWords) do begin
     m:=m or Mantissa[j];
    end;
    Mantissa[i]:=Mantissa[i] or Bit;
    if m<>0 then begin
     result:=RoundAbsUp;
    end else begin
     if MantissaTestBit(Mantissa,BitPos-1) then begin
      result:=RoundAbsUp;
     end else begin
      result:=RoundAbsDown;
     end;
    end;
   end else begin
    result:=RoundAbsDown;
   end;
  end;
 begin
  i:=BitPos shr WordBitShift;
  p:=BitPos and WordBitMask;
  Bit:=WordTopBit shr p;
  case RoundingMode of
   BESEN_ROUND_TO_NEAREST:begin
    result:=RoundNear;
   end;
   BESEN_ROUND_TOWARD_ZERO:begin
    result:=RoundAbsDown;
   end;
   BESEN_ROUND_UPWARD:begin
    if Negative then begin
     result:=RoundAbsDown;
    end else begin
     result:=RoundTowardsInfinity;
    end;
   end;
   BESEN_ROUND_DOWNWARD:begin
    if Negative then begin
     result:=RoundTowardsInfinity;
    end else begin
     result:=RoundAbsDown;
    end;
   end;
   else begin
    result:=false;
   end;
  end;
 end;
begin
 ResultCasted:=pointer(@result);
 ResultCasted^.Hi:=$7ff80000;
 ResultCasted^.Lo:=$00000000;
 i:=1;
 while (i<=length(StringValue)) and BESENUnicodeIsStringWhiteSpace(word(widechar(StringValue[i]))) do begin
  inc(i);
 end;
 if (i<=length(StringValue)) and ((StringValue[i]='-') or (StringValue[i]='+')) then begin
  Negative:=StringValue[i]='-';
  inc(i);
 end else begin
  Negative:=false;
 end;
 if ((i+7)<=length(StringValue)) and ((StringValue[i]='I') and (StringValue[i+1]='n') and (StringValue[i+2]='f') and (StringValue[i+3]='i') and (StringValue[i+4]='n') and (StringValue[i+5]='i') and (StringValue[i+6]='t') and (StringValue[i+7]='y')) then begin
  if Negative then begin
   ResultCasted^.Hi:=$fff00000;
   ResultCasted^.Lo:=$00000000;
  end else begin
   ResultCasted^.Hi:=$7ff00000;
   ResultCasted^.Lo:=$00000000;
  end;
 end else if ((i+2)<=length(StringValue)) and ((StringValue[i]='N') and (StringValue[i+1]='a') and (StringValue[i+2]='N')) then begin
  ResultCasted^.Hi:=$7ff80000;
  ResultCasted^.Lo:=$00000000;
 end else begin
  New(MantissaMultiplicator);
  New(Mantissa);
  New(Temp);
  New(Digits);
  try
   FillChar(Digits^,sizeof(TDigits),#0);

   p:=0;
   TenPower:=0;
   HasDigits:=false;
   Started:=false;
   ExponentNegative:=false;
   Exponent:=0;
   while i<=length(StringValue) do begin
    case word(widechar(StringValue[i])) of
     ord('0'):begin
      HasDigits:=true;
      inc(i);
     end;
     else begin
      break;
     end;
    end;
   end;
   while i<=length(StringValue) do begin
    case word(widechar(StringValue[i])) of
     ord('0')..ord('9'):begin
      HasDigits:=true;
      Started:=true;
      if p<=high(TDigits) then begin
       Digits^[p]:=word(widechar(StringValue[i]))-ord('0');
       inc(p);
      end;
      inc(TenPower);
      inc(i);
     end;
     else begin
      break;
     end;
    end;
   end;
   if (i<=length(StringValue)) and (StringValue[i]='.') then begin
    inc(i);
    if not Started then begin
     while i<=length(StringValue) do begin
      case word(widechar(StringValue[i])) of
       ord('0'):begin
        HasDigits:=true;
        dec(TenPower);
        inc(i);
       end;
       else begin
        break;
       end;
      end;
     end;
    end;
    while i<=length(StringValue) do begin
     case word(widechar(StringValue[i])) of
      ord('0')..ord('9'):begin
       HasDigits:=true;
       if p<=high(TDigits) then begin
        Digits^[p]:=word(widechar(StringValue[i]))-ord('0');
        inc(p);
       end;
       inc(i);
      end;
      else begin
       break;
      end;
     end;
    end;
   end;
   if HasDigits then begin
    if (i<=length(StringValue)) and ((StringValue[i]='e') or (StringValue[i]='E')) then begin
     inc(i);
     if (i<=length(StringValue)) and ((StringValue[i]='+') or (StringValue[i]='-')) then begin
      ExponentNegative:=StringValue[i]='-';
      inc(i);
     end;
     HasDigits:=false;
     while i<=length(StringValue) do begin
      case word(widechar(StringValue[i])) of
       ord('0')..ord('9'):begin
        Exponent:=(Exponent*10)+integer(word(widechar(StringValue[i]))-ord('0'));
        HasDigits:=true;
        inc(i);
       end;
       else begin
        break;
       end;
      end;
     end;
    end;
    if HasDigits then begin
     if ExponentNegative then begin
      dec(TenPower,Exponent);
     end else begin
      inc(TenPower,Exponent);
     end;

     FillChar(Mantissa^,sizeof(TWords),#0);

     Bit:=WordTopBit;
     q:=0;
     Started:=false;
     TwoPower:=0;
     MantissaPosition:=0;
     while MantissaPosition<MantissaWords do begin
      Carry:=0;
      while (p>q) and (Digits^[p-1]=0) do begin
       dec(p);
      end;
      if p<=q then begin
       break;
      end;
      r:=p;
      while r>q do begin
       dec(r);
       i:=(2*Digits^[r])+Carry;
       if i>=10 then begin
        dec(i,10);
        Carry:=1;
       end else begin
        Carry:=0;
       end;
       Digits^[r]:=i;
      end;
      if Carry<>0 then begin
       Mantissa^[MantissaPosition]:=Mantissa^[MantissaPosition] or Bit;
       Started:=true;
      end;
      if Started then begin
       if Bit=1 then begin
        Bit:=WordTopBit;
        inc(MantissaPosition);
       end else begin
        Bit:=Bit shr 1;
       end;
      end else begin
       dec(TwoPower);
      end;
     end;
     inc(TwoPower,TenPower);

     if TenPower<0 then begin
      for i:=0 to high(TWords)-1 do begin
       MantissaMultiplicator^[i]:=$cccc;
      end;
      MantissaMultiplicator^[high(TWords)]:=$cccd;
      ExtraTwos:=-2;
      TenPower:=-TenPower;
     end else if TenPower>0 then begin
      MantissaMultiplicator^[0]:=$a000;
      for i:=1 to high(TWords) do begin
       MantissaMultiplicator^[i]:=$0000;
      end;
      ExtraTwos:=3;
     end else begin
      ExtraTwos:=0;
     end;
     while TenPower<>0 do begin
      if (TenPower and 1)<>0 then begin
       inc(TwoPower,ExtraTwos+MantissaMultiply(Mantissa,MantissaMultiplicator));
      end;
      inc(ExtraTwos,ExtraTwos+MantissaMultiply(MantissaMultiplicator,MantissaMultiplicator));
      TenPower:=TenPower shr 1;
     end;

     Exponent:=TwoPower;
     if (Mantissa^[0] and WordTopBit)<>0 then begin
      dec(Exponent);

      if (Exponent>=(2-IEEEFormatExpMax)) and (Exponent<=IEEEFormatExpMax) then begin
       inc(Exponent,IEEEFormatExpMax-1);
       MantissaShiftRight(Mantissa^,IEEEFormatExponent+IEEEFormatExplicit);
       MantissaRound(Negative,Mantissa^,IEEEFormatBits);
       if MantissaTestBit(Mantissa^,IEEEFormatExponent+IEEEFormatExplicit-1) then begin
        MantissaShiftRight(Mantissa^,1);
        inc(Exponent);
       end;
       if Exponent>=(IEEEFormatExpMax shl 1)-1 then begin
        ResultCasted^.Hi:=$7ff00000;
        ResultCasted^.Lo:=$00000000;
       end else begin
        ResultCasted^.Hi:=(((Exponent shl 4) or (Mantissa^[0] and $f)) shl 16) or Mantissa^[1];
        ResultCasted^.Lo:=(Mantissa^[2] shl 16) or Mantissa^[3];
       end;
      end else if Exponent>0 then begin
       ResultCasted^.Hi:=$7ff00000;
       ResultCasted^.Lo:=$00000000;
      end else begin
       Shift:=IEEEFormatExplicit-(Exponent+(IEEEFormatExpMax-(2+IEEEFormatExponent)));
       MantissaShiftRight(Mantissa^,Shift);
       MantissaRound(Negative,Mantissa^,IEEEFormatBits);
       if (Mantissa^[IEEEFormatOnePos] and IEEEFormatOneMask)<>0 then begin
        Exponent:=1;
        if IEEEFormatExplicit=0 then begin
         Mantissa^[IEEEFormatOnePos]:=Mantissa^[IEEEFormatOnePos] and not IEEEFormatOneMask;
        end;
        Mantissa^[0]:=Mantissa^[0] or (Exponent shl (WordBitMask-IEEEFormatExponent));
        ResultCasted^.Hi:=(((Exponent shl 4) or (Mantissa^[0] and $f)) shl 16) or Mantissa^[1];
        ResultCasted^.Lo:=(Mantissa^[2] shl 16) or Mantissa^[3];
       end else begin
        if MantissaIsZero(Mantissa^) then begin
         ResultCasted^.Hi:=$00000000;
         ResultCasted^.Lo:=$00000000;
        end else begin
         ResultCasted^.Hi:=(Mantissa^[0] shl 16) or Mantissa^[1];
         ResultCasted^.Lo:=(Mantissa^[2] shl 16) or Mantissa^[3];
        end;
       end;
      end;
      if Negative then begin
       ResultCasted^.Hi:=ResultCasted^.Hi or $80000000;
      end;
     end else begin
      ResultCasted^.Hi:=$00000000;
      ResultCasted^.Lo:=$00000000;
     end;
    end;
   end;
  finally
   Dispose(MantissaMultiplicator);
   Dispose(Mantissa);
   Dispose(Temp);
   Dispose(Digits);
  end;
 end;
end;

function BESENStringToDoubleFast(const StringValue:TBESENString;RoundingMode:integer=BESEN_ROUND_TO_NEAREST):double;
type PDoubleCasted=^TDoubleCasted;
     TDoubleCasted=packed record
      case byte of
       0:(Value:double);
       1:({$ifdef BIG_ENDIAN}Hi,Lo{$else}Lo,Hi{$endif}:longword);
     end;
const MaxExponentOfTen=308;
      MaxExponentOfTenMinusTwo=MaxExponentOfTen-2;
      PowersOfTen:array[0..8] of double=(1e1,1e2,1e4,1e8,1e16,1e32,1e64,1e128,1e256);
var i:integer;
    Negative,HasDigits:boolean;
    Mantissa:int64;
    MantissaSize,FractionalDigits,FractionalExponent,ExponentSign,Exponent:integer;
    ResultCasted:PDoubleCasted;
    FloatValue,ExponentFactor:double;
    MantissaExponent,PartExponent,Index:integer;
    OldFPUExceptionMask:TFPUExceptionMask;
    OldFPURoundingMode,NewFPURoundingMode:TFPURoundingMode;
    OldFPUPrecisionMode:TFPUPrecisionMode;
begin
 ResultCasted:=pointer(@result);
 ResultCasted^.Hi:=$7ff80000;
 ResultCasted^.Lo:=$00000000;
 i:=1;
 while (i<=length(StringValue)) and (BESENUnicodeIsStringWhiteSpace(word(widechar(StringValue[i])))) do begin
  inc(i);
 end;
 if (i<=length(StringValue)) and ((StringValue[i]='-') or (StringValue[i]='+')) then begin
  Negative:=StringValue[i]='-';
  inc(i);
 end else begin
  Negative:=false;
 end;
 if ((i+7)<=length(StringValue)) and ((StringValue[i]='I') and (StringValue[i+1]='n') and (StringValue[i+2]='f') and (StringValue[i+3]='i') and (StringValue[i+4]='n') and (StringValue[i+5]='i') and (StringValue[i+6]='t') and (StringValue[i+7]='y')) then begin
  if Negative then begin
   ResultCasted^.Hi:=$fff00000;
   ResultCasted^.Lo:=$00000000;
  end else begin
   ResultCasted^.Hi:=$7ff00000;
   ResultCasted^.Lo:=$00000000;
  end;
 end else if ((i+2)<=length(StringValue)) and ((StringValue[i]='N') and (StringValue[i+1]='a') and (StringValue[i+2]='N')) then begin
  ResultCasted^.Hi:=$7ff80000;
  ResultCasted^.Lo:=$00000000;
 end else begin
  FractionalDigits:=0;
  FractionalExponent:=0;
  MantissaSize:=0;
  Mantissa:=0;
  HasDigits:=false;
  ExponentSign:=1;
  Exponent:=0;
  while i<=length(StringValue) do begin
   case word(widechar(StringValue[i])) of
    ord('0'):begin
     HasDigits:=true;
     inc(i);
    end;
    else begin
     break;
    end;
   end;
  end;
  while i<=length(StringValue) do begin
   case word(widechar(StringValue[i])) of
    ord('0')..ord('9'):begin
     if MantissaSize<18 then begin
      Mantissa:=(Mantissa*10)+(word(widechar(StringValue[i]))-ord('0'));
     end;
     inc(MantissaSize);
     HasDigits:=true;
     inc(i);
    end;
    else begin
     break;
    end;
   end;
  end;
  if (i<=length(StringValue)) and (StringValue[i]='.') then begin
   inc(i);
   if (MantissaSize=0) and (Mantissa=0) then begin
    while i<=length(StringValue) do begin
     case word(widechar(StringValue[i])) of
      ord('0'):begin
       inc(FractionalExponent);
       HasDigits:=true;
       inc(i);
      end;
      else begin
       break;
      end;
     end;
    end;
   end;
   while i<=length(StringValue) do begin
    case word(widechar(StringValue[i])) of
     ord('0')..ord('9'):begin
      if MantissaSize<18 then begin
       Mantissa:=(Mantissa*10)+(word(widechar(StringValue[i]))-ord('0'));
       inc(MantissaSize);
       inc(FractionalDigits);
      end;
      HasDigits:=true;
      inc(i);
     end;
     else begin
      break;
     end;
    end;
   end;
  end;
  if HasDigits then begin
   if (i<=length(StringValue)) and ((StringValue[i]='e') or (StringValue[i]='E')) then begin
    inc(i);
    if (i<=length(StringValue)) and ((StringValue[i]='+') or (StringValue[i]='-')) then begin
     if StringValue[i]='-' then begin
      ExponentSign:=-1;
     end;
     inc(i);
    end;
    HasDigits:=false;
    while i<=length(StringValue) do begin
     case word(widechar(StringValue[i])) of
      ord('0')..ord('9'):begin
       Exponent:=(Exponent*10)+integer(word(widechar(StringValue[i]))-ord('0'));
       HasDigits:=true;
       inc(i);
      end;
      else begin
       break;
      end;
     end;
    end;
   end;
   if HasDigits then begin

    case RoundingMode of
     BESEN_ROUND_TO_NEAREST:begin
      NewFPURoundingMode:=rmNearest;
     end;
     BESEN_ROUND_TOWARD_ZERO:begin
      NewFPURoundingMode:=rmTruncate;
     end;
     BESEN_ROUND_UPWARD:begin
      NewFPURoundingMode:=rmUp;
     end;
     BESEN_ROUND_DOWNWARD:begin
      NewFPURoundingMode:=rmDown;
     end;
     else begin
      NewFPURoundingMode:=rmNearest;
     end;
    end;

    OldFPUExceptionMask:=GetExceptionMask;
    OldFPURoundingMode:=GetRoundMode;
    OldFPUPrecisionMode:=GetPrecisionMode;
    try
     if OldFPUExceptionMask<>BESENFPUExceptionMask then begin
      SetExceptionMask(BESENFPUExceptionMask);
     end;
     if OldFPURoundingMode<>NewFPURoundingMode then begin
      SetRoundMode(NewFPURoundingMode);
     end;
     if OldFPUPrecisionMode<>BESENFPUPrecisionMode then begin
      SetPrecisionMode(BESENFPUPrecisionMode);
     end;

     FloatValue:=Mantissa;

     MantissaExponent:=FractionalDigits+FractionalExponent;
     if MantissaSize>18 then begin
      dec(MantissaExponent,MantissaSize-18);
     end;

     if ExponentSign>0 then begin
      dec(Exponent,MantissaExponent);
     end else begin
      inc(Exponent,MantissaExponent);
     end;
     if Exponent<0 then begin
      ExponentSign:=-ExponentSign;
      Exponent:=-Exponent;
     end;

     while Exponent>0 do begin
      PartExponent:=Exponent;
      if PartExponent>MaxExponentOfTenMinusTwo then begin
       PartExponent:=MaxExponentOfTenMinusTwo;
      end;
      dec(Exponent,PartExponent);

      Index:=0;
      ExponentFactor:=1;
      while (PartExponent<>0) and (Index<length(PowersOfTen)) do begin
       if (PartExponent and 1)<>0 then begin
        ExponentFactor:=ExponentFactor*PowersOfTen[Index];
       end;
       inc(Index);
       PartExponent:=PartExponent shr 1;
      end;

      if ExponentSign>0 then begin
       FloatValue:=FloatValue*ExponentFactor;
      end else begin
       FloatValue:=FloatValue/ExponentFactor;
      end;
     end;

     ResultCasted^.Value:=FloatValue;
     if Negative then begin
      ResultCasted^.Hi:=ResultCasted^.Hi or $80000000;
     end;
    finally
     if OldFPUExceptionMask<>BESENFPUExceptionMask then begin
      SetExceptionMask(OldFPUExceptionMask);
     end;
     if OldFPURoundingMode<>NewFPURoundingMode then begin
      SetRoundMode(OldFPURoundingMode);
     end;
     if OldFPUPrecisionMode<>BESENFPUPrecisionMode then begin
      SetPrecisionMode(OldFPUPrecisionMode);
     end;
    end;
   end;
  end;
 end;
end;

function BESENCheckNumberString(const StringValue:TBESENString;var StartPosition,EndPosition:integer;IsParseFloat,AcceptHex,OnlyNumber,AcceptHexSign:boolean):integer;
var i,OldPosition:integer;
    Negative,HasDigits,HasSign:boolean;
begin
 result:=BESEN_CHECKNUMBERSTRING_FAIL;
 i:=1;
 while (i<=length(StringValue)) and (BESENUnicodeIsStringWhiteSpace(word(widechar(StringValue[i])))) do begin
  inc(i);
 end;
 StartPosition:=i;
 EndPosition:=i;
 if (i<=length(StringValue)) and ((StringValue[i]='-') or (StringValue[i]='+')) then begin
  Negative:=StringValue[i]='-';
  HasSign:=true;
  inc(i);
 end else begin
  Negative:=false;
  HasSign:=false;
 end;
 if i>length(StringValue) then begin
  result:=BESEN_CHECKNUMBERSTRING_EMPTY;
 end else if ((i+7)<=length(StringValue)) and ((StringValue[i]='I') and (StringValue[i+1]='n') and (StringValue[i+2]='f') and (StringValue[i+3]='i') and (StringValue[i+4]='n') and (StringValue[i+5]='i') and (StringValue[i+6]='t') and (StringValue[i+7]='y')) then begin
  EndPosition:=i+7;
  if Negative then begin
   result:=BESEN_CHECKNUMBERSTRING_INFNEG;
  end else begin
   result:=BESEN_CHECKNUMBERSTRING_INFPOS;
  end;
 end else if ((i+2)<=length(StringValue)) and ((StringValue[i]='N') and (StringValue[i+1]='a') and (StringValue[i+2]='N')) then begin
  EndPosition:=i+2;
  result:=BESEN_CHECKNUMBERSTRING_NAN;
 end else begin
  HasDigits:=false;
  if (i<=length(StringValue)) and (StringValue[i]='0') then begin
   inc(i);
   HasDigits:=true;
   if (i<=length(StringValue)) and ((StringValue[i]='x') or (StringValue[i]='X')) then begin
    inc(i);
    HasDigits:=false;
    while i<=length(StringValue) do begin
     case word(widechar(StringValue[i])) of
      ord('0')..ord('9'),ord('a')..ord('f'),ord('A')..ord('F'):begin
       HasDigits:=true;
       inc(i);
      end;
      else begin
       break;
      end;
     end;
    end;
    if HasDigits then begin
     EndPosition:=i-1;
     if AcceptHex and (AcceptHexSign or not HasSign) then begin
      result:=BESEN_CHECKNUMBERSTRING_VALID;
     end else begin
      result:=BESEN_CHECKNUMBERSTRING_FAIL;
     end;
    end;
    exit;
   end;
  end;
  while i<=length(StringValue) do begin
   case word(widechar(StringValue[i])) of
    ord('0')..ord('9'):begin
     HasDigits:=true;
     inc(i);
    end;
    else begin
     break;
    end;
   end;
  end;
  if (i<=length(StringValue)) and (StringValue[i]='.') then begin
   inc(i);
   while i<=length(StringValue) do begin
    case word(widechar(StringValue[i])) of
     ord('0')..ord('9'):begin
      HasDigits:=true;
      inc(i);
     end;
     else begin
      break;
     end;
    end;
   end;
  end;
  if HasDigits then begin
   OldPosition:=i;
   if (i<=length(StringValue)) and ((StringValue[i]='e') or (StringValue[i]='E')) then begin
    inc(i);
    if (i<=length(StringValue)) and ((StringValue[i]='+') or (StringValue[i]='-')) then begin
     inc(i);
    end;
    HasDigits:=false;
    while i<=length(StringValue) do begin
     case word(widechar(StringValue[i])) of
      ord('0')..ord('9'):begin
       HasDigits:=true;
       inc(i);
      end;
      else begin
       break;
      end;
     end;
    end;
    if IsParseFloat and not HasDigits then begin
     i:=OldPosition;
     HasDigits:=true;
    end;
   end;
   if HasDigits then begin
    EndPosition:=i-1;
    if OnlyNumber then begin
     while (i<=length(StringValue)) and (BESENUnicodeIsStringWhiteSpace(word(widechar(StringValue[i])))) do begin
      inc(i);
     end;
     if i>length(StringValue) then begin
      result:=BESEN_CHECKNUMBERSTRING_VALID;
     end else begin
      result:=BESEN_CHECKNUMBERSTRING_FAIL;
     end;
    end else begin
     result:=BESEN_CHECKNUMBERSTRING_VALID;
    end;
   end;
  end;
 end;
end;

function BESENStringToNumber(const StringValue:TBESENString;EmptyIsValid:TBESENBoolean=true;AcceptHex:TBESENBoolean=false;OnlyNumber:boolean=false;AcceptHexSign:boolean=true):TBESENNumber;
var StartPosition,EndPosition:integer;
begin
 case BESENCheckNumberString(StringValue,StartPosition,EndPosition,false,AcceptHex,OnlyNumber,AcceptHexSign) of
  BESEN_CHECKNUMBERSTRING_EMPTY:begin
   if EmptyIsValid then begin
    result:=0;
   end else begin
    result:=TBESENNumber(pointer(@BESENDoubleNaN)^);
   end;
  end;
  BESEN_CHECKNUMBERSTRING_VALID:begin
   result:=strtod(PAnsiChar(ansistring(copy(StringValue,StartPosition,(EndPosition-StartPosition)+1))),nil);
  end;
  BESEN_CHECKNUMBERSTRING_INFNEG:begin
   result:=TBESENNumber(pointer(@BESENDoubleInfNeg)^);
  end;
  BESEN_CHECKNUMBERSTRING_INFPOS:begin
   result:=TBESENNumber(pointer(@BESENDoubleInfPos)^);
  end;
  else begin
   // CHECKNUMBERSTRING_FAIL,CHECKNUMBERSTRING_NAN
   result:=TBESENNumber(pointer(@BESENDoubleNaN)^);
  end;
 end;
end;

function BESENNumberToString(const Value:TBESENNumber):TBESENString;
begin
 if BESENIsNaN(Value) then begin
  result:='NaN';
 end else if BESENIsZero(Value) then begin
  result:='0';
 end else if BESENIsNegInfinite(Value) then begin
  result:='-Infinity';
 end else if BESENIsNegative(Value) then begin
  result:='-'+BESENFloatToStr(-Value);
 end else if BESENIsInfinite(Value) then begin
  result:='Infinity';
 end else begin
  result:=dtostr(Value,DTOSTR_STANDARD,0);
 end;
end;

function BESENFloatToStr(const v:TBESENNumber):TBESENString;
begin
 if BESENIsNaN(v) then begin
  result:='NaN';
 end else if BESENIsZero(v) then begin
  result:='0';
 end else if BESENIsNegInfinite(v) then begin
  result:='-Infinity';
 end else if BESENIsNegative(v) then begin
  result:='-'+BESENFloatToStr(-v);
 end else if BESENIsInfinite(v) then begin
  result:='Infinity';
 end else begin
  result:=dtostr(v,DTOSTR_STANDARD,0);
 end;
end;

function BESENFloatToLocaleStr(const v:TBESENNumber):TBESENString;
var i:integer;
begin
 if BESENIsNaN(v) then begin
  result:='NaN';
 end else if BESENIsZero(v) then begin
  result:='0';
 end else if BESENIsNegInfinite(v) then begin
  result:='-Infinity';
 end else if BESENIsNegative(v) then begin
  result:='-'+BESENFloatToLocaleStr(-v);
 end else if BESENIsInfinite(v) then begin
  result:='Infinity';
 end else begin
  result:=dtostr(v,DTOSTR_STANDARD,0);
  for i:=1 to length(result) do begin
   case word(result[i]) of
    ord('.'):begin
     result[i]:=widechar(word(ord(BESENLocaleFormatSettings.DecimalSeparator)));
    end;
   end;
  end;
 end;
end;

function BESENSameValue(const A,B:TBESENNumber):boolean;
 function min(a,b:TBESENNumber):TBESENNumber;
 begin
  if a<b then begin
   result:=a;
  end else begin
   result:=b;
  end;
 end;
 function max(a,b:TBESENNumber):TBESENNumber;
 begin
  if a>b then begin
   result:=a;
  end else begin
   result:=b;
  end;
 end;
const FuzzFactor=1000;
      DoubleResolution=1E-15*FuzzFactor;
var Epsilon:TBESENNumber;
begin
 if int64(pointer(@A)^)=int64(pointer(@B)^) then begin
  result:=true;
 end else begin
  Epsilon:=max(min(abs(A),abs(B))*DoubleResolution,DoubleResolution);
  if A>B then begin
   result:=(A-B)<=Epsilon;
  end else begin
   result:=(B-A)<=Epsilon;
  end;
 end;
end;

function BESENSameNumber(const A,B:TBESENNumber):boolean;
 function min(a,b:TBESENNumber):TBESENNumber;
 begin
  if a<b then begin
   result:=a;
  end else begin
   result:=b;
  end;
 end;
 function max(a,b:TBESENNumber):TBESENNumber;
 begin
  if a>b then begin
   result:=a;
  end else begin
   result:=b;
  end;
 end;
const FuzzFactor=1000;
      DoubleResolution=1E-15*FuzzFactor;
var Epsilon:TBESENNumber;
begin
 if int64(pointer(@A)^)=int64(pointer(@B)^) then begin
  result:=true;
 end else if BESENIsNaN(A) and BESENIsNaN(B) then begin
  result:=true;
 end else if (abs(A)=0) and (abs(B)=0) then begin
  result:=(int64(pointer(@A)^) shr 63)=(int64(pointer(@B)^) shr 63);
 end else begin
  Epsilon:=max(min(abs(A),abs(B))*DoubleResolution,DoubleResolution);
  if A>B then begin
   result:=(A-B)<=Epsilon;
  end else begin
   result:=(B-A)<=Epsilon;
  end;
 end;
end;

function BESENFloor(FloatValue:TBESENNumber):TBESENNumber; {$ifdef caninline}inline;{$endif}
begin
 result:=System.int(FloatValue);
 if System.frac(FloatValue)<0 then begin
  result:=result-1;
 end;
end;

function BESENCeil(FloatValue:TBESENNumber):TBESENNumber; {$ifdef caninline}inline;{$endif}
begin
 result:=System.int(FloatValue);
 if System.frac(FloatValue)>0 then begin
  result:=result+1;
 end;
end;

function BESENToInt(v:TBESENNumber):TBESENINT64;
begin
 result:=trunc(v);
end;

function BESENToInt32(v:TBESENNumber):TBESENINT32;
var Sign:longword;
begin
 if BESENIsNaN(v) or BESENIsInfinite(v) or BESENIsZero(v) then begin
  v:=0.0;
 end else begin
  Sign:=PBESENDoubleHiLo(@v)^.Hi and $80000000;
  PBESENDoubleHiLo(@v)^.Hi:=PBESENDoubleHiLo(@v)^.Hi and $7fffffff;
  v:=BESENFloor(v);
  PBESENDoubleHiLo(@v)^.Hi:=PBESENDoubleHiLo(@v)^.Hi or Sign;
  v:=BESENModulo(System.int(v),4294967296.0);
  if (PBESENDoubleHiLo(@v)^.Hi and $80000000)<>0 then begin
   v:=v+4294967296.0;
  end;
  if v>=2147483648.0 then begin
   v:=v-4294967296.0;
  end;
 end;
 result:=trunc(v);
end;

function BESENToUInt32(v:TBESENNumber):TBESENUINT32;
var Sign:longword;
begin
 if BESENIsNaN(v) or BESENIsInfinite(v) or BESENIsZero(v) then begin
  v:=0.0;
 end else begin
  Sign:=PBESENDoubleHiLo(@v)^.Hi and $80000000;
  PBESENDoubleHiLo(@v)^.Hi:=PBESENDoubleHiLo(@v)^.Hi and $7fffffff;
  v:=BESENFloor(v);
  PBESENDoubleHiLo(@v)^.Hi:=PBESENDoubleHiLo(@v)^.Hi or Sign;
  v:=BESENModulo(System.int(v),4294967296.0);
  if (PBESENDoubleHiLo(@v)^.Hi and $80000000)<>0 then begin
   v:=v+4294967296.0;
  end;
 end;
 result:=trunc(v);
end;

function BESENToInt16(v:TBESENNumber):TBESENINT32;
var Sign:longword;
begin
 if BESENIsNaN(v) or BESENIsInfinite(v) or BESENIsZero(v) then begin
  v:=0.0;
 end else begin
  Sign:=PBESENDoubleHiLo(@v)^.Hi and $80000000;
  PBESENDoubleHiLo(@v)^.Hi:=PBESENDoubleHiLo(@v)^.Hi and $7fffffff;
  v:=BESENFloor(v);
  PBESENDoubleHiLo(@v)^.Hi:=PBESENDoubleHiLo(@v)^.Hi or Sign;
  v:=BESENModulo(System.int(v),65536.0);
  if (PBESENDoubleHiLo(@v)^.Hi and $80000000)<>0 then begin
   v:=v+65536.0;
  end;
  if v>=32768.0 then begin
   v:=v-65536.0;
  end;
 end;
 result:=trunc(v);
end;

function BESENToUInt16(v:TBESENNumber):TBESENUINT32;
var Sign:longword;
begin
 if BESENIsNaN(v) or BESENIsInfinite(v) or BESENIsZero(v) then begin
  v:=0.0;
 end else begin
  Sign:=PBESENDoubleHiLo(@v)^.Hi and $80000000;
  PBESENDoubleHiLo(@v)^.Hi:=PBESENDoubleHiLo(@v)^.Hi and $7fffffff;
  v:=BESENFloor(v);
  PBESENDoubleHiLo(@v)^.Hi:=PBESENDoubleHiLo(@v)^.Hi or Sign;
  v:=BESENModulo(System.int(v),65536.0);
  if (PBESENDoubleHiLo(@v)^.Hi and $80000000)<>0 then begin
   v:=v+65536.0;
  end;
 end;
 result:=trunc(v);
end;

function BESENToIntFast(v:PBESENNumber):TBESENINT64; {$ifdef UseRegister}register;{$endif}{$ifdef caninline}inline;{$endif}
begin
 result:=trunc(v^);
end;

function BESENToInt32Fast(v:PBESENNumber):TBESENINT32; {$ifdef UseRegister}register;{$endif}{$ifdef caninline}inline;{$endif}
begin
 if not ((v^>=2147483648.0) and (v^<2147483648.0)) then begin
  result:=BESENToInt32(v^);
 end else begin
  result:=trunc(v^);
 end;
end;

function BESENToUInt32Fast(v:PBESENNumber):TBESENUINT32; {$ifdef UseRegister}register;{$endif}{$ifdef caninline}inline;{$endif}
begin
 if not ((v^>=0.0) and (v^<4294967296.0)) then begin
  result:=BESENToUInt32(v^);
 end else begin
  result:=int64(trunc(v^));
 end;
end;

function BESENToInt16Fast(v:PBESENNumber):TBESENINT32; {$ifdef UseRegister}register;{$endif}{$ifdef caninline}inline;{$endif}
begin
 if not ((v^>=-32768.0) and (v^<32768.0)) then begin
  result:=BESENToInt16(v^);
 end else begin
  result:=trunc(v^);
 end;
end;

function BESENToUInt16Fast(v:PBESENNumber):TBESENUINT32; {$ifdef UseRegister}register;{$endif}{$ifdef caninline}inline;{$endif}
begin
 if not ((v^>=0.0) and (v^<65536.0)) then begin
  result:=BESENToUInt16(v^);
 end else begin
  result:=trunc(v^);
 end;
end;

function BESENIntLog2(x:longword):longword; {$ifdef cpu386}assembler; register;
asm
 test eax,eax
 jz @Done
 bsr eax,eax
 @Done:
end;
{$else}
begin
 x:=x or (x shr 1);
 x:=x or (x shr 2);
 x:=x or (x shr 4);
 x:=x or (x shr 8);
 x:=x or (x shr 16);
 x:=x shr 1;
 x:=x-((x shr 1) and $55555555);
 x:=((x shr 2) and $33333333)+(x and $33333333);
 x:=((x shr 4)+x) and $0f0f0f0f;
 x:=x+(x shr 8);
 x:=x+(x shr 16);
 result:=x and $3f;
end;
{$endif}

{$ifdef cpu64}
function BESENIsNaN(const AValue:TBESENNumber):boolean; {$ifdef caninline}inline;{$endif}
begin
 result:=((PBESENINT64(@AValue)^ and $7ff0000000000000)=$7ff0000000000000) and ((PBESENINT64(@AValue)^ and $000fffffffffffff)<>$0000000000000000);
end;

function BESENIsInfinite(const AValue:TBESENNumber):boolean; {$ifdef caninline}inline;{$endif}
begin
 result:=(PBESENINT64(@AValue)^ and $7fffffffffffffff)=$7ff0000000000000;
end;

function BESENIsFinite(const AValue:TBESENNumber):boolean; {$ifdef caninline}inline;{$endif}
begin
 result:=(PBESENINT64(@AValue)^ and $7ff0000000000000)<>$7ff0000000000000;
end;

function BESENIsPosInfinite(const AValue:TBESENNumber):boolean; {$ifdef caninline}inline;{$endif}
begin
 result:=PBESENINT64(@AValue)^=int64($7ff0000000000000);
end;

function BESENIsNegInfinite(const AValue:TBESENNumber):boolean; {$ifdef caninline}inline;{$endif}
begin
{$ifdef fpc}
 result:=qword(pointer(@AValue)^)=qword($fff0000000000000);
{$else}
 result:=PBESENINT64(@AValue)^=int64($fff0000000000000);
{$endif}
end;

function BESENIsPosZero(const AValue:TBESENNumber):boolean; {$ifdef caninline}inline;{$endif}
begin
 result:=PBESENINT64(@AValue)^=int64($0000000000000000);
end;

function BESENIsNegZero(const AValue:TBESENNumber):boolean; {$ifdef caninline}inline;{$endif}
begin
{$ifdef fpc}
 result:=qword(pointer(@AValue)^)=qword($8000000000000000);
{$else}
 result:=PBESENINT64(@AValue)^=int64($8000000000000000);
{$endif}
end;

function BESENIsZero(const AValue:TBESENNumber):boolean; {$ifdef caninline}inline;{$endif}
begin
{$ifdef fpc}
 result:=(qword(pointer(@AValue)^) and qword($7fffffffffffffff))=qword($0000000000000000);
{$else}
 result:=(PBESENINT64(@AValue)^ and int64($7fffffffffffffff))=int64($0000000000000000);
{$endif}
end;

function BESENIsNegative(const AValue:TBESENNumber):boolean; {$ifdef caninline}inline;{$endif}
begin
{$ifdef fpc}
 result:=(qword(pointer(@AValue)^) and qword($8000000000000000))<>0;
{$else}
 result:=(PBESENINT64(@AValue)^ shr 63)<>0;
{$endif}
end;
{$else}
{$ifdef TrickyNumberChecks}
function BESENIsNaN(const AValue:TBESENNumber):boolean; {$ifdef caninline}inline;{$endif}
var l:longword;
begin
 l:=PBESENDoubleHiLo(@AValue)^.Lo;
 result:=(longword($7ff00000-longword(longword(PBESENDoubleHiLo(@AValue)^.Hi and $7fffffff) or ((l or (-l)) shr 31))) shr 31)<>0;
end;

function BESENIsInfinite(const AValue:TBESENNumber):boolean; {$ifdef caninline}inline;{$endif}
begin
 result:=longword((longword(PBESENDoubleHiLo(@AValue)^.Hi and $7fffffff) xor $7ff00000) or PBESENDoubleHiLo(@AValue)^.Lo)=0;
end;

function BESENIsFinite(const AValue:TBESENNumber):boolean; {$ifdef caninline}inline;{$endif}
begin
 result:=(longword((PBESENDoubleHiLo(@AValue)^.Hi and $7fffffff)-$7ff00000) shr 31)<>0;
end;

function BESENIsPosInfinite(const AValue:TBESENNumber):boolean; {$ifdef caninline}inline;{$endif}
var h:longword;
begin
 h:=PBESENDoubleHiLo(@AValue)^.Hi;
 result:=longword(((longword(h and $7fffffff) xor $7ff00000) or PBESENDoubleHiLo(@AValue)^.Lo) or longword(h shr 31))=0;
end;

function BESENIsNegInfinite(const AValue:TBESENNumber):boolean; {$ifdef caninline}inline;{$endif}
var h:longword;
begin
 h:=PBESENDoubleHiLo(@AValue)^.Hi;
 result:=longword(((longword(h and $7fffffff) xor $7ff00000) or PBESENDoubleHiLo(@AValue)^.Lo) or longword(longword(not h) shr 31))=0;
end;

function BESENIsPosZero(const AValue:TBESENNumber):boolean; {$ifdef caninline}inline;{$endif}
var h:longword;
begin
 h:=PBESENDoubleHiLo(@AValue)^.Hi;
 result:=longword(longword(longword(h and $7fffffff) or PBESENDoubleHiLo(@AValue)^.Lo) or longword(h shr 31))=0;
end;

function BESENIsNegZero(const AValue:TBESENNumber):boolean; {$ifdef caninline}inline;{$endif}
var h:longword;
begin
 h:=PBESENDoubleHiLo(@AValue)^.Hi;
 result:=longword(longword(longword(h and $7fffffff) or PBESENDoubleHiLo(@AValue)^.Lo) or longword(longword(not h) shr 31))=0;
end;

function BESENIsZero(const AValue:TBESENNumber):boolean; {$ifdef caninline}inline;{$endif}
begin
 result:=longword(longword(PBESENDoubleHiLo(@AValue)^.Hi and $7fffffff) or PBESENDoubleHiLo(@AValue)^.Lo)=0;
end;

function BESENIsNegative(const AValue:TBESENNumber):boolean; {$ifdef caninline}inline;{$endif}
begin
 result:=longword(PBESENDoubleHiLo(@AValue)^.Hi and longword($80000000))<>0;
end;
{$else}
function BESENIsNaN(const AValue:TBESENNumber):boolean; {$ifdef caninline}inline;{$endif}
begin
 result:=((PBESENDoubleHiLo(@AValue)^.Hi and $7ff00000)=$7ff00000) and (((PBESENDoubleHiLo(@AValue)^.Hi and $000fffff) or PBESENDoubleHiLo(@AValue)^.Lo)<>0);
end;

function BESENIsInfinite(const AValue:TBESENNumber):boolean; {$ifdef caninline}inline;{$endif}
begin
 result:=((PBESENDoubleHiLo(@AValue)^.Hi and $7fffffff)=$7ff00000) and (PBESENDoubleHiLo(@AValue)^.Lo=0);
end;

function BESENIsFinite(const AValue:TBESENNumber):boolean; {$ifdef caninline}inline;{$endif}
begin
 result:=(PBESENDoubleHiLo(@AValue)^.Hi and $7ff00000)<>$7ff00000;
end;

function BESENIsPosInfinite(const AValue:TBESENNumber):boolean; {$ifdef caninline}inline;{$endif}
begin
 result:=(PBESENDoubleHiLo(@AValue)^.Hi=$7ff00000) and (PBESENDoubleHiLo(@AValue)^.Lo=0);
end;

function BESENIsNegInfinite(const AValue:TBESENNumber):boolean; {$ifdef caninline}inline;{$endif}
begin
 result:=(PBESENDoubleHiLo(@AValue)^.Hi=$fff00000) and (PBESENDoubleHiLo(@AValue)^.Lo=0);
end;

function BESENIsPosZero(const AValue:TBESENNumber):boolean; {$ifdef caninline}inline;{$endif}
begin
 result:=(PBESENDoubleHiLo(@AValue)^.Hi or PBESENDoubleHiLo(@AValue)^.Lo)=0;
end;

function BESENIsNegZero(const AValue:TBESENNumber):boolean; {$ifdef caninline}inline;{$endif}
begin
 result:=(PBESENDoubleHiLo(@AValue)^.Hi=$80000000) and (PBESENDoubleHiLo(@AValue)^.Lo=0);
end;

function BESENIsZero(const AValue:TBESENNumber):boolean; {$ifdef caninline}inline;{$endif}
begin
 result:=((PBESENDoubleHiLo(@AValue)^.Hi and $7fffffff) or PBESENDoubleHiLo(@AValue)^.Lo)=0;
end;

function BESENIsNegative(const AValue:TBESENNumber):boolean; {$ifdef caninline}inline;{$endif}
begin
 result:=(PBESENDoubleHiLo(@AValue)^.Hi and $80000000)<>0;
end;
{$endif}
{$endif}

function BESENIsSameValue(const a,b:TBESENNumber):boolean; {$ifdef caninline}inline;{$endif}
begin
{$ifdef fpc}
 result:=qword(pointer(@a)^)=qword(pointer(@b)^);
{$else}
 result:=PBESENINT64(@a)^=PBESENINT64(@b)^;
{$endif}
end;

function BESENModulo(x,y:double):double;{$ifdef cpu386}stdcall; assembler;
asm
 fld qword ptr y
 fld qword ptr x
 @Repeat:
  fprem
  fstsw ax
  sahf
  jp @Repeat
 fstp st(1)
end;
{$else}
begin
 result:=x-(BESENFloor(x/y)*y);
end;
{$endif}

function BESENModuloPos(x,y:double):double;
begin
 result:=BESENModulo(x,y);
 if BESENIsNegative(result) then begin
  result:=result+y;
 end;
end;

procedure InitBESEN;
begin
 InitBESENNumberTables;
end;

procedure DoneBESEN;
begin
end;

initialization
 InitBESEN;
finalization
 DoneBESEN;
end.
