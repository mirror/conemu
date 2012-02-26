@echo off
lng.generator.exe -nc -i lang.ini -ol Lang.templ

if "%~1"=="" goto fin

echo Copying *.lng files to %1
copy lng\PictureView_*.lng "%~1"

:fin
