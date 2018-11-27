#ifndef CDPICHECKER_H
#define CDPICHECKER_H

#include <QDesktopWidget>
#include <QScreen>

#include "applicationmanager.h"

class CDpiChecker : public CAscDpiChecker
{
public:
    // wrappers
    virtual int GetWindowDpi(WindowHandleId wid, unsigned int* dx, unsigned int* dy)
    {
        return CAscDpiChecker::GetWindowDpi(wid, dx, dy);
    }

    virtual int GetMonitorDpi(int nScreenNumber, unsigned int* dx, unsigned int* dy)
    {
        int nBaseRet = CAscDpiChecker::GetMonitorDpi(nScreenNumber, dx, dy);
        if (-1 != nBaseRet)
            return nBaseRet;

        QScreen * _screen;
        if (nScreenNumber >=  0 && nScreenNumber < QApplication::screens().count())
            _screen = QApplication::screens().at(nScreenNumber);
        else
            _screen = QApplication::primaryScreen();

        int nDpiX = _screen->physicalDotsPerInchX();
        int nDpiY = _screen->physicalDotsPerInchY();

        QSize size = _screen->size();
        if (nDpiX > 150 && nDpiX < 180 && nDpiY > 150 && nDpiY < 180 && size.width() >= 3840 && size.height() >= 2160)
        {
            nDpiX = 192;
            nDpiY = 192;
        }

        *dx = nDpiX;
        *dy = nDpiY;

        return 0;
    }

    // app realize
    virtual int GetWidgetImplDpi(CCefViewWidgetImpl* w, unsigned int* dx, unsigned int* dy)
    {
        return GetWidgetDpi((QCefView*)w, dx, dy);
    }

    virtual int GetWidgetDpi(QWidget* w, unsigned int* dx, unsigned int* dy)
    {
        QDesktopWidget* pDesktop = QApplication::desktop();
        if (!pDesktop && (0 == pDesktop->screenCount()))
        {
            *dx = 96;
            *dy = 96;
            return 0;
        }
        int nScreenNumber = QApplication::desktop()->screenNumber(w);
        return GetMonitorDpi(nScreenNumber, dx, dy);
    }
};

#endif // CDPICHECKER_H
