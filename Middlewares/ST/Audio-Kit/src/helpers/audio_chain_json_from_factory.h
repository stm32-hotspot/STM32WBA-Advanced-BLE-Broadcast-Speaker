/**
******************************************************************************
* @file          audio_chain_json_from_factory.h
* @author        MCD Application Team
* @brief         manage the conversion of factory in json
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



#ifndef __AUDIOCHAIN_JSON_FROM_FACTORY
#define __AUDIOCHAIN_JSON_FROM_FACTORY

#ifdef __cplusplus
extern "C"
{
#endif
#include "st_json.h"

/* Includes ------------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported macros ------------------------------------------------------------*/
#define LIVETUNE_CHUNK_SCRATCH_SIZE  4096
#define CNX_PARAM_SKIP_MARKER           "NO_CHANGE"


/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

void     AudioChainJson_update_fill_instance_param(json_instance_t *pJsonInst, jsonID hObj, const audio_descriptor_param_t *pParam, int32_t type);
void     AudioChainJson_add_attrib(json_instance_t *pJsonInst, jsonID rootID, audio_algo_common_t const *pCap);
void     AudioChainJson_json_add_param(json_instance_t *pJsonInst, jsonID hCnx, int32_t paramPos, const char *pDescription, uint32_t type, const char *pName, const char *pValue, const char *pControl, const char *pControlParam, const char *pDefault, double min, double max, uint8_t bDisabled);
void     AudioChainJson_add_pindef(json_instance_t *pJsonInst, jsonID rootID, audio_algo_common_t const *pCap, const audio_capabilities_ios_descr_t *pAcPin, uint32_t indexPinDef, uint32_t cnxLimit, char *pPinType);
void     AudioChainJson_update_json_element_capabilities(json_instance_t *pJsonInst, jsonID rootID, const audio_algo_factory_t *pFactory);
void     AudioChainJson_update_instance_params(json_instance_t *pJsonInst, jsonID hParam, const audio_descriptor_params_t *pTemplate, int32_t type, int32_t *pIndex);
int32_t  AudioChainJson_update_element(const audio_algo_factory_t *pFactory, json_instance_t *pJsonInst, jsonID rootID);
char_t  *AudioChainJson_factory_translate_key_value(const audio_descriptor_param_t *pParam, const char_t *pStringValue);


#ifdef __cplusplus
};
#endif


#endif  // __AUDIOCHAIN_JSON_FROM_FACTORY

