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
unit BESENDecompiler;
{$i BESEN.inc}

interface

uses SysUtils,Math,BESENConstants,BESENTypes,BESENValue,BESENBaseObject,BESENASTNodes,
     BESENEvalCacheItem;

type TBESENDecompiler=class(TBESENBaseObject)
      public
       constructor Create(AInstance:TObject); overload; override;
       destructor Destroy; override;
       function Decompile(RootNode:TBESENASTNode):TBESENUTF8STRING;
     end;

implementation

uses BESEN,BESENUtils,BESENPointerList,BESENHashMap,BESENErrors,BESENNumberUtils,
     BESENCode,BESENCodeGeneratorContext,BESENOpcodes,BESENHashUtils,BESENGlobals,
     BESENParser,BESENStringUtils;

constructor TBESENDecompiler.Create(AInstance:TObject);
begin
 inherited Create(AInstance);
end;

destructor TBESENDecompiler.Destroy;
begin
 inherited Destroy;
end;

function TBESENDecompiler.Decompile(RootNode:TBESENASTNode):TBESENUTF8STRING;
var Code,s:TBESENUTF8STRING;
    Indent:integer;
 procedure Add(s:TBESENUTF8STRING);
 begin
  if (((length(Code)>0) and (Code[length(Code)] in [' ',#13,#10])) or (length(Code)=0)) and ((length(s)>0) and (s[1]=' ')) then begin
   delete(s,1,1);
  end;
  Code:=Code+s;
 end;
 procedure AddCRLF(s:TBESENUTF8STRING);
 begin
  Add(s);
  Add(#13#10);
 end;
 procedure AddIndent;
 var i:integer;
 begin
  if (length(Code)>0) and not (Code[length(Code)] in [' ',#13,#10]) then begin
   Code:=Code+#13#10;
  end;
  for i:=1 to Indent do begin
   Code:=Code+#9;
  end;
 end;
 procedure Visit(ToVisit:TBESENASTNode;NeedParens:boolean=true);
 var Counter:integer;
     First:boolean;
 begin
  if assigned(ToVisit) then begin
   case ToVisit.NodeType of
    bntNONE:begin
    end;
    bntEXPRESSION:begin
    end;
    bntLITERAL:begin
    end;
    bntIDENTIFIER:begin
     Add(BESENUTF16ToUTF8(TBESENASTNodeIdentifier(ToVisit).Name));
    end;
    bntVARIABLEDECLARATION:begin
     Visit(TBESENASTNodeVariableDeclaration(ToVisit).Identifier);
     if assigned(TBESENASTNodeVariableDeclaration(ToVisit).Expression) then begin
      Add(' = ');
      Visit(TBESENASTNodeVariableDeclaration(ToVisit).Expression,false);
     end;
    end;
    bntVARIABLEEXPRESSION:begin
     First:=true;
     Add('var ');
     First:=true;
     for Counter:=0 to length(TBESENASTNodeVariableExpression(ToVisit).Declarations)-1 do begin
      if First then begin
       First:=false;
      end else begin
       Add(', ');
      end;
      Visit(TBESENASTNodeVariableExpression(ToVisit).Declarations[Counter]);
     end;
    end;
    bntFUNCTIONBODY:begin
     if length(TBESENASTNodeFunctionBody(ToVisit).Variables)>0 then begin
      First:=true;
      for Counter:=0 to length(TBESENASTNodeFunctionBody(ToVisit).Variables)-1 do begin
       if not (TBESENASTNodeFunctionBody(ToVisit).Variables[Counter].IsParameter or TBESENASTNodeFunctionBody(ToVisit).Variables[Counter].IsReached) then begin
        if First then begin
         AddIndent;
         Add('var ');
         First:=false;
        end else begin
         Add(', ');
        end;
        Visit(TBESENASTNodeFunctionBody(ToVisit).Variables[Counter]);
       end;
      end;
      if not First then begin
       Add(';');
      end;
     end;
     for Counter:=0 to length(TBESENASTNodeFunctionBody(ToVisit).Functions)-1 do begin
      Visit(TBESENASTNodeFunctionBody(ToVisit).Functions[Counter]);
     end;
     for Counter:=0 to length(TBESENASTNodeFunctionBody(ToVisit).Statements)-1 do begin
      Visit(TBESENASTNodeFunctionBody(ToVisit).Statements[Counter]);
     end;
    end;
    bntFUNCTIONLITERAL:begin
     if assigned(TBESENASTNodeFunctionLiteral(ToVisit).Name) then begin
      AddIndent;
      Add('function ');
      Visit(TBESENASTNodeFunctionLiteral(ToVisit).Name);
     end else begin
      Add('function');
     end;
     Add('(');
     for Counter:=0 to length(TBESENASTNodeFunctionLiteral(ToVisit).Body.Parameters)-1 do begin
      if Counter>0 then begin
       Add(', ');
      end;
      Visit(TBESENASTNodeFunctionLiteral(ToVisit).Body.Parameters[Counter]);
     end;
     AddCRLF(') {');
     inc(Indent);
     Visit(TBESENASTNodeFunctionLiteral(ToVisit).Body);
     dec(Indent);
     AddIndent;
     Add('}');
    end;
    bntSTATEMENT:begin
     Add(';');
    end;
    bntVARIABLESTATEMENT:begin
     AddIndent;
     Add('var ');
     for Counter:=0 to length(TBESENASTNodeVariableStatement(ToVisit).Declarations)-1 do begin
      if Counter>0 then begin
       Add(', ');
      end;
      Visit(TBESENASTNodeVariableStatement(ToVisit).Declarations[Counter]);
     end;
     Add(';');
    end;
    bntFUNCTIONDECLARATION:begin
     if assigned(TBESENASTNodeFunctionDeclaration(ToVisit).Container) then begin
      AddIndent;
      Visit(TBESENASTNodeFunctionDeclaration(ToVisit).Container.Literal);
      Add(';');
     end;
    end;
    bntEXPRESSIONSTATEMENT:begin
     AddIndent;
     Visit(TBESENASTNodeExpressionStatement(ToVisit).Expression,false);
     Add(';');
    end;
    bntEMPTYSTATEMENT:begin
    end;
    bntBLOCKSTATEMENT:begin
     AddIndent;
     Add('{');
     inc(Indent);
     for Counter:=0 to length(TBESENASTNodeBlockStatement(ToVisit).Statements)-1 do begin
      Visit(TBESENASTNodeBlockStatement(ToVisit).Statements[Counter]);
     end;
     dec(Indent);
     AddIndent;
     Add('}');
    end;
    bntDEBUGGERSTATEMENT:begin
     AddIndent;
     Add('debugger;');
    end;
    bntBREAKSTATEMENT:begin
     AddIndent;
     if assigned(TBESENASTNodeBreakStatement(ToVisit).Identifier) then begin
      Add('break ');
      Visit(TBESENASTNodeBreakStatement(ToVisit).Identifier);
      AddCRLF(';');
     end else begin
      Add('break;');
     end;
    end;
    bntCONTINUESTATEMENT:begin
     AddIndent;
     if assigned(TBESENASTNodeContinueStatement(ToVisit).Identifier) then begin
      Add('continue ');
      Visit(TBESENASTNodeContinueStatement(ToVisit).Identifier);
      AddCRLF(';');
     end else begin
      Add('continue;');
     end;
    end;
    bntDOSTATEMENT:begin
     AddIndent;
     Add('do');
     Visit(TBESENASTNodeDoStatement(ToVisit).Statement);
     Add('while(');
     Visit(TBESENASTNodeDoStatement(ToVisit).Expression,false);
     AddCRLF(');');
    end;
    bntWHILESTATEMENT:begin
     AddIndent;
     Add('while(');
     Visit(TBESENASTNodeWhileStatement(ToVisit).Expression,false);
     Add(')');
     Visit(TBESENASTNodeWhileStatement(ToVisit).Statement);
    end;
    bntWITHSTATEMENT:begin
     AddIndent;
     Add('with(');
     Visit(TBESENASTNodeWithStatement(ToVisit).Expression,false);
     Add(')');
     Visit(TBESENASTNodeWithStatement(ToVisit).Statement);
    end;
    bntFORSTATEMENT:begin
     AddIndent;
     Add('for(');
     Visit(TBESENASTNodeForStatement(ToVisit).Initial);
     Add(';');
     Visit(TBESENASTNodeForStatement(ToVisit).Condition);
     Add(';');
     Visit(TBESENASTNodeForStatement(ToVisit).Increment);
     Add(')');
     Visit(TBESENASTNodeForStatement(ToVisit).Statement);
    end;
    bntFORINSTATEMENT:begin
     AddIndent;
     Add('for(');
     if assigned(TBESENASTNodeForInStatement(ToVisit).Variable) and (TBESENASTNodeForInStatement(ToVisit).Variable.NodeType=bntVARIABLEDECLARATION) then begin
      Add('var ');
     end;
     Visit(TBESENASTNodeForInStatement(ToVisit).Variable);
     Add(' in ');
     Visit(TBESENASTNodeForInStatement(ToVisit).Expression);
     Add(')');
     Visit(TBESENASTNodeForInStatement(ToVisit).Statement);
    end;
    bntIFSTATEMENT:begin
     AddIndent;
     Add('if(');
     Visit(TBESENASTNodeIfStatement(ToVisit).Expression,false);
     AddCRLF(')');
     inc(Indent);
     Visit(TBESENASTNodeIfStatement(ToVisit).TrueStatement);
     dec(Indent);
     if assigned(TBESENASTNodeIfStatement(ToVisit).FalseStatement) then begin
      AddCRLF('');
      AddIndent;
      Add('else');
      inc(Indent);
      Visit(TBESENASTNodeIfStatement(ToVisit).FalseStatement);
      dec(Indent);
      AddCRLF('');
     end;
    end;
    bntLABELLEDSTATEMENT:begin
     AddIndent;
     for Counter:=0 to length(TBESENASTNodeLabelledStatement(ToVisit).Identifiers)-1 do begin
      Visit(TBESENASTNodeLabelledStatement(ToVisit).Identifiers[Counter]);
      AddCRLF(': ');
     end;
     Visit(TBESENASTNodeLabelledStatement(ToVisit).Statement);
    end;
    bntCASESTATEMENT:begin
     AddIndent;
     if assigned(TBESENASTNodeCaseStatement(ToVisit).Expression) then begin
      Add('case ');
      Visit(TBESENASTNodeCaseStatement(ToVisit).Expression,false);
     end else begin
      Add('default');
     end;
     AddCRLF(':');
     inc(Indent);
     for Counter:=0 to length(TBESENASTNodeCaseStatement(ToVisit).Statements)-1 do begin
      Visit(TBESENASTNodeCaseStatement(ToVisit).Statements[Counter]);
     end;
     dec(Indent);
    end;
    bntSWITCHSTATEMENT:begin
     AddIndent;
     Add('switch(');
     Visit(TBESENASTNodeSwitchStatement(ToVisit).Expression,false);
     AddCRLF(') {');
     inc(Indent);
     for Counter:=0 to length(TBESENASTNodeSwitchStatement(ToVisit).CaseStatements)-1 do begin
      Visit(TBESENASTNodeSwitchStatement(ToVisit).CaseStatements[Counter]);
     end;
     dec(Indent);
     AddIndent;
     Add('}');
    end;
    bntTHROWSTATEMENT:begin
     AddIndent;
     Add('throw ');
     Visit(TBESENASTNodeThrowStatement(ToVisit).Expression,false);
     AddCRLF(';');
    end;
    bntTRYSTATEMENT:begin
     AddIndent;
     Add('try');
     Visit(TBESENASTNodeTryStatement(ToVisit).TryBlock);
     if assigned(TBESENASTNodeTryStatement(ToVisit).CatchBlock) then begin
      AddIndent;
      Add('catch(');
      Visit(TBESENASTNodeTryStatement(ToVisit).CatchIdentifier,false);
      Add(')');
      Visit(TBESENASTNodeTryStatement(ToVisit).CatchBlock);
     end;
     if assigned(TBESENASTNodeTryStatement(ToVisit).FinallyBlock) then begin
      AddIndent;
      Add('finally');
      Visit(TBESENASTNodeTryStatement(ToVisit).FinallyBlock);
     end;
    end;
    bntARRAYLITERAL:begin
     Add('[');
     for Counter:=0 to length(TBESENASTNodeArrayLiteral(ToVisit).Elements)-1 do begin
      if Counter>0 then begin
       Add(', ');
      end;
      Visit(TBESENASTNodeArrayLiteral(ToVisit).Elements[Counter]);
     end;
     Add(']');
    end;
    bntBINARYEXPRESSION:begin
     Visit(TBESENASTNodeBinaryExpression(ToVisit).LeftExpression);
     Visit(TBESENASTNodeBinaryExpression(ToVisit).RightExpression);
    end;
    bntASSIGNMENTEXPRESSION:begin
     if NeedParens then begin
      Add('(');
     end;
     Visit(TBESENASTNodeAssignmentExpression(ToVisit).LeftExpression);
     Add(' = ');
     Visit(TBESENASTNodeAssignmentExpression(ToVisit).RightExpression);
     if NeedParens then begin
      Add(')');
     end;
    end;
    bntASSIGNMENTOPERATOREXPRESSION:begin
     if NeedParens then begin
      Add('(');
     end;
     Visit(TBESENASTNodeAssignmentOperatorExpression(ToVisit).LeftExpression);
     Visit(TBESENASTNodeAssignmentOperatorExpression(ToVisit).RightExpression);
     if NeedParens then begin
      Add(')');
     end;
    end;
    bntASSIGNMENTMULTIPLYEXPRESSION:begin
     if NeedParens then begin
      Add('(');
     end;
     Visit(TBESENASTNodeAssignmentMultiplyExpression(ToVisit).LeftExpression);
     Add(' *= ');
     Visit(TBESENASTNodeAssignmentMultiplyExpression(ToVisit).RightExpression);
     if NeedParens then begin
      Add(')');
     end;
    end;
    bntASSIGNMENTDIVIDEEXPRESSION:begin
     if NeedParens then begin
      Add('(');
     end;
     Visit(TBESENASTNodeAssignmentDivideExpression(ToVisit).LeftExpression);
     Add(' /= ');
     Visit(TBESENASTNodeAssignmentDivideExpression(ToVisit).RightExpression);
     if NeedParens then begin
      Add(')');
     end;
    end;
    bntASSIGNMENTMODULOEXPRESSION:begin
     if NeedParens then begin
      Add('(');
     end;
     Visit(TBESENASTNodeAssignmentModuloExpression(ToVisit).LeftExpression);
     Add(' %= ');
     Visit(TBESENASTNodeAssignmentModuloExpression(ToVisit).RightExpression);
     if NeedParens then begin
      Add(')');
     end;
    end;
    bntASSIGNMENTPLUSEXPRESSION:begin
     if NeedParens then begin
      Add('(');
     end;
     Visit(TBESENASTNodeAssignmentPlusExpression(ToVisit).LeftExpression);
     Add(' += ');
     Visit(TBESENASTNodeAssignmentPlusExpression(ToVisit).RightExpression);
     if NeedParens then begin
      Add(')');
     end;
    end;
    bntASSIGNMENTMINUSEXPRESSION:begin
     if NeedParens then begin
      Add('(');
     end;
     Visit(TBESENASTNodeAssignmentMinusExpression(ToVisit).LeftExpression);
     Add(' -= ');
     Visit(TBESENASTNodeAssignmentMinusExpression(ToVisit).RightExpression);
     if NeedParens then begin
      Add(')');
     end;
    end;
    bntASSIGNMENTSHIFTLEFTEXPRESSION:begin
     if NeedParens then begin
      Add('(');
     end;
     Visit(TBESENASTNodeAssignmentShiftLeftExpression(ToVisit).LeftExpression);
     Add(' <<= ');
     Visit(TBESENASTNodeAssignmentShiftLeftExpression(ToVisit).RightExpression);
     if NeedParens then begin
      Add(')');
     end;
    end;
    bntASSIGNMENTSHIFTRIGHTEXPRESSION:begin
     if NeedParens then begin
      Add('(');
     end;
     Visit(TBESENASTNodeAssignmentShiftRightExpression(ToVisit).LeftExpression);
     Add(' >>= ');
     Visit(TBESENASTNodeAssignmentShiftRightExpression(ToVisit).RightExpression);
     if NeedParens then begin
      Add(')');
     end;
    end;
    bntASSIGNMENTSHIFTRIGHTUNSIGNEDEXPRESSION:begin
     if NeedParens then begin
      Add('(');
     end;
     Visit(TBESENASTNodeAssignmentShiftRightUnsignedExpression(ToVisit).LeftExpression);
     Add(' >>>= ');
     Visit(TBESENASTNodeAssignmentShiftRightUnsignedExpression(ToVisit).RightExpression);
     if NeedParens then begin
      Add(')');
     end;
    end;
    bntASSIGNMENTBITWISEANDEXPRESSION:begin
     if NeedParens then begin
      Add('(');
     end;
     Visit(TBESENASTNodeAssignmentBitwiseAndExpression(ToVisit).LeftExpression);
     Add(' &= ');
     Visit(TBESENASTNodeAssignmentBitwiseAndExpression(ToVisit).RightExpression);
     if NeedParens then begin
      Add(')');
     end;
    end;
    bntASSIGNMENTBITWISEXOREXPRESSION:begin
     if NeedParens then begin
      Add('(');
     end;
     Visit(TBESENASTNodeAssignmentBitwiseXorExpression(ToVisit).LeftExpression);
     Add(' ^= ');
     Visit(TBESENASTNodeAssignmentBitwiseXorExpression(ToVisit).RightExpression);
     if NeedParens then begin
      Add(')');
     end;
    end;
    bntASSIGNMENTBITWISEOREXPRESSION:begin
     if NeedParens then begin
      Add('(');
     end;
     Visit(TBESENASTNodeAssignmentBitwiseOrExpression(ToVisit).LeftExpression);
     Add(' |= ');
     Visit(TBESENASTNodeAssignmentBitwiseOrExpression(ToVisit).RightExpression);
     if NeedParens then begin
      Add(')');
     end;
    end;
    bntBINARYOPERATOREXPRESSION:begin
     if NeedParens then begin
      Add('(');
     end;
     Visit(TBESENASTNodeBinaryOperatorExpression(ToVisit).LeftExpression);
     Visit(TBESENASTNodeBinaryOperatorExpression(ToVisit).RightExpression);
     if NeedParens then begin
      Add(')');
     end;
    end;
    bntBINARYCOMMAEXPRESSION:begin
     if NeedParens then begin
      Add('(');
     end;
     Visit(TBESENASTNodeBinaryCommaExpression(ToVisit).LeftExpression);
     Add(', ');
     Visit(TBESENASTNodeBinaryCommaExpression(ToVisit).RightExpression);
     if NeedParens then begin
      Add(')');
     end;
    end;
    bntBINARYDIVIDEEXPRESSION:begin
     if NeedParens then begin
      Add('(');
     end;
     Visit(TBESENASTNodeBinaryDivideExpression(ToVisit).LeftExpression);
     Add(' / ');
     Visit(TBESENASTNodeBinaryDivideExpression(ToVisit).RightExpression);
     if NeedParens then begin
      Add(')');
     end;
    end;
    bntBINARYMODULOEXPRESSION:begin
     if NeedParens then begin
      Add('(');
     end;
     Visit(TBESENASTNodeBinaryModuloExpression(ToVisit).LeftExpression);
     Add(' % ');
     Visit(TBESENASTNodeBinaryModuloExpression(ToVisit).RightExpression);
     if NeedParens then begin
      Add(')');
     end;
    end;
    bntBINARYMULTIPLYEXPRESSION:begin
     if NeedParens then begin
      Add('(');
     end;
     Visit(TBESENASTNodeBinaryMultiplyExpression(ToVisit).LeftExpression);
     Add(' * ');
     Visit(TBESENASTNodeBinaryMultiplyExpression(ToVisit).RightExpression);
     if NeedParens then begin
      Add(')');
     end;
    end;
    bntBINARYPLUSEXPRESSION:begin
     if NeedParens then begin
      Add('(');
     end;
     Visit(TBESENASTNodeBinaryPlusExpression(ToVisit).LeftExpression);
     Add(' + ');
     Visit(TBESENASTNodeBinaryPlusExpression(ToVisit).RightExpression);
     if NeedParens then begin
      Add(')');
     end;
    end;
    bntBINARYMINUSEXPRESSION:begin
     if NeedParens then begin
      Add('(');
     end;
     Visit(TBESENASTNodeBinaryMinusExpression(ToVisit).LeftExpression);
     Add(' - ');
     Visit(TBESENASTNodeBinaryMinusExpression(ToVisit).RightExpression);
     if NeedParens then begin
      Add(')');
     end;
    end;
    bntBINARYSHIFTLEFTEXPRESSION:begin
     if NeedParens then begin
      Add('(');
     end;
     Visit(TBESENASTNodeBinaryShiftLeftExpression(ToVisit).LeftExpression);
     Add(' << ');
     Visit(TBESENASTNodeBinaryShiftLeftExpression(ToVisit).RightExpression);
     if NeedParens then begin
      Add(')');
     end;
    end;
    bntBINARYSHIFTRIGHTEXPRESSION:begin
     if NeedParens then begin
      Add('(');
     end;
     Visit(TBESENASTNodeBinaryShiftRightExpression(ToVisit).LeftExpression);
     Add(' >> ');
     Visit(TBESENASTNodeBinaryShiftRightExpression(ToVisit).RightExpression);
     if NeedParens then begin
      Add(')');
     end;
    end;
    bntBINARYSHIFTRIGHTUNSIGNEDEXPRESSION:begin
     if NeedParens then begin
      Add('(');
     end;
     Visit(TBESENASTNodeBinaryShiftRightUnsignedExpression(ToVisit).LeftExpression);
     Add(' >>> ');
     Visit(TBESENASTNodeBinaryShiftRightUnsignedExpression(ToVisit).RightExpression);
     if NeedParens then begin
      Add(')');
     end;
    end;
    bntBINARYGREATERTHANEXPRESSION:begin
     if NeedParens then begin
      Add('(');
     end;
     Visit(TBESENASTNodeBinaryGreaterThanExpression(ToVisit).LeftExpression);
     Add(' > ');
     Visit(TBESENASTNodeBinaryGreaterThanExpression(ToVisit).RightExpression);
     if NeedParens then begin
      Add(')');
     end;
    end;
    bntBINARYGREATERTHANOREQUALEXPRESSION:begin
     if NeedParens then begin
      Add('(');
     end;
     Visit(TBESENASTNodeBinaryGreaterThanOrEqualExpression(ToVisit).LeftExpression);
     Add(' >= ');
     Visit(TBESENASTNodeBinaryGreaterThanOrEqualExpression(ToVisit).RightExpression);
     if NeedParens then begin
      Add(')');
     end;
    end;
    bntBINARYLESSTHANEXPRESSION:begin
     if NeedParens then begin
      Add('(');
     end;
     Visit(TBESENASTNodeBinaryLessThanExpression(ToVisit).LeftExpression);
     Add(' < ');
     Visit(TBESENASTNodeBinaryLessThanExpression(ToVisit).RightExpression);
     if NeedParens then begin
      Add(')');
     end;
    end;
    bntBINARYLESSTHANOREQUALEXPRESSION:begin
     if NeedParens then begin
      Add('(');
     end;
     Visit(TBESENASTNodeBinaryLessThanOrEqualExpression(ToVisit).LeftExpression);
     Add(' <= ');
     Visit(TBESENASTNodeBinaryLessThanOrEqualExpression(ToVisit).RightExpression);
     if NeedParens then begin
      Add(')');
     end;
    end;
    bntBINARYINSTANCEOFEXPRESSION:begin
     if NeedParens then begin
      Add('(');
     end;
     Visit(TBESENASTNodeBinaryInstanceOfExpression(ToVisit).LeftExpression);
     Add(' instanceof ');
     Visit(TBESENASTNodeBinaryInstanceOfExpression(ToVisit).RightExpression);
     if NeedParens then begin
      Add(')');
     end;
    end;
    bntBINARYINEXPRESSION:begin
     if NeedParens then begin
      Add('(');
     end;
     Visit(TBESENASTNodeBinaryInExpression(ToVisit).LeftExpression);
     Add(' in ');
     Visit(TBESENASTNodeBinaryInExpression(ToVisit).RightExpression);
     if NeedParens then begin
      Add(')');
     end;
    end;
    bntBINARYEQUALEQUALEXPRESSION:begin
     if NeedParens then begin
      Add('(');
     end;
     Visit(TBESENASTNodeBinaryEqualEqualExpression(ToVisit).LeftExpression);
     Add(' == ');
     Visit(TBESENASTNodeBinaryEqualEqualExpression(ToVisit).RightExpression);
     if NeedParens then begin
      Add(')');
     end;
    end;
    bntBINARYEQUALEQUALEQUALEXPRESSION:begin
     if NeedParens then begin
      Add('(');
     end;
     Visit(TBESENASTNodeBinaryEqualEqualEqualExpression(ToVisit).LeftExpression);
     Add(' === ');
     Visit(TBESENASTNodeBinaryEqualEqualEqualExpression(ToVisit).RightExpression);
     if NeedParens then begin
      Add(')');
     end;
    end;
    bntBINARYNOTEQUALEXPRESSION:begin
     if NeedParens then begin
      Add('(');
     end;
     Visit(TBESENASTNodeBinaryNotEqualExpression(ToVisit).LeftExpression);
     Add(' != ');
     Visit(TBESENASTNodeBinaryNotEqualExpression(ToVisit).RightExpression);
     if NeedParens then begin
      Add(')');
     end;
    end;
    bntBINARYNOTEQUALEQUALEXPRESSION:begin
     if NeedParens then begin
      Add('(');
     end;
     Visit(TBESENASTNodeBinaryNotEqualEqualExpression(ToVisit).LeftExpression);
     Add(' !== ');
     Visit(TBESENASTNodeBinaryNotEqualEqualExpression(ToVisit).RightExpression);
     if NeedParens then begin
      Add(')');
     end;
    end;
    bntBINARYBITWISEANDEXPRESSION:begin
     if NeedParens then begin
      Add('(');
     end;
     Visit(TBESENASTNodeBinaryBitwiseAndExpression(ToVisit).LeftExpression);
     Add(' & ');
     Visit(TBESENASTNodeBinaryBitwiseAndExpression(ToVisit).RightExpression);
     if NeedParens then begin
      Add(')');
     end;
    end;
    bntBINARYBITWISEXOREXPRESSION:begin
     if NeedParens then begin
      Add('(');
     end;
     Visit(TBESENASTNodeBinaryBitwiseXorExpression(ToVisit).LeftExpression);
     Add(' ^ ');
     Visit(TBESENASTNodeBinaryBitwiseXorExpression(ToVisit).RightExpression);
     if NeedParens then begin
      Add(')');
     end;
    end;
    bntBINARYBITWISEOREXPRESSION:begin
     if NeedParens then begin
      Add('(');
     end;
     Visit(TBESENASTNodeBinaryBitwiseOrExpression(ToVisit).LeftExpression);
     Add(' | ');
     Visit(TBESENASTNodeBinaryBitwiseOrExpression(ToVisit).RightExpression);
     if NeedParens then begin
      Add(')');
     end;
    end;
    bntBOOLEANLITERAL:begin
     if TBESENASTNodeBooleanLiteral(ToVisit).Value then begin
      Add('true');
     end else begin
      Add('false');
     end;
    end;
    bntCALLEXPRESSION:begin
     Visit(TBESENASTNodeCallExpression(ToVisit).TheFunction);
     Add('(');
     for Counter:=0 to length(TBESENASTNodeCallExpression(ToVisit).Arguments)-1 do begin
      if Counter>0 then begin
       Add(', ');
      end;
      Visit(TBESENASTNodeCallExpression(ToVisit).Arguments[Counter]);
     end;
     Add(')');
    end;
    bntNEWEXPRESSION:begin
     Add('new ');
     Visit(TBESENASTNodeNewExpression(ToVisit).TheFunction);
     Add('(');
     for Counter:=0 to length(TBESENASTNodeNewExpression(ToVisit).Arguments)-1 do begin
      if Counter>0 then begin
       Add(', ');
      end;
      Visit(TBESENASTNodeNewExpression(ToVisit).Arguments[Counter]);
     end;
     Add(')');
    end;
    bntCONDITIONALEXPRESSION:begin
     if NeedParens then begin
      Add('(');
     end;
     Visit(TBESENASTNodeConditionalExpression(ToVisit).Expression);
     Add(' ? ');
     Visit(TBESENASTNodeConditionalExpression(ToVisit).TrueExpression);
     Add(' : ');
     Visit(TBESENASTNodeConditionalExpression(ToVisit).FalseExpression);
     if NeedParens then begin
      Add(')');
     end;
    end;
    bntUNARYEXPRESSION:begin
     Visit(TBESENASTNodeUnaryExpression(ToVisit).SubExpression);
    end;
    bntUNARYOPERATOREXPRESSION:begin
     Visit(TBESENASTNodeUnaryOperatorExpression(ToVisit).SubExpression);
    end;
    bntUNARYPLUSEXPRESSION:begin
     if NeedParens then begin
      Add('(');
     end;
     Add('+');
     Visit(TBESENASTNodeUnaryPlusExpression(ToVisit).SubExpression);
     if NeedParens then begin
      Add(')');
     end;
    end;
    bntUNARYMINUSEXPRESSION:begin
     if NeedParens then begin
      Add('(');
     end;
     Add('-');
     Visit(TBESENASTNodeUnaryMinusExpression(ToVisit).SubExpression);
     if NeedParens then begin
      Add(')');
     end;
    end;
    bntUNARYBITWISENOTEXPRESSION:begin
     if NeedParens then begin
      Add('(');
     end;
     Add('~');
     Visit(TBESENASTNodeUnaryBitwiseNotExpression(ToVisit).SubExpression);
     if NeedParens then begin
      Add(')');
     end;
    end;
    bntUNARYLOGICALNOTEXPRESSION:begin
     if NeedParens then begin
      Add('(');
     end;
     Add('!');
     Visit(TBESENASTNodeUnaryLogicalNotExpression(ToVisit).SubExpression);
     if NeedParens then begin
      Add(')');
     end;
    end;
    bntUNARYVOIDEXPRESSION:begin
     Add('void');
     Visit(TBESENASTNodeUnaryVoidExpression(ToVisit).SubExpression);
    end;
    bntUNARYTYPEOFEXPRESSION:begin
     Add('typeof');
     Visit(TBESENASTNodeUnaryTypeOfExpression(ToVisit).SubExpression);
    end;
    bntPROPERTYEXPRESSION:begin
     Visit(TBESENASTNodePropertyExpression(ToVisit).LeftExpression);
     if TBESENASTNodePropertyExpression(ToVisit).Dot and assigned(TBESENASTNodePropertyExpression(ToVisit).RightExpression) and (TBESENASTNodePropertyExpression(ToVisit).RightExpression is TBESENASTNodeStringLiteral) then begin
      Add('.');
      Add(TBESENASTNodeStringLiteral(TBESENASTNodePropertyExpression(ToVisit).RightExpression).Value);
     end else begin
      Add('[');
      Visit(TBESENASTNodePropertyExpression(ToVisit).RightExpression);
      Add(']');
     end;
    end;
    bntLOGICALANDEXPRESSION:begin
     if NeedParens then begin
      Add('(');
     end;
     Visit(TBESENASTNodeLogicalAndExpression(ToVisit).LeftExpression);
     Add(' && ');
     Visit(TBESENASTNodeLogicalAndExpression(ToVisit).RightExpression);
     if NeedParens then begin
      Add(')');
     end;
    end;
    bntLOGICALOREXPRESSION:begin
     if NeedParens then begin
      Add('(');
     end;
     Visit(TBESENASTNodeLogicalOrExpression(ToVisit).LeftExpression);
     Add(' || ');
     Visit(TBESENASTNodeLogicalOrExpression(ToVisit).RightExpression);
     if NeedParens then begin
      Add(')');
     end;
    end;
    bntDELETEEXPRESSION:begin
     Add('delete ');
     Visit(TBESENASTNodeDeleteExpression(ToVisit).SubExpression);
    end;
    bntPOSTFIXINCEXPRESSION:begin
     if NeedParens then begin
      Add('(');
     end;
     Visit(TBESENASTNodePostfixIncExpression(ToVisit).SubExpression);
     Add('++');
     if NeedParens then begin
      Add(')');
     end;
    end;
    bntPOSTFIXDECEXPRESSION:begin
     if NeedParens then begin
      Add('(');
     end;
     Visit(TBESENASTNodePostfixDecExpression(ToVisit).SubExpression);
     Add('--');
     if NeedParens then begin
      Add(')');
     end;
    end;
    bntPREFIXINCEXPRESSION:begin
     if NeedParens then begin
      Add('(');
     end;
     Add('++');
     Visit(TBESENASTNodePrefixIncExpression(ToVisit).SubExpression);
     if NeedParens then begin
      Add(')');
     end;
    end;
    bntPREFIXDECEXPRESSION:begin
     if NeedParens then begin
      Add('(');
     end;
     Add('--');
     Visit(TBESENASTNodePrefixDecExpression(ToVisit).SubExpression);
     if NeedParens then begin
      Add(')');
     end;
    end;
    bntNULLLITERAL:begin
     Add('null');
    end;
    bntNUMBERLITERAL:begin
     if TBESENASTNodeNumberLiteral(ToVisit).Value=trunc(TBESENASTNodeNumberLiteral(ToVisit).Value) then begin
      str(trunc(TBESENASTNodeNumberLiteral(ToVisit).Value),s);
     end else begin
      str(TBESENASTNodeNumberLiteral(ToVisit).Value,s);
     end;
     Add(s);
    end;
    bntREGEXPLITERAL:begin
     Add('/'+BESENUTF16ToUTF8(TBESENASTNodeRegExpLiteral(ToVisit).Source)+'/'+BESENUTF16ToUTF8(TBESENASTNodeRegExpLiteral(ToVisit).Flags));
    end;
    bntSTRINGLITERAL:begin
     Add(BESENUTF16ToUTF8(BESENJSONStringQuote(TBESENASTNodeStringLiteral(ToVisit).Value)));
    end;
    bntTHISLITERAL:begin
     Add('this');
    end;
    bntOBJECTLITERALPROPERTY:begin
     case TBESENASTNodeObjectLiteralProperty(ToVisit).PropertyType of
      banolptACCESSOR:begin
       case TBESENASTNodeObjectLiteralProperty(ToVisit).PropertyAccessorType of
        banolpatGET:begin
         Add('get ');
        end;
        banolpatSET:begin
         Add('set ');
        end;
       end;
       Add(TBESENASTNodeObjectLiteralProperty(ToVisit).Name);
       if assigned(TBESENASTNodeObjectLiteralProperty(ToVisit).Container) then begin
        Visit(TBESENASTNodeObjectLiteralProperty(ToVisit).Container.Literal);
       end;
      end;
      banolptDATA:begin
       Add(TBESENASTNodeObjectLiteralProperty(ToVisit).Name);
       Add(': ');
       Visit(TBESENASTNodeObjectLiteralProperty(ToVisit).Value);
      end;
     end;
    end;
    bntOBJECTLITERAL:begin
     Add('{');
     inc(Indent);
     for Counter:=0 to length(TBESENASTNodeObjectLiteral(ToVisit).Properties)-1 do begin
      if Counter>0 then begin
       Add(', ');
      end;
      Visit(TBESENASTNodeObjectLiteral(ToVisit).Properties[Counter]);
     end;
     dec(Indent);
     Add('}');
    end;
    bntRETURNSTATEMENT:begin
     AddIndent;
     Add('return ');
     Visit(TBESENASTNodeReturnStatement(ToVisit).Expression,false);
     AddCRLF(';');
    end;
    bntPROGRAM:begin
     Visit(TBESENASTNodeProgram(ToVisit).Body);
    end;
    bntFUNCTIONEXPRESSION:begin
     if assigned(TBESENASTNodeFunctionDeclaration(ToVisit).Container) then begin
      AddIndent;
      Visit(TBESENASTNodeFunctionExpression(ToVisit).Container.Literal);
     end;
    end;
   end;
  end;
 end;
begin
 Code:='';
 Indent:=0;
 Visit(RootNode);
 result:=Code;
end;

end.
