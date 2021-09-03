﻿; -- Installer Common --

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

#include sBrandingFolder + "\win-linux\package\windows\defines.iss"

#ifndef sAppVersion
  #define sAppVersion             GetFileVersion(AddBackslash(DEPLOY_PATH) + NAME_EXE_OUT)
#endif
#define sAppVerShort                Copy(sAppVersion, 0, 3)

#ifndef sOutputFileName
  #define sOutputFileName           str(sPackageName + "_" + sAppVersion + "_" + sWinArchFull)
#endif

#define sBrandingFile               sBrandingFolder + "\win-linux\package\windows\branding.iss"
#if FileExists(sBrandingFile)
  #include sBrandingFile
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
UninstallDisplayName      = {#sAppName} {#sAppVerShort} ({#sWinArch})
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
Name: bg; MessagesFile: compiler:Languages\Bulgarian.isl;   LicenseFile: {#sBrandingFolder}\common\package\license\{#LIC_FILE}.rtf;
Name: ca; MessagesFile: compiler:Languages\Catalan.isl;   LicenseFile: {#sBrandingFolder}\common\package\license\{#LIC_FILE}.rtf;
Name: cs; MessagesFile: compiler:Languages\Czech.isl;   LicenseFile: {#sBrandingFolder}\common\package\license\{#LIC_FILE}.rtf;
Name: el; MessagesFile: compiler:Languages\Greek.isl;   LicenseFile: {#sBrandingFolder}\common\package\license\{#LIC_FILE}.rtf;
Name: et; MessagesFile: compiler:Languages\Estonian.isl;   LicenseFile: {#sBrandingFolder}\common\package\license\{#LIC_FILE}.rtf;
Name: fi; MessagesFile: compiler:Languages\Finnish.isl;   LicenseFile: {#sBrandingFolder}\common\package\license\{#LIC_FILE}.rtf;
Name: lt; MessagesFile: compiler:Languages\Lithuanian.isl;   LicenseFile: {#sBrandingFolder}\common\package\license\{#LIC_FILE}.rtf;
Name: lo; MessagesFile: compiler:Default.isl;           LicenseFile: {#sBrandingFolder}\common\package\license\{#LIC_FILE}.rtf;
Name: nl; MessagesFile: compiler:Languages\Dutch.isl;   LicenseFile: {#sBrandingFolder}\common\package\license\{#LIC_FILE}.rtf;
Name: de; MessagesFile: compiler:Languages\German.isl;     LicenseFile: {#sBrandingFolder}\common\package\license\{#LIC_FILE}.rtf;
Name: fr; MessagesFile: compiler:Languages\French.isl;     LicenseFile: {#sBrandingFolder}\common\package\license\{#LIC_FILE}.rtf;
Name: es; MessagesFile: compiler:Languages\Spanish.isl;    LicenseFile: {#sBrandingFolder}\common\package\license\{#LIC_FILE}.rtf;
Name: pt_BR; MessagesFile: compiler:Languages\BrazilianPortuguese.isl; LicenseFile: {#sBrandingFolder}\common\package\license\{#LIC_FILE}.rtf;
Name: pt_PT; MessagesFile: compiler:Languages\Portuguese.isl; LicenseFile: {#sBrandingFolder}\common\package\license\{#LIC_FILE}.rtf;
Name: in; MessagesFile: compiler:Languages\Indonesian.isl; LicenseFile: {#sBrandingFolder}\common\package\license\{#LIC_FILE}.rtf;
Name: it_IT; MessagesFile: compiler:Languages\Italian.isl; LicenseFile: {#sBrandingFolder}\common\package\license\{#LIC_FILE}.rtf;
Name: pl; MessagesFile: compiler:Languages\Polish.isl;     LicenseFile: {#sBrandingFolder}\common\package\license\{#LIC_FILE}.rtf;
Name: ro; MessagesFile: compiler:Languages\Romanian.isl;     LicenseFile: {#sBrandingFolder}\common\package\license\{#LIC_FILE}.rtf;
Name: sk; MessagesFile: compiler:Languages\Slovak.isl;     LicenseFile: {#sBrandingFolder}\common\package\license\{#LIC_FILE}.rtf;
Name: sl; MessagesFile: compiler:Languages\Slovenian.isl;     LicenseFile: {#sBrandingFolder}\common\package\license\{#LIC_FILE}.rtf;
Name: sv; MessagesFile: compiler:Languages\Swedish.isl;     LicenseFile: {#sBrandingFolder}\common\package\license\{#LIC_FILE}.rtf;
Name: tr; MessagesFile: compiler:Languages\Turkish.isl;     LicenseFile: {#sBrandingFolder}\common\package\license\{#LIC_FILE}.rtf;
Name: vi; MessagesFile: compiler:Default.isl;               LicenseFile: {#sBrandingFolder}\common\package\license\{#LIC_FILE}.rtf;
Name: zh_CN; MessagesFile: compiler:Languages\ChineseTraditional.isl;  LicenseFile: {#sBrandingFolder}\common\package\license\{#LIC_FILE}.rtf;


[CustomMessages]
;======================================================================================================
en.Launch =Launch %1
bg.Launch =Пускане %1
ca.Launch =Llança %1
cs.Launch =Spuštění %1
el.Launch =Εκκίνηση %
et.Launch =Käivita %1
fi.Launch =Käynnistys %1
lt.Launch =Paleisti %1
lo.Launch =ເປິດຕົວ %1
nl.Launch =Start %1
ru.Launch =Запустить %1
de.Launch =%1 starten
fr.Launch =Lancer %1
es.Launch =Ejecutar %1
in.Launch =Luncurkan %1
it_IT.Launch =Eseguire %1
pt_BR.Launch =Lance o %1
pt_PT.Launch =Iniciar %1
pl.Launch =Uruchom %1
ro.Launch =Lansare %1
sk.Launch =Zahájiť %1
sl.Launch =Zaženi %1
sv.Launch =Starta %1
tr.Launch =%1 Başlatma
vi.Launch =Khởi động %1
zh_CN.Launch =启动%1
;======================================================================================================
en.CreateDesktopIcon =Create %1 &desktop icon
bg.CreateDesktopIcon =Създаване на %1 &икона на работния плот
ca.CreateDesktopIcon =Crea una icona d'escriptori per a %1
cs.CreateDesktopIcon =Vytvořte %1 &ikonu pracovní plochy
el.CreateDesktopIcon =Δημιουργία %1 &εικονίδιο επιφάνειας εργασίας
et.CreateDesktopIcon =Loo %1 & töölaua ikoon
fi.CreateDesktopIcon =Luo %1 &työpöydän kuvake
lt.CreateDesktopIcon =Sukurti %1 &darbalaukio piktogramą
lo.CreateDesktopIcon =ສ້າງຮູບສັນຍາລັກ %1 &ໄອຄອນໜ້າຈໍ
nl.CreateDesktopIcon =Maak %1 &desktop icoon aan
ru.CreateDesktopIcon =Создать иконку %1 на &рабочем столе
de.CreateDesktopIcon =%1 &Desktop-Icon erstellen
fr.CreateDesktopIcon =Créer l'icône du bureau pour %1
es.CreateDesktopIcon =Crear %1 &icono en el escritorio
in.CreateDesktopIcon =Buat &ikon desktop %1
it_IT.CreateDesktopIcon =Creare un collegamento %1 sul &desktop
pt_BR.CreateDesktopIcon =Criar ícone de &desktop do %1
pt_PT.CreateDesktopIcon =Criar um ícone de %1 &desktop
pl.CreateDesktopIcon =Stwórz %1 oraz ikonę pulpitu
ro.CreateDesktopIcon =Adãugarea %1 &pictogramelor de pe desktop
sk.CreateDesktopIcon =Vytvoriť %1 ikonu na &pracovnej ploche
sl.CreateDesktopIcon =Ustvari %1 &desktop ikono
sv.CreateDesktopIcon =Skapa %1 &ikon på skrivbordet
tr.CreateDesktopIcon =%1 &masaüstü simgesi oluştur
vi.CreateDesktopIcon =Tạo %1 &biểu tượng máy tính
zh_CN.CreateDesktopIcon =创建%1和桌面图标
;======================================================================================================
en.InstallAdditionalComponents =Installing additional system components. Please wait...
ca.InstallAdditionalComponents =Instal·lació de components de sistema addicionals. Si us plau, espereu...
bg.InstallAdditionalComponents =Инсталиране на допълнителни системни компоненти. Моля, изчакайте...
cs.InstallAdditionalComponents =Instalace dalších systémových komponent. Prosím, čekejte...
el.InstallAdditionalComponents =Εγκατάσταση πρόσθετων στοιχείων συστήματος. Παρακαλούμε περιμένετε...
et.InstallAdditionalComponents =Laeb alla süsteemi lisa komponente. Palun oota...
fi.InstallAdditionalComponents =Asennetaan valinnaisia systemin osia. Ole hyvä ja odota...
lt.InstallAdditionalComponents =Diegiami papildomi sistemos komponentai. Prašome palaukti...
lo.InstallAdditionalComponents =ຕິດຕັ້ງສ່ວນປະກອບຂອງລະບົບເພີ່ມເຕີມ. ກະລຸນາລໍຖ້າ...
nl.InstallAdditionalComponents =Installeren van extra systeemcomponenten. Moment a.u.b...
ru.InstallAdditionalComponents =Установка дополнительных системных компонентов. Пожалуйста, подождите...
de.InstallAdditionalComponents =Installation zusätzlicher Systemkomponenten. Bitte warten...
fr.InstallAdditionalComponents =L'installation des composants supplémentaires du système. Attendez...
es.InstallAdditionalComponents =Instalando componentes adicionales del sistema. Por favor espere...
in.InstallAdditionalComponents =Menginstal komponen tambahan sistem. Mohon tunggu...
it_IT.InstallAdditionalComponents =Installazione dei componenti addizionali del sistema. Per favore, attendi...
pt_BR.InstallAdditionalComponents =Instalando componentes do sistema adicional. Aguarde...
pt_PT.InstallAdditionalComponents =A instalar mais componentes do sistema. Por favor aguarde...
pl.InstallAdditionalComponents =Instalacja dodatkowych elementów systemu. Proszę czekać...
ro.InstallAdditionalComponents =Instalarea componentelor suplimentare sistem. Vã rugãm așteptați...
sk.InstallAdditionalComponents =Inštalujú sa ďalšie komponenty systému. Čakajte prosím...
sl.InstallAdditionalComponents =Nameščanje dodatnih sistemskih komponent. Prosimo, počakajte...
sv.InstallAdditionalComponents =Installerar ytterligare systemkomponenter. Var vänlig vänta...
tr.InstallAdditionalComponents =Ek sistem bileşenleri kuruluyor. Lütfen bekleyin...
vi.InstallAdditionalComponents =Đang cài đặt các  phần hệ thống bổ sung. Xin vui lòng chờ...
zh_CN.InstallAdditionalComponents =安装其他系统组件。请稍候...
;======================================================================================================
en.AdditionalTasks =Tasks:
bg.AdditionalTasks =Задачи:
ca.AdditionalTasks =Tasques:
cs.AdditionalTasks =Úkoly:
el.AdditionalTasks =Εργασίες:
et.AdditionalTasks =Ülesanded:
fi.AdditionalTasks =Tasks:
lt.AdditionalTasks =Užduotys:
lo.AdditionalTasks =ໜ້າວຽກ:
nl.AdditionalTasks =Taken:
ru.AdditionalTasks =Задачи:
de.AdditionalTasks =Aufgaben:
fr.AdditionalTasks =Tâches:
in.AdditionalTasks =Tugas:
es.AdditionalTasks =Tareas:
it_IT.AdditionalTasks =Attività:
pt_BR.AdditionalTasks =Tarefas:
pt_PT.AdditionalTasks =Tarefas:
pl.AdditionalTasks =Zadania:
ro.AdditionalTasks =Sarcini:
sk.AdditionalTasks =Úlohy:
sl.AdditionalTasks =Naloge:
sv.AdditionalTasks =Uppgifter:
tr.AdditionalTasks =Görevler:
vi.AdditionalTasks =Nhiệm vụ:
zh_CN.AdditionalTasks =任务：
;======================================================================================================
en.Uninstall =Uninstall
bg.AdditionalTasks =Деинсталиране
ca.AdditionalTasks =Desinstal·lar
cs.Uninstall =Odinstalovat
el.Uninstall =Απεγκατάσταση
et.Uninstall =Lae maha
fi.Uninstall =Poista asennus
lt.Uninstall =Išinstaliuoti
lo.Uninstall =ຖອນການຕິດຕັ້ງ
nl.Uninstall =Verwijderen
ru.Uninstall =Удаление
de.Uninstall =Deinstallieren
fr.Uninstall =Desinstaller
es.Uninstall =Desinstalar
in.Uninstall =Uninstal
it_IT.Uninstall =Disinstalla
pt_BR.Uninstall =Desinstalar
pt_PT.Uninstall =Desinstalar
pl.Uninstall =Odinstaluj
ro.Uninstall =Dezinstalarea
sk.Uninstall =Odinštalovať
sl.Uninstall =Odstrani
sv.Uninstall =Avinstallera
tr.Uninstall =Kaldır
vi.Uninstall =Gỡ cài đặt
zh_CN.Uninstall =卸载
;======================================================================================================
en.WarningWrongArchitecture =You are trying to install the %1-bit application version over the %2-bit version installed. Please uninstall the previous version first or download the correct version for installation.
bg.WarningWrongArchitecture =Опитвате се да инсталирате %1-битовата версия на приложението над инсталираната %2-битова версия. Моля, деинсталирайте първо предишната версия или изтеглете правилната версия за инсталиране.
ca.WarningWrongArchitecture =Estau provant a instal·lar la versió de l'aplicació %1-bit amb la versió %2-bit instal·lada. Si us plau, desintaleu primer la versió anterior o descarregeu la versió correcta per a la instal·lació.
cs.WarningWrongArchitecture =Pokoušíte se nainstalovat %1-bit verzi aplikace na nainstalovanou %2-bitovou verzi. Nejprve odinstalujte předchozí verzi nebo stáhněte správnou verzi pro instalaci.
el.WarningWrongArchitecture =Προσπαθείτε να εγκαταστήσετε την έκδοση εφαρμογής %1-bit έναντι της εγκατεστημένης έκδοσης %2-bit. Παρακαλούμε απεγκαταστήστε πρώτα την προηγούμενη έκδοση ή κατεβάστε τη σωστή έκδοση για εγκατάσταση.
et.WarningWrongArchitecture =Sa proovid alla laadida %1-bit rakenduse versiooni, mitte %2-bit versiooni, mis on juba alla laetud. Palun lae maha eelnev versioon enne või lae alla õige versioon.
fi.WarningWrongArchitecture =Yrität asentaa %1-bittistä versiota %2-bittisen version päälle. Ole hyvä ja poista aiemman asennettu versio ensin tai lataa oikea versio asennettavaksi.
lt.WarningWrongArchitecture =Jūs bandote įdiegti %1-bitų programos versiją vietoj %2-bitų instaliuotos versijos. Pirmiausia pašalinkite ankstesnę versiją arba atsisiųskitė tinkamą versiją diegimui.
lo.WarningWrongArchitecture =ທ່ານ ກຳ ລັງພະຍາຍາມຕິດຕັ້ງເວີຊັນ application 1-bit ໃນເວີຊັນ% 2-bit ທີ່ຕິດຕັ້ງໄວ້. ກະລຸນາຖອນການຕິດຕັ້ງລຸ້ນກ່ອນກ່ອນອື່ນ ໝົດ ຫຼືດາວໂຫລດເວີຊັນທີ່ຖືກຕ້ອງເພື່ອຕິດຕັ້ງ.
nl.WarningWrongArchitecture =U probeert de %1-bits applicatieversie te installeren over de geïnstalleerde %2-bits versie. Verwijder eerst de vorige versie of download de juiste versie voor de installatie.
ru.WarningWrongArchitecture =Вы устанавливаете %1-битную версию приложения на уже установленную %2-битную. Пожалуйста, удалите предыдущую версию приложения или скачайте подходящую.
de.WarningWrongArchitecture =Sie versuchen die %1-Bit-Version der Anwendung über die %2-Bit-Version, die schon installiert ist, zu installieren. Entfernen Sie bitte die Vorgängerversion zuerst oder laden Sie die richtige Version für die Installation herunter.
fr.WarningWrongArchitecture =Vous essayez d'installer la version %1-bit sur la version %2-bit déjà installée. Veuillez désinstaller l'ancienne version d'abord ou télécharger la version correcte à installer.
es.WarningWrongArchitecture =Usted está tratando de instalar la versión de la aplicación de %1 bits sobre la versión de %2 bits instalada. Por favor, desinstale la versión anterior primero o descargue la versión correcta para la instalación.
in.WarningWrongArchitecture =Anda mencoba menginstal aplikasi versi %1-bit yang sudah terinstal versi %2-bit. Silakan uninstal terlebih dahulu versi sebelumnya atau unduh versi instalasi yang benar.
it_IT.WarningWrongArchitecture =Stai provando ad installare la versione dell'applicazione %1-bit sulla versione %2-bit installata. Si prega di disinstallare prima la versione precedente o scaricare la versione corretta per l'installazione.
pt_BR.WarningWrongArchitecture =Você está tentando instalar a versão do aplicativo de %1 bits por cima da versão de %2 bits instalada. Desinstale primeiro a versão anterior ou baixe a versão correta para instalação.
pt_PT.WarningWrongArchitecture =Está a tentar instalar a versão de %1-bites por cima da versão de %2-bites que já está instalada. Por favor desinstale primeiro a versão anterior ou faça o download da versão correta para instalar.
pl.WarningWrongArchitecture =Próbujesz zainstalować %1-bitową wersję aplikacji na %2-bitowej wersji zainstalowanej. Odinstaluj najpierw poprzednią wersję lub pobierz odpowiednią wersję dla instalacji.
ro.WarningWrongArchitecture =Încercați sã instalați o versiune a aplicației pe %1 biți pe deasupra versiunei deja instalate pe%2 biți. Mai întâi, dezinstalați versiunea anterioarã sau încãrcați versiunea portivitã.
sk.WarningWrongArchitecture =Pokúšate sa nainštalovať %1-bitovú verziu aplikácie cez nainštalovanú %2-bitovú verziu. Najskôr odinštalujte predchádzajúcu verziu alebo stiahnite správnu verziu na inštaláciu.
sl.WarningWrongArchitecture =Poskušate namestiti %1-bit verzijo aplikacije preko %2-bit verzijo, ki je nameščena. Odstranite najprej prejšnjo verzijo ali prenesite pravo verzijo za namestitev.
sv.WarningWrongArchitecture =Du försöker installera %1-bit-programversionen över %2-bit-programversionen som redan är installerad. Var vänlig avinstallera den föregående versionen eller ladda ner den korrekta versionen för installation.
tr.WarningWrongArchitecture =Yüklenmiş %2-bit uygulama üzerine %1-bit uygulama yüklemeye çalışıyorsunuz. Lütfen ilk olarak önceki sürümü kaldırın veya kurulum için doğru sürümü indirin.
vi.WarningWrongArchitecture =Bạn đang cố gắng cài đặt phiên bản ứng dụng %1-bit đè lên phiên bản %2-bit đã được cài đặt. Vui lòng gỡ phiên bản trước hoặc tải phiên bản đúng để cài đặt.
zh_CN.WarningWrongArchitecture =您正在尝试在已安装的%2-bit版本上安装%1-bit应用版本。请首先卸载之前版本，或下载正确的安装版本。
;======================================================================================================

en.UpdateAppRunning=Setup has detected that %1 is currently running.%n%nIt'll be closed automatically. Click OK to continue, or Cancel to exit.
bg.UpdateAppRunning=Настройката установи, че %1 в момента работи.%n%nТова ще бъде затворено автоматично. Щракнете върху OK, за да продължите, или Отказ, за да излезете.
ca.UpdateAppRunning=La configuració ha detectat que actualment s'està executant %1.%n%n Es tancarà automàticament. Feu clic a D'acord per continuar o Cancel·la per sortir.
cs.UpdateAppRunning=V rámci nastavení bylo zjištěno, že je aktuálně spuštěné 1%.%n%nBude automaticky zavřen. Chcete-li pokračovat, klikněte na tlačítko OK nebo Zrušit pro ukončení.
el.UpdateAppRunning=Η εγκατάσταση έχει εντοπίσει ότι το %1 εκτελείται αυτήν τη στιγμή. Κάντε κλικ στο Εντάξει για συνέχεια ή στο Άκυρο για έξοδο.
et.UpdateAppRunning=Süsteem on tuvastanud, et %1 töötab praegu. See suletakse automaatselt. Vajuta OK, et jätkata või Tühista, et katkestada.
fi.UpdateAppRunning=Asennus havaitsi, että %1 on käynnissä.%n%nSe suljetaan automaattisesti. Klikkaa OK jatkaaksesi tai Peruuta poistuaksesi asennuksesta.
lt.UpdateAppRunning=Sąranka nustatė, kad šiuo metu veikia %1.%n%nTai bus uždaroma automatiškai. Spustelkite GERAI jei norite tęsti, arba Atšaukti, jei norite išeiti.
lo.UpdateAppRunning=ການຕັ້ງຄ່າໄດ້ກວດພົບວ່າ% 1 ກຳລັງໃຊ້ງານຢູ່.% n% n ມັນຈະຖືກປິດໂດຍອັດຕະໂນມັດ. ກົດ OK ເພື່ອສືບຕໍ່, ຫຼືຍົກເລີກເພື່ອອອກຈາກ.
nl.UpdateAppRunning=Setup heeft vastgesteld dat %1 momenteel loopt.%n%nHet wordt automatisch gesloten. Klik op OK om door te gaan, of op Annuleren om af te sluiten.
ru.UpdateAppRunning=Обнаружен запущенный экземпляр %1.%n%nДля обновления он будет автоматически закрыт. Нажмите «OK», чтобы продолжить, или «Отмена», чтобы выйти.
de.UpdateAppRunning=Setup hat festgestellt, dass es aktuell %1 läuft. %n%nEs wird automatisch geschlossen. Klicken Sie zum Fortfahren auf OK oder auf Abbrechen zum Beenden des Programms.
fr.UpdateAppRunning=L'installation a détecté que %1 est en cours d'exécution. %n%nIl sera fermé automatiquement. Cliquez sur OK pour continuer, ou Annuler pour quitter le programme.
es.UpdateAppRunning=Programa de instalación ha detectado que actualmente %1 está funcionando.%n%nSe cerrará  automáticamente. Haga clic en OK para continuar o Cerrar para salir.
in.UpdateAppRunning=Pengaturan mendeteksi %1 sedang berjalan.%n%nAkan ditutup secara otomatis. Klik OK untuk melanjutkan, atau Batal untuk keluar.
it_IT.UpdateAppRunning= Il programma di installazione ha rilevato che% 1 è attualmente in esecuzione.%n%nVerrà chiuso automaticamente. Fare clic su OK per continuare o su Annulla per uscire.
pt_BR.UpdateAppRunning=A configuração detectou que %1 está atualmente em execução.%n%nEla será fechada automaticamente. Clique em OK para continuar ou em Cancelar para sair.
pt_PT.UpdateAppRunning=O Setup detetou que %1 está atualmente aberta.%n%nEla será fechada automaticamente. Clique OK para continuar, ou Cancelar para sair.
pl.UpdateAppRunning=Konfiguracja wykryła , że %1 jest uruchomiona.%n%nZostanie ona automatycznie zamknięta. Kliknij OK, aby kontynuować lub Anuluj, aby wyjść.
ro.UpdateAppRunning=Programul de instalare a detectat cã %1 ruleazã acum.%n%nAplicatia va fi inchisa automat. Apãsați OK dacã doriți sã continuați, sau apãsați Revicare dacã doriți sã ieșiți..
sk.UpdateAppRunning=Inštalátor zistil, že %1 je momentálne spustený.%n%nBude automaticky zatvorený. Pokračujte kliknutím na tlačidlo OK, zrušte akciu kliknutím na tlačidlo exit.
sl.UpdateAppRunning=Namestitveni program je zaznal, da trenutno poteka %1.%n%nSamodejno bo zaprt. Pritisnite V redu za nadaljevanje ali Prekliči za izhod.
sv.UpdateAppRunning=Installationsprogrammet har upptäckt att %1 för närvarande körs.%n%nDet kommer stängas automatiskt. Tryck på OK för att fortsätta, eller Avbryt för att avsluta.
tr.UpdateAppRunning=Kurulum %1’in şu anda çalışıyor olduğunu algıladı. %n%nIt otomatik olarak kapatılacak. Devam etmek için Tamam’a veya çıkmak için İptal’e tıklayın.
vi.UpdateAppRunning=Mục cài đặt đã phát hiện ra rằng %1 đang chạy.%n%nIt sẽ được đóng tự động. Hãy chọn OK để tiếp tục hay Hủy để thoát.
zh_CN.UpdateAppRunning=安装程序检测到%1当前正在运行。%n%n将自动关闭。单击“确定”继续，或“取消”退出。
;======================================================================================================
en.WarningClearAppData =Do you want to clear the user settings and application cached data?
bg.WarningClearAppData =Искате ли да изчистите потребителските настройки и кешираните данни на приложението?
ca.WarningClearAppData =Voleu esborrar la configuració de l'usuari i les dades de la memòria cau de l'aplicació?
cs.WarningClearAppData =Chcete zrušit uživatelské nastavení a údaje uložené v paměti?
el.WarningClearAppData =Θέλετε να διαγράψετε τις ρυθμίσεις χρήστη και τα δεδομένα cache της εφαρμογής;
et.WarningClearAppData =Kas sa tahad puhastada kasutaja seaded ja rakenduse salvestatud andmed?
fi.WarningClearAppData =Haluatko poistaa käyttäjä asetukset ja sovelluksen välimuistin?
lt.WarningClearAppData =Ar norite išvalyti vartotojo nustatymus ir programos talpyklos duomenis?
lo.WarningClearAppData =ທ່ານຕ້ອງການລຶບລ້າງຂໍ້ມູນການຕັ້ງຄ່າຂອງຜູ້ໃຊ້ແລະຂໍ້ມູນທີ່ເກັບຂໍ້ມູນໄວ້?
nl.WarningClearAppData =Wilt u de gebruikersinstellingen en de in de cache opgeslagen gegevens wissen?
ru.WarningClearAppData =Вы хотите очистить пользовательские настройки и кэш приложения?
de.WarningClearAppData =Möchten Sie die Benutzereinstellungen und die zwischengespeicherten Daten der Anwendung löschen?
fr.WarningClearAppData =Voulez-vous effacer les paramètres utilisateur et les données en cache de l'application ?
es.WarningClearAppData =¿Desea eliminar los ajustes de usuario y datos en caché de la aplicación?
in.WarningClearAppData =Apakah Anda ingin menghapus pengaturan pengguna dan data cache aplikasi?
it_IT.WarningClearAppData =Vuoi cancellare le impostazioni utente e i dati memorizzati nella cache dell’applicazione?
pt_BR.WarningClearAppData =Você deseja limpar as definições de usuário e dados salvos do programa?
pt_PT.WarningClearAppData =Quer apagar as definições de utilizador e os dados de cache da aplicação?
pl.WarningClearAppData =Czy chcesz usunąć ustawienia użytkownika oraz dane pamięci podręcznej aplikacji?
ro.WarningClearAppData =Doriți sã mergeţi setãrile personale și sã goliți memoria cache?
sk.WarningClearAppData =Chcete vymazať nastavenia používateľa a údaje uložené v pamäti aplikácie?
sl.WarningClearAppData =Ali želite počistiti uporabniške nastavitve in predpomnjene podatke aplikacije?
sv.WarningClearAppData =Vill du rensa användarinställningar och programmets cachade data?
tr.WarningClearAppData =Kullanıcı ayarlarını veya uygulama önbelleğine alınmış verileri silmek istiyor musunuz?
vi.WarningClearAppData =Bạn có muốn xóa thiết lập người dùng và dữ liệu được lưu trong bộ nhớ cache của ứng dụng không?
zh_CN.WarningClearAppData =您是否要清除用户设置和应用缓存数据？
;======================================================================================================


;en.AssociateDescription =Associate office document file types with %1
;bg.AssociateDescription =Свържете типовете файлове на офис документи с %1
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

  Params := '/quiet /norestart';

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

Source: {#DEPLOY_PATH}\*;                               DestDir: {app}; Flags: recursesubdirs;
Source: {#DEPLOY_PATH}\*.exe;                           DestDir: {app}; Flags: signonce;
Source: {#DEPLOY_PATH}\ascdocumentscore.dll;            DestDir: {app}; Flags: signonce;
Source: {#DEPLOY_PATH}\hunspell.dll;                    DestDir: {app}; Flags: signonce;
Source: {#DEPLOY_PATH}\ooxmlsignature.dll;              DestDir: {app}; Flags: signonce;
Source: {#DEPLOY_PATH}\WinSparkle.dll;                  DestDir: {app}; Flags: signonce;
Source: {#DEPLOY_PATH}\converter\*.dll;                 DestDir: {app}\converter; Flags: signonce;
Source: {#DEPLOY_PATH}\converter\*.exe;                 DestDir: {app}\converter; Flags: signonce;

[InstallDelete]
Type: filesandordirs; Name: {app}\editors\sdkjs-plugins

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

#ifdef _ONLYOFFICE
Root: HKLM; Subkey: "SOFTWARE\Classes\{#sAppProtocol}"; ValueType: "string"; ValueData: "URL:{#sAppName} Protocol"; Flags: uninsdeletekey;
Root: HKLM; Subkey: "SOFTWARE\Classes\{#sAppProtocol}"; ValueType: "string"; ValueName: "URL Protocol"; ValueData: "";
Root: HKLM; Subkey: "SOFTWARE\Classes\{#sAppProtocol}\DefaultIcon"; ValueType: "string"; ValueData: "{app}\{#iconsExe},0";
Root: HKLM; Subkey: "SOFTWARE\Classes\{#sAppProtocol}\Shell\Open\Command"; ValueType: "string"; ValueData: """{app}\{#iconsExe}"" ""%1""";
#endif

[UninstallDelete]
Type: filesandordirs; Name: {commonappdata}\{#APP_PATH}\*;  AfterInstall: RefreshEnvironment;
