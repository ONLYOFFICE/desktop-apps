#ifndef CWINDOWMANAGER_H
#define CWINDOWMANAGER_H

#include <vector>

#ifdef _WIN32
#include "win/mainwindow.h"
#else
#include "linux/cmainwindow.h"
#include "linux/singleapplication.h"
#endif


class CWindowManager
{
private:
    CWindowManager();
    ~CWindowManager();

    std::vector<size_t> m_vecWidows;

public:
    static CWindowManager& getInstance();

    static void             startApp();
    static CMainWindow *    createMainWindow(QRect&);
    static void             closeMainWindow(const size_t);
};

#endif // CWINDOWMANAGER_H
