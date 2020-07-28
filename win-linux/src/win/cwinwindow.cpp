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

#include "cwinwindow.h"
#include "utils.h"
#include <stdexcept>
#include <QDebug>

LRESULT CALLBACK wndproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_CHAR:
        if (wParam == VK_ESCAPE) {
            PostMessage(hWnd, WM_CLOSE, 0, 0);
            return 0;
        }

        break;
    case WM_SIZE:
        {
            CWinWindow * window = reinterpret_cast<CWinWindow *>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
            if ( window ) {
                window->onScreenScaling();
            }
        }
        break;
    case WM_CLOSE: {
        HWND pwnd = GetWindow(hWnd, GW_OWNER);
        if (pwnd) {
            EnableWindow(pwnd, TRUE);
            SetFocus(pwnd);
        }

        DestroyWindow(hWnd);
        return 0; }
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        break;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

CWinWindow::CWinWindow(HWND parent, const QString& title)
    : m_hSelf(0)
    , m_hParent(parent)
{
    HINSTANCE hInstance = GetModuleHandle(NULL);

    WNDCLASSEXW wcx{0};

    wcx.cbSize          = sizeof( WNDCLASSEX );
    wcx.style           = CS_HREDRAW | CS_VREDRAW;
    wcx.hInstance       = hInstance;
    wcx.lpfnWndProc     = wndproc;
    wcx.cbClsExtra      = 0;
    wcx.cbWndExtra      = 0;
    wcx.lpszClassName   = L"WindowClass";
    wcx.hbrBackground   = (HBRUSH)(COLOR_WINDOW+1);
    wcx.hCursor         = LoadCursor( hInstance, IDC_ARROW );
    wcx.hIcon           = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPLICATION));

    if ( FAILED(RegisterClassExW(&wcx)) )
        throw std::runtime_error( "Couldn't register window class" );

    int _title_height = GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYFIXEDFRAME) * 2,
        _border_width = GetSystemMetrics(SM_CYFIXEDFRAME) * 2;
    m_hSelf = CreateWindow(wcx.lpszClassName, title.toStdWString().c_str(),
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        CW_USEDEFAULT, CW_USEDEFAULT, 300 + _border_width, 200 + _title_height,
        parent, NULL, hInstance, NULL);

    HMENU hmenu = GetSystemMenu(m_hSelf, false);
    DeleteMenu(hmenu, SC_SIZE, MF_BYCOMMAND);
    DeleteMenu(hmenu, SC_MAXIMIZE, MF_BYCOMMAND);
    DeleteMenu(hmenu, SC_MINIMIZE, MF_BYCOMMAND);
    DeleteMenu(hmenu, SC_RESTORE, MF_BYCOMMAND);

    SetWindowLong(m_hSelf, GWL_EXSTYLE, GetWindowLong(m_hSelf, GWL_EXSTYLE) | WS_EX_DLGMODALFRAME);
    SetWindowLongPtr(m_hSelf, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

    if ( !m_hSelf )
        throw std::runtime_error("Couldn't register window class");
}

void CWinWindow::modal()
{
    EnableWindow(m_hParent, FALSE);

    ShowWindow(m_hSelf, SW_SHOW);
    UpdateWindow(m_hSelf);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

//    EnableWindow(m_hParent, TRUE);
//    SetActiveWindow(m_hParent);
}

void CWinWindow::close()
{
    PostMessage(m_hSelf, WM_CLOSE, 0, 0);
}

void CWinWindow::setSize(int w, int h)
{
    RECT wrc, crc;
    GetWindowRect(m_hSelf, &wrc);
    GetClientRect(m_hSelf, &crc);

    int _title_height = wrc.bottom - wrc.top - crc.bottom + crc.top,
        _border_width = wrc.right - wrc.left - crc.right + crc.left;

    SetWindowPos(m_hSelf, NULL, 0, 0, w + _border_width, h + _title_height, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
}

void CWinWindow::center()
{
    RECT rc1, rc2;
    GetWindowRect(m_hParent, &rc1);
    GetWindowRect(m_hSelf, &rc2);

    QPoint center = Utils::getScreenGeometry(QPoint((rc1.left + rc1.right)/2, (rc1.top + rc1.bottom)/2)).center();

    int x = center.x() - (rc2.right - rc2.left) /2;
    int y = center.y() - (rc2.bottom - rc2.top)/2;

    SetWindowPos(m_hSelf, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

void CWinWindow::onScreenScaling()
{

}

HWND CWinWindow::handle()
{
    return m_hSelf;
}
