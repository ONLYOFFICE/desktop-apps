
QT  += core gui widgets gui-private widgets-private core-private printsupport
QT  += svg

TEMPLATE = app
CONFIG += app_bundle
CONFIG += c++11

TRANSLATIONS = ./langs/en.ts \
                ./langs/ru.ts \
                ./langs/de.ts \
                ./langs/es.ts \
                ./langs/cs.ts \
                ./langs/sk.ts \
                ./langs/fr.ts \
                ./langs/pt_BR.ts \
                ./langs/it_IT.ts \
                ./langs/zh_CN.ts \
                ./langs/pl.ts

CORE_SRC_PATH = ../../core/DesktopEditor
BASEEDITORS_PATH = ../../desktop-sdk/ChromiumBasedEditors
CORE_LIB_PATH = ../../core/build
CORE_3DPARTY_PATH = ../../core/Common/3dParty

OBJECTS_DIR = ./obj
MOC_DIR = ./moc
RCC_DIR = ./rcc

INCLUDEPATH += $$BASEEDITORS_PATH/lib/include \
                $$BASEEDITORS_PATH/lib/qcefview \
                $$CORE_SRC_PATH

HEADERS += \
    $$BASEEDITORS_PATH/lib/qcefview/qcefview.h \
    $$BASEEDITORS_PATH/lib/qcefview/qcefview_media.h \
    $$PWD/src/asctabwidget.h \
    $$PWD/src/version.h \
    $$PWD/src/defines.h \
    $$PWD/src/cdownloadwidget.h \
    $$PWD/src/cpushbutton.h \
    $$PWD/src/cfiledialog.h \
    $$PWD/src/cprintprogress.h \
    $$PWD/src/ccefeventstransformer.h \
    $$PWD/src/cascapplicationmanagerwrapper.h \
    $$PWD/src/ctabbar.h \
    $$PWD/src/casctabdata.h \
    $$PWD/src/utils.h \
    $$PWD/src/cstyletweaks.h \
    $$PWD/src/chelp.h \
    $$PWD/src/cmainpanel.h \
    $$PWD/src/csplash.h \
    $$PWD/src/cmessage.h \
    $$PWD/src/cfilechecker.h \
    $$PWD/src/clogger.h \
    $$PWD/src/clangater.h \
    $$PWD/src/cwindowbase.h \
    $$PWD/src/canimatedicon.h \
    $$PWD/src/cscalingwrapper.h \
    $$PWD/src/ctabundockevent.h \
    $$PWD/src/cmainwindowbase.h \
    $$PWD/src/ctabpanel.h \
    $$PWD/src/cdpichecker.h \
    $$PWD/src/csinglewindowbase.h \
    $$PWD/src/ceditorwindow.h \
    $$PWD/src/ccefeventsgate.h \
    $$PWD/src/ceditorwindow_p.h \
    $$PWD/src/ceditortools.h \
    $$PWD/src/cwindowsqueue.h
#    src/ctabbar_p.h \
#    src/ctabstyle.h \
#    src/ctabstyle_p.h
#    src/casclabel.h

SOURCES += \
    $$BASEEDITORS_PATH/lib/qcefview/qcefview.cpp \
    $$BASEEDITORS_PATH/lib/qcefview/qcefview_media.cpp \
    $$PWD/src/main.cpp \
    $$PWD/src/asctabwidget.cpp\
    $$PWD/src/cdownloadwidget.cpp \
    $$PWD/src/cpushbutton.cpp \
    $$PWD/src/cfiledialog.cpp \
    $$PWD/src/cprintprogress.cpp \
    $$PWD/src/ccefeventstransformer.cpp \
    $$PWD/src/cascapplicationmanagerwrapper.cpp \
    $$PWD/src/ctabbar.cpp \
    $$PWD/src/casctabdata.cpp \
    $$PWD/src/cstyletweaks.cpp \
    $$PWD/src/chelp.cpp \
    $$PWD/src/cmainpanel.cpp \
    $$PWD/src/cmessage.cpp \
    $$PWD/src/cfilechecker.cpp \
    $$PWD/src/clogger.cpp \
    $$PWD/src/clangater.cpp \
    $$PWD/src/canimatedicon.cpp \
    $$PWD/src/cscalingwrapper.cpp \
    $$PWD/src/ctabundockevent.cpp \
    $$PWD/src/cmainwindowbase.cpp \
    $$PWD/src/ctabpanel.cpp \
    $$PWD/src/csinglewindowbase.cpp \
    $$PWD/src/ceditorwindow.cpp \
    $$PWD/src/ccefeventsgate.cpp \
    $$PWD/src/ceditortools.cpp
#    src/ctabstyle.cpp
#    src/casclabel.cpp

RESOURCES += $$PWD/resources.qrc

isEqual(QT_MAJOR_VERSION, 5) {
    lessThan(QT_MINOR_VERSION, 10) {
        DEFINES += _QTVER_DOWNGRADE
        message("QTVER_DOWNGRADE")
    }
}

ENV_BUILD_NUMBER = $$(BUILD_NUMBER)
!isEmpty(ENV_BUILD_NUMBER) {
    DEFINES += VER_NUM_REVISION=$$ENV_BUILD_NUMBER
}

linux-g++ {
    CONFIG += app_linux
	linux-g++:contains(QMAKE_HOST.arch, x86_64): {
		CONFIG += app_linux_64
		PLATFORM_BUILD = linux_64
	}
	linux-g++:!contains(QMAKE_HOST.arch, x86_64): {
		CONFIG += app_linux_32
		PLATFORM_BUILD = linux_32
	}
}

linux-g++-64 {
    CONFIG += app_linux
    CONFIG += app_linux_64
    PLATFORM_BUILD = linux_64
}
linux-g++-32 {
    CONFIG += app_linux
    CONFIG += app_linux_32
    PLATFORM_BUILD = linux_32
}

win32 {
    CONFIG -= debug_and_release debug_and_release_target

    contains(QMAKE_TARGET.arch, x86_64):{
        QMAKE_LFLAGS_WINDOWS = /SUBSYSTEM:WINDOWS,5.02
        PLATFORM_BUILD = win_64
    } else {
        QMAKE_LFLAGS_WINDOWS = /SUBSYSTEM:WINDOWS,5.01
        PLATFORM_BUILD = win_32
    }
}

CORE_LIB_PATH_PLATFORM=$$PWD/$$CORE_LIB_PATH/lib/$$PLATFORM_BUILD

win32 {
    CONFIG(debug, debug|release) {
        CORE_LIB_PATH_PLATFORM=$$CORE_LIB_PATH_PLATFORM/DEBUG
        LIBS += -L$$PWD/$$CORE_3DPARTY_PATH/cef/$$PLATFORM_BUILD/build
    }
}

LIBS += -L$$CORE_LIB_PATH_PLATFORM -lPdfReader -lPdfWriter -lDjVuFile -lXpsFile -lHtmlRenderer -lUnicodeConverter -lhunspell -looxmlsignature -lkernel -lgraphics

INCLUDEPATH += ../../core-ext/desktop-sdk-wrapper/additional
QT += multimedia multimediawidgets
build_xp {
    LIBS += -L$$CORE_LIB_PATH_PLATFORM/xp -lvideoplayer
} else {
    LIBS += -L$$CORE_LIB_PATH_PLATFORM -lvideoplayer
}

app_linux {
    QT += network x11extras

    QMAKE_LFLAGS += "-Wl,-rpath,\'\$$ORIGIN\'"
    QMAKE_LFLAGS += "-Wl,-rpath,\'\$$ORIGIN/converter\'"
    QMAKE_LFLAGS += -static-libstdc++ -static-libgcc

    LIBS += -L$$PWD/$$CORE_3DPARTY_PATH/cef/$$PLATFORM_BUILD/build -lcef

    HEADERS += $$PWD/src/linux/cmainwindow.h \
                $$PWD/src/linux/cx11decoration.h \
                $$PWD/src/linux/csinglewindow.h \
                $$PWD/src/linux/csinglewindowplatform.h \
                $$PWD/src/linux/singleapplication.h
    SOURCES += $$PWD/src/linux/cmainwindow.cpp \
                $$PWD/src/linux/cx11decoration.cpp \
                $$PWD/src/linux/cx11caption.cpp \
                $$PWD/src/linux/csinglewindow.cpp \
                $$PWD/src/linux/csinglewindowplatform.cpp \
                $$PWD/src/linux/singleapplication.cpp

    HEADERS += $$PWD/src/linux/cdialogopenssl.h
    SOURCES += $$PWD/src/linux/cdialogopenssl.cpp

    DEFINES += LINUX _LINUX
    CONFIG += link_pkgconfig
    PKGCONFIG += glib-2.0 gdk-2.0 atk cairo gtk+-unix-print-2.0
    LIBS += -lX11

    LIBS += -L$$PWD/$$CORE_3DPARTY_PATH/cef/$$PLATFORM_BUILD/build -lcef
    LIBS += $$PWD/$$CORE_3DPARTY_PATH/icu/$$PLATFORM_BUILD/build/libicuuc.so.58
    LIBS += $$PWD/$$CORE_3DPARTY_PATH/icu/$$PLATFORM_BUILD/build/libicudata.so.58

    DEFINES += DOCUMENTSCORE_OPENSSL_SUPPORT
}


win32 {
    DEFINES += JAS_WIN_MSVC_BUILD WIN32
    DEFINES += WIN32
    DEFINES += Q_COMPILER_INITIALIZER_LISTS

    RC_ICONS += ./res/icons/desktop_icons.ico

    HEADERS += $$PWD/src/win/mainwindow.h \
                $$PWD/src/win/qwinwidget.h \
                $$PWD/src/win/qwinhost.h \
                $$PWD/src/win/cwinpanel.h \
                $$PWD/src/win/cwinwindow.h \
                $$PWD/src/win/csinglewindow.h \
                $$PWD/src/win/csinglewindowplatform.h \
                $$PWD/src/win/cprintdialog.h

    SOURCES += $$PWD/src/win/mainwindow.cpp \
                $$PWD/src/win/qwinwidget.cpp \
                $$PWD/src/win/qwinhost.cpp \
                $$PWD/src/win/cwinpanel.cpp \
                $$PWD/src/win/cwinwindow.cpp \
                $$PWD/src/win/csinglewindow.cpp \
                $$PWD/src/win/csinglewindowplatform.cpp \
                $$PWD/src/win/cprintdialog.cpp

    LIBS += -lwininet \
            -ldnsapi \
            -lversion \
            -lmsimg32 \
            -lws2_32 \
            -lusp10 \
            -lpsapi \
            -ldbghelp \
            -lwinmm \
            -lshlwapi \
            -lkernel32 \
            -lgdi32 \
            -lwinspool \
            -lcomdlg32 \
            -ladvapi32 \
            -lshell32 \
            -lole32 \
            -loleaut32 \
            -luser32 \
            -luuid \
            -lodbc32 \
            -lodbccp32 \
            -ldelayimp \
            -lcredui \
            -lnetapi32 \
            -lcomctl32 \
            -lrpcrt4
#            -ldwmapi
#            -lOpenGL32
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
