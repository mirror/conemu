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
unit BESENObjectEnvironmentRecord;
{$i BESEN.inc}

interface

uses BESENConstants,BESENTypes,BESENEnvironmentRecord,BESENObject,
     BESENObjectPropertyDescriptor,BESENValue;

type TBESENObjectEnvironmentRecord=class(TBESENEnvironmentRecord)
      public
       BindingObject:TBESENObject;
       ProvideThis:TBESENBoolean;
       constructor Create(AInstance:TObject); overload; override;
       destructor Destroy; override;
       function HasBindingEx(const N:TBESENString;var Descriptor:TBESENObjectPropertyDescriptor;Hash:TBESENHash=0):TBESENBoolean; override;
       function CreateMutableBinding(const N:TBESENString;const D:TBESENBoolean=false;Hash:TBESENHash=0):TBESENBoolean; override;
       function SetMutableBindingEx(const N:TBESENString;const V:TBESENValue;const S:TBESENBoolean;var Descriptor,OwnDescriptor:TBESENObjectPropertyDescriptor;var TempValue:TBESENValue;Hash:TBESENHash=0):TBESENBoolean; override;
       procedure GetBindingValueEx(const N:TBESENString;const S:TBESENBoolean;var R:TBESENValue;var Descriptor:TBESENObjectPropertyDescriptor;Hash:TBESENHash=0); override;
       function DeleteBindingEx(const N:TBESENString;var Descriptor:TBESENObjectPropertyDescriptor;Hash:TBESENHash=0):TBESENBoolean; override;
       procedure UpdateImplicitThisValue; override;
       function SetIndexValue(const I,ID:integer;const V:TBESENValue;const S:TBESENBoolean):TBESENBoolean; override;
       procedure GetIndexValue(const I,ID:integer;const S:TBESENBoolean;var R:TBESENValue); override;
       function DeleteIndex(const I,ID:integer):TBESENBoolean; override;
       procedure Finalize; override;
       procedure Mark; override;
     end;

implementation

uses BESEN,BESENStringUtils,BESENErrors;

constructor TBESENObjectEnvironmentRecord.Create(AInstance:TObject);
begin
 inherited Create(AInstance);
 BindingObject:=nil;
 ProvideThis:=false;
 RecordType:=BESENEnvironmentRecordTypeObject;
end;

destructor TBESENObjectEnvironmentRecord.Destroy;
begin
 BindingObject:=nil;
 inherited Destroy;
end;

function TBESENObjectEnvironmentRecord.HasBindingEx(const N:TBESENString;var Descriptor:TBESENObjectPropertyDescriptor;Hash:TBESENHash=0):TBESENBoolean;
begin
 result:=BindingObject.HasPropertyEx(N,Descriptor,Hash);
end;

function TBESENObjectEnvironmentRecord.CreateMutableBinding(const N:TBESENString;const D:TBESENBoolean=false;Hash:TBESENHash=0):TBESENBoolean;
begin
 if BindingObject.HasProperty(N,Hash) then begin
  BESENThrowTypeError('CreateMutableBinding for "'+BESENUTF16ToUTF8(N)+'" failed');
 end;
 if D then begin
  result:=BindingObject.DefineOwnProperty(N,BESENDataPropertyDescriptor(BESENUndefinedValue,[bopaWRITABLE,bopaENUMERABLE,bopaCONFIGURABLE]),true,Hash); // ES5 errata false to true
 end else begin
  result:=BindingObject.DefineOwnProperty(N,BESENDataPropertyDescriptor(BESENUndefinedValue,[bopaWRITABLE,bopaENUMERABLE]),true,Hash); // ES5 errata false to true
 end;
end;

function TBESENObjectEnvironmentRecord.SetMutableBindingEx(const N:TBESENString;const V:TBESENValue;const S:TBESENBoolean;var Descriptor,OwnDescriptor:TBESENObjectPropertyDescriptor;var TempValue:TBESENValue;Hash:TBESENHash=0):TBESENBoolean;
begin
 BindingObject.PutEx(N,V,S,Descriptor,OwnDescriptor,TempValue,Hash);
 result:=true;
end;

procedure TBESENObjectEnvironmentRecord.GetBindingValueEx(const N:TBESENString;const S:TBESENBoolean;var R:TBESENValue;var Descriptor:TBESENObjectPropertyDescriptor;Hash:TBESENHash=0);
 procedure ThrowIt;
 begin
  BESENThrowTypeError('GetBindingValue for "'+BESENUTF16ToUTF8(N)+'" failed');
 end;
begin
 if not BindingObject.GetEx(N,R,Descriptor,BindingObject,Hash) then begin
  if S then begin
   ThrowIt;
  end else begin
   R.ValueType:=bvtUNDEFINED;
  end;
 end;
end;

function TBESENObjectEnvironmentRecord.DeleteBindingEx(const N:TBESENString;var Descriptor:TBESENObjectPropertyDescriptor;Hash:TBESENHash=0):TBESENBoolean;
begin
 result:=BindingObject.DeleteEx(N,false,Descriptor,Hash);
end;

procedure TBESENObjectEnvironmentRecord.UpdateImplicitThisValue;
begin
 if ProvideThis then begin
  ImplicitThisValue.ValueType:=bvtOBJECT;
  ImplicitThisValue.Obj:=BindingObject;
 end else begin
  ImplicitThisValue.ValueType:=bvtUNDEFINED;
  ImplicitThisValue.Obj:=nil;
 end;
end;

function TBESENObjectEnvironmentRecord.SetIndexValue(const I,ID:integer;const V:TBESENValue;const S:TBESENBoolean):TBESENBoolean;
begin
 BindingObject.PutIndex(I,ID,V,S);
 result:=true;
end;

procedure TBESENObjectEnvironmentRecord.GetIndexValue(const I,ID:integer;const S:TBESENBoolean;var R:TBESENValue);
 procedure ThrowIt;
 begin
  BESENThrowTypeError('GetIndexValue failed');
 end;
begin
 if not BindingObject.GetIndex(I,ID,R,BindingObject) then begin
  if S then begin
   ThrowIt;
  end else begin
   R.ValueType:=bvtUNDEFINED;
  end;
 end;
end;

function TBESENObjectEnvironmentRecord.DeleteIndex(const I,ID:integer):TBESENBoolean;
begin
 result:=BindingObject.DeleteIndex(I,iD,false);
end;

procedure TBESENObjectEnvironmentRecord.Finalize;
begin
 BindingObject:=nil;
 inherited Finalize;
end;

procedure TBESENObjectEnvironmentRecord.Mark;
begin
 TBESEN(Instance).GarbageCollector.GrayIt(BindingObject);
 inherited Mark;
end;

end.
