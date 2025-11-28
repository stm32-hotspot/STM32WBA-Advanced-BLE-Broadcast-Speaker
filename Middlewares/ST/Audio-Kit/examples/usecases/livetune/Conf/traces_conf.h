/**
******************************************************************************
* @file    traces_conf.h
* @author  MCD Application Team
* @brief   Header Conf for traces.c module
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TRACES_CONF_H
#define __TRACES_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32xxx_voice_bsp.h"
#include "irq_utils.h"

/* Exported constants --------------------------------------------------------*/

#define TRACE_SCROLL_SCREEN

#ifdef USE_SCREEN
//#define TRACE_USE_DISPLAY
#endif

#ifdef USE_UART
#define TRACE_USE_UART
#define TRACE_fputc UTIL_UART_fputc
#define TRACE_fputs UTIL_UART_fputs
#endif

#define TRACE_DISABLE_IRQ disable_irq_with_cnt
#define TRACE_ENABLE_IRQ  enable_irq_with_cnt

/* Set trace level */
#if defined(USE_LIVETUNE_DESIGNER)
#define TRACE_LVL_DEFAULT  (TRACE_LVL_ERROR_FATAL | TRACE_LVL_ERROR | TRACE_LVL_WARNING | TRACE_LVL_INFO | TRACE_LVL_LOG | TRACE_LVL_DEBUG)
#elif defined(USE_LIVETUNE_TUNER)
#define TRACE_LVL_DEFAULT  (TRACE_LVL_ERROR_FATAL | TRACE_LVL_ERROR | TRACE_LVL_WARNING | TRACE_LVL_INFO | TRACE_LVL_LOG)
#elif defined(AUDIO_CHAIN_RELEASE)
#define TRACE_LVL_DEFAULT  (TRACE_LVL_ERROR_FATAL | TRACE_LVL_ERROR | TRACE_LVL_LOG)
#endif

#if defined(__ICCARM__)
/* New definition from EWARM V9, compatible with EWARM8 */
#define PUTCHAR_PROTOTYPE int putchar(int ch)
#elif defined ( __CC_ARM ) || defined(__ARMCC_VERSION)
/* ARM Compiler 5/6*/
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#elif defined(__GNUC__)
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#endif /* __ICCARM__ */


#define TRACE_MAX_LOG_LINE_SIZE 256U

/* Exported macro ------------------------------------------------------------*/
/* Exported structures--------------------------------------------------------*/
/* Exported variables ------------------------------------------------------- */
/* Exported functions ------------------------------------------------------- */
/* Exported macro ------------------------------------------------------------*/
/* Exported structures--------------------------------------------------------*/
/* Exported variables ------------------------------------------------------- */
/* Exported functions ------------------------------------------------------- */
#ifdef __cplusplus
}
#endif

#endif  /* __TRACES_CONF_H */



