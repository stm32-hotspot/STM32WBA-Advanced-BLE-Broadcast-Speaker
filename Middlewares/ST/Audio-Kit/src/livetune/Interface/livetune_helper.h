/**
******************************************************************************
* @file          livetune_helper.h
* @author        MCD Application Team
* @brief         some common utilities
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



#ifndef LIVETUNE_HELPER
#define LIVETUNE_HELPER

#ifdef __cplusplus
extern "C"
{
#endif
/* Includes ------------------------------------------------------------------*/
#include "st_base.h"
#include "st_os.h"
#include "livetune.h"
#include "livetune_generate.h"
#include "livetune_ac_wrapper.h"
//#include "st_ac_types.h"




/* Exported constants --------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/


typedef struct livetune_helper_builder_def
{
  const char_t *pAlgoName;
  uint32_t      pinPropagation;
  void         *pCookie;
} livetune_helper_builder_def;




typedef struct livetune_helper_builder
{
  const livetune_helper_builder_def *pBuilderDef;
  livetune_pipe                     *pPipe;
  livetune_db_instance              *pInstance;
  char_t                             sCbCtrlName[LIVETUNE_VAR_NAME_MAX];
  const audio_algo_factory_t        *pFactory;
} livetune_helper_builder;


typedef struct st_class_element_instance
{
  livetune_helper_builder hBuilder;
} st_class_element_instance;



/* Exported macros ------------------------------------------------------------*/
#define CNX_PARAM_DEFAULT_MARKER "INIT_DEFAULT"
#define LIVETUNE_STRINGIFY(a) #a

/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

#define LIVETUNE_HELP_CLASS_EXT(pCls, type) (type)(((uint8_t*)(pCls)) + sizeof(*(pCls)))
int32_t                      livetune_helper_build_audio_chain(livetune_pipe *pHandle);
uint32_t                     livetune_helper_cnx_nb(livetune_db_instance_pins_def **pDefList, uint32_t pinDef);
livetune_db_instance_cnx *livetune_helper_cnx_get(livetune_db_instance_pins_def **pDefList, uint32_t pinDef, uint32_t indexCnx);
int32_t                      livetune_helper_cnx_find_index(livetune_db_instance_pins_def **pDef, uint32_t pin, char_t *pName);
void                         livetune_helper_make_c_name(char_t *pName);
void                         livetune_helper_format_clear(char_t *pScratch);
void                         livetune_helper_format_tab(char_t *pScratch, int32_t size);
void                         livetune_helper_format_add(char_t *pScratch, uint32_t size, char_t *pFormat, ...);
void                         livetune_helper_chunk_propagation(livetune_db_instance *pInstance, uint32_t pinIn, uint32_t pinOut, char_t *pChunkPrefix, int32_t typeOverWrite);
void                         livetune_helper_set_pin_in(livetune_helper_builder *pBuilder, uint32_t pinID, livetune_db_instance_cnx *pCnxIn);
void                         livetune_helper_set_pin_out(livetune_helper_builder *pBuilder, uint32_t pinID, livetune_db_instance_cnx *pCnxOut);
ST_Result                    livetune_helper_chunk_copy(livetune_db_instance *pInstance, uint32_t pinDefIn, char_t *pCnxInName, uint32_t pinDefOut, char_t *pCnxOutName, char_t *pChunkPrefix, int32_t typeOverWrite);
void                         livetune_helper_generate_local(livetune_pipe *pHandle, char_t *pType, char_t *pVarName, char_t *pComment);
int32_t                      livetune_helper_string2json(const char_t *pRawStr, char_t *pStrJson, int32_t szStrJson);
void                         livetune_helper_add_prefix(char_t *pText, char_t *prefix);
char_t                      *livetune_helper_format(char_t *pFormat, ...);
char_t                      *livetune_helper_format_float(double value);
int8_t                       livetune_helper_instance_has_control(livetune_db_instance *pInstance);
void                         livetune_helper_builder_parameter_change_cb_default(livetune_db_instance *pInstance, livetune_info_param typeParam);
livetune_builder_result      livetune_helper_set_default_attrb(livetune_db_instance *pInstance);
void                         livetune_helper_builder_create_chunk_handles(livetune_helper_builder *pBuilder);
void                         livetune_helper_builder_pipe_init(livetune_pipe *pHandle);
ST_Result                    livetune_helper_builder_delete(st_class_element_instance *pCls);
livetune_builder_result      livetune_helper_builder_pre_init_graph_default(livetune_helper_builder *pBuilder);
livetune_builder_result      livetune_helper_builder_pipe_start_default(livetune_helper_builder *pBuilder);
livetune_builder_result      livetune_helper_builder_pipe_prolog_default(livetune_helper_builder *pBuilder);
livetune_builder_result      livetune_helper_builder_pipe_init_graph_element_default(livetune_helper_builder *pBuilder);
livetune_builder_result      livetune_helper_builder_pipe_init_graph_attach_cnx_default(livetune_helper_builder *pBuilder);
livetune_builder_result      livetune_helper_builder_post_init_graph_default(livetune_helper_builder *pBuilder);
livetune_builder_result      livetune_helper_builder_pipe_init_graph_element_chuck_default(livetune_helper_builder *pBuilder);
livetune_builder_result      livetune_helper_builder_pipe_init_graph_element_apply_default(livetune_helper_builder *pBuilder);
livetune_builder_result      livetune_helper_builder_pipe_init_graph_element_generate_default(livetune_helper_builder *pBuilder);
void                         livetune_helper_processing_constructor_default(livetune_db_instance *pInstance);
void                         livetune_helper_processing_destructor_default(livetune_db_instance *pInstance);
void                         livetune_helper_create_element_instance(livetune_db_element *pElement);

#ifdef __cplusplus
};
#endif


#endif



