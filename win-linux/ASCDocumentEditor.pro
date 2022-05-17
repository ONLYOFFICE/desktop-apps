
TARGET = DesktopEditors
DESTDIR = $$PWD

include(defaults.pri)

INCLUDEPATH += $$PWD/src/prop \
                $$PWD/src

HEADERS += \
    #src/prop/csplash_p.h \
    src/prop/defines_p.h \
    src/prop/cascapplicationmanagerwrapperintf.h \
    src/prop/version_p.h

SOURCES += \
    src/prop/cmainwindowimpl.cpp \
    src/prop/utils.cpp

RC_FILE = $$PWD/version.rc
#DEFINES += _GLIBCXX_USE_CXX11_ABI=0
DEFINES += __DONT_WRITE_IN_APP_TITLE

message($$PLATFORM_BUILD)

win32 {
    #CONFIG += updmodule
    updmodule {
        DEFINES += _UPDMODULE
        DEFINES += URL_APPCAST_UPDATES=$$join(LINK,,\\\",\\\")
        LIBS += -L$$PWD/3dparty/WinSparkle/$$PLATFORM_BUILD -lWinSparkle

        message(updates is turned on)
        message(url: $$join(LINK,,\\\",\\\"))
    }
}

HEADERS += \
    src/prop/cmainwindowimpl.h
