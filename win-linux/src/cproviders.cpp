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
#include <QVector>
#include <QRegularExpression>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>


struct ProviderData {
    QString provider, editorPage;
    bool hasFrame = false,
         useRegex = false;
};

class CProviders::CProvidersPrivate
{
public:
    QVector<ProviderData> m_provid_vec;
};

CProviders::CProviders() :
    pimpl(new CProvidersPrivate)
{}

CProviders::~CProviders()
{
    delete pimpl, pimpl = nullptr;
}

CProviders& CProviders::instance()
{
    static CProviders inst;
    return inst;
}

void CProviders::init(const QString &prvds_json)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(prvds_json.toUtf8(), &err);
    if (err.error == QJsonParseError::NoError) {
        const QJsonArray arr = doc.array();
        for (const auto &val : arr) {
            QJsonObject obj = val.toObject();
            ProviderData pd;
            pd.provider = obj["provider"].toString().toLower();
            pd.hasFrame = obj["editorFrameSize"].toString() == "finite";
            pd.editorPage = obj["editorPage"].toString();
            QString reg("regex:");
            int ind = pd.editorPage.indexOf(reg);
            if (ind != -1) {
                pd.useRegex = true;
                pd.editorPage = pd.editorPage.mid(ind + reg.length());
            }
            pimpl->m_provid_vec.push_back(std::move(pd));
        }
    }
}

bool CProviders::editorsHasFrame(const QString &url, const QString &cloud)
{
    foreach (const auto &pd, pimpl->m_provid_vec) {
        if (!pd.provider.isEmpty() && pd.provider == cloud)
            return pd.hasFrame;
        if (!pd.editorPage.isEmpty()) {
            if (pd.useRegex) {
                QRegularExpression rgx(pd.editorPage, QRegularExpression::CaseInsensitiveOption);
                if (rgx.match(url).hasMatch())
                    return pd.hasFrame;
            } else
            if (url.indexOf(pd.editorPage) != -1)
                return pd.hasFrame;
        }
    }
    return false;
}
