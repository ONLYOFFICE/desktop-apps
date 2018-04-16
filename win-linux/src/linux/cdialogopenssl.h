#ifndef DIALOG_OPEN_SSL_H
#define DIALOG_OPEN_SSL_H

#include <QMainWindow>
#include <QTextEdit>
#include <QPushButton>
#include <QResizeEvent>
#include <QPlainTextEdit>
#include <QDialog>

#include "../../../desktop-sdk/ChromiumBasedEditors/lib/include/applicationmanager.h"

class CCertificateSelectDialogOpenSsl : public ICertificateSelectDialogOpenSsl
{
public:
    QWidget*                m_pParent;
    CAscApplicationManager* m_pManager = nullptr;

    std::wstring            m_sCertPath;
    std::wstring            m_sCertPassword;
    std::wstring            m_sKeyPath;
    std::wstring            m_sKeyPassword;

public:
    CCertificateSelectDialogOpenSsl(QWidget * parent = nullptr) : m_pParent(parent) {}
    virtual ~CCertificateSelectDialogOpenSsl(){}

public:
    virtual std::wstring GetCertificatePath();
    virtual std::wstring GetCertificatePassword();

    virtual std::wstring GetKeyPath();
    virtual std::wstring GetKeyPassword();

    virtual bool ShowSelectDialog();
    virtual int ShowCertificate(ICertificate* pCert);
};

class CSslDialog_Private;
class CDialogOpenSsl : public QDialog
{
    Q_OBJECT

public:
    explicit CDialogOpenSsl(QWidget *parent, CCertificateSelectDialogOpenSsl* openssl);
    ~CDialogOpenSsl();

    virtual void resizeEvent(QResizeEvent*);
    void CalculatePlaces();

    friend class CCertificateSelectDialogOpenSsl;

public:
    bool m_bIsOK;

private:
    bool checkCertificate();

protected:
    CSslDialog_Private * m_private;

private slots:
    void onBtnCertificateClick();
    void onBtnKeyClick();
};

#endif // DIALOG_OPEN_SSL_H
