#include <stm32f0xx.h>
#include <stm32f0xx_misc.h>

#include <usb_core.h>
#include <usb_regs.h>
#include <usb_control.h>

typedef struct {
  uint8_t bmRequestType;
  uint8_t bRequest;
  uint16_t wValue;
  uint16_t wIndex;
  uint16_t wLength;
} usb_setup_req_t;

/**********************************************************************
 * Descriptors
 */
#define USB_SIZ_DEVICE_DESC                     18
#define USBD_VID                        0x0483
#define USBD_PID                        0x5740

const uint8_t DeviceDescriptor[USB_SIZ_DEVICE_DESC] =
{
  0x12,                       /*bLength */
  DESC_DEVICE, /*bDescriptorType*/
  0x00,                       /*bcdUSB */
  0x02,
  0x00,                       /*bDeviceClass*/
  0x00,                       /*bDeviceSubClass*/
  0x00,                       /*bDeviceProtocol*/
  64,           /*bMaxPacketSize*/
  LOBYTE(USBD_VID),           /*idVendor*/
  HIBYTE(USBD_VID),           /*idVendor*/
  LOBYTE(USBD_PID),           /*idVendor*/
  HIBYTE(USBD_PID),           /*idVendor*/
  0x00,                       /*bcdDevice rel. 2.00*/
  0x02,
  0x00,           /*Index of manufacturer  string*/
  0x00,       /*Index of product string*/
  0x00,        /*Index of serial number string*/
  1            /*bNumConfigurations*/
} ; /* USB_DeviceDescriptor */

#define  USB_LEN_DEV_QUALIFIER_DESC                     0x0A
static const uint8_t DeviceQualifier[USB_LEN_DEV_QUALIFIER_DESC] =
{
  USB_LEN_DEV_QUALIFIER_DESC,
  DESC_DEVICE_QUALIFIER,
  0x00,
  0x02,
  0x00,
  0x00,
  0x00,
  0x40,
  0x01,
  0x00,
};

/**********************************************************************
 * Control Request Handers 
 */
void WriteEP0(const uint8_t* buf, uint8_t size) {
  UserToPMABufferCopy(buf, EP0_TX_ADDR, size);
  _SetEPTxCount(0, size);
  _SetEPTxStatus(0, EP_TX_VALID);
}

volatile usb_setup_req_t last_setup;

static void HandleGetDescriptor(usb_setup_req_t* setup, uint8_t* rx_buffer) {
  const uint8_t* to_send;
  uint8_t to_send_size = 0;
  last_setup = *setup;

  uint8_t descriptor = HIBYTE(setup->wValue);

  switch(descriptor) {
  case DESC_DEVICE_QUALIFIER:
    to_send = DeviceQualifier;
    to_send_size = sizeof(DeviceQualifier);
    break;
  case DESC_DEVICE:
    to_send = DeviceDescriptor;
    to_send_size = sizeof(DeviceDescriptor);
    break;
  default:
    break;
  }
  if (to_send_size) {
    WriteEP0(to_send, to_send_size);
  }
}
static void HandleStandardRequest(usb_setup_req_t* setup, uint8_t* rx_buffer) {
  switch(setup->bmRequestType) {
  case REQ_GET_CONFIGURATION:
    switch(setup->bRequest) {
    case REQ_GET_DESCRIPTOR:
      HandleGetDescriptor(setup, rx_buffer);
      break;
    default: break;
    }
    break;
  default:
    break;
  }
}

static void HandleSetupPacket() {
  uint8_t rx_buffer[64];
  usb_setup_req_t setup;

  uint8_t xfer_count = _GetEPRxCount(0);

  PMAToUserBufferCopy(rx_buffer, EP0_RX_ADDR, xfer_count);
  
  setup.bmRequestType = *(uint8_t *)  (rx_buffer);
  setup.bRequest      = *(uint8_t *)  (rx_buffer + 1);
  setup.wValue        = SWAPBYTE      (rx_buffer + 2);
  setup.wIndex        = SWAPBYTE      (rx_buffer + 4);
  setup.wLength       = SWAPBYTE      (rx_buffer + 6);

  switch(setup.bmRequestType & REQUEST_TYPE_MASK) {
  case REQUEST_TYPE_STD:
    HandleStandardRequest(&setup, rx_buffer);
    break;
  default:
    break;
  }
}

static void HandleControlPacket() {

}

void HandleEP0(void) {
  uint16_t istr = _GetISTR();

  if ((istr & ISTR_DIR) == 0) {
    /* This is an IN endpoint. Our transmission worked! */
    _ClearEP_CTR_TX(0);
  } else {
    /* This is an OUT endpoint. We've got data waiting for us. */
    if (_GetENDPOINT(0) & EP_SETUP) {
      HandleSetupPacket();
    } else {
      HandleControlPacket();
    }
    _ClearEP_CTR_RX(0);
  }
}
