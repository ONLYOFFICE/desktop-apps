Summary: Desktop editors for text, spreadsheet and presentation files
Name: %{_package_name}
Version: %{_product_version}
Release: %{_build_number}
License: AGPLv3
Group: Applications/Office
URL: %{_publisher_url}
Vendor: %{_publisher_name}
Packager: %{_publisher_name} %{_support_mail}
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
BuildArch: %{_package_arch}
AutoReq: no
AutoProv: no

%description
%{_company_name} %{_product_name} installation package
 %{_company_name} %{_product_name} is an application for editing office documents (text documents, spreadsheets and presentations) from %{_company_name} cloud portal on local computer without browser using.

%include ../rpm/common.spec
