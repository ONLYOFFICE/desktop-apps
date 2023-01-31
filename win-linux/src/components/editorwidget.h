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

#ifndef ASCEDITORWIDGET
#define ASCEDITORWIDGET

#include <QWidget>
#include <QTabWidget>
#include <QTabBar>
#include <QStyleOption>
#include <QPainter>
#include <QApplication>

namespace NSAscEditor
{
    enum EditorType
    {
        etDocument      = 0,
        etPresentation  = 1,
        etSpreadsheet   = 2
    };
}

#if 0
class CAscEditorWidget : public QWidget
{
    Q_OBJECT

public:
    NSAscEditor::EditorType m_etType;

public:
    CAscEditorWidget(QWidget *parent = 0, NSAscEditor::EditorType etType = NSAscEditor::etDocument) : QWidget(parent)
    {
        m_etType = etType;
        setStyleSheet("background-color:#FF00FF");
    }

    ~CAscEditorWidget()
    {
    }

    void SetIcon(int nIndex, QTabWidget* pWidget)
    {
        int nSelectedIndex = pWidget->tabBar()->currentIndex();

        QTabBar* pTabBar = pWidget->tabBar();

        bool bIsActive = (nIndex == nSelectedIndex) ? true : false;
        switch (m_etType)
        {
            case NSAscEditor::etPresentation:
            {
                pTabBar->setTabIcon(nIndex, bIsActive ? QIcon(":/Icons/tabicon_PE_active.png") : QIcon(":/Icons/tabicon_PE_normal.png"));
                break;
            }
            case NSAscEditor::etSpreadsheet:
            {
                pTabBar->setTabIcon(nIndex, bIsActive ? QIcon(":/Icons/tabicon_SE_active.png") : QIcon(":/Icons/tabicon_SE_normal.png"));
                break;
            }
            case NSAscEditor::etDocument:
            default:
            {
                pTabBar->setTabIcon(nIndex, bIsActive ? QIcon(":/Icons/tabicon_DE_active.png") : QIcon(":/Icons/tabicon_DE_normal.png"));
                break;
            }
        }

        pTabBar->setTabTextColor(nIndex, bIsActive ? QColor(255, 255, 255) : QColor(51, 51, 51));
    }

protected:
    void paintEvent(QPaintEvent* e)
    {
        Q_UNUSED(e);

        QStyleOption opt;
        opt.init(this);
        QPainter p(this);
        style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    }
};
#else
#include "./cef/qcefwebview.h"
class CAscEditorWidget : public QCefWebView
{
    Q_OBJECT

public:
    NSAscEditor::EditorType m_etType;

public:
    CAscEditorWidget(QWidget *parent = 0, NSAscEditor::EditorType etType = NSAscEditor::etDocument, std::wstring strUrl = L"") : QCefWebView(parent)
    {
        this->setParentTabs((QTabWidget*)parent);
        m_etType = etType;

        std::wstring sUrl = QApplication::applicationDirPath().toStdWString();
        sUrl += L"/deploy/apps/api/documents/index.html";

        if (etType == NSAscEditor::etSpreadsheet)
            sUrl += L"?doctype=spreadsheet";
        if (etType == NSAscEditor::etPresentation)
            sUrl += L"?doctype=presentation";

        if (strUrl.length() != 0)
            sUrl = strUrl;

        this->load(sUrl);
    }

    virtual ~CAscEditorWidget()
    {
    }

    void SetIcon(int nIndex, QTabWidget* pWidget)
    {
        int nSelectedIndex = pWidget->tabBar()->currentIndex();

        QTabBar* pTabBar = pWidget->tabBar();

        bool bIsActive = (nIndex == nSelectedIndex) ? true : false;

        switch (m_etType)
        {
            case NSAscEditor::etPresentation:
            {
                pTabBar->setTabIcon(nIndex, bIsActive ? QIcon(":/res/icons/tabicon_PE_active.png") : QIcon(":/res/icons/tabicon_PE_normal.png"));
                break;
            }
            case NSAscEditor::etSpreadsheet:
            {
                pTabBar->setTabIcon(nIndex, bIsActive ? QIcon(":/res/icons/tabicon_SE_active.png") : QIcon(":/res/icons/tabicon_SE_normal.png"));
                break;
            }
            case NSAscEditor::etDocument:
            default:
            {
                pTabBar->setTabIcon(nIndex, bIsActive ? QIcon(":/res/icons/tabicon_DE_active.png") : QIcon(":/res/icons/tabicon_DE_normal.png"));
                break;
            }
        }

        //pTabBar->setTabTextColor(nIndex, bIsActive ? QColor(255, 255, 255) : QColor(51, 51, 51));
        pTabBar->setTabTextColor(nIndex, QColor(51, 51, 51));
    }
};
#endif

#endif // ASCEDITORWIDGET

