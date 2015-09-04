#include "common.iss"


[Setup]
AppName                 ={#sAppName}
AppVersion              ={#sAppVersion}
OutputBaseFileName      =DesktopEditors_x64
MinVersion              =0,5.0.2195
ArchitecturesAllowed    =x64
ArchitecturesInstallIn64BitMode=x64
;ShowLanguageDialog      = false
;ShowUndisplayableLanguages = true

[Code]

[Files]
Source: data\vcredist\vcredist_x64.exe;       DestDir: {app}\; Flags: deleteafterinstall; \
    AfterInstall: installVCRedist(ExpandConstant('{app}\vcredist_x64.exe'), ExpandConstant('{cm:InstallAdditionalComponents}')); Check: not checkVCRedist;

Source: ..\build\Release\release\DesktopEditors.exe; DestDir: {app}; 

Source: ..\libs\ChromiumBasedEditors\app\corebuilds\win64\ascdocumentscore.dll;  DestDir: {app}\; Flags: ignoreversion;
Source: ..\libs\ChromiumBasedEditors\app\cefbuilds\win64\*;                      DestDir: {app}\; Excludes: *.lib; Flags: ignoreversion recursesubdirs;
Source: data\libs\qt\win64\*;                               DestDir: {app}\; Flags: ignoreversion recursesubdirs;

;
; some files placed in common.iss
;