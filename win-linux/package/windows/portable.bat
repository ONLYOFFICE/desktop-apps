if not exist "node_modules/generate-evb" (
    call npm install generate-evb
)
if "%PACKAGE%"=="" ( 
    set PACKAGE=DesktopEditors
)

set ORG=onlyoffice
set CURL=curl -L
set OUTPATH="%~dp0..\..\..\..\build_tools\out\%PLATFORM%\%ORG%\%PACKAGE%"
set SYSTEM32=%SystemRoot%\System32

if "%PLATFORM%"=="win_32" (
    set ARCH=x86
    copy %SYSTEM32%\vcruntime140.dll %OUTPATH%
    copy %SYSTEM32%\msvcp140.dll %OUTPATH%
    if errorlevel 1 (
            exit 1
    )
) else if "%PLATFORM%"=="win_64" (
    set ARCH=x64
    copy %SYSTEM32%\vcruntime140.dll %OUTPATH%
    copy %SYSTEM32%\vcruntime140_1.dll %OUTPATH%
    copy %SYSTEM32%\msvcp1401.dll %OUTPATH%
    if errorlevel 1 (
            exit 1
    )
) else (
    exit
)

node create_portable.js %PLATFORM% %ORG% %PACKAGE%
%enigmavbconsole% DesktopEditorsPortable.evb
