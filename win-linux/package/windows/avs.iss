
[Setup]
AppPublisher            = Online Media Technologies Ltd.
AppPublisherURL         = http://www.avs4you.com
AppSupportURL           = http://www.avs4you.com/support.aspx
AppCopyright            = Online Media Technologies Ltd., 2016
AppUpdatesURL           = http://www.avs4you.com/AVS-Document-Editor.aspx

DefaultGroupName        =AVS4YOU
WizardImageFile         = data\avs\dialogpicture.bmp
WizardSmallImageFile    = data\avs\dialogicon.bmp

AppId                   ={#sAppName}
ShowLanguageDialog      =no
LanguageDetectionMethod =none

;MinVersion              = 0, 5.0.2195
#ifdef _AVS_LIGHT_VERSION
  OutputBaseFilename    = {#sShortAppName}_light
#elif defined(_WIN64)
  OutputBaseFilename    = {#sShortAppName}_x64
#else
  OutputBaseFilename    = {#sShortAppName}_x86
#endif

AppMutex                = AVSMEDIA

[Languages]
Name: en; MessagesFile: compiler:Default.isl;           LicenseFile: ..\..\..\common\package\license\eula_avs.rtf;
Name: ru; MessagesFile: compiler:Languages\Russian.isl; LicenseFile: ..\..\..\common\package\license\eula_avs.rtf;

[Messages]
;================================== New strings for overloading of Inno Setup .isl files ==============

en.BeveledLabel = English
ru.BeveledLabel = Russian

;======================================================================================================

en.SetupAppRunningError=Setup has detected that one or more of AVS programs are currently running.%n%nPlease close all instances of AVS running programs now, then click OK to continue, or Cancel to exit.
en.UninstallAppRunningError=Uninstall has detected that one or more of AVS programs are currently running.%n%nPlease close all instances of AVS running programs now, then click OK to continue, or Cancel to exit.

ru.SetupAppRunningError=Обнаружен запущенный экземпляр программы AVS.%n%nПожалуйста, закройте все экземпляры приложения, затем нажмите «OK», чтобы продолжить, или «Отмена», чтобы выйти.
ru.UninstallAppRunningError=Деинсталлятор обнаружил запущенный экземпляр программы AVS.%n%nПожалуйста, закройте все экземпляры приложения, затем нажмите «OK», чтобы продолжить, или «Отмена», чтобы выйти.

;======================================================================================================

#if !defined(_AVS_LIGHT_VERSION)
en.FinishedLabelNoIcons=Setup has finished installing [name] on your computer.%n%nYou can read our software uninstallation guidelines clicking the 'User Guides' link.
en.FinishedLabel=Setup has finished installing [name] on your computer. The application may be launched by selecting the installed icons.%n%nYou can read our software uninstallation guidelines clicking the 'User Guides' link.

ru.FinishedLabelNoIcons=Программа [name] установлена на ваш компьютер.%n%nРуководства по удалению программ вы можете найти, нажав на ссылку Руководства пользователя.
ru.FinishedLabel=Программа [name] установлена на ваш компьютер. Приложение можно запустить с помощью соответствующего значка.%n%nРуководства по удалению программ вы можете найти, нажав на ссылку Руководства пользователя.
#endif

[CustomMessages]
;================================== Guides ============================================================
en.Guides =User Guides
ru.Guides =Руководства пользователя


[Dirs]
Name: "{commonappdata}\AVS4YOU\"; Permissions: everyone-modify


[Files]
#ifndef _AVS_LIGHT_VERSION
Source: data\avs\serviceprograms\Registration.exe;          DestDir: "{app}"; Flags: deleteafterinstall;
#endif

Source: ..\..\res\icons\avs\desktopeditors.ico;           DestDir: {app}\; DestName: app.ico; 
Source: ..\..\..\common\loginpage\deploy\index.avs.html;  DestDir: {commonappdata}\{#APP_PATH}\webdata\local; DestName: index.html;
Source: ..\..\..\common\package\license\eula_avs.htm;     DestDir: {app};  DestName: LICENSE.htm;
Source: data\projicons_omt.exe;                           DestDir: {app}\; DestName: {#iconsExe};

[Icons]
Name: {group}\Documents\{#sAppName}; Filename: {app}\{#NAME_EXE_OUT}; WorkingDir: {app}; IconFilename: {app}\app.ico;

[Run]
#ifndef _AVS_LIGHT_VERSION
Filename: {app}\Registration.exe; Parameters: "/VERYSILENT /SUPPRESSMSGBOXES /GROUP=""{groupname}"" /LANG={code:LanguageName} {code:SetupParam} /noRepairing";
#endif


[Registry]
Root: HKLM32; Subkey: SOFTWARE\AVS4YOU\Uninstall; ValueType: string; ValueName: {#sAppName}; ValueData: {uninstallexe}; Flags: uninsdeletevalue;

Root: HKLM32; Subkey: SOFTWARE\AVS4YOU\Navigator; ValueType: string; ValueName: {#sAppName}; ValueData: "http://www.avs4you.com/downloads/{#sShortAppName}_x86.exe";

Root: HKLM; Subkey: "{#APP_REG_PATH}";            ValueType: string; ValueName: "IDownload"; ValueData: "http://www.avs4you.com/downloads/{#sShortAppName}_x86.exe"; Check: not IsWin64 or not Is64BitInstallMode;
Root: HKLM; Subkey: "{#APP_REG_PATH}";            ValueType: string; ValueName: "IDownload"; ValueData: "http://www.avs4you.com/downloads/{#sShortAppName}_x64.exe"; Check: IsWin64 and Is64BitInstallMode;

Root: HKLM; Subkey: "{#APP_REG_PATH}";            ValueType: string; ValueName: "PathToExe"; ValueData: "{app}\{#NAME_EXE_OUT}"; Flags: uninsdeletevalue;


[UninstallDelete]
#ifdef _AVS_LIGHT_VERSION
Type: dirifempty;     Name: "{app}";
#endif


[Code]
#include "avs_utils.iss"

function GetHKLM: Integer; forward;

var
  AlreadyInstalled:  Boolean;
  FinishedUninstall: Boolean;
#ifndef _AVS_LIGHT_VERSION
  GuidesLinkLabel:   TLabel;

procedure GuidesLinkClick(Sender: TObject);
var
  ErrorCode: Integer;
begin
  ShellExec('', InternationalizeURL3('http://www.avs4you.com/guides/index.aspx'), '', '',
            SW_SHOW, ewNoWait, ErrorCode);
end;

procedure InitializeGuidesLink;
begin
  GuidesLinkLabel := TLabel.Create(WizardForm);
  GuidesLinkLabel.Parent := WizardForm;
  GuidesLinkLabel.Left := 12;
  GuidesLinkLabel.Top := WizardForm.ClientHeight - 
    GuidesLinkLabel.ClientHeight - 16;
  GuidesLinkLabel.Cursor := crHand;
  GuidesLinkLabel.Font.Color := clBlue;
  GuidesLinkLabel.Font.Style := [fsUnderline];
  GuidesLinkLabel.Caption := ExpandConstant('{cm:Guides}');
  GuidesLinkLabel.OnClick := @GuidesLinkClick;
end;

procedure CurPageChanged(CurPageID: Integer);
begin
    GuidesLinkLabel.Visible := CurPageID = wpFinished;
end;
#endif

function SetupParam(Param: String): String;
begin
  if (AlreadyInstalled) then
    Result := '/NOINC'
  else
    Result := '';
end;

function LanguageName(Param: String): String;
begin
  Result := ActiveLanguage;
  if ( Result = 'es' ) then    //service programs expect spanish language as 'sp'
    Result := 'sp';
end;

procedure DoInstall();
begin
  AlreadyInstalled := RegValueExists(GetHKLM(), 'SOFTWARE\AVS4YOU\DocumentEditor', 'AppPath');
end;

procedure DoInstallDone();
var
  DVDOpenMode : Integer;
  RegnowURL, Publisher, BuyURL, PostInstallURL, FeedbackURL : String;
  BuyURLAbout : String;
  ProlongURL, TestString : String;
  HomePageURL, SupportURL, SupportMail, RegistrationRegKey : String;
  ErrorCode : Integer;
  bExist, bEmpty : Boolean;
begin
  DVDOpenMode     := 1; // not used
  Publisher       := '';
  PostInstallURL  := '';
  FeedbackURL     := '';

  RegnowURL := GetRegnowBuyURL('14438-2');

  ReadInstallInfo(DVDOpenMode, Publisher, BuyURL,
                  PostInstallURL, FeedbackURL, ProlongURL,
                  HomePageURL, SupportURL, SupportMail);

  if ( Length(RegnowURL) > 0 ) then
    BuyURL := RegnowURL;

  // Publisher
  bExist := RegValueExists(HKLM32, 'SOFTWARE\AVS4YOU', 'Publisher');
  if (bExist) then
    if (RegQueryStringValue(HKLM32, 'SOFTWARE\AVS4YOU', 'Publisher', TestString)) then
      if (Length(TestString) = 0) then
        bEmpty := True
      else
      begin
        bEmpty := False;
        Publisher := TestString;
      end
    else
      bEmpty := True
  else
    bEmpty := True;
  if (not bExist or (bExist and bEmpty)) then
    RegWriteStringValue(HKLM32, 'SOFTWARE\AVS4YOU', 'Publisher', Publisher);

  // PostInstallURL
  if ( Length(PostInstallURL) = 0 ) then
    PostInstallURL := ParametrizeURL( InternationalizeURL2('http://www.avs4you.com/Register.aspx'), '{#AppInternalId}', 'Install', 'Register', Publisher );

  // BuyURL
  if ( Length(BuyURL) = 0 ) then
    BuyURL := GoogleParametrizeURL( InternationalizeURL2('http://www.avs4you.com/Register.aspx'), '{#AppInternalId}', 'Register', 'Register', Publisher )
  else
    BuyURLAbout := BuyURL;

  // ProlongURL
  if ( Length(ProlongURL) = 0 ) then
    ProlongURL := ParametrizeURL( InternationalizeURL('http://reg.avs4you.com/prolongation/prolongation.aspx'), '{#AppInternalId}', 'App', 'Prolong', Publisher );

  // HomePageURL
  if ( Length(HomePageURL) = 0 ) then
    HomePageURL := 'http://www.avs4you.com/index.aspx';

  // SupportURL
  if ( Length(SupportURL) = 0 ) then
    SupportURL := 'http://www.avs4you.com/support.aspx';

  // SupportMail
  if ( Length(SupportMail) = 0 ) then
    SupportMail := 'support@avs4you.com';

  if (Length(BuyURLAbout) = 0) then
    BuyURLAbout := 'https://store.avs4you.com/order/checkout.php?PRODS=604132&QTY=1&CURRENCY=USD&DCURRENCY=USD&LANGUAGES=en,de,fr,es,it,ja,nl,da,ko,pl,ru,pt&CART=1&CARD=1&CLEAN_CART=ALL&SHORT_FORM=1&AUTO_PREFILL=1&SRC=InProductAbout';

  // IBuyAbout
  bExist := RegValueExists(HKLM, '{#APP_REG_PATH}', 'IBuyAbout');
  if (bExist) then
    if (RegQueryStringValue(HKLM, '{#APP_REG_PATH}', 'IBuyAbout', TestString)) then
      if (Length(TestString) = 0) then
        bEmpty := True
      else
        bEmpty := False
    else
      bEmpty := True
  else
    bEmpty := True;
  if (not bExist or (bExist and bEmpty)) or (not AlreadyInstalled) then
    RegWriteStringValue( HKLM, '{#APP_REG_PATH}', 'IBuyAbout',   BuyURLAbout );

  // IBuy
  bExist := RegValueExists(HKLM, '{#APP_REG_PATH}', 'IBuy');
  if (bExist) then
    if (RegQueryStringValue(HKLM, '{#APP_REG_PATH}', 'IBuy', TestString)) then
      if (Length(TestString) = 0) then
        bEmpty := True
      else
        bEmpty := False
    else
      bEmpty := True
  else
    bEmpty := True;
  if (not bExist or (bExist and bEmpty)) or (not AlreadyInstalled) then
    RegWriteStringValue( HKLM, '{#APP_REG_PATH}', 'IBuy',        BuyURL );

  // IBuy for service program Registration
  RegistrationRegKey := 'SOFTWARE\AVS4YOU\Registration';
  bExist := RegValueExists(HKLM32, RegistrationRegKey, 'IBuy');
  if (bExist) then
    if (RegQueryStringValue(HKLM32, RegistrationRegKey, 'IBuy', TestString)) then
      if (Length(TestString) = 0) then
        bEmpty := True
      else
        bEmpty := False
    else
      bEmpty := True
  else
    bEmpty := True;
  if (not bExist or (bExist and bEmpty)) or
     (not RegValueExists(HKLM32, RegistrationRegKey, 'Uninstall')) then
    RegWriteStringValue( HKLM32, RegistrationRegKey, 'IBuy', BuyURL )
  else if (bExist and not bEmpty) then
  begin
    if ((Pos('http://www.avs4you.com', TestString) > 0) and
        (Pos('http://www.avs4you.com', BuyURL) = 0)) then
      RegWriteStringValue( HKLM32, RegistrationRegKey, 'IBuy', BuyURL )
  end;

  // IProlong
  bExist := RegValueExists(HKLM, '{#APP_REG_PATH}', 'IProlong');
  if (bExist) then
    if (RegQueryStringValue(HKLM, '{#APP_REG_PATH}', 'IProlong', TestString)) then
      if (Length(TestString) = 0) then
        bEmpty := True
      else
        bEmpty := False
    else
      bEmpty := True
  else
    bEmpty := True;
  if (not bExist or (bExist and bEmpty)) or (not AlreadyInstalled) then
    RegWriteStringValue( HKLM, '{#APP_REG_PATH}', 'IProlong',    ProlongURL );

  // IHome
  bExist := RegValueExists(HKLM, '{#APP_REG_PATH}', 'IHome');
  if (bExist) then
    if (RegQueryStringValue(HKLM, '{#APP_REG_PATH}', 'IHome', TestString)) then
      if (Length(TestString) = 0) then
        bEmpty := True
      else
        bEmpty := False
    else
      bEmpty := True
  else
    bEmpty := True;
  if (not bExist or (bExist and bEmpty)) or (not AlreadyInstalled) then
    RegWriteStringValue( HKLM, '{#APP_REG_PATH}', 'IHome',       HomePageURL );

  // ISupport
  bExist := RegValueExists(HKLM, '{#APP_REG_PATH}', 'ISupport');
  if (bExist) then
    if (RegQueryStringValue(HKLM, '{#APP_REG_PATH}', 'ISupport', TestString)) then
      if (Length(TestString) = 0) then
        bEmpty := True
      else
        bEmpty := False
    else
      bEmpty := True
  else
    bEmpty := True;
  if (not bExist or (bExist and bEmpty)) or (not AlreadyInstalled) then
    RegWriteStringValue( HKLM, '{#APP_REG_PATH}', 'ISupport',    SupportURL );

  // ESupport
  bExist := RegValueExists(HKLM32, 'SOFTWARE\AVS4YOU', 'ESupport');
  if (bExist) then
    if (RegQueryStringValue(HKLM32, 'SOFTWARE\AVS4YOU', 'ESupport', TestString)) then
      if (Length(TestString) = 0) then
        bEmpty := True
      else
        bEmpty := False
    else
      bEmpty := True
  else
    bEmpty := True;
  if (not bExist or (bExist and bEmpty)) or (not AlreadyInstalled) then
    RegWriteStringValue( HKLM32, 'SOFTWARE\AVS4YOU', 'ESupport',    SupportMail );

  if ( Length(FeedbackURL) > 0 ) then
    RegWriteStringValue(HKLM32, 'Software\AVS4YOU\Common\', 'FeedbackURL', FeedbackURL);
end;

procedure DeinitializeUninstall();
var
  Path      : String;
  ErrorCode : Integer;
  Counter   : Cardinal;
begin
  if (FinishedUninstall and NeedUninstall(HKLM32, 'SOFTWARE\AVS4YOU\Registration')) then
  begin
    Path := UninstallerPath(HKLM32, 'SOFTWARE\AVS4YOU\Registration');
    if (Length(Path) > 1) then
      Exec(Path, '/VERYSILENT /SUPPRESSMSGBOXES', '', SW_SHOW, ewWaitUntilTerminated, ErrorCode);
  end;
end;

