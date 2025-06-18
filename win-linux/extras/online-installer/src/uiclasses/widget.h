#ifndef WIDGET_H
#define WIDGET_H

#include "object.h"
#include "drawingsurface.h"
#include "layout.h"
#include "commondefines.h"
#include <unordered_map>
#include <Windows.h>


class Widget : public Object, public DrawningSurface
{
public:
    Widget(Widget *parent = nullptr);
    virtual ~Widget();

    enum Properties : BYTE {
        HSizeBehavior,
        VSizeBehavior,
        PROPERTIES_COUNT
    };

    enum SizeBehavior : BYTE {
        Fixed,
        Expanding,
        //Preferred
    };

    virtual void setGeometry(int, int, int, int);
    void setDisabled(bool);
    void close();
    void move(int, int);
    void resize(int, int);
    Widget* parentWidget();
    std::wstring title();
    Size size();
    void size(int*, int*);
    void setWindowTitle(const std::wstring &title);
    void setProperty(Properties, int);
    void setFont(const std::wstring &font);
    void show();
    void hide();
    void repaint();
    void update();
    void setLayout(Layout *lut);
    bool isCreated();
    bool underMouse();
    int  property(Properties);
    Layout* layout();
    HWND nativeWindowHandle();
    static Widget* widgetFromHwnd(Widget *parent, HWND);

    /* callback */
    int onResize(const FnVoidIntInt &callback);
    int onMove(const FnVoidIntInt &callback);
    int onAboutToDestroy(const FnVoidVoid &callback);
    int onCreate(const FnVoidVoid &callback);
    int onClose(const FnVoidBoolPtr &callback);

    virtual void disconnect(int) override;

protected:
    friend class Application;
    Widget(Widget *parent, ObjectType type, HWND hWnd = nullptr, const Rect &rc = Rect(CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT));
    virtual bool event(UINT, WPARAM, LPARAM, LRESULT*);

    HWND         m_hWnd;
    HFONT        m_hFont;
    Layout      *m_layout;
    std::wstring m_title;
    bool         m_disabled;

private:
    void setNativeWindowHandle(HWND);

    int m_properties[PROPERTIES_COUNT];
    std::unordered_map<int, FnVoidIntInt> m_resize_callbacks,
                                          m_move_callbacks;
    std::unordered_map<int, FnVoidVoid>   m_create_callbacks,
                                          m_destroy_callbacks;
    std::unordered_map<int, FnVoidBoolPtr> m_close_callbacks;

    bool    m_is_created,
            m_is_destroyed,
            m_is_class_destroyed,
            m_mouse_entered;
};

#endif // WIDGET_H
