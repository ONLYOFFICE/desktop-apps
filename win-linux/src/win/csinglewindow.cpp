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

#include "csinglewindow.h"
#include "cascapplicationmanagerwrapper.h"
#include "../utils.h"
#include "cwindowbase.h"
#include "defines.h"

#include <windowsx.h>
#include <functional>
#include <QApplication>
#include <QDesktopWidget>
#include <QIcon>
#include <QLabel>
#include <QTimer>

Q_GUI_EXPORT HICON qt_pixmapToWinHICON(const QPixmap &);
using namespace std::placeholders;


CSingleWindow::CSingleWindow(const QRect& rect, const QString& title, QWidget * view)
{
    // adjust window size
    QRect _window_rect = rect;
    m_dpiRatio = Utils::getScreenDpiRatio(_window_rect.topLeft());

    if ( _window_rect.isEmpty() )
        _window_rect = QRect(100, 100, 1324 * m_dpiRatio, 800 * m_dpiRatio);

    QRect _screen_size = Utils::getScreenGeometry(_window_rect.topLeft());
    if ( _screen_size.width() < _window_rect.width() + 120 ||
            _screen_size.height() < _window_rect.height() + 120 )
    {
        _window_rect.setLeft(_screen_size.left()),
        _window_rect.setTop(_screen_size.top());

        if ( _screen_size.width() < _window_rect.width() ) _window_rect.setWidth(_screen_size.width());
        if ( _screen_size.height() < _window_rect.height() ) _window_rect.setHeight(_screen_size.height());
    }

    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASSEXW wcx{ sizeof(WNDCLASSEX) };
    wcx.style = CS_HREDRAW | CS_VREDRAW;
    wcx.hInstance = hInstance;
    wcx.lpfnWndProc = CSingleWindow::WndProc;
    wcx.cbClsExtra	= 0;
    wcx.cbWndExtra	= 0;
    wcx.lpszClassName = L"ReporterWindowClass";
    wcx.hbrBackground = CreateSolidBrush(WINDOW_BACKGROUND_COLOR);
    wcx.hCursor = LoadCursor( hInstance, IDC_ARROW );

    QIcon icon = Utils::appIcon();
    wcx.hIcon = qt_pixmapToWinHICON(QSysInfo::windowsVersion() == QSysInfo::WV_XP ?
                                        icon.pixmap(icon.availableSizes().first()) : icon.pixmap(QSize(32,32)) );

    if ( FAILED( RegisterClassExW( &wcx ) ) )
        throw std::runtime_error( "Couldn't register window class" );

    m_hWnd = CreateWindow( L"ReporterWindowClass", title.toStdWString().c_str(), static_cast<DWORD>(WindowBase::Style::windowed),
                            _window_rect.x(), _window_rect.y(), _window_rect.width(), _window_rect.height(), 0, 0, hInstance, nullptr );
    if ( !m_hWnd )
        throw std::runtime_error("couldn't create window because of reasons");

    SetWindowLongPtr( m_hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this) );
    setMinimumSize(WINDOW_MIN_WIDTH * m_dpiRatio, WINDOW_MIN_HEIGHT * m_dpiRatio);

    m_pWinPanel = new CWinPanel(m_hWnd);
    m_pMainPanel = createMainPanel(m_pWinPanel, title, true, view);

    m_pWinPanel->show();
    recalculatePlaces();
}

CSingleWindow::CSingleWindow(const QRect& rect)
    : CSingleWindow(rect, QString("ONLYOFFICE Editor"), AscAppManager::createViewer(nullptr))
{
}

CSingleWindow::~CSingleWindow()
{
    m_closed = true;

    if ( m_pWinPanel ) {
//        delete m_pWinPanel, m_pWinPanel = nullptr;
    }

    hide();
    DestroyWindow(m_hWnd);
}

LRESULT CALLBACK CSingleWindow::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    CSingleWindow * window = reinterpret_cast<CSingleWindow *>( GetWindowLongPtr(hWnd, GWLP_USERDATA) );
    if ( !window )
        return DefWindowProc(hWnd, message, wParam, lParam);

    switch ( message ) {
    case WM_KEYDOWN: {
        if ( wParam != VK_TAB )
            return DefWindowProc( hWnd, message, wParam, lParam );

        SetFocus( HWND(window->m_pWinPanel->winId()) );
        break;
    }

    // ALT + SPACE or F10 system menu
    case WM_SYSCOMMAND: {
        if ( wParam == SC_KEYMENU ) {
            return 0;
        }

        return DefWindowProc( hWnd, message, wParam, lParam );
    }

    case WM_SETFOCUS: {
        window->focusMainPanel();
        break;
    }

    case WM_NCCALCSIZE: {
        //this kills the window frame and title bar we added with
        //WS_THICKFRAME and WS_CAPTION
        if (window->m_borderless)
            return 0;

        break;
    }

    case WM_CLOSE: {
            QTimer::singleShot(0, window->m_pMainPanel, [=]{
                window->pushButtonCloseClicked();
            });
        }
        return 0;

    case WM_TIMER:
        CAscApplicationManagerWrapper::getInstance().CheckKeyboard();
        break;

    case WM_NCPAINT:
        return 0;

    case WM_NCHITTEST: {
        if ( window->m_borderless ) {
            const LONG borderWidth = 8; //in pixels
            RECT winrect;
            GetWindowRect( hWnd, &winrect );
            long x = GET_X_LPARAM( lParam );
            long y = GET_Y_LPARAM( lParam );

            if ( window->m_borderlessResizeable ) {
                //bottom left corner
                if ( x >= winrect.left && x < winrect.left + borderWidth &&
                    y < winrect.bottom && y >= winrect.bottom - borderWidth )
                {
                    return HTBOTTOMLEFT;
                }
                //bottom right corner
                if ( x < winrect.right && x >= winrect.right - borderWidth &&
                    y < winrect.bottom && y >= winrect.bottom - borderWidth )
                {
                    return HTBOTTOMRIGHT;
                }
                //top left corner
                if ( x >= winrect.left && x < winrect.left + borderWidth &&
                    y >= winrect.top && y < winrect.top + borderWidth )
                {
                    return HTTOPLEFT;
                }
                //top right corner
                if ( x < winrect.right && x >= winrect.right - borderWidth &&
                    y >= winrect.top && y < winrect.top + borderWidth )
                {
                    return HTTOPRIGHT;
                }
                //left border
                if ( x >= winrect.left && x < winrect.left + borderWidth )
                {
                    return HTLEFT;
                }
                //right border
                if ( x < winrect.right && x >= winrect.right - borderWidth )
                {
                    return HTRIGHT;
                }
                //bottom border
                if ( y < winrect.bottom && y >= winrect.bottom - borderWidth )
                {
                    return HTBOTTOM;
                }
                //top border
                if ( y >= winrect.top && y < winrect.top + borderWidth )
                {
                    return HTTOP;
                }
            }

            return HTCAPTION;
        }
        break;
    }

    case WM_SIZE:
        if ( !window->m_closed && window->m_pWinPanel ) {
            if (wParam == SIZE_MINIMIZED) {
                window->applyWindowState(Qt::WindowMinimized);
            } else {
                if ( wParam == SIZE_MAXIMIZED )
                    window->applyWindowState(Qt::WindowMaximized);  else
                    window->applyWindowState(Qt::WindowNoState);

                window->adjustGeometry();
            }

            window->recalculatePlaces();
        }
        break;

    case WM_EXITSIZEMOVE: {
        uchar dpi_ratio = Utils::getScreenDpiRatioByHWND(int(hWnd));

        if ( dpi_ratio != window->m_dpiRatio )
            window->setScreenScalingFactor(dpi_ratio);

        break;
    }

    case WM_NCACTIVATE:
        return TRUE;

    case WM_PAINT: {
        RECT rect;
        GetClientRect(hWnd, &rect);

        PAINTSTRUCT ps;
        HDC hDC = ::BeginPaint(hWnd, &ps);
        HPEN hpenOld = static_cast<HPEN>(::SelectObject(hDC, ::GetStockObject(DC_PEN)));
        ::SetDCPenColor(hDC, RGB(136, 136, 136));

        HBRUSH hBrush = ::CreateSolidBrush(WINDOW_BACKGROUND_COLOR);
        HBRUSH hbrushOld = static_cast<HBRUSH>(::SelectObject(hDC, hBrush));

        ::Rectangle(hDC, rect.left, rect.top, rect.right, rect.bottom);

        ::SelectObject(hDC, hbrushOld);
        ::DeleteObject(hBrush);

        ::SelectObject(hDC, hpenOld);
        ::EndPaint(hWnd, &ps);
        return 0;}

    case WM_ERASEBKGND: {
        return TRUE; }

    case WM_GETMINMAXINFO: {
        MINMAXINFO * minMaxInfo = (MINMAXINFO *)lParam;
        if ( window->minimumSize.required ) {
            minMaxInfo->ptMinTrackSize.x = window->getMinimumWidth();
            minMaxInfo->ptMinTrackSize.y = window->getMinimumHeight();
        }

        if ( window->maximumSize.required ) {
            minMaxInfo->ptMaxTrackSize.x = window->getMaximumWidth();
            minMaxInfo->ptMaxTrackSize.y = window->getMaximumHeight();
        }

        return 1;
    }

    case WM_ENDSESSION:
//        CAscApplicationManagerWrapper::getInstance().CloseApplication();

        break;

    case WM_WINDOWPOSCHANGING: { break; }
    default: break;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

void CSingleWindow::toggleBorderless(bool showmax)
{
    if ( m_visible ) {
        LONG newStyle = m_borderless ?
                    long(WindowBase::Style::aero_borderless) : long(WindowBase::Style::windowed)/* & ~WS_CAPTION*/;

        SetWindowLongPtr( m_hWnd, GWL_STYLE, newStyle );

        m_borderless = !m_borderless;

        //redraw frame
        SetWindowPos( m_hWnd, 0, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE /*| SWP_NOZORDER | SWP_NOOWNERZORDER */);
        show(showmax);
    }
}

void CSingleWindow::toggleResizeable()
{
    m_borderlessResizeable = !m_borderlessResizeable;
}

void CSingleWindow::show(bool maximized)
{
    ShowWindow(m_hWnd, maximized ? SW_MAXIMIZE : SW_SHOW);

    m_visible = true;
}

void CSingleWindow::hide()
{
    ShowWindow(m_hWnd, SW_HIDE);
    m_visible = false;
}

bool CSingleWindow::isVisible()
{
    return m_visible;
}

// Minimum size
void CSingleWindow::setMinimumSize( const int width, const int height )
{
    minimumSize.required = true;
    minimumSize.width = width;
    minimumSize.height = height;
}

void CSingleWindow::removeMinimumSize()
{
    minimumSize.required = false;
    minimumSize.width = 0;
    minimumSize.height = 0;
}

int CSingleWindow::getMinimumWidth() const
{
    return minimumSize.width;
}

int CSingleWindow::getMinimumHeight() const
{
    return minimumSize.height;
}

// Maximum size
void CSingleWindow::setMaximumSize( const int width, const int height )
{
    maximumSize.required = true;
    maximumSize.width = width;
    maximumSize.height = height;
}

void CSingleWindow::removeMaximumSize()
{
    maximumSize.required = false;
    maximumSize.width = 0;
    maximumSize.height = 0;
}
int CSingleWindow::getMaximumWidth()
{
    return maximumSize.width;
}
int CSingleWindow::getMaximumHeight()
{
    return maximumSize.height;
}

void CSingleWindow::adjustGeometry()
{
    RECT lpWindowRect, clientRect;
    GetWindowRect(m_hWnd, &lpWindowRect);
    GetClientRect(m_hWnd, &clientRect);

    int border_size = 0,
        nMaxOffsetX = 0,
        nMaxOffsetY = 0,
        nMaxOffsetR = 0,
        nMaxOffsetB = 0;

    if ( IsZoomed(m_hWnd) != 0 ) {      // is window maximized
        LONG lTestW = 640,
             lTestH = 480;

        RECT wrect{0,0,lTestW,lTestH};
        AdjustWindowRectEx(&wrect, (GetWindowStyle(m_hWnd) & ~WS_DLGFRAME), FALSE, 0);

        if (0 > wrect.left) nMaxOffsetX = -wrect.left;
        if (0 > wrect.top)  nMaxOffsetY = -wrect.top;

        if (wrect.right > lTestW)   nMaxOffsetR = (wrect.right - lTestW);
        if (wrect.bottom > lTestH)  nMaxOffsetB = (wrect.bottom - lTestH);

        // TODO: вот тут бордер!!!
        m_pWinPanel->setGeometry( nMaxOffsetX + border_size, nMaxOffsetY + border_size,
                                                    clientRect.right - (nMaxOffsetX + nMaxOffsetR + 2 * border_size),
                                                    clientRect.bottom - (nMaxOffsetY + nMaxOffsetB + 2 * border_size));
    } else {
        border_size = MAIN_WINDOW_BORDER_WIDTH * m_dpiRatio;

        // TODO: вот тут бордер!!!
        m_pWinPanel->setGeometry(border_size, border_size,
                            clientRect.right - 2 * border_size, clientRect.bottom - 2 * border_size);
    }

    HRGN hRgn = CreateRectRgn(nMaxOffsetX, nMaxOffsetY,
                                lpWindowRect.right - lpWindowRect.left - nMaxOffsetX,
                                lpWindowRect.bottom - lpWindowRect.top - nMaxOffsetY);

    SetWindowRgn(m_hWnd, hRgn, TRUE);
    DeleteObject(hRgn);
}

void CSingleWindow::setScreenScalingFactor(uchar factor)
{
    QString css(AscAppManager::getWindowStylesheets(factor));

    if ( !css.isEmpty() ) {
        bool increase = factor > m_dpiRatio;
        m_dpiRatio = factor;

        m_pMainPanel->setStyleSheet(css);
        /**/
//            if ( m_isCustomWindow ) {
                QSize small_btn_size(40*m_dpiRatio, TOOLBTN_HEIGHT*m_dpiRatio);

                m_pButtonMinimize->setFixedSize(small_btn_size);
                m_pButtonMaximize->setFixedSize(small_btn_size);
                m_pButtonClose->setFixedSize(small_btn_size);

                m_boxTitleBtns->setFixedSize(282*m_dpiRatio, TOOLBTN_HEIGHT*m_dpiRatio);
//            } else {
//                m_boxTitleBtns->setFixedSize(342*m_dpiRatio, 16*m_dpiRatio);
//            }

            QLayout * layoutBtns = m_boxTitleBtns->layout();
//            layoutBtns->setContentsMargins(0,0,0,0);
            layoutBtns->setSpacing(1*m_dpiRatio);
        /**/
        setMinimumSize(WINDOW_MIN_WIDTH * factor, WINDOW_MIN_HEIGHT * factor);

        RECT lpWindowRect;
        GetWindowRect(m_hWnd, &lpWindowRect);

        unsigned _new_width = lpWindowRect.right - lpWindowRect.left,
                _new_height = lpWindowRect.bottom - lpWindowRect.top;

        if ( increase )
            _new_width *= 2, _new_height *= 2;  else
            _new_width /= 2, _new_height /= 2;

        SetWindowPos(m_hWnd, NULL, 0, 0, _new_width, _new_height, SWP_NOMOVE | SWP_NOZORDER);
    }
}

QWidget * CSingleWindow::createMainPanel(QWidget * parent, const QString& title, bool custom, QWidget * view)
{
    QWidget * mainPanel = new QWidget(parent);
//    mainpanel->setObjectName("mainPanel");

    QGridLayout * mainGridLayout = new QGridLayout();
    mainGridLayout->setSpacing(0);
    mainGridLayout->setMargin(0);
    mainPanel->setLayout(mainGridLayout);
    mainPanel->setStyleSheet(AscAppManager::getWindowStylesheets(m_dpiRatio));

    // Central widget
    QWidget * centralWidget = new QWidget(mainPanel);
    centralWidget->setObjectName("centralWidget");
    centralWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

#ifdef __linux__
    m_boxTitleBtns = new CX11Caption(centralWidget);
#else
    m_boxTitleBtns = new QWidget(centralWidget);
#endif

    QHBoxLayout * layoutBtns = new QHBoxLayout(m_boxTitleBtns);
    QLabel * label = new QLabel(title);
    label->setObjectName("labelAppTitle");
    label->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

    layoutBtns->setContentsMargins(0,0,0,0);
    QSize small_btn_size(40*m_dpiRatio, TOOLBTN_HEIGHT*m_dpiRatio);

    layoutBtns->setSpacing(1*m_dpiRatio);
    layoutBtns->addWidget(label);

    if ( custom ) {
        auto _creatToolButton = [small_btn_size](const QString& name, QWidget * parent) {
            QPushButton * btn = new QPushButton(parent);
            btn->setObjectName(name);
            btn->setProperty("class", "normal");
            btn->setProperty("act", "tool");
            btn->setFixedSize(small_btn_size);

            return btn;
        };

        // Minimize
        m_pButtonMinimize = _creatToolButton("toolButtonMinimize", centralWidget);
        QObject::connect(m_pButtonMinimize, &QPushButton::clicked, bind(&CSingleWindow::pushButtonMinimizeClicked, this));

        // Maximize
        m_pButtonMaximize = _creatToolButton("toolButtonMaximize", centralWidget);
        QObject::connect(m_pButtonMaximize, &QPushButton::clicked, bind(&CSingleWindow::pushButtonMaximizeClicked, this));

        // Close
        m_pButtonClose = _creatToolButton("toolButtonClose", centralWidget);
        QObject::connect(m_pButtonClose, &QPushButton::clicked, bind(&CSingleWindow::pushButtonCloseClicked, this));

        layoutBtns->addWidget(m_pButtonMinimize);
        layoutBtns->addWidget(m_pButtonMaximize);
        layoutBtns->addWidget(m_pButtonClose);

#ifdef __linux__
        mainGridLayout->setMargin( CX11Decoration::customWindowBorderWith() );

        QPalette _palette(parent->palette());
        _palette.setColor(QPalette::Background, QColor(0x31, 0x34, 0x37));
        parent->setAutoFillBackground(true);
        parent->setPalette(_palette);

        connect(m_boxTitleBtns, SIGNAL(mouseDoubleClicked()), this, SLOT(pushButtonMaximizeClicked()));
#endif

//        m_boxTitleBtns->setFixedSize(282*m_dpiRatio, TOOLBTN_HEIGHT*m_dpiRatio);

        QWidget * _lb = new QWidget;
        _lb->setFixedWidth( (small_btn_size.width() + layoutBtns->spacing()) * 3 );
        layoutBtns->insertWidget(0, _lb);
    } else {
        QLinearGradient gradient(centralWidget->rect().topLeft(), QPoint(centralWidget->rect().left(), 29));
        gradient.setColorAt(0, QColor("#eee"));
        gradient.setColorAt(1, QColor("#e4e4e4"));

        label->setFixedHeight(0);
        m_boxTitleBtns->setFixedSize(342*m_dpiRatio, 16*m_dpiRatio);
    }

    if ( !view ) {
        QCefView * pMainWidget = AscAppManager::createViewer(centralWidget);
        pMainWidget->Create(&AscAppManager::getInstance(), cvwtSimple);
        pMainWidget->setObjectName( "mainPanel" );
        pMainWidget->setHidden(false);

        m_pMainView = (QWidget *)pMainWidget;
    } else {
        m_pMainView = view;
        m_pMainView->setParent(centralWidget);
        m_pMainView->show();
    }

//    m_pMainWidget->setVisible(false);

    mainGridLayout->addWidget( centralWidget );
    return mainPanel;
}

void CSingleWindow::recalculatePlaces()
{
    int cbw = 0;

#ifdef __linux
    QWidget * cw = findChild<QWidget *>("centralWidget");
    int windowW = cw->width(),
        windowH = cw->height(),
#else
    int windowW = m_pMainPanel->width(),
        windowH = m_pMainPanel->height(),
#endif
        captionH = TITLE_HEIGHT * m_dpiRatio;

    int contentH = windowH - captionH;
    if ( contentH < 1 ) contentH = 1;

    m_boxTitleBtns->setFixedSize(windowW, TOOLBTN_HEIGHT * m_dpiRatio);
    m_boxTitleBtns->move(windowW - m_boxTitleBtns->width() + cbw, cbw);
    m_pMainView->setGeometry(cbw, captionH + cbw, windowW, contentH);
}


void CSingleWindow::pushButtonCloseClicked()
{
    if ( m_pMainView ) {
        AscAppManager::getInstance().DestroyCefView(
                ((QCefView *)m_pMainView)->GetCefView()->GetId() );

//        m_pMainView = nullptr;
    }
}

void CSingleWindow::pushButtonMinimizeClicked()
{
    ShowWindow(m_hWnd, SW_MINIMIZE);
}

void CSingleWindow::pushButtonMaximizeClicked()
{
    bool _is_maximized = IsZoomed(m_hWnd) != 0;
//    if ( m_isCustomWindow )
    {
#ifdef __linux__
        layout()->setMargin(s == _is_maximized ? 0 : CX11Decoration::customWindowBorderWith());
#endif

        m_pButtonMaximize->setProperty("class", _is_maximized ? "min" : "normal");
        m_pButtonMaximize->style()->polish(m_pButtonMaximize);
    }

    ShowWindow(m_hWnd, _is_maximized ? SW_RESTORE : SW_MAXIMIZE );
}

void CSingleWindow::focusMainPanel()
{
    if ( m_pMainView ) ((QCefView *)m_pMainView)->setFocusToCef();
}

bool CSingleWindow::holdView(int id) const
{
    return ((QCefView *)m_pMainView)->GetCefView()->GetId() == id;
}

void CSingleWindow::applyWindowState(Qt::WindowState s)
{
//    if ( m_isCustomWindow )
    {
#ifdef __linux__
        layout()->setMargin(s == Qt::WindowMaximized ? 0 : CX11Decoration::customWindowBorderWith());
#endif

        m_pButtonMaximize->setProperty("class", s == Qt::WindowMaximized ? "min" : "normal") ;
        m_pButtonMaximize->style()->polish(m_pButtonMaximize);
    }
}

