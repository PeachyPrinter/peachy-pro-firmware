#include "pwmout.h"

static volatile int g_xout = 512;
static volatile int g_yout = 512;

void initialize_pwm(void) {
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

  GPIO_InitTypeDef gp;
  gp.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_2;
  gp.GPIO_Mode = GPIO_Mode_AF;
  gp.GPIO_Speed = GPIO_Speed_50MHz;
  gp.GPIO_OType = GPIO_OType_PP;
  gp.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA, &gp);
        
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource0, GPIO_AF_2); // GPIO_AF_2 = TIM2
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_2); // GPIO_AF_2 = TIM2

  TIM_TimeBaseInitTypeDef ti;
  ti.TIM_Prescaler = 0;
  ti.TIM_CounterMode = TIM_CounterMode_Up;
  ti.TIM_Period = 1024;
  ti.TIM_ClockDivision = TIM_CKD_DIV1;
  ti.TIM_RepetitionCounter = 0;
  TIM_TimeBaseInit(TIM2, &ti);
  TIM_Cmd(TIM2, ENABLE);

  TIM_OCInitTypeDef oc = {0,};
  oc.TIM_OCMode = TIM_OCMode_PWM1;
  oc.TIM_OutputState = TIM_OutputState_Enable;
  oc.TIM_OutputNState = 0;
  oc.TIM_Pulse = 150;
  oc.TIM_OCPolarity = TIM_OCPolarity_High;
  oc.TIM_OCNPolarity = 0;
  oc.TIM_OCIdleState = 0;
  oc.TIM_OCNIdleState = 0;

  TIM_OC1Init(TIM2, &oc);
  TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Enable);

  TIM_OC3Init(TIM2, &oc);
  TIM_OC3PreloadConfig(TIM2, TIM_OCPreload_Enable);
}

void update_pwm(void) {
  static int dir = 1;
  
  g_xout += dir;
  g_yout += dir;

  if (g_xout > 1023) { 
    dir = -1; 
  }
  if (g_xout < 1) { dir = 1; }

  TIM_SetCompare1(TIM2, g_xout);
  TIM_SetCompare3(TIM2, g_yout);
}
