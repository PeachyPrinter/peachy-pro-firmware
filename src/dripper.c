#include "stm32f0xx_gpio.h"
#include "stm32f0xx_exti.h"
#include "stm32f0xx_conf.h"
#include "dripper.h"
#include "serialio.h"
#include "reprog.h"
#include "hwaccess.h"
#include "keycard.h"

#include <usb_cdc.h>
#include "pb_encode.h"

#include "messages.pb.h"

extern volatile uint32_t tick;

volatile uint32_t g_dripcount = 0;
volatile uint32_t g_dripghosts = 0;
uint16_t g_drip_toggle_count=0;
uint32_t g_next_drip_tick=0;
uint32_t g_start_drip_tick=0;
uint16_t g_drip_state=DRIPPER_IDLE;

void EXTI0_1_IRQHandler(void) {
  //Dripper
  setCoilLed(1);
  if (EXTI_GetITStatus(EXTI_Line1) != RESET) { //if it's the dripper that called this

    //Start off from idle state
    if (g_drip_state==DRIPPER_IDLE){
      //IF IDLE, start a new count
      g_drip_toggle_count=1;
      g_start_drip_tick=tick;
      g_next_drip_tick=g_start_drip_tick+DRIP_TICKS_PER_TOGGLE_MAX;
      g_drip_state=DRIPPER_CHECKING;
    }
    else if (g_drip_state==DRIPPER_DEAD){
      //if timer is longer than minimum, reset state
      if (TIM14->CNT > DRIP_DEADTIME){
        g_drip_state=DRIPPER_IDLE;
      }
    }
    else if (g_drip_state == DRIPPER_CHECKING){ //the else/if is redundant for now

      //Roll over case... once every 2^32 * 0.25ms this thing MIGHT miss a drip.
      // That's like once every 12 days... one drip every 12 days
      // Keep in mind that had to be hit DURING the timeout period
      // (~10ms window every 12 days) ... I hope this happens at least once
      //.
      //.... I'm glad I wrote this .....

      /*
      if (g_next_drip_tick<g_start_drip_tick){ //Roll over case..
        //The tick can be less than next drip, OR more than the start drip
        if ((tick<g_next_drip_tick)|(tick>g_start_drip_tick)){
          g_drip_toggle_count++;
        }
        else{ //ghost edge, kick this back to idle
          g_drip_state=DRIPPER_IDLE;
        }
      }
      //The normal case
      else*/ if (tick<g_next_drip_tick){
        g_drip_toggle_count++; //Got a good toggle
        g_start_drip_tick=tick; //Reset the ticks for next one
        g_next_drip_tick=g_start_drip_tick+DRIP_TICKS_PER_TOGGLE_MAX;
      }
      else{ //else this was a ghost edge, keep waiting for a drip
        g_drip_state=DRIPPER_IDLE;
      }

      //If we got enough drip toggles, we got a verified drip!
      if (g_drip_toggle_count>DRIP_TOGGLES){
        g_dripcount++; //GOOD DRIP
        TIM14->CNT=0;  //zero the deadtime counter
        g_drip_state=DRIPPER_DEAD;
      }
    } //DRIPPER_CHECKING
    EXTI_ClearITPendingBit(EXTI_Line1);
  }
  //Key Clock
  else if (EXTI_GetITStatus(EXTI_Line0) != RESET){
    read_key(); //Read the key bits
    EXTI_ClearITPendingBit(EXTI_Line0);
  }
  setCoilLed(0);
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
  TIM_TimeBaseInit(TIM14, &ti); //What actually sets the registers.

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

  initialize_debouncer();
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
