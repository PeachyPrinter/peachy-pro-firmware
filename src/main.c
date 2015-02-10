#include "stm32f0xx_conf.h"
#include "stm32f0xx_tim.h"
#include "stm32f0xx_gpio.h"

#include "iolib.h"
#include "serialio.h"
#include "pwmout.h"

#include <usb_core.h>
#include <usb_cdc.h>

#include <stm32f0xx_i2c.h>

static unsigned char* msg = (unsigned char*)"Hello\r\n";
static volatile uint32_t tick = 0;

void delay_ms(int ms) {
  uint32_t end = tick + (ms*2);
  while(tick < end);
}

void SysTick_Handler(void) {
  tick += 1;
  static int tick_count = 0;
  if (tick_count > 2000) {
    if(!WouldTxBlock()) { QueueTx(msg, 7); }
    tick_count = 0;
  }
  tick_count++;

  update_pwm();
}

volatile void* i2c_address = I2C1;

void i2c_trigger_capture(void) {
  while(I2C_GetFlagStatus(I2C1, I2C_ISR_BUSY) != RESET);

  I2C_TransferHandling(I2C1, 0x90, 3, I2C_AutoEnd_Mode, I2C_Generate_Start_Write);
  I2C_ClearFlag(I2C1, I2C_ICR_NACKCF | I2C_ICR_STOPCF);

  I2C_SendData(I2C1, 0x01);

  while(I2C_GetFlagStatus(I2C1, I2C_FLAG_TXIS) == RESET);

  while(I2C_GetFlagStatus(I2C1, I2C_FLAG_TXIS) == RESET);
  I2C_SendData(I2C1, 0x83);

  while(I2C_GetFlagStatus(I2C1, I2C_FLAG_TXIS) == RESET);
  I2C_SendData(I2C1, 0x83);

  while(I2C_GetFlagStatus(I2C1, I2C_ISR_STOPF) == RESET);
  I2C_ClearFlag(I2C1, I2C_ICR_STOPCF);
}

void i2c_read_values(void) {
  volatile uint8_t i2crx[2] = { 0, 0 };

  I2C_TransferHandling(I2C1, 0x90, 1, I2C_AutoEnd_Mode, I2C_Generate_Start_Write);
  I2C_ClearFlag(I2C1, I2C_ICR_NACKCF | I2C_ICR_STOPCF);
  I2C_GenerateSTART(I2C1, ENABLE);

  while(I2C_GetFlagStatus(I2C1, I2C_FLAG_TXIS) == RESET);
  I2C_SendData(I2C1, 0x00);
  while(I2C_GetFlagStatus(I2C1, I2C_ISR_STOPF) == RESET);
  I2C_ClearFlag(I2C1, I2C_ICR_STOPCF);

  I2C_TransferHandling(I2C1, 0x90, 2, I2C_AutoEnd_Mode, I2C_Generate_Start_Read);
  while(I2C_GetFlagStatus(I2C1, I2C_ISR_RXNE) == RESET);
  i2crx[0] = I2C_ReceiveData(I2C1);
  while(I2C_GetFlagStatus(I2C1, I2C_ISR_RXNE) == RESET);
  i2crx[1] = I2C_ReceiveData(I2C1);

  while(I2C_GetFlagStatus(I2C1, I2C_ISR_STOPF) == RESET);
  I2C_ClearFlag(I2C1, I2C_ICR_STOPCF);
}

int main(void)
{
  USB_Start();
  
  RCC_I2CCLKConfig(RCC_I2C1CLK_SYSCLK);
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOF, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
  
  // Set Up PA4 and PA5 as debug pins
  GPIO_InitTypeDef gp;
  gp.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
  gp.GPIO_Mode = GPIO_Mode_OUT;
  gp.GPIO_Speed = GPIO_Speed_50MHz;
  gp.GPIO_OType = GPIO_OType_PP;
  gp.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA, &gp);


  // Set up the I2C peripheral
  gp.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
  gp.GPIO_Mode = GPIO_Mode_AF;
  gp.GPIO_OType = GPIO_OType_OD;
  gp.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(GPIOF, &gp);

  GPIO_PinAFConfig(GPIOF, GPIO_PinSource0, GPIO_AF_1);
  GPIO_PinAFConfig(GPIOF, GPIO_PinSource1, GPIO_AF_1);
  
  I2C_DeInit(I2C1);

  I2C_InitTypeDef i2c;
  i2c.I2C_Timing = 0x10805E89; // calculated from ST's spreadsheet
  i2c.I2C_AnalogFilter = I2C_AnalogFilter_Enable;
  i2c.I2C_DigitalFilter = 0;
  i2c.I2C_Mode = I2C_Mode_I2C;
  i2c.I2C_OwnAddress1 = 0;
  i2c.I2C_Ack = I2C_Ack_Enable;
  i2c.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
  I2C_Init(I2C1, &i2c);

//  I2C_StretchClockCmd(I2C1, ENABLE);

  I2C_Cmd(I2C1, ENABLE);


  initialize_pwm();
  
  SysTick_Config(SystemCoreClock / 2000);
  while(1) {
    i2c_trigger_capture();
    delay_ms(100);
    i2c_read_values();
    delay_ms(100);
    //serialio_feed();
  }
}
