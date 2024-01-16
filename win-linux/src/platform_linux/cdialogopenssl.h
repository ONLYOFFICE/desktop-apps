#ifndef DIALOG_OPEN_SSL_H
#define DIALOG_OPEN_SSL_H

#include <QDialog>
#include "applicationmanager_events.h"


class CSslDialog_Private;
class CDialogOpenSsl : public QDialog
{
    Q_OBJECT

public:
    explicit CDialogOpenSsl(QWidget *parent);
    ~CDialogOpenSsl();

    void getResult(NSEditorApi::CAscOpenSslData&);
private:
    bool checkCertificate();

protected:
    CSslDialog_Private * m_private;

private slots:
    void onBtnCertificateClick();
    void onBtnKeyClick();
};

#endif // DIALOG_OPEN_SSL_H
