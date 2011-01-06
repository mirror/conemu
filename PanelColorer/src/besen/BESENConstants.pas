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
unit BESENConstants;
{$i BESEN.inc}

interface

uses Math;

const BESENFPUExceptionMask:TFPUExceptionMask=[exInvalidOp,exDenormalized,exZeroDivide,exOverflow,exUnderflow,exPrecision];

      BESENFPUPrecisionMode:TFPUPrecisionMode={$ifdef HAS_TYPE_EXTENDED}pmEXTENDED{$else}pmDOUBLE{$endif};

{$ifndef fpc}
      FPC_VERSION=2;
      FPC_RELEASE=5;
      FPC_PATCH=1;
{$endif}

      COMPAT_UTF8_UNSAFE=1 shl 1; // accept 'valid but insecure' UTF8
      COMPAT_SGMLCOM=1 shl 2; // treat '<!--' as a '//' comment
      COMPAT_BESEN=1 shl 3; // BESEN-specific extensions
      COMPAT_JS=1 shl 4; // JavaScript-specific extensions

      BESEN_GC_SWEEPCOUNT:integer=128;
      BESEN_GC_MARKCOUNT:integer=128;
      BESEN_GC_TRIGGERCOUNT_PER_COLLECT:integer=1;

      bimMAXIDENTS=1 shl 24;

      BESENHashItemsPerBucketsThreshold:longword=5;
      BESENHashMaxSize:longword=1 shl 16;

      BESENMaxCountOfFreeCodeContexts:integer=16;

      BESENMaxCountOfFreeContexts:integer=16;

      BESEN_JIT_LOOPCOMPILETHRESHOLD:longword=1000;

      BESEN_CMWCRND_A=18782;
      BESEN_CMWCRND_M=longword($fffffffe);
      BESEN_CMWCRND_BITS=12;
      BESEN_CMWCRND_SIZE=1 shl BESEN_CMWCRND_BITS;
      BESEN_CMWCRND_MASK=BESEN_CMWCRND_SIZE-1;

      bncmmMINBLOCKCONTAINERSIZE=1048576;

      bcttNOTARGET=0;

      BESENEvalCacheSize:integer=256;
      BESENEvalCacheMaxSourceLength:integer=256;

      bncfZERO=1 shl 0;
      bncfNAN=1 shl 1;
      bncfINFINITE=1 shl 2;
      bncfNEGATIVE=1 shl 3;

      BESENObjectStructureIDManagerHashSize=65536;
      BESENObjectStructureIDManagerHashSizeMask=BESENObjectStructureIDManagerHashSize-1;

      BESENPolymorphicInlineCacheSize=8;
      BESENPolymorphicInlineCacheSizeMask=BESENPolymorphicInlineCacheSize-1;

      BESENPolymorphicInlineCacheStartItemPositions=longword((0 shl 0) or (1 shl 4) or (2 shl 8) or (3 shl 12) or (4 shl 16) or (5 shl 20) or (6 shl 24) or (7 shl 28));

      BESENLongBooleanValues:array[boolean] of longbool=(false,true);

      BESENNumberBooleanValues:array[boolean] of integer=(0,1);

implementation

end.
