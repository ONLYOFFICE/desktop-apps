#ifndef CMESSAGE_H
#define CMESSAGE_H

#if defined(_WIN32)
#include "qwinwidget.h"
#endif

#include <QWidget>

class CMessage : public QWinWidget
{
    Q_OBJECT

public:
    explicit CMessage(HWND hParentWnd);

    void error(const QString& title, const QString& text);

signals:

public slots:
};

#endif // CMESSAGE_H
