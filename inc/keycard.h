/*
 * keycard.h
 *
 *  Created on: Jan 18, 2016
 *      Author: green
 */

#ifndef INC_KEYCARD_H_
#define INC_KEYCARD_H_

#define KEY_VALID 2
#define KEY_CHECKING 1
#define KEY_MISSING 0
//#define KEY_MASTER 108
#define KEY_MASTER 0b0010
#define KEY_LENGTH 4
#define KEY_TIMEOUT 1000 //This should be in milliseconds - untested

#define KEY_TONE_LENGTH 250 //beep ON/OFF time (Time per half cycle)
#define KEY_TONE_NUM_BEEPS 8 //NOTE: this is 2x number of beeps you want (counts half cycles)

void setup_keycard(void);
void read_key(void);
void key_check(uint8_t key_bit);
void update_key_state(void);



#endif /* INC_KEYCARD_H_ */
