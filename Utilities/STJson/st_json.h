/**
  ******************************************************************************
  * @file    st_json.h
  * @author  MCD Application Team
  * @brief   Header for st_json.c module
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2019(-2022) STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ST_JSON__
#define __ST_JSON__

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdlib.h>
#include "st_json_conf.h"
#include "st_json_typedef.h"

/* Exported types ------------------------------------------------------------*/
/* See st_json_typedef.h */

/* Exported constants --------------------------------------------------------*/
/* See st_json_typedef.h */

/* Exported macros -----------------------------------------------------------*/
/* See st_json_typedef.h */

/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

/* common API */
void          json_set_alloc_funcs(json_realloc_t pRealloc, json_free_t pFree);
const char_t *json_encode_char(char_t ch);
uint32_t      json_decode_char(const char_t *pText, int32_t *pToken);
void          json_free(void *ptr);
void         *json_realoc(void *ptr, size_t size);
void          json_dump_registry(json_instance_t *pInstance, char_t *pTitle);
const char_t *json_dumps(json_instance_t *pInstance, jsonID root, uint16_t flags);
const char_t *json_stringify(json_instance_t *pInstance, jsonID root, uint16_t flags, json_dump_t *pDump);
jsonErr       json_load(json_instance_t *pInstance, const char_t *pJson, jsonID *pId);
jsonErr       json_shutdown(json_instance_t *pInstance);
jsonErr       json_create_number(json_instance_t *pInstance, double value, jsonID *pId);
jsonErr       json_create_integer(json_instance_t *pInstance, int64_t value, jsonID *pId);
jsonErr       json_create_unsigned_integer(json_instance_t *pInstance, uint64_t value, jsonID *pId);
jsonErr       json_create_string(json_instance_t *pInstance, const char_t *pStr, jsonID *pId);
jsonErr       json_create_true(json_instance_t *pInstance, jsonID *pId);
jsonErr       json_create_false(json_instance_t *pInstance, jsonID *pId);
jsonErr       json_create_boolean(json_instance_t *pInstance, int8_t state, jsonID *pId);
jsonErr       json_create_array(json_instance_t *pInstance, jsonID *pId);
jsonErr       json_create_object(json_instance_t *pInstance, jsonID *pId);
jsonErr       json_create_null(json_instance_t *pInstance, jsonID *pId);
jsonErr       json_get_string_from_id(json_instance_t *pInstance, jsonID id, const char_t **ppStr);
jsonErr       json_get_integer_from_id(json_instance_t *pInstance, jsonID id, int64_t *pVal);
jsonErr       json_object_get_string_size(json_instance_t *pInstance, jsonID root, const char_t *pTree, const char_t *pKey, int32_t *pSize);
jsonErr       json_get_unsigned_integer_from_id(json_instance_t *pInstance, jsonID id, uint64_t *pVal);
jsonErr       json_get_number_from_id(json_instance_t *pInstance, jsonID id, double *pVal);
jsonErr       json_parse_string(json_instance_t *pInstance, json_parse_stream_t *pStream, char_t separator, jsonID *pId);
jsonErr       json_decref(json_instance_t *pInstance, jsonID root);
jsonErr       json_incref(json_instance_t *pInstance, jsonID root);

/* API to check nature of jsonID */
int8_t json_is_object(json_instance_t *pInstance, jsonID root);
int8_t json_is_array(json_instance_t *pInstance, jsonID root);
int8_t json_is_list(json_instance_t *pInstance, jsonID root);
int8_t json_is_true(json_instance_t *pInstance, jsonID root);
int8_t json_is_false(json_instance_t *pInstance, jsonID root);


/* list API */
jsonErr json_list_add_pair(json_instance_t *pInstance, jsonID root, jsonID key, jsonID value);
jsonErr json_list_pair(json_instance_t *pInstance, jsonID root, uint16_t index, jsonID *pKey, jsonID *pValue);
jsonErr json_list_del_pair(json_instance_t *pInstance, jsonID root, uint16_t index);
jsonErr json_list_get_count(json_instance_t *pInstance, jsonID root, uint16_t *pCount);

/* object API */
jsonErr json_object_set(json_instance_t *pInstance, jsonID root, const char_t *pTree, jsonID *pId);
jsonErr json_object_set_new(json_instance_t *pInstance, jsonID root, const char_t *pName, jsonID value);
jsonErr json_object_set_string(json_instance_t *pInstance, jsonID root, const char_t *pTree, const char_t *pKey, const char_t *pString);
jsonErr json_object_set_param(json_instance_t *pInstance, jsonID root, const char_t *pTree, const char_t *pKey, const char_t *pString, json_param_type_t const dataType);
jsonErr json_object_set_num_data(json_instance_t *pInstance, jsonID root, const char_t *pTree, const char_t *pKey, json_numData_t *pNumData);
jsonErr json_object_set_integer(json_instance_t *pInstance, jsonID root, const char_t *pTree, const char_t *pKey, int64_t integer);
jsonErr json_object_set_unsigned_integer(json_instance_t *pInstance, jsonID root, const char_t *pTree, const char_t *pKey, uint64_t integer);
jsonErr json_object_set_number(json_instance_t *pInstance, jsonID root, const char_t *pTree, const char_t *pKey, double number);
jsonErr json_object_set_boolean(json_instance_t *pInstance, jsonID root, const char_t *pTree, const char_t *pKey, uint8_t state);
jsonErr json_object_get_id_from_tree(json_instance_t *pInstance, jsonID root, const char_t *pTree, jsonID *pId);
jsonErr json_object_get_id_from_name(json_instance_t *pInstance, jsonID root, const char_t *pName, jsonID *pId);
jsonErr json_object_get_string(json_instance_t *pInstance, jsonID root, const char_t *pTree, const char_t *pKey, const char **ppStr);
jsonErr json_object_get_integer(json_instance_t *pInstance, jsonID root, const char_t *pTree, const char_t *pKey, int64_t *pVal);
jsonErr json_object_get_unsigned_integer(json_instance_t *pInstance, jsonID root, const char_t *pTree, const char_t *pKey, uint64_t *pVal);
jsonErr json_object_get_number(json_instance_t *pInstance, jsonID root, const char_t *pTree, const char_t *pKey, double *pVal);
jsonErr json_object_get_boolean(json_instance_t *pInstance, jsonID root, const char_t *pTree, const char_t *pKey, uint8_t *pBool);
jsonErr json_object_check_list_key(json_instance_t *pInstance, jsonID root, const char_t *pTree, const char_t *pKey);
jsonErr json_object_insert_new(json_instance_t *pInstance, uint16_t index, jsonID root, const char_t *pName, jsonID value);

/* array API */
jsonErr json_array_get(json_instance_t *pInstance, jsonID root, const char_t *pTree, jsonID *pId);
jsonErr json_array_set(json_instance_t *pInstance, jsonID root, const char_t *pTree, jsonID *pId);
jsonErr json_array_append_new(json_instance_t *pInstance, jsonID root, jsonID value);
jsonErr json_array_insert_new(json_instance_t *pInstance, jsonID root, int32_t index, jsonID value);


#ifdef __cplusplus
};
#endif

#endif /* __ST_JSON__ */
