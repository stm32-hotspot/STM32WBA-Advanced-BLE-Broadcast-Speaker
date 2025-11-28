/**
  ******************************************************************************
  * @file    stm32_audio_mdf.h
  * @author  MCD Application Team
  * @brief   Header for stm32_audio.c module
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32_AUDIO_MDF_H
#define __STM32_AUDIO_MDF_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32_audio.h"

/** @addtogroup MIDDLEWARES
* @{
*/

/** @defgroup STM32_AUDIO_MDF STM32_AUDIO_MDF
* @{
*/
/* Exported types ------------------------------------------------------------*/

typedef struct
{
  uint32_t Hpf;
  int32_t  Gain;
  uint32_t DecimationRatio;
  uint32_t CicMode;
  uint32_t ProcClockDivider;
  uint32_t OutputClockDivider;
} UTIL_AUDIO_MDF_Config_t;

typedef struct
{
  void                    *pHdle;
  void                    *pFilterHdle;
  UTIL_AUDIO_MDF_Config_t  hMdfConf;
} UTIL_AUDIO_MDF_User_t;

/** @defgroup STM32_AUDIO_Exported_Types STM32_AUDIO_MDF Exported Types
* @{
*/
/**
 * @brief  audio hw config.
 */
/**
  * @}
  */

/* Exported constants --------------------------------------------------------*/


/** @defgroup STM32_AUDIO_Exported_Constants STM32_AUDIO_MDF Exported Constants
* @{
*/

/** @defgroup STM32_AUDIO_default_conf
* @brief    Default configuration , can be overwritten inside stm32_audio_conf.h
* @{
*/

/**
* @}
*/


/**
* @}
*/

/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
/** @defgroup STM32_AUDIO_Exported_Functions STM32_AUDIO_MDF Exported Functions
* @{
*/

/** @defgroup STM32_AUDIO_common_services
* @brief    Common services for MDF audio capture
* @{
*/

/**
* @brief  Initialize recording from MDF (PDM decimation).
* @param  AudioInit Init structure
* @retval BSP status
*/
int32_t UTIL_AUDIO_MDF_Init(UTIL_AUDIO_params_t *AudioInit);

/**
* @brief  DeInitialize MDF for PDM decimation.
* @param  None
* @retval Status
*/
int32_t UTIL_AUDIO_MDF_DeInit(void);

/**
* @brief  This function needs is called by UTIL_AUDIO_MDF_Init and must be implemented by user.
* @param  Pointer to user structure
* @retval Status
*/
int32_t UTIL_AUDIO_MDF_MX_Init(UTIL_AUDIO_MDF_User_t *pUser);

/**
* @brief  Weak function for clock config (either calling existing function from BSP or write your own). It is called at init.
* @note   Being __weak it can be overwritten by the application
* @param  Sampling Frequency
* @retval Status
*/
int32_t UTIL_AUDIO_MDF_ClockConfig(uint32_t SampleRate);


/**
  * @brief  Starts MDF recording.
  * @param  pBuf      Main buffer pointer for the recorded data storing
  * @param  NbrOfBytes Size of the recorded buffer in bytes
  * @retval status
  */
int32_t UTIL_AUDIO_MDF_Start(uint8_t *pBuf, uint32_t NbrOfBytes);

/**
  * @brief  Stops audio recording with MDF.
  * @param  pBuf      Main buffer pointer for the recorded data storing
  * @param  NbrOfBytes Size of the recorded buffer in bytes
  * @retval status
  */
int32_t UTIL_AUDIO_MDF_Stop(void);




/**
* @}
*/

/**
* @}
*/

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __STM32_AUDIO_MDF_H */
