/**
  ******************************************************************************
  * @file    audio_$(ALGO_NAME_MINUS_STD).h
  * @author  MCD Application Team
  * @brief   produce sound $(ALGO_NAME)
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2019(-2022) STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __AUDIO_$(ALGO_NAME_MAJ_STD)_H
#define __AUDIO_$(ALGO_NAME_MAJ_STD)_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "audio_chain.h"

/* Exported types ------------------------------------------------------------*/
/**
* @brief  Library dynamic configuration handler. It contains dynamic parameters.
*/

typedef struct
{
  uint8_t ramType;
} $(ALGO_NAME_MINUS_STD)_static_config_t;

typedef struct
{
$(ALGO_PARAM_STRUCT)
} $(ALGO_NAME_MINUS_STD)_dynamic_config_t;

/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */


#ifdef __cplusplus
}
#endif

#endif /* __AUDIO_$(ALGO_NAME_MAJ_STD)_H */
