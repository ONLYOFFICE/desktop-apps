@echo off
set LibsPath=svn://192.168.3.15/activex/AVS/Sources/TeamlabOffice/trunk/ServerComponents/DesktopEditor

if exist ".svn" (
    svn update
) else (
    svn checkout %LibsPath% ./. --depth empty
    svn update --set-depth infinity ChromiumBasedEditors2
    svn update --set-depth infinity common
    svn update --set-depth infinity Word_Api
)

reg Query "HKLM\Hardware\Description\System\CentralProcessor\0" | find /i "x86" > NUL && set OS=32BIT || set OS=64BIT
%OS%==32BIT && set OSNAME=win32 || set OSNAME=win64

set trunk_path=svn://192.168.3.15/activex/AVS/Sources/TeamlabOffice/trunk
set export_path=..\converter\windows
if %OS%==32BIT (
    md %export_path%\%OSNAME%

    svn export --force %trunk_path%/AsyncServerComponents/Bin/Windows/x2t32.exe %export_path%\%OSNAME%
    svn export --force %trunk_path%/ServerComponents/SDK/lib/win_32 %export_path%\%OSNAME%
) else (
    md %export_path%\%OSNAME%

    svn export --force %trunk_path%/AsyncServerComponents/Bin/Windows/x2t.exe %export_path%\%OSNAME%
    svn export --force %trunk_path%/ServerComponents/SDK/lib/win_64/ %export_path%\%OSNAME%
)

svn export --force %trunk_path%/ServerComponents/SDK/bin/windows/icudt.dll %export_path%
svn export --force %trunk_path%/ServerComponents/UnicodeConverter/icubuilds/%OSNAME%/bin/icudt55.dll %export_path%\%OSNAME%
svn export --force %trunk_path%/ServerComponents/UnicodeConverter/icubuilds/%OSNAME%/bin/icuuc55.dll %export_path%\%OSNAME%

rd /s /q %export_path%\%OSNAME%\debug
del %export_path%\%OSNAME%\*.lib %export_path%\%OSNAME%\html*.exe