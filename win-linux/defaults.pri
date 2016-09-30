
QT  += core gui widgets gui-private widgets-private core-private printsupport

TEMPLATE = app
CONFIG += app_bundle
CONFIG += c++11

TRANSLATIONS = ./langs/en.ts \
                ./langs/ru.ts \
                ./langs/de.ts \
                ./langs/sp.ts \
                ./langs/fr.ts

CORE_SRC_PATH = ../../core/DesktopEditor
BASEEDITORS_PATH = ../../desktop-sdk/ChromiumBasedEditors
CORE_LIB_PATH = ../../core/build

INCLUDEPATH += $$BASEEDITORS_PATH/lib/include \
                $$BASEEDITORS_PATH/lib/qcefview \
                $$CORE_SRC_PATH

HEADERS += \
    $$BASEEDITORS_PATH/lib/qcefview/qcefview.h \
    $$PWD/src/asctabwidget.h \
    $$PWD/src/version.h \
    $$PWD/src/defines.h \
    $$PWD/src/cdownloadwidget.h \
    $$PWD/src/cpushbutton.h \
    $$PWD/src/cfiledialog.h \
    $$PWD/src/cprintprogress.h \
    $$PWD/src/ccefeventstransformer.h \
    $$PWD/src/cascapplicationmanagerwrapper.h \
    $$PWD/src/ctabbar.h \
    $$PWD/src/casctabdata.h \
    $$PWD/src/utils.h \
    $$PWD/src/cstyletweaks.h \
    $$PWD/src/chelp.h \
    $$PWD/src/cmainpanel.h \
    $$PWD/src/csplash.h \
    $$PWD/src/cmessage.h
#    src/ctabbar_p.h \
#    src/ctabstyle.h \
#    src/ctabstyle_p.h
#    src/casclabel.h

SOURCES += \
    $$BASEEDITORS_PATH/lib/qcefview/qcefview.cpp \
    $$PWD/src/main.cpp \
    $$PWD/src/asctabwidget.cpp\
    $$PWD/src/cdownloadwidget.cpp \
    $$PWD/src/cpushbutton.cpp \
    $$PWD/src/cfiledialog.cpp \
    $$PWD/src/cprintprogress.cpp \
    $$PWD/src/ccefeventstransformer.cpp \
    $$PWD/src/ctabbar.cpp \
    $$PWD/src/casctabdata.cpp \
    $$PWD/src/cstyletweaks.cpp \
    $$PWD/src/chelp.cpp \
    $$PWD/src/cmainpanel.cpp \
    $$PWD/src/cmessage.cpp
#    src/ctabstyle.cpp
#    src/casclabel.cpp

RESOURCES += $$PWD/resources.qrc

linux-g++ {
    contains(QMAKE_HOST.arch, x86_64):{
        PLATFORM_BUILD = linux_64
    } else {
        PLATFORM_BUILD = linux_32
    }

    QT += network x11extras

    QMAKE_LFLAGS += "-Wl,-rpath,\'\$$ORIGIN\'"
    QMAKE_LFLAGS += "-Wl,-rpath,\'\$$ORIGIN/converter\'"
    QMAKE_LFLAGS += -static-libstdc++ -static-libgcc

    LIBS += -L$$PWD/$$CORE_LIB_PATH/cef/$$PLATFORM_BUILD -lcef
    LIBS += -L$$PWD/$$CORE_LIB_PATH/lib/$$PLATFORM_BUILD -lDjVuFile -lXpsFile -lPdfReader -lPdfWriter -lHtmlRenderer

    HEADERS += src/linux/cmainwindow.h \
                src/linux/cx11decoration.h \
                src/linux/singleapplication.h
    SOURCES += src/linux/cmainwindow.cpp \
                src/linux/cx11decoration.cpp \
                src/linux/cx11caption.cpp \
                src/linux/singleapplication.cpp

    DEFINES += LINUX _LINUX
    CONFIG += link_pkgconfig
    PKGCONFIG += glib-2.0 gdk-2.0 gtkglext-1.0 atk cairo gtk+-unix-print-2.0

    build_for_centos6 {
        QMAKE_LFLAGS += -Wl,--dynamic-linker=./ld-linux-x86-64.so.2
    }
}


win32 {
    DEFINES += JAS_WIN_MSVC_BUILD WIN32
    RC_ICONS += ./res/icons/desktop_icons.ico

    HEADERS += $$PWD/src/win/mainwindow.h \
                $$PWD/src/win/qwinwidget.h \
                $$PWD/src/win/qwinhost.h \
                $$PWD/src/win/cwinpanel.h \
                $$PWD/src/win/cwinwindow.h \
                $$PWD/src/win/cprintdialog.h

    SOURCES += $$PWD/src/win/mainwindow.cpp \
                $$PWD/src/win/qwinwidget.cpp \
                $$PWD/src/win/qwinhost.cpp \
                $$PWD/src/win/cwinpanel.cpp \
                $$PWD/src/win/cwinwindow.cpp \
                $$PWD/src/win/cprintdialog.cpp

    LIBS += -lwininet \
            -ldnsapi \
            -lversion \
            -lmsimg32 \
            -lws2_32 \
            -lusp10 \
            -lpsapi \
            -ldbghelp \
            -lwinmm \
            -lshlwapi \
            -lkernel32 \
            -lgdi32 \
            -lwinspool \
            -lcomdlg32 \
            -ladvapi32 \
            -lshell32 \
            -lole32 \
            -loleaut32 \
            -luser32 \
            -luuid \
            -lodbc32 \
            -lodbccp32 \
            -ldelayimp \
            -lcredui \
            -lnetapi32 \
            -lcomctl32 \
            -lrpcrt4
#            -ldwmapi
#            -lOpenGL32

    contains(QMAKE_TARGET.arch, x86_64):{
        QMAKE_LFLAGS_WINDOWS = /SUBSYSTEM:WINDOWS,5.02
        PLATFORM_BUILD = win_64
    } else {
        QMAKE_LFLAGS_WINDOWS = /SUBSYSTEM:WINDOWS,5.01
        PLATFORM_BUILD = win_32
    }

    LIBS += -L$$PWD/$$CORE_LIB_PATH/cef/$$PLATFORM_BUILD -llibcef
}
