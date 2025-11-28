/**
******************************************************************************
* @file          audio_persist_config.h
* @author        MCD Application Team
* @brief         Manage the persistent audio configuration
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


#ifndef _audio_persist_config_
#define _audio_persist_config_


#ifdef __cplusplus
extern "C"
{
#endif
/* Includes ------------------------------------------------------------------*/
#include "stdint.h"

/* Exported defines --------------------------------------------------------*/
/* Exported types --------------------------------------------------------*/
typedef struct audio_persist_config
{
  /* Common parameter for In & Out */
  const char *pConfigName;
  uint16_t audioMs;

  /* Parameter for AudioIn */
  uint32_t audioInFreq;
  uint8_t  audioInCh;
  uint8_t  audioInHwInterleave;
  uint8_t  audioInResolution;
  uint8_t  audioInHwResolution;
  uint32_t audioInHwCh;
  uint32_t audioInDeviceSelect;
  uint32_t audioInVolume;
  uint32_t audioInDeviceOutFormat;
  uint8_t  audioInUsePdm;
  uint8_t  audioInUsePdm2Pcm;
  uint32_t ac_sysInMicBuffType;


  /* Parameter for AudioOut */
  uint32_t audioOutVolume;
  uint8_t  audioOutResolution;
  uint32_t audioOutFreq;
  uint8_t  audioOutCh;

  /* Parameter for AudioChain */
  uint8_t  audioChainLowLatency;

  /* Parameter for Usb */
  uint8_t  usbResolution;
  uint32_t ac_sysUsbBuffType;

} audio_persist_config;


const audio_persist_config    *audio_persist_get_config_cur(void);
const audio_persist_config    *audio_persist_get_config(int32_t index);
int32_t                        audio_persist_get_config_number(void);
int32_t                        audio_persist_get_config_index(void);
void                           audio_persist_set_config_index(int32_t index);
void                           audio_persist_get_config_id(const audio_persist_config *pConfig, char *pBuffer, uint32_t szBuffer);
int32_t                        audio_persist_get_config_index_from_string(const char *pId);
void                           audio_persist_hook(void);
int32_t                        audio_persist_get_config_index_from_name(const char *pId);



#ifdef __cplusplus
};
#endif

#endif


