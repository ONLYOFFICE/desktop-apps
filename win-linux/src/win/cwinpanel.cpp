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

#include <windows.h>
#include "cwinpanel.h"
#include "mainwindow.h"

CWinPanel::CWinPanel(CMainWindow * w)
    : CWinPanel(w->hWnd)
{
    m_parent = w;
}

CWinPanel::CWinPanel( HWND hWnd )
    : QWinWidget( hWnd )
    , m_pPanel(nullptr)
{
    windowHandle = hWnd;
    setProperty("handleTopWindow", (int)hWnd);

//    setObjectName("mainPanel");

    /*
    CAscLocalOpenFiles * pData = (CAscLocalOpenFiles *)data;
    vector<std::wstring>& vctFiles = pData->get_Files();

    for (vector<wstring>::iterator i = vctFiles.begin(); i != vctFiles.end(); i++) {
        COpenOptions opts = {(*i), etLocalFile};
        m_lastOpenPath = QFileInfo(opts.url).absoluteDir().absolutePath();
        openLocalFile(opts);
    }

    */
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
    if ( event->type() == QEvent::MouseButtonDblClick ) {
        if (event -> button() == Qt::LeftButton) {
            WINDOWPLACEMENT wp;
            wp.length = sizeof( WINDOWPLACEMENT );

            GetWindowPlacement( parentWindow(), &wp );
            ShowWindow( parentWindow(), wp.showCmd == SW_MAXIMIZE ? SW_RESTORE : SW_MAXIMIZE );
        }
    } else
    if ( event->button() == Qt::LeftButton ) {
        ReleaseCapture();
        SendMessage( windowHandle, WM_NCLBUTTONDOWN, HTCAPTION, 0 );
    }

}

/*void CWinPanel::resizeEvent(QResizeEvent* event)
{
    QWinWidget::resizeEvent(event);

    if ( !m_pPanel && !children().isEmpty() ) {
        m_pPanel = findChild<QWidget *>();
    }

    if ( m_pPanel ) m_pPanel->setGeometry(QRect(0, 0, event->size().width(), event->size().height()));
}*/

CMainWindow * CWinPanel::parent()
{
    return m_parent;
}
