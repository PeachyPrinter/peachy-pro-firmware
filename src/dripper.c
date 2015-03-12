#include "stm32f0xx_gpio.h"
#include "stm32f0xx_exti.h"
#include "dripper.h"
#include "serialio.h"

#include <usb_cdc.h>
#include "pb_encode.h"

#include "messages.pb.h"

volatile uint32_t g_dripcount = 0;

void EXTI0_1_IRQHandler(void) {
  if (EXTI_GetITStatus(EXTI_Line1) != RESET) {
    g_dripcount++;
    EXTI_ClearITPendingBit(EXTI_Line1);
  }
}

void initialize_dripper(void) {
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

  // Turn on the GPIO pins we're going to use

  GPIO_InitTypeDef gp;
  gp.GPIO_Pin = GPIO_Pin_0;
  gp.GPIO_Mode = GPIO_Mode_OUT;
  gp.GPIO_Speed = GPIO_Speed_2MHz;
  gp.GPIO_OType = GPIO_OType_PP;
  gp.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA, &gp);
  
  GPIO_WriteBit(GPIOA, GPIO_Pin_0, 1);

  gp.GPIO_Pin = GPIO_Pin_1;
  gp.GPIO_Mode = GPIO_Mode_IN;
  gp.GPIO_PuPd = GPIO_PuPd_DOWN;
  GPIO_Init(GPIOA, &gp);

  // Configure the interrupt on GPIO A, pin 1  
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource1);

  EXTI_InitTypeDef exti;
  exti.EXTI_Line = EXTI_Line1;
  exti.EXTI_Mode = EXTI_Mode_Interrupt;
  exti.EXTI_Trigger = EXTI_Trigger_Rising;
  exti.EXTI_LineCmd = ENABLE;
  EXTI_Init(&exti);
  
  NVIC_InitTypeDef nvic;
  nvic.NVIC_IRQChannel = EXTI0_1_IRQn;
  nvic.NVIC_IRQChannelPriority = 0x00;
  nvic.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&nvic);
}

void send_updated_drip_count(void) {
  uint8_t out[16];
  uint8_t type;
  pb_ostream_t stream = pb_ostream_from_buffer(out, sizeof(out));
  bool status;
  DripRecorded record;

  type = DRIP_RECORDED;
  pb_write(&stream, &type, 1);

  record.drips = g_dripcount;

  status = pb_encode(&stream, DripRecorded_fields, &record);
  if (status) {
    serialio_write(out, stream.bytes_written);
  }
}
