CC=arm-none-eabi-gcc
AR=arm-none-eabi-ar

LIBNAME = libstm32f0.a

SRCS = stm32f0xx_adc.c \
  stm32f0xx_crs.c \
  stm32f0xx_dma.c \
  stm32f0xx_exti.c \
  stm32f0xx_flash.c \
  stm32f0xx_gpio.c \
  stm32f0xx_i2c.c \
  stm32f0xx_misc.c \
  stm32f0xx_pwr.c \
  stm32f0xx_rcc.c \
  stm32f0xx_rtc.c \
  stm32f0xx_spi.c \
  stm32f0xx_syscfg.c \
  stm32f0xx_tim.c \
  stm32f0xx_usart.c

vpath %.c source

CFLAGS  = -g -O2 -Wall
CFLAGS += -mlittle-endian -mthumb -mcpu=cortex-m0 -march=armv6s-m
CFLAGS += -ffreestanding -nostdlib
CFLAGS += -Iinclude -I../cmsis_boot -I../cmsis_core
#CFLAGS += -includestm32f0xx_conf.h -ICMSIS/Include -ICMSIS/Device/ST/STM32F0xx/Include -ISTM32F0xx_StdPeriph_Driver/inc

OBJS = $(SRCS:.c=.o)

.PHONY: all

all: $(LIBNAME)

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $^

$(LIBNAME): $(OBJS)
	$(AR) -r $@ $(OBJS)

clean:
	rm -f $(OBJS) $(LIBNAME)
