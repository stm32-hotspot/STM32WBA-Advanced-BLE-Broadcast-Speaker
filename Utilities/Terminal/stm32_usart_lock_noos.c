/**
******************************************************************************
* @file    stm32_usart_lockl_noos.c
* @author  MCD Application Team
* @brief   This file provides set of firmware functions to manage UART
******************************************************************************
* @attention
*
* Copyright (c) 2022(-2022) STMicroelectronics.
* All rights reserved.
*
* This software is licensed under terms that can be found in the LICENSE file
* in the root directory of this software component.
* If no LICENSE file comes with this software, it is provided AS-IS.
*
******************************************************************************
*/

#ifndef USE_THREADS

/* Includes ------------------------------------------------------------------*/
#include "stm32_usart.h"

/* Global variables ----------------------------------------------------------*/
/* Global define    ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Functions Definition ------------------------------------------------------*/

/* lock to prevent preemption. */
void UTIL_UART_Lock(void)
{
  /* baremetal: Disable interrupts */
  UTIL_UART_DISABLE_IRQ();
}


/* unlock to prevent preemption. */
void UTIL_UART_UnLock(void)
{
  /* baremetal: Enable interrupts back only if they were enabled before we disable it here in this function */
  UTIL_UART_ENABLE_IRQ();
}

#endif // !USE_THREADS