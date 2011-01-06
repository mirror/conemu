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
unit BESENStringTree;
{$i BESEN.inc}

interface

uses BESENConstants,BESENTypes,BESENStringUtils;

type TBESENStringTreeData=record
      case boolean of
       false:(i:int64);
       true:(p:pointer);
     end;

     PBESENStringTreeNode=^TBESENStringTreeNode;
     TBESENStringTreeNode=record
      TheChar:widechar;
      Data:TBESENStringTreeData;
      DataExist:longbool;
      Previous,Next,Up,Down:PBESENStringTreeNode;
     end;

     TBESENStringTree=class
      public
       Root:PBESENStringTreeNode;
       function CreateBESENStringTreeNode(AChar:widechar):PBESENStringTreeNode;
       procedure DestroyBESENStringTreeNode(Node:PBESENStringTreeNode);
      public
       constructor Create;
       destructor Destroy; override;
       procedure Clear;
       procedure DumpTree;
       procedure DumpList;
       procedure AppendTo(DestBESENStringTree:TBESENStringTree);
       procedure Optimize(DestBESENStringTree:TBESENStringTree);
       function Add(Content:TBESENString;const Data:TBESENStringTreeData;Replace:boolean=false):boolean;
       function Delete(Content:TBESENString):boolean;
       function Find(Content:TBESENString;var Data:TBESENStringTreeData):boolean;
     end;

implementation

constructor TBESENStringTree.Create;
begin
 inherited Create;
 Root:=nil;
 Clear;
end;

destructor TBESENStringTree.Destroy;
begin
 Clear;
 inherited Destroy;
end;

function TBESENStringTree.CreateBESENStringTreeNode(AChar:widechar):PBESENStringTreeNode;
begin
 getmem(result,sizeof(TBESENStringTreeNode));
 fillchar(result^.Data,sizeof(TBESENStringTreeData),#0);
 result^.TheChar:=AChar;
 result^.DataExist:=false;
 result^.Previous:=nil;
 result^.Next:=nil;
 result^.Up:=nil;
 result^.Down:=nil;
end;

procedure TBESENStringTree.DestroyBESENStringTreeNode(Node:PBESENStringTreeNode);
begin
 if not assigned(Node) then exit;
 DestroyBESENStringTreeNode(Node^.Next);
 DestroyBESENStringTreeNode(Node^.Down);
 freemem(Node);
end;

procedure TBESENStringTree.Clear;
begin
 DestroyBESENStringTreeNode(Root);
 Root:=nil;
end;

procedure TBESENStringTree.DumpTree;
var Ident:integer;
 procedure DumpNode(Node:PBESENStringTreeNode);
 var SubNode:PBESENStringTreeNode;
     IdentCounter,IdentOld:integer;
 begin
  for IdentCounter:=1 to Ident do write(' ');
  write(Node^.TheChar);
  IdentOld:=Ident;
  SubNode:=Node^.Next;
  while assigned(SubNode) do begin
   write(SubNode^.TheChar);
   if not assigned(SubNode^.Next) then break;
   inc(Ident);
   SubNode:=SubNode^.Next;
  end;
  writeln;
  inc(Ident);
  while assigned(SubNode) and (SubNode<>Node) do begin
   if assigned(SubNode^.Down) then DumpNode(SubNode^.Down);
   SubNode:=SubNode^.Previous;
   dec(Ident);
  end;
  Ident:=IdentOld;
  if assigned(Node^.Down) then DumpNode(Node^.Down);
 end;
begin
 Ident:=0;
 DumpNode(Root);
end;

procedure TBESENStringTree.DumpList;
 procedure DumpNode(Node:PBESENStringTreeNode;const ParentStr:TBESENString);
 var s:TBESENString;
 begin
  if not assigned(Node) then exit;
  if Node^.DataExist then begin
   s:=copy(ParentStr,0,length(ParentStr))+Node^.TheChar;
   writeln(BESENUTF16ToUTF8(s));
  end;
  if assigned(Node^.Next) then begin
   s:=copy(ParentStr,0,length(ParentStr))+Node^.TheChar;
   DumpNode(Node^.Next,s);
  end;
  if assigned(Node^.Down) then DumpNode(Node^.Down,ParentStr);
 end;
begin
 if not assigned(Root) then exit;
 DumpNode(Root,'');
end;

procedure TBESENStringTree.AppendTo(DestBESENStringTree:TBESENStringTree);
 procedure DumpNode(Node:PBESENStringTreeNode;const ParentStr:TBESENString);
 var s:TBESENString;
 begin
  if not assigned(Node) then exit;
  if Node^.DataExist then begin
   s:=copy(ParentStr,0,length(ParentStr))+Node^.TheChar;
   DestBESENStringTree.Add(s,Node^.Data);
  end;
  if assigned(Node^.Next) then begin
   s:=copy(ParentStr,0,length(ParentStr))+Node^.TheChar;
   DumpNode(Node^.Next,s);
  end;
  if assigned(Node^.Down) then DumpNode(Node^.Down,ParentStr);
 end;
begin
 if not assigned(DestBESENStringTree) then exit;
 if not assigned(Root) then exit;
 DumpNode(Root,'');
end;

procedure TBESENStringTree.Optimize(DestBESENStringTree:TBESENStringTree);
 procedure DumpNode(Node:PBESENStringTreeNode;ParentStr:TBESENString);
 var s:TBESENString;
 begin
  if not assigned(Node) then exit;
  ParentStr:=ParentStr;
  if Node^.DataExist then begin
   s:=copy(ParentStr,0,length(ParentStr))+Node^.TheChar;
   DestBESENStringTree.Add(s,Node^.Data);
  end;
  if assigned(Node^.Next) then begin
   s:=copy(ParentStr,0,length(ParentStr))+Node^.TheChar;
   DumpNode(Node^.Next,s);
  end;
  if assigned(Node^.Down) then DumpNode(Node^.Down,ParentStr);
 end;
begin
 if not assigned(DestBESENStringTree) then exit;
 DestBESENStringTree.Clear;
 if not assigned(Root) then exit;
 DumpNode(Root,'');
end;

function TBESENStringTree.Add(Content:TBESENString;const Data:TBESENStringTreeData;Replace:boolean=false):boolean;
var StringLength,Position,PositionCounter:integer;
    NewNode,LastNode,Node:PBESENStringTreeNode;
    StringChar,NodeChar:widechar;
begin
 result:=false;
 StringLength:=length(Content);
 if StringLength>0 then begin
  LastNode:=nil;
  Node:=Root;
  for Position:=1 to StringLength do begin
   StringChar:=Content[Position];
   if assigned(Node) then begin
    NodeChar:=Node^.TheChar;
    if NodeChar=StringChar then begin
     LastNode:=Node;
     Node:=Node^.Next;
   end else begin
     while (NodeChar<StringChar) and assigned(Node^.Down) do begin
      Node:=Node^.Down;
      NodeChar:=Node^.TheChar;
     end;
     if NodeChar=StringChar then begin
      LastNode:=Node;
      Node:=Node^.Next;
     end else begin
      NewNode:=CreateBESENStringTreeNode(StringChar);
      if NodeChar<StringChar then begin
       NewNode^.Down:=Node^.Down;
       NewNode^.Up:=Node;
       if assigned(NewNode^.Down) then begin
        NewNode^.Down^.Up:=NewNode;
       end;
       NewNode^.Previous:=Node^.Previous;
       Node^.Down:=NewNode;
      end else if NodeChar>StringChar then begin
       NewNode^.Down:=Node;
       NewNode^.Up:=Node^.Up;
       if assigned(NewNode^.Up) then begin
        NewNode^.Up^.Down:=NewNode;
       end;
       NewNode^.Previous:=Node^.Previous;
       if not assigned(NewNode^.Up) then begin
        if assigned(NewNode^.Previous) then begin
         NewNode^.Previous^.Next:=NewNode;
        end else begin
         Root:=NewNode;
        end;
       end;
       Node^.Up:=NewNode;
      end;
      LastNode:=NewNode;
      Node:=LastNode^.Next;
     end;
    end;
   end else begin
    for PositionCounter:=Position to StringLength do begin
     NewNode:=CreateBESENStringTreeNode(Content[PositionCounter]);
     if assigned(LastNode) then begin
      NewNode^.Previous:=LastNode;
      LastNode^.Next:=NewNode;
      LastNode:=LastNode^.Next;
     end else begin
      if not assigned(Root) then begin
       Root:=NewNode;
       LastNode:=Root;
      end;
     end;
    end;
    break;
   end;
  end;
  if assigned(LastNode) then begin
   if Replace or not LastNode^.DataExist then begin
    LastNode^.Data:=Data;
    LastNode^.DataExist:=true;
    result:=true;
   end;
  end;
 end;
end;

function TBESENStringTree.Delete(Content:TBESENString):boolean;
var StringLength,Position:integer;
    Node:PBESENStringTreeNode;
    StringChar,NodeChar:widechar;
begin
 result:=false;
 StringLength:=length(Content);
 if StringLength>0 then begin
  Node:=Root;
  for Position:=1 to StringLength do begin
   StringChar:=Content[Position];
   if assigned(Node) then begin
    NodeChar:=Node^.TheChar;
    while (NodeChar<>StringChar) and assigned(Node^.Down) do begin
     Node:=Node^.Down;
     NodeChar:=Node^.TheChar;
    end;
    if NodeChar=StringChar then begin
     if (Position=StringLength) and Node^.DataExist then begin
      Node^.DataExist:=false;
      result:=true;
      exit;
     end;
     Node:=Node^.Next;
    end else begin
     break;
    end;
   end else begin
    break;
   end;
  end;
 end;
end;

function TBESENStringTree.Find(Content:TBESENString;var Data:TBESENStringTreeData):boolean;
var StringLength,Position:integer;
    Node:PBESENStringTreeNode;
    StringChar,NodeChar:TBESENString;
begin
 result:=false;
 StringLength:=length(Content);
 if StringLength>0 then begin
  Node:=Root;
  for Position:=1 to StringLength do begin
   StringChar:=Content[Position];
   if assigned(Node) then begin
    NodeChar:=Node^.TheChar;
    while (NodeChar<>StringChar) and assigned(Node^.Down) do begin
     Node:=Node^.Down;
     NodeChar:=Node^.TheChar;
    end;
    if NodeChar=StringChar then begin
     if (Position=StringLength) and Node^.DataExist then begin
      Data:=Node^.Data;
      result:=true;
      exit;
     end;
     Node:=Node^.Next;
    end else begin
     break;
    end;
   end else begin
    break;
   end;
  end;
 end;
end;

end.
