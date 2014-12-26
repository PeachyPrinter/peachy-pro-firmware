#include "stm32f0xx_conf.h"
#include "stm32f0xx_tim.h"
#include "stm32f0xx_gpio.h"

static volatile int g_xout = 0;
static volatile int g_yout = 128;

void SysTick_Handler(void) {
  g_xout += 1;
  g_yout += 1;

  if (g_xout > 255) { g_xout = 0; }
  if (g_yout > 255) { g_yout = 0; }

  TIM_SetCompare1(TIM2, g_xout);
  TIM_SetCompare2(TIM2, g_yout);
}

void initialize_tim2(void) {
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource0, GPIO_AF_2); // GPIO_AF_2 = TIM2
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource1, GPIO_AF_2); // GPIO_AF_2 = TIM2

  TIM_TimeBaseInitTypeDef ti;
  ti.TIM_Prescaler = 0;
  ti.TIM_CounterMode = TIM_CounterMode_Up;
  ti.TIM_Period = 256;
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

  oc.TIM_Pulse = 64;

  TIM_OC2Init(TIM2, &oc);
  TIM_OC2PreloadConfig(TIM2, TIM_OCPreload_Enable);
}

int main(void)
{
	volatile int x = 0;

        //RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
        RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
        __asm("dsb");

        GPIO_InitTypeDef gp;
        gp.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
        gp.GPIO_Mode = GPIO_Mode_AF;
        gp.GPIO_Speed = GPIO_Speed_50MHz;
        gp.GPIO_OType = GPIO_OType_PP;
        gp.GPIO_PuPd = GPIO_PuPd_NOPULL;
        GPIO_Init(GPIOA, &gp);

        gp.GPIO_Pin = GPIO_Pin_2;
        gp.GPIO_Mode = GPIO_Mode_OUT;
        gp.GPIO_Speed = GPIO_Speed_10MHz;
        gp.GPIO_OType = GPIO_OType_PP;
        gp.GPIO_PuPd = GPIO_PuPd_NOPULL;
        GPIO_Init(GPIOA, &gp);

        initialize_tim2();

        SysTick_Config(SystemCoreClock / 100);

	while(1) {
          GPIOA->ODR ^= 0x04;
        }
}
