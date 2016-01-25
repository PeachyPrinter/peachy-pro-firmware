#ifndef __HWACCESS_H
#define __HWACCESS_H

#define ADC_CHANS 4
#define TEMP110_CAL ((uint16_t*) ((uint32_t) 0x1FFFF7C2)) //32 bit pointer to a 16 bit uint
#define TEMP30_CAL ((uint16_t*) ((uint32_t) 0x1FFFF7B8)) //32 bit pointer to a 16 bit uint
#define VREFINT_CAL ((uint16_t*) ((uint32_t) 0x1FFFF7BA))

void setupJP5(void);
void setJP5_PA5(uint8_t);
void setJP5_PA6(uint8_t);
void setJP5_PA7(uint8_t);
uint8_t getDebugSwitch(void);
uint16_t getADCVal();
void updateADC();
void setupADC_DMA();
void checkCoils();
void setupADC();
void setupTIM1();
void adcCal(void);
void twigCoils(void);

void setupJP6(void);

void setupLeds(void);
void setInLed(uint8_t);
void setCoilLed(uint8_t);
void setCornerLed(uint8_t);
void setUSBLed(uint8_t);

void laser_on(void);
void laser_off(void);

#endif
