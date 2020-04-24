#!/bin/bash

outFileDir="malu_output"
servFileName="malu.go"
prefix="http://malu.me/"
index="http://cdn.malu.me/gpu/"

. ../lib
cd -
sed -i 's@"./malu.me/m@"./m@g' index.html
