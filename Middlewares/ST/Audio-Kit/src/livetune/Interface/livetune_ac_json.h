/**
******************************************************************************
* @file          livetune_ac_json.h
* @author        MCD Application Team
* @brief         manage the build or read of jsons for audio chain
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



#ifndef LIVETUNE_AC_CHUNK
#define LIVETUNE_AC_CHUNK

#ifdef __cplusplus
extern "C"
{
#endif
/* Includes ------------------------------------------------------------------*/
#include "livetune_helper.h"
//#include "st_ac_types.h"


/* Exported constants --------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported macros ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void          livetune_ac_json_update_json_auto_cnx_params(json_instance_t *pJsonInst, jsonID hPinDef);
void          livetune_ac_json_fill_fake_parameters(json_instance_t *pJsonInst, jsonID hParam, livetune_chunk_conf_t *pChunConf);
void          livetune_ac_json_update_element_description(json_instance_t *pJsonInst, jsonID rootID, livetune_db_element *pElement);
void          livetune_ac_json_update_fill_instance_param(json_instance_t *pJsonInst, jsonID hObj, const audio_descriptor_param_t *pParam, int32_t type);
char_t       *livetune_ac_json_update_element(livetune_db *pHandle, livetune_db_element *pElement, uint32_t szHeaderDump);
char_t       *livetune_ac_json_sysio_create_def(audio_chain_sys_connection_conf_t *pConf, const char_t *pDesc);


#ifdef __cplusplus
};
#endif


#endif



