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
unit BESENObjectJSON;
{$i BESEN.inc}

interface

uses Math,BESENConstants,BESENTypes,BESENObject,BESENValue,BESENObjectPropertyDescriptor;

type TBESENObjectJSON=class(TBESENObject)
      public
       constructor Create(AInstance:TObject;APrototype:TBESENObject=nil;AHasPrototypeProperty:longbool=false); overload; override;
       destructor Destroy; override;
       procedure NativeParse(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeStringify(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
     end;

implementation

uses BESEN,BESENGlobals,BESENUtils,BESENArrayUtils,BESENStringUtils,BESENPointerList,
     BESENStringTree,BESENNumberUtils,BESENErrors,BESENObjectBoolean,BESENObjectNumber,
     BESENObjectString,BESENObjectArray;

constructor TBESENObjectJSON.Create(AInstance:TObject;APrototype:TBESENObject=nil;AHasPrototypeProperty:longbool=false);
begin
 inherited Create(AInstance,APrototype,AHasPrototypeProperty);
 ObjectClassName:='JSON';

 RegisterNativeFunction('parse',NativeParse,2,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('stringify',NativeStringify,3,[bopaWRITABLE,bopaCONFIGURABLE],false);
end;

destructor TBESENObjectJSON.Destroy;
begin
 inherited Destroy;
end;

procedure TBESENObjectJSON.NativeParse(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
var Root,Recviver:TBESENObject;
 procedure Walk(const Holder:TBESENObject;const Name:TBESENString;var rv:TBESENValue);
 var Val,NewElement,Temp:TBESENValue;
     i,Len:int64;
     Enumerator:TBESENObjectPropertyEnumerator;
     PropKey:TBESENString;
     Keys:TBESENStrings;
     p:TBESENString;
     ValuePointers:array[0..1] of PBESENValue;
     o:TBESENObject;
 begin
  Keys:=nil;
  Holder.Get(Name,Val);
  if (Val.ValueType=bvtOBJECT) and assigned(Val.Obj) then begin
   o:=TBESENObject(Val.Obj);
   o.GarbageCollectorLock;
   try
    if Val.Obj is TBESENObjectArray then begin
     i:=0;
     TBESENObject(Val.Obj).Get('length',Temp,TBESENObject(Val.Obj),BESENLengthHash);
     Len:=TBESEN(Instance).ToInt(Temp);
     while i<Len do begin
      Walk(TBESENObject(Val.Obj),BESENArrayIndexToStr(i),NewElement);
      if NewElement.ValueType=bvtUNDEFINED then begin
       TBESENObject(Val.Obj).Delete(BESENArrayIndexToStr(i),false);
      end else begin
       TBESENObject(Val.Obj).DefineOwnProperty(BESENArrayIndexToStr(i),BESENDataPropertyDescriptor(NewElement,[bopaWRITABLE,bopaENUMERABLE,bopaCONFIGURABLE]),false);
      end;
      inc(i);
     end;
    end else begin
     try
      i:=0;
      Enumerator:=nil;
      PropKey:='';
      try
       Enumerator:=TBESENObject(Val.Obj).Enumerator(true,false);
       Enumerator.Reset;
       while Enumerator.GetNext(PropKey) do begin
        if i>=length(Keys) then begin
         SetLength(Keys,i+256);
        end;
        Keys[i]:=PropKey;
        inc(i);
       end;
      finally
       BESENFreeAndNil(Enumerator);
      end;
      i:=0;
      while i<length(Keys) do begin
       p:=Keys[i];
       Walk(TBESENObject(Val.Obj),p,NewElement);
       if NewElement.ValueType=bvtUNDEFINED then begin
        TBESENObject(Val.Obj).Delete(p,false);
       end else begin
        TBESENObject(Val.Obj).DefineOwnProperty(p,BESENDataPropertyDescriptor(NewElement,[bopaWRITABLE,bopaENUMERABLE,bopaCONFIGURABLE]),false);
       end;
       inc(i);
      end;
     finally
      SetLength(Keys,0);
     end;
    end;
   finally
    o.GarbageCollectorUnlock;
   end;
  end;
  Temp.ValueType:=bvtSTRING;
  Temp.Str:=Name;
  ValuePointers[0]:=@Temp;
  ValuePointers[1]:=@Val;
  TBESEN(Instance).ObjectCall(Recviver,BESENObjectValue(Holder),@ValuePointers,2,rv);
 end;
begin
 if CountArguments>0 then begin
  ResultValue:=TBESEN(Instance).JSONEval(BESENUTF16ToUTF8(TBESEN(Instance).ToStr(Arguments^[0]^)));
 end else begin
  ResultValue:=BESENUndefinedValue;
 end;
 if (CountArguments>1) and BESENIsCallable(Arguments^[1]^) then begin
  Recviver:=TBESENObject(Arguments^[1]^.Obj);
  Root:=TBESENObject.Create(Instance,TBESEN(Instance).ObjectPrototype);
  TBESEN(Instance).GarbageCollector.Add(Root);
  Root.GarbageCollectorLock;
  try
   Root.DefineOwnProperty('',BESENDataPropertyDescriptor(ResultValue,[bopaWRITABLE,bopaENUMERABLE,bopaCONFIGURABLE]),false);
   Walk(Root,'',ResultValue);
  finally
   Root.GarbageCollectorUnlock;
  end;
 end;
end;

procedure TBESENObjectJSON.NativeStringify(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
var Value,Replacer,Space,v:TBESENValue;
    i,j,k:int64;
    Gap,Ident,vs:TBESENString;
    ReplacerFunction,Wrapper:TBESENObject;
    Stack:TBESENPointerList;
    PropertyList:TBESENStrings;
    HasPropertyList:boolean;
    PropertyListStringTree:TBESENStringTree;
    PropertyListStringTreeData:TBESENStringTreeData;
 procedure Str(const Key:TBESENANSISTRING;const Holder:TBESENObject;var rv:TBESENValue);
 var Value,toJSON,v,Temp:TBESENValue;
     Output,StepBack:TBESENString;
     i,j:int64;
     oi:integer;
     Enumerator:TBESENObjectPropertyEnumerator;
     PropKey:TBESENString;
     Keys:TBESENStrings;
     ValuePointers:array[0..1] of PBESENValue;
 begin
  Keys:=nil;
  Holder.Get(Key,Value);
  if Value.ValueType=bvtOBJECT then begin
   TBESENObject(Value.Obj).Get('toJSON',toJSON);
   iF BESENIsCallable(toJSON) then begin
    v.ValueType:=bvtSTRING;
    v.Str:=Key;
    ValuePointers[0]:=@v;
    TBESEN(Instance).ObjectCall(TBESENObject(toJSON.Obj),Value,@ValuePointers,1,Temp);
    BESENCopyValue(Value,Temp);
   end;
  end;
  if assigned(ReplacerFunction) then begin
   v.ValueType:=bvtSTRING;
   v.Str:=Key;
   ValuePointers[0]:=@v;
   ValuePointers[1]:=@Value;
   TBESEN(Instance).ObjectCall(ReplacerFunction,BESENObjectValue(Holder),@ValuePointers,2,Temp);
   BESENCopyValue(Value,Temp);
  end;
  if (Value.ValueType=bvtOBJECT) and not assigned(Value.Obj) then begin
   Value.ValueType:=bvtNULL;
  end;
  if Value.ValueType=bvtOBJECT then begin
   if Value.Obj is TBESENObjectNumber then begin
    TBESEN(Instance).ToNumberValue(Value,Temp);
    BESENCopyValue(Value,Temp);
   end else if Value.Obj is TBESENObjectString then begin
    TBESEN(Instance).ToStringValue(Value,Temp);
    BESENCopyValue(Value,Temp);
   end else if Value.Obj is TBESENObjectBoolean then begin
    TBESEN(Instance).ToPrimitiveValue(Value,Temp);
    BESENCopyValue(Value,Temp); 
   end;
  end;
  case Value.ValueType of
   bvtNULL:begin
    rv:=BESENStringValue('null');
   end;
   bvtBOOLEAN:begin
    if Value.Bool then begin
     rv:=BESENStringValue('true');
    end else begin
     rv:=BESENStringValue('false');
    end;
   end;
   bvtSTRING:begin
    rv:=BESENStringValue(BESENJSONStringQuote(TBESEN(Instance).ToStr(Value)));
   end;
   bvtNUMBER:begin
    if BESENIsFinite(Value.Num) then begin
     TBESEN(Instance).ToStringValue(Value,rv);
    end else begin
     rv:=BESENStringValue('null');
    end;
   end;
   bvtOBJECT:begin
    if Stack.Find(Value.Obj)>=0 then begin
     raise EBESENTypeError.Create('Cyclical situation found');
    end;
    oi:=Stack.Add(Value.Obj);
    StepBack:=Ident;
    Ident:=Ident+Gap;
    if BESENIsCallable(Value) then begin
     rv:=BESENUndefinedValue;
    end else if Value.Obj is TBESENObjectArray then begin
     TBESENObject(Value.Obj).Get('length',Temp,TBESENObject(Value.Obj),BESENLengthHash);
     j:=TBESEN(Instance).ToUInt32(Temp);
     if j=0 then begin
      Output:='[]';
     end else begin
      if length(Ident)>0 then begin
       Output:='['#10+Ident;
      end else begin
       Output:='[';
      end;
      i:=0;
      while i<j do begin
       Str(BESENArrayIndexToStr(i),TBESENObject(Value.Obj),v);
       if v.ValueType=bvtUNDEFINED then begin
        Output:=Output+'null';
       end else begin
        Output:=Output+TBESEN(Instance).ToStr(v);
       end;
       inc(i);
       if i<j then begin
        Output:=Output+',';
        if length(Ident)>0 then begin
         Output:=Output+#10+Ident;
        end;
       end;
      end;
      if length(Ident)>0 then begin
       Output:=Output+#10+StepBack+']';
      end else begin
       Output:=Output+']';
      end;
     end;
     rv:=BESENStringValue(Output);
    end else begin
     try
      if HasPropertyList and (length(PropertyList)>0) then begin
       Keys:=copy(PropertyList,0,length(PropertyList));
       j:=length(Keys);
      end else begin
       j:=0;
       Enumerator:=nil;
       PropKey:='';
       try
        Enumerator:=TBESENObject(Value.Obj).Enumerator(true,false);
        Enumerator.Reset;
        while Enumerator.GetNext(PropKey) do begin
         if j>=length(Keys) then begin
          SetLength(Keys,j+256);
         end;
         Keys[j]:=PropKey;
         inc(j);
        end;
       finally
        BESENFreeAndNil(Enumerator);
       end;
      end;
      if j=0 then begin
       Output:='{}';
      end else begin
       if length(Ident)>0 then begin
        Output:='{'#10+Ident;
       end else begin
        Output:='{';
       end;
       i:=0;
       while i<j do begin
        Str(Keys[i],TBESENObject(Value.Obj),v);
        Output:=Output+BESENJSONStringQuote(Keys[i]);
        if length(Ident)>0 then begin
         Output:=Output+': ';
        end else begin
         Output:=Output+':';
        end;
        if v.ValueType=bvtUNDEFINED then begin
         Output:=Output+'null';
        end else begin
         Output:=Output+TBESEN(Instance).ToStr(v);
        end;
        inc(i);
        if i<j then begin
         Output:=Output+',';
         if length(Ident)>0 then begin
          Output:=Output+#10+Ident;
         end;
        end;
       end;
       if length(Ident)>0 then begin
        Output:=Output+#10+StepBack+'}';
       end else begin
        Output:=Output+'}';
       end;
      end;
      rv:=BESENStringValue(Output);
     finally
      SetLength(Keys,0);
     end;
    end;
    Stack.Delete(oi);
    Ident:=StepBack;
   end;
   else begin
    rv:=BESENUndefinedValue;
   end;
  end;
 end;
var PropItem:TBESENObjectProperty;
begin
 Gap:='';
 Ident:='';
 ReplacerFunction:=nil;
 PropertyList:=nil;
 Stack:=TBESENPointerList.Create;
 try
  if CountArguments>0 then begin
   Value:=Arguments^[0]^;
  end else begin
   Value:=BESENUndefinedValue;
  end;
  if CountArguments>1 then begin
   Replacer:=Arguments^[1]^;
  end else begin
   Replacer:=BESENUndefinedValue;
  end;
  HasPropertyList:=false;
  if (Replacer.ValueType=bvtOBJECT) and assigned(Replacer.Obj) then begin
   if BESENIsCallable(Replacer) then begin
    ReplacerFunction:=TBESENObject(Replacer.Obj);
   end else if Replacer.Obj is TBESENObjectArray then begin
    PropertyListStringTree:=TBESENStringTree.Create;
    try
     HasPropertyList:=true;
     PropItem:=TBESENObject(Replacer.Obj).Properties.First;
     k:=0;
     while assigned(PropItem) do begin
      if (boppENUMERABLE in PropItem.Descriptor.Presents) and (bopaENUMERABLE in PropItem.Descriptor.Attributes) and BESENArrayToIndex(PropItem.Key,i) then begin
       TBESENObject(Replacer.Obj).Get(PropItem.Key,v);
       if (v.ValueType in [bvtSTRING,bvtNUMBER]) or ((v.ValueType=bvtOBJECT) and assigned(TBESENObject(v.Obj)) and ((TBESENObject(v.Obj) is TBESENObjectString) or (TBESENObject(v.Obj) is TBESENObjectNumber))) then begin
        vs:=TBESEN(Instance).ToStr(v);
        if not PropertyListStringTree.Find(vs,PropertyListStringTreeData) then begin
         PropertyListStringTreeData.i:=k;
         PropertyListStringTree.Add(vs,PropertyListStringTreeData);
         if k>=length(PropertyList) then begin
          SetLength(PropertyList,k+256);
         end;
         PropertyList[k]:=vs;
         inc(k);
        end;
       end;
      end;
      PropItem:=PropItem.Next;
     end;
     SetLength(PropertyList,k);
    finally
     BESENFreeAndNil(PropertyListStringTree);
    end;
   end;
  end;
  if CountArguments>2 then begin
   if (Arguments^[2]^.ValueType=bvtOBJECT) and assigned(Arguments^[2]^.Obj) then begin
    if Arguments^[2]^.Obj is TBESENObjectNumber then begin
     TBESEN(Instance).ToNumberValue(Arguments^[2]^,Space);
    end else if Arguments^[2]^.Obj is TBESENObjectString then begin
     TBESEN(Instance).ToStringValue(Arguments^[2]^,Space);
    end;
   end else begin
    Space:=Arguments^[2]^;
   end;
   if Space.ValueType=bvtNumber then begin
    j:=min(10,TBESEN(Instance).ToInt(Space));
    i:=0;
    while i<j do begin
     Gap:=Gap+' ';
     inc(i);
    end;
   end else if Space.ValueType=bvtSTRING then begin
    Gap:=copy(TBESEN(Instance).ToStr(Space),1,10);
   end;
  end;
  Wrapper:=TBESENObject.Create(Instance,TBESEN(Instance).ObjectPrototype);
  TBESEN(Instance).GarbageCollector.Add(Wrapper);
  Wrapper.GarbageCollectorLock;
  try
   Wrapper.DefineOwnProperty('',BESENDataPropertyDescriptor(Value,[bopaWRITABLE,bopaENUMERABLE,bopaCONFIGURABLE]),false);
   Str('',Wrapper,ResultValue);
  finally
   Wrapper.GarbageCollectorUnlock;
  end;
 finally
  BESENFreeAndNil(Stack);
  SetLength(PropertyList,0);
 end;
end;

end.
