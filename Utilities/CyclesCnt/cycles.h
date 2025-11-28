/**
  ******************************************************************************
  * @file    cycles.h
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
#ifndef __CYCLES_H
#define __CYCLES_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include "cycles_conf.h"
#include "cycles_typedef.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
/* to be called only once for all measures */
void cycleMeasure_Init(void);

/* reset pCycleStats measure current stats */
void cycleMeasure_FullReset(CycleStatsTypeDef *const pCycleStats);

/* reset pCycleStats measure current & last stats */
void cycleMeasure_Reset(CycleStatsTypeDef *const pCycleStats);

/* start measure for pCycleStats measure context */
void cycleMeasure_Start(CycleStatsTypeDef *const pCycleStats);

/* stop measure and update measure statistics for pCycleStats measure context
*  returns 1 if time duration since last cycleMeasure_Reset() call >= timeout
*  else    0
*/
bool cycleMeasure_Stop(CycleStatsTypeDef *const pCycleStats, uint32_t const timeoutMs);

/* returns if time duration since last cycleMeasure_Reset() call >= timeout, else 0 */
bool cycleMeasure_isTimeoutExpired(CycleStatsTypeDef *const pCycleStats, uint32_t const timeoutMs);

uint32_t cycleMeasure_getSystemCoreClock(void);
uint32_t cycleMeasure_currentCycles(void);
uint32_t cycleMeasure_minCycles(CycleStatsTypeDef            const *const pCycleStats, CycleMeasureType_t const measureType);
uint32_t cycleMeasure_maxCycles(CycleStatsTypeDef            const *const pCycleStats, CycleMeasureType_t const measureType);
uint32_t cycleMeasure_averageCycles(CycleStatsTypeDef        const *const pCycleStats, CycleMeasureType_t const measureType);
uint32_t cycleMeasure_stdDevCycles(CycleStatsTypeDef         const *const pCycleStats, CycleMeasureType_t const measureType);
uint32_t cycleMeasure_minCpuLoadHz(CycleStatsTypeDef         const *const pCycleStats, CycleMeasureType_t const measureType);
uint32_t cycleMeasure_maxCpuLoadHz(CycleStatsTypeDef         const *const pCycleStats, CycleMeasureType_t const measureType);
uint32_t cycleMeasure_averageCpuLoadHz(CycleStatsTypeDef     const *const pCycleStats, CycleMeasureType_t const measureType);
uint32_t cycleMeasure_averageCpuLoadMHz(CycleStatsTypeDef    const *const pCycleStats, CycleMeasureType_t const measureType);
uint32_t cycleMeasure_stdDevCpuLoadHz(CycleStatsTypeDef      const *const pCycleStats, CycleMeasureType_t const measureType);
float    cycleMeasure_minCpuLoadPcent(CycleStatsTypeDef      const *const pCycleStats, CycleMeasureType_t const measureType);
float    cycleMeasure_maxCpuLoadPcent(CycleStatsTypeDef      const *const pCycleStats, CycleMeasureType_t const measureType);
float    cycleMeasure_averageCpuLoadPcent(CycleStatsTypeDef  const *const pCycleStats, CycleMeasureType_t const measureType);
float    cycleMeasure_stdDevCpuLoadPcent(CycleStatsTypeDef   const *const pCycleStats, CycleMeasureType_t const measureType);
void     cycleMeasure_getCyclesMgntStats(CycleStatsTypeDef   const *const pCycleStats, cycles_stats_t *const pTaskCyclesStats, CycleMeasureType_t const measureType);
void     cycleMeasure_displayCpuLoadOnUart(CycleStatsTypeDef const *const pCycleStats);

#ifdef __cplusplus
}
#endif

#endif /* __CYCLES_H */
