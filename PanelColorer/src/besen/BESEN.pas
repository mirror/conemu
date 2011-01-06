(*******************************************************************************
                                    B E S E N
********************************************************************************
 Version: See at line in BESENVersionConstants.pas, which contains "BESENVersion"
--------------------------------------------------------------------------------

 Copyright (C) 2009-2010, Benjamin 'BeRo' Rosseaux <benjamin@rosseaux.com>

 Website: www.rosseaux.com

********************************************************************************

BESEN is an acronym for "Bero's EcmaScript Engine", and it is a complete
ECMAScript Fifth Edition Implemention in Object Pascal, which is compilable
with Delphi >=7 and FreePascal >= 2.5.1 (maybe also 2.4.1).

BESEN contains the following features:

- Complete implementation of the ECMAScript Fifth Edition standard
- Own bytecode-based ECMA262-complaint Regular Expression Engine
- Incremental praise/exact mark-and-sweep garbage collector
- Unicode UTF8/UCS2/UTF16/UCS4/UTF32 support (on ECMAScript level, UCS2/UTF16)
- Compatibility modes, for example also a facile JavaScript compatibility mode
- Bytecode compiler
- Call-Subroutine-Threaded Register-based virtual machine
- Context-Threaded 32-bit x86 and 64-bit x64/AMD64 Just-in-Time Compiler
- Constant folding
- Dead code elimination
- Abstract-Syntax-Tree based optimizations
- Type inference (both exact and speculative)
- Polymorphic Inline Cache based on object structure and property key IDs
- Perfomance optimized hash maps
- Self balanced trees

To-Do:

- Bytecode peephole optimization
- Aggressive bytecode copy propagation (for less read/write register access)
- Optimizing bytecode opcode set, adding more specialized combined superopcodes
- Implement ARM EABI VFPv3 Just-In-time Compiler
- Regular Expression Just-In-Compiler
- Checking/testing the code
- Fix bugs :-)

********************************************************************************
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
unit BESEN;
{$i BESEN.inc}

interface

uses SysUtils,Classes,Math,SyncObjs,TypInfo,Variants,BESENVersionConstants,
     BESENConstants,BESENTypes,BESENCharset,BESENStringUtils,BESENOpcodes,
     BESENNativeCodeMemoryManager,BESENValueContainer,BESENObject,
     BESENSelfBalancedTree,BESENGlobals,BESENErrors,
     BESENPointerSelfBalancedTree,BESENInt64SelfBalancedTree,
     BESENPointerList,BESENStringList,BESENIntegerList,BESENHashUtils,
     BESENHashMap,BESENBaseObject,BESENCollectorObject,BESENCollector,
     BESENGarbageCollector,BESENValue,BESENObjectPropertyDescriptor,
     BESENRegExp,BESENCode,BESENCodeContext,BESENContext,
     BESENEnvironmentRecord,BESENDeclarativeEnvironmentRecord,
     BESENObjectEnvironmentRecord,BESENLexicalEnvironment,
     BESENASTNodes,BESENCodeGeneratorContext,BESENParser,
     BESENObjectPrototype,BESENObjectFunction,
     BESENObjectConstructor,BESENObjectGlobal,
     BESENObjectJSON,BESENObjectMath,BESENObjectFunctionArguments,
     BESENObjectFunctionPrototype,BESENObjectFunctionConstructor,
     BESENObjectDeclaredFunction,BESENObjectThrowTypeErrorFunction,
     BESENObjectArgGetterFunction,BESENObjectArgSetterFunction,
     BESENObjectBindingFunction,BESENObjectBoolean,
     BESENObjectBooleanPrototype,BESENObjectBooleanConstructor,
     BESENObjectRegExp,BESENObjectRegExpPrototype,
     BESENObjectRegExpConstructor,BESENObjectDate,
     BESENObjectDatePrototype,BESENObjectDateConstructor,
     BESENObjectError,BESENObjectErrorPrototype,
     BESENObjectErrorConstructor,BESENObjectNumber,
     BESENObjectNumberPrototype,BESENObjectNumberConstructor,
     BESENObjectString,BESENObjectStringPrototype,
     BESENObjectStringConstructor,BESENObjectArray,
     BESENObjectArrayPrototype,BESENObjectArrayConstructor,
     BESENNativeObject,BESENRandomGenerator,BESENKeyIDManager,
     BESENRegExpCache,BESENEvalCacheItem,BESENEvalCache,
     BESENCompiler,BESENDecompiler,BESENLocale,
     BESENCodeSnapshot;

type TBESEN=class;

     TBESENTransitSecurityDomain=procedure(const Instance:TBESEN;const NewSecurityDomain:pointer) of object;

     TBESENTraceHook=function(const Instance:TBESEN;const Context:TBESENContext;const FunctionBody:TBESENASTNodeFunctionBody;PC:TBESENUINT32;const TraceType:TBESENTraceType):boolean of object;

     TBESENPeriodicHook=function(const Instance:TBESEN):boolean of object;

     TBESEN=class
      public
       CriticalSection:TCriticalSection;
       Collector:TBESENCollector;
       GarbageCollector:TBESENGarbageCollector;
       KeyIDManager:TBESENKeyIDManager;
       ObjectStructureIDManager:TBESENObjectStructureIDManager;
       RegExpCache:TBESENRegExpCache;
       DefaultRegExp:TBESENRegExp;
       EvalCache:TBESENEvalCache;
       Compiler:TBESENCompiler;
       Decompiler:TBESENDecompiler;
       CodeSnapshot:TBESENCodeSnapshot;
       NativeCodeMemoryManager:TBESENNativeCodeMemoryManager;
       InlineCacheEnabled:TBESENBoolean;
       ContextFirst,ContextLast:TBESENContext;
       CodeFirst,CodeLast:TBESENCode;
       ProgramNodes:TBESENPointerSelfBalancedTree;
       IsStrict:longbool;
       Compatibility:longword;
       RecursionLimit:integer;
       SecurityDomain:pointer;
       TransitSecurityDomain:TBESENTransitSecurityDomain;
       UseSecurity:TBESENBoolean;
       CodeLineInfo:TBESENBoolean;
       CodeTracable:TBESENBoolean;
       TraceHook:TBESENTraceHook;
       PeriodicHook:TBESENPeriodicHook;
       LineNumber:TBESENUINT32;
       RandomGenerator:TBESENRandomGenerator;
       RegExpMaxStatesHoldInMemory:integer;
       JITLoopCompileThreshold:longword;
       MaxCountOfFreeCodeContexts:integer;
       MaxCountOfFreeContexts:integer;
       ObjectPrototype:TBESENObjectPrototype;
       ObjectConstructor:TBESENObjectConstructor;
       ObjectFunctionPrototype:TBESENObjectFunctionPrototype;
       ObjectFunctionConstructor:TBESENObjectFunctionConstructor;
       ObjectBooleanPrototype:TBESENObjectBooleanPrototype;
       ObjectBooleanConstructor:TBESENObjectBooleanConstructor;
       ObjectRegExpPrototype:TBESENObjectRegExpPrototype;
       ObjectRegExpConstructor:TBESENObjectRegExpConstructor;
       ObjectDatePrototype:TBESENObjectDatePrototype;
       ObjectDateConstructor:TBESENObjectDateConstructor;
       ObjectErrorPrototype:TBESENObjectErrorPrototype;
       ObjectEvalErrorPrototype:TBESENObjectErrorPrototype;
       ObjectRangeErrorPrototype:TBESENObjectErrorPrototype;
       ObjectReferenceErrorPrototype:TBESENObjectErrorPrototype;
       ObjectSyntaxErrorPrototype:TBESENObjectErrorPrototype;
       ObjectTypeErrorPrototype:TBESENObjectErrorPrototype;
       ObjectURIErrorPrototype:TBESENObjectErrorPrototype;
       ObjectErrorConstructor:TBESENObjectErrorConstructor;
       ObjectEvalErrorConstructor:TBESENObjectErrorConstructor;
       ObjectRangeErrorConstructor:TBESENObjectErrorConstructor;
       ObjectReferenceErrorConstructor:TBESENObjectErrorConstructor;
       ObjectSyntaxErrorConstructor:TBESENObjectErrorConstructor;
       ObjectTypeErrorConstructor:TBESENObjectErrorConstructor;
       ObjectURIErrorConstructor:TBESENObjectErrorConstructor;
       ObjectNumberConstructor:TBESENObjectNumberConstructor;
       ObjectNumberPrototype:TBESENObjectNumberPrototype;
       ObjectStringPrototype:TBESENObjectStringPrototype;
       ObjectStringConstructor:TBESENObjectStringConstructor;
       ObjectArrayPrototype:TBESENObjectArrayPrototype;
       ObjectArrayConstructor:TBESENObjectArrayConstructor;
       ObjectJSON:TBESENObjectJSON;
       ObjectMath:TBESENObjectMath;
       ObjectEmpty:TBESENObject;
       ObjectThrowTypeErrorFunction:TBESENObjectThrowTypeErrorFunction;
       ObjectGlobal:TBESENObjectGlobal;
       ObjectGlobalEval:TBESENObjectFunction;
       GlobalLexicalEnvironment:TBESENLexicalEnvironment;
       ObjectNumberConstructorValue:TBESENValue;
       ObjectStringConstructorValue:TBESENValue;
       constructor Create(ACompatibility:longword=0); overload;
       destructor Destroy; override;
       procedure Lock;
       procedure Unlock;
       procedure LockObject(Obj:TBESENGarbageCollectorObject);
       procedure UnlockObject(Obj:TBESENGarbageCollectorObject);
       procedure LockValue(const Value:TBESENValue);
       procedure UnlockValue(const Value:TBESENValue);
       function GetRandom:longword;
       procedure RegisterNativeObject(const AName:TBESENANSISTRING;const AClass:TBESENNativeObjectClass;const Attributes:TBESENObjectPropertyDescriptorAttributes=[bopaWRITABLE,bopaCONFIGURABLE]);
       procedure FunctionCall(Obj:TBESENObject;const ThisArgument:TBESENValue;const Arguments:array of TBESENValue;var AResult:TBESENValue);
       procedure FunctionConstruct(Obj:TBESENObject;const ThisArgument:TBESENValue;const Arguments:array of TBESENValue;var AResult:TBESENValue);
       function MakeError(const Name:TBESENString;var ConstructorObject:TBESENObjectErrorConstructor;const ProtoProto:TBESENObject):TBESENObjectErrorPrototype;
       function LoadFromStream(const Stream:TStream):TBESENASTNode;
       procedure SaveToStream(const Stream:TStream;const RootNode:TBESENASTNode);
       function Compile(InputSource:TBESENUTF8STRING;const Parameters:TBESENUTF8STRING='';IsFunction:TBESENBoolean=false;IsJSON:TBESENBoolean=false):TBESENASTNode;
       function Decompile(RootNode:TBESENASTNode):TBESENUTF8STRING;
       function ObjectInstanceOf(const v:TBESENValue;Obj:TBESENObject):boolean;
       function MakeFunction(Node:TBESENASTNodeFunctionLiteral;Name:TBESENString;ParentLexicalEnvironment:TBESENLexicalEnvironment=nil):TBESENObjectDeclaredFunction;
       function Execute(Source:TBESENUTF8STRING;const ThisArgument:TBESENValue;const PrecompiledASTNode:TBESENASTNode=nil;const IsEval:TBESENBoolean=false):TBESENValue; overload;
       function Execute(Source:TBESENUTF8STRING;const PrecompiledASTNode:TBESENASTNode=nil;const IsEval:TBESENBoolean=false):TBESENValue; overload;
       function Eval(Source:TBESENUTF8STRING;const ThisArgument:TBESENValue;const PrecompiledASTNode:TBESENASTNode=nil):TBESENValue; overload;
       function Eval(Source:TBESENUTF8STRING;const PrecompiledASTNode:TBESENASTNode=nil):TBESENValue; overload;
       function JSONEval(Source:TBESENUTF8STRING;const PrecompiledASTNode:TBESENASTNode=nil):TBESENValue; overload;
       function NewDeclarativeEnvironment(const Environment:TBESENLexicalEnvironment;const IsItStrict,HasMaybeDirectEval:TBESENBoolean):TBESENLexicalEnvironment;
       function NewObjectEnvironment(const BindingObject:TBESENObject;const Environment:TBESENLexicalEnvironment;const IsItStrict,HasMaybeDirectEval:TBESENBoolean):TBESENLexicalEnvironment;
       function ParseNumber(const s:TBESENString):TBESENNumber;
       procedure ToPrimitiveValue(const AValue,AType:TBESENValue;var AResult:TBESENValue); overload;
       procedure ToPrimitiveValue(const AValue:TBESENValue;var AResult:TBESENValue); overload;
       procedure ToBooleanValue(const AValue:TBESENValue;var AResult:TBESENValue);
       procedure ToNumberValue(const AValue:TBESENValue;var AResult:TBESENValue);
       procedure ToIntegerValue(const AValue:TBESENValue;var AResult:TBESENValue);
       procedure ToStringValue(const AValue:TBESENValue;var AResult:TBESENValue);
       procedure ToObjectValue(const AValue:TBESENValue;var AResult:TBESENValue);
       function ToInt(const AValue:TBESENValue):int64;
       function ToInt32(const AValue:TBESENValue):TBESENINT32;
       function ToUInt32(const AValue:TBESENValue):TBESENUINT32;
       function ToInt16(const AValue:TBESENValue):TBESENINT16;
       function ToUInt16(const AValue:TBESENValue):TBESENUINT16;
       function ToBool(const AValue:TBESENValue):TBESENBoolean;
       function ToNum(const AValue:TBESENValue):TBESENNumber;
       function ToStr(const AValue:TBESENValue):TBESENString;
       function ToObj(const AValue:TBESENValue):TBESENObject;
       procedure EqualityExpressionSub(const a,b:TBESENValue;var AResult:TBESENValue);
       function EqualityExpressionEquals(const a,b:TBESENValue):boolean;
       function EqualityExpressionCompare(const a,b:TBESENValue):integer;
       procedure FromPropertyDescriptor(const Descriptor:TBESENObjectPropertyDescriptor;var AResult:TBESENValue);
       procedure ToPropertyDescriptor(const v:TBESENValue;var AResult:TBESENObjectPropertyDescriptor);
       function SameValue(const va,vb:TBESENValue):TBESENBoolean; overload;
       function SameValue(const oa,ob:TBESENObject):TBESENBoolean; overload;
       function SameObject(const oa,ob:TBESENObject):TBESENBoolean;
       procedure AddProgramNode(Node:TBESENASTNodeProgram);
       procedure RemoveProgramNode(Node:TBESENASTNodeProgram);
       procedure GlobalEval(const Context:TBESENContext;const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;const DirectCall:boolean;var AResult:TBESENValue);
       procedure ObjectCallConstruct(Obj:TBESENObject;const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;Construct:boolean;var AResult:TBESENValue);
       procedure ObjectCall(Obj:TBESENObject;const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var AResult:TBESENValue);
       procedure ObjectConstruct(Obj:TBESENObject;const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var AResult:TBESENValue);
       property PropertyCacheEnabled:TBESENBoolean read InlineCacheEnabled write InlineCacheEnabled;
     end;

implementation

uses BESENLexer,BESENDTOA,BESENNumberUtils,BESENUtils,BESENStringTree,BESENDateUtils,BESENArrayUtils;

constructor TBESEN.Create(ACompatibility:longword=0);
var v:TBESENValue;
begin
 inherited Create;
 InlineCacheEnabled:=true;
 ContextFirst:=nil;
 ContextLast:=nil;
 CodeFirst:=nil;
 CodeLast:=nil;
 CriticalSection:=TCriticalSection.Create;
 Collector:=TBESENCollector.Create(self);
 GarbageCollector:=TBESENGarbageCollector.Create(self);
 KeyIDManager:=TBESENKeyIDManager.Create(self);
 ObjectStructureIDManager:=TBESENObjectStructureIDManager.Create(self);
 Compiler:=TBESENCompiler.Create(self);
 Decompiler:=TBESENDecompiler.Create(self);
 CodeSnapshot:=TBESENCodeSnapshot.Create(self);
 NativeCodeMemoryManager:=TBESENNativeCodeMemoryManager.Create;
 ProgramNodes:=TBESENPointerSelfBalancedTree.Create;
 IsStrict:=false;
 Compatibility:=ACompatibility;
 RecursionLimit:=-1;
 SecurityDomain:=nil;
 TransitSecurityDomain:=nil;
 UseSecurity:=false;
 CodeLineInfo:=true;
 CodeTracable:=false;
 TraceHook:=nil;
 PeriodicHook:=nil;
 LineNumber:=0;
 RandomGenerator:=TBESENRandomGenerator.Create(self);
 RegExpMaxStatesHoldInMemory:=breMAXSTATESHOLDINMEMORY;
 JITLoopCompileThreshold:=BESEN_JIT_LOOPCOMPILETHRESHOLD;
 MaxCountOfFreeCodeContexts:=BESENMaxCountOfFreeCodeContexts;
 MaxCountOfFreeContexts:=BESENMaxCountOfFreeContexts;

 RegExpCache:=TBESENRegExpCache.Create(self);
 DefaultRegExp:=TBESENRegExp.Create(selF);

 EvalCache:=TBESENEvalCache.Create(self);

 v:=BESENEmptyValue;

 ObjectPrototype:=nil;

 begin
  ObjectFunctionPrototype:=TBESENObjectFunctionPrototype.Create(self,ObjectPrototype,false);
  GarbageCollector.AddRoot(ObjectFunctionPrototype);
  ObjectFunctionPrototype.RegisterNativeFunction('toString',ObjectFunctionPrototype.NativeToString,1,[bopaWRITABLE,bopaCONFIGURABLE],false);
  ObjectFunctionPrototype.RegisterNativeFunction('apply',ObjectFunctionPrototype.NativeApply,2,[bopaWRITABLE,bopaCONFIGURABLE],false);
  ObjectFunctionPrototype.RegisterNativeFunction('call',ObjectFunctionPrototype.NativeCall,1,[bopaWRITABLE,bopaCONFIGURABLE],false);
  ObjectFunctionPrototype.RegisterNativeFunction('bind',ObjectFunctionPrototype.NativeBind,1,[bopaWRITABLE,bopaCONFIGURABLE],false);

  ObjectPrototype:=TBESENObjectPrototype.Create(self,nil,false);
  GarbageCollector.AddRoot(ObjectPrototype);

  ObjectFunctionPrototype.Prototype:=ObjectPrototype;

  ObjectFunctionConstructor:=TBESENObjectFunctionConstructor.Create(self,ObjectFunctionPrototype,true);
  GarbageCollector.AddRoot(ObjectFunctionConstructor);

  ObjectFunctionPrototype.OverwriteData('constructor',BESENObjectValue(ObjectFunctionConstructor),[bopaWRITABLE,bopaCONFIGURABLE]);
 end;

 begin
  ObjectBooleanPrototype:=TBESENObjectBooleanPrototype.Create(self,ObjectPrototype,false);
  GarbageCollector.AddRoot(ObjectBooleanPrototype);

  ObjectBooleanConstructor:=TBESENObjectBooleanConstructor.Create(self,ObjectFunctionPrototype,false);
  GarbageCollector.AddRoot(ObjectBooleanConstructor);

  ObjectBooleanConstructor.OverwriteData('prototype',BESENObjectValue(ObjectBooleanPrototype),[]);

  ObjectBooleanPrototype.OverwriteData('constructor',BESENObjectValue(ObjectBooleanConstructor),[bopaWRITABLE,bopaCONFIGURABLE]);
 end;

 begin
  ObjectRegExpPrototype:=TBESENObjectRegExpPrototype.Create(self,ObjectPrototype,false);
  GarbageCollector.AddRoot(ObjectRegExpPrototype);

  ObjectRegExpConstructor:=TBESENObjectRegExpConstructor.Create(self,ObjectFunctionPrototype,false);
  GarbageCollector.AddRoot(ObjectRegExpConstructor);

  ObjectRegExpConstructor.OverwriteData('prototype',BESENObjectValue(ObjectRegExpPrototype),[]);

  ObjectRegExpPrototype.OverwriteData('constructor',BESENObjectValue(ObjectRegExpConstructor),[bopaWRITABLE,bopaCONFIGURABLE]);
 end;

 begin
  ObjectDatePrototype:=TBESENObjectDatePrototype.Create(self,ObjectPrototype,false);
  GarbageCollector.AddRoot(ObjectDatePrototype);

  ObjectDateConstructor:=TBESENObjectDateConstructor.Create(self,ObjectFunctionPrototype,false);
  GarbageCollector.AddRoot(ObjectDateConstructor);

  ObjectDateConstructor.OverwriteData('prototype',BESENObjectValue(ObjectDatePrototype),[]);

  ObjectDatePrototype.OverwriteData('constructor',BESENObjectValue(ObjectDateConstructor),[bopaWRITABLE,bopaCONFIGURABLE]);
 end;

 begin
  ObjectErrorPrototype:=TBESENObjectErrorPrototype.Create(self,ObjectPrototype,false);
  GarbageCollector.AddRoot(ObjectErrorPrototype);

  ObjectErrorConstructor:=TBESENObjectErrorConstructor.Create(self,ObjectFunctionPrototype,false);
  GarbageCollector.AddRoot(ObjectErrorConstructor);
  ObjectErrorConstructor.OverwriteData('prototype',BESENObjectValueEx(ObjectErrorPrototype),[]);
  v:=BESENStringValue('Error');
  ObjectErrorConstructor.OverwriteData('name',v,[bopaWRITABLE,bopaCONFIGURABLE]);

  v:=BESENObjectValue(ObjectErrorConstructor);
  ObjectErrorPrototype.OverwriteData('constructor',v,[bopaWRITABLE,bopaCONFIGURABLE]);

  ObjectEvalErrorPrototype:=MakeError('EvalError',ObjectEvalErrorConstructor,ObjectErrorPrototype);
  ObjectRangeErrorPrototype:=MakeError('RangeError',ObjectRangeErrorConstructor,ObjectErrorPrototype);
  ObjectReferenceErrorPrototype:=MakeError('ReferenceError',ObjectReferenceErrorConstructor,ObjectErrorPrototype);
  ObjectSyntaxErrorPrototype:=MakeError('SyntaxError',ObjectSyntaxErrorConstructor,ObjectErrorPrototype);
  ObjectTypeErrorPrototype:=MakeError('TypeError',ObjectTypeErrorConstructor,ObjectErrorPrototype);
  ObjectURIErrorPrototype:=MakeError('URIError',ObjectURIErrorConstructor,ObjectErrorPrototype);

  GarbageCollector.AddRoot(ObjectEvalErrorPrototype);
  GarbageCollector.AddRoot(ObjectRangeErrorPrototype);
  GarbageCollector.AddRoot(ObjectReferenceErrorPrototype);
  GarbageCollector.AddRoot(ObjectSyntaxErrorPrototype);
  GarbageCollector.AddRoot(ObjectTypeErrorPrototype);
  GarbageCollector.AddRoot(ObjectURIErrorPrototype);

  GarbageCollector.AddRoot(ObjectEvalErrorConstructor);
  GarbageCollector.AddRoot(ObjectRangeErrorConstructor);
  GarbageCollector.AddRoot(ObjectReferenceErrorConstructor);
  GarbageCollector.AddRoot(ObjectSyntaxErrorConstructor);
  GarbageCollector.AddRoot(ObjectTypeErrorConstructor);
  GarbageCollector.AddRoot(ObjectURIErrorConstructor);
 end;

 begin
  ObjectNumberPrototype:=TBESENObjectNumberPrototype.Create(self,ObjectPrototype,false);
  GarbageCollector.AddRoot(ObjectNumberPrototype);

  ObjectNumberConstructor:=TBESENObjectNumberConstructor.Create(self,ObjectFunctionPrototype,false);
  GarbageCollector.AddRoot(ObjectNumberConstructor);

  ObjectNumberConstructor.OverwriteData('prototype',BESENObjectValue(ObjectNumberPrototype),[]);

  ObjectNumberPrototype.OverwriteData('constructor',BESENObjectValue(ObjectNumberConstructor),[bopaWRITABLE,bopaCONFIGURABLE]);
 end;

 begin
  ObjectStringPrototype:=TBESENObjectStringPrototype.Create(self,ObjectPrototype,false);
  GarbageCollector.AddRoot(ObjectStringPrototype);

  ObjectStringConstructor:=TBESENObjectStringConstructor.Create(self,ObjectFunctionPrototype,false);
  GarbageCollector.AddRoot(ObjectStringConstructor);

  ObjectStringConstructor.OverwriteData('prototype',BESENObjectValue(ObjectStringPrototype),[]);

  ObjectStringPrototype.OverwriteData('constructor',BESENObjectValue(ObjectStringConstructor),[bopaWRITABLE,bopaCONFIGURABLE]);
 end;

 begin
  ObjectArrayPrototype:=TBESENObjectArrayPrototype.Create(self,ObjectPrototype,false);
  GarbageCollector.AddRoot(ObjectArrayPrototype);

  ObjectArrayConstructor:=TBESENObjectArrayConstructor.Create(self,ObjectFunctionPrototype,false);
  GarbageCollector.AddRoot(ObjectArrayConstructor);

  ObjectArrayConstructor.OverwriteData('prototype',BESENObjectValue(ObjectArrayPrototype),[]);

  ObjectArrayPrototype.OverwriteData('constructor',BESENObjectValue(ObjectArrayConstructor),[bopaWRITABLE,bopaCONFIGURABLE]);
 end;

 begin
  ObjectJSON:=TBESENObjectJSON.Create(self,ObjectPrototype,false);
  GarbageCollector.AddRoot(ObjectJSON);
 end;

 begin
  ObjectMath:=TBESENObjectMath.Create(self,ObjectPrototype,false);
  GarbageCollector.AddRoot(ObjectMath);
 end;

 begin
  ObjectEmpty:=TBESENObject.Create(self,ObjectPrototype,false);
  GarbageCollector.AddRoot(ObjectEmpty);
 end;

 begin
  ObjectThrowTypeErrorFunction:=TBESENObjectThrowTypeErrorFunction.Create(self,ObjectFunctionPrototype,true);
  GarbageCollector.AddRoot(ObjectThrowTypeErrorFunction);
 end;

 begin
  ObjectConstructor:=TBESENObjectConstructor.Create(self,ObjectFunctionPrototype,false);
  GarbageCollector.AddRoot(ObjectConstructor);

  ObjectConstructor.OverwriteData('prototype',BESENObjectValue(ObjectPrototype),[]);

  ObjectPrototype.OverwriteData('constructor',BESENObjectValue(ObjectConstructor),[bopaWRITABLE,bopaCONFIGURABLE]);
 end;

 if (Compatibility and COMPAT_JS)<>0 then begin
  ObjectGlobal:=TBESENObjectGlobal.Create(self,ObjectPrototype,true);
 end else begin
  ObjectGlobal:=TBESENObjectGlobal.Create(self,nil,false);
 end;
 GarbageCollector.AddRoot(ObjectGlobal);

 ObjectGlobal.Get('eval',v);
 if v.ValueType=bvtOBJECT then begin
  ObjectGlobalEval:=TBESENObjectFunction(TBESENObject(v.Obj));
 end else begin
  ObjectGlobalEval:=nil;
 end;
 if (Compatibility and COMPAT_JS)<>0 then begin
  ObjectPrototype.OverwriteData('eval',v,[bopaWRITABLE,bopaCONFIGURABLE]);
 end;

 v:=BESENObjectValue(ObjectConstructor);
 ObjectGlobal.OverwriteData('Object',v,[bopaWRITABLE,bopaCONFIGURABLE]);

 v:=BESENObjectValue(ObjectFunctionConstructor);
 ObjectGlobal.OverwriteData('Function',v,[bopaWRITABLE,bopaCONFIGURABLE]);

 v:=BESENObjectValue(ObjectBooleanConstructor);
 ObjectGlobal.OverwriteData('Boolean',v,[bopaWRITABLE,bopaCONFIGURABLE]);

 v:=BESENObjectValue(ObjectRegExpConstructor);
 ObjectGlobal.OverwriteData('RegExp',v,[bopaWRITABLE,bopaCONFIGURABLE]);

 v:=BESENObjectValue(ObjectDateConstructor);
 ObjectGlobal.OverwriteData('Date',v,[bopaWRITABLE,bopaCONFIGURABLE]);

 v:=BESENObjectValue(ObjectNumberConstructor);
 ObjectGlobal.OverwriteData('Number',v,[bopaWRITABLE,bopaCONFIGURABLE]);

 v:=BESENObjectValue(ObjectStringConstructor);
 ObjectGlobal.OverwriteData('String',v,[bopaWRITABLE,bopaCONFIGURABLE]);

 v:=BESENObjectValue(ObjectArrayConstructor);
 ObjectGlobal.OverwriteData('Array',v,[bopaWRITABLE,bopaCONFIGURABLE]);

 v:=BESENObjectValue(ObjectJSON);
 ObjectGlobal.OverwriteData('JSON',v,[bopaWRITABLE,bopaCONFIGURABLE]);

 v:=BESENObjectValue(ObjectMath);
 ObjectGlobal.OverwriteData('Math',v,[bopaWRITABLE,bopaCONFIGURABLE]);

 v:=BESENObjectValue(ObjectErrorConstructor);
 ObjectGlobal.OverwriteData('Error',v,[bopaWRITABLE,bopaCONFIGURABLE]);

 v:=BESENObjectValue(ObjectEvalErrorConstructor);
 ObjectGlobal.OverwriteData('EvalError',v,[bopaWRITABLE,bopaCONFIGURABLE]);

 v:=BESENObjectValue(ObjectRangeErrorConstructor);
 ObjectGlobal.OverwriteData('RangeError',v,[bopaWRITABLE,bopaCONFIGURABLE]);

 v:=BESENObjectValue(ObjectReferenceErrorConstructor);
 ObjectGlobal.OverwriteData('ReferenceError',v,[bopaWRITABLE,bopaCONFIGURABLE]);

 v:=BESENObjectValue(ObjectSyntaxErrorConstructor);
 ObjectGlobal.OverwriteData('SyntaxError',v,[bopaWRITABLE,bopaCONFIGURABLE]);

 v:=BESENObjectValue(ObjectTypeErrorConstructor);
 ObjectGlobal.OverwriteData('TypeError',v,[bopaWRITABLE,bopaCONFIGURABLE]);

 v:=BESENObjectValue(ObjectURIErrorConstructor);
 ObjectGlobal.OverwriteData('URIError',v,[bopaWRITABLE,bopaCONFIGURABLE]);

 GlobalLexicalEnvironment:=NewObjectEnvironment(ObjectGlobal,nil,false,false);
 GarbageCollector.AddRoot(GlobalLexicalEnvironment);

 ObjectNumberConstructorValue:=BESENObjectValue(ObjectNumberConstructor);
 ObjectStringConstructorValue:=BESENObjectValue(ObjectStringConstructor);
end;

destructor TBESEN.Destroy;
begin
 TBESENObjectEnvironmentRecord(GlobalLexicalEnvironment.EnvironmentRecord).BindingObject:=nil;

 EvalCache.Destroy;
 
 GarbageCollector.Destroy;

 ProgramNodes.Destroy;

 DefaultRegExp.Destroy;

 RegExpCache.Destroy;

 Collector.Destroy;

 NativeCodeMemoryManager.Destroy;

 CodeSnapshot.Destroy;

 Decompiler.Destroy;

 Compiler.Destroy;

 ObjectStructureIDManager.Destroy;

 KeyIDManager.Destroy;

 CriticalSection.Destroy;
 inherited Destroy;
end;

procedure TBESEN.Lock;
begin
 CriticalSection.Enter;
end;

procedure TBESEN.Unlock;
begin
 CriticalSection.Leave;
end;

procedure TBESEN.LockObject(Obj:TBESENGarbageCollectorObject);
begin
 GarbageCollector.LockObject(Obj);
end;

procedure TBESEN.UnlockObject(Obj:TBESENGarbageCollectorObject);
begin
 GarbageCollector.UnlockObject(Obj);
end;

procedure TBESEN.LockValue(const Value:TBESENValue);
begin
 GarbageCollector.LockValue(Value);
end;

procedure TBESEN.UnlockValue(const Value:TBESENValue);
begin
 GarbageCollector.UnlockValue(Value);
end;

function TBESEN.GetRandom:longword;
begin
 result:=RandomGenerator.Get;
end;

procedure TBESEN.RegisterNativeObject(const AName:TBESENANSISTRING;const AClass:TBESENNativeObjectClass;const Attributes:TBESENObjectPropertyDescriptorAttributes=[bopaWRITABLE,bopaCONFIGURABLE]);
var v:TBESENValue;
begin
 v.ValueType:=bvtOBJECT;
 TBESENObject(v.Obj):=AClass.Create(self,ObjectPrototype);
 GarbageCollector.AddRoot(TBESENObject(v.Obj));
 ObjectGlobal.OverwriteData(AName,v,Attributes);
end;

function TBESEN.MakeError(const Name:TBESENString;var ConstructorObject:TBESENObjectErrorConstructor;const ProtoProto:TBESENObject):TBESENObjectErrorPrototype;
var v:TBESENValue;
begin
 result:=TBESENObjectErrorPrototype.Create(self,ProtoProto,false);

 ConstructorObject:=TBESENObjectErrorConstructor.Create(self,ObjectFunctionPrototype,false);
 ConstructorObject.OverwriteData('prototype',BESENObjectValueEx(result),[]);

 v:=BESENEmptyValue;
 v:=BESENObjectValue(ConstructorObject);
 result.OverwriteData('constructor',v,[bopaWRITABLE,bopaCONFIGURABLE]);

 v:=BESENStringValue(Name);
 result.OverwriteData('name',v,[bopaWRITABLE,bopaCONFIGURABLE]);
 result.OverwriteData('message',v,[bopaWRITABLE,bopaCONFIGURABLE]);

 v:=BESENNumberValue(1);
 result.OverwriteData('length',v,[bopaWRITABLE,bopaCONFIGURABLE]);

 v:=BESENObjectValue(result);
 ConstructorObject.OverwriteData('constructor',v,[bopaWRITABLE,bopaCONFIGURABLE]);
end;

function TBESEN.LoadFromStream(const Stream:TStream):TBESENASTNode;
begin
 result:=CodeSnapshot.LoadFromStream(Stream);
end;

procedure TBESEN.SaveToStream(const Stream:TStream;const RootNode:TBESENASTNode);
begin
 CodeSnapshot.SaveToStream(Stream,RootNode);
end;

function TBESEN.Compile(InputSource:TBESENUTF8STRING;const Parameters:TBESENUTF8STRING='';IsFunction:TBESENBoolean=false;IsJSON:TBESENBoolean=false):TBESENASTNode;
begin
 result:=Compiler.Compile(InputSource,Parameters,IsFunction,IsJSON);
end;

function TBESEN.Decompile(RootNode:TBESENASTNode):TBESENUTF8STRING;
begin
 result:=Decompiler.Decompile(RootNode);
end;

function TBESEN.ObjectInstanceOf(const v:TBESENValue;Obj:TBESENObject):boolean;
var vp:TBESENValue;
    o:TBESENObject;
begin
 if not assigned(Obj) then begin
  raise EBESENTypeError.Create('Null object');
 end;
 if Obj.HasHasInstance then begin
  result:=Obj.HasInstance(v);
 end else if (Compatibility and COMPAT_JS)<>0 then begin
  if (v.ValueType<>bvtOBJECT) or not assigned(TBESENObject(v.Obj)) then begin
   result:=false;
  end else begin
   Obj.Get('prototype',vp);
   result:=false;
   if (vp.ValueType=bvtOBJECT) and assigned(vp.Obj) then begin
    o:=TBESENObject(v.Obj);
    while assigned(o) do begin
     if o.Prototype=vp.Obj then begin
      result:=true;
      break;
     end;
     o:=o.Prototype;
    end;
   end;
  end;
 end else begin
  raise EBESENTypeError.Create('No hasInstance');
 end;
end;

procedure TBESEN.ObjectCallConstruct(Obj:TBESENObject;const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;Construct:boolean;var AResult:TBESENValue);
var OldRecursionLimit:integer;
    OldSecurityDomain:pointer;
begin
 if not assigned(Obj) then begin
  BESENThrowTypeError('Null object');
 end;
 OldRecursionLimit:=RecursionLimit;
 OldSecurityDomain:=SecurityDomain;
 if RecursionLimit=0 then begin
  BESENThrowRecursionLimitReached;
 end else if RecursionLimit>0 then begin
  dec(RecursionLimit);
 end;
 if UseSecurity and assigned(TransitSecurityDomain) and Obj.HasGetSecurityDomain then begin
  SecurityDomain:=Obj.GetSecurityDomain;
  if SecurityDomain<>OldSecurityDomain then begin
   TransitSecurityDomain(self,SecurityDomain);
  end;
 end;
 try
  if Construct then begin
   if Obj.HasConstruct then begin
    Obj.Construct(ThisArgument,Arguments,CountArguments,AResult);
   end else begin
    BESENThrowTypeError('No constructable');
   end;
  end else begin
   if Obj.HasCall then begin
    Obj.Call(ThisArgument,Arguments,CountArguments,AResult);
   end else begin
    BESENThrowTypeError('No callable');
   end;
  end;
 finally
  SecurityDomain:=OldSecurityDomain;
  RecursionLimit:=OldRecursionLimit;
 end;
end;

procedure TBESEN.ObjectCall(Obj:TBESENObject;const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var AResult:TBESENValue);
begin
 if UseSecurity or (RecursionLimit>=0) then begin
  ObjectCallConstruct(Obj,ThisArgument,Arguments,CountArguments,false,AResult);
 end else begin
  if assigned(Obj) then begin
   if Obj.HasCall then begin
    Obj.Call(ThisArgument,Arguments,CountArguments,AResult);
   end else begin
    BESENThrowTypeError('No callable');
   end;
  end else begin
   BESENThrowTypeError('Null object');
  end;
 end;
end;

procedure TBESEN.ObjectConstruct(Obj:TBESENObject;const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var AResult:TBESENValue);
begin
 if UseSecurity or (RecursionLimit>=0) then begin
  ObjectCallConstruct(Obj,ThisArgument,Arguments,CountArguments,true,AResult);
 end else begin
  if assigned(Obj) then begin
   if Obj.HasConstruct then begin
    Obj.Construct(ThisArgument,Arguments,CountArguments,AResult);
   end else begin
    BESENThrowTypeError('No constructable');
   end;
  end else begin
   BESENThrowTypeError('Null object');
  end;
 end;
end;

procedure TBESEN.FunctionCall(Obj:TBESENObject;const ThisArgument:TBESENValue;const Arguments:array of TBESENValue;var AResult:TBESENValue);
var pArguments:TBESENValuePointers;
    i:integer;
begin
 pArguments:=nil;
 SetLength(pArguments,length(Arguments));
 try
  for i:=0 to length(Arguments)-1 do begin
   pArguments[i]:=@Arguments[i];
  end;
  ObjectCallConstruct(Obj,ThisArgument,@pArguments[0],length(pArguments),false,AResult);
 finally
  SetLength(pArguments,0);
 end;
end;

procedure TBESEN.FunctionConstruct(Obj:TBESENObject;const ThisArgument:TBESENValue;const Arguments:array of TBESENValue;var AResult:TBESENValue);
var pArguments:TBESENValuePointers;
    i:integer;
begin
 pArguments:=nil;
 SetLength(pArguments,length(Arguments));
 try
  for i:=0 to length(Arguments)-1 do begin
   pArguments[i]:=@Arguments[i];
  end;
  ObjectCallConstruct(Obj,ThisArgument,@pArguments[0],length(pArguments),true,AResult);
 finally
  SetLength(pArguments,0);
 end;
end;

function TBESEN.MakeFunction(Node:TBESENASTNodeFunctionLiteral;Name:TBESENString;ParentLexicalEnvironment:TBESENLexicalEnvironment=nil):TBESENObjectDeclaredFunction;
var i:integer;
    Prototype:TBESENObject;
    Body:TBESENASTNodeFunctionBody;
begin
 if not assigned(ParentLexicalEnvironment) then begin
  ParentLexicalEnvironment:=GlobalLexicalEnvironment;
 end;
 result:=TBESENObjectDeclaredFunction.Create(self,ObjectFunctionPrototype,false);
 GarbageCollector.Add(result);
 result.GarbageCollectorLock;
 try
  result.SecurityDomain:=SecurityDomain;
  result.Node:=Node;
  result.Container:=Node.Container;
  result.ObjectName:=Name;
  result.LexicalEnvironment:=ParentLexicalEnvironment;
  result.Extensible:=true;
  
  Body:=Node.Body;
  SetLength(result.Parameters,length(Body.Parameters));
  for i:=0 to length(result.Parameters)-1 do begin
   if assigned(Body.Parameters[i]) then begin
    result.Parameters[i]:=Body.Parameters[i].Name;
   end else begin
    result.Parameters[i]:='param#'+inttostr(i);
   end;
  end;

  if (Compatibility and COMPAT_JS)<>0 then begin
   result.OverwriteData('name',BESENStringValue(Name),[]);
  end;

  result.OverwriteData('length',BESENNumberValue(length(result.Parameters)),[]);

  Prototype:=TBESENObject.Create(self,ObjectPrototype,false);
  GarbageCollector.Add(Prototype);

  Prototype.OverwriteData('constructor',BESENObjectValue(result),[bopaWRITABLE,bopaCONFIGURABLE]);

  result.OverwriteData('prototype',BESENObjectValue(Prototype),[bopaWRITABLE]);

  if IsStrict then begin
   result.OverwriteAccessor('caller',ObjectThrowTypeErrorFunction,ObjectThrowTypeErrorFunction,[],false);
   result.OverwriteAccessor('arguments',ObjectThrowTypeErrorFunction,ObjectThrowTypeErrorFunction,[],false);
  end else if (Compatibility and COMPAT_JS)<>0 then begin
   result.OverwriteData('arguments',BESENNullValue,[]);
  end;
 finally
  result.GarbageCollectorUnlock;
 end;
end;

procedure TBESEN.AddProgramNode(Node:TBESENASTNodeProgram);
var Value:TBESENPointerSelfBalancedTreeValue;
begin
 if assigned(Node) then begin
  if not ProgramNodes.Find(Node,Value) then begin
   Value.p:=Node;
   ProgramNodes.Insert(Node,Value);
  end;
 end;
end;

procedure TBESEN.RemoveProgramNode(Node:TBESENASTNodeProgram);
begin
 if assigned(Node) then begin
  ProgramNodes.Remove(Node);
 end;
end;

procedure TBESEN.GlobalEval(const Context:TBESENContext;const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;const DirectCall:boolean;var AResult:TBESENValue);
var Node:TBESENASTNode;
    v:TBESENValue;
    Lex:TBESENLexicalEnvironment;
    NewContext:TBESENContext;
    CacheItem:TBESENEvalCacheItem;
    Source:TBESENString;
begin
 AResult.ValueType:=bvtUNDEFINED;
 v.ValueType:=bvtuNDEFINED;
 if CountArguments>0 then begin
  BESENCopyValue(v,Arguments^[0]^);
 end;
 if v.ValueType<>bvtSTRING then begin
  BESENCopyValue(AResult,v);
 end else begin
  Source:=ToStr(v);
  try
   if (EvalCache.HashSize>0) and (length(Source)<=EvalCache.MaxSourceLength) then begin
    CacheItem:=EvalCache.Get(Source,IsStrict);
   end else begin
    CacheItem:=nil;
   end;
   if assigned(CacheItem) then begin
    CacheItem.IncRef;
    Node:=TBESENASTNode(CacheItem.Node);
   end else begin
    Node:=Compile(BESENUTF16ToUTF8(Source));
   end;
  finally
  Source:='';
  end;
  NewContext:=TBESENContext.Create(self);
  try
   if assigned(Node) then begin
    try
     if Node is TBESENASTNodeProgram then begin
      AddProgramNode(TBESENASTNodeProgram(Node));
      if DirectCall then begin
       NewContext.LexicalEnvironment:=Context.LexicalEnvironment;
       NewContext.VariableEnvironment:=Context.VariableEnvironment;
       BESENCopyValue(NewContext.ThisBinding,Context.ThisBinding);
      end else begin
       NewContext.LexicalEnvironment:=GlobalLexicalEnvironment;
       NewContext.VariableEnvironment:=GlobalLexicalEnvironment;
       NewContext.ThisBinding.ValueType:=bvtOBJECT;
       NewContext.ThisBinding.Obj:=ObjectGlobal;
      end;
      if IsStrict or TBESENASTNodeProgram(Node).Body.IsStrict then begin
       Lex:=NewDeclarativeEnvironment(NewContext.LexicalEnvironment,true,TBESENCode(TBESENASTNodeProgram(Node).Body.Code).HasMaybeDirectEval);
       GarbageCollector.Add(Lex);
       NewContext.LexicalEnvironment:=Lex;
       NewContext.VariableEnvironment:=Lex;
      end;
      NewContext.InitializeDeclarationBindingInstantiation(TBESENASTNodeProgram(Node).Body,nil,true,nil,0,false);
      Node.ExecuteCode(NewContext,AResult);
     end;
    finally
     if not assigned(CacheItem) then begin
      BesenFreeAndNil(Node);
     end;
    end;
   end;
  finally
   if assigned(CacheItem) then begin
    CacheItem.DecRef;
   end;
   NewContext.Destroy;
  end;
 end;
 if (AResult.ValueType=bvtOBJECT) and assigned(AResult.Obj) then begin
  TBESENObject(AResult.Obj).GarbageCollectorLock;
  try
   GarbageCollector.CollectAll;
  finally
   TBESENObject(AResult.Obj).GarbageCollectorUnlock;
  end;
 end else begin
  GarbageCollector.CollectAll;
 end;
end;

function TBESEN.Execute(Source:TBESENUTF8STRING;const ThisArgument:TBESENValue;const PrecompiledASTNode:TBESENASTNode=nil;const IsEval:TBESENBoolean=false):TBESENValue;
var Node:TBESENASTNode;
    Lex:TBESENLexicalEnvironment;
    NewContext:TBESENContext;
begin
 result:=BESENEmptyValue;
 if assigned(PrecompiledASTNode) then begin
  Node:=PrecompiledASTNode;
 end else begin
  Node:=Compile(Source);
 end;
 NewContext:=TBESENContext.Create(self);
 try
  if assigned(Node) then begin
   try
    if Node is TBESENASTNodeProgram then begin
     AddProgramNode(TBESENASTNodeProgram(Node));
     if (ThisArgument.ValueType=bvtOBJECT) and assigned(TBESENObject(ThisArgument.Obj)) and (TBESENObject(ThisArgument.Obj)<>ObjectGlobal) then begin
      NewContext.LexicalEnvironment:=NewObjectEnvironment(TBESENObject(ThisArgument.Obj),GlobalLexicalEnvironment,TBESENASTNodeProgram(Node).Body.IsStrict,TBESENCode(TBESENASTNodeProgram(Node).Body.Code).HasMaybeDirectEval);
      NewContext.VariableEnvironment:=NewDeclarativeEnvironment(GlobalLexicalEnvironment,TBESENASTNodeProgram(Node).Body.IsStrict,TBESENCode(TBESENASTNodeProgram(Node).Body.Code).HasMaybeDirectEval);
      BESENCopyValue(NewContext.ThisBinding,ThisArgument);
      GarbageCollector.Add(NewContext.LexicalEnvironment);
      GarbageCollector.Add(NewContext.VariableEnvironment);
     end else begin
      NewContext.LexicalEnvironment:=GlobalLexicalEnvironment;
      NewContext.VariableEnvironment:=GlobalLexicalEnvironment;
      NewContext.ThisBinding.ValueType:=bvtOBJECT;
      NewContext.ThisBinding.Obj:=ObjectGlobal;
     end;
     if IsEval and (IsStrict or TBESENASTNodeProgram(Node).Body.IsStrict) then begin
      Lex:=NewDeclarativeEnvironment(NewContext.LexicalEnvironment,true,(assigned(Node) and ((Node is TBESENASTNodeProgram) and assigned(TBESENASTNodeProgram(Node).Body) and TBESENCode(TBESENASTNodeProgram(Node).Body.Code).HasMaybeDirectEval)));
      GarbageCollector.Add(Lex);
      NewContext.LexicalEnvironment:=Lex;
      NewContext.VariableEnvironment:=Lex;
     end;
     NewContext.InitializeDeclarationBindingInstantiation(TBESENASTNodeProgram(Node).Body,nil,IsEval,nil,0,false);
     Node.ExecuteCode(NewContext,result);
    end;
   finally
    if not assigned(PrecompiledASTNode) then begin
     BesenFreeAndNil(Node);
    end;
   end;
  end;
 finally
  NewContext.Destroy;
 end;
 if (result.ValueType=bvtOBJECT) and assigned(result.Obj) then begin
  TBESENObject(result.Obj).GarbageCollectorLock;
  try
   GarbageCollector.CollectAll;
  finally
   TBESENObject(result.Obj).GarbageCollectorUnlock;
  end;
 end else begin
  GarbageCollector.CollectAll;
 end;
end;

function TBESEN.Execute(Source:TBESENUTF8STRING;const PrecompiledASTNode:TBESENASTNode=nil;const IsEval:TBESENBoolean=false):TBESENValue;
begin
 BesenCopyValue(result,Execute(Source,BESENUndefinedValue,PrecompiledASTNode,IsEval));
end;

function TBESEN.Eval(Source:TBESENUTF8STRING;const ThisArgument:TBESENValue;const PrecompiledASTNode:TBESENASTNode=nil):TBESENValue;
begin
 BesenCopyValue(result,Execute(Source,ThisArgument,PrecompiledASTNode,true));
end;

function TBESEN.Eval(Source:TBESENUTF8STRING;const PrecompiledASTNode:TBESENASTNode=nil):TBESENValue;
begin
 BesenCopyValue(result,Execute(Source,BESENUndefinedValue,PrecompiledASTNode,true));
end;

function TBESEN.JSONEval(Source:TBESENUTF8STRING;const PrecompiledASTNode:TBESENASTNode=nil):TBESENValue;
var Node:TBESENASTNode;
    Lex:TBESENLexicalEnvironment;
    NewContext:TBESENContext;
begin
 result:=BESENEmptyValue;
 if assigned(PrecompiledASTNode) then begin
  Node:=PrecompiledASTNode;
 end else begin
  Node:=Compile(Source,'',false,true);
 end;
 NewContext:=TBESENContext.Create(self);
 try
  if assigned(Node) then begin
   try
    if Node is TBESENASTNodeProgram then begin
     AddProgramNode(TBESENASTNodeProgram(Node));
     NewContext.LexicalEnvironment:=GlobalLexicalEnvironment;
     NewContext.VariableEnvironment:=GlobalLexicalEnvironment;
     NewContext.ThisBinding.ValueType:=bvtNULL;
     Lex:=NewDeclarativeEnvironment(NewContext.LexicalEnvironment,TBESENASTNodeProgram(Node).Body.IsStrict,TBESENCode(TBESENASTNodeProgram(Node).Body.Code).HasMaybeDirectEval);
     GarbageCollector.Add(Lex);
     NewContext.LexicalEnvironment:=Lex;
     NewContext.VariableEnvironment:=Lex;
     NewContext.InitializeDeclarationBindingInstantiation(TBESENASTNodeProgram(Node).Body,nil,true,nil,0,false);
     Node.ExecuteCode(NewContext,result);
    end;
   finally
    if not assigned(PrecompiledASTNode) then begin
     BesenFreeAndNil(Node);
    end;
   end;
  end;
 finally
  NewContext.Destroy;
 end;
 if (result.ValueType=bvtOBJECT) and assigned(result.Obj) then begin
  TBESENObject(result.Obj).GarbageCollectorLock;
  try
   GarbageCollector.CollectAll;
  finally
   TBESENObject(result.Obj).GarbageCollectorUnlock;
  end;
 end else begin
  GarbageCollector.CollectAll;
 end;
end;

function TBESEN.NewDeclarativeEnvironment(const Environment:TBESENLexicalEnvironment;const IsItStrict,HasMaybeDirectEval:TBESENBoolean):TBESENLexicalEnvironment;
begin
 result:=TBESENLexicalEnvironment.Create(self);
 result.EnvironmentRecord:=TBESENDeclarativeEnvironmentRecord.Create(self);
 result.EnvironmentRecord.IsStrict:=IsItStrict;
 result.EnvironmentRecord.HasMaybeDirectEval:=HasMaybeDirectEval;
 result.EnvironmentRecord.UpdateImplicitThisValue;
 result.Outer:=Environment;
end;

function TBESEN.NewObjectEnvironment(const BindingObject:TBESENObject;const Environment:TBESENLexicalEnvironment;const IsItStrict,HasMaybeDirectEval:TBESENBoolean):TBESENLexicalEnvironment;
begin
 result:=TBESENLexicalEnvironment.Create(self);
 result.EnvironmentRecord:=TBESENObjectEnvironmentRecord.Create(self);
 result.EnvironmentRecord.IsStrict:=IsItStrict;
 result.EnvironmentRecord.HasMaybeDirectEval:=HasMaybeDirectEval;
 TBESENObjectEnvironmentRecord(result.EnvironmentRecord).BindingObject:=BindingObject;
 result.EnvironmentRecord.UpdateImplicitThisValue;
 result.Outer:=Environment;
end;

function TBESEN.ParseNumber(const s:TBESENString):TBESENNumber;
begin
 result:=BESENStringToNumber(s,true,true,true,false);
end;

procedure TBESEN.ToPrimitiveValue(const AValue,AType:TBESENValue;var AResult:TBESENValue);
 procedure BESENThrowIt;
 begin
  BESENThrowTypeError('Bad object');
 end;
begin
 if AValue.ValueType=bvtOBJECT then begin
  if assigned(AValue.Obj) then begin
   TBESENObject(AValue.Obj).DefaultValue(AType,AResult);
  end else begin
   BESENThrowIt;
  end;
 end else if @AResult<>@AValue then begin
  BESENCopyValue(AResult,AValue);
 end;
end;

procedure TBESEN.ToPrimitiveValue(const AValue:TBESENValue;var AResult:TBESENValue);
begin
 ToPrimitiveValue(AValue,BESENUndefinedValue,AResult);
end;

procedure TBESEN.ToBooleanValue(const AValue:TBESENValue;var AResult:TBESENValue);
 procedure BESENThrowIt;
 begin
  BESENThrowTypeError('Bad to boolean conversation');
 end;
var bo:TBESENObject;
    vo:TBESENValue;
begin
 case AValue.ValueType of
  bvtUNDEFINED:begin
   AResult.ValueType:=bvtBOOLEAN;
   AResult.Bool:=false;
  end;
  bvtNULL:begin
   AResult.ValueType:=bvtBOOLEAN;
   AResult.Bool:=false;
  end;
  bvtBOOLEAN:begin
   AResult.ValueType:=bvtBOOLEAN;
   AResult.Bool:=AValue.Bool;
  end;
  bvtNUMBER:begin
   AResult.ValueType:=bvtBOOLEAN;
   AResult.Bool:=BESENIsInfinite(AValue.Num) or not (BESENIsNaN(AValue.Num) or (abs(AValue.Num)=0));
  end;
  bvtSTRING:begin
   AResult.ValueType:=bvtBOOLEAN;
   AResult.Bool:=length(AValue.Str)>0;
  end;
  bvtOBJECT:begin
   AResult.ValueType:=bvtBOOLEAN;
   AResult.Bool:=true;
   if (Compatibility and COMPAT_JS)<>0 then begin
    bo:=TBESENObject(AValue.Obj);
    if bo is TBESENObjectBoolean then begin
     vo:=BESENEmptyValue;
     bo.Get('valueOf',vo);
     if (vo.ValueType=bvtOBJECT) and assigned(TBESENObject(vo.Obj)) and (TBESENObject(vo.Obj).HasCall) then begin
      ObjectCall(TBESENObject(vo.Obj),BESENObjectValue(bo),nil,0,AResult);
     end;
    end;
   end;
  end;
  else begin
   BESENThrowIt;
  end;
 end;
{$ifdef UseAssert}
 Assert(AResult.ValueType=bvtBOOLEAN);
{$endif}
end;

procedure TBESEN.ToNumberValue(const AValue:TBESENValue;var AResult:TBESENValue);
 procedure BESENThrowIt;
 begin
  BESENThrowTypeError('Bad to number conversation');
 end;
const BooleanToNumber:array[boolean] of TBESENNumber=(0.0,1.0);
var v:TBESENValue;
begin
 case AValue.ValueType of
  bvtUNDEFINED:begin
   AResult.ValueType:=bvtNUMBER;
   int64(pointer(@AResult.Num)^):=int64(pointer(@BESENDoubleNAN)^);
  end;
  bvtNULL:begin
   AResult.ValueType:=bvtNUMBER;
   AResult.Num:=0;
  end;
  bvtBOOLEAN:begin
   AResult.ValueType:=bvtNUMBER;
   AResult.Num:=BooleanToNumber[boolean(AValue.Bool)];
  end;
  bvtNUMBER:begin
   AResult.ValueType:=bvtNUMBER;
   int64(pointer(@AResult.Num)^):=int64(pointer(@AValue.Num)^);
  end;
  bvtSTRING:begin
   AResult.ValueType:=bvtNUMBER;
   AResult.Num:=ParseNumber(AValue.Str);
  end;
  bvtOBJECT:begin
   ToPrimitiveValue(AValue,ObjectNumberConstructorValue,v);
   ToNumberValue(v,AResult);
  end;
  else begin
   BESENThrowIt;
  end;
 end;
{$ifdef UseAssert}
 Assert(AResult.ValueType=bvtNUMBER);
{$endif}
end;

procedure TBESEN.ToIntegerValue(const AValue:TBESENValue;var AResult:TBESENValue);
var Sign:longword;
begin
 ToNumberValue(AValue,AResult);
 if BESENIsNaN(AResult.Num) then begin
  AResult.Num:=0.0;
 end else if not (BESENIsInfinite(AResult.Num) or BESENIsZero(AResult.Num)) then begin
  Sign:=PBESENDoubleHiLo(@AResult.Num)^.Hi and $80000000;
  PBESENDoubleHiLo(@AResult.Num)^.Hi:=PBESENDoubleHiLo(@AResult.Num)^.Hi and $7fffffff;
  AResult.Num:=BESENFloor(AResult.Num);
  PBESENDoubleHiLo(@AResult.Num)^.Hi:=PBESENDoubleHiLo(@AResult.Num)^.Hi or Sign;
 end;
end;

procedure TBESEN.ToStringValue(const AValue:TBESENValue;var AResult:TBESENValue);
 procedure BESENThrowIt;
 begin
  BESENThrowTypeError('Bad to string conversation');
 end;
var v:TBESENValue;
begin
 case AValue.ValueType of
  bvtUNDEFINED:begin
   AResult.ValueType:=bvtSTRING;
   AResult.Str:='undefined';
  end;
  bvtNULL:begin
   AResult.ValueType:=bvtSTRING;
   AResult.Str:='null';
  end;
  bvtBOOLEAN:begin
   AResult.ValueType:=bvtSTRING;
   if AValue.Bool then begin
    AResult.Str:='true';
   end else begin
    AResult.Str:='false';
   end;
  end;
  bvtNUMBER:begin
   AResult.ValueType:=bvtSTRING;
   AResult.Str:=BESENFloatToStr(AValue.Num);
  end;
  bvtSTRING:begin
   AResult.ValueType:=bvtSTRING;
   AResult.Str:=AValue.Str;
  end;
  bvtOBJECT:begin
   ToPrimitiveValue(AValue,ObjectStringConstructorValue,v);
   ToStringValue(v,AResult);
  end;
  else begin
   BESENThrowIt;
  end;
 end;
{$ifdef UseAssert}
 Assert(AResult.ValueType=bvtSTRING);
{$endif}
end;

procedure TBESEN.ToObjectValue(const AValue:TBESENValue;var AResult:TBESENValue);
 procedure BESENThrowIt;
 begin
  BESENThrowTypeError('Bad to object conversation');
 end;
begin
 case AValue.ValueType of
  bvtUNDEFINED:begin
   raise EBESENTypeError.Create('ToObjectValue undefined');
  end;
  bvtNULL:begin
   raise EBESENTypeError.Create('ToObjectValue null');
  end;
  bvtBOOLEAN:begin
   AResult.ValueType:=bvtOBJECT;
   AResult.Obj:=TBESENObjectBoolean.Create(self,ObjectBooleanPrototype,false);
   GarbageCollector.Add(TBESENObject(AResult.Obj));
   TBESENObjectBoolean(AResult.Obj).Value:=AValue.Bool;
  end;
  bvtNUMBER:begin
   AResult.ValueType:=bvtOBJECT;
   AResult.Obj:=TBESENObjectNumber.Create(self,ObjectNumberPrototype,false);
   GarbageCollector.Add(TBESENObject(AResult.Obj));
   TBESENObjectNumber(AResult.Obj).Value:=AValue.Num;
  end;
  bvtSTRING:begin
   AResult.ValueType:=bvtOBJECT;
   AResult.Obj:=TBESENObjectString.Create(self,ObjectStringPrototype,false);
   GarbageCollector.Add(TBESENObject(AResult.Obj));
   TBESENObjectString(AResult.Obj).Value:=AValue.Str;
   TBESENObjectString(AResult.Obj).UpdateLength;
  end;
  bvtOBJECT:begin
   AResult.ValueType:=bvtOBJECT;
   AResult.Obj:=AValue.Obj;
  end;
  else begin
   BESENThrowIt;
  end;
 end;
{$ifdef UseAssert}
 Assert(AResult.ValueType=bvtOBJECT);
{$endif}
end;

function TBESEN.ToInt(const AValue:TBESENValue):int64;
var v:TBESENValue;
begin
 ToIntegerValue(AValue,v);
 result:=trunc(v.Num);
end;

function TBESEN.ToInt32(const AValue:TBESENValue):TBESENINT32;
var v:TBESENValue;
    Sign:longword;
begin
 ToNumberValue(AValue,v);
 if BESENIsNaN(v.Num) or BESENIsInfinite(v.Num) or BESENIsZero(v.Num) then begin
  v.Num:=0.0;
 end else begin
  Sign:=PBESENDoubleHiLo(@v.Num)^.Hi and $80000000;
  PBESENDoubleHiLo(@v.Num)^.Hi:=PBESENDoubleHiLo(@v.Num)^.Hi and $7fffffff;
  v.Num:=BESENFloor(v.Num);
  PBESENDoubleHiLo(@v.Num)^.Hi:=PBESENDoubleHiLo(@v.Num)^.Hi or Sign;
  v.Num:=BESENModulo(System.int(v.Num),4294967296.0);
  if (PBESENDoubleHiLo(@v.Num)^.Hi and $80000000)<>0 then begin
   v.Num:=v.Num+4294967296.0;
  end;
  if v.Num>=2147483648.0 then begin
   v.Num:=v.Num-4294967296.0;
  end;
 end;
 result:=trunc(v.Num);
end;

function TBESEN.ToUInt32(const AValue:TBESENValue):TBESENUINT32;
var v:TBESENValue;
    Sign:longword;
begin
 ToNumberValue(AValue,v);
 if BESENIsNaN(v.Num) or BESENIsInfinite(v.Num) or BESENIsZero(v.Num) then begin
  v.Num:=0.0;
 end else begin
  Sign:=PBESENDoubleHiLo(@v.Num)^.Hi and $80000000;
  PBESENDoubleHiLo(@v.Num)^.Hi:=PBESENDoubleHiLo(@v.Num)^.Hi and $7fffffff;
  v.Num:=BESENFloor(v.Num);
  PBESENDoubleHiLo(@v.Num)^.Hi:=PBESENDoubleHiLo(@v.Num)^.Hi or Sign;
  v.Num:=BESENModulo(System.int(v.Num),4294967296.0);
  if (PBESENDoubleHiLo(@v.Num)^.Hi and $80000000)<>0 then begin
   v.Num:=v.Num+4294967296.0;
  end;
 end;
 result:=trunc(v.Num);
end;

function TBESEN.ToInt16(const AValue:TBESENValue):TBESENINT16;
var v:TBESENValue;
    Sign:longword;
begin
 ToNumberValue(AValue,v);
 if BESENIsNaN(v.Num) or BESENIsInfinite(v.Num) or BESENIsZero(v.Num) then begin
  v.Num:=0.0;
 end else begin
  Sign:=PBESENDoubleHiLo(@v.Num)^.Hi and $80000000;
  PBESENDoubleHiLo(@v.Num)^.Hi:=PBESENDoubleHiLo(@v.Num)^.Hi and $7fffffff;
  v.Num:=BESENFloor(v.Num);
  PBESENDoubleHiLo(@v.Num)^.Hi:=PBESENDoubleHiLo(@v.Num)^.Hi or Sign;
  v.Num:=BESENModulo(System.int(v.Num),65536.0);
  if (PBESENDoubleHiLo(@v.Num)^.Hi and $80000000)<>0 then begin
   v.Num:=v.Num+65536.0;
  end;
  if v.Num>=32768.0 then begin
   v.Num:=v.Num-65536.0;
  end;
 end;
 result:=trunc(v.Num);
end;

function TBESEN.ToUInt16(const AValue:TBESENValue):TBESENUINT16;
var v:TBESENValue;
    Sign:longword;
begin
 ToNumberValue(AValue,v);
 if BESENIsNaN(v.Num) or BESENIsInfinite(v.Num) or BESENIsZero(v.Num) then begin
  v.Num:=0.0;
 end else begin
  Sign:=PBESENDoubleHiLo(@v.Num)^.Hi and $80000000;
  PBESENDoubleHiLo(@v.Num)^.Hi:=PBESENDoubleHiLo(@v.Num)^.Hi and $7fffffff;
  v.Num:=BESENFloor(v.Num);
  PBESENDoubleHiLo(@v.Num)^.Hi:=PBESENDoubleHiLo(@v.Num)^.Hi or Sign;
  v.Num:=BESENModulo(System.int(v.Num),65536.0);
  if (PBESENDoubleHiLo(@v.Num)^.Hi and $80000000)<>0 then begin
   v.Num:=v.Num+65536.0;
  end;
 end;
 result:=trunc(v.Num);
end;

function TBESEN.ToBool(const AValue:TBESENValue):TBESENBoolean;
var b:TBESENValue;
begin
 ToBooleanValue(AValue,b);
 result:=b.Bool;
end;

function TBESEN.ToNum(const AValue:TBESENValue):TBESENNumber;
var n:TBESENValue;
begin
 ToNumberValue(AValue,n);
 result:=n.Num;
end;

function TBESEN.ToStr(const AValue:TBESENValue):TBESENString;
var s:TBESENValue;
begin
 ToStringValue(AValue,s);
 result:=s.Str;
end;

function TBESEN.ToObj(const AValue:TBESENValue):TBESENObject;
var o:TBESENValue;
begin
 ToObjectValue(AValue,o);
 result:=TBESENObject(o.Obj);
end;

procedure TBESEN.EqualityExpressionSub(const a,b:TBESENValue;var AResult:TBESENValue);
var r1,r2:TBESENValue;
    n1,n2:TBESENNumber;
begin
 ToPrimitiveValue(a,ObjectNumberConstructorValue,r1);
 ToPrimitiveValue(b,ObjectNumberConstructorValue,r2);
 if (r1.ValueType=bvtSTRING) and (r2.ValueType=bvtSTRING) then begin
  AResult.ValueType:=bvtBOOLEAN;
  AResult.Bool:=r1.Str<r2.Str;
 end else begin
  n1:=ToNum(r1);
  n2:=ToNum(r2);
  if BESENIsNaN(n1) or BESENIsNaN(n2) then begin
   AResult.ValueType:=bvtUNDEFINED;
  end else if n1=n2 then begin
   AResult.ValueType:=bvtBOOLEAN;
   AResult.Bool:=false;
  end else if BESENIsPosInfinite(n1) then begin
   AResult.ValueType:=bvtBOOLEAN;
   AResult.Bool:=false;
  end else if BESENIsPosInfinite(n2) then begin
   AResult.ValueType:=bvtBOOLEAN;
   AResult.Bool:=true;
  end else if BESENIsNegInfinite(n1) then begin
   AResult.ValueType:=bvtBOOLEAN;
   AResult.Bool:=true;
  end else if BESENIsNegInfinite(n2) then begin
   AResult.ValueType:=bvtBOOLEAN;
   AResult.Bool:=false;
  end else begin
   AResult.ValueType:=bvtBOOLEAN;
   AResult.Bool:=n1<n2;
  end;
 end;
end;

function TBESEN.EqualityExpressionEquals(const a,b:TBESENValue):boolean;
 function BooleanAgainstAllAnother:boolean;
 var TempValue:TBESENValue;
 begin
  ToNumberValue(a,TempValue);
  result:=EqualityExpressionEquals(TempValue,b);
 end;
 function AllAnotherAgainstBoolean:boolean;
 var TempValue:TBESENValue;
 begin
  ToNumberValue(b,TempValue);
  result:=EqualityExpressionEquals(a,TempValue);
 end;
 function NumberAgainstString:boolean;
 var TempValue:TBESENValue;
 begin
  ToNumberValue(b,TempValue);
  result:=EqualityExpressionEquals(a,TempValue);
 end;
 function StringAgainstNumber:boolean;
 var TempValue:TBESENValue;
 begin
  ToNumberValue(a,TempValue);
  result:=EqualityExpressionEquals(TempValue,b);
 end;
 function StringNumberAgainstObject:boolean;
 var TempValue:TBESENValue;
 begin
  ToPrimitiveValue(b,BESENUndefinedValue,TempValue);
  result:=EqualityExpressionEquals(a,TempValue);
 end;
 function ObjectAgainstStringNumber:boolean;
 var TempValue:TBESENValue;
 begin
  ToPrimitiveValue(a,BESENUndefinedValue,TempValue);
  result:=EqualityExpressionEquals(TempValue,b);
 end;
begin
 if a.ValueType=b.ValueType then begin
  case a.ValueType of
   bvtUNDEFINED:begin
    result:=true;
   end;
   bvtNULL:begin
    result:=true;
   end;
   bvtNUMBER:begin
{$ifdef UseSafeOperations}
    if BESENIsNaN(a.Num) then begin
     result:=false;
    end else if BESENIsNaN(b.Num) then begin
     result:=false;
    end else begin
     result:=(a.Num=b.Num) or (BESENIsZero(a.Num) and BESENIsZero(b.Num));
    end;
{$else}
    result:=(not (BESENIsNaN(a.Num) or BESENIsNaN(b.Num))) and (a.Num=b.Num);
{$endif}
   end;
   bvtSTRING:begin
    result:=a.Str=b.Str;
   end;
   bvtBOOLEAN:begin
    result:=a.Bool=b.Bool;
   end;
   bvtOBJECT:begin
    result:=a.Obj=b.Obj;
   end;
   else begin
    result:=false;
   end;
  end;
 end else begin
  if (a.ValueType=bvtNULL) and (b.ValueType=bvtUNDEFINED) then begin
   result:=true;
  end else if (a.ValueType=bvtUNDEFINED) and (b.ValueType=bvtNULL) then begin
   result:=true;
  end else if (a.ValueType=bvtNUMBER) and (b.ValueType=bvtSTRING) then begin
   result:=NumberAgainstString;
  end else if (a.ValueType=bvtSTRING) and (b.ValueType=bvtNUMBER) then begin
   result:=StringAgainstNumber;
  end else if a.ValueType=bvtBOOLEAN then begin
   result:=BooleanAgainstAllAnother;
  end else if b.ValueType=bvtBOOLEAN then begin
   result:=AllAnotherAgainstBoolean;
  end else if ((a.ValueType=bvtSTRING) or (a.ValueType=bvtNUMBER)) and (b.ValueType=bvtOBJECT) then begin
   result:=StringNumberAgainstObject;
  end else if (a.ValueType=bvtOBJECT) and ((b.ValueType=bvtSTRING) or (b.ValueType=bvtNUMBER)) then begin
   result:=ObjectAgainstStringNumber;
  end else begin
   result:=false;
  end;
 end;
end;

function TBESEN.EqualityExpressionCompare(const a,b:TBESENValue):integer;
 function DoItSafe:integer;
 var v:TBESENValue;
 begin
  try
   EqualityExpressionSub(a,b,v);
   if (v.ValueType=bvtBOOLEAN) and v.Bool then begin
    result:=-1;
   end else begin
    result:=1;
   end;
  except
   result:=0;
  end;
 end;
 function DoItForNumbers:integer;
 var n1,n2:TBESENNumber;
 begin
  n1:=a.Num;
  n2:=b.Num;
  if BESENIsNaN(n1) or BESENIsNaN(n2) then begin
   result:=1;
  end else if BESENIsSameValue(n1,n2) then begin
   result:=1;
  end else if (BESENIsZero(n1) and BESENIsZero(n2)) and (BESENIsNegative(n1)<>BESENIsNegative(n2)) then begin
   result:=1;
  end else if BESENIsPosInfinite(n1) then begin
   result:=1;
  end else if BESENIsPosInfinite(n2) then begin
   result:=-1;
  end else if BESENIsNegInfinite(n1) then begin
   result:=-1;
  end else if BESENIsNegInfinite(n2) then begin
   result:=1;
  end else begin
   if n1<n2 then begin
    result:=-1;
   end else begin
    result:=1;
   end;
  end;
 end;
begin
 if EqualityExpressionEquals(a,b) then begin
  result:=0;
 end else begin
  if (a.ValueType=bvtNUMBER) and (b.ValueType=bvtNUMBER) then begin
   result:=DoItForNumbers;
  end else begin
   result:=DoItSafe;
  end;
 end;
end;

procedure TBESEN.FromPropertyDescriptor(const Descriptor:TBESENObjectPropertyDescriptor;var AResult:TBESENValue);
var bv:TBESENValue;
begin
 if Descriptor.Presents=[] then begin
  AResult:=BESENUndefinedValue;
 end else begin
  AResult:=BESENObjectValue(TBESENObject.Create(self,ObjectPrototype));
  bv.ValueType:=bvtBOOLEAN;
  if ([boppVALUE,boppWRITABLE]*Descriptor.Presents)<>[] then begin
   TBESENObject(AResult.Obj).OverwriteData('value',Descriptor.Value,[bopaWRITABLE,bopaENUMERABLE,bopaCONFIGURABLE],false);
   bv.Bool:=bopaWRITABLE in Descriptor.Attributes;
   TBESENObject(AResult.Obj).OverwriteData('writable',bv,[bopaWRITABLE,bopaENUMERABLE,bopaCONFIGURABLE],false);
  end else if ([boppGETTER,boppSETTER]*Descriptor.Presents)<>[] then begin
   if boppGETTER in Descriptor.Presents then begin
    if assigned(Descriptor.Getter) then begin
     TBESENObject(AResult.Obj).OverwriteData('get',BESENObjectValueEx(Descriptor.Getter),[bopaWRITABLE,bopaENUMERABLE,bopaCONFIGURABLE],false);
    end else begin
     TBESENObject(AResult.Obj).OverwriteData('get',BESENUndefinedValue,[bopaWRITABLE,bopaENUMERABLE,bopaCONFIGURABLE],false);
    end;
   end;
   if boppSETTER in Descriptor.Presents then begin
    if assigned(Descriptor.Setter) then begin
     TBESENObject(AResult.Obj).OverwriteData('set',BESENObjectValueEx(Descriptor.Setter),[bopaWRITABLE,bopaENUMERABLE,bopaCONFIGURABLE],false);
    end else begin
     TBESENObject(AResult.Obj).OverwriteData('set',BESENUndefinedValue,[bopaWRITABLE,bopaENUMERABLE,bopaCONFIGURABLE],false);
    end;
   end;
  end else begin
   BESENFreeAndNil(AResult.Obj);
   raise EBESENInternalError.Create('201003121938-0001');
  end;
  GarbageCollector.Add(TBESENObject(AResult.Obj));
  TBESENObject(AResult.Obj).GarbageCollectorLock;
  try
   bv.Bool:=bopaENUMERABLE in Descriptor.Attributes;
   TBESENObject(AResult.Obj).OverwriteData('enumerable',bv,[bopaWRITABLE,bopaENUMERABLE,bopaCONFIGURABLE],false);
   bv.Bool:=bopaCONFIGURABLE in Descriptor.Attributes;
   TBESENObject(AResult.Obj).OverwriteData('configurable',bv,[bopaWRITABLE,bopaENUMERABLE,bopaCONFIGURABLE],false);
  finally
   TBESENObject(AResult.Obj).GarbageCollectorUnlock;
  end;
 end;
end;

procedure TBESEN.ToPropertyDescriptor(const v:TBESENValue;var AResult:TBESENObjectPropertyDescriptor);
var t:TBESENValue;
begin
 if not ((v.ValueType=bvtOBJECT) and assigned(TBESENObject(v.Obj))) then begin
  raise EBESENTypeError.Create('ToPropertyDescriptor failed');
 end;
 AResult:=BESENUndefinedPropertyDescriptor;
 if TBESENObject(v.Obj).Get('enumerable',t) then begin
  if ToBool(t) then begin
   AResult.Attributes:=AResult.Attributes+[bopaENUMERABLE];
  end;
  AResult.Presents:=AResult.Presents+[boppENUMERABLE];
 end;
 if TBESENObject(v.Obj).Get('configurable',t) then begin
  if ToBool(t) then begin
   AResult.Attributes:=AResult.Attributes+[bopaCONFIGURABLE];
  end;
  AResult.Presents:=AResult.Presents+[boppCONFIGURABLE];
 end;
 if TBESENObject(v.Obj).Get('value',AResult.Value) then begin
  AResult.Presents:=AResult.Presents+[boppVALUE];
 end;
 if TBESENObject(v.Obj).Get('writable',t) then begin
  if ToBool(t) then begin
   AResult.Attributes:=AResult.Attributes+[bopaWRITABLE];
  end;
  AResult.Presents:=AResult.Presents+[boppWRITABLE];
 end;
 if TBESENObject(v.Obj).Get('get',t) then begin
  if BESENIsCallable(t) then begin
   AResult.Getter:=TBESENObject(t.Obj);
  end else if t.ValueType<>bvtUNDEFINED then begin
   raise EBESENTypeError.Create('ToPropertyDescriptor failed');
  end;
  AResult.Presents:=AResult.Presents+[boppGETTER];
 end;
 if TBESENObject(v.Obj).Get('set',t) then begin
  if BESENIsCallable(t) then begin
   AResult.Setter:=TBESENObject(t.Obj);
  end else if t.ValueType<>bvtUNDEFINED then begin
   raise EBESENTypeError.Create('ToPropertyDescriptor failed');
  end;
  AResult.Presents:=AResult.Presents+[boppSETTER];
 end;
 if BESENIsInconsistentDescriptor(AResult) then begin
  raise EBESENTypeError.Create('ToPropertyDescriptor failed');
 end;
end;

function TBESEN.SameValue(const va,vb:TBESENValue):TBESENBoolean;
var vaNAN,vbNAN:boolean;
begin
 if va.ValueType<>vb.ValueType then begin
  result:=false;
 end else begin
  case va.ValueType of
   bvtUNDEFINED:begin
    result:=true;
   end;
   bvtNULL:begin
    result:=true;
   end;
   bvtNUMBER:begin
    vaNAN:=BESENIsNaN(va.Num);
    vbNAN:=BESENIsNaN(vb.Num);
    if vaNAN or vbNAN then begin
     result:=vaNAN and vbNAN;
    end else if (abs(va.Num)=0) and (abs(vb.Num)=0) then begin
     result:=(int64(pointer(@va.Num)^) shr 63)=(int64(pointer(@vb.Num)^) shr 63);
    end else begin
     result:=(int64(pointer(@va.Num)^)=int64(pointer(@vb.Num)^)) or (va.Num=vb.Num);
    end;
   end;
   bvtSTRING:begin
    result:=va.Str=vb.Str;
   end;
   bvtBOOLEAN:begin
    result:=va.Bool=vb.Bool;
   end;
   bvtOBJECT:begin
    result:=va.Obj=vb.Obj;
   end;
   else begin
    result:=false;
   end;
  end;
 end;
end;

function TBESEN.SameValue(const oa,ob:TBESENObject):TBESENBoolean;
begin
 result:=oa=ob;
end;

function TBESEN.SameObject(const oa,ob:TBESENObject):TBESENBoolean;
begin
 result:=oa=ob;
end;

procedure InitBESEN;
const BESENSignature:TBESENANSISTRING='BESEN - A ECMAScript 5th edition engine - Version '+BESENVersion+' - Copyright (C) 2010, Benjamin ''BeRo'' Rosseaux - benjamin@rosseaux.com - http://www.rosseaux.com ';
begin
 if length(BESENSignature)>0 then begin
  BESENLengthHash:=BESENHashKey('length');
 end;
end;

procedure DoneBESEN;
begin
end;

initialization
 InitBESEN;
finalization
 DoneBESEN;
end.
