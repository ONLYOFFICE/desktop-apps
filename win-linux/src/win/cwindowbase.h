#ifndef CWINDOWBASE_H
#define CWINDOWBASE_H

namespace WindowBase
{
    enum class Style : DWORD
    {
//        windowed        = ( WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN ),
        windowed        = ( WS_OVERLAPPED | WS_THICKFRAME | WS_CAPTION | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_CLIPCHILDREN ),
        aero_borderless = ( WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME | WS_CLIPCHILDREN )
    };

    struct CWindowGeometry
    {
        CWindowGeometry() {}

        bool required = false;
        int width = 0;
        int height = 0;
    };

}

#endif // CWINDOWBASE_H
