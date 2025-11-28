/**
  ******************************************************************************
  * @file    stm32_audio_pdm2pcm.h
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
#ifndef __STM32_AUDIO_PDM2PCM_H
#define __STM32_AUDIO_PDM2PCM_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32_audio.h"

/** @addtogroup MIDDLEWARES
* @{
*/

/** @defgroup STM32_AUDIO_PDM2PCM STM32_AUDIO_PDM2PCM
* @{
*/
/* Exported types ------------------------------------------------------------*/

/** @defgroup STM32_AUDIO_Exported_Types STM32_AUDIO_PDM2PCM Exported Types
* @{
*/
/**
 * @brief  audio hw config.
 */
/**
  * @}
  */

/* Exported constants --------------------------------------------------------*/


/** @defgroup STM32_AUDIO_Exported_Constants STM32_AUDIO_PDM2PCM Exported Constants
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
/** @defgroup STM32_AUDIO_Exported_Functions STM32_AUDIO_PDM2PCM Exported Functions
* @{
*/


/**
* @brief  Initialise PDM2PCM library.
* @retval status
*/
int32_t UTIL_AUDIO_PDM2PCM_Init(void);

/** @defgroup STM32_AUDIO_common_services
* @brief    Common services for PDM2PCM audio capture
* @{
*/

/**
* @brief  Runs PDM2PCM library.
* @param  pBuf      Main buffer pointer for the recorded data storing
* @param  NbrOfBytes Size of the recorded buffer in bytes
* @retval status
*/
int32_t UTIL_AUDIO_PDM2PCM_Decimate(uint8_t *PDMBuf, uint16_t *PCMBuf);





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

#endif /* __STM32_AUDIO_PDM2PCM_H */
