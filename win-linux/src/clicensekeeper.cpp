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
#include "cmessage.h"
#include "../../core/DesktopEditor/common/Types.h"
#include <QDir>

#ifdef _WIN32
#include "shlobj.h"
extern HWND gTopWinId;
#else
#include <QApplication>
QWidget * gTopWinId = nullptr;
#endif

#include <QDebug>

CLicensekeeper::CLicensekeeper()
    : m_waitServerLicense(false)
    , m_procLicenseType(LICENSE_TYPE_BUSINESS)
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

bool CLicensekeeper::tempLicenseExist()
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

void CLicensekeeper::selfActivation(int type)
{
    if ( !getInstance().lickeyCount() ) {
        getInstance().m_procLicenseType = type;

        if (type == LICENSE_TYPE_FREE) {
            getInstance().m_waitServerLicense = true;
            activateLicense("free");
        } else {
            activateLicense("demo");
        }
    }
}

void CLicensekeeper::serverActivationDone(void * d)
{
    getInstance().m_waitServerLicense = false;

    if ( getInstance().m_appReady ) {
        getInstance().removeTempLicense();
        getInstance().processServerLicense(d);

        if ( getInstance().server_lic_callback ) {
            getInstance().server_lic_callback(LICENSE_ACTION_NO_ACTION);
            getInstance().server_lic_callback = nullptr;
        }
    } else {
        getInstance().m_waitNoConnection =
                static_cast<NSEditorApi::CAscLicenceActual *>(d)->get_IsServerUnavailable();
    }
}

void CLicensekeeper::checkLocalLicense(std::function<void(int)> callback)
{
    int _lic_res = checkLocalLicense();

    if (_lic_res == LICENSE_ACTION_WAIT_LICENSE)
        getInstance().server_lic_callback = callback;
    else
    if (callback) callback(_lic_res);
}

int CLicensekeeper::checkLocalLicense()
{
#if __linux
    if ( !gTopWinId ) {
        QWidgetList widgets = qApp->topLevelWidgets();

        for (auto w : widgets) {
            if (w->objectName() == "MainWindow") {
                gTopWinId = w;
                break;
            }
        }
    }
#endif

    NSEditorApi::CAscLicenceActual * pData = localLicense();

    int _out = LICENSE_ACTION_NO_ACTION;

    if (getInstance().m_waitServerLicense)
        _out = LICENSE_ACTION_WAIT_LICENSE;
    else
    if ( tempLicenseExist() ) {
        removeTempLicense();
        if (getInstance().m_waitNoConnection) {
            pData->put_IsServerUnavailable(true);
            getInstance().m_waitNoConnection = false;
        }

        getInstance().processServerLicense(pData);
    } else
        _out = processLicense(pData);

    getInstance().m_appReady = true;
    RELEASEINTERFACE(pData)

    return _out;
}

int CLicensekeeper::processLicense(void * data)
{
    NSEditorApi::CAscLicenceActual * pData = static_cast<NSEditorApi::CAscLicenceActual *>(data);

    if ( !pData->get_Licence() ) {
        getInstance().showActivationPage(true);
        warnNoLicense();
    } else {
        removeTempLicense();

        if (pData->get_IsDemo()) {
            return getInstance().warnDemoLicense(pData);
        } else {
            return getInstance().dailyLicense(pData);
        }
    }

    return LICENSE_ACTION_NO_ACTION;
}

void CLicensekeeper::warnNoLicense()
{
    CMessage mess(gTopWinId);

#if !defined(_AVS)
    mess.setButtons(QObject::tr("Buy Now"), "");
    if (MODAL_RESULT_BTN1 == mess.showModal(
                QObject::tr("The program is unregistered"), QMessageBox::Information)) {
        Utils::openUrl(URL_BUYNOW);
    }

    getInstance().selectActivationPage();
#else
    mess.setButtons(QObject::tr("Activate"), QObject::tr("Continue")+":focus");
    int _modal_res = mess.showModal(QObject::tr("The application isn't activated! A watermark will be added to document."), QMessageBox::Information);

    if (_modal_res == MODAL_RESULT_BTN1)
        getInstance().selectActivationPage();
#endif
}

int CLicensekeeper::warnDemoLicense(void * d)
{
    NSEditorApi::CAscLicenceActual * pData = static_cast<NSEditorApi::CAscLicenceActual *>(d);

    showActivationPage(true);

    CMessage mess(gTopWinId);
    if (pData->get_DaysLeft() == 0) {
        mess.setButtons(QObject::tr("Activate"), QObject::tr("Continue"));

        if (MODAL_RESULT_BTN1 == mess.showModal(QObject::tr("The trial period is over."), QMessageBox::Information)) {

            selectActivationPage();
            return LICENSE_ACTION_GO_ACTIVATE;
        }
    } else {
        mess.showModal(QObject::tr("Trial period expired for %1 days.")
                            .arg(pData->get_DaysLeft()), QMessageBox::Information);
    }

    return LICENSE_ACTION_NO_ACTION;
}

int CLicensekeeper::dailyLicense(void * d)
{
    NSEditorApi::CAscLicenceActual * pData = static_cast<NSEditorApi::CAscLicenceActual *>(d);

    if (pData->get_DaysBetween() > 0) {
        // the license checked more then 1 day before
        if (pData->get_DaysLeft() < 15) {
            showActivationPage(true);

            CMessage mess(gTopWinId);
            mess.setButtons(QObject::tr("Continue"), "");
            mess.showModal(QObject::tr("%1 days left before the license end")
                                .arg(pData->get_DaysLeft()), QMessageBox::Information);
        }
    }

    showActivationPage(localLicenseType()!=LICENSE_TYPE_BUSINESS);
    return LICENSE_ACTION_NO_ACTION;
}

void CLicensekeeper::processServerLicense(void * d)
{
    m_waitServerLicense = false;
//    removeTempLicense();

    NSEditorApi::CAscLicenceActual * pData =
            static_cast<NSEditorApi::CAscLicenceActual *>(d);

    CMessage mess(gTopWinId);
    if ( pData->get_Licence() ) {
        QString descr = QObject::tr("Congrats! %1 %2 was succefully activated!")
#if defined(_IVOLGA_PRO) || defined(_AVS)
        .arg(APP_TITLE);
#else
        .arg("ONLYOFFICE Desktop Editors");
#endif

        QString _edition;
#if !defined(_AVS)
        if (pData->get_IsFree()) {
            _edition = "(" + QObject::tr("Home") + ")";
        } else
        if (!pData->get_IsDemo()) {
            _edition = "(" + QObject::tr("Business") + ")";
        }
#endif
        showActivationPage(localLicenseType()!=LICENSE_TYPE_BUSINESS);

        mess.showModal(descr.arg(_edition), QMessageBox::Information);
    } else {
        showActivationPage(true);

        if (pData->get_IsServerUnavailable()) {
            processNoConnection();
        } else {
            mess.showModal(QObject::tr("Activation failed! Check entered data and try again."), QMessageBox::Information);
        }
    }
}

void CLicensekeeper::processNoConnection()
{
    QString descr = QObject::tr("Activation failed! Check internet connection and try again.");

    CMessage mess(gTopWinId);
    if (m_procLicenseType == LICENSE_TYPE_FREE) {
        mess.setButtons(QObject::tr("Activate"), QObject::tr("Continue"));

        if (MODAL_RESULT_BTN1 == mess.showModal(descr, QMessageBox::Warning)) {
            m_waitServerLicense = true;
            activateLicense("free");
        } else {
            makeTempLicense();
        }
    } else {
        mess.showModal(descr, QMessageBox::Information);
    }
}

int CLicensekeeper::lickeyCount()
{
    return QDir(QString::fromStdWString(licensePath()))
                    .entryInfoList(QStringList("*.lickey"), QDir::Files).size();
}

void CLicensekeeper::showActivationPage(bool show)
{
    cmdMainPage("lic:active", QString::number(!show));
}

void CLicensekeeper::selectActivationPage()
{
    cmdMainPage("lic:selectpanel", "");
}

void CLicensekeeper::cmdMainPage(const QString& cmd, const QString& args) const
{
    NSEditorApi::CAscExecCommandJS * pCommand = new NSEditorApi::CAscExecCommandJS;
    pCommand->put_Command(cmd.toStdWString());
    if (args.size())
        pCommand->put_Param(args.toStdWString());

    NSEditorApi::CAscMenuEvent * pEvent = new NSEditorApi::CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_EXECUTE_COMMAND_JS);
    pEvent->m_pData = pCommand;

    int id = 1; // TODO: get id of "start" page
    CCefView * mainPanel = getInstance().m_pManager->GetViewById(id);
    if (mainPanel) {
        mainPanel->Apply(pEvent);
    }

//    RELEASEOBJECT(pEvent)
//    RELEASEOBJECT(pCommand)
}
