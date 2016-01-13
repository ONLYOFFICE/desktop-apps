;#define sAppName            'ONLYOFFICE Desktop Editors'
#define sAppName            'Иволга ПРО'

#define NAME_EXE_OUT        'DesktopEditors.exe'
;#define ASC_PATH            'ONLYOFFICE\DesktopEditors'
;#define ASC_REG_PATH        'Software\ONLYOFFICE\DesktopEditors'
;#define REG_SC_PATH         'Software\Classes\OnlyOffice'
#define ASC_PATH            'IvolgaPRO\DesktopEditors'
#define ASC_REG_PATH        'Software\IvolgaPRO\DesktopEditors'
#define REG_SC_PATH         'Software\Classes\IvolgaPRO'

#define PATH_EXE            '..\Build\Release\release\DesktopEditors.exe'
#define sAppVersion         GetFileVersion(AddBackslash(SourcePath) + PATH_EXE)
#define sAppVerShort        Copy(GetFileVersion(AddBackslash(SourcePath) + PATH_EXE), 0, 3)

#define iconsExe            'projicons.exe'

[Setup]
AppVerName              ={cm:AppName} {#sAppVerShort}
VersionInfoVersion      ={#sAppVersion}
AppPublisher            =ZAO "New Communication Technologies".
;AppPublisherURL         =http://www.onlyoffice.com/
;AppSupportURL           =http://www.onlyoffice.com/support.aspx
AppPublisherURL         =http://www.ivolgapro.com/
AppSupportURL           =http://www.ivolgapro.com/support.aspx
AppCopyright            =Copyright (C) 2016 ZAO "New Communication Technologies".
UsePreviousAppDir       =no
DirExistsWarning        =no
DefaultDirName          ={pf}\{#ASC_PATH}
;DefaultGroupName        =ONLYOFFICE
DefaultGroupName        ={cm:AppName}
DisableProgramGroupPage = yes
AllowNoIcons            = yes
WizardImageFile         = data\dialogpicture.bmp
WizardSmallImageFile    = data\dialogicon.bmp
UninstallDisplayIcon    = {app}\{#NAME_EXE_OUT}
OutputDir               =.\
Compression             =lzma
PrivilegesRequired      =admin
AppMutex                =TEAMLAB
ChangesEnvironment      = yes

[Languages]
Name: en; MessagesFile: compiler:Default.isl; LicenseFile: license\License_ivolga.rtf;
Name: ru; MessagesFile: compiler:Languages\Russian.isl; LicenseFile: license\License_ivolga.rtf;
;Name: de; MessagesFile: compiler:Languages\German.isl;
;Name: fr; MessagesFile: compiler:Languages\French.isl;
;Name: es; MessagesFile: compiler:Languages\Spanish.isl;
;Name: it; MessagesFile: compiler:Languages\Italian.isl;

[CustomMessages]
;======================================================================================================
en.AppName=Ivolga PRO
ru.AppName=Иволга ПРО
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
ru.InstallAdditionalComponents =Установка дополнительных системных компонент. Пожалуйста, подождите...
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
en.AssociateCaption =File Associations
ru.AssociateCaption =Ассоциации файлов
;de.AssociateCaption =Dateiassoziationen
;fr.AssociateCaption =Associations de fichiers
;es.AssociateCaption =Asociaciones de archivos
;it.AssociateCaption =Associazioni dei file
;======================================================================================================
en.AssociateDescription =Associate office document file types with ONLYOFFICE Desktop
ru.AssociateDescription =Ассоциировать типы файлов офисных документов с ONLYOFFICE Desktop
;de.AssociateDescription =Video- und Audiodateitypen mit AVS Media Player assoziieren
;fr.AssociateDescription =Associer des types de fichiers vidйo et audio а AVS Media Player
;es.AssociateDescription =Asociar tipos de archivos de vнdeo y audio con AVS Media Player
;it.AssociateDescription =Associare i tipi di file video/audio ad AVS Media Player
;======================================================================================================

[Code]
procedure GetSystemTimeAsFileTime(var lpFileTime: TFileTime); external 'GetSystemTimeAsFileTime@kernel32.dll';

procedure checkArchitectureVersion; forward;
function GetHKLM: Integer; forward;

function InitializeSetup(): Boolean;
var
  OutResult: Boolean;
begin
  OutResult := True;

  if IsWin64 then
  begin 
    if Is64BitInstallMode then
    begin
      if RegKeyExists(GetHKLM(), 'SOFTWARE\Wow6432Node\ONLYOFFICE\DesktopEditors') then
      begin      
        MsgBox(ExpandConstant('{cm:WarningWrongArchitecture,64,32}'), mbInformation, MB_OK)
        OutResult := False
      end
    end else if RegKeyExists(GetHKLM(), 'SOFTWARE\ONLYOFFICE\DesktopEditors') then
    begin
      MsgBox(ExpandConstant('{cm:WarningWrongArchitecture,32,64}'), mbInformation, MB_OK)
      OutResult := False
    end
  end;

  Result := OutResult;
end;

procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
begin
  if CurUninstallStep = usUninstall then
  begin
//    if MsgBox('Do you want to clear application cashed data?.', mbConfirmation, MB_YESNO) == IDYES then
//    begin
//      DelTree('', True, True, True)
//    end
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

procedure checkArchitectureVersion;
var
  isExists: Boolean;

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

  //Result := True;
end;

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

const
  SMTO_ABORTIFHUNG = 2;
  WM_WININICHANGE = $001A;
  WM_SETTINGCHANGE = WM_WININICHANGE;

type
  WPARAM = UINT_PTR;
  LPARAM = INT_PTR;
  LRESULT = INT_PTR;

function SendTextMessageTimeout(hWnd: HWND; Msg: UINT; wParam: WPARAM; lParam: PAnsiChar; fuFlags: UINT; uTimeout: UINT; out lpdwResult: DWORD): LRESULT;
  external 'SendMessageTimeoutA@user32.dll stdcall';  

procedure RefreshEnvironment;
var
  S: AnsiString;
  MsgResult: DWORD;
begin
  S := 'Environment';
  SendTextMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, 0, PAnsiChar(S), SMTO_ABORTIFHUNG, 5000, MsgResult);
end;

[Dirs]
Name: {commonappdata}\{#ASC_PATH}\webdata\cloud; Flags: uninsalwaysuninstall

[Files]
Source: .\launch.bat;           DestDir: {app}\;

Source: ..\res\icons\desktopeditors.ico;           DestDir: {app}\;
Source: data\projicons.exe;                        DestDir: {app}\;
;Source: data\webdata\cloud\*;                      DestDir: {commonappdata}\{#ASC_PATH}\webdata\cloud; Flags: recursesubdirs;
Source: ..\..\common\loginpage\deploy\*;           DestDir: {commonappdata}\{#ASC_PATH}\webdata\local;
Source: ..\..\common\package\dictionaries\*;       DestDir: {app}\dictionaries; Flags: recursesubdirs;

Source: ..\..\common\editors\apps\*;                DestDir: {app}\editors\apps;   Flags: recursesubdirs;
Source: ..\..\common\editors\vendor\*;              DestDir: {app}\editors\vendor; Flags: recursesubdirs;
Source: ..\..\common\converter\empty\*;             DestDir: {app}\converter\empty; Flags: recursesubdirs;
Source: ..\..\common\converter\DoctRenderer.config; DestDir: {app}\converter;
Source: ..\..\common\converter\windows\icudt.dll;   DestDir: {app}\converter; Flags: ignoreversion;
Source: ..\..\common\libs\ChromiumBasedEditors2\app\jsbuilds\*; DestDir: {app}\editors\sdk; Flags: recursesubdirs;

Source: ..\..\common\package\fonts\LICENSE.txt;                    DestDir: {commonappdata}\{#ASC_PATH}\webdata\local\fonts;
Source: ..\..\common\package\fonts\OpenSans-Bold.ttf;              DestDir: {commonappdata}\{#ASC_PATH}\webdata\local\fonts; Flags: onlyifdoesntexist;
Source: ..\..\common\package\fonts\OpenSans-Regular.ttf;           DestDir: {commonappdata}\{#ASC_PATH}\webdata\local\fonts; Flags: onlyifdoesntexist;
Source: ..\..\common\package\fonts\OpenSans-ExtraBold.ttf;         DestDir: {commonappdata}\{#ASC_PATH}\webdata\local\fonts; Flags: onlyifdoesntexist;
Source: ..\..\common\package\fonts\OpenSans-Light.ttf;             DestDir: {commonappdata}\{#ASC_PATH}\webdata\local\fonts; Flags: onlyifdoesntexist;
Source: ..\..\common\package\fonts\OpenSans-Semibold.ttf;          DestDir: {commonappdata}\{#ASC_PATH}\webdata\local\fonts; Flags: onlyifdoesntexist;
;Source: data\fonts\OpenSans-ExtraBoldItalic.ttf;           DestDir: {fonts}; FontInstall: Open Sans Extrabold Italic; Flags: onlyifdoesntexist uninsneveruninstall;
;Source: data\fonts\OpenSans-BoldItalic.ttf;                DestDir: {fonts}; FontInstall: Open Sans Bold Italic;      Flags: onlyifdoesntexist uninsneveruninstall;
;Source: data\fonts\OpenSans-Italic.ttf;                    DestDir: {fonts}; FontInstall: Open Sans Italic;           Flags: onlyifdoesntexist uninsneveruninstall;
;Source: data\fonts\OpenSans-LightItalic.ttf;               DestDir: {fonts}; FontInstall: Open Sans Light Italic;     Flags: onlyifdoesntexist uninsneveruninstall;
;Source: data\fonts\OpenSans-SemiboldItalic.ttf;            DestDir: {fonts}; FontInstall: Open Sans Semibold Italic;  Flags: onlyifdoesntexist uninsneveruninstall;

Source: ..\..\common\package\fonts\Asana-Math.ttf;          DestDir: {app}\fonts; Flags: onlyifdoesntexist;
Source: ..\..\common\package\fonts\Carlito-Bold.ttf;        DestDir: {app}\fonts; Flags: onlyifdoesntexist;
Source: ..\..\common\package\fonts\Carlito-BoldItalic.ttf;  DestDir: {app}\fonts; Flags: onlyifdoesntexist;
Source: ..\..\common\package\fonts\Carlito-Italic.ttf;      DestDir: {app}\fonts; Flags: onlyifdoesntexist;
Source: ..\..\common\package\fonts\Carlito-Regular.ttf;     DestDir: {app}\fonts; Flags: onlyifdoesntexist;


[Tasks]
Name: desktopicon; Description: {cm:CreateDesktopIcon,{cm:AppName}}; GroupDescription: {cm:AdditionalIcons};
;Name: fileassoc; Description: {cm:AssociateCaption};   GroupDescription: {cm:AssociateDescription};

[Icons]
;Name: {commondesktop}\{#sAppName}; FileName: {app}\{#NAME_EXE_OUT}; WorkingDir: {app}; Tasks: desktopicon;
Name: {commondesktop}\{cm:AppName}; FileName: {app}\{#NAME_EXE_OUT}; WorkingDir: {app}; Tasks: desktopicon; IconFilename: {app}\desktopeditors.ico;
Name: {group}\{cm:AppName};         Filename: {app}\{#NAME_EXE_OUT}; WorkingDir: {app}; IconFilename: {app}\desktopeditors.ico;
Name: {group}\{cm:Uninstall}; Filename: {uninstallexe}; WorkingDir: {app};

[Run]
;Filename: {app}\{#NAME_EXE_OUT}; Description: {cm:Launch,{#sAppName}}; Flags: postinstall nowait skipifsilent;
Filename: {app}\launch.bat; Description: {cm:Launch,{cm:AppName}}; Flags: postinstall nowait skipifsilent runhidden;
;Filename: http://www.onlyoffice.com/remove-portal-feedback-form.aspx; Description: Visit website; Flags: postinstall shellexec nowait 

[Ini]
;Filename: {app}\opts; Section: app; Key: lang; String: {language};

[Registry]
;Root: HKLM; Subkey: {#ASC_REG_PATH};  Flags: uninsdeletekey;
Root: HKLM; Subkey: {#ASC_REG_PATH};  ValueType: string;   ValueName: locale;  ValueData: {language};             Flags: uninsdeletevalue;
Root: HKLM; Subkey: {#ASC_REG_PATH};  ValueType: qword;    ValueName: timestamp;  ValueData: {code:getPosixTime}; Flags: uninsdeletevalue;

;Root: HKLM; Subkey: Software\Classes\.doc;                         ValueType:string;ValueName:""; Tasks:fileassoc; ValueData: "OnlyOffice.Document.8";              Flags: uninsdeletevalue
;Root: HKLM; Subkey: {#REG_SC_PATH}.Document.8;                     ValueType:string;ValueName:""; Tasks:fileassoc; ValueData: "Microsoft Word 97 - 2003 Document";  Flags: uninsdeletekey
;Root: HKLM; Subkey: {#REG_SC_PATH}.Document.8\DefaultIcon;         ValueType:string;ValueName:""; Tasks:fileassoc; ValueData: "{app}\{#iconsExe},7";            
;Root: HKLM; Subkey: {#REG_SC_PATH}.Document.8\shell\open\command;  ValueType:string;ValueName:""; Tasks:fileassoc; ValueData: """{app}\{#NAME_EXE_OUT}"" ""%1""";   

;Root: HKLM; Subkey: Software\Classes\.docx;                         ValueType:string;ValueName:""; Tasks:fileassoc; ValueData: "OnlyOffice.Document.12";            Flags: uninsdeletevalue
;Root: HKLM; Subkey: {#REG_SC_PATH}.Document.12;                     ValueType:string;ValueName:""; Tasks:fileassoc; ValueData: "Microsoft Word Document";           Flags: uninsdeletekey
;Root: HKLM; Subkey: {#REG_SC_PATH}.Document.12\DefaultIcon;         ValueType:string;ValueName:""; Tasks:fileassoc; ValueData: "{app}\{#iconsExe},7";           
;Root: HKLM; Subkey: {#REG_SC_PATH}.Document.12\shell\open\command;  ValueType:string;ValueName:""; Tasks:fileassoc; ValueData: """{app}\{#NAME_EXE_OUT}"" ""%1""";  

;Root: HKLM; Subkey: Software\Classes\.xls;                        ValueType:string;ValueName:""; Tasks:fileassoc; ValueData: "OnlyOffice.Sheet.8";                  Flags: uninsdeletevalue
;Root: HKLM; Subkey: {#REG_SC_PATH}.Sheet.8;                       ValueType:string;ValueName:""; Tasks:fileassoc; ValueData: "Microsoft Excel 97-2003 Worksheet";   Flags: uninsdeletekey
;Root: HKLM; Subkey: {#REG_SC_PATH}.Sheet.8\DefaultIcon;           ValueType:string;ValueName:""; Tasks:fileassoc; ValueData: "{app}\{#iconsExe},12";             
;Root: HKLM; Subkey: {#REG_SC_PATH}.Sheet.8\shell\open\command;    ValueType:string;ValueName:""; Tasks:fileassoc; ValueData: """{app}\{#NAME_EXE_OUT}"" ""%1""";    

;Root: HKLM; Subkey: Software\Classes\.xlsx;                       ValueType:string;ValueName:""; Tasks:fileassoc; ValueData: "OnlyOffice.Sheet.12";               Flags: uninsdeletevalue
;Root: HKLM; Subkey: {#REG_SC_PATH}.Sheet.12;                      ValueType:string;ValueName:""; Tasks:fileassoc; ValueData: "Microsoft Excel Worksheet";         Flags: uninsdeletekey
;Root: HKLM; Subkey: {#REG_SC_PATH}.Sheet.12\DefaultIcon;          ValueType:string;ValueName:""; Tasks:fileassoc; ValueData: "{app}\{#iconsExe},12";           
;Root: HKLM; Subkey: {#REG_SC_PATH}.Sheet.12\shell\open\command;   ValueType:string;ValueName:""; Tasks:fileassoc; ValueData: """{app}\{#NAME_EXE_OUT}"" ""%1""";  

;Root: HKLM; Subkey: Software\Classes\.ppt;                        ValueType:string;ValueName:""; Tasks:fileassoc; ValueData: "OnlyOffice.Show.8";                         Flags: uninsdeletevalue
;Root: HKLM; Subkey: {#REG_SC_PATH}.Show.8;                        ValueType:string;ValueName:""; Tasks:fileassoc; ValueData: "Microsoft PowerPoint 97-2003 Presentation"; Flags: uninsdeletekey
;Root: HKLM; Subkey: {#REG_SC_PATH}.Show.8\DefaultIcon;            ValueType:string;ValueName:""; Tasks:fileassoc; ValueData: "{app}\{#iconsExe},1";                   
;Root: HKLM; Subkey: {#REG_SC_PATH}.Show.8\shell\open\command;     ValueType:string;ValueName:""; Tasks:fileassoc; ValueData: """{app}\{#NAME_EXE_OUT}"" ""%1""";          

;Root: HKLM; Subkey: Software\Classes\.pptx;                       ValueType:string;ValueName:""; Tasks:fileassoc; ValueData: "OnlyOffice.Show.12";                Flags: uninsdeletevalue
;Root: HKLM; Subkey: {#REG_SC_PATH}.Show.12;                       ValueType:string;ValueName:""; Tasks:fileassoc; ValueData: "Microsoft PowerPoint Presentation"; Flags: uninsdeletekey
;Root: HKLM; Subkey: {#REG_SC_PATH}.Show.12\DefaultIcon;           ValueType:string;ValueName:""; Tasks:fileassoc; ValueData: "{app}\{#iconsExe},1";           
;Root: HKLM; Subkey: {#REG_SC_PATH}.Show.12\shell\open\command;    ValueType:string;ValueName:""; Tasks:fileassoc; ValueData: """{app}\{#NAME_EXE_OUT}"" ""%1""";  

;Root: HKLM; Subkey: Software\Classes\.pps;                        ValueType:string;ValueName:""; Tasks:fileassoc; ValueData: "OnlyOffice.SlideShow.8";                       Flags: uninsdeletevalue
;Root: HKLM; Subkey: {#REG_SC_PATH}.SlideShow.8;                   ValueType:string;ValueName:""; Tasks:fileassoc; ValueData: "Microsoft PowerPoint 97-2003 Slide Show"; Flags: uninsdeletekey
;Root: HKLM; Subkey: {#REG_SC_PATH}.SlideShow.8\DefaultIcon;       ValueType:string;ValueName:""; Tasks:fileassoc; ValueData: "{app}\{#iconsExe},2";                   
;Root: HKLM; Subkey: {#REG_SC_PATH}.SlideShow.8\shell\open\command;ValueType:string;ValueName:""; Tasks:fileassoc; ValueData: """{app}\{#NAME_EXE_OUT}"" ""%1""";          

;Root: HKLM; Subkey: Software\Classes\.ppsx;                       ValueType:string;ValueName:""; Tasks:fileassoc; ValueData: "OnlyOffice.SlideShow.12";                Flags: uninsdeletevalue
;Root: HKLM; Subkey: {#REG_SC_PATH}.SlideShow.12;                  ValueType:string;ValueName:""; Tasks:fileassoc; ValueData: "Microsoft PowerPoint Slide Show"; Flags: uninsdeletekey
;Root: HKLM; Subkey: {#REG_SC_PATH}.SlideShow.12\DefaultIcon;      ValueType:string;ValueName:""; Tasks:fileassoc; ValueData: "{app}\{#iconsExe},2";           
;Root: HKLM; Subkey: {#REG_SC_PATH}.SlideShow.12\shell\open\command;ValueType:string;ValueName:""; Tasks:fileassoc; ValueData: """{app}\{#NAME_EXE_OUT}"" ""%1""";  

;Root: HKLM; Subkey: Software\Classes\.odt;                        ValueType:string;ValueName:""; Tasks:fileassoc; ValueData: "OnlyOffice.Document.21";            Flags: uninsdeletevalue
;Root: HKLM; Subkey: {#REG_SC_PATH}.Document.21;                   ValueType:string;ValueName:""; Tasks:fileassoc; ValueData: "OpenOffice Document";               Flags: uninsdeletekey
;Root: HKLM; Subkey: {#REG_SC_PATH}.Document.21\DefaultIcon;       ValueType:string;ValueName:""; Tasks:fileassoc; ValueData: "{app}\{#iconsExe},8";           
;Root: HKLM; Subkey: {#REG_SC_PATH}.Document.21\shell\open\command;ValueType:string;ValueName:""; Tasks:fileassoc; ValueData: """{app}\{#NAME_EXE_OUT}"" ""%1""";  

;Root: HKLM; Subkey: Software\Classes\.ods;                        ValueType:string;ValueName:""; Tasks:fileassoc; ValueData: "OnlyOffice.Sheet.21";             Flags: uninsdeletevalue
;Root: HKLM; Subkey: {#REG_SC_PATH}.Sheet.21;                      ValueType:string;ValueName:""; Tasks:fileassoc; ValueData: "OpenOffice Worksheet";            Flags: uninsdeletekey
;Root: HKLM; Subkey: {#REG_SC_PATH}.Sheet.21\DefaultIcon;          ValueType:string;ValueName:""; Tasks:fileassoc; ValueData: "{app}\{#iconsExe},13";           
;Root: HKLM; Subkey: {#REG_SC_PATH}.Sheet.21\shell\open\command;   ValueType:string;ValueName:""; Tasks:fileassoc; ValueData: """{app}\{#NAME_EXE_OUT}"" ""%1""";  

;Root: HKLM; Subkey: Software\Classes\.odp;                        ValueType:string;ValueName:""; Tasks:fileassoc; ValueData: "OnlyOffice.Show.21";              Flags: uninsdeletevalue
;Root: HKLM; Subkey: {#REG_SC_PATH}.Show.21;                       ValueType:string;ValueName:""; Tasks:fileassoc; ValueData: "OpenOffice Presentation";         Flags: uninsdeletekey
;Root: HKLM; Subkey: {#REG_SC_PATH}.Show.21\DefaultIcon;           ValueType:string;ValueName:""; Tasks:fileassoc; ValueData: "{app}\{#iconsExe},3";           
;Root: HKLM; Subkey: {#REG_SC_PATH}.Show.21\shell\open\command;    ValueType:string;ValueName:""; Tasks:fileassoc; ValueData: """{app}\{#NAME_EXE_OUT}"" ""%1""";  

;Root: HKLM; Subkey: Software\Classes\.csv;                        ValueType:string;ValueName:""; Tasks:fileassoc; ValueData: "OnlyOffice.Csv.1";                            Flags: uninsdeletevalue
;Root: HKLM; Subkey: {#REG_SC_PATH}.Csv.1;                         ValueType:string;ValueName:""; Tasks:fileassoc; ValueData: "Microsoft Excel Comma Separated Values File"; Flags: uninsdeletekey
;Root: HKLM; Subkey: {#REG_SC_PATH}.Csv.1\DefaultIcon;             ValueType:string;ValueName:""; Tasks:fileassoc; ValueData: "{app}\{#iconsExe},14";           
;Root: HKLM; Subkey: {#REG_SC_PATH}.Csv.1\shell\open\command;      ValueType:string;ValueName:""; Tasks:fileassoc; ValueData: """{app}\{#NAME_EXE_OUT}"" ""%1""";  

;Root: HKLM; Subkey: Software\Classes\.rtf;                        ValueType:string;ValueName:""; Tasks:fileassoc; ValueData: "OnlyOffice.Rtf.1";                Flags: uninsdeletevalue
;Root: HKLM; Subkey: {#REG_SC_PATH}.Rtf.1;                         ValueType:string;ValueName:""; Tasks:fileassoc; ValueData: "Rich Text Format";                Flags: uninsdeletekey
;Root: HKLM; Subkey: {#REG_SC_PATH}.Rtf.1\DefaultIcon;             ValueType:string;ValueName:""; Tasks:fileassoc; ValueData: "{app}\{#iconsExe},9";           
;Root: HKLM; Subkey: {#REG_SC_PATH}.Rtf.1\shell\open\command;      ValueType:string;ValueName:""; Tasks:fileassoc; ValueData: """{app}\{#NAME_EXE_OUT}"" ""%1""";  

;Root: HKLM; Subkey: Software\Classes\.txt;                        ValueType:string;ValueName:""; Tasks:fileassoc; ValueData: "OnlyOffice.Txt.1";                Flags: uninsdeletevalue
;Root: HKLM; Subkey: {#REG_SC_PATH}.Txt.1;                         ValueType:string;ValueName:""; Tasks:fileassoc; ValueData: "Txt file";                        Flags: uninsdeletekey
;Root: HKLM; Subkey: {#REG_SC_PATH}.Txt.1\DefaultIcon;             ValueType:string;ValueName:""; Tasks:fileassoc; ValueData: "{app}\{#iconsExe},6";           
;Root: HKLM; Subkey: {#REG_SC_PATH}.Txt.1\shell\open\command;      ValueType:string;ValueName:""; Tasks:fileassoc; ValueData: """{app}\{#NAME_EXE_OUT}"" ""%1""";  

;Root: HKLM; Subkey: Software\Classes\.xps;                        ValueType:string;ValueName:""; Tasks:fileassoc; ValueData: "OnlyOffice.Xps.1";              Flags: uninsdeletevalue
;Root: HKLM; Subkey: {#REG_SC_PATH}.Xps.1;                         ValueType:string;ValueName:""; Tasks:fileassoc; ValueData: "Xps file";                      Flags: uninsdeletekey
;Root: HKLM; Subkey: {#REG_SC_PATH}.Xps.1\DefaultIcon;             ValueType:string;ValueName:""; Tasks:fileassoc; ValueData: "{app}\{#iconsExe},6";           
;Root: HKLM; Subkey: {#REG_SC_PATH}.Xps.1\shell\open\command;      ValueType:string;ValueName:""; Tasks:fileassoc; ValueData: """{app}\{#NAME_EXE_OUT}"" ""%1""";  

;Root: HKLM; Subkey: Software\Classes\.pdf;                        ValueType:string;ValueName:""; Tasks:fileassoc; ValueData: "OnlyOffice.Pdf.1";                Flags: uninsdeletevalue
;Root: HKLM; Subkey: {#REG_SC_PATH}.Pdf.1;                         ValueType:string;ValueName:""; Tasks:fileassoc; ValueData: "Pdf document";                    Flags: uninsdeletekey
;Root: HKLM; Subkey: {#REG_SC_PATH}.Pdf.1\DefaultIcon;             ValueType:string;ValueName:""; Tasks:fileassoc; ValueData: "{app}\{#iconsExe},5";           
;Root: HKLM; Subkey: {#REG_SC_PATH}.Pdf.1\shell\open\command;      ValueType:string;ValueName:""; Tasks:fileassoc; ValueData: """{app}\{#NAME_EXE_OUT}"" ""%1""";  

;Root: HKLM; Subkey: Software\Classes\.djvu;                       ValueType:string;ValueName:""; Tasks:fileassoc; ValueData: "OnlyOffice.Djvu.1";               Flags: uninsdeletevalue
;Root: HKLM; Subkey: {#REG_SC_PATH}.Djvu.1;                        ValueType:string;ValueName:""; Tasks:fileassoc; ValueData: "Djvu file";                       Flags: uninsdeletekey
;Root: HKLM; Subkey: {#REG_SC_PATH}.Djvu.1\DefaultIcon;            ValueType:string;ValueName:""; Tasks:fileassoc; ValueData: "{app}\{#iconsExe},4";           
;Root: HKLM; Subkey: {#REG_SC_PATH}.Djvu.1\shell\open\command;     ValueType:string;ValueName:""; Tasks:fileassoc; ValueData: """{app}\{#NAME_EXE_OUT}"" ""%1"""; 

Root: HKLM; Subkey: SYSTEM\CurrentControlSet\Control\Session Manager\Environment; ValueType: expandsz; ValueName: Path; ValueData: "{olddata};{app}\converter"; Check: NeedsAddPath(ExpandConstant('{app}\converter')); AfterInstall: RefreshEnvironment;

[UninstallDelete]
Type: filesandordirs; Name: {commonappdata}\{#ASC_PATH}\*;  AfterInstall: RefreshEnvironment;