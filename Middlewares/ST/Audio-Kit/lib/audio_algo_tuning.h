/**
******************************************************************************
* @file    audio_algo_tuning.h
* @author  MCD Application Team
* @brief   algo tuning.
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
#ifndef __AUDIO_ALGO_TUNING_H
#define __AUDIO_ALGO_TUNING_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "audio_descriptor.h"
#include "audio_algo.h"


/* Exported types ------------------------------------------------------------*/
typedef enum
{
  AUDIO_ALGO_STATIC_PARAM,
  AUDIO_ALGO_DYNAMIC_PARAM,
  AUDIO_ALGO_NB_STAT_DYN_PARAM_TYPES
} audio_algo_tuning_stat_dyn_param_t;


/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
bool    AudioAlgoTuning_isUnsignedDecimal(char *const pString);
bool    AudioAlgoTuning_isSignedDecimal(char   *const pString);
bool    AudioAlgoTuning_isFloat(char           *const pString);

int32_t AudioAlgoTuning_getParamsDescr(audio_algo_t         *const pAlgo, audio_algo_tuning_stat_dyn_param_t const statDynParam, audio_descriptor_params_t const **const ppParamsDescr, char **ppErrorString);
int32_t AudioAlgoTuning_getParamsNb(audio_algo_t            *const pAlgo, audio_algo_tuning_stat_dyn_param_t const statDynParam, uint8_t *const pNbParam, char **ppErrorString);
int32_t AudioAlgoTuning_getParamVal(audio_algo_t            *const pAlgo, audio_algo_tuning_stat_dyn_param_t const statDynParam, uint8_t  const paramId, audio_descriptor_type_union_t *const pParamVal, char **ppErrorString);
int32_t AudioAlgoTuning_getParamMin(audio_algo_t            *const pAlgo, audio_algo_tuning_stat_dyn_param_t const statDynParam, uint8_t  const paramId, audio_descriptor_type_union_t *const pParamVal, char **ppErrorString);
int32_t AudioAlgoTuning_getParamMax(audio_algo_t            *const pAlgo, audio_algo_tuning_stat_dyn_param_t const statDynParam, uint8_t  const paramId, audio_descriptor_type_union_t *const pParamVal, char **ppErrorString);
int32_t AudioAlgoTuning_getParamValStr(audio_algo_t         *const pAlgo, audio_algo_tuning_stat_dyn_param_t const statDynParam, uint8_t  const paramId, char *const pParamValString,         size_t const size, char **ppErrorString);
int32_t AudioAlgoTuning_getParamValStrWithName(audio_algo_t *const pAlgo, audio_algo_tuning_stat_dyn_param_t const statDynParam, uint8_t  const paramId, char *const pParamValStringWithName, size_t const size, char **ppErrorString);

/*************************************************************************************************************************************/
/* AudioAlgoTuning_setParamVal() and AudioAlgoTuning_setParamValStr() routines can be used only when graph is initialized            */
/* (they can even be called dynamically while graph is running)                                                                      */
/* AudioAlgoTuning_setParamVal() gets parameter by numerical value, AudioAlgoTuning_setParamValStr() gets parameter by string value, */
/* they set parameter's value in a temporary config structure but do not apply it immediately,                                       */
/* when all algo's parameters have been updated, AudioAlgo_requestTuningUpdate() routine must be called: this routine checks the new */
/* config consistency and apply it at the right time (when algo process task is not running) if it is consistent                     */
/*                                                                                                                                   */
/*************************************************************************************************************************************/
int32_t AudioAlgoTuning_setParamVal(audio_algo_t            *const pAlgo, audio_algo_tuning_stat_dyn_param_t const statDynParam, uint8_t  const paramId, audio_descriptor_type_union_t *const pParamVal, char **ppErrorString);
int32_t AudioAlgoTuning_setParamValStr(audio_algo_t         *const pAlgo, audio_algo_tuning_stat_dyn_param_t const statDynParam, uint8_t  const paramId, char const *const pParamValString,              char **ppErrorString);
int32_t AudioAlgoTuning_set(audio_algo_t                    *const pAlgo, audio_algo_tuning_stat_dyn_param_t typeParam, const audio_descriptor_params_t *pParamTemplate, const char *pKey, const char *pValue, char **ppErrorString);


#ifdef __cplusplus
}
#endif

#endif  /* __AUDIO_ALGO_TUNING_H */


