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
#include <Windows.h>
#ifndef __OS_WIN_XP
# include <commctrl.h>
#endif


int WinMsg::showMessage(QWidget *parent,
                        const QString &msg,
                        MsgType msgType,
                        MsgBtns msgBtns)
{
    std::wstring lpCaption = QString("  %1").arg(APP_TITLE).toStdWString();
    std::wstring lpText = QTextDocumentFragment::fromHtml(msg).toPlainText().toStdWString();
    HWND parent_hwnd = (parent) ? (HWND)parent->winId() : nullptr;

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

    TASKDIALOG_COMMON_BUTTON_FLAGS dwFlags{0};
    switch (msgBtns) {
    case MsgBtns::mbYesNo:          dwFlags |= TDCBF_YES_BUTTON | TDCBF_NO_BUTTON; break;
    case MsgBtns::mbYesDefNo:       dwFlags |= TDCBF_YES_BUTTON | TDCBF_NO_BUTTON; break;
    case MsgBtns::mbYesNoCancel:    dwFlags |= TDCBF_YES_BUTTON | TDCBF_NO_BUTTON | TDCBF_CANCEL_BUTTON; break;
    case MsgBtns::mbYesDefNoCancel: dwFlags |= TDCBF_YES_BUTTON | TDCBF_NO_BUTTON | TDCBF_CANCEL_BUTTON; break;
    case MsgBtns::mbOkCancel:       dwFlags |= TDCBF_OK_BUTTON | TDCBF_CANCEL_BUTTON; break;
    case MsgBtns::mbOkDefCancel:    dwFlags |= TDCBF_OK_BUTTON | TDCBF_CANCEL_BUTTON; break;
    default:                        dwFlags |= TDCBF_OK_BUTTON; break;
    }

    int nDefltBtn{0};
    switch (msgBtns) {
    case MsgBtns::mbYesNo:          nDefltBtn = IDNO; break;
    case MsgBtns::mbYesDefNo:       nDefltBtn = IDYES; break;
    case MsgBtns::mbYesNoCancel:    nDefltBtn = IDCANCEL; break;
    case MsgBtns::mbYesDefNoCancel: nDefltBtn = IDYES; break;
    case MsgBtns::mbOkCancel:       nDefltBtn = IDCANCEL; break;
    case MsgBtns::mbOkDefCancel:    nDefltBtn = IDOK; break;
    default:                        nDefltBtn = IDOK; break;
    }

    TASKDIALOGCONFIG config = {0};
    ZeroMemory(&config, sizeof(config));
    config.cbSize             = sizeof(config);
    config.hwndParent         = parent_hwnd;
    config.hInstance          = NULL;
    config.dwCommonButtons    = dwFlags;
    config.nDefaultButton     = nDefltBtn;
    config.pszMainIcon        = pIcon;
    config.pszWindowTitle     = lpCaption.c_str();
    config.pszMainInstruction = lpText.c_str();
    config.pszContent         = NULL;

    TaskDialogIndirect(&config, &msgboxID, NULL, NULL);
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
    case IDYES: result = MODAL_RESULT_YES; break;
    case IDNO:  result = MODAL_RESULT_NO; break;
    case IDOK:  result = MODAL_RESULT_OK; break;
    case IDCANCEL:
    default:
        break;
    }

    return result;
}
