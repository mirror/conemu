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
unit BESENHashMap;
{$i BESEN.inc}

interface

uses BESENConstants,BESENTypes,BESENStringUtils,BESENHashUtils;

type PBESENHashMapItem=^TBESENHashMapItem;
     TBESENHashMapItem=record
      Previous,Next,HashPrevious,HashNext:PBESENHashMapItem;
      Hash:TBESENHash;
      Key:TBESENString;
      Value:int64;
      Ptr:pointer;
     end;

     TBESENHashMapHashBucket=record
      HashFirst,HashLast:PBESENHashMapItem;
     end;

     TBESENHashMapHashBuckets=array of TBESENHashMapHashBucket;

     TBESENHashMap=class
      private
       LastUsedItem:PBESENHashMapItem;
       procedure GrowAndRehashIfNeeded;
      public
       First,Last:PBESENHashMapItem;
       HashBuckets:TBESENHashMapHashBuckets;
       HashSize:longword;
       HashSizeMask:longword;
       HashedItems:longword;
       HashBucketsUsed:longword;
       constructor Create;
       destructor Destroy; override;
       procedure Clear;
       function GetKey(const Key:TBESENString;Hash:TBESENHash=0):PBESENHashMapItem;
       function NewKey(const Key:TBESENString;Force:boolean=false;Hash:TBESENHash=0):PBESENHashMapItem;
       function DeleteKey(const Item:PBESENHashMapItem):TBESENBoolean;
     end;

implementation

constructor TBESENHashMap.Create;
var Hash:TBESENHash;
begin
 inherited Create;
 FillChar(HashBuckets,sizeof(TBESENHashMapHashBuckets),#0);
 First:=nil;
 Last:=nil;
 HashSize:=256;
 HashSizeMask:=HashSize-1;
 HashedItems:=0;
 HashBucketsUsed:=0;
 HashBuckets:=nil;
 SetLength(HashBuckets,HashSize);
 for Hash:=0 to HashSizeMask do begin
  HashBuckets[Hash].HashFirst:=nil;
  HashBuckets[Hash].HashLast:=nil;
 end;
 LastUsedItem:=nil;
end;

destructor TBESENHashMap.Destroy;
begin
 Clear;
 SetLength(HashBuckets,0);
 inherited Destroy;
end;

procedure TBESENHashMap.Clear;
var Hash:TBESENHash;
    Item,NextItem:PBESENHashMapItem;
begin
 Item:=First;
 while assigned(Item) do begin
  NextItem:=Item^.Next;
  Item^.Next:=nil;
  Item^.Key:='';
  Dispose(Item);
  Item:=NextItem;
 end;
 First:=nil;
 Last:=nil;
 LastUsedItem:=nil;
 HashSize:=256;
 HashSizeMask:=HashSize-1;
 HashedItems:=0;
 HashBucketsUsed:=0;
 SetLength(HashBuckets,HashSize);
 for Hash:=0 to HashSizeMask do begin
  HashBuckets[Hash].HashFirst:=nil;
  HashBuckets[Hash].HashLast:=nil;
 end;
end;

procedure TBESENHashMap.GrowAndRehashIfNeeded;
var Hash:TBESENHash;
    Item:PBESENHashMapItem;
begin
 if (HashSize<BESENHashMaxSize) and (HashedItems>=(HashBucketsUsed*BESENHashItemsPerBucketsThreshold)) then begin
  LastUsedItem:=nil;
  for Hash:=0 to HashSizeMask do begin
   HashBuckets[Hash].HashFirst:=nil;
   HashBuckets[Hash].HashLast:=nil;
  end;
  inc(HashSize,HashSize);
  if HashSize>BESENHashMaxSize then begin
   HashSize:=BESENHashMaxSize;
  end;
  HashSizeMask:=HashSize-1;
  SetLength(HashBuckets,HashSize);
  for Hash:=0 to HashSizeMask do begin
   HashBuckets[Hash].HashFirst:=nil;
   HashBuckets[Hash].HashLast:=nil;
  end;
  HashedItems:=0;
  Item:=First;
  while assigned(Item) do begin
   inc(HashedItems);
   Item^.HashPrevious:=nil;
   Item^.HashNext:=nil;
   Item:=Item^.Next;
  end;
  HashBucketsUsed:=0;
  Item:=First;
  while assigned(Item) do begin
   Hash:=BESENHashKey(Item^.Key) and HashSizeMask;
   Item^.Hash:=Hash;
   if assigned(HashBuckets[Hash].HashLast) then begin
    HashBuckets[Hash].HashLast^.HashNext:=Item;
    Item^.HashPrevious:=HashBuckets[Hash].HashLast;
    HashBuckets[Hash].HashLast:=Item;
    Item^.HashNext:=nil;
   end else begin
    inc(HashBucketsUsed);
    HashBuckets[Hash].HashFirst:=Item;
    HashBuckets[Hash].HashLast:=Item;
    Item^.HashPrevious:=nil;
    Item^.HashNext:=nil;
   end;
   Item:=Item^.Next;
  end;
 end;
end;

function TBESENHashMap.GetKey(const Key:TBESENString;Hash:TBESENHash=0):PBESENHashMapItem;
begin
 if assigned(LastUsedItem) and (LastUsedItem^.Key=Key) then begin
  result:=LastUsedItem;
  Hash:=result^.Hash;
 end else begin
  if Hash=0 then begin
   Hash:=BESENHashKey(Key);
  end;
  Hash:=Hash and HashSizeMask;
  result:=HashBuckets[Hash].HashFirst;
  while assigned(result) and (result^.Key<>Key) do begin
   result:=result^.HashNext;
  end;
 end;
 if assigned(result) then begin
  LastUsedItem:=result;
  if HashBuckets[Hash].HashFirst<>result then begin
   if assigned(result^.HashPrevious) then begin
    result^.HashPrevious^.HashNext:=result^.HashNext;
   end;
   if assigned(result^.HashNext) then begin
    result^.HashNext^.HashPrevious:=result^.HashPrevious;
   end else if HashBuckets[Hash].HashLast=result then begin
    HashBuckets[Hash].HashLast:=result^.HashPrevious;
   end;
   HashBuckets[Hash].HashFirst^.HashPrevious:=result;
   result^.HashNext:=HashBuckets[Hash].HashFirst;
   result^.HashPrevious:=nil;
   HashBuckets[Hash].HashFirst:=result;
  end;
 end;
end;

function TBESENHashMap.NewKey(const Key:TBESENString;Force:boolean=false;Hash:TBESENHash=0):PBESENHashMapItem;
begin
 if Force then begin
  result:=nil;
  if Hash=0 then begin
   Hash:=BESENHashKey(Key);
  end;
  Hash:=Hash and HashSizeMask;
 end else if assigned(LastUsedItem) and (LastUsedItem^.Key=Key) then begin
  result:=LastUsedItem;
  Hash:=result^.Hash;
 end else begin
  if Hash=0 then begin
   Hash:=BESENHashKey(Key);
  end;
  Hash:=Hash and HashSizeMask;
  result:=HashBuckets[Hash].HashFirst;
  if not assigned(result) then begin
   inc(HashBucketsUsed);
  end;
  while assigned(result) and not (result^.Key<>Key) do begin
   result:=result^.HashNext;
  end;
 end;
 if not assigned(result) then begin
  inc(HashedItems);
  New(result);
  fillchar(result^,sizeof(TBESENHashMapItem),#0);
  result^.Hash:=Hash;
  result^.Key:=Key;
  if assigned(HashBuckets[Hash].HashLast) then begin
   HashBuckets[Hash].HashLast^.HashNext:=result;
   result^.HashPrevious:=HashBuckets[Hash].HashLast;
   result^.HashNext:=nil;
   HashBuckets[Hash].HashLast:=result;
  end else begin
   HashBuckets[Hash].HashFirst:=result;
   HashBuckets[Hash].HashLast:=result;
   result^.HashPrevious:=nil;
   result^.HashNext:=nil;
  end;
  if assigned(Last) then begin
   Last^.Next:=result;
   result^.Previous:=Last;
   result^.Next:=nil;
   Last:=result;
  end else begin
   First:=result;
   Last:=result;
   result^.Previous:=nil;
   result^.Next:=nil;
  end;
  LastUsedItem:=result;
 end;
 GrowAndRehashIfNeeded;
end;

function TBESENHashMap.DeleteKey(const Item:PBESENHashMapItem):TBESENBoolean;
begin
 result:=assigned(Item);
 if result then begin
  if LastUsedItem=Item then begin
   if assigned(Item^.Next) then begin
    LastUsedItem:=Item^.Next;
   end else begin
    LastUsedItem:=Item^.Previous;
   end;
  end;
  if assigned(Item^.Previous) then begin
   Item^.Previous^.Next:=Item^.Next;
  end else if First=Item then begin
   First:=Item^.Next;
  end;
  if assigned(Item^.Next) then begin
   Item^.Next^.Previous:=Item^.Previous;
  end else if Last=Item then begin
   Last:=Item^.Previous;
  end;
  Item^.Next:=nil;
  Item^.Previous:=nil;
  if assigned(Item^.HashPrevious) then begin
   Item^.HashPrevious^.HashNext:=Item^.HashNext;
  end else if HashBuckets[Item^.Hash].HashFirst=Item then begin
   HashBuckets[Item^.Hash].HashFirst:=Item^.HashNext;
  end;
  if assigned(Item^.HashNext) then begin
   Item^.HashNext^.HashPrevious:=Item^.HashPrevious;
  end else if HashBuckets[Item^.Hash].HashLast=Item then begin
   HashBuckets[Item^.Hash].HashLast:=Item^.HashPrevious;
  end;
  Item^.HashNext:=nil;
  Item^.HashPrevious:=nil;
  Item^.Key:='';
  Dispose(Item);
 end;
end;

end.
