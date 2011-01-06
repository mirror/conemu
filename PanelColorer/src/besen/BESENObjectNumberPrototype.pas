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
unit BESENObjectNumberPrototype;
{$i BESEN.inc}

interface

uses Math,BESENConstants,BESENTypes,BESENObject,BESENObjectNumber,BESENValue,BESENObjectPropertyDescriptor,
     BESENObjectBoolean;

type TBESENObjectNumberPrototype=class(TBESENObjectNumber)
      public
       constructor Create(AInstance:TObject;APrototype:TBESENObject=nil;AHasPrototypeProperty:longbool=false); overload; override;
       destructor Destroy; override;
       procedure NativeToString(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeToLocaleString(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeValueOf(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeToFixed(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeToExponential(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeToPrecision(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
     end;

implementation

uses BESEN,BESENErrors,BESENNumberUtils,BESENDTOA;

constructor TBESENObjectNumberPrototype.Create(AInstance:TObject;APrototype:TBESENObject=nil;AHasPrototypeProperty:longbool=false);
begin
 inherited Create(AInstance,APrototype,AHasPrototypeProperty);
 ObjectClassName:='Number';
 ObjectName:='Number';

 if (TBESEN(Instance).Compatibility and COMPAT_JS)<>0 then begin
  OverwriteData('name',BESENStringValue(ObjectName),[]);
 end;

 RegisterNativeFunction('toString',NativeToString,1,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('toLocaleString',NativeToLocaleString,0,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('valueOf',NativeValueOf,0,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('toFixed',NativeToFixed,1,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('toExponential',NativeToExponential,1,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('toPrecision',NativeToPrecision,1,[bopaWRITABLE,bopaCONFIGURABLE],false);
end;

destructor TBESENObjectNumberPrototype.Destroy;
begin
 inherited Destroy;
end;

procedure TBESENObjectNumberPrototype.NativeToString(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
var Radix:integer;
    nv:TBESENValue;
begin
 if ThisArgument.ValueType=bvtNUMBER then begin
  nv:=ThisArgument;
 end else if ((ThisArgument.ValueType=bvtOBJECT) and assigned(TBESENObject(ThisArgument.Obj))) and (TBESENObject(ThisArgument.Obj) is TBESENObjectNumber) then begin
  nv:=BESENNumberValue(TBESENObjectNumber(TBESENObject(ThisArgument.Obj)).Value);
 end else begin
  raise EBESENTypeError.Create('Not a Number object');
 end;
 if CountArguments=0 then begin
  Radix:=10;
 end else begin
  Radix:=TBESEN(Instance).ToInt32(Arguments^[0]^);
 end;
 if Radix=10 then begin
  TBESEN(Instance).ToStringValue(nv,ResultValue);
 end else if Radix in [2..36] then begin
  ResultValue.ValueType:=bvtSTRING;
  if BESENIsNaN(nv.Num) then begin
   ResultValue.Str:='NaN';
  end else if BESENIsZero(nv.Num) then begin
   ResultValue.Str:='0';
  end else if BESENIsInfinite(nv.Num) then begin
   if BESENIsNegative(nv.Num) then begin
    ResultValue.Str:='-Infinity';
   end else begin
    ResultValue.Str:='Infinity';
   end;
  end else begin
   ResultValue.Str:=dtobase(nv.Num,Radix);
  end;
 end else begin
  raise EBESENRangeError.Create('Bad radix');
 end;
end;

procedure TBESENObjectNumberPrototype.NativeToLocaleString(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
var nv:TBESENValue;
begin
 if ThisArgument.ValueType=bvtNUMBER then begin
  nv:=ThisArgument;
 end else if ((ThisArgument.ValueType=bvtOBJECT) and assigned(TBESENObject(ThisArgument.Obj))) and (TBESENObject(ThisArgument.Obj) is TBESENObjectNumber) then begin
  nv:=BESENNumberValue(TBESENObjectNumber(TBESENObject(ThisArgument.Obj)).Value);
 end else begin
  raise EBESENTypeError.Create('Not a Number object');
 end;
 ResultValue:=BESENStringValue(BESENFloatToLocaleStr(nv.Num));
end;

procedure TBESENObjectNumberPrototype.NativeValueOf(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 if ThisArgument.ValueType=bvtNUMBER then begin
  ResultValue:=ThisArgument;
 end else if ((ThisArgument.ValueType=bvtOBJECT) and assigned(TBESENObject(ThisArgument.Obj))) and (TBESENObject(ThisArgument.Obj) is TBESENObjectNumber) then begin
  ResultValue:=BESENNumberValue(TBESENObjectNumber(TBESENObject(ThisArgument.Obj)).Value);
 end else begin
  raise EBESENTypeError.Create('Not a Number object');
 end;
end;

procedure TBESENObjectNumberPrototype.NativeToFixed(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
{$ifdef UseDTOA}
var f:int64;
    v,nv:TBESENValue;
    x:TBESENNumber;
begin
 if ThisArgument.ValueType=bvtNUMBER then begin
  nv:=ThisArgument;
 end else if ((ThisArgument.ValueType=bvtOBJECT) and assigned(TBESENObject(ThisArgument.Obj))) and (TBESENObject(ThisArgument.Obj) is TBESENObjectNumber) then begin
  nv:=BESENNumberValue(TBESENObjectNumber(TBESENObject(ThisArgument.Obj)).Value);
 end else begin
  raise EBESENTypeError.Create('Not a Number object');
 end;
 f:=0;
 if (CountArguments>0) and (Arguments^[0]^.ValueType<>bvtUNDEFINED) then begin
  TBESEN(Instance).ToIntegerValue(Arguments^[0]^,v);
  if (TBESEN(Instance).Compatibility and COMPAT_BESEN)<>0 then begin
   if BESENIsNaN(v.Num) or ((v.Num<0) or (v.Num>1076)) then begin
    raise EBESENRangeError.Create('Fixed width '+BESENFloatToStr(v.Num)+' out of range');
   end;
  end else begin
   if BESENIsNaN(v.Num) or ((v.Num<0) or (v.Num>20)) then begin
    raise EBESENRangeError.Create('Fixed width '+BESENFloatToStr(v.Num)+' out of range');
   end;
  end;
  f:=trunc(v.Num);
 end;
 x:=nv.Num;
 ResultValue.ValueType:=bvtSTRING;
 if (not BESENIsFinite(x)) or ((x<-1e+21) or (x>1e+21)) then begin
  ResultValue.Str:=BESENFloatToStr(x);
 end else begin
  ResultValue.Str:=dtostr(x,DTOSTR_FIXED,f);
 end;
end;
{$else}
var f:int64;
    v,nv:TBESENValue;
    x:TBESENNumber;
    s:TBESENString;
begin
 if ThisArgument.ValueType=bvtNUMBER then begin
  nv:=ThisArgument;
 end else if ((ThisArgument.ValueType=bvtOBJECT) and assigned(TBESENObject(ThisArgument.Obj))) and (TBESENObject(ThisArgument.Obj) is TBESENObjectNumber) then begin
  nv:=BESENNumberValue(TBESENObjectNumber(TBESENObject(ThisArgument.Obj)).Value);
 end else begin
  raise EBESENTypeError.Create('Not a Number object');
 end;
 f:=0;
 if (CountArguments>0) and (Arguments^[0]^.ValueType<>bvtUNDEFINED) then begin
  TBESEN(Instance).ToIntegerValue(Arguments^[0]^,v);
  if BESENIsNaN(v.Num) or ((v.Num<0) or (v.Num>20)) then begin
   raise EBESENRangeError.Create('Fixed width '+BESENFloatToStr(v.Num)+' out of range');
  end;
  f:=trunc(v.Num);
 end;
 x:=nv.Num;
 ResultValue.ValueType:=bvtSTRING;
 if (not BESENIsFinite(x)) or ((x<-1e+21) or (x>1e+21)) then begin
  ResultValue.Str:=BESENFloatToStr(x);
 end else begin
  str(x:1:f,s);
  ResultValue.Str:=s;
 end;
end;
{$endif}

procedure TBESENObjectNumberPrototype.NativeToExponential(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
var v,nv:TBESENValue;
    x:TBESENNumber;
    f:integer;
begin
 if ThisArgument.ValueType=bvtNUMBER then begin
  nv:=ThisArgument;
 end else if ((ThisArgument.ValueType=bvtOBJECT) and assigned(TBESENObject(ThisArgument.Obj))) and (TBESENObject(ThisArgument.Obj) is TBESENObjectNumber) then begin
  nv:=BESENNumberValue(TBESENObjectNumber(TBESENObject(ThisArgument.Obj)).Value);
 end else begin
  raise EBESENTypeError.Create('Not a Number object');
 end;
 f:=0;
 if (CountArguments>0) and (Arguments^[0]^.ValueType<>bvtUNDEFINED) then begin
  TBESEN(Instance).ToIntegerValue(Arguments^[0]^,v);
  if (TBESEN(Instance).Compatibility and COMPAT_BESEN)<>0 then begin
   if BESENIsNaN(v.Num) or ((v.Num<0) or (v.Num>1076)) then begin
    raise EBESENRangeError.Create('Exponent width '+BESENFloatToStr(v.Num)+' out of range');
   end;
  end else begin
   if BESENIsNaN(v.Num) or ((v.Num<0) or (v.Num>20)) then begin
    raise EBESENRangeError.Create('Exponent width '+BESENFloatToStr(v.Num)+' out of range');
   end;
  end;
  f:=trunc(v.Num);
 end;
 x:=nv.Num;
 if not BESENIsFinite(x) then begin
  ResultValue:=BESENStringValue(BESENFloatToStr(x));
 end else begin
  if (CountArguments>0) and (Arguments^[0]^.ValueType<>bvtUNDEFINED) then begin
   ResultValue:=BESENStringValue(dtostr(x,DTOSTR_EXPONENTIAL,f+1));
  end else begin
   ResultValue:=BESENStringValue(dtostr(x,DTOSTR_STANDARD_EXPONENTIAL,0));
  end;
 end;
end;

procedure TBESENObjectNumberPrototype.NativeToPrecision(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
var f:int64;
    v,nv:TBESENValue;
    x:TBESENNumber;
begin
 if ThisArgument.ValueType=bvtNUMBER then begin
  nv:=ThisArgument;
 end else if ((ThisArgument.ValueType=bvtOBJECT) and assigned(TBESENObject(ThisArgument.Obj))) and (TBESENObject(ThisArgument.Obj) is TBESENObjectNumber) then begin
  nv:=BESENNumberValue(TBESENObjectNumber(TBESENObject(ThisArgument.Obj)).Value);
 end else begin
  raise EBESENTypeError.Create('Not a Number object');
 end;
 f:=0;
 if (CountArguments>0) and (Arguments^[0]^.ValueType<>bvtUNDEFINED) then begin
  TBESEN(Instance).ToIntegerValue(Arguments^[0]^,v);
  if (TBESEN(Instance).Compatibility and COMPAT_BESEN)<>0 then begin
   if BESENIsNaN(v.Num) or ((v.Num<1) or (v.Num>1076)) then begin
    raise EBESENRangeError.Create('Precision '+BESENFloatToStr(v.Num)+' out of range');
   end;
  end else begin
   if BESENIsNaN(v.Num) or ((v.Num<1) or (v.Num>21)) then begin
    raise EBESENRangeError.Create('Precision '+BESENFloatToStr(v.Num)+' out of range');
   end;
  end;
  f:=trunc(v.Num);
 end;
 x:=nv.Num;
 if (not BESENIsFinite(x)) or ((x<-1e+21) or (x>1e+21)) then begin
  ResultValue:=BESENStringValue(BESENFloatToStr(x));
 end else begin
  ResultValue:=BESENStringValue(dtostr(x,DTOSTR_PRECISION,f));
 end;
end;

end.
