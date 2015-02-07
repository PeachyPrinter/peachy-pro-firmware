#ifndef __USB_CDC_H
#define __USB_CDC_H

void HandleCDC(usb_dev_t* usb, uint8_t epIndex); 
void CDC_SetConfiguration(void);
void QueueTx(unsigned char* out, int len);
int WouldTxBlock(void);

#endif
