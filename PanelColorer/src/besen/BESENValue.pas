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
unit BESENValue;
{$i BESEN.inc}

interface

uses BESENConstants,BESENTypes,BESENStringUtils,BESENCharSet,Variants;

const brbvtUNDEFINED=0;
      brbvtBOOLEAN=1;
      brbvtNUMBER=2;
      brbvtSTRING=3;
      brbvtOBJECT=4;
      brbvtENVREC=5;

      brbvtFIRST=brbvtUNDEFINED;
      brbvtLAST=brbvtENVREC;

      bvtUNDEFINED=0;
      bvtNULL=1;
      bvtBOOLEAN=2;
      bvtNUMBER=3;
      bvtSTRING=4;
      bvtOBJECT=5;
      bvtREFERENCE=6;
      bvtLOCAL=7;
      bvtENVREC=8;
      bvtNONE=9;

      bvtFIRST=bvtUNDEFINED;
      bvtLAST=bvtNONE;

type TBESENReferenceBaseValueType=ptruint;

     PBESENReferenceBaseValue=^TBESENReferenceBaseValue;
     TBESENReferenceBaseValue=record
      Str:TBESENString;
      case ValueType:TBESENReferenceBaseValueType of
       brbvtUNDEFINED:(
       );
       brbvtBOOLEAN:(
        Bool:TBESENBoolean;
       );
       brbvtNUMBER:(
        Num:TBESENNumber;
       );
       brbvtSTRING:(
       );
       brbvtOBJECT:(
        Obj:TObject;
       );
       brbvtENVREC:(
        EnvRec:TObject;
       );
     end;

     TBESENValueType=ptruint;

     PBESENValue=^TBESENValue;
     TBESENValue=record
      Str:TBESENString;
      ReferenceBase:TBESENReferenceBaseValue;
      case ValueType:TBESENValueType of
       bvtUNDEFINED:(
       );
       bvtNULL:(
       );
       bvtBOOLEAN:(
        Bool:TBESENBoolean;
       );
       bvtNUMBER:(
        Num:TBESENNumber;
       );
       bvtSTRING:(
       );
       bvtOBJECT:(
        Obj:TObject;
       );
       bvtREFERENCE:(
        ReferenceIsStrict:longbool;
        ReferenceHash:TBESENHash;
        ReferenceIndex:TBESENINT32;
        ReferenceID:TBESENINT32;
       );
       bvtLOCAL:(
        LocalIndex:TBESENINT32;
       );
       bvtENVREC:(
        EnvRec:TObject;
       );
       bvtNONE:(
       );
     end;

     TBESENValueTypes=array of TBESENValueType;

     TBESENValueTypesItems=array of TBESENValueTypes;
     
     TBESENValues=array of TBESENValue;

     TBESENValuePointers=array of PBESENValue;

     PPBESENValues=^TPBESENValues;
     TPBESENValues=array[0..($7fffffff div sizeof(PBESENValue))-1] of PBESENValue;

     TBESENPointerToValues=array of PBESENValue;

     TBESENCopyReferenceBaseValueProc=procedure(var Dest:TBESENReferenceBaseValue;const Src:TBESENReferenceBaseValue); {$ifdef UseRegister}register;{$endif}

     TBESENCopyReferenceBaseValueProcs=array[brbvtFIRST..brbvtLAST] of TBESENCopyReferenceBaseValueProc;

     TBESENCopyValueProc=procedure(var Dest:TBESENValue;const Src:TBESENValue); {$ifdef UseRegister}register;{$endif}

     TBESENCopyValueProcs=array[bvtFIRST..bvtLAST] of TBESENCopyValueProc;

     TBESENValueToRefBaseValueProc=procedure(var Dest:TBESENReferenceBaseValue;const Src:TBESENValue); {$ifdef UseRegister}register;{$endif}

     TBESENValueToRefBaseValueProcs=array[bvtFIRST..bvtLAST] of TBESENValueToRefBaseValueProc;

     TBESENRefBaseValueToValueProc=procedure(var Dest:TBESENValue;const Src:TBESENReferenceBaseValue); {$ifdef UseRegister}register;{$endif}

     TBESENRefBaseValueToValueProcs=array[brbvtFIRST..brbvtLAST] of TBESENRefBaseValueToValueProc;

     TBESENRefBaseValueToCallThisArgValueProc=procedure(var Dest:TBESENValue;const Src:TBESENReferenceBaseValue);

     TBESENRefBaseValueToCallThisArgValueProcs=array[brbvtFIRST..brbvtLAST] of TBESENRefBaseValueToCallThisArgValueProc;

procedure BESENCopyReferenceBaseValueUndefined(var Dest:TBESENReferenceBaseValue;const Src:TBESENReferenceBaseValue); {$ifdef UseRegister}register;{$endif}
procedure BESENCopyReferenceBaseValueBoolean(var Dest:TBESENReferenceBaseValue;const Src:TBESENReferenceBaseValue); {$ifdef UseRegister}register;{$endif}
procedure BESENCopyReferenceBaseValueNumber(var Dest:TBESENReferenceBaseValue;const Src:TBESENReferenceBaseValue); {$ifdef UseRegister}register;{$endif}
procedure BESENCopyReferenceBaseValueString(var Dest:TBESENReferenceBaseValue;const Src:TBESENReferenceBaseValue); {$ifdef UseRegister}register;{$endif}
procedure BESENCopyReferenceBaseValueObject(var Dest:TBESENReferenceBaseValue;const Src:TBESENReferenceBaseValue); {$ifdef UseRegister}register;{$endif}
procedure BESENCopyReferenceBaseValueEnvRec(var Dest:TBESENReferenceBaseValue;const Src:TBESENReferenceBaseValue); {$ifdef UseRegister}register;{$endif}
procedure BESENCopyReferenceBaseValueNone(var Dest:TBESENReferenceBaseValue;const Src:TBESENReferenceBaseValue); {$ifdef UseRegister}register;{$endif}

procedure BESENCopyReferenceBaseValue(var Dest:TBESENReferenceBaseValue;const Src:TBESENReferenceBaseValue); {$ifdef UseRegister}register;{$endif}

procedure BESENCopyValueUndefined(var Dest:TBESENValue;const Src:TBESENValue); {$ifdef UseRegister}register;{$endif}
procedure BESENCopyValueNull(var Dest:TBESENValue;const Src:TBESENValue); {$ifdef UseRegister}register;{$endif}
procedure BESENCopyValueBoolean(var Dest:TBESENValue;const Src:TBESENValue); {$ifdef UseRegister}register;{$endif}
procedure BESENCopyValueNumber(var Dest:TBESENValue;const Src:TBESENValue); {$ifdef UseRegister}register;{$endif}
procedure BESENCopyValueString(var Dest:TBESENValue;const Src:TBESENValue); {$ifdef UseRegister}register;{$endif}
procedure BESENCopyValueObject(var Dest:TBESENValue;const Src:TBESENValue); {$ifdef UseRegister}register;{$endif}
procedure BESENCopyValueReference(var Dest:TBESENValue;const Src:TBESENValue); {$ifdef UseRegister}register;{$endif}
procedure BESENCopyValueLocal(var Dest:TBESENValue;const Src:TBESENValue); {$ifdef UseRegister}register;{$endif}
procedure BESENCopyValueEnvRec(var Dest:TBESENValue;const Src:TBESENValue); {$ifdef UseRegister}register;{$endif}
procedure BESENCopyValueNone(var Dest:TBESENValue;const Src:TBESENValue); {$ifdef UseRegister}register;{$endif}

procedure BESENCopyValue(var Dest:TBESENValue;const Src:TBESENValue); {$ifdef UseRegister}register;{$endif}

procedure BESENValueToRefBaseValueUndefined(var Dest:TBESENReferenceBaseValue;const Src:TBESENValue); {$ifdef UseRegister}register;{$endif}
procedure BESENValueToRefBaseValueNull(var Dest:TBESENReferenceBaseValue;const Src:TBESENValue); {$ifdef UseRegister}register;{$endif}
procedure BESENValueToRefBaseValueBoolean(var Dest:TBESENReferenceBaseValue;const Src:TBESENValue); {$ifdef UseRegister}register;{$endif}
procedure BESENValueToRefBaseValueNumber(var Dest:TBESENReferenceBaseValue;const Src:TBESENValue); {$ifdef UseRegister}register;{$endif}
procedure BESENValueToRefBaseValueString(var Dest:TBESENReferenceBaseValue;const Src:TBESENValue); {$ifdef UseRegister}register;{$endif}
procedure BESENValueToRefBaseValueObject(var Dest:TBESENReferenceBaseValue;const Src:TBESENValue); {$ifdef UseRegister}register;{$endif}
procedure BESENValueToRefBaseValueReference(var Dest:TBESENReferenceBaseValue;const Src:TBESENValue); {$ifdef UseRegister}register;{$endif}
procedure BESENValueToRefBaseValueLocal(var Dest:TBESENReferenceBaseValue;const Src:TBESENValue); {$ifdef UseRegister}register;{$endif}
procedure BESENValueToRefBaseValueEnvRec(var Dest:TBESENReferenceBaseValue;const Src:TBESENValue); {$ifdef UseRegister}register;{$endif}
procedure BESENValueToRefBaseValueNone(var Dest:TBESENReferenceBaseValue;const Src:TBESENValue); {$ifdef UseRegister}register;{$endif}

procedure BESENValueToReferenceBaseValue(const Value:TBESENValue;var AResult:TBESENReferenceBaseValue); {$ifdef UseRegister}register;{$endif}

procedure BESENRefBaseValueToValueUndefined(var Dest:TBESENValue;const Src:TBESENReferenceBaseValue); {$ifdef UseRegister}register;{$endif}
procedure BESENRefBaseValueToValueBoolean(var Dest:TBESENValue;const Src:TBESENReferenceBaseValue); {$ifdef UseRegister}register;{$endif}
procedure BESENRefBaseValueToValueNumber(var Dest:TBESENValue;const Src:TBESENReferenceBaseValue); {$ifdef UseRegister}register;{$endif}
procedure BESENRefBaseValueToValueString(var Dest:TBESENValue;const Src:TBESENReferenceBaseValue); {$ifdef UseRegister}register;{$endif}
procedure BESENRefBaseValueToValueObject(var Dest:TBESENValue;const Src:TBESENReferenceBaseValue); {$ifdef UseRegister}register;{$endif}
procedure BESENRefBaseValueToValueEnvRec(var Dest:TBESENValue;const Src:TBESENReferenceBaseValue); {$ifdef UseRegister}register;{$endif}

procedure BESENReferenceBaseValueToValue(const Value:TBESENReferenceBaseValue;var AResult:TBESENValue); {$ifdef UseRegister}register;{$endif}

procedure BESENRefBaseValueToCallThisArgValueUndefined(var Dest:TBESENValue;const Src:TBESENReferenceBaseValue); {$ifdef UseRegister}register;{$endif}
procedure BESENRefBaseValueToCallThisArgValueBoolean(var Dest:TBESENValue;const Src:TBESENReferenceBaseValue); {$ifdef UseRegister}register;{$endif}
procedure BESENRefBaseValueToCallThisArgValueNumber(var Dest:TBESENValue;const Src:TBESENReferenceBaseValue); {$ifdef UseRegister}register;{$endif}
procedure BESENRefBaseValueToCallThisArgValueString(var Dest:TBESENValue;const Src:TBESENReferenceBaseValue); {$ifdef UseRegister}register;{$endif}
procedure BESENRefBaseValueToCallThisArgValueObject(var Dest:TBESENValue;const Src:TBESENReferenceBaseValue); {$ifdef UseRegister}register;{$endif}
procedure BESENRefBaseValueToCallThisArgValueEnvRec(var Dest:TBESENValue;const Src:TBESENReferenceBaseValue); {$ifdef UseRegister}register;{$endif}

const BESENCopyReferenceBaseValueProcs:TBESENCopyReferenceBaseValueProcs=(BESENCopyReferenceBaseValueUndefined,
                                                                          BESENCopyReferenceBaseValueBoolean,
                                                                          BESENCopyReferenceBaseValueNumber,
                                                                          BESENCopyReferenceBaseValueString,
                                                                          BESENCopyReferenceBaseValueObject,
                                                                          BESENCopyReferenceBaseValueEnvRec);

      BESENCopyValueProcs:TBESENCopyValueProcs=(BESENCopyValueUndefined,
                                                BESENCopyValueNull,
                                                BESENCopyValueBoolean,
                                                BESENCopyValueNumber,
                                                BESENCopyValueString,
                                                BESENCopyValueObject,
                                                BESENCopyValueReference,
                                                BESENCopyValueLocal,
                                                BESENCopyValueEnvRec,
                                                BESENCopyValueNone);

      BESENValueToRefBaseValueProcs:TBESENValueToRefBaseValueProcs=(BESENValueToRefBaseValueUndefined,
                                                                    BESENValueToRefBaseValueNull,
                                                                    BESENValueToRefBaseValueBoolean,
                                                                    BESENValueToRefBaseValueNumber,
                                                                    BESENValueToRefBaseValueString,
                                                                    BESENValueToRefBaseValueObject,
                                                                    BESENValueToRefBaseValueReference,
                                                                    BESENValueToRefBaseValueLocal,
                                                                    BESENValueToRefBaseValueEnvRec,
                                                                    BESENValueToRefBaseValueNone);

      BESENRefBaseValueToValueProcs:TBESENRefBaseValueToValueProcs=(BESENRefBaseValueToValueUndefined,
                                                                    BESENRefBaseValueToValueBoolean,
                                                                    BESENRefBaseValueToValueNumber,
                                                                    BESENRefBaseValueToValueString,
                                                                    BESENRefBaseValueToValueObject,
                                                                    BESENRefBaseValueToValueEnvRec);

      BESENRefBaseValueToCallThisArgValueProcs:TBESENRefBaseValueToCallThisArgValueProcs=(BESENRefBaseValueToCallThisArgValueUndefined,
                                                                                          BESENRefBaseValueToCallThisArgValueBoolean,
                                                                                          BESENRefBaseValueToCallThisArgValueNumber,
                                                                                          BESENRefBaseValueToCallThisArgValueString,
                                                                                          BESENRefBaseValueToCallThisArgValueObject,
                                                                                          BESENRefBaseValueToCallThisArgValueEnvRec);

function BESENValueToVariant(const v:TBESENValue):Variant;
procedure BESENVariantToValue(const vt:Variant;var v:TBESENValue);

function BESENBooleanValue(const Bool:TBESENBoolean):TBESENValue;
function BESENNumberValue(const Num:TBESENNumber):TBESENValue;
function BESENStringValue(const Str:TBESENString):TBESENValue;
function BESENStringLocaleCharsetValue(const Str:TBESENString):TBESENValue;
function BESENObjectValue(const Obj:TObject):TBESENValue;
function BESENObjectValueEx(const Obj:TObject):TBESENValue;

function BESENEqualityExpressionStrictEquals(const a,b:TBESENValue):longbool; 

var BESENEmptyValue:TBESENValue;
    BESENNullValue:TBESENValue;
    BESENUndefinedValue:TBESENValue;
    BESENDummyValue:TBESENValue;

implementation

uses BESEN,BESENNumberUtils,BESENEnvironmentRecord;

procedure BESENCopyReferenceBaseValueUndefined(var Dest:TBESENReferenceBaseValue;const Src:TBESENReferenceBaseValue); {$ifdef UseRegister}register;{$endif}
begin
 Dest.ValueType:=brbvtUNDEFINED;
end;

procedure BESENCopyReferenceBaseValueBoolean(var Dest:TBESENReferenceBaseValue;const Src:TBESENReferenceBaseValue); {$ifdef UseRegister}register;{$endif}
begin
 Dest.ValueType:=brbvtBOOLEAN;
 Dest.Bool:=Src.Bool;
end;

procedure BESENCopyReferenceBaseValueNumber(var Dest:TBESENReferenceBaseValue;const Src:TBESENReferenceBaseValue); {$ifdef UseRegister}register;{$endif}
begin
 Dest.ValueType:=brbvtNUMBER;
 Dest.Num:=Src.Num;
end;

procedure BESENCopyReferenceBaseValueString(var Dest:TBESENReferenceBaseValue;const Src:TBESENReferenceBaseValue); {$ifdef UseRegister}register;{$endif}
begin
 Dest.ValueType:=brbvtSTRING;
 Dest.Str:=Src.Str;
end;

procedure BESENCopyReferenceBaseValueObject(var Dest:TBESENReferenceBaseValue;const Src:TBESENReferenceBaseValue); {$ifdef UseRegister}register;{$endif}
begin
 Dest.ValueType:=brbvtOBJECT;
 Dest.Obj:=Src.Obj;
end;

procedure BESENCopyReferenceBaseValueEnvRec(var Dest:TBESENReferenceBaseValue;const Src:TBESENReferenceBaseValue); {$ifdef UseRegister}register;{$endif}
begin
 Dest.ValueType:=brbvtENVREC;
 Dest.EnvRec:=Src.EnvRec;
end;

procedure BESENCopyReferenceBaseValueNone(var Dest:TBESENReferenceBaseValue;const Src:TBESENReferenceBaseValue); {$ifdef UseRegister}register;{$endif}
begin
 Dest.ValueType:=bvtNONE;
end;

procedure BESENCopyReferenceBaseValue(var Dest:TBESENReferenceBaseValue;const Src:TBESENReferenceBaseValue); {$ifdef UseRegister}register;{$endif}
begin
 BESENCopyReferenceBaseValueProcs[Src.ValueType](Dest,Src);
end;

procedure BESENCopyValueUndefined(var Dest:TBESENValue;const Src:TBESENValue); {$ifdef UseRegister}register;{$endif}
begin
 Dest.ValueType:=bvtUNDEFINED;
end;

procedure BESENCopyValueNull(var Dest:TBESENValue;const Src:TBESENValue); {$ifdef UseRegister}register;{$endif}
begin
 Dest.ValueType:=bvtNULL;
end;

procedure BESENCopyValueBoolean(var Dest:TBESENValue;const Src:TBESENValue); {$ifdef UseRegister}register;{$endif}
begin
 Dest.ValueType:=bvtBOOLEAN;
 Dest.Bool:=Src.Bool;
end;

procedure BESENCopyValueNumber(var Dest:TBESENValue;const Src:TBESENValue); {$ifdef UseRegister}register;{$endif}
begin
 Dest.ValueType:=bvtNUMBER;
 Dest.Num:=Src.Num;
end;

procedure BESENCopyValueString(var Dest:TBESENValue;const Src:TBESENValue); {$ifdef UseRegister}register;{$endif}
begin
 Dest.ValueType:=bvtSTRING;
 Dest.Str:=Src.Str;
end;

procedure BESENCopyValueObject(var Dest:TBESENValue;const Src:TBESENValue); {$ifdef UseRegister}register;{$endif}
begin
 Dest.ValueType:=bvtOBJECT;
 Dest.Obj:=Src.Obj;
end;

procedure BESENCopyValueReference(var Dest:TBESENValue;const Src:TBESENValue); {$ifdef UseRegister}register;{$endif}
begin
 Dest.ValueType:=bvtREFERENCE;
 BESENCopyReferenceBaseValue(Dest.ReferenceBase,Src.ReferenceBase);
 Dest.Str:=Src.Str;
 Dest.ReferenceIsStrict:=Src.ReferenceIsStrict;
 Dest.ReferenceHash:=Src.ReferenceHash;
 Dest.ReferenceIndex:=Src.ReferenceIndex;
 Dest.ReferenceID:=Src.ReferenceID;
end;

procedure BESENCopyValueLocal(var Dest:TBESENValue;const Src:TBESENValue); {$ifdef UseRegister}register;{$endif}
begin
 Dest.ValueType:=bvtLOCAL;
 Dest.LocalIndex:=Src.LocalIndex;
end;

procedure BESENCopyValueEnvRec(var Dest:TBESENValue;const Src:TBESENValue); {$ifdef UseRegister}register;{$endif}
begin
 Dest.ValueType:=bvtENVREC;
 Dest.EnvRec:=Src.EnvRec;
end;

procedure BESENCopyValueNone(var Dest:TBESENValue;const Src:TBESENValue); {$ifdef UseRegister}register;{$endif}
begin
 Dest.ValueType:=bvtNONE;
end;

procedure BESENCopyValue(var Dest:TBESENValue;const Src:TBESENValue); {$ifdef UseRegister}register;{$endif}
begin
 BESENCopyValueProcs[Src.ValueType](Dest,Src);
end;

procedure BESENValueToRefBaseValueUndefined(var Dest:TBESENReferenceBaseValue;const Src:TBESENValue); {$ifdef UseRegister}register;{$endif}
begin
 Dest.ValueType:=brbvtUNDEFINED;
end;

procedure BESENValueToRefBaseValueNull(var Dest:TBESENReferenceBaseValue;const Src:TBESENValue); {$ifdef UseRegister}register;{$endif}
begin
 Dest.ValueType:=brbvtUNDEFINED;
end;

procedure BESENValueToRefBaseValueBoolean(var Dest:TBESENReferenceBaseValue;const Src:TBESENValue); {$ifdef UseRegister}register;{$endif}
begin
 Dest.ValueType:=brbvtBOOLEAN;
 Dest.Bool:=Src.Bool;
end;

procedure BESENValueToRefBaseValueNumber(var Dest:TBESENReferenceBaseValue;const Src:TBESENValue); {$ifdef UseRegister}register;{$endif}
begin
 Dest.ValueType:=brbvtNUMBER;
 Dest.Num:=Src.Num;
end;

procedure BESENValueToRefBaseValueString(var Dest:TBESENReferenceBaseValue;const Src:TBESENValue); {$ifdef UseRegister}register;{$endif}
begin
 Dest.ValueType:=brbvtSTRING;
 Dest.Str:=Src.Str;
end;

procedure BESENValueToRefBaseValueObject(var Dest:TBESENReferenceBaseValue;const Src:TBESENValue); {$ifdef UseRegister}register;{$endif}
begin
 Dest.ValueType:=brbvtOBJECT;
 Dest.Obj:=Src.Obj;
end;

procedure BESENValueToRefBaseValueReference(var Dest:TBESENReferenceBaseValue;const Src:TBESENValue); {$ifdef UseRegister}register;{$endif}
begin
 Dest.ValueType:=brbvtUNDEFINED;
end;

procedure BESENValueToRefBaseValueLocal(var Dest:TBESENReferenceBaseValue;const Src:TBESENValue); {$ifdef UseRegister}register;{$endif}
begin
 Dest.ValueType:=brbvtUNDEFINED;
end;

procedure BESENValueToRefBaseValueEnvRec(var Dest:TBESENReferenceBaseValue;const Src:TBESENValue); {$ifdef UseRegister}register;{$endif}
begin
 Dest.ValueType:=brbvtENVREC;
 Dest.EnvRec:=Src.EnvRec;
end;

procedure BESENValueToRefBaseValueNone(var Dest:TBESENReferenceBaseValue;const Src:TBESENValue); {$ifdef UseRegister}register;{$endif}
begin
 Dest.ValueType:=brbvtUNDEFINED;
end;

procedure BESENValueToReferenceBaseValue(const Value:TBESENValue;var AResult:TBESENReferenceBaseValue); {$ifdef UseRegister}register;{$endif}
begin
 BESENValueToRefBaseValueProcs[Value.ValueType](AResult,Value);
end;

procedure BESENRefBaseValueToValueUndefined(var Dest:TBESENValue;const Src:TBESENReferenceBaseValue); {$ifdef UseRegister}register;{$endif}
begin
 Dest.ValueType:=bvtUNDEFINED;
end;

procedure BESENRefBaseValueToValueBoolean(var Dest:TBESENValue;const Src:TBESENReferenceBaseValue); {$ifdef UseRegister}register;{$endif}
begin
 Dest.ValueType:=bvtBOOLEAN;
 Dest.Bool:=Src.Bool;
end;

procedure BESENRefBaseValueToValueNumber(var Dest:TBESENValue;const Src:TBESENReferenceBaseValue); {$ifdef UseRegister}register;{$endif}
begin
 Dest.ValueType:=bvtNUMBER;
 Dest.Num:=Src.Num;
end;

procedure BESENRefBaseValueToValueString(var Dest:TBESENValue;const Src:TBESENReferenceBaseValue); {$ifdef UseRegister}register;{$endif}
begin
 Dest.ValueType:=bvtSTRING;
 Dest.Str:=Src.Str;
end;

procedure BESENRefBaseValueToValueObject(var Dest:TBESENValue;const Src:TBESENReferenceBaseValue); {$ifdef UseRegister}register;{$endif}
begin
 Dest.ValueType:=bvtOBJECT;
 Dest.Obj:=Src.Obj;
end;

procedure BESENRefBaseValueToValueEnvRec(var Dest:TBESENValue;const Src:TBESENReferenceBaseValue); {$ifdef UseRegister}register;{$endif}
begin
 Dest.ValueType:=bvtENVREC;
 Dest.EnvRec:=Src.EnvRec;
end;

procedure BESENReferenceBaseValueToValue(const Value:TBESENReferenceBaseValue;var AResult:TBESENValue); {$ifdef UseRegister}register;{$endif}
begin
 BESENRefBaseValueToValueProcs[Value.ValueType](AResult,Value);
end;

procedure BESENRefBaseValueToCallThisArgValueUndefined(var Dest:TBESENValue;const Src:TBESENReferenceBaseValue); {$ifdef UseRegister}register;{$endif}
begin
 Dest.ValueType:=bvtUNDEFINED;
end;

procedure BESENRefBaseValueToCallThisArgValueBoolean(var Dest:TBESENValue;const Src:TBESENReferenceBaseValue); {$ifdef UseRegister}register;{$endif}
begin
 Dest.ValueType:=bvtBOOLEAN;
 Dest.Bool:=Src.Bool;
end;

procedure BESENRefBaseValueToCallThisArgValueNumber(var Dest:TBESENValue;const Src:TBESENReferenceBaseValue); {$ifdef UseRegister}register;{$endif}
begin
 Dest.ValueType:=bvtNUMBER;
 Dest.Num:=Src.Num;
end;

procedure BESENRefBaseValueToCallThisArgValueString(var Dest:TBESENValue;const Src:TBESENReferenceBaseValue); {$ifdef UseRegister}register;{$endif}
begin
 Dest.ValueType:=bvtSTRING;
 Dest.Str:=Src.Str;
end;

procedure BESENRefBaseValueToCallThisArgValueObject(var Dest:TBESENValue;const Src:TBESENReferenceBaseValue); {$ifdef UseRegister}register;{$endif}
begin
 Dest.ValueType:=bvtOBJECT;
 Dest.Obj:=Src.Obj;
end;

procedure BESENRefBaseValueToCallThisArgValueEnvRec(var Dest:TBESENValue;const Src:TBESENReferenceBaseValue); {$ifdef UseRegister}register;{$endif}
var ImplicitThisValue:PBESENValue;
begin
 ImplicitThisValue:=@TBESENEnvironmentRecord(Src.EnvRec).ImplicitThisValue;
 Dest.ValueType:=ImplicitThisValue.ValueType;
 Dest.Obj:=ImplicitThisValue.Obj;
end;

function BESENValueToVariant(const v:TBESENValue):Variant;
begin
 case v.ValueType of
  bvtNULL:begin
   result:=Variants.Null;
  end;
  bvtBOOLEAN:begin
   result:=V.Bool;
  end;
  bvtSTRING:begin
   result:=V.Str;
  end;
  bvtNUMBER:begin
   result:=V.Num;
  end;
  else begin
   result:=Variants.Unassigned;
  end;
 end;
end;

procedure BESENVariantToValue(const vt:Variant;var v:TBESENValue);
begin
 try
  case VarType(vt) of
   varNull:begin
    V.ValueType:=bvtNULL;
   end;
   varSmallInt,varInteger,varShortInt,varByte,varWord,varLongWord,varInt64{$ifdef fpc},varQWord{$endif}:begin
    V.ValueType:=bvtNUMBER;
    V.Num:=vt;
   end;
   varSingle,varDouble,varDATE,varCurrency:begin
    V.ValueType:=bvtNUMBER;
    V.Num:=vt;
   end;
   varBoolean:begin
    V.ValueType:=bvtBOOLEAN;
    V.Bool:=vt;
   end;
   varString,varOleStr:begin
    V.ValueType:=bvtSTRING;
    V.Str:=vt;
   end;
   else begin
    V.ValueType:=bvtUNDEFINED;
   end;
  end;
 except
  V.ValueType:=bvtUNDEFINED;
 end;
end;

function BESENBooleanValue(const Bool:TBESENBoolean):TBESENValue;
begin
 result.ValueType:=bvtBOOLEAN;
 result.Bool:=result.Bool;
end;

function BESENNumberValue(const Num:TBESENNumber):TBESENValue;
begin
 result.ValueType:=bvtNUMBER;
 result.Num:=Num;
end;

function BESENStringValue(const Str:TBESENString):TBESENValue;
begin
 result.ValueType:=bvtSTRING;
 result.Str:=Str;
end;

function BESENStringLocaleCharsetValue(const Str:TBESENString):TBESENValue; 
begin
 result.ValueType:=bvtSTRING;
 result.Str:=BESENUTF8ToUTF16(BESENEncodeString(Str,BESENLocaleCharset,UTF_8));
end;

function BESENObjectValue(const Obj:TObject):TBESENValue;
begin
 result.ValueType:=bvtOBJECT;
 result.Obj:=Obj;
end;

function BESENObjectValueEx(const Obj:TObject):TBESENValue;
begin
 if assigned(Obj) then begin
  result.ValueType:=bvtOBJECT;
  result.Obj:=Obj;
 end else begin
  result:=BESENNullValue;
 end;
end;

function BESENObjectValueEx2(const Obj:TObject):TBESENValue;
begin
 if assigned(Obj) then begin
  result.ValueType:=bvtOBJECT;
  result.Obj:=Obj;
 end else begin
  result:=BESENUndefinedValue;
 end;
end;

function BESENEqualityExpressionStrictEquals(const a,b:TBESENValue):longbool;
begin
 if a.ValueType<>b.ValueType then begin
  result:=false;
 end else begin
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
 end;
end;

procedure InitBESEN;
begin
 fillchar(BESENEmptyValue,sizeof(TBESENValue),#0);
 fillchar(BESENNullValue,sizeof(TBESENValue),#0);
 fillchar(BESENUndefinedValue,sizeof(TBESENValue),#0);
 BESENEmptyValue.ValueType:=bvtUNDEFINED;
 BESENNullValue.ValueType:=bvtNULL;
 BESENUndefinedValue.ValueType:=bvtUNDEFINED;
 BESENDummyValue:=BESENEmptyValue;
end;

procedure DoneBESEN;
begin
end;

initialization
 InitBESEN;
finalization
 DoneBESEN;
end.
