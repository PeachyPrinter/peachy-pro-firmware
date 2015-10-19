#ifndef __HWACCESS_H
#define __HWACCESS_H

void setupJP5(void);
void setJP5_PA5(uint8_t);
void setJP5_PA6(uint8_t);
void setJP5_PA7(uint8_t);
uint8_t getDebugSwitch(void);

void setupJP6(void);

void setupLeds(void);
void setInLed(uint8_t);
void setCoilLed(uint8_t);
void setCornerLed(uint8_t);

void laser_on(void);
void laser_off(void);

#endif
