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

#ifdef _WIN32
# include <QCoreApplication>
# include <QDir>
# include "utils.h"
# define APP_LAUNCH_NAME "\\" REG_APP_NAME ".exe"
# define APP_SHORTCUT_NAME "\\" APP_REG_NAME ".lnk"
#else
# include <libnotify/notify.h>
# include <gio/gio.h>
# include <glib.h>
# include <unordered_map>
# include "utils.h"
# include <QDBusConnection>
# include <QDBusInterface>
# include <QDBusMessage>
# include <QDBusVariant>
#endif
#include <QTextDocumentFragment>
#include "components/cnotification.h"
#include "defines.h"

#define NOTIF_TIMEOUT_MS 10000

#ifdef __linux__
struct NtfData
{
    FnVoidInt callback;
    std::vector<int> results;
};

typedef std::unordered_map<NotifyNotification*, NtfData> NtfMap;

// Action names are button indices "0", "1", ...
static void action_callback(NotifyNotification *ntf, char *action, void *data)
{
    if (!data)
        return;
    NtfMap *ntfMap = (NtfMap*)data;
    auto it = ntfMap->find(ntf);
    if (it != ntfMap->end()) {
        const NtfData &ntfData = it->second;
        if (ntfData.callback) {
            const size_t index = size_t(atoi(action));
            int res = index < ntfData.results.size() ? ntfData.results[index] : -1;
            ntfData.callback(res);
        }
    }
}

static void on_close(NotifyNotification *ntf, void *data)
{
    if (!data)
        return;
    NtfMap *ntfMap = (NtfMap*)data;
    auto it = ntfMap->find(ntf);
    if (it != ntfMap->end()) {
        g_object_unref(ntf);
        ntfMap->erase(it);
    }
}

static bool isNotificationsEnabled()
{
    if (WindowHelper::getEnvInfo() == WindowHelper::GNOME) {
        GSettings *stn = g_settings_new("org.gnome.desktop.notifications");
        GVariant *var = g_settings_get_value(stn, "show-banners");
        gboolean res = false;
        g_variant_get(var, "b", &res);
        g_object_unref(var);
        g_object_unref(stn);
        return res;
    } else
    if (WindowHelper::getEnvInfo() == WindowHelper::CINNAMON) {
        GSettings *stn = g_settings_new("org.cinnamon.desktop.notifications");
        GVariant *var = g_settings_get_value(stn, "display-notifications");
        gboolean res = false;
        g_variant_get(var, "b", &res);
        g_object_unref(var);
        g_object_unref(stn);
        return res;
    } else
    if (WindowHelper::getEnvInfo() == WindowHelper::KDE) {
        QDBusConnection conn = QDBusConnection::sessionBus();
        if (conn.isConnected()) {
            QDBusInterface itf("org.freedesktop.Notifications", "/org/freedesktop/Notifications", "org.freedesktop.DBus.Properties", conn);
            if (itf.isValid()) {
                QDBusMessage msg = itf.call("Get", "org.freedesktop.Notifications", "Inhibited");
                if (msg.type() == QDBusMessage::ReplyMessage && msg.arguments().size() > 0) {
                    QVariant var = msg.arguments().at(0);
                    if (var.canConvert<QDBusVariant>()) {
                        QVariant res = var.value<QDBusVariant>().variant();
                        if (res.type() == QVariant::Bool && !res.toBool())
                            return true;
                    }
                }
            }
        }
    } else
    if (WindowHelper::getEnvInfo() == WindowHelper::XFCE) {
        QDBusConnection conn = QDBusConnection::sessionBus();
        if (conn.isConnected()) {
            QDBusInterface itf("org.xfce.Xfconf", "/org/xfce/Xfconf", "org.xfce.Xfconf", conn);
            if (itf.isValid()) {
                QDBusMessage msg = itf.call("GetProperty",  "xfce4-notifyd", "/do-not-disturb");
                if (msg.type() == QDBusMessage::ErrorMessage)
                    return true; // By default the property is not defined and notifications are enabled
                if (msg.type() == QDBusMessage::ReplyMessage && msg.arguments().size() > 0) {
                    QVariant var = msg.arguments().at(0);
                    if (var.canConvert<QDBusVariant>()) {
                        QVariant res = var.value<QDBusVariant>().variant();
                        if (res.type() == QVariant::Bool && !res.toBool())
                            return true;
                    }
                }
            }
        }
    }
    return false;
}
#else
# include "platform_win/wintoastlib.h"

using namespace WinToastLib;

class ToastHandler : public IWinToastHandler {
public:
    ToastHandler(const std::vector<int> &_results, FnVoidInt _callback) : results(_results), callback(_callback)
    {}
    virtual void toastActivated() const final // The user clicked in this toast
    {}
    virtual void toastActivated(int actionIndex) const final // The user clicked on button #actionIndex
    {
        int res = (actionIndex >= 0 && size_t(actionIndex) < results.size()) ? results[actionIndex] : -1;
        if (callback)
            callback(res);
    }
    virtual void toastActivated(const char* response) const final
    {}
    virtual void toastFailed() const final // Error showing current toast
    {
        if (callback)
            callback(NOTIF_FAILED);
    }
    virtual void toastDismissed(WinToastDismissalReason state) const final
    {
        switch (state) {
        case UserCanceled: // The user dismissed this toast
            break;
        case ApplicationHidden: // The application hide the toast using ToastNotifier.hide()
            break;
        case TimedOut: // The toast has timed out
            break;
        default: // Toast not activated
            break;
        }
    }

private:
    std::vector<int> results;
    FnVoidInt callback;
};
#endif

class CNotification::CNotificationPrivate
{
public:
#ifdef __linux__
    ~CNotificationPrivate()
    {
        clear();
    }
    void clear()
    {
        for (auto it = ntfMap.begin(); it != ntfMap.end();) {
            NotifyNotification *ntf = it->first;
            it = ntfMap.erase(it);
            notify_notification_close(ntf, NULL);
            g_object_unref(ntf);
        }
    }

    NtfMap ntfMap;
#endif
    bool show(const QString &msg, const QString &content,
              const QStringList &btns, const std::vector<int> &results,
              const FnVoidInt &callback);

    int isInit = -1;
};

bool CNotification::CNotificationPrivate::show(const QString &msg, const QString &content,
                                               const QStringList &btns, const std::vector<int> &results,
                                               const FnVoidInt &callback)
{
#ifdef __linux__
    if (!isNotificationsEnabled())
        return false;
    QString lpText = QTextDocumentFragment::fromHtml(msg).toPlainText();
    NotifyNotification *ntf = notify_notification_new(lpText.toLocal8Bit().data(), content.toLocal8Bit().data(), NULL);
    g_signal_connect(G_OBJECT(ntf), "closed", G_CALLBACK(on_close), (void*)&ntfMap);
    notify_notification_set_urgency(ntf, NotifyUrgency::NOTIFY_URGENCY_NORMAL);
    notify_notification_set_timeout(ntf, NOTIF_TIMEOUT_MS);
    ntfMap[ntf] = NtfData{callback, results};

    if (callback) {
        for (int i = 0; i < btns.size(); i++) {
            const QByteArray action = QByteArray::number(i);
            notify_notification_add_action(ntf, action.constData(), btns[i].toLocal8Bit().constData(),
                                           NOTIFY_ACTION_CALLBACK(action_callback), (void*)&ntfMap, NULL);
        }
    }
    return notify_notification_show(ntf, NULL);
#else
    std::wstring lpText = QTextDocumentFragment::fromHtml(msg).toPlainText().toStdWString();
    std::wstring lpContent = content.toStdWString();

    WinToastTemplate tmpl(WinToastTemplate::WinToastTemplateType::ImageAndText02);
    tmpl.setTextField(lpText, WinToastTemplate::FirstLine);
    tmpl.setTextField(lpContent, WinToastTemplate::SecondLine);
    const QString appIcon = QCoreApplication::applicationDirPath() + "/app.ico";
    if (QFileInfo::exists(appIcon))
        tmpl.setImagePath(appIcon.toStdWString());
    tmpl.setAudioPath(WinToastTemplate::AudioSystemFile::Mail);
    tmpl.setDuration(WinToastTemplate::Duration::System);
    tmpl.setExpiration(NOTIF_TIMEOUT_MS);

    for (const QString &btn : btns)
        tmpl.addAction(btn.toStdWString());
    return WinToast::instance()->showToast(tmpl, new ToastHandler(results, callback)) > -1;
#endif
}


CNotification& CNotification::instance()
{
    static CNotification inst;
    return inst;
}

CNotification::CNotification() :
    pimpl(new CNotificationPrivate)
{}

CNotification::~CNotification()
{
    delete pimpl, pimpl = nullptr;
#ifdef __linux__
    if (notify_is_initted())
        notify_uninit();
#endif
}

bool CNotification::init()
{
    if (pimpl->isInit != -1)
        return pimpl->isInit;

#ifdef __linux__
    pimpl->isInit = notify_init(WINDOW_TITLE);
#else
    if (Utils::getWinVersion() < Utils::WinVer::Win10) {
        pimpl->isInit = 0;
        return false;
    }
    WinToast::instance()->setAppName(TEXT(WINDOW_TITLE));
    WinToast::instance()->setAppUserModelId(TEXT(APP_USER_MODEL_ID));
    if (IsPackage(Portable)) {
        WinToast::instance()->setShortcutPolicy(WinToastLib::WinToast::SHORTCUT_POLICY_IGNORE);
    } else {
        WinToast::instance()->setShortcutPolicy(WinToastLib::WinToast::SHORTCUT_POLICY_REQUIRE_NO_CREATE);
        PWSTR progPath = nullptr;
        if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_CommonPrograms, 0, nullptr, &progPath))) {
            std::wstring shortcutPath(progPath);
            shortcutPath.append(L"\\");
            shortcutPath.append(TEXT(APP_REG_NAME));
            shortcutPath.append(TEXT(APP_SHORTCUT_NAME));
            WinToast::instance()->setShortcutPath(shortcutPath);
        }
        CoTaskMemFree(progPath);
    }
    pimpl->isInit = WinToast::instance()->initialize();
#endif
    return pimpl->isInit;
}

void CNotification::clear()
{
    if (pimpl->isInit == 1)
#ifdef __linux__
        pimpl->clear();
#else
        WinToast::instance()->clear();
#endif
}

bool CNotification::show(const QString &msg, const QString &content)
{
    return pimpl->show(msg, content, {}, {}, nullptr);
}

bool CNotification::show(const QString &msg, const QString &content,
                         CMessage::Buttons dlgBtns, const FnVoidInt &callback)
{
    using Buttons = CMessage::Buttons;
    using Result = CMessage::Result;
    using BtnText = CMessage::ButtonText;

    QStringList btns;
    std::vector<int> results;
    switch (dlgBtns) {
    case Buttons::Ok:
        btns << BtnText::ok();
        results = {int(Result::Ok)};
        break;
    case Buttons::OkCancel:
    case Buttons::OkDefCancel:
        btns << BtnText::ok() << BtnText::cancel();
        results = {int(Result::Ok), int(Result::Cancel)};
        break;
    case Buttons::YesNo:
    case Buttons::YesDefNo:
        btns << BtnText::yes() << BtnText::no();
        results = {int(Result::Yes), int(Result::No)};
        break;
    case Buttons::YesNoCancel:
    case Buttons::YesDefNoCancel:
        btns << BtnText::yes() << BtnText::no() << BtnText::cancel();
        results = {int(Result::Yes), int(Result::No), int(Result::Cancel)};
        break;
    default:
        break;
    }
    return pimpl->show(msg, content, btns, results, callback);
}

bool CNotification::show(const QString &msg, const QString &content,
                         std::initializer_list<QString> btns, const FnVoidInt &callback)
{
    QStringList btnList;
    std::vector<int> results;
    int index = 0;
    for (const QString &btn : btns) {
        btnList << btn;
        results.push_back(int(CMessage::Result::FirstButton) + index);
        index++;
    }
    return pimpl->show(msg, content, btnList, results, callback);
}
