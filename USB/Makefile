CC=arm-none-eabi-gcc
AR=arm-none-eabi-ar

STD_PERIPH_LIB = ../Libraries

CFLAGS  = -g -O2 -Wall
CFLAGS += -mlittle-endian -mthumb -mcpu=cortex-m0 -march=armv6s-m
CFLAGS += -ffreestanding -nostdlib

CFLAGS += -I inc -I $(STD_PERIPH_LIB) -I $(STD_PERIPH_LIB)/CMSIS/Device/ST/STM32F0xx/Include
CFLAGS += -I $(STD_PERIPH_LIB)/CMSIS/Include -I $(STD_PERIPH_LIB)/STM32F0xx_StdPeriph_Driver/inc
CFLAGS += -include $(STD_PERIPH_LIB)/stm32f0xx_conf.h

all: libstmusb.a

CFLAGS += -Iinc

SRCS = usb_bsp.c  usb_core.c  usb_dcd.c  usbd_cdc_core.c \
  usbd_cdc_vcp.c  usb_dcd_int.c  usbd_desc.c  usbd_pwr.c  usbd_usr.c
OBJS = $(SRCS:.c=.o)

vpath %.c src

libstmusb.a: $(OBJS)
	$(AR) -r $@ $(OBJS)

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $^

clean:
	rm -f $(OBJS) libstmusb.a
