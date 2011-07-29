@echo off

if exist makefile_lib_vc set MAKEFILE=makefile_lib_vc
if exist makefile_vc set MAKEFILE=makefile_vc

if exist "%VS90COMNTOOLS%..\..\VC\BIN\vcvars32.bat" (
  call "%VS90COMNTOOLS%..\..\VC\BIN\vcvars32.bat"
) else (
  call "C:\Program Files\Microsoft Visual Studio 9.0\VC\bin\vcvars32.bat"
)

set WIDE=1
set IA64=
set AMD64=
set DEBUG=1

nmake /A /B /F %MAKEFILE%
rem nmake /F %MAKEFILE% DEBUG=%DEBUG%
if errorlevel 1 goto end

rem call "%VS90COMNTOOLS%..\..\VC\BIN\x86_amd64\vcvarsx86_amd64.bat"
if exist "%VS90COMNTOOLS%..\..\VC\BIN\x86_amd64\vcvarsx86_amd64.bat" (
  call "%VS90COMNTOOLS%..\..\VC\BIN\x86_amd64\vcvarsx86_amd64.bat"
) else (
  call "C:\Program Files\Microsoft Visual Studio 9.0\VC\bin\vcvarsx86_amd64.bat"
)
set AMD64=1
nmake /A /B /F %MAKEFILE%
rem nmake /F %MAKEFILE% DEBUG=%DEBUG%


set BUILD_RC=ERROR

set CONF="ReleaseW3|Win32" "ReleaseW3|x64"

call "%~dp0build.sol.bat" /build Far3Wrap.vcproj %CONF%

if %BUILD_RC% == OK echo Build succeeded & goto fin

echo Build failed, code: %BUILD_RC%

:fin
if exist "%~dp0..\*.exp" del "%~dp0..\*.exp"
if exist "%~dp0..\Loader*.lib" del "%~dp0..\Loader*.lib"
if exist "%~dp0..\Far3Wrap*.lib" del "%~dp0..\Far3Wrap*.lib"
pause

:end
