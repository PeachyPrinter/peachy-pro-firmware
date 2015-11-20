#!/bin/bash
cd /home/peachy/peachy_files/peachy-pro-firmware

echo
echo 'Git repository at:' $PWD
echo 'Short Programming pins and plug in usb (pins labeled JP4)'
while true; do
    echo 
    echo '1) update git'
    echo '2) program peachy'
    read -p "Give option (1|2):" input
    
    if [ $input == 1 ]; then
        git pull;
    elif [ $input == 2 ]; then
        make
        dfu-util -a 0 --dfuse-address 0x08000000 -D main.bin -v -d 16d0:0af3
    fi
done
