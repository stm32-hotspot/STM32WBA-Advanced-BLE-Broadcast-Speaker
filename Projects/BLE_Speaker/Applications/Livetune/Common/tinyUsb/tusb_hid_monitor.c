/**
******************************************************************************
* @file    tusb_hid_buffer.c
* @author  MCD Application Team
* @brief   capture buffer levels and exposit as hid packets with python
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
#include "tusb_config.h"


/*
  To HID monitor audio buffer to a project
  add the following flags to the project
    CFG_TUD_HID_MONITOR=1

  run the tool audio_monitor.sh
  the graph will display buffer levels play and rec

*/




#if CFG_TUD_HID_MONITOR

#include "tinyusb.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
/*cstat -MISRAC2012-* tinyusb not misra compliant */
#include "tusb.h"
#include "usb_descriptors.h"





/* private defines -----------------------------------------------------------*/

#define CFG_TUD_HID_REFRESH_PERIOD    100U       //python pause(0.1)


/* Private typedef -----------------------------------------------------------*/


/* monitor info struct */

typedef struct
{
  uint16_t play_read;                                   // percent read available for playback
  uint16_t rec_read;                                    // percent read available for record
  uint16_t max_read;                                    // 100
} monitor_info_t;


/* Private variables ---------------------------------------------------------*/

static int32_t interfacehid;
static int8_t  EP_HID_IN;


//--------------------------------------------------------------------+
// HID Report Descriptor
//--------------------------------------------------------------------+

static uint8_t const desc_hid_report[] =
{
  HID_USAGE_PAGE_N(HID_USAGE_PAGE_VENDOR, 2), \
  HID_USAGE(0x01), \
  HID_COLLECTION(HID_COLLECTION_APPLICATION), \
  HID_USAGE(0x02), \
  HID_LOGICAL_MIN(0x00), \
  HID_LOGICAL_MAX_N(0xff, 2), \
  HID_REPORT_SIZE(8), \
  HID_REPORT_COUNT(sizeof(monitor_info_t)), \
  HID_INPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE), \
  HID_COLLECTION_END
};

/* Private function prototypes -----------------------------------------------*/
uint8_t const *tud_hid_descriptor_report_cb(uint8_t itf);


/* Functions Definition ------------------------------------------------------*/

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
 * @brief called from the idle task, send the hid monitor  report
 *
 * @param pHandle
 */
static void tusb_hid_task_mng(tusb_handle_t *pHandle)
{
  if (pHandle->bMounted)
  {
    static uint32_t lastTime = 0;
    static uint32_t  dltTime = 0;
    if (lastTime == 0) { lastTime  = DWT->CYCCNT; }

    dltTime += DWT->CYCCNT - lastTime  ;
    lastTime  = DWT->CYCCNT;

    if (tud_hid_ready() && (dltTime > ((SystemCoreClock / 1000ULL)*CFG_TUD_HID_REFRESH_PERIOD)))
    {
      dltTime = 0;
      monitor_info_t monitor_info;
      monitor_info.play_read  = (pHandle->lvlPlay * 100U) / pHandle->hRbPlay.szBuffer;
      monitor_info.rec_read   = (pHandle->lvlRec * 100U) / pHandle->hRbRec.szBuffer;
      monitor_info.max_read   = 100U;
      tud_hid_report(0, &monitor_info, sizeof(monitor_info));
    }
  }
}





/**
 * @brief called from the descriptor construction allow to build an interface index
 *
 * @param numInterface  the ref num interface
 */
static void s_add_interface(int32_t *numInterface)
{
  
  interfacehid = *numInterface;
  (*numInterface)++;
}


/**
 * @brief called from the descriptor construction allow add descriptors to the main instance
 *
 * @param pDescriptor  descriptor instance under construction pointer
 */


static void s_descriptor(uint8_t **pDescriptor)
{
  EP_HID_IN = tusb_allocate_ep();
  
  TUD_HID_DESCRIPTOR_D(*pDescriptor, interfacehid, TUD_HID_STRING_INDEX, HID_ITF_PROTOCOL_NONE, sizeof(desc_hid_report), (EP_HID_IN | 0x80), CFG_TUD_HID_EP_BUFSIZE, 7);
}


void tusb_hid_monitor_init(void)
{
  if(tusb_handle()->hConfig.mount & CFG_HID_MONITOR_ENABLED)
  {
    tud_extra_class_add(s_add_interface, s_descriptor);
    tud_task_listener_add(tusb_hid_task_mng);
  }
}



// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen)
{
  // TODO not Implemented
  (void) itf;
  (void) report_id;
  (void) report_type;
  (void) buffer;
  (void) reqlen;

  return 0;
}
#endif
