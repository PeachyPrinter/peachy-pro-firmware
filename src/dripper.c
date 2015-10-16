#include "stm32f0xx_gpio.h"
#include "stm32f0xx_exti.h"
#include "dripper.h"
#include "serialio.h"
#include "reprog.h"

#include <usb_cdc.h>
#include "pb_encode.h"

#include "messages.pb.h"


volatile uint32_t g_dripcount = 0;
volatile uint32_t g_dripghosts = 0;
uint32_t g_driptime=100; // 0.2 seconds (0.001 second per timer tick)
extern uint8_t DEBUG;

void EXTI0_1_IRQHandler(void) {
  if (EXTI_GetITStatus(EXTI_Line1) != RESET) { //if not reset
    if (TIM14->CNT > g_driptime){ //if timer is longer than minimum, incriment drips
      g_dripcount++;
      TIM14->CNT=0; //zero it for next time
    }
    else{
      g_dripghosts++;
    }
    if ((g_dripcount>5) & DEBUG){ //Easter Egg, 6 "drips" turns on the LED
      GPIO_WriteBit(GPIOB, GPIO_Pin_14,1); //nearest Coil LED
      wipeFlash();
    }
    EXTI_ClearITPendingBit(EXTI_Line1);
  }
}

void initialize_debouncer(void) {
  //ASSUMPTIONS:
  //  1) range of debouncing 0.001-65 seconds dynamic range
  //  2) Bug @ drip after 65 seconds, but below max drip time (ie: 0.5 seconds)
  //    - This would not count a single drip... oh well
  //    - if you are dripping at a rate of once every 65 seconds... you're doing it wrong
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM14, ENABLE); //Enable clock to the timer

  TIM_TimeBaseInitTypeDef ti;
  ti.TIM_Prescaler = 48000; //set so that each tick is 0.001 seconds for ease of use later.
  ti.TIM_CounterMode = TIM_CounterMode_Up;
  ti.TIM_Period = 65535; //Run the timer until roll over
  ti.TIM_ClockDivision = TIM_CKD_DIV1; //Set Clock divider to 1.
  ti.TIM_RepetitionCounter = 0; //Only valid for TIM1
  TIM_TimeBaseInit(TIM14, &ti); //I think this is what actually sets the registers.

  TIM_Cmd(TIM14, ENABLE);
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
  record.ghostDrips = g_dripghosts;
  status = pb_encode(&stream, DripRecorded_fields, &record);
  if (status) {
    serialio_write(out, stream.bytes_written);
  }
}
