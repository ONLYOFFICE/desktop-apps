#ifndef WINDOW_H
#define WINDOW_H

#include "widget.h"

#define DEFAULT_WINDOW_RECT Rect(100,100,1368,768)


struct FRAME {
    FRAME() : left(0), top(0)
    {}
    FRAME(FRAME &frame) {
        left = frame.left;
        top = frame.top;
    }
    int left, top;
};

class Window : public Widget
{
public:
    Window(Widget *parent = nullptr, const Rect &rc = DEFAULT_WINDOW_RECT);
    virtual ~Window();

    void setCentralWidget(Widget*);
    void setContentsMargins(int, int, int, int);
    void setResizable(bool);
    void showAll();
    void showNormal();
    void showMinimized();
    void showMaximized();
    void setIcon(int);
    void setLayout(Layout*) = delete;
    bool isMinimized();
    bool isMaximized();
    Widget *centralWidget();
    Layout *layout() = delete;

    /* callback */
    int onStateChanged(const FnVoidInt &callback);

    virtual void disconnect(int) override;

protected:
    virtual bool event(UINT, WPARAM, LPARAM, LRESULT*) override;

private:
    Widget  *m_centralWidget;
    Margins  m_contentMargins;
    COLORREF m_brdColor;
    int      m_brdWidth,
             m_resAreaWidth,
             m_state;
    double   m_dpi;
    FRAME    m_frame;
    bool     m_borderless,
             m_isResizable,
             m_isMaximized,
             m_isThemeActive,
             m_isTaskbarAutoHideOn,
             m_scaleChanged;
    Size     m_init_size;

    std::unordered_map<int, FnVoidInt> m_state_callbacks;
};

#endif // WINDOW_H
