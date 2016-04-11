/*
 * (c) Copyright Ascensio System SIA 2010-2016
 *
 * This program is a free software product. You can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License (AGPL)
 * version 3 as published by the Free Software Foundation. In accordance with
 * Section 7(a) of the GNU AGPL its Section 15 shall be amended to the effect
 * that Ascensio System SIA expressly excludes the warranty of non-infringement
 * of any third-party rights.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR  PURPOSE. For
 * details, see the GNU AGPL at: http://www.gnu.org/licenses/agpl-3.0.html
 *
 * You can contact Ascensio System SIA at Lubanas st. 125a-25, Riga, Latvia,
 * EU, LV-1021.
 *
 * The  interactive user interfaces in modified source and object code versions
 * of the Program must display Appropriate Legal Notices, as required under
 * Section 5 of the GNU AGPL version 3.
 *
 * Pursuant to Section 7(b) of the License you must retain the original Product
 * logo when distributing the program. Pursuant to Section 7(e) we decline to
 * grant you any rights under trademark law for use of our trademarks.
 *
 * All the Product's GUI elements, including illustrations and icon sets, as
 * well as technical writing content are licensed under the terms of the
 * Creative Commons Attribution-ShareAlike 4.0 International. See the License
 * terms at http://creativecommons.org/licenses/by-sa/4.0/legalcode
 *
*/

#ifndef DEFINES_H
#define DEFINES_H

#define rePortalName        "^https?:\\/\\/(.+)"
#define reFileExtension     "\\.(\\w{1,10})$"
#define reUserName          "([^\\.]+)\\.?([^\\.]+)?"
#define reCmdLang           "^--(?:keep)?lang:\\w{2}"
#define reCmdKeepLang       "^--keeplang:\\w{2}"


#define FILE_DOWNLOAD_START     3
#define FILE_DOWNLOAD_END       4

#ifdef _IVOLGA_PRO
  #define PROD_ID_DESKTOP_EDITORS 301

  #define APP_TITLE g_lang.compare("ru") == 0 ? "Иволга ПРО" : "Ivolga PRO"
  #define WINDOW_NAME APP_TITLE

  #ifdef __linux
    #define APP_DATA_PATH "/ivolgapro/desktopeditors"
    #define APP_LICENSE_PATH "/ivolgapro/license"
    #define REG_GROUP_KEY "ivolgapro"
    #define LIC_KEY_FILENAME ".doceditors.ivo.asc"
  #else
    #define APP_DATA_PATH "/IvolgaPRO/DesktopEditors"
    #define APP_LICENSE_PATH "/IvolgaPRO/License"
    #define REG_GROUP_KEY "IvolgaPRO"
    #define LIC_KEY_FILENAME "doceditors.ivo.asc"
  #endif

  #define URL_BUYNOW "http://www.ivolgapro.ru/buynow"
  #define REG_APP_NAME "DesktopEditors"
#elif defined(_AVS)
  #define PROD_ID_DESKTOP_EDITORS 98

  #define APP_NAME "DocumentEditor"
  #define APP_TITLE "AVS Document Editor"

  #ifdef __linux
    #define APP_DATA_PATH "/ivolgapro/desktopeditors"
    #define APP_LICENSE_PATH "/ivolgapro/license"
    #define REG_GROUP_KEY "ivolgapro"
//    #define LIC_KEY_FILENAME ".doceditors.avs"
  #else
    #define APP_DATA_PATH "/AVS4YOU/AVSDocumentEditor"
    #define APP_LICENSE_PATH "/AVS4YOU/Licence"
    #define REG_GROUP_KEY "AVS4YOU"
    #define LIC_KEY_FILENAME "doceditors.avs"
  #endif

  #define WINDOW_NAME "AVS Document Editor"
  #define URL_BUYNOW "http://www.avs4you.com/Register.aspx?utm_source=300&utm_medium=Register&utm_content=Register"
  #define REG_APP_NAME "DocumentEditor"
#else
  #define PROD_ID_DESKTOP_EDITORS 302

  #define APP_NAME "DesktopEditors"
  #define APP_TITLE "ONLYOFFICE"

  #ifdef __linux
    #define APP_DATA_PATH "/onlyoffice/desktopeditors"
    #define APP_LICENSE_PATH "/onlyoffice/license"
    #define REG_GROUP_KEY "onlyoffice"
    #define LIC_KEY_FILENAME ".doceditors.asc"
  #else
    #define APP_DATA_PATH "/ONLYOFFICE/DesktopEditors"
    #define APP_LICENSE_PATH "/ONLYOFFICE/License"
    #define REG_GROUP_KEY "ONLYOFFICE"
    #define LIC_KEY_FILENAME "doceditors.asc"
  #endif

  #define WINDOW_NAME "ONLYOFFICE Desktop Editors"
  #define URL_BUYNOW "http://www.onlyoffice.com/desktopeditors.aspx"
  #define REG_APP_NAME "DesktopEditors"
#endif

#define WINDOW_TITLE_MIN_WIDTH 400

#define GET_REGISTRY_USER(variable) \
    QSettings variable(QSettings::NativeFormat, QSettings::UserScope, REG_GROUP_KEY, REG_APP_NAME);
#define GET_REGISTRY_SYSTEM(variable) \
    QSettings variable(QSettings::SystemScope, REG_GROUP_KEY, REG_APP_NAME);

#define LOCAL_PATH_OPEN         1
#define LOCAL_PATH_SAVE         2

#define MODAL_RESULT_YES        1
#define MODAL_RESULT_NO         0
#define MODAL_RESULT_CANCEL     -1
#define MODAL_RESULT_BTN1       201
#define MODAL_RESULT_BTN2       202

#define ACTIONPANEL_CONNECT     255
#define ACTIONPANEL_ACTIVATE    ACTIONPANEL_CONNECT + 1

#define LICENSE_TYPE_FREE       255
#define LICENSE_TYPE_BUSINESS   254
#define LICENSE_TYPE_TRIAL      253
#define LICENSE_TYPE_NONE       0

#ifdef __linux
typedef unsigned char BYTE;
#endif

#endif // DEFINES_H

