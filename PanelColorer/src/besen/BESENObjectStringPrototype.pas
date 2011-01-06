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
unit BESENObjectStringPrototype;
{$i BESEN.inc}

interface

uses SysUtils,Math,BESENConstants,BESENTypes,BESENObject,BESENObjectString,BESENValue,BESENObjectPropertyDescriptor,
     BESENObjectBoolean;

type TBESENObjectStringPrototype=class(TBESENObjectString)
      private
       function RegExpArg(Arguments:PPBESENValues;CountArguments:integer):TBESENObject;
      public
       constructor Create(AInstance:TObject;APrototype:TBESENObject=nil;AHasPrototypeProperty:longbool=false); overload; override;
       destructor Destroy; override;
       procedure NativeToString(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeValueOf(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeCharAt(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeCharCodeAt(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeConcat(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeIndexOf(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeLastIndexOf(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeLocaleCompare(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeMatch(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeReplace(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeSearch(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeSlice(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeSplit(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeSubstring(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeToLowerCase(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeToLocaleLowerCase(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeToUpperCase(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeToLocaleUpperCase(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeTrim(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeSubstr(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeAnchor(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeBig(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeBlink(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeBold(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeFixed(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeFontColor(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeFontSize(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeItalics(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeLink(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeSmall(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeStrike(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeSub(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeSup(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
     end;

implementation

uses BESEN,BESENErrors,BESENObjectRegExp,BESENNumberUtils,BESENRegExp,BESENStringUtils,
     BESENObjectArray;

constructor TBESENObjectStringPrototype.Create(AInstance:TObject;APrototype:TBESENObject=nil;AHasPrototypeProperty:longbool=false);
begin
 inherited Create(AInstance,APrototype,AHasPrototypeProperty);
 ObjectClassName:='String';
 ObjectName:='Number';

 if (TBESEN(Instance).Compatibility and COMPAT_JS)<>0 then begin
  OverwriteData('name',BESENStringValue(ObjectName),[]);
 end;

 OverwriteData('length',BESENNumberValue(0),[]);

 RegisterNativeFunction('toString',NativeToString,0,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('valueOf',NativeValueOf,0,[bopaWRITABLE,bopaCONFIGURABLE],false);

 RegisterNativeFunction('charAt',NativeCharAt,1,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('charCodeAt',NativeCharCodeAt,1,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('concat',NativeConcat,1,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('indexOf',NativeIndexOf,1,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('lastIndexOf',NativeLastIndexOf,1,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('localeCompare',NativeLocaleCompare,1,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('match',NativeMatch,1,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('replace',NativeReplace,1,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('search',NativeSearch,1,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('slice',NativeSlice,2,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('split',NativeSplit,2,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('substring',NativeSubString,2,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('toLowerCase',NativeToLowerCase,0,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('toLocaleLowerCase',NativeToLocaleLowerCase,0,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('toUpperCase',NativeToUpperCase,0,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('toLocaleUpperCase',NativeToLocaleUpperCase,0,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('trim',NativeTrim,0,[bopaWRITABLE,bopaCONFIGURABLE],false);
                                   
 RegisterNativeFunction('substr',NativeSubStr,2,[bopaWRITABLE,bopaCONFIGURABLE],false);

 if (TBESEN(Instance).Compatibility and COMPAT_JS)<>0 then begin
  RegisterNativeFunction('anchor',NativeAnchor,1,[bopaWRITABLE,bopaCONFIGURABLE],false);
  RegisterNativeFunction('big',NativeBig,0,[bopaWRITABLE,bopaCONFIGURABLE],false);
  RegisterNativeFunction('blink',NativeBlink,0,[bopaWRITABLE,bopaCONFIGURABLE],false);
  RegisterNativeFunction('bold',NativeBold,0,[bopaWRITABLE,bopaCONFIGURABLE],false);
  RegisterNativeFunction('fixed',NativeFixed,0,[bopaWRITABLE,bopaCONFIGURABLE],false);
  RegisterNativeFunction('fontcolor',NativeFontColor,1,[bopaWRITABLE,bopaCONFIGURABLE],false);
  RegisterNativeFunction('fontsize',NativeFontSize,1,[bopaWRITABLE,bopaCONFIGURABLE],false);
  RegisterNativeFunction('italics',NativeItalics,0,[bopaWRITABLE,bopaCONFIGURABLE],false);
  RegisterNativeFunction('link',NativeLink,1,[bopaWRITABLE,bopaCONFIGURABLE],false);
  RegisterNativeFunction('small',NativeSmall,0,[bopaWRITABLE,bopaCONFIGURABLE],false);
  RegisterNativeFunction('strike',NativeStrike,0,[bopaWRITABLE,bopaCONFIGURABLE],false);
  RegisterNativeFunction('sub',NativeSub,0,[bopaWRITABLE,bopaCONFIGURABLE],false);
  RegisterNativeFunction('sup',NativeSup,1,[bopaWRITABLE,bopaCONFIGURABLE],false);
 end;
end;

destructor TBESENObjectStringPrototype.Destroy;
begin
 inherited Destroy;
end;

function TBESENObjectStringPrototype.RegExpArg(Arguments:PPBESENValues;CountArguments:integer):TBESENObject;
var v:TBESENValue;
begin
 v:=BESENEmptyValue;
 if CountArguments<0 then begin
  TBESEN(Instance).ObjectConstruct(TBESEN(Instance).ObjectRegExpConstructor,BESENObjectValue(TBESEN(Instance).ObjectRegExpConstructor),nil,0,v);
  if v.ValueType=bvtOBJECT then begin
   result:=TBESENObject(v.Obj);
  end else begin
   result:=nil;
  end;
 end else if (Arguments^[0]^.ValueType=bvtOBJECT) and assigned(Arguments^[0]^.Obj) and (Arguments^[0]^.Obj is TBESENObjectRegExp) then begin
  result:=TBESENObject(Arguments^[0]^.Obj);
 end else begin
  TBESEN(Instance).ObjectConstruct(TBESEN(Instance).ObjectRegExpConstructor,BESENObjectValue(TBESEN(Instance).ObjectRegExpConstructor),Arguments,1,v);
  if v.ValueType=bvtOBJECT then begin
   result:=TBESENObject(v.Obj);
  end else begin
   result:=nil;
  end;
 end;
{$ifdef UseAssert}
 Assert(assigned(result));
{$endif}
end;

procedure TBESENObjectStringPrototype.NativeToString(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 if ThisArgument.ValueType=bvtSTRING then begin
  BESENCopyValue(ResultValue,ThisArgument);
 end else if ((ThisArgument.ValueType=bvtOBJECT) and assigned(TBESENObject(ThisArgument.Obj))) and (TBESENObject(ThisArgument.Obj) is TBESENObjectString) then begin
  ResultValue:=BESENStringValue(TBESENObjectString(TBESENObject(ThisArgument.Obj)).Value);
 end else begin
  raise EBESENTypeError.Create('Not a string object');
 end;
end;

procedure TBESENObjectStringPrototype.NativeValueOf(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 if ThisArgument.ValueType=bvtSTRING then begin
  BESENCopyValue(ResultValue,ThisArgument);
 end else if ((ThisArgument.ValueType=bvtOBJECT) and assigned(TBESENObject(ThisArgument.Obj))) and (TBESENObject(ThisArgument.Obj) is TBESENObjectString) then begin
  ResultValue:=BESENStringValue(TBESENObjectString(TBESENObject(ThisArgument.Obj)).Value);
 end else begin
  raise EBESENTypeError.Create('Not a string object');
 end;
end;

procedure TBESENObjectStringPrototype.NativeCharAt(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
var v:TBESENValue;
    i:integer;
    s:TBESENString;
begin
 BESENCheckObjectCoercible(ThisArgument);
 s:=TBESEN(Instance).ToStr(ThisArgument);
 v:=BESENEmptyValue;
 if (CountArguments>0) and (Arguments^[0]^.ValueType<>bvtUNDEFINED) then begin
  i:=-1;
  TBESEN(Instance).ToIntegerValue(Arguments^[0]^,v);
  if BESENIsFinite(v.Num) then begin
   i:=trunc(v.Num);
  end;
 end else begin
  i:=0;
 end;
 if (i>=0) and (i<length(s)) then begin
  ResultValue:=BESENStringValue(copy(s,i+1,1));
 end else begin
  ResultValue:=BESENStringValue('');
 end;
end;

procedure TBESENObjectStringPrototype.NativeCharCodeAt(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
var v:TBESENValue;
    i:integer;
    s:TBESENString;
begin
 BESENCheckObjectCoercible(ThisArgument);
 s:=TBESEN(Instance).ToStr(ThisArgument);
 if (CountArguments>0) and (Arguments^[0]^.ValueType<>bvtUNDEFINED) then begin
  i:=-1;
  TBESEN(Instance).ToIntegerValue(Arguments^[0]^,v);
  if BESENIsFinite(v.Num) then begin
   i:=trunc(v.Num);
  end;
 end else begin
  i:=0;
 end;
 if (i>=0) and (i<length(s)) then begin
  ResultValue:=BESENNumberValue(word(widechar(s[i+1])));
 end else begin
  ResultValue:=BESENNumberValue(0);
  move(BESENDoubleNaN,ResultValue.Num,sizeof(TBESENNumber));
 end;
end;

procedure TBESENObjectStringPrototype.NativeConcat(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
var s:TBESENString;
    i:integer;
begin
 BESENCheckObjectCoercible(ThisArgument);
 s:=TBESEN(Instance).ToStr(ThisArgument);
 for i:=0 to CountArguments-1 do begin
  s:=s+TBESEN(Instance).ToStr(Arguments^[i]^);
 end;
 ResultValue:=BESENStringValue(s);
end;

procedure TBESENObjectStringPrototype.NativeIndexOf(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
var v:TBESENValue;
    i,j,k,l,ls,p,fp,fl:integer;
    s,ss:TBESENString;
begin
 BESENCheckObjectCoercible(ThisArgument);
 s:=TBESEN(Instance).ToStr(ThisArgument);
 if CountArguments<1 then begin
  ss:='';
 end else begin
  ss:=TBESEN(Instance).ToStr(Arguments^[0]^);
 end;
 p:=0;
 if (CountArguments>1) and (Arguments^[1]^.ValueType<>bvtUNDEFINED) then begin
  TBESEN(Instance).ToIntegerValue(Arguments^[1]^,v);
  p:=trunc(v.Num);
 end;
 if p<0 then begin
  p:=0;
 end else if p>=length(s) then begin
  p:=length(s)-1;
 end;
 inc(p);
 ls:=length(ss);
 l:=(length(s)-ls)+1;
 fp:=0;
 for i:=p to l do begin
  fl:=0;
  for j:=1 to ls do begin
   k:=i+j-1;
   if ((k<1) or (k>length(s))) or (s[k]<>ss[j]) then begin
    break;
   end else begin
    inc(fl);
   end;
  end;
  if fl=ls then begin
   fp:=i;
   break;
  end;
 end;
 ResultValue:=BESENNumberValue(fp-1);
end;

procedure TBESENObjectStringPrototype.NativeLastIndexOf(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
var v,vi:TBESENValue;
    i,j,k,l,ls,p,fp,fl:integer;
    s,ss:TBESENString;
begin
 BESENCheckObjectCoercible(ThisArgument);
 s:=TBESEN(Instance).ToStr(ThisArgument);
 if CountArguments<1 then begin
  ss:='';
 end else begin
  ss:=TBESEN(Instance).ToStr(Arguments^[0]^);
 end;
 vi:=BESENEmptyValue;
 if (CountArguments>1) and (Arguments^[1]^.ValueType<>bvtUNDEFINED) then begin
  TBESEN(Instance).ToIntegerValue(Arguments^[1]^,v);
 end else begin
  v:=BESENNumberValue(0);
  move(BESENDoubleNaN,v.Num,sizeof(TBESENNumber));
 end;
 if BESENIsNaN(v.Num) then begin
  vi:=BESENNumberValue(0);
  move(BESENDoubleInfPos,vi.Num,sizeof(TBESENNumber));
 end else begin
  TBESEN(Instance).ToIntegerValue(v,vi);
 end;
 l:=trunc(min(max(vi.Num,0),length(s)));
 ls:=length(ss);
 if l<ls then begin
  ResultValue:=BESENNumberValue(-1);
  exit;
 end;
 p:=length(s)-ls;
 if p<l then begin
  l:=p;
 end;
 fp:=-1;
 for i:=l downto 0 do begin
  fl:=0;
  for j:=1 to ls do begin
   k:=i+j;
   if ((k<1) or (k>length(s))) or (s[k]<>ss[j]) then begin
    break;
   end else begin
    inc(fl);
   end;
  end;
  if fl=ls then begin
   fp:=i;
   break;
  end;
 end;
 ResultValue:=BESENNumberValue(fp);
end;

procedure TBESENObjectStringPrototype.NativeLocaleCompare(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
var s,ss:TBESENString;
begin
 BESENCheckObjectCoercible(ThisArgument);
 s:=TBESEN(Instance).ToStr(ThisArgument);
 if CountArguments<1 then begin
  ss:='undefined';
 end else begin
  ss:=TBESEN(Instance).ToStr(Arguments^[0]^);
 end;
 if s<ss then begin
  ResultValue:=BESENNumberValue(-1);
 end else if s>ss then begin
  ResultValue:=BESENNumberValue(1);
 end else begin
  ResultValue:=BESENNumberValue(0);
 end;
end;

procedure TBESENObjectStringPrototype.NativeMatch(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
var RegExp:TBESENObjectRegExp;
    v,vr,vs:TBESENValue;
    Exec:TBESENObject;
    Global:boolean;
    s:TBESENString;
    a:TBESENObjectArray;
    n,Matches:integer;
    ValuePointers:array[0..0] of PBESENValue;
begin
 BESENCheckObjectCoercible(ThisArgument);
 s:=TBESEN(Instance).ToStr(ThisArgument);

 v:=BESENEmptyValue;

 RegExp:=TBESENObjectRegExp(RegExpArg(Arguments,CountArguments));
 RegExp.Get('exec',v);
{$ifdef UseAssert}
 Assert((v.ValueType=bvtOBJECT) and assigned(TBESENObject(v.Obj)) and TBESENObject(v.Obj).HasCall);
{$endif}
 Exec:=TBESENObject(v.Obj);

 RegExp.Get('global',v);
{$ifdef UseAssert}
 Assert(v.ValueType=bvtBOOLEAN);
{$endif}
 Global:=v.Bool;
 if Global then begin
  v:=BESENEmptyValue;
  vr:=BESENEmptyValue;

  Matches:=0;

  RegExp.OverwriteData('lastIndex',BESENNumberValue(0),[bopaWRITABLE]);

  a:=TBESENObjectArray.Create(Instance,TBESEN(Instance).ObjectArrayPrototype,false);
  TBESEN(Instance).GarbageCollector.Add(a);
  a.GarbageCollectorLock;
  try
   n:=0;
   while true do begin
    vs.ValueType:=bvtSTRING;
    vs.Str:=s;
    ValuePointers[0]:=@vs;
    TBESEN(Instance).ObjectCall(Exec,BESENObjectValue(RegExp),@ValuePointers,1,vr);
    if vr.ValueType=bvtNULL then begin
     break;
    end;

 {$ifdef UseAssert}
    Assert((vr.ValueType=bvtOBJECT) and assigned(vr.Obj) and (vr.Obj is TBESENObjectArray));
 {$endif}
    TBESENObject(vr.Obj).Get('0',v);
 {$ifdef UseAssert}
    Assert(v.ValueType=bvtSTRING);
 {$endif}

    a.OverwriteData(inttostr(n),v,[bopaWRITABLE,bopaENUMERABLE,bopaCONFIGURABLE]);
    inc(Matches);

    if length(v.Str)=0 then begin
     RegExp.Get('lastIndex',v);
 {$ifdef UseAssert}
     Assert(v.ValueType=bvtNUMBER);
 {$endif}
     v.Num:=v.Num+1;
     RegExp.OverwriteData('lastIndex',v,[bopaWRITABLE]);
    end;
    inc(n);
   end;
  finally
   a.GarbageCollectorUnlock;
  end;
  if Matches=0 then begin
   ResultValue:=BESENNullValue;
  end else begin
   ResultValue:=BESENObjectValue(a);
  end;
 end else begin
  vs.ValueType:=bvtSTRING;
  vs.Str:=s;
  ValuePointers[0]:=@vs;
  TBESEN(Instance).ObjectCall(Exec,BESENObjectValue(RegExp),@ValuePointers,1,ResultValue);
 end;
end;

procedure TBESENObjectStringPrototype.NativeReplace(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
 procedure Helper(var PrevIndexP:longword;a:TBESENObject;var OutStr:TBESENString;const Source:TBESENString;var ReplaceValue:TBESENValue;CountCaptures:integer);
 var v,vr:TBESENValue;
     n:integer;
     Index,i,j,k:longword;
     ms,Replace:TBESENString;
     Values:TBESENValues;
     pValues:TBESENValuePointers;
 begin
  v:=BESENEmptyValue;

  a.Get('index',v);
  Index:=TBESEN(Instance).ToUInt32(v);

  a.Get('0',v);
{$ifdef UseAssert}
  Assert(v.ValueType=bvtSTRING);
{$endif}
  ms:=TBESEN(Instance).ToStr(v);

  i:=PrevIndexP;
  if i<Index then begin
   OutStr:=OutStr+copy(Source,i+1,Index-i);
  end;
  PrevIndexP:=Index+longword(length(ms));

  if ReplaceValue.ValueType=bvtOBJECT then begin
   Values:=nil;
   pValues:=nil;
   try
    SetLength(Values,CountCaptures+2);
    SetLength(pValues,CountCaptures+2);
    for n:=0 to length(Values)-1 do begin
     Values[n]:=BESENEmptyValue;
    end;
    for n:=0 to CountCaptures-1 do begin
     a.Get(inttostr(n),Values[n]);
    end;
    Values[CountCaptures]:=BESENNumberValue(Index);
    Values[CountCaptures+1]:=BESENStringValue(Source);
    for n:=0 to length(Values)-1 do begin
     pValues[n]:=@Values[n];
    end;
    TBESEN(Instance).ObjectCall(TBESENObject(ReplaceValue.Obj),BESENObjectValueEx(TBESENObject(ReplaceValue.Obj)),@pValues[0],length(pValues),vr);
    OutStr:=OutStr+TBESEN(Instance).ToStr(vr);
   finally
    SetLength(Values,0);
    SetLength(pValues,0);
   end;
   exit;
  end;

  Replace:=TBESEN(Instance).ToStr(ReplaceValue);

  i:=0;
  while i<longword(length(Replace)) do begin
   if (Replace[i+1]='$') and ((i+1)<longword(length(Replace))) then begin
    inc(i);
    case Replace[i+1] of
     '$':begin
      OutStr:=OutStr+'$';
      inc(i);
      continue;
     end;
     '`':begin
      k:=0;
      while k<Index do begin
       OutStr:=OutStr+Source[k+1];
       inc(k);
      end;
      inc(i);
      continue;
     end;
     '''':begin
      k:=PrevIndexP;
      while k<longword(length(Source)) do begin
       OutStr:=OutStr+Source[k+1];
       inc(k);
      end;
      inc(i);
      continue;
     end;
     '&':begin
      OutStr:=OutStr+ms;
      inc(i);
      continue;
     end;
    end;
    j:=i;
    n:=0;
    while (j<longword(length(Replace))) and ((word(widechar(Replace[j+1]))>=ord('0')) and (word(widechar(Replace[j+1]))<=ord('9'))) do begin
     n:=(n*10)+(word(widechar(Replace[j+1]))-ord('0'));
     inc(j);
    end;
    if i=j then begin
     OutStr:=OutStr+'$';
     continue;
    end;
    a.Get(inttostr(n),v);
    if v.ValueType<>bvtUNDEFINED then begin
{$ifdef UseAssert}
     Assert(v.ValueType=bvtSTRING);
{$endif}
     OutStr:=OutStr+v.Str;
    end;
    i:=j;
   end else begin
    OutStr:=OutStr+Replace[i+1];
    inc(i);
   end;
  end;
 end;
var RegExp:TBESENObjectRegExp;
    CountCaptures:integer;
    ReplaceValue,v,v2,vr,vs:TBESENValue;
    Exec:TBESENObject;
    Global:boolean;
    s,OutStr:TBESENString;
    PrevIndex:longword;
    ValuePointers:array[0..0] of PBESENValue;
    HasOutputString:boolean;
begin
 BESENCheckObjectCoercible(ThisArgument);
 s:=TBESEN(Instance).ToStr(ThisArgument);

 RegExp:=TBESENObjectRegExp(RegExpArg(Arguments,CountArguments));
 CountCaptures:=RegExp.Engine.CountOfCaptures;
 ReplaceValue:=BESENEmptyValue;
 v:=BESENEmptyValue;
 v2:=BESENEmptyValue;
 vr:=BESENEmptyValue;
 OutStr:='';

 PrevIndex:=0;

 if CountArguments<2 then begin
  ReplaceValue.ValueType:=bvtSTRING;
  ReplaceValue.Str:='';
 end else if (Arguments^[1]^.ValueType=bvtOBJECT) and assigned(Arguments^[1]^.Obj) and TBESENObject(Arguments^[1]^.Obj).HasCall then begin
  BESENCopyValue(ReplaceValue,Arguments^[1]^);
 end else begin
  TBESEN(Instance).ToStringValue(Arguments^[1]^,ReplaceValue);
 end;

 RegExp.Get('exec',v);
{$ifdef UseAssert}
 Assert((v.ValueType=bvtOBJECT) and assigned(TBESENObject(v.Obj)) and TBESENObject(v.Obj).HasCall);
{$endif}
 Exec:=TBESENObject(v.Obj);

 RegExp.Get('global',v);
{$ifdef UseAssert}
 Assert(v.ValueType=bvtBOOLEAN);
{$endif}
 Global:=v.Bool;

 HasOutputString:=false;

 if Global then begin
  RegExp.OverwriteData('lastIndex',BESENNumberValue(0),[bopaWRITABLE]);
  while true do begin
   vs.ValueType:=bvtSTRING;
   vs.Str:=s;
   ValuePointers[0]:=@vs;
   TBESEN(Instance).ObjectCall(Exec,BESENObjectValue(RegExp),@ValuePointers,1,vr);
   if vr.ValueType=bvtNULL then begin
    break;
   end;
{$ifdef UseAssert}
   Assert((vr.ValueType=bvtOBJECT) and assigned(TBESENObject(vr.Obj)) and (TBESENObject(vr.Obj) is TBESENObjectArray));
{$endif}
   TBESENObject(vr.Obj).Get('0',v);
{$ifdef UseAssert}
   Assert(v.ValueType=bvtSTRING);
{$endif}
   if length(v.Str)<>0 then begin
    Helper(PrevIndex,TBESENObject(vr.Obj),OutStr,s,ReplaceValue,CountCaptures);
    HasOutputString:=true;
   end else begin
    RegExp.Get('lastIndex',v);
{$ifdef UseAssert}
    Assert(v.ValueType=bvtNUMBER);
{$endif}
    v.Num:=v.Num+1;
    RegExp.OverwriteData('lastIndex',v,[bopaWRITABLE]);
   end;
  end;
 end else begin
  v.ValueType:=bvtSTRING;
  v.Str:=s;
  ValuePointers[0]:=@v;
  TBESEN(Instance).ObjectCall(Exec,BESENObjectValue(RegExp),@ValuePointers,1,v2);
  if v2.ValueType<>bvtNULL then begin
{$ifdef UseAssert}
   Assert((v2.ValueType=bvtOBJECT) and assigned(v2.Obj) and (v2.Obj is TBESENObjectArray));
{$endif}
   Helper(PrevIndex,TBESENObject(v2.Obj),OutStr,s,ReplaceValue,CountCaptures);
   HasOutputString:=true;
  end;
 end;

 if HasOutputString then begin
  ResultValue:=BESENStringValue(OutStr+copy(s,PrevIndex+1,(longword(length(s))-PrevIndex)+1));
 end else begin
  ResultValue:=BESENStringValue(s);
 end;
end;

procedure TBESENObjectStringPrototype.NativeSearch(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
var RegExp:TBESENObjectRegExp;
    s:TBESENANSISTRING;
    i:integer;
    Captures:TBESENRegExpCaptures;
begin
 BESENCheckObjectCoercible(ThisArgument);
 s:=TBESEN(Instance).ToStr(ThisArgument);
 RegExp:=TBESENObjectRegExp(RegExpArg(Arguments,CountArguments));
 Captures:=nil;
 try
  ResultValue:=BESENNumberValue(-1);
  for i:=0 to length(s)-1 do begin
   if RegExp.Engine.Match(s,i,Captures) then begin
    ResultValue:=BESENNumberValue(Captures[0].s);
    break;
   end;
  end;
 finally
  SetLength(Captures,0);
 end;
end;

procedure TBESENObjectStringPrototype.NativeSlice(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
var s:TBESENString;
    len,intStart,intEnd,sFrom,sTo,span:integer;
begin
 BESENCheckObjectCoercible(ThisArgument);

 s:=TBESEN(Instance).ToStr(ThisArgument);

 len:=length(s);

 if (CountArguments<1) or (Arguments^[0]^.ValueType=bvtUNDEFINED) then begin
  intStart:=0;
 end else begin
  intStart:=TBESEN(Instance).ToInt(Arguments^[0]^);
 end;

 if (CountArguments<2) or (Arguments^[1]^.ValueType=bvtUNDEFINED) then begin
  intEnd:=len;
 end else begin
  intEnd:=TBESEN(Instance).ToInt(Arguments^[1]^);
 end;

 if intStart<0 then begin
  sFrom:=max(len+intStart,0);
 end else begin
  sFrom:=min(intStart,len);
 end;

 if intEnd<0 then begin
  sTo:=max(len+intEnd,0);
 end else begin
  STo:=min(intEnd,len);
 end;

 span:=max(sTo-sFrom,0);

 if span=0 then begin
  ResultValue:=BESENStringValue('');
 end else begin
  ResultValue:=BESENStringValue(copy(s,sFrom+1,span));
 end;
end;

procedure TBESENObjectStringPrototype.NativeSplit(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
 function SplitMatch(var r:TBESENValue;var s:TBESENString;q:integer;var Captures:TBESENRegExpCaptures):boolean;
 var rl,sl,i:integer;
 begin
  if r.ValueType<>bvtOBJECT then begin
   rl:=length(r.Str);
   sl:=length(s);
   if (q+rl)>sl then begin
    result:=false;
   end else begin
    for i:=0 to rl-1 do begin
     if s[q+i+1]<>r.Str[i+1] then begin
      result:=false;
      exit;
     end;
    end;
    Captures[0].s:=q;
    Captures[0].e:=q+rl;
    result:=true;
   end;
  end else begin
   result:=TBESENObjectRegExp(R.Obj).Engine.Match(s,q,Captures);
  end;
 end;
var v,av,r:TBESENValue;
    s,rs:TBESENString;
    a:TBESENObjectArray;
    lim:TBESENUINT32;
    p,sl,CountCaptures,e,q,i:integer;
    Captures:TBESENRegExpCaptures;
    n,Done:boolean;
    ValuePointers:array[0..0] of PBESENValue;
begin
 BESENCheckObjectCoercible(ThisArgument);
 s:=TBESEN(Instance).ToStr(ThisArgument);

 v:=BESENEmptyValue;

 a:=TBESENObjectArray.Create(Instance,TBESEN(Instance).ObjectArrayPrototype,false);
 TBESEN(Instance).GarbageCollector.Add(a);
 a.GarbageCollectorLock;
 try
  ResultValue:=BESENObjectValue(a);

  if (CountArguments<2) or (Arguments^[1]^.ValueType=bvtUNDEFINED) then begin
   lim:=$ffffffff;
  end else begin
   lim:=TBESEN(Instance).ToUInt32(Arguments^[1]^);
  end;

  Done:=false;
  sl:=length(s);
  if (sl=0) and ((TBESEN(Instance).Compatibility and COMPAT_JS)<>0) then begin
   Done:=true;
  end;

  if not Done then begin
   v:=BESENEmptyValue;
   av:=BESENEmptyValue;
   r:=BESENEmptyValue;
   Captures:=nil;
   try
    p:=0;
    if (CountArguments<1) or (Arguments^[0]^.ValueType=bvtUNDEFINED) then begin
     r:=BESENStringValue('undefined');
     CountCaptures:=1;
    end else if (Arguments^[0]^.ValueType=bvtOBJECT) and assigned(Arguments^[0]^.Obj) and (Arguments^[0]^.Obj is TBESENObjectRegExp) then begin
     BESENCopyValue(r,Arguments^[0]^);
     CountCaptures:=TBESENObjectRegExp(Arguments^[0]^.Obj).Engine.CountOfCaptures;
    end else begin
     rs:=TBESEN(Instance).ToStr(Arguments^[0]^);
     if ((TBESEN(Instance).Compatibility and COMPAT_JS)<>0) and (rs=' ') then begin
      av:=BESENStringValue('\s+');
      ValuePointers[0]:=@av;
      TBESEN(Instance).ObjectConstruct(TBESEN(Instance).ObjectRegExpConstructor,BESENObjectValue(TBESEN(Instance).ObjectRegExpConstructor),@ValuePointers,1,r);
      while (p<sl) and BESENUnicodeIsStringWhiteSpace(word(widechar(s[p+1]))) do begin
       inc(p);
      end;
     end else begin
      r:=BESENStringValue(rs);
     end;
     CountCaptures:=1;
    end;
    if CountCaptures<>0 then begin
     SetLength(Captures,CountCaptures);
    end;
    if lim>0 then begin
     if (CountArguments<1) or ((Arguments^[0]^.ValueType=bvtUNDEFINED) and ((TBESEN(Instance).Compatibility and COMPAT_JS)=0)) then begin
      v:=BESENStringValue(s);
      a.Push(v);
     end else if length(s)=0 then begin
      if not SplitMatch(r,s,0,Captures) then begin
       v:=BESENStringValue(s);
       a.Push(v);
      end;
     end else begin
      q:=p;
      while true do begin
       if q=sl then begin
        v:=BESENStringValue(copy(s,p+1,sl-p));
        a.Push(v);
        break;
       end else begin
        if not SplitMatch(r,s,q,Captures) then begin
         inc(q);
         continue;
        end else begin
         e:=Captures[0].e;
         if e=p then begin
          inc(q);
          continue;
         end else begin
          v:=BESENStringValue(copy(s,p+1,q-p));
          a.Push(v);
          if A.Len=lim then begin
           break;
          end else begin
           p:=e;
           i:=0;
           n:=false;
           while true do begin
            if i=(CountCaptures-1) then begin
             q:=p;
             n:=false;
             break;
            end else begin
             inc(i);
             if Captures[i].e=brecUNDEFINED then begin
              v:=BESENUndefinedValue;
             end else begin
              v:=BESENStringValue(copy(s,Captures[i].s+1,Captures[i].e-Captures[i].s));
             end;
             a.Push(v);
             if A.Len=lim then begin
              n:=true;
              break;
             end;
            end;
           end;
           if n then begin
            break;
           end else begin
            continue;
           end;
          end;
         end;
        end;
       end;
      end;
     end;
    end;
   finally
    SetLength(Captures,0);
   end;
  end;
 finally
  a.GarbageCollectorUnlock;
 end;
end;

procedure TBESENObjectStringPrototype.NativeSubstring(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
var v:TBESENValue;
    s:TBESENString;
    a,b,ss,se,sl:integer;
begin
 BESENCheckObjectCoercible(ThisArgument);
 s:=TBESEN(Instance).ToStr(ThisArgument);

 if CountArguments<1 then begin
  a:=0;
 end else begin
  TBESEN(Instance).ToIntegerValue(Arguments^[0]^,v);
  if BESENIsNaN(v.Num) then begin
   a:=0;
  end else begin
   a:=trunc(min(max(v.Num,0),length(s)));
  end;
 end;

 if (CountArguments<2) or (Arguments^[1]^.ValueType=bvtUNDEFINED) then begin
  b:=length(s);
 end else begin
  TBESEN(Instance).ToIntegerValue(Arguments^[1]^,v);
  if BESENIsNaN(v.Num) then begin
   b:=0;
  end else begin
   b:=trunc(min(max(v.Num,0),length(s)));
  end;
 end;

 ss:=min(a,b);
 se:=max(a,b);
 sl:=max(se-ss,0);

 if sl=0 then begin
  ResultValue:=BESENStringValue('');
 end else begin
  ResultValue:=BESENStringValue(copy(s,ss+1,sl));
 end;
end;

procedure TBESENObjectStringPrototype.NativeToLowerCase(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 BESENCheckObjectCoercible(ThisArgument);
 ResultValue:=BESENStringValue(BESENLowercase(TBESEN(Instance).ToStr(ThisArgument)));
end;

procedure TBESENObjectStringPrototype.NativeToLocaleLowerCase(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 BESENCheckObjectCoercible(ThisArgument);
 ResultValue:=BESENStringValue(BESENLowercase(TBESEN(Instance).ToStr(ThisArgument)));
end;

procedure TBESENObjectStringPrototype.NativeToUpperCase(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 BESENCheckObjectCoercible(ThisArgument);
 ResultValue:=BESENStringValue(BESENUppercase(TBESEN(Instance).ToStr(ThisArgument)));
end;

procedure TBESENObjectStringPrototype.NativeToLocaleUpperCase(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 BESENCheckObjectCoercible(ThisArgument);
 ResultValue:=BESENStringValue(BESENUppercase(TBESEN(Instance).ToStr(ThisArgument)));
end;

procedure TBESENObjectStringPrototype.NativeTrim(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
var s:TBESENString;
    StartPosition,LengthCount:integer;
begin
 BESENCheckObjectCoercible(ThisArgument);
 s:=TBESEN(Instance).ToStr(ThisArgument);
 LengthCount:=length(s);
 if LengthCount>0 then begin
  while (LengthCount>0) and BESENUnicodeIsStringWhiteSpace(word(widechar(s[LengthCount]))) do begin
   dec(LengthCount);
  end;
  StartPosition:=1;
  while (StartPosition<=LengthCount) and BESENUnicodeIsStringWhiteSpace(word(widechar(s[StartPosition]))) do begin
   inc(StartPosition);
  end;
  s:=copy(s,StartPosition,LengthCount-StartPosition+1);
 end;
 ResultValue:=BESENStringValue(s);
end;

procedure TBESENObjectStringPrototype.NativeSubstr(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
var v:TBESENValue;
    s:TBESENString;
    ss,sl:integer;
begin
 BESENCheckObjectCoercible(ThisArgument);
 s:=TBESEN(Instance).ToStr(ThisArgument);

 if CountArguments<1 then begin
  v:=BESENNumberValue(0);
 end else begin
  TBESEN(Instance).ToIntegerValue(Arguments^[0]^,v);
 end;
 if BESENIsNegative(v.Num) then begin
  ss:=trunc(max(v.Num+length(s),0));
 end else begin
  ss:=trunc(min(v.Num,length(s)));
 end;

 if (CountArguments<2) or (Arguments^[1]^.ValueType=bvtUNDEFINED) then begin
  sl:=length(s)-ss;
 end else begin
  sl:=min(TBESEN(Instance).ToInt(Arguments^[1]^),length(s)-ss);
 end;

 if sl=0 then begin
  ResultValue:=BESENStringValue('');
 end else begin
  ResultValue:=BESENStringValue(copy(s,ss+1,sl));
 end;
end;

procedure TBESENObjectStringPrototype.NativeAnchor(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 if not ((ThisArgument.ValueType=bvtOBJECT) and assigned(TBESENObject(ThisArgument.Obj))) then begin
  raise EBESENTypeError.Create('Null this object');
 end;
 ResultValue:=BESENStringValue('');
end;

procedure TBESENObjectStringPrototype.NativeBig(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 if not ((ThisArgument.ValueType=bvtOBJECT) and assigned(TBESENObject(ThisArgument.Obj))) then begin
  raise EBESENTypeError.Create('Null this object');
 end;
 ResultValue:=BESENStringValue('');
end;

procedure TBESENObjectStringPrototype.NativeBlink(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 if not ((ThisArgument.ValueType=bvtOBJECT) and assigned(TBESENObject(ThisArgument.Obj))) then begin
  raise EBESENTypeError.Create('Null this object');
 end;
 ResultValue:=BESENStringValue('');
end;

procedure TBESENObjectStringPrototype.NativeBold(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 if not ((ThisArgument.ValueType=bvtOBJECT) and assigned(TBESENObject(ThisArgument.Obj))) then begin
  raise EBESENTypeError.Create('Null this object');
 end;
 ResultValue:=BESENStringValue('');
end;

procedure TBESENObjectStringPrototype.NativeFixed(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 if not ((ThisArgument.ValueType=bvtOBJECT) and assigned(TBESENObject(ThisArgument.Obj))) then begin
  raise EBESENTypeError.Create('Null this object');
 end;
 ResultValue:=BESENStringValue('');
end;

procedure TBESENObjectStringPrototype.NativeFontColor(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 if not ((ThisArgument.ValueType=bvtOBJECT) and assigned(TBESENObject(ThisArgument.Obj))) then begin
  raise EBESENTypeError.Create('Null this object');
 end;
 ResultValue:=BESENStringValue('');
end;

procedure TBESENObjectStringPrototype.NativeFontSize(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 if not ((ThisArgument.ValueType=bvtOBJECT) and assigned(TBESENObject(ThisArgument.Obj))) then begin
  raise EBESENTypeError.Create('Null this object');
 end;
 ResultValue:=BESENStringValue('');
end;

procedure TBESENObjectStringPrototype.NativeItalics(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 if not ((ThisArgument.ValueType=bvtOBJECT) and assigned(TBESENObject(ThisArgument.Obj))) then begin
  raise EBESENTypeError.Create('Null this object');
 end;
 ResultValue:=BESENStringValue('');
end;

procedure TBESENObjectStringPrototype.NativeLink(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 if not ((ThisArgument.ValueType=bvtOBJECT) and assigned(TBESENObject(ThisArgument.Obj))) then begin
  raise EBESENTypeError.Create('Null this object');
 end;
 ResultValue:=BESENStringValue('');
end;

procedure TBESENObjectStringPrototype.NativeSmall(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 if not ((ThisArgument.ValueType=bvtOBJECT) and assigned(TBESENObject(ThisArgument.Obj))) then begin
  raise EBESENTypeError.Create('Null this object');
 end;
 ResultValue:=BESENStringValue('');
end;

procedure TBESENObjectStringPrototype.NativeStrike(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 if not ((ThisArgument.ValueType=bvtOBJECT) and assigned(TBESENObject(ThisArgument.Obj))) then begin
  raise EBESENTypeError.Create('Null this object');
 end;
 ResultValue:=BESENStringValue('');
end;

procedure TBESENObjectStringPrototype.NativeSub(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 if not ((ThisArgument.ValueType=bvtOBJECT) and assigned(TBESENObject(ThisArgument.Obj))) then begin
  raise EBESENTypeError.Create('Null this object');
 end;
 ResultValue:=BESENStringValue('');
end;

procedure TBESENObjectStringPrototype.NativeSup(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 if not ((ThisArgument.ValueType=bvtOBJECT) and assigned(TBESENObject(ThisArgument.Obj))) then begin
  raise EBESENTypeError.Create('Null this object');
 end;
 ResultValue:=BESENStringValue('');
end;

end.
