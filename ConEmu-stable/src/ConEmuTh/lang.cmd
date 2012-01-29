@echo off
cd /d "%~dp0"
..\lng.generator.exe -nc -i lang.ini -ol Lang.templ
echo Copying *.lng files to Release.x64 folder
copy ..\..\plugins\ConEmu\Thumbs\ConEmuTh_*.lng ..\..\Release.x64\plugins\ConEmu\Thumbs\
