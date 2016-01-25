/*
 * led_override.c
 *
 *  Created on: Jan 22, 2016
 *      Author: green
 */
#include "stm32f0xx_conf.h"
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_rcc.h"

#include "hwaccess.h"
#include "led_override.h"
#include "overrides.h"

extern volatile uint8_t g_led_control;

//Led Locations:
//[8,4]
//[1,2]
//add numbers to together to turn multiple on
uint8_t l_two_spin[9]={1,2,4,8,1,2,4,8,0};
uint8_t l_three_spin[52]={1,2,4,8,1,2,4,8,1,2,4,8,0,1,2,4,8,1,2,4,8,1,2,4,8,0,1,2,4,8,1,2,4,8,1,2,4,8,0,1,2,4,8,1,2,4,8,1,2,4,8,0};
uint8_t l_left_right[4]={9,6,9,6};
uint8_t* led_steps;

uint8_t g_pattern_pos=0;

void play_spin(){
  start_led_steps(l_two_spin,9);
}

void start_led_steps(uint8_t new_steps[], uint8_t num_steps){
  led_steps=new_steps;
  g_pattern_pos=num_steps;
}

void next_led_step(){
  //g_song_pos must be >0 to enter this function
  //guarenteed in TIM17 interrupt handler
  turn_leds_on(led_steps[g_pattern_pos-1]);
  g_pattern_pos--;
}

void turn_leds_on(uint8_t leds){
  GPIO_WriteBit(GPIOB, GPIO_Pin_12, leds&0x1);
  GPIO_WriteBit(GPIOB, GPIO_Pin_13, (leds&0x2)>>1);
  GPIO_WriteBit(GPIOB, GPIO_Pin_14, (leds&0x4)>>2);
  GPIO_WriteBit(GPIOB, GPIO_Pin_15, (leds&0x8)>>3);
}

void initialize_led_override(void){
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM17, ENABLE);//Initialize key timeout timer

  TIM_TimeBaseInitTypeDef ti;
  ti.TIM_Prescaler = 24000; //Should work 48MHz/2==24MHz, pre-scaler of 24k gives me 1ms ticks?
  ti.TIM_CounterMode = TIM_CounterMode_Up;
  ti.TIM_Period = LED_TIME_STEP; //in ms for ease of use
  ti.TIM_ClockDivision = TIM_CKD_DIV2;
  ti.TIM_RepetitionCounter = 0;
  TIM_TimeBaseInit(TIM17, &ti);

  TIM_ITConfig(TIM17,TIM_IT_Update,ENABLE);
  TIM_Cmd(TIM17, ENABLE);

  NVIC_InitTypeDef nvic;
  nvic.NVIC_IRQChannel = TIM17_IRQn;
  nvic.NVIC_IRQChannelPriority = 0x00;
  nvic.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&nvic);
}
void TIM17_IRQHandler(void){
  if (TIM_GetITStatus(TIM17,TIM_IT_Update) != RESET){
    if (g_pattern_pos!=0){
      g_led_control=0; //turn off the normal led control while we play
      next_led_step();
    }
    else{
      g_led_control=1; //led control as normal
    }

  }
  TIM_ClearITPendingBit(TIM17,TIM_IT_Update);
}
