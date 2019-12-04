%if %{_package_arch} == "i586"
%if %{_package_edition} == "full"
Requires: curl, libx11_6, xdg-utils, fonts-ttf-dejavu, fonts-ttf-liberation
%else
Requires: curl, libx11_6, libxscrnsaver1, libatk1.0_0, libgtk+2.0_0, libcairo2, xdg-utils, fonts-ttf-dejavu, fonts-ttf-liberation
%endif
%endif
%if %{_package_arch} == "x86_64"
%if %{_package_edition} == "full"
Requires: curl, lib64x11_6, xdg-utils, fonts-ttf-dejavu, fonts-ttf-liberation
%else
Requires: curl, lib64x11_6, lib64xscrnsaver1, lib64atk1.0_0, lib64gtk+2.0_0, lib64cairo2, xdg-utils, fonts-ttf-dejavu, fonts-ttf-liberation
%endif
%endif
