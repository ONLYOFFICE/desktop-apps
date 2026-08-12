%if "%{_package_edition}" == "full"
Requires: curl, libcups2, libX11-6, xdg-utils, dejavu-fonts, liberation-fonts
%else
Requires: curl, libcups2, libX11-6, libatk-1_0-0, libgtk-3-0, libstdc++6, xdg-utils, libxcb-util1, libxkbcommon-x11-0, dejavu-fonts, liberation-fonts
%endif
