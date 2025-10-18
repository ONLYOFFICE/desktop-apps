
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

#DEFINES += _GLIBCXX_USE_CXX11_ABI=0
DEFINES += __DONT_WRITE_IN_APP_TITLE
DEFINES += APP_ICON_PATH=\"./res/icons/desktopeditors.ico\"

message($$PLATFORM_BUILD)

#win32 {
    updmodule:!build_xp {
        DEFINES += _UPDMODULE
        message(updates is turned on)
    }
#}

core_windows {
    VER_PRODUCTNAME      = ONLYOFFICE
    VER_INTERNALNAME     = Desktop Editors
    VER_FILEDESCRIPTION  = ONLYOFFICE Desktop Editors
    VER_LEGALTRADEMARKS1 = All Rights Reserved
    VER_LEGALTRADEMARKS2 = $$VER_LEGALTRADEMARKS1
    VER_ORIGINALFILENAME = editors.exe
    VER_LANG_ID          = 0x0409
    VER_CHARSET_ID       = 0x04E4
    VER_LANG_AND_CHARSET = 040904E4

    version_info.input = $$PWD/version.rc.in
    version_info.output = $$PWD/version.rc
    version_info.variables = \
        VER_PRODUCT_VERSION_COMMAS \
        VER_LANG_AND_CHARSET \
        VER_COMPANYNAME \
        VER_FILEDESCRIPTION \
        VER_PRODUCT_VERSION \
        VER_INTERNALNAME \
        VER_LEGALCOPYRIGHT \
        VER_LEGALTRADEMARKS1 \
        VER_LEGALTRADEMARKS2 \
        VER_ORIGINALFILENAME \
        VER_PRODUCTNAME \
        VER_LANG_ID \
        VER_CHARSET_ID

    QMAKE_SUBSTITUTES += version_info

    RC_FILE = $$PWD/resource.rc
    OTHER_FILES += \
        $$PWD/resource.rc \
        $$PWD/version.rc \
        $$PWD/version.rc.in
}

core_linux {
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
