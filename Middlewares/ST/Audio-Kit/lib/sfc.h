/**
******************************************************************************
* @file    sfc.h
* @author  MCD Application Team
* @brief   samples format conversion
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

#ifndef __SFC_H
#define __SFC_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include "audio_buffer.h"
#include "audio_chain_utilities.h"

/* Exported types ------------------------------------------------------------*/
typedef struct
{
  bool                     initDone;
  bool                     mix;
  bool                     applyGain;
  float                    gain;
  audio_chain_utilities_t *pUtilsHandle;
  audio_buffer_type_t      inType;
  audio_buffer_type_t      outType;
  int                      silenceFillByte;
  int                      inChannelsOffsetParam;
  int                      inSamplesOffsetParam;
  int                      outChannelsOffsetParam;
  int                      outSamplesOffsetParam;
  void(* pSampleBufferConvert)(void *const pSampleIn,
                               int   const inChannelsOffset,
                               int   const inSamplesOffset,
                               void *const pSampleOut,
                               int   const outChannelsOffset,
                               int   const outSamplesOffset,
                               int   const nbChannels,
                               int   const nbSamples,
                               bool  const mix,
                               bool  const applyGain,
                               float const gain);
  void(* pSampleBufferClear)(void *const pSampleOut,
                             int   const outChannelsOffset,
                             int   const outSamplesOffset,
                             int   const nbChannels,
                             int   const nbSamples,
                             int   const silenceChar);
}
sfcContext_t;

/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void    sfcResetContext(sfcContext_t              *const pSfcContext);
int32_t sfcSetContext(sfcContext_t                *const pSfcContext, audio_buffer_t const *const pBuffIn, audio_buffer_t const *const pBuffOut, bool const mix, float const gain, audio_chain_utilities_t *const pUtilsHandle);
int32_t sfcUpdateContext(sfcContext_t             *const pSfcContext, bool const mix, float const gain);
void    sfcSampleBufferConvert(sfcContext_t const *const pSfcContext, void *const pSampleIn, void *const pSampleOut, int const nbChannels, int const nbSamples);
void    sfcSampleBufferClear(sfcContext_t   const *const pSfcContext,                        void *const pSampleOut, int const nbChannels, int const nbSamples);


#ifdef __cplusplus
}
#endif

#endif /* __SFC_H */
