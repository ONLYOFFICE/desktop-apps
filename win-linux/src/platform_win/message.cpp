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

#include <Windows.h>
#include <QTextDocumentFragment>
#include "message.h"


int WinMsg::showMessage(QWidget *parent,
                        const QString &msg,
                        MsgType msgType,
                        MsgBtns msgBtns)
{
    std::wstring lpCaption = QString("  %1").arg(APP_TITLE).toStdWString();
    std::wstring lpText = QTextDocumentFragment::fromHtml(msg).toPlainText().toStdWString();
    HWND parent_hwnd = (parent) ? (HWND)parent->winId() : nullptr;

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

    int msgboxID = MessageBoxW(parent_hwnd,
                               lpText.c_str(),
                               lpCaption.c_str(),
                               uType);

    int result = MODAL_RESULT_CANCEL;
    switch (msgboxID) {
    case IDYES: result = MODAL_RESULT_CUSTOM + 0; break;
    case IDNO:  result = MODAL_RESULT_CUSTOM + 1; break;
    case IDOK:
        if (msgBtns == MsgBtns::mbOk)
            result = MODAL_RESULT_YES;
        else
        if (msgBtns == MsgBtns::mbOkCancel)
            result = MODAL_RESULT_CUSTOM + 0;
        break;
    case IDCANCEL:
    default:
        break;
    }

    return result;
}
