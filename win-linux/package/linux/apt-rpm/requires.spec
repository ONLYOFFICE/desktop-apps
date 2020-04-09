%if %{_package_edition} == "full"
Requires: curl, libX11, xdg-utils, fonts-ttf-dejavu, fonts-ttf-liberation, fonts-ttf-google-crosextra-carlito
%else
Requires: curl, libX11, libXScrnSaver, libatk, libgtk+3, libcairo, xdg-utils, fonts-ttf-dejavu, fonts-ttf-liberation, fonts-ttf-google-crosextra-carlito
%endif
