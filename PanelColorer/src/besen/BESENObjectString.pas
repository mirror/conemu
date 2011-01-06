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
unit BESENObjectString;
{$i BESEN.inc}

interface

uses SysUtils,Math,BESENConstants,BESENTypes,BESENObject,BESENValue,BESENObjectPropertyDescriptor;

type TBESENObjectString=class(TBESENObject)
      public
       Value:TBESENString;
       constructor Create(AInstance:TObject;APrototype:TBESENObject=nil;AHasPrototypeProperty:longbool=false); overload; override;
       destructor Destroy; override;
       function GetEx(const P:TBESENString;var AResult:TBESENValue;var Descriptor:TBESENObjectPropertyDescriptor;Base:TBESENObject=nil;Hash:TBESENHash=0):boolean; override;
       function GetOwnProperty(const P:TBESENString;var Descriptor:TBESENObjectPropertyDescriptor;Hash:TBESENHash=0):boolean; override;
       procedure Finalize; override;
       procedure Mark; override;
       procedure UpdateLength;
     end;

implementation

uses BESEN,BESENStringUtils,BESENNumberUtils;

constructor TBESENObjectString.Create(AInstance:TObject;APrototype:TBESENObject=nil;AHasPrototypeProperty:longbool=false);
begin
 inherited Create(AInstance,APrototype,AHasPrototypeProperty);
 ObjectClassName:='String';
 ObjectName:='';

 Value:='';
 OverwriteData('length',BESENNumberValue(0),[]);
end;

destructor TBESENObjectString.Destroy;
begin
 Value:='';
 inherited Destroy;
end;

function TBESENObjectString.GetEx(const P:TBESENString;var AResult:TBESENValue;var Descriptor:TBESENObjectPropertyDescriptor;Base:TBESENObject=nil;Hash:TBESENHash=0):boolean;
begin
 result:=false;
 AResult.ValueType:=bvtUNDEFINED;
 if GetProperty(P,Descriptor,Hash) then begin
  if ([boppVALUE,boppWRITABLE]*Descriptor.Presents)<>[] then begin
   if boppVALUE in Descriptor.Presents then begin
    BESENCopyValue(AResult,Descriptor.Value);
   end;
  end else if ([boppGETTER,boppSETTER]*Descriptor.Presents)<>[] then begin
   if assigned(Base) then begin
    GetGetter(Base,AResult,Descriptor);
   end else begin
    GetGetter(self,AResult,Descriptor);
   end;
  end;
  result:=true;
 end;
end;

function TBESENObjectString.GetOwnProperty(const P:TBESENString;var Descriptor:TBESENObjectPropertyDescriptor;Hash:TBESENHash=0):boolean;
var Index:int64;
    v:TBESENValue;
begin
 // ES5 errata fix
 Descriptor.Value.ValueType:=BESENUndefinedPropertyDescriptor.Value.ValueType;
 Descriptor.Getter:=BESENUndefinedPropertyDescriptor.Getter;
 Descriptor.Setter:=BESENUndefinedPropertyDescriptor.Setter;
 Descriptor.Attributes:=BESENUndefinedPropertyDescriptor.Attributes;
 Descriptor.Presents:=BESENUndefinedPropertyDescriptor.Presents;
 result:=inherited GetOwnProperty(P,Descriptor,Hash);
 if not result then begin
  TBESEN(Instance).ToIntegerValue(BESENStringValue(p),v);
  if BESENIsFinite(v.Num) then begin
   Index:=BESENToInt(v.Num);
   if (IntToStr(abs(Index))=P) and ((Index>=0) and (Index<length(Value))) then begin
    Descriptor.Value.ValueType:=bvtSTRING;
    Descriptor.Value.Str:=copy(Value,Index+1,1);
    Descriptor.Getter:=nil;
    Descriptor.Setter:=nil;
    Descriptor.Attributes:=[bopaENUMERABLE];
    Descriptor.Presents:=[boppVALUE,boppWRITABLE,boppENUMERABLE,boppCONFIGURABLE];
    result:=true;
   end;
  end;
 end;
end;

procedure TBESENObjectString.Finalize;
begin
 inherited Finalize;
end;

procedure TBESENObjectString.Mark;
begin
 inherited Mark;
end;

procedure TBESENObjectString.UpdateLength;
begin
 OverwriteData('length',BESENNumberValue(length(Value)),[]);
end;

end.
