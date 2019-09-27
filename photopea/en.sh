#!/bin/bash

outFileDir="en_output"
servFileName="photopea.go"
prefix="https://www.photopea.com/"

. ../lib
cd -
sed -E -i 's@(.*src="ads.js".*)@@g' index.html
