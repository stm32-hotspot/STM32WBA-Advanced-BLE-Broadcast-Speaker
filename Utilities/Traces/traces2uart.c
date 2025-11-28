/**
  ******************************************************************************
  * @file    traces2uart.c
  * @author  MCD Application Team
  * @brief   basic traces mechanism over uart
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

/* Includes ------------------------------------------------------------------*/
#include "traces_internals.h"


#ifdef TRACE_USE_UART

/* Global variables ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Functions Definition ------------------------------------------------------*/

/**
  * @brief  Send string to uart.
  * @param  string
  * @retval None
  */
void trace2uart(char *const pString)
{
  trace_coreLock_tasks();
  TRACE_fputs(pString);
  trace_coreUnlock_tasks();
}


#ifdef PUTCHAR_PROTOTYPE
/**
  * @brief  Re targets the C library putchar function to the USART.
  * @param  character
  * @retval input character
  */
PUTCHAR_PROTOTYPE
{
  /* Place your implementation of fputc here */
  TRACE_fputc(ch);
  return ch;
}
#endif
#endif // TRACE_USE_UART



