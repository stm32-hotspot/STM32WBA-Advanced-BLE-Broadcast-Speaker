/**
******************************************************************************
* @file    tusb_audio.c
* @author  MCD Application Team
* @brief   audio streaming mng
*
* Tiny USB provides an unified direct access to HW FIFOs and assumes that
* the application sends the packet right  on time according to
* the frame length described in the audio descriptor and classes .
* This means that there is no latency and jitter buffer able to compensate
* the small drift between a packet received from a capture thread
* at the exact time to feed the FIFO.
*
* If we just use tud_audio_write() to send a packet as soon as
* the capture thread gets its frame every ms, we can observe
* pops every ~5 secs due clock drift.
* To solve this problem, we need to add a latency buffer as a ring buffer + a jitter management that read more or less samples to prevent overflow and underflow
* The same thing is done for the playback, we use the same system but the jitter management is done using the feedback end point
*
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
#include <stdio.h>
#include <string.h>
/*cstat -MISRAC2012-* tinyusb not misra compliant */
#include "tinyusb.h"
#include "tusb.h"
#include "usb_descriptors.h"

#if CFG_TUD_UAC2_AUDIO==1

/*cstat +MISRAC2012-* */

//   Calculate Endpoints wMaxPacketSize

#define UAC2_EP_SIZE(speed,_maxFrequency, _nBytesPerSample, _nChannels) \
    ((((_maxFrequency + ((speed) ? 8000 : 1000)) / ((speed) ? 8000 : 1000))) * _nBytesPerSample * _nChannels)
      



/* Private prototypes -----------------------------------------------------------*/

static bool s_audio_init_ring_buffers(void);

/* Private variables ---------------------------------------------------------*/

static tud_audio_info   tAudioInfo[2];
static uint32_t lenAudioDescriptor;
static int32_t interfaceControl = 0;
static int32_t interfaceplay;
static int32_t interfacerec;
static int32_t nbAudioInterface ;
static int8_t EP_NUM_AUDIO_REC;
static int8_t EP_NUM_AUDIO_PLAY;
static int8_t EP_NUM_AUDIO_PLAY_FB;

/**
 * @brief called from the descriptor construction allow to build an interface index
 *
 * @param numInterface  the ref num interface
 */
static void s_uac2_audio_add_interface(int32_t *numInterface)
{
  interfaceControl = 0;
  interfaceControl = (*numInterface)++;

  if (tusb_handle()->hConfig.uac2_audio.play_enabled)
  {
    interfaceplay = (*numInterface)++; ;
  }

  if (tusb_handle()->hConfig.uac2_audio.rec_enabled)
  {
    interfacerec = (*numInterface)++;
  }
  nbAudioInterface = (*numInterface);
}


/**
 * @brief called from the descriptor construction allow add descriptors to the main instance

    To create an audio descriptor, we must first define the device with TUD_CONFIG_DESCRIPTOR_D
    then we need to create a sub-packet including terminal property,Interface Association Descriptor (IAD),it start by TUD_AUDIO_DESC_CS_AC_D , this descriptor include the size of the sub-packet
    properties are: frequency, channels, type of EP ( streaming), features ( volume, mute), etc...
    the couple of macros DESC_START_LEN/DESC_STOP_LEN_16 allows to compute cleanly the sub-packet size.
    finally, we must create endpoints.

    An endpoint is attached to an interface ( Mic and Spk), and can carry several channels
    in each endpoint, we have to define the terminal attached, the audio version used, the format (PCM,16 bits, 2 bytes,etc..), the frame size , etc...
    The first descriptor TUD_CONFIG_DESCRIPTOR_D include the size of the whole packet descriptor.


     Capture from wireshark


          1             0.000000            host       3.13.0   USB        36           GET DESCRIPTOR Request DEVICE
          2             0.000000            3.13.0   host       USB        46           GET DESCRIPTOR Response DEVICE
          3             0.000000            host       3.13.0   USB        36           GET DESCRIPTOR Request CONFIGURATION
          4             0.000000            3.13.0   host       USB        247          GET DESCRIPTOR Response CONFIGURATION
          5             0.000000            host       3.13.0   USB        36           SET CONFIGURATION Request
          6             0.000000            3.13.0   host       USB        28           SET CONFIGURATION Response

          Frame 4: 247 bytes on wire (1976 bits), 247 bytes captured (1976 bits) on interface \\.\USBPcap3, id 0
          USB URB
          CONFIGURATION DESCRIPTOR
          INTERFACE ASSOCIATION DESCRIPTOR
          INTERFACE DESCRIPTOR (0.0): class Audio
          Class-specific Audio Control Interface Descriptor: Header Descriptor
          Class-specific Audio Control Interface Descriptor: Clock source descriptor
          Class-specific Audio Control Interface Descriptor: Output terminal descriptor
          Class-specific Audio Control Interface Descriptor: Input terminal descriptor
          Class-specific Audio Control Interface Descriptor: Clock source descriptor
          Class-specific Audio Control Interface Descriptor: Output terminal descriptor
          Class-specific Audio Control Interface Descriptor: Input terminal descriptor
          INTERFACE DESCRIPTOR (1.0): class Audio
          INTERFACE DESCRIPTOR (1.1): class Audio
          Class-specific Audio Streaming Interface Descriptor: General AS Descriptor
          Class-specific Audio Streaming Interface Descriptor: Format type descriptor
          ENDPOINT DESCRIPTOR
          Class-specific Audio Streaming Endpoint Descriptor
          INTERFACE DESCRIPTOR (2.0): class Audio
          INTERFACE DESCRIPTOR (2.1): class Audio
          Class-specific Audio Streaming Interface Descriptor: General AS Descriptor
          Class-specific Audio Streaming Interface Descriptor: Format type descriptor
          ENDPOINT DESCRIPTOR
          Class-specific Audio Streaming Endpoint Descriptor

 *
 * @param pDescriptor  descriptor instance under construction pointer
 */
static void s_uac2_audio_descriptor(uint8_t **pDescriptor)
{
  DESC_LEN hASLen;
  EP_NUM_AUDIO_REC = tusb_allocate_ep();
  EP_NUM_AUDIO_PLAY = tusb_allocate_ep();
  EP_NUM_AUDIO_PLAY_FB = tusb_allocate_ep();

  uint32_t  _epin  = EP_IN(EP_NUM_AUDIO_REC);     // device  to host   : input rec
  uint32_t  _epout = EP_OUT(EP_NUM_AUDIO_PLAY);  // host to device     : output play

  UAC2_DESC_IAD_D(*pDescriptor,/*_firstitf*/ interfaceControl, /*_nitfs ITF_AUD_NUM_TOTAL*/ nbAudioInterface, /*_stridx*/ 0);
  UAC2_DESC_STD_AC_D(*pDescriptor,/*_itfnum*/ interfaceControl, /*_nEPs*/ 0x00, /*_stridx*/ TUD_PRODUCT_STRING_INDEX);

  /* Start Len Computation (_totallen)*/
  DESC_START_LEN(hASLen, *pDescriptor);
  UAC2_DESC_CS_AC_D(*pDescriptor,/*_bcdADC*/ 0x0200, /*_category*/ tusb_handle()->hConfig.deviceType, /*_totallen*/ -1, /*_ctrl*/ AUDIO_CS_AS_INTERFACE_CTRL_LATENCY_POS);
  int32_t  featureId = UAC2_ENTITY_PLAY_INPUT_TERMINAL;

  if (tusb_handle()->hConfig.uac2_audio.play_enabled)
  {
    /* The device has a clock */
    /* Change the icon on the PC desktop*/
    /* Set the In and Out as USB streaming */
    /* optionally, add a feature unit mute and volume e */
    if (tusb_handle()->hConfig.uac2_audio.feature_enabled)
    {
      featureId = UAC2_ENTITY_PLAY_FEATURE_UNIT;
      /* mute & volume master only */
      UAC2_DESC_FEATURE_UNIT_TWO_CHANNEL_D(*pDescriptor,
                                                /*_unitid*/ featureId,
                                                /*_srcid*/ UAC2_ENTITY_PLAY_INPUT_TERMINAL,
                                                /*_ctrlch0master*/ (AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_MUTE_POS | AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_VOLUME_POS),
                                                /*_ctrlch1*/ (AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_MUTE_POS | AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_VOLUME_POS),
                                                /*_ctrlch2 (AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_MUTE_POS | AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_VOLUME_POS)*/ 0,
                                                /*_stridx*/ 0x00);
    }

    /* the device has a clock sync control */
    UAC2_DESC_CLK_SRC_D(*pDescriptor,
                             /*_clkid*/ UAC2_ENTITY_CLOCK_PLAY,
                             /*_attr*/ AUDIO_CLOCK_SOURCE_ATT_INT_PRO_CLK,
                             /*_ctrl*/ (AUDIO_CTRL_R << AUDIO_CLOCK_SOURCE_CTRL_CLK_FRQ_POS),
                             /*_assocTerm*/ interfaceplay,
                             /*_stridx*/ 0x00);

    UAC2_DESC_INPUT_TERM_D(*pDescriptor,
                                /*_termid*/ UAC2_ENTITY_PLAY_INPUT_TERMINAL,
                                /*_termtype*/   AUDIO_TERM_TYPE_USB_STREAMING,
                                /*_assocTerm*/ 0x00,
                                /*_clkid*/ UAC2_ENTITY_CLOCK_PLAY,
                                /*_nchannelslogical*/ tusb_handle()->hConfig.uac2_audio.rec_ch,
                                /*_channelcfg*/ AUDIO_CHANNEL_CONFIG_NON_PREDEFINED,
                                /*_idxchannelnames*/ 0x00, /*_ctrl*/ 0 * (AUDIO_CTRL_R << AUDIO_IN_TERM_CTRL_CONNECTOR_POS),
                                /*_stridx*/ 0x00);

    UAC2_DESC_OUTPUT_TERM_D(*pDescriptor,
                                 /*_termid*/ UAC2_ENTITY_PLAY_OUTPUT_TERMINAL,
                                 /*_termtype*/ tusb_handle()->hConfig.uac2_audio.play_deviceType,
                                 /*_assocTerm*/ 0x00, 
                                 /*_srcid*/ featureId,
                                 /*_clkid*/ UAC2_ENTITY_CLOCK_PLAY,
                                 /*_ctrl*/ 0x0000,
                                 /*_stridx*/ 0x00);


  }

  if (tusb_handle()->hConfig.uac2_audio.rec_enabled)
  {
    /* Set the device type for in and out and device, change the icon on the PC desktop, set the control volume and mute capabilities for the SPK */
    /* set the In and Out as USB streaming */
    UAC2_DESC_CLK_SRC_D(*pDescriptor,
                             /*_clkid*/ UAC2_ENTITY_CLOCK_REC,
                             /*_attr*/ AUDIO_CLOCK_SOURCE_ATT_INT_PRO_CLK,
                             /*_ctrl*/ (AUDIO_CTRL_R << AUDIO_CLOCK_SOURCE_CTRL_CLK_FRQ_POS),
                             /*_assocTerm*/ interfacerec,
                             /*_stridx*/ 0x00);

    UAC2_DESC_OUTPUT_TERM_D(*pDescriptor,
                                 /*_termid*/ UAC2_ENTITY_REC_OUTPUT_TERMINAL,
                                 /*_termtype*/ AUDIO_TERM_TYPE_USB_STREAMING,
                                 /*_assocTerm*/ 0x00,
                                 /*_srcid*/ UAC2_ENTITY_REC_INPUT_TERMINAL,
                                 /*_clkid*/ UAC2_ENTITY_CLOCK_REC,
                                 /*_ctrl*/ 0x0000,
                                 /*_stridx*/ 0x00);

    UAC2_DESC_INPUT_TERM_D(*pDescriptor,
                                /*_termid*/ UAC2_ENTITY_REC_INPUT_TERMINAL,
                                /*_termtype*/ tusb_handle()->hConfig.uac2_audio.rec_deviceType,
                                /*_assocTerm*/ 0x00,
                                /*_clkid*/ UAC2_ENTITY_CLOCK_REC,
                                /*_nchannelslogical 0x01*/tusb_handle()->hConfig.uac2_audio.rec_ch,
                                /*_channelcfg*/ AUDIO_CHANNEL_CONFIG_NON_PREDEFINED,
                                /*_idxchannelnames*/ 0x00,
                                /*_ctrl*/ 0 * (AUDIO_CTRL_R << AUDIO_IN_TERM_CTRL_CONNECTOR_POS),
                                /*_stridx*/ 0x00);
  }
  /* Finalize the variable len, pacth a  16 bits len at the offset 6 */
  DESC_STOP_LEN_16(hASLen, *pDescriptor, 6, 0);


  if (tusb_handle()->hConfig.uac2_audio.play_enabled)
  {
    /* define End point for interface  1.0 and 1.1 ie :interfaceplay ch 1 and 2 */
    UAC2_DESC_STD_AS_INT_D(*pDescriptor,
                                /*_itfnum*/ (uint8_t)(interfaceplay),
                                /*_altset*/ 0x00,
                                /*_nEPs*/ 0x00,
                                /*_stridx*/ TUD_PLAY_STRING_INDEX);

    UAC2_DESC_STD_AS_INT_D(*pDescriptor,
                                /*_itfnum*/ (uint8_t)(interfaceplay),
                                /*_altset*/ 0x01,
                                /*_nEPs*/ (CFG_TUD_AUDIO_ENABLE_FEEDBACK_EP == 1) ? 2 /* stream + feedback*/ : 1 /* a single endpoint */,
                                /*_stridx*/ TUD_PLAY_STRING_INDEX);

    UAC2_DESC_CS_AS_INT_D(*pDescriptor,
                               /*_termid            */ UAC2_ENTITY_PLAY_INPUT_TERMINAL,
                               /*_ctrl              */ AUDIO_CTRL_NONE,
                               /*_formattype        */ AUDIO_FORMAT_TYPE_I,
                               /*_formats           */ AUDIO_DATA_FORMAT_TYPE_I_PCM,
                               /*_nchannelsphysical */ tusb_handle()->hConfig.uac2_audio.play_ch,
                               /*_channelcfg        */ AUDIO_CHANNEL_CONFIG_NON_PREDEFINED,
                               /*_stridx*/ TUD_PLAY_STRING_INDEX);

    UAC2_DESC_TYPE_I_FORMAT_D(*pDescriptor,
                                   tusb_handle()->hConfig.uac2_audio.play_szSple,
                                   tusb_handle()->hConfig.uac2_audio.play_szSple * 8U);

    UAC2_DESC_STD_AS_ISO_EP_D(*pDescriptor,
                                   /*_ep*/ _epout,
                                   /*_attr*/ (uint8_t)((uint8_t)TUSB_XFER_ISOCHRONOUS | (uint8_t)TUSB_ISO_EP_ATT_ASYNCHRONOUS | (uint8_t)TUSB_ISO_EP_ATT_DATA),
                                   /*_maxEPsize*/  UAC2_EP_SIZE((CFG_TUD_MAX_SPEED == OPT_MODE_HIGH_SPEED),CFG_TUD_AUDIO_MAX_SAMPLE_RATE, tusb_handle()->hConfig.uac2_audio.play_szSple, tusb_handle()->hConfig.uac2_audio.play_ch),
                                   /*_interval*/ 0x01);

    UAC2_DESC_CS_AS_ISO_EP_D(*pDescriptor,
                                  /*_attr*/ AUDIO_CS_AS_ISO_DATA_EP_ATT_NON_MAX_PACKETS_OK,
                                  /*_ctrl*/ AUDIO_CTRL_NONE,
                                  /*_lockdelayunit*/ AUDIO_CS_AS_ISO_DATA_EP_LOCK_DELAY_UNIT_MILLISEC,
                                  /*_lockdelay*/ 0x0001);

    #if CFG_TUD_AUDIO_ENABLE_FEEDBACK_EP != 0
    /* Standard AS Isochronous Feedback Endpoint Descriptor(4.10.2.1) */
    UAC2_DESC_STD_AS_ISO_FB_EP_D(*pDescriptor,/*_ep*/ EP_IN(EP_NUM_AUDIO_PLAY_FB),/*_epfbsize*/4,  /*_interval */ TUD_OPT_HIGH_SPEED ? 4 : 1);
    #endif

  }

  if (tusb_handle()->hConfig.uac2_audio.rec_enabled)
  {
    /* define End point for interface 2.0 and 2.1 ie : interfacerec ch 1 and 2 */
    UAC2_DESC_STD_AS_INT_D(*pDescriptor,
                                /*_itfnum*/ (uint8_t)(interfacerec),
                                /*_altset*/ 0x00,
                                /*_nEPs*/ 0x00,
                                /*_stridx*/ TUD_REC_STRING_INDEX);

    UAC2_DESC_STD_AS_INT_D(*pDescriptor,
                                /*_itfnum*/ (uint8_t)(interfacerec),
                                /*_altset*/ 0x01,
                                /*_nEPs*/ 1 /* a single endpoint */,
                                /*_stridx*/ TUD_REC_STRING_INDEX);

    UAC2_DESC_CS_AS_INT_D(*pDescriptor,
                               /*_termid*/ UAC2_ENTITY_REC_OUTPUT_TERMINAL,
                               /*_ctrl*/ AUDIO_CTRL_NONE,
                               /*_formattype*/ AUDIO_FORMAT_TYPE_I,
                               /*_formats*/ AUDIO_DATA_FORMAT_TYPE_I_PCM,
                               /*_nchannelsphysical*/ tusb_handle()->hConfig.uac2_audio.rec_ch,
                               /*_channelcfg*/ AUDIO_CHANNEL_CONFIG_NON_PREDEFINED,
                               /*_stridx*/ TUD_REC_STRING_INDEX);

    UAC2_DESC_TYPE_I_FORMAT_D(*pDescriptor,
                                   tusb_handle()->hConfig.uac2_audio.rec_szSple,
                                   tusb_handle()->hConfig.uac2_audio.rec_szSple * 8U);

    UAC2_DESC_STD_AS_ISO_EP_D(*pDescriptor,
                                   /*_ep*/ _epin,
//                                   /*_attr*/ (uint8_t)((uint8_t)TUSB_XFER_ISOCHRONOUS | (uint8_t)TUSB_ISO_EP_ATT_ASYNCHRONOUS | (uint8_t)TUSB_ISO_EP_ATT_DATA),
                                   /*_attr*/ (uint8_t)((uint8_t)TUSB_XFER_ISOCHRONOUS | ((CFG_TUD_AUDIO_ENABLE_FEEDBACK_EP==1) ? (uint8_t)TUSB_ISO_EP_ATT_ASYNCHRONOUS : (uint8_t)TUSB_ISO_EP_ATT_NO_SYNC)),
                                   /*_maxEPsize*/ UAC2_EP_SIZE((CFG_TUD_MAX_SPEED == OPT_MODE_HIGH_SPEED),CFG_TUD_AUDIO_MAX_SAMPLE_RATE, tusb_handle()->hConfig.uac2_audio.rec_szSple, tusb_handle()->hConfig.uac2_audio.rec_ch),
                                   /*_interval*/ 0x01);

    UAC2_DESC_CS_AS_ISO_EP_D(*pDescriptor,/*_attr*/ AUDIO_CS_AS_ISO_DATA_EP_ATT_NON_MAX_PACKETS_OK, /*_ctrl*/ AUDIO_CTRL_NONE, /*_lockdelayunit*/ AUDIO_CS_AS_ISO_DATA_EP_LOCK_DELAY_UNIT_UNDEFINED, /*_lockdelay*/ 0x0000);
  }
  /* Finalize the variable len, patch a  16 bits len at the offset 2*/
  lenAudioDescriptor = ((*pDescriptor) - tud_descriptor_get_configuration()) - TUD_CONFIG_DESC_LEN; /* the len is  without the device descriptor */
}




/**
 * @brief Initialize a jitter manager compensation
 *
 * @param pHandle  the instance
 * @param pRb      the ring buffer associated
 * @param compensation compensation delay in ms
 * @param jitterUp  limit up in bytes
 * @param jitterDn  limit down in bytes
 *
 */
static void s_jitter_buffer_mng_init(jitter_buffer_mng_t *pHandle, ring_buff_t *pRb, uint32_t compensation, int32_t freq)
{
  if (tusb_handle()->hConfig.mount & CFG_UAC2_AUDIO_ENABLED)
  {
    memset(pHandle, 0, sizeof(*pHandle));
    pHandle->pRb          = pRb;
    pHandle->compensation = compensation;
    pHandle->freq         = freq;
  }

}

/**
 * @brief return info Rect
 *
 * @return tud_audio_info*
 */
static tud_audio_info *s_audio_get_info_rec(void)
{
  return &tAudioInfo[UAC2_ENTITY_CLOCK_REC - 1];
}


/**
 * @brief return info  Play
 *
 * @return tud_audio_info*
 */
static tud_audio_info *s_audio_get_info_play(void)
{
  return &tAudioInfo[UAC2_ENTITY_CLOCK_PLAY - 1];
}

/**
 * @brief return info  Play or Rec
 *
 * @param id  Clock ID
 * @return tud_audio_info*
 */
static tud_audio_info *s_audio_get_info_from_id(uint32_t id)
{
  return &tAudioInfo[id];
}


/**
 * @brief idle the jitter manager
 * In House algo to  keep the buffer at the normal level
 * @param pHandle the instance
 * @return int32_t  action integer 0 neutral  , -1 too slow, 1 to fast
 */
static int32_t s_jitter_buffer_mng_idle(jitter_buffer_mng_t *pHandle, uint32_t szPktSize)
{
  int32_t szBuffer = (int32_t)szPktSize;
  pHandle->frameCount += szPktSize;
  if (pHandle->frameCount > pHandle->compensation)
  {
    /* reset compensation re-evaluation counter */
    pHandle->frameCount = 0UL;
    tusb_device_conf_t        *const pConf      = &tusb_handle()->hConfig;
    tusb_device_audio_conf_t *const pAudioConf = &pConf->uac2_audio;

    int32_t sampleSize = (int32_t)pAudioConf->rec_ch * (int32_t)pAudioConf->rec_szSple;
    /*
      If there is no jitter, the normal ring buffer level is tusb_handle()->hRb .szBuffer/2
      so, jitter as bytes   = ((read level) s_rb_read_available(h) - (normal level) szBuffer/2
      so, jitter as samples = jitter in bytes / sampleSize;
      so, the frequency variation between host and device is (normal frequency - jitter as sample)
    */

    int32_t lvlBuffer = (int32_t)s_rb_read_available(&tusb_handle()->hRbRec) - szBuffer; // minus szBuffer because we will consume 1 buffer right after this call
    /* compute the distance between the current state and the normal state */
    int32_t sampleOffset = lvlBuffer - ((int32_t)tusb_handle()->hRbRec.szBuffer / 2);
    if (sampleSize != 0)
    {
      /* convert in samples */
      sampleOffset /= sampleSize;
      /* compute the time between each sample correction, sampleOffset is the offset to commpensate the drift we add some samples to reconverge to its normal state and invert the slope*/
      sampleOffset += (sampleOffset / 4);
      if (sampleOffset < 0)
      {
        sampleOffset = -sampleOffset; // abs(sampleOffset) without MISRAC issue
        /* compute the action, remove a sample */
        pHandle->action = -sampleSize;
      }
      else
      {
        /* compute the action, add a sample */
        pHandle->action = sampleSize;
      }
      pHandle->delayCompensation = (sampleOffset == 0) ? 0 : ((int32_t)pAudioConf->rec_freq / sampleOffset); // avoid MISRAC issue: division by 0
    }
  }
  pHandle->countCompensation += szBuffer;
  /* if the delay is not elapsed, do nothing ie add/remove sample */
  int32_t action = 0;
  if (pHandle->countCompensation > pHandle->delayCompensation)
  {
    /* if we reach the delay, apply the action compensation  and reset the delay counter */
    action += pHandle->action;
    pHandle->countCompensation -= pHandle->delayCompensation;
  }
  return action ;
}

/**
 * @brief Idle the USB feedback and send the feedback message on the end point
 *
 */
static void s_compute_usb_feedback(void)
{
  #if CFG_TUD_AUDIO_ENABLE_FEEDBACK_EP != 0
  tusb_handle()->countPlayCompensation++;
  if ((tusb_handle()->countPlayCompensation % JITTER_PLAY_COMPENSATION) == 0U)
  {
    tusb_device_conf_t        *const pConf      = &tusb_handle()->hConfig;
    tusb_device_audio_conf_t *const pAudioConf = &pConf->uac2_audio;
    uint32_t sampleSize   = (uint32_t)pAudioConf->play_ch * (uint32_t)pAudioConf->play_szSple;
    /*
      If there is no jitter, the normal ring buffer level is tusb_handle()->hRbPlay.szBuffer/2

      so, jitter as bytes   = ((read level) s_rb_read_available(h) - (normal level) szBuffer/2
      so, jitter as samples = jitter in bytes / sampleSize;
      so, the frequency variation between host and device is (normal frequency - jitter as sample)

      We need to convert the frequency variation in feedback integer according to the Usb specification.

      The frequency variation float = frequency variation / frame Time
      The frame time depends on the USB speed, 1 ms (1/1000) for Full speed and 125 us high speed ( 1/8000)

      The feedback message to send is an unsigned fixed point 16.16 format.
      So,  feedback message = frequency variation float * 0x10000(65536)

    */


    int32_t sampleOffset = (int32_t)(s_rb_read_available(&tusb_handle()->hRbPlay)) - (int32_t)(tusb_handle()->hRbPlay.szBuffer / 2U) ;/*cstat !MISRAC2012-Rule-10.8 false positive both variable are unsigned */
    if (sampleSize)
    {
      uint32_t frameDiv  = (TUSB_SPEED_FULL == tud_speed_get()) ? 1000U : 8000U;
      sampleOffset /= (int32_t)sampleSize;
      int32_t curFrequency = (int32_t)pAudioConf->play_freq - sampleOffset;
      uint32_t feedback = (uint32_t)(((float)curFrequency / (float)frameDiv) * 65536.0f);/*cstat !MISRAC2012-Rule-10.8 false positive both variable are unsigned */
      tud_audio_n_fb_set(0, feedback);
    }
  }
  #endif
}


/*
    USB API support

*/

/**
 * @brief return the volume if feature_enabled is true
 *
 * @return uint32_t
 */
uint32_t  tud_audio_get_volume(void)
{
  return (uint32_t)tusb_handle()->volume;
}

/**
 * @brief return the mute  if feature_enabled is true
 *
 * @return uint32_t
 */
uint32_t  tud_audio_get_mute(void)
{
  return (uint32_t)tusb_handle()->mute;
}

/**
 * @brief rn the record frequency
 *
 * @return uint32_t
 */
uint32_t tud_audio_rec_frequency(void)
{
  return s_audio_get_info_play()->Freq;
}

/**
 * @brief return the record channel count
 *
 * @return uint32_t
 */
uint32_t tud_audio_rec_channel_count(void)
{
  return tusb_handle()->hConfig.uac2_audio.rec_ch;
}

/**
 * @brief return the Play frequency
 *
 * @return uint32_t
 */
uint32_t tud_audio_play_frequency(void)
{
  return s_audio_get_info_rec()->Freq ;
}

/**
 * @brief return the play  channel count
 *
 * @return uint32_t
 */
uint32_t tud_audio_play_channel_count(void)
{
  return tusb_handle()->hConfig.uac2_audio.play_ch;
}

/**
 * @brief return true if the playback is connected
 *
 * @return uint8_t
 */
uint8_t tud_audio_is_play_streaming(void)
{
  return (uint8_t)s_audio_get_info_play()->bStreaming;
}

/**
 * @brief return true if the record is connected
 *
 * @return uint8_t
 */
uint8_t tud_audio_is_rec_streaming(void)
{
  return (uint8_t)s_audio_get_info_rec()->bStreaming;
}

/**
 * @brief play a payload
 *
 * @param pDestData  payload pointer
 * @param szBytes   payload size
 * @return uint8_t
 */
uint8_t tud_audio_play(void *pDestData, uint32_t szBytes)
{
  if (tusb_handle()->hConfig.uac2_audio.play_enabled)
  {
    if (tud_audio_is_play_streaming())
    {
      tusb_handle()->lvlPlay =  s_rb_read_available(&tusb_handle()->hRbPlay);
      if (s_rb_read(&tusb_handle()->hRbPlay, pDestData, szBytes) != 0U)
      {
        TU_LOG1("Play Read:Underrun %d vs %d\n", s_rb_read_available(&tusb_handle()->hRbPlay), szBytes);
      }
    }
    else
    {
      memset(pDestData, 0, szBytes);
    }
  }
  return true;
}

/**
 * @brief  Record a payload
 *
 * @param pDestData  payload pointer
 * @param szBytes   payload size
 * @return uint8_t
 */
uint8_t  tud_audio_rec(void *pDestData, uint32_t szBytes)
{
  if ((tusb_handle()->hConfig.uac2_audio.rec_enabled != 0U) && (s_audio_get_info_rec()->bEpOpened == true))
  {
    if (s_audio_get_info_rec()->bStreaming == true)
    {
      tusb_handle()->lvlRec =  s_rb_read_available(&tusb_handle()->hRbRec);
      s_rb_write(&tusb_handle()->hRbRec, pDestData, szBytes);
    }
  }
  return true;
}

/**
 * @brief return the total description len minus the header
 *
 * @return uint32_t len
 */
uint32_t tud_get_dynamic_descriptor_len(void)
{
  return lenAudioDescriptor;
}

/**
 * @brief return the Speaker interface id
 *
 * @return uint32_t  id
 */
uint32_t tud_audio_get_interface_play()
{
  return (uint32_t)interfaceplay;
}

/**
 * @brief return the microphone interface id
 *
 * @return uint32_t id
 */
uint32_t tud_audio_get_interface_rec()
{
  return (uint32_t)interfacerec;
}

/**
 * @brief init the audio usb instance
 *
 */

void tusb_uac2_audio_init(void)
{
  if(tusb_handle()->hConfig.mount & CFG_UAC2_AUDIO_ENABLED)
  {
  tusb_device_conf_t *const pConf = &tusb_handle()->hConfig;
  tusb_device_audio_conf_t *const pAudioConf = &pConf->uac2_audio;
  tud_extra_class_add(s_uac2_audio_add_interface, s_uac2_audio_descriptor);

  /* compute some vars for play */
  s_audio_get_info_play()->Freq     = pAudioConf->play_freq;
  s_audio_get_info_play()->szPkt1ms = (pAudioConf->play_freq / 1000U) * pAudioConf->play_ch * pAudioConf->play_szSple;

  /* compute some vars for record */
  s_audio_get_info_rec()->Freq      = pAudioConf->rec_freq;
  s_audio_get_info_rec()->szPkt1ms  = (pAudioConf->rec_freq  / 1000U) * pAudioConf->rec_ch * pAudioConf->rec_szSple;

  #if CFG_DYNAMIC_MULTI_FREQUENCY == 0
  s_audio_get_info_rec()->nbFreqDef = 1;
  s_audio_get_info_rec()->tFreqDef[0] = s_audio_get_info_rec()->Freq;

  s_audio_get_info_play()->nbFreqDef = 1;
  s_audio_get_info_play()->tFreqDef[0] = s_audio_get_info_play()->Freq;
  #else
  const uint32_t tRec[] = CFG_DYNAMIC_MULTI_FREQUENCY_SET;
  memcpy(s_audio_get_info_rec()->tFreqDef, tRec, sizeof(tRec));
  s_audio_get_info_rec()->nbFreqDef = sizeof(tRec) / sizeof(tRec[0]);
  memcpy(s_audio_get_info_play()->tFreqDef, tRec, sizeof(tRec));
  s_audio_get_info_play()->nbFreqDef = sizeof(tRec) / sizeof(tRec[0]);
  #endif

  s_audio_init_ring_buffers();
  }
}




/*
      TinyUsb Integration
*/

/**
 * @brief  tusb idl, this stack in in charge to do the negotiation at the boot up and dispatch fifo messages
*/

/**
 * @brief Reinit rings buffer according to the frequency and channels
 *
 * @return true
 * @return false
 */
static bool s_audio_init_ring_buffers(void)
{
  bool result = true;
  s_rb_term(&tusb_handle()->hRbRec);
  s_rb_term(&tusb_handle()->hRbPlay);

  /* create 2 ring buffers */
  uint32_t szB = (ST_RB_REC_MS_NUM * s_audio_get_info_rec()->szPkt1ms) + 1UL;
  szB = (uint32_t)((float) szB * tusb_handle()->hConfig.uac2_audio.timeFrameMs);
  result = s_rb_init(&tusb_handle()->hRbRec, szB); /* +1 to get  real number of complete  packets */
  if (result)
  {
    szB = ST_RB_PLAY_MS_NUM * (s_audio_get_info_play()->szPkt1ms) + 1U;
    szB = (uint32_t)((float) szB * tusb_handle()->hConfig.uac2_audio.timeFrameMs);
    result = s_rb_init(&tusb_handle()->hRbPlay, szB); /* +1 to get  real number of complete  packets */
  }
  s_jitter_buffer_mng_init(&tusb_handle()->hJitterRec, &tusb_handle()->hRbRec, JITTER_REC_COMPENSATION * s_audio_get_info_rec()->szPkt1ms, (int32_t)s_audio_get_info_rec()->Freq);
  return result;
}

/**
 * @brief Helper for clock get requests, clock are audio frequency in HZ
 *
 * @return true
 * @return false
 */
static bool s_audio_clock_get_request(uint8_t rhport, audio_control_request_t const *request)
{
  tud_audio_info *pFreqDef = s_audio_get_info_from_id((uint32_t)request->bEntityID - 1U);

  if (request->bControlSelector == (uint8_t)AUDIO_CS_CTRL_SAM_FREQ)
  {
    if (request->bRequest == (uint8_t)AUDIO_CS_REQ_CUR)
    {
      TU_LOG1("Clock get current freq %lu\r\n", pFreqDef->Freq);

      audio_control_cur_4_t curf = { (int32_t) tu_htole32(pFreqDef->Freq) };
      return tud_audio_buffer_and_schedule_control_xfer(rhport, (tusb_control_request_t const *)request, &curf, (uint16_t)sizeof(curf));
    }
    else if (request->bRequest == (uint8_t)AUDIO_CS_REQ_RANGE)
    {
      uint8_t   tRangef[100];
      uint32_t  nbFrequ = 0U;
      nbFrequ  = pFreqDef->nbFreqDef;
      SET_TU_ATTR_PACKED_LEN(tRangef, nbFrequ);
      TU_LOG1("Clock get %d freq ranges\r\n", pFreqDef->nbFreqDef);
      for (uint8_t i = 0; i < pFreqDef->nbFreqDef; i++)
      {
        SET_TU_ATTR_PACKED_CLOCK(tRangef, i, pFreqDef->tFreqDef[i], pFreqDef->tFreqDef[i], 0);/*cstat !MISRAC2012-Rule-10.3  !MISRAC2012-Rule-10.4_a false positive, cstat cannot parse this macro */
        TU_LOG1("Range %d (%d, %d, %d)\r\n", i, pFreqDef->tFreqDef[i], pFreqDef->tFreqDef[i], 0);
      }
      return tud_audio_buffer_and_schedule_control_xfer(rhport, (tusb_control_request_t const *)request, (void *)tRangef, (uint16_t)(SIZE_TU_ATTR_PACKED(nbFrequ))); /*cstat !MISRAC2012-Rule-12.1 !MISRAC2012-Rule-10.7 false positive, cstat cannot parse this macro */
    }
  }
  else if (((request->bControlSelector == ((uint8_t)AUDIO_CS_CTRL_CLK_VALID)) && ((request->bRequest == (uint8_t)AUDIO_CS_REQ_CUR))))
  {
    audio_control_cur_1_t cur_valid = { .bCur = 1 };
    TU_LOG1("Clock get is valid %u\r\n", cur_valid.bCur);
    return tud_audio_buffer_and_schedule_control_xfer(rhport, (tusb_control_request_t const *)request, &cur_valid, (uint16_t)sizeof(cur_valid));
  }
  TU_LOG1("Clock get request not supported, entity = %u, selector = %u, request = %u\r\n", request->bEntityID, request->bControlSelector, request->bRequest);
  return false;
}

/**
 * @brief Helper for clock set requests
 *
 * @return true
 * @return false
 */
static bool s_audio_clock_set_request(uint8_t rhport, audio_control_request_t const *request, uint8_t const *buf)
{
  (void)rhport;

  tud_audio_info *pInfo = s_audio_get_info_from_id((uint32_t)request->bEntityID - 1U);

  TU_VERIFY(request->bRequest == (uint8_t)AUDIO_CS_REQ_CUR);

  if (request->bControlSelector == (uint8_t)AUDIO_CS_CTRL_SAM_FREQ)
  {
    TU_VERIFY(request->wLength == sizeof(audio_control_cur_4_t));
    TU_LOG1("Clock set current freq: %ld\r\n", pInfo->Freq);
    pInfo->Freq = (uint32_t)((audio_control_cur_4_t const *)buf)->bCur;
    if (CFG_DYNAMIC_MULTI_FREQUENCY == 1)
    {
      s_audio_init_ring_buffers();
    }
    return true;
  }
  else
  {
    TU_LOG1("Clock set request not supported, entity = %u, selector = %u, request = %u\r\n",
            request->bEntityID, request->bControlSelector, request->bRequest);
    return false;
  }
}

/**
 * @brief Helper for feature unit get requests ( volume & mute)
 *
 * @return true
 * @return false
 */
static bool s_audio_feature_unit_get_request(uint8_t rhport, audio_control_request_t const *request)
{
  TU_ASSERT(request->bEntityID == (uint8_t)UAC2_ENTITY_PLAY_FEATURE_UNIT);
  TU_ASSERT(request->bChannelNumber == 0U); // only master

  if ((request->bControlSelector == (uint8_t)AUDIO_FU_CTRL_MUTE) && (request->bRequest == (uint8_t)AUDIO_CS_REQ_CUR))
  {
    audio_control_cur_1_t mute1 = { .bCur = tusb_handle()->mute};
    TU_LOG1("Get channel %u mute %d\r\n", request->bChannelNumber, mute1.bCur);
    return tud_audio_buffer_and_schedule_control_xfer(rhport, (tusb_control_request_t const *)request, &mute1, (uint16_t)sizeof(mute1));
  }
  else if ((request->bControlSelector == (uint8_t)AUDIO_FU_CTRL_VOLUME))
  {
    if (request->bRequest == (uint8_t)AUDIO_CS_REQ_RANGE)
    {
      audio_control_range_2_n_t(1) range_vol =
      {
        .wNumSubRanges = tu_htole16(1),
        .subrange[0] = { .bMin = (int16_t)tu_htole16(-VOLUME_CTRL_50_DB), (int16_t)tu_htole16(VOLUME_CTRL_0_DB), (uint16_t)tu_htole16(256) }
      };
      TU_LOG1("Get channel %u volume range (%d, %d, %u) dB\r\n", request->bChannelNumber,
              range_vol.subrange[0].bMin / 256, range_vol.subrange[0].bMax / 256, range_vol.subrange[0].bRes / 256);
      return tud_audio_buffer_and_schedule_control_xfer(rhport, (tusb_control_request_t const *)request, &range_vol, (uint16_t)sizeof(range_vol));
    }
    else if (request->bRequest == (uint8_t)AUDIO_CS_REQ_CUR)
    {

      audio_control_cur_2_t cur_vol = { .bCur = tu_htole16(tusb_handle()->volume) };
      TU_LOG1("Get channel %u volume %d dB\r\n", request->bChannelNumber, cur_vol.bCur / 256);
      return tud_audio_buffer_and_schedule_control_xfer(rhport, (tusb_control_request_t const *)request, &cur_vol, (uint16_t)sizeof(cur_vol));
    }
  }
  TU_LOG1("Feature unit get request not supported, entity = %u, selector = %u, request = %u\r\n",
          request->bEntityID, request->bControlSelector, request->bRequest);

  return false;
}

/**
 * @brief Helper for feature unit set requests
 *
 * @return true
 * @return false
 */
static bool s_audio_feature_unit_set_request(uint8_t rhport, audio_control_request_t const *request, uint8_t const *buf)
{
  (void)rhport;

  TU_ASSERT(request->bEntityID == (uint8_t)UAC2_ENTITY_PLAY_FEATURE_UNIT);
  TU_VERIFY(request->bRequest == (uint8_t)AUDIO_CS_REQ_CUR);

  if (request->bControlSelector == (uint8_t)AUDIO_FU_CTRL_MUTE)
  {
    TU_VERIFY(request->wLength == sizeof(audio_control_cur_1_t));

    tusb_handle()->mute = ((audio_control_cur_1_t const *)buf)->bCur;

    TU_LOG1("Set channel %d Mute: %d\r\n", request->bChannelNumber, tusb_handle()->mute);

    return true;
  }
  else if (request->bControlSelector == (uint8_t)AUDIO_FU_CTRL_VOLUME)
  {
    TU_VERIFY(request->wLength == sizeof(audio_control_cur_2_t));

    tusb_handle()->volume = ((audio_control_cur_2_t const *)buf)->bCur;

    TU_LOG1("Set channel %d volume: %d dB\r\n", request->bChannelNumber, tusb_handle()->volume / 256);

    return true;
  }
  else
  {
    TU_LOG1("Feature unit set request not supported, entity = %u, selector = %u, request = %u\r\n",
            request->bEntityID, request->bControlSelector, request->bRequest);
    return false;
  }
}

/**
 * @brief Invoked when audio class specific get request received for an entity
 *
 * @return true
 * @return false
 */
bool tud_audio_get_req_entity_cb(uint8_t rhport, tusb_control_request_t const *p_request)
{
  audio_control_request_t const *request = (audio_control_request_t const *)p_request;

  if (request->bEntityID == (uint8_t)UAC2_ENTITY_CLOCK_PLAY)
  {
    return s_audio_clock_get_request(rhport, request);
  }

  if (request->bEntityID == (uint8_t)UAC2_ENTITY_CLOCK_REC)
  {
    return s_audio_clock_get_request(rhport, request);
  }


  if (request->bEntityID == (uint8_t)UAC2_ENTITY_PLAY_FEATURE_UNIT)
  {
    return s_audio_feature_unit_get_request(rhport, request);
  }
  else
  {
    TU_LOG1("Get request not handled, entity = %d, selector = %d, request = %d\r\n",
            request->bEntityID, request->bControlSelector, request->bRequest);
  }
  return false;
}

/**
 * @brief Invoked when audio class specific set request received for an entity
 *
 * @return true
 * @return false
 */
bool tud_audio_set_req_entity_cb(uint8_t rhport, tusb_control_request_t const *p_request, uint8_t *buf)
{
  audio_control_request_t const *request = (audio_control_request_t const *)p_request;

  if (request->bEntityID == (uint8_t)UAC2_ENTITY_PLAY_FEATURE_UNIT)
  {
    return s_audio_feature_unit_set_request(rhport, request, buf);
  }
  if (request->bEntityID == (uint8_t)UAC2_ENTITY_CLOCK_PLAY)
  {
    return s_audio_clock_set_request(rhport, request, buf);
  }

  if (request->bEntityID == (uint8_t)UAC2_ENTITY_CLOCK_REC)
  {
    return s_audio_clock_set_request(rhport, request, buf);
  }

  TU_LOG1("Set request not handled, entity = %d, selector = %d, request = %d\r\n", request->bEntityID, request->bControlSelector, request->bRequest);

  return false;
}

/**
 * @brief Invoked when audio endpoint is closed
 *
 * @return true
 * @return false
 */
bool tud_audio_set_itf_close_EP_cb(uint8_t rhport, tusb_control_request_t const *p_request)
{
  (void)rhport;

  uint8_t const itf = tu_u16_low(tu_le16toh(p_request->wIndex));
  uint8_t const alt = tu_u16_low(tu_le16toh(p_request->wValue));
  if (itf == tud_audio_get_interface_play())
  {
    s_audio_get_info_play()->bEpOpened  = false;
    s_audio_get_info_play()->bStreaming =  false;
    TU_LOG0("Close play");

  }
  if (itf == tud_audio_get_interface_rec())
  {
    s_audio_get_info_rec()->bEpOpened = false;
    s_audio_get_info_rec()->bStreaming =  false;
    TU_LOG0("Close rec");
  }
  TU_LOG2("Stopped %d:%dn", itf, alt);
  (void)alt; /* avoid warning when TU_LOG2 is doing nothing with its arguments */
  return true;
}


/**
 * @brief interface is opened
 *
 * @return true
 * @return false
 */
bool tud_audio_set_itf_cb(uint8_t rhport, tusb_control_request_t const *p_request)
{
  (void)rhport;
  uint8_t const itf = tu_u16_low(tu_le16toh(p_request->wIndex));
  uint8_t const alt = tu_u16_low(tu_le16toh(p_request->wValue));

  if ((itf == tud_audio_get_interface_play()) && (alt != 0U))
  {
    s_audio_get_info_play()->bEpOpened  = true;
    TU_LOG0("Open play");
  }
  if ((itf == tud_audio_get_interface_rec()) && (alt != 0U))
  {
    s_audio_get_info_rec()->bEpOpened = true;
    TU_LOG0("Open Rec");
  }
  TU_LOG2("Set interface %d alt %d\r\n", itf, alt);
  return true;
}


/**
 * @brief Invoked when audio class need to write samples
 * @return true
 * @return false
 */
bool tud_audio_tx_done_pre_load_cb(uint8_t rhport, uint8_t itf, uint8_t ep_in, uint8_t cur_alt_setting)
{
  (void)rhport;
  (void)itf;
  (void)ep_in;
  (void)cur_alt_setting;

  if (s_audio_get_info_rec()->bStreaming == false)
  {
    s_rb_reset(&tusb_handle()->hRbRec, tusb_handle()->hRbRec.szBuffer / 2U);
    s_audio_get_info_rec()->bStreaming =  true;
  }
  int32_t szPktByIt   = s_audio_get_info_rec()->szPkt1ms / (TUSB_SPEED_FULL == tud_speed_get() ? 1 : 8);
  int32_t length_usb_pck = (szPktByIt)  + (s_jitter_buffer_mng_idle(&tusb_handle()->hJitterRec, szPktByIt));
  static uint8_t tSamples[ST_FRAME_PACKET_MAX];
  uint32_t szFrame = s_rb_read(&tusb_handle()->hRbRec, tSamples, (uint32_t)length_usb_pck);
  tud_audio_write(tSamples, (uint16_t)szFrame);
  return true;
}




/**
 * @brief Invoked when audio class need to read samples
 * @return true
 * @return false
 */
bool tud_audio_rx_done_pre_read_cb(uint8_t rhport, uint16_t n_bytes_received, uint8_t func_id, uint8_t ep_out, uint8_t cur_alt_setting)
{
  (void)rhport;
  (void)func_id;
  (void)ep_out;
  (void)cur_alt_setting;
  /* if first samples, reset the buffer and signal the streaming started */
  if (s_audio_get_info_play()->bStreaming == false)
  {
    s_rb_reset(&tusb_handle()->hRbPlay, tusb_handle()->hRbPlay.szBuffer / 2U);
    s_audio_get_info_play()->bStreaming =  true;
  }
  /* read and inject the payload in the ring buffer */
  while (n_bytes_received)
  {
    uint16_t blk = (uint16_t)s_audio_get_info_play()->szPkt1ms;
    if (n_bytes_received < blk)
    {
      blk = n_bytes_received;
    }
    static uint8_t tSample[ST_FRAME_PACKET_MAX];
    tud_audio_read(tSample, blk);
    if (s_rb_write(&tusb_handle()->hRbPlay, tSample, blk) != 0U)
    {
      TU_LOG1("Play Write:Underrun %d vs %d\n", s_rb_write_available(&tusb_handle()->hRbPlay), blk);
    }
    n_bytes_received -= blk;
  }
  s_compute_usb_feedback();
  return true;
}

#endif
