/**
  ******************************************************************************
  * @file    fir.h
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

#ifndef __FIR_H
#define __FIR_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "audio_buffer.h"
#include "common/fir_config.h"

/* Exported types ------------------------------------------------------------*/

typedef struct firHandlerStruct
{
  union
  {
    firCoeffInt16_t const *pFirQ15;
    firCoeffInt32_t const *pFirQ31;
    firCoeffFloat_t const *pFirF32;
    void            const *pFirVoid;
  };
  void       *pInternalMem;
  memPool_t   memPool;
} firHandler_t;

/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
int32_t firInit(firHandler_t    *const pHdle, void const *const pFirVoid, audio_buffer_type_t const sampleType, uint8_t const nbChannels, uint32_t const nbSamples, uint8_t const downSamplingFactor, uint8_t const upSamplingFactor, memPool_t const memPool);
int32_t firDeInit(firHandler_t  *const pHdle);
int32_t firProcess(firHandler_t *const pHdle, void *const in, void *const out, int const ch, int const nbSamplesIn);


#ifdef __cplusplus
}
#endif

#endif // __FIR_H
