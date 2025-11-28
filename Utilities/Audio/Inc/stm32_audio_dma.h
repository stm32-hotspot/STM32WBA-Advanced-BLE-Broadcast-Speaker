/**
  ******************************************************************************
  * @file    stm32_audio_dma.h
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
#ifndef __STM32_AUDIO_DMA_H
#define __STM32_AUDIO_DMA_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32_audio.h"

/** @addtogroup MIDDLEWARES
* @{
*/

/** @defgroup STM32_AUDIO_DMA STM32_AUDIO_DMA
* @{
*/
/* Exported types ------------------------------------------------------------*/

/** @defgroup STM32_AUDIO_Exported_Types STM32_AUDIO_DMA Exported Types
* @{
*/



typedef enum
{
  UTIL_AUDIO_DMA_IP_TYPE_SAI,
  UTIL_AUDIO_DMA_IP_TYPE_DFSDM,
  UTIL_AUDIO_DMA_IP_TYPE_MDF,
  UTIL_AUDIO_DMA_IP_TYPE_NONE = 0xF
} UTIL_AUDIO_DMA_ip_type_t;

typedef struct
{
  void                    *pIpHdle;
  UTIL_AUDIO_DMA_ip_type_t type;
  uint32_t                 request;
  uint32_t                 preemptPriority;
  void                    *pInstance;
  IRQn_Type                irqn;
  uint32_t                 srcDataWidth;
  uint32_t                 destDataWidth;
  uint32_t                 direction;
} UTIL_AUDIO_DMA_t;

/**
  * @}
  */

/* Exported constants --------------------------------------------------------*/


/** @defgroup STM32_AUDIO_Exported_Constants STM32_AUDIO_DMA Exported Constants
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
/** @defgroup STM32_AUDIO_Exported_Functions STM32_AUDIO_DMA Exported Functions
* @{
*/

/** @defgroup STM32_AUDIO_common_services
* @brief    Common services for DMA audio capture
* @{
*/
/**
* @brief  Initialize DMA.
* @param  pHdle     Pointer to handler
* @retval None
*/
int32_t UTIL_AUDIO_DMA_Init(UTIL_AUDIO_DMA_t const *const pHdle);

/**
* @brief  DeInitialize DMA.
* @param  pHdle     Pointer to handler
* @retval None
*/
int32_t UTIL_AUDIO_DMA_DeInit(UTIL_AUDIO_DMA_t const *const pHdle);



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

#endif /* __STM32_AUDIO_DMA_H */
