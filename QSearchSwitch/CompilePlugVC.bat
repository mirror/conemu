@echo off
@cls

rem =============== Use Microsoft Visual Studio 2009 ======================

if exist "%VS90COMNTOOLS%..\..\VC\BIN\vcvars32.bat" call "%VS90COMNTOOLS%..\..\VC\BIN\vcvars32.bat"

rem  ======================== Set name and version ... =========================

@set PlugName=QSearchSwitch
@set fileversion=1,0,2,0
@set fileversion_str=1.0 build 2
@set comments=ConEmu.Maximus5@gmail.com
@set filedescription=QSearch switch for Far Manager
@set legalcopyright=© Maximus5 2012

call :build
if errorlevel 1 goto err1
rem if not exist Version md Version
rem move Version.dll Version\Version.dll

goto :EOF

:build

rem  ==================== remove temp files ====================================

if not exist %PlugName%.map goto nomap
attrib -H %PlugName%.map
del %PlugName%.map
:nomap

rem  ==================== Make %PlugName%.def file... ==========================

echo Make %PlugName%.def file...

@echo EXPORTS                                              >  %PlugName%.def
@echo   GetGlobalInfoW                                     >> %PlugName%.def
@echo   GetMinFarVersionW                                  >> %PlugName%.def
@echo   GetPluginInfoW                                     >> %PlugName%.def
@echo   SetStartupInfoW                                    >> %PlugName%.def
@echo   ProcessConsoleInputW                               >> %PlugName%.def

@if exist %PlugName%.def echo ... succesfully

rem  ================== Make %PlugName%.rc file... =============================

echo Make %PlugName%.rc file...

@echo #define VERSIONINFO_1   1                                 > %PlugName%.rc
@echo.                                                          >> %PlugName%.rc
@echo VERSIONINFO_1  VERSIONINFO                                >> %PlugName%.rc
@echo FILEVERSION    %fileversion%                              >> %PlugName%.rc
@echo PRODUCTVERSION 3,0,0,0                                    >> %PlugName%.rc
@echo FILEFLAGSMASK  0x0                                        >> %PlugName%.rc
@echo FILEFLAGS      0x0                                        >> %PlugName%.rc
@echo FILEOS         0x4                                        >> %PlugName%.rc
@echo FILETYPE       0x2                                        >> %PlugName%.rc
@echo FILESUBTYPE    0x0                                        >> %PlugName%.rc
@echo {                                                         >> %PlugName%.rc
@echo   BLOCK "StringFileInfo"                                  >> %PlugName%.rc
@echo   {                                                       >> %PlugName%.rc
@echo     BLOCK "000004E4"                                      >> %PlugName%.rc
@echo     {                                                     >> %PlugName%.rc
@echo       VALUE "CompanyName",      "%companyname%\0"         >> %PlugName%.rc
@echo       VALUE "FileDescription",  "%filedescription%\0"     >> %PlugName%.rc
@echo       VALUE "FileVersion",      "%fileversion_str%\0"     >> %PlugName%.rc
@echo       VALUE "InternalName",     "%PlugName%\0"            >> %PlugName%.rc
@echo       VALUE "LegalCopyright",   "%legalcopyright%\0"      >> %PlugName%.rc
@echo       VALUE "OriginalFilename", "%PlugName%.dll\0"        >> %PlugName%.rc
@echo       VALUE "ProductName",      "FAR Manager\0"           >> %PlugName%.rc
@echo       VALUE "ProductVersion",   "3.0\0"                   >> %PlugName%.rc
@echo       VALUE "Comments",         "%comments%\0"            >> %PlugName%.rc
@echo     }                                                     >> %PlugName%.rc
@echo   }                                                       >> %PlugName%.rc
@echo   BLOCK "VarFileInfo"                                     >> %PlugName%.rc
@echo   {                                                       >> %PlugName%.rc
@echo     VALUE "Translation", 0, 0x4e4                         >> %PlugName%.rc
@echo   }                                                       >> %PlugName%.rc
@echo }                                                         >> %PlugName%.rc

@if exist %PlugName%.rc echo ... succesfully

rem  ==================== Compile %PlugName%.fmt file...========================

@if exist %PlugName%.dll del %PlugName%.dll>nul
@rc /l 0x4E4 %PlugName%.rc
if errorlevel 1 goto err1

@echo ***************
set libcrt=
if exist libCRT.lib set libcrt=/nodefaultlib libCRT.lib
@cl /Zp8 /O2 /GF /Gr /GR- /D_UNICODE /DUNICODE /EHs-c- /MT %PlugName%.cpp /link /DLL /RELEASE /subsystem:console /machine:I386 /noentry /def:%PlugName%.def %libcrt% kernel32.lib User32.lib %PlugName%.res /map:"%PlugName%.map" /out:"%PlugName%.dll" /merge:.rdata=.text
if errorlevel 1 goto err1

if exist %PlugName%.exp del %PlugName%.exp>nul
if exist %PlugName%.obj del %PlugName%.obj>nul
if exist %PlugName%.lib del %PlugName%.lib>nul
if exist %PlugName%.res del %PlugName%.res>nul

goto :EOF




echo ***************



:err1
