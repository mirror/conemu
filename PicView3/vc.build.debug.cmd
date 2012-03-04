@echo off

set BUILD_RC=ERROR

set CONF="Debug3|Win32"
rem if %COMPUTERNAME%==MAX set CONF="Debug|Win32"
rem if %COMPUTERNAME%==MAXPC set CONF="DebugEEE|Win32"

call "%~dp0build.cmd" /build 0PicView.vcproj %CONF% x86

if %BUILD_RC% == OK echo Build succeeded & goto gcc

echo Build failed, code: %BUILD_RC%
goto fin

:gcc
rem %~d0
rem cd "%~p0"
rem call gcc
rem cd ..

:fin
pause
