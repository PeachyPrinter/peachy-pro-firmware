#include <stm32f0xx.h>
#include <stm32f0xx_misc.h>

#include <usb_core.h>
#include <usb_regs.h>
#include <usb_control.h>
#include <usb_cdc.h>

typedef struct __attribute__ ((__packed__)) _encoded_packet {
  uint32_t magic;
  uint8_t data_bytes;
  uint16_t sequence;  
} encoded_packet_t;

#define INPUT_SIZE 128

struct _input_packet_buffer {
  // Ring buffer
  uint8_t buf[INPUT_SIZE];
  int read_idx;
  int write_idx;
  int avail_to_read;
};

static struct _input_packet_buffer input_packets = { "", 0, 0, 0 };

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

static void read_from_out_ep() {
  if ((_GetEPRxStatus(2) & EP_RX_MASK) != EP_RX_NAK) { return; }
  
  uint8_t xfer_count = _GetEPRxCount(2);
  if (xfer_count == 0) {
    return; // ignore zlp
    _SetEPRxStatus(2, EP_RX_VALID);
  }

  uint8_t usb_read[64];
  PMAToUserBufferCopy(usb_read, EP2_RX_ADDR, xfer_count);
  int i;

  encoded_packet_t* hdr = (encoded_packet_t*)&usb_read[0];
  if (hdr->magic != 0xdeadbeef) {
    return;
  }
  // TODO check sequence number?
  uint8_t* body = &usb_read[sizeof(encoded_packet_t)];

  if ((INPUT_SIZE - input_packets.avail_to_read) < hdr->data_bytes) {
    return; // no room
  }

  for(i = 0; i < hdr->data_bytes; i++) {
    input_packets.buf[input_packets.write_idx] = body[i];
    input_packets.write_idx += 1;
    if (input_packets.write_idx >= INPUT_SIZE) {
      input_packets.write_idx = 0;
    }
  }
  input_packets.avail_to_read += hdr->data_bytes;
  _SetEPRxStatus(2, EP_RX_VALID);
}

int CDC_ReadBytes(unsigned char* out) {
  /* Algorithm thoughts here:
     - check our input buffer first. 
       - If there are no bytes available, read from PMA
         - return 0 if the PMA is empty
       - check to see if we have an entire command worth. 
       - If yes, memcpy it into *out
       - If not, 
         - return 0 if no bytes available in PMA
         - else read into the other half of the buffer
  */   
  if (out_ep_busy) { return 0; }

  read_from_out_ep();

  if (input_packets.avail_to_read == 0) {
    return 0;
  }
  // now we know there's data available, check to see if we have a full
  // command worth
  uint8_t cmd_length = input_packets.buf[input_packets.read_idx];
  if (input_packets.avail_to_read < (cmd_length + 1)) {
    // +1 in th above line is to include the length byte
    return 0; // not a full command worth available
  }

  // skip over the length byte
  input_packets.read_idx = (input_packets.read_idx + 1) % INPUT_SIZE;

  // pull the rest of the bytes into the output buffer
  int i;
  for (i = 0; i < cmd_length; i++) {
    out[i] = input_packets.buf[input_packets.read_idx];
    input_packets.read_idx += 1;
    if (input_packets.read_idx >= INPUT_SIZE) {
      input_packets.read_idx = 0;
    }
  }
  input_packets.avail_to_read -= cmd_length + 1; // again, compensating for length byte
  return cmd_length;
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

