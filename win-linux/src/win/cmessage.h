#ifndef CMESSAGE_H
#define CMESSAGE_H

#if defined(_WIN32)
#include "qwinwidget.h"
#endif

#include <QFormLayout>
#include <QDialog>
#include <QLabel>

class CMessage : public QWinWidget
{
    Q_OBJECT

public:
    explicit CMessage(HWND hParentWnd);

    void error(const QString& title, const QString& text);
    int showModal(const QString&);

private:
    QDialog m_pDlg;
    int m_result;
    QFormLayout * m_fLayout;
    QLabel * m_message;

signals:

public slots:
    void onYesClicked();
};

#endif // CMESSAGE_H
