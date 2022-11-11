; -- Installer Help --

#if str(_ARCH) == "64"
  #define sWinArch "x64"
  #define sPlatform "win_64"
#elif str(_ARCH) == "32"
  #define sWinArch "x86"
  #define sPlatform "win_32"
#endif
#ifndef _WIN_XP
  #define sWinArchFull sWinArch
  #define sPlatformFull sPlatform
#else
  #define sWinArchFull sWinArch + "_xp"
  #define sPlatformFull sPlatform + "_xp"
#endif

#ifndef sBrandingFolder
  #define sBrandingFolder "..\..\.."
#endif

#include sBrandingFolder + "\win-linux\package\windows\defines.iss"

#ifndef sAppVersion
  #define sAppVersion "0.0.0.0"
#endif
#define sAppVerShort Copy(sAppVersion, 0, 3)

#ifndef sOutputFileName
  #define sOutputFileName str(sPackageHelpName + "_" + sAppVersion + "_" + sWinArchFull)
#endif

[Setup]
AppName={#sAppHelpName}
AppVersion={#sAppVersion}
AppVerName={#sAppHelpName} {#sAppVerShort}
AppCopyright={#sAppCopyright}
; AppMutex=
AppPublisher={#sAppPublisher}
AppPublisherURL={#sAppPublisherURL}
AppSupportURL={#sAppSupportURL}
; AppComments=

PrivilegesRequired=admin

AlwaysShowDirOnReadyPage=yes
DefaultDirName={code:GetInstallPath}
DisableDirPage=yes
DirExistsWarning=no

SetupIconFile={#sBrandingFolder}\win-linux\extras\projicons\res\desktopeditors.ico

WizardImageFile={#sBrandingFolder}\win-linux\package\windows\data\dialogpicture*.bmp
WizardSmallImageFile={#sBrandingFolder}\win-linux\package\windows\data\dialogicon*.bmp

UninstallDisplayName={#sAppHelpName} {#sAppVerShort} ({#sWinArch})
UninstallDisplayIcon={app}\app.ico

VersionInfoVersion={#sAppVersion}

OutputDir=.\
OutputBaseFileName={#sOutputFileName}

#if str(_ARCH) == "64"
ArchitecturesInstallIn64BitMode=x64
ArchitecturesAllowed=x64
#endif

#ifdef ENABLE_SIGNING
SignTool=byparam $p
#endif

SolidCompression=yes
Compression=lzma2/ultra64
LZMAUseSeparateProcess=yes

[Languages]
#ifdef _ONLYOFFICE
Name: en;    MessagesFile: compiler:Default.isl;
Name: be;    MessagesFile: compiler:Languages\Belarusian.isl;
Name: bg;    MessagesFile: compiler:Languages\Bulgarian.isl;
Name: ca;    MessagesFile: compiler:Languages\Catalan.isl;
Name: cs;    MessagesFile: compiler:Languages\Czech.isl;
Name: da;    MessagesFile: compiler:Languages\Danish.isl;
Name: de;    MessagesFile: compiler:Languages\German.isl;
Name: el;    MessagesFile: compiler:Languages\Greek.isl;
Name: es;    MessagesFile: compiler:Languages\Spanish.isl;
Name: fi;    MessagesFile: compiler:Languages\Finnish.isl;
Name: fr;    MessagesFile: compiler:Languages\French.isl;
Name: gl;    MessagesFile: compiler:Languages\Galician.isl;
Name: hu;    MessagesFile: compiler:Languages\Hungarian.isl;
Name: hy_AM; MessagesFile: compiler:Languages\Armenian.isl;
Name: id;    MessagesFile: compiler:Languages\Indonesian.isl;
Name: it_IT; MessagesFile: compiler:Languages\Italian.isl;
Name: ja;    MessagesFile: compiler:Languages\Japanese.isl;
Name: ko;    MessagesFile: compiler:Languages\Korean.isl;
Name: lo;    MessagesFile: compiler:Default.isl;
Name: lv;    MessagesFile: compiler:Languages\Latvian.isl;
Name: nl;    MessagesFile: compiler:Languages\Dutch.isl;
Name: nn_NO; MessagesFile: compiler:Languages\Norwegian.isl;
Name: pl;    MessagesFile: compiler:Languages\Polish.isl;
Name: pt_BR; MessagesFile: compiler:Languages\BrazilianPortuguese.isl;
Name: pt_PT; MessagesFile: compiler:Languages\Portuguese.isl;
Name: ro;    MessagesFile: compiler:Languages\Romanian.isl;
Name: ru;    MessagesFile: compiler:Languages\Russian.isl;
Name: sk;    MessagesFile: compiler:Languages\Slovak.isl;
Name: sl;    MessagesFile: compiler:Languages\Slovenian.isl;
Name: sv;    MessagesFile: compiler:Languages\Swedish.isl;
Name: tr;    MessagesFile: compiler:Languages\Turkish.isl;
Name: uk;    MessagesFile: compiler:Languages\Ukrainian.isl;
Name: vi;    MessagesFile: compiler:Languages\Vietnamese.isl;
Name: zh_CN; MessagesFile: compiler:Languages\ChineseSimplified.isl;
#else
Name: ru;    MessagesFile: compiler:Languages\Russian.isl;
Name: en;    MessagesFile: compiler:Default.isl;
#endif

[Files]
Source: "{#DEPLOY_PATH}\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs

[Code]
var
  InstallPath: string;

function GetHKLM: Integer;
begin
  if IsWin64 then
    Result := HKLM64
  else
    Result := HKLM32;
end;

function GetInstallPath(Param: string): string;
begin
  RegQueryStringValue(GetHKLM(), '{#APP_REG_PATH}', 'AppPath', InstallPath);
  Result := InstallPath;
end;

function InitializeSetup(): Boolean;
begin
  Result := True;
  if not RegValueExists(GetHKLM(), '{#APP_REG_PATH}', 'AppPath') then
  begin
    Result := False;
    MsgBox('Error {#APP_REG_PATH}\AppPath not found', mbError, MB_OK);
  end;
end;
