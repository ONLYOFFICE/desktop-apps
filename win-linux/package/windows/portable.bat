if not exist "node_modules/generate-evb" (
    call npm install generate-evb
)
if "%PACKAGE%"=="" ( 
    set PACKAGE=DesktopEditors
)
set LINK86=https://s3.eu-west-1.amazonaws.com/repo-doc-onlyoffice-com/onlyoffice/unstable/windows/99.99.99-2847/desktop/ONLYOFFICE_DesktopEditors_99.99.99.2847_x86.exe
set LINK64=https://s3.eu-west-1.amazonaws.com/repo-doc-onlyoffice-com/onlyoffice/unstable/windows/99.99.99-2847/desktop/ONLYOFFICE_DesktopEditors_99.99.99.2847_x64.exe
set ORG=onlyoffice
set CURL=curl -L
set OUTPATH="%~dp0..\..\..\..\build_tools\out\%PLATFORM%\%ORG%\%PACKAGE%"

if "%PLATFORM%"=="win_32" (
    set ARCH=x86
    %CURL% %LINK86% --output DesktopEditors_x86.exe /w
    start DesktopEditors_x86.exe /silent /w
    copy "%SystemRoot%\System32\vcruntime140.dll" %OUTPATH%
    copy "%SystemRoot%\System32\msvcp140.dll" %OUTPATH%
) else if "%PLATFORM%"=="win_64" (
    set ARCH=x64
    %CURL% %LINK64% --output DesktopEditors_x64.exe /w
    start /w DesktopEditors_x64.exe /silent
    copy "%SystemRoot%\System32\vcruntime140.dll" %OUTPATH%
    copy "%SystemRoot%\System32\msvcp140.dll" %OUTPATH%
) else (
    exit
)

node create_portable.js %PLATFORM% %ORG% %PACKAGE%
