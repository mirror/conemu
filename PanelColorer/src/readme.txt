Panel Colorer Plugin v 0.94

Author: Igor Afanasyev <igor.afanasyev (at) gmail (dot) com>

License: GPL v.3 (see license.txt)

For latest releases and source code please visit
http://code.google.com/p/panel-colorer/

Description
-----------

Panel Colorer is an innovative and highly experimental plugin for
FAR manager and ConEmu console emulator (i.e. it requires both).

Though FAR is a console application, ConEmu adds GUI flavor to it and
allows some behavior a console application alone can't implement.
Panel Colorer uses ConEmu to dynamically change the background of FAR
application based on current FAR panels/windows state.

For screenshots, please visit
http://code.google.com/p/panel-colorer/


Features
--------

* Panels are colored based on volume drive type (fixed, removable,
  etc.), and each type has a set of slightly different colors to
  distinguish one drive from another.

* Each panel features drive description, its size and free/total
  usage meter at the bottom of the panel.

* Volumes mounted as folders (NTFS junctions) are recognized.

* Plugin panels have their own color, too; Panel Colorer recognizes
  most popular FAR plugins and provides special text and images for them.

* Rules for special directories like "C:\Windows\Temp" or ".svn".


Requirements
------------

* FAR 2.0 (build 1661+) x86.

* ConEmu 100906+ (either x86 or x64 version).


Installation
------------

1 Install FAR.
  http://www.farmanager.com/download.php

2 Install ConEmu into the same folder where FAR is located from.
  http://code.google.com/p/conemu-maximus5/downloads/list

3 Unpack plugin to FAR 'plugins' folder.

4 Start ConEmu (which will load FAR, and plugin will be enabled by default).

5 In ConEmu options, make sure 'Enable plugins' option is turned on.

6 In ConEmu options, adjust background opacity to your liking
  (recommended is the value of 127).


Configuration
-------------

All plugin rules are defined in config.js JavaScript file. See this file
for additional information on which parameters are passed to the script
and which parameters this script modifies.

If you want to set your own coloring/rendering logic, do not modify
config.js directly, as it will be overwritten with plugin update.
Instead, create a separate 'user.js' file in the same folder with
the 'main()' function and put your override rules there.
See sample user.js_*.txt files. Please note that if you want to use
Unicode strings or regular expressions, save your user.js script as UTF-8.


TODO
----

* More configurability.

* More rules for plugins, special folders out-of-the-box.


History
-------

0.94 (2010-Oct-07)
-----------------

- Added DiskInfoCache unit to limit the ratio of costly disk API calls -- this should lower CPU usage in file panel mode.
- Fixed a bug when setting usageMeterVisible to false would cause optimizations to fail
- Added new sample user.js script that removes usage meter from file panels
- More debug logging


0.93 (2010-Sep-26)
-----------------

- Now plugin calls FAR API functions from the main FAR thread, which should fix random crashes
- Added the ability to define a 'user.js' file with override coloring rules
- Added a couple of sample user.js files to the distribution
- img/plugin.png is now used as a default plugin image
- Minimum required version of ConEmu is now 100906


0.92 (2010-Sep-07)
-----------------

- Fixed rendering issues when console buffer has bigger height than physical window rows
- Fixed disk usage meter updating logic so it now redraws immediately when its size has changed


0.91 (2010-Sep-06)
-----------------

Massive changes towards 1.0 release. Now configuration is taken from config.js
file and is reloaded on the fly once this file is changed.
Still have to convert some hardcoded variables.


0.9 (2010-Sep-04)
-----------------

Initial release (source and binaries published).