#include "iolib.h"
#include "usbd_core.h"

/* aja: terribly named variables that match the usbd_cdc_vcp.c USB receive buffers */
#define     USB_RX_BUFFERSIZE 256

int usbrxheadptr=0, usbrxtailptr=0;
unsigned char usbrxbuffer[USB_RX_BUFFERSIZE];
int vcp_overflow = 0;

uint16_t VCP_DataTx (uint8_t* Buf, uint32_t Len);

// USB putchar function
void PutChar(unsigned char outchar)
{
	unsigned char outbuf[2];
	outbuf[0] = outchar;
	while(VCP_DataTx(outbuf,1) == USBD_FAIL);
}

// USB get char no wait
int GetCharnw(void)
{
	int intchar;

        if (vcp_overflow) {
          __disable_irq();
          vcp_overflow = 0;
          usbd_cdc_PrepareDeferredRx();
          __enable_irq();
        }

	if(usbrxheadptr != usbrxtailptr) {
          intchar = usbrxbuffer[usbrxtailptr++];

          if(usbrxtailptr==USB_RX_BUFFERSIZE) {
            usbrxtailptr = 0;
          }

          return intchar;
        }

	else return -1;
}

// USB get char wait
unsigned char GetChar(void)
{
	int intchar;

	while((intchar=GetCharnw())==-1) ;
	return (unsigned int) intchar;
}

void Puts(const char *outstrg)
{
	int i;

	for(i=0; outstrg[i]; i++) PutChar(outstrg[i]);
}
