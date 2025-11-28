/**
  ******************************************************************************
  * @file    util.h
  * @author  MCD Application Team
  * @brief   utilities routines
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
#ifndef __UTIL_H
#define __UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* Exported types ------------------------------------------------------------*/

typedef struct
{
  uint8_t prev_state;
  float   threshold_high;
  float   threshold_low;
}
util_hysteresis_t;
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
uint32_t util_pgcd(uint32_t const x, uint32_t const y);
uint32_t util_ppcm(uint32_t const x, uint32_t const y);

uint8_t  util_clamp_u8(uint8_t   const x, uint8_t  const val_min, uint8_t  const val_max);
int8_t   util_clamp_s8(int8_t    const x, int8_t   const val_min, int8_t   const val_max);
uint16_t util_clamp_u16(uint16_t const x, uint16_t const val_min, uint16_t const val_max);
int16_t  util_clamp_s16(int16_t  const x, int16_t  const val_min, int16_t  const val_max);
uint32_t util_clamp_u32(uint32_t const x, uint32_t const val_min, uint32_t const val_max);
int32_t  util_clamp_s32(int32_t  const x, int32_t  const val_min, int32_t  const val_max);
uint64_t util_clamp_u64(uint64_t const x, uint64_t const val_min, uint64_t const val_max);
int64_t  util_clamp_s64(int64_t  const x, int64_t  const val_min, int64_t  const val_max);
float    util_clamp_f32(float    const x, float    const val_min, float    const val_max);

uint8_t util_hysteresis_f32(util_hysteresis_t *const pHdle, float input);


#ifdef __cplusplus
}
#endif

#endif /* __UTIL_H */
