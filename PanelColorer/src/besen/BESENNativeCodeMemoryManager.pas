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
unit BESENNativeCodeMemoryManager;
{$i BESEN.inc}

interface

uses {$ifdef windows}Windows,MMSystem,{$endif}{$ifdef unix}dl,BaseUnix,Unix,UnixType,{$endif}
     SysUtils,Classes,BESENConstants,BESENTypes;

type PBESENNativeCodeMemoryManagerBlock=^TBESENNativeCodeMemoryManagerBlock;
     TBESENNativeCodeMemoryManagerBlock=packed record
      Signature:ptruint;
      Previous:PBESENNativeCodeMemoryManagerBlock;
      Next:PBESENNativeCodeMemoryManagerBlock;
      Size:ptruint;
     end;

     PBESENNativeCodeMemoryManagerBlockContainer=^TBESENNativeCodeMemoryManagerBlockContainer;
     TBESENNativeCodeMemoryManagerBlockContainer=record
      Previous:PBESENNativeCodeMemoryManagerBlockContainer;
      Next:PBESENNativeCodeMemoryManagerBlockContainer;
      Base:pointer;
      Size:ptruint;
      Used:ptruint;
      First:PBESENNativeCodeMemoryManagerBlock;
      Last:PBESENNativeCodeMemoryManagerBlock;
     end;

     TBESENNativeCodeMemoryManager=class
      private
       PageSize:ptruint;
       Alignment:ptruint;
       function AllocateBlockContainer(BlockContainerSize:ptruint):PBESENNativeCodeMemoryManagerBlockContainer;
       procedure FreeBlockContainer(BlockContainer:PBESENNativeCodeMemoryManagerBlockContainer);
      public
       First,Last:PBESENNativeCodeMemoryManagerBlockContainer;
       constructor Create;
       destructor Destroy; override;
       function GetMemory(Size:ptruint):pointer;
       procedure FreeMemory(p:pointer);
       function ReallocMemory(p:pointer;Size:ptruint):pointer;
     end;

{$ifdef HasJIT}
{$ifdef unix}
var fpmprotect:function(__addr:pointer;__len:cardinal;__prot:longint):longint; cdecl;// external 'c' name 'mprotect';
{$endif}
{$endif}

implementation

uses BESENUtils;

const bncmmMemoryBlockSignature:ptruint={$ifdef cpu64}$1337bab3deadc0d3{$else}$deadc0d3{$endif};

constructor TBESENNativeCodeMemoryManager.Create;
{$ifdef windows}
var SystemInfo:TSystemInfo;
{$else}
{$ifdef unix}
{$endif}
{$endif}
begin
 inherited Create;
{$ifdef windows}
 GetSystemInfo(SystemInfo);
 PageSize:=BESENRoundUpToPowerOf2(SystemInfo.dwPageSize);
{$else}
{$ifdef unix}
 PageSize:=4096;
{$else}
 PageSize:=4096;
{$endif}
{$endif}
{$ifdef cpu386}
 Alignment:=16;
{$else}
{$ifdef cpuamd64}
 Alignment:=16;
{$else}
{$ifdef cpuarm}
 Alignment:=16;
{$else}
 Alignment:=PageSize;
{$endif}
{$endif}
{$endif}
 First:=nil;
 Last:=nil;
end;

destructor TBESENNativeCodeMemoryManager.Destroy;
begin
 while assigned(First) do begin
  FreeBlockContainer(First);
 end;
 inherited Destroy;
end;

function TBESENNativeCodeMemoryManager.AllocateBlockContainer(BlockContainerSize:ptruint):PBESENNativeCodeMemoryManagerBlockContainer;
var Size:ptruint;
    Block:PBESENNativeCodeMemoryManagerBlock;
begin
 if BlockContainerSize>0 then begin
  Size:=BESENRoundUpToMask(BlockContainerSize,PageSize);
  New(result);
{$ifdef windows}
  result^.Base:=VirtualAlloc(nil,Size,MEM_COMMIT,PAGE_EXECUTE_READWRITE);
{$else}
{$ifdef unix}
  result^.Base:=fpmmap(nil,Size,PROT_READ or PROT_WRITE or PROT_EXEC,MAP_PRIVATE or MAP_ANONYMOUS,-1,0);
{$else}
  GetMem(result^.Base,Size);
{$endif}
{$endif}
  result^.Size:=Size;
  result^.Used:=sizeof(TBESENNativeCodeMemoryManagerBlock)*2;
  if assigned(Last) then begin
   Last^.Next:=result;
   result^.Previous:=Last;
   Last:=result;
   result^.Next:=nil;
  end else begin
   First:=result;
   Last:=result;
   result^.Previous:=nil;
   result^.Next:=nil;
  end;
  FillChar(result^.Base^,result^.Size,#0);
  result^.First:=result^.Base;
  result^.Last:=pointer(@PBESENByteArray(result^.Base)[result^.Size-sizeof(TBESENNativeCodeMemoryManagerBlock)]);
  Block:=result^.First;
  Block^.Signature:=bncmmMemoryBlockSignature;
  Block^.Previous:=nil;
  Block^.Next:=result^.Last;
  Block^.Size:=0;
  Block:=result^.Last;
  Block^.Signature:=bncmmMemoryBlockSignature;
  Block^.Previous:=result^.First;
  Block^.Next:=nil;
  Block^.Size:=0;
 end else begin
  result:=nil;
 end;
end;

procedure TBESENNativeCodeMemoryManager.FreeBlockContainer(BlockContainer:PBESENNativeCodeMemoryManagerBlockContainer);
begin
 if assigned(BlockContainer^.Previous) then begin
  BlockContainer^.Previous^.Next:=BlockContainer^.Next;
 end else begin
  First:=BlockContainer^.Next;
 end;
 if assigned(BlockContainer^.Next) then begin
  BlockContainer^.Next^.Previous:=BlockContainer^.Previous;
 end else begin
  Last:=BlockContainer^.Previous;
 end;
{$ifdef windows}
 VirtualFree(BlockContainer^.Base,0,MEM_RELEASE);
{$else}
{$ifdef unix}
 fpmunmap(BlockContainer^.Base,BlockContainer^.Size);
{$else}
 FreeMem(BlockContainer^.Base);
{$endif}
{$endif}
 Dispose(BlockContainer);
end;

function TBESENNativeCodeMemoryManager.GetMemory(Size:ptruint):pointer;
var BlockContainer:PBESENNativeCodeMemoryManagerBlockContainer;
    CurrentBlock,NewBlock:PBESENNativeCodeMemoryManagerBlock;
    DestSize,BlockContainerSize:ptruint;
begin
 result:=nil;
 if Size>0 then begin
  DestSize:=Size+sizeof(TBESENNativeCodeMemoryManagerBlock);
  BlockContainer:=First;
  while true do begin
   while assigned(BlockContainer) do begin
    if (BlockContainer^.Used+DestSize)<=BlockContainer^.Size then begin
     CurrentBlock:=BlockContainer^.First;
     while assigned(CurrentBlock) and (CurrentBlock^.Signature=bncmmMemoryBlockSignature) and assigned(CurrentBlock^.Next) do begin
      NewBlock:=pointer(ptruint(BESENRoundUpToMask(ptruint(pointer(@PBESENByteArray(CurrentBlock)[(sizeof(TBESENNativeCodeMemoryManagerBlock)*2)+CurrentBlock^.Size])),Alignment)-sizeof(TBESENNativeCodeMemoryManagerBlock)));
      if (ptruint(CurrentBlock^.Next)-ptruint(NewBlock))>=DestSize then begin
       NewBlock^.Signature:=bncmmMemoryBlockSignature;
       NewBlock^.Previous:=CurrentBlock;
       NewBlock^.Next:=CurrentBlock^.Next;
       NewBlock^.Size:=Size;
       CurrentBlock^.Next^.Previous:=NewBlock;
       CurrentBlock^.Next:=NewBlock;
       result:=pointer(@PBESENByteArray(NewBlock)[sizeof(TBESENNativeCodeMemoryManagerBlock)]);
       inc(BlockContainer^.Used,DestSize);
       exit;
      end else begin
       CurrentBlock:=CurrentBlock^.Next;
      end;
     end;
    end;
    BlockContainer:=BlockContainer^.Next;
   end;
   if DestSize<=bncmmMINBLOCKCONTAINERSIZE then begin
    BlockContainerSize:=bncmmMINBLOCKCONTAINERSIZE;
   end else begin
    BlockContainerSize:=BESENRoundUpToPowerOf2(DestSize);
   end;
   BlockContainer:=AllocateBlockContainer(BlockContainerSize);
   if not assigned(BlockContainer) then begin
    break;
   end;
  end;
 end;
end;

procedure TBESENNativeCodeMemoryManager.FreeMemory(p:pointer);
var BlockContainer:PBESENNativeCodeMemoryManagerBlockContainer;
    CurrentBlock:PBESENNativeCodeMemoryManagerBlock;
begin
 BlockContainer:=First;
 while assigned(BlockContainer) do begin
  if ((ptruint(BlockContainer^.Base)+sizeof(TBESENNativeCodeMemoryManagerBlock))<=ptruint(p)) and ((ptruint(p)+sizeof(TBESENNativeCodeMemoryManagerBlock))<(ptruint(BlockContainer^.Base)+BlockContainer^.Size)) then begin
   CurrentBlock:=pointer(ptruint(ptruint(p)-sizeof(TBESENNativeCodeMemoryManagerBlock)));
   if (CurrentBlock^.Signature=bncmmMemoryBlockSignature) and (CurrentBlock<>BlockContainer^.First) and (CurrentBlock<>BlockContainer^.Last) then begin
    dec(BlockContainer^.Used,CurrentBlock^.Size+sizeof(TBESENNativeCodeMemoryManagerBlock));
    CurrentBlock^.Signature:=0;
    CurrentBlock^.Previous^.Next:=CurrentBlock^.Next;
    CurrentBlock^.Next^.Previous:=CurrentBlock^.Previous;
    if (assigned(BlockContainer^.First) and (BlockContainer^.First^.Next=BlockContainer^.Last)) or not assigned(BlockContainer^.First) then begin
     FreeBlockContainer(BlockContainer);
    end;
    exit;
   end;
  end;
  BlockContainer:=BlockContainer^.Next;
 end;
end;

function TBESENNativeCodeMemoryManager.ReallocMemory(p:pointer;Size:ptruint):pointer;
var BlockContainer:PBESENNativeCodeMemoryManagerBlockContainer;
    CurrentBlock:PBESENNativeCodeMemoryManagerBlock;
    DestSize:ptruint;
begin
 result:=nil;
 if assigned(p) then begin
  if Size=0 then begin
   FreeMemory(p);
  end else begin
   DestSize:=Size+sizeof(TBESENNativeCodeMemoryManagerBlock);
   BlockContainer:=First;
   while assigned(BlockContainer) do begin
    if ((ptruint(BlockContainer^.Base)+sizeof(TBESENNativeCodeMemoryManagerBlock))<=ptruint(p)) and ((ptruint(p)+sizeof(TBESENNativeCodeMemoryManagerBlock))<(ptruint(BlockContainer^.Base)+BlockContainer^.Size)) then begin
     CurrentBlock:=pointer(ptruint(ptruint(p)-sizeof(TBESENNativeCodeMemoryManagerBlock)));
     if (CurrentBlock^.Signature=bncmmMemoryBlockSignature) and (CurrentBlock<>BlockContainer^.First) and (CurrentBlock<>BlockContainer^.Last) then begin
      if (ptruint(CurrentBlock^.Next)-ptruint(CurrentBlock))>=DestSize then begin
       CurrentBlock^.Size:=Size;
       result:=p;
       exit;
      end else begin
       result:=GetMemory(Size);
       if assigned(result) then begin
        if CurrentBlock^.Size<Size then begin
         Move(p^,result^,CurrentBlock^.Size);
        end else begin
         Move(p^,result^,Size);
        end;
       end;
       FreeMemory(p);
       exit;
      end;
     end;
    end;
    BlockContainer:=BlockContainer^.Next;
   end;
  end;
  FreeMemory(p);
 end else if Size<>0 then begin
  result:=GetMemory(Size);
 end;
end;

procedure InitBESEN;
begin
{$ifdef HasJIT}
{$ifdef unix}
{$ifdef darwin}
 fpmprotect:=dlsym(dlopen('libc.dylib',RTLD_NOW),'mprotect');
{$else}
 fpmprotect:=dlsym(dlopen('libc.so',RTLD_NOW),'mprotect');
{$endif}
 if not assigned(fpmprotect) then begin
  raise Exception.Create('Importing of mprotect from libc.so failed!');
 end;
{$endif}
{$endif}
end;

procedure DoneBESEN;
begin
end;

initialization
 InitBESEN;
finalization
 DoneBESEN;
end.
