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
  gstreamer1.0-libav,
  gstreamer1.0-plugins-ugly,
  libxss1,
  libatk1.0-0,
  libgtk-3-0,
  libcairo2,
  libgconf-2-4,
  libstdc++6 (>=4.8),
  fonts-dejavu | ttf-dejavu,
  fonts-liberation,
  fonts-crosextra-carlito,
  xdg-utils')
Recommends: ttf-mscorefonts-installer, fonts-takao-gothic 
Description: M4_COMPANY_NAME M4_PRODUCT_NAME installation package
 M4_COMPANY_NAME M4_PRODUCT_NAME is an application for editing office documents (text documents, spreadsheets and presentations) from M4_COMPANY_NAME cloud portal on local computer without browser using.
