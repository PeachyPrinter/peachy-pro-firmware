#include <stm32f0xx.h>
#include <stm32f0xx_misc.h>

#include <usb_core.h>
#include <usb_regs.h>
#include <usb_control.h>
#include <usb_cdc.h>

void HandleRX(void) {
  _ClearEP_CTR_RX(2);
  _SetEPRxStatus(2, EP_RX_VALID);
}

void HandleCDC(usb_dev_t* usb, uint8_t epIndex) {
  switch(epIndex) {
  case 1:
    /* Interrupt endpoint */
    _ClearEP_CTR_TX(1);
    break;
  case 2:
    /* OUT endpoint */
    HandleRX();
    break;
  case 3:
    /* IN endpoint */
    _ClearEP_CTR_TX(3);
    break;
  }
}

