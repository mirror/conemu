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
unit BESENRegExp;
{$i BESEN.inc}

interface

uses SysUtils,BESENConstants,BESENTypes,BESENValue,BESENCollectorObject;

const breoFAIL=0;     // match failed
      breoSUCCEED=1;  // match succeeded
      breoCHAR=2;     // match a char class instance
      breoZERO=3;     // reset counter
      breoREACH=4;    // test counter over
      breoNREACH=5;   // test counter under
      breoSTART=6;    // enter a group
      breoEND=7;      // exit a group
      breoUNDEF=8;    // reset a group
      breoMARK=9;     // record a position
      breoFDIST=10;   // position test
      breoRDIST=11;   // position and counter test
      breoMNEXT=12;   // max-loop
      breoRNEXT=13;   // reach-loop
      breoGOTO=14;    // Branch
      breoGS=15;      // greeedy success
      breoNS=16;      // non-greeedy success
      breoGF=17;      // greedy fail
      breoNF=18;      // non-greeedy fail
      breoAS=19;      // assert success
      breoAF=20;      // assert fail
      breoBOL=21;     // test begin of a line
      breoEOL=22;     // test end of a line
      breoBRK=23;     // test work-break
      breoNBRK=24;    // test non-work-break
      breoBACKREF=25; // backreference match

      brecUNDEFINED=longword($ffffffff);
      brecINFINITY=-1;

      bresADDR=2;
      bresINT=2;

      breMAXSTATESHOLDINMEMORY:integer=16;

      BESENRegExpCacheSize:integer=256;

type TBESENRegExpOpcode=byte;

     TBESENRegExpFlag=(brefGLOBAL,brefIGNORECASE,brefMULTILINE);

     TBESENRegExpFlags=set of TBESENRegExpFlag;

     TBESENRegExpCharClass=class;

     TBESENRegExpCharClassRange=class
      public
       CharClass:TBESENRegExpCharClass;
       Previous,Next:TBESENRegExpCharClassRange;
       Lo,Hi:TBESENUTF32CHAR;
       constructor Create(ACharClass:TBESENRegExpCharClass;ALo,AHi:TBESENUTF32CHAR);
       constructor CreateBefore(ACharClass:TBESENRegExpCharClass;ABefore:TBESENRegExpCharClassRange;ALo,AHi:TBESENUTF32CHAR);
       constructor CreateAfter(ACharClass:TBESENRegExpCharClass;AAfter:TBESENRegExpCharClassRange;ALo,AHi:TBESENUTF32CHAR);
       destructor Destroy; override;
     end;

     TBESENRegExp=class;

     TBESENRegExpCharClass=class
      public
       RegEx:TBESENRegExp;
       Previous,Next:TBESENRegExpCharClass;
       First,Last:TBESENRegExpCharClassRange;
       Inverted:longbool;
       Canonicalized:longbool;
       constructor Create(ARegEx:TBESENRegExp);
       destructor Destroy; override;
       procedure Clear;
       procedure Dump;
       procedure Optimize;
       procedure AddRange(Lo,Hi:TBESENUTF32CHAR;IgnoreCase:boolean=false);
       procedure AddChar(c:TBESENUTF32CHAR;IgnoreCase:boolean=false);
       procedure TakeoverCombine(From:TBESENRegExpCharClass);
       procedure Assign(From:TBESENRegExpCharClass);
       procedure Append(From:TBESENRegExpCharClass);
       procedure Invert;
       procedure Canonicalize;
       function Count:longword;
       function Contains(c:TBESENUTF32CHAR):boolean;
       function IsSingle:boolean;
     end;

     TBESENRegExpCharClasses=array of TBESENRegExpCharClass;

     TBESENRegExpCapture=record
      s,e:longword;
     end;

     TBESENRegExpCaptures=array of TBESENRegExpCapture;

     TBESENRegExpValue=integer;

     TBESENRegExpValues=array of TBESENRegExpValue;

     TBESENRegExpState=class
      public
       RegEx:TBESENRegExp;
       Previous,Next:TBESENRegExpState;
       Captures:TBESENRegExpCaptures;
       Counters:TBESENRegExpValues;
       Marks:TBESENRegExpValues;
       constructor Create(ARegEx:TBESENRegExp);
       destructor Destroy; override;
     end;

     TBESENRegExp=class(TBESENCollectorObject)
      private
       FirstCharClass,LastCharClass:TBESENRegExpCharClass;
       FirstState,LastState,CurrentState:TBESENRegExpState;
       CountStates:integer;
       function InternCharClass(c:TBESENRegExpCharClass):integer;
       function CodePos:integer;
       procedure CodePosSet(p:integer);
       procedure CodeAdd(c:integer);
       procedure CodePatch(p,c:integer);
       procedure CodeInsert(p,n:integer);
       procedure CodeAddI(c:integer);
       procedure CodeAddA(c:integer);
       procedure CodePatchI(p,c:integer);
       procedure CodePatchA(p,c:integer);
       function AllocateState:TBESENRegExpState;
       procedure CleanupStates;
       procedure CopyState(const a,b:TBESENRegExpState);
       function DisassembleOpcode(PC:integer):TBESENString;
       function Execute(PC:integer;const Input:TBESENString;var State:TBESENRegExpState):boolean;
      public
       Source:TBESENString;
       ByteCode:TBESENBytes;
       ByteCodeLen:integer;
       Flags:TBESENRegExpFlags;
       CountOfCaptures:integer;
       CountOfCounters:integer;
       CountOfMarks:integer;
       MaxRef:integer;
       CharClasses:TBESENRegExpCharClasses;
       CharClassesCount:integer;
       ReferenceCounter:integer;
       constructor Create(AInstance:TObject); override;
       destructor Destroy; override;
       procedure IncRef;
       procedure DecRef;
       procedure Compile(const ASource:TBESENString;const AFlags:TBESENRegExpFlags=[]);
       function Match(const Input:TBESENString;Index:integer;var Captures:TBESENRegExpCaptures):boolean;
       function Disassemble:TBESENString;
     end;

implementation

uses BESEN,BESENUtils,BESENStringUtils,BESENErrors;

constructor TBESENRegExpCharClassRange.Create(ACharClass:TBESENRegExpCharClass;ALo,AHi:TBESENUTF32CHAR);
begin
 inherited Create;
 CharClass:=ACharClass;
 Lo:=ALo;
 Hi:=AHi;
 if assigned(CharClass.Last) then begin
  Previous:=CharClass.Last;
  CharClass.Last:=self;
  Previous.Next:=self;
  Next:=nil;
 end else begin
  CharClass.First:=self;
  CharClass.Last:=self;
  Previous:=nil;
  Next:=nil;
 end;
end;

constructor TBESENRegExpCharClassRange.CreateBefore(ACharClass:TBESENRegExpCharClass;ABefore:TBESENRegExpCharClassRange;ALo,AHi:TBESENUTF32CHAR);
begin
 inherited Create;
 CharClass:=ACharClass;
 Lo:=ALo;
 Hi:=AHi;
 Previous:=ABefore.Previous;
 Next:=ABefore;
 ABefore.Previous:=self;
 if assigned(Previous) then begin
  Previous.Next:=self;
 end else begin
  CharClass.First:=self;
 end;
end;

constructor TBESENRegExpCharClassRange.CreateAfter(ACharClass:TBESENRegExpCharClass;AAfter:TBESENRegExpCharClassRange;ALo,AHi:TBESENUTF32CHAR);
begin
 inherited Create;
 CharClass:=ACharClass;
 Lo:=ALo;
 Hi:=AHi;
 Previous:=AAfter;
 Next:=AAfter.Next;
 AAfter.Next:=self;
 if assigned(Next) then begin
  Next.Previous:=self;
 end else begin
  CharClass.Last:=self;
 end;
end;

destructor TBESENRegExpCharClassRange.Destroy;
begin
 if assigned(Previous) then begin
  Previous.Next:=Next;
 end else if CharClass.First=self then begin
  CharClass.First:=Next;
 end;
 if assigned(Next) then begin
  Next.Previous:=Previous;
 end else if CharClass.Last=self then begin
  CharClass.Last:=Previous;
 end;
 Previous:=nil;
 Next:=nil;
 inherited Destroy;
end;

constructor TBESENRegExpCharClass.Create(ARegEx:TBESENRegExp);
begin
 inherited Create;
 RegEx:=ARegEx;
 if assigned(RegEx.LastCharClass) then begin
  Previous:=RegEx.LastCharClass;
  RegEx.LastCharClass:=self;
  Previous.Next:=self;
  Next:=nil;
 end else begin
  RegEx.FirstCharClass:=self;
  RegEx.LastCharClass:=self;
  Previous:=nil;
  Next:=nil;
 end;
 First:=nil;
 Last:=nil;
 Inverted:=false;
 Canonicalized:=false;
end;

destructor TBESENRegExpCharClass.Destroy;
begin
 while assigned(First) do begin
  First.Destroy;
 end;
 if assigned(Previous) then begin
  Previous.Next:=Next;
 end else if RegEx.FirstCharClass=self then begin
  RegEx.FirstCharClass:=Next;
 end;
 if assigned(Next) then begin
  Next.Previous:=Previous;
 end else if RegEx.LastCharClass=self then begin
  RegEx.LastCharClass:=Previous;
 end;
 Previous:=nil;
 Next:=nil;
 inherited Destroy;
end;

procedure TBESENRegExpCharClass.Clear;
begin
 while assigned(First) do begin
  First.Destroy;
 end;
 Inverted:=false;
 Canonicalized:=false;
end;

procedure TBESENRegExpCharClass.Dump;
var Range:TBESENRegExpCharClassRange;
begin
 Range:=First;
 while assigned(Range) do begin
  writeln(Range.Lo:8,' ',Range.Hi:8);
  Range:=Range.Next;
 end;
end;

procedure TBESENRegExpCharClass.Optimize;
var Range:TBESENRegExpCharClassRange;
begin
 Range:=First;
 while assigned(Range) do begin
  if assigned(Range.Previous) and (((Range.Previous.Hi>=Range.Lo) or ((Range.Previous.Hi+1)=Range.Lo))) then begin
   if Range.Lo>Range.Previous.Lo then begin
    Range.Lo:=Range.Previous.Lo;
   end;
   if Range.Hi<Range.Previous.Hi then begin
    Range.Hi:=Range.Previous.Hi;
   end;
   Range.Previous.Destroy;
   if assigned(Range.Previous) then begin
    Range:=Range.Previous;
   end;
  end else if assigned(Range.Next) and (((Range.Hi>=Range.Next.Lo) or ((Range.Hi+1)=Range.Next.Lo))) then begin
   if Range.Lo>Range.Next.Lo then begin
    Range.Lo:=Range.Next.Lo;
   end;
   if Range.Hi<Range.Next.Hi then begin
    Range.Hi:=Range.Next.Hi;
   end;
   Range.Next.Destroy;
   if assigned(Range.Previous) then begin
    Range:=Range.Previous;
   end;
  end else begin
   Range:=Range.Next;
  end;
 end;
end;

procedure TBESENRegExpCharClass.AddRange(Lo,Hi:TBESENUTF32CHAR;IgnoreCase:boolean=false);
var Range:TBESENRegExpCharClassRange;
    c,cl,cu:TBESENUTF32CHAR;
    NeedToCanonicalize:boolean;
begin
 if IgnoreCase then begin
  NeedToCanonicalize:=false;
  for c:=Lo to Hi do begin
   cl:=BESENUnicodeToLower(c);
   cu:=BESENUnicodeToUpper(c);
   if (cl<>cu) or (cl<>c) or (cu<>c) then begin
    NeedToCanonicalize:=true;
    break;
   end;
  end;
  if NeedToCanonicalize then begin
   for c:=Lo to Hi do begin
    cl:=BESENUnicodeToLower(c);
    cu:=BESENUnicodeToUpper(c);
    if (cl=cu) and (cl=c) then begin
     AddRange(c,c,false);
    end else begin
     AddRange(cl,cl,false);
     AddRange(cu,cu,false);
     if (cl<>c) and (cu<>c) then begin
      AddRange(c,c,false);
     end;
    end;
   end;
  end else begin
   AddRange(Lo,Hi,false);
  end;
 end else begin
  Range:=First;
  while assigned(Range) do begin
   if (Lo>=Range.Lo) and (Hi<=Range.Hi) then begin
    exit;
   end else if (Lo<=Range.Lo) or ((Lo=Range.Lo) and (Hi<=Range.Hi)) then begin
    break;
   end;
   Range:=Range.Next;
  end;
  if assigned(Range) then begin
   TBESENRegExpCharClassRange.CreateBefore(self,Range,Lo,Hi);
  end else begin
   TBESENRegExpCharClassRange.Create(self,Lo,Hi);
  end;
  Optimize;
 end;
end;

procedure TBESENRegExpCharClass.AddChar(c:TBESENUTF32CHAR;IgnoreCase:boolean=false);
begin
 AddRange(c,c,IgnoreCase);
end;

procedure TBESENRegExpCharClass.TakeoverCombine(From:TBESENRegExpCharClass);
var Range:TBESENRegExpCharClassRange;
begin
 if assigned(From) then begin
  if assigned(First) then begin
   Canonicalized:=Canonicalized and From.Canonicalized;
  end else begin
   Canonicalized:=From.Canonicalized;
  end;
  Range:=From.First;
  while assigned(Range) do begin
   Range.CharClass:=self;
   Range:=Range.Next;
  end;
  if assigned(Last) then begin
   Last.Next:=From.First;
   From.First.Previous:=Last;
   Last:=From.Last;
  end else begin
   First:=From.First;
   Last:=From.Last;
  end;
  From.First:=nil;
  From.Last:=nil;
 end;
end;

procedure TBESENRegExpCharClass.Assign(From:TBESENRegExpCharClass);
var Range:TBESENRegExpCharClassRange;
begin
 if assigned(From) then begin
  while assigned(First) do begin
   First.Destroy;
  end;
  Inverted:=From.Inverted;
  Canonicalized:=From.Canonicalized;
  Range:=From.First;
  while assigned(Range) do begin
   Range.CharClass:=self;
   Range:=Range.Next;
  end;
  First:=From.First;
  Last:=From.Last;
  From.First:=nil;
  From.Last:=nil;
 end;
end;

procedure TBESENRegExpCharClass.Append(From:TBESENRegExpCharClass);
var Range:TBESENRegExpCharClassRange;
begin
 if assigned(From) then begin
  Range:=From.First;
  while assigned(Range) do begin
   TBESENRegExpCharClassRange.Create(self,Range.Lo,Range.Hi);
   Range:=Range.Next;
  end;
 end;
end;

procedure TBESENRegExpCharClass.Invert;
var NewList:TBESENRegExpCharClass;
    Range:TBESENRegExpCharClassRange;
    Lo,Hi:TBESENUTF32CHAR;
begin
 Optimize;
 Inverted:=not Inverted;
 if assigned(First) and (First=Last) and (First.Lo=0) and (First.Hi=$ffffffff) then begin
  First.Destroy;
 end else if not assigned(First) then begin
  TBESENRegExpCharClassRange.Create(self,0,$ffffffff);
 end else begin
  NewList:=TBESENRegExpCharClass.Create(RegEx);
  try
   Range:=First;
   if Range.Lo>0 then begin
    TBESENRegExpCharClassRange.Create(NewList,0,Range.Lo-1);
   end;
   Lo:=Range.Hi;
   Range:=Range.Next;
   while assigned(Range) do begin
    if (Lo+1)<Range.Lo then begin
     Hi:=Range.Lo;
     TBESENRegExpCharClassRange.Create(NewList,Lo+1,Hi-1);
    end;
    Lo:=Range.Hi;
    Range:=Range.Next;
   end;
   if Lo<>$ffffffff then begin
    TBESENRegExpCharClassRange.Create(NewList,Lo+1,$ffffffff);
   end;
   while assigned(First) do begin
    First.Destroy;
   end;
   Range:=NewList.First;
   while assigned(Range) do begin
    Range.CharClass:=self;
    Range:=Range.Next;
   end;
   First:=NewList.First;
   Last:=NewList.Last;
   NewList.First:=nil;
   NewList.Last:=nil;
   Range:=First;
   while assigned(Range) do begin
    Range.CharClass:=self;
    Range:=Range.Next;
   end;
  finally
   NewList.Destroy;
  end;
 end;
end;

procedure TBESENRegExpCharClass.Canonicalize;
var NewList:TBESENRegExpCharClass;
    Range:TBESENRegExpCharClassRange;
    OldInverted:boolean;
begin
 if not Canonicalized then begin
  NewList:=TBESENRegExpCharClass.Create(RegEx);
  try
   OldInverted:=Inverted;
   if Inverted then begin
    Invert;
   end;
   Range:=First;
   while assigned(Range) do begin
    NewList.AddRange(Range.Lo,Range.Hi,true);
    Range:=Range.Next;
   end;
   while assigned(First) do begin
    First.Destroy;
   end;
   First:=NewList.First;
   Last:=NewList.Last;
   NewList.First:=nil;
   NewList.Last:=nil;
   Range:=First;
   while assigned(Range) do begin
    Range.CharClass:=self;
    Range:=Range.Next;
   end;
   if OldInverted then begin
    Invert;
   end;
   Inverted:=OldInverted;
  finally
   NewList.Destroy;
  end;
  Canonicalized:=true;
 end;
end;

function TBESENRegExpCharClass.Count:longword;
var Range:TBESENRegExpCharClassRange;
begin
 result:=0;
 Range:=First;
 while assigned(Range) do begin
  inc(result,(Range.Hi-Range.Lo)+1);
  Range:=Range.Next;
 end;
end;

function TBESENRegExpCharClass.Contains(c:TBESENUTF32CHAR):boolean;
var Range:TBESENRegExpCharClassRange;
begin
 result:=false;
 Range:=First;
 while assigned(Range) do begin
  if (c>=Range.Lo) and (c<=Range.Hi) then begin
   result:=true;
   break;
  end;
  Range:=Range.Next;
 end;
end;

function TBESENRegExpCharClass.IsSingle:boolean;
begin
 result:=(First=Last) and ((assigned(First) and (First.Lo=First.Hi)) or not assigned(First));
end;

function CompareCharClasses(c1,c2:TBESENRegExpCharClass):integer;
var r1,r2:TBESENRegExpCharClassRange;
begin
 r1:=c1.First;
 r2:=c2.First;
 while assigned(r1) and assigned(r2) do begin
  if r1.Lo<>r2.Lo then begin
   result:=longint(r1.Lo)-longint(r2.Lo);
   exit;
  end;
  if r1.Hi<>r2.Hi then begin
   result:=longint(r1.Hi)-longint(r2.Hi);
   exit;
  end;
  r1:=r1.Next;
  r2:=r2.Next;
 end;
 if assigned(r1) then begin
  result:=1;
 end else if assigned(r2) then begin
  result:=-1;
 end else begin
  result:=0;
 end;
end;

constructor TBESENRegExpState.Create(ARegEx:TBESENRegExp);
begin
 inherited Create;
 RegEx:=ARegEx;
 if assigned(RegEx.LastState) then begin
  Previous:=RegEx.LastState;
  RegEx.LastState:=self;
  Previous.Next:=self;
  Next:=nil;
 end else begin
  RegEx.FirstState:=self;
  RegEx.LastState:=self;
  Previous:=nil;
  Next:=nil;
 end;
 Captures:=nil;
 Counters:=nil;
 Marks:=nil;
 SetLength(Captures,RegEx.CountOfCaptures);
 SetLength(Counters,RegEx.CountOfCounters);
 SetLength(Marks,RegEx.CountOfMarks);
end;

destructor TBESENRegExpState.Destroy;
begin
 SetLength(Captures,0);
 SetLength(Counters,0);
 SetLength(Marks,0);
 if assigned(Previous) then begin
  Previous.Next:=Next;
 end else if RegEx.FirstState=self then begin
  RegEx.FirstState:=Next;
 end;
 if assigned(Next) then begin
  Next.Previous:=Previous;
 end else if RegEx.LastState=self then begin
  RegEx.LastState:=Previous;
 end;
 Previous:=nil;
 Next:=nil;
 inherited Destroy;
end;

constructor TBESENRegExp.Create(AInstance:TObject);
begin
 inherited Create(AInstance);
 FirstCharClass:=nil;
 LastCharClass:=nil;
 FirstState:=nil;
 LastState:=nil;
 CurrentState:=nil;
 CountStates:=0;
 Source:='';
 ByteCode:=nil;
 ByteCodeLen:=0;
 Flags:=[];
 CountOfCaptures:=0;
 CountOfCounters:=0;
 CountOfMarks:=0;
 MaxRef:=0;
 CharClasses:=nil;
 CharClassesCount:=0;
 ReferenceCounter:=0;
end;

destructor TBESENRegExp.Destroy;
begin
 SetLength(CharClasses,0);
 while assigned(FirstCharClass) do begin
  FirstCharClass.Destroy;
 end;
 while assigned(FirstState) do begin
  FirstState.Destroy;
 end;
 Source:='';
 SetLength(ByteCode,0);
 inherited Destroy;
end;

procedure TBESENRegExp.IncRef;
begin
 inc(ReferenceCounter);
end;

procedure TBESENRegExp.DecRef;
begin
 if ReferenceCounter>1 then begin
  dec(ReferenceCounter);
 end else begin
  ReferenceCounter:=0;
  Destroy;
 end;
end;

function TBESENRegExp.InternCharClass(c:TBESENRegExpCharClass):integer;
var i:integer;
begin
 for i:=0 to CharClassesCount-1 do begin
  if CompareCharClasses(c,CharClasses[i])=0 then begin
   result:=i;
   exit;
  end;
 end;
 result:=CharClassesCount;
 inc(CharClassesCount);
 if CharClassesCount>=length(CharClasses) then begin
  SetLength(CharClasses,(CharClassesCount+256) and not 255);
 end;
 CharClasses[result]:=c;
end;

function TBESENRegExp.CodePos:integer;
begin
 result:=ByteCodeLen;
end;

procedure TBESENRegExp.CodePosSet(p:integer);
begin
 ByteCodeLen:=p;
 if ByteCodeLen>=length(ByteCode) then begin
  SetLength(ByteCode,(ByteCodeLen+4097) and 4095);
 end;
end;

procedure TBESENRegExp.CodeAdd(c:integer);
var i:integer;
begin
 i:=ByteCodeLen;
 inc(ByteCodeLen);
 if ByteCodeLen>=length(ByteCode) then begin
  SetLength(ByteCode,(ByteCodeLen+4097) and 4095);
 end;
 ByteCode[i]:=c;
end;

procedure TBESENRegExp.CodePatch(p,c:integer);
var i:integer;
begin
 i:=p;
 if ByteCodeLen>=length(ByteCode) then begin
  SetLength(ByteCode,(ByteCodeLen+4097) and 4095);
 end;
 ByteCode[i]:=c;
end;

procedure TBESENRegExp.CodeInsert(p,n:integer);
var i,j:integer;
begin
 for i:=1 to n do begin
  CodeAdd(0);
 end;
 for i:=ByteCodeLen-n downto p+1 do begin
  j:=i-1;
  ByteCode[j+n]:=ByteCode[j];
 end;
end;

procedure TBESENRegExp.CodeAddI(c:integer);
begin
 CodeAdd((c shr 8) and $ff);
 CodeAdd(c and $ff);
end;

procedure TBESENRegExp.CodeAddA(c:integer);
begin
 CodeAddI(c-CodePos);
end;

procedure TBESENRegExp.CodePatchI(p,c:integer);
begin
 CodePatch(p,(c shr 8) and $ff);
 CodePatch(p+1,c and $ff);
end;

procedure TBESENRegExp.CodePatchA(p,c:integer);
begin
 CodePatchI(p,c-p);
end;

function TBESENRegExp.AllocateState:TBESENRegExpState;
begin
 if assigned(CurrentState) then begin
  if assigned(CurrentState.Next) then begin
   CurrentState:=CurrentState.Next;
  end else begin
   CurrentState:=TBESENRegExpState.Create(self);
   inc(CountStates);
  end;
 end else begin
  if assigned(FirstState) then begin
   CurrentState:=FirstState;
  end else begin
   CurrentState:=TBESENRegExpState.Create(self);
   inc(CountStates);
  end;
 end;
 result:=CurrentState;
end;

procedure TBESENRegExp.CleanupStates;
begin
 while assigned(LastState) and (CountStates>TBESEN(Instance).RegExpMaxStatesHoldInMemory) do begin
  LastState.Destroy;
  dec(CountStates);
 end;
 CurrentState:=nil;
end;

procedure TBESENRegExp.CopyState(const a,b:TBESENRegExpState);
begin
 if CountOfCaptures>0 then begin
  move(b.Captures[0],a.Captures[0],CountOfCaptures*sizeof(TBESENRegExpCapture));
 end;
 if CountOfCounters>0 then begin
  move(b.Counters[0],a.Counters[0],CountOfCounters*sizeof(TBESENRegExpValue));
 end;
 if CountOfMarks>0 then begin
  move(b.Marks[0],a.Marks[0],CountOfMarks*sizeof(TBESENRegExpValue));
 end;
end;

procedure TBESENRegExp.Compile(const ASource:TBESENString;const AFlags:TBESENRegExpFlags=[]);
var AtEOF:boolean;
    CurrentChar:TBESENUTF32CHAR;
    UTF32Source:TBESENUTF32STRING;
    SourcePos:integer;
 function NextChar:TBESENUTF32CHAR;
 begin
  if (SourcePos+1)<length(UTF32Source) then begin
   inc(SourcePos);
   CurrentChar:=UTF32Source[SourcePos];
  end else begin
   CurrentChar:=0;
   AtEOF:=true;
  end;
  result:=CurrentChar;
 end;
 procedure SyntaxError;
 begin
  raise EBESENSyntaxError.Create('regex syntax error');
 end;
 procedure Expect(c:TBESENUTF32CHAR);
 begin
  if AtEOF or (CurrentChar<>c) then begin
   SyntaxError;
  end;
  NextChar;
 end;
 function ParseInteger:integer;
 var OK:boolean;
 begin
  result:=0;
  OK:=false;
  while (not AtEOF) and (CurrentChar>=ord('0')) and (CurrentChar<=ord('9')) do begin
   result:=(result*10)+integer(CurrentChar-ord('0'));
   OK:=true;
   NextChar;
  end;
  if not OK then begin
   SyntaxError;
  end;
 end;
 function ParseHex:longword;
 begin
  result:=0;
  if AtEOF then begin
   SyntaxError;
  end else if (CurrentChar>=ord('0')) and (CurrentChar<=ord('9')) then begin
   result:=CurrentChar-ord('0');
   NextChar;
  end else if (CurrentChar>=ord('a')) and (CurrentChar<=ord('f')) then begin
   result:=CurrentChar-ord('a')+$a;
   NextChar;
  end else if (CurrentChar>=ord('A')) and (CurrentChar<=ord('F')) then begin
   result:=CurrentChar-ord('A')+$a;
   NextChar;
  end else begin
   SyntaxError;
  end;
 end;
 procedure ParseDisjunction; forward;
 function ParseClassEscape(CanBeAlreadyCanonicalized:boolean):TBESENRegExpCharClass;
 var i:integer;
     ch:TBESENUTF32CHAR;
     IgnoreCase:boolean;
 begin
  result:=nil;
  try
   IgnoreCase:=CanBeAlreadyCanonicalized and (brefIGNORECASE in Flags);
   result:=TBESENRegExpCharClass.Create(self);
   if (CurrentChar>=ord('0')) and (CurrentChar<=ord('9')) then begin
    if ((TBESEN(Instance).Compatibility and COMPAT_JS)<>0) and
       (CurrentChar=ord('0')) and ((SourcePos+2)<length(UTF32Source)) and ((UTF32Source[SourcePos+1]>=ord('0')) and (UTF32Source[SourcePos+1]<=ord('7'))) and
       ((UTF32Source[SourcePos+2]>=ord('0')) and (UTF32Source[SourcePos+2]<=ord('7'))) then begin
     result.AddChar(((UTF32Source[SourcePos+1]-ord('0'))*8)+(UTF32Source[SourcePos+2]-ord('0')),IgnoreCase);
     result.Canonicalized:=IgnoreCase;
     NextChar;
     NextChar;
     NextChar;
     exit;
    end;
    i:=0;
    while (not AtEOF) and (CurrentChar>=ord('0')) and (CurrentChar<=ord('9')) do begin
     i:=(i*10)+integer(CurrentChar-ord('0'));
     NextChar;
    end;
    if i<>0 then begin
     SyntaxError;
    end;
    result.AddChar(i,IgnoreCase);
    result.Canonicalized:=IgnoreCase;
    exit;
   end;
   ch:=CurrentChar;
   NextChar;
   case ch of
    ord('b'):begin
     result.AddChar($0008);
     result.Canonicalized:=true;
    end;
    ord('t'):begin
     result.AddChar($0009);
     result.Canonicalized:=true;
    end;
    ord('n'):begin
     result.AddChar($000a);
     result.Canonicalized:=true;
    end;
    ord('v'):begin
     result.AddChar($000b);
     result.Canonicalized:=true;
    end;
    ord('f'):begin
     result.AddChar($000c);
     result.Canonicalized:=true;
    end;
    ord('r'):begin
     result.AddChar($000d);
     result.Canonicalized:=true;
    end;
    ord('d'):begin
     result.AddRange(ord('0'),ord('9'));
     result.Canonicalized:=true;
    end;
    ord('D'):begin
     result.AddRange(ord('0'),ord('9'));
     result.Invert;
     result.Inverted:=false;
     result.Canonicalized:=true;
    end;
    ord('w'):begin
     result.AddRange(ord('a'),ord('z'));
     result.AddRange(ord('A'),ord('Z'));
     result.AddRange(ord('0'),ord('9'));
     result.AddChar(ord('_'));
     result.Canonicalized:=true;
    end;
    ord('W'):begin
     result.AddRange(ord('a'),ord('z'));
     result.AddRange(ord('A'),ord('Z'));
     result.AddRange(ord('0'),ord('9'));
     result.AddChar(ord('_'));
     result.Invert;
     result.Inverted:=false;
     result.Canonicalized:=true;
    end;
    ord('s'):begin
     result.AddRange($0009,$000d);
     result.AddChar($0020);
     result.AddChar($00a0);
     result.AddChar($1680);
     result.AddChar($180e);
     result.AddRange($2000,$200b);
     result.AddRange($2028,$2029);
     result.AddChar($202f);
     result.AddChar($205f);
     result.AddChar($3000);
     result.AddChar($fffe);
     result.AddChar($feff);
     result.Canonicalized:=true;
    end;
    ord('S'):begin
     result.AddRange($0009,$000d);
     result.AddChar($0020);
     result.AddChar($00a0);
     result.AddChar($1680);
     result.AddChar($180e);
     result.AddRange($2000,$200b);
     result.AddRange($2028,$2029);
     result.AddChar($202f);
     result.AddChar($205f);
     result.AddChar($3000);
     result.AddChar($fffe);
     result.AddChar($feff);
     result.Invert;
     result.Inverted:=false;
     result.Canonicalized:=true;
    end;
    ord('c'):begin
     if AtEOF then begin
      SyntaxError;
     end;
     ch:=CurrentChar;
     NextChar;
     if ((ch>=ord('a')) and (ch<=ord('z'))) or ((ch>=ord('A')) and (ch<=ord('Z'))) then begin
      result.AddChar(ch mod 32,IgnoreCase);
      result.Canonicalized:=IgnoreCase;
     end else begin
      SyntaxError;
     end;
    end;
    ord('x'):begin
     ch:=ParseHex;
     ch:=(ch shl 4) or ParseHex;
     result.AddChar(ch,IgnoreCase);
     result.Canonicalized:=IgnoreCase;
    end;
    ord('u'):begin
     ch:=ParseHex;
     ch:=(ch shl 4) or ParseHex;
     ch:=(ch shl 4) or ParseHex;
     ch:=(ch shl 4) or ParseHex;
     result.AddChar(ch,IgnoreCase);
     result.Canonicalized:=IgnoreCase;
    end;
    else begin
     result.AddChar(ch,IgnoreCase);
     result.Canonicalized:=IgnoreCase;
    end;
   end;
  except
   BESENFreeAndNil(result);
   raise;
  end;
 end;
 function ParseClassAtom:TBESENRegExpCharClass;
 begin
  if AtEOF then begin
   SyntaxError;
  end;
  result:=nil;
  try
   if CurrentChar=ord('\') then begin
    NextChar;
    result:=ParseClassEscape(false);
   end else begin
    result:=TBESENRegExpCharClass.Create(self);
    result.AddChar(CurrentChar);
    NextChar;
   end;
  except
   BESENFreeAndNil(result);
   raise;
  end;
 end;
 function ParseCharacterClass:TBESENRegExpCharClass;
 var InvertFlag:boolean;
     a,b:TBESENRegExpCharClass;
 begin
  result:=nil;
  try
   a:=nil;
   b:=nil;
   try
    result:=TBESENRegExpCharClass.Create(self);
    Expect(ord('['));
    InvertFlag:=(not AtEOF) and (CurrentChar=ord('^'));
    if InvertFlag then begin
     NextChar;
    end;
    while (not AtEOF) and (CurrentChar<>ord(']')) do begin
     a:=ParseClassAtom;
     if (not AtEOF) and (CurrentChar=ord('-')) then begin
      NextChar;
      if (not AtEOF) and (CurrentChar=ord(']')) then begin
       a.AddChar(ord('-'));
      end else begin
       if not a.IsSingle then begin
        BESENFreeAndNil(a);
        SyntaxError;
       end;
       b:=ParseClassAtom;
       if (not b.IsSingle) or ((assigned(a.Last) and assigned(b.Last)) and not (a.Last.Lo<=b.Last.Hi)) then begin
        BESENFreeAndNil(a);
        BESENFreeAndNil(b);
        SyntaxError;
       end;
       if assigned(a.Last) and assigned(b.Last) then begin
        a.Last.Hi:=b.Last.Hi;
       end else begin
        a.TakeoverCombine(b);
       end;
       BESENFreeAndNil(b);
      end;
     end;
     result.TakeoverCombine(a);
     BESENFreeAndNil(a);
    end;
    Expect(ord(']'));
    if (brefIGNORECASE in Flags) and not result.Canonicalized then begin
     result.Canonicalize;
    end;
    if InvertFlag then begin
     result.Invert;
    end;
   finally
    BESENFreeAndNil(a);
    BESENFreeAndNil(b);
   end;
  except
   BESENFreeAndNil(result);
   raise;
  end;
 end;
 procedure ParseAtom;
 var neg:boolean;
     i,p1,px:integer;
     c:TBESENRegExpCharClass;
 begin
  c:=nil;
  try
   if CurrentChar=ord('(') then begin
    NextChar;
    if (not AtEOF) and (CurrentChar=ord('?')) then begin
     NextChar;
     if (not AtEOF) and (CurrentChar=ord(':')) then begin
      NextChar;
      ParseDisjunction;
     end else if (not AtEOF) and ((CurrentChar=ord('=')) or (CurrentChar=ord('!'))) then begin
      neg:=CurrentChar=ord('!');
      NextChar;
      if neg then begin
       CodeAdd(breoAF);
      end else begin
       CodeAdd(breoAS);
      end;
      p1:=CodePos;
      CodeAddA(0);
      ParseDisjunction;
      CodeAdd(breoSUCCEED);
      px:=CodePos;
      CodePatchA(p1,px);
     end else begin
      SyntaxError;
     end;
    end else begin
     i:=CountOfCaptures;
     inc(CountOfCaptures);
     CodeAdd(breoSTART);
     CodeAddI(i);
     ParseDisjunction;
     CodeAdd(breoEND);
     CodeAddI(i);
    end;
    Expect(ord(')'));
   end else begin
    case CurrentChar of
     ord('\'):begin
      NextChar;
      if AtEOF then begin
       SyntaxError;
      end;
      if (CurrentChar>=ord('1')) and (CurrentChar<=ord('9')) then begin
       i:=ParseInteger;
       CodeAdd(breoBACKREF);
       CodeAddI(i);
       if i>MaxRef then begin
        MaxRef:=i;
       end;
       exit;
      end;
      c:=ParseClassEscape(true);
     end;
     ord('['):begin
      c:=ParseCharacterClass;
     end;
     ord('.'):begin
      NextChar;
      c:=TBESENRegExpCharClass.Create(self);
      c.AddChar($000a);
      c.AddChar($000d);
      c.AddRange($2028,$2029);
      c.Invert;
      c.Inverted:=false;
      c.Canonicalized:=true;
     end;
     else begin
      c:=TBESENRegExpCharClass.Create(self);
      c.AddChar(CurrentChar,brefIGNORECASE in Flags);
      c.Canonicalized:=brefIGNORECASE in Flags;
      NextChar;
     end;
    end;
    i:=InternCharClass(c);
    CodeAdd(breoCHAR);
    CodeAddI(i);
   end;
  except
   BESENFreeAndNil(c);
   raise;
  end;
 end;
 procedure ParseTerm;
  function QuantifierIsNext:boolean;
  var Lookahead:PBESENUTF32CHARS;
      Len,p:integer;
  begin
   if CurrentChar<>ord('{') then begin
    result:=false;
   end else if not (TBESEN(Instance).Compatibility and COMPAT_JS)<>0 then begin
    result:=true;
   end else begin
    Lookahead:=@UTF32Source[SourcePos];
    Len:=length(UTF32Source)-SourcePos;
    p:=1;
    if Len=0 then begin
     result:=false;
    end else begin
     while (p<Len) and (Lookahead^[p]>=ord('0')) and (Lookahead^[p]<=ord('9')) do begin
      inc(p);
     end;
     if (p<Len) and (Lookahead^[p]>=ord(',')) then begin
      inc(p);
      while (p<Len) and (Lookahead^[p]>=ord('0')) and (Lookahead^[p]<=ord('9')) do begin
       inc(p);
      end;
      if (p<Len) and (Lookahead^[p]>=ord('}')) then begin
       result:=true;
      end else begin
       result:=false;
      end;
     end else if (p<Len) and (Lookahead^[p]>=ord('}')) then begin
      result:=p>1;
     end else begin
      result:=false;
     end;
    end;
   end;
  end;
 var Lookahead:PBESENUTF32CHARS;
     Len,Position,oparen,cparen,min,max,m,px,py,i,c,p,p1,k:integer;
     Greedy:boolean;
 begin
  Lookahead:=@UTF32Source[SourcePos];
  Len:=length(UTF32Source)-SourcePos;
  case CurrentChar of
   ord('\'):begin
    if (Len>1) and (Lookahead^[1]=ord('b')) then begin
     NextChar;
     NextChar;
     CodeAdd(breoBRK);
     exit;
    end;
    if (Len>1) and (Lookahead^[1]=ord('B')) then begin
     NextChar;
     NextChar;
     CodeAdd(breoNBRK);
     exit;
    end;
   end;
   ord('^'):begin
    NextChar;
    CodeAdd(breoBOL);
    exit;
   end;
   ord('$'):begin
    NextChar;
    CodeAdd(breoEOL);
    exit;
   end;
   ord('*'),ord('+'),ord('?'),ord(')'),ord(']'),ord('{'),ord('}'),ord('|'):begin
    SyntaxError;
   end;
  end;

  Position:=CodePos;
  oparen:=CountOfCaptures;
  ParseAtom;
  cparen:=CountOfCaptures;

  if AtEOF then begin
   min:=1;
   max:=1;
  end else if CurrentChar=ord('*') then begin
   NextChar;
   min:=0;
   max:=brecINFINITY;
  end else if CurrentChar=ord('+') then begin
   NextChar;
   min:=1;
   max:=brecINFINITY;
  end else if CurrentChar=ord('?') then begin
   NextChar;
   min:=0;
   max:=1;
  end else if QuantifierIsNext then begin
   NextChar;
   min:=ParseInteger;
   if (not AtEOF) and (CurrentChar=ord(',')) then begin
    Expect(ord(','));
    if (not AtEOF) and (CurrentChar=ord('}')) then begin
     max:=brecINFINITY;
    end else begin
     max:=ParseInteger;
    end;
   end else begin
    max:=min;
   end;
   Expect(ord('}'));
  end else begin
   min:=1;
   max:=1;
  end;

  if (not AtEOF) and (CurrentChar=ord('?')) then begin
   NextChar;
   Greedy:=false;
  end else begin
   Greedy:=true;
  end;

  if (min=max) and not Greedy then begin
   Greedy:=true;
  end;

  if (Max<>brecINFINITY) and (min>max) then begin
   SyntaxError;
  end;

  if (min=1) and (max=1) then begin
   exit;
  end;

  if max=0 then begin
   CodePosSet(Position);
   exit;
  end;

  if oparen<>cparen then begin
   CodeInsert(Position,1+(2*bresINT));
   CodePatch(Position,breoUNDEF);
   CodePatchI(Position+1,oparen);
   CodePatchI(Position+1+bresINT,cparen);
  end;

  if min=max then begin
   p:=Position;
   i:=1+bresINT;
   c:=CountOfCounters;
   inc(CountOfCounters);
   CodeInsert(Position,i);
   CodePatch(p,breoZERO);
   inc(p);
   CodePatchI(p,c);
   inc(p,bresINT);
   px:=p;
{$ifdef UseAssert}
   Assert(p=(Position+i));
{$endif}
   CodeAdd(breoRNEXT);
   CodeAddI(c);
   CodeAddI(max);
   CodeAddA(px);
  end else if (min=0) and (max=1) then begin
   p:=Position;
   i:=1+bresADDR;
   CodeInsert(Position,i);
   if Greedy then begin
    CodePatch(p,breoGF);
   end else begin
    CodePatch(p,breoNF);
   end;
   inc(p);
   p1:=p;
   inc(p,bresADDR);
{$ifdef UseAssert}
   Assert(p=(Position+i));
{$endif}
   px:=CodePos;
   CodePatchA(p1,px);
  end else if (min=0) and (max=brecINFINITY) then begin
   p:=Position;
   i:=2+bresADDR+bresINT;
   m:=CountOfMarks;
   inc(CountOfMarks);
   CodeInsert(Position,i);
   px:=p;
   if Greedy then begin
    CodePatch(p,breoGF);
   end else begin
    CodePatch(p,breoNF);
   end;
   inc(p);
   p1:=p;
   inc(p,bresADDR);
   CodePatch(p,breoMARK);
   inc(p);
   CodePatchI(p,m);
   inc(p,bresINT);
{$ifdef UseAssert}
   Assert(p=(Position+i));
{$endif}
   CodeAdd(breoFDIST);
   CodeAddI(m);
   CodeAdd(breoGOTO);
   CodeAddA(px);
   py:=CodePos;
   CodePatchA(p1,py);
  end else begin
   p:=Position;
   i:=3+(bresINT*2)+bresADDR;
   c:=CountOfCounters;
   k:=CountOfMarks;
   inc(CountOfCounters);
   inc(CountOfMarks);
   CodeInsert(Position,i);
   CodePatch(p,breoZERO);
   inc(p);
   CodePatchI(p,c);
   inc(p,bresINT);
   px:=p;
   if Greedy then begin
    CodePatch(p,breoGF);
   end else begin
    CodePatch(p,breoNF);
   end;
   inc(p);
   p1:=p;
   inc(p,bresADDR);
   CodePatch(p,breoMARK);
   inc(p);
   CodePatchI(p,k);
   inc(p,bresINT);
{$ifdef UseAssert}
   Assert(p=(Position+i));
{$endif}
   if min<>0 then begin
    CodeAdd(breoRDIST);
    CodeAddI(k);
    CodeAddI(c);
    CodeAddI(min);
   end else begin
    CodeAdd(breoFDIST);
    CodeAddI(k);
   end;
   if max<>brecINFINITY then begin
    CodeAdd(breoRNEXT);
    CodeAddI(c);
    CodeAddI(max);
    CodeAddA(px);
   end else begin
    CodeAdd(breoMNEXT);
    CodeAddI(c);
    CodeAddI(min);
    CodeAddA(px);
   end;
   py:=CodePos;
   if min<>0 then begin
    CodeAdd(breoREACH);
    CodeAddI(c);
    CodeAddI(min);
   end;
   CodePatchA(p1,py);
  end;
 end;
 procedure ParseAlternative;
 begin
  while not (AtEOF or (CurrentChar=ord(')')) or (CurrentChar=ord('|'))) do begin
   ParseTerm;
  end;
 end;
 procedure ParseDisjunction;
 var Position,p,p1,p2,x1,x2,i:integer;
 begin
  Position:=CodePos;
  ParseAlternative;
  if (not AtEOF) and (CurrentChar=ord('|')) then begin
   NextChar;

   p:=Position;
   i:=1+bresADDR;

   CodeInsert(Position,i);

   CodePatch(p,breoGF);
   inc(p);
   p1:=p;
   inc(p,bresADDR);
{$ifdef UseAssert}
   Assert(p=(Position+i));
{$endif}

   CodeAdd(breoGOTO);
   p2:=CodePos;

   CodeAddA(0);
   x1:=CodePos;

   ParseDisjunction;

   x2:=CodePos;
   CodePatchA(p1,x1);
   CodePatchA(p2,x2);
  end;
 end;
 procedure Optimize;
 begin
 end;
begin
 Flags:=AFlags;
 CountOfCaptures:=1;
 Source:=ASource;
 AtEOF:=false;
 UTF32Source:=nil;
 try
  UTF32Source:=BESENUTF16ToUTF32(Source);
  SourcePos:=-1;
  NextChar;
  ParseDisjunction;
  CodeAdd(breoSUCCEED);
  SetLength(ByteCode,ByteCodeLen);
  Optimize;
  SetLength(CharClasses,CharClassesCount);
 finally
  SetLength(UTF32Source,0);
 end;
end;

function TBESENRegExp.Disassemble:TBESENString;
 function CodeMakeI(a:integer):integer;
 begin
  if (a+2)<=length(ByteCode) then begin
   result:=(ByteCode[a] shl 8) or ByteCode[a+1];
  end else begin
   raise EBESENInternalError.Create('Internal error: 201002250421-0000');
  end;
 end;
 function CodeMakeA(a:integer):integer;
 begin
  result:=(CodeMakeI(a)+a) and $ffff;
 end;
var PC,i,i2,i3,a:integer;
    Opcode:byte;
  procedure GetParams1; {$ifdef caninline}inline;{$endif}
  begin
   i:=CodeMakeI(PC);
   inc(PC,bresINT);
  end;
  procedure GetParams2; {$ifdef caninline}inline;{$endif}
  begin
   i:=CodeMakeI(PC);
   inc(PC,bresINT);
   i2:=CodeMakeI(PC);
   inc(PC,bresINT);
  end;
  procedure GetParams3; {$ifdef caninline}inline;{$endif}
  begin
   i:=CodeMakeI(PC);
   inc(PC,bresINT);
   i2:=CodeMakeI(PC);
   inc(PC,bresINT);
   i3:=CodeMakeI(PC);
   inc(PC,bresINT);
  end;
  procedure GetParams2A; {$ifdef caninline}inline;{$endif}
  begin
   i:=CodeMakeI(PC);
   inc(PC,bresINT);
   i2:=CodeMakeI(PC);
   inc(PC,bresINT);
   a:=CodeMakeA(PC);
   inc(PC,bresADDR);
  end;
  procedure GetParamsA; {$ifdef caninline}inline;{$endif}
  begin
   a:=CodeMakeA(PC);
   inc(PC,bresADDR);
  end;
begin
 result:='';
 PC:=0;
 i:=0;
 i2:=0;
 i3:=0;
 a:=0;
 while (PC>=0) and (PC<ByteCodeLen) do begin
  result:=result+'0x'+IntToHex(PC,8)+': ';
  Opcode:=ByteCode[PC];
  inc(PC);
  case Opcode of
   breoFAIL:begin
    result:=result+'FAIL';
   end;
   breoSUCCEED:begin
    result:=result+'SUCCEED';
   end;
   breoCHAR:begin
    GetParams1;
    result:=result+'CHAR '+IntToStr(i);
   end;
   breoZERO:begin
    GetParams1;
    result:=result+'ZERO '+IntToStr(i);
   end;
   breoREACH:begin
    GetParams2;
    result:=result+'REACH '+IntToStr(i)+', '+IntToStr(i2);
   end;
   breoNREACH:begin
    GetParams2;
    result:=result+'NREACH '+IntToStr(i)+', '+IntToStr(i2);
   end;
   breoSTART:begin
    GetParams1;
    result:=result+'START '+IntToStr(i);
   end;
   breoEND:begin
    GetParams1;
    result:=result+'END '+IntToStr(i);
   end;
   breoUNDEF:begin
    GetParams2;
    result:=result+'UNDEF '+IntToStr(i)+', '+IntToStr(i2);
   end;
   breoMARK:begin
    GetParams1;
    result:=result+'MARK '+IntToStr(i);
   end;
   breoFDIST:begin
    GetParams1;
    result:=result+'FDIST '+IntToStr(i);
   end;
   breoRDIST:begin
    GetParams3;
    result:=result+'RDIST '+IntToStr(i)+', '+IntToStr(i2)+', '+IntToStr(i3);
   end;
   breoMNEXT:begin
    GetParams2A;
    result:=result+'MNEXT '+IntToStr(i)+', '+IntToStr(i2)+', 0x'+IntToHex(a,8);
   end;
   breoRNEXT:begin
    GetParams2A;
    result:=result+'RNEXT '+IntToStr(i)+', '+IntToStr(i2)+', 0x'+IntToHex(a,8);
   end;
   breoGOTO:begin
    GetParamsA;
    result:=result+'GOTO 0x'+IntToHex(a,8);
   end;
   breoGS:begin
    GetParamsA;
    result:=result+'GS 0x'+IntToHex(a,8);
   end;
   breoNS:begin
    GetParamsA;
    result:=result+'NS 0x'+IntToHex(a,8);
   end;
   breoGF:begin
    GetParamsA;
    result:=result+'GF 0x'+IntToHex(a,8);
   end;
   breoNF:begin
    GetParamsA;
    result:=result+'NF 0x'+IntToHex(a,8);
   end;
   breoAS:begin
    GetParamsA;
    result:=result+'AS 0x'+IntToHex(a,8);
   end;
   breoAF:begin
    GetParamsA;
    result:=result+'AF 0x'+IntToHex(a,8);
   end;
   breoBOL:begin
    result:=result+'BOL';
   end;
   breoEOL:begin
    result:=result+'EOL';
   end;
   breoBRK:begin
    result:=result+'BRK';
   end;
   breoNBRK:begin
    result:=result+'NBRK';
   end;
   breoBACKREF:begin
    GetParams1;
    result:=result+'BACKREF '+IntToStr(i);
   end;
   else begin
    raise EBESENInternalError.Create('Internal error: 201002250323-0000');
   end;
  end;
  result:=result+#13#10;
 end;
end;

function TBESENRegExp.DisassembleOpcode(PC:integer):TBESENString;
 function CodeMakeI(a:integer):integer;
 begin
  if (a+2)<=length(ByteCode) then begin
   result:=(ByteCode[a] shl 8) or ByteCode[a+1];
  end else begin
   raise EBESENInternalError.Create('Internal error: 201002250421-0000');
  end;
 end;
 function CodeMakeA(a:integer):integer;
 begin
  result:=(CodeMakeI(a)+a) and $ffff;
 end;
var i,i2,i3,a:integer;
    Opcode:byte;
  procedure GetParams1; {$ifdef caninline}inline;{$endif}
  begin
   i:=CodeMakeI(PC);
   inc(PC,bresINT);
  end;
  procedure GetParams2; {$ifdef caninline}inline;{$endif}
  begin
   i:=CodeMakeI(PC);
   inc(PC,bresINT);
   i2:=CodeMakeI(PC);
   inc(PC,bresINT);
  end;
  procedure GetParams3; {$ifdef caninline}inline;{$endif}
  begin
   i:=CodeMakeI(PC);
   inc(PC,bresINT);
   i2:=CodeMakeI(PC);
   inc(PC,bresINT);
   i3:=CodeMakeI(PC);
   inc(PC,bresINT);
  end;
  procedure GetParams2A; {$ifdef caninline}inline;{$endif}
  begin
   i:=CodeMakeI(PC);
   inc(PC,bresINT);
   i2:=CodeMakeI(PC);
   inc(PC,bresINT);
   a:=CodeMakeA(PC);
   inc(PC,bresADDR);
  end;
  procedure GetParamsA; {$ifdef caninline}inline;{$endif}
  begin
   a:=CodeMakeA(PC);
   inc(PC,bresADDR);
  end;
begin
 result:='';
 i:=0;
 i2:=0;
 i3:=0;
 a:=0;
 if (PC>=0) and (PC<ByteCodeLen) then begin
  result:=result+'0x'+IntToHex(PC,8)+': ';
  Opcode:=ByteCode[PC];
  inc(PC);
  case Opcode of
   breoFAIL:begin
    result:=result+'FAIL';
   end;
   breoSUCCEED:begin
    result:=result+'SUCCEED';
   end;
   breoCHAR:begin
    GetParams1;
    result:=result+'CHAR '+IntToStr(i);
   end;
   breoZERO:begin
    GetParams1;
    result:=result+'ZERO '+IntToStr(i);
   end;
   breoREACH:begin
    GetParams2;
    result:=result+'REACH '+IntToStr(i)+', '+IntToStr(i2);
   end;
   breoNREACH:begin
    GetParams2;
    result:=result+'NREACH '+IntToStr(i)+', '+IntToStr(i2);
   end;
   breoSTART:begin
    GetParams1;
    result:=result+'START '+IntToStr(i);
   end;
   breoEND:begin
    GetParams1;
    result:=result+'END '+IntToStr(i);
   end;
   breoUNDEF:begin
    GetParams2;
    result:=result+'UNDEF '+IntToStr(i)+', '+IntToStr(i2);
   end;
   breoMARK:begin
    GetParams1;
    result:=result+'MARK '+IntToStr(i);
   end;
   breoFDIST:begin
    GetParams1;
    result:=result+'FDIST '+IntToStr(i);
   end;
   breoRDIST:begin
    GetParams3;
    result:=result+'RDIST '+IntToStr(i)+', '+IntToStr(i2)+', '+IntToStr(i3);
   end;
   breoMNEXT:begin
    GetParams2A;
    result:=result+'MNEXT '+IntToStr(i)+', '+IntToStr(i2)+', 0x'+IntToHex(a,8);
   end;
   breoRNEXT:begin
    GetParams2A;
    result:=result+'RNEXT '+IntToStr(i)+', '+IntToStr(i2)+', 0x'+IntToHex(a,8);
   end;
   breoGOTO:begin
    GetParamsA;
    result:=result+'GOTO 0x'+IntToHex(a,8);
   end;
   breoGS:begin
    GetParamsA;
    result:=result+'GS 0x'+IntToHex(a,8);
   end;
   breoNS:begin
    GetParamsA;
    result:=result+'NS 0x'+IntToHex(a,8);
   end;
   breoGF:begin
    GetParamsA;
    result:=result+'GF 0x'+IntToHex(a,8);
   end;
   breoNF:begin
    GetParamsA;
    result:=result+'NF 0x'+IntToHex(a,8);
   end;
   breoAS:begin
    GetParamsA;
    result:=result+'AS 0x'+IntToHex(a,8);
   end;
   breoAF:begin
    GetParamsA;
    result:=result+'AF 0x'+IntToHex(a,8);
   end;
   breoBOL:begin
    result:=result+'BOL';
   end;
   breoEOL:begin
    result:=result+'EOL';
   end;
   breoBRK:begin
    result:=result+'BRK';
   end;
   breoNBRK:begin
    result:=result+'NBRK';
   end;
   breoBACKREF:begin
    GetParams1;
    result:=result+'BACKREF '+IntToStr(i);
   end;
   else begin
    raise EBESENInternalError.Create('Internal error: 201002250323-0000');
   end;
  end;
 end;
end;

function TBESENRegExp.Execute(PC:integer;const Input:TBESENString;var State:TBESENRegExpState):boolean;
 function CodeMakeI(a:integer):integer; {$ifdef caninline}inline;{$endif}
 begin
  if (a+2)<=length(ByteCode) then begin
   result:=(ByteCode[a] shl 8) or ByteCode[a+1];
  end else begin
   BESENThrowInternalError('Internal error: 201002250421-0000');
   result:=0;
  end;
 end;
 function CodeMakeA(a:integer):integer; {$ifdef caninline}inline;{$endif}
 begin
  result:=(CodeMakeI(a)+a) and $ffff;
 end;
 function GetChar(i:integer):TBESENUTF32CHAR; {$ifdef caninline}inline;{$endif}
 begin
  if (i>=0) and (i<length(Input)) then begin
   result:=word(widechar(Input[i+1]));
  end else begin
   BESENThrowInternalError('Internal error: 201002250421-0001');
   result:=0;
  end;
 end;
 function IsWordChar(i:integer):boolean; {$ifdef caninline}inline;{$endif}
 begin
  if (i>=0) and (i<length(Input)) then begin
   case word(widechar(Input[i+1])) of
    ord('a')..ord('z'),ord('A')..ord('Z'),ord('0')..ord('9'),ord('_'):begin
     result:=true;
    end;
    else begin
     result:=false;
    end;
   end;
  end else begin
   result:=false;
  end;
 end;
var Opcode:byte;
    i,i2,i3,a,x,br,len:integer;
    ch:TBESENUTF32CHAR;
    OldCurrentState,NewState:TBESENRegExpState;
    DoBreak:boolean;
  procedure GetParams1; {$ifdef caninline}inline;{$endif}
  begin
   i:=CodeMakeI(PC);
   inc(PC,bresINT);
  end;
  procedure GetParams2; {$ifdef caninline}inline;{$endif}
  begin
   i:=CodeMakeI(PC);
   inc(PC,bresINT);
   i2:=CodeMakeI(PC);
   inc(PC,bresINT);
  end;
  procedure GetParams3; {$ifdef caninline}inline;{$endif}
  begin
   i:=CodeMakeI(PC);
   inc(PC,bresINT);
   i2:=CodeMakeI(PC);
   inc(PC,bresINT);
   i3:=CodeMakeI(PC);
   inc(PC,bresINT);
  end;
  procedure GetParams2A; {$ifdef caninline}inline;{$endif}
  begin
   i:=CodeMakeI(PC);
   inc(PC,bresINT);
   i2:=CodeMakeI(PC);
   inc(PC,bresINT);
   a:=CodeMakeA(PC);
   inc(PC,bresADDR);
  end;
  procedure GetParamsA; {$ifdef caninline}inline;{$endif}
  begin
   a:=CodeMakeA(PC);
   inc(PC,bresADDR);
  end;
{$ifdef RegExDebug}
var c,opc:integer;
    e:longword;
{$endif}
begin
 result:=false;
 OldCurrentState:=CurrentState;
 try
  NewState:=AllocateState;
  i:=0;
  i2:=0;
  i3:=0;
  a:=0;
  while (PC>=0) and (PC<ByteCodeLen) do begin
   Opcode:=ByteCode[PC];
{$ifdef RegExDebug}
   opc:=PC;
   write('index=',State.Captures[0].e,' captures=[');
   for c:=0 to CountOfCaptures-1 do begin
    if c<>0 then begin
     write(',');
    end;
    if State.Captures[c].s=brecUNDEFINED then begin
     write('undef');
    end else if int64(State.Captures[c].s+State.Captures[c].e)>length(Input) then begin
     write('bad<',IntToHex(State.Captures[c].s,8),',',IntToHex(State.Captures[c].e,8),'>');
    end else begin
     e:=State.Captures[c].e;
     if e=brecUNDEFINED then begin
      e:=State.Captures[0].e;
     end;
     write('"',copy(Input,State.Captures[c].s+1,e-State.Captures[c].s),'"');
     if State.Captures[c].e=brecUNDEFINED then begin
      write('+');
     end;
    end;
   end;
   write(']');
   if Opcode in [breoZERO,breoREACH,breoNREACH,breoMNEXT,breoRNEXT] then begin
    write(' counters=[');
    for c:=0 to CountOfCounters-1 do begin
     if c<>0 then begin
      write(',');
     end;
     write(State.Counters[c]);
    end;
    write(']');
   end;
   if Opcode in [breoMARK,breoFDIST,breoRDIST] then begin
    write(' marks=[');
    for c:=0 to CountOfMarks-1 do begin
     if c<>0 then begin
      write(',');
     end;
     write(State.Marks[c]);
    end;
    write(']');
   end;
   if Opcode<>breoCHAR then begin
    writeln;
    writeln(DumpOpcode(opc));
   end;
{$endif}
   inc(PC);
   case Opcode of
    breoFAIL:begin
     result:=false;
     break;
    end;
    breoSUCCEED:begin
     result:=true;
     break;
    end;
    breoCHAR:begin
     GetParams1;
     if State.Captures[0].e<longword(length(Input)) then begin
      ch:=GetChar(State.Captures[0].e);
      inc(State.Captures[0].e);
      if ((ch and $fc00)=$d800) and (State.Captures[0].e<longword(length(Input))) and ((GetChar(State.Captures[0].e) and $fc00)=$dc00) then begin
       ch:=(((ch and $3ff) shl 10) or (GetChar(State.Captures[0].e) and $3ff))+$10000;
      end;
{$ifdef RegExDebug}
      writeln(' ch=',widechar(word(ch)),' ',CharClasses[i].Contains(ch));
      writeln(DumpOpcode(opc));
{$endif}
      if not CharClasses[i].Contains(ch) then begin
       result:=false;
       break;
      end;
     end else begin
{$ifdef RegExDebug}
      writeln;
      writeln(DumpOpcode(opc));
{$endif}
      result:=false;
      break;
     end;
    end;
    breoZERO:begin
     GetParams1;
     State.Counters[i]:=0;
    end;
    breoREACH:begin
     GetParams2;
     if State.Counters[i]<i2 then begin
      result:=false;
      break;
     end;
    end;
    breoNREACH:begin
     GetParams2;
     if State.Counters[i]>=i2 then begin
      result:=false;
      break;
     end;
    end;
    breoSTART:begin
     GetParams1;
     State.Captures[i].s:=State.Captures[0].e;
     State.Captures[i].e:=brecUNDEFINED;
    end;
    breoEND:begin
     GetParams1;
     State.Captures[i].e:=State.Captures[0].e;
    end;
    breoUNDEF:begin
     GetParams2;
     while i<i2 do begin
      State.Captures[i].s:=brecUNDEFINED;
      State.Captures[i].e:=brecUNDEFINED;
      inc(i);
     end;
    end;
    breoMARK:begin
     GetParams1;
     State.Marks[i]:=State.Captures[0].e;
    end;
    breoFDIST:begin
     GetParams1;
     if State.Marks[i]=longint(State.Captures[0].e) then begin
      result:=false;
      break;
     end;
    end;
    breoRDIST:begin
     GetParams3;
     if (State.Marks[i]=longint(State.Captures[0].e)) and (State.Counters[i2]>=i3) then begin
      result:=false;
      break;
     end;
    end;
    breoMNEXT:begin
     GetParams2A;
     if State.Counters[i]<i2 then begin
      inc(State.Counters[i]);
     end;
     PC:=a;
    end;
    breoRNEXT:begin
     GetParams2A;
     inc(State.Counters[i]);
     if State.Counters[i]<i2 then begin
      PC:=a;
     end;
    end;
    breoGOTO:begin
     GetParamsA;
     PC:=a;
    end;
    breoGS:begin
     GetParamsA;
     CopyState(NewState,State);
     result:=Execute(PC,Input,NewState);
     if result then begin
      PC:=a;
     end else begin
      result:=false;
      break;
     end;
    end;
    breoNS:begin
     GetParamsA;
     CopyState(NewState,State);
     result:=Execute(a,Input,NewState);
     if not result then begin
      result:=false;
      break;
     end;
    end;
    breoGF:begin
     GetParamsA;
     CopyState(NewState,State);
     result:=Execute(PC,Input,NewState);
     if result then begin
      CopyState(State,NewState);
      result:=true;
      break;
     end else begin
      PC:=a;
     end;
    end;
    breoNF:begin
     GetParamsA;
     CopyState(NewState,State);
     result:=Execute(a,Input,NewState);
     if result then begin
      CopyState(State,NewState);
      result:=true;
      break;
     end;
    end;
    breoAS:begin
     GetParamsA;
     CopyState(NewState,State);
     result:=Execute(PC,Input,NewState);
     if result then begin
      i:=State.Captures[0].e;
      CopyState(State,NewState);
      State.Captures[0].e:=i;
      PC:=a;
     end else begin
      result:=false;
      break;
     end;
    end;
    breoAF:begin
     GetParamsA;
     CopyState(NewState,State);
     result:=Execute(PC,Input,NewState);
     if result then begin
      result:=false;
      break;
     end else begin
      PC:=a;
     end;
    end;
    breoBOL:begin
     if State.Captures[0].e=0 then begin
     end else if not (brefMULTILINE in Flags) then begin
      result:=false;
      break;
     end else if (word(widechar(Input[State.Captures[0].e]))=$000a) or
                 (word(widechar(Input[State.Captures[0].e]))=$000d) or
                 (word(widechar(Input[State.Captures[0].e]))=$2028) or
                 (word(widechar(Input[State.Captures[0].e]))=$2029) then begin
     end else begin
      result:=false;
      break;
     end;
    end;
    breoEOL:begin
     if State.Captures[0].e=longword(length(Input)) then begin
     end else if not (brefMULTILINE in Flags) then begin
      result:=false;
      break;
     end else if (word(widechar(Input[State.Captures[0].e]))=$000a) or
                 (word(widechar(Input[State.Captures[0].e]))=$000d) or
                 (word(widechar(Input[State.Captures[0].e]))=$2028) or
                 (word(widechar(Input[State.Captures[0].e]))=$2029) then begin
     end else begin
      result:=false;
      break;
     end;
    end;
    breoBRK:begin
     if IsWordChar(State.Captures[0].e-1)=IsWordChar(State.Captures[0].e) then begin
      result:=false;
      break;
     end;
    end;
    breoNBRK:begin
     if IsWordChar(State.Captures[0].e-1)<>IsWordChar(State.Captures[0].e) then begin
      result:=false;
      break;
     end;
    end;
    breoBACKREF:begin
     GetParams1;
     if State.Captures[i].e<>brecUNDEFINED then begin
      br:=State.Captures[i].s;
      len:=longword(longword(State.Captures[i].e)-longword(br));
      if int64(len+int64(State.Captures[0].e))>length(Input) then begin
       result:=false;
       break;
      end;
      DoBreak:=false;
      if brefIGNORECASE in Flags then begin
       for x:=0 to len-1 do begin
        if BESENUnicodeToUpper(GetChar(br+x))<>BESENUnicodeToUpper(GetChar(longint(State.Captures[0].e)+x)) then begin
         result:=false;
         DoBreak:=true;
         break;
        end;
       end;
      end else begin
       for x:=0 to len-1 do begin
        if GetChar(br+x)<>GetChar(longint(State.Captures[0].e)+x) then begin
         result:=false;
         DoBreak:=true;
         break;
        end;
       end;
      end;
      if DoBreak then begin
       break;
      end;
      inc(State.Captures[0].e,len);
     end;
    end;
    else begin
     BESENThrowInternalError('Internal error: 201002250323-0000');
    end;
   end;
  end;
 finally
  CurrentState:=OldCurrentState;
 end;
end;

function TBESENRegExp.Match(const Input:TBESENString;Index:integer;var Captures:TBESENRegExpCaptures):boolean;
var State:TBESENRegExpState;
    i:integer;
begin
 CurrentState:=nil;
 State:=AllocateState;
 try
  Captures:=nil;
  State.Captures[0].s:=Index;
  State.Captures[0].e:=Index;
  for i:=1 to CountOfCaptures-1 do begin
   State.Captures[i].s:=brecUNDEFINED;
   State.Captures[i].e:=brecUNDEFINED;
  end;
  result:=Execute(0,Input,State);
  if result then begin
   if length(Captures)<>length(State.Captures) then begin
    SetLength(Captures,length(State.Captures));
   end;
   if length(State.Captures)>0 then begin
    move(State.Captures[0],Captures[0],length(State.Captures)*sizeof(TBESENRegExpCapture));
   end;
  end else begin
   SetLength(Captures,0);
  end;
 finally
  CleanupStates;
 end;
end;

end.
