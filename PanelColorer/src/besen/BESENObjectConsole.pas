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
unit BESENObjectConsole;
{$i BESEN.inc}

interface

const BESENObjectConsoleSource:widestring=
'/**'+#13#10+
'* Console object for BESEN'+#13#10+
'* @author Dmitry A. Soshnikov <dmitry.soshnikov@gmail.com>'+#13#10+
'*/'+#13#10+
'(function initConsole(global) {'+#13#10+
''+#13#10+
'  // helpers'+#13#10+
''+#13#10+
'  var getClass = Object.prototype.toString;'+#13#10+
'  var timeMap = {};'+#13#10+
''+#13#10+
'  function repeatSring(string, times) {'+#13#10+
'    return Array(times + 1).join(string);'+#13#10+
'  }'+#13#10+
''+#13#10+
'  function dir(object, deep, level) {'+#13#10+
'    level || (level = 1);'+#13#10+
'    typeof deep == "undefined" && (deep = true);'+#13#10+
'    var openBracket, closeBracket;'+#13#10+
'    if (getClass.call(object) == "[object Object]") {'+#13#10+
'      openBracket = "{"; closeBracket = "}"'+#13#10+
'    } else if (Array.isArray(object)) {'+#13#10+
'      openBracket = "["; closeBracket = "]"'+#13#10+
'    }'+#13#10+
'    var props = [];'+#13#10+
'    var indent = repeatSring(console.dir.indention, level);'+#13#10+
'    var data, current;'+#13#10+
'    var goDeeper = (typeof deep == "number" ? deep > level : deep);'+#13#10+
'    Object.getOwnPropertyNames(object).forEach(function (property) {'+#13#10+
'      current = object[property];'+#13#10+
'      if (goDeeper && (getClass.call(current) == "[object Object]" || Array.isArray(current))) {'+#13#10+
'        data = dir(current, deep, level + 1);'+#13#10+
'      } else {'+#13#10+
'        data = ('+#13#10+
'          typeof current == "function" ? "function" : ('+#13#10+
'            Array.isArray(current) ? "[" + current + "]" :'+#13#10+
'            current'+#13#10+
'          )'+#13#10+
'        );'+#13#10+
'      }'+#13#10+
'      props.push(indent + property + ": " + data);'+#13#10+
'    });'+#13#10+
'    return "".concat('+#13#10+
'      openBracket, "\n", props.join(",\n"), "\n",'+#13#10+
'      (level > 1 ? repeatSring(console.dir.indention, level - 1) : ""),'+#13#10+
'      closeBracket'+#13#10+
'    );'+#13#10+
'  }'+#13#10+
''+#13#10+
'  /**'+#13#10+
'   * console object;'+#13#10+
'   * implements: log, dir, time, timeEnd'+#13#10+
'   */'+#13#10+
'  this.console = {'+#13#10+
''+#13#10+
'    /**'+#13#10+
'     * simple log using toString'+#13#10+
'     */'+#13#10+
'    log: println,'+#13#10+
''+#13#10+
'    /**'+#13#10+
'     * dir an object'+#13#10+
'     * @param {Object} object'+#13#10+
'     * @param {Variant} deep - level of depth, default is {Boolean} true'+#13#10+
'     * can be set also to {Number} value specifying needed level of depth'+#13#10+
'     * Examples:'+#13#10+
'     * - console.dir(obj) // console.log(obj, true)'+#13#10+
'     * - console.dir(obj, false); // only first level is shown'+#13#10+
'     * - console.dir(obj, 3); // properties of three levels are shown'+#13#10+
'     */'+#13#10+
'    dir: function (object, deep) {'+#13#10+
'      // if called for a primitive'+#13#10+
'      if (Object(object) !== object) {'+#13#10+
'        return console.log(object);'+#13#10+
'      }'+#13#10+
'      // else for an object'+#13#10+
'      return println(dir(object, deep));'+#13#10+
'    },'+#13#10+
''+#13#10+
'    // time functions borrowed from Firebug'+#13#10+
''+#13#10+
'    /**'+#13#10+
'     * time start'+#13#10+
'     */'+#13#10+
'    time: function(name) {'+#13#10+
'      timeMap[name] = Date.now();//(new Date()).getTime();'+#13#10+
'    },'+#13#10+
''+#13#10+
'    /**'+#13#10+
'     * time end'+#13#10+
'     */'+#13#10+
'    timeEnd: function(name) {'+#13#10+
'      if (name in timeMap) {'+#13#10+
'        var delta = /*(new Date()).getTime()*/ Date.now() - timeMap[name];'+#13#10+
'        println(name + ": ", delta + "ms");'+#13#10+
'        delete timeMap[name];'+#13#10+
'      }'+#13#10+
'    }'+#13#10+
'  };'+#13#10+
''+#13#10+
'  // indention for dir, default is 4 spaces'+#13#10+
'  console.dir.indention = "    ";'+#13#10+
''+#13#10+
'})(this);'+#13#10;

implementation

end.
