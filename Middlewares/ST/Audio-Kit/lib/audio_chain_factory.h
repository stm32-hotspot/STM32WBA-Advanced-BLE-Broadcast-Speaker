/**
******************************************************************************
* @file    audio_chain_factory.h
* @author  MCD Application Team
* @brief   Header for audio_chain_factory.c module
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
#ifndef __AUDIO_CHAIN_FACTORY_H
#define __AUDIO_CHAIN_FACTORY_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "audio_descriptor.h"
#include "audio_algo_typedef.h"
#include <stdarg.h>

/* Exported types ------------------------------------------------------------*/

typedef struct audio_factory_entry
{
  const audio_algo_factory_t             *pFactory;
  const char                             *pFactoryName;
} audio_factory_entry_t;


/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/

// this AUDIOCHAINFACTORY_INT2STR macro can be used for static const table initialization with const number between 0 and 15 (for instance coming from #define or enum)
// it is typically used for pDefault string initialization in a parameter description controlled though a droplist
#define AUDIOCHAINFACTORY_INT2STR(n) ((n) == 0) ? "0" : ((n) == 1) ? "1" : ((n) == 2) ? "2" : ((n) == 3) ? "3" : ((n) == 4) ? "4" : ((n) == 5) ? "5" : ((n) == 6) ? "6" : ((n) == 7) ? "7" : ((n) == 8) ? "8" : ((n) == 9) ? "9" : ((n) == 10) ? "10" : ((n) == 11) ? "11" : ((n) == 12) ? "12" : ((n) == 13) ? "13" : ((n) == 14) ? "14" : ((n) == 15) ? "15" : "0"


#if defined(AUDIO_CHAIN_RELEASE)
#define AUDIO_ALGO_OPT_STR(str) ""
#else
#define AUDIO_ALGO_OPT_STR(str) str
#endif


#if defined(AUDIO_CHAIN_ACSDK_USED) || defined(AUDIO_CHAIN_CONF_TUNING_CLI_USED)
#define AUDIO_ALGO_OPT_TUNING(template) template
#else
#define AUDIO_ALGO_OPT_TUNING(template) NULL
#endif


#if defined(__ICCARM__) /*!< IAR Compiler */

#pragma section = "algo_factory"
#define ALGO_FACTORY_GET_ENTRIES(pEntry)  pEntry = (const audio_factory_entry_t*)__section_begin("algo_factory")

#define ALGO_FACTORY_DECLARE_FORCE(fact) \
        _Pragma("location=\"algo_factory\"") \
        __root static const audio_factory_entry_t algo_##fact = {.pFactory = &fact,.pFactoryName = #fact};

#elif defined(__GNUC__) || defined(__CC_ARM)

#ifdef VALIDATION_X86
#define ALGO_FACTORY_GET_ENTRIES(pEntry) pEntry = NULL
#define ALGO_FACTORY_DECLARE_FORCE(fact)
#else
#define ALGO_FACTORY_GET_ENTRIES(pEntry) extern  char algo_factory;\
                                                     pEntry = (const audio_factory_entry_t*)(uint32_t)&algo_factory
#define ALGO_FACTORY_DECLARE_FORCE(fact) __attribute__((section(".algo_factory")))  const audio_factory_entry_t algo_##fact = {.pFactory = &fact,.pFactoryName = #fact};
#endif

#else

#error "Tool chain not supported"

#endif


/*In devel mode, we use the auto-declare mechanism, this mode is flexible but the side effect is that ALL algos will be linked  even if it is not used in a graph.
To work around this issue, if the flag ALGO_USE_LIST  is defined, the auto-declare mechanism mode is disabled and you have to declare manually each algo available in the graph. It is done using ALGO_FACTORY_DECLARE_FORCE(factory)
Notice terminal tool exist to build this list automatically. */


#if defined(ALGO_USE_LIST) || !defined(AUDIO_CHAIN_ACSDK_USED)
#define ALGO_FACTORY_DECLARE(fact)
#else
#define ALGO_FACTORY_DECLARE(fact) ALGO_FACTORY_DECLARE_FORCE(fact)
#endif


/* Exported functions ------------------------------------------------------- */
const audio_algo_descriptor_t    *AudioChainFactory_getCommonDescriptors(void);
const audio_descriptor_params_t  *AudioChainFactory_getChunkTemplate(void);
const audio_algo_factory_t       *AudioChainFactory_getAlgoFactory(const char *const pName);
const audio_factory_entry_t      *AudioChainFactory_getAlgoFactoryEntry(const char *const pName);
const audio_algo_descriptor_t    *AudioChainFactory_getAlgoCommonDescriptor(const char *const pKey);
const audio_factory_entry_t      *AudioChainFactory_getFactoryEntry(void);

void    AudioChainFactory_dumpAlgoCommonValue(audio_algo_t     *const pAlgo);
int32_t AudioChainFactory_getAlgoCommonConfig(audio_algo_t     *const pAlgo, const char *const pKey, void **const pData);
int32_t AudioChainFactory_setAlgoCommonConfig(audio_algo_t     *const pAlgo, const char *const pKey, void *arg);
int32_t AudioChainFactory_getAlgoCommonDescConfig(audio_algo_t *const pAlgo, const audio_algo_descriptor_t *const pCommonDesc, const char *pKey, void **const pData);


#ifdef __cplusplus
}
#endif

#endif  /* __AUDIO_CHAIN_FACTORY_H */


