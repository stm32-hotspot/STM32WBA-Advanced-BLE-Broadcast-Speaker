/**
******************************************************************************
* @file        st_json_legacy.c
* @author      MCD Application Team
* @brief       wrapper to support the previous API
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
#include "st_json_conf.h"
#include "st_json.h"
#include "st_json_legacy.h"
#include "math.h"

/* Global variables ----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Functions Definition ------------------------------------------------------*/


/**
 * @brief reads a json stream and load registry
 *
 * @param pInstance the json instance
 * @param pJson the json string to load
 * @return jsonID or JSON_ID_NULL
 */
jsonID json_loads(json_instance_t *pInstance, const char_t *pJson)
{
  jsonID ret;
  if (pInstance == NULL)
  {
    ret = JSON_ID_NULL;
  }
  else
  {
    json_load(pInstance, pJson, &ret);
  }
  return ret;
}



/**
 * @brief creates a json string
 *
 * @param pInstance the json instance
 * @param value the string
 * @return jsonID or JSON_ID_NULL
 */
jsonID json_string(json_instance_t *pInstance, const char_t *value)
{
  jsonID ret = JSON_ID_NULL;
  json_create_string(pInstance, value, &ret);
  return ret;
}


/**
 * @brief creates a json integer
 *
 * @param pInstance the json instance
 * @param value the integer value
 * @return jsonID or JSON_ID_NULL
 */
jsonID json_integer(json_instance_t *pInstance, int64_t value)
{
  jsonID ret = JSON_ID_NULL;
  json_create_integer(pInstance, value, &ret);
  return ret;
}


/**
 * @brief returns a string from a jsonID
 *
 * @param pInstance the json instance
 * @param root the json id
 * @return const char_t*
 */
const char_t *json_string_value(json_instance_t *pInstance, jsonID root)
{
  const char_t *pRet = NULL;
  json_get_string_from_id(pInstance, root, &pRet);
  return pRet;
}

/**
 * @brief returns a integer from a jsonID
 *
 * @param pInstance the json instance
 * @param the json id
 * @return integer value or JSON_INTERGER_ERROR
 */
int64_t json_integer_value(json_instance_t *pInstance, jsonID root)
{
  int64_t ret = JSON_INTERGER_ERROR;
  if (json_get_integer_from_id(pInstance, root, &ret) != JSON_OK)
  {
    ret = JSON_INTERGER_ERROR;
  }
  return ret;
}

/**
 * @brief returns a unsigned integer from a jsonID
 *
 * @param pInstance the json instance
 * @param root the json id
 * @return unsigned integer value or JSON_INTERGER_ERROR
 */
uint64_t json_unsigned_integer_value(json_instance_t *pInstance, jsonID root)
{
  uint64_t ret = JSON_INTERGER_ERROR;
  if (json_get_unsigned_integer_from_id(pInstance, root, &ret) != JSON_OK)
  {
    ret = JSON_INTERGER_ERROR;
  }
  return ret;
}


/**
 * @brief returns a number from a jsonID
 *
 * @param pInstance the json instance
 * @param root the json id
 * @return a double or NAN
 */
double json_number_value(json_instance_t *pInstance, jsonID root)
{
  double ret = NAN;
  if (json_get_number_from_id(pInstance, root, &ret) != JSON_OK)
  {
    ret = NAN;
  }
  return ret;
}

/**
 * @brief creates a json unsigned integer
 *
 * @param pInstance the json instance
 * @param value the unsigned integer value
 * @return jsonID
 */
jsonID json_unsigned_integer(json_instance_t *pInstance, uint64_t value)
{
  jsonID id;
  json_create_unsigned_integer(pInstance, value, &id);
  return id ;
}


/**
 * @brief creates a json number as double
 *
 * @param pInstance the json instance
 * @param value  number value
 * @return jsonID
 */
jsonID json_number(json_instance_t *pInstance, double value)
{
  jsonID id;
  jsonErr error = json_create_number(pInstance, value, &id);
  return (error == JSON_OK) ? id : error;
}


/**
 * @brief create a json boolean true
 *
 * @param pInstance the json instance
 * @return jsonID
 */
jsonID json_true(json_instance_t *pInstance)
{
  jsonID id;
  jsonErr error = json_create_true(pInstance, &id);
  return (error == JSON_OK) ? id : error;
}



/**
 * @brief creates a json boolean false
 *
 * @param pInstance the json instance
 * @return jsonID
 */
jsonID json_false(json_instance_t *pInstance)
{
  jsonID id;
  jsonErr error = json_create_false(pInstance, &id);
  return (error == JSON_OK) ? id : error;
}



/**
 * @brief creates a json boolean
 *
 * @param pInstance the json instance
 * @param state the boolean value
 * @return jsonID
 */
jsonID json_boolean(json_instance_t *pInstance, int8_t state)
{
  jsonID id;
  jsonErr error = json_create_boolean(pInstance, state, &id);
  return (error == JSON_OK) ? id : error;
}



/**
 * @brief create a json null
 *
 * @param pInstance the json instance
 * @return jsonID
 */
jsonID json_null(json_instance_t *pInstance)
{
  jsonID id;
  jsonErr error = json_create_null(pInstance, &id);
  return (error == JSON_OK) ? id : error;
}


/**
 * @brief creates an Json object
 *
 * @param pInstance  the json instance
 * @return jsonID
 */
jsonID json_object(json_instance_t *pInstance)
{
  jsonID ret;
  json_create_object(pInstance, &ret);
  return ret;
}


/**
 * @brief  creates an Json array
 *
 * @param pInstance the json instance
 * @return jsonID
 */
jsonID json_array(json_instance_t *pInstance)
{
  jsonID ret;
  json_create_array(pInstance, &ret);
  return ret;
}





/**
 * @brief returns the object id from a name
 *
 * @param pInstance the json instance
 * @param root the root id
 * @param pTree  the json path speared by a dot
 * @return jsonID
 */
jsonID json_get_object(json_instance_t *pInstance, jsonID root, const char_t *pTree)
{
  jsonID ret = JSON_ID_NULL;
  json_object_get_id_from_tree(pInstance, root, pTree, &ret);
  return ret;
}

/**
 * @brief returns an jsonID from key name
 *
 * @param pInstance the json instance
 * @param root the json id
 * @param pName  the key name
 * @return jsonID
 */
jsonID json_object_get(json_instance_t *pInstance, jsonID root, const char_t *pName)
{
  jsonID ret;
  json_object_get_id_from_name(pInstance, root, pName, &ret);
  return ret;
}

/**
 * @brief sets an json id in an object
 *
 * @param pInstance the json instance
 * @param root the json id
 * @param pTree  the json path speared by a dot
 * @return jsonID
 */
jsonID json_set_object(json_instance_t *pInstance, jsonID root, const char_t *pTree)
{
  jsonID  ret  = root;
  json_object_set(pInstance, root, pTree, &ret);
  return ret;
}


/**
 * @brief gets a object string
 *
 * @param pInstance the json instance
 * @param root the json id
 * @param pTree the json path speared by a dot
 * @param pKey the json key
 * @return const char*
 */
const char *json_get_object_string(json_instance_t *pInstance, jsonID root, const char_t *pTree, const char_t *pKey)
{
  const char_t *pStr;
  json_object_get_string(pInstance, root, pTree, pKey, &pStr);
  return pStr;
}



/**
 * @brief returns the object string size
 *
 * @param pInstance the json instance
 * @param root the json id
 * @param pTree the json path speared by a dot
 * @param pKey the key string
 * @return int32_t
 */
int32_t json_get_object_string_size(json_instance_t *pInstance, jsonID root, const char_t *pTree, const char_t *pKey)
{
  int32_t ret;
  json_object_get_string_size(pInstance, root, pTree, pKey, &ret);
  return ret;
}


/**
 * @brief returns an integer value
 *
 * @param pInstance the json instance
 * @param root the json id
 * @param pTree the json path speared by a dot
 * @param pKey the key string
 * @return int64_t
 */
int64_t json_get_object_integer(json_instance_t *pInstance, jsonID root, const char_t *pTree, const char_t *pKey)
{
  int64_t ret;
  jsonErr error = json_object_get_integer(pInstance, root, pTree, pKey, &ret);
  if (error != JSON_OK)
  {
    ret = -1;
  }
  return ret;
}


/**
 * @brief returns an unsigned integer value
 *
 * @param pInstance the json instance
 * @param root the json id
 * @param pTree the json path speared by a dot
 * @param pKey the key string
 * @return uint64_t
 */
uint64_t json_get_object_unsigned_integer(json_instance_t *pInstance, jsonID root, const char_t *pTree, const char_t *pKey)
{
  uint64_t ret;
  jsonErr  error = json_object_get_unsigned_integer(pInstance, root, pTree, pKey, &ret);
  if (error != JSON_OK)
  {
    ret = -1;
  }
  return ret;
}


/**
 * @brief returns a number value
 *
 * @param pInstance the json instance
 * @param root the json id
 * @param pTree the json path speared by a dot
 * @param pKey the key string
 * @return double
 */
double json_get_object_number(json_instance_t *pInstance, jsonID root, const char_t *pTree, const char_t *pKey)
{
  double  ret;
  jsonErr error = json_object_get_number(pInstance, root, pTree, pKey, &ret);
  if (error != JSON_OK)
  {
    ret = -1.0;
  }
  return ret;
}


/**
 * @brief returns a bool value
 *
 * @param pInstance the json instance
 * @param root the json id
 * @param pTree the json path speared by a dot
 * @param pKey the key string
 * @return true or false
 */
uint8_t json_get_object_boolean(json_instance_t *pInstance, jsonID root, const char_t *pTree, const char_t *pKey)
{
  uint8_t ret;
  if (json_object_get_boolean(pInstance, root, pTree, pKey, &ret) != JSON_OK)
  {
    ret = -1;
  }
  return ret;
}


/**
 * @brief sets an integer in an object
 *
 * @param pInstance the json instance
 * @param root the json id
 * @param pTree the json path speared by a dot
 * @param pKey the key string
 * @param integer the integer
 * @return jsonID
 */
jsonID json_set_object_integer(json_instance_t *pInstance, jsonID root, const char_t *pTree, const char_t *pKey, int64_t integer)
{
  jsonID ret = root;
  if (json_object_set_integer(pInstance, root, pTree, pKey, integer) != JSON_OK)
  {
    ret = JSON_ID_NULL;
  }
  return ret;
}



/**
 * @brief sets an unsigned integer in an object
 *
 * @param pInstance the json instance
 * @param root the json id
 * @param pTree the json path speared by a dot
 * @param pKey the key string
 * @param integer the unsigned integer
 * @return jsonID
 */
jsonID json_set_object_unsigned_integer(json_instance_t *pInstance, jsonID root, const char_t *pTree, const char_t *pKey, uint64_t integer)
{
  jsonID ret = root;
  if (json_object_set_unsigned_integer(pInstance, root, pTree, pKey, integer) != JSON_OK)
  {
    ret = JSON_ID_NULL;
  }
  return ret;

}



/**
 * @brief sets an integer in an object
 *
 * @param pInstance the json instance
 * @param root the json id
 * @param pTree the json path speared by a dot
 * @param pKey the key string
 * @param number the number
 * @return jsonID
 */
jsonID json_set_object_number(json_instance_t *pInstance, jsonID root, const char_t *pTree, const char_t *pKey, double number)
{
  jsonID ret = root;
  if (json_object_set_number(pInstance, root, pTree, pKey, number) != JSON_OK)
  {
    ret = JSON_ID_NULL;
  }
  return ret;

}



/**
 * @brief sets an bool in an object
 *
 * @param pInstance the json instance
 * @param root the json id
 * @param pTree the json path speared by a dot
 * @param pKey the key string
 * @param ret the value
 * @return jsonID
 */
jsonID json_set_object_boolean(json_instance_t *pInstance, jsonID root, const char_t *pTree, const char_t *pKey, uint8_t state)
{
  jsonID ret = root;
  if (json_object_set_boolean(pInstance, root, pTree, pKey, state) != JSON_OK)
  {
    ret = JSON_ID_NULL;
  }
  return ret;
}

/**
 * @brief return the item count from a list object
 *
 * @param pInstance the json instance
 * @param root the json id
 * @return the object count
 */

jsonErr json_list_count(json_instance_t *pInstance, jsonID root)
{
  uint16_t ret;
  if (json_list_get_count(pInstance, root, &ret) != JSON_OK)
  {
    ret = 0;
  }
  return ret;
}


/**
 * @brief sets an object string
 *
 * @param pInstance the json instance
 * @param root the json id
 * @param pTree the json path speared by a dot
 * @param pKey the key string
 * @param pString  the string
 * @return jsonID
 */
jsonID json_set_object_string(json_instance_t *pInstance, jsonID root, const char_t *pTree, const char_t *pKey, const char_t *pString)
{
  jsonID ret = root;
  if (json_object_set_string(pInstance, root, pTree, pKey, pString) != JSON_OK)
  {
    ret = JSON_ID_NULL;
  }
  return ret;
}

jsonErr json_check_list_key(json_instance_t *pInstance, jsonID root, const char_t *pTree, const char_t *pKey)
{
  return json_object_check_list_key(pInstance, root, pTree, pKey);
}



/**
 * @brief gets an array
 *
 * @param pInstance the json instance
 * @param root the json id
 * @param pTree the json path speared by a dot
 * @return jsonID
 */
jsonID json_get_array(json_instance_t *pInstance, jsonID root, const char_t *pTree)
{
  jsonID ret = JSON_ID_NULL;
  json_array_get(pInstance, root, pTree, &ret);
  return ret;
}

/**
 * @brief set a json array
 *
 * @param pInstance the json instance
 * @param root the json id
 * @param pTree the json path speared by a dot
 * @return jsonID
 */
jsonID json_set_array(json_instance_t *pInstance, jsonID root, const char_t *pTree)
{
  jsonID ret = JSON_ID_NULL;
  json_array_set(pInstance, root, pTree, &ret);
  return ret;
}


/**
 * @brief free a json pointer
 *
 * @param ptr  json ptr
 */
void  jsonp_free(void *ptr)
{
  json_free(ptr);
}


