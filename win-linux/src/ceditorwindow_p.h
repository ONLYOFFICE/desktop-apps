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

#include <QPrinterInfo>

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

public:
    CEditorWindowPrivate(CEditorWindow * w) : window(w)
    {}

    void onEditorConfig(int, std::wstring cfg)
    {
//        if ( id == window->holdView(id) )

        QJsonParseError jerror;
        QJsonDocument jdoc = QJsonDocument::fromJson(QString::fromStdWString(cfg).toLatin1(), &jerror);

        if( jerror.error == QJsonParseError::NoError ) {
            QJsonObject objRoot = jdoc.object();

            if ( objRoot.contains("user") ) {
                iconuser->setToolTip(objRoot["user"].toObject().value("name").toString());
            }

            if ( objRoot.contains("extraleft") ) {
                titleLeftOffset = objRoot["extraleft"].toInt();

                int diffW = (titleLeftOffset - (TOOLBTN_WIDTH * 5)) * window->m_dpiRatio; // 5 right tool buttons: close, min, max, drop, user icon
                QString _label_styles = diffW > 0 ? QString("padding:0 %1px 0 0;").arg(diffW) : QString("padding:0 0 0 %1px;").arg(abs(diffW));
                window->m_labelTitle->setStyleSheet(_label_styles);
            }
        }
    }

    void onDocumentName(void * data)
    {
        CCefEventsGate::onDocumentName(data);

        if ( window->m_labelTitle )
            window->m_labelTitle->setText(m_panel->data()->title());
    }

    void onDocumentChanged(int id, bool state)
    {
        CCefEventsGate::onDocumentChanged(id, state);

        if ( window->m_labelTitle )
            window->m_labelTitle->setText(m_panel->data()->title());
    }

    void onDocumentSave(int id, bool cancel = false)
    {
        CCefEventsGate::onDocumentSave(id, cancel);

        if ( m_panel->data()->closed() ) {
            if ( !((CCefViewEditor *)m_panel->cef())->CheckCloudCryptoNeedBuild() ) {
                AscAppManager::getInstance().DestroyCefView(m_panel->cef()->GetId());
                window->hide();
            }
        }
    }

    void onDocumentSaveInnerRequest(int)
    {
        CMessage mess(parentWindow(), CMessageOpts::moButtons::mbYesDefNo);
        int reply = mess.confirm(QObject::tr("Document must be saved to continue.<br>Save the document?"));

        CAscEditorSaveQuestion * pData = new CAscEditorSaveQuestion;
        pData->put_Value((reply == MODAL_RESULT_CUSTOM + 0) ? true : false);

        CAscMenuEvent * pEvent = new CAscMenuEvent(ASC_MENU_EVENT_TYPE_DOCUMENTEDITORS_SAVE_YES_NO);
        pEvent->m_pData = pData;

        m_panel->cef()->Apply(pEvent);
    }

    void onDocumentFragmented(int id, bool needbuild)
    {
        if ( needbuild ) {
//            static const bool _skip_user_warning = !Utils::appArgsContains("--warning-doc-fragmented");
//            if ( _skip_user_warning )
                m_panel->cef()->Apply(new CAscMenuEvent(ASC_MENU_EVENT_TYPE_ENCRYPTED_CLOUD_BUILD));
        } else
            onDocumentFragmentedBuild(id, 0);

    }

    void onDocumentFragmentedBuild(int id, int error)
    {
        CCefEventsGate::onDocumentFragmentedBuild(id, error);

        if ( m_panel->data()->closed() ) {
            AscAppManager::getInstance().DestroyCefView(m_panel->cef()->GetId());
            window->hide();
        }
    }

    void onDocumentPrint(int currentpage, uint pagescount)
    {
        if ( isPrinting ) return;
        isPrinting = true;

        if ( !(pagescount < 1) ) {
            CAscMenuEvent * pEvent;
            QAscPrinterContext * pContext = m_printData._printer_info.isNull() ?
                        new QAscPrinterContext() : new QAscPrinterContext(m_printData._printer_info);

            QPrinter * printer = pContext->getPrinter();
            printer->setOutputFileName("");
            printer->setFromTo(1, pagescount);

#ifdef _WIN32
            CPrintDialogWinWrapper wrapper(printer, parentWindow());
            QPrintDialog * dialog = wrapper.q_dialog();
#else
            QPrintDialog * dialog =  new QPrintDialog(printer, this);
#endif // _WIN32

            dialog->setWindowTitle(tr("Print Document"));
            dialog->setEnabledOptions(QPrintDialog::PrintPageRange | QPrintDialog::PrintCurrentPage | QPrintDialog::PrintToFile);
            if (!(currentpage < 0))
                currentpage++, dialog->setOptions(dialog->options() | QPrintDialog::PrintCurrentPage);
            dialog->setPrintRange(m_printData._print_range);

            int start = -1, finish = -1;
            if ( dialog->exec() == QDialog::Accepted ) {
                m_printData._printer_info = QPrinterInfo::printerInfo(printer->printerName());
                m_printData._print_range = dialog->printRange();

                switch(dialog->printRange()) {
                case QPrintDialog::AllPages: start = 1, finish = pagescount; break;
                case QPrintDialog::PageRange:
                    start = dialog->fromPage(), finish = dialog->toPage(); break;
                case QPrintDialog::Selection: break;
                case QPrintDialog::CurrentPage: start = currentpage, finish = currentpage; break;
                }

                CEditorTools::print({m_panel->cef(), pContext, start, finish, parentWindow()});
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


    void onEditorAllowedClose(int)
    {
        AscAppManager::unbindReceiver(m_panel->cef()->GetId());
        AscAppManager::closeEditorWindow(size_t(window));
    }

    void onLocalFileSaveAs(void * d)
    {
        window->onLocalFileSaveAs(d);
    }

    void onScreenScalingFactor(uint f)
    {
        int diffW = (titleLeftOffset - (TOOLBTN_WIDTH * 5)) * f; // 5 tool buttons: min+max+close+drop+usericon
        QString _label_styles = diffW > 0 ? QString("padding:0 %1px 0 0;").arg(diffW) : QString("padding:0 0 0 %1px;").arg(abs(diffW));
        window->m_labelTitle->setStyleSheet(_label_styles);

        if ( iconuser ) {
            iconuser->setPixmap(f > 1 ? QPixmap(":/user_2x.png") : QPixmap(":/user.png"));
            iconuser->setFixedSize(QSize(TOOLBTN_WIDTH*f, 16*f));
        }

        if ( btndock )
            btndock->setFixedSize(QSize(TOOLBTN_WIDTH*f, TOOLBTN_HEIGHT*f));
    }

    QLabel * const iconUser()
    {
        if ( !iconuser ) {
            iconuser = new QLabel(window->m_boxTitleBtns);
            iconuser->setPixmap(window->m_dpiRatio > 1 ? QPixmap(":/user_2x.png") : QPixmap(":/user.png"));
            iconuser->setFixedSize(QSize(TOOLBTN_WIDTH*window->m_dpiRatio,16*window->m_dpiRatio));
            iconuser->setAlignment(Qt::AlignCenter);
        }

        return iconuser;
    }

    QPushButton * const buttonDock()
    {
        if ( !btndock ) {
            btndock = window->createToolButton(window->m_boxTitleBtns);
            btndock->setObjectName("toolButtonDock");
        }

        return btndock;
    }

#ifdef Q_OS_WIN
    HWND parentWindow() const
    {
        return (HWND)window->m_pMainPanel->winId();
    }
#else
    QWidget * parentWindow() const
    {
        return window->m_pMainPanel;
    }
#endif

public:
    int titleLeftOffset = 168;

private:
    CEditorWindow * window = nullptr;
    QLabel * iconuser = nullptr;
    QPushButton * btndock = nullptr;
    bool isPrinting = false;
};

#endif // CEDITORWINDOW_P_H
