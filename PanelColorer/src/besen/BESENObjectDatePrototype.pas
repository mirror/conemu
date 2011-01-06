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
unit BESENObjectDatePrototype;
{$i BESEN.inc}

interface

uses Math,BESENConstants,BESENTypes,BESENObject,BESENObjectDate,BESENValue,BESENObjectPropertyDescriptor,
     BESENRegExp;

type TBESENObjectDatePrototype=class(TBESENObjectDate)
      public
       constructor Create(AInstance:TObject;APrototype:TBESENObject=nil;AHasPrototypeProperty:longbool=false); overload; override;
       destructor Destroy; override;
       procedure NativeToString(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeToDateString(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeToTimeString(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeToLocaleString(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeToLocaleDateString(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeToLocaleTimeString(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeToUTCString(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeValueOf(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeGetTime(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeGetFullYear(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeGetUTCFullYear(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeGetMonth(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeGetUTCMonth(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeGetDate(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeGetUTCDate(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeGetDay(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeGetUTCDay(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeGetHours(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeGetUTCHours(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeGetMinutes(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeGetUTCMinutes(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeGetSeconds(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeGetUTCSeconds(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeGetMilliseconds(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeGetUTCMilliseconds(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeGetTimezoneOffset(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeSetTime(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeSetMilliseconds(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeSetUTCMilliseconds(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeSetSeconds(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeSetUTCSeconds(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeSetMinutes(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeSetUTCMinutes(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeSetHours(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeSetUTCHours(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeSetDate(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeSetUTCDate(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeSetMonth(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeSetUTCMonth(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeSetFullYear(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeSetUTCFullYear(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeToGMTString(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeToISOString(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeToJSON(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeGetYear(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
       procedure NativeSetYear(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
    end;

implementation

uses BESEN,BESENErrors,BESENNumberUtils,BESENArrayUtils,BESENDateUtils,BESENLocale;

constructor TBESENObjectDatePrototype.Create(AInstance:TObject;APrototype:TBESENObject=nil;AHasPrototypeProperty:longbool=false);
begin
 inherited Create(AInstance,APrototype,AHasPrototypeProperty);
 ObjectClassName:='Date';
 ObjectName:='Date';

 if (TBESEN(Instance).Compatibility and COMPAT_JS)<>0 then begin
  OverwriteData('name',BESENStringValue(ObjectName),[]);
 end;
 
 RegisterNativeFunction('toString',NativeToString,0,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('toDateString',NativeToDateString,0,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('toTimeString',NativeToTimeString,0,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('toLocaleString',NativeToLocaleString,0,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('toLocaleDateString',NativeToLocaleDateString,0,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('toLocaleTimeString',NativeToLocaleTimeString,0,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('toUTCString',NativeToUTCString,0,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('valueOf',NativeValueOf,0,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('getTime',NativeGetTime,0,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('getFullYear',NativeGetFullYear,0,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('getUTCFullYear',NativeGetUTCFullYear,0,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('getMonth',NativeGetMonth,0,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('getUTCMonth',NativeGetUTCMonth,0,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('getDate',NativeGetDate,0,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('getUTCDate',NativeGetUTCDate,0,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('getDay',NativeGetDay,0,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('getUTCDay',NativeGetUTCDay,0,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('getHours',NativeGetHours,0,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('getUTCHours',NativeGetUTCHours,0,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('getMinutes',NativeGetMinutes,0,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('getUTCMinutes',NativeGetUTCMinutes,0,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('getSeconds',NativeGetSeconds,0,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('getUTCSeconds',NativeGetUTCSeconds,0,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('getMilliseconds',NativeGetMilliseconds,0,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('getUTCMilliseconds',NativeGetUTCMilliseconds,0,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('getTimezoneOffset',NativeGetTimezoneOffset,0,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('setTime',NativeSetTime,0,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('setMilliseconds',NativeSetMilliseconds,0,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('setUTCMilliseconds',NativeSetUTCMilliseconds,0,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('setSeconds',NativeSetSeconds,0,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('setUTCSeconds',NativeSetUTCSeconds,0,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('setMinutes',NativeSetMinutes,0,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('setUTCMinutes',NativeSetUTCMinutes,0,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('setHours',NativeSetHours,0,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('setUTCHours',NativeSetUTCHours,0,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('setDate',NativeSetDate,0,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('setUTCDate',NativeSetUTCDate,0,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('setMonth',NativeSetMonth,0,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('setUTCMonth',NativeSetUTCMonth,0,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('setFullYear',NativeSetFullYear,0,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('setUTCFullYear',NativeSetUTCFullYear,0,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('toGMTString',NativeToGMTString,0,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('toISOString',NativeToISOString,0,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('toJSON',NativeToJSON,1,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('getYear',NativeGetYear,0,[bopaWRITABLE,bopaCONFIGURABLE],false);
 RegisterNativeFunction('setYear',NativeSetYear,0,[bopaWRITABLE,bopaCONFIGURABLE],false);
end;

destructor TBESENObjectDatePrototype.Destroy;
begin
 inherited Destroy;
end;

procedure TBESENObjectDatePrototype.NativeToString(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 if not ((ThisArgument.ValueType=bvtOBJECT) and assigned(TBESENObject(ThisArgument.Obj))) then begin
  raise EBESENTypeError.Create('Null this object');
 end;
 if assigned(TBESENObject(ThisArgument.Obj)) and (TBESENObject(ThisArgument.Obj) is TBESENObjectDate) then begin
  if BESENIsNaN(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value) then begin
   if (TBESEN(Instance).Compatibility and COMPAT_JS)<>0 then begin
    ResultValue:=BESENStringValue('Invalid Date');
   end else begin
    ResultValue:=BESENStringValue('NaN');
   end;
  end else begin
   ResultValue:=BESENStringLocaleCharsetValue(BESENFormatDateTime('ddd mmm dd yyyy hh:nn:ss',BESENUTCToLocalDateTime(BESENDateToDateTime(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value)),BESENDefaultFormatSettings)+' GMT'+BESENGetDateTimeOffsetString(BESENGetLocalDateTimeZone));
  end;
 end else begin
  raise EBESENTypeError.Create('Not date object');
 end;
end;

procedure TBESENObjectDatePrototype.NativeToDateString(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 if not ((ThisArgument.ValueType=bvtOBJECT) and assigned(TBESENObject(ThisArgument.Obj))) then begin
  raise EBESENTypeError.Create('Null this object');
 end;
 if assigned(TBESENObject(ThisArgument.Obj)) and (TBESENObject(ThisArgument.Obj) is TBESENObjectDate) then begin
  if BESENIsNaN(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value) then begin
   if (TBESEN(Instance).Compatibility and COMPAT_JS)<>0 then begin
    ResultValue:=BESENStringValue('Invalid Date');
   end else begin
    ResultValue:=BESENStringValue('NaN');
   end;
  end else begin
   ResultValue:=BESENStringLocaleCharsetValue(BESENFormatDateTime('ddd mmm dd yyy',BESENUTCToLocalDateTime(BESENDateToDateTime(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value)),BESENDefaultFormatSettings));
  end;
 end else begin
  raise EBESENTypeError.Create('Not date object');
 end;
end;

procedure TBESENObjectDatePrototype.NativeToTimeString(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 if not ((ThisArgument.ValueType=bvtOBJECT) and assigned(TBESENObject(ThisArgument.Obj))) then begin
  raise EBESENTypeError.Create('Null this object');
 end;
 if assigned(TBESENObject(ThisArgument.Obj)) and (TBESENObject(ThisArgument.Obj) is TBESENObjectDate) then begin
  if BESENIsNaN(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value) then begin
   if (TBESEN(Instance).Compatibility and COMPAT_JS)<>0 then begin
    ResultValue:=BESENStringValue('Invalid Date');
   end else begin
    ResultValue:=BESENStringValue('NaN');
   end;
  end else begin
   ResultValue:=BESENStringLocaleCharsetValue(BESENFormatDateTime('hh:nn:ss',BESENUTCToLocalDateTime(BESENDateToDateTime(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value)),BESENDefaultFormatSettings));
  end;
 end else begin
  raise EBESENTypeError.Create('Not date object');
 end;
end;

procedure TBESENObjectDatePrototype.NativeToLocaleString(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 if not ((ThisArgument.ValueType=bvtOBJECT) and assigned(TBESENObject(ThisArgument.Obj))) then begin
  raise EBESENTypeError.Create('Null this object');
 end;
 if assigned(TBESENObject(ThisArgument.Obj)) and (TBESENObject(ThisArgument.Obj) is TBESENObjectDate) then begin
  if BESENIsNaN(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value) then begin
   if (TBESEN(Instance).Compatibility and COMPAT_JS)<>0 then begin
    ResultValue:=BESENStringValue('Invalid Date');
   end else begin
    ResultValue:=BESENStringValue('NaN');
   end;
  end else begin
   ResultValue:=BESENStringLocaleCharsetValue(BESENFormatDateTime('dddddd tt',BESENUTCToLocalDateTime(BESENDateToDateTime(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value)),BESENLocaleFormatSettings));
  end;
 end else begin
  raise EBESENTypeError.Create('Not date object');
 end;
end;

procedure TBESENObjectDatePrototype.NativeToLocaleDateString(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 if not ((ThisArgument.ValueType=bvtOBJECT) and assigned(TBESENObject(ThisArgument.Obj))) then begin
  raise EBESENTypeError.Create('Null this object');
 end;
 if assigned(TBESENObject(ThisArgument.Obj)) and (TBESENObject(ThisArgument.Obj) is TBESENObjectDate) then begin
  if BESENIsNaN(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value) then begin
   if (TBESEN(Instance).Compatibility and COMPAT_JS)<>0 then begin
    ResultValue:=BESENStringValue('Invalid Date');
   end else begin
    ResultValue:=BESENStringValue('NaN');
   end;
  end else begin
   ResultValue:=BESENStringLocaleCharsetValue(BESENFormatDateTime('dddddd',BESENUTCToLocalDateTime(BESENDateToDateTime(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value)),BESENLocaleFormatSettings));
  end;
 end else begin
  raise EBESENTypeError.Create('Not date object');
 end;
end;

procedure TBESENObjectDatePrototype.NativeToLocaleTimeString(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 if not ((ThisArgument.ValueType=bvtOBJECT) and assigned(TBESENObject(ThisArgument.Obj))) then begin
  raise EBESENTypeError.Create('Null this object');
 end;
 if assigned(TBESENObject(ThisArgument.Obj)) and (TBESENObject(ThisArgument.Obj) is TBESENObjectDate) then begin
  if BESENIsNaN(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value) then begin
   if (TBESEN(Instance).Compatibility and COMPAT_JS)<>0 then begin
    ResultValue:=BESENStringValue('Invalid Date');
   end else begin
    ResultValue:=BESENStringValue('NaN');
   end;
  end else begin
   ResultValue:=BESENStringLocaleCharsetValue(BESENFormatDateTime('tt',BESENUTCToLocalDateTime(BESENDateToDateTime(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value)),BESENLocaleFormatSettings));
  end;
 end else begin
  raise EBESENTypeError.Create('Not date object');
 end;
end;

procedure TBESENObjectDatePrototype.NativeToUTCString(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 if not ((ThisArgument.ValueType=bvtOBJECT) and assigned(TBESENObject(ThisArgument.Obj))) then begin
  raise EBESENTypeError.Create('Null this object');
 end;
 if assigned(TBESENObject(ThisArgument.Obj)) and (TBESENObject(ThisArgument.Obj) is TBESENObjectDate) then begin
  if BESENIsNaN(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value) then begin
   if (TBESEN(Instance).Compatibility and COMPAT_JS)<>0 then begin
    ResultValue:=BESENStringValue('Invalid Date');
   end else begin
    ResultValue:=BESENStringValue('NaN');
   end;
  end else begin
   ResultValue:=BESENStringLocaleCharsetValue(BESENFormatDateTime('ddd, dd mmm yyyy hh:nn:ss',BESENDateToDateTime(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value),BESENDefaultFormatSettings)+' GMT');
  end;
 end else begin
  raise EBESENTypeError.Create('Not date object');
 end;

end;

procedure TBESENObjectDatePrototype.NativeValueOf(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 if not ((ThisArgument.ValueType=bvtOBJECT) and assigned(TBESENObject(ThisArgument.Obj))) then begin
  raise EBESENTypeError.Create('Null this object');
 end;
 if assigned(TBESENObject(ThisArgument.Obj)) and (TBESENObject(ThisArgument.Obj) is TBESENObjectDate) then begin
  ResultValue:=BESENNumberValue(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value);
 end else begin
  raise EBESENTypeError.Create('Not a date object');
 end;
end;

procedure TBESENObjectDatePrototype.NativeGetTime(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 if not ((ThisArgument.ValueType=bvtOBJECT) and assigned(TBESENObject(ThisArgument.Obj))) then begin
  raise EBESENTypeError.Create('Null this object');
 end;
 if assigned(TBESENObject(ThisArgument.Obj)) and (TBESENObject(ThisArgument.Obj) is TBESENObjectDate) then begin
  ResultValue:=BESENNumberValue(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value);
 end else begin
  raise EBESENTypeError.Create('Not a date object');
 end;
end;

procedure TBESENObjectDatePrototype.NativeGetFullYear(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 if not ((ThisArgument.ValueType=bvtOBJECT) and assigned(TBESENObject(ThisArgument.Obj))) then begin
  raise EBESENTypeError.Create('Null this object');
 end;
 if assigned(TBESENObject(ThisArgument.Obj)) and (TBESENObject(ThisArgument.Obj) is TBESENObjectDate) then begin
  if BESENIsNaN(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value) then begin
   ResultValue:=BESENNumberValue(double(pointer(@BESENDoubleNaN)^));
  end else begin
   ResultValue:=BESENNumberValue(BESENYearFromTime(BESENLocalTime(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value)));
  end;
 end else begin
  raise EBESENTypeError.Create('Not a date object');
 end;
end;

procedure TBESENObjectDatePrototype.NativeGetUTCFullYear(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 if not ((ThisArgument.ValueType=bvtOBJECT) and assigned(TBESENObject(ThisArgument.Obj))) then begin
  raise EBESENTypeError.Create('Null this object');
 end;
 if assigned(TBESENObject(ThisArgument.Obj)) and (TBESENObject(ThisArgument.Obj) is TBESENObjectDate) then begin
  if BESENIsNaN(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value) then begin
   ResultValue:=BESENNumberValue(double(pointer(@BESENDoubleNaN)^));
  end else begin
   ResultValue:=BESENNumberValue(BESENYearFromTime(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value));
  end;
 end else begin
  raise EBESENTypeError.Create('Not a date object');
 end;
end;

procedure TBESENObjectDatePrototype.NativeGetMonth(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 if not ((ThisArgument.ValueType=bvtOBJECT) and assigned(TBESENObject(ThisArgument.Obj))) then begin
  raise EBESENTypeError.Create('Null this object');
 end;
 if assigned(TBESENObject(ThisArgument.Obj)) and (TBESENObject(ThisArgument.Obj) is TBESENObjectDate) then begin
  if BESENIsNaN(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value) then begin
   ResultValue:=BESENNumberValue(double(pointer(@BESENDoubleNaN)^));
  end else begin
   ResultValue:=BESENNumberValue(BESENMonthFromTime(BESENLocalTime(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value)));
  end;
 end else begin
  raise EBESENTypeError.Create('Not a date object');
 end;
end;

procedure TBESENObjectDatePrototype.NativeGetUTCMonth(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 if not ((ThisArgument.ValueType=bvtOBJECT) and assigned(TBESENObject(ThisArgument.Obj))) then begin
  raise EBESENTypeError.Create('Null this object');
 end;
 if assigned(TBESENObject(ThisArgument.Obj)) and (TBESENObject(ThisArgument.Obj) is TBESENObjectDate) then begin
  if BESENIsNaN(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value) then begin
   ResultValue:=BESENNumberValue(double(pointer(@BESENDoubleNaN)^));
  end else begin
   ResultValue:=BESENNumberValue(BESENMonthFromTime(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value));
  end;
 end else begin
  raise EBESENTypeError.Create('Not a date object');
 end;
end;

procedure TBESENObjectDatePrototype.NativeGetDate(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 if not ((ThisArgument.ValueType=bvtOBJECT) and assigned(TBESENObject(ThisArgument.Obj))) then begin
  raise EBESENTypeError.Create('Null this object');
 end;
 if assigned(TBESENObject(ThisArgument.Obj)) and (TBESENObject(ThisArgument.Obj) is TBESENObjectDate) then begin
  if BESENIsNaN(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value) then begin
   ResultValue:=BESENNumberValue(double(pointer(@BESENDoubleNaN)^));
  end else begin
   ResultValue:=BESENNumberValue(BESENDayFromTime(BESENLocalTime(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value)));
  end;
 end else begin
  raise EBESENTypeError.Create('Not a date object');
 end;
end;

procedure TBESENObjectDatePrototype.NativeGetUTCDate(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 if not ((ThisArgument.ValueType=bvtOBJECT) and assigned(TBESENObject(ThisArgument.Obj))) then begin
  raise EBESENTypeError.Create('Null this object');
 end;
 if assigned(TBESENObject(ThisArgument.Obj)) and (TBESENObject(ThisArgument.Obj) is TBESENObjectDate) then begin
  if BESENIsNaN(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value) then begin
   ResultValue:=BESENNumberValue(double(pointer(@BESENDoubleNaN)^));
  end else begin
   ResultValue:=BESENNumberValue(BESENDayFromTime(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value));
  end;
 end else begin
  raise EBESENTypeError.Create('Not a date object');
 end;
end;

procedure TBESENObjectDatePrototype.NativeGetDay(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 if not ((ThisArgument.ValueType=bvtOBJECT) and assigned(TBESENObject(ThisArgument.Obj))) then begin
  raise EBESENTypeError.Create('Null this object');
 end;
 if assigned(TBESENObject(ThisArgument.Obj)) and (TBESENObject(ThisArgument.Obj) is TBESENObjectDate) then begin
  if BESENIsNaN(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value) then begin
   ResultValue:=BESENNumberValue(double(pointer(@BESENDoubleNaN)^));
  end else begin
   ResultValue:=BESENNumberValue(BESENWeekDay(BESENLocalTime(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value)));
  end;
 end else begin
  raise EBESENTypeError.Create('Not a date object');
 end;
end;

procedure TBESENObjectDatePrototype.NativeGetUTCDay(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 if not ((ThisArgument.ValueType=bvtOBJECT) and assigned(TBESENObject(ThisArgument.Obj))) then begin
  raise EBESENTypeError.Create('Null this object');
 end;
 if assigned(TBESENObject(ThisArgument.Obj)) and (TBESENObject(ThisArgument.Obj) is TBESENObjectDate) then begin
  if BESENIsNaN(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value) then begin
   ResultValue:=BESENNumberValue(double(pointer(@BESENDoubleNaN)^));
  end else begin
   ResultValue:=BESENNumberValue(BESENWeekDay(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value));
  end;
 end else begin
  raise EBESENTypeError.Create('Not a date object');
 end;
end;

procedure TBESENObjectDatePrototype.NativeGetHours(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 if not ((ThisArgument.ValueType=bvtOBJECT) and assigned(TBESENObject(ThisArgument.Obj))) then begin
  raise EBESENTypeError.Create('Null this object');
 end;
 if assigned(TBESENObject(ThisArgument.Obj)) and (TBESENObject(ThisArgument.Obj) is TBESENObjectDate) then begin
  if BESENIsNaN(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value) then begin
   ResultValue:=BESENNumberValue(double(pointer(@BESENDoubleNaN)^));
  end else begin
   ResultValue:=BESENNumberValue(BESENHourFromTime(BESENLocalTime(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value)));
  end;
 end else begin
  raise EBESENTypeError.Create('Not a date object');
 end;
end;

procedure TBESENObjectDatePrototype.NativeGetUTCHours(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 if not ((ThisArgument.ValueType=bvtOBJECT) and assigned(TBESENObject(ThisArgument.Obj))) then begin
  raise EBESENTypeError.Create('Null this object');
 end;
 if assigned(TBESENObject(ThisArgument.Obj)) and (TBESENObject(ThisArgument.Obj) is TBESENObjectDate) then begin
  if BESENIsNaN(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value) then begin
   ResultValue:=BESENNumberValue(double(pointer(@BESENDoubleNaN)^));
  end else begin
   ResultValue:=BESENNumberValue(BESENHourFromTime(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value));
  end;
 end else begin
  raise EBESENTypeError.Create('Not a date object');
 end;
end;

procedure TBESENObjectDatePrototype.NativeGetMinutes(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 if not ((ThisArgument.ValueType=bvtOBJECT) and assigned(TBESENObject(ThisArgument.Obj))) then begin
  raise EBESENTypeError.Create('Null this object');
 end;
 if assigned(TBESENObject(ThisArgument.Obj)) and (TBESENObject(ThisArgument.Obj) is TBESENObjectDate) then begin
  if BESENIsNaN(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value) then begin
   ResultValue:=BESENNumberValue(double(pointer(@BESENDoubleNaN)^));
  end else begin
   ResultValue:=BESENNumberValue(BESENMinuteFromTime(BESENLocalTime(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value)));
  end;
 end else begin
  raise EBESENTypeError.Create('Not a date object');
 end;
end;

procedure TBESENObjectDatePrototype.NativeGetUTCMinutes(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 if not ((ThisArgument.ValueType=bvtOBJECT) and assigned(TBESENObject(ThisArgument.Obj))) then begin
  raise EBESENTypeError.Create('Null this object');
 end;
 if assigned(TBESENObject(ThisArgument.Obj)) and (TBESENObject(ThisArgument.Obj) is TBESENObjectDate) then begin
  if BESENIsNaN(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value) then begin
   ResultValue:=BESENNumberValue(double(pointer(@BESENDoubleNaN)^));
  end else begin
   ResultValue:=BESENNumberValue(BESENMinuteFromTime(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value));
  end;
 end else begin
  raise EBESENTypeError.Create('Not a date object');
 end;
end;

procedure TBESENObjectDatePrototype.NativeGetSeconds(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 if not ((ThisArgument.ValueType=bvtOBJECT) and assigned(TBESENObject(ThisArgument.Obj))) then begin
  raise EBESENTypeError.Create('Null this object');
 end;
 if assigned(TBESENObject(ThisArgument.Obj)) and (TBESENObject(ThisArgument.Obj) is TBESENObjectDate) then begin
  if BESENIsNaN(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value) then begin
   ResultValue:=BESENNumberValue(double(pointer(@BESENDoubleNaN)^));
  end else begin
   ResultValue:=BESENNumberValue(BESENSecondFromTime(BESENLocalTime(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value)));
  end;
 end else begin
  raise EBESENTypeError.Create('Not a date object');
 end;
end;

procedure TBESENObjectDatePrototype.NativeGetUTCSeconds(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 if not ((ThisArgument.ValueType=bvtOBJECT) and assigned(TBESENObject(ThisArgument.Obj))) then begin
  raise EBESENTypeError.Create('Null this object');
 end;
 if assigned(TBESENObject(ThisArgument.Obj)) and (TBESENObject(ThisArgument.Obj) is TBESENObjectDate) then begin
  if BESENIsNaN(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value) then begin
   ResultValue:=BESENNumberValue(double(pointer(@BESENDoubleNaN)^));
  end else begin
   ResultValue:=BESENNumberValue(BESENSecondFromTime(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value));
  end;
 end else begin
  raise EBESENTypeError.Create('Not a date object');
 end;
end;

procedure TBESENObjectDatePrototype.NativeGetMilliseconds(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 if not ((ThisArgument.ValueType=bvtOBJECT) and assigned(TBESENObject(ThisArgument.Obj))) then begin
  raise EBESENTypeError.Create('Null this object');
 end;
 if assigned(TBESENObject(ThisArgument.Obj)) and (TBESENObject(ThisArgument.Obj) is TBESENObjectDate) then begin
  if BESENIsNaN(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value) then begin
   ResultValue:=BESENNumberValue(double(pointer(@BESENDoubleNaN)^));
  end else begin
   ResultValue:=BESENNumberValue(BESENMillisecondFromTime(BESENLocalTime(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value)));
  end;
 end else begin
  raise EBESENTypeError.Create('Not a date object');
 end;
end;

procedure TBESENObjectDatePrototype.NativeGetUTCMilliseconds(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 if not ((ThisArgument.ValueType=bvtOBJECT) and assigned(TBESENObject(ThisArgument.Obj))) then begin
  raise EBESENTypeError.Create('Null this object');
 end;
 if assigned(TBESENObject(ThisArgument.Obj)) and (TBESENObject(ThisArgument.Obj) is TBESENObjectDate) then begin
  if BESENIsNaN(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value) then begin
   ResultValue:=BESENNumberValue(double(pointer(@BESENDoubleNaN)^));
  end else begin
   ResultValue:=BESENNumberValue(BESENMillisecondFromTime(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value));
  end;
 end else begin
  raise EBESENTypeError.Create('Not a date object');
 end;
end;

procedure TBESENObjectDatePrototype.NativeGetTimezoneOffset(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 if not ((ThisArgument.ValueType=bvtOBJECT) and assigned(TBESENObject(ThisArgument.Obj))) then begin
  raise EBESENTypeError.Create('Null this object');
 end;
 if assigned(TBESENObject(ThisArgument.Obj)) and (TBESENObject(ThisArgument.Obj) is TBESENObjectDate) then begin
  if BESENIsNaN(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value) then begin
   ResultValue:=BESENNumberValue(double(pointer(@BESENDoubleNaN)^));
  end else begin
   ResultValue:=BESENNumberValue((TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value-BESENLocalTime(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value))/BESENmsPerMinute);
  end;
 end else begin
  raise EBESENTypeError.Create('Not a date object');
 end;
end;

procedure TBESENObjectDatePrototype.NativeSetTime(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 if not ((ThisArgument.ValueType=bvtOBJECT) and assigned(TBESENObject(ThisArgument.Obj))) then begin
  raise EBESENTypeError.Create('Null this object');
 end;
 if assigned(TBESENObject(ThisArgument.Obj)) and (TBESENObject(ThisArgument.Obj) is TBESENObjectDate) then begin
  if CountArguments=0 then begin
   TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value:=double(pointer(@BESENDoubleNaN)^);
  end else begin
   TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value:=BESENTimeClip(TBESEN(Instance).ToNum(Arguments^[0]^));
  end;
  ResultValue:=BESENNumberValue(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value);
 end else begin
  raise EBESENTypeError.Create('Not a date object');
 end;
end;

procedure TBESENObjectDatePrototype.NativeSetMilliseconds(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
var t:TBESENDate;
    ms:TBESENNumber;
begin
 if not ((ThisArgument.ValueType=bvtOBJECT) and assigned(TBESENObject(ThisArgument.Obj))) then begin
  raise EBESENTypeError.Create('Null this object');
 end;
 if assigned(TBESENObject(ThisArgument.Obj)) and (TBESENObject(ThisArgument.Obj) is TBESENObjectDate) then begin
  if CountArguments=0 then begin
   TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value:=double(pointer(@BESENDoubleNaN)^);
  end else begin
   t:=BESENLocalTime(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value);
   ms:=TBESEN(Instance).ToNum(Arguments^[0]^);
   TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value:=BESENTimeClip(BESENUTC(BESENMakeDate(BESENDay(t),BESENMakeTime(BESENHourFromTime(t),BESENMinuteFromTime(t),BESENSecondFromTime(t),ms))));
  end;
  ResultValue:=BESENNumberValue(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value);
 end else begin
  raise EBESENTypeError.Create('Not a date object');
 end;
end;

procedure TBESENObjectDatePrototype.NativeSetUTCMilliseconds(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
var t:TBESENDate;
    ms:TBESENNumber;
begin
 if not ((ThisArgument.ValueType=bvtOBJECT) and assigned(TBESENObject(ThisArgument.Obj))) then begin
  raise EBESENTypeError.Create('Null this object');
 end;
 if assigned(TBESENObject(ThisArgument.Obj)) and (TBESENObject(ThisArgument.Obj) is TBESENObjectDate) then begin
  if CountArguments=0 then begin
   TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value:=double(pointer(@BESENDoubleNaN)^);
  end else begin
   t:=TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value;
   ms:=TBESEN(Instance).ToNum(Arguments^[0]^);
   TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value:=BESENTimeClip(BESENMakeDate(BESENDay(t),BESENMakeTime(BESENHourFromTime(t),BESENMinuteFromTime(t),BESENSecondFromTime(t),ms)));
  end;
  ResultValue:=BESENNumberValue(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value);
 end else begin
  raise EBESENTypeError.Create('Not a date object');
 end;
end;

procedure TBESENObjectDatePrototype.NativeSetSeconds(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
var t:TBESENDate;
    Seconds,ms:TBESENNumber;
begin
 if not ((ThisArgument.ValueType=bvtOBJECT) and assigned(TBESENObject(ThisArgument.Obj))) then begin
  raise EBESENTypeError.Create('Null this object');
 end;
 if assigned(TBESENObject(ThisArgument.Obj)) and (TBESENObject(ThisArgument.Obj) is TBESENObjectDate) then begin
  if CountArguments=0 then begin
   TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value:=double(pointer(@BESENDoubleNaN)^);
  end else begin
   t:=BESENLocalTime(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value);
   Seconds:=TBESEN(Instance).ToNum(Arguments^[0]^);
   if CountArguments>1 then begin
    ms:=TBESEN(Instance).ToNum(Arguments^[1]^);
   end else begin
    ms:=BESENMillisecondFromTime(t);
   end;
   TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value:=BESENTimeClip(BESENUTC(BESENMakeDate(BESENDay(t),BESENMakeTime(BESENHourFromTime(t),BESENMinuteFromTime(t),Seconds,ms))));
  end;
  ResultValue:=BESENNumberValue(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value);
 end else begin
  raise EBESENTypeError.Create('Not a date object');
 end;
end;

procedure TBESENObjectDatePrototype.NativeSetUTCSeconds(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
var t:TBESENDate;
    Seconds,ms:TBESENNumber;
begin
 if not ((ThisArgument.ValueType=bvtOBJECT) and assigned(TBESENObject(ThisArgument.Obj))) then begin
  raise EBESENTypeError.Create('Null this object');
 end;
 if assigned(TBESENObject(ThisArgument.Obj)) and (TBESENObject(ThisArgument.Obj) is TBESENObjectDate) then begin
  if CountArguments=0 then begin
   TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value:=double(pointer(@BESENDoubleNaN)^);
  end else begin
   t:=TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value;
   Seconds:=TBESEN(Instance).ToNum(Arguments^[0]^);
   if CountArguments>1 then begin
    ms:=TBESEN(Instance).ToNum(Arguments^[1]^);
   end else begin
    ms:=BESENMillisecondFromTime(t);
   end;
   TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value:=BESENTimeClip(BESENMakeDate(BESENDay(t),BESENMakeTime(BESENHourFromTime(t),BESENMinuteFromTime(t),Seconds,ms)));
  end;
  ResultValue:=BESENNumberValue(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value);
 end else begin
  raise EBESENTypeError.Create('Not a date object');
 end;
end;

procedure TBESENObjectDatePrototype.NativeSetMinutes(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
var t:TBESENDate;
    Minutes,Seconds,ms:TBESENNumber;
begin
 if not ((ThisArgument.ValueType=bvtOBJECT) and assigned(TBESENObject(ThisArgument.Obj))) then begin
  raise EBESENTypeError.Create('Null this object');
 end;
 if assigned(TBESENObject(ThisArgument.Obj)) and (TBESENObject(ThisArgument.Obj) is TBESENObjectDate) then begin
  if CountArguments=0 then begin
   TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value:=double(pointer(@BESENDoubleNaN)^);
  end else begin
   t:=BESENLocalTime(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value);
   Minutes:=TBESEN(Instance).ToNum(Arguments^[0]^);
   if CountArguments>1 then begin
    Seconds:=TBESEN(Instance).ToNum(Arguments^[1]^);
   end else begin
    Seconds:=BESENSecondFromTime(t);
   end;
   if CountArguments>2 then begin
    ms:=TBESEN(Instance).ToNum(Arguments^[2]^);
   end else begin
    ms:=BESENMillisecondFromTime(t);
   end;
   TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value:=BESENTimeClip(BESENUTC(BESENMakeDate(BESENDay(t),BESENMakeTime(BESENHourFromTime(t),Minutes,Seconds,ms))));
  end;
  ResultValue:=BESENNumberValue(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value);
 end else begin
  raise EBESENTypeError.Create('Not a date object');
 end;
end;

procedure TBESENObjectDatePrototype.NativeSetUTCMinutes(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
var t:TBESENDate;
    Minutes,Seconds,ms:TBESENNumber;
begin
 if not ((ThisArgument.ValueType=bvtOBJECT) and assigned(TBESENObject(ThisArgument.Obj))) then begin
  raise EBESENTypeError.Create('Null this object');
 end;
 if assigned(TBESENObject(ThisArgument.Obj)) and (TBESENObject(ThisArgument.Obj) is TBESENObjectDate) then begin
  if CountArguments=0 then begin
   TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value:=double(pointer(@BESENDoubleNaN)^);
  end else begin
   t:=TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value;
   Minutes:=TBESEN(Instance).ToNum(Arguments^[0]^);
   if CountArguments>1 then begin
    Seconds:=TBESEN(Instance).ToNum(Arguments^[1]^);
   end else begin
    Seconds:=BESENSecondFromTime(t);
   end;
   if CountArguments>2 then begin
    ms:=TBESEN(Instance).ToNum(Arguments^[2]^);
   end else begin
    ms:=BESENMillisecondFromTime(t);
   end;
   TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value:=BESENTimeClip(BESENMakeDate(BESENDay(t),BESENMakeTime(BESENHourFromTime(t),Minutes,Seconds,ms)));
  end;
  ResultValue:=BESENNumberValue(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value);
 end else begin
  raise EBESENTypeError.Create('Not a date object');
 end;
end;

procedure TBESENObjectDatePrototype.NativeSetHours(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
var t:TBESENDate;
    Hours,Minutes,Seconds,ms:TBESENNumber;
begin
 if not ((ThisArgument.ValueType=bvtOBJECT) and assigned(TBESENObject(ThisArgument.Obj))) then begin
  raise EBESENTypeError.Create('Null this object');
 end;
 if assigned(TBESENObject(ThisArgument.Obj)) and (TBESENObject(ThisArgument.Obj) is TBESENObjectDate) then begin
  if CountArguments=0 then begin
   TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value:=double(pointer(@BESENDoubleNaN)^);
  end else begin
   t:=BESENLocalTime(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value);
   Hours:=TBESEN(Instance).ToNum(Arguments^[0]^);
   if CountArguments>1 then begin
    Minutes:=TBESEN(Instance).ToNum(Arguments^[1]^);
   end else begin
    Minutes:=BESENMinuteFromTime(t);
   end;
   if CountArguments>2 then begin
    Seconds:=TBESEN(Instance).ToNum(Arguments^[2]^);
   end else begin
    Seconds:=BESENSecondFromTime(t);
   end;
   if CountArguments>3 then begin
    ms:=TBESEN(Instance).ToNum(Arguments^[3]^);
   end else begin
    ms:=BESENMillisecondFromTime(t);
   end;
   TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value:=BESENTimeClip(BESENUTC(BESENMakeDate(BESENDay(t),BESENMakeTime(Hours,Minutes,Seconds,ms))));
  end;
  ResultValue:=BESENNumberValue(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value);
 end else begin
  raise EBESENTypeError.Create('Not a date object');
 end;
end;

procedure TBESENObjectDatePrototype.NativeSetUTCHours(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
var t:TBESENDate;
    Hours,Minutes,Seconds,ms:TBESENNumber;
begin
 if not ((ThisArgument.ValueType=bvtOBJECT) and assigned(TBESENObject(ThisArgument.Obj))) then begin
  raise EBESENTypeError.Create('Null this object');
 end;
 if assigned(TBESENObject(ThisArgument.Obj)) and (TBESENObject(ThisArgument.Obj) is TBESENObjectDate) then begin
  if CountArguments=0 then begin
   TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value:=double(pointer(@BESENDoubleNaN)^);
  end else begin
   t:=TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value;
   Hours:=TBESEN(Instance).ToNum(Arguments^[0]^);
   if CountArguments>1 then begin
    Minutes:=TBESEN(Instance).ToNum(Arguments^[1]^);
   end else begin
    Minutes:=BESENMinuteFromTime(t);
   end;
   if CountArguments>2 then begin
    Seconds:=TBESEN(Instance).ToNum(Arguments^[2]^);
   end else begin
    Seconds:=BESENSecondFromTime(t);
   end;
   if CountArguments>3 then begin
    ms:=TBESEN(Instance).ToNum(Arguments^[3]^);
   end else begin
    ms:=BESENMillisecondFromTime(t);
   end;
   TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value:=BESENTimeClip(BESENMakeDate(BESENDay(t),BESENMakeTime(Hours,Minutes,Seconds,ms)));
  end;
  ResultValue:=BESENNumberValue(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value);
 end else begin
  raise EBESENTypeError.Create('Not a date object');
 end;
end;

procedure TBESENObjectDatePrototype.NativeSetDate(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
var t:TBESENDate;
    Date:TBESENNumber;
begin
 if not ((ThisArgument.ValueType=bvtOBJECT) and assigned(TBESENObject(ThisArgument.Obj))) then begin
  raise EBESENTypeError.Create('Null this object');
 end;
 if assigned(TBESENObject(ThisArgument.Obj)) and (TBESENObject(ThisArgument.Obj) is TBESENObjectDate) then begin
  if CountArguments=0 then begin
   TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value:=double(pointer(@BESENDoubleNaN)^);
  end else begin
   t:=BESENLocalTime(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value);
   Date:=TBESEN(Instance).ToNum(Arguments^[0]^);
   TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value:=BESENTimeClip(BESENUTC(BESENMakeDate(BESENMakeDay(BESENYearFromTime(t),BESENMonthFromTime(t),Date),BESENTimeWithinDay(t))));
  end;
  ResultValue:=BESENNumberValue(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value);
 end else begin
  raise EBESENTypeError.Create('Not a date object');
 end;
end;

procedure TBESENObjectDatePrototype.NativeSetUTCDate(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
var t:TBESENDate;
    Date:TBESENNumber;
begin
 if not ((ThisArgument.ValueType=bvtOBJECT) and assigned(TBESENObject(ThisArgument.Obj))) then begin
  raise EBESENTypeError.Create('Null this object');
 end;
 if assigned(TBESENObject(ThisArgument.Obj)) and (TBESENObject(ThisArgument.Obj) is TBESENObjectDate) then begin
  if CountArguments=0 then begin
   TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value:=double(pointer(@BESENDoubleNaN)^);
  end else begin
   t:=TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value;
   Date:=TBESEN(Instance).ToNum(Arguments^[0]^);
   TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value:=BESENTimeClip(BESENMakeDate(BESENMakeDay(BESENYearFromTime(t),BESENMonthFromTime(t),Date),BESENTimeWithinDay(t)));
  end;
  ResultValue:=BESENNumberValue(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value);
 end else begin
  raise EBESENTypeError.Create('Not a date object');
 end;
end;

procedure TBESENObjectDatePrototype.NativeSetMonth(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
var t:TBESENDate;
    Month,Date:TBESENNumber;
begin
 if not ((ThisArgument.ValueType=bvtOBJECT) and assigned(TBESENObject(ThisArgument.Obj))) then begin
  raise EBESENTypeError.Create('Null this object');
 end;
 if assigned(TBESENObject(ThisArgument.Obj)) and (TBESENObject(ThisArgument.Obj) is TBESENObjectDate) then begin
  if CountArguments=0 then begin
   TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value:=double(pointer(@BESENDoubleNaN)^);
  end else begin
   t:=BESENLocalTime(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value);
   Month:=TBESEN(Instance).ToNum(Arguments^[0]^);
   if CountArguments>1 then begin
    Date:=TBESEN(Instance).ToNum(Arguments^[1]^);
   end else begin
    Date:=BESENDayFromTime(t);
   end;
   TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value:=BESENTimeClip(BESENUTC(BESENMakeDate(BESENMakeDay(BESENYearFromTime(t),Month,Date),BESENTimeWithinDay(t))));
  end;
  ResultValue:=BESENNumberValue(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value);
 end else begin
  raise EBESENTypeError.Create('Not a date object');
 end;
end;

procedure TBESENObjectDatePrototype.NativeSetUTCMonth(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
var t:TBESENDate;
    Month,Date:TBESENNumber;
begin
 if not ((ThisArgument.ValueType=bvtOBJECT) and assigned(TBESENObject(ThisArgument.Obj))) then begin
  raise EBESENTypeError.Create('Null this object');
 end;
 if assigned(TBESENObject(ThisArgument.Obj)) and (TBESENObject(ThisArgument.Obj) is TBESENObjectDate) then begin
  if CountArguments=0 then begin
   TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value:=double(pointer(@BESENDoubleNaN)^);
  end else begin
   t:=TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value;
   Month:=TBESEN(Instance).ToNum(Arguments^[0]^);
   if CountArguments>1 then begin
    Date:=TBESEN(Instance).ToNum(Arguments^[1]^);
   end else begin
    Date:=BESENDayFromTime(t);
   end;
   TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value:=BESENTimeClip(BESENMakeDate(BESENMakeDay(BESENYearFromTime(t),Month,Date),BESENTimeWithinDay(t)));
  end;
  ResultValue:=BESENNumberValue(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value);
 end else begin
  raise EBESENTypeError.Create('Not a date object');
 end;
end;

procedure TBESENObjectDatePrototype.NativeSetFullYear(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
var t:TBESENDate;
    Year,Month,Date:TBESENNumber;
begin
 if not ((ThisArgument.ValueType=bvtOBJECT) and assigned(TBESENObject(ThisArgument.Obj))) then begin
  raise EBESENTypeError.Create('Null this object');
 end;
 if assigned(TBESENObject(ThisArgument.Obj)) and (TBESENObject(ThisArgument.Obj) is TBESENObjectDate) then begin
  if CountArguments=0 then begin
   TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value:=double(pointer(@BESENDoubleNaN)^);
  end else begin
   t:=BESENLocalTime(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value);
   Year:=TBESEN(Instance).ToNum(Arguments^[0]^);
   if CountArguments>1 then begin
    Month:=TBESEN(Instance).ToNum(Arguments^[1]^);
   end else begin
    Month:=BESENMonthFromTime(t);
   end;
   if CountArguments>2 then begin
    Date:=TBESEN(Instance).ToNum(Arguments^[2]^);
   end else begin
    Date:=BESENDayFromTime(t);
   end;
   TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value:=BESENTimeClip(BESENUTC(BESENMakeDate(BESENMakeDay(Year,Month,Date),BESENTimeWithinDay(t))));
  end;
  ResultValue:=BESENNumberValue(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value);
 end else begin
  raise EBESENTypeError.Create('Not a date object');
 end;
end;

procedure TBESENObjectDatePrototype.NativeSetUTCFullYear(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
var t:TBESENDate;
    Year,Month,Date:TBESENNumber;
begin
 if not ((ThisArgument.ValueType=bvtOBJECT) and assigned(TBESENObject(ThisArgument.Obj))) then begin
  raise EBESENTypeError.Create('Null this object');
 end;
 if assigned(TBESENObject(ThisArgument.Obj)) and (TBESENObject(ThisArgument.Obj) is TBESENObjectDate) then begin
  if CountArguments=0 then begin
   TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value:=double(pointer(@BESENDoubleNaN)^);
  end else begin
   t:=TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value;
   Year:=TBESEN(Instance).ToNum(Arguments^[0]^);
   if CountArguments>1 then begin
    Month:=TBESEN(Instance).ToNum(Arguments^[1]^);
   end else begin
    Month:=BESENMonthFromTime(t);
   end;
   if CountArguments>2 then begin
    Date:=TBESEN(Instance).ToNum(Arguments^[2]^);
   end else begin
    Date:=BESENDayFromTime(t);
   end;
   TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value:=BESENTimeClip(BESENMakeDate(BESENMakeDay(Year,Month,Date),BESENTimeWithinDay(t)));
  end;
  ResultValue:=BESENNumberValue(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value);
 end else begin
  raise EBESENTypeError.Create('Not a date object');
 end;
end;

procedure TBESENObjectDatePrototype.NativeToGMTString(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 NativeToUTCString(ThisArgument,Arguments,CountArguments,ResultValue);
end;

procedure TBESENObjectDatePrototype.NativeToISOString(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 if not ((ThisArgument.ValueType=bvtOBJECT) and assigned(TBESENObject(ThisArgument.Obj))) then begin
  raise EBESENTypeError.Create('Null this object');
 end;
 if assigned(TBESENObject(ThisArgument.Obj)) and (TBESENObject(ThisArgument.Obj) is TBESENObjectDate) then begin
  if BESENIsNaN(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value) then begin
   if (TBESEN(Instance).Compatibility and COMPAT_JS)<>0 then begin
    ResultValue:=BESENStringValue('Invalid Date');
   end else begin
    ResultValue:=BESENStringValue('NaN');
   end;
  end else begin
   ResultValue:=BESENStringLocaleCharsetValue(BESENFormatDateTime('yyyy-mm-dd"T"hh:nn:ss.zzz"Z"',BESENDateToDateTime(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value),BESENDefaultFormatSettings));
  end;
 end else begin
  raise EBESENTypeError.Create('Not date object');
 end;
end;

procedure TBESENObjectDatePrototype.NativeToJSON(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
var ov,tv:TBESENValue;
begin
 TBESEN(Instance).ToObjectValue(ThisArgument,ov);
 TBESEN(Instance).ToPrimitiveValue(ov,TBESEN(Instance).ObjectNumberConstructorValue,tv);
 if (tv.ValueType=bvtNUMBER) and not BESENIsFinite(tv.Num) then begin
  ResultValue:=BESENNullValue;
 end else begin
  TBESENObject(ov.Obj).GarbageCollectorLock;
  try
   TBESENObject(ov.Obj).Get('toISOString',tv);
   if not BESENIsCallable(tv) then begin
    raise EBESENTypeError.Create('no "toISOString" callable object');
   end;
   TBESEN(Instance).ObjectCall(TBESENObject(tv.Obj),ov,nil,0,ResultValue);
  finally
   TBESENObject(ov.Obj).GarbageCollectorUnlock;
  end;
 end;
end;

procedure TBESENObjectDatePrototype.NativeGetYear(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
begin
 if not ((ThisArgument.ValueType=bvtOBJECT) and assigned(TBESENObject(ThisArgument.Obj))) then begin
  raise EBESENTypeError.Create('Null this object');
 end;
 if assigned(TBESENObject(ThisArgument.Obj)) and (TBESENObject(ThisArgument.Obj) is TBESENObjectDate) then begin
  if BESENIsNaN(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value) then begin
   ResultValue:=BESENNumberValue(double(pointer(@BESENDoubleNaN)^));
  end else begin
   ResultValue:=BESENNumberValue(BESENYearFromTime(BESENLocalTime(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value))-1900);
  end;
 end else begin
  raise EBESENTypeError.Create('Not a date object');
 end;
end;

procedure TBESENObjectDatePrototype.NativeSetYear(const ThisArgument:TBESENValue;Arguments:PPBESENValues;CountArguments:integer;var ResultValue:TBESENValue);
var t:TBESENDate;
    Year:TBESENNumber;
begin
 if not ((ThisArgument.ValueType=bvtOBJECT) and assigned(TBESENObject(ThisArgument.Obj))) then begin
  raise EBESENTypeError.Create('Null this object');
 end;
 if assigned(TBESENObject(ThisArgument.Obj)) and (TBESENObject(ThisArgument.Obj) is TBESENObjectDate) then begin
  if CountArguments=0 then begin
   TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value:=double(pointer(@BESENDoubleNaN)^);
  end else begin
   t:=BESENLocalTime(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value);
   Year:=TBESEN(Instance).ToNum(Arguments^[0]^);
   if (0<=Year) and (Year<=99) then begin
    Year:=Year+1900;
   end;
   TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value:=BESENTimeClip(BESENUTC(BESENMakeDate(BESENMakeDay(Year,BESENMonthFromTime(t),BESENDayFromTime(t)),BESENTimeWithinDay(t))));
  end;
  ResultValue:=BESENNumberValue(TBESENObjectDate(TBESENObject(ThisArgument.Obj)).Value);
 end else begin
  raise EBESENTypeError.Create('Not a date object');
 end;
end;

end.
