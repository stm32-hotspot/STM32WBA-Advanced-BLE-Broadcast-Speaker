/**
******************************************************************************
* @file    stm32_term_tasks_stos.c
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

#include "st_os.h"

/* Global variables ----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
typedef struct t_stm32_term
{
  st_stream      hStream;
  st_task      hTask;
  volatile uint8_t         bTerminalEnabled;
} UTIL_TERM_tasks_t;

/* Private function prototypes -----------------------------------------------*/
//static void s_UTIL_TERM_TASK_process(ARGUMENT_TYPE argument);
static void s_UTIL_TERM_process(const void *pCookie);

/* Private macros ------------------------------------------------------------*/
#ifndef UTIL_TERM_TASK_MAX_RCV_RING_BUFFER
  #define UTIL_TERM_TASK_MAX_RCV_RING_BUFFER  2048
#endif

static uint8_t tStreamBuffer[UTIL_TERM_TASK_MAX_RCV_RING_BUFFER];


#ifndef UTIL_TERM_TASK_NAME
  #define UTIL_TERM_TASK_NAME                 "UTIL_TERMINAL"
#endif

#ifndef UTIL_TERM_TASK_PRIORITY
  #define UTIL_TERM_TASK_PRIORITY             (ST_Priority_Below_Normal2)
#endif

#ifndef UTIL_TERM_TASK_STACK_SIZE_WORD32
  #define UTIL_TERM_TASK_STACK_SIZE_WORD32    800UL
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
  if (st_os_stream_create(&gContext.hStream, tStreamBuffer, sizeof(tStreamBuffer)) != ST_OS_OK)
  {
    error = UTIL_ERROR_ALLOC;
  }
  gContext.bTerminalEnabled = TRUE;
  if (st_os_task_create(&gContext.hTask, UTIL_TERM_TASK_NAME, s_UTIL_TERM_process, &gContext, UTIL_TERM_TASK_STACK_SIZE_WORD32, UTIL_TERM_TASK_PRIORITY) != ST_OS_OK)
  {
    error = UTIL_ERROR_ALLOC;
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
  gContext.bTerminalEnabled = FALSE;
  st_os_task_delete(&gContext.hTask);
  st_os_stream_delete(&gContext.hStream);
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
  UTIL_TERM_TASK_send_data(pData, size);
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
  if (size)
  {
    st_os_stream_write(&gContext.hStream, pData, size);
  }
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
  size_t read = st_os_stream_read(&gContext.hStream, pData, size, timeout);
  if (read == size)
  {
    *pRemainingBytes = st_os_stream_read_available(&gContext.hStream);
  }
  else
  {
    *pRemainingBytes = 0;
    ret = UTIL_ERROR_TIMEOUT;
  }
  /* signal a timeout */
  return ret;
}

static void s_UTIL_TERM_process(const void *pCookie)
{
  const UTIL_TERM_tasks_t *pHandle = pCookie;
  while (pHandle->bTerminalEnabled)
  {
    if (UTIL_TERM_process() == UTIL_ERROR)
    {
      st_os_task_tdelay(1);
    }
  }
}

#endif /* UTIL_TERM_TASK_RTOS_USED */

