/**
******************************************************************************
* @file    stm32_usart.c
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
#include <stdlib.h>
#include "stm32_usart.h"

/* Global variables ----------------------------------------------------------*/
/* Global define    ----------------------------------------------------------*/

#define FLASH_CHARS_TIMEOUT      100U  /* in ms  delay after the last char emitted to check if there is something to flush */
#define SEND_BLOCKS_NB_SLOTS     100U  /* nb slot for the DMA transmission, a slot can send a buffer longer than ne DMA size */
#define TIMEOUT_WAIT_ASYNC       100U  /* 10 ms max before timeout when we push a char stream */
#define UTIL_UART_TX_STREAM_SIZE 120U  /* size of text we can accumulate before to flush */


/* Private typedef -----------------------------------------------------------*/

typedef struct UTIL_UART_send_slot_t
{
  const uint8_t           *pBlock;
  volatile const uint8_t  *pCurBlock;
  volatile uint32_t        szRemaining;
  void                    *pMem2Free;
  void                    *pCookie;
  UTIL_UART_SEND_BLOCK_CB  cbFreeBlockIsr;
} UTIL_UART_send_slot_t;


typedef struct UTIL_UART_send_blocks_t
{
  int8_t                   bEnable;
  volatile int8_t          bDmaPending;
  volatile uint8_t         iReadSlot;
  volatile uint8_t         iWriteSlot;

  volatile uint8_t         szPendingSlot;
  uint8_t                  szSlotPeak;

  UTIL_UART_send_slot_t    tItems[SEND_BLOCKS_NB_SLOTS];
} UTIL_UART_send_blocks_t;

typedef struct UTIL_UART_stream_dma_t
{
  volatile uint32_t        iCurDma;
  uint8_t                  tDma[UTIL_UART_TX_STREAM_SIZE];
} UTIL_UART_stream_dma_t;


typedef struct UTIL_UART_stream_blocks_t
{
  volatile uint8_t         remainingStream;
  volatile uint8_t         posStream;
  uint8_t                  szStream;
  UTIL_UART_stream_dma_t  *pStream;
} UTIL_UART_stream_blocks_t;



/* Private defines -----------------------------------------------------------*/

static bool                             iMuteRefPrint;
static UTIL_UART_cbs_t                  G_cbs;
static UART_HandleTypeDef              *G_pHdle;
static TIM_HandleTypeDef               *G_pHTim;
static UTIL_UART_send_blocks_t          hSendBlockAsync;
static UTIL_UART_stream_blocks_t        hTransmitAsync;


/* Private function prototypes -----------------------------------------------*/
static void    s_UTIL_UART_Dma_TxCpltCallback(UART_HandleTypeDef *huart);
static void    s_UTIL_UART_Tim_PeriodElapsedCallback(TIM_HandleTypeDef *htim);
static int32_t s_UTIL_UART_TransmitAsync(UART_HandleTypeDef *pHdle, uint8_t *pData, uint32_t size);
static void    s_UTIL_UART_FillBlock_Unlocked(void);
static int32_t s_UTIL_UART_SendDmaBlock_Unlocked(const void *pBlock, uint32_t szBlock, UTIL_UART_SEND_BLOCK_CB cbFreeBlock, void *pMem2Free, void *pCookie);


/* Functions Definition ------------------------------------------------------*/

/**
  * @brief  Register BSP audio callbacks or other functions to control audio
  * @param  Handler of the callbacks
  * @retval None
  */
void UTIL_UART_registerCallbacks(UTIL_UART_cbs_t const *const pCbs)
{
  G_cbs = *pCbs;
}


/**
  * @brief  Reset audio callbacks
  * @param  Handler of the callbacks
  * @retval None
  */
void UTIL_UART_resetCallbacks(UTIL_UART_cbs_t *const pCbs)
{
  memset(pCbs, 0, sizeof(UTIL_UART_cbs_t));
}


UART_HandleTypeDef *UTIL_UART_GetHdle(void)
{
  return G_pHdle;
}


/**
* @brief  Configures the USART for vCom
* @param  None
* @retval None
*/
int32_t UTIL_UART_Init(UTIL_UART_t *const pConfig)
{
  int32_t error = UTIL_ERROR;

  iMuteRefPrint = false;
  memset(&G_cbs,           0, sizeof(G_cbs));
  memset(&hSendBlockAsync, 0, sizeof(hSendBlockAsync));
  memset(&hTransmitAsync,  0, sizeof(hTransmitAsync));

  UTIL_UART_setCallbacks(); /* MUST BE DEFINED BY USER */

  if (G_cbs.init != NULL)
  {
    error = G_cbs.init(pConfig);
  }

  G_pHdle = pConfig->pHdle;
  G_pHTim = pConfig->pHTim;

  if (error == UTIL_ERROR_NONE)
  {
    if (G_cbs.setRxBufferAddr != NULL)
    {
      G_cbs.setRxBufferAddr(G_pHdle);
    }
    if (G_cbs.initDmaRx != NULL)
    {
      error = G_cbs.initDmaRx(pConfig);
    }
  }

  if (error == UTIL_ERROR_NONE)
  {
    if (G_cbs.setTxBufferAddr != NULL)
    {
      G_cbs.setTxBufferAddr(G_pHdle);
    }
    if (G_cbs.initDmaTx != NULL)
    {
      error = G_cbs.initDmaTx(pConfig);
    }
  }

  if (error == UTIL_ERROR_NONE)
  {
    if (G_cbs.initTimeoutIT != NULL)
    {
      G_cbs.initTimeoutIT(pConfig->pHTim, FLASH_CHARS_TIMEOUT);
    }
  }
  return error;
}


/**
* @brief  Deinit the USART.
* @param  None
* @retval None
*/
int32_t UTIL_UART_DeInit(UART_HandleTypeDef const *const pHdle)
{
  int32_t error = UTIL_ERROR_NONE;
  if (G_cbs.deInit != NULL)
  {
    error = G_cbs.deInit(pHdle);
  }
  if (error == UTIL_ERROR_NONE)
  {
    UTIL_UART_Mute(UTIL_UART_MUTE_FORCE_OFF);
  }
  return error;
}


/**
* @brief  return true if the log is muted
* @retval None
*/
bool UTIL_UART_IsLogMuted(void)
{
  return iMuteRefPrint;
}


/**
* @brief  Mute the UART: tx bytes are not sent, rx byte are not taken in.
* @param  mute type
* @retval old mute state
*/
bool UTIL_UART_Mute(UTIL_UART_MUTE_t mute)
{
  bool oldRef = iMuteRefPrint;
  switch (mute)
  {
    case UTIL_UART_MUTE_FORCE_ON:
    {
      iMuteRefPrint = false;
      break;
    }
    case UTIL_UART_MUTE_FORCE_OFF:
    {
      iMuteRefPrint = true;
      break;
    }
  }
  return oldRef;
}


/**
* @brief  Check if new data are available
* @param  None
* @retval error
*/
int32_t UTIL_UART_StartRxDma(uint32_t size)
{
  int32_t error = UTIL_ERROR_NONE;

  if (G_cbs.startDmaRx != NULL)
  {
    error = G_cbs.startDmaRx(G_pHdle, size);
  }
  return error;
}


/**
* @brief  Start a DMA RX using idle detection
* @param  pBuffer dma buffer
* @param  szBufffer size dma buffer
* @retval error
*/
int32_t  UTIL_UART_StartRxDmaToIdle(void *pBuffer, uint32_t szBuffer)
{
  int32_t error = UTIL_ERROR_NONE;

  if (G_cbs.startRxDmaToIdle != NULL)
  {
    error = G_cbs.startRxDmaToIdle(G_pHdle, pBuffer, szBuffer);
  }
  return error;
}


/**
* @brief  Check if new data are available
* @param  None
* @retval error
*/
int32_t UTIL_UART_StartTxDma(uint32_t size)
{
  int32_t error = UTIL_ERROR_NONE;
  if (G_cbs.startDmaTx != NULL)
  {
    error = G_cbs.startDmaTx(G_pHdle, size);
  }
  return error;
}


/**
* @brief  Get DMA RX counter
* @param  None
* @retval DMA RX counter
*/
uint32_t UTIL_UART_GetRxDmaCounter(void)
{
  return __HAL_DMA_GET_COUNTER(G_pHdle->hdmarx);
}


/**
* @brief  Get DMA TX counter
* @param  None
* @retval DMA TX counter
*/
uint32_t UTIL_UART_GetTxDmaCounter(void)
{
  return __HAL_DMA_GET_COUNTER(G_pHdle->hdmatx);
}


/**
* @brief  Block DMA for RX
* @param  None
* @retval None
*/
int32_t UTIL_UART_BlockRxDma(void)
{
  int32_t error = UTIL_ERROR;
  if (G_cbs.stopDmaRx != NULL)
  {
    error = G_cbs.stopDmaRx(G_pHdle);
  }
  return error;
}


/**
* @brief  Block DMA for TX
* @param  None
* @retval error
*/
int32_t UTIL_UART_BlockTxDma(void)
{
  int32_t error = UTIL_ERROR;
  if (G_cbs.stopDmaTx != NULL)
  {
    error = G_cbs.stopDmaTx(G_pHdle);
  }
  return error;
}


/**
* @brief  Send data via USART
* @param  msg_size: size of the msg to be sent
* @retval error
*/
int32_t UTIL_UART_SendTxData(uint32_t msg_size)
{
  int32_t error = UTIL_ERROR;
  if (msg_size <= (uint32_t)G_pHdle->TxXferSize)
  {
    G_pHdle->TxXferSize  = (uint16_t)msg_size;
    G_pHdle->TxXferCount = (uint16_t)msg_size;

    if (G_cbs.transmit != NULL)
    {
      error = G_cbs.transmit(G_pHdle, (uint8_t *)G_pHdle->pTxBuffPtr, msg_size);
    }
  }
  return error;
}


/**
* @brief  Send character via USART if UTIL_UART was not muted thru UTIL_UART_Mute
* @param  character to print
* @retval error
*/
int32_t UTIL_UART_fputc(int32_t data)
{
  int32_t error = UTIL_ERROR_NONE;
  if (UTIL_UART_IsLogMuted() == false)
  {
    error = UTIL_UART_fputc_force(data);
  }
  return error;
}


/**
* @brief  Send character via USART
* @param  character to print
* @retval error
*/
int32_t UTIL_UART_fputc_force(int32_t data)
{
  int32_t error = UTIL_ERROR;
  if (G_cbs.transmit != NULL)
  {
    error = G_cbs.transmit(G_pHdle, (uint8_t *)&data, 1);
  }
  return error;
}


/**
* @brief  Send string via USART if UTIL_UART was not muted thru UTIL_UART_Mute
* @param  string to print
* @retval error
*/
int32_t UTIL_UART_fputs(char *pData)
{
  int32_t error = UTIL_ERROR_NONE;
  if (UTIL_UART_IsLogMuted() == false)
  {
    error = UTIL_UART_fputs_force(pData);
  }
  return error;
}


/**
* @brief  Send text data via USART
* @param  text to print
* @retval error
*/
int32_t UTIL_UART_fputs_force(char *pData)
{
  int32_t error = UTIL_ERROR;
  if (G_cbs.transmit != NULL)
  {
    error = G_cbs.transmit(G_pHdle, (uint8_t *)pData, strlen(pData));
  }
  return error;
}



/**
* @brief  Send  data via USART
* @param  pointer data
* @param  pointer data size
* @retval error
*/
int32_t UTIL_UART_write(unsigned char *pData, uint32_t szData)
{
  int32_t error = UTIL_ERROR;
  if (G_cbs.transmit != NULL)
  {
    error = G_cbs.transmit(G_pHdle, (uint8_t *)pData, szData);
  }
  return error;
}


/**
* @brief  Receive character via USART if UTIL_UART was not muted thru UTIL_UART_Mute
* @param  character to print
* @retval error
*/
int32_t UTIL_UART_Receive(int32_t *pData)
{
  int32_t error = UTIL_ERROR_NONE;
  if (UTIL_UART_IsLogMuted() == false)
  {
    error = UTIL_ERROR;
    if (G_cbs.receive != NULL)
    {
      *pData = 0;
      error = G_cbs.receive(G_pHdle, (uint8_t *)pData, 1);
    }
  }
  return error;
}


/**
* @brief  Return the USART Rx buffer
* @param  None
* @retval USART Rx buffer
*/
uint8_t *UTIL_UART_GetRxBuffer(void)
{
  return G_pHdle->pRxBuffPtr;
}


/**
* @brief  Return the USART Tx buffer
* @param  None
* @retval USART Tx buffer
*/
uint8_t *UTIL_UART_GetTxBuffer(void)
{
  return (uint8_t *)G_pHdle->pTxBuffPtr;
}


/**
* @brief  return true if IT are enabled  ie we can use the DMA
*/

static bool s_UTIL_UART_Is_It_Enabled(void)
{
  /* check if we are in __disable_irq or from the ISR that disable isr by default
    On entry to an exception (interrupt):
      * interrupt requests (IRQs) are disabled for all exceptions
      * fast interrupt requests (FIQs) are disabled for FIQ and Reset exceptions.
  */
  // int a = __get_PRIMASK() & 1 ;

  if ((__get_IPSR() != 0U))
  {
    /* irq are locked */
    return true;
  }
  return false;
}


/**
* @brief  Start the dma transfer, may be called from an IT
*/
static void s_UTIL_UART_FillBlock_Unlocked()
{
  uint8_t *pDmaBuffer = (uint8_t *)G_pHdle->pTxBuffPtr;
  if (pDmaBuffer)
  {
    UTIL_UART_send_slot_t *pBlk = &hSendBlockAsync.tItems[hSendBlockAsync.iReadSlot];
    /* compute the size to send */
    uint32_t szToSend = pBlk->szRemaining;
    if (szToSend > UTIL_UART_TX_STREAM_SIZE)
    {
      szToSend = UTIL_UART_TX_STREAM_SIZE;
    }
    memcpy((void *)pDmaBuffer, (void *)pBlk->pCurBlock, szToSend);
    pBlk->pCurBlock   += szToSend;
    pBlk->szRemaining -= szToSend;
    hSendBlockAsync.bDmaPending = TRUE;
    /* szToSend == 0 is not allowed and UTIL_UART_StartTxDma should always start with no error because paced by end of transmission IT */
    int32_t error = UTIL_UART_StartTxDma(szToSend);
    if ((error != 0) || (szToSend == 0U))
    {
      while (1);
    }
  }
}



/**
* @brief  Enable  message log DMA asynchronous
* @param  bState  true or false
*/

int32_t UTIL_UART_EnableLogAsync(uint8_t bState, uint8_t szStream)
{
  int32_t error = UTIL_ERROR_NONE;

  if ((G_cbs.initTimeoutIT != NULL)  && (G_cbs.setTxBufferAddr != NULL))
  {
    UTIL_UART_stream_dma_t *pStream2Free, *pNewStream;

    memset(&hSendBlockAsync, 0, sizeof(hSendBlockAsync));
    memset(&hTransmitAsync,  0, sizeof(hTransmitAsync));

    if (bState)
    {
      if (hTransmitAsync.szStream == szStream)
      {
        /* no szStream change => keep the same pStream */
        pStream2Free = NULL;
        pNewStream   = hTransmitAsync.pStream;
      }
      else
      {
        pStream2Free = hTransmitAsync.pStream;
        pNewStream   = (UTIL_UART_stream_dma_t *)malloc((uint32_t)szStream * sizeof(UTIL_UART_stream_dma_t));/*cstat !MISRAC2012-Dir-4.13_b the pointer will be freed when the module will be closed */
        if (pNewStream == NULL)
        {
          error = UTIL_ERROR_ALLOC;
        }
      }
      UTIL_UART_Lock();
      hSendBlockAsync.bEnable        = TRUE;
      G_pHdle->TxCpltCallback        = s_UTIL_UART_Dma_TxCpltCallback;
      G_pHTim->PeriodElapsedCallback = s_UTIL_UART_Tim_PeriodElapsedCallback;
      G_cbs.transmit                 = s_UTIL_UART_TransmitAsync;
    }
    else
    {
      pStream2Free = hTransmitAsync.pStream;
      pNewStream   = NULL;
      UTIL_UART_Lock();
      hSendBlockAsync.bEnable        = FALSE;
      G_pHdle->TxCpltCallback        = NULL;
      G_pHTim->PeriodElapsedCallback = NULL;
    }
    hTransmitAsync.pStream = pNewStream;
    if (pNewStream)
    {
      memset(pNewStream, 0, (uint32_t)szStream * sizeof(UTIL_UART_stream_dma_t));
      hTransmitAsync.szStream = szStream;
    }
    else
    {
      hTransmitAsync.szStream = 0;
    }
    UTIL_UART_UnLock();
    if (pStream2Free)
    {
      free(pStream2Free);/*cstat !MISRAC2012-Dir-4.13_g the pointer is allocated at the module init */
    }
  }
  return error;
}



/**
* @brief  If all buffer Q are full, wait for the DMA flush
*/

bool UTIL_UART_BlockAsyncBusy(void)
{
  bool bResult = false;
  uint32_t timeout = TIMEOUT_WAIT_ASYNC;
  if (hSendBlockAsync.szPendingSlot >= SEND_BLOCKS_NB_SLOTS)
  {
    while (timeout)
    {
      if (hSendBlockAsync.szPendingSlot < SEND_BLOCKS_NB_SLOTS)
      {
        break;
      }
      HAL_Delay(1);
      timeout--;
    }
    if (timeout == 0U)
    {
      bResult = true;
    }
  }
  return bResult;
}


__weak void UTIL_UART_IRQ_cycleMeasure_Init(void)
{
}


__weak void UTIL_UART_IRQ_cycleMeasure_Reset(void)
{
}


__weak void UTIL_UART_IRQ_cycleMeasure_Start(void)
{
}


__weak void UTIL_UART_IRQ_cycleMeasure_Stop(void)
{
}


__weak void UTIL_UART_DMA_STREAM_RX_IRQ_cycleMeasure_Init(void)
{
}


__weak void UTIL_UART_DMA_STREAM_RX_IRQ_cycleMeasure_Reset(void)
{
}


__weak void UTIL_UART_DMA_STREAM_RX_IRQ_cycleMeasure_Start(void)
{
}


__weak void UTIL_UART_DMA_STREAM_RX_IRQ_cycleMeasure_Stop(void)
{
}


__weak void UTIL_UART_DMA_STREAM_TX_IRQ_cycleMeasure_Init(void)
{
}


__weak void UTIL_UART_DMA_STREAM_TX_IRQ_cycleMeasure_Reset(void)
{
}


__weak void UTIL_UART_DMA_STREAM_TX_IRQ_cycleMeasure_Start(void)
{
}


__weak void UTIL_UART_DMA_STREAM_TX_IRQ_cycleMeasure_Stop(void)
{
}


/**
* @brief  Send a block using DMA
* @param  pBlock   buffer to send
* @param  szBlock  buffer size
* @param  cbFreeBlock callback to free the buffer or NULL
* @param  pMem2Free mem pointer to free passed by the callback
* @retval error code
*/

static int32_t s_UTIL_UART_SendDmaBlock_Unlocked(const void *pBlock, uint32_t szBlock, UTIL_UART_SEND_BLOCK_CB cbFreeBlock, void *pMem2Free, void *pCookie)
{
  int32_t error = UTIL_ERROR;
  if (hSendBlockAsync.bEnable)
  {
    /* check if the queue is not full */
    if (hSendBlockAsync.szPendingSlot < SEND_BLOCKS_NB_SLOTS)
    {
      /* record the new entry in the queue */
      UTIL_UART_send_slot_t *pSlot = &hSendBlockAsync.tItems[hSendBlockAsync.iWriteSlot];
      memset(pSlot, 0, sizeof(UTIL_UART_send_slot_t));
      pSlot->pBlock         = pBlock;
      pSlot->pCurBlock      = pBlock;
      pSlot->pCookie        = pCookie;
      pSlot->szRemaining    = szBlock;
      pSlot->cbFreeBlockIsr = cbFreeBlock;
      pSlot->pMem2Free      = pMem2Free;
      hSendBlockAsync.iWriteSlot++;
      if (hSendBlockAsync.iWriteSlot == SEND_BLOCKS_NB_SLOTS)
      {
        hSendBlockAsync.iWriteSlot = 0;
      }
      hSendBlockAsync.szPendingSlot++;
      if (hSendBlockAsync.szPendingSlot > hSendBlockAsync.szSlotPeak)
      {
        hSendBlockAsync.szSlotPeak = hSendBlockAsync.szPendingSlot;
      }
      /* check if a dma transfer is already started,
        in this case the current transfer will start after the current one */
      if (hSendBlockAsync.bDmaPending == FALSE)
      {
        s_UTIL_UART_FillBlock_Unlocked();
      }
      error = UTIL_ERROR_NONE;
    }
    else
    {
      /* the queue is full, this should never be true*/
      //      while (1);
    }
  }
  return error;
}

/**
* @brief  Send a block using DMA
* @param  pBlock   buffer to send
* @param  szBlock  buffer size
* @param  cbFreeBlock callback to free the buffer or NULL
* @param  pMem2Free mem pointer to free passed by the callback
* @retval error code
*/

int32_t UTIL_UART_SendBlockAsync(const void *pBlock, uint32_t szBlock, UTIL_UART_SEND_BLOCK_CB cbFreeBlock, void *pMem2Free, void *pCookie)
{
  UTIL_UART_Lock();
  int32_t error = s_UTIL_UART_SendDmaBlock_Unlocked(pBlock, szBlock, cbFreeBlock, pMem2Free, pCookie);
  UTIL_UART_UnLock();
  return error;
}


/**
* @brief  load the dma a proceed the next block, called from ISR
*/

static void s_UTIL_UART_BlockNext_Unlocked(void)
{
  UTIL_UART_send_slot_t *pBlk = &hSendBlockAsync.tItems[hSendBlockAsync.iReadSlot];
  if (pBlk->szRemaining)
  {
    /* some data remaining */
    s_UTIL_UART_FillBlock_Unlocked();
  }
  else
  {
    /* free the memory if it is mandatory*/
    if (pBlk->cbFreeBlockIsr)
    {
      pBlk->cbFreeBlockIsr(pBlk->pMem2Free, pBlk->pCookie);
    }
    /* the slot is consumed completely */
    pBlk->pBlock    = NULL;
    pBlk->pCurBlock = NULL;
    if (hSendBlockAsync.szPendingSlot)
    {
      hSendBlockAsync.szPendingSlot--;
      /* next slot */
      hSendBlockAsync.iReadSlot++;
      if (hSendBlockAsync.iReadSlot == SEND_BLOCKS_NB_SLOTS)
      {
        hSendBlockAsync.iReadSlot = 0;
      }
      /* Check if another slot is pending */
      if (hSendBlockAsync.szPendingSlot)
      {
        s_UTIL_UART_FillBlock_Unlocked();
      }
      else
      {
        /* the queue is empty, we stop ... */
        hSendBlockAsync.bDmaPending = FALSE;
      }
    }
  }
}


/**
* @brief  If all buffer are full, wait for the DMA flush of at least 1 pending block
*/

static bool s_UTIL_UART_Wait_Queue_Busy(void)
{
  bool bResult = false;
  uint32_t timeout = TIMEOUT_WAIT_ASYNC;
  if (hTransmitAsync.remainingStream >= hTransmitAsync.szStream)
  {
    /* if called from ISR we cannot wait for a timeout, it are locked and the DMA is locked too */
    if (s_UTIL_UART_Is_It_Enabled())
    {
      bResult = true;
    }
    else
    {
      while (timeout)
      {
        if (hTransmitAsync.remainingStream < hTransmitAsync.szStream)
        {
          break;
        }
        HAL_Delay(1);
        timeout--;
      }
      if (timeout == 0U)
      {
        bResult = true;
      }
    }
  }
  return bResult;
}


/**
* @brief  Free a stream buffer already processed by the DMA
*/

static void s_UTIL_UART_freeAsyncBuffer(void *pBuffer, void *pCookie)
{
  UTIL_UART_stream_dma_t *pStream = (UTIL_UART_stream_dma_t *)pBuffer;
  pStream->iCurDma = 0;
  hTransmitAsync.remainingStream--;
}

/**
* @brief  Flush pending chars in the accumulation buffer
*/
static void s_UTIL_UART_Flush_Stream_Unlocked(void)
{
  UTIL_UART_stream_dma_t *pDma = &hTransmitAsync.pStream[hTransmitAsync.posStream];

  /* if there are some chars in the stream and a room in the DMA Q, pushes immediately */
  if ((pDma != NULL) && (pDma->iCurDma != 0U) && (hTransmitAsync.remainingStream < hTransmitAsync.szStream))/*cstat !MISRAC2012-Rule-13.5  false positive */
  {
    if (s_UTIL_UART_SendDmaBlock_Unlocked(pDma->tDma, pDma->iCurDma, s_UTIL_UART_freeAsyncBuffer, pDma, NULL) == UTIL_ERROR_NONE)
    {
      hTransmitAsync.remainingStream++;
      hTransmitAsync.posStream++;
      if (hTransmitAsync.posStream == hTransmitAsync.szStream)
      {
        hTransmitAsync.posStream = 0;
      }
    }
    else
    {
      /* re-prog the timeout due to the error*/
      G_cbs.setTimeoutIT(G_pHTim, TRUE);
    }
  }
}


/**
* @brief  Flush chars pending in the stream buffer
* @retval error code
*/

void UTIL_UART_FlushAsyncChars()
{
  UTIL_UART_Lock();
  s_UTIL_UART_Flush_Stream_Unlocked();
  UTIL_UART_UnLock();
}


/**
* @brief  Send a block using DMA
* @param  pBlock   buffer to send
* @param  szBlock  buffer size
* @param  cbFreeBlock callback to free the buffer or NULL
* @param  pMem2Free mem pointer to free passed by the callback
* @retval error code
*/

static bool s_UTIL_UART_Try_SendDmaBlock(const void *pBlock, uint32_t szBlock, UTIL_UART_SEND_BLOCK_CB cbFreeBlock, void *pMem2Free, void *pCookie)
{
  bool bResult = true;
  uint32_t timeout = TIMEOUT_WAIT_ASYNC;
  while (timeout != 0U)
  {
    UTIL_UART_Lock();
    int32_t error = s_UTIL_UART_SendDmaBlock_Unlocked(pBlock, szBlock, cbFreeBlock, pMem2Free, pCookie);
    UTIL_UART_UnLock();
    if (error == UTIL_ERROR_NONE)
    {
      /* the packet is sent */
      break;
    }
    if (s_UTIL_UART_Is_It_Enabled())
    {
      /*
      if IT are disabled, the DMA is locked, no chance to find free space in the queue
      so, we drop the packet
      */
      bResult = false;
      break;
    }
    HAL_Delay(1); /* wait by pooling, and not osDelay because we can be in ISR */
    timeout--;
  }
  if (timeout == 0U)
  {
    /* it is a bug, The dma is probably stopped for any reason*/
    bResult = false;
  }
  return bResult;
}



/**
* @brief  Adds a string to the asynchronous buffer stream, and send it if it is full or arm a timer to flush the buffer after 100ms of no activity
*/
static int32_t s_UTIL_UART_TransmitAsync(UART_HandleTypeDef *pHdle, uint8_t *pData, uint32_t size)
{
  int32_t error = UTIL_ERROR_NONE;
  G_cbs.setTimeoutIT(G_pHTim, FALSE);
  if (size)
  {
    while ((size != 0U) && (error == UTIL_ERROR_NONE))
    {
      if (s_UTIL_UART_Wait_Queue_Busy())
      {
        /* the packet is dropped, probably because the print comes from an src and we cannot wait in interrupt */
        break;
      }
      /* prevent pending IT timer */
      UTIL_UART_Lock();
      /*  compute the current buffer*/
      UTIL_UART_stream_dma_t *pDma = &hTransmitAsync.pStream[hTransmitAsync.posStream];

      /*  compute the size buffer to send */
      uint32_t szSend = size;
      if ((pDma->iCurDma + szSend) > UTIL_UART_TX_STREAM_SIZE)
      {
        szSend = UTIL_UART_TX_STREAM_SIZE - pDma->iCurDma;
      }
      /*  Add to the stream */
      memcpy(&pDma->tDma[pDma->iCurDma], pData, szSend);
      /* update the buffer */
      size          -= szSend;
      pData         += szSend;
      pDma->iCurDma += szSend;
      UTIL_UART_UnLock();

      /* if the buffer is full, send it to the async queue */

      if (pDma->iCurDma == UTIL_UART_TX_STREAM_SIZE)
      {
        if (s_UTIL_UART_Try_SendDmaBlock(pDma->tDma, pDma->iCurDma, s_UTIL_UART_freeAsyncBuffer, pDma, NULL) == false)
        {
          /* the packet is dropped, probably because the print comes from interrupt and we cannot wait in interrupt ( no dma It)*/
          error = UTIL_ERROR;
        }
        /* prevent pending IT timer */
        UTIL_UART_Lock();
        /* the block is sent, free  the stream */
        hTransmitAsync.remainingStream++;
        hTransmitAsync.posStream++;
        if (hTransmitAsync.posStream == hTransmitAsync.szStream)
        {
          hTransmitAsync.posStream = 0;
        }
        UTIL_UART_UnLock();
      }
    }
  }
  G_cbs.setTimeoutIT(G_pHTim, TRUE);
  return error;
}


/**
* @brief  Callback DMA & TIM IT
*/
static void s_UTIL_UART_Tim_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* stop the It and check if the dma is stopped and there are some chars in the buffer, in this case we flush them */
  G_cbs.setTimeoutIT(G_pHTim, FALSE);
  //UTIL_UART_Lock();
  s_UTIL_UART_Flush_Stream_Unlocked();
  //UTIL_UART_UnLock();
}


/**
* @brief  Chain the DMA and send pending blocks
*/
static void s_UTIL_UART_Dma_TxCpltCallback(UART_HandleTypeDef *huart)
{
  /* no need to lock, we are in ISR */
  //UTIL_UART_Lock();
  s_UTIL_UART_BlockNext_Unlocked();
  //UTIL_UART_UnLock();
}
