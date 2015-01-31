#include <stm32f0xx.h>
#include <stm32f0xx_misc.h>
#include <stm32f0xx_rcc.h>
#include <stm32f0xx_crs.h>

#include <usb_core.h>
#include <usb_regs.h>

static void CRS_Config(void)
{
  /*Enable CRS Clock*/
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_CRS, ENABLE);
  
  /* Select USB SOF as synchronization source */
  CRS_SynchronizationSourceConfig(CRS_SYNCSource_USB);
  
  /*Enables the automatic hardware adjustment of TRIM bits: AUTOTRIMEN:*/
  CRS_AutomaticCalibrationCmd(ENABLE);
  
  /*Enables the oscillator clock for frequency error counter CEN*/
  CRS_FrequencyErrorCounterCmd(ENABLE);
}

void USB_Start(void) {
  /* Enable USB clock */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, ENABLE);

  /* Select HSI48 as USB clock */
  RCC_USBCLKConfig(RCC_USBCLK_HSI48);
  
  /* Configure the Clock Recovery System */
  CRS_Config();  
  
  /* Flip the Reset bits around to initialize the core */
  _SetCNTR(CNTR_FRES);
  _SetCNTR(0);

  /* Clear interrupts */
  _SetISTR(0);

  /* BTable describes the PMA layout, and must be in the PMA */
  _SetBTABLE(BTABLE_ADDRESS);

  /* Set interrupt mask */
  _SetCNTR(CNTR_CTRM | CNTR_WKUPM | CNTR_SUSPM | CNTR_ERRM | CNTR_SOFM | CNTR_ESOFM | CNTR_RESETM);


  /* Connect the interrupts from the USB macrocell to the NVIC */
  NVIC_InitTypeDef NVIC_InitStructure;
  
  NVIC_InitStructure.NVIC_IRQChannel = USB_LP_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

}

