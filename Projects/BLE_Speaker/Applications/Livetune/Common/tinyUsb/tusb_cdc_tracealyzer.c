/**
******************************************************************************
* @file    tusb_cdc_tracealyzer.c
* @author  MCD Application Team
* @brief   connected tracalyzer to the CDC for fast streaming
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
  To add traceAlyzer to a project
  add the following flags to the project
    CFG_TUD_TRACEALYZER_CDC=1
    USE_TRACEALYZER=1
   the folder utilities/TraceRecorder must be configured for \streamports\TinyUsb

  Open Tracealyzer 4
    Open record setting in PFS settings
    set target connection to serial
    set device with the comport attached to the CDC , example COM44 (Windows frendly name is "STM32 Debug CDC (COMXX)"

  it is possible to capture the pfs using python -u dump_cdc.py tracealyzer --com COMXX


*/



/* Includes ------------------------------------------------------------------*/
/*cstat +MISRAC2012-* */
#include "tinyusb.h"
#include "tusb.h"
#include "usb_descriptors.h"

#if CFG_TUD_TRACEALYZER_CDC

/* Private variables ---------------------------------------------------------*/

static int32_t interfacecdc;
static int8_t (*tusb_cdc_tracer_rx_cb)(uint8_t *pBuffer, uint32_t *pszBuffer);

void s_cdc_rx_cb(uint8_t itf);

/**
 * @brief called from the descriptor construction allow to build an interface index
 *
 * @param numInterface  the ref num interface
 */
static void tusb_cdc_tracealyzer_add_interface(int32_t *numInterface)
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


static void tusb_cdc_tracealyzer_descriptor(uint8_t **pDescriptor)
{
  static int8_t EP_NUM_CDC_TRACEALYZER_NOTIF;
  static int8_t EP_NUM_CDC_TRACEALYZER;
  EP_NUM_CDC_TRACEALYZER_NOTIF = tusb_allocate_ep();
  EP_NUM_CDC_TRACEALYZER = tusb_allocate_ep();
  TUD_CDC_DESCRIPTOR_D(*pDescriptor, interfacecdc,TUD_CDC_TRACEALYZER_STRING_INDEX /* cdc string */, EP_IN(EP_NUM_CDC_TRACEALYZER_NOTIF), 8, EP_NUM_CDC_TRACEALYZER, EP_IN(EP_NUM_CDC_TRACEALYZER), CFG_TUD_CDC_EP_BUFSIZE);
}




/**
 * @brief Init the instance for the CDC TraceAlyzer
 *
 */

void tusb_cdc_tracealyzer_init(void)
{
  tud_extra_class_add(tusb_cdc_tracealyzer_add_interface, tusb_cdc_tracealyzer_descriptor);
  tud_cdc_add_rx_listeners(s_cdc_rx_cb);
  
}



/**
 * @brief Set the RX callback
 *
 */

int8_t tusb_cdc_tracealyzer_set_callback(int8_t (*cb)(uint8_t *pBuffer, uint32_t *pszBuffer))
{
  tusb_cdc_tracer_rx_cb = cb;
  return 1;
}


// Invoked when CDC interface received data from host
void s_cdc_rx_cb(uint8_t itf)
{
  (void)itf;
  uint8_t buf[64];
  uint32_t count;

  if (tud_cdc_n_available(itf)) // data is available
  {
    count = tud_cdc_n_read(itf, buf, sizeof(buf));
    /* TODO Handle RX data */
    // 01 01 00 00 00 00 fd ff is the sequence to start an analyze
    if (tusb_cdc_tracer_rx_cb)
    {
      tusb_cdc_tracer_rx_cb(buf, &count);
    }
  }
}


/**
 * @brief Write a CDC payload
 *
 * @param pBuffer   buffer to write
 * @param szBuffer  buffer size to write
 * @param pSent     size realy writted
 */


int8_t tusb_cdc_tracealyzer_write(uint8_t *pBuffer, uint32_t szBuffer, int32_t *pSent)
{
  uint32_t szSent = 0;
  /* limits the payload to the max fifo size */
  if (szBuffer > CFG_TUD_CDC_TX_BUFSIZE)
  {
    szBuffer = CFG_TUD_CDC_TX_BUFSIZE;
  }
  while (szSent < szBuffer)
  {
    uint32_t written = tud_cdc_write(pBuffer, szBuffer - szSent);
    pBuffer += written ;
    szSent  += written ;
    /* if the slot written is smaller than the usb bulk size, the fifo is full. Give up to leave the fifo to be consumed in IT */
    if ((szSent != 0) && (written < (uint32_t)CFG_TUD_CDC_EP_BUFSIZE))
    {
      break;
    }
  }
  *pSent = szSent;
  return szSent ? 1 : 0;
}

#endif




