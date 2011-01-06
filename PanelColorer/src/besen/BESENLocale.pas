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
unit BESENLocale;
{$i BESEN.inc}

interface

uses {$ifdef windows}Windows,MMSystem,{$endif}{$ifdef unix}dl,BaseUnix,Unix,
     UnixType,{$endif}SysUtils,Classes,BESENConstants,BESENTypes,BESENBaseObject;

const BESENDefaultFormatSettings:TFormatSettings=(
       CurrencyFormat:1;
       NegCurrFormat:5;
       ThousandSeparator:',';
       DecimalSeparator:'.';
       CurrencyDecimals:2;
       DateSeparator:'-';
       TimeSeparator:':';
       ListSeparator:',';
       CurrencyString:'$';
       ShortDateFormat:'d/m/y';
       LongDateFormat:'dd" "mmmm" "yyyy';
       TimeAMString:'AM';
       TimePMString:'PM';
       ShortTimeFormat:'hh:nn';
       LongTimeFormat:'hh:nn:ss';
       ShortMonthNames:('Jan','Feb','Mar','Apr','May','Jun','Jul','Aug','Sep','Oct','Nov','Dec');
       LongMonthNames:('January','February','March','April','May','June','July','August','September','October','November','December');
       ShortDayNames:('Sun','Mon','Tue','Wed','Thu','Fri','Sat');
       LongDayNames:('Sunday','Monday','Tuesday','Wednesday','Thursday','Friday','Saturday');
       TwoDigitYearCenturyWindow:50;
      );

var BESENLocaleFormatSettings:TFormatSettings;

implementation

uses BESENCharSet,BESENStringUtils;

{$warnings off}
procedure InitLocaleFormatSettings;
{$ifdef windows}
{$ifdef fpc}
var i:integer;
begin
 BESENLocaleCharset:=ISO_8859_1;//BESENGetCodePage('ISO-8859-1');
 BESENLocaleFormatSettings:=BESENDefaultFormatSettings;
 for i:= 1 to 12 do begin
  BESENLocaleFormatSettings.ShortMonthNames[i]:=SysUtils.ShortMonthNames[i];
  BESENLocaleFormatSettings.LongMonthNames[i]:=SysUtils.LongMonthNames[i];
 end;
 for i:=1 to 7 do begin
  BESENLocaleFormatSettings.ShortDayNames[i]:=SysUtils.ShortDayNames[i];
  BESENLocaleFormatSettings.LongDayNames[i]:=SysUtils.LongDayNames[i];
 end;
 BESENLocaleFormatSettings.DateSeparator:=SysUtils.DateSeparator;
 BESENLocaleFormatSettings.ShortDateFormat:=SysUtils.ShortDateFormat;
 BESENLocaleFormatSettings.LongDateFormat:=SysUtils.LongDateFormat;
 BESENLocaleFormatSettings.TimeSeparator:=SysUtils.TimeSeparator;
 BESENLocaleFormatSettings.TimeAMString:=SysUtils.TimeAMString;
 BESENLocaleFormatSettings.TimePMString:=SysUtils.TimePMString;
 BESENLocaleFormatSettings.ShortTimeFormat:=SysUtils.ShortTimeFormat;
 BESENLocaleFormatSettings.LongTimeFormat:=SysUtils.LongTimeFormat;
 BESENLocaleFormatSettings.CurrencyString:=SysUtils.CurrencyString;
 BESENLocaleFormatSettings.CurrencyFormat:=SysUtils.CurrencyFormat;
 BESENLocaleFormatSettings.NegCurrFormat:=SysUtils.NegCurrFormat;
 BESENLocaleFormatSettings.ThousandSeparator:=SysUtils.ThousandSeparator;
 BESENLocaleFormatSettings.DecimalSeparator:=SysUtils.DecimalSeparator;
 BESENLocaleFormatSettings.CurrencyDecimals:=SysUtils.CurrencyDecimals;
 BESENLocaleFormatSettings.ListSeparator:=SysUtils.ListSeparator;
end;
{$else}
var HourFormat,TimePrefix,TimePostfix:TBESENANSISTRING;
    LID:LCID;
    i,LCP,Day:integer;
begin
 BESENLocaleFormatSettings:=BESENDefaultFormatSettings;
 LID:=GetThreadLocale;
 if not TryStrToInt(GetLocaleStr(LID,LOCALE_IDEFAULTANSICODEPAGE,inttostr(GetACP)),LCP) then begin
  LCP:=GetACP;
 end;
 if LCP>0 then begin
  BESENLocaleCharset:=BESENGetCodePage('WINDOWS-'+inttostr(LCP));
 end else begin
  BESENLocaleCharset:=ISO_8859_1;
 end;
 for i:=1 to 12 do begin
  BESENLocaleFormatSettings.ShortMonthNames[i]:=GetLocaleStr(LID,LOCALE_SABBREVMONTHNAME1+i-1,ShortMonthNames[i]);
  BESENLocaleFormatSettings.LongMonthNames[i]:=GetLocaleStr(LID,LOCALE_SMONTHNAME1+i-1,LongMonthNames[i]);
 end;
 for i:=1 to 7 do begin
  Day:=(i+5) mod 7;
  BESENLocaleFormatSettings.ShortDayNames[i]:=GetLocaleStr(LID,LOCALE_SABBREVDAYNAME1+Day,ShortDayNames[i]);
  BESENLocaleFormatSettings.LongDayNames[i]:=GetLocaleStr(LID,LOCALE_SDAYNAME1+Day,LongDayNames[i]);
 end;
 BESENLocaleFormatSettings.DateSeparator:=GetLocaleChar(LID,LOCALE_SDATE,'/');
 BESENLocaleFormatSettings.ShortDateFormat:=GetLocaleStr(LID,LOCALE_SSHORTDATE,'m/d/yy');
 BESENLocaleFormatSettings.LongDateFormat:=GetLocaleStr(LID,LOCALE_SLONGDATE,'mmmm d, yyyy');
 BESENLocaleFormatSettings.TimeSeparator:=GetLocaleChar(LID,LOCALE_STIME,':');
 BESENLocaleFormatSettings.TimeAMString:=GetLocaleStr(LID,LOCALE_S1159,'AM');
 BESENLocaleFormatSettings.TimePMString:=GetLocaleStr(LID,LOCALE_S2359,'PM');
 if StrToIntDef(GetLocaleStr(LID,LOCALE_ITLZERO,'0'),0)=0 then begin
  HourFormat:='h';
 end else begin
  HourFormat:='hh';
 end;
 TimePostfix:='';
 TimePrefix:='';
 if StrToIntDef(GetLocaleStr(LID,LOCALE_ITIME,'0'),0)=0 then begin
  if StrToIntDef(GetLocaleStr(LID,LOCALE_ITIMEMARKPOSN,'0'),0)=0 then begin
   TimePostfix:=' AMPM';
  end else begin
   TimePrefix:='AMPM ';
  end;
 end;
 BESENLocaleFormatSettings.ShortTimeFormat:=TimePrefix+HourFormat+':nn'+TimePrefix;
 BESENLocaleFormatSettings.LongTimeFormat:=TimePrefix+HourFormat+':nn:ss'+TimePrefix;
 BESENLocaleFormatSettings.CurrencyString:=GetLocaleStr(LID,LOCALE_SCURRENCY,'');
 BESENLocaleFormatSettings.CurrencyFormat:=StrToIntDef(GetLocaleStr(LID,LOCALE_ICURRENCY,'0'),0);
 BESENLocaleFormatSettings.NegCurrFormat:=StrToIntDef(GetLocaleStr(LID,LOCALE_INEGCURR,'0'),0);
 BESENLocaleFormatSettings.ThousandSeparator:=GetLocaleChar(LID,LOCALE_STHOUSAND,',');
 BESENLocaleFormatSettings.DecimalSeparator:=GetLocaleChar(LID,LOCALE_SDECIMAL,'.');
 BESENLocaleFormatSettings.CurrencyDecimals:=StrToIntDef(GetLocaleStr(LID,LOCALE_ICURRDIGITS,'0'),0);
 BESENLocaleFormatSettings.ListSeparator:=GetLocaleChar(LID,LOCALE_SLIST,',');
end;
{$endif}
{$else}
var i:integer;
begin
 BESENLocaleCharset:=ISO_8859_1;//BESENGetCodePage('ISO-8859-1');
 BESENLocaleFormatSettings:=BESENDefaultFormatSettings;
 for i:= 1 to 12 do begin
  BESENLocaleFormatSettings.ShortMonthNames[i]:=SysUtils.ShortMonthNames[i];
  BESENLocaleFormatSettings.LongMonthNames[i]:=SysUtils.LongMonthNames[i];
 end;
 for i:=1 to 7 do begin
  BESENLocaleFormatSettings.ShortDayNames[i]:=SysUtils.ShortDayNames[i];
  BESENLocaleFormatSettings.LongDayNames[i]:=SysUtils.LongDayNames[i];
 end;
 BESENLocaleFormatSettings.DateSeparator:=SysUtils.DateSeparator;
 BESENLocaleFormatSettings.ShortDateFormat:=SysUtils.ShortDateFormat;
 BESENLocaleFormatSettings.LongDateFormat:=SysUtils.LongDateFormat;
 BESENLocaleFormatSettings.TimeSeparator:=SysUtils.TimeSeparator;
 BESENLocaleFormatSettings.TimeAMString:=SysUtils.TimeAMString;
 BESENLocaleFormatSettings.TimePMString:=SysUtils.TimePMString;
 BESENLocaleFormatSettings.ShortTimeFormat:=SysUtils.ShortTimeFormat;
 BESENLocaleFormatSettings.LongTimeFormat:=SysUtils.LongTimeFormat;
 BESENLocaleFormatSettings.CurrencyString:=SysUtils.CurrencyString;
 BESENLocaleFormatSettings.CurrencyFormat:=SysUtils.CurrencyFormat;
 BESENLocaleFormatSettings.NegCurrFormat:=SysUtils.NegCurrFormat;
 BESENLocaleFormatSettings.ThousandSeparator:=SysUtils.ThousandSeparator;
 BESENLocaleFormatSettings.DecimalSeparator:=SysUtils.DecimalSeparator;
 BESENLocaleFormatSettings.CurrencyDecimals:=SysUtils.CurrencyDecimals;
 BESENLocaleFormatSettings.ListSeparator:=SysUtils.ListSeparator;
end;
{$endif}
{$warnings on}

procedure InitBESEN;
begin
 InitLocaleFormatSettings;
end;

procedure DoneBESEN;
begin
end;

initialization
 InitBESEN;
finalization
 DoneBESEN;

end.
