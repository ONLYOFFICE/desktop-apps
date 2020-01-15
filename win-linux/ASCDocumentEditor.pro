
TARGET = DesktopEditors

include(defaults.pri)

INCLUDEPATH += $$PWD/src/prop \
                $$PWD/src

HEADERS += \
    src/prop/defines_p.h \
    src/prop/version_p.h

SOURCES += \
    src/prop/csplash.cpp \
    src/prop/cmainpanelimpl.cpp \
    src/prop/cascapplicationmanagerwrapper_private.h \
    src/prop/utils.cpp

RC_FILE = $$PWD/version.rc

DEFINES += __DONT_WRITE_IN_APP_TITLE

message($$PLATFORM_BUILD)

linux-g++ {
    DEFINES += _GLIBCXX_USE_CXX11_ABI=0
    message($$PLATFORM_BUILD)
}

win32 {    
    #CONFIG += updmodule
    updmodule {
        DEFINES += _UPDMODULE
        DEFINES += URL_APPCAST_UPDATES=$$join(LINK,,\\\",\\\")
        LIBS += -L$$PWD/3dparty/WinSparkle/$$PLATFORM_BUILD -lWinSparkle

        HEADERS += $$PWD/3dparty/WinToast/src/wintoastlib.h\
                    $$PWD/src/win/cnotifications.h
        SOURCES += $$PWD/3dparty/WinToast/src/wintoastlib.cpp \
                    $$PWD/src/win/cnotifications.cpp

        message(updates is turned on)
        message(url: $$join(LINK,,\\\",\\\"))
    }
}

HEADERS += \
    src/prop/cmainpanelimpl.h
