#include <stm32f0xx.h>
#include <stm32f0xx_misc.h>

#include <usb_core.h>
#include <usb_regs.h>
#include <usb_control.h>
#include <usb_cdc.h>

static volatile uint8_t out_ep_busy = 1;

void QueueTx(unsigned char* out, int len) {
  /* wait for any outbound data to get sent */
  while(_GetEPTxStatus(3) == EP_TX_VALID) ;

  UserToPMABufferCopy(out, EP3_TX_ADDR, len);
  _SetEPTxCount(3, len);
  _SetEPTxStatus(3, EP_TX_VALID);  
}

int WouldTxBlock() {
  return _GetEPTxStatus(3) == EP_TX_VALID;
}

void CDC_SetConfiguration(void) {
  out_ep_busy = 0;
}

void HandleTx(void) {
  _ClearEP_CTR_TX(3);
}

void HandleRx(void) {
  _ClearEP_CTR_RX(2);
  //_SetEPRxStatus(2, EP_RX_VALID);
}

int CDC_ReadBytes(unsigned char* out) {
  if (out_ep_busy) { return 0; }
  if ((_GetEPRxStatus(2) & EP_RX_MASK) != EP_RX_NAK) { return 0; }

  uint8_t xfer_count = _GetEPRxCount(2);
  PMAToUserBufferCopy(out, EP2_RX_ADDR, xfer_count);

  _SetEPRxStatus(2, EP_RX_VALID);
  return xfer_count;
}

void HandleCDC(usb_dev_t* usb, uint8_t epIndex) {
  switch(epIndex) {
  case 1:
    /* Interrupt endpoint */
    _ClearEP_CTR_TX(1);
    break;
  case 2:
    /* OUT endpoint */
    HandleRx();
    break;
  case 3:
    /* IN endpoint */
    HandleTx();
    break;
  }
}

