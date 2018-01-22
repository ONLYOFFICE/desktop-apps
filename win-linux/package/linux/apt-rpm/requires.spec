%if %{_package_edition} == "full"
Requires: libX11, fonts-ttf-dejavu, fonts-ttf-liberation, fonts-ttf-google-crosextra-carlito
%else
Requires: libX11, libXScrnSaver, libcurl, libgtkglext, libcairo, fonts-ttf-dejavu, fonts-ttf-liberation, fonts-ttf-google-crosextra-carlito
%endif
