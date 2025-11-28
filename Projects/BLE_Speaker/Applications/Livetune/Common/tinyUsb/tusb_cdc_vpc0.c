/**
******************************************************************************
* @file    tusb_cdc_vpc0.c
* @author  MCD Application Team
* @brief   Implement a VPC  CDC 
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

/* Includes ------------------------------------------------------------------*/
/*cstat +MISRAC2012-* */
#include "tinyusb.h"
#include "tusb.h"
#include "usb_descriptors.h"
#include "stdarg.h"


/*
  	CDC Virtual Port Com (VPC)
    add the following flags to the project
    CFG_TUD_CDC_VPC1=1

*/



#if CFG_TUD_CDC_VPC0==1
#define FIRST_INDEX_CDC_TUD            0

/* Private variables ---------------------------------------------------------*/

static int8_t (*tusb_cdc_vpc0_rx_cb)(uint8_t *pBuffer, uint16_t pszBuffer);
static void s_cdc_rx_cb(uint8_t itf);
static int32_t interfacecdc;
static int8_t EP_NUM_CDC_VPC0_NOTIF;
static int8_t EP_NUM_CDC_VPC0;


/**
 * @brief Set the RX callback
 *
 */

int8_t tusb_cdc_vpc0_set_rx_callback(int8_t (*cb)(uint8_t *pBuffer, uint16_t pszBuffer))
{
  tusb_cdc_vpc0_rx_cb = cb;
  return 1;
}



/**
 * @brief called from the descriptor construction allow to build an interface index
 *
 * @param numInterface  the ref num interface
 */
static void tusb_cdc_vpc0_add_interface(int32_t *numInterface)
{
  interfacecdc = *numInterface;
  (*numInterface)++;
  /* CDC has 2 interface CDC and CDC_DATA , TUD_CDC_DESCRIPTOR assumes that CDC_DATA = CDC+1, but we need to count CDC_DATA in num Interface */
  (*numInterface)++;
}


/**
 * @brief called from the descriptor construction allow add descriptors to the main instance
 *
 * @param pDescriptor  descriptor instance under construction pointer
 */


static void tusb_cdc_vpc0_descriptor(uint8_t **pDescriptor)
{
  EP_NUM_CDC_VPC0_NOTIF = tusb_allocate_ep();
  EP_NUM_CDC_VPC0 = tusb_allocate_ep();
  
  TUD_CDC_DESCRIPTOR_D(*pDescriptor, interfacecdc, TUD_CDC_VPC0_STRING_INDEX /* cdc string */, EP_IN(EP_NUM_CDC_VPC0_NOTIF), 8, EP_NUM_CDC_VPC0, EP_IN(EP_NUM_CDC_VPC0), CFG_TUD_CDC_EP_BUFSIZE);
}

void tusb_cdc_vpc0_init(void)
{
  if(tusb_handle()->hConfig.mount & CFG_CDC_VP0_ENABLED)
  {
    tud_extra_class_add(tusb_cdc_vpc0_add_interface, tusb_cdc_vpc0_descriptor);
    tud_cdc_add_rx_listeners(s_cdc_rx_cb);
  }
}


void tusb_cdc_vpc0_get_info(uint32_t *pComEp,uint32_t *pNotifyEp,uint32_t *pInterface)
{
  if(pComEp) *pComEp = EP_NUM_CDC_VPC0;
  if(pNotifyEp) *pNotifyEp = EP_NUM_CDC_VPC0_NOTIF;
  if(pInterface) *pInterface = interfacecdc;
  
}





// Invoked when CDC interface received data from host
static void s_cdc_rx_cb(uint8_t itf)
{
  (void)itf;
 if(FIRST_INDEX_CDC_TUD ==itf) // notice 0 == first CDC 
  {
    if (tusb_cdc_vpc0_rx_cb && tud_cdc_n_connected(itf))
    {
      uint8_t buf[64];
      uint32_t count;
      while (tud_cdc_n_available(itf)) // data is available
      {
        tusb_lock(true);
        count = tud_cdc_n_read(itf, buf, sizeof(buf));
        tusb_lock(false);
        (void) count;
        /* TODO Handle RX data */
        tusb_cdc_vpc0_rx_cb(buf, count);

      }
    }
  }
}



/**
 * @brief Write a CDC payload
 *
 * @param pBuffer   buffer to write
 * @param szBuffer  buffer size to write
 */


int32_t tusb_cdc_vpc0_write(uint8_t *pBuffer, uint32_t szBuffer, uint8_t flush)
{
  uint32_t count = 0;
  if (tud_ready())
  {
    while (szBuffer)
    {
      tusb_lock(true);
      int32_t written = tud_cdc_n_write(FIRST_INDEX_CDC_TUD,pBuffer, szBuffer);
      tusb_lock(false);
      if(written ==0) 
      {
        break;
      }
      pBuffer += written ;
      szBuffer -= written ;
      count += written ;
    }
    if (flush)
    {
      tusb_lock(true);
      tud_cdc_n_write_flush(FIRST_INDEX_CDC_TUD);
      tusb_lock(false);
    }
    
  }
  return count;
}



#endif




