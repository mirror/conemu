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
unit BESENObjectRegExpConstructor;
{$i BESEN.inc}

interface

uses Math,BESENConstants,BESENTypes,BESENObject,BESENObjectFunction,BESENValue,BESENObjectPropertyDescriptor;

type TBESENObjectRegExpConstructor=class(TBESENObjectFunction)
      public
       constructor Create(AInstance:TObject;APrototype:TBESENObject=nil;AHasPrototypeProperty:longbool=false); overload; override;
       destructor Destroy; override;
       procedure Construct(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var AResult:TBESENValue); override;
       procedure Call(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var AResult:TBESENValue); override;
       function HasConstruct:TBESENBoolean; override;
       function HasCall:TBESENBoolean; override;
       function HasInstance(const AInstance:TBESENValue):TBESENBoolean; override;
       function HasHasInstance:TBESENBoolean; override;
     end;

implementation

uses BESEN,BESENObjectRegExp,BESENRegExp,BESENErrors;

constructor TBESENObjectRegExpConstructor.Create(AInstance:TObject;APrototype:TBESENObject=nil;AHasPrototypeProperty:longbool=false);
begin
 inherited Create(AInstance,APrototype,AHasPrototypeProperty);
 ObjectClassName:='Function';
 ObjectName:='RegExp';
end;

destructor TBESENObjectRegExpConstructor.Destroy;
begin
 inherited Destroy;
end;

procedure TBESENObjectRegExpConstructor.Construct(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var AResult:TBESENValue);
var r1:TBESENObjectRegExp;
    s,f:TBESENString;
    Flags:TBESENRegExpFlags;
    v:TBESENValue;
    i:integer;
begin
 r1:=TBESENObjectRegExp.Create(Instance,TBESEN(Instance).ObjectRegExpPrototype,false);
 TBESEN(Instance).GarbageCollector.Add(r1);
 r1.GarbageCollectorLock;
 try
  if (CountArguments>0) and (Arguments^[0]^.ValueType=bvtOBJECT) and assigned(Arguments^[0]^.Obj) and (Arguments^[0]^.Obj is TBESENObjectRegExp) then begin
   r1.Engine:=TBESENObjectRegExp(Arguments^[0]^.Obj).Engine;
  end else begin
   if (CountArguments<1) or (Arguments^[0]^.ValueType=bvtUNDEFINED) then begin
    s:='';
   end else begin
    s:=TBESEN(Instance).ToStr(Arguments^[0]^);
   end;
   if CountArguments>1 then begin
    f:=TBESEN(Instance).ToStr(Arguments^[1]^);
   end else begin
    f:='';
   end;
   Flags:=[];
   for i:=1 to length(f) do begin
    case f[i] of
     'g':begin
      if brefGLOBAL in Flags then begin
       raise EBESENSyntaxError.Create('Too many global regular expression flags');
      end else begin
       Flags:=Flags+[brefGLOBAL];
      end;
     end;
     'i':begin
      if brefIGNORECASE in Flags then begin
       raise EBESENSyntaxError.Create('Too many ignorecase regular expression flags');
      end else begin
       Flags:=Flags+[brefIGNORECASE];
      end;
     end;
     'm':begin
      if brefMULTILINE in Flags then begin
       raise EBESENSyntaxError.Create('Too many multiline regular expression flags');
      end else begin
       Flags:=Flags+[brefMULTILINE];
      end;
     end;
     else begin
      raise EBESENSyntaxError.Create('Unknown regular expression flag');
     end;
    end;
   end;
   try
    r1.Engine:=TBESEN(Instance).RegExpCache.Get(s,Flags);
   except
    raise EBESENSyntaxError.Create('Invalid regular expression');
   end;
  end;

  if assigned(r1.Engine) then begin
   r1.Engine.IncRef;
  end else begin
   raise EBESENError.Create('Fatal error');
  end;

  v.ValueType:=bvtSTRING;
  v.Str:=r1.Engine.Source;
  r1.OverwriteData('source',v,[]);

  v.ValueType:=bvtBOOLEAN;
  v.Bool:=brefGLOBAL in r1.Engine.Flags;
  r1.OverwriteData('global',v,[]);

  v.ValueType:=bvtBOOLEAN;
  v.Bool:=brefIGNORECASE in r1.Engine.Flags;
  r1.OverwriteData('ignoreCase',v,[]);

  v.ValueType:=bvtBOOLEAN;
  v.Bool:=brefMULTILINE in r1.Engine.Flags;
  r1.OverwriteData('multiline',v,[]);

  v.ValueType:=bvtNUMBER;
  v.Num:=0;
  r1.OverwriteData('lastIndex',v,[bopaWRITABLE]);
 finally
  r1.GarbageCollectorUnlock;
 end;
 AResult:=BESENObjectValue(r1);
end;

procedure TBESENObjectRegExpConstructor.Call(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var AResult:TBESENValue);
begin
 Construct(ThisArgument,Arguments,CountArguments,AResult);
end;

function TBESENObjectRegExpConstructor.HasConstruct:TBESENBoolean;
begin
 result:=true;
end;

function TBESENObjectRegExpConstructor.HasCall:TBESENBoolean;
begin
 result:=true;
end;

function TBESENObjectRegExpConstructor.HasInstance(const AInstance:TBESENValue):TBESENBoolean;
begin
 if (TBESEN(Instance).Compatibility and COMPAT_JS)<>0 then begin
  result:=(AInstance.ValueType=bvtOBJECT) and assigned(AInstance.Obj) and (AInstance.Obj is TBESENObjectRegExp);
 end else begin
  raise EBESENTypeError.Create('Has no instance');
 end;
end;

function TBESENObjectRegExpConstructor.HasHasInstance:TBESENBoolean;
begin
 result:=(TBESEN(Instance).Compatibility and COMPAT_JS)<>0;
end;

end.
