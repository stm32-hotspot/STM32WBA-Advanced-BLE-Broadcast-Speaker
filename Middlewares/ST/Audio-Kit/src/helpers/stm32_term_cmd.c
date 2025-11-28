/*
******************************************************************************
* @file    stm32_term_cmd.c
* @author  MCD Application Team
* @brief   Custom definition of commands for terminal
******************************************************************************
* @attention
*
* Copyright (c) 2022(-2022) STMicroelectronics.
* All rights reserved.
*
* This software is licensed under terms that can be found in the LICENSE file
* in the root directory of this software component.
* If no LICENSE file comes with this software, it is provided AS-IS.
*
******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <errno.h>
#include "stm32_term_conf.h"
#include "stm32_term.h"
#include "audio_chain_instance.h"
#include "cycles_measure.h"
#include "algos_memory_usage.h"
#ifdef USE_ETHERNET
  #include "network.h"
#endif
#include "audio_algo_tuning.h"
#include "st_json.h"

/* Global variables ----------------------------------------------------------*/

/* Private defines -----------------------------------------------------------*/
#define MAX_CHUNK_DESCR_STR_SIZE     (200)
#define UTIL_TERM_CMD_JSON_BUFF_SIZE (10UL * 1024UL)
#define AC_PARAM_NAME_LENGTH_MAX     (100U)

/* Private macros ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
typedef struct
{
  const char *pTitle;
  const char *pID;
  const char *pCmd;
  const char *pType;
  uint32_t (*get_value_cb)(void);
}
cfg_def_toolsbar_t;

typedef struct
{
  char pJsonBuff[UTIL_TERM_CMD_JSON_BUFF_SIZE];
}
UTIL_TERM_CMD_t;

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
static void       s_cmd_get_algo_list(int        argc, char *argv[]);
static void       s_cmd_get_cfg(int              argc, char *argv[]);
static void       s_cmd_set_cycles_cnt_state(int argc, char *argv[]);
static void       s_cmd_print_cpu_status(int     argc, char *argv[]);
static void       s_cmd_print_mem_status(int     argc, char *argv[]);
static void       s_cmd_print_mem2_status(int    argc, char *argv[]);
static void       s_cmd_set_traces_state(int     argc, char *argv[]);

static bool       s_check_args_boolean(int argc, char *argv[], bool *const pBoolean);

static uint32_t   s_get_cycles_cnt_status_cb(void);
static uint32_t   s_get_trace_status_cb(void);

static jsonErr    s_get_cfg_build_tools_bar(json_pack_t        *const pInstance, jsonID const root);
static jsonErr    s_update_json_graph_connections(json_pack_t  *const pInstance, jsonID const root);
static jsonErr    s_update_json_from_ac_params(json_pack_t     *const pInstance, jsonID const algoJsonId, audio_algo_t *const pAlgo, audio_algo_tuning_stat_dyn_param_t const statDynParam);
static jsonErr    s_update_json_graph_system_chunk(json_pack_t *const pInstance, jsonID const root, audio_chunk_list_t *const pChunkList, const char *const strType);
static jsonErr    s_update_json_graph_system_ios(json_pack_t   *const pInstance, jsonID const root);
//static void       s_cmd_toggle_usb_rec(int argc, char *argv[]);
static void       s_cmd_get_generic_info(int argc, char *argv[]);

#ifdef USE_ETHERNET
  static void     s_cmd_set_net_traces_state(int argc, char *argv[]);
  static uint32_t s_get_net_trace_status_cb(void);
#endif

#if defined(AUDIO_CHAIN_ACSDK_USED) || defined(AUDIO_CHAIN_CONF_TUNING_CLI_USED)
/* Private variables ---------------------------------------------------------*/
static const char *const s_pStrParamType[] =
{
  [AUDIO_ALGO_STATIC_PARAM]  = "Static",
  [AUDIO_ALGO_DYNAMIC_PARAM] = "Dynamic"
};

static const char *const s_pStrJsonParamType[] =
{
  [AUDIO_ALGO_STATIC_PARAM]  = "StaticParameters",
  [AUDIO_ALGO_DYNAMIC_PARAM] = "DynamicParameters"
};

/* Private function prototypes -----------------------------------------------*/
static void     s_cmd_get_all_algo_param_list(int argc, char *argv[]);
static void     s_cmd_get_all_param_list(int      argc, char *argv[]);
static void     s_cmd_get_dynamic_param_list(int  argc, char *argv[]);
static void     s_cmd_get_dynamic_param_val(int   argc, char *argv[]);
static void     s_cmd_set_dynamic_param_val(int   argc, char *argv[]);
static void     s_cmd_apply_dynamic_param_val(int argc, char *argv[]);
static void     s_cmd_get_static_param_list(int   argc, char *argv[]);
static void     s_cmd_get_static_param_val(int    argc, char *argv[]);
static void     s_cmd_set_static_param_val(int    argc, char *argv[]);
static void     s_cmd_apply_static_param_val(int  argc, char *argv[]);
static void     s_cmd_apply_cfg(int               argc, char *argv[]);
static void     s_cmd_set_cfg(int                 argc, char *argv[]);

static void     s_args_get_param_list(int         argc, char *argv[], audio_algo_tuning_stat_dyn_param_t const statDynParam);
static void     s_args_get_param_val(int          argc, char *argv[], audio_algo_tuning_stat_dyn_param_t const statDynParam);
static int      s_args_set_param_val(int          argc, char *argv[], audio_algo_tuning_stat_dyn_param_t const statDynParam);

static int      s_check_args_algoId_paramId(int   argc, char *argv[], audio_algo_t **const ppAlgo, uint8_t *const pAlgoId, uint8_t *const pParamId, audio_algo_tuning_stat_dyn_param_t const statDynParam);
static int      s_check_args_algoId(int           argc, char *argv[], audio_algo_t **const ppAlgo, uint8_t *const pAlgoId);

static void     s_set_param_val(audio_algo_t          *pAlgo, uint8_t const algoId, audio_algo_tuning_stat_dyn_param_t const statDynParam, uint8_t const paramId, char *const pStrParamVal);
static void     s_print_param_val(audio_algo_t        *pAlgo, uint8_t const algoId, audio_algo_tuning_stat_dyn_param_t const statDynParam, uint8_t const paramId);
static int32_t  s_getParamValStrWithName(audio_algo_t *pAlgo, uint8_t const algoId, audio_algo_tuning_stat_dyn_param_t const statDynParam, uint8_t const paramId, char *const pString, size_t const size);

static jsonErr  s_update_json_from_ac_param(json_pack_t  *const pInstance, jsonID const paramTypeJsonId, audio_algo_t *const pAlgo, audio_algo_tuning_stat_dyn_param_t const statDynParam, uint8_t const paramId);
static jsonErr  s_update_ac_param_from_json(json_pack_t  *const pInstance, jsonID const paramTypeJsonId, audio_algo_t *const pAlgo, audio_algo_tuning_stat_dyn_param_t const statDynParam, uint8_t const paramId);
static jsonErr  s_update_ac_params_from_json(json_pack_t *const pInstance, jsonID const algoJsonId,      audio_algo_t *const pAlgo, audio_algo_tuning_stat_dyn_param_t const statDynParam);
#endif

//__weak void  toggle_usb_rec(void);
__weak void  get_infos(void);



/**
* @brief Command definition
*
*/
const UTIL_TERM_cmd_str_entry_t gUTIL_TERM_customCmds[] =
{
  {"help",              NULL,                             "This help",                                                   UTIL_TERM_cmd_help,                0},
  {"reboot",            NULL,                             "Reboot the board",                                            UTIL_TERM_cmd_reboot,              0},
  {"get_algo_list",     NULL,                             "display algo list in current graph",                          s_cmd_get_algo_list,               0},
  #if defined(AUDIO_CHAIN_ACSDK_USED) || defined(AUDIO_CHAIN_CONF_TUNING_CLI_USED)
  {"get_all_par",       NULL,                             "display all algo's static & dynamic param",                   s_cmd_get_all_algo_param_list,     0},
  {"get_par_list",      "algo id",                        "display algo's static & dynamic param list",                  s_cmd_get_all_param_list,          0},
  {"get_stat_par_list", "algo id",                        "display algo's static param list",                            s_cmd_get_static_param_list,       0},
  {"get_dyn_par_list",  "algo id",                        "display algo's dynamic param list",                           s_cmd_get_dynamic_param_list,      0},
  {"get_stat_par",      "algo id, param id",              "display algo's static param value",                           s_cmd_get_static_param_val,        0},
  {"get_dyn_par",       "algo id, param id",              "display algo's dynamic param value",                          s_cmd_get_dynamic_param_val,       0},
  {"set_stat_par",      "algo id, param id, param value", "set algo's static param value (not applied)",                 s_cmd_set_static_param_val,        0},
  {"set_dyn_par",       "algo id, param id, param value", "set algo's dynamic param value (not applied)",                s_cmd_set_dynamic_param_val,       0},
  {"apply_stat_par",    "algo id, param id, param value", "set algo's static param value and apply it",                  s_cmd_apply_static_param_val,      0},
  {"apply_dyn_par",     "algo id, param id, param value", "set algo's dynamic param value and apply it",                 s_cmd_apply_dynamic_param_val,     0},
  {"apply_cfg",         "algo id",                        "apply new algo's config (static & dynamic params)",           s_cmd_apply_cfg,                   0},
  {"set_cfg",           "[quiet]",                        "Apply the json configuration",                                s_cmd_set_cfg,                     0},
  #endif
  {"get_cfg",           "[compact]",                      "Show the json configuration",                                 s_cmd_get_cfg,                     0},
  {"cpu",               NULL,                             "Print the cpu status",                                        s_cmd_print_cpu_status,            0},
  {"mem",               NULL,                             "Print the memory status with algos memory usage summary",     s_cmd_print_mem_status,            0},
  {"mem2",              NULL,                             "Print the memory status with algos detailed memory usage",    s_cmd_print_mem2_status,           0},
  {"set_cycles_cnt",    "0 or 1",                         "enable(1) disable(0) cycle counting",                         s_cmd_set_cycles_cnt_state,        0},
  {"set_traces",        "0 or 1",                         "enable(1) disable(0) traces",                                 s_cmd_set_traces_state,            0},
  #ifdef USE_ETHERNET
  {"set_net_traces",    "0 or 1",                         "enable(1) disable(0) network traces",                         s_cmd_set_net_traces_state,        0},
  #endif
  //  {"toggle_usb_rec",    NULL,                             "toggles from Mic to Afe output",                              s_cmd_toggle_usb_rec,              0},
  {"get_infos",         NULL,                             "user defined command, up to the developer to implemented it", s_cmd_get_generic_info,            0},

  {NULL,                NULL,                             NULL,                                                          NULL,                              0}
};


/* Functions Definition ------------------------------------------------------*/

/**
* @brief Must return pointer on custom commands
*
* @param argc  num args
* @param argv  args list
*/
UTIL_TERM_cmd_str_entry_t const *UTIL_TERM_getCustomCommands(void)
{
  return gUTIL_TERM_customCmds;
}


/* Static Functions Definition -----------------------------------------------*/

static void s_cmd_get_algo_list(int argc, char *argv[])
{
  if (AudioChain_getAlgosList(&AudioChainInstance) == NULL)
  {
    UTIL_TERM_printf("No Audio Chain is instantiated\n");
  }
  for (audio_algo_list_t *pCurrent = AudioChain_getAlgosList(&AudioChainInstance); pCurrent != NULL; pCurrent = pCurrent->next)
  {
    audio_algo_t *pAlgo = pCurrent->pAlgo;

    UTIL_TERM_printf("%2d: %s %s\n", AudioAlgo_getIndex(pAlgo), pAlgo->pName, pAlgo->pDesc);
  }
}


static void s_cmd_set_cycles_cnt_state(int argc, char *argv[])
{
  bool enable;

  if (s_check_args_boolean(argc, argv, &enable))
  {
    AudioChainInstance_setEnableCyclesCnt(enable);
  }
}

static void s_cmd_print_cpu_status(int argc, char *argv[])
{
  cyclesMeasure_values_t cyclesMeasure_values;

  displayDetailedCycleCount(true, false, &cyclesMeasure_values);
}


static void s_cmd_print_mem_status(int argc, char *argv[])
{
  displayDetailedAlgosMemoryUsage(false);
}


static void s_cmd_print_mem2_status(int argc, char *argv[])
{
  displayDetailedAlgosMemoryUsage(true);
}


static void s_cmd_set_traces_state(int argc, char *argv[])
{
  bool enable;

  if (s_check_args_boolean(argc, argv, &enable))
  {
    AudioChainInstance_setEnableTraces(enable);
  }
}


static bool s_check_args_boolean(int argc, char *argv[], bool *const pBoolean)
{
  bool ok = true;

  if (argc == 2)
  {
    if (AudioAlgoTuning_isSignedDecimal(argv[1]))
    {
      int boolean;

      errno = 0;
      boolean = (int)strtol(argv[1], NULL, 10);
      if (errno != 0)
      {
        UTIL_TERM_printf("%s command: strtol error\n", argv[0]);
        ok = false;
      }
      if (ok)
      {
        if ((boolean != 0) && (boolean != 1))
        {
          UTIL_TERM_printf("%s command: %s is not a boolean\n", argv[0], argv[1]);
          ok = false;
        }
        else
        {
          *pBoolean = (boolean == 1);
        }
      }
    }
    else
    {
      UTIL_TERM_printf("%s command: %s is not an integer\n", argv[0], argv[1]);
      ok = false;
    }
  }
  else
  {
    UTIL_TERM_printf("%s command: wrong number of arguments\n", argv[0]);
    ok = false;
  }

  return ok;
}


/****************************/
/* JSON management routines */
/****************************/

static uint32_t s_get_trace_status_cb(void)
{
  return (uint32_t)AudioChainInstance_getTracesStatus();
}


static uint32_t s_get_cycles_cnt_status_cb(void)
{
  return (uint32_t)AudioChainInstance_getCyclesCntStatus();
}


/**
* @brief Expose the terminal boutton bar
* @params pInstance json instance
* @param root json root id
*/
static jsonErr s_get_cfg_build_tools_bar(json_pack_t *const pInstance, jsonID const root)
{
  static const cfg_def_toolsbar_t tToolsBar[] =
  {
    {
      .pTitle       = "Help",
      .pID          = "ToolBar#00",
      .pCmd         = "help",
      .pType        = "push", /* push button to send a command */
      .get_value_cb = NULL
    },
    {
      .pTitle       = "Toggle traces",
      .pID          = "ToolBar#01",
      .pCmd         = "set_traces {0}",
      .pType        = "state", /* state button is true or false and put a "stateOff" attribute on the UI */
      .get_value_cb = s_get_trace_status_cb
    },
    {
      .pTitle       = "Cycles Count",
      .pID          = "ToolBar#02",
      .pCmd         = "set_cycles_cnt {0}",
      .pType        = "state", /* state button is true or false and put a "stateOff" attribute on the UI */
      .get_value_cb = s_get_cycles_cnt_status_cb
    },
    #ifdef USE_ETHERNET
    {
      .pTitle       = "Network Traces",
      .pID          = "ToolBar#03",
      .pCmd         = "set_net_traces {0}",
      .pType        = "state",  /* state button  is true or false and put a "stateOff" attribute  on the UI */
      .get_value_cb = s_get_net_trace_status_cb
    },
    #endif
  };
  jsonID  toolsID = JSON_ID_NULL;
  jsonErr jsError = JSON_OK;

  JSON_CREATE_ARRAY(jsError, pInstance, &toolsID);
  JSON_OBJECT_SET_NEW(jsError, pInstance, root,     "ToolsBar", toolsID);
  for (size_t a = 0UL; (jsError == JSON_OK) && (a < (sizeof(tToolsBar) / sizeof(tToolsBar[0]))); a++)
  {
    jsonID toolsEntry = JSON_ID_NULL;

    JSON_CREATE_OBJECT(jsError, pInstance, &toolsEntry);
    JSON_OBJECT_SET_STRING(jsError, pInstance, toolsEntry,  "", "Title", tToolsBar[a].pTitle);
    JSON_OBJECT_SET_STRING(jsError, pInstance, toolsEntry,  "", "ID",    tToolsBar[a].pID);
    JSON_OBJECT_SET_STRING(jsError, pInstance, toolsEntry,  "", "Cmd",   tToolsBar[a].pCmd);
    JSON_OBJECT_SET_STRING(jsError, pInstance, toolsEntry,  "", "Type",  tToolsBar[a].pType);
    if (tToolsBar[a].get_value_cb != NULL)
    {
      uint64_t u64 = tToolsBar[a].get_value_cb();
      JSON_OBJECT_SET_UNSIGNED_INTEGER(jsError, pInstance, toolsEntry, "", "Value", u64);
    }
    JSON_ARRAY_APPEND_NEW(jsError, pInstance, toolsID, toolsEntry);
  }
  if (jsError != JSON_OK)
  {
    UTIL_TERM_printf("%s, %s(), line %d: error %d\n", __FILE__, __FUNCTION__, __LINE__, jsError);
  }

  return jsError;
}


/**
* @brief Remove unexpected chars by a space
*
* @param pString the string
* @param ichar   the char to remove
*/

static void s_update_json_graph_remove_unexpected_char(char *pString, char ichar)
{
  while (*pString)
  {
    if (*pString == ichar)
    {
      *pString = ' ';
    }
    pString ++;
  }
}


/**
* @brief Add graph connections
*
* @param root json root id
*/
static jsonErr s_update_json_graph_connections(json_pack_t *const pInstance, jsonID const root)
{
  /* iterate for all instances */
  jsonErr jsError = JSON_OK;
  jsonID  graphID = JSON_ID_NULL;

  JSON_CREATE_ARRAY(jsError, pInstance, &graphID);
  JSON_OBJECT_SET_NEW(jsError, pInstance, root, "Graph", graphID);

  for (audio_algo_list_t *pCurrent = AudioChain_getAlgosList(&AudioChainInstance); (jsError == JSON_OK) && (pCurrent != NULL); pCurrent = pCurrent->next)
  {
    audio_algo_t *const pAlgo = pCurrent->pAlgo;

    /* record pin out connected to pin in */
    if (AudioAlgo_getChunkPtrOut(pAlgo, 0U) != NULL)
    {
      int outPin = 0;

      for (audio_chunk_list_t *pPinListOut = AudioAlgo_getChunksOut(pAlgo); (jsError == JSON_OK) && (pPinListOut != NULL); pPinListOut = pPinListOut->next)
      {
        if (pPinListOut->pChunk != NULL)
        {
          /* we get the connection or the pin in */
          uint8_t inPinId;
          uint8_t inAlgoId;

          if (AudioError_isOk(AudioChainInstance_getChunkAlgoIn(pPinListOut->pChunk, &inAlgoId, &inPinId)))
          {
            char   tTmp[MAX_CHUNK_DESCR_STR_SIZE + 16];
            char   tDesc[MAX_CHUNK_DESCR_STR_SIZE] = "";
            jsonID tmpId = 0U;

            AudioChunk_getDescriptionString(pPinListOut->pChunk, tDesc, sizeof(tDesc));
            /* description should not have # */
            s_update_json_graph_remove_unexpected_char(tDesc, '#');

            snprintf(tTmp, sizeof(tTmp), "%d#%d#%d#%d#%s", (int)AudioAlgo_getIndex(pAlgo), outPin, (int)inAlgoId, (int)inPinId, tDesc);
            JSON_CREATE_STRING(jsError, pInstance, tTmp, &tmpId);
            JSON_ARRAY_APPEND_NEW(jsError, pInstance, graphID, tmpId);

            // chunk's description as string: may be added as chunk info in the graph

          }
        }
        outPin++;
      }
    }
  }
  if (jsError != JSON_OK)
  {
    UTIL_TERM_printf("%s, %s(), line %d: error %d \n", __FILE__, __FUNCTION__, __LINE__, jsError);
  }
  return jsError;
}


/**
* @brief Add system inputs/outputs info
*
* @param jsonID_sysIos json jsonID_sysIos id
*/
static jsonErr s_add_json_graph_system_ios_description(json_pack_t *pInstance, jsonID jsonID_sysIos, uint8_t algoId, uint8_t pinId, audio_chunk_t *pChunk, const char *strType)
{
  jsonErr jsError = JSON_OK;

  char   tTmp[MAX_CHUNK_DESCR_STR_SIZE + 16];
  char   tDesc[MAX_CHUNK_DESCR_STR_SIZE] = "";
  jsonID tmpId = 0U;

  AudioChunk_getDescriptionString(pChunk, tDesc, sizeof(tDesc));
  /* description should not have # */
  s_update_json_graph_remove_unexpected_char(tDesc, '#');
  snprintf(tTmp, sizeof(tTmp), "%d#%d#%s#%s", (int)algoId, (int)pinId, tDesc, strType);
  JSON_CREATE_STRING(jsError, pInstance, tTmp,   &tmpId);
  JSON_ARRAY_APPEND_NEW(jsError, pInstance, jsonID_sysIos, tmpId);
  return jsError;
}


/**
* @brief Add system inputs/outputs info
*
* @param jsonID_sysIos json jsonID_sysIos id
*/
static jsonErr s_update_json_graph_system_chunk(json_pack_t *const pInstance, jsonID const jsonID_sysIos, audio_chunk_list_t *const pChunkList, const char *const strType)
{
  jsonErr jsError = JSON_OK;
  int32_t acError = AUDIO_ERR_MGNT_NONE;

  for (audio_chunk_list_t *pCurrent = pChunkList; (jsError == JSON_OK) && (pCurrent != NULL); pCurrent = pCurrent->next)
  {
    if (pCurrent->pChunk != NULL)
    {
      uint8_t pinId, algoId;

      if (AudioChunk_getType(pCurrent->pChunk) == AUDIO_CHUNK_TYPE_SYS_IN)
      {
        acError = AudioChainInstance_getChunkAlgoIn(pCurrent->pChunk, &algoId, &pinId);
        if (AudioError_isOk(acError)) /* acError = AUDIO_ERR_MGNT_NOT_FOUND if error while searching the id*/
        {
          jsError = s_add_json_graph_system_ios_description(pInstance, jsonID_sysIos, algoId, pinId, pCurrent->pChunk, strType);
        }
      }
      else if (AudioChunk_getType(pCurrent->pChunk) == AUDIO_CHUNK_TYPE_SYS_OUT)
      {
        acError = AudioChainInstance_getChunkAlgoOut(pCurrent->pChunk, &algoId, &pinId);
        if (AudioError_isOk(acError)) /* acError = AUDIO_ERR_MGNT_NOT_FOUND if error while searching the id*/
        {
          jsError = s_add_json_graph_system_ios_description(pInstance, jsonID_sysIos, algoId, pinId, pCurrent->pChunk, strType);
        }
      }
    }
  }
  return jsError;
}


/**
* @brief Add system inputs/outputs info
*
* @param root json root id
*/
static jsonErr s_update_json_graph_system_ios(json_pack_t *const pInstance, jsonID const root)
{
  /* iterate for all instances */
  jsonErr jsError = JSON_OK;
  jsonID  graphID = JSON_ID_NULL;

  JSON_CREATE_ARRAY(jsError, pInstance, &graphID);
  JSON_OBJECT_SET_NEW(jsError, pInstance, root, "SystemIOs", graphID);

  for (audio_algo_list_t *pCurrent = AudioChain_getAlgosList(&AudioChainInstance); (jsError == JSON_OK) && (pCurrent != NULL); pCurrent = pCurrent->next)
  {
    audio_algo_t *const pAlgo = pCurrent->pAlgo;

    /* record pin out connected to pin in */
    if (AudioAlgo_getChunkPtrIn(pAlgo, 0U) != NULL)
    {
      jsError = s_update_json_graph_system_chunk(pInstance, graphID, AudioAlgo_getChunksIn(pAlgo), "src");
    }

    if (AudioAlgo_getChunkPtrOut(pAlgo, 0U) != NULL)
    {
      jsError = s_update_json_graph_system_chunk(pInstance, graphID, AudioAlgo_getChunksOut(pAlgo), "sink");
    }
  }
  if (jsError != JSON_OK)
  {
    UTIL_TERM_printf("%s, %s(), line %d: error %d\n", __FILE__, __FUNCTION__, __LINE__, jsError);
  }
  return jsError;
}


/**
* @brief prints the configuration file
*
* @param argc  num args
* @param argv  args list
*/
static void s_cmd_get_cfg(int argc, char *argv[])
{
  uint8_t  bOk      = FALSE;
  uint16_t dumpFlag = JSON_INDENT;

  /* compact produces a json without indentation, it is faster to download */
  if (argc == 1)
  {
    bOk = TRUE;
  }
  else if (argc == 2)
  {
    if (strcmp(argv[1], "compact") == 0)
    {
      bOk      = TRUE;
      dumpFlag = 0;
    }
  }
  else
  {
    UTIL_TERM_printf("%s command: wrong number of arguments\n", argv[0]);
  }

  if (bOk)
  {
    jsonErr     jsError   = JSON_OK;
    json_pack_t jsonInst  = {0};
    jsonID      root      = JSON_ID_NULL;
    jsonID      algos     = JSON_ID_NULL;
    int32_t     iInstance = 0;

    JSON_CREATE_OBJECT(jsError, &jsonInst, &root);

    /* generate the tile file */
    JSON_OBJECT_SET_STRING(jsError, &jsonInst, root, "", "Title",       AudioChainInstance.pName);
    JSON_OBJECT_SET_STRING(jsError, &jsonInst, root, "", "Description", AudioChainInstance.pDesc);
    JSON_OBJECT_SET_STRING(jsError, &jsonInst, root, "", "Version",     AC_VERSION);

    jsError = s_get_cfg_build_tools_bar(&jsonInst, root);

    /* create the first properties: { */
    JSON_CREATE_ARRAY(jsError, &jsonInst, &algos);
    JSON_OBJECT_SET_NEW(jsError, &jsonInst, root, "Algos", algos);

    /* iterate for all instances */
    for (audio_algo_list_t *pCurrent = AudioChain_getAlgosList(&AudioChainInstance); (jsError == JSON_OK) && (pCurrent != NULL); pCurrent = pCurrent->next)
    {
      audio_algo_t *const pAlgo      = pCurrent->pAlgo;
      jsonID              algoJsonId = 0U;

      JSON_CREATE_OBJECT(jsError, &jsonInst, &algoJsonId);
      if (pAlgo->pDesc != NULL)
      {
        JSON_OBJECT_SET_STRING(jsError, &jsonInst, algoJsonId, "", "Description", pAlgo->pDesc);
      }
      else
      {
        JSON_OBJECT_SET_STRING(jsError, &jsonInst, algoJsonId, "", "Description", "");
      }
      JSON_OBJECT_SET_STRING(jsError, &jsonInst, algoJsonId, "", "Title", pAlgo->pName);
      JSON_OBJECT_SET_UNSIGNED_INTEGER(jsError, &jsonInst, algoJsonId, "", "PinOut", (uint64_t)AudioChunkList_getNbElements(AudioAlgo_getChunksOut(pAlgo)));
      JSON_OBJECT_SET_UNSIGNED_INTEGER(jsError, &jsonInst, algoJsonId, "", "PinIn", (uint64_t)AudioChunkList_getNbElements(AudioAlgo_getChunksIn(pAlgo)));
      JSON_OBJECT_SET_UNSIGNED_INTEGER(jsError, &jsonInst, algoJsonId, "", "InstanceNum", (uint64_t)AudioAlgo_getIndex(pAlgo));
      /* generate visibility */
      //JSON_OBJECT_SET_BOOLEAN(jsError, &jsonInst, algoJsonId, "", "Disabled", (AudioAlgo_getState(pAlgo) == AUDIO_ALGO_TUNING_DISABLED) ? TRUE : FALSE);

      JSON_ARRAY_APPEND_NEW(jsError, &jsonInst, algos, algoJsonId);
      /* create the static array */
      jsError = s_update_json_from_ac_params(&jsonInst, algoJsonId, pAlgo, AUDIO_ALGO_STATIC_PARAM);
      jsError = s_update_json_from_ac_params(&jsonInst, algoJsonId, pAlgo, AUDIO_ALGO_DYNAMIC_PARAM);
      iInstance++;
    }

    jsError = s_update_json_graph_connections(&jsonInst, root);
    jsError = s_update_json_graph_system_ios(&jsonInst, root);

    JSON_OBJECT_SET_INTEGER(jsError, &jsonInst, root, "", "InstanceNumber", (int64_t)iInstance);

    if (jsError == JSON_OK)
    {
      char *pJson = (char *)json_dumps(&jsonInst, root, dumpFlag);
      //char *pJson = (char *)json_dumps(&jsonInst, root, JSON_INDENT);

      if (pJson != NULL)
      {
        UTIL_TERM_puts(pJson);
        UTIL_TERM_puts("\n");
        json_free(pJson);
      }

      json_shutdown(&jsonInst);
    }
  }
}


#ifdef USE_ETHERNET
static void s_cmd_set_net_traces_state(int argc, char *argv[])
{
  bool enable;

  if (s_check_args_boolean(argc, argv, &enable))
  {
    net_set_trace_status(enable);
  }
}


static uint32_t s_get_net_trace_status_cb(void)
{
  return (uint32_t)net_get_trace_status();
}
#endif

static void s_cmd_get_generic_info(int argc, char *argv[])
{
  get_infos();
}


//static void s_cmd_toggle_usb_rec(int argc, char *argv[])
//{
//  toggle_usb_rec();
//}
//
//__weak void  toggle_usb_rec(void)
//{
//}


__weak void  get_infos(void)
{
}


#if defined(AUDIO_CHAIN_ACSDK_USED) || defined(AUDIO_CHAIN_CONF_TUNING_CLI_USED)
static jsonErr s_update_json_from_ac_params(json_pack_t *const pInstance, jsonID const algoJsonId, audio_algo_t *const pAlgo, audio_algo_tuning_stat_dyn_param_t const statDynParam)
{
  jsonErr jsError         = JSON_OK;
  jsonID  paramTypeJsonId = JSON_ID_NULL;
  uint8_t nbParams        = 0U;

  JSON_CREATE_ARRAY(jsError, pInstance, &paramTypeJsonId);
  JSON_OBJECT_SET_NEW(jsError, pInstance, algoJsonId, s_pStrJsonParamType[statDynParam], paramTypeJsonId);

  if (jsError == JSON_OK)
  {
    int32_t error = AudioAlgoTuning_getParamsNb(pAlgo, statDynParam, &nbParams, NULL);

    if (AudioError_isError(error))
    {
      UTIL_TERM_printf("%s, %s(), line %d: AudioAlgoTuning_getParamsNb error %d  %d \"%s\" \"%s\"\n", __FILE__, __FUNCTION__, __LINE__, error);
      jsError = JSON_ERROR;
    }
  }
  for (uint8_t paramId = 0U; (jsError == JSON_OK) && (paramId < nbParams); paramId++)
  {
    jsError = s_update_json_from_ac_param(pInstance, paramTypeJsonId, pAlgo, statDynParam, paramId);
  }

  if (jsError != JSON_OK)
  {
    UTIL_TERM_printf("%s, %s(), line %d: error %d  %d \"%s\" \"%s\"\n", __FILE__, __FUNCTION__, __LINE__, jsError, algoJsonId, pAlgo->pName, s_pStrJsonParamType[(statDynParam)]);
  }
  return jsError;
}


static void s_cmd_get_all_algo_param_list(int argc, char *argv[])
{
  if (AudioChain_getAlgosList(&AudioChainInstance) == NULL)
  {
    UTIL_TERM_printf("No Audio Chain is instantiated\n");
  }
  for (audio_algo_list_t *pCurrent = AudioChain_getAlgosList(&AudioChainInstance); pCurrent != NULL; pCurrent = pCurrent->next)
  {
    char  string[4];
    char *newArgv[2];

    snprintf(string, sizeof(string), "%d", AudioAlgo_getIndex(pCurrent->pAlgo));
    newArgv[0] = argv[0];
    newArgv[1] = string;
    s_cmd_get_all_param_list(2, newArgv);
  }
}


static void s_cmd_get_all_param_list(int argc, char *argv[])
{
  s_cmd_get_static_param_list(argc, argv);
  s_cmd_get_dynamic_param_list(argc, argv);
}


static void s_cmd_get_static_param_list(int argc, char *argv[])
{
  s_args_get_param_list(argc, argv, AUDIO_ALGO_STATIC_PARAM);
}


static void s_cmd_get_dynamic_param_list(int argc, char *argv[])
{
  s_args_get_param_list(argc, argv, AUDIO_ALGO_DYNAMIC_PARAM);
}


static void s_cmd_get_static_param_val(int argc, char *argv[])
{
  s_args_get_param_val(argc, argv, AUDIO_ALGO_STATIC_PARAM);
}


static void s_cmd_get_dynamic_param_val(int argc, char *argv[])
{
  s_args_get_param_val(argc, argv, AUDIO_ALGO_DYNAMIC_PARAM);
}


static void s_cmd_set_static_param_val(int argc, char *argv[])
{
  s_args_set_param_val(argc, argv, AUDIO_ALGO_STATIC_PARAM);
}


static void s_cmd_set_dynamic_param_val(int argc, char *argv[])
{
  s_args_set_param_val(argc, argv, AUDIO_ALGO_DYNAMIC_PARAM);
}


static void s_cmd_apply_static_param_val(int argc, char *argv[])
{
  if (s_args_set_param_val(argc, argv, AUDIO_ALGO_STATIC_PARAM))
  {
    s_cmd_apply_cfg(argc - 2, argv);
  }
}


static void s_cmd_apply_dynamic_param_val(int argc, char *argv[])
{
  if (s_args_set_param_val(argc, argv, AUDIO_ALGO_DYNAMIC_PARAM))
  {
    s_cmd_apply_cfg(argc - 2, argv);
  }
}


static void s_args_get_param_list(int argc, char *argv[], audio_algo_tuning_stat_dyn_param_t const statDynParam)
{
  audio_algo_t *pAlgo;
  uint8_t       algoId;

  if (s_check_args_algoId(argc, argv, &pAlgo, &algoId))
  {
    // algo found
    char          const *const paramTypeString = s_pStrParamType[statDynParam];
    int32_t                    error;
    uint8_t                    nbParams;

    error = AudioAlgoTuning_getParamsNb(pAlgo, statDynParam, &nbParams, NULL);
    if (AudioError_isOk(error))
    {
      UTIL_TERM_printf("algo %d: %s %s, %d %s param%s\n", algoId, pAlgo->pName, pAlgo->pDesc, nbParams, paramTypeString, (nbParams > 1U) ? "s" : "");
      for (uint8_t paramId = 0U; paramId < nbParams; paramId++)
      {
        s_print_param_val(pAlgo, algoId, statDynParam, paramId);
      }
    }
    else
    {
      UTIL_TERM_printf("%s command: algo %d get number of %s param error %d\n", argv[0], algoId, paramTypeString, error);
    }
  }
}


static void s_args_get_param_val(int argc, char *argv[], audio_algo_tuning_stat_dyn_param_t const statDynParam)
{
  audio_algo_t *pAlgo;
  uint8_t       algoId, paramId;

  if (s_check_args_algoId_paramId(argc, argv, &pAlgo, &algoId, &paramId, statDynParam))
  {
    s_print_param_val(pAlgo, algoId, statDynParam, paramId);
  }
}


static int s_args_set_param_val(int argc, char *argv[], audio_algo_tuning_stat_dyn_param_t const statDynParam)
{
  audio_algo_t *pAlgo;
  uint8_t       algoId, paramId;
  int           ret = s_check_args_algoId_paramId(argc - 1, argv, &pAlgo, &algoId, &paramId, statDynParam);

  if (ret)
  {
    s_set_param_val(pAlgo, algoId, statDynParam, paramId, argv[3]);
  }

  return ret;
}


static void s_cmd_apply_cfg(int argc, char *argv[])
{
  audio_algo_t *pAlgo;
  uint8_t       algoId;

  if (s_check_args_algoId(argc, argv, &pAlgo, &algoId))
  {
    // algo found
    int32_t error;

    error = AudioAlgo_requestTuningUpdate(pAlgo);
    if (AudioError_isError(error))
    {
      UTIL_TERM_printf("ERROR %s: AudioAlgo_requestTuningUpdate failed; error = %d\n", __FUNCTION__, error);
    }
  }
}


static void s_print_param_val(audio_algo_t *const pAlgo, uint8_t const algoId, audio_algo_tuning_stat_dyn_param_t const statDynParam, uint8_t const paramId)
{
  char valString[AC_PARAM_NAME_LENGTH_MAX];

  if (AudioError_isOk(s_getParamValStrWithName(pAlgo, algoId, statDynParam, paramId, valString, AC_PARAM_NAME_LENGTH_MAX)))
  {
    UTIL_TERM_printf("  %2d: %s\n", paramId, valString);
  }
}


static void s_set_param_val(audio_algo_t *const pAlgo, uint8_t const algoId, audio_algo_tuning_stat_dyn_param_t const statDynParam, uint8_t const paramId, char *const pStrParamVal)
{
  char valString[AC_PARAM_NAME_LENGTH_MAX];

  if (AudioError_isOk(s_getParamValStrWithName(pAlgo, algoId, statDynParam, paramId, valString, AC_PARAM_NAME_LENGTH_MAX)))
  {
    int32_t                    error;
    char                      *pErrorString;

    error = AudioAlgoTuning_setParamValStr(pAlgo, statDynParam, paramId, pStrParamVal, &pErrorString);
    if (AudioError_isOk(error))
    {
      UTIL_TERM_printf("%s -> %s\n", valString, pStrParamVal);
    }
    else
    {
      UTIL_TERM_printf("algo %d, %s param %d: %s\n", algoId, s_pStrParamType[statDynParam], paramId, pErrorString);
    }
  }
}


static int32_t s_getParamValStrWithName(audio_algo_t *const pAlgo, uint8_t const algoId, audio_algo_tuning_stat_dyn_param_t const statDynParam, uint8_t const paramId, char *const pString, size_t const size)
{
  int32_t                    error;
  char                      *pErrorString;
  uint8_t                    nbParams;

  error = AudioAlgoTuning_getParamsNb(pAlgo, statDynParam, &nbParams, NULL);
  if (AudioError_isOk(error))
  {
    if (nbParams == 0U)
    {
      UTIL_TERM_printf("algo %d has no %s param\n", algoId, s_pStrParamType[statDynParam]);
      error = AUDIO_ERR_MGNT_ERROR;
    }
  }
  if (AudioError_isOk(error))
  {
    error = AudioAlgoTuning_getParamValStrWithName(pAlgo, statDynParam, paramId, pString, size, &pErrorString);
    if (AudioError_isError(error))
    {
      UTIL_TERM_printf("algo %d, %s param %d: %s\n", algoId, s_pStrParamType[statDynParam], paramId, pErrorString);
    }
  }

  return error;
}


static int s_check_args_algoId_paramId(int argc, char *argv[], audio_algo_t **const ppAlgo, uint8_t *const pAlgoId, uint8_t *const pParamId, audio_algo_tuning_stat_dyn_param_t const statDynParam)
{
  int ret = s_check_args_algoId(argc - 1, argv, ppAlgo, pAlgoId);

  if (ret)
  {
    if (AudioAlgoTuning_isUnsignedDecimal(argv[2]))
    {
      uint8_t paramId;
      uint8_t nbParams;

      errno = 0;
      paramId = (uint8_t)strtoul(argv[2], NULL, 10);
      if (errno != 0)
      {
        UTIL_TERM_printf("%s command: algo id %s %s param id %s, strtoul error\n", argv[0], argv[1], s_pStrParamType[statDynParam], argv[2]);
        ret = 0;
      }
      else if (AudioError_isOk(AudioAlgoTuning_getParamsNb(*ppAlgo, statDynParam, &nbParams, NULL)))
      {
        if (paramId < nbParams)
        {
          *pParamId = paramId;
        }
        else
        {
          UTIL_TERM_printf("%s command: algo id %s %s param id %s too big (>= %d)\n", argv[0], argv[1], s_pStrParamType[statDynParam], argv[2], nbParams);
          ret = 0;
        }
      }
      else
      {
        UTIL_TERM_printf("%s command: can't get algo id %s %s params number\n", argv[0], argv[1], s_pStrParamType[statDynParam]);
        ret = 0;
      }
    }
    else
    {
      UTIL_TERM_printf("%s command: param id %s is not an unsigned integer\n", argv[0], argv[2]);
      ret = 0;
    }
  }

  return ret;
}


static int s_check_args_algoId(int argc, char *argv[], audio_algo_t **const ppAlgo, uint8_t *const pAlgoId)
{
  int ret = 1;

  if (argc == 2)
  {
    if (AudioAlgoTuning_isUnsignedDecimal(argv[1]))
    {
      uint8_t algoId;

      errno = 0;
      algoId = (uint8_t)strtoul(argv[1], NULL, 10);
      if (errno != 0)
      {
        UTIL_TERM_printf("%s command: algo id %, strtoul error\n", argv[0], argv[1]);
        ret = 0;
      }
      else
      {
        audio_algo_t *const pAlgo  = AudioChain_getAlgoFromId(&AudioChainInstance, algoId);

        if (pAlgo == NULL)
        {
          // algo not found
          UTIL_TERM_printf("algo %s not found\n", argv[1]);
          ret = 0;
        }
        else
        {
          *ppAlgo  = pAlgo;
          *pAlgoId = algoId;
        }
      }
    }
    else
    {
      UTIL_TERM_printf("%s command: algo id %s is not an unsigned integer\n", argv[0], argv[1]);
      ret = 0;
    }
  }
  else
  {
    UTIL_TERM_printf("%s command: wrong number of arguments\n", argv[0]);
    ret = 0;
  }

  return ret;
}


/**
* @brief prints the configuration json schema for a parameter
*
* @param pInstance        json instance
* @param paramTypeJsonId  json root id
* @param pAlgo            algo pointer
* @param statDynParam     static or dynamic parameters
* @param paramId          parameter id
*/
static jsonErr s_update_json_from_ac_param(json_pack_t *const pInstance, jsonID const paramTypeJsonId, audio_algo_t *const pAlgo, audio_algo_tuning_stat_dyn_param_t const statDynParam, uint8_t const paramId)
{
  static const char *const tParamTypeString[AUDIO_ALGO_NB_PARAM_TYPES] =
  {
    [AUDIO_DESC_PARAM_TYPE_INT8]   = "PARAM_INT8",
    [AUDIO_DESC_PARAM_TYPE_UINT8]  = "PARAM_UINT8",
    [AUDIO_DESC_PARAM_TYPE_INT16]  = "PARAM_INT16",
    [AUDIO_DESC_PARAM_TYPE_UINT16] = "PARAM_UINT16",
    [AUDIO_DESC_PARAM_TYPE_INT32]  = "PARAM_INT32",
    [AUDIO_DESC_PARAM_TYPE_UINT32] = "PARAM_UINT32",
    [AUDIO_DESC_PARAM_TYPE_FLOAT]  = "PARAM_FLOAT"
  };
  int32_t                          error       = AUDIO_ERR_MGNT_NONE;
  jsonErr                          jsError     = JSON_OK;
  audio_descriptor_params_t const *pParamsDesc = NULL;
  audio_descriptor_param_t        *pParam      = NULL;
  jsonID                           paramJsonId = 0U;

  error = AudioAlgoTuning_getParamsDescr(pAlgo, statDynParam, &pParamsDesc, NULL);
  if (AudioError_isError(error))
  {
    UTIL_TERM_printf("%s, %s(), line %d: error AudioAlgoTuning_getParamsDescr %d \n", __FILE__, __FUNCTION__, __LINE__, error);
    jsError = JSON_ERROR;
  }
  else
  {
    if (pParamsDesc != NULL)
    {
      pParam = &pParamsDesc->pParam[paramId];
    }
  }
  if ((pParam != NULL) && (pParam->iParamFlag != AUDIO_DESC_PARAM_TYPE_FLAG_PRIVATE))
  {
    JSON_CREATE_OBJECT(jsError,     pInstance,                  &paramJsonId);
    JSON_ARRAY_APPEND_NEW(jsError,  pInstance, paramTypeJsonId, paramJsonId);
    JSON_OBJECT_SET_STRING(jsError, pInstance,                  paramJsonId, "", "Title",       pParam->pName);
    JSON_OBJECT_SET_STRING(jsError, pInstance,                  paramJsonId, "", "Description", pParam->pName);
    JSON_OBJECT_SET_STRING(jsError, pInstance,                  paramJsonId, "", "ParamType",   tParamTypeString[pParam->paramType]);
    JSON_OBJECT_SET_STRING(jsError, pInstance,                  paramJsonId, "", "ParamDesc",   "Description");

    if (jsError == JSON_OK)
    {
      audio_descriptor_type_union_t paramVal, paramMin, paramMax;
      char                   *pErrorString;

      error = AudioAlgoTuning_getParamVal(pAlgo, statDynParam, paramId, &paramVal, &pErrorString);
      if (AudioError_isError(error))
      {
        UTIL_TERM_printf("%s, %s(), line %d: error AudioAlgoTuning_getParamVal %s \n", __FILE__, __FUNCTION__, __LINE__, pErrorString);
        jsError = JSON_ERROR;
      }
      if (AudioError_isOk(error))
      {
        error = AudioAlgoTuning_getParamMin(pAlgo, statDynParam, paramId, &paramMin, &pErrorString);
        if (AudioError_isError(error))
        {
          UTIL_TERM_printf("%s, %s(), line %d: error AudioAlgoTuning_getParamMin %s \n", __FILE__, __FUNCTION__, __LINE__, pErrorString);
          jsError = JSON_ERROR;
        }
      }
      if (AudioError_isOk(error))
      {
        error = AudioAlgoTuning_getParamMax(pAlgo, statDynParam, paramId, &paramMax, &pErrorString);
        if (AudioError_isError(error))
        {
          UTIL_TERM_printf("%s, %s(), line %d: error AudioAlgoTuning_getParamMax %s \n", __FILE__, __FUNCTION__, __LINE__, pErrorString);
          jsError = JSON_ERROR;
        }
      }
      if (jsError == JSON_OK)
      {
        json_numData_t json_numData;

        switch (pParam->paramType)
        {
          case AUDIO_DESC_PARAM_TYPE_INT8:
            json_numData.dataType = JSON_SIGNED64;
            json_numData.val.s64 = (int64_t)paramVal.s8;
            json_numData.min.s64 = (int64_t)paramMin.s8;
            json_numData.max.s64 = (int64_t)paramMax.s8;
            break;
          case AUDIO_DESC_PARAM_TYPE_INT16:
            json_numData.dataType = JSON_SIGNED64;
            json_numData.val.s64 = (int64_t)paramVal.s16;
            json_numData.min.s64 = (int64_t)paramMin.s16;
            json_numData.max.s64 = (int64_t)paramMax.s16;
            break;
          case AUDIO_DESC_PARAM_TYPE_INT32:
            json_numData.dataType = JSON_SIGNED64;
            json_numData.val.s64 = (int64_t)paramVal.s32;
            json_numData.min.s64 = (int64_t)paramMin.s32;
            json_numData.max.s64 = (int64_t)paramMax.s32;
            break;
          case AUDIO_DESC_PARAM_TYPE_UINT8:
            json_numData.dataType = JSON_UNSIGNED64;
            json_numData.val.u64 = (uint64_t)paramVal.u8;
            json_numData.min.u64 = (uint64_t)paramMin.u8;
            json_numData.max.u64 = (uint64_t)paramMax.u8;
            break;
          case AUDIO_DESC_PARAM_TYPE_UINT16:
            json_numData.dataType = JSON_UNSIGNED64;
            json_numData.val.u64 = (uint64_t)paramVal.u16;
            json_numData.min.u64 = (uint64_t)paramMin.u16;
            json_numData.max.u64 = (uint64_t)paramMax.u16;
            break;
          case AUDIO_DESC_PARAM_TYPE_UINT32:
            json_numData.dataType = JSON_UNSIGNED64;
            json_numData.val.u64 = (uint64_t)paramVal.u32;
            json_numData.min.u64 = (uint64_t)paramMin.u32;
            json_numData.max.u64 = (uint64_t)paramMax.u32;
            break;
          case AUDIO_DESC_PARAM_TYPE_FLOAT:
            json_numData.dataType = JSON_DOUBLE;
            json_numData.val.dbl = (double)paramVal.f;
            json_numData.min.dbl = (double)paramMin.f;
            json_numData.max.dbl = (double)paramMax.f;
            break;
          default:
            /* Do Nothing MISRA */
            break;
        }
        JSON_OBJECT_SET_NUM_DATA(jsError, pInstance, paramJsonId, "", "Value",   &json_numData);
        /* copy min value in param's value because json_object_set_num_data creates json structure from param's value */
        json_numData.val = json_numData.min;
        JSON_OBJECT_SET_NUM_DATA(jsError, pInstance, paramJsonId, "", "Minimum", &json_numData);
        /* copy max value in param's value because json_object_set_num_data creates json structure from param's value */
        json_numData.val = json_numData.max;
        JSON_OBJECT_SET_NUM_DATA(jsError, pInstance, paramJsonId, "", "Maximum", &json_numData);
      }
    }
  }
  if (jsError != JSON_OK)
  {
    UTIL_TERM_printf("%s, %s(), line %d: error %d %d %d \"%s\" \"%s\"\n", __FILE__, __FUNCTION__, __LINE__, jsError, paramTypeJsonId, paramId, pAlgo->pName, s_pStrJsonParamType[(statDynParam)]);
  }
  return jsError;
}


/**
* @brief Re-config from a json
*
* @param argc  num args
* @param argv  args list
*/
static void s_cmd_set_cfg(int argc, char *argv[])
{
  static UTIL_TERM_CMD_t gContext;
  bool                   bQuiet = false;
  bool                   bOk    = false;

  if (argc == 1)
  {
    bOk = true;
  }
  else if (argc == 2)
  {
    /* the quiet mode remove the help text before to drag and drop the file.cfg in the terminal */
    if (strcmp(argv[1], "quiet") == 0)
    {
      bQuiet = true;
      bOk    = true;
    }
  }
  else
  {
    UTIL_TERM_printf("%s command: wrong number of arguments\n", argv[0]);
  }

  if (bOk)
  {
    int32_t  error   = UTIL_ERROR_NONE;
    uint32_t syncTmo = 60UL;
    uint32_t szCurTuningParams;

    if (!bQuiet)
    {
      UTIL_TERM_printf("Audio Chain Live Tuning\n");
      UTIL_TERM_printf("When the board is ready, the char 'C' is printed repeatedly during %d secs before the timeout.\n", syncTmo);
      UTIL_TERM_printf("Before the timeout, send a the *.json file or just paste its content from the terminal. For example, using teraterm (alt+v)\n");
    }

    error = UTIL_TERM_receive_raw_file((uint8_t *)gContext.pJsonBuff, UTIL_TERM_CMD_JSON_BUFF_SIZE, syncTmo * 1000UL, &szCurTuningParams);
    if (error == UTIL_ERROR_NONE)
    {
      UTIL_TERM_printf("\n");
    }

    if (error == UTIL_ERROR_NONE)
    {
      jsonErr     jsError         = JSON_OK;
      json_pack_t jsonInst        = {0};
      uint8_t     iInstanceCount  = AudioChain_getNbAlgos(&AudioChainInstance);
      jsonID      algosListJsonId = JSON_ID_NULL;
      uint16_t    algosNb         = 0U;
      uint64_t    nbInstance      = 0ULL;
      const char *pAudioChainName = NULL;
      const char *pAudioChainDesc = NULL;

      /* the text is in the buffer */
      UTIL_TERM_printf("Data received: %d bytes\n", szCurTuningParams);
      gContext.pJsonBuff[szCurTuningParams] = '\0';
      UTIL_TERM_puts(gContext.pJsonBuff);
      UTIL_TERM_puts("\n");

      /* let's start the json parsing */
      memset(&jsonInst, 0, sizeof(jsonInst));

      JSON_LOAD(jsError, &jsonInst, (const char *)gContext.pJsonBuff, &jsonInst.pack_root);
      JSON_OBJECT_GET_UNSIGNED_INTEGER(jsError, &jsonInst, jsonInst.pack_root, "", "InstanceNumber", &nbInstance);
      if (jsError == JSON_OK)
      {
        /* check instance number consistency */
        if ((uint8_t)nbInstance != iInstanceCount)
        {
          UTIL_TERM_printf("Configuration file not compatible\n");
          jsError = JSON_ERROR;
        }
      }
      /* parse the header title file */
      JSON_OBJECT_GET_STRING(jsError, &jsonInst, jsonInst.pack_root, "", "Title", &pAudioChainName);

      if (jsError == JSON_OK)
      {
        if (pAudioChainName != NULL)
        {
          UTIL_TERM_printf("Config Title       : %s\n", pAudioChainName);
        }
        JSON_OBJECT_GET_STRING(jsError, &jsonInst, jsonInst.pack_root, "", "Description", &pAudioChainDesc);
      }

      if (jsError == JSON_OK)
      {
        if (pAudioChainDesc != NULL)
        {
          UTIL_TERM_printf("Config Description : %s\n", pAudioChainDesc);
        }

        /* Iteration for all each instances */
        JSON_ARRAY_GET(jsError, JSON_OK, &jsonInst, jsonInst.pack_root, "Algos", &algosListJsonId);
      }

      JSON_LIST_GET_COUNT(jsError, &jsonInst, algosListJsonId, &algosNb);
      for (uint16_t algoId = 0U; (jsError == JSON_OK) && (algoId < algosNb); algoId++)
      {
        jsonID algoJsonId;
        if (json_list_pair(&jsonInst, algosListJsonId, algoId, NULL, &algoJsonId) == JSON_OK) /* do not manage error: it can be != JSON_OK even if there is no issue */
        {
          /* retrieve basic title infos */
          const char *pAlgoName = NULL;
          const char *pAlgoDesc = NULL;
          uint8_t     bDisabled = 0U;
          uint64_t    iInstance = 0ULL;

          JSON_OBJECT_GET_STRING(jsError,           &jsonInst, algoJsonId, "", "Title",       &pAlgoName);
          JSON_OBJECT_GET_STRING(jsError,           &jsonInst, algoJsonId, "", "Description", &pAlgoDesc);
          JSON_OBJECT_GET_BOOLEAN(jsError,          &jsonInst, algoJsonId, "", "Disabled",    &bDisabled);
          JSON_OBJECT_GET_UNSIGNED_INTEGER(jsError, &jsonInst, algoJsonId, "", "InstanceNum", &iInstance);
          if (jsError == JSON_OK)
          {
            if (iInstance >= nbInstance)
            {
              UTIL_TERM_printf("Wrong instance number %d>=%d\n", iInstance, nbInstance);
              jsError = JSON_ERROR;
            }
          }
          if (jsError == JSON_OK)
          {
            audio_algo_t *pAlgo = AudioChain_getAlgoFromId(&AudioChainInstance, (uint8_t)iInstance);

            if (pAlgo != NULL)
            {
              /* check the file coherency */
              if (strcmp(pAlgo->pName, pAlgoName) != 0)
              {
                pAlgo = NULL;
              }
            }

            /* Everything went fine, now update all parameters of the audio chain's algorithm */
            if (pAlgo != NULL)
            {
              /* Update visibility */
              //audio_algo_state_t algoState = AudioAlgo_getState(pAlgo);
              //if (bDisabled)
              //{
              //  algoState &= ~AUDIO_ALGO_TUNING_DISABLED;
              //}
              //else
              //{
              //  algoState |= AUDIO_ALGO_TUNING_DISABLED;
              //}
              //AudioAlgo_setState(pAlgo, algoState);
              UTIL_TERM_printf("\tParsing algo : %s:%s\n", (pAlgoName != NULL) ? pAlgoName : "none", (pAlgoDesc != NULL) ? pAlgoDesc : "none");

              /* update static & dynamic params (algo is disabled during this update) */
              jsError = s_update_ac_params_from_json(&jsonInst, algoJsonId, pAlgo, AUDIO_ALGO_STATIC_PARAM);
              jsError = s_update_ac_params_from_json(&jsonInst, algoJsonId, pAlgo, AUDIO_ALGO_DYNAMIC_PARAM);

              /* apply new algo's config */
              if (jsError == JSON_OK)
              {
                error = AudioAlgo_requestTuningUpdate(pAlgo);
                if (AudioError_isError(error))
                {
                  UTIL_TERM_printf("ERROR %s: AudioAlgo_requestTuningUpdate failed; error = %d\n", __FUNCTION__, error);
                  jsError = JSON_ERROR;
                }
              }
            }
            else
            {
              UTIL_TERM_printf("\talgo : %s not found\n", (pAlgoName != NULL) ? pAlgoName : "none");
            }
          }
        }
      }

      /* cleanup */
      if (jsonInst.pack_root == JSON_ID_NULL)
      {
        UTIL_TERM_printf("\tJson parse error:\n");
        UTIL_TERM_puts(gContext.pJsonBuff);
      }
      json_shutdown(&jsonInst);
    }
    else
    {
      UTIL_TERM_printf("\tJson parsing error\n");
    }
    //  if (error != UTIL_ERROR_ALLOC)
    //  {
    //    UTIL_TERM_Free(pTuningParams);
    //  }
  }
}


/**
* @brief Parse json parameters & update audio chain
*
* @param pInstance  json instance
* @param jsonID     algo json id handle
* @param pParams    algo params
*/
static jsonErr s_update_ac_params_from_json(json_pack_t *const pInstance, jsonID const algoJsonId, audio_algo_t *const pAlgo, audio_algo_tuning_stat_dyn_param_t const statDynParam)
{
  jsonErr  jsError         = JSON_OK;
  jsonID   paramTypeJsonId = JSON_ID_NULL;
  uint16_t paramNb         = 0U;

  JSON_ARRAY_GET(jsError, JSON_ERR_FOUND, pInstance, algoJsonId, s_pStrJsonParamType[statDynParam], &paramTypeJsonId);
  if (jsError == JSON_ERR_FOUND)
  {
    jsError = JSON_OK;
  }
  else
  {
    JSON_LIST_GET_COUNT(jsError, pInstance, paramTypeJsonId, &paramNb);

    for (uint8_t paramId = 0U; (jsError == JSON_OK) && (paramId < paramNb); paramId++)
    {
      jsError = s_update_ac_param_from_json(pInstance, paramTypeJsonId, pAlgo, statDynParam, paramId);
    }
  }
  if (jsError != JSON_OK)
  {
    UTIL_TERM_printf("%s, %s(), line %d: error %d %d \"%s\" \"%s\"\n", __FILE__, __FUNCTION__, __LINE__, jsError, algoJsonId, pAlgo->pName, s_pStrJsonParamType[(statDynParam)]);
  }
  return jsError;
}


/**
* @brief Parse json parameters & update audio chain
*
* @param pInstance  json instance
* @param jsonID     algo json id handle
* @param pParams    algo params
*/
static jsonErr s_update_ac_param_from_json(json_pack_t *const pInstance, jsonID const paramTypeJsonId, audio_algo_t *const pAlgo, audio_algo_tuning_stat_dyn_param_t const statDynParam, uint8_t const paramId)
{
  jsonErr                           jsError      = JSON_OK;
  int32_t                           error        = AUDIO_ERR_MGNT_NONE;
  char                             *pErrorString = NULL;
  jsonID                            paramJsonId  = JSON_ID_NULL;
  audio_descriptor_params_t const  *pParamsDesc  = NULL;
  const char                       *pParamName   = NULL;
  char                             *pStrParamVal = NULL;

  error = AudioAlgoTuning_getParamsDescr(pAlgo, statDynParam, &pParamsDesc, NULL);
  if (AudioError_isError(error))
  {
    UTIL_TERM_printf("%s, %s(), line %d: AudioAlgoTuning_getParamsDescr error %d\n", __FILE__, __FUNCTION__, __LINE__, error);
    jsError = JSON_ERROR;
  }
  if ((pParamsDesc != NULL) && (pParamsDesc->pParam[paramId].iParamFlag != AUDIO_DESC_PARAM_TYPE_FLAG_PRIVATE))
  {
    JSON_LIST_PAIR(jsError, pInstance, paramTypeJsonId, (uint16_t)paramId, NULL, &paramJsonId);
    JSON_OBJECT_GET_STRING(jsError, pInstance, paramJsonId, "", "Title", &pParamName);
    if (jsError == JSON_OK)
    {
      if (strcmp(pParamName, pParamsDesc->pParam[paramId].pName) != 0)
      {
        UTIL_TERM_printf("%s, %s(), line %d: param name issue (\"%s\" != \"%s\")\n", __FILE__, __FUNCTION__, __LINE__, pParamName, pParamsDesc->pParam[paramId].pName);
        jsError = JSON_ERROR;
      }
    }

    /* Update values */
    JSON_OBJECT_GET_STRING(jsError, pInstance, paramJsonId, "", "Value", (const char **)&pStrParamVal);

    if (jsError == JSON_OK)
    {
      error = AudioAlgoTuning_setParamValStr(pAlgo, statDynParam, paramId, pStrParamVal, &pErrorString);
      if (AudioError_isError(error))
      {
        UTIL_TERM_printf("%s, %s(), line %d: AudioAlgoTuning_setParamValStr failed for %s param %s; error = %d %s\n", __FILE__, __FUNCTION__, __LINE__, s_pStrParamType[statDynParam], (pParamName != NULL) ? pParamName : "none", error, pErrorString);
        jsError = JSON_ERROR;
      }
    }

  }
  if (jsError != JSON_OK)
  {
    UTIL_TERM_printf("%s, %s(), line %d: error %d %d %d \"%s\" \"%s\"\n", __FILE__, __FUNCTION__, __LINE__, jsError, paramTypeJsonId, paramId, pAlgo->pName, s_pStrJsonParamType[(statDynParam)]);
  }
  return jsError;
}

#else // AUDIO_CHAIN_ACSDK_USED || AUDIO_CHAIN_CONF_TUNING_CLI_USED

static jsonErr s_update_json_from_ac_params(json_pack_t *const pInstance, jsonID const algoJsonId, audio_algo_t *const pAlgo, audio_algo_tuning_stat_dyn_param_t const statDynParam)
{
  return JSON_OK;
}

#endif // AUDIO_CHAIN_ACSDK_USED || AUDIO_CHAIN_CONF_TUNING_CLI_USED
