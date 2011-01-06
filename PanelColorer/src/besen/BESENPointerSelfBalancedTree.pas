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
unit BESENPointerSelfBalancedTree;
{$i BESEN.inc}

interface

uses BESENConstants,BESENTypes,BESENStringUtils;

type PBESENPointerSelfBalancedTreeValue=^TBESENPointerSelfBalancedTreeValue;
     TBESENPointerSelfBalancedTreeValue=record
      case boolean of
       false:(i:int64);
       true:(p:pointer);
     end;

     PBESENPointerSelfBalancedTreeNode=^TBESENPointerSelfBalancedTreeNode;
     TBESENPointerSelfBalancedTreeNode=record
      Parent,Left,Right,PreviousKey,NextKey:PBESENPointerSelfBalancedTreeNode;
      Level:int64;
      Key:pointer;
      Value:TBESENPointerSelfBalancedTreeValue;
     end;

     TBESENPointerSelfBalancedTreeKeys=array of pointer;

     TBESENPointerSelfBalancedTree=class
      protected
       procedure Skew(OldParent:PBESENPointerSelfBalancedTreeNode);
       function Split(OldParent:PBESENPointerSelfBalancedTreeNode):boolean;
       procedure RebalanceAfterLeafAdd(n:PBESENPointerSelfBalancedTreeNode);
       procedure DeleteNode(n:PBESENPointerSelfBalancedTreeNode); 
       function First(StartNode:PBESENPointerSelfBalancedTreeNode):PBESENPointerSelfBalancedTreeNode;
       function Next(n:PBESENPointerSelfBalancedTreeNode):PBESENPointerSelfBalancedTreeNode;
       function FindNode(const Key:pointer):PBESENPointerSelfBalancedTreeNode;
       procedure ClearNode(var Node:PBESENPointerSelfBalancedTreeNode);
       procedure OptimizeNode(var Node:PBESENPointerSelfBalancedTreeNode;MoreOptimize:boolean);
       function GetValue(Key:pointer):TBESENPointerSelfBalancedTreeValue;
       procedure SetValue(Key:pointer;Value:TBESENPointerSelfBalancedTreeValue);
      public
       RootNode:PBESENPointerSelfBalancedTreeNode;
       FirstKey,LastKey:PBESENPointerSelfBalancedTreeNode;
       constructor Create;
       destructor Destroy; override;
       function Find(const Key:pointer;var Value:TBESENPointerSelfBalancedTreeValue):boolean;
       function Insert(const Key:pointer;Value:TBESENPointerSelfBalancedTreeValue):PBESENPointerSelfBalancedTreeNode;
       procedure Remove(const Key:pointer);
       procedure Optimize;
       function Keys:TBESENPointerSelfBalancedTreeKeys;
       property Values[Key:pointer]:TBESENPointerSelfBalancedTreeValue read GetValue write SetValue; default;
     end;

implementation

constructor TBESENPointerSelfBalancedTree.Create;
begin
 inherited Create;
 new(RootNode);
 fillchar(RootNode^,sizeof(TBESENPointerSelfBalancedTreeNode),#0);
 RootNode^.Level:=$7fffffffffffffff;
 FirstKey:=nil;
 LastKey:=nil;
end;

destructor TBESENPointerSelfBalancedTree.Destroy;
begin
 ClearNode(RootNode^.Left);
 dispose(RootNode);
 inherited Destroy;
end;

function TBESENPointerSelfBalancedTree.First(StartNode:PBESENPointerSelfBalancedTreeNode):PBESENPointerSelfBalancedTreeNode;
begin
 try
  if not assigned(StartNode^.Left) then begin
   result:=nil;
   exit;
  end;
  result:=StartNode;
  while assigned(result^.Left) do begin
   result:=result^.Left;
  end;
 except
  result:=nil;
 end;
end;

function TBESENPointerSelfBalancedTree.Next(n:PBESENPointerSelfBalancedTreeNode):PBESENPointerSelfBalancedTreeNode;
begin
 try
  if assigned(n^.Right) then begin
   result:=n^.Right;
   while assigned(result^.Left) do begin
    result:=result^.Left;
   end;
  end else begin
   while assigned(n^.Parent) and (n^.Parent^.Right=n) do begin
    n:=n^.Parent;
   end;
   n:=n^.Parent;
   if not assigned(n) then begin
    result:=nil;
    exit;
   end;
   result:=n;
  end;
 except
  result:=nil;
 end;
end;

procedure TBESENPointerSelfBalancedTree.Skew(OldParent:PBESENPointerSelfBalancedTreeNode);
var NewParent:PBESENPointerSelfBalancedTreeNode;
begin
{$ifdef UseAssert}
 Assert(assigned(OldParent));
{$endif}
 NewParent:=OldParent^.Left;
{$ifdef UseAssert}
 Assert(assigned(NewParent));
{$endif}
 if OldParent^.Parent^.Left=OldParent then begin
  OldParent^.Parent^.Left:=NewParent;
 end else begin
  OldParent^.Parent^.Right:=NewParent;
 end;
 NewParent^.Parent:=OldParent^.Parent;
 OldParent^.Parent:=NewParent;

 OldParent^.Left:=NewParent^.Right;
 if assigned(OldParent^.Left) then begin
  OldParent^.Left^.Parent:=OldParent;
 end;
 NewParent^.Right:=OldParent;

 if assigned(OldParent^.Left) then begin
  OldParent^.level:=OldParent^.Left^.level+1;
 end else begin
  OldParent^.level:=1;
 end;
end;

function TBESENPointerSelfBalancedTree.Split(OldParent:PBESENPointerSelfBalancedTreeNode):boolean;
var NewParent:PBESENPointerSelfBalancedTreeNode;
begin
{$ifdef UseAssert}
 Assert(assigned(OldParent));
{$endif}
 NewParent:=OldParent^.Right;
 if assigned(NewParent) and assigned(NewParent^.Right) and (NewParent^.Right^.level=OldParent^.Level) then begin
  if OldParent^.Parent^.Left=OldParent then begin
   OldParent^.Parent^.Left:=NewParent;
  end else begin
   OldParent^.Parent^.Right:=NewParent;
  end;
  NewParent^.Parent:=OldParent^.Parent;
  OldParent^.Parent:=NewParent;

  OldParent^.Right:=NewParent^.Left;
  if assigned(OldParent^.Right) then begin
   OldParent^.Right^.Parent:=OldParent;
  end;
  NewParent^.Left:=OldParent;

  NewParent^.level:=OldParent^.level+1;

  result:=true;
 end else begin
  result:=false;
 end;
end;

procedure TBESENPointerSelfBalancedTree.RebalanceAfterLeafAdd(n:PBESENPointerSelfBalancedTreeNode);
begin
 // n is a node that has just been inserted and is now a Leaf node.
 n^.Level:=1;
 n^.Left:=nil;
 n^.Right:=nil;
 n:=n^.Parent;
 while n<>RootNode do begin
  if (assigned(n^.Left) and (n^.Level<>(n^.Left^.Level+1))) or ((not assigned(n^.Left)) and (n^.Level<>1)) then begin
   // this point the tree is correct, except (AA2) for n->Parent
   Skew(n);
   // We handle it (a Left add) by changing it into a Right add using Skew
   // If the original add was to the Left side of a node that is on the
   // Right side of a horizontal link, n now points to the rights side
   // of the second horizontal link, which is correct.
   // However if the original add was to the Left of node with a horizontal
   // link, we must get to the Right side of the second link.
   if (not assigned(n^.Right)) or (n^.Level<>n^.Right^.Level) then begin
    n:=n^.Parent;
   end;
  end;
  if not Split(n^.Parent) then begin
   break;
  end;
  n:=n^.Parent;
 end;
end;

function TBESENPointerSelfBalancedTree.FindNode(const Key:pointer):PBESENPointerSelfBalancedTreeNode;
var n:PBESENPointerSelfBalancedTreeNode;
begin
 try
  result:=nil;
  n:=RootNode^.Left;
  while assigned(n) do begin
   if Key=n^.Key then begin
    result:=n;
    break;
   end else if ptruint(Key)<ptruint(n^.Key) then begin
    n:=n^.Left;
   end else begin
    n:=n^.Right;
   end;
  end;
 except
  result:=nil;
 end;
end;

function TBESENPointerSelfBalancedTree.Insert(const Key:pointer;Value:TBESENPointerSelfBalancedTreeValue):PBESENPointerSelfBalancedTreeNode;
var n,s:PBESENPointerSelfBalancedTreeNode;
    LessThan:boolean;
begin
 result:=nil;
 try
  n:=nil;
  s:=RootNode^.Left;
  while assigned(s) do begin
   if Key=s^.Key then begin
    n:=s;
    break;
   end else if ptruint(Key)<ptruint(s^.Key) then begin
    s:=s^.Left;
   end else begin
    s:=s^.Right;
   end;
  end;
  if assigned(s) then begin
   n^.Value:=Value;
  end else begin
   new(n);
   fillchar(n^,sizeof(TBESENPointerSelfBalancedTreeNode),#0);
   n^.Key:=Key;
   n^.Value:=Value;
   if assigned(LastKey) then begin
    n^.PreviousKey:=LastKey;
    LastKey^.NextKey:=n;
    LastKey:=n;
   end else begin
    FirstKey:=n;
    LastKey:=n;
   end;
   s:=RootNode;
   LessThan:=true;
   while (LessThan and assigned(s^.Left)) or ((not LessThan) and assigned(s^.Right)) do begin
    if LessThan then begin
     s:=s^.Left;
    end else begin
     s:=s^.Right;
    end;
    LessThan:=ptruint(Key)<ptruint(s^.Key);
   end;
   if LessThan then begin
    s^.Left:=n;
   end else begin
    s^.Right:=n;
   end;
   n^.Parent:=s;
   RebalanceAfterLeafAdd(n);
   result:=n;
  end;
 except
  result:=nil;
 end;
end;

procedure TBESENPointerSelfBalancedTree.DeleteNode(n:PBESENPointerSelfBalancedTreeNode);
var Leaf,Temp:PBESENPointerSelfBalancedTreeNode;
begin
 try
  // If n is not a Leaf, we first swap it out with the Leaf node that just
  // precedes it.
  Leaf:=n;
  if assigned(n^.Left) then begin
   Leaf:=n^.Left;
   while assigned(Leaf^.Right) do begin
    Leaf:=Leaf^.Right;
   end;
  end else if assigned(n^.Right) then begin
   Leaf:=n^.Right;
  end;

  if Leaf^.Parent=n then begin
   Temp:=Leaf;
  end else begin
   Temp:=Leaf^.Parent;
  end;
  if Leaf^.Parent^.Left=Leaf then begin
   Leaf^.Parent^.Left:=nil;
  end else begin
   Leaf^.Parent^.Right:=nil;
  end;

  if n<>Leaf then begin
   if n^.Parent^.Left=n then begin
    n^.Parent^.Left:=Leaf;
   end else begin
    n^.Parent^.Right:=Leaf;
   end;
   Leaf^.Parent:=n^.Parent;
   if assigned(n^.Left) then begin
    n^.Left^.Parent:=Leaf;
   end;
   Leaf^.Left:=n^.Left;
   if assigned(n^.Right) then begin
    n^.Right^.Parent:=Leaf;
   end;
   Leaf^.Right:=n^.Right;
   Leaf^.level:=n^.level;
  end;
  if n<>RootNode then begin
   n^.Key:=nil;
   if assigned(n^.PreviousKey) then begin
    n^.PreviousKey^.NextKey:=n^.NextKey;
   end else if FirstKey=n then begin
    FirstKey:=n^.NextKey;
   end;
   if assigned(n^.NextKey) then begin
    n^.NextKey^.PreviousKey:=n^.PreviousKey;
   end else if LastKey=n then begin
    LastKey:=n^.PreviousKey;
   end;
   dispose(n);
  end;

  while Temp<>RootNode do begin
   if (assigned(Temp^.Left) and (Temp^.level>(Temp^.Left^.level+1))) or ((not assigned(Temp^.Left)) and (Temp^.level>1)) then begin
    dec(Temp^.level);
    if Split(Temp) then begin
     if Split(Temp) then begin
      Skew(Temp^.Parent^.Parent);
     end;
     break;
    end;
    Temp:=Temp^.Parent;
   end else if (assigned(Temp^.Right) and (Temp^.level<=(Temp^.Right^.level+1))) or ((not assigned(Temp^.Right)) and (Temp^.level<=1)) then begin
    break;
   end else begin
    Skew(Temp);
 {  if assigned(Temp^.Right) then begin
     if assigned(Temp^.Right^.Left) then begin
      Temp^.Right^.level:=Temp^.Right^.level+1;
     end else begin
      Temp^.Right^.level:=1;
     end;
    end;}
    if Temp^.level>Temp^.Parent^.level then begin
     Skew(Temp);
     Split(Temp^.Parent^.Parent);
     break;
    end;
    Temp:=Temp^.Parent^.Parent;
   end;
  end;
 except
 end;
end;

procedure TBESENPointerSelfBalancedTree.Remove(const Key:pointer);
var n:PBESENPointerSelfBalancedTreeNode;
begin
 try
  n:=RootNode^.Left;
  while assigned(n) do begin
   if Key=n^.Key then begin
    DeleteNode(n);
    break;
   end else if ptruint(Key)<ptruint(n^.Key) then begin
    n:=n^.Left;
   end else begin
    n:=n^.Right;
   end;
  end;
 except
 end;
end;

procedure TBESENPointerSelfBalancedTree.ClearNode(var Node:PBESENPointerSelfBalancedTreeNode);
begin
 if not assigned(Node) then begin
  exit;
 end;
 Node^.Key:=nil;
 if assigned(Node^.PreviousKey) then begin
  Node^.PreviousKey^.NextKey:=Node^.NextKey;
 end else if FirstKey=Node then begin
  FirstKey:=Node^.NextKey;
 end;
 if assigned(Node^.NextKey) then begin
  Node^.NextKey^.PreviousKey:=Node^.PreviousKey;
 end else if LastKey=Node then begin
  LastKey:=Node^.PreviousKey;
 end;
 ClearNode(Node^.Left);
 ClearNode(Node^.Right);
 dispose(Node);
 Node:=nil;
end;

procedure TBESENPointerSelfBalancedTree.OptimizeNode(var Node:PBESENPointerSelfBalancedTreeNode;MoreOptimize:boolean);
var Nodes:array of TBESENPointerSelfBalancedTreeNode;
    NodeCount,NodeIndex:integer;
 procedure CountNodes(Node:PBESENPointerSelfBalancedTreeNode);
 begin
  if not assigned(Node) then begin
   exit;
  end;
  CountNodes(Node^.Left);
  CountNodes(Node^.Right);
  inc(NodeCount);
 end;
 procedure CollectNodes(Node:PBESENPointerSelfBalancedTreeNode);
 begin
  if not assigned(Node) then begin
   exit;
  end;
  CollectNodes(Node^.Left);
  if NodeIndex>=length(Nodes) then begin
   NodeCount:=NodeIndex+1;
   SetLength(Nodes,NodeCount);
  end;
  Nodes[NodeIndex]:=Node^;
  inc(NodeIndex);
  CollectNodes(Node^.Right);
 end;
 procedure FreeNodes(var Node:PBESENPointerSelfBalancedTreeNode);
 begin
  if not assigned(Node) then begin
   exit;
  end;
  Node^.Key:=nil;
  if assigned(Node^.PreviousKey) then begin
   Node^.PreviousKey^.NextKey:=Node^.NextKey;
  end;
  if assigned(Node^.NextKey) then begin
   Node^.NextKey^.PreviousKey:=Node^.PreviousKey;
  end;
  if FirstKey=Node then begin
   FirstKey:=Node^.NextKey;
  end;
  if LastKey=Node then begin
   LastKey:=Node^.PreviousKey;
  end;
  CountNodes(Node^.Left);
  CountNodes(Node^.Right);
  dispose(Node);
  Node:=nil;
 end;
 procedure DoInsertNode(const Node:TBESENPointerSelfBalancedTreeNode);
 var n,s:PBESENPointerSelfBalancedTreeNode;
     LessThan:boolean;
 begin
  new(n);
  n^:=Node;
  n^.Parent:=nil;
  n^.Left:=nil;
  n^.Right:=nil;
  n^.Level:=0;
  s:=RootNode;
  LessThan:=true;
  while (LessThan and assigned(s^.Left)) or ((not LessThan) and assigned(s^.Right)) do begin
   if LessThan then begin
    s:=s^.Left;
   end else begin
    s:=s^.Right;
   end;
   LessThan:=ptruint(n^.Key)<ptruint(s^.Key);
  end;
  if LessThan then begin
   s^.Left:=n;
  end else begin
   s^.Right:=n;
  end;
  n^.Parent:=s;
  if assigned(LastKey) then begin
   n^.PreviousKey:=LastKey;
   LastKey^.NextKey:=n;
   LastKey:=n;
  end else begin
   FirstKey:=n;
   LastKey:=n;
  end;
  if not MoreOptimize then begin
   RebalanceAfterLeafAdd(n);
  end;
 end;
 procedure RepairNodes(var Node:PBESENPointerSelfBalancedTreeNode);
 begin
  if not assigned(Node) then begin
   exit;
  end;
  RepairNodes(Node^.Left);
  RepairNodes(Node^.Right);
  if assigned(Node^.Left) and assigned(Node^.Right) then begin
   Node^.Level:=Node^.Left^.Level+1;
  end else begin
   Node^.Level:=1;
  end;
 end;
 procedure ReinsertNodesForRepair(LowNode,HighNode:integer);
 var MiddleNode:integer;
 begin
  if HighNode<LowNode then begin
   exit;
  end;
  MiddleNode:=LowNode+((HighNode-LowNode) div 2);
  DoInsertNode(Nodes[MiddleNode]);
  ReinsertNodesForRepair(LowNode,MiddleNode-1);
  ReinsertNodesForRepair(MiddleNode+1,HighNode);
 end;
 procedure ReinsertNodes(LowNode,HighNode:integer);
 var i:integer;
 begin
  for i:=LowNode to HighNode do begin
   DoInsertNode(Nodes[i]);
  end;
 end;
begin
 if not assigned(Node) then begin
  exit;
 end;
 try
  Nodes:=nil;
  NodeCount:=0;
  CountNodes(Node);
  SetLength(Nodes,NodeCount);
  NodeIndex:=0;
  CollectNodes(Node);
  FreeNodes(Node);
  if MoreOptimize then begin
   ReinsertNodesForRepair(0,length(Nodes)-1);
   RepairNodes(RootNode^.Left);
  end else begin
   ReinsertNodes(0,length(Nodes)-1);
  end;
  SetLength(Nodes,0);
 except
 end;
end;

function TBESENPointerSelfBalancedTree.Find(const Key:pointer;var Value:TBESENPointerSelfBalancedTreeValue):boolean;
var n:PBESENPointerSelfBalancedTreeNode;
begin
 n:=FindNode(Key);
 if assigned(n) then begin
  Value:=n^.Value;
  result:=true;
 end else begin
  fillchar(Value,sizeof(TBESENPointerSelfBalancedTreeValue),#0);
  result:=false;
 end;
end;

procedure TBESENPointerSelfBalancedTree.Optimize;
begin
 OptimizeNode(RootNode^.Left,true);
end;

function TBESENPointerSelfBalancedTree.Keys:TBESENPointerSelfBalancedTreeKeys;
var CurrentNode:PBESENPointerSelfBalancedTreeNode;
    Count:integer;
begin
 result:=nil;
 Count:=0;
 CurrentNode:=FirstKey;
 while assigned(CurrentNode) do begin
  inc(Count);
  CurrentNode:=CurrentNode^.NextKey;
 end;
 SetLength(result,Count);
 Count:=0;
 CurrentNode:=FirstKey;
 while assigned(CurrentNode) do begin
  result[Count]:=CurrentNode^.Key;
  inc(Count);
  CurrentNode:=CurrentNode^.NextKey;
 end;
end;

function TBESENPointerSelfBalancedTree.GetValue(Key:pointer):TBESENPointerSelfBalancedTreeValue;
begin
 Find(Key,result);
end;

procedure TBESENPointerSelfBalancedTree.SetValue(Key:pointer;Value:TBESENPointerSelfBalancedTreeValue);
begin
 Insert(Key,Value);
end;

end.
