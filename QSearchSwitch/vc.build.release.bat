@echo off

if exist makefile_lib_vc set MAKEFILE=makefile_lib_vc
if exist makefile_vc set MAKEFILE=makefile_vc


call "%VS90COMNTOOLS%..\..\VC\BIN\vcvars32.bat"
set WIDE=
set IA64=
set AMD64=
rem ¬ключим отладочную информацию
rem set DEBUG=1
set DEBUG=
rem nmake /A /B /F %MAKEFILE%
rem nmake /F %MAKEFILE%
rem if errorlevel 1 goto end

rem set WIDE=1
rem nmake /A /B /F %MAKEFILE%
rem nmake /F %MAKEFILE%
rem if errorlevel 1 goto end

set WIDE=3
nmake /A /B /F %MAKEFILE%
rem nmake /F %MAKEFILE%
if errorlevel 1 goto end


call "%VS90COMNTOOLS%..\..\VC\BIN\x86_amd64\vcvarsx86_amd64.bat"
set WIDE=
set AMD64=1
rem nmake /A /B /F %MAKEFILE%
rem nmake /F %MAKEFILE%

set WIDE=1
rem nmake /A /B /F %MAKEFILE%
rem nmake /F %MAKEFILE%

set WIDE=3
nmake /A /B /F %MAKEFILE%
rem nmake /F %MAKEFILE%

:end
