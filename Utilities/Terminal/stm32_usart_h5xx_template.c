/**
******************************************************************************
* @file    stm32_usart_h5xx_template.c
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
#include <stdio.h>
#include "stm32_term_conf.h"
#include "stm32_usart.h"

static UART_HandleTypeDef *pUartHdle   = NULL;
static TIM_HandleTypeDef   hTimTimeout = {0};
static DMA_HandleTypeDef   hTxDma      = {0};
static DMA_NodeTypeDef     hTxDmaNode  = {0};
static DMA_QListTypeDef    hTxtDmaQ    = {0};
static DMA_HandleTypeDef   hRxDma      = {0};
static DMA_NodeTypeDef     hRxDmaNode  = {0};
static DMA_QListTypeDef    hRxtDmaQ    = {0};

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
  #define UTIL_UART_CONFIG "921600:8:N:2"
#endif

#ifndef UTIL_UART_RX_BUFFER_SIZE
  #define UTIL_UART_RX_BUFFER_SIZE 1024U
#endif /* UTIL_UART_RX_BUFFER_SIZE */

#ifndef UTIL_UART_TX_BUFFER_SIZE
  #define UTIL_UART_TX_BUFFER_SIZE 1024U
#endif /* UTIL_UART_TX_BUFFER_SIZE */

/* Private variables ---------------------------------------------------------*/
VAR_DECLARE_ALIGN4_AT_SECTION(uint8_t, UTIL_UART_RxBuffer[UTIL_UART_RX_BUFFER_SIZE], UTIL_UART_MEMORY_SECTION_HW_BUFFERS);  /*cstat !MISRAC2012-Rule-1.4_a extended language features needed*/
VAR_DECLARE_ALIGN4_AT_SECTION(uint8_t, UTIL_UART_TxBuffer[UTIL_UART_TX_BUFFER_SIZE], UTIL_UART_MEMORY_SECTION_HW_BUFFERS);  /*cstat !MISRAC2012-Rule-1.4_a extended language features needed*/

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
  static int32_t s_startRxDmaToIdle(UART_HandleTypeDef *pHdle, uint8_t *pBuffer, uint32_t szBuffer);
  static int32_t s_stopDmaRx(UART_HandleTypeDef *pHdle);
  static int32_t s_stopDmaTx(UART_HandleTypeDef *pHdle);
  static void    s_setRxBufferAddr(UART_HandleTypeDef *pHdle);
  static void    s_setTxBufferAddr(UART_HandleTypeDef *pHdle);
  static void    s_initTimeoutIT(TIM_HandleTypeDef *pHdle, uint32_t period);
  static void    s_setTimeoutIT(TIM_HandleTypeDef *pHdle, uint32_t state);
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
  util_uart_cbs.startRxDmaToIdle = s_startRxDmaToIdle;
  util_uart_cbs.stopDmaRx        = s_stopDmaRx;
  util_uart_cbs.stopDmaTx        = s_stopDmaTx;
  util_uart_cbs.setTxBufferAddr  = s_setTxBufferAddr;
  util_uart_cbs.setRxBufferAddr  = s_setRxBufferAddr;

  if (CONF_UART_DMA == UTIL_UART_DMA_RX_TX)
  {
    util_uart_cbs.initTimeoutIT = s_initTimeoutIT;
    util_uart_cbs.setTimeoutIT  = s_setTimeoutIT;
  }
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
  int32_t  ret = UTIL_ERROR_NONE;
  uint32_t baudRate;
  char     parity;
  char     wordlen;
  char     stopBits[10];

  if (CONF_UART_DMA == UTIL_UART_DMA_RX_TX)
  {
    pConf->pHTim = &hTimTimeout;
  }
  pUartHdle           = pConf->pHdle;
  pUartHdle->Instance = UTIL_UART_INSTANCE;


  /* Standard uart config string "921600:8:N:1" */

  sscanf(UTIL_UART_CONFIG, "%d:%c:%c:%s", &baudRate, &wordlen, &parity, stopBits);
  pUartHdle->Init.BaudRate   = baudRate;
  pUartHdle->Init.WordLength = UART_WORDLENGTH_8B;
  if (wordlen == '7')
  {
    pUartHdle->Init.WordLength = UART_WORDLENGTH_7B;
  }
  if (wordlen == '9')
  {
    pUartHdle->Init.WordLength = UART_WORDLENGTH_9B;
  }


  pUartHdle->Init.StopBits = UART_STOPBITS_1;
  if (strcmp(stopBits, "0.5") == 0)
  {
    pUartHdle->Init.StopBits = UART_STOPBITS_0_5;
  }
  if (strcmp(stopBits, "1.5") == 0)
  {
    pUartHdle->Init.StopBits = UART_STOPBITS_1_5;
  }
  if (strcmp(stopBits, "2") == 0)
  {
    pUartHdle->Init.StopBits = UART_STOPBITS_2;
  }
  pUartHdle->Init.Parity = UART_PARITY_NONE;
  if (parity == 'E')
  {
    pUartHdle->Init.Parity = UART_PARITY_EVEN;
  }
  if (parity == 'O')
  {
    pUartHdle->Init.Parity = UART_PARITY_ODD;
  }
  pUartHdle->Init.HwFlowCtl    = UART_HWCONTROL_NONE;
  pUartHdle->Init.Mode         = UART_MODE_TX_RX;
  pUartHdle->Init.OverSampling = UART_OVERSAMPLING_16;

  UTIL_UART_IRQ_cycleMeasure_Init();
  UTIL_UART_IRQ_cycleMeasure_Reset();

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
  return (int32_t)HAL_UART_DeInit((UART_HandleTypeDef *)pHdle);
}

static int32_t s_transmit(UART_HandleTypeDef *pHdle, uint8_t *pData, uint32_t size)
{
  if (size < 0x10000UL)
  {
    return (int32_t)HAL_UART_Transmit(pHdle, pData, (uint16_t)size, 0xFFFFU);
  }
  else
  {
    return (int32_t)HAL_ERROR;
  }
}


//static int32_t s_receive(UART_HandleTypeDef *pHdle, uint8_t *pData, uint32_t size)
//{
//  if (size < 0x10000UL)
//  {
//  return (int32_t)HAL_UART_Receive(pHdle, pData, (uint16_t)size, 0xFFFF);
//  }
//  else
//  {
//    return (int32_t)HAL_ERROR;
//  }
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
  GPIO_InitTypeDef GPIO_InitStruct;

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
  GPIO_InitStruct.Pin       = UTIL_UART_RX_PIN;
  GPIO_InitStruct.Alternate = UTIL_UART_RX_AF;

  HAL_GPIO_Init(UTIL_UART_RX_GPIO_PORT, &GPIO_InitStruct);


  HAL_NVIC_SetPriority(UTIL_UART_IRQn, UTIL_UART_PRIO, 0);
  HAL_NVIC_EnableIRQ(UTIL_UART_IRQn);
}




#ifdef UTIL_UART_DMA

/**
* @brief Init the timer managing the timeout by IT
* @param pHdle: TIM handle pointer
* @param itFreqMs: IT Delay for the timeout
*/

static void s_initTimeoutIT(TIM_HandleTypeDef *pHdle, uint32_t itFreqMs)
{
  RCC_ClkInitTypeDef clkconfig;
  uint32_t           uwTimclock, uwAPB1Prescaler = 0U;
  uint32_t           iLatency;
  memset(pHdle, 0, sizeof(*pHdle));

  /*Configure the TIM6 IRQ priority */
  HAL_NVIC_SetPriority(UTIL_UART_TIMx_IRQn, UTIL_UART_TIMx_PRIORITY, 0U);

  /* Enable the TIM6 global Interrupt */
  HAL_NVIC_EnableIRQ(UTIL_UART_TIMx_IRQn);

  /* Enable TIM6 clock */
  UTIL_UART_TIMx_CLK_ENABLE();

  /* Get clock configuration */
  HAL_RCC_GetClockConfig(&clkconfig, &iLatency);

  /* Get APB1 prescaler */
  uwAPB1Prescaler = clkconfig.APB1CLKDivider;

  /* Compute TIMx clock */
  if (uwAPB1Prescaler == RCC_HCLK_DIV1)
  {
    uwTimclock = HAL_RCC_GetPCLK1Freq();
  }
  else
  {
    uwTimclock = 2U * HAL_RCC_GetPCLK1Freq();
  }

  /* Compute the prescaler value to have TIM counter clock equal to 1MHz */
  uint32_t timerClk = 10000; /* timer clk at 10MHZ, for 260 mhz allows a rang from 1ms to 6.5 secs with an accuracy of 1/10 ms */

  /* Initialize TIM */
  pHdle->Instance               = UTIL_UART_TIMx_INSTANCE;
  pHdle->Init.Period            = (itFreqMs * 10U) - 1U;
  pHdle->Init.Prescaler         = (uint32_t)(uwTimclock / (timerClk)) - 1U;
  pHdle->Init.ClockDivision     = 0;
  pHdle->Init.CounterMode       = TIM_COUNTERMODE_UP;
  pHdle->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  pHdle->Init.RepetitionCounter = 0;
  HAL_TIM_Base_Init(pHdle);
}

/**
* @brief set the timeout IT state
* @param pHdle: TIM handle pointer
*/

static void s_setTimeoutIT(TIM_HandleTypeDef *pHdle, uint32_t state)
{
  if (state)
  {
    /* Start the TIM time Base generation in interrupt mode
       Clear the update flag to prevent an IT at de start of the timeout delay */

    __HAL_TIM_CLEAR_FLAG(pHdle, TIM_FLAG_UPDATE);
    HAL_TIM_Base_Start_IT(pHdle);
  }
  else
  {
    /* Stop the TIM time Base generation in interrupt mode */
    HAL_TIM_Base_Stop_IT(pHdle);
  }
}


static void s_setRxBufferAddr(UART_HandleTypeDef *pHdle)
{
  pHdle->pRxBuffPtr = UTIL_UART_RxBuffer;
  pHdle->RxXferSize = UTIL_UART_RX_BUFFER_SIZE;
}


static void s_setTxBufferAddr(UART_HandleTypeDef *pHdle)
{
  pHdle->pTxBuffPtr = UTIL_UART_TxBuffer;
  pHdle->TxXferSize = UTIL_UART_TX_BUFFER_SIZE;
}


static int32_t s_startRxDmaToIdle(UART_HandleTypeDef *pHdle, uint8_t *pBuffer, uint32_t szBuffer)
{
  /* Initiate a dma terminated by a buffer full or IDLE state detected */
  __HAL_UART_CLEAR_FLAG(pHdle, UART_CLEAR_IDLEF);
  SET_BIT(pHdle->Instance->CR1, USART_CR1_IDLEIE);
  return (int32_t)HAL_UART_Receive_DMA(pHdle, pBuffer, (uint16_t)szBuffer);
}


static int32_t s_startDmaRx(UART_HandleTypeDef *pHdle, uint32_t size)
{
  int32_t error;

  if (size <= UTIL_UART_RX_BUFFER_SIZE)
  {
    HAL_StatusTypeDef status;
    do
    {
      status = HAL_UART_Receive_DMA(pHdle, pHdle->pRxBuffPtr, (uint16_t)size);
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



static int32_t s_initDmaTxNodes(UTIL_UART_t *const pConf)
{
  DMA_NodeConfTypeDef pNodeConfig;
  uint32_t   ret = 0;
  memset(&pNodeConfig, 0, sizeof(pNodeConfig));
  memset(&hTxtDmaQ, 0, sizeof(hTxtDmaQ));
  memset(&hTxDmaNode, 0, sizeof(hTxDmaNode));
  pNodeConfig.NodeType                         = DMA_GPDMA_LINEAR_NODE;
  pNodeConfig.Init.Request                     = UTIL_UART_DMA_CHANNEL_TX;
  pNodeConfig.Init.BlkHWRequest                = DMA_BREQ_SINGLE_BURST;
  pNodeConfig.Init.Direction                   = DMA_MEMORY_TO_PERIPH;
  pNodeConfig.Init.SrcInc                      = DMA_SINC_INCREMENTED;
  pNodeConfig.Init.DestInc                     = DMA_DINC_FIXED;
  pNodeConfig.Init.SrcDataWidth                = DMA_SRC_DATAWIDTH_BYTE;
  pNodeConfig.Init.DestDataWidth               = DMA_DEST_DATAWIDTH_BYTE;
  pNodeConfig.Init.SrcBurstLength              = 1;
  pNodeConfig.Init.DestBurstLength             = 1;
  pNodeConfig.Init.TransferAllocatedPort       = DMA_SRC_ALLOCATED_PORT0 | DMA_DEST_ALLOCATED_PORT0;
  pNodeConfig.Init.TransferEventMode           = DMA_TCEM_LAST_LL_ITEM_TRANSFER;
  pNodeConfig.TriggerConfig.TriggerPolarity    = DMA_TRIG_POLARITY_MASKED;
  pNodeConfig.DataHandlingConfig.DataExchange  = DMA_EXCHANGE_NONE;
  pNodeConfig.DataHandlingConfig.DataAlignment = DMA_DATA_RIGHTALIGN_ZEROPADDED;
  ret |= (uint32_t)HAL_DMAEx_List_BuildNode(&pNodeConfig, &hTxDmaNode);
  ret |= (uint32_t)HAL_DMAEx_List_InsertNode_Tail(&hTxtDmaQ, &hTxDmaNode);
  return (int32_t)ret;
}


static int32_t s_startDmaTx(UART_HandleTypeDef *pHdle, uint32_t size)
{
  int32_t error;
  int32_t count = 100;
  HAL_NVIC_EnableIRQ(UTIL_UART_DMA_STREAM_TX_IRQn);

  if (size <= UTIL_UART_TX_BUFFER_SIZE)
  {
    HAL_StatusTypeDef status;
    do
    {
      status = HAL_UART_Transmit_DMA(pHdle, pHdle->pTxBuffPtr, (uint16_t)size);
      count--;
    }
    while ((status == HAL_BUSY) || (count < 0));
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
  error = (int32_t)HAL_UART_Abort(pHdle);
  if (error == UTIL_ERROR_NONE)
  {
    HAL_NVIC_DisableIRQ(UTIL_UART_DMA_STREAM_RX_IRQn);
  }
  #endif
  return error;
}


static int32_t s_stopDmaTx(UART_HandleTypeDef *pHdle)
{
  int32_t error = UTIL_ERROR_NONE;
  #ifdef UTIL_UART_DMA_STREAM_TX_IRQHandler
  error = (int32_t)HAL_UART_Abort(pHdle);
  if (error == UTIL_ERROR_NONE)
  {
    HAL_NVIC_DisableIRQ(UTIL_UART_DMA_STREAM_TX_IRQn);
  }
  #endif
  return error;
}

/* for the RX we need to build a circular buffer */

static int32_t s_initDmaRxNodes(UTIL_UART_t *const pConf)
{
  DMA_NodeConfTypeDef pNodeConfig;
  uint32_t   ret = 0;
  memset(&pNodeConfig, 0, sizeof(pNodeConfig));
  memset(&hRxtDmaQ, 0, sizeof(hRxtDmaQ));
  memset(&hRxDmaNode, 0, sizeof(hRxDmaNode));
  pNodeConfig.NodeType                         = DMA_GPDMA_LINEAR_NODE;
  pNodeConfig.Init.Request                     = UTIL_UART_DMA_CHANNEL_RX;
  pNodeConfig.Init.BlkHWRequest                = DMA_BREQ_SINGLE_BURST;
  pNodeConfig.Init.Direction                   = DMA_PERIPH_TO_MEMORY;
  pNodeConfig.Init.SrcInc                      = DMA_SINC_FIXED;
  pNodeConfig.Init.DestInc                     = DMA_DINC_INCREMENTED;
  pNodeConfig.Init.SrcDataWidth                = DMA_SRC_DATAWIDTH_BYTE;
  pNodeConfig.Init.DestDataWidth               = DMA_DEST_DATAWIDTH_BYTE;
  pNodeConfig.Init.SrcBurstLength              = 1;
  pNodeConfig.Init.DestBurstLength             = 1;
  pNodeConfig.Init.TransferAllocatedPort       = DMA_SRC_ALLOCATED_PORT0 | DMA_DEST_ALLOCATED_PORT0;
  pNodeConfig.Init.TransferEventMode           = DMA_TCEM_BLOCK_TRANSFER;
  pNodeConfig.TriggerConfig.TriggerPolarity    = DMA_TRIG_POLARITY_MASKED;
  pNodeConfig.DataHandlingConfig.DataExchange  = DMA_EXCHANGE_NONE;
  pNodeConfig.DataHandlingConfig.DataAlignment = DMA_DATA_RIGHTALIGN_ZEROPADDED;
  ret |= (uint32_t)HAL_DMAEx_List_BuildNode(&pNodeConfig, &hRxDmaNode);
  ret |= (uint32_t)HAL_DMAEx_List_InsertNode_Tail(&hRxtDmaQ, &hRxDmaNode);
  ret |= (uint32_t)HAL_DMAEx_List_SetCircularMode(&hRxtDmaQ);
  return (int32_t)ret;
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
  /* Enable DMA IT*/
  HAL_NVIC_SetPriority(UTIL_UART_DMA_STREAM_RX_IRQn, UTIL_UART_DMA_RX_PRIO, 0);
  HAL_NVIC_EnableIRQ(UTIL_UART_DMA_STREAM_RX_IRQn);
  if ((pConf->dma == UTIL_UART_DMA_RX) || ((pConf->dma == UTIL_UART_DMA_RX_TX)))
  {
    DMA_NodeConfTypeDef pNodeConfig;
    memset(&pNodeConfig, 0, sizeof(pNodeConfig));
    /* create a node in circular mode */

    hRxDma.Instance                         = UTIL_UART_DMA_STREAM_RX;
    hRxDma.InitLinkedList.Priority          = DMA_LOW_PRIORITY_HIGH_WEIGHT;
    hRxDma.InitLinkedList.LinkStepMode      = DMA_LSM_FULL_EXECUTION;
    hRxDma.InitLinkedList.LinkAllocatedPort = DMA_LINK_ALLOCATED_PORT0;
    hRxDma.InitLinkedList.TransferEventMode = DMA_TCEM_LAST_LL_ITEM_TRANSFER;
    hRxDma.InitLinkedList.LinkedListMode    = DMA_LINKEDLIST_CIRCULAR;

    if (HAL_DMAEx_List_Init(&hRxDma) != HAL_OK)
    {
      return UTIL_ERROR;
    }
    if (HAL_DMA_ConfigChannelAttributes(&hRxDma, DMA_CHANNEL_NPRIV) != HAL_OK)
    {
      return UTIL_ERROR;
    }
    ret = s_initDmaRxNodes(pConf);

    /* Link UART queue to DMA channel */
    HAL_DMAEx_List_LinkQ(&hRxDma, &hRxtDmaQ);

    /* Associate the initialized GPDMA handle to the UART handle */
    UTIL_UART_DMA_STREAM_RX_IRQ_cycleMeasure_Init();
    UTIL_UART_DMA_STREAM_RX_IRQ_cycleMeasure_Reset();
    __HAL_LINKDMA(pConf->pHdle, hdmarx, hRxDma);
  }
  return ret;
}


static void s_XferErrorCallback(struct __DMA_HandleTypeDef *hdma)
{
  while (1);
}

/**
  * @brief GPDMA1 Initialization Function
  * @param None
  * @retval None
  */
static int32_t s_initDmaTx(UTIL_UART_t *const pConf)
{
  int32_t error = 0;

  /* Peripheral clock enable */
  __HAL_RCC_GPDMA1_CLK_ENABLE();

  /* GPDMA1 interrupt Init */

  HAL_NVIC_SetPriority(UTIL_UART_DMA_STREAM_TX_IRQn, 0, 0); // should be UTIL_UART_DMA_TX_PRIO !!!
  HAL_NVIC_EnableIRQ(UTIL_UART_DMA_STREAM_TX_IRQn);

  memset(&hTxDma, 0, sizeof(hTxDma));

  hTxDma.Instance                         = UTIL_UART_DMA_STREAM_TX;
  hTxDma.InitLinkedList.Priority          = DMA_LOW_PRIORITY_LOW_WEIGHT;
  hTxDma.InitLinkedList.LinkStepMode      = DMA_LSM_FULL_EXECUTION;
  hTxDma.InitLinkedList.LinkAllocatedPort = DMA_LINK_ALLOCATED_PORT1;
  hTxDma.InitLinkedList.TransferEventMode = DMA_TCEM_LAST_LL_ITEM_TRANSFER;

  hTxDma.XferErrorCallback                = s_XferErrorCallback;
  if (pConf->dma_mode_tx == 0U)
  {
    hTxDma.InitLinkedList.LinkedListMode = DMA_LINKEDLIST_NORMAL;
  }
  else
  {
    hTxDma.InitLinkedList.LinkedListMode = DMA_LINKEDLIST_CIRCULAR;
  }
  if (HAL_DMAEx_List_Init(&hTxDma) != HAL_OK)
  {
    return UTIL_ERROR;
  }
  if (HAL_DMA_ConfigChannelAttributes(&hTxDma, DMA_CHANNEL_NPRIV) != HAL_OK)
  {
    return UTIL_ERROR;
  }
  error = s_initDmaTxNodes(pConf);
  /* Link UART queue to DMA channel */
  HAL_DMAEx_List_LinkQ(&hTxDma, &hTxtDmaQ);
  /* Associate the initialized GPDMA handle to the UART handle */
  UTIL_UART_DMA_STREAM_TX_IRQ_cycleMeasure_Init();
  UTIL_UART_DMA_STREAM_TX_IRQ_cycleMeasure_Reset();
  __HAL_LINKDMA(pConf->pHdle, hdmatx, hTxDma);

  return error;
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
  /* manage the IDLE detection statet while we use a DMA circular mode, this implementation doesn't exist in HAL */

  uint32_t isrflags   = READ_REG(pUartHdle->Instance->ISR);
  uint32_t cr1its     = READ_REG(pUartHdle->Instance->CR1);

  UTIL_UART_IRQ_cycleMeasure_Start();
  if (
    ((isrflags & USART_ISR_IDLE) != 0U)
    && ((cr1its & USART_ISR_IDLE) != 0U))
  {
    /* We have an IDLE IT */
    /* clear the IT */
    __HAL_UART_CLEAR_FLAG(pUartHdle, UART_CLEAR_IDLEF);

    /* Get the DMA count position in the full buffer */
    uint32_t nb_remaining_rx_data = __HAL_DMA_GET_COUNTER(pUartHdle->hdmarx);
    if ((nb_remaining_rx_data > 0U)
        && (nb_remaining_rx_data < pUartHdle->RxXferSize) && (pUartHdle->RxEventCallback != NULL))
    {
      /* Call the listenner  passing the buffer position */
      pUartHdle->RxEventCallback(pUartHdle, pUartHdle->RxXferSize - nb_remaining_rx_data);
    }
  }
  HAL_UART_IRQHandler(pUartHdle);
  UTIL_UART_IRQ_cycleMeasure_Stop();
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
  UTIL_UART_DMA_STREAM_RX_IRQ_cycleMeasure_Start();
  HAL_DMA_IRQHandler(pUartHdle->hdmarx);
  UTIL_UART_DMA_STREAM_RX_IRQ_cycleMeasure_Stop();
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
  UTIL_UART_DMA_STREAM_TX_IRQ_cycleMeasure_Start();
  HAL_DMA_IRQHandler(pUartHdle->hdmatx);
  UTIL_UART_DMA_STREAM_TX_IRQ_cycleMeasure_Stop();
}
#endif



/**
 * @brief TIM Timeout  IRQ Handler
 *
 */
#ifdef UTIL_UART_TIMx_IRQHandler
void UTIL_UART_TIMx_IRQHandler(void);
void UTIL_UART_TIMx_IRQHandler(void)
{
  HAL_TIM_IRQHandler(&hTimTimeout);
}
#endif


