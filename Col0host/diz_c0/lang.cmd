@echo off
..\tools\lng.generator.exe -nc -i lang.ini -ol dizlang.templ

if "%~1"=="" goto fin

rem echo Copying *.lng files to %1
rem copy lng\PictureView_*.lng "%~1"

:fin
