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
unit BESENObjectPrototype;
{$i BESEN.inc}

interface

uses BESENConstants,BESENTypes,BESENObject,BESENValue,BESENObjectPropertyDescriptor;

type TBESENObjectPrototype=class(TBESENObject)
      public
       constructor Create(AInstance:TObject;APrototype:TBESENObject=nil;AHasPrototypeProperty:longbool=false); overload; override;
       destructor Destroy; override;
       procedure NativeToString(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeToLocaleString(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeToSource(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeValueOf(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeHasOwnProperty(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeIsPrototypeOf(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativePropertyIsEnumerable(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
     end;

implementation

uses BESEN,BESENErrors;

constructor TBESENObjectPrototype.Create(AInstance:TObject;APrototype:TBESENObject=nil;AHasPrototypeProperty:longbool=false);
begin
 inherited Create(AInstance,APrototype,AHasPrototypeProperty);
 ObjectName:='prototype';

 if (TBESEN(Instance).Compatibility and COMPAT_JS)<>0 then begin
  OverwriteData('name',BESENStringValue(ObjectName),[]);
 end;

 RegisterNativeFunction('toString',NativeToString,0,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('toLocaleString',NativeToLocaleString,0,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('toSource',NativeToSource,0,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('valueOf',NativeValueOf,0,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('hasOwnProperty',NativeHasOwnProperty,1,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('isPrototypeOf',NativeIsPrototypeOf,0,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('propertyIsEnumerable',NativePropertyIsEnumerable,0,[bopaWRITABLE,bopaCONFIGURABLE],false);
end;

destructor TBESENObjectPrototype.Destroy;
begin
 inherited Destroy;
end;

procedure TBESENObjectPrototype.NativeToString(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
var o:TBesenObject;
begin
 // ES5 errata fix
 case ThisArgument.ValueType of
  bvtUNDEFINED:begin
   ResultValue:=BESENStringValue('[object Undefined]');
  end;
  bvtNULL:begin
   ResultValue:=BESENStringValue('[object Null]');
  end;
  else begin
   o:=TBESEN(Instance).ToObj(ThisArgument);
   o.GarbageCollectorLock;
   try
    if assigned(o) then begin
     if length(o.ObjectClassName)>0 then begin
      ResultValue:=BESENStringValue('[object '+o.ObjectClassName+']');
     end else begin
      ResultValue:=BESENStringValue('[object Object]');
     end;
    end else begin
     BESENThrowTypeError('Null this object');
    end;
   finally
    o.GarbageCollectorUnlock;
   end;
  end;
 end;
end;

procedure TBESENObjectPrototype.NativeToLocaleString(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
var v:TBESENValue;
    o:TBesenObject;
begin
 o:=TBESEN(Instance).ToObj(ThisArgument);
 if assigned(o) then begin
  o.GarbageCollectorLock;
  try
   o.Get('toString',v);
   if (v.ValueType=bvtOBJECT) and assigned(TBESENObject(v.Obj)) and TBESENObject(v.Obj).HasCall then begin
    TBESEN(Instance).ObjectCall(TBESENObject(v.Obj),ThisArgument,Arguments,CountArguments,ResultValue);
   end else begin
    BESENThrowTypeError('Null this object');
   end;
  finally
   o.GarbageCollectorUnlock;
  end;
 end else begin
  BESENThrowTypeError('Null this object');
 end;
end;

procedure TBESENObjectPrototype.NativeToSource(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 ResultValue:=BESENStringValue('/* Unimplemented */');
end;

procedure TBESENObjectPrototype.NativeValueOf(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 TBESEN(Instance).ToObjectValue(ThisArgument,ResultValue);
end;

procedure TBESENObjectPrototype.NativeHasOwnProperty(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
var Descriptor:TBESENObjectPropertyDescriptor;
    o:TBesenObject;
begin
 o:=TBESEN(Instance).ToObj(ThisArgument);
 if not assigned(o) then begin
  raise EBESENTypeError.Create('Null this object');
 end;
 o.GarbageCollectorLock;
 try
  ResultValue.ValueType:=bvtBOOLEAN;
  if CountArguments>0 then begin
   ResultValue.Bool:=o.GetOwnProperty(TBESEN(Instance).ToStr(Arguments^[0]^),Descriptor);
  end else begin
   ResultValue.Bool:=false;
  end;
 finally
  o.GarbageCollectorUnlock;
 end;
end;

procedure TBESENObjectPrototype.NativeIsPrototypeOf(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
var v,o:TBESENObject;
begin
 ResultValue.ValueType:=bvtBOOLEAN;
 ResultValue.Bool:=false;
 if (CountArguments>0) and (Arguments^[0]^.ValueType=bvtOBJECT) then begin
  o:=TBESEN(Instance).ToObj(ThisArgument);
  if not assigned(o) then begin
   raise EBESENTypeError.Create('Null this object');
  end;
  o.GarbageCollectorLock;
  try
   v:=TBESENObject(Arguments^[0]^.Obj);
   while assigned(v) do begin
    v:=v.Prototype;
    if assigned(v) then begin
     if o=v then begin
      ResultValue.Bool:=true;
      break;
     end;
    end else begin
     ResultValue.Bool:=false;
     break;
    end;
   end;
  finally
   o.GarbageCollectorUnlock;
  end;
 end else begin
  ResultValue.Bool:=false;
 end;
end;

procedure TBESENObjectPrototype.NativePropertyIsEnumerable(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
var Descriptor:TBESENObjectPropertyDescriptor;
    o:TBESENObject;
    s:TBESENString;
begin
 ResultValue.ValueType:=bvtBOOLEAN;
 if CountArguments>0 then begin
  s:=TBESEN(Instance).ToStr(Arguments^[0]^);
  o:=TBESEN(Instance).ToObj(ThisArgument);
  if not assigned(o) then begin
   raise EBESENTypeError.Create('Null this object');
  end;
  o.GarbageCollectorLock;
  try
   if o.GetOwnProperty(s,Descriptor) then begin
    ResultValue.Bool:=(boppENUMERABLE in Descriptor.Presents) and (bopaENUMERABLE in Descriptor.Attributes);
   end else begin
    ResultValue.Bool:=false;
   end;
  finally
   o.GarbageCollectorUnlock;
  end;
 end else begin
  ResultValue.Bool:=false;
 end;
end;

end.
 