/**
******************************************************************************
* @file        st_json.c
* @author      MCD Application Team
* @brief       Parses and creates json strings
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


/* Includes ------------------------------------------------------------------*/
#include <assert.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "st_json.h"

/* Global variables ----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
#ifdef __GNUC__
  #define JSON_LOCAL_ITOA_USED
#endif

#define JSON_GROW_LIST       2U   /* Grow list by step */
#define JSON_GROW_REG        256U /* Grow registry by step */
#define JSON_LAST_ERROR      JSON_ERR_MAX
#define JSON_DUMP_GROW       512U          /* grow dump by step */
#define JSON_MAX_STRING_SIZE (20U * 1024U) /* maximum string allowed */
#define JSON_STRING_GROW     32U           /* grow string */

#define JSON_END_QUOTE           '\0'
#define JSON_END_ALPHA           '\1'
#define JSON_END_DIGIT           '\2'
#define JSON_END_WHILE_ALPHA_NUM '\3'

#define JSON_ENCODE_STRING

#ifndef TRUE
  #define TRUE 1
#endif
#ifndef FALSE
  #define FALSE 0
#endif
#ifndef DBL_MIN
  #define DBL_MIN 2.2250738585072014E-308
#endif
#ifndef DBL_MAX
  #define DBL_MAX 1.7976931348623157E+308
#endif

/* Private macros ------------------------------------------------------------*/
/* gets the offset of 2 ptr from a base */
#define JSON_OFFSET(base, pobj) (uint32_t)((char_t*)(pobj) - (char_t*)(base))
/* gets the item payload, following the item  */
#define JSON_PAYLOAD(type, json) ((type)(&(json)[1]))

#define JSON_MAIN_TYPE(a)    ((a)&0x0FU)
#define JSON_SUB_TYPE(a)     (((a) >> 4U) & 0x0FU)
#define JSON_MAKE_TYPE(m, s) (uint8_t)((((uint8_t)(s)) << 4U) | (((uint8_t)(m)) & 0x0FU))

#ifndef JSON_ASSERT
#define JSON_ASSERT(a)                        \
    if ((a) == 0)                             \
    {                                         \
        s_assert(#a, __FUNCTION__, __LINE__); \
    }
#ifndef S_ASSERT
  #define S_ASSERT
#endif
#endif

#ifndef JSON_ERR_PRINT
#define JSON_ERR_PRINT(a)                     \
    if ((a) != JSON_OK)                       \
    {                                         \
        s_assert(#a, __FUNCTION__, __LINE__); \
    }
#ifndef S_ASSERT
  #define S_ASSERT
#endif
#endif

#ifndef JSON_PRINT
  #define JSON_PRINT(...) printf(__VA_ARGS__)
#endif

#ifndef MIN
  #define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif



/* Private typedef -----------------------------------------------------------*/
/**
* @brief Main json type
*
*/
typedef enum
{
  JSON_TYPE_LIST,
  JSON_TYPE_VALUE,
} json_main_type;

/**
* @brief Registry entry header
*
*/
typedef struct t_json_item
{
  uint8_t  type;
  uint8_t  ref;
  jsonID   id;
  uint16_t szObj;
} json_item_t;

/**
* @brief List pair item
*
*/
typedef struct t_json_pair
{
  jsonID keyId;
  jsonID valueId;
} json_pair_t;

/**
* @brief List entry , followed by json_pair_t
*
*/
typedef struct t_json_list
{
  uint16_t max;
  uint16_t cur;
} json_list_t;


/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

/* Allocator overload */
static json_realloc_t s_realloc = realloc;
static json_free_t    s_free    = free;

/* static functions working on local instance */
static jsonErr      s_json_get_new_id(json_instance_t *pInstance, jsonID *pId, json_sub_type type);
static jsonErr      s_check_shrink(json_instance_t *pInstance);
static jsonErr      s_check_grow(json_instance_t *pInstance, uint16_t grow);
static jsonErr      s_del_by_id(json_instance_t *pInstance, jsonID id);
static jsonErr      s_dump_serialize_add(json_instance_t *pInstance, json_dump_t *pDump, const char_t *pString);
static jsonErr      s_dump_serialize_indent(json_instance_t *pInstance, json_dump_t *pDump, uint16_t index);
static jsonErr      s_serialize_pair(json_instance_t *pInstance, json_dump_t *pDump, json_pair_t *pPair, uint16_t index);
static jsonID       s_generate_id(json_instance_t *pInstance);
static jsonErr      s_get_string_quoted(json_instance_t *pInstance, json_parse_stream_t *pStream, jsonID *pId);
static jsonErr      s_parse_list(json_instance_t *pInstance, json_parse_stream_t *pStream, jsonID root);
static jsonErr      s_grow_list(json_instance_t *pInstance, jsonID root, uint16_t grow);
static json_item_t *s_add_id(json_instance_t *pInstance, uint16_t len);
static json_item_t *s_create_string(json_instance_t *pInstance, const char_t *str_value, uint32_t szvalue);
static json_item_t *s_start_registry(json_instance_t *pInstance);
static json_item_t *s_get_registry_next(json_instance_t *pInstance, json_item_t *pReg);
static json_item_t *s_find_string(json_instance_t *pInstance, const char_t *str_value);
static json_item_t *s_find_id(json_instance_t *pInstance, jsonID id);
static jsonErr      s_set_type(json_instance_t *pInstance, jsonID id, uint8_t type);
static uint8_t      s_get_main_type(json_instance_t *pInstance, jsonID id);
static uint8_t      s_get_sub_type(json_instance_t *pInstance, jsonID id);
static jsonErr      s_find_list_entry(json_instance_t *pInstance, jsonID root, const char_t *pKeyName, jsonID *pId);
static jsonErr      s_parse_element(json_instance_t *pInstance, json_parse_stream_t *pStream, jsonID *pId);
static void         s_add_key_val(json_instance_t *pInstance, jsonID root, jsonID key, jsonID value);
static void         s_insert_key_val(json_instance_t *pInstance, jsonID root, int32_t index, jsonID key, jsonID value);
static jsonErr      s_create_and_get_string_id(json_instance_t *pInstance, const char_t *str, int32_t len, jsonID *pId);
static jsonErr      s_get_string_id(json_instance_t *pInstance, const char_t *str, int32_t len, jsonID *pId);
static jsonErr      s_object_set_string(json_instance_t *pInstance, jsonID root, const char_t *pTree, const char_t *pKey, const char_t *pString, jsonID *pId);
static jsonErr      s_create_num_data(json_instance_t *pInstance, json_numData_t *pNumData, jsonID *pId);
static jsonErr      s_create_special_types(json_instance_t *pInstance, const char_t *value, json_sub_type subType, jsonID *pId);
static jsonErr      s_object_get_num_data(json_instance_t *pInstance, jsonID root, const char_t *pTree, const char_t *pKey, json_numData_t *pNumData);
/* static functions working on pStream */
static jsonErr       s_read_stream_eof(json_parse_stream_t *pStream);
static jsonErr       s_read_stream_next_char(json_parse_stream_t *pStream);
static jsonErr       s_read_stream_seek_next(json_parse_stream_t *pStream);
static jsonErr       s_read_stream_copy_until(json_parse_stream_t *pStream, char_t separator, int32_t *pNbChar);
static char_t        s_read_stream(json_parse_stream_t *pStream);
static void          s_read_rewind(json_parse_stream_t *pStream);
static jsonErr       s_decode_char(json_parse_stream_t *pStream, char_t ch, char_t **pBuffer, uint16_t *index);
static jsonErr       s_convert_str_to_num_data(json_numData_t *pNumData, const char *pStr);
static const char_t *s_get_type_name(uint8_t type);
static int8_t        s_is_alpha_num(char_t ch);
static int8_t        s_is_white_space(char_t ch);
static int8_t        s_is_int_string(char_t *pString);
static int8_t        s_is_number_string(char_t *pString);
#ifdef S_ASSERT
  static void          s_assert(const char_t *pError, const char_t *pFunction, uint32_t line);
#endif

#ifdef JSON_LOCAL_ITOA_USED
  static void s_itoa(int64_t val, char_t *pString, uint32_t szString);
  static void s_uitoa(uint64_t val, char_t *pString, uint32_t szString);
#endif


/* Functions Definition ------------------------------------------------------*/


#ifdef S_ASSERT
static void s_assert(const char_t *pError, const char_t *pFunction, uint32_t line)
{
  printf("JSON:assert (%s) %s:%d\r\n", pError, pFunction, line);
  #ifndef __CSTAT__
  while (1); /* cause incredible error */
  #endif
}
#endif


/**
* @brief overload allocator functions
*
* @param pRealloc callback realloc
* @param pFree callback free
*/
void json_set_alloc_funcs(json_realloc_t pRealloc, json_free_t pFree)
{
  s_realloc = pRealloc;
  s_free = pFree;
}


/**
* @brief for debug purpose, list all registry entries
*
* @param pInstance the json instance
* @param pTitle  the dump title
*/
void json_dump_registry(json_instance_t *pInstance, char_t *pTitle)
{
  uint32_t index = 0;
  json_item_t *pRes = s_start_registry(pInstance);

  if (pTitle)
  {
    JSON_PRINT("%s\r\n", pTitle);
  }

  while (pRes)
  {
    JSON_PRINT("%02d: %s: sz:%d - %p\r\n", pRes->id, s_get_type_name(pRes->type), pRes->szObj, pRes);
    switch (JSON_MAIN_TYPE(pRes->type))
    {
      case JSON_TYPE_VALUE:
      {
        JSON_PRINT("\t%s\r\n", JSON_PAYLOAD(char_t *, pRes));
        break;
      }
      case JSON_TYPE_LIST:
      {
        json_list_t *pList = JSON_PAYLOAD(json_list_t *, pRes);
        JSON_PRINT("\t%d:%d\r\n", pList->cur, pList->max);
        json_pair_t *pPair = JSON_PAYLOAD(json_pair_t *, pList);
        for (uint16_t a = 0; a < pList->cur; a++)
        {
          char_t strKey[30];
          char_t strValue[30];
          const char_t *pKeyName, *pKeyValue;
          jsonErr jsError = json_get_string_from_id(pInstance, pPair[a].keyId, &pKeyName);

          if (jsError == JSON_OK)
          {
            snprintf(strKey, sizeof(strKey), "{%d}%s", pPair[a].keyId, pKeyName);
          }
          else
          {
            pKeyName = strKey;
            snprintf(strKey, sizeof(strKey), "{%d}", pPair[a].keyId);
          }

          if (json_is_list(pInstance, pPair[a].valueId) == JSON_TRUE)
          {
            snprintf(strValue, sizeof(strValue), "ID:{%d}", pPair[a].valueId);
          }
          else
          {
            jsError = json_get_string_from_id(pInstance, pPair[a].valueId, &pKeyValue);
            if (jsError == JSON_OK)
            {
              snprintf(strValue, sizeof(strValue), "{%d}-%s", pPair[a].valueId, pKeyValue);
            }
          }

          JSON_PRINT("\t  %02d:[%s]:%s\r\n", a, strKey, strValue);
        }
        break;
      }
      default:
        /* Do Nothing MISRA*/
        break;
    }
    pRes = s_get_registry_next(pInstance, pRes);
    index++;
  }
}


/**
* @brief returns a string
*
* @param pInstance the json instance
* @param pStream  the stream instance
* @param separator the separator
* @return jsonID the json id
*/
jsonErr json_parse_string(json_instance_t *pInstance, json_parse_stream_t *pStream, char_t separator, jsonID *pId)
{
  int32_t nbChar;
  jsonErr error = s_read_stream_copy_until(pStream, separator, &nbChar);
  if (error == JSON_OK)
  {
    error = s_create_and_get_string_id(pInstance, pStream->pTmpString, 0, pId);
  }
  return error;
}



/**
* @brief adds an Json object to an  array
*
* @param pInstance the json instance
* @param root the root id
* @param value the json value id
* @return jsonErr
*/
jsonErr json_array_append_new(json_instance_t *pInstance, jsonID root, jsonID value)
{
  jsonErr error = s_grow_list(pInstance, root, JSON_GROW_LIST);
  if (error == JSON_OK)
  {
    if (json_is_array(pInstance, root) == JSON_FALSE)
    {
      error = JSON_ERR_TYPE;
    }
  }
  if (error == JSON_OK)
  {
    if (value == JSON_ID_NULL)
    {
      error = JSON_ERR_FOUND;
    }
  }
  if (error == JSON_OK)
  {
    s_add_key_val(pInstance, root, JSON_ID_NULL, value);
  }
  JSON_ERR_PRINT(error);
  return error;
}


/**
* @brief insert an Json object to an  array
*
* @param pInstance the json instance
* @param root the root id
* @param value the json value id
* @return jsonErr
*/
jsonErr json_array_insert_new(json_instance_t *pInstance, jsonID root, int32_t index, jsonID value)
{
  jsonErr error = s_grow_list(pInstance, root, JSON_GROW_LIST);
  if (error == JSON_OK)
  {
    if (json_is_array(pInstance, root) == JSON_FALSE)
    {
      error = JSON_ERR_TYPE;
    }
  }
  if (error == JSON_OK)
  {
    if (value == JSON_ID_NULL)
    {
      error = JSON_ERR_FOUND;
    }
  }
  if (error == JSON_OK)
  {
    if (index != -1)
    {
      s_insert_key_val(pInstance, root, index, JSON_ID_NULL, value);
    }
    else
    {
      s_add_key_val(pInstance, root, JSON_ID_NULL, value);
    }
  }
  JSON_ERR_PRINT(error);
  return error;
}



/**
* @brief sets a json pair to an object
*
* @param pInstance the json instance
* @param root the json root id
* @param key the key json id
* @param value the value json id
* @return jsonErr
*/
jsonErr json_list_add_pair(json_instance_t *pInstance, jsonID root, jsonID key, jsonID value)
{
  jsonErr error = s_grow_list(pInstance, root, JSON_GROW_LIST);
  if (error == JSON_OK)
  {
    s_add_key_val(pInstance, root, key, value);
  }
  JSON_ERR_PRINT(error);
  return error;
}


/**
* @brief  set a json pair to an object
*
* @param pInstance the json instance
* @param root the json root id
* @param pName the string key
* @param value the json id value
* @return jsonErr
*/
jsonErr json_object_set_new(json_instance_t *pInstance, jsonID root, const char_t *pName, jsonID value)
{
  jsonErr error = s_grow_list(pInstance, root, JSON_GROW_LIST);
  jsonID keyID = 0;
  if (error == JSON_OK)
  {
    keyID = s_create_string(pInstance, pName, 0)->id;
    if (keyID == JSON_ID_NULL)
    {
      error = JSON_ERR_FOUND;
    }
  }
  if (error == JSON_OK)
  {
    if (json_is_object(pInstance, root) == JSON_FALSE)
    {
      error = JSON_ERR_TYPE;
    }
  }
  if (error == JSON_OK)
  {
    if (value == JSON_ID_NULL)
    {
      error = JSON_ERR_FOUND;
    }
  }
  if (error == JSON_OK)
  {
    s_add_key_val(pInstance, root, keyID, value);
  }
  JSON_ERR_PRINT(error);
  return error;
}


/**
* @brief sets a json pair to an object
*
* @param pInstance the json instance
* @param index the index object
* @param root the root json id
* @param pName the key string
* @param value the json id value
* @return jsonErr
*/
jsonErr json_object_insert_new(json_instance_t *pInstance, uint16_t index, jsonID root, const char_t *pName, jsonID value)
{
  jsonErr error = s_grow_list(pInstance, root, JSON_GROW_LIST);
  jsonID keyID = 0;
  json_list_t *pList = NULL;

  if (error == JSON_OK)
  {
    keyID = s_create_string(pInstance, pName, 0)->id;
    if (keyID == JSON_ID_NULL)
    {
      error = JSON_ERR_FOUND;
    }
  }

  if (error == JSON_OK)
  {
    if (json_is_object(pInstance, root) == JSON_FALSE)
    {
      error = JSON_ERR_TYPE;
    }
  }

  if (error == JSON_OK)
  {
    if (value == JSON_ID_NULL)
    {
      error = JSON_ERR_FOUND;
    }
  }

  if (error == JSON_OK)
  {
    json_item_t *pRoot = s_find_id(pInstance, root);
    JSON_ASSERT(pRoot != NULL);
    pList = JSON_PAYLOAD(json_list_t *, pRoot);
    if (index >= pList->cur)
    {
      error = JSON_ERR_FOUND;
    }
  }

  if (error == JSON_OK)
  {
    pList->cur++;
    json_pair_t *pPair = JSON_PAYLOAD(json_pair_t *, pList);
    uint16_t len = ((uint16_t)pList->cur - index) - 1U;
    JSON_ASSERT(len != 0U);
    memmove(&pPair[index + 1U], &pPair[index], len * sizeof(json_pair_t));
    pPair[index].keyId = keyID;
    pPair[index].valueId = value;
  }

  JSON_ERR_PRINT(error);
  return error;
}


/**
* @brief creates a json string
*
* @param pInstance (IN) the json instance
* @param pStr(IN) the string
* @param pId (OUT) pointer of the index
* @return error
*/
jsonErr json_create_string(json_instance_t *pInstance, const char_t *pStr, jsonID *pId)
{
  return s_create_and_get_string_id(pInstance, pStr, 0, pId);
}


/**
* @brief creates a json integer
*
* @param pInstance the json instance
* @param value the integer value
* @param pId pointer to jsonID
* @return error
*/
jsonErr json_create_integer(json_instance_t *pInstance, int64_t value, jsonID *pId)
{
  json_numData_t signedInt =
  {
    .dataType = JSON_SIGNED64,
    .val.s64  = value,
    .min.s64  = INT64_MIN,
    .max.s64  = INT64_MAX
  };

  return s_create_num_data(pInstance, &signedInt, pId);
}


/**
* @brief creates a json unsigned integer
*
* @param pInstance the json instance
* @param value the unsigned integer value
* @param pId pointer to jsonID
* @return error
*/
jsonErr json_create_unsigned_integer(json_instance_t *pInstance, uint64_t value, jsonID *pId)
{
  json_numData_t unsignedInt =
  {
    .dataType = JSON_UNSIGNED64,
    .val.u64  = value,
    .min.u64  = 0ULL,
    .max.u64  = UINT64_MAX
  };

  return s_create_num_data(pInstance, &unsignedInt, pId);
}


/**
* @brief creates a json number as double
*
* @param pInstance the json instance
* @param value  number value
* @param pId pointer to jsonID
* @return error
*/
jsonErr json_create_number(json_instance_t *pInstance, double value, jsonID *pId)
{
  json_numData_t doubleNumber =
  {
    .dataType = JSON_DOUBLE,
    .val.dbl  = value,
    .min.dbl  = DBL_MIN,
    .max.dbl  = DBL_MAX
  };

  return s_create_num_data(pInstance, &doubleNumber, pId);
}


/**
* @brief create a json boolean true
*
* @param pInstance the json instance
* @param pId pointer to jsonID
* @return error
*/
jsonErr json_create_true(json_instance_t *pInstance, jsonID *pId)
{
  return s_create_special_types(pInstance, "true", JSON_SUB_TRUE, pId);
}


/**
* @brief create a json boolean false
*
* @param pInstance the json instance
* @param pId pointer to jsonID
* @return error
*/
jsonErr json_create_false(json_instance_t *pInstance, jsonID *pId)
{
  return s_create_special_types(pInstance, "false", JSON_SUB_FALSE, pId);
}


/**
* @brief create a json boolean
*
* @param pInstance the json instance
* @param state the boolean value
* @param pId pointer to jsonID
* @return error
*/
jsonErr json_create_boolean(json_instance_t *pInstance, int8_t state, jsonID *pId)
{
  return (state != 0) ? json_create_true(pInstance, pId) : json_create_false(pInstance, pId);
}


/**
* @brief create a json array
*
* @param pInstance the json instance
* @param pId pointer to jsonID
* @return error
*/
jsonErr json_create_array(json_instance_t *pInstance, jsonID *pId)
{
  return s_json_get_new_id(pInstance, pId, JSON_SUB_ARRAY);
}


/**
* @brief create a json object
*
* @param pInstance the json instance
* @param pId pointer to jsonID
* @return error
*/
jsonErr json_create_object(json_instance_t *pInstance, jsonID *pId)
{
  return s_json_get_new_id(pInstance, pId, JSON_SUB_OBJ);
}


/**
* @brief create a json null
*
* @param pInstance the json instance
* @param pId pointer to jsonID
* @return error
*/
jsonErr json_create_null(json_instance_t *pInstance, jsonID *pId)
{
  return s_create_special_types(pInstance, "null", JSON_SUB_NULL, pId);
}


/**
* @brief dec a reference object
*
* @param pInstance the json instance
* @param root the json id
* @return jsonErr
*/
jsonErr json_decref(json_instance_t *pInstance, jsonID root)
{
  jsonErr error = JSON_OK;
  if (root != JSON_ID_NULL)
  {
    json_item_t *pItem = s_find_id(pInstance, root);
    if (pItem == NULL)
    {
      error = JSON_ERROR;
    }
    else
    {
      pItem->ref--;
      if (pItem->ref == 0U)
      {
        if (JSON_MAIN_TYPE(pItem->type) == (uint8_t)JSON_TYPE_LIST)
        {
          for (uint16_t index = 0; error == JSON_OK; index++)
          {
            pItem = s_find_id(pInstance, root);
            json_list_t *pList = JSON_PAYLOAD(json_list_t *, pItem);
            if (index >= (uint8_t)pList->cur)
            {
              break;
            }
            json_pair_t *pPair = JSON_PAYLOAD(json_pair_t *, pList);
            /* fetch id because the registry will change */
            jsonID key = pPair[index].keyId;
            jsonID value = pPair[index].valueId;
            if (json_decref(pInstance, key) != JSON_OK)
            {
              error = JSON_ERROR;
            }
            else if (json_decref(pInstance, value) != JSON_OK)
            {
              error = JSON_ERROR;
            }
            else
            {
              /* do nothing: MISRAC */
            }
          }
        }
        if (error == JSON_OK)
        {
          if (s_del_by_id(pInstance, root) != JSON_OK)
          {
            error = JSON_ERROR;
          }
        }
      }
    }
  }
  JSON_ERR_PRINT(error);
  return error;
}


/**
* @brief  inc a reference object
*
* @param pInstance the json instance
* @param root the json id
* @return jsonErr
*/
jsonErr json_incref(json_instance_t *pInstance, jsonID root)
{
  jsonErr error = JSON_OK;
  if (root != JSON_ID_NULL)
  {
    json_item_t *pItem = s_find_id(pInstance, root);
    if (pItem == NULL)
    {
      error = JSON_ERROR;
    }
    else
    {
      pItem->ref++;
    }
  }
  JSON_ERR_PRINT(error);
  return error;
}

/**
* @brief converts  jsonID as string
*
* @param pInstance the json instance
* @param root the json id
* @param flags the flag
* @return const char_t*
*/
const char_t *json_dumps(json_instance_t *pInstance, jsonID root, uint16_t flags)
{
  json_dump_t dump =
  {
    .szDump  = 0,
    .curDump = 0,
    .pDump   = NULL
  };
  return json_stringify(pInstance, root, flags, &dump);
}



/**
* @brief converts  jsonID as string
*
* @param pInstance the json instance
* @param root the json id
* @param flags the flag
* @return const char_t*
*/
const char_t *json_stringify(json_instance_t *pInstance, jsonID root, uint16_t flags, json_dump_t *pDump)
{
  if (pDump == NULL)
  {
    return NULL;
  }
  json_item_t *pRoot = s_find_id(pInstance, root);
  if (pRoot != NULL)
  {
    uint8_t type = JSON_SUB_TYPE(pRoot->type);
    if ((type == (uint8_t)JSON_SUB_OBJ) || (type == (uint8_t)JSON_SUB_ARRAY))
    {
      s_dump_serialize_indent(pInstance, pDump, flags);
      if (type == (uint8_t)JSON_SUB_OBJ)
      {
        s_dump_serialize_add(pInstance, pDump, "{");
      }
      if (type == (uint8_t)JSON_SUB_ARRAY)
      {
        s_dump_serialize_add(pInstance, pDump, "[");
      }
      json_list_t *pList = JSON_PAYLOAD(json_list_t *, pRoot);
      json_pair_t *pPair = JSON_PAYLOAD(json_pair_t *, pList);
      for (uint16_t a = 0; a < pList->cur; a++)
      {
        if (a != 0U)
        {
          s_dump_serialize_add(pInstance, pDump, ",");
        }
        if (s_serialize_pair(pInstance, pDump, &pPair[a], flags + 1U) != JSON_OK)
        {
          pDump->pDump = NULL;
          break;
        }
      }

      if (pDump->pDump != NULL)
      {
        s_dump_serialize_indent(pInstance, pDump, flags);
        if (type == (uint8_t)JSON_SUB_OBJ)
        {
          s_dump_serialize_add(pInstance, pDump, "}");
        }
        if (type == (uint8_t)JSON_SUB_ARRAY)
        {
          s_dump_serialize_add(pInstance, pDump, "]");
        }
        s_dump_serialize_add(pInstance, pDump, "\0");
      }
    }
  }
  return pDump->pDump;
}



/**
* @brief reads a json stream and load registry
*
* @param pInstance the json instance
* @param pJson the json string to load
* @param pId the json id (output parameter)
* @return error
*/
jsonErr json_load(json_instance_t *pInstance, const char_t *pJson, jsonID *pId)
{
  jsonID root = JSON_ID_NULL;
  jsonErr error = JSON_OK;
  char_t  sChar = '\0';

  if ((pInstance == NULL) || (pJson == NULL) || (pId == NULL))
  {
    error = JSON_ERROR;
  }
  if (error == JSON_OK)
  {
    *pId = JSON_ID_NULL;
    memset(&pInstance->stream, 0, sizeof(pInstance->stream));
    pInstance->stream.pStreamBuffer = pJson;
    pInstance->stream.pStream = pJson;
    pInstance->stream.szStream = (uint16_t)strlen(pJson);
    pInstance->stream.szTmpString = 0;
    pInstance->stream.pTmpString = 0;
    sChar = s_read_stream(&pInstance->stream);
    if ((sChar == '{') || (sChar == '['))
    {
      error = s_json_get_new_id(pInstance, &root, JSON_SUB_OBJ);
    }
    else
    {
      error = JSON_ERROR;
    }
  }
  if (error == JSON_OK)
  {
    error = s_parse_list(pInstance, &pInstance->stream, root);
  }
  if (error == JSON_OK)
  {
    if (pInstance->stream.pTmpString)
    {
      s_free(pInstance->stream.pTmpString);
    }
    char_t eChar = s_read_stream(&pInstance->stream);
    if ((sChar  == '{') && (eChar  == '}'))
    {
      *pId = root;
    }
    else if ((sChar  == '[') && (eChar  == ']'))
    {
      *pId = root;
    }
    else
    {
      error = JSON_ERROR;
    }

  }

  return error;
}


/**
* @brief  Free all
*
* @param pInstance the json instance
* @return jsonErr
*/
jsonErr json_shutdown(json_instance_t *pInstance)
{
  if (pInstance->pRegistry)
  {
    s_free(pInstance->pRegistry);
  }
  memset(pInstance, 0, sizeof(json_instance_t));
  return JSON_OK;
}


/**
* @brief checks if it is true
*
* @param pInstance the json instance
* @param root the json id
* @return boolean
*/
int8_t json_is_true(json_instance_t *pInstance, jsonID root)
{
  return (s_get_sub_type(pInstance, root) == (uint8_t)JSON_SUB_TRUE) ? JSON_TRUE : JSON_FALSE;
}


/**
* @brief checks if it is false
*
* @param pInstance the json instance
* @param root the json id
* @return boolean
*/
int8_t json_is_false(json_instance_t *pInstance, jsonID root)
{
  return (s_get_sub_type(pInstance, root) == (uint8_t)JSON_SUB_FALSE) ? JSON_TRUE : JSON_FALSE;
}


/**
* @brief checks if it is a list
*
* @param pInstance the json instance
* @param root the json id
* @return boolean
*/
int8_t json_is_list(json_instance_t *pInstance, jsonID root)
{
  return (s_get_main_type(pInstance, root) == (uint8_t)JSON_TYPE_LIST) ? JSON_TRUE : JSON_FALSE;
}


/**
* @brief checks if it is an object
*
* @param pInstance the json instance
* @param root the json id
* @return boolean
*/
int8_t json_is_object(json_instance_t *pInstance, jsonID root)
{
  return (s_get_sub_type(pInstance, root) == (uint8_t)JSON_SUB_OBJ) ? JSON_TRUE : JSON_FALSE;
}


/**
* @brief checks if it is an array
*
* @param pInstance the json instance
* @param root the json id
* @return boolean
*/
int8_t json_is_array(json_instance_t *pInstance, jsonID root)
{
  return (s_get_sub_type(pInstance, root) == (uint8_t)JSON_SUB_ARRAY) ? JSON_TRUE : JSON_FALSE;
}


/**
* @brief returns a string from a jsonID
*
* @param pInstance the json instance
* @param the json id
* @param output string
* @return error
*/
jsonErr json_get_string_from_id(json_instance_t *pInstance, jsonID id, const char_t **ppStr)
{
  jsonErr error = JSON_ERROR;
  json_item_t *pItem = s_find_id(pInstance, id);

  if ((ppStr != NULL) && (pItem != NULL) && (JSON_MAIN_TYPE(pItem->type) != (uint8_t)JSON_TYPE_LIST))
  {
    *ppStr = JSON_PAYLOAD(const char_t *, pItem);
    error = JSON_OK;
  }
  return error;
}


/**
* @brief returns a integer from a jsonID
*
* @param pInstance the json instance
* @param the json id
* @param output integer value
* @return error
*/
jsonErr json_get_integer_from_id(json_instance_t *pInstance, jsonID id, int64_t *pVal)
{
  jsonErr error = JSON_ERROR;
  json_item_t *pItem = s_find_id(pInstance, id);

  if ((pVal != NULL) && (pItem != NULL) && (JSON_MAIN_TYPE(pItem->type) != (uint8_t)JSON_TYPE_LIST))
  {
    *pVal = strtoll(JSON_PAYLOAD(const char_t *, pItem), NULL, 10);/*cstat !MISRAC2012-Rule-22.8 !MISRAC2012-Rule-22.9 errno not supported by this OS */
    error = JSON_OK;
  }
  return error;
}


/**
* @brief returns a integer from a jsonID
*
* @param pInstance the json instance
* @param the json id
* @param output integer value
* @return error
*/
jsonErr json_get_unsigned_integer_from_id(json_instance_t *pInstance, jsonID id, uint64_t *pVal)
{
  jsonErr error = JSON_ERROR;
  json_item_t *pItem = s_find_id(pInstance, id);

  if ((pVal != NULL) && (pItem != NULL) && (JSON_MAIN_TYPE(pItem->type) != (uint8_t)JSON_TYPE_LIST))
  {
    *pVal = strtoull(JSON_PAYLOAD(const char_t *, pItem), NULL, 10);/*cstat !MISRAC2012-Rule-22.8 !MISRAC2012-Rule-22.9 not supported by this OS */
    error = JSON_OK;
  }
  return error;
}


/**
* @brief returns a number from a jsonID
*
* @param pInstance the json instance
* @param id the json id
* @param output value
* @return error
*/
jsonErr json_get_number_from_id(json_instance_t *pInstance, jsonID id, double *pVal)
{
  jsonErr error = JSON_ERROR;
  json_item_t *pItem = s_find_id(pInstance, id);

  if ((pVal != NULL) && (pItem != NULL) && (JSON_MAIN_TYPE(pItem->type) != (uint8_t)JSON_TYPE_LIST))
  {
    *pVal = strtod(JSON_PAYLOAD(const char_t *, pItem), NULL);/*cstat !MISRAC2012-Rule-22.8 !MISRAC2012-Rule-22.9 not supported by this OS */
    error = JSON_OK;
  }
  return error;
}


/**
* @brief frees an external resource such as dump
*
* @param ptr the pointer
* @return None
*/
void json_free(void *ptr)
{
  if (ptr)
  {
    s_free(ptr);
  }
}


/**
* @brief realloc an external resource such as dump
*
* @param ptr the pointer
* @param size the pointer size
* @return None
*/
void *json_realoc(void *ptr, size_t size)
{
    return s_realloc(ptr,size);

}


/**
* @brief return the object count from an jsonID list
*
* @param pInstance the json instance
* @param root the json id
* @param pCount is the object count
* @return error
*/
jsonErr json_list_get_count(json_instance_t *pInstance, jsonID root, uint16_t *pCount)
{
  jsonErr error = JSON_ERROR;
  json_item_t *pItem = s_find_id(pInstance, root);

  if ((pCount != NULL) && (pItem != NULL) && (JSON_MAIN_TYPE(pItem->type) == (uint8_t)JSON_TYPE_LIST))
  {
    *pCount = (uint16_t)JSON_PAYLOAD(json_list_t *, pItem)->cur;
    error = JSON_OK;
  }
  return error;
}


/**
* @brief returns the object pair from an jsonID list
*
* @param pInstance the json instance
* @param root the json id
* @param index the array index
* @param pKey the json id key
* @param pValue  the json key value
* @return jsonErr
*/
jsonErr json_list_pair(json_instance_t *pInstance, jsonID root, uint16_t index, jsonID *pKey, jsonID *pValue)
{
  jsonErr error = JSON_OK;
  json_list_t *pList = NULL;
  json_item_t *pItem = s_find_id(pInstance, root);
  jsonID valueId = JSON_ID_NULL;
  jsonID keyId = JSON_ID_NULL;

  if ((pItem == NULL) || (JSON_MAIN_TYPE(pItem->type) != (uint8_t)JSON_TYPE_LIST))
  {
    error = JSON_ERROR;
  }
  if (error == JSON_OK)
  {
    pList = JSON_PAYLOAD(json_list_t *, pItem);
    if ((uint16_t)pList->cur < index)
    {
      error =  JSON_ERROR;
    }
  }
  if (error == JSON_OK)
  {
    json_pair_t *pPair = JSON_PAYLOAD(json_pair_t *, pList);
    keyId = pPair[index].keyId;
    valueId = pPair[index].valueId;
  }
  if (pKey != NULL)
  {
    *pKey = keyId;
  }
  if (pValue != NULL)
  {
    *pValue = valueId;
  }
  JSON_ERR_PRINT(error);
  return error;
}


/**
* @brief delete an array index
*
* @param pInstance the json instance
* @param root the json id
* @param index the array index
* @return jsonErr
*/
jsonErr json_list_del_pair(json_instance_t *pInstance, jsonID root, uint16_t index)
{
  jsonErr error = JSON_OK;
  json_item_t *pItem = s_find_id(pInstance, root);
  if (pItem == NULL)
  {
    error = JSON_ERROR;
  }
  else if (JSON_MAIN_TYPE(pItem->type) != (uint8_t)JSON_TYPE_LIST)
  {
    error = JSON_ERROR;
  }
  else
  {
    json_list_t *pList = JSON_PAYLOAD(json_list_t *, pItem);
    if ((uint16_t)pList->cur < index)
    {
      error = JSON_ERROR;
    }
    else
    {
      json_pair_t *pPair = JSON_PAYLOAD(json_pair_t *, pList);
      /* delete entries */
      if (pPair[index].keyId != JSON_ID_NULL)
      {
        json_decref(pInstance, pPair[index].keyId);
      }
      if (pPair[index].valueId != JSON_ID_NULL)
      {
        json_decref(pInstance, pPair[index].valueId);
      }
      uint32_t offsetD = JSON_OFFSET(pInstance->pRegistry, &pPair[index]);
      uint32_t offsetS = JSON_OFFSET(pInstance->pRegistry, &pPair[index + 1U]);
      uint32_t offsetE = JSON_OFFSET(pInstance->pRegistry, &pPair[pList->cur]);
      int32_t movLen = ((int32_t)offsetE  - (int32_t)offsetS);
      if (movLen > 0)
      {
        memmove(pInstance->pRegistry + offsetD, pInstance->pRegistry + offsetS, (uint32_t)movLen);
      }
      pList->cur--;
    }
  }
  JSON_ERR_PRINT(error);
  return error;
}


/**
* @brief returns the object id from a name
*
* @param pInstance the json instance
* @param root the root id
* @param pTree  the json path separated by a dot
* @param pId  the json id
* @return error
*/
jsonErr json_object_get_id_from_name(json_instance_t *pInstance, jsonID root, const char_t *pName, jsonID *pId)
{
  jsonErr error = JSON_ERROR;
  json_item_t *pItemRoot = s_find_id(pInstance, root);

  if ((pId != NULL) && (pItemRoot != NULL) && (JSON_MAIN_TYPE(pItemRoot->type) == (uint8_t)JSON_TYPE_LIST))
  {
    json_list_t *pList = JSON_PAYLOAD(json_list_t *, pItemRoot);
    json_pair_t *pPair = JSON_PAYLOAD(json_pair_t *, pList);

    *pId = JSON_ID_NULL;
    for (uint16_t a = 0; a < pList->cur; a++)
    {
      json_item_t *pItem = s_find_id(pInstance, pPair[a].keyId);
      if (pItem)
      {
        char_t *pKeyName = JSON_PAYLOAD(char_t *, pItem);
        if (strcmp(pKeyName, pName) == 0)
        {
          *pId = pPair[a].valueId;
          error = JSON_OK;
          break;
        }
      }
    }
  }
  return error;
}


/**
* @brief returns the object id from a name
*
* @param pInstance the json instance
* @param root the root id
* @param pTree  the json path separated by a dot
* @return jsonID
*/
jsonErr json_object_get_id_from_tree(json_instance_t *pInstance, jsonID root, const char_t *pTree, jsonID *pId)
{
  jsonErr error = JSON_OK;
  char_t  tTreeToken[40];
  uint8_t bRet = 0;

  if (pId == NULL)
  {
    error = JSON_ERROR;
  }
  else
  {
    *pId = root;
    while ((strlen(pTree) != 0U) && (bRet == 0U))
    {
      char_t *pNext = strchr(pTree, (int32_t) '.');
      if (pNext)
      {
        uint16_t len = (uint16_t)(pNext - pTree);
        strncpy(tTreeToken, pTree, MIN((sizeof(tTreeToken) - 1U), (len)));/*cstat !MISRAC2012-Rule-12.1  false positive, there is parentheses */
        tTreeToken[len] = '\0';
        pTree = pNext + 1;
      }
      else
      {
        strncpy(tTreeToken, pTree, sizeof(tTreeToken) - 1UL);
        pTree = strchr(pTree, 0);
      }
      jsonID index = JSON_ID_NULL;
      error = s_find_list_entry(pInstance, *pId, tTreeToken, &index);
      if (error == JSON_OK)
      {
        error = json_list_pair(pInstance, *pId, index, NULL, pId);
        if (error != JSON_OK)
        {
          bRet = 1;
        }
      }
      else
      {
        *pId  = JSON_ID_NULL;
        bRet = 1;
      }
    }
  }
  return error;
}


/**
* @brief sets an json id in an object
*
* @param pInstance the json instance
* @param root the json id
* @param pTree  the json path separated by a dot
* @return jsonID
*/
jsonErr json_object_set(json_instance_t *pInstance, jsonID root, const char_t *pTree, jsonID *pId)
{
  jsonErr error = JSON_OK;
  char_t  tTreeToken[40];
  uint8_t bRet = 0;

  if (pId == NULL)
  {
    error = JSON_ERROR;
  }
  else
  {
    *pId = root;
    while ((strlen(pTree) != 0U) && (bRet == 0U))
    {
      char_t *pNext = strchr(pTree, (int32_t) '.');
      if (pNext)
      {
        uint16_t len = (uint16_t)(pNext - pTree);
        strncpy(tTreeToken, pTree, MIN((sizeof(tTreeToken) - 1U), len));/*cstat !MISRAC2012-Rule-12.1  false positive, there is parentheses */
        tTreeToken[len] = '\0';
        pTree = pNext + 1;
      }
      else
      {
        strncpy(tTreeToken, pTree, sizeof(tTreeToken) - 1UL);
        pTree = strchr(pTree, 0);
      }

      jsonID value;
      jsonID index = JSON_ID_NULL;
      if (s_find_list_entry(pInstance, *pId, tTreeToken, &index) != JSON_OK)
      {
        error = s_json_get_new_id(pInstance, &value, JSON_SUB_OBJ);
        JSON_ERR_PRINT(error);
        if (error == JSON_OK)
        {
          error = json_object_set_new(pInstance, *pId, tTreeToken, value);
          if (error != JSON_OK)
          {
            value = JSON_ID_NULL;
            bRet  = 1;
          }
        }
      }
      else
      {
        error = json_list_pair(pInstance, *pId, index, NULL, &value);
        if (error != JSON_OK)
        {
          value = JSON_ID_NULL;
          bRet  = 1;
        }
      }
      *pId = value;
    }
  }
  return error;
}


/**
* @brief gets a object string
*
* @param pInstance the json instance
* @param root the json id
* @param pTree the json path separated by a dot
* @param pKey the json key
* @param ppStr pointer to const char*
* @return error
*/
jsonErr json_object_get_string(json_instance_t *pInstance, jsonID root, const char_t *pTree, const char_t *pKey, const char **ppStr)
{
  jsonID  node  = JSON_ID_NULL;
  jsonErr error = json_object_get_id_from_tree(pInstance, root, pTree, &node);

  if ((ppStr != NULL) && (error == JSON_OK))
  {
    *ppStr = NULL;
    jsonID index = JSON_ID_NULL;
    jsonID value = JSON_ID_NULL;
    error = s_find_list_entry(pInstance, node, pKey, &index);
    if (error == JSON_OK)
    {
      error = json_list_pair(pInstance, node, index, NULL, &value);
    }
    if (error == JSON_OK)
    {
      if (json_is_list(pInstance, value) == JSON_TRUE)
      {
        error = JSON_ERROR;
      }
    }
    if (error == JSON_OK)
    {
      *ppStr = JSON_PAYLOAD(const char_t *, s_find_id(pInstance, value));
    }
  }
  return error;
}


/**
* @brief returns the object string size
*
* @param pInstance the json instance
* @param root the json id
* @param pTree the json path separated by a dot
* @param pKey the key string
* @param pSize the size
* @return error
*/
jsonErr json_object_get_string_size(json_instance_t *pInstance, jsonID root, const char_t *pTree, const char_t *pKey, int32_t *pSize)
{
  jsonID value;
  jsonID node = JSON_ID_NULL;
  jsonErr error = json_object_get_id_from_tree(pInstance, root, pTree, &node);

  if (error == JSON_OK)
  {
    if (pSize == NULL)
    {
      error = JSON_ERROR;
    }
    else
    {
      *pSize = -1; /* Legacy value if not good was -1; now that error is managed, it should be 0 */
    }
  }
  if (error == JSON_OK)
  {
    jsonID index = JSON_ID_NULL;
    error = s_find_list_entry(pInstance, node, pKey, &index);
    if (error == JSON_OK)
    {
      error = json_list_pair(pInstance, node, index, NULL, &value);
      if (error == JSON_OK)
      {
        if (json_is_list(pInstance, value) == JSON_FALSE) /* We don't want it to be a list at that stage */
        {
          json_item_t *pItem = s_find_id(pInstance, value);
          *pSize = (int32_t)pItem->szObj;
        }
        else
        {
          error = JSON_ERROR;
        }
      }
    }
  }
  return error;
}


/**
* @brief returns an integer value
*
* @param pInstance the json instance
* @param root the json id
* @param pTree the json path separated by a dot
* @param pKey the key string
* @param pVal pointer to int64_t
* @return error
*/
jsonErr json_object_get_integer(json_instance_t *pInstance, jsonID root, const char_t *pTree, const char_t *pKey, int64_t *pVal)
{
  json_numData_t signedInt =
  {
    .dataType = JSON_SIGNED64,
    .val.s64  = 0LL,
    .min.s64  = INT64_MIN,
    .max.s64  = INT64_MAX
  };
  jsonErr error = s_object_get_num_data(pInstance, root, pTree, pKey, &signedInt);
  if (error == JSON_OK)
  {
    if (pVal == NULL)
    {
      error = JSON_ERROR;
    }
    else
    {
      *pVal = signedInt.val.s64;
    }
  }
  return error;
}


/**
* @brief returns an unsigned integer value
*
* @param pInstance the json instance
* @param root the json id
* @param pTree the json path separated by a dot
* @param pKey the key string
* @param pVal pointer to uint64_t
* @return error
*/
jsonErr json_object_get_unsigned_integer(json_instance_t *pInstance, jsonID root, const char_t *pTree, const char_t *pKey, uint64_t *pVal)
{
  json_numData_t unsignedInt =
  {
    .dataType = JSON_UNSIGNED64,
    .val.u64  = 0ULL,
    .min.u64  = 0ULL,
    .max.u64  = UINT64_MAX
  };
  jsonErr error = s_object_get_num_data(pInstance, root, pTree, pKey, &unsignedInt);
  if (error == JSON_OK)
  {
    if (pVal == NULL)
    {
      error = JSON_ERROR;
    }
    else
    {
      *pVal = unsignedInt.val.u64;
    }
  }
  return error;
}


/**
* @brief returns a number value
*
* @param pInstance the json instance
* @param root the json id
* @param pTree the json path separated by a dot
* @param pKey the key string
* @param pVal pointer to double
* @return error
*/
jsonErr json_object_get_number(json_instance_t *pInstance, jsonID root, const char_t *pTree, const char_t *pKey, double *pVal)
{
  json_numData_t doubleNumber =
  {
    .dataType = JSON_DOUBLE,
    .val.dbl  = 0.0,
    .min.dbl  = DBL_MIN,
    .max.dbl  = DBL_MAX
  };
  jsonErr error = s_object_get_num_data(pInstance, root, pTree, pKey, &doubleNumber);
  if (error == JSON_OK)
  {
    if (pVal == NULL)
    {
      error = JSON_ERROR;
    }
    else
    {
      *pVal = doubleNumber.val.dbl;
    }
  }
  return error;
}


/**
* @brief returns a bool value
*
* @param pInstance the json instance
* @param root the json id
* @param pTree the json path separated by a dot
* @param pKey the key string
* @param pBool pointer to boolean (true or false)
* @return error
*/
jsonErr json_object_get_boolean(json_instance_t *pInstance, jsonID root, const char_t *pTree, const char_t *pKey, uint8_t *pBool)
{
  const char *pStr;
  jsonErr error = json_object_get_string(pInstance, root, pTree, pKey, &pStr);
  if (error == JSON_OK)
  {
    if (pBool == NULL)
    {
      error = JSON_ERROR;
    }
    else if (strcmp(pStr, "true") == 0)
    {
      *pBool = 1;
    }
    else if (strcmp(pStr, "false") == 0)
    {
      *pBool = 0;
    }
    else
    {
      error = JSON_ERROR;
    }
  }
  return error;
}


/**
* @brief sets a json_numData_t in an object
*
* @param pInstance the json instance
* @param root the json id
* @param pTree the json path separated by a dot
* @param pKey the key string
* @param integer the integer
* @return error
*/
jsonErr json_object_set_num_data(json_instance_t *pInstance, jsonID root, const char_t *pTree, const char_t *pKey, json_numData_t *pNumData)
{
  jsonErr error  = JSON_OK;
  jsonID  node   = JSON_ID_NULL;
  jsonID  index1 = JSON_ID_NULL;
  jsonID  index2 = JSON_ID_NULL;
  /* Move to th root node */
  error = json_object_set(pInstance, root, pTree, &node);
  if (error == JSON_OK)
  {
    /* verify if the node already exist, else delete it, the same key is not allowed in an object */
    if (s_find_list_entry(pInstance, node, pKey, &index1) == JSON_OK)
    {
      if (json_list_del_pair(pInstance, node, index1) != JSON_OK)
      {
        error = JSON_ERROR;
      }
    }
    if (error == JSON_OK)
    {
      /* create a number entry */
      error = s_create_num_data(pInstance, pNumData, &index2);
      if (error == JSON_OK)
      {
        /* create the key value in the object */
        error = json_object_set_new(pInstance, node, pKey, index2);
      }
    }
  }

  return error;
}


/**
* @brief Set a param from string & its type
*
* @param pInstance the json instance
* @param root the json id
* @param pTree the json path separated by a dot
* @param pKey the key string
* @param pString param value string format
* @param param type
* @return error
*/
jsonErr json_object_set_param(json_instance_t *pInstance, jsonID root, const char_t *pTree, const char_t *pKey, const char_t *pString, json_param_type_t const dataType)
{
  jsonErr        error = JSON_OK;
  json_numData_t num_data;

  num_data.dataType = dataType;
  error = s_convert_str_to_num_data(&num_data, pString);
  if (error == JSON_OK)
  {
    error = json_object_set_num_data(pInstance, root, pTree, pKey, &num_data);
  }
  return error;
}


/**
* @brief sets an double in an object
*
* @param pInstance the json instance
* @param root the json id
* @param pTree the json path separated by a dot
* @param pKey the key string
* @param double the double
* @return error
*/
jsonErr json_object_set_number(json_instance_t *pInstance, jsonID root, const char_t *pTree, const char_t *pKey, double number)
{
  json_numData_t doubleNumber =
  {
    .dataType = JSON_DOUBLE,
    .val.dbl  = number,
    .min.dbl  = DBL_MIN,
    .max.dbl  = DBL_MAX
  };
  return json_object_set_num_data(pInstance, root, pTree, pKey, &doubleNumber);
}


/**
* @brief sets an unsigned integer in an object
*
* @param pInstance the json instance
* @param root the json id
* @param pTree the json path separated by a dot
* @param pKey the key string
* @param unsigned integer the unsigned integer
* @return error
*/
jsonErr json_object_set_unsigned_integer(json_instance_t *pInstance, jsonID root, const char_t *pTree, const char_t *pKey, uint64_t integer)
{
  json_numData_t unsignedInt =
  {
    .dataType = JSON_UNSIGNED64,
    .val.u64  = integer,
    .min.u64  = 0ULL,
    .max.u64  = UINT64_MAX
  };
  return json_object_set_num_data(pInstance, root, pTree, pKey, &unsignedInt);
}


/**
* @brief sets an integer in an object
*
* @param pInstance the json instance
* @param root the json id
* @param pTree the json path separated by a dot
* @param pKey the key string
* @param integer the integer
* @return error
*/
jsonErr json_object_set_integer(json_instance_t *pInstance, jsonID root, const char_t *pTree, const char_t *pKey, int64_t integer)
{
  json_numData_t signedInt =
  {
    .dataType = JSON_SIGNED64,
    .val.s64  = integer,
    .min.s64  = INT64_MIN,
    .max.s64  = INT64_MAX
  };
  return json_object_set_num_data(pInstance, root, pTree, pKey, &signedInt);
}


/**
* @brief set  a boolean
*
* @param pInstance the json instance
* @param root the json id
* @param pTree the json path separated by a dot
* @param pKey the key string
* @param state  the state
* @return error
*/
jsonErr json_object_set_boolean(json_instance_t *pInstance, jsonID root, const char_t *pTree, const char_t *pKey, uint8_t state)
{
  jsonErr error = JSON_OK;
  jsonID  node = JSON_ID_NULL;
  error = json_object_set(pInstance, root, pTree, &node);

  if (error == JSON_OK)
  {
    jsonID  index = JSON_ID_NULL;
    if (s_find_list_entry(pInstance, node, pKey, &index) == JSON_OK)
    {
      error = json_list_del_pair(pInstance, node, index);
    }
    if (error == JSON_OK)
    {
      error = json_create_boolean(pInstance, (int8_t)state, &index);
    }
    if (error == JSON_OK)
    {
      error = json_object_set_new(pInstance, node, pKey, index);
    }
  }
  return error;
}


/**
* @brief sets an object string
*
* @param pInstance the json instance
* @param root the json id
* @param pTree the json path separated by a dot
* @param pKey the key string
* @param pString  the string
* @return jsonID
*/
jsonErr json_object_set_string(json_instance_t *pInstance, jsonID root, const char_t *pTree, const char_t *pKey, const char_t *pString)
{
  jsonID ret = JSON_ID_NULL; /* Dummy stuff (to keep old behavior); should be removed when legacy API is removed */
  jsonErr error = s_object_set_string(pInstance, root, pTree, pKey, pString, &ret);
  return error;
}


/**
* @brief checks if a list exists
*
* @param pInstance the json instance
* @param root the json id
* @param pTree the json path separated by a dot
* @param pKey the key string
* @return jsonErr
*/
jsonErr json_object_check_list_key(json_instance_t *pInstance, jsonID root, const char_t *pTree, const char_t *pKey)
{
  jsonErr error;
  jsonID node = JSON_ID_NULL;
  error = json_object_set(pInstance, root, pTree, &node);

  if (error == JSON_OK)
  {
    jsonID index = JSON_ID_NULL;
    error = s_find_list_entry(pInstance, node, pKey, &index);
  }
  return error;
}


/**
* @brief gets an array
*
* @param pInstance the json instance
* @param root the json id
* @param pTree the json path separated by a dot
* @return jsonID
*/
jsonErr json_array_get(json_instance_t *pInstance, jsonID root, const char_t *pTree, jsonID *pId)
{
  jsonErr error = JSON_OK;
  char_t  tTreeToken[40];

  if (pId == NULL)
  {
    error = JSON_ERROR;
  }
  if (error == JSON_OK)
  {
    *pId = root;
    while ((strlen(pTree) != 0U) && (*pId != JSON_ID_NULL))
    {
      char_t *pNext = strchr(pTree, (int32_t)'.');
      if (pNext)
      {
        uint16_t len = (uint16_t)(pNext - pTree);
        strncpy(tTreeToken, pTree, MIN(sizeof((tTreeToken) - 1U), len));/*cstat !MISRAC2012-Rule-12.1  false positive, there is parentheses */
        tTreeToken[len] = '\0';
        pTree = pNext + 1;
      }
      else
      {
        strncpy(tTreeToken, pTree, sizeof(tTreeToken) - 1UL);
        pTree = strchr(pTree, 0);
      }

      jsonID index = JSON_ID_NULL;
      error = s_find_list_entry(pInstance, *pId, tTreeToken, &index);
      if (error == JSON_OK)
      {
        error = json_list_pair(pInstance, *pId, index, NULL, pId);
      }
      else
      {
        *pId = JSON_ID_NULL;
      }
    }
  }
  return error;
}


/**
* @brief set a json array
*
* @param pInstance the json instance
* @param root the json id
* @param pTree the json path separated by a dot
* @return jsonID
*/
jsonErr json_array_set(json_instance_t *pInstance, jsonID root, const char_t *pTree, jsonID *pId)
{
  jsonErr error = JSON_OK;
  if (pId == NULL)
  {
    error = JSON_ERROR;
  }
  else
  {
    *pId = JSON_ID_NULL;
    error = json_object_set(pInstance, root, pTree, pId);
  }
  if ((error == JSON_OK) && (*pId != JSON_ID_NULL))
  {
    json_item_t *pItem = s_find_id(pInstance, *pId);
    if (pItem != NULL)
    {
      pItem->type = (uint8_t)JSON_MAKE_TYPE(JSON_TYPE_LIST, JSON_SUB_ARRAY);
    }
  }
  return error;
}


/* Static Functions Definition -----------------------------------------------*/


#ifdef JSON_LOCAL_ITOA_USED
/**
* @brief converts int64 as string for nano lib gcc support
*
* @param val numeric value
* @param pString the string buffer
* @param szString the string size buffer
*/
static void s_itoa(int64_t val, char_t *pString, uint32_t szString)
{
  uint64_t uval;
  if (val < 0)
  {
    uval = (uint64_t)(-val);
    *pString = '-';
    pString++;
    szString--;
  }
  else
  {
    uval = (uint64_t)val;
  }
  s_uitoa(uval, pString, szString);
}


/**
* @brief converts uint64 as string for nano lib gcc support
*
* @param val numeric value
* @param pString the string buffer
* @param szString the string size buffer
*/
static void s_uitoa(uint64_t val, char_t *pString, uint32_t szString)
{
  char_t tBuffer[20];
  uint32_t index = 0;
  do
  {
    uint32_t v = val % 10;
    tBuffer[index] = '0' + v;
    index++;
    val /= 10;
  }
  while (val != 0);
  while (szString && index)
  {
    index--;
    *pString = tBuffer[index];
    pString++;
    szString--;
  }
  *pString = 0;
}
#endif


/**
* @brief parses an element ( string, block , token )
*
* @param pInstance the json instance
* @param pStream  the stream instance
* @return jsonID
*/
static jsonErr s_parse_element(json_instance_t *pInstance, json_parse_stream_t *pStream, jsonID *pId)
{
  jsonErr error = JSON_OK;
  char_t ch = s_read_stream(pStream);

  if (pId == NULL)
  {
    error = JSON_ERROR;
  }
  else
  {
    *pId = JSON_ID_NULL;

    /* check string */
    if (ch == '\"')
    {
      s_read_rewind(pStream);
      error = s_get_string_quoted(pInstance, pStream, pId);
    }
    else if (ch == '{') /* obj */
    {
      error = s_json_get_new_id(pInstance, pId, JSON_SUB_OBJ);
      if (error == JSON_OK)
      {
        error = s_parse_list(pInstance, pStream, *pId);
      }

      if (error == JSON_OK)
      {
        if (s_read_stream(pStream) != '}')
        {
          *pId = JSON_ID_NULL;
        }
      }
    }
    else if (ch == '[') /* check array */
    {
      error = s_json_get_new_id(pInstance, pId, JSON_SUB_ARRAY);
      if (error == JSON_OK)
      {
        error = s_parse_list(pInstance, pStream, *pId);
      }

      if (error == JSON_OK)
      {
        if (s_read_stream(pStream) != ']')
        {
          *pId = JSON_ID_NULL;
        }
      }
    }
    else
    {
      /* at this point; we should have a token or a numeric */
      s_read_rewind(pStream); /* rewind to start in the first char */
      int32_t nbChar;
      error = s_read_stream_copy_until(pStream, JSON_END_WHILE_ALPHA_NUM, &nbChar);
      if (error == JSON_OK)
      {
        if (strlen(pStream->pTmpString) == 0U)
        {
          /* illegal */
          error = JSON_ERROR;
        }
        else if (strcmp(pStream->pTmpString, "null") == 0)
        {
          error = json_create_null(pInstance, pId);
        }
        else if (strcmp(pStream->pTmpString, "true") == 0)
        {
          error = json_create_true(pInstance, pId);
        }
        else if (strcmp(pStream->pTmpString, "false") == 0)
        {
          error = json_create_false(pInstance, pId);
        }
        else if (s_is_int_string(pStream->pTmpString))
        {
          error = s_create_and_get_string_id(pInstance, pStream->pTmpString, 0, pId);
          if (error == JSON_OK)
          {
            s_set_type(pInstance, *pId, JSON_MAKE_TYPE(JSON_TYPE_VALUE, JSON_SUB_INT));
          }
        }
        else if (s_is_number_string(pStream->pTmpString))
        {
          error = json_create_number(pInstance, strtod(pStream->pTmpString, NULL), pId);/*cstat !MISRAC2012-Rule-22.8 !MISRAC2012-Rule-22.9 errno not supported by this OS */
          if (error == JSON_OK)
          {
            s_set_type(pInstance, *pId, JSON_MAKE_TYPE(JSON_TYPE_VALUE, JSON_SUB_NUMBER));
          }
        }
        else
        {
          /* do nothing: MISRAC */
        }
      }
      else
      {
        /* read error */
        error = JSON_ERROR; /* This is new; check if it breaks */
      }
    }
  }
  JSON_ERR_PRINT(error);
  return error;
}


/**
* @brief generates an ID
*
* @param pInstance json instance
* @return jsonID json id
*/
static jsonID s_generate_id(json_instance_t *pInstance)
{
  pInstance->gItemID++; /* new ID starts always at JSON_ID_NULL + 1*/
  return pInstance->gItemID;
}


/**
* @brief  returns a start item for an enumeration
*
* @param pInstance json instance
* @return json_item_t*
*/
static json_item_t *s_start_registry(json_instance_t *pInstance)
{
  return (json_item_t *)pInstance->pRegistry;
}


/**
* @brief  returns the  next  item for an enumeration
*
* @param pInstance json instance
* @param pReg the item
* @return json_item_t*
*/
static json_item_t *s_get_registry_next(json_instance_t *pInstance, json_item_t *pReg)
{
  json_item_t *pRet = (json_item_t *)(((char_t *)pReg) + pReg->szObj);
  if ((char_t *)pRet >= (pInstance->pRegistry + pInstance->curRegistry))
  {
    pRet = NULL;
  }
  return pRet;
}


/**
* @brief returns a sub object name string
*
* @param type  the type
* @return const char_t* the type string
*/

static const char_t *s_get_type_name(uint8_t type)
{
  char_t const *pChar;

  switch (JSON_SUB_TYPE(type))
  {
    case JSON_SUB_STRING:
      pChar = "STRING";
      break;
    case JSON_SUB_INT:
      pChar = "INT";
      break;
    case JSON_SUB_OBJ:
      pChar = "OBJ";
      break;
    case JSON_SUB_ARRAY:
      pChar = "ARRAY";
      break;
    default:
      pChar = "REG_UKNOW";
      break;
  }

  return pChar;
}









/**
* @brief returns the item from a string from its subtype
*
* @param pInstance the json instance
* @param str_value the string value
* @return json_item_t*
*/
static json_item_t *s_find_string_subtype(json_instance_t *pInstance, const char_t *str_value, json_sub_type subtype)
{
  json_item_t *pRes = s_start_registry(pInstance);
  while (pRes)
  {
    if (JSON_SUB_TYPE(pRes->type) == (uint8_t)subtype)
    {
      const char_t *pString = JSON_PAYLOAD(const char_t *, pRes);
      if (*pString == *str_value)
      {
        if (strcmp(pString, str_value) == 0)
        {
          break;
        }
      }
    }
    pRes = s_get_registry_next(pInstance, pRes);
  }
  return pRes;
}

/**
* @brief returns the item from a string
*
* @param pInstance the json instance
* @param str_value the string value
* @return json_item_t*
*/
static json_item_t *s_find_string(json_instance_t *pInstance, const char_t *str_value)
{
  return s_find_string_subtype(pInstance, str_value, JSON_SUB_STRING);
}




/**
* @brief returns the item string
*
* @param pInstance the json instance
* @param str_value  the string value
* @param szvalue the string value size
* @return json_item_t*
*/
static json_item_t *s_create_string(json_instance_t *pInstance, const char_t *str_value, uint32_t szvalue)
{
  /* check if exiting */
  json_item_t *pString = s_find_string(pInstance, str_value);
  if (pString)
  {
    pString->ref++;
  }
  else
  {
    if (szvalue == 0U)
    {
      szvalue = strlen(str_value) + 1U;
    }
    pString = s_add_id(pInstance, (uint16_t)szvalue);
    if (pString)
    {
      memmove(JSON_PAYLOAD(char_t *, pString), str_value, szvalue);
      pString->type = JSON_MAKE_TYPE(JSON_TYPE_VALUE, JSON_SUB_STRING);
    }
  }
  return pString;
}


/**
* @brief returns an item pointer from an id
*
* @param pInstance the json instance
* @param id the json ID
* @return json_item_t*
*/
static json_item_t *s_find_id(json_instance_t *pInstance, jsonID id)
{
  json_item_t *pRes = s_start_registry(pInstance);
  while (pRes)
  {
    if (pRes->id == id)
    {
      break;
    }
    pRes = s_get_registry_next(pInstance, pRes);
  }
  return pRes;
}


/**
* @brief checks if the registry is empty and free the malloc
*
* @param pInstance the json instance
* @return jsonErr
*/
static jsonErr s_check_shrink(json_instance_t *pInstance)
{
  if (pInstance->curRegistry == 0U)
  {
    s_free(pInstance->pRegistry);
    pInstance->szRegistry = 0;
    pInstance->pRegistry = NULL;
  }
  return JSON_OK;
}


/**
* @brief  checks and grow the registry allocator if needed
*
* @param pInstance the json instance
* @param grow  the size to grow
* @return jsonErr
*/
static jsonErr s_check_grow(json_instance_t *pInstance, uint16_t grow)
{
  jsonErr error = JSON_OK;
  if ((pInstance->curRegistry + grow) >= pInstance->szRegistry)
  {
    uint32_t szGrow = pInstance->curRegistry + grow + JSON_GROW_REG;
    pInstance->pRegistry = (char_t *)s_realloc(pInstance->pRegistry, szGrow);
    if (pInstance->pRegistry == NULL)
    {
      error = JSON_ERR_MEM;
    }
    if (error == JSON_OK)
    {
      pInstance->szRegistry = szGrow;
    }
  }
  JSON_ERR_PRINT(error);
  return error;
}


/**
* @brief adds an item/id to the registry
*
* @param pInstance the json instance
* @param len the size
* @return json_item_t*
*/
static json_item_t *s_add_id(json_instance_t *pInstance, uint16_t len)
{
  json_item_t *pRes = NULL;
  if (s_check_grow(pInstance, len + (uint16_t)sizeof(json_item_t)) == JSON_OK)
  {
    /* Align the len to a short */
    len += 1U;
    len &= ~1U;
    len += (uint16_t)sizeof(json_item_t);

    pRes = (json_item_t *)(pInstance->pRegistry + pInstance->curRegistry);
    memset(pRes, 0, len);
    pRes->id = s_generate_id(pInstance);
    pRes->szObj = len;
    pRes->ref = 1;
    pInstance->curRegistry += len;
  }
  return pRes;
}


/**
* @brief set  the type on an item
*
* @param pInstance the json instance
* @param id the json id
* @param type the json type
* @return jsonErr
*/
static jsonErr s_set_type(json_instance_t *pInstance, jsonID id, uint8_t type)
{
  jsonErr error = JSON_OK;
  json_item_t *pRef = s_find_id(pInstance, id);
  if (pRef == NULL)
  {
    error = JSON_ERR_FOUND;
  }
  else
  {
    pRef->type = type;
  }
  JSON_ERR_PRINT(error);
  return error;
}


/**
* @brief returns the main type
*
* @param pInstance the json instance
* @param id the json id
* @return uint8_t
*/
static uint8_t s_get_main_type(json_instance_t *pInstance, jsonID id)
{
  uint8_t ret = 0;
  json_item_t *pRef = s_find_id(pInstance, id);
  if (pRef == NULL)
  {
    ret = (uint8_t) -1;
  }
  else
  {
    ret = JSON_MAIN_TYPE(pRef->type);
  }
  return ret;
}


/**
* @brief return the sub type
*
* @param pInstance the json instance
* @param id the json id
* @return uint8_t
*/
static uint8_t s_get_sub_type(json_instance_t *pInstance, jsonID id)
{
  uint8_t ret = 0;
  json_item_t *pRef = s_find_id(pInstance, id);
  if (pRef == NULL)
  {
    ret = (uint8_t) -1;
  }
  else
  {
    ret = JSON_SUB_TYPE(pRef->type);
  }
  return ret;
}


/**
* @brief deletes an item/id from the registry
*
* @param pInstance the json instance
* @param id the json id
* @return jsonErr
*/
static jsonErr s_del_by_id(json_instance_t *pInstance, jsonID id)
{
  json_item_t *pRef = s_find_id(pInstance, id);
  json_item_t *pNext = (json_item_t *)(((char_t *)pRef) + pRef->szObj);
  uint32_t lenRec = pRef->szObj;
  uint32_t offsetD = JSON_OFFSET(pInstance->pRegistry, pRef);
  uint32_t offsetS = JSON_OFFSET(pInstance->pRegistry, pNext);

  uint32_t movLen = JSON_OFFSET(pInstance->pRegistry, &pInstance->pRegistry[pInstance->szRegistry]) - offsetS;
  memmove(pInstance->pRegistry + offsetD, pInstance->pRegistry + offsetS, movLen);
  pInstance->curRegistry -= lenRec;
  s_check_shrink(pInstance);
  return JSON_OK;
}


/**
* @brief grows a item list if mandatory
*
* @param pInstance the json instance
* @param root the toot id
* @param grow the size to grow
* @return jsonErr
*/
static jsonErr s_grow_list(json_instance_t *pInstance, jsonID root, uint16_t grow)
{
  jsonErr error = JSON_OK;
  json_list_t *pList = NULL;
  json_item_t *pRoot = s_find_id(pInstance, root);

  if (pRoot == NULL)
  {
    error = JSON_ERR_FOUND;
  }
  else
  {
    pList = JSON_PAYLOAD(json_list_t *, pRoot);
    if ((pList->cur + 1U) < pList->max)
    {
      /* nothing more to do */
    }
    else
    {
      error = s_check_grow(pInstance, grow * (uint16_t)sizeof(json_pair_t));
      if (error == JSON_OK)
      {
        /* re acquire after grow */
        pRoot = s_find_id(pInstance, root);
        if (pRoot == NULL)
        {
          error = JSON_ERR_FOUND;
        }
        else
        {
          /* Get the list info */
          pList = JSON_PAYLOAD(json_list_t *, pRoot);

          /* Get the first pair */
          json_pair_t *pPair = JSON_PAYLOAD(json_pair_t *, pList);
          /* Get the last pair */
          json_pair_t *pEnd = &pPair[pList->max];
          /* compute start and destination move to enlarge the hole */
          uint32_t offsetS = JSON_OFFSET(pInstance->pRegistry, pEnd);
          uint32_t offsetD = JSON_OFFSET(pInstance->pRegistry, &pEnd[grow]);
          uint32_t movLen = JSON_OFFSET(pInstance->pRegistry, &pInstance->pRegistry[pInstance->szRegistry]) - offsetS;
          movLen -= grow * sizeof(json_pair_t);
          memmove(pInstance->pRegistry + offsetD, pInstance->pRegistry + offsetS, movLen);
          /*update the max in the list */
          pList->max += grow;
          pRoot->szObj += grow * (uint16_t)sizeof(json_pair_t);
          pInstance->curRegistry += grow * sizeof(json_pair_t);
        }
      }
    }
  }
  JSON_ERR_PRINT(error);
  return error;
}


/**
* @brief adds a string to the dump buffer
*
* @param pInstance the json instance
* @param pDump the dump string
* @param pString  the string to add
* @return jsonErr
*/
static jsonErr s_dump_serialize_add(json_instance_t *pInstance, json_dump_t *pDump, const char_t *pString)
{
  jsonErr error = JSON_OK;
  uint32_t len = (uint32_t)strlen(pString);
  if ((pDump->curDump + len) >= pDump->szDump)
  {
    pDump->szDump += JSON_DUMP_GROW;
    pDump->pDump = (char_t *)s_realloc(pDump->pDump, pDump->szDump);
    if (pDump->pDump == NULL)
    {
      error = JSON_ERROR;
    }
  }
  if (error == JSON_OK)
  {
    strcpy(pDump->pDump + pDump->curDump, pString);
    pDump->curDump += len;
  }
  JSON_ERR_PRINT(error);
  return error;
}


/**
* @brief Generate the dump indentation
*
* @param pInstance the json instance
* @param pDump the dump buffer
* @param index the level of indentation
* @return jsonErr
*/
static jsonErr s_dump_serialize_indent(json_instance_t *pInstance, json_dump_t *pDump, uint16_t index)
{
  jsonErr error = JSON_OK;
  if ((index & JSON_INDENT) != 0U)
  {
    index &= (uint16_t)~JSON_INDENT;
    error |= s_dump_serialize_add(pInstance, pDump, "\n");
    for (uint32_t a = 0; a < index; a++)
    {
      error |= s_dump_serialize_add(pInstance, pDump, "  ");
    }
  }
  JSON_ERR_PRINT(error);
  return error;
}


/**
* @brief contaminates  a string to the dump buffer
*
* @param pInstance
* @param pDump
* @param pString
*/
static void s_dump_serialize_add_string(json_instance_t *pInstance, json_dump_t *pDump, const char_t *pString)
{
  const char_t *pText;
  s_dump_serialize_add(pInstance, pDump, "\"");

  if (pString)
  {
    while (*pString)
    {
      pText = json_encode_char(*pString);
      s_dump_serialize_add(pInstance, pDump, pText);
      pString++;
    }
  }
  s_dump_serialize_add(pInstance, pDump, "\"");
}


/**
* @brief  Dumps a pair of json object  recursively
*
* @param pInstance the json instance
* @param pDump  the dump buffer
* @param pPair  the json item
* @param index  the indent level
* @return jsonErr
*/
static jsonErr s_serialize_pair(json_instance_t *pInstance, json_dump_t *pDump, json_pair_t *pPair, uint16_t index)
{
  jsonErr error = JSON_OK;
  json_item_t *pKey;
  if (pPair->keyId != JSON_ID_NULL)
  {
    pKey = s_find_id(pInstance, pPair->keyId);
    if (pKey == NULL)
    {
      error = JSON_ERROR;
    }
    else
    {
      s_dump_serialize_indent(pInstance, pDump, index);
      s_dump_serialize_add_string(pInstance, pDump, JSON_PAYLOAD(char_t *, pKey));
      s_dump_serialize_add(pInstance, pDump, ":");
    }
  }
  if (error == JSON_OK)
  {
    json_item_t *pValue = s_find_id(pInstance, pPair->valueId);
    if (pValue == NULL)
    {
      error = JSON_ERROR;
    }
    else if (JSON_MAIN_TYPE(pValue->type) == (uint8_t)JSON_TYPE_LIST)
    {
      if (JSON_SUB_TYPE(pValue->type) == (uint8_t)JSON_SUB_ARRAY)
      {
        s_dump_serialize_add(pInstance, pDump, "[");
      }
      if (JSON_SUB_TYPE(pValue->type) == (uint8_t)JSON_SUB_OBJ)
      {
        s_dump_serialize_indent(pInstance, pDump, index);
        s_dump_serialize_add(pInstance, pDump, "{");
      }
      json_list_t *pList = JSON_PAYLOAD(json_list_t *, pValue);
      json_pair_t *pPair2 = JSON_PAYLOAD(json_pair_t *, pList);
      for (uint16_t a = 0; a < pList->cur; a++)
      {
        if (a != 0U)
        {
          s_dump_serialize_add(pInstance, pDump, ",");
        }
        if (s_serialize_pair(pInstance, pDump, &pPair2[a], index + 1U) != JSON_OK)
        {
          error = JSON_ERROR;
          break;
        }
      }
      if (error == JSON_OK)
      {
        if (JSON_SUB_TYPE(pValue->type) == (uint8_t)JSON_SUB_ARRAY)
        {
          s_dump_serialize_add(pInstance, pDump, "]");
        }
        if (JSON_SUB_TYPE(pValue->type) == (uint8_t)JSON_SUB_OBJ)
        {
          s_dump_serialize_indent(pInstance, pDump, index);
          s_dump_serialize_add(pInstance, pDump, "}");
        }
      }
    }
    else
    {
      if (JSON_SUB_TYPE(pValue->type) == (uint8_t)JSON_SUB_STRING)
      {
        s_dump_serialize_add_string(pInstance, pDump, JSON_PAYLOAD(char_t *, pValue));
      }
      else if (JSON_SUB_TYPE(pValue->type) == (uint8_t)JSON_SUB_TRUE)
      {
        s_dump_serialize_add(pInstance, pDump, "true");
      }
      else if (JSON_SUB_TYPE(pValue->type) == (uint8_t)JSON_SUB_FALSE)
      {
        s_dump_serialize_add(pInstance, pDump, "false");
      }
      else if (JSON_SUB_TYPE(pValue->type) == (uint8_t)JSON_SUB_NULL)
      {
        s_dump_serialize_add(pInstance, pDump, "null");
      }
      else if (JSON_SUB_TYPE(pValue->type) == (uint8_t)JSON_SUB_INT)
      {
        s_dump_serialize_add(pInstance, pDump, JSON_PAYLOAD(char_t *, pValue));
      }
      else if (JSON_SUB_TYPE(pValue->type) == (uint8_t)JSON_SUB_NUMBER)
      {
        s_dump_serialize_add(pInstance, pDump, JSON_PAYLOAD(char_t *, pValue));
      }
      else
      {
        error = JSON_ERROR;
      }
    }
  }
  JSON_ERR_PRINT(error);
  return error;
}


/**
* @brief checks if it is a  separator
*
* @param ch the char
* @return uint8_t TRUE or FALSE
*/
static int8_t s_is_white_space(char_t ch)
{
  return ((ch == ' ') || (ch == '\t') || (ch == '\r') || (ch == '\n')) ? 1 : 0;
}


/**
* @brief  checks if it is the end of stream
*
* @param pStream the stream instance
* @return jsonErr
*/
static jsonErr s_read_stream_eof(json_parse_stream_t *pStream)
{
  jsonErr error = JSON_OK;
  if (pStream->pStream >= (pStream->pStream + pStream->szStream))
  {
    error = JSON_ERROR;
  }
  JSON_ERR_PRINT(error);
  return error;
}


/**
* @brief Move to the next char
*
* @param pStream the stream instance
* @return jsonErr
*/
static jsonErr s_read_stream_next_char(json_parse_stream_t *pStream)
{
  pStream->pStream++;
  return s_read_stream_eof(pStream);
}


/**
* @brief seeks to the net char
*
* @param pStream the stream instance
* @return jsonErr
*/
static jsonErr s_read_stream_seek_next(json_parse_stream_t *pStream)
{
  jsonErr error = JSON_OK;
  while (error == JSON_OK)
  {
    if (s_is_white_space(*pStream->pStream) == 0)
    {
      break;
    }
    error = s_read_stream_next_char(pStream);
  }
  JSON_ERR_PRINT(error);
  return error;
}


/**
* @brief rewin the stream of 1 char skipping white space
*
* @param pStream
*/
static void s_read_rewind(json_parse_stream_t *pStream)
{
  const char_t *pBuff = pStream->pStream;
  while (pBuff > pStream->pStreamBuffer)
  {
    pBuff--;
    if (pBuff != pStream->pStreamBuffer)
    {
      if (s_is_white_space(*pBuff) == 0)
      {
        break;
      }
    }
  }
  pStream->pStream = pBuff;
}


/**
* @brief encodes a char using escape
*
* @param ch the char
* @return const char_t*
*/
const char_t *json_encode_char(char_t ch)
{
  static char_t tChar[10] = "A";
  tChar[0] = ch;
  tChar[1] = '\0';
  const char *pText = tChar;
  #ifdef JSON_ENCODE_STRING
  switch (ch)
  {
    case '\\':
      pText = "\\\\";
      break;
    case '\"':
      pText = "\\\"";
      break;
    case '/':
      pText = "\\/";
      break;
    case '\b':
      pText = "\\b";
      break;
    case '\f':
      pText = "\\f";
      break;
    case '\n':
      pText = "\\n";
      break;
    case '\r':
      pText = "\\r";
      break;
    case '\t':
      pText = "\\t";
      break;
    default:
      if (ch < ' ')
      {
        snprintf(tChar, sizeof(tChar), "\\\\u%x", ch);
      }
      pText = tChar;
      break;
  }
  #endif
  return pText;
}

static jsonErr s_decode_char(json_parse_stream_t *pStream, char_t ch, char_t **pBuffer, uint16_t *index)
{
  jsonErr error = JSON_OK;

  int32_t  szDec = 0;
  if (ch == '\\')
  {
    s_read_rewind(pStream);
    ch = (char_t)json_decode_char(pStream->pStream, &szDec);
    for (int32_t index = 0; index < szDec; index++)
    {
      if (s_read_stream_next_char(pStream) != JSON_OK)
      {
        error = JSON_ERROR;
      }
    }
  }
  if (error == JSON_OK)
  {
    *(*pBuffer) = ch;
    (*pBuffer)++;
    (*index)++;
  }
  JSON_ERR_PRINT(error);
  return error;
}



/**
* @brief encodes a char using escape
*
* @param pText the string
* @return the char decoded
*/
uint32_t json_decode_char(const char_t *pText, int32_t *pToken)
{
  uint32_t ch = 0;
  const char_t *pTextStart = pText;
  if (*pText == '\\')
  {
    pText++;
    char_t char2 = *pText++;
    switch (char2)
    {
      case 'b':
        ch = (uint32_t)'\b';
        break;

      case '\"':
        ch = (uint32_t)'\"';
        break;

      case '\\':
        ch = (uint32_t)'\\';
        switch (*pText)
        {
          default:
          {
            break;
          }
          case 'u':
          {
            pText++;
            ch = 0;
            while (1)
            {
              char_t char3 = (char_t)toupper((int32_t) * pText++);
              int32_t val = 0;
              if (isdigit((int32_t)char3))/*cstat  !MISRAC2012-Dir-4.11_h the stdlib macro  isdigit produce this error, we cannot fix it */
              {
                val  = (int32_t)(char3 - '0');
              }
              else if ((char3 >= 'A') && (char3 <= 'F'))
              {
                val  = (char3 - 'A') + 10;
              }
              else
              {
                pText--;
                break;
              }
              ch <<= 4U;
              ch |= (uint8_t)val;
            }
            break;

          }
        }
        break;

      case 'f':
        ch = (uint32_t)'\f';
        break;

      case '/':
        ch = (uint32_t)'/';
        break;

      case 'n':
        ch = (uint32_t)'\n';
        break;

      case 'r':
        ch = (uint32_t)'\r';
        break;

      case 't':
        ch = (uint32_t)'\t';
        break;

      default:
        ch = (uint32_t)'?';
        break;
    }
  }
  else
  {
    ch = (uint32_t) * pText++;
  }
  if (pToken)
  {
    *pToken = (int32_t)(pText - pTextStart);
  }
  return ch;
}


/**
* @brief reads a char from the stream and skip space chars
*
* @param pStream the stream instance
* @return char_t
*/
static char_t s_read_stream(json_parse_stream_t *pStream)
{
  char_t ch = '\0';
  if (pStream != NULL)
  {
    if (s_read_stream_seek_next(pStream) == JSON_OK)
    {
      ch = *pStream->pStream;
      pStream->pStream++;
    }
  }
  return ch;
}


/**
* @brief copies the string until the pattern is found
*
* @param pStream the stream instance
* @param separator the separator
* @return int32_t
*/
static jsonErr s_read_stream_copy_until(json_parse_stream_t *pStream, char_t separator, int32_t *pNbChar)
{
  jsonErr error = JSON_OK;
  uint8_t end_string = 0U;
  uint32_t szBuffer = JSON_MAX_STRING_SIZE - 1U; /* /0*/
  char_t *pBuffer = pStream->pTmpString;
  uint16_t index = 0;

  while ((index < szBuffer) && (end_string == 0U) && (error == JSON_OK))
  {
    if ((pBuffer == NULL) || ((index + 1U) >= pStream->szTmpString))
    {
      pStream->szTmpString += JSON_STRING_GROW;
      pStream->pTmpString = s_realloc(pStream->pTmpString, pStream->szTmpString);
      if (pStream->pTmpString == NULL)
      {
        error = JSON_ERROR;
      }
      pBuffer = pStream->pTmpString + index;
    }
    if (error == JSON_OK)
    {
      char_t ch = *pStream->pStream;

      error = s_read_stream_next_char(pStream);
      if (error != JSON_OK)
      {
        *pBuffer = '\0';
        pBuffer++;
        end_string = 1U;
      }
      else
      {
        switch (separator)
        {
          case JSON_END_QUOTE:
            if (ch == '\"')
            {
              end_string = 1U;
            }
            break;

          case JSON_END_DIGIT:
            if (isdigit((int32_t)ch) == 0) /*cstat  !MISRAC2012-Dir-4.11_h the stdlib macro  isdigit produce this error, we cannot fix it */
            {
              pStream->pStream--;
              end_string = 1U;
            }
            break;

          case JSON_END_ALPHA:
            if (isalpha((int32_t)ch) == 0) /*cstat  !MISRAC2012-Dir-4.11_h the stdlib macro  isdigit produce this error, we cannot fix it */
            {
              pStream->pStream--;
              end_string = 1U;
            }
            break;

          case JSON_END_WHILE_ALPHA_NUM:
            if (s_is_alpha_num((char_t)ch) == 0)
            {
              pStream->pStream--;
              end_string = 1U;
            }
            break;

          default:
            /* Do Nothing MISRA */
            break;
        }
        if (end_string == 0U)
        {
          if (s_decode_char(pStream, ch, &pBuffer, &index) != JSON_OK)
          {
            end_string = 1U;
          }
        }
      }
    }
  }
  if (error == JSON_OK)
  {
    *pBuffer = '\0';
    *pNbChar = ((int32_t)index + 1L);
  }
  return error;
}


/**
* @brief return quoted  string
*
* @param pInstance the json instance
* @param pStream  the stream instance
* @return jsonID
*/
static jsonErr s_get_string_quoted(json_instance_t *pInstance, json_parse_stream_t *pStream, jsonID *pId)
{
  jsonErr error = JSON_OK;
  if (pId == NULL)
  {
    error = JSON_ERROR;
  }
  else if (s_read_stream(pStream) != '\"')
  {
    *pId = JSON_ID_NULL;
  }
  else
  {
    int32_t nbChar;
    error = s_read_stream_copy_until(pStream, JSON_END_QUOTE, &nbChar);
    if (error == JSON_OK)
    {
      error = s_create_and_get_string_id(pInstance, pStream->pTmpString, nbChar, pId);
    }
  }
  JSON_ERR_PRINT(error);
  return error;
}


/**
* @brief return true if it is an alpha numeric
*
* @param ch
* @return uint8_t TRUE or FALSE
*/
static int8_t s_is_alpha_num(char_t ch)
{
  int8_t ret;
  if (isdigit((int32_t)ch))/*cstat  !MISRAC2012-Dir-4.11_h the stdlib macro  isdigit produce this error, we cannot fix it */
  {
    ret = TRUE;
  }
  else if (isalpha((int32_t)ch))/*cstat  !MISRAC2012-Dir-4.11_h the stdlib macro  isdigit produce this error, we cannot fix it */
  {
    ret = TRUE;
  }
  else if (ch == '-')
  {
    ret = TRUE;
  }
  else if (ch == '+')
  {
    ret = TRUE;
  }
  else if (ch == '.')
  {
    ret = TRUE;
  }
  else
  {
    ret = FALSE;
  }
  return ret;
}


/**
* @brief returns tur if it is a number
*
* @param pString
* @return uint8_t TRUE or FALSE
*/
static int8_t s_is_number_string(char_t *pString)
{
  int8_t ret = TRUE;

  while (*pString)
  {
    int32_t bDigit =  isdigit((int) * pString);/*cstat  !MISRAC2012-Dir-4.11_h the stdlib macro  isdigit produce this error, we cannot fix it */
    if ((bDigit == 0) && (*pString != '-') && (*pString != '+') && (*pString != 'e') && (*pString != 'E') && (*pString != '.'))
    {
      ret = FALSE;
      break;
    }
    pString++;
  }

  return ret;
}


/**
* @brief returns true if it is a integer
*
* @param pString
* @return uint8_t TRUE or FALSE
*/
static int8_t s_is_int_string(char_t *pString)
{
  int8_t ret = TRUE;

  while (*pString)
  {
    if (isdigit((int32_t)*pString) == 0) /*cstat  !MISRAC2012-Dir-4.11_h the stdlib macro  isdigit produce this error, we cannot fix it */
    {
      if ((*pString != '-') && (*pString != '+'))
      {
        ret = FALSE;
        break;
      }
    }
    pString++;
  }

  return ret;
}


/**
* @brief Parse a json list
*
* @param pInstance the json instance
* @param pStream the stream instance
* @param root the root id
* @return jsonErr
*/
static jsonErr s_parse_list(json_instance_t *pInstance, json_parse_stream_t *pStream, jsonID root)
{
  uint8_t bRet = 0;
  jsonErr error = JSON_OK;
  while ((error == JSON_OK) && (bRet == 0U))
  {
    error = s_read_stream_eof(pStream);
    jsonID value = JSON_ID_NULL;
    jsonID key = JSON_ID_NULL;
    char_t ch = s_read_stream(pStream);
    /* check empty list */
    if ((ch == '}') || (ch == ']'))
    {
      s_read_rewind(pStream);
      bRet = 1;
    }
    else
    {
      s_read_rewind(pStream);
      error = s_parse_element(pInstance, pStream, &value);
      ch = s_read_stream(pStream);
      if (ch == ':')
      {
        key = value;
        error = s_parse_element(pInstance, pStream, &value);
        ch = s_read_stream(pStream);
      }
      if (json_list_add_pair(pInstance, root, key, value) != JSON_OK)
      {
        error = JSON_ERROR;
        bRet = 1;
      }
      else if (ch != ',')
      {
        s_read_rewind(pStream);
        bRet = 1;
      }
      else
      {
        /* do nothing: MISRAC */
      }
    }
  }
  JSON_ERR_PRINT(error);
  return error;
}


/**
* @brief finds an entry in a array
*
* @param pInstance the json instance
* @param root the json id
* @param pKeyName the key string
* @return int32_t
*/
static jsonErr s_find_list_entry(json_instance_t *pInstance, jsonID root, const char_t *pKeyName, jsonID *pId)
{
  jsonErr error = JSON_OK;

  if (pId == NULL)
  {
    error = JSON_ERROR;
  }
  if (error == JSON_OK)
  {
    *pId = JSON_ID_NULL;
    if (json_is_list(pInstance, root) == JSON_TRUE)
    {
      uint16_t count = 0;
      error = json_list_get_count(pInstance, root, &count);
      if (error == JSON_OK)
      {
        uint8_t  bRet  = 0;
        uint16_t a;

        for (a = 0; (a < count) && (bRet == 0U); a++)
        {
          jsonID key;
          error = json_list_pair(pInstance, root, a, &key, NULL);
          /* Key could be null in case of array with object typically  [{..}] */
          if (error == JSON_OK)
          {
            if (key != JSON_ID_NULL)
            {
              json_item_t *pItem = s_find_id(pInstance, key);
              const char_t *pName = JSON_PAYLOAD(const char_t *, pItem);
              if (strcmp(pName, pKeyName) == 0)
              {
                *pId = a;
                bRet = 1; /* exit loop with ok status */
              }
            }
          }
          else
          {
            bRet = 1; /* exit loop with nok status */
          }
        }
        if ((bRet == 0U) && (error == JSON_OK))
        {
          /* it means that id was not found */
          error = JSON_ERR_FOUND;
        }
      }
    }
  }
  return error;
}


/**
* @brief sets a json pair to an object
*
* @param pInstance the json instance
* @param root the json root id
* @param key the key json id
* @param value the value json id
* @return None
*/
static void s_add_key_val(json_instance_t *pInstance, jsonID root, jsonID key, jsonID value)
{
  json_item_t *pRoot = s_find_id(pInstance, root);
  JSON_ASSERT((pRoot != NULL) ? 1 : 0);
  json_list_t *pList = JSON_PAYLOAD(json_list_t *, pRoot);
  json_pair_t *pPair = JSON_PAYLOAD(json_pair_t *, pList);
  pPair[pList->cur].keyId = key;
  pPair[pList->cur].valueId = value;
  pList->cur++;
}



/**
* @brief insert  a json pair to an object
*
* @param pInstance the json instance
* @param root the json root id
* @param index the position
* @param key the key json id
* @param value the value json id
* @return None
*/
static void s_insert_key_val(json_instance_t *pInstance, jsonID root, int32_t index, jsonID key, jsonID value)
{
  json_item_t *pRoot = s_find_id(pInstance, root);
  JSON_ASSERT((pRoot != NULL) ? 1 : 0);
  json_list_t *pList = JSON_PAYLOAD(json_list_t *, pRoot);
  json_pair_t *pPair = JSON_PAYLOAD(json_pair_t *, pList);
  pList->cur++;
  int16_t len = ((int16_t)pList->cur - (int16_t)index) - 1;
  JSON_ASSERT((len >= 0) ? 1 : 0);

  if (len)
  {
    memmove(&pPair[index + 1], &pPair[index], (uint32_t)len * sizeof(json_pair_t));
  }
  pPair[index].keyId = key;
  pPair[index].valueId = value;
}




/**
* @brief return an id from a string
*
* @param pInstance the json instance
* @param str the string
* @param len  max len
* @param pId the id returned
* @return error
*/
static jsonErr s_get_string_id(json_instance_t *pInstance, const char_t *str, int32_t len, jsonID *pId)
{
  jsonErr error = JSON_OK;
  if (pId == NULL)
  {
    error = JSON_ERROR;
  }
  else
  {
    json_item_t *pjson = (json_item_t *)s_find_string(pInstance, str);
    if (pjson == NULL)
    {
      error = JSON_ERR_FOUND;
    }
    else
    {
      pjson->ref++;
      *pId = pjson->id;
    }
  }
  return error;
}


/**
* @brief Create a string
*
* @param pInstance the json instance
* @param str the string
* @param len  max len
* @param pId the id returned
* @return error
*/
static jsonErr s_create_and_get_string_id(json_instance_t *pInstance, const char_t *str, int32_t len, jsonID *pId)
{
  jsonErr error = s_get_string_id(pInstance, str, len, pId);
  if (error == JSON_ERR_FOUND)
  {
    json_item_t *pNew = s_create_string(pInstance, str, (uint32_t)len);
    if (pNew == NULL)
    {
      error = JSON_ERR_MEM;
    }
    else
    {
      *pId = pNew->id;
      error = JSON_OK;
    }
  }
  return error;
}


/**
* @brief set  a string
*
* @param pInstance the json instance
* @param root root id
* @param pTree location in the json ( ie "object.object2"
* @param pKey the key attribute
* @param pString the string
* @return error
*/
static jsonErr s_object_set_string(json_instance_t *pInstance, jsonID root, const char_t *pTree, const char_t *pKey, const char_t *pString, jsonID *pId)
{
  jsonErr error = JSON_OK;
  uint8_t bRet = 0;
  jsonID node = JSON_ID_NULL;
  error = json_object_set(pInstance, root, pTree, &node);
  if (error == JSON_OK)
  {
    jsonID index = JSON_ID_NULL;
    if (s_find_list_entry(pInstance, node, pKey, &index) == JSON_OK)
    {
      if (json_list_del_pair(pInstance, node, index) != JSON_OK)
      {
        bRet = 1U;
      }
    }
    if (bRet == 0U)
    {
      error = s_create_and_get_string_id(pInstance, pString, 0, pId);
      if (error == JSON_OK)
      {
        error = json_object_set_new(pInstance, node, pKey, *pId);
      }
    }
  }
  return error;
}


/**
* @brief create a numdata
*
* @param pInstance the json instance
* @param pNumData the numdata
* @param pId  the id
* @return error
*/
static jsonErr s_create_num_data(json_instance_t *pInstance, json_numData_t *pNumData, jsonID *pId)
{
  jsonErr       error = JSON_OK;
  jsonID        id    = JSON_ID_NULL;
  char_t        numData[30];
  json_sub_type subType = JSON_SUB_NULL;

  json_item_t  *pjson = NULL;

  if ((pNumData == NULL) || (pId == NULL))
  {
    error = JSON_ERROR;
  }
  if (error == JSON_OK)
  {
    switch (pNumData->dataType)
    {
      case JSON_SIGNED64:
        #ifdef JSON_LOCAL_ITOA_USED
        s_itoa(pNumData->val.s64, numData, sizeof(numData));
        #else
        snprintf(numData, sizeof(numData), "%lld", pNumData->val.s64);
        #endif
        subType = JSON_SUB_INT;
        break;
      case JSON_UNSIGNED64:
        #ifdef JSON_LOCAL_ITOA_USED
        s_uitoa(pNumData->val.u64, numData, sizeof(numData));
        #else
        snprintf(numData, sizeof(numData), "%llu", pNumData->val.u64);
        #endif
        subType = JSON_SUB_INT;
        break;
      case JSON_DOUBLE:
      {
        char *pChar1;
        snprintf(numData, sizeof(numData), "%f", pNumData->val.dbl);
        // remove useless trailing zeroes after decimal point
        pChar1 = strchr(numData, (int)'.');
        if (pChar1 != NULL)
        {
          for (char *pChar2 = pChar1 + 1; *pChar2 != '\0'; pChar2++)
          {
            if (*pChar2 != '0')
            {
              pChar1 = pChar2 + 1;
            }
          }
          *pChar1 = '\0';
        }
        subType = JSON_SUB_NUMBER;
        break;
      }
      default:
        numData[0] = '\0';
        subType = JSON_SUB_NULL;
        error = JSON_ERROR;
        break;
    }
  }
  if (error == JSON_OK)
  {
    pjson = (json_item_t *)s_find_string_subtype(pInstance, numData, subType);
    if (pjson)
    {
      pjson->ref++;
      id = pjson->id;
    }
    else
    {
      json_item_t *pNew = s_create_string(pInstance, numData, 0);
      if (pNew == NULL)
      {
        error = JSON_ERR_MEM;
      }
      else
      {
        pNew->type = JSON_MAKE_TYPE(JSON_TYPE_VALUE, subType);
        id = pNew->id;
      }
    }
    *pId = id;
  }
  JSON_ERR_PRINT(error);
  return error;
}


/**
* @brief create a type
*
* @param pInstance the json instance
* @param value the value
* @param subType  the subtype
* @param pId      the id

* @return error
*/
static jsonErr s_create_special_types(json_instance_t *pInstance, const char_t *value, json_sub_type subType, jsonID *pId)
{
  jsonErr      error = JSON_OK;
  jsonID       id    = JSON_ID_NULL;
  json_item_t *pjson = (json_item_t *)s_find_string_subtype(pInstance, value, subType);

  if (pjson)
  {
    pjson->ref++;
    id = pjson->id;
  }
  else
  {
    json_item_t *pNew = s_create_string(pInstance, value, 0);
    if (pNew == NULL)
    {
      error = JSON_ERR_MEM;
    }
    else
    {
      pNew->type = JSON_MAKE_TYPE(JSON_TYPE_VALUE, subType);
      id = pNew->id;
    }
  }
  if (error == JSON_OK)
  {
    if (pId == NULL)
    {
      error = JSON_ERROR;
    }
  }
  if (error == JSON_OK)
  {
    *pId = id;
  }
  JSON_ERR_PRINT(error);
  return error;
}


/**
* @brief get  a numdata
*
* @param pInstance the json instance
* @param root  the root id
* @param pTree location in the json ( ie "object.object2"
* @param pKey the key attribute
* @param pNumData the numdata
* @return error
*/
static jsonErr s_object_get_num_data(json_instance_t *pInstance, jsonID root, const char_t *pTree, const char_t *pKey, json_numData_t *pNumData)
{
  const char *pStr;
  jsonErr error = json_object_get_string(pInstance, root, pTree, pKey, &pStr);

  if (error == JSON_OK)
  {
    error = s_convert_str_to_num_data(pNumData, pStr);
  }
  return error;
}


/**
* @brief convert  a numdata
*
* @param pNumData the numdata
* @param pStr     the string
* @return error
*/
static jsonErr s_convert_str_to_num_data(json_numData_t *pNumData, const char *pStr)
{
  jsonErr error = JSON_OK;
  switch (pNumData->dataType)
  {
    case JSON_SIGNED64:
      pNumData->val.s64 = strtoll(pStr, NULL, 10);/*cstat !MISRAC2012-Rule-22.8 !MISRAC2012-Rule-22.9 errno not supported by this OS */
      break;
    case JSON_UNSIGNED64:
      pNumData->val.u64 = strtoull(pStr, NULL, 10);/*cstat !MISRAC2012-Rule-22.8 !MISRAC2012-Rule-22.9 errno not supported by this OS */
      break;
    case JSON_DOUBLE:
      pNumData->val.dbl = strtod(pStr, NULL);/*cstat !MISRAC2012-Rule-22.8 !MISRAC2012-Rule-22.9 errno not supported by this OS */
      break;
    default:
      error = JSON_ERROR;
      break;
  }
  return error;
}


/**
* @brief create a new id from type
*
* @param pInstance the json instance
* @param pId the  id
* @param type the  type
* @return error
*/
static jsonErr s_json_get_new_id(json_instance_t *pInstance, jsonID *pId, json_sub_type type)
{
  jsonErr error = JSON_OK;
  if (pId == NULL)
  {
    error = JSON_ERROR;
  }
  if (error == JSON_OK)
  {
    json_item_t *pRef = s_add_id(pInstance, (uint16_t)sizeof(json_list_t));
    if (pRef == NULL)
    {
      error = JSON_ERR_MEM;
    }
    else
    {
      pRef->type = JSON_MAKE_TYPE(JSON_TYPE_LIST, type);
      *pId = pRef->id;
    }
  }
  JSON_ERR_PRINT(error);
  return error;
}
