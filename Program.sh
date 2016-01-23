#!/bin/bash
cd /home/peachyprinter/git/peachy-pro-firmware

NOT_SAFE_DEFINES="
	#ifndef INC_OVERRIDES_H_
	#define INC_OVERRIDES_H_

	#define ADC_KEY_EN 0
	#define INTERLOCK_KEY_EN 0
	#define LED_OVERRIDES_EN 1

	#endif"

MASTER_DEFINES="
	#ifndef INC_OVERRIDES_H_
	#define INC_OVERRIDES_H_

	#define ADC_KEY_EN 1
	#define INTERLOCK_KEY_EN 1
	#define LED_OVERRIDES_EN 0

	#endif"

MODE="UNKOWN"

echo
echo 'Is the Git repository at:' $PWD ' ?'
echo 'if not change path inside script'
echo
echo 'Short Programming pins and plug in usb (pins labeled JP4)'
while true; do
	echo 
	echo ---------------------------
	echo 'Current git state:' $MODE
	echo ---------------------------
	echo 'CTRL + c to quit'
	echo '1) program peachy'
	echo '2) compile not_safe branch'
	echo '3) compile master branch'
	echo '4) recompile current files'
	echo '5) merge master into not_safe'
	read -p "Give option (1|2|3|4|5):" input
	
	if [ $input == 1 ]; then
		dfu-util -a 0 --dfuse-address 0x08000000 -D main.bin -v -d 16d0:0af3
	elif [ $input == 2 ]; then
		git pull;
		git checkout not_safe
		echo $NOT_SAFE_DEFINES "> inc/overrides.h"
		make
		MODE="NOT_SAFE"
	elif [ $input == 3 ]; then
		git pull;
		git checkout master 
		echo $MASTER_DEFINES "> inc/overrides.h"
		make
		MODE="MASTER"
	elif [ $input == 4 ]; then
		make
		MODE="UNKOWN"
	elif [ $input == 5 ]; then
		echo
		echo ---------------------------
		echo 'Please only do this if your name is Will'
		echo 'Gavin.... Just stop ;)'
		echo ---------------------------
		echo
		read -p "Are you sure you want to merge? (y|N):" merge
		if [ "$merge" == "y" ]; then
			source merge_master_not_safe.sh
		fi
	fi
done
