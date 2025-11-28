/**
******************************************************************************
* @file          plugin_terminal_cmd_designer.c
* @author        MCD Application Team
* @brief         Manage terminal communication with designer
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

#ifdef USE_LIVETUNE_DESIGNER

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include "st_base.h"
#include "st_registry.h"
#include "livetune.h"
#include "plugin_terminal_cmd.h"
#include "livetune_db_json.h"
#include "livetune_helper.h"
#include "st_os_monitor_cpu.h"
#include "string.h"


#define ST_INSTANCE_MAX        128LL


/* Global variables ----------------------------------------------------------*/
static uint32_t connectCount = 0;

/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private declaration ------------------------------------------------------- */
static void plugin_terminal_cmd_stop(int argc, char_t *argv[]);


/* Private functions ------------------------------------------------------- */




/**
* @brief overload message handler
*/


uint32_t  plugin_terminal_cmd_parse_designer_event(ST_Handle hInst, ST_Message_Event evt, st_message_param wParam, st_message_param pParam)
{
  if (evt == ST_EVT_START_ISSUE_MSG)
  {
    /* there are an issue after the start, dispatch it to the tool */
    char_t *pMsg = (char_t *)ST_LOW_INT64(pParam);
    livetune_send_block_text_async(FALSE, "startMsg", "Message", NULL, pMsg);
  }
  if (evt == ST_EVT_UPDATE_CONFIG_MSG)
  {
    /* there a param update, dispatch it to the tool */
    char_t *pMsg = (char_t *)ST_LOW_INT64(pParam);
    livetune_send_block_text_async(FALSE, "Update", "params", NULL, pMsg);
  }
  if ((evt == ST_EVT_STOP_PIPE) || (evt == ST_EVT_PIPE_BUILT))
  {
    /*the pipe is about to start or stop, , dispatch it to the tool  to clear the notification*/
    livetune_send_block_text_async(FALSE, "startMsg", "Message", NULL, "");
  }
  if (evt == ST_EVT_SYSTEM_READY)
  {
    livetune_send_sync_host();
  }
  if (evt == ST_EVT_PIPE_STOP_ERROR)
  {
    /* if the pipe report an error during the playback, we stop */
    plugin_terminal_cmd_stop_pipe();
  }
  return 0U;
}




/**
* @brief Will be called every secondes
*/

static void plugin_terminal_Idle_heart_beat(void)
{
  char_t tScratch[128];
  float  currentPercent, interruptsPcent;
  st_os_monitor_cpu_percent(&currentPercent, NULL);
  acCyclesDump(false, true, &interruptsPcent, NULL);  // do not display on terminal and compute only interrupts cpu load
  /* notify in same time, the pipe state and the cpu load */
  snprintf(tScratch, sizeof(tScratch), "\"State\":%d,\"cpu\":%.0f", UTIL_TERM_get_instance()->hAudioPipe.bPipeState, (double)currentPercent + (double)interruptsPcent);
  livetune_send_block_text_async(FALSE, "Heart", "beat", tScratch, NULL);
}



/**
* @brief when the terminal wants to access to the registry,
  we must stop the pipe because some algos access the flash ext
  and during the registry save, the memory mapping will be stopped ie: crash
  so we need to stop the pipe and inform the designer live tune before all flash prog
*/


void plugin_terminal_cmd_stop_pipe(void)
{
  livetune_pipe_stop(&livetune_get_instance()->hAudioPipe);
  plugin_terminal_Idle_heart_beat();


}




/**
* @brief Will be called every secondes
*/

static void plugin_terminal_heart_beat(void)
{
  static uint32_t beatCount = 0;

  if ((connectCount == 0U) && (beatCount == 1U))
  {
    char *pMessage = "";
    /*
    send a reset message after the first beat, this will open the alert "the board is reset ..."
    but if the user has already connected to board, this messages is not sent
    */
    connectCount++;
    st_persist_sys *pSystem = st_registry_lock_sys(&livetune_get_instance()->hRegistry);
    if (pSystem)
    {
      if (pSystem->hUser.bRebootAction)
      {
        pMessage = "valid";
      }
      st_registry_unlock_sys(&livetune_get_instance()->hRegistry, FALSE);
    }
    livetune_send_block_text_async(FALSE, "reset", "Message", NULL, pMessage);
  }
  else
  {
    if (livetune_state_get_flags() & ST_STATE_FLG_HEART)
    {
      if (UTIL_TERM_IsLogMuted() == false)
      {
        plugin_terminal_Idle_heart_beat();
      }
    }
  }
  beatCount++;
}


/* possible overload for the test */

__weak void plugin_terminal_cmd_ack_hook(uint8_t result, char_t *pSucessCmdMsg, char_t *pErrorCmdMsg, char_t *pDesc)
{
}

/* send a command acknowledge for the board */

void plugin_terminal_cmd_ack(uint8_t result, char_t *pSuccessCmdMsg, char_t *pErrorCmdMsg, char_t *pDesc, char_t *pExtra)
{
  plugin_terminal_cmd_ack_hook(result, pSuccessCmdMsg, pErrorCmdMsg, pDesc);
  if (result == 0U)
  {
    livetune_send_block_text_async(TRUE, "Ack", "Message", pExtra, (pErrorCmdMsg == NULL) ? "" : pErrorCmdMsg);
  }
  else
  {
    livetune_send_block_text_async(TRUE, "Ack", "Message", pExtra, (pSuccessCmdMsg == NULL) ? "" : pSuccessCmdMsg);
  }
}


/**
* @brief Hook start, add the heart beat
*/

void UTIL_TERM_Initialized(UTIL_TERM_t *gContext)
{
  UTIL_TERM_add_Idle_listener(plugin_terminal_heart_beat);
}



/**
* @brief Reset board
*
* @param argc  num args
* @param argv  args list
*/

static void plugin_terminal_cmd_reboot(int argc, char_t *argv[])
{
  if (argc != 1)
  {
    UTIL_TERM_printf_forced_cr("Error:Wrong Command parameters");
    return;
  }
  livetune_reset();
}

/**
* @brief Expose platform params
* @param pInstance json instance
* @param root json root id
*/
uint8_t plugin_terminal_send_config(void)
{
  uint8_t  bResult  = FALSE;
  json_instance_t hJson;
  const char_t   *pJson = NULL;

  memset(&hJson, 0, sizeof(hJson));
  json_create_object(&hJson, &hJson.pack_root);
  if (hJson.pack_root)
  {
    json_object_set_string(&hJson, hJson.pack_root, "", "Signature", "CnxConfig");
    char tScratch[50];
    audio_persist_get_config_id(audio_persist_get_config_cur(), tScratch, sizeof(tScratch));
    json_object_set_string(&hJson, hJson.pack_root, "", "AudioConfig", tScratch);
    snprintf(tScratch, sizeof(tScratch), "HW_%s", STM32_NAME_STRING);
    json_object_set_string(&hJson, hJson.pack_root, "", "HWConfig", tScratch);
    json_object_set_boolean(&hJson, hJson.pack_root, "", "CanImport", 1);
    json_object_set_boolean(&hJson, hJson.pack_root, "", "CanExport", 1);
    json_object_set_boolean(&hJson, hJson.pack_root, "", "CanFlash", 1);
    json_object_set_boolean(&hJson, hJson.pack_root, "", "CanNew", 1);
    pJson = json_dumps(&hJson, hJson.pack_root, 0);
    json_shutdown(&hJson);
    if (pJson)
    {
      livetune_send_and_wait_consumed((void *)pJson, strlen(pJson), TRUE);
      json_free((char_t *)pJson);
      bResult = TRUE;
    }
  }

  return bResult;
}




/**
* @brief  export the json instances ( .livetune file )
*
* @param argc  num args
* @param argv  args list
*/
static void plugin_terminal_cmd_get_instances(int argc, char_t *argv[])
{
  uint8_t  bOk      = FALSE;
  uint16_t dumpFlag = 0;
  uint8_t  bResult  = FALSE;

  /* compact produces a json without indentation, it is faster to download */
  if (argc == 1)
  {
    bOk = TRUE;
  }
  else if (argc == 2)
  {
    if (strcmp(argv[1], "formatted") == 0)
    {
      bOk      = TRUE;
      dumpFlag = JSON_INDENT;
    }
  }
  if (bOk)
  {
    const char *pJson = NULL;

    bResult = plugin_terminal_send_config();
    if (bResult)
    {
      json_instance_t jsonInst;
      memset(&jsonInst, 0, sizeof(jsonInst));
      json_create_object(&jsonInst, &jsonInst.pack_root);
      if (jsonInst.pack_root)
      {
        if (livetune_db_json_dump(&UTIL_TERM_get_instance()->hDesignerDB, &jsonInst) == ST_OK)
        {

          json_object_set_string(&jsonInst, jsonInst.pack_root, "", "Signature", "Instance");
          json_object_set_string(&jsonInst, jsonInst.pack_root, "", "Version", AC_VERSION);
          json_object_set_boolean(&jsonInst, jsonInst.pack_root, "", "Editable", 1);
          json_object_set_integer(&jsonInst, jsonInst.pack_root, "", "InstanceMax", ST_INSTANCE_MAX);
          json_object_set_integer(&jsonInst, jsonInst.pack_root, "", "ConnectionOutMax", ST_AC_DEFAULT_OUT_MAX);
          json_object_set_integer(&jsonInst, jsonInst.pack_root, "", "ConnectionInMax", ST_AC_DEFAULT_IN_MAX);
          json_object_set_string(&jsonInst, jsonInst.pack_root, "", "ProjectType", "AudioKit");
          pJson = json_dumps(&jsonInst, jsonInst.pack_root, dumpFlag);
          if (pJson != NULL)
          {
            bResult = TRUE;
          }
        }
      }
      json_shutdown(&jsonInst);
      if (pJson)
      {
        livetune_send_and_wait_consumed((void *)pJson, strlen(pJson), TRUE);
        json_free((char_t *)pJson);
      }
    }
  }
  plugin_terminal_cmd_ack(bResult, NULL, "Error: Load Instance", "get_instances", NULL);
}


/**
* @brief  Save instances from the persistent
*
* @param argc  num args
* @param argv  args list
*/
static void plugin_terminal_cmd_save(int argc, char_t *argv[])
{
  /* Stop to prevent noise garbages during the flash access & crash when access to the flash */
  plugin_terminal_cmd_stop_pipe();
  uint8_t bResult = (livetune_db_json_save(&UTIL_TERM_get_instance()->hDesignerDB) == ST_OK) ? 1U : 0U;
  plugin_terminal_cmd_ack(bResult, "Save: successfully", "Save: Error", "save", NULL);
}



/**
* @brief  Reset  the graph
*
* @param argc  num args
* @param argv  args list
*/
static void plugin_terminal_cmd_new(int argc, char_t *argv[])
{
  /* stop to prevent noise garbages during the flash access & crash when access to the flash */
  plugin_terminal_cmd_stop_pipe();
  uint8_t bResult = (livetune_db_new(&UTIL_TERM_get_instance()->hDesignerDB) == ST_OK) ? 1U : 0U;
  plugin_terminal_cmd_ack(bResult, NULL, "Error: New", "new", NULL);
}


/**
* @brief  Load instances from the persistent
*
* @param argc  num args
* @param argv  args list
*/
static void plugin_terminal_cmd_load(int argc, char_t *argv[])
{
  /* stop to prevent noise garbages during the flash access & crash when access to the flash */
  plugin_terminal_cmd_stop_pipe();
  uint8_t bResult = (livetune_db_json_load(&UTIL_TERM_get_instance()->hDesignerDB) == ST_OK) ? 1U : 0U;
  plugin_terminal_cmd_ack(bResult, NULL, "Error: Load", "load", NULL);
}




/**
* @brief  Import  the json instances ( .livetune file)
*
* @param argc  num args
* @param argv  args list
*/
static void plugin_terminal_cmd_set_instances(int argc, char_t *argv[])
{
  uint8_t  bResult = 0U;
  uint32_t syncTmo = 60;
  char_t  *pJson   = NULL;
  pJson = NULL;
  uint32_t szRead;
  /* stop to prevent noise garbages during the flash access & crash when access to the flash */
  plugin_terminal_cmd_stop_pipe();

  int32_t  error = UTIL_TERM_receive_file(&pJson, &szRead, syncTmo * 1000U);
  if (error == UTIL_ERROR_NONE)
  {
    if (livetune_db_json_parse(&UTIL_TERM_get_instance()->hDesignerDB, pJson) != ST_OK)
    {
      UTIL_TERM_printf_forced_cr("json file invalid %s", pJson);
    }
    else
    {
      UTIL_TERM_printf_forced_cr("Import and parsing successful: %d bytes", szRead);
      bResult = 1U;
    }
  }

  if (pJson)
  {
    st_os_mem_free(pJson);
  }
  plugin_terminal_cmd_ack(bResult, NULL, "Error: set_instance", "set_instance", NULL);
}

/**
* @brief  Update the json instances
*
* @param argc  num args
* @param argv  args list
*/
static void plugin_terminal_cmd_update_instances(int argc, char_t *argv[])
{
  uint8_t  bOk     = 0U;
  uint8_t  bResult = 0U;
  uint32_t syncTmo = 60UL;
  char_t  *pJson   = NULL;
  uint32_t szRead  = 0UL;
  if (argc == 1)
  {
    bOk = TRUE;
  }
  pJson = NULL;
  if ((bOk != 0U))
  {
    int32_t  error = UTIL_TERM_receive_file(&pJson, &szRead, syncTmo * 1000UL);
    UTIL_TERM_printf_forced_cr("");
    if ((szRead > 0U) && (error == UTIL_ERROR_NONE))
    {
      if (livetune_db_json_update(&UTIL_TERM_get_instance()->hDesignerDB, pJson) == ST_OK)
      {
        bResult = 1U;
      }
    }
    else
    {
      UTIL_TERM_printf_forced_cr("Aborted or error");
    }
  }
  if (pJson)
  {
    st_os_mem_free(pJson);
  }
  plugin_terminal_cmd_ack(bResult, NULL, "Error: Update", "update_instances", NULL);
}

/**
* @brief  Export the json instances
*
* @param argc  num args
* @param argv  args list
*/
static void plugin_terminal_cmd_get_elements(int argc, char_t *argv[])
{
  uint8_t bResult = FALSE;

  uint32_t nbElement = livetune_db_element_nb_get(&UTIL_TERM_get_instance()->hDesignerDB);
  char_t  headerJson[50];
  for (uint32_t indexElem = 0; indexElem < nbElement; indexElem++)
  {

    livetune_db_element *pElement = livetune_db_element_get(&UTIL_TERM_get_instance()->hDesignerDB, indexElem);
    if (pElement)
    {
      char_t tNum[10];
      snprintf(tNum, sizeof(tNum), "%d", indexElem);
      headerJson[0] = '\0';
      strcat(headerJson, "{\"Signature\":\"Element\",");
      strcat(headerJson, "\"Num\":");
      strcat(headerJson, tNum);
      strcat(headerJson, ",");
      strcat(headerJson, "\"Element\":");
      ST_ASSERT(strlen(headerJson) < sizeof(headerJson));
      char_t *pJsonDesc = livetune_ac_json_update_element(&UTIL_TERM_get_instance()->hDesignerDB, pElement, strlen(headerJson));
      if (pJsonDesc)
      {
        /* Copy the header in the provisioned size */
        memcpy(pJsonDesc, headerJson, strlen(headerJson));
        /* grow the buffer to add the terminator */
        char *pJsonFinal = st_os_mem_realloc(ST_Mem_Type_ANY_SLOW, pJsonDesc, strlen(pJsonDesc) + 2U);
        if (pJsonFinal)
        {
          /* add the terminator */
          strcat(pJsonFinal, "}");
          /* send the payload */
          livetune_send_and_wait_consumed((void *)pJsonFinal, strlen(pJsonFinal), TRUE);
        }
        else
        {
          /* if a realloc return null, the pointer to grow is not freed */
          st_os_mem_free((void *)pJsonDesc);
          UTIL_TERM_printf_forced_cr("Alloc Error:Json num %d:%s", indexElem, pElement->pName);
        }
        st_os_mem_free((void *)pJsonFinal);
      }
      else
      {
        UTIL_TERM_printf_forced_cr("Alloc Error:Json num %d:%s", indexElem, pElement->pName);
      }
      //      st_os_task_delay(5);
    }
    else
    {
      UTIL_TERM_printf_forced_cr("Unknown element");
    }
    bResult = TRUE;
  }
  plugin_terminal_cmd_ack(bResult, NULL, "Error: Get element", "get_elements", NULL);
}



/**
* @brief  Designer connection , send elements and instances
*
* @param argc  num args
* @param argv  args list
*/
static void plugin_terminal_cmd_connect(int argc, char_t *argv[])
{
  /* send a sync message in order to purge the serial queue */
  livetune_send_sync_host();

  /* stop to prevent noise garbages during the flash access & crash when access to the flash */
  plugin_terminal_cmd_stop_pipe();

  /* send the ack to prevent no response message  */
  plugin_terminal_cmd_ack(TRUE, NULL, "Error: Connect", "Connect", NULL);
  /* Disable the reset message if armed */
  if (connectCount == 0U)
  {
    connectCount++;
  }
  /* the system is maybe booting, wait the end before to send all elements */
  livetune_wait_ready();
  /* send elements */
  plugin_terminal_cmd_get_elements(argc, argv);
  /* send the project */
  plugin_terminal_cmd_get_instances(argc, argv);
}


/**

* @brief start the audio pipe
*
* @param argc  num args
* @param argv  args list
*/

static void plugin_terminal_cmd_start(int argc, char_t *argv[])
{
  uint8_t bResult = (livetune_pipe_start(&UTIL_TERM_get_instance()->hAudioPipe) == ST_OK) ? 1U : 0U;
  /* When a error occurs during the start, we directly switch the terminal to check the error */
  char *pExtra = (bResult == 0U) ? " \"cmd\" : \"show_terminal\"" : NULL;
  /* first Ack the start because the timeout could fire  */
  plugin_terminal_cmd_ack(bResult, NULL, "Error: Pipe Start, invalid pipeline.<br>Please check the log in the terminal", "start", pExtra);
  /* header color update immediate */
  plugin_terminal_Idle_heart_beat();
  /* clear icons start message warning */
  livetune_send_block_text_async(FALSE, "startMsg", "Message", NULL, "");
}

/**

* @brief Stop the audio pipe
*
* @param argc  num args
* @param argv  args list
*/

static void plugin_terminal_cmd_stop(int argc, char_t *argv[])
{
  uint8_t bResult = (livetune_pipe_stop(&UTIL_TERM_get_instance()->hAudioPipe) == ST_OK) ? 1U : 0U;
  /* header color update immediate */
  plugin_terminal_Idle_heart_beat();
  plugin_terminal_cmd_ack(bResult, NULL, "Error: Pipe Start", "stop", NULL);
}


/**

* @brief Generate the code
*
* @param argc  num args
* @param argv  args list
*/

static void plugin_terminal_cmd_generate(int argc, char_t *argv[])
{
  uint8_t result = 0U;

  char_t *pCode = livetune_generate_code(&UTIL_TERM_get_instance()->hAudioPipe);
  if (pCode)
  {
    size_t codeSize = strlen(pCode);
    if (codeSize > 0UL)
    {
      livetune_send_block_text_async(FALSE, "Text", "Audio chain Code generation", "\"Viewer\":\"codepreview\"", pCode);
      st_os_mem_free(pCode);
      result = 1U;
    }
  }
  plugin_terminal_cmd_ack(result, NULL, "Error:Generation", "generate", NULL);
}

TERM_CMD_DECLARE_HIDE("set_instances", NULL, "Export the json instance", plugin_terminal_cmd_set_instances);
TERM_CMD_DECLARE_HIDE("get_instances", "[formatted]", "Export the json instance", plugin_terminal_cmd_get_instances);
TERM_CMD_DECLARE_HIDE("update_instances", NULL, "Update the graph", plugin_terminal_cmd_update_instances);
TERM_CMD_DECLARE_HIDE("get_elements", NULL, "Export the json elements", plugin_terminal_cmd_get_elements);
TERM_CMD_DECLARE_HIDE("connect", NULL, "Connect the board with the designer", plugin_terminal_cmd_connect);
TERM_CMD_DECLARE("start", NULL, "Audio pipe start ", plugin_terminal_cmd_start);
TERM_CMD_DECLARE("stop", NULL, "Audio pipe stop", plugin_terminal_cmd_stop);
TERM_CMD_DECLARE_HIDE("save", NULL, "Audio pipe save", plugin_terminal_cmd_save);
TERM_CMD_DECLARE_HIDE("load", NULL, "Audio pipe load", plugin_terminal_cmd_load);
TERM_CMD_DECLARE_HIDE("new", NULL, "Reset the graph", plugin_terminal_cmd_new);
TERM_CMD_DECLARE("reboot", NULL, "Reboot the board", plugin_terminal_cmd_reboot);
TERM_CMD_DECLARE_HIDE("generate", NULL, "generate source code", plugin_terminal_cmd_generate);


#endif
