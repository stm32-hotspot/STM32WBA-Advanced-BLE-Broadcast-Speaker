/**
******************************************************************************
* @file    usb_descriptors.h
* @author  MCD Application Team
* @brief   tinyusb descriptor
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


#ifndef _USB_DESCRIPTORS_H_
#define _USB_DESCRIPTORS_H_

#ifdef __cplusplus
extern "C" {
#endif
/* Includes ------------------------------------------------------------------*/
#include "tusb_config.h"
/* Exported defines -----------------------------------------------------------*/


// Speaker and mic must be 1 and 2 to address tClockFreqRef[id-1];
#define UAC2_ENTITY_CLOCK_PLAY           0x01
#define UAC2_ENTITY_CLOCK_REC           0x02

// Play  path
#define UAC2_ENTITY_PLAY_INPUT_TERMINAL  0x03
#define UAC2_ENTITY_PLAY_OUTPUT_TERMINAL 0x04
#define UAC2_ENTITY_PLAY_FEATURE_UNIT    0x05

// Microphone path
#define UAC2_ENTITY_REC_INPUT_TERMINAL  0x11
#define UAC2_ENTITY_REC_OUTPUT_TERMINAL 0x13

#if  (CFG_SHOW_DESCRIPTOR == 1)
#define DUMP_DESC(p,name){\
  int32_t sz = sizeof(p);\
  uint8_t *pDesc = p;\
  while(sz)\
  {\
    tud_dump_array(#name,(void *)pDesc,pDesc[0],100);\
    sz -= pDesc[0];\
    pDesc += pDesc[0];\
  }\
}
#else
  #define DUMP_DESC(p,name)
#endif
  
  


/* the following macros are equivalent to its version without "_D" but create the descriptor dynamically in  memory */

#define TUD_CONFIG_DESCRIPTOR_D(pDesc,config_num, _itfcount, _stridx, _total_len, _attribute, _power_ma) {\
  uint8_t tArray[]={TUD_CONFIG_DESCRIPTOR(config_num, _itfcount, _stridx, _total_len, _attribute, _power_ma)};\
  DUMP_DESC(tArray,CONFIG_DESCRIPTOR_D);memcpy(pDesc,tArray,sizeof(tArray));pDesc+=sizeof(tArray);}

#define UAC2_DESC_IAD_D(pDesc,_firstitf, _nitfs, _stridx) {\
  uint8_t tArray[]={TUD_AUDIO_DESC_IAD(_firstitf, _nitfs, _stridx)};\
  DUMP_DESC(tArray,UAC2_DESC_IAD_D);memcpy(pDesc,tArray,sizeof(tArray));pDesc+=sizeof(tArray);}

#define UAC2_DESC_STD_AC_D(pDesc,_itfnum, _nEPs, _stridx){\
  uint8_t tArray[]={TUD_AUDIO_DESC_STD_AC(_itfnum, _nEPs, _stridx)};\
  DUMP_DESC(tArray,UAC2_DESC_STD_AC_D);memcpy(pDesc,tArray,sizeof(tArray));pDesc+=sizeof(tArray);}

#define UAC2_DESC_CS_AC_D(pDesc,_bcdADC, _category, _totallen, _ctrl) {\
  uint8_t tArray[]={TUD_AUDIO_DESC_CS_AC(_bcdADC, _category, _totallen, _ctrl)};\
  DUMP_DESC(tArray,UAC2_DESC_CS_AC_D);memcpy(pDesc,tArray,sizeof(tArray));pDesc+=sizeof(tArray);}

#define UAC2_DESC_CLK_SRC_D(pDesc,_clkid, _attr, _ctrl, _assocTerm, _stridx) {\
  uint8_t tArray[]={TUD_AUDIO_DESC_CLK_SRC(_clkid, _attr, _ctrl, _assocTerm, _stridx)};\
  DUMP_DESC(tArray,UAC2_DESC_CLK_SRC_D);memcpy(pDesc,tArray,sizeof(tArray));pDesc+=sizeof(tArray);}

#define UAC2_DESC_INPUT_TERM_D(pDesc,_termid, _termtype, _assocTerm, _clkid, _nchannelslogical, _channelcfg, _idxchannelnames, _ctrl, _stridx){\
  uint8_t tArray[]={TUD_AUDIO_DESC_INPUT_TERM(_termid, _termtype, _assocTerm, _clkid, _nchannelslogical, _channelcfg, _idxchannelnames, _ctrl, _stridx)};\
  DUMP_DESC(tArray,UAC2_DESC_INPUT_TERM_D);memcpy(pDesc,tArray,sizeof(tArray));pDesc+=sizeof(tArray);}

#define UAC2_DESC_FEATURE_UNIT_TWO_CHANNEL_D(pDesc,_unitid, _srcid, _ctrlch0master, _ctrlch1, _ctrlch2, _stridx){ \
   uint8_t tArray[]={TUD_AUDIO_DESC_FEATURE_UNIT_TWO_CHANNEL(_unitid, _srcid, _ctrlch0master, _ctrlch1, _ctrlch2, _stridx)};\
   DUMP_DESC(tArray,UAC2_DESC_FEATURE_UNIT_TWO_CHANNEL_D);memcpy(pDesc,tArray,sizeof(tArray));pDesc+=sizeof(tArray);}

#define UAC2_DESC_FEATURE_UNIT_ONE_CHANNEL_D(pDesc,_unitid, _srcid, _ctrlch0master, _ctrlch1, _stridx){ \
   uint8_t tArray[]={TUD_AUDIO_DESC_FEATURE_UNIT_ONE_CHANNEL(_unitid, _srcid, _ctrlch0master, _ctrlch1, _stridx)};\
   DUMP_DESC(tArray,UAC2_DESC_FEATURE_UNIT_ONE_CHANNEL_D);memcpy(pDesc,tArray,sizeof(tArray));pDesc+=sizeof(tArray);}


#define UAC2_DESC_OUTPUT_TERM_D(pDesc,_termid, _termtype, _assocTerm, _srcid, _clkid, _ctrl, _stridx){\
   uint8_t tArray[]={TUD_AUDIO_DESC_OUTPUT_TERM(_termid, _termtype, _assocTerm, _srcid, _clkid, _ctrl,_stridx)};\
   DUMP_DESC(tArray,UAC2_DESC_OUTPUT_TERM_D);memcpy(pDesc,tArray,sizeof(tArray));pDesc+=sizeof(tArray);}


#define UAC2_DESC_STD_AS_INT_D(pDesc,_itfnum, _altset, _nEPs, _stridx) {\
   uint8_t tArray[]={TUD_AUDIO_DESC_STD_AS_INT(_itfnum, _altset, _nEPs, _stridx)};\
   DUMP_DESC(tArray,UAC2_DESC_STD_AS_INT_D);memcpy(pDesc,tArray,sizeof(tArray));pDesc+=sizeof(tArray);}

#define UAC2_DESC_CS_AS_INT_D(pDesc,_termid, _ctrl, _formattype, _formats, _nchannelsphysical, _channelcfg, _stridx){ \
   uint8_t tArray[]={TUD_AUDIO_DESC_CS_AS_INT(_termid,_ctrl, _formattype, _formats, _nchannelsphysical, _channelcfg, _stridx)};\
   DUMP_DESC(tArray,UAC2_DESC_CS_AS_INT_D);memcpy(pDesc,tArray,sizeof(tArray));pDesc+=sizeof(tArray);}

#define UAC2_DESC_TYPE_I_FORMAT_D(pDesc,_subslotsize, _bitresolution) /* _subslotsize is number of bytes per sample (i.e. subslot) and can be 1,2,3, or 4 */{\
   uint8_t tArray[]={TUD_AUDIO_DESC_TYPE_I_FORMAT(_subslotsize, _bitresolution)};\
   DUMP_DESC(tArray,UAC2_DESC_TYPE_I_FORMAT_D);memcpy(pDesc,tArray,sizeof(tArray));pDesc+=sizeof(tArray);}

#define UAC2_DESC_STD_AS_ISO_EP_D(pDesc,_ep, _attr, _maxEPsize, _interval) {\
   uint8_t tArray[]={TUD_AUDIO_DESC_STD_AS_ISO_EP(_ep, _attr, _maxEPsize, _interval)};\
   DUMP_DESC(tArray,UAC2_DESC_STD_AS_ISO_EP_D);memcpy(pDesc,tArray,sizeof(tArray));pDesc+=sizeof(tArray);}

#define UAC2_DESC_CS_AS_ISO_EP_D(pDesc,_attr, _ctrl, _lockdelayunit, _lockdelay) {\
   uint8_t tArray[]={TUD_AUDIO_DESC_CS_AS_ISO_EP(_attr, _ctrl, _lockdelayunit, _lockdelay)};\
   DUMP_DESC(tArray,UAC2_DESC_CS_AS_ISO_EP_D);memcpy(pDesc,tArray,sizeof(tArray));pDesc+=sizeof(tArray);}

#if TUSB_VERSION_MINOR <= 16
#define UAC2_DESC_STD_AS_ISO_FB_EP_D(pDesc,_ep, _epfbsize, _interval){\
   uint8_t tArray[]={TUD_AUDIO_DESC_STD_AS_ISO_FB_EP(_ep, _interval)};\
   DUMP_DESC(tArray,UAC2_DESC_STD_AS_ISO_FB_EP_D);memcpy(pDesc,tArray,sizeof(tArray));pDesc+=sizeof(tArray);}
#else
#define UAC2_DESC_STD_AS_ISO_FB_EP_D(pDesc,_ep, _epfbsize, _interval){\
   DUMP_DESC(tArray,UAC2_DESC_STD_AS_ISO_FB_EP_D);uint8_t tArray[]={TUD_AUDIO_DESC_STD_AS_ISO_FB_EP(_ep,_epfbsize,_interval)};\
   memcpy(pDesc,tArray,sizeof(tArray));pDesc+=sizeof(tArray);}

#endif
#define TUD_HID_DESCRIPTOR_D(pDesc,_itfnum, _stridx, _boot_protocol, _report_desc_len, _epin, _epsize, _ep_interval){\
   uint8_t tArray[]={TUD_HID_DESCRIPTOR(_itfnum, _stridx, _boot_protocol, _report_desc_len, _epin, _epsize, _ep_interval)};\
   DUMP_DESC(tArray,HID_DESCRIPTOR_D);memcpy(pDesc,tArray,sizeof(tArray));pDesc+=sizeof(tArray);}

#define TUD_HID_INOUT_DESCRIPTOR_D(pDesc,_itfnum, _stridx, _boot_protocol, _report_desc_len, _epin,_epout, _epsize, _ep_interval){\
   uint8_t tArray[]={TUD_HID_INOUT_DESCRIPTOR(_itfnum, _stridx, _boot_protocol, _report_desc_len, _epin,_epout, _epsize, _ep_interval)};\
   DUMP_DESC(tArray,HID_INOUT_DESCRIPTOR_D);memcpy(pDesc,tArray,sizeof(tArray));pDesc+=sizeof(tArray);}



#define TUD_CDC_DESCRIPTOR_D(pDesc,_itfnum, _stridx, _ep_notif, _ep_notif_size, _epout, _epin, _epsize){\
   uint8_t tArray[]={TUD_CDC_DESCRIPTOR(_itfnum, _stridx, _ep_notif, _ep_notif_size, _epout, _epin, _epsize)};\
   DUMP_DESC(tArray,CDC_DESCRIPTOR_D);memcpy(pDesc,tArray,sizeof(tArray));pDesc+=sizeof(tArray);}




#define TUD_VIDEO_DESC_IAD_D(pDesc,_firstitf, _nitfs, _stridx){\
   uint8_t tArray[]={TUD_VIDEO_DESC_IAD(_firstitf, _nitfs, _stridx)};\
   DUMP_DESC(tArray,VIDEO_DESC_IAD_D);memcpy(pDesc,tArray,sizeof(tArray));pDesc+=sizeof(tArray);}



#define TUD_MIDI_DESCRIPTOR_IN_D(pDesc,_itfnum, _stridx, _epout, _epin, _epsize){\
   uint8_t tArray[]={TUD_MIDI_DESCRIPTOR(_itfnum, _stridx,_epout,_epin, _epsize)};\
   DUMP_DESC(tArray,MIDI_DESCRIPTOR_IN_D);memcpy(pDesc,tArray,sizeof(tArray));pDesc+=sizeof(tArray);}

#define TUD_VIDEO_DESC_CS_VC_D(pDesc,_bcdUVC, _totallen, _clkfreq, ...) {\
   uint8_t tArray[]={TUD_VIDEO_DESC_CS_VC(_bcdUVC, _totallen, _clkfreq,__VA_ARGS__)};\
   DUMP_DESC(tArray,VIDEO_DESC_CS_VC_D);memcpy(pDesc,tArray,sizeof(tArray));pDesc+=sizeof(tArray);}


#define TUD_DESCRIPTOR_RAW_D(pDesc,pRaw,szRaw){\
   ((uint8_t*)pRaw)[0]=(uint8_t)szRaw;\
   DUMP_DESC(tArray,DESCRIPTOR_RAW_D);memcpy(pDesc,pRaw,szRaw);(pDesc)+=szRaw;}


static void TUD_DUMP_DESC(void *pDesc)
{
  uint8_t *pDesc8 = (uint8_t *)pDesc;
  static int32_t cpt = 0;
  printf("%02X: ", cpt++);
  for (int32_t a = 0; a < pDesc8[0]; a++) { printf("%02X ", pDesc8[a]); }
  printf("\n");
}



#define DESC_LEN                                 uint8_t *
#define DESC_START_LEN(h,pCur)              {h=pCur;}
#define DESC_STOP_LEN_16(h,pCur,offset,add) {uint8_t tArray[]={U16_TO_U8S_LE((pCur-h)+add)}; memcpy(&((uint8_t*)h)[offset],tArray,sizeof(tArray));}
#define DESC_GET_LEN_16(h,pCur,add)         U16_TO_U8S_LE((((uint8_t *)pCur)-((uint8_t *)h))+add)



/* Exported typedef -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
uint8_t *tud_build_dynamic_descriptor(void);
uint8_t *tud_descriptor_get_configuration(void);
uint8_t tud_extra_class_add(void (*interface)(int32_t *numInterface), void (*descriptor)(uint8_t **pDescriptor));
tusb_desc_device_t *tud_descriptor_get_descriptor_device(void);
int8_t  tusb_allocate_ep(void);
#ifdef __cplusplus
}
#endif

#endif


