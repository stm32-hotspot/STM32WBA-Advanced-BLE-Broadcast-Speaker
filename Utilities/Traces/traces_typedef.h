/**
  ******************************************************************************
  * @file    traces_typedef.h
  * @author  MCD Application Team
  * @brief   Header for traces.c module
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


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TRACES_TYPEDEF_H
#define __TRACES_TYPEDEF_H

#ifdef __cplusplus
extern "C" {
#endif
/* Includes ------------------------------------------------------------------*/
#include <stdarg.h>
#include <stdint.h>

/* Exported constants --------------------------------------------------------*/
#define TRACE_UART_RESET                     "\033[0m"
#define TRACE_UART_BOLD                      "\033[1m"
#define TRACE_UART_UNDERLINED                "\033[4m"
#define TRACE_UART_REVERSED                  "\033[7m"
#define TRACE_UART_CHARACTER_BLACK           "\033[30m"
#define TRACE_UART_CHARACTER_RED             "\033[31m"
#define TRACE_UART_CHARACTER_GREEN           "\033[32m"
#define TRACE_UART_CHARACTER_YELLOW          "\033[33m"
#define TRACE_UART_CHARACTER_BLUE            "\033[34m"
#define TRACE_UART_CHARACTER_MAGENTA         "\033[35m"
#define TRACE_UART_CHARACTER_CYAN            "\033[36m"
#define TRACE_UART_CHARACTER_WHITE           "\033[37m"
#define TRACE_UART_CHARACTER_LIGHT_BLACK     "\033[90m"
#define TRACE_UART_CHARACTER_LIGHT_RED       "\033[91m"
#define TRACE_UART_CHARACTER_LIGHT_GREEN     "\033[92m"
#define TRACE_UART_CHARACTER_LIGHT_YELLOW    "\033[93m"
#define TRACE_UART_CHARACTER_LIGHT_BLUE      "\033[94m"
#define TRACE_UART_CHARACTER_LIGHT_MAGENTA   "\033[95m"
#define TRACE_UART_CHARACTER_LIGHT_CYAN      "\033[96m"
#define TRACE_UART_CHARACTER_LIGHT_WHITE     "\033[97m"
#define TRACE_UART_CHARACTER_BRIGHT_BLACK    "\033[30;1m"
#define TRACE_UART_CHARACTER_BRIGHT_RED      "\033[31;1m"
#define TRACE_UART_CHARACTER_BRIGHT_GREEN    "\033[32;1m"
#define TRACE_UART_CHARACTER_BRIGHT_YELLOW   "\033[33;1m"
#define TRACE_UART_CHARACTER_BRIGHT_BLUE     "\033[34;1m"
#define TRACE_UART_CHARACTER_BRIGHT_MAGENTA  "\033[35;1m"
#define TRACE_UART_CHARACTER_BRIGHT_CYAN     "\033[36;1m"
#define TRACE_UART_CHARACTER_BRIGHT_WHITE    "\033[37;1m"
#define TRACE_UART_BACKGROUND_BLACK          "\033[40m"
#define TRACE_UART_BACKGROUND_RED            "\033[41m"
#define TRACE_UART_BACKGROUND_GREEN          "\033[42m"
#define TRACE_UART_BACKGROUND_YELLOW         "\033[43m"
#define TRACE_UART_BACKGROUND_BLUE           "\033[44m"
#define TRACE_UART_BACKGROUND_MAGENTA        "\033[45m"
#define TRACE_UART_BACKGROUND_CYAN           "\033[46m"
#define TRACE_UART_BACKGROUND_WHITE          "\033[47m"
#define TRACE_UART_BACKGROUND_LIGHT_BLACK    "\033[100m"
#define TRACE_UART_BACKGROUND_LIGHT_RED      "\033[101m"
#define TRACE_UART_BACKGROUND_LIGHT_GREEN    "\033[102m"
#define TRACE_UART_BACKGROUND_LIGHT_YELLOW   "\033[103m"
#define TRACE_UART_BACKGROUND_LIGHT_BLUE     "\033[104m"
#define TRACE_UART_BACKGROUND_LIGHT_MAGENTA  "\033[105m"
#define TRACE_UART_BACKGROUND_LIGHT_CYAN     "\033[106m"
#define TRACE_UART_BACKGROUND_LIGHT_WHITE    "\033[107m"
#define TRACE_UART_BACKGROUND_BRIGHT_BLACK   "\033[40;1m"
#define TRACE_UART_BACKGROUND_BRIGHT_RED     "\033[41;1m"
#define TRACE_UART_BACKGROUND_BRIGHT_GREEN   "\033[42;1m"
#define TRACE_UART_BACKGROUND_BRIGHT_YELLOW  "\033[43;1m"
#define TRACE_UART_BACKGROUND_BRIGHT_BLUE    "\033[44;1m"
#define TRACE_UART_BACKGROUND_BRIGHT_MAGENTA "\033[45;1m"
#define TRACE_UART_BACKGROUND_BRIGHT_CYAN    "\033[46;1m"

/* Exported types ------------------------------------------------------------*/
enum
{
  TRACE_LVL_VERBOSE_BIT,                                                    /*!< Trace verbose     messages */
  TRACE_LVL_DEBUG_BIT,                                                      /*!< Trace debug       messages */
  TRACE_LVL_LOG_BIT,                                                        /*!< Trace log         messages */
  TRACE_LVL_INFO_BIT,                                                       /*!< Trace info        messages */
  TRACE_LVL_INFO2_BIT,                                                      /*!< Trace info2       messages */
  TRACE_LVL_WARNING_BIT,                                                    /*!< Trace warning     messages */
  TRACE_LVL_ERROR_BIT,                                                      /*!< Trace error       messages */
  TRACE_LVL_ERROR_FATAL_BIT,                                                /*!< Trace fatal error messages */
  NB_TRACE_LVL
};

typedef uint16_t traceLvl_t;

/* These flags can be combinated to produce a trace configuration  */

#define TRACE_LVL_MUTE        (0U)                                          /*!< No Trace messages */
#define TRACE_LVL_VERBOSE     (1U << TRACE_LVL_VERBOSE_BIT)                 /*!< Trace verbose     messages */
#define TRACE_LVL_DEBUG       (1U << TRACE_LVL_DEBUG_BIT)                   /*!< Trace debug       messages */
#define TRACE_LVL_LOG         (1U << TRACE_LVL_LOG_BIT)                     /*!< Trace log         messages */
#define TRACE_LVL_INFO        (1U << TRACE_LVL_INFO_BIT)                    /*!< Trace info        messages */
#define TRACE_LVL_INFO2       (1U << TRACE_LVL_INFO2_BIT)                   /*!< Trace info2       messages */
#define TRACE_LVL_WARNING     (1U << TRACE_LVL_WARNING_BIT)                 /*!< Trace warning     messages */
#define TRACE_LVL_ERROR       (1U << TRACE_LVL_ERROR_BIT)                   /*!< Trace error       messages */
#define TRACE_LVL_ERROR_FATAL (1U << TRACE_LVL_ERROR_FATAL_BIT)             /*!< Trace fatal error messages */
#define TRACE_LVL_ALL         0xFFU                                         /*!< All traces messages */



typedef enum
{
  TRACE_OUTPUT_UART    = (1U << 0),
  TRACE_OUTPUT_DISPLAY = (1U << 1),
  TRACE_OUTPUT_ALL     = (TRACE_OUTPUT_UART | TRACE_OUTPUT_DISPLAY)
} traceOutput_t;

typedef void (trace_t)(const char *const pCompName, traceLvl_t const level, const char *const pFile, int const line, const char *const pFormat, va_list args);


#ifdef __cplusplus
}
#endif

#endif /* __TRACES_TYPEDEF_H */

