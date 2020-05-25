; -- Update Common --

#if str(_ARCH) == "64"
  #define sWinArch                      "x64"
  #define sPlatform                     "win_64"
#elif str(_ARCH) == "32"
  #define sWinArch                      "x86"
  #define sPlatform                     "win_32"
#endif
#ifndef _WIN_XP
  #define sWinArchFull                  sWinArch
  #define sPlatformFull                 sPlatform
#else
  #define sWinArchFull                  sWinArch + "_xp"
  #define sPlatformFull                 sPlatform + "_xp"
#endif

#ifndef sBrandingFolder
  #define sBrandingFolder               "..\..\.."
#endif

#include sBrandingFolder + "\win-linux\package\windows\defines.iss"

#ifndef sAppVersion
  #define sAppVersion                   GetFileVersion(AddBackslash(DEPLOY_PATH) + NAME_EXE_OUT)
#endif
#define sAppVerShort                    Copy(sAppVersion, 0, 3)

#define TARGET_NAME                     str(sPackageName + "_" + sAppVersion + "_" + sWinArchFull + ".exe")

#ifndef sOutputFileName
  #define sOutputFileName               str(sPackageName + "_update_" + sAppVersion + "_" + sWinArchFull)
#endif

[Setup]
AppName                   ={#sAppName}
AppVerName                ={#sAppName} {#sAppVerShort}
AppVersion                ={#sAppVersion}
VersionInfoVersion        ={#sAppVersion}

AppPublisher              ={#sAppPublisher}
AppPublisherURL           ={#sAppPublisherURL}
AppSupportURL             ={#sAppSupportURL}
AppCopyright              ={#sAppCopyright}

DisableDirPage            =true
DisableFinishedPage       =true
DisableProgramGroupPage   =true
DisableReadyMemo          =true
DisableReadyPage          =true
;DisableStartupPrompt      =Yes
;DisableWelcomePage        =Yes

DefaultDirName            ={pf}\{#APP_PATH}\update
OutputDir                 =.\
OutputBaseFileName        ={#sOutputFileName}

Uninstallable             =false
PrivilegesRequired        =lowest

#ifdef ENABLE_SIGNING
SignTool                  =byparam $p
#endif

[Code]

function GetHKLM: Integer;
begin
  if IsWin64 then
    Result := HKLM64
  else 
    Result := HKEY_LOCAL_MACHINE;
end;

function getAppPrevLang: string;
var
  lang: string;
begin
  if RegValueExists(GetHKLM(), '{#APP_REG_PATH}', 'locale') and
    RegQueryStringValue(GetHKLM(), '{#APP_REG_PATH}', 'locale', lang) then
  begin
    result := lang
  end else
    result := '';
end;

function InitializeSetup: Boolean;
var
  I, ResultCode: Integer;
  InArgs: String;
begin
  InArgs := '';

  for I := 2 to ParamCount do
    InArgs := InArgs + ' ' + ParamStr(I);

  if Length(InArgs) > 0 then InArgs := InArgs + ' ';
  InArgs := InArgs + '/LANG='+getAppPrevLang() + ' /silent /update';

  ExtractTemporaryFiles('{tmp}\{#TARGET_NAME}');
  ExecAsOriginalUser(ExpandConstant('{tmp}\{#TARGET_NAME}'), InArgs, '', SW_SHOWNORMAL, ewNoWait, ResultCode);

  Result := False;
end;

[Files]
Source: {#TARGET_NAME}; Flags: dontcopy;

[Run]
;Filename: DesktopEditors_x64.exe; Parameters: "C:\test.txt"; Description: MyApp;