
#include "cascapplicationmanagerwrapper.h"

#include <QMutexLocker>
#include <QTimer>
#include <QDir>

#include "defines.h"
#include "cfiledialog.h"
#include "utils.h"

#define APP_CAST(app) \
    CAscApplicationManagerWrapper & app = dynamic_cast<CAscApplicationManagerWrapper &>(getInstance());

using namespace NSEditorApi;

CAscApplicationManagerWrapper::CAscApplicationManagerWrapper()
    : CAscApplicationManager()
    , CCefEventsTransformer(nullptr)
    , QObject(nullptr)
{
    CAscApplicationManager::SetEventListener(this);

    QObject::connect(this, &CAscApplicationManagerWrapper::coreEvent,
                        this, &CAscApplicationManagerWrapper::onCoreEvent);
}

CAscApplicationManagerWrapper::~CAscApplicationManagerWrapper()
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

int CAscApplicationManagerWrapper::GetPlatformKeyboardLayout()
{
    if (this->IsPlatformKeyboardSupport())
        return CAscApplicationManager::GetPlatformKeyboardLayout();

    return -1;
}

void CAscApplicationManagerWrapper::StartSaveDialog(const std::wstring& sName, unsigned int nId)
{
    CMainWindow * _window = mainWindowFromViewId(nId);
    if ( _window ) {
        QMetaObject::invokeMethod(_window->mainPanel(), "onDialogSave", Qt::QueuedConnection, Q_ARG(std::wstring, sName), Q_ARG(uint, nId));
    }
}

void CAscApplicationManagerWrapper::OnNeedCheckKeyboard()
{
    if ( !m_vecWidows.empty() ) {
        CMainWindow * _window = reinterpret_cast<CMainWindow *>(m_vecWidows.front());
        QMetaObject::invokeMethod(_window->mainPanel(), "onNeedCheckKeyboard", Qt::QueuedConnection);
    }
}

void CAscApplicationManagerWrapper::OnEvent(CAscCefMenuEvent * event)
{
    QMutexLocker locker( &m_oMutex );

    if ( event->m_nType == ASC_MENU_EVENT_TYPE_CEF_EXECUTE_COMMAND ) {
        CAscExecCommand * pData = reinterpret_cast<CAscExecCommand *>(event->m_pData);

        if ( pData->get_Command().compare(L"portal:open") == 0 ) {
            event->m_nType = ASC_MENU_EVENT_TYPE_CEF_PORTAL_OPEN;
        }
    }

    QMetaObject::invokeMethod(this, "onCoreEvent", Qt::QueuedConnection, Q_ARG(void *, event));
}

void CAscApplicationManagerWrapper::onCoreEvent(void * e)
{
    QMutexLocker locker( &m_oMutex );

    CAscCefMenuEvent * _event = static_cast<CAscCefMenuEvent *>(e);
    CMainWindow * _window, * _target = nullptr, * _dest = nullptr;

    int _uid = _event->get_SenderId();

#if 0
    if ( _event->m_nType == ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_OPEN ) {
        CAscLocalFileOpen * pData = (CAscLocalFileOpen*)_event->m_pData;
        QString inpath = QString().fromStdWString(pData->get_Directory());

        _window = mainWindowFromViewId(_event->get_SenderId());
#ifdef _WIN32
        CFileDialogWrapper dlg(_window->hWnd);
#else
        CFileDialogWrapper dlg(qobject_cast<QWidget *>(parent()));
#endif

        QString _path = !inpath.isEmpty() && QDir(inpath).exists() ?
                            inpath : Utils::lastPath(LOCAL_PATH_OPEN);

        if (!(_path = dlg.modalOpen(_path)).isEmpty()) {
            Utils::keepLastPath(LOCAL_PATH_OPEN, QFileInfo(_path).absolutePath());

            _event->Release();
            _event = new CAscCefMenuEvent(ASC_MENU_EVENT_TYPE_CEF_LOCALFILES_OPEN);
            _event->put_SenderId( _uid );

            CAscLocalOpenFileRecent_Recover * pData = new CAscLocalOpenFileRecent_Recover;
            pData->put_Path( _path.toStdWString() );

            _event->m_pData = pData;
        }
    }
#endif

    for (auto const& w : m_vecWidows) {
        _window = reinterpret_cast<CMainWindow *>(w);

        if ( _event->m_nType == ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_RECENTOPEN ||
                _event->m_nType == ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_RECOVEROPEN )
        {
            CAscLocalOpenFileRecent_Recover * pData = (CAscLocalOpenFileRecent_Recover *)_event->m_pData;
            QString path = QString::fromStdWString(pData->get_Path());

            if ( _window->mainPanel()->holdUrl(path, etLocalFile) ) {
                _target = _window;
                break;
            }
        } else
        if ( _event->m_nType == ASC_MENU_EVENT_TYPE_CEF_PORTAL_OPEN ) {
            CAscExecCommand * pData = (CAscExecCommand *)_event->m_pData;
            QString url = QString::fromStdWString(pData->get_Param());

            if ( _window->mainPanel()->holdUrl(url, etPortal) ) {
                _target = _window;
                break;
            }
        }

        if ( _window->holdView(_uid) ) {
            _dest = _window;
        }
    }

    if ( _target )
        _window = _target; else
        _window = _dest;

    if ( _window ) {
        if ( countMainWindow() > 1 )
                ::SetForegroundWindow(_window->hWnd);

        CCefEventsTransformer::OnEvent(_window->mainPanel(), _event);
    }

}

CAscApplicationManager & CAscApplicationManagerWrapper::getInstance()
{
    static CAscApplicationManagerWrapper _manager;
    return _manager;
}

CAscApplicationManager * CAscApplicationManagerWrapper::createInstance()
{
    return new CAscApplicationManagerWrapper;
}

void CAscApplicationManagerWrapper::startApp()
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

CMainWindow * CAscApplicationManagerWrapper::createMainWindow(QRect& rect)
{
    APP_CAST(_app)

    QMutexLocker locker( &_app.m_oMutex );

    CMainWindow * _window = new CMainWindow(rect);
    _app.m_vecWidows.push_back( size_t(_window) );

    return _window;
}

void CAscApplicationManagerWrapper::closeMainWindow(const size_t p)
{
    APP_CAST(_app)

    QMutexLocker locker( &_app.m_oMutex );
    size_t _size = _app.m_vecWidows.size();

    if ( _size > 1 ) {
        vector<size_t>::iterator it = _app.m_vecWidows.begin();
        while ( it != _app.m_vecWidows.end() ) {
            if ( *it == p && _app.m_vecWidows.size() ) {
                CMainWindow * _w = reinterpret_cast<CMainWindow*>(*it);

                delete _w, _w = nullptr;

                _app.m_vecWidows.erase(it);
                break;
            }

            ++it;
        }
    } else
    if ( _size == 1 && _app.m_vecWidows[0] == p ) {
        AscAppManager::getInstance().DestroyCefView(-1);
    }
}

int CAscApplicationManagerWrapper::countMainWindow()
{
    APP_CAST(_app)

    return _app.m_vecWidows.size();
}

CMainWindow * CAscApplicationManagerWrapper::mainWindowFromViewId(int uid) const
{
    CMainWindow * _window = nullptr;

    for (auto const& w : m_vecWidows) {
        _window = reinterpret_cast<CMainWindow *>(w);

        if ( _window->holdView(uid) )
            return _window;
    }

    return 0;
}

void CAscApplicationManagerWrapper::processMainWindowMoving(const size_t s, const QPoint& c)
{
    APP_CAST(_app);

    if ( _app.m_vecWidows.size() > 1 ) {
        QPoint _local_pos;
        CMainWindow * _window = nullptr,
                    * _source = reinterpret_cast<CMainWindow *>(s);
        for (auto const& w : _app.m_vecWidows) {
            if ( w != s ) {
                _window = reinterpret_cast<CMainWindow *>(w);
                _local_pos = _window->mainPanel()->mapFromGlobal(c);

                if ( _window->mainPanel()->isPointInTabs(_local_pos) ) {
                    QTimer::singleShot(0, [=] {
                        QPoint pos = QCursor::pos();
                        PostMessage(_source->hWnd, WM_LBUTTONUP, MK_LBUTTON, MAKELPARAM(pos.x(), pos.y()));

                        QWidget * panel = _source->m_pWinPanel->getMainPanel()->releaseEditor();
                        _window->joinTab(panel);

                        closeMainWindow(s);
                    });

                    break;
                }
            }
        }
    }
}
