Breaking down how all this USB stuff works:

Entry point from application:

# USBD_Init - defined in usbd_core.c

    void USBD_Init(USB_CORE_HANDLE *pdev,
                   USBD_DEVICE *pDevice,                  
                   USBD_Class_cb_TypeDef *class_cb, 
                   USBD_Usr_cb_TypeDef *usr_cb)

Example code calls with:

    USBD_Init(&USB_Device_dev,
              &USR_desc,
              &USBD_CDC_cb,
              &USR_cb);
    
USB_BSP_Init(pdev)
USBD_DeInit(pdev)

Set up callbacks in pdev structure for class and usr

DCD_Init(pdev) - device core

call the usr_cb->Init() callback

Enable usb interrupts

Call the pullup (either DCD or USB_BSP)

# USB_BSP_Init(pdev)

Has a few ifdefs to set up the clock source. Want to use USB_CLOCK_SOURCE_CRS for us

# USBD_DeInit

nop

# Callbacks

USBD_CDC_cb looks interesting

# USBD_CDC_cb

Defined in usbd_cdc_core:

    USBD_Class_cb_TypeDef  USBD_CDC_cb = 
    {
      usbd_cdc_Init,
      usbd_cdc_DeInit,
      usbd_cdc_Setup,
      NULL,                 /* EP0_TxSent, */
      usbd_cdc_EP0_RxReady,
      usbd_cdc_DataIn,
      usbd_cdc_DataOut,
      usbd_cdc_SOF,    
      USBD_cdc_GetCfgDesc,
    };
    
Lots of callbacks for all of the usb cdc functions.

DataIn and DataOut seem most interesting for what I'm up to.

# usbd_cdc_DataIn

This refers to APP_Rx_*. DataIn appears to be DataIn to the host? Calls DCD_EP_Tx()

# usbd_cdc_DataOut

Makes reference to APP_FOPS.pIf_DataRx, writes data into a receive buffer.

Has a pair of buffers, USB_Rx_Buffer and APP_Rx_Buffer

# APP_FOPS - alias for VCP_fops

VCP\_fops defined in usbd\_cdc\_vcp.c

    CDC_IF_Prop_TypeDef VCP_fops = 
    {
      VCP_Init,
      VCP_DeInit,
      VCP_Ctrl,
      VCP_DataTx,
      VCP_DataRx
    };
    
Again Tx and Rx seem most interesting.

Based on analysis of VCP\_DataTx, looks like we want our own
implementation of VCP\_fops. The usbd\_cdc\_if\_template.c file
appears to be what we use to implement that.

# VCP_DataTx

This starts making weird USART calls. Application specific? Looking at
other functions in this file, it appears that it's written
specifically for a VCP to USART adapter.

# Files to include then...




