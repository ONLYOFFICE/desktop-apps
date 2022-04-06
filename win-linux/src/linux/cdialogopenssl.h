#ifndef DIALOG_OPEN_SSL_H
#define DIALOG_OPEN_SSL_H

#include <QMainWindow>
#include <QTextEdit>
#include <QPushButton>
#include <QResizeEvent>
#include <QPlainTextEdit>
#include <QDialog>

#include "applicationmanager.h"

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
    bool m_CmdUseNativeDialogFlag;

protected:
    CSslDialog_Private * m_private;

private slots:
    void onBtnCertificateClick();
    void onBtnKeyClick();
};

#endif // DIALOG_OPEN_SSL_H
