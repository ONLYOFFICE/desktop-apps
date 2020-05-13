
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
#include "cmessage.h"
#include "ceditortools.h"

#ifdef _WIN32
#include "csplash.h"

# ifdef _UPDMODULE
   #include "3dparty/WinSparkle/include/winsparkle.h"
# endif
#endif

#include "../../../desktop-sdk/ChromiumBasedEditors/videoplayerlib/qascvideoview.h"


#define APP_CAST(app) \
    CAscApplicationManagerWrapper & app = static_cast<CAscApplicationManagerWrapper &>(AscAppManager::getInstance());

#define SKIP_EVENTS_QUEUE(callback) QTimer::singleShot(0, callback)

extern QStringList g_cmdArgs;

using namespace NSEditorApi;
using namespace std::placeholders;

CAscApplicationManagerWrapper::CAscApplicationManagerWrapper(CAscApplicationManagerWrapper const&)
{

}

CAscApplicationManagerWrapper::CAscApplicationManagerWrapper()
    : QAscApplicationManager()
    , CCefEventsTransformer(nullptr)
    , QObject(nullptr)
    , m_private(new CAscApplicationManagerWrapper::CAscApplicationManagerWrapper_Private(this))
    , m_queueToClose(new CWindowsQueue<sWinTag>)
{
    CAscApplicationManager::SetEventListener(this);

    QObject::connect(this, &CAscApplicationManagerWrapper::coreEvent,
                        this, &CAscApplicationManagerWrapper::onCoreEvent);

    m_queueToClose->setcallback(std::bind(&CAscApplicationManagerWrapper::onQueueCloseWindow,this, _1));

    NSBaseVideoLibrary::Init(nullptr);
}

CAscApplicationManagerWrapper::~CAscApplicationManagerWrapper()
{
    NSBaseVideoLibrary::Destroy();

    delete m_queueToClose, m_queueToClose = nullptr;

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
    for (auto const& w : m_vecWindows) {
        _window = reinterpret_cast<CMainWindow *>(w);

        if ( _window ) {
#ifdef _WIN32
            delete _window, _window = NULL;
#else
            _window->deleteLater();
#endif
        }
    }

    m_vecWindows.clear();
//    m_vecEditors.clear();
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

    for (auto const& w : m_vecWindows) {
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

        if ( cmd.compare(L"portal:login") == 0 ) {
            AscAppManager::sendCommandTo(SEND_TO_ALL_START_PAGE, L"portal:login", Utils::encodeJson(pData->get_Param()));
            return true;
        } else
        if ( cmd.compare(L"portal:logout") == 0 ) {
            const wstring& portal = pData->get_Param();
            if ( (m_closeCount = logoutCount(portal)) > 0 ) {
                m_closeTarget = portal;
                broadcastEvent(event);
            } else {
                Logout(portal);
                AscAppManager::sendCommandTo(SEND_TO_ALL_START_PAGE, L"portal:logout", portal);
            }

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
//                int id = event->get_SenderId();
//                SKIP_EVENTS_QUEUE([=]{
//                    manageUndocking(id, action);
//                });

                return true;
            }
        } else
#if defined(__APP_MULTI_WINDOW)
        if ( !(cmd.find(L"window:features") == wstring::npos) ) {
            const wstring& param = pData->get_Param();
            if ( param.compare(L"request") == 0 ) {
//                QJsonObject _json_obj{{"canUndock", "true"}};

//                AscAppManager::sendCommandTo(AscAppManager::GetViewById(event->get_SenderId()),
//                                    L"window:features", Utils::encodeJson(_json_obj).toStdWString());
            }
            return true;
        } else
#endif
        if ( !(cmd.find(L"update") == std::wstring::npos) ) {
#ifdef _UPDMODULE
            if ( QString::fromStdWString(pData->get_Param()) == "check" ) {
                CMainWindow::checkUpdates();
            }
#endif

            return true;
        } else
        if ( cmd.compare(L"title:button") == 0 ) {
            map<int, CCefEventsGate *>::const_iterator it = m_receivers.find(event->get_SenderId());
            if ( it != m_receivers.cend() ) {
                QMetaObject::invokeMethod(it->second, "onWebTitleChanged", Qt::QueuedConnection, Q_ARG(int, event->get_SenderId()), Q_ARG(std::wstring, pData->get_Param()));
                return true;
            }
        } else
        if ( !(cmd.find(L"go:folder") == std::wstring::npos) ) {
            if ( pData->get_Param() == L"offline" ) {}
            else {
                topWindow()->mainPanel()->onFileLocation(-1, QString::fromStdWString(pData->get_Param()));
                return true;
            }
        } else
        if ( cmd.compare(L"open:folder") == 0 ) {
            QString path = CEditorTools::getlocalfile(pData->get_Param());

            if ( !path.isEmpty() ) {
                CEditorWindow * editor = editorWindowFromUrl(path);
                if ( editor ) {
                    editor->bringToTop();
                } else {
                    CMainWindow * _w = mainWindowFromViewId(event->get_SenderId());
                    if ( _w ) {
                        _w->mainPanel()->doOpenLocalFiles(QStringList{path});
                    }
                }
            }

            return true;
        } else
        if ( cmd.compare(L"create:new") == 0 ) {
            wstring format = pData->get_Param();
            int _f = format == L"word" ? etDocument :
                        format == L"cell" ? etSpreadsheet :
                        format == L"slide" ? etPresentation : etUndefined;

            topWindow()->mainPanel()->createLocalFile(AscAppManager::newFileName(_f), _f);
            return true;
        }

        break; }

    case ASC_MENU_EVENT_TYPE_SSO_TOKEN: {
//        CAscSSOToken * pData = (CAscSSOToken *)_event->m_pData;
        return true; }

    case ASC_MENU_EVENT_TYPE_REPORTER_CREATE: {
        CSingleWindow * reporterWindow = createReporterWindow(event->m_pData, event->get_SenderId());
#ifdef __linux
        reporterWindow->show();
#else
        reporterWindow->show(false);
        reporterWindow->toggleBorderless(false);
#endif

//        RELEASEINTERFACE(event);
        return true; }

    case ASC_MENU_EVENT_TYPE_REPORTER_END: {
        // close editor window
        CAscTypeId * pData = static_cast<CAscTypeId *>(event->m_pData);

        if ( !m_winsReporter.empty() && m_winsReporter.find(pData->get_Id()) != m_winsReporter.end() ) {
            AscAppManager::getInstance().DestroyCefView(pData->get_Id());
        }

//        RELEASEINTERFACE(event);
        return true; }

    case ASC_MENU_EVENT_TYPE_REPORTER_MESSAGE_TO:
    case ASC_MENU_EVENT_TYPE_REPORTER_MESSAGE_FROM: return true;

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
        --m_countViews;

        if ( !m_winsReporter.empty() ) {
            std::map<int, CSingleWindow *>::const_iterator switer = m_winsReporter.find(event->get_SenderId());

            if (switer != m_winsReporter.end() ) {
                CSingleWindow * reporterWindow = switer->second;
                delete reporterWindow, reporterWindow = nullptr;
                m_winsReporter.erase(switer);

                return true;
            }
        }

        if ( m_closeTarget.compare(L"app") == 0 ) {
            switch ( m_countViews ) {
            case 1: DestroyCefView(-1); break;
            case 0: {
                CMainWindow * _w = reinterpret_cast<CMainWindow *>(m_vecWindows.at(0));
                if ( _w ) {
                    _w->hide();

                    delete _w, _w = nullptr;
                }

                m_vecWindows.clear();
            }
            default: break;
            }
        } else
        if ( m_closeCount && --m_closeCount == 0 && !m_closeTarget.empty() ) {
            if ( m_closeTarget.find(L"http") != wstring::npos ) {
                Logout(m_closeTarget);

                AscAppManager::sendCommandTo(SEND_TO_ALL_START_PAGE, L"portal:logout", m_closeTarget);
                m_closeTarget.clear();
            }
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
                    _json_obj["nameLocale"] = jdoc.object();
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

    case ASC_MENU_EVENT_TYPE_SYSTEM_EXTERNAL_MEDIA_START:
    case ASC_MENU_EVENT_TYPE_SYSTEM_EXTERNAL_MEDIA_END: {
        CCefView * _cef = GetViewById(event->get_SenderId());
        if ( _cef ) {
            CCefViewWidgetImpl * _impl = _cef->GetWidgetImpl();

            if ( _impl ) {
                event->m_nType == ASC_MENU_EVENT_TYPE_SYSTEM_EXTERNAL_MEDIA_START ?
                    static_cast<QCefView *>(_impl)->OnMediaStart(static_cast<CAscExternalMedia *>(event->m_pData)) :
                        static_cast<QCefView *>(_impl)->OnMediaEnd();
            }
        }

        return true;
    }

    case ASC_MENU_EVENT_TYPE_CEF_LOCALFILES_OPEN: {
        CAscLocalOpenFiles * pData = (CAscLocalOpenFiles *)event->m_pData;
        vector<wstring>& files = pData->get_Files();

        CEditorWindow * _editor;
        for (size_t i(files.size()); i --> 0;) {
            _editor = editorWindowFromUrl(QString::fromStdWString(files[i]));

            if ( _editor ) {
                files.erase(files.begin() + i);
#ifdef Q_OS_WIN
                SetForegroundWindow(_editor->handle());
#else
                _editor->activateWindow();
#endif
            }
        }
    }

    case ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_RECOVEROPEN:
    case ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_RECENTOPEN: {
        CAscLocalOpenFileRecent_Recover * data = (CAscLocalOpenFileRecent_Recover *)event->m_pData;
        CCefView * view = GetViewByRecentId(data->get_Id());
        if ( view ) {
            CEditorWindow * editor = editorWindowFromViewId(view->GetId());

            if ( editor ) {
#ifdef Q_OS_WIN
                SetForegroundWindow(editor->handle());
#else
                editor->activateWindow();
#endif
                return true;
            }
        }

        break;}

    case ASC_MENU_EVENT_TYPE_CEF_CREATETAB: {
        CEditorWindow * editor = editorWindowFromViewId(event->get_SenderId());
        if ( editor ) {
            QRect winrect{editor->geometry().translated(QPoint(50, 50))};

            CAscCreateTab& data = *static_cast<CAscCreateTab *>(event->m_pData);
            CTabPanel * _panel = CEditorTools::createEditorPanel(COpenOptions{data.get_Url()}, winrect.adjusted(4,4,-4,-4));
            _panel->data()->setContentType(editor->editorType());
            _panel->data()->setUrl("");

            CEditorWindow * editor_win = new CEditorWindow(winrect, _panel);
            editor_win->show(editor->windowState() == Qt::WindowMaximized);

            m_vecEditors.push_back( size_t(editor_win) );
//            sendCommandTo(_panel->cef(), L"window:features", Utils::encodeJson(QJsonObject{{"skiptoparea", TOOLBTN_HEIGHT}}).toStdWString());
            return true;
        }
        break;}

    case ASC_MENU_EVENT_TYPE_CEF_TABEDITORTYPE: {
        CCefView * pView = GetViewById(event->get_SenderId());
        if (NULL != pView && pView->GetType() == cvwtEditor) {
            CAscTabEditorType& data = *static_cast<CAscTabEditorType *>(event->m_pData);
            ((CCefViewEditor *)pView)->SetEditorType(AscEditorType(data.get_Type()));
        }
        break;
    }

    default: break;
    }

    return false;
}

void CAscApplicationManagerWrapper::broadcastEvent(NSEditorApi::CAscCefMenuEvent * event)
{
    CMainWindow * _window;
    for ( auto const& w : m_vecWindows ) {
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

CAscApplicationManagerWrapper & CAscApplicationManagerWrapper::getInstance()
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
//    _window->toggleBorderless(_is_maximized);

    if ( _is_maximized ) {
        WINDOWPLACEMENT wp{sizeof(WINDOWPLACEMENT)};
        if (GetWindowPlacement(_window->hWnd, &wp)) {
            wp.rcNormalPosition = {_start_rect.x(), _start_rect.y(), _start_rect.right(), _start_rect.bottom()};

            SetWindowPlacement(_window->hWnd, &wp);
        }
    }
#endif

    QStringList * _files = Utils::getInputFiles(g_cmdArgs);
    if ( _files ) {
        _window->mainPanel()->doOpenLocalFiles(*_files);
        if ( getInstance().m_private->allowedCreateLocalFile() ) {
            QRegularExpression re("^--new:(word|cell|slide)");
            QStringListIterator i(*_files);
            while (i.hasNext()) {
                QString n = i.next();
                if ( n.startsWith("--new:") ) {
                    QRegularExpressionMatch match = re.match(n);
                    if ( match.hasMatch() ) {
                        int _format;
                        if ( match.captured(1) == "word" ) _format = etDocument; else
                        if ( match.captured(1) == "cell" ) _format = etSpreadsheet; else
                        if ( match.captured(1) == "slide" ) _format = etPresentation;

                        _window->mainPanel()->createLocalFile(AscAppManager::newFileName(_format), _format);
                    }
                }
            }
        }
    }

#ifdef DOCUMENTSCORE_OPENSSL_SUPPORT
    APP_CAST(_app);
#endif
}

void CAscApplicationManagerWrapper::initializeApp()
{
    APP_CAST(_app);
    _app.m_private->initializeApp();

#ifdef _WIN32
//    CSplash::showSplash();
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

    CMainWindow * _window = new CMainWindow(rect);
    _app.m_vecWindows.push_back( size_t(_window) );

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

            _window->setReporterMode(true);
        }
    }

    if ( QApplication::desktop()->screenCount() > 1 ) {
        int _scrNum = QApplication::desktop()->screenNumber(_currentRect.topLeft());
        QRect _scrRect = QApplication::desktop()->screenGeometry(QApplication::desktop()->screenCount()-_scrNum-1);        
        int _srcDpiRatio = Utils::getScreenDpiRatio(_scrRect.topLeft());

        _windowRect.setSize(QSize(1000,700)*_srcDpiRatio);
        _windowRect.moveCenter(_scrRect.center());
    }

    CSingleWindow * reporterWindow = new CSingleWindow(_windowRect, tr("Presenter View") + " - " + _doc_name, pView);
    m_winsReporter[pView->GetCefView()->GetId()] = reporterWindow;

//    QTimer::singleShot(5000, [=]{
//        ::SetForegroundWindow((HWND)_window->handle());
//        ::FlashWindow((HWND)_window->handle(), TRUE);
//    });

    return reporterWindow;
}

void CAscApplicationManagerWrapper::closeMainWindow(const size_t p)
{
    APP_CAST(_app)

    size_t _size = _app.m_vecWindows.size();

    if ( _size > 1 ) {
//        vector<size_t>::iterator it = _app.m_vecWindows.begin();
//        while ( it != _app.m_vecWindows.end() ) {
//            if ( *it == p && _app.m_vecWindows.size() ) {
//                CMainWindow * _w = reinterpret_cast<CMainWindow*>(*it);

//                delete _w, _w = nullptr;

//                _app.m_vecWindows.erase(it);
//                break;
//            }

//            ++it;
//        }
    } else
    if ( _size == 1 && _app.m_vecWindows[0] == p ) {
        if ( _app.m_closeTarget.empty() ) {
            QTimer::singleShot(0, &_app, &CAscApplicationManagerWrapper::launchAppClose);
        }
    }
}

void CAscApplicationManagerWrapper::launchAppClose()
{
    if ( canAppClose() ) {
        CMainWindow * _w = reinterpret_cast<CMainWindow *>(m_vecWindows[0]);

        if ( m_countViews > 1 ) {
            m_closeTarget = L"app";

            /* close all editors windows */
            vector<size_t>::const_iterator it = m_vecEditors.begin();
            while ( it != m_vecEditors.end() ) {
                CEditorWindow * _w = reinterpret_cast<CEditorWindow *>(*it);

                int _r = _w->closeWindow();
                if ( _r == MODAL_RESULT_CANCEL ) {
                    AscAppManager::cancelClose();
                    return;
                } else ++it;
            }

            /* close main window */
            _w->bringToTop();
            if ( !_w->mainPanel()->closeAll() ) {
                AscAppManager::cancelClose();
                return;
            }
        }

        if ( !(m_countViews > 1) ) {
            DestroyCefView(-1);
        }

        closeQueue().leave(sWinTag{1,size_t(_w)});
    } else {
        cancelClose();
    }
}

void CAscApplicationManagerWrapper::closeEditorWindow(const size_t p)
{
    APP_CAST(_app)

    if ( p ) {
#if defined(__GNUC__) && __GNUC__ <= 4 && __GNUC_MINOR__ < 9
        vector<size_t>::iterator
#else
        vector<size_t>::const_iterator
#endif
        it = _app.m_vecEditors.begin();
        while ( it != _app.m_vecEditors.end() ) {
            if ( *it == p /*&& !_app.m_vecEditors.empty()*/ ) {
                CSingleWindowBase * _w = reinterpret_cast<CSingleWindowBase *>(*it);

                AscAppManager::unbindReceiver(static_cast<const CCefEventsGate *>(_w->receiver()));

                delete _w, _w = nullptr;

                it = _app.m_vecEditors.erase(it);
                break;
            } else ++it;
        }
    }
}

uint CAscApplicationManagerWrapper::countMainWindow()
{
    APP_CAST(_app)

    return _app.m_vecWindows.size();
}

CMainWindow * CAscApplicationManagerWrapper::mainWindowFromViewId(int uid) const
{
    CMainWindow * _window = nullptr;

    for (auto const& w : m_vecWindows) {
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

CEditorWindow * CAscApplicationManagerWrapper::editorWindowFromUrl(const QString& url) const
{
    CEditorWindow * _window = nullptr;

    for (auto const& w : m_vecEditors) {
        _window = reinterpret_cast<CEditorWindow *>(w);

        if ( _window->holdView(url.toStdWString()) )
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

    if ( _app.m_vecWindows.size() > 1 ) {
        CMainWindow * _window = nullptr,
                    * _source = reinterpret_cast<CMainWindow *>(s);
        for (auto const& w : _app.m_vecWindows) {
            if ( w != s ) {
                _window = reinterpret_cast<CMainWindow *>(w);

                if ( _window->pointInTabs(c) ) {
                    _source->hide();
                    _window->attachEditor(
                            _source->editor(GET_CURRENT_PANEL), c );

                    closeMainWindow(s);
                    break;
                }
            }
        }
    }
}

namespace Drop {
    const int drop_timeout = 300;
    auto callback_to_attach(const CEditorWindow * editor) -> void {
        if ( editor ) {
            CTabPanel * tabpanel = editor->releaseEditorView();
//            QJsonObject _json_obj{{"action", "undocking"},{"status", "docked"}};
//            CAscApplicationManagerWrapper::sendCommandTo(tabpanel->cef(), L"window:status", Utils::encodeJson(_json_obj).toStdWString());

            CAscApplicationManagerWrapper::topWindow()->attachEditor(tabpanel, QCursor::pos());
            CAscApplicationManagerWrapper::closeEditorWindow(size_t(editor));

            AscAppManager::sendCommandTo(tabpanel->cef(), L"window:features", Utils::encodeJson(QJsonObject{{"skiptoparea", 0}}).toStdWString());
            CAscApplicationManagerWrapper::topWindow()->bringToTop();
        }
    }


    size_t drop_handle;
    auto validate_drop(size_t handle, const QPoint& pt) -> void {
        CMainWindow * main_window = CAscApplicationManagerWrapper::topWindow();
        drop_handle = handle;

        static QPoint last_cursor_pos;
        static QTimer * drop_timer = nullptr;
        if ( !drop_timer ) {
            drop_timer = new QTimer;
            QObject::connect(qApp, &QCoreApplication::aboutToQuit, drop_timer, &QTimer::deleteLater);
            QObject::connect(drop_timer, &QTimer::timeout, []{
                CMainWindow * main_window = CAscApplicationManagerWrapper::topWindow();
                QPoint current_cursor = QCursor::pos();
                if ( main_window->pointInTabs(current_cursor) ) {
                    if ( current_cursor == last_cursor_pos ) {
                        drop_timer->stop();

                        if (QApplication::mouseButtons().testFlag(Qt::LeftButton))
                            callback_to_attach(CAscApplicationManagerWrapper::editorWindowFromHandle(drop_handle) );
                    } else {
                        last_cursor_pos = current_cursor;
                    }
                } else {
                    drop_timer->stop();
                }
            });
        }

        if ( main_window->pointInTabs(pt) ) {
            if ( !drop_timer->isActive() )
                drop_timer->start(drop_timeout);

            last_cursor_pos = QCursor::pos();
        } else
        if ( drop_timer->isActive() )
            drop_timer->stop();
    }
}

const CEditorWindow * CAscApplicationManagerWrapper::editorWindowFromHandle(size_t handle)
{
    APP_CAST(_app)

    for (auto const& w : _app.m_vecEditors) {
        CEditorWindow * e = reinterpret_cast<CEditorWindow *>(w);

        if ( (size_t)e->handle() == handle ) {
            return e;
        }
    }

    return nullptr;
}


void CAscApplicationManagerWrapper::editorWindowMoving(const size_t h, const QPoint& pt)
{
#if 1
    Drop::validate_drop(h,pt);
#else
    APP_CAST(_app)

    if ( _app.m_vecWindows.size() > 0 ) {
        CMainWindow * _main_window = reinterpret_cast<CMainWindow *>(_app.m_vecWindows.at(0));

        if ( _main_window && _main_window->pointInTabs(pt) ) {
            CSingleWindowBase * editor_win = nullptr;
            for (auto const& w : _app.m_vecEditors) {
                CEditorWindow * _e = reinterpret_cast<CEditorWindow *>(w);

                if ( (size_t)_e->handle() == h ) {
                    editor_win = _e;
                    break;
                }
            }

            if ( editor_win ) {
                CTabPanel * tabpanel = static_cast<CEditorWindow *>(editor_win)->releaseEditorView();
                QJsonObject _json_obj{{"action", "undocking"},{"status", "docked"}};
                sendCommandTo(tabpanel->cef(), L"window:status", Utils::encodeJson(_json_obj).toStdWString());

                SKIP_EVENTS_QUEUE([=]{
                    _main_window->attachEditor(tabpanel);

                    closeEditorWindow(size_t(editor_win));
                });
            }
        }
    }
#endif
}

CMainWindow * CAscApplicationManagerWrapper::topWindow()
{
    APP_CAST(_app);

    if ( _app.m_vecWindows.size() > 0 )
        return reinterpret_cast<CMainWindow *>(_app.m_vecWindows.at(0));
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

QString CAscApplicationManagerWrapper::getWindowStylesheets(int dpifactor)
{
    APP_CAST(_app);
    return Utils::readStylesheets(&_app.m_vecStyles, &_app.m_vecStyles2x, dpifactor);
}

bool CAscApplicationManagerWrapper::event(QEvent *event)
{
    if ( event->type() == CTabUndockEvent::type() ) {
        CMainWindow * _main_window = topWindow();
        if ( _main_window ) {
            CTabUndockEvent * e = static_cast<CTabUndockEvent *>(event);

            CTabPanel * _editor = nullptr;
            if ( e->panel() ) {
                _editor = static_cast<CTabPanel *>(e->panel());
            } else
            if ( !(e->index() < 0) ) {
                _editor = static_cast<CTabPanel *>(_main_window->editor(e->index()));
            }

            if ( _editor ) {
//                _editor->setParent(nullptr);
                e->accept();
//                QJsonObject _json_obj{{"action", "undocking"},
//                                      {"status", "undocked"}};
//                sendCommandTo(_editor->cef(), L"window:status", Utils::encodeJson(_json_obj).toStdWString());

//                SKIP_EVENTS_QUEUE([=]{
                    if ( _main_window ) {
                        QRect rect = _main_window->windowRect();

                        CEditorWindow * editor_win = new CEditorWindow(rect.translated(QPoint(50,50)), _editor);
                        editor_win->undock(_main_window->isMaximized());

                        m_vecEditors.push_back( size_t(editor_win) );
                        sendCommandTo(_editor->cef(), L"window:features", Utils::encodeJson(QJsonObject{{"skiptoparea", TOOLBTN_HEIGHT}}).toStdWString());
                    }
//                });
            }
        }

        return true;
    }

    return QObject::event(event);
}

bool CAscApplicationManagerWrapper::applySettings(const wstring& wstrjson)
{
    QJsonParseError jerror;
    QByteArray stringdata = QString::fromStdWString(wstrjson).toUtf8();
    QJsonDocument jdoc = QJsonDocument::fromJson(stringdata, &jerror);

    if( jerror.error == QJsonParseError::NoError ) {
        GET_REGISTRY_USER(_reg_user)
        _reg_user.setValue("appdata", stringdata.toBase64());

        QJsonObject objRoot = jdoc.object();
        QString _user_newname = objRoot["username"].toString();
        if ( _user_newname.isEmpty() )
            _user_newname = QString::fromStdWString(Utils::systemUserName());

        QString _lang_id = CLangater::getCurrentLangCode();
        if ( objRoot.contains("langid") ) {
            QString l = objRoot.value("langid").toString();
            if ( _lang_id != l ) {
                _lang_id = l;

                _reg_user.setValue("locale", _lang_id);
                CLangater::reloadTranslations(_lang_id);
            }
        }

        wstring params = QString("lang=%1&username=%3&location=%2")
                            .arg(_lang_id, Utils::systemLocationCode(), QUrl::toPercentEncoding(_user_newname)).toStdWString();

        if ( objRoot["docopenmode"].toString() == "view" )
            params.append(L"&mode=view");

#ifdef _UPDMODULE
        if ( objRoot.contains("checkupdatesrate") ) {
            CMainWindow::setAutocheckUpdatesInterval(objRoot.value("checkupdatesrate").toString());
        }
#endif

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
        for (auto const& w : m_vecWindows) {
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

bool CAscApplicationManagerWrapper::canAppClose()
{
#ifdef Q_OS_WIN
# ifdef _UPDMODULE
    if ( win_sparkle_is_processing() ) {
        CMessage mess(topWindow()->handle(), CMessageOpts::moButtons::mbYesNo);
        return mess.confirm(tr("Update is running. Break update and close the app?")) == MODAL_RESULT_CUSTOM;
    }
# endif
#endif

    APP_CAST(_app);

    if ( !_app.m_vecEditors.empty() ) {
        bool _has_opened_editors = std::find_if(_app.m_vecEditors.begin(), _app.m_vecEditors.end(),
                [](size_t h){
                    CEditorWindow * _e = reinterpret_cast<CEditorWindow *>(h);
                    return _e && !_e->closed();
                }) != _app.m_vecEditors.end();

        if ( _has_opened_editors ) {
            topWindow()->bringToTop();

            CMessage mess(topWindow()->handle(), CMessageOpts::moButtons::mbYesNo);
            if ( mess.warning(tr("Close all editors windows?")) == MODAL_RESULT_CUSTOM + 0 ) {
                return true;
            } else return false;
        }
    }

    return true;
}

QCefView * CAscApplicationManagerWrapper::createViewer(QWidget * parent)
{
    APP_CAST(_app);

    ++_app.m_countViews;
    return _app.m_private->createView(parent);
}

void CAscApplicationManagerWrapper::destroyViewer(int id)
{
    APP_CAST(_app);
    _app.DestroyCefView(id);
}

void CAscApplicationManagerWrapper::destroyViewer(QCefView * v)
{
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
            sendCommandTo(tabpanel->cef(), L"window:status", Utils::encodeJson(_json_obj).toStdWString());

            CMainWindow * main_win = topWindow();
            main_win->attachEditor(tabpanel);

            closeEditorWindow(size_t(editor_win));
        }
    } else {
        _json_obj["status"] = "undocked";

        CMainWindow * const main_win = mainWindowFromViewId(id);
        if ( main_win ) {
            int index = main_win->mainPanel()->tabWidget()->tabIndexByView(id);
            if ( !(index < 0) ) {
                QRect r = main_win->windowRect();
                tabpanel = qobject_cast<CTabPanel *>(main_win->editor(index));

                CTabUndockEvent event(tabpanel);
                QApplication::sendEvent(this, &event);
            }
        }
    }
}

uint CAscApplicationManagerWrapper::logoutCount(const wstring& portal) const
{
    uint _count = 0;
    CMainWindow * _window;
    for (auto const& w : m_vecWindows ) {
        _window = reinterpret_cast<CMainWindow *>(w);
        _count += _window->editorsCount(portal);
    }

    CEditorWindow * _editor;
    for (auto const& e : m_vecEditors ) {
        _editor = reinterpret_cast<CEditorWindow *>(e);
        if ( _editor->holdView(portal) )
            ++_count;
    }

    return _count;
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

void CAscApplicationManagerWrapper::unbindReceiver(const CCefEventsGate * receiver)
{
    APP_CAST(_app);

    map<int, CCefEventsGate *>::const_iterator it = _app.m_receivers.begin();
    while ( it != _app.m_receivers.cend() ) {
        if ( it->second == receiver ) {
            it = _app.m_receivers.erase(it);
            break;
        } else ++it;
    }

}

void CAscApplicationManagerWrapper::onDownloadSaveDialog(const std::wstring& name, uint id)
{
#ifdef Q_OS_WIN
    HWND parent = GetActiveWindow();
#else
    QWidget * parent = topWindow();
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

void CAscApplicationManagerWrapper::cancelClose()
{
    APP_CAST(_app);

    if ( _app.m_closeTarget.find(L"http") != wstring::npos ) {
        _app.sendCommandTo(SEND_TO_ALL_START_PAGE, L"portal:logout:cancel", _app.m_closeTarget);
    }

    _app.m_closeCount = 0;
    _app.m_closeTarget.clear();

    getInstance().closeQueue().cancel();
}

CWindowsQueue<sWinTag>& CAscApplicationManagerWrapper::closeQueue()
{
    return *m_queueToClose;
}

CEventDriver& CAscApplicationManagerWrapper::commonEvents()
{
    return m_eventDriver;
}

void CAscApplicationManagerWrapper::onQueueCloseWindow(const sWinTag& t)
{
    if ( t.type == 1 ) {
        closeMainWindow(t.handle);
    } else {
        CEditorWindow * _e = reinterpret_cast<CEditorWindow *>(t.handle);
        int res = _e->closeWindow();
        if ( res == MODAL_RESULT_CANCEL ) {
            AscAppManager::getInstance().closeQueue().cancel();
        } else {
            _e->hide();
            AscAppManager::getInstance().closeQueue().leave(t);
        }
    }
}

QString CAscApplicationManagerWrapper::newFileName(int format)
{
    static int docx_count = 0,
                 xlsx_count = 0,
                 pptx_count = 0;

    switch ( format ) {
    case etDocument:        return tr("Document%1.docx").arg(++docx_count);
    case etSpreadsheet:     return tr("Book%1.xlsx").arg(++xlsx_count);
    case etPresentation:    return tr("Presentation%1.pptx").arg(++pptx_count);
    default:                return "Document.asc";
    }
}

void CAscApplicationManagerWrapper::checkUpdates()
{
    APP_CAST(_app);

    if ( !_app.m_updater ) {
        _app.m_updater = std::make_shared<CAppUpdater>();
    }

    _app.m_updater->checkUpdates();
}
