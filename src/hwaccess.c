#include "stm32f0xx_conf.h"
#include "stm32f0xx_adc.h"
#include "stm32f0xx_rcc.h"
#include "stm32f0xx_gpio.h"
#include "hwaccess.h"

#define ADC_CHANS 1

uint16_t g_adcVals[ADC_CHANS];

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

  //DATASHEET Abilities per pin
  //PF0: I2C1_SDA|CRS_SYNC|OSC_IN
  //PF1: I2C1_SCL|OSC_OUT
  //PA2: USART2_TX|TIM2_CH3|TSC_G1_IO3|ADC_IN2|WKUP4
  //PA3: USART2_RX|TIM2_CH4|TSC_G1_IO4|ADC_IN3

	//Pin 2 and 3 as ADC (for later Dev)
  GPIO_InitTypeDef gp;
  gp.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
  gp.GPIO_Mode = GPIO_Mode_AN;
  gp.GPIO_Speed = GPIO_Speed_50MHz;
  gp.GPIO_OType = GPIO_OType_OD;
  gp.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA, &gp);

  setupADC();
}

void setupADC(){

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1,ENABLE);//The ADC1 is connected the APB2 peripheral bus
  ADC_InitTypeDef adc;
  adc.ADC_ContinuousConvMode = DISABLE; //For testing initially. Eventually tie to TIM15 for auto sampling.
  adc.ADC_Resolution = ADC_Resolution_12b; //Go for broke and get all the bits!
  adc.ADC_DataAlign = ADC_DataAlign_Right; // 4095-0 for uint16_t
  adc.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T15_TRGO; //Sample on TIM15 for auto samples?
  adc.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_Rising; //Sample on the rising edge
  adc.ADC_ScanDirection = ADC_ScanDirection_Upward;//Start at the bottom and rotate around. Important for autosampling
  ADC_Init(ADC1 , &adc);

  RCC_AHBPeriphClockCmd(RCC_AHBENR_DMA1EN,ENABLE); //Enable Clock to DMA
  DMA_InitTypeDef dma;
  dma.DMA_BufferSize=ADC_CHANS; //DEFINED at 1 for now
  dma.DMA_MemoryBaseAddr=&g_adcVals; //Store results in array of adcVals (circular writing)
  dma.DMA_DIR=DMA_DIR_PeripheralSRC; //ADC is source of DMA
  dma.DMA_M2M=DMA_M2M_Disable; //Disable memory to memory writing (read from ADC)
  dma.DMA_MemoryDataSize=DMA_MemoryDataSize_HalfWord; //uint16_t is half of the 32 bit word
  dma.DMA_PeripheralDataSize=DMA_PeripheralDataSize_HalfWord;
  dma.DMA_MemoryInc=DMA_MemoryInc_Enable; //Shouldn't make a difference for 1
  dma.DMA_Mode=DMA_Mode_Circular; //Shouldn't make a difference for 1
  dma.DMA_PeripheralBaseAddr=(&(ADC1->DR)); //Pointer to the ADC results register
  dma.DMA_PeripheralInc=DMA_PeripheralInc_Disable; //ADC only has one output register

  DMA_Init(DMA1_Channel1,&dma); //ADC on DMA channel 1

  ADC_DMARequestModeConfig(ADC1,ADC_DMAMode_Circular);
  ADC_DMACmd(ADC1,ENABLE);
  ADC_Cmd(ADC1, ENABLE);
  //ADC_TempSensorCmd(DISABLE); //Add this for later (it's tied to ADC16)

  //Now setup the individual channels we are going to use
  /* From Doxy:
    ADC_SampleTime_1_5Cycles: Sample time equal to 1.5 cycles
    ADC_SampleTime_7_5Cycles: Sample time equal to 7.5 cycles
    ADC_SampleTime_13_5Cycles: Sample time equal to 13.5 cycles
    ADC_SampleTime_28_5Cycles: Sample time equal to 28.5 cycles
    ADC_SampleTime_41_5Cycles: Sample time equal to 41.5 cycles
    ADC_SampleTime_55_5Cycles: Sample time equal to 55.5 cycles
    ADC_SampleTime_71_5Cycles: Sample time equal to 71.5 cycles
    ADC_SampleTime_239_5Cycles: Sample time equal to 239.5 cycles
   */
  //Add any channels you like, but make sure you update the DEFINE of ADC_CHANS at the top
  ADC_ChannelConfig(ADC1,ADC_Channel_3,ADC_SampleTime_55_5Cycles);

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

  //DATASHEET Abilities per pin
  //PA5: SPI1_SCK| I2S1_CK|CEC|TIM2_CH1_ETR|TSC_G2_IO2|ADC_IN5
  //PA6: SPI1_MSIO|I2S1_MCK|TIM3_CH1|TIM1_BKIN|TIM16_CH1|TSC_G2_IO3|EVENTOUT|ADC_IN7
  //PA7: SPI1_MOSI|I2S1_SD|TIM1_CH1N|TIM17_CH1|TSC_G2_IO4|EVENTOUT|ADC_IN8
  //PB0: TIM3_CH3|TIM1_CH2N|TSC_G3_IO2|EVENTOUT|ADC_IN8

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

uint16_t getADC1(){
  //Start conversion, wait for it to finish, then clear the flags and return the value we got
  //One question is how do we get more than one ADC channel....
  //Guess... each sample grab rotates through the channels.

  //NOTE: This is slow, it takes ~15-20 cycles to do the entire conversion.
  // this probably only works when continuous sampling is off.
  ADC_StartOfConversion(ADC1);
  while (!ADC_GetFlagStatus(ADC1,ADC_FLAG_EOC)){}
  ADC_ClearFlag(ADC1,ADC_FLAG_EOC);
  return ADC_GetConversionValue(ADC1);
}

void setJP5_PA5(uint8_t instate){
	GPIO_WriteBit(GPIOA, GPIO_Pin_5, instate);
}

void setJP5_PA6(uint8_t instate){
	GPIO_WriteBit(GPIOA, GPIO_Pin_6, instate);
}

void setJP5_PA7(uint8_t instate){
	GPIO_WriteBit(GPIOA, GPIO_Pin_7, instate);
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

