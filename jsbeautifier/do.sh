#!/bin/bash

outFileDir="jsb_output"
servFileName="jsbeautifier.go"
prefix="https://beautifier.io/"

function abspath() {
	if [[ -d "$1" ]]; then
		pushd "$1" >/dev/null
		pwd
		popd >/dev/null
	elif [[ -e $1 ]]; then
		pushd "$(dirname "$1")" >/dev/null
		echo "$(pwd)/$(basename "$1")"
		popd >/dev/null
	else
		echo "$1" does not exist! >&2
		return 127
	fi
}

if [ $# -ne 1 ]; then
	echo "Usage: $0 all-request-curl-file"
	exit
fi

if [ -d "$outFileDir" ]; then
	echo "output directory $outFileDir alread exists, check it, exit now"
	exit
fi

allCurlReq="$1"
acrPath=$(abspath "$allCurlReq")

mkdir "$outFileDir"
cd "$outFileDir"

cat "$acrPath" | grep -v "curl 'data:image/" | grep -v "curl '[^ ]\+google.*" | grep -v "curl '[^ ]\+gstatic.*" | grep -v "curl '[^ ]\+doubleclick.net.*" |
	while read line; do
		link=$(echo "$line" | awk -F"'" '{print $2}')
		truncLine=$(echo "$link" | sed "s@$prefix@@g")
		if [ ${#truncLine} -eq 0 ]; then
			continue
		fi

		dirName=$(dirname $truncLine)
		fileName=$(basename $truncLine)
		if [ ! -d "$dirName" ]; then
			mkdir -p "$dirName"
		fi

		if [ ! -f "$truncLine" ]; then
			wget -c "$link" -O "$truncLine"
		else
			echo "file $truncLine alread exists, jump over"
		fi
	done

wget -c "$prefix" -O index.html
sed -E -i 's@$prefix@@g;s@(.+)(apis.google.com)(.*)@@g' index.html

cat >"$servFileName" <<EOF
package main

import (
	"log"
	"net/http"

	"github.com/gobuffalo/packr"
)

func main() {
	box := packr.NewBox("./$outFileDir")

	http.Handle("/", http.FileServer(box))
	host := ":3000"
	log.Printf("listen on %s\n", host)
	http.ListenAndServe(host, nil)
}
EOF
