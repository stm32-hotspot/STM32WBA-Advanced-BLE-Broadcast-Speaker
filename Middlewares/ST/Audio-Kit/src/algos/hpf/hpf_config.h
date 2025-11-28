/**
  ******************************************************************************
  * @file    hpf_config.h
  * @author  MCD Application Team
  * @brief   config of dc removing algo
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
#ifndef __HPF_CONFIG_H
#define __HPF_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "audio_chain.h"
#include "audio_chain_conf.h"

/* Exported constants --------------------------------------------------------*/


#if !defined(USE_HPF_TYPE_FIR) && !defined(USE_HPF_TYPE_IIR)
#error "at least USE_HPF_TYPE_FIR or USE_HPF_TYPE_IIR must be defined"
#endif

/* Exported types ------------------------------------------------------------*/
/**
* @brief  Library dynamic configuration handler. It contains dynamic parameters.
*/

/*cstat -MISRAC2012-Rule-8.12 duplicated constant convenient to define ranges*/
typedef enum
{
  #ifdef USE_HPF_TYPE_FIR
  HPF_TYPE_PARKSMCCLELLAN,
  HPF_TYPE_KAISERWINDOW,
  HPF_TYPE_FIRST_FIR = HPF_TYPE_PARKSMCCLELLAN,
  HPF_TYPE_LAST_FIR  = HPF_TYPE_KAISERWINDOW,
  #endif
  #ifdef USE_HPF_TYPE_IIR
  HPF_TYPE_BUTTERWORTH,
  HPF_TYPE_CHEBYSHEV1,
  HPF_TYPE_CHEBYSHEV2,
  HPF_TYPE_ELLIPTIC,
  HPF_TYPE_FIRST_IIR = HPF_TYPE_BUTTERWORTH,
  HPF_TYPE_LAST_IIR  = HPF_TYPE_ELLIPTIC,
  #endif
  HPF_TYPE_NB
} hpf_type_t;
/*cstat +MISRAC2012-Rule-8.12 */

typedef struct
{
  uint8_t filterType;
  uint8_t ramType;
}
hpf_static_config_t;

/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */


#ifdef __cplusplus
}
#endif

#endif /* __HPF_CONFIG_H */
