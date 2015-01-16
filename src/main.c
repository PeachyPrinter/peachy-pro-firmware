#include "stm32f0xx_conf.h"
#include "stm32f0xx_tim.h"
#include "stm32f0xx_gpio.h"

#include "usbd_cdc_core.h"
#include "usbd_usr.h"

#include "iolib.h"
#include "serialio.h"
#include "pwmout.h"

USB_CORE_HANDLE  USB_Device_dev ;

void USB_Init(void)
{
  USBD_Init(&USB_Device_dev,
            &USR_desc,
            &USBD_CDC_cb,
            &USR_cb);
}

void USB_LP_IRQHandler(void)
{
  USB_Istr();
}


void SysTick_Handler(void) {
  update_pwm();
}


int main(void)
{
  
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
  
  SysTick_Config(SystemCoreClock / 20);
  
  USB_Init();
  
  while(1) {
    serialio_feed();
  }
}
