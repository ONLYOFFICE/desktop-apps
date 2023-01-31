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

#include <windowsx.h>
#include "singleapplication.h"
#include "defines.h"
#include <QThread>

#define RECEIVER_WINDOW WINDOW_CLASS_NAME
#define RETRIES_COUNT 10
#define RETRIES_DELAY_MS 500


SingleApplication::SingleApplication(int &argc, char *argv[], const QString &instanceName) :
    QApplication(argc, argv),
    m_isPrimary(false)
{
    m_hMutex = CreateMutex(NULL, FALSE, (LPCTSTR)instanceName.data());
    if (GetLastError() != ERROR_ALREADY_EXISTS) {
        m_isPrimary = true;
        startPrimary();
    }
}

SingleApplication::~SingleApplication()
{
    if (m_isPrimary && m_hMutex != nullptr)
        CloseHandle(m_hMutex);

    if (m_hWnd)
        DestroyWindow(m_hWnd);
}

bool SingleApplication::isPrimary() const
{
    return m_isPrimary;
}

bool SingleApplication::sendMessage(const QByteArray &message)
{
    if (m_isPrimary)
        return false;   

    HWND hwnd = NULL;
    int retries = RETRIES_COUNT;
    do {
        QThread::msleep(RETRIES_DELAY_MS);
        hwnd = FindWindow(RECEIVER_WINDOW, NULL);
    } while (retries-- > 0 && hwnd == NULL);

    if (hwnd != NULL) {
        QString msg(message);
        wchar_t *cm_line = new wchar_t[msg.length() + 1];
        msg.toWCharArray(cm_line);
        cm_line[msg.length()] = '\0';

        COPYDATASTRUCT MyCDS = {1};
        MyCDS.cbData = sizeof(WCHAR) * (wcslen(cm_line) + 1);
        MyCDS.lpData = cm_line;
        SendMessage(hwnd, WM_COPYDATA, WPARAM(0), LPARAM((LPVOID)&MyCDS));
        delete[] cm_line;
        return true;
    }
    return false;
}

void SingleApplication::startPrimary()
{
    HINSTANCE hInstance = GetModuleHandle(nullptr);
    WNDCLASSEX wcx{sizeof(WNDCLASSEX)};
    wcx.style = CS_HREDRAW | CS_VREDRAW;
    wcx.hInstance = hInstance;
    wcx.lpfnWndProc = WndProc;
    wcx.cbClsExtra	= 0;
    wcx.cbWndExtra	= 0;
    wcx.lpszClassName = RECEIVER_WINDOW;
    wcx.hbrBackground = CreateSolidBrush(0x00ffffff);
    wcx.hCursor = LoadCursor(hInstance, IDC_ARROW);
    RegisterClassEx(&wcx);

    m_hWnd = CreateWindowEx(
                WS_EX_TOOLWINDOW | WS_EX_LAYERED,
                RECEIVER_WINDOW,
                L"",
                WS_MAXIMIZEBOX | WS_THICKFRAME,
                0, 0, 0, 0,
                nullptr,
                nullptr,
                hInstance,
                nullptr);
    SetWindowLongPtr(m_hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
    ShowWindow(m_hWnd, SW_HIDE);
}

LRESULT SingleApplication::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    SingleApplication *app = reinterpret_cast<SingleApplication*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    if (!app)
        return DefWindowProc(hWnd, msg, wParam, lParam);

    switch (msg) {
    case WM_NCCALCSIZE: {
        if (wParam == TRUE) {
            SetWindowLong(hWnd, DWLP_MSGRESULT, 0);
            return TRUE;
        }
        return FALSE;
    }

    case WM_NCACTIVATE:
        return TRUE;

    case WM_COPYDATA: {
        if (app->m_isPrimary) {
            COPYDATASTRUCT* pcds = (COPYDATASTRUCT*)lParam;
            if (pcds->dwData == 1) {
                WCHAR *args = (WCHAR*)pcds->lpData;
                if (args != nullptr) {
                    emit app->receivedMessage(QString::fromWCharArray(args).toUtf8());
                }
            }
        }
        break;
    }

    default:
        break;
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}
