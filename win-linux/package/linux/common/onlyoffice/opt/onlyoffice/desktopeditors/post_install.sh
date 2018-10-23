#!/bin/bash

SCRIPT=$(readlink -f "$0")
SCRIPTPATH=$(dirname "$SCRIPT")

echo "$SCRIPTPATH"

if [[ -f "$SCRIPTPATH/converter/libcurl-gnutls.so.4" ]]
then
exit
fi

search_place=( "/lib" "/lib/x86_64-linux-gnu" "/lib64" "/lib64/x86_64-linux-gnu" "/usr/lib" "/usr/lib/x86_64-linux-gnu" "/usr/lib64" "/usr/lib64/x86_64-linux-gnu" "/lib/i386-linux-gnu" "/usr/lib/i386-linux-gnu")

for element in ${search_place[@]}
do
if [[ -f "$element/libcurl-gnutls.so.4" ]]
then 
echo "libcurl-gnutls.so.4: $element/libcurl-gnutls.so.4"
exit
fi
done

for element in ${search_place[@]}
do
if [[ -f "$element/libcurl.so.4" ]]
then 
ln -s "$element/libcurl.so.4" "$element/libcurl-gnutls.so.4"
echo "libcurl-gnutls.so.4: symlink to $element/libcurl.so.4"
exit
fi
done

echo "need install full version!!!"
