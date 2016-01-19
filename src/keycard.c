#include "stm32f0xx_conf.h"
#include "stm32f0xx_tim.h"
#include "stm32f0xx_gpio.h"

#define KEY_VALID 2
#define KEY_CHECKING 1
#define KEY_MISSING 0
#define KEY_MASTER 0b11001001
#define KEY_LENGTH 8

uint32_t g_key_state=KEY_MISSING;
uint32_t g_key_code=0;
uint32_t g_key_pos=0;

void setup_keycard(void){

  //Initialize GPIO's PF0 && PF1 (INPUTS)
  GPIO_InitTypeDef gp;
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOF, ENABLE);

  gp.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
  gp.GPIO_Mode = GPIO_Mode_IN;
  gp.GPIO_Speed = GPIO_Speed_50MHz;
  gp.GPIO_OType = GPIO_OType_OD;
  gp.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(GPIOF, &gp);

  //Initialize External Interrupts on PF0, both edges
  // Rising edge KEY INPUT
  // Falling Edge nothing - unless g_key_state==VALID
  //  then g_key_state==KEY_MISSING

  //Setup TIM16 as key timeout
  // Set roll over to ~2 seconds.
  //   IF roll over then g_key_state==KEY_MISSING
  //      g_key_code=0
  //      g_key_pos=0
}

//TODO: Fix this function, it's still a copy paste
void EXTI0_1_IRQHandler(void) {
  if (EXTI_GetITStatus(EXTI_Line1) != RESET) { //if not reset
    if (TIM14->CNT > g_driptime){ //if timer is longer than minimum, incriment drips
      g_dripcount++;
      TIM14->CNT=0; //zero it for next time
    }
    else{
      g_dripghosts++;
    }
    if ((g_dripcount>5) & g_debug){ //g_debug, 6 "drips" turns on the LED for check if it works
      setCoilLed(1);
    }
    EXTI_ClearITPendingBit(EXTI_Line1);
  }
}

void key_check(uint32_t key_bit){

  //If we need more bits, add em in!
  if (g_key_state==KEY_CHECKING || g_key_state==KEY_MISSING){
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
