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
unit BESENGarbageCollector;
{$i BESEN.inc}

interface

uses BESENConstants,BESENTypes,BESENBaseObject,BESENCollectorObject,
     BESENPointerSelfBalancedTree,BESENValue;

type TBESENGarbageCollectorObjectList=class;

     TBESENGarbageCollectorObject=class(TBESENCollectorObject)
      public
       GarbageCollectorPrevious,GarbageCollectorNext,
       GarbageCollectorRootObjectListPrevious,GarbageCollectorRootObjectListNext,
       GarbageCollectorProtectedObjectListPrevious,GarbageCollectorProtectedObjectListNext,
       GarbageCollectorObjectListPrevious,GarbageCollectorObjectListNext:TBESENGarbageCollectorObject;
       GarbageCollectorObjectList:TBESENGarbageCollectorObjectList;
       GarbageCollectorDependsChildren,GarbageCollectorDependsParents:TBESENPointerSelfBalancedTree;
       GarbageCollectorLockReferenceCounter:integer;
       constructor Create(AInstance:TObject); overload; override;
       destructor Destroy; override;
       procedure Finalize; virtual;
       procedure Mark; virtual;
       procedure DependsOn(Parent:TBESENGarbageCollectorObject);
       procedure GarbageCollectorLock;
       procedure GarbageCollectorUnlock;
     end;

     TBESENGarbageCollectorRootObjectList=class(TBESENBaseObject)
      public
       First,Last:TBESENGarbageCollectorObject;
       constructor Create(AInstance:TObject); overload; override;
       destructor Destroy; override;
       procedure Clear;
       procedure ClearWithFree;
       procedure Add(AObject:TBESENGarbageCollectorObject);
       procedure Remove(AObject:TBESENGarbageCollectorObject);
       function Contains(AObject:TBESENGarbageCollectorObject):boolean;
       procedure Push(AObject:TBESENGarbageCollectorObject);
       function Pop:TBESENGarbageCollectorObject;
     end;

     TBESENGarbageCollectorProtectedObjectList=class(TBESENBaseObject)
      public
       First,Last:TBESENGarbageCollectorObject;
       constructor Create(AInstance:TObject); overload; override;
       destructor Destroy; override;
       procedure Clear;
       procedure ClearWithFree;
       procedure Add(AObject:TBESENGarbageCollectorObject);
       procedure Remove(AObject:TBESENGarbageCollectorObject);
       function Contains(AObject:TBESENGarbageCollectorObject):boolean;
       procedure Push(AObject:TBESENGarbageCollectorObject);
       function Pop:TBESENGarbageCollectorObject;
     end;

     TBESENGarbageCollectorObjectList=class(TBESENBaseObject)
      public
       First,Last:TBESENGarbageCollectorObject;
       constructor Create(AInstance:TObject); overload; override;
       destructor Destroy; override;
       procedure Clear;
       procedure ClearWithFree;
       procedure Add(AObject:TBESENGarbageCollectorObject);
       procedure Remove(AObject:TBESENGarbageCollectorObject);
       function Contains(AObject:TBESENGarbageCollectorObject):boolean;
       procedure Push(AObject:TBESENGarbageCollectorObject);
       function Pop:TBESENGarbageCollectorObject;
     end;

     TBESENGarbageCollectorState=(bgcsINIT,bgcsMARKROOTS,bgcsMARKPROTECTED,bgcsMARKPROGRAMNODES,bgcsMARKCONTEXTS,bgcsMARKGRAYS,bgcsCHECKFORLOCKEDWHITES,bgcsMARKLOCKEDWHITES,bgcsFINALIZEWHITES,bgcsSWEEPWHITES,bgcsDONE);

     TBESENGarbageCollector=class(TBESENBaseObject)
      public
       First,Last:TBESENGarbageCollectorObject;
       CurrentRootObject:TBESENGarbageCollectorObject;
       CurrentProtectedObject:TBESENGarbageCollectorObject;
       CurrentMarkObject:TBESENGarbageCollectorObject;
       CurrentSweepObject:TBESENGarbageCollectorObject;
       CurrentContext:TObject;
       RootObjectList:TBESENGarbageCollectorRootObjectList;
       ProtectedObjectList:TBESENGarbageCollectorProtectedObjectList;
       WhiteObjectList:TBESENGarbageCollectorObjectList;
       GrayObjectList:TBESENGarbageCollectorObjectList;
       BlackObjectList:TBESENGarbageCollectorObjectList;
       IsSweeping:longbool;
       State:TBESENGarbageCollectorState;
       TriggerCounter:integer;
       SweepCount:integer;
       MarkCount:integer;
       TriggerCountPerCollect:integer;
       constructor Create(AInstance:TObject); overload; override;
       destructor Destroy; override;
       procedure Clear;
       procedure Reset;
       procedure WhiteIt(AObject:TBESENGarbageCollectorObject);
       procedure ForceGrayIt(AObject:TBESENGarbageCollectorObject);
       procedure GrayIt(AObject:TBESENGarbageCollectorObject);
       procedure BlackIt(AObject:TBESENGarbageCollectorObject);
       procedure Protect(AObject:TBESENGarbageCollectorObject);
       procedure Unprotect(AObject:TBESENGarbageCollectorObject);
       procedure FinalizeObjectForSweeping(AObject:TBESENGarbageCollectorObject);
       procedure GrayValue(var Value:TBESENValue);
       procedure FinalizeValue(var Value:TBESENValue);
       procedure Mark(AObject:TBESENGarbageCollectorObject);
       procedure Flip;
       procedure TriggerCollect;
       function Collect:boolean;
       procedure CollectAll;
       procedure Use(AObject:TBESENGarbageCollectorObject);
       procedure Add(AObject:TBESENGarbageCollectorObject);
       procedure AddRoot(AObject:TBESENGarbageCollectorObject);
       procedure RemoveRoot(AObject:TBESENGarbageCollectorObject);
       procedure AddProtected(AObject:TBESENGarbageCollectorObject);
       procedure RemoveProtected(AObject:TBESENGarbageCollectorObject);
       procedure LockObject(Obj:TBESENGarbageCollectorObject);
       procedure UnlockObject(Obj:TBESENGarbageCollectorObject);
       procedure LockValue(const Value:TBESENValue);
       procedure UnlockValue(const Value:TBESENValue);
     end;

implementation

uses BESEN,BESENUtils,BESENObject,BESENEnvironmentRecord,
     BESENASTNodes,BESENCode,BESENContext;

constructor TBESENGarbageCollectorObject.Create(AInstance:TObject);
begin
 inherited Create(AInstance);
 GarbageCollectorPrevious:=TBESEN(Instance).GarbageCollector.Last;
 GarbageCollectorNext:=nil;
 if assigned(GarbageCollectorPrevious) then begin
  GarbageCollectorPrevious.GarbageCollectorNext:=self;
 end else begin
  TBESEN(Instance).GarbageCollector.First:=self;
 end;
 TBESEN(Instance).GarbageCollector.Last:=self;
 GarbageCollectorRootObjectListPrevious:=nil;
 GarbageCollectorRootObjectListNext:=nil;
 GarbageCollectorProtectedObjectListPrevious:=nil;
 GarbageCollectorProtectedObjectListNext:=nil;
 GarbageCollectorObjectListPrevious:=nil;
 GarbageCollectorObjectListNext:=nil;
 GarbageCollectorObjectList:=nil;
 GarbageCollectorDependsChildren:=TBESENPointerSelfBalancedTree.Create;
 GarbageCollectorDependsParents:=TBESENPointerSelfBalancedTree.Create;
 GarbageCollectorLockReferenceCounter:=0;
end;

destructor TBESENGarbageCollectorObject.Destroy;
var n:PBESENPointerSelfBalancedTreeNode;
begin
 if assigned(GarbageCollectorPrevious) then begin
  GarbageCollectorPrevious.GarbageCollectorNext:=GarbageCollectorNext;
 end else if TBESEN(Instance).GarbageCollector.First=self then begin
  TBESEN(Instance).GarbageCollector.First:=GarbageCollectorNext;
 end;
 if assigned(GarbageCollectorNext) then begin
  GarbageCollectorNext.GarbageCollectorPrevious:=GarbageCollectorPrevious;
 end else if TBESEN(Instance).GarbageCollector.Last=self then begin
  TBESEN(Instance).GarbageCollector.Last:=GarbageCollectorPrevious;
 end;
 GarbageCollectorNext:=nil;
 GarbageCollectorPrevious:=nil;
 if assigned(GarbageCollectorObjectList) then begin
  GarbageCollectorObjectList.Remove(self);
  GarbageCollectorObjectList:=nil;
 end;
 if assigned(GarbageCollectorDependsParents) then begin
  n:=GarbageCollectorDependsParents.FirstKey;
  while assigned(n) do begin
   if assigned(n^.Key) and assigned(TBESENGarbageCollectorObject(n^.Key).GarbageCollectorDependsChildren) then begin
    TBESENGarbageCollectorObject(n^.Key).GarbageCollectorDependsChildren.Remove(self);
   end;
   n:=n^.NextKey;
  end;
 end;
 if assigned(GarbageCollectorDependsChildren) then begin
  n:=GarbageCollectorDependsChildren.FirstKey;
  while assigned(n) do begin
   if assigned(n^.Key) and assigned(TBESENGarbageCollectorObject(n^.Key).GarbageCollectorDependsParents) then begin
    TBESENGarbageCollectorObject(n^.Key).GarbageCollectorDependsParents.Remove(self);
   end;
   n:=n^.NextKey;
  end;
 end;
 BESENFreeAndNil(GarbageCollectorDependsChildren);
 BESENFreeAndNil(GarbageCollectorDependsParents);

 if assigned(Instance) and assigned(TBESEN(Instance).GarbageCollector) then begin
  if assigned(TBESEN(Instance).GarbageCollector.RootObjectList) and TBESEN(Instance).GarbageCollector.RootObjectList.Contains(self) then begin
   TBESEN(Instance).GarbageCollector.RootObjectList.Remove(self);
  end;
  if assigned(TBESEN(Instance).GarbageCollector.ProtectedObjectList) and TBESEN(Instance).GarbageCollector.ProtectedObjectList.Contains(self) then begin
   TBESEN(Instance).GarbageCollector.ProtectedObjectList.Remove(self);
  end;
 end;
 inherited Destroy;
end;

procedure TBESENGarbageCollectorObject.Finalize;
var n:PBESENPointerSelfBalancedTreeNode;
begin
 if assigned(GarbageCollectorDependsParents) then begin
  n:=GarbageCollectorDependsParents.FirstKey;
  while assigned(n) do begin
   if assigned(n^.Key) and assigned(TBESENGarbageCollectorObject(n^.Key).GarbageCollectorDependsChildren) then begin
    TBESENGarbageCollectorObject(n^.Key).GarbageCollectorDependsChildren.Remove(self);
   end;
   n:=n^.NextKey;
  end;
 end;
 if assigned(GarbageCollectorDependsChildren) then begin
  n:=GarbageCollectorDependsChildren.FirstKey;
  while assigned(n) do begin
   if assigned(n^.Key) and assigned(TBESENGarbageCollectorObject(n^.Key).GarbageCollectorDependsParents) then begin
    TBESENGarbageCollectorObject(n^.Key).GarbageCollectorDependsParents.Remove(self);
   end;
   n:=n^.NextKey;
  end;
 end;
end;

procedure TBESENGarbageCollectorObject.Mark;
var n:PBESENPointerSelfBalancedTreeNode;
begin
 if assigned(GarbageCollectorDependsChildren) then begin
  n:=GarbageCollectorDependsChildren.FirstKey;
  while assigned(n) do begin
   if assigned(n^.Key) then begin
    TBESEN(Instance).GarbageCollector.GrayIt(TBESENGarbageCollectorObject(n^.Key));
   end;
   n:=n^.NextKey;
  end;
 end;
end;

procedure TBESENGarbageCollectorObject.DependsOn(Parent:TBESENGarbageCollectorObject);
var v:TBESENPointerSelfBalancedTreeValue;
begin
 if assigned(Parent) then begin
  if assigned(GarbageCollectorDependsParents) then begin
   v.p:=Parent;
   GarbageCollectorDependsParents.Insert(Parent,v);
  end;
  if assigned(Parent.GarbageCollectorDependsChildren) then begin
   v.p:=self;
   Parent.GarbageCollectorDependsChildren.Insert(self,v);
  end;
 end;
end;

procedure TBESENGarbageCollectorObject.GarbageCollectorLock;
begin
 inc(GarbageCollectorLockReferenceCounter);
end;

procedure TBESENGarbageCollectorObject.GarbageCollectorUnlock;
begin
 dec(GarbageCollectorLockReferenceCounter);
end;

constructor TBESENGarbageCollectorRootObjectList.Create(AInstance:TObject);
begin
 inherited Create(AInstance);
 Clear;
end;

destructor TBESENGarbageCollectorRootObjectList.Destroy;
begin
 Clear;
 inherited Destroy;
end;

procedure TBESENGarbageCollectorRootObjectList.Clear;
var Item,NextItem:TBESENGarbageCollectorObject;
begin
 Item:=First;                             
 while assigned(Item) do begin
  NextItem:=Item.GarbageCollectorRootObjectListNext;
  Item.GarbageCollectorRootObjectListPrevious:=nil;
  Item.GarbageCollectorRootObjectListNext:=nil;
  Item:=NextItem;
 end;
 First:=nil;
 Last:=nil;
end;

procedure TBESENGarbageCollectorRootObjectList.ClearWithFree;
begin
 while assigned(First) do begin
  First.Destroy;
 end;
 Clear;
end;

procedure TBESENGarbageCollectorRootObjectList.Add(AObject:TBESENGarbageCollectorObject);
begin
 if assigned(Last) then begin
  Last.GarbageCollectorRootObjectListNext:=AObject;
  AObject.GarbageCollectorRootObjectListPrevious:=Last;
  AObject.GarbageCollectorRootObjectListNext:=nil;
  Last:=AObject;
 end else begin
  First:=AObject;
  Last:=AObject;
 end;
end;

procedure TBESENGarbageCollectorRootObjectList.Remove(AObject:TBESENGarbageCollectorObject);
begin
 if assigned(AObject.GarbageCollectorRootObjectListPrevious) then begin
  AObject.GarbageCollectorRootObjectListPrevious.GarbageCollectorRootObjectListNext:=AObject.GarbageCollectorRootObjectListNext;
 end else if First=AObject then begin
  First:=AObject.GarbageCollectorRootObjectListNext;
 end;
 if assigned(AObject.GarbageCollectorRootObjectListNext) then begin
  AObject.GarbageCollectorRootObjectListNext.GarbageCollectorRootObjectListPrevious:=AObject.GarbageCollectorRootObjectListPrevious;
 end else if Last=AObject then begin
  Last:=AObject.GarbageCollectorRootObjectListPrevious;
 end;
 AObject.GarbageCollectorRootObjectListNext:=nil;
 AObject.GarbageCollectorRootObjectListPrevious:=nil;
end;

function TBESENGarbageCollectorRootObjectList.Contains(AObject:TBESENGarbageCollectorObject):boolean;
begin
 result:=((First=AObject) or (Last=AObject)) or (assigned(AObject.GarbageCollectorRootObjectListNext) or assigned(AObject.GarbageCollectorRootObjectListPrevious));
end;

procedure TBESENGarbageCollectorRootObjectList.Push(AObject:TBESENGarbageCollectorObject);
begin
 Add(AObject);
end;

function TBESENGarbageCollectorRootObjectList.Pop:TBESENGarbageCollectorObject;
begin
 result:=Last;
 if assigned(result) then begin
  Remove(result);
 end;
end;

constructor TBESENGarbageCollectorProtectedObjectList.Create(AInstance:TObject);
begin
 inherited Create(AInstance);
 Clear;
end;

destructor TBESENGarbageCollectorProtectedObjectList.Destroy;
begin
 Clear;
 inherited Destroy;
end;

procedure TBESENGarbageCollectorProtectedObjectList.Clear;
var Item,NextItem:TBESENGarbageCollectorObject;
begin
 Item:=First;
 while assigned(Item) do begin
  NextItem:=Item.GarbageCollectorProtectedObjectListNext;
  Item.GarbageCollectorProtectedObjectListPrevious:=nil;
  Item.GarbageCollectorProtectedObjectListNext:=nil;
  Item:=NextItem;
 end;
 First:=nil;
 Last:=nil;
end;

procedure TBESENGarbageCollectorProtectedObjectList.ClearWithFree;
begin
 while assigned(First) do begin
  First.Destroy;
 end;
 Clear;
end;

procedure TBESENGarbageCollectorProtectedObjectList.Add(AObject:TBESENGarbageCollectorObject);
begin
 if assigned(Last) then begin
  Last.GarbageCollectorProtectedObjectListNext:=AObject;
  AObject.GarbageCollectorProtectedObjectListPrevious:=Last;
  AObject.GarbageCollectorProtectedObjectListNext:=nil;
  Last:=AObject;
 end else begin
  First:=AObject;
  Last:=AObject;
 end;
end;

procedure TBESENGarbageCollectorProtectedObjectList.Remove(AObject:TBESENGarbageCollectorObject);
begin
 if assigned(AObject.GarbageCollectorProtectedObjectListPrevious) then begin
  AObject.GarbageCollectorProtectedObjectListPrevious.GarbageCollectorProtectedObjectListNext:=AObject.GarbageCollectorProtectedObjectListNext;
 end else if First=AObject then begin
  First:=AObject.GarbageCollectorProtectedObjectListNext;
 end;
 if assigned(AObject.GarbageCollectorProtectedObjectListNext) then begin
  AObject.GarbageCollectorProtectedObjectListNext.GarbageCollectorProtectedObjectListPrevious:=AObject.GarbageCollectorProtectedObjectListPrevious;
 end else if Last=AObject then begin
  Last:=AObject.GarbageCollectorProtectedObjectListPrevious;
 end;
 AObject.GarbageCollectorProtectedObjectListNext:=nil;
 AObject.GarbageCollectorProtectedObjectListPrevious:=nil;
end;

function TBESENGarbageCollectorProtectedObjectList.Contains(AObject:TBESENGarbageCollectorObject):boolean;
begin
 result:=((First=AObject) or (Last=AObject)) or (assigned(AObject.GarbageCollectorProtectedObjectListNext) or assigned(AObject.GarbageCollectorProtectedObjectListPrevious));
end;

procedure TBESENGarbageCollectorProtectedObjectList.Push(AObject:TBESENGarbageCollectorObject);
begin
 Add(AObject);
end;

function TBESENGarbageCollectorProtectedObjectList.Pop:TBESENGarbageCollectorObject;
begin
 result:=Last;
 if assigned(result) then begin
  Remove(result);
 end;
end;

constructor TBESENGarbageCollectorObjectList.Create(AInstance:TObject);
begin
 inherited Create(AInstance);
 Clear;
end;

destructor TBESENGarbageCollectorObjectList.Destroy;
begin
 Clear;
 inherited Destroy;
end;

procedure TBESENGarbageCollectorObjectList.Clear;
var Item,NextItem:TBESENGarbageCollectorObject;
begin
 Item:=First;
 while assigned(Item) do begin
  NextItem:=Item.GarbageCollectorObjectListNext;
  Item.GarbageCollectorObjectListPrevious:=nil;
  Item.GarbageCollectorObjectListNext:=nil;
  Item.GarbageCollectorObjectList:=nil;
  Item:=NextItem;
 end;
 First:=nil;
 Last:=nil;
end;

procedure TBESENGarbageCollectorObjectList.ClearWithFree;
begin
 while assigned(First) do begin
  First.Destroy;
 end;
 Clear;
end;

procedure TBESENGarbageCollectorObjectList.Add(AObject:TBESENGarbageCollectorObject);
begin
 if assigned(AObject.GarbageCollectorObjectList) then begin
  AObject.GarbageCollectorObjectList.Remove(AObject);
 end;
 AObject.GarbageCollectorObjectList:=self;
 if assigned(Last) then begin
  AObject.GarbageCollectorObjectListPrevious:=Last;
  Last.GarbageCollectorObjectListNext:=AObject;
 end else begin
  First:=AObject;
  AObject.GarbageCollectorObjectListPrevious:=nil;
 end;
 AObject.GarbageCollectorObjectListNext:=nil;
 Last:=AObject;
end;

procedure TBESENGarbageCollectorObjectList.Remove(AObject:TBESENGarbageCollectorObject);
begin
 if assigned(AObject.GarbageCollectorObjectListPrevious) then begin
  AObject.GarbageCollectorObjectListPrevious.GarbageCollectorObjectListNext:=AObject.GarbageCollectorObjectListNext;
 end else if First=AObject then begin
  First:=AObject.GarbageCollectorObjectListNext;
 end;
 if assigned(AObject.GarbageCollectorObjectListNext) then begin
  AObject.GarbageCollectorObjectListNext.GarbageCollectorObjectListPrevious:=AObject.GarbageCollectorObjectListPrevious;
 end else if Last=AObject then begin
  Last:=AObject.GarbageCollectorObjectListPrevious;
 end;
 AObject.GarbageCollectorObjectListNext:=nil;
 AObject.GarbageCollectorObjectListPrevious:=nil;
 AObject.GarbageCollectorObjectList:=nil;
end;

function TBESENGarbageCollectorObjectList.Contains(AObject:TBESENGarbageCollectorObject):boolean;
begin
 result:=AObject.GarbageCollectorObjectList=self;
end;

procedure TBESENGarbageCollectorObjectList.Push(AObject:TBESENGarbageCollectorObject);
begin
 Add(AObject);
end;

function TBESENGarbageCollectorObjectList.Pop:TBESENGarbageCollectorObject;
begin
 result:=Last;
 if assigned(result) then begin
  Remove(result);
 end;
end;

constructor TBESENGarbageCollector.Create(AInstance:TObject);
begin
 inherited Create(AInstance);
 First:=nil;
 Last:=nil;
 RootObjectList:=TBESENGarbageCollectorRootObjectList.Create(Instance);
 ProtectedObjectList:=TBESENGarbageCollectorProtectedObjectList.Create(Instance);
 WhiteObjectList:=TBESENGarbageCollectorObjectList.Create(Instance);
 GrayObjectList:=TBESENGarbageCollectorObjectList.Create(Instance);
 BlackObjectList:=TBESENGarbageCollectorObjectList.Create(Instance);
 IsSweeping:=false;
 State:=bgcsINIT;
 TriggerCounter:=0;
 SweepCount:=BESEN_GC_SWEEPCOUNT;
 MarkCount:=BESEN_GC_MARKCOUNT;
 TriggerCountPerCollect:=BESEN_GC_TRIGGERCOUNT_PER_COLLECT;
end;

destructor TBESENGarbageCollector.Destroy;
begin
 Clear;
 RootObjectList.Destroy;
 ProtectedObjectList.Destroy;
 WhiteObjectList.Destroy;
 GrayObjectList.Destroy;
 BlackObjectList.Destroy;
 inherited Destroy;
end;

procedure TBESENGarbageCollector.Clear;
var CurrentObject:TBESENGarbageCollectorObject;
begin
 CurrentObject:=First;
 while assigned(CurrentObject) do begin
  FinalizeObjectForSweeping(CurrentObject);
  CurrentObject:=CurrentObject.GarbageCollectorNext;
 end;
 while assigned(First) do begin
  First.Destroy;
 end;
 First:=nil;
 Last:=nil;
 RootObjectList.Clear;
 ProtectedObjectList.Clear;
 WhiteObjectList.Clear;
 GrayObjectList.Clear;
 BlackObjectList.Clear;
 CurrentRootObject:=nil;
 CurrentProtectedObject:=nil;
 CurrentMarkObject:=nil;
 CurrentSweepObject:=nil;
 TriggerCounter:=0;
 State:=bgcsINIT;
 IsSweeping:=false;
end;

procedure TBESENGarbageCollector.Reset;
begin
 Flip;
 CurrentRootObject:=nil;
 CurrentProtectedObject:=nil;
 CurrentSweepObject:=nil;
 State:=bgcsINIT;
end;

procedure TBESENGarbageCollector.WhiteIt(AObject:TBESENGarbageCollectorObject);
begin
 if assigned(AObject) and not WhiteObjectList.Contains(AObject) then begin
  if assigned(AObject.GarbageCollectorObjectList) then begin
   AObject.GarbageCollectorObjectList.Remove(AObject);
   AObject.GarbageCollectorObjectList:=nil;
  end;
  WhiteObjectList.Push(AObject);
 end;
end;

procedure TBESENGarbageCollector.ForceGrayIt(AObject:TBESENGarbageCollectorObject);
begin
 if assigned(AObject) and not GrayObjectList.Contains(AObject) then begin
  if assigned(AObject.GarbageCollectorObjectList) then begin
   AObject.GarbageCollectorObjectList.Remove(AObject);
   AObject.GarbageCollectorObjectList:=nil;
  end;
  GrayObjectList.Push(AObject);
 end;
end;

procedure TBESENGarbageCollector.GrayIt(AObject:TBESENGarbageCollectorObject);
begin
 if assigned(AObject) and not (GrayObjectList.Contains(AObject) or BlackObjectList.Contains(AObject)) then begin
  if assigned(AObject.GarbageCollectorObjectList) then begin
   AObject.GarbageCollectorObjectList.Remove(AObject);
   AObject.GarbageCollectorObjectList:=nil;
  end;
  GrayObjectList.Push(AObject);
 end;
end;

procedure TBESENGarbageCollector.BlackIt(AObject:TBESENGarbageCollectorObject);
begin
 if assigned(AObject) and not BlackObjectList.Contains(AObject) then begin
  if assigned(AObject.GarbageCollectorObjectList) then begin
   AObject.GarbageCollectorObjectList.Remove(AObject);
   AObject.GarbageCollectorObjectList:=nil;
  end;
  BlackObjectList.Push(AObject);
 end;
end;

procedure TBESENGarbageCollector.Protect(AObject:TBESENGarbageCollectorObject);
begin
 AddProtected(AObject);
end;

procedure TBESENGarbageCollector.Unprotect(AObject:TBESENGarbageCollectorObject);
begin
 RemoveProtected(AObject);
end;

procedure TBESENGarbageCollector.GrayValue(var Value:TBESENValue);
begin
 case Value.ValueType of
  bvtOBJECT:begin
   if assigned(Value.Obj) then begin
    GrayIt(TBESENObject(Value.Obj));
   end;
  end;
  bvtREFERENCE:begin
   case Value.ReferenceBase.ValueType of
    brbvtOBJECT:begin
     if assigned(Value.ReferenceBase.Obj) then begin
      GrayIt(TBESENObject(Value.ReferenceBase.Obj));
     end;
    end;
    brbvtENVREC:begin
     if assigned(Value.ReferenceBase.EnvRec) then begin
      GrayIt(TBESENEnvironmentRecord(Value.ReferenceBase.EnvRec));
     end;
    end;
   end;
  end;
 end;
end;

procedure TBESENGarbageCollector.FinalizeValue(var Value:TBESENValue);
begin
 Value:=BESENEmptyValue;
end;

procedure TBESENGarbageCollector.Mark(AObject:TBESENGarbageCollectorObject);
begin
 if assigned(AObject) then begin
  AObject.Mark;
 end;
end;

procedure TBESENGarbageCollector.FinalizeObjectForSweeping(AObject:TBESENGarbageCollectorObject);
begin
 if assigned(AObject) then begin
  AObject.Finalize;
 end;
end;

procedure TBESENGarbageCollector.Flip;
var TempObjectList:TBESENGarbageCollectorObjectList;
begin
 TempObjectList:=WhiteObjectList;
 WhiteObjectList:=BlackObjectList;
 BlackObjectList:=TempObjectList;
end;

procedure TBESENGarbageCollector.TriggerCollect;
begin
 inc(TriggerCounter);
 if TriggerCounter>=TriggerCountPerCollect then begin
  TriggerCounter:=0;
  Collect;
 end;
end;

function TBESENGarbageCollector.Collect:boolean;
 function MarkRootObjectChildren:boolean;
 var i:integer;
 begin
  result:=false;
  for i:=1 to MarkCount do begin
   if assigned(CurrentRootObject) then begin
    BlackIt(CurrentRootObject);
    Mark(CurrentRootObject);
    CurrentRootObject:=CurrentRootObject.GarbageCollectorRootObjectListNext;
    result:=assigned(CurrentRootObject);
   end else begin
    break;
   end;
  end;
 end;
 function MarkProtectedObjectChildren:boolean;
 var i:integer;
 begin
  result:=false;
  for i:=1 to MarkCount do begin
   if assigned(CurrentProtectedObject) then begin
    BlackIt(CurrentProtectedObject);
    Mark(CurrentProtectedObject);
    CurrentProtectedObject:=CurrentProtectedObject.GarbageCollectorProtectedObjectListNext;
    result:=assigned(CurrentProtectedObject);
   end else begin
    break;
   end;
  end;
 end;
 function MarkProgramNodes:boolean;
 var Current:PBESENPointerSelfBalancedTreeNode;
     Node:TBESENASTNodeProgram;
 begin
  result:=false;
  Current:=TBESEN(Instance).ProgramNodes.FirstKey;
  while assigned(Current) do begin
   Node:=TBESENASTNodeProgram(Current^.Key);
   if assigned(Node) and assigned(Node.Body) and assigned(Node.Body.Code) then begin
    TBESENCode(Node.Body.Code).Mark;
    result:=true;
   end;
   Current:=Current^.NextKey;
  end;
 end;
 function MarkContexts:boolean;
 var i:integer;
 begin
  result:=false;
  for i:=1 to MarkCount do begin
   if assigned(CurrentContext) then begin
    TBESENContext(CurrentContext).Mark;
    CurrentContext:=TBESENContext(CurrentContext).Next;
    result:=assigned(CurrentContext);
   end else begin
    break;
   end;
  end;
 end;
 function BlackGrayObjectsAndMarkTheirChildren:boolean;
 var i:integer;
 begin
  result:=false;
  for i:=1 to MarkCount do begin
   CurrentMarkObject:=GrayObjectList.Pop;
   if assigned(CurrentMarkObject) then begin
    Mark(CurrentMarkObject);
    BlackIt(CurrentMarkObject);
    result:=assigned(GrayObjectList.Last);
   end else begin
    break;
   end;
  end;
 end;
 function CheckWhiteObjectLockReferenceCounters:boolean;
 var i:integer;
     NextObject:TBESENGarbageCollectorObject;
 begin
  result:=false;
  for i:=1 to SweepCount do begin
   if assigned(CurrentSweepObject) then begin
    NextObject:=CurrentSweepObject.GarbageCollectorObjectListNext;
    if CurrentSweepObject.GarbageCollectorLockReferenceCounter>0 then begin
     GrayIt(CurrentSweepObject);
    end;
    CurrentSweepObject:=NextObject;
    result:=assigned(CurrentSweepObject);
   end else begin
    break;
   end;
  end;
 end;
 function FinalizeWhiteObjectsForSweeping:boolean;
 var i:integer;
 begin
  result:=false;
  for i:=1 to SweepCount do begin
   if assigned(CurrentSweepObject) then begin
    FinalizeObjectForSweeping(CurrentSweepObject);
    CurrentSweepObject:=CurrentSweepObject.GarbageCollectorObjectListNext;
    result:=assigned(CurrentSweepObject);
   end else begin
    break;
   end;
  end;
 end;
 function SweepWhiteObjects:boolean;
 var i:integer;
 begin
  result:=false;
  for i:=1 to SweepCount do begin
   if assigned(WhiteObjectList.First) then begin
    IsSweeping:=true;
    WhiteObjectList.First.Destroy;
    IsSweeping:=false;
    result:=assigned(WhiteObjectList.First);
   end else begin
    break;
   end;
  end;
 end;
begin
 result:=false;
 while true do begin
  case State of
   bgcsINIT:begin
    State:=bgcsMARKROOTS;
    CurrentRootObject:=RootObjectList.First;
   end;
   bgcsMARKROOTS:begin
    if MarkRootObjectChildren then begin
     result:=true;
     break;
    end else begin
     CurrentProtectedObject:=ProtectedObjectList.First;
     State:=bgcsMARKPROTECTED;
    end;
   end;
   bgcsMARKPROTECTED:begin
    if MarkProtectedObjectChildren then begin
     result:=true;
     break;
    end else begin
     State:=bgcsMARKPROGRAMNODES;
    end;
   end;
   bgcsMARKPROGRAMNODES:begin
    MarkProgramNodes;
    CurrentContext:=TBESEN(Instance).ContextFirst;
    State:=bgcsMARKCONTEXTS;
   end;
   bgcsMARKCONTEXTS:begin
    if MarkContexts then begin
     result:=true;
     break;
    end else begin
     State:=bgcsMARKGRAYS;
    end;
   end;
   bgcsMARKGRAYS:begin
    if BlackGrayObjectsAndMarkTheirChildren then begin
     result:=true;
     break;
    end else begin
     CurrentSweepObject:=WhiteObjectList.First;
     State:=bgcsCHECKFORLOCKEDWHITES;
    end;
   end;
   bgcsCHECKFORLOCKEDWHITES:begin
    if CheckWhiteObjectLockReferenceCounters then begin
     result:=true;
     break;
    end else begin
     if assigned(GrayObjectList.First) then begin
      State:=bgcsMARKLOCKEDWHITES;
     end else begin
      CurrentSweepObject:=WhiteObjectList.First;
      if assigned(CurrentSweepObject) then begin
       State:=bgcsFINALIZEWHITES;
      end else begin
       State:=bgcsDONE;
      end;
     end;
    end;
   end;
   bgcsMARKLOCKEDWHITES:begin
    if BlackGrayObjectsAndMarkTheirChildren then begin
     result:=true;
     break;
    end else begin
     CurrentSweepObject:=WhiteObjectList.First;
     if assigned(CurrentSweepObject) then begin
      State:=bgcsFINALIZEWHITES;
     end else begin
      State:=bgcsDONE;
     end;
    end;
   end;
   bgcsFINALIZEWHITES:begin
    if FinalizeWhiteObjectsForSweeping then begin
     result:=true;
     break;
    end else begin
     if assigned(WhiteObjectList.First) then begin
      State:=bgcsSWEEPWHITES;
     end else begin
      State:=bgcsDONE;
     end;
    end;
   end;
   bgcsSWEEPWHITES:begin
    if SweepWhiteObjects then begin
     result:=true;
     break;
    end else begin
     State:=bgcsDONE;
    end;
   end;
   bgcsDONE:begin
    Flip;
    State:=bgcsINIT;
    result:=false;
    break;
   end;
  end;
 end;
end;

procedure TBESENGarbageCollector.CollectAll;
begin
 while Collect do begin
 end;
end;

procedure TBESENGarbageCollector.Use(AObject:TBESENGarbageCollectorObject);
begin
 ForceGrayIt(AObject);
end;

procedure TBESENGarbageCollector.Add(AObject:TBESENGarbageCollectorObject);
begin
 GrayIt(AObject);
end;

procedure TBESENGarbageCollector.AddRoot(AObject:TBESENGarbageCollectorObject);
begin
 RootObjectList.Add(AObject);
 State:=bgcsINIT;
end;

procedure TBESENGarbageCollector.RemoveRoot(AObject:TBESENGarbageCollectorObject);
begin
 State:=bgcsINIT;
 if RootObjectList.Contains(AObject) then begin
  RootObjectList.Remove(AObject);
  ForceGrayIt(AObject);
 end;
end;

procedure TBESENGarbageCollector.AddProtected(AObject:TBESENGarbageCollectorObject);
begin
 GrayIt(AObject);
 ProtectedObjectList.Add(AObject);
 State:=bgcsINIT;
end;

procedure TBESENGarbageCollector.RemoveProtected(AObject:TBESENGarbageCollectorObject);
begin
 State:=bgcsINIT;
 if ProtectedObjectList.Contains(AObject) then begin
  ProtectedObjectList.Remove(AObject);
  ForceGrayIt(AObject);
 end;
end;

procedure TBESENGarbageCollector.LockObject(Obj:TBESENGarbageCollectorObject);
begin
 if assigned(Obj) then begin
  Obj.GarbageCollectorLock;
 end;
end;

procedure TBESENGarbageCollector.UnlockObject(Obj:TBESENGarbageCollectorObject);
begin
 if assigned(Obj) then begin
  Obj.GarbageCollectorUnlock;
 end;
end;

procedure TBESENGarbageCollector.LockValue(const Value:TBESENValue);
begin
 case Value.ValueType of
  bvtOBJECT:begin
   if assigned(Value.Obj) then begin
    LockObject(TBESENObject(Value.Obj));
   end;
  end;
  bvtREFERENCE:begin
   case Value.ReferenceBase.ValueType of
    brbvtOBJECT:begin
     if assigned(Value.ReferenceBase.Obj) then begin
      LockObject(TBESENObject(Value.ReferenceBase.Obj));
     end;
    end;
    brbvtENVREC:begin
     if assigned(Value.ReferenceBase.EnvRec) then begin
      LockObject(TBESENEnvironmentRecord(Value.ReferenceBase.EnvRec));
     end;
    end;
   end;
  end;
 end;
end;

procedure TBESENGarbageCollector.UnlockValue(const Value:TBESENValue);
begin
 case Value.ValueType of
  bvtOBJECT:begin
   if assigned(Value.Obj) then begin
    UnlockObject(TBESENObject(Value.Obj));
   end;
  end;
  bvtREFERENCE:begin
   case Value.ReferenceBase.ValueType of
    brbvtOBJECT:begin
     if assigned(Value.ReferenceBase.Obj) then begin
      UnlockObject(TBESENObject(Value.ReferenceBase.Obj));
     end;
    end;
    brbvtENVREC:begin
     if assigned(Value.ReferenceBase.EnvRec) then begin
      UnlockObject(TBESENEnvironmentRecord(Value.ReferenceBase.EnvRec));
     end;
    end;
   end;
  end;
 end;
end;

end.
