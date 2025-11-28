/**
******************************************************************************
* @file    audio_algo_typedef.h
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
#ifndef __AUDIO_ALGO_TYPE_DEF_H
#define __AUDIO_ALGO_TYPE_DEF_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "audio_capabilities.h"
#include "audio_descriptor.h"

/* Exported constants --------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
typedef enum
{
  AUDIO_ALGO_STATE_DISABLED,              /* algo is not running                                                                                  */
  AUDIO_ALGO_STATE_ENABLED,               /* algo is running                                                                                      */
  AUDIO_ALGO_STATE_APPLY_CONFIG_REQUEST,  /* algo's dynamic config has been updated (in copy structure) and is waiting to be validate and applied */
  AUDIO_ALGO_STATE_APPLY_REINIT_REQUEST,  /* algo's static  config has been updated (in copy structure) and is waiting to be validate and applied */
  AUDIO_ALGO_STATE_APPLY_REINIT_ON_GOING, /* reinit is on-going (no process during this state in which algo is not ready)                         */
  AUDIO_ALGO_STATE_APPLY_REINIT_COMPLETED /* reinit is completed but config is not (no process during this state in which algo is not ready)      */
} audio_algo_state_t;

typedef struct audio_algo_common          /* struct name for forward declaration purpose */
{
  audio_capabilities_ios_descr_t       iosIn;
  audio_capabilities_ios_descr_t       iosOut;
  audio_capability_prio_level_t        prio_level;
  audio_capability_chunk_consistency_t chunks_consistency;
  audio_capabilities_misc_t            misc;
  /*audio_capabilities_ios_descr_t  iosCtrl; Todo*/
  char const                          *pName;
} audio_algo_common_t;


typedef struct audio_algo
{
  uint32_t                             signature;
  char const                          *pName;
  char const                          *pDesc;
  void                                *pUser;
  void                                *pInternalMem;
} audio_algo_t;

struct audio_algo_factory ;
typedef int32_t (*audio_algo_cb_t)(audio_algo_t *const pAlgo);
typedef void (*audio_factory_constructor_t)(const struct audio_algo_factory *pFactory);

typedef struct audio_algo_cbs
{
  audio_algo_cb_t init;
  audio_algo_cb_t deinit;                 /* WARNING: deinit routine mustn't use algo's input/output chunk lists because it is called after having deinitialized connected chunks */
  audio_algo_cb_t configure;
  audio_algo_cb_t dataInOut;
  audio_algo_cb_t process;
  audio_algo_cb_t control;
  audio_algo_cb_t checkConsistency;
  audio_algo_cb_t isDisabled;
  audio_algo_cb_t isDisabledCheckConsistency;
} audio_algo_cbs_t;


typedef struct audio_algo_factory
{
  audio_descriptor_params_t         const *pStaticParamTemplate;
  audio_descriptor_params_t         const *pDynamicParamTemplate;
  audio_algo_control_params_descr_t const *pControlTemplate;
  audio_algo_common_t               const *pCapabilities;
  audio_algo_cbs_t                        *pExecutionCbs;
  audio_factory_constructor_t              pFactoryConstructorCb;
} audio_algo_factory_t;


typedef struct audio_algo_descriptor
{
  char                          *pName;
  char                          *pDescription;
  char                          *pExpectedValue;
  audio_descriptor_param_type_t  paramType;

  int32_t (*set_cb)(audio_algo_t *const pAlgo, void *const  arg);
  int32_t (*get_cb)(audio_algo_t *const pAlgo, void **const pData);
} audio_algo_descriptor_t;

/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/

/**
* @brief Key value converter
*
*/
#define ALGO_KEY_VALUE_STRINGIFY(a) {#a,a}


/* Exported functions ------------------------------------------------------- */

#ifdef __cplusplus
}
#endif

#endif  /* __AUDIO_ALGO_TYPE_DEF_H */
