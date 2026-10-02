%if "%{_package_edition}" == "full"
Requires: curl, cups-libs, libX11, xdg-utils, dejavu-sans-fonts, dejavu-sans-mono-fonts, dejavu-serif-fonts, liberation-mono-fonts, liberation-sans-fonts, liberation-serif-fonts
%else
Requires: curl, cups-libs, libX11, libXScrnSaver, atk, gtk3, libstdc++ >= 4.8.0, boost-filesystem, xcb-util-renderutil, xcb-util-image, xcb-util-wm, libxcb, xcb-util-keysyms, xdg-utils, libxkbcommon-x11, libnotify, dejavu-sans-fonts, dejavu-sans-mono-fonts, dejavu-serif-fonts, liberation-mono-fonts, liberation-narrow-fonts, liberation-sans-fonts, liberation-serif-fonts
%if "%{_media_player}" == "libvlc"
# libX11-xcb.so.1 is a part of libX11 on EL7 and a separate package since EL8
Requires: alsa-lib, libdrm, libglvnd-egl, libglvnd-glx, libvdpau, libX11-xcb.so.1()(64bit), libXext, libXfixes, pulseaudio-libs, zlib
%endif
%endif
