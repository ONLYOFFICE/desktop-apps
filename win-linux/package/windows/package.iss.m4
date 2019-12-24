changequote(', ')dnl
changecom(';',)dnl
#define sCompanyName                "M4_COMPANY_NAME"
#define sProductName                "M4_PRODUCT_NAME"
#define sProductNameShort           "M4_PRODUCT_NAME_SHORT"
#define sPublisherName              "M4_PUBLISHER_NAME"
#define sPublisherURL               "M4_PUBLISHER_URL"
#define sSupportMail                "M4_SUPPORT_MAIL"
#define sSupportURL                 "M4_SUPPORT_URL"
#define sPackageName                "M4_PACKAGE_NAME"
#define sPackageVersion             "M4_PACKAGE_VERSION"
#define sWinArch                    "M4_WIN_ARCH"
#define sWinXP                      "M4_WIN_XP_FLAG"

#if str(sWinArch) == x64
  #define sArch                     "64"
  #define _WIN64
#endif
#if str(sWinArch) == x86
  #define sArch                     "32"
#endif
#ifdef sWinXP
  #define _WIN_XP
#endif
#define CORE_DIR                    "..\..\..\..\core\build"
#define PATH_PREFIX                 "win_{#sArch}\build"
#define VC_REDIST_VER               "vcredist_{#sWinArch}.exe"

#include "common.iss"

[Setup]
#ifndef _WIN_XP
OutputBaseFileName                = "{#sPackageName}_{#sWinArch}"
MinVersion                        = 6.1
#else
OutputBaseFileName                = "{#sPackageName}_{#sWinArch}_xp"
MinVersion                        = 5.0
;MinVersion                       = 5.0.2195
OnlyBelowVersion                  = 6.1
#endif
#if str(sWinArch) == x64
ArchitecturesAllowed              = x64
ArchitecturesInstallIn64BitMode   = x64
;ShowUndisplayableLanguages       = true
;UsePreviousLanguage=no
#endif
#if str(sWinArch) == x86
;ArchitecturesAllowed             = x86
#endif

[Files]
#ifdef _WIN_XP
Source: data\vcredist\vcredist_{#sWinArch}.exe;                    DestDir: {app}\; Flags: deleteafterinstall; \
AfterInstall: installVCRedist(ExpandConstant("{app}\{#VC_REDIST_VER}"), ExpandConstant("{cm:InstallAdditionalComponents}")); Check: not checkVCRedist2015;

  #ifndef SCRIPT_CUSTOM_FILES
Source: {#CORE_DIR}\{#PATH_PREFIX}\bin\win_{#sArch}\x2t.exe;       DestDir: {app}\converter; Flags: ignoreversion;
Source: {#CORE_DIR}\bin\win_{#sArch}\icudt.dll;                    DestDir: {app}\converter; Flags: ignoreversion;
Source: {#CORE_DIR}\bin\icu\win_{#sArch}\*;                        DestDir: {app}\converter; Flags: ignoreversion; Excludes: *.lib;

Source: {#CORE_DIR}\cef\winxp_{#sArch}\*;                          DestDir: {app}\; Excludes: *.lib; Flags: ignoreversion recursesubdirs;
Source: data\libs\qt\win{#sArch}\*;                                DestDir: {app}\; Flags: ignoreversion recursesubdirs;
Source: ..\..\3dparty\WinSparkle\win_{#sArch}\WinSparkle.dll;      DestDir: {app}\; Flags: ignoreversion;
  #endif
#endif
