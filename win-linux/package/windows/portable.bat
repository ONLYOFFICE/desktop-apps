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
    %CURL% https://aka.ms/vs/17/release/vc_redist.x86.exe --output vcredist_2022_x86.exe /w
    start vcredist_2022_x86.exe /silent /w
    copy %SYSTEM32%\vcruntime140.dll %OUTPATH%
    copy %SYSTEM32%\msvcp140.dll %OUTPATH%
) else if "%PLATFORM%"=="win_64" (
    set ARCH=x64
    %CURL% https://aka.ms/vs/17/release/vc_redist.x64.exe --output vcredist_2022_x64.exe /w
    start /w vcredist_2022_x64.exe /silent
    copy %SYSTEM32%\vcruntime140.dll %OUTPATH%
    copy %SYSTEM32%\vcruntime140_1.dll %OUTPATH%
    copy %SYSTEM32%\msvcp140.dll %OUTPATH%
) else (
    exit
)

node create_portable.js %PLATFORM% %ORG% %PACKAGE%
%enigmavbconsole% DesktopEditorsPortable.evb
