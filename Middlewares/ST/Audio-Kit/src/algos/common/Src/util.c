/**
  ******************************************************************************
  * @file    util.c
  * @author  MCD Application Team
  * @brief   utilities routines
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

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "common/util.h"

/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Global variables ----------------------------------------------------------*/
/* Functions Definition ------------------------------------------------------*/

uint32_t util_pgcd(uint32_t const x, uint32_t const y)
{
  uint32_t a = (x < y) ? x : y;  /* min(x, y) */
  uint32_t b = (x < y) ? y : x;  /* max(x, y) */
  uint32_t pgcd;

  if (a == 0UL)
  {
    pgcd = b;
  }
  else
  {
    if ((b % a) != 0UL)
    {
      while (a != b)
      {
        if (a > b)
        {
          a -= b;
        }
        else
        {
          b -= a;
        }
      }
    }
    pgcd = a;
  }

  return pgcd;
}


uint32_t util_ppcm(uint32_t const x, uint32_t const y)
{
  uint32_t const pgcd = util_pgcd(x, y);

  return (pgcd == 0UL) ? 0UL : ((x / pgcd) * y);
}


uint8_t util_clamp_u8(uint8_t const x, uint8_t const val_min, uint8_t const val_max)
{
  uint8_t y = x;

  if (y < val_min)
  {
    y = val_min;
  }
  else if (y > val_max)
  {
    y = val_max;
  }
  else
  {
    /* MISRAC: do nothing */
  }

  return y;
}


int8_t util_clamp_s8(int8_t const x, int8_t const val_min, int8_t const val_max)
{
  int8_t y = x;

  if (y < val_min)
  {
    y = val_min;
  }
  else if (y > val_max)
  {
    y = val_max;
  }
  else
  {
    /* MISRAC: do nothing */
  }

  return y;
}


uint16_t util_clamp_u16(uint16_t const x, uint16_t const val_min, uint16_t const val_max)
{
  uint16_t y = x;

  if (y < val_min)
  {
    y = val_min;
  }
  else if (y > val_max)
  {
    y = val_max;
  }
  else
  {
    /* MISRAC: do nothing */
  }

  return y;
}


int16_t util_clamp_s16(int16_t const x, int16_t const val_min, int16_t const val_max)
{
  int16_t y = x;

  if (y < val_min)
  {
    y = val_min;
  }
  else if (y > val_max)
  {
    y = val_max;
  }
  else
  {
    /* MISRAC: do nothing */
  }

  return y;
}


uint32_t util_clamp_u32(uint32_t const x, uint32_t const val_min, uint32_t const val_max)
{
  uint32_t y = x;

  if (y < val_min)
  {
    y = val_min;
  }
  else if (y > val_max)
  {
    y = val_max;
  }
  else
  {
    /* MISRAC: do nothing */
  }

  return y;
}


int32_t util_clamp_s32(int32_t const x, int32_t const val_min, int32_t const val_max)
{
  int32_t y = x;

  if (y < val_min)
  {
    y = val_min;
  }
  else if (y > val_max)
  {
    y = val_max;
  }
  else
  {
    /* MISRAC: do nothing */
  }

  return y;
}


uint64_t util_clamp_u64(uint64_t const x, uint64_t const val_min, uint64_t const val_max)
{
  uint64_t y = x;

  if (y < val_min)
  {
    y = val_min;
  }
  else if (y > val_max)
  {
    y = val_max;
  }
  else
  {
    /* MISRAC: do nothing */
  }

  return y;
}


int64_t util_clamp_s64(int64_t const x, int64_t const val_min, int64_t const val_max)
{
  int64_t y = x;

  if (y < val_min)
  {
    y = val_min;
  }
  else if (y > val_max)
  {
    y = val_max;
  }
  else
  {
    /* MISRAC: do nothing */
  }

  return y;
}


float util_clamp_f32(float const x, float const val_min, float const val_max)
{
  float y = x;

  if (y < val_min)
  {
    y = val_min;
  }
  else if (y > val_max)
  {
    y = val_max;
  }
  else
  {
    /* MISRAC: do nothing */
  }

  return y;
}


uint8_t util_hysteresis_f32(util_hysteresis_t *const pHdle, float input)
{
  uint8_t ret = 0U;
  if (input >= pHdle->threshold_high)
  {
    ret = 1U;
    pHdle->prev_state = ret;
  }
  else if (input < pHdle->threshold_low) /* no = to support case threshold_low = threshold_high*/
  {
    ret = 0U;
    pHdle->prev_state = ret;
  }
  else
  {
    ret = pHdle->prev_state;
  }

  return ret;
}
