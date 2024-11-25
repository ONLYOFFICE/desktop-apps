DESTDIR = $$PWD
include(common.pri)

DEFINES += COPYRIGHT_YEAR=$${CURRENT_YEAR}
DEFINES += APP_ICON_PATH=\"./icons/desktopeditors.ico\"
DEFINES += APP_LANG_PATH=\"./langs/langs.iss\"

OTHER_FILES += $$PWD/res/langs/langs.iss

ENV_URL_INSTALL_X64 = $$(DESKTOP_URL_INSTALL_CHANNEL_X64)
!isEmpty(ENV_URL_INSTALL_X64) {
    DEFINES += URL_INSTALL_X64=\\\"$${ENV_URL_INSTALL_X64}\\\"
}

ENV_URL_INSTALL_X86 = $$(DESKTOP_URL_INSTALL_CHANNEL_X86)
!isEmpty(ENV_URL_INSTALL_X86) {
    DEFINES += URL_INSTALL_X86=\\\"$${ENV_URL_INSTALL_X86}\\\"
}

ENV_URL_INSTALL_X64_MSI = $$(DESKTOP_URL_INSTALL_CHANNEL_X64_MSI)
!isEmpty(ENV_URL_INSTALL_X64_MSI) {
    DEFINES += URL_INSTALL_X64_MSI=\\\"$${ENV_URL_INSTALL_X64_MSI}\\\"
}

ENV_URL_INSTALL_X86_MSI = $$(DESKTOP_URL_INSTALL_CHANNEL_X86_MSI)
!isEmpty(ENV_URL_INSTALL_X86_MSI) {
    DEFINES += URL_INSTALL_X86_MSI=\\\"$${ENV_URL_INSTALL_X86_MSI}\\\"
}

message(install x64 url: \\\"$$ENV_URL_INSTALL_X64\\\")
message(install x86 url: \\\"$$ENV_URL_INSTALL_X86\\\")
message(install x64 xp url: \\\"$$ENV_URL_INSTALL_X64_XP\\\")
message(install x86 xp url: \\\"$$ENV_URL_INSTALL_X86_XP\\\")
message(install x64 msi url: \\\"$$ENV_URL_INSTALL_X64_MSI\\\")
message(install x86 msi url: \\\"$$ENV_URL_INSTALL_X86_MSI\\\")
