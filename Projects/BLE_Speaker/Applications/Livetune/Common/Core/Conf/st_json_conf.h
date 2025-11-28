/**
  ******************************************************************************
  * @file    st_json_conf.h
  * @author  MCD Application Team
  * @brief   Header conf for json_*.c module
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
#ifndef __ST_JSON_CONF_H
#define __ST_JSON_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "traces.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
#define JSON_ASSERT(a)                                                                                                    \
  do                                                                                                                      \
  {                                                                                                                       \
    if ((a) == 0)                                                                                                         \
    {                                                                                                                     \
      trace_print(TRACE_OUTPUT_UART, TRACE_LVL_ERROR_FATAL, "JSON:assert %s:%d\r\n", __FUNCTION__, __LINE__);             \
    }                                                                                                                     \
  }                                                                                                                       \
  while(0)

#define JSON_ERR_PRINT(error)                                                                                             \
  do                                                                                                                      \
  {                                                                                                                       \
    if ((error) != JSON_OK)                                                                                               \
    {                                                                                                                     \
      trace_print(TRACE_OUTPUT_UART, TRACE_LVL_ERROR, "JSON:error %d %s:%d\r\n", (error), __FUNCTION__, __LINE__);        \
    }                                                                                                                     \
  }                                                                                                                       \
  while(0)

#define JSON_PRINT(...)                                                                                                   \
  do                                                                                                                      \
  {                                                                                                                       \
    bool traceAsynchronous = trace_setAsynchronous(false);                                                                \
    trace_print(TRACE_OUTPUT_UART, TRACE_LVL_INFO, __VA_ARGS__);                                                          \
    trace_setAsynchronous(traceAsynchronous);                                                                             \
  }                                                                                                                       \
  while(0)

/* Exported functions ------------------------------------------------------- */

#ifdef __cplusplus
}
#endif

#endif  /* __ST_JSON_CONF_H */



