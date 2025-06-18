DESTDIR = $$PWD
include(common.pri)

DEFINES += COPYRIGHT_YEAR=$${CURRENT_YEAR}
DEFINES += APP_ICON_PATH=\"./icons/desktopeditors.ico\"
DEFINES += APP_LANG_PATH=\"./langs/langs.iss\"

OTHER_FILES += $$PWD/res/langs/langs.iss

ENV_URL_INSTALL = $$(DESKTOP_URL_UPDATES_MAIN_CHANNEL)
isEmpty(ENV_URL_INSTALL): DEFINES += URL_INSTALL=\\\"\\\"
else: DEFINES += URL_INSTALL=\\\"$${ENV_URL_INSTALL}\\\"

ENV_URL_INSTALL_DEV = $$(DESKTOP_URL_UPDATES_DEV_CHANNEL)
isEmpty(ENV_URL_INSTALL_DEV): DEFINES += URL_INSTALL_DEV=\\\"\\\"
else: DEFINES += URL_INSTALL_DEV=\\\"$${ENV_URL_INSTALL_DEV}\\\"

message(install url: \\\"$$ENV_URL_INSTALL\\\")
message(install dev url: \\\"$$ENV_URL_INSTALL_DEV\\\")
