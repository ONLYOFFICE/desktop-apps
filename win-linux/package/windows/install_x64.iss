
#define os_arch 'win_64'
#define _WIN64
#define PATH_PREFIX 'win_64\build'
#define VC_REDIST_VER 'vcredist_x64.exe'

#include "common.iss"

[Setup]
OutputBaseFileName    =DesktopEditors_x64
MinVersion              =6.1
ArchitecturesAllowed    =x64
ArchitecturesInstallIn64BitMode=x64
;ShowUndisplayableLanguages = true
;UsePreviousLanguage=no


[Files]
#ifndef SCRIPT_CUSTOM_FILES
Source: data\libs\qt\win64\*;                                     DestDir: {app}\; Flags: ignoreversion recursesubdirs;
#endif
