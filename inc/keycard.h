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
#define KEY_MASTER 0b01101100
#define KEY_LENGTH 8
#define KEY_TIMEOUT 1000 //This should be in milliseconds - untested

void setup_keycard(void);
void read_key(void);
void key_check(uint8_t key_bit);
void update_key_state(void);



#endif /* INC_KEYCARD_H_ */
