/**
  ******************************************************************************
  * @file    speex_aec_config.h
  * @author  MCD Application Team
  * @brief   config of Speex acoustic echo canceler algo
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
#ifndef __SPEEX_AEC_CONFIG_H
#define __SPEEX_AEC_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "audio_chain.h"

/* Exported constants --------------------------------------------------------*/
#define CONF_DEBUG_ACTIVE
//#define CONF_DEBUG_DELAY

/* Exported types ------------------------------------------------------------*/
typedef enum
{
  AC_SPEEX_AEC_DEBUG_OUTPUT_NONE = 0,
  AC_SPEEX_AEC_DEBUG_OUTPUT_DELAYREF,
  AC_SPEEX_AEC_DEBUG_OUTPUT_PROCESSED,
} aec_speex_debug_t;

typedef struct
{
  float    delay_length;      /*!< Specifies the delay applied on echo path, unit is second */
  float    echo_length;       /*!< Specifies the echo length, unit is second. */
  uint8_t  preprocess_init;   /*!< Specifies if the preprocessor will be initialized, can be enabled/disable dynamically.*/
  #ifdef CONF_DEBUG_DELAY
  uint8_t  debug_delay;
  #endif
  uint8_t  ramType;
}
aec_speex_static_config_t;

typedef struct
{
  uint8_t           preprocess_state;       /*!< Enable or disable pre-process function */
  uint8_t           residual_echo_remove;   /*!< Activate residual echo removal */
  float             AGC_value;              /*!< Specifies the threshold for the AGC, if activated */
  int8_t            noise_suppress;         /*!< Specifies the noise suppress parameter of the preprocessor */
  int8_t            echo_suppress;          /*!< Specifies the echo suppress parameter of the preprocessor */
  int8_t            echo_suppress_active;   /*!< Specifies the echo suppress active parameter of the preprocessor */
  #ifdef CONF_DEBUG_ACTIVE
  uint8_t           debug_output;
  #endif
}
aec_speex_dynamic_config_t;

/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */


#ifdef __cplusplus
}
#endif

#endif /* __SPEEX_AEC_CONFIG_H */
