#include "pwmout.h"

extern volatile uint8_t move_start;
extern volatile uint8_t move_count;
extern Move move_buffer[MOVE_SIZE];

void initialize_pwm(void) {
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

  GPIO_InitTypeDef gp;
  gp.GPIO_Mode = GPIO_Mode_AF;
  gp.GPIO_Speed = GPIO_Speed_50MHz;
  gp.GPIO_OType = GPIO_OType_PP;
  gp.GPIO_PuPd = GPIO_PuPd_NOPULL;

  // X-Coarse
  gp.GPIO_Pin = GPIO_Pin_15;
  GPIO_Init(GPIOA, &gp);
  
  // X-Fine
  gp.GPIO_Pin = GPIO_Pin_3;
  GPIO_Init(GPIOB, &gp);

  // Y-Coarse
  gp.GPIO_Pin = GPIO_Pin_10;
  GPIO_Init(GPIOB, &gp);

  // Y-Fine
  gp.GPIO_Pin = GPIO_Pin_11;
  GPIO_Init(GPIOB, &gp);
 
  // Laser Power
  gp.GPIO_Pin = GPIO_Pin_4;
  GPIO_Init(GPIOB, &gp);
       
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

  GPIO_PinAFConfig(GPIOA, GPIO_PinSource15, GPIO_AF_2); // GPIO_AF_2 = TIM2
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource3, GPIO_AF_2); // GPIO_AF_2 = TIM2
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_2); // GPIO_AF_2 = TIM2
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_2); // GPIO_AF_2 = TIM2

  GPIO_PinAFConfig(GPIOB, GPIO_PinSource4, GPIO_AF_1); // GPIO_AF_2 = TIM3 for PB4

  TIM_TimeBaseInitTypeDef ti;
  ti.TIM_Prescaler = 0;
  ti.TIM_CounterMode = TIM_CounterMode_Up;
  ti.TIM_Period = 256;
  ti.TIM_ClockDivision = TIM_CKD_DIV1;
  ti.TIM_RepetitionCounter = 0;
  TIM_TimeBaseInit(TIM2, &ti);
  TIM_Cmd(TIM2, ENABLE);

  TIM_TimeBaseInit(TIM3, &ti);
  TIM_Cmd(TIM3, ENABLE);

  TIM_OCInitTypeDef oc = {0,};
  oc.TIM_OCMode = TIM_OCMode_PWM1;
  oc.TIM_OutputState = TIM_OutputState_Enable;
  oc.TIM_OutputNState = 0;
  oc.TIM_Pulse = 0;
  oc.TIM_OCPolarity = TIM_OCPolarity_High;
  oc.TIM_OCNPolarity = 0;
  oc.TIM_OCIdleState = 0;
  oc.TIM_OCNIdleState = 0;

  TIM_OC1Init(TIM2, &oc);
  TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Enable);

  TIM_OC2Init(TIM2, &oc);
  TIM_OC2PreloadConfig(TIM2, TIM_OCPreload_Enable);

  TIM_OC3Init(TIM2, &oc);
  TIM_OC3PreloadConfig(TIM2, TIM_OCPreload_Enable);

  TIM_OC4Init(TIM2, &oc);
  TIM_OC4PreloadConfig(TIM2, TIM_OCPreload_Enable);

  // Laser power
  TIM_OC1Init(TIM3, &oc);
  TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);
}

void update_pwm(void) {
  int32_t xout;
  int32_t yout;
  uint32_t laserpower;

  if (move_count > 0) {
    xout = move_buffer[move_start].x;
    yout = move_buffer[move_start].y;
    laserpower = move_buffer[move_start].laserPower;

    // Position
    TIM_SetCompare1(TIM2, xout >> 8);
    TIM_SetCompare2(TIM2, xout & 0xFF);
    TIM_SetCompare3(TIM2, yout >> 8);
    TIM_SetCompare4(TIM2, yout & 0xFF);
    
    // Laser Power
    TIM_SetCompare1(TIM3, laserpower & 0xFF);

    move_start = (move_start + 1) % MOVE_SIZE;
    move_count--;
  } else {
    // Turn off laser if we don't have any pending move commands
    TIM_SetCompare1(TIM3, 0);
  }
}
