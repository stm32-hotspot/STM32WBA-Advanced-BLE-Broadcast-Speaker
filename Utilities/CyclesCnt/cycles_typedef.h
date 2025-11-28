/**
  ******************************************************************************
  * @file    cycles_typedef.h
  * @author  MCD Application Team
  * @brief   Header for cycles.c module
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CYCLES_TYPEDEF_H
#define __CYCLES_TYPEDEF_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>


/* Exported define -----------------------------------------------------------*/
#define CYCLES_MEASURE_DEFAULT_TIMEOUT_MS 200UL /* see cycleMeasure_Stop routine below*/

/* Exported types ------------------------------------------------------------*/
typedef enum
{
  CYCLES_LAST_MEASURE,    // last complete measure
  CYCLES_CURRENT_MEASURE  // current on-going measure
} CycleMeasureType_t;

typedef struct
{
  enum {INTERRUPT, DATAINOUT_TASK, PROCESS_TASK, PROCESSLOWLEVEL_TASK, CONTROL_TASK, OTHER_TASK} task;
  enum {START_CYCLES, STOP_CYCLES}                                                               nextState;
  uint32_t    lastCycles;
  uint32_t    cyclesToRemove;
  struct
  {
    uint64_t  totalCycles;
    uint32_t  count;
    uint32_t  min;
    uint32_t  max;
    uint64_t  sum;
    uint64_t  sum2;
  } last, current;
  char const *pName;
  void       *pUserCookie;
} CycleStatsTypeDef;

typedef void (CycleStatsCb_t)(CycleStatsTypeDef const *const pCycleStats);

typedef struct
{
  CycleStatsTypeDef  stats;
  uint32_t           measureTimeoutMs;
  uint32_t           cbTimeoutMs;
  uint32_t           lastCbTime;
  CycleStatsCb_t    *cb;
} CycleStats_Mgnt_t;

typedef struct
{
  uint32_t averageCpuLoadHz;
  uint32_t minCpuLoadHz;
  uint32_t maxCpuLoadHz;
  uint32_t stdDevCpuLoadHz;
  float    minCpuLoadPcent;
  float    maxCpuLoadPcent;
  float    averageCpuLoadPcent;
  float    stdDevCpuLoadPcent;
} cycles_stats_t;

/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */



#ifdef __cplusplus
}
#endif

#endif /* __CYCLES_TYPEDEF_H */
