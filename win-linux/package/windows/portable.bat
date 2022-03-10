if not exist "node_modules/generate-evb" (
    call npm install generate-evb
)
set ORG=onlyoffice
if "%PACKAGE%"=="" ( 
    set PACKAGE=DesktopEditors
)
if "%PLATFORM%"=="win_32" (
    set ARCH=x86
) else if "%PLATFORM%"=="win_64" (
    set ARCH=x64
) else (
    exit
)
node create_portable.js
%enigmavbconsole% DesktopEditorsPortable.evb
