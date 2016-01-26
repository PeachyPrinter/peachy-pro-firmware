#!/bin/bash
cd /home/peachyprinter/git/peachy-pro-firmware

OVERRIDE_FILE="inc/overrides.h"

NOT_SAFE_OVERRIDES="./overrides/not_safe_overrides.h"
SAFE_OVERRIDES="./overrides/safe_overrides.h"

MODE="UNKOWN"
VERSION="$(./version.sh)"

echo
echo 'Is the Git repository at:' $PWD ' ?'
echo 'if not change path inside script'
echo
echo 'Short Programming pins and plug in usb (pins labeled JP4)'

while true; do
	echo 
	echo ---------------------------
	echo 'Checked out git:' $VERSION
	echo 'Current git state:' $MODE
	echo ---------------------------
	echo 'q) Quit (CTRL+c at any time)'
	echo '1) program peachy'
	echo '2) recompile current files'
	echo '3) compile master branch'
	echo '4) compile not_safe branch'
	echo '5) git pull'
	echo '6) reset local git files'
	echo '7) merge master into not_safe'
	read -p "Give option (q|1|2|3|4|5|6|7):" input
	
	if [ $input == 1 ]; then
		dfu-util -a 0 --dfuse-address 0x08000000 -D main.bin -v -d 16d0:0af3
	elif [ $input == 2 ]; then
		make
		MODE="USER-COMPILED"
	elif [ $input == 3 ]; then
		git checkout master 
		cp $SAFE_OVERRIDES $OVERRIDE_FILE
		make
		MODE="MASTER"
		VERSION="$(./version.sh)"
	elif [ $input == 4 ]; then
		#git checkout not_safe
		cp $NOT_SAFE_OVERRIDES $OVERRIDE_FILE
		make
		MODE="NOT_SAFE"
		VERSION="$(./version.sh)"
	elif [ $input == 5 ]; then
		git pull
		VERSION="$(./version.sh)"
	elif [ $input == 6 ]; then
		echo
		echo ---------------------------
		echo 'CAUTION: This will remove all your modified files'
		echo '         This deletes files and re-pulls'
		echo ---------------------------
		echo
		read -p "Are you sure you want to start over? (y|N):" choice
		if [ "$choice" == "y" ]; then
			git rm --cached -r .
			git reset --hard
			git checkout master
			cp $SAFE_OVERRIDES $OVERRIDE_FILE
			make
			MODE="MASTER"
		fi
	elif [ $input == 7 ]; then
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
	elif [ "$input" == "q" ]; then
		exit
	fi
done
