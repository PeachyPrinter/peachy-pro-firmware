#ifndef __PWMOUT__H__
#define __PWMOUT__H__
#include "stm32f0xx_tim.h"
#include "stm32f0xx_gpio.h"
#include "messages.pb.h"

void initialize_pwm(void);
void update_pwm(void);
void set_pwm(int32_t,int32_t,uint32_t);

#define MOVE_SIZE 8

#endif
