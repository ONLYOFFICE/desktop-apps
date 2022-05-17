
#include "cdialogcertificateinfo.h"
#include <QBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <QPushButton>
#include <QDebug>

class CDialogCertificateInfo::Intf {
public:
    Intf(QWidget * p)
        : textInfo(new QTextEdit(p))
        , buttonOk(new QPushButton(p))
    {
        QPalette palette(textInfo->palette());
        palette.setColor(QPalette::Base, p->palette().color(QPalette::Window));
        textInfo->setPalette(palette);
        textInfo->setReadOnly(true);

        buttonOk->setText(tr("OK"));
        buttonOk->setFixedWidth(80);
        buttonOk->setAutoDefault(true);
    }

    QTextEdit * textInfo = nullptr;
    QPushButton * buttonOk = nullptr;
};

CDialogCertificateInfo::CDialogCertificateInfo(QWidget *parent)
    : QDialog(parent)
    , m_priv(new CDialogCertificateInfo::Intf(this))
{
    setMinimumWidth(500);
    setMinimumHeight(400);
    setWindowTitle(tr("Certificate Details"));

    QVBoxLayout * _layout = new QVBoxLayout;
    _layout->addWidget(m_priv->textInfo);

    QHBoxLayout * _hlayout = new QHBoxLayout;
    _hlayout->addWidget(m_priv->buttonOk);
    _layout->addLayout(_hlayout);

    connect(m_priv->buttonOk, &QPushButton::clicked, [=]{
        close();
    });

    setLayout(_layout);
}

CDialogCertificateInfo::CDialogCertificateInfo(QWidget * p, const std::wstring & info)
    : CDialogCertificateInfo(p)
{
    m_priv->textInfo->setText(QString::fromStdWString(info));
}

