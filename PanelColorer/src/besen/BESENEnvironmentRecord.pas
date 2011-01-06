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
unit BESENEnvironmentRecord;
{$i BESEN.inc}

interface

uses BESENConstants,BESENTypes,BESENValue,BESENBaseObject,
     BESENCollectorObject,BESENObjectPropertyDescriptor,
     BESENSelfBalancedTree,BESENHashMap,
     BESENGarbageCollector;

const BESENEnvironmentRecordTypeDeclarative=0;
      BESENEnvironmentRecordTypeObject=longword($ffffffff);
      
type TBESENEnvironmentRecord=class(TBESENGarbageCollectorObject)
      public
       IsStrict:TBESENBoolean;
       HasMaybeDirectEval:TBESENBoolean;
       ImplicitThisValue:TBESENValue;
       RecordType:TBESENUINT32;
       constructor Create(AInstance:TObject); overload; override;
       destructor Destroy; override;
       function HasBindingEx(const N:TBESENString;var Descriptor:TBESENObjectPropertyDescriptor;Hash:TBESENHash=0):TBESENBoolean; virtual;
       function HasBinding(const N:TBESENString;Hash:TBESENHash=0):TBESENBoolean; virtual;
       function CreateMutableBinding(const N:TBESENString;const D:TBESENBoolean=false;Hash:TBESENHash=0):TBESENBoolean; virtual;
       function SetMutableBindingEx(const N:TBESENString;const V:TBESENValue;const S:TBESENBoolean;var Descriptor,OwnDescriptor:TBESENObjectPropertyDescriptor;var TempValue:TBESENValue;Hash:TBESENHash=0):TBESENBoolean; virtual;
       procedure GetBindingValueEx(const N:TBESENString;const S:TBESENBoolean;var R:TBESENValue;var Descriptor:TBESENObjectPropertyDescriptor;Hash:TBESENHash=0); virtual;
       function SetMutableBinding(const N:TBESENString;const V:TBESENValue;const S:TBESENBoolean;Hash:TBESENHash=0):TBESENBoolean; virtual;
       procedure GetBindingValue(const N:TBESENString;const S:TBESENBoolean;var R:TBESENValue;Hash:TBESENHash=0); virtual;
       function DeleteBindingEx(const N:TBESENString;var Descriptor:TBESENObjectPropertyDescriptor;Hash:TBESENHash=0):TBESENBoolean; virtual;
       function DeleteBinding(const N:TBESENString;Hash:TBESENHash=0):TBESENBoolean; virtual;
       procedure UpdateImplicitThisValue; virtual;
       function CreateImmutableBinding(const N:TBESENString;Hash:TBESENHash=0):TBESENBoolean; virtual;
       function InitializeImmutableBinding(const N:TBESENString;const V:TBESENValue;Hash:TBESENHash=0):TBESENBoolean; virtual;
       function SetBindingValueIndex(const N:TBESENString;const I:integer;Hash:TBESENHash=0):TBESENBoolean; virtual;
       function SetIndexValue(const I,ID:integer;const V:TBESENValue;const S:TBESENBoolean):TBESENBoolean; virtual;
       procedure GetIndexValue(const I,ID:integer;const S:TBESENBoolean;var R:TBESENValue); virtual;
       function DeleteIndex(const I,ID:integer):TBESENBoolean; virtual;
       function SetArrayIndexValue(const I:TBESENUINT32;const V:TBESENValue;const S:TBESENBoolean):TBESENBoolean; virtual;
       procedure GetArrayIndexValue(const I:TBESENUINT32;const S:TBESENBoolean;var R:TBESENValue); virtual;
       function DeleteArrayIndex(const I:TBESENUINT32):TBESENBoolean; virtual;
     end;

implementation

uses BESEN,BESENArrayUtils,BESENHashUtils,BESENErrors;

constructor TBESENEnvironmentRecord.Create(AInstance:TObject);
begin
 inherited Create(AInstance);
 IsStrict:=TBESEN(Instance).IsStrict;
 HasMaybeDirectEval:=true;
 ImplicitThisValue:=BESENUndefinedValue;
end;

destructor TBESENEnvironmentRecord.Destroy;
begin
 inherited Destroy;
end;

{$warnings off}
function TBESENEnvironmentRecord.HasBindingEx(const N:TBESENString;var Descriptor:TBESENObjectPropertyDescriptor;Hash:TBESENHash=0):TBESENBoolean;
begin
 raise EBESENInternalError.Create('201003160116-0000');
end;

function TBESENEnvironmentRecord.HasBinding(const N:TBESENString;Hash:TBESENHash=0):TBESENBoolean;
var Descriptor:TBESENObjectPropertyDescriptor;
begin
 result:=HasBindingEx(N,Descriptor,Hash);
end;

function TBESENEnvironmentRecord.CreateMutableBinding(const N:TBESENString;const D:TBESENBoolean=false;Hash:TBESENHash=0):TBESENBoolean;
begin
 raise EBESENInternalError.Create('201003160116-0001');
end;

function TBESENEnvironmentRecord.SetMutableBindingEx(const N:TBESENString;const V:TBESENValue;const S:TBESENBoolean;var Descriptor,OwnDescriptor:TBESENObjectPropertyDescriptor;var TempValue:TBESENValue;Hash:TBESENHash=0):TBESENBoolean;
begin
 raise EBESENInternalError.Create('201003160116-0002');
end;

procedure TBESENEnvironmentRecord.GetBindingValueEx(const N:TBESENString;const S:TBESENBoolean;var R:TBESENValue;var Descriptor:TBESENObjectPropertyDescriptor;Hash:TBESENHash=0);
begin
 raise EBESENInternalError.Create('201003160116-0003');
end;

function TBESENEnvironmentRecord.SetMutableBinding(const N:TBESENString;const V:TBESENValue;const S:TBESENBoolean;Hash:TBESENHash=0):TBESENBoolean;
var Descriptor,OwnDescriptor:TBESENObjectPropertyDescriptor;
    Temp:TBESENValue;
begin
 result:=SetMutableBindingEx(N,V,S,Descriptor,OwnDescriptor,Temp,Hash);
end;

procedure TBESENEnvironmentRecord.GetBindingValue(const N:TBESENString;const S:TBESENBoolean;var R:TBESENValue;Hash:TBESENHash=0);
var Descriptor:TBESENObjectPropertyDescriptor;
begin
 GetBindingValueEx(N,S,R,Descriptor,Hash);
end;

function TBESENEnvironmentRecord.DeleteBindingEx(const N:TBESENString;var Descriptor:TBESENObjectPropertyDescriptor;Hash:TBESENHash=0):TBESENBoolean;
begin
 raise EBESENInternalError.Create('201003160116-0004');
end;

function TBESENEnvironmentRecord.DeleteBinding(const N:TBESENString;Hash:TBESENHash=0):TBESENBoolean;
var Descriptor:TBESENObjectPropertyDescriptor;
begin
 result:=DeleteBindingEx(N,Descriptor,Hash);
end;

procedure TBESENEnvironmentRecord.UpdateImplicitThisValue;
begin
 raise EBESENInternalError.Create('201003160116-0005');
end;

function TBESENEnvironmentRecord.CreateImmutableBinding(const N:TBESENString;Hash:TBESENHash=0):TBESENBoolean;
begin
 raise EBESENInternalError.Create('201003160116-0006');
end;

function TBESENEnvironmentRecord.InitializeImmutableBinding(const N:TBESENString;const V:TBESENValue;Hash:TBESENHash=0):TBESENBoolean;
begin
 raise EBESENInternalError.Create('201003160116-0007');
end;

function TBESENEnvironmentRecord.SetBindingValueIndex(const N:TBESENString;const I:integer;Hash:TBESENHash=0):TBESENBoolean;
begin
 raise EBESENInternalError.Create('201003160116-0008');
end;

function TBESENEnvironmentRecord.SetIndexValue(const I,ID:integer;const V:TBESENValue;const S:TBESENBoolean):TBESENBoolean;
begin
 raise EBESENInternalError.Create('201003160116-0009');
end;

procedure TBESENEnvironmentRecord.GetIndexValue(const I,ID:integer;const S:TBESENBoolean;var R:TBESENValue);
begin
 raise EBESENInternalError.Create('201003160116-0010');
end;

function TBESENEnvironmentRecord.DeleteIndex(const I,ID:integer):TBESENBoolean;
begin
 raise EBESENInternalError.Create('201003160116-0011');
end;

function TBESENEnvironmentRecord.SetArrayIndexValue(const I:TBESENUINT32;const V:TBESENValue;const S:TBESENBoolean):TBESENBoolean;
var N:TBESENString;
begin
 N:=BESENArrayIndexToStr(I);
 result:=SetMutableBinding(N,V,S,BESENHashKey(N));
 N:='';
end;

procedure TBESENEnvironmentRecord.GetArrayIndexValue(const I:TBESENUINT32;const S:TBESENBoolean;var R:TBESENValue);
var N:TBESENString;
begin
 N:=BESENArrayIndexToStr(I);
 GetBindingValue(N,S,R,BESENHashKey(N));
 N:='';
end;

function TBESENEnvironmentRecord.DeleteArrayIndex(const I:TBESENUINT32):TBESENBoolean;
var N:TBESENString;
begin
 N:=BESENArrayIndexToStr(I);
 result:=DeleteBinding(N,BESENHashKey(N));
 N:='';
end;
{$warnings on}

end.
