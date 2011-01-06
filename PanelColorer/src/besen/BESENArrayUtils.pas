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
unit BESENArrayUtils;
{$i BESEN.inc}

interface

uses SysUtils,BESENConstants,BESENTypes,BESENValue;

function BESENArrayToIndex(AProp:TBESENString;var v:int64):TBESENBoolean;
function BESENArrayToIndexEx(AProp:TBESENString):int64;
function BESENArrayIndexToStr(v:TBESENUINT32):TBESENString;
procedure BESENArrayCheckTooLong(const a,b:TBESENUINT32);

implementation

uses BESEN,BESENUtils,BESENErrors;

function BESENArrayToIndex(AProp:TBESENString;var v:int64):TBESENBoolean;
var i:integer;
begin
 if length(AProp)=0 then begin
  result:=false;
 end else if (length(AProp)>1) and (AProp[1]='0') then begin
  result:=false;
 end else begin
  result:=true;
  v:=0;
  for i:=1 to length(AProp) do begin
   if (word(widechar(AProp[i]))>=ord('0')) and (word(widechar(AProp[i]))<=ord('9')) then begin
    v:=(v*10)+(word(widechar(AProp[i]))-ord('0'));
   end else begin
    result:=false;
    break;
   end;
  end;
  if result and ((IntToStr(v and $ffffffff)<>AProp) or (v>=$ffffffff)) then begin
   result:=false;
  end;
 end;
end;

function BESENArrayToIndexEx(AProp:TBESENString):int64;
var i:integer;
begin
 if length(AProp)=0 then begin
  result:=0;
 end else if (length(AProp)>1) and (AProp[1]='0') then begin
  result:=0;
 end else begin
  result:=0;
  for i:=1 to length(AProp) do begin
   if (word(widechar(AProp[i]))>=ord('0')) and (word(widechar(AProp[i]))<=ord('9')) then begin
    result:=(result*10)+(word(widechar(AProp[i]))-ord('0'));
   end else begin
    result:=0;
    break;
   end;
  end;
 end;
end;

function BESENArrayIndexToStr(v:TBESENUINT32):TBESENString;
var i:integer;
    o:TBESENUINT32;
begin
 if v=0 then begin
  result:='0';
 end else begin
  result:='';
  i:=0;
  o:=v;
  while o<>0 do begin
   inc(i);
   o:=o div 10;
  end;
  SetLength(result,i);
  while (v<>0) and (i>0) do begin
   result[i]:=widechar(word(v mod 10)+ord('0'));
   dec(i);
   v:=v div 10;
  end;
 end;
end;

procedure BESENArrayCheckTooLong(const a,b:TBESENUINT32);
var c:TBESENUINT32;
begin
 c:=TBESENUINT32(a+b) and $ffffffff;
 if (c<a) or (c<b) then begin
  BESENThrowRangeError('Array too long');
 end;
end;

end.
