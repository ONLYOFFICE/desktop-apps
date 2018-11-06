%if %{_package_edition} == "full"
Requires: libX11, xdg-utils, fonts-ttf-dejavu, fonts-ttf-liberation, fonts-ttf-google-crosextra-carlito
%else
Requires: libX11, libXScrnSaver, libgtkglext, libcairo, xdg-utils, fonts-ttf-dejavu, fonts-ttf-liberation, fonts-ttf-google-crosextra-carlito
%endif
