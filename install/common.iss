#define sAppName            'ONLYOFFICE Desktop Editors'

#define NAME_EXE_OUT        'DesktopEditors.exe'
#define ASC_PATH            'ONLYOFFICE\DesktopEditors'
#define ASC_REG_PATH        'Software\ONLYOFFICE\DesktopEditors'

#define PATH_EXE            '..\Build\Release\release\DesktopEditors.exe'
#define sAppVersion         GetFileVersion(AddBackslash(SourcePath) + PATH_EXE)
#define sAppVerShort        Copy(GetFileVersion(AddBackslash(SourcePath) + PATH_EXE), 0, 3)

[Setup]
AppVerName              ={#sAppName} {#sAppVerShort}
VersionInfoVersion      ={#sAppVersion}
AppPublisher            =Ascensio System SIA.
AppPublisherURL         =http://www.onlyoffice.com/
AppSupportURL           =http://www.onlyoffice.com/support.aspx
UsePreviousAppDir       =no
DirExistsWarning        =no
DefaultDirName          ={pf}\{#ASC_PATH}
DefaultGroupName        =ONLYOFFICE
DisableProgramGroupPage = yes
AllowNoIcons            = yes
WizardImageFile         = data\dialogpicture.bmp
WizardSmallImageFile    = data\dialogicon.bmp
UninstallDisplayIcon    = {app}\{#NAME_EXE_OUT}
OutputDir               =.\
Compression             =lzma
PrivilegesRequired      =admin
AppMutex                =TEAMLAB

[Languages]
Name: en; MessagesFile: compiler:Default.isl;
Name: ru; MessagesFile: compiler:Languages\Russian.isl;
Name: de; MessagesFile: compiler:Languages\German.isl;
Name: fr; MessagesFile: compiler:Languages\French.isl;
Name: sp; MessagesFile: compiler:Languages\Spanish.isl;
;Name: it; MessagesFile: compiler:Languages\Italian.isl;

[CustomMessages]
en.Launch =Launch %1
ru.Launch =Запустить %1
de.Launch =%1 starten
fr.Launch =Lancer %1
sp.Launch =Ejecutar %1
;it.Launch =Eseguire %1
;======================================================================================================
en.CreateDesktopIcon =Create %1 &desktop icon
ru.CreateDesktopIcon =Создать иконку %1 на &рабочем столе
de.CreateDesktopIcon =%1 &Desktop-Icon erstellen
fr.CreateDesktopIcon =Crйer l'icфne du bureau pour %1
sp.CreateDesktopIcon =Crear %1 &icono en el escritorio
;it.CreateDesktopIcon =Creare un collegamento %1 sul &desktop
;======================================================================================================
en.InstallAdditionalComponents =Installing additional system components. Please wait...
ru.InstallAdditionalComponents =Установка дополнительных системных компонент. Пожалуйста, подождите...
de.InstallAdditionalComponents =Installation zusдtzlicher Systemkomponenten. Bitte warten...
fr.InstallAdditionalComponents =L'installation des composants supplйmentaires du systиme. Attendez...
sp.InstallAdditionalComponents =Instalando componentes adicionales del sistema. Por favor espere...
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
de.Uninstall =Deinstallieren
fr.Uninstall =Desinstaller
sp.Uninstall =Desinstalar
;it.Uninstall =Disinstalla
;======================================================================================================
en.WarningWrongArchitecture =You are trying to install the %1-bit application version over the %2-bit version installed. Please uninstall the previous version first or download the correct version for installation.
ru.WarningWrongArchitecture =Вы устанавливаете %1-битную версию приложения на уже установленную %2-битную. Пожалуйста, удалите предыдущую версию приложения или скачайте подходящую.
de.WarningWrongArchitecture =Sie versuchen die %1-Bit-Version der Anwendung über die %2-Bit-Version, die schon installiert ist, zu installieren. Entfernen Sie bitte die Vorgängerversion zuerst oder laden Sie die richtige Version für die Installation herunter.
fr.WarningWrongArchitecture =Vous essayez d'installer la version %1-bit sur la version %2-bit déjà installée. Veuillez désinstaller l'ancienne version d'abord ou télécharger la version correcte à installer.
sp.WarningWrongArchitecture =Usted está tratando de instalar la versión de la aplicación de %1 bits sobre la versión de %2 bits instalada. Por favor, desinstale la versión anterior primero o descargue la versión correcta para la instalación.
;it.Uninstall =Disinstalla

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


[Files]
Source: data\desktop_icons.ico;                    DestDir: {app}\;
Source: data\webdata\cloud\*;                      DestDir: {commonappdata}\{#ASC_PATH}\webdata\cloud; Flags: recursesubdirs;
Source: ..\loginpage\deploy\*;                     DestDir: {commonappdata}\{#ASC_PATH}\webdata\local;
Source: data\dictionaries\*;                       DestDir: {app}\dictionaries; Flags: recursesubdirs;

Source: data\fonts\LICENSE.txt;                    DestDir: {app}\;
Source: data\fonts\OpenSans-Bold.ttf;              DestDir: {fonts}; FontInstall: Open Sans Bold;             Flags: onlyifdoesntexist uninsneveruninstall;
Source: data\fonts\OpenSans-Bold.ttf;              DestDir: {fonts}; FontInstall: Open Sans Bold;             Flags: onlyifdoesntexist uninsneveruninstall;
;Source: data\fonts\OpenSans-BoldItalic.ttf;        DestDir: {fonts}; FontInstall: Open Sans Bold Italic;      Flags: onlyifdoesntexist uninsneveruninstall;
Source: data\fonts\OpenSans-Regular.ttf;           DestDir: {fonts}; FontInstall: Open Sans Regular;          Flags: onlyifdoesntexist uninsneveruninstall;
;Source: data\fonts\OpenSans-Italic.ttf;            DestDir: {fonts}; FontInstall: Open Sans Italic;           Flags: onlyifdoesntexist uninsneveruninstall;
Source: data\fonts\OpenSans-ExtraBold.ttf;         DestDir: {fonts}; FontInstall: Open Sans Extrabold;        Flags: onlyifdoesntexist uninsneveruninstall;
;Source: data\fonts\OpenSans-ExtraBoldItalic.ttf;   DestDir: {fonts}; FontInstall: Open Sans Extrabold Italic; Flags: onlyifdoesntexist uninsneveruninstall;
Source: data\fonts\OpenSans-Light.ttf;             DestDir: {fonts}; FontInstall: Open Sans Light;            Flags: onlyifdoesntexist uninsneveruninstall;
;Source: data\fonts\OpenSans-LightItalic.ttf;       DestDir: {fonts}; FontInstall: Open Sans Light Italic;     Flags: onlyifdoesntexist uninsneveruninstall;
Source: data\fonts\OpenSans-Semibold.ttf;          DestDir: {fonts}; FontInstall: Open Sans Semibold;         Flags: onlyifdoesntexist uninsneveruninstall;
;Source: data\fonts\OpenSans-SemiboldItalic.ttf;    DestDir: {fonts}; FontInstall: Open Sans Semibold Italic;  Flags: onlyifdoesntexist uninsneveruninstall;


[Tasks]
Name: desktopicon; Description: {cm:CreateDesktopIcon,{#sAppName}}; GroupDescription: {cm:AdditionalIcons};

[Icons]
;Name: {commondesktop}\{#sAppName}; FileName: {app}\{#NAME_EXE_OUT}; WorkingDir: {app}; Tasks: desktopicon;
Name: {commondesktop}\{#sAppName}; FileName: {app}\{#NAME_EXE_OUT}; WorkingDir: {app}; Tasks: desktopicon; IconFilename: {app}\desktop_icons.ico;
Name: {group}\{#sAppName};         Filename: {app}\{#NAME_EXE_OUT}; WorkingDir: {app}; IconFilename: {app}\desktop_icons.ico;
Name: {group}\{cm:Uninstall}; Filename: {uninstallexe}; WorkingDir: {app};

[Run]
Filename: {app}\{#NAME_EXE_OUT}; Description: {cm:Launch,{#sAppName}}; Flags: postinstall nowait skipifsilent
;Filename: http://www.onlyoffice.com/remove-portal-feedback-form.aspx; Description: Visit website; Flags: postinstall shellexec nowait 

[Ini]
;Filename: {app}\opts; Section: app; Key: lang; String: {language};

[Registry]
Root: HKLM; Subkey: {#ASC_REG_PATH};  Flags: uninsdeletekey;
Root: HKLM; Subkey: {#ASC_REG_PATH};  ValueType: string;   ValueName: lang;  ValueData: {language};
Root: HKLM; Subkey: {#ASC_REG_PATH};  ValueType: qword;    ValueName: timestamp;  ValueData: {code:getPosixTime};

[UninstallDelete]
Type: filesandordirs; Name: {commonappdata}\{#ASC_PATH}\*;  