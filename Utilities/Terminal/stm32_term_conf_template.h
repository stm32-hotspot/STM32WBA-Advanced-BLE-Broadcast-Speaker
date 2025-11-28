/**
  ******************************************************************************
  * @file        stm32_term_conf_template.h
  * @author      SMD/AME application teams
  * @brief       STSAFE-A1XX Middleware configuration file
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32_TERM_CONF_TEMPLATE_H
#define __STM32_TERM_CONF_TEMPLATE_H

#ifdef __cplusplus
extern "C" {
#endif

//#define UTIL_TERM_DMA_RX            DMA2_Stream4
//#define UTIL_TERM_DMA_RX_CH         DMA_REQUEST_USART1_RX
//#define UTIL_TERM_DMA_RX_IRQHandler DMA2_Stream4_IRQHandler
//#define UTIL_TERM_DMA_RX_ENABLE     __HAL_RCC_DMA2_CLK_ENABLE
//#define UTIL_TERM_DMA_RX_IRQn       DMA2_Stream4_IRQn
//#define UTIL_TERM_DMA_RX_PRIORITY   5
//#define UTIL_TERM_DMA_BLOCK_SIZE    80
//#define UTIL_TERM_DMA_CHAR_SIZE     2

#define UTIL_UART_PRIO 12
#define UTIL_UART_BAUD_RATE               115200
#define UTIL_UART_RX_BUFFER_SIZE          80
#define UTIL_UART_TX_BUFFER_SIZE          80


#define UTIL_TERM_DMA_RX            DMA2_Stream1
#define UTIL_TERM_DMA_RX_CH         DMA_REQUEST_USART3_RX
#define UTIL_TERM_DMA_RX_IRQHandler DMA2_Stream1_IRQHandler
#define UTIL_TERM_DMA_RX_ENABLE     __HAL_RCC_DMA2_CLK_ENABLE
#define UTIL_TERM_DMA_RX_IRQn       DMA2_Stream1_IRQn
#define UTIL_TERM_DMA_RX_PRIORITY   6
#define UTIL_TERM_DMA_BLOCK_SIZE    UTIL_UART_RX_BUFFER_SIZE
#define UTIL_TERM_DMA_CHAR_SIZE     2

#define UTIL_TERM_Free(x) free(x)
#define UTIL_TERM_Alloc(a,x) malloc(x)
#define UTIL_TERM_ASSERT(...)                 ((void)0)
#define UTIL_TERM_MEM_PULL 0
#ifdef __cplusplus
}
#endif

#endif /* __STM32_TERM_CONF_TEMPLATE_H */


