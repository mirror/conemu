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
unit BESENObjectConstructor;
{$i BESEN.inc}

interface

uses BESENConstants,BESENTypes,BESENObject,BESENObjectFunction,BESENValue,BESENObjectPropertyDescriptor;

type TBESENObjectConstructor=class(TBESENObjectFunction)
      public
       constructor Create(AInstance:TObject;APrototype:TBESENObject=nil;AHasPrototypeProperty:longbool=false); overload; override;
       destructor Destroy; override;
       procedure Construct(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var AResult:TBESENValue); override;
       procedure Call(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var AResult:TBESENValue); override;
       function HasConstruct:TBESENBoolean; override;
       function HasCall:TBESENBoolean; override;
       procedure NativeGetPrototypeOf(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeGetOwnPropertyDescriptor(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeGetOwnPropertyNames(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeCreate(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeDefineProperty(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeDefineProperties(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeSeal(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeFreeze(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativePreventExtensions(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeIsSealed(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeIsFrozen(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeIsExtensible(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeKeys(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
     end;

implementation

uses BESEN,BESENUtils,BESENArrayUtils,BESENErrors,BESENObjectString,BESENObjectArray;

constructor TBESENObjectConstructor.Create(AInstance:TObject;APrototype:TBESENObject=nil;AHasPrototypeProperty:longbool=false);
begin
 inherited Create(AInstance,APrototype,AHasPrototypeProperty);
 ObjectClassName:='Function';

 RegisterNativeFunction('getPrototypeOf',NativeGetPrototypeOf,1,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('getOwnPropertyDescriptor',NativeGetOwnPropertyDescriptor,2,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('getOwnPropertyNames',NativeGetOwnPropertyNames,1,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('create',NativeCreate,2,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('defineProperty',NativeDefineProperty,3,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('defineProperties',NativeDefineProperties,2,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('seal',NativeSeal,1,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('freeze',NativeFreeze,1,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('preventExtensions',NativePreventExtensions,1,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('isSealed',NativeIsSealed,1,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('isFrozen',NativeIsFrozen,1,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('isExtensible',NativeIsExtensible,1,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('keys',NativeKeys,1,[bopaWRITABLE,bopaCONFIGURABLE],false);
end;

destructor TBESENObjectConstructor.Destroy;
begin
 inherited Destroy;
end;

procedure TBESENObjectConstructor.Construct(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var AResult:TBESENValue);
begin
 if CountArguments>0 then begin
  case Arguments^[0]^.ValueType of
   bvtNULL,bvtUNDEFINED:begin
    AResult.ValueType:=bvtOBJECT;
    AResult.Obj:=TBESENObject.Create(Instance,TBESEN(Instance).ObjectPrototype);
   end;
   else begin
    TBESEN(Instance).ToObjectValue(Arguments^[0]^,AResult);
   end;
  end;
 end else begin
  AResult.ValueType:=bvtOBJECT;
  AResult.Obj:=TBESENObject.Create(Instance,TBESEN(Instance).ObjectPrototype);
 end;
 if (AResult.ValueType=bvtOBJECT) and assigned(AResult.Obj) then begin
  TBESEN(Instance).GarbageCollector.Add(TBESENObject(AResult.Obj));
 end;
end;

procedure TBESENObjectConstructor.Call(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var AResult:TBESENValue);
begin
 Construct(ThisArgument,Arguments,CountArguments,AResult);
end;

function TBESENObjectConstructor.HasConstruct:TBESENBoolean;
begin
 result:=true;
end;

function TBESENObjectConstructor.HasCall:TBESENBoolean;
begin
 result:=true;
end;

procedure TBESENObjectConstructor.NativeGetPrototypeOf(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 if not ((CountArguments>0) and (Arguments^[0]^.ValueType=bvtOBJECT) and assigned(Arguments^[0]^.Obj)) then begin
  raise EBESENTypeError.Create('No object');
 end;
 ResultValue:=BESENObjectValueEx(TBESENObject(Arguments^[0]^.Obj).Prototype);
end;

procedure TBESENObjectConstructor.NativeGetOwnPropertyDescriptor(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
var n:TBESENString;
    Descriptor:TBESENObjectPropertyDescriptor;
begin
 if not ((CountArguments>0) and (Arguments^[0]^.ValueType=bvtOBJECT) and assigned(Arguments^[0]^.Obj)) then begin
  raise EBESENTypeError.Create('No object');
 end;
 if CountArguments>1 then begin
  n:=TBESEN(Instance).ToStr(Arguments^[1]^);
 end else begin
  n:='';
 end;
 TBESENObject(Arguments^[0]^.Obj).GetOwnProperty(n,Descriptor);
 TBESEN(Instance).FromPropertyDescriptor(Descriptor,ResultValue);
 if (ResultValue.ValueType=bvtOBJECT) and assigned(ResultValue.Obj) then begin
  TBESEN(Instance).GarbageCollector.Add(TBESENObject(ResultValue.Obj));
 end;
end;

procedure TBESENObjectConstructor.NativeGetOwnPropertyNames(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
var ArrayObject:TBESENObjectArray;
    o:TBESENObject;
    PropItem:TBESENObjectProperty;
    n:longword;
begin
 if not ((CountArguments>0) and (Arguments^[0]^.ValueType=bvtOBJECT) and assigned(Arguments^[0]^.Obj)) then begin
  raise EBESENTypeError.Create('No object');
 end;
 ArrayObject:=TBESENObjectArray.Create(Instance,TBESEN(Instance).ObjectArrayPrototype,false);
 TBESEN(Instance).GarbageCollector.Add(ArrayObject);
 ArrayObject.GarbageCollectorLock;
 try
  o:=TBESENObject(Arguments^[0]^.Obj);
  if o is TBESENObjectString then begin
   for n:=1 to length(TBESENObjectString(o).Value) do begin
    ArrayObject.DefineOwnProperty(BESENArrayIndexToStr(n-1),BESENDataPropertyDescriptor(BESENStringValue(BESENArrayIndexToStr(n-1)),[bopaWRITABLE,bopaENUMERABLE,bopaCONFIGURABLE]),false);
   end;
   n:=length(TBESENObjectString(o).Value);
  end else begin
   n:=0;
  end;
  PropItem:=o.Properties.First;
  while assigned(PropItem) do begin
   ArrayObject.DefineOwnProperty(BESENArrayIndexToStr(n),BESENDataPropertyDescriptor(BESENStringValue(PropItem.Key),[bopaWRITABLE,bopaENUMERABLE,bopaCONFIGURABLE]),false);
   inc(n);
   PropItem:=PropItem.Next;
  end;
  ArrayObject.Len:=n;
 finally
  ArrayObject.GarbageCollectorUnlock;
 end;
 ResultValue:=BESENObjectValue(ArrayObject);
end;

procedure TBESENObjectConstructor.NativeCreate(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
var o:TBESENObject;
    vo:TBESENValue;
    ValuePointers:array[0..1] of PBESENValue;
begin
 if not ((CountArguments>0) and ((Arguments^[0]^.ValueType=bvtNULL) or ((Arguments^[0]^.ValueType=bvtOBJECT) and assigned(Arguments^[0]^.Obj)))) then begin
  raise EBESENTypeError.Create('No object and not null');
 end;
 if Arguments^[0]^.ValueType=bvtNULL then begin
  o:=TBESENObject.Create(Instance,nil);
 end else begin
  o:=TBESENObject.Create(Instance,TBESENObject(Arguments^[0]^.Obj));
 end;
 if CountArguments>1 then begin
  vo.ValueType:=bvtOBJECT;
  TBESENObject(vo.Obj):=o;
  ValuePointers[0]:=@vo;
  ValuePointers[1]:=Arguments^[1];
  NativeDefineProperties(ThisArgument,@ValuePointers,CountArguments,ResultValue);
 end;
 ResultValue.ValueType:=bvtOBJECT;
 ResultValue.Obj:=o;
end;

procedure TBESENObjectConstructor.NativeDefineProperty(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
var n:TBESENString;
    Descriptor:TBESENObjectPropertyDescriptor;
begin
 if not ((CountArguments>0) and (Arguments^[0]^.ValueType=bvtOBJECT) and assigned(Arguments^[0]^.Obj)) then begin
  raise EBESENTypeError.Create('No object');
 end;
 if CountArguments>1 then begin
  n:=TBESEN(Instance).ToStr(Arguments^[1]^);
 end else begin
  n:='';
 end;
 if CountArguments>2 then begin
  TBESEN(Instance).ToPropertyDescriptor(Arguments^[2]^,Descriptor);
 end else begin
  Descriptor:=BESENUndefinedPropertyDescriptor;
 end;
 TBESENObject(Arguments^[0]^.Obj).DefineOwnProperty(n,Descriptor,true);
 BESENCopyValue(ResultValue,Arguments^[0]^);
end;

procedure TBESENObjectConstructor.NativeDefineProperties(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
var Props:TBESENObject;
    Names:TBESENStrings;
    Descriptors:TBESENObjectPropertyDescriptors;
    Enumerator:TBESENObjectPropertyEnumerator;
    i,Count:integer;
    v:TBESENValue;
    Key:TBESENString;
begin
 Names:=nil;
 Descriptors:=nil;
 Enumerator:=nil;
 Key:='';
 if not ((CountArguments>0) and (Arguments^[0]^.ValueType=bvtOBJECT) and assigned(Arguments^[0]^.Obj)) then begin
  raise EBESENTypeError.Create('No object');
 end;
 try
  if CountArguments>1 then begin
   Props:=TBESEN(Instance).ToObj(Arguments^[1]^);
  end else begin
   Props:=TBESEN(Instance).ToObj(BESENUndefinedValue);
  end;
  TBESEN(Instance).GarbageCollector.Add(Props);
  Props.GarbageCollectorLock;
  try
   Count:=0;
   Enumerator:=Props.Enumerator(true,false);
   Enumerator.Reset;
   while Enumerator.GetNext(Key) do begin
    if Count>=length(Names) then begin
     SetLength(Names,Count+256);
    end;
    if Count>=length(Descriptors) then begin
     SetLength(Descriptors,Count+256);
    end;
    Names[Count]:=Key;
    Props.Get(Key,v);
    TBESEN(Instance).ToPropertyDescriptor(v,Descriptors[Count]);
    inc(Count);
   end;
   for i:=0 to Count-1 do begin
    TBESENObject(Arguments^[0]^.Obj).DefineOwnProperty(Names[i],Descriptors[i],true);
   end;
  finally
   Props.GarbageCollectorUnlock;
  end;
 finally
  BESENFreeAndNil(Enumerator);
  SetLength(Names,0);
  SetLength(Descriptors,0);
 end;
 ResultValue:=Arguments^[0]^;
end;


procedure TBESENObjectConstructor.NativeSeal(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
var Descriptor:TBESENObjectPropertyDescriptor;
    Enumerator:TBESENObjectPropertyEnumerator;
    Key:TBESENString;
begin
 if not ((CountArguments>0) and (Arguments^[0]^.ValueType=bvtOBJECT) and assigned(Arguments^[0]^.Obj)) then begin
  raise EBESENTypeError.Create('No object');
 end;
 Enumerator:=nil;
 Key:='';
 try
  Enumerator:=TBESENObject(Arguments^[0]^.Obj).Enumerator(true,true);
  Enumerator.Reset;
  while Enumerator.GetNext(Key) do begin
   TBESENObject(Arguments^[0]^.Obj).GetOwnProperty(Key,Descriptor);
   Descriptor.Attributes:=Descriptor.Attributes-[bopaCONFIGURABLE];
   TBESENObject(Arguments^[0]^.Obj).DefineOwnProperty(Key,Descriptor,true);
  end;
 finally
  BESENFreeAndNil(Enumerator);
 end;
 TBESENObject(Arguments^[0]^.Obj).Extensible:=false;
 BESENCopyValue(ResultValue,Arguments^[0]^);
end;

procedure TBESENObjectConstructor.NativeFreeze(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
var Descriptor:TBESENObjectPropertyDescriptor;
    Enumerator:TBESENObjectPropertyEnumerator;
    Key:TBESENString;
begin
 if not ((CountArguments>0) and (Arguments^[0]^.ValueType=bvtOBJECT) and assigned(TBESENObject(Arguments^[0]^.Obj))) then begin
  raise EBESENTypeError.Create('No object');
 end;
 Enumerator:=nil;
 Key:='';
 try
  Enumerator:=TBESENObject(Arguments^[0]^.Obj).Enumerator(true,true);
  Enumerator.Reset;
  while Enumerator.GetNext(Key) do begin
   TBESENObject(Arguments^[0]^.Obj).GetOwnProperty(Key,Descriptor);
   if ([boppVALUE,boppWRITABLE]*Descriptor.Presents)<>[] then begin
    Descriptor.Attributes:=Descriptor.Attributes-[bopaWRITABLE];
   end;
   Descriptor.Attributes:=Descriptor.Attributes-[bopaCONFIGURABLE];
   TBESENObject(Arguments^[0]^.Obj).DefineOwnProperty(Key,Descriptor,true);
  end;
 finally
  BESENFreeAndNil(Enumerator);
 end;
 TBESENObject(Arguments^[0]^.Obj).Extensible:=false;
 BESENCopyValue(ResultValue,Arguments^[0]^);
end;

procedure TBESENObjectConstructor.NativePreventExtensions(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 if not ((CountArguments>0) and (Arguments^[0]^.ValueType=bvtOBJECT) and assigned(TBESENObject(Arguments^[0]^.Obj))) then begin
  raise EBESENTypeError.Create('No object');
 end;
 TBESENObject(Arguments^[0]^.Obj).Extensible:=false;
 BESENCopyValue(ResultValue,Arguments^[0]^);
end;

procedure TBESENObjectConstructor.NativeIsSealed(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
var Descriptor:TBESENObjectPropertyDescriptor;
    Enumerator:TBESENObjectPropertyEnumerator;
    Key:TBESENString;
    IsSealed:boolean;
begin
 if not ((CountArguments>0) and (Arguments^[0]^.ValueType=bvtOBJECT) and assigned(TBESENObject(Arguments^[0]^.Obj))) then begin
  raise EBESENTypeError.Create('No object');
 end;
 IsSealed:=true;
 Enumerator:=nil;
 Key:='';
 try
  Enumerator:=TBESENObject(Arguments^[0]^.Obj).Enumerator(true,true);
  Enumerator.Reset;
  while Enumerator.GetNext(Key) do begin
   TBESENObject(Arguments^[0]^.Obj).GetOwnProperty(Key,Descriptor);
   if (boppCONFIGURABLE In Descriptor.Presents) and (bopaCONFIGURABLE In Descriptor.Attributes) then begin
    IsSealed:=false;
    break;
   end;
  end;
 finally
  BESENFreeAndNil(Enumerator);
 end;
 if TBESENObject(Arguments^[0]^.Obj).Extensible then begin
  IsSealed:=false;
 end;
 ResultValue.ValueType:=bvtBOOLEAN;
 ResultValue.Bool:=IsSealed;
end;

procedure TBESENObjectConstructor.NativeIsFrozen(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
var Descriptor:TBESENObjectPropertyDescriptor;
    Enumerator:TBESENObjectPropertyEnumerator;
    Key:TBESENString;
    IsFrozen:boolean;
begin
 if not ((CountArguments>0) and (Arguments^[0]^.ValueType=bvtOBJECT) and assigned(TBESENObject(Arguments^[0]^.Obj))) then begin
  raise EBESENTypeError.Create('No object');
 end;
 IsFrozen:=true;
 Enumerator:=nil;
 Key:='';
 try
  Enumerator:=TBESENObject(Arguments^[0]^.Obj).Enumerator(true,true);
  Enumerator.Reset;
  while Enumerator.GetNext(Key) do begin
   TBESENObject(Arguments^[0]^.Obj).GetOwnProperty(Key,Descriptor);
   if ((([boppVALUE,boppWRITABLE]*Descriptor.Presents)<>[]) and ((boppWRITABLE In Descriptor.Presents) and (bopaWRITABLE In Descriptor.Attributes))) or
      ((boppCONFIGURABLE In Descriptor.Presents) and (bopaCONFIGURABLE In Descriptor.Attributes)) then begin
    IsFrozen:=false;
    break;
   end;
  end;
 finally
  BESENFreeAndNil(Enumerator);
 end;
 if TBESENObject(Arguments^[0]^.Obj).Extensible then begin
  IsFrozen:=false;
 end;
 ResultValue.ValueType:=bvtBOOLEAN;
 ResultValue.Bool:=IsFrozen;
end;

procedure TBESENObjectConstructor.NativeIsExtensible(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 if not ((CountArguments>0) and (Arguments^[0]^.ValueType=bvtOBJECT) and assigned(TBESENObject(Arguments^[0]^.Obj))) then begin
  raise EBESENTypeError.Create('No object');
 end;
 ResultValue.ValueType:=bvtBOOLEAN;
 ResultValue.Bool:=TBESENObject(Arguments^[0]^.Obj).Extensible;
end;

procedure TBESENObjectConstructor.NativeKeys(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
var ArrayObject:TBESENObjectArray;
    Enumerator:TBESENObjectPropertyEnumerator;
    Key:TBESENString;
    Index:longword;
begin
 if not ((CountArguments>0) and (Arguments^[0]^.ValueType=bvtOBJECT) and assigned(TBESENObject(Arguments^[0]^.Obj))) then begin
  raise EBESENTypeError.Create('No object');
 end;
 ArrayObject:=TBESENObjectArray.Create(Instance,TBESEN(Instance).ObjectArrayPrototype,false);
 TBESEN(Instance).GarbageCollector.Add(ArrayObject);
 ArrayObject.GarbageCollectorLock;
 try
  Index:=0;
  Enumerator:=nil;
  Key:='';
  try
   Enumerator:=TBESENObject(Arguments^[0]^.Obj).Enumerator(true,false);
   Enumerator.Reset;
   while Enumerator.GetNext(Key) do begin
    ArrayObject.DefineOwnProperty(BESENArrayIndexToStr(Index),BESENDataPropertyDescriptor(BESENStringValue(Key),[bopaWRITABLE,bopaENUMERABLE,bopaCONFIGURABLE]),false);
    inc(Index);
   end;
  finally
   BESENFreeAndNil(Enumerator);
  end;
  ArrayObject.Len:=Index;
 finally
  ArrayObject.GarbageCollectorUnlock;
 end;
 ResultValue:=BESENObjectValue(ArrayObject);
end;

end.
