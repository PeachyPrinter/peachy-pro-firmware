#ifndef INC_ADCLOCKOUT_H_
#define INC_ADCLOCKOUT_H_

#include "stm32f0xx_conf.h"
#include "stm32f0xx_adc.h"
#include "stm32f0xx_rcc.h"
#include "stm32f0xx_gpio.h"
#include "hwaccess.h"

#define ADC_LOCKOUT_POSITION 0 //ADC2
#define ADC_LOCKOUT_MISSING 0
#define ADC_LOCKOUT_VALID 1
#define ADC_MIN 1000
#define ADC_MAX 3000
#define ADC_TOGGLE_MAX 8

void check_adcLockout(void);
void adc_toggle_leds(void);


#endif /* INC_ADCLOCKOUT_H_ */
