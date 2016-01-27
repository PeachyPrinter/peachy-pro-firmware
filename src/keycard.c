#include "stm32f0xx_conf.h"
#include "stm32f0xx_tim.h"
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_exti.h"

#include "hwaccess.h"
#include "led_override.h"
#include "keycard.h"
#include "ADClockout.h"


uint8_t g_key_state=KEY_MISSING;
uint32_t g_key_code=0;
uint32_t g_key_pos=0;
uint32_t g_key_beeps=0;
uint32_t g_key_beep_counter=0;

extern uint8_t g_adc_state;

void setup_keycard(void){

  //Initialize GPIO's PF0 && PF1 (INPUTS)
  //PF0 Edge triggered interrupt (key clock)
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
  //ASSUMPTION: The NVIC is enabled for the exti lines 0-1 in the dripper

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM16, ENABLE);//Initialize key timeout timer
  TIM_TimeBaseInitTypeDef ti;
  ti.TIM_Prescaler = 24000; //Should work 48MHz/2==24MHz, pre-scaler of 24k gives me 1ms ticks?
  ti.TIM_CounterMode = TIM_CounterMode_Up;
  ti.TIM_Period = KEY_TIMEOUT; //in ms for ease of use
  ti.TIM_ClockDivision = TIM_CKD_DIV2;
  ti.TIM_RepetitionCounter = 0;
  TIM_TimeBaseInit(TIM16, &ti);

  TIM_ITConfig(TIM16,TIM_IT_Update,ENABLE);
  TIM_Cmd(TIM16, ENABLE);

  NVIC_InitTypeDef nvic;
  nvic.NVIC_IRQChannel = TIM16_IRQn;
  nvic.NVIC_IRQChannelPriority = 0x00;
  nvic.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&nvic);

}
void TIM16_IRQHandler(void){
  if (TIM_GetITStatus(TIM16,TIM_IT_Update) != RESET){
    if (g_key_state==KEY_CHECKING){
      g_key_pos=0;
      g_key_state=KEY_MISSING;
      g_key_code=0;
    }
    // If the data sensor sees dark AND key is valid -> turn off laser
    else if ((GPIO_ReadInputDataBit(GPIOF, GPIO_Pin_1)) & (g_key_state==KEY_VALID)){
      g_key_pos=0;
      g_key_state=KEY_MISSING;
      g_key_code=0;
    }
    TIM_ClearITPendingBit(TIM16,TIM_IT_Update);
  }
}

void update_key_state(void){
  if ((g_key_state==KEY_VALID) & (g_adc_state==ADC_LOCKOUT_VALID)){
    setCornerLed(1);
  }
  else if (g_key_state==KEY_CHECKING){
    setCornerLed(GPIO_ReadInputDataBit(GPIOF, GPIO_Pin_0)); //LED on clock sensor sees dark
	}
  else{
    setCornerLed(0);
	}
}

void read_key(void){
  key_check(GPIO_ReadInputDataBit(GPIOF, GPIO_Pin_1));
}

void key_check(uint8_t key_bit){

  //Sensor saw light, Key gone!
  if (g_key_state==KEY_VALID){
    g_key_state=KEY_MISSING;
    g_key_pos=0;
    g_key_code=0;
		stop_led_steps(L_LONG_SPIN);
  }
  //else, if we need more bits, add em in!
  else if ((g_key_state==KEY_CHECKING) || (g_key_state==KEY_MISSING)){
    TIM16->CNT=0; //Zero the count for timeout per bit
    key_bit=key_bit & 0x1;//Make sure it's a single bit
    g_key_code+=key_bit<<g_key_pos;
    g_key_pos++;
    g_key_state=KEY_CHECKING;
		stop_led_steps(L_LONG_SPIN);
  }

  //If we have 8 bits of the code, check it now
  if (g_key_pos==KEY_LENGTH){
    if (g_key_code==KEY_MASTER){
      g_key_state=KEY_VALID;

      //Let the user know ALL THE WAYS
      g_key_beeps=KEY_TONE_NUM_BEEPS;
      g_key_beep_counter=KEY_TONE_LENGTH;
      play_long_spin();
    }
    else{
      g_key_state=KEY_MISSING;
    }
    g_key_pos=0;
    g_key_code=0;
  }
}
