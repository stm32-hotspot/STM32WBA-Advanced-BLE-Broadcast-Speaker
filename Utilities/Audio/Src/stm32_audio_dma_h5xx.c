/**
******************************************************************************
* @file    stm32_audio_dma_h5xx.c
* @author  MCD Application Team
* @brief   Manage dma init for STM32H5xx
******************************************************************************
* @attention
*
* Copyright (c) 2018(-2022) STMicroelectronics.
* All rights reserved.
*
* This software is licensed under terms that can be found in the LICENSE file
* in the root directory of this software component.
* If no LICENSE file comes with this software, it is provided AS-IS.
*
******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include "stm32_audio_dma.h"

#ifdef UTIL_AUDIO_DMA_USED

/* Global variables ----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
#ifndef UTIL_AUDIO_DMA_INSTANCES_MAX_NB
  #define UTIL_AUDIO_DMA_INSTANCES_MAX_NB 1U
#endif

/* Private macros ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
typedef struct
{
  DMA_NodeConfTypeDef     DmaNodeConfig;
  DMA_NodeTypeDef         DmaNode;
} UnCachedContext_t;

typedef struct
{
  UTIL_AUDIO_DMA_t const *pHdle;
  DMA_HandleTypeDef       hDma;
  DMA_QListTypeDef        hDmaQueue;
} CachedContext_t;


/* Private variables ---------------------------------------------------------*/
VAR_DECLARE_ALIGN4_AT_SECTION(UnCachedContext_t, G_UnCachedContext[UTIL_AUDIO_DMA_INSTANCES_MAX_NB], UTIL_AUDIO_MEMORY_SECTION_HW_BUFFERS);  /*cstat !MISRAC2012-Rule-1.4_a extended language features needed*/
static CachedContext_t G_CachedContext[UTIL_AUDIO_DMA_INSTANCES_MAX_NB];
static uint8_t         G_ContextNb = 0U;


/* Private function prototypes -----------------------------------------------*/
/* Functions Definition ------------------------------------------------------*/

int32_t UTIL_AUDIO_DMA_Init(UTIL_AUDIO_DMA_t const *const pHdle)
{
  int32_t            error        = UTIL_AUDIO_ERROR_NONE;
  UnCachedContext_t *pUnCachedCtx = NULL;
  CachedContext_t   *pCachedCtx   = NULL;

  if (G_ContextNb == 0U)
  {
    /* first time => reset all contexts */
    memset(G_CachedContext, 0, sizeof(G_CachedContext));
  }

  if (pHdle == NULL)
  {
    /* no handle => error */
    error = UTIL_AUDIO_ERROR;
  }

  if (error == UTIL_AUDIO_ERROR_NONE)
  {
    for (uint8_t idx = 0U; idx < UTIL_AUDIO_DMA_INSTANCES_MAX_NB; idx++)
    {
      if (pHdle == G_CachedContext[idx].pHdle)
      {
        /* pHdle already used => error */
        error = UTIL_AUDIO_ERROR;
        break;
      }
      if ((pCachedCtx == NULL) && (G_CachedContext[idx].pHdle == NULL))
      {
        /* available context found but maybe pHdle already used in another context
           => do not exit loop to try to find it */
        pUnCachedCtx = &G_UnCachedContext[idx];
        pCachedCtx   = &G_CachedContext[idx];
      }
    }
  }

  if (error == UTIL_AUDIO_ERROR_NONE)
  {
    if (pCachedCtx == NULL)
    {
      /* no more space for a new dma instance */
      error = UTIL_AUDIO_ERROR;
    }
  }

  if (error == UTIL_AUDIO_ERROR_NONE)
  {
    if (pCachedCtx->hDmaQueue.Head != NULL)
    {
      /* already initialized => error */
      error = UTIL_AUDIO_ERROR;
    }
  }

  if (error == UTIL_AUDIO_ERROR_NONE)
  {
    /* it is a new context */
    pCachedCtx->pHdle = pHdle;
    G_ContextNb++;
    memset(pUnCachedCtx, 0, sizeof(*pUnCachedCtx));

    /* Set node type */
    pUnCachedCtx->DmaNodeConfig.NodeType                         = DMA_GPDMA_LINEAR_NODE;

    /* Set common node parameters */
    pUnCachedCtx->DmaNodeConfig.Init.Request                     = pHdle->request;
    pUnCachedCtx->DmaNodeConfig.Init.BlkHWRequest                = DMA_BREQ_SINGLE_BURST;
    pUnCachedCtx->DmaNodeConfig.Init.Direction                   = pHdle->direction;
    pUnCachedCtx->DmaNodeConfig.Init.SrcInc                      = DMA_SINC_FIXED;
    pUnCachedCtx->DmaNodeConfig.Init.DestInc                     = DMA_DINC_INCREMENTED;
    pUnCachedCtx->DmaNodeConfig.Init.SrcDataWidth                = pHdle->srcDataWidth;
    pUnCachedCtx->DmaNodeConfig.Init.DestDataWidth               = pHdle->destDataWidth;
    pUnCachedCtx->DmaNodeConfig.Init.SrcBurstLength              = 1UL;
    pUnCachedCtx->DmaNodeConfig.Init.DestBurstLength             = 1UL;
    pUnCachedCtx->DmaNodeConfig.Init.Priority                    = DMA_HIGH_PRIORITY;
    pUnCachedCtx->DmaNodeConfig.Init.TransferEventMode           = DMA_TCEM_BLOCK_TRANSFER;
    pUnCachedCtx->DmaNodeConfig.Init.TransferAllocatedPort       = DMA_SRC_ALLOCATED_PORT0 | DMA_DEST_ALLOCATED_PORT1;
    pUnCachedCtx->DmaNodeConfig.Init.Mode                        = DMA_NORMAL;

    /* Set node data handling parameters */
    pUnCachedCtx->DmaNodeConfig.DataHandlingConfig.DataExchange  = DMA_EXCHANGE_NONE;
    pUnCachedCtx->DmaNodeConfig.DataHandlingConfig.DataAlignment = DMA_DATA_RIGHTALIGN_ZEROPADDED;

    /* Set node trigger parameters */
    pUnCachedCtx->DmaNodeConfig.TriggerConfig.TriggerPolarity    = DMA_TRIG_POLARITY_MASKED;

    /* Build Node */
    if (HAL_DMAEx_List_BuildNode(&pUnCachedCtx->DmaNodeConfig, &pUnCachedCtx->DmaNode) != HAL_OK)
    {
      error = UTIL_AUDIO_ERROR;
    }
  }

  if (error == UTIL_AUDIO_ERROR_NONE)
  {
    /* Insert NodeTx to SAI queue */
    if (HAL_DMAEx_List_InsertNode_Tail(&pCachedCtx->hDmaQueue, &pUnCachedCtx->DmaNode) != HAL_OK)
    {
      error = UTIL_AUDIO_ERROR;
    }
  }

  if (error == UTIL_AUDIO_ERROR_NONE)
  {
    /* Set queue circular mode for sai queue */
    if (HAL_DMAEx_List_SetCircularMode(&pCachedCtx->hDmaQueue) != HAL_OK)
    {
      error = UTIL_AUDIO_ERROR;
    }
  }

  if (error == UTIL_AUDIO_ERROR_NONE)
  {
    /* DMA for Rx */
    pCachedCtx->hDma.Instance                         = (DMA_Channel_TypeDef *)pHdle->pInstance; /* pInstance wording is used since it is a common driver, on H5 family it corresponds to channel */

    pCachedCtx->hDma.InitLinkedList.Priority          = DMA_HIGH_PRIORITY;
    pCachedCtx->hDma.InitLinkedList.LinkStepMode      = DMA_LSM_FULL_EXECUTION;
    pCachedCtx->hDma.InitLinkedList.LinkAllocatedPort = DMA_LINK_ALLOCATED_PORT1;
    pCachedCtx->hDma.InitLinkedList.TransferEventMode = DMA_TCEM_LAST_LL_ITEM_TRANSFER;
    pCachedCtx->hDma.InitLinkedList.LinkedListMode    = DMA_LINKEDLIST_CIRCULAR;

    /* DMA linked list init */
    if (HAL_DMAEx_List_Init(&pCachedCtx->hDma) != HAL_OK)
    {
      error = UTIL_AUDIO_ERROR;
    }
  }

  if (error == UTIL_AUDIO_ERROR_NONE)
  {
    /* Link SAI queue to DMA channel */
    if (HAL_DMAEx_List_LinkQ(&pCachedCtx->hDma, &pCachedCtx->hDmaQueue) != HAL_OK)
    {
      error = UTIL_AUDIO_ERROR;
    }
  }

  if (error == UTIL_AUDIO_ERROR_NONE)
  {
    switch (pHdle->type)
    {
      case UTIL_AUDIO_DMA_IP_TYPE_SAI:
        /* Associate the DMA handle */
        if (pHdle->direction == DMA_PERIPH_TO_MEMORY)
        {
          SAI_HandleTypeDef *hsai = (SAI_HandleTypeDef *)pHdle->pIpHdle;
          __HAL_LINKDMA(hsai, hdmarx, pCachedCtx->hDma);
        }
        else
        {
          SAI_HandleTypeDef *hsai = (SAI_HandleTypeDef *)pHdle->pIpHdle;
          __HAL_LINKDMA(hsai, hdmatx, pCachedCtx->hDma);
        }
        break;

      default:
        error = UTIL_AUDIO_ERROR;
        break;
    }
  }

  if (error == UTIL_AUDIO_ERROR_NONE)
  {
    /* SAI DMA IRQ Channel configuration */
    HAL_NVIC_SetPriority(pHdle->irqn, pHdle->preemptPriority, 0UL);
    HAL_NVIC_EnableIRQ(pHdle->irqn);
  }

  return error;
}


int32_t UTIL_AUDIO_DMA_DeInit(UTIL_AUDIO_DMA_t const *const pHdle)
{
  int32_t          error      = UTIL_AUDIO_ERROR_NONE;
  CachedContext_t *pCachedCtx = NULL;

  if ((pHdle == NULL) || (G_ContextNb == 0U))
  {
    error = UTIL_AUDIO_ERROR;
  }

  if (error == UTIL_AUDIO_ERROR_NONE)
  {
    for (uint8_t idx = 0U; idx < UTIL_AUDIO_DMA_INSTANCES_MAX_NB; idx++)
    {
      if (pHdle == G_CachedContext[idx].pHdle)
      {
        /* pHdle found */
        pCachedCtx = &G_CachedContext[idx];
        break;
      }
    }

    if (pCachedCtx == NULL)
    {
      /* there is no dma instance associated to this pHdle */
      error = UTIL_AUDIO_ERROR;
    }
  }

  if (error == UTIL_AUDIO_ERROR_NONE)
  {
    /* Disable SAI DMA Channel IRQ */
    HAL_NVIC_DisableIRQ(pHdle->irqn);

    /* Reset the DMA Channel configuration*/
    if (HAL_DMAEx_List_DeInit(&pCachedCtx->hDma) != HAL_OK)
    {
      error = UTIL_AUDIO_ERROR;
    }

    /* Reset RxQueue */
    if (HAL_DMAEx_List_ResetQ(&pCachedCtx->hDmaQueue) != HAL_OK)
    {
      error = UTIL_AUDIO_ERROR;
    }

    /* clear context */
    memset(pCachedCtx, 0, sizeof(CachedContext_t));
    G_ContextNb--;
  }

  return error;
}

#endif /* UTIL_AUDIO_DMA_USED */
