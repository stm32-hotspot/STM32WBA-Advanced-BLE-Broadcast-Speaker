/**
******************************************************************************
* @file    stm32_usart_u5xx_template.c
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
DMA_NodeTypeDef Node_DmaRx = {0};
DMA_QListTypeDef List_DmaRx = {0};
DMA_HandleTypeDef hRxDma = {0};

DMA_NodeTypeDef Node_DmaTx = {0};
DMA_QListTypeDef List_DmaTx = {0};
DMA_HandleTypeDef hTxDma = {0};

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
VAR_DECLARE_ALIGN4_AT_SECTION(volatile uint8_t, UTIL_UART_RxBuffer[UTIL_UART_RX_BUFFER_SIZE * 4], UTIL_UART_MEMORY_SECTION_HW_BUFFERS); /*cstat !MISRAC2012-Rule-1.4_a extended language features needed*/
VAR_DECLARE_ALIGN4_AT_SECTION(volatile uint8_t, UTIL_UART_TxBuffer[UTIL_UART_TX_BUFFER_SIZE * 4], UTIL_UART_MEMORY_SECTION_HW_BUFFERS); /*cstat !MISRAC2012-Rule-1.4_a extended language features needed*/

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
  pUartHdle->Instance          = UTIL_UART_INSTANCE;
  /* Standard uart config string "921600:8:N:1" */
  sscanf(UTIL_UART_CONFIG, "%d:%c:%c:%s", &baudRate, &wordlen, &parity, stopBits);

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
  return (int32_t) HAL_UART_DeInit((UART_HandleTypeDef *) pHdle);
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
static int32_t s_initDmaRx(UTIL_UART_t *const pConf)
{
  int32_t ret = UTIL_ERROR_NONE;

  /* Enable DMA clock */
  UTIL_UART_DMA_CLK_ENABLE();

  UART_HandleTypeDef *huart = pUartHdle;
  if ((pConf->dma == UTIL_UART_DMA_RX) || ((pConf->dma == UTIL_UART_DMA_RX_TX)))
  {
    DMA_NodeConfTypeDef pNodeConfig;

    hRxDma.Instance = UTIL_UART_DMA_STREAM_RX;
    hRxDma.InitLinkedList.Priority = DMA_LOW_PRIORITY_HIGH_WEIGHT;
    hRxDma.InitLinkedList.LinkStepMode = DMA_LSM_FULL_EXECUTION;
    hRxDma.InitLinkedList.LinkAllocatedPort = DMA_LINK_ALLOCATED_PORT0;
    hRxDma.InitLinkedList.TransferEventMode = DMA_TCEM_LAST_LL_ITEM_TRANSFER;
    hRxDma.InitLinkedList.LinkedListMode = DMA_LINKEDLIST_CIRCULAR;
    if (HAL_DMAEx_List_Init(&hRxDma) != HAL_OK)
    {
      return UTIL_ERROR;
    }
    if (HAL_DMA_ConfigChannelAttributes(&hRxDma, DMA_CHANNEL_NPRIV) != HAL_OK)
    {
      return UTIL_ERROR;
    }

    /* Set node configuration ################################################*/
    pNodeConfig.NodeType = DMA_GPDMA_LINEAR_NODE;
    pNodeConfig.Init.Request = UTIL_UART_DMA_CHANNEL_RX;
    pNodeConfig.Init.BlkHWRequest = DMA_BREQ_SINGLE_BURST;
    pNodeConfig.Init.Direction = DMA_PERIPH_TO_MEMORY;
    pNodeConfig.Init.SrcInc = DMA_SINC_FIXED;
    pNodeConfig.Init.DestInc = DMA_DINC_INCREMENTED;
    pNodeConfig.Init.SrcDataWidth = DMA_SRC_DATAWIDTH_BYTE;
    pNodeConfig.Init.DestDataWidth = DMA_DEST_DATAWIDTH_BYTE;
    pNodeConfig.Init.SrcBurstLength = 1;
    pNodeConfig.Init.DestBurstLength = 1;
    pNodeConfig.Init.TransferAllocatedPort = DMA_SRC_ALLOCATED_PORT0 | DMA_DEST_ALLOCATED_PORT0;
    pNodeConfig.Init.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;
    pNodeConfig.TriggerConfig.TriggerPolarity = DMA_TRIG_POLARITY_MASKED;
    pNodeConfig.DataHandlingConfig.DataExchange = DMA_EXCHANGE_NONE;
    pNodeConfig.DataHandlingConfig.DataAlignment = DMA_DATA_RIGHTALIGN_ZEROPADDED;
    pNodeConfig.SrcAddress = 0;
    pNodeConfig.DstAddress = 0;
    pNodeConfig.DataSize = 0;

    /* Build Node_GPDMA1_Channel0 Node */
    ret |= HAL_DMAEx_List_BuildNode(&pNodeConfig, &Node_DmaRx);

    /* Insert Node_GPDMA1_Channel0 to Queue */
    ret |= HAL_DMAEx_List_InsertNode_Tail(&List_DmaRx, &Node_DmaRx);

    __HAL_LINKDMA(huart, hdmarx, hRxDma);

    if (HAL_DMAEx_List_SetCircularMode(&List_DmaRx) != HAL_OK)
    {
      return UTIL_ERROR;
    }
    if (HAL_DMAEx_List_LinkQ(&hRxDma, &List_DmaRx) != HAL_OK)
    {
      return UTIL_ERROR;
    }
  }
  return ret;
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
    DMA_NodeConfTypeDef pNodeConfig;

    hTxDma.Instance = UTIL_UART_DMA_STREAM_TX;
    hTxDma.InitLinkedList.Priority = DMA_LOW_PRIORITY_HIGH_WEIGHT;
    hTxDma.InitLinkedList.LinkStepMode = DMA_LSM_FULL_EXECUTION;
    hTxDma.InitLinkedList.LinkAllocatedPort = DMA_LINK_ALLOCATED_PORT0;
    hTxDma.InitLinkedList.TransferEventMode = DMA_TCEM_LAST_LL_ITEM_TRANSFER;
    hTxDma.InitLinkedList.LinkedListMode = DMA_LINKEDLIST_CIRCULAR;
    if (HAL_DMAEx_List_Init(&hTxDma) != HAL_OK)
    {
      return UTIL_ERROR;
    }
    if (HAL_DMA_ConfigChannelAttributes(&hTxDma, DMA_CHANNEL_NPRIV) != HAL_OK)
    {
      return UTIL_ERROR;
    }

    /* Set node configuration ################################################*/
    pNodeConfig.NodeType = DMA_GPDMA_LINEAR_NODE;
    pNodeConfig.Init.Request = UTIL_UART_DMA_CHANNEL_TX;
    pNodeConfig.Init.BlkHWRequest = DMA_BREQ_SINGLE_BURST;
    pNodeConfig.Init.Direction = DMA_PERIPH_TO_MEMORY;
    pNodeConfig.Init.SrcInc = DMA_SINC_FIXED;
    pNodeConfig.Init.DestInc = DMA_DINC_INCREMENTED;
    pNodeConfig.Init.SrcDataWidth = DMA_SRC_DATAWIDTH_BYTE;
    pNodeConfig.Init.DestDataWidth = DMA_DEST_DATAWIDTH_BYTE;
    pNodeConfig.Init.SrcBurstLength = 1;
    pNodeConfig.Init.DestBurstLength = 1;
    pNodeConfig.Init.TransferAllocatedPort = DMA_SRC_ALLOCATED_PORT0 | DMA_DEST_ALLOCATED_PORT0;
    pNodeConfig.Init.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;
    pNodeConfig.TriggerConfig.TriggerPolarity = DMA_TRIG_POLARITY_MASKED;
    pNodeConfig.DataHandlingConfig.DataExchange = DMA_EXCHANGE_NONE;
    pNodeConfig.DataHandlingConfig.DataAlignment = DMA_DATA_RIGHTALIGN_ZEROPADDED;
    pNodeConfig.SrcAddress = 0;
    pNodeConfig.DstAddress = 0;
    pNodeConfig.DataSize = 0;

    /* Build Node_GPDMA1_Channel0 Node */
    ret |= HAL_DMAEx_List_BuildNode(&pNodeConfig, &Node_DmaTx);

    /* Insert Node_GPDMA1_Channel0 to Queue */
    ret |= HAL_DMAEx_List_InsertNode_Tail(&List_DmaTx, &Node_DmaTx);

    __HAL_LINKDMA(huart, hdmarx, hTxDma);

    if (HAL_DMAEx_List_SetCircularMode(&List_DmaTx) != HAL_OK)
    {
      return UTIL_ERROR;
    }
    if (HAL_DMAEx_List_LinkQ(&hTxDma, &List_DmaTx) != HAL_OK)
    {
      return UTIL_ERROR;
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

