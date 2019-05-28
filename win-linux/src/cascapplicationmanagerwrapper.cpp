
#include "cascapplicationmanagerwrapper.h"
#include "cascapplicationmanagerwrapper_private.h"

#include <QMutexLocker>
#include <QTimer>
#include <QDir>
#include <QDateTime>
#include <QDesktopWidget>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <algorithm>
#include <functional>

#include "cstyletweaks.h"
#include "defines.h"
#include "cfiledialog.h"
#include "utils.h"
#include "common/Types.h"
#include "ctabundockevent.h"
#include "clangater.h"
#include "cdpichecker.h"
#include "cmessage.h"
#include "ceditortools.h"

#ifdef _WIN32
#include "csplash.h"

# ifdef _UPDMODULE
   #include "3dparty/WinSparkle/include/winsparkle.h"
# endif
#endif


#define APP_CAST(app) \
    CAscApplicationManagerWrapper & app = static_cast<CAscApplicationManagerWrapper &>(getInstance());

#define SKIP_EVENTS_QUEUE(callback) QTimer::singleShot(0, callback)

using namespace NSEditorApi;

CAscApplicationManagerWrapper::CAscApplicationManagerWrapper(CAscApplicationManagerWrapper const&)
{

}

CAscApplicationManagerWrapper::CAscApplicationManagerWrapper()
    : CAscApplicationManager()
    , CCefEventsTransformer(nullptr)
    , QObject(nullptr)
    , m_private(new CAscApplicationManagerWrapper::CAscApplicationManagerWrapper_Private(this))
{
    CAscApplicationManager::SetEventListener(this);

    QObject::connect(this, &CAscApplicationManagerWrapper::coreEvent,
                        this, &CAscApplicationManagerWrapper::onCoreEvent);
}

CAscApplicationManagerWrapper::~CAscApplicationManagerWrapper()
{
//    CSingleWindow * _sw = nullptr;
//    for (auto const& w : m_vecEditors) {
//        _sw = reinterpret_cast<CSingleWindow *>(w);

//        if ( _sw ) {
//#ifdef _WIN32
//            delete _sw, _sw = NULL;
//#else
//            _sw->deleteLater();
//#endif
//        }
//    }


    CMainWindow * _window = nullptr;
    for (auto const& w : m_vecWidows) {
        _window = reinterpret_cast<CMainWindow *>(w);

        if ( _window ) {
#ifdef _WIN32
            delete _window, _window = NULL;
#else
            _window->deleteLater();
#endif
        }
    }

    m_vecWidows.clear();
//    m_vecEditors.clear();
}

int CAscApplicationManagerWrapper::GetPlatformKeyboardLayout()
{
    if (this->IsPlatformKeyboardSupport())
        return CAscApplicationManager::GetPlatformKeyboardLayout();

    return -1;
}

void CAscApplicationManagerWrapper::StartSaveDialog(const std::wstring& sName, unsigned int nId)
{
    CAscSaveDialog * data = new CAscSaveDialog;
    data->put_FilePath(sName);
    data->put_IdDownload(nId);

    CAscCefMenuEvent * event = new CAscCefMenuEvent(ASC_MENU_EVENT_TYPE_CEF_SAVEFILEDIALOG);
    event->m_pData = data;

    OnEvent(event);
}

void CAscApplicationManagerWrapper::OnNeedCheckKeyboard()
{
    OnEvent(new CAscCefMenuEvent(ASC_MENU_EVENT_TYPE_CEF_CHECK_KEYBOARD));
}

void CAscApplicationManagerWrapper::OnEvent(CAscCefMenuEvent * event)
{
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

    if ( m_private->processEvent(_event) || processCommonEvent(_event) )  {
        RELEASEINTERFACE(_event);
        return;
    }

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
            QString json = QString::fromStdWString(pData->get_Param()),
                    url;

            QRegularExpression re("portal[\":]+([^\"]+)");
            QRegularExpressionMatch match = re.match(json);
            if ( match.hasMatch() ) url = match.captured(1);

            if ( !url.isEmpty() && _window->mainPanel()->holdUrl(url, etPortal) ) {
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
        if ( (_event->m_nType == ASC_MENU_EVENT_TYPE_PAGE_GOT_FOCUS) && (countMainWindow() > 1) ) {
#ifdef __linux
            QApplication::setActiveWindow(_window);
#else
            ::SetForegroundWindow(_window->hWnd);
#endif
        }

        CCefEventsTransformer::OnEvent(_window->mainPanel(), _event);
    } else {
/**/
        map<int, CCefEventsGate *>::const_iterator it = m_receivers.find(_uid);
        if ( it != m_receivers.cend() ) {
            CCefEventsTransformer::OnEvent(it->second, _event);
            return;
        }

        RELEASEINTERFACE(_event);
    }
}

bool CAscApplicationManagerWrapper::processCommonEvent(NSEditorApi::CAscCefMenuEvent * event)
{
    switch ( event->m_nType ) {
    case ASC_MENU_EVENT_TYPE_CEF_ONOPENLINK: {
        event->AddRef();

        SKIP_EVENTS_QUEUE([event] () {
            CAscOnOpenExternalLink * const pData = static_cast<CAscOnOpenExternalLink *>(event->m_pData);
            Utils::openUrl( QString::fromStdWString(pData->get_Url()) );

            event->Release();
        });
        return true; }

    case ASC_MENU_EVENT_TYPE_CEF_EXECUTE_COMMAND: {
        CAscExecCommand * const pData = static_cast<CAscExecCommand * const>(event->m_pData);
        std::wstring const & cmd = pData->get_Command();

        if ( cmd.compare(L"portal:logout") == 0 ) {
            broadcastEvent(event);

//            RELEASEINTERFACE(event);
            return true;
        } else
        if ( cmd.compare(0, 8, L"settings") == 0 ) {
            if ( cmd.rfind(L"apply") != wstring::npos ) {
                applySettings(pData->get_Param());
            } else
            if ( cmd.rfind(L"get") != wstring::npos ) {
                sendSettings(pData->get_Param());
            }

//            RELEASEINTERFACE(event);
            return true;
        } else
        if ( !(cmd.find(L"editor:event") == wstring::npos) ) {
            wstring action = pData->get_Param();
            if ( action.find(L"undocking") != wstring::npos ) {
                int id = event->get_SenderId();
                SKIP_EVENTS_QUEUE([=]{
                    manageUndocking(id, action);
                });

                return true;
            }
        } else
        if ( !(cmd.find(L"window:features") == wstring::npos) ) {
            const wstring& param = pData->get_Param();
            if ( param.compare(L"request") == 0 ) {
                QJsonObject _json_obj{{"canUndock", "true"}};

                AscAppManager::sendCommandTo(AscAppManager::GetViewById(event->get_SenderId()),
                                    L"window:features", Utils::encodeJson(_json_obj).toStdWString());
            }
            return true;
        } else
        if ( !(cmd.find(L"update") == std::wstring::npos) ) {
#ifdef _UPDMODULE
            if ( QString::fromStdWString(pData->get_Param()) == "check" ) {
                CMainWindow::checkUpdates();
            }
#endif

            return true;
        }

        break; }

    case ASC_MENU_EVENT_TYPE_SSO_TOKEN: {
//        CAscSSOToken * pData = (CAscSSOToken *)_event->m_pData;
        return true; }

    case ASC_MENU_EVENT_TYPE_REPORTER_CREATE: {
        CSingleWindow * pEditorWindow = createReporterWindow(event->m_pData, event->get_SenderId());
#ifdef __linux
        pEditorWindow->show();
#else
        pEditorWindow->show(false);
        pEditorWindow->toggleBorderless(false);
#endif

//        RELEASEINTERFACE(event);
        return true; }

    case ASC_MENU_EVENT_TYPE_REPORTER_END: {
        // close editor window
        if ( m_reporterWindow && m_reporterWindow->holdView(event->get_SenderId()) ) {
            AscAppManager::getInstance().DestroyCefView(event->get_SenderId());
        }

//        RELEASEINTERFACE(event);
        return true; }

    case ASC_MENU_EVENT_TYPE_REPORTER_MESSAGE_TO:
    case ASC_MENU_EVENT_TYPE_REPORTER_MESSAGE_FROM: {
        CAscReporterMessage * pData = (CAscReporterMessage *)event->m_pData;
        CCefView * pView = GetViewById(pData->get_ReceiverId());
        if ( pView ) {
            pView->Apply(event);
        }
        return true; }
    case ASC_MENU_EVENT_TYPE_UI_THREAD_MESSAGE: {
        event->AddRef();
        this->Apply(event);
        return true; }

    case ASC_MENU_EVENT_TYPE_PAGE_SELECT_OPENSSL_CERTIFICATE: {
#ifdef DOCUMENTSCORE_OPENSSL_SUPPORT
        CMainWindow * _window = mainWindowFromViewId(event->get_SenderId());
        if ( _window ) {
            _window->sendSertificate(event->get_SenderId());
        }
#endif
        return true; }
    case ASC_MENU_EVENT_TYPE_CEF_DESTROYWINDOW: {
        if ( m_reporterWindow && m_reporterWindow->holdView(event->get_SenderId()) ) {
            delete m_reporterWindow, m_reporterWindow = nullptr;
            return true;
        }

        break;
    }

    case ASC_MENU_EVENT_TYPE_SYSTEM_EXTERNAL_PLUGINS: {
        CAscSystemExternalPlugins * pData = static_cast<CAscSystemExternalPlugins *>(event->m_pData);
        QJsonObject _json_obj;
        QJsonParseError jerror;
        QJsonDocument jdoc;

        for (const CAscSystemExternalPlugins::CItem& item: pData->get_Items()) {
            _json_obj["name"] = QString::fromStdWString(item.name);
            _json_obj["id"] = QString::fromStdWString(item.id);
            _json_obj["url"] = QString::fromStdWString(item.url);

            if ( !item.nameLocale.empty() ) {
                jdoc = QJsonDocument::fromJson(QString::fromStdWString(item.nameLocale).toUtf8(), &jerror);

                if( jerror.error == QJsonParseError::NoError ) {
                    QString _lang(CLangater::getCurrentLangCode());

                    if ( jdoc.object().contains(_lang) || jdoc.object().contains((_lang = _lang.left(2))) ) {
                        _json_obj["name"] = jdoc.object()[_lang].toString();
                    }
                }
            }

            AscAppManager::sendCommandTo(SEND_TO_ALL_START_PAGE, "panel:external", Utils::encodeJson(_json_obj));
        }

        return true; }

    case ASC_MENU_EVENT_TYPE_SYSTEM_EXTERNAL_PROCESS: {
        CAscExternalProcess * pData = (CAscExternalProcess *)event->m_pData;
        QStringList arguments;
        const vector<wstring>& srcArgs = pData->get_Arguments();

        for (auto & wstr: srcArgs)
            arguments.append(QString::fromStdWString(wstr));

        if (pData->get_Detached())
            QProcess::startDetached(QString::fromStdWString(pData->get_Program()), arguments, QString::fromStdWString(pData->get_WorkingDirectory()));
        else QProcess::execute(QString::fromStdWString(pData->get_Program()), arguments);

//        RELEASEINTERFACE(event);
        return true; }

    case ASC_MENU_EVENT_TYPE_CEF_SAVEFILEDIALOG:{
        CAscSaveDialog * pData = (CAscSaveDialog *)event->m_pData;
        SKIP_EVENTS_QUEUE(std::bind(&CAscApplicationManagerWrapper::onDownloadSaveDialog, this, pData->get_FilePath(), pData->get_IdDownload()));
        return true;}

    case ASC_MENU_EVENT_TYPE_CEF_DOWNLOAD: {
        CMainWindow * mw = topWindow();
        if ( mw ) mw->mainPanel()->onDocumentDownload(event->m_pData);
        return true;}

    case ASC_MENU_EVENT_TYPE_CEF_CHECK_KEYBOARD:
        CheckKeyboard();
        return true;

    case ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_ADDIMAGE: {
        static_cast<CAscLocalOpenFileDialog *>(event->m_pData)->put_Filter(L"image");}

    case ASC_MENU_EVENT_TYPE_DOCUMENTEDITORS_OPENFILENAME_DIALOG: {
        event->AddRef();
        SKIP_EVENTS_QUEUE([event]{
            CEditorTools::getlocalfile(event);
            event->Release();
        });

        return true;}

    default: break;
    }

    return false;
}

void CAscApplicationManagerWrapper::broadcastEvent(NSEditorApi::CAscCefMenuEvent * event)
{
    CMainWindow * _window;
    for ( auto const& w : m_vecWidows ) {
        _window = reinterpret_cast<CMainWindow *>(w);

        ADDREFINTERFACE(event);
        CCefEventsTransformer::OnEvent(_window->mainPanel(), event);
    }

    for (const auto& it: m_receivers) {
        ADDREFINTERFACE(event);
        CCefEventsTransformer::OnEvent(it.second, event);
    }

//    RELEASEINTERFACE(event);
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

#ifdef __linux
    _window->show();
    if ( _is_maximized )
        _window->slot_windowChangeState(Qt::WindowMaximized);
#else
    _window->show(_is_maximized);
    _window->toggleBorderless(_is_maximized);

    if ( _is_maximized ) {
        WINDOWPLACEMENT wp{sizeof(WINDOWPLACEMENT)};
        if (GetWindowPlacement(_window->hWnd, &wp)) {
            wp.rcNormalPosition = {_start_rect.x(), _start_rect.y(), _start_rect.right(), _start_rect.bottom()};

            SetWindowPlacement(_window->hWnd, &wp);
        }
    }
#endif

#ifdef DOCUMENTSCORE_OPENSSL_SUPPORT
    APP_CAST(_app);
#endif
}

void CAscApplicationManagerWrapper::initializeApp()
{
    APP_CAST(_app);
    _app.m_private->initializeApp();

#ifdef _WIN32
    CSplash::showSplash();
    QApplication::processEvents();
#endif

    /* prevent drawing of focus rectangle on a button */
    QApplication::setStyle(new CStyleTweaks);

    GET_REGISTRY_SYSTEM(reg_system)
    GET_REGISTRY_USER(reg_user)
    reg_user.setFallbacksEnabled(false);

    // read installation time and clean cash folders if expired
    if ( reg_system.contains("timestamp") ) {
        QString user_data_path = Utils::getUserPath() + APP_DATA_PATH;

        QDateTime time_install, time_clear;
        time_install.setMSecsSinceEpoch(reg_system.value("timestamp", 0).toULongLong());

        bool clean = true;
        if ( reg_user.contains("timestamp") ) {
            time_clear.setMSecsSinceEpoch(reg_user.value("timestamp", 0).toULongLong());

            clean = time_install > time_clear;
        }

        if ( clean ) {
            reg_user.setValue("timestamp", QDateTime::currentDateTime().toMSecsSinceEpoch());
            QDir(user_data_path + "/fonts").removeRecursively();
        }
    }

    _app.m_vecStyles.push_back(":styles/res/styles/styles.qss");
    _app.m_vecStyles2x.push_back(":styles@2x/styles.qss");
    _app.m_private->applyStylesheets();

    // TODO: merge stylesheets and apply for the whole app
    qApp->setStyleSheet( Utils::readStylesheets(":styles/res/styles/styles.qss") );

    // Font
    QFont mainFont = QApplication::font();
    mainFont.setStyleStrategy( QFont::PreferAntialias );
    QApplication::setFont( mainFont );

}

CMainWindow * CAscApplicationManagerWrapper::createMainWindow(QRect& rect)
{
    APP_CAST(_app)

    QMutexLocker locker( &_app.m_oMutex );

    CMainWindow * _window = new CMainWindow(rect);
    _app.m_vecWidows.push_back( size_t(_window) );

    return _window;
}

CSingleWindow * CAscApplicationManagerWrapper::createReporterWindow(void * data, int parentid)
{
//    QMutexLocker locker( &m_oMutex );

    CAscReporterCreate * pData = (CAscReporterCreate *)data;
    CAscReporterData * pCreateData = reinterpret_cast<CAscReporterData *>(pData->get_Data());
    pData->put_Data(NULL);

    QCefView * pView = createViewer(NULL);
    pView->CreateReporter(this, pCreateData);

    QString _doc_name;
    QRect _windowRect{100,100,1000,700}, _currentRect;
    CMainWindow * _main_window = mainWindowFromViewId(parentid);
    if ( _main_window ) {
        _doc_name = _main_window->documentName(parentid);
        _currentRect = _main_window->windowRect();
    } else {
        CEditorWindow * _window = editorWindowFromViewId(parentid);

        if ( _window ) {
            _doc_name = _window->documentName();
            _currentRect = _window->geometry();
        }
    }

    if ( QApplication::desktop()->screenCount() > 1 ) {
        int _scrNum = QApplication::desktop()->screenNumber(_currentRect.topLeft());
        QRect _scrRect = QApplication::desktop()->screenGeometry(QApplication::desktop()->screenCount()-_scrNum-1);

        _windowRect.setSize(QSize(1000,700));
        _windowRect.moveCenter(_scrRect.center());
    }

    m_reporterWindow = new CSingleWindow(_windowRect, tr("Presenter View") + " - " + _doc_name, pView);


    return m_reporterWindow;
}

void CAscApplicationManagerWrapper::closeMainWindow(const size_t p)
{
    APP_CAST(_app)

    QMutexLocker locker( &_app.m_oMutex );
    size_t _size = _app.m_vecWidows.size();

    if ( _size > 1 ) {
//        vector<size_t>::iterator it = _app.m_vecWidows.begin();
//        while ( it != _app.m_vecWidows.end() ) {
//            if ( *it == p && _app.m_vecWidows.size() ) {
//                CMainWindow * _w = reinterpret_cast<CMainWindow*>(*it);

//                delete _w, _w = nullptr;

//                _app.m_vecWidows.erase(it);
//                break;
//            }

//            ++it;
//        }
    } else
    if ( _size == 1 && _app.m_vecWidows[0] == p ) {
        SKIP_EVENTS_QUEUE([p]{
            if ( canAppClose() ) {
                CMainWindow * _w = reinterpret_cast<CMainWindow *>(p);
                if ( _w ) {
                    _w->mainPanel()->closeAll();
                }
            }
        });
    }
}

void CAscApplicationManagerWrapper::destroyMainWindow(const size_t p)
{
    CMainWindow * _w = reinterpret_cast<CMainWindow *>(p);
    if ( _w ) {
        APP_CAST(_app);
        auto & it = find(_app.m_vecWidows.begin(), _app.m_vecWidows.end(), p);
        if ( it != _app.m_vecWidows.end() ) {
            _app.m_vecWidows.erase(it);
        }

        if (_app.m_vecWidows.empty()) {
            while (!_app.m_vecEditors.empty()) {
                qApp->processEvents();
            }
        }

        delete _w, _w = nullptr;
    }
}

void CAscApplicationManagerWrapper::closeEditorWindow(const size_t p)
{
    APP_CAST(_app)
//    QMutexLocker locker( &_app.m_oMutex );

    vector<size_t>::iterator it = _app.m_vecEditors.begin();
    while ( it != _app.m_vecEditors.end() ) {
        if ( *it == p && !_app.m_vecEditors.empty() ) {
            CSingleWindowBase * _w = reinterpret_cast<CSingleWindowBase *>(*it);

            delete _w, _w = nullptr;

            _app.m_vecEditors.erase(it);
            break;
        }

        ++it;
    }
}

uint CAscApplicationManagerWrapper::countMainWindow()
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

    return nullptr;
}

CEditorWindow * CAscApplicationManagerWrapper::editorWindowFromViewId(int uid) const
{
    CEditorWindow * _window = nullptr;

    for (auto const& w : m_vecEditors) {
        _window = reinterpret_cast<CEditorWindow *>(w);

        if ( _window->holdView(uid) )
            return _window;
    }

    return nullptr;
}

ParentHandle CAscApplicationManagerWrapper::windowHandleFromId(int id)
{
    APP_CAST(_app);

    CMainWindow * w = _app.mainWindowFromViewId(id);
    if ( w ) return w->handle();
    else {
        CEditorWindow * e = _app.editorWindowFromViewId(id);
        if ( e ) return e->handle();
    }

    return nullptr;
}

void CAscApplicationManagerWrapper::processMainWindowMoving(const size_t s, const QPoint& c)
{
#define GET_CURRENT_PANEL -1
    APP_CAST(_app);

    if ( _app.m_vecWidows.size() > 1 ) {
        CMainWindow * _window = nullptr,
                    * _source = reinterpret_cast<CMainWindow *>(s);
        for (auto const& w : _app.m_vecWidows) {
            if ( w != s ) {
                _window = reinterpret_cast<CMainWindow *>(w);

                if ( _window->pointInTabs(c) ) {
                    _source->hide();
                    _window->attachEditor(
                            _source->getEditor(GET_CURRENT_PANEL), c );

                    closeMainWindow(s);
                    break;
                }
            }
        }
    }
}

CMainWindow * CAscApplicationManagerWrapper::topWindow()
{
    APP_CAST(_app);

    if ( _app.m_vecWidows.size() > 0 )
        return reinterpret_cast<CMainWindow *>(_app.m_vecWidows.at(0));
    else return nullptr;
}

void CAscApplicationManagerWrapper::sendCommandTo(QCefView * target, const QString& cmd, const QString& args)
{
    sendCommandTo(target ? target->GetCefView() : nullptr, cmd.toStdWString(), args.toStdWString() );
}

void CAscApplicationManagerWrapper::sendCommandTo(CCefView * target, const wstring& cmd, const wstring& args)
{
    CAscExecCommandJS * pCommand = new CAscExecCommandJS;
    pCommand->put_Command(cmd);
    if ( !args.empty() )
        pCommand->put_Param(args);

    CAscMenuEvent * pEvent = new CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_EXECUTE_COMMAND_JS);
    pEvent->m_pData = pCommand;

    if ( target ) {
        if ( target->GetType() == cvwtEditor )
            pCommand->put_FrameName(L"frameEditor");

        target->Apply(pEvent);
    } else AscAppManager::getInstance().SetEventToAllMainWindows(pEvent);
}

void CAscApplicationManagerWrapper::sendEvent(int type, void * data)
{
    CAscMenuEvent * pEvent = new CAscMenuEvent(type);
    pEvent->m_pData = static_cast<IMenuEventDataBase *>(data);

    AscAppManager::getInstance().Apply(pEvent);

//    delete pEvent;
}

QString CAscApplicationManagerWrapper::getWindowStylesheets(uint dpifactor)
{
    APP_CAST(_app);
    return Utils::readStylesheets(&_app.m_vecStyles, &_app.m_vecStyles2x, dpifactor);
}

bool CAscApplicationManagerWrapper::event(QEvent *event)
{
    if ( event->type() == CTabUndockEvent::type() ) {
        CTabUndockEvent * e = static_cast<CTabUndockEvent *>(event);
        if ( e->panel() ) {
            e->accept();

            CTabPanel * _panel = static_cast<CTabPanel *>(e->panel());
            QTimer::singleShot(0, this, [=]{
                CMainWindow * _main_window = nullptr;

#ifdef _WIN32
                CWinPanel * _wp = dynamic_cast<CWinPanel *>(_panel->window());
                if ( _wp ) _main_window = _wp->parent();
#else
                _main_window = dynamic_cast<CMainWindow *>(_panel->window());
#endif

                if ( _main_window ) {
                    QRect _win_rect = _main_window->windowRect();
                    _win_rect.moveTo(QCursor::pos() - QPoint(BUTTON_MAIN_WIDTH + 50, 20));
                    CMainWindow * window = createMainWindow(_win_rect);

                    bool _is_maximized = _main_window->isMaximized();
#ifdef Q_OS_WIN
                    window->show(_is_maximized);
                    window->toggleBorderless(_is_maximized);
#else
                    window->show();
                    if ( _is_maximized )
                        window->slot_windowChangeState(Qt::WindowMaximized);
#endif
                    window->attachEditor( _panel );
                }
            });
        }

        return true;
    }

    return QObject::event(event);
}

bool CAscApplicationManagerWrapper::applySettings(const wstring& wstrjson)
{
    QJsonParseError jerror;
    QJsonDocument jdoc = QJsonDocument::fromJson(QString::fromStdWString(wstrjson).toUtf8(), &jerror);

    if( jerror.error == QJsonParseError::NoError ) {
        QJsonObject objRoot = jdoc.object();

        QString _user_newname = objRoot["username"].toString();
        if ( _user_newname.isEmpty() )
            _user_newname = QString::fromStdWString(Utils::systemUserName());

        QString _lang_id = CLangater::getCurrentLangCode();
        if ( objRoot.contains("langid") ) {
            QString l = objRoot.value("langid").toString();
            if ( _lang_id != l ) {
                _lang_id = l;

                GET_REGISTRY_USER(_reg_user)
                _reg_user.setValue("locale", _lang_id);

                CLangater::reloadTranslations(_lang_id);
            }
        }

        wstring params = QString("lang=%1&username=%3&location=%2")
                            .arg(_lang_id, Utils::systemLocationCode(), QUrl::toPercentEncoding(_user_newname)).toStdWString();

        if ( objRoot["docopenmode"].toString() == "view" )
            params.append(L"&mode=view");

        AscAppManager::getInstance().InitAdditionalEditorParams( params );
    } else {
        /* parse settings error */
    }

    return true;
}

void CAscApplicationManagerWrapper::sendSettings(const wstring& opts)
{
    wstring _send_cmd, _send_opts;
    if ( opts == L"username" ) {
        _send_cmd = L"settings:username";
        _send_opts = Utils::systemUserName();
    } else
    if ( opts == L"has:opened" ) {
        _send_cmd = L"settings:hasopened";

        CMainWindow * _window = nullptr;
        for (auto const& w : m_vecWidows) {
            _window = reinterpret_cast<CMainWindow *>(w);

            if ( _window && _window->editorsCount() ) {
                _send_opts = L"has";
                break;
            }
        }
    }

    if ( !_send_opts.empty() )
        QTimer::singleShot(0, [_send_cmd, _send_opts] {
            AscAppManager::sendCommandTo(SEND_TO_ALL_START_PAGE, _send_cmd, _send_opts);
        });
}

CAscDpiChecker* CAscApplicationManagerWrapper::InitDpiChecker()
{
    return new CDpiChecker();
}

bool CAscApplicationManagerWrapper::canAppClose()
{
    APP_CAST(_app);

    if ( !_app.m_vecEditors.empty() ) {
        CMessage mess(topWindow()->hWnd, CMessageOpts::moButtons::mbYesNo);
        if ( mess.confirm(QObject::tr("Close all editors windows?")) == MODAL_RESULT_CUSTOM + 0 ) {
            /* close all editors windows */
            auto it = _app.m_vecEditors.begin();
            while ( it != _app.m_vecEditors.end() ) {
                CEditorWindow * _w = reinterpret_cast<CEditorWindow *>(*it);

                int _r = _w->closeWindow();
                if ( _r == MODAL_RESULT_CANCEL ) {
                    return false;
                } else ++it;
            }

            return true;
        } else
            return false;
    }

#ifdef Q_OS_WIN
# ifdef _UPDMODULE
    if ( win_sparkle_is_processing() ) {
        CMessage mess(topWindow()->hWnd);
        mess.setButtons({tr("Yes"), tr("No") + ":default"});
        return mess.confirm(QObject::tr("Update is running. Break update and close the app?")) == MODAL_RESULT_CUSTOM;
    }
# endif
#endif

    return true;
}

QCefView * CAscApplicationManagerWrapper::createViewer(QWidget * parent)
{
    APP_CAST(_app);

    return _app.m_private->createView(parent);
}

void CAscApplicationManagerWrapper::destroyViewer(int id)
{
    APP_CAST(_app);
    _app.DestroyCefView(id);
}

void CAscApplicationManagerWrapper::destroyViewer(QCefView * v)
{
    APP_CAST(_app);
    destroyViewer(v->GetCefView()->GetId());
}

void CAscApplicationManagerWrapper::manageUndocking(int id, const std::wstring& action)
{
    CTabPanel * tabpanel = nullptr;
    QJsonObject _json_obj;
    _json_obj["action"] = "undocking";


    if ( action.find(L"undock") == wstring::npos ) {
        _json_obj["status"] = "docked";

        CSingleWindowBase * editor_win = nullptr;
        for (auto const& w : m_vecEditors) {
            CSingleWindowBase * _w = reinterpret_cast<CSingleWindowBase *>(w);

            if ( _w->holdView(id) ) {
                editor_win = _w;
                break;
            }
        }

        if ( editor_win ) {
            tabpanel = static_cast<CEditorWindow *>(editor_win)->releaseEditorView();

            CMainWindow * main_win = topWindow();
            main_win->attachEditor(tabpanel);

            closeEditorWindow(size_t(editor_win));
        }
    } else {
        _json_obj["status"] = "undocked";

        CMainWindow * const main_win = mainWindowFromViewId(id);
        int index = main_win->mainPanel()->tabWidget()->tabIndexByView(id);
        if ( !(index < 0) ) {
            QRect r = main_win->windowRect();
            tabpanel = qobject_cast<CTabPanel *>(main_win->getEditor(index));

            CEditorWindow * editor_win = new CEditorWindow(QRect(r.left() + 50, r.top() + 50, r.width(), r.height()), tabpanel);
            editor_win->show(false);
            editor_win->toggleBorderless(false);

            m_vecEditors.push_back( size_t(editor_win) );
//            main_win->mainPanel()->tabWidget()->removeTab(index);
        }
    }

    if ( tabpanel ) {
        sendCommandTo(tabpanel->cef(), L"window:status", Utils::encodeJson(_json_obj).toStdWString());
    }
}

void CAscApplicationManagerWrapper::bindReceiver(int view_id, CCefEventsGate * const receiver)
{
    APP_CAST(_app);
    _app.m_receivers[view_id] = receiver;
}

void CAscApplicationManagerWrapper::unbindReceiver(int view_id)
{
    APP_CAST(_app);
    _app.m_receivers.erase(view_id);
}

void CAscApplicationManagerWrapper::onDownloadSaveDialog(const std::wstring& name, uint id)
{
#ifdef Q_OS_WIN
    HWND parent = GetActiveWindow();
#else
    QWidget * parent = nullptr;
#endif

    if ( parent ) {
        static bool saveInProcess = false;
        if ( !saveInProcess ) {
            saveInProcess = true;

            if ( name.size() ) {
                QString savePath = Utils::lastPath(LOCAL_PATH_SAVE);
                QString fullPath = savePath + "/" + QString().fromStdWString(name);
                CFileDialogWrapper dlg(parent);

                if ( dlg.modalSaveAs(fullPath) ) {
                    Utils::keepLastPath(LOCAL_PATH_SAVE, QFileInfo(fullPath).absoluteDir().absolutePath());
                }

                AscAppManager::getInstance().EndSaveDialog(fullPath.toStdWString(), id);
            }

            saveInProcess = false;
        }
    }
}

