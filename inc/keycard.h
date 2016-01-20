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
#define KEY_MASTER 0b11001001
#define KEY_LENGTH 8

void setup_keycard(void);
void read_key(void);
void key_check(uint32_t key_bit);
void update_key_state(void);



#endif /* INC_KEYCARD_H_ */
