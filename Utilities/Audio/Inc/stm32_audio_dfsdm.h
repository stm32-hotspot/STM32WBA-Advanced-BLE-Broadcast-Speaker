/**
  ******************************************************************************
  * @file    stm32_audio_dfsdm.h
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
#ifndef __STM32_AUDIO_DFSDM_H
#define __STM32_AUDIO_DFSDM_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32_audio.h"

/** @addtogroup MIDDLEWARES
* @{
*/

/** @defgroup STM32_AUDIO_DFSDM STM32_AUDIO_DFSDM
* @{
*/
/* Exported types ------------------------------------------------------------*/

/** @defgroup STM32_AUDIO_Exported_Types STM32_AUDIO_DFSDM Exported Types
* @{
*/
/**
 * @brief  audio hw config.
 */
/**
  * @}
  */

/* Exported constants --------------------------------------------------------*/


/** @defgroup STM32_AUDIO_Exported_Constants STM32_AUDIO_DFSDM Exported Constants
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
/** @defgroup STM32_AUDIO_Exported_Functions STM32_AUDIO_DFSDM Exported Functions
* @{
*/

/** @defgroup STM32_AUDIO_common_services
* @brief    Common services for DFSDM audio capture
* @{
*/

/**
* @brief  Initialize recording from DFSDM.
* @param  AudioInit Init structure
* @retval BSP status
*/
int32_t UTIL_AUDIO_DFSDM_Init(UTIL_AUDIO_params_t *AudioInit);

/**
* @brief  Weak function for clock config (either calling existing function from BSP or write your own). It is called at init.
* @note   Being __weak it can be overwritten by the application
* @param  Sampling Frequency
* @retval Status
*/
int32_t UTIL_AUDIO_DFSDM_ClockConfig(uint32_t SampleRate);


/**
  * @brief  Starts DFSDM recording.
  * @param  pBuf      Main buffer pointer for the recorded data storing
  * @param  NbrOfBytes Size of the recorded buffer in bytes
  * @retval status
  */
int32_t UTIL_AUDIO_DFSDM_Record(uint8_t **pBuf, uint32_t NbrOfBytes);

/**
  * @brief  Stops audio recording with DFSDM.
  * @param  pBuf      Main buffer pointer for the recorded data storing
  * @param  NbrOfBytes Size of the recorded buffer in bytes
  * @retval status
  */
int32_t UTIL_AUDIO_DFSDM_Stop(void);

/**
  * @brief  processes/conditions data from DFSDM. Outputs 32-bit samples.
  * @param  pBuffInfo       pointer to buffer info that host all data delivered by UTIL_AUDIO_IN functions
  * @param  pChannelBuffer  Pointer to buffer addresses per channel
  * @param  nbSamples       Number fo samples
  * @param  offsetSpleBytes Offset in bytes where to llok for data (ping pong dma buffer)
  * @retval status
  */
void UTIL_AUDIO_DFSDM_32bitInt_process(audio_buffer_t *const pBuffInfo, int32_t **pChannelBuffer, uint32_t const nbSamples, uint32_t const offsetSpleBytes);


/**
  * @brief  processes/conditions data from DFSDM. Outputs 16-bit samples.
  * @param  pBuffInfo       pointer to buffer info that host all data delivered by UTIL_AUDIO_IN functions
  * @param  pChannelBuffer  Pointer to buffer addresses per channel
  * @param  nbSamples       Number fo samples
  * @param  offsetSpleBytes Offset in bytes where to llok for data (ping pong dma buffer)
  * @retval status
  */
void UTIL_AUDIO_DFSDM_16bitInt_process(audio_buffer_t *const pBuffInfo, int32_t **pChannelBuffer, uint32_t const nbSamples, uint32_t const offsetSpleBytes);

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

#endif /* __STM32_AUDIO_DFSDM_H */
