@echo off

set ConEmuMacroResult=
set ESC=

if not "%ConEmuANSI%"=="ON" (
echo ConEmu ANSI X3.64 was not set!
goto :EOF
)

echo %ESC%[s%ESC%]9;6;"IsConEmu"%ESC%\%ESC%[u%ESC%[A
if "%ConEmuMacroResult%"=="Yes" (
echo ConEmu found!
) else (
echo ConEmu NOT found!
)
