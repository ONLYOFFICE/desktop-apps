
QT  += core gui widgets gui-private widgets-private core-private printsupport

TEMPLATE = app
CONFIG += app_bundle
CONFIG += c++11

TARGET = DesktopEditors

TRANSLATIONS = ./langs/en.ts \
                ./langs/ru.ts \
                ./langs/de.ts \
                ./langs/es.ts \
                ./langs/fr.ts

CHROMIUM_LIB_PATH = ../common/libs/ChromiumBasedEditors2
COMMON_LIB_PATH = ../common/converter/linux

INCLUDEPATH += $$CHROMIUM_LIB_PATH/lib/include \
                $$CHROMIUM_LIB_PATH/lib/qcefview

HEADERS += \
    $$CHROMIUM_LIB_PATH/lib/qcefview/qcefview.h \
    ./src/asctabwidget.h \
    src/cascuser.h \
    src/version.h \
    src/csavefilemessage.h \
    src/cuserprofilewidget.h \
    src/defines.h \
    src/cdownloadwidget.h \
    src/cpushbutton.h \
#    src/cmyapplicationmanager.h \
    src/cfiledialog.h \
    src/cprintprogress.h \
    src/ccefeventstransformer.h \
    src/qascmainpanel.h \
    src/cascapplicationmanagerwrapper.h \
    src/cchooselicensedialog.h \
    src/ctabbar.h \
    src/casctabdata.h \
    src/cmessage.h \
    src/utils.h
#    src/ctabbar_p.h \
#    src/ctabstyle.h \
#    src/ctabstyle_p.h
#    src/casclabel.h

SOURCES += \
    ./src/main.cpp \
    ./src/asctabwidget.cpp\
    $$CHROMIUM_LIB_PATH/lib/qcefview/qcefview.cpp \
    src/cascuser.cpp \
    src/csavefilemessage.cpp \
    src/cuserprofilewidget.cpp \
    src/cdownloadwidget.cpp \
    src/cpushbutton.cpp \
    src/cfiledialog.cpp \
    src/cprintprogress.cpp \
    src/ccefeventstransformer.cpp \
    src/cchooselicensedialog.cpp \
    src/qascmainpanel.cpp \
    src/ctabbar.cpp \
    src/casctabdata.cpp \
    src/cmessage.cpp
#    src/ctabstyle.cpp
#    src/casclabel.cpp

RESOURCES += resources.qrc
RC_FILE = version.rc
#RES_FILE = version.res

DEFINES += \
    _QT \
    FT2_BUILD_LIBRARY \
    EXCLUDE_JPG_SUPPORT \
    MNG_SUPPORT_DISPLAY \
    MNG_SUPPORT_READ \
    MNG_SUPPORT_WRITE \
    MNG_ACCESS_CHUNKS \
    MNG_STORE_CHUNKS\
    MNG_ERROR_TELLTALE

#DEFINES += _IVOLGA_PRO

linux-g++ {
    contains(QMAKE_HOST.arch, x86_64):{
        PLATFORM_BUILD = linux64
    } else {
        PLATFORM_BUILD = linux32
    }

    QMAKE_LFLAGS += "-Wl,-rpath,\'\$$ORIGIN\'"
    QMAKE_LFLAGS += "-Wl,-rpath,\'\$$ORIGIN/converter\'"
    QMAKE_LFLAGS += -static-libstdc++ -static-libgcc

    LIBS += -L$$PWD/$$CHROMIUM_LIB_PATH/app/cefbuilds/$$PLATFORM_BUILD -lcef
    LIBS += -L$$PWD/$$CHROMIUM_LIB_PATH/app/corebuilds/$$PLATFORM_BUILD -lascdocumentscore
    LIBS += -L$$PWD/$$COMMON_LIB_PATH -lDjVuFile -lXpsFile -lPdfReader -lPdfWriter

    HEADERS += src/linux/cmainwindow.h
    SOURCES += src/linux/cmainwindow.cpp

    DEFINES += LINUX _LINUX _LINUX_QT
    CONFIG += link_pkgconfig
    PKGCONFIG += glib-2.0 gdk-2.0 gtkglext-1.0 atk cairo gtk+-unix-print-2.0

    build_for_centos6 {
        QMAKE_LFLAGS += -Wl,--dynamic-linker=./ld-linux-x86-64.so.2
    }

    message($$PLATFORM_BUILD)
}


win32 {
    DEFINES += JAS_WIN_MSVC_BUILD WIN32
    RC_ICONS += ./res/icons/desktop_icons.ico

    HEADERS += ./src/win/mainwindow.h \
                ./src/win/qwinwidget.h \
                ./src/win/qwinhost.h \
                ./src/win/cwinpanel.h \
                ./src/win/cprintdialog.h

    SOURCES += ./src/win/mainwindow.cpp \
                ./src/win/qwinwidget.cpp \
                ./src/win/qwinhost.cpp \
                ./src/win/cwinpanel.cpp \
                ./src/cprintdialog.cpp

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

#    PRE_TARGETDEPS += ../common/converter/windows/x64/DjVuFile.dll \
#                        ../common/converter/windows/x64/PdfReader.dll \
#                        ../common/converter/windows/x64/PdfWriter.dll \
#                        ../common/converter/windows/x64/XpsFile.dll

    contains(QMAKE_TARGET.arch, x86_64):{
        QMAKE_LFLAGS_WINDOWS = /SUBSYSTEM:WINDOWS,5.02
        PLATFORM_BUILD = win64
    } else {
        QMAKE_LFLAGS_WINDOWS = /SUBSYSTEM:WINDOWS,5.01
        PLATFORM_BUILD = win32
    }

    LIBS += -L$$PWD/$$CHROMIUM_LIB_PATH/app/cefbuilds/$$PLATFORM_BUILD -llibcef
    CONFIG(debug, debug|release) {
        LIBS += -L$$PWD/$$CHROMIUM_LIB_PATH/app/corebuilds/$$PLATFORM_BUILD/debug -lascdocumentscore
    } else {
        LIBS += -L$$PWD/$$CHROMIUM_LIB_PATH/app/corebuilds/$$PLATFORM_BUILD -lascdocumentscore
    }

    message($$PLATFORM_BUILD)
}
