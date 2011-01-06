(***************************************************7****************************
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
unit BESENCodeJITx86;
{$i BESEN.inc}

interface

uses {$ifdef windows}Windows,MMSystem,{$endif}{$ifdef unix}dl,BaseUnix,Unix,
     UnixType,{$endif}BESENConstants,BESENTypes;

function BESENGenerateNativeCode(ACodeContext:TObject):TBESENBoolean;
function BESENExecuteNativeCode(ACodeContext:TObject):TBESENBoolean; {$ifdef UseRegister}register;{$endif}

implementation

uses BESEN,BESENValue,BESENASTNodes,BESENCode,BESENCodeContext,BESENContext,BESENOpcodes,
     BESENGarbageCollector,BESENNumberUtils,BESENLexicalEnvironment,
     BESENDeclarativeEnvironmentRecord,BESENNativeCodeMemoryManager,
     BESENObject,BESENObjectEnvironmentRecord;

function BESENGenerateNativeCode(ACodeContext:TObject):TBESENBoolean;
const sizeofTBESENValue=sizeof(TBESENValue);
type TFixupKind=(fkPTR,fkRET,fkOFS);
     TFixup=record
      Kind:TFixupKind;
      Ofs:integer;
      Dest:pointer;
      ToOfs:integer;
     end;
     TFixups=array of TFixup;
var Fixups:TFixups;
    CountFixups,i:integer;
    Offsets:array of longword;
    Opcode:byte;
    Instruction:TBESENUINT32;
    CodeBuffer:TBESENBytes;
    CodeBufferLen,OldCodeBufferLen:integer;
    CurrentPC,Temp,RetOfs,Literal:longword;
    CodeBegin,CodeEnd:pointer;
    CodeVars:array[0..3] of pointer;
    ByteCode:PBESENUINT32Array;
    Operands:PBESENINT32Array;
    Code:TBESENCode;
    CodeContext:TBESENCodeContext;
    v:TBESENValue;
 procedure Add(const s:TBESENANSISTRING);
 begin
  if length(s)>0 then begin
   if (CodeBufferLen+length(s))>=length(CodeBuffer) then begin
    SetLength(CodeBuffer,(CodeBufferLen+length(s)+4096) and not 4095);
   end;
   move(s[1],CodeBuffer[CodeBufferLen],length(s));
   inc(CodeBufferLen,length(s));
  end;
 end;
 procedure AddCode(CodeBegin,CodeEnd:pointer);
 var CodeLen:ptrint;
{$ifdef windows}
     OldProtect,OldProtectDummy:longword;
     OK:boolean;
{$endif}
 begin
  CodeLen:=ptrint(ptruint(CodeEnd)-ptruint(CodeBegin));
  if CodeLen>0 then begin
{$ifdef windows}
   OK:=VirtualProtect(CodeBegin,CodeLen,PAGE_EXECUTE_READWRITE,OldProtect);
{$endif}
{$ifdef unix}
   fpmprotect(CodeBegin,CodeLen,PROT_READ or PROT_WRITE or PROT_EXEC);
{$endif}
   if (CodeBufferLen+CodeLen)>=length(CodeBuffer) then begin
    SetLength(CodeBuffer,(CodeBufferLen+CodeLen+4096) and not 4095);
   end;
   move(CodeBegin^,CodeBuffer[CodeBufferLen],CodeLen);
   inc(CodeBufferLen,CodeLen);
{$ifdef windows}
   if OK then begin
    VirtualProtect(CodeBegin,CodeLen,OldProtect,OldProtectDummy);
   end;
{$endif}
  end;
 end;
 procedure AddDWord(const v:longword);
 begin
  if (CodeBufferLen+sizeof(longword))>=length(CodeBuffer) then begin
   SetLength(CodeBuffer,(CodeBufferLen+sizeof(longword)+4096) and not 4095);
  end;
  move(v,CodeBuffer[CodeBufferLen],sizeof(longword));
  inc(CodeBufferLen,sizeof(longword));
 end;
 procedure AddDispatcherPointer;
 begin
  if CountFixups>=length(Fixups) then begin
   SetLength(Fixups,CountFixups+4096);
  end;
  Fixups[CountFixups].Kind:=fkPTR;
  Fixups[CountFixups].Ofs:=CodeBufferLen-4;
  Fixups[CountFixups].Dest:=BESENCodeContextOpcodes[Opcode];
  inc(CountFixups);
 end;
 procedure AddDispatcher;
 var Temp:longword;
     CodeBegin,CodeEnd:pointer;
 begin
  Add(#$89#$f0); // mov eax,esi

  case Opcode of
   bopEND,bopJZ,bopJNZ,bopJNULL,bopLOOPENUM,bopTRACE,bopJZERO,bopJNZERO:begin
    Add(#$c7#$80); // mov dword ptr [eax+TBESENCodeContext.PC],CurrentPC
    asm
     mov dword ptr Temp,offset TBESENCodeContext.PC
    end;
    AddDWord(Temp);
    AddDWord(CurrentPC);
   end;
  end;

  Add(#$ba); // mov edx,Operands
  AddDWord(ptruint(Operands));

  Add(#$e8); // call OpcodeDispatcher
  if CountFixups>=length(Fixups) then begin
   SetLength(Fixups,CountFixups+4096);
  end;
  Fixups[CountFixups].Kind:=fkPTR;
  Fixups[CountFixups].Ofs:=CodeBufferLen;
  Fixups[CountFixups].Dest:=BESENCodeContextOpcodes[Opcode];
  inc(CountFixups);
  Add(#$00#$00#$00#$00);

  case Opcode of
   bopEND,bopTRACE,bopENDF:begin
    if (Opcode<>bopEND) or (Operands^[0]=0) then begin
     asm
      jmp @Skip
       @CodeBegin:
        cmp dword ptr [esi+TBESENCodeContext.BlockRunning],0
       @CodeEnd:
      @Skip:
      mov dword ptr CodeBegin,offset @CodeBegin
      mov dword ptr CodeEnd,offset @CodeEnd
     end;
     AddCode(CodeBegin,CodeEnd);

     Add(#$0f#$84); // jz RET
     if CountFixups>=length(Fixups) then begin
      SetLength(Fixups,CountFixups+4096);
     end;
     Fixups[CountFixups].Kind:=fkRET;
     Fixups[CountFixups].Ofs:=CodeBufferLen;
     inc(CountFixups);
     Add(#$00#$00#$00#$00);
    end;
   end;
  end;

  case Opcode of
   bopEND,bopENDF,bopJMP,bopJZ,bopJNZ,bopJNULL,bopLOOPENUM,bopJZERO,bopJNZERO:begin
    asm
     jmp @Skip
      @CodeBegin:
//     mov ebp,dword ptr [esi+TBESENCodeContext.Code]
//     mov ebp,dword ptr [ebp+TBESENCode.NativeCodePCOffsets]
       mov edx,dword ptr [esi+TBESENCodeContext.PC]
       jmp dword ptr [ebp+edx*4]
      @CodeEnd:
     @Skip:
     mov dword ptr CodeBegin,offset @CodeBegin
     mov dword ptr CodeEnd,offset @CodeEnd
    end;
    AddCode(CodeBegin,CodeEnd);
   end;
  end;

 end;
begin
 result:=false;
 try
  CodeContext:=TBESENCodeContext(ACodeContext);
  Code:=TBESENCode(CodeContext.Code);
  if assigned(Code.NativeCode) then begin
   TBESEN(CodeContext.Instance).NativeCodeMemoryManager.FreeMemory(Code.NativeCode);
   Code.NativeCode:=nil;
   Code.NativeCodeSize:=0;
  end;
  CodeBuffer:=nil;
  CodeBufferLen:=0;
  CurrentPC:=0;
  Fixups:=nil;
  CountFixups:=0;
  Offsets:=nil;
  ByteCode:=@Code.ByteCode[0];
  SetLength(Offsets,Code.ByteCodeLen);
  while CurrentPC<TBESENUINT32(Code.ByteCodeLen) do begin
   Offsets[CurrentPC]:=CodeBufferLen;
   Instruction:=ByteCode^[CurrentPC];
   Operands:=@ByteCode^[CurrentPC+1];
   inc(CurrentPC,1+(Instruction shr 8));
   Opcode:=Instruction and $ff;

   case Opcode of
    bopSTOP:begin
     asm
      jmp @Skip
       @CodeBegin:
        mov dword ptr [esi+TBESENCodeContext.BlockRunning],0
        mov dword ptr [esi+TBESENCodeContext.Running],0
       @CodeEnd:
      @Skip:
      mov dword ptr CodeBegin,offset @CodeBegin
      mov dword ptr CodeEnd,offset @CodeEnd
     end;
     AddCode(CodeBegin,CodeEnd);

     Add(#$e9); // jmp to code end
     if CountFixups>=length(Fixups) then begin
      SetLength(Fixups,CountFixups+4096);
     end;
     Fixups[CountFixups].Kind:=fkRET;
     Fixups[CountFixups].Ofs:=CodeBufferLen;
     inc(CountFixups);
     Add(#$00#$00#$00#$00);
    end;
    bopNEW:begin
     AddDispatcher;
    end;
    bopCALL:begin
     AddDispatcher;
    end;
    bopEND:begin
     AddDispatcher;
    end;
    bopVREF:begin
{$ifdef UseSafeOperations}
     AddDispatcher;
{$else}
     Add(#$ba); // mov edx,Operands
     AddDWord(ptruint(Operands));

     Add(#$81#$bf); // cmp dword ptr [edi+RegisterOfs+TBESENValue.ValueType],bvtREFERENCE
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.ValueType))-ptruint(pointer(@v))));
     AddDWord(bvtREFERENCE);

     // Monomorphic precheck before the polymorphic/megamorphic checks in the opcode handler itself
     asm
      jmp @Skip
       @CodeBegin:
        jnz @DoDispatcher
         mov ecx,dword ptr [edx+4*4]
         test ecx,ecx
         js @DoDispatcher
          mov eax,dword ptr [esi+TBESENCodeContext.Context]
          mov eax,dword ptr [eax+TBESENContext.VariableEnvironment]
          mov eax,dword ptr [eax+TBESENLexicalEnvironment.EnvironmentRecord]
          cmp dword ptr [eax+TBESENObjectEnvironmentRecord.RecordType],0
          jz @DoDispatcher
           mov eax,dword ptr [eax+TBESENObjectEnvironmentRecord.BindingObject]
           mov eax,dword ptr [eax+TBESENObject.StructureID]
           cmp eax,ecx
           jz @SkipDispatcher
         @DoDispatcher:
          mov eax,esi
          db $e8; dd $00; // call OpcodeDispatcher
         @SkipDispatcher:
       @CodeEnd:
      @Skip:
      mov dword ptr CodeBegin,offset @CodeBegin
      mov dword ptr CodeEnd,offset @CodeEnd
     end;
     AddCode(CodeBegin,CodeEnd);
     AddDispatcherPointer;
{$endif}
    end;
    bopLREF:begin
     Add(#$c7#$87); // mov dword ptr [edi+RegisterOfs+TBESENValue.ValueType],bvtLOCAL
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.ValueType))-ptruint(pointer(@v))));
     AddDWord(bvtLOCAL);

     Add(#$c7#$87); // mov dword ptr [edi+RegisterOfs+TBESENValue.LocalIndex],LocalIndex
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.LocalIndex))-ptruint(pointer(@v))));
     AddDWord(Operands^[1]);
    end;
    bopNOP:begin
    end;
    bopCOPY:begin
     Add(#$8d#$87); // lea eax,dword ptr [edi+RegisterOfs]
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue)));

     Add(#$8d#$97); // lea edx,dword ptr [edi+RegisterOfs]
     AddDWord(ptruint(Operands^[1]*sizeof(TBESENValue)));

     asm
      jmp @Skip
       @CodeBegin:
        mov ecx,dword ptr [edx+TBESENValue.ValueType]
        call dword ptr [ecx*4+offset BESENCopyValueProcs]
       @CodeEnd:
      @Skip:
      mov dword ptr CodeBegin,offset @CodeBegin
      mov dword ptr CodeEnd,offset @CodeEnd
     end;
     AddCode(CodeBegin,CodeEnd);
    end;
    bopNEQ:begin
     AddDispatcher;
    end;
    bopNSEQ:begin
     AddDispatcher;
    end;
    bopAREF:begin
     AddDispatcher;
    end;
    bopTHROW:begin
     AddDispatcher;
    end;
    bopSETC:begin
     Add(#$8d#$97); // lea edx,dword ptr [edi+RegisterOfs]
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue)));
     asm
      jmp @Skip
       @CodeBegin:
        lea eax,dword ptr [esi+TBESENCodeContext.ResultValue]
        mov ecx,dword ptr [edx+TBESENValue.ValueType]
        call dword ptr [ecx*4+offset BESENCopyValueProcs]
       @CodeEnd:
      @Skip:
      mov dword ptr CodeBegin,offset @CodeBegin
      mov dword ptr CodeEnd,offset @CodeEnd
     end;
     AddCode(CodeBegin,CodeEnd);
    end;
    bopGETC:begin
     Add(#$8d#$87); // lea eax,dword ptr [edi+RegisterOfs]
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue)));
     asm
      jmp @Skip
       @CodeBegin:
        lea edx,dword ptr [esi+TBESENCodeContext.ResultValue]
        mov ecx,dword ptr [edx+TBESENValue.ValueType]
        call dword ptr [ecx*4+offset BESENCopyValueProcs]
       @CodeEnd:
      @Skip:
      mov dword ptr CodeBegin,offset @CodeBegin
      mov dword ptr CodeEnd,offset @CodeEnd
     end;
     AddCode(CodeBegin,CodeEnd);
    end;
    bopTHIS:begin
     Add(#$8d#$87); // lea eax,dword ptr [edi+RegisterOfs]
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue)));
     asm
      jmp @Skip
       @CodeBegin:
        mov edx,dword ptr [esi+TBESENCodeContext.Context]
        lea edx,dword ptr [edx+TBESENContext.ThisBinding]
        mov ecx,dword ptr [edx+TBESENValue.ValueType]
        call dword ptr [ecx*4+offset BESENCopyValueProcs]
       @CodeEnd:
      @Skip:
      mov dword ptr CodeBegin,offset @CodeBegin
      mov dword ptr CodeEnd,offset @CodeEnd
     end;
     AddCode(CodeBegin,CodeEnd);
    end;
    bopOBJECT:begin
     Add(#$c7#$87); // mov dword ptr [edi+RegisterOfs+TBESENValue.ValueType],bvtOBJECT
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.ValueType))-ptruint(pointer(@v))));
     AddDWord(bvtOBJECT);

     Add(#$8d#$97); // lea edx,dword ptr [edi+RegisterOfs+TBESENValue.Obj]
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.Obj))-ptruint(pointer(@v))));
     asm
      jmp @Skip
       @CodeBegin:
        mov eax,dword ptr [esi+TBESENCodeContext.Instance]
        mov eax,dword ptr [eax+TBESEN.ObjectConstructor]
        mov dword ptr [edx],eax
       @CodeEnd:
      @Skip:
      mov dword ptr CodeBegin,offset @CodeBegin
      mov dword ptr CodeEnd,offset @CodeEnd
     end;
     AddCode(CodeBegin,CodeEnd);
    end;
    bopARRAY:begin
     Add(#$c7#$87); // mov dword ptr [edi+RegisterOfs+TBESENValue.ValueType],bvtOBJECT
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.ValueType))-ptruint(pointer(@v))));
     AddDWord(bvtOBJECT);
     Add(#$8d#$97); // lea edx,dword ptr [edi+RegisterOfs+TBESENValue.Obj]
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.Obj))-ptruint(pointer(@v))));
     asm
      jmp @Skip
       @CodeBegin:
        mov eax,dword ptr [esi+TBESENCodeContext.Instance]
        mov eax,dword ptr [eax+TBESEN.ObjectArrayConstructor]
        mov dword ptr [edx],eax
       @CodeEnd:
      @Skip:
      mov dword ptr CodeBegin,offset @CodeBegin
      mov dword ptr CodeEnd,offset @CodeEnd
     end;
     AddCode(CodeBegin,CodeEnd);
    end;
    bopREGEXP:begin
     Add(#$c7#$87); // mov dword ptr [edi+RegisterOfs+TBESENValue.ValueType],bvtOBJECT
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.ValueType))-ptruint(pointer(@v))));
     AddDWord(bvtOBJECT);
     Add(#$8d#$97); // lea edx,dword ptr [edi+RegisterOfs+TBESENValue.Obj]
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.Obj))-ptruint(pointer(@v))));
     asm
      jmp @Skip
       @CodeBegin:
        mov eax,dword ptr [esi+TBESENCodeContext.Instance]
        mov eax,dword ptr [eax+TBESEN.ObjectRegExpConstructor]
        mov dword ptr [edx],eax
       @CodeEnd:
      @Skip:
      mov dword ptr CodeBegin,offset @CodeBegin
      mov dword ptr CodeEnd,offset @CodeEnd
     end;
     AddCode(CodeBegin,CodeEnd);
    end;
    bopREF:begin
     if TBESENUINT32(Operands^[3])<>$fefefefe then begin
      Add(#$ba); // mov edx,Operands
      AddDWord(ptruint(Operands));

      Add(#$81#$bf); // cmp dword ptr [edi+RegisterOfs+TBESENValue.ValueType],bvtREFERENCE
      AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.ValueType))-ptruint(pointer(@v))));
      AddDWord(bvtREFERENCE);

      // Monomorphic precheck before the polymorphic/megamorphic checks in the opcode handler itself
      asm
       jmp @Skip
        @CodeBegin:
         jnz @DoDispatcher
          mov ecx,dword ptr [edx+6*4]
          test ecx,ecx
          js @DoDispatcher
           @CodeVars0:
           cmp dword ptr [edi+$1337c0de],bvtOBJECT
           jnz @DoDispatcher
            @CodeVars1:
            cmp dword ptr [edi+$1337c0de],brbvtOBJECT
            jnz @DoDispatcher
             @CodeVars2:
             mov eax,dword ptr [edi+$1337c0de]
             cmp dword ptr [eax+TBESENObject.StructureID],ecx
             jnz @DoDispatcher
              @CodeVars3:
              mov dword ptr [edi+$1337c0de],eax
             jmp @SkipDispatcher
          @DoDispatcher:
           mov eax,esi
           db $e8; dd $00; // call OpcodeDispatcher
         @SkipDispatcher:
        @CodeEnd:
       @Skip:
       mov dword ptr CodeBegin,offset @CodeBegin
       mov dword ptr CodeEnd,offset @CodeEnd
       mov dword ptr [CodeVars+0*4],offset @CodeVars0+2
       mov dword ptr [CodeVars+1*4],offset @CodeVars1+2
       mov dword ptr [CodeVars+2*4],offset @CodeVars2+2
       mov dword ptr [CodeVars+3*4],offset @CodeVars3+2
      end;
      OldCodeBufferLen:=CodeBufferLen;
      AddCode(CodeBegin,CodeEnd);
      AddDispatcherPointer;
      PBESENUINT32(@CodeBuffer[ptruint(OldCodeBufferLen)+ptruint(ptruint(CodeVars[0])-ptruint(CodeBegin))])^:=ptruint(Operands^[1]*sizeof(TBESENValue))+(ptruint(pointer(@v.ValueType))-ptruint(pointer(@v)));
      PBESENUINT32(@CodeBuffer[ptruint(OldCodeBufferLen)+ptruint(ptruint(CodeVars[1])-ptruint(CodeBegin))])^:=ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.ReferenceBase.ValueType))-ptruint(pointer(@v)));
      PBESENUINT32(@CodeBuffer[ptruint(OldCodeBufferLen)+ptruint(ptruint(CodeVars[2])-ptruint(CodeBegin))])^:=ptruint(Operands^[1]*sizeof(TBESENValue))+(ptruint(pointer(@v.Obj))-ptruint(pointer(@v)));
      PBESENUINT32(@CodeBuffer[ptruint(OldCodeBufferLen)+ptruint(ptruint(CodeVars[3])-ptruint(CodeBegin))])^:=ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.ReferenceBase.Obj))-ptruint(pointer(@v)));
     end else begin
      AddDispatcher;
     end;
    end;
    bopGETVALUE:begin
     AddDispatcher;
    end;
    bopLOOKUP:begin
     AddDispatcher;
    end;
    bopPUTVALUE:begin
     AddDispatcher;
    end;
    bopDELETE:begin
     AddDispatcher;
    end;
    bopTYPEOF:begin
     AddDispatcher;
    end;
    bopTOOBJECT:begin
     AddDispatcher;
    end;
    bopTONUMBER:begin
{$ifdef UseSafeOperations}
     AddDispatcher;
{$else}
     if Operands^[0]=Operands^[1] then begin
      Add(#$ba); // mov edx,Operands
      AddDWord(ptruint(Operands));

      Add(#$81#$bf); // cmp dword ptr [edi+RegisterOfs+TBESENValue.ValueType],bvtNUMBER
      AddDWord(ptruint(Operands^[1]*sizeof(TBESENValue))+(ptruint(pointer(@v.ValueType))-ptruint(pointer(@v))));
      AddDWord(bvtNUMBER);
      asm
       jmp @Skip
        @CodeBegin:
         jz @SkipDispatcher
          mov eax,esi
          db $e8; dd $00; // call OpcodeDispatcher
         @SkipDispatcher:
        @CodeEnd:
       @Skip:
       mov dword ptr CodeBegin,offset @CodeBegin
       mov dword ptr CodeEnd,offset @CodeEnd
      end;
      AddDispatcherPointer;
     end else begin
      Add(#$ba); // mov edx,Operands
      AddDWord(ptruint(Operands));

      Add(#$81#$bf); // cmp dword ptr [edi+RegisterOfs+TBESENValue.ValueType],bvtNUMBER
      AddDWord(ptruint(Operands^[1]*sizeof(TBESENValue))+(ptruint(pointer(@v.ValueType))-ptruint(pointer(@v))));
      AddDWord(bvtNUMBER);
      asm
       jmp @Skip
        @CodeBegin:
         jnz @DoDispatcher
          @CodeVars0:
          mov dword ptr [edi+$1337c0d3],bvtNUMBER
          @CodeVars1:
          fld qword ptr [edi+$1337c0d3]
          @CodeVars2:
          fstp qword ptr [edi+$1337c0d3]
          jmp @SkipDispatcher
          @DoDispatcher:
           mov eax,esi
           db $e8; dd $00; // call OpcodeDispatcher
          @SkipDispatcher:
        @CodeEnd:
       @Skip:
       mov dword ptr CodeBegin,offset @CodeBegin
       mov dword ptr CodeEnd,offset @CodeEnd
       mov dword ptr [CodeVars+0*4],offset @CodeVars0+2
       mov dword ptr [CodeVars+1*4],offset @CodeVars1+2
       mov dword ptr [CodeVars+2*4],offset @CodeVars2+2
      end;
      OldCodeBufferLen:=CodeBufferLen;
      AddCode(CodeBegin,CodeEnd);
      AddDispatcherPointer;
      PBESENUINT32(@CodeBuffer[ptruint(OldCodeBufferLen)+ptruint(ptruint(CodeVars[0])-ptruint(CodeBegin))])^:=ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.ValueType))-ptruint(pointer(@v)));
      PBESENUINT32(@CodeBuffer[ptruint(OldCodeBufferLen)+ptruint(ptruint(CodeVars[1])-ptruint(CodeBegin))])^:=ptruint(Operands^[1]*sizeof(TBESENValue))+(ptruint(pointer(@v.Num))-ptruint(pointer(@v)));
      PBESENUINT32(@CodeBuffer[ptruint(OldCodeBufferLen)+ptruint(ptruint(CodeVars[2])-ptruint(CodeBegin))])^:=ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.Num))-ptruint(pointer(@v)));
     end;
{$endif}
    end;
    bopTOBOOLEAN:begin
{$ifdef UseSafeOperations}
     AddDispatcher;
{$else}
     if Operands^[0]=Operands^[1] then begin
      Add(#$ba); // mov edx,Operands
      AddDWord(ptruint(Operands));

      Add(#$81#$bf); // cmp dword ptr [edi+RegisterOfs+TBESENValue.ValueType],bvtBOOLEAN
      AddDWord(ptruint(Operands^[1]*sizeof(TBESENValue))+(ptruint(pointer(@v.ValueType))-ptruint(pointer(@v))));
      AddDWord(bvtBOOLEAN);
      asm
       jmp @Skip
        @CodeBegin:
         jz @SkipDispatcher
          mov eax,esi
          db $e8; dd $00; // call OpcodeDispatcher
         @SkipDispatcher:
        @CodeEnd:
       @Skip:
       mov dword ptr CodeBegin,offset @CodeBegin
       mov dword ptr CodeEnd,offset @CodeEnd
      end;
      AddDispatcherPointer;
     end else begin
      Add(#$ba); // mov edx,Operands
      AddDWord(ptruint(Operands));

      Add(#$81#$bf); // cmp dword ptr [edi+RegisterOfs+TBESENValue.ValueType],bvtBOOLEAN
      AddDWord(ptruint(Operands^[1]*sizeof(TBESENValue))+(ptruint(pointer(@v.ValueType))-ptruint(pointer(@v))));
      AddDWord(bvtBOOLEAN);
      asm
       jmp @Skip
        @CodeBegin:
         jnz @DoDispatcher
          @CodeVars0:
          mov dword ptr [edi+$1337c0d3],bvtBOOLEAN
          @CodeVars1:
          mov eax,dword ptr [edi+$1337c0d3]
          @CodeVars2:
          mov dword ptr [edi+$1337c0d3],eax
          jmp @SkipDispatcher
          @DoDispatcher:
           mov eax,esi
           db $e8; dd $00; // call OpcodeDispatcher
          @SkipDispatcher:
        @CodeEnd:
       @Skip:
       mov dword ptr CodeBegin,offset @CodeBegin
       mov dword ptr CodeEnd,offset @CodeEnd
       mov dword ptr [CodeVars+0*4],offset @CodeVars0+2
       mov dword ptr [CodeVars+1*4],offset @CodeVars1+2
       mov dword ptr [CodeVars+2*4],offset @CodeVars2+2
      end;
      OldCodeBufferLen:=CodeBufferLen;
      AddCode(CodeBegin,CodeEnd);
      AddDispatcherPointer;
      PBESENUINT32(@CodeBuffer[ptruint(OldCodeBufferLen)+ptruint(ptruint(CodeVars[0])-ptruint(CodeBegin))])^:=ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.ValueType))-ptruint(pointer(@v)));
      PBESENUINT32(@CodeBuffer[ptruint(OldCodeBufferLen)+ptruint(ptruint(CodeVars[1])-ptruint(CodeBegin))])^:=ptruint(Operands^[1]*sizeof(TBESENValue))+(ptruint(pointer(@v.Bool))-ptruint(pointer(@v)));
      PBESENUINT32(@CodeBuffer[ptruint(OldCodeBufferLen)+ptruint(ptruint(CodeVars[2])-ptruint(CodeBegin))])^:=ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.Bool))-ptruint(pointer(@v)));
     end;
{$endif}
    end;
    bopTOSTRING:begin
     AddDispatcher;
    end;
    bopTOPRIMITIVE:begin
     AddDispatcher;
    end;
    bopNEG:begin
     if Operands^[0]<>Operands^[1] then begin
      Add(#$c7#$87); // mov dword ptr [edi+RegisterOfs+TBESENValue.ValueType],bvtNUMBER
      AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.ValueType))-ptruint(pointer(@v))));
      AddDWord(bvtNUMBER);
     end;

     Add(#$dd#$87); // fld qword ptr [edi+RegisterOfs+TBESENValue.Num]
     AddDWord(ptruint(Operands^[1]*sizeof(TBESENValue))+(ptruint(pointer(@v.Num))-ptruint(pointer(@v))));

     Add(#$dd#$9f); // fstp qword ptr [edi+RegisterOfs+TBESENValue.Num]
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.Num))-ptruint(pointer(@v))));

     Add(#$81#$b7); // xor dword ptr [edi+RegisterOfs+TBESENValue.Num+4],$80000000
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+((ptruint(pointer(@v.Num))+4)-ptruint(pointer(@v))));
     AddDWord($80000000);
    end;
    bopINV:begin
     AddDispatcher;
    end;
    bopNOT:begin
     Add(#$8b#$87); // mov eax,dword ptr [edi+RegisterOfs+TBESENValue.Bool]
     AddDWord(ptruint(Operands^[1]*sizeof(TBESENValue))+(ptruint(pointer(@v.Bool))-ptruint(pointer(@v))));

     asm
      jmp @Skip
       @CodeBegin:
        cmp eax,$01
        sbb eax,eax
        neg eax
        neg al
        sbb eax,eax
       @CodeEnd:
      @Skip:
      mov dword ptr CodeBegin,offset @CodeBegin
      mov dword ptr CodeEnd,offset @CodeEnd
     end;
     AddCode(CodeBegin,CodeEnd);

     if Operands^[0]<>Operands^[1] then begin
      Add(#$c7#$87); // mov dword ptr [edi+RegisterOfs+TBESENValue.ValueType],bvtBOOLEAN
      AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.ValueType))-ptruint(pointer(@v))));
      AddDWord(bvtBOOLEAN);
     end;

     Add(#$89#$87); // mov dword ptr [edi+RegisterOfs+TBESENValue.Bool],eax
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.Bool))-ptruint(pointer(@v))));
    end;
    bopMUL:begin
     if (Operands^[0]<>Operands^[1]) and (Operands^[0]<>Operands^[2]) then begin
      Add(#$c7#$87); // mov dword ptr [edi+RegisterOfs+TBESENValue.ValueType],bvtNUMBER
      AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.ValueType))-ptruint(pointer(@v))));
      AddDWord(bvtNUMBER);
     end;

     Add(#$dd#$87); // fld qword ptr [edi+RegisterOfs+TBESENValue.Num]
     AddDWord(ptruint(Operands^[1]*sizeof(TBESENValue))+(ptruint(pointer(@v.Num))-ptruint(pointer(@v))));

     Add(#$dc#$8f); // fmul qword ptr [edi+RegisterOfs+TBESENValue.Num]
     AddDWord(ptruint(Operands^[2]*sizeof(TBESENValue))+(ptruint(pointer(@v.Num))-ptruint(pointer(@v))));

     Add(#$dd#$9f); // fstp qword ptr [edi+RegisterOfs+TBESENValue.Num]
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.Num))-ptruint(pointer(@v))));
    end;
    bopDIV:begin
     if (Operands^[0]<>Operands^[1]) and (Operands^[0]<>Operands^[2]) then begin
      Add(#$c7#$87); // mov dword ptr [edi+RegisterOfs+TBESENValue.ValueType],bvtNUMBER
      AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.ValueType))-ptruint(pointer(@v))));
      AddDWord(bvtNUMBER);
     end;

     Add(#$dd#$87); // fld qword ptr [edi+RegisterOfs+TBESENValue.Num]
     AddDWord(ptruint(Operands^[1]*sizeof(TBESENValue))+(ptruint(pointer(@v.Num))-ptruint(pointer(@v))));

     Add(#$dc#$b7); // fdiv qword ptr [edi+RegisterOfs+TBESENValue.Num]
     AddDWord(ptruint(Operands^[2]*sizeof(TBESENValue))+(ptruint(pointer(@v.Num))-ptruint(pointer(@v))));

     Add(#$dd#$9f); // fstp qword ptr [edi+RegisterOfs+TBESENValue.Num]
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.Num))-ptruint(pointer(@v))));
    end;
    bopMOD:begin
     if (Operands^[0]<>Operands^[1]) and (Operands^[0]<>Operands^[2]) then begin
      Add(#$c7#$87); // mov dword ptr [edi+RegisterOfs+TBESENValue.ValueType],bvtNUMBER
      AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.ValueType))-ptruint(pointer(@v))));
      AddDWord(bvtNUMBER);
     end;

     Add(#$dd#$87); // fld qword ptr [edi+RegisterOfs+TBESENValue.Num]
     AddDWord(ptruint(Operands^[2]*sizeof(TBESENValue))+(ptruint(pointer(@v.Num))-ptruint(pointer(@v))));

     Add(#$dd#$87); // fld qword ptr [edi+RegisterOfs+TBESENValue.Num]
     AddDWord(ptruint(Operands^[1]*sizeof(TBESENValue))+(ptruint(pointer(@v.Num))-ptruint(pointer(@v))));

     asm
      jmp @Skip
       @CodeBegin:
        @Repeat:
         fprem
         fstsw ax
         sahf
         jp @Repeat
        fstp st(1)
       @CodeEnd:
      @Skip:
      mov dword ptr CodeBegin,offset @CodeBegin
      mov dword ptr CodeEnd,offset @CodeEnd
     end;
     AddCode(CodeBegin,CodeEnd);

     Add(#$dd#$9f); // fstp qword ptr [edi+RegisterOfs+TBESENValue.Num]
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.Num))-ptruint(pointer(@v))));
    end;
    bopADD:begin
     AddDispatcher;
    end;
    bopADDNUM:begin
     if (Operands^[0]<>Operands^[1]) and (Operands^[0]<>Operands^[2]) then begin
      Add(#$c7#$87); // mov dword ptr [edi+RegisterOfs+TBESENValue.ValueType],bvtNUMBER
      AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.ValueType))-ptruint(pointer(@v))));
      AddDWord(bvtNUMBER);
     end;

     Add(#$dd#$87); // fld qword ptr [edi+RegisterOfs+TBESENValue.Num]
     AddDWord(ptruint(Operands^[1]*sizeof(TBESENValue))+(ptruint(pointer(@v.Num))-ptruint(pointer(@v))));

     Add(#$dc#$87); // fadd qword ptr [edi+RegisterOfs+TBESENValue.Num]
     AddDWord(ptruint(Operands^[2]*sizeof(TBESENValue))+(ptruint(pointer(@v.Num))-ptruint(pointer(@v))));

     Add(#$dd#$9f); // fstp qword ptr [edi+RegisterOfs+TBESENValue.Num]
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.Num))-ptruint(pointer(@v))));
    end;
    bopSUB:begin
     if (Operands^[0]<>Operands^[1]) and (Operands^[0]<>Operands^[2]) then begin
      Add(#$c7#$87); // mov dword ptr [edi+RegisterOfs+TBESENValue.ValueType],bvtNUMBER
      AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.ValueType))-ptruint(pointer(@v))));
      AddDWord(bvtNUMBER);
     end;

     Add(#$dd#$87); // fld qword ptr [edi+RegisterOfs+TBESENValue.Num]
     AddDWord(ptruint(Operands^[1]*sizeof(TBESENValue))+(ptruint(pointer(@v.Num))-ptruint(pointer(@v))));

     Add(#$dc#$a7); // fsub qword ptr [edi+RegisterOfs+TBESENValue.Num]
     AddDWord(ptruint(Operands^[2]*sizeof(TBESENValue))+(ptruint(pointer(@v.Num))-ptruint(pointer(@v))));

     Add(#$dd#$9f); // fstp qword ptr [edi+RegisterOfs+TBESENValue.Num]
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.Num))-ptruint(pointer(@v))));
    end;
    bopSHL:begin
     AddDispatcher;
    end;
    bopSHR:begin
     AddDispatcher;
    end;
    bopUSHR:begin
     AddDispatcher;
    end;
    bopLT:begin
     AddDispatcher;
    end;
    bopGT:begin
     AddDispatcher;
    end;
    bopLE:begin
     AddDispatcher;
    end;
    bopGE:begin
     AddDispatcher;
    end;
    bopINSTANCEOF:begin
     AddDispatcher;
    end;
    bopIN:begin
     AddDispatcher;
    end;
    bopEQ:begin
     AddDispatcher;
    end;
    bopSEQ:begin
     AddDispatcher;
    end;
    bopBAND:begin
     AddDispatcher;
    end;
    bopBXOR:begin
     AddDispatcher;
    end;
    bopBOR:begin
     AddDispatcher;
    end;
    bopSENUM:begin
     AddDispatcher;
    end;
    bopSWITH:begin
     AddDispatcher;
    end;
    bopSCATCH:begin
     AddDispatcher;
    end;
    bopENDF:begin
     AddDispatcher;
    end;
    bopJMP:begin
     if longword(Operands^[0])<>CurrentPC then begin
      Add(#$e9); // jmp Arg
      if CountFixups>=length(Fixups) then begin
       SetLength(Fixups,CountFixups+4096);
      end;
      Fixups[CountFixups].Kind:=fkOFS;
      Fixups[CountFixups].Ofs:=CodeBufferLen;
      Fixups[CountFixups].ToOfs:=Operands^[0];
      inc(CountFixups);
      Add(#$00#$00#$00#$00);
     end;
    end;
    bopJZ:begin
     if longword(Operands^[0])<>CurrentPC then begin
      Add(#$83#$bf); // cmp dword ptr [edi+RegisterOfs+TBESENValue.Bool],0
      AddDWord(ptruint(Operands^[1]*sizeof(TBESENValue))+(ptruint(pointer(@v.Bool))-ptruint(pointer(@v))));
      Add(#$00);

      Add(#$0f#$85); // jnz Arg
      if CountFixups>=length(Fixups) then begin
       SetLength(Fixups,CountFixups+4096);
      end;
      Fixups[CountFixups].Kind:=fkOFS;
      Fixups[CountFixups].Ofs:=CodeBufferLen;
      Fixups[CountFixups].ToOfs:=Operands^[0];
      inc(CountFixups);
      Add(#$00#$00#$00#$00);
     end;
    end;
    bopJNZ:begin
     if longword(Operands^[0])<>CurrentPC then begin
      Add(#$83#$bf); // cmp dword ptr [edi+RegisterOfs+TBESENValue.Bool],0
      AddDWord(ptruint(Operands^[1]*sizeof(TBESENValue))+(ptruint(pointer(@v.Bool))-ptruint(pointer(@v))));
      Add(#$00);

      Add(#$0f#$84); // jz Arg
      if CountFixups>=length(Fixups) then begin
       SetLength(Fixups,CountFixups+4096);
      end;
      Fixups[CountFixups].Kind:=fkOFS;
      Fixups[CountFixups].Ofs:=CodeBufferLen;
      Fixups[CountFixups].ToOfs:=Operands^[0];
      inc(CountFixups);
      Add(#$00#$00#$00#$00);
     end;
    end;
    bopJNULL:begin
     AddDispatcher;
    end;
    bopLOOPENUM:begin
     AddDispatcher;
    end;
    bopSTRYC:begin
     AddDispatcher;
    end;
    bopSTRYF:begin
     AddDispatcher;
    end;
    bopLITERALUNDEF:begin
     Add(#$c7#$87); // mov dword ptr [edi+RegisterOfs+TBESENValue.ValueType],bvtUNDEFINED
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.ValueType))-ptruint(pointer(@v))));
     AddDWord(bvtUNDEFINED);
    end;
    bopLITERALNULL:begin
     Add(#$c7#$87); // mov dword ptr [edi+RegisterOfs+TBESENValue.ValueType],bvtNULL
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.ValueType))-ptruint(pointer(@v))));
     AddDWord(bvtNULL);
    end;
    bopLITERALBOOL:begin
     Add(#$c7#$87); // mov dword ptr [edi+RegisterOfs+TBESENValue.ValueType],bvtBOOLEAN
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.ValueType))-ptruint(pointer(@v))));
     AddDWord(bvtBOOLEAN);

     Add(#$c7#$87); // mov dword ptr [edi+RegisterOfs+TBESENValue.Bool],Bool
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.Bool))-ptruint(pointer(@v))));
     if Operands^[1]<>0 then begin
      AddDWord(longword(pointer(@BESENLongBooleanValues[true])^));
     end else begin
      AddDWord(longword(pointer(@BESENLongBooleanValues[false])^));
     end;
    end;
    bopLITERALNUM:begin
     Add(#$c7#$87); // mov dword ptr [edi+RegisterOfs+TBESENValue.ValueType],bvtNUMBER
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.ValueType))-ptruint(pointer(@v))));
     AddDWord(bvtNUMBER);

     Add(#$dd#$05); // fld qword ptr [Num]
     AddDWord(ptruint(pointer(@Code.Literals[Operands^[1]].Num)));

     Add(#$dd#$9f); // fstp qword ptr [edi+RegisterOfs+TBESENValue.Num]
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.Num))-ptruint(pointer(@v))));
    end;
    bopLITERALSTR:begin
     AddDispatcher;
    end;
    bopLITERALOBJ:begin
     Add(#$c7#$87); // mov dword ptr [edi+RegisterOfs+TBESENValue.ValueType],bvtOBJECT
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.ValueType))-ptruint(pointer(@v))));
     AddDWord(bvtOBJECT);

     Add(#$c7#$87); // mov dword ptr [edi+RegisterOfs+TBESENValue.Obj],Obj
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.Obj))-ptruint(pointer(@v))));
     AddDWord(ptruint(pointer(Code.Literals[Operands^[1]].Obj)));
    end;
    bopFUNC:begin
     AddDispatcher;
    end;
    bopLINE:begin
     Add(#$b8); // mov eax,Arg
     AddDWord(Code.Locations[Operands^[0]].LineNumber);
     asm
      jmp @Skip
       @CodeBegin:
        mov edx,dword ptr [esi+TBESENCodeContext.Instance]
        mov dword ptr [edx+TBESEN.LineNumber],eax
       @CodeEnd:
      @Skip:
      mov dword ptr CodeBegin,offset @CodeBegin
      mov dword ptr CodeEnd,offset @CodeEnd
     end;
     AddCode(CodeBegin,CodeEnd);
    end;
    bopGC:begin
     asm
      jmp @Skip
       @CodeBegin:
        mov eax,dword ptr [esi+TBESENCodeContext.Instance]
        mov eax,dword ptr [eax+TBESEN.GarbageCollector]
       @CodeEnd:
      @Skip:
      mov dword ptr CodeBegin,offset @CodeBegin
      mov dword ptr CodeEnd,offset @CodeEnd
     end;
     AddCode(CodeBegin,CodeEnd);

     Add(#$e8); // call TBESENGarbageCollector.TriggerCollect
     if CountFixups>=length(Fixups) then begin
      SetLength(Fixups,CountFixups+4096);
     end;
     Fixups[CountFixups].Kind:=fkPTR;
     Fixups[CountFixups].Ofs:=CodeBufferLen;
     Fixups[CountFixups].Dest:=@TBESENGarbageCollector.TriggerCollect;
     inc(CountFixups);
     Add(#$00#$00#$00#$00);
    end;
    bopSTRICT:begin
     Add(#$b8); // mov eax,Arg
     AddDWord(longword(BESENLongBooleanValues[Operands^[0]<>0]));
     asm
      jmp @Skip
       @CodeBegin:
        mov edx,dword ptr [esi+TBESENCodeContext.Instance]
        mov dword ptr [edx+TBESEN.IsStrict],eax
       @CodeEnd:
      @Skip:
      mov dword ptr CodeBegin,offset @CodeBegin
      mov dword ptr CodeEnd,offset @CodeEnd
     end;
     AddCode(CodeBegin,CodeEnd);
    end;
    bopSTRICTCHECKREF:begin
     if not Code.Body.IsStrict then begin
      Add(#$8b#$87); // mov eax,dword ptr [edi+RegisterOfs+TBESENValue.ReferenceIsStrict]
      AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.ReferenceIsStrict))-ptruint(pointer(@v))));

      Add(#$85#$c0); // test eax,rax

      Add(#$0f#$84); // jz Arg
      if CountFixups>=length(Fixups) then begin
       SetLength(Fixups,CountFixups+4096);
      end;
      Fixups[CountFixups].Kind:=fkOFS;
      Fixups[CountFixups].Ofs:=CodeBufferLen;
      Fixups[CountFixups].ToOfs:=CurrentPC;
      inc(CountFixups);
      Add(#$00#$00#$00#$00);
     end;

     AddDispatcher;
    end;
    bopDEBUGGER:begin
     if TBESEN(CodeContext.Instance).CodeTracable then begin
      AddDispatcher;
     end;
    end;
    bopCHECKOBJECTCOERCIBLE:begin
     AddDispatcher;
    end;
    bopPUTOBJVALUE:begin
     AddDispatcher;
    end;
    bopPUTOBJGET:begin
     AddDispatcher;
    end;
    bopPUTOBJSET:begin
     AddDispatcher;
    end;
    bopINC:begin
     if Operands^[0]<>Operands^[1] then begin
      Add(#$c7#$87); // mov dword ptr [edi+RegisterOfs+TBESENValue.ValueType],bvtNUMBER
      AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.ValueType))-ptruint(pointer(@v))));
      AddDWord(bvtNUMBER);
     end;

     Add(#$d9#$e8); // fld1

     Add(#$dc#$87); // fadd qword ptr [edi+RegisterOfs+TBESENValue.Num]
     AddDWord(ptruint(Operands^[1]*sizeof(TBESENValue))+(ptruint(pointer(@v.Num))-ptruint(pointer(@v))));

     Add(#$dd#$9f); // fstp qword ptr [edi+RegisterOfs+TBESENValue.Num]
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.Num))-ptruint(pointer(@v))));
    end;
    bopDEC:begin
     if Operands^[0]<>Operands^[1] then begin
      Add(#$c7#$87); // mov dword ptr [edi+RegisterOfs+TBESENValue.ValueType],bvtNUMBER
      AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.ValueType))-ptruint(pointer(@v))));
      AddDWord(bvtNUMBER);
     end;

     Add(#$dd#$87); // fld qword ptr [edi+RegisterOfs+TBESENValue.Num]
     AddDWord(ptruint(Operands^[1]*sizeof(TBESENValue))+(ptruint(pointer(@v.Num))-ptruint(pointer(@v))));

     Add(#$dc#$25); // fsub qword ptr [BESENNumOne]
     AddDWord(ptruint(pointer(@BESENNumOne)));

     Add(#$dd#$9f); // fstp qword ptr [edi+RegisterOfs+TBESENValue.Num]
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.Num))-ptruint(pointer(@v))));
    end;
    bopCOPYBOOL:begin
     if Operands^[0]<>Operands^[1] then begin
      Add(#$c7#$87); // mov dword ptr [edi+RegisterOfs+TBESENValue.ValueType],bvtBOOLEAN
      AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.ValueType))-ptruint(pointer(@v))));
      AddDWord(bvtBOOLEAN);

      Add(#$ff#$b7); // push dword ptr [edi+RegisterOfs+TBESENValue.Bool]
      AddDWord(ptruint(Operands^[1]*sizeof(TBESENValue))+(ptruint(pointer(@v.Bool))-ptruint(pointer(@v))));

      Add(#$8f#$87); // pop dword ptr [edi+RegisterOfs+TBESENValue.Bool]
      AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.Bool))-ptruint(pointer(@v))));
     end;
    end;
    bopCOPYNUM:begin
     if Operands^[0]<>Operands^[1] then begin
      Add(#$c7#$87); // mov dword ptr [edi+RegisterOfs+TBESENValue.ValueType],bvtNUMBER
      AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.ValueType))-ptruint(pointer(@v))));
      AddDWord(bvtNUMBER);

      Add(#$dd#$87); // fld qword ptr [edi+RegisterOfs+TBESENValue.Num]
      AddDWord(ptruint(Operands^[1]*sizeof(TBESENValue))+(ptruint(pointer(@v.Num))-ptruint(pointer(@v))));

      Add(#$dd#$9f); // fstp qword ptr [edi+RegisterOfs+TBESENValue.Num]
      AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.Num))-ptruint(pointer(@v))));
     end;
    end;
    bopCOPYOBJ:begin
     if Operands^[0]<>Operands^[1] then begin
      Add(#$c7#$87); // mov dword ptr [edi+RegisterOfs+TBESENValue.ValueType],bvtOBJECT
      AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.ValueType))-ptruint(pointer(@v))));
      AddDWord(bvtOBJECT);

      Add(#$ff#$b7); // push dword ptr [edi+RegisterOfs+TBESENValue.Obj]
      AddDWord(ptruint(Operands^[1]*sizeof(TBESENValue))+(ptruint(pointer(@v.Obj))-ptruint(pointer(@v))));

      Add(#$8f#$87); // pop dword ptr [edi+RegisterOfs+TBESENValue.Obj]
      AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.Obj))-ptruint(pointer(@v))));
     end;
    end;
    bopCOPYREF:begin
     if Operands^[0]<>Operands^[1] then begin
      AddDispatcher;
     end;
    end;
    bopCOPYLOCAL:begin
     if Operands^[0]<>Operands^[1] then begin
      Add(#$c7#$87); // mov dword ptr [edi+RegisterOfs+TBESENValue.ValueType],bvtLOCAL
      AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.ValueType))-ptruint(pointer(@v))));
      AddDWord(bvtLOCAL);

      Add(#$ff#$b7); // push dword ptr [edi+RegisterOfs+TBESENValue.LocalIndex]
      AddDWord(ptruint(Operands^[1]*sizeof(TBESENValue))+(ptruint(pointer(@v.LocalIndex))-ptruint(pointer(@v))));

      Add(#$8f#$87); // pop dword ptr [edi+RegisterOfs+TBESENValue.LocalIndex]
      AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.LocalIndex))-ptruint(pointer(@v))));
     end;
    end;
    bopGETVALUEREF:begin
     AddDispatcher;
    end;
    bopPUTVALUEREF:begin
     AddDispatcher;
    end;
    bopGETVALUELOCAL:begin
     AddDispatcher;
    end;
    bopPUTVALUELOCAL:begin
     AddDispatcher;
    end;
    bopGETVALUELOCALFAST:begin
     Add(#$8d#$87); // lea eax,dword ptr [edi+RegisterOfs]
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue)));

     Add(#$8d#$97); // lea edx,dword ptr [edi+RegisterOfs]
     AddDWord(ptruint(Operands^[1]*sizeof(TBESENValue)));
     asm
      jmp @Skip
       @CodeBegin:
        mov ecx,dword ptr [edx+TBESENValue.LocalIndex]
        mov edx,dword ptr [ebx+ecx*4]
        mov ecx,dword ptr [edx+TBESENValue.ValueType]
        call dword ptr [ecx*4+offset BESENCopyValueProcs]
       @CodeEnd:
      @Skip:
      mov dword ptr CodeBegin,offset @CodeBegin
      mov dword ptr CodeEnd,offset @CodeEnd
     end;
     AddCode(CodeBegin,CodeEnd);
    end;
    bopPUTVALUELOCALFAST:begin
     Add(#$8d#$87); // lea eax,dword ptr [edi+RegisterOfs]
     AddDWord(ptruint(Operands^[1]*sizeof(TBESENValue)));

     Add(#$8d#$97); // lea edx,dword ptr [edi+RegisterOfs]
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue)));
     asm
      jmp @Skip
       @CodeBegin:
        mov ecx,dword ptr [edx+TBESENValue.LocalIndex]
        mov edx,dword ptr [ebx+ecx*4]
        xchg eax,edx
        mov ecx,dword ptr [edx+TBESENValue.ValueType]
        call dword ptr [ecx*4+offset BESENCopyValueProcs]
       @CodeEnd:
      @Skip:
      mov dword ptr CodeBegin,offset @CodeBegin
      mov dword ptr CodeEnd,offset @CodeEnd
     end;
     AddCode(CodeBegin,CodeEnd);
    end;
    bopGETVALUELOCALBOOL:begin
     Add(#$8d#$87); // lea eax,dword ptr [edi+RegisterOfs]
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue)));

     Add(#$8d#$97); // lea edx,dword ptr [edi+RegisterOfs]
     AddDWord(ptruint(Operands^[1]*sizeof(TBESENValue)));
     asm
      jmp @Skip
       @CodeBegin:
        mov ecx,dword ptr [edx+TBESENValue.LocalIndex]
        mov edx,dword ptr [ebx+ecx*4]
        mov dword ptr [eax+TBESENValue.ValueType],bvtBOOLEAN
        mov ecx,dword ptr [edx+TBESENValue.Bool]
        mov dword ptr [eax+TBESENValue.Bool],ecx
       @CodeEnd:
      @Skip:
      mov dword ptr CodeBegin,offset @CodeBegin
      mov dword ptr CodeEnd,offset @CodeEnd
     end;
     AddCode(CodeBegin,CodeEnd);
    end;
    bopPUTVALUELOCALBOOL:begin
     Add(#$8d#$87); // lea eax,dword ptr [edi+RegisterOfs]
     AddDWord(ptruint(Operands^[1]*sizeof(TBESENValue)));

     Add(#$8d#$97); // lea edx,dword ptr [edi+RegisterOfs]
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue)));
     asm
      jmp @Skip
       @CodeBegin:
        mov ecx,dword ptr [edx+TBESENValue.LocalIndex]
        mov edx,dword ptr [ebx+ecx*4]
        mov dword ptr [edx+TBESENValue.ValueType],bvtBOOLEAN
        mov ecx,dword ptr [eax+TBESENValue.Bool]
        mov dword ptr [edx+TBESENValue.Bool],ecx
       @CodeEnd:
      @Skip:
      mov dword ptr CodeBegin,offset @CodeBegin
      mov dword ptr CodeEnd,offset @CodeEnd
     end;
     AddCode(CodeBegin,CodeEnd);
    end;
    bopGETVALUELOCALNUM:begin
     Add(#$8d#$87); // lea eax,dword ptr [edi+RegisterOfs]
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue)));

     Add(#$8d#$97); // lea edx,dword ptr [edi+RegisterOfs]
     AddDWord(ptruint(Operands^[1]*sizeof(TBESENValue)));
     asm
      jmp @Skip
       @CodeBegin:
        mov ecx,dword ptr [edx+TBESENValue.LocalIndex]
        mov edx,dword ptr [ebx+ecx*4]
        mov dword ptr [eax+TBESENValue.ValueType],bvtNUMBER
        fld qword ptr [edx+TBESENValue.Num]
        fstp qword ptr [eax+TBESENValue.Num]
       @CodeEnd:
      @Skip:
      mov dword ptr CodeBegin,offset @CodeBegin
      mov dword ptr CodeEnd,offset @CodeEnd
     end;
     AddCode(CodeBegin,CodeEnd);
    end;
    bopPUTVALUELOCALNUM:begin
     Add(#$8d#$87); // lea eax,dword ptr [edi+RegisterOfs]
     AddDWord(ptruint(Operands^[1]*sizeof(TBESENValue)));

     Add(#$8d#$97); // lea edx,dword ptr [edi+RegisterOfs]
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue)));
     asm
      jmp @Skip
       @CodeBegin:
        mov ecx,dword ptr [edx+TBESENValue.LocalIndex]
        mov edx,dword ptr [ebx+ecx*4]
        mov dword ptr [edx+TBESENValue.ValueType],bvtNUMBER
        fld qword ptr [eax+TBESENValue.Num]
        fstp qword ptr [edx+TBESENValue.Num]
       @CodeEnd:
      @Skip:
      mov dword ptr CodeBegin,offset @CodeBegin
      mov dword ptr CodeEnd,offset @CodeEnd
     end;
     AddCode(CodeBegin,CodeEnd);
    end;
    bopGETVALUELOCALSTR:begin
     AddDispatcher;
    end;
    bopPUTVALUELOCALSTR:begin
     AddDispatcher;
    end;
    bopGETVALUELOCALOBJ:begin
     Add(#$8d#$87); // lea eax,dword ptr [edi+RegisterOfs]
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue)));

     Add(#$8d#$97); // lea edx,dword ptr [edi+RegisterOfs]
     AddDWord(ptruint(Operands^[1]*sizeof(TBESENValue)));
     asm
      jmp @Skip
       @CodeBegin:
        mov ecx,dword ptr [edx+TBESENValue.LocalIndex]
        mov edx,dword ptr [ebx+ecx*4]
        mov dword ptr [eax+TBESENValue.ValueType],bvtOBJECT
        mov ecx,dword ptr [edx+TBESENValue.Obj]
        mov dword ptr [eax+TBESENValue.Obj],ecx
       @CodeEnd:
      @Skip:
      mov dword ptr CodeBegin,offset @CodeBegin
      mov dword ptr CodeEnd,offset @CodeEnd
     end;
     AddCode(CodeBegin,CodeEnd);
    end;
    bopPUTVALUELOCALOBJ:begin
     Add(#$8d#$87); // lea eax,dword ptr [edi+RegisterOfs]
     AddDWord(ptruint(Operands^[1]*sizeof(TBESENValue)));

     Add(#$8d#$97); // lea edx,dword ptr [edi+RegisterOfs]
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue)));
     asm
      jmp @Skip
       @CodeBegin:
        mov ecx,dword ptr [edx+TBESENValue.LocalIndex]
        mov edx,dword ptr [ebx+ecx*4]
        mov dword ptr [edx+TBESENValue.ValueType],bvtOBJECT
        mov ecx,dword ptr [eax+TBESENValue.Obj]
        mov dword ptr [edx+TBESENValue.Obj],ecx
       @CodeEnd:
      @Skip:
      mov dword ptr CodeBegin,offset @CodeBegin
      mov dword ptr CodeEnd,offset @CodeEnd
     end;
     AddCode(CodeBegin,CodeEnd);
    end;
    bopGETVALUELOCALINDEX:begin
     Add(#$8d#$87); // lea eax,dword ptr [edi+RegisterOfs]
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue)));

     Add(#$8b#$93); // mov edx,dword ptr [ebx+LocalIndex*4]
     AddDWord(ptruint(Operands^[1])*4);
     asm
      jmp @Skip
       @CodeBegin:
        mov ecx,dword ptr [edx+TBESENValue.ValueType]
        call dword ptr [ecx*4+offset BESENCopyValueProcs]
       @CodeEnd:
      @Skip:
      mov dword ptr CodeBegin,offset @CodeBegin
      mov dword ptr CodeEnd,offset @CodeEnd
     end;
     AddCode(CodeBegin,CodeEnd);
    end;
    bopPUTVALUELOCALINDEX:begin
     Add(#$8d#$97); // lea edx,dword ptr [edi+RegisterOfs]
     AddDWord(ptruint(Operands^[1]*sizeof(TBESENValue)));

     Add(#$8b#$83); // mov eax,dword ptr [ebx+LocalIndex*4]
     AddDWord(ptruint(Operands^[0])*4);
     asm
      jmp @Skip
       @CodeBegin:
        mov ecx,dword ptr [edx+TBESENValue.ValueType]
        call dword ptr [ecx*4+offset BESENCopyValueProcs]
       @CodeEnd:
      @Skip:
      mov dword ptr CodeBegin,offset @CodeBegin
      mov dword ptr CodeEnd,offset @CodeEnd
     end;
     AddCode(CodeBegin,CodeEnd);
    end;
    bopGETVALUELOCALINDEXBOOL:begin
     Add(#$8d#$87); // lea eax,dword ptr [edi+RegisterOfs]
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue)));

     Add(#$8b#$93); // mov edx,dword ptr [ebx+LocalIndex*4]
     AddDWord(ptruint(Operands^[1])*4);
     asm
      jmp @Skip
       @CodeBegin:
        mov dword ptr [eax+TBESENValue.ValueType],bvtBOOLEAN
        mov ecx,dword ptr [edx+TBESENValue.Bool]
        mov dword ptr [eax+TBESENValue.Bool],ecx
       @CodeEnd:
      @Skip:
      mov dword ptr CodeBegin,offset @CodeBegin
      mov dword ptr CodeEnd,offset @CodeEnd
     end;
     AddCode(CodeBegin,CodeEnd);
    end;
    bopPUTVALUELOCALINDEXBOOL:begin
     Add(#$8d#$97); // lea edx,dword ptr [edi+RegisterOfs]
     AddDWord(ptruint(Operands^[1]*sizeof(TBESENValue)));

     Add(#$8b#$83); // mov eax,dword ptr [ebx+LocalIndex*4]
     AddDWord(ptruint(Operands^[0])*4);
     asm
      jmp @Skip
       @CodeBegin:
        mov dword ptr [eax+TBESENValue.ValueType],bvtBOOLEAN
        mov ecx,dword ptr [edx+TBESENValue.Bool]
        mov dword ptr [eax+TBESENValue.Bool],ecx
       @CodeEnd:
      @Skip:
      mov dword ptr CodeBegin,offset @CodeBegin
      mov dword ptr CodeEnd,offset @CodeEnd
     end;
     AddCode(CodeBegin,CodeEnd);
    end;
    bopGETVALUELOCALINDEXNUM:begin
     Add(#$8d#$87); // lea eax,dword ptr [edi+RegisterOfs]
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue)));

     Add(#$8b#$93); // mov edx,dword ptr [ebx+LocalIndex*4]
     AddDWord(ptruint(Operands^[1])*4);
     asm
      jmp @Skip
       @CodeBegin:
        mov dword ptr [eax+TBESENValue.ValueType],bvtNUMBER
        fld qword ptr [edx+TBESENValue.Num]
        fstp qword ptr [eax+TBESENValue.Num]
       @CodeEnd:
      @Skip:
      mov dword ptr CodeBegin,offset @CodeBegin
      mov dword ptr CodeEnd,offset @CodeEnd
     end;
     AddCode(CodeBegin,CodeEnd);
    end;
    bopPUTVALUELOCALINDEXNUM:begin
     Add(#$8d#$97); // lea edx,dword ptr [edi+RegisterOfs]
     AddDWord(ptruint(Operands^[1]*sizeof(TBESENValue)));

     Add(#$8b#$83); // mov eax,dword ptr [ebx+LocalIndex*4]
     AddDWord(ptruint(Operands^[0])*4);
     asm
      jmp @Skip
       @CodeBegin:
        mov dword ptr [eax+TBESENValue.ValueType],bvtNUMBER
        fld qword ptr [edx+TBESENValue.Num]
        fstp qword ptr [eax+TBESENValue.Num]
       @CodeEnd:
      @Skip:
      mov dword ptr CodeBegin,offset @CodeBegin
      mov dword ptr CodeEnd,offset @CodeEnd
     end;
     AddCode(CodeBegin,CodeEnd);
    end;
    bopGETVALUELOCALINDEXSTR:begin
     AddDispatcher;
    end;
    bopPUTVALUELOCALINDEXSTR:begin
     AddDispatcher;
    end;
    bopGETVALUELOCALINDEXOBJ:begin
     Add(#$8d#$87); // lea eax,dword ptr [edi+RegisterOfs]
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue)));

     Add(#$8b#$93); // mov edx,dword ptr [ebx+LocalIndex*4]
     AddDWord(ptruint(Operands^[1])*4);
     asm
      jmp @Skip
       @CodeBegin:
        mov dword ptr [eax+TBESENValue.ValueType],bvtOBJECT
        mov ecx,dword ptr [edx+TBESENValue.Obj]
        mov dword ptr [eax+TBESENValue.Obj],ecx
       @CodeEnd:
      @Skip:
      mov dword ptr CodeBegin,offset @CodeBegin
      mov dword ptr CodeEnd,offset @CodeEnd
     end;
     AddCode(CodeBegin,CodeEnd);
    end;
    bopPUTVALUELOCALINDEXOBJ:begin
     Add(#$8d#$97); // lea edx,dword ptr [edi+RegisterOfs]
     AddDWord(ptruint(Operands^[1]*sizeof(TBESENValue)));

     Add(#$8b#$83); // mov eax,dword ptr [ebx+LocalIndex*4]
     AddDWord(ptruint(Operands^[0])*4);
     asm
      jmp @Skip
       @CodeBegin:
        mov dword ptr [eax+TBESENValue.ValueType],bvtOBJECT
        mov ecx,dword ptr [edx+TBESENValue.Obj]
        mov dword ptr [eax+TBESENValue.Obj],ecx
       @CodeEnd:
      @Skip:
      mov dword ptr CodeBegin,offset @CodeBegin
      mov dword ptr CodeEnd,offset @CodeEnd
     end;
     AddCode(CodeBegin,CodeEnd);
    end;
    bopLOOPINITCOUNT:begin
    end;
    bopLOOPADDCOUNT:begin
    end;
    bopTRACE:begin
     if TBESEN(CodeContext.Instance).CodeTracable then begin
      AddDispatcher;
     end;
    end;
    bopLTBOOL:begin
{$ifdef UseSafeOperations}
     AddDispatcher;
{$else}
     Add(#$8b#$87); // mov eax,dword ptr [edi+RegisterOfs+TBESENValue.Bool]
     AddDWord(ptruint(Operands^[1]*sizeof(TBESENValue))+(ptruint(pointer(@v.Bool))-ptruint(pointer(@v))));

     Add(#$8b#$97); // mov edx,dword ptr [edi+RegisterOfs+TBESENValue.Bool]
     AddDWord(ptruint(Operands^[2]*sizeof(TBESENValue))+(ptruint(pointer(@v.Bool))-ptruint(pointer(@v))));
     
     asm
      jmp @Skip
       @CodeBegin:
        cmp al,$01
        sbb eax,eax
        inc eax
        cmp dl,$01
        sbb edx,edx
        inc edx
        cmp al,dl
        setb al
        neg al
        sbb eax,eax
       @CodeEnd:
      @Skip:
      mov dword ptr CodeBegin,offset @CodeBegin
      mov dword ptr CodeEnd,offset @CodeEnd
     end;

     AddCode(CodeBegin,CodeEnd);

     if (Operands^[0]<>Operands^[1]) and (Operands^[0]<>Operands^[2]) then begin
      Add(#$c7#$87); // mov dword ptr [edi+RegisterOfs+TBESENValue.ValueType],bvtBOOLEAN
      AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.ValueType))-ptruint(pointer(@v))));
      AddDWord(bvtBOOLEAN);
     end;

     Add(#$89#$87); // mov dword ptr [edi+RegisterOfs+TBESENValue.Bool],eax
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.Bool))-ptruint(pointer(@v))));
{$endif}
    end;
    bopGTBOOL:begin
{$ifdef UseSafeOperations}
     AddDispatcher;
{$else}
     Add(#$8b#$87); // mov eax,dword ptr [edi+RegisterOfs+TBESENValue.Bool]
     AddDWord(ptruint(Operands^[1]*sizeof(TBESENValue))+(ptruint(pointer(@v.Bool))-ptruint(pointer(@v))));

     Add(#$8b#$97); // mov edx,dword ptr [edi+RegisterOfs+TBESENValue.Bool]
     AddDWord(ptruint(Operands^[2]*sizeof(TBESENValue))+(ptruint(pointer(@v.Bool))-ptruint(pointer(@v))));
     
     asm
      jmp @Skip
       @CodeBegin:
        cmp al,$01
        sbb eax,eax
        inc eax
        cmp dl,$01
        sbb edx,edx
        inc edx
        cmp al,dl
        setnbe al
        neg al
        sbb eax,eax
       @CodeEnd:
      @Skip:
      mov dword ptr CodeBegin,offset @CodeBegin
      mov dword ptr CodeEnd,offset @CodeEnd
     end;

     AddCode(CodeBegin,CodeEnd);

     if (Operands^[0]<>Operands^[1]) and (Operands^[0]<>Operands^[2]) then begin
      Add(#$c7#$87); // mov dword ptr [edi+RegisterOfs+TBESENValue.ValueType],bvtBOOLEAN
      AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.ValueType))-ptruint(pointer(@v))));
      AddDWord(bvtBOOLEAN);
     end;

     Add(#$89#$87); // mov dword ptr [edi+RegisterOfs+TBESENValue.Bool],eax
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.Bool))-ptruint(pointer(@v))));
{$endif}
    end;
    bopLEBOOL:begin
{$ifdef UseSafeOperations}
     AddDispatcher;
{$else}
     Add(#$8b#$87); // mov eax,dword ptr [edi+RegisterOfs+TBESENValue.Bool]
     AddDWord(ptruint(Operands^[1]*sizeof(TBESENValue))+(ptruint(pointer(@v.Bool))-ptruint(pointer(@v))));

     Add(#$8b#$97); // mov edx,dword ptr [edi+RegisterOfs+TBESENValue.Bool]
     AddDWord(ptruint(Operands^[2]*sizeof(TBESENValue))+(ptruint(pointer(@v.Bool))-ptruint(pointer(@v))));

     asm
      jmp @Skip
       @CodeBegin:
        cmp al,$01
        sbb eax,eax
        inc eax
        cmp dl,$01
        sbb edx,edx
        inc edx
        cmp al,dl
        setbe al
        neg al
        sbb eax,eax
       @CodeEnd:
      @Skip:
      mov dword ptr CodeBegin,offset @CodeBegin
      mov dword ptr CodeEnd,offset @CodeEnd
     end;

     AddCode(CodeBegin,CodeEnd);

     if (Operands^[0]<>Operands^[1]) and (Operands^[0]<>Operands^[2]) then begin
      Add(#$c7#$87); // mov dword ptr [edi+RegisterOfs+TBESENValue.ValueType],bvtBOOLEAN
      AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.ValueType))-ptruint(pointer(@v))));
      AddDWord(bvtBOOLEAN);
     end;

     Add(#$89#$87); // mov dword ptr [edi+RegisterOfs+TBESENValue.Bool],eax
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.Bool))-ptruint(pointer(@v))));
{$endif}
    end;
    bopGEBOOL:begin
{$ifdef UseSafeOperations}
     AddDispatcher;
{$else}
     Add(#$8b#$87); // mov eax,dword ptr [edi+RegisterOfs+TBESENValue.Bool]
     AddDWord(ptruint(Operands^[1]*sizeof(TBESENValue))+(ptruint(pointer(@v.Bool))-ptruint(pointer(@v))));

     Add(#$8b#$97); // mov edx,dword ptr [edi+RegisterOfs+TBESENValue.Bool]
     AddDWord(ptruint(Operands^[2]*sizeof(TBESENValue))+(ptruint(pointer(@v.Bool))-ptruint(pointer(@v))));

     asm
      jmp @Skip
       @CodeBegin:
        cmp al,$01
        sbb eax,eax
        inc eax
        cmp dl,$01
        sbb edx,edx
        inc edx
        cmp al,dl
        setnb al
        neg al
        sbb eax,eax
       @CodeEnd:
      @Skip:
      mov dword ptr CodeBegin,offset @CodeBegin
      mov dword ptr CodeEnd,offset @CodeEnd
     end;

     AddCode(CodeBegin,CodeEnd);

     if (Operands^[0]<>Operands^[1]) and (Operands^[0]<>Operands^[2]) then begin
      Add(#$c7#$87); // mov dword ptr [edi+RegisterOfs+TBESENValue.ValueType],bvtBOOLEAN
      AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.ValueType))-ptruint(pointer(@v))));
      AddDWord(bvtBOOLEAN);
     end;

     Add(#$89#$87); // mov dword ptr [edi+RegisterOfs+TBESENValue.Bool],eax
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.Bool))-ptruint(pointer(@v))));
{$endif}
    end;
    bopEQBOOL:begin
{$ifdef UseSafeOperations}
     AddDispatcher;
{$else}
     Add(#$8b#$87); // mov eax,dword ptr [edi+RegisterOfs+TBESENValue.Bool]
     AddDWord(ptruint(Operands^[1]*sizeof(TBESENValue))+(ptruint(pointer(@v.Bool))-ptruint(pointer(@v))));

     Add(#$8b#$97); // mov edx,dword ptr [edi+RegisterOfs+TBESENValue.Bool]
     AddDWord(ptruint(Operands^[2]*sizeof(TBESENValue))+(ptruint(pointer(@v.Bool))-ptruint(pointer(@v))));

     asm
      jmp @Skip
       @CodeBegin:
        cmp eax,edx
        setz al
        neg al
        sbb eax,eax
       @CodeEnd:
      @Skip:
      mov dword ptr CodeBegin,offset @CodeBegin
      mov dword ptr CodeEnd,offset @CodeEnd
     end;

     AddCode(CodeBegin,CodeEnd);

     if (Operands^[0]<>Operands^[1]) and (Operands^[0]<>Operands^[2]) then begin
      Add(#$c7#$87); // mov dword ptr [edi+RegisterOfs+TBESENValue.ValueType],bvtBOOLEAN
      AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.ValueType))-ptruint(pointer(@v))));
      AddDWord(bvtBOOLEAN);
     end;

     Add(#$89#$87); // mov dword ptr [edi+RegisterOfs+TBESENValue.Bool],eax
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.Bool))-ptruint(pointer(@v))));
{$endif}
    end;
    bopNEQBOOL:begin
{$ifdef UseSafeOperations}
     AddDispatcher;
{$else}
     Add(#$8b#$87); // mov eax,dword ptr [edi+RegisterOfs+TBESENValue.Bool]
     AddDWord(ptruint(Operands^[1]*sizeof(TBESENValue))+(ptruint(pointer(@v.Bool))-ptruint(pointer(@v))));

     Add(#$8b#$97); // mov edx,dword ptr [edi+RegisterOfs+TBESENValue.Bool]
     AddDWord(ptruint(Operands^[2]*sizeof(TBESENValue))+(ptruint(pointer(@v.Bool))-ptruint(pointer(@v))));

     asm
      jmp @Skip
       @CodeBegin:
        cmp eax,edx
        setnz al
        neg al
        sbb eax,eax
       @CodeEnd:
      @Skip:
      mov dword ptr CodeBegin,offset @CodeBegin
      mov dword ptr CodeEnd,offset @CodeEnd
     end;

     AddCode(CodeBegin,CodeEnd);

     if (Operands^[0]<>Operands^[1]) and (Operands^[0]<>Operands^[2]) then begin
      Add(#$c7#$87); // mov dword ptr [edi+RegisterOfs+TBESENValue.ValueType],bvtBOOLEAN
      AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.ValueType))-ptruint(pointer(@v))));
      AddDWord(bvtBOOLEAN);
     end;

     Add(#$89#$87); // mov dword ptr [edi+RegisterOfs+TBESENValue.Bool],eax
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.Bool))-ptruint(pointer(@v))));
{$endif}
    end;
    bopLTNUM:begin
{$ifdef UseSafeOperations}
     AddDispatcher;
{$else}
     Add(#$dd#$87); // fld qword ptr [edi+RegisterOfs+TBESENValue.Num]
     AddDWord(ptruint(Operands^[1]*sizeof(TBESENValue))+(ptruint(pointer(@v.Num))-ptruint(pointer(@v))));

     Add(#$dc#$9f); // fcomp qword ptr [edi+RegisterOfs+TBESENValue.Num]
     AddDWord(ptruint(Operands^[2]*sizeof(TBESENValue))+(ptruint(pointer(@v.Num))-ptruint(pointer(@v))));

     asm
      jmp @Skip
       @CodeBegin:
        fstsw ax
        sahf
        setb al
        setnp cl
        and al,cl
        neg al
        sbb eax,eax
       @CodeEnd:
      @Skip:
      mov dword ptr CodeBegin,offset @CodeBegin
      mov dword ptr CodeEnd,offset @CodeEnd
     end;

     AddCode(CodeBegin,CodeEnd);

     Add(#$c7#$87); // mov dword ptr [edi+RegisterOfs+TBESENValue.ValueType],bvtBOOLEAN
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.ValueType))-ptruint(pointer(@v))));
     AddDWord(bvtBOOLEAN);

     Add(#$89#$87); // mov dword ptr [edi+RegisterOfs+TBESENValue.Bool],eax
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.Bool))-ptruint(pointer(@v))));
{$endif}
    end;
    bopGTNUM:begin
{$ifdef UseSafeOperations}
     AddDispatcher;
{$else}
     Add(#$dd#$87); // fld qword ptr [edi+RegisterOfs+TBESENValue.Num]
     AddDWord(ptruint(Operands^[1]*sizeof(TBESENValue))+(ptruint(pointer(@v.Num))-ptruint(pointer(@v))));

     Add(#$dc#$9f); // fcomp qword ptr [edi+RegisterOfs+TBESENValue.Num]
     AddDWord(ptruint(Operands^[2]*sizeof(TBESENValue))+(ptruint(pointer(@v.Num))-ptruint(pointer(@v))));

     asm
      jmp @Skip
       @CodeBegin:
        fstsw ax
        sahf
        setnbe al
        setnp cl
        and al,cl
        neg al
        sbb eax,eax
       @CodeEnd:
      @Skip:
      mov dword ptr CodeBegin,offset @CodeBegin
      mov dword ptr CodeEnd,offset @CodeEnd
     end;

     AddCode(CodeBegin,CodeEnd);

     Add(#$c7#$87); // mov dword ptr [edi+RegisterOfs+TBESENValue.ValueType],bvtBOOLEAN
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.ValueType))-ptruint(pointer(@v))));
     AddDWord(bvtBOOLEAN);

     Add(#$89#$87); // mov dword ptr [edi+RegisterOfs+TBESENValue.Bool],eax
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.Bool))-ptruint(pointer(@v))));
{$endif}
    end;
    bopLENUM:begin
{$ifdef UseSafeOperations}
     AddDispatcher;
{$else}
     Add(#$dd#$87); // fld qword ptr [edi+RegisterOfs+TBESENValue.Num]
     AddDWord(ptruint(Operands^[1]*sizeof(TBESENValue))+(ptruint(pointer(@v.Num))-ptruint(pointer(@v))));

     Add(#$dc#$9f); // fcomp qword ptr [edi+RegisterOfs+TBESENValue.Num]
     AddDWord(ptruint(Operands^[2]*sizeof(TBESENValue))+(ptruint(pointer(@v.Num))-ptruint(pointer(@v))));

     asm
      jmp @Skip
       @CodeBegin:
        fstsw ax
        sahf
        setbe al
        setnp cl
        and al,cl
        neg al
        sbb eax,eax
       @CodeEnd:
      @Skip:
      mov dword ptr CodeBegin,offset @CodeBegin
      mov dword ptr CodeEnd,offset @CodeEnd
     end;

     AddCode(CodeBegin,CodeEnd);

     Add(#$c7#$87); // mov dword ptr [edi+RegisterOfs+TBESENValue.ValueType],bvtBOOLEAN
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.ValueType))-ptruint(pointer(@v))));
     AddDWord(bvtBOOLEAN);

     Add(#$89#$87); // mov dword ptr [edi+RegisterOfs+TBESENValue.Bool],eax
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.Bool))-ptruint(pointer(@v))));
{$endif}
    end;
    bopGENUM:begin
{$ifdef UseSafeOperations}
     AddDispatcher;
{$else}
     Add(#$dd#$87); // fld qword ptr [edi+RegisterOfs+TBESENValue.Num]
     AddDWord(ptruint(Operands^[1]*sizeof(TBESENValue))+(ptruint(pointer(@v.Num))-ptruint(pointer(@v))));

     Add(#$dc#$9f); // fcomp qword ptr [edi+RegisterOfs+TBESENValue.Num]
     AddDWord(ptruint(Operands^[2]*sizeof(TBESENValue))+(ptruint(pointer(@v.Num))-ptruint(pointer(@v))));

     asm
      jmp @Skip
       @CodeBegin:
        fstsw ax
        sahf
        setnb al
        setnp cl
        and al,cl
        neg al
        sbb eax,eax
       @CodeEnd:
      @Skip:
      mov dword ptr CodeBegin,offset @CodeBegin
      mov dword ptr CodeEnd,offset @CodeEnd
     end;

     AddCode(CodeBegin,CodeEnd);

     Add(#$c7#$87); // mov dword ptr [edi+RegisterOfs+TBESENValue.ValueType],bvtBOOLEAN
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.ValueType))-ptruint(pointer(@v))));
     AddDWord(bvtBOOLEAN);

     Add(#$89#$87); // mov dword ptr [edi+RegisterOfs+TBESENValue.Bool],eax
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.Bool))-ptruint(pointer(@v))));
{$endif}
    end;
    bopEQNUM:begin
{$ifdef UseSafeOperations}
     AddDispatcher;
{$else}
     Add(#$dd#$87); // fld qword ptr [edi+RegisterOfs+TBESENValue.Num]
     AddDWord(ptruint(Operands^[1]*sizeof(TBESENValue))+(ptruint(pointer(@v.Num))-ptruint(pointer(@v))));

     Add(#$dc#$9f); // fcomp qword ptr [edi+RegisterOfs+TBESENValue.Num]
     AddDWord(ptruint(Operands^[2]*sizeof(TBESENValue))+(ptruint(pointer(@v.Num))-ptruint(pointer(@v))));

     asm
      jmp @Skip
       @CodeBegin:
        fstsw ax
        sahf
        setz al
        setnp cl
        and al,cl
        neg al
        sbb eax,eax
       @CodeEnd:
      @Skip:
      mov dword ptr CodeBegin,offset @CodeBegin
      mov dword ptr CodeEnd,offset @CodeEnd
     end;

     AddCode(CodeBegin,CodeEnd);

     Add(#$c7#$87); // mov dword ptr [edi+RegisterOfs+TBESENValue.ValueType],bvtBOOLEAN
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.ValueType))-ptruint(pointer(@v))));
     AddDWord(bvtBOOLEAN);

     Add(#$89#$87); // mov dword ptr [edi+RegisterOfs+TBESENValue.Bool],eax
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.Bool))-ptruint(pointer(@v))));
{$endif}
    end;
    bopNEQNUM:begin
{$ifdef UseSafeOperations}
     AddDispatcher;
{$else}
     Add(#$dd#$87); // fld qword ptr [edi+RegisterOfs+TBESENValue.Num]
     AddDWord(ptruint(Operands^[1]*sizeof(TBESENValue))+(ptruint(pointer(@v.Num))-ptruint(pointer(@v))));

     Add(#$dc#$9f); // fcomp qword ptr [edi+RegisterOfs+TBESENValue.Num]
     AddDWord(ptruint(Operands^[2]*sizeof(TBESENValue))+(ptruint(pointer(@v.Num))-ptruint(pointer(@v))));

     asm
      jmp @Skip
       @CodeBegin:
        fstsw ax
        sahf
        setz al
        setnp cl
        and al,cl
        neg al
        sbb eax,eax
        not eax
       @CodeEnd:
      @Skip:
      mov dword ptr CodeBegin,offset @CodeBegin
      mov dword ptr CodeEnd,offset @CodeEnd
     end;

     AddCode(CodeBegin,CodeEnd);

     Add(#$c7#$87); // mov dword ptr [edi+RegisterOfs+TBESENValue.ValueType],bvtBOOLEAN
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.ValueType))-ptruint(pointer(@v))));
     AddDWord(bvtBOOLEAN);

     Add(#$89#$87); // mov dword ptr [edi+RegisterOfs+TBESENValue.Bool],eax
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.Bool))-ptruint(pointer(@v))));
{$endif}
    end;
    bopLTSTR:begin
     AddDispatcher;
    end;
    bopGTSTR:begin
     AddDispatcher;
    end;
    bopLESTR:begin
     AddDispatcher;
    end;
    bopGESTR:begin
     AddDispatcher;
    end;
    bopEQSTR:begin
     AddDispatcher;
    end;
    bopNEQSTR:begin
     AddDispatcher;
    end;
    bopSHLBOOL:begin
     AddDispatcher;
    end;
    bopSHRBOOL:begin
     AddDispatcher;
    end;
    bopBANDBOOL:begin
     AddDispatcher;
    end;
    bopBXORBOOL:begin
     AddDispatcher;
    end;
    bopBORBOOL:begin
     AddDispatcher;
    end;
    bopSHLNUM:begin
     AddDispatcher;
    end;
    bopSHRNUM:begin
     AddDispatcher;
    end;
    bopUSHRNUM:begin
     AddDispatcher;
    end;
    bopBANDNUM:begin
     AddDispatcher;
    end;
    bopBXORNUM:begin
     AddDispatcher;
    end;
    bopBORNUM:begin
     AddDispatcher;
    end;
    bopSETCUNDEF:begin
     asm
      jmp @Skip
       @CodeBegin:
        lea eax,dword ptr [esi+TBESENCodeContext.ResultValue]
        mov dword ptr [eax+TBESENValue.ValueType],bvtUNDEFINED
       @CodeEnd:
      @Skip:
      mov dword ptr CodeBegin,offset @CodeBegin
      mov dword ptr CodeEnd,offset @CodeEnd
     end;
     AddCode(CodeBegin,CodeEnd);
    end;
    bopSETCNULL:begin
     asm
      jmp @Skip
       @CodeBegin:
        lea eax,dword ptr [esi+TBESENCodeContext.ResultValue]
        mov dword ptr [eax+TBESENValue.ValueType],bvtNULL
       @CodeEnd:
      @Skip:
      mov dword ptr CodeBegin,offset @CodeBegin
      mov dword ptr CodeEnd,offset @CodeEnd
     end;
     AddCode(CodeBegin,CodeEnd);
    end;
    bopSETCBOOL:begin
     Add(#$8d#$97); // lea edx,dword ptr [edi+RegisterOfs]
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue)));
     asm
      jmp @Skip
       @CodeBegin:
        lea eax,dword ptr [esi+TBESENCodeContext.ResultValue]
        mov dword ptr [eax+TBESENValue.ValueType],bvtBOOLEAN
        mov ecx,dword ptr [edx+TBESENValue.Bool]
        mov dword ptr [eax+TBESENValue.Bool],ecx
       @CodeEnd:
      @Skip:
      mov dword ptr CodeBegin,offset @CodeBegin
      mov dword ptr CodeEnd,offset @CodeEnd
     end;
     AddCode(CodeBegin,CodeEnd);
    end;
    bopSETCNUM:begin
     Add(#$8d#$97); // lea edx,dword ptr [edi+RegisterOfs]
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue)));
     asm
      jmp @Skip
       @CodeBegin:
        lea eax,dword ptr [esi+TBESENCodeContext.ResultValue]
        mov dword ptr [eax+TBESENValue.ValueType],bvtNUMBER
        fld qword ptr [edx+TBESENValue.Num]
        fstp qword ptr [eax+TBESENValue.Num]
       @CodeEnd:
      @Skip:
      mov dword ptr CodeBegin,offset @CodeBegin
      mov dword ptr CodeEnd,offset @CodeEnd
     end;
     AddCode(CodeBegin,CodeEnd);
    end;
    bopSETCSTR:begin
     AddDispatcher;
    end;
    bopSETCOBJ:begin
     Add(#$8d#$97); // lea edx,dword ptr [edi+RegisterOfs]
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue)));
     asm
      jmp @Skip
       @CodeBegin:
        lea eax,dword ptr [esi+TBESENCodeContext.ResultValue]
        mov dword ptr [eax+TBESENValue.ValueType],bvtOBJECT
        mov ecx,dword ptr [edx+TBESENValue.Obj]
        mov dword ptr [eax+TBESENValue.Obj],ecx
       @CodeEnd:
      @Skip:
      mov dword ptr CodeBegin,offset @CodeBegin
      mov dword ptr CodeEnd,offset @CodeEnd
     end;
     AddCode(CodeBegin,CodeEnd);
    end;
    bopTRACENEW:begin
     AddDispatcher;
    end;
    bopTRACECALL:begin
     AddDispatcher;
    end;
    bopLTNUMCONST:begin
{$ifdef UseSafeOperations}
     AddDispatcher;
{$else}
     Add(#$dd#$87); // fld qword ptr [edi+RegisterOfs+TBESENValue.Num]
     AddDWord(ptruint(Operands^[1]*sizeof(TBESENValue))+(ptruint(pointer(@v.Num))-ptruint(pointer(@v))));

     Add(#$dc#$1d); // fcomp qword ptr [Literal.Num]
     AddDWord(ptruint(pointer(@Code.Literals[Operands^[2]].Num)));

     asm
      jmp @Skip
       @CodeBegin:
        fstsw ax
        sahf
        setb al
        setnp cl
        and al,cl
        neg al
        sbb eax,eax
       @CodeEnd:
      @Skip:
      mov dword ptr CodeBegin,offset @CodeBegin
      mov dword ptr CodeEnd,offset @CodeEnd
     end;

     AddCode(CodeBegin,CodeEnd);

     Add(#$c7#$87); // mov dword ptr [edi+RegisterOfs+TBESENValue.ValueType],bvtBOOLEAN
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.ValueType))-ptruint(pointer(@v))));
     AddDWord(bvtBOOLEAN);

     Add(#$89#$87); // mov dword ptr [edi+RegisterOfs+TBESENValue.Bool],eax
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.Bool))-ptruint(pointer(@v))));
{$endif}
    end;
    bopGTNUMCONST:begin
{$ifdef UseSafeOperations}
     AddDispatcher;
{$else}
     Add(#$dd#$87); // fld qword ptr [edi+RegisterOfs+TBESENValue.Num]
     AddDWord(ptruint(Operands^[1]*sizeof(TBESENValue))+(ptruint(pointer(@v.Num))-ptruint(pointer(@v))));

     Add(#$dc#$1d); // fcomp qword ptr [Literal.Num]
     AddDWord(ptruint(pointer(@Code.Literals[Operands^[2]].Num)));

     asm
      jmp @Skip
       @CodeBegin:
        fstsw ax
        sahf
        setnbe al
        setnp cl
        and al,cl
        neg al
        sbb eax,eax
       @CodeEnd:
      @Skip:
      mov dword ptr CodeBegin,offset @CodeBegin
      mov dword ptr CodeEnd,offset @CodeEnd
     end;

     AddCode(CodeBegin,CodeEnd);

     Add(#$c7#$87); // mov dword ptr [edi+RegisterOfs+TBESENValue.ValueType],bvtBOOLEAN
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.ValueType))-ptruint(pointer(@v))));
     AddDWord(bvtBOOLEAN);

     Add(#$89#$87); // mov dword ptr [edi+RegisterOfs+TBESENValue.Bool],eax
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.Bool))-ptruint(pointer(@v))));
{$endif}
    end;
    bopLENUMCONST:begin
{$ifdef UseSafeOperations}
     AddDispatcher;
{$else}
     Add(#$dd#$87); // fld qword ptr [edi+RegisterOfs+TBESENValue.Num]
     AddDWord(ptruint(Operands^[1]*sizeof(TBESENValue))+(ptruint(pointer(@v.Num))-ptruint(pointer(@v))));

     Add(#$dc#$1d); // fcomp qword ptr [Literal.Num]
     AddDWord(ptruint(pointer(@Code.Literals[Operands^[2]].Num)));

     asm
      jmp @Skip
       @CodeBegin:
        fstsw ax
        sahf
        setbe al
        setnp cl
        and al,cl
        neg al
        sbb eax,eax
       @CodeEnd:
      @Skip:
      mov dword ptr CodeBegin,offset @CodeBegin
      mov dword ptr CodeEnd,offset @CodeEnd
     end;

     AddCode(CodeBegin,CodeEnd);

     Add(#$c7#$87); // mov dword ptr [edi+RegisterOfs+TBESENValue.ValueType],bvtBOOLEAN
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.ValueType))-ptruint(pointer(@v))));
     AddDWord(bvtBOOLEAN);

     Add(#$89#$87); // mov dword ptr [edi+RegisterOfs+TBESENValue.Bool],eax
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.Bool))-ptruint(pointer(@v))));
{$endif}
    end;
    bopGENUMCONST:begin
{$ifdef UseSafeOperations}
     AddDispatcher;
{$else}
     Add(#$dd#$87); // fld qword ptr [edi+RegisterOfs+TBESENValue.Num]
     AddDWord(ptruint(Operands^[1]*sizeof(TBESENValue))+(ptruint(pointer(@v.Num))-ptruint(pointer(@v))));

     Add(#$dc#$1d); // fcomp qword ptr [Literal.Num]
     AddDWord(ptruint(pointer(@Code.Literals[Operands^[2]].Num)));

     asm
      jmp @Skip
       @CodeBegin:
        fstsw ax
        sahf
        setnb al
        setnp cl
        and al,cl
        neg al
        sbb eax,eax
       @CodeEnd:
      @Skip:
      mov dword ptr CodeBegin,offset @CodeBegin
      mov dword ptr CodeEnd,offset @CodeEnd
     end;

     AddCode(CodeBegin,CodeEnd);

     Add(#$c7#$87); // mov dword ptr [edi+RegisterOfs+TBESENValue.ValueType],bvtBOOLEAN
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.ValueType))-ptruint(pointer(@v))));
     AddDWord(bvtBOOLEAN);

     Add(#$89#$87); // mov dword ptr [edi+RegisterOfs+TBESENValue.Bool],eax
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.Bool))-ptruint(pointer(@v))));
{$endif}
    end;
    bopEQNUMCONST:begin
{$ifdef UseSafeOperations}
     AddDispatcher;
{$else}
     Add(#$dd#$87); // fld qword ptr [edi+RegisterOfs+TBESENValue.Num]
     AddDWord(ptruint(Operands^[1]*sizeof(TBESENValue))+(ptruint(pointer(@v.Num))-ptruint(pointer(@v))));

     Add(#$dc#$1d); // fcomp qword ptr [Literal.Num]
     AddDWord(ptruint(pointer(@Code.Literals[Operands^[2]].Num)));

     asm
      jmp @Skip
       @CodeBegin:
        fstsw ax
        sahf
        setz al
        setnp cl
        and al,cl
        neg al
        sbb eax,eax
       @CodeEnd:
      @Skip:
      mov dword ptr CodeBegin,offset @CodeBegin
      mov dword ptr CodeEnd,offset @CodeEnd
     end;

     AddCode(CodeBegin,CodeEnd);

     Add(#$c7#$87); // mov dword ptr [edi+RegisterOfs+TBESENValue.ValueType],bvtBOOLEAN
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.ValueType))-ptruint(pointer(@v))));
     AddDWord(bvtBOOLEAN);

     Add(#$89#$87); // mov dword ptr [edi+RegisterOfs+TBESENValue.Bool],eax
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.Bool))-ptruint(pointer(@v))));
{$endif}
    end;
    bopNEQNUMCONST:begin
{$ifdef UseSafeOperations}
     AddDispatcher;
{$else}
     Add(#$dd#$87); // fld qword ptr [edi+RegisterOfs+TBESENValue.Num]
     AddDWord(ptruint(Operands^[1]*sizeof(TBESENValue))+(ptruint(pointer(@v.Num))-ptruint(pointer(@v))));

     Add(#$dc#$1d); // fcomp qword ptr [Literal.Num]
     AddDWord(ptruint(pointer(@Code.Literals[Operands^[2]].Num)));

     asm
      jmp @Skip
       @CodeBegin:
        fstsw ax
        sahf
        setz al
        setnp cl
        and al,cl
        neg al
        sbb eax,eax
        not eax
       @CodeEnd:
      @Skip:
      mov dword ptr CodeBegin,offset @CodeBegin
      mov dword ptr CodeEnd,offset @CodeEnd
     end;

     AddCode(CodeBegin,CodeEnd);

     Add(#$c7#$87); // mov dword ptr [edi+RegisterOfs+TBESENValue.ValueType],bvtBOOLEAN
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.ValueType))-ptruint(pointer(@v))));
     AddDWord(bvtBOOLEAN);

     Add(#$89#$87); // mov dword ptr [edi+RegisterOfs+TBESENValue.Bool],eax
     AddDWord(ptruint(Operands^[0]*sizeof(TBESENValue))+(ptruint(pointer(@v.Bool))-ptruint(pointer(@v))));
{$endif}
    end;
    bopJZERO:begin
{$ifdef UseSafeOperations}
     AddDispatcher;
{$else}
     if longword(Operands^[0])<>CurrentPC then begin
      Add(#$dd#$87); // fld qword ptr [edi+RegisterOfs+TBESENValue.Num]
      AddDWord(ptruint(Operands^[1]*sizeof(TBESENValue))+(ptruint(pointer(@v.Num))-ptruint(pointer(@v))));

      Add(#$dc#$1d); // fcomp qword ptr [BESENDoubleZero]
      AddDWord(ptruint(pointer(@BESENDoubleZero)));

      asm
       jmp @Skip
        @CodeBegin:
         fstsw ax
         sahf
         setz al
         setnp cl
         and al,cl
         neg al
         sbb eax,eax
         cmp eax,0
        @CodeEnd:
       @Skip:
       mov dword ptr CodeBegin,offset @CodeBegin
       mov dword ptr CodeEnd,offset @CodeEnd
      end;

      AddCode(CodeBegin,CodeEnd);

      Add(#$0f#$85); // jnz Arg
      if CountFixups>=length(Fixups) then begin
       SetLength(Fixups,CountFixups+4096);
      end;
      Fixups[CountFixups].Kind:=fkOFS;
      Fixups[CountFixups].Ofs:=CodeBufferLen;
      Fixups[CountFixups].ToOfs:=Operands^[0];
      inc(CountFixups);
      Add(#$00#$00#$00#$00);
     end;
{$endif}
    end;
    bopJNZERO:begin
{$ifdef UseSafeOperations}
     AddDispatcher;
{$else}
     if longword(Operands^[0])<>CurrentPC then begin
      Add(#$dd#$87); // fld qword ptr [edi+RegisterOfs+TBESENValue.Num]
      AddDWord(ptruint(Operands^[1]*sizeof(TBESENValue))+(ptruint(pointer(@v.Num))-ptruint(pointer(@v))));

      Add(#$dc#$1d); // fcomp qword ptr [BESENDoubleZero]
      AddDWord(ptruint(pointer(@BESENDoubleZero)));

      asm
       jmp @Skip
        @CodeBegin:
         fstsw ax
         sahf
         setz al
         setnp cl
         and al,cl
         neg al
         sbb eax,eax
         cmp eax,0
        @CodeEnd:
       @Skip:
       mov dword ptr CodeBegin,offset @CodeBegin
       mov dword ptr CodeEnd,offset @CodeEnd
      end;

      AddCode(CodeBegin,CodeEnd);

      Add(#$0f#$84); // jz Arg
      if CountFixups>=length(Fixups) then begin
       SetLength(Fixups,CountFixups+4096);
      end;
      Fixups[CountFixups].Kind:=fkOFS;
      Fixups[CountFixups].Ofs:=CodeBufferLen;
      Fixups[CountFixups].ToOfs:=Operands^[0];
      inc(CountFixups);
      Add(#$00#$00#$00#$00);
     end;
{$endif}
    end;
    else begin
     AddDispatcher;
    end;
   end;

  end;
  RetOfs:=CodeBufferLen;
  Add(#$c3); // ret
  SetLength(CodeBuffer,CodeBufferLen);
  Code.NativeCodeSize:=CodeBufferLen;
  Code.NativeCode:=TBESEN(CodeContext.Instance).NativeCodeMemoryManager.GetMemory(Code.NativeCodeSize);
  move(CodeBuffer[0],Code.NativeCode^,Code.NativeCodeSize);
  SetLength(Code.NativeCodePCOffsets,Code.ByteCodeLen);
  for i:=0 to Code.ByteCodeLen-1 do begin
   Code.NativeCodePCOffsets[i]:=pointer(@PBESENByteArray(Code.NativeCode)[Offsets[i]]);
  end;
  for i:=0 to CountFixups-1 do begin
   case FixUps[i].Kind of
    fkPTR:begin
     ptruint(pointer(@PBESENByteArray(Code.NativeCode)[FixUps[i].Ofs])^):=(ptruint(FixUps[i].Dest)-(ptruint(pointer(@PBESENByteArray(Code.NativeCode)[FixUps[i].Ofs]))+4));
    end;
    fkRET:begin
     ptruint(pointer(@PBESENByteArray(Code.NativeCode)[FixUps[i].Ofs])^):=(ptruint(pointer(@PBESENByteArray(Code.NativeCode)[RetOfs]))-(ptruint(pointer(@PBESENByteArray(Code.NativeCode)[FixUps[i].Ofs]))+4));
    end;
    fkOFS:begin
     ptruint(pointer(@PBESENByteArray(Code.NativeCode)[FixUps[i].Ofs])^):=(ptruint(Code.NativeCodePCOffsets[FixUps[i].ToOfs])-(ptruint(pointer(@PBESENByteArray(Code.NativeCode)[FixUps[i].Ofs]))+4));
    end;
   end;
  end;
  result:=true;
 finally
  SetLength(Fixups,0);
  SetLength(Offsets,0);
  SetLength(CodeBuffer,0);
 end;
end;

function BESENExecuteNativeCode(ACodeContext:TObject):TBESENBoolean; {$ifdef UseRegister}register;{$endif}
asm
 push ebp
 push ebx
 push esi
 push edi
  mov esi,eax //ACodeContext
  mov dword ptr [esi+TBESENCodeContext.BlockRunning],$ffffffff
  mov edi,dword ptr [esi+TBESENCodeContext.RegisterValues]
  mov edx,dword ptr [esi+TBESENCodeContext.Code]
  mov ebx,dword ptr [edx+TBESENCode.Body]
  mov ebx,dword ptr [ebx+TBESENASTNodeFunctionBody.IsFunction]
  test ebx,ebx
  jz @IsNoFunction
   mov ebx,dword ptr [esi+TBESENCodeContext.Context]
   mov ebx,dword ptr [ebx+TBESENContext.VariableEnvironment]
   mov ebx,dword ptr [ebx+TBESENLexicalEnvironment.EnvironmentRecord]
   mov ebx,dword ptr [ebx+TBESENDeclarativeEnvironmentRecord.HashValues]
  @IsNoFunction:
  mov ebp,dword ptr [edx+TBESENCode.NativeCodePCOffsets]
  mov edx,dword ptr [esi+TBESENCodeContext.PC]
  call dword ptr [ebp+edx*4]
  mov eax,dword ptr [esi+TBESENCodeContext.BlockRunning]
 pop edi
 pop esi
 pop ebx
 pop ebp
end;

end.
