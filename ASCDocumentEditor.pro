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

INCLUDEPATH += ./include \
        ./libs/lib/include

HEADERS += \
    ./src/mainwindow.h \
    ./src/qmainpanel.h \
    ./src/qwinhost.h \
    ./src/qwinwidget.h \
    ./libs/lib/qcefview/qcefview.h \
    ./src/asctabwidget.h \
    src/cascuser.h \
    src/version.h \
    src/csavefilemessage.h \
    src/cuserprofilewidget.h \
    src/defines.h \
    src/cdownloadwidget.h \
    src/cpushbutton.h \
    src/cprintdialog.h \
    src/cmyapplicationmanager.h \
    src/cfiledialog.h \
    src/cprintprogress.h
#    src/casclabel.h

SOURCES += \
    ./src/main.cpp \
    ./src/asctabwidget.cpp\
    ./src/mainwindow.cpp \
    ./src/qmainpanel.cpp \
    ./src/qwinhost.cpp \
    ./src/qwinwidget.cpp \
    ./libs/lib/qcefview/qcefview.cpp \
    src/cascuser.cpp \
    src/csavefilemessage.cpp \
    src/cuserprofilewidget.cpp \
    src/cdownloadwidget.cpp \
    src/cpushbutton.cpp \
    src/cprintdialog.cpp \
    src/cfiledialog.cpp \
    src/cprintprogress.cpp
#    src/casclabel.cpp

RESOURCES += resources.qrc

RC_FILE = version.rc

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

win32 {
    DEFINES += JAS_WIN_MSVC_BUILD WIN32
    RC_ICONS += ./res/icons/desktop_icons.ico

    contains(QMAKE_TARGET.arch, x86_64):{
        QMAKE_LFLAGS_WINDOWS = /SUBSYSTEM:WINDOWS,5.02
        LIBS += -L$$PWD/libs/app/cefbuilds/win64 -llibcef

        CONFIG(debug, debug|release) {
            LIBS += -L$$PWD/libs/app/corebuilds/win64/debug -lascdocumentscore
        } else {
            LIBS += -L$$PWD/libs/app/corebuilds/win64 -lascdocumentscore
        }

        message(windows64)
    }

    !contains(QMAKE_TARGET.arch, x86_64):{
        QMAKE_LFLAGS_WINDOWS = /SUBSYSTEM:WINDOWS,5.01
        LIBS += -L$$PWD/libs/app/cefbuilds/win32 -llibcef

        CONFIG(debug, debug|release) {
            LIBS += -L$$PWD/libs/app/corebuilds/win32/debug -lascdocumentscore
        } else {
            LIBS += -L$$PWD/libs/app/corebuilds/win32 -lascdocumentscore
        }

        message(windows32)
    }
}
