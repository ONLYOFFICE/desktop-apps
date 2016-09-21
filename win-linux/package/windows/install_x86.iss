#include "common.iss"
#define os_arch 'win_32'


[Setup]
OutputBaseFileName    =DesktopEditors_x86
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
Source: data\libs\qt\win32\*;                                         DestDir: {app}\; Flags: ignoreversion recursesubdirs;
Source: ..\..\3dparty\WinSparkle\win_32\WinSparkle.dll;               DestDir: {app}\; Flags: ignoreversion;


