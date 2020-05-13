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

using namespace std;
namespace InputArgs {
    auto contains(const QString&) -> bool;
    auto get_arg_value(const QString& param) -> QString;
}

class Utils {
public:
    static QStringList * getInputFiles(const QStringList& inlist);
    static QString lastPath(int type);
    static void keepLastPath(int type, const QString&);
    static QString getUserPath();
    static wstring systemUserName();
    static wstring appUserName();
    static QString getAppCommonPath();
    static QRect getScreenGeometry(const QPoint&);
    static void openUrl(const QString&);
    static void openFileLocation(const QString&);
    static QString getPortalName(const QString&);
    static unsigned getScreenDpiRatio(int);
    static unsigned getScreenDpiRatio(const QPoint&);
    static unsigned getScreenDpiRatioByHWND(int);
    static unsigned getScreenDpiRatioByWidget(QWidget*);
    static QString replaceBackslash(const QString&);
    static bool isFileLocal(const QString&);
    static bool setAppUserModelId(const QString&);

    static bool makepath(const QString&);

    static QString systemLocationCode();
    static QIcon appIcon();

    static QString encodeJson(const QJsonObject&);
    static QString encodeJson(const QString&);
    static wstring encodeJson(const wstring&);

//    static QByteArray getAppStylesheets(int);
    static QByteArray readStylesheets(std::vector<QString> *, std::vector<QString> *, int);
    static QByteArray readStylesheets(std::vector<QString> *);
    static QByteArray readStylesheets(const QString&);

#ifdef Q_OS_WIN
    //TODO: move to window.base class
    static void adjustWindowRect(HWND, int, LPRECT);
#endif
};

namespace WindowHelper {
#ifdef Q_OS_LINUX
    class CParentDisable
    {
        QWidget* m_pChild = nullptr;
    public:
        CParentDisable(QWidget* parent = nullptr);
        ~CParentDisable();

        void disable(QWidget* parent);
        void enable();
    };
#else
    auto isLeftButtonPressed() -> bool;
    auto isWindowSystemDocked(HWND handle) -> bool;
    auto correctWindowMinimumSize(HWND handle) -> void;
    auto correctModalOrder(HWND windowhandle, HWND modalhandle) -> void;
#endif
}

#endif // UTILS_H

