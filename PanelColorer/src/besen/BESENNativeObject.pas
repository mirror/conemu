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
unit BESENNativeObject;
{$i BESEN.inc}

interface

uses TypInfo,Variants,BESENConstants,BESENTypes,BESENObject,BESENObjectFunction,BESENValue,BESENObjectPropertyDescriptor,
     BESENEnvironmentRecord;

type TBESENNativeObject=class(TBESENObjectFunction)
      private
       PropList:PPropList;
       PropListLen:integer;
       function GetProp(const Prop:TBESENObjectProperty;var V:TBESENValue;Throw:boolean):boolean;
       function PutProp(const Prop:TBESENObjectProperty;const V:TBESENValue;Throw:boolean):boolean;
      protected
       procedure ConstructObject(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer); virtual;
       procedure InitializeObject; virtual;
       procedure FinalizeObject; virtual;
      public
       Initialized:TBESENBoolean;
       constructor Create(AInstance:TObject;APrototype:TBESENObject=nil;AHasPrototypeProperty:longbool=false); overload; override;
       destructor Destroy; override;
       function GetOwnProperty(const P:TBESENString;var Descriptor:TBESENObjectPropertyDescriptor;Hash:TBESENHash=0):boolean; override;
       function GetEx(const P:TBESENString;var AResult:TBESENValue;var Descriptor:TBESENObjectPropertyDescriptor;Base:TBESENObject=nil;Hash:TBESENHash=0):boolean; override;
       function GetIndex(const Index,ID:integer;var AResult:TBESENValue;Base:TBESENObject=nil):boolean; override;
       procedure PutEx(const P:TBESENString;const V:TBESENValue;Throw:TBESENBoolean;var Descriptor,OwnDescriptor:TBESENObjectPropertyDescriptor;var TempValue:TBESENValue;Hash:TBESENHash=0); override;
       procedure PutIndex(const Index,ID:integer;const V:TBESENValue;Throw:TBESENBoolean); override;
       function HasPropertyEx(const P:TBESENString;var Descriptor:TBESENObjectPropertyDescriptor;Hash:TBESENHash=0):TBESENBoolean; override;
       function DeleteEx(const P:TBESENString;Throw:TBESENBoolean;var Descriptor:TBESENObjectPropertyDescriptor;Hash:TBESENHash=0):TBESENBoolean; override;
       function DefineOwnPropertyEx(const P:TBESENString;const Descriptor:TBESENObjectPropertyDescriptor;Throw:TBESENBoolean;var Current:TBESENObjectPropertyDescriptor;Hash:TBESENHash=0):TBESENBoolean; override;
       procedure Construct(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var AResult:TBESENValue); override;
       procedure Call(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var AResult:TBESENValue); override;
       function HasConstruct:TBESENBoolean; override;
       function HasCall:TBESENBoolean; override;
       procedure Finalize; override;
       procedure Mark; override;
     end;

     TBESENNativeObjectClass=class of TBESENNativeObject;

implementation

uses BESEN,BESENErrors,BESENHashUtils,BESENStringUtils,BESENGarbageCollector;

constructor TBESENNativeObject.Create(AInstance:TObject;APrototype:TBESENObject=nil;AHasPrototypeProperty:longbool=false);
begin
 inherited Create(AInstance,APrototype,AHasPrototypeProperty);
 Initialized:=false;
 PropList:=nil;
 PropListLen:=0;
 ObjectClassName:=ClassName+'$Constructor';
end;

destructor TBESENNativeObject.Destroy;
begin
 if Initialized then begin
  FinalizeObject;
 end;
 if assigned(PropList) then begin
  FreeMem(PropList);
  PropList:=nil;
  PropListLen:=0;
 end;
 inherited Destroy;
end;

procedure TBESENNativeObject.ConstructObject(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer);
begin
end;

procedure TBESENNativeObject.InitializeObject;
type PShortString=^ShortString;
     PMethodNameRec=^TMethodNameRec;
     TMethodNameRec=packed record
{$ifdef fpc}
      Name:PShortString;
      Address:pointer;
{$else}
      Size:word;
      Address:pointer;
      Name:ShortString;
{$endif}
     end;
     TMethodNameRecs=packed array[word] of TMethodNameRec;
     PMethodNameTable=^TMethodNameTable;
     TMethodNameTable=packed record
      Count:{$ifdef fpc}longword{$else}word{$endif};
      Methods:TMethodNameRecs;
     end;
var i:integer;
    Prop:TBESENObjectProperty;
    Item:PPropInfo;
    MethodTable:PMethodNameTable;
    MethodNameRec:PMethodNameRec;
    MethodName:TBESENANSISTRING;
    MethodAddress:pointer;
    NativeFunction:TBESENNativeFunction;
begin
 if not Initialized then begin
  Initialized:=true;

  ObjectClassName:=ClassName+'$Instance';

  try
   MethodTable:=pointer(pointer(ptrint(ptrint(pointer(self)^)+vmtMethodTable))^);
   if assigned(MethodTable) then begin
    MethodNameRec:=@MethodTable^.Methods[0];
    for i:=0 to MethodTable^.Count-1 do begin
{$ifdef fpc}
     MethodName:=MethodNameRec^.Name^;
{$else}
     MethodName:=MethodNameRec^.Name;
{$endif}
     MethodAddress:=MethodNameRec^.Address;
     if (length(MethodName)>0) and assigned(MethodAddress) then begin
      TMethod(NativeFunction).Code:=MethodAddress;
      TMethod(NativeFunction).Data:=self;
      RegisterNativeFunction(MethodName,NativeFunction,0,[],false);
     end;
{$ifdef fpc}
     inc(MethodNameRec);
{$else}
     inc(ptruint(MethodNameRec),MethodNameRec^.Size);
{$endif}
    end;
   end;
  except
   raise;
  end;

  PropListLen:=GetPropList(self,PropList);
  try
   for i:=0 to PropListLen-1 do begin
    Item:=PropList^[i];
    if assigned(Item^.PropType) then begin
     case Item^.PropType^.Kind of
      tkLString,tkWString{$ifdef fpc},tkAString{$endif},tkVariant{$ifdef fpc},tkUString{$endif},
      tkInteger,tkChar,tkFloat,tkEnumeration,tkWChar,tkSet{$ifdef fpc},tkSString{$endif},
      tkInt64{$ifdef fpc},tkQWORD{$endif},tkClass:begin
       if not assigned(Item^.GetProc) then begin
        BESENThrowNotReadable(Item^.Name);
       end;
       Prop:=Properties.Get(Item^.Name);
       if not assigned(Prop) then begin
        Prop:=Properties.Put(Item^.Name);
       end;
       Prop.PropIndex:=i;
       Prop.Descriptor.Value.ValueType:=bvtUNDEFINED;
       Prop.Descriptor.Getter:=nil;
       Prop.Descriptor.Setter:=nil;
       if assigned(Item^.SetProc) then begin
        Prop.Descriptor.Attributes:=[bopaWRITABLE,bopaENUMERABLE];
       end else begin
        Prop.Descriptor.Attributes:=[bopaENUMERABLE];
       end;
       Prop.Descriptor.Presents:=[boppVALUE,boppWRITABLE,boppENUMERABLE,boppCONFIGURABLE];
      end;
     end;
    end;
   end;
  except
   FreeMem(PropList);
   PropList:=nil;
   PropListLen:=0;
   raise;
  end;
  
  InvalidateStructure;
 end;
end;

procedure TBESENNativeObject.FinalizeObject;
begin
 if Initialized then begin
  Initialized:=false;
  if assigned(PropList) then begin
   FreeMem(PropList);
   PropList:=nil;
   PropListLen:=0;
  end;
 end;
end;

function TBESENNativeObject.GetProp(const Prop:TBESENObjectProperty;var V:TBESENValue;Throw:boolean):boolean;
var Item:PPropInfo;
    O:TObject;
    Index:integer;
begin                      
 result:=false;
 if not Initialized then begin
  exit;
 end;

 Index:=Prop.PropIndex;
 if (Index>=0) and (Index<PropListLen) then begin
  Item:=PropList^[Index];
 end else begin
  exit;
 end;

 if not assigned(Item^.GetProc) then begin
  if Throw then begin
   BESENThrowNotReadable(Item^.Name);
  end;
  exit;
 end;

 case Item^.PropType^.Kind of
  tkLString{$ifdef fpc},tkAString,tkSString{$endif}:begin
   V.ValueType:=bvtSTRING;
   V.Str:=BESENConvertToUTF8(GetStrProp(self,Item));
  end;
  tkWString{$ifdef fpc},tkUString{$endif}:begin
   V.ValueType:=bvtSTRING;
   V.Str:=GetWideStrProp(self,Item);
  end;
  tkEnumeration:begin
   V.ValueType:=bvtSTRING;
   V.Str:=GetEnumProp(self,Item);
  end;
  tkSet:begin
   V.ValueType:=bvtSTRING;
   V.Str:=GetSetProp(self,Item,false);
  end;
  tkInteger:begin
   V.ValueType:=bvtNUMBER;
   V.Num:=GetOrdProp(self,Item);
  end;
  tkChar:begin
   V.ValueType:=bvtSTRING;
   V.Str:=TBESENCHAR(byte(GetOrdProp(self,Item)));
  end;
  tkWChar:begin
   V.ValueType:=bvtSTRING;
   V.Str:=widechar(word(GetOrdProp(self,Item)));
  end;
  tkInt64{$ifdef fpc},tkQWORD{$endif}:begin
   V.ValueType:=bvtNUMBER;
   V.Num:=GetInt64Prop(self,Item);
  end;
  tkFloat:begin
   V.ValueType:=bvtNUMBER;
   V.Num:=GetFloatProp(self,Item);
  end;
  tkClass:begin
   O:=GetObjectProp(self,Item);
   if assigned(O) then begin
    if O is TBESENObject then begin
     V.ValueType:=bvtOBJECT;
     TBESENObject(v.Obj):=TBESENObject(o);
    end else begin
     V.ValueType:=bvtUNDEFINED;
    end;
   end else begin
    V.ValueType:=bvtNULL;
   end;
  end;
  tkVariant:begin
   BESENVariantToValue(GetVariantProp(self,Item),V);
  end;
  else begin
   if Throw then begin
    BESENThrowTypeError('Unknown native property data type');
   end;
   exit;
  end;
 end;

 result:=true;
end;

function TBESENNativeObject.PutProp(const Prop:TBESENObjectProperty;const V:TBESENValue;Throw:boolean):boolean;
var Item:PPropInfo;
    Index:integer;
begin
 result:=false;
 if not Initialized then begin
  exit;
 end;

 Index:=Prop.PropIndex;
 if (Index>=0) and (Index<PropListLen) then begin
  Item:=PropList^[Index];
 end else begin
  exit;
 end;

 if not (assigned(Item^.SetProc) and ((boppWRITABLE in Prop.Descriptor.Presents) and (bopaWRITABLE in Prop.Descriptor.Attributes))) then begin
  if Throw then begin
   BESENThrowNotWritable(Item^.Name);
  end;
  exit;
 end;

 case Item^.PropType^.Kind of
  tkLString{$ifdef fpc},tkAString,tkSString{$endif}:begin
   SetStrProp(self,Item,BESENUTF16ToUTF8(TBESEN(Instance).ToStr(V)));
  end;
  tkWString{$ifdef fpc},tkUString{$endif}:begin
   SetWideStrProp(self,Item,TBESEN(Instance).ToStr(V));
  end;
  tkEnumeration:begin
   SetEnumProp(self,Item,TBESEN(Instance).ToStr(V));
  end;
  tkSet:begin
   SetSetProp(self,Item,TBESEN(Instance).ToStr(V));
  end;
  tkInteger:begin
   SetOrdProp(self,Item,TBESEN(Instance).ToInt32(V));
  end;
  tkChar:begin
   if (V.ValueType=bvtSTRING) and (length(V.Str)>0) then begin
    SetOrdProp(self,Item,byte(word(widechar(V.Str[1]))));
   end else begin
    SetOrdProp(self,Item,TBESEN(Instance).ToInt32(V));
   end;
  end;
  tkWChar:begin
   if (V.ValueType=bvtSTRING) and (length(V.Str)>0) then begin
    SetOrdProp(self,Item,word(widechar(V.Str[1])));
   end else begin
    SetOrdProp(self,Item,TBESEN(Instance).ToInt32(V));
   end;
  end;
  tkInt64{$ifdef fpc},tkQWORD{$endif}:begin
   SetInt64Prop(self,Item,TBESEN(Instance).ToInt(V));
  end;
  tkFloat:begin
   SetFloatProp(self,Item,TBESEN(Instance).ToNum(V));
  end;
  tkClass:begin
   SetObjectProp(self,Item,TBESEN(Instance).ToObj(V));
  end;
  tkVariant:begin
   SetVariantProp(self,Item,BESENValueToVariant(V));
  end;
  else begin
   if Throw then begin
    BESENThrowTypeError('Wrong native property data type');
   end;
   exit;
  end;
 end;

 result:=true;
end;

function TBESENNativeObject.GetOwnProperty(const P:TBESENString;var Descriptor:TBESENObjectPropertyDescriptor;Hash:TBESENHash=0):boolean;
begin
 LastProp:=Properties.Get(P,Hash);
 if assigned(LastProp) and (LastProp.PropIndex>=0) then begin
  if GetProp(LastProp,Descriptor.Value,true) then begin
   Descriptor.Getter:=LastProp.Descriptor.Getter;
   Descriptor.Setter:=LastProp.Descriptor.Setter;
   Descriptor.Attributes:=LastProp.Descriptor.Attributes;
   Descriptor.Presents:=LastProp.Descriptor.Presents;
   result:=true;
  end else begin
   result:=false;
  end;
 end else begin
  result:=inherited GetOwnProperty(P,Descriptor);
 end;
end;

function TBESENNativeObject.GetEx(const P:TBESENString;var AResult:TBESENValue;var Descriptor:TBESENObjectPropertyDescriptor;Base:TBESENObject=nil;Hash:TBESENHash=0):boolean;
begin
 if not assigned(Base) then begin
  Base:=self;
 end;
 LastProp:=Properties.Get(P,Hash);
 if assigned(LastProp) and (LastProp.PropIndex>=0) then begin
  result:=GetProp(LastProp,AResult,true);
 end else begin
  result:=inherited GetEx(P,AResult,Descriptor,Base,Hash);
 end;
end;

function TBESENNativeObject.GetIndex(const Index,ID:integer;var AResult:TBESENValue;Base:TBESENObject=nil):boolean;
begin
 if not assigned(Base) then begin
  Base:=self;
 end;
 if (((Index>=0) and (Index<Properties.ItemCount)) and (Properties.Items[Index].ID=ID)) and (Properties.Items[Index].PropIndex>=0) then begin
  result:=GetProp(Properties.Items[Index],AResult,true);
 end else begin
  result:=Get(TBESEN(Instance).KeyIDManager.List[ID],AResult,Base);
 end;
end;

procedure TBESENNativeObject.PutEx(const P:TBESENString;const V:TBESENValue;Throw:TBESENBoolean;var Descriptor,OwnDescriptor:TBESENObjectPropertyDescriptor;var TempValue:TBESENValue;Hash:TBESENHash=0);
begin
 LastProp:=Properties.Get(P,Hash);
 if assigned(LastProp) and (LastProp.PropIndex>=0) then begin
  PutProp(LastProp,V,Throw);
 end else begin
  inherited PutEx(P,V,Throw,Descriptor,OwnDescriptor,TempValue,Hash);
 end;
end;

procedure TBESENNativeObject.PutIndex(const Index,ID:integer;const V:TBESENValue;Throw:TBESENBoolean);
begin
 if (((Index>=0) and (Index<Properties.ItemCount)) and (Properties.Items[Index].ID=ID)) and (Properties.Items[Index].PropIndex>=0) then begin
  PutProp(Properties.Items[Index],V,Throw);
 end else begin
  Put(TBESEN(Instance).KeyIDManager.List[ID],V,Throw);
 end;
end;

function TBESENNativeObject.HasPropertyEx(const P:TBESENString;var Descriptor:TBESENObjectPropertyDescriptor;Hash:TBESENHash=0):TBESENBoolean;
begin
 LastProp:=Properties.Get(P,Hash);
 if assigned(LastProp) and (LastProp.PropIndex>=0) then begin
  result:=true;
 end else begin
  result:=inherited HasPropertyEx(P,Descriptor,Hash);
 end;
end;

function TBESENNativeObject.DeleteEx(const P:TBESENString;Throw:TBESENBoolean;var Descriptor:TBESENObjectPropertyDescriptor;Hash:TBESENHash=0):TBESENBoolean;
 procedure BESENThrowIt;
 begin
  BESENThrowTypeError('Delete for "'+BESENUTF16ToUTF8(P)+'" failed');
 end;
begin
 LastProp:=Properties.Get(P,Hash);
 if assigned(LastProp) and (LastProp.PropIndex>=0) then begin
  result:=false;
  if Throw then begin
   BESENThrowIt;
  end;
 end else begin
  result:=DeleteEx(P,Throw,Descriptor,Hash);
 end;
end;

function TBESENNativeObject.DefineOwnPropertyEx(const P:TBESENString;const Descriptor:TBESENObjectPropertyDescriptor;Throw:TBESENBoolean;var Current:TBESENObjectPropertyDescriptor;Hash:TBESENHash=0):TBESENBoolean;
begin
 LastProp:=Properties.Get(P,Hash);
 if assigned(LastProp) and (LastProp.PropIndex>=0) then begin
  if (boppVALUE in Descriptor.Presents) and ((boppWRITABLE in Descriptor.Presents) and (bopaWRITABLE in Descriptor.Attributes)) then begin
   PutProp(LastProp,Descriptor.Value,Throw);
   result:=true;
  end else begin
   if Throw then begin
    BESENThrowDefineOwnProperty(P);
   end;
   result:=false;
  end;
 end else begin
  result:=inherited DefineOwnPropertyEx(P,Descriptor,Throw,Current,Hash);
 end;
end;
                                
procedure TBESENNativeObject.Construct(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var AResult:TBESENValue);
begin
 AResult.ValueType:=bvtOBJECT;
 AResult.Obj:=TBESENNativeObjectClass(ClassType).Create(Instance,self,false);
 TBESEN(Instance).GarbageCollector.Add(TBESENObject(AResult.Obj));
 TBESENObject(AResult.Obj).GarbageCollectorLock;
 try
  TBESENNativeObject(AResult.Obj).ObjectClassName:=AResult.Obj.ClassName;
  TBESENNativeObject(AResult.Obj).InitializeObject;
  TBESENNativeObject(AResult.Obj).ConstructObject(ThisArgument,Arguments,CountArguments);
 finally
  TBESENObject(AResult.Obj).GarbageCollectorUnlock;
 end;
end;

procedure TBESENNativeObject.Call(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var AResult:TBESENValue);
begin
 BESENThrowTypeError('Not a function');
end;

function TBESENNativeObject.HasConstruct:TBESENBoolean;
begin
 result:=true;
end;

function TBESENNativeObject.HasCall:TBESENBoolean;
begin
 result:=false;
end;

procedure TBESENNativeObject.Finalize;
var i:integer;
    o:TObject;
begin
 if assigned(PropList) then begin
  for i:=0 to PropListLen-1 do begin
   if assigned(PropList^[i].PropType) then begin
    case PropList^[i].PropType^.Kind of
     tkCLASS:begin
      o:=GetObjectProp(self,PropList^[i]);
      if o is TBESENGarbageCollectorObject then begin
       SetObjectProp(self,PropList^[i],nil);
      end;
     end;
    end;
   end;
  end;
 end;
 inherited Finalize;
end;

procedure TBESENNativeObject.Mark;
var i:integer;
    o:TObject;
begin
 if assigned(PropList) then begin
  for i:=0 to PropListLen-1 do begin
   if assigned(PropList^[i].PropType) then begin
    case PropList^[i].PropType^.Kind of
     tkCLASS:begin
      o:=GetObjectProp(self,PropList^[i]);
      if o is TBESENGarbageCollectorObject then begin
       TBESEN(Instance).GarbageCollector.GrayIt(TBESENObject(o));
      end;
     end;
    end;
   end;
  end;
 end;
 inherited Mark;
end;

end.
