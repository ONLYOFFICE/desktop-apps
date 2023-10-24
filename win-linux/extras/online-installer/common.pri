
TARGET  = online-installer
CONFIG  += c++11 utf8_source
CONFIG  -= app_bundle
CONFIG  -= qt
CONFIG  -= debug_and_release debug_and_release_target

TEMPLATE = app

CORE_ROOT_DIR = $$PWD/../../../../core

CONFIG += core_no_dst
include($$CORE_ROOT_DIR/Common/base.pri)

INCLUDEPATH += $$PWD/src
INCLUDEPATH += $$PWD/../../src/prop

HEADERS += $$PWD/src/version.h \
           $$PWD/src/resource.h \
           $$PWD/src/cdownloader.h \
           $$PWD/src/translator.h \
           $$PWD/src/utils.h

SOURCES += $$PWD/src/main.cpp \
           $$PWD/src/cdownloader.cpp \
           $$PWD/src/translator.cpp \
           $$PWD/src/utils.cpp

OTHER_FILES += $$PWD/res/dialog.rc \
               $$PWD/res/manifest/online-installer.exe.manifest

ENV_PRODUCT_VERSION = $$(PRODUCT_VERSION)
!isEmpty(ENV_PRODUCT_VERSION) {
    FULL_PRODUCT_VERSION = $${ENV_PRODUCT_VERSION}.$$(BUILD_NUMBER)
    DEFINES += VER_PRODUCT_VERSION=$$FULL_PRODUCT_VERSION \
               VER_PRODUCT_VERSION_COMMAS=$$replace(FULL_PRODUCT_VERSION, \., ",")
}

CONFIG -= embed_manifest_exe
RC_FILE = $$PWD/res/dialog.rc

contains(QMAKE_TARGET.arch, x86_64):{
    QMAKE_LFLAGS_WINDOWS = /SUBSYSTEM:WINDOWS,5.02
} else {
    QMAKE_LFLAGS_WINDOWS = /SUBSYSTEM:WINDOWS,5.01
}

core_release:DESTDIR = $$DESTDIR/build
core_debug:DESTDIR = $$DESTDIR/build/debug

!isEmpty(OO_BUILD_BRANDING) {
    DESTDIR = $$DESTDIR/$$OO_BUILD_BRANDING
}

DESTDIR = $$DESTDIR/$$CORE_BUILDS_PLATFORM_PREFIX

build_xp {
    DESTDIR = $$DESTDIR/xp
    DEFINES += __OS_WIN_XP
}

LIBS += -luser32 \
        -lshell32 \
        -lwinhttp \
        -lwintrust \
        -lcomctl32

OBJECTS_DIR = $$DESTDIR/obj
MOC_DIR = $$DESTDIR/moc
RCC_DIR = $$DESTDIR/rcc
