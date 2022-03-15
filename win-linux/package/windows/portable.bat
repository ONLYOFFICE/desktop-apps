setlocal enabledelayedexpansion 

if not exist "node_modules/generate-evb" (
    call npm install generate-evb
)
if "%PACKAGE%"=="" ( 
    set PACKAGE=DesktopEditors
)

set ORG=onlyoffice
set OUTPATH="%~dp0..\..\..\..\build_tools\out\%PLATFORM%\%ORG%\%PACKAGE%"

if "%PLATFORM%"=="win_32" (
    set ARCH=x86
    for %%i in (msvcp140.dll vcruntime140.dll) do (
        copy %SystemRoot%\SysWOW64\%%i %OUTPATH%
        if errorlevel 1 (
            echo dll not found
            exit 1
        )
    )
) else if "%PLATFORM%"=="win_64" (
    set ARCH=x64
    for %%i in (msvcp140.dll vcruntime140.dll vcruntime140_1.dll) do (
        copy %SystemRoot%\System32\%%i %OUTPATH%
        if errorlevel 1 (
            echo dll not found
            exit 1
        )
    )
) else (
    exit
)

node create_portable.js %PLATFORM% %ORG% %PACKAGE%
%enigmavbconsole% DesktopEditorsPortable.evb
