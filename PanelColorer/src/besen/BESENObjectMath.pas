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
unit BESENObjectMath;
{$i BESEN.inc}

interface

uses Math,BESENConstants,BESENTypes,BESENObject,BESENValue,BESENObjectPropertyDescriptor;

type TBESENObjectMath=class(TBESENObject)
      public
       constructor Create(AInstance:TObject;APrototype:TBESENObject=nil;AHasPrototypeProperty:longbool=false); overload; override;
       destructor Destroy; override;
       procedure NativeAbs(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeACos(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeASin(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeATan(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeATan2(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeCeil(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeCos(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeExp(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeFloor(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeLog(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeMax(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeMin(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativePow(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeRandom(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeRound(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeSin(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeSqrt(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeTan(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
     end;

implementation

uses BESEN,BESENNumberUtils;

constructor TBESENObjectMath.Create(AInstance:TObject;APrototype:TBESENObject=nil;AHasPrototypeProperty:longbool=false);
var v:TBESENValue;
begin
 inherited Create(AInstance,APrototype,AHasPrototypeProperty);
 ObjectClassName:='Math';

 v:=BESENEmptyValue;
 v:=BESENNumberValue(2.7182818284590452354);
 OverwriteData('E',v,[]);

 v:=BESENNumberValue(2.7182818284590452354);
 OverwriteData('E',v,[]);

 v:=BESENNumberValue(2.302585092994046);
 OverwriteData('LN10',v,[]);

 v:=BESENNumberValue(0.6931471805599453);
 OverwriteData('LN2',v,[]);

 v:=BESENNumberValue(1.4426950408889634);
 OverwriteData('LOG2E',v,[]);

 v:=BESENNumberValue(0.4342944819032518);
 OverwriteData('LOG10E',v,[]);

 v:=BESENNumberValue(3.1415926535897932);
 OverwriteData('PI',v,[]);

 v:=BESENNumberValue(0.7071067811865476);
 OverwriteData('SQRT1_2',v,[]);

 v:=BESENNumberValue(1.4142135623730951);
 OverwriteData('SQRT2',v,[]);

 RegisterNativeFunction('abs',NativeAbs,1,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('acos',NativeACos,1,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('asin',NativeASin,1,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('atan',NativeATan,1,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('atan2',NativeATan2,2,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('ceil',NativeCeil,1,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('cos',NativeCos,1,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('exp',NativeExp,1,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('floor',NativeFloor,1,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('log',NativeLog,1,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('max',NativeMax,2,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('min',NativeMin,2,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('pow',NativePow,2,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('random',NativeRandom,0,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('round',NativeRound,1,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('sin',NativeSin,1,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('sqrt',NativeSqrt,1,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('tan',NativeTan,1,[bopaWRITABLE,bopaCONFIGURABLE],false);
end;

destructor TBESENObjectMath.Destroy;
begin
 inherited Destroy;
end;

procedure TBESENObjectMath.NativeAbs(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 if CountArguments=0 then begin
  ResultValue.ValueType:=bvtNUMBER;
  ResultValue.Num:=TBESENNumber(pointer(@BESENDoubleNaN)^);
 end else begin
  TBESEN(Instance).ToNumberValue(Arguments^[0]^,ResultValue);
  if not BESENIsNaN(ResultValue.Num) then begin
   PBESENDoubleHiLo(@ResultValue.Num)^.Hi:=PBESENDoubleHiLo(@ResultValue.Num)^.Hi and $7fffffff;
  end;
 end;
end;

procedure TBESENObjectMath.NativeACos(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 if CountArguments=0 then begin
  ResultValue.ValueType:=bvtNUMBER;
  ResultValue.Num:=TBESENNumber(pointer(@BESENDoubleNaN)^);
 end else begin
  TBESEN(Instance).ToNumberValue(Arguments^[0]^,ResultValue);
  if not BESENIsNaN(ResultValue.Num) then begin
   if (ResultValue.Num<-1) or (ResultValue.Num>1) then begin
    int64(pointer(@ResultValue.Num)^):=int64(pointer(@BESENDoubleNaN)^);
   end else if ResultValue.Num=1 then begin
    ResultValue.Num:=0;
   end else begin
    ResultValue.Num:=arccos(ResultValue.Num);
   end;
  end;
 end;
end;

procedure TBESENObjectMath.NativeASin(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 if CountArguments=0 then begin
  ResultValue.ValueType:=bvtNUMBER;
  ResultValue.Num:=TBESENNumber(pointer(@BESENDoubleNaN)^);
 end else begin
  TBESEN(Instance).ToNumberValue(Arguments^[0]^,ResultValue);
  if not BESENIsNaN(ResultValue.Num) then begin
   if (ResultValue.Num<-1) or (ResultValue.Num>1) then begin
    int64(pointer(@ResultValue.Num)^):=int64(pointer(@BESENDoubleNaN)^);
   end else if not BESENIsZero(ResultValue.Num) then begin
    ResultValue.Num:=arcsin(ResultValue.Num);
   end;
  end;
 end;
end;

procedure TBESENObjectMath.NativeATan(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 if CountArguments=0 then begin
  ResultValue.ValueType:=bvtNUMBER;
  ResultValue.Num:=TBESENNumber(pointer(@BESENDoubleNaN)^);
 end else begin
  TBESEN(Instance).ToNumberValue(Arguments^[0]^,ResultValue);
  if not (BESENIsNaN(ResultValue.Num) or BESENIsZero(ResultValue.Num)) then begin
   if BESENIsPosInfinite(ResultValue.Num) then begin
    ResultValue.Num:=PI*0.5;
   end else if BESENIsNegInfinite(ResultValue.Num) then begin
    ResultValue.Num:=-(PI*0.5);
   end else begin
    ResultValue.Num:=arctan(ResultValue.Num);
   end;
  end;
 end;
end;

procedure TBESENObjectMath.NativeATan2(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
var x,y:TBESENNumber;
begin
 ResultValue.ValueType:=bvtNUMBER;
 if CountArguments<2 then begin
  ResultValue.Num:=TBESENNumber(pointer(@BESENDoubleNaN)^);
 end else begin
  y:=TBESEN(Instance).ToNum(Arguments^[0]^);
  x:=TBESEN(Instance).ToNum(Arguments^[1]^);
  if BESENIsNaN(y) or BESENIsNaN(x) then begin
   ResultValue.Num:=TBESENNumber(pointer(@BESENDoubleNaN)^);
  end else if (y>0) and BESENIsZero(x) then begin
   ResultValue.Num:=PI*0.5;
  end else if BESENIsPosZero(y) and ((x>0) or BESENIsPosZero(x)) then begin
   ResultValue.Num:=0;
  end else if BESENIsPosZero(y) and (BESENIsNegZero(x) or (x<0)) then begin
   ResultValue.Num:=PI;
  end else if BESENIsNegZero(y) and ((BESENIsFinite(x) and not BESENIsNegative(x)) or BESENIsPosZero(x)) then begin
   ResultValue.Num:=0;
   PBESENDoubleHiLo(@ResultValue.Num)^.Hi:=PBESENDoubleHiLo(@ResultValue.Num)^.Hi or $80000000;
  end else if BESENIsNegZero(y) and (BESENIsNegZero(x) or (x<0)) then begin
   ResultValue.Num:=-PI;
  end else if (y<0) and BESENIsZero(x) then begin
   ResultValue.Num:=-PI*0.5;
  end else if ((y>0) and BESENIsFinite(y)) and BESENIsPosInfinite(x) then begin
   ResultValue.Num:=0;
  end else if ((y>0) and BESENIsFinite(y)) and BESENIsNegInfinite(x) then begin
   ResultValue.Num:=PI;
  end else if ((y<0) and BESENIsFinite(y)) and BESENIsPosInfinite(x) then begin
   ResultValue.Num:=0;
   PBESENDoubleHiLo(@ResultValue.Num)^.Hi:=PBESENDoubleHiLo(@ResultValue.Num)^.Hi or $80000000;
  end else if ((y<0) and BESENIsFinite(y)) and BESENIsNegInfinite(x) then begin
   ResultValue.Num:=-PI;
  end else if BESENIsPosInfinite(y) and BESENIsFinite(x) then begin
   ResultValue.Num:=PI*0.5;
  end else if BESENIsNegInfinite(y) and BESENIsFinite(x) then begin
   ResultValue.Num:=-(PI*0.5);
  end else if BESENIsPosInfinite(y) and BESENIsPosInfinite(x) then begin
   ResultValue.Num:=PI*0.25;
  end else if BESENIsPosInfinite(y) and BESENIsNegInfinite(x) then begin
   ResultValue.Num:=(3*PI)*0.25;
  end else if BESENIsNegInfinite(y) and BESENIsPosInfinite(x) then begin
   ResultValue.Num:=-(PI*0.25);
  end else if BESENIsNegInfinite(y) and BESENIsNegInfinite(x) then begin
   ResultValue.Num:=-((3*PI)*0.25);
  end else begin
   ResultValue.Num:=arctan2(y,x);
  end;
 end;
end;

procedure TBESENObjectMath.NativeCeil(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 ResultValue.ValueType:=bvtNUMBER;
 if CountArguments=0 then begin
  ResultValue.Num:=TBESENNumber(pointer(@BESENDoubleNaN)^);
 end else begin
  TBESEN(Instance).ToNumberValue(Arguments^[0]^,ResultValue);
  if not (BESENIsNaN(ResultValue.Num) or BESENIsInfinite(ResultValue.Num) or BESENIsZero(ResultValue.Num)) then begin
   if (ResultValue.Num<0) and (ResultValue.Num>(-1)) then begin
    ResultValue.Num:=0;
    PBESENDoubleHiLo(@ResultValue.Num)^.Hi:=PBESENDoubleHiLo(@ResultValue.Num)^.Hi or $80000000;
   end else begin
    ResultValue.Num:=BESENCeil(ResultValue.Num);
   end;
  end;
 end;
end;

procedure TBESENObjectMath.NativeCos(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 ResultValue.ValueType:=bvtNUMBER;
 if CountArguments=0 then begin
  ResultValue.Num:=TBESENNumber(pointer(@BESENDoubleNaN)^);
 end else begin
  TBESEN(Instance).ToNumberValue(Arguments^[0]^,ResultValue);
  if BESENIsNaN(ResultValue.Num) or BESENIsInfinite(ResultValue.Num) then begin
   ResultValue.Num:=TBESENNumber(pointer(@BESENDoubleNaN)^);
  end else if BESENIsZero(ResultValue.Num) then begin
   ResultValue.Num:=1;
  end else begin
   ResultValue.Num:=cos(ResultValue.Num);
  end;
 end;
end;

procedure TBESENObjectMath.NativeExp(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 ResultValue.ValueType:=bvtNUMBER;
 if CountArguments=0 then begin
  ResultValue.Num:=TBESENNumber(pointer(@BESENDoubleNaN)^);
 end else begin
  TBESEN(Instance).ToNumberValue(Arguments^[0]^,ResultValue);
  if BESENIsNaN(ResultValue.Num) then begin
   ResultValue.Num:=TBESENNumber(pointer(@BESENDoubleNaN)^);
  end else if BESENIsZero(ResultValue.Num) then begin
   ResultValue.Num:=1;
  end else if BESENIsPosInfinite(ResultValue.Num) then begin
   ResultValue.Num:=TBESENNumber(pointer(@BESENDoubleInfPos)^);
  end else if BESENIsNegInfinite(ResultValue.Num) then begin
   ResultValue.Num:=0;
  end else begin
   ResultValue.Num:=exp(ResultValue.Num);
  end;
 end;
end;

procedure TBESENObjectMath.NativeFloor(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 ResultValue.ValueType:=bvtNUMBER;
 if CountArguments=0 then begin
  ResultValue.Num:=TBESENNumber(pointer(@BESENDoubleNaN)^);
 end else begin
  TBESEN(Instance).ToNumberValue(Arguments^[0]^,ResultValue);
  if not (BESENIsNaN(ResultValue.Num) or BESENIsInfinite(ResultValue.Num) or BESENIsZero(ResultValue.Num)) then begin
   if (ResultValue.Num>0) and (ResultValue.Num<1) then begin
    ResultValue.Num:=0;
   end else begin
    ResultValue.Num:=BESENFloor(ResultValue.Num);
   end;
  end;
 end;
end;

procedure TBESENObjectMath.NativeLog(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 ResultValue.ValueType:=bvtNUMBER;
 if CountArguments=0 then begin
  ResultValue.Num:=TBESENNumber(pointer(@BESENDoubleNaN)^);
 end else begin
  TBESEN(Instance).ToNumberValue(Arguments^[0]^,ResultValue);
  if BESENIsNaN(ResultValue.Num) or (ResultValue.Num<0) then begin
   ResultValue.Num:=TBESENNumber(pointer(@BESENDoubleNaN)^);
  end else if BESENIsZero(ResultValue.Num) then begin
   ResultValue.Num:=TBESENNumber(pointer(@BESENDoubleInfNeg)^);
  end else if ResultValue.Num=1 then begin
   ResultValue.Num:=0;
  end else if BESENIsPosInfinite(ResultValue.Num) then begin
   ResultValue.Num:=TBESENNumber(pointer(@BESENDoubleInfPos)^);
  end else begin
   ResultValue.Num:=ln(ResultValue.Num);
  end;
 end;
end;

procedure TBESENObjectMath.NativeMax(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
var x,n:TBESENNumber;
    i:integer;
begin
 ResultValue.ValueType:=bvtNUMBER;
 if CountArguments=0 then begin
  ResultValue.Num:=TBESENNumber(pointer(@BESENDoubleInfNeg)^);
 end else begin
  n:=TBESENNumber(pointer(@BESENDoubleInfNeg)^);
  for i:=0 to CountArguments-1 do begin
   x:=TBESEN(Instance).ToNum(Arguments^[i]^);
   if BESENIsNaN(x) then begin
    n:=TBESENNumber(pointer(@BESENDoubleNaN)^);
    break;
   end;
   if (i=0) or ((x>n) or (BESENIsPosInfinite(x) or (BESENIsPosZero(x) and BESENIsNegZero(n)))) then begin
    n:=x;
   end;
  end;
  ResultValue.Num:=n;
 end;
end;

procedure TBESENObjectMath.NativeMin(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
var x,n:TBESENNumber;
    i:integer;
begin
 ResultValue.ValueType:=bvtNUMBER;
 if CountArguments=0 then begin
  ResultValue.Num:=TBESENNumber(pointer(@BESENDoubleInfPos)^);
 end else begin
  n:=TBESENNumber(pointer(@BESENDoubleInfPos)^);
  for i:=0 to CountArguments-1 do begin
   x:=TBESEN(Instance).ToNum(Arguments^[i]^);
   if BESENIsNaN(x) then begin
    n:=TBESENNumber(pointer(@BESENDoubleNaN)^);
    break;
   end;
   if (i=0) or ((x<n) or (BESENIsNegInfinite(x) or (BESENIsNegZero(x) and BESENIsPosZero(n)))) then begin
    n:=x;
   end;
  end;
  ResultValue.Num:=n;
 end;
end;

procedure TBESENObjectMath.NativePow(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
var x,y:TBESENNumber;
begin
 ResultValue.ValueType:=bvtNUMBER;
 if CountArguments<2 then begin
  ResultValue.Num:=TBESENNumber(pointer(@BESENDoubleNaN)^);
 end else begin
  x:=TBESEN(Instance).ToNum(Arguments^[0]^);
  y:=TBESEN(Instance).ToNum(Arguments^[1]^);
  if BESENIsNaN(y) then begin
   ResultValue.Num:=TBESENNumber(pointer(@BESENDoubleNaN)^);
  end else if BESENIsZero(y) then begin
   ResultValue.Num:=1;
  end else if BESENIsNaN(x) and not BESENIsZero(y) then begin
   ResultValue.Num:=TBESENNumber(pointer(@BESENDoubleNaN)^);
  end else if (abs(x)>1) and BESENIsPosInfinite(y) then begin
   ResultValue.Num:=TBESENNumber(pointer(@BESENDoubleInfPos)^);
  end else if (abs(x)>1) and BESENIsNegInfinite(y) then begin
   ResultValue.Num:=0;
  end else if (abs(x)=1) and BESENIsInfinite(y) then begin
   ResultValue.Num:=TBESENNumber(pointer(@BESENDoubleNaN)^);
  end else if (abs(x)<1) and BESENIsPosInfinite(y) then begin
   ResultValue.Num:=0;
  end else if (abs(x)<1) and BESENIsNegInfinite(y) then begin
   ResultValue.Num:=TBESENNumber(pointer(@BESENDoubleInfPos)^);
  end else if BESENIsPosInfinite(x) and (y>0) then begin
   ResultValue.Num:=TBESENNumber(pointer(@BESENDoubleInfPos)^);
  end else if BESENIsPosInfinite(x) and (y<0) then begin
   ResultValue.Num:=0;
  end else if BESENIsNegInfinite(x) and (y>0) then begin
   y:=abs(BESENModulo(y,2.0));
   if y=1 then begin
    ResultValue.Num:=TBESENNumber(pointer(@BESENDoubleInfNeg)^);
   end else begin
    ResultValue.Num:=TBESENNumber(pointer(@BESENDoubleInfPos)^);
   end;
  end else if BESENIsNegInfinite(x) and (y<0) then begin
   y:=abs(BESENModulo(-y,2.0));
   ResultValue.Num:=0;
   if y=1 then begin
    PBESENDoubleHiLo(@ResultValue.Num)^.Hi:=PBESENDoubleHiLo(@ResultValue.Num)^.Hi or $80000000;
   end;
  end else if BESENIsPosZero(x) and (y>0) then begin
   ResultValue.Num:=0;
  end else if BESENIsPosZero(x) and (y<0) then begin
   ResultValue.Num:=TBESENNumber(pointer(@BESENDoubleInfPos)^);
  end else if BESENIsNegZero(x) and (y>0) then begin
   y:=abs(BESENModulo(-y,2.0));
   ResultValue.Num:=0;
   if y=1 then begin
    PBESENDoubleHiLo(@ResultValue.Num)^.Hi:=PBESENDoubleHiLo(@ResultValue.Num)^.Hi or $80000000;
   end;
  end else if BESENIsNegZero(x) and (y<0) then begin
   y:=abs(BESENModulo(y,2.0));
   if y=1 then begin
    ResultValue.Num:=TBESENNumber(pointer(@BESENDoubleInfNeg)^);
   end else begin
    ResultValue.Num:=TBESENNumber(pointer(@BESENDoubleInfPos)^);
   end;
  end else if ((x<0) and BESENIsFinite(x)) and (BESENIsFinite(y) and not BESENIsZero(frac(y))) then begin
   ResultValue.Num:=TBESENNumber(pointer(@BESENDoubleNaN)^);
  end else begin
   ResultValue.Num:=power(x,y);
  end;
 end;
end;

procedure TBESENObjectMath.NativeRandom(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 ResultValue.ValueType:=bvtNUMBER;
 ResultValue.Num:=TBESEN(Instance).RandomGenerator.GetNumber;
end;

procedure TBESENObjectMath.NativeRound(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 ResultValue.ValueType:=bvtNUMBER;
 if CountArguments=0 then begin
  ResultValue.Num:=TBESENNumber(pointer(@BESENDoubleNaN)^);
 end else begin
  TBESEN(Instance).ToNumberValue(Arguments^[0]^,ResultValue);
  if BESENIsNaN(ResultValue.Num) then begin
   ResultValue.Num:=TBESENNumber(pointer(@BESENDoubleNaN)^);
  end else if not (BESENIsZero(ResultValue.Num) or BESENIsInfinite(ResultValue.Num)) then begin
   if (ResultValue.Num>=-0.5) and (ResultValue.Num<0) then begin
    ResultValue.Num:=0;
    PBESENDoubleHiLo(@ResultValue.Num)^.Hi:=PBESENDoubleHiLo(@ResultValue.Num)^.Hi or $80000000;
   end else if (ResultValue.Num>0) and (ResultValue.Num<0.5) then begin
    ResultValue.Num:=0;
   end else begin
    ResultValue.Num:=BESENFloor(ResultValue.Num+0.5);
   end;
  end;
 end;
end;

procedure TBESENObjectMath.NativeSin(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 ResultValue.ValueType:=bvtNUMBER;
 if CountArguments=0 then begin
  ResultValue.Num:=TBESENNumber(pointer(@BESENDoubleNaN)^);
 end else begin
  TBESEN(Instance).ToNumberValue(Arguments^[0]^,ResultValue);
  if BESENIsNaN(ResultValue.Num) or BESENIsInfinite(ResultValue.Num) then begin
   ResultValue.Num:=TBESENNumber(pointer(@BESENDoubleNaN)^);
  end else if not BESENIsZero(ResultValue.Num) then begin
   ResultValue.Num:=sin(ResultValue.Num);
  end;
 end;
end;

procedure TBESENObjectMath.NativeSqrt(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 ResultValue.ValueType:=bvtNUMBER;
 if CountArguments=0 then begin
  ResultValue.Num:=TBESENNumber(pointer(@BESENDoubleNaN)^);
 end else begin
  TBESEN(Instance).ToNumberValue(Arguments^[0]^,ResultValue);
  if BESENIsNaN(ResultValue.Num) or BESENIsNegative(ResultValue.Num) then begin
   ResultValue.Num:=TBESENNumber(pointer(@BESENDoubleNaN)^);
  end else if not (BESENIsZero(ResultValue.Num) or BESENIsPosInfinite(ResultValue.Num)) then begin
   ResultValue.Num:=sqrt(ResultValue.Num);
  end;
 end;
end;

procedure TBESENObjectMath.NativeTan(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 ResultValue.ValueType:=bvtNUMBER;
 if CountArguments=0 then begin
  ResultValue.Num:=TBESENNumber(pointer(@BESENDoubleNaN)^);
 end else begin                                                                       
  TBESEN(Instance).ToNumberValue(Arguments^[0]^,ResultValue);
  if BESENIsNaN(ResultValue.Num) or BESENIsInfinite(ResultValue.Num) then begin
   ResultValue.Num:=TBESENNumber(pointer(@BESENDoubleNaN)^);
  end else if not BESENIsZero(ResultValue.Num) then begin
   ResultValue.Num:=tan(ResultValue.Num);
  end;
 end;
end;

end.
