%if %{_package_edition} == "full"
Requires: curl, libX11, xdg-utils, dejavu-lgc-sans-fonts, dejavu-lgc-sans-mono-fonts, dejavu-lgc-serif-fonts, dejavu-sans-fonts, dejavu-sans-mono-fonts, dejavu-serif-fonts, libreoffice-opensymbol-fonts, liberation-mono-fonts, liberation-sans-fonts, liberation-serif-fonts
%else
Requires: curl, libX11, libXScrnSaver, atk, gtk3, libstdc++ >= 4.8.0, boost-filesystem, xcb-util-renderutil, xcb-util-image, xcb-util-wm, libxcb, xcb-util-keysyms, xdg-utils, dejavu-lgc-sans-fonts, dejavu-lgc-sans-mono-fonts, dejavu-lgc-serif-fonts, dejavu-sans-fonts, dejavu-sans-mono-fonts, dejavu-serif-fonts, libreoffice-opensymbol-fonts, liberation-mono-fonts, liberation-narrow-fonts, liberation-sans-fonts, liberation-serif-fonts
%endif
