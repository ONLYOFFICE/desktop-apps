@echo off
set LibsPath=svn://fileserver/activex/AVS/Sources/TeamlabOffice/trunk/ServerComponents/DesktopEditor/ChromiumBasedEditors

if exist ".svn" (
    TortoiseProc.exe /command:update /path:.\ /closeonend:0
) else (
    TortoiseProc.exe /command:checkout /path:.\ /url:%LibsPath% /closeonend:0
)
