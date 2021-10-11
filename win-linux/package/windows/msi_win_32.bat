::change to 32-bit
%AdvancedInstaller% /edit "DesktopEditors.aip" /AddOsLc -buildname DefaultBuild -arch x86

:synchronization 32-bit folder
%AdvancedInstaller% /edit "DesktopEditors.aip" /NewSync APPDIR "%~dp0..\..\..\..\build_tools\out\win_32\onlyoffice\DesktopEditors"




start DesktopEditors.aip
