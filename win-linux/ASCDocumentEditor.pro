
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
DEFINES += APP_ICON_PATH=\"./res/icons/desktopeditors.ico\"

message($$PLATFORM_BUILD)

#win32 {
    updmodule:!build_xp {
        DEFINES += _UPDMODULE

        ENV_URL_APPCAST_MAIN = $$(DESKTOP_URL_UPDATES_MAIN_CHANNEL)
        !isEmpty(ENV_URL_APPCAST_MAIN) {
            DEFINES += URL_APPCAST_UPDATES=\\\"$${ENV_URL_APPCAST_MAIN}\\\"
        }

        ENV_URL_APPCAST_DEV = $$(DESKTOP_URL_UPDATES_DEV_CHANNEL)
        !isEmpty(ENV_URL_APPCAST_DEV) {
            DEFINES += URL_APPCAST_DEV_CHANNEL=\\\"$${ENV_URL_APPCAST_DEV}\\\"
        }

        message(updates is turned on)
        message(appcast url: $$ENV_URL_APPCAST_MAIN)
        message(appcast dev url: \\\"$$ENV_URL_APPCAST_DEV\\\")
    }
#}

updmodule:core_linux {
    GLIB_RESOURCE_FILES += $$PWD/res/gresource.xml

    glib_resources.name = gresource
    glib_resources.input = GLIB_RESOURCE_FILES
    glib_resources.output = $$PWD/res/${QMAKE_FILE_IN_BASE}.c
    glib_resources.commands = glib-compile-resources --target ${QMAKE_FILE_OUT} --sourcedir ${QMAKE_FILE_IN_PATH} --generate-source ${QMAKE_FILE_IN}
    glib_resources.variable_out = SOURCES
    QMAKE_EXTRA_COMPILERS += glib_resources
}

HEADERS += \
    src/prop/cmainwindowimpl.h
