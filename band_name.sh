#!/bin/bash


CUR_DIR=$(pwd)

cd ../gsm/src
gsm_band=$(cat *data_ag.c |grep -E "(rf_card_.*_sig_cfg) |(rf_card_.*device_info)" - |grep -v "init" |grep -v "cal0" | sed -E 's/.*(rf_card_.*_sig_cfg).*/\1/' -|sed -E 's/.*(rf_card_.*device_info).*/\1/' - |uniq )
cd $CUR_DIR

cd ../lte/src
wcdma_band=$(cat *data_ag.c |grep -E "(rf_card_.*_sig_cfg) |(rf_card_.*device_info)" - |grep -v "init" | grep -v "cal0" | sed -E 's/.*(rf_card_.*_sig_cfg).*/\1/' -|sed -E 's/.*(rf_card_.*device_info).*/\1/' - |uniq )
cd $CUR_DIR

cd ../wcdma/src
lte_band=$(cat *data_ag.c |grep -E "(rf_card_.*_sig_cfg) |(rf_card_.*device_info)" - |grep -v "init" |grep -v "cal0" | sed -E 's/.*(rf_card_.*_sig_cfg).*/\1/' -|sed -E 's/.*(rf_card_.*device_info).*/\1/' - |uniq )
cd $CUR_DIR

cd ../cdma/src
cdma_band=$(cat *data_ag.c |grep -E "(rf_card_.*_sig_cfg) |(rf_card_.*device_info)" - |grep -v "init" | grep -v "cal0" | sed -E 's/.*(rf_card_.*_sig_cfg).*/\1/' -|sed -E 's/.*(rf_card_.*device_info).*/\1/' - |uniq )
cd $CUR_DIR

echo > band_name.txt
for loop in $gsm_band
do
	echo $loop >> band_name.txt
done
for loop in $wcdma_band
do
	echo $loop >> band_name.txt
done
for loop in $lte_band
do
	echo $loop >> band_name.txt
done
for loop in $cdma_band
do
	echo $loop >> band_name.txt
done