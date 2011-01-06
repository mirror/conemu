set PATH=C:\Program Files (x86)\Embarcadero\RAD Studio\7.0\bin;%PATH%

cd /d "%~dp0"
if not exist ..\Temp md ..\Temp
if not exist ..\Release md ..\Release

dcc32.exe -B PanelColorer.dpr
