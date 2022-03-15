setlocal enabledelayedexpansion 

if not exist "node_modules/generate-evb" (
    call npm install generate-evb
)
if "%PACKAGE%"=="" ( 
    set PACKAGE=DesktopEditors
)

set ORG=onlyoffice
set OUTPATH="%~dp0..\..\..\..\build_tools\out\%PLATFORM%\%ORG%\%PACKAGE%"
set SYSTEM32=%SystemRoot%\System32
set SYSWOW64=%SystemRoot%\SysWOW64
set files[0]=msvcp140.dll
set files[1]=vcruntime140.dll
set files[2]=vcruntime140_1.dll

if "%PLATFORM%"=="win_32" (
    set ARCH=x86
    for /l %%i in (0 1) do (
        copy %SYTWOW64%!files[%%i]! %OUTPATH%
        if errorlevel 1 (
            exit 1
        )
    )
) else if "%PLATFORM%"=="win_64" (
    set ARCH=x64
    for /l %%i in (0 1 2) do (
        copy %SYTEM32%!files[%%i]! %OUTPATH%
        if errorlevel 1 (
            exit 1
        )
    )
) else (
    exit
)

node create_portable.js %PLATFORM% %ORG% %PACKAGE%
%enigmavbconsole% DesktopEditorsPortable.evb
