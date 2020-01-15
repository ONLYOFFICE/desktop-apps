
QT  += core gui widgets gui-private widgets-private core-private printsupport
QT  += multimedia multimediawidgets
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

CORE_ROOT_DIR = $$PWD/../../core
BASEEDITORS_PATH = $$PWD/../../desktop-sdk/ChromiumBasedEditors
CORE_3DPARTY_PATH = $$PWD/../../core/Common/3dParty

CONFIG += core_no_dst
include($$CORE_ROOT_DIR/Common/base.pri)

INCLUDEPATH += \
    $$BASEEDITORS_PATH/lib/include \
    $$BASEEDITORS_PATH/lib/qcefview \
    $$CORE_ROOT_DIR/DesktopEditor

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
    $$PWD/src/cwindowsqueue.h \
    $$PWD/src/ceventdriver.h \
    $$PWD/src/csvgpushbutton.h
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
    $$PWD/src/ceditortools.cpp \
    $$PWD/src/ceventdriver.cpp \
    $$PWD/src/csvgpushbutton.cpp
#    src/ctabstyle.cpp
#    src/casclabel.cpp

RESOURCES += $$PWD/resources.qrc

isEqual(QT_MAJOR_VERSION, 5) {
    lessThan(QT_MINOR_VERSION, 10) {
        DEFINES += _QTVER_DOWNGRADE
        message("QTVER_DOWNGRADE")
    }
}

ENV_PRODUCT_VERSION = $$(PRODUCT_VERSION)
!isEmpty(ENV_PRODUCT_VERSION) {
    FULL_PRODUCT_VERSION = $${ENV_PRODUCT_VERSION}.$$(BUILD_NUMBER)
    DEFINES += VER_PRODUCT_VERSION=$$FULL_PRODUCT_VERSION \
               VER_PRODUCT_VERSION_COMMAS=$$replace(FULL_PRODUCT_VERSION, \., ",")
}

PLATFORM_BUILD=$$CORE_BUILDS_PLATFORM_PREFIX

# cef
core_windows:LIBS += -L$$CORE_3DPARTY_PATH/cef/$$PLATFORM_BUILD/build -llibcef
core_linux:LIBS += -L$$CORE_3DPARTY_PATH/cef/$$PLATFORM_BUILD/build -lcef

# core
ADD_DEPENDENCY(PdfReader, PdfWriter, DjVuFile, XpsFile, HtmlRenderer, UnicodeConverter, hunspell, ooxmlsignature, kernel, graphics, videoplayer, ascdocumentscore)

core_linux {
    QT += network x11extras

    QMAKE_LFLAGS += "-Wl,-rpath,\'\$$ORIGIN\'"
    QMAKE_LFLAGS += "-Wl,-rpath,\'\$$ORIGIN/converter\'"
    QMAKE_LFLAGS += -static-libstdc++ -static-libgcc

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

    CONFIG += link_pkgconfig
    PKGCONFIG += glib-2.0 gdk-2.0 atk cairo gtk+-unix-print-2.0
    LIBS += -lX11

    LIBS += $$CORE_3DPARTY_PATH/icu/$$PLATFORM_BUILD/build/libicuuc.so.58
    LIBS += $$CORE_3DPARTY_PATH/icu/$$PLATFORM_BUILD/build/libicudata.so.58

    DEFINES += DOCUMENTSCORE_OPENSSL_SUPPORT
}

core_windows {
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

DESTDIR = $$PWD
TARGET = $$join(TARGET,,,_$$PLATFORM_BUILD)

ADDITIONAL_BUILD = $$PLATFORM_BUILD
!isEmpty(OO_BUILD_BRANDING) {
    DESTDIR = $$DESTDIR/$$OO_BUILD_BRANDING
    ADDITIONAL_BUILD = $$ADDITIONAL_BUILD/OO_BUILD_BRANDING
}
build_xp {
    TARGET = $$join(TARGET,,,_xp)
    DESTDIR = $$DESTDIR/xp
    ADDITIONAL_BUILD = $$ADDITIONAL_BUILD/xp
}

OBJECTS_DIR = ./obj/$$ADDITIONAL_BUILD
MOC_DIR = ./moc/$$ADDITIONAL_BUILD
RCC_DIR = ./rcc/$$ADDITIONAL_BUILD
