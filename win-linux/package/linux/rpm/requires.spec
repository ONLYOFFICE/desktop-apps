%if %{_package_edition} == "full"
Requires: libX11, xdg-utils, dejavu-lgc-sans-fonts, dejavu-lgc-sans-mono-fonts, dejavu-lgc-serif-fonts, dejavu-sans-fonts, dejavu-sans-mono-fonts, dejavu-serif-fonts, libreoffice-opensymbol-fonts, liberation-mono-fonts, liberation-sans-fonts, liberation-serif-fonts
%else
Requires: libX11, libXScrnSaver, gtkglext-libs, libstdc++ >= 4.8.0, boost-filesystem, xcb-util-renderutil, xcb-util-image, xcb-util-wm, libxcb, xcb-util-keysyms, xdg-utils, dejavu-lgc-sans-fonts, dejavu-lgc-sans-mono-fonts, dejavu-lgc-serif-fonts, dejavu-sans-fonts, dejavu-sans-mono-fonts, dejavu-serif-fonts, libreoffice-opensymbol-fonts, liberation-mono-fonts, liberation-narrow-fonts, liberation-sans-fonts, liberation-serif-fonts
%endif
