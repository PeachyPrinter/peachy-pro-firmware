/*
 * led_override.h
 *
 *  Created on: Jan 22, 2016
 *      Author: green
 */

#ifndef INC_LED_OVERRIDE_H_
#define INC_LED_OVERRIDE_H_

#define LED_TIME_STEP 50 //step between led steps in ms

void turn_leds_on(uint8_t);
void initialize_led_override(void);
void next_led_step(void);
void start_led_steps(uint8_t new_steps[], uint8_t num_steps);
void next_led_step(void);

#endif /* INC_LED_OVERRIDE_H_ */
