#ifndef __PWMOUT__H__
#define __PWMOUT__H__
#include "stm32f0xx_tim.h"
#include "stm32f0xx_gpio.h"

void initialize_pwm(void);
void update_pwm(void);

#endif
