#include "application.h"
#include "widget.h"
#include "src/resource.h"
#include <gdiplus.h>


class Application::ApplicationPrivate
{
public:
    ApplicationPrivate();
    ~ApplicationPrivate();

    ULONG_PTR gdi_token;
    HINSTANCE hInstance;
    LayoutDirection layoutDirection;
    int windowId;
    ATOM registerClass(LPCWSTR className, HINSTANCE hInstance);
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};

Application::ApplicationPrivate::ApplicationPrivate() :
    gdi_token(0),
    hInstance(nullptr),
    layoutDirection(LayoutDirection::LeftToRight),
    windowId(0)
{
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    Gdiplus::GdiplusStartup(&gdi_token, &gdiplusStartupInput, nullptr);
}

Application::ApplicationPrivate::~ApplicationPrivate()
{
    Gdiplus::GdiplusShutdown(gdi_token);
}

ATOM Application::ApplicationPrivate::registerClass(LPCWSTR className, HINSTANCE hInstance)
{
    WNDCLASSEX wcx;
    memset(&wcx, 0, sizeof(wcx));
    wcx.cbSize = sizeof(WNDCLASSEX);
    wcx.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wcx.hInstance = hInstance;
    wcx.lpfnWndProc = WndProc;
    wcx.cbClsExtra	= 0;
    wcx.cbWndExtra	= 0;
    // wcx.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAINICON));
    // wcx.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAINICON));
    wcx.lpszClassName = className;
    wcx.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wcx.hCursor = LoadCursor(NULL, IDC_ARROW);
    return RegisterClassEx(&wcx);
}

LRESULT CALLBACK Application::ApplicationPrivate::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_CREATE) {
        if (CREATESTRUCT *cs = (CREATESTRUCT*)lParam) {
            if (Widget *wgt = (Widget*)cs->lpCreateParams) {
                wgt->setNativeWindowHandle(hWnd);
                SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)wgt);
                LRESULT result = 0;
                if (wgt->event(msg, wParam, lParam, &result))
                    return result;
            }
        }
    } else
    if (Widget *wgt = (Widget*)GetWindowLongPtr(hWnd, GWLP_USERDATA)) {
         LRESULT result = 0;
         if (wgt->event(msg, wParam, lParam, &result))
             return result;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

Application *Application::inst = nullptr;

Application::Application(HINSTANCE hInstance, PWSTR cmdline, int cmdshow) :
    Application()
{
    d_ptr->hInstance = hInstance;
    if (!d_ptr->hInstance)
        d_ptr->hInstance = GetModuleHandle(NULL);
    inst = this;
}

Application::Application() :
    Object(nullptr),
    d_ptr(new ApplicationPrivate)
{

}

Application *Application::instance()
{
    return inst;
}

HINSTANCE Application::moduleHandle()
{
    return d_ptr->hInstance;
}

void Application::setLayoutDirection(LayoutDirection layoutDirection)
{
    d_ptr->layoutDirection = layoutDirection;
}

Application::~Application()
{
    delete d_ptr, d_ptr = nullptr;
}

int Application::exec()
{
    MSG msg;
    BOOL res;
    while ((res = GetMessage(&msg, NULL, 0, 0)) != 0 && res != -1) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}

void Application::exit(int code)
{
    PostQuitMessage(code);
}

void Application::registerWidget(Widget *wgt, ObjectType objType, const Rect &rc)
{
    std::wstring className;
    DWORD style = WS_CLIPCHILDREN;
    DWORD exStyle = d_ptr->layoutDirection == LayoutDirection::RightToLeft ? WS_EX_LAYOUTRTL : 0;
    HWND hWndParent = wgt->parentWidget() ? wgt->parentWidget()->nativeWindowHandle() : HWND_DESKTOP;

    switch (objType) {
    case ObjectType::WindowType:
        className = L"MainWindow " + std::to_wstring(++d_ptr->windowId);
        style |= WS_OVERLAPPEDWINDOW;
        exStyle |= WS_EX_APPWINDOW;
        break;    

    case ObjectType::DialogType:
        className = L"Dialog " + std::to_wstring(++d_ptr->windowId);
        style |= WS_CAPTION | WS_SYSMENU /*| DS_MODALFRAME*/;
        exStyle |= WS_EX_DLGMODALFRAME;
        break;

    case ObjectType::PopupType:
        className = L"Popup " + std::to_wstring(++d_ptr->windowId);
        style |= WS_POPUP;
        exStyle |= WS_EX_TOOLWINDOW | WS_EX_LAYERED;
        break;

    case ObjectType::WidgetType:
    default:
        className = L"Widget " + std::to_wstring(++d_ptr->windowId);
        style |= WS_CHILD;
        break;
    }

    // if (wgt->parent()) {
    //     if (wgt->parentWidget()->isCreated()) {
    //         d_ptr->registerClass(className.c_str(), hInstance);
    //         wgt->m_hWnd = CreateWindowEx(exStyle, className.c_str(), wgt->title().c_str(), style, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hWndParent, NULL, hInstance, NULL);
    //         SetWindowLongPtr(wgt->m_hWnd, GWLP_USERDATA, (LONG_PTR)wgt);
    //     } else {
    //         wgt->connectOnCreate([=]() {
    //             wgt->m_hWnd = CreateWindowEx(exStyle, className.c_str(), wgt->title().c_str(), style, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hWndParent, NULL, hInstance, NULL);
    //             SetWindowLongPtr(wgt->m_hWnd, GWLP_USERDATA, (LONG_PTR)wgt);
    //         });
    //     }
    // } else {
        d_ptr->registerClass(className.c_str(), d_ptr->hInstance);
        CreateWindowEx(exStyle, className.c_str(), wgt->title().c_str(), style, rc.x, rc.y, rc.width, rc.height,
                       hWndParent, NULL, d_ptr->hInstance, (LPVOID)wgt);
    // }
}


