/**
******************************************************************************
* @file    audio_chain_sysIOs_conf.h
* @author  MCD Application Team
* @brief   Header for audio_chain_sysIOs.c module
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


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __AUDIO_CHAIN_SYSIOS_CONF_H
#define __AUDIO_CHAIN_SYSIOS_CONF_H

/* Includes ------------------------------------------------------------------*/
#include "audio_chain_instance_conf.h"
#include "audio_chain_sysIOs.h"

/* Exported constants --------------------------------------------------------*/
#define AC_SYSIN_PDM_NAME        "SysIn_pdmMic"     /*!< System In  connected to the microphone pdm format if any */
#define AC_SYSIN_BLE_LC3_NAME    "SysIn_BLE_LC3"    /*!< System In  connected to the LC3 decoder */
#define AC_SYSIN_CODEC_ADC_NAME  "SysIn_codec_DAC"  /*!< System In  connected to the CODEC ADC if any */
#define AC_SYSIN_WAVFILE_NAME    "SysIn_WavFile"    /*!< System In  connected to wav file from flash */

#define AC_SYSOUT_CODEC_DAC_NAME "SysOut_codec_ADC" /*!< System Out connected to the DAC (loud speaker) */
#define AC_SYSOUT_BLE_LC3_NAME   "SysOut_BLE_LC3"   /*!< System Out connected to the LC3 encoder */

/* Exported macro ------------------------------------------------------------*/
/* Exported structures--------------------------------------------------------*/
typedef enum
{
  AC_SYSIN_PDM,
  AC_SYSIN_BLE_LC3,
  AC_SYSIN_WAVFILE,
  AC_SYSIN_CODEC_ADC,
  AC_NB_MAX_SYS_IN
} ac_sys_in_id_t;

typedef enum
{
  AC_SYSOUT_CODEC_DAC,
  AC_SYSOUT_BLE_LC3,
  AC_NB_MAX_SYS_OUT
} ac_sys_out_id_t;

/* Exported variables ------------------------------------------------------- */
extern const char *tLiveTuneDefineConv[][2];

/* Exported functions ------------------------------------------------------- */


#endif /* __AUDIO_CHAIN_SYSIOS_CONF_H */

