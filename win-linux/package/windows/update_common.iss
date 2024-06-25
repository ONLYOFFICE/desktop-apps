; -- Update Common --

#ifndef BRANDING_DIR
#define BRANDING_DIR '.'
#endif
#include BRANDING_DIR + '\defines.iss'

#ifndef VERSION
#define VERSION '0.0.0.0'
#endif
#ifndef ARCH
#define ARCH 'x64'
#endif
#ifndef OUTPUT_DIR
#define OUTPUT_DIR '.'
#endif
#ifndef OUTPUT_FILE
#define OUTPUT_FILE sPackageName + '-Update-' + VERSION + '-' + ARCH
#ifdef _WIN_XP
#define OUTPUT_FILE OUTPUT_FILE + '-xp'
#endif
#endif
#ifndef TARGET_NAME
#define TARGET_NAME sPackageName + '-' + VERSION + '-' + ARCH
#ifdef _WIN_XP
#define TARGET_NAME TARGET_NAME + '-xp'
#endif
#endif

[Setup]
AppName                   ={#sAppName}
AppVerName                ={#sAppName} {#Copy(VERSION,1,RPos('.',VERSION)-1)}
AppVersion                ={#VERSION}
VersionInfoVersion        ={#VERSION}

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
OutputDir                 ={#OUTPUT_DIR}
OutputBaseFileName        ={#OUTPUT_FILE}

Uninstallable             =false
PrivilegesRequired        =lowest

#ifdef SIGN
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

  ExtractTemporaryFiles('{tmp}\{#TARGET_NAME}.exe');
  ExecAsOriginalUser(ExpandConstant('{tmp}\{#TARGET_NAME}.exe'), InArgs, '', SW_SHOWNORMAL, ewNoWait, ResultCode);

  Result := False;
end;

[Files]
Source: "{#TARGET_NAME}.exe"; Flags: dontcopy;

[Run]
;Filename: DesktopEditors_x64.exe; Parameters: "C:\test.txt"; Description: MyApp;