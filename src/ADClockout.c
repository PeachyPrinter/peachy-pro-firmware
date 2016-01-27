#include "ADClockout.h"
#include "stm32f0xx_conf.h"
#include "stm32f0xx_adc.h"
#include "stm32f0xx_rcc.h"
#include "stm32f0xx_gpio.h"
#include "hwaccess.h"
#include "led_override.h"


extern volatile uint16_t g_adcVals[ADC_CHANS];
extern volatile uint32_t tick;
uint8_t g_adc_state = ADC_LOCKOUT_MISSING;
uint8_t g_adc_togglecount=0;

void check_adcLockout(void){
  if ((g_adcVals[ADC_LOCKOUT_POSITION]<ADC_LOWER_LIMIT) | (g_adcVals[ADC_LOCKOUT_POSITION]>ADC_UPPER_LIMIT)){
    if (g_adc_state != ADC_LOCKOUT_VALID){
      g_adc_togglecount=0;
      g_adc_state = ADC_LOCKOUT_VALID;
			play_blink_four();
    }
  }
  else{
    g_adc_state = ADC_LOCKOUT_MISSING;
    g_adc_togglecount=ADC_TOGGLE_MAX;
		stop_led_steps(L_BLINK_FOUR);
  }
	//adc_toggle_leds();
}

void adc_toggle_leds(void){
  if (g_adc_togglecount<ADC_TOGGLE_MAX){
    if ((tick&0x01FF)==0){
        setInLed(g_adc_togglecount&0x1);
        g_adc_togglecount++;
    }
  }
}
