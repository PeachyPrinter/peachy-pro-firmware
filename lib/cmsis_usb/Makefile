CC=arm-none-eabi-gcc
AR=arm-none-eabi-ar

LIBNAME = libstm32f0-usb.a

SRCS = usb_bsp.c \
  usbd_cdc_vcp.c \
  usbd_desc.c \
  usbd_pwr.c \
  usbd_usr.c


##############################################################################

vpath %.c src

CFLAGS  = -g -O2 -Wall
CFLAGS += -mlittle-endian -mthumb -mcpu=cortex-m0 -march=armv6s-m
CFLAGS += -ffreestanding -nostdlib
CFLAGS += -I./inc -I../cmsis_boot -I../cmsis_core -I./STM32_USB_Device_Driver/inc
CFLAGS += -I./STM32_USB_Device_Library/Class/cdc/inc -I./STM32_USB_Device_Library/Core/inc
CFLAGS += -I../cmsis_lib/include

OBJS = $(SRCS:.c=.o)

.PHONY: all

all: $(LIBNAME)

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $^

$(LIBNAME): $(OBJS)
	$(AR) -r $@ $(OBJS)

clean:
	rm -f $(OBJS) $(LIBNAME)
