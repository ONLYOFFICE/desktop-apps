
TARGET  = update-daemon
CONFIG  += c++11 console
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
           $$PWD/src/utils.h \
           $$PWD/src/resource.h \
           $$PWD/src/svccontrol.h \
           $$PWD/src/classes/capplication.h \
           $$PWD/src/classes/csocket.h \
           $$PWD/src/classes/cunzip.h \
           $$PWD/src/classes/cupdatemanager.h \
           $$PWD/src/classes/cdownloader.h

SOURCES += $$PWD/src/main.cpp \
           $$PWD/src/utils.cpp \
           $$PWD/src/svccontrol.cpp \
           $$PWD/src/classes/capplication.cpp \
           $$PWD/src/classes/csocket.cpp \
           $$PWD/src/classes/cunzip.cpp \
           $$PWD/src/classes/cupdatemanager.cpp \
           $$PWD/src/classes/cdownloader.cpp

ENV_PRODUCT_VERSION = $$(PRODUCT_VERSION)
!isEmpty(ENV_PRODUCT_VERSION) {
    FULL_PRODUCT_VERSION = $${ENV_PRODUCT_VERSION}.$$(BUILD_NUMBER)
    DEFINES += VER_PRODUCT_VERSION=$$FULL_PRODUCT_VERSION \
               VER_PRODUCT_VERSION_COMMAS=$$replace(FULL_PRODUCT_VERSION, \., ",")
}

RC_FILE = $$PWD/version.rc

CONFIG += embed_manifest_exe
# Uncomment to testing service control
#QMAKE_LFLAGS += /MANIFESTUAC:$$quote(\"level=\'requireAdministrator\' uiAccess=\'false\'\")

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
        -lkernel32 \
        -lshell32 \
        -lshlwapi \
        -lole32 \
        -loleaut32 \
        -lcomsuppw \
        -ladvapi32 \
        -lurlmon \
        -lwininet \
        -lws2_32 \
        -lrpcrt4 \
        -lwtsapi32 \
        -lcrypt32 \
        -lwintrust \
        -luserenv

OBJECTS_DIR = $$DESTDIR/obj
MOC_DIR = $$DESTDIR/moc
RCC_DIR = $$DESTDIR/rcc
