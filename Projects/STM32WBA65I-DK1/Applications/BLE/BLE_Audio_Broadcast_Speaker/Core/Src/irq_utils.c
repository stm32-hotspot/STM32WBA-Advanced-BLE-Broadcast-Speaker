/**
******************************************************************************
* @file    irq_utils.c
* @author  MCD Application Team
* @brief   irq utils
******************************************************************************
* @attention
*
* Copyright (c) 2018(-2022) STMicroelectronics.
* All rights reserved.
*
* This software is licensed under terms that can be found in the LICENSE file
* in the root directory of this software component.
* If no LICENSE file comes with this software, it is provided AS-IS.
*
******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <cmsis_compiler.h>     /* for __disable_irq(); and __enable_irq(); */
#include "irq_utils.h"

/* Private define ------------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static uint8_t  irq_disable_nested_cnt = 0U;
static uint8_t  irq_disable_nested_max = 0U; // for debug only: to know how many max __disable_irq() are nested

/* Private functions prototypes-----------------------------------------------*/
/* Functions Definition ------------------------------------------------------*/

void disable_irq_with_cnt(void)
{
  __disable_irq();
  irq_disable_nested_cnt++;
  if (irq_disable_nested_cnt > irq_disable_nested_max)
  {
    irq_disable_nested_max = irq_disable_nested_cnt;
  }
}


void enable_irq_with_cnt(void)
{
  __disable_irq();
  if (irq_disable_nested_cnt > 0U)
  {
    irq_disable_nested_cnt--;
  }
  if (irq_disable_nested_cnt == 0U)
  {
    __enable_irq();
  }
}
