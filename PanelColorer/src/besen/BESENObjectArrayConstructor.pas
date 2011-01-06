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
unit BESENObjectArrayConstructor;
{$i BESEN.inc}

interface

uses SysUtils,Math,BESENConstants,BESENTypes,BESENObject,BESENObjectFunction,BESENValue,BESENObjectPropertyDescriptor;

type TBESENObjectArrayConstructor=class(TBESENObjectFunction)
      public
       constructor Create(AInstance:TObject;APrototype:TBESENObject=nil;AHasPrototypeProperty:longbool=false); overload; override;
       destructor Destroy; override;
       procedure Construct(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var AResult:TBESENValue); override;
       procedure Call(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var AResult:TBESENValue); override;
       function HasConstruct:TBESENBoolean; override;
       function HasCall:TBESENBoolean; override;
       procedure NativeIsArray(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
     end;

implementation

uses BESEN,BESENGlobals,BESENObjectArray,BESENNumberUtils,BESENErrors;

constructor TBESENObjectArrayConstructor.Create(AInstance:TObject;APrototype:TBESENObject=nil;AHasPrototypeProperty:longbool=false);
begin
 inherited Create(AInstance,APrototype,AHasPrototypeProperty);

 ObjectClassName:='Function';
 ObjectName:='Array';

 RegisterNativeFunction('isArray',NativeIsArray,1,[bopaWRITABLE,bopaCONFIGURABLE],false);
end;

destructor TBESENObjectArrayConstructor.Destroy;
begin
 inherited Destroy;
end;

procedure TBESENObjectArrayConstructor.Construct(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var AResult:TBESENValue);
var r1:TBESENObjectArray;
    r3:TBESENValue;
    l:int64;
    i:integer;
begin
 r3:=BESENEmptyValue;
 r1:=TBESENObjectArray.Create(Instance,TBESEN(Instance).ObjectArrayPrototype,false);
 TBESEN(Instance).GarbageCollector.Add(r1);
 if CountArguments>0 then begin
  r1.GarbageCollectorLock;
  try
   if (CountArguments=1) and (Arguments^[0]^.ValueType=bvtNUMBER) then begin
    r3:=Arguments^[0]^;
    l:=TBESEN(Instance).ToUInt32(r3);
    if (l<>r3.Num) or not BESENIsFinite(r3.Num) then begin
     raise EBESENRangeError.Create('Bad array length');
    end;
    r1.Put('length',r3,true,BESENLengthHash);
   end else begin
    for i:=0 to CountArguments-1 do begin
     r1.DefineOwnProperty(inttostr(i),BESENDataPropertyDescriptor(Arguments^[i]^,[bopaWRITABLE,bopaENUMERABLE,bopaCONFIGURABLE]),true);
    end;
    r3:=BESENNumberValue(CountArguments);
    r1.Put('length',r3,true,BESENLengthHash);
   end;
  finally
   r1.GarbageCollectorUnlock;
  end;
 end;
 AResult.ValueType:=bvtOBJECT;
 AResult.Obj:=r1;
end;

procedure TBESENObjectArrayConstructor.Call(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var AResult:TBESENValue);
begin
 Construct(ThisArgument,Arguments,CountArguments,AResult);
end;

function TBESENObjectArrayConstructor.HasConstruct:TBESENBoolean;
begin
 result:=true;
end;

function TBESENObjectArrayConstructor.HasCall:TBESENBoolean;
begin
 result:=true;
end;

procedure TBESENObjectArrayConstructor.NativeIsArray(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 ResultValue.ValueType:=bvtBOOLEAN;
 ResultValue.Bool:=(CountArguments>0) and (((Arguments^[0]^.ValueType=bvtOBJECT) and assigned(Arguments^[0]^.Obj)) and (TBESENObject(Arguments^[0]^.Obj).ObjectClassName='Array'));
end;

end.
