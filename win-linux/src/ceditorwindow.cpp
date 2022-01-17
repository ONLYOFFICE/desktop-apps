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

#include "ceditorwindow.h"
#include "utils.h"
#include "cwindowbase.h"
#include "defines.h"
#include "cascapplicationmanagerwrapper.h"
#include "cfiledialog.h"
#include "cmessage.h"
#include "../Common/OfficeFileFormats.h"
#include "common/Types.h"

#include <QGridLayout>
#include <QPushButton>
#include <QRegion>
#include <QLabel>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDir>

#include "ceditorwindow_p.h"

CEditorWindow::CEditorWindow()
    : CSingleWindowPlatform(QRect(100, 100, 900, 800), "Desktop Editor", nullptr)
{
}

CEditorWindow::CEditorWindow(const QRect& rect, CTabPanel* panel)
    : CSingleWindowPlatform(rect, panel->data()->title(), panel)
    , d_ptr(new CEditorWindowPrivate(this))
{
    d_ptr.get()->init(panel);

#ifdef Q_OS_LINUX
    if ( !CX11Decoration::isDecorated() )
        applyTheme(AscAppManager::themes().current().id());

    setObjectName("editorWindow");
    m_pMainPanel = createMainPanel(this);
    setCentralWidget(m_pMainPanel);

    if ( !CX11Decoration::isDecorated() ) {
        CX11Decoration::setTitleWidget(m_boxTitleBtns);
        m_pMainPanel->setMouseTracking(true);
        setMouseTracking(true);
    }
#else

    applyTheme(AscAppManager::themes().current().id());

    m_pMainPanel = createMainPanel(m_pWinPanel);
    m_pWinPanel->show();

    recalculatePlaces();
#endif

    QTimer::singleShot(0, [=]{m_pMainView->show();});
    AscAppManager::bindReceiver(panel->cef()->GetId(), d_ptr.get());
    AscAppManager::sendCommandTo(panel->cef(), L"editor:config", L"request");

    QFileInfo i{QString::fromStdWString(panel->data()->url())};
    if ( i.suffix() == "oform" || panel->data()->hasFeature(L"uitype\":\"fillform") ) {
        d_ptr->ffWindowCustomize();
    }

//    QObject::connect(d_ptr.get()->buttonDock(), &QPushButton::clicked, [=]{
//        if ( !d_ptr->isReporterMode ) {
//            CAscApplicationManagerWrapper & app = static_cast<CAscApplicationManagerWrapper &>(AscAppManager::getInstance());
//            app.manageUndocking(
//                    qobject_cast<CTabPanel *>(m_pMainView)->view()->GetCefView()->GetId(), L"dock");
//        }
//    });
}

CEditorWindow::CEditorWindow(const QRect& r, const QString& s, QWidget * w)
    : CSingleWindowPlatform(r,s,w)
{

}

CEditorWindow::~CEditorWindow()
{
    m_pMainPanel->deleteLater();
}

bool CEditorWindow::holdView(int id) const
{
    return qobject_cast<CTabPanel *>(m_pMainView)->view()->GetCefView()->GetId() == id;
}

bool CEditorWindow::holdView(const std::wstring& portal) const
{
    return qobject_cast<CTabPanel *>(m_pMainView)->data()->url().find(portal) != std::wstring::npos;
}

void CEditorWindow::undock(bool maximized)
{
#ifdef Q_OS_LINUX
    maximized = false;
#else
    if ( maximized ) {
        m_restoreMaximized = true;
        maximized = false;
    }
#endif

    CSingleWindowPlatform::show(maximized);
    if ( isCustomWindowStyle() )
        CSingleWindowPlatform::captureMouse();
}

int CEditorWindow::closeWindow()
{
    d_ptr.get()->onFullScreen(false);

    CTabPanel * panel = d_ptr.get()->panel();

    int _reply = MODAL_RESULT_YES;
    if ( panel->data()->hasChanges() && !panel->data()->closed() ) {
        if (windowState() == Qt::WindowMinimized)
            setWindowState(Qt::WindowNoState);

        bringToTop();

        CMessage mess(handle(), CMessageOpts::moButtons::mbYesDefNoCancel);
//            modal_res = mess.warning(getSaveMessage().arg(m_pTabs->titleByIndex(index)));
        _reply = mess.warning(tr("%1 has been changed. Save changes?").arg(panel->data()->title(true)));

        switch (_reply) {
        case MODAL_RESULT_CUSTOM + 1:
            _reply = MODAL_RESULT_YES;
            break;
        case MODAL_RESULT_CANCEL:
        case MODAL_RESULT_CUSTOM + 2:
            return MODAL_RESULT_CANCEL;

        case MODAL_RESULT_CUSTOM + 0:
        default:
            panel->data()->close();
            panel->cef()->Apply(new CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_SAVE));

            _reply = MODAL_RESULT_NO;
            break;
        }
    }

    if ( _reply == MODAL_RESULT_YES ) {
        panel->data()->close();
        d_ptr.get()->onDocumentSave(panel->cef()->GetId());
    }

    return _reply;
}

QWidget * CEditorWindow::createMainPanel(QWidget * parent)
{
    return createMainPanel(parent, d_ptr->panel()->data()->title());
}

QWidget * CEditorWindow::createMainPanel(QWidget * parent, const QString& title)
{
    // create min/max/close buttons
    CSingleWindowPlatform::createMainPanel(parent, title);

    QWidget * mainPanel = new QWidget(parent);
    mainPanel->setObjectName("mainPanel");

    QGridLayout * mainGridLayout = new QGridLayout();
    mainGridLayout->setSpacing(0);
#ifdef Q_OS_WIN
    mainGridLayout->setMargin(0);
#else
    int b = !isCustomWindowStyle() ? 0 : CX11Decoration::customWindowBorderWith() * m_dpiRatio;
    mainGridLayout->setContentsMargins(QMargins(b,b,b,b));
#endif
    mainPanel->setLayout(mainGridLayout);
//    mainPanel->setStyleSheet(AscAppManager::getWindowStylesheets(m_dpiRatio));
//    mainPanel->setStyleSheet("background-color:#446995;");

    // Central widget
//    QWidget * centralWidget = new QWidget(mainPanel);
//    centralWidget->setObjectName("centralWidget");
//    centralWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    if ( m_dpiRatio > 1.75 )
        mainPanel->setProperty("zoom", "2x");
    else
    if ( m_dpiRatio > 1.5 )
        mainPanel->setProperty("zoom", "1.75x");
    else
    if ( m_dpiRatio > 1.25 )
        mainPanel->setProperty("zoom", "1.5x");
    else
    if ( m_dpiRatio > 1 )
        mainPanel->setProperty("zoom", "1.25x");

    mainPanel->setProperty("uitheme", QString::fromStdWString(AscAppManager::themes().current().id()));
    mainPanel->setStyleSheet(AscAppManager::getWindowStylesheets(m_dpiRatio) + m_css);

    if ( isCustomWindowStyle() ) {
        if ( !d_ptr->canExtendTitle() ) {
            mainGridLayout->addWidget(m_boxTitleBtns);
            m_labelTitle->setText(APP_TITLE);
        } else {
            if (d_ptr->panel()->data()->contentType() != etUndefined)
                mainPanel->setProperty("window", "pretty");
            m_boxTitleBtns->setParent(mainPanel);
            m_boxTitleBtns->layout()->addWidget(d_ptr.get()->iconUser());
        }

        m_boxTitleBtns->layout()->addWidget(m_buttonMinimize);
        m_boxTitleBtns->layout()->addWidget(m_buttonMaximize);
        m_boxTitleBtns->layout()->addWidget(m_buttonClose);

        d_ptr->customizeTitleLabel();

//        m_boxTitleBtns->setFixedSize(282*m_dpiRatio, TOOLBTN_HEIGHT*m_dpiRatio);

//        QWidget * _lb = new QWidget;
//        _lb->setFixedWidth( (small_btn_size.width() + layoutBtns->spacing()) * 3 );
//        layoutBtns->insertWidget(0, _lb);
    } else {
//        QLinearGradient gradient(centralWidget->rect().topLeft(), QPoint(centralWidget->rect().left(), 29));
//        gradient.setColorAt(0, QColor("#eee"));
//        gradient.setColorAt(1, QColor("#e4e4e4"));

//        label->setFixedHeight(0);
//        m_boxTitleBtns->setFixedSize(342*m_dpiRatio, 16*m_dpiRatio);
    }

    if ( !d_ptr->panel() ) {
//        QCefView * pMainWidget = AscAppManager::createViewer(centralWidget);
//        pMainWidget->Create(&AscAppManager::getInstance(), cvwtSimple);
//        pMainWidget->setObjectName( "mainPanel" );
//        pMainWidget->setHidden(false);

//        m_pMainView = (QWidget *)pMainWidget;
    } else {
        m_pMainView = d_ptr->panel();
        m_pMainView->setParent(mainPanel);

//        m_pMainView->setGeometry(mainPanel->geometry());
//        m_pMainView->show();
    }

//    m_pMainWidget->setVisible(false);

//    mainGridLayout->addWidget(centralWidget);
    d_ptr.get()->onScreenScalingFactor(m_dpiRatio);
    mainGridLayout->addWidget(m_pMainView, 1, 0);
    mainGridLayout->setRowStretch(1,1);
    return mainPanel;
}

void CEditorWindow::onCloseEvent()
{
    if ( m_pMainView ) {
        if ( closeWindow() == MODAL_RESULT_YES ) {
            hide();
        }
    }
}

void CEditorWindow::onMinimizeEvent()
{
    if ( !d_ptr->isReporterMode ) {
        CSingleWindowPlatform::onMinimizeEvent();
    }
}

void CEditorWindow::onClickButtonHome()
{
    AscAppManager::gotoMainWindow(size_t(this));
}

void CEditorWindow::onMaximizeEvent()
{
    if ( !d_ptr->isReporterMode ) {
        CSingleWindowPlatform::onMaximizeEvent();
    }
}

void CEditorWindow::onSizeEvent(int type)
{
    CSingleWindowPlatform::onSizeEvent(type);
    recalculatePlaces();
}

void CEditorWindow::onMoveEvent(const QRect&)
{
#ifdef Q_OS_WIN
    POINT pt{0,0};
    if ( ::GetCursorPos(&pt) ) {
        AscAppManager::editorWindowMoving((size_t)handle(), QPoint(pt.x,pt.y));
    }
#else
    AscAppManager::editorWindowMoving((size_t)handle(), QCursor::pos());
#endif
}

void CEditorWindow::onExitSizeMove()
{
    CSingleWindowPlatform::onExitSizeMove();

    if ( m_restoreMaximized ) {
        m_restoreMaximized = false;
        CSingleWindowPlatform::show(true);
    }
}

void CEditorWindow::onDpiChanged(double newfactor, double prevfactor)
{
#ifdef Q_OS_LINUX
    CX11Decoration::onDpiChanged(newfactor);
#endif

//    CSingleWindowPlatform::onDpiChanged(newfactor, prevfactor);
    setScreenScalingFactor(newfactor);
}

void CEditorWindow::setScreenScalingFactor(double newfactor)
{
    CSingleWindowPlatform::setScreenScalingFactor(newfactor);

    if ( newfactor > 1.75 ) m_pMainPanel->setProperty("zoom", "2x"); else
    if ( newfactor > 1.5 ) m_pMainPanel->setProperty("zoom", "1.75x"); else
    if ( newfactor > 1.25 ) m_pMainPanel->setProperty("zoom", "1.5x"); else
    if ( newfactor > 1 ) m_pMainPanel->setProperty("zoom", "1.25x");
    else m_pMainPanel->setProperty("zoom", "1");

    QString css(AscAppManager::getWindowStylesheets(newfactor));
    css.append(m_css);
    m_pMainPanel->setStyleSheet(css);


    d_ptr.get()->onScreenScalingFactor(newfactor);

    adjustGeometry();
    recalculatePlaces();
    updateTitleCaption();
}

void CEditorWindow::recalculatePlaces()
{
    if ( !m_pMainView || !isCustomWindowStyle() ) return;

    int windowW = m_pMainPanel->width(),
        windowH = m_pMainPanel->height(),
        captionH = int(TITLE_HEIGHT * m_dpiRatio);

    if (!QCefView::IsSupportLayers())
    {
        d_ptr->panel()->view()->SetCaptionMaskSize(int(TITLE_HEIGHT * m_dpiRatio));
    }

//    int contentH = windowH - captionH;
//    if ( contentH < 1 ) contentH = 1;

//    int nCaptionR = 200;
    int nCaptionL = 0 /*d_ptr.get()->titleLeftOffset * m_dpiRatio*/;

    if ( d_ptr->canExtendTitle() )
    {
//    QSize _s{TOOLBTN_WIDTH * 3, TOOLBTN_HEIGHT};
//    _s *= m_dpiRatio;
//    m_boxTitleBtns->setFixedWidth(_s.width());
#ifdef Q_OS_WIN
    m_boxTitleBtns->setGeometry(nCaptionL, 0, windowW - nCaptionL, captionH);
#else
    int cbw = CX11Decoration::customWindowBorderWith()*m_dpiRatio;
    m_boxTitleBtns->setGeometry(cbw, cbw, windowW - cbw * 2, captionH);
#endif
//    m_boxTitleBtns->move(windowW - m_boxTitleBtns->width() + cbw, cbw);
//    m_pMainView->setGeometry(0, captionH, windowW, windowH - captionH);

//    QRegion reg(0, captionH, windowW, windowH - captionH);
//    reg = reg.united(QRect(0, 0, nCaptionL, captionH));
//    reg = reg.united(QRect(windowW - nCaptionR, 0, nCaptionR, captionH));
//    m_pMainView->clearMask();
//    m_pMainView->setMask(reg);

    m_pMainView->lower();
    }
}

void CEditorWindow::focus()
{
    mainView()->view()->setFocusToCef();
}

void CEditorWindow::setReporterMode(bool apply)
{
    if ( apply ) {
    }

    d_ptr->isReporterMode = apply;
}

CTabPanel * CEditorWindow::mainView() const
{
    return qobject_cast<CTabPanel *>(m_pMainView);
}

CTabPanel * CEditorWindow::releaseEditorView() const
{
    m_pMainView->clearMask();
    return qobject_cast<CTabPanel *>(m_pMainView);
}

const QObject * CEditorWindow::receiver()
{
    return d_ptr.get();
}

QString CEditorWindow::documentName() const
{
    return d_ptr.get()->panel()->data()->title(true);
}

bool CEditorWindow::closed() const
{
    return d_ptr.get()->panel()->data()->closed();
}

AscEditorType CEditorWindow::editorType() const
{
    return d_ptr.get()->panel()->data()->contentType();
}

int CEditorWindow::calcTitleCaptionWidth()
{
    int base_width = CSingleWindowPlatform::calcTitleCaptionWidth();
    return d_ptr->calcTitleLabelWidth(base_width);
}

void CEditorWindow::applyTheme(const std::wstring& theme)
{
    d_ptr->changeTheme(theme);
}
