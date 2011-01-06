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
unit BESENObjectFunctionPrototype;
{$i BESEN.inc}

interface

uses SysUtils,Math,BESENConstants,BESENTypes,BESENObject,BESENObjectFunction,BESENValue,BESENObjectPropertyDescriptor,
     BESENObjectNativeFunction;

type TBESENObjectFunctionPrototype=class(TBESENObjectFunction)
      public
       constructor Create(AInstance:TObject;APrototype:TBESENObject=nil;AHasPrototypeProperty:longbool=false); overload; override;
       destructor Destroy; override;
       procedure Call(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var AResult:TBESENValue); override;
       function HasCall:TBESENBoolean; override;
       procedure NativeToString(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeApply(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeCall(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeBind(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
     end;

implementation

uses BESEN,BESENGlobals,BESENStringUtils,BESENErrors,BESENObjectDeclaredFunction,
     BESENObjectThrowTypeErrorFunction,BESENObjectArgGetterFunction,
     BESENObjectArgSetterFunction,BESENObjectBindingFunction;

constructor TBESENObjectFunctionPrototype.Create(AInstance:TObject;APrototype:TBESENObject=nil;AHasPrototypeProperty:longbool=false);
begin                                  
 inherited Create(AInstance,APrototype,AHasPrototypeProperty);
 ObjectClassName:='Function';
 ObjectName:='prototype';

 OverwriteData('length',BESENNumberValue(0),[]);
 if (TBESEN(Instance).Compatibility and COMPAT_JS)<>0 then begin
  OverwriteData('name',BESENStringValue(ObjectName),[]);
 end;
end;

destructor TBESENObjectFunctionPrototype.Destroy;
begin
 inherited Destroy;
end;

procedure TBESENObjectFunctionPrototype.Call(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var AResult:TBESENValue);
begin
 AResult:=BESENUndefinedValue;
end;

function TBESENObjectFunctionPrototype.HasCall:TBESENBoolean;
begin
 result:=true;
end;

procedure TBESENObjectFunctionPrototype.NativeToString(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 if not ((ThisArgument.ValueType=bvtOBJECT) and assigned(ThisArgument.Obj)) then begin
  raise EBESENTypeError.Create('Null this object');
 end;
 if assigned(ThisArgument.Obj) and (ThisArgument.Obj is TBESENObjectDeclaredFunction) then begin
  if assigned(TBESENObjectDeclaredFunction(ThisArgument.Obj).Node) then begin
   ResultValue:=BESENStringValue(BESENUTF8ToUTF16(TBESEN(Instance).Decompile(TBESENObjectDeclaredFunction(ThisArgument.Obj).Node)));
  end else begin
   ResultValue:=BESENStringValue('function () {'#10'}'#10);
  end;
 end else if assigned(ThisArgument.Obj) and (ThisArgument.Obj is TBESENObjectNativeFunction) then begin
  ResultValue:=BESENStringValue('function () {'#10#9'[native code]'#10'}'#10);
 end else if assigned(ThisArgument.Obj) and (ThisArgument.Obj is TBESENObjectThrowTypeErrorFunction) then begin
  ResultValue:=BESENStringValue('function () {'#10#9'[native code, ThrowTypeError]'#10'}'#10);
 end else if assigned(ThisArgument.Obj) and (ThisArgument.Obj is TBESENObjectArgGetterFunction) then begin
  ResultValue:=BESENStringValue('function () {'#10#9'[native code, ArgGetter]'#10'}'#10);
 end else if assigned(ThisArgument.Obj) and (ThisArgument.Obj is TBESENObjectArgSetterFunction) then begin
  ResultValue:=BESENStringValue('function () {'#10#9'[native code, ArgSetter]'#10'}'#10);
 end else if assigned(ThisArgument.Obj) and (ThisArgument.Obj is TBESENObjectBindingFunction) then begin
  ResultValue:=BESENStringValue('function () {'#10#9'[native code, Binding]'#10'}'#10);
 end else if assigned(ThisArgument.Obj) and (ThisArgument.Obj is TBESENObjectFunction) then begin
  ResultValue:=BESENStringValue('function () {'#10'}'#10);
 end else begin
  raise EBESENTypeError.Create('Not a function object');
 end;
end;

procedure TBESENObjectFunctionPrototype.NativeApply(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
var CallThisArg:TBESENValue;
    v2,v3:TBESENValue;
    vArgs:TBESENValues;
    pArgs:TBESENValuePointers;
    i,j:integer;
begin
 // ES5 errata fix
 vArgs:=nil;
 pArgs:=nil;
 if not ((ThisArgument.ValueType=bvtOBJECT) and assigned(ThisArgument.Obj)) then begin
  raise EBESENTypeError.Create('Null this object');
 end;
 if not TBESENObject(ThisArgument.Obj).HasCall then begin
  raise EBESENTypeError.Create('No callable');
 end;
 v2:=BESENEmptyValue;
 v3:=BESENEmptyValue;
 try
  if CountArguments<1 then begin
   CallThisArg:=BESENUndefinedValue;
  end else begin
   BESENCopyValue(CallThisArg,Arguments^[0]^);
   if (CountArguments>1) and not (Arguments^[1]^.ValueType in [bvtUNDEFINED,bvtNULL]) then begin
    TBESEN(Instance).ToObjectValue(Arguments^[1]^,v2);
    if (v2.ValueType=bvtOBJECT) and assigned(v2.Obj) then begin
     TBESENObject(v2.Obj).GarbageCollectorLock;
    end;
    TBESENObject(v2.Obj).Get('length',v3,TBESENObject(v2.Obj),BESENLengthHash);
    j:=TBESEN(Instance).ToUInt32(v3);
    SetLength(vArgs,j);
    SetLength(pArgs,j);
    for i:=0 to j-1 do begin
     TBESENObject(v2.Obj).Get(inttostr(i),vArgs[i]);
     pArgs[i]:=@vArgs[i];
    end;
   end;
  end;
  TBESEN(Instance).ObjectCall(TBESENObject(ThisArgument.Obj),CallThisArg,@pArgs[0],length(pArgs),ResultValue);
 finally
  if (v2.ValueType=bvtOBJECT) and assigned(v2.Obj) then begin
   TBESENObject(v2.Obj).GarbageCollectorUnlock;
  end;
  SetLength(vArgs,0);
  SetLength(pArgs,0);
 end;
end;

procedure TBESENObjectFunctionPrototype.NativeCall(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
var CallThisArg:TBESENValue;
    pArgs:TBESENValuePointers;
    i:integer;
begin
 if not ((ThisArgument.ValueType=bvtOBJECT) and assigned(ThisArgument.Obj)) then begin
  raise EBESENTypeError.Create('Null this object');
 end;
 if not TBESENObject(ThisArgument.Obj).HasCall then begin
  raise EBESENTypeError.Create('No callable');
 end;
 pArgs:=nil;
 try
  if CountArguments<1 then begin
   CallThisArg:=BESENUndefinedValue;
  end else begin
   BESENCopyValue(CallThisArg,Arguments^[0]^);
   if CountArguments>1 then begin
    SetLength(pArgs,CountArguments-1);
    for i:=0 to length(pArgs)-1 do begin
     pArgs[i]:=Arguments[i+1];
    end;
   end;
  end;
  TBESEN(Instance).ObjectCall(TBESENObject(ThisArgument.Obj),CallThisArg,@pArgs[0],length(pArgs),ResultValue);
 finally
  SetLength(pArgs,0);
 end;
end;

procedure TBESENObjectFunctionPrototype.NativeBind(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
var o:TBESENObjectBindingFunction;
    i:integer;
    v:TBESENValue;
begin
 if not ((ThisArgument.ValueType=bvtOBJECT) and assigned(ThisArgument.Obj)) then begin
  raise EBESENTypeError.Create('Null this object');
 end;
 if not TBESENObject(ThisArgument.Obj).HasCall then begin
  raise EBESENTypeError.Create('Bad arg');
 end;                                                            
 o:=TBESENObjectBindingFunction.Create(Instance,TBESEN(Instance).ObjectFunctionPrototype,false);
 TBESEN(Instance).GarbageCollector.Add(o);
 o.GarbageCollectorLock;
 try
  o.TargetFunction:=TBESENObject(ThisArgument.Obj);
  if CountArguments>0 then begin
   BESENCopyValue(o.BoundThis,Arguments^[0]^);
  end else begin
   o.BoundThis.ValueType:=bvtUNDEFINED;
  end;
  if CountArguments>1 then begin
   SetLength(o.BoundArguments,CountArguments-1);
   for i:=1 to CountArguments-1 do begin
    BESENCopyValue(o.BoundArguments[i-1],Arguments^[i]^);
   end;
  end else begin
   SetLength(o.BoundArguments,0);
  end;
  if o.TargetFunction is TBESENObjectFunction then begin
   TBESENObjectFunction(o.TargetFunction).Get('length',v,o.TargetFunction,BESENLengthHash);
   o.OverwriteData('length',BESENNumberValue(max(0,TBESEN(Instance).ToInt(v)-length(o.BoundArguments))),[]);
  end else begin
   o.OverwriteData('length',BESENNumberValue(0),[]);
  end;
  o.Extensible:=true;
  o.OverwriteAccessor('caller',TBESEN(Instance).ObjectThrowTypeErrorFunction,TBESEN(Instance).ObjectThrowTypeErrorFunction,[],false);
  o.OverwriteAccessor('arguments',TBESEN(Instance).ObjectThrowTypeErrorFunction,TBESEN(Instance).ObjectThrowTypeErrorFunction,[],false);
 finally
  o.GarbageCollectorUnlock;
 end;
 ResultValue:=BESENObjectValue(o);
end;

end.
 