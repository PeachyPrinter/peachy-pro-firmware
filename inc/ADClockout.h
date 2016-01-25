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
#define ADC_LOWER_LIMIT 1400 //Lower than the MIN_LIMIT or
#define ADC_UPPER_LIMIT 2600 //Above the MAX_LIMIT
#define ADC_TOGGLE_MAX 8

void check_adcLockout(void);
void adc_toggle_leds(void);


#endif /* INC_ADCLOCKOUT_H_ */
