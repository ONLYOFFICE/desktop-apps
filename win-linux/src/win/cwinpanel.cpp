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

#include <windows.h>

#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QPushButton>
#include <QScrollArea>
#include <QStandardPaths>
#include <QTimer>

#include "cwinpanel.h"
#include <QMenu>

#include <QJsonDocument>
#include <QJsonObject>

#include <QDir>
#include <QDialog>
#include <QWindow>
#include <QWidgetAction>
#include <QDesktopServices>
#include <QFileInfo>

#include "../defines.h"
#include "../utils.h"
#include "../csplash.h"
#include "../clangater.h"

//#include <QScreen>
#include <QSettings>
#include <QPrinterInfo>

#ifdef _UPDMODULE
  #include "3dparty/WinSparkle/include/winsparkle.h"
  #include "../version.h"
#endif


CWinPanel::CWinPanel( HWND hWnd, CAscApplicationManager* pManager )
    : QWinWidget( hWnd )
{
    windowHandle = hWnd;
    m_pManager = pManager;

//    setObjectName("mainPanel");

    CMainPanelImpl * panel = new CMainPanelImpl(this, pManager, true);
    m_pMainPanel = panel;

    show();

    connect(panel, &CMainPanelImpl::mainWindowChangeState, this, &CWinPanel::slot_windowChangeState);
    connect(panel, &CMainPanelImpl::mainWindowClose, this, &CWinPanel::slot_windowClose);
    connect(panel, &CMainPanelImpl::mainPageReady, this, &CWinPanel::slot_mainPageReady);

#ifdef _UPDMODULE
    connect(panel, &CMainPanelImpl::checkUpdates, this, []{
        win_sparkle_check_update_with_ui();
    });
#endif

    /*
    CAscLocalOpenFiles * pData = (CAscLocalOpenFiles *)data;
    vector<std::wstring>& vctFiles = pData->get_Files();

    for (vector<wstring>::iterator i = vctFiles.begin(); i != vctFiles.end(); i++) {
        COpenOptions opts = {(*i), etLocalFile};
        m_lastOpenPath = QFileInfo(opts.url).absoluteDir().absolutePath();
        openLocalFile(opts);
    }

    */

//    m_pManager->SetEventListener(this);

    panel->setInputFiles(Utils::getInputFiles(qApp->arguments()));
//    parseInputArgs(qApp->arguments());
}

void CWinPanel::parseInputArgs(const QStringList& args)
{
}

bool CWinPanel::nativeEvent( const QByteArray &, void * msg, long * result)
{
    Q_UNUSED(result);
    MSG *message = ( MSG * )msg;
    switch ( message->message ) {
    case WM_SYSKEYDOWN:
        if ( message->wParam == VK_SPACE )
        {
            RECT winrect;
            GetWindowRect( windowHandle, &winrect );
            TrackPopupMenu( GetSystemMenu( windowHandle, false ), TPM_TOPALIGN | TPM_LEFTALIGN, winrect.left + 5, winrect.top + 5, 0, windowHandle, NULL);
        }
        break;
    case WM_KEYDOWN:
        if ( message->wParam == VK_F5 || message->wParam == VK_F6 || message->wParam == VK_F7)
        {
            SendMessage( windowHandle, WM_KEYDOWN, message->wParam, message->lParam );
        }
        break;
    }

    return false;
}

void CWinPanel::mousePressEvent( QMouseEvent *event )
{
    if ( event->button() == Qt::LeftButton )
    {
        ReleaseCapture();
        SendMessage( windowHandle, WM_NCLBUTTONDOWN, HTCAPTION, 0 );
    }

    if ( event->type() == QEvent::MouseButtonDblClick )
    {
        if (event -> button() == Qt::LeftButton)
        {
            WINDOWPLACEMENT wp;
            wp.length = sizeof( WINDOWPLACEMENT );
            GetWindowPlacement( parentWindow(), &wp );
            if ( wp.showCmd == SW_MAXIMIZE )
            {
                ShowWindow( parentWindow(), SW_RESTORE );
            }
            else
            {
                ShowWindow( parentWindow(), SW_MAXIMIZE );
            }
        }
    }
}

void CWinPanel::resizeEvent(QResizeEvent* event)
{
    QWinWidget::resizeEvent(event);
    m_pMainPanel->setGeometry(QRect(0, 0, event->size().width(), event->size().height()));
}

CMainPanelImpl * CWinPanel::getMainPanel()
{
    return m_pMainPanel;
}

void CWinPanel::goStartPage()
{
    m_pMainPanel->goStart();
}

void CWinPanel::focus()
{
    m_pMainPanel->focus();
}

void CWinPanel::applyWindowState(Qt::WindowState state)
{
    m_pMainPanel->applyMainWindowState(state);
}

void CWinPanel::slot_windowClose()
{
    m_pManager->DestroyCefView(-1);
//    m_pManager->GetApplication()->ExitMessageLoop();
//    PostQuitMessage(0);
}

void CWinPanel::doClose()
{
    QTimer::singleShot(500, this, [=]{
        m_pMainPanel->pushButtonCloseClicked();
    });
}

void CWinPanel::slot_windowChangeState(Qt::WindowState s)
{
    int cmdShow = SW_RESTORE;
    switch (s) {
    case Qt::WindowMaximized:
        cmdShow = SW_MAXIMIZE;
        break;
    case Qt::WindowMinimized:
        cmdShow = SW_MINIMIZE;
        break;
    case Qt::WindowFullScreen:
        cmdShow = SW_HIDE;
        break;
    default:
    case Qt::WindowNoState:
        break;
    }

    ShowWindow(parentWindow(), cmdShow);
}

void CWinPanel::slot_mainPageReady()
{
    CSplash::hideSplash();

#ifdef _UPDMODULE
    QString _prod_name = WINDOW_NAME;
    qDebug() << "update's window title: " << _prod_name;

    GET_REGISTRY_USER(_user)
    if (!_user.contains("CheckForUpdates")) {
        _user.setValue("CheckForUpdates", "1");
    }

    win_sparkle_set_app_details(QString(VER_COMPANYNAME_STR).toStdWString().c_str(),
                                    _prod_name.toStdWString().c_str(),
                                    QString(VER_FILEVERSION_STR).toStdWString().c_str());
    win_sparkle_set_appcast_url(URL_APPCAST_UPDATES);
    win_sparkle_set_registry_path(QString("Software\\%1\\%2").arg(REG_GROUP_KEY).arg(REG_APP_NAME).toLatin1());
    win_sparkle_set_lang(CLangater::getLanguageName().toLatin1());
    win_sparkle_init();
#endif
}

void CWinPanel::updatePanelStylesheets()
{
    m_pMainPanel->updateStylesheets();
}
