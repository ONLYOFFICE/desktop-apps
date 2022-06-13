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

#include "windows/platform_linux/cwindowplatform.h"
#include "cascapplicationmanagerwrapper.h"
#include "defines.h"
#include "utils.h"
#include <QTimer>


#ifdef DOCUMENTSCORE_OPENSSL_SUPPORT
# include "platform_linux/cdialogopenssl.h"
#endif


CWindowPlatform::CWindowPlatform(const QRect &rect) :
    CWindowBase(rect),
    CX11Decoration(this)
{
    if (isCustomWindowStyle())
        CX11Decoration::turnOff();
    setIsCustomWindowStyle(!CX11Decoration::isDecorated());
}

CWindowPlatform::~CWindowPlatform()
{

}

/** Public **/

void CWindowPlatform::sendSertificate(int viewid)
{
#ifdef DOCUMENTSCORE_OPENSSL_SUPPORT
    CDialogOpenSsl _dialog(this);

    NSEditorApi::CAscOpenSslData * pData = new NSEditorApi::CAscOpenSslData;
    if ( _dialog.exec() == QDialog::Accepted ) {
        _dialog.getResult(*pData);
    }

    NSEditorApi::CAscMenuEvent * pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_PAGE_SELECT_OPENSSL_CERTIFICATE);
    pEvent->m_pData = pData;
    AscAppManager::getInstance().GetViewById(viewid)->Apply(pEvent);
#endif
}

void CWindowPlatform::bringToTop()
{
    QMainWindow::raise();
    QMainWindow::activateWindow();
}

void CWindowPlatform::show(bool maximized)
{
    QMainWindow::show();
    if (maximized) {
        QMainWindow::setWindowState(Qt::WindowMaximized);
    }
}

void CWindowPlatform::setWindowColors(const QColor& background, const QColor& border)
{
    Q_UNUSED(border)
    if (!CX11Decoration::isDecorated()) {
        CWindowBase::setWindowColors(background, border);
    }
}

void CWindowPlatform::adjustGeometry()
{
    if (!isMaximized()) {
        const int border = int(CX11Decoration::customWindowBorderWith() * m_dpiRatio);
        setContentsMargins(border, border, border, border);
    } else {
        setContentsMargins(0, 0, 0, 0);
    }
}

/** Protected **/

bool CWindowPlatform::event(QEvent * event)
{
    static bool _flg_motion = false;
    static bool _flg_left_button = false;
    if (event->type() == QEvent::WindowStateChange) {
        CX11Decoration::setMaximized(isMaximized() ? true : false);
        /*if(windowState().testFlag(Qt::WindowMaximized)) {
            applyWindowState(Qt::WindowMaximized);
        } else
        if (windowState().testFlag(Qt::WindowNoState)) {
            applyWindowState(Qt::WindowNoState);
        } else
        if (windowState().testFlag(Qt::WindowMinimized)) {
            applyWindowState(Qt::WindowMinimized);
        }*/
        applyWindowState();
        adjustGeometry();
    } else
    if ( event->type() == QEvent::MouseButtonPress ) {
        _flg_left_button = static_cast<QMouseEvent *>(event)->buttons().testFlag(Qt::LeftButton);
    } else
    if ( event->type() == QEvent::MouseButtonRelease ) {
        if ( _flg_left_button && _flg_motion ) {
            updateScaling();
        }
        _flg_left_button = _flg_motion = false;
    } else
    if ( event->type() == QEvent::Move ) {
        if (!_flg_motion)
            _flg_motion = true;
    }
    return CWindowBase::event(event);
}

void CWindowPlatform::setScreenScalingFactor(double factor)
{
    CX11Decoration::onDpiChanged(factor);
    CWindowBase::setScreenScalingFactor(factor);
}

/** Private **/

void CWindowPlatform::mouseMoveEvent(QMouseEvent *e)
{
    CX11Decoration::dispatchMouseMove(e);
}

void CWindowPlatform::mousePressEvent(QMouseEvent *e)
{
    CX11Decoration::dispatchMouseDown(e);
}

void CWindowPlatform::mouseReleaseEvent(QMouseEvent *e)
{
    CX11Decoration::dispatchMouseUp(e);
}

void CWindowPlatform::mouseDoubleClickEvent(QMouseEvent *)
{
    if (m_boxTitleBtns) {
        if (m_boxTitleBtns->underMouse())
            onMaximizeEvent();
    }
}
