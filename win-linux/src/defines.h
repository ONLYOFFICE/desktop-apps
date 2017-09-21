/*
 * (c) Copyright Ascensio System SIA 2010-2017
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
#define reCmdLang           "--(keep)?lang:(\\w{2,5})"


#define FILE_DOWNLOAD_START     3
#define FILE_DOWNLOAD_END       4

#define APP_NAME "DesktopEditors"
#define APP_TITLE "ONLYOFFICE"
#ifdef __linux
# define APP_DATA_PATH "/onlyoffice/desktopeditors"
# define REG_GROUP_KEY "onlyoffice"
#else
# define APP_DATA_PATH "/ONLYOFFICE/DesktopEditors"
# define REG_GROUP_KEY "ONLYOFFICE"
#endif

#define WINDOW_NAME "ONLYOFFICE Desktop Editors"
#define REG_APP_NAME "DesktopEditors"
#define APP_MUTEX_NAME "TEAMLAB"

#define URL_SITE                "http://www.onlyoffice.com"
//#define URL_APPCAST_UPDATES     ""
#define URL_SIGNUP              "https://onlyoffice.com/registration.aspx?desktop=true"

#define WINDOW_TITLE_MIN_WIDTH 400
#define MAIN_WINDOW_MIN_WIDTH  800
#define MAIN_WINDOW_MIN_HEIGHT 600

#define GET_REGISTRY_USER(variable) \
    QSettings variable(QSettings::NativeFormat, QSettings::UserScope, REG_GROUP_KEY, REG_APP_NAME);
#define GET_REGISTRY_SYSTEM(variable) \
    QSettings variable(QSettings::SystemScope, REG_GROUP_KEY, REG_APP_NAME);

#define LOCAL_PATH_OPEN         1
#define LOCAL_PATH_SAVE         2

#define MODAL_RESULT_YES        1
#define MODAL_RESULT_NO         0
#define MODAL_RESULT_CANCEL     -1
#define MODAL_RESULT_CUSTOM     100

#define MESSAGE_TYPE_INFO       1
#define MESSAGE_TYPE_WARN       2
#define MESSAGE_TYPE_CONFIRM    3
#define MESSAGE_TYPE_ERROR      4

#define ACTIONPANEL_CONNECT     255
#define ACTIONPANEL_ACTIVATE    ACTIONPANEL_CONNECT + 1

#define URL_AGPL "https://www.gnu.org/licenses/agpl-3.0.en.html"

#ifdef __linux
typedef unsigned char BYTE;
#else
# define UM_INSTALL_UPDATE      WM_USER+254
# define UM_CLOSE_MAINWINDOW    WM_USER+253
#endif

#include "defines_p.h"

#endif // DEFINES_H

