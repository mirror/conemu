@echo off

set BUILD_RC=ERROR
call "%~dp0build.sln.cmd" /Rebuild pvdDJVU.vcproj "Release_Lib|Win32"
if NOT %BUILD_RC% == OK goto err

set BUILD_RC=ERROR
call "%~dp0build.sln.cmd" /Rebuild pvdDJVU.vcproj "Release_Lib|x64"
if NOT %BUILD_RC% == OK goto err

echo Build succeeded

goto fin

:err
echo Build failed, code: %BUILD_RC%
goto fin

:fin
pause
