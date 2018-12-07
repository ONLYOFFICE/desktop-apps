
QT      += core
QT      -= gui

OBJECTS_DIR = ./obj
MOC_DIR = ./moc

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
} else {
    QMAKE_LFLAGS_WINDOWS = /SUBSYSTEM:WINDOWS,5.01
}
