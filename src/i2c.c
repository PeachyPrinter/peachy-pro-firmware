#include <i2c.h>
#include <stm32f0xx_i2c.h>
#include <clock.h>

void i2c_trigger_capture(uint8_t chan) {
  while(I2C_GetFlagStatus(I2C1, I2C_ISR_BUSY) != RESET);

  // put the 3 lsb channel bits into bit 14:12.
// from the ads1115 datasheet:
// 0x00 = AINp = AIN0 and AINn = AIN1
// 0x01 = AINp = AIN0 and AINn = AIN3
// 0x02 = AINp = AIN1 and AINn = AIN3
// 0x03 = AINp = AIN2 and AINn = AIN3
// 0x04 = AINp = AIN0 and AINn = GND
// 0x05 = AINp = AIN1 and AINn = GND
// 0x06 = AINp = AIN2 and AINn = GND
// 0x07 = AINp = AIN3 and AINn = GND

  chan = (chan & 0x03) << 4;

  I2C_TransferHandling(I2C1, 0x90, 3, I2C_AutoEnd_Mode, I2C_Generate_Start_Write);
  I2C_ClearFlag(I2C1, I2C_ICR_NACKCF | I2C_ICR_STOPCF);

  I2C_SendData(I2C1, 0x01);

  while(I2C_GetFlagStatus(I2C1, I2C_FLAG_TXIS) == RESET);

  while(I2C_GetFlagStatus(I2C1, I2C_FLAG_TXIS) == RESET);
  I2C_SendData(I2C1, 0x83 | chan);

  while(I2C_GetFlagStatus(I2C1, I2C_FLAG_TXIS) == RESET);
  I2C_SendData(I2C1, 0x83);

  while(I2C_GetFlagStatus(I2C1, I2C_ISR_STOPF) == RESET);
  I2C_ClearFlag(I2C1, I2C_ICR_STOPCF);
}

uint16_t i2c_read_values(void) {
  uint8_t i2crx[2];

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

  return ((uint16_t)i2crx[0] << 8) | i2crx[1];
}

void i2c_init(void) {
  GPIO_InitTypeDef gp;

  RCC_I2CCLKConfig(RCC_I2C1CLK_SYSCLK);
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOF, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

  gp.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
  gp.GPIO_Mode = GPIO_Mode_AF;
  gp.GPIO_Speed = GPIO_Speed_50MHz;
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
}


/*void i2c_capture(void) {
    i2c_trigger_capture();
    delay_ms(100);
    i2c_read_values();
    delay_ms(100);
    }*/
