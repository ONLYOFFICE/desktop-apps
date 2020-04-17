
#define sAppName          'ONLYOFFICE Desktop Editors'
#define APP_PATH          'ONLYOFFICE\DesktopEditors'
#define APP_REG_PATH      'Software\ONLYOFFICE\DesktopEditors'

#ifndef sAppVersion
  #define sAppVersion     GetFileVersion(AddBackslash(SourcePath) + '..\' + TARGET_NAME)
#endif
#define sAppVerShort      Copy(sAppVersion, 0, 3)

[Setup]
AppName                   ={#sAppName}
AppVerName                ={#sAppName} {#sAppVerShort}
AppVersion                ={#sAppVersion}
VersionInfoVersion        ={#sAppVersion}

AppPublisher              =Ascensio System SIA.
AppPublisherURL           =http://www.onlyoffice.com/
AppSupportURL             =http://www.onlyoffice.com/support.aspx
AppCopyright              =Copyright (C) 2018 Ascensio System SIA.

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