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


#include "csplash.h"
#include "defines.h"
#include <QApplication>
#include <QScreen>
#include <QSettings>
#include <QStyle>
#include "utils.h"
#include "csplash_p.cpp"

CSplash * _splash;

auto screenAt(const QPoint& pt) -> QScreen * {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
    return QApplication::screenAt(pt);
#else
    QVarLengthArray<const QScreen *, 8> _cached_screens;
    for (const QScreen *screen : QApplication::screens()) {
        if (_cached_screens.contains(screen))
            continue;

        for (QScreen *sibling : screen->virtualSiblings()) {
            if (sibling->geometry().contains(pt))
                return sibling;

            _cached_screens.append(sibling);
        }
    }

    return nullptr;
#endif
}

CSplash::CSplash(const QPixmap &p, Qt::WindowFlags f)
    : QSplashScreen(p, f)
{
    _splash = NULL;
}

void CSplash::show(int scrnum)
{
    setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, size(), QApplication::screens().at(scrnum)->availableGeometry()));
    QSplashScreen::show();
}

void CSplash::showSplash()
{
    if ( !_splash ) {
        GET_REGISTRY_USER(reg_user)

        int _scr_num = 0;
        _splash = new CSplash(QPixmap(), Qt::WindowStaysOnTopHint);

        if (QApplication::screens().count() > 1) {
            QScreen * _screen = screenAt(reg_user.value("position").toRect().topLeft());

            if ( _screen ) {
                _splash->move(_screen->geometry().center());
                _scr_num = QApplication::screens().indexOf(_screen);
            }
        }

        uchar _dpi_ratio = Utils::getScreenDpiRatioByHWND(_splash->winId());

//        _splash->setPixmap(_dpi_ratio > 1 ? QPixmap(":/res/icons/splash_2x.png") : QPixmap(":/res/icons/splash.png"));
        _splash->setPixmap(getSplashImage(_dpi_ratio));
        _splash->show(_scr_num);
    }
}

void CSplash::hideSplash()
{
    if (_splash) {
//        g_splash->setParent((QWidget *)parent());
        _splash->close();

        delete _splash, _splash = NULL;
    }
}

uint CSplash::startupDpiRatio()
{
    if (_splash) {
        return Utils::getScreenDpiRatioByHWND(_splash->winId());
    } else {
        QSplashScreen splash;

        if (QApplication::screens().count() > 1) {
            GET_REGISTRY_USER(reg_user)

            QScreen * _screen = screenAt(reg_user.value("position").toRect().topLeft());
            if ( _screen ) {
                splash.move(_screen->geometry().center());
            }
        }

        return Utils::getScreenDpiRatioByHWND(splash.winId());
    }

    return 1;
}
