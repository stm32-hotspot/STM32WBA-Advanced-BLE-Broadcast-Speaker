/**
******************************************************************************
* @file          livetune_db_json.h
* @author        MCD Application Team
* @brief         manage the db as json
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


#ifndef livetune_db_json_h
#define livetune_db_json_h


#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "st_registry.h"
#include "st_json.h"
#include "livetune_db.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

ST_Result livetune_db_element_json_register(livetune_db *pHandle, const char_t *pJson, const void *pRefBuilder, livetune_interface cb, livetune_db_element **ppElement);
ST_Result livetune_db_json_parse(livetune_db *pHandle, char_t *pJsonDesc);
ST_Result livetune_db_json_update(livetune_db *pHandle, char_t *pJsonDesc);
ST_Result livetune_db_json_dump(livetune_db *pHandle, json_instance_t *pJsonInstance);
ST_Result livetune_db_json_load(livetune_db *pHandle);
ST_Result livetune_db_json_save(livetune_db *pHandle);
#ifdef __cplusplus
};
#endif
#endif



