
TARGET  = online-installer
CONFIG  += c++11 utf8_source
CONFIG  -= app_bundle
CONFIG  -= qt
CONFIG  -= debug_and_release debug_and_release_target

TEMPLATE = app

CORE_ROOT_DIR = $$PWD/../../../../core
UICLASSES = $$PWD/src/uiclasses

CONFIG += core_no_dst
include($$CORE_ROOT_DIR/Common/base.pri)

INCLUDEPATH += $$PWD/src \
               $$UICLASSES
INCLUDEPATH += $$PWD/../../src/prop

HEADERS += $$PWD/src/version.h \
           $$PWD/src/resource.h \
           $$PWD/src/mainwindow.h \
           $$PWD/src/cdownloader.h \
           $$PWD/src/translator.h \
           $$PWD/src/utils.h \
           $$UICLASSES/commondefines.h \
           $$UICLASSES/baseutils.h \
           $$UICLASSES/common.h \
           $$UICLASSES/metrics.h \
           $$UICLASSES/palette.h \
           $$UICLASSES/drawningengine.h \
           $$UICLASSES/drawingsurface.h \
           $$UICLASSES/object.h \
           $$UICLASSES/application.h \
           $$UICLASSES/window.h \
           $$UICLASSES/widget.h \
           $$UICLASSES/label.h \
           $$UICLASSES/caption.h \
           $$UICLASSES/abstractbutton.h \
           $$UICLASSES/button.h \
           $$UICLASSES/checkbox.h \
           $$UICLASSES/radiobutton.h \
           $$UICLASSES/progressbar.h \
           $$UICLASSES/layoutitem.h \
           $$UICLASSES/layout.h \
           $$UICLASSES/boxlayout.h

SOURCES += $$PWD/src/main.cpp \
           $$PWD/src/mainwindow.cpp \
           $$PWD/src/cdownloader.cpp \
           $$PWD/src/translator.cpp \
           $$PWD/src/utils.cpp \
           $$UICLASSES/baseutils.cpp \
           $$UICLASSES/common.cpp \
           $$UICLASSES/metrics.cpp \
           $$UICLASSES/palette.cpp \
           $$UICLASSES/drawningengine.cpp \
           $$UICLASSES/drawingsurface.cpp \
           $$UICLASSES/object.cpp \
           $$UICLASSES/application.cpp \
           $$UICLASSES/window.cpp \
           $$UICLASSES/widget.cpp \
           $$UICLASSES/label.cpp \
           $$UICLASSES/caption.cpp \
           $$UICLASSES/abstractbutton.cpp \
           $$UICLASSES/button.cpp \
           $$UICLASSES/checkbox.cpp \
           $$UICLASSES/radiobutton.cpp \
           $$UICLASSES/progressbar.cpp \
           $$UICLASSES/layoutitem.cpp \
           $$UICLASSES/layout.cpp \
           $$UICLASSES/boxlayout.cpp

OTHER_FILES += $$PWD/res/version.rc \
               $$PWD/res/manifest/online-installer.exe.manifest

ENV_PRODUCT_VERSION = $$(PRODUCT_VERSION)
!isEmpty(ENV_PRODUCT_VERSION) {
    FULL_PRODUCT_VERSION = $${ENV_PRODUCT_VERSION}.$$(BUILD_NUMBER)
    DEFINES += VER_PRODUCT_VERSION=$$FULL_PRODUCT_VERSION \
               VER_PRODUCT_VERSION_COMMAS=$$replace(FULL_PRODUCT_VERSION, \., ",")
}

CONFIG -= embed_manifest_exe
RC_FILE = $$PWD/res/version.rc
QMAKE_CXXFLAGS += -D_UNICODE

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

DEFINES -= NOMINMAX

LIBS += -luser32 \
        -lshell32 \
        -lshlwapi \
        -lwinhttp \
        -lwintrust \
        -lgdi32 \
        -lgdiplus \
        -ladvapi32 \
        -lrpcrt4 \
        -lole32 \
        -lmsi \
        -lwinmm \
        -lcomctl32

OBJECTS_DIR = $$DESTDIR/obj
MOC_DIR = $$DESTDIR/moc
RCC_DIR = $$DESTDIR/rcc
