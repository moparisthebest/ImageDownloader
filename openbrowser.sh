#!/bin/bash
set -e
URL=$1
BROWSER=xdg-open
AES_GCM="$(dirname $(readlink -f $0))/aesgcm"
if [ ${URL: -97:1} == "#" ]
then
    [ -e "$AES_GCM" ] || gcc "$(dirname $(readlink -f $0))/aesgcm.c" -lcrypto -o "$AES_GCM"
    curl "$URL" | "$AES_GCM" "${URL: -96}" | feh -. -
elif echo $URL | grep -i '\.pgp$'
then
    curl "$URL" | gpg2 -d | feh -. -
else
   $BROWSER $URL
fi
