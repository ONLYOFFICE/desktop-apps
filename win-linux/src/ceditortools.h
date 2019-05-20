#ifndef CEDITORTOOLS_H
#define CEDITORTOOLS_H

#include "qascprinter.h"
#include "cascapplicationmanagerwrapper.h"

namespace CEditorTools
{
    struct sPrintConf
    {
#ifdef Q_OS_WIN
        sPrintConf(CCefView * v, QAscPrinterContext * c, int s, int b, HWND p)
#else
        sPrintConf(CCefView * v, QAscPrinterContext * c, int s, int b, QWidget * p)
#endif
            : view(v)
            , context(c)
            , pagetstart(s)
            , pagestop(b)
            , parent(p)
        {}

        CCefView * view;
        QAscPrinterContext * context;
        int pagetstart,
            pagestop;
#ifdef Q_OS_WIN
        HWND parent;
#else
        QWidget * parent;
#endif
    };

    void print(const sPrintConf&);
}

#endif // CEDITORTOOLS_H
