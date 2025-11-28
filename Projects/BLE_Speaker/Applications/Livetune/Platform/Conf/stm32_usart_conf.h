/**
  ******************************************************************************
  * @file    stm32_uart_conf.h
  * @author  MCD Application Team
  * @brief   Header for stm32_usart*.c module
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
#ifndef __STM32_USART_CONF_H
#define __STM32_USART_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "platform_def.h"
#include "stm32h5xx_hal.h"
#include "irq_utils.h"

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/

#define UTIL_UART_MEMORY_SECTION_HW_BUFFERS ".noncacheable"

#ifdef USE_LIVETUNE_DESIGNER
#define CONF_UART_DMA UTIL_UART_DMA_RX_TX
#define UTIL_UART_CONFIG "921600:8:N:2"

#endif


#ifdef AUDIO_CHAIN_CONF_TUNING_CLI_USED
#include "stm32_term_conf.h"
#ifndef CONF_UART_DMA
#define CONF_UART_DMA UTIL_UART_DMA_RX
#endif
#else
#define CONF_UART_DMA UTIL_UART_DMA_NONE
#endif

#define UTIL_UART_INSTANCE                     USART3
#define UTIL_UART_CLK_ENABLE()                 __USART3_CLK_ENABLE()
#define UTIL_UART_CLK_DISABLE()                __USART3_CLK_DISABLE()
#define UTIL_UART_FORCE_RESET()                __USART3_FORCE_RESET()
#define UTIL_UART_RELEASE_RESET()              __USART3_RELEASE_RESET()

#define UTIL_UART_PINS_SPEED                   GPIO_SPEED_FREQ_HIGH

#define UTIL_UART_TX_PIN                       GPIO_PIN_10
#define UTIL_UART_TX_GPIO_PORT                 GPIOC
#define UTIL_UART_TX_GPIO_CLK_ENABLE()         __GPIOC_CLK_ENABLE()
#define UTIL_UART_TX_GPIO_CLK_DISABLE()        __GPIOC_CLK_DISABLE()
#define UTIL_UART_TX_AF                        GPIO_AF7_USART3
#define UTIL_UART_RX_PIN                       GPIO_PIN_11
#define UTIL_UART_RX_GPIO_PORT                 GPIOC
#define UTIL_UART_RX_GPIO_CLK_ENABLE()         __GPIOC_CLK_ENABLE()
#define UTIL_UART_RX_GPIO_CLK_DISABLE()        __GPIOC_CLK_DISABLE()
#define UTIL_UART_RX_AF                        GPIO_AF7_USART3

#define UTIL_UART_IRQn                         USART3_IRQn
#define UTIL_UART_IRQHandler                   USART3_IRQHandler

#define  UTIL_UART_TIMx_INSTANCE                       TIM7
#define  UTIL_UART_TIMx_IRQHandler            TIM7_IRQHandler
#define  UTIL_UART_TIMx_CLK_ENABLE()          __HAL_RCC_TIM7_CLK_ENABLE()
#define  UTIL_UART_TIMx_IRQn                  TIM7_IRQn
#define  UTIL_UART_TIMx_PRIORITY              5




#ifndef UTIL_UART_DMA_MODE_RX
#define UTIL_UART_DMA_MODE_RX          DMA_NORMAL /* FixMe implement DMA_NORMAL only on H5 */
#endif

#ifndef UTIL_UART_DMA_MODE_TX
#define UTIL_UART_DMA_MODE_TX          DMA_NORMAL
#endif




#define UTIL_UART_DMA                          DMA2
#define UTIL_UART_DMA_CLK_ENABLE()             __HAL_RCC_USART3_CLK_ENABLE()
#define UTIL_UART_DMA_CLK_DISABLE()            __HAL_RCC_USART3_CLK_DISABLE()

#define UTIL_UART_DMA_STREAM_RX                GPDMA1_Channel6
#define UTIL_UART_DMA_CHANNEL_RX               GPDMA1_REQUEST_USART3_RX
#define UTIL_UART_DMA_STREAM_RX_IRQHandler     GPDMA1_Channel6_IRQHandler
#define UTIL_UART_DMA_STREAM_RX_IRQn           GPDMA1_Channel6_IRQn

#define UTIL_UART_DMA_STREAM_TX                GPDMA1_Channel7
#define UTIL_UART_DMA_CHANNEL_TX               GPDMA1_REQUEST_USART3_TX
#define UTIL_UART_DMA_STREAM_TX_IRQn           GPDMA1_Channel7_IRQn
#define UTIL_UART_DMA_STREAM_TX_IRQHandler     GPDMA1_Channel7_IRQHandler


#define UTIL_UART_DISABLE_IRQ disable_irq_with_cnt
#define UTIL_UART_ENABLE_IRQ  enable_irq_with_cnt

#ifdef __cplusplus
}
#endif

#endif /* __STM32_USART_CONF_H */

