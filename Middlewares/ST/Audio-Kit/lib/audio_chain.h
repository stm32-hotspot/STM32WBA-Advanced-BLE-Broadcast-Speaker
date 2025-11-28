/**
******************************************************************************
* @file    audio_chain.h
* @author  MCD Application Team
* @brief   Header for audio_chain.c module
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
#ifndef __AUDIO_CHAIN_H
#define __AUDIO_CHAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "cycles_typedef.h"
#include "traces_typedef.h"
#include "audio_algo_list.h"
#include "audio_chain_version.h"
#include "audio_mem_mgnt.h"


#define AUDIO_INSTANCE_SIGNATURE               0x54534E49U /* 'INST' */

/* Exported types ------------------------------------------------------------*/
typedef struct
{
  void (* init)(bool const logTaskQueueLevels);
  void (* deinit)(void);
  void (* trigger)(void);
} audio_chain_task_cbs_t;

typedef struct
{
  CycleStats_Mgnt_t      cycleMgnt;
  audio_chain_task_cbs_t cbs;
} audio_chain_task_t;

typedef struct audio_chain_sys_connection_conf
{
  audio_chunk_t  *pSysChunk;
  audio_buffer_t *pAudioBuffer;
  const char     *pDescription;
  const char     *pName;
  const char     *pVarName;
} audio_chain_sys_connection_conf_t;

typedef struct audio_chain_sys_connections
{
  uint8_t                            nb;
  audio_chain_sys_connection_conf_t *pConf;
} audio_chain_sys_connections_t;

typedef struct
{
  audio_chain_task_t                 dataInOut;
  audio_chain_task_t                 process;
  audio_chain_task_t                 process_lowlevel;               // remark: process_lowlevel cbs are not used (process cbs are used instead), only cycleMgnt is used
  audio_chain_task_t                 control;
} audio_chain_tasks_t;

typedef struct audio_chain
{
  uint32_t                           signature;
  char const                        *pName;
  char const                        *pDesc;
  void                              *pInternalMem;
} audio_chain_t;

/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
int32_t                               AudioChain_init(audio_chain_t                                     *const pHdle);
int32_t                               AudioChain_deinit(audio_chain_t                                   *const pHdle);
int32_t                               AudioChain_initGraph(audio_chain_t                                *const pHdle);
int32_t                               AudioChain_deinitGraph(audio_chain_t                              *const pHdle);
int32_t                               AudioChain_initTuning(audio_chain_t                               *const pHdle);
int32_t                               AudioChain_deinitTuning(audio_chain_t                             *const pHdle);
int32_t                               AudioChain_deinitGraphRequest(audio_chain_t                       *const pHdle);
audio_chain_utilities_t              *AudioChain_getUtilsHdle(audio_chain_t                             *const pHdle); /* Todo; check we can remove it from chunk init */
void                                  AudioChain_registerUtils(audio_chain_t                            *const pHdle, audio_chain_utilities_t *const pUtilsHandle);
void                                  AudioChain_setMaxReinitLoadPcent(audio_chain_t                    *const pHdle, uint8_t const pcent);
uint32_t                              AudioChain_getMaxAlgoReinitStepCycles(audio_chain_t               *const pHdle);
void                                  AudioChain_registerWarningMsgCb(audio_chain_t                     *const pHdle, void (*const warningMsgCb)(const char *const pMsg));
void                                  AudioChain_registerUpdateCfgMsgCb(audio_chain_t                   *const pHdle, void (*const updateCfgMsgCb)(const char *const pMsg));
void                                  AudioChain_registerMutexCbs(audio_chain_t                         *const pHdle, void (*const mutexLock)(void), void (*const mutexUnlock)(void));
void                                  AudioChain_setFrameAndRunDurations(audio_chain_t                  *const pHdle, uint32_t const frameNs, uint32_t const runNs);
void                                  AudioChain_setDataInOutTaskCycleMgntCbTimeout(audio_chain_t       *const pHdle, CycleStatsCb_t *const cb, uint32_t const measureTimeoutMs, uint32_t const cbTimeoutMs);
void                                  AudioChain_setProcessTaskCycleMgntCbTimeout(audio_chain_t         *const pHdle, CycleStatsCb_t *const cb, uint32_t const measureTimeoutMs, uint32_t const cbTimeoutMs);
void                                  AudioChain_setProcessLowLevelTaskCycleMgntCbTimeout(audio_chain_t *const pHdle, CycleStatsCb_t *const cb, uint32_t const measureTimeoutMs, uint32_t const cbTimeoutMs);
void                                  AudioChain_setControlTaskCycleMgntCbTimeout(audio_chain_t         *const pHdle, CycleStatsCb_t *const cb, uint32_t const measureTimeoutMs, uint32_t const cbTimeoutMs);
CycleStatsTypeDef                    *AudioChain_getDataInOutTaskCycleMgntStats(audio_chain_t           *const pHdle);
CycleStatsTypeDef                    *AudioChain_getProcessTaskCycleMgntStats(audio_chain_t             *const pHdle);
CycleStatsTypeDef                    *AudioChain_getProcessLowLevelTaskCycleMgntStats(audio_chain_t     *const pHdle);
CycleStatsTypeDef                    *AudioChain_getControlTaskCycleMgntStats(audio_chain_t             *const pHdle);
void                                  AudioChain_registerTasks(audio_chain_t                            *const pHdle, audio_chain_tasks_t *const pTasksHandle);
bool                                  AudioChain_isInitialized(audio_chain_t                            *const pHdle);
bool                                  AudioChain_isStarted(audio_chain_t                                *const pHdle);
bool                                  AudioChain_isDeinitGraphRequested(audio_chain_t                   *const pHdle);
uint64_t                              AudioChain_getInterruptNb(audio_chain_t                           *const pHdle);
uint8_t                               AudioChain_getNbAlgos(audio_chain_t                               *const pHdle);
audio_capability_prio_level_t         AudioChain_getPrioLevel(audio_chain_t                             *const pHdle);
audio_algo_t                         *AudioChain_getCurrentAlgo(audio_chain_t                           *const pHdle);
int32_t                               AudioChain_process(audio_chain_t                                  *const pHdle, audio_capability_prio_level_t const prio_level);
int32_t                               AudioChain_control(audio_chain_t                                  *const pHdle);
int32_t                               AudioChain_dataInOut(audio_chain_t                                *const pHdle);

audio_algo_t                         *AudioChain_getAlgoFromId(audio_chain_t                            *const pHdle, uint8_t const algoId);
audio_algo_list_t                    *AudioChain_getAlgosList(audio_chain_t                             *const pHdle);
void                                  AudioChain_run(audio_chain_t                                      *const pHdle);
void                                  AudioChain_error(audio_chain_t                                    *const pHdle, const char *pFile, int const line, const char *pErrorMsg);
void                                  AudioChain_trace(audio_chain_t                                    *const pHdle, const char *const pCompName, traceLvl_t const level, const char *pFile, int const line, ...);
void                                  AudioChain_trace_args(audio_chain_t                               *const pHdle, const char *const pCompName, traceLvl_t const level, const char *pFile, int const line, char *const pFormat, va_list args);
int32_t                               AudioChain_registerAlgos(audio_chain_t                            *const pHdle, ...);
int32_t                               AudioChain_getChunkAlgoOut(audio_chain_t                          *const pHdle, audio_chunk_t *const pChunk, audio_algo_t **const ppAlgo, uint8_t *const pChunkID);
int32_t                               AudioChain_getChunkAlgoIn(audio_chain_t                           *const pHdle, audio_chunk_t *const pChunk, audio_algo_t **const ppAlgo, uint8_t *const pChunkID);
bool                                  AudioChain_setEnableCyclesCnt(audio_chain_t                       *const pHdle, bool const enable);
bool                                  AudioChain_getCyclesCntStatus(audio_chain_t                       *const pHdle);
bool                                  AudioChain_setEnableTraces(audio_chain_t                          *const pHdle, bool const enable);
bool                                  AudioChain_getTracesStatus(audio_chain_t                          *const pHdle);
audio_chain_sys_connections_t const  *AudioChain_getSysConnection(audio_chain_t                         *const pHdle, audio_chunk_type_t chunkType);
void                                  AudioChain_setLogInit(audio_chain_t                               *const pHdle, bool const logInit);
void                                  AudioChain_setLogCmsisOs(audio_chain_t                            *const pHdle, bool const logCmsisOs);
int32_t                               AudioChain_configPendingChunks(audio_chain_t                      *const pHdle);
int32_t                               AudioChain_dumpAlgoDescriptor(audio_chain_t                       *const pHdle, const char *const pName);
int32_t                               AudioChain_dumpFactoryEntry(audio_chain_t                         *const pHdle);
void                                  AudioChain_dumpParamDescriptor(audio_chain_t                      *const pHdle, const audio_descriptor_params_t *pParamTemplate, void *const pDataConfig, const char *const pTypeName);

// for algos' mem usage statistics
void                                  AudioChain_setBuffersMallocStatsPtr(audio_chain_t                 *const pHdle);
void                                  AudioChain_setChunksMallocStatsPtr(audio_chain_t                  *const pHdle);
memAllocStat_t                       *AudioChain_getBuffersMallocStatsPtr(audio_chain_t                 *const pHdle);
memAllocStat_t                       *AudioChain_getChunksMallocStatsPtr(audio_chain_t                  *const pHdle);

#ifdef __cplusplus
}
#endif

#endif  /* __AUDIO_CHAIN_H */
