/**
  ******************************************************************************
  * @file    stm32_term.h
  * @author  MCD Application Team
  * @brief   Header for stm32_term.c module
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2019(-2021) STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _STM32_TERM_CONF_
#define _STM32_TERM_CONF_

#ifdef __cplusplus
extern "C" {
#endif
/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
#ifdef USE_THREADS
#define UTIL_TERM_TASK_RTOS_USED
#endif

#if defined(USE_LIVETUNE_DESIGNER) || defined(USE_LIVETUNE_TUNER)

#define UTIL_TERM_TASK_STACK_SIZE_WORD32 2000UL  /* The default terminal stack is too short with this project */
#define UTIL_TERM_PROMPT_PROLOGUE "cli> "        /* Overload the prolog terminal text as minimal */
#define UTIL_TERM_SYNC_TIMEOUT 1000U             /* Restore the terminal timeout, the initial default value produce read errors */

#endif /* USE_LIVETUNE_DESIGNER || USE_LIVETUNE_TUNER */


//#define UTIL_UART_PRIO            12
//#define UTIL_UART_DMA_RX_PRIO     8
//#define UTIL_UART_DMA_TX_PRIO     8

#define UTIL_UART_BAUD_RATE       921600
#define UTIL_UART_RX_BUFFER_SIZE  (2U*80U)
#define UTIL_UART_TX_BUFFER_SIZE  (2U*80U)

#define UTIL_TERM_DMA_BLOCK_SIZE  UTIL_UART_RX_BUFFER_SIZE
#define UTIL_TERM_DMA_CHAR_SIZE   2

#define UTIL_TERM_Free(x)         free(x)
#define UTIL_TERM_Alloc(a,x)      malloc(x)
#define UTIL_TERM_ASSERT(...)     ((void)0)
#define UTIL_TERM_MEM_PULL        0

/* Exported functions ------------------------------------------------------- */

#ifdef __cplusplus
}
#endif

#endif /* _STM32_TERM_CONF_ */

