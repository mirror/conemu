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
unit BESENObjectFunctionArguments;
{$i BESEN.inc}

interface

uses SysUtils,BESENConstants,BESENTypes,BESENObject,BESENValue,BESENObjectPropertyDescriptor;

type TBESENObjectFunctionArguments=class(TBESENObject)
      public
       ParameterMap:TBESENObject;
       IsStrict:longbool;
       constructor Create(AInstance:TObject;APrototype:TBESENObject=nil;AHasPrototypeProperty:longbool=false); overload; override;
       destructor Destroy; override;
       function GetOwnProperty(const P:TBESENString;var Descriptor:TBESENObjectPropertyDescriptor;Hash:TBESENHash=0):boolean; override;
       function GetEx(const P:TBESENString;var AResult:TBESENValue;var Descriptor:TBESENObjectPropertyDescriptor;Base:TBESENObject=nil;Hash:TBESENHash=0):boolean; override;
       function GetIndex(const Index,ID:integer;var AResult:TBESENValue;Base:TBESENObject=nil):boolean; override;
       function DeleteEx(const P:TBESENString;Throw:TBESENBoolean;var Descriptor:TBESENObjectPropertyDescriptor;Hash:TBESENHash=0):TBESENBoolean; override;
       procedure PutEx(const P:TBESENString;const V:TBESENValue;Throw:TBESENBoolean;var Descriptor,OwnDescriptor:TBESENObjectPropertyDescriptor;var TempValue:TBESENValue;Hash:TBESENHash=0); override;
       procedure PutIndex(const Index,ID:integer;const V:TBESENValue;Throw:TBESENBoolean); override;
       function DefineOwnPropertyEx(const P:TBESENString;const Descriptor:TBESENObjectPropertyDescriptor;Throw:TBESENBoolean;var Current:TBESENObjectPropertyDescriptor;Hash:TBESENHash=0):TBESENBoolean; override;
       procedure DefaultValue(const AHint:TBESENValue;var AResult:TBESENValue); override;
       procedure Finalize; override;
       procedure Mark; override;
     end;

implementation

uses BESEN,BESENGlobals,BESENStringUtils,BESENErrors;

constructor TBESENObjectFunctionArguments.Create(AInstance:TObject;APrototype:TBESENObject=nil;AHasPrototypeProperty:longbool=false);
begin
 inherited Create(AInstance,APrototype,AHasPrototypeProperty);
 ObjectClassName:='Arguments';
 ObjectName:='Arguments';
 ParameterMap:=nil;
end;

destructor TBESENObjectFunctionArguments.Destroy;
begin
 inherited Destroy;
end;

function TBESENObjectFunctionArguments.GetOwnProperty(const P:TBESENString;var Descriptor:TBESENObjectPropertyDescriptor;Hash:TBESENHash=0):boolean;
var MappedDescriptor:TBESENObjectPropertyDescriptor;
begin
 if IsStrict or not assigned(ParameterMap) then begin
  result:=inherited GetOwnProperty(P,Descriptor,Hash);
 end else begin
  result:=inherited GetOwnProperty(P,Descriptor,Hash);
  if result then begin
   MappedDescriptor:=Descriptor;
   if not ParameterMap.GetOwnProperty(P,MappedDescriptor,Hash) then begin
    ParameterMap.Get(P,Descriptor.Value);
    result:=Descriptor.Presents<>[];
   end;
  end;
 end;
end;

function TBESENObjectFunctionArguments.GetEx(const P:TBESENString;var AResult:TBESENValue;var Descriptor:TBESENObjectPropertyDescriptor;Base:TBESENObject=nil;Hash:TBESENHash=0):boolean;
begin
 if not assigned(Base) then begin
  Base:=self;
 end;
 if IsStrict or not assigned(ParameterMap) then begin
  result:=inherited GetEx(P,AResult,Descriptor,Base,Hash);
 end else begin
  if ParameterMap.GetOwnProperty(P,Descriptor,Hash) then begin
   result:=ParameterMap.GetEx(P,AResult,Descriptor,Base,Hash);
  end else begin
   result:=inherited GetEx(P,AResult,Descriptor,Base,Hash);
   if IsStrict and (P='caller') then begin
    BESENThrowCaller;
   end;
  end;
 end;
end;

function TBESENObjectFunctionArguments.GetIndex(const Index,ID:integer;var AResult:TBESENValue;Base:TBESENObject=nil):boolean;
var Descriptor:TBESENObjectPropertyDescriptor;
begin
 if not assigned(Base) then begin
  Base:=self;
 end;
 if IsStrict or not assigned(ParameterMap) then begin
  result:=inherited GetIndex(Index,ID,AResult,Base);
 end else begin
  if ParameterMap.GetOwnProperty(TBESEN(Instance).KeyIDManager.List[ID],Descriptor) then begin
   result:=ParameterMap.GetEx(TBESEN(Instance).KeyIDManager.List[ID],AResult,Descriptor,Base);
  end else begin
   result:=inherited GetIndex(Index,ID,AResult,Base);
   if IsStrict and (ID=TBESEN(Instance).KeyIDManager.CallerID) then begin
    BESENThrowCaller;
   end;
  end;
 end;
end;

function TBESENObjectFunctionArguments.DeleteEx(const P:TBESENString;Throw:TBESENBoolean;var Descriptor:TBESENObjectPropertyDescriptor;Hash:TBESENHash=0):TBESENBoolean;
var IsMapped:boolean;
begin
 if IsStrict or not assigned(ParameterMap) then begin
  result:=inherited DeleteEx(P,Throw,Descriptor,Hash);
 end else begin
  IsMapped:=ParameterMap.GetOwnProperty(P,Descriptor,Hash);
  result:=inherited DeleteEx(P,Throw,Descriptor,Hash);
  if result and IsMapped then begin
   ParameterMap.DeleteEx(P,false,Descriptor,Hash);
  end;
 end;
end;

procedure TBESENObjectFunctionArguments.PutEx(const P:TBESENString;const V:TBESENValue;Throw:TBESENBoolean;var Descriptor,OwnDescriptor:TBESENObjectPropertyDescriptor;var TempValue:TBESENValue;Hash:TBESENHash=0);
begin
 PutFull(P,V,false,Descriptor,OwnDescriptor,TempValue,Hash);
end;

procedure TBESENObjectFunctionArguments.PutIndex(const Index,ID:integer;const V:TBESENValue;Throw:TBESENBoolean);
begin
 Put(TBESEN(Instance).KeyIDManager.List[ID],V,Throw);
end;

function TBESENObjectFunctionArguments.DefineOwnPropertyEx(const P:TBESENString;const Descriptor:TBESENObjectPropertyDescriptor;Throw:TBESENBoolean;var Current:TBESENObjectPropertyDescriptor;Hash:TBESENHash=0):TBESENBoolean;
var IsMapped,Allowed:boolean;
 procedure ThrowIt;
 begin
  BESENThrowTypeError('DefineOwnProperty for "'+BESENUTF16ToUTF8(P)+'" failed');
 end;
begin
 if IsStrict or not assigned(ParameterMap) then begin
  result:=inherited DefineOwnPropertyEx(P,Descriptor,false,Current,Hash);
 end else begin
  IsMapped:=ParameterMap.GetOwnProperty(P,Current,Hash);
  Allowed:=inherited DefineOwnPropertyEx(P,Descriptor,false,Current,Hash);
  if not Allowed then begin
   if Throw then begin
    ThrowIt;
   end;
   result:=false;
   exit;
  end;
  if IsMapped then begin
   if ([boppGETTER,boppSETTER]*Descriptor.Presents)<>[] then begin
    ParameterMap.DeleteEx(p,false,Current,Hash);
   end else begin
    if boppVALUE in Descriptor.Presents then begin
     ParameterMap.Put(p,Descriptor.Value,Throw,Hash);
    end;
    if (boppWRITABLE in Descriptor.Presents) and not (bopaWRITABLE in Descriptor.Attributes) then begin
     ParameterMap.DeleteEx(p,false,Current,Hash);
    end;
   end;
  end;
  result:=true;
 end;
end;

procedure TBESENObjectFunctionArguments.DefaultValue(const AHint:TBESENValue;var AResult:TBESENValue);
var i,j:integer;
    s:TBESENString;
    v:TBESENValue;
 procedure ThrowIt;
 begin
  BESENThrowTypeError('Bad default value');
 end;
begin
 if (TBESEN(Instance).Compatibility and COMPAT_JS)<>0 then begin
  s:='[';
  Get('length',v,self,BESENLengthHash);
  j:=TBESEN(Instance).ToInt32(v);
  for i:=0 to j-1 do begin
   if i>0 then begin
    s:=s+', ';
   end;
   Get(IntToStr(i),v);
   s:=s+inttostr(i)+'='+TBESEN(Instance).ToStr(v);
  end;
  s:=s+']';
  AResult:=BESENStringValue(s);
 end else begin
  inherited DefaultValue(AHint,AResult);
 end;
end;

procedure TBESENObjectFunctionArguments.Finalize;
begin
 ParameterMap:=nil;
 inherited Finalize;
end;

procedure TBESENObjectFunctionArguments.Mark;
begin
 if assigned(ParameterMap) then begin
  TBESEN(Instance).GarbageCollector.GrayIt(ParameterMap);
 end;
 inherited Mark;
end;

end.
 