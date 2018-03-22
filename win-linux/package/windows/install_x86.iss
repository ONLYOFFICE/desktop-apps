
#define os_arch 'win_32'
#define PATH_PREFIX 'win_32\build'
#include "common.iss"


[Setup]
OutputBaseFileName    =DesktopEditors_x86
MinVersion            =6.1
;ArchitecturesAllowed    =x86


[Files]
Source: data\vcredist\vcredist_x86.exe;       DestDir: {app}\; Flags: deleteafterinstall; \
    AfterInstall: installVCRedist(ExpandConstant('{app}\vcredist_x86.exe'), ExpandConstant('{cm:InstallAdditionalComponents}')); Check: not checkVCRedist;

Source: ..\..\..\..\core\build\{#PATH_PREFIX}\bin\win_32\x2t.exe;     DestDir: {app}\converter; Flags: ignoreversion;
Source: ..\..\..\..\core\build\bin\win_32\icudt.dll;                  DestDir: {app}\converter; Flags: ignoreversion;
Source: ..\..\..\..\core\build\bin\icu\{#os_arch}\*;                  DestDir: {app}\converter; Flags: ignoreversion; Excludes: *.lib;

Source: ..\..\..\..\core\build\cef\{#os_arch}\*;                      DestDir: {app}\; Excludes: *.lib; Flags: ignoreversion recursesubdirs;
Source: data\libs\qt\win32\*;                                         DestDir: {app}\; Flags: ignoreversion recursesubdirs;
Source: ..\..\3dparty\WinSparkle\win_32\WinSparkle.dll;               DestDir: {app}\; Flags: ignoreversion;


