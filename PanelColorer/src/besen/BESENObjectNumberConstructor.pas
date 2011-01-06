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
unit BESENObjectNumberConstructor;
{$i BESEN.inc}

interface

uses Math,BESENConstants,BESENTypes,BESENObject,BESENObjectFunction,BESENValue,BESENObjectPropertyDescriptor;

type TBESENObjectNumberConstructor=class(TBESENObjectFunction)
      public
       constructor Create(AInstance:TObject;APrototype:TBESENObject=nil;AHasPrototypeProperty:longbool=false); overload; override;
       destructor Destroy; override;
       procedure Construct(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var AResult:TBESENValue); override;
       procedure Call(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var AResult:TBESENValue); override;
       function HasConstruct:TBESENBoolean; override;
       function HasCall:TBESENBoolean; override;
     end;

implementation

uses BESEN,BESENObjectNumber,BESENNumberUtils,BESENGlobals,BESENObjectArray;

constructor TBESENObjectNumberConstructor.Create(AInstance:TObject;APrototype:TBESENObject=nil;AHasPrototypeProperty:longbool=false);
var v:TBESENValue;
begin
 inherited Create(AInstance,APrototype,AHasPrototypeProperty);
 ObjectClassName:='Function';
 ObjectName:='Number';

 v:=BESENEmptyValue;

 v:=BESENNumberValue(0);
 move(BESENDoubleMax,v.Num,sizeof(TBESENNumber));
 OverwriteData('MAX_VALUE',v,[]);

 v:=BESENNumberValue(0);
 move(BESENDoubleMin,v.Num,sizeof(TBESENNumber));
 OverwriteData('MIN_VALUE',v,[]);

 v:=BESENNumberValue(0);
 move(BESENDoubleNaN,v.Num,sizeof(TBESENNumber));
 OverwriteData('NaN',v,[]);

 v:=BESENNumberValue(0);
 move(BESENDoubleInfNeg,v.Num,sizeof(TBESENNumber));
 OverwriteData('NEGATIVE_INFINITY',v,[]);

 v:=BESENNumberValue(0);
 move(BESENDoubleInfPos,v.Num,sizeof(TBESENNumber));
 OverwriteData('POSITIVE_INFINITY',v,[]);
end;

destructor TBESENObjectNumberConstructor.Destroy;
begin
 inherited Destroy;
end;

procedure TBESENObjectNumberConstructor.Construct(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var AResult:TBESENValue);
var r1:TBESENObjectNumber;
begin
 r1:=TBESENObjectNumber.Create(Instance,TBESEN(Instance).ObjectNumberPrototype,false);
 TBESEN(Instance).GarbageCollector.Add(r1);
 r1.GarbageCollectorLock;
 try
  if CountArguments>0 then begin
   r1.Value:=TBESEN(Instance).ToNum(Arguments^[0]^);
  end else begin
   r1.Value:=0.0;
  end;
 finally
  r1.GarbageCollectorUnlock;
 end;
 AResult.ValueType:=bvtOBJECT;
 AResult.Obj:=r1;
end;

procedure TBESENObjectNumberConstructor.Call(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var AResult:TBESENValue);
var v:TBESENValue;
begin
 if CountArguments<1 then begin
  AResult.ValueType:=bvtNUMBER;
  AResult.Num:=0;
 end else if ((TBESEN(Instance).Compatibility and COMPAT_JS)<>0) and (Arguments^[0]^.ValueType=bvtOBJECT) and assigned(Arguments^[0]^.Obj) and (Arguments^[0]^.Obj is TBESENObjectArray) then begin
  TBESENObjectArray(Arguments^[0]^.Obj).Get('length',v,TBESENObject(Arguments^[0]^.Obj),BESENLengthHash);
  TBESEN(Instance).ToNumberValue(v,AResult);
 end else begin
  TBESEN(Instance).ToNumberValue(Arguments^[0]^,AResult);
 end;
end;

function TBESENObjectNumberConstructor.HasConstruct:TBESENBoolean;
begin
 result:=true;
end;

function TBESENObjectNumberConstructor.HasCall:TBESENBoolean;
begin
 result:=true;
end;

end.
