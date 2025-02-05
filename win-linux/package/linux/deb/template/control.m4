Source: M4_PACKAGE_NAME
Section: editors
Priority: optional
Maintainer: M4_PUBLISHER_NAME <M4_SUPPORT_MAIL>
Build-Depends: debhelper (>= 8.0.0)

Package: M4_PACKAGE_NAME
#Standards-Version: 3.9.4
Architecture: M4_DEB_ARCH
Pre-Depends: dpkg (>= 1.14.0)
Depends:
ifelse(M4_PACKAGE_EDITION, full,
` x11-common,
  curl | wget,
  fonts-dejavu | ttf-dejavu'
,
` x11-common,
  libasound2,
  curl | wget,
  desktop-file-utils,
  gstreamer1.0-libav,
  gstreamer1.0-plugins-ugly,
  libxss1,
  libatk1.0-0,
  libgtk-3-0,
  libcairo2,
  libstdc++6 (>=4.8),
  libxkbcommon-x11-0,
  fonts-dejavu | ttf-dejavu,
  fonts-liberation,
  fonts-crosextra-carlito,
  xdg-utils')
Recommends: ttf-mscorefonts-installer, fonts-takao-gothic 
ifelse(M4_COMPANY_NAME, ONLYOFFICE,Suggests: M4_PACKAGE_NAME-help
,)dnl
Description: office suite
 M4_COMPANY_NAME M4_PRODUCT_NAME is a free office suite that combines text,
 spreadsheet and presentation editors allowing to create, view and edit
 documents without an Internet connection.
 It is fully compatible with Office Open XML formats: .docx, .xlsx, .pptx.

ifelse(M4_COMPANY_NAME, ONLYOFFICE,
Package: M4_PACKAGE_NAME-help
Architecture: all
Pre-Depends: dpkg (>= 1.14.0)
Depends:
  M4_PACKAGE_NAME
Description: offline help for M4_COMPANY_NAME M4_PRODUCT_NAME
 This package contains offline help files.,)
