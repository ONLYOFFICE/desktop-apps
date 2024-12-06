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

#include "windows/platform_linux/cwindowplatform.h"
#include "cascapplicationmanagerwrapper.h"
#include "defines.h"
#include "utils.h"
#pragma push_macro("signals")
#undef signals
#include <gtk/gtk.h>
#include <gtk/gtkx.h>
#include <cairo.h>
#pragma pop_macro("signals")
#include <QTimer>
#include <QPainter>
#include <QX11Info>
#include <xcb/xcb.h>

#ifdef DOCUMENTSCORE_OPENSSL_SUPPORT
# include "platform_linux/cdialogopenssl.h"
#endif
#define WINDOW_CORNER_RADIUS 6

typedef std::function<bool(QEvent *ev)> FnEvent;
typedef std::function<void(QCloseEvent*)> FnCloseEvent;


static void sendConfigureNotify(QWidget *wgt, int x, int y, int width, int height)
{
    xcb_connection_t* con = QX11Info::connection();
    xcb_window_t wnd = (xcb_window_t)wgt->winId();
    xcb_configure_notify_event_t ev;
    memset(&ev, 0, sizeof(ev));
    ev.response_type = XCB_CONFIGURE_NOTIFY;
    ev.event = wnd;
    ev.window = wnd;
    ev.x = x;
    ev.y = y;
    ev.width = width;
    ev.height = height;
    ev.border_width = 0;
    ev.above_sibling = XCB_WINDOW_NONE;
    ev.override_redirect = 0;
    xcb_send_event(con, 0, wnd, XCB_EVENT_MASK_STRUCTURE_NOTIFY, (const char*)&ev);
    xcb_flush(con);
}

static void sendFocusIn(QWidget *wgt, int focus)
{
    xcb_connection_t* con = QX11Info::connection();
    xcb_window_t wnd = (xcb_window_t)wgt->winId();
    xcb_client_message_event_t ev;
    memset(&ev, 0, sizeof(ev));
    ev.response_type = /*(focus == 1) ?*/ XCB_FOCUS_IN /*: XCB_FOCUS_OUT*/;
    ev.window = wnd;
    ev.type = XCB_INPUT_FOCUS_POINTER_ROOT;
    xcb_send_event(con, 0, wnd, XCB_EVENT_MASK_STRUCTURE_NOTIFY, (const char*)&ev);
    xcb_flush(con);
}

class GtkMainWindow
{
public:
    GtkMainWindow(QWidget *_underlay);
    ~GtkMainWindow();
    void init();

    QWidget *underlay = nullptr;
    GtkWidget *wnd = nullptr;
    guint state = 0;
    FnCloseEvent close_event;
    FnEvent event;
    QPoint pos, normalPos;
    QSize size, normalSize;
    bool is_maximized = false,
        is_support_round_corners = false;

private:
    static gboolean on_event(GtkWidget *wgt, GdkEvent *ev, gpointer data);
    static void on_event_after(GtkWidget *wgt, GdkEvent *ev, gpointer data);
    static void set_rounded_corners(GtkWidget *wgt, double rad);
    static void on_size_allocate(GtkWidget *wgt, GdkRectangle *alloc, gpointer data);
    static void on_size_allocate_top(GtkWidget *wgt, GdkRectangle*, gpointer data);
};

GtkMainWindow::GtkMainWindow(QWidget *_underlay) : underlay(_underlay)
{
    underlay->createWinId();
}

GtkMainWindow::~GtkMainWindow()
{
    gtk_widget_destroy(GTK_WIDGET(wnd));
    wnd = nullptr;
}

void GtkMainWindow::init()
{
    wnd = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_type_hint(GTK_WINDOW(wnd), GdkWindowTypeHint::GDK_WINDOW_TYPE_HINT_NORMAL);
//    gtk_window_set_title(GTK_WINDOW(wnd), "GtkMainWindow");
//    gtk_window_set_position(GTK_WINDOW(wnd), GtkWindowPosition::GTK_WIN_POS_CENTER);

    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider, "decoration {border-radius: 6px 6px 0px 0px;}", -1, NULL);
    GtkStyleContext *context = gtk_widget_get_style_context(wnd);
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    GtkWidget *header = gtk_header_bar_new();
    gtk_window_set_titlebar(GTK_WINDOW(wnd), header);
    gtk_widget_destroy(header);

    GtkWidget *socket = gtk_socket_new();
    gtk_widget_set_name(socket, "socket");
    gtk_widget_set_has_window(socket, TRUE);
    gtk_container_add(GTK_CONTAINER(wnd), socket);

    gtk_widget_set_app_paintable(wnd, TRUE);
    GdkScreen *scr = gtk_widget_get_screen(wnd);
    if (GdkVisual *vis = gdk_screen_get_rgba_visual(scr))
        gtk_widget_set_visual(wnd, vis);

    gtk_socket_add_id(GTK_SOCKET(socket), (Window)underlay->winId());
    g_signal_connect(G_OBJECT(socket), "size-allocate", G_CALLBACK(on_size_allocate), this);
    g_signal_connect(G_OBJECT(wnd), "size-allocate", G_CALLBACK(on_size_allocate_top), this);
    g_signal_connect(G_OBJECT(wnd), "event", G_CALLBACK(on_event), this);
    g_signal_connect(G_OBJECT(wnd), "event-after", G_CALLBACK(on_event_after), this);
}

gboolean GtkMainWindow::on_event(GtkWidget *wgt, GdkEvent *ev, gpointer data)
{
    GtkMainWindow *pimpl = (GtkMainWindow*)data;
    switch (ev->type) {
    case GDK_DELETE: {
        QCloseEvent qtcev;
        (pimpl->close_event)(&qtcev);
        return !qtcev.isAccepted();
    }
    default:
        break;
    }
    return FALSE;
}

void GtkMainWindow::on_event_after(GtkWidget *wgt, GdkEvent *ev, gpointer data)
{
    GtkMainWindow *pimpl = (GtkMainWindow*)data;
    switch (ev->type) {
    case GDK_CONFIGURE: {
        gint x = 0, y = 0;
        gint f = gtk_widget_get_scale_factor(wgt);
        gtk_window_get_position(GTK_WINDOW(pimpl->wnd), &x, &y);
        pimpl->pos = QPoint(f*x, f*y);
        if (!pimpl->is_maximized) {
            pimpl->normalPos = pimpl->pos;
        }
        sendConfigureNotify(pimpl->underlay, f*x, f*y, pimpl->underlay->width(), pimpl->underlay->height());
        break;
    }
    case GDK_FOCUS_CHANGE: {
        if (ev->focus_change.in == 1)
            sendFocusIn(pimpl->underlay, ev->focus_change.in);
            //qApp->postEvent(pimpl->cw, new QEvent(Event_GtkFocusIn));
        break;
    }
    case GDK_WINDOW_STATE: {
        guint state = guint(ev->window_state.new_window_state) & (GDK_WINDOW_STATE_ICONIFIED | GDK_WINDOW_STATE_MAXIMIZED | GDK_WINDOW_STATE_FULLSCREEN);
        if (pimpl->state != state) {
            pimpl->state = state;
            pimpl->is_maximized = state & GDK_WINDOW_STATE_MAXIMIZED;
            QEvent ev(QEvent::WindowStateChange);
            (pimpl->event)(&ev);
            //qApp->postEvent(pimpl->cw, new QEvent(QEvent::WindowStateChange));
        }
        break;
    }
    default:
        break;
    }
}

void GtkMainWindow::set_rounded_corners(GtkWidget *wgt, double rad)
{
    if (GdkWindow *gdk_window = gtk_widget_get_window(wgt)) {
        int w = gtk_widget_get_allocated_width(wgt);
        int h = gtk_widget_get_allocated_height(wgt);

        cairo_surface_t *sfc = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
        cairo_t *cr = cairo_create(sfc);
        cairo_set_source_rgba(cr, 1, 1, 1, 1);
        cairo_move_to(cr, rad, 0);
        // cairo_arc(cr, w - rad, rad, rad, -G_PI_2, 0);
        // cairo_arc(cr, w - rad, h - rad, rad, 0, G_PI_2);
        // cairo_arc(cr, rad, h - rad, rad, G_PI_2, G_PI);
        // cairo_arc(cr, rad, rad, rad, G_PI, -G_PI_2);
        cairo_arc(cr, w - rad, rad, rad, -G_PI_2, 0);
        cairo_line_to(cr, w, h);
        cairo_line_to(cr, 0, h);
        cairo_line_to(cr, 0, rad);
        cairo_arc(cr, rad, rad, rad, G_PI, -G_PI_2);
        cairo_close_path(cr);
        cairo_fill(cr);

        cairo_surface_t *sfc_tgt = cairo_get_target(cr);
        cairo_surface_flush(sfc_tgt);

        cairo_region_t *mask = gdk_cairo_region_create_from_surface(sfc_tgt);
        gdk_window_shape_combine_region(gdk_window, mask, 0, 0);

        cairo_region_destroy(mask);
        cairo_destroy(cr);
        cairo_surface_destroy(sfc);
    }
}

void GtkMainWindow::on_size_allocate(GtkWidget *wgt, GdkRectangle *alloc, gpointer data)
{
    GtkMainWindow *pimpl = (GtkMainWindow*)data;
    gint f = gtk_widget_get_scale_factor(wgt);
    pimpl->underlay->resize(f*alloc->width, f*alloc->height);
    set_rounded_corners(wgt, pimpl->is_maximized ? 0 : 1.18 * WINDOW_CORNER_RADIUS);
    pimpl->underlay->update();
}

void GtkMainWindow::on_size_allocate_top(GtkWidget *wgt, GdkRectangle*, gpointer data)
{
    GtkMainWindow *pimpl = (GtkMainWindow*)data;
    gint w = 0, h = 0;
    gint f = gtk_widget_get_scale_factor(wgt);
    gtk_window_get_size(GTK_WINDOW(pimpl->wnd), &w, &h);
    pimpl->size = QSize(f*w, f*h);
    if (!pimpl->is_maximized) {
        pimpl->normalSize = pimpl->size;
    }
}

CWindowPlatform::CWindowPlatform(const QRect &rect) :
    CWindowBase(rect),
    CX11Decoration(this)
{
#ifndef DONT_USE_GTK_MAINWINDOW
    pimpl = new GtkMainWindow(this);
    pimpl->event = std::bind(&CWindowPlatform::event, this, std::placeholders::_1);
    pimpl->close_event = std::bind(&CWindowPlatform::closeEvent, this, std::placeholders::_1);
    pimpl->init();

    setWindowIcon(Utils::appIcon());
    setMinimumSize(WINDOW_MIN_WIDTH * m_dpiRatio, WINDOW_MIN_HEIGHT * m_dpiRatio);
    setGeometry(m_window_rect);
#endif
    if (AscAppManager::isRtlEnabled())
        setLayoutDirection(Qt::RightToLeft);
    if (isCustomWindowStyle()) {
#ifdef DONT_USE_GTK_MAINWINDOW
        if (QX11Info::isCompositingManagerRunning())
            setAttribute(Qt::WA_TranslucentBackground);
#endif
        CX11Decoration::turnOff();
    }
    setIsCustomWindowStyle(!CX11Decoration::isDecorated());
    setFocusPolicy(Qt::StrongFocus);
    setProperty("stabilized", true);
    m_propertyTimer = new QTimer(this);
    m_propertyTimer->setSingleShot(true);
    m_propertyTimer->setInterval(100);
    connect(m_propertyTimer, &QTimer::timeout, this, [=]() {
        setProperty("stabilized", true);
    });
}

CWindowPlatform::~CWindowPlatform()
{
    if (pimpl)
        delete pimpl, pimpl = nullptr;
}

/** Public **/

void CWindowPlatform::bringToTop()
{
    if (isMinimized()) {
        windowState() == (Qt::WindowMinimized | Qt::WindowMaximized) ?
                    showMaximized() : showNormal();
    }
    CX11Decoration::raiseWindow();
}

void CWindowPlatform::show(bool maximized)
{
#ifdef DONT_USE_GTK_MAINWINDOW
    QMainWindow::show();
    if (maximized) {
        QMainWindow::setWindowState(Qt::WindowMaximized);
    }
#else
    show();
    if (maximized) {
        setWindowState(Qt::WindowMaximized);
    }
#endif
}

void CWindowPlatform::setWindowColors(const QColor& background, const QColor& border, bool isActive)
{
    Q_UNUSED(border)
    if (!CX11Decoration::isDecorated()) {
        m_brdColor = border;
#ifdef DONT_USE_GTK_MAINWINDOW
        setStyleSheet(QString("QMainWindow{border:1px solid %1; background-color: %2;}").arg(border.name(), background.name()));
#else
        setStyleSheet("QWidget#underlay{border: none; background: transparent;}");
        setBackgroundColor(background.name());
#endif
    }
}

void CWindowPlatform::adjustGeometry()
{
#ifdef DONT_USE_GTK_MAINWINDOW
    int border = (CX11Decoration::isDecorated() || isMaximized()) ? 0 : qRound(CX11Decoration::customWindowBorderWith() * m_dpiRatio);
    setContentsMargins(border, border, border, border);
#endif
}

#ifndef DONT_USE_GTK_MAINWINDOW
void CWindowPlatform::move(const QPoint &pos)
{
    gint f = gtk_widget_get_scale_factor(pimpl->wnd);
    gtk_window_move(GTK_WINDOW(pimpl->wnd), pos.x()/f, pos.y()/f);
    gdk_window_process_all_updates();
}

void CWindowPlatform::setGeometry(const QRect &rc)
{
    gint f = gtk_widget_get_scale_factor(pimpl->wnd);
    gtk_window_resize(GTK_WINDOW(pimpl->wnd), rc.width()/f, rc.height()/f);
    gtk_window_move(GTK_WINDOW(pimpl->wnd), rc.x()/f, rc.y()/f);
    gdk_window_process_all_updates();
}

void CWindowPlatform::setWindowIcon(const QIcon &icon)
{
    if (!icon.isNull()) {
        QImage img = icon.pixmap(96, 96).toImage().rgbSwapped();
        if (!img.isNull()) {
            if (GdkPixbuf *pb = gdk_pixbuf_new_from_data(img.constBits(), GDK_COLORSPACE_RGB, TRUE, 8, img.width(), img.height(),
                                                         img.bytesPerLine(), NULL, NULL)) {
                gtk_window_set_icon(GTK_WINDOW(pimpl->wnd), pb);
                g_object_unref(pb);
            }
        }
    }
}

void CWindowPlatform::setWindowTitle(const QString &title)
{
    gtk_window_set_title(GTK_WINDOW(pimpl->wnd), title.toLocal8Bit().constData());
    if (m_labelTitle)
        m_labelTitle->setText(title);
}

void CWindowPlatform::setBackgroundColor(const QString &color)
{
    GdkRGBA c;
    gdk_rgba_parse(&c, color.toLocal8Bit().constData());
    gtk_widget_override_background_color(pimpl->wnd, GTK_STATE_FLAG_NORMAL, &c);
}

void CWindowPlatform::setFocus()
{
    gtk_window_present(GTK_WINDOW(pimpl->wnd));
    sendFocusIn(this, 1);
}

void CWindowPlatform::setWindowState(Qt::WindowStates ws)
{
    if (ws.testFlag(Qt::WindowMaximized))
        gtk_window_maximize(GTK_WINDOW(pimpl->wnd));
    if (ws.testFlag(Qt::WindowMinimized))
        gtk_window_iconify(GTK_WINDOW(pimpl->wnd));
    if (ws.testFlag(Qt::WindowFullScreen))
        gtk_window_fullscreen(GTK_WINDOW(pimpl->wnd));
    if (ws.testFlag(Qt::WindowActive))
        gtk_window_present(GTK_WINDOW(pimpl->wnd));
}

void CWindowPlatform::show()
{
    gtk_widget_show_all(pimpl->wnd);
    while (gtk_events_pending())
        gtk_main_iteration_do(FALSE);
    //GdkDisplay *dsp = gdk_display_get_default();
    //gdk_display_sync(dsp);
    //gdk_display_flush(dsp);
    //gdk_window_process_all_updates();
    GdkWindow *gdk_wnd = gtk_widget_get_window(pimpl->wnd);
    Window xid = GDK_WINDOW_XID(gdk_wnd);
    setProperty("gtk_window_xid", QVariant::fromValue(xid));
    QMainWindow::show();
    //qApp->processEvents();
}

void CWindowPlatform::showMinimized()
{
    gtk_window_iconify(GTK_WINDOW(pimpl->wnd));
}

void CWindowPlatform::showMaximized()
{
    gtk_window_maximize(GTK_WINDOW(pimpl->wnd));
}

void CWindowPlatform::showNormal()
{
    if (gtk_window_is_maximized(GTK_WINDOW(pimpl->wnd)))
        gtk_window_unmaximize(GTK_WINDOW(pimpl->wnd));
    // gtk_window_present(GTK_WINDOW(pimpl->wnd));
}

void CWindowPlatform::activateWindow()
{
    gtk_window_present(GTK_WINDOW(pimpl->wnd));
}

void CWindowPlatform::setMinimumSize(int w, int h)
{
    gint f = gtk_widget_get_scale_factor(pimpl->wnd);
    gtk_widget_set_size_request(pimpl->wnd, w/f, h/f);
    gdk_window_process_all_updates();
}

void CWindowPlatform::hide() const
{
    gtk_widget_hide(pimpl->wnd);
}

bool CWindowPlatform::isMaximized()
{
    return gtk_window_is_maximized(GTK_WINDOW(pimpl->wnd));
}

bool CWindowPlatform::isMinimized()
{
    return pimpl->state & GDK_WINDOW_STATE_ICONIFIED;
}

bool CWindowPlatform::isActiveWindow()
{
    return gtk_window_is_active(GTK_WINDOW(pimpl->wnd));
}

bool CWindowPlatform::isVisible() const
{
    return gtk_widget_is_visible(pimpl->wnd);
}

bool CWindowPlatform::isHidden() const
{
    return !gtk_widget_is_visible(pimpl->wnd);
}

QString CWindowPlatform::windowTitle() const
{
    return gtk_window_get_title(GTK_WINDOW(pimpl->wnd));
}

QPoint CWindowPlatform::mapToGlobal(const QPoint &pt) const
{
    return QMainWindow::mapToGlobal(pt); //(pt + geometry().topLeft());
}

QPoint CWindowPlatform::mapFromGlobal(const QPoint &pt) const
{
    return QMainWindow::mapFromGlobal(pt); //(pt - geometry().topLeft());
}

QSize CWindowPlatform::size() const
{
    return pimpl->size;
}

QRect CWindowPlatform::geometry() const
{
    return QRect(pimpl->pos, pimpl->size);
}

QRect CWindowPlatform::normalGeometry() const
{
    return QRect(pimpl->normalPos, pimpl->normalSize);
}

Qt::WindowStates CWindowPlatform::windowState() const
{
    Qt::WindowStates ws;
    if (pimpl->state == 0)
        ws.setFlag(Qt::WindowNoState);
    if (pimpl->state & GDK_WINDOW_STATE_MAXIMIZED)
        ws.setFlag(Qt::WindowMaximized);
    if (pimpl->state & GDK_WINDOW_STATE_ICONIFIED)
        ws.setFlag(Qt::WindowMinimized);
    if (pimpl->state & GDK_WINDOW_STATE_FULLSCREEN)
        ws.setFlag(Qt::WindowFullScreen);
    //    if (pimpl->state & GDK_WINDOW_STATE_FOCUSED)
    //        ws.setFlag(Qt::WindowActive);
    return ws;
}
#endif

/** Protected **/
void CWindowPlatform::onMaximizeEvent()
{
    isMaximized() ? showNormal() : showMaximized();
}

void CWindowPlatform::onMinimizeEvent()
{
#ifdef DONT_USE_GTK_MAINWINDOW
    CX11Decoration::setMinimized();
#else
    setMinimized();
#endif
}

bool CWindowPlatform::event(QEvent * event)
{
    if (event->type() == QEvent::WindowStateChange) {
        CX11Decoration::setMaximized(isMaximized());
        applyWindowState();
        adjustGeometry();
    } else
    if (event->type() == QEvent::HoverLeave) {
        if (m_boxTitleBtns)
            m_boxTitleBtns->setCursor(QCursor(Qt::ArrowCursor));
    } else
    if (event->type() == QEvent::LayoutDirectionChange) {
        if (m_pMainPanel) {
            m_pMainPanel->setProperty("rtl", AscAppManager::isRtlEnabled());
            onLayoutDirectionChanged();
        }
    } else
    if (event->type() == QEvent::MouseMove) {
        if (!property("blocked").toBool()) {
            QMouseEvent *me = static_cast<QMouseEvent*>(event);
            CX11Decoration::dispatchMouseMove(me);
        }
    } else
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *me = static_cast<QMouseEvent*>(event);
        CX11Decoration::dispatchMouseDown(me);
    } else
    if (event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *me = static_cast<QMouseEvent*>(event);
        CX11Decoration::dispatchMouseUp(me);
    } else
    if (event->type() == QEvent::MouseButtonDblClick) {
        QMouseEvent *me = static_cast<QMouseEvent*>(event);
        if (m_boxTitleBtns && m_boxTitleBtns->geometry().contains(me->pos()))
            onMaximizeEvent();
    }
// #ifndef DONT_USE_GTK_MAINWINDOW
//     else
//     if (event->type() == Event_GtkFocusIn) {
//         if (isNativeFocus()) {
//             focus();
//             m_propertyTimer->stop();
//             if (property("stabilized").toBool())
//                 setProperty("stabilized", false);
//             m_propertyTimer->start();
//         }
//     }
// #endif
    return CWindowBase::event(event);
}

void CWindowPlatform::setScreenScalingFactor(double factor, bool resize)
{
    CX11Decoration::onDpiChanged(factor);
    CWindowBase::setScreenScalingFactor(factor, resize);
}

bool CWindowPlatform::nativeEvent(const QByteArray &ev_type, void *msg, long *res)
{
    if (ev_type == "xcb_generic_event_t") {
        xcb_generic_event_t *ev = static_cast<xcb_generic_event_t*>(msg);
        switch (ev->response_type & ~0x80) {
        case XCB_FOCUS_IN:
            if (isNativeFocus()) {
                focus();
                m_propertyTimer->stop();
                if (property("stabilized").toBool())
                    setProperty("stabilized", false);
                m_propertyTimer->start();
            }
            break;
        default:
            break;
        }
    }
    return CWindowBase::nativeEvent(ev_type, msg, res);
}

#ifdef DONT_USE_GTK_MAINWINDOW
void CWindowPlatform::paintEvent(QPaintEvent *event)
{
    CWindowBase::paintEvent(event);
    if (!QX11Info::isCompositingManagerRunning())
        return;

    QPainter pnt(this);
    pnt.setRenderHint(QPainter::Antialiasing);
    int d = 2 * WINDOW_CORNER_RADIUS * m_dpiRatio;
    QPainterPath path;
    path.moveTo(width(), d/2);
    path.arcTo(width() - d, 0, d, d, 0, 90);
    path.lineTo(d/2, 0);
    path.arcTo(0, 0, d, d, 90, 90);
    path.lineTo(0, height());
    path.lineTo(width(), height());
    path.lineTo(width(), d/2);
    path.closeSubpath();
    pnt.fillPath(path, palette().window().color());
    pnt.strokePath(path, QPen(m_brdColor, 1));
    pnt.end();
}
#else
void CWindowPlatform::applyWindowState()
{
    if (isCustomWindowStyle() && m_pTopButtons[BtnType::Btn_Maximize]) {
        m_pTopButtons[BtnType::Btn_Maximize]->setProperty("class", isMaximized() ? "min" : "normal") ;
        m_pTopButtons[BtnType::Btn_Maximize]->style()->polish(m_pTopButtons[BtnType::Btn_Maximize]);
    }
}

void CWindowPlatform::saveWindowState(const QString &baseKey)
{
    if (!windowState().testFlag(Qt::WindowFullScreen)) {
        GET_REGISTRY_USER(reg_user)
        reg_user.setValue(baseKey + "position", normalGeometry());
        if (windowState().testFlag(Qt::WindowMaximized)) {
            reg_user.setValue(baseKey + "maximized", true);
        } else {
            reg_user.remove(baseKey + "maximized");
        }
    }
}
#endif

/** Private **/
