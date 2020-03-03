;-- Common --

#if str(_ARCH) == "64"
  #define sWinArch                  "x64"
  #define sPlatform                 "win_64"
#elif str(_ARCH) == "32"
  #define sWinArch                  "x86"
  #define sPlatform                 "win_32"
#endif
#ifndef _WIN_XP
  #define sWinArchFull              sWinArch
  #define sPlatformFull             sPlatform
#else
  #define sWinArchFull              sWinArch + "_xp"
  #define sPlatformFull             sPlatform + "_xp"
#endif

#ifndef sBrandingFolder
  #define sBrandingFolder           "..\..\.."
#endif
#define sBrandingFile               sBrandingFolder + "\win-linux\package\windows\branding.iss"
#if FileExists(sBrandingFile)
  #include sBrandingFile
#endif

#ifndef sCompanyName
  #define sCompanyName              "ONLYOFFICE"
#endif
#ifndef sIntCompanyName
  #define sIntCompanyName           sCompanyName
#endif
#ifndef sProductName
  #define sProductName              "Desktop Editors"
#endif
#ifndef sIntProductName
  #define sIntProductName           "DesktopEditors"
#endif
#ifndef sAppName
  #define sAppName                  sCompanyName + " " + sProductName
#endif
#ifndef sAppPublisher
  #define sAppPublisher             "Ascensio System SIA"
#endif
#ifndef sAppPublisherURL
  #define sAppPublisherURL          "https://www.onlyoffice.com/"
#endif
#ifndef sAppSupportURL
  #define sAppSupportURL            "https://www.onlyoffice.com/support.aspx"
#endif
#ifndef sAppCopyright
  #define sAppCopyright             str("Copyright (C) " + GetDateTimeString('yyyy',,) + " " + sAppPublisher)
#endif
#ifndef sAppIconName
  #define sAppIconName              "ONLYOFFICE Editors"
#endif

#ifndef APP_PATH
  #define APP_PATH                  sIntCompanyName + "\" + sIntProductName
#endif
#ifndef APP_REG_PATH
  #define APP_REG_PATH              "Software\" + APP_PATH
#endif
#ifndef DEPLOY_PATH
  #define DEPLOY_PATH               "..\..\..\..\build_tools\out\" + sPlatformFull + "\" + APP_PATH
#endif
#ifndef APP_USER_MODEL_ID
  #define APP_USER_MODEL_ID         "ASC.Documents.5"
#endif
#ifndef APP_MUTEX_NAME
  #define APP_MUTEX_NAME            "TEAMLAB"
#endif
#ifndef APPWND_CLASS_NAME
  #define APPWND_CLASS_NAME         "DocEditorsWindowClass"
#endif

#ifndef iconsExe
  #define iconsExe                  "DesktopEditors.exe"
#endif
#ifndef NAME_EXE_IN
  #define NAME_EXE_IN               "DesktopEditors_" + sWinArch + ".exe"
#endif
#ifndef NAME_EXE_OUT
  #define NAME_EXE_OUT              "editors.exe"
#endif
#define VISEFFECTS_MANIFEST_NAME    ChangeFileExt(iconsExe, "VisualElementsManifest.xml")
#ifndef LIC_FILE
  #define LIC_FILE                  "agpl-3.0"
#endif

#ifndef SCRIPT_CUSTOM_FILES
#  define sAppVersion         GetFileVersion(AddBackslash(SourcePath) + '..\..\Build\Release\' + NAME_EXE_IN)
  #ifndef sAppVersion
    #define sAppVersion             "0.0.0.0"
  #endif
#else
  #define sAppVersion               GetFileVersion(AddBackslash(DEPLOY_PATH) + NAME_EXE_OUT)
#endif
#define sAppVerShort        Copy(sAppVersion, 0, 3)

#ifndef sOutputFileName
  #define sOutputFileName           sIntCompanyName + '_' + sIntProductName + '_' + sAppVersion + '_' + sWinArchFull
#endif

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

AppPublisher              = {#sAppPublisher}
AppPublisherURL           = {#sAppPublisherURL}
AppSupportURL             = {#sAppSupportURL}
AppCopyright              = {#sAppCopyright}
AppComments               = {cm:defprogAppDescription}

DefaultGroupName          = {#sCompanyName}
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
UninstallDisplayIcon      = {app}\app.ico
OutputDir                 =.\
Compression               =lzma
PrivilegesRequired        =admin
AppMutex                  ={code:getAppMutex}
ChangesEnvironment        =yes
SetupMutex                =ASC

#if str(_ARCH) == "64"
ArchitecturesAllowed              = x64
ArchitecturesInstallIn64BitMode   = x64
#endif

#ifndef _WIN_XP
MinVersion                        = 6.1
#else
MinVersion                        = 5.0
OnlyBelowVersion                  = 6.1
#endif
OutputBaseFileName                = {#sOutputFileName}

#ifdef ENABLE_SIGNING
SignTool                  =byparam $p
#endif

SetupIconFile                     = {#sBrandingFolder}\win-linux\extras\projicons\res\desktopeditors.ico
WizardImageFile                   = {#sBrandingFolder}\win-linux\package\windows\data\dialogpicture.bmp
WizardSmallImageFile              = {#sBrandingFolder}\win-linux\package\windows\data\dialogicon.bmp

[Languages]
#ifdef _ONLYOFFICE
Name: en; MessagesFile: compiler:Default.isl;              LicenseFile: {#sBrandingFolder}\common\package\license\{#LIC_FILE}.rtf;
Name: ru; MessagesFile: compiler:Languages\Russian.isl;    LicenseFile: {#sBrandingFolder}\common\package\license\{#LIC_FILE}.rtf;
#else
Name: ru; MessagesFile: compiler:Languages\Russian.isl;    LicenseFile: {#sBrandingFolder}\common\package\license\{#LIC_FILE}.rtf;
Name: en; MessagesFile: compiler:Default.isl;              LicenseFile: {#sBrandingFolder}\common\package\license\{#LIC_FILE}.rtf;
#endif
Name: cs_CZ; MessagesFile: compiler:Languages\Czech.isl;   LicenseFile: {#sBrandingFolder}\common\package\license\{#LIC_FILE}.rtf;
Name: sk; MessagesFile: compiler:Languages\Slovak.isl;     LicenseFile: {#sBrandingFolder}\common\package\license\{#LIC_FILE}.rtf;
Name: de; MessagesFile: compiler:Languages\German.isl;     LicenseFile: {#sBrandingFolder}\common\package\license\{#LIC_FILE}.rtf;
Name: fr; MessagesFile: compiler:Languages\French.isl;     LicenseFile: {#sBrandingFolder}\common\package\license\{#LIC_FILE}.rtf;
Name: es; MessagesFile: compiler:Languages\Spanish.isl;    LicenseFile: {#sBrandingFolder}\common\package\license\{#LIC_FILE}.rtf;
Name: pt_BR; MessagesFile: compiler:Languages\BrazilianPortuguese.isl; LicenseFile: {#sBrandingFolder}\common\package\license\{#LIC_FILE}.rtf;
Name: it_IT; MessagesFile: compiler:Languages\Italian.isl; LicenseFile: {#sBrandingFolder}\common\package\license\{#LIC_FILE}.rtf;
Name: pl; MessagesFile: compiler:Languages\Polish.isl;     LicenseFile: {#sBrandingFolder}\common\package\license\{#LIC_FILE}.rtf;
Name: zh_CN; MessagesFile: compiler:Languages\ChineseTraditional.isl;  LicenseFile: {#sBrandingFolder}\common\package\license\{#LIC_FILE}.rtf;


[CustomMessages]
;======================================================================================================
en.Launch =Launch %1
cs_CZ.Launch =Spuštění %1
sk.Launch =Spustenie %1
ru.Launch =Запустить %1
de.Launch =%1 starten
fr.Launch =Lancer %1
es.Launch =Ejecutar %1
it_IT.Launch =Eseguire %1
pt_BR.Launch =Lance o %1
pl.Launch =Uruchom %1
zh_CN.Launch =启动%1
;======================================================================================================
en.CreateDesktopIcon =Create %1 &desktop icon
cs_CZ.CreateDesktopIcon =Vytvořte %1 &ikonu pracovní plochy
sk.CreateDesktopIcon =Vytvoriť %1 &ikonu na ploche
ru.CreateDesktopIcon =Создать иконку %1 на &рабочем столе
de.CreateDesktopIcon =%1 &Desktop-Icon erstellen
fr.CreateDesktopIcon =Créer l'icône du bureau pour %1
es.CreateDesktopIcon =Crear %1 &icono en el escritorio
it_IT.CreateDesktopIcon =Creare un collegamento %1 sul &desktop
pt_BR.CreateDesktopIcon =Criar ícone de &desktop do %1
pl.CreateDesktopIcon =Stwórz %1 oraz ikonę pulpitu
zh_CN.CreateDesktopIcon =创建%1和桌面图标
;======================================================================================================
en.InstallAdditionalComponents =Installing additional system components. Please wait...
cs_CZ.InstallAdditionalComponents =Instalace dalších systémových komponent. Prosím, čekejte...
sk.InstallAdditionalComponents =Inštalácia ďalších systémových súčastí. Prosím čakajte...
ru.InstallAdditionalComponents =Установка дополнительных системных компонентов. Пожалуйста, подождите...
de.InstallAdditionalComponents =Installation zusätzlicher Systemkomponenten. Bitte warten...
fr.InstallAdditionalComponents =L'installation des composants supplémentaires du système. Attendez...
es.InstallAdditionalComponents =Instalando componentes adicionales del sistema. Por favor espere...
it_IT.InstallAdditionalComponents =Installazione dei componenti addizionali del sistema. Per favore, attendi...
pt_BR.InstallAdditionalComponents =Instalando componentes do sistema adicional. Aguarde...
pl.InstallAdditionalComponents =Instalacja dodatkowych elementów systemu. Proszę czekać...
zh_CN.InstallAdditionalComponents =安装其他系统组件。请稍候...
;======================================================================================================
en.AdditionalTasks =Tasks:
cs_CZ.AdditionalTasks =Úkoly:
sk.AdditionalTasks =Úlohy:
ru.AdditionalTasks =Задачи:
de.AdditionalTasks =Aufgaben:
fr.AdditionalTasks =Tâches:
es.AdditionalTasks =Tareas:
it_IT.AdditionalTasks =Attività:
pt_BR.AdditionalTasks =Tarefas:
pl.AdditionalTasks =Zadania:
zh_CN.AdditionalTasks =任务：
;======================================================================================================
en.Uninstall =Uninstall
cs_CZ.Uninstall =Odinstalovat
sk.Uninstall =Odinštalovať
ru.Uninstall =Удаление
de.Uninstall =Deinstallieren
fr.Uninstall =Desinstaller
es.Uninstall =Desinstalar
it_IT.Uninstall =Disinstalla
pt_BR.Uninstall =Desinstalar
pl.Uninstall =Odinstaluj
zh_CN.Uninstall =卸载
;======================================================================================================
en.WarningWrongArchitecture =You are trying to install the %1-bit application version over the %2-bit version installed. Please uninstall the previous version first or download the correct version for installation.
cs_CZ.WarningWrongArchitecture =Pokoušíte se nainstalovat %1-bit verzi aplikace na nainstalovanou %2-bitovou verzi. Nejprve odinstalujte předchozí verzi nebo stáhněte správnou verzi pro instalaci.
sk.WarningWrongArchitecture =Pokúšate sa nainštalovať %1-bitovej verziu na nainštalovanú %2-bitovú verziu. Najskôr odinštalujte predchádzajúcu verziu alebo stiahnite správnu verziu pre inštaláciu.
ru.WarningWrongArchitecture =Вы устанавливаете %1-битную версию приложения на уже установленную %2-битную. Пожалуйста, удалите предыдущую версию приложения или скачайте подходящую.
de.WarningWrongArchitecture =Sie versuchen die %1-Bit-Version der Anwendung über die %2-Bit-Version, die schon installiert ist, zu installieren. Entfernen Sie bitte die Vorgängerversion zuerst oder laden Sie die richtige Version für die Installation herunter.
fr.WarningWrongArchitecture =Vous essayez d'installer la version %1-bit sur la version %2-bit déjà installée. Veuillez désinstaller l'ancienne version d'abord ou télécharger la version correcte à installer.
es.WarningWrongArchitecture =Usted está tratando de instalar la versión de la aplicación de %1 bits sobre la versión de %2 bits instalada. Por favor, desinstale la versión anterior primero o descargue la versión correcta para la instalación.
it_IT.WarningWrongArchitecture =Stai provando ad installare la versione dell'applicazione %1-bit sulla versione %2-bit installata. Si prega di disinstallare prima la versione precedente o scaricare la versione corretta per l'installazione.
pt_BR.WarningWrongArchitecture =Você está tentando instalar a versão do aplicativo de %1 bits por cima da versão de %2 bits instalada. Desinstale primeiro a versão anterior ou baixe a versão correta para instalação.
pl.WarningWrongArchitecture =Próbujesz zainstalować %1-bitową wersję aplikacji na %2-bitowej wersji zainstalowanej. Odinstaluj najpierw poprzednią wersję lub pobierz odpowiednią wersję dla instalacji.
zh_CN.WarningWrongArchitecture =您正在尝试在已安装的%2-bit版本上安装%1-bit应用版本。请首先卸载之前版本，或下载正确的安装版本。
;======================================================================================================

en.UpdateAppRunning=Setup has detected that %1 is currently running.%n%nIt'll be closed automatically. Click OK to continue, or Cancel to exit.
cs_CZ.UpdateAppRunning=V rámci nastavení bylo zjištěno, že je aktuálně spuštěné 1%.%n%nBude automaticky zavřen. Chcete-li pokračovat, klikněte na tlačítko OK nebo Zrušit pro ukončení.
sk.UpdateAppRunning=Inštalátor zistil, že % 1 aktuálne prebieha.%n%nBude automaticky zatvorené. Ak chcete pokračovať, kliknite na tlačidlo OK.
ru.UpdateAppRunning=Обнаружен запущенный экземпляр %1.%n%nДля обновления он будет автоматически закрыт. Нажмите «OK», чтобы продолжить, или «Отмена», чтобы выйти.
de.UpdateAppRunning=Setup hat festgestellt, dass es aktuell %1 läuft. %n%nEs wird automatisch geschlossen. Klicken Sie zum Fortfahren auf OK oder auf Abbrechen zum Beenden des Programms.
fr.UpdateAppRunning=L'installation a détecté que %1 est en cours d'exécution. %n%nIl sera fermé automatiquement. Cliquez sur OK pour continuer, ou Annuler pour quitter le programme.
es.UpdateAppRunning=Programa de instalación ha detectado que actualmente %1 está funcionando.%n%nSe cerrará  automáticamente. Haga clic en OK para continuar o Cerrar para salir.
it_IT.UpdateAppRunning= Il programma di installazione ha rilevato che% 1 è attualmente in esecuzione.%n%nVerrà chiuso automaticamente. Fare clic su OK per continuare o su Annulla per uscire.
pt_BR.UpdateAppRunning=A configuração detectou que %1 está atualmente em execução.%n%nEla será fechada automaticamente. Clique em OK para continuar ou em Cancelar para sair.
pl.UpdateAppRunning=Konfiguracja wykryła , że %1 jest uruchomiona.%n%nZostanie ona automatycznie zamknięta. Kliknij OK, aby kontynuować lub Anuluj, aby wyjść.
zh_CN.UpdateAppRunning=安装程序检测到%1当前正在运行。%n%n将自动关闭。单击“确定”继续，或“取消”退出。
;======================================================================================================
en.WarningClearAppData =Do you want to clear the user settings and application cached data?
cs_CZ.WarningClearAppData =Chcete zrušit uživatelské nastavení a údaje uložené v paměti?
sk.WarningClearAppData =Chcete zrušiť používateľské nastavenia a údaje uložené vo vyrovnávacej pamäti?
ru.WarningClearAppData =Вы хотите очистить пользовательские настройки и кэш приложения?
de.WarningClearAppData =Möchten Sie die Benutzereinstellungen und die zwischengespeicherten Daten der Anwendung löschen?
fr.WarningClearAppData =Voulez-vous effacer les paramètres utilisateur et les données en cache de l'application ?
es.WarningClearAppData =¿Desea eliminar los ajustes de usuario y datos en caché de la aplicación?
it_IT.WarningClearAppData =Vuoi cancellare le impostazioni utente e i dati memorizzati nella cache dell’applicazione?
pt_BR.WarningClearAppData =Você deseja limpar as definições de usuário e dados salvos do programa?
pl.WarningClearAppData =Czy chcesz usunąć ustawienia użytkownika oraz dane pamięci podręcznej aplikacji?
zh_CN.WarningClearAppData =您是否要清除用户设置和应用缓存数据？
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
        'SOFTWARE\{#APP_PATH}',
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

    userPath := ExpandConstant('{localappdata}\{#sIntCompanyName}');
    if regValue = 'soft' then begin
      RegDeleteKeyIncludingSubkeys(GetHKLM(), 'Software\{#sIntCompanyName}');
      RegDeleteKeyIncludingSubkeys(HKEY_CURRENT_USER, 'Software\{#sIntCompanyName}');

      // remove all app and user cashed data except of folders 'recover' and 'sdkjs-plugins'
      userPath := userPath + '\{#sIntProductName}';
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
      RegDeleteKeyIncludingSubkeys(GetHKLM(), 'Software\{#sIntCompanyName}');
      RegDeleteKeyIncludingSubkeys(HKEY_CURRENT_USER, 'Software\{#sIntCompanyName}');
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
    Result := '{#APP_MUTEX_NAME}'
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
Source: data\vcredist\vcredist_2015_{#sWinArch}.exe; DestDir: {app}; Flags: deleteafterinstall; \
  AfterInstall: installVCRedist(ExpandConstant('{app}\vcredist_2015_{#sWinArch}.exe'), ExpandConstant('{cm:InstallAdditionalComponents}')); \
  Check: not checkVCRedist2015;

Source: {#sBrandingFolder}\win-linux\package\windows\data\VisualElementsManifest.xml;        DestDir: {app}; DestName: {#VISEFFECTS_MANIFEST_NAME}; MinVersion: 6.3;
Source: {#sBrandingFolder}\win-linux\package\windows\data\visual_elements_icon_150x150.png;  DestDir: {app}\browser;   MinVersion: 6.3;
Source: {#sBrandingFolder}\win-linux\package\windows\data\visual_elements_icon_71x71.png;    DestDir: {app}\browser;   MinVersion: 6.3;

#ifndef SCRIPT_CUSTOM_FILES

Source: ..\..\deploy\{#sPlatform}\3dparty\Qt\*;                 DestDir: {app}; Flags: ignoreversion recursesubdirs;

Source: data\projicons.exe;                                     DestDir: {app}; DestName: {#iconsExe};
Source: ..\..\build\Release\{#NAME_EXE_IN};                     DestDir: {app}; DestName: {#NAME_EXE_OUT};

Source: {#sBrandingFolder}\win-linux\extras\projicons\res\icons\desktopeditors.ico;  DestDir: {app}; DestName: app.ico;
Source: ..\..\..\common\loginpage\deploy\index.html;            DestDir: {app}; DestName: index.html;
Source: {#sBrandingFolder}\common\package\license\{#LIC_FILE}.htm;         DestDir: {app}; DestName: LICENSE.htm;
Source: {#sBrandingFolder}\common\package\license\3dparty\3DPARTYLICENSE;  DestDir: {app};
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

Source: ..\..\deploy\{#sPlatform}\libs\*; DestDir: {app}\converter; Excludes: *.lib,*.exp,*.exe,ascdocumentscore.dll,ooxmlsignature.dll,hunspell.dll; Flags: ignoreversion;

Source: ..\..\deploy\{#sPlatform}\libs\HtmlFileInternal.exe;      DestDir: {app}; Flags: ignoreversion;
Source: ..\..\deploy\{#sPlatform}\libs\hunspell.dll;              DestDir: {app}; Flags: ignoreversion;
Source: ..\..\deploy\{#sPlatform}\libs\ooxmlsignature.dll;        DestDir: {app}; Flags: ignoreversion;
Source: ..\..\deploy\{#sPlatform}\libs\x2t.exe;                   DestDir: {app}\converter; Flags: ignoreversion;
#ifdef _WIN_XP
Source: ..\..\..\..\core\build\lib\{#sPlatform}\xp\ascdocumentscore.dll; DestDir: {app}; Flags: ignoreversion;
#else
Source: ..\..\deploy\{#sPlatform}\libs\ascdocumentscore.dll;      DestDir: {app}; Flags: ignoreversion;
#endif

Source: ..\..\..\..\core\Common\3dParty\v8\v8\out.gn\{#sPlatform}\release\icudtl.dat; DestDir: {app}\converter; Flags: ignoreversion;
Source: ..\..\..\..\core\Common\3dParty\icu\{#sPlatform}\build\icu*58.dll;  DestDir: {app}\converter; Flags: ignoreversion;
Source: ..\..\..\..\core\Common\3dParty\cef\{#sPlatform}\build\*;           DestDir: {app}; Excludes: *.lib; Flags: ignoreversion recursesubdirs;

#ifdef _ENCRYPT_MODULE
Source: ..\..\..\..\desktop-sdk\ChromiumBasedEditors\plugins\encrypt\ui\common\*; DestDir: {app}\editors\sdkjs-plugins; Flags: recursesubdirs;
Source: ..\..\..\..\desktop-sdk\ChromiumBasedEditors\plugins\encrypt\ui\engine\blockchain\*; DestDir: {app}\editors\sdkjs-plugins; Flags: recursesubdirs;
#endif

#ifdef _UPDMODULE
Source: ..\..\3dparty\WinSparkle\{#sPlatform}\WinSparkle.dll;           DestDir: {app}\; Flags: ignoreversion;
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

  #ifdef _WIN_XP
Source: ..\..\..\..\core\build\{#PATH_PREFIX}\bin\{#sPlatform}\x2t.exe; DestDir: {app}\converter; Flags: ignoreversion;
Source: ..\..\..\..\core\build\bin\{#sPlatform}\icudt.dll;     DestDir: {app}\converter; Flags: ignoreversion;
Source: ..\..\..\..\core\build\bin\icu\{#sPlatform}\*;         DestDir: {app}\converter; Flags: ignoreversion; Excludes: *.lib;

Source: ..\..\..\..\core\build\cef\winxp_{#_ARCH}\*;           DestDir: {app}\; Excludes: *.lib; Flags: ignoreversion recursesubdirs;
Source: data\libs\qt\win{#_ARCH}\*;                            DestDir: {app}\; Flags: ignoreversion recursesubdirs;
;Source: ..\..\3dparty\WinSparkle\{#sPlatform}\WinSparkle.dll;  DestDir: {app}\; Flags: ignoreversion;
  #endif

#else

Source: {#DEPLOY_PATH}\*;                               DestDir: {app}; Flags: recursesubdirs;
Source: {#DEPLOY_PATH}\*.exe;                           DestDir: {app}; Flags: signonce;
Source: {#DEPLOY_PATH}\ascdocumentscore.dll;            DestDir: {app}; Flags: signonce;
Source: {#DEPLOY_PATH}\hunspell.dll;                    DestDir: {app}; Flags: signonce;
Source: {#DEPLOY_PATH}\ooxmlsignature.dll;              DestDir: {app}; Flags: signonce;
Source: {#DEPLOY_PATH}\WinSparkle.dll;                  DestDir: {app}; Flags: signonce;
Source: {#DEPLOY_PATH}\converter\DjVuFile.dll;          DestDir: {app}\converter; Flags: signonce;
Source: {#DEPLOY_PATH}\converter\doctrenderer.dll;      DestDir: {app}\converter; Flags: signonce;
Source: {#DEPLOY_PATH}\converter\graphics.dll;          DestDir: {app}\converter; Flags: signonce;
Source: {#DEPLOY_PATH}\converter\HtmlFile.dll;          DestDir: {app}\converter; Flags: signonce;
Source: {#DEPLOY_PATH}\converter\HtmlRenderer.dll;      DestDir: {app}\converter; Flags: signonce;
Source: {#DEPLOY_PATH}\converter\kernel.dll;            DestDir: {app}\converter; Flags: signonce;
Source: {#DEPLOY_PATH}\converter\PdfReader.dll;         DestDir: {app}\converter; Flags: signonce;
Source: {#DEPLOY_PATH}\converter\PdfWriter.dll;         DestDir: {app}\converter; Flags: signonce;
Source: {#DEPLOY_PATH}\converter\UnicodeConverter.dll;  DestDir: {app}\converter; Flags: signonce;
Source: {#DEPLOY_PATH}\converter\x2t.exe;               DestDir: {app}\converter; Flags: signonce;
Source: {#DEPLOY_PATH}\converter\XpsFile.dll;           DestDir: {app}\converter; Flags: signonce;

[InstallDelete]
Type: filesandordirs; Name: {app}\editors\sdkjs-plugins

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
