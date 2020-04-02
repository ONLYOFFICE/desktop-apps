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
#include "cmessage.h"
#include "qascprinter.h"
#include "ceditortools.h"
#include "csvgpushbutton.h"

#include <QPrinterInfo>
#include <QDesktopWidget>
#include <QJsonDocument>
#include <QJsonArray>

#ifdef _WIN32
#include "win/cprintdialog.h"
#else
#endif


using namespace NSEditorApi;

class CEditorWindowPrivate : public CCefEventsGate
{
    struct sPrintData {
        sPrintData() : _print_range(QPrintDialog::PrintRange::AllPages)
        {}

        QPrinterInfo _printer_info;
        QPrintDialog::PrintRange _print_range;
    };

    sPrintData m_printData;

    CEditorWindow * window = nullptr;
    QLabel * iconuser = nullptr;
    QPushButton * btndock = nullptr;
    bool isPrinting = false,
        isFullScreen = false;
    Qt::WindowFlags window_orig_flags;

    QMap<QString, CSVGPushButton*> m_mapTitleButtons;

public:
    int titleLeftOffset = 0;
    bool isReporterMode = false;

public:
    CEditorWindowPrivate(CEditorWindow * w) : window(w)
    {}

    QPushButton * cloneEditorHeaderButton(const QJsonObject& jsonobj)
    {
        QString action = jsonobj["action"].toString();
        CSVGPushButton * btn = new CSVGPushButton;
        btn->setProperty("class", "normal");
        btn->setProperty("act", "tool");
        btn->setFixedSize(QSize(TOOLBTN_WIDTH,TOOLBTN_HEIGHT) * window->m_dpiRatio);
        btn->setDisabled(jsonobj["disabled"].toBool());
        btn->setIconSize(QSize(20,20) * window->m_dpiRatio);
        btn->setMouseTracking(true);

        m_mapTitleButtons[action] = btn;

        connect(btn, &QPushButton::clicked, [=]{
            QJsonObject _json_obj{{"action", action}};
            AscAppManager::sendCommandTo(panel()->cef(), L"button:click", Utils::encodeJson(_json_obj).toStdWString());
        });

        if ( jsonobj.contains("icon") ) {
            btn->setIcon(":/title/icons/buttons.svg", "svg-btn-" + jsonobj["icon"].toString());
        } else btn->setIcon(":/title/icons/buttons.svg", "svg-btn-" + action);

        btn->setToolTip(jsonobj["hint"].toString());
        return btn;
    }

    void onEditorConfig(int, std::wstring cfg)
    {
//        if ( id == window->holdView(id) )
        QJsonParseError jerror;
        QJsonDocument jdoc = QJsonDocument::fromJson(QString::fromStdWString(cfg).toUtf8(), &jerror);
        if( jerror.error == QJsonParseError::NoError ) {
            QJsonObject objRoot = jdoc.object();

            int _user_width = 0;
            if ( canExtendTitle() ) {
                if ( objRoot.contains("user") ) {
                    iconuser->setToolTip(objRoot["user"].toObject().value("name").toString());
                    iconuser->setText(objRoot["user"].toObject().value("name").toString());

                    iconuser->adjustSize();
                    _user_width = iconuser->width();
                }

                if ( objRoot.contains("title") ) {
                    QJsonArray _btns = objRoot["title"].toObject().value("buttons").toArray();

                    QPushButton * _btn;
                    for (int i = _btns.size(); i --> 0; ) {
                        _btn = cloneEditorHeaderButton(_btns.at(i).toObject());
                        qobject_cast<QHBoxLayout *>(window->m_boxTitleBtns->layout())->insertWidget(0, _btn);

                        titleLeftOffset += 40/*_btn->width()*/;
                    }

                }
            }

            int _btncount = /*iconuser ? 4 :*/ 3;
            int diffW = (titleLeftOffset - TOOLBTN_WIDTH * _btncount) * window->m_dpiRatio; // 4 right tool buttons: close, min, max, user icon
            diffW -= _user_width;

            diffW > 0 ? window->m_labelTitle->setContentsMargins(0, 0, diffW, 2*window->m_dpiRatio) :
                            window->m_labelTitle->setContentsMargins(-diffW, 0, 0, 2*window->m_dpiRatio);
        }
    }

    void onDocumentName(void * data) override
    {
        CCefEventsGate::onDocumentName(data);

        if ( canExtendTitle() ) {
            window->setWindowTitle(m_panel->data()->title());
            window->m_boxTitleBtns->repaint();
        }
    }

    void onDocumentChanged(int id, bool state) override
    {
        if ( panel()->data()->hasChanges() != state ) {
            CCefEventsGate::onDocumentChanged(id, state);

            if ( canExtendTitle() ) {
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

    void onDocumentPrint(int currentpage, uint pagescount) override
    {
        if ( isPrinting ) return;
        isPrinting = true;

#ifdef Q_OS_LINUX
        WindowUtils::CParentDisable locker(window);
#endif
        if ( !(pagescount < 1) ) {
            CAscMenuEvent * pEvent;
            QAscPrinterContext * pContext = m_printData._printer_info.isNull() ?
                        new QAscPrinterContext() : new QAscPrinterContext(m_printData._printer_info);

            QPrinter * printer = pContext->getPrinter();
            printer->setOutputFileName("");
            printer->setFromTo(1, pagescount);

#ifdef _WIN32
            CPrintDialogWinWrapper wrapper(printer, window->handle());
            QPrintDialog * dialog = wrapper.q_dialog();
#else
            QPrintDialog * dialog =  new QPrintDialog(printer, window->handle());
#endif // _WIN32

            dialog->setWindowTitle(CEditorWindow::tr("Print Document"));
            dialog->setEnabledOptions(QPrintDialog::PrintPageRange | QPrintDialog::PrintCurrentPage | QPrintDialog::PrintToFile);
            if (!(currentpage < 0))
                currentpage++, dialog->setOptions(dialog->options() | QPrintDialog::PrintCurrentPage);
            dialog->setPrintRange(m_printData._print_range);

            int start = -1, finish = -1;
#ifdef _WIN32
            if ( wrapper.showModal() == QDialog::Accepted ) {
#else
            if ( dialog->exec() == QDialog::Accepted ) {
#endif
                m_printData._printer_info = QPrinterInfo::printerInfo(printer->printerName());
                m_printData._print_range = dialog->printRange();

                switch(dialog->printRange()) {
                case QPrintDialog::AllPages: start = 1, finish = pagescount; break;
                case QPrintDialog::PageRange:
                    start = dialog->fromPage(), finish = dialog->toPage(); break;
                case QPrintDialog::Selection: break;
                case QPrintDialog::CurrentPage: start = currentpage, finish = currentpage; break;
                }

                CEditorTools::print({m_panel->cef(), pContext, start, finish, window->handle()});
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
    }

    void onLocalFileSaveAs(void * d) override
    {
        window->onLocalFileSaveAs(d);
    }

    void onScreenScalingFactor(uint f)
    {
        int _btncount = /*iconuser ? 4 :*/ 3;
        int diffW = (titleLeftOffset - (TOOLBTN_WIDTH * _btncount)) * f; // 4 tool buttons: min+max+close+usericon

        if ( iconuser ) {
//            iconuser->setPixmap(f > 1 ? QPixmap(":/user_2x.png") : QPixmap(":/user.png"));
//            iconuser->setFixedSize(QSize(TOOLBTN_WIDTH*f, 16*f));

            iconuser->setContentsMargins(0,0,0,2*f);
            iconuser->adjustSize();
            diffW -= iconuser->width();
        }

        diffW > 0 ? window->m_labelTitle->setContentsMargins(0, 0, diffW, 2*f) :
                        window->m_labelTitle->setContentsMargins(-diffW, 0, 0, 2*f);

//        if ( btndock )
//            btndock->setFixedSize(QSize(TOOLBTN_WIDTH*f, TOOLBTN_HEIGHT*f));

        for (auto btn: m_mapTitleButtons) {
            btn->setFixedSize(QSize(TOOLBTN_WIDTH*f, TOOLBTN_HEIGHT*f));
            btn->setIconSize(QSize(20,20) * f);
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

#ifdef Q_OS_LINUX
            _fs_widget->overrideWindowFlags(window_orig_flags);
#endif
            window->show(false);

//            _fs_widget->view()->resize(_fs_widget->size().width(), _fs_widget->size().height()-1);
            window->m_pMainPanel->layout()->addWidget(_fs_widget);
            window->recalculatePlaces();
            _fs_widget->showNormal();
            _fs_widget->cef()->focus();

            disconnect(cefConnection);
        } else {
            QPoint pt = _fs_widget->mapToGlobal(_fs_widget->pos());
            _fs_widget->setWindowIcon(Utils::appIcon());
            _fs_widget->setWindowTitle(panel()->data()->title());

#ifdef _WIN32
            _fs_widget->setParent(nullptr);
            _fs_widget->showFullScreen();
#else
            window_orig_flags = _fs_widget->windowFlags();
            _fs_widget->setParent(nullptr);
            _fs_widget->setWindowFlags(Qt::FramelessWindowHint);
            _fs_widget->showFullScreen();
            _fs_widget->setGeometry(QApplication::desktop()->screenGeometry(pt));
#endif
            _fs_widget->view()->setFocusToCef();
            window->hide();

            cefConnection = connect(_fs_widget, &CTabPanel::closePanel, [=](QCloseEvent * e){
                _break_demonstration();

                e->ignore();
                window->closeWindow();
            });

#ifdef _WIN32
            _fs_widget->setGeometry(QApplication::desktop()->screenGeometry(pt));
            _fs_widget->setWindowState(Qt::WindowFullScreen);                       // fullscreen widget clears that flag after changing geometry
#endif
        }
    }

    void onFullScreen(int, bool apply) override
    {
        onFullScreen(apply);
    }

    void onPortalLogout(wstring portal) override
    {
        if ( m_panel && !portal.empty() ) {
            if ( !m_panel->data()->closed() &&
                    m_panel->data()->url().find(portal) != wstring::npos )
            {
                window->closeWindow();
            }
        }
    }

    void onFileLocation(int, QString param)
    {
        if ( param == "offline" ) {
            const wstring& path = m_panel->data()->url();
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
        if ( !iconuser ) {
            iconuser = new QLabel(window->m_boxTitleBtns);
            iconuser->setObjectName("iconuser");
            iconuser->setContentsMargins(0,0,0,2 * window->m_dpiRatio);
//            iconuser->setPixmap(window->m_dpiRatio > 1 ? QPixmap(":/user_2x.png") : QPixmap(":/user.png"));
//            iconuser->setFixedSize(QSize(TOOLBTN_WIDTH*window->m_dpiRatio,16*window->m_dpiRatio));
        }

        return iconuser;
    }

    QPushButton * buttonDock()
    {
        if ( !btndock ) {
            btndock = window->createToolButton(window->m_boxTitleBtns);
            btndock->setObjectName("toolButtonDock");
        }

        return btndock;
    }

    void onWebAppsFeatures(int, std::wstring f) override
    {
        panel()->data()->setFeatures(f);
    }

    void onWebTitleChanged(int, std::wstring json)
    {
        if ( !m_mapTitleButtons.isEmpty() ) {
            QJsonParseError jerror;
            QJsonDocument jdoc = QJsonDocument::fromJson(QString::fromStdWString(json).toUtf8(), &jerror);
            if( jerror.error == QJsonParseError::NoError ) {
                QJsonObject objRoot = jdoc.object();

                if (objRoot.contains("disabled")) {
                    QJsonObject _disabled = objRoot["disabled"].toObject();

                    if ( _disabled.contains("all") ) {
                        bool _is_disabled = _disabled["all"].toBool();

                        for (auto& btn: m_mapTitleButtons) {
                            btn->setDisabled(_is_disabled);
                        }
                    } else {
                        for (const auto& k: _disabled.keys()) {
                            m_mapTitleButtons[k]->setDisabled(_disabled.value(k).toBool());
                        }
                    }
                } else
                if (objRoot.contains("icon:changed")) {
                    QJsonObject _btns_changed = objRoot["icon:changed"].toObject();

                    for (const auto& b: _btns_changed.keys()) {
                        m_mapTitleButtons[b]->setIcon(":/title/icons/buttons.svg", "svg-btn-" + _btns_changed.value(b).toString());
                    }
                }
            }
        }
    }

    bool canExtendTitle()
    {
        return /* !panel()->isReadonly() && */ panel()->data()->isLocal() || panel()->data()->hasFeature(L"titlebuttons:");
    }
};

#endif // CEDITORWINDOW_P_H
