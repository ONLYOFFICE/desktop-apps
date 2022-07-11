::if variable isn't defined
if "%ORG%"=="" (set ORG=onlyoffice)
if "%PACKAGE%"=="" (SET PACKAGE=DesktopEditors)
::check arch
if "%PLATFORM%"=="win_32" (
set ARCH=x86
%AdvancedInstaller% /edit "DesktopEditors.aip" /SetPackageType x86
%AdvancedInstaller% /edit "DesktopEditors.aip" /SetAppdir -buildname DefaultBuild -path [ProgramFilesFolder][MANUFACTURER_INSTALL_FOLDER]\[PRODUCT_INSTALL_FOLDER]
%AdvancedInstaller% /edit "DesktopEditors.aip" /DelPrerequisite "Microsoft Visual C++ 2015-2022 Redistributable (x64)"
)else if "%PLATFORM%"=="win_64"(
set ARCH=x64
%AdvancedInstaller% /edit "DesktopEditors.aip" /DelPrerequisite "Microsoft Visual C++ 2015-2022 Redistributable (x86)"
) else(
exit
)

%AdvancedInstaller% /edit "DesktopEditors.aip" /AddOsLc -buildname DefaultBuild -arch %ARCH%
%AdvancedInstaller% /edit "DesktopEditors.aip" /NewSync APPDIR "%~dp0..\..\..\..\build_tools\out\%PLATFORM%\%ORG%\%PACKAGE%"
%AdvancedInstaller% /build DesktopEditors.aip
