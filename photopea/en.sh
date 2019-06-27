#!/bin/bash

servfile_name="photopea.go"
prefix="https://www.photopea.com/"

if [ $# -ne 1 ]
then
	echo "Usage: $0 all-request-curl-file"
	exit
fi

cat $1 | grep -v "curl 'data:image/" | grep -v "curl '[^ ]\+google.*" | grep -v "curl '[^ ]\+gstatic.*" | grep -v "curl '[^ ]\+doubleclick.net.*" |
while read line
do
	link=$(echo "$line" | awk -F"'" '{print $2}')
	truncLine=$(echo "$link" | sed "s@$prefix@@g")
	if [ ${#truncLine} -eq 0 ]
	then
		continue
	fi

	dirName=$(dirname $truncLine)
	fileName=$(basename $truncLine)
	if [ ! -d "$dirName" ]
	then
		mkdir -p "$dirName"
	fi
	
	if [ ! -f "$truncLine" ]
	then
		wget -c "$link" -O "$truncLine"
	else
		echo "file $truncLine alread exists, jump over"
	fi
done

cat > "$servfile_name" << EOF
package main

import (
	"flag"
	"html/template"
	"log"
	"net/http"
)

const (
	MainPage = "index.html"
)

var port string

func init() {
	flag.StringVar(&port, "p", "3000", "listen port")
	flag.Parse()
}

func main() {
	http.HandleFunc("/backend", handler)
	http.Handle("/", http.StripPrefix("/", http.FileServer(http.Dir("."))))

	log.Printf("Listen at :%s", port)
	if err := http.ListenAndServe(":"+port, nil); err != nil {
		log.Fatal(err)
	}
}

func handler(rw http.ResponseWriter, req *http.Request) {
	t, _ := template.ParseFiles(MainPage)
	t.Execute(rw, nil)
}
EOF
