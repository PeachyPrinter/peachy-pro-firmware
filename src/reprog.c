#include "reprog.h"
#include "hwaccess.h"
#include "stm32f0xx_conf.h"
#include "stm32f0xx_gpio.h"
#include <usb_core.h>
#include <usb_cdc.h>
#include "clock.h"

//uint32_t KEY1=0x45670123;
//uint32_t KEY2=0xCDEF89AB;

void wipeFlash(){
	__disable_irq(); //Turn off all interrupts so we can do this cleanly
	setCoilLed(1); //Flash the LED
	FLASH_Unlock();
  FLASH_ErasePage(0x00000000);
	NVIC_SystemReset();
}

void RebootToBootloader(){
  //It looks like the bss fields are loaded by
  // lib/cmsis_boot/startup/startup_stm32f0xx.s
  // We should be able to get away not re-loading them, and letting the reset function go
  // The other option is overriding the Reset_Handler function and doing the loading ourselves
  // This function should be first called in the main.c
	
	//Steps:
	//1) Set magic value at magic place in ram
	//2) reboot
  *BOOTLOADER_MAGIC_ADDR = BOOTLOADER_MAGIC_TOKEN;
	NVIC_SystemReset();
}

static uint32_t *bootloader_msp = (void *)BOOTLOADER_START_ADDR;
static void *(**bootloader)() = (void *)BOOTLOADER_START_ADDR + 4;

void bootloaderSwitcher(){
  uint32_t jumpaddr,tmp;

  tmp=*BOOTLOADER_MAGIC_ADDR;
  //tmp=BOOTLOADER_MAGIC_TOKEN; //DEBUG
  if (tmp == BOOTLOADER_MAGIC_TOKEN){
    *BOOTLOADER_MAGIC_ADDR=0; //Zero it so we don't loop by accident

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOF, ENABLE);
    GPIO_InitTypeDef gp;
    gp.GPIO_Pin = GPIO_Pin_11; //BOOT0 pin
    gp.GPIO_Mode = GPIO_Mode_OUT;
    gp.GPIO_Speed = GPIO_Speed_2MHz;
    gp.GPIO_OType = GPIO_OType_PP;
    gp.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOF, &gp);
    GPIO_WriteBit(GPIOF, GPIO_Pin_11, 1);

    void (*bootloader)(void) = 0; //Zero the function pointer.
    jumpaddr = *(__IO uint32_t*)(BOOTLOADER_START_ADDR + 4);
    bootloader = (void (*)(void)) jumpaddr; //Set the function pointer to bootaddr +4
    __set_MSP(*(__IO uint32_t*) BOOTLOADER_START_ADDR); //load the stackpointer - bye bye program
    bootloader(); //GO TO DFU MODE MOFO :D

    //this should never be hit, trap for debugging
    while(1){}
  }

}

void bootloaderSwitcher2(){
  uint32_t tmp;

	//Steps:
	//1) check magic place in ram for DEADBEEF
	//2.0) Configure clocks to system+PF
	//2.1) set PF11 (BOOT0) pin as pull UP
	//3) Jump to bootloader code

  RCC->APB2ENR = RCC_APB2ENR_SYSCFGEN; //Enable the system clock

  tmp=*BOOTLOADER_MAGIC_ADDR;
  //tmp=BOOTLOADER_MAGIC_TOKEN; //DEBUG
  if (tmp == BOOTLOADER_MAGIC_TOKEN){
    *BOOTLOADER_MAGIC_ADDR=0; //Zero it so we don't loop by accident

    //Enable PF11 (BOOT0) as input pullUP
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOF, ENABLE);
    //2 bit shifts per port in PUPDR reg ie: 11 positions * 2 bits == 22
    //0b01 defined as Pull-up - Reset condition is input pull down
    //GPIOF->PUPDR|=0b01<<22;
     GPIO_InitTypeDef gp;
    gp.GPIO_Pin = GPIO_Pin_11; //BOOT0 pin
    gp.GPIO_Mode = GPIO_Mode_OUT;
    gp.GPIO_Speed = GPIO_Speed_2MHz;
    gp.GPIO_OType = GPIO_OType_PP;
    gp.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOF, &gp);

    __set_MSP(*bootloader_msp); //set the stack pointer to the addr
    (*bootloader)(); //Execute program at location bootloader

    /*//CMSIS full definition for PF11

    */
  }

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
