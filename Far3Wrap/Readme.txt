Usage
=====
1. Unpack distribution contents to %FARHOME% directory.
2. Copy Loader.dll (Loader64.dll using Far3 x64) to the directory,
   containing Far2 plugin binary (i.e. where colorer.dll located).
3. (optionally) Rename Far2 plugin binary (i.e. colorer.dll -> colorer.dl_).
4. (optionally) Rename Loader.dll to more obvious name (i.e. colorer3.dll).
5. (optionally) Create wrapper ini-file. It must match exactly with wrapper
   file name, renamed on step 3 (i.e. colorer3.ini). If you don't create
   ini-file, wrapper will create it for you with unique GUIDs.
   !!!WARNING!!! When you create ini-files manually, you must create
   unique GUIDs in each ini wrapper file.

Sample wrapper ini files are included in distribution.


License
=======
Copyright (c) 2011 Maximus5
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ''AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.



TechInfo
========
Far3Wrap.dll (or Far3Wrap64.dll using Far3 x64) may be located in:
1) directory with Loader.dll (Loader64.dll using Far3 x64)
2) directory, defined by "Far3WrapperPath" environment variable
3) "%FARHOME%\Plugins"
4) "%FARHOME%"

When wrapper ini file was not created yet, Loader.dll try to
locate Far2 plugin in it's own directory.
Wrapper try to find *.dll and *.dl_ files, which
HAVE SetStartupInfoW and DON'T HAVE GetGlobalInfoW.
So, if Far2 plugin does not export SetStartupInfoW you must
create wrapper ini file manually!
