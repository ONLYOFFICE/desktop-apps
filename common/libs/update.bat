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

set trunk_path=svn://192.168.3.15/activex/AVS/Sources/TeamlabOffice/trunk
set export_path=..\converter\windows
reg Query "HKLM\Hardware\Description\System\CentralProcessor\0" | find /i "x86" > NUL && set OS=32BIT || set OS=64BIT
if %OS%==32BIT (
    set os_path=win32

    md %export_path%\%os_path%

    svn export --force %trunk_path%/AsyncServerComponents/Bin/Windows/x2t32.exe %export_path%/%os_path%
    svn export --force %trunk_path%/ServerComponents/SDK/lib/win_32 %export_path%/%os_path%
) else (
    set os_path=win64

    md %export_path%\%os_path%

    svn export --force %trunk_path%/AsyncServerComponents/Bin/Windows/x2t.exe %export_path%/%os_path%
    svn export --force %trunk_path%/ServerComponents/SDK/lib/win_64/ %export_path%/%os_path%
)

svn export --force %trunk_path%/ServerComponents/SDK/bin/windows/icudt.dll %export_path%
svn export --force %trunk_path%/ServerComponents/UnicodeConverter/icubuilds/%os_path%/bin/icudt55.dll %export_path%/%os_path%
svn export --force %trunk_path%/ServerComponents/UnicodeConverter/icubuilds/%os_path%/bin/icuuc55.dll %export_path%/%os_path%

rd /s /q %export_path%\%os_path%\debug
del %export_path%\%os_path%\*.lib %export_path%\%os_path%\html*.exe