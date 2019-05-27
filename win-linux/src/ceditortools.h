#ifndef CEDITORTOOLS_H
#define CEDITORTOOLS_H

#include "qascprinter.h"
#include "cascapplicationmanagerwrapper.h"

namespace CEditorTools
{
    struct sPrintConf
    {
        sPrintConf(CCefView * v, QAscPrinterContext * c, int s, int b, ParentHandle p)
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
        ParentHandle parent;
    };

    void print(const sPrintConf&);
    void getlocalfile(void * data);
}

#endif // CEDITORTOOLS_H
