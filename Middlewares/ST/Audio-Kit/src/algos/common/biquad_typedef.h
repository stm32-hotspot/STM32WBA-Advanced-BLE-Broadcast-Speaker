/**
  ******************************************************************************
  * @file    biquad_typedef.h
  * @author  MCD Application Team
  * @brief   biquad filtering typedefs
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

#ifndef __BIQUAD_TYPEDEF_H
#define __BIQUAD_TYPEDEF_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* Exported constants --------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/

typedef struct
{
  int32_t b0, b1, b2, a1, a2;
} biquadCellInt32_t;

typedef struct
{
  int32_t                  gainMant;
  uint8_t                  gainExp;
  uint8_t                  nbCells;
  biquadCellInt32_t const *pBiquadCell;
} biquadInt32_t;

typedef struct
{
  float b0, b1, b2, a1, a2;
} biquadCellFloat_t;

typedef struct
{
  uint8_t                  nbCells;
  biquadCellFloat_t const *pBiquadCell;
} biquadFloat_t;


#ifdef __cplusplus
}
#endif

#endif // __BIQUAD_TYPEDEF_H
