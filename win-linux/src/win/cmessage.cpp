#include "cmessage.h"
#include "../defines.h"

#include <QMessageBox>
#include <QDialogButtonBox>
#include <QPushButton>

#include <QLabel>
#include "qcefview.h"

extern BYTE g_dpi_ratio;

CMessage::CMessage(HWND hParentWnd)
    : QWinWidget(hParentWnd)
    , m_pDlg(this)
    , m_result(MODAL_RESULT_CANCEL)
    , m_fLayout(new QFormLayout)
{
    m_pDlg.setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint
                          | Qt::WindowCloseButtonHint | Qt::MSWindowsFixedSizeDialogHint);

    QVBoxLayout * layout = new QVBoxLayout;
    QHBoxLayout * h_layout2 = new QHBoxLayout;
    QHBoxLayout * h_layout1 = new QHBoxLayout;
    layout->addLayout(h_layout2, 1);
    layout->addLayout(h_layout1, 0);

    QLabel * icon = new QLabel;
    icon->setProperty("class","msg-icon");
    icon->setProperty("type","msg-error");
    icon->setFixedSize(35*g_dpi_ratio, 35*g_dpi_ratio);

    m_message = new QLabel("some message");
    m_message->setStyleSheet(QString("margin-bottom: %1px;").arg(8*g_dpi_ratio));
//    question->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    m_fLayout->addWidget(m_message);
    h_layout2->addWidget(icon, 0, Qt::AlignTop);
    h_layout2->addLayout(m_fLayout, 1);

    QPushButton * btn_yes       = new QPushButton(tr("&OK"));
//    QPushButton * btn_no        = new QPushButton("&No");
//    QPushButton * btn_cancel    = new QPushButton("&Cancel");
    QWidget * box = new QWidget;
    box->setLayout(new QHBoxLayout);
    box->layout()->addWidget(btn_yes);
//    box->layout()->addWidget(btn_no);
//    box->layout()->addWidget(btn_cancel);
    box->layout()->setContentsMargins(0,8*g_dpi_ratio,0,0);
    h_layout1->addWidget(box, 0, Qt::AlignCenter);

    m_pDlg.setLayout(layout);
    m_pDlg.setMinimumWidth(300*g_dpi_ratio);
    m_pDlg.setWindowTitle(APP_TITLE);

    connect(btn_yes, &QPushButton::clicked, this, &CMessage::onYesClicked);
//    connect(btn_no, SIGNAL(clicked()), this, SLOT(onNoClicked()));
//    connect(btn_cancel, SIGNAL(clicked()), this, SLOT(onCancelClicked()));
}

void CMessage::error(const QString& title, const QString& text)
{
    QMessageBox msgBox(this);
    msgBox.setWindowTitle( title );
    msgBox.setText( text );
    msgBox.setWindowModality(Qt::ApplicationModal);
    msgBox.setIcon(QMessageBox::Critical);

    msgBox.setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint
                          | Qt::WindowCloseButtonHint | Qt::MSWindowsFixedSizeDialogHint);
    msgBox.exec();

//    msgBox->setModal( false ); // if you want it non-modal
//    msgBox->open( this, SLOT(msgBoxClosed(QAbstractButton*)) );
//    QMessageBox::critical(new CMessage(wnd), title, text, QMessageBox::Ok, QMessageBox::Ok);
}

int CMessage::showModal(const QString& mess)
{
    m_message->setText(mess);
    m_pDlg.adjustSize();

#if defined(_WIN32)
    RECT rc;
    ::GetWindowRect(parentWindow(), &rc);

    int x = rc.left + (rc.right - rc.left - m_pDlg.width())/2;
    int y = (rc.bottom - rc.top - m_pDlg.height())/2;

    m_pDlg.move(x, y);
#endif

    m_pDlg.exec();

    return m_result;
}

void CMessage::onYesClicked()
{
    m_result = MODAL_RESULT_YES;
    m_pDlg.accept();
}
