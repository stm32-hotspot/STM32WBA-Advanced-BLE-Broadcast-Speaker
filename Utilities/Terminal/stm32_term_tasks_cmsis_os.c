/**
******************************************************************************
* @file    stm32_term_tasks_cmsis_os.c
* @author  MCD Application Team
* @brief   Manage terminal communication threads
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
#include <string.h>   /* for memset */
#include "stm32_term_tasks.h"

#ifdef UTIL_TERM_TASK_RTOS_USED

#include "st_os_hl.h"
/*cstat -MISRAC2012-Rule-10.4_a  A third-party produces a violation message we cannot fix*/
#include "FreeRTOS.h"
/*cstat +MISRAC2012-Rule-10.4_a */
#include "stream_buffer.h"

/* Global variables ----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
typedef struct t_stm32_term
{
  StreamBufferHandle_t hdleStream;
  void                *pOsTermThread_handler;
} UTIL_TERM_tasks_t;

/* Private function prototypes -----------------------------------------------*/
static void s_UTIL_TERM_TASK_process(ARGUMENT_TYPE argument);
static void s_UTIL_TERM_process(uint16_t const param, ARGUMENT_TYPE argument);

/* Private macros ------------------------------------------------------------*/
#ifndef UTIL_TERM_TASK_MAX_RCV_RING_BUFFER
  #define UTIL_TERM_TASK_MAX_RCV_RING_BUFFER  2048
#endif

#ifndef UTIL_TERM_TASK_NAME
  #define UTIL_TERM_TASK_NAME                 "UTIL_TERMINAL"
#endif

#ifndef UTIL_TERM_TASK_PRIORITY
  #define UTIL_TERM_TASK_PRIORITY             (st_task_priority)ST_Priority_Below_Normal2
#endif

#ifndef UTIL_TERM_TASK_STACK_SIZE
  #ifndef UTIL_TERM_TASK_STACK_SIZE_WORD32
    #define UTIL_TERM_TASK_STACK_SIZE         2048UL
  #else
    #define UTIL_TERM_TASK_STACK_SIZE         (UTIL_TERM_TASK_STACK_SIZE_WORD32 << 2U)
  #endif
#endif


/* Private variables ---------------------------------------------------------*/
static UTIL_TERM_tasks_t gContext;


/* Functions Definition ------------------------------------------------------*/

/**
* @brief  creates task for Term
* @param  None
* @retval None
*/
int32_t UTIL_TERM_TASK_create(void)
{
  int32_t error = UTIL_ERROR_NONE;

  memset(&gContext, 0, sizeof(gContext));

  /* alloc ring buffer and dma */
  gContext.hdleStream = xStreamBufferCreate(UTIL_TERM_TASK_MAX_RCV_RING_BUFFER, 1);
  if (gContext.hdleStream == NULL)
  {
    error = UTIL_ERROR_ALLOC;
  }

  if (error == UTIL_ERROR_NONE)
  {
    /* Create Term task  */
    st_os_hl_task_create(UTIL_TERM_TASK_NAME,                // thread_name
                         NULL,                               // queue_name (no queue)
                         UTIL_TERM_TASK_STACK_SIZE,          // stack_size
                         UTIL_TERM_TASK_PRIORITY,            // priority
                         0UL,                                // queue_deepness (no queue)
                         osWaitForever,                      // queue_timeout
                         s_UTIL_TERM_TASK_process,           // thread_func
                         s_UTIL_TERM_process,                // task_func
                         NULL,                               // cmsisos void *argument
                         false,                              // not a back-ground task
                         false,                              // don't activate back-ground task
                         false,                              // log CMSIS OS stack/queue level
                         &gContext.pOsTermThread_handler);   // returned handler
    if (gContext.pOsTermThread_handler == NULL)
    {
      error = UTIL_ERROR;
    }
  }

  return error;
}


/**
* @brief  terminates task for Term
* @param  None
* @retval None
*/
void UTIL_TERM_TASK_terminate(void)
{
  if (gContext.pOsTermThread_handler != NULL)
  {
    if (st_os_hl_task_terminate(&gContext.pOsTermThread_handler, 1000UL))
    {
      gContext.pOsTermThread_handler = NULL;
    }
    else
    {
      /* error */
      while (1)
      {
      }
    }
  }

  if (gContext.hdleStream != NULL)
  {
    vStreamBufferDelete(gContext.hdleStream);
    gContext.hdleStream = NULL;
  }
}


/**
* @brief send data
*
* @param pData data buffer
* @param pData data buffer size
* @param tmo  timeout
* @return None
*/
int32_t UTIL_TERM_TASK_send_data_from_isr(uint8_t const *const pData, size_t const size)
{
  if (size)
  {
    portBASE_TYPE taskWoken = pdFALSE;
    xStreamBufferSendFromISR(gContext.hdleStream, pData, size, &taskWoken);
    portEND_SWITCHING_ISR(taskWoken);
  }
  return UTIL_ERROR_NONE;
}


/**
* @brief send data
*
* @param pData data buffer
* @param pData data buffer size
* @param tmo  timeout
* @return None
*/
int32_t UTIL_TERM_TASK_send_data(uint8_t const *const pData, size_t const size)
{
  xStreamBufferSend(gContext.hdleStream, pData, size, UTIL_TERM_SYNC_TIMEOUT);
  return UTIL_ERROR_NONE;
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
  int32_t ret = UTIL_ERROR_NONE;
  if (xStreamBufferReceive(gContext.hdleStream, pData, size, timeout) == size)
  {
    *pRemainingBytes = xStreamBufferBytesAvailable(gContext.hdleStream);
  }
  else
  {
    *pRemainingBytes = 0;
    ret = UTIL_ERROR_TIMEOUT;
  }
  /* signal a timeout */
  return ret;
}


/**
* @brief Command processing thread
*
* @param argument the opaque parameter
*/
static void s_UTIL_TERM_TASK_process(ARGUMENT_TYPE argument)
{
  st_os_hl_task_thread(gContext.pOsTermThread_handler, argument);/*cstat !MISRAC2012-Dir-4.13_h false positive, this message means nothing */
}


static void s_UTIL_TERM_process(uint16_t const param, ARGUMENT_TYPE argument)
{
  /* param is useless for UTIL_TERM_process */
  UTIL_TERM_process();
}

#endif /* UTIL_TERM_TASK_RTOS_USED */

