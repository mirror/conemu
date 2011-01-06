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
unit BESENObjectRegExp;
{$i BESEN.inc}

interface

uses Math,BESENConstants,BESENTypes,BESENObject,BESENValue,BESENObjectPropertyDescriptor,
     BESENRegExp;

type TBESENObjectRegExp=class(TBESENObject)
      public
       Engine:TBESENRegExp;
       constructor Create(AInstance:TObject;APrototype:TBESENObject=nil;AHasPrototypeProperty:longbool=false); overload; override;
       destructor Destroy; override;
       procedure Call(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var AResult:TBESENValue); override;
       function HasCall:TBESENBoolean; override;
       procedure Finalize; override;
       procedure Mark; override;
       procedure SetStatic(const Input:TBESENString;const Captures:TBESENRegExpCaptures);
     end;

implementation

uses BESEN,BESENErrors;

constructor TBESENObjectRegExp.Create(AInstance:TObject;APrototype:TBESENObject=nil;AHasPrototypeProperty:longbool=false);
var bv:TBESENValue;
begin
 inherited Create(AInstance,APrototype);
 ObjectClassName:='RegExp';
 ObjectName:='';

 Engine:=TBESEN(Instance).DefaultRegExp;

 OverwriteData('source',BESENStringValue(''),[]);

 bv.ValueType:=bvtBOOLEAN;
 bv.Bool:=brefGLOBAL in Engine.Flags;
 OverwriteData('global',bv,[]);

 bv.Bool:=brefIGNORECASE in Engine.Flags;
 OverwriteData('ignoreCase',bv,[]);

 bv.Bool:=brefMULTILINE in Engine.Flags;
 OverwriteData('multiline',bv,[]);

 OverwriteData('lastIndex',BESENNumberValue(0),[bopaWRITABLE]);
end;

destructor TBESENObjectRegExp.Destroy;
begin
 if assigned(Engine) then begin
  if Engine<>TBESEN(Instance).DefaultRegExp then begin
   Engine.DecRef;
  end;
  Engine:=nil;
 end;
 inherited Destroy;
end;

procedure TBESENObjectRegExp.Call(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var AResult:TBESENValue);
begin
 if (TBESEN(Instance).Compatibility and COMPAT_JS)<>0 then begin
  TBESEN(Instance).ObjectRegExpPrototype.NativeExec(ThisArgument,Arguments,CountArguments,AResult);
 end else begin
  raise EBESENTypeError.Create('Not callable');
 end;
end;

function TBESENObjectRegExp.HasCall:TBESENBoolean;
begin
 result:=(TBESEN(Instance).Compatibility and COMPAT_JS)<>0;
end;

procedure TBESENObjectRegExp.Finalize;
begin
 inherited Finalize;
end;

procedure TBESENObjectRegExp.Mark;
begin
 inherited Mark;
end;

procedure TBESENObjectRegExp.SetStatic(const Input:TBESENString;const Captures:TBESENRegExpCaptures);
var i:integer;
    v:TBESENValue;
    pn,lastParen:TBESENString;
begin
 if (TBESEN(Instance).Compatibility and COMPAT_JS)=0 then begin
  exit;
 end;                            
 v:=BESENEmptyValue;
 lastParen:='';
 for i:=0 to 9 do begin
  case i of
   0:pn:='$&';
   1:pn:='$1';
   2:pn:='$2';
   3:pn:='$3';
   4:pn:='$4';
   5:pn:='$5';
   6:pn:='$6';
   7:pn:='$7';
   8:pn:='$8';
   9:pn:='$9';
   else pn:='$0';
  end;
  if (i<length(Captures)) and (Captures[i].e<>brecUNDEFINED) then begin
   v:=BESENStringValue(copy(Input,Captures[i].s+1,Captures[i].e-Captures[i].s));
  end else begin
   v:=BESENStringValue('');
  end;
  if (i>0) and (i<length(Captures)) then begin
   lastParen:=v.Str;
  end;
  OverwriteData(pn,v,[bopaWRITABLE,bopaENUMERABLE,bopaCONFIGURABLE]);
  if i=0 then begin
   OverwriteData('lastMatch',v,[bopaWRITABLE,bopaCONFIGURABLE]);
  end;
 end;

 v.ValueType:=bvtBOOLEAN;
 v.Bool:=brefMULTILINE in Engine.Flags;
 OverwriteData('$*',v,[bopaWRITABLE,bopaCONFIGURABLE]);
 OverwriteData('multiline',v,[]);

 v:=BESENStringValue(Input);
 OverwriteData('$_',v,[bopaWRITABLE,bopaCONFIGURABLE]);
 OverwriteData('input',v,[bopaWRITABLE,bopaCONFIGURABLE]);

 v:=BESENStringValue(lastParen);
 OverwriteData('$+',v,[bopaWRITABLE,bopaCONFIGURABLE]);
 OverwriteData('leftParen',v,[bopaWRITABLE,bopaCONFIGURABLE]);

 if (length(Captures)>0) and not (brefGLOBAL in Engine.Flags) then begin
  v:=BESENStringValue(copy(Input,1,Captures[0].s));
 end else begin
  v:=BESENStringValue('');
 end;
 OverwriteData('$`',v,[bopaWRITABLE,bopaCONFIGURABLE]);
 OverwriteData('leftContext',v,[bopaWRITABLE,bopaCONFIGURABLE]);

 if (length(Captures)>0) and not (brefGLOBAL in Engine.Flags) then begin
  v:=BESENStringValue(copy(Input,Captures[0].e,(length(Input)-longint(Captures[0].e))+1));
 end else begin
  v:=BESENStringValue('');
 end;
 OverwriteData('$''',v,[bopaWRITABLE,bopaCONFIGURABLE]);
 OverwriteData('rightContext',v,[bopaWRITABLE,bopaCONFIGURABLE]);

 v.ValueType:=bvtBOOLEAN;
 v.Bool:=brefGLOBAL in Engine.Flags;
 OverwriteData('global',v,[]);

 v.ValueType:=bvtBOOLEAN;
 v.Bool:=brefIGNORECASE in Engine.Flags;
 OverwriteData('ignoreCase',v,[]);

 if (length(Captures)>0) and not (brefGLOBAL in Engine.Flags) then begin
  v:=BESENNumberValue(Captures[0].e);
 end else begin
  v:=BESENNumberValue(0);
 end;
 OverwriteData('lastIndex',v,[bopaWRITABLE]);

 OverwriteData('source',BESENStringValue(Engine.Source),[]);
end;

end.
