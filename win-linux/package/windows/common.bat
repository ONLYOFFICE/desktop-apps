::if variable isn't defined
IF "%ORG%"=="" (SET ORG=onlyoffice)
IF "%PACKAGE%"=="" (SET PACKAGE=DesktopEditors)
::check arch
IF "%PLATFORM%"=="win_32" (
SET ARCH=x86
)else if "%PLATFORM%"=="win_64"(
SET ARCH=x64
) else(
EXIT
)

%AdvancedInstaller% /edit "DesktopEditors.aip" /AddOsLc -buildname DefaultBuild -arch %ARCH%
%AdvancedInstaller% /edit "DesktopEditors.aip" /NewSync APPDIR "%~dp0..\..\..\..\build_tools\out\%PLATFORM%\%ORG%\%PACKAGE%"
%AdvancedInstaller% /build DesktopEditors.aip
