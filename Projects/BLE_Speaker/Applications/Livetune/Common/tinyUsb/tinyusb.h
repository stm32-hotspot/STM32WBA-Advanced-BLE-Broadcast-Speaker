/**
******************************************************************************
* @file    tinyusb.h
* @author  MCD Application Team
* @brief   tinyusb implementation
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

#ifndef _TINYUSB_H_
#define _TINYUSB_H_

#ifdef __cplusplus
extern "C" {
#endif
/* Includes ------------------------------------------------------------------*/
#include "tusb_config.h"
#include "tusb_rb.h"
#include <stdbool.h>

/* Exported defines -----------------------------------------------------------*/

#define SIZE_TU_ATTR_PACKED(n)                                (uint16_t)(sizeof(int16_t) + ((uint16_t)(n))*3U*sizeof(uint32_t))
#define SET_TU_ATTR_PACKED_LEN(pkt,len)                       {*((int16_t*)pkt)=(int16_t)len;}
#define SET_TU_ATTR_PACKED_CLOCK(pkt, num, min, max,res)      {int32_t *pAttrb = (int32_t *)(((uint8_t *)pkt) + sizeof(int16_t) + ((num) * (3 * sizeof(int32_t))));pAttrb[0] = min;pAttrb[1] = max;pAttrb[2] = res;}

#define ST_RB_PLAY_MS_NUM                                     15UL      /* the usb host feedback algo takes time to converge and need more buffer */
#define ST_RB_REC_MS_NUM                                      8UL
#define ST_FRAME_PACKET_MAX                                   (((CFG_TUD_AUDIO_MAX_SAMPLE_RATE/1000U) + 1U) * CFG_TUD_AUDIO_MAX_CHANNELS * CFG_TUD_AUDIO_MAX_SPLE_SIZE)  /* +1 sample for the feedback */
#define JITTER_REC_COMPENSATION                                2U       /* compensation every n ms for the inhouse algo */
#define JITTER_PLAY_COMPENSATION                               2U       /* compensation every n ms for the usb feedback algo */
#define MAX_TASK_LISTENER                                      4U


#define EP_IN(_ep)  (((uint32_t)(_ep)) | ((uint32_t)0x80U)) // 80 marks a In endpoint      (in,write,rec,Tx)       device  to host   : input 
#define EP_OUT(_ep) (_ep ) // no D7 flags marks a Out endpoint  ( out,read,play,RX ) host    to device : output 


#define   VOLUME_CTRL_0_DB     0
#define   VOLUME_CTRL_10_DB    2560
#define   VOLUME_CTRL_20_DB    5120
#define   VOLUME_CTRL_30_DB    7680
#define   VOLUME_CTRL_40_DB    10240
#define   VOLUME_CTRL_50_DB    12800
#define   VOLUME_CTRL_60_DB    15360
#define   VOLUME_CTRL_70_DB    17920
#define   VOLUME_CTRL_80_DB    20480
#define   VOLUME_CTRL_90_DB    23040
#define   VOLUME_CTRL_100_DB   25600
#define   VOLUME_CTRL_SILENCE  0x8000

/* Exported typedef -----------------------------------------------------------*/

typedef struct
{
  uint8_t  rec_szSple;                              // Samples size in bytes
  uint8_t  rec_enabled;                             // enable record if true
  uint32_t rec_freq;                                // frequency rec in hz
  uint32_t rec_ch;                                  // nb rec channels
  uint32_t rec_deviceType;                          // hots icon  rec device

  uint8_t  play_szSple;                             // Samples size in bytes
  uint8_t  play_enabled;                            // enable play   if true
  uint32_t play_freq;                               // frequency play in hz
  uint32_t play_ch;                                 // nb play channels
  uint32_t play_deviceType;                         // host icon play device

  uint8_t  feature_enabled;                         // enable feature control if true ( volume,mute,...)
  float    timeFrameMs;                             // nb Ms in a frame injection
} tusb_device_audio_conf_t;


#define TUSB_VIDEO_DESC_ID_H264_FRAME_BASED         0                

#ifndef ST_VIDEO_NB_BUFF
#define ST_VIDEO_NB_BUFF             10
#endif



typedef struct tud_device_conf
{
  uint32_t                 pid;
  uint32_t                 mount;
  tusb_device_audio_conf_t uac2_audio;
  #if CFG_TUD_VIDEO != 0
  struct
  {
    int32_t     width;                                // size in pixls
    int32_t     height;                               // size in pixls
    int32_t     bitrateMin;                           
    int32_t     bitrateMax;                           
    int32_t     fps;                                 // nb frame by second
    int32_t     descID;                              // descriptor used id  , 0 = H264 frame based 
  } video;
  #endif
  uint32_t deviceType;
} tusb_device_conf_t;


/* jitter management instance */
typedef struct jitter_buffer_mng_t
{
  ring_buff_t   *pRb;                                  // ring buffer
  uint32_t      frameCount;                            // frame counter
  int32_t       action;                                // if 0 = neutral, 1 = accelerate -1 = slowdown
  uint32_t      compensation;                          // nb frame for skipped with action 0
  int32_t       freq;                                  // stream frequency
  int32_t       delayCompensation;
  int32_t       countCompensation;
} jitter_buffer_mng_t;


/* usb handle instance */
typedef struct tusb_handle_t
{
  tusb_device_conf_t    hConfig;                       // save the config
  ring_buff_t           hRbRec;                        // ring buffer record
  ring_buff_t           hRbPlay;                       // ring buffer playback
  jitter_buffer_mng_t   hJitterRec;                    // jitter record
  uint32_t              countPlayCompensation;         // compensation for the the USB playback
  int8_t                mute;                          // true if muted
  int16_t               volume;                        // volume
  bool                  bMounted;                      // true if mounted
  uint32_t              lvlRec;                        // capture for stats
  uint32_t              lvlPlay;                       // capture for stats

  void (*tTaskListener[MAX_TASK_LISTENER])(struct tusb_handle_t *pHandle);
  uint8_t               nbTaskListener;



} tusb_handle_t;


/* holds variables by channel */
typedef struct tud_audio_info
{
  bool            bStreaming ;                          // streaming opened if true
  bool            bEpOpened;                            // endpoint opened
  uint32_t        Freq;                                 // current frequency
  uint32_t        nbFreqDef;                            // if multifreq nb freq
  uint32_t        tFreqDef[5];                          // if multifreq freque table
  int32_t         szPkt1ms;                             // nb bytes ub the  packet for a frame of 1 ms

} tud_audio_info;






typedef enum tusb_schedule_type
{
  TUSB_SCHEDULE_BY_IT,
  TUSB_SCHEDULE_USR
} tusb_schedule_type;

struct video_probe_and_commit_control_t;

typedef void    (*tusb_midi_listener_cb)(uint32_t port, const void *pPacket, uint32_t szPacket, void *pCookie);
typedef void    (*vs_frame_complete_cb)(uint_fast8_t ctl_idx, uint_fast8_t stm_idx);
typedef int     (*vs_commit_cb)(uint_fast8_t ctl_idx, uint_fast8_t stm_idx,struct video_probe_and_commit_control_t const *parameters);
typedef int32_t (*vs_wait_frame_complete_cb)(void);
                 
                 
struct cdc_line_coding_t ;

/* Exported functions ------------------------------------------------------- */

int32_t               tusb_cdc_vpc0_write(uint8_t *pBuffer, uint32_t szBuffer, uint8_t flush);
int8_t                tusb_cdc_vpc0_set_rx_callback(int8_t (*cb)(uint8_t *pBuffer, uint16_t pszBuffer));
void                  tusb_cdc_vpc0_get_info(uint32_t *pComEp, uint32_t *pNotifyEp, uint32_t *pInterface);

int32_t               tusb_cdc_vpc1_write(uint8_t *pBuffer, uint32_t szBuffer, uint8_t flush);
int8_t                tusb_cdc_vpc1_set_rx_callback(int8_t (*cb)(uint8_t *pBuffer, uint16_t pszBuffer));
void                  tusb_cdc_vpc1_get_info(uint32_t *pComEp, uint32_t *pNotifyEp, uint32_t *pInterface);

uint8_t               tud_cdc_add_rx_listeners(void (*listener_cb)(uint8_t numInterface));
uint8_t               tud_cdc_add_line_coding_listeners(void (*listener_cb)(uint8_t itf, struct cdc_line_coding_t const *p_line_coding));
uint8_t               tud_cdc_add_line_state_listeners(void (*listener_cb)(uint8_t numInterface, bool dtr, bool rts));
void                  tusb_cdc_vpc0_printf(char *pFormat, ...);
int8_t                tusb_cdc_tracealyzer_set_callback(int8_t (*cb)(uint8_t *pBuffer, uint32_t *pszBuffer));
int8_t                tusb_cdc_tracealyzer_write(uint8_t *pBuffer, uint32_t szBuffer, int32_t *pSent);
uint8_t               tud_task_listener_add(void (*task)(struct tusb_handle_t *pHandle));
tusb_handle_t        *tusb_handle(void);
uint8_t               tusb_device_start(void);
uint8_t               tusb_device_stop(void);
uint8_t               tusb_device_init(tusb_device_conf_t *pConf);
void                  tusb_device_deinit(void);
void                  tusb_device_default_conf(tusb_device_conf_t *pConfig);
int8_t                tusb_set_descriptor_string(int32_t index, const char *pString);
void                  tusb_set_schedule_mode(tusb_schedule_type type);
void                  tusb_lock(int8_t bState);




uint8_t               tud_audio_is_rec_streaming(void);
uint8_t               tud_audio_is_play_streaming(void);
uint32_t              tud_audio_get_volume(void);
uint32_t              tud_audio_get_mute(void);
uint32_t              tud_audio_play_frequency(void);
uint32_t              tud_audio_play_channel_count(void);
uint32_t              tud_audio_rec_frequency(void);
uint32_t              tud_audio_rec_channel_count(void);
uint32_t              tud_get_dynamic_descriptor_len(void);
uint8_t               tud_audio_play(void *pDestData, uint32_t szBytes);
uint8_t               tud_audio_rec(void *pDestData, uint32_t szBytes);
uint32_t              tud_audio_get_interface_play(void);
uint32_t              tud_audio_get_interface_rec(void);
void                  tusb_device_get_usb_ids(uint32_t *pVid, uint32_t *pPid);
uint8_t               usb_midi_listener_register(tusb_midi_listener_cb cb, void *pCookie);
uint8_t               usb_midi_listener_unregister(tusb_midi_listener_cb cb);
uint8_t               usb_midi_send_events(uint8_t port, uint8_t const  *pEvents, uint32_t szEvents);
int32_t               tusb_video_h264_write(uint8_t *pBuffer, uint32_t szBuffer);
int32_t               tusb_video_set_cb( vs_frame_complete_cb frame_complete,vs_wait_frame_complete_cb wait_frame_complete,vs_commit_cb vs_commit);
int32_t               tusb_video_push(uint8_t *pBuffer, uint32_t szBuffer);
void                  tud_dump_array(const char *pVariable, void *pData, uint32_t size, int maxLine);
void                  tusb_task_idle(void);



void     tusb_cdc_tracealyzer_init(void);
void     tusb_cdc_vpc0_init(void);
void     tusb_cdc_vpc1_init(void);
void     tusb_hid_monitor_init(void);
void     tusb_hid_awe4_init(void);
void     tusb_uac2_audio_init(void);
void     tusb_midi_in_init(void);
void     tusb_video_uvc_init(void);



#ifdef __cplusplus
}
#endif

#endif /* _TUSB_CONFIG_H_ */
