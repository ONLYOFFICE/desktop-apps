%if %{_package_edition} == "full"
Requires: curl, libx11_6, xdg-utils, fonts-ttf-dejavu, fonts-ttf-liberation
%else
Requires: curl, libx11_6, libxscrnsaver1, libatk1.0_0, libgtk+3_0, libcairo2, xdg-utils, fonts-ttf-dejavu, fonts-ttf-liberation
%endif
#Suggests: webcore-fonts
