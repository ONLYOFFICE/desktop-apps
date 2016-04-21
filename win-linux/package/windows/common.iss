
;#define _IVOLGA_PRO
;#define _AVS
;#define _AVS_LIGHT_VERSION

#define sAppName            'ONLYOFFICE Desktop Editors'
#define APP_PATH            'ONLYOFFICE\DesktopEditors'
#define APP_REG_PATH        'Software\ONLYOFFICE\DesktopEditors'
#define NAME_EXE_OUT        'DesktopEditors.exe'
#define iconsExe            'projicons.exe'

#ifdef _IVOLGA_PRO
  #define sAppName          'Иволга ПРО'
  #define NAME_EXE_OUT      'IvolgaPRO.exe'
  #define APP_PATH          'IvolgaPRO\DesktopEditors'
  #define APP_REG_PATH      'Software\IvolgaPRO\DesktopEditors'
#elif defined(_AVS)
  #define sAppName          'AVS Document Editor'
  #define sShortAppName     'AVSDocumentEditor'
  #define AppInternalId     98
  #define NAME_EXE_OUT      sShortAppName + '.exe'
  #define APP_PATH          'AVS4YOU\' + sShortAppName
  #define APP_REG_PATH      'Software\AVS4YOU\DocumentEditor'

  #include "avs.iss"
#endif


#define PATH_EXE            '..\..\Build\Release\release\DesktopEditors.exe'
#define sAppVersion         GetFileVersion(AddBackslash(SourcePath) + PATH_EXE)
#ifdef _AVS
  #define sAppVerShort      Copy(sAppVersion, 0, 5)
#else
  #define sAppVerShort      Copy(sAppVersion, 0, 3)
#endif

#include "associate_page.iss"


[Setup]
AppName                   ={#sAppName}
AppVerName                ={#sAppName} {#sAppVerShort}
AppVersion                ={#sAppVersion}
VersionInfoVersion        ={#sAppVersion}

#ifdef _IVOLGA_PRO
  AppPublisher            =Novie kommunikacionnie tehnologii, CJSC
  AppPublisherURL         =http://www.ivolgapro.ru/
  AppSupportURL           =http://www.ivolgapro.ru/support.aspx
  AppCopyright            =Copyright (C) 2016 Novie kommunikacionnie tehnologii, CJSC.
  DefaultGroupName        ={#sAppName}
  WizardImageFile         = data\ivolga\dialogpicture.bmp
  WizardSmallImageFile    = data\ivolga\dialogicon.bmp

  ShowLanguageDialog      =no
  LanguageDetectionMethod =none
#elif defined(_AVS)
#else
  AppPublisher            =Ascensio System SIA.
  AppPublisherURL         =http://www.onlyoffice.com/
  AppSupportURL           =http://www.onlyoffice.com/support.aspx
  AppCopyright            =Copyright (C) 2016 Ascensio System SIA.

  DefaultGroupName        =ONLYOFFICE
  WizardImageFile         = data\dialogpicture.bmp
  WizardSmallImageFile    = data\dialogicon.bmp
#endif

UsePreviousAppDir         =no
DirExistsWarning          =no
DefaultDirName            ={pf}\{#APP_PATH}
DisableProgramGroupPage   = yes
DisableWelcomePage        = no
AllowNoIcons              = yes
UninstallDisplayIcon      = {app}\{#NAME_EXE_OUT}
OutputDir                 =.\
Compression               =lzma
PrivilegesRequired        =admin
#ifndef _AVS
  AppMutex                = TEAMLAB
#endif
ChangesEnvironment        =yes
SetupMutex                =ASC


[Languages]
#ifdef _IVOLGA_PRO
  Name: ru; MessagesFile: compiler:Languages\Russian.isl; LicenseFile: ..\..\..\common\package\license\eula_ivolgapro.rtf;
  Name: en; MessagesFile: compiler:Default.isl;           LicenseFile: ..\..\..\common\package\license\eula_ivolgapro.rtf;
#elif defined(_AVS)
#else
  Name: en; MessagesFile: compiler:Default.isl;           LicenseFile: ..\..\..\common\package\license\eula_onlyoffice.rtf;
  Name: ru; MessagesFile: compiler:Languages\Russian.isl; LicenseFile: ..\..\..\common\package\license\eula_onlyoffice.rtf;
#endif
;Name: de; MessagesFile: compiler:Languages\German.isl;
;Name: fr; MessagesFile: compiler:Languages\French.isl;
;Name: es; MessagesFile: compiler:Languages\Spanish.isl;
;Name: it; MessagesFile: compiler:Languages\Italian.isl;


[CustomMessages]
;======================================================================================================
;en.AppName=Ivolga PRO
;ru.AppName=Иволга ПРО
;======================================================================================================
en.Launch =Launch %1
ru.Launch =Запустить %1
;de.Launch =%1 starten
;fr.Launch =Lancer %1
;es.Launch =Ejecutar %1
;it.Launch =Eseguire %1
;======================================================================================================
en.CreateDesktopIcon =Create %1 &desktop icon
ru.CreateDesktopIcon =Создать иконку %1 на &рабочем столе
;de.CreateDesktopIcon =%1 &Desktop-Icon erstellen
;fr.CreateDesktopIcon =Crйer l'icфne du bureau pour %1
;es.CreateDesktopIcon =Crear %1 &icono en el escritorio
;it.CreateDesktopIcon =Creare un collegamento %1 sul &desktop
;======================================================================================================
en.InstallAdditionalComponents =Installing additional system components. Please wait...
ru.InstallAdditionalComponents =Установка дополнительных системных компонентов. Пожалуйста, подождите...
;de.InstallAdditionalComponents =Installation zusдtzlicher Systemkomponenten. Bitte warten...
;fr.InstallAdditionalComponents =L'installation des composants supplйmentaires du systиme. Attendez...
;es.InstallAdditionalComponents =Instalando componentes adicionales del sistema. Por favor espere...
;it.InstallAdditionalComponents =Installazione dei componenti addizionali del sistema. Per favore, attendi...
;======================================================================================================
;en.AdditionalTasks =Tasks:
;ru.AdditionalTasks =Задачи:
; de.AdditionalTasks =Aufgaben:
;fr.AdditionalTasks =Tвches:
;es.AdditionalTasks =Tareas:
;it.AdditionalTasks =Compiti:
;======================================================================================================
en.Uninstall =Uninstall
ru.Uninstall =Удаление
;de.Uninstall =Deinstallieren
;fr.Uninstall =Desinstaller
;es.Uninstall =Desinstalar
;it.Uninstall =Disinstalla
;======================================================================================================
en.WarningWrongArchitecture =You are trying to install the %1-bit application version over the %2-bit version installed. Please uninstall the previous version first or download the correct version for installation.
ru.WarningWrongArchitecture =Вы устанавливаете %1-битную версию приложения на уже установленную %2-битную. Пожалуйста, удалите предыдущую версию приложения или скачайте подходящую.
;de.WarningWrongArchitecture =Sie versuchen die %1-Bit-Version der Anwendung über die %2-Bit-Version, die schon installiert ist, zu installieren. Entfernen Sie bitte die Vorgängerversion zuerst oder laden Sie die richtige Version für die Installation herunter.
;fr.WarningWrongArchitecture =Vous essayez d'installer la version %1-bit sur la version %2-bit déjà installée. Veuillez désinstaller l'ancienne version d'abord ou télécharger la version correcte à installer.
;es.WarningWrongArchitecture =Usted está tratando de instalar la versión de la aplicación de %1 bits sobre la versión de %2 bits instalada. Por favor, desinstale la versión anterior primero o descargue la versión correcta para la instalación.
;it.Uninstall =Disinstalla
;======================================================================================================

;en.AssociateDescription =Associate office document file types with %1
;ru.AssociateDescription =Ассоциировать типы файлов офисных документов с %1


[Code]
const
  SMTO_ABORTIFHUNG = 2;
  WM_WININICHANGE = $001A;
  WM_SETTINGCHANGE = WM_WININICHANGE;

type
  WPARAM = UINT_PTR;
  LPARAM = INT_PTR;
  LRESULT = INT_PTR;


procedure GetSystemTimeAsFileTime(var lpFileTime: TFileTime); external 'GetSystemTimeAsFileTime@kernel32.dll';
function SendTextMessageTimeout(hWnd: HWND; Msg: UINT; wParam: WPARAM; lParam: PAnsiChar; fuFlags: UINT; uTimeout: UINT; out lpdwResult: DWORD): LRESULT;
  external 'SendMessageTimeoutA@user32.dll stdcall';

//procedure checkArchitectureVersion; forward;
#ifndef _AVS
function GetHKLM: Integer; forward;
#endif

procedure InitializeWizard();
begin
#if defined(_AVS) && !defined(_AVS_LIGHT_VERSION)
  InitializeGuidesLink();
#endif

  if not WizardSilent then
    InitializeAssociatePage();
end;

function InitializeSetup(): Boolean;
var
  OutResult: Boolean;
  path, mess: string;
  regkey: integer;
begin
  OutResult := True;

  if IsWin64 then
  begin 
    if Is64BitInstallMode then
    begin
      regkey := HKLM32;
      mess := ExpandConstant('{cm:WarningWrongArchitecture,64,32}')
    end else
    begin
      regkey := HKLM64;
      mess := ExpandConstant('{cm:WarningWrongArchitecture,32,64}')
    end;

    if RegQueryStringValue(regkey,
  #if   defined (_IVOLGA_PRO)
        'SOFTWARE\IvolgaPRO\DesktopEditors',
  #elif defined (_AVS)
        'SOFTWARE\AVS4YOU\DocumentEditor',
  #else
        'SOFTWARE\ONLYOFFICE\DesktopEditors',
  #endif
        'AppPath', path) then
    begin
      if FileExists(path + '\{#NAME_EXE_OUT}') then
      begin
        MsgBox(mess, mbInformation, MB_OK)
        OutResult := False
      end
    end
  end;

  Result := OutResult;
end;

procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
begin
#ifdef _AVS
  FinishedUninstall := (CurUninstallStep = usDone);
#endif

  if CurUninstallStep = usUninstall then
  begin
//    if MsgBox('Do you want to clear application cashed data?.', mbConfirmation, MB_YESNO) == IDYES then
//    begin
//      DelTree('', True, True, True)
//    end
    UnassociateExtensions();
  end;
end;

procedure installVCRedist(FileName, LabelCaption: String);
var
  Params:    String;
  ErrorCode: Integer;
begin
  if Length(LabelCaption) > 0 then WizardForm.StatusLabel.Caption := LabelCaption;

  Params := '/quiet';

  ShellExec('', FileName, Params, '', SW_SHOW, ewWaitUntilTerminated, ErrorCode);

  WizardForm.StatusLabel.Caption := SetupMessage(msgStatusExtractFiles);
end;

function GetHKLM: Integer;
begin
  if IsWin64 then
    Result := HKLM64
  else
    Result := HKEY_LOCAL_MACHINE;
end;

function checkVCRedist: Boolean;
var
  isExists: Boolean;
begin
  isExists := False;

  if not IsWin64 or Is64BitInstallMode then
    isExists := RegKeyExists(GetHKLM(), 'SOFTWARE\Microsoft\DevDiv\vc\Servicing\12.0\RuntimeMinimum')
  else
    isExists := RegKeyExists(GetHKLM(), 'SOFTWARE\Wow6432Node\Microsoft\DevDiv\vc\Servicing\12.0\RuntimeMinimum');

  Result := isExists;
end;

(*
procedure checkArchitectureVersion;
//var
  //isExists: Boolean;
begin
  if IsWin64 then
  begin 
    if Is64BitInstallMode then
    begin
      //isExists := RegKeyExists(GetHKLM(), 'SOFTWARE\Wow6432Node\ONLYOFFICE\ASCDocumentEditor')
      MsgBox(ExpandConstant('{cm:WarningWrongArchitecture,64,32}'), mbInformation, MB_OK)
    end else 
    begin
      //isExists := RegKeyExists(GetHKLM(), 'SOFTWARE\ONLYOFFICE\ASCDocumentEditor');
      MsgBox(ExpandConstant('{cm:WarningWrongArchitecture,32,64}'), mbInformation, MB_OK)
    end
  end;
end;
*)

function getPosixTime: string;
var 
  fileTime: TFileTime;
  fileTimeNano100: Int64;  
begin
  //GetSystemTime(systemTime);

  // the current file time
  //SystemTimeToFileTime(systemTime, fileTime);
  GetSystemTimeAsFileTime(fileTime);

  // filetime in 100 nanosecond resolution
  fileTimeNano100 := Int64(fileTime.dwHighDateTime) shl 32 + fileTime.dwLowDateTime;

  //Log('The Value is: ' + IntToStr(fileTimeNano100/10000 - 11644473600000));

  //to milliseconds and unix windows epoche offset removed
  Result := IntToStr(fileTimeNano100/10000 - 11644473600000);
end;

function libExists(const dllname: String) : boolean;
begin
  Result := not FileExists(ExpandConstant('{sys}\'+dllname));
end;

function NeedsAddPath(Param: string): boolean;
var
  OrigPath: string;
begin
  if not RegQueryStringValue(GetHKLM(), 'SYSTEM\CurrentControlSet\Control\Session Manager\Environment', 'Path', OrigPath)
  then begin
    Result := True;
    exit;
  end;
  // look for the path with leading and trailing semicolon
  // Pos() returns 0 if not found
  Result := Pos(';' + Param + ';', ';' + OrigPath + ';') = 0;
end;

procedure RefreshEnvironment;
var
  S: AnsiString;
  MsgResult: DWORD;
begin
  S := 'Environment';
  SendTextMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, 0, PAnsiChar(S), SMTO_ABORTIFHUNG, 5000, MsgResult);
end;

procedure CurStepChanged(CurStep: TSetupStep);
begin
  if CurStep = ssPostInstall then 
  begin
    DoPostInstall();
#ifdef _AVS
  end
  else if CurStep = ssInstall then
  begin
    DoInstall();
  end
  else if CurStep = ssDone then
  begin
    DoInstallDone();
#endif
  end;
end;


[Dirs]
Name: {commonappdata}\{#APP_PATH}\webdata\cloud; Flags: uninsalwaysuninstall


[Files]
Source: .\launch.bat;           DestDir: {app}\;

Source: ..\..\build\Release\release\DesktopEditors.exe;  DestDir: {app}\; DestName: {#NAME_EXE_OUT}; 
#if   defined(_IVOLGA_PRO)
Source: ..\..\res\icons\ivolga\desktopeditors.ico;              DestDir: {app}\; DestName: app.ico; 
Source: ..\..\..\common\loginpage\deploy\index.ivolgapro.html;  DestDir: {commonappdata}\{#APP_PATH}\webdata\local; DestName: index.html;
;Source: ..\..\common\package\license\eula_ivolga.rtf; DestDir: {app}; DestName: LICENSE.rtf;
Source: ..\..\..\common\package\license\eula_ivolgapro.htm;     DestDir: {app}; DestName: LICENSE.htm;
Source: data\projicons_nct.exe;                                 DestDir: {app}; DestName: {#iconsExe};
#elif defined(_AVS)
#else
Source: ..\..\res\icons\desktopeditors.ico;                     DestDir: {app}; DestName: app.ico;
Source: ..\..\..\common\loginpage\deploy\index.html;            DestDir: {commonappdata}\{#APP_PATH}\webdata\local; DestName: index.html;
;Source: ..\..\common\package\license\eula_onlyoffice.rtf; DestDir: {app}; DestName: LICENSE.rtf;
Source: ..\..\..\common\package\license\eula_onlyoffice.htm;    DestDir: {app}; DestName: LICENSE.htm;
Source: data\projicons_asc.exe;                                 DestDir: {app}; DestName: {#iconsExe};
#endif
Source: ..\..\..\common\package\license\3dparty\3DPARTYLICENSE; DestDir: {app};
;Source: data\webdata\cloud\*;                      DestDir: {commonappdata}\{#APP_PATH}\webdata\cloud; Flags: recursesubdirs;
;Source: ..\..\common\loginpage\deploy\*;           DestDir: {commonappdata}\{#APP_PATH}\webdata\local;
Source: ..\..\..\common\package\dictionaries\*;       DestDir: {app}\dictionaries; Flags: recursesubdirs;

Source: ..\..\..\common\editors\*;                      DestDir: {app}\editors\web-apps;  Flags: recursesubdirs;
Source: ..\..\..\..\core\build\jsbuilds\*;              DestDir: {app}\editors\sdkjs;     Flags: recursesubdirs;
Source: ..\..\..\common\converter\DoctRenderer.config;  DestDir: {app}\converter;
Source: ..\..\..\..\core\build\empty\*;                 DestDir: {app}\converter\empty;

Source: ..\..\..\common\package\fonts\LICENSE.txt;                    DestDir: {commonappdata}\{#APP_PATH}\webdata\local\fonts;
Source: ..\..\..\common\package\fonts\OpenSans-Bold.ttf;              DestDir: {commonappdata}\{#APP_PATH}\webdata\local\fonts; Flags: onlyifdoesntexist;
Source: ..\..\..\common\package\fonts\OpenSans-Regular.ttf;           DestDir: {commonappdata}\{#APP_PATH}\webdata\local\fonts; Flags: onlyifdoesntexist;
Source: ..\..\..\common\package\fonts\OpenSans-ExtraBold.ttf;         DestDir: {commonappdata}\{#APP_PATH}\webdata\local\fonts; Flags: onlyifdoesntexist;
Source: ..\..\..\common\package\fonts\OpenSans-Light.ttf;             DestDir: {commonappdata}\{#APP_PATH}\webdata\local\fonts; Flags: onlyifdoesntexist;
Source: ..\..\..\common\package\fonts\OpenSans-Semibold.ttf;          DestDir: {commonappdata}\{#APP_PATH}\webdata\local\fonts; Flags: onlyifdoesntexist;
;Source: data\fonts\OpenSans-ExtraBoldItalic.ttf;           DestDir: {fonts}; FontInstall: Open Sans Extrabold Italic; Flags: onlyifdoesntexist uninsneveruninstall;
;Source: data\fonts\OpenSans-BoldItalic.ttf;                DestDir: {fonts}; FontInstall: Open Sans Bold Italic;      Flags: onlyifdoesntexist uninsneveruninstall;
;Source: data\fonts\OpenSans-Italic.ttf;                    DestDir: {fonts}; FontInstall: Open Sans Italic;           Flags: onlyifdoesntexist uninsneveruninstall;
;Source: data\fonts\OpenSans-LightItalic.ttf;               DestDir: {fonts}; FontInstall: Open Sans Light Italic;     Flags: onlyifdoesntexist uninsneveruninstall;
;Source: data\fonts\OpenSans-SemiboldItalic.ttf;            DestDir: {fonts}; FontInstall: Open Sans Semibold Italic;  Flags: onlyifdoesntexist uninsneveruninstall;

Source: ..\..\..\common\package\fonts\Asana-Math.ttf;          DestDir: {app}\fonts; Flags: onlyifdoesntexist;
Source: ..\..\..\common\package\fonts\Carlito-Bold.ttf;        DestDir: {app}\fonts; Flags: onlyifdoesntexist;
Source: ..\..\..\common\package\fonts\Carlito-BoldItalic.ttf;  DestDir: {app}\fonts; Flags: onlyifdoesntexist;
Source: ..\..\..\common\package\fonts\Carlito-Italic.ttf;      DestDir: {app}\fonts; Flags: onlyifdoesntexist;
Source: ..\..\..\common\package\fonts\Carlito-Regular.ttf;     DestDir: {app}\fonts; Flags: onlyifdoesntexist;


[Tasks]
Name: desktopicon; Description: {cm:CreateDesktopIcon,{#sAppName}}; GroupDescription: {cm:AdditionalIcons};
;Name: fileassoc; Description: {cm:AssociateCaption};   GroupDescription: {cm:AssociateDescription};


[Icons]
;Name: {commondesktop}\{#sAppName}; FileName: {app}\{#NAME_EXE_OUT}; WorkingDir: {app}; Tasks: desktopicon;
Name: {commondesktop}\{#sAppName}; FileName: {app}\{#NAME_EXE_OUT}; WorkingDir: {app}; Tasks: desktopicon; IconFilename: {app}\app.ico;
#ifndef _AVS
Name: {group}\{#sAppName};         Filename: {app}\{#NAME_EXE_OUT}; WorkingDir: {app}; IconFilename: {app}\app.ico;
Name: {group}\{cm:Uninstall}; Filename: {uninstallexe}; WorkingDir: {app};
#endif


[Run]
;Filename: {app}\{#NAME_EXE_OUT}; Description: {cm:Launch,{#sAppName}}; Flags: postinstall nowait skipifsilent;
Filename: {app}\launch.bat; Parameters: {#NAME_EXE_OUT}; Description: {cm:Launch,{#sAppName}}; Flags: postinstall nowait skipifsilent runhidden;
;Filename: http://www.onlyoffice.com/remove-portal-feedback-form.aspx; Description: Visit website; Flags: postinstall shellexec nowait 


[Ini]
;Filename: {app}\opts; Section: app; Key: lang; String: {language};


[Registry]
;Root: HKLM; Subkey: {#APP_REG_PATH};  Flags: uninsdeletekey;
Root: HKLM; Subkey: {#APP_REG_PATH};  ValueType: string;   ValueName: locale;  ValueData: {language};             Flags: uninsdeletevalue;
Root: HKLM; Subkey: {#APP_REG_PATH};  ValueType: qword;    ValueName: timestamp;  ValueData: {code:getPosixTime}; Flags: uninsdeletevalue;
Root: HKLM; Subkey: SYSTEM\CurrentControlSet\Control\Session Manager\Environment; ValueType: expandsz; ValueName: Path; ValueData: "{olddata};{app}\converter"; Check: NeedsAddPath(ExpandConstant('{app}\converter')); AfterInstall: RefreshEnvironment;


[UninstallDelete]
Type: filesandordirs; Name: {commonappdata}\{#APP_PATH}\*;  AfterInstall: RefreshEnvironment;
