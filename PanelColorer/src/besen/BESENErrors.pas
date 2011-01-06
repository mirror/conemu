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
unit BESENErrors;
{$i BESEN.inc}

interface

uses SysUtils,Classes,BESENConstants,BESENTypes,BESENValue;

type EBESENError=class(Exception)
      public
       Name:TBESENString;
       Value:TBESENValue;
       constructor Create; overload; virtual;
       constructor Create(const Msg:TBESENANSISTRING); overload; virtual;
       constructor Create(const AValue:TBESENValue); overload; virtual;
       constructor Create(const Msg:TBESENANSISTRING;const AValue:TBESENValue); overload; virtual;
       constructor Create(const AName,Msg:TBESENANSISTRING); overload; virtual;
       constructor Create(const AName,Msg:TBESENANSISTRING;const AValue:TBESENValue); overload; virtual;
       destructor Destroy; override;
     end;

     EBESENUseStrict=class(EBESENError)
     end;

     EBESENInternalError=class(EBESENError)
      public
       constructor Create; overload; override;
       constructor Create(const Msg:TBESENANSISTRING); overload; override;
       constructor Create(const AValue:TBESENValue); overload; override;
       constructor Create(const Msg:TBESENANSISTRING;const AValue:TBESENValue); overload; override;
     end;

     EBESENCompilerError=class(EBESENError)
      public
       constructor Create; overload; override;
       constructor Create(const Msg:TBESENANSISTRING); overload; override;
       constructor Create(const AValue:TBESENValue); overload; override;
       constructor Create(const Msg:TBESENANSISTRING;const AValue:TBESENValue); overload; override;
     end;

     EBESENEvalError=class(EBESENError)
      public
       constructor Create; overload; override;
       constructor Create(const Msg:TBESENANSISTRING); overload; override;
       constructor Create(const AValue:TBESENValue); overload; override;
       constructor Create(const Msg:TBESENANSISTRING;const AValue:TBESENValue); overload; override;
     end;

     EBESENRangeError=class(EBESENError)
      public
       constructor Create; overload; override;
       constructor Create(const Msg:TBESENANSISTRING); overload; override;
       constructor Create(const AValue:TBESENValue); overload; override;
       constructor Create(const Msg:TBESENANSISTRING;const AValue:TBESENValue); overload; override;
     end;

     EBESENReferenceError=class(EBESENError)
      public
       constructor Create; overload; override;
       constructor Create(const Msg:TBESENANSISTRING); overload; override;
       constructor Create(const AValue:TBESENValue); overload; override;
       constructor Create(const Msg:TBESENANSISTRING;const AValue:TBESENValue); overload; override;
     end;

     EBESENSyntaxError=class(EBESENError)
      public
       constructor Create; overload; override;
       constructor Create(const Msg:TBESENANSISTRING); overload; override;
       constructor Create(const AValue:TBESENValue); overload; override;
       constructor Create(const Msg:TBESENANSISTRING;const AValue:TBESENValue); overload; override;
     end;

     EBESENTypeError=class(EBESENError)
      public
       constructor Create; overload; override;
       constructor Create(const Msg:TBESENANSISTRING); overload; override;
       constructor Create(const AValue:TBESENValue); overload; override;
       constructor Create(const Msg:TBESENANSISTRING;const AValue:TBESENValue); overload; override;
     end;

     EBESENURIError=class(EBESENError)
      public
       constructor Create; overload; override;
       constructor Create(const Msg:TBESENANSISTRING); overload; override;
       constructor Create(const AValue:TBESENValue); overload; override;
       constructor Create(const Msg:TBESENANSISTRING;const AValue:TBESENValue); overload; override;
     end;

     EBESENThrowException=class(EBESENError)
      public
       constructor Create; overload; override;
       constructor Create(const Msg:TBESENANSISTRING); overload; override;
       constructor Create(const AValue:TBESENValue); overload; override;
       constructor Create(const Msg:TBESENANSISTRING;const AValue:TBESENValue); overload; override;
     end;

procedure BESENThrowReferenceError(const Msg:TBESENString);
procedure BESENThrowSyntaxError(const Msg:TBESENString);
procedure BESENThrowTypeError(const Msg:TBESENString);
procedure BESENThrowRangeError(const Msg:TBESENString);
procedure BESENThrowInternalError(const Msg:TBESENString);
procedure BESENThrowError(const Msg:TBESENString);
procedure BESENThrowCodeGeneratorInvalidRegister;
procedure BESENThrowRecursionLimitReached;
procedure BESENThrowNotDefined(const ARef:TBESENValue);
procedure BESENThrowReference;
procedure BESENThrowNotAccessable(const ARef:TBESENValue);
procedure BESENThrowNotReadable(const P:TBESENString);
procedure BESENThrowNotWritable(const P:TBESENString);
procedure BESENThrowNoSetter(const P:TBESENString);
procedure BESENThrowRcursivePrototypeChain;
procedure BESENThrowPut(const P:TBESENString);
procedure BESENThrowPutRecursivePrototypeChain;
procedure BESENThrowPutInvalidPrototype;
procedure BESENThrowDefineOwnProperty(const P:TBESENString);
procedure BESENThrowCaller;
procedure BESENThrowTypeErrorDeclarationBindingInstantiationAtFunctionBinding(const fn:TBESENString);
procedure BESENThrowTypeErrorNotAConstructorObject;
procedure BESENThrowTypeErrorObjectHasNoConstruct;
procedure BESENThrowTypeErrorNotAFunction;
procedure BESENThrowTypeErrorNotCallable;

implementation

uses BESEN,BESENStringUtils;

constructor EBESENError.Create;
begin
 inherited Create('');
 Name:='Error';
 Value:=BESENEmptyValue;
end;

constructor EBESENError.Create(const Msg:TBESENANSISTRING);
begin
 inherited Create(Msg);
 Name:='Error';
 Value:=BESENEmptyValue;
end;

constructor EBESENError.Create(const AValue:TBESENValue);
begin
 inherited Create('');
 Name:='Error';
 Value:=AValue;
end;

constructor EBESENError.Create(const Msg:TBESENANSISTRING;const AValue:TBESENValue);
begin
 inherited Create(Msg);
 Name:='Error';
 Value:=AValue;
end;

constructor EBESENError.Create(const AName,Msg:TBESENANSISTRING);
begin
 inherited Create(Msg);
 Name:=AName;
 Value:=BESENEmptyValue;
end;

constructor EBESENError.Create(const AName,Msg:TBESENANSISTRING;const AValue:TBESENValue);
begin
 inherited Create(Msg);
 Name:=AName;
 Value:=AValue;
end;

destructor EBESENError.Destroy;
begin
 Value.Str:='';
 Value.ReferenceBase.Str:='';
 Value:=BESENEmptyValue;
 Name:='';
 inherited Destroy;
end;

constructor EBESENInternalError.Create;
begin
 inherited Create;
 Name:='InternalError';
end;

constructor EBESENInternalError.Create(const Msg:TBESENANSISTRING);
begin
 inherited Create(Msg);
 Name:='InternalError';
end;

constructor EBESENInternalError.Create(const AValue:TBESENValue);
begin
 inherited Create(AValue);
 Name:='InternalError';
end;

constructor EBESENInternalError.Create(const Msg:TBESENANSISTRING;const AValue:TBESENValue);
begin

 inherited Create(Msg,AValue);
 Name:='InternalError';
end;

constructor EBESENCompilerError.Create;
begin
 inherited Create;
 Name:='CompilerError';
end;

constructor EBESENCompilerError.Create(const Msg:TBESENANSISTRING);
begin
 inherited Create(Msg);
 Name:='CompilerError';
end;

constructor EBESENCompilerError.Create(const AValue:TBESENValue);
begin
 inherited Create(AValue);
 Name:='CompilerError';
end;

constructor EBESENCompilerError.Create(const Msg:TBESENANSISTRING;const AValue:TBESENValue);
begin
 inherited Create(Msg,AValue);
 Name:='CompilerError';
end;

constructor EBESENEvalError.Create;
begin
 inherited Create;
 Name:='EvalError';
end;

constructor EBESENEvalError.Create(const Msg:TBESENANSISTRING);
begin
 inherited Create(Msg);
 Name:='EvalError';
end;

constructor EBESENEvalError.Create(const AValue:TBESENValue);
begin
 inherited Create(AValue);
 Name:='EvalError';
end;

constructor EBESENEvalError.Create(const Msg:TBESENANSISTRING;const AValue:TBESENValue);
begin
 inherited Create(Msg,AValue);
 Name:='EvalError';
end;

constructor EBESENRangeError.Create;
begin
 inherited Create;
 Name:='RangeError';
end;

constructor EBESENRangeError.Create(const Msg:TBESENANSISTRING);
begin
 inherited Create(Msg);
 Name:='RangeError';
end;

constructor EBESENRangeError.Create(const AValue:TBESENValue);
begin
 inherited Create(AValue);
 Name:='RangeError';
end;

constructor EBESENRangeError.Create(const Msg:TBESENANSISTRING;const AValue:TBESENValue);
begin
 inherited Create(Msg,AValue);
 Name:='RangeError';
end;

constructor EBESENReferenceError.Create;
begin
 inherited Create;
 Name:='ReferenceError';
end;

constructor EBESENReferenceError.Create(const Msg:TBESENANSISTRING);
begin
 inherited Create(Msg);
 Name:='ReferenceError';
end;

constructor EBESENReferenceError.Create(const AValue:TBESENValue);
begin
 inherited Create(AValue);
 Name:='ReferenceError';
end;

constructor EBESENReferenceError.Create(const Msg:TBESENANSISTRING;const AValue:TBESENValue);
begin
 inherited Create(Msg,AValue);
 Name:='ReferenceError';
end;

constructor EBESENSyntaxError.Create;
begin
 inherited Create;
 Name:='SyntaxError';
end;

constructor EBESENSyntaxError.Create(const Msg:TBESENANSISTRING);
begin
 inherited Create(Msg);
 Name:='SyntaxError';
end;

constructor EBESENSyntaxError.Create(const AValue:TBESENValue);
begin
 inherited Create(AValue);
 Name:='SyntaxError';
end;

constructor EBESENSyntaxError.Create(const Msg:TBESENANSISTRING;const AValue:TBESENValue);
begin
 inherited Create(Msg,AValue);
 Name:='SyntaxError';
end;

constructor EBESENTypeError.Create;
begin
 inherited Create;
 Name:='TypeError';
end;

constructor EBESENTypeError.Create(const Msg:TBESENANSISTRING);
begin
 inherited Create(Msg);
 Name:='TypeError';
end;

constructor EBESENTypeError.Create(const AValue:TBESENValue);
begin
 inherited Create(AValue);
 Name:='TypeError';
end;

constructor EBESENTypeError.Create(const Msg:TBESENANSISTRING;const AValue:TBESENValue);
begin
 inherited Create(Msg,AValue);
 Name:='TypeError';
end;

constructor EBESENURIError.Create;
begin
 inherited Create;
 Name:='URIError';
end;

constructor EBESENURIError.Create(const Msg:TBESENANSISTRING);
begin
 inherited Create(Msg);
 Name:='URIError';
end;

constructor EBESENURIError.Create(const AValue:TBESENValue);
begin
 inherited Create(AValue);
 Name:='URIError';
end;

constructor EBESENURIError.Create(const Msg:TBESENANSISTRING;const AValue:TBESENValue);
begin
 inherited Create(Msg,AValue);
 Name:='URIError';
end;

constructor EBESENThrowException.Create;
begin
 inherited Create;
 Name:='ThrowException';
end;

constructor EBESENThrowException.Create(const Msg:TBESENANSISTRING);
begin
 inherited Create(Msg);
 Name:='ThrowException';
end;

constructor EBESENThrowException.Create(const AValue:TBESENValue);
begin
 inherited Create(AValue);
 Name:='ThrowException';
end;

constructor EBESENThrowException.Create(const Msg:TBESENANSISTRING;const AValue:TBESENValue);
begin
 inherited Create(Msg,AValue);
 Name:='ThrowException';
end;

procedure BESENThrowReferenceError(const Msg:TBESENString);
begin
 raise EBESENReferenceError.Create(Msg);
end;

procedure BESENThrowSyntaxError(const Msg:TBESENString);
begin
 raise EBESENSyntaxError.Create(Msg);
end;

procedure BESENThrowTypeError(const Msg:TBESENString);
begin
 raise EBESENTypeError.Create(Msg);
end;

procedure BESENThrowRangeError(const Msg:TBESENString);
begin
 raise EBESENRangeError.Create(Msg);
end;

procedure BESENThrowInternalError(const Msg:TBESENString);
begin
 raise EBESENInternalError.Create(Msg);
end;

procedure BESENThrowError(const Msg:TBESENString);
begin
 raise EBESENError.Create(Msg);
end;

procedure BESENThrowCodeGeneratorInvalidRegister;
begin
 BESENThrowError('Invalid register in code generation');
end;

procedure BESENThrowRecursionLimitReached;
begin
 BESENThrowError('Recursion limit reached');
end;

procedure BESENThrowNotDefined(const ARef:TBESENValue);
begin
 BESENThrowReferenceError('"'+BESENUTF16ToUTF8(ARef.Str)+'" is not defined');
end;

procedure BESENThrowReference;
begin
 BESENThrowReferenceError('Reference error');
end;

procedure BESENThrowNotAccessable(const ARef:TBESENValue);
begin
 BESENThrowReferenceError('"'+BESENUTF16ToUTF8(ARef.Str)+'" is not accessable');
end;

procedure BESENThrowNotReadable(const P:TBESENString);
begin
 BESENThrowReferenceError('"'+BESENUTF16ToUTF8(P)+'" is not readable');
end;

procedure BESENThrowNotWritable(const P:TBESENString);
begin
 BESENThrowReferenceError('"'+BESENUTF16ToUTF8(P)+'" is not writable');
end;

procedure BESENThrowNoSetter(const P:TBESENString);
begin
  BESENThrowTypeError('"'+BESENUTF16ToUTF8(P)+'" has no setter');
end;

procedure BESENThrowRcursivePrototypeChain;
begin
 BESENThrowTypeError('Recursive prototype chain not allowed');
end;

procedure BESENThrowPut(const P:TBESENString);
begin
 BESENThrowTypeError('Put for "'+BESENUTF16ToUTF8(P)+'" failed');
end;

procedure BESENThrowPutRecursivePrototypeChain;
begin
 BESENThrowTypeError('Put for "__proto__" failed, because the prototype chain would be recursive');
end;

procedure BESENThrowPutInvalidPrototype;
begin
 BESENThrowTypeError('Put for "__proto__" failed, because the prototype would be invalid');
end;

procedure BESENThrowDefineOwnProperty(const P:TBESENString);
begin
 BESENThrowTypeError('DefineOwnProperty for "'+BESENUTF16ToUTF8(P)+'" failed');
end;

procedure BESENThrowCaller;
begin
 BESENThrowTypeError('"caller" not allowed here');
end;

procedure BESENThrowTypeErrorDeclarationBindingInstantiationAtFunctionBinding(const fn:TBESENString);
begin
 BESENThrowTypeError('"'+BESENUTF16ToUTF8(fn)+'" not writable or is a accessor descriptor');
end;

procedure BESENThrowTypeErrorNotAConstructorObject;
begin
 BESENThrowTypeError('Not a constructor object');
end;

procedure BESENThrowTypeErrorObjectHasNoConstruct;
begin
 BESENThrowTypeError('Object has no construct');
end;

procedure BESENThrowTypeErrorNotAFunction;
begin
 BESENThrowTypeError('Not a function');
end;

procedure BESENThrowTypeErrorNotCallable;
begin
 BESENThrowTypeError('Not callable');
end;

end.
