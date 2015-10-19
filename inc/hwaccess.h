#ifndef __HWACCESS_H
#define __HWACCESS_H

uint8_t getDebugSwitch(void);
void setInLed(uint8_t);
void setCoilLed(uint8_t);
void setCornerLed(uint8_t);
void laser_on(void);
void laser_off(void);

#endif
