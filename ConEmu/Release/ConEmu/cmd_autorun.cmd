@echo off

if "%1"=="" goto noparm
goto checkparm

:noparm
rem чтобы не раздражать лишним текстом, когда ConEmu уже запущено...
rem так определять не совсем корректно. cmd могли запустить из ConEmu
rem в режиме создания новой консоли... ну да ладно, пока так...
if "%ConEmuHWND%"=="" goto noconemu
goto conemufound


:checkparm
if "%1"=="/?" goto help
if "%1"=="-?" goto help

if "%1"=="/i" goto install
if "%1"=="-i" goto install

if "%1"=="/u" goto uninstall
if "%1"=="-u" goto uninstall

rem Под телнетом - не запускать!
if not "%TERM%"=="" goto termfound

rem Проверить, может ConEmu уже запущено?
rem Warning!!! Это не работает, если в ConMan(!) указан
rem "CreateInNewEnvironment"=dword:00000001
rem Нужно так:
rem REGEDIT4
rem [HKEY_CURRENT_USER\Software\HoopoePG_2x]
rem "CreateInNewEnvironment"=dword:00000000

:noconemu
set ConEmuPath=%~dp0ConEmuC.exe
if not exist "%ConEmuPath%" set ConEmuPath=ConEmuC.exe
if not exist "%ConEmuPath%" goto notfound
echo ConEmu autorun (c) Maximus5
echo Starting "%ConEmuPath%" in "Attach" mode
call "%ConEmuPath%" /ATTACH /NOCMD
echo Done
goto fin

:notfound
Echo ConEmu not found! "%~dp0ConEmuC.exe"
goto fin
:termfound
Echo ConEmu can not be started under Telnet!
goto fin
:conemufound
rem Echo ConEmu already started (%ConEmuHWND%)
goto fin

:help
echo ConEmu autorun (c) Maximus5
echo Usage
echo    cmd_autorun.cmd /?  - displays this help
echo    cmd_autorun.cmd /i  - turn ON ConEmu autorun for cmd.exe
echo    cmd_autorun.cmd /u  - turn OFF ConEmu autorun
goto fin

:install
set ConEmuPath=%~dp0ConEmu.exe
if not exist "%ConEmuPath%" set ConEmuPath=ConEmu.exe
if not exist "%ConEmuPath%" goto notfound
echo ConEmu autorun (c) Maximus5
echo Enabling autorun
call "%ConEmuPath%" /autosetup 1 "%~0"
goto CheckRet

:uninstall
set ConEmuPath=%~dp0ConEmu.exe
if not exist "%ConEmuPath%" set ConEmuPath=ConEmu.exe
if not exist "%ConEmuPath%" goto notfound
echo ConEmu autorun (c) Maximus5
echo Disabling autorun
call "%ConEmuPath%" /autosetup 0
goto CheckRet


:CheckRet
if errorlevel 2 echo ConEmu failed
if errorlevel 1 echo All done
goto fin

:fin
