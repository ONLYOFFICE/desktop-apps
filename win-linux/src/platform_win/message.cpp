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
#include "message.h"
#include "utils.h"
#include <string.h>
#include <Windows.h>
#include <QTimer>
#ifndef __OS_WIN_XP
# include <commctrl.h>
#endif

#define toWCharPtr(qstr) _wcsdup(qstr.toStdWString().c_str())
#define TEXT_CANCEL toWCharPtr(BTN_TEXT_CANCEL)
#define TEXT_YES    toWCharPtr(BTN_TEXT_YES)
#define TEXT_NO     toWCharPtr(BTN_TEXT_NO)
#define TEXT_OK     toWCharPtr(BTN_TEXT_OK)
#define TEXT_SKIP   toWCharPtr(BTN_TEXT_SKIP)
#define TEXT_BUY    toWCharPtr(BTN_TEXT_BUY)
#define TEXT_ACTIVATE   toWCharPtr(BTN_TEXT_ACTIVATE)
#define TEXT_CONTINUE   toWCharPtr(BTN_TEXT_CONTINUE)


#ifndef __OS_WIN_XP
static HRESULT CALLBACK Pftaskdialogcallback(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, LONG_PTR lpRefData)
{
    switch (msg) {
    case TDN_DIALOG_CONSTRUCTED: {
        QTimer::singleShot(0, [=]() {
            if (hwnd)
                WindowHelper::bringToTop(hwnd);
        });
        break;
    }
    default:
        break;
    }
    return S_OK;
}
#endif

int WinMsg::showMessage(QWidget *parent,
                        const QString &msg,
                        MsgType msgType,
                        MsgBtns msgBtns,
                        bool  *checkBoxState,
                        const QString &chekBoxText)
{
    std::wstring lpCaption = QString("  %1").arg(WINDOW_TITLE).toStdWString();
    std::wstring lpText = QTextDocumentFragment::fromHtml(msg).toPlainText().toStdWString();
    std::wstring lpCheckBoxText = chekBoxText.toStdWString();
    HWND parent_hwnd = (parent) ? (HWND)parent->winId() : nullptr;
    if (parent_hwnd && IsIconic(parent_hwnd))
        ShowWindow(parent_hwnd, SW_RESTORE);

    int msgboxID = 0;
#ifndef __OS_WIN_XP
    PCWSTR pIcon = NULL;
    switch (msgType) {
    case MsgType::MSG_INFO:    pIcon = TD_INFORMATION_ICON; break;
    case MsgType::MSG_WARN:    pIcon = TD_WARNING_ICON; break;
    case MsgType::MSG_CONFIRM: pIcon = TD_SHIELD_ICON; break;
    case MsgType::MSG_ERROR:   pIcon = TD_ERROR_ICON; break;
    default:                   pIcon = TD_INFORMATION_ICON; break;
    }

    TASKDIALOG_BUTTON *pButtons;
    uint cButtons = 0;
    switch (msgBtns) {
    case MsgBtns::mbYesNo:
    case MsgBtns::mbYesDefNo:
        cButtons = 2;
        pButtons = new TASKDIALOG_BUTTON[cButtons];
        pButtons[0] = {IDYES, TEXT_YES};
        pButtons[1] = {IDNO,  TEXT_NO};
        break;
    case MsgBtns::mbYesNoCancel:
    case MsgBtns::mbYesDefNoCancel:
        cButtons = 3;
        pButtons = new TASKDIALOG_BUTTON[cButtons];
        pButtons[0] = {IDYES, TEXT_YES};
        pButtons[1] = {IDNO,  TEXT_NO};
        pButtons[2] = {IDCANCEL, TEXT_CANCEL};
        break;
    case MsgBtns::mbOkCancel:
    case MsgBtns::mbOkDefCancel:
        cButtons = 2;
        pButtons = new TASKDIALOG_BUTTON[cButtons];
        pButtons[0] = {IDOK, TEXT_OK};
        pButtons[1] = {IDCANCEL, TEXT_CANCEL};
        break;
    case MsgBtns::mbYesDefSkipNo:
        cButtons = 3;
        pButtons = new TASKDIALOG_BUTTON[cButtons];
        pButtons[0] = {IDYES, TEXT_YES};
        pButtons[1] = {IDRETRY, TEXT_SKIP};
        pButtons[2] = {IDNO, TEXT_NO};
        break;
    case MsgBtns::mbBuy:
        cButtons = 1;
        pButtons = new TASKDIALOG_BUTTON[cButtons];
        pButtons[0] = {IDYES, TEXT_BUY};
        break;
    case MsgBtns::mbActivateDefContinue:
        cButtons = 2;
        pButtons = new TASKDIALOG_BUTTON[cButtons];
        pButtons[0] = {IDYES, TEXT_ACTIVATE};
        pButtons[1] = {IDNO, TEXT_CONTINUE};
        break;
    case MsgBtns::mbContinue:
        cButtons = 1;
        pButtons = new TASKDIALOG_BUTTON[cButtons];
        pButtons[0] = {IDOK, TEXT_CONTINUE};
        break;
    default:
        cButtons = 1;
        pButtons = new TASKDIALOG_BUTTON[cButtons];
        pButtons[0] = {IDOK, TEXT_OK};
        break;
    }

    int nDefltBtn{0};
    switch (msgBtns) {
    case MsgBtns::mbYesNo:          nDefltBtn = IDNO; break;
    case MsgBtns::mbYesDefNo:       nDefltBtn = IDYES; break;
    case MsgBtns::mbYesNoCancel:    nDefltBtn = IDCANCEL; break;
    case MsgBtns::mbYesDefNoCancel: nDefltBtn = IDYES; break;
    case MsgBtns::mbOkCancel:       nDefltBtn = IDCANCEL; break;
    case MsgBtns::mbOkDefCancel:    nDefltBtn = IDOK; break;
    case MsgBtns::mbYesDefSkipNo:   nDefltBtn = IDYES; break;
    case MsgBtns::mbBuy:            nDefltBtn = IDYES; break;
    case MsgBtns::mbActivateDefContinue:   nDefltBtn = IDYES; break;
    case MsgBtns::mbContinue:       nDefltBtn = IDOK; break;
    default:                        nDefltBtn = IDOK; break;
    }

    BOOL chkState = (checkBoxState) ? (BOOL)*checkBoxState : FALSE;

    TASKDIALOGCONFIG config = {0};
    ZeroMemory(&config, sizeof(config));
    config.cbSize             = sizeof(config);
    config.dwFlags            = TDF_POSITION_RELATIVE_TO_WINDOW |
                                TDF_ALLOW_DIALOG_CANCELLATION |
                                TDF_SIZE_TO_CONTENT;
    config.hwndParent         = parent_hwnd;
    config.hInstance          = GetModuleHandle(NULL);
    config.pfCallback         = (PFTASKDIALOGCALLBACK)Pftaskdialogcallback;
    config.pButtons           = pButtons;
    config.cButtons           = cButtons;
    config.nDefaultButton     = nDefltBtn;
    config.pszMainIcon        = pIcon;
    config.pszWindowTitle     = lpCaption.c_str();
    config.pszMainInstruction = lpText.c_str();
    config.pszContent         = NULL;

    if (chkState == TRUE)
        config.dwFlags |= TDF_VERIFICATION_FLAG_CHECKED;
    if (checkBoxState)
        config.pszVerificationText = lpCheckBoxText.c_str();

    TaskDialogIndirect(&config, &msgboxID, NULL, (checkBoxState != nullptr) ? &chkState : NULL);
    if (checkBoxState != nullptr)
        *checkBoxState = (chkState == TRUE);

    for (int i = 0; i < (int)cButtons; i++)
        free((void*)pButtons[i].pszButtonText);
    delete[] pButtons;
#else
    DWORD uType{0};
    switch (msgType) {
    case MsgType::MSG_INFO:    uType |= MB_ICONINFORMATION; break;
    case MsgType::MSG_WARN:    uType |= MB_ICONWARNING; break;
    case MsgType::MSG_CONFIRM: uType |= MB_ICONQUESTION; break;
    case MsgType::MSG_ERROR:   uType |= MB_ICONERROR; break;
    default:                   uType |= MB_ICONINFORMATION; break;
    }

    switch (msgBtns) {
    case MsgBtns::mbYesNo:          uType |= MB_YESNO | MB_DEFBUTTON2; break;
    case MsgBtns::mbYesDefNo:       uType |= MB_YESNO | MB_DEFBUTTON1; break;
    case MsgBtns::mbYesNoCancel:    uType |= MB_YESNOCANCEL | MB_DEFBUTTON3; break;
    case MsgBtns::mbYesDefNoCancel: uType |= MB_YESNOCANCEL | MB_DEFBUTTON1; break;
    case MsgBtns::mbOkCancel:       uType |= MB_OKCANCEL | MB_DEFBUTTON2; break;
    case MsgBtns::mbOkDefCancel:    uType |= MB_OKCANCEL | MB_DEFBUTTON1; break;
    default:                        uType |= MB_OK | MB_DEFBUTTON1; break;
    }

    msgboxID = MessageBoxW(parent_hwnd,
                           lpText.c_str(),
                           lpCaption.c_str(),
                           uType);
#endif

    int result = MODAL_RESULT_CANCEL;
    switch (msgboxID) {
    case IDYES: result = (msgBtns == MsgBtns::mbBuy) ? MODAL_RESULT_BUY :
                         (msgBtns == MsgBtns::mbActivateDefContinue) ? MODAL_RESULT_ACTIVATE :
                                                                       MODAL_RESULT_YES; break;
    case IDNO:  result = (msgBtns == MsgBtns::mbActivateDefContinue) ? MODAL_RESULT_CONTINUE :
                                                                       MODAL_RESULT_NO; break;
    case IDOK:  result = (msgBtns == MsgBtns::mbContinue) ? MODAL_RESULT_CONTINUE :
                                                            MODAL_RESULT_OK; break;
    case IDRETRY:  result = MODAL_RESULT_SKIP; break;
    case IDCANCEL:
    default:
        break;
    }

    return result;
}
