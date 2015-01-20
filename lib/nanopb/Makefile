CC=arm-none-eabi-gcc
AR=arm-none-eabi-ar

LIBNAME = libnanopb.a

SRCS = pb_encode.c pb_decode.c pb_common.c

##############################################################################

CFLAGS  = -g -O2 -Wall
CFLAGS += -mlittle-endian -mthumb -mcpu=cortex-m0 -march=armv6s-m
CFLAGS += -ffreestanding -nostdlib

OBJS = $(SRCS:.c=.o)

.PHONY: all clean

all: $(LIBNAME)

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $^

$(LIBNAME): $(OBJS)
	$(AR) -r $@ $(OBJS)

clean:
	rm -f $(OBJS) $(LIBNAME)

