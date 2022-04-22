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


#include "cpresenterwindow.h"
#include "cascapplicationmanagerwrapper.h"
#include "defines.h"
#include "utils.h"
#include "csplash.h"
#include "clogger.h"
#include "clangater.h"
#include <QDesktopWidget>
#include <QGridLayout>
#include <QTimer>
#include <stdexcept>
#include <functional>
#include <QApplication>
#include <QIcon>

#ifdef _WIN32
# include "win/caption.h"
#endif

using namespace std::placeholders;


CPresenterWindow::CPresenterWindow(const QRect &rect, const QString &title, QCefView *view) :
    CWindowPlatform(rect, WindowType::REPORTER)
{    
    m_pMainPanel = createMainPanel(centralWidget(), title, true, static_cast<QWidget*>(view));
}

CPresenterWindow::~CPresenterWindow()
{

}

/** Public **/

void CPresenterWindow::applyTheme(const std::wstring& theme)
{
    CWindowPlatform::applyTheme(theme);
    m_pMainPanel->setProperty("uitheme", QString::fromStdWString(theme));
    m_labelTitle->style()->polish(m_labelTitle);
    m_buttonMinimize->style()->polish(m_buttonMinimize);
    m_buttonMaximize->style()->polish(m_buttonMaximize);
    m_buttonClose->style()->polish(m_buttonClose);
    m_boxTitleBtns->style()->polish(m_boxTitleBtns);
    m_pMainPanel->style()->polish(m_pMainPanel);
    update();
}

bool CPresenterWindow::holdView(int id) const
{
    return ((QCefView *)m_pMainView)->GetCefView()->GetId() == id;
}

/** Private **/

QWidget * CPresenterWindow::createMainPanel(QWidget * parent, const QString& title, bool custom, QWidget * view)
{
    QWidget * mainPanel = new QWidget(parent);
#ifndef __linux__
    parent->layout()->addWidget(mainPanel);
#endif
    mainPanel->setObjectName("mainPanel");
    mainPanel->setProperty("uitheme", QString::fromStdWString(AscAppManager::themes().current().id()));
    mainPanel->setStyleSheet(AscAppManager::getWindowStylesheets(m_dpiRatio));

    QGridLayout * mainGridLayout = new QGridLayout(mainPanel);
    mainGridLayout->setSpacing(0);
    mainGridLayout->setMargin(0);
    mainPanel->setLayout(mainGridLayout);

#ifdef __linux__
    m_boxTitleBtns = new QWidget(mainPanel);
#else
    m_boxTitleBtns = static_cast<QWidget*>(new Caption(mainPanel));
#endif
    m_boxTitleBtns->setObjectName("box-title-tools");
    m_boxTitleBtns->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_boxTitleBtns->setFixedHeight(TOOLBTN_HEIGHT * m_dpiRatio);

    QHBoxLayout * layoutBtns = new QHBoxLayout(m_boxTitleBtns);
    layoutBtns->setContentsMargins(0,0,0,0);
    layoutBtns->setSpacing(1*m_dpiRatio);
    m_boxTitleBtns->setLayout(layoutBtns);

    m_labelTitle = new CElipsisLabel(title, m_boxTitleBtns);
    m_labelTitle->setObjectName("labelAppTitle");
    m_labelTitle->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    layoutBtns->addWidget(m_labelTitle);

    if (custom) {
        initTopButtons(m_boxTitleBtns);
        layoutBtns->addWidget(m_buttonMinimize);
        layoutBtns->addWidget(m_buttonMaximize);
        layoutBtns->addWidget(m_buttonClose);
#ifdef __linux__
        m_labelTitle->setMouseTracking(true);
        mainGridLayout->setMargin(CX11Decoration::customWindowBorderWith() * m_dpiRatio);
        /*QPalette _palette(palette());
        _palette.setColor(QPalette::Background, QColor("#f1f1f1"));
        setAutoFillBackground(true);
        setPalette(_palette);
        setStyleSheet("QMainWindow{border:1px solid #888;}");*/
#else
        QSize small_btn_size(int(TOOLBTN_WIDTH*m_dpiRatio), int(TOOLBTN_HEIGHT*m_dpiRatio));
        QWidget * _lb = new QWidget(m_boxTitleBtns);
        _lb->setFixedWidth( (small_btn_size.width() + layoutBtns->spacing()) * 3 );
        layoutBtns->insertWidget(0, _lb);
#endif
    } else {
        QLinearGradient gradient(mainPanel->rect().topLeft(), QPoint(mainPanel->rect().left(), 29));
        gradient.setColorAt(0, QColor("#eee"));
        gradient.setColorAt(1, QColor("#e4e4e4"));
        m_labelTitle->setFixedHeight(0);
        m_boxTitleBtns->setFixedHeight(16*m_dpiRatio);
    }
    mainGridLayout->addWidget(m_boxTitleBtns, 0, 0, Qt::AlignTop);
    if (!view) {
        QCefView * pMainWidget = AscAppManager::createViewer(mainPanel);
        pMainWidget->Create(&AscAppManager::getInstance(), cvwtSimple);
        pMainWidget->setObjectName("mainPanel");
        pMainWidget->setHidden(false);
        m_pMainView = (QWidget*)pMainWidget;
    } else {
        m_pMainView = view;
        m_pMainView->setParent(mainPanel);
        m_pMainView->show();
    }
    m_pMainView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mainGridLayout->addWidget(m_pMainView, 1, 0);

    return mainPanel;
}

void CPresenterWindow::onMaximizeEvent()
{
    m_buttonMaximize->setProperty("class", isMaximized() ? "min" : "normal");
    m_buttonMaximize->style()->polish(m_buttonMaximize);
    CWindowBase::onMaximizeEvent();
}

void CPresenterWindow::onCloseEvent() // Reporter mode
{
    if (m_pMainView) {
        AscAppManager::getInstance().DestroyCefView(((QCefView *)m_pMainView)->GetCefView()->GetId() );
    }
}

#if defined (_WIN32)
void CPresenterWindow::focus()
{
    if (m_pMainView)
        ((QCefView *)m_pMainView)->setFocusToCef();
}
#endif
