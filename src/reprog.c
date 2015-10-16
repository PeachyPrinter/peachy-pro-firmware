#include "reprog.h"
#include <usb_cdc.h>
#include "pb_encode.h"

#include "messages.pb.h"

extern bool DEBUG;

volatile int delay;
//uint32_t KEY1=0x45670123;
//uint32_t KEY2=0xCDEF89AB;

void wipeFlash(){
  unlockFlash();
  FLASH_ErasePage(0x00000000);
  FLASH_ErasePage(0x00000001);
  lockFlash();
  reboot();
}

void unlockFlash(){
	__disable_irq(); //Turn off all interrupts so we can do this cleanly
	delay=1; //Assign a variable as hacky delay
	FLASH_Unlock(); //Built in.... I love it
	__enable_irq();
}

void lockFlash(){
	__disable_irq(); //Turn off all interrupts so we can do this quickly
	delay=1; //Assign a variable as hacky delay
	FLASH_Lock();
	__enable_irq();
}

void reboot(){
	__disable_irq(); //Turn off all interrupts so we can do this quickly
	delay=1; //Assign a variable as hacky delay
	NVIC_SystemReset();
	__enable_irq();


	//__enable_irq();
}

