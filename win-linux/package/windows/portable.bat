setlocal enabledelayedexpansion 

if not exist "node_modules/generate-evb" (
    call npm install generate-evb
)
if "%PACKAGE%"=="" ( 
    set PACKAGE=DesktopEditors
)

set ORG=onlyoffice
set OUTPATH=%~dp0..\..\..\..\build_tools\out\%PLATFORM%\%ORG%\%PACKAGE%
set FILES=msvcp140.dll vcruntime140.dll
if "%PLATFORM%"=="win_32" (
    set ARCH=x86
    set SOURCE=%SystemRoot%\SysWOW64
) else if "%PLATFORM%"=="win_64" (
    set ARCH=x64
    set SOURCE=%SystemRoot%\System32
    set FILES=%FILES% vcruntime140_1.dll
) else (
    echo "Architecture definition error"
    exit
)

robocopy %SOURCE% %OUTPATH% %FILES%
if not errorlevel 1 (
    echo "Dll not found, need to install vcredist_%PLATFORM%"
    exit
)

node create_portable.js %PLATFORM% %ORG% %PACKAGE%
%enigmavbconsole% DesktopEditorsPortable.evb
