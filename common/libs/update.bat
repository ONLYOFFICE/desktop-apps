@echo off
rem set LibsPath=svn://fileserver/activex/AVS/Sources/TeamlabOffice/trunk/ServerComponents/DesktopEditor/ChromiumBasedEditors
set LibsPath=svn://fileserver/activex/AVS/Sources/TeamlabOffice/trunk/ServerComponents/DesktopEditor

if exist ".svn" (
    TortoiseProc.exe /command:update /path:.\ /closeonend:0
) else (
    TortoiseProc.exe /command:checkout /blockpathadjustments /path:.\ /url:%LibsPath% /closeonend:0
)
