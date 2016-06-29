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

#include "cmainwindow.h"
#include "../defines.h"
#include "../qascmainpanel.h"
#include "../cascapplicationmanagerwrapper.h"
#include <QProxyStyle>
#include <QApplication>
#include <QFileInfo>
#include "../utils.h"
#include "../clicensekeeper.h"
#include <QTimer>

CMainWindow::CMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , CX11Decoration(this)
{
    resize(1200, 700);
}

CMainWindow::CMainWindow(CAscApplicationManager * pAppManager)
    : CMainWindow((QWidget *)0) /* doesn't compile via gcc 4.8 without parameter */
{
    parseInputArgs(qApp->arguments());

    setWindowIcon(Utils::appIcon());
    setObjectName("MainWindow");

    GET_REGISTRY_SYSTEM(reg_system)
    GET_REGISTRY_USER(reg_user)
    if (reg_user.value("titlebar") == "custom" ||
            reg_system.value("titlebar") == "custom" )
        CX11Decoration::turnOff();

    m_pMainPanel = new QAscMainPanel(this, pAppManager, !CX11Decoration::isDecorated());
//    m_pMainPanel = new QAscMainPanel(this, pAppManager, true);
    setCentralWidget(m_pMainPanel);

    if (CLicensekeeper::tempLicenseExist())
            CLicensekeeper::selfActivation(LICENSE_TYPE_FREE);

    if ( !CX11Decoration::isDecorated() ) {
        CX11Decoration::setTitleWidget(((QAscMainPanel *)m_pMainPanel)->getTitleWidget());
        ((QAscMainPanel *)m_pMainPanel)->setMouseTracking(true);
        setMouseTracking(true);
    }

    ((CAscApplicationManagerWrapper *)pAppManager)->setMainPanel((QAscMainPanel *)m_pMainPanel);

    restoreGeometry(reg_user.value("position").toByteArray());
    restoreState(reg_user.value("windowstate").toByteArray());

    QMetaObject::connectSlotsByName(this);

    pAppManager->StartSpellChecker();
    pAppManager->StartKeyboardChecker();

    QAscMainPanel * pMainPanel = qobject_cast<QAscMainPanel *>(m_pMainPanel);
    connect(pMainPanel, &QAscMainPanel::mainWindowChangeState, this, &CMainWindow::slot_windowChangeState);
    connect(pMainPanel, &QAscMainPanel::mainWindowClose, this, &CMainWindow::slot_windowClose);
}

void CMainWindow::parseInputArgs(const QStringList& inlist)
{
    GET_REGISTRY_USER(reg_user)

    QString _arg;
    QStringListIterator i(inlist); i.next();
    while (i.hasNext()) {
        _arg = i.next();

        if (_arg.contains(QRegExp(reCmdKeepLang))) {
            reg_user.setValue("locale", _arg.right(2));
        } else
        if (_arg.contains("--system-title-bar")) {
            reg_user.setValue("titlebar", "system");
        } else
        if (_arg.contains("--custom-title-bar")) {
            reg_user.setValue("titlebar", "custom");
        }
    }

    if (!reg_user.contains("titlebar"))
        reg_user.setValue("titlebar", "custom");
}

void CMainWindow::closeEvent(QCloseEvent * e)
{
    ((QAscMainPanel *)m_pMainPanel)->pushButtonCloseClicked();
    e->ignore();
}

void CMainWindow::showEvent(QShowEvent * e)
{
    Q_UNUSED(e)

//    qDebug() << "SHOW EVENT: " << e->type();
}

bool CMainWindow::event(QEvent * event)
{
    if (event->type() == QEvent::WindowStateChange) {
        QWindowStateChangeEvent * _e_statechange = static_cast< QWindowStateChangeEvent* >( event );

        CX11Decoration::setMaximized(this->windowState() == Qt::WindowMaximized ? true : false);

//        if( _e_statechange->oldState() & Qt::WindowMinimized ) {
//            qDebug() << "Window restored (to normal or maximized state)!";
//        } else
        if( _e_statechange->oldState() == Qt::WindowNoState && this->windowState() == Qt::WindowMaximized ) {
            ((QAscMainPanel *)m_pMainPanel)->applyMainWindowState(Qt::WindowMaximized);
        } else
        if (/*_e_statechange->oldState() == Qt::WindowMaximized &*/ this->windowState() == Qt::WindowNoState) {
            ((QAscMainPanel *)m_pMainPanel)->applyMainWindowState(Qt::WindowNoState);
        }
    }

    return QMainWindow::event(event);
}

void CMainWindow::mouseMoveEvent(QMouseEvent *e)
{
    CX11Decoration::dispatchMouseMove(e);
}
void CMainWindow::mousePressEvent(QMouseEvent *e)
{
    CX11Decoration::dispatchMouseDown(e);
}
void CMainWindow::mouseReleaseEvent(QMouseEvent *e)
{
    CX11Decoration::dispatchMouseUp(e);
}

void CMainWindow::slot_windowChangeState(Qt::WindowState s)
{
    if (s == Qt::WindowFullScreen) {
        GET_REGISTRY_USER(reg_user)
        reg_user.setValue("position", saveGeometry());
        reg_user.setValue("windowstate", saveState());

        showFullScreen();
    } else {
        setWindowState(s);

        switch (s) {
        case Qt::WindowMaximized:
        case Qt::WindowMinimized:
            break;
        default:
        case Qt::WindowNoState:
            activateWindow();
            break;
        }
    }
}

void CMainWindow::slot_windowClose()
{
    if (windowState() != Qt::WindowFullScreen) {
        GET_REGISTRY_USER(reg_user)
        reg_user.setValue("position", saveGeometry());
        reg_user.setValue("windowstate", saveState());
    }

    ((QAscMainPanel *)m_pMainPanel)->getAscApplicationManager()->DestroyCefView(-1);

//    close();
}
