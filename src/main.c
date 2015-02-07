#include "stm32f0xx_conf.h"
#include "stm32f0xx_tim.h"
#include "stm32f0xx_gpio.h"

#include "iolib.h"
#include "serialio.h"
#include "pwmout.h"

#include <usb_core.h>
#include <usb_cdc.h>

static unsigned char* msg = (unsigned char*)"Hello\r\n";

void SysTick_Handler(void) {
  static int tick_count = 0;
  if (tick_count > 2000) {
    if(!WouldTxBlock()) { QueueTx(msg, 7); }
    tick_count = 0;
  }
  tick_count++;

  update_pwm();
}


int main(void)
{
  USB_Start();
  
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
  
  // Set Up PA4 and PA5 as debug pins
  GPIO_InitTypeDef gp;
  gp.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
  gp.GPIO_Mode = GPIO_Mode_OUT;
  gp.GPIO_Speed = GPIO_Speed_50MHz;
  gp.GPIO_OType = GPIO_OType_PP;
  gp.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA, &gp);
  
  initialize_pwm();
  
  SysTick_Config(SystemCoreClock / 2000);
  while(1) {
    serialio_feed();
  }
}
