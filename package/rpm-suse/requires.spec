%if "%{_package_edition}" == "full"
Requires: curl, libcups2, libX11-6, xdg-utils, dejavu-fonts, liberation-fonts
%else
Requires: curl, libcups2, libX11-6, libatk-1_0-0, libgtk-3-0, libstdc++6, xdg-utils, libxcb-util1, libxkbcommon-x11-0, dejavu-fonts, liberation-fonts
%if "%{_media_player}" == "libvlc"
Requires: libasound2, libdrm2, libglvnd, libpulse0, libvdpau1, libX11-xcb1, libxcb1, libxcb-dri3-0, libxcb-keysyms1, libxcb-shm0, libxcb-xv0, libXext6, libXfixes3, libz1
%endif
%endif
