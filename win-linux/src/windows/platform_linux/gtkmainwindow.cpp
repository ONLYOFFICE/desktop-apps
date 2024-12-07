#include "gtkmainwindow.h"
#include "platform_linux/xcbutils.h"
#include <QTimer>
#include <QX11Info>
#include <xcb/xcb.h>
#pragma push_macro("signals")
#undef signals
#include <gtk/gtk.h>
#include <gtk/gtkx.h>
#include <cairo.h>
#pragma pop_macro("signals")

#define WINDOW_CORNER_RADIUS 6


class GtkMainWindowPrivate
{
public:
    GtkMainWindowPrivate();
    ~GtkMainWindowPrivate();

    void init();

    QWidget *underlay = nullptr;
    GtkWidget *wnd = nullptr;
    guint state = 0;
    FnCloseEvent close_event;
    FnEvent event;
    QPoint pos, normalPos;
    QSize size, normalSize;
    bool is_maximized = false,
         is_focused = false,
         is_support_round_corners = false;

private:
    static gboolean on_event(GtkWidget *wgt, GdkEvent *ev, gpointer data);
    static void on_event_after(GtkWidget *wgt, GdkEvent *ev, gpointer data);
    static void set_rounded_corners(GtkWidget *wgt, double rad);
    static void on_size_allocate(GtkWidget *wgt, GdkRectangle *alloc, gpointer data);
    static void on_size_allocate_top(GtkWidget *wgt, GdkRectangle*, gpointer data);
};

GtkMainWindowPrivate::GtkMainWindowPrivate()
{}

GtkMainWindowPrivate::~GtkMainWindowPrivate()
{
    gtk_widget_destroy(GTK_WIDGET(wnd));
    wnd = nullptr;
}

void GtkMainWindowPrivate::init()
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
    /* Call the show according to the GtkSocket documentation */
    gtk_widget_show(socket);
//    gtk_widget_set_name(socket, "socket");
    gtk_container_add(GTK_CONTAINER(wnd), socket);
    /* The following call is only necessary if one of
    the ancestors of the socket is not yet visible
    (according to the GtkSocket documentation) */
    gtk_widget_realize(socket);

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

gboolean GtkMainWindowPrivate::on_event(GtkWidget *wgt, GdkEvent *ev, gpointer data)
{
    GtkMainWindowPrivate *pimpl = (GtkMainWindowPrivate*)data;
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

void GtkMainWindowPrivate::on_event_after(GtkWidget *wgt, GdkEvent *ev, gpointer data)
{
    GtkMainWindowPrivate *pimpl = (GtkMainWindowPrivate*)data;
    switch (ev->type) {
    case GDK_CONFIGURE: {
        gint x = 0, y = 0;
        gint f = gtk_widget_get_scale_factor(wgt);
        gtk_window_get_position(GTK_WINDOW(pimpl->wnd), &x, &y);
        pimpl->pos = QPoint(f*x, f*y);
        if (!pimpl->is_maximized) {
            pimpl->normalPos = pimpl->pos;
        }
        XcbUtils::sendConfigureNotify(pimpl->underlay->winId(), f*x, f*y, pimpl->underlay->width(), pimpl->underlay->height());
        break;
    }
    case GDK_FOCUS_CHANGE: {
        if ((pimpl->is_focused = ev->focus_change.in) == 1)
            XcbUtils::sendNativeFocusTo(pimpl->underlay->winId(), 1);
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

void GtkMainWindowPrivate::set_rounded_corners(GtkWidget *wgt, double rad)
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

void GtkMainWindowPrivate::on_size_allocate(GtkWidget *wgt, GdkRectangle *alloc, gpointer data)
{
    GtkMainWindowPrivate *pimpl = (GtkMainWindowPrivate*)data;
    gint f = gtk_widget_get_scale_factor(wgt);
    pimpl->underlay->resize(f*alloc->width, f*alloc->height);
    set_rounded_corners(wgt, pimpl->is_maximized ? 0 : 1.18 * WINDOW_CORNER_RADIUS);
    pimpl->underlay->update();
}

void GtkMainWindowPrivate::on_size_allocate_top(GtkWidget *wgt, GdkRectangle*, gpointer data)
{
    GtkMainWindowPrivate *pimpl = (GtkMainWindowPrivate*)data;
    gint w = 0, h = 0;
    gint f = gtk_widget_get_scale_factor(wgt);
    gtk_window_get_size(GTK_WINDOW(pimpl->wnd), &w, &h);
    pimpl->size = QSize(f*w, f*h);
    if (!pimpl->is_maximized) {
        pimpl->normalSize = pimpl->size;
    }
}


GtkMainWindow::GtkMainWindow(QWidget *underlay, const FnEvent &qev, const FnCloseEvent &qcev) :
    pimpl(new GtkMainWindowPrivate)
{
    pimpl->event = qev;
    pimpl->close_event = qcev;
    pimpl->underlay = underlay;
    pimpl->underlay->createWinId();
    pimpl->init();
}

GtkMainWindow::~GtkMainWindow()
{
    delete pimpl, pimpl = nullptr;
}

void GtkMainWindow::move(const QPoint &pos)
{
    gint f = gtk_widget_get_scale_factor(pimpl->wnd);
    gtk_window_move(GTK_WINDOW(pimpl->wnd), pos.x()/f, pos.y()/f);
    gdk_window_process_all_updates();
}

void GtkMainWindow::setGeometry(const QRect &rc)
{
    gint f = gtk_widget_get_scale_factor(pimpl->wnd);
    gtk_window_resize(GTK_WINDOW(pimpl->wnd), rc.width()/f, rc.height()/f);
    gtk_window_move(GTK_WINDOW(pimpl->wnd), rc.x()/f, rc.y()/f);
    gdk_window_process_all_updates();
}

void GtkMainWindow::setWindowIcon(const QIcon &icon)
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

void GtkMainWindow::setWindowTitle(const QString &title)
{
    gtk_window_set_title(GTK_WINDOW(pimpl->wnd), title.toLocal8Bit().constData());
}

void GtkMainWindow::setBackgroundColor(const QString &color)
{
    GdkRGBA c;
    gdk_rgba_parse(&c, color.toLocal8Bit().constData());
    gtk_widget_override_background_color(pimpl->wnd, GTK_STATE_FLAG_NORMAL, &c);
}

void GtkMainWindow::setFocus()
{
    gtk_window_present(GTK_WINDOW(pimpl->wnd));
    XcbUtils::sendNativeFocusTo(pimpl->underlay->winId(), 1);
}

void GtkMainWindow::setWindowState(Qt::WindowStates ws)
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

void GtkMainWindow::show()
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
    pimpl->underlay->setProperty("gtk_window_xid", QVariant::fromValue(xid));
    pimpl->underlay->show();
    //qApp->processEvents();
}

void GtkMainWindow::showMinimized()
{
    gtk_window_iconify(GTK_WINDOW(pimpl->wnd));
}

void GtkMainWindow::showMaximized()
{
    gtk_window_maximize(GTK_WINDOW(pimpl->wnd));
}

void GtkMainWindow::showNormal()
{
    if (gtk_window_is_maximized(GTK_WINDOW(pimpl->wnd)))
        gtk_window_unmaximize(GTK_WINDOW(pimpl->wnd));
    // gtk_window_present(GTK_WINDOW(pimpl->wnd));
}

void GtkMainWindow::activateWindow()
{
    gtk_window_present(GTK_WINDOW(pimpl->wnd));
}

void GtkMainWindow::setMinimumSize(int w, int h)
{
    gint f = gtk_widget_get_scale_factor(pimpl->wnd);
    gtk_widget_set_size_request(pimpl->wnd, w/f, h/f);
    gdk_window_process_all_updates();
}

void GtkMainWindow::hide() const
{
    gtk_widget_hide(pimpl->wnd);
}

bool GtkMainWindow::isMaximized()
{
    return gtk_window_is_maximized(GTK_WINDOW(pimpl->wnd));
}

bool GtkMainWindow::isMinimized()
{
    return pimpl->state & GDK_WINDOW_STATE_ICONIFIED;
}

bool GtkMainWindow::isActiveWindow()
{
    return gtk_window_is_active(GTK_WINDOW(pimpl->wnd));
}

bool GtkMainWindow::isFocused()
{
    return pimpl->is_focused;
}

bool GtkMainWindow::isVisible() const
{
    return gtk_widget_is_visible(pimpl->wnd);
}

bool GtkMainWindow::isHidden() const
{
    return !gtk_widget_is_visible(pimpl->wnd);
}

QString GtkMainWindow::windowTitle() const
{
    return gtk_window_get_title(GTK_WINDOW(pimpl->wnd));
}

QPoint GtkMainWindow::mapToGlobal(const QPoint &pt) const
{
    return (pt + geometry().topLeft());
}

QPoint GtkMainWindow::mapFromGlobal(const QPoint &pt) const
{
    return (pt - geometry().topLeft());
}

QSize GtkMainWindow::size() const
{
    return pimpl->size;
}

QRect GtkMainWindow::geometry() const
{
    return QRect(pimpl->pos, pimpl->size);
}

QRect GtkMainWindow::normalGeometry() const
{
    return QRect(pimpl->normalPos, pimpl->normalSize);
}

Qt::WindowStates GtkMainWindow::windowState() const
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
