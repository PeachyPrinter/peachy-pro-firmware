# put your *.o targets here, make should handle the rest!
SRCS = main.c system_stm32f0xx.c usbd_desc.c usb_bsp.c usbd_usr.c usbd_cdc_vcp.c

# all the files will be generated with this name (main.elf, main.bin, main.hex, etc)
PROJ_NAME=main

# Location of the Libraries folder from the STM32F0xx Standard Peripheral Library
STD_PERIPH_LIB=lib/cmsis_lib
USB_PERIPH_LIB=lib/cmsis_usb

# Location of the linker scripts
LDSCRIPT_INC=lib/cmsis_boot/ldscripts

# location of OpenOCD Board .cfg files (only used with 'make program')
OPENOCD_BOARD_DIR=/usr/local/share/openocd/scripts/board

# Configuration (cfg) file containing programming directives for OpenOCD
OPENOCD_PROC_FILE=stlink.cfg

# that's it, no need to change anything below this line!

###################################################

CC=arm-none-eabi-gcc
OBJCOPY=arm-none-eabi-objcopy
OBJDUMP=arm-none-eabi-objdump
SIZE=arm-none-eabi-size

CFLAGS  = -Wall -g -std=c99 -O0 
#CFLAGS += -mlittle-endian -mthumb -mcpu=cortex-m0 -march=armv6s-m
CFLAGS += -mlittle-endian -mcpu=cortex-m0  -march=armv6-m -mthumb
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += -Wl,--gc-sections -Wl,-Map=$(PROJ_NAME).map

###################################################

vpath %.c src
vpath %.a $(STD_PERIPH_LIB) $(USB_PERIPH_LIB)

ROOT=$(shell pwd)

CFLAGS += -I./inc -I $(STD_PERIPH_LIB)/include -I lib/cmsis_boot -I lib/cmsis_core
CFLAGS += -include lib/cmsis_boot/stm32f0xx_conf.h 
CFLAGS += -I./lib/cmsis_usb/STM32_USB_Device_Driver/inc
CFLAGS += -I./lib/cmsis_usb/STM32_USB_Device_Library/Class/cdc/inc 
CFLAGS += -I./lib/cmsis_usb/STM32_USB_Device_Library/Core/inc

SRCS += lib/cmsis_boot/startup/startup_stm32f0xx.s # add startup file to build

# need if you want to build with -DUSE_CMSIS 
#SRCS += stm32f0_discovery.c
#SRCS += stm32f0_discovery.c stm32f0xx_it.c

OBJS = $(SRCS:.c=.o)

###################################################

.PHONY: lib proj usb

all: lib proj usb

usb:
	$(MAKE) -C $(USB_PERIPH_LIB)

lib:
	$(MAKE) -C $(STD_PERIPH_LIB)

proj: 	$(PROJ_NAME).elf

$(PROJ_NAME).elf: $(SRCS)
	$(CC) $(CFLAGS) $^ -o $@ -L$(STD_PERIPH_LIB) -lstm32f0 -L$(USB_PERIPH_LIB) -lstm32f0-usb -L$(LDSCRIPT_INC) -Tstm32f0.ld 
	$(OBJCOPY) -O ihex $(PROJ_NAME).elf $(PROJ_NAME).hex
	$(OBJCOPY) -O binary $(PROJ_NAME).elf $(PROJ_NAME).bin
	$(OBJDUMP) -St $(PROJ_NAME).elf >$(PROJ_NAME).lst
	$(SIZE) $(PROJ_NAME).elf

program: $(PROJ_NAME).bin
#	openocd -f $(OPENOCD_BOARD_DIR)/stm32f0discovery.cfg -f $(OPENOCD_PROC_FILE) -c "stm_flash `pwd`/$(PROJ_NAME).bin" -c shutdown
	dfu-util -a 0 --dfuse-address 0x08000000 -D main.bin -v

clean:
	find ./ -name '*~' | xargs rm -f	
	rm -f *.o
	rm -f $(PROJ_NAME).elf
	rm -f $(PROJ_NAME).hex
	rm -f $(PROJ_NAME).bin
	rm -f $(PROJ_NAME).map
	rm -f $(PROJ_NAME).lst

reallyclean: clean
	$(MAKE) -C $(STD_PERIPH_LIB) clean
