#include "reprog.h"
#include "hwaccess.h"
#include "stm32f0xx_conf.h"
#include "stm32f0xx_gpio.h"
#include <usb_core.h>
#include <usb_cdc.h>
#include "pb_encode.h"
#include "messages.pb.h"

extern bool g_debug;

volatile int delay;
//uint32_t KEY1=0x45670123;
//uint32_t KEY2=0xCDEF89AB;

void wipeFlash(){
  unlockFlash();
	setCoilLed(1); //Flash the led as it reboots
  FLASH_ErasePage(0x00000000);
  FLASH_ErasePage(0x00000001);
  reboot();
}

void unlockFlash(){
	__disable_irq(); //Turn off all interrupts so we can do this cleanly
	delay=1; //Assign a variable as hacky delay
	FLASH_Unlock(); //They forgot to disable the interrupts, could lead to fails.
	__enable_irq();
}

void reboot(){
  //uint8_t button;
	//if (g_debug){
	//	button = getDebugSwitch();
	//	while (!button){
	//		button = getDebugSwitch();
	//	}
	//}

	NVIC_SystemReset();
	//bye bye firmware
}

#ifdef _g_debug
//This is debug code. Don't judge
void enableUsb(){
  //This was easier than finding the built in function
  //I don't even think it exists tbh...
  uint16_t USB_CNTR;

  //USB settings aren't direct memory mapped
  //You need to do a read-mod-write thing
  USB_CNTR = _GetCNTR();
  //withdraw powerdown
  USB_CNTR &= !(CNTR_PDWN); //(xxx & !(010) = x0x)
  //Write it
  _SetCNTR(USB_CNTR);
}

void disableUsb(){
  //Opposite of enableUsb()
  uint16_t USB_CNTR;

  USB_CNTR = _GetCNTR();
  USB_CNTR |= CNTR_PDWN; //(xxx | 010 == x1x)
  _SetCNTR(USB_CNTR);
}

void lockFlash(){
	__disable_irq(); //Turn off all interrupts so we can do this quickly
	delay=1; //Assign a variable as hacky delay
	FLASH_Lock(); //They forgot to disable the interrupts, could lead to fails
	__enable_irq();
}

#endif
