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

#ifndef CEDITORWINDOW_P_H
#define CEDITORWINDOW_P_H

#include "ccefeventsgate.h"
#include "ceditorwindow.h"
#include "cascapplicationmanagerwrapper.h"
#include "cascapplicationmanagerwrapper_private.h"
#include "applicationmanager_events.h"
#include "utils.h"
#include "common/Types.h"
#include "components/cmessage.h"
#include "qascprinter.h"
#include "ceditortools.h"
#include "components/csvgpushbutton.h"
#include "defines.h"
#include "components/cfullscrwidget.h"
#include "components/cprintdialog.h"

#include <QPrinterInfo>
#include <QDesktopWidget>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QGridLayout>
#include <QStandardPaths>
#include <QPrintEngine>

#ifdef __linux__
# include "platform_linux/gtkprintdialog.h"
#endif

#define TOP_PANEL_OFFSET 6*TOOLBTN_WIDTH

using namespace NSEditorApi;

const QString g_css =
        "#mainPanel{background-color:%1;}"
        "#box-title-tools{background-color:%1;}"
        "QPushButton[act=tool]:hover{background-color:rgba(0,0,0,20%);}"
        "QPushButton#toolButtonClose:hover{background-color:#d42b2b;}"
        "QPushButton#toolButtonClose:pressed{background-color:#d75050;}"
#ifdef Q_OS_LINUX
        "#box-title-tools QLabel{font-size:11px;font-family:\"Helvetica Neue\",Helvetica,Arial,sans-serif;}"
        "#labelTitle{color:#444;}"
#else
        "#box-title-tools QLabel{font-family:\"Helvetica Neue\",Helvetica,Arial,sans-serif;font-weight:bold;}"
        "#labelTitle{color:#444;}"
        "#mainPanel[window=pretty] #labelTitle{font-size:12px;}"
#endif
        "#iconuser{color:#fff;font-size:11px;}"
        "#mainPanel[window=pretty] QPushButton[act=tool]:hover{background-color:rgba(255,255,255,20%);}"
        "#mainPanel[window=pretty] QPushButton#toolButtonMinimize,"
        "#mainPanel[window=pretty] QPushButton#toolButtonClose {background-image:url(:/minclose_light.png);}"
        "#mainPanel[window=pretty] QPushButton#toolButtonClose:hover{background-color:#d42b2b;}"
        "#mainPanel[window=pretty] QPushButton#toolButtonMaximize{background-image:url(:/max_light.png);}"
        "#mainPanel[window=pretty] #labelTitle{color:#fff;}"
        "#mainPanel[zoom=\"1.25x\"] #toolButtonMinimize,#mainPanel[zoom=\"1.25x\"] #toolButtonClose,"
        "#mainPanel[zoom=\"1.25x\"] #toolButtonMaximize{padding: 6px 15px 9px;}"
        "#mainPanel[zoom=\"1.25x\"] #iconuser,"
        "#mainPanel[zoom=\"1.25x\"] #labelTitle{font-size:15px;}"
        "#mainPanel[zoom=\"1.25x\"][window=pretty] QPushButton#toolButtonMinimize,"
        "#mainPanel[zoom=\"1.25x\"][window=pretty] QPushButton#toolButtonClose {background-image:url(:/minclose_light_1.25x.png);}"
        "#mainPanel[zoom=\"1.25x\"][window=pretty] QPushButton#toolButtonMaximize{background-image:url(:/max_light_1.25x.png);}"
        "#mainPanel[zoom=\"1.5x\"] #toolButtonMinimize,#mainPanel[zoom=\"1.5x\"] #toolButtonClose,"
        "#mainPanel[zoom=\"1.5x\"] #toolButtonMaximize{padding: 8px 18px 11px;}"
        "#mainPanel[zoom=\"1.5x\"] #iconuser,"
        "#mainPanel[zoom=\"1.5x\"] #labelTitle{font-size:18px;}"
        "#mainPanel[zoom=\"1.5x\"][window=pretty] QPushButton#toolButtonMinimize,"
        "#mainPanel[zoom=\"1.5x\"][window=pretty] QPushButton#toolButtonClose {background-image:url(:/minclose_light_1.5x.png);}"
        "#mainPanel[zoom=\"1.5x\"][window=pretty] QPushButton#toolButtonMaximize{background-image:url(:/max_light_1.5x.png);}"
        "#mainPanel[zoom=\"1.75x\"] #toolButtonMinimize,#mainPanel[zoom=\"1.75x\"] #toolButtonClose,"
        "#mainPanel[zoom=\"1.75x\"] #toolButtonMaximize{padding: 9px 21px 12px;}"
        "#mainPanel[zoom=\"1.75x\"] #iconuser,"
        "#mainPanel[zoom=\"1.75x\"] #labelTitle{font-size:21px;}"
        "#mainPanel[zoom=\"1.75x\"][window=pretty] QPushButton#toolButtonMinimize,"
        "#mainPanel[zoom=\"1.75x\"][window=pretty] QPushButton#toolButtonClose {background-image:url(:/minclose_light_1.75x.png);}"
        "#mainPanel[zoom=\"1.75x\"][window=pretty] QPushButton#toolButtonMaximize{background-image:url(:/max_light_1.75x.png);}"
        "#mainPanel[zoom=\"2x\"] #toolButtonMinimize,#mainPanel[zoom=\"2x\"] #toolButtonClose,"
        "#mainPanel[zoom=\"2x\"] #toolButtonMaximize{padding: 10px 24px 14px;}"
        "#mainPanel[zoom=\"2x\"] #iconuser,"
        "#mainPanel[zoom=\"2x\"] #labelTitle{font-size:24px;}"
        "#mainPanel[zoom=\"2x\"][window=pretty] QPushButton#toolButtonMinimize,"
        "#mainPanel[zoom=\"2x\"][window=pretty] QPushButton#toolButtonClose {background-image:url(:/minclose_light_2x.png);}"
        "#mainPanel[zoom=\"2x\"][window=pretty] QPushButton#toolButtonMaximize{background-image:url(:/max_light_2x.png);}"
        "#mainPanel[uitheme=theme-dark] #iconuser,"
        "#mainPanel[uitheme=theme-dark] #labelTitle{color:rgba(255,255,255,80%);}"
        "#mainPanel[uitheme=theme-contrast-dark] #iconuser,"
        "#mainPanel[uitheme=theme-contrast-dark] #labelTitle{color:#e8e8e8;}";

auto prepare_editor_css(int type, const CTheme& theme) -> QString {
    std::wstring c;
    switch (type) {
    default: c = theme.value(CTheme::ColorRole::ecrTabWordActive); break;
    case etDocument: c = theme.value(CTheme::ColorRole::ecrTabWordActive); break;
    case etPresentation: c = theme.value(CTheme::ColorRole::ecrTabSlideActive); break;
    case etSpreadsheet: c = theme.value(CTheme::ColorRole::ecrTabCellActive); break;
    }

    return g_css.arg(QString::fromStdWString(c));
}

auto editor_color(int type) -> QColor {
    switch (type) {
    case etDocument: return GetColorByRole(ecrTabWordActive);
    case etPresentation: return GetColorByRole(ecrTabSlideActive);
    case etSpreadsheet: return GetColorByRole(ecrTabCellActive);
    default: return GetColorByRole(ecrTabWordActive);
    }
}

class CEditorWindowPrivate : public CCefEventsGate
{
    CEditorWindow * window = nullptr;
    CElipsisLabel * iconuser = nullptr;
    QPushButton * btndock = nullptr;
    bool isPrinting = false,
        isFullScreen = false;
    CFullScrWidget * fs_parent = nullptr;
    QLabel * iconcrypted = nullptr;
    QWidget * boxtitlelabel = nullptr,
            * leftboxbuttons = nullptr;

    QMap<QString, CSVGPushButton*> m_mapTitleButtons;

public:
    int titleLeftOffset = 0;
    bool isReporterMode = false,
         usedOldEditorVersion = false;

public:
    CEditorWindowPrivate(CEditorWindow * w) : window(w) {}
    ~CEditorWindowPrivate() override {
        if ( leftboxbuttons )
            leftboxbuttons->deleteLater();
    }

    void init(CTabPanel * const p) override {
        CCefEventsGate::init(p);
        if (!m_panel->data()->hasFeature(L"btnhome") || viewerMode()) {  // For old editors only
            usedOldEditorVersion = true;
            leftboxbuttons = new QWidget;
            leftboxbuttons->setLayout(new QHBoxLayout);
            leftboxbuttons->layout()->setSpacing(0);
            leftboxbuttons->layout()->setMargin(0);

            CSVGPushButton * btnHome = new CSVGPushButton;
            btnHome->setProperty("class", "normal");
            btnHome->setProperty("act", "tool");
            btnHome->setFixedSize(QSize(TOOLBTN_WIDTH,TOOLBTN_HEIGHT) * window->m_dpiRatio);
            btnHome->setIconSize(QSize(20,20) * window->m_dpiRatio);
            btnHome->setMouseTracking(true);
            btnHome->setIcon(":/title/icons/buttons.svg", "svg-btn-home");
            btnHome->setToolTip(CEditorWindow::tr("Open main window"));
            btnHome->setIconOpacity(AscAppManager::themes().current().color(CTheme::ColorRole::ecrButtonNormalOpacity));
            m_mapTitleButtons["home"] = btnHome;
            connect(btnHome, &QPushButton::clicked, std::bind(&CEditorWindow::onClickButtonHome, window));
            leftboxbuttons->layout()->addWidget(btnHome);
        }
    }

    QPushButton * cloneEditorHeaderButton(const QJsonObject& jsonobj)  // For old editors only
    {
        QString action = jsonobj["action"].toString();
        CSVGPushButton * btn = new CSVGPushButton;
        btn->setProperty("class", "normal");
        btn->setProperty("act", "tool");
        btn->setFixedSize(QSize(TOOLBTN_WIDTH,TOOLBTN_HEIGHT) * window->m_dpiRatio);
        btn->setDisabled(jsonobj["disabled"].toBool());
        btn->setIconSize(QSize(20,20) * window->m_dpiRatio);
        btn->setMouseTracking(true);
        btn->setIconOpacity(AscAppManager::themes().current().color(CTheme::ColorRole::ecrButtonNormalOpacity));
        if ( jsonobj.contains("visible") && !jsonobj["visible"].toBool() ) {
            btn->hide();
        }

        m_mapTitleButtons[action] = btn;
        connect(btn, &QPushButton::clicked, [=]{
            if ( action == "home" ) {
            } else {
                QJsonObject _json_obj{{"action", action}};
                AscAppManager::sendCommandTo(panel()->cef(), L"button:click", Utils::stringifyJson(_json_obj).toStdWString());
                window->focus();
            }
        });

        if ( jsonobj.contains("icon") ) {
            btn->setIcon(":/title/icons/buttons.svg", "svg-btn-" + jsonobj["icon"].toString());
        } else btn->setIcon(":/title/icons/buttons.svg", "svg-btn-" + action);

        btn->setToolTip(jsonobj["hint"].toString());
        return btn;
    }

    auto extendableTitleToSimple() -> void {
        Q_ASSERT(window->m_boxTitleBtns != nullptr);
        QGridLayout * const _layout = static_cast<QGridLayout*>(window->m_pMainPanel->layout());
        if ( !_layout->findChild<QWidget*>(window->m_boxTitleBtns->objectName()) ) {
            _layout->addWidget(window->m_boxTitleBtns,0,0,Qt::AlignTop);
            iconUser()->hide();
            window->m_labelTitle->setText(APP_TITLE);
            changeTheme(GetCurrentTheme().id());
        }
    }

    void onEditorConfig(int, std::wstring cfg) override
    {
//        if ( id == window->holdView(id) )
        if ( !window->isCustomWindowStyle() ) return;

        QJsonParseError jerror;
        QJsonDocument jdoc = QJsonDocument::fromJson(QString::fromStdWString(cfg).toUtf8(), &jerror);
        if( jerror.error == QJsonParseError::NoError ) {
            QJsonObject objRoot = jdoc.object();

            if ( viewerMode() )
                extendableTitleToSimple();

            int _user_width = 0;
            if ( canExtendTitle() ) {
                if ( objRoot.contains("user") ) {
                    QString _user_name = objRoot["user"].toObject().value("name").toString();
                    iconUser()->setToolTip(_user_name);
                    iconuser->setText(_user_name);

                    iconuser->adjustSize();
                    _user_width = iconuser->width();
                }

                if ( objRoot.contains("title") /*&& m_mapTitleButtons.empty()*/ ) {
                    QJsonArray _btns = objRoot["title"].toObject().value("buttons").toArray();
                    QHBoxLayout * _layout = qobject_cast<QHBoxLayout *>(window->m_boxTitleBtns->layout());

                    if (usedOldEditorVersion) {  // For old editors only
                        for (const auto jv: _btns) {
                            const QJsonObject obj = jv.toObject();
                            if ( !m_mapTitleButtons.contains(obj["action"].toString()) )
                                leftboxbuttons->layout()->addWidget(cloneEditorHeaderButton(jv.toObject()));

                            titleLeftOffset += 40;
                        }

                        if ( _layout->itemAt(0)->widget() != leftboxbuttons )
                            _layout->insertWidget(0, leftboxbuttons);
                    }
                }

                // update title caption for elipsis
                window->updateTitleCaption();
            }

            int _btncount = /*iconuser ? 4 :*/ 3;
            int diffW = (titleLeftOffset - TOOLBTN_WIDTH * _btncount) * window->m_dpiRatio; // 4 right tool buttons: close, min, max, user icon
            diffW -= _user_width;

            int left = usedOldEditorVersion ? -diffW : 0;   // For old editors only
            int right = viewerMode() ? 40 * window->m_dpiRatio : 0;
            diffW > 0 ? boxtitlelabel->setContentsMargins(0, 0, diffW, 2*window->m_dpiRatio) :
                            boxtitlelabel->setContentsMargins(left, 0, right, 2*window->m_dpiRatio);
        }
    }

    void onEditorActionRequest(int, const QString& json) override
    {
        if ( json.contains(QRegExp("action\\\":\\\"file:close")) ) {
            window->closeWindow();
        }
    }

    void adjustToNewEditorVersion()  // For old editors only
    {
        if (window->isCustomWindowStyle()) {
            leftboxbuttons->hide();
            auto layout = qobject_cast<QHBoxLayout*>(window->m_boxTitleBtns->layout());
            if (canExtendTitle()) {
                auto icon = layout->takeAt(3);
                if (icon && icon->widget())
                    layout->insertWidget(2, icon->widget());
            }
            auto stretch = layout->takeAt(1);
            if (stretch)
                delete stretch;
            auto mainGridLayout = dynamic_cast<QGridLayout*>(window->m_pMainPanel->layout());
            if (mainGridLayout) {
                auto mainView = mainGridLayout->itemAtPosition(1, 0);
                if (mainView)
                    mainGridLayout->removeItem(mainView);
                QLayoutItem *boxTitleBtns = canExtendTitle() ? mainGridLayout->itemAtPosition(1, 0) : nullptr;
                if (boxTitleBtns)
                    mainGridLayout->removeItem(boxTitleBtns);

                if (mainView && mainView->widget())
                    mainGridLayout->addWidget(mainView->widget(), 1, 0, 1, 2);
                if (boxTitleBtns && boxTitleBtns->widget()) {
                    mainGridLayout->addWidget(boxTitleBtns->widget(), 1, 1, Qt::AlignTop);
                    window->m_pSpacer = new QSpacerItem(int(TOP_PANEL_OFFSET*window->m_dpiRatio), 5,
                                                QSizePolicy::Fixed, QSizePolicy::Fixed);
                    mainGridLayout->addItem(window->m_pSpacer, 1, 0, Qt::AlignTop);
                }
            }
        }
    }

    void onDocumentReady(int uid) override
    {
//        if (window->holdView(uid))
            if ( panel()->data()->features().empty() ) {
                panel()->data()->setFeatures(L"old version of editor");
                extendableTitleToSimple();
            }
            if (m_panel->data()->hasFeature(L"btnhome") && usedOldEditorVersion && !viewerMode()) {  // For old editors only
                usedOldEditorVersion = false;
                adjustToNewEditorVersion();
            }
            panel()->setReady();
            if (window->isActiveWindow())
                window->focus();
    }

    void onDocumentName(void * data) override
    {
        CCefEventsGate::onDocumentName(data);

        if ( window->isCustomWindowStyle() ) {
            if ( !canExtendTitle() /*|| !window->isCustomWindowStyle()*/ ) {
                window->m_labelTitle->setText(APP_TITLE);
            } else {
                window->setWindowTitle(m_panel->data()->title());
                window->m_boxTitleBtns->repaint();
            }
        }
    }

    void onDocumentType(int id, int type) override
    {
        CCefEventsGate::onDocumentType(id, type);

        if ( canExtendTitle() && window->isCustomWindowStyle() ) {
            window->m_pMainPanel->setProperty("window", "pretty");
            changeTheme(AscAppManager::themes().current().id());
        }
    }

    void setWindowColors()
    {
        std::wstring background, border;
        switch (panel()->data()->contentType()) {
        case etDocument:
            background = AscAppManager::themes().current().value(CTheme::ColorRole::ecrTabWordActive);
            border = background;
            break;
        case etPresentation:
            background = AscAppManager::themes().current().value(CTheme::ColorRole::ecrTabSlideActive);
            border = background;
            break;
        case etSpreadsheet:
            background = AscAppManager::themes().current().value(CTheme::ColorRole::ecrTabCellActive);
            border = background;
            break;
        default:
            background = AscAppManager::themes().current().value(CTheme::ColorRole::ecrWindowBackground);
            border = AscAppManager::themes().current().value(CTheme::ColorRole::ecrWindowBorder);
        }

        window->setWindowColors(QColor(QString::fromStdWString(background)), QColor(QString::fromStdWString(border)));
    }

    void changeTheme(const std::wstring& theme)
    {
        if (window->isCustomWindowStyle()) {
            Q_ASSERT(window->m_pMainPanel);
            window->m_pMainPanel->setProperty("uitheme", QString::fromStdWString(theme));
            window->m_pMainPanel->setProperty("uithemetype", GetCurrentTheme().stype());
            if (!viewerMode()) {
                if (usedOldEditorVersion) {   // For old editors only
                    foreach (auto btn, m_mapTitleButtons)
                        btn->setIconOpacity(GetColorByRole(ecrButtonNormalOpacity));
                }
            } else {
                window->m_pMainPanel->setProperty("window", "pretty");
                m_mapTitleButtons["home"]->setIconOpacity(GetColorByRole(ecrButtonNormalOpacity));
            }
            AscEditorType editor_type = panel()->data()->contentType();
            window->m_css = prepare_editor_css(editor_type, GetCurrentTheme());
            QString css(AscAppManager::getWindowStylesheets(window->m_dpiRatio));
            css.append(window->m_css);
            window->m_pMainPanel->setStyleSheet(css);
        }
        setWindowColors();
        AscAppManager::sendCommandTo(panel()->cef(), L"uitheme:changed", theme);
    }

    void onDocumentChanged(int id, bool state) override
    {
        if ( panel()->data()->hasChanges() != state ) {
            CCefEventsGate::onDocumentChanged(id, state);

            if ( canExtendTitle() && window->isCustomWindowStyle() ) {
                window->setWindowTitle(m_panel->data()->title());
                window->m_boxTitleBtns->repaint();
            }
        }
    }

    void onDocumentSave(int id, bool cancel = false) override
    {
        CCefEventsGate::onDocumentSave(id, cancel);

        if ( m_panel->data()->closed() ) {
            if ( !((CCefViewEditor *)m_panel->cef())->CheckCloudCryptoNeedBuild() ) {
                AscAppManager::getInstance().DestroyCefView(m_panel->cef()->GetId());
                window->hide();
            }
        } else {
            AscAppManager::cancelClose();
        }
    }

    void onDocumentSaveInnerRequest(int) override
    {
        CMessage mess(window->handle(), CMessageOpts::moButtons::mbYesDefNo);
        int reply = mess.confirm(CEditorWindow::tr("Document must be saved to continue.<br>Save the document?"));

        CAscEditorSaveQuestion * pData = new CAscEditorSaveQuestion;
        pData->put_Value((reply == MODAL_RESULT_CUSTOM + 0) ? true : false);

        CAscMenuEvent * pEvent = new CAscMenuEvent(ASC_MENU_EVENT_TYPE_DOCUMENTEDITORS_SAVE_YES_NO);
        pEvent->m_pData = pData;

        m_panel->cef()->Apply(pEvent);
    }

    void onDocumentFragmented(int id, bool needbuild) override
    {
        if ( needbuild ) {
//            static const bool _skip_user_warning = !Utils::appArgsContains("--warning-doc-fragmented");
//            if ( _skip_user_warning )
                m_panel->cef()->Apply(new CAscMenuEvent(ASC_MENU_EVENT_TYPE_ENCRYPTED_CLOUD_BUILD));
        } else
            onDocumentFragmentedBuild(id, 0);

    }

    void onDocumentFragmentedBuild(int id, int error) override
    {
        CCefEventsGate::onDocumentFragmentedBuild(id, error);

        if ( m_panel->data()->closed() ) {
            AscAppManager::getInstance().DestroyCefView(m_panel->cef()->GetId());
            window->hide();
        } else AscAppManager::cancelClose();
    }

    void onDocumentPrint(void * data)  override
    {
        onDocumentPrint(AscAppManager::printData().pageCurent(), AscAppManager::printData().pagesCount());
    }

    void onDocumentPrint(int currentpage, uint pagescount) override
    {
#ifdef __OS_WIN_XP
        if (QPrinterInfo::availablePrinterNames().size() == 0) {
            CMessage::info(window->handle(), tr("There are no printers available"));
            return;
        }
#endif
        if ( isPrinting ) return;
        isPrinting = true;

#ifdef Q_OS_LINUX
        WindowHelper::CParentDisable oDisabler(window->handle());
#endif
        if ( !(pagescount < 1) ) {
            CAscMenuEvent * pEvent;
            QAscPrinterContext * pContext = new QAscPrinterContext(AscAppManager::printData().printerInfo());

            QPrinter * printer = pContext->getPrinter();
            printer->setOutputFileName("");
            printer->setFromTo(1, pagescount);
            printer->printEngine()->setProperty(QPrintEngine::PPK_DocumentName, m_panel->data()->title(true));

            if ( !AscAppManager::printData().isQuickPrint() ) {
                printer->setPageOrientation(AscAppManager::printData().pageOrientation());
                printer->setPageSize(AscAppManager::printData().pageSize());
            }

#ifdef _WIN32
            CPrintDialog * dialog =  new CPrintDialog(printer, window->handle());
#else
# ifdef FILEDIALOG_DONT_USE_NATIVEDIALOGS
            CPrintDialog * dialog =  new CPrintDialog(printer, window->handle());
# else
            GtkPrintDialog * dialog = new GtkPrintDialog(printer, window->handle());
# endif
#endif // _WIN32

            dialog->setWindowTitle(CEditorWindow::tr("Print Document"));
            dialog->setEnabledOptions(QPrintDialog::PrintPageRange | QPrintDialog::PrintToFile);
            if (!(currentpage < 0)) {
                currentpage++;
                dialog->setEnabledOptions(dialog->enabledOptions() | QPrintDialog::PrintCurrentPage);
                dialog->setOptions(dialog->options() | QPrintDialog::PrintCurrentPage);
            }

            dialog->setPrintRange(AscAppManager::printData().printRange());
            if ( dialog->printRange() == QPrintDialog::PageRange )
                dialog->setFromTo(AscAppManager::printData().pageFrom(), AscAppManager::printData().pageTo());

            int modal_res = QDialog::Accepted;
            if ( AscAppManager::printData().isQuickPrint() ) {
                dialog->accept();
            } else modal_res = dialog->exec();

            if ( modal_res == QDialog::Accepted ) {
                if ( !AscAppManager::printData().isQuickPrint() )
                    AscAppManager::printData().setPrinterInfo(QPrinterInfo::printerInfo(printer->printerName()));

                QVector<PageRanges> page_ranges;

#ifdef Q_OS_LINUX
                if ( AscAppManager::printData().isQuickPrint() && printer->outputFormat() == QPrinter::PdfFormat )
                    printer->setOutputFileName(Utils::uniqFileName(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/print.pdf"));
#endif

                switch(dialog->printRange()) {
                case QPrintDialog::AllPages:
                case QPrintDialog::PageRange:
                    page_ranges = dialog->getPageRanges();
                    break;
                case QPrintDialog::Selection:
                    page_ranges.append(PageRanges(-1, -1));
                    break;
                case QPrintDialog::CurrentPage:
                    page_ranges.append(PageRanges(currentpage, currentpage));
                    break;
                }

                CEditorTools::print({m_panel->cef(), pContext, &page_ranges, window->handle()});
            }

            pContext->Release();

            pEvent = new CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_PRINT_END);
            m_panel->cef()->Apply(pEvent);
    //        RELEASEOBJECT(pEvent)

#ifndef _WIN32
            RELEASEOBJECT(dialog)
#endif
        }

        isPrinting = false;
    }


    void onEditorAllowedClose(int) override
    {
        AscAppManager::closeEditorWindow(size_t(window));

        CInAppEventBase _event{CInAppEventBase::CEventType::etEditorClosed};
        AscAppManager::getInstance().commonEvents().signal(&_event);
    }

    void onScreenScalingFactor(double f)
    {
        if ( window->isCustomWindowStyle() ) {
            int _btncount = /*iconuser ? 4 :*/ 3;
            int diffW = int((titleLeftOffset - (TOOLBTN_WIDTH * _btncount)) * f); // 4 tool buttons: min+max+close+usericon

            if ( iconuser ) {
                iconuser->setMaximumWidth(int(200 * f));
                iconuser->setContentsMargins(int(12*f),0,int(12*f),int(2*f));
                iconuser->setMaximumWidth(int(200*f));
                iconuser->adjustSize();
                if (!viewerMode())
                    diffW -= iconuser->width();
            }

            if ( iconcrypted ) {
                iconcrypted->setPixmap(QIcon{":/title/icons/secure.svg"}.pixmap(QSize(20,20) * f));
            }

            int left = usedOldEditorVersion ? -diffW : 0;   // For old editors only
            int right = viewerMode() ? 40 * window->m_dpiRatio : 0;
            diffW > 0 ? boxtitlelabel->setContentsMargins(0, 0, diffW, int(2*f)) :
                            boxtitlelabel->setContentsMargins(left, 0, right, int(2*f));

            for (const auto& btn: m_mapTitleButtons) {
                btn->setFixedSize(QSize(int(TOOLBTN_WIDTH*f), int(TOOLBTN_HEIGHT*f)));
                btn->setIconSize(QSize(20,20) * f);
            }
        }
    }

    void onFullScreen(bool apply)
    {
        if ( !apply ) isReporterMode = false;
        if ( apply == isFullScreen ) return;
        isFullScreen = apply;

        auto _break_demonstration = [&] {
            CAscExecCommandJS * pCommand = new CAscExecCommandJS;
            pCommand->put_Command(L"editor:stopDemonstration");

            CAscMenuEvent * pEvent = new CAscMenuEvent(ASC_MENU_EVENT_TYPE_CEF_EDITOR_EXECUTE_COMMAND);
            pEvent->m_pData = pCommand;
            panel()->cef()->Apply(pEvent);
        };

        CTabPanel * _fs_widget = panel();
        static QMetaObject::Connection cefConnection;
        if (!apply) {
            _break_demonstration();

            window->show(false);

//            _fs_widget->view()->resize(_fs_widget->size().width(), _fs_widget->size().height()-1);
            if (usedOldEditorVersion) {  // For old editors only
                qobject_cast<QGridLayout *>(window->m_pMainPanel->layout())->addWidget(_fs_widget, 1, 0);
            } else {
                qobject_cast<QGridLayout *>(window->m_pMainPanel->layout())->addWidget(_fs_widget, 1, 0, 1, 2);
            }
            window->recalculatePlaces();
            _fs_widget->showNormal();
            _fs_widget->cef()->focus();

            if ( fs_parent )
                delete fs_parent, fs_parent = nullptr;

            disconnect(cefConnection);
        } else {
            fs_parent = WindowHelper::constructFullscreenWidget(_fs_widget);

            _fs_widget->view()->setFocusToCef();
            window->hide();
            connect(fs_parent, &CFullScrWidget::closeRequest, this, [=]() {
                disconnect(fs_parent);
                onFullScreen(false);
            });

            cefConnection = connect(_fs_widget, &CTabPanel::closePanel, [=](QCloseEvent * e){
                _break_demonstration();

                e->ignore();
                window->closeWindow();
            });
        }
    }

    void onFullScreen(int, bool apply) override
    {
        onFullScreen(apply);
    }

    void onPortalLogout(std::wstring wjson) override
    {
        QJsonParseError jerror;
        QByteArray stringdata = QString::fromStdWString(wjson).toUtf8();
        QJsonDocument jdoc = QJsonDocument::fromJson(stringdata, &jerror);

        if( jerror.error == QJsonParseError::NoError ) {
            QJsonObject objRoot = jdoc.object();
            QString portal = objRoot["domain"].toString();

            if ( m_panel && !portal.isEmpty() ) {
                if ( !m_panel->data()->closed() && QString::fromStdWString(m_panel->data()->url()).startsWith(portal) )
                    window->closeWindow();
            }
        }
    }

    void onFileLocation(int, QString param) override
    {
        if ( param == "offline" ) {
            const std::wstring& path = m_panel->data()->url();
            if (!path.empty()) {
                Utils::openFileLocation(QString::fromStdWString(path));
            } else
                CMessage::info(window->handle(), CEditorWindow::tr("Document must be saved firstly."));
        } else {
            /**
            * portals open in tabbar in main window only
            * processing is in app cntroller
            */
        }
    }

    QLabel * iconUser()
    {
        Q_ASSERT(window->m_boxTitleBtns != nullptr);
        if ( !iconuser ) {
            iconuser = new CElipsisLabel(window->m_boxTitleBtns);
            iconuser->setObjectName("iconuser");
            iconuser->setContentsMargins(0,0,0,int(2 * window->m_dpiRatio));
            iconuser->setMaximumWidth(int(200 * window->m_dpiRatio));
//            iconuser->setPixmap(window->m_dpiRatio > 1 ? QPixmap(":/user_2x.png") : QPixmap(":/user.png"));
//            iconuser->setFixedSize(QSize(TOOLBTN_WIDTH*window->m_dpiRatio,16*window->m_dpiRatio));
        }

        return iconuser;
    }

    QLabel * iconCrypted()
    {
        Q_ASSERT(window->m_boxTitleBtns != nullptr);
        if ( !iconcrypted ) {
            iconcrypted = new QLabel(window->m_boxTitleBtns);
            iconcrypted->setObjectName("iconcrypted");

            iconcrypted->setPixmap(QIcon{":/title/icons/secure.svg"}.pixmap(QSize(20,20) * window->m_dpiRatio));
        }

        return iconcrypted;
    }

    QPushButton * buttonDock()
    {
        Q_ASSERT(window->m_boxTitleBtns != nullptr);
        if ( !btndock ) {
            btndock = window->createToolButton(window->m_boxTitleBtns, "toolButtonDock");
        }

        return btndock;
    }

    void onWebAppsFeatures(int, std::wstring f) override
    {
        panel()->data()->setFeatures(f);

        if ( m_panel->data()->hasFeature(L"uitype\":\"fillform") ) {
             ffWindowCustomize();
        }

        if ( panel()->data()->hasFeature(L"crypted\":true") && boxtitlelabel && !iconcrypted ) {
            qobject_cast<QBoxLayout *>(boxtitlelabel->layout())->insertWidget(0, iconCrypted());
        }

        if ( panel()->data()->hasFeature(L"readonly\":") && boxtitlelabel ) {
            window->setWindowTitle(m_panel->data()->title());
            window->m_boxTitleBtns->repaint();
        }
    }

    void onWebTitleChanged(int, std::wstring json) override
    {
        QJsonParseError jerror;
        QJsonDocument jdoc = QJsonDocument::fromJson(QString::fromStdWString(json).toUtf8(), &jerror);

        if( jerror.error == QJsonParseError::NoError ) {
            QJsonObject objRoot = jdoc.object();
            if ( usedOldEditorVersion ) {  // For old editors only
                if (objRoot.contains("disabled")) {
                    QJsonObject _disabled = objRoot["disabled"].toObject();

                    if ( _disabled.contains("all") ) {
                        bool _is_disabled = _disabled["all"].toBool();

                        for (auto& btn: m_mapTitleButtons) {
                            if ( m_mapTitleButtons["home"] != btn )
                                btn->setDisabled(_is_disabled);
                        }
                    } else {
                        for (const auto& k: _disabled.keys()) {
                            if ( m_mapTitleButtons.contains(k) )
                                m_mapTitleButtons[k]->setDisabled(_disabled.value(k).toBool());
                        }
                    }
                } else
                if (objRoot.contains("icon:changed")) {
                    QJsonObject _btns_changed = objRoot["icon:changed"].toObject();

                    for (const auto& b: _btns_changed.keys()) {
                        if ( m_mapTitleButtons.contains(b) )
                            m_mapTitleButtons[b]->setIcon(":/title/icons/buttons.svg", "svg-btn-" + _btns_changed.value(b).toString());
                    }
                } else
                if ( objRoot.contains("visible") ) {
                    QJsonObject _btns_changed = objRoot["visible"].toObject();
                    if ( _btns_changed.contains("quickprint") ) {
                        m_mapTitleButtons["quickprint"]->setVisible(_btns_changed["quickprint"].toBool());
                    }
                }
            } else
            if ( objRoot.contains("click") ) {
                QString _btns_changed = objRoot["click"].toString();

                if ( _btns_changed == "home" ) {
                    window->onClickButtonHome();
                }
            }
        }
    }

    bool canExtendTitle() const
    {
        if ( m_panel->data()->features().empty() ) return true;
        else if ( m_panel->data()->hasFeature(L"uitype\":\"fillform") ) return true;
        else return !viewerMode() && (m_panel->data()->isLocal() || m_panel->data()->hasFeature(L"titlebuttons\":"));
    }

    auto viewerMode() const -> bool {
        return m_panel->data()->hasFeature(L"viewmode\":true");
    }

    auto calcTitleLabelWidth(int basewidth) const -> int {
        if ( iconuser )
            basewidth -= iconuser->width();

        basewidth -= boxtitlelabel->contentsMargins().left() + boxtitlelabel->contentsMargins().right();
        if ( iconcrypted )
            basewidth -= iconcrypted->width();

        basewidth -= m_mapTitleButtons.count() * (TOOLBTN_WIDTH + 1) * window->m_dpiRatio;

        return basewidth;
    }

    auto customizeTitleLabel() -> void {
        Q_ASSERT(window->m_boxTitleBtns != nullptr);
        QHBoxLayout * _layout = qobject_cast<QHBoxLayout *>(window->m_boxTitleBtns->layout());
        _layout->removeWidget(window->m_labelTitle);

        if (!usedOldEditorVersion) {
            QLayoutItem *stretch = _layout->takeAt(0);
            if (stretch)
                delete stretch;
        }
        boxtitlelabel = new QWidget(window->m_boxTitleBtns);
        boxtitlelabel->setLayout(new QHBoxLayout(boxtitlelabel));
        boxtitlelabel->layout()->setSpacing(0);
        boxtitlelabel->layout()->setMargin(0);
        boxtitlelabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

        if ( m_panel->data()->hasFeature(L"crypted\":true") && !iconcrypted ) {
            boxtitlelabel->layout()->addWidget(iconCrypted());
        }

        boxtitlelabel->layout()->addWidget(window->m_labelTitle);

        if (usedOldEditorVersion) {  // For old editors only
            _layout->insertWidget(1, boxtitlelabel);
             if ( _layout->itemAt(0)->widget() != leftboxbuttons )
                 _layout->insertWidget(0, leftboxbuttons);
        } else {
            _layout->insertWidget(0, boxtitlelabel);
        }
    }

    auto ffWindowCustomize() -> void {
        Q_ASSERT(window->m_boxTitleBtns != nullptr);
        QGridLayout * const _layout = static_cast<QGridLayout*>(window->m_pMainPanel->layout());
        if ( !_layout->findChild<QWidget*>(window->m_boxTitleBtns->objectName()) ) {
            _layout->addWidget(window->m_boxTitleBtns,0,0,Qt::AlignTop);
        }
    }
};

#endif // CEDITORWINDOW_P_H
