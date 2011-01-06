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
unit BESENObjectDeclaredFunction;
{$i BESEN.inc}

interface

uses BESENConstants,BESENTypes,BESENObject,BESENObjectFunction,BESENValue,BESENObjectPropertyDescriptor,
     BESENLexicalEnvironment,BESENContext,BESENASTNodes;

type TBESENObjectDeclaredFunctionParameters=array of TBESENString;

     TBESENObjectDeclaredFunction=class(TBESENObjectFunction)
      public
       LexicalEnvironment:TBESENLexicalEnvironment;
       Node:TBESENASTNodeFunctionLiteral;
       Parameters:TBESENObjectDeclaredFunctionParameters;
       Container:TBESENFunctionLiteralContainer;
       ContextCache:TBESENContextCache;
       constructor Create(AInstance:TObject;APrototype:TBESENObject=nil;AHasPrototypeProperty:longbool=false); overload; override;
       destructor Destroy; override;
       function GetEx(const P:TBESENString;var AResult:TBESENValue;var Descriptor:TBESENObjectPropertyDescriptor;Base:TBESENObject=nil;Hash:TBESENHash=0):boolean; override;
       function GetIndex(const Index,ID:integer;var AResult:TBESENValue;Base:TBESENObject=nil):boolean; override;
       procedure Construct(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var AResult:TBESENValue); override;
       procedure Call(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var AResult:TBESENValue); override;
       procedure CallEx(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var AResult:TBESENValue;IsConstruct:boolean); virtual;
       function HasConstruct:TBESENBoolean; override;
       function HasCall:TBESENBoolean; override;
       procedure Finalize; override;
       procedure Mark; override;
     end;

implementation

uses BESEN,BESENErrors,BESENUtils,BESENCode,BESENDeclarativeEnvironmentRecord;

constructor TBESENObjectDeclaredFunction.Create(AInstance:TObject;APrototype:TBESENObject=nil;AHasPrototypeProperty:longbool=false);
begin
 inherited Create(AInstance,APrototype,AHasPrototypeProperty);
 ObjectClassName:='Function';
 ObjectName:='declared';
 LexicalEnvironment:=nil;
 Node:=nil;
 Parameters:=nil;
 Container:=nil;
 ContextCache:=TBESENContextCache.Create(Instance);
end;

destructor TBESENObjectDeclaredFunction.Destroy;
begin
 SetLength(Parameters,0);
 BESENFreeAndNil(ContextCache);
 inherited Destroy;
end;

function TBESENObjectDeclaredFunction.GetEx(const P:TBESENString;var AResult:TBESENValue;var Descriptor:TBESENObjectPropertyDescriptor;Base:TBESENObject=nil;Hash:TBESENHash=0):boolean;
begin
 if not assigned(Base) then begin
  Base:=self;
 end;
 result:=inherited GetEx(P,AResult,Descriptor,Base,Hash);
 if Node.Body.IsStrict and (P='caller') then begin
  raise EBESENTypeError.Create('"caller" not allowed here');
 end;
end;

function TBESENObjectDeclaredFunction.GetIndex(const Index,ID:integer;var AResult:TBESENValue;Base:TBESENObject=nil):boolean;
begin
 if not assigned(Base) then begin
  Base:=self;
 end;
 result:=inherited GetIndex(Index,ID,AResult,Base);
 if TBESEN(Instance).IsStrict and (ID=TBESEN(Instance).KeyIDManager.CallerID) then begin
  raise EBESENTypeError.Create('"caller" not allowed here');
 end;
end;

procedure TBESENObjectDeclaredFunction.Construct(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var AResult:TBESENValue);
var r1:TBESENObject;
    r3:TBESENValue;
begin
 Get('prototype',r3);
 if (r3.ValueType=bvtOBJECT) and assigned(TBESENObject(r3.Obj)) then begin
  r1:=TBESENObject.Create(Instance,TBESENObject(r3.Obj),false);
 end else begin
  r1:=TBESENObject.Create(Instance,TBESEN(Instance).ObjectPrototype,false);
 end;
 TBESEN(Instance).GarbageCollector.Add(r1);
 r1.GarbageCollectorLock;
 try
  r1.Extensible:=true;
  CallEx(BESENObjectValue(r1),Arguments,CountArguments,AResult,true);
 finally
  r1.GarbageCollectorUnlock;
 end;
 if AResult.ValueType<>bvtOBJECT then begin
  AResult.ValueType:=bvtOBJECT;
  AResult.Obj:=r1;
 end;
end;

procedure TBESENObjectDeclaredFunction.Call(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var AResult:TBESENValue);
begin
 CallEx(ThisArgument,Arguments,CountArguments,AResult,false);
end;

procedure TBESENObjectDeclaredFunction.CallEx(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var AResult:TBESENValue;IsConstruct:boolean);
var NewContext:TBESENContext;
    LocalEnv:TBESENLexicalEnvironment;
 procedure SetThisBinding;
 begin
  if Node.Body.IsStrict then begin
   BESENCopyValue(NewContext.ThisBinding,ThisArgument);
  end else if ThisArgument.ValueType in [bvtUNDEFINED,bvtNULL] then begin
   NewContext.ThisBinding.ValueType:=bvtOBJECT;
   NewContext.ThisBinding.Obj:=TBESEN(Instance).ObjectGlobal;
  end else if ThisArgument.ValueType<>bvtOBJECT then begin
   TBESEN(Instance).ToObjectValue(ThisArgument,NewContext.ThisBinding);
  end else begin
   BESENCopyValue(NewContext.ThisBinding,ThisArgument);
  end;
 end;
begin
 if assigned(Node) and (assigned(Node.Body) and not (Node.Body.IsEmpty and (CountArguments=0))) then begin
  GarbageCollectorLock;
  try
   NewContext:=ContextCache.Pop;
   if assigned(NewContext) then begin
    LocalEnv:=NewContext.VariableEnvironment;
   end else begin
    NewContext:=TBESENContext.Create(Instance);
    LocalEnv:=nil;
   end;
   if assigned(LocalEnv) then begin
    LocalEnv.Outer:=LexicalEnvironment;
    NewContext.LexicalEnvironment:=LocalEnv;
    SetThisBinding;
    NewContext.InitializeDeclarationBindingInstantiation(Node.Body,self,false,Arguments,CountArguments,true);
   end else begin
    LocalEnv:=TBESEN(Instance).NewDeclarativeEnvironment(LexicalEnvironment,Node.Body.IsStrict,TBESENCode(Node.Body.Code).HasMaybeDirectEval);
    TBESEN(Instance).GarbageCollector.Add(LocalEnv);
    NewContext.LexicalEnvironment:=LocalEnv;
    NewContext.VariableEnvironment:=LocalEnv;
    SetThisBinding;
    NewContext.InitializeDeclarationBindingInstantiation(Node.Body,self,false,Arguments,CountArguments,false);
    if not IsConstruct then begin
     TBESEN(Instance).GarbageCollector.TriggerCollect;
    end;
   end;
   try
    if Node.Body.IsEmpty then begin
     AResult.ValueType:=bvtUNDEFINED;
    end else begin
     Node.ExecuteCode(NewContext,AResult);
    end;
    if (ContextCache.Count<TBESEN(Instance).MaxCountOfFreeContexts) and (Node.Body.DisableArgumentsObject and ((length(Node.Body.Functions)=0) and (assigned(Node.Body.Code) and ((TBESENCode(Node.Body.Code).CountFunctionLiteralContainers=0) and not (TBESENCode(Node.Body.Code).IsComplexFunction or TBESENCode(Node.Body.Code).HasLocalDelete))))) then begin
     NewContext.Reset;
     if (((assigned(LocalEnv) and (NewContext.VariableEnvironment=LocalEnv){ and (LocalEnv.Outer=Instance.GlobalLexicalEnvironment)}) and assigned(LocalEnv.EnvironmentRecord)) and (LocalEnv.EnvironmentRecord is TBESENDeclarativeEnvironmentRecord)) and not TBESENDeclarativeEnvironmentRecord(LocalEnv.EnvironmentRecord).Touched then begin
      LocalEnv.Outer:=nil;
      TBESENDeclarativeEnvironmentRecord(LocalEnv.EnvironmentRecord).Reset;
      ContextCache.Push(NewContext);
      NewContext:=nil;
     end;
    end;
   finally
    if assigned(NewContext) then begin
     NewContext.Destroy;
    end;
   end;
  finally
   GarbageCollectorUnlock;
  end;
 end else begin
  AResult.ValueType:=bvtUNDEFINED;
 end;
end;

function TBESENObjectDeclaredFunction.HasConstruct:TBESENBoolean;
begin
 result:=true;
end;

function TBESENObjectDeclaredFunction.HasCall:TBESENBoolean;
begin
 result:=true;
end;

procedure TBESENObjectDeclaredFunction.Finalize;
begin
 Container:=nil;
 LexicalEnvironment:=nil;
 inherited Finalize;
end;

procedure TBESENObjectDeclaredFunction.Mark;
begin
 if assigned(Container) then begin
  TBESEN(Instance).GarbageCollector.GrayIt(Container);
 end;
 if assigned(LexicalEnvironment) then begin
  TBESEN(Instance).GarbageCollector.GrayIt(LexicalEnvironment);
 end;
 if assigned(ContextCache) then begin
  ContextCache.Mark;
 end;
 inherited Mark;
end;

end.
