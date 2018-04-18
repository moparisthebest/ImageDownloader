#!/bin/bash
#exec firefox "$@"
set -e
URL=$1
BROWSER=xdg-open
BROWSER=firefox
AES_GCM="$(dirname $(readlink -f $0))/aesgcm"
if [ ${URL: -97:1} == "#" -o ${URL: -89:1} == "#" ]
then
    [ -e "$AES_GCM" ] || gcc "$(dirname $(readlink -f $0))/aesgcm.c" -lcrypto -o "$AES_GCM"
    iv_key="$(echo "$URL" | sed 's/^.*#//')"
    URL="$(echo "$URL" | sed 's/^aesgcm/https/')"
    curl "$URL" | "$AES_GCM" "$iv_key" | feh -. -
elif echo $URL | grep -i '\.pgp$'
then
    curl "$URL" | gpg2 -d | feh -. -
else
   $BROWSER $URL
fi
