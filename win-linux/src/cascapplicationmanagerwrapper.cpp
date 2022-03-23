
#include "cascapplicationmanagerwrapper.h"
//#include "src/cascapplicationmanagerwrapper_private.h"
#include "cascapplicationmanagerwrapperintf.h"

#include <QMutexLocker>
#include <QTimer>
#include <QDir>
#include <QDateTime>
#include <QDesktopWidget>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QProcess>
#include <algorithm>
#include <functional>

#include "cstyletweaks.h"
#include "defines.h"
#include "cfiledialog.h"
#include "utils.h"
#include "common/Types.h"
#include "common/File.h"
#include "ctabundockevent.h"
#include "clangater.h"
#include "cmessage.h"
#include "ceditortools.h"
#include "cfilechecker.h"
#include "OfficeFileFormats.h"

#ifdef _WIN32
# include <io.h>
# include "csplash.h"
# include <VersionHelpers.h>

# ifdef _UPDMODULE
   #include "3dparty/WinSparkle/include/winsparkle.h"
# endif
#else
# include <unistd.h>

# ifdef DOCUMENTSCORE_OPENSSL_SUPPORT
#  include "linux/cdialogcertificateinfo.h"
# endif
#endif

#include "../../../desktop-sdk/ChromiumBasedEditors/videoplayerlib/qascvideoview.h"


#define APP_CAST(app) \
    CAscApplicationManagerWrapper & app = static_cast<CAscApplicationManagerWrapper &>(AscAppManager::getInstance());

#define SKIP_EVENTS_QUEUE(callback) QTimer::singleShot(0, callback)

using namespace NSEditorApi;
using namespace std;
using namespace std::placeholders;

CAscApplicationManagerWrapper::CAscApplicationManagerWrapper(CAscApplicationManagerWrapper const&)
{

}

CAscApplicationManagerWrapper::CAscApplicationManagerWrapper(CAscApplicationManagerWrapper_Private * ptrprivate)
    : QObject()
    , QAscApplicationManager()
    , CCefEventsTransformer(nullptr)
    , m_queueToClose(new CWindowsQueue<sWinTag>)
    , m_private(ptrprivate)
{
    m_private->init();
    CAscApplicationManager::SetEventListener(this);

    QObject::connect(this, &CAscApplicationManagerWrapper::coreEvent, this, &CAscApplicationManagerWrapper::onCoreEvent);
    QObject::connect(CExistanceController::getInstance(), &CExistanceController::checked, this, &CAscApplicationManagerWrapper::onFileChecked);
    QObject::connect(&commonEvents(), &CEventDriver::onEditorClosed, this, &CAscApplicationManagerWrapper::onEditorWidgetClosed);

    m_queueToClose->setcallback(std::bind(&CAscApplicationManagerWrapper::onQueueCloseWindow,this, _1));

    NSBaseVideoLibrary::Init(nullptr);

    m_themes = std::make_shared<CThemes>();
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


    if ( m_pMainWindow ) {
#ifdef _WIN32
        delete m_pMainWindow, m_pMainWindow= nullptr;
#else
        m_pMainWindow->deleteLater();
#endif
    }

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

    if ( m_pMainWindow && m_pMainWindow->holdView(_event->get_SenderId()) ) {
        CCefEventsTransformer::OnEvent(m_pMainWindow->mainPanel(), _event);
    } else {
/**/
        map<int, CCefEventsGate *>::const_iterator it = m_receivers.find(_event->get_SenderId());
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

        if ( !(cmd.find(L"webapps:entry") == std::wstring::npos) ) {
            int sid = event->get_SenderId();
            CCefView * ptr = GetViewById(event->get_SenderId());
            if ( ptr ) {
#ifdef __OS_WIN_XP
                sendCommandTo(ptr, L"window:features", Utils::stringifyJson(QJsonObject{{"lockthemes", true}}).toStdWString());
#else
                // TODO: unlock for ver 6.4 because bug 50589
                // TODO: unlock for back compatibility with ver 6.4 on portals
                sendCommandTo(ptr, L"uitheme:changed", themes().current().id());
#endif

                if ( !((pData->get_Param()).find(L"fillform") == std::wstring::npos) ) {
                    if ( m_receivers.find(sid) != m_receivers.end() )
                        m_receivers[sid]->onWebAppsFeatures(sid,L"\"uitype\":\"fillform\"");
                }

                auto * editor = editorWindowFromViewId(event->get_SenderId());
                if ( editor && editor->isCustomWindowStyle() ) {
                    sendCommandTo(ptr, L"window:features", Utils::stringifyJson(QJsonObject{{"singlewindow",true}}).toStdWString());
                }
            }
            return true;
        } else
        if ( cmd.compare(L"portal:login") == 0 ) {
            AscAppManager::sendCommandTo(SEND_TO_ALL_START_PAGE, L"portal:login", pData->get_Param());
            return true;
        } else
        if ( cmd.compare(L"portal:logout") == 0 ) {
            const wstring& wjson = pData->get_Param();
            wstring wportal;

            QRegularExpression re("domain\":\"(https?:\\/\\/[^\\s\"]+)");
            QRegularExpressionMatch match = re.match(QString::fromStdWString(wjson));
            if ( match.hasMatch() )
                wportal = match.captured(1).toStdWString();

            if ( !wportal.empty() ) {
                if ( (m_closeCount = logoutCount(wportal)) > 0 ) {
                    m_closeTarget = wjson;
                    broadcastEvent(event);
                } else {
                    Logout(wjson);
                }
            }

//            RELEASEINTERFACE(event);
            return true;
        } else
#ifdef Q_OS_WIN
        if ( cmd.find(L"app:onready") != std::wstring::npos ) {
            if ( !IsWindowsVistaOrGreater() )
                sendCommandTo(SEND_TO_ALL_START_PAGE, "panel:hide", "connect");
        } else
#endif
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
                mainWindow()->mainPanel()->onFileLocation(-1, QString::fromStdWString(pData->get_Param()));
#ifdef Q_OS_LINUX
                mainWindow()->bringToTop();
#endif
                return true;
            }
        } else
//        if ( cmd.compare(L"open:folder") == 0 ) {
//            QString path = CEditorTools::getlocalfile(pData->get_Param());

//            if ( !path.isEmpty() ) {
//                CEditorWindow * editor = editorWindowFromUrl(path);
//                if ( editor ) {
//                    editor->bringToTop();
//                } else {
//                    CMainWindow * _w = mainWindowFromViewId(event->get_SenderId());
//                    if ( _w ) {
//                        _w->mainPanel()->doOpenLocalFiles(QStringList{path});
//                    }
//                }
//            }

//            return true;
//        } else
//        if ( cmd.compare(L"open:recent") == 0 ) {
//            QJsonObject objRoot = Utils::parseJson(pData->get_Param());
//            if ( !objRoot.isEmpty() ) {
//                objRoot["type"].toString();

//                COpenOptions opts{objRoot["path"].toString().toStdWString(), etRecentFile, objRoot["id"].toInt()};
//                opts.format = objRoot["type"].toInt();

//                QRegularExpression re(rePortalName);
//                QRegularExpressionMatch match = re.match(opts.url);

//                if ( !match.hasMatch() ) {
//                    QFileInfo _info(opts.url);
//                    if ( /*!data->get_IsRecover() &&*/ !_info.exists() ) {
//                        CMessage mess(m_pMainWindow->handle(), CMessageOpts::moButtons::mbYesDefNo);
//                        int modal_res = mess.warning(tr("%1 doesn't exists!<br>Remove file from the list?").arg(_info.fileName()));

//                        if (modal_res == MODAL_RESULT_CUSTOM) {
//                            AscAppManager::sendCommandTo(SEND_TO_ALL_START_PAGE, "file:skip", QString::number(opts.id));
//                        }

//                        return true;
//                    }
//                }

//                m_private->openDocument(opts);
////                    mainWindow()->mainPanel()->onLocalFileRecent(opts);
//            }

//            return true;
//        } else
//        if ( cmd.compare(L"create:new") == 0 ) {
//            wstring format = pData->get_Param();
//            int _f = format == L"word" ? AVS_OFFICESTUDIO_FILE_DOCUMENT_DOCX :
//                        format == L"cell" ? AVS_OFFICESTUDIO_FILE_SPREADSHEET_XLSX :
//                        format == L"slide" ? AVS_OFFICESTUDIO_FILE_PRESENTATION_PPTX : AVS_OFFICESTUDIO_FILE_UNKNOWN;

//            mainWindow()->mainPanel()->createLocalFile(AscAppManager::newFileName(_f), _f);
//            return true;
//        } else
        if ( !(cmd.find(L"uitheme:changed") == std::wstring::npos) ) {
            applyTheme( themes().parseThemeName(pData->get_Param()) );
            return true;
        } else
        if ( !(cmd.find(L"files:check") == std::wstring::npos) ) {
            CExistanceController::check(QString::fromStdWString(pData->get_Param()));
            return true;
//        } else
//        if ( cmd.compare(L"open:document") == 0 ) {
//            wstring _url = pData->get_Param();
//            if ( !_url.empty() ) {
//                CCefView * _view = GetViewByUrl(_url);
//                int _id = _view ? _view->GetId() : -1;
//                if ( _url.rfind(L"http://",0) == 0 || _url.rfind(L"https://",0) == 0 ) {
//                    mainWindow()->mainPanel()->onCloudDocumentOpen(_url, _id, true);
//                } else {
//                    /* open local file */
//                }
//            }
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

    case ASC_MENU_EVENT_TYPE_WINDOW_SHOW_CERTIFICATE: {
#ifdef DOCUMENTSCORE_OPENSSL_SUPPORT
        CAscX509CertificateData * pData = reinterpret_cast<CAscX509CertificateData *>(event->m_pData);

        CDialogCertificateInfo _dialog(mainWindow(), pData->get_Data());
        _dialog.exec();
#endif
        return true;
    }
    case ASC_MENU_EVENT_TYPE_PAGE_SELECT_OPENSSL_CERTIFICATE: {
#ifdef DOCUMENTSCORE_OPENSSL_SUPPORT
        CMainWindow * _window = mainWindowFromViewId(event->get_SenderId());
        if ( _window ) {
            _window->sendSertificate(event->get_SenderId());
        }
#endif
        return true; }
    case ASC_MENU_EVENT_TYPE_CEF_ONSAVE: {
        CAscDocumentOnSaveData * pData = reinterpret_cast<CAscDocumentOnSaveData *>(event->m_pData);

        if (pData->get_IsCancel()) {
            AscAppManager::cancelClose();
        }
        break; }
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
//            case 1: DestroyCefView(-1); break;
            case 0: {
                if ( m_pMainWindow ) {
                    m_pMainWindow->hide();

                    delete m_pMainWindow, m_pMainWindow = nullptr;
                }

                break;
            }
            default: break;
            }
        } else
        if ( m_closeCount && --m_closeCount == 0 && !m_closeTarget.empty() ) {
            if ( m_closeTarget.find(L"http") != wstring::npos ) {
                Logout(m_closeTarget);
                m_closeTarget.clear();
            }
        }
        else
        if ( m_closeTarget.find(L"main") != wstring::npos ) {
            if ( !m_vecEditors.empty() && mainWindow()->mainPanel()->tabWidget()->count() == 0 ) {
                m_closeTarget.clear();
                mainWindow()->hide();
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

            AscAppManager::sendCommandTo(SEND_TO_ALL_START_PAGE, "panel:external", Utils::stringifyJson(_json_obj));
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
        CMainWindow * mw = mainWindow();
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

    case ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_SAVE: {
        CEditorTools::processLocalFileSaveAs(event);
        return true; }

    case ASC_MENU_EVENT_TYPE_DOCUMENTEDITORS_OPENDIRECTORY_DIALOG: {
        CAscLocalOpenDirectoryDialog * data = static_cast<CAscLocalOpenDirectoryDialog *>(event->m_pData);
        std::wstring path = CEditorTools::getFolder(data->get_Path(), event->get_SenderId());

        data->put_Path(path);
        event->AddRef();

        AscAppManager::getInstance().Apply(event);
        return true; }

    case ASC_MENU_EVENT_TYPE_CEF_ONKEYBOARDDOWN: {
        CAscKeyboardDown * data = static_cast<CAscKeyboardDown *>(event->m_pData);

        switch ( data->get_KeyCode() ) {
        case VK_F4:
            if ( data->get_IsAlt() ) {
                CEditorWindow * editor = editorWindowFromViewId(event->get_SenderId());
                if ( editor ) {
                    editor->closeWindow();
                    return true;
                } else
                if ( mainWindow()->holdView(event->get_SenderId()) ) {
                     closeMainWindow();
                     return true;
                }
            }
        }
    }

    default: break;
    }

    return false;
}

void CAscApplicationManagerWrapper::broadcastEvent(NSEditorApi::CAscCefMenuEvent * event)
{
    if ( m_pMainWindow ) {
        ADDREFINTERFACE(event);
        CCefEventsTransformer::OnEvent(m_pMainWindow->mainPanel(), event);
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

auto prepareMainWindow(const QRect& r = QRect()) -> CMainWindow * {
    APP_CAST(_app);
    GET_REGISTRY_USER(reg_user);

    QRect _start_rect{r};
    if ( r.isEmpty() )
        _start_rect = reg_user.value("position").toRect();

    QPointer<QCefView> _startPanel = AscAppManager::createViewer(nullptr);
    _startPanel->Create(&_app, cvwtSimple);
    _startPanel->setObjectName("mainPanel");
    _startPanel->resize(_start_rect.width(), _start_rect.height());

    CMainWindow * _window = new CMainWindow(_start_rect);
    _window->mainPanel()->attachStartPanel(_startPanel);

    QString data_path;
#if defined(QT_DEBUG)
    data_path = reg_user.value("startpage").value<QString>();
#endif

    if ( data_path.isEmpty() )
        data_path = qApp->applicationDirPath() + "/index.html";

    QString additional = "?waitingloader=yes&lang=" + CLangater::getCurrentLangCode();
    QString _portal = reg_user.value("portal").value<QString>();
    if (!_portal.isEmpty()) {
        QString arg_portal = (additional.isEmpty() ? "?portal=" : "&portal=") + _portal;
        additional.append(arg_portal);
    }

#if defined(__OS_WIN_XP)
    additional.append("&osver=winxp");
#endif

    std::wstring start_path = ("file:///" + data_path + additional).toStdWString();
    _startPanel->GetCefView()->load(start_path);

    return _window;
}

void CAscApplicationManagerWrapper::handleInputCmd(const std::vector<wstring>& vargs)
{
    APP_CAST(_app);
    GET_REGISTRY_USER(reg_user);

    auto check_param = [] (const wstring& line, const wstring& param) {
        return line.find(param) == 0;
    };

    auto check_params = [] (const wstring& line, const std::vector<std::wstring>& params) {
        for (size_t i(0); i < params.size(); ++i) {
            if (line.find(params[i]) == 0)
                return int(i);
        }

        return -1;
    };

    QRect _start_rect = reg_user.value("position").toRect();

    const wstring prefix{L"--"};
    std::vector<std::wstring> arg_check_list{L"--review",L"--view",L"--edit"};
    std::vector<COpenOptions> list_failed;
//    bool open_in_new_window = std::find(vargs.begin(), vargs.end(), L"--force-use-tab") == std::end(vargs);
    bool open_in_new_window = _app.m_private->preferOpenEditorWindow() || (std::find(vargs.begin(), vargs.end(), L"--force-use-window") != std::end(vargs));
    for (const auto& arg: vargs) {
        COpenOptions open_opts;
        open_opts.name = QCoreApplication::translate("CAscTabWidget", "Document");
        open_opts.srctype = etUndefined;

        const size_t p = arg.find(prefix);
        if ( p == 0 ) {
            auto i = check_params(arg, arg_check_list);
            if ( !(i < 0) ) {
                auto c = arg_check_list[size_t(i)].size();
                if ( !(c < 0) )
                    open_opts.wurl = arg.substr(++c);

                open_in_new_window = true;
                open_opts.mode = i == 0 ? COpenOptions::eOpenMode::review :
                                    i == 1 ? COpenOptions::eOpenMode::view : COpenOptions::eOpenMode::edit;
            }else
            if ( check_param(arg, L"--new" ) ) {
                open_opts.srctype = etNewFile;
                open_opts.format = arg.rfind(L"cell") != wstring::npos ? AVS_OFFICESTUDIO_FILE_SPREADSHEET_XLSX :
                                    arg.rfind(L"slide") != wstring::npos ? AVS_OFFICESTUDIO_FILE_PRESENTATION_PPTX :
                                    arg.rfind(L"form") != wstring::npos ? AVS_OFFICESTUDIO_FILE_DOCUMENT_DOCXF :
                            /*if ( line.rfind(L"word") != wstring::npos )*/ AVS_OFFICESTUDIO_FILE_DOCUMENT_DOCX;

                open_opts.name = AscAppManager::newFileName(open_opts.format);
            } else continue;

//            if ( check_param(arg, L"single-window") )
//                in_new_window = true;
        } else {
            open_opts.wurl = arg;
        }

        if (open_opts.srctype == etUndefined) {
            if ( CFileInspector::isLocalFile(QString::fromStdWString(open_opts.wurl)) ) {
                open_opts.srctype = etLocalFile;
#ifdef Q_OS_WIN
                int _error = _waccess(open_opts.wurl.c_str(), 0);
                auto _c_pos = open_opts.wurl.rfind(L"\\");
                if ( _c_pos == std::wstring::npos )
                    _c_pos = open_opts.wurl.rfind(L"/");
#else
                int _error = access(U_TO_UTF8(open_opts.wurl).c_str(), F_OK);
                auto _c_pos = open_opts.wurl.rfind(QString(QDir::separator()).toStdWString());
#endif
                if ( _error == 0 ) {
                    _error = CCefViewEditor::GetFileFormat(open_opts.wurl) == 0 ? EBADF : 0;
                }

                if ( _error == 0 ) {
                    if ( _c_pos != std::wstring::npos )
                        open_opts.name = QString::fromStdWString(open_opts.wurl.substr(++_c_pos));

//                    open_opts.srctype = etLocalFile;
                } else
                if ( _error == EBADF && !open_in_new_window ) {
                    list_failed.push_back({open_opts});
                    continue;
                } else {
                    /* file doesn't exists */
                    open_opts.srctype = etNewFile;
                    open_opts.format = open_opts.format = AVS_OFFICESTUDIO_FILE_DOCUMENT_DOCX;
                    open_opts.name = AscAppManager::newFileName(open_opts.format);
                }
            } else
            if ( check_params(open_opts.wurl, {L"http://",L"https://",L"oo-office:"}) < 0 )
                continue;
        }

        // TODO: remove for ver 7.2. skip single window for --review flag without --forse-use-window
        if ( open_in_new_window )
            open_in_new_window = std::find(vargs.begin(), vargs.end(), L"--force-use-tab") == std::end(vargs);
        //

        CTabPanel * panel = CEditorTools::createEditorPanel(open_opts);
        if ( panel ) {
            if ( open_in_new_window ) {
                CEditorWindow * editor_win = new CEditorWindow(_start_rect, panel);
                editor_win->show(false);

                _app.m_vecEditors.push_back(size_t(editor_win));
                if ( editor_win->isCustomWindowStyle() )
                    sendCommandTo(panel->cef(), L"window:features",
                              Utils::stringifyJson(QJsonObject{{"skiptoparea", TOOLBTN_HEIGHT},{"singlewindow",true}}).toStdWString());
            } else {
                if ( !_app.m_pMainWindow ) {
                    _app.m_pMainWindow = prepareMainWindow(_start_rect);
                    _app.m_pMainWindow->show(reg_user.value("maximized", false).toBool());
                }

                _app.mainWindow()->attachEditor(panel);
            }
        }
    }

    if ( !list_failed.empty() && !open_in_new_window ) {
        if ( !_app.m_pMainWindow ) {
            _app.m_pMainWindow = prepareMainWindow(_start_rect);
            _app.m_pMainWindow->show(reg_user.value("maximized", false).toBool());
        }

        for ( auto & o : list_failed ) {
            COpenOptions opts{o};
            opts.url = QString::fromStdWString(opts.wurl);

            _app.m_pMainWindow->mainPanel()->doOpenLocalFile(opts);
        }
    }
}

void CAscApplicationManagerWrapper::startApp()
{
    APP_CAST(_app);
    GET_REGISTRY_USER(reg_user)

//    QRect _start_rect = reg_user.value("position").toRect();
    bool _is_maximized = reg_user.value("maximized", false).toBool();

#if 0
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
                        int _format = AVS_OFFICESTUDIO_FILE_UNKNOWN;
                        if ( match.captured(1) == "word" ) _format = AVS_OFFICESTUDIO_FILE_DOCUMENT_DOCX; else
                        if ( match.captured(1) == "cell" ) _format = AVS_OFFICESTUDIO_FILE_SPREADSHEET_XLSX; else
                        if ( match.captured(1) == "slide" ) _format = AVS_OFFICESTUDIO_FILE_PRESENTATION_PPTX;

                        _window->mainPanel()->createLocalFile(AscAppManager::newFileName(_format), _format);
                    }
                }
            }
        }
    }
#endif

    std::vector<std::wstring> in_args{InputArgs::arguments()};
    bool open_in_new_window = std::find(in_args.begin(), in_args.end(), L"--force-use-window") != std::end(in_args);
    bool files_in_args = std::find_if(in_args.begin(), in_args.end(),
                                     [](const std::wstring& arg){
                                            return (arg.rfind(L"--review", 0) != std::string::npos) || (arg.rfind(L"--view", 0) != std::string::npos) ||
                                                        (arg.rfind(L"--edit", 0) != std::string::npos) || arg.rfind(L"--", 0 == std::string::npos);
                                        }) != std::end(in_args);
    if ( !files_in_args && open_in_new_window ) {
        in_args.push_back(L"--new:word");
    }

    handleInputCmd(in_args);
    if ( _app.m_vecEditors.empty() && !_app.m_pMainWindow ) {
//        _app.m_private->createStartPanel();

//        CMainWindow * _window = createMainWindow(_start_rect);
//        _window->mainPanel()->attachStartPanel(_app.m_private->m_pStartPanel);
//        _window->show(_is_maximized);

        _app.m_pMainWindow = prepareMainWindow();
        _app.m_pMainWindow->show(_is_maximized);
    }

    QObject::connect(CExistanceController::getInstance(), &CExistanceController::checked, [] (const QString& name, int uid, bool exists) {
        if ( !exists ) {
            QJsonObject _json_obj{{QString::number(uid), exists}};
            QString json = QJsonDocument(_json_obj).toJson(QJsonDocument::Compact);

            AscAppManager::sendCommandTo(SEND_TO_ALL_START_PAGE, L"files:checked", json.toStdWString());
        }
    });
}

void CAscApplicationManagerWrapper::initializeApp()
{
    APP_CAST(_app);
    _app.m_private->initializeApp();

#ifdef _WIN32
//    CSplash::showSplash();
    QApplication::processEvents();
#else //defined(Q_OS_LINUX)
    if ( !InputArgs::contains(L"--single-window-app") ) {
        SingleApplication * app = static_cast<SingleApplication *>(QCoreApplication::instance());
        connect(app, &SingleApplication::showUp, [](QString args){
            std::vector<std::wstring> vec_inargs;
            QStringListIterator iter(args.split(";")); iter.next();
            while ( iter.hasNext() ) {
                QString arg = iter.next();
                if ( !arg.isEmpty() )
                    vec_inargs.push_back(arg.toStdWString());
            }

            if ( !vec_inargs.empty() ) {
                handleInputCmd(vec_inargs);
            }

//            QTimer::singleShot(0, []{
                if ( mainWindow() )
                    mainWindow()->bringToTop();
//            });
        });
    }
#endif

    /* prevent drawing of focus rectangle on a button */
//    QApplication::setStyle(new CStyleTweaks);

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

    _app.addStylesheets(CScalingFactor::SCALING_FACTOR_1, ":styles/res/styles/styles.qss");
    _app.addStylesheets(CScalingFactor::SCALING_FACTOR_1_25, ":styles@1.25x/styles.qss");
    _app.addStylesheets(CScalingFactor::SCALING_FACTOR_1_5, ":styles@1.5x/styles.qss");
    _app.addStylesheets(CScalingFactor::SCALING_FACTOR_1_75, ":styles@1.75x/styles.qss");
    _app.addStylesheets(CScalingFactor::SCALING_FACTOR_2, ":styles@2x/styles.qss");

    _app.m_private->applyStylesheets();

    // TODO: merge stylesheets and apply for the whole app
    qApp->setStyleSheet( Utils::readStylesheets(":styles/res/styles/styles.qss") );

    // Font
    QFont mainFont = QApplication::font();
    mainFont.setStyleStrategy( QFont::PreferAntialias );
    QApplication::setFont( mainFont );

    wstring wparams{InputArgs::webapps_params()};
    if ( !wparams.empty() ) wparams += L"&";
    wparams += QString("lang=%1&username=%3&location=%2").arg(CLangater::getCurrentLangCode(), Utils::systemLocationCode()).toStdWString();
    wstring user_name = Utils::appUserName();

    wparams.replace(wparams.find(L"%3"), 2, user_name);
    InputArgs::set_webapps_params(wparams);

    AscAppManager::getInstance().InitAdditionalEditorParams(wparams);
    AscAppManager::getInstance().applyTheme(themes().current().id(), true);

    QJsonObject jtheme{
        {"type", _app.m_themes->current().stype()},
        {"id", QString::fromStdWString(_app.m_themes->current().id())}
    };
#ifdef __OS_WIN_XP
    QJsonObject _json_obj{{"theme", jtheme}, {"os", "winxp"}};
#else
    QJsonObject _json_obj{{"theme", jtheme}};
#endif
    AscAppManager::getInstance().SetRendererProcessVariable(Utils::stringifyJson(_json_obj).toStdWString());
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

void CAscApplicationManagerWrapper::gotoMainWindow(size_t src)
{
    APP_CAST(_app)

    if ( !_app.m_pMainWindow ) {
        GET_REGISTRY_USER(reg_user)

        QRect _start_rect;
        if ( src ) {
            const CEditorWindow & _editor = *reinterpret_cast<CEditorWindow *>(src);
            _start_rect = _editor.geometry().translated(QPoint(50,50) * _editor.scaling());
        }

        _app.m_pMainWindow = prepareMainWindow(_start_rect);
        _app.m_pMainWindow->show(reg_user.value("maximized", false).toBool());
    }

    if ( !_app.m_pMainWindow->isVisible() )
        _app.m_pMainWindow->show(mainWindow()->isMaximized());

//    _app.m_pMainWindow->bringToTop();
    QTimer::singleShot(0, []{
        AscAppManager::mainWindow()->bringToTop();
    });
}

void CAscApplicationManagerWrapper::closeMainWindow()
{
    APP_CAST(_app)

    if ( _app.m_pMainWindow ) {
        if ( !_app.m_vecEditors.empty() ) {
//            CMessage m(mainWindow()->handle(), CMessageOpts::moButtons::mbYesNo);
//            m.setButtons({"Close all", "Current only", "Cancel"});
//            switch (m.warning(tr("Do you want to close all editor windows?"))) {
//            case MODAL_RESULT_CUSTOM + 0: break;
//            case MODAL_RESULT_CUSTOM + 1:
                if ( mainWindow()->mainPanel()->tabWidget()->count() ) {
                    _app.m_closeTarget = L"main";
                    QTimer::singleShot(0, []{
                        if ( mainWindow()->mainPanel()->tabCloseRequest() == MODAL_RESULT_CANCEL )
                            AscAppManager::cancelClose();
                    });
                } else {
                    mainWindow()->hide();
                }
                return;
//            default: return;
//            }
        }

        if ( _app.m_closeTarget.empty() ) {
            QTimer::singleShot(0, &_app, &CAscApplicationManagerWrapper::launchAppClose);
        }
    }
}

void CAscApplicationManagerWrapper::launchAppClose()
{
    if ( canAppClose() ) {
        if ( m_countViews > 1 ) {
            m_closeTarget = L"app";

            /* close all editors windows */
            if ( !m_vecEditors.empty() ) {
                vector<size_t>::const_iterator it = m_vecEditors.begin();
                if ( it != m_vecEditors.end() ) {
                    CEditorWindow * _editor = reinterpret_cast<CEditorWindow *>(*it);

                    if ( _editor && _editor->closeWindow() == MODAL_RESULT_CANCEL )
                        AscAppManager::cancelClose();
                }
            } else {
                if ( AscAppManager::mainWindow()->mainPanel()->tabCloseRequest() == MODAL_RESULT_CANCEL )
                    AscAppManager::cancelClose();
            }
        } else
        if ( !(m_countViews > 1) ) {
            DestroyCefView(-1);

            if ( m_pMainWindow ) {
                closeQueue().leave(sWinTag{1,size_t(m_pMainWindow)});
            }
        }
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

        if ( _app.m_vecEditors.empty() ) {
            if ( !_app.m_pMainWindow || !_app.m_pMainWindow->isVisible() ) {
                if ( _app.m_closeTarget.empty() ) {
                    QTimer::singleShot(0, &_app, &CAscApplicationManagerWrapper::launchAppClose);
                }
            }
        }
    }
}

CMainWindow * CAscApplicationManagerWrapper::mainWindowFromViewId(int uid) const
{
    return m_pMainWindow && m_pMainWindow->holdView(uid) ? m_pMainWindow : nullptr;
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

namespace Drop {
    const int drop_timeout = 300;
    auto callback_to_attach(const CEditorWindow * editor) -> void {
        if ( editor ) {
            CTabPanel * tabpanel = editor->releaseEditorView();

            CAscApplicationManagerWrapper::mainWindow()->attachEditor(tabpanel, QCursor::pos());
            CAscApplicationManagerWrapper::closeEditorWindow(size_t(editor));

            AscAppManager::sendCommandTo(tabpanel->cef(), L"window:features",
                      Utils::stringifyJson(QJsonObject{{"skiptoparea", 0},{"singlewindow",false}}).toStdWString());
            CAscApplicationManagerWrapper::mainWindow()->bringToTop();
        }
    }


    size_t drop_handle;
    auto validate_drop(size_t handle, const QPoint& pt) -> void {
        CMainWindow * main_window = CAscApplicationManagerWrapper::mainWindow();
        if ( main_window && main_window->isVisible() ) {
            drop_handle = handle;

            static QPoint last_cursor_pos;
            static QTimer * drop_timer = nullptr;
            if ( !drop_timer ) {
                drop_timer = new QTimer;
                QObject::connect(qApp, &QCoreApplication::aboutToQuit, drop_timer, &QTimer::deleteLater);
                QObject::connect(drop_timer, &QTimer::timeout, []{
                    CMainWindow * main_window = CAscApplicationManagerWrapper::mainWindow();
                    QPoint current_cursor = QCursor::pos();
                    if ( main_window->pointInTabs(current_cursor) ) {
                        if ( current_cursor == last_cursor_pos ) {
                            drop_timer->stop();

                            if ( WindowHelper::isLeftButtonPressed() )
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
                SKIP_EVENTS_QUEUE([=]{
                    _main_window->attachEditor(tabpanel);

                    closeEditorWindow(size_t(editor_win));
                });
            }
        }
    }
#endif
}

CMainWindow * CAscApplicationManagerWrapper::mainWindow()
{
    APP_CAST(_app);
    return _app.m_pMainWindow;
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

QString CAscApplicationManagerWrapper::getWindowStylesheets(double dpifactor)
{
    if ( dpifactor > 1.75 )
        return getWindowStylesheets(CScalingFactor::SCALING_FACTOR_2);
    else
    if ( dpifactor > 1.5 )
        return getWindowStylesheets(CScalingFactor::SCALING_FACTOR_1_75);
    else
    if ( dpifactor > 1.25 )
        return getWindowStylesheets(CScalingFactor::SCALING_FACTOR_1_5);
    else
    if ( dpifactor > 1 )
        return getWindowStylesheets(CScalingFactor::SCALING_FACTOR_1_25);
    else return getWindowStylesheets(CScalingFactor::SCALING_FACTOR_1);
}

QString CAscApplicationManagerWrapper::getWindowStylesheets(CScalingFactor factor)
{
    APP_CAST(_app);

    QByteArray _out = Utils::readStylesheets(&_app.m_mapStyles[CScalingFactor::SCALING_FACTOR_1]);
    if ( factor != CScalingFactor::SCALING_FACTOR_1 )
        _out.append(Utils::readStylesheets(&_app.m_mapStyles[factor]));

    return _out;
}

bool CAscApplicationManagerWrapper::event(QEvent *event)
{
    if ( event->type() == CTabUndockEvent::type() ) {
        CMainWindow * _main_window = mainWindow();
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
//                SKIP_EVENTS_QUEUE([=]{
                    if ( _main_window ) {
                        QRect rect = _main_window->windowRect();

                        CEditorWindow * editor_win = new CEditorWindow(rect.translated(QPoint(50,50)), _editor);
                        editor_win->undock(_main_window->isMaximized());

                        m_vecEditors.push_back( size_t(editor_win) );
                        if ( editor_win->isCustomWindowStyle() )
                            sendCommandTo(_editor->cef(), L"window:features",
                                    Utils::stringifyJson(QJsonObject{{"skiptoparea", TOOLBTN_HEIGHT},{"singlewindow",true}}).toStdWString());
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

        if ( objRoot.contains("uiscaling") ) {
            wstring sets;
            switch (objRoot["uiscaling"].toString().toInt()) {
            case 100: sets = L"1"; break;
            case 125: sets = L"1.25"; break;
            case 150: sets = L"1.5"; break;
            case 175: sets = L"1.75"; break;
            case 200: sets = L"2"; break;
            default: sets = L"default";
            }

            setUserSettings(L"force-scale", sets);
            m_pMainWindow->updateScaling();

            CEditorWindow * _editor = nullptr;
            foreach ( auto const& e, m_vecEditors ) {
                _editor = reinterpret_cast<CEditorWindow *>(e);
                _editor->updateScaling();
            }
        }

        if ( objRoot.contains("spellcheckdetect") ) {
            setUserSettings(L"spell-check-input-mode", objRoot["spellcheckdetect"].toString() == "off" ? L"0" : L"default");
        }

        wstring params = QString("lang=%1&username=%3&location=%2")
                            .arg(_lang_id, Utils::systemLocationCode(), QUrl::toPercentEncoding(_user_newname)).toStdWString();

        if ( objRoot["docopenmode"].toString() == "view" ) {
            params.append(L"&mode=view");
        }

#ifdef _UPDMODULE
        if ( objRoot.contains("checkupdatesrate") ) {
            CMainWindow::setAutocheckUpdatesInterval(objRoot.value("checkupdatesrate").toString());
        }
#endif

        InputArgs::set_webapps_params(params);
        AscAppManager::getInstance().InitAdditionalEditorParams( params );

        if ( objRoot.contains("uitheme") ) {
            applyTheme(objRoot["uitheme"].toString().toStdWString());
        }

        if ( objRoot.contains("editorwindowmode") ) {
            m_private->m_openEditorWindow = objRoot["editorwindowmode"].toBool();
            _reg_user.setValue("editorWindowMode", m_private->m_openEditorWindow);
        }
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

        if ( m_pMainWindow && m_pMainWindow->editorsCount() ) {
            _send_opts = L"has";
        }
    }

    if ( !_send_opts.empty() )
        QTimer::singleShot(0, [_send_cmd, _send_opts] {
            AscAppManager::sendCommandTo(SEND_TO_ALL_START_PAGE, _send_cmd, _send_opts);
        });
}

void CAscApplicationManagerWrapper::applyTheme(const wstring& theme, bool force)
{
    APP_CAST(_app);

    if ( !_app.m_themes->isCurrent(theme) ) {
        const std::wstring old_theme = _app.m_themes->current().id();
        _app.m_themes->setCurrentTheme(theme);

        std::wstring params{InputArgs::change_webapps_param(L"&uitheme=" + old_theme, L"&uitheme=" + theme)};
        AscAppManager::getInstance().InitAdditionalEditorParams(params);

        QJsonObject jtheme{
            {"type", _app.m_themes->current().stype()},
            {"id", QString::fromStdWString(_app.m_themes->current().id())}
        };
#ifdef __OS_WIN_XP
        QJsonObject _json_obj{{"theme", jtheme}, {"os", "winxp"}};
#else
        QJsonObject _json_obj{{"theme", jtheme}};
#endif
        AscAppManager::getInstance().SetRendererProcessVariable(Utils::stringifyJson(_json_obj).toStdWString());

        // TODO: remove
        if ( mainWindow() ) mainWindow()->applyTheme(theme);

        for ( auto const& r : m_winsReporter ) {
            r.second->applyTheme(theme);
        }


        CEditorWindow * _editor = nullptr;
        for ( auto const& e : m_vecEditors ) {
            _editor = reinterpret_cast<CEditorWindow *>(e);
            _editor->applyTheme(theme);
        }

        AscAppManager::sendCommandTo(SEND_TO_ALL_START_PAGE, L"uitheme:changed", theme);
    }
}

CThemes& CAscApplicationManagerWrapper::themes()
{
    return *(AscAppManager::getInstance().m_themes);
}

bool CAscApplicationManagerWrapper::canAppClose()
{
#ifdef Q_OS_WIN
# ifdef _UPDMODULE
    if ( win_sparkle_is_processing() ) {
        CMessage mess(mainWindow()->handle(), CMessageOpts::moButtons::mbYesNo);
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
            mainWindow()->bringToTop();

//            CMessage mess(mainWindow()->handle(), CMessageOpts::moButtons::mbYesNo);
//            mess.setButtons({"Yes", "No", "Hide main window"});
//            switch (mess.warning(tr("Close all editors windows?"))) {
//            case MODAL_RESULT_CUSTOM + 0: return true;
//            case MODAL_RESULT_CUSTOM + 2:
                mainWindow()->hide();
                return false;
//            default: return false;
//            }
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

uint CAscApplicationManagerWrapper::logoutCount(const wstring& portal) const
{
    uint _count = m_pMainWindow ? m_pMainWindow->editorsCount(portal) : 0;

    CEditorWindow * _editor;
    for (auto const& e : m_vecEditors ) {
        _editor = reinterpret_cast<CEditorWindow *>(e);
        if ( _editor->holdView(portal) )
            ++_count;
    }

    return _count;
}

void CAscApplicationManagerWrapper::Logout(const wstring& wjson)
{
    if ( !wjson.empty() ) {
        QJsonParseError jerror;
        QByteArray stringdata = QString::fromStdWString(wjson).toUtf8();
        QJsonDocument jdoc = QJsonDocument::fromJson(stringdata, &jerror);

        if( jerror.error == QJsonParseError::NoError ) {
            QJsonObject objRoot = jdoc.object();
            const wstring& portal = objRoot["domain"].toString().toStdWString();

            CAscApplicationManager::Logout(portal);
            if ( objRoot.contains("extra") && objRoot["extra"].isArray() ) {
                QJsonArray a = objRoot["extra"].toArray();
                for (auto v: a)
                    CAscApplicationManager::Logout(v.toString().toStdWString());
            }

            sendCommandTo(SEND_TO_ALL_START_PAGE, L"portal:logout", portal);

            int index = mainWindow()->mainPanel()->tabWidget()->tabIndexByUrl(portal);
            if ( !(index < 0) ) {
                if ( objRoot.contains("onsuccess") &&
                        objRoot["onsuccess"].toString() == "reload" )
                {
                    mainWindow()->mainPanel()->tabWidget()->panel(index)->cef()->reload();
                } else mainWindow()->mainPanel()->tabWidget()->closeEditorByIndex(index);
            }
        }
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
    QWidget * parent = mainWindow();
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
        closeMainWindow();
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
    case AVS_OFFICESTUDIO_FILE_DOCUMENT_DOCX:        return tr("Document%1.docx").arg(++docx_count);
    case AVS_OFFICESTUDIO_FILE_DOCUMENT_DOCXF:       return tr("Document%1.docx").arg(++docx_count) + "f";
    case AVS_OFFICESTUDIO_FILE_SPREADSHEET_XLSX:     return tr("Book%1.xlsx").arg(++xlsx_count);
    case AVS_OFFICESTUDIO_FILE_PRESENTATION_PPTX:    return tr("Presentation%1.pptx").arg(++pptx_count);
    default:                                         return "Document.asc";
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

wstring CAscApplicationManagerWrapper::userSettings(const wstring& name)
{
    unique_ptr<CUserSettings> pSettings{AscAppManager::getInstance().GetUserSettings()};
    return pSettings->Get(name);
//    delete pSettings;
}

void CAscApplicationManagerWrapper::setUserSettings(const wstring& name, const wstring& value)
{
    unique_ptr<CUserSettings> pSettings{AscAppManager::getInstance().GetUserSettings()};
    pSettings->Set(name, value);
}

void CAscApplicationManagerWrapper::onFileChecked(const QString& name, int uid, bool exists)
{
    Q_UNUSED(name)

    if ( !exists ) {
        QJsonObject _json_obj{{QString::number(uid), exists}};
        QString json = QJsonDocument(_json_obj).toJson(QJsonDocument::Compact);

        sendCommandTo(SEND_TO_ALL_START_PAGE, "files:checked", json);
    }
}

void CAscApplicationManagerWrapper::onEditorWidgetClosed()
{
    if ( m_closeTarget == L"app" ) {
        launchAppClose();
    } else
    if ( m_closeTarget == L"main" ) {
        if ( mainWindow()->mainPanel()->tabCloseRequest() == MODAL_RESULT_CANCEL )
            AscAppManager::cancelClose();
        else
        if ( mainWindow()->mainPanel()->tabWidget()->count() == 0 ) {
            m_closeTarget.clear();
            mainWindow()->hide();
        }
    }
}

void CAscApplicationManagerWrapper::addStylesheets(CScalingFactor f, const std::string& path)
{
    if ( m_mapStyles.find(f) == m_mapStyles.end() ) {
        m_mapStyles[f] = {path};
    } else m_mapStyles[f].push_back(path);

}
