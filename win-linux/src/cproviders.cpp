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

#include "cproviders.h"
#include "defines.h"
#include <QSettings>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>


CProviders::CProviders()
{}

CProviders::~CProviders()
{}

void CProviders::updateProviders(const QString &prvds_json)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(prvds_json.toUtf8(), &err);
    if (err.error == QJsonParseError::NoError) {
        const QJsonArray arr = doc.array();
        GET_REGISTRY_USER(reg_user)
        if (!reg_user.contains("providers")) {
            QJsonObject prvds;
            for (const auto &val : arr) {
                QJsonObject obj = val.toObject();
                prvds.insert(obj["provider"].toString().toLower(), QJsonObject({{"path", obj["path"]}}));
            }
            if (!prvds.isEmpty())
                reg_user.setValue("providers", prvds);
        } else {
            bool need_update = false;
            QJsonObject prvds = reg_user.value("providers").toJsonObject();
            for (const auto &val : arr) {
                QJsonObject obj = val.toObject();
                QString pvd = obj["provider"].toString().toLower();
                if (!prvds.contains(pvd)) {
                    prvds.insert(pvd, QJsonObject({{"path", obj["path"]}}));
                    if (!need_update)
                        need_update = true;
                }
            }
            if (need_update)
                reg_user.setValue("providers", prvds);
        }
    }
}

void CProviders::addRootUrl(const QString &login_json)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(login_json.toUtf8(), &err);
    if (err.error == QJsonParseError::NoError) {
        QJsonObject obj = doc.object();
        if (obj.contains("provider") && obj.contains("domain")) {
            QString pvd = obj["provider"].toString().toLower();
            GET_REGISTRY_USER(reg_user)
            QJsonObject prvds = reg_user.value("providers").toJsonObject();
            if (prvds.contains(pvd)) {
                QString domain = obj["domain"].toString();
                QJsonObject val = prvds[pvd].toObject();
                if (!val.contains("domain")) {
                    QJsonArray arr;
                    arr.append(domain);
                    val.insert("domain", arr);
                    prvds[pvd] = val;
                    reg_user.setValue("providers", prvds);
                } else {
                    QJsonArray arr = val["domain"].toArray();
                    if (!arr.contains(domain)) {
                        arr.append(domain);
                        val["domain"] = arr;
                        prvds[pvd] = val;
                        reg_user.setValue("providers", prvds);
                    }
                }
            }
        }
    }
}

bool CProviders::providerIsIntegrator(const QString &url)
{
    QString root_url = QUrl(url).toString(QUrl::RemovePath | QUrl::RemoveQuery | QUrl::RemoveFragment);
    if (!root_url.isEmpty()) {
        const QStringList integrators({"moodle"});
        GET_REGISTRY_USER(reg_user)
        const QJsonObject prvds = reg_user.value("providers").toJsonObject();
        for (const auto &pvd : integrators) {
            if (prvds.contains(pvd)) {
                QJsonObject val = prvds[pvd].toObject();
                if (val.contains("domain")) {
                    const QJsonArray arr = val["domain"].toArray();
                    if (arr.contains(root_url))
                        return true;
                }
            }
        }
    }
    return false;
}
