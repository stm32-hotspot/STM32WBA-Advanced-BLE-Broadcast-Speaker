/**
  ******************************************************************************
  * @file    resample_config.h
  * @author  MCD Application Team
  * @brief   config of resampling (upsampling & downsampling) algo
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
#ifndef __RESAMPLE_CONFIG_H
#define __RESAMPLE_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "audio_chain.h"
#include "audio_chain_conf.h"
/* Exported constants --------------------------------------------------------*/


#if !defined(USE_RESAMPLE_TYPE_FIR) && !defined(USE_RESAMPLE_TYPE_IIR)
#error "at least USE_RESAMPLE_TYPE_FIR or USE_RESAMPLE_TYPE_IIR must be defined"
#endif

/* Exported types ------------------------------------------------------------*/
/**
* @brief  Library dynamic configuration handler. It contains dynamic parameters.
*/

/*cstat -MISRAC2012-Rule-8.12 duplicated constant convenient to define ranges*/
typedef enum
{
  #ifdef USE_RESAMPLE_TYPE_FIR
  RESAMPLE_TYPE_PARKSMCCLELLAN,
  RESAMPLE_TYPE_KAISERWINDOW,
  RESAMPLE_TYPE_FIRST_FIR = RESAMPLE_TYPE_PARKSMCCLELLAN,
  RESAMPLE_TYPE_LAST_FIR  = RESAMPLE_TYPE_KAISERWINDOW,
  #endif
  #ifdef USE_RESAMPLE_TYPE_IIR
  RESAMPLE_TYPE_BUTTERWORTH,
  RESAMPLE_TYPE_CHEBYSHEV1,
  RESAMPLE_TYPE_CHEBYSHEV2,
  RESAMPLE_TYPE_ELLIPTIC,
  RESAMPLE_TYPE_FIRST_IIR = RESAMPLE_TYPE_BUTTERWORTH,
  RESAMPLE_TYPE_LAST_IIR  = RESAMPLE_TYPE_ELLIPTIC,
  #endif
  RESAMPLE_TYPE_NO_FILTERING,
  RESAMPLE_TYPE_NB        = RESAMPLE_TYPE_NO_FILTERING
} resample_type_t;
/*cstat +MISRAC2012-Rule-8.12 */

typedef struct
{
  uint8_t filterType;
  uint8_t ramType;
}
resample_static_config_t;

/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */


#ifdef __cplusplus
}
#endif

#endif /* __RESAMPLE_CONFIG_H */
