#ifndef __DRIPPER_H
#define __DRIPPER_H

#define DRIP_DEADTIME 100
#define DRIP_TOGGLES 10
#define DRIP_TICKS_PER_TOGGLE_MAX 5 //

#define DRIPPER_IDLE 0
#define DRIPPER_CHECKING 1
#define DRIPPER_DEAD 2

void initialize_dripper(void);
void initialize_debouncer(void);
void send_updated_drip_count(void);

#endif
