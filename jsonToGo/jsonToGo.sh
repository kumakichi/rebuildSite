#!/bin/bash

outFileDir="jtg_output"
servFileName="jtg.go"
prefix="https://mholt.github.io/json-to-go/"

. ../lib

cd -
sed -E -i 's@(^initAnalytics.*)@//\1@g' resources/js/common.js
