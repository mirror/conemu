@echo off

set BUILD_RC=ERROR

call "%~dp0..\..\build.cmd" /build "%~dp0pvdWIC.vcproj" "Debug|Win32" x86
if not %BUILD_RC% == OK goto err

echo Build succeeded
goto gcc

:err
echo Build failed, code: %BUILD_RC%
goto fin

:gcc
rem %~d0
rem cd "%~p0"
rem call gcc
rem cd ..

:fin
pause
