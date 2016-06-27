
#define os_arch 'win_64'
#define _WIN64

#include "common.iss"

[Setup]
#ifdef _ONLY_RU
  OutputBaseFileName    =DesktopEditors_onru_x64
#elif defined(_AVS)
#else
  OutputBaseFileName    =DesktopEditors_x64
#endif
MinVersion              =0,5.0.2195
;MinVersion              =6.0
ArchitecturesAllowed    =x64
ArchitecturesInstallIn64BitMode=x64
;ShowUndisplayableLanguages = true
;UsePreviousLanguage=no


[Files]
Source: data\vcredist\vcredist_x64.exe;       DestDir: {app}\; Flags: deleteafterinstall; \
    AfterInstall: installVCRedist(ExpandConstant('{app}\vcredist_x64.exe'), ExpandConstant('{cm:InstallAdditionalComponents}')); Check: not checkVCRedist;

Source: ..\..\..\..\core\build\bin\windows\x2t.exe;               DestDir: {app}\converter; Flags: ignoreversion;
Source: ..\..\..\..\core\build\bin\windows\icudt.dll;             DestDir: {app}\converter; Flags: ignoreversion;
Source: ..\..\..\..\core\build\bin\icu\{#os_arch}\*;              DestDir: {app}\converter; Flags: ignoreversion; Excludes: *.lib;
Source: ..\..\..\..\core\build\lib\{#os_arch}\*;                      DestDir: {app}\converter; Excludes: *.lib,HtmlFileInternal.exe,ascdocumentscore.dll; Flags: ignoreversion;
Source: ..\..\..\..\core\build\lib\{#os_arch}\HtmlFileInternal.exe;   DestDir: {app}\; Flags: ignoreversion;
Source: ..\..\..\..\core\build\lib\{#os_arch}\ascdocumentscore.dll;   DestDir: {app}\; Flags: ignoreversion;

Source: ..\..\..\..\core\build\cef\{#os_arch}\*;                      DestDir: {app}\; Excludes: *.lib; Flags: ignoreversion recursesubdirs;
Source: data\libs\qt\win64\*;                                     DestDir: {app}\; Flags: ignoreversion recursesubdirs;
Source: ..\..\3dparty\WinSparkle\win_64\WinSparkle.dll;           DestDir: {app}\; Flags: ignoreversion;
;Source: data\libs\chromium\win64\dbghelp.dll;               DestDir: {app}\; Flags: onlyifdoesntexist; Check: libExists('dbghelp.dll');

;
; some files placed in common.iss
;