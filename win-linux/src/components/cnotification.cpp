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

#include "components/cnotification.h"
#include "defines.h"
#include "version.h"

#ifdef __linux__

#else
# include "platform_win/wintoastlib.h"

using namespace WinToastLib;


class ToastHandler : public IWinToastHandler {
public:
    void toastActivated() const {
        std::wcout << L"The user clicked in this toast" << std::endl;
    }
    void toastActivated(int actionIndex) const {
        std::wcout << L"The user clicked on button #" << actionIndex << L" in this toast" << std::endl;
    }
    void toastFailed() const {
        std::wcout << L"Error showing current toast" << std::endl;
    }
    void toastDismissed(WinToastDismissalReason state) const {
        switch (state) {
        case UserCanceled:
            std::wcout << L"The user dismissed this toast" << std::endl;
            break;
        case ApplicationHidden:
            std::wcout << L"The application hid the toast using ToastNotifier.hide()" << std::endl;
            break;
        case TimedOut:
            std::wcout << L"The toast has timed out" << std::endl;
            break;
        default:
            std::wcout << L"Toast not activated" << std::endl;
            break;
        }
    }
};
#endif

void CNotification::showNotification(const QString &msg)
{
#ifdef __linux__

#else
    WinToast::instance()->setAppName(TEXT(WINDOW_TITLE));
    WinToast::instance()->setAppUserModelId(WinToast::configureAUMI(TEXT(VER_COMPANYNAME_STR), TEXT(VER_PRODUCTNAME_STR), TEXT(VER_PRODUCTNAME_STR), TEXT(VER_FILEVERSION_STR)));
    if (!WinToast::instance()->initialize()) {
        std::wcout << L"Error, your system in not compatible!" << std::endl;
        return;
    }
    WinToastTemplate templ = WinToastTemplate(WinToastTemplate::WinToastTemplateType::Text03);
    templ.setTextField(msg.toStdWString(), WinToastTemplate::FirstLine);
    templ.setTextField(L"Second", WinToastTemplate::SecondLine);
    templ.setTextField(L"Third", WinToastTemplate::ThirdLine);
    templ.setExpiration(5000);

    templ.addAction(L"Yes");
    templ.addAction(L"No");
    templ.addAction(L"Ok");

    if (WinToast::instance()->showToast(templ, new ToastHandler()) < 0) {
        std::wcout << L"Could not launch your toast notification!" << std::endl;
    }
#endif
}
