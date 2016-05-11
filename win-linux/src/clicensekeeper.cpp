/*
 * (c) Copyright Ascensio System SIA 2010-2016
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

#include "clicensekeeper.h"
#include "defines.h"
#include "utils.h"
#include "../../core/DesktopEditor/common/Types.h"
#include <QDir>

#ifdef _WIN32
#include "shlobj.h"
#endif

#include <QDebug>

CLicensekeeper::CLicensekeeper()
{
}

void CLicensekeeper::init(CAscApplicationManager * m)
{
    getInstance().m_pManager = m;
    getInstance().m_pathLicense.assign(getInstance().licensePath());
}

CLicensekeeper& CLicensekeeper::getInstance()
{
    static CLicensekeeper instance;
    return instance;
}

NSEditorApi::CAscLicenceActual * CLicensekeeper::localLicense()
{
    NSEditorApi::CAscLicenceActual * pData = new NSEditorApi::CAscLicenceActual;
    pData->AddRef(); // strange code
    pData->put_Path(getInstance().m_pathLicense);
    pData->put_ProductId(PROD_ID_DESKTOP_EDITORS);

    NSEditorApi::CAscMenuEvent * pEvent =
            new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_DOCUMENTEDITORS_LICENCE_ACTUAL);
    pEvent->m_pData = pData;
    getInstance().m_pManager->Apply(pEvent);

    return pData;
}

int CLicensekeeper::localLicenseType()
{
    NSEditorApi::CAscLicenceActual * pData = localLicense();

    BYTE _ret_type = LICENSE_TYPE_NONE;
    if (pData->get_IsFree())    _ret_type = LICENSE_TYPE_FREE; else
    if (pData->get_IsDemo())    _ret_type = LICENSE_TYPE_TRIAL; else
    if (pData->get_Licence())   _ret_type = LICENSE_TYPE_BUSINESS;

    RELEASEINTERFACE(pData)

    return _ret_type;
}

void CLicensekeeper::activateLicense(const QString& key)
{
    NSEditorApi::CAscMenuEvent * pEvent = new NSEditorApi::CAscMenuEvent;

    QRegExp re("^(demo|free)");
    if (!(re.indexIn(key) < 0)) {
        NSEditorApi::CAscLicenceActual * pData = new NSEditorApi::CAscLicenceActual;
        pData->put_ProductId(PROD_ID_DESKTOP_EDITORS);
        pData->put_Path(getInstance().m_pathLicense);

        if (re.cap(1).compare("free") == 0) {
            pEvent->m_nType = ASC_MENU_EVENT_TYPE_DOCUMENTEDITORS_LICENCE_GENERATE_FREE;
        } else
        /*if (key.compare("demo") == 0)*/ {
            pEvent->m_nType = ASC_MENU_EVENT_TYPE_DOCUMENTEDITORS_LICENCE_GENERATE_DEMO;
        }

        pEvent->m_pData = pData;
    } else {
        NSEditorApi::CAscLicenceKey * pData = new NSEditorApi::CAscLicenceKey;
        pData->put_ProductId(PROD_ID_DESKTOP_EDITORS);
        pData->put_Path(getInstance().m_pathLicense);
        pData->put_Key(key.toStdString());

        pEvent->m_nType = ASC_MENU_EVENT_TYPE_DOCUMENTEDITORS_LICENCE_SEND_KEY;
        pEvent->m_pData = pData;
    }

    getInstance().m_pManager->Apply(pEvent);
//    delete pData;
}

std::wstring CLicensekeeper::licensePath()
{
    std::wstring sAppData(L"");
#ifdef _WIN32
    WCHAR szPath[MAX_PATH];
    if ( SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_COMMON_APPDATA, NULL, 0, szPath)) ) {
        sAppData = std::wstring(szPath);
        std::replace(sAppData.begin(), sAppData.end(), '\\', '/');
        sAppData.append(QString(APP_LICENSE_PATH).toStdWString());
    }

    if (sAppData.size()) {
        QDir().mkpath(QString::fromStdWString(sAppData));
    }

#else
    sAppData = QString("/var/lib").append(APP_LICENSE_PATH).toStdWString();
    QFileInfo fi(QString::fromStdWString(sAppData));
    if (!Utils::makepath(fi.absoluteFilePath())) {
        if (!fi.isWritable()) {
            // TODO: check directory permissions and warn the user
            qDebug() << "directory permission error";
        }
    }
#endif

    return sAppData;
}

void CLicensekeeper::makeTempLicense()
{
    QString _file_name = QString::fromStdWString(getInstance().m_pathLicense);
#ifdef _WIN32
    _file_name.append("/home.lic");
#else
    _file_name.append("/.home.lic");
#endif

    QFile _file(_file_name);
    if (_file.open(QFile::WriteOnly)) {
        _file.write("Ȓ»оЇ", 7);
        _file.close();

#ifdef _WIN32
        SetFileAttributes(_file_name.toStdWString().c_str(), FILE_ATTRIBUTE_HIDDEN);
#endif
    }
}

bool CLicensekeeper::isTempLicense()
{
    QString _file_name = QString::fromStdWString(getInstance().m_pathLicense);
#ifdef _WIN32
    _file_name.append("/home.lic");
#else
    _file_name.append("/.home.lic");
#endif

    return QFileInfo(_file_name).exists();
}

void CLicensekeeper::removeTempLicense()
{
    QString _file_name = QString::fromStdWString(getInstance().m_pathLicense);
#ifdef _WIN32
    _file_name.append("/home.lic");
#else
    _file_name.append("/.home.lic");
#endif

    QFile::remove(_file_name);
}

bool CLicensekeeper::hasActiveLicense()
{
    NSEditorApi::CAscLicenceActual * pData = new NSEditorApi::CAscLicenceActual;
//    pData->AddRef();
    pData->put_Path(getInstance().m_pathLicense);
    pData->put_ProductId(PROD_ID_DESKTOP_EDITORS);

    NSEditorApi::CAscMenuEvent * pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_DOCUMENTEDITORS_LICENCE_ACTUAL);
    pEvent->m_pData = pData;
    getInstance().m_pManager->Apply(pEvent);

    bool _out = pData->get_Licence() && pData->get_DaysLeft() > 0;
//    delete pData, pData = NULL;
//    delete pEvent, pEvent = NULL;
    return _out;
}
