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
unit BESENCodeGeneratorContext;
{$i BESEN.inc}

interface

uses BESENConstants,BESENTypes,BESENValue,BESENBaseObject,
     BESENCollectorObject,BESENHashMap;

type TBESENCodeGeneratorContextVariableScope=record
      Ident:TBESENString;
      ID:integer;
      InScope:longbool;
      ValueType:TBESENValueType;
      Initialized:longbool;
      MutableOrDeletable:longbool;
      RegNr:integer;
     end;

     TBESENCodeGeneratorContextVariableScopes=array of TBESENCodeGeneratorContextVariableScope;

     TBESENCodeGeneratorContextPatchables=class(TBESENBaseObject)
      public
       Continues:TBESENIntegers;
       Breaks:TBESENIntegers;
       CountContinues:integer;
       CountBreaks:integer;
       Previous:TBESENCodeGeneratorContextPatchables;
       Continuable:longbool;
       BlockDepth:integer;
       Target:integer;
       ContinueValueTypesItems:TBESENValueTypesItems;
       BreakValueTypesItems:TBESENValueTypesItems;
       CountContinueValueTypesItems:integer;
       CountBreakValueTypesItems:integer;
       constructor Create(AInstance:TObject); overload; override;
       destructor Destroy; override;
       procedure AddContinue(Address:integer;const ValueTypes:TBESENValueTypes);
       procedure AddBreak(Address:integer;const ValueTypes:TBESENValueTypes);
     end;

     TBESENCodeGeneratorContextRegister=record
      IsWhat:longword;
      IsLocal:longbool;
      IsInlineCacheRegister:boolean;
      LocalIndex:longint;
      ReferenceValueType:TBESENValueType;
      InUse:longbool;
      Variable:longint;
     end;

     TBESENCodeGeneratorContextRegisters=array of TBESENCodeGeneratorContextRegister;

     TBESENCodeGeneratorContextRegisterStates=record
      Registers:TBESENCodeGeneratorContextRegisters;
      MaxRegisters:integer;
     end;

     TBESENCodeGeneratorContext=class(TBESENBaseObject)
      public
       Next:TBESENCodeGeneratorContext;
       Code:TObject;
       BlockDepth:integer;
       MaxBlockDepth:integer;
       LoopDepth:integer;
       MaxLoopDepth:integer;
       MaxParamArgs:integer;
       Patchables:TBESENCodeGeneratorContextPatchables;
       VariableScopes:TBESENCodeGeneratorContextVariableScopes;
       CountVariableScopes:integer;
       InVariableScope:longbool;
       VariableScopeHashMap:TBESENHashMap;
       VariableNameHashMap:TBESENHashMap;
       Registers:TBESENCodeGeneratorContextRegisters;
       MaxRegisters:integer;
       LookupHashMap:TBESENHashMap;
       constructor Create(AInstance:TObject); overload; override;
       destructor Destroy; override;
       procedure PushPatchables(Target:integer;Continuable:boolean);
       procedure PopPatchables(ContinueAddr,BreakAddr:integer);
       function FindPatchables(Target:integer;Continuable:boolean):TBESENCodeGeneratorContextPatchables;
       procedure BlockEnter;
       procedure BlockLeave;
       function BlockCurrent:integer;
       function AllocateRegister:integer;
       procedure DeallocateRegister(var RegNr:integer);
       procedure GetRegisterStates(var RegisterStates:TBESENCodeGeneratorContextRegisterStates);
       procedure SetRegisterStates(var RegisterStates:TBESENCodeGeneratorContextRegisterStates);
       function VariableIndex(const Ident:TBESENString):integer;
       function VariableGetInitialized(const Ident:TBESENString):TBESENBoolean;
       function VariableGetMutableOrDeletable(const Ident:TBESENString):TBESENBoolean;
       procedure VariableSetFlags(const Ident:TBESENString;const Initialized,MutableOrDeletable:boolean);
       function VariableGetRegister(const Ident:TBESENString):integer;
       procedure VariableSetRegister(const Ident:TBESENString;const RegNr:integer);
       function VariableGetType(const Ident:TBESENString):TBESENValueType;
       procedure VariableSetType(const Ident:TBESENString;const ValueType:TBESENValueType);
       procedure VariableAllSetType(const ValueType:TBESENValueType);
       function VariableGetTypes:TBESENValueTypes;
       procedure VariableSetTypes(const ValueTypes:TBESENValueTypes);
       function VariableGetIdent(const Index:integer):TBESENString;
       function VariableID(const Ident:TBESENString):integer;
       function IsVariableInScope(const Ident:TBESENString):boolean;
       procedure VariableSetScope(const Ident:TBESENString;IsInScope,IsParameter:boolean;ParameterIndex:integer);
       function VariableSetAllScope(IsInScope:boolean):boolean;
     end;

implementation

uses BESEN,BESENUtils,BESENCode;

constructor TBESENCodeGeneratorContextPatchables.Create(AInstance:TObject);
begin
 inherited Create(AInstance);
 Continues:=nil;
 Breaks:=nil;
 CountContinues:=0;
 CountBreaks:=0;
 Previous:=nil;
 Continuable:=false;
 BlockDepth:=0;
 Target:=0;
 ContinueValueTypesItems:=nil;
 BreakValueTypesItems:=nil;
 CountContinueValueTypesItems:=0;
 CountBreakValueTypesItems:=0;
end;

destructor TBESENCodeGeneratorContextPatchables.Destroy;
begin
 SetLength(Continues,0);
 SetLength(Breaks,0);
 SetLength(ContinueValueTypesItems,0);
 SetLength(BreakValueTypesItems,0);
 inherited Destroy;
end;

procedure TBESENCodeGeneratorContextPatchables.AddContinue(Address:integer;const ValueTypes:TBESENValueTypes);
begin
 if CountContinues>=length(Continues) then begin
  SetLength(Continues,CountContinues+256);
 end;
 Continues[CountContinues]:=Address;
 inc(CountContinues);
 if CountContinueValueTypesItems>=length(ContinueValueTypesItems) then begin
  SetLength(ContinueValueTypesItems,CountContinueValueTypesItems+256);
 end;
 ContinueValueTypesItems[CountContinueValueTypesItems]:=copy(ValueTypes,0,length(ValueTypes));
 inc(CountContinueValueTypesItems);
end;

procedure TBESENCodeGeneratorContextPatchables.AddBreak(Address:integer;const ValueTypes:TBESENValueTypes);
begin
 if CountBreaks>=length(Breaks) then begin
  SetLength(Breaks,CountBreaks+256);
 end;
 Breaks[CountBreaks]:=Address;
 inc(CountBreaks);
 if CountBreakValueTypesItems>=length(BreakValueTypesItems) then begin
  SetLength(BreakValueTypesItems,CountBreakValueTypesItems+256);
 end;
 BreakValueTypesItems[CountBreakValueTypesItems]:=copy(ValueTypes,0,length(ValueTypes));
 inc(CountBreakValueTypesItems);
end;

constructor TBESENCodeGeneratorContext.Create(AInstance:TObject);
begin
 inherited Create(AInstance);
 Next:=nil;
 Code:=nil;
 BlockDepth:=0;
 MaxBlockDepth:=0;
 LoopDepth:=0;
 MaxLoopDepth:=0;
 MaxParamArgs:=0;
 Patchables:=nil;
 VariableScopes:=nil;
 CountVariableScopes:=0;
 InVariableScope:=true;
 VariableScopeHashMap:=TBESENHashMap.Create;
 VariableNameHashMap:=TBESENHashMap.Create;
 Registers:=nil;
 MaxRegisters:=0;
 LookupHashMap:=TBESENHashMap.Create;
end;

destructor TBESENCodeGeneratorContext.Destroy;
var NextPatchables:TBESENCodeGeneratorContextPatchables;
begin
 BESENFreeAndNil(LookupHashMap);
 while assigned(Patchables) do begin
  NextPatchables:=Patchables.Previous;
  Patchables.Destroy;
  Patchables:=NextPatchables;
 end;
 SetLength(Registers,0);
 SetLength(VariableScopes,0);
 VariableScopeHashMap.Destroy;
 VariableNameHashMap.Destroy;
 inherited Destroy;
end;

procedure TBESENCodeGeneratorContext.PushPatchables(Target:integer;Continuable:boolean);
var p:TBESENCodeGeneratorContextPatchables;
begin
 p:=TBESENCodeGeneratorContextPatchables.Create(Instance);
 p.Target:=Target;
 p.Continuable:=Continuable;
 p.Previous:=Patchables;
 p.BlockDepth:=BlockDepth;
 Patchables:=p;
end;

procedure TBESENCodeGeneratorContext.PopPatchables(ContinueAddr,BreakAddr:integer);
var i:integer;
    p:TBESENCodeGeneratorContextPatchables;
begin
 p:=Patchables;
 if ContinueAddr>=0 then begin
  for i:=0 to p.CountContinues-1 do begin
   TBESENCode(Code).Patch(p.Continues[i],ContinueAddr);
  end;
 end;
 if BreakAddr>=0 then begin
  for i:=0 to p.CountBreaks-1 do begin
   TBESENCode(Code).Patch(p.Breaks[i],BreakAddr);
  end;
 end;
 Patchables:=p.Previous;
 BESENFreeAndNil(p);
end;

function TBESENCodeGeneratorContext.FindPatchables(Target:integer;Continuable:boolean):TBESENCodeGeneratorContextPatchables;
var p:TBESENCodeGeneratorContextPatchables;
begin
 result:=nil;
 if (Target=bcttNOTARGET) and Continuable then begin
  p:=Patchables;
  while assigned(p) do begin
   if p.Continuable then begin
    result:=p;
    break;
   end;
   p:=p.Previous;
  end;
 end else if Target=bcttNOTARGET then begin
  result:=Patchables;
 end else begin
  p:=Patchables;
  while assigned(p) do begin
   if p.Target=Target then begin
    result:=p;
    break;
   end;
   p:=p.Previous;
  end;
 end;
{$ifdef UseAssert}
 Assert(assigned(result),'Lost patchable');
{$endif}
end;

procedure TBESENCodeGeneratorContext.BlockEnter;
begin
 inc(BlockDepth);
 if MaxBlockDepth<BlockDepth then begin
  MaxBlockDepth:=BlockDepth;
 end;
end;

procedure TBESENCodeGeneratorContext.BlockLeave;
begin
 dec(BlockDepth);
end;

function TBESENCodeGeneratorContext.BlockCurrent:integer;
begin
 result:=BlockDepth;
end;

function TBESENCodeGeneratorContext.AllocateRegister:integer;
var i:integer;
begin
 result:=-1;
 for i:=0 to MaxRegisters-1 do begin
  if not Registers[i].InUse then begin
   result:=i;
   break;
  end;
 end;
 if result<0 then begin
  result:=MaxRegisters;
  inc(MaxRegisters);
  if result>=length(Registers) then begin
   SetLength(Registers,result+256);
  end;
 end;
 Registers[result].InUse:=true;
 Registers[result].IsWhat:=0;
 Registers[result].IsLocal:=false;
 Registers[result].IsInlineCacheRegister:=false;
 Registers[result].LocalIndex:=-1;
 Registers[result].Variable:=-1;
end;

procedure TBESENCodeGeneratorContext.DeallocateRegister(var RegNr:integer);
begin
{if ((RegNr>=0) and (RegNr<MaxRegisters)) and not ((Registers[RegNr].Variable>=0) or Registers[RegNr].IsInlineCacheRegister) then begin
  Registers[RegNr].InUse:=false;
  Registers[RegNr].IsWhat:=0;
  Registers[RegNr].IsLocal:=false;
  Registers[RegNr].IsInlineCacheRegister:=false;
  Registers[RegNr].LocalIndex:=-1;
 end;}
 RegNr:=-1;
end;

procedure TBESENCodeGeneratorContext.GetRegisterStates(var RegisterStates:TBESENCodeGeneratorContextRegisterStates);
begin
 RegisterStates.Registers:=copy(Registers,0,length(Registers));
 RegisterStates.MaxRegisters:=MaxRegisters;
end;

procedure TBESENCodeGeneratorContext.SetRegisterStates(var RegisterStates:TBESENCodeGeneratorContextRegisterStates);
begin
 Registers:=copy(RegisterStates.Registers,0,length(RegisterStates.Registers));
 MaxRegisters:=RegisterStates.MaxRegisters;
end;

function TBESENCodeGeneratorContext.VariableIndex(const Ident:TBESENString):integer;
var Item:PBESENHashMapItem;
begin
 Item:=VariableScopeHashMap.GetKey(Ident);
 if assigned(Item) then begin
  result:=Item^.Value;
 end else begin
  result:=-1;
 end;
end;

function TBESENCodeGeneratorContext.VariableGetInitialized(const Ident:TBESENString):TBESENBoolean;
var i:integer;
begin
 i:=VariableIndex(Ident);
 if i>=0 then begin
  result:=VariableScopes[i].Initialized;
 end else begin
  result:=false;
 end;
end;

function TBESENCodeGeneratorContext.VariableGetMutableOrDeletable(const Ident:TBESENString):TBESENBoolean;
var i:integer;
begin
 i:=VariableIndex(Ident);
 if i>=0 then begin
  result:=VariableScopes[i].MutableOrDeletable;
 end else begin
  result:=false;
 end;
end;

procedure TBESENCodeGeneratorContext.VariableSetFlags(const Ident:TBESENString;const Initialized,MutableOrDeletable:boolean);
var i:integer;
begin
 i:=VariableIndex(Ident);
 if i>=0 then begin
  VariableScopes[i].Initialized:=Initialized;
  VariableScopes[i].MutableOrDeletable:=MutableOrDeletable;
 end;
end;

function TBESENCodeGeneratorContext.VariableGetRegister(const Ident:TBESENString):integer;
var i:integer;
begin
 i:=VariableIndex(Ident);
 if i>=0 then begin
  result:=VariableScopes[i].RegNr;
 end else begin
  result:=-1;
 end;
end;

procedure TBESENCodeGeneratorContext.VariableSetRegister(const Ident:TBESENString;const RegNr:integer);
var i:integer;
begin
 i:=VariableIndex(Ident);
 if i>=0 then begin
  VariableScopes[i].RegNr:=RegNr;
 end;
end;

function TBESENCodeGeneratorContext.VariableGetType(const Ident:TBESENString):TBESENValueType;
var i:integer;
begin
 i:=VariableIndex(Ident);
 if i>=0 then begin
  result:=VariableScopes[i].ValueType;
 end else begin
  result:=bvtUNDEFINED;
 end;
end;

procedure TBESENCodeGeneratorContext.VariableSetType(const Ident:TBESENString;const ValueType:TBESENValueType);
var i:integer;
begin
 i:=VariableIndex(Ident);
 if i>=0 then begin
  VariableScopes[i].ValueType:=ValueType;
 end;
end;

procedure TBESENCodeGeneratorContext.VariableAllSetType(const ValueType:TBESENValueType);
var i:integer;
begin
 for i:=0 to CountVariableScopes-1 do begin
  VariableScopes[i].ValueType:=ValueType;
 end;
end;

function TBESENCodeGeneratorContext.VariableGetTypes:TBESENValueTypes;
var i:integer;
begin
 SetLength(result,CountVariableScopes);
 for i:=0 to CountVariableScopes-1 do begin
  result[i]:=VariableScopes[i].ValueType;
 end;
end;

procedure TBESENCodeGeneratorContext.VariableSetTypes(const ValueTypes:TBESENValueTypes);
var i,j:integer;
begin
 j:=CountVariableScopes;
 if j>length(ValueTypes) then begin
  j:=length(ValueTypes);
 end;
 for i:=0 to j-1 do begin
  VariableScopes[i].ValueType:=ValueTypes[i];
 end;
end;

function TBESENCodeGeneratorContext.VariableGetIdent(const Index:integer):TBESENString;
begin
 if (Index>=0) and (Index<CountVariableScopes) then begin
  result:=VariableScopes[Index].Ident;
 end else begin
  result:='';
 end;
end;

function TBESENCodeGeneratorContext.VariableID(const Ident:TBESENString):integer;
var i:integer;
begin
 i:=VariableIndex(Ident);
 if i>=0 then begin
  result:=VariableScopes[i].ID;
 end else begin
  result:=-1;
 end;
end;

function TBESENCodeGeneratorContext.IsVariableInScope(const Ident:TBESENString):boolean;
var i:integer;
begin
 result:=false;
 if InVariableScope then begin
  i:=VariableIndex(Ident);
  if i>=0 then begin
   result:=VariableScopes[i].InScope;
  end;
 end;
end;

procedure TBESENCodeGeneratorContext.VariableSetScope(const Ident:TBESENString;IsInScope,IsParameter:boolean;ParameterIndex:integer);
var i:integer;
    Item:PBESENHashMapItem;
begin
 i:=VariableIndex(Ident);
 if i>=0 then begin
  VariableScopes[i].InScope:=IsInScope;
 end else begin
  if IsInScope then begin
   if CountVariableScopes>=length(VariableScopes) then begin
    SetLength(VariableScopes,CountVariableScopes+256);
   end;
   VariableScopes[CountVariableScopes].Ident:=Ident;
   VariableScopes[CountVariableScopes].ID:=TBESENCode(Code).GenVariable(Ident,IsParameter,ParameterIndex,self);
   VariableScopes[CountVariableScopes].InScope:=IsInScope;
   VariableScopes[CountVariableScopes].Initialized:=false;
   VariableScopes[CountVariableScopes].MutableOrDeletable:=false;
   VariableScopes[CountVariableScopes].ValueType:=bvtUNDEFINED;
   VariableScopes[CountVariableScopes].RegNr:=-1;
   Item:=VariableScopeHashMap.NewKey(Ident,true);
   Item^.Value:=CountVariableScopes;
   inc(CountVariableScopes);
  end;
 end;
end;

function TBESENCodeGeneratorContext.VariableSetAllScope(IsInScope:boolean):boolean;
begin
 result:=InVariableScope;
 InVariableScope:=IsInScope;
end;

end.
