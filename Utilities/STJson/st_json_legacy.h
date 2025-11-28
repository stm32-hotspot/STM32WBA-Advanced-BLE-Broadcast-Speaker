/**
  ******************************************************************************
  * @file    st_json_legacy.h
  * @author  MCD Application Team
  * @brief   Legacy Header for st_json.c module
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
#ifndef __ST_JSON_LEGACY_
#define __ST_JSON_LEGACY_

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdlib.h>
#include "st_json_conf.h"
#include "st_json_typedef.h"
#include "limits.h"


/* Exported types ------------------------------------------------------------*/
/* See st_json_typedef.h */

/* Exported constants --------------------------------------------------------*/
/* See st_json_typedef.h */

/* Exported macros -----------------------------------------------------------*/
#define JSON_INTERGER_ERROR     LLONG_MAX
/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
/* common api */

jsonID        json_loads(json_instance_t                       *pInstance, const char_t *pJson);                                                        /* new api => json_load */
void          jsonp_free(void *ptr);
jsonID        json_string(json_instance_t                      *pInstance, const char_t *value);                                                        /* new api => json_create_string */
jsonID        json_integer(json_instance_t                     *pInstance, int64_t value);                                                              /* new api => json_create_integer */
jsonID        json_unsigned_integer(json_instance_t            *pInstance, uint64_t value);                                                             /* new api => json_create_unsigned_integer */
jsonID        json_number(json_instance_t                      *pInstance, double value);                                                               /* new api => json_create_number */
jsonID        json_true(json_instance_t                        *pInstance);                                                                             /* new api => json_create_true */
jsonID        json_false(json_instance_t                       *pInstance);                                                                             /* new api => json_create_false */
jsonID        json_boolean(json_instance_t                     *pInstance, int8_t state);                                                               /* new api => json_create_boolean */
jsonID        json_null(json_instance_t                        *pInstance);                                                                             /* new api => json_create_null */
jsonID        json_object(json_instance_t                      *pInstance);                                                                             /* new api => json_get_new_id */

const char_t *json_string_value(json_instance_t                *pInstance, jsonID root);
int64_t       json_integer_value(json_instance_t               *pInstance, jsonID root);
uint64_t      json_unsigned_integer_value(json_instance_t      *pInstance, jsonID root);
double        json_number_value(json_instance_t                *pInstance, jsonID root);

/* list API */
uint16_t      json_list_count(json_instance_t                  *pInstance, jsonID root);

/* object api */
jsonID        json_get_object(json_instance_t                  *pInstance, jsonID root, const char_t *pTree);                                           /* new api => json_object_get_id_from_tree */
jsonID        json_set_object(json_instance_t                  *pInstance, jsonID root, const char_t *pTree);                                           /* new api => json_object_set */
jsonID        json_set_object_integer(json_instance_t          *pInstance, jsonID root, const char_t *pTree, const char_t *pKey, int64_t integer);      /* new api => json_object_set_integer */
jsonID        json_set_object_unsigned_integer(json_instance_t *pInstance, jsonID root, const char_t *pTree, const char_t *pKey, uint64_t integer);     /* new api => json_object_set_unsigned_integer */
jsonID        json_set_object_number(json_instance_t           *pInstance, jsonID root, const char_t *pTree, const char_t *pKey, double number);        /* new api => json_object_set_number */
jsonID        json_set_object_boolean(json_instance_t          *pInstance, jsonID root, const char_t *pTree, const char_t *pKey, uint8_t state);        /* new api => json_object_set_boolean */
jsonID        json_set_object_string(json_instance_t           *pInstance, jsonID root, const char_t *pTree, const char_t *pKey, const char_t *pString);/* new api => json_object_set_string */
jsonID        json_object_get(json_instance_t                  *pInstance, jsonID root, const char_t *pName);                                           /* new api => json_object_get_id_from_name */
uint8_t       json_get_object_boolean(json_instance_t          *pInstance, jsonID root, const char_t *pTree, const char_t *pKey);                       /* new api => json_object_get_boolean */
const char   *json_get_object_string(json_instance_t           *pInstance, jsonID root, const char_t *pTree, const char_t *pKey);                       /* new api => json_object_get_string */
int32_t       json_get_object_string_size(json_instance_t      *pInstance, jsonID root, const char_t *pTree, const char_t *pKey);                       /* new api => json_object_get_string_size */
int64_t       json_get_object_integer(json_instance_t          *pInstance, jsonID root, const char_t *pTree, const char_t *pKey);                       /* new api => json_object_get_integer */
uint64_t      json_get_object_unsigned_integer(json_instance_t *pInstance, jsonID root, const char_t *pTree, const char_t *pKey);                       /* new api => json_object_get_unsigned_integer */
double        json_get_object_number(json_instance_t           *pInstance, jsonID root, const char_t *pTree, const char_t *pKey);                       /* new api => json_object_get_number */
jsonErr       json_check_list_key(json_instance_t *pInstance, jsonID root, const char_t *pTree, const char_t *pKey);

/* objects api */
jsonID        json_array(json_instance_t                       *pInstance);                                                                             /* new api => json_get_new_id */
jsonID        json_object(json_instance_t                      *pInstance);                                                                             /* new api => json_get_new_id */
jsonID        json_get_array(json_instance_t                   *pInstance, jsonID root, const char_t *pTree);                                           /* new api => json_array_get */
jsonID        json_set_array(json_instance_t                   *pInstance, jsonID root, const char_t *pTree);                                           /* new api => json_array_set */



#ifdef __cplusplus
};
#endif

#endif /* __ST_JSON_LEGACY_ */

