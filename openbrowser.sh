#!/bin/bash
URL=$1
BROWSER=firefox
JAVA=/usr/lib/jvm/java-8-openjdk/bin/java
IMAGE_DOWNLOADER=/home/daniel/Projects/ImageDownloader/target/ImageDownloader-0.1.jar
if [ ${URL: -97:1} == "#" ]; then
	$JAVA -jar $IMAGE_DOWNLOADER $URL | feh -. -
else
   $BROWSER $URL
fi
