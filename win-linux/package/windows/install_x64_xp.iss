
#define os_arch 'win_64'
#define _WIN64
#define PATH_PREFIX 'win_64\build'
#define _WIN_XP
#define VC_REDIST_VER 'vcredist_x64.exe'
#include "common.iss"

[Setup]
OutputBaseFileName    =DesktopEditors_x64_xp
MinVersion              =5.0
OnlyBelowVersion        =6.1
ArchitecturesAllowed    =x64
ArchitecturesInstallIn64BitMode=x64
;ShowUndisplayableLanguages = true
;UsePreviousLanguage=no


[Files]
Source: data\vcredist\vcredist_x64.exe;       DestDir: {app}\; Flags: deleteafterinstall; \
    AfterInstall: installVCRedist(ExpandConstant('{app}\vcredist_x64.exe'), ExpandConstant('{cm:InstallAdditionalComponents}')); Check: not checkVCRedist;

#ifndef SCRIPT_CUSTOM_FILES
Source: ..\..\..\..\core\build\{#PATH_PREFIX}\bin\win_64\x2t.exe; DestDir: {app}\converter; Flags: ignoreversion;
Source: ..\..\..\..\core\build\bin\win_64\icudt.dll;              DestDir: {app}\converter; Flags: ignoreversion;
Source: ..\..\..\..\core\build\bin\icu\{#os_arch}\*;              DestDir: {app}\converter; Flags: ignoreversion; Excludes: *.lib;

Source: ..\..\..\..\core\build\cef\winxp_64\*;                    DestDir: {app}\; Excludes: *.lib; Flags: ignoreversion recursesubdirs;
Source: data\libs\qt\win64\*;                                     DestDir: {app}\; Flags: ignoreversion recursesubdirs;
Source: ..\..\3dparty\WinSparkle\win_64\WinSparkle.dll;           DestDir: {app}\; Flags: ignoreversion;
#endif
