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

#ifndef QMAINPANEL_H
#define QMAINPANEL_H

#include <QMouseEvent>
#include <QResizeEvent>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QPushButton>
#include "qwinwidget.h"

#include "applicationmanager.h"
#include <QHBoxLayout>
#include "asctabwidget.h"
#include "cuserprofilewidget.h"
#include "cdownloadwidget.h"
#include "cpushbutton.h"
#include <QLabel>

#define APP_NAME "DesktopEditors"
#define GET_REGISTRY_USER(variable) \
    QSettings variable(QSettings::NativeFormat, QSettings::UserScope, "ONLYOFFICE", APP_NAME);

struct CPrintData;

class QMainPanel : public QWinWidget, public NSEditorApi::CAscMenuEventListener
{
    Q_OBJECT

public:
    QMainPanel( HWND hWnd, CAscApplicationManager* pManager );

    bool nativeEvent(const QByteArray &, void *msg, long *result);

    void mousePressEvent( QMouseEvent *event );
    void resizeEvent(QResizeEvent* event);
    void loadStartPage();
    void toggleButtonMain(bool);
    void checkModified(byte action);

    void focus();
signals:
    void downloadEvent(NSEditorApi::CAscDownloadFileInfo *);

public slots:
    void pushButtonMinimizeClicked();
    void pushButtonMaximizeClicked();
    void pushButtonCloseClicked();
    void pushButtonMainClicked();

    void onTabClicked(int);
    void onTabChanged(int);
    void onTabClosed(int, int);
    void onTabCloseRequest(int);
    void onMenuLogout();
    void onAddEditor(bool, int);
    void onLogin(QString);
    void onLogout();
    void onJSMessage(QString, QString);
    void onDocumentPrint(void *);
    void onDialogSave(std::wstring sName);
    void onFullScreen(bool);
    void onKeyDown(void *);

private:
    HWND windowHandle;

    std::wstring    m_sDownloadName;

    QPushButton*    m_pButtonMain;
    QWidget*        m_pMainWidget;

    QPushButton*    m_pButtonMinimize;
    QPushButton*    m_pButtonMaximize;
    QPushButton*    m_pButtonClose;
    QPushButton*    m_pButtonProfile;
    CPushButton*    m_pButtonDownload;

    QHBoxLayout *   m_layoutBtns;
    QWidget *       m_boxTitleBtns;
    CAscTabWidget * m_pTabs;
    bool            m_isMaximized;
    CPrintData *    m_printData;

    QWidget*        m_pSeparator;
    CUserProfileWidget *    m_pWidgetProfile;
    CDownloadWidget *       m_pWidgetDownload;

    CAscApplicationManager* m_pManager;

public:
    HWND GetHwndForKeyboard()
    {
        return (HWND)((QWidget*)m_pTabs->parent())->winId();
    }

public:
    virtual void OnEvent(NSEditorApi::CAscMenuEvent* pEvent);
    virtual bool IsSupportEvent(int nEventType)
    {
        Q_UNUSED(nEventType);
        return true;
    }

private:
    void RecalculatePlaces();
};

#endif // QMAINPANEL_H
