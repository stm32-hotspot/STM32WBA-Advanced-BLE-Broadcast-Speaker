/**
******************************************************************************
* @file          plugin_com_usb.c
* @author        MCD Application Team
* @brief         init the Usb interface
******************************************************************************
* @attention
*
* Copyright (c) 2018(-2022) STMicroelectronics.
* All rights reserved.
*
* This software is licensed under terms that can be found in the LICENSE file
* in the root directory of this software component.
* If no LICENSE file comes with this software, it is provided AS-IS.
*
******************************************************************************
*/
/* Includes ------------------------------------------------------------------*/

#include <string.h>
#include "tinyusb.h"
#include "tusb.h"
#include "st_os.h"

int8_t av_usb_state(void);
void tusb_lock(int8_t state);
void av_usb_init(void);
void main_print_usb_info(void);
/*
  Overloading the USB service availability
*/
int8_t av_usb_state(void)
{
  return TRUE;
}


static st_task            hUsbTask;
static st_mutex           hMutex;
static int8_t             bInitDone;
//static st_mutex           hVpc0Mutex;
/*
  overload tinyusb lock task
*/
void tusb_lock(int8_t state)
{
  if(bInitDone)
  {
    if(state) st_os_mutex_lock(&hMutex);
    else      st_os_mutex_unlock(&hMutex);
  }
}



/*
  This task is mandatory to trigger the TX FIFO to be sent.
  By default, the task is handled by the USB ISR when there is high traffic of SOF (Start of Frame).
  For the CDC, we can encounter a condition where FIFO packets are locked and not transmitted because tud_task() is not called.
  This task ensures that all packets will be transmitted.
*/
extern uint8_t USB_Write_Flag;
static void task_cb(const void *pCookie)
{
  while(1)
  {
    HAL_NVIC_SetPendingIRQ(GPIO_TUSB_EXTI_IRQn);
    //tud_task();
    st_os_task_delay(1);
  }
}





void av_usb_init(void)
{

  /* make sure we have the default configuration */

  tusb_device_conf_t hTusbConfig;
  tusb_device_default_conf(&hTusbConfig);

  /* change application name and CDC point name */

  tusb_set_descriptor_string(11,"Stm32 Capture VEnc");
  tusb_set_descriptor_string(2 ,"Stm32 Speaker Livetune");
  /*
    by default, all classes are mounted as soon as the define is set,
    and the descriptor is sent to the host.
    Here, we need to mount the descriptor conditionally ( flags livetune_vpc).
    Therefore, we set/reset the mount flag according to the condition.*/

  /*if(!(av_get_persistent_flags() & ST_FLAGS_LIVETUNE_VPC))
  {
    hTusbConfig.mount &= ~CFG_CDC_VP0_ENABLED;
  }
  else
  {
    tusb_set_descriptor_string(7,"LiveTune Connect Usb");
  }*/
  tusb_device_init(&hTusbConfig);
  tusb_device_start();
  bInitDone = TRUE;

  /* small delay for the descriptor negotiation (useless ?)*/
  HAL_Delay(100);

  /* We use a CDC high speed, we need to schudule manually tud_task to prevent packet lost */

  ST_VERIFY(st_os_task_create(&hUsbTask,"tud_task()",task_cb,NULL,600,ST_Priority_High) == ST_OS_OK);
  ST_VERIFY(st_os_mutex_create(&hMutex) == ST_OS_OK);
  /* switch the schedule mod by task rather than by IT */

//  tusb_set_schedule_mode(TUSB_SCHEDULE_USR);
//  printf("USB ready\n");
//  while(1) st_os_task_delay(100);


}


void main_print_usb_info(void)
{
  uint32_t vid, pid;
  tusb_device_get_usb_ids(&vid, &pid);
  ST_TRACE_INFO("\tUSB VID:PID : %d:%d", vid, pid);
}


