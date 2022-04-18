/*
 * (c) Copyright Ascensio System SIA 2010-2019
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
 * You can contact Ascensio System SIA at 20A-12 Ernesta Birznieka-Upisha
 * street, Riga, Latvia, EU, LV-1050.
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

#ifndef UTILS_H
#define UTILS_H

#include <QStringList>
#include <QFileInfo>
#include <QWidget>
#ifdef Q_OS_WIN
# include <Windows.h>
#endif

namespace InputArgs {
    auto init(int argc, char** const argv) -> void;
    auto init(wchar_t const * argv) -> void;
    auto contains(const std::wstring&) -> bool;
    auto argument_value(const std::wstring& param) -> std::wstring;
    auto arguments() -> const std::vector<std::wstring>&;

    auto webapps_params() -> const std::wstring&;
    auto set_webapps_params(const std::wstring&) -> void;
    auto change_webapps_param(const std::wstring& from, const std::wstring& to) -> const std::wstring&;
}

class Utils {
public:
    static QStringList * getInputFiles(const QStringList& inlist);
    static QString lastPath(int type);
    static void keepLastPath(int type, const QString&);
    static QString getUserPath();
    static std::wstring systemUserName();
    static std::wstring appUserName();
    static QString getAppCommonPath();
    static QRect getScreenGeometry(const QPoint&);
    static void openUrl(const QString&);
    static void openFileLocation(const QString&);
    static QString getPortalName(const QString&);
    static double getScreenDpiRatio(int);
    static double getScreenDpiRatio(const QPoint&);
    static double getScreenDpiRatioByHWND(int);
    static double getScreenDpiRatioByWidget(QWidget*);
    static QScreen * screenAt(const QPoint&);
    static QString replaceBackslash(const QString&);
    static bool isFileLocal(const QString&);
    static bool setAppUserModelId(const QString&);

    static bool makepath(const QString&);

    static QString systemLocationCode();
    static QIcon appIcon();

    static QString stringifyJson(const QJsonObject&);

    static QByteArray readStylesheets(std::vector<std::string> const *);
    static QByteArray readStylesheets(const QString&);
    static QJsonObject parseJson(const std::wstring&);
};

namespace WindowHelper {
//#ifdef Q_OS_LINUX
    class CParentDisable
    {
        QWidget* m_pChild = nullptr;
    public:
        CParentDisable(QWidget* parent = nullptr);
        ~CParentDisable();

        void disable(QWidget* parent);
        void enable();
    };

//    auto check_button_state(Qt::MouseButton b) -> bool;
//#else
#ifdef Q_OS_WIN
    auto isWindowSystemDocked(HWND handle) -> bool;
    auto correctWindowMinimumSize(HWND handle) -> void;
    auto correctModalOrder(HWND windowhandle, HWND modalhandle) -> void;
    auto adjustWindowRect(HWND, double, LPRECT) -> void;
#endif

    auto correctWindowMinimumSize(const QRect&, const QSize&) -> QSize;
    auto isLeftButtonPressed() -> bool;
    auto constructFullscreenWidget(QWidget * panel) -> QWidget *;
}

#endif // UTILS_H

