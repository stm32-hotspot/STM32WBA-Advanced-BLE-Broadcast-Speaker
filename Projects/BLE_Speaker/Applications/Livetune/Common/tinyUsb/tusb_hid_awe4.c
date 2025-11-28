/**
******************************************************************************
* @file    tusb_awe4_hid.c
* @author  MCD Application Team
* @brief   Interface HID with audio weaver
*
*
*******************************************************************************
* @attention
*
* Copyright (c) 2019(-2022) STMicroelectronics.
* All rights reserved.
*
* This software is licensed under terms that can be found in the LICENSE file
* in the root directory of this software component.
* If no LICENSE file comes with this software, it is provided AS-IS.
*
********************************************************************************
*/
/*


add the following flag CFG_TUD_AWE4_HID=1 to enable this class
*/


/* Includes ------------------------------------------------------------------*/
/*cstat +MISRAC2012-* */
#include "tusb_config.h"
#if CFG_TUD_AWE4_HID 

#include "tinyusb.h"
#include "tusb.h"
#include "usb_descriptors.h"
#include <string.h>
/*cstat -MISRAC2012-* tinyusb not misra compliant */

/* redefine the 4 prototypes rather that include the header to prevent cascade and mixup includes  */

void awe4stm32_hl_transmitTuningData(uint8_t  ep_addr, uint8_t  *pbuf, uint16_t  size);
void awe4stm32_hl_prepareTuningData(uint8_t  ep_addr, uint8_t  *pbuf, uint16_t  size);
void awe4stm32_tuningMsgReceive(int32_t ep);
void awe4stm32_tuningMsgSend(void);


/* private defines -----------------------------------------------------------*/

#define CFG_TUD_AWE4_PACKET_COUNT       64


/* Private typedef -----------------------------------------------------------*/



/* Private variables ---------------------------------------------------------*/

static int32_t         interfacehid1;
static void           *pTinyHidPacket = NULL;
static uint32_t        szTinyHidPacket = 0;
static int8_t           EP_HID_IN;
static int8_t           EP_HID_OUT;


//--------------------------------------------------------------------+
// HID Report Descriptor
//--------------------------------------------------------------------+


static uint8_t const desc_hid_report[] =
{
  HID_USAGE_PAGE_N(HID_USAGE_PAGE_VENDOR, 2), \
  HID_USAGE(1), \
  HID_COLLECTION(HID_COLLECTION_APPLICATION), \
  HID_LOGICAL_MIN(0x00), \
  HID_LOGICAL_MAX(0xff), \
  HID_REPORT_ID(1) \
  HID_REPORT_SIZE(8), \
  HID_REPORT_COUNT(0x37), \
  HID_USAGE(2), \
  HID_INPUT(0x86), \
  HID_REPORT_ID(1) \
  HID_REPORT_SIZE(8), \
  HID_REPORT_COUNT(0x37), \
  HID_USAGE(2), \
  HID_OUTPUT(0x86), \
  HID_COLLECTION_END \
};

/* Private function prototypes -----------------------------------------------*/
uint8_t const *tud_hid_descriptor_report_cb(uint8_t itf);

/* Functions Definition ------------------------------------------------------*/

/* send the packet to the host */

void awe4stm32_hl_transmitTuningData(uint8_t  ep_addr, uint8_t  *pbuf, uint16_t  size)
{
  if (size < CFG_TUD_AWE4_PACKET_COUNT)
  {
    tud_hid_report(0, pbuf, size);
  }

}



/**
 * @brief set the next address to copy data coming from the host
 * @param ep_addr useless
 * @param pbuf  packet address
 * @param size  packet size
 */

void awe4stm32_hl_prepareTuningData(uint8_t  ep_addr, uint8_t  *pbuf, uint16_t  size)
{

  pTinyHidPacket  = pbuf;
  szTinyHidPacket = size;
}






/**
 * @brief Invoked when received GET HID REPORT DESCRIPTOR
 * Application return pointer to descriptor
 * Descriptor contents must exist long enough for transfer to complete
 *
 * @param itf interface
 * @return uint8_t const*
 */
uint8_t const *tud_hid_descriptor_report_cb(uint8_t itf)
{
  (void) itf;
  return desc_hid_report;
}


/**
 * @brief called from the descriptor construction allow to build an interface index
 *
 * @param numInterface  the ref num interface
 */
static void tusb_hid_awe4_add_interface(int32_t *numInterface)
{
  interfacehid1 = *numInterface;
  (*numInterface)++;
}


/**
 * @brief called from the descriptor construction allow add descriptors to the main instance
 *
 * @param pDescriptor  descriptor instance under construction pointer
 */


static void tusb_hid_awe4_descriptor(uint8_t **pDescriptor)
{
  EP_HID_OUT = tusb_allocate_ep();
  EP_HID_IN = tusb_allocate_ep();
  
  TUD_HID_INOUT_DESCRIPTOR_D(*pDescriptor, interfacehid1,TUD_HID_STRING_INDEX, HID_ITF_PROTOCOL_NONE, sizeof(desc_hid_report), EP_HID_OUT, EP_HID_IN, CFG_TUD_AWE4_PACKET_COUNT, 1);
}


/**
 * @brief called at the init to register the class
 *
 */


void tusb_hid_awe4_init(void)
{
  if(tusb_handle()->hConfig.mount & CFG_AWE4_HID_ENABLED)
  {
    interfacehid1 = 0;
    pTinyHidPacket = NULL;
    szTinyHidPacket = 0;
    tud_extra_class_add(tusb_hid_awe4_add_interface, tusb_hid_awe4_descriptor);
  }
}



/**
 * @brief the host request a packet
 *
 *  function Invoked when received SET_REPORT control request or
 *  received data on OUT endpoint ( Report ID = 0, Type = 0 )
 */


void tud_hid_set_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize)
{
  /* if the next location is set, copy the content in the buffer */
  if (pTinyHidPacket && szTinyHidPacket)
  {
    memcpy(pTinyHidPacket, buffer, szTinyHidPacket);
    pTinyHidPacket = NULL;
    szTinyHidPacket = 0;
  }
  awe4stm32_tuningMsgReceive(EP_HID_IN); /* Ep is not used , but we pass it anywait */
}



/**
 * @brief the host notify a packet is received, let signal the queue about this event
 *
 * function Invoked when sent REPORT successfully to host
 *  Application can use this to send the next report
 * Note: For composite reports, report[0] is report ID
 */


void tud_hid_report_complete_cb(uint8_t instance, uint8_t const *report, uint16_t len)
{
  awe4stm32_tuningMsgSend();
}

#endif
