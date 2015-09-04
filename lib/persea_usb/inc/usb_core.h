/**
  ******************************************************************************
  * @file    usb_core.h
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    31-January-2014
  * @brief   This file provides Interface prototype functions to USB cell registers
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/*****************************************
This file has been heavily modified by Persea Systems
<t.arkles@gmail.com>. I'm pretty much just reusing these register
definitions and discarding all of the code from ST's USB driver.
*****************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USB_CORE_H__
#define __USB_CORE_H__
/* Includes ------------------------------------------------------------------*/
#include "usb_regs.h"

/* Data Structures -----------------------------------------------------------*/
typedef struct {
  uint8_t halted;
} endpoint_t;

typedef enum { DEFAULT = 0, CHANGE_ADDRESS, ADDRESS, CONFIGURED } usb_state_t;

typedef struct {
  endpoint_t in_ep[4];
  endpoint_t out_ep[4];
  usb_state_t state;
  uint8_t address;
} usb_dev_t;


/* Exported defines ----------------------------------------------------------*/
#define PMA_BASE (0x40006000)
#define EP0_TX_ADDR (0x0020)
#define EP0_RX_ADDR (0x0060)
#define EP1_TX_ADDR (0x00a0)
#define EP2_RX_ADDR (0x00e0)
#define EP3_TX_ADDR (0x0120)

#define CDC_CMD_EP 0x81
#define CDC_OUT_EP 0x02
#define CDC_IN_EP 0x83

#define EP_TYPE_CTRL                           0

#define USB_EP0_IDLE                          0
#define USB_EP0_SETUP                         1
#define USB_EP0_DATA_IN                       2
#define USB_EP0_DATA_OUT                      3
#define USB_EP0_STATUS_IN                     4
#define USB_EP0_STATUS_OUT                    5


#define USB_MAX_EP0_SIZE                     64

#define USB_SPEED_FULL                       1

/* Put the BTABLE at the start of the PMA. There's no reason not to. */
#define BTABLE_ADDRESS (0x000)

#define EP_IN 0
#define EP_OUT 1

/* USB Device Requests */

#define REQUEST_TYPE_MASK (0x60)
#define REQUEST_TYPE_STD (0x00)
#define REQUEST_TYPE_CLASS (0x20)
#define REQUEST_TYPE_VENDOR (0x40)

#define REQ_GET (0x80)
#define REQ_SET (0x00)

#define REQ_GET_DESCRIPTOR (0x06)
#define REQ_SET_ADDRESS (0x05)
#define REQ_SET_CONFIGURATION (0x09)

/* From Table 9-5 in the USB 2.0 Spec, pdf page 279, doc page 251 */
#define DESC_DEVICE 1
#define DESC_CONFIGURATION 2
#define DESC_STRING 3
#define DESC_INTERFACE 4
#define DESC_ENDPOINT 5
#define DESC_DEVICE_QUALIFIER 6
#define DESC_OTHER_SPEED 7
#define DESC_INTERFACE_POWER 8

/* Exported macros -----------------------------------------------------------*/
#define  SWAPBYTE(addr)        (((uint16_t)(*((uint8_t *)(addr)))) + \
                               (((uint16_t)(*(((uint8_t *)(addr)) + 1))) << 8))
#define LOBYTE(x)  ((uint8_t)(x & 0x00FF))
#define HIBYTE(x)  ((uint8_t)((x & 0xFF00) >>8))

/* SetCNTR */
#define _SetCNTR(wRegValue)  (*CNTR   = (uint16_t)wRegValue)

/* SetISTR */
#define _SetISTR(wRegValue)  (*ISTR   = (uint16_t)wRegValue)

/* SetDADDR */
#define _SetDADDR(wRegValue) (*DADDR  = (uint16_t)wRegValue)

/* SetBCDR */
#define _SetBCDR(wRegValue)  (*BCDR   = (uint16_t)wRegValue)

/* SetBTABLE */
#define _SetBTABLE(wRegValue)(*BTABLE = (uint16_t)(wRegValue & 0xFFF8))

/*SetLPMCSR */
#define _SetLPMCSR(wRegValue) (*LPMCSR = (uint16_t)wRegValue)

/* GetCNTR */
#define _GetCNTR()   ((uint16_t) *CNTR)

/* GetISTR */
#define _GetISTR()   ((uint16_t) *ISTR)

/* GetFNR */
#define _GetFNR()    ((uint16_t) *FNR)

/* GetDADDR */
#define _GetDADDR()  ((uint16_t) *DADDR)

/* GetBTABLE */
#define _GetBTABLE() ((uint16_t) *BTABLE)

/*GetLPMCSR */
#define _GetLPMCSR() ((uint16_t) *LPMCSR)


/* SetENDPOINT */
#define _SetENDPOINT(bEpNum,wRegValue)  (*(EP0REG + bEpNum)= \
    (uint16_t)wRegValue)

/* GetENDPOINT */
#define _GetENDPOINT(bEpNum)        ((uint16_t)(*(EP0REG + bEpNum)))



/**
  * @brief  sets the type in the endpoint register(bits EP_TYPE[1:0])
  * @param  bEpNum: Endpoint Number.
  * @param  wType: Endpoint Type.
  * @retval None
  */
#define _SetEPType(bEpNum,wType) (_SetENDPOINT(bEpNum,\
                                  ((_GetENDPOINT(bEpNum) & EP_T_MASK) | wType )))

/**
  * @brief  gets the type in the endpoint register(bits EP_TYPE[1:0])
  * @param  bEpNum: Endpoint Number.
  * @retval Endpoint Type
  */
#define _GetEPType(bEpNum) (_GetENDPOINT(bEpNum) & EP_T_FIELD)


/**
  * @brief  sets the status for tx transfer (bits STAT_TX[1:0]).
  * @param  bEpNum: Endpoint Number.
  * @param  wState: new state
  * @retval None
  */
#define _SetEPTxStatus(bEpNum,wState) {\
register uint16_t _wRegVal;       \
  _wRegVal = _GetENDPOINT(bEpNum) & EPTX_DTOGMASK;\
    /* toggle first bit ? */     \
      if((EPTX_DTOG1 & wState)!= 0)      \
        _wRegVal ^= EPTX_DTOG1;        \
          /* toggle second bit ?  */         \
            if((EPTX_DTOG2 & wState)!= 0)      \
              _wRegVal ^= EPTX_DTOG2;        \
    _SetENDPOINT(bEpNum, (_wRegVal | EP_CTR_RX|EP_CTR_TX));    \
  } /* _SetEPTxStatus */

/**
  * @brief  sets the status for rx transfer (bits STAT_TX[1:0])
  * @param  bEpNum: Endpoint Number.
  * @param  wState: new state
  * @retval None
  */
#define _SetEPRxStatus(bEpNum,wState) {\
    register uint16_t _wRegVal;   \
    \
    _wRegVal = _GetENDPOINT(bEpNum) & EPRX_DTOGMASK;\
    /* toggle first bit ? */  \
    if((EPRX_DTOG1 & wState)!= 0) \
      _wRegVal ^= EPRX_DTOG1;  \
    /* toggle second bit ? */  \
    if((EPRX_DTOG2 & wState)!= 0) \
      _wRegVal ^= EPRX_DTOG2;  \
    _SetENDPOINT(bEpNum, (_wRegVal | EP_CTR_RX|EP_CTR_TX)); \
  } /* _SetEPRxStatus */

/**
  * @brief  sets the status for rx & tx (bits STAT_TX[1:0] & STAT_RX[1:0])
  * @param  bEpNum: Endpoint Number.
  * @param  wStaterx: new state.
  * @param  wStatetx: new state.
  * @retval None
  */
#define _SetEPRxTxStatus(bEpNum,wStaterx,wStatetx) {\
    register uint32_t _wRegVal;   \
    \
    _wRegVal = _GetENDPOINT(bEpNum) & (EPRX_DTOGMASK |EPTX_STAT) ;\
    /* toggle first bit ? */  \
    if((EPRX_DTOG1 & wStaterx)!= 0) \
      _wRegVal ^= EPRX_DTOG1;  \
    /* toggle second bit ? */  \
    if((EPRX_DTOG2 & wStaterx)!= 0) \
      _wRegVal ^= EPRX_DTOG2;  \
    /* toggle first bit ? */     \
    if((EPTX_DTOG1 & wStatetx)!= 0)      \
      _wRegVal ^= EPTX_DTOG1;        \
    /* toggle second bit ?  */         \
    if((EPTX_DTOG2 & wStatetx)!= 0)      \
      _wRegVal ^= EPTX_DTOG2;        \
    _SetENDPOINT(bEpNum, _wRegVal | EP_CTR_RX|EP_CTR_TX);    \
  } /* _SetEPRxTxStatus */

/**
  * @brief  gets the status for tx/rx transfer (bits STAT_TX[1:0]
  *         /STAT_RX[1:0])
  * @param  bEpNum: Endpoint Number.
  * @retval status
  */
#define _GetEPTxStatus(bEpNum) ((uint16_t)_GetENDPOINT(bEpNum) & EPTX_STAT)

#define _GetEPRxStatus(bEpNum) ((uint16_t)_GetENDPOINT(bEpNum) & EPRX_STAT)

/**
  * @brief  sets directly the VALID tx/rx-status into the endpoint register
  * @param  bEpNum: Endpoint Number.
  * @retval None
  */
#define _SetEPTxValid(bEpNum)     (_SetEPTxStatus(bEpNum, EP_TX_VALID))

#define _SetEPRxValid(bEpNum)     (_SetEPRxStatus(bEpNum, EP_RX_VALID))

/**
  * @brief  checks stall condition in an endpoint.
  * @param  bEpNum: Endpoint Number.
  * @retval TRUE = endpoint in stall condition.
  */
#define _GetTxStallStatus(bEpNum) (_GetEPTxStatus(bEpNum) \
                                   == EP_TX_STALL)
#define _GetRxStallStatus(bEpNum) (_GetEPRxStatus(bEpNum) \
                                   == EP_RX_STALL)

/**
  * @brief  set & clear EP_KIND bit.
  * @param  bEpNum: Endpoint Number.
  * @retval None
  */
#define _SetEP_KIND(bEpNum)    (_SetENDPOINT(bEpNum, \
                                (EP_CTR_RX|EP_CTR_TX|((_GetENDPOINT(bEpNum) | EP_KIND) & EPREG_MASK))))
#define _ClearEP_KIND(bEpNum)  (_SetENDPOINT(bEpNum, \
                                (EP_CTR_RX|EP_CTR_TX|(_GetENDPOINT(bEpNum) & EPKIND_MASK))))

/**
  * @brief  Sets/clears directly STATUS_OUT bit in the endpoint register.
  * @param  bEpNum: Endpoint Number.
  * @retval None
  */
#define _Set_Status_Out(bEpNum)    _SetEP_KIND(bEpNum)
#define _Clear_Status_Out(bEpNum)  _ClearEP_KIND(bEpNum)

/**
  * @brief  Sets/clears directly EP_KIND bit in the endpoint register.
  * @param  bEpNum: Endpoint Number.
  * @retval None
  */
#define _SetEPDoubleBuff(bEpNum)   _SetEP_KIND(bEpNum)
#define _ClearEPDoubleBuff(bEpNum) _ClearEP_KIND(bEpNum)

/**
  * @brief  Clears bit CTR_RX / CTR_TX in the endpoint register.
  * @param  bEpNum: Endpoint Number.
  * @retval None
  */
#define _ClearEP_CTR_RX(bEpNum)   (_SetENDPOINT(bEpNum,\
                                   _GetENDPOINT(bEpNum) & 0x7FFF & EPREG_MASK))
#define _ClearEP_CTR_TX(bEpNum)   (_SetENDPOINT(bEpNum,\
                                   _GetENDPOINT(bEpNum) & 0xFF7F & EPREG_MASK))

/**
  * @brief  Toggles DTOG_RX / DTOG_TX bit in the endpoint register.
  * @param  bEpNum: Endpoint Number.
  * @retval None
  */
#define _ToggleDTOG_RX(bEpNum)    (_SetENDPOINT(bEpNum, \
                                   EP_CTR_RX|EP_CTR_TX|EP_DTOG_RX | (_GetENDPOINT(bEpNum) & EPREG_MASK)))
#define _ToggleDTOG_TX(bEpNum)    (_SetENDPOINT(bEpNum, \
                                   EP_CTR_RX|EP_CTR_TX|EP_DTOG_TX | (_GetENDPOINT(bEpNum) & EPREG_MASK)))

/**
  * @brief  Clears DTOG_RX / DTOG_TX bit in the endpoint register.
  * @param  bEpNum: Endpoint Number.
  * @retval None
  */
#define _ClearDTOG_RX(bEpNum)  if((_GetENDPOINT(bEpNum) & EP_DTOG_RX) != 0)\
    _ToggleDTOG_RX(bEpNum)
#define _ClearDTOG_TX(bEpNum)  if((_GetENDPOINT(bEpNum) & EP_DTOG_TX) != 0)\
    _ToggleDTOG_TX(bEpNum)
      
/**
  * @brief  Sets address in an endpoint register.
  * @param  bEpNum: Endpoint Number.
  * @param  bAddr: Address.
  * @retval None
  */
#define _SetEPAddress(bEpNum,bAddr) _SetENDPOINT(bEpNum,\
    EP_CTR_RX|EP_CTR_TX|(_GetENDPOINT(bEpNum) & EPREG_MASK) | bAddr)

/**
  * @brief  Gets address in an endpoint register.
  * @param  bEpNum: Endpoint Number.
  * @retval None
  */
#define _GetEPAddress(bEpNum) ((uint8_t)(_GetENDPOINT(bEpNum) & EPADDR_FIELD))
#define _pEPTxAddr(bEpNum) ((uint16_t *)((_GetBTABLE()+bEpNum*8) + PMAAddr))
#define _pEPTxCount(bEpNum) ((uint16_t *)((_GetBTABLE()+bEpNum*8+2) + PMAAddr))
#define _pEPRxAddr(bEpNum) ((uint16_t *)((_GetBTABLE()+bEpNum*8+4) + PMAAddr))
#define _pEPRxCount(bEpNum) ((uint16_t *)((_GetBTABLE()+bEpNum*8+6) + PMAAddr))

/**
  * @brief  sets address of the tx/rx buffer.
  * @param  bEpNum: Endpoint Number.
  * @param  wAddr: address to be set (must be word aligned).
  * @retval None
  */
#define _SetEPTxAddr(bEpNum,wAddr) (*_pEPTxAddr(bEpNum) = ((wAddr >> 1) << 1))
#define _SetEPRxAddr(bEpNum,wAddr) (*_pEPRxAddr(bEpNum) = ((wAddr >> 1) << 1))

/**
  * @brief  Gets address of the tx/rx buffer.
  * @param  bEpNum: Endpoint Number.
  * @retval address of the buffer.
  */
#define _GetEPTxAddr(bEpNum) ((uint16_t)*_pEPTxAddr(bEpNum))
#define _GetEPRxAddr(bEpNum) ((uint16_t)*_pEPRxAddr(bEpNum))

/**
  * @brief  Sets counter of rx buffer with no. of blocks.
  * @param  bEpNum: Endpoint Number.
  * @param  wCount: Counter.
  * @retval None
  */
#define _BlocksOf32(dwReg,wCount,wNBlocks) {\
    wNBlocks = wCount >> 5;\
    if((wCount & 0x1f) == 0)\
      wNBlocks--;\
    *pdwReg = (uint16_t)((wNBlocks << 10) | 0x8000);\
  }/* _BlocksOf32 */

#define _BlocksOf2(dwReg,wCount,wNBlocks) {\
    wNBlocks = wCount >> 1;\
    if((wCount & 0x1) != 0)\
      wNBlocks++;\
    *pdwReg = (uint16_t)(wNBlocks << 10);\
  }/* _BlocksOf2 */

#define _SetEPCountRxReg(dwReg,wCount)  {\
    uint16_t wNBlocks;\
    if(wCount > 62){_BlocksOf32(dwReg,wCount,wNBlocks);}\
    else {_BlocksOf2(dwReg,wCount,wNBlocks);}\
  }/* _SetEPCountRxReg */

#define _SetEPRxDblBuf0Count(bEpNum,wCount) {\
    uint16_t *pdwReg = _pEPTxCount(bEpNum); \
    _SetEPCountRxReg(pdwReg, wCount);\
  }
/**
  * @brief  sets counter for the tx/rx buffer.
  * @param  bEpNum: Endpoint Number.
  * @param  wCount: Counter value.
  * @retval None
  */
#define _SetEPTxCount(bEpNum,wCount) (*_pEPTxCount(bEpNum) = wCount)
#define _SetEPRxCount(bEpNum,wCount) {\
    uint16_t *pdwReg = _pEPRxCount(bEpNum); \
    _SetEPCountRxReg(pdwReg, wCount);\
  }

/**
  * @brief  gets counter of the tx buffer.
  * @param  bEpNum: Endpoint Number.
  * @retval Counter value
  */
#define _GetEPTxCount(bEpNum)((uint16_t)(*_pEPTxCount(bEpNum)) & 0x3ff)
#define _GetEPRxCount(bEpNum)((uint16_t)(*_pEPRxCount(bEpNum)) & 0x3ff)

/**
  * @brief  Sets buffer 0/1 address in a double buffer endpoint.
  * @param  bEpNum: Endpoint Number.
  * @param  wBuf0Addr: buffer 0 address.
  * @retval Counter value
  */
#define _SetEPDblBuf0Addr(bEpNum,wBuf0Addr) {_SetEPTxAddr(bEpNum, wBuf0Addr);}
#define _SetEPDblBuf1Addr(bEpNum,wBuf1Addr) {_SetEPRxAddr(bEpNum, wBuf1Addr);}

/**
  * @brief  Sets addresses in a double buffer endpoint.
  * @param  bEpNum: Endpoint Number.
  * @param  wBuf0Addr: buffer 0 address.
  * @param  wBuf1Addr = buffer 1 address.
  * @retval None
  */
#define _SetEPDblBuffAddr(bEpNum,wBuf0Addr,wBuf1Addr) { \
    _SetEPDblBuf0Addr(bEpNum, wBuf0Addr);\
    _SetEPDblBuf1Addr(bEpNum, wBuf1Addr);\
  } /* _SetEPDblBuffAddr */

/**
  * @brief  Gets buffer 0/1 address of a double buffer endpoint.
  * @param  bEpNum: Endpoint Number.
  * @retval None
  */
#define _GetEPDblBuf0Addr(bEpNum) (_GetEPTxAddr(bEpNum))
#define _GetEPDblBuf1Addr(bEpNum) (_GetEPRxAddr(bEpNum))

/**
  * @brief  Gets buffer 0/1 address of a double buffer endpoint.
  * @param  bEpNum: Endpoint Number.
  *        bDir: endpoint dir  EP_DBUF_OUT = OUT 
  *         EP_DBUF_IN  = IN 
  * @param  wCount: Counter value 
  * @retval None
  */
#define _SetEPDblBuf0Count(bEpNum, bDir, wCount)  { \
    if(bDir == EP_DBUF_OUT)\
      /* OUT endpoint */ \
    {_SetEPRxDblBuf0Count(bEpNum,wCount);} \
    else if(bDir == EP_DBUF_IN)\
      /* IN endpoint */ \
      *_pEPTxCount(bEpNum) = (uint32_t)wCount;  \
  } /* SetEPDblBuf0Count*/

#define _SetEPDblBuf1Count(bEpNum, bDir, wCount)  { \
    if(bDir == EP_DBUF_OUT)\
      /* OUT endpoint */ \
    {_SetEPRxCount(bEpNum,wCount);}\
    else if(bDir == EP_DBUF_IN)\
      /* IN endpoint */\
      *_pEPRxCount(bEpNum) = (uint32_t)wCount; \
  } /* SetEPDblBuf1Count */

#define _SetEPDblBuffCount(bEpNum, bDir, wCount) {\
    _SetEPDblBuf0Count(bEpNum, bDir, wCount); \
    _SetEPDblBuf1Count(bEpNum, bDir, wCount); \
  } /* _SetEPDblBuffCount  */

/**
  * @brief  Gets buffer 0/1 rx/tx counter for double buffering.
  * @param  bEpNum: Endpoint Number.
  * @retval None
  */
#define _GetEPDblBuf0Count(bEpNum) (_GetEPTxCount(bEpNum))
#define _GetEPDblBuf1Count(bEpNum) (_GetEPRxCount(bEpNum))

/* Exported variables --------------------------------------------------------*/
extern __IO uint16_t wIstr;  /* ISTR register last read value */

/* Exported functions ------------------------------------------------------- */

void USB_Start(void);

void EP_Config(uint8_t ep, uint16_t dir, uint16_t type, uint32_t addr);

void UserToPMABufferCopy(const uint8_t *pbUsrBuf, uint16_t wPMABufAddr, uint16_t wNBytes);
void PMAToUserBufferCopy(uint8_t *pbUsrBuf, uint16_t wPMABufAddr, uint16_t wNBytes);

#endif /* __USB_CORE_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
