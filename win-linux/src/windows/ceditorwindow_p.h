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
#else
# include "platform_win/printdialog.h"
#endif

#define TOP_PANEL_OFFSET 6*TOOLBTN_WIDTH
#define ICON_SPACER_WIDTH 9
#define ICON_SIZE QSize(20,20)

using namespace NSEditorApi;


auto prepare_editor_css(AscEditorType type, const CTheme& theme) -> QString {
    std::wstring c;
    switch (type) {
    default: c = theme.value(CTheme::ColorRole::ecrTabWordActive); break;
    case AscEditorType::etDocument: c = theme.value(CTheme::ColorRole::ecrTabWordActive); break;
    case AscEditorType::etPresentation: c = theme.value(CTheme::ColorRole::ecrTabSlideActive); break;
    case AscEditorType::etSpreadsheet: c = theme.value(CTheme::ColorRole::ecrTabCellActive); break;
    case AscEditorType::etPdf: c = theme.value(CTheme::ColorRole::ecrTabViewerActive); break;
    }
    QString g_css(Utils::readStylesheets(":/styles/editor.qss"));
#ifdef __linux__
    g_css.append(Utils::readStylesheets(":styles/editor_unix.qss"));
#endif
    return g_css.arg(QString::fromStdWString(c));
}

auto editor_color(AscEditorType type) -> QColor {
    switch (type) {
    case AscEditorType::etDocument: return GetColorByRole(ecrTabWordActive);
    case AscEditorType::etPresentation: return GetColorByRole(ecrTabSlideActive);
    case AscEditorType::etSpreadsheet: return GetColorByRole(ecrTabCellActive);
    case AscEditorType::etPdf: return GetColorByRole(ecrTabViewerActive);
    default: return GetColorByRole(ecrTabWordActive);
    }
}

class CEditorWindowPrivate : public CCefEventsGate
{
    CEditorWindow * window = nullptr;
    QLabel * iconuser = nullptr;
    bool isPrinting = false,
        isFullScreen = false;
    CFullScrWidget * fs_parent = nullptr;
    QLabel * iconcrypted = nullptr;
    QWidget * boxtitlelabel = nullptr,
            * leftboxbuttons = nullptr;

    QMap<QString, CSVGPushButton*> m_mapTitleButtons;

public:
    int titleLeftOffset = 0;
    bool usedOldEditorVersion = false;

public:
    CEditorWindowPrivate(CEditorWindow * w) : window(w) {}
    ~CEditorWindowPrivate() override {
        if ( leftboxbuttons )
            leftboxbuttons->deleteLater();
    }

    void init(CTabPanel * const p) override {
        CCefEventsGate::init(p);
        if (!m_panel->data()->hasFeature(L"btnhome") || viewerMode() || fillformMode()) {  // For old editors only
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
            //btnHome->setToolTip(CEditorWindow::tr("Open main window"));
            btnHome->setProperty("ToolTip", CEditorWindow::tr("Open main window"));
            btnHome->setIconOpacity(GetColorByRole(ecrButtonNormalOpacity));
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
        btn->setIconOpacity(GetColorByRole(ecrButtonNormalOpacity));
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

        //btn->setToolTip(jsonobj["hint"].toString());
        btn->setProperty("ToolTip", jsonobj["hint"].toString());
        return btn;
    }

    auto extendableTitleToSimple() -> void {
        Q_ASSERT(window->m_boxTitleBtns != nullptr);
        QGridLayout * const _layout = static_cast<QGridLayout*>(window->m_pMainPanel->layout());
        if ( !_layout->itemAtPosition(0,0) && !_layout->findChild<QWidget*>(window->m_boxTitleBtns->objectName()) ) {
            _layout->addWidget(window->m_boxTitleBtns,0,0,Qt::AlignTop);
            if (iconuser)
                iconuser->hide();
            window->m_labelTitle->setText(APP_TITLE);
            changeTheme(GetCurrentTheme().id());
        }
    }

    auto getInitials(const QString &name) -> QString {
        auto fio = name.split(' ');
        QString initials = !fio[0].isEmpty() ? fio[0].mid(0, 1).toUpper() : "";
        for (int i = fio.size() - 1; i > 0; i--) {
            if (!fio[i].isEmpty() && fio[i].at(0) != '('
                    && fio[i].at(0) != ')') {
                initials += fio[i].mid(0, 1).toUpper();
                break;
            }
        }
        return initials;
    }

    auto centerTitle(double dpiRatio)->void
    {
        int left_btns = (viewerMode() || fillformMode()) ? 1 : 6;
        int right_btns = 3;
        int spacing = window->m_boxTitleBtns->layout()->spacing();
        int left_offset = left_btns*TOOLBTN_WIDTH + 3*spacing; // added extra spacing
        int right_offset = right_btns*(TITLEBTN_WIDTH + spacing);
        int diffW = (left_offset - right_offset)*dpiRatio;
        if (iconuser) {
            diffW -= ICON_SPACER_WIDTH + spacing*dpiRatio;
            if (!viewerMode() && !fillformMode()) {
                diffW -= iconuser->width() + spacing*dpiRatio;
            }
        }
        QMargins mrg(0, 0, 0, 2*dpiRatio);
        diffW > 0 ? mrg.setRight(diffW) : mrg.setLeft(-diffW);
        boxtitlelabel->setContentsMargins(mrg);
    }

    void onEditorConfig(int, std::wstring cfg) override
    {
//        if ( id == window->holdView(id) )
        if ( !window->isCustomWindowStyle() ) return;

        QJsonParseError jerror;
        QJsonDocument jdoc = QJsonDocument::fromJson(QString::fromStdWString(cfg).toUtf8(), &jerror);
        if( jerror.error == QJsonParseError::NoError ) {
            QJsonObject objRoot = jdoc.object();

            if ( viewerMode() && !fillformMode())
                extendableTitleToSimple();

            if ( canExtendTitle() ) {
                if ( objRoot.contains("user") ) {
                    QString _user_name = objRoot["user"].toObject().value("name").toString();
                    //iconUser()->setToolTip(_user_name);
                    iconUser()->setProperty("ToolTip", _user_name);
                    adjustIconUser();
                    iconuser->setText(getInitials(_user_name));
                }

                if ( objRoot.contains("title") /*&& m_mapTitleButtons.empty()*/ ) {
                    QJsonArray _btns = objRoot["title"].toObject().value("buttons").toArray();
                    QHBoxLayout * _layout = qobject_cast<QHBoxLayout *>(window->m_boxTitleBtns->layout());

                    if (usedOldEditorVersion) {  // For old editors only
                        for (const auto jv: _btns) {
                            const QJsonObject obj = jv.toObject();
                            if ( !m_mapTitleButtons.contains(obj["action"].toString()) )
                                leftboxbuttons->layout()->addWidget(cloneEditorHeaderButton(jv.toObject()));
                        }

                        if ( _layout->itemAt(0)->widget() != leftboxbuttons )
                            _layout->insertWidget(0, leftboxbuttons);
                    }
                }

                // update title caption for elipsis
                window->updateTitleCaption();
            }
            centerTitle(window->m_dpiRatio);
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
            if (m_panel->data()->hasFeature(L"btnhome") && usedOldEditorVersion && !viewerMode() && !fillformMode()) {  // For old editors only
                usedOldEditorVersion = false;
                adjustToNewEditorVersion();
            }
            panel()->setReady();
            if (window->isActiveWindow())
                window->focus();
            AscAppManager::getInstance().onDocumentReady(uid);
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
            changeTheme(GetCurrentTheme().id());
        }
    }

    void setWindowColors()
    {
        std::wstring background, border;
        switch (panel()->data()->contentType()) {
        case AscEditorType::etDocument:
            background = GetColorValueByRole(ecrTabWordActive);
            border = background;
            break;
        case AscEditorType::etPresentation:
            background = GetColorValueByRole(ecrTabSlideActive);
            border = background;
            break;
        case AscEditorType::etSpreadsheet:
            background = GetColorValueByRole(ecrTabCellActive);
            border = background;
            break;
        case AscEditorType::etPdf:
            background = GetColorValueByRole(ecrTabViewerActive);
            border = background;
            break;
        default:
            background = GetColorValueByRole(ecrWindowBackground);
            border = GetColorValueByRole(ecrWindowBorder);
        }

        window->setWindowColors(QColor(QString::fromStdWString(background)), QColor(QString::fromStdWString(border)));
    }

    void changeTheme(const std::wstring& theme)
    {
        if (window->isCustomWindowStyle()) {
            Q_ASSERT(window->m_pMainPanel);
            window->m_pMainPanel->setProperty("uitheme", QString::fromStdWString(GetActualTheme(theme)));
            window->m_pMainPanel->setProperty("uithemetype", GetCurrentTheme().stype());
            if (!viewerMode()) {
                if (usedOldEditorVersion) {   // For old editors only
                    foreach (auto btn, m_mapTitleButtons)
                        btn->setIconOpacity(GetColorByRole(ecrButtonNormalOpacity));
                }
            } else {
                window->m_pMainPanel->setProperty("window", "pretty");
                if ( m_mapTitleButtons.contains("home") )
                    m_mapTitleButtons["home"]->setIconOpacity(GetColorByRole(ecrButtonNormalOpacity));
            }
            AscEditorType editor_type = panel()->data()->contentType();
            window->m_css = prepare_editor_css(editor_type, GetCurrentTheme());
            QString css(AscAppManager::getWindowStylesheets(window->m_dpiRatio));
            css.append(window->m_css);
#ifdef __linux__
            css.append(Utils::readStylesheets(":styles/styles_unix.qss"));
#endif
            window->m_pMainPanel->setStyleSheet(css);
        }
        setWindowColors();
        AscAppManager::sendCommandTo(panel()->cef(), L"uitheme:changed", theme);
    }

    void onDocumentChanged(int id, bool state) override
    {
        CCefEventsGate::onDocumentChanged(id, state);
        if ( panel()->data()->hasChanges() != state ) {
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
        int reply = CMessage::showMessage(window->handle(),
                                          CEditorWindow::tr("Document must be saved to continue.<br>Save the document?"),
                                          MsgType::MSG_CONFIRM, MsgBtns::mbYesDefNo);
        CAscEditorSaveQuestion * pData = new CAscEditorSaveQuestion;
        pData->put_Value((reply == MODAL_RESULT_YES) ? true : false);

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
        onDocumentPrint(AscAppManager::printData().pageCurrent(), AscAppManager::printData().pagesCount());
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

        QWidget *parent = window->handle();
#ifdef Q_OS_LINUX
        WindowHelper::CParentDisable oDisabler(parent);
#endif
        if ( !(pagescount < 1) ) {
            CAscMenuEvent * pEvent;
            QAscPrinterContext * pContext = new QAscPrinterContext(AscAppManager::printData().printerInfo());
            QString documentName = m_panel->data()->title(true);

            QPrinter * printer = pContext->getPrinter();
            printer->setFromTo(1, pagescount);
            printer->printEngine()->setProperty(QPrintEngine::PPK_DocumentName, documentName);
            printer->setDuplex(AscAppManager::printData().duplexMode());
            if ( printer->supportsMultipleCopies() ) {
                printer->setCopyCount(AscAppManager::printData().copiesCount());
            }

            if ( !AscAppManager::printData().isQuickPrint() ) {
                printer->setPageOrientation(AscAppManager::printData().pageOrientation());
                printer->setPageSize(AscAppManager::printData().pageSize());
            }

#ifdef _WIN32
            printer->setOutputFileName("");
            PrintDialog * dialog =  new PrintDialog(printer, window->handle());
#else
            QFileInfo info(documentName);
            QString pdfName = Utils::lastPath(LOCAL_PATH_SAVE) + "/" + info.baseName() + ".pdf";
            QString outputName = AscAppManager::printData().isQuickPrint() ? Utils::uniqFileName(pdfName) : pdfName;
            if ( AscAppManager::printData().printerInfo().printerName().isEmpty() ) {
                printer->setOutputFileName(outputName);
            } else {
                printer->printEngine()->setProperty(QPrintEngine::PPK_OutputFileName, outputName);
            }

# ifdef FILEDIALOG_DONT_USE_NATIVEDIALOGS
            CPrintDialog * dialog =  new CPrintDialog(printer, parent);
# else
            GtkPrintDialog * dialog = new GtkPrintDialog(printer, parent);
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
                    AscAppManager::printData().setPrinterInfo(*printer);

                QVector<PageRanges> page_ranges;

#ifdef Q_OS_LINUX
                if ( printer->outputFormat() == QPrinter::PdfFormat ) {
                    if ( !AscAppManager::printData().isQuickPrint() ) {
                        info.setFile(printer->outputFileName());
                        Utils::keepLastPath(LOCAL_PATH_SAVE, info.absolutePath());
                    }
                } else {
                    if ( AscAppManager::printData().isQuickPrint() && !printer->outputFileName().isEmpty() ) {
                        info.setFile(printer->outputFileName());
                        if ( info.suffix() == "pdf" )
                            printer->setOutputFileName("");
                    }
                }
#endif

                switch(dialog->printRange()) {
                case QPrintDialog::AllPages:
                    page_ranges.append(PageRanges(1, pagescount));
                    break;
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
        } else
            CMessage::warning(parent, tr("There are no pages set to print."));

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
            if ( iconuser ) {
                adjustIconUser();
            }

            if ( iconcrypted ) {
                iconcrypted->setPixmap(QIcon{":/title/icons/secure.svg"}.pixmap(QSize(20,20) * f));
                iconcrypted->setFixedSize(ICON_SIZE * f);
            }

            for (const auto& btn: m_mapTitleButtons) {
                btn->setFixedSize(QSize(int(TITLEBTN_WIDTH*f), int(TOOLBTN_HEIGHT*f)));
                btn->setIconSize(QSize(20,20) * f);
            }
            centerTitle(f);
        }
    }

    void onFullScreen(bool apply)
    {
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
        if ( m_panel->data()->closed() ) return;

        QJsonParseError jerror;
        QByteArray stringdata = QString::fromStdWString(wjson).toUtf8();
        QJsonDocument jdoc = QJsonDocument::fromJson(stringdata, &jerror);

        if( jerror.error == QJsonParseError::NoError ) {
            QJsonObject objRoot = jdoc.object();
            std::vector<QString> _portals{objRoot["domain"].toString()};

            if ( objRoot.contains("extra") && objRoot["extra"].isArray() ) {
                QJsonArray a = objRoot["extra"].toArray();
                for (auto&& v: a) {
                    _portals.push_back(v.toString());
                }
            }

            for (auto& u: _portals) {
                if ( QString::fromStdWString(m_panel->data()->url()).startsWith(u) ) {
                    window->closeWindow();
                    break;
                }
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
            iconuser = new QLabel(window->m_boxTitleBtns);
            iconuser->setObjectName("iconuser");
            iconuser->setContentsMargins(0,0,0,0);
            iconuser->setAlignment(Qt::AlignCenter);
        }

        return iconuser;
    }

    void adjustIconUser()
    {
        iconuser->setFixedHeight(20 * window->m_dpiRatio);
        iconuser->setFixedWidth(iconuser->height());
        iconuser->setStyleSheet(QString("#iconuser {border-radius: %1px;}")
                     .arg(QString::number(iconuser->height()/2)));
    }

    QLabel * iconCrypted()
    {
        Q_ASSERT(window->m_labelTitle != nullptr);
        if ( !iconcrypted ) {
            iconcrypted = new QLabel(window->m_labelTitle);
            iconcrypted->setObjectName("iconcrypted");

            iconcrypted->setPixmap(QIcon{":/title/icons/secure.svg"}.pixmap(QSize(20,20) * window->m_dpiRatio));
            iconcrypted->setFixedSize(ICON_SIZE * window->m_dpiRatio);
            iconcrypted->show();
            int y = (window->m_labelTitle->height() - ICON_SIZE.height() * window->m_dpiRatio)/2;
            iconcrypted->move(0, y);
            connect(window->m_labelTitle, &CElipsisLabel::onResize, this, [=](QSize size, int textWidth) {
                if (iconcrypted) {
                    int x = (size.width() - textWidth)/2 - ((ICON_SIZE.width() + 6) * window->m_dpiRatio);
                    int y = (size.height() - ICON_SIZE.height() * window->m_dpiRatio)/2;
                    iconcrypted->move(x, y);
                }
            });
        }

        return iconcrypted;
    }

    void onWebAppsFeatures(int, std::wstring f) override
    {
        bool is_read_only = panel()->data()->hasFeature(L"readonly\":true");
        panel()->data()->setFeatures(f);

        if ( m_panel->data()->hasFeature(L"uitype\":\"fillform") ) {
             ffWindowCustomize();
             centerTitle(window->m_dpiRatio);
        }

        if ( panel()->data()->hasFeature(L"crypted\":true") && boxtitlelabel && !iconcrypted ) {
             iconCrypted();
        }

        if ( is_read_only != panel()->data()->hasFeature(L"readonly\":true") && boxtitlelabel ) {
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

    auto fillformMode() -> bool {
        QFileInfo i{QString::fromStdWString(m_panel->data()->url())};
        return i.suffix() == "oform" || m_panel->data()->hasFeature(L"uitype\":\"fillform");
    }

    auto calcTitleLabelWidth(int basewidth) const -> int {
        if ( iconuser )
            basewidth -= iconuser->width();

        basewidth -= boxtitlelabel->contentsMargins().left() + boxtitlelabel->contentsMargins().right();
        if ( iconcrypted )
            basewidth -= iconcrypted->width();

        basewidth -= m_mapTitleButtons.count() * (TITLEBTN_WIDTH + 1) * window->m_dpiRatio;

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
        boxtitlelabel->layout()->addWidget(window->m_labelTitle);
        if ( m_panel->data()->hasFeature(L"crypted\":true") && !iconcrypted ) {
            iconCrypted();
        }

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
        if ( !_layout->itemAtPosition(0,0) && !_layout->findChild<QWidget*>(window->m_boxTitleBtns->objectName()) ) {
            _layout->addWidget(window->m_boxTitleBtns,0,0,Qt::AlignTop);
            if (iconuser)
                 iconuser->hide();
            auto layout = qobject_cast<QHBoxLayout*>(window->m_boxTitleBtns->layout());
            auto stretch = layout->takeAt(1);
            if (stretch)
                 delete stretch;
            stretch = layout->takeAt(2);
            if (stretch)
                 delete stretch;
        }
    }
};

#endif // CEDITORWINDOW_P_H
