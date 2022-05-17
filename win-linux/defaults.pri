
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
                ./langs/pl.ts \
                ./langs/be.ts \
                ./langs/bg.ts \
                ./langs/ca.ts \
                ./langs/da.ts \
                ./langs/el_GR.ts \
                ./langs/et.ts \
                ./langs/fi.ts \
                ./langs/ga.ts \
                ./langs/gl.ts \
                ./langs/hi.ts \
                ./langs/hr.ts \
                ./langs/hu.ts \
                ./langs/hy.ts \
                ./langs/id.ts \
                ./langs/ja.ts \
                ./langs/ko.ts \
                ./langs/lo.ts \
                ./langs/lt.ts \
                ./langs/lv.ts \
                ./langs/nl.ts \
                ./langs/no.ts \
                ./langs/pt_PT.ts \
                ./langs/ro.ts \
                ./langs/sl.ts \
                ./langs/sv.ts \
                ./langs/tr.ts \
                ./langs/uk.ts \
                ./langs/vi.ts \
                ./langs/be.ts \
                ./langs/zh_HK.ts


CORE_ROOT_DIR = $$PWD/../../core
BASEEDITORS_PATH = $$PWD/../../desktop-sdk/ChromiumBasedEditors
CORE_3DPARTY_PATH = $$PWD/../../core/Common/3dParty

CONFIG += core_no_dst
include($$CORE_ROOT_DIR/Common/base.pri)

INCLUDEPATH += \
    $$BASEEDITORS_PATH/lib/include \
    $$BASEEDITORS_PATH/lib/qt_wrapper/include \
    $$CORE_ROOT_DIR/DesktopEditor \
    $$CORE_ROOT_DIR/Common

HEADERS += \
    $$PWD/src/windows/cmainwindow.h \
    $$PWD/src/windows/cwindowbase.h \
    $$PWD/src/windows/ceditorwindow.h \
    $$PWD/src/windows/ceditorwindow_p.h \
    $$PWD/src/windows/cpresenterwindow.h \
    $$PWD/src/components/asctabwidget.h \
    $$PWD/src/components/ctabbarwrapper.h \
    $$PWD/src/components/cdownloadwidget.h \
    $$PWD/src/components/cpushbutton.h \
    $$PWD/src/components/cfiledialog.h \
    $$PWD/src/components/cprintprogress.h \
    $$PWD/src/components/ctabbar.h \
    $$PWD/src/components/cmessage.h \
    $$PWD/src/components/canimatedicon.h \
    $$PWD/src/components/ctabpanel.h \
    $$PWD/src/components/csvgpushbutton.h \
    $$PWD/src/components/celipsislabel.h \
    $$PWD/src/version.h \
    $$PWD/src/defines.h \
    $$PWD/src/ccefeventstransformer.h \
    $$PWD/src/cascapplicationmanagerwrapper.h \
    $$PWD/src/cascapplicationmanagerwrapper_private.h \
    $$PWD/src/casctabdata.h \
    $$PWD/src/utils.h \
    $$PWD/src/cstyletweaks.h \
    $$PWD/src/chelp.h \
    #$$PWD/src/csplash.h \
    $$PWD/src/cfilechecker.h \
    $$PWD/src/clogger.h \
    $$PWD/src/clangater.h \
    $$PWD/src/cscalingwrapper.h \
    $$PWD/src/ctabundockevent.h \
    $$PWD/src/ccefeventsgate.h \
    $$PWD/src/ceditortools.h \
    $$PWD/src/cwindowsqueue.h \
    $$PWD/src/ceventdriver.h \
    $$PWD/src/cappeventfilter.h \
    $$PWD/src/cthemes.h
#    src/ctabbar_p.h \
#    src/ctabstyle.h \
#    src/ctabstyle_p.h
#    src/components/casclabel.h

SOURCES += \
    $$PWD/src/windows/cmainwindow.cpp \
    $$PWD/src/windows/cwindowbase.cpp \
    $$PWD/src/windows/ceditorwindow.cpp \
    $$PWD/src/windows/cpresenterwindow.cpp \
    $$PWD/src/components/asctabwidget.cpp\
    $$PWD/src/components/cdownloadwidget.cpp \
    $$PWD/src/components/cpushbutton.cpp \
    $$PWD/src/components/cfiledialog.cpp \
    $$PWD/src/components/cprintprogress.cpp \
    $$PWD/src/components/ctabbarwrapper.cpp \
    $$PWD/src/components/ctabbar.cpp \
    $$PWD/src/components/cmessage.cpp \
    $$PWD/src/components/canimatedicon.cpp \
    $$PWD/src/components/ctabpanel.cpp \
    $$PWD/src/components/csvgpushbutton.cpp \
    $$PWD/src/components/celipsislabel.cpp \
    #$$PWD/src/csplash.cpp \
    $$PWD/src/main.cpp \
    $$PWD/src/ccefeventstransformer.cpp \
    $$PWD/src/cascapplicationmanagerwrapper.cpp \
    $$PWD/src/casctabdata.cpp \
    $$PWD/src/utils.cpp \
    $$PWD/src/cstyletweaks.cpp \
    $$PWD/src/chelp.cpp \
    $$PWD/src/cfilechecker.cpp \
    $$PWD/src/clogger.cpp \
    $$PWD/src/clangater.cpp \
    $$PWD/src/cscalingwrapper.cpp \
    $$PWD/src/ctabundockevent.cpp \
    $$PWD/src/ccefeventsgate.cpp \
    $$PWD/src/ceditortools.cpp \
    $$PWD/src/ceventdriver.cpp \
    $$PWD/src/cappeventfilter.cpp \
    $$PWD/src/cthemes.cpp
#    src/ctabstyle.cpp
#    src/components/casclabel.cpp

RESOURCES += $$PWD/resources.qrc
DEFINES += COPYRIGHT_YEAR=$${CURRENT_YEAR}

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
core_windows {
    PLATFORM_BUILD_FULL = $$PLATFORM_BUILD
    build_xp:PLATFORM_BUILD_FULL=$$join(PLATFORM_BUILD_FULL, PLATFORM_BUILD_FULL, "", "_xp")
    LIBS += -L$$CORE_3DPARTY_PATH/cef/$$PLATFORM_BUILD_FULL/build -llibcef
}
core_linux:LIBS += -L$$CORE_3DPARTY_PATH/cef/$$PLATFORM_BUILD/build -lcef

ADD_DEPENDENCY(PdfReader, PdfWriter, DjVuFile, XpsFile, HtmlRenderer, UnicodeConverter, hunspell, ooxmlsignature, kernel, kernel_network, graphics, videoplayer, ascdocumentscore, qtascdocumentscore)

core_linux {
    QT += network x11extras

    QMAKE_LFLAGS += "-Wl,-rpath,\'\$$ORIGIN\'"
    QMAKE_LFLAGS += "-Wl,-rpath,\'\$$ORIGIN/converter\'"
    QMAKE_LFLAGS += -static-libstdc++ -static-libgcc

    HEADERS +=  $$PWD/src/windows/platform_linux/cx11decoration.h \
                #$$PWD/src/windows/platform_linux/gtk_addon.h \
                $$PWD/src/windows/platform_linux/cwindowplatform.h \
                $$PWD/src/platform_linux/cdialogopenssl.h \
                $$PWD/src/platform_linux/cdialogcertificateinfo.h \
                $$PWD/src/platform_linux/singleapplication.h

    SOURCES +=  $$PWD/src/windows/platform_linux/cx11decoration.cpp \
                #$$PWD/src/windows/platform_linux/gtk_addon.cpp \
                #$$PWD/src/windows/platform_linux/cx11caption.cpp \
                $$PWD/src/windows/platform_linux/cwindowplatform.cpp \
                $$PWD/src/platform_linux/cdialogopenssl.cpp \
                $$PWD/src/platform_linux/cdialogcertificateinfo.cpp \
                $$PWD/src/platform_linux/singleapplication.cpp

    CONFIG += link_pkgconfig
    PKGCONFIG += glib-2.0 gtk+-3.0 atk
    LIBS += -lX11

    LIBS += $$CORE_3DPARTY_PATH/icu/$$PLATFORM_BUILD/build/libicuuc.so.58
    LIBS += $$CORE_3DPARTY_PATH/icu/$$PLATFORM_BUILD/build/libicudata.so.58

    DEFINES += DOCUMENTSCORE_OPENSSL_SUPPORT
}

core_windows {
    DEFINES += Q_COMPILER_INITIALIZER_LISTS

    RC_ICONS += ./res/icons/desktop_icons.ico

    HEADERS += $$PWD/src/windows/platform_win/cwindowplatform.h \
               $$PWD/src/windows/platform_win/caption.h

    SOURCES += $$PWD/src/windows/platform_win/cwindowplatform.cpp


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
            -lrpcrt4 \
            -ldwmapi
#            -lOpenGL32

    build_xp {
        DEFINES += __OS_WIN_XP
    }
}

core_release:DESTDIR = $$DESTDIR/build
core_debug:DESTDIR = $$DESTDIR/build/debug

!isEmpty(OO_BUILD_BRANDING) {
    DESTDIR = $$DESTDIR/$$OO_BUILD_BRANDING
}

DESTDIR = $$DESTDIR/$$PLATFORM_BUILD
build_xp {
    DESTDIR = $$DESTDIR/xp
}

OBJECTS_DIR = $$DESTDIR/obj
MOC_DIR = $$DESTDIR/moc
RCC_DIR = $$DESTDIR/rcc
