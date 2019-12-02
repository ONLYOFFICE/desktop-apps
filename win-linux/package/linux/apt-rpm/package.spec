Summary: Desktop editors for text, spreadsheet and presentation files
Name: %{_package_name}
Version: %{_product_version}
Release: %{_build_number}
License: AGPLv3
Group: Applications/Office
URL: %{_publisher_url}
Vendor: %{_publisher_name}
Packager: %{_publisher_name} %{_support_mail}
%if %{_package_edition} == "full"
Requires: curl, libX11, xdg-utils, fonts-ttf-dejavu, fonts-ttf-liberation, fonts-ttf-google-crosextra-carlito
%else
Requires: curl, libX11, libXScrnSaver, libatk, libgtk+2, libcairo, xdg-utils, fonts-ttf-dejavu, fonts-ttf-liberation, fonts-ttf-google-crosextra-carlito
%endif
BuildArch: %{_package_arch}
AutoReq: no
AutoProv: no

%description
%{_company_name} %{_product_name} installation package
 %{_company_name} %{_product_name} is an application for editing office documents (text documents, spreadsheets and presentations) from %{_company_name} cloud portal on local computer without browser using.

%include ../rpm/common.spec
