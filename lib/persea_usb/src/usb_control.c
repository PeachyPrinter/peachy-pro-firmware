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

typedef struct {
  const uint8_t* buf;
  uint16_t count;
  uint8_t send_zlp;
} outgoing_data_t;

static outgoing_data_t ep0_output;

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

#define USB_CDC_CONFIG_DESC_SIZ (67)
const uint8_t CdcConfig[USB_CDC_CONFIG_DESC_SIZ] =
{
  /*Configuration Descriptor*/
  0x09,   /* bLength: Configuration Descriptor size */
  DESC_CONFIGURATION,      /* bDescriptorType: Configuration */
  USB_CDC_CONFIG_DESC_SIZ,                /* wTotalLength:no of returned bytes */
  0x00,
  0x02,   /* bNumInterfaces: 2 interface */
  0x01,   /* bConfigurationValue: Configuration value */
  0x00,   /* iConfiguration: Index of string descriptor describing the configuration */
  0x80,   /* bmAttributes: self powered */
  250,   /* MaxPower 0 mA */
  
  /*---------------------------------------------------------------------------*/
  
  /*Interface Descriptor */
  0x09,   /* bLength: Interface Descriptor size */
  DESC_INTERFACE,  /* bDescriptorType: Interface */
  /* Interface descriptor type */
  0x00,   /* bInterfaceNumber: Number of Interface */
  0x00,   /* bAlternateSetting: Alternate setting */
  0x01,   /* bNumEndpoints: One endpoints used */
  0x02,   /* bInterfaceClass: Communication Interface Class */
  0x02,   /* bInterfaceSubClass: Abstract Control Model */
  0x01,   /* bInterfaceProtocol: Common AT commands */
  0x00,   /* iInterface: */
  
  /*Header Functional Descriptor*/
  0x05,   /* bLength: Endpoint Descriptor size */
  0x24,   /* bDescriptorType: CS_INTERFACE */
  0x00,   /* bDescriptorSubtype: Header Func Desc */
  0x10,   /* bcdCDC: spec release number */
  0x01,
  
  /*Call Management Functional Descriptor*/
  0x05,   /* bFunctionLength */
  0x24,   /* bDescriptorType: CS_INTERFACE */
  0x01,   /* bDescriptorSubtype: Call Management Func Desc */
  0x00,   /* bmCapabilities: D0+D1 */
  0x01,   /* bDataInterface: 1 */
  
  /*ACM Functional Descriptor*/
  0x04,   /* bFunctionLength */
  0x24,   /* bDescriptorType: CS_INTERFACE */
  0x02,   /* bDescriptorSubtype: Abstract Control Management desc */
  0x02,   /* bmCapabilities */
  
  /*Union Functional Descriptor*/
  0x05,   /* bFunctionLength */
  0x24,   /* bDescriptorType: CS_INTERFACE */
  0x06,   /* bDescriptorSubtype: Union func desc */
  0x00,   /* bMasterInterface: Communication class interface */
  0x01,   /* bSlaveInterface0: Data Class Interface */
  
  /*Endpoint 2 Descriptor*/
  0x07,                           /* bLength: Endpoint Descriptor size */
  DESC_ENDPOINT,   /* bDescriptorType: Endpoint */
  CDC_CMD_EP,                     /* bEndpointAddress */
  0x03,                           /* bmAttributes: Interrupt */
  LOBYTE(8),     /* wMaxPacketSize: */
  HIBYTE(8),
  0xFF,                           /* bInterval: */
  
  /*---------------------------------------------------------------------------*/
  
  /*Data class interface descriptor*/
  0x09,   /* bLength: Endpoint Descriptor size */
  DESC_INTERFACE,  /* bDescriptorType: */
  0x01,   /* bInterfaceNumber: Number of Interface */
  0x00,   /* bAlternateSetting: Alternate setting */
  0x02,   /* bNumEndpoints: Two endpoints used */
  0x0A,   /* bInterfaceClass: CDC */
  0x00,   /* bInterfaceSubClass: */
  0x00,   /* bInterfaceProtocol: */
  0x00,   /* iInterface: */
  
  /*Endpoint OUT Descriptor*/
  0x07,   /* bLength: Endpoint Descriptor size */
  DESC_ENDPOINT,      /* bDescriptorType: Endpoint */
  CDC_OUT_EP,                        /* bEndpointAddress */
  0x02,                              /* bmAttributes: Bulk */
  LOBYTE(64),  /* wMaxPacketSize: */
  HIBYTE(64),
  0x00,                              /* bInterval: ignore for Bulk transfer */
  
  /*Endpoint IN Descriptor*/
  0x07,   /* bLength: Endpoint Descriptor size */
  DESC_ENDPOINT,      /* bDescriptorType: Endpoint */
  CDC_IN_EP,                         /* bEndpointAddress */
  0x02,                              /* bmAttributes: Bulk */
  LOBYTE(64),  /* wMaxPacketSize: */
  HIBYTE(64),
  0x00                               /* bInterval: ignore for Bulk transfer */
} ;

/**********************************************************************
 * Control Request Handers 
 */
void WriteEP0Ctrl(const uint8_t* buf, uint8_t size) {
  if (size) {
    UserToPMABufferCopy(buf, EP0_TX_ADDR, size);
  }

  _SetEPTxCount(0, size);
  _SetEPTxStatus(0, EP_TX_VALID);
}

void WriteEP0Status(void) {
  // Use DTOG = 1 for transmitting all EP0 Status packets
  _ClearDTOG_TX(0);
  _ToggleDTOG_TX(0);

  _SetEPTxCount(0, 0);
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
  case DESC_CONFIGURATION:
    to_send = CdcConfig;
    to_send_size = sizeof(CdcConfig);
  default:
    break;
  }

  if (to_send_size) {
    if (to_send_size > setup->wLength) {
      to_send_size = setup->wLength;
    }
    ep0_output.buf = to_send;
    ep0_output.count = to_send_size;
    ep0_output.send_zlp = 0;
  }
}

static void HandleSetAddress(usb_dev_t* usb, usb_setup_req_t* setup, uint8_t* rx_buffer) {
  WriteEP0Status();

  usb->state = CHANGE_ADDRESS;
  usb->address = setup->wValue & 0x7F;

//  _SetDADDR((setup->wValue & 0x7F) | DADDR_EF);
}

static void HandleSetConfiguration(usb_dev_t* usb, usb_setup_req_t* setup, uint8_t* rx_buffer) {
  WriteEP0Status();
  usb->state = CONFIGURED;

  EP_Config(0, EP_OUT, EP_CONTROL, EP0_RX_ADDR);
  EP_Config(0, EP_IN, EP_CONTROL, EP0_TX_ADDR);
  _SetEPAddress(0, 0);

  EP_Config(1, EP_IN, EP_INTERRUPT, EP1_TX_ADDR);
  _SetEPTxStatus(1, EP_TX_NAK);
  _SetEPAddress(1, 1);

  EP_Config(2, EP_OUT, EP_BULK, EP2_RX_ADDR);
  _SetEPRxStatus(2, EP_RX_VALID);
  _SetEPAddress(2, 2);

  EP_Config(3, EP_IN, EP_BULK, EP3_TX_ADDR);
  _SetEPTxStatus(3, EP_TX_NAK);
  _SetEPAddress(3, 3);

}

void DoNothingFunction() {
  volatile int i = 0;
  for(i = 0; i < 100; i++) ;
}

static void HandleStandardRequest(usb_dev_t* usb, usb_setup_req_t* setup, uint8_t* rx_buffer) {
  switch(setup->bmRequestType & 0x80) {
  case REQ_GET:
    switch(setup->bRequest) {
    case REQ_GET_DESCRIPTOR:
      HandleGetDescriptor(setup, rx_buffer);
      break;
    }
    /* The Status packet is going to be DTOG=1 */
    _ClearDTOG_RX(0);
    _ToggleDTOG_RX(0);
  case REQ_SET:
    switch(setup->bRequest) {
    case REQ_SET_ADDRESS:
      HandleSetAddress(usb, setup, rx_buffer);
      break;
    case REQ_SET_CONFIGURATION:
      HandleSetConfiguration(usb, setup, rx_buffer);
      break;
    default: 
      DoNothingFunction();
      break;
    }
    break;
  default:
    break;
  }
}

static void HandleClassGetRequest(usb_dev_t* usb, usb_setup_req_t* setup, uint8_t* rx_buffer) {
  ep0_output.count = 0;
  ep0_output.buf = 0;
  ep0_output.send_zlp = 1;

  /* get ready for a status packet */
  _ClearDTOG_RX(0);
  _ToggleDTOG_RX(0);
}

static void HandleClassSetRequest(usb_dev_t* usb, usb_setup_req_t* setup, uint8_t* rx_buffer) {
  WriteEP0Status();
}


static void HandleClassRequest(usb_dev_t* usb, usb_setup_req_t* setup, uint8_t* rx_buffer) {
  /* can we get away with just ignoring everything? */
  switch(setup->bmRequestType & 0x80) {
  case REQ_GET:
    HandleClassGetRequest(usb, setup, rx_buffer);
    break;
  case REQ_SET:
    HandleClassSetRequest(usb, setup, rx_buffer);
    break;
  }
}

static void HandleSetupPacket(usb_dev_t* usb) {
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
    HandleStandardRequest(usb, &setup, rx_buffer);
    break;
  case REQUEST_TYPE_CLASS:
    HandleClassRequest(usb, &setup, rx_buffer);
    break;
  default:
    DoNothingFunction();
    break;
  }

  /* Rearm the RX (EP0 OUT) endpoint */
  _SetEPRxStatus(0, EP_RX_VALID);
}

static void ChangeAddress(usb_dev_t* usb) {
  _SetDADDR(usb->address | DADDR_EF);
  usb->state = ADDRESS;
}

static void HandleControlPacket() {
  _SetEPRxStatus(0, EP_RX_VALID);
}

static void SendNextEP0() {
  if(ep0_output.count == 0 && ep0_output.send_zlp == 0) {
    return;
  }
  if (ep0_output.count == 0 && ep0_output.send_zlp) {
    ep0_output.send_zlp = 0;
    _SetEPTxCount(0,0);
    _SetEPTxStatus(0, EP_TX_VALID);
    return;
  }
  if(ep0_output.count < 64) {
    WriteEP0Ctrl(ep0_output.buf, ep0_output.count);
    ep0_output.count = 0;
    return;
  }
  /* From here on, we know we're doing a multi-part packet */
  if(ep0_output.count == 64) {
    ep0_output.send_zlp = 1;
  }
  WriteEP0Ctrl(ep0_output.buf, 64);
  ep0_output.buf += 64;
  ep0_output.count -= 64;
}

void HandleEP0(usb_dev_t* usb) {
  uint16_t istr = _GetISTR();

  if ((istr & ISTR_DIR) == 0) {
    /* This is an IN endpoint. Our transmission worked! */
    if (usb->state == CHANGE_ADDRESS) {
      ChangeAddress(usb);
    }
    
    if((istr & ISTR_EP_ID) == 0) {
      SendNextEP0();
    }
    _ClearEP_CTR_TX(0);
  } else {
    /* This is an OUT endpoint. We've got data waiting for us. */
    if (_GetENDPOINT(0) & EP_SETUP) {
      HandleSetupPacket(usb);
    } else {
      HandleControlPacket();
    }
    SendNextEP0();
    _ClearEP_CTR_RX(0);
  }
}
