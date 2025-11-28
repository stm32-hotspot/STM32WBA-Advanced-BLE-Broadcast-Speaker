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
#include "stdio.h"
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
#ifndef JSON_ASSERT
#define JSON_ASSERT(val)                                                                                              \
  do                                                                                                                \
  {                                                                                                                 \
    if ((val)  == 0)                                                                                         \
    {                                                                                                               \
      printf( "JSON:assert %d %s:%d\r\n", (int)(val), __FUNCTION__, __LINE__);                                         \
      while(1);                                                                                                     \
    }                                                                                                               \
  }                                                                                                                 \
  while(0)
#endif
#ifndef JSON_ERR_PRINT    
#define JSON_ERR_PRINT(error)                                                                                       \
  do                                                                                                                \
  {                                                                                                                 \
    if ((error) != JSON_OK)                                                                                         \
    {                                                                                                               \
      printf( "JSON:error %d %s:%d\r\n", (error), __FUNCTION__, __LINE__);                                          \
    }                                                                                                               \
  }                                                                                                                 \
  while(0)
#endif
#ifndef JSON_PRINT
#define JSON_PRINT(...)  printf(__VA_ARGS__)                                                                                      
#endif

/* Exported functions ------------------------------------------------------- */

#ifdef __cplusplus
}
#endif

#endif  /* __ST_JSON_CONF_H */



