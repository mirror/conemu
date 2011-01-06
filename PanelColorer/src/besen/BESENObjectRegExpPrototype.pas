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
unit BESENObjectRegExpPrototype;
{$i BESEN.inc}

interface

uses Math,BESENConstants,BESENTypes,BESENObject,BESENObjectRegExp,BESENValue,BESENObjectPropertyDescriptor,
     BESENRegExp;

type TBESENObjectRegExpPrototype=class(TBESENObjectRegExp)
      public
       constructor Create(AInstance:TObject;APrototype:TBESENObject=nil;AHasPrototypeProperty:longbool=false); overload; override;
       destructor Destroy; override;
       procedure NativeToString(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeTest(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeExec(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
     end;

implementation

uses BESEN,BESENErrors,BESENNumberUtils,BESENArrayUtils,BESENObjectArray;

constructor TBESENObjectRegExpPrototype.Create(AInstance:TObject;APrototype:TBESENObject=nil;AHasPrototypeProperty:longbool=false);
begin
 inherited Create(AInstance,APrototype,AHasPrototypeProperty);
 ObjectClassName:='RegExp';
 ObjectName:='RegExp';

 if (TBESEN(Instance).Compatibility and COMPAT_JS)<>0 then begin
  OverwriteData('name',BESENStringValue(ObjectName),[]);
 end;

 RegisterNativeFunction('toString',NativeToString,0,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('test',NativeTest,1,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('exec',NativeExec,1,[bopaWRITABLE,bopaCONFIGURABLE],false);
end;

destructor TBESENObjectRegExpPrototype.Destroy;
begin
 inherited Destroy;
end;

procedure TBESENObjectRegExpPrototype.NativeToString(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
var s:TBESENString;
    i:integer;
    c:widechar;
begin
 if ((TBESEN(Instance).Compatibility and COMPAT_JS)<>0) and (((ThisArgument.ValueType=bvtOBJECT) and assigned(TBESENObject(ThisArgument.Obj)))and (TBESENObject(ThisArgument.Obj) is TBESENObjectRegExpPrototype)) then begin
  ResultValue:=BESENStringValue('RegExp.prototype');
 end else if ((ThisArgument.ValueType=bvtOBJECT) and assigned(TBESENObject(ThisArgument.Obj))) and (TBESENObject(ThisArgument.Obj) is TBESENObjectRegExp) then begin
  s:='/';
  i:=1;
  while i<=length(TBESENObjectRegExp(TBESENObject(ThisArgument.Obj)).Engine.Source) do begin
   c:=TBESENObjectRegExp(TBESENObject(ThisArgument.Obj)).Engine.Source[i];
   case c of
    '/':begin
     s:=s+'\/';
     inc(i);
    end;
    '\':begin
     s:=s+'\\';
     inc(i);
     if i<=length(TBESENObjectRegExp(TBESENObject(ThisArgument.Obj)).Engine.Source) then begin
      c:=TBESENObjectRegExp(TBESENObject(ThisArgument.Obj)).Engine.Source[i];
      s:=s+c;
      inc(i);
     end;
    end;
    else begin
     s:=s+c;
     inc(i);
    end;
   end;
  end;
  s:=s+'/';
  if brefGLOBAL in TBESENObjectRegExp(TBESENObject(ThisArgument.Obj)).Engine.Flags then begin
   s:=s+'g';
  end;
  if brefIGNORECASE in TBESENObjectRegExp(TBESENObject(ThisArgument.Obj)).Engine.Flags then begin
   s:=s+'i';
  end;
  if brefMULTILINE in TBESENObjectRegExp(TBESENObject(ThisArgument.Obj)).Engine.Flags then begin
   s:=s+'m';
  end;
  ResultValue:=BESENStringValue(s);
 end else begin
  raise EBESENTypeError.Create('Not a RegExp object');
 end;
end;

procedure TBESENObjectRegExpPrototype.NativeTest(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
var v,vo,vs:TBESENValue;
    ValuePointers:array[0..0] of PBESENValue;
begin
 Get('exec',v);
 TBESEN(Instance).ToObjectValue(v,vo);
 if (vo.ValueType<>bvtOBJECT) or not (assigned(TBESENObject(vo.Obj)) and TBESENObject(vo.Obj).HasCall) then begin
  raise EBESENTypeError.Create('No callable');
 end;
 TBESENObject(vo.Obj).GarbageCollectorLock;
 try
  if CountArguments<1 then begin
   ValuePointers[0]:=@BESENUndefinedValue;
  end else begin
   ValuePointers[0]:=Arguments^[0];
  end;
  TBESEN(Instance).ObjectCall(TBESENObject(vo.Obj),ThisArgument,@ValuePointers,1,vs);
 finally
  TBESENObject(vo.Obj).GarbageCollectorUnlock;
 end;
 ResultValue.ValueType:=bvtBOOLEAN;
 ResultValue.Bool:=TBESEN(Instance).EqualityExpressionCompare(vs,BESENNullValue)<>0;
end;

procedure TBESENObjectRegExpPrototype.NativeExec(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
var v,vi:TBESENValue;
    i:integer;
    s:TBESENString;
    Captures:TBESENRegExpCaptures;
    o:TBESENObjectArray;
begin
 if not ((ThisArgument.ValueType=bvtOBJECT) and assigned(TBESENObject(ThisArgument.Obj))) then begin
  raise EBESENTypeError.Create('Null this object');
 end;
 if not (TBESENObject(ThisArgument.Obj) is TBESENObjectRegExp) then begin
  raise EBESENTypeError.Create('Not a RegExp object');
 end;
 if CountArguments<1 then begin
  raise EBESENRangeError.Create('Bad argument count');
 end;

 s:=TBESEN(Instance).ToStr(Arguments^[0]^);

 i:=0;

 try
  TBESENObject(ThisArgument.Obj).Get('lastIndex',v);
  TBESEN(Instance).ToNumberValue(v,vi);
  if not (brefGLOBAL in TBESENObjectRegExp(TBESENObject(ThisArgument.Obj)).Engine.Flags) then begin
   v:=BESENNumberValue(0);
  end;
  if (not BESENIsFinite(v.Num)) or (v.Num<0) or (v.Num>length(s)) then begin
   TBESENObject(ThisArgument.Obj).Put('lastIndex',BESENNumberValue(0),true);
   ResultValue:=BESENNullValue;
   exit;
  end;
  i:=trunc(vi.Num);
 except
 end;

{$ifdef UseAssert}
 Assert(TBESENObjectRegExp(TBESENObject(ThisArgument.Obj)).Engine.CountOfCaptures>0);
{$endif}
 Captures:=nil;
 try
  SetLength(Captures,TBESENObjectRegExp(TBESENObject(ThisArgument.Obj)).Engine.CountOfCaptures);
  while not TBESENObjectRegExp(TBESENObject(ThisArgument.Obj)).Engine.Match(s,i,Captures) do begin
   inc(i);
   if i>length(s) then begin
    TBESENObject(ThisArgument.Obj).Put('lastIndex',BESENNumberValue(0),true);
    ResultValue:=BESENNullValue;
    for i:=0 to length(Captures)-1 do begin
     Captures[i].e:=brecUNDEFINED;
    end;
    TBESENObjectRegExp(TBESENObject(ThisArgument.Obj)).SetStatic(s,Captures);
    SetLength(Captures,0);
    exit;
   end;
  end;
  TBESENObjectRegExp(TBESENObject(ThisArgument.Obj)).SetStatic(s,Captures);
  if brefGLOBAL in TBESENObjectRegExp(TBESENObject(ThisArgument.Obj)).Engine.Flags then begin
   TBESENObject(ThisArgument.Obj).Put('lastIndex',BESENNumberValue(Captures[0].e),true);
  end;
  v:=BESENEmptyValue;
  o:=TBESENObjectArray.Create(Instance,TBESEN(Instance).ObjectArrayPrototype,false);
  TBESEN(Instance).GarbageCollector.Add(o);
  o.GarbageCollectorLock;
  try
   for i:=0 to length(Captures)-1 do begin
    if Captures[i].e=brecUNDEFINED then begin
     v:=BESENUndefinedValue;
    end else begin
     v:=BESENStringValue(copy(s,Captures[i].s+1,Captures[i].e-Captures[i].s));
    end;
    o.DefineOwnProperty(BESENArrayIndexToStr(i),BESENDataPropertyDescriptor(v,[bopaWRITABLE,bopaENUMERABLE,bopaCONFIGURABLE]),true);
   end;
   o.Len:=length(Captures);
   o.DefineOwnProperty('index',BESENDataPropertyDescriptor(BESENNumberValue(Captures[0].s),[bopaWRITABLE,bopaENUMERABLE,bopaCONFIGURABLE]),true);
   o.DefineOwnProperty('input',BESENDataPropertyDescriptor(BESENStringValue(s),[bopaWRITABLE,bopaENUMERABLE,bopaCONFIGURABLE]),true);
  finally
   o.GarbageCollectorUnlock;
  end;
  ResultValue:=BESENObjectValue(o);
 finally
  SetLength(Captures,0);
 end;
end;

end.
