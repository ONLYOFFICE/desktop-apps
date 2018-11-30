
#define sAppName            'ONLYOFFICE Desktop Editors'
#define APP_PATH            'ONLYOFFICE\DesktopEditors'
#define APP_REG_PATH        'Software\ONLYOFFICE\DesktopEditors'
#define APP_USER_MODEL_ID   'ASC.Documents.5'
#define sAppIconName        'ONLYOFFICE Editors'
#define NAME_EXE_IN         'DesktopEditors.exe'
#define NAME_EXE_OUT        'editors.exe'
#define iconsExe            'DesktopEditors.exe'
#define licfile             'agpl-3.0'
#define APPWND_CLASS_NAME   'DocEditorsWindowClass'

#define sAppVersion         GetFileVersion(AddBackslash(SourcePath) + '..\..\Build\Release\' + NAME_EXE_IN)
#define sAppVerShort        Copy(sAppVersion, 0, 3)

#include "utils.iss"
#include "associate_page.iss"

#define UNINSTALL_USE_CLEAR_PAGE
#ifdef UNINSTALL_USE_CLEAR_PAGE
# include "uninstall_page.iss"
#endif


[Setup]
AppName                   ={#sAppName}
AppVerName                ={#sAppName} {#sAppVerShort}
AppVersion                ={#sAppVersion}
VersionInfoVersion        ={#sAppVersion}

AppPublisher              =Ascensio System SIA.
AppPublisherURL           =http://www.onlyoffice.com/
AppSupportURL             =http://www.onlyoffice.com/support.aspx
AppCopyright              =Copyright (C) 2018 Ascensio System SIA.

DefaultGroupName          =ONLYOFFICE
WizardImageFile           = data\dialogpicture.bmp
WizardSmallImageFile      = data\dialogicon.bmp

;UsePreviousAppDir         =no
DirExistsWarning          =no
DefaultDirName            ={pf}\{#APP_PATH}
DisableProgramGroupPage   = yes
DisableWelcomePage        = no
DEPCompatible             = no
ASLRCompatible            = no
DisableDirPage            = auto
AllowNoIcons              = yes
AlwaysShowDirOnReadyPage  = yes
UninstallDisplayIcon      = {app}\{#NAME_EXE_OUT}
OutputDir                 =.\
Compression               =lzma
PrivilegesRequired        =admin
AppMutex                  ={code:getAppMutex}
ChangesEnvironment        =yes
SetupMutex                =ASC

[Languages]
Name: en; MessagesFile: compiler:Default.isl;             LicenseFile: ..\..\..\common\package\license\{#licfile}.rtf;
Name: ru; MessagesFile: compiler:Languages\Russian.isl;   LicenseFile: ..\..\..\common\package\license\{#licfile}.rtf;
Name: cs; MessagesFile: compiler:Languages\Czech.isl;     LicenseFile: ..\..\..\common\package\license\{#licfile}.rtf;
Name: sk; MessagesFile: compiler:Languages\Slovak.isl;    LicenseFile: ..\..\..\common\package\license\{#licfile}.rtf;
Name: de; MessagesFile: compiler:Languages\German.isl;    LicenseFile: ..\..\..\common\package\license\{#licfile}.rtf;
Name: fr; MessagesFile: compiler:Languages\French.isl;    LicenseFile: ..\..\..\common\package\license\{#licfile}.rtf;
Name: es; MessagesFile: compiler:Languages\Spanish.isl;   LicenseFile: ..\..\..\common\package\license\{#licfile}.rtf;
Name: pt_BR; MessagesFile: compiler:Languages\BrazilianPortuguese.isl; LicenseFile: ..\..\..\common\package\license\{#licfile}.rtf;
Name: it_IT; MessagesFile: compiler:Languages\Italian.isl; LicenseFile: ..\..\..\common\package\license\{#licfile}.rtf;
Name: pl; MessagesFile: compiler:Languages\Polish.isl;    LicenseFile: ..\..\..\common\package\license\{#licfile}.rtf;


[CustomMessages]
;======================================================================================================
en.Launch =Launch %1
cs.Launch =Spuštění %1
sk.Launch =Spustenie %1
ru.Launch =Запустить %1
de.Launch =%1 starten
fr.Launch =Lancer %1
es.Launch =Ejecutar %1
it_IT.Launch =Eseguire %1
pt_BR.Launch =Lance o %1
pl.Launch =Uruchom %1
;======================================================================================================
en.CreateDesktopIcon =Create %1 &desktop icon
cs.CreateDesktopIcon =Vytvořte %1 &ikonu pracovní plochy
sk.CreateDesktopIcon =Vytvoriť %1 &ikonu na ploche
ru.CreateDesktopIcon =Создать иконку %1 на &рабочем столе
de.CreateDesktopIcon =%1 &Desktop-Icon erstellen
fr.CreateDesktopIcon =Créer l'icône du bureau pour %1
es.CreateDesktopIcon =Crear %1 &icono en el escritorio
it_IT.CreateDesktopIcon =Creare un collegamento %1 sul &desktop
pt_BR.CreateDesktopIcon =Criar ícone de &desktop do %1
pl.CreateDesktopIcon =Stwórz %1 oraz ikonę pulpitu
;======================================================================================================
en.InstallAdditionalComponents =Installing additional system components. Please wait...
cs.InstallAdditionalComponents =Instalace dalších systémových komponent. Prosím, čekejte...
sk.InstallAdditionalComponents =Inštalácia ďalších systémových súčastí. Prosím čakajte...
ru.InstallAdditionalComponents =Установка дополнительных системных компонентов. Пожалуйста, подождите...
de.InstallAdditionalComponents =Installation zusätzlicher Systemkomponenten. Bitte warten...
fr.InstallAdditionalComponents =L'installation des composants supplémentaires du système. Attendez...
es.InstallAdditionalComponents =Instalando componentes adicionales del sistema. Por favor espere...
it_IT.InstallAdditionalComponents =Installazione dei componenti addizionali del sistema. Per favore, attendi...
pt_BR.InstallAdditionalComponents =Instalando componentes do sistema adicional. Aguarde...
pl.InstallAdditionalComponents =Instalacja dodatkowych elementów systemu. Proszę czekać...
;======================================================================================================
en.AdditionalTasks =Tasks:
cs.AdditionalTasks =Úkoly:
sk.AdditionalTasks =Úlohy:
ru.AdditionalTasks =Задачи:
de.AdditionalTasks =Aufgaben:
fr.AdditionalTasks =Tâches:
es.AdditionalTasks =Tareas:
it_IT.AdditionalTasks =Attività:
pt_BR.AdditionalTasks =Tarefas:
pl.AdditionalTasks =Zadania:
;======================================================================================================
en.Uninstall =Uninstall
cs.Uninstall =Odinstalovat
sk.Uninstall =Odinštalovať
ru.Uninstall =Удаление
de.Uninstall =Deinstallieren
fr.Uninstall =Desinstaller
es.Uninstall =Desinstalar
it_IT.Uninstall =Disinstalla
pt_BR.Uninstall =Desinstalar
pl.Uninstall =Odinstaluj
;======================================================================================================
en.WarningWrongArchitecture =You are trying to install the %1-bit application version over the %2-bit version installed. Please uninstall the previous version first or download the correct version for installation.
cs.WarningWrongArchitecture =Pokoušíte se nainstalovat %1-bit verzi aplikace na nainstalovanou %2-bitovou verzi. Nejprve odinstalujte předchozí verzi nebo stáhněte správnou verzi pro instalaci.
sk.WarningWrongArchitecture =Pokúšate sa nainštalovať %1-bitovej verziu na nainštalovanú %2-bitovú verziu. Najskôr odinštalujte predchádzajúcu verziu alebo stiahnite správnu verziu pre inštaláciu.
ru.WarningWrongArchitecture =Вы устанавливаете %1-битную версию приложения на уже установленную %2-битную. Пожалуйста, удалите предыдущую версию приложения или скачайте подходящую.
de.WarningWrongArchitecture =Sie versuchen die %1-Bit-Version der Anwendung über die %2-Bit-Version, die schon installiert ist, zu installieren. Entfernen Sie bitte die Vorgängerversion zuerst oder laden Sie die richtige Version für die Installation herunter.
fr.WarningWrongArchitecture =Vous essayez d'installer la version %1-bit sur la version %2-bit déjà installée. Veuillez désinstaller l'ancienne version d'abord ou télécharger la version correcte à installer.
es.WarningWrongArchitecture =Usted está tratando de instalar la versión de la aplicación de %1 bits sobre la versión de %2 bits instalada. Por favor, desinstale la versión anterior primero o descargue la versión correcta para la instalación.
it_IT.WarningWrongArchitecture =Stai provando ad installare la versione dell'applicazione %1-bit sulla versione %2-bit installata. Si prega di disinstallare prima la versione precedente o scaricare la versione corretta per l'installazione.
pt_BR.WarningWrongArchitecture =Você está tentando instalar a versão do aplicativo de %1 bits por cima da versão de %2 bits instalada. Desinstale primeiro a versão anterior ou baixe a versão correta para instalação.
pl.WarningWrongArchitecture =Próbujesz zainstalować %1-bitową wersję aplikacji na %2-bitowej wersji zainstalowanej. Odinstaluj najpierw poprzednią wersję lub pobierz odpowiednią wersję dla instalacji.
;======================================================================================================

en.UpdateAppRunning=Setup has detected that %1 is currently running.%n%nIt'll be closed automatically. Click OK to continue, or Cancel to exit.
cs.UpdateAppRunning=V rámci nastavení bylo zjištěno, že je aktuálně spuštěné 1%.%n%nBude automaticky zavřen. Chcete-li pokračovat, klikněte na tlačítko OK nebo Zrušit pro ukončení.
sk.UpdateAppRunning=Inštalátor zistil, že % 1 aktuálne prebieha.%n%nBude automaticky zatvorené. Ak chcete pokračovať, kliknite na tlačidlo OK.
ru.UpdateAppRunning=Обнаружен запущенный экземпляр %1.%n%nДля обновления он будет автоматически закрыт. Нажмите «OK», чтобы продолжить, или «Отмена», чтобы выйти.
de.UpdateAppRunning=Setup hat festgestellt, dass es aktuell %1 läuft. %n%nEs wird automatisch geschlossen. Klicken Sie zum Fortfahren auf OK oder auf Abbrechen zum Beenden des Programms.
fr.UpdateAppRunning=L'installation a détecté que %1 est en cours d'exécution. %n%nIl sera fermé automatiquement. Cliquez sur OK pour continuer, ou Annuler pour quitter le programme.
es.UpdateAppRunning=Programa de instalación ha detectado que actualmente %1 está funcionando.%n%nSe cerrará  automáticamente. Haga clic en OK para continuar o Cerrar para salir.
it_IT.UpdateAppRunning= Il programma di installazione ha rilevato che% 1 è attualmente in esecuzione.%n%nVerrà chiuso automaticamente. Fare clic su OK per continuare o su Annulla per uscire.
pt_BR.UpdateAppRunning=A configuração detectou que %1 está atualmente em execução.%n%nEla será fechada automaticamente. Clique em OK para continuar ou em Cancelar para sair.
pl.UpdateAppRunning=Konfiguracja wykryła , że %1 jest uruchomiona.%n%nZostanie ona automatycznie zamknięta. Kliknij OK, aby kontynuować lub Anuluj, aby wyjść.
;======================================================================================================
en.WarningClearAppData =Do you want to clear the user settings and application cached data?
cs.WarningClearAppData =Chcete zrušit uživatelské nastavení a údaje uložené v paměti?
sk.WarningClearAppData =Chcete zrušiť používateľské nastavenia a údaje uložené vo vyrovnávacej pamäti?
ru.WarningClearAppData =Вы хотите очистить пользовательские настройки и кэш приложения?
de.WarningClearAppData =Möchten Sie die Benutzereinstellungen und die zwischengespeicherten Daten der Anwendung löschen?
fr.WarningClearAppData =Voulez-vous effacer les paramètres utilisateur et les données en cache de l'application ?
es.WarningClearAppData =¿Desea eliminar los ajustes de usuario y datos en caché de la aplicación?
it_IT.WarningClearAppData =Vuoi cancellare le impostazioni utente e i dati memorizzati nella cache dell’applicazione?
pt_BR.WarningClearAppData =Você deseja limpar as definições de usuário e dados salvos do programa?
pl.WarningClearAppData =Czy chcesz usunąć ustawienia użytkownika oraz dane pamięci podręcznej aplikacji?
;======================================================================================================


;en.AssociateDescription =Associate office document file types with %1
;it_IT.AssociateDescription =Associa i file documentodi Office con %1
;cs.AssociateDescription =Asociovat typy souborů kancelářských dokumentů s %1
;sk.AssociateDescription =Asociovať typy súborov kancelárskych dokumentov %1
;ru.AssociateDescription =Ассоциировать типы файлов офисных документов с %1


[Code]
const
  SMTO_ABORTIFHUNG = 2;
  WM_WININICHANGE = $001A;
  WM_SETTINGCHANGE = WM_WININICHANGE;
  WM_USER = $400;

type
  WPARAM = UINT_PTR;
  LPARAM = INT_PTR;
  LRESULT = INT_PTR;

var
  gHWND: Longint;
  isInstalled: Boolean;

procedure GetSystemTimeAsFileTime(var lpFileTime: TFileTime); external 'GetSystemTimeAsFileTime@kernel32.dll';
function SendTextMessageTimeout(hWnd: HWND; Msg: UINT; wParam: WPARAM; lParam: PAnsiChar; fuFlags: UINT; uTimeout: UINT; out lpdwResult: DWORD): LRESULT;
  external 'SendMessageTimeoutA@user32.dll stdcall';

//procedure checkArchitectureVersion; forward;
function GetHKLM: Integer; forward;

procedure InitializeWizard();
var
  paramSkip: string;
  path: string;
begin
  InitializeAssociatePage();

  if RegQueryStringValue(GetHKLM(), '{#APP_REG_PATH}', 'AppPath', path) and
        FileExists(path + '\{#NAME_EXE_OUT}') then
    isInstalled := false
  else isInstalled := true;
end;

function InitializeSetup(): Boolean;
var
  OutResult: Boolean;
  path, mess: string;
  regkey: integer;

  hWnd: Longint;
  msg: string;
begin
  gHWND := 0;
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
        'SOFTWARE\ONLYOFFICE\DesktopEditors',
        'AppPath', path) then
    begin
      if FileExists(path + '\{#NAME_EXE_OUT}') then
      begin
        MsgBox(mess, mbInformation, MB_OK)
        OutResult := False
      end
    end
  end;

  if OutResult then begin
    if CheckCommandlineParam('/update') then
    begin
      gHWND := FindWindowByClassName('{#APPWND_CLASS_NAME}');
      if gHWND <> 0 then begin
        OutResult := (IDOK = MsgBox(ExpandConstant('{cm:UpdateAppRunning,{#sAppName}}'), mbInformation, MB_OKCANCEL));
        if OutResult then begin
          PostMessage(gHWND, WM_USER+254, 0, 0);
          Sleep(1000);

          while true do begin
            hWnd := FindWindowByClassName('{#APPWND_CLASS_NAME}');
            if hWnd <> 0 then begin
              msg := FmtMessage(SetupMessage(msgSetupAppRunningError), ['{#sAppName}']);
              if IDCANCEL = MsgBox(msg, mbError, MB_OKCANCEL) then begin
                OutResult := false;
                break;
              end;
            end else
              break;
          end;
        end;
      end;
    end;
  end;

  Result := OutResult;
end;

procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
var
  regValue, userPath: string;
  findRec: TFindRec;
begin
  if CurUninstallStep = usUninstall then
  begin
    RegQueryStringValue(GetHKLM(), ExpandConstant('{#APP_REG_PATH}'), 'uninstall', regValue);

    if (regValue <> 'full') and
#ifndef UNINSTALL_USE_CLEAR_PAGE
        (MsgBox(ExpandConstant('{cm:WarningClearAppData}'), mbConfirmation, MB_YESNO) = IDYES)
#else
        IsClearData
#endif
            then regValue := 'soft';

    userPath := ExpandConstant('{localappdata}\ONLYOFFICE');
    if regValue = 'soft' then begin
      RegDeleteKeyIncludingSubkeys(GetHKLM(), 'Software\ONLYOFFICE');
      RegDeleteKeyIncludingSubkeys(HKEY_CURRENT_USER, 'Software\ONLYOFFICE');

      // remove all app and user cashed data except of folders 'recover' and 'sdkjs-plugins'
      userPath := userPath + '\DesktopEditors';
      DelTree(userPath + '\*', False, True, False);

      userPath := userPath + '\data';
      if FindFirst(userPath + '\*', findRec) then begin
        try repeat
            if findRec.Attributes and FILE_ATTRIBUTE_DIRECTORY = 0 then
              DeleteFile(userPath + '\' + findRec.Name)
            else if (findRec.Name <> '.') and (findRec.Name <> '..') and
                (findRec.Name <> 'recover') and (findRec.Name <> 'sdkjs-plugins') then begin
              DelTree(userPath + '\' + findRec.Name, True, True, True);
            end;
          until not FindNext(findRec);
        finally
          FindClose(findRec);
        end;
      end;

    end else
    if regValue = 'full' then begin
      DelTree(userPath, True, True, True);
      RegDeleteKeyIncludingSubkeys(GetHKLM(), 'Software\ONLYOFFICE');
      RegDeleteKeyIncludingSubkeys(HKEY_CURRENT_USER, 'Software\ONLYOFFICE');
    end;

    UnassociateExtensions();
  end else
  if CurUninstallStep = usPostUninstall then begin
  end;
end;

procedure CurStepChanged(CurStep: TSetupStep);
var
  commonCachePath, userCachePath: string;
  paramStore: string;
  ErrorCode: Integer;
begin
  if CurStep = ssPostInstall then begin
    DoPostInstall();

    // migrate from the prev version when user's data saved to system common path
    commonCachePath := ExpandConstant('{commonappdata}\{#APP_PATH}\data\cache');
    userCachePath := ExpandConstant('{localappdata}\{#APP_PATH}\data\cache');
    if DirExists(commonCachePath) then begin
      ForceDirectories(userCachePath);
      DirectoryCopy(commonCachePath, userCachePath);
    end;

    paramStore := GetCommandlineParam('/store');
    if Length(paramStore) > 0 then begin
      RegWriteStringValue(HKEY_LOCAL_MACHINE, ExpandConstant('{#APP_REG_PATH}'), 'Store', paramStore);
    end;

    paramStore := GetCommandlineParam('/uninst');
    if (Length(paramStore) > 0) and (paramStore = 'full') then begin
      RegWriteStringValue(HKEY_LOCAL_MACHINE, ExpandConstant('{#APP_REG_PATH}'), 'uninstall', paramStore);
    end;
  end else
  if CurStep = ssDone then begin
    if not (gHWND = 0) then begin
      ShellExec('', ExpandConstant('{app}\{#iconsExe}'), '', '', SW_SHOW, ewNoWait, ErrorCode);
    end
  end else
    WizardForm.CancelButton.Enabled := isInstalled;
end;

function PrepareToInstall(var NeedsRestart: Boolean): String;
var
  path: string;
begin
  path := ExpandConstant('{app}\editors\web-apps');
  if DirExists(path) then DelTree(path, true, true, true);

  path := ExpandConstant('{app}\editors\sdkjs');
  if DirExists(path) then DelTree(path, true, true, true)
end;

function getAppMutex(P: String): String;
var
  hWnd: Longint;
begin
  if not CheckCommandlineParam('/update') then
    Result := 'TEAMLAB'
  else
    Result := 'UPDATE';
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

function getAppPrevLang(param: string): string;
var
  lang: string;
begin
  if not (WizardSilent and
        RegValueExists(GetHKLM(), '{#APP_REG_PATH}', 'locale') and
            RegQueryStringValue(GetHKLM(), '{#APP_REG_PATH}', 'locale', lang)) then
  begin
    lang := ExpandConstant('{language}')
    if (Length(lang) > 2) and (lang[3] = '_') then StringChangeEx(lang, '_', '-', false);
  end;

  result := lang;
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

[Dirs]
Name: {commonappdata}\{#APP_PATH}\webdata\cloud; Flags: uninsalwaysuninstall;


[Files]

Source: data\vcredist\{#VC_REDIST_VER};       DestDir: {app}\; Flags: deleteafterinstall; \
    AfterInstall: installVCRedist(ExpandConstant('{app}\{#VC_REDIST_VER}'), ExpandConstant('{cm:InstallAdditionalComponents}')); Check: not checkVCRedist;

#ifndef SCRIPT_CUSTOM_FILES
Source: ..\..\deploy\{#os_arch}\3dparty\Qt\*;                   DestDir: {app}; Flags: ignoreversion recursesubdirs;

Source: .\data\projicons.exe;                                   DestDir: {app}; DestName: {#iconsExe};
Source: ..\..\build\Release\{#NAME_EXE_IN};                     DestDir: {app}; DestName: {#NAME_EXE_OUT};

Source: ..\..\res\icons\desktopeditors.ico;                     DestDir: {app}; DestName: app.ico;
Source: ..\..\..\common\loginpage\deploy\index.html;            DestDir: {app}; DestName: index.html;
Source: ..\..\..\common\package\license\{#licfile}.htm;         DestDir: {app}; DestName: LICENSE.htm;
Source: ..\..\..\common\package\license\3dparty\3DPARTYLICENSE; DestDir: {app};
;Source: data\webdata\cloud\*;                      DestDir: {commonappdata}\{#APP_PATH}\webdata\cloud; Flags: recursesubdirs;
;Source: ..\..\common\loginpage\deploy\*;           DestDir: {commonappdata}\{#APP_PATH}\webdata\local;
Source: ..\..\..\..\dictionaries\*;                             DestDir: {app}\dictionaries; Flags: recursesubdirs;

Source: ..\..\..\..\core\build\jsdesktop\web-apps\*;            DestDir: {app}\editors\web-apps;      Flags: recursesubdirs;
Source: ..\..\..\..\core\build\jsdesktop\sdkjs\*;               DestDir: {app}\editors\sdkjs;         Flags: recursesubdirs;
Source: ..\..\..\..\core\build\jsdesktop\sdkjs-plugins\*;       DestDir: {app}\editors\sdkjs-plugins; Flags: recursesubdirs;
Source: ..\..\..\common\loginpage\addon\externalcloud.json;     DestDir: {app}\editors;               Flags: recursesubdirs;
Source: ..\..\..\common\converter\empty\*;                      DestDir: {app}\converter\empty;       Flags: recursesubdirs;
;Source: ..\..\..\common\converter\empty\ru-RU\*.*;              DestDir: {app}\converter\empty\ru;
;Source: ..\..\..\common\converter\empty\fr-FR\*.*;              DestDir: {app}\converter\empty\fr;
;Source: ..\..\..\common\converter\empty\es-ES\*.*;              DestDir: {app}\converter\empty\es;
;Source: ..\..\..\common\converter\empty\de-DE\*.*;              DestDir: {app}\converter\empty\de;
;Source: ..\..\..\common\converter\empty\cs-CZ\*.*;              DestDir: {app}\converter\empty\cs;
;Source: ..\..\..\common\converter\empty\it-IT\*.*;              DestDir: {app}\converter\empty\it-IT;
;Source: ..\..\..\common\converter\empty\pt-BR\*.*;              DestDir: {app}\converter\empty\pt-BR;
Source: ..\..\..\common\converter\DoctRenderer.config;          DestDir: {app}\converter;

Source: ..\..\deploy\{#os_arch}\libs\*; DestDir: {app}\converter; Excludes: *.lib,*.exp,*.exe,ascdocumentscore.dll,ooxmlsignature.dll,hunspell.dll; Flags: ignoreversion;

Source: ..\..\deploy\{#os_arch}\libs\HtmlFileInternal.exe;      DestDir: {app}; Flags: ignoreversion;
Source: ..\..\deploy\{#os_arch}\libs\hunspell.dll;              DestDir: {app}; Flags: ignoreversion;
Source: ..\..\deploy\{#os_arch}\libs\ooxmlsignature.dll;        DestDir: {app}; Flags: ignoreversion;
Source: ..\..\deploy\{#os_arch}\libs\x2t.exe;                   DestDir: {app}\converter; Flags: ignoreversion;
#if defined _WIN_XP
Source: ..\..\..\..\core\build\lib\{#os_arch}\xp\ascdocumentscore.dll;   DestDir: {app}; Flags: ignoreversion;
#else
Source: ..\..\deploy\{#os_arch}\libs\ascdocumentscore.dll;      DestDir: {app}; Flags: ignoreversion;
#endif

Source: ..\..\..\..\core\Common\3dParty\icu\{#os_arch}\build\icu*58.dll;  DestDir: {app}\converter; Flags: ignoreversion;
Source: ..\..\..\..\core\Common\3dParty\cef\{#os_arch}\build\*;           DestDir: {app}; Excludes: *.lib; Flags: ignoreversion recursesubdirs;

#ifdef _ENCRYPT_MODULE
Source: ..\..\..\..\desktop-sdk\ChromiumBasedEditors\plugins\encrypt\ui\common\*; DestDir: {app}\editors\sdkjs-plugins; Flags: recursesubdirs;
Source: ..\..\..\..\desktop-sdk\ChromiumBasedEditors\plugins\encrypt\ui\engine\blockchain\*; DestDir: {app}\editors\sdkjs-plugins; Flags: recursesubdirs;
#endif

#ifdef _UPDMODULE
Source: ..\..\3dparty\WinSparkle\{#os_arch}\WinSparkle.dll;           DestDir: {app}\; Flags: ignoreversion;
#endif

Source: ..\..\..\common\package\fonts\LICENSE.txt;                    DestDir: {app}\fonts;
Source: ..\..\..\common\package\fonts\OpenSans-Bold.ttf;              DestDir: {app}\fonts; Flags: onlyifdoesntexist;
Source: ..\..\..\common\package\fonts\OpenSans-Regular.ttf;           DestDir: {app}\fonts; Flags: onlyifdoesntexist;
Source: ..\..\..\common\package\fonts\OpenSans-ExtraBold.ttf;         DestDir: {app}\fonts; Flags: onlyifdoesntexist;
Source: ..\..\..\common\package\fonts\OpenSans-Light.ttf;             DestDir: {app}\fonts; Flags: onlyifdoesntexist;
Source: ..\..\..\common\package\fonts\OpenSans-Semibold.ttf;          DestDir: {app}\fonts; Flags: onlyifdoesntexist;
;Source: data\fonts\OpenSans-ExtraBoldItalic.ttf;           DestDir: {fonts}; FontInstall: Open Sans Extrabold Italic; Flags: onlyifdoesntexist uninsneveruninstall;
;Source: data\fonts\OpenSans-BoldItalic.ttf;                DestDir: {fonts}; FontInstall: Open Sans Bold Italic;      Flags: onlyifdoesntexist uninsneveruninstall;
;Source: data\fonts\OpenSans-Italic.ttf;                    DestDir: {fonts}; FontInstall: Open Sans Italic;           Flags: onlyifdoesntexist uninsneveruninstall;
;Source: data\fonts\OpenSans-LightItalic.ttf;               DestDir: {fonts}; FontInstall: Open Sans Light Italic;     Flags: onlyifdoesntexist uninsneveruninstall;
;Source: data\fonts\OpenSans-SemiboldItalic.ttf;            DestDir: {fonts}; FontInstall: Open Sans Semibold Italic;  Flags: onlyifdoesntexist uninsneveruninstall;

Source: ..\..\..\common\package\fonts\ASANA.ttc;               DestDir: {app}\fonts; Flags: onlyifdoesntexist;
Source: ..\..\..\common\package\fonts\Carlito-Bold.ttf;        DestDir: {app}\fonts; Flags: onlyifdoesntexist;
Source: ..\..\..\common\package\fonts\Carlito-BoldItalic.ttf;  DestDir: {app}\fonts; Flags: onlyifdoesntexist;
Source: ..\..\..\common\package\fonts\Carlito-Italic.ttf;      DestDir: {app}\fonts; Flags: onlyifdoesntexist;
Source: ..\..\..\common\package\fonts\Carlito-Regular.ttf;     DestDir: {app}\fonts; Flags: onlyifdoesntexist;
#else
Source: ..\..\..\ONLYOFFICE\DesktopEditors\*;     DestDir: {app}; Flags: recursesubdirs;

#ifdef _UPDMODULE
Source: data\winsparkle\WinSparkle.dll;           DestDir: {app}\; Flags: ignoreversion;
#endif

#endif

[Tasks]
Name: desktopicon; Description: {cm:CreateDesktopIcon,{#sAppName}}; GroupDescription: {cm:AdditionalIcons};
;Name: fileassoc; Description: {cm:AssociateCaption};   GroupDescription: {cm:AssociateDescription};


[Icons]
;Name: {commondesktop}\{#sAppName}; FileName: {app}\{#NAME_EXE_OUT}; WorkingDir: {app}; Tasks: desktopicon;
Name: {commondesktop}\{#sAppIconName}; FileName: {app}\{#iconsExe}; WorkingDir: {app}; Tasks: desktopicon; IconFilename: {app}\app.ico; AppUserModelID: {#APP_USER_MODEL_ID};
Name: {group}\{#sAppIconName};         Filename: {app}\{#iconsExe}; WorkingDir: {app}; IconFilename: {app}\app.ico; AppUserModelID: {#APP_USER_MODEL_ID};
Name: {group}\{cm:Uninstall}; Filename: {uninstallexe}; WorkingDir: {app};


[Run]
;Filename: {app}\{#NAME_EXE_OUT}; Description: {cm:Launch,{#sAppName}}; Flags: postinstall nowait skipifsilent;
Filename: {app}\{#iconsExe}; Description: {cm:Launch,{#sAppName}}; Flags: postinstall nowait skipifsilent;
;Filename: http://www.onlyoffice.com/remove-portal-feedback-form.aspx; Description: Visit website; Flags: postinstall shellexec nowait 


[Ini]
;Filename: {app}\opts; Section: app; Key: lang; String: {language};


[Registry]
;Root: HKLM; Subkey: {#APP_REG_PATH};  Flags: uninsdeletekey;
Root: HKLM; Subkey: {#APP_REG_PATH};  ValueType: string;   ValueName: AppPath;    ValueData: {app};               Flags: uninsdeletevalue;
Root: HKLM; Subkey: {#APP_REG_PATH};  ValueType: string;   ValueName: locale;     ValueData: {code:getAppPrevLang}; Flags: uninsdeletevalue;
Root: HKCU; Subkey: {#APP_REG_PATH};  ValueType: string;   ValueName: locale;     ValueData: {code:getAppPrevLang}; Flags: uninsdeletevalue;
Root: HKLM; Subkey: {#APP_REG_PATH};  ValueType: qword;    ValueName: timestamp;  ValueData: {code:getPosixTime}; Flags: uninsdeletevalue;

[UninstallDelete]
Type: filesandordirs; Name: {commonappdata}\{#APP_PATH}\*;  AfterInstall: RefreshEnvironment;
