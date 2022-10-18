; -- ONLYOFFICE Desktop Editors Defines --

#define sCompanyName                    "ONLYOFFICE"
#define sIntCompanyName                 sCompanyName
#define sProductName                    "Desktop Editors"
#define sIntProductName                 "DesktopEditors"
#define sAppName                        str(sCompanyName + " " + sProductName)
#define sPackageName                    str(sIntCompanyName + "_" + sIntProductName)
#define sAppPublisher                   "Ascensio System SIA"
#define sAppPublisherURL                "https://www.onlyoffice.com/"
#define sAppSupportURL                  "https://www.onlyoffice.com/support.aspx"
#define sAppCopyright                   str("Copyright (C) " + GetDateTimeString("yyyy",,) + " " + sAppPublisher)
#define sAppIconName                    "ONLYOFFICE Editors"
#define sAppProtocol                    'oo-office'

#define sAppHelpName                    str(sAppName + " Help")
#define sPackageHelpName                str(sPackageName + "_Help")

#define APP_PATH                        str(sIntCompanyName + "\" + sIntProductName)
#define APP_REG_PATH                    str("Software\" + APP_PATH)
#ifndef DEPLOY_PATH
  #define DEPLOY_PATH                   str("..\..\..\..\build_tools\out\" + sPlatformFull + "\" + APP_PATH)
#endif
#define APP_USER_MODEL_ID               "ASC.Documents.5"
#define APP_MUTEX_NAME                  "TEAMLAB"
#define APPWND_CLASS_NAME               "DocEditorsWindowClass"

#define iconsExe                        "DesktopEditors.exe"
#define NAME_EXE_IN                     str("DesktopEditors_" + sWinArch + ".exe")
#define NAME_EXE_OUT                    "editors.exe"
#define VISEFFECTS_MANIFEST_NAME        ChangeFileExt(iconsExe, "VisualElementsManifest.xml")
#define LIC_FILE                        "agpl-3.0"

#define ASSC_APP_NAME                   "ONLYOFFICE"
#define ASCC_REG_PREFIX                 "ASC"
#define ASCC_REG_REGISTERED_APP_NAME    "ONLYOFFICE Editors"
#define ASSOC_PROG_ID                   "ASC.Editors"
#define ASSOC_APP_FRIENDLY_NAME         "ONLYOFFICE Editors"
