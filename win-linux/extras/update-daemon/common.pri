
TARGET  = updatesvc
CONFIG  += c++11 console utf8_source
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
           $$PWD/src/classes/csocket.h \
           $$PWD/src/classes/csvcmanager.h \
           $$PWD/src/classes/cjson_p.h \
           $$PWD/src/classes/cjson.h \
           $$PWD/src/classes/translator.h

SOURCES += $$PWD/src/classes/csocket.cpp \
           $$PWD/src/classes/csvcmanager.cpp \
           $$PWD/src/classes/cjson.cpp \
           $$PWD/src/classes/translator.cpp

ENV_PRODUCT_VERSION = $$(PRODUCT_VERSION)
!isEmpty(ENV_PRODUCT_VERSION) {
    FULL_PRODUCT_VERSION = $${ENV_PRODUCT_VERSION}.$$(BUILD_NUMBER)
    DEFINES += VER_PRODUCT_VERSION=$$FULL_PRODUCT_VERSION \
               VER_PRODUCT_VERSION_COMMAS=$$replace(FULL_PRODUCT_VERSION, \., ",")
}

core_windows {
    CONFIG -= embed_manifest_exe
    RC_FILE = $$PWD/res/version.rc

    contains(QMAKE_TARGET.arch, x86_64):{
        QMAKE_LFLAGS_WINDOWS = /SUBSYSTEM:WINDOWS,5.02
    } else {
        QMAKE_LFLAGS_WINDOWS = /SUBSYSTEM:WINDOWS,5.01
    }
}

core_release:DESTDIR = $$DESTDIR/build
core_debug:DESTDIR = $$DESTDIR/build/debug

!isEmpty(OO_BUILD_BRANDING) {
    DESTDIR = $$DESTDIR/$$OO_BUILD_BRANDING
}

DESTDIR = $$DESTDIR/$$CORE_BUILDS_PLATFORM_PREFIX

core_windows {
    ZLIB_DIR = $$CORE_ROOT_DIR/OfficeUtils/src/zlib-1.2.11
    MINIZIP_DIR = $$ZLIB_DIR/contrib/minizip
    INCLUDEPATH += $$ZLIB_DIR \
                   $$ZLIB_DIR/../../src \
                   $$MINIZIP_DIR

    HEADERS += $$PWD/src/platform_win/utils.h \
               $$PWD/src/platform_win/resource.h \
               $$PWD/src/platform_win/svccontrol.h \
               $$PWD/src/classes/platform_win/capplication.h \
               $$PWD/src/classes/platform_win/cunzip.h \
               $$PWD/src/classes/platform_win/cdownloader.h \
               $$PWD/src/classes/platform_win/ctimer.h

    SOURCES += $$PWD/src/platform_win/main.cpp \
               $$PWD/src/platform_win/utils.cpp \
               $$PWD/src/platform_win/svccontrol.cpp \
               $$PWD/src/classes/platform_win/capplication.cpp \
               $$PWD/src/classes/platform_win/cunzip.cpp \
               $$PWD/src/classes/platform_win/cdownloader.cpp \
               $$PWD/src/classes/platform_win/ctimer.cpp

    SOURCES += $$ZLIB_DIR/../../src/zlib_addon.c \
               $$ZLIB_DIR/adler32.c \
               $$ZLIB_DIR/crc32.c \
               $$ZLIB_DIR/inffast.c \
               $$ZLIB_DIR/inflate.c \
               $$ZLIB_DIR/inftrees.c \
               $$ZLIB_DIR/zutil.c \
               $$MINIZIP_DIR/ioapi.c \
               $$MINIZIP_DIR/iowin32.c \
               $$MINIZIP_DIR/unzip.c

    HEADERS += $$ZLIB_DIR/../../src/zlib_addon.h \
               $$ZLIB_DIR/crc32.h \
               $$ZLIB_DIR/inffast.h \
               $$ZLIB_DIR/inflate.h \
               $$ZLIB_DIR/inftrees.h \
               $$ZLIB_DIR/zutil.h \
               $$MINIZIP_DIR/ioapi.h \
               $$MINIZIP_DIR/iowin32.h \
               $$MINIZIP_DIR/unzip.h

    OTHER_FILES += $$PWD/res/version.rc \
                   $$PWD/res/manifest/updatesvc.exe.manifest

    build_xp {
        DESTDIR = $$DESTDIR/xp
        DEFINES += __OS_WIN_XP
    }

    LIBS += -luser32 \
            -lshell32 \
            -lshlwapi \
            -ladvapi32 \
            -lwinhttp \
            -lws2_32 \
            -lole32 \
            -lrpcrt4 \
            -lwtsapi32 \
            -lwintrust \
            -luserenv
}

core_linux {
    HEADERS += $$PWD/src/platform_linux/utils.h \
               $$PWD/src/classes/platform_linux/capplication.h \
               $$PWD/src/classes/platform_linux/cunzip.h \
               $$PWD/src/classes/platform_linux/cdownloader.h \
               $$PWD/src/classes/platform_linux/ctimer.h

    SOURCES += $$PWD/src/platform_linux/main.cpp \
               $$PWD/src/platform_linux/utils.cpp \
               $$PWD/src/classes/platform_linux/capplication.cpp \
               $$PWD/src/classes/platform_linux/cunzip.cpp \
               $$PWD/src/classes/platform_linux/cdownloader.cpp \
               $$PWD/src/classes/platform_linux/ctimer.cpp

    CONFIG += link_pkgconfig
    PKGCONFIG += gtk+-3.0
    LIBS += -lSDL2 -lcurl -luuid -larchive -lpthread -lcrypto
}

OBJECTS_DIR = $$DESTDIR/obj
MOC_DIR = $$DESTDIR/moc
RCC_DIR = $$DESTDIR/rcc
