#include "stm32f0xx_conf.h"
#include "stm32f0xx_tim.h"
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_exti.h"

#include "keycard.h"
#include "hwaccess.h"

uint32_t g_key_state=KEY_MISSING;
uint32_t g_key_code=0;
uint32_t g_key_pos=0;

uint32_t g_debugger=0;

void setup_keycard(void){

  //Initialize GPIO's PF0 && PF1 (INPUTS)
  //PF0 Edge triggered interrupt
  //PF1 simple GPIO read (key data)
  GPIO_InitTypeDef gp;
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOF, ENABLE);

  gp.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
  gp.GPIO_Mode = GPIO_Mode_IN;
  gp.GPIO_Speed = GPIO_Speed_2MHz;
  gp.GPIO_OType = GPIO_OType_PP;
  gp.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOF, &gp);

  //Initialize External Interrupts on PF0
  // Falling edge KEY CLOCK (PF0)
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOF, EXTI_PinSource0);

  EXTI_InitTypeDef exti;
  exti.EXTI_Line = EXTI_Line0;
  exti.EXTI_Mode = EXTI_Mode_Interrupt;
  exti.EXTI_Trigger = EXTI_Trigger_Falling;
  exti.EXTI_LineCmd = ENABLE;
  EXTI_Init(&exti);
  //ASSUMPTION: The NVIC is enabled for the lines 0-1 in the dripper

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM16, ENABLE);//Initialize key timeout timer
  TIM_TimeBaseInitTypeDef ti;
  ti.TIM_Prescaler = 24000; //Should work 48MHz/4==12MHz, pre-scaler of 12k gives me 1ms ticks?
  ti.TIM_CounterMode = TIM_CounterMode_Up;
  ti.TIM_Period = 1000; //in ms for ease of use
  ti.TIM_ClockDivision = TIM_CKD_DIV4;
  ti.TIM_RepetitionCounter = 0;
  TIM_TimeBaseInit(TIM16, &ti);

  TIM_ITConfig(TIM16,TIM_IT_Update,ENABLE);
  TIM_Cmd(TIM16, ENABLE);

  //TODO: enable timer interrupt and zero key
  NVIC_InitTypeDef nvic;
  nvic.NVIC_IRQChannel = TIM16_IRQn;
  nvic.NVIC_IRQChannelPriority = 0x00;
  nvic.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&nvic);


  //Setup TIM16 as key timeout
  // Set roll over to ~2 seconds.
  //   IF roll over then g_key_state==KEY_MISSING
  //      g_key_code=0
  //      g_key_pos=0
}
void TIM16_IRQHandler(void){
  if (TIM_GetITStatus(TIM16,TIM_IT_Update) != RESET){
    if (g_key_state==KEY_CHECKING){
      g_key_pos=0;
      g_key_state=KEY_MISSING;
      g_key_code=0;
    }
    TIM_ClearITPendingBit(TIM16,TIM_IT_Update);
    if (g_key_state==KEY_VALID){
      if (!GPIO_ReadInputDataBit(GPIOF, GPIO_Pin_1)){
        g_key_pos=0;
        g_key_state=KEY_MISSING;
        g_key_code=0;
      }
    }
  }
}

void update_key_state(void){
  if (g_key_state==KEY_VALID)
    setCornerLed(1);
  else
    setCornerLed(0);
}

void read_key(void){
  key_check(GPIO_ReadInputDataBit(GPIOF, GPIO_Pin_1));
}

void key_check(uint32_t key_bit){

  //If we need more bits, add em in!
  if ((g_key_state==KEY_CHECKING) || (g_key_state==KEY_MISSING)){
    TIM16->CNT=0; //Zero the count for timeout per bit
    key_bit=key_bit & 0x1;//Make sure it's a single bit
    g_key_code+=key_bit<<g_key_pos;
    g_key_pos++;
    g_key_state=KEY_CHECKING;
  }

  //If we have 8 bits of the code, check it now
  if (g_key_pos==KEY_LENGTH){
    if (g_key_code==KEY_MASTER){
      g_key_state=KEY_VALID;
    }
    else{
      g_key_state=KEY_MISSING;
    }
    g_key_pos=0;
    g_key_code=0;
  }
}
