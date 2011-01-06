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
unit BESENLexer;
{$i BESEN.inc}

interface

uses BESENConstants,BESENTypes,BESENStringUtils,BESENBaseObject;

type TBESENLexerTokenType=(ltNONE,ltUNKNOWN,ltEOF,

                           // Types
                           lttIDENTIFIER,lttSTRING,lttINTEGER,lttFLOAT,

                           // Operators
                           ltoASSIGNMENT,ltoBITWISEAND,ltoBITWISEANDASSIGNMENT,
                           ltoBITWISENOT,ltoBITWISEOR,ltoBITWISEORASSIGNMENT,
                           ltoBITWISEXOR,ltoBITWISEXORASSIGNMENT,ltoCLOSEBRACE,
                           ltoCLOSEPAREN,ltoCLOSESQUARE,ltoCOLON,ltoCOMMA,
                           ltoCONDITIONAL,ltoDIVIDE,ltoDIVIDEASSIGNMENT,ltoDOT,
                           ltoEQUALEQUAL,ltoEQUALEQUALEQUAL,ltoGREATERTHAN,
                           ltoGREATERTHANOREQUAL,ltoLESSTHAN,ltoLESSTHANOREQUAL,
                           ltoLOGICALAND,ltoLOGICALNOT,ltoLOGICALOR,ltoMINUS,
                           ltoMINUSASSIGNMENT,ltoMINUSMINUS,ltoMODULO,
                           ltoMODULOASSIGNMENT,ltoMULTIPLY,
                           ltoMULTIPLYASSIGNMENT,ltoNOTEQUAL,ltoNOTEQUALEQUAL,
                           ltoOPENBRACE,ltoOPENPAREN,ltoOPENSQUARE,ltoPLUS,
                           ltoPLUSASSIGNMENT,ltoPLUSPLUS,ltoSEMICOLON,
                           ltoSHIFTLEFT,ltoSHIFTLEFTASSIGNMENT,ltoSHIFTRIGHT,
                           ltoSHIFTRIGHTASSIGNMENT,ltoSHIFTRIGHTUNSIGNED,
                           ltoSHIFTRIGHTUNSIGNEDASSIGNMENT,

                           // Used keywords
                           ltkBREAK,ltkCASE,ltkCATCH,ltkCONTINUE,ltkDEBUGGER,
                           ltkDEFAULT,ltkDELETE,ltkDO,ltkELSE,ltkFALSE,
                           ltkFINALLY,ltkFOR,ltkFUNCTION,ltkIF,ltkIN,
                           ltkINSTANCEOF,ltkNEW,ltkNULL,ltkRETURN,ltkSWITCH,
                           ltkTHIS,ltkTHROW,ltkTRUE,ltkTRY,ltkTYPEOF,ltkVAR,
                           ltkVOID,ltkWHILE,ltkWITH,

                           // Reserved keywords
                           ltkCLASS,ltkCONST,ltkENUM,ltkEXPORT,ltkEXTENDS,
                           ltkIMPORT,ltkSUPER,

                           // Additional reserved keywords in strict mode
                           ltkIMPLEMENTS,ltkINTERFACE,ltkLET,ltkPACKAGE,
                           ltkPRIVATE,ltkPROTECTED,ltkPUBLIC,ltkSTATIC,
                           ltkYIELD
                          );

     TBESENLexerTokenTypeSet=set of TBESENLexerTokenType;

     TBESENLexerToken=record
      TokenType:TBESENLexerTokenType;
      Name:TBESENString;
      StringValue:TBESENString;
      IntValue:int64;
      FloatValue:double;
      LineNumber:integer;
      LineEnd:longbool;
      WasLineEnd:longbool;
      OldChar:TBESENUTF32CHAR;
      OldLineNumber:integer;
      OldPosition:integer;
      OldCharEOF:longbool;
      OldTokenEOF:longbool;
     end;

     TBESENLexer=class(TBESENBaseObject)
      public
       Source:TBESENUTF8STRING;
       Position:integer;
       LineNumber:integer;
       CurrentChar:TBESENUTF32CHAR;
       WarningProc:TBESENWarningProc;
       CharEOF:longbool;
       TokenEOF:longbool;
       LastWasLineEnd:longbool;
       constructor Create(AInstance:TObject); overload; override;
       destructor Destroy; override;
       procedure NextChar;
       procedure Restore(var Token:TBESENLexerToken);
       function ParseRegExpLiteral(var Token:TBESENLexerToken;var ASource,AFlags:TBESENString):boolean;
       function IsEOF:boolean;
       procedure GetToken(var AResult:TBESENLexerToken);
     end;

const BESENLexerReservedTokens:TBESENLexerTokenTypeSet=[ltkCLASS,ltkCONST,ltkENUM,ltkEXPORT,ltkEXTENDS,ltkIMPORT,ltkSUPER];

      BESENLexerStrictReservedTokens:TBESENLexerTokenTypeSet=[ltkIMPLEMENTS,ltkINTERFACE,ltkLET,ltkPACKAGE,ltkPRIVATE,
                                                              ltkPROTECTED,ltkPUBLIC,ltkSTATIC,ltkYIELD];

      BESENLexerKeywordsTokens:TBESENLexerTokenTypeSet=[ltkBREAK..ltkYIELD];

      BESENTokenStrings:array[TBESENLexerTokenType] of TBESENString=('','','EOF',

                                                                     // Types
                                                                     'IDENTIFIER','STRING','INTEGER','FLOAT',

                                                                     // Operators
                                                                     '==','&','&=',
                                                                     '~','|','|=',
                                                                     '^','^=','}',
                                                                     ')',']',':',',',
                                                                     '?','/','/=','.',
                                                                     '==','===','>',
                                                                     '>=','<','<=',
                                                                     '&&','!','||','-',
                                                                     '-=','--','%',
                                                                     '%=','*',
                                                                     '*=','!=','!==',
                                                                     '{','(','[','+',
                                                                     '+=','++',';',
                                                                     '<<','<<=','>>',
                                                                     '>>=','>>>','>>>=',

                                                                     // Used keywords
                                                                     'break','case','catch','continue','debugger',
                                                                     'default','delete','do','else','false',
                                                                     'finally','for','function','if','in',
                                                                     'instanceof','new','null','return','switch',
                                                                     'this','throw','true','try','typeof','var',
                                                                     'void','while','with',

                                                                     // Reserved keywords
                                                                     'class','const','enum','export','extends',
                                                                     'import','super',

                                                                     // Additional reserved keywords in strict mode
                                                                     'implements','interface','let','package',
                                                                     'private','protected','public','static',
                                                                     'yield');

implementation

uses BESEN,BESENDTOA,BESENRegExp,BESENErrors;

type TKeywordToken=ltkBREAK..ltkYIELD;

     TKeyword=TKeywordToken;

     TKeywords=array of TKeyword;

const KeywordNames:array[TKeywordToken] of TBESENUTF16STRING=
       (// Used keywords
        'break','case','catch','continue','debugger',
        'default','delete','do','else','false',
        'finally','for','function','if','in',
        'instanceof','new','null','return','switch',
        'this','throw','true','try','typeof','var',
        'void','while','with',

        // Reserved keywords
        'class','const','enum','export','extends',
        'import','super',

        // Additional reserved keywords in strict mode
        'implements','interface','let','package',
        'private','protected','public','static',
        'yield');

      Keywords:TKeywords=nil;

constructor TBESENLexer.Create(AInstance:TObject);
begin
 inherited Create(AInstance);
 Source:='';
 Position:=1;
 LineNumber:=1;
 WarningProc:=nil;
 CharEOF:=false;
 TokenEOF:=false;
 LastWasLineEnd:=false;
end;

destructor TBESENLexer.Destroy;
begin
 Source:='';
 inherited Destroy;
end;

function TBESENLexer.IsEOF:boolean;
begin
 result:=TokenEOF;
end;

procedure TBESENLexer.NextChar;
 function UTF8GetNextChar(var s:TBESENUTF8STRING;var i:integer):TBESENUTF32CHAR;
 var b:byte;
     UnsafeUTF8:boolean;
 begin
  if (i<1) or (i>length(s)) then begin
   result:=0;
   inc(i);
   exit;
  end;
{$ifdef strictutf8}
  UnsafeUTF8:=(TBESEN(Instance).Compatibility and COMPAT_UTF8_UNSAFE)<>0;
{$else}
  UnsafeUTF8:=true;
{$endif}
  b:=byte(s[i]);
  if (b and $80)=0 then begin
   result:=b;
   inc(i);
  end else if ((i+1)<=length(s)) and ((b and $e0)=$c0) and ((byte(s[i+1]) and $c0)=$80) then begin
   result:=((byte(s[i]) and $1f) shl 6) or (byte(s[i+1]) and $3f);
   inc(i,2);
  end else if ((i+2)<=length(s)) and ((b and $f0)=$e0) and ((byte(s[i+1]) and $c0)=$80) and ((byte(s[i+2]) and $c0)=$80) then begin
   result:=((byte(s[i]) and $0f) shl 12) or ((byte(s[i+1]) and $3f) shl 6) or (byte(s[i+2]) and $3f);
   inc(i,3);
  end else if ((i+3)<=length(s)) and ((b and $f8)=$f0) and ((byte(s[i+1]) and $c0)=$80) and ((byte(s[i+2]) and $c0)=$80) and ((byte(s[i+3]) and $c0)=$80) then begin
   result:=((byte(s[i]) and $07) shl 18) or ((byte(s[i+1]) and $3f) shl 12) or ((byte(s[i+2]) and $3f) shl 6) or (byte(s[i+3]) and $3f);
   inc(i,4);
  end else if UnsafeUTF8 and ((i+4)<=length(s)) and ((b and $fc)=$f8) and ((byte(s[i+1]) and $c0)=$80) and ((byte(s[i+2]) and $c0)=$80) and ((byte(s[i+3]) and $c0)=$80) and ((byte(s[i+4]) and $c0)=$80) then begin
   result:=((byte(s[i]) and $03) shl 24) or ((byte(s[i+1]) and $3f) shl 18) or ((byte(s[i+2]) and $3f) shl 12) or ((byte(s[i+3]) and $3f) shl 6) or (byte(s[i+4]) and $3f);
   inc(i,5);
  end else if UnsafeUTF8 and ((i+5)<=length(s)) and ((b and $fe)=$fc) and ((byte(s[i+1]) and $c0)=$80) and ((byte(s[i+2]) and $c0)=$80) and ((byte(s[i+3]) and $c0)=$80) and ((byte(s[i+4]) and $c0)=$80) and ((byte(s[i+5]) and $c0)=$80) then begin
   result:=((byte(s[i]) and $01) shl 30) or ((byte(s[i+1]) and $3f) shl 24) or ((byte(s[i+2]) and $3f) shl 18) or ((byte(s[i+3]) and $3f) shl 12) or ((byte(s[i+4]) and $3f) shl 6) or (byte(s[i+5]) and $3f);
   inc(i,6);
  end else begin
   result:=$fffd;
   inc(i);
  end;
 end;
begin
 if Position<=length(Source) then begin
  CurrentChar:=UTF8GetNextChar(Source,Position);
 end else begin
  CurrentChar:=0;
  CharEOF:=true;
 end;
end;

procedure TBESENLexer.Restore(var Token:TBESENLexerToken);
begin
 Position:=Token.OldPosition;
 LineNumber:=Token.OldLineNumber;
 CurrentChar:=Token.OldChar;
 CharEOF:=Token.OldCharEOF;
 TokenEOF:=Token.OldTokenEOF;
end;

function TBESENLexer.ParseRegExpLiteral(var Token:TBESENLexerToken;var ASource,AFlags:TBESENString):boolean;
var InCharClass,fg,fi,fm:boolean;
    RegExpFlags:TBESENRegExpFlags;
begin
 result:=false;
 ASource:='';
 AFlags:='';
 InCharClass:=false;
 while not CharEOF do begin
  NextChar;
  case CurrentChar of
   $00,$0a,$0d,$2028,$2029:begin
    exit;
   end;
   ord('\'):begin
    ASource:=ASource+BESENUTF32CHARToUTF16(CurrentChar);
    NextChar;
   end;
   ord('['):begin
    InCharClass:=true;
   end;
   ord(']'):begin
    InCharClass:=false;
   end;
   ord('/'):begin
    if not InCharClass then begin
     break;
    end;
   end;
  end;
  ASource:=ASource+BESENUTF32CHARToUTF16(CurrentChar);
 end;
 if (not CharEOF) and (CurrentChar=ord('/')) then begin
  NextChar;
 end else begin
  raise EBESENSyntaxError.Create('unterminated regex');
 end;
 AFlags:='';
 fg:=false;
 fi:=false;
 fm:=false;
 while not CharEOF do begin
  case CurrentChar of
   ord('g'):begin
    if fg then begin
     raise EBESENSyntaxError.Create('Too many global regular expression flags');
    end else begin
     fg:=true;
     AFlags:=AFlags+BESENUTF32CHARToUTF16(CurrentChar);
     NextChar;
    end;
   end;
   ord('i'):begin
    if fi then begin
     raise EBESENSyntaxError.Create('Too many ignorecase regular expression flags');
    end else begin
     fi:=true;
     AFlags:=AFlags+BESENUTF32CHARToUTF16(CurrentChar);
     NextChar;
    end;
   end;
   ord('m'):begin
    if fm then begin
     raise EBESENSyntaxError.Create('Too many multiline regular expression flags');
    end else begin
     fm:=true;
     AFlags:=AFlags+BESENUTF32CHARToUTF16(CurrentChar);
     NextChar;
    end;
   end;
   else begin
    break;
   end;
  end;
 end;
 if BESENUnicodeIsLetter(CurrentChar) then begin
  raise EBESENSyntaxError.Create('Unknown regular expression flag');
 end;
 RegExpFlags:=[];
 if fg then begin
  RegExpFlags:=RegExpFlags+[brefGLOBAL];
 end;
 if fi then begin
  RegExpFlags:=RegExpFlags+[brefIGNORECASE];
 end;
 if fm then begin
  RegExpFlags:=RegExpFlags+[brefMULTILINE];
 end;
 try
  TBESEN(Instance).RegExpCache.Get(ASource,RegExpFlags);
 except
  raise EBESENSyntaxError.Create('Invalid regular expression');
 end;
 Token.StringValue:='/'+ASource+'/'+AFlags;
 result:=true;
end;

procedure TBESENLexer.GetToken(var AResult:TBESENLexerToken);
var c:TBESENUTF32CHAR;
    v:longword;
 procedure AddError(const Msg:TBESENANSISTRING);
 begin
  TBESEN(Instance).LineNumber:=LineNumber;
  raise EBESENSyntaxError.Create(Msg);
 end;
 procedure AddWarning(const Msg:TBESENANSISTRING);
 begin
  if assigned(WarningProc) then begin
   WarningProc(LineNumber,Msg);
  end;
 end;
 procedure ParseNumber(c:widechar;var Token:TBESENLexerToken);
 var s:TBESENString;
     HasDigits:boolean;
 begin
  Token.TokenType:=ltUNKNOWN;

  HasDigits:=false;

  s:='';
  
  if c='.' then begin
   if (not CharEOF) and ((CurrentChar>=ord('0')) and (CurrentChar<=ord('9'))) then begin
    s:=s+'.';

    while (not CharEOF) and (CurrentChar=ord('0')) do begin
     s:=s+widechar(CurrentChar);
     NextChar;
    end;

    while (not CharEOF) and ((CurrentChar>=ord('0')) and (CurrentChar<=ord('9'))) do begin
     s:=s+widechar(CurrentChar);
     NextChar;
    end;

    if (not CharEOF) and ((CurrentChar=ord('e')) or (CurrentChar=ord('E'))) then begin
     s:=s+widechar(CurrentChar);
     NextChar;
     if (not CharEOF) and ((CurrentChar=ord('+')) or (CurrentChar=ord('-'))) then begin
      s:=s+widechar(CurrentChar);
      NextChar;
     end;
     while (not CharEOF) and ((CurrentChar>=ord('0')) and (CurrentChar<=ord('9'))) do begin
      s:=s+widechar(CurrentChar);
      NextChar;
     end;
    end;

    Token.TokenType:=lttFLOAT;
    Token.FloatValue:=strtod(PAnsiChar(ansistring('0'+s)),nil,false);
    s:='';
   end;
   exit;
  end;

  if (not CharEOF) and (CurrentChar=ord('0')) then begin
   NextChar;
   case CurrentChar of
    ord('x'),ord('X'):begin
     NextChar;
     s:='0x';
     while not CharEOF do begin
      case CurrentChar of
       ord('a')..ord('f'),ord('A')..ord('F'),ord('0')..ord('9'):begin
        s:=s+widechar(word(CurrentChar));
        HasDigits:=true;
        NextChar;
       end;
       else begin
        break;
       end;
      end;
     end;
     if HasDigits then begin
      Token.TokenType:=lttFLOAT;
      Token.FloatValue:=strtod(puchar(ansistring(s)),nil,false);
     end;
     s:='';
     exit;
    end;
    ord('0')..ord('7'):begin
     if ((TBESEN(Instance).Compatibility and COMPAT_JS)<>0) and not TBESEN(Instance).IsStrict then begin
      s:='0';
      while not CharEOF do begin
       case CurrentChar of
        ord('0')..ord('7'):begin
         s:=s+widechar(word(CurrentChar));
         HasDigits:=true;
         NextChar;
        end;
        else begin
         break;
        end;
       end;
      end;
      if HasDigits then begin
       Token.TokenType:=lttFLOAT;
       Token.FloatValue:=strtod(puchar(ansistring(s)),nil,true);
      end;
      s:='';
      exit;
     end else begin
      HasDigits:=true;
      s:=s+widechar('0');
     end;
    end;
    else begin
     HasDigits:=true;
     s:=s+widechar('0');
    end;
   end;
  end;

  if TBESEN(Instance).IsStrict and HasDigits and ((not CharEOF) and ((CurrentChar>=ord('0')) and (CurrentChar<=ord('9')))) then begin
   AddError('Bad number literal');
  end else begin

   while (not CharEOF) and (CurrentChar=ord('0')) do begin
    s:=s+widechar(CurrentChar);
    HasDigits:=true;
    NextChar;
   end;

   if TBESEN(Instance).IsStrict and HasDigits and ((not CharEOF) and ((CurrentChar>=ord('0')) and (CurrentChar<=ord('9')))) then begin
    AddError('Bad number literal');
   end else begin

    while (not CharEOF) and ((CurrentChar>=ord('0')) and (CurrentChar<=ord('9'))) do begin
     s:=s+widechar(CurrentChar);
     HasDigits:=true;
     NextChar;
    end;

    if (not CharEOF) and (CurrentChar=ord('.')) then begin
     s:=s+widechar(CurrentChar);
     NextChar;

     while (not CharEOF) and ((CurrentChar>=ord('0')) and (CurrentChar<=ord('9'))) do begin
      s:=s+widechar(CurrentChar);
      NextChar;
      HasDigits:=true;
     end;
    end;

    if HasDigits then begin

     if (not CharEOF) and ((CurrentChar=ord('e')) or (CurrentChar=ord('E'))) then begin
      HasDigits:=false;
      s:=s+widechar(CurrentChar);
      NextChar;
      if (not CharEOF) and ((CurrentChar=ord('+')) or (CurrentChar=ord('-'))) then begin
       s:=s+widechar(CurrentChar);
       NextChar;
      end;
      if (not CharEOF) and ((CurrentChar>=ord('0')) and (CurrentChar<=ord('9'))) then begin
       HasDigits:=true;
       while (not CharEOF) and ((CurrentChar>=ord('0')) and (CurrentChar<=ord('9'))) do begin
        s:=s+widechar(CurrentChar);
        NextChar;
       end;
      end else begin
       exit;
      end;
     end;

     if HasDigits then begin
      Token.TokenType:=lttFLOAT;
      Token.FloatValue:=strtod(PAnsiChar(ansistring(s)),nil,false);
     end;
    end;
   end;
  end;

  s:='';
 end;
 function FindKeyword(const Name:TBESENUTF16STRING):TBESENLexerTokenType;
 var LowIndex,HighIndex,MiddleIndex:integer;
 begin
  result:=lttIDENTIFIER;
  LowIndex:=0;
  HighIndex:=length(Keywords)-1;
  while LowIndex<=HighIndex do begin
   case (HighIndex-LowIndex)+1 of
    1:begin
     if KeywordNames[Keywords[LowIndex]]=Name then begin
      result:=Keywords[LowIndex];
     end;
    end;
    2:begin
     if KeywordNames[Keywords[LowIndex]]=Name then begin
      result:=Keywords[LowIndex];
     end else if KeywordNames[Keywords[HighIndex]]=Name then begin
      result:=Keywords[HighIndex];
     end;
    end;
    else begin
     MiddleIndex:=(LowIndex+HighIndex) div 2;
     if KeywordNames[Keywords[MiddleIndex]]=Name then begin
      result:=Keywords[MiddleIndex];
     end else if LowIndex<>HighIndex then begin
      if KeywordNames[Keywords[MiddleIndex]]>Name then begin
       HighIndex:=MiddleIndex-1;
      end else begin
       LowIndex:=MiddleIndex+1;
      end;
      continue;
     end;
    end;
   end;
   break;
  end;
 end;
begin
 AResult.Name:='';
 AResult.StringValue:='';
 AResult.FloatValue:=0;
 AResult.IntValue:=0;
 AResult.LineNumber:=LineNumber;
 AResult.LineEnd:=false;
 AResult.WasLineEnd:=LastWasLineEnd;
 AResult.OldChar:=CurrentChar;
 AResult.OldLineNumber:=LineNumber;
 AResult.OldPosition:=Position;
 AResult.OldCharEOF:=CharEOF;
 AResult.OldTokenEOF:=TokenEOF;
 LastWasLineEnd:=false;
 AResult.TokenType:=ltUNKNOWN;
 while true do begin
  if CharEOF then begin
   TokenEOF:=true;
   AResult.TokenType:=ltEOF;
   break;
  end else begin
   while BESENUnicodeIsParserWhiteSpace(CurrentChar) do begin
    case CurrentChar of
     0:begin
      NextChar;
      if not CharEOF then begin
       CurrentChar:=1;
      end;
     end;
     $000d:begin
      NextChar;
      if CurrentChar=$000a then begin
       NextChar;
      end;
      inc(LineNumber);
      AResult.WasLineEnd:=true;
     end;
     $000a,$2028,$2029:begin
      NextChar;
      inc(LineNumber);
      AResult.WasLineEnd:=true;
     end;
     else begin
      NextChar;
     end;
    end;
   end;
   if CharEOF then begin
    AResult.TokenType:=ltEOF;
    TokenEOF:=true;
    break;
   end else begin
    AResult.LineNumber:=LineNumber;
    AResult.OldChar:=CurrentChar;
    AResult.OldLineNumber:=LineNumber;
    AResult.OldPosition:=Position;
    AResult.OldCharEOF:=CharEOF;
    AResult.OldTokenEOF:=TokenEOF;
    case CurrentChar of
     0:begin
     end;
     ord('='):begin
      NextChar;
      case CurrentChar of
       ord('='):begin
        NextChar;
        case CurrentChar of
         ord('='):begin
          NextChar;
          AResult.TokenType:=ltoEQUALEQUALEQUAL;
         end;
         else begin
          AResult.TokenType:=ltoEQUALEQUAL;
         end;
        end;
       end;
       else begin
        AResult.TokenType:=ltoASSIGNMENT;
       end;
      end;
     end;
     ord('&'):begin
      NextChar;
      case CurrentChar of
       ord('&'):begin
        NextChar;
        AResult.TokenType:=ltoLOGICALAND;
       end;
       ord('='):begin
        NextChar;
        AResult.TokenType:=ltoBITWISEANDASSIGNMENT;
       end;
       else begin
        AResult.TokenType:=ltoBITWISEAND;
       end;
      end;
     end;
     ord('|'):begin
      NextChar;
      case CurrentChar of
       ord('|'):begin
        NextChar;
        AResult.TokenType:=ltoLOGICALOR;
       end;
       ord('='):begin
        NextChar;
        AResult.TokenType:=ltoBITWISEORASSIGNMENT;
       end;
       else begin
        AResult.TokenType:=ltoBITWISEOR;
       end;
      end;
     end;
     ord('^'):begin
      NextChar;
      case CurrentChar of
       ord('='):begin
        NextChar;
        AResult.TokenType:=ltoBITWISEXORASSIGNMENT;
       end;
       else begin
        AResult.TokenType:=ltoBITWISEXOR;
       end;
      end;
     end;
     ord('/'):begin
      NextChar;
      case CurrentChar of
       ord('/'):begin
        while ((CurrentChar<>0) and not BESENUnicodeIsLineTerminator(CurrentChar)) and not CharEOF do begin
         NextChar;
        end;
        if BESENUnicodeIsLineTerminator(CurrentChar) then begin
         inc(LineNumber);
         AResult.WasLineEnd:=true;
         if CurrentChar=$000d then begin
          NextChar;
          if CurrentChar=$000a then begin
           NextChar;
          end;
         end else begin
          NextChar;
         end;
        end;
        continue;
       end;
       ord('*'):begin
        NextChar;
        c:=0;
        while (not ((c=ord('*')) and (CurrentChar=ord('/')))) and not CharEOF do begin
         if BESENUnicodeIsLineTerminator(CurrentChar) then begin
          inc(LineNumber);
          AResult.WasLineEnd:=true;
          if CurrentChar=$000d then begin
           c:=CurrentChar;
           NextChar;
           if CurrentChar=$000a then begin
            c:=CurrentChar;
            NextChar;
           end;
          end else begin
           c:=CurrentChar;
           NextChar;
          end;
         end else begin
          c:=CurrentChar;
          NextChar;
         end;
        end;
        if CurrentChar=ord('/') then begin
         NextChar;
        end;
        continue;
       end;
       ord('='):begin
        NextChar;
        AResult.TokenType:=ltoDIVIDEASSIGNMENT;
       end;
       else begin
        AResult.TokenType:=ltoDIVIDE;
       end;
      end;
     end;
     ord('>'):begin
      NextChar;
      case CurrentChar of
       ord('>'):begin
        NextChar;
        case CurrentChar of
         ord('>'):begin
          NextChar;
          case CurrentChar of
           ord('='):begin
            NextChar;
            AResult.TokenType:=ltoSHIFTRIGHTUNSIGNEDASSIGNMENT;
           end;
           else begin
            AResult.TokenType:=ltoSHIFTRIGHTUNSIGNED;
           end;
          end;
         end;
         ord('='):begin
          NextChar;
          AResult.TokenType:=ltoSHIFTRIGHTASSIGNMENT;
         end;
         else begin
          AResult.TokenType:=ltoSHIFTRIGHT;
         end;
        end;
       end;
       ord('='):begin
        NextChar;
        AResult.TokenType:=ltoGREATERTHANOREQUAL;
       end;
       else begin
        AResult.TokenType:=ltoGREATERTHAN;
       end;
      end;
     end;
     ord('<'):begin
      NextChar;
      case CurrentChar of
       ord('<'):begin
        NextChar;
        case CurrentChar of
         ord('='):begin
          NextChar;
          AResult.TokenType:=ltoSHIFTLEFTASSIGNMENT;
         end;
         else begin
          AResult.TokenType:=ltoSHIFTLEFT;
         end;
        end;
       end;
       ord('='):begin
        NextChar;
        AResult.TokenType:=ltoLESSTHANOREQUAL;
       end;
       ord('!'):begin
        if (TBESEN(Instance).Compatibility and COMPAT_SGMLCOM)<>0 then begin
         if ((Position+2)<=length(Source)) and (Source[Position]='-') and (Source[Position+1]='-') then begin
          while ((CurrentChar<>0) and not BESENUnicodeIsLineTerminator(CurrentChar)) and not CharEOF do begin
           NextChar;
          end;
          if BESENUnicodeIsLineTerminator(CurrentChar) then begin
           inc(LineNumber);
           AResult.WasLineEnd:=true;
           if CurrentChar=$000d then begin
            NextChar;
            if CurrentChar=$000a then begin
             NextChar;
            end;
           end else begin
            NextChar;
           end;
           continue;
          end;
         end else begin
          AResult.TokenType:=ltoLESSTHAN;
         end;
        end else begin
         AResult.TokenType:=ltoLESSTHAN;
        end;
       end;
       else begin
        AResult.TokenType:=ltoLESSTHAN;
       end;
      end;
     end;
     ord('-'):begin
      NextChar;
      case CurrentChar of
       ord('-'):begin
        NextChar;
        if (TBESEN(Instance).Compatibility and COMPAT_SGMLCOM)<>0 then begin
         if (Position=length(Source)) and (Source[Position]='>') then begin
          while ((CurrentChar<>0) and not BESENUnicodeIsLineTerminator(CurrentChar)) and not CharEOF do begin
           NextChar;
          end;
          if BESENUnicodeIsLineTerminator(CurrentChar) then begin
           inc(LineNumber);
           AResult.WasLineEnd:=true;
           if CurrentChar=$000d then begin
            NextChar;
            if CurrentChar=$000a then begin
             NextChar;
            end;
           end else begin
            NextChar;
           end;
           continue;
          end;
         end else begin
          AResult.TokenType:=ltoMINUSMINUS;
         end;
        end else begin
         AResult.TokenType:=ltoMINUSMINUS;
        end;
       end;
       ord('='):begin
        NextChar;
        AResult.TokenType:=ltoMINUSASSIGNMENT;
       end;
       else begin
        AResult.TokenType:=ltoMINUS;
       end;
      end;
     end;
     ord('+'):begin
      NextChar;
      case CurrentChar of
       ord('+'):begin
        NextChar;
        AResult.TokenType:=ltoPLUSPLUS;
       end;
       ord('='):begin
        NextChar;
        AResult.TokenType:=ltoPLUSASSIGNMENT;
       end;
       else begin
        AResult.TokenType:=ltoPLUS;
       end;
      end;
     end;
     ord('%'):begin
      NextChar;
      case CurrentChar of
       ord('='):begin
        NextChar;
        AResult.TokenType:=ltoMODULOASSIGNMENT;
       end;
       else begin
        AResult.TokenType:=ltoMODULO;
       end;
      end;
     end;
     ord('*'):begin
      NextChar;
      case CurrentChar of
       ord('='):begin
        NextChar;
        AResult.TokenType:=ltoMULTIPLYASSIGNMENT;
       end;
       else begin
        AResult.TokenType:=ltoMULTIPLY;
       end;
      end;
     end;
     ord('!'):begin
      NextChar;
      case CurrentChar of
       ord('='):begin
        NextChar;
        case CurrentChar of
         ord('='):begin
          NextChar;
          AResult.TokenType:=ltoNOTEQUALEQUAL;
         end else begin
          AResult.TokenType:=ltoNOTEQUAL;
         end;
        end;
       end;
       else begin
        AResult.TokenType:=ltoLOGICALNOT;
       end;
      end;
     end;
     ord('~'):begin
      NextChar;
      AResult.TokenType:=ltoBITWISENOT;
     end;
     ord('}'):begin
      NextChar;
      AResult.TokenType:=ltoCLOSEBRACE;
     end;
     ord(')'):begin
      NextChar;
      AResult.TokenType:=ltoCLOSEPAREN;
     end;
     ord(']'):begin
      NextChar;
      AResult.TokenType:=ltoCLOSESQUARE;
     end;
     ord('{'):begin
      NextChar;
      AResult.TokenType:=ltoOPENBRACE;
     end;
     ord('('):begin
      NextChar;
      AResult.TokenType:=ltoOPENPAREN;
     end;
     ord('['):begin
      NextChar;
      AResult.TokenType:=ltoOPENSQUARE;
     end;
     ord(':'):begin
      NextChar;
      AResult.TokenType:=ltoCOLON;
     end;
     ord(';'):begin
      NextChar;
      AResult.TokenType:=ltoSEMICOLON;
     end;
     ord(','):begin
      NextChar;
      AResult.TokenType:=ltoCOMMA;
     end;
     ord('?'):begin
      NextChar;
      AResult.TokenType:=ltoCONDITIONAL;
     end;
     ord('.'):begin
      NextChar;
      case CurrentChar of
       ord('0')..ord('9'):begin
        ParseNumber('.',AResult);
       end;
       else begin
        AResult.TokenType:=ltoDOT;
       end;
      end;
     end;
     ord('0')..ord('9'):begin
      ParseNumber(#0,AResult);
     end;
     ord('"'),ord(''''):begin
      AResult.TokenType:=lttSTRING;
      c:=CurrentChar;
      NextChar;
      AResult.StringValue:='';
      while (CurrentChar<>c) and (Position<=length(Source)) do begin
       case CurrentChar of
        $000a,$000d,$2028,$2029:begin
         if (TBESEN(Instance).Compatibility and COMPAT_JS)=0 then begin
          AddError('Unterminated string literal');
          break;
         end else begin
          NextChar;
         end;
        end;
        $fffe,$feff:begin
         AResult.StringValue:=AResult.StringValue+widechar(word(CurrentChar));
         NextChar;
        end;
        ord('\'):begin
         NextChar;
         case CurrentChar of
          $000d:begin
           AResult.StringValue:=AResult.StringValue+#$000d;
           NextChar;
           if CurrentChar=$000a then begin
            AResult.StringValue:=AResult.StringValue+#$000a;
            NextChar;
           end;
           inc(LineNumber);
           AResult.WasLineEnd:=true;
          end;
          $000a,$2028,$2029:begin
           AResult.StringValue:=AResult.StringValue+widechar(word(CurrentChar));
           inc(LineNumber);
           AResult.WasLineEnd:=true;
           break;
          end;
          ord('b'):begin
           NextChar;
           AResult.StringValue:=AResult.StringValue+#$0008;
          end;
          ord('t'):begin
           NextChar;
           AResult.StringValue:=AResult.StringValue+#$0009;
          end;
          ord('n'):begin
           NextChar;
           AResult.StringValue:=AResult.StringValue+#$000a;
          end;
          ord('v'):begin
           NextChar;
           AResult.StringValue:=AResult.StringValue+#$000b;
          end;
          ord('f'):begin
           NextChar;
           AResult.StringValue:=AResult.StringValue+#$000c;
          end;
          ord('r'):begin
           NextChar;
           AResult.StringValue:=AResult.StringValue+#$000d;
          end;
          ord('s'):begin
           NextChar;
           AResult.StringValue:=AResult.StringValue+#$fffe;
          end;
          ord('0')..ord('7'):begin
           if TBESEN(Instance).IsStrict then begin
            AResult.StringValue:=AResult.StringValue+widechar(word(CurrentChar));
            NextChar;
            if (CurrentChar>=ord('0')) and (CurrentChar<=ord('7')) then begin
             raise EBESENSyntaxError.Create('Octals aren''t allowed in strict mode');
            end;
           end else begin
            v:=CurrentChar-ord('0');
            NextChar;
            if (CurrentChar>=ord('0')) and (CurrentChar<=ord('7')) then begin
             v:=(v shl 3) or longword(CurrentChar-ord('0'));
             NextChar;
             if (CurrentChar>=ord('0')) and (CurrentChar<=ord('7')) then begin
              v:=(v shl 3) or longword(CurrentChar-ord('0'));
              NextChar;
             end;
            end;
            AResult.StringValue:=AResult.StringValue+widechar(word(v));
           end;
          end;
          ord('x'):begin
           if ((Position+1)<=length(Source)) and BESENIsHex(word(widechar(Source[Position]))) and BESENIsHex(word(widechar(Source[Position+1]))) then begin
            NextChar;
            v:=0;
            if ((CurrentChar>=ord('0')) and (CurrentChar<=ord('9'))) or ((CurrentChar>=ord('a')) and (CurrentChar<=ord('f'))) or ((CurrentChar>=ord('A')) and (CurrentChar<=ord('F'))) then begin
             case CurrentChar of
              ord('0')..ord('9'):begin
               v:=(v shl 4) or longword(CurrentChar-ord('0'));
              end;
              ord('a')..ord('f'):begin
               v:=(v shl 4) or (longword(CurrentChar-ord('a'))+$a);
              end;
              ord('A')..ord('F'):begin
               v:=(v shl 4) or (longword(CurrentChar-ord('A'))+$a);
              end;
             end;
             NextChar;
             if ((CurrentChar>=ord('0')) and (CurrentChar<=ord('9'))) or ((CurrentChar>=ord('a')) and (CurrentChar<=ord('f'))) or ((CurrentChar>=ord('A')) and (CurrentChar<=ord('F'))) then begin
              case CurrentChar of
               ord('0')..ord('9'):begin
                v:=(v shl 4) or longword(CurrentChar-ord('0'));
               end;
               ord('a')..ord('f'):begin
                v:=(v shl 4) or (longword(CurrentChar-ord('a'))+$a);
               end;
               ord('A')..ord('F'):begin
                v:=(v shl 4) or (longword(CurrentChar-ord('A'))+$a);
               end;
              end;
              NextChar;
             end;
            end;
            AResult.StringValue:=AResult.StringValue+widechar(word(v));
           end else begin
            if (TBESEN(Instance).Compatibility and COMPAT_JS)=0 then begin
             AddError('Invalid escape char x');
            end else begin
             NextChar;
             AResult.StringValue:=AResult.StringValue+'x';
            end;
           end;
          end;
          ord('u'):begin
           if ((Position+3)<=length(Source)) and BESENIsHex(word(widechar(Source[Position]))) and BESENIsHex(word(widechar(Source[Position+1]))) and BESENIsHex(word(widechar(Source[Position+2]))) and BESENIsHex(word(widechar(Source[Position+3]))) then begin
            NextChar;
            v:=0;
            if ((CurrentChar>=ord('0')) and (CurrentChar<=ord('9'))) or ((CurrentChar>=ord('a')) and (CurrentChar<=ord('f'))) or ((CurrentChar>=ord('A')) and (CurrentChar<=ord('F'))) then begin
             case CurrentChar of
              ord('0')..ord('9'):begin
               v:=(v shl 4) or longword(CurrentChar-ord('0'));
              end;
              ord('a')..ord('f'):begin
               v:=(v shl 4) or (longword(CurrentChar-ord('a'))+$a);
              end;
              ord('A')..ord('F'):begin
               v:=(v shl 4) or (longword(CurrentChar-ord('A'))+$a);
              end;
             end;
             NextChar;
             if ((CurrentChar>=ord('0')) and (CurrentChar<=ord('9'))) or ((CurrentChar>=ord('a')) and (CurrentChar<=ord('f'))) or ((CurrentChar>=ord('A')) and (CurrentChar<=ord('F'))) then begin
              case CurrentChar of
               ord('0')..ord('9'):begin
                v:=(v shl 4) or longword(CurrentChar-ord('0'));
               end;
               ord('a')..ord('f'):begin
                v:=(v shl 4) or (longword(CurrentChar-ord('a'))+$a);
               end;
               ord('A')..ord('F'):begin
                v:=(v shl 4) or (longword(CurrentChar-ord('A'))+$a);
               end;
              end;
              NextChar;
              if ((CurrentChar>=ord('0')) and (CurrentChar<=ord('9'))) or ((CurrentChar>=ord('a')) and (CurrentChar<=ord('f'))) or ((CurrentChar>=ord('A')) and (CurrentChar<=ord('F'))) then begin
               case CurrentChar of
                ord('0')..ord('9'):begin
                 v:=(v shl 4) or longword(CurrentChar-ord('0'));
                end;
                ord('a')..ord('f'):begin
                 v:=(v shl 4) or (longword(CurrentChar-ord('a'))+$a);
                end;
                ord('A')..ord('F'):begin
                 v:=(v shl 4) or (longword(CurrentChar-ord('A'))+$a);
                end;
               end;
               NextChar;
               if ((CurrentChar>=ord('0')) and (CurrentChar<=ord('9'))) or ((CurrentChar>=ord('a')) and (CurrentChar<=ord('f'))) or ((CurrentChar>=ord('A')) and (CurrentChar<=ord('F'))) then begin
                case CurrentChar of
                 ord('0')..ord('9'):begin
                  v:=(v shl 4) or longword(CurrentChar-ord('0'));
                 end;
                 ord('a')..ord('f'):begin
                  v:=(v shl 4) or (longword(CurrentChar-ord('a'))+$a);
                 end;
                 ord('A')..ord('F'):begin
                  v:=(v shl 4) or (longword(CurrentChar-ord('A'))+$a);
                 end;
                end;
                NextChar;
               end;
              end;
             end;
            end;
            AResult.StringValue:=AResult.StringValue+widechar(word(v));
           end else begin
            if (TBESEN(Instance).Compatibility and COMPAT_JS)=0 then begin
             AddError('Invalid escape char u');
            end else begin
             NextChar;
             AResult.StringValue:=AResult.StringValue+'u';
            end;
           end;
          end;
          else begin
           AResult.StringValue:=AResult.StringValue+BESENUTF32CHARToUTF16(CurrentChar);
           NextChar;
          end;
         end;
        end;
        else begin
         AResult.StringValue:=AResult.StringValue+BESENUTF32CHARToUTF16(CurrentChar);
         NextChar;
        end;
       end;
      end;
      if CurrentChar=c then begin
       NextChar;
      end else begin
       AddError('Unterminated string literal');
      end;
     end;
     else begin
      if ((TBESEN(Instance).Compatibility and COMPAT_JS)<>0) and (CurrentChar=ord('#')) then begin
       NextChar;
       AResult.Name:='function';
       AResult.TokenType:=ltkFUNCTION;
      end else if BESENUnicodeIsIDStart(CurrentChar) or (CurrentChar=ord('\')) then begin
       AResult.Name:='';
       while BESENUnicodeIsIDPart(CurrentChar) or (CurrentChar=ord('\')) do begin
        if CurrentChar=ord('\') then begin
         NextChar;
         case CurrentChar of
          ord('x'),ord('X'):begin
           NextChar;
           v:=0;
           if ((CurrentChar>=ord('0')) and (CurrentChar<=ord('9'))) or ((CurrentChar>=ord('a')) and (CurrentChar<=ord('f'))) or ((CurrentChar>=ord('A')) and (CurrentChar<=ord('F'))) then begin
            case CurrentChar of
             ord('0')..ord('9'):begin
              v:=(v shl 4) or longword(CurrentChar-ord('0'));
             end;
             ord('a')..ord('f'):begin
              v:=(v shl 4) or (longword(CurrentChar-ord('a'))+$a);
             end;
             ord('A')..ord('F'):begin
              v:=(v shl 4) or (longword(CurrentChar-ord('A'))+$a);
             end;
            end;
            NextChar;
            if ((CurrentChar>=ord('0')) and (CurrentChar<=ord('9'))) or ((CurrentChar>=ord('a')) and (CurrentChar<=ord('f'))) or ((CurrentChar>=ord('A')) and (CurrentChar<=ord('F'))) then begin
             case CurrentChar of
              ord('0')..ord('9'):begin
               v:=(v shl 4) or longword(CurrentChar-ord('0'));
              end;
              ord('a')..ord('f'):begin
               v:=(v shl 4) or (longword(CurrentChar-ord('a'))+$a);
              end;
              ord('A')..ord('F'):begin
               v:=(v shl 4) or (longword(CurrentChar-ord('A'))+$a);
              end;
             end;
             NextChar;
            end;
           end;
           AResult.Name:=AResult.Name+widechar(word(v));
          end;
          ord('u'),ord('U'):begin
           NextChar;
           v:=0;
           if ((CurrentChar>=ord('0')) and (CurrentChar<=ord('9'))) or ((CurrentChar>=ord('a')) and (CurrentChar<=ord('f'))) or ((CurrentChar>=ord('A')) and (CurrentChar<=ord('F'))) then begin
            case CurrentChar of
             ord('0')..ord('9'):begin
              v:=(v shl 4) or longword(CurrentChar-ord('0'));
             end;
             ord('a')..ord('f'):begin
              v:=(v shl 4) or (longword(CurrentChar-ord('a'))+$a);
             end;
             ord('A')..ord('F'):begin
              v:=(v shl 4) or (longword(CurrentChar-ord('A'))+$a);
             end;
            end;
            NextChar;
            if ((CurrentChar>=ord('0')) and (CurrentChar<=ord('9'))) or ((CurrentChar>=ord('a')) and (CurrentChar<=ord('f'))) or ((CurrentChar>=ord('A')) and (CurrentChar<=ord('F'))) then begin
             case CurrentChar of
              ord('0')..ord('9'):begin
               v:=(v shl 4) or longword(CurrentChar-ord('0'));
              end;
              ord('a')..ord('f'):begin
               v:=(v shl 4) or (longword(CurrentChar-ord('a'))+$a);
              end;
              ord('A')..ord('F'):begin
               v:=(v shl 4) or (longword(CurrentChar-ord('A'))+$a);
              end;
             end;
             NextChar;
             if ((CurrentChar>=ord('0')) and (CurrentChar<=ord('9'))) or ((CurrentChar>=ord('a')) and (CurrentChar<=ord('f'))) or ((CurrentChar>=ord('A')) and (CurrentChar<=ord('F'))) then begin
              case CurrentChar of
               ord('0')..ord('9'):begin
                v:=(v shl 4) or longword(CurrentChar-ord('0'));
               end;
               ord('a')..ord('f'):begin
                v:=(v shl 4) or (longword(CurrentChar-ord('a'))+$a);
               end;
               ord('A')..ord('F'):begin
                v:=(v shl 4) or (longword(CurrentChar-ord('A'))+$a);
               end;
              end;
              NextChar;
              if ((CurrentChar>=ord('0')) and (CurrentChar<=ord('9'))) or ((CurrentChar>=ord('a')) and (CurrentChar<=ord('f'))) or ((CurrentChar>=ord('A')) and (CurrentChar<=ord('F'))) then begin
               case CurrentChar of
                ord('0')..ord('9'):begin
                 v:=(v shl 4) or longword(CurrentChar-ord('0'));
                end;
                ord('a')..ord('f'):begin
                 v:=(v shl 4) or (longword(CurrentChar-ord('a'))+$a);
                end;
                ord('A')..ord('F'):begin
                 v:=(v shl 4) or (longword(CurrentChar-ord('A'))+$a);
                end;
               end;
               NextChar;
              end;
             end;
            end;
           end;
           AResult.Name:=AResult.Name+widechar(word(v));
          end;
          else begin
           AResult.Name:=AResult.Name+'\'+BESENUTF32CHARToUTF16(CurrentChar);
           NextChar;
          end;
         end;
        end else begin
         AResult.Name:=AResult.Name+BESENUTF32CHARToUTF16(CurrentChar);
         NextChar;
        end;
       end;
       AResult.TokenType:=FindKeyword(AResult.Name);
       if AResult.TokenType in BESENLexerStrictReservedTokens then begin
        if TBESEN(Instance).IsStrict then begin
         raise EBESENSyntaxError.Create('"'+BESENUTF16ToUTF8(AResult.Name)+'" is a reserved keyword');
        end else begin
         AResult.TokenType:=lttIDENTIFIER;
        end;
       end;
      end;
     end;
    end;
   end;
   break;
  end;
  AResult.LineEnd:=BESENUnicodeIsLineTerminator(CurrentChar);
  LastWasLineEnd:=AResult.LineEnd;
 end;
end;

procedure InitKeywords;
var kt:TKeywordToken;
    i:integer;
    k:TKeyword;
begin
 SetLength(Keywords,(integer(high(TKeywordToken))-integer(low(TKeywordToken)))+1);
 i:=0;
 for kt:=low(TKeywordToken) to high(TKeywordToken) do begin
  Keywords[i]:=kt;
  inc(i);
 end;
 i:=0;
 while (i+1)<length(Keywords) do begin
  if KeywordNames[Keywords[i]]>KeywordNames[Keywords[i+1]] then begin
   k:=Keywords[i];
   Keywords[i]:=Keywords[i+1];
   Keywords[i+1]:=k;
   if i>0 then begin
    dec(i);
   end else begin
    inc(i);
   end;
  end else begin
   inc(i);
  end;
 end;
end;

procedure DoneKeywords;
begin
 SetLength(Keywords,0);
end;

procedure InitBESEN;
begin
 InitKeywords;
end;

procedure DoneBESEN;
begin
 DoneKeywords;
end;

initialization
 InitBESEN;
finalization
 DoneBESEN;
end.
