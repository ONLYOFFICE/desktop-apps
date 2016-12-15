
TARGET = DesktopEditors

include(defaults.pri)

INCLUDEPATH += $$PWD/src/prop \
                $$PWD/src

HEADERS += \
    src/prop/defines_p.h

SOURCES += \
    src/csplash.cpp \
    src/prop/ccefeventsimpl.cpp \
    src/prop/cmainpanelimpl.cpp \
    src/prop/utils.cpp

RC_FILE = $$PWD/version.rc

linux-g++ {
    LIBS += -L$$PWD/$$CORE_LIB_PATH/lib/$$PLATFORM_BUILD -lascdocumentscore
    DEFINES += LINUX _LINUX _LINUX_QT _GLIBCXX_USE_CXX11_ABI=0

    message($$PLATFORM_BUILD)
}

win32 {
#    CONFIG += updmodule
    updmodule {
        DEFINES += _UPDMODULE
        DEFINES += URL_APPCAST_UPDATES=$$join(LINK,,\\\",\\\")
        LIBS += -L$$PWD/3dparty/WinSparkle/$$PLATFORM_BUILD -lWinSparkle
    }

    CONFIG(debug, debug|release) {
        LIBS += -L$$PWD/$$CORE_LIB_PATH/lib/$$PLATFORM_BUILD/debug -lascdocumentscore
        LIBS += -L$$PWD/$$CORE_LIB_PATH/lib/$$PLATFORM_BUILD/debug
    } else {
        LIBS += -L$$PWD/$$CORE_LIB_PATH/lib/$$PLATFORM_BUILD -lascdocumentscore
    }

    message($$PLATFORM_BUILD)
}

HEADERS += \
    src/prop/cmainpanelimpl.h
