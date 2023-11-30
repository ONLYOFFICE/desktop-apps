
DESTDIR = $$PWD
include(common.pri)

DEFINES += COPYRIGHT_YEAR=$${CURRENT_YEAR}
DEFINES += APP_ICON_PATH=\"./icons/desktopeditors.ico\"
DEFINES += APP_LANG_PATH=\"./langs/langs.iss\"

core_linux {
    SOURCES += $$PWD/res/gresource.c
}

OTHER_FILES += $$PWD/res/langs/langs.iss
