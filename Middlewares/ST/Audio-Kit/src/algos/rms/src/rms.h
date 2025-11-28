/**
  ******************************************************************************
  * @file    rms.h
  * @author  MCD Application Team
  * @brief   rms algo (rms estimation from input time samples)
  *******************************************************************************
  * @attention
  *
  * Copyright (c) 2019(-2022) STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ********************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RMS_H
#define __RMS_H

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/

typedef struct
{
  audio_buffer_type_t  audioType;
  bool                 isDoublePrecision;
  uint8_t              nbChannels;
  uint32_t             fs;
  float               *pRmsValues;
  void                *pInternalMem;
  size_t               internalMemSize;
} rmsHdler_t;

/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
int32_t rmsGetMemorySize(rmsHdler_t *const pHdle);
int32_t rmsInit(rmsHdler_t *const pHdle);
int32_t rmsConfigure(rmsHdler_t *const pHdle, uint16_t const smoothingTimeInMs);
int32_t rmsProcess(rmsHdler_t   *const pHdle, audio_chunk_t const *const pChunkIn);

#ifdef __cplusplus
}
#endif

#endif /* __RMS_H */
