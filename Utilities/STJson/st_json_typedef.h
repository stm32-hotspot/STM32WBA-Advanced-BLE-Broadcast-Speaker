/**
  ******************************************************************************
  * @file    st_json_typedef.h
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
#ifndef __ST_JSON_TYPEDEF_
#define __ST_JSON_TYPEDEF_

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/**
* @brief sub type
*
*/
typedef enum
{
  JSON_SUB_STRING,
  JSON_SUB_INT,
  JSON_SUB_OBJ,
  JSON_SUB_ARRAY,
  JSON_SUB_TRUE,
  JSON_SUB_FALSE,
  JSON_SUB_NULL,
  JSON_SUB_NUMBER,
} json_sub_type;

typedef enum
{
  JSON_SIGNED64,
  JSON_UNSIGNED64,
  JSON_DOUBLE
} json_param_type_t;


/**
* @brief Tmp struct hold the growing buffer for the result
*
*/
#ifndef HAVE_CHAR_T
typedef char char_t;
#endif

typedef struct t_json_dump
{
  uint32_t szDump;
  uint32_t curDump;
  char_t  *pDump;
} json_dump_t;

typedef struct
{
  json_param_type_t dataType;
  union
  {
    int64_t  s64;
    uint64_t u64;
    double   dbl;
  } val, min, max;
} json_numData_t;



typedef uint16_t jsonID;
typedef uint16_t jsonErr;

/* allocator prototype if overloaded */
typedef void *(*json_realloc_t)(void *ptr, size_t size);
typedef void (*json_free_t)(void *ptr);

/* Hold the parsing string*/
typedef struct t_json_parse_stream
{
  const char_t *pStreamBuffer;
  const char_t *pStream;
  uint32_t      szStream;
  uint32_t      curStream;
  char_t       *pTmpString;
  uint16_t      szTmpString;
} json_parse_stream_t;

/* Json instance, hold the item registry*/

typedef struct json_instance_t
{
  char_t             *pRegistry;
  uint32_t            szRegistry;
  uint32_t            curRegistry;
  uint16_t            gItemID;
  uint8_t             jsonStandard;
  jsonID              pack_root;
  json_parse_stream_t stream;

} json_instance_t;

typedef json_instance_t json_pack_t;

/* Exported constants --------------------------------------------------------*/
#define JSON_TRUE  1
#define JSON_FALSE 0

#define JSON_INDENT    0x80U
#define JSON_OK        0U
#define JSON_ERROR     1U
#define JSON_ERR_FOUND 2U
#define JSON_ERR_TYPE  3U
#define JSON_ERR_MEM   4U
#define JSON_ERR_MAX   5U
#define JSON_ID_NULL   0U
/* Exported macros -----------------------------------------------------------*/

#define JSON_CREATE_OBJECT(jsError, pInstance, pId)                                                                                                                                     \
    do                                                                                                                                                                                  \
    {                                                                                                                                                                                   \
        if ((jsError) == JSON_OK)                                                                                                                                                       \
        {                                                                                                                                                                               \
            (jsError) = json_create_object((pInstance), (pId));                                                                                                                         \
            if ((jsError) != JSON_OK)                                                                                                                                                   \
            {                                                                                                                                                                           \
                JSON_PRINT("%s, %s(), line %d: error %d - json_create_object\n", __FILE__, __FUNCTION__, __LINE__, (jsError));                                                          \
            }                                                                                                                                                                           \
        }                                                                                                                                                                               \
    } while (0)


#define JSON_CREATE_ARRAY(jsError, pInstance, pId)                                                                                                                                      \
    do                                                                                                                                                                                  \
    {                                                                                                                                                                                   \
        if ((jsError) == JSON_OK)                                                                                                                                                       \
        {                                                                                                                                                                               \
            (jsError) = json_create_array((pInstance), (pId));                                                                                                                          \
            if ((jsError) != JSON_OK)                                                                                                                                                   \
            {                                                                                                                                                                           \
                JSON_PRINT("%s, %s(), line %d: error %d - json_create_array\n", __FILE__, __FUNCTION__, __LINE__, (jsError));                                                           \
            }                                                                                                                                                                           \
        }                                                                                                                                                                               \
    } while (0)

#define JSON_OBJECT_SET_NEW(jsError, pInstance, root, pName, value)                                                                                                                     \
    do                                                                                                                                                                                  \
    {                                                                                                                                                                                   \
        if ((jsError) == JSON_OK)                                                                                                                                                       \
        {                                                                                                                                                                               \
            (jsError) = json_object_set_new((pInstance), (root), (pName), (value));                                                                                                     \
            if ((jsError) != JSON_OK)                                                                                                                                                   \
            {                                                                                                                                                                           \
                JSON_PRINT("%s, %s(), line %d: error %d - json_object_set_new %d \"%s\" %d\n", __FILE__, __FUNCTION__, __LINE__, (jsError), (root), (pName), (value));                  \
            }                                                                                                                                                                           \
        }                                                                                                                                                                               \
    } while (0)

#define JSON_ARRAY_APPEND_NEW(jsError, pInstance, root, value)                                                                                                                          \
    do                                                                                                                                                                                  \
    {                                                                                                                                                                                   \
        if ((jsError) == JSON_OK)                                                                                                                                                       \
        {                                                                                                                                                                               \
            (jsError) = json_array_append_new((pInstance), (root), (value));                                                                                                            \
            if ((jsError) != JSON_OK)                                                                                                                                                   \
            {                                                                                                                                                                           \
                JSON_PRINT("%s, %s(), line %d: error %d - json_array_append_new %d %d\n", __FILE__, __FUNCTION__, __LINE__, (jsError), (root), (value));                                \
            }                                                                                                                                                                           \
        }                                                                                                                                                                               \
    } while (0)

#define JSON_ARRAY_GET(jsError, acceptableError, pInstance, root, pTree, pId)                                                                                                           \
    do                                                                                                                                                                                  \
    {                                                                                                                                                                                   \
        if ((jsError) == JSON_OK)                                                                                                                                                       \
        {                                                                                                                                                                               \
            (jsError) = json_array_get((pInstance), (root), (pTree), (pId));                                                                                                            \
            if ((jsError) != JSON_OK)                                                                                                                                                   \
            {                                                                                                                                                                           \
                if ((jsError) != (acceptableError))                                                                                                                                     \
                {                                                                                                                                                                       \
                    JSON_PRINT("%s, %s(), line %d: error %d - json_array_get %d \"%s\"\n", __FILE__, __FUNCTION__, __LINE__, (jsError), (root), (pTree));                               \
                }                                                                                                                                                                       \
            }                                                                                                                                                                           \
        }                                                                                                                                                                               \
    } while (0)

#define JSON_LIST_GET_COUNT(jsError, pInstance, root, pCount)                                                                                                                           \
    do                                                                                                                                                                                  \
    {                                                                                                                                                                                   \
        if ((jsError) == JSON_OK)                                                                                                                                                       \
        {                                                                                                                                                                               \
            (jsError) = json_list_get_count((pInstance), (root), (pCount));                                                                                                             \
            if ((jsError) != JSON_OK)                                                                                                                                                   \
            {                                                                                                                                                                           \
                JSON_PRINT("%s, %s(), line %d: error %d - json_list_get_count %d\n", __FILE__, __FUNCTION__, __LINE__, (jsError), (root));                                              \
            }                                                                                                                                                                           \
        }                                                                                                                                                                               \
    } while (0)

#define JSON_CREATE_STRING(jsError, pInstance, pStr, pId)                                                                                                                               \
    do                                                                                                                                                                                  \
    {                                                                                                                                                                                   \
        if ((jsError) == JSON_OK)                                                                                                                                                       \
        {                                                                                                                                                                               \
            (jsError) = json_create_string((pInstance), (pStr), (pId));                                                                                                                 \
            if ((jsError) != JSON_OK)                                                                                                                                                   \
            {                                                                                                                                                                           \
                JSON_PRINT("%s, %s(), line %d: error %d - json_create_string \"%s\"\n", __FILE__, __FUNCTION__, __LINE__, (jsError), (pStr));                                           \
            }                                                                                                                                                                           \
        }                                                                                                                                                                               \
    } while (0)

#define JSON_OBJECT_SET_STRING(jsError, pInstance, root, pTree, pKey, pString)                                                                                                          \
    do                                                                                                                                                                                  \
    {                                                                                                                                                                                   \
        if ((jsError) == JSON_OK)                                                                                                                                                       \
        {                                                                                                                                                                               \
            (jsError) = json_object_set_string((pInstance), (root), (pTree), (pKey), (pString));                                                                                        \
            if ((jsError) != JSON_OK)                                                                                                                                                   \
            {                                                                                                                                                                           \
                JSON_PRINT("%s, %s(), line %d: error %d - json_object_set_string \"%s\" \"%s\" \"%s\"\n", __FILE__, __FUNCTION__, __LINE__, (jsError), (pTree), (pKey), (pString));     \
            }                                                                                                                                                                           \
        }                                                                                                                                                                               \
    } while (0)


#define JSON_OBJECT_GET_STRING(jsError, pInstance, root, pTree, pKey, ppStr)                                                                                                            \
    do                                                                                                                                                                                  \
    {                                                                                                                                                                                   \
        if ((jsError) == JSON_OK)                                                                                                                                                       \
        {                                                                                                                                                                               \
            (jsError) = json_object_get_string((pInstance), (root), (pTree), (pKey), (ppStr));                                                                                          \
            if ((jsError) != JSON_OK)                                                                                                                                                   \
            {                                                                                                                                                                           \
                JSON_PRINT("%s, %s(), line %d: error %d - json_object_get_string \"%s\" \"%s\"\n", __FILE__, __FUNCTION__, __LINE__, (jsError), (pTree), (pKey));                       \
            }                                                                                                                                                                           \
        }                                                                                                                                                                               \
    } while (0)

#define JSON_OBJECT_SET_NUM_DATA(jsError, pInstance, root, pTree, pKey, pNumData)                                                                                                       \
    do                                                                                                                                                                                  \
    {                                                                                                                                                                                   \
        if ((jsError) == JSON_OK)                                                                                                                                                       \
        {                                                                                                                                                                               \
            (jsError) = json_object_set_num_data((pInstance), (root), (pTree), (pKey), (pNumData));                                                                                     \
            if ((jsError) != JSON_OK)                                                                                                                                                   \
            {                                                                                                                                                                           \
                JSON_PRINT("%s, %s(), line %d: error %d - json_object_set_num_data \"%s\" \"%s\"\n", __FILE__, __FUNCTION__, __LINE__, (jsError), (pTree), (pKey));                     \
            }                                                                                                                                                                           \
        }                                                                                                                                                                               \
    } while (0)

#define JSON_OBJECT_SET_INTEGER(jsError, pInstance, root, pTree, pKey, s64)                                                                                                             \
    do                                                                                                                                                                                  \
    {                                                                                                                                                                                   \
        if ((jsError) == JSON_OK)                                                                                                                                                       \
        {                                                                                                                                                                               \
            (jsError) = json_object_set_integer((pInstance), (root), (pTree), (pKey), (s64));                                                                                           \
            if ((jsError) != JSON_OK)                                                                                                                                                   \
            {                                                                                                                                                                           \
                JSON_PRINT("%s, %s(), line %d: error %d - json_object_set_integer \"%s\" \"%s\" %lld\n", __FILE__, __FUNCTION__, __LINE__, (jsError), (pTree), (pKey), (s64));          \
            }                                                                                                                                                                           \
        }                                                                                                                                                                               \
    } while (0)

#define JSON_OBJECT_GET_INTEGER(jsError, pInstance, root, pTree, pKey, pVal)                                                                                                            \
    do                                                                                                                                                                                  \
    {                                                                                                                                                                                   \
        if ((jsError) == JSON_OK)                                                                                                                                                       \
        {                                                                                                                                                                               \
            (jsError) = json_object_get_integer((pInstance), (root), (pTree), (pKey), (pVal));                                                                                          \
            if ((jsError) != JSON_OK)                                                                                                                                                   \
            {                                                                                                                                                                           \
                JSON_PRINT("%s, %s(), line %d: error %d - json_object_get_integer \"%s\" \"%s\"\n", __FILE__, __FUNCTION__, __LINE__, (jsError), (pTree), (pKey));                      \
            }                                                                                                                                                                           \
        }                                                                                                                                                                               \
    } while (0)

#define JSON_OBJECT_SET_UNSIGNED_INTEGER(jsError, pInstance, root, pTree, pKey, u64)                                                                                                    \
    do                                                                                                                                                                                  \
    {                                                                                                                                                                                   \
        if ((jsError) == JSON_OK)                                                                                                                                                       \
        {                                                                                                                                                                               \
            (jsError) = json_object_set_unsigned_integer((pInstance), (root), (pTree), (pKey), (u64));                                                                                  \
            if ((jsError) != JSON_OK)                                                                                                                                                   \
            {                                                                                                                                                                           \
                JSON_PRINT("%s, %s(), line %d: error %d - json_object_set_unsigned_integer \"%s\" \"%s\" %lud\n", __FILE__, __FUNCTION__, __LINE__, (jsError), (pTree), (pKey), (u64)); \
            }                                                                                                                                                                           \
        }                                                                                                                                                                               \
    } while (0)

#define JSON_OBJECT_GET_UNSIGNED_INTEGER(jsError, pInstance, root, pTree, pKey, pVal)                                                                                                   \
    do                                                                                                                                                                                  \
    {                                                                                                                                                                                   \
        if ((jsError) == JSON_OK)                                                                                                                                                       \
        {                                                                                                                                                                               \
            (jsError) = json_object_get_unsigned_integer((pInstance), (root), (pTree), (pKey), (pVal));                                                                                 \
            if ((jsError) != JSON_OK)                                                                                                                                                   \
            {                                                                                                                                                                           \
                JSON_PRINT("%s, %s(), line %d: error %d - json_object_get_unsigned_integer \"%s\" \"%s\"\n", __FILE__, __FUNCTION__, __LINE__, (jsError), (pTree), (pKey));             \
            }                                                                                                                                                                           \
        }                                                                                                                                                                               \
    } while (0)

#define JSON_OBJECT_SET_NUMBER(jsError, pInstance, root, pTree, pKey, dbl)                                                                                                              \
    do                                                                                                                                                                                  \
    {                                                                                                                                                                                   \
        if ((jsError) == JSON_OK)                                                                                                                                                       \
        {                                                                                                                                                                               \
            (jsError) = json_object_set_number((pInstance), (root), (pTree), (pKey), (number));                                                                                         \
            if ((jsError) != JSON_OK)                                                                                                                                                   \
            {                                                                                                                                                                           \
                JSON_PRINT("%s, %s(), line %d: error %d - json_object_set_number \"%s\" \"%s\" %lf\n", __FILE__, __FUNCTION__, __LINE__, (jsError), (pTree), (pKey), (number));         \
            }                                                                                                                                                                           \
        }                                                                                                                                                                               \
    } while (0)

#define JSON_OBJECT_GET_NUMBER(jsError, pInstance, root, pTree, pKey, pVal)                                                                                                             \
    do                                                                                                                                                                                  \
    {                                                                                                                                                                                   \
        if ((jsError) == JSON_OK)                                                                                                                                                       \
        {                                                                                                                                                                               \
            (jsError) = json_object_get_number((pInstance), (root), (pTree), (pKey), (pVal));                                                                                           \
            if ((jsError) != JSON_OK)                                                                                                                                                   \
            {                                                                                                                                                                           \
                JSON_PRINT("%s, %s(), line %d: error %d - json_object_get_number \"%s\" \"%s\"\n", __FILE__, __FUNCTION__, __LINE__, (jsError), (pTree), (pKey));                       \
            }                                                                                                                                                                           \
        }                                                                                                                                                                               \
    } while (0)

#define JSON_OBJECT_SET_BOOLEAN(jsError, pInstance, root, pTree, pKey, state)                                                                                                           \
    do                                                                                                                                                                                  \
    {                                                                                                                                                                                   \
        if ((jsError) == JSON_OK)                                                                                                                                                       \
        {                                                                                                                                                                               \
            (jsError) = json_object_set_boolean((pInstance), (root), (pTree), (pKey), (state));                                                                                         \
            if ((jsError) != JSON_OK)                                                                                                                                                   \
            {                                                                                                                                                                           \
                JSON_PRINT("%s, %s(), line %d: error %d - json_object_set_boolean \"%s\" \"%s\" %d\n", __FILE__, __FUNCTION__, __LINE__, (jsError), (pTree), (pKey), (state));          \
            }                                                                                                                                                                           \
        }                                                                                                                                                                               \
    } while (0)

#define JSON_OBJECT_GET_BOOLEAN(jsError, pInstance, root, pTree, pKey, pBool)                                                                                                           \
    do                                                                                                                                                                                  \
    {                                                                                                                                                                                   \
        if ((jsError) == JSON_OK)                                                                                                                                                       \
        {                                                                                                                                                                               \
            (jsError) = json_object_get_boolean((pInstance), (root), (pTree), (pKey), (pBool));                                                                                         \
            if ((jsError) != JSON_OK)                                                                                                                                                   \
            {                                                                                                                                                                           \
                JSON_PRINT("%s, %s(), line %d: error %d - json_object_get_boolean \"%s\" \"%s\"\n", __FILE__, __FUNCTION__, __LINE__, (jsError), (pTree), (pKey));                      \
            }                                                                                                                                                                           \
        }                                                                                                                                                                               \
    } while (0)

#define JSON_LIST_PAIR(jsError, pInstance, root, index, pKey, pValue)                                                                                                                   \
    do                                                                                                                                                                                  \
    {                                                                                                                                                                                   \
        if ((jsError) == JSON_OK)                                                                                                                                                       \
        {                                                                                                                                                                               \
            (jsError) = json_list_pair((pInstance), (root), (index), (pKey), (pValue));                                                                                                 \
            if ((jsError) != JSON_OK)                                                                                                                                                   \
            {                                                                                                                                                                           \
                JSON_PRINT("%s, %s(), line %d: error %d - json_list_pair %d\n", __FILE__, __FUNCTION__, __LINE__, (jsError), (index));                                                  \
            }                                                                                                                                                                           \
        }                                                                                                                                                                               \
    } while (0)

#define JSON_LOAD(jsError, pInstance, pJson, pId)                                                                                                                                       \
    do                                                                                                                                                                                  \
    {                                                                                                                                                                                   \
        if ((jsError) == JSON_OK)                                                                                                                                                       \
        {                                                                                                                                                                               \
            (jsError) = json_load((pInstance), (pJson), (pId));                                                                                                                         \
            if ((jsError) != JSON_OK)                                                                                                                                                   \
            {                                                                                                                                                                           \
                JSON_PRINT("%s, %s(), line %d: error %d - json_load\n", __FILE__, __FUNCTION__, __LINE__, (jsError));                                                                   \
            }                                                                                                                                                                           \
        }                                                                                                                                                                               \
    } while (0)

/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */


#ifdef __cplusplus
};
#endif

#endif /* __ST_JSON_TYPEDEF_ */
