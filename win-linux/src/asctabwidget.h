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

#ifndef ASCTABWIDGET
#define ASCTABWIDGET

#include <QResizeEvent>
//#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTabBar>
#include <QtWidgets/QPushButton>

#include "applicationmanager.h"
#include "qcefview.h"

#include <QDebug>

class CAscTabWidget : public QTabWidget
{
    Q_OBJECT

    struct CFullScreenData {
    public:
        CFullScreenData(int i, QWidget * w, void * d) : _widget(w), _index(i), _data(d) {}
        CFullScreenData() : CFullScreenData(-1, 0, 0) {}

        QWidget * widget() { return _widget; }
        int tabindex() { return _index; }
        void * data() { return _data; }
    private:
        QWidget * _widget;
        void * _data;
        int _index;
    };

public:
    QWidget* m_pMainWidget;
    QPushButton* m_pMainButton;
    CAscApplicationManager* m_pManager;

private:
    std::map<int, QCefView*> m_mapDownloads;
    CFullScreenData * m_dataFullScreen;

signals:
//    void sendAddEditor();
    void tabClosed(int, int);

public:
    CAscTabWidget(QWidget *parent = 0);

    int  addEditor(QString strName, AscEditorType etType = etDocument, std::wstring strUrl = L"");
    void closeEditorByIndex(int index, bool checkmodified = false);
    void closeAllEditors();

protected:
    void resizeEvent(QResizeEvent* e);

public:
    int         tabIndexByView(int);
    int         viewByIndex(int);
    QString     titleByIndex(int, bool original = true);
    bool        modifiedByIndex(int);
    int         modifiedCount();
    void        editorCloseRequest(int);

    void adjustTabsSize();
    void activate(bool);
    bool isActive();

    void updateIcons();
    void updateTabIcon(int);
    void setFocusedView(int index = -1);
    void setFullScreen(bool);

public slots:
    void onDocumentOpen(std::wstring, bool, int);
    void onDocumentNameChanged(int, QString);
    void onDocumentChanged(int, bool);
    void onDocumentSave(int);
    void onDocumentType(int, int);
};

#endif // ASCTABWIDGET

