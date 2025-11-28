/**
******************************************************************************
* @file    audio_chain_instance.h
* @author  MCD Application Team
* @brief   Header for audio_chain_instance.c module
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
#ifndef __AUDIO_CHAIN_INSTANCE_H
#define __AUDIO_CHAIN_INSTANCE_H

/* Includes ------------------------------------------------------------------*/
#include "audio_chain.h"
#include "audio_chain_factory.h"
#include "audio_chain_instance_conf.h"
#include "audio_chain_sysIOs.h"

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported structures--------------------------------------------------------*/
typedef struct
{
  bool                         traceEnable;
  bool                         isDataInOutSpecificTask;
  bool                         isProcessSpecificTask;
  bool                         isControlSpecificTask;
  bool                         logInit;
  bool                         logCmsisOs;
} audio_chain_instance_params_t;

/* Exported variables ------------------------------------------------------- */
extern audio_chain_t           AudioChainInstance;

/* Exported functions ------------------------------------------------------- */

/* Audio API */
void                           AudioChainInstance_init(audio_chain_instance_params_t *pParams);
void                           AudioChainInstance_deinit(void);
void                           AudioChainInstance_initGraph(void); /*To be written by User to create audio flow */
void                           AudioChainInstance_deinitGraph(void);
void                           AudioChainInstance_initTuning(void);
void                           AudioChainInstance_deinitTuning(void);
void                           AudioChainInstance_deinitGraphRequest(void);
bool                           AudioChainInstance_isInitialized(void);
bool                           AudioChainInstance_isStarted(void);
bool                           AudioChainInstance_isDeinitGraphRequested(void);
void                           AudioChainInstance_reset(void);
uint64_t                       AudioChainInstance_getNbMsFromStart(void);
int32_t                        AudioChainInstance_getChunkAlgoOut(audio_chunk_t *const pChunk, uint8_t *const pAlgoId, uint8_t *const pChunkId);
int32_t                        AudioChainInstance_getChunkAlgoIn(audio_chunk_t  *const pChunk, uint8_t *const pAlgoId, uint8_t *const pChunkId);
void                           AudioChainInstance_run(void);
void                           AudioChainInstance_idle(void);

/* Common error routine */
void                           AudioChainInstance_error(const char *pFile, int const line, const char *pErrorMsg);

/* Traces management api*/
bool                           AudioChainInstance_setEnableTraces(bool const enable);
bool                           AudioChainInstance_getTracesStatus(void);
void                           AudioChainInstance_dumpControl(audio_algo_control_params_descr_t const *const pParamTemplate);
int32_t                        AudioChainInstance_dumpFactoryEntry(void);

/* Cycles Count routines */
bool                           AudioChainInstance_setEnableCyclesCnt(bool const enable);
bool                           AudioChainInstance_getCyclesCntStatus(void);
uint32_t                       AudioChainInstance_getCycleCountMeasureTimeout(void);
uint32_t                       AudioChainInstance_getCycleCountCbTimeout(void);

/* Environmental data management routines */
const audio_algo_descriptor_t *AudioChainInstance_getEnvDescriptors(void);
int32_t                        AudioChainInstance_setEnvDescriptors(const char *pKey, uint32_t value);
int32_t                        AudioChainInstance_dumpEnvDescriptors(void);
int32_t                        AudioChainInstance_getEnvConfig(const char *pKey, void *pParam);


#endif /* __AUDIO_CHAIN_INSTANCE_H */

