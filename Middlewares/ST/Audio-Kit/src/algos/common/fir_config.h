/**
  ******************************************************************************
  * @file    fir_config.h
  * @author  MCD Application Team
  * @brief   fir firing algo
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

#ifndef __FIR_CONFIG_H
#define __FIR_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "audio_buffer.h"
#include "audio_chain_conf.h"
/* Exported types ------------------------------------------------------------*/

typedef struct
{
  uint16_t       nbTaps;
  float   const *pTaps;
} firCoeffFloat_t;

typedef struct
{
  uint16_t       nbTaps;
  //  int32_t        gainMant;
  //  int8_t         gainExp;
  int16_t const *pTaps;
} firCoeffInt16_t;

typedef struct
{
  uint16_t       nbTaps;
  //  int32_t        gainMant;
  //  int8_t         gainExp;
  int32_t const *pTaps;
} firCoeffInt32_t;


/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

#ifdef __cplusplus
}
#endif

#endif // __FIR_CONFIG_H
