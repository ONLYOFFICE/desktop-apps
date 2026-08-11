/*
 * Copyright (C) Ascensio System SIA, 2009-2026
 *
 * This program is a free software product. You can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License (AGPL)
 * version 3 as published by the Free Software Foundation, together with the
 * additional terms provided in the LICENSE file.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. For
 * details, see the GNU AGPL at: https://www.gnu.org/licenses/agpl-3.0.html
 *
 * You can contact Ascensio System SIA by email at info@onlyoffice.com
 * or by postal mail at 20A-6 Ernesta Birznieka-Upisha Street, Riga,
 * LV-1050, Latvia, European Union.
 *
 * The interactive user interfaces in modified versions of the Program
 * are required to display Appropriate Legal Notices in accordance with
 * Section 5 of the GNU AGPL version 3.
 *
 * No trademark rights are granted under this License.
 *
 * All non-code elements of the Product, including illustrations,
 * icon sets, and technical writing content, are licensed under the
 * Creative Commons Attribution-ShareAlike 4.0 International License:
 * https://creativecommons.org/licenses/by-sa/4.0/legalcode
 *
 * This license applies only to such non-code elements and does not
 * modify or replace the licensing terms applicable to the Program's
 * source code, which remains licensed under the GNU Affero General
 * Public License v3.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#ifndef CMESSAGE_WIN32_H
#define CMESSAGE_WIN32_H

#include "components/cmessage.h"
#include "cascapplicationmanagerwrapper.h"
#include "platform_win/resource.h"
#include "platform_win/taskdialogtheme.h"
#include "defines.h"
#include "utils.h"
#include <QTextDocumentFragment>
#include <QWidget>
#include <vector>
#include <string.h>
#include <Windows.h>
#include <commctrl.h>

#define DLG_PADDING 7
#define BTN_SPACING 5
#define BTN_PADDING 13
#define DLG_PREF_WIDTH 240

#define CUSTOM_BUTTON_BASE_ID 100

namespace
{
struct TaskDialogContext
{
    TASKDIALOGCONFIG *pConfig;
    HICON hIcon;
    TaskDialogTheme::Theme theme;
};

HRESULT CALLBACK Pftaskdialogcallback(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, LONG_PTR lpRefData)
{
    Q_UNUSED(wParam)
    TaskDialogContext *pCtx = (TaskDialogContext*)lpRefData;

    switch (msg)
    {
    case TDN_CREATED:
    {
        if (pCtx->theme != TaskDialogTheme::Theme::Light)
            TaskDialogTheme::AllowForTaskDialog(hwnd, pCtx->pConfig, pCtx->theme);

        SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)pCtx->hIcon);
        SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)pCtx->hIcon);
        break;
    }
    case TDN_NAVIGATED:
        if (pCtx->theme != TaskDialogTheme::Theme::Light)
            TaskDialogTheme::AllowForTaskDialog(hwnd, (TASKDIALOGCONFIG*)lParam, pCtx->theme);
        break;
    case TDN_DESTROYED:
        if (pCtx->theme != TaskDialogTheme::Theme::Light)
            TaskDialogTheme::RemoveFromTaskDialog(hwnd);
        break;
    case TDN_HYPERLINK_CLICKED:
        ShellExecute(NULL, L"open", (PCWSTR)lParam, NULL, NULL, SW_SHOWNORMAL);
        break;
    default:
        break;
    }
    return S_OK;
}

int calcApproxMinWidth(const TASKDIALOG_BUTTON *pButtons, uint cButtons)
{
    int width = 0;
    HDC hdc = GetDC(NULL);
    long units = GetDialogBaseUnits();
    HGDIOBJ hFont = GetStockObject(DEFAULT_GUI_FONT);
    SelectObject(hdc, hFont);
    for (uint i = 0; i < cButtons; i++)
    {
        SIZE textSize = {0,0};
        const wchar_t *text = pButtons[i].pszButtonText;
        GetTextExtentPoint32(hdc, text, (int)std::wcslen(text), &textSize);
        width += MulDiv(textSize.cx, 4, LOWORD(units));
    }
    ReleaseDC(NULL, hdc);
    width +=  (2 * DLG_PADDING) + (2 * BTN_PADDING * cButtons) + (BTN_SPACING * (cButtons - 1));
    return width;
}
}

class MessagePlatformBackend
{
public:
    explicit MessagePlatformBackend(QWidget *parent)
        : m_parent(parent)
    {}

    static int showMessage(QWidget *parent, const QString &msg, CMessage::Icon msgIcon,
                           CMessage::Buttons msgBtns, const CMessage::Config &conf)
    {
        MessagePlatformBackend dlg(parent);
        dlg.setText(msg);
        dlg.setIcon(msgIcon);
        dlg.setButtons(msgBtns);
        dlg.setConfig(conf);
        return dlg.exec();
    }

    void setButtons(std::initializer_list<QString> btns)
    {
        m_customButtons.assign(btns.begin(), btns.end());
    }

    void setButtons(CMessage::Buttons btns)
    {
        m_buttons = btns;
        m_customButtons.clear();
    }

    void setDefaultButton(CMessage::DefaultButton btn)
    {
        m_defaultButton = int(btn);
    }

    void setIcon(CMessage::Icon msgIcon)
    {
        m_icon = msgIcon;
    }

    void setText(const QString &text)
    {
        m_text = text;
    }

    void setConfig(const CMessage::Config &conf)
    {
        m_config = conf;
    }

    int exec();

private:
    QWidget *m_parent = nullptr;
    QString m_text;
    CMessage::Icon m_icon = CMessage::Icon::Info;
    CMessage::Buttons m_buttons = CMessage::Buttons::Ok;
    std::vector<QString> m_customButtons;
    int m_defaultButton = 0;
    CMessage::Config m_config;
};


inline int MessagePlatformBackend::exec()
{
    using Buttons = CMessage::Buttons;
    using Type = CMessage::Icon;
    using Result = CMessage::Result;

    std::wstring lpCaption = QString("%1").arg(WINDOW_TITLE).toStdWString();
    std::wstring lpText = QTextDocumentFragment::fromHtml(m_text).toPlainText().toStdWString();
    std::wstring lpContent = m_config.contentText.toStdWString();
    if (!lpContent.empty() && !m_config.linkText.isEmpty())
        lpContent.append(L"\n");
    lpContent.append(m_config.linkText.toStdWString());
    std::wstring lpCheckBoxText = m_config.checkBoxText.toStdWString();
    HWND parent_hwnd = (m_parent) ? (HWND)m_parent->winId() : nullptr;
    if (parent_hwnd && IsIconic(parent_hwnd))
        ShowWindow(parent_hwnd, SW_RESTORE);

    int msgboxID = 0;
    PCWSTR pIcon = NULL;
    switch (m_icon)
    {
    case Type::Info:    pIcon = TD_INFORMATION_ICON; break;
    case Type::Warning: pIcon = TD_WARNING_ICON; break;
    case Type::Confirm: pIcon = TD_SHIELD_ICON; break;
    case Type::Error:   pIcon = TD_ERROR_ICON; break;
    case Type::Brand:   pIcon = MAKEINTRESOURCE(IDI_MAINICON); break;
    default:            pIcon = TD_INFORMATION_ICON; break;
    }

    std::vector<TASKDIALOG_BUTTON> buttons;
    std::vector<std::wstring> btnTexts;
    int nDefltBtn = IDOK;
    auto addBtn =
        [&buttons, &btnTexts](int id, const QString &text)
        {
            btnTexts.push_back(text.toStdWString());
            buttons.push_back({id, nullptr});
        };

    if (!m_customButtons.empty())
    {
        int index = 0;
        for (const QString &btn : m_customButtons)
        {
            addBtn(CUSTOM_BUTTON_BASE_ID + index, btn);
            index++;
        }
        nDefltBtn = CUSTOM_BUTTON_BASE_ID + (m_defaultButton < index ? m_defaultButton : 0);
    }
    else
    {
        using BtnText = CMessage::ButtonText;
        switch (m_buttons)
        {
        case Buttons::YesNo:
        case Buttons::YesDefNo:
            addBtn(IDYES, BtnText::yes());
            addBtn(IDNO,  BtnText::no());
            nDefltBtn = (m_buttons == Buttons::YesDefNo) ? IDYES : IDNO;
            break;
        case Buttons::YesNoCancel:
        case Buttons::YesDefNoCancel:
            addBtn(IDYES, BtnText::yes());
            addBtn(IDNO,  BtnText::no());
            addBtn(IDCANCEL, BtnText::cancel());
            nDefltBtn = (m_buttons == Buttons::YesDefNoCancel) ? IDYES : IDCANCEL;
            break;
        case Buttons::OkCancel:
        case Buttons::OkDefCancel:
            addBtn(IDOK, BtnText::ok());
            addBtn(IDCANCEL, BtnText::cancel());
            nDefltBtn = (m_buttons == Buttons::OkDefCancel) ? IDOK : IDCANCEL;
            break;
        default:
            addBtn(IDOK, BtnText::ok());
            nDefltBtn = IDOK;
            break;
        }
    }
    for (size_t i = 0; i < buttons.size(); i++)
        buttons[i].pszButtonText = btnTexts[i].c_str();
    TASKDIALOG_BUTTON *pButtons = buttons.data();
    uint cButtons = (uint)buttons.size();

    HMODULE hInstance = GetModuleHandle(nullptr);
    BOOL chkState = (m_config.checkBoxState) ? (BOOL)*m_config.checkBoxState : FALSE;

    TaskDialogContext ctx;
    ctx.theme = TaskDialogTheme::Theme::Light;
    if (Utils::getWinVersion() >= Utils::WinVer::Win10)
    {
        const CTheme& theme = GetCurrentTheme();
        ctx.theme = theme.isSystem()
            ? TaskDialogTheme::Theme::System
            : theme.isDark()
                ? TaskDialogTheme::Theme::Dark
                : TaskDialogTheme::Theme::Light;
    }
    ctx.hIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_MAINICON), IMAGE_ICON,
                                  GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);

    TASKDIALOGCONFIG config = {0};
    ZeroMemory(&config, sizeof(config));
    config.cbSize             = sizeof(config);
    config.dwFlags            = TDF_POSITION_RELATIVE_TO_WINDOW |
                     TDF_ALLOW_DIALOG_CANCELLATION |
                     TDF_SIZE_TO_CONTENT | TDF_ENABLE_HYPERLINKS;
    if (AscAppManager::isRtlEnabled())
        config.dwFlags |= TDF_RTL_LAYOUT;
    config.hwndParent         = parent_hwnd;
    config.hInstance          = hInstance;
    config.pfCallback         = (PFTASKDIALOGCALLBACK)Pftaskdialogcallback;
    ctx.pConfig = &config;
    config.lpCallbackData     = (LONG_PTR)&ctx;
    config.pButtons           = pButtons;
    config.cButtons           = cButtons;
    config.nDefaultButton     = nDefltBtn;
    config.pszMainIcon        = pIcon;
    config.pszWindowTitle     = lpCaption.c_str();
    config.pszMainInstruction = lpText.c_str();
    config.pszContent         = !lpContent.empty() ? lpContent.c_str() : NULL;
    if (m_icon == Type::Brand)
        config.cxWidth        = calcApproxMinWidth(pButtons, cButtons) > DLG_PREF_WIDTH ? 0 : DLG_PREF_WIDTH;

    if (chkState == TRUE)
        config.dwFlags |= TDF_VERIFICATION_FLAG_CHECKED;
    if (m_config.checkBoxState)
        config.pszVerificationText = lpCheckBoxText.c_str();

    TaskDialogIndirect(&config, &msgboxID, NULL, (m_config.checkBoxState != nullptr) ? &chkState : NULL);
    if (m_config.checkBoxState != nullptr)
        *m_config.checkBoxState = (chkState == TRUE);

    if (ctx.hIcon)
        DestroyIcon(ctx.hIcon);

    int result = int(Result::Cancel);
    switch (msgboxID)
    {
    case IDYES: result = int(Result::Yes); break;
    case IDNO:  result = int(Result::No); break;
    case IDOK:  result = int(Result::Ok); break;
    case IDCANCEL: break;
    default:
        if (msgboxID >= CUSTOM_BUTTON_BASE_ID)
            result = int(Result::FirstButton) + (msgboxID - CUSTOM_BUTTON_BASE_ID);
        break;
    }

    return result;
}

#endif // CMESSAGE_WIN32_H

