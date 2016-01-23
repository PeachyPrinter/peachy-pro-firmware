#include "stm32f0xx_conf.h"
#include "stm32f0xx_adc.h"
#include "stm32f0xx_rcc.h"
#include "stm32f0xx_gpio.h"
#include "hwaccess.h"
#include "clock.h"
#include "pwmout.h"
#include "ADClockout.h"
#include "keycard.h"
#include "overrides.h"

volatile uint16_t g_adcVals[ADC_CHANS]; //ADC_CHANS defined in headder
volatile uint16_t g_adcCal;
volatile uint8_t g_adc_indexer=0;
volatile uint32_t g_coil_twig_state=0;
volatile uint32_t g_musicVar=0;
volatile uint8_t g_led_control=1;

extern volatile uint32_t tick;
extern uint8_t g_adc_state;
extern uint8_t g_adc_togglecount;
extern uint8_t g_key_state;


void twigCoils(){
  uint32_t max=0x3FFFF;//18 bits full range
  uint32_t middle=0x20000;
  //uint32_t musicVar=0;

  //Systick is 1/24000 seconds, once every 2^10 (~0.5 seconds) it switches state
  if ((tick&0x03FF)==0){ //IF the lower 14 bits are 0
    setUSBLed(0);
    g_coil_twig_state=(g_coil_twig_state+1)%5; //
    switch(g_coil_twig_state){
      case 0: //stop
        set_pwm(middle,middle,0);
        break;
      case 1: //far right
        set_pwm(max,max,0);
        break;
      case 2: //stop
        set_pwm(middle,middle,0);
        break;
      case 3: //far left
        set_pwm(0,0,0);
        break;
      case 4:
        break; //Do the noise thing
    }
  }
  else if (g_coil_twig_state==4){ //BEEP TIME!
    setUSBLed(1);
    if (g_musicVar){
      g_musicVar=0;
      set_pwm(g_musicVar,g_musicVar,0);
    }
    else{
      g_musicVar=0x3FFFF;
      set_pwm(g_musicVar,g_musicVar,0);
    }
  }
}

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
  gp.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA, &gp);

  //Setup the ADC for circular, auto sampling.
  //setupTIM1(); //AutoSampling
  //setupADC_DMA(); //Circular on TIM1 into g_adcVals[]
  setupADC(); //Setup with Circular sampling

  //The enables are out here since we needed to stay disabled for DMA config
  //ADC_DMACmd(ADC1,ENABLE);
  //DMA_Cmd(DMA1_Channel1,ENABLE);
  //TIM_Cmd(TIM1, ENABLE);
  ADC_Cmd(ADC1,ENABLE);
}

//Unused, I couldn't get TIM1 to run, it's a 32bit counter though.
void setupTIM1(){
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1,ENABLE);

  TIM_InternalClockConfig(TIM1);

  TIM_TimeBaseInitTypeDef ti;
  ti.TIM_Prescaler = 480; // 48MHz/480 gives us 10us steps
  ti.TIM_CounterMode = TIM_CounterMode_Up;
  ti.TIM_Period = 1000; //Run the timer 10ms TODO: Make this a config?
  ti.TIM_ClockDivision = TIM_CKD_DIV1; //Set Clock divider to 1.
  ti.TIM_RepetitionCounter = 0; //Only valid for TIM1
  TIM_TimeBaseInit(TIM1, &ti);
  TIM_SelectOutputTrigger(TIM1,TIM_TRGOSource_Update);
  TIM_Cmd(TIM1, ENABLE);
}

void setupADC(){

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1,ENABLE);//The ADC1 is connected the APB2 peripheral bus
  RCC_HSI14Cmd(ENABLE);
  RCC_HSI14ADCRequestCmd(ENABLE);
  RCC_ADCCLKConfig(RCC_ADCCLK_HSI14);

  g_adcCal=ADC_GetCalibrationFactor(ADC1);

  ADC_DiscModeCmd(ADC1,ENABLE); //Sample one at at a time

  ADC_InitTypeDef adc;
  adc.ADC_ContinuousConvMode = DISABLE;
  adc.ADC_Resolution = ADC_Resolution_12b; //take all the bits since accuracy > speed
  adc.ADC_DataAlign = ADC_DataAlign_Right; // 4095-0 for uint16_t
  adc.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_TRGO; //Sample on TIM1 for auto samples (Not needed yet)
  adc.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_Falling; //Sample on the rising edge
  adc.ADC_ScanDirection = ADC_ScanDirection_Upward;//Start at the bottom and rotate around.
  ADC_StructInit(&adc);

  ADC_TempSensorCmd(ENABLE);
  ADC_VrefintCmd(ENABLE);

  ADC_ChannelConfig(ADC1,ADC_Channel_2,ADC_SampleTime_239_5Cycles);  //PA2
  ADC_ChannelConfig(ADC1,ADC_Channel_3,ADC_SampleTime_239_5Cycles);  //PA3
  ADC_ChannelConfig(ADC1,ADC_Channel_16,ADC_SampleTime_239_5Cycles); //Temp sensor tied to chan 16
  ADC_ChannelConfig(ADC1,ADC_Channel_17,ADC_SampleTime_239_5Cycles); //Vref sensor tied to chan 17
}

//Unused, leaving here for now
void setupADC_DMA(){

  RCC_AHBPeriphClockCmd(RCC_AHBENR_DMA1EN,ENABLE); //Enable Clock to DMA

  ADC_DMARequestModeConfig(ADC1,ADC_DMAMode_Circular);

  DMA_InitTypeDef dma;
  dma.DMA_BufferSize = ADC_CHANS; // Same size as channels - this does the pointer modulous.
  dma.DMA_MemoryBaseAddr = (uint32_t)g_adcVals[0]; //Store results in array of adcVals (circular writing)
  dma.DMA_DIR = DMA_DIR_PeripheralSRC; //ADC is source of DMA transfer
  dma.DMA_M2M = DMA_M2M_Disable; //Disable memory to memory writing (read from ADC)
  dma.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord; //uint16_t is half of the 32 bit word
  dma.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  dma.DMA_MemoryInc = DMA_MemoryInc_Enable; //After writing, incriment pointer: base+(pointer++)%ADC_CHANS
  dma.DMA_Mode = DMA_Mode_Circular; //Rotate through the ADC chans that are enabled
  dma.DMA_PeripheralBaseAddr = (uint32_t)(&(ADC1->DR)); //Pointer to the ADC results register
  dma.DMA_PeripheralInc = DMA_PeripheralInc_Disable; //ADC only has one output register

  DMA_Init(DMA1_Channel1,&dma); //ADC on DMA channel 1
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

  //PA6 and PA7 are tied to laser testing signal for now
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

void updateADC(){ //Single Conversions only
  ADC_StartOfConversion(ADC1);
  while (!ADC_GetFlagStatus(ADC1,ADC_FLAG_EOC)){}
  ADC_ClearFlag(ADC1,ADC_FLAG_EOC);
  g_adcVals[g_adc_indexer]=ADC1->DR;
  g_adc_indexer=(g_adc_indexer+1)%ADC_CHANS;
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

void setCornerLed(uint8_t instate){
  if (g_led_control)
    GPIO_WriteBit(GPIOB, GPIO_Pin_15, instate);
}

void setCoilLed(uint8_t instate){
  if (g_led_control)
    GPIO_WriteBit(GPIOB, GPIO_Pin_14, instate);
}

void setInLed(uint8_t instate){
  if (g_led_control)
    GPIO_WriteBit(GPIOB, GPIO_Pin_13, instate); //Inside corner
}	

void setUSBLed(uint8_t instate){
  if (g_led_control)
    GPIO_WriteBit(GPIOB, GPIO_Pin_12, instate); //Inside corner
}

void laser_on(void) {
  bool laser_gating=1;
  if (ADC_KEY_EN){
    if (g_adc_state!=ADC_LOCKOUT_VALID)
      laser_gating=0;
  }
  if (INTERLOCK_KEY_EN){
    if (g_key_state!=KEY_VALID)
      laser_gating=0;
  }
  GPIO_WriteBit(GPIOB, GPIO_Pin_5, laser_gating);
  setInLed(laser_gating);
}

void laser_off(void) {
  GPIO_WriteBit(GPIOB, GPIO_Pin_5, 0);
  if (g_adc_togglecount==ADC_TOGGLE_MAX)
    setInLed(0);
}

uint8_t getDebugSwitch(){
  //returns:  1 Closed, 0 Open
  return !GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0);
}

