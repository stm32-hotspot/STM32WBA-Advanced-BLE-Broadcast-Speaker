/**
******************************************************************************
* @file    tusb_device.c
* @author  MCD Application Team
* @brief   manage the device usb
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


#include <stdio.h>
#include <string.h>
/*cstat -MISRAC2012-* tinyusb not misra compliant */
#include "tusb.h"
#include "usb_descriptors.h"

/*cstat +MISRAC2012-* */
#include "tinyusb.h"
#include <stdarg.h>

#if defined(__ICCARM__)
  /* New definition from EWARM V9, compatible with EWARM8 */
  int putchar(int ch);
  #define PUTCHAR(ch) putchar(ch)
#elif defined(__GNUC__)
  int __io_putchar(int ch);
  #define PUTCHAR(ch) __io_putchar(ch)
#endif /* __ICCARM__ */




/* Private prototypes -----------------------------------------------------------*/




/* Private variables ---------------------------------------------------------*/
static tusb_handle_t    hUsb;   /* Usb instance */

/**
* @brief implement a uart log callable from interrupt
*
*/

int  tud_log(const char *pFormat, ...)
{
  int32_t len;
  va_list args;
  va_start(args, pFormat);
  char tBuffer[128];
  len = vsnprintf(tBuffer, sizeof(tBuffer) - 1, pFormat, args);
  for (int a = 0 ; tBuffer[a] ; a++)
  {
    PUTCHAR(tBuffer[a]);
  }
  va_end(args);
  return len;
}


/**
* @brief return the usb handle  singleton
*
*/

tusb_handle_t *tusb_handle(void)
{
  return &hUsb;
}


/**
*   @brief If TUSB_SCHEDULE_BY_IT is set, the tud_task() function will be called automatically in the ISR (default case).
*   If TUSB_SCHEDULE_USR is set, it is up to the application to call tud_task() regularly in order to send pending USB FIFO data.
*   The TUSB_SCHEDULE_USR mode should be preferred when the application uses a CDC  with high speed large packets).
*/

void tusb_set_schedule_mode(tusb_schedule_type type)
{
  tusb_hw_enable_schedule((type == TUSB_SCHEDULE_BY_IT) ? TRUE : FALSE);
}



/**
 * @brief The TinyUSB implementation is bare metal but can be used with an operating system.
 * In this case, we need to prevent concurrent calls to TinyUSB functions.
 * Each time a class needs to access TinyUSB, the function calls tusb_lock to give the caller the opportunity to lock and unlock the TinyUSB task.
 *
 * @param bState   mutex lock state
 */

__weak void tusb_lock(int8_t bState)
{


}

/**
* @brief Add a task listener
*
*/

uint8_t tud_task_listener_add(void (*task)(struct tusb_handle_t *pHandle))
{
  hUsb.tTaskListener[hUsb.nbTaskListener] = task;
  hUsb.nbTaskListener++;
  TU_ASSERT(hUsb.nbTaskListener <  MAX_TASK_LISTENER);
  return 1;
}

/**
* @brief fill a default configuration
*
* @param pConfig config pointer
*/

void tusb_device_default_conf(tusb_device_conf_t *pConfig)
{
  memset(pConfig, 0, sizeof(*pConfig));

  pConfig->mount = (uint32_t) -1;       /* by default all device are mounted if the define is set */
  #if CFG_TUD_UAC2_AUDIO != 0
  pConfig->uac2_audio.rec_szSple         = sizeof(int16_t);
  pConfig->uac2_audio.rec_freq           = 16000;
  pConfig->uac2_audio.rec_ch             = 2;
  pConfig->uac2_audio.play_szSple        = sizeof(int16_t);
  pConfig->uac2_audio.play_freq          = 16000;
  pConfig->uac2_audio.play_ch            = 2;
  pConfig->uac2_audio.rec_deviceType     = (uint32_t)AUDIO_TERM_TYPE_IN_GENERIC_MIC;
  pConfig->uac2_audio.play_deviceType    = (uint32_t)AUDIO_TERM_TYPE_OUT_GENERIC_SPEAKER;
  pConfig->uac2_audio.rec_enabled        = true;
  pConfig->uac2_audio.play_enabled       = true;
  pConfig->uac2_audio.feature_enabled    = false; /* disable control by default */
  pConfig->uac2_audio.timeFrameMs        = 1.0f;     /* default injection size 1 ms ( used for the buffer size computation ) */

  pConfig->deviceType      = (uint32_t)AUDIO_FUNC_SOUND_RECODER;
  #endif
  #if CFG_TUD_VIDEO != 0
  pConfig->video.width        = 800;
  pConfig->video.height       = 480;
  pConfig->video.fps          = 30;
  pConfig->video.bitrateMin   = (pConfig->video.width * pConfig->video.height * 16U * (pConfig->video.fps));
  pConfig->video.bitrateMax   = (pConfig->video.width * pConfig->video.height * 16U * (pConfig->video.fps));
  pConfig->video.descID       = TUSB_VIDEO_DESC_ID_H264_FRAME_BASED;

  #endif
}


TU_ATTR_WEAK void     tusb_cdc_tracealyzer_init(void)
{
}
TU_ATTR_WEAK void     tusb_cdc_vpc0_init(void)
{
}
TU_ATTR_WEAK void     tusb_hid_monitor_init(void)
{
}
TU_ATTR_WEAK void     tusb_hid_awe4_init(void)
{
}
TU_ATTR_WEAK void     tusb_uac2_audio_init(void)
{
}

TU_ATTR_WEAK void    tusb_midi_in_init(void)
{
}


TU_ATTR_WEAK void    tusb_video_uvc_init(void)
{
}


TU_ATTR_WEAK  void     tusb_cdc_vpc1_init(void)
{
}






/**
* @brief return  USB IDS
*
* @param pVid void  pointer
* @param pPid void  pointer
*/


void tusb_device_get_usb_ids(uint32_t *pVid, uint32_t *pPid)
{
  if (pVid)
  {
    *pVid = USBD_VID;
  }

  if (pPid)
  {
    *pPid = tud_descriptor_get_descriptor_device()->idProduct;
  }
}




/**
* @brief init the audio usb instance
*
* @param pConfig config pointer
* @return uint8_t
*/

uint8_t tusb_device_init(tusb_device_conf_t *pConfig)
{
  hUsb.hConfig = *pConfig;
  /* register classes */

  //tusb_uac2_audio_init();
  //tusb_cdc_tracealyzer_init();
  //tusb_hid_monitor_init();
  //tusb_hid_awe4_init();
  //tusb_midi_in_init();
  tusb_cdc_vpc0_init();
  //tusb_video_uvc_init();
  //tusb_cdc_vpc1_init();


  tud_build_dynamic_descriptor();
  /* init the HW support */
  tusb_hw_config();
  /* create the tinyusb instance */
  uint8_t result = tud_init(BOARD_TUD_RHPORT) ? 1U : 0U;
  /* Wait a bit to make sure nobody will disable IRQ during the host acknowledgment. */
  HAL_Delay(20);
  return result ;
}

/**
* @brief deinit the audio usb instance
*
*/

void tusb_device_deinit(void)
{
  tusb_device_stop();
  // TODO create a callback deinit
  s_rb_term(&hUsb.hRbRec);
  s_rb_term(&hUsb.hRbPlay);
}


/**
* @brief Start the USB
*
* @return uint8_t
*/

uint8_t  tusb_device_start(void)
{
  return (uint8_t)tusb_hw_device_start(tusb_task_idle);
}
/**
* @brief  Stop  the USB
*
* @return uint8_t
*/
uint8_t  tusb_device_stop(void)
{
  return tusb_hw_device_stop();
}

/**
* @brief  tusb idl, this stack in in charge to do the negotiation at the boot up and dispatch fifo messages
*/

void tusb_task_idle(void)
{
  tud_task();
  for (uint32_t count = 0; count < hUsb.nbTaskListener ; count++)
  {
    hUsb.tTaskListener[count](&hUsb);
  }
}

#if CFG_TUD_HID
// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
TU_ATTR_WEAK void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize)
{
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
TU_ATTR_WEAK uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen)
{
  return 0;
}


#endif

/**
* @brief Invoked when device is mounted
*
*/

void tud_mount_cb(void)
{
  TU_LOG1("tud_mount_cb\r\n");
  hUsb.bMounted = true;
}

/**
* @brief Invoked when device is unmounted
*
*/
void tud_umount_cb(void)
{
  TU_LOG1("tud_umount_cb\r\n");
  hUsb.bMounted = false;
}



#if (CFG_TUD_CDC_VPC0 != 0 ) || ( CFG_TUD_CDC_VPC1 != 0)

static void (*tCdcRxListerners[2])(uint8_t numInterface);
static int8_t iCdcRxListener;
static void (*tCdcLineStateListerners[2])(uint8_t numInterface, bool dtr, bool rts);
static int8_t iCdcLineStateListener;
static void (*tCdcLineCodingListerners[2])(uint8_t numInterface, struct  cdc_line_coding_t const *p_line_coding);
static int8_t iCdcLineCodingListener;



uint8_t tud_cdc_add_rx_listeners(void (*listener_cb)(uint8_t numInterface))
{
  TU_ASSERT(iCdcRxListener <  2);
  tCdcRxListerners[iCdcRxListener] = listener_cb;
  iCdcRxListener++;
  return 1;
}


uint8_t tud_cdc_add_line_state_listeners(void (*listener_cb)(uint8_t numInterface, bool dtr, bool rts))
{
  TU_ASSERT(iCdcLineStateListener <  2);
  tCdcLineStateListerners[iCdcLineStateListener] = listener_cb;
  iCdcLineStateListener++;
  return 1;
}


uint8_t tud_cdc_add_line_coding_listeners(void (*listener_cb)(uint8_t itf, struct cdc_line_coding_t const *p_line_coding))
{
  TU_ASSERT(iCdcLineCodingListener <  2);
  tCdcLineCodingListerners[iCdcLineCodingListener] = listener_cb;
  iCdcLineCodingListener++;
  return 1;
}



void tud_cdc_rx_cb(uint8_t itf)
{
  for (int32_t index = 0; index < iCdcRxListener; index++)
  {
    tCdcRxListerners[index](itf);
  }
}


void tud_cdc_line_coding_cb(uint8_t itf, cdc_line_coding_t const *p_line_coding)
{
  for (int32_t index = 0; index < iCdcLineCodingListener; index++)
  {
    tCdcLineCodingListerners[index](itf, (struct cdc_line_coding_t const *)p_line_coding);
  }
}


// Invoked when cdc when line state changed e.g connected/disconnected
// DTR Data Terminal Ready (DTR) is a control signal in RS-232 serial communications,
// transmitted from data terminal equipment (DTE), such as a computer,
// to data communications equipment (DCE), for example a modem,
// to indicate that the terminal is ready for communications and the modem may initiate a communications channel.
// RTS:  (Request to Send): This signal is used by a Data Terminal Equipment (DTE) device,
// such as a computer, to indicate that it is ready to receive data from the Data Communication Equipment (DCE),
// such as a modem. When the RTS signal is asserted (set to a high state),
// it informs the DCE that the DTE is ready to receive data.

void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts)

{
  for (int32_t index = 0; index < iCdcLineStateListener; index++)
  {
    tCdcLineStateListerners[index](itf, dtr, rts);
  }
}

#endif


#if CFG_TUSB_DEBUG != 0


/**
* @brief  Invoked when usb bus is suspended
remote_wakeup_en : if host allow us  to perform remote wakeup
Within 7ms, device must draw an average of current less than 2.5 mA from bus
*
*/


void tud_suspend_cb(bool remote_wakeup_en)
{
  (void)remote_wakeup_en;
  TU_LOG1("tud_suspend_cb low power not supported yet\r\n");
}

/**
* @brief Invoked when usb bus is resumed
*
*/
void tud_resume_cb(void)
{
  TU_LOG1("tud_resume_cb low power not supported yet\r\n");
}

#endif
