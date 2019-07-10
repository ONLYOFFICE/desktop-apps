
QT      += core
QT      -= gui

OBJECTS_DIR = ./obj
MOC_DIR = ./moc
RCC_DIR = ./rcc

TARGET  = projicons
CONFIG  -= console
CONFIG  -= app_bundle
CONFIG  -= debug_and_release debug_and_release_target

TEMPLATE = app

HEADERS += $$PWD/src/version.h
SOURCES += $$PWD/src/main.cpp

RC_FILE = $$PWD/version.rc

contains(QMAKE_TARGET.arch, x86_64):{
    QMAKE_LFLAGS_WINDOWS = /SUBSYSTEM:WINDOWS,5.02
    PLATFORM_BUILD = win_64
} else {
    QMAKE_LFLAGS_WINDOWS = /SUBSYSTEM:WINDOWS,5.01
    PLATFORM_BUILD = win_32
}

TARGET = $$join(TARGET,,,_$$PLATFORM_BUILD)
OBJECTS_DIR = $$join(OBJECTS_DIR,,./$$PLATFORM_BUILD/,)
MOC_DIR = $$join(MOC_DIR,,./$$PLATFORM_BUILD/,)
RCC_DIR = $$join(RCC_DIR,,./$$PLATFORM_BUILD/,)

win32:build_xp {
    TARGET = $$join(TARGET,,,_xp)
    OBJECTS_DIR = $$replace(OBJECTS_DIR, $$PLATFORM_BUILD/,$$PLATFORM_BUILD/xp/)
    MOC_DIR = $$replace(MOC_DIR, $$PLATFORM_BUILD/,$$PLATFORM_BUILD/xp/)
    RCC_DIR = $$replace(RCC_DIR, $$PLATFORM_BUILD/,$$PLATFORM_BUILD/xp/)
}
