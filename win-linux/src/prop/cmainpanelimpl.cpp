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

#include "cmainpanelimpl.h"
#include "cascapplicationmanagerwrapper.h"
#include "defines.h"
#include "utils.h"
#include "version.h"
#include "clangater.h"

#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>

#define QCEF_CAST(Obj) qobject_cast<QCefView *>(Obj)

CMainPanelImpl::CMainPanelImpl(QWidget *parent, bool isCustomWindow, double scale)
    : CMainPanel(parent, isCustomWindow, scale)
{
    QObject::connect(CLangater::getInstance(), &CLangater::onLangChanged, std::bind(&CMainPanelImpl::refreshAboutVersion, this));
}

void CMainPanelImpl::refreshAboutVersion()
{
    QString _license = tr("Licensed under") + " &lt;a class=\"link\" onclick=\"window.open('" URL_AGPL "')\" draggable=\"false\" href=\"#\"&gt;GNU AGPL v3&lt;/a&gt;";

    QJsonObject _json_obj;
    _json_obj["version"]    = VER_FILEVERSION_STR;
    _json_obj["edition"]    = _license;
    _json_obj["appname"]    = WINDOW_NAME;
    _json_obj["rights"]     = "© " ABOUT_COPYRIGHT_STR;
    _json_obj["link"]       = URL_SITE;

    AscAppManager::sendCommandTo(SEND_TO_ALL_START_PAGE, "app:version", Utils::stringifyJson(_json_obj));

    _json_obj.empty();
    _json_obj.insert("locale",
        QJsonObject({
            {"current", CLangater::getCurrentLangCode()},
            {"langs", CLangater::availableLangsToJson()}
        })
    );

    std::wstring _force_value = AscAppManager::userSettings(L"force-scale");
    if ( _force_value == L"1" )
        _json_obj["uiscaling"] = 100;
    else
    if ( _force_value == L"1.5" )
        _json_obj["uiscaling"] = 150;
    else
    if ( _force_value == L"2" )
        _json_obj["uiscaling"] = 200;
    else _json_obj["uiscaling"] = 0;

    _json_obj["uitheme"] = QString::fromStdWString(AscAppManager::themes().current());

    AscAppManager::sendCommandTo(SEND_TO_ALL_START_PAGE, "settings:init", Utils::stringifyJson(_json_obj));
    if ( InputArgs::contains(L"--ascdesktop-reveal-app-config") )
            AscAppManager::sendCommandTo( nullptr, "retrive:localoptions", "" );
}

void CMainPanelImpl::updateScaling(double dpiratio)
{
    CMainPanel::updateScaling(dpiratio);

    std::wstring prefix{AscAppManager::themes().value(CThemes::ColorRole::ecrLogoColor)};
    QString logo_name = QString(":/logo_%1%2.png")
            .arg(QString::fromStdWString(prefix))
            .arg(dpiratio > 1 ? dpiratio > 1.5 ? "@2x" : "@1.5x" : "");
//    QPixmap pixmap(dpiratio > 1 ? ":/logo@2x.png" : ":/logo.png");
    QPixmap pixmap(logo_name);
    m_pButtonMain->setText(QString());
    m_pButtonMain->setIcon(QIcon(pixmap));
    m_pButtonMain->setIconSize(pixmap.size());
}

void CMainPanelImpl::applyTheme(const std::wstring& theme)
{
    CMainPanel::applyTheme(theme);

    int dpiratio = scaling();
    std::wstring prefix{AscAppManager::themes().value(theme, CThemes::ColorRole::ecrLogoColor)};
    QString logo_name = QString(":/logo_%1%2.png")
            .arg(QString::fromStdWString(prefix))
            .arg(dpiratio > 1 ? dpiratio > 1.5 ? "@2x" : "@1.5x" : "");
    QPixmap pixmap(logo_name);
    m_pButtonMain->setText(QString());
    m_pButtonMain->setIcon(QIcon(pixmap));
    m_pButtonMain->setIconSize(pixmap.size());
}

void CMainPanelImpl::onLocalOptions(const QString& json)
{
    QJsonParseError jerror;
    QJsonDocument jdoc = QJsonDocument::fromJson(json.toLatin1(), &jerror);

    if( jerror.error == QJsonParseError::NoError ) {
        QFile file(Utils::getAppCommonPath() + "/app.conf");
        file.open(QFile::WriteOnly);
        file.write(jdoc.toJson());
        file.close();
    }
}
