%if %{_package_edition} == "full"
Requires: libx11_6, fonts-ttf-dejavu, fonts-ttf-liberation
%else
Requires: libx11_6, libxscrnsaver1, libcurl4, libgtkglext1.0_0, libcairo2, fonts-ttf-dejavu, fonts-ttf-liberation
%endif
#Suggests: webcore-fonts
