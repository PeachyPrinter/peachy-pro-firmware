CC=arm-none-eabi-gcc
AR=arm-none-eabi-ar

LIBNAME = libstm32f0-usb.a

SRCS = usb_core.c usb_dcd.c usb_dcd_int.c usbd_core.c usbd_ioreq.c usbd_req.c usbd_cdc_core.c

##############################################################################

vpath %.c STM32_USB_Device_Driver/src STM32_USB_Device_Library/Core/src STM32_USB_Device_Library/Class/cdc/src

CFLAGS  = -g -O2 -Wall
CFLAGS += -mlittle-endian -mthumb -mcpu=cortex-m0 -march=armv6s-m
CFLAGS += -ffreestanding -nostdlib
CFLAGS += -I./inc -I../cmsis_boot -I../cmsis_core -I./STM32_USB_Device_Driver/inc
CFLAGS += -I./STM32_USB_Device_Library/Class/cdc/inc -I./STM32_USB_Device_Library/Core/inc
CFLAGS += -I../cmsis_lib/include

OBJS = $(SRCS:.c=.o)

.PHONY: all clean

all: $(LIBNAME)

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $^

$(LIBNAME): $(OBJS)
	$(AR) -r $@ $(OBJS)

clean:
	rm -f $(OBJS) $(LIBNAME)
