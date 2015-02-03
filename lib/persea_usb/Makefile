CC=arm-none-eabi-gcc
AR=arm-none-eabi-ar

LIBNAME = libpersea-usb.a

SRCS = usb_core.c usb_control.c usb_cdc.c

########################################

vpath %.c src

CFLAGS  = -g -O0 -Wall -Werror
CFLAGS += -mlittle-endian -mthumb -mcpu=cortex-m0 -march=armv6s-m
CFLAGS += -ffreestanding -nostdlib
CFLAGS += -I./inc -I../cmsis_boot -I../cmsis_core -I../cmsis_lib/include

OBJS = $(SRCS:.c=.o)

.PHONY: all clean

all: $(LIBNAME)

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $^

$(LIBNAME): $(OBJS)
	$(AR) -r $@ $(OBJS)

clean:
	rm -f $(OBJS) $(LIBNAME)

