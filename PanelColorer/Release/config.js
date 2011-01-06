/*

NOTE: please *DO NOT EDIT* this file, as it will be overwritten
with plugin update. Instead, create a separate 'user.js' file
in the same folder with the 'main()' function and put
your override rules there. See sample user.js_*.txt files.
Please note that if you want to use Unicode strings or regular
expressions, save your user.js script as UTF-8.


Plugin version: 0.93

=======================================================
Input variables (these will be set by the calling host)
=======================================================

---------------------------------------------------
These variables will be set for windows of any type
---------------------------------------------------

windowType (number, enumeration)
      type of the window
      see window type constants below

symbolWidth (number)
symbolHeight (number)
      dimensions of console symbol in pixels, can be used
      to calculate geometry in accordance with current console font


--------------------------------------------------
These variables will be set for panels of any type
--------------------------------------------------

panelType (number, enumeration)
      see PTYPE_ constants below

isPlugin (boolean)
      set to true if panel contents are provided by a plugin

path (string)
      current panel directory (for plugin panels, this equals to "panelHostFile\panelPath", see below)
      Example: 'C:\Program Files'

volumeRoot (string)
      calculated based from path and provides real root
      (which can be different for volumes and junctions)
      Example: 'C:\' or '\\?\Volume{...}'

driveType (number, enumeration)
      gathered based on volumeRoot
      see DRIVE_* constants below


-------------------------------------------
These variables will be set for file panels
-------------------------------------------

bytesTotal (number)
      total size of the disk in bytes

bytesFree (number)
      free bytes left on the disk

----------------------------------------------------------
The following variables will be set only for plugin panels
----------------------------------------------------------

panelFormat (string)
      [requires FAR build 1557+, otherwise will be empty]
      can be provided by a plugin, format depends on the plugin
      Example: "ZIP archive"

panelHostFile (string)
      [requires FAR build 1557+, otherwise will be empty]
      when panel is rendered by a plugin, it might provide a path
      to a host file (for example, when browsing zip file contents,
      this will be a path to the zip file)


--------------------------------------------------------------
The following variables will be set for both viewer and editor
--------------------------------------------------------------

codepage (string)
      [not implemented]
      current codepage


---------------------------------------------------
The following variables will be set only for viewer
---------------------------------------------------

viewerHexMode (boolean)
      [not implemented]
      set when viewer is in hex mode


---------------------------------------------------
The following variables will be set only for editor
---------------------------------------------------

editorModified (boolean)
      [not implemented]
      set to true when file is changed in editor

editorSaved (boolean)
      [not implemented]
      set to true when file was saved in editor

editorLocked (boolean)
      [not implemented]
      set to true when file is locked in editor (Ctrl+L)


==================================================
Output variables (which can be set by this script)
==================================================

text (string)
      text to render on panel

backgroundColor (number)
      panel/window background color
      see color constants below for examples

color (number)
      [not implemented]
      text color; also the key color for overlay images

image (string)
      path to image file that should be used instead of text

fontFamily (string)
      [not implemented]
      font family to use to render text on panel

fontSize (number)
      [not implemented]
      text size in pt

usageMeterVisible (boolean)
      if set to true or false, will override the default
      visiblity of the disk usage meter

usageMeterBackgroundColor (number)
      [not implemented]

usageMeterColor (number)
      [not implemented]

usageMeterLeft (number)
      [not implemented]

usageMeterTop (number)
      [not implemented]

usageMeterRight (number)
      [not implemented]

usageMeterBottom (number)
      [not implemented]

usageMeterWidth (number)
      [not implemented]
      width of the progress indicator in pixels


*/

// the inline code below will be executed once (at script compilation time).
// use it for initialization, defining constants, etc.

// window type constants (see FAR API)

WTYPE_BACKGROUND = 0; // special "window type" for console buffer (when running a command or after pressing Ctrl+O)
WTYPE_PANELS     = 1; // panels (one or both) are active
WTYPE_VIEWER     = 2; // viewer is active
WTYPE_EDITOR     = 3; // editor is active
// other window types below are not supported by this plugin
//WTYPE_DIALOG = 4;
//WTYPE_VMENU  = 5;
//WTYPE_HELP   = 6;

// panel type constants (see FAR API)

PTYPE_FILEPANEL  = 0; // File panel
PTYPE_TREEPANEL  = 1; // Directory tree
PTYPE_QVIEWPANEL = 2; // File quick preview
PTYPE_INFOPANEL  = 3; // drive/directory/memory information

// drive types (see WinAPI)

DRIVE_UNKNOWN     = 0; // The drive type cannot be determined.
DRIVE_NO_ROOT_DIR = 1; // The root path is invalid; for example, there is no volume mounted at the specified path.
DRIVE_REMOVABLE   = 2; // The drive has removable media; for example, a floppy drive, thumb drive, or flash card reader.
DRIVE_FIXED       = 3; // The drive has fixed media; for example, a hard drive or flash drive.
DRIVE_REMOTE      = 4; // The drive is a remote (network) drive.
DRIVE_CDROM       = 5; // The drive is a CD-ROM drive.
DRIVE_RAMDISK     = 6; // The drive is a RAM disk.

// color constants (0xAARRGGBB)

// standard 16-color palette

clBlack   = 0xff000000;
clNavy    = 0xff000080;
clGreen   = 0xff008000;
clTeal    = 0xff008080;
clMaroon  = 0xff800000;
clPurple  = 0xff800080;
clOlive   = 0xff808000;
clSilver  = 0xffc0c0c0;
clGray    = 0xff808080;
clBlue    = 0xff0000ff;
clLime    = 0xff00ff00;
clAqua    = 0xff00ffff;
clRed     = 0xffff0000;
clFuchsia = 0xffff00ff;
clYellow  = 0xffffff00;
clWhite   = 0xffffffff;

// additional colors

clBrown    = 0xff663300;
clDarkGray = 0xff333333;
clSkyBlue  = 0xFF0099FF;

// semitransparent colors

clBlack50 = setAlpha(clBlack, 0.5);
clWhite50 = setAlpha(clWhite, 0.5);

// helper functions

function setAlpha(c, a) { // alpha is in [0..1] range
  return setAlphaByte(c, Math.round(a * 255)); 
}

function setAlphaByte(c, a) { // alpha is in [0..255] range
  return (c & 0xffffff | a << 24) >>> 32; // use ">>> 32" trick to convert to unsigned int
}

// All disk sizes are in megabytes/gigabytes (1000^N),
// not in mebibytes/gibibytes (1024^N)
function formatSize(size) {
  if (size < 1000) {
    return size + ' B';
  }

  if (size < 1000 * 1000) {
    return Math.round(size / 1000) + ' kB';
  }

  if (size < 1000 * 1000 * 1000) {
    return Math.round(size / 1000 / 1000) + ' MB';
  }

  if (size < 1000 * 1000 * 1000 * 1000) {
    return Math.round(size / 1000 / 1000 / 1000) + ' GB';
  }

  return Math.round(10 * size / 1000 / 1000 / 1000 / 1000) / 10 + ' TB'; // with one decimal
}

// this function will be called whenever plugin needs to recalculate colors
// note that all parameters needed for rendering (see documentation
// at the beginning of this file) are set as global variables
//------------------------------------------------------------------------------
function main() {
//------------------------------------------------------------------------------

  // by default, console has black background
  // note that function main() for WTYPE_BACKGROUND is called only once
  // when plugin is [re-]loaded

  if (windowType == WTYPE_BACKGROUND) {
    backgroundColor = clBlack;
    return;
  }

  // viewer window has blue background
  // called every time a viewer window is opened or switched to (for each viewer instance)

  if (windowType == WTYPE_VIEWER) {
    backgroundColor = clNavy;
    return;
  }

  // editor window has green background
  // called every time an editor window is opened or switched to (for each editor instance)

  if (windowType == WTYPE_EDITOR) {
    backgroundColor = clGreen;
    return;
  }

  // this will be called separately for each visible panel, every time a panel is shown
  // or panel type is changed (e.g. user opens information or quick preview panel)
  // or when panels path has changed (e.g. user switches to another directory)

  if (windowType == WTYPE_PANELS) {

    // render plugin panels with their own style

    if (isPlugin) {
      image = 'img/plugin.png';
      backgroundColor = clGray; // default color for all plugin panels
      text = panelFormat ? panelFormat : 'Plugin'; // default text for all plugin panels

      // 'MultiArc' plugin provides format as e.g. 'ZIP archive', 'RAR archive'
      if (panelFormat.match(/^(\w+) archive$/)) {
        image = 'img/archive.png';
        return;
      }

      // 'arclite' plugin (MultiArc replacement) uses 'arc' constant
      if (panelFormat == 'arc') {
        image = 'img/archive.png';
        if (a = panelHostFile.match(/\.(.+)/)) {
          text = a[1] + ' archive'; // e.g. 'ZIP archive' or 'ISO archive'
        } else {
          text = 'Archive';
        }
        return;
      }

      // 'Resource Browser' plugin provides format as 'Resource' constant
      if (panelFormat == 'Resource') {
        text = 'Resources';
        return;
      }

      // Two 'Registry Browser' plugins provide format as 'Reg' or 'reg2' constants
      if ((panelFormat == 'Reg') || (panelFormat == 'reg2')) {
        image = 'img/caution.png';
        text = 'Registry';
        return;
      }

      // 'Network Browser' plugin provides format as 'Network' constant
      if (panelFormat == 'Network') {
        image = 'img/network.png';
        return;
      }

      // 'FTP' plugin provides format as e.g. '//Hosts/', '//domain.com/path'
      if (panelFormat.indexOf('//') == 0) {
        image = 'img/globe.png';
        if (text == '//Hosts/') {
          text = 'FTP Hosts';
        } else if (a = panelFormat.match(/^\/\/(.*?)\//)) {
          text = a[1]; // e.g. 'domain.com'
        }
        return;
      }

      return;
    } // end if isPlugin

    // provide individual styles for folders with special meaning

    // temporary folder
    if (path.indexOf('C:\\Windows\\Temp') == 0) {
      backgroundColor = clBrown;
      image = 'img/trash.png';
      text = 'Temporary files';
      return;
    }

    // version control folders
    if (a = path.match(/(\.svn|\.git|\.hg|\.bzr|CVS)$/)) {
      backgroundColor = clDarkGray;
      image = 'img/caution.png';
      text = a[1]; // e.g. '.svn'
      return;
    }

    // Dropbox folder
    if (path.indexOf('My Dropbox') >= 0) {
      backgroundColor = clSkyBlue;
      image = 'img/dropbox.png';
      text = 'Dropbox';
      return;
    }

    // default file panel style

    image = [
      'img/drive_fixed.png',     // DRIVE_UNKNOWN
      'img/drive_fixed.png',     // DRIVE_NO_ROOT_DIR
      'img/drive_removable.png', // DRIVE_REMOVABLE
      'img/drive_fixed.png',     // DRIVE_FIXED
      'img/drive_network.png',   // DRIVE_REMOTE
      'img/drive_cdrom.png',     // DRIVE_CDROM
      'img/drive_ramdisk.png'    // DRIVE_RAMDISK
    ][driveType];

    text = volumeRoot.substr(0, 2); // text by default is first two chars of the path (e.g. "C:");

    // volumes that have no disk char assigned (e.g. junctions) start with "\\?\Volume"
    if (volumeRoot.indexOf('\\\\?\\') == 0) {
      text = 'Volume';
    }

    text += ' ' + formatSize(bytesTotal);

    // color drives based on their type
    // (use 5 slightly different colors for each type)
    backgroundColor = [
      [clBlue,     clBlue,     clBlue,     clBlue,     clBlue    ], // DRIVE_UNKNOWN
      [clBlue,     clBlue,     clBlue,     clBlue,     clBlue    ], // DRIVE_NO_ROOT_DIR
      [0xff8cd900, 0xff70d900, 0xff0bd900, 0xff00d94b, 0xff00d976], // DRIVE_REMOVABLE
      [0xff0059ff, 0xff0043ff, clBlue,     0xff3b00ff, 0xff5f00ff], // DRIVE_FIXED
      [0xffe600ff, 0xfff300ff, clFuchsia,  0xffff00ea, 0xffff00d5], // DRIVE_REMOTE
      [0xffff007e, 0xffff005d, clRed,      0xffff4000, 0xffff5900], // DRIVE_CDROM
      [clOlive,    clOlive,    clOlive,    clOlive,    clOlive   ]  // DRIVE_RAMDISK
    ][driveType][Math.abs(path.charCodeAt(0) - 'A'.charCodeAt(0)) % 5];

  } // end of WTYPE_PANELS
} // end of main()
