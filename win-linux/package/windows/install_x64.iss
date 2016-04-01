#include "common.iss"
#define os_arch 'win_64'

[Setup]
AppName                 ={#sAppName}
AppVersion              ={#sAppVersion}
#ifdef _IVOLGA_PRO
OutputBaseFileName      =Ivolgapro_x64
#else
OutputBaseFileName      =DesktopEditors_x64
#endif
;MinVersion              =0,5.0.2195
MinVersion              =6.0
ArchitecturesAllowed    =x64
ArchitecturesInstallIn64BitMode=x64
;ShowUndisplayableLanguages = true
;UsePreviousLanguage=no

[Code]

[Files]
Source: data\vcredist\vcredist_x64.exe;       DestDir: {app}\; Flags: deleteafterinstall; \
    AfterInstall: installVCRedist(ExpandConstant('{app}\vcredist_x64.exe'), ExpandConstant('{cm:InstallAdditionalComponents}')); Check: not checkVCRedist;

Source: ..\..\..\..\core\build\bin\windows\x2t.exe;               DestDir: {app}\converter; Flags: ignoreversion;
Source: ..\..\..\..\core\build\bin\windows\icudt.dll;             DestDir: {app}\converter; Flags: ignoreversion;
Source: ..\..\..\..\core\build\bin\icu\{#os_arch}\*;              DestDir: {app}\converter; Flags: ignoreversion; Excludes: *.lib;
Source: ..\..\..\..\core\build\lib\{#os_arch}\*;                      DestDir: {app}\converter; Excludes: *.lib HtmlFileInternal.exe ascdocumentscore.dll; Flags: ignoreversion;
Source: ..\..\..\..\core\build\lib\{#os_arch}\HtmlFileInternal.exe;   DestDir: {app}\; Flags: ignoreversion;
Source: ..\..\..\..\core\build\lib\{#os_arch}\ascdocumentscore.dll;   DestDir: {app}\; Flags: ignoreversion;

Source: ..\..\..\..\core\build\cef\{#os_arch}\*;                      DestDir: {app}\; Excludes: *.lib; Flags: ignoreversion recursesubdirs;
#ifdef _IVOLGA_PRO
Source: data\projicons_nct.exe;                             DestDir: {app}\; DestName: {#iconsExe};
#else
Source: data\projicons_asc.exe;                             DestDir: {app}\; DestName: {#iconsExe};
#endif
Source: data\libs\qt\win64\*;                               DestDir: {app}\; Flags: ignoreversion recursesubdirs;
Source: data\libs\chromium\win64\dbghelp.dll;               DestDir: {app}\; Flags: onlyifdoesntexist; Check: libExists('dbghelp.dll');

;
; some files placed in common.iss
;