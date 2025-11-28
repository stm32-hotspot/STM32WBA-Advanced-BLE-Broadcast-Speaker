/**
******************************************************************************
* @file    stm32_term_tasks_noos.c
* @author  MCD Application Team
* @brief   Manage terminal communication baremetal SW IT
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

/* Includes ------------------------------------------------------------------*/
#include <string.h>

#include "stm32_term_tasks.h"

#ifndef UTIL_TERM_TASK_RTOS_USED

#include "stm32_buff.h"

/* Global variables ----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/
typedef struct t_stm32_term
{
  UTIL_BYTE_BUFF_t     hdleBuff;
} UTIL_TERM_tasks_t;

/* Private variables ---------------------------------------------------------*/
static UTIL_TERM_tasks_t *gContext;

/* Private function prototypes -----------------------------------------------*/
static void s_UTIL_TERM_TASK_start(void);
static void s_UTIL_TERM_TASK_process(void);
static int32_t s_term_task_start_if_ok(int32_t const error);

/* Private macros ------------------------------------------------------------*/
#ifndef UTIL_TERM_TASK_MAX_RCV_RING_BUFFER
  #define UTIL_TERM_TASK_MAX_RCV_RING_BUFFER  2048
#endif

#ifndef UTIL_TERM_TASK_PRIORITY
  #define UTIL_TERM_TASK_PRIORITY (0x0F)
#endif

#ifndef UTIL_TERM_TASK_IRQHandler
  #define UTIL_TERM_TASK_IRQHandler EXTI2_IRQHandler //ADC_IRQHandler
#endif

#ifndef UTIL_TERM_TASK_IRQ
  #define UTIL_TERM_TASK_IRQ EXTI2_IRQn //ADC_IRQn
#endif

/* Functions Definition ------------------------------------------------------*/
/**
* @brief  Term task
* @param  argument: pointer that is passed to the thread function as start argument.
* @retval None
*/
#ifdef UTIL_TERM_TASK_IRQHandler
void UTIL_TERM_TASK_IRQHandler(void);
void UTIL_TERM_TASK_IRQHandler(void)
{
  s_UTIL_TERM_TASK_process();
}
#endif


/**
* @brief  creates task for Term capture
* @param  None
* @retval None
*/
int32_t UTIL_TERM_TASK_create(void)
{
  int32_t error = UTIL_ERROR_NONE;

  gContext = UTIL_TERM_Alloc(UTIL_TERM_MEM_PULL, sizeof(*gContext));
  if (gContext == NULL)
  {
    error = UTIL_ERROR_ALLOC;
  }
  if (error == UTIL_ERROR_NONE)
  {
    memset(gContext, 0, sizeof(*gContext));
    error = UTIL_BYTE_BUFF_alloc(&gContext->hdleBuff, UTIL_TERM_TASK_MAX_RCV_RING_BUFFER);
  }
  if (error == UTIL_ERROR_NONE)
  {
    HAL_NVIC_SetPriority(UTIL_TERM_TASK_IRQ, (int32_t)UTIL_TERM_TASK_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(UTIL_TERM_TASK_IRQ);
  }

  return error;
}


static void s_UTIL_TERM_TASK_start(void)
{
  HAL_NVIC_SetPendingIRQ(UTIL_TERM_TASK_IRQ);
}


/**
* @brief  terminates task for Term capture
* @param  None
* @retval None
*/
void UTIL_TERM_TASK_terminate(void)
{
  /* Stop the thread.*/
  HAL_NVIC_DisableIRQ(UTIL_TERM_TASK_IRQ);
  HAL_NVIC_SetPriority(UTIL_TERM_TASK_IRQ, 0x0F, 0);

  UTIL_BYTE_BUFF_free(&gContext->hdleBuff);
  UTIL_TERM_Free(gContext);
}


/**
* @brief send data
*
* @param pData data buffer
* @param pData data buffer size
* @return error
*/
int32_t UTIL_TERM_TASK_send_data_from_isr(uint8_t const *const pData, size_t const size)
{
  int32_t error;

  UTIL_UART_DISABLE_IRQ();
  error = UTIL_BYTE_BUFF_feed(&gContext->hdleBuff, pData, size);
  UTIL_UART_ENABLE_IRQ();

  return s_term_task_start_if_ok(error);
}


/**
* @brief send data
*
* @param pData data buffer
* @param pData data buffer size
* @return error
*/
int32_t UTIL_TERM_TASK_send_data(uint8_t const *const pData, size_t const size)
{
  int32_t error = UTIL_BYTE_BUFF_feed(&gContext->hdleBuff, pData, size);

  return s_term_task_start_if_ok(error);
}


/**
* @brief wait data
*
* @param pData data buffer
* @param size data buffer size
* @param timeout  timeout
* @param pRemainingBytes pointer to remaining size in the buffer
* @return error
*/
int32_t UTIL_TERM_TASK_wait_data(uint8_t *const pData, size_t const size, uint32_t const timeout, size_t *const pRemainingBytes)
{
  int32_t  error     = UTIL_ERROR_NONE;
  uint32_t tickStart = HAL_GetTick();

  while ((error == UTIL_ERROR_NONE) && (gContext->hdleBuff.availables < size))
  {
    if ((HAL_GetTick() - tickStart) > timeout)
    {
      error = UTIL_ERROR_TIMEOUT; /* signal a timeout */
    }
  }

  if (error == UTIL_ERROR_NONE)
  {
    error = UTIL_BYTE_BUFF_consume(&gContext->hdleBuff, pData, size);
    *pRemainingBytes = 0;
    switch (error)
    {
      case UTIL_ERROR_NONE:
        *pRemainingBytes = gContext->hdleBuff.availables;
        break;
      case UTIL_ERROR_ALLOC:
        UTIL_TERM_printf("UTIL_BYTE_BUFF_consume => Error, NULL pointer\n");
        break;
      case UTIL_ERROR_UNDERRUN:
        UTIL_TERM_printf("UTIL_BYTE_BUFF_consume => Underrun\n");
        break;
      default:
        UTIL_TERM_printf("UTIL_BYTE_BUFF_consume => unknown error %d\n", error);
        break;
    }
  }

  return error;
}


/**
* @brief Command processing thread
*
*/
static void s_UTIL_TERM_TASK_process(void)
{
  while (UTIL_TERM_process() == UTIL_ERROR_NONE)
  {
  }
}



static int32_t s_term_task_start_if_ok(int32_t const error)
{
  switch (error)
  {
    case UTIL_ERROR_NONE:
      // it's OK => launch UTIL_TERM_TASK
      s_UTIL_TERM_TASK_start();
      break;
    case UTIL_ERROR_ALLOC:
      UTIL_TERM_printf("UTIL_BYTE_BUFF_feed => Error, NULL pointer\n");
      break;
    case UTIL_ERROR_OVERRUN:
      UTIL_TERM_printf("UTIL_BYTE_BUFF_feed => Overrun\n");
      break;
    default:
      UTIL_TERM_printf("UTIL_BYTE_BUFF_feed => unknown error %d\n", error);
      break;
  }

  return error;
}

#endif  // !UTIL_TERM_TASK_RTOS_USED
