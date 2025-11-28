/**
  ******************************************************************************
  * @file    stm32_usart.h
  * @author  MCD Application Team
  * @brief   Header for stm32_uart.c module
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022(-2022) STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32_USART_H
#define __STM32_USART_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include "stm32_usart_conf.h"
#include "stm32_errors.h"

/* Exported types ------------------------------------------------------------*/

typedef enum
{
  UTIL_UART_DMA_NONE = 0,
  UTIL_UART_DMA_RX,
  UTIL_UART_DMA_TX,
  UTIL_UART_DMA_RX_TX
} UTIL_UART_DMA_t;


typedef enum
{
  UTIL_UART_MUTE_FORCE_ON = 0,
  UTIL_UART_MUTE_FORCE_OFF
} UTIL_UART_MUTE_t;


typedef struct
{
  UTIL_UART_DMA_t     dma;
  uint32_t            mode;
  uint32_t            dma_mode_rx;
  uint32_t            dma_mode_tx;
  UART_HandleTypeDef *pHdle;
  TIM_HandleTypeDef  *pHTim;
  uint32_t            reserved;
} UTIL_UART_t;

typedef struct
{
  int32_t (* init)(UTIL_UART_t *pConf);
  int32_t (* deInit)(UART_HandleTypeDef const *const pHdle);
  int32_t (* transmit)(UART_HandleTypeDef *pHdle, uint8_t *pData, uint32_t size);
  int32_t (* receive)(UART_HandleTypeDef *pHdle,  uint8_t *pData, uint32_t size);
  int32_t (* initDmaRx)(UTIL_UART_t *pConf);
  int32_t (* initDmaTx)(UTIL_UART_t *pConf);
  int32_t (* startDmaRx)(UART_HandleTypeDef *pHdle, uint32_t size);
  int32_t (* startDmaTx)(UART_HandleTypeDef *pHdle, uint32_t size);
  int32_t (* startRxDmaToIdle)(UART_HandleTypeDef *pHdle, uint8_t *pBuffer, uint32_t szBuffer);
  int32_t (* stopDmaRx)(UART_HandleTypeDef *pHdle);
  int32_t (* stopDmaTx)(UART_HandleTypeDef *pHdle);
  void (* setRxBufferAddr)(UART_HandleTypeDef *pHdle);
  void (* setTxBufferAddr)(UART_HandleTypeDef *pHdle);
  void (* initTimeoutIT)(TIM_HandleTypeDef *pHdle, uint32_t itFreqMs);
  void (* setTimeoutIT)(TIM_HandleTypeDef *pHdle, uint32_t state);
} UTIL_UART_cbs_t;

typedef void (*UTIL_UART_SEND_BLOCK_CB)(void *pBlock, void *pCookie);

/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void                UTIL_UART_Lock(void);
void                UTIL_UART_UnLock(void);
void                UTIL_UART_resetCallbacks(UTIL_UART_cbs_t *const pCbs);
void                UTIL_UART_registerCallbacks(UTIL_UART_cbs_t const *const pCbs);
UART_HandleTypeDef *UTIL_UART_GetHdle(void);
int32_t             UTIL_UART_Init(UTIL_UART_t *const pConfig);
int32_t             UTIL_UART_DeInit(UART_HandleTypeDef const *const pHdle);
int32_t             UTIL_UART_StartRxDma(uint32_t size);
int32_t             UTIL_UART_StartTxDma(uint32_t size);
int32_t             UTIL_UART_StartRxDmaToIdle(void *pBuffer, uint32_t szBuffer);
int32_t             UTIL_UART_BlockRxDma(void);
int32_t             UTIL_UART_BlockTxDma(void);
bool                UTIL_UART_Mute(UTIL_UART_MUTE_t mute);
bool                UTIL_UART_IsLogMuted(void);
uint32_t            UTIL_UART_GetRxDmaCounter(void);
uint32_t            UTIL_UART_GetTxDmaCounter(void);
int32_t             UTIL_UART_SendTxData(uint32_t msg_size);
int32_t             UTIL_UART_fputs(char *pData);
int32_t             UTIL_UART_fputs_force(char *pData);
int32_t             UTIL_UART_fputc(int32_t data);
int32_t             UTIL_UART_fputc_force(int32_t data);
int32_t             UTIL_UART_write(unsigned char *pData, uint32_t szData);
int32_t             UTIL_UART_Receive(int32_t *pData);
uint8_t            *UTIL_UART_GetRxBuffer(void);
uint8_t            *UTIL_UART_GetTxBuffer(void);
void                UTIL_UART_setCallbacks(void);
int32_t             UTIL_UART_SendBlockAsync(const void *pBlock, uint32_t szBlock, UTIL_UART_SEND_BLOCK_CB cbFreeBlock, void *pMem2Free, void *pCookie);
int32_t             UTIL_UART_EnableLogAsync(uint8_t bState, uint8_t szBufferStream);
void                UTIL_UART_FlushAsyncChars(void);
bool                UTIL_UART_BlockAsyncBusy(void);
void                UTIL_UART_IRQ_cycleMeasure_Init(void);
void                UTIL_UART_IRQ_cycleMeasure_Reset(void);
void                UTIL_UART_IRQ_cycleMeasure_Start(void);
void                UTIL_UART_IRQ_cycleMeasure_Stop(void);
void                UTIL_UART_DMA_STREAM_RX_IRQ_cycleMeasure_Init(void);
void                UTIL_UART_DMA_STREAM_RX_IRQ_cycleMeasure_Reset(void);
void                UTIL_UART_DMA_STREAM_RX_IRQ_cycleMeasure_Start(void);
void                UTIL_UART_DMA_STREAM_RX_IRQ_cycleMeasure_Stop(void);
void                UTIL_UART_DMA_STREAM_TX_IRQ_cycleMeasure_Init(void);
void                UTIL_UART_DMA_STREAM_TX_IRQ_cycleMeasure_Reset(void);
void                UTIL_UART_DMA_STREAM_TX_IRQ_cycleMeasure_Start(void);
void                UTIL_UART_DMA_STREAM_TX_IRQ_cycleMeasure_Stop(void);

#ifdef __cplusplus
}
#endif

#endif /* __STM32_USART_H */
