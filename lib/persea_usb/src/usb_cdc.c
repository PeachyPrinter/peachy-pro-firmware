#include <stm32f0xx.h>
#include <stm32f0xx_misc.h>

#include <usb_core.h>
#include <usb_regs.h>
#include <usb_control.h>
#include <usb_cdc.h>

static volatile uint8_t in_ep_busy = 1;

void QueueTx(unsigned char* out, int len) {
  /* wait for any outbound data to get sent */
  while(in_ep_busy);
  in_ep_busy = 1;

  UserToPMABufferCopy(out, EP3_TX_ADDR, len);
  _SetEPTxCount(3, len);
  _SetEPTxStatus(3, EP_TX_VALID);  
}

int WouldTxBlock() {
  return in_ep_busy;
}

void CDC_SetConfiguration(void) {
  in_ep_busy = 0;
}

void HandleTx(void) {
  _ClearEP_CTR_TX(3);
  in_ep_busy = 0;
}

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
    HandleTx();
    break;
  }
}

