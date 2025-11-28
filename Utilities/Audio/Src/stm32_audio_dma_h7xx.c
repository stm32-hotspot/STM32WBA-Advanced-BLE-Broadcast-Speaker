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
  UTIL_AUDIO_DMA_t const *pHdle;
  DMA_HandleTypeDef       hDma;
} Context_t;


/* Private variables ---------------------------------------------------------*/
static Context_t G_Context[UTIL_AUDIO_DMA_INSTANCES_MAX_NB];
static uint8_t   G_ContextNb = 0U;


/* Private function prototypes -----------------------------------------------*/
/* Functions Definition ------------------------------------------------------*/

int32_t UTIL_AUDIO_DMA_Init(UTIL_AUDIO_DMA_t const *const pHdle)
{
  int32_t error = UTIL_AUDIO_ERROR_NONE;

  if (pHdle == NULL)
  {
    error = UTIL_AUDIO_ERROR;
  }
  else
  {
    Context_t *pCtx = NULL;

    if (G_ContextNb == 0U)
    {
      /* first time => reset all contexts */
      memset(G_Context, 0, sizeof(G_Context));
    }

    for (uint8_t idx = 0U; idx < UTIL_AUDIO_DMA_INSTANCES_MAX_NB; idx++)
    {
      if (pHdle == G_Context[idx].pHdle)
      {
        /* pHdle already initialized
           => re-use its dma instance
           => exit loop */
        pCtx = &G_Context[idx];
        break;
      }
      if (G_Context[idx].pHdle == NULL)
      {
        /* available context found but maybe pHdle already in another context
           => do not exit loop to try to find it */
        pCtx = &G_Context[idx];
      }
    }

    if (pCtx == NULL)
    {
      /* no more space for a new dma instance */
      error = UTIL_AUDIO_ERROR;
    }
    else
    {
      if (pCtx->pHdle == NULL)
      {
        /* it is a new dma init, new element in context table */
        pCtx->pHdle = pHdle;
        G_ContextNb++;
      }

      /* Configure the pCtx->hDma handle parameters */
      pCtx->hDma.Init.Request               = pHdle->request;
      pCtx->hDma.Init.Direction             = pHdle->direction;
      pCtx->hDma.Init.PeriphInc             = DMA_PINC_DISABLE;
      pCtx->hDma.Init.MemInc                = DMA_MINC_ENABLE;

      if (pHdle->direction == DMA_PERIPH_TO_MEMORY)
      {
        pCtx->hDma.Init.PeriphDataAlignment = pHdle->srcDataWidth;
        pCtx->hDma.Init.MemDataAlignment    = pHdle->destDataWidth;
      }
      else
      {
        pCtx->hDma.Init.PeriphDataAlignment = pHdle->destDataWidth;
        pCtx->hDma.Init.MemDataAlignment    = pHdle->srcDataWidth;
      }

      pCtx->hDma.Init.Mode                  = DMA_CIRCULAR;
      pCtx->hDma.Init.Priority              = DMA_PRIORITY_HIGH;
      pCtx->hDma.Init.FIFOMode              = DMA_FIFOMODE_DISABLE;
      pCtx->hDma.Init.FIFOThreshold         = DMA_FIFO_THRESHOLD_FULL; //DMA_FIFO_THRESHOLD_1QUARTERFULL
      pCtx->hDma.Init.MemBurst              = DMA_MBURST_SINGLE;
      pCtx->hDma.Init.PeriphBurst           = DMA_MBURST_SINGLE;

      pCtx->hDma.Instance = pHdle->pInstance; /* pInstance wording is used since it is a common driver, on H7 family it corresponds to stream */

      switch (pHdle->type)
      {
        case UTIL_AUDIO_DMA_IP_TYPE_SAI:

          /* Associate the DMA handle */
          if (pHdle->direction == DMA_PERIPH_TO_MEMORY)
          {
            SAI_HandleTypeDef *hsai = (SAI_HandleTypeDef *)pHdle->pIpHdle;
            __HAL_LINKDMA(hsai, hdmarx, pCtx->hDma);
          }
          else
          {
            SAI_HandleTypeDef *hsai = (SAI_HandleTypeDef *)pHdle->pIpHdle;
            __HAL_LINKDMA(hsai, hdmatx, pCtx->hDma);
          }
          break;
        case UTIL_AUDIO_DMA_IP_TYPE_DFSDM:
          pCtx->hDma.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_1QUARTERFULL;

          /* Associate the DMA handle */
          if (pHdle->direction == DMA_PERIPH_TO_MEMORY)
          {
            DFSDM_Filter_HandleTypeDef *hDfsdmFilter = (DFSDM_Filter_HandleTypeDef *)pHdle->pIpHdle;
            __HAL_LINKDMA(hDfsdmFilter, hdmaReg, pCtx->hDma);
          }
          else
          {
            error = UTIL_AUDIO_ERROR;
          }
          break;
        default:
          error = UTIL_AUDIO_ERROR;
          break;
      }

      if (error == UTIL_AUDIO_ERROR_NONE)
      {
        if (HAL_DMA_DeInit(&pCtx->hDma) != HAL_OK)
        {
          /* Deinitialize the Stream for new transfer */
          error = UTIL_AUDIO_ERROR;
        }
      }

      if (error == UTIL_AUDIO_ERROR_NONE)
      {
        if (HAL_DMA_Init(&pCtx->hDma) != HAL_OK)
        {
          /* Configure the DMA Stream */
          error = UTIL_AUDIO_ERROR;
        }
      }

      if (error == UTIL_AUDIO_ERROR_NONE)
      {
        /* SAI DMA IRQ Channel configuration */
        HAL_NVIC_SetPriority(pHdle->irqn, pHdle->preemptPriority, 0UL);
        HAL_NVIC_EnableIRQ(pHdle->irqn);
      }
    }
  }

  return error;
}


int32_t UTIL_AUDIO_DMA_DeInit(UTIL_AUDIO_DMA_t const *const pHdle)
{
  int32_t error = UTIL_AUDIO_ERROR_NONE;

  if ((pHdle == NULL) || (G_ContextNb == 0U))
  {
    error = UTIL_AUDIO_ERROR;
  }
  else
  {
    Context_t *pCtx = NULL;

    for (uint8_t idx = 0U; idx < UTIL_AUDIO_DMA_INSTANCES_MAX_NB; idx++)
    {
      if (pHdle == G_Context[idx].pHdle)
      {
        /* pHdle found */
        pCtx = &G_Context[idx];
        break;
      }
    }

    if (pCtx == NULL)
    {
      /* there is no dma instance associated to this pHdle */
      error = UTIL_AUDIO_ERROR;
    }
    else
    {
      /* Disable SAI DMA Channel IRQ */
      HAL_NVIC_DisableIRQ(pHdle->irqn);

      /* Reset the DMA Channel configuration*/
      if (HAL_DMA_DeInit(&pCtx->hDma) != HAL_OK)
      {
        error = UTIL_AUDIO_ERROR;
      }

      /* clear context */
      memset(pCtx, 0, sizeof(Context_t));
      G_ContextNb--;
    }
  }

  return error;
}

#endif /* UTIL_AUDIO_DMA_USED */
