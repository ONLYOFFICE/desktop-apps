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

#include <QTextDocumentFragment>
#include "updatedialog.h"
#include "platform_win/resource.h"
#include "defines.h"
#include <string.h>
#include <Windows.h>
#include <CommCtrl.h>
#include <QTimer>

#define toWCharPtr(qstr) _wcsdup(qstr.toStdWString().c_str())
#define TEXT_SKIP        toWCharPtr(QObject::tr("Skip this version"))
#define TEXT_REMIND      toWCharPtr(QObject::tr("Remind me later"))
#define TEXT_INSTALL     toWCharPtr(QObject::tr("Install update"))
#define TEXT_INSLATER    toWCharPtr(QObject::tr("Install on Next Start"))
#define TEXT_RESTART     toWCharPtr(QObject::tr("Save and Restart Now"))
#define TEXT_SAVEANDINS  toWCharPtr(QObject::tr("Save and Install Now"))
#define TEXT_DOWNLOAD    toWCharPtr(QObject::tr("Download update"))


static void BringToTop(HWND hwnd)
{
    DWORD appID = ::GetCurrentThreadId();
    DWORD frgID = ::GetWindowThreadProcessId(::GetForegroundWindow(), NULL);
    ::AttachThreadInput(frgID, appID, TRUE);
    ::SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
    ::SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE);
    ::SetForegroundWindow(hwnd);
    ::SetFocus(hwnd);
    ::SetActiveWindow(hwnd);
    ::AttachThreadInput(frgID, appID, FALSE);
}

static HRESULT Pftaskdialogcallback(HWND hwnd, UINT msg, WPARAM wParam,
                                    LPARAM lParam, LONG_PTR lpRefData)
{
    switch (msg) {
    case TDN_HYPERLINK_CLICKED:
        ShellExecute(NULL, L"open", (PCWSTR)lParam, NULL, NULL, SW_SHOWNORMAL);
        break;
    case TDN_DIALOG_CONSTRUCTED: {
        QTimer::singleShot(0, [=]() {
            if (hwnd)
                BringToTop(hwnd);
        });
        break;
    }
    default:
        break;
    }
    return S_OK;
}

int WinDlg::showDialog(QWidget *parent,
                       const QString &msg,
                       const QString &content,
                       DlgBtns dlgBtns)
{
    std::wstring lpCaption = QString("  %1").arg(QObject::tr("Software Update")).toStdWString();
    std::wstring lpText = QTextDocumentFragment::fromHtml(msg).toPlainText().toStdWString();
    QString linkText = !QString(RELEASE_NOTES).isEmpty() ?
                QString("\n<A HREF=\"%1\">%2</A>").arg(QString(RELEASE_NOTES), QObject::tr("Release notes")) : "";
    std::wstring lpContent = QString("%1\n%2").arg(content, linkText).toStdWString();
    HWND parent_hwnd = (parent) ? (HWND)parent->winId() : NULL;

    int msgboxID = 0;
    PCWSTR pIcon = MAKEINTRESOURCE(IDI_MAINICON);
    TASKDIALOG_BUTTON *pButtons = NULL;
    uint cButtons = 0;
    switch (dlgBtns) {
    case DlgBtns::mbInslaterRestart:
        cButtons = 2;
        pButtons = new TASKDIALOG_BUTTON[cButtons];
        pButtons[0] = {IDYES, TEXT_INSLATER};
        pButtons[1] = {IDNO,  TEXT_RESTART};
        break;
    case DlgBtns::mbSkipRemindInstall:
        cButtons = 3;
        pButtons = new TASKDIALOG_BUTTON[cButtons];
        pButtons[0] = {IDRETRY, TEXT_SKIP};
        pButtons[1] = {IDNO,  TEXT_REMIND};
        pButtons[2] = {IDYES, TEXT_INSTALL};
        break;
    case DlgBtns::mbSkipRemindSaveandinstall:
        cButtons = 3;
        pButtons = new TASKDIALOG_BUTTON[cButtons];
        pButtons[0] = {IDRETRY, TEXT_SKIP};
        pButtons[1] = {IDNO,  TEXT_REMIND};
        pButtons[2] = {IDYES, TEXT_SAVEANDINS};
        break;
    case DlgBtns::mbSkipRemindDownload:
        cButtons = 3;
        pButtons = new TASKDIALOG_BUTTON[cButtons];
        pButtons[0] = {IDRETRY, TEXT_SKIP};
        pButtons[1] = {IDNO,  TEXT_REMIND};
        pButtons[2] = {IDYES, TEXT_DOWNLOAD};
        break;
    default:
        break;
    }

    int nDefltBtn{0};
    switch (dlgBtns) {
    case DlgBtns::mbInslaterRestart:   nDefltBtn = IDNO; break;
    case DlgBtns::mbSkipRemindInstall: nDefltBtn = IDYES; break;
    case DlgBtns::mbSkipRemindSaveandinstall: nDefltBtn = IDYES; break;
    case DlgBtns::mbSkipRemindDownload: nDefltBtn = IDYES; break;
    default: break;
    }

    TASKDIALOGCONFIG config = {0};
    ZeroMemory(&config, sizeof(config));
    config.cbSize             = sizeof(config);
    config.dwFlags            = TDF_ENABLE_HYPERLINKS |
                                TDF_POSITION_RELATIVE_TO_WINDOW |
                                TDF_ALLOW_DIALOG_CANCELLATION;
    config.hwndParent         = parent_hwnd;
    config.hInstance          = GetModuleHandle(NULL);
    config.pfCallback         = (PFTASKDIALOGCALLBACK)Pftaskdialogcallback;
    config.pButtons           = pButtons;
    config.cButtons           = cButtons;
    config.nDefaultButton     = nDefltBtn;
    config.pszMainIcon        = pIcon;
    config.pszWindowTitle     = lpCaption.c_str();
    config.pszMainInstruction = lpText.c_str();
    config.pszContent         = lpContent.c_str();
//    config.cxWidth            = 240;

    TaskDialogIndirect(&config, &msgboxID, NULL, NULL);
    for (int i = 0; i < (int)cButtons; i++)
        free((void*)pButtons[i].pszButtonText);
    delete[] pButtons;

    int result = -1;
    switch (msgboxID) {
    case IDYES: result = (dlgBtns == DlgBtns::mbSkipRemindInstall ||
                          dlgBtns == DlgBtns::mbSkipRemindSaveandinstall) ? DLG_RESULT_INSTALL :
                         (dlgBtns == DlgBtns::mbSkipRemindDownload) ? DLG_RESULT_DOWNLOAD :
                                                                      DLG_RESULT_INSLATER; break;
    case IDNO:  result = (dlgBtns == DlgBtns::mbSkipRemindInstall ||
                          dlgBtns == DlgBtns::mbSkipRemindSaveandinstall ||
                          dlgBtns == DlgBtns::mbSkipRemindDownload) ? DLG_RESULT_REMIND :
                                                                      DLG_RESULT_RESTART; break;
    case IDRETRY: result = DLG_RESULT_SKIP; break;
    case IDCANCEL:
    default:
        break;
    }

    return result;
}
