#!/bin/sh

CUR_DIR=$(pwd)

MODE="gsm lte wcdma cdma"

rm -rf code_changed 2>/dev/null
mkdir code_changed

for loop in $MODE
do
	mkdir code_changed/$loop
	mkdir code_changed/$loop/src
	file=$(ls ../$loop/src/*data_ag.c)
	file=$(basename $file)
	./main -i ../$loop/src/$file  -o code_changed/$loop/src/$file
done
