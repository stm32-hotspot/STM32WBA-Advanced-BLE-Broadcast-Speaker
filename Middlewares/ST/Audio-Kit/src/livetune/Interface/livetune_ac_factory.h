/**
******************************************************************************
* @file          livetune_ac_factory.h
* @author        MCD Application Team
* @brief         manage the access to ac factory and descriptors
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



#ifndef LIVETUNE_AC_FACTORY
#define LIVETUNE_AC_FACTORY

#ifdef __cplusplus
extern "C"
{
#endif
/* Includes ------------------------------------------------------------------*/
#include "livetune_helper.h"
#include "audio_chain_json_from_factory.h"
//#include "st_ac_types.h"

/* Exported constants --------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/

struct st_class_element_instance;

typedef enum livetune_ac_factory_sys_type
{
  AC_SYS_IN,
  AC_SYS_OUT,
} livetune_ac_factory_sys_type;

/* Exported macros ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
const audio_descriptor_param_t    *livetune_ac_factory_get_algo_param(acAlgo hAlgo, const char_t *pKey);
void                               livetune_ac_factory_init_sysio(const char_t *pSysIoName, livetune_db_instance_cnx *pCnx);
void                               livetune_ac_factory_get_min_max(const audio_descriptor_param_t *pParam, double *pMin, double *pMax);
int8_t                             livetune_ac_factory_is_default_value_string(audio_descriptor_param_t *pParam, char_t *pValue);
void                               livetune_ac_factory_get_string_value(audio_descriptor_param_t *pParam, void *pData, char_t *pValue, uint32_t szValue);
void                               livetune_ac_factory_config_string_value(const audio_descriptor_params_t *const pParam, void *const pData, const char *const pKey, const char *const pValue);
const audio_algo_factory_t        *livetune_ac_factory_get_algo_factory_name(const char_t *pName);
const audio_descriptor_param_t    *livetune_ac_factory_get_algo_config_param_descriptor(struct livetune_helper_builder *pBuilder, const char_t *pKey);
const audio_algo_descriptor_t     *livetune_ac_factory_get_algo_common_config_param_descriptor(const char_t *pKey);
int8_t                             livetune_ac_factory_cmp_default_value(const audio_descriptor_param_t *pParam, void *pValue);
int32_t                            livetune_ac_factory_get_chunk_config_param_descriptor(const char_t *pKey, const audio_descriptor_param_t **const ppParam);
const audio_factory_entry_t       *livetune_ac_factory_get_algo_factory_entry(void);
const audio_descriptor_params_t   *livetune_ac_factory_get_chunk_descriptor(void);
void                               livetune_ac_factory_apply_changes(livetune_db_instance *pInstance, uint32_t pinOut);
void                               livetune_ac_factory_init_default(livetune_db_instance *pInstance, livetune_chunk_conf_t *pConf);
int8_t                             livetune_ac_factory_is_default_value(char_t *pKey, void *pData);
uint8_t                            livetune_ac_factory_is_acBus(livetune_db_instance *pInstance, int32_t bType, uint32_t pinDef);
const audio_algo_factory_t        *livetune_ac_factory_get_factory_from_element(livetune_db_element *pElement);
uint32_t                           livetune_ac_factory_get_sys_connection_nb(livetune_ac_factory_sys_type type);
audio_chain_sys_connection_conf_t *livetune_ac_factory_get_sys_connection_conf(livetune_ac_factory_sys_type type, int32_t index);
const char_t                      *livetune_ac_factory_get_sys_connection_name(const audio_chain_sys_connection_conf_t *pConf);
void                               livetune_ac_factory_clone_sys_connection_chunk(const audio_chain_sys_connection_conf_t *pConf, livetune_chunk_conf_t *pCloneChunk);
const char_t                      *livetune_ac_factory_get_factory_instance_name(const char_t *pAlgoName);
const char_t                      *livetune_ac_factory_get_ac_version_string(void);
void                               livetune_ac_factory_copy_chunk(const livetune_chunk_conf_t *pSrcConf, livetune_chunk_conf_t *pDstCong, int32_t typeOverWrite);
void                               livetune_ac_factory_apply_algo_config_params(struct livetune_helper_builder *pBuilder);
struct st_class_element_instance  *livetune_ac_factory_builder_create(livetune_db_instance *pInstance, uint32_t extSize);

void          livetune_helper_builder_create_algo(struct livetune_helper_builder *pBuilder);
const char_t *livetune_factory_get_algo_name(const audio_algo_factory_t *pFactory);
const char_t *livetune_factory_get_algo_desc(const audio_algo_factory_t *pFactory);


#ifdef __cplusplus
};
#endif
#endif

