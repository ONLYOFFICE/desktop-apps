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

#include "windows/ceditorwindow.h"
#include "utils.h"
#include "defines.h"
#include "cascapplicationmanagerwrapper.h"
#include "components/cfiledialog.h"
#include "components/cmessage.h"
#include "../Common/OfficeFileFormats.h"
#include "common/Types.h"

#include <QGridLayout>
#include <QPushButton>
#include <QRegion>
#include <QLabel>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDir>

#include "windows/ceditorwindow_p.h"

#define CAPTURED_WINDOW_OFFSET_X  6*TOOLBTN_WIDTH + 10
#define CAPTURED_WINDOW_OFFSET_Y  15


CEditorWindow::CEditorWindow(const QRect& rect, CTabPanel* panel)
    : CWindowPlatform(rect)
    , d_ptr(new CEditorWindowPrivate(this))
{
    setObjectName("editorWindow");
    setWindowTitle("_");
    d_ptr.get()->init(panel);
    m_pMainPanel = createMainPanel(this, d_ptr->panel()->data()->title());
    setCentralWidget(m_pMainPanel);
#ifdef __linux__
    if (isCustomWindowStyle()) {
        CX11Decoration::setTitleWidget(m_boxTitleBtns);
        m_pMainPanel->setMouseTracking(true);
        setMouseTracking(true);
    }
#else
    recalculatePlaces();
#endif

    QTimer::singleShot(0, this, [=]{m_pMainView->show();});
    AscAppManager::bindReceiver(panel->cef()->GetId(), d_ptr.get());
    AscAppManager::sendCommandTo(panel->cef(), L"editor:config", L"request");

    QFileInfo i{QString::fromStdWString(panel->data()->url())};
    if ( i.suffix() == "oform" || panel->data()->hasFeature(L"uitype\":\"fillform") ) {
        d_ptr->ffWindowCustomize();
    }

    QTimer::singleShot(200, this, [=]() {
        if (d_ptr->canExtendTitle())
            setWindowTitle(panel->data()->title());
    });
}

CEditorWindow::~CEditorWindow()
{
    m_pMainPanel->deleteLater();
}

/** Public **/

const QObject * CEditorWindow::receiver()
{
    return d_ptr.get();
}

CTabPanel * CEditorWindow::releaseEditorView() const
{
    m_pMainView->clearMask();
    return qobject_cast<CTabPanel *>(m_pMainView);
}

AscEditorType CEditorWindow::editorType() const
{
    return d_ptr.get()->panel()->data()->contentType();
}

QString CEditorWindow::documentName() const
{
    return d_ptr.get()->panel()->data()->title(true);
}

double CEditorWindow::scaling() const
{
    return m_dpiRatio;
}

int CEditorWindow::closeWindow()
{
    d_ptr.get()->onFullScreen(false);

    CTabPanel * panel = d_ptr.get()->panel();

    int _reply = MODAL_RESULT_YES;
    if ( panel->data()->hasChanges() && !panel->data()->closed() ) {
        if (windowState() == Qt::WindowMinimized)
            showNormal();

        bringToTop();

        _reply = CMessage::showMessage(handle(),
                                       tr("%1 has been changed. Save changes?").arg(panel->data()->title(true)),
                                       MsgType::MSG_WARN, MsgBtns::mbYesDefNoCancel);
//            modal_res = mess.warning(getSaveMessage().arg(m_pTabs->titleByIndex(index)));
        switch (_reply) {
        case MODAL_RESULT_NO:
            _reply = MODAL_RESULT_YES;
            break;
        case MODAL_RESULT_CANCEL:
            return MODAL_RESULT_CANCEL;

        case MODAL_RESULT_YES:
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

bool CEditorWindow::closed() const
{
    return d_ptr.get()->panel()->data()->closed();
}

bool CEditorWindow::holdView(const std::wstring& portal) const
{
    return qobject_cast<CTabPanel *>(m_pMainView)->data()->url().find(portal) != std::wstring::npos;
}

void CEditorWindow::undock(bool maximized)
{
    if (isCustomWindowStyle()) {
        m_restoreMaximized = maximized;
        CWindowPlatform::show(false);
        captureMouse();
    } else {
        CWindowPlatform::show(maximized);
    }
}

bool CEditorWindow::holdView(int id) const
{
    return qobject_cast<CTabPanel *>(m_pMainView)->view()->GetCefView()->GetId() == id;
}

void CEditorWindow::applyTheme(const std::wstring& theme)
{
    d_ptr->changeTheme(theme);
}

/** Private **/

QWidget * CEditorWindow::createMainPanel(QWidget * parent, const QString& title)
{
    QWidget * mainPanel = new QWidget(parent);
    mainPanel->setObjectName("mainPanel");

    QGridLayout * mainGridLayout = new QGridLayout(mainPanel);
    mainGridLayout->setSpacing(0);
    mainGridLayout->setMargin(0);
    mainPanel->setLayout(mainGridLayout);

    if (isCustomWindowStyle()) {
        m_boxTitleBtns = createTopPanel(mainPanel);
        m_boxTitleBtns->setObjectName("box-title-tools");

        m_labelTitle = new CElipsisLabel(title, m_boxTitleBtns);
        m_labelTitle->setObjectName("labelTitle");
        m_labelTitle->setMouseTracking(true);
        m_labelTitle->setEllipsisMode(Qt::ElideMiddle);
        m_labelTitle->setAlignment(Qt::AlignCenter);
        m_labelTitle->setMinimumWidth(100);
        static_cast<QHBoxLayout*>(m_boxTitleBtns->layout())->insertWidget(0, m_labelTitle);
        if (d_ptr->usedOldEditorVersion)  // For old editors only
            static_cast<QHBoxLayout*>(m_boxTitleBtns->layout())->insertStretch(0);

        if ( !d_ptr->canExtendTitle() ) {
            mainGridLayout->addWidget(m_boxTitleBtns, 0, 0);
            m_labelTitle->setText(APP_TITLE);
        } else {
            if (d_ptr->panel()->data()->contentType() != etUndefined)
                mainPanel->setProperty("window", "pretty");
            int pos = (d_ptr->usedOldEditorVersion) ? 3 : 2;  // For old editors only
            auto *pIconSpacer = new QSpacerItem(ICON_SPACER_WIDTH, 5, QSizePolicy::Fixed, QSizePolicy::Fixed);
            auto *pTopLayout = static_cast<QHBoxLayout*>(m_boxTitleBtns->layout());
            pTopLayout->insertWidget(pos, d_ptr->iconUser());
            pTopLayout->insertSpacerItem(pos + 1, pIconSpacer);
        }

        d_ptr->customizeTitleLabel();
    } else {
//        QLinearGradient gradient(centralWidget->rect().topLeft(), QPoint(centralWidget->rect().left(), 29));
//        gradient.setColorAt(0, QColor("#eee"));
//        gradient.setColorAt(1, QColor("#e4e4e4"));
    }

    mainPanel->setProperty("zoom", QString::number(m_dpiRatio) + "x");
    mainPanel->setProperty("uitheme", QString::fromStdWString(GetCurrentTheme().id()));
    mainPanel->setStyleSheet(AscAppManager::getWindowStylesheets(m_dpiRatio) + m_css);

    if ( !d_ptr->panel() ) {
//        QCefView * pMainWidget = AscAppManager::createViewer(centralWidget);
//        pMainWidget->Create(&AscAppManager::getInstance(), cvwtSimple);
//        pMainWidget->setObjectName( "mainPanel" );
//        pMainWidget->setHidden(false);
//        m_pMainView = (QWidget *)pMainWidget;
    } else {
        m_pMainView = static_cast<QWidget*>(d_ptr->panel());
        m_pMainView->setParent(mainPanel);
        m_pMainView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }

    d_ptr.get()->onScreenScalingFactor(m_dpiRatio);
    if (d_ptr->usedOldEditorVersion)  // For old editors only
        mainGridLayout->addWidget(m_pMainView, 1, 0);
    else
        mainGridLayout->addWidget(m_pMainView, 1, 0, 1, 2);
    mainGridLayout->setRowStretch(1,1);

    if (d_ptr->canExtendTitle()) {
        if (d_ptr->usedOldEditorVersion)  // For old editors only
            mainGridLayout->addWidget(m_boxTitleBtns, 1, 0, Qt::AlignTop);
        else {
            m_pSpacer = new QSpacerItem(int(TOP_PANEL_OFFSET*m_dpiRatio), 5,
                                        QSizePolicy::Fixed, QSizePolicy::Fixed);
            mainGridLayout->addItem(m_pSpacer, 1, 0, Qt::AlignTop);
            mainGridLayout->addWidget(m_boxTitleBtns, 1, 1, Qt::AlignTop);
        }
    }
    return mainPanel;
}

CTabPanel * CEditorWindow::mainView() const
{
    return qobject_cast<CTabPanel *>(m_pMainView);
}

void CEditorWindow::recalculatePlaces()
{
    if ( !m_pMainView || !isCustomWindowStyle() ) return;

    if (!QCefView::IsSupportLayers()) {
        d_ptr->panel()->view()->SetCaptionMaskSize(int(TITLE_HEIGHT * m_dpiRatio));
    }

    if ( d_ptr->canExtendTitle() ) {
        m_pMainView->lower();
    }
}

void CEditorWindow::updateTitleCaption()
{
    if (m_labelTitle) {
//        int _width = calcTitleCaptionWidth();
//        if (_width >= 0) {
//            m_labelTitle->setMaximumWidth(_width);
            m_labelTitle->updateText();
//        }
    }
}

void CEditorWindow::onSizeEvent(int type)
{
    Q_UNUSED(type)
    updateTitleCaption();
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
    if ( m_restoreMaximized ) {
        m_restoreMaximized = false;
        CWindowPlatform::show(true);
    }
    if (!isActiveWindow())
        activateWindow();
}

void CEditorWindow::captureMouse()
{
    auto dpiCorr = [=](int val)->int {
        return int(val * m_dpiRatio);
    };
#ifdef _WIN32
    POINT cursor{0,0};
    if (GetCursorPos(&cursor)) {
        QRect _g{geometry()};
        int _window_offset_x;
        if (cursor.x - _g.x() < dpiCorr(CAPTURED_WINDOW_OFFSET_X))
            _window_offset_x = dpiCorr(CAPTURED_WINDOW_OFFSET_X);
        else
        if ( cursor.x > _g.right() - dpiCorr(150) )
            _window_offset_x = _g.right() - dpiCorr(150);
        else _window_offset_x = cursor.x - _g.x();
        move(cursor.x - _window_offset_x, cursor.y - dpiCorr(CAPTURED_WINDOW_OFFSET_Y));
        ReleaseCapture();
        PostMessage((HWND)winId(), WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(cursor.x, cursor.y));
    }
#else
    QMouseEvent _event(QEvent::MouseButtonRelease, QCursor::pos(), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(AscAppManager::mainWindow(), &_event);
    setGeometry(QRect(QCursor::pos() -
                      QPoint(dpiCorr(CAPTURED_WINDOW_OFFSET_X), dpiCorr(CAPTURED_WINDOW_OFFSET_Y)), size()));
    Q_ASSERT(m_boxTitleBtns != nullptr);
    QPoint pt_in_title = (m_boxTitleBtns->geometry().topLeft() +
                          QPoint(dpiCorr(CAPTURED_WINDOW_OFFSET_X), dpiCorr(CAPTURED_WINDOW_OFFSET_Y)));
    _event = {QEvent::MouseButtonPress, pt_in_title, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier};
    CX11Decoration::dispatchMouseDown(&_event);
    _event = {QEvent::MouseMove, QCursor::pos(), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier};
    CX11Decoration::dispatchMouseMove(&_event);
#endif
}

int CEditorWindow::calcTitleCaptionWidth()
{
    int base_width = (isCustomWindowStyle()) ? m_boxTitleBtns->width() -
                     (m_pTopButtons[BtnType::Btn_Maximize]->width() * 3) : 0;
    return d_ptr->calcTitleLabelWidth(base_width);
}

void CEditorWindow::focus()
{
    if (d_ptr->panel()->isReady())
        mainView()->view()->setFocusToCef();
}

void CEditorWindow::onCloseEvent()
{
    if ( m_pMainView ) {
        if ( closeWindow() == MODAL_RESULT_YES ) {
            CWindowBase::saveWindowState();
            hide();
        }
    }
}

void CEditorWindow::onMinimizeEvent()
{
    if ( !qobject_cast<CTabPanel *>(m_pMainView)->reporterMode() ) {
        CWindowPlatform::onMinimizeEvent();
    }
}

void CEditorWindow::onMaximizeEvent()
{
    if ( !qobject_cast<CTabPanel *>(m_pMainView)->reporterMode() ) {
        CWindowPlatform::onMaximizeEvent();
    }
}

bool CEditorWindow::event(QEvent * event)
{
    if (event->type() == QEvent::Resize) {
        onSizeEvent(0);
    } else
    if (event->type() == QEvent::User) {
        onExitSizeMove();
    } else
    if (event->type() == QEvent::Move) {
        QMoveEvent * _e = static_cast<QMoveEvent *>(event);
        onMoveEvent(QRect(_e->pos(), QSize(1,1)));
    }
    return CWindowPlatform::event(event);
}

void CEditorWindow::setScreenScalingFactor(double factor)
{
    CWindowPlatform::setScreenScalingFactor(factor);
    if (isCustomWindowStyle()) {
        m_boxTitleBtns->setFixedHeight(int(TOOLBTN_HEIGHT * factor));
        if (m_pSpacer && !d_ptr->usedOldEditorVersion) {
            m_pSpacer->changeSize(int(TOP_PANEL_OFFSET*m_dpiRatio), 5,
                                  QSizePolicy::Fixed, QSizePolicy::Fixed);
        }
    }
    QString zoom = QString::number(factor) + "x";
    m_pMainPanel->setProperty("zoom", zoom);

    QString css(AscAppManager::getWindowStylesheets(factor));
    css.append(m_css);
    m_pMainPanel->setStyleSheet(css);

    d_ptr.get()->onScreenScalingFactor(factor);
    recalculatePlaces();
    updateTitleCaption();
}

void CEditorWindow::onClickButtonHome()
{
    AscAppManager::gotoMainWindow(size_t(this));
}

void CEditorWindow::closeEvent(QCloseEvent * e)
{
    AscAppManager::getInstance().closeQueue().enter(sWinTag{CLOSE_QUEUE_WIN_TYPE_EDITOR, size_t(this)});
    e->ignore();
}
