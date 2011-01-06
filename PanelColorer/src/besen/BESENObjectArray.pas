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
unit BESENObjectArray;
{$i BESEN.inc}

interface

uses Math,BESENConstants,BESENTypes,BESENObject,BESENValue,BESENObjectPropertyDescriptor;

type TBESENObjectArray=class(TBESENObject)
      private
       ArrayLength:TBESENUINT32;
       function ToIndex(AProp:TBESENString;var v:int64):TBESENBoolean;
       function GetLen:TBESENUINT32;
       procedure SetLen(NewLen:TBESENUINT32);
      public
       constructor Create(AInstance:TObject;APrototype:TBESENObject=nil;AHasPrototypeProperty:longbool=false); overload; override;
       destructor Destroy; override;
       procedure PutEx(const P:TBESENString;const V:TBESENValue;Throw:TBESENBoolean;var Descriptor,OwnDescriptor:TBESENObjectPropertyDescriptor;var TempValue:TBESENValue;Hash:TBESENHash=0); override;
       procedure PutIndex(const Index,ID:integer;const V:TBESENValue;Throw:TBESENBoolean); override;
       function DefineOwnPropertyEx(const P:TBESENString;const Descriptor:TBESENObjectPropertyDescriptor;Throw:TBESENBoolean;var Current:TBESENObjectPropertyDescriptor;Hash:TBESENHash=0):TBESENBoolean; override;
       function GetArrayIndex(const Index:TBESENUINT32;var AResult:TBESENValue;Base:TBESENObject=nil):boolean; override;
       procedure PutArrayIndex(const Index:TBESENUINT32;const V:TBESENValue;Throw:TBESENBoolean); override;
       function DeleteArrayIndex(const Index:TBESENUINT32;Throw:TBESENBoolean):TBESENBoolean; override;
       procedure Finalize; override;
       procedure Mark; override;
       procedure Push(const AValue:TBESENValue);
       property Len:TBESENUINT32 read GetLen write SetLen;
     end;

implementation

uses BESEN,BESENUtils,BESENArrayUtils,BESENHashUtils,BESENGlobals,
     BESENNumberUtils,BESENErrors;

constructor TBESENObjectArray.Create(AInstance:TObject;APrototype:TBESENObject=nil;AHasPrototypeProperty:longbool=false);
begin
 inherited Create(AInstance,APrototype,AHasPrototypeProperty);
 ObjectClassName:='Array';
 ObjectName:='array';
 Len:=0;
 ArrayLength:=0;

 Properties.Sort(true);

 inherited OverwriteData('length',BESENNumberValue(0),[bopaWRITABLE]);
end;

destructor TBESENObjectArray.Destroy;
begin
 inherited Destroy;
end;

function TBESENObjectArray.ToIndex(AProp:TBESENString;var v:int64):TBESENBoolean;
begin
 result:=BESENArrayToIndex(AProp,v);
end;

{procedure TBESENObjectArray.SetLen(const AValue:TBESENValue);
var NewLen,i:TBESENUInt32;
begin
 NewLen:=TBESEN(Instance).ToUInt32(AValue);
 if Len>NewLen then begin
  for i:=Len+1 to NewLen do begin
   inherited Delete(IndexToStr(i-1),true);
  end;
 end;
 Len:=NewLen;
end;}

function TBESENObjectArray.GetLen:TBESENUINT32;
var LenDesc:TBESENObjectPropertyDescriptor;
begin
 GetOwnProperty('length',LenDesc,BESENLengthHash);
 result:=TBESEN(Instance).ToUINT32(LenDesc.Value);
 ArrayLength:=result;
end;

procedure TBESENObjectArray.SetLen(NewLen:TBESENUINT32);
begin
 ArrayLength:=NewLen;
 inherited OverwriteData('length',BESENNumberValue(NewLen),[bopaWRITABLE]);
end;

procedure TBESENObjectArray.PutEx(const P:TBESENString;const V:TBESENValue;Throw:TBESENBoolean;var Descriptor,OwnDescriptor:TBESENObjectPropertyDescriptor;var TempValue:TBESENValue;Hash:TBESENHash=0);
var Index:int64;
    Prop:TBESENObjectProperty;
begin
 // Trying faster pre-catching implementation first
 if ToIndex(P,Index) then begin
  if (Index>=0) and (Index<ArrayLength) then begin
   Prop:=Properties.Get(P,Hash);
   if assigned(Prop) then begin
    if (([boppVALUE,boppWRITABLE]*Prop.Descriptor.Presents)<>[]) and (bopaWRITABLE in Prop.Descriptor.Attributes) then begin
     BESENCopyValue(Prop.Descriptor.Value,v);
     exit;
    end;
   end else if Extensible then begin
    Prop:=Properties.Put(P,Hash);
    if assigned(Prop) then begin
     BESENCopyValue(Prop.Descriptor.Value,v);
     Prop.Descriptor.Getter:=nil;
     Prop.Descriptor.Setter:=nil;
     Prop.Descriptor.Presents:=[boppVALUE,boppWRITABLE,boppENUMERABLE,boppCONFIGURABLE];
     Prop.Descriptor.Attributes:=[bopaWRITABLE,bopaENUMERABLE,bopaCONFIGURABLE];
     if Prop.ID>=0 then begin
      InvalidateStructure;
     end;
     exit;
    end;
   end;
  end;
 end;

 // Fallback to specification-conformant implementation
 PutFull(P,V,false,Descriptor,OwnDescriptor,TempValue,Hash);
end;

procedure TBESENObjectArray.PutIndex(const Index,ID:integer;const V:TBESENValue;Throw:TBESENBoolean);
begin
 if ID=TBESEN(Instance).KeyIDManager.LengthID then begin
  Put(TBESEN(Instance).KeyIDManager.List[ID],V,Throw);
 end else if ((Index>=0) and (Index<Properties.ItemCount)) and assigned(Properties.Items[Index]) and (Properties.Items[Index].ID=ID) then begin
  inherited PutIndex(Index,ID,V,Throw);
 end else begin
  Put(TBESEN(Instance).KeyIDManager.List[ID],V,Throw);
 end;
end;

function TBESENObjectArray.DefineOwnPropertyEx(const P:TBESENString;const Descriptor:TBESENObjectPropertyDescriptor;Throw:TBESENBoolean;var Current:TBESENObjectPropertyDescriptor;Hash:TBESENHash=0):TBESENBoolean;
var OldLenDesc,NewLenDesc:TBESENObjectPropertyDescriptor;
    NewLen,OldLen:TBESENUInt32;
    NewWritable:boolean;
    Index:int64;
    Num:double;
begin
 GetOwnProperty('length',OldLenDesc,BESENLengthHash);
 OldLen:=TBESEN(Instance).ToUINT32(OldLenDesc.Value);
 if P='length' then begin
  if not (boppVALUE in Descriptor.Presents) then begin
   result:=inherited DefineOwnPropertyEx('length',Descriptor,Throw,Current,BESENLengthHash);
   exit;
  end;
  NewLenDesc:=Descriptor;
  NewLen:=TBESEN(Instance).ToUInt32(Descriptor.Value);
  Num:=TBESEN(Instance).ToNum(Descriptor.Value);
  if (NewLen<>Num) or not BESENIsFinite(Num) then begin
   raise EBESENRangeError.Create('DefineOwnProperty for "'+P+'" failed');
  end;
  if NewLen>=OldLen then begin
   ArrayLength:=NewLen;
   result:=inherited DefineOwnPropertyEx('length',Descriptor,Throw,Current,BESENLengthHash);
   exit;
  end;
  if not ((boppWRITABLE in OldLenDesc.Presents) and (bopaWRITABLE in OldLenDesc.Attributes)) then begin
   if Throw then begin
    raise EBESENTypeError.Create('DefineOwnProperty for "'+P+'" failed');
   end else begin
    result:=false;
    exit;
   end;
  end;
  if (boppWRITABLE in NewLenDesc.Presents) and (bopaWRITABLE in NewLenDesc.Attributes) then begin
   NewWritable:=true;
  end else begin
   NewWritable:=false;
   NewLenDesc.Presents:=NewLenDesc.Presents+[boppWRITABLE];
   NewLenDesc.Attributes:=NewLenDesc.Attributes+[bopaWRITABLE];
  end;
  result:=inherited DefineOwnPropertyEx('length',NewLenDesc,Throw,Current,BESENLengthHash);
  if not result then begin
   exit;
  end;
  ArrayLength:=NewLen;
  while NewLen<OldLen do begin
   dec(OldLen);
   if not Delete(BESENArrayIndexToStr(OldLen),false) then begin
    NewLenDesc.Value:=BESENNumberValue(OldLen+1);
    if not NewWritable then begin
     NewLenDesc.Attributes:=NewLenDesc.Attributes-[bopaWRITABLE];
    end;
    inherited DefineOwnPropertyEx('length',NewLenDesc,false,Current,BESENLengthHash);
    if Throw then begin
     raise EBESENTypeError.Create('DefineOwnProperty for "'+P+'" failed');
    end else begin
     result:=false;
     exit;
    end;
   end;
  end;
  if not NewWritable then begin
   NewLenDesc:=BESENUndefinedPropertyDescriptor;
   NewLenDesc.Presents:=NewLenDesc.Presents+[boppWRITABLE];
   NewLenDesc.Attributes:=NewLenDesc.Attributes-[bopaWRITABLE];
   inherited DefineOwnPropertyEx('length',NewLenDesc,false,Current,BESENLengthHash);
  end;
  result:=true;
 end else if ToIndex(P,Index) then begin
  if (Index>=OldLen) and not ((boppWRITABLE in OldLenDesc.Presents) and (bopaWRITABLE in OldLenDesc.Attributes)) then begin
   if Throw then begin
    raise EBESENTypeError.Create('DefineOwnProperty for "'+P+'" failed');
   end else begin
    result:=false;
    exit;
   end;
  end;
  result:=inherited DefineOwnPropertyEx(P,Descriptor,false,Current,Hash);
  if not result then begin
   if Throw then begin
    raise EBESENTypeError.Create('DefineOwnProperty for "'+P+'" failed');
   end else begin
    result:=false;
    exit;
   end;
  end;
  if Index>=OldLen then begin
   ArrayLength:=Index+1;
   OldLenDesc.Value:=BESENNumberValue(Index+1);
   inherited DefineOwnPropertyEx('length',OldLenDesc,false,Current,BESENLengthHash);
  end;
  result:=true;
 end else begin
  result:=inherited DefineOwnPropertyEx(P,Descriptor,Throw,Current,Hash);
 end;
end;

function TBESENObjectArray.GetArrayIndex(const Index:TBESENUINT32;var AResult:TBESENValue;Base:TBESENObject=nil):boolean;
 function Fallback:boolean;
 begin
  result:=Get(BESENArrayIndexToStr(Index),AResult,Base);
 end;
var Prop:TBESENObjectProperty;
begin
 result:=false;
 if not assigned(Base) then begin
  Base:=self;
 end;
 if Index<TBESENUINT32(Properties.ArrayItemCount) then begin
  Prop:=Properties.ArrayItems[Index];
  if assigned(Prop) then begin
   if ([boppVALUE,boppWRITABLE]*Prop.Descriptor.Presents)<>[] then begin
    BESENCopyValue(AResult,Prop.Descriptor.Value);
   end else if ([boppGETTER,boppSETTER]*Prop.Descriptor.Presents)<>[] then begin
    GetGetter(Base,AResult,Prop.Descriptor);
   end else begin
    AResult.ValueType:=bvtUNDEFINED;
   end;
   result:=true;
   exit;
  end;
 end;
 result:=Fallback;
end;

procedure TBESENObjectArray.PutArrayIndex(const Index:TBESENUINT32;const V:TBESENValue;Throw:TBESENBoolean);
 procedure Fallback;
 begin
  Put(BESENArrayIndexToStr(Index),V,Throw);
 end;
 procedure PutSetter(Obj:TBESENObject;Prop:TBESENObjectProperty);
 var TempValue:TBESENValue;
     ValuePointers:array[0..0] of PBESENValue;
 begin
  if (boppSETTER in Prop.Descriptor.Presents) and assigned(Prop.Descriptor.Setter) then begin
   ValuePointers[0]:=@V;
   TBESEN(Instance).ObjectCall(TBESENObject(Prop.Descriptor.Setter),BESENObjectValue(Obj),@ValuePointers,1,TempValue);
   exit;
  end;
  if Throw then begin
   BESENThrowNoSetter(Prop.Key);
  end;
 end;
var Prop:TBESENObjectProperty;
begin
 if Index<TBESENUINT32(Properties.ArrayItemCount) then begin
  Prop:=Properties.ArrayItems[Index];
  if assigned(Prop) then begin
   if ([boppVALUE,boppWRITABLE]*Prop.Descriptor.Presents)<>[] then begin
    if bopaWRITABLE in Prop.Descriptor.Attributes then begin
     BESENCopyValue(Prop.Descriptor.Value,V);
    end else begin
     if Throw then begin
      BESENThrowPut(Prop.Key);
     end;
    end;
    exit;
   end else if ([boppGETTER,boppSETTER]*Prop.Descriptor.Presents)<>[] then begin
    PutSetter(self,Prop);
    exit;
   end;
  end;
 end;
 Fallback;
end;

function TBESENObjectArray.DeleteArrayIndex(const Index:TBESENUINT32;Throw:TBESENBoolean):TBESENBoolean;
 function Fallback:boolean;
 begin
  result:=Delete(BESENArrayIndexToStr(Index),Throw);
 end;
var Prop:TBESENObjectProperty;
begin
 if Index<TBESENUINT32(Properties.ArrayItemCount) then begin
  Prop:=Properties.ArrayItems[Index];
  if assigned(Prop) then begin
   result:=Delete(Prop.Key,Throw);
   exit;
  end;
 end;
 result:=Fallback;
end;

procedure TBESENObjectArray.Finalize;
begin
 inherited Finalize;
end;

procedure TBESENObjectArray.Mark;
begin
 inherited Mark;
end;

procedure TBESENObjectArray.Push(const AValue:TBESENValue);
var v,tv:TBESENValue;
    ValuePointers:array[0..0] of PBESENValue;
begin
 Get('push',v);
 if (v.ValueType=bvtOBJECT) and assigned(TBESENObject(v.Obj)) and TBESENObject(v.Obj).HasCall then begin
  ValuePointers[0]:=@AValue;
  TBESEN(Instance).ObjectCall(TBESENObject(v.Obj),BESENObjectValue(self),@ValuePointers,1,tv);
 end;
end;

end.
