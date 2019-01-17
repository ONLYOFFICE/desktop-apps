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

#ifndef CMAINPANEL_H
#define CMAINPANEL_H

#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QSettings>

#include "asctabwidget.h"
#include "cdownloadwidget.h"
#include "cpushbutton.h"
#include "ccefeventstransformer.h"
#include "cscalingwrapper.h"


struct printdata;

class CMainPanel : public QWidget, public CScalingWrapper
{
    Q_OBJECT

public:
    explicit CMainPanel(QWidget *parent, bool isCustomWindow, uchar scale);

    void applyMainWindowState(Qt::WindowState);

    void goStart();
    void focus();
    virtual void doOpenLocalFile(COpenOptions&);
    void doOpenLocalFiles(const vector<wstring> *);
    void doOpenLocalFiles(const QStringList&);
    void doOpenLocalFiles();
    void setInputFiles(QStringList *);
    void setScreenScalingFactor(uchar);
    bool holdUid(int) const;
    bool holdUrl(const QString&, AscEditorType) const;
    void toggleButtonMain(bool, bool delay = false);
    CAscTabWidget * tabWidget();

    void loadStartPage();
    virtual void updateScaling(int);

#ifdef __linux
    QWidget * getTitleWidget();
    void setMouseTracking(bool);
#endif

protected:
    virtual void refreshAboutVersion() = 0;
    virtual QString getSaveMessage();

private:
//    bool nativeEvent(const QByteArray &, void *msg, long *result);
//    void mousePressEvent( QMouseEvent *event );

    void resizeEvent(QResizeEvent* event);
//    bool eventFilter(QObject *obj, QEvent *event);

    void doLogout(const QString&, bool);
    int  trySaveDocument(int);
    void RecalculatePlaces();

signals:
//    void downloadEvent(NSEditorApi::CAscDownloadFileInfo *);
    void mainWindowChangeState(Qt::WindowState);
    void mainWindowClose();
    void mainPageReady();
    void checkUpdates();

public slots:
    void pushButtonMinimizeClicked();
    void pushButtonMaximizeClicked();
    void pushButtonCloseClicked();
    void pushButtonMainClicked();

    void onTabClicked(int);
    void onTabChanged(int);
    void onTabCloseRequest(int);
    void onAppCloseRequest();
    void onEditorActionRequest(int, const QString&);
    void onTabsCountChanged(int, int, int);

    void onCloudDocumentOpen(std::wstring, int, bool);
    virtual void onDocumentReady(int);
    void onDocumentType(int id, int type);
    void onDocumentName(void *);
    void onDocumentOptions(int id, QString);
    void onDocumentChanged(int id, bool changed);
    void onDocumentSave(int id, bool cancel);
    void onDocumentSaveInnerRequest(int id);
    void onDocumentDownload(void * info);
    void onDocumentLoadFinished(int);

    void onDocumentFragmented(int, bool);
    void onDocumentFragmentedBuild(int, int);

    virtual void onDocumentPrint(void *);
    void onDialogSave(std::wstring sName, uint id);
    void onFullScreen(bool apply, int id = -1);
    void onKeyDown(void *);

    void onNeedCheckKeyboard();

    virtual void onLocalOptions(const QString&){}
    virtual void onLocalFileOpen(const QString&);
    void onLocalFilesOpen(void *);
    virtual void onLocalFileCreate(int);
    void onLocalFileRecent(void *);
    virtual void onLocalFileSaveAs(void *);
    void onLocalFilesCheck(QString);
    void onLocalFileLocation(QString);
    void onLocalFileLocation(int, QString);
    void onLocalGetFile(int eventtype, void *);
    void onPortalOpen(QString);
    void onPortalLogin(int, QString);
    void onPortalLogout(QString);
    void onPortalNew(QString);
    void onPortalCreate();
    void onOutsideAuth(QString);

    void onEditorAllowedClose(int);

    void onFileChecked(const QString&, int, bool);
    void onCheckUpdates();

protected:
    CAscTabWidget * m_pTabs;
    QPushButton*    m_pButtonMain;
    bool            m_isCustomWindow;

private:
    std::wstring    m_sDownloadName;

    QWidget*        m_pMainWidget;

    QPushButton*    m_pButtonMinimize;
    QPushButton*    m_pButtonMaximize;
    QPushButton*    m_pButtonClose;
    QPushButton*    m_pButtonProfile;

    QHBoxLayout *   m_layoutBtns;
    QWidget *       m_boxTitleBtns;
    bool            m_isMaximized;

    CDownloadWidget *   m_pWidgetDownload = Q_NULLPTR;

    printdata *    m_printData;
    Qt::WindowState m_mainWindowState;

    QStringList * m_inFiles;

    QString m_savePortal;
    int m_saveAction;

public:
    WId GetHwndForKeyboard()
    {
        return ((QWidget*)m_pTabs->parent())->winId();
    }

};

#endif // CMAINPANEL_H
