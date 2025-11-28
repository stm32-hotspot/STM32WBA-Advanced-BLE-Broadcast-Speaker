/**
******************************************************************************
* @file    stm32_usart_lock_cmsis_oc.c
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

#ifdef USE_THREADS

/* Includes ------------------------------------------------------------------*/
#include "stm32_usart.h"
/*cstat -MISRAC2012-Rule-10.4_a  A third-party produces a violation message we cannot fix*/
#include "cmsis_os.h"
/*cstat +MISRAC2012-Rule-10.4_a*/

/* Global variables ----------------------------------------------------------*/
/* Global define    ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/

static int8_t G_iLockRef = 0;
static struct
{
  osMutexAttr_t attr;
  osMutexId_t   id;
}
hLockUART = { .id = NULL };


/* Private function prototypes -----------------------------------------------*/
/* Functions Definition ------------------------------------------------------*/

/* lock to prevent preemption. */
void UTIL_UART_Lock(void)
{
  if (hLockUART.id == NULL)
  {
    /* mutex creation */
    hLockUART.attr.name      = NULL;
    hLockUART.attr.attr_bits = osMutexRecursive;
    hLockUART.id             = osMutexNew(&hLockUART.attr);
  }
  UTIL_UART_DISABLE_IRQ();    /* disable irq to insure atomic access to G_iLockRef */
  G_iLockRef++;
  if (G_iLockRef == 1)
  {
    /* with FreeRTOS, do not disable irq, lock mutex instead */
    UTIL_UART_ENABLE_IRQ();
    /* lock mutex */
    osMutexAcquire(hLockUART.id, osWaitForever);
  }
  else
  {
    /* with FreeRTOS, do not disable irq => re-enable it */
    UTIL_UART_ENABLE_IRQ();
  }
}


/* unlock to prevent preemption. */
void UTIL_UART_UnLock(void)
{
  /* Enable interrupts back only if they were enabled before we disable it here in this function */
  UTIL_UART_DISABLE_IRQ();    /* disable irq to insure atomic access to G_iLockRef */
  if (G_iLockRef > 0)
  {
    G_iLockRef--;
    if (G_iLockRef == 0)
    {
      /* with FreeRTOS, do not disable irq => re-enable irq and unlock mutex instead */
      UTIL_UART_ENABLE_IRQ();
      /* unlock mutex only if G_iLockRef is equal to 0 */
      if (hLockUART.id != NULL)
      {
        osMutexRelease(hLockUART.id);
      }
    }
    else
    {
      /* with FreeRTOS, do not disable irq => re-enable irq */
      UTIL_UART_ENABLE_IRQ();
    }
  }
  else
  {
    /* G_iLockRef was already equal to 0 => do not decrement it and re-enable irq */
    UTIL_UART_ENABLE_IRQ();
  }
}

#endif // USE_THREADS