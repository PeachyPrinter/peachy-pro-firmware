#ifndef __REPROG_H
#define __REPROG_H

//32 bit pointer to 32 bit uint32_t (read inside out)
#define BOOTLOADER_MAGIC_ADDR ((uint32_t*) ((uint32_t) 0x20001000)) //4k into SRAM (out of 6k)
#define BOOTLOADER_MAGIC_TOKEN 0xDEADBEEF

//Value taken from CD00167594.pdf page 35, system memory start.
#define BOOTLOADER_START_ADDR 0x1fffc400 //This may be c800

uint8_t getDebugSwitch(void);
void wipeFlash(void);
void unlockFlash(void);
void lockFlash(void);
void reboot(void);
void disableUsb(void);
void RebootToBootloader(void);
void bootloaderSwitcher(void);

#endif
