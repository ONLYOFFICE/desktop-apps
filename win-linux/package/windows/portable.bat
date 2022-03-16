setlocal enabledelayedexpansion 

if not exist "node_modules/generate-evb" (
    call npm install generate-evb
)
if "%PACKAGE%"=="" ( 
    set PACKAGE=DesktopEditors
)

set ORG=onlyoffice
set OUTPATH=%~dp0..\..\..\..\build_tools\out\%PLATFORM%\%ORG%\%PACKAGE%

if "%PLATFORM%"=="win_32" (
    set ARCH=x86
    set SOURCE=%SystemRoot%\SysWOW64
    set files = msvcp140.dll vcruntime140.dll
) else if "%PLATFORM%"=="win_64" (
    set ARCH=x64
    set SOURCE=%SystemRoot%\System32
    set files=msvcp140.dll vcruntime140.dll vcruntime140_1.dll
) else (
    exit
)

for %%i in (%files%) do (
    copy %SOURCE%\%%i %OUTPATH%
    ::to avoid the error, need to install vcredist 2015-2022 x64 and x86
    if errorlevel 1 (
        echo "Dll not found, need to install vcredist_%PLATFORM%"
        exit 1
    )
)

node create_portable.js %PLATFORM% %ORG% %PACKAGE%
%enigmavbconsole% DesktopEditorsPortable.evb
