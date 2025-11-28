/**
  ******************************************************************************
  * @file    traces_conf.h
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
#ifndef __TRACES_CONF_H
#define __TRACES_CONF_H

/* Includes ------------------------------------------------------------------*/
// includes for HAL of your device: for instance for STM32H7
// #include "stm32h7xx_hal.h"
// #include "stm32h7xx_hal_def.h"
// #include "stm32h7xx.h"

#define TRACE_SCROLL_SCREEN

//#define TRACE_DISPLAY_FORCED_SYNCHRONOUS // should be avoided

#ifdef USE_SCREEN
  #define TRACE_USE_DISPLAY
#endif

#ifdef USE_UART
  #define TRACE_USE_UART
#endif

#endif  /* __TRACES_CONF_H */



