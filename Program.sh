#!/bin/bash
cd /home/peachyprinter/git/peachy-pro-firmware

echo
echo 'Is the Git repository at:' $PWD ' ?'
echo 'if not change path inside script'
echo
echo 'Short Programming pins and plug in usb (pins labeled JP4)'
echo 'CTRL + C to quit'
while true; do
    echo 
    echo '1) program peachy'
    echo '2) compile` not_safe git'
    echo '3) compile master git'
		echo '4) merge master into not_safe'
    read -p "Give option (1|2|3|4):" input
    
    if [ $input == 1 ]; then
        dfu-util -a 0 --dfuse-address 0x08000000 -D main.bin -v -d 16d0:0af3
    elif [ $input == 2 ]; then
        git pull;
				git checkout not_safe
        make
				echo 
				echo ----------------------
    elif [ $input == 3 ]; then
        git pull;
				git checkout master 
        make
				echo 
				echo ----------------------
    elif [ $input == 4 ]; then
			echo
			echo 'Please only do this if you know what you are doing - Will'
			read -p "Are you sure you want to merge? (y|n)" $merge
			if [[ $merge == "y" ]]; then
				./merge_master_not_safe.sh
			fi
    fi
done
