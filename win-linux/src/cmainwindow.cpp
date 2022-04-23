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



#include "cmainwindow.h"
#include "cascapplicationmanagerwrapper.h"
#include "defines.h"
#include "utils.h"
#include "csplash.h"
#include "clogger.h"
#include "clangater.h"
#include <QDesktopWidget>
#include <QGridLayout>
#include <QTimer>
#include <stdexcept>
#include <functional>
#include <QApplication>
#include <QIcon>


#ifdef _UPDMODULE
  #include "3dparty/WinSparkle/include/winsparkle.h"
  #include "../version.h"
#endif

using namespace std::placeholders;


CMainWindow::CMainWindow(const QRect &rect) :
    CWindowPlatform(rect, WindowType::MAIN)
{
    _m_pMainPanel = new CMainPanelImpl(centralWidget(), true, m_dpiRatio);
    centralWidget()->layout()->addWidget(_m_pMainPanel);
    _m_pMainPanel->setStyleSheet(AscAppManager::getWindowStylesheets(m_dpiRatio));
    _m_pMainPanel->updateScaling(m_dpiRatio);
    _m_pMainPanel->goStart();

    CMainPanel * mainpanel = static_cast<CMainPanel*>(_m_pMainPanel);
    QObject::connect(mainpanel, &CMainPanel::mainWindowChangeState, bind(&CMainWindow::setWindowState, this, _1));
    QObject::connect(mainpanel, &CMainPanel::mainWindowWantToClose, std::bind(&CMainWindow::onCloseEvent, this));
    QObject::connect(mainpanel, &CMainPanel::mainPageReady, std::bind(&CMainWindow::slot_mainPageReady, this));
    QObject::connect(&AscAppManager::getInstance().commonEvents(), &CEventDriver::onModalDialog, this, &CMainWindow::slot_modalDialog);
}

CMainWindow::~CMainWindow()
{

}

/** Public **/

QWidget * CMainWindow::editor(int index)
{
    return mainPanel()->tabWidget()->panel(index);
}

QRect CMainWindow::windowRect() const
{
    return normalGeometry();
}

QString CMainWindow::documentName(int vid)
{
    int i = mainPanel()->tabWidget()->tabIndexByView(vid);
    if ( !(i < 0) ) {
        return mainPanel()->tabWidget()->panel(i)->data()->title();
    }
    return "";
}

void CMainWindow::selectView(int viewid) const
{
    int _index = mainPanel()->tabWidget()->tabIndexByView(viewid);
    if ( !(_index < 0) ) {
        mainPanel()->tabWidget()->setCurrentIndex(_index);
        mainPanel()->toggleButtonMain(false);
    }
}

void CMainWindow::selectView(const QString& url) const
{
    int _index = mainPanel()->tabWidget()->tabIndexByUrl(url);
    if ( !(_index < 0) ) {
        mainPanel()->tabWidget()->setCurrentIndex(_index);
        mainPanel()->toggleButtonMain(false);
    }
}

int CMainWindow::attachEditor(QWidget * panel, int index)
{
    CMainPanel * _pMainPanel = mainPanel();

    if (!QCefView::IsSupportLayers())
    {
        CTabPanel * _panel = dynamic_cast<CTabPanel *>(panel);
        if (_panel)
            _panel->view()->SetCaptionMaskSize(0);
    }

    int _index = _pMainPanel->tabWidget()->insertPanel(panel, index);
    if ( !(_index < 0) ) {
        _pMainPanel->toggleButtonMain(false);

        _pMainPanel->tabWidget()->setCurrentIndex(_index);
    }
    return _index;
}

int CMainWindow::attachEditor(QWidget * panel, const QPoint& pt)
{
    CMainPanel * _pMainPanel = mainPanel();
    QPoint _pt_local = _pMainPanel->tabWidget()->tabBar()->mapFromGlobal(pt);
#ifdef Q_OS_WIN
# if (QT_VERSION < QT_VERSION_CHECK(5, 10, 0))
    QPoint _tl = windowRect().topLeft();
    if ( _tl.x() < _pt_local.x() && _tl.y() < _pt_local.y() )
        _pt_local -= windowRect().topLeft();
# endif
#endif
    int _index = _pMainPanel->tabWidget()->tabBar()->tabAt(_pt_local);

    if ( !(_index < 0) ) {
        QRect _rc_tab = _pMainPanel->tabWidget()->tabBar()->tabRect(_index);
        if ( _pt_local.x() > _rc_tab.left() + (_rc_tab.width() / 2) ) ++_index;
    }

    return attachEditor(panel, _index);
}

int CMainWindow::editorsCount() const
{
    return mainPanel()->tabWidget()->count(cvwtEditor);
}

int CMainWindow::editorsCount(const std::wstring& portal) const
{
    return mainPanel()->tabWidget()->count(portal, true);
}

bool CMainWindow::pointInTabs(const QPoint& pt) const
{
    QRect _rc_title(mainPanel()->geometry());
    _rc_title.setHeight(mainPanel()->tabWidget()->tabBar()->height());
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
#ifdef Q_OS_LINUX
    _rc_title.moveTop(1);
#endif
#endif
    return _rc_title.contains(mainPanel()->mapFromGlobal(pt));
}

bool CMainWindow::holdView(int id) const
{
    return mainPanel()->holdUid(id);
}

CMainPanel * CMainWindow::mainPanel() const
{
    return _m_pMainPanel;
}

void CMainWindow::applyTheme(const std::wstring& theme)
{
    CWindowPlatform::applyTheme(theme);
    mainPanel()->applyTheme(theme);
}

#ifdef _UPDMODULE
void CMainWindow::checkUpdates()
{
    win_sparkle_check_update_with_ui();
}

void CMainWindow::setAutocheckUpdatesInterval(const QString& s)
{
    if ( s == "never" )
        win_sparkle_set_automatic_check_for_updates(0);
    else {
        win_sparkle_set_automatic_check_for_updates(1);

        s == "week" ?
            win_sparkle_set_update_check_interval(RATE_MS_WEEK):
                win_sparkle_set_update_check_interval(RATE_MS_DAY);

    }
}
#endif

/** Private **/

#ifdef _UPDMODULE
void CMainWindow::updateFound()
{
    CLogger::log("updates found");
}

void CMainWindow::updateNotFound()
{
    CLogger::log("updates isn't found");
}

void CMainWindow::updateError()
{
    CLogger::log("updates error");
}
#endif

void CMainWindow::setWindowState(Qt::WindowState state)
{
    switch (state) {
    case Qt::WindowMaximized:  showMaximized(); break;
    case Qt::WindowMinimized:  showMinimized(); break;
    case Qt::WindowFullScreen: hide(); break;
    case Qt::WindowNoState:
    default: showNormal(); break;}
}

void CMainWindow::slot_mainPageReady()
{
    CSplash::hideSplash();

#ifdef _UPDMODULE
    GET_REGISTRY_SYSTEM(reg_system)

    OSVERSIONINFO osvi;

    ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    GetVersionEx(&osvi);

    // skip updates for XP
    if ( osvi.dwMajorVersion > 5 && reg_system.value("CheckForUpdates", true).toBool() ) {
        win_sparkle_set_lang(CLangater::getCurrentLangCode().toLatin1());

        const std::wstring argname{L"--updates-appcast-url"};
        QString _appcast_url = !InputArgs::contains(argname) ? URL_APPCAST_UPDATES : QString::fromStdWString(InputArgs::argument_value(argname));
        static bool _init = false;
        if ( !_init ) {
            _init = true;

            QString _prod_name = WINDOW_NAME;

            GET_REGISTRY_USER(_user)
            if (!_user.contains("CheckForUpdates")) {
                _user.setValue("CheckForUpdates", "1");
            }

            win_sparkle_set_app_details(QString(VER_COMPANYNAME_STR).toStdWString().c_str(),
                                            _prod_name.toStdWString().c_str(),
                                            QString(VER_FILEVERSION_STR).toStdWString().c_str());
            win_sparkle_set_appcast_url(_appcast_url.toStdString().c_str());
            win_sparkle_set_registry_path(QString("Software\\%1\\%2").arg(REG_GROUP_KEY).arg(REG_APP_NAME).toLatin1());

            win_sparkle_set_did_find_update_callback(&CMainWindow::updateFound);
            win_sparkle_set_did_not_find_update_callback(&CMainWindow::updateNotFound);
            win_sparkle_set_error_callback(&CMainWindow::updateError);

            win_sparkle_init();
        }

        AscAppManager::sendCommandTo(0, "updates:turn", "on");
        CLogger::log("updates is on: " + _appcast_url);

#define RATE_MS_DAY 3600*24
#define RATE_MS_WEEK RATE_MS_DAY*7

        std::wstring _wstr_rate{L"day"};
        if ( !win_sparkle_get_automatic_check_for_updates() ) {
            _wstr_rate = L"never";
        } else {
            int _rate{win_sparkle_get_update_check_interval()};
            if ( !(_rate < RATE_MS_WEEK) ) {
                if ( _rate != RATE_MS_WEEK )
                    win_sparkle_set_update_check_interval(RATE_MS_WEEK);

                _wstr_rate = L"week";
            } else {
                if ( _rate != RATE_MS_DAY )
                    win_sparkle_set_update_check_interval(RATE_MS_DAY);
            }
        }

        AscAppManager::sendCommandTo(0, L"settings:check.updates", _wstr_rate);
    }
#endif
}

void CMainWindow::onCloseEvent()
{
    if (isVisible()) {
        GET_REGISTRY_USER(reg_user)
        if (isMaximized()) {
            reg_user.setValue("maximized", true);
        } else {
            reg_user.remove("maximized");
            reg_user.setValue("position", geometry());
        }
    }
    AscAppManager::closeMainWindow();
}

void CMainWindow::applyWindowState(Qt::WindowState s)
{
    _m_pMainPanel->applyMainWindowState(s);
}

void CMainWindow::focus()
{
    _m_pMainPanel->focus();
}
