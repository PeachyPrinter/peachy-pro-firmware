sudo add-apt-repository ppa:terry.guo/gcc-arm-embedded
# this may just say 0 removed. that's fine.
sudo apt-get remove binutils-arm-none-eabi gcc-arm-none-eabi
sudo apt-get update
sudo apt-get install gcc-arm-none-eabi

git clone git@github.com:peachyprinter/peachy-pro-firmware.git
cd peachy-pro-firmware
make

should result in a main.bin

Get the source for dfu-util:
git clone git://git.code.sf.net/p/dfu-util/dfu-util
./autogen.sh
./configure
sudo make install

Get openocd:
sudo apt-get remove openocd
git clone git://git.code.sf.net/p/openocd/code openocd-code
cd openocd-code
./bootstrap
./configure
make
sudo make install

Connect openocd to the stmf4discovery
sudo openocd -f stlink.cfg

Discovery board (reading SWD) : [o,NRST,SDIO,GND,SDCLK,o]
Peachy (boards 1.13 and older): [o,NRST,GND;o,SDCLK,SDIO]
-top down USB on the left


Run arm-none-eabi-gdb with main.bin
arm-none-eabi-gdb main.elf

Connect to openocd:
(gdb) target remote localhost:3333

Commands:

monitor reset halt - stop the microcontroller
c - continue
break file.c:32 - break on line 32 in file.c
