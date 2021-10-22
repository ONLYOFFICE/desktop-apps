::if variable isn't defined
if "%ORG%"=="" (set ORG=onlyoffice)
if "%PACKAGE%"=="" (SET PACKAGE=DesktopEditors)
::check arch
if "%PLATFORM%"=="win_32" (
set ARCH=x86
%AdvancedInstaller% /edit "DesktopEditors.aip" /SetPackageType x86
)else if "%PLATFORM%"=="win_64"(
set ARCH=x64
) else(
exit
)

%AdvancedInstaller% /edit "DesktopEditors.aip" /AddOsLc -buildname DefaultBuild -arch %ARCH%
%AdvancedInstaller% /edit "DesktopEditors.aip" /NewSync APPDIR "%~dp0..\..\..\..\build_tools\out\%PLATFORM%\%ORG%\%PACKAGE%"
%AdvancedInstaller% /build DesktopEditors.aip
