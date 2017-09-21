#include "cwindowmanager.h"
#include "defines.h"
#include "cascapplicationmanagerwrapper.h"

CWindowManager::CWindowManager()
{
//    m_vecWidows.reserve(3);
}

CWindowManager::~CWindowManager()
{
    CMainWindow * _window = nullptr;
    for (auto const& w : m_vecWidows) {
        _window = reinterpret_cast<CMainWindow *>(w);

        if ( _window ) {
            delete _window,
            _window = NULL;
        }
    }

    m_vecWidows.clear();
}

CWindowManager& CWindowManager::getInstance()
{
    static CWindowManager _instance;
    return _instance;
}

void CWindowManager::startApp()
{
    GET_REGISTRY_USER(reg_user)

    QRect _start_rect = reg_user.value("position").toRect();
    bool _is_maximized = reg_user.value("maximized", false).toBool();

    CMainWindow * _window = createMainWindow(_start_rect);

    _window->show(_is_maximized);
    _window->toggleBorderless(_is_maximized);

    if ( _is_maximized ) {
        WINDOWPLACEMENT wp{sizeof(WINDOWPLACEMENT)};
        if (GetWindowPlacement(_window->hWnd, &wp)) {
            wp.rcNormalPosition = {_start_rect.x(), _start_rect.y(), _start_rect.right(), _start_rect.bottom()};

            SetWindowPlacement(_window->hWnd, &wp);
        }
    }
}

CMainWindow * CWindowManager::createMainWindow(QRect& rect)
{
    CMainWindow * _window = new CMainWindow(rect);
    getInstance().m_vecWidows.push_back( size_t(_window) );

    return _window;
}

void CWindowManager::closeMainWindow(const size_t p)
{
    size_t _size = getInstance().m_vecWidows.size();

    if ( _size > 1 ) {
        std::vector<size_t>::iterator it = getInstance().m_vecWidows.begin();
        while ( it != getInstance().m_vecWidows.end() ) {
            if ( *it == p && getInstance().m_vecWidows.size() ) {
                CMainWindow * _w = reinterpret_cast<CMainWindow*>(*it);

                delete _w, _w = NULL;

                getInstance().m_vecWidows.erase(it);
                break;
            }

            ++it;
        }
    } else
    if ( _size == 1 && getInstance().m_vecWidows[0] == p ) {
        AscAppManager::getInstance().DestroyCefView(-1);
    }
}
