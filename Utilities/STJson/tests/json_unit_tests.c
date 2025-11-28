/**
  ******************************************************************************
  * @file        json_unit_test.c
  * @author      MCD Application Team
  * @brief       json unit
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "st_json.h"
#include "st_json_legacy.h"
#include "st_json_conf.h"


/* Global variables ----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
#define JSON_TRACE_INFO(...) JSON_PRINT(__VA_ARGS__);JSON_PRINT("\r\n")

/* observer macros */

#define CHECK_JSON_ID(id)               if((id) == JSON_ID_NULL) {bFlagSucess=false;}
#define CHECK_JSON_ERROR(err)           if((err) != JSON_OK) {bFlagSucess=false;}
#define CHECK_JSON_RETURN(fn,ret)           if((fn) != (ret)) {bFlagSucess=false;}
#define CHECK_JSON_STRING(sRef,sRead)   if(strcmp((char *)(sRef),(char *)(sRead)) != 0 ) {bFlagSucess=false;}
#define CHECK_JSON_INTEGER(iRef,iRead)  if((iRef) != (iRead) ) {bFlagSucess=false;}
#define CHECK_JSON_NUMBER(iRef,iRead)   if((iRef) != (iRead) ) {bFlagSucess=false;}
#define CHECK_JSON_BOOLEAN(iRef,iRead)  if((iRef) != (iRead) ) {bFlagSucess=false;}

/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static json_instance_t    hInstance;
static uint32_t                 bFlagSucess;
static jsonID                   hRoot;
static const char               tRefJson[] = "{\"string\":\"ok\",\"int\":10,\"number\":10.100000,\"bool\":true,\"object1\":{\"string\":\"ok\",\"object2\":{\"string\":\"ok\"}},\"array\":[\"value1\",\"value2\",{\"string\":\"ok\"}]}";

/* Private function prototypes -----------------------------------------------*/








static void  json_unit_print_result(char_t *pTitle)
{
  JSON_PRINT("%-10s : %-050s: %s\r\n", "st_json", pTitle, bFlagSucess ? "PASS" : "FAILS");
}

static void json_unit_test_reset(uint8_t hard)
{
  bFlagSucess = true;
  if (hard)
  {
    json_shutdown(&hInstance);
    memset(&hInstance, 0, sizeof(hInstance));
  }
}


void json_unit_test(void)
{
  const char *pString;
  int64_t     iInterger;
  double      iNumber;
  uint8_t     iBoolean;
  jsonID      hId1;
  jsonID      hId2;
  jsonID      hId3;
  jsonID      hId4;
  jsonID      hStr1;
  jsonID      hStr2;
  jsonID      hStr3;
  uint16_t    iCount;
  jsonID      hKey;
  jsonID      hValue;


  JSON_TRACE_INFO("************ JSON  unit test ************");

  /*
      TODO: this test checks only the basic API, it must be improved

      We will check and use only high level API
      Consequently will check also low level API because high level API uses low level
      This will simplify the test.
  */

  /*  Create the main object {}  */

  json_unit_test_reset(true);
  hRoot = json_object(&hInstance);
  CHECK_JSON_ID(hRoot);
  json_unit_print_result("Create object container");

  /* Add a string , readback and check*/
  json_unit_test_reset(false);
  CHECK_JSON_ERROR(json_object_set_string(&hInstance, hRoot, "", "string", "ok"));
  CHECK_JSON_ERROR(json_object_get_string(&hInstance, hRoot, "", "string", &pString));
  CHECK_JSON_STRING(pString, "ok");
  json_unit_print_result("Object string r/w");

  /* Add an integer , readback and check*/

  json_unit_test_reset(false);
  CHECK_JSON_ERROR(json_object_set_integer(&hInstance, hRoot, "", "int", 10));
  CHECK_JSON_ERROR(json_object_get_integer(&hInstance, hRoot, "", "int", &iInterger));
  CHECK_JSON_INTEGER(iInterger, 10);
  json_unit_print_result("Object integer r/w");

  /* Add a number, readback and check*/


  json_unit_test_reset(false);
  CHECK_JSON_ERROR(json_object_set_number(&hInstance, hRoot, "", "number", 10.1));
  CHECK_JSON_ERROR(json_object_get_number(&hInstance, hRoot, "", "number", &iNumber));
  CHECK_JSON_NUMBER(iNumber, 10.1);
  json_unit_print_result("Object number r/w");


  /* Add a boolean, readback and check*/

  json_unit_test_reset(false);
  CHECK_JSON_ERROR(json_object_set_boolean(&hInstance, hRoot, "", "bool", 1));
  CHECK_JSON_ERROR(json_object_get_boolean(&hInstance, hRoot, "", "bool", &iBoolean));
  CHECK_JSON_BOOLEAN(iBoolean, 1);
  json_unit_print_result("Object boolean r/w");


  /* Check a sub object like {{"string":"ok"} readback and check*/

  json_unit_test_reset(false);
  CHECK_JSON_ERROR(json_get_new_id(&hInstance, &hId1, JSON_SUB_OBJ));
  CHECK_JSON_RETURN(json_is_object(&hInstance, hId1), JSON_TRUE);
  CHECK_JSON_ERROR(json_object_set_new(&hInstance, hRoot, "object1", hId1));
  CHECK_JSON_ERROR(json_object_set_string(&hInstance, hId1, "", "string", "ok"));
  CHECK_JSON_ERROR(json_object_get_string(&hInstance, hRoot, "object1", "string", &pString));
  CHECK_JSON_STRING(pString, "ok");
  json_unit_print_result("Simple sub object r/w");


  /* Check sub object nexted like {{"object":{"string":"ok"}} readback and check*/

  json_unit_test_reset(false);
  CHECK_JSON_ERROR(json_get_new_id(&hInstance, &hId2, JSON_SUB_OBJ));
  CHECK_JSON_RETURN(json_is_object(&hInstance, hId2), JSON_TRUE);
  CHECK_JSON_ERROR(json_object_set_new(&hInstance, hId1, "object2", hId2));
  CHECK_JSON_ERROR(json_object_set_string(&hInstance, hId2, "", "string", "ok"));
  CHECK_JSON_ERROR(json_object_get_string(&hInstance, hRoot, "object1.object2", "string", &pString));
  CHECK_JSON_STRING(pString, "ok");
  json_unit_print_result("Nexted sub object r/w");


  /* Check simple array like {"object":["1","2"]} readback and check*/

  json_unit_test_reset(false);
  CHECK_JSON_ERROR(json_get_new_id(&hInstance, &hId1, JSON_SUB_ARRAY));
  CHECK_JSON_RETURN(json_is_array(&hInstance, hId1), JSON_TRUE);
  CHECK_JSON_ERROR(json_object_set_new(&hInstance, hRoot, "array", hId1));
  CHECK_JSON_ERROR(json_create_string(&hInstance, "value1", &hStr1));
  CHECK_JSON_ERROR(json_create_string(&hInstance, "value2", &hStr2));
  CHECK_JSON_ERROR(json_array_append_new(&hInstance, hId1, hStr1));
  CHECK_JSON_ERROR(json_array_append_new(&hInstance, hId1, hStr2));
  json_unit_print_result("Create Simple Array ");


  /* Check  array with objects like {"object":["1",{"test":1}]} readback and check*/


  CHECK_JSON_ERROR(json_get_new_id(&hInstance, &hId3, JSON_SUB_OBJ));
  CHECK_JSON_ERROR(json_object_set_string(&hInstance, hId3, "", "string", "ok"));
  CHECK_JSON_ERROR(json_array_append_new(&hInstance, hId1, hId3));
  CHECK_JSON_ERROR(json_array_get(&hInstance, hRoot, "array", &hId4));
  CHECK_JSON_RETURN(json_is_array(&hInstance, hId4), JSON_TRUE);
  CHECK_JSON_ERROR(json_list_get_count(&hInstance, hId4, &iCount));
  CHECK_JSON_INTEGER(3, iCount);

  CHECK_JSON_ERROR(json_list_pair(&hInstance, hId4, 0, &hKey, &hValue));
  CHECK_JSON_ERROR(json_get_string_from_id(&hInstance, hValue, &pString));
  CHECK_JSON_INTEGER(hKey, 0);
  CHECK_JSON_STRING(pString, "value1");

  CHECK_JSON_ERROR(json_list_pair(&hInstance, hId4, 1, &hKey, &hValue));
  CHECK_JSON_ERROR(json_get_string_from_id(&hInstance, hValue, &pString));
  CHECK_JSON_INTEGER(hKey, 0);
  CHECK_JSON_STRING(pString, "value2");

  CHECK_JSON_ERROR(json_list_pair(&hInstance, hId4, 2, &hKey, &hValue));
  CHECK_JSON_INTEGER(hKey, 0);
  CHECK_JSON_ERROR(json_object_get_string(&hInstance, hValue, "", "string", &pString));
  CHECK_JSON_STRING(pString, "ok");
  json_unit_print_result("Check Array with objects r/w");


  /* check the output file format (compact only) */

  json_unit_test_reset(false);
  const char *pDump = json_dumps(&hInstance, hRoot, 0);
  CHECK_JSON_STRING(pDump, tRefJson);
  json_free((void *)pDump);
  json_unit_print_result("Dump compact format");

  /* Check the load  json string */

  json_unit_test_reset(true);
  CHECK_JSON_RETURN(json_load(&hInstance, tRefJson, &hRoot), JSON_OK);
  json_unit_print_result("Load correct json string ");

  json_unit_test_reset(true);
  CHECK_JSON_RETURN(json_load(&hInstance, "{\"test\":1,,\"test2\":2}", &hRoot), JSON_ERROR);
  json_unit_print_result("Load corrupted json string ");
  json_shutdown(&hInstance);

}

