/**
  ******************************************************************************
  * @file    stm32_audio_sai_pdm.h
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
#ifndef __STM32_AUDIO_SAI_PDM_H
#define __STM32_AUDIO_SAI_PDM_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32_audio.h"

/** @addtogroup MIDDLEWARES
* @{
*/

/** @defgroup STM32_AUDIO_SAI_PDM STM32_AUDIO_SAI_PDM
* @{
*/
/* Exported types ------------------------------------------------------------*/
typedef struct
{
  uint32_t AudioFrequency;
  uint32_t AudioMode;
  uint32_t DataSize;
  uint32_t MonoStereoMode;
  uint32_t ClockStrobing;
  uint32_t Synchro;
  uint32_t OutputDrive;
  uint32_t SynchroExt;
  uint32_t FrameLength;
  uint32_t ActiveFrameLength;
  uint32_t SlotActive;
  uint32_t MckDiv;
} UTIL_AUDIO_SAI_PDM_Config_t;

typedef struct
{
  void                        *pHdle;
  UTIL_AUDIO_params_t         *pAudioConf;
  UTIL_AUDIO_SAI_PDM_Config_t  hSaiPdmConf;
} UTIL_AUDIO_SAI_PDM_User_t;

/** @defgroup STM32_AUDIO_Exported_Types STM32_AUDIO_SAI_PDM Exported Types
* @{
*/
/**
 * @brief  audio hw config.
 */
/**
  * @}
  */

/* Exported constants --------------------------------------------------------*/


/** @defgroup STM32_AUDIO_Exported_Constants STM32_AUDIO_SAI_PDM Exported Constants
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
/** @defgroup STM32_AUDIO_Exported_Functions STM32_AUDIO_SAI_PDM Exported Functions
* @{
*/

/** @defgroup STM32_AUDIO_common_services
* @brief    Common services for SAI_PDM audio capture
* @{
*/

/**
* @brief  Initialize recording from SAI_PDM.
* @param  AudioInit Init structure
* @retval BSP status
*/
int32_t UTIL_AUDIO_SAI_PDM_Init(UTIL_AUDIO_params_t *AudioInit);

/**
* @brief  DeInitialize SAI PDM.
* @param  None
* @retval Status
*/
int32_t UTIL_AUDIO_SAI_PDM_DeInit(void);

/**
* @brief  This function needs is called by UTIL_AUDIO_SAI_PDM_Init and must be implemented by user.
* @param  Pointer to user structure
* @retval Status
*/
int32_t UTIL_AUDIO_SAI_PDM_MX_Init(UTIL_AUDIO_SAI_PDM_User_t *pUser);

/**
* @brief  Weak function for clock config (either calling existing function from BSP or write your own). It is called at init.
* @note   Being __weak it can be overwritten by the application
* @param  Sampling Frequency
* @retval Status
*/
int32_t UTIL_AUDIO_SAI_PDM_ClockConfig(uint32_t SampleRate);


/**
  * @brief  Starts SAI_PDM recording.
  * @param  pBuf      Main buffer pointer for the recorded data storing
  * @param  NbrOfBytes Size of the recorded buffer in bytes
  * @retval status
  */
int32_t UTIL_AUDIO_SAI_PDM_Start(uint8_t *pBuf, uint32_t NbrOfBytes);

/**
  * @brief  Stops audio recording with SAI_PDM.
  * @param  pBuf      Main buffer pointer for the recorded data storing
  * @param  NbrOfBytes Size of the recorded buffer in bytes
  * @retval status
  */
int32_t UTIL_AUDIO_SAI_PDM_Stop(void);

/**
  * @brief  Half reception complete callback.
  * @param  hsai   SAI handle.
  * @retval None
  */
void UTIL_AUDIO_SAI_PDM_RxHalfCpltCallback(void *pHdleSai);

/**
  * @brief  Reception complete callback.
  * @param  hsai   SAI handle.
  * @retval None
  */
void UTIL_AUDIO_SAI_PDM_RxCpltCallback(void *pHdleSai);



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

#endif /* __STM32_AUDIO_SAI_PDM_H */
