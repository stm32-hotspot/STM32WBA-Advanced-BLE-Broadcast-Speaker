/**
******************************************************************************
* @file    stm32_usart_f4xx_template.c
* @author  MCD Application Team
* @brief   This file provides set of firmware functions to manage UART
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

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "stdio.h"
#include "stm32_usart.h"

static UART_HandleTypeDef *pUartHdle;

/* Private macros ------------------------------------------------------------*/
#ifndef UTIL_UART_PRIO
  #define UTIL_UART_PRIO 12
#endif

#ifndef UTIL_UART_DMA_RX_PRIO
  #define UTIL_UART_DMA_RX_PRIO 5
#endif

#ifndef UTIL_UART_DMA_TX_PRIO
  #define UTIL_UART_DMA_TX_PRIO 5
#endif

/* USART Baud Rate configuration */
#ifndef UTIL_UART_CONFIG
  #define UTIL_UART_CONFIG           "921600:8:N:1"
#endif

#ifndef UTIL_UART_RX_BUFFER_SIZE
  #define UTIL_UART_RX_BUFFER_SIZE          1024
#endif /* UTIL_UART_RX_BUFFER_SIZE */

#ifndef UTIL_UART_TX_BUFFER_SIZE
  #define UTIL_UART_TX_BUFFER_SIZE          1024
#endif /* UTIL_UART_TX_BUFFER_SIZE */

/* Private variables ---------------------------------------------------------*/
VAR_DECLARE_ALIGN4_AT_SECTION(volatile uint8_t, UTIL_UART_RxBuffer[UTIL_UART_RX_BUFFER_SIZE], UTIL_UART_MEMORY_SECTION_HW_BUFFERS); /*cstat !MISRAC2012-Rule-1.4_a extended language features needed*/
VAR_DECLARE_ALIGN4_AT_SECTION(volatile uint8_t, UTIL_UART_TxBuffer[UTIL_UART_TX_BUFFER_SIZE], UTIL_UART_MEMORY_SECTION_HW_BUFFERS); /*cstat !MISRAC2012-Rule-1.4_a extended language features needed*/

/* Private function prototypes -----------------------------------------------*/
static int32_t s_init(UTIL_UART_t *const pConf);
static int32_t s_deInit(UART_HandleTypeDef const *const pHdle);
static int32_t s_transmit(UART_HandleTypeDef *pHdle, uint8_t *pData, uint32_t size);
//static int32_t s_receive        (UART_HandleTypeDef *pHdle,       uint8_t *pData, uint32_t size);

#ifdef UTIL_UART_DMA
  static int32_t s_initDmaRx(UTIL_UART_t *const pConf);
  static int32_t s_initDmaTx(UTIL_UART_t *const pConf);
  static int32_t s_startDmaRx(UART_HandleTypeDef *pHdle, uint32_t size);
  static int32_t s_startDmaTx(UART_HandleTypeDef *pHdle, uint32_t size);
  static int32_t s_stopDmaRx(UART_HandleTypeDef *pHdle);
  static int32_t s_stopDmaTx(UART_HandleTypeDef *pHdle);
  static void    s_setRxBufferAddr(UART_HandleTypeDef *pHdle);
  static void    s_setTxBufferAddr(UART_HandleTypeDef *pHdle);
#endif

/* Functions Definition ------------------------------------------------------*/

void UTIL_UART_setCallbacks(void)
{
  UTIL_UART_cbs_t util_uart_cbs;
  UTIL_UART_resetCallbacks(&util_uart_cbs);

  util_uart_cbs.init             = s_init;
  util_uart_cbs.deInit           = s_deInit;
  util_uart_cbs.transmit         = s_transmit;
  //  util_uart_cbs.receive          = s_receive;
  #ifdef UTIL_UART_DMA
  util_uart_cbs.initDmaRx        = s_initDmaRx;
  util_uart_cbs.initDmaTx        = s_initDmaTx;
  util_uart_cbs.startDmaRx       = s_startDmaRx;
  util_uart_cbs.startDmaTx       = s_startDmaTx;
  util_uart_cbs.stopDmaRx        = s_stopDmaRx;
  util_uart_cbs.stopDmaTx        = s_stopDmaTx;
  util_uart_cbs.setTxBufferAddr  = s_setTxBufferAddr;
  util_uart_cbs.setRxBufferAddr  = s_setRxBufferAddr;
  #endif
  UTIL_UART_registerCallbacks(&util_uart_cbs);
}


/**
* @brief  Configures the USART for vCom
* @param  None
* @retval None
*/
static int32_t s_init(UTIL_UART_t *const pConf)
{
  int32_t ret = UTIL_ERROR_NONE;
  uint32_t  baudRate;
  char      parity;
  char      wordlen;
  char      stopBits[10];
  pUartHdle = pConf->pHdle;

  /* Standard uart config string "921600:8:N:1" */
  sscanf(UTIL_UART_CONFIG, "%d:%c:%c:%s", &baudRate, &wordlen, &parity, stopBits);


  pUartHdle->Instance          = UTIL_UART_INSTANCE;

  pUartHdle->Init.BaudRate     = baudRate;
  pUartHdle->Init.WordLength   = UART_WORDLENGTH_8B;
  if (wordlen == '7')
  {
    pUartHdle->Init.WordLength   = UART_WORDLENGTH_7B;
  }
  if (wordlen == '9')
  {
    pUartHdle->Init.WordLength   = UART_WORDLENGTH_9B;
  }


  pUartHdle->Init.StopBits     = UART_STOPBITS_1;
  if (strcmp(stopBits, "0.5") == 0)
  {
    pUartHdle->Init.StopBits     = UART_STOPBITS_0_5;
  }
  if (strcmp(stopBits, "1.5") == 0)
  {
    pUartHdle->Init.StopBits     = UART_STOPBITS_1_5;
  }
  if (strcmp(stopBits, "2") == 0)
  {
    pUartHdle->Init.StopBits     = UART_STOPBITS_2;
  }
  pUartHdle->Init.Parity       = UART_PARITY_NONE;
  if (parity == 'E')
  {
    pUartHdle->Init.Parity    = UART_PARITY_EVEN;
  }
  if (parity == 'O')
  {
    pUartHdle->Init.Parity    = UART_PARITY_ODD;
  }
  pUartHdle->Init.HwFlowCtl    = UART_HWCONTROL_NONE;
  pUartHdle->Init.Mode         = UART_MODE_TX_RX;
  pUartHdle->Init.OverSampling = UART_OVERSAMPLING_16;


  ret = (int32_t)HAL_UART_Init(pUartHdle);
  return ret;
}

/**
* @brief  Deinit the USART for vCom
* @param  None
* @retval None
*/
static int32_t s_deInit(UART_HandleTypeDef const *const pHdle)
{
  return (int32_t) HAL_UART_DeInit((UART_HandleTypeDef *)pHdle);
}

static int32_t s_transmit(UART_HandleTypeDef *pHdle, uint8_t *pData, uint32_t size)
{
  return HAL_UART_Transmit(pHdle, pData, size, 0xFFFF) ;
}


//static int32_t s_receive(UART_HandleTypeDef *pHdle, uint8_t *pData, uint32_t size)
//{
//  return HAL_UART_Receive(pHdle, pData, size, 0xFFFF) ;
//}


/**
* @brief USART MSP Initialization
*        This function configures the hardware resources used in this example:
*           - Peripheral's clock enable
*           - Peripheral's GPIO Configuration
*           - DMA configuration for transmission request by peripheral
*           - NVIC configuration for DMA interrupt request enable
* @param huart: USART handle pointer
* @retval None
*/
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{

  GPIO_InitTypeDef  GPIO_InitStruct;

  /*##-1- Enable peripherals and GPIO Clocks #################################*/
  /* Enable GPIO clock */
  UTIL_UART_TX_GPIO_CLK_ENABLE();
  UTIL_UART_RX_GPIO_CLK_ENABLE();
  /* Enable USART clock */
  UTIL_UART_CLK_ENABLE();


  /*##-2- Configure peripheral GPIO ##########################################*/
  /* USART TX GPIO pin configuration  */
  GPIO_InitStruct.Pin       = UTIL_UART_TX_PIN;
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull      = GPIO_PULLUP;
  GPIO_InitStruct.Speed     = UTIL_UART_PINS_SPEED;
  GPIO_InitStruct.Alternate = UTIL_UART_TX_AF;

  HAL_GPIO_Init(UTIL_UART_TX_GPIO_PORT, &GPIO_InitStruct);

  /* USART RX GPIO pin configuration  */
  GPIO_InitStruct.Pin = UTIL_UART_RX_PIN;
  GPIO_InitStruct.Alternate = UTIL_UART_RX_AF;

  HAL_GPIO_Init(UTIL_UART_RX_GPIO_PORT, &GPIO_InitStruct);


  HAL_NVIC_SetPriority(UTIL_UART_IRQn, UTIL_UART_PRIO, 0);
  HAL_NVIC_EnableIRQ(UTIL_UART_IRQn);
}



#ifdef UTIL_UART_DMA
static void s_setRxBufferAddr(UART_HandleTypeDef *pHdle)
{
  pHdle->pRxBuffPtr = (uint8_t *)&UTIL_UART_RxBuffer[0];
  pHdle->RxXferSize = UTIL_UART_RX_BUFFER_SIZE;
}


static void s_setTxBufferAddr(UART_HandleTypeDef *pHdle)
{
  pHdle->pRxBuffPtr = (uint8_t *)&UTIL_UART_TxBuffer[0];
  pHdle->RxXferSize = UTIL_UART_TX_BUFFER_SIZE;
}


static int32_t s_startDmaRx(UART_HandleTypeDef *pHdle, uint32_t size)
{
  int32_t error;
  if (size <= UTIL_UART_RX_BUFFER_SIZE)
  {
    HAL_StatusTypeDef status;
    do
    {
      status = HAL_UART_Receive_DMA(pHdle, pHdle->pRxBuffPtr, size);
    }
    while (status == HAL_BUSY);
    error = (int32_t)status;
  }
  else
  {
    error = UTIL_ERROR_ALLOC;
  }
  return error;
}

static int32_t s_startDmaTx(UART_HandleTypeDef *pHdle, uint32_t size)
{
  int32_t error;
  if (size <= UTIL_UART_TX_BUFFER_SIZE)
  {
    HAL_StatusTypeDef status;
    do
    {
      status = HAL_UART_Transmit_DMA(pHdle, pHdle->pTxBuffPtr, size);
    }
    while (status == HAL_BUSY);
    error = (int32_t)status;
  }
  else
  {
    error = UTIL_ERROR_ALLOC;
  }
  return error;
}


static int32_t s_stopDmaRx(UART_HandleTypeDef *pHdle)
{
  int32_t error = UTIL_ERROR_NONE;
  #ifdef UTIL_UART_DMA_STREAM_RX_IRQHandler
  error = (int32_t) HAL_UART_Abort(pHdle);
  if (error == UTIL_ERROR_NONE)
  {
    HAL_NVIC_EnableIRQ(UTIL_UART_DMA_STREAM_RX_IRQn);
  }
  #endif
  return error;
}

static int32_t s_stopDmaTx(UART_HandleTypeDef *pHdle)
{
  int32_t error = UTIL_ERROR_NONE;
  #ifdef UTIL_UART_DMA_STREAM_TX_IRQHandler
  error = (int32_t) HAL_UART_Abort(pHdle);
  if (error == UTIL_ERROR_NONE)
  {
    HAL_NVIC_EnableIRQ(UTIL_UART_DMA_STREAM_TX_IRQn);
  }
  #endif
  return error;
}


/**
* @brief  Configures the TX DMA of USART for vCom
* @param  None
* @retval None
*/
static int32_t s_initDmaTx(UTIL_UART_t *const pConf)
{
  int32_t ret = UTIL_ERROR_NONE;

  /* Enable DMA clock */
  UTIL_UART_DMA_CLK_ENABLE();

  UART_HandleTypeDef *huart = pUartHdle;
  if ((pConf->dma == UTIL_UART_DMA_TX) || ((pConf->dma == UTIL_UART_DMA_RX_TX)))
  {
    static DMA_HandleTypeDef   hTxDma;

    /*## Configure the DMA streams ##########################################*/
    /* Configure the DMA handler for Transmission process */
    hTxDma.Instance                 = UTIL_UART_DMA_STREAM_TX;
    hTxDma.Init.Channel             = UTIL_UART_DMA_CHANNEL_TX;
    hTxDma.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
    hTxDma.Init.MemBurst            = DMA_MBURST_SINGLE;
    hTxDma.Init.PeriphBurst         = DMA_MBURST_SINGLE;
    hTxDma.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    hTxDma.Init.PeriphInc           = DMA_PINC_DISABLE;
    hTxDma.Init.MemInc              = DMA_MINC_ENABLE;
    hTxDma.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hTxDma.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    hTxDma.Init.Mode                = pConf->dma_mode_tx;
    hTxDma.Init.Priority            = DMA_PRIORITY_MEDIUM;
    hTxDma.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    ret = (int32_t) HAL_DMA_Init(&hTxDma);
    if (ret == UTIL_ERROR_NONE)
    {
      /* Associate the initialized DMA handle to the USART handle */
      __HAL_LINKDMA(huart, hdmatx, hTxDma);
      pUartHdle->hdmatx = &hTxDma;
      /* NVIC configuration for DMA transfer complete interrupt  */
      HAL_NVIC_SetPriority(UTIL_UART_DMA_STREAM_TX_IRQn, UTIL_UART_DMA_TX_PRIO, 0);
      HAL_NVIC_EnableIRQ(UTIL_UART_DMA_STREAM_TX_IRQn);
    }
  }
  return ret;

}

/**
* @brief  Configures the TX DMA of USART for vCom
* @param  None
* @retval None
*/
static int32_t s_initDmaRx(UTIL_UART_t *const pConf)
{
  int32_t ret = UTIL_ERROR_NONE;

  /* Enable DMA clock */
  UTIL_UART_DMA_CLK_ENABLE();

  UART_HandleTypeDef *huart = pUartHdle;
  if ((pConf->dma == UTIL_UART_DMA_RX) || ((pConf->dma == UTIL_UART_DMA_RX_TX)))
  {
    static DMA_HandleTypeDef   hRxDma;
    /* Configure the DMA handler for reception process */
    hRxDma.Instance                 = UTIL_UART_DMA_STREAM_RX;
    hRxDma.Init.Channel             = UTIL_UART_DMA_CHANNEL_RX;
    hRxDma.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
    hRxDma.Init.MemBurst            = DMA_MBURST_SINGLE;
    hRxDma.Init.PeriphBurst         = DMA_MBURST_SINGLE;
    hRxDma.Init.Direction           = DMA_PERIPH_TO_MEMORY;
    hRxDma.Init.PeriphInc           = DMA_PINC_DISABLE;
    hRxDma.Init.MemInc              = DMA_MINC_ENABLE;
    hRxDma.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hRxDma.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    hRxDma.Init.Mode                = pConf->dma_mode_rx;
    hRxDma.Init.Priority            = DMA_PRIORITY_MEDIUM;
    hRxDma.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;

    ret = (int32_t) HAL_DMA_Init(&hRxDma);
    if (ret == UTIL_ERROR_NONE)
    {
      /* Associate the initialized DMA handle to the the USART handle */
      __HAL_LINKDMA(huart, hdmarx, hRxDma);

      /* NVIC configuration for DMA transfer complete interrupt  */
      HAL_NVIC_SetPriority(UTIL_UART_DMA_STREAM_RX_IRQn, UTIL_UART_DMA_RX_PRIO, 0);
      HAL_NVIC_EnableIRQ(UTIL_UART_DMA_STREAM_RX_IRQn);
      pUartHdle->hdmarx = &hRxDma;
    }
  }

  return ret;
}
#endif /* UTIL_UART_DMA */


/* IT Handlers definition*/
/**
  * @brief  This function handles UART interrupt request.
  * @param  None
  * @retval None
  */
#ifdef UTIL_UART_IRQHandler
void UTIL_UART_IRQHandler(void);
void UTIL_UART_IRQHandler(void)
{
  HAL_UART_IRQHandler(pUartHdle);
}
#endif

/**
 * @brief DMA RX IRQ Handler
 *
 */
#ifdef UTIL_UART_DMA_STREAM_RX_IRQHandler
void UTIL_UART_DMA_STREAM_RX_IRQHandler(void);
void UTIL_UART_DMA_STREAM_RX_IRQHandler(void)
{
  HAL_DMA_IRQHandler(pUartHdle->hdmarx);
}
#endif

/**
 * @brief DMA RX IRQ Handler
 *
 */
#ifdef UTIL_UART_DMA_STREAM_TX_IRQHandler
void UTIL_UART_DMA_STREAM_TX_IRQHandler(void);
void UTIL_UART_DMA_STREAM_TX_IRQHandler(void)
{
  HAL_DMA_IRQHandler(pUartHdle->hdmatx);
}
#endif

/* Private Functions Definition ----------------------------------------------*/

