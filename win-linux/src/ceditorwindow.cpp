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

#include "ceditorwindow.h"
#include "utils.h"
#include "cwindowbase.h"
#include "defines.h"
#include "cascapplicationmanagerwrapper.h"
#include "cfiledialog.h"
#include "cmessage.h"
#include "../Common/OfficeFileFormats.h"
#include "common/Types.h"

#include <QGridLayout>
#include <QPushButton>
#include <QRegion>
#include <QLabel>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDir>

#include "ceditorwindow_p.h"

QString g_css =
        "#mainPanel{background-color:#aa5252;}"
        "#box-title-tools[editor=word]{background-color:#446995;}"
        "#box-title-tools[editor=cell]{background-color:#40865c;}"
        "#box-title-tools[editor=slide]{background-color:#aa5252;}"
        "QPushButton[act=tool]:hover{background-color:rgba(255,255,255,20%)}"
        "QPushButton#toolButtonClose:hover{background-color:#d42b2b;}"
        "QPushButton#toolButtonClose:pressed{background-color:#d75050;}"
        "QPushButton#toolButtonMinimize,QPushButton#toolButtonClose {background-image:url(:/minclose_light.png);}"
        "QPushButton#toolButtonMaximize{background-image:url(:/max_light.png);}"
        "QPushButton#toolButtonDock{background-origin:content; padding: 5px 12px 7px; background-image:url(:/dock.png);background-position:left top;}"
        "#labelTitle{color:white;font-size:11px;padding-bottom:2px;}";

QString g_css2x =
        "QPushButton#toolButtonMinimize,QPushButton#toolButtonClose {background-image:url(:/minclose_light_2x.png);}"
        "#toolButtonMinimize,#toolButtonClose,#toolButtonMaximize{padding: 10px 24px 14px;}"
        "QPushButton#toolButtonMaximize{background-image:url(:/max_light_2x.png);}"
        "QPushButton#toolButtonDock{background-image:url(:/dock_2x.png); padding: 10px 24px 14px;}"
        "#labelTitle{font-size:24px;padding-bottom:5px;}";


CEditorWindow::CEditorWindow()
    : CSingleWindowPlatform(QRect(100, 100, 900, 800), "Desktop Editor", nullptr)
{
}

CEditorWindow::CEditorWindow(const QRect& rect, CTabPanel* panel)
    : CSingleWindowPlatform(rect, panel->data()->title(), panel)
    , d_ptr(new CEditorWindowPrivate(this))
{
    d_ptr.get()->init(panel);

#ifdef Q_OS_LINUX
    setObjectName("editorWindow");
    m_pMainPanel = createMainPanel(this, panel);
    setCentralWidget(m_pMainPanel);

    QString background = "#editorWindow{background-color:";
    switch (panel->data()->contentType()) {
    case etDocument: background.append(TAB_COLOR_DOCUMENT";}"); break;
    case etPresentation: background.append(TAB_COLOR_PRESENTATION";}"); break;
    case etSpreadsheet: background.append(TAB_COLOR_SPREADSHEET";}"); break;
    default:break;
    }

    setStyleSheet(background);

    if ( !CX11Decoration::isDecorated() ) {
        CX11Decoration::setTitleWidget(m_boxTitleBtns);
        m_pMainPanel->setMouseTracking(true);
        setMouseTracking(true);
    }
#else
    QColor color;
    switch (panel->data()->contentType()) {
    case etDocument: color = QColor(TAB_COLOR_DOCUMENT); break;
    case etPresentation: color = QColor(TAB_COLOR_PRESENTATION); break;
    case etSpreadsheet: color = QColor(TAB_COLOR_SPREADSHEET); break;
    }

    m_bgColor = RGB(color.red(), color.green(), color.blue());

    m_pMainPanel = createMainPanel(m_pWinPanel, panel);
    m_pWinPanel->show();
#endif

    QTimer::singleShot(0, [=]{m_pMainView->show();});
    AscAppManager::bindReceiver(panel->cef()->GetId(), d_ptr.get());
    AscAppManager::sendCommandTo(panel->cef(), L"editor:config", L"request");

    QObject::connect(d_ptr.get()->buttonDock(), &QPushButton::clicked, [=]{
        CAscApplicationManagerWrapper & app = static_cast<CAscApplicationManagerWrapper &>(AscAppManager::getInstance());

        app.manageUndocking(
                    qobject_cast<CTabPanel *>(m_pMainView)->view()->GetCefView()->GetId(), L"dock");
    });
}

CEditorWindow::CEditorWindow(const QRect& r, const QString& s, QWidget * w)
    : CSingleWindowPlatform(r,s,w)
{

}

CEditorWindow::~CEditorWindow()
{
    m_pMainPanel->deleteLater();
}

bool CEditorWindow::holdView(int id) const
{
    return qobject_cast<CTabPanel *>(m_pMainView)->view()->GetCefView()->GetId() == id;
}

bool CEditorWindow::holdView(const wstring& portal) const
{
    return qobject_cast<CTabPanel *>(m_pMainView)->data()->url().find(portal) != wstring::npos;
}

int CEditorWindow::closeWindow()
{
    d_ptr.get()->onFullScreen(false);

    CTabPanel * panel = d_ptr.get()->panel();

    int _reply = MODAL_RESULT_YES;
    if ( panel->data()->hasChanges() && !panel->data()->closed() ) {
        if (windowState() == Qt::WindowMinimized)
            setWindowState(Qt::WindowNoState);

        activateWindow();

        CMessage mess(handle(), CMessageOpts::moButtons::mbYesDefNoCancel);
//            modal_res = mess.warning(getSaveMessage().arg(m_pTabs->titleByIndex(index)));
        _reply = mess.warning(tr("%1 has been changed. Save changes?").arg(panel->data()->title(true)));

        switch (_reply) {
        case MODAL_RESULT_CUSTOM + 1:
            _reply = MODAL_RESULT_YES;
            break;
        case MODAL_RESULT_CANCEL:
        case MODAL_RESULT_CUSTOM + 2:
            return MODAL_RESULT_CANCEL;

        case MODAL_RESULT_CUSTOM + 0:
        default:
            panel->data()->close();
            panel->cef()->Apply(new CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_SAVE));

            _reply = MODAL_RESULT_NO;
            break;
        }
    }

    if ( _reply == MODAL_RESULT_YES ) {
        panel->data()->close();
        d_ptr.get()->onDocumentSave(panel->cef()->GetId());
    }

    return _reply;
}

QWidget * CEditorWindow::createMainPanel(QWidget * parent, CTabPanel * const panel)
{
    return createMainPanel(parent, panel->data()->title(), true, panel);
}

QWidget * CEditorWindow::createMainPanel(QWidget * parent, const QString& title, bool custom, QWidget * panel)
{
    // create min/max/close buttons
    CSingleWindowBase::createMainPanel(parent, title, custom, panel);

    QWidget * mainPanel = new QWidget(parent);
    mainPanel->setObjectName("mainPanel");

    QGridLayout * mainGridLayout = new QGridLayout();
    mainGridLayout->setSpacing(0);
#ifdef Q_OS_WIN
    mainGridLayout->setMargin(0);
#else
    int b = CX11Decoration::customWindowBorderWith() * m_dpiRatio;
    mainGridLayout->setContentsMargins(QMargins(b,b,b,b));
#endif
    mainPanel->setLayout(mainGridLayout);
//    mainPanel->setStyleSheet(AscAppManager::getWindowStylesheets(m_dpiRatio));
//    mainPanel->setStyleSheet("background-color:#446995;");

    // Central widget
//    QWidget * centralWidget = new QWidget(mainPanel);
//    centralWidget->setObjectName("centralWidget");
//    centralWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_boxTitleBtns = new QWidget(mainPanel);
    m_boxTitleBtns->setObjectName("box-title-tools");

    switch (qobject_cast<CTabPanel *>(panel)->data()->contentType()) {
    case etDocument: m_boxTitleBtns->setProperty("editor","word"); break;
    case etPresentation: m_boxTitleBtns->setProperty("editor","slide"); break;
    case etSpreadsheet: m_boxTitleBtns->setProperty("editor","cell"); break;
    }

    mainPanel->setStyleSheet(m_dpiRatio > 1 ? g_css + g_css2x : g_css);

    QHBoxLayout * layoutBtns = new QHBoxLayout(m_boxTitleBtns);
    layoutBtns->setContentsMargins(0,0,0,0);
    layoutBtns->setSpacing(1*m_dpiRatio);

    layoutBtns->addWidget(m_labelTitle);
    layoutBtns->addWidget(d_ptr.get()->iconUser());
    layoutBtns->addWidget(d_ptr.get()->buttonDock());

    if ( custom ) {
        layoutBtns->addWidget(m_buttonMinimize);
        layoutBtns->addWidget(m_buttonMaximize);
        layoutBtns->addWidget(m_buttonClose);

//        m_boxTitleBtns->setFixedSize(282*m_dpiRatio, TOOLBTN_HEIGHT*m_dpiRatio);

//        QWidget * _lb = new QWidget;
//        _lb->setFixedWidth( (small_btn_size.width() + layoutBtns->spacing()) * 3 );
//        layoutBtns->insertWidget(0, _lb);
    } else {
//        QLinearGradient gradient(centralWidget->rect().topLeft(), QPoint(centralWidget->rect().left(), 29));
//        gradient.setColorAt(0, QColor("#eee"));
//        gradient.setColorAt(1, QColor("#e4e4e4"));

//        label->setFixedHeight(0);
//        m_boxTitleBtns->setFixedSize(342*m_dpiRatio, 16*m_dpiRatio);
    }

    if ( !panel ) {
//        QCefView * pMainWidget = AscAppManager::createViewer(centralWidget);
//        pMainWidget->Create(&AscAppManager::getInstance(), cvwtSimple);
//        pMainWidget->setObjectName( "mainPanel" );
//        pMainWidget->setHidden(false);

//        m_pMainView = (QWidget *)pMainWidget;
    } else {
        m_pMainView = panel;
        m_pMainView->setParent(mainPanel);

        m_pMainView->setGeometry(mainPanel->geometry());
//        m_pMainView->show();
    }

//    m_pMainWidget->setVisible(false);

//    mainGridLayout->addWidget(centralWidget);
    d_ptr.get()->onScreenScalingFactor(m_dpiRatio);
    mainGridLayout->addWidget(m_pMainView);
    return mainPanel;
}

void CEditorWindow::onCloseEvent()
{
    if ( m_pMainView ) {
        if ( closeWindow() == MODAL_RESULT_YES ) {
            hide();
        }
    }
}

void CEditorWindow::onMinimizeEvent()
{
    CSingleWindowPlatform::onMinimizeEvent();
}

void CEditorWindow::onMaximizeEvent()
{
    CSingleWindowPlatform::onMaximizeEvent();
}

void CEditorWindow::onSizeEvent(int type)
{
    CSingleWindowPlatform::onSizeEvent(type);
    recalculatePlaces();
}

void CEditorWindow::onScreenScalingFactor(uint newfactor)
{
    CSingleWindowPlatform::onScreenScalingFactor(newfactor);

    d_ptr.get()->onScreenScalingFactor(newfactor);

    QString css(AscAppManager::getWindowStylesheets(newfactor));
    css += newfactor > 1 ? g_css + g_css2x : g_css;
    m_pMainPanel->setStyleSheet(css);

    m_boxTitleBtns->layout()->setSpacing(1*newfactor);

    m_dpiRatio = newfactor;
    adjustGeometry();
    recalculatePlaces();
}

void CEditorWindow::recalculatePlaces()
{
    if ( !m_pMainView ) return;

    int windowW = m_pMainPanel->width(),
        windowH = m_pMainPanel->height(),
        captionH = TITLE_HEIGHT * m_dpiRatio;

//    int contentH = windowH - captionH;
//    if ( contentH < 1 ) contentH = 1;

//    int nCaptionR = 200;
    int nCaptionL = d_ptr.get()->titleLeftOffset * m_dpiRatio;

    QSize _s{TOOLBTN_WIDTH * 3, TOOLBTN_HEIGHT};
    _s *= m_dpiRatio;
//    m_boxTitleBtns->setFixedSize(_s.width(), _s.height());
#ifdef Q_OS_WIN
    m_boxTitleBtns->setGeometry(nCaptionL, 0, windowW - nCaptionL, _s.height());
#else
    int cbw = CX11Decoration::customWindowBorderWith()*m_dpiRatio;
    m_boxTitleBtns->setGeometry(nCaptionL, cbw, windowW - nCaptionL - cbw, _s.height());
#endif
//    m_boxTitleBtns->move(windowW - m_boxTitleBtns->width() + cbw, cbw);
//    m_pMainView->setGeometry(cbw, captionH + cbw, windowW, contentH);

    QRegion reg(0, captionH, windowW, windowH - captionH);
    reg = reg.united(QRect(0, 0, nCaptionL, captionH));
//    reg = reg.united(QRect(windowW - nCaptionR, 0, nCaptionR, captionH));
    m_pMainView->clearMask();
    m_pMainView->setMask(reg);

//    m_pMainView->lower();
}

void CEditorWindow::setReporterMode(bool apply)
{
    if ( apply ) {
        int windowW = m_pMainPanel->width(),
            windowH = m_pMainPanel->height(),
            captionH = TITLE_HEIGHT * m_dpiRatio;

        QRegion reg(0, captionH, windowW, windowH - captionH);
        m_pMainView->clearMask();
        m_pMainView->setMask(reg);
    }
}

CTabPanel * CEditorWindow::mainView() const
{
    return qobject_cast<CTabPanel *>(m_pMainView);
}

CTabPanel * CEditorWindow::releaseEditorView() const
{
    m_pMainView->clearMask();
    return qobject_cast<CTabPanel *>(m_pMainView);
}

const QObject * CEditorWindow::receiver()
{
    return d_ptr.get();
}

void CEditorWindow::onLocalFileSaveAs(void * d)
{
    CAscLocalSaveFileDialog * pData = static_cast<CAscLocalSaveFileDialog *>(d);

    QFileInfo info(QString::fromStdWString(pData->get_Path()));
    if ( !info.fileName().isEmpty() ) {
        bool _keep_path = false;
        QString _full_path;

        if ( info.exists() ) _full_path = info.absoluteFilePath();
        else _full_path = Utils::lastPath(LOCAL_PATH_SAVE) + "/" + info.fileName(), _keep_path = true;

        CFileDialogWrapper dlg(handle());
        dlg.setFormats(pData->get_SupportFormats());

        CAscLocalSaveFileDialog * pSaveData = new CAscLocalSaveFileDialog();
        pSaveData->put_Id(pData->get_Id());
        pSaveData->put_Path(L"");

        if ( dlg.modalSaveAs(_full_path) ) {
            if ( _keep_path )
                Utils::keepLastPath(LOCAL_PATH_SAVE, QFileInfo(_full_path).absoluteDir().absolutePath());

            bool _allowed = true;
            if ( dlg.getFormat() == AVS_OFFICESTUDIO_FILE_SPREADSHEET_CSV ) {
                CMessage mess(handle(), CMessageOpts::moButtons::mbOkDefCancel);
                _allowed =  MODAL_RESULT_CUSTOM == mess.warning(tr("Some data will lost.<br>Continue?"));
            }

            if ( _allowed ) {
                pSaveData->put_Path(_full_path.toStdWString());
                int format = dlg.getFormat() > 0 ? dlg.getFormat() :
                        AscAppManager::GetFileFormatByExtentionForSave(pSaveData->get_Path());

                pSaveData->put_FileType(format > -1 ? format : 0);
            }
        }

        CAscMenuEvent* pEvent = new CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_LOCALFILE_SAVE_PATH);
        pEvent->m_pData = pSaveData;

        AscAppManager::getInstance().Apply(pEvent);

//        RELEASEINTERFACE(pData)
//        RELEASEINTERFACE(pEvent)
    }

    RELEASEINTERFACE(pData);
}

QString CEditorWindow::documentName() const
{
    return d_ptr.get()->panel()->data()->title(true);
}

bool CEditorWindow::closed() const
{
    return d_ptr.get()->panel()->data()->closed();
}
