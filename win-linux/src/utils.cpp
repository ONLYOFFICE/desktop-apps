/*
 * (c) Copyright Ascensio System SIA 2010-2019
 *
 * This program is a free software product. You can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License (AGPL)
 * version 3 as published by the Free Software Foundation. In accordance with
 * Section 7(a) of the GNU AGPL its Section 15 shall be amended to the effect
 * that Ascensio System SIA expressly excludes the warranty of non-infringement
 * of any third-party rights.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR  PURPOSE. For
 * details, see the GNU AGPL at: http://www.gnu.org/licenses/agpl-3.0.html
 *
 * You can contact Ascensio System SIA at 20A-12 Ernesta Birznieka-Upisha
 * street, Riga, Latvia, EU, LV-1050.
 *
 * The  interactive user interfaces in modified source and object code versions
 * of the Program must display Appropriate Legal Notices, as required under
 * Section 5 of the GNU AGPL version 3.
 *
 * Pursuant to Section 7(b) of the License you must retain the original Product
 * logo when distributing the program. Pursuant to Section 7(e) we decline to
 * grant you any rights under trademark law for use of our trademarks.
 *
 * All the Product's GUI elements, including illustrations and icon sets, as
 * well as technical writing content are licensed under the terms of the
 * Creative Commons Attribution-ShareAlike 4.0 International. See the License
 * terms at http://creativecommons.org/licenses/by-sa/4.0/legalcode
 *
*/

#include "utils.h"
#include "defines.h"
#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QRegularExpression>
#include <QIcon>
#include <QSysInfo>
#include <QApplication>
#include <QDesktopWidget>
#include <QDesktopServices>
#include <QUrl>
#include <QJsonDocument>
#include <QProcess>
#include <QScreen>
#include <QStorageInfo>
#include <regex>

#include "cascapplicationmanagerwrapper.h"
#include "qdpichecker.h"

#ifdef _WIN32
#include "shlobj.h"
#include "lmcons.h"
typedef HRESULT (__stdcall *SetCurrentProcessExplicitAppUserModelIDProc)(PCWSTR AppID);
#else
#include <sys/stat.h>
#endif

#include <QDebug>
extern QStringList g_cmdArgs;

namespace InputArgs {
    auto contains(const QString& param) -> bool {
        auto iter = std::find_if(begin(g_cmdArgs), end(g_cmdArgs),
            [&param](const QString& s) {
                return s.startsWith(param);
        });

        return iter != end(g_cmdArgs);
    }

    auto get_arg_value(const QString& param) -> QString {
        QRegularExpression _re("^" + param + "[=|:]([\\w\\\":\\\\/]+)", QRegularExpression::CaseInsensitiveOption);

        for (const auto& item: g_cmdArgs) {
            QRegularExpressionMatch _match = _re.match(item);
            if ( _match.hasMatch() )
                return _match.captured(1);
        }

        return QString();
    }
}

QStringList * Utils::getInputFiles(const QStringList& inlist)
{
    QStringList * _ret_files_list = nullptr;

    if ( !inlist.isEmpty() ) {
        _ret_files_list = new QStringList;

        QStringListIterator i(inlist);
        while (i.hasNext()) {
            QString arg = i.next();

            if ( arg.startsWith("--new:") || arg.startsWith("--new=") )
                _ret_files_list->append( arg );
            else {
                QFileInfo info( arg );
                if ( info.isFile() ) {
                    _ret_files_list->append(info.absoluteFilePath());
                }
            }
        }
    }

    return _ret_files_list;
}

QString Utils::lastPath(int t)
{
    GET_REGISTRY_USER(_reg_user)

    QString _path;
    if (t == LOCAL_PATH_OPEN)
        _path = _reg_user.value("openPath").value<QString>(); else
        _path = _reg_user.value("savePath").value<QString>();

    return !_path.isEmpty() && QDir(_path).exists() ?
        _path : QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
}

void Utils::keepLastPath(int t, const QString& p)
{
    GET_REGISTRY_USER(_reg_user)
    if (t == LOCAL_PATH_OPEN)
        _reg_user.setValue("openPath", p); else
        _reg_user.setValue("savePath", p);
}

bool Utils::makepath(const QString& p)
{
#ifdef __linux
    mode_t _mask = umask(0);
    (_mask & S_IRWXO) && umask(_mask & ~S_IRWXO);
#endif
    return QDir().mkpath(p);
}

QRect Utils::getScreenGeometry(const QPoint& leftTop)
{
//    int _scr_num = QApplication::desktop()->screenNumber(leftTop); - return the wrong number
//    return QApplication::desktop()->screenGeometry(_scr_num);
#ifdef __linux
    auto pointToRect = [](const QPoint &p, const QRect &r) -> int {
        int dx = 0, dy = 0;
        if (p.x() < r.left()) dx = r.left() - p.x(); else
        if (p.x() > r.right()) dx = p.x() - r.right();

        if (p.y() < r.top()) dy = r.top() - p.y(); else
        if (p.y() > r.bottom()) dy = p.y() - r.bottom();

        return dx + dy;
    };

    int closestScreen = 0;
    int shortestDistance = pointToRect(leftTop, QApplication::desktop()->screenGeometry(0));

    for (int i = 0; ++i < QApplication::desktop()->screenCount(); ) {
        int thisDistance = pointToRect(leftTop, QApplication::desktop()->screenGeometry(i));
        if (thisDistance < shortestDistance) {
            shortestDistance = thisDistance;
            closestScreen = i;
        }
    }

    return QApplication::desktop()->screenGeometry(closestScreen);
#else
    POINT lt{leftTop.x(), leftTop.y()};
    MONITORINFO mi{sizeof(MONITORINFO)};
    ::GetMonitorInfo(::MonitorFromPoint(lt, MONITOR_DEFAULTTOPRIMARY), &mi);

    return QRect(QPoint(mi.rcWork.left, mi.rcWork.top), QPoint(mi.rcWork.right, mi.rcWork.bottom));
#endif
}

QString Utils::systemLocationCode()
{
#define LOCATION_MAX_LEN 9
#ifdef _WIN32
    WCHAR _country_code[LOCATION_MAX_LEN]{0};
    // "no entry point for GetLocaleInfoEx" error on win_xp
//    if ( QSysInfo::windowsVersion() >= QSysInfo::WV_VISTA ) {
//        if (!GetLocaleInfoEx(LOCALE_NAME_SYSTEM_DEFAULT, LOCALE_SISO3166CTRYNAME, _country_code, LOCATION_MAX_LEN))
//            return "unknown";
//    } else {
        if (!GetLocaleInfo(LOCALE_SYSTEM_DEFAULT, LOCALE_SISO3166CTRYNAME, _country_code, LOCATION_MAX_LEN))
            return "unknown";
//    }

    return QString::fromWCharArray(_country_code);
#else
    QStringList list = QLocale().name().split('_');
    if (list.size() < 2)
        return "EN";

    return list.at(1);
#endif
}

void Utils::openUrl(const QString& url)
{
#ifdef __linux
    QUrl _url(url);
    if ( _url.scheme() == "mailto" ) {
        system(QString("LD_LIBRARY_PATH='' xdg-email %1")                   // xdg-email filepath email
                            .arg(QString( _url.toEncoded() )).toUtf8());
    } else {
        system(QString("LD_LIBRARY_PATH='' xdg-open %1")                    // xdg-open workingpath path
                            .arg(QString( _url.toEncoded() )).toUtf8());
    }
#else
    QDesktopServices::openUrl(QUrl(url));
#endif
}

void Utils::openFileLocation(const QString& path)
{
#if defined(Q_OS_WIN)
    QStringList args{"/select,", QDir::toNativeSeparators(path)};
    QProcess::startDetached("explorer", args);
#else
    static QString _file_browser;
    static QString _arg_select = "--no-desktop";
    if ( _file_browser.isEmpty() ) {
        auto _get_cmd_output = [](const QString& cmd, const QStringList& args, QString& error) {
            QProcess process;
            process.start(cmd, args);
            process.waitForFinished(-1);

            error = process.readAllStandardError();
            return process.readAllStandardOutput();
        };

        QString _error;
        QString _cmd_output = _get_cmd_output("xdg-mime", QStringList{"query", "default", "inode/directory"}, _error);

        //    if ( _error.isEmpty() )
        {
            if ( _cmd_output.contains(QRegularExpression("nautilus\\.desktop", QRegularExpression::CaseInsensitiveOption)) ||
                    _cmd_output.contains(QRegularExpression("nautilus-folder-handler\\.desktop", QRegularExpression::CaseInsensitiveOption)) )
            {
//                _cmd_output = _get_cmd_output("nautilus", QStringList{"--version"}, _error);

        //        if ( _error.isEmpty() )
                {
//                    QRegularExpression _regex("nautilus\\s(\\d{1,3})\\.(\\d{1,3})(?:\\.(\\d{1,5}))?");
//                    QRegularExpressionMatch match = _regex.match(_cmd_output);
//                    if ( match.hasMatch() ) {
//                        bool is_verion_supported = match.captured(1).toInt() > 3;
//                        if ( !is_verion_supported && match.captured(1).toInt() == 3 ) {
//                            is_verion_supported = match.captured(2).toInt() > 0;

//                            if ( !is_verion_supported && !match.captured(3).isEmpty() )
//                                is_verion_supported = !(match.captured(3).toInt() < 2);
//                        }

//                        is_file_browser_supported = is_verion_supported;
//                    }
                }

                _file_browser = "nautilus";
            } else
            if ( _cmd_output.contains(QRegularExpression("dolphin\\.desktop", QRegularExpression::CaseInsensitiveOption)) ) {
                _file_browser = "dolphin";
                _arg_select = "--select";
            } else
            if ( _cmd_output == "caja-folder-handler.desktop" ) {
                _file_browser = "caja";
            } else
            if ( _cmd_output == "nemo.desktop" ) {
                _file_browser = "nemo";
            } else
            if ( _cmd_output == "kfmclient_dir.desktop" ) {
                _file_browser = "konqueror";
                _arg_select = "--select";
            } else {
                _file_browser = "unknown";
            }
        }
    }

    QFileInfo fileInfo(path);
    if ( !_file_browser.isEmpty() && _file_browser != "unknown" ) {
        qputenv("LD_LIBRARY_PATH", "");
        QProcess::startDetached(_file_browser, QStringList{_arg_select, fileInfo.absoluteFilePath()});        
    } else
        system(QString("LD_LIBRARY_PATH='' xdg-open \"%1\"").arg(fileInfo.path()).toUtf8());
#endif
}

bool Utils::isFileLocal(const QString& path)
{
    QStorageInfo storage(QFileInfo(path).dir());

# ifdef Q_OS_WIN
    return storage.device().startsWith("\\\\?\\");
# else
    return storage.device().startsWith("/dev/");
# endif
}

QString Utils::getPortalName(const QString& url)
{
    if ( !url.isEmpty() ) {
        QRegularExpressionMatch match = QRegularExpression(rePortalName).match(url);
        if (match.hasMatch()) {
            QString out = match.captured(1);
            return out.endsWith('/') ? out.remove(-1, 1) : out;
        }
    }

    return url;
}

QString Utils::encodeJson(const QJsonObject& obj)
{
    return Utils::encodeJson(
                QJsonDocument(obj).toJson(QJsonDocument::Compact) );
}

QString Utils::encodeJson(const QString& s)
{
    return QString(s).replace("\"", "\\\"");
}

wstring Utils::encodeJson(const wstring& s)
{
#if defined(__GNUC__) && __GNUC__ <= 4 && __GNUC_MINOR__ < 9
    return QString::fromStdWString(s).replace("\"", "\\\"").toStdWString();
#else
    return std::regex_replace(wstring(s), std::wregex(L"\""), L"\\\"");
#endif
}

unsigned Utils::getScreenDpiRatio(int scrnum)
{
    unsigned int _dpi_x = 0;
    unsigned int _dpi_y = 0;
    int nScale = AscAppManager::getInstance().GetMonitorScaleByIndex(scrnum, _dpi_x, _dpi_y);
    return (-1 == nScale) ? 1 : nScale;
}

unsigned Utils::getScreenDpiRatio(const QPoint& pt)
{
    QWidget _w;
    _w.setGeometry(QRect(pt, QSize(10,10)));

    return getScreenDpiRatioByHWND(_w.winId());
}

unsigned Utils::getScreenDpiRatioByHWND(int hwnd)
{
    unsigned int _dpi_x = 0;
    unsigned int _dpi_y = 0;
    int nScale = AscAppManager::getInstance().GetMonitorScaleByWindow((WindowHandleId)hwnd, _dpi_x, _dpi_y);
    return (-1 == nScale) ? 1 : nScale;
}

unsigned Utils::getScreenDpiRatioByWidget(QWidget* wid)
{
    if (!wid)
        return 1;

    CAscDpiChecker* pDpiCheckerBase = CAscApplicationManager::GetDpiChecker();
    if (!pDpiCheckerBase)
        return 1;

    QDpiChecker * pDpiChecker = (QDpiChecker *)pDpiCheckerBase;
    unsigned int nDpiX = 0;
    unsigned int nDpiY = 0;
    int nRet = pDpiChecker->GetWidgetDpi(wid, &nDpiX, &nDpiY);

    if (nRet >= 0) {
        double dDpiApp = pDpiChecker->GetScale(nDpiX, nDpiY);

        // пока только 1 или 2
        return (dDpiApp > 1.9) ? 2 : 1;
    }

    return wid->devicePixelRatio();
}

/*
QByteArray Utils::getAppStylesheets(int scale)
{
    auto read_styles = [](const QString& dir) {
        QByteArray _css;
        QFile file;
        QFileInfoList files = QDir(dir).entryInfoList(QStringList("*.qss"), QDir::Files);
        foreach(const QFileInfo &info, files) {
            file.setFileName(info.absoluteFilePath());
            if ( file.open(QIODevice::ReadOnly | QIODevice::Text) ) {
                _css.append(file.readAll());
                file.close();
            }
        }

        return _css;
    };

    QByteArray _out( read_styles(QString(":styles/res/styles")) );

    if ( scale > 1 ) {
        _out.append( read_styles(QString(":styles@2x")) );
    }

    return _out;
}
*/

QByteArray Utils::readStylesheets(std::vector<QString> * list, std::vector<QString> * list2x, int scale)
{
    QByteArray _out = readStylesheets(list);

    if ( scale > 1 ) {
        _out.append( readStylesheets(list2x) );
    }

    return _out;
}

QByteArray Utils::readStylesheets(std::vector<QString> * list)
{
    auto read_styles = [](const std::vector<QString> * inl) {
        QByteArray _css;
        QFile file;
        for ( auto &path : *inl ) {
            file.setFileName(path);
            if ( file.open(QIODevice::ReadOnly | QIODevice::Text) ) {
                _css.append(file.readAll());
                file.close();
            }
        }

        return std::move(_css);
    };

    return read_styles(list);
}

QByteArray Utils::readStylesheets(const QString& path)
{
    QByteArray _css;
    QFile file(path);
    if ( file.open(QIODevice::ReadOnly | QIODevice::Text) ) {
        _css.append(file.readAll());
        file.close();
    }

    return _css;
}

QString Utils::replaceBackslash(const QString& path)
{
    return QString(path).replace(QRegularExpression("\\\\"), "/");
}

bool Utils::setAppUserModelId(const QString& modelid)
{
    bool _result = false;

#ifdef Q_OS_WIN
    HMODULE _lib_shell32 = ::LoadLibrary(L"shell32.dll");
    if ( _lib_shell32 != NULL ) {
        SetCurrentProcessExplicitAppUserModelIDProc setCurrentProcessExplicitAppUserModelId =
            reinterpret_cast<SetCurrentProcessExplicitAppUserModelIDProc>(GetProcAddress(_lib_shell32, "SetCurrentProcessExplicitAppUserModelID"));

        if ( setCurrentProcessExplicitAppUserModelId != NULL ) {
            _result = setCurrentProcessExplicitAppUserModelId(modelid.toStdWString().c_str()) == S_OK;
        }

        ::FreeLibrary(_lib_shell32);
    }
#endif

    return _result;
}

wstring Utils::systemUserName()
{
#ifdef Q_OS_WIN
    WCHAR _env_name[UNLEN + 1]{0};
    DWORD _size = UNLEN + 1;

    return GetUserName(_env_name, &_size) ?
                            wstring(_env_name) : L"Unknown.User";
#else
    QString _env_name = qgetenv("USER");
    if ( _env_name.isEmpty() ) {
        _env_name = qgetenv("USERNAME");

        if (_env_name.isEmpty())
            _env_name = "Unknown.User";
    }

    return _env_name.toStdWString();
#endif
}

wstring Utils::appUserName()
{
    GET_REGISTRY_USER(_reg_user)

    QString data = QByteArray::fromBase64(_reg_user.value("appdata").toByteArray());
    if (!data.isEmpty()) {
        QRegularExpression _re("username\\\":\\\"(.+?)\\\"");
        QRegularExpressionMatch _match = _re.match(data);
        if ( _match.hasMatch() )
            return _match.captured(1).toStdWString();
    }

    return systemUserName();
}

#ifdef Q_OS_WIN
#include <windowsx.h>
void Utils::adjustWindowRect(HWND handle, int dpiratio, LPRECT rect)
{
    typedef BOOL (__stdcall *AdjustWindowRectExForDpiW)(LPRECT lpRect, DWORD dwStyle, BOOL bMenu, DWORD dwExStyle, UINT dpi);

    static AdjustWindowRectExForDpiW _adjustWindowRectEx = NULL;
    static bool _is_read = false;
    if ( !_is_read && !_adjustWindowRectEx ) {
        HMODULE _lib = ::LoadLibrary(L"user32.dll");
        _adjustWindowRectEx = reinterpret_cast<AdjustWindowRectExForDpiW>(GetProcAddress(_lib, "AdjustWindowRectExForDpi"));
        FreeLibrary(_lib);

        _is_read = true;
    }

    if ( _adjustWindowRectEx != NULL ) {
        _adjustWindowRectEx(rect, (GetWindowStyle(handle) & ~WS_DLGFRAME), FALSE, 0, 96*dpiratio);
    } else AdjustWindowRectEx(rect, (GetWindowStyle(handle) & ~WS_DLGFRAME), FALSE, 0);
}
#endif

namespace WindowHelper {
#ifdef Q_OS_LINUX
    CParentDisable::CParentDisable(QWidget* parent)
    {
        disable(parent);
    }

    CParentDisable::~CParentDisable()
    {
        if (m_pChild)
            m_pChild->deleteLater();
    }

    void CParentDisable::disable(QWidget* parent)
    {
        if (parent) {
            if (QCefView::IsSupportLayers())
            {
                m_pChild = new QWidget(parent);
            }
            else
            {
                QWindow * win = new QWindow;
                win->setOpacity(1);
                m_pChild = QWidget::createWindowContainer(win, parent);
            }

            m_pChild->setMouseTracking(true);
            m_pChild->setGeometry(0, 0, parent->width(), parent->height());
            m_pChild->setStyleSheet("background-color: rgba(255,0,0,0)");
            m_pChild->setAttribute(Qt::WA_NoSystemBackground);
            m_pChild->setAttribute(Qt::WA_TranslucentBackground);
            m_pChild->show();
        }
    }

    void CParentDisable::enable()
    {
        if ( m_pChild ) {
            m_pChild->deleteLater();
            m_pChild = nullptr;
        }
    }
#else
    auto isLeftButtonPressed() -> bool {
        return (::GetKeyState(VK_LBUTTON) & 0x8000) != 0;
    }

    auto isWindowSystemDocked(HWND handle) -> bool {
        RECT windowrect;
        WINDOWPLACEMENT wp; wp.length = sizeof(WINDOWPLACEMENT);
        if ( GetWindowRect(handle, &windowrect) && GetWindowPlacement(handle, &wp) && wp.showCmd == SW_SHOWNORMAL ) {
            return (wp.rcNormalPosition.right - wp.rcNormalPosition.left != windowrect.right - windowrect.left) ||
                        (wp.rcNormalPosition.bottom - wp.rcNormalPosition.top != windowrect.bottom - windowrect.top);
        }

        return false;
    }

    auto correctWindowMinimumSize(HWND handle) -> void {
        WINDOWPLACEMENT wp; wp.length = sizeof(WINDOWPLACEMENT);
        if ( GetWindowPlacement(handle, &wp) ) {
            int dpi_ratio = Utils::getScreenDpiRatioByHWND((int)handle);
            QSize _min_windowsize{MAIN_WINDOW_MIN_WIDTH * dpi_ratio,MAIN_WINDOW_MIN_HEIGHT * dpi_ratio};
            QRect windowRect{QPoint(wp.rcNormalPosition.left, wp.rcNormalPosition.top),
                                    QPoint(wp.rcNormalPosition.right, wp.rcNormalPosition.bottom)};

            if ( windowRect.width() < _min_windowsize.width() ||
                    windowRect.height() < _min_windowsize.height() )
            {
//                if ( windowRect.width() < _min_windowsize.width() )
                    wp.rcNormalPosition.right = wp.rcNormalPosition.left + _min_windowsize.width();

//                if ( windowRect.height() < _min_windowsize.height() )
                    wp.rcNormalPosition.bottom = wp.rcNormalPosition.top + _min_windowsize.height();

                SetWindowPlacement(handle, &wp);
            }
        }
    }

    auto correctModalOrder(HWND windowhandle, HWND modalhandle) -> void
    {
        if ( !IsWindowEnabled(windowhandle) && modalhandle && modalhandle != windowhandle ) {
            SetActiveWindow(modalhandle);
            SetWindowPos(windowhandle, modalhandle, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
        }
    }

#endif
}
