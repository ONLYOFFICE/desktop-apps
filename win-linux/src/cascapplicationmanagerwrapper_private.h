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

#ifndef CASCAPPLICATIONMANAGERWRAPPER_PRIVATE_H
#define CASCAPPLICATIONMANAGERWRAPPER_PRIVATE_H

#include "cascapplicationmanagerwrapper.h"
#include "qcefview_media.h"
#include "defines.h"
#include "clangater.h"

class CAscApplicationManagerWrapper_Private
{
public:
    CAscApplicationManagerWrapper_Private(CAscApplicationManagerWrapper * manager)
        : m_appmanager(*manager)
    {
    }

    virtual ~CAscApplicationManagerWrapper_Private() {}

    virtual void initializeApp() {}
    virtual bool processEvent(NSEditorApi::CAscCefMenuEvent *) { return false; }
    virtual void applyStylesheets() {}
    virtual QCefView * createView(QWidget * parent)
    {
        return new QCefView_Media(parent);
    }

    bool allowedCreateLocalFile()
    {
        return true;
    }

    virtual void init()
    {
        m_appmanager.m_oSettings.sign_support = false;
    }

    auto createStartPanel() -> void {
        GET_REGISTRY_USER(reg_user)

        m_pStartPanel = AscAppManager::createViewer(nullptr);
        m_pStartPanel->Create(&m_appmanager, cvwtSimple);
        m_pStartPanel->setObjectName("mainPanel");

        QString data_path;
#if defined(QT_DEBUG)
        data_path = reg_user.value("startpage").value<QString>();
#endif

        if ( data_path.isEmpty() )
            data_path = qApp->applicationDirPath() + "/index.html";

        QString additional = "?waitingloader=yes&lang=" + CLangater::getCurrentLangCode();
        QString _portal = reg_user.value("portal").value<QString>();
        if (!_portal.isEmpty()) {
            QString arg_portal = (additional.isEmpty() ? "?portal=" : "&portal=") + _portal;
            additional.append(arg_portal);
        }

        std::wstring start_path = ("file:///" + data_path + additional).toStdWString();
        m_pStartPanel->GetCefView()->load(start_path);
    }

    auto mainWindow() -> CMainWindow * const {
        return m_appmanager.m_vecWindows.empty() ? nullptr : reinterpret_cast<CMainWindow * const>(m_appmanager.m_vecWindows[0]);
    }

    auto extendStylesheets(const std::vector<QString>& veccss) -> void {
        if ( !veccss.empty() ) {
            m_appmanager.m_vecStyles.push_back(veccss[0]);

            if ( veccss.size() > 1 )
                m_appmanager.m_vecStyles2x.push_back(veccss[1]);
        }
    }

public:
    CAscApplicationManagerWrapper& m_appmanager;
    QPointer<QCefView> m_pStartPanel;
};

//CAscApplicationManagerWrapper::CAscApplicationManagerWrapper()
//    : CAscApplicationManagerWrapper(new CAscApplicationManagerWrapper::CAscApplicationManagerWrapper_Private(this))
//{
//}

#endif // CASCAPPLICATIONMANAGERWRAPPER_PRIVATE_H
