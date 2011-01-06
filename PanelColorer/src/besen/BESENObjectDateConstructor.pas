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
unit BESENObjectDateConstructor;
{$i BESEN.inc}

interface

uses SysUtils,BESENConstants,BESENTypes,BESENObject,BESENObjectFunction,BESENValue,BESENObjectPropertyDescriptor;

type TBESENObjectDateConstructor=class(TBESENObjectFunction)
      public
       constructor Create(AInstance:TObject;APrototype:TBESENObject=nil;AHasPrototypeProperty:longbool=false); overload; override;
       destructor Destroy; override;
       procedure Construct(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var AResult:TBESENValue); override;
       procedure Call(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var AResult:TBESENValue); override;
       function HasConstruct:TBESENBoolean; override;
       function HasCall:TBESENBoolean; override;
       procedure NativeNow(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeParse(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeUTC(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
     end;
   
implementation

uses BESEN,BESENErrors,BESENASTNodes,BESENStringUtils,BESENUtils,BESENObjectDate,BESENDateUtils,BESENNumberUtils,BESENLocale;

constructor TBESENObjectDateConstructor.Create(AInstance:TObject;APrototype:TBESENObject=nil;AHasPrototypeProperty:longbool=false);
begin
 inherited Create(AInstance,APrototype,AHasPrototypeProperty);
 ObjectClassName:='Function';
 ObjectName:='Date';

 RegisterNativeFunction('now',NativeNow,0,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('parse',NativeParse,1,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('UTC',NativeUTC,7,[bopaWRITABLE,bopaCONFIGURABLE],false);
end;

destructor TBESENObjectDateConstructor.Destroy;
begin
 inherited Destroy;
end;

procedure TBESENObjectDateConstructor.Construct(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var AResult:TBESENValue);
var r1:TBESENObjectDate;
    r3:TBESENValue;
    s:TBESENString;
    year,month,date,hours,minutes,seconds,ms:TBESENNumber;
begin
 r3:=BESENEmptyValue;
 r1:=TBESENObjectDate.Create(Instance,TBESEN(Instance).ObjectDatePrototype,false);
 TBESEN(Instance).GarbageCollector.Add(r1);
 r1.GarbageCollectorLock;
 try
  if CountArguments=0 then begin
   r1.Value:=BESENTimeClip(BESENDateTimeToBESENDate(BESENGetUTCDateTime));
  end else if CountArguments=1 then begin
   TBESEN(Instance).ToPrimitiveValue(Arguments^[0]^,TBESEN(Instance).ObjectNumberConstructorValue,r3);
   if r3.ValueType<>bvtSTRING then begin
    r1.Value:=BESENTimeClip(TBESEN(Instance).ToNum(r3));
   end else begin
    s:=TBESEN(Instance).ToStr(r3);
    r1.Value:=BESENParseTime(s);
    if BESENIsNaN(r1.Value) then begin
     r1.Value:=BESENParseISOTime(s);
     if BESENIsNaN(r1.Value) then begin
      r1.Value:=BESENParseNetscapeTime(s);
      if BESENIsNaN(r1.Value) then begin
       try
        r1.Value:=BESENTimeClip(BESENDateTimeToBESENDate(BESENLocalDateTimeToUTC(StrToDateTime(s{$if ((FPC_VERSION>=3) or ((FPC_VERSION>=2) and ((FPC_RELEASE>=5) or ((FPC_RELEASE>=4) and (FPC_PATCH>=1)))))},BESENLocaleFormatSettings{$ifend}))));
       except
        r1.Value:=double(pointer(@BESENDoubleNaN)^);
       end;
      end;
     end;
    end;
   end;
  end else if CountArguments>1 then begin
   year:=TBESEN(Instance).ToNum(Arguments^[0]^);
   month:=TBESEN(Instance).ToNum(Arguments^[1]^);
   date:=1;
   hours:=0;
   minutes:=0;
   seconds:=0;
   ms:=0;
   if CountArguments>2 then begin
    date:=TBESEN(Instance).ToNum(Arguments^[2]^);
    if CountArguments>3 then begin
     hours:=TBESEN(Instance).ToNum(Arguments^[3]^);
     if CountArguments>4 then begin
      minutes:=TBESEN(Instance).ToNum(Arguments^[4]^);
      if CountArguments>5 then begin
       seconds:=TBESEN(Instance).ToNum(Arguments^[5]^);
       if CountArguments>6 then begin
        ms:=TBESEN(Instance).ToNum(Arguments^[6]^);
       end;
      end;
     end;
    end;
   end;
   r1.Value:=BESENTimeClip(BESENMakeDate(BESENMakeDay(Year,Month,Date),BESENMakeTime(Hours,Minutes,Seconds,ms)));
  end;
 finally
  r1.GarbageCollectorUnlock;
 end;
 AResult:=BESENObjectValue(r1);
end;

procedure TBESENObjectDateConstructor.Call(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var AResult:TBESENValue);
var Value:TBESENDate;
begin
 Value:=BESENTimeClip(BESENGetUTCBESENDate);
 if BESENIsNaN(Value) then begin
  if (TBESEN(Instance).Compatibility and COMPAT_JS)<>0 then begin
   AResult:=BESENStringValue('Invalid Date');
  end else begin
   AResult:=BESENStringValue('NaN');
  end;
 end else begin
  AResult:=BESENStringLocaleCharsetValue(BESENFormatDateTime('ddd mmm dd yyyy hh:nn:ss',BESENUTCToLocalDateTime(BESENDateToDateTime(Value)),BESENDefaultFormatSettings)+' GMT'+BESENGetDateTimeOffsetString(BESENGetLocalDateTimeZone));
 end;
end;

procedure TBESENObjectDateConstructor.NativeNow(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 ResultValue:=BESENNumberValue(BESENGetUTCBESENDate);
end;

procedure TBESENObjectDateConstructor.NativeParse(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
var v:TBESENNumber;
    s:TBESENString;
begin
 try
  if CountArguments>0 then begin
   s:=TBESEN(Instance).ToStr(Arguments^[0]^);
   v:=BESENParseTime(s);
   if BESENIsNaN(v) then begin
    v:=BESENParseISOTime(s);
    if BESENIsNaN(v) then begin
     v:=BESENParseNetscapeTime(s);
     if BESENIsNaN(v) then begin
      try
       v:=BESENTimeClip(BESENDateTimeToBESENDate(BESENLocalDateTimeToUTC(StrToDateTime(s{$if ((FPC_VERSION>=3) or ((FPC_VERSION>=2) and ((FPC_RELEASE>=5) or ((FPC_RELEASE>=4) and (FPC_PATCH>=1)))))},BESENLocaleFormatSettings{$ifend}))));
      except
       v:=double(pointer(@BESENDoubleNaN)^);
      end;
     end;
    end;
   end;
   ResultValue:=BESENNumberValue(v);
  end else begin
   ResultValue:=BESENNumberValue(double(pointer(@BESENDoubleNaN)^));
  end;
 finally
 end;
end;

procedure TBESENObjectDateConstructor.NativeUTC(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
var year,month,date,hours,minutes,seconds,ms:TBESENNumber;
begin
 try
  if CountArguments>1 then begin
   year:=TBESEN(Instance).ToNum(Arguments^[0]^);
   month:=TBESEN(Instance).ToNum(Arguments^[1]^);
   date:=1;
   hours:=0;
   minutes:=0;
   seconds:=0;
   ms:=0;
   if CountArguments>2 then begin
    date:=TBESEN(Instance).ToNum(Arguments^[2]^);
    if CountArguments>3 then begin
     hours:=TBESEN(Instance).ToNum(Arguments^[3]^);
     if CountArguments>4 then begin
      minutes:=TBESEN(Instance).ToNum(Arguments^[4]^);
      if CountArguments>5 then begin
       seconds:=TBESEN(Instance).ToNum(Arguments^[5]^);
       if CountArguments>6 then begin
        ms:=TBESEN(Instance).ToNum(Arguments^[6]^);
       end;
      end;
     end;
    end;
   end;
   ResultValue:=BESENNumberValue(BESENTimeClip(BESENMakeDate(BESENMakeDay(Year,Month,Date),BESENMakeTime(Hours,Minutes,Seconds,ms))));
  end else begin
   ResultValue:=BESENNumberValue(double(pointer(@BESENDoubleNaN)^));
  end;
 finally
 end;
end;

function TBESENObjectDateConstructor.HasConstruct:TBESENBoolean;
begin
 result:=true;
end;

function TBESENObjectDateConstructor.HasCall:TBESENBoolean;
begin
 result:=true;
end;

end.
 