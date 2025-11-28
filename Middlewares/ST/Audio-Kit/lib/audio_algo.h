/**
******************************************************************************
* @file    audio_algo.h
* @author  MCD Application Team
* @brief   Header for audio_algo.c module
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
#ifndef __AUDIO_ALGO_H
#define __AUDIO_ALGO_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include "audio_chain_factory.h"
#include "audio_chunk_list.h"

/* Exported constants --------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
int32_t                       AudioAlgo_config(audio_algo_t                               *const pAlgo,
                                               audio_algo_state_t                          const state,
                                               const char                                 *const pDesc,
                                               void                                       *const pStaticConfig,
                                               void                                       *const pDynamicConfig,
                                               const audio_algo_factory_t                 *const pFactory,
                                               uint32_t                                    const measureTimeoutMs,
                                               uint32_t                                    const cbTimeoutMs,
                                               CycleStatsCb_t                             *const cycleMgntCb,
                                               memPool_t                                   const memPool);

int32_t                       AudioAlgo_create(audio_algo_t                              **const ppAlgo,
                                               const char                                 *const pName,
                                               audio_algo_state_t                          const state,
                                               const char                                 *const pDesc,
                                               uint32_t                                    const measureTimeoutMs,
                                               uint32_t                                    const cbTimeoutMs,
                                               CycleStatsCb_t                             *const cycleMgntCb,
                                               memPool_t                                   const memPool);
int32_t                       AudioAlgo_setCycleMgntCb(audio_algo_t                       *const pAlgo,
                                                       uint32_t                            const measureTimeoutMs,
                                                       uint32_t                            const cbTimeoutMs,
                                                       CycleStatsCb_t                     *const cycleMgntCb);
bool                          AudioAlgo_isInitialized(audio_algo_t                        *const pAlgo);
audio_algo_state_t            AudioAlgo_getState(audio_algo_t                             *const pAlgo);
void                          AudioAlgo_setState(audio_algo_t                             *const pAlgo, audio_algo_state_t const state);
bool                          AudioAlgo_isActive(audio_algo_t                             *const pAlgo);
int32_t                       AudioAlgo_resetConfig(audio_algo_t                          *const pAlgo);

void                          AudioAlgo_enable(audio_algo_t                               *const pAlgo);
void                          AudioAlgo_setInstanceAllocated(audio_algo_t                 *const pAlgo, bool const state);
void                          AudioAlgo_disable(audio_algo_t                              *const pAlgo);
uint8_t                       AudioAlgo_getIndex(audio_algo_t                             *const pAlgo);
void                          AudioAlgo_setManualPtrUpdate(audio_algo_t                   *const pAlgo);

int32_t                       AudioAlgo_addInput(audio_algo_t                             *const pAlgo, audio_chunk_t *const pChunk);
int32_t                       AudioAlgo_addOutput(audio_algo_t                            *const pAlgo, audio_chunk_t *const pChunk);

int32_t                       AudioAlgo_setInput(audio_algo_t                             *const pAlgo, audio_chunk_t *const pChunk, int const chunkId);
int32_t                       AudioAlgo_setOutput(audio_algo_t                            *const pAlgo, audio_chunk_t *const pChunk, int const chunkId);

void                         *AudioAlgo_getStaticConfig(audio_algo_t                      *const pAlgo);  // to be used outside of algos' check consistency routines
void                         *AudioAlgo_getDynamicConfig(audio_algo_t                     *const pAlgo);  // to be used outside of algos' check consistency routines
void                         *AudioAlgo_getStaticConfig4Check(audio_algo_t                *const pAlgo);  // to be used inside  of algos' check consistency routines
void                         *AudioAlgo_getDynamicConfig4Check(audio_algo_t               *const pAlgo);  // to be used inside  of algos' check consistency routines
void                         *AudioAlgo_getCtrlData(audio_algo_t                          *const pAlgo);
void                          AudioAlgo_setCtrlData(audio_algo_t                          *const pAlgo, void *const pCtrl);
audio_algo_cb_t               AudioAlgo_getCtrlCb(audio_algo_t                            *const pAlgo);
void                          AudioAlgo_setCtrlCb(audio_algo_t                            *const pAlgo, audio_algo_cb_t const cb);
int32_t                       AudioAlgo_getControlVarPtr(audio_algo_t                     *const pAlgo, const char *pKey, void **const ppOutParam);
int32_t                       AudioAlgo_setControlParam(audio_algo_t                      *const pAlgo, const char *pKey, va_list args);
int32_t                       AudioAlgo_getControlParam(audio_algo_t                      *const pAlgo, const char *pKey, void *const pParam);
const audio_algo_factory_t   *AudioAlgo_getFactory(audio_algo_t                           *const pAlgo);
int32_t                       AudioAlgo_checkCapabilityConsistency(audio_algo_t           *const pAlgo);

void                          AudioAlgo_setDataInOutCyclesMgntCb(audio_algo_t             *const pAlgo, CycleStatsCb_t *const pCb);
void                          AudioAlgo_setProcessCyclesMgntCb(audio_algo_t               *const pAlgo, CycleStatsCb_t *const pCb);
void                          AudioAlgo_setControlCyclesMgntCb(audio_algo_t               *const pAlgo, CycleStatsCb_t *const pCb);
CycleStatsCb_t               *AudioAlgo_getDataInOutCyclesMgntCb(audio_algo_t             *const pAlgo);
CycleStatsCb_t               *AudioAlgo_getProcessCyclesMgntCb(audio_algo_t               *const pAlgo);
CycleStatsCb_t               *AudioAlgo_getControlCyclesMgntCb(audio_algo_t               *const pAlgo);
CycleStatsTypeDef            *AudioAlgo_getDataInOutCyclesMgntStats(audio_algo_t          *const pAlgo);
CycleStatsTypeDef            *AudioAlgo_getProcessCyclesMgntStats(audio_algo_t            *const pAlgo);
CycleStatsTypeDef            *AudioAlgo_getControlCyclesMgntStats(audio_algo_t            *const pAlgo);
void                          AudioAlgo_setDataInOutCyclesMgntMeasureTimeout(audio_algo_t *const pAlgo, uint32_t const measureTimeoutMs);
void                          AudioAlgo_setProcessCyclesMgntMeasureTimeout(audio_algo_t   *const pAlgo, uint32_t const measureTimeoutMs);
void                          AudioAlgo_setControlCyclesMgntMeasureTimeout(audio_algo_t   *const pAlgo, uint32_t const measureTimeoutMs);
void                          AudioAlgo_setDataInOutCyclesMgntCbTimeout(audio_algo_t      *const pAlgo, uint32_t const cbTimeoutMs);
void                          AudioAlgo_setProcessCyclesMgntCbTimeout(audio_algo_t        *const pAlgo, uint32_t const cbTimeoutMs);
void                          AudioAlgo_setControlCyclesMgntCbTimeout(audio_algo_t        *const pAlgo, uint32_t const cbTimeoutMs);
uint32_t                      AudioAlgo_getDataInOutCyclesMgntMeasureTimeout(audio_algo_t *const pAlgo);
uint32_t                      AudioAlgo_getProcessCyclesMgntMeasureTimeout(audio_algo_t   *const pAlgo);
uint32_t                      AudioAlgo_getControlCyclesMgntMeasureTimeout(audio_algo_t   *const pAlgo);
uint32_t                      AudioAlgo_getDataInOutCyclesMgntCbTimeout(audio_algo_t      *const pAlgo);
uint32_t                      AudioAlgo_getProcessCyclesMgntCbTimeout(audio_algo_t        *const pAlgo);
uint32_t                      AudioAlgo_getControlCyclesMgntCbTimeout(audio_algo_t        *const pAlgo);
int32_t                       AudioAlgo_setCommonDescConfig(audio_algo_t                  *const pAlgo, const audio_algo_descriptor_t *const pCommonDesc, const char *const pKey, void *const pParam);
int32_t                       AudioAlgo_setColdConfig(audio_algo_t                        *const pAlgo, const char *pKey, const char *pValue, char **ppErrorString);
int32_t                       AudioAlgo_setHotConfig(audio_algo_t                         *const pAlgo, const char *pKey, const char *pValue, char **ppErrorString);
int32_t                       AudioAlgo_setColdConfigPtr(audio_algo_t                     *const pAlgo, const char *pKey, const void *pValue, char **ppErrorString);

void                          AudioAlgo_setWrapperContext(audio_algo_t                    *const pAlgo, void *pWrapperContext);
void                         *AudioAlgo_getWrapperContext(audio_algo_t                    *const pAlgo);
audio_chain_utilities_t      *AudioAlgo_getUtilsHdle(audio_algo_t                         *const pAlgo);
int32_t                       AudioAlgo_setUtilsHdle(audio_algo_t                         *const pAlgo, audio_chain_utilities_t *const pUtilsHandle);

audio_chunk_list_t           *AudioAlgo_getChunksIn(audio_algo_t                          *const pAlgo);
audio_chunk_list_t           *AudioAlgo_getChunksOut(audio_algo_t                         *const pAlgo);
audio_chunk_t                *AudioAlgo_getChunkPtrIn(audio_algo_t                        *const pAlgo, uint8_t const chunkId);
audio_chunk_t                *AudioAlgo_getChunkPtrOut(audio_algo_t                       *const pAlgo, uint8_t const chunkId);
char const                   *AudioAlgo_getDescriptionString(audio_algo_t                 *const pAlgo);
char const                   *AudioAlgo_getFullDescriptionString(audio_algo_t             *const pAlgo);
char const                   *AudioAlgo_getInstanceName(audio_algo_t                      *const pAlgo);
audio_capability_prio_level_t AudioAlgo_getPrioLevel(audio_algo_t                         *const pAlgo);

uint8_t                       AudioAlgo_getNbFramesReady(audio_algo_t                     *const pAlgo);
void                          AudioAlgo_incReadyForProcess(audio_algo_t                   *const pAlgo);
void                          AudioAlgo_incReadyForProcessProtected(audio_algo_t          *const pAlgo);
void                          AudioAlgo_incReadyForControlProtected(audio_algo_t          *const pAlgo);
void                          AudioAlgo_incReadyForControl(audio_algo_t                   *const pAlgo);

int32_t                       AudioAlgo_requestTuningUpdate(audio_algo_t                  *const pAlgo);

void                         *AudioAlgo_getUserPointer(audio_algo_t                       *const pAlgo);
void                          AudioAlgo_setUserPointer(audio_algo_t                       *const pAlgo, void *const pUser);

void                          AudioAlgo_trace(audio_algo_t                                *const pAlgo,
                                              traceLvl_t                                   const level,
                                              const char                                  *const pFile,
                                              int                                          const line,
                                              ...);

void                         *AudioAlgo_mallocDebug(size_t const size, memPool_t const memPool, char const *const file, int const line);
void                         *AudioAlgo_callocDebug(size_t const nbElements, size_t const elementSize, memPool_t const memPool, char const *const file, int const line);
void                         *AudioAlgo_reallocDebug(void *const ptr, size_t const size, memPool_t const memPool, char const *const file, int const line);
void                          AudioAlgo_freeDebug(void *const pMemToFree, memPool_t const memPool);
#ifdef AUDIO_MEM_CONF_TRACK_MALLOC
#define AudioAlgo_malloc(size, memPool)                     AudioAlgo_mallocDebug((size), (memPool), __FILE__, __LINE__)
#define AudioAlgo_calloc(nbElements, elementSize, memPool)  AudioAlgo_callocDebug((nbElements), (elementSize), (memPool), __FILE__, __LINE__)
#define AudioAlgo_realloc(ptr, size, memPool)               AudioAlgo_reallocDebug((ptr), (size), (memPool), __FILE__, __LINE__)  /*cstat !MISRAC2012-Rule-20.7 don't know why MISRAC complains: there is no issue*/
#define AudioAlgo_free(pMemToFree, memPool)                 AudioAlgo_freeDebug((pMemToFree), (memPool))                          /*cstat !MISRAC2012-Rule-20.7 don't know why MISRAC complains: there is no issue*/
#else
void                         *AudioAlgo_malloc(size_t const size, memPool_t const memPool);
void                         *AudioAlgo_calloc(size_t const nbElements, size_t const elementSize, memPool_t const memPool);
void                         *AudioAlgo_realloc(void *const ptr, size_t const size, memPool_t const memPool);
void                          AudioAlgo_free(void *const pMemToFree, memPool_t const memPool);
#endif
void                          AudioAlgo_setAudioChainHdle(void *const pChain);
memAllocStat_t               *AudioAlgo_getMallocStats(audio_algo_t *const pAlgo);

#ifdef __cplusplus
}
#endif

#endif  /* __AUDIO_ALGO_H */
