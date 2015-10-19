#ifndef __REPROG_H
#define __REPROG_H

uint8_t getDebugSwitch(void);
void wipeFlash(void);
void unlockFlash(void);
void lockFlash(void);
void reboot(void);
void disableUsb(void);

#endif
