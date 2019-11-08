Package: {{PACKAGE_NAME}}
Version: {{PACKAGE_VERSION}}
Architecture: amd64
Section: editors
Maintainer: Ascensio System SIA <support@onlyoffice.com>
#Source: ca-certificates-local
#Build-Depends: debhelper (>= 8.0.0)
Pre-Depends: dpkg (>= 1.14.0)
#FullDepends: x11-common, curl | wget, fonts-dejavu | ttf-dejavu, fonts-opensymbol 
Depends: x11-common, libasound2, curl | wget, libxss1, libatk1.0-0, libgtk2.0-0, libcairo2, libgconf-2-4, libstdc++6 (>=4.8), fonts-dejavu | ttf-dejavu, fonts-opensymbol, fonts-liberation, fonts-crosextra-carlito, xdg-utils
Recommends: ttf-mscorefonts-installer, fonts-takao-gothic 
#Standards-Version: 3.9.4
Priority: optional
Installed-Size: 183024
Description: ONLYOFFICE DesktopEditors installation package
 ONLYOFFICE DesktopEditors is an application for editing office documents (text documents, spreadsheets and presentations) from onlyoffice cloud portal on local computer without browser using.

