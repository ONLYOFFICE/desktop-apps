if not exist "node_modules/generate-evb" (
    call npm install generate-evb
)
if "%PACKAGE%"=="" ( 
    set PACKAGE=DesktopEditors
)

set ORG=onlyoffice
set OUTPATH=--output "%~dp0..\..\..\..\build_tools\out\%PLATFORM%\%ORG%\%PACKAGE%"
set CURL=curl -L

echo "%~dp0..\..\..\..\build_tools\out\%PLATFORM%\%ORG%\%PACKAGE%"
if "%PLATFORM%"=="win_32" (
    set ARCH=x86
    %CURL% https://github.com/eymeen/MSVC.dll/raw/main/32/msvcp140.dll %OUTPATH%\msvcp140.dll
    %CURL% https://github.com/heemskerkerik/vcruntime140/raw/master/runtimes/win-x86/native/vcruntime140.dll %OUTPATH%\vcruntime140.dll
) else if "%PLATFORM%"=="win_64" (
    set ARCH=x64
    %CURL% https://github.com/eymeen/MSVC.dll/raw/main/64/msvcp140.dll %OUTPATH%\msvcp140.dll
    %CURL% https://github.com/manojvsp12/FineCash/raw/master/windows/vcruntime140.dll %OUTPATH%\vcruntime140.dll
    %CURL% http://github.com/manojvsp12/FineCash/raw/master/windows/vcruntime140_1.dll %OUTPATH%\vcruntime140_1.dll
) else (
    exit
)

node create_portable.js %PLATFORM% %ORG% %PACKAGE%
%enigmavbconsole% DesktopEditorsPortable.evb
