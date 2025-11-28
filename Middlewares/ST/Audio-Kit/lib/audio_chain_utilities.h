/**
******************************************************************************
* @file    audio_chain_utilities.h
* @author  MCD Application Team
* @brief   Header for audio_chain_utilities.c module
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __AUDIO_CHAIN_UTILITIES_H
#define __AUDIO_CHAIN_UTILITIES_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "cycles_typedef.h"
#include "traces_typedef.h"

/* Exported types ------------------------------------------------------------*/
typedef struct
{
  uint32_t (* coreClock)(void);
  uint32_t (* currentCycles)(void);
  void (* init)(void);
  void (* reset)(CycleStatsTypeDef            *const pCycleStats);
  void (* fullReset)(CycleStatsTypeDef        *const pCycleStats);
  void (* start)(CycleStatsTypeDef            *const pCycleStats);
  bool (* stop)(CycleStatsTypeDef             *const pCycleStats, uint32_t const timeoutMs);
  bool (* isTimeoutExpired)(CycleStatsTypeDef *const pCycleStats, uint32_t const timeoutMs);
} audio_chain_cycleStats_cbs_t;

typedef struct
{
  bool                          logInit;
  bool                          logTaskQueueLevels;
  bool                          enableCyclesCnt;
  bool                          enableTraces;
  bool                          lockFwOnAssert;
  struct audio_chain           *pChain;
  audio_chain_cycleStats_cbs_t  cyclesCbs;
  trace_t                      *trace;
  uint64_t (* getNbMsFromStart)(void);
  void (* warningMsgCb)(const char *const pMsg);
  void (* updateCfgMsgCb)(const char *const pMsg);
  void (* error)(void);
  void (* mutexLock)(void);
  void (* mutexUnlock)(void);
} audio_chain_utilities_t;

/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void AudioChainUtils_trace_args(audio_chain_utilities_t                *const pUtilsHandle, const char *const pCompName, traceLvl_t const level, const char *const pFile, int const line, const char *const pFormat, va_list args);
void AudioChainUtils_initCyclesCount(audio_chain_utilities_t           *const pUtilsHandle);
void AudioChainUtils_startCyclesCount(audio_chain_utilities_t          *const pUtilsHandle, CycleStats_Mgnt_t *const pCycleStats);
void AudioChainUtils_stopCyclesCount(audio_chain_utilities_t           *const pUtilsHandle, CycleStats_Mgnt_t *const pCycleStats);
void AudioChainUtils_resetCyclesCount(audio_chain_utilities_t          *const pUtilsHandle, CycleStats_Mgnt_t *const pCycleStats);
void AudioChainUtils_resetCyclesCountOnTimeout(audio_chain_utilities_t *const pUtilsHandle, CycleStats_Mgnt_t *const pCycleStats);
void AudioChainUtils_assert(audio_chain_utilities_t                    *const pUtilsHandle, bool const cond, char const *const condString, char const *const file, int const line, char const *const pFormat, ...);
void AudioChainUtils_error(audio_chain_utilities_t                     *const pUtilsHandle);
bool AudioChainUtils_setEnableCyclesCnt(audio_chain_utilities_t        *const pUtilsHandle, bool const enable);
bool AudioChainUtils_getCyclesCntStatus(audio_chain_utilities_t        *const pUtilsHandle);
bool AudioChainUtils_setEnableTraces(audio_chain_utilities_t           *const pUtilsHandle, bool const enable);
bool AudioChainUtils_getTracesStatus(audio_chain_utilities_t           *const pUtilsHandle);
void AudioChainUtils_sendWarningMsg(audio_chain_utilities_t            *const pUtilsHandle, const char *pFormat, ...);


#ifdef __cplusplus
}
#endif

#endif  /* __AUDIO_CHAIN_UTILITIES_H */


