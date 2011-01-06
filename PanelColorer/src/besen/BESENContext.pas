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
unit BESENContext;
{$i BESEN.inc}

interface

uses BESENConstants,BESENTypes,BESENValue,BESENBaseObject,
     BESENCollectorObject,BESENObjectPropertyDescriptor,
     BESENLexicalEnvironment,BESENASTNodes,
     BESENEnvironmentRecord,BESENStringTree,
     BESENObjectFunctionArguments;

type TBESENContextCache=class;

     TBESENContext=class(TBESENCollectorObject)
      private
       Descriptor,OwnDescriptor:TBESENObjectPropertyDescriptor;
       Temp:TBESENValue;
      public
       Cache:TBESENContextCache;
       CachePrevious,CacheNext:TBESENContext;
       Previous,Next:TBESENContext;
       CodeContext:TObject;
       LexicalEnvironment:TBESENLexicalEnvironment;
       VariableEnvironment:TBESENLexicalEnvironment;
       ThisBinding:TBESENValue;
       constructor Create(AInstance:TObject); overload; override;
       destructor Destroy; override;
       procedure Reset;
       function CreateArgumentsObject(const Body:TBESENASTNodeFunctionBody;const FunctionObject:TObject;Arguments:PPBESENValues;CountArguments:integer;const Env:TBESENEnvironmentRecord;const IsStrict:longbool):TBESENObjectFunctionArguments;
       procedure InitializeDeclarationBindingInstantiation(const Body:TBESENASTNodeFunctionBody;const FunctionObject:TObject;const IsEval:boolean;Arguments:PPBESENValues;CountArguments:integer;const Reinitialize:boolean);
       procedure Mark;
     end;

     TBESENContextCacheItems=array of TBESENContext;

     TBESENContextCache=class(TBESENCollectorObject)
      public
       First,Last:TBESENContext;
       Count:integer;
       constructor Create(AInstance:TObject); overload; override;
       destructor Destroy; override;
       procedure Clear;
       procedure Cleanup;
       procedure Push(Context:TBESENContext);
       function Pop:TBESENContext;
       procedure Mark;
     end;

implementation

uses BESEN,BESENUtils,BESENDeclarativeEnvironmentRecord,BESENObject,
     BESENArrayUtils,BESENCodeContext,BESENErrors,
     BESENObjectDeclaredFunction,BESENObjectArgGetterFunction,
     BESENObjectArgSetterFunction;

constructor TBESENContext.Create(AInstance:TObject);
begin
 inherited Create(AInstance);
 Cache:=nil;
 CachePrevious:=nil;
 CacheNext:=nil;
 Previous:=TBESEN(Instance).ContextLast;
 Next:=nil;
 if assigned(Previous) then begin
  Previous.Next:=self;
 end else begin
  TBESEN(Instance).ContextFirst:=self;
 end;
 TBESEN(Instance).ContextLast:=self;
 CodeContext:=nil;
 LexicalEnvironment:=nil;
 VariableEnvironment:=nil;
 ThisBinding.ValueType:=bvtUNDEFINED;
end;

destructor TBESENContext.Destroy;
begin
 if assigned(Instance) and assigned(TBESEN(Instance).GarbageCollector) and (TBESEN(Instance).GarbageCollector.CurrentContext=self) then begin
  TBESEN(Instance).GarbageCollector.CurrentContext:=Next;
 end;
 if assigned(Previous) then begin
  Previous.Next:=Next;
 end else if TBESEN(Instance).ContextFirst=self then begin
  TBESEN(Instance).ContextFirst:=Next;
 end;
 if assigned(Next) then begin
  Next.Previous:=Previous;
 end else if TBESEN(Instance).ContextLast=self then begin
  TBESEN(Instance).ContextLast:=Previous;
 end;
 Next:=nil;
 Previous:=nil;
 BESENFreeAndNil(CodeContext);
 if assigned(Cache) then begin
  dec(Cache.Count);
  if assigned(CachePrevious) then begin
   CachePrevious.CacheNext:=CacheNext;
  end else if Cache.First=self then begin
   Cache.First:=CacheNext;
  end;
  if assigned(CacheNext) then begin
   CacheNext.CachePrevious:=CachePrevious;
  end else if Cache.Last=self then begin
   Cache.Last:=CachePrevious;
  end;
  Cache:=nil;
  CacheNext:=nil;
  CachePrevious:=nil;
 end;
 inherited Destroy;
end;

procedure TBESENContext.Reset;
begin
 CodeContext:=nil;
 ThisBinding.ValueType:=bvtUNDEFINED;
 while assigned(VariableEnvironment) and assigned(VariableEnvironment.EnvironmentRecord) do begin
  if VariableEnvironment.EnvironmentRecord is TBESENDeclarativeEnvironmentRecord then begin
   break;
  end;
  VariableEnvironment:=VariableEnvironment.Outer;
 end;
 if not (assigned(VariableEnvironment) and assigned(VariableEnvironment.EnvironmentRecord) and (VariableEnvironment.EnvironmentRecord is TBESENDeclarativeEnvironmentRecord)) then begin
  VariableEnvironment:=nil;
 end;
 LexicalEnvironment:=nil;
end;

function TBESENContext.CreateArgumentsObject(const Body:TBESENASTNodeFunctionBody;const FunctionObject:TObject;Arguments:PPBESENValues;CountArguments:integer;const Env:TBESENEnvironmentRecord;const IsStrict:longbool):TBESENObjectFunctionArguments;
var Len,Index,NamesCount:integer;
    Map:TBESENObject;
    Val:TBESENValue;
    Name:TBESENString;
    MappedNames:TBESENStringTree;
    MappedNamesCount:integer;
    StringTreeData:TBESENStringTreeData;
 function MakeArgGetter(const Name:TBESENString;const Env:TBESENEnvironmentRecord):TBESENObjectArgGetterFunction;
 begin
  result:=TBESENObjectArgGetterFunction.Create(Instance,TBESEN(Instance).ObjectFunctionPrototype,true);
  TBESEN(Instance).GarbageCollector.Add(result);
  result.Env:=Env;
  result.ArgName:=Name;
 end;
 function MakeArgSetter(const Name:TBESENString;const Env:TBESENEnvironmentRecord):TBESENObjectArgSetterFunction;
 begin
  result:=TBESENObjectArgSetterFunction.Create(Instance,TBESEN(Instance).ObjectFunctionPrototype,true);
  TBESEN(Instance).GarbageCollector.Add(result);
  result.Env:=Env;
  result.ArgName:=Name;
 end;
begin
 Len:=CountArguments;
 NamesCount:=length(TBESENObjectDeclaredFunction(FunctionObject).Parameters);
 result:=TBESENObjectFunctionArguments.Create(Instance,TBESEN(Instance).ObjectPrototype,false);
 result.GarbageCollectorLock;
 try
  result.IsStrict:=IsStrict;
  result.OverwriteData('length',BESENNumberValue(Len),[bopaWRITABLE,bopaCONFIGURABLE],false);
  Map:=TBESENObject.Create(Instance,TBESEN(Instance).ObjectPrototype);
  MappedNames:=TBESENStringTree.Create;
  MappedNamesCount:=0;
  try
   for Index:=Len-1 downto 0 do begin
    BESENCopyValue(Val,Arguments^[Index]^);
    result.DefineOwnProperty(BESENArrayIndexToStr(Index),BESENDataPropertyDescriptor(Val,[bopaWRITABLE,bopaENUMERABLE,bopaCONFIGURABLE]),false);
    if (not IsStrict) and (Index<NamesCount) then begin
     Name:=TBESENObjectDeclaredFunction(FunctionObject).Parameters[Index];
     if not MappedNames.Find(Name,StringTreeData) then begin
      StringTreeData.i:=Index;
      MappedNames.Add(Name,StringTreeData,true);
      inc(MappedNamesCount);
      Map.DefineOwnProperty(BESENArrayIndexToStr(Index),BESENAccessorPropertyDescriptor(MakeArgGetter(Name,Env),MakeArgSetter(Name,Env),[bopaCONFIGURABLE]),false);
     end;
    end;
   end;
  finally
   MappedNames.Destroy;
  end;
  if MappedNamesCount>0 then begin
   result.ParameterMap:=Map;
  end;
  if IsStrict then begin
   result.OverwriteAccessor('caller',TBESEN(Instance).ObjectThrowTypeErrorFunction,TBESEN(Instance).ObjectThrowTypeErrorFunction,[],false);
   result.OverwriteAccessor('callee',TBESEN(Instance).ObjectThrowTypeErrorFunction,TBESEN(Instance).ObjectThrowTypeErrorFunction,[],false);
  end else begin
   result.OverwriteData('callee',BESENObjectValueEx(TBESENObjectDeclaredFunction(FunctionObject)),[bopaWRITABLE,bopaCONFIGURABLE],false);
  end;
 finally
  result.GarbageCollectorUnlock;
 end;
end;

procedure TBESENContext.InitializeDeclarationBindingInstantiation(const Body:TBESENASTNodeFunctionBody;const FunctionObject:TObject;const IsEval:boolean;Arguments:PPBESENValues;CountArguments:integer;const Reinitialize:boolean);
var ConfigurableBindings,IsStrict,IsFunction:boolean;
    ParamCount,i,j:integer;
    v:TBESENValue;
    Env:TBESENEnvironmentRecord;
    ArgName,fn,dn:TBESENString;
    fd:TBESENASTNodeFunctionDeclaration;
    fo:TBESENObjectDeclaredFunction;
    ArgsObj:TBESENObjectFunctionArguments;
    d:TBESENASTNodeIdentifier;
    go:TBESENObject;
begin
 Env:=VariableEnvironment.EnvironmentRecord;
 ConfigurableBindings:=IsEval;
 IsStrict:=Body.IsStrict;
 IsFunction:=assigned(TBESENObjectDeclaredFunction(FunctionObject)) and Body.IsFunction;
 if IsFunction then begin
  ParamCount:=length(Body.Parameters);
  if (Env is TBESENDeclarativeEnvironmentRecord) and TBESENDeclarativeEnvironmentRecord(Env).IndexInitialized then begin
   for i:=0 to ParamCount-1 do begin
    j:=Body.Parameters[i].ParameterIndex;
    if i<CountArguments then begin
     BESENCopyValue(TBESENDeclarativeEnvironmentRecord(Env).HashValues[j]^,Arguments^[i]^);
    end else begin
     TBESENDeclarativeEnvironmentRecord(Env).HashValues[j]^.ValueType:=bvtUNDEFINED;
    end;
   end;
  end else begin
   for i:=0 to ParamCount-1 do begin
    ArgName:=Body.Parameters[i].Name;
    if i<CountArguments then begin
     BESENCopyValue(v,Arguments^[i]^);
    end else begin
     v.ValueType:=bvtUNDEFINED;
    end;
    if not Env.HasBindingEx(ArgName,Descriptor) then begin
     Env.CreateMutableBinding(ArgName);
    end;
    Env.SetMutableBindingEx(ArgName,v,IsStrict,Descriptor,OwnDescriptor,Temp);
   end;
  end;
 end;
 for i:=0 to length(Body.Functions)-1 do begin
  if assigned(Body.Functions[i]) and (Body.Functions[i] is TBESENASTNodeFunctionDeclaration) then begin
   if assigned(TBESENASTNodeFunctionDeclaration(Body.Functions[i]).Container.Literal) and assigned(TBESENASTNodeFunctionDeclaration(Body.Functions[i]).Container.Literal.Name) then begin
    fd:=TBESENASTNodeFunctionDeclaration(Body.Functions[i]);
    fn:=fd.Container.Literal.Name.Name;
    fo:=TBESEN(Instance).MakeFunction(fd.Container.Literal,fd.Container.Literal.Name.Name,LexicalEnvironment);
    if not Env.HasBindingEx(fn,Descriptor) then begin
     Env.CreateMutableBinding(fn,ConfigurableBindings);
    end else if Env=TBESEN(Instance).GlobalLexicalEnvironment.EnvironmentRecord then begin
     // ES5-errata fix
     go:=TBESEN(Instance).ObjectGlobal;
     go.GetProperty(fn,Descriptor);
     if (boppCONFIGURABLE in Descriptor.Presents) and (bopaCONFIGURABLE in Descriptor.Attributes) then begin
      if ConfigurableBindings then begin
       go.DefineOwnPropertyEx(fn,BESENDataPropertyDescriptor(BESENUndefinedValue,[bopaWRITABLE,bopaENUMERABLE,bopaCONFIGURABLE]),true,Descriptor);
      end else begin
       go.DefineOwnPropertyEx(fn,BESENDataPropertyDescriptor(BESENUndefinedValue,[bopaWRITABLE,bopaENUMERABLE]),true,Descriptor);
      end;
     end else if (([boppGETTER,boppSETTER]*Descriptor.Presents)<>[]) or ((([boppWRITABLE,boppENUMERABLE]*Descriptor.Presents)<>[boppWRITABLE,boppENUMERABLE]) or (([bopaWRITABLE,bopaENUMERABLE]*Descriptor.Attributes)<>[bopaWRITABLE,bopaENUMERABLE])) then begin
      BESENThrowTypeErrorDeclarationBindingInstantiationAtFunctionBinding(fn);
     end;
    end;
    Env.SetMutableBinding(fn,BESENObjectValue(fo),IsStrict);
   end;
  end;
 end;
 if not Reinitialize then begin
  if IsFunction and not (Body.DisableArgumentsObject or Env.HasBinding('arguments')) then begin
   ArgsObj:=CreateArgumentsObject(Body,TBESENObjectDeclaredFunction(FunctionObject),Arguments,CountArguments,Env,IsStrict);
   if IsStrict then begin
    Env.CreateImmutableBinding('arguments');
    Env.InitializeImmutableBinding('arguments',BESENObjectValue(ArgsObj));
   end else begin
    Env.CreateMutableBinding('arguments');
    Env.SetMutableBindingEx('arguments',BESENObjectValue(ArgsObj),false,Descriptor,OwnDescriptor,Temp);
   end;
  end;
  for i:=0 to length(Body.Variables)-1 do begin
   d:=Body.Variables[i];
   if assigned(d) and (length(d.Name)>0) then begin
    dn:=d.Name;
    if not Env.HasBindingEx(dn,Descriptor) then begin
     Env.CreateMutableBinding(dn,ConfigurableBindings);
     Env.SetMutableBindingEx(dn,BESENUndefinedValue,IsStrict,Descriptor,OwnDescriptor,Temp);
    end;
   end;
  end;
 end;
 if Env is TBESENDeclarativeEnvironmentRecord then begin
  TBESENDeclarativeEnvironmentRecord(Env).Touched:=false;
 end;
end;

procedure TBESENContext.Mark;
var i:integer;
begin
 if assigned(LexicalEnvironment) then begin
  TBESEN(Instance).GarbageCollector.GrayIt(LexicalEnvironment);
 end;
 if assigned(VariableEnvironment) then begin
  TBESEN(Instance).GarbageCollector.GrayIt(VariableEnvironment);
 end;                        
 TBESEN(Instance).GarbageCollector.GrayValue(ThisBinding);
 if assigned(CodeContext) then begin
  if assigned(TBESENCodeContext(CodeContext).Code) then begin
   TBESENCodeContext(CodeContext).Code.Mark;
  end;
  for i:=0 to length(TBESENCodeContext(CodeContext).RegisterValues)-1 do begin
   TBESEN(Instance).GarbageCollector.GrayValue(TBESENCodeContext(CodeContext).RegisterValues[i]);
  end;
 end;
end;

constructor TBESENContextCache.Create(AInstance:TObject);
begin
 inherited Create(AInstance);
 First:=nil;
 Last:=nil;
 Count:=0;
end;

destructor TBESENContextCache.Destroy;
begin
 Clear;
 inherited Destroy;
end;

procedure TBESENContextCache.Clear;
var CurrentContext:TBESENContext;
begin
 while true do begin
  CurrentContext:=Pop;
  if assigned(CurrentContext) then begin
   BESENFreeAndNil(CurrentContext);
  end else begin
   break;
  end;
 end;
end;

procedure TBESENContextCache.Cleanup;
var CurrentContext:TBESENContext;
begin
 while Count>TBESEN(Instance).MaxCountOfFreeContexts do begin
  CurrentContext:=Pop;
  if assigned(CurrentContext) then begin
   BESENFreeAndNil(CurrentContext);
  end else begin
   break;
  end;
 end;
end;

procedure TBESENContextCache.Push(Context:TBESENContext);
begin
 Context.Cache:=self;
 if assigned(Last) then begin
  Last.CacheNext:=Context;
  Context.CachePrevious:=Last;
  Context.CacheNext:=nil;
  Last:=Context;
 end else begin
  First:=Context;
  Last:=Context;
  Context.CachePrevious:=nil;
  Context.CacheNext:=nil;
 end;
 inc(Count);
end;

function TBESENContextCache.Pop:TBESENContext;
begin
 if assigned(Last) then begin
  result:=Last;
  if assigned(result.CachePrevious) then begin
   result.CachePrevious.CacheNext:=result.CacheNext;
  end else if First=result then begin
   First:=result.CacheNext;
  end;
  if assigned(result.CacheNext) then begin
   result.CacheNext.CachePrevious:=result.CachePrevious;
  end else if Last=result then begin
   Last:=result.CachePrevious;
  end;
  result.Cache:=nil;
  result.CachePrevious:=nil;
  result.CacheNext:=nil;
  dec(Count);
 end else begin
  result:=nil;
  Count:=0;
 end;
end;

procedure TBESENContextCache.Mark;
var CurrentContext:TBESENContext;
begin
 CurrentContext:=First;
 while assigned(CurrentContext) do begin
  CurrentContext.Mark;
  CurrentContext:=CurrentContext.CacheNext;
 end;
end;

end.
