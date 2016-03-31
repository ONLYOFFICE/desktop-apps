#include "common.iss"


[Setup]
AppName                 ={#sAppName}
AppVersion              ={#sAppVersion}
#ifdef _IVOLGA_PRO
OutputBaseFileName      =Ivolgapro_x86
#else
OutputBaseFileName      =DesktopEditors_x86
#endif
MinVersion              =0,5.0.2195
;ArchitecturesAllowed    =x86


[Files]
Source: data\vcredist\vcredist_x86.exe;       DestDir: {app}\; Flags: deleteafterinstall; \
    AfterInstall: installVCRedist(ExpandConstant('{app}\vcredist_x86.exe'), ExpandConstant('{cm:InstallAdditionalComponents}')); Check: not checkVCRedist;

;Source: ..\..\common\converter\windows\win32\*;           DestDir: {app}\converter; Flags: recursesubdirs ignoreversion;
Source: ..\..\common\libs\converter\win_32\*;                    DestDir: {app}\converter; Excludes: HtmlFileInternal.exe; Flags: recursesubdirs;
Source: ..\..\common\libs\converter\win_32\HtmlFileInternal.exe; DestDir: {app}\; Flags: recursesubdirs;

Source: ..\..\common\libs\core\win_32\ascdocumentscore.dll;  DestDir: {app}\; Flags: ignoreversion;
Source: ..\..\common\libs\cef\win_32\*;                      DestDir: {app}\; Excludes: *.lib; Flags: ignoreversion recursesubdirs;
#ifdef _IVOLGA_PRO
Source: data\projicons_nct_x86.exe;                       DestDir: {app}\; DestName: {#iconsExe};
#else
Source: data\projicons_asc_x86.exe;                       DestDir: {app}\; DestName: {#iconsExe};
#endif
Source: data\libs\qt\win32\*;                             DestDir: {app}\; Flags: ignoreversion recursesubdirs;
Source: data\libs\chromium\win_xp\dbghelp.dll;            DestDir: {app}\; Flags: onlyifdoesntexist; OnlyBelowVersion: 6.0;
Source: data\libs\chromium\win32\dbghelp.dll;             DestDir: {app}\; Flags: onlyifdoesntexist; MinVersion: 6.0; Check: libExists('dbghelp.dll');


