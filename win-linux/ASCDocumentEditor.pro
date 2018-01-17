
TARGET = DesktopEditors

#CONFIG += build_for_centos6
#CONFIG += core_build_deploy

include(defaults.pri)

core_build_deploy {
    build_for_centos6 {
        DESTDIR=$$PWD/../../core/build/linux_desktop/app/$$PLATFORM_BUILD/CentOS6
    } else {
        DESTDIR=$$PWD/../../core/build/linux_desktop/app/$$PLATFORM_BUILD
    }
}

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

linux-g++ {
    LIBS += -L$$PWD/$$CORE_LIB_PATH/lib/$$PLATFORM_BUILD -lascdocumentscore -lhunspell -looxmlsignature
    DEFINES += LINUX _LINUX _LINUX_QT _GLIBCXX_USE_CXX11_ABI=0

    message($$PLATFORM_BUILD)
}

win32 {
#    CONFIG += updmodule
    updmodule {
        DEFINES += _UPDMODULE
        DEFINES += URL_APPCAST_UPDATES=$$join(LINK,,\\\",\\\")
        LIBS += -L$$PWD/3dparty/WinSparkle/$$PLATFORM_BUILD -lWinSparkle

        message(updates is turned on)
        message(url: $$join(LINK,,\\\",\\\"))
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
