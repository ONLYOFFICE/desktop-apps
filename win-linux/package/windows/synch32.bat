SET AdvancedInstaller = %~dp0\..\..\..\..\bin\x86\AdvancedInstaller.com 
SET Path = %~dp0..\..\..\..\desktop\build_tools\out\win_32\onlyoffice\DesktopEditors
SET File = %cd%
%~dp0..\..\..\..\bin\x86\AdvancedInstaller.com  /edit "Sample.aip" /AddOsLc -buildname DefaultBuild -arch x86
%AdvancedInstaller% /edit "Sample.aip" /NewSync APPDIR "Path"


%AdvancedInstaller% /edit "DesktopEditors.aip" /AddOsLc -buildname DefaultBuild -arch x86


"C:\Program Files (x86)\Caphyon\Advanced Installer 18.7\bin\x86\AdvancedInstaller.com" /edit "DesktopEditors.aip" /NewSync APPDIR "C:\Users\Egor.Bespalov\Desktop\desktop\build_tools\out\win_32\onlyoffice\DesktopEditors"


