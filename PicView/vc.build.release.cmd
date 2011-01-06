@echo off

set BUILD_RC=ERROR
set CONF="Release|Win32"
call :do_build /build 0PictureView.sln %CONF% x86
if not %BUILD_RC% == OK echo Build failed, code: %BUILD_RC% & goto fin1

set BUILD_RC=ERROR
set CONF="Release|x64"
call :do_build /build 0PictureView.sln %CONF% x64
if not %BUILD_RC% == OK echo Build failed, code: %BUILD_RC% & goto fin1

echo Build succeeded


rem set BUILD_RC=ERROR
rem 
rem set CONF="Release|Win32"
rem call "%~dp0build.cmd" /build 0PictureView.vcproj %CONF%
rem 
rem if %BUILD_RC% == OK echo Build succeeded & goto gcc
rem 
rem echo Build failed, code: %BUILD_RC%
rem goto fin1
rem 
rem :gcc
rem rem %~d0
rem rem cd "%~p0"
rem rem call gcc
rem rem cd ..

:fin1
pause
goto :eof




:do_build
set BUILD_RC=ERROR
if "%~4"=="" goto no_parms
set WIDE=1
rem cls
call "%VS90COMNTOOLS%vsvars32.bat" %3

rem Use:
rem vcexpress  [solutionfile | projectfile | anyfile.ext]  [switches]
rem 
rem Command line builds:
rem devenv solutionfile.sln /build [ solutionconfig ] [ /project projectnameorfile [ /projectconfig name ] ]
rem Available command line switches:
rem 
rem /Log	Logs IDE activity to the specified file for troubleshooting.

%~d0
cd "%~p0"
cd

if exist !Errors.log del !Errors.log

set DEVEXE=.
if exist "%VS90COMNTOOLS%..\IDE\VCExpress.exe" set DEVEXE="%VS90COMNTOOLS%..\IDE\VCExpress.exe"
if exist "%VS90COMNTOOLS%..\IDE\devenv.com" set DEVEXE="%VS90COMNTOOLS%..\IDE\devenv.com"
if %DEVEXE%==. goto nostudio

echo #pragma once > "%~dp0PVDInterface\usetodo.hpp"
echo #define HIDE_TODO >> "%~dp0PVDInterface\usetodo.hpp"

echo Building config: %3
%DEVEXE% %2 %1 %3 /Out !Errors.log
if errorlevel 1 goto e1
set BUILD_RC=OK

goto typeerr
:e1
set BUILD_RC=BUILD_ERRORS
:typeerr

echo #pragma once > "%~dp0PVDInterface\usetodo.hpp"
echo //#define HIDE_TODO >> "%~dp0PVDInterface\usetodo.hpp"

if exist !Errors.log type !Errors.log
goto fin
:nostudio
set BUILD_RC=VC_NOT_FOUND
Echo Visual Studio 9 Pro or Express not found!
goto fin
:no_parms
Echo Usage: "%~nx0" {/build ^| /rebuild} {sln_file} {config}
echo Exmpl: "%~nx0" /build ce.sln "Debug|Win32"
goto fin
:fin
