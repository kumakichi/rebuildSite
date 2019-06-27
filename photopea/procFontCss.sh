#!/bin/bash

if [ $# -ne 1 ]
then
	echo "Usage: $0 font-css-file"
	echo "eg. $0 font.css"
	exit
fi

fontCss=$1

if [ ! -d "../fonts" ]; then
	mkdir ../fonts
fi

for i in $(cat "$fontCss" | grep 'url(https' | awk -F[\)\(] '{print $6}'); do
	wget -c "$i" -O ../fonts/$(basename $i)
done

sed -i -E 's@(.*)(url\(https:[^\)]+/)([^/\)]+)(\).+)@\1url("../fonts/\3"\4@g' "$fontCss"
