/**
******************************************************************************
* @file          livetune_generate.h
* @author        MCD Application Team
* @brief         Generate sthe source code
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



#ifndef LIVETUNE_GENERATE
#define LIVETUNE_GENERATE

#ifdef __cplusplus
extern "C"
{
#endif
/* Includes ------------------------------------------------------------------*/
#include "st_base.h"
#include "st_os.h"
#include "livetune_pipe.h"
#include "livetune_helper.h"
//#include "st_ac_types.h"

/* Exported constants --------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
typedef void (*livetune_generate_control_cb)(struct livetune_helper_builder *pBuilder);
typedef struct livetune_generate_conf
{
  audio_descriptor_param_t *pParamDesc;
  char_t                   *pValue;
  char_t                   *pFunction;
  char_t                   *pVarName;
} livetune_generate_conf;


/* Exported macros ------------------------------------------------------------*/
#define ST_GENERATOR_TAB_1 4
#define ST_GENERATOR_TAB_2 8
#define ST_GENERATOR_TAB_3 12

#define ST_GENERATOR_SZ_CODE_1  10
#define ST_GENERATOR_SZ_CODE_2  40
#define ST_GENERATOR_SZ_CODE_3  30
#define ST_GENERATOR_SZ_COMMENT 10

#define ST_GENERATOR_PIN_TYPE_OUT 0
#define ST_GENERATOR_PIN_TYPE_IN  1


/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
char_t       *livetune_generate_code(livetune_pipe *pHandle);
void          livetune_generate_chunk_handles(livetune_pipe *pHandle, livetune_db_instance_pins_def **pPinDef, uint32_t pin);
void          livetune_generate_connect_pins(livetune_pipe *pHandle, char_t *pAlgoName, uint32_t pinID, char_t *pChunkName, uint32_t pinType);
void          livetune_generate_connect(struct livetune_helper_builder *pBuilder, char_t *pChunkName, char_t *pAlgoDst);
void          livetune_generate_text(livetune_pipe *pHandle, char_t *pFormat, ...);
void          livetune_generate_initalized_array(void *pArray, char_t *pScratch, int32_t nbRow, int32_t nbColumn, int32_t type);
void          livetune_generate_header_add(livetune_pipe *pHandle, const char_t *pHeader);
void          livetune_generate_header(livetune_pipe *pHandle);
void          livetune_generate_create_chunks(struct livetune_helper_builder *pBuilder, livetune_db_instance_pins_def **pPinDef, uint32_t pinDef);
void          livetune_generate_aligned(livetune_pipe *pHandle, int32_t wPos1, char_t *pPos1, int32_t wPos2, char_t *pPos2, int32_t wPos3, char_t *pPos3, int32_t wPos4, char_t *pPos4);
void          livetune_generate_global(livetune_pipe *pHandle, char_t *pType, char_t *pVarName, char_t *pComment);
void          livetune_generate_fn_declare(livetune_pipe *pHandle, char_t *pType, char_t *pBody);
void          livetune_generate_fn_body(livetune_pipe *pHandle, char_t *pBody, char_t *pComment);
void          livetune_generate_comment(livetune_pipe *pHandle, char_t *pText);
void          livetune_generate_control_callback(struct livetune_helper_builder *pBuilder, char_t *fnName, livetune_generate_control_cb generateCB);
const char_t *livetune_generate_get_type_str(struct livetune_helper_builder *pBuilder, audio_descriptor_param_type_t type);
void          livetune_generate_algo_create(struct livetune_helper_builder *pBuilder, char_t *pOptCbsName);
void          livetune_generate_algo_config(struct livetune_helper_builder *pBuilder);
void          livetune_generate_algo_set_config_nb(struct livetune_helper_builder *pBuilder, const audio_descriptor_params_t *pParamDescr, uint32_t *pControIndex);
void          livetune_generate_algo_set_config_value(struct livetune_helper_builder *pBuilder, const char_t *pKey, const char_t *pValue);
void          livetune_generate_algo_set_config_value_comment(struct livetune_helper_builder *pBuilder, const char_t *pKey);
void          livetune_generate_algo_set_common_config_value_comment(struct livetune_helper_builder *pBuilder, const char_t *pKey);
void          livetune_generate_algo_set_common_config_value(struct livetune_helper_builder *pBuilder, const char_t *pKey, const char_t *pValue);
void          livetune_generate_algo_set_config_value_ptr(struct livetune_helper_builder *pBuilder, const char_t *pKey, const char_t *pValue);
void          livetune_generate_chunk_set_config_value_comment(livetune_pipe *pPipe, const char_t *pKey);
void          livetune_generate_global(livetune_pipe *pHandle, char_t *pType, char_t *pVarName, char_t *pComment);
#endif

