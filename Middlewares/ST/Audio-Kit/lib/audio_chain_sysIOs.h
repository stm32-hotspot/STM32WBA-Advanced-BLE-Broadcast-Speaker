/**
******************************************************************************
* @file    audio_chain_sysIOs.h
* @author  MCD Application Team
* @brief   Header for audio_chain_sysIOs.c module
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
#ifndef __AUDIO_CHAIN_SYSIOS_H
#define __AUDIO_CHAIN_SYSIOS_H

/* Includes ------------------------------------------------------------------*/
#include "audio_chain.h"
#include "audio_chain_sysIOs_typedefs.h"

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported structures--------------------------------------------------------*/
/* Exported variables ------------------------------------------------------- */
/* Exported functions ------------------------------------------------------- */
const ac_sys_ios_t                  *AudioChainSysIOs_get(void);      /* routine that must implemented by user to give to AudioChain the user's defined system IOs definition */

void                                 AudioChainSysIOs_init(audio_chain_t *const pHdle);
void                                 AudioChainSysIOs_deinit(void);

void                                 AudioChainSysIOs_initIn(uint8_t  const id);
void                                 AudioChainSysIOs_initOut(uint8_t const id);

audio_chunk_t                       *AudioChainSysIOs_getInChunk(uint8_t  const id);
audio_chunk_t                       *AudioChainSysIOs_getOutChunk(uint8_t const id);

audio_chunk_conf_t            const *AudioChainSysIOs_getConf(audio_chunk_t *const pChunk); /* get system in or system out chunk conf */
int32_t                              AudioChainSysIOs_create(const char *const pName, audio_chunk_t **const ppChunk);
void                                 AudioChainSysIOs_addIn(const  char *const pName, const char *const pDescription, audio_buffer_t *const pAudioBuffer, uint8_t const sysIoId, ac_sys_ios_cb_t const availabilityCb);
void                                 AudioChainSysIOs_addOut(const char *const pName, const char *const pDescription, audio_buffer_t *const pAudioBuffer, uint8_t const sysIoId, ac_sys_ios_cb_t const availabilityCb);

audio_chain_sys_connections_t const *AudioChainSysIOs_getCnxIn(void);
audio_chain_sys_connections_t const *AudioChainSysIOs_getCnxOut(void);


#endif /* __AUDIO_CHAIN_SYSIOS_H */

