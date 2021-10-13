%AdvancedInstaller% /edit "DesktopEditors.aip" /NewSync APPDIR "%~dp0..\..\..\..\build_tools\out\%PLATFORM%\%ORG%\%PACKAGE%"
%AdvancedInstaller% /edit "DesktopEditors.aip" /AddOsLc -buildname DefaultBuild -arch %ARCH%
%AdvancedInstaller% /build DesktopEditors.aip