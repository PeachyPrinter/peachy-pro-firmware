#include "stm32f0xx_conf.h"
#include "hwaccess.h"

void setupJP5(){
  // Set Up PA4 and PA5 as debug pins
  GPIO_InitTypeDef gp;
  gp.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
  gp.GPIO_Mode = GPIO_Mode_OUT;
  gp.GPIO_Speed = GPIO_Speed_50MHz;
  gp.GPIO_OType = GPIO_OType_PP;
  gp.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA, &gp);

  //Setup PB0 as laser override pin
  gp.GPIO_Pin = GPIO_Pin_0 ;
  gp.GPIO_Mode = GPIO_Mode_IN;
  gp.GPIO_Speed = GPIO_Speed_50MHz;
  gp.GPIO_OType = GPIO_OType_OD;
  gp.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(GPIOB, &gp);
}


void setupLeds(void){
  //LEDs for debug (not sure pin 12, seems to be used for usb sending)
  GPIO_InitTypeDef gp;
  gp.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
  gp.GPIO_Mode = GPIO_Mode_OUT;
  gp.GPIO_Speed = GPIO_Speed_50MHz;
  gp.GPIO_OType = GPIO_OType_PP;
  gp.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOB, &gp);
}

void setJP5_midOut(uint8_t instate){
	GPIO_WriteBit(GPIOB, GPIO_Pin_11, instate);
}

void setCoilLed(uint8_t instate){
  GPIO_WriteBit(GPIOB, GPIO_Pin_14, instate); //Nearest coil header
}

void setCornerLed(uint8_t instate){
  GPIO_WriteBit(GPIOB, GPIO_Pin_15, instate); //Corner
}

void setInLed(uint8_t instate){
  GPIO_WriteBit(GPIOB, GPIO_Pin_13, instate); //Inside corner
}	

void laser_on(void) {
  GPIO_WriteBit(GPIOB, GPIO_Pin_5, 1);
}

void laser_off(void) {
  GPIO_WriteBit(GPIOB, GPIO_Pin_5, 0);
}


uint8_t getDebugSwitch(){
  //returns:  1 Closed, 0 Open
  return !GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0);
}
