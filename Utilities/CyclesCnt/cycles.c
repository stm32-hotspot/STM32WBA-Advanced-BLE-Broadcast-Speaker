/**
  ******************************************************************************
  * @file    cycles.c
  * @author  MCD Application Team
  * @brief   cycles counter/statistics management
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2017(-2022) STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/

#include <string.h>   // for memmove
#include "cycles.h"

#ifdef CYCLES_CNT_TRACES_USED
  #include "traces.h"
#endif

//#define DEBUG_CYCLES
#ifdef DEBUG_CYCLES
#include <stdio.h>

#define DEBUG_CYCLES_SIZE 500
static struct
{
  const char                                           *pName;
  uint32_t                                              time;
  enum {CMD_FULL_RESET, CMD_RESET, CMD_START, CMD_STOP} cmd;
  uint8_t                                               deepness;
  //CycleStatsTypeDef                                    *pCycleStats;
} s_debugCycles[DEBUG_CYCLES_SIZE];
static uint32_t s_idDebugCycles = 0UL;
#endif

/* Global variables ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
#define CYCLES_COUNT_MIN_FOR_MEASURE 2UL

#ifndef CYCLES_CNT_PARALLEL_MEASURE_MAX_NB
  #define CYCLES_CNT_PARALLEL_MEASURE_MAX_NB 10
#endif

#if defined(CYCLES_CNT_TRACES_USED) && defined(CYCLES_CNT_PRINTF_USED)
  #error "CYCLES_CNT_TRACES_USED & CYCLES_CNT_PRINTF_USED are exclusive; please select one or the other"
#endif

/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static CycleStatsTypeDef *pCycleStatsMeasureActive[CYCLES_CNT_PARALLEL_MEASURE_MAX_NB];
static int nbParallelCyclesMeasure = 0;

/* Private function prototypes -----------------------------------------------*/
static uint64_t s_correctedTotalCycles(CycleStatsTypeDef const *const pCycleStats, bool const last);
static void s_trace(traceLvl_t const level, const char *pStr);
#ifdef DEBUG_CYCLES
  static void s_debugCyclesFunc(CycleStatsTypeDef *const pCycleStats, uint32_t const time, uint8_t const cmd);
#endif

/* Functions Definition ------------------------------------------------------*/

/**
  * @brief  Init Cortex M registers and static variables
  * @param  Nones
  * @retval None
  */
void cycleMeasure_Init(void)
{
  static bool initDone = false;

  if (!initDone)
  {
    initDone                = true;
    CoreDebug->DEMCR       |= 0x01000000UL;
    DWT->CYCCNT             = 0UL;                  // reset the counter
    DWT->CTRL              |= 1UL;                  // enable the counter
    #if !defined(STM32H5) && !defined(STM32N6) && !defined(STM32WBA)
    DWT->LAR                = 0xC5ACCE55UL;         // <-- added unlock access to DWT (ITM, etc.)registers
    #endif
    nbParallelCyclesMeasure = 0;
  }
}


/**
  * @brief  Reset current cycles stats
  * @param  Handler for cycles statistics
  * @retval None
  */
void cycleMeasure_Reset(CycleStatsTypeDef *const pCycleStats)
{
  if (pCycleStats != NULL)
  {
    // reset cycles now and set state to START_CYCLES
    memset(&pCycleStats->current, 0, sizeof(pCycleStats->current));
    pCycleStats->lastCycles     = DWT->CYCCNT;
    pCycleStats->nextState      = START_CYCLES;
    pCycleStats->cyclesToRemove = 0UL;
    #ifdef DEBUG_CYCLES
    s_debugCyclesFunc(pCycleStats, DWT->CYCCNT, CMD_RESET);
    #endif
  }
}


/**
  * @brief  Reset current and last cycles stats
  * @param  Handler for cycles statistics
  * @retval None
  */
void cycleMeasure_FullReset(CycleStatsTypeDef *const pCycleStats)
{
  if (pCycleStats != NULL)
  {
    // reset cycles now and set state to RESET_CYCLES
    memset(&pCycleStats->current, 0, sizeof(pCycleStats->current));
    memset(&pCycleStats->last,    0, sizeof(pCycleStats->last));
    pCycleStats->lastCycles     = DWT->CYCCNT;
    pCycleStats->nextState      = START_CYCLES;
    pCycleStats->cyclesToRemove = 0UL;
    #ifdef DEBUG_CYCLES
    s_debugCyclesFunc(pCycleStats, DWT->CYCCNT, CMD_FULL_RESET);
    #endif
  }
}


/**
  * @brief  Start counting cycles
  * @param  Handler for cycles statistics
  * @retval None
  */
void cycleMeasure_Start(CycleStatsTypeDef *const pCycleStats)
{
  if (pCycleStats != NULL)
  {
    const char *pTrace1     = NULL;
    traceLvl_t  traceLevel1 = TRACE_LVL_MUTE;
    const char *pTrace2     = NULL;
    traceLvl_t  traceLevel2 = TRACE_LVL_MUTE;
    uint32_t    startCycles, nbCycles;

    CYCLES_CNT_DISABLE_IRQ();
    startCycles = DWT->CYCCNT;
    nbCycles    = 0UL;
    if (pCycleStats->nextState == START_CYCLES)
    {
      if (pCycleStats->current.count == 0UL)
      {
        // beginning: do not update totalCycles
      }
      else
      {
        nbCycles = startCycles - pCycleStats->lastCycles;
      }
    }
    else if (pCycleStats->nextState == STOP_CYCLES)
    {
      // shouldn't happen !
      traceLevel1 = TRACE_LVL_ERROR;
      pTrace1     = "cycleMeasure_Start: 2 successive calls !\n";
      cycleMeasure_Reset(pCycleStats);
    }
    else
    {
      // shouldn't happen !
      traceLevel1 = TRACE_LVL_ERROR_FATAL;
      pTrace1     = "cycleMeasure_Start: unknown state !\n";
      cycleMeasure_Reset(pCycleStats);
    }

    if (nbParallelCyclesMeasure < CYCLES_CNT_PARALLEL_MEASURE_MAX_NB)
    {
      pCycleStatsMeasureActive[nbParallelCyclesMeasure] = pCycleStats;
      nbParallelCyclesMeasure++;
      #ifdef DEBUG_CYCLES
      s_debugCyclesFunc(pCycleStats, startCycles, CMD_START);
      #endif

      pCycleStats->current.totalCycles += (uint64_t)nbCycles;
      pCycleStats->cyclesToRemove       = 0UL;
      pCycleStats->lastCycles           = startCycles;
      pCycleStats->nextState            = STOP_CYCLES;
    }
    else
    {
      // shouldn't happen => increase CYCLES_CNT_PARALLEL_MEASURE_MAX_NB !
      traceLevel2 = TRACE_LVL_ERROR;
      pTrace2     = "cycleMeasure_Start: too many parallel measures, CYCLES_CNT_PARALLEL_MEASURE_MAX_NB should be increased\n";
    }
    CYCLES_CNT_ENABLE_IRQ();

    if (pTrace1 != NULL)
    {
      s_trace(traceLevel1, pTrace1);
    }
    if (pTrace2 != NULL)
    {
      s_trace(traceLevel2, pTrace2);
    }
  }
}


/**
  * @brief  Stop counting cycles
  * @param  Handler for cycles statistics
  * @param  timeout unit ms
  * @retval 1 if time duration since last cycleMeasure_Reset() call >= timeout, else 0
  */
bool cycleMeasure_Stop(CycleStatsTypeDef *const pCycleStats, uint32_t const timeoutMs)
{
  bool ret = false;

  if (pCycleStats != NULL)
  {
    const char *pTrace     = NULL;
    traceLvl_t  traceLevel = TRACE_LVL_MUTE;
    uint32_t    stopCycles;

    CYCLES_CNT_DISABLE_IRQ();
    stopCycles = DWT->CYCCNT;
    if (pCycleStats->nextState == STOP_CYCLES)
    {
      uint32_t nbCycles = stopCycles - pCycleStats->lastCycles - pCycleStats->cyclesToRemove;   // corrected nbCycles: removed interrupting tasks cycles
      uint32_t elapsedTimeMs;

      if (pCycleStats->current.count == 0UL)
      {
        pCycleStats->current.min = nbCycles;
        pCycleStats->current.max = nbCycles;
      }
      else
      {
        pCycleStats->current.min = (nbCycles < pCycleStats->current.min) ? nbCycles : pCycleStats->current.min;
        pCycleStats->current.max = (nbCycles > pCycleStats->current.max) ? nbCycles : pCycleStats->current.max;
      }
      pCycleStats->current.sum  += (uint64_t) nbCycles;
      pCycleStats->current.sum2 += (uint64_t) nbCycles * nbCycles;
      pCycleStats->current.count++;

      // remove cycles of interrupted tasks
      if (nbParallelCyclesMeasure == 0)
      {
        // shouldn't happen !
        traceLevel = TRACE_LVL_ERROR;
        pTrace     = "cycleMeasure_Stop: nbParallelCyclesMeasure == 0 !\n";
        cycleMeasure_Reset(pCycleStats);
      }
      else
      {
        int parallelCyclesMeasureIdx;

        for (parallelCyclesMeasureIdx = nbParallelCyclesMeasure - 1; parallelCyclesMeasureIdx >= 0; parallelCyclesMeasureIdx--)
        {
          if (pCycleStatsMeasureActive[parallelCyclesMeasureIdx] == pCycleStats)
          {
            // pCycleStats found
            break;
          }
        }
        if (parallelCyclesMeasureIdx < 0)
        {
          // pCycleStats not found: shouldn't happen !
          traceLevel = TRACE_LVL_ERROR;
          pTrace     = "cycleMeasure_Stop: pCycleStats not found !\n";
        }
        else
        {
          // pCycleStats found: remove pCycleStats entry of pCycleStatsMeasureActive table and remove nbCycles from interrupted tasks (tasks whose pCycleStatsMeasureActive index < index of current pCycleStats)
          nbParallelCyclesMeasure--;
          if (parallelCyclesMeasureIdx < nbParallelCyclesMeasure)
          {
            memmove(&pCycleStatsMeasureActive[parallelCyclesMeasureIdx],
                    &pCycleStatsMeasureActive[parallelCyclesMeasureIdx + 1],
                    ((uint32_t)nbParallelCyclesMeasure - (uint32_t)parallelCyclesMeasureIdx) * sizeof(pCycleStatsMeasureActive[0]));
          }
          for (parallelCyclesMeasureIdx--; parallelCyclesMeasureIdx >= 0; parallelCyclesMeasureIdx--)
          {
            pCycleStatsMeasureActive[parallelCyclesMeasureIdx]->cyclesToRemove += nbCycles;
          }
          #ifdef DEBUG_CYCLES
          s_debugCyclesFunc(pCycleStats, stopCycles, CMD_STOP);
          #endif
        }
      }

      // real nbCycles for totalCycles estimation
      nbCycles += pCycleStats->cyclesToRemove;

      pCycleStats->current.totalCycles += (uint64_t)nbCycles;
      pCycleStats->lastCycles           = stopCycles;
      pCycleStats->nextState            = START_CYCLES;

      elapsedTimeMs = (uint32_t)((1000ULL * pCycleStats->current.totalCycles) / ((SystemCoreClock == 0UL) ? 1UL : SystemCoreClock));

      ret = (elapsedTimeMs >= timeoutMs);

      if (ret)
      {
        pCycleStats->last = pCycleStats->current;
      }
    }
    else if (pCycleStats->nextState == START_CYCLES)
    {
      // shouldn't happen !
      traceLevel = TRACE_LVL_ERROR;
      pTrace     = "cycleMeasure_Stop: 2 successive calls !\n";
      cycleMeasure_Reset(pCycleStats);
    }
    else
    {
      // shouldn't happen !
      traceLevel = TRACE_LVL_ERROR_FATAL;
      pTrace     = "cycleMeasure_Stop: unknown state !\n";
      cycleMeasure_Reset(pCycleStats);
    }
    CYCLES_CNT_ENABLE_IRQ();

    if (pTrace != NULL)
    {
      s_trace(traceLevel, pTrace);
    }
  }

  return ret;
}


/**
  * @brief  checks if timeout has expired
  * @param  Handler for cycles statistics
  * @param  timeout unit ms
  * @retval 1 if time duration since last cycleMeasure_Reset() call >= timeout, else 0
  */
bool cycleMeasure_isTimeoutExpired(CycleStatsTypeDef *const pCycleStats, uint32_t const timeoutMs)
{
  bool timeoutExpired = false;

  if ((timeoutMs > 0UL) && (pCycleStats->nextState == START_CYCLES))
  {
    // nextState == START_CYCLES means that current measured task is not active
    // thus we update pCycleStats->current.totalCycles as well as pCycleStats->lastCycles
    // and compute elapsed time since last timeout
    //
    // pCycleStats->lastCycles mustn't be updated if current task is active, it would
    // create a wrong current cycles evaluation in cycleMeasure_Stop()
    uint32_t const startCycles = DWT->CYCCNT;
    uint32_t const nbCycles    = startCycles - pCycleStats->lastCycles;
    uint32_t       elapsedTimeMs;

    pCycleStats->current.totalCycles += (uint64_t)nbCycles;
    pCycleStats->lastCycles           = startCycles;
    elapsedTimeMs                     = (uint32_t)((1000ULL * pCycleStats->current.totalCycles) / ((SystemCoreClock == 0UL) ? 1UL : SystemCoreClock));
    timeoutExpired                    = (elapsedTimeMs >= timeoutMs);
  }

  return timeoutExpired;
}


/**
  * @brief  Gets system core clock frequency
  * @param  None
  * @retval system core clock frequency (Hz)
  */
uint32_t cycleMeasure_getSystemCoreClock(void)
{
  return SystemCoreClock;
}


/**
  * @brief  Gets current cycles count of Cortex register
  * @param  None
  * @retval current cycles count of Cortex register
  */
uint32_t cycleMeasure_currentCycles(void)
{
  return DWT->CYCCNT;
}

/**
  * @brief  Gets current minimum cycles counted
  * @param  pCycleStats: Handler for cycles statistics
  * @param  measureType: last complete measure (if available) or current on-going measure
  * @retval current minimum cycles counted
  */
uint32_t cycleMeasure_minCycles(CycleStatsTypeDef const *const pCycleStats, CycleMeasureType_t const measureType)
{
  uint32_t ret = 0UL;

  if (pCycleStats != NULL)
  {
    CYCLES_CNT_DISABLE_IRQ();
    if ((measureType == CYCLES_LAST_MEASURE) && (pCycleStats->last.count >= CYCLES_COUNT_MIN_FOR_MEASURE))
    {
      ret = pCycleStats->last.min;
    }
    else if (pCycleStats->current.count >= CYCLES_COUNT_MIN_FOR_MEASURE)
    {
      ret = pCycleStats->current.min;
    }
    else
    {
      /* MISRAC: do nothing */
    }
    CYCLES_CNT_ENABLE_IRQ();
  }

  return ret;
}


/**
  * @brief  Gets current maximum cycles counted
  * @param  pCycleStats: Handler for cycles statistics
  * @param  measureType: last complete measure (if available) or current on-going measure
  * @retval current maximum cycles counted
  */
uint32_t cycleMeasure_maxCycles(CycleStatsTypeDef const *const pCycleStats, CycleMeasureType_t const measureType)
{
  uint32_t ret = 0UL;

  if (pCycleStats != NULL)
  {
    CYCLES_CNT_DISABLE_IRQ();
    if ((measureType == CYCLES_LAST_MEASURE) && (pCycleStats->last.count >= CYCLES_COUNT_MIN_FOR_MEASURE))
    {
      ret = pCycleStats->last.max;
    }
    else if (pCycleStats->current.count >= CYCLES_COUNT_MIN_FOR_MEASURE)
    {
      ret = pCycleStats->current.max;
    }
    else
    {
      /* MISRAC: do nothing */
    }
    CYCLES_CNT_ENABLE_IRQ();
  }

  return ret;
}


/**
  * @brief  Gets current average cycles counted
  * @param  pCycleStats: Handler for cycles statistics
  * @param  measureType: last complete measure (if available) or current on-going measure
  * @retval current average cycles counted
  */
uint32_t cycleMeasure_averageCycles(CycleStatsTypeDef const *const pCycleStats, CycleMeasureType_t const measureType)
{
  uint32_t ret = 0UL;

  if (pCycleStats != NULL)
  {
    CYCLES_CNT_DISABLE_IRQ();
    if ((measureType == CYCLES_LAST_MEASURE) && (pCycleStats->last.count >= CYCLES_COUNT_MIN_FOR_MEASURE))
    {
      ret = (uint32_t)(pCycleStats->last.sum / (uint64_t)pCycleStats->last.count);
    }
    else if (pCycleStats->current.count >= CYCLES_COUNT_MIN_FOR_MEASURE)
    {
      ret = (uint32_t)(pCycleStats->current.sum / (uint64_t)pCycleStats->current.count);
    }
    else
    {
      /* MISRAC: do nothing */
    }
    CYCLES_CNT_ENABLE_IRQ();
  }

  return ret;
}


/**
  * @brief  Gets current standard deviation of cycles counted
  * @param  pCycleStats: Handler for cycles statistics
  * @param  measureType: last complete measure (if available) or current on-going measure
  * @retval current standard deviation of cycles counted
  */
uint32_t cycleMeasure_stdDevCycles(CycleStatsTypeDef const *const pCycleStats, CycleMeasureType_t const measureType)
{
  uint32_t ret        = 0UL;
  bool     ok         = true;
  float    sumFloat   = 0.0f;
  float    sum2Float  = 0.0f;
  float    countFloat = 1.0f;

  if (pCycleStats != NULL)
  {
    CYCLES_CNT_DISABLE_IRQ();
    if ((measureType == CYCLES_LAST_MEASURE) && (pCycleStats->last.count >= CYCLES_COUNT_MIN_FOR_MEASURE))
    {
      sumFloat   = (float)pCycleStats->last.sum;
      sum2Float  = (float)pCycleStats->last.sum2;
      countFloat = (float)pCycleStats->last.count;
    }
    else if (pCycleStats->current.count >= CYCLES_COUNT_MIN_FOR_MEASURE)
    {
      sumFloat   = (float)pCycleStats->current.sum;
      sum2Float  = (float)pCycleStats->current.sum2;
      countFloat = (float)pCycleStats->current.count;
    }
    else
    {
      ok = false;
    }
    CYCLES_CNT_ENABLE_IRQ();
  }
  else
  {
    ok = false;
  }

  if (ok)
  {
    float const average = sumFloat / countFloat;
    float const stdDev2 = (sum2Float / countFloat) - (average * average);

    ret = (stdDev2 <= 0.0f) ? 0UL : (uint32_t)sqrtf(stdDev2); /*cstat !MISRAC2012-Rule-22.8 no issue with sqrtf of a positive value => errno check is useless*/
  }

  return ret;
}


/**
  * @brief  Gets current minimum CPU load measured - unit is Hz
  * @param  pCycleStats: Handler for cycles statistics
  * @param  measureType: last complete measure (if available) or current on-going measure
  * @retval current minimum CPU load measured - unit is Hz
  */
uint32_t cycleMeasure_minCpuLoadHz(CycleStatsTypeDef const *const pCycleStats, CycleMeasureType_t const measureType)
{
  uint32_t ret = 0UL;

  if (pCycleStats != NULL)
  {
    CYCLES_CNT_DISABLE_IRQ();
    if ((measureType == CYCLES_LAST_MEASURE) && (pCycleStats->last.count >= CYCLES_COUNT_MIN_FOR_MEASURE))
    {
      ret = (uint32_t)(((uint64_t)SystemCoreClock * (uint64_t)pCycleStats->last.min * (uint64_t)pCycleStats->last.count) / s_correctedTotalCycles(pCycleStats, true));
    }
    else if (pCycleStats->current.count >= CYCLES_COUNT_MIN_FOR_MEASURE)
    {
      ret = (uint32_t)(((uint64_t)SystemCoreClock * (uint64_t)pCycleStats->current.min * (uint64_t)pCycleStats->current.count) / s_correctedTotalCycles(pCycleStats, false));
    }
    else
    {
      /* MISRAC: do nothing */
    }
    CYCLES_CNT_ENABLE_IRQ();
  }

  return ret;
}


/**
  * @brief  Gets current maximum CPU load measured - unit is Hz
  * @param  pCycleStats: Handler for cycles statistics
  * @param  measureType: last complete measure (if available) or current on-going measure
  * @retval current maximum CPU load measured - unit is Hz
  */
uint32_t cycleMeasure_maxCpuLoadHz(CycleStatsTypeDef const *const pCycleStats, CycleMeasureType_t const measureType)
{
  uint32_t ret = 0UL;

  if (pCycleStats != NULL)
  {
    CYCLES_CNT_DISABLE_IRQ();
    if ((measureType == CYCLES_LAST_MEASURE) && (pCycleStats->last.count >= CYCLES_COUNT_MIN_FOR_MEASURE))
    {
      ret = (uint32_t)(((uint64_t)SystemCoreClock * (uint64_t)pCycleStats->last.max * (uint64_t)pCycleStats->last.count) / s_correctedTotalCycles(pCycleStats, true));
    }
    else if (pCycleStats->current.count >= CYCLES_COUNT_MIN_FOR_MEASURE)
    {
      ret = (uint32_t)(((uint64_t)SystemCoreClock * (uint64_t)pCycleStats->current.max * (uint64_t)pCycleStats->current.count) / s_correctedTotalCycles(pCycleStats, false));
    }
    else
    {
      /* MISRAC: do nothing */
    }
    CYCLES_CNT_ENABLE_IRQ();
  }

  return ret;
}


/**
  * @brief  Gets current average CPU load measured - unit is Hz
  * @param  pCycleStats: Handler for cycles statistics
  * @param  measureType: last complete measure (if available) or current on-going measure
  * @retval current average CPU load measured - unit is Hz
  */
uint32_t cycleMeasure_averageCpuLoadHz(CycleStatsTypeDef const *const pCycleStats, CycleMeasureType_t const measureType)
{
  uint32_t ret = 0UL;

  if (pCycleStats != NULL)
  {
    CYCLES_CNT_DISABLE_IRQ();
    if ((measureType == CYCLES_LAST_MEASURE) && (pCycleStats->last.count >= CYCLES_COUNT_MIN_FOR_MEASURE))
    {
      ret = (uint32_t)(((uint64_t)SystemCoreClock * pCycleStats->last.sum) / s_correctedTotalCycles(pCycleStats, true));
    }
    else if (pCycleStats->current.count >= CYCLES_COUNT_MIN_FOR_MEASURE)
    {
      ret = (uint32_t)(((uint64_t)SystemCoreClock * pCycleStats->current.sum) / s_correctedTotalCycles(pCycleStats, false));
    }
    else
    {
      /* MISRAC: do nothing */
    }
    CYCLES_CNT_ENABLE_IRQ();
  }

  return ret;
}


/**
  * @brief  Gets current average CPU load measured - unit is MHz
  * @param  pCycleStats: Handler for cycles statistics
  * @param  measureType: last complete measure (if available) or current on-going measure
  * @retval current average CPU load measured - unit is MHz
  */
uint32_t cycleMeasure_averageCpuLoadMHz(CycleStatsTypeDef const *const pCycleStats, CycleMeasureType_t const measureType)
{
  uint32_t ret = 0UL;

  if (pCycleStats != NULL)
  {
    CYCLES_CNT_DISABLE_IRQ();
    if ((measureType == CYCLES_LAST_MEASURE) && (pCycleStats->last.count >= CYCLES_COUNT_MIN_FOR_MEASURE))
    {
      ret = (uint32_t)(((uint64_t)SystemCoreClock * pCycleStats->last.sum) / (s_correctedTotalCycles(pCycleStats, true) * 1000000ULL));
    }
    else if (pCycleStats->current.count >= CYCLES_COUNT_MIN_FOR_MEASURE)
    {
      ret = (uint32_t)(((uint64_t)SystemCoreClock * pCycleStats->current.sum) / (s_correctedTotalCycles(pCycleStats, false) * 1000000ULL));
    }
    else
    {
      /* MISRAC: do nothing */
    }
    CYCLES_CNT_ENABLE_IRQ();
  }

  return ret;
}


/**
  * @brief  Gets current standard deviation of CPU load measured - unit is Hz
  * @param  pCycleStats: Handler for cycles statistics
  * @param  measureType: last complete measure (if available) or current on-going measure
  * @retval current standard deviation of CPU load measured - unit is Hz
  */
uint32_t cycleMeasure_stdDevCpuLoadHz(CycleStatsTypeDef const *const pCycleStats, CycleMeasureType_t const measureType)
{
  uint32_t ret              = 0UL;
  bool     ok               = true;
  float    sumFloat         = 0.0f;
  float    sum2Float        = 0.0f;
  float    countFloat       = 1.0f;
  float    totalCyclesFloat = 1.0f;

  if (pCycleStats != NULL)
  {
    CYCLES_CNT_DISABLE_IRQ();
    if ((measureType == CYCLES_LAST_MEASURE) && (pCycleStats->last.count >= CYCLES_COUNT_MIN_FOR_MEASURE))
    {
      sumFloat         = (float)pCycleStats->last.sum;
      sum2Float        = (float)pCycleStats->last.sum2;
      countFloat       = (float)pCycleStats->last.count;
      totalCyclesFloat = (float)s_correctedTotalCycles(pCycleStats, true);
    }
    else if (pCycleStats->current.count >= CYCLES_COUNT_MIN_FOR_MEASURE)
    {
      sumFloat         = (float)pCycleStats->current.sum;
      sum2Float        = (float)pCycleStats->current.sum2;
      countFloat       = (float)pCycleStats->current.count;
      totalCyclesFloat = (float)s_correctedTotalCycles(pCycleStats, false);
    }
    else
    {
      ok = false;
    }
    CYCLES_CNT_ENABLE_IRQ();
  }
  else
  {
    ok = false;
  }

  if (ok)
  {
    float const tmp           = (sum2Float * countFloat) - (sumFloat * sumFloat);
    float const stdDevHzFloat = (tmp <= 0.0f) ? 0.0f : ((float)SystemCoreClock * sqrtf(tmp) / totalCyclesFloat); /*cstat !MISRAC2012-Rule-22.8 no issue with sqrtf of a positive value => errno check is useless*/

    ret = (uint32_t)stdDevHzFloat;
  }

  return ret;
}


/**
  * @brief  Gets current min CPU load in %
  * @param  pCycleStats: Handler for cycles statistics
  * @param  measureType: last complete measure (if available) or current on-going measure
  * @retval current min CPU load in %
  */
float cycleMeasure_minCpuLoadPcent(CycleStatsTypeDef const *const pCycleStats, CycleMeasureType_t const measureType)
{
  float ret = 0.0f;

  if (pCycleStats != NULL)
  {
    CYCLES_CNT_DISABLE_IRQ();
    if ((measureType == CYCLES_LAST_MEASURE) && (pCycleStats->last.count >= CYCLES_COUNT_MIN_FOR_MEASURE))
    {
      ret = 100.0f * (float)pCycleStats->last.min * (float)pCycleStats->last.count / (float)s_correctedTotalCycles(pCycleStats, true);
    }
    else if (pCycleStats->current.count >= CYCLES_COUNT_MIN_FOR_MEASURE)
    {
      ret = 100.0f * (float)pCycleStats->current.min * (float)pCycleStats->current.count / (float)s_correctedTotalCycles(pCycleStats, false);
    }
    else
    {
      /* MISRAC: do nothing */
    }
    CYCLES_CNT_ENABLE_IRQ();
  }

  return ret;
}


/**
  * @brief  Gets current max CPU load in %
  * @param  pCycleStats: Handler for cycles statistics
  * @param  measureType: last complete measure (if available) or current on-going measure
  * @retval current max CPU load in %
  */
float cycleMeasure_maxCpuLoadPcent(CycleStatsTypeDef const *const pCycleStats, CycleMeasureType_t const measureType)
{
  float ret = 0.0f;

  if (pCycleStats != NULL)
  {
    CYCLES_CNT_DISABLE_IRQ();
    if ((measureType == CYCLES_LAST_MEASURE) && (pCycleStats->last.count >= CYCLES_COUNT_MIN_FOR_MEASURE))
    {
      ret = 100.0f * (float)pCycleStats->last.max * (float)pCycleStats->last.count / (float)s_correctedTotalCycles(pCycleStats, true);
    }
    else if (pCycleStats->current.count >= CYCLES_COUNT_MIN_FOR_MEASURE)
    {
      ret = 100.0f * (float)pCycleStats->current.max * (float)pCycleStats->current.count / (float)s_correctedTotalCycles(pCycleStats, false);
    }
    else
    {
      /* MISRAC: do nothing */
    }
    CYCLES_CNT_ENABLE_IRQ();
  }

  return ret;
}


/**
  * @brief  Gets current average CPU load in %
  * @param  pCycleStats: Handler for cycles statistics
  * @param  measureType: last complete measure (if available) or current on-going measure
  * @retval current average CPU load in %
  */
float cycleMeasure_averageCpuLoadPcent(CycleStatsTypeDef const *const pCycleStats, CycleMeasureType_t const measureType)
{
  float ret = 0.0f;

  if (pCycleStats != NULL)
  {
    CYCLES_CNT_DISABLE_IRQ();
    if ((measureType == CYCLES_LAST_MEASURE) && (pCycleStats->last.count >= CYCLES_COUNT_MIN_FOR_MEASURE))
    {
      ret = 100.0f * (float)pCycleStats->last.sum / (float)s_correctedTotalCycles(pCycleStats, true);
    }
    else if (pCycleStats->current.count >= CYCLES_COUNT_MIN_FOR_MEASURE)
    {
      ret = 100.0f * (float)pCycleStats->current.sum / (float)s_correctedTotalCycles(pCycleStats, false);
    }
    else
    {
      /* MISRAC: do nothing */
    }
    CYCLES_CNT_ENABLE_IRQ();
  }

  return ret;
}


/**
  * @brief  Gets current standard deviation of CPU load in %
  * @param  pCycleStats: Handler for cycles statistics
  * @param  measureType: last complete measure (if available) or current on-going measure
  * @retval current standard deviation of CPU load in %
  */
float cycleMeasure_stdDevCpuLoadPcent(CycleStatsTypeDef const *const pCycleStats, CycleMeasureType_t const measureType)
{
  float ret              = 0.0f;
  bool  ok               = true;
  float sumFloat         = 0.0f;
  float sum2Float        = 0.0f;
  float countFloat       = 1.0f;
  float totalCyclesFloat = 1.0f;

  if (pCycleStats != NULL)
  {
    CYCLES_CNT_DISABLE_IRQ();
    if ((measureType == CYCLES_LAST_MEASURE) && (pCycleStats->last.count >= CYCLES_COUNT_MIN_FOR_MEASURE))
    {
      sumFloat         = (float)pCycleStats->last.sum;
      sum2Float        = (float)pCycleStats->last.sum2;
      countFloat       = (float)pCycleStats->last.count;
      totalCyclesFloat = (float)s_correctedTotalCycles(pCycleStats, true);
    }
    else if (pCycleStats->current.count >= CYCLES_COUNT_MIN_FOR_MEASURE)
    {
      sumFloat         = (float)pCycleStats->current.sum;
      sum2Float        = (float)pCycleStats->current.sum2;
      countFloat       = (float)pCycleStats->current.count;
      totalCyclesFloat = (float)s_correctedTotalCycles(pCycleStats, false);
    }
    else
    {
      ok = false;
    }
    CYCLES_CNT_ENABLE_IRQ();
  }
  else
  {
    ok = false;
  }

  if (ok)
  {
    float const tmp = (sum2Float * countFloat) - (sumFloat * sumFloat);

    ret = (tmp <= 0.0f) ? 0.f : (100.0f * sqrtf(tmp) / totalCyclesFloat); /*cstat !MISRAC2012-Rule-22.8 no issue with sqrtf of a positive value => errno check is useless*/
  }

  return ret;
}


void cycleMeasure_getCyclesMgntStats(CycleStatsTypeDef const *const pCycleStats, cycles_stats_t *const pTaskCyclesStats, CycleMeasureType_t const measureType)
{
  bool     ok          = true;
  uint64_t totalCycles = 1ULL;
  uint32_t count       = 1UL;
  uint32_t min         = 0UL;
  uint32_t max         = 0UL;
  uint64_t sum         = 0ULL;
  uint64_t sum2        = 0ULL;

  if (pCycleStats != NULL)
  {
    CYCLES_CNT_DISABLE_IRQ();
    if ((measureType == CYCLES_LAST_MEASURE) && (pCycleStats->last.count >= CYCLES_COUNT_MIN_FOR_MEASURE))
    {
      totalCycles = s_correctedTotalCycles(pCycleStats, true);
      count       = pCycleStats->last.count;
      min         = pCycleStats->last.min;
      max         = pCycleStats->last.max;
      sum         = pCycleStats->last.sum;
      sum2        = pCycleStats->last.sum2;
    }
    else if (pCycleStats->current.count >= CYCLES_COUNT_MIN_FOR_MEASURE)
    {
      totalCycles = s_correctedTotalCycles(pCycleStats, false);
      count       = pCycleStats->current.count;
      min         = pCycleStats->current.min;
      max         = pCycleStats->current.max;
      sum         = pCycleStats->current.sum;
      sum2        = pCycleStats->current.sum2;
    }
    else
    {
      ok = false;
    }
    CYCLES_CNT_ENABLE_IRQ();
  }
  else
  {
    ok = false;
  }

  if (ok)
  {
    float const totalCyclesFloat     = (float)totalCycles;
    float const minFloat             = (float)min;
    float const maxFloat             = (float)max;
    float const sumFloat             = (float)sum;
    float const sum2Float            = (float)sum2;
    float const countFloat           = (float)count;
    float const tmp1                 = (sum2Float * countFloat) - (sumFloat * sumFloat);
    float const tmp2                 = (tmp1 <= 0.0f) ? 0.0f : (sqrtf(tmp1) / totalCyclesFloat); /*cstat !MISRAC2012-Rule-22.8 no issue with sqrtf of a positive value => errno check is useless*/
    float const stdDevHzFloat        = tmp2 * (float)SystemCoreClock;
    float const stdDevCpuLoadPcent   = tmp2 * 100.0f;

    pTaskCyclesStats->minCpuLoadHz        = (uint32_t)(((uint64_t)SystemCoreClock * (uint64_t)min * (uint64_t)count) / totalCycles);
    pTaskCyclesStats->maxCpuLoadHz        = (uint32_t)(((uint64_t)SystemCoreClock * (uint64_t)max * (uint64_t)count) / totalCycles);
    pTaskCyclesStats->averageCpuLoadHz    = (uint32_t)(((uint64_t)SystemCoreClock * sum) / totalCycles);
    pTaskCyclesStats->stdDevCpuLoadHz     = (uint32_t)stdDevHzFloat;
    pTaskCyclesStats->minCpuLoadPcent     = 100.0f * minFloat * countFloat / totalCyclesFloat;
    pTaskCyclesStats->maxCpuLoadPcent     = 100.0f * maxFloat * countFloat / totalCyclesFloat;
    pTaskCyclesStats->averageCpuLoadPcent = 100.0f * sumFloat / totalCyclesFloat;
    pTaskCyclesStats->stdDevCpuLoadPcent  = stdDevCpuLoadPcent;
  }
  else
  {
    memset(pTaskCyclesStats, 0, sizeof(cycles_stats_t));
  }
}


__weak void cycleMeasure_displayCpuLoadOnUart(CycleStatsTypeDef const *const pCycleStats)
{
  static const char *const taskName[] =
  {
    [DATAINOUT_TASK]       = "dataInOut",
    [PROCESS_TASK]         = "process",
    [PROCESSLOWLEVEL_TASK] = "processLowLevel",
    [CONTROL_TASK]         = "control"
  };
  uint32_t const averageCpuLoadHz  = cycleMeasure_averageCpuLoadHz(pCycleStats, CYCLES_LAST_MEASURE);
  uint32_t const minCpuLoadHz      = cycleMeasure_minCpuLoadHz(pCycleStats,     CYCLES_LAST_MEASURE);
  uint32_t const maxCpuLoadHz      = cycleMeasure_maxCpuLoadHz(pCycleStats,     CYCLES_LAST_MEASURE);
  uint32_t const averageCpuLoadMHz = averageCpuLoadHz / 1000000UL;
  uint32_t const minCpuLoadMHz     = minCpuLoadHz     / 1000000UL;
  uint32_t const maxCpuLoadMHz     = maxCpuLoadHz     / 1000000UL;
  #ifdef CYCLES_CNT_TRACES_USED
  trace_print(TRACE_OUTPUT_UART, TRACE_LVL_DEBUG, "%-50s %-15s:  %3d.%06d MHz (min %3d.%06d, max %3d.%06d)\n",
              (pCycleStats->pName == NULL) ? "???" : pCycleStats->pName,
              taskName[pCycleStats->task],
              averageCpuLoadMHz, averageCpuLoadHz - (averageCpuLoadMHz * 1000000UL),
              minCpuLoadMHz,     minCpuLoadHz     - (minCpuLoadMHz     * 1000000UL),
              maxCpuLoadMHz,     maxCpuLoadHz     - (maxCpuLoadMHz     * 1000000UL));
  #elif defined(CYCLES_CNT_PRINTF_USED)
  printf("%-50s %-15s:  %3d.%06d MHz (min %3d.%06d, max %3d.%06d)\n",
         (pCycleStats->pName == NULL) ? "???" : pCycleStats->pName,
         taskName[pCycleStats->task],
         averageCpuLoadMHz, averageCpuLoadHz - (averageCpuLoadMHz * 1000000UL),
         minCpuLoadMHz,     minCpuLoadHz     - (minCpuLoadMHz     * 1000000UL),
         maxCpuLoadMHz,     maxCpuLoadHz     - (maxCpuLoadMHz     * 1000000UL));
  #endif
}


static uint64_t s_correctedTotalCycles(CycleStatsTypeDef const *const pCycleStats, bool const last)
{
  uint64_t ret = 0ULL;

  if (last)
  {
    // pCycleStats->last.sum         contains pCycleStats->last.count activity cycles periods
    // pCycleStats->last.totalCycles contains pCycleStats->last.count activity cycles periods + pCycleStats->last.count-1 inactivity cycles periods
    // thus 1 inactivity  cyclesperiod is missing to obtain a relevant statistic
    // thus pCycleStats->last.totalCycles must be corrected
    ret  = pCycleStats->last.totalCycles - pCycleStats->last.sum;         // inactivity cycles
    ret *= (uint64_t)pCycleStats->last.count;
    ret /= (uint64_t)pCycleStats->last.count - 1ULL;                      // corrected inactivity cycles (assumes pCycleStats->current.count >= 2; true thanks to CYCLES_COUNT_MIN_FOR_MEASURE)
    ret += pCycleStats->last.sum;                                         // corrected total cycles
  }
  else if (pCycleStats->nextState == START_CYCLES)
  {
    // pCycleStats->current.sum         contains pCycleStats->current.count activity cycles periods
    // pCycleStats->current.totalCycles contains pCycleStats->current.count activity cycles periods + pCycleStats->current.count-1 inactivity cycles periods
    // thus 1 inactivity cycles period is missing to obtain a relevant statistic
    // thus pCycleStats->current.totalCycles must be corrected
    ret  = pCycleStats->current.totalCycles - pCycleStats->current.sum;   // inactivity cycles
    ret *= (uint64_t)pCycleStats->current.count;
    ret /= (uint64_t)pCycleStats->current.count - 1ULL;                   // corrected inactivity cycles (assumes pCycleStats->current.count >= 2; true thanks to CYCLES_COUNT_MIN_FOR_MEASURE)
    ret += pCycleStats->current.sum;                                      // corrected total cycles
  }
  else
  {
    // pCycleStats->current.sum         contains pCycleStats->current.count activity cycles periods
    // pCycleStats->current.totalCycles contains pCycleStats->current.count activity cycles periods + pCycleStats->current.count inactivity cycles periods
    // thus no pCycleStats->current.totalCycles correction needed
    ret = pCycleStats->current.totalCycles;
  }

  return ret;
}


static void s_trace(traceLvl_t const level, const char *pStr)
{
  #ifdef CYCLES_CNT_TRACES_USED
  trace_print(TRACE_OUTPUT_UART, level, pStr);
  #elif defined(CYCLES_CNT_PRINTF_USED)
  printf(pStr);
  #endif
}


#ifdef DEBUG_CYCLES
static void s_debugCyclesFunc(CycleStatsTypeDef *const pCycleStats, uint32_t const time, uint8_t const cmd)
{
  s_debugCycles[s_idDebugCycles].pName       = pCycleStats->pName;
  s_debugCycles[s_idDebugCycles].cmd         = cmd;
  s_debugCycles[s_idDebugCycles].time        = DWT->CYCCNT;
  s_debugCycles[s_idDebugCycles].deepness    = (uint8_t)nbParallelCyclesMeasure;
  //s_debugCycles[s_idDebugCycles].pCycleStats = pCycleStats;
  s_idDebugCycles                            = (s_idDebugCycles == (DEBUG_CYCLES_SIZE - 1UL)) ? 0UL : (s_idDebugCycles + 1UL);
}
#endif
