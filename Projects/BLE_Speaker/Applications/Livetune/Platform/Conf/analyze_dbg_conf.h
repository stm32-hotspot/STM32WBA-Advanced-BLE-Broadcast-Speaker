/**
******************************************************************************
* @file          analyze_dbg_conf.h
* @author        MCD Application Team
* @brief         interface to use a digital analyzer on the arduino D0-D5
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

#ifndef __ANALYZER_DEBUG_TP__
#define __ANALYZER_DEBUG_TP__
#ifdef ST_USE_DEBUG

#if defined(USE_STM32H735G_DK)
  #include "stm32h7xx_hal.h"

  #define ANALYZER_DBG_INIT_DV_PIN_0   GPIO_PIN_15
  #define ANALYZER_DBG_INIT_DV_PORT_0  GPIOB
  #define ANALYZER_DBG_INIT_DV_PIN_1   GPIO_PIN_14
  #define ANALYZER_DBG_INIT_DV_PORT_1  GPIOB
  #define ANALYZER_DBG_INIT_DV_PIN_2   GPIO_PIN_3
  #define ANALYZER_DBG_INIT_DV_PORT_2  GPIOG
  #define ANALYZER_DBG_INIT_DV_PIN_3   GPIO_PIN_4
  #define ANALYZER_DBG_INIT_DV_PORT_3  GPIOG
  #define ANALYZER_DBG_INIT_CLK()      __HAL_RCC_GPIOB_CLK_ENABLE();__HAL_RCC_GPIOG_CLK_ENABLE()
#endif
#if defined(USE_STM32H6570_DK)
  #include "stm32n6xx_hal.h"
  #define ANALYZER_DBG_INIT_DV_PIN_0   GPIO_PIN_6
  #define ANALYZER_DBG_INIT_DV_PORT_0  GPIOF
  #define ANALYZER_DBG_INIT_DV_PIN_1   GPIO_PIN_5
  #define ANALYZER_DBG_INIT_DV_PORT_1  GPIOD
  #define ANALYZER_DBG_INIT_DV_PIN_2   GPIO_PIN_0
  #define ANALYZER_DBG_INIT_DV_PORT_2  GPIOD
  #define ANALYZER_DBG_INIT_DV_PIN_3   GPIO_PIN_9
  #define ANALYZER_DBG_INIT_DV_PORT_3  GPIOE
  #define ANALYZER_DBG_INIT_CLK()      __HAL_RCC_GPIOF_CLK_ENABLE();__HAL_RCC_GPIOD_CLK_ENABLE();__HAL_RCC_GPIOE_CLK_ENABLE()
#endif

#if defined(USE_STM32H747I_DK)
  #include "stm32h7xx_hal.h"
  #define ANALYZER_DBG_INIT_DV_PIN_0   GPIO_PIN_9
  #define ANALYZER_DBG_INIT_DV_PORT_0  GPIOJ
  #define ANALYZER_DBG_INIT_DV_PIN_1   GPIO_PIN_8
  #define ANALYZER_DBG_INIT_DV_PORT_1  GPIOJ
  #define ANALYZER_DBG_INIT_DV_PIN_2   GPIO_PIN_3
  #define ANALYZER_DBG_INIT_DV_PORT_2  GPIOJ
  #define ANALYZER_DBG_INIT_DV_PIN_3   GPIO_PIN_8
  #define ANALYZER_DBG_INIT_DV_PORT_3  GPIOF
  #define ANALYZER_DBG_INIT_CLK()      __HAL_RCC_GPIOJ_CLK_ENABLE();__HAL_RCC_GPIOF_CLK_ENABLE()
#endif


#if defined(USE_STM32H573I_DK)
  #include "stm32h5xx_hal.h"
  #define ANALYZER_DBG_INIT_DV_PIN_0   GPIO_PIN_11
  #define ANALYZER_DBG_INIT_DV_PORT_0  GPIOB
  #define ANALYZER_DBG_INIT_DV_PIN_1   GPIO_PIN_10
  #define ANALYZER_DBG_INIT_DV_PORT_1  GPIOB
  #define ANALYZER_DBG_INIT_DV_PIN_2   GPIO_PIN_15
  #define ANALYZER_DBG_INIT_DV_PORT_2  GPIOG
  #define ANALYZER_DBG_INIT_DV_PIN_3   GPIO_PIN_4
  #define ANALYZER_DBG_INIT_DV_PORT_3  GPIOG
  #define ANALYZER_DBG_INIT_CLK()      __HAL_RCC_GPIOB_CLK_ENABLE();__HAL_RCC_GPIOG_CLK_ENABLE()

#endif


#define ANALYZER_DBG_INIT_DV()                                    \
    {                                                         \
        GPIO_InitTypeDef GPIO_InitStruct;                     \
        memset(&GPIO_InitStruct, 0, sizeof(GPIO_InitStruct)); \
        ANALYZER_DBG_INIT_CLK();                                  \
        GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;          \
        GPIO_InitStruct.Pull  = GPIO_PULLUP;                  \
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;         \
        GPIO_InitStruct.Pin   = ANALYZER_DBG_INIT_DV_PIN_0;             \
        HAL_GPIO_Init(ANALYZER_DBG_INIT_DV_PORT_0, &GPIO_InitStruct);   \
        GPIO_InitStruct.Pin   = ANALYZER_DBG_INIT_DV_PIN_1;             \
        HAL_GPIO_Init(ANALYZER_DBG_INIT_DV_PORT_1, &GPIO_InitStruct);   \
        GPIO_InitStruct.Pin   = ANALYZER_DBG_INIT_DV_PIN_2;             \
        HAL_GPIO_Init(ANALYZER_DBG_INIT_DV_PORT_2, &GPIO_InitStruct);   \
        GPIO_InitStruct.Pin   = ANALYZER_DBG_INIT_DV_PIN_3;             \
        HAL_GPIO_Init(ANALYZER_DBG_INIT_DV_PORT_3, &GPIO_InitStruct);   \
    }

#define ANALYZER_DBG_TOGGLE_DV(pin)                     \
    {                                               \
        if (pin == 0)                               \
            HAL_GPIO_TogglePin(ANALYZER_DBG_INIT_DV_PORT_0, ANALYZER_DBG_INIT_DV_PIN_0); \
        if (pin == 1)                               \
            HAL_GPIO_TogglePin(ANALYZER_DBG_INIT_DV_PORT_1, ANALYZER_DBG_INIT_DV_PIN_1); \
        if (pin == 2)                               \
            HAL_GPIO_TogglePin(ANALYZER_DBG_INIT_DV_PORT_2, ANALYZER_DBG_INIT_DV_PIN_2);  \
        if (pin == 3)                               \
            HAL_GPIO_TogglePin(ANALYZER_DBG_INIT_DV_PORT_3, ANALYZER_DBG_INIT_DV_PIN_3);  \
    }


#define ANALYZER_DBG_PULSE_DV(pin)                     \
    {                                               \
        if (pin == 0)                               \
            {HAL_GPIO_WritePin(ANALYZER_DBG_INIT_DV_PORT_0, ANALYZER_DBG_INIT_DV_PIN_0,GPIO_PIN_SET); \
            HAL_GPIO_WritePin(ANALYZER_DBG_INIT_DV_PORT_0, ANALYZER_DBG_INIT_DV_PIN_0,GPIO_PIN_RESET);} \
        if (pin == 1)                               \
            {HAL_GPIO_WritePin(ANALYZER_DBG_INIT_DV_PORT_1, ANALYZER_DBG_INIT_DV_PIN_1,GPIO_PIN_SET); \
            HAL_GPIO_WritePin(ANALYZER_DBG_INIT_DV_PORT_1, ANALYZER_DBG_INIT_DV_PIN_1,GPIO_PIN_RESET);} \
        if (pin == 2)                               \
            {HAL_GPIO_WritePin(ANALYZER_DBG_INIT_DV_PORT_2, ANALYZER_DBG_INIT_DV_PIN_2,GPIO_PIN_SET); \
            HAL_GPIO_WritePin(ANALYZER_DBG_INIT_DV_PORT_2, ANALYZER_DBG_INIT_DV_PIN_2,GPIO_PIN_RESET);} \
        if (pin == 3)                               \
            {HAL_GPIO_WritePin(ANALYZER_DBG_INIT_DV_PORT_3, ANALYZER_DBG_INIT_DV_PIN_3,GPIO_PIN_SET); \
            HAL_GPIO_WritePin(ANALYZER_DBG_INIT_DV_PORT_3, ANALYZER_DBG_INIT_DV_PIN_3,GPIO_PIN_RESET);} \
    }



#define ANALYZER_DBG_SET_DV(pin, state)                                                       \
    {                                                                                     \
        if (pin == 0)                                                                     \
            HAL_GPIO_WritePin(ANALYZER_DBG_INIT_DV_PORT_0, ANALYZER_DBG_INIT_DV_PIN_0, state ? GPIO_PIN_SET : GPIO_PIN_RESET); \
        if (pin == 1)                                                                     \
            HAL_GPIO_WritePin(ANALYZER_DBG_INIT_DV_PORT_1, ANALYZER_DBG_INIT_DV_PIN_1, state ? GPIO_PIN_SET : GPIO_PIN_RESET); \
        if (pin == 2)                                                                     \
            HAL_GPIO_WritePin(ANALYZER_DBG_INIT_DV_PORT_2, ANALYZER_DBG_INIT_DV_PIN_2, state ? GPIO_PIN_SET : GPIO_PIN_RESET);  \
        if (pin == 3)                                                                     \
            HAL_GPIO_WritePin(ANALYZER_DBG_INIT_DV_PORT_3, ANALYZER_DBG_INIT_DV_PIN_3, state ? GPIO_PIN_SET : GPIO_PIN_RESET);  \
    }


#else
#define ANALYZER_DBG_INIT_DV()
#define ANALYZER_DBG_TOGGLE_DV(pin)
#define ANALYZER_DBG_SET_DV(pin, state)
#endif

#endif