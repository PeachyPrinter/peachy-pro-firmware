#!/bin/bash
cd /home/peachy/peachy_files/peachy-pro-firmware

echo 'Git repository at:' $PWD
echo 
echo 'Short Programming pins and plug in (usually JP4)'
read -rsp 'LEDS should be off, press any key to program'
make
sudo dfu-util -a 0 --dfuse-address 0x08000000 -D main.bin -v -d 16d0:0af3
#sudo dfu-util 0.5
