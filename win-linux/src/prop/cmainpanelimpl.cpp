/*
 * (c) Copyright Ascensio System SIA 2010-2017
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
 * You can contact Ascensio System SIA at Lubanas st. 125a-25, Riga, Latvia,
 * EU, LV-1021.
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
#include "version_p.h"

#include <QJsonObject>

#define HTML_QUOTE "\\u005c&quot;" // \" symbols
#define QCEF_CAST(Obj) qobject_cast<QCefView *>(Obj)

CMainPanelImpl::CMainPanelImpl(QWidget *parent, bool isCustomWindow, uchar scale)
    : CMainPanel(parent, isCustomWindow, scale)
{
}

void CMainPanelImpl::refreshAboutVersion()
{
    QString _license = "Licensed under &lt;a onclick=" HTML_QUOTE "window.open('" URL_AGPL "')" HTML_QUOTE
                            " href=" HTML_QUOTE "#" HTML_QUOTE "&gt;GNU AGPL v3&lt;/a&gt;";

    QJsonObject _json_obj;
    _json_obj["version"]    = VER_FILEVERSION_STR;
    _json_obj["edition"]    = "%1";
    _json_obj["appname"]    = WINDOW_NAME;
    _json_obj["rights"]     = "© " ABOUT_COPYRIGHT_STR;
    _json_obj["link"]       = URL_SITE;

    AscAppManager::sendCommandTo( nullptr, "app:version", Utils::encodeJson(_json_obj).arg(_license) );
}

void CMainPanelImpl::updateScaling()
{
    CMainPanel::updateScaling();

    QString _tabs_stylesheets = m_dpiRatio > 1 ? ":/sep-styles/tabbar@2x" : ":/sep-styles/tabbar";
    if ( m_isCustomWindow ) {
        _tabs_stylesheets += ".qss";

    } else {
#ifdef __linux__
        _tabs_stylesheets += ".nix.qss";
#endif
    }

    QFile styleFile(_tabs_stylesheets);
    styleFile.open( QFile::ReadOnly );
    m_pTabs->setStyleSheet(QString(styleFile.readAll()));

    std::map<int, std::pair<QString, QString> > icons;
    if ( m_dpiRatio > 1 ) {
        icons.insert({
            {etUndefined, std::make_pair(":/newdocument@2x.png", ":/newdocument@2x.png")},
            {etDocument, std::make_pair(":/de_normal@2x.png", ":/de_active@2x.png")},
            {etPresentation, std::make_pair(":/pe_normal@2x.png", ":/pe_active@2x.png")},
            {etSpreadsheet, std::make_pair(":/se_normal@2x.png", ":/se_active@2x.png")},
            {etPortal, std::make_pair(":/portal.png", ":/portal@2x.png")}
        });
    } else {
        icons.insert({
            {etUndefined, std::make_pair(":/newdocument.png", ":/newdocument.png")},
            {etDocument, std::make_pair(":/de_normal.png", ":/de_active.png")},
            {etPresentation, std::make_pair(":/pe_normal.png", ":/pe_active.png")},
            {etSpreadsheet, std::make_pair(":/se_normal.png", ":/se_active.png")},
            {etPortal, std::make_pair(":/portal.png", ":/portal.png")}
        });
    }

    m_pTabs->setTabIcons(icons);
    m_pTabs->setScaling(m_dpiRatio);
}
