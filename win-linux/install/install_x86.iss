#include "common.iss"


[Setup]
AppName                 ={#sAppName}
AppVersion              ={#sAppVersion}
OutputBaseFileName      =DesktopEditors_x86
MinVersion              =0,5.0.2195
;ArchitecturesAllowed    =x86


[Files]
Source: data\vcredist\vcredist_x86.exe;       DestDir: {app}\; Flags: deleteafterinstall; \
    AfterInstall: installVCRedist(ExpandConstant('{app}\vcredist_x86.exe'), ExpandConstant('{cm:InstallAdditionalComponents}')); Check: not checkVCRedist;

Source: ..\build\Release\release\DesktopEditors.exe;     DestDir: {app}; 

Source: ..\libs\ChromiumBasedEditors\app\corebuilds\win32\ascdocumentscore.dll;  DestDir: {app}\; Flags: ignoreversion;
Source: ..\libs\ChromiumBasedEditors\app\cefbuilds\win32\*;                      DestDir: {app}\; Excludes: *.lib; Flags: ignoreversion recursesubdirs;
Source: data\libs\qt\win32\*;                               DestDir: {app}\; Flags: ignoreversion recursesubdirs;


