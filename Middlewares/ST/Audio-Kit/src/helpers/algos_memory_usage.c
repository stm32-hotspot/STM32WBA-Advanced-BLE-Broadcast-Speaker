/**
******************************************************************************
* @file    algos_memory_usage.c
* @author  MCD Application Team
* @brief   Helper file to manage memory usage
*          parallel audio path
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


/* Includes ------------------------------------------------------------------*/
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "algos_memory_usage.h"
#include "audio_chain_instance.h"
#include "audio_mem_mgnt.h"
#include "audio_assert.h"
#include "stm32_term.h"
#include "irq_utils.h"


/* Global variables ----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/

// if DISPLAY_REQUESTED_AND_ALLOCATED is enabled, displayDetailedAlgosMemoryUsage() displays both requested allocation and real allocation
// else it displays only real allocation
#define DISPLAY_REQUESTED_AND_ALLOCATED

#define AUDIO_MEM_DISABLE_IRQ disable_irq_with_cnt
#define AUDIO_MEM_ENABLE_IRQ  enable_irq_with_cnt

/* Private macros ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Functions Definition ------------------------------------------------------*/
void displayDetailedAlgosMemoryUsage(bool const verbose)
{
  if (AudioChain_isStarted(&AudioChainInstance))
  {
    audio_chain_utilities_t *const pUtilsHandle        = AudioChain_getUtilsHdle(&AudioChainInstance);
    memAllocStat_t    const *const pBuffersMallocStats = AudioChain_getBuffersMallocStatsPtr(&AudioChainInstance);
    memAllocStat_t    const *const pChunksMallocStats  = AudioChain_getChunksMallocStatsPtr(&AudioChainInstance);
    struct
    {
      const char    *pName;
      memPool_t      memPool;
      memAllocStat_t chunksMallocStat;
      memAllocStat_t algosMallocStat;
    }
    memPoolList[AUDIO_MEM_NB_POOL] =
    {
      {
        .pName     = "TCM   ",
        .memPool   = AUDIO_MEM_TCM,
        .chunksMallocStat =
        {
          .totalRequestedAllocSize = 0UL,
          .totalAllocatedAllocSize = 0UL,
          .nbRequestedMalloc       = 0U,
          .nbAllocatedMalloc       = 0U
        },
        .algosMallocStat =
        {
          .totalRequestedAllocSize = 0UL,
          .totalAllocatedAllocSize = 0UL,
          .nbRequestedMalloc       = 0U,
          .nbAllocatedMalloc       = 0U
        }
      },
      {
        .pName     = "RAMINT",
        .memPool   = AUDIO_MEM_RAMINT,
        .chunksMallocStat =
        {
          .totalRequestedAllocSize = 0UL,
          .totalAllocatedAllocSize = 0UL,
          .nbRequestedMalloc       = 0U,
          .nbAllocatedMalloc       = 0U
        },
        .algosMallocStat =
        {
          .totalRequestedAllocSize = 0UL,
          .totalAllocatedAllocSize = 0UL,
          .nbRequestedMalloc       = 0U,
          .nbAllocatedMalloc       = 0U
        }
      },
      {
        .pName     = "RAMEXT",
        .memPool   = AUDIO_MEM_RAMEXT,
        .chunksMallocStat =
        {
          .totalRequestedAllocSize = 0UL,
          .totalAllocatedAllocSize = 0UL,
          .nbRequestedMalloc       = 0U,
          .nbAllocatedMalloc       = 0U
        },
        .algosMallocStat =
        {
          .totalRequestedAllocSize = 0UL,
          .totalAllocatedAllocSize = 0UL,
          .nbRequestedMalloc       = 0U,
          .nbAllocatedMalloc       = 0U
        }
      },
      #ifdef AUDIO_MEM_NOCACHED_ENABLED
      {
        .pName     = "NOCACH",
        .memPool   = AUDIO_MEM_NOCACHED,
        .chunksMallocStat =
        {
          .totalRequestedAllocSize = 0UL,
          .totalAllocatedAllocSize = 0UL,
          .nbRequestedMalloc       = 0U,
          .nbAllocatedMalloc       = 0U
        },
        .algosMallocStat =
        {
          .totalRequestedAllocSize = 0UL,
          .totalAllocatedAllocSize = 0UL,
          .nbRequestedMalloc       = 0U,
          .nbAllocatedMalloc       = 0U
        }
      }
      #endif
    };
    size_t   chunksTotalRequestedAllocSize = 0UL;
    size_t   chunksTotalAllocatedAllocSize = 0UL;
    size_t   grandTotalRequestedAllocSize  = 0UL;
    size_t   grandTotalAllocatedAllocSize  = 0UL;
    uint16_t chunksTotalNbRequestedMalloc  = 0U;
    uint16_t chunksTotalNbAllocatedMalloc  = 0U;
    uint16_t grandTotalNbRequestedMalloc   = 0U;
    uint16_t grandTotalNbAllocatedMalloc   = 0U;

    if (verbose)
    {
      UTIL_TERM_printf("\naudio-chain chunks:\n");
    }
    for (size_t idxMemPool = 0U; idxMemPool < (size_t)AUDIO_MEM_NB_POOL; idxMemPool++)
    {
      memAllocStat_t const *const pBuffersMallocStat = &pBuffersMallocStats[memPoolList[idxMemPool].memPool];
      memAllocStat_t const *const pChunksMallocStat  = &pChunksMallocStats[memPoolList[idxMemPool].memPool];

      memPoolList[idxMemPool].chunksMallocStat.totalRequestedAllocSize = pBuffersMallocStat->totalRequestedAllocSize + pChunksMallocStat->totalRequestedAllocSize;
      memPoolList[idxMemPool].chunksMallocStat.totalAllocatedAllocSize = pBuffersMallocStat->totalAllocatedAllocSize + pChunksMallocStat->totalAllocatedAllocSize;
      memPoolList[idxMemPool].chunksMallocStat.nbRequestedMalloc       = pBuffersMallocStat->nbRequestedMalloc       + pChunksMallocStat->nbRequestedMalloc;
      memPoolList[idxMemPool].chunksMallocStat.nbAllocatedMalloc       = pBuffersMallocStat->nbAllocatedMalloc       + pChunksMallocStat->nbAllocatedMalloc;
      chunksTotalRequestedAllocSize                                   += memPoolList[idxMemPool].chunksMallocStat.totalRequestedAllocSize;
      chunksTotalAllocatedAllocSize                                   += memPoolList[idxMemPool].chunksMallocStat.totalAllocatedAllocSize;
      chunksTotalNbRequestedMalloc                                    += memPoolList[idxMemPool].chunksMallocStat.nbRequestedMalloc;
      chunksTotalNbAllocatedMalloc                                    += memPoolList[idxMemPool].chunksMallocStat.nbAllocatedMalloc;

      if (verbose)
      {
        #ifdef DISPLAY_REQUESTED_AND_ALLOCATED
        UTIL_TERM_printf("    %s : requested %6d bytes in %3d malloc - allocated %6d bytes in %3d malloc\n",
                         memPoolList[idxMemPool].pName,
                         memPoolList[idxMemPool].chunksMallocStat.totalRequestedAllocSize,
                         memPoolList[idxMemPool].chunksMallocStat.nbRequestedMalloc,
                         memPoolList[idxMemPool].chunksMallocStat.totalAllocatedAllocSize,
                         memPoolList[idxMemPool].chunksMallocStat.nbAllocatedMalloc);
        #else
        UTIL_TERM_printf("    %s : %6d bytes in %3d malloc\n",
                         memPoolList[idxMemPool].pName,
                         memPoolList[idxMemPool].chunksMallocStat.totalAllocatedAllocSize,
                         memPoolList[idxMemPool].chunksMallocStat.nbAllocatedMalloc);
        #endif
      }
    }
    AUDIO_ASSERT(chunksTotalRequestedAllocSize == chunksTotalAllocatedAllocSize, pUtilsHandle, "");
    AUDIO_ASSERT(chunksTotalNbRequestedMalloc  == chunksTotalNbAllocatedMalloc,  pUtilsHandle, "");
    if (verbose)
    {
      UTIL_TERM_printf("    Total  :           %6d bytes in %3d malloc\n", chunksTotalAllocatedAllocSize, chunksTotalNbAllocatedMalloc);
    }
    else
    {
      UTIL_TERM_printf("\naudio-chain chunks:                                             %6d bytes\n", chunksTotalAllocatedAllocSize);
    }

    UTIL_TERM_printf("\naudio-chain algos:\n");
    for (audio_algo_list_t *pAlgoList = AudioChain_getAlgosList(&AudioChainInstance); pAlgoList != NULL; pAlgoList = pAlgoList->next)
    {
      memAllocStat_t              algoMallocStats[AUDIO_MEM_NB_POOL];
      memAllocStat_t const *const pAlgoMallocStats            = AudioAlgo_getMallocStats(pAlgoList->pAlgo);
      size_t                      algoTotalRequestedAllocSize = 0UL;
      size_t                      algoTotalAllocatedAllocSize = 0UL;
      uint16_t                    algoTotalNbRequestedMalloc  = 0U;
      uint16_t                    algoTotalNbAllocatedMalloc  = 0U;

      // copy pAlgoMallocStats into algosAllocStats with interrupts disabled to have consistent stats
      AUDIO_MEM_DISABLE_IRQ();
      for (size_t idxMemPool = 0U; idxMemPool < (size_t)AUDIO_MEM_NB_POOL; idxMemPool++)
      {
        algoMallocStats[idxMemPool] = pAlgoMallocStats[memPoolList[idxMemPool].memPool];
      }
      AUDIO_MEM_ENABLE_IRQ();

      if (verbose)
      {
        UTIL_TERM_printf("  %s:\n", AudioAlgo_getFullDescriptionString(pAlgoList->pAlgo));
      }
      for (size_t idxMemPool = 0U; idxMemPool < (size_t)AUDIO_MEM_NB_POOL; idxMemPool++)
      {
        memAllocStat_t const *const pMallocStat = &algoMallocStats[idxMemPool];

        if (verbose)
        {
          #ifdef DISPLAY_REQUESTED_AND_ALLOCATED
          UTIL_TERM_printf("    %s : requested %6d bytes in %3d malloc - allocated %6d bytes in %3d malloc\n",
                           memPoolList[idxMemPool].pName,
                           pMallocStat->totalRequestedAllocSize,
                           pMallocStat->nbRequestedMalloc,
                           pMallocStat->totalAllocatedAllocSize,
                           pMallocStat->nbAllocatedMalloc);
          #else
          UTIL_TERM_printf("    %s : %6d bytes in %3d malloc\n",
                           memPoolList[idxMemPool].pName,
                           pMallocStat->totalAllocatedAllocSize,
                           pMallocStat->nbAllocatedMalloc);
          #endif
        }

        memPoolList[idxMemPool].algosMallocStat.totalRequestedAllocSize += pMallocStat->totalRequestedAllocSize;
        memPoolList[idxMemPool].algosMallocStat.totalAllocatedAllocSize += pMallocStat->totalAllocatedAllocSize;
        memPoolList[idxMemPool].algosMallocStat.nbRequestedMalloc       += pMallocStat->nbRequestedMalloc;
        memPoolList[idxMemPool].algosMallocStat.nbAllocatedMalloc       += pMallocStat->nbAllocatedMalloc;
        algoTotalRequestedAllocSize                                     += pMallocStat->totalRequestedAllocSize;
        algoTotalAllocatedAllocSize                                     += pMallocStat->totalAllocatedAllocSize;
        algoTotalNbRequestedMalloc                                      += pMallocStat->nbRequestedMalloc;
        algoTotalNbAllocatedMalloc                                      += pMallocStat->nbAllocatedMalloc;
      }
      AUDIO_ASSERT(algoTotalRequestedAllocSize == algoTotalAllocatedAllocSize, pUtilsHandle, "");
      AUDIO_ASSERT(algoTotalNbRequestedMalloc  == algoTotalNbAllocatedMalloc,  pUtilsHandle, "");
      if (verbose)
      {
        UTIL_TERM_printf("    Total  :           %6d bytes in %3d malloc\n", algoTotalAllocatedAllocSize, algoTotalNbAllocatedMalloc);
      }
      else
      {
        UTIL_TERM_printf("  %-60s: %6d bytes\n", AudioAlgo_getFullDescriptionString(pAlgoList->pAlgo), algoTotalAllocatedAllocSize);
      }
    }

    if (verbose)
    {
      UTIL_TERM_printf("\nall audio-chain algos:\n");
    }
    for (size_t idxMemPool = 0U; idxMemPool < (size_t)AUDIO_MEM_NB_POOL; idxMemPool++)
    {
      if (verbose)
      {
        #ifdef DISPLAY_REQUESTED_AND_ALLOCATED
        UTIL_TERM_printf("    %s : requested %6d bytes in %3d malloc - allocated %6d bytes in %3d malloc\n",
                         memPoolList[idxMemPool].pName,
                         memPoolList[idxMemPool].algosMallocStat.totalRequestedAllocSize,
                         memPoolList[idxMemPool].algosMallocStat.nbRequestedMalloc,
                         memPoolList[idxMemPool].algosMallocStat.totalAllocatedAllocSize,
                         memPoolList[idxMemPool].algosMallocStat.nbAllocatedMalloc);
        #else
        UTIL_TERM_printf("    %s : %6d bytes in %3d malloc\n",
                         memPoolList[idxMemPool].pName,
                         memPoolList[idxMemPool].algosMallocStat.totalAllocatedAllocSize,
                         memPoolList[idxMemPool].algosMallocStat.nbAllocatedMalloc);
        #endif
      }

      grandTotalRequestedAllocSize += memPoolList[idxMemPool].algosMallocStat.totalRequestedAllocSize;
      grandTotalAllocatedAllocSize += memPoolList[idxMemPool].algosMallocStat.totalAllocatedAllocSize;
      grandTotalNbRequestedMalloc  += memPoolList[idxMemPool].algosMallocStat.nbRequestedMalloc;
      grandTotalNbAllocatedMalloc  += memPoolList[idxMemPool].algosMallocStat.nbAllocatedMalloc;
    }
    AUDIO_ASSERT(grandTotalRequestedAllocSize == grandTotalAllocatedAllocSize, pUtilsHandle, "");
    AUDIO_ASSERT(grandTotalNbRequestedMalloc  == grandTotalNbAllocatedMalloc,  pUtilsHandle, "");
    if (verbose)
    {
      UTIL_TERM_printf("    Total  :           %6d bytes in %3d malloc\n", grandTotalAllocatedAllocSize, grandTotalNbAllocatedMalloc);
    }
    else
    {
      UTIL_TERM_printf("all audio-chain algos:                                          %6d bytes\n", grandTotalAllocatedAllocSize);
    }

    if (verbose)
    {
      UTIL_TERM_printf("\nall audio-chain chunks and algos:\n");
      for (size_t idxMemPool = 0U; idxMemPool < (size_t)AUDIO_MEM_NB_POOL; idxMemPool++)
      {
        #ifdef DISPLAY_REQUESTED_AND_ALLOCATED
        UTIL_TERM_printf("    %s : requested %6d bytes in %3d malloc - allocated %6d bytes in %3d malloc\n",
                         memPoolList[idxMemPool].pName,
                         memPoolList[idxMemPool].chunksMallocStat.totalRequestedAllocSize + memPoolList[idxMemPool].algosMallocStat.totalRequestedAllocSize,
                         memPoolList[idxMemPool].chunksMallocStat.nbRequestedMalloc       + memPoolList[idxMemPool].algosMallocStat.nbRequestedMalloc,
                         memPoolList[idxMemPool].chunksMallocStat.totalAllocatedAllocSize + memPoolList[idxMemPool].algosMallocStat.totalAllocatedAllocSize,
                         memPoolList[idxMemPool].chunksMallocStat.nbAllocatedMalloc       + memPoolList[idxMemPool].algosMallocStat.nbAllocatedMalloc);
        #else
        UTIL_TERM_printf("    %s : %6d bytes in %3d malloc\n",
                         memPoolList[idxMemPool].pName,
                         memPoolList[idxMemPool].chunksMallocStat.totalAllocatedAllocSize + memPoolList[idxMemPool].algosMallocStat.totalAllocatedAllocSize,
                         memPoolList[idxMemPool].chunksMallocStat.nbAllocatedMalloc       + memPoolList[idxMemPool].algosMallocStat.nbAllocatedMalloc);
        #endif
      }
      UTIL_TERM_printf("    Total  :           %6d bytes in %3d malloc\n\n\n", chunksTotalAllocatedAllocSize + grandTotalAllocatedAllocSize, chunksTotalNbAllocatedMalloc + grandTotalNbAllocatedMalloc);
    }
    else
    {
      UTIL_TERM_printf("\nall audio-chain chunks and algos:                               %6d bytes\n\n\n", chunksTotalAllocatedAllocSize + grandTotalAllocatedAllocSize);
    }
  }
}

