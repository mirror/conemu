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
unit BESENObjectPropertyDescriptor;
{$i BESEN.inc}

interface

uses BESENConstants,BESENTypes,BESENValue;

type TBESENObjectPropertyDescriptorAttribute=(bopaWRITABLE,bopaENUMERABLE,bopaCONFIGURABLE);

     TBESENObjectPropertyDescriptorAttributes=set of TBESENObjectPropertyDescriptorAttribute;

     TBESENObjectPropertyDescriptorPresent=(boppVALUE,boppGETTER,boppSETTER,boppWRITABLE,boppENUMERABLE,boppCONFIGURABLE,boppPROTO);

     TBESENObjectPropertyDescriptorPresents=set of TBESENObjectPropertyDescriptorPresent;

     PBESENObjectPropertyDescriptor=^TBESENObjectPropertyDescriptor;
     TBESENObjectPropertyDescriptor=record
      Value:TBESENValue;
      Getter:TObject;
      Setter:TObject;
      Attributes:TBESENObjectPropertyDescriptorAttributes;
      Presents:TBESENObjectPropertyDescriptorPresents;
     end;

     TBESENObjectPropertyDescriptors=array of TBESENObjectPropertyDescriptor;

var BESENUndefinedPropertyDescriptor:TBESENObjectPropertyDescriptor;

function BESENAccessorPropertyDescriptor(const Getter,Setter:TObject;const Attributes:TBESENObjectPropertyDescriptorAttributes):TBESENObjectPropertyDescriptor; {$ifdef caninline}inline;{$endif}
function BESENDataPropertyDescriptor(const Value:TBESENValue;const Attributes:TBESENObjectPropertyDescriptorAttributes):TBESENObjectPropertyDescriptor; {$ifdef caninline}inline;{$endif}
function BESENPropertyDescriptor(const Value:TBESENValue;const Getter,Setter:TObject;const Attributes:TBESENObjectPropertyDescriptorAttributes):TBESENObjectPropertyDescriptor; {$ifdef caninline}inline;{$endif}

function BESENIsUndefinedDescriptor(const Descriptor:TBESENObjectPropertyDescriptor):boolean; {$ifdef caninline}inline;{$endif}
function BESENIsAccessorDescriptor(const Descriptor:TBESENObjectPropertyDescriptor):boolean; {$ifdef caninline}inline;{$endif}
function BESENIsDataDescriptor(const Descriptor:TBESENObjectPropertyDescriptor):boolean; {$ifdef caninline}inline;{$endif}
function BESENIsGenericDescriptor(const Descriptor:TBESENObjectPropertyDescriptor):boolean; {$ifdef caninline}inline;{$endif}
function BESENIsInconsistentDescriptor(const Descriptor:TBESENObjectPropertyDescriptor):boolean; {$ifdef caninline}inline;{$endif}

implementation

uses BESEN;

function BESENAccessorPropertyDescriptor(const Getter,Setter:TObject;const Attributes:TBESENObjectPropertyDescriptorAttributes):TBESENObjectPropertyDescriptor; {$ifdef caninline}inline;{$endif}
begin
 result.Value.ValueType:=bvtUNDEFINED;
 result.Getter:=Getter;
 result.Setter:=Setter;
 result.Attributes:=Attributes*[bopaENUMERABLE,bopaCONFIGURABLE];
 result.Presents:=[boppENUMERABLE,boppCONFIGURABLE];
 if assigned(result.Getter) then begin
  result.Presents:=result.Presents+[boppGETTER];
 end;
 if assigned(result.Setter) then begin
  result.Presents:=result.Presents+[boppSETTER];
 end;
end;

function BESENDataPropertyDescriptor(const Value:TBESENValue;const Attributes:TBESENObjectPropertyDescriptorAttributes):TBESENObjectPropertyDescriptor; {$ifdef caninline}inline;{$endif}
begin
 BESENCopyValue(result.Value,Value);
 result.Getter:=nil;
 result.Setter:=nil;
 result.Attributes:=Attributes;
 result.Presents:=[boppVALUE,boppWRITABLE,boppENUMERABLE,boppCONFIGURABLE];
end;

function BESENPropertyDescriptor(const Value:TBESENValue;const Getter,Setter:TObject;const Attributes:TBESENObjectPropertyDescriptorAttributes):TBESENObjectPropertyDescriptor; {$ifdef caninline}inline;{$endif}
begin
 BESENCopyValue(result.Value,Value);
 result.Getter:=Getter;
 result.Setter:=Setter;
 result.Attributes:=Attributes;
 result.Presents:=[boppVALUE,boppGETTER,boppSETTER,boppWRITABLE,boppENUMERABLE,boppCONFIGURABLE];
end;

function BESENIsUndefinedDescriptor(const Descriptor:TBESENObjectPropertyDescriptor):boolean; {$ifdef caninline}inline;{$endif}
begin
 result:=Descriptor.Presents=[];
end;

function BESENIsAccessorDescriptor(const Descriptor:TBESENObjectPropertyDescriptor):boolean; {$ifdef caninline}inline;{$endif}
begin
 result:=([boppGETTER,boppSETTER]*Descriptor.Presents)<>[];
end;

function BESENIsDataDescriptor(const Descriptor:TBESENObjectPropertyDescriptor):boolean; {$ifdef caninline}inline;{$endif}
begin
 result:=([boppVALUE,boppWRITABLE]*Descriptor.Presents)<>[];
end;

function BESENIsGenericDescriptor(const Descriptor:TBESENObjectPropertyDescriptor):boolean; {$ifdef caninline}inline;{$endif}
begin
 result:=not (BESENIsAccessorDescriptor(Descriptor) or BESENIsDataDescriptor(Descriptor));
end;

function BESENIsInconsistentDescriptor(const Descriptor:TBESENObjectPropertyDescriptor):boolean; {$ifdef caninline}inline;{$endif}
begin
 result:=(([boppGETTER,boppSETTER]*Descriptor.Presents)<>[]) and (([boppVALUE,boppWRITABLE]*Descriptor.Presents)<>[]);
end;

procedure InitBESEN;
begin
 fillchar(BESENUndefinedPropertyDescriptor,sizeof(TBESENObjectPropertyDescriptor),#0);
 BESENUndefinedPropertyDescriptor.Presents:=[];
end;

procedure DoneBESEN;
begin
end;

initialization
 InitBESEN;
finalization
 DoneBESEN;
end.

