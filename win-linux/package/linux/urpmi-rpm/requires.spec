%if %{_package_edition} == "full"
Requires: lib64x11_6, fonts-ttf-dejavu, fonts-ttf-liberation
%else
Requires: lib64x11_6, lib64xscrnsaver1, lib64curl4, lib64gtkglext1.0_0, lib64cairo2, fonts-ttf-dejavu, fonts-ttf-liberation
%end
#Suggests: webcore-fonts