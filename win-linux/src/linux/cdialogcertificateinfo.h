#ifndef CDIALOGCERTIFICATEINFO_H
#define CDIALOGCERTIFICATEINFO_H

#include <QDialog>

class CDialogCertificateInfo : public QDialog
{
public:
    explicit CDialogCertificateInfo(QWidget *parent);
    CDialogCertificateInfo(QWidget *, const std::wstring &);

private:
    class Intf;
    Intf * m_priv;
};

#endif // CDIALOGCERTIFICATEINFO_H
