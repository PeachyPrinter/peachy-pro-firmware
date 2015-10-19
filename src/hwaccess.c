#include "stm32f0xx_conf.h"
#include "hwaccess.h"

void setupJP6(){

	//Mapping Tables:

	//EAGLE
	//[3.3V, SDA]
	//[ADC2, SCL]
	//[ADC3, GND]
	
	//PINOUT
	//[3.3V, PF0]
	//[PA2 , PF1]
	//[PA3 , GND]
	
	//Pin 2 and 3 as ADC (for later Dev)
  GPIO_InitTypeDef gp;
  gp.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
  gp.GPIO_Mode = GPIO_Mode_IN;
  gp.GPIO_Speed = GPIO_Speed_50MHz;
  gp.GPIO_OType = GPIO_OType_PP;
  gp.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA, &gp);

}

void setupJP5(){

	//Mapping Tables:

	//EAGLE
	//[3.3V, MISO]
	//[SCK , MOSI]
	//[SPICS, GND]
	
	//PINOUT
	//[3.3V, PA6]
	//[PA5 , PA7]
	//[PB0 , GND]

	//Future SD card location... Hopefully
  GPIO_InitTypeDef gp;
  gp.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
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

void setupLeds(){
  //LEDs for debug (not sure pin 12, looks like it's usb grounded...)
  GPIO_InitTypeDef gp;
  gp.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
  gp.GPIO_Mode = GPIO_Mode_OUT;
  gp.GPIO_Speed = GPIO_Speed_50MHz;
  gp.GPIO_OType = GPIO_OType_PP;
  gp.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOB, &gp);
}

void setJP5_PA5(uint8_t instate){
	GPIO_WriteBit(GPIOA, GPIO_Pin_5, instate);
}

void setJP5_PA6(uint8_t instate){
	GPIO_WriteBit(GPIOA, GPIO_Pin_5, instate);
}

void setJP5_PA7(uint8_t instate){
	GPIO_WriteBit(GPIOA, GPIO_Pin_5, instate);
}

void setCoilLed(uint8_t instate){
  GPIO_WriteBit(GPIOB, GPIO_Pin_14, instate);
}

void setCornerLed(uint8_t instate){
  GPIO_WriteBit(GPIOB, GPIO_Pin_15, instate);
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

