#!/bin/sh
#set -x

./band_name.sh

rm -rf code_changed
mkdir code_changed

mkdir code_changed/lte
mkdir code_changed/lte/src
file=$(ls ../lte/src/*data_ag.c)
file=$(basename $file)
./main ../lte/src/$file  code_changed/lte/src/$file

mkdir code_changed/gsm
mkdir code_changed/gsm/src
file=$(ls ../gsm/src/*data_ag.c)
file=$(basename $file)
./main ../gsm/src/$file  code_changed/gsm/src/$file

mkdir code_changed/wcdma
mkdir code_changed/wcdma/src
file=$(ls ../wcdma/src/*data_ag.c)
file=$(basename $file)
./main ../wcdma/src/$file  code_changed/wcdma/src/$file

mkdir code_changed/cdma
mkdir code_changed/cdma/src
file=$(ls ../cdma/src/*data_ag.c)
file=$(basename $file)
./main ../cdma/src/$file  code_changed/cdma/src/$file

