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
unit BESENIntegerList;
{$i BESEN.inc}

interface

uses BESENConstants,BESENTypes;

type PBESENIntegerArray=^TBESENIntegerArray;
     TBESENIntegerArray=array[0..(2147483647 div sizeof(integer))-1] of integer;

     TBESENIntegerList=class
      private
       FList:PBESENIntegerArray;
       FCount,FSize:integer;
       function GetItem(index:integer):integer;
       procedure SetItem(index:integer;Value:integer);
       function GetItemPointer(index:integer):pointer;
      public
       constructor Create;
       destructor Destroy; override;
       procedure Clear;
       function Add(Item:integer):integer;
       procedure Insert(index:integer;Item:integer);
       procedure Delete(index:integer);
       function Remove(Item:integer):integer;
       function Find(Item:integer):integer;
       function IndexOf(Item:integer):integer;
       procedure Exchange(Index1,Index2:integer);
       procedure SetCapacity(NewCapacity:integer);
       procedure SetCount(NewCount:integer);
       property Count:integer read FCount;
       property Capacity:integer read FSize write SetCapacity;
       property Item[index:integer]:integer read GetItem write SetItem; default;
       property Items[index:integer]:integer read GetItem write SetItem;
       property PItems[index:integer]:pointer read GetItemPointer;
     end;

implementation

constructor TBESENIntegerList.Create;
begin
 inherited Create;
 FCount:=0;
 FSize:=0;
 FList:=nil;
 Clear;
end;

destructor TBESENIntegerList.Destroy;
begin
 Clear;
 inherited Destroy;
end;

procedure TBESENIntegerList.Clear;
begin
 FCount:=0;
 FSize:=0;
 ReallocMem(FList,0);
end;

procedure TBESENIntegerList.SetCapacity(NewCapacity:integer);
begin
 if (NewCapacity>=0) and (NewCapacity<high(TBESENIntegerArray)) then begin
  NewCapacity:=(NewCapacity+256) and not 255;
  if FSize<>NewCapacity then begin
   ReallocMem(FList,NewCapacity*sizeof(integer));
   if FSize<NewCapacity then begin
    FillChar(FList^[FSize],(NewCapacity-FSize)*sizeof(integer),#0);
   end;
   FSize:=NewCapacity;
  end;
 end;
end;

procedure TBESENIntegerList.SetCount(NewCount:integer);
begin
 if (NewCount>=0) and (NewCount<high(TBESENIntegerArray)) then begin
  if NewCount<FCount then begin
   FillChar(FList^[NewCount],(FCount-NewCount)*sizeof(integer),#0);
  end;
  SetCapacity(NewCount);
  FCount:=NewCount;
 end;
end;

function TBESENIntegerList.Add(Item:integer):integer;
begin
 result:=FCount;
 SetCount(result+1);
 FList^[result]:=Item;
end;

procedure TBESENIntegerList.Insert(index:integer;Item:integer);
var I:integer;
begin
 if (index>=0) and (index<FCount) then begin
  SetCount(FCount+1);
  for I:=FCount-1 downto index do FList^[I+1]:=FList^[I];
  FList^[index]:=Item;
 end else if index=FCount then begin
  Add(Item);
 end else if index>FCount then begin
  SetCount(index);
  Add(Item);
 end;
end;

procedure TBESENIntegerList.Delete(index:integer);
var I,J,K:integer;
begin
 if (index>=0) and (index<FCount) then begin
  K:=FCount-1;
  J:=index;
  for I:=J to K-1 do FList^[I]:=FList^[I+1];
  SetCount(K);
 end;
end;

function TBESENIntegerList.Remove(Item:integer):integer;
var I,J,K:integer;
begin
 result:=-1;
 K:=FCount;
 J:=-1;
 for I:=0 to K-1 do begin
  if FList^[I]=Item then begin
   J:=I;
   break;
  end;
 end;
 if J>=0 then begin
  dec(K);
  for I:=J to K-1 do FList^[I]:=FList^[I+1];
  SetCount(K);
  result:=J;
 end;
end;

function TBESENIntegerList.Find(Item:integer):integer;
var I:integer;
begin
 result:=-1;
 for I:=0 to FCount-1 do begin
  if FList^[I]=Item then begin
   result:=I;
   exit;
  end;
 end;
end;

function TBESENIntegerList.IndexOf(Item:integer):integer;
var I:integer;
begin
 result:=-1;
 for I:=0 to FCount-1 do begin
  if FList^[I]=Item then begin
   result:=I;
   exit;
  end;
 end;
end;

procedure TBESENIntegerList.Exchange(Index1,Index2:integer);
var TempInteger:integer;
begin
 if (Index1>=0) and (Index1<FCount) and (Index2>=0) and (Index2<FCount) then begin
  TempInteger:=FList^[Index1];
  FList^[Index1]:=FList^[Index2];
  FList^[Index2]:=TempInteger;
 end;
end;

function TBESENIntegerList.GetItem(index:integer):integer;
begin
 if (index>=0) and (index<FCount) then begin
  result:=FList^[index];
 end else begin
  result:=0;
 end;
end;

procedure TBESENIntegerList.SetItem(index:integer;Value:integer);
begin
 if (index>=0) and (index<FCount) then begin
  FList^[index]:=Value;
 end;
end;

function TBESENIntegerList.GetItemPointer(index:integer):pointer;
begin
 if (index>=0) and (index<FCount) then begin
  result:=@FList^[index];
 end else begin
  result:=nil;
 end;
end;

end.
 