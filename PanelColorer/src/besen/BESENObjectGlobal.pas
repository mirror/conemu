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
unit BESENObjectGlobal;
{$i BESEN.inc}

interface

uses Math,BESENConstants,BESENTypes,BESENObject,BESENValue,BESENObjectPropertyDescriptor,BESENCharSet;

type TBESENObjectGlobal=class(TBESENObject)
      private
       function Encode(const s:TBESENString;const NotToEscapeChars:TBESENCharBitmap):TBESENString;
       function Decode(const s:TBESENString;const ReservedChars:TBESENCharBitmap):TBESENString;
      public
       constructor Create(AInstance:TObject;APrototype:TBESENObject=nil;AHasPrototypeProperty:longbool=false); overload; override;
       destructor Destroy; override;
       procedure NativeEval(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeParseInt(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeParseFloat(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeIsNaN(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeIsFinite(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeDecodeURI(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeDecodeURIComponent(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeEncodeURI(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeEncodeURIComponent(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeEscape(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeUnescape(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeCompatability(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
     end;

implementation

uses BESEN,BESENDTOA,BESENNumberUtils,BESENStringUtils,BESENErrors;

const BESENCompatibilityModes:TBESENCompatibilityModes=(
        (Name:'utf8_unsafe';Flag:COMPAT_UTF8_UNSAFE),
        (Name:'sgmlcom';Flag:COMPAT_SGMLCOM),
        (Name:'besen';Flag:COMPAT_BESEN),
        (Name:'js';Flag:COMPAT_JS));

constructor TBESENObjectGlobal.Create(AInstance:TObject;APrototype:TBESENObject=nil;AHasPrototypeProperty:longbool=false);
var v:TBESENValue;
begin
 inherited Create(AInstance,APrototype,AHasPrototypeProperty);

 v:=BESENEmptyValue;

 v:=BESENNumberValue(double(pointer(@BESENDoubleNaN)^));
 OverwriteData('NaN',v,[]);

 v:=BESENNumberValue(double(pointer(@BESENDoubleInfPos)^));
 OverwriteData('Infinity',v,[]);

 v:=BESENUndefinedValue;
 OverwriteData('undefined',v,[]);

 RegisterNativeFunction('eval',NativeEval,1,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('parseInt',NativeParseInt,2,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('parseFloat',NativeParseFloat,1,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('isNaN',NativeIsNaN,1,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('isFinite',NativeIsFinite,1,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('decodeURI',NativeDecodeURI,1,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('decodeURIComponent',NativeDecodeURIComponent,1,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('encodeURI',NativeEncodeURI,1,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('encodeURIComponent',NativeEncodeURIComponent,1,[bopaWRITABLE,bopaCONFIGURABLE],false);

 RegisterNativeFunction('escape',NativeEscape,1,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('unescape',NativeUnescape,1,[bopaWRITABLE,bopaCONFIGURABLE],false);

 if (TBESEN(Instance).Compatibility and COMPAT_BESEN)<>0 then begin
  RegisterNativeFunction('compat',NativeCompatability,1,[bopaWRITABLE,bopaCONFIGURABLE],false);
  RegisterNativeFunction('compatability',NativeCompatability,1,[bopaWRITABLE,bopaCONFIGURABLE],false);
 end;
end;

destructor TBESENObjectGlobal.Destroy;
begin
 inherited Destroy;
end;

procedure TBESENObjectGlobal.NativeEval(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 ResultValue:=BESENUndefinedValue;
end;

procedure TBESENObjectGlobal.NativeParseInt(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
var s:TBESENString;
    i,StartPos,UntilPos,r,c:integer;
    Negative:boolean;
begin
 ResultValue.ValueType:=bvtNUMBER;
 ResultValue.Num:=TBESENNumber(pointer(@BESENDoubleNaN)^);
 if CountArguments>0 then begin
  if CountArguments>1 then begin
   r:=TBESEN(Instance).ToInt(Arguments^[1]^);
  end else begin
   r:=0;
  end;
  if (r<>0) and ((r<2) or (r>36)) then begin
   ResultValue.Num:=TBESENNumber(pointer(@BESENDoubleNaN)^);
   exit;
  end;
  s:=TBESEN(Instance).ToStr(Arguments^[0]^);
  i:=1;
  while (i<=length(s)) and (BESENUnicodeIsStringWhiteSpace(word(widechar(s[i])))) do begin
   inc(i);
  end;
  if (i<=length(s)) and ((s[i]='-') or (s[i]='+')) then begin
   Negative:=s[i]='-';
   inc(i);
  end else begin
   Negative:=false;
  end;
  if (r in [0,16]) and ((i+1)<=length(s)) and (s[i]='0') and ((s[i+1]='x') or (s[i+1]='X')) then begin
   r:=16;
   inc(i,2);
  end else if (((TBESEN(Instance).Compatibility and COMPAT_JS)<>0) and not TBESEN(Instance).IsStrict) and ((r=0) and ((i<=length(s)) and (s[i]='0'))) then begin
   r:=8;
   inc(i);
  end;
  if r=0 then begin
   r:=10;
  end;
  StartPos:=i;
  UntilPos:=i;
  while UntilPos<=length(s) do begin
   c:=word(widechar(s[UntilPos]));
   case c of
    ord('0')..ord('9'):begin
     c:=c-ord('0');
    end;
    ord('a')..ord('z'):begin
     c:=(c+10)-ord('a');
    end;
    ord('A')..ord('Z'):begin
     c:=(c+10)-ord('A');
    end;
    else begin
     break;
    end;
   end;
   if c>=r then begin
    break;
   end;
   inc(UntilPos);
  end;
  if StartPos>=UntilPos then begin
   ResultValue.Num:=TBESENNumber(pointer(@BESENDoubleNaN)^);
   exit;
  end;
  i:=UntilPos-StartPos;
  if r=10 then begin
   ResultValue.Num:=BESENStringToNumber(copy(s,StartPos,UntilPos-StartPos),false,false);
  end else begin
   ResultValue.Num:=basetod(pansichar(ansistring(copy(s,StartPos,UntilPos-StartPos))),r);
  end;
  if Negative and not BESENIsNaN(ResultValue.Num) then begin
   PBESENDoubleHiLo(@ResultValue.Num)^.Hi:=PBESENDoubleHiLo(@ResultValue.Num)^.Hi or $80000000;
  end;
 end;
end;

procedure TBESENObjectGlobal.NativeParseFloat(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 ResultValue.ValueType:=bvtNUMBER;
 if CountArguments=0 then begin
  ResultValue.Num:=TBESENNumber(pointer(@BESENDoubleNaN)^);
 end else begin
  ResultValue.Num:=BESENStringToNumber(TBESEN(Instance).ToStr(Arguments^[0]^),false,false);
 end;
end;

procedure TBESENObjectGlobal.NativeIsNaN(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 ResultValue.ValueType:=bvtBOOLEAN;
 if CountArguments=0 then begin
  ResultValue.Bool:=true;
 end else begin
  ResultValue.Bool:=BESENIsNaN(TBESEN(Instance).ToNum(Arguments^[0]^));
 end;
end;

procedure TBESENObjectGlobal.NativeIsFinite(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 ResultValue.ValueType:=bvtBOOLEAN;
 if CountArguments=0 then begin
  ResultValue.Bool:=false;
 end else begin
  ResultValue.Bool:=BESENIsFinite(TBESEN(Instance).ToNum(Arguments^[0]^));
 end;
end;

function TBESENObjectGlobal.Encode(const s:TBESENString;const NotToEscapeChars:TBESENCharBitmap):TBESENString;
var i:integer;
    c:TBESENUTF32CHAR;
    r:TBESENString;
 procedure DoEscape(v:longword);
 begin
  r:=r+'%'+BESENHexChars[false,(v shr 4) and $f]+BESENHexChars[false,v and $f];
 end;
begin
 r:='';
 i:=1;
 while i<=length(s) do begin
  c:=word(widechar(s[i]));
  if (c and $fc00)=$dc00 then begin
   raise EBESENURIError.Create('Bad UTF16 string');
  end else if (c and $fc00)=$d800 then begin
   if ((i+1)<=length(s)) and ((word(widechar(s[i+1])) and $fc00)=$dc00) then begin
    c:=(((c and $3ff) shl 10) or (word(widechar(s[i+1])) and $3ff))+$10000;
    inc(i,2);
   end else begin
    raise EBESENURIError.Create('Bad UTF16 string');
   end;
  end else begin
   inc(i);
  end;
  if c<=$7f then begin
   if (NotToEscapeChars[(c and $7f) shr 3] and (1 shl (c and 7)))<>0 then begin
    r:=r+widechar(word(c));
   end else begin
    DoEscape(c);
   end;                 
  end else if c<=$7ff then begin
   DoEscape($c0 or (c shr 6));
   DoEscape($80 or (c and $3f));
  end else if c<=$ffff then begin
   DoEscape($e0 or (c shr 12));
   DoEscape($80 or ((c shr 6) and $3f));
   DoEscape($80 or (c and $3f));
  end else if c<=$1fffff then begin
   DoEscape($f0 or (c shr 18));
   DoEscape($80 or ((c shr 12) and $3f));
   DoEscape($80 or ((c shr 6) and $3f));
   DoEscape($80 or (c and $3f));
  end else begin
   raise EBESENURIError.Create('Bad UTF16 string');
  end;
 end;
 result:=r;
end;

function TBESENObjectGlobal.Decode(const s:TBESENString;const ReservedChars:TBESENCharBitmap):TBESENString;
const UTF8Mask:array[0..5] of byte=($c0,$e0,$f0,$f8,$fc,$fe);
var i,j,k,h:integer;
    c,ac:TBESENUTF32CHAR;
begin
 result:='';
 i:=1;
 while i<=length(s) do begin
  j:=i;
  c:=word(widechar(s[i]));
  if (c and $fc00)=$dc00 then begin
   raise EBESENURIError.Create('Bad UTF16 string');
  end else if (c and $fc00)=$d800 then begin
   if ((i+1)<=length(s)) and ((word(widechar(s[i+1])) and $fc00)=$dc00) then begin
    c:=(((c and $3ff) shl 10) or (word(widechar(s[i+1])) and $3ff))+$10000;
    inc(i,2);
   end else begin
    raise EBESENURIError.Create('Bad UTF16 string');
   end;
  end else begin
   inc(i);
  end;
  if c=ord('%') then begin
   if ((i+1)<=length(s)) and (BESENIsHex(word(widechar(s[i]))) and BESENIsHex(word(widechar(s[i+1])))) then begin
    c:=(BESENHexValues[word(widechar(s[i]))] shl 4) or BESENHexValues[word(widechar(s[i+1]))];
   end else begin
    raise EBESENURIError.Create('Bad URI hex');
   end;
   inc(i,2);
   if (c and $80)<>0 then begin
    k:=1;
    while k<6 do begin
     if (c and UTF8Mask[k])=UTF8Mask[k-1] then begin
      break;
     end;
     inc(k);
    end;
    if k>=6 then begin
     raise EBESENURIError.Create('Bad UTF8');
    end;
    c:=c and not UTF8Mask[k];
    for h:=1 to k do begin
     if ((i+2)<=length(s)) and ((s[i]='%') and (BESENIsHex(word(widechar(s[i+1]))) and BESENIsHex(word(widechar(s[i+2]))))) then begin
      ac:=(BESENHexValues[word(widechar(s[i+1]))] shl 4) or BESENHexValues[word(widechar(s[i+2]))];
     end else begin
      raise EBESENURIError.Create('Bad URI hex');
     end;
     inc(i,3);
     if (ac and not $3f)<>$80 then begin
      raise EBESENURIError.Create('Bad UTF8');
     end;
     c:=(C shl 6) or (ac and $3f);
    end;
   end;
  end;
  if c<=$ffff then begin
   if (c<=$7f) and ((ReservedChars[(c and $7f) shr 3] and (1 shl (c and 7)))<>0) then begin
    result:=result+copy(s,j,(i-j)+1);
   end else begin
    result:=result+widechar(word(c));
   end;
  end else if c<=$10ffff then begin
   dec(c,$100000);
   result:=result+widechar(word($d800 or ((c shr 10) and $3ff)))+widechar(word($dc00 or (c and $3ff)));
  end else begin
   raise EBESENURIError.Create('Bad unicode');
  end;
 end;
end;

procedure TBESENObjectGlobal.NativeDecodeURI(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
const CharBitmap:TBESENCharBitmap=($00,$00,$00,$00,$58,$98,$00,$ac,$01,$00,$00,$00,$00,$00,$00,$00); // [;/?:@&=+$,#]
begin
 if CountArguments=0 then begin
  ResultValue:=BESENStringValue('undefined');
 end else begin
  ResultValue:=BESENStringValue(Decode(TBESEN(Instance).ToStr(Arguments^[0]^),CharBitmap));
 end;
end;

procedure TBESENObjectGlobal.NativeDecodeURIComponent(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
const CharBitmap:TBESENCharBitmap=($00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00); // []
begin
 if CountArguments=0 then begin
  ResultValue:=BESENStringValue('undefined');
 end else begin
  ResultValue:=BESENStringValue(Decode(TBESEN(Instance).ToStr(Arguments^[0]^),CharBitmap));
 end;
end;

procedure TBESENObjectGlobal.NativeEncodeURI(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
const CharBitmap:TBESENCharBitmap=($00,$00,$00,$00,$da,$ff,$ff,$af,$ff,$ff,$ff,$87,$fe,$ff,$ff,$47); // [-_.!~*'();/?:@&=+$,#a-zA-Z0-9]
begin
 if CountArguments=0 then begin
  ResultValue:=BESENStringValue('undefined');
 end else begin
  ResultValue:=BESENStringValue(Encode(TBESEN(Instance).ToStr(Arguments^[0]^),CharBitmap));
 end;
end;

procedure TBESENObjectGlobal.NativeEncodeURIComponent(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
const CharBitmap:TBESENCharBitmap=($00,$00,$00,$00,$82,$67,$ff,$03,$fe,$ff,$ff,$87,$fe,$ff,$ff,$47); // [-_.!~*'()a-zA-Z0-9]
begin
 if CountArguments=0 then begin
  ResultValue:=BESENStringValue('undefined');
 end else begin
  ResultValue:=BESENStringValue(Encode(TBESEN(Instance).ToStr(Arguments^[0]^),CharBitmap));
 end;
end;

procedure TBESENObjectGlobal.NativeEscape(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
const NotToEscapeChars:TBESENCharBitmap=($00,$00,$00,$00,$00,$ec,$ff,$03,$ff,$ff,$ff,$87,$fe,$ff,$ff,$07); // [A-Za-z0-9@*_+\-./]
var s,ss:TBESENString;
    i:integer;
    c:word;
    HexUppercase:boolean;
begin
 if CountArguments=0 then begin
  ResultValue:=BESENStringValue('undefined');
 end else begin
  HexUppercase:=(TBESEN(Instance).Compatibility and COMPAT_JS)<>0;
  s:=TBESEN(Instance).ToStr(Arguments^[0]^);
  ss:='';
  for i:=1 to length(s) do begin
   c:=word(widechar(s[i]));
   if (c<$80) and ((NotToEscapeChars[c shr 3] and (1 shl (c and 7)))<>0) then begin
    ss:=ss+widechar(c);
   end else if c<$100 then begin
    ss:=ss+'%'+BESENHexChars[HexUppercase,(c shr 4) and $f]+BESENHexChars[HexUppercase,c and $f];
   end else begin
    ss:=ss+'%u'+BESENHexChars[HexUppercase,(c shr 12) and $f]+BESENHexChars[HexUppercase,(c shr 8) and $f]+BESENHexChars[HexUppercase,(c shr 4) and $f]+BESENHexChars[HexUppercase,c and $f];
   end;
  end;
  ResultValue:=BESENStringValue(ss);
 end;
end;

procedure TBESENObjectGlobal.NativeUnescape(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
var s,ss:TBESENString;
    i:integer;
    c:word;
begin
 if CountArguments=0 then begin
  ResultValue:=BESENStringValue('undefined');
 end else begin
  s:=TBESEN(Instance).ToStr(Arguments^[0]^);
  ss:='';
  i:=1;
  while i<=length(s) do begin                        
   c:=word(widechar(s[i]));
   if c=ord('%') then begin
    inc(i);
    if ((i+4)<=length(s)) and (s[i]='u') then begin
     ss:=ss+widechar(word((BESENHexValues[word(widechar(s[i+1]))] shl 12) or (BESENHexValues[word(widechar(s[i+2]))] shl 8) or (BESENHexValues[word(widechar(s[i+3]))] shl 4) or BESENHexValues[word(widechar(s[i+4]))]));
     inc(i,5);
    end else if (i+1)<=length(s) then begin
     ss:=ss+widechar(word((BESENHexValues[word(widechar(s[i]))] shl 4) or BESENHexValues[word(widechar(s[i+1]))]));
     inc(i,2);
    end else begin
     ss:=ss+widechar(c);
     inc(i);
    end;
   end else begin
    ss:=ss+widechar(c);
    inc(i);
   end;
  end;
  ResultValue:=BESENStringValue(ss);
 end;
end;

procedure TBESENObjectGlobal.NativeCompatability(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
 procedure ParseCompatibility(s:TBESENString);
 var No:boolean;
     i:integer;
 begin
  s:=BESENLowercase(s);
  if copy(s,1,3)='no_' then begin
   System.delete(s,1,3);
   No:=true;
  end else if copy(s,1,4)='not_' then begin
   System.delete(s,1,4);
   No:=true;
  end else if copy(s,1,1)='-' then begin
   System.delete(s,1,1);
   No:=true;
  end else if copy(s,1,1)='!' then begin
   System.delete(s,1,1);
   No:=true;
  end else if copy(s,1,1)='~' then begin
   System.delete(s,1,1);
   No:=true;
  end else begin
   No:=false;
  end;
  for i:=low(TBESENCompatibilityModes) to high(TBESENCompatibilityModes) do begin
   if s=BESENCompatibilityModes[i].Name then begin
    if No then begin
     TBESEN(Instance).Compatibility:=TBESEN(Instance).Compatibility and not BESENCompatibilityModes[i].Flag;
    end else begin
     TBESEN(Instance).Compatibility:=TBESEN(Instance).Compatibility or BESENCompatibilityModes[i].Flag;
    end;
    break;
   end;
  end;
 end;
var i:integer;
    s,ps:TBESENString;
begin
 s:='';
 for i:=0 to CountArguments-1 do begin
  if length(s)>0 then begin
   s:=s+' ';
  end;
  s:=TBESEN(Instance).ToStr(Arguments^[i]^);
 end;
 ps:='';
 for i:=1 to length(s) do begin
  if BESENUnicodeIsStringWhiteSpace(word(widechar(s[i]))) then begin
   if length(ps)>0 then begin
    ParseCompatibility(ps);
    ps:='';
   end;
  end else begin
   ps:=ps+s[i];
  end;
 end;
 if length(ps)>0 then begin
  ParseCompatibility(ps);
  ps:='';
 end;
 s:='';
 for i:=low(TBESENCompatibilityModes) to high(TBESENCompatibilityModes) do begin
  if (TBESEN(Instance).Compatibility and BESENCompatibilityModes[i].Flag)<>0 then begin
   if length(s)>0 then begin
    s:=s+' ';
   end;
   s:=s+BESENCompatibilityModes[i].Name;
  end;
 end;
 ResultValue.ValueType:=bvtSTRING;
 ResultValue.Str:=s;
end;

end.
 