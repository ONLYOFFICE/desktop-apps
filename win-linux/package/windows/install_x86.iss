#include "common.iss"
#define os_arch 'win_32'


[Setup]
AppName                 ={#sAppName}
AppVersion              ={#sAppVersion}
#ifdef _IVOLGA_PRO
  OutputBaseFileName    =Ivolgapro_x86
#elif defined(_AVS)
  #ifdef _AVS_LIGHT_VERSION
    OutputBaseFilename  ={#sShortAppName}_x86_light
  #else
    OutputBaseFilename  ={#sShortAppName}_x86
  #endif
#else
  OutputBaseFileName    =DesktopEditors_x86
#endif
MinVersion              =0,5.0.2195
;ArchitecturesAllowed    =x86


[Files]
Source: data\vcredist\vcredist_x86.exe;       DestDir: {app}\; Flags: deleteafterinstall; \
    AfterInstall: installVCRedist(ExpandConstant('{app}\vcredist_x86.exe'), ExpandConstant('{cm:InstallAdditionalComponents}')); Check: not checkVCRedist;

Source: ..\..\..\..\core\build\bin\windows\x2t32.exe;             DestDir: {app}\converter; Flags: ignoreversion;
Source: ..\..\..\..\core\build\bin\windows\icudt.dll;             DestDir: {app}\converter; Flags: ignoreversion;
Source: ..\..\..\..\core\build\bin\icu\{#os_arch}\*;              DestDir: {app}\converter; Flags: ignoreversion; Excludes: *.lib;
Source: ..\..\..\..\core\build\lib\{#os_arch}\*;                      DestDir: {app}\converter; Excludes: *.lib,HtmlFileInternal.exe,ascdocumentscore.dll; Flags: ignoreversion;
Source: ..\..\..\..\core\build\lib\{#os_arch}\HtmlFileInternal.exe;   DestDir: {app}\; Flags: ignoreversion;
Source: ..\..\..\..\core\build\lib\{#os_arch}\ascdocumentscore.dll;   DestDir: {app}\; Flags: ignoreversion;

Source: ..\..\..\..\core\build\cef\{#os_arch}\*;                      DestDir: {app}\; Excludes: *.lib; Flags: ignoreversion recursesubdirs;
#ifdef _IVOLGA_PRO
Source: data\projicons_nct.exe;                           DestDir: {app}\; DestName: {#iconsExe};
#elif defined(_AVS)
Source: data\projicons_omt.exe;                           DestDir: {app}\; DestName: {#iconsExe};
#else
Source: data\projicons_asc.exe;                           DestDir: {app}\; DestName: {#iconsExe};
#endif
Source: data\libs\qt\win32\*;                             DestDir: {app}\; Flags: ignoreversion recursesubdirs;
Source: data\libs\chromium\win_xp\dbghelp.dll;            DestDir: {app}\; Flags: onlyifdoesntexist; OnlyBelowVersion: 6.0;
Source: data\libs\chromium\win32\dbghelp.dll;             DestDir: {app}\; Flags: onlyifdoesntexist; MinVersion: 6.0; Check: libExists('dbghelp.dll');


