/**
******************************************************************************
* @file          plugin_terminal_cmd_common.c
* @author        MCD Application Team
* @brief         general purpose commands
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
#include "st_base.h"
#include "st_registry.h"
#include "livetune.h"
#include "plugin_terminal_cmd.h"
#include "livetune_db_json.h"
#include "livetune_ac_wrapper.h"
#include "string.h"
#include "st_os_monitor_cpu.h"


/* Global variables ----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/

typedef struct cfg_def_toolsbar
{
  const char *pTitle;
  const char *pID;
  const char *pCmd;
  const char *pType;
  uint32_t (*get_value_cb)(void);
} cfg_def_toolsbar;



/* Private functions ------------------------------------------------------- */
static void plugin_terminal_cmd_ac_flags(int argc, char_t *argv[]);
static void plugin_terminal_cmd_audio_config(int argc, char_t *argv[]);
static void plugin_terminal_cmd_flags_help(const char *pTab);
static void plugin_terminal_cmd_audio_config_help(const char *pTab);
static void plugin_terminal_cmd_chunk_pool_help(const char *pTab);
static void plugin_terminal_cmd_chunk_pool(int argc, char_t *argv[]);
static void plugin_terminal_cmd_algo_pool_help(const char *pTab);
static void plugin_terminal_cmd_algo_pool(int argc, char_t *argv[]);


/**
* @brief Flag name prefixed by "_" to avoid wrong Id parsed by JavaScript
*/

static const struct t_debug_flags
{
  const char_t *pName;
  uint32_t      msk;
} tListDbgFlg[] =
{
  {"error",     ST_TRACE_LVL_ERROR},
  {"warning",   ST_TRACE_LVL_WARNING},
  {"info",      ST_TRACE_LVL_INFO},
  {"verbose",   ST_TRACE_LVL_VERBOSE},
  {"debug",     ST_TRACE_LVL_DEBUG},
  {"json",      ST_TRACE_LVL_JSON},
  {"formatted", ST_TRACE_LVL_JSON_FORMATTED},
  {"colorize",  ST_TRACE_LVL_COLORIZE},
  {0, 0}
};

static const state_flags_t tListStateFlags[] =
{
  {"auto_start", ST_STATE_FLG_AUTO_START},
  {"heart",      ST_STATE_FLG_HEART},
  {0, 0}
};


/**
* @brief Command list accepted for the ac
*
*/
/* struct definition sub commands */
typedef struct plugin_terminal_cmd_ac_t
{
  char_t *pName;
  char_t *pDesc;
  void (*cmdCb)(int argc, char_t *argv[]);
  void (*cmdHelp)(const char *pTab);

} plugin_terminal_cmd_ac_t;


static const plugin_terminal_cmd_ac_t tListAcCmd[] =
{
  {"help",       "This help",                             NULL,                             NULL},
  {"set",        "Set audio chain flags",                 plugin_terminal_cmd_ac_flags,     plugin_terminal_cmd_flags_help},
  {"clear",      "Clear audio chain flags",               plugin_terminal_cmd_ac_flags,     plugin_terminal_cmd_flags_help},
  {"chunk_pool", "Select the chunk memory pool",          plugin_terminal_cmd_chunk_pool,   plugin_terminal_cmd_chunk_pool_help},
  {"algo_pool",  "Select the algo memory pool",           plugin_terminal_cmd_algo_pool,    plugin_terminal_cmd_algo_pool_help},
  {"audio",      "Select the audio configuration number", plugin_terminal_cmd_audio_config, plugin_terminal_cmd_audio_config_help},
  {NULL,         NULL,                                    NULL,                             NULL}
};


/**
* @brief return the list of ac flags
*/
__weak const struct t_state_flags *plugin_terminal_cmd_get_flags(void)
{
  return tListStateFlags;
}


/**
* @brief return the command description from a string
*
* @param pName command name
* @return const plugin_terminal_cmd_test_t* description pointer
*/

static const plugin_terminal_cmd_ac_t *plugin_terminal_get_ac_cmd(char_t *pName)
{
  const plugin_terminal_cmd_ac_t *pCmd = NULL;

  for (const plugin_terminal_cmd_ac_t *pItem = tListAcCmd; pItem->pName != NULL; pItem++)
  {
    if (strcmp(pItem->pName, pName) == 0)
    {
      pCmd = pItem;
      break;
    }
  }
  return pCmd;
}


/**
* @brief set the trace level
* @param pName  flag name
* @param state  true or false
*/
static void plugin_terminal_set_trace_value(const char_t *pName, bool state)
{
  for (const struct t_debug_flags *pList = tListDbgFlg; pList->pName != NULL; pList++)
  {
    if (strcmp(pList->pName, pName) == 0)
    {
      uint32_t gdblvl = st_base_get_debug_level();
      if (state)
      {
        gdblvl |= pList->msk;
      }
      else
      {
        gdblvl &= ~pList->msk;
      }
      st_persist_sys *pSystem = st_registry_lock_sys(&UTIL_TERM_get_instance()->hRegistry);
      if (pSystem)
      {
        pSystem->iLogLevel = gdblvl;
        st_base_set_debug_level(gdblvl);
        st_registry_unlock_sys(&UTIL_TERM_get_instance()->hRegistry, TRUE);
        break;
      }
    }
  }
}


/**
* @brief set the state level
* @param pName  flag name
* @param state  true or false
*/
static void plugin_terminal_set_state_value(const char_t *pName, bool state)
{
  for (const struct t_state_flags *pList = plugin_terminal_cmd_get_flags(); pList->pName != NULL; pList++)
  {
    if (strcmp(pList->pName, pName) == 0)
    {
      uint32_t stateFlg = livetune_state_get_flags();
      if (state)
      {
        stateFlg |= pList->msk;
      }
      else
      {
        stateFlg &= ~pList->msk;
      }
      livetune_state_set_flags(stateFlg);
      break;
    }
  }
}



/**

* @brief Change debug trace levels
*
* @param argc  num args
* @param argv  args list
*/

static void plugin_terminal_cmd_trace(int argc, char_t *argv[])
{
  if (argc < 2)
  {
    UTIL_TERM_printf_forced_cr("Error:Wrong Command parameters");
    return;
  }

  if (strcmp(argv[1], "help") == 0)
  {
    uint32_t i = 0;
    UTIL_TERM_printf_forced_cr("Debug trace level name:");
    uint32_t curFlag = st_base_get_debug_level();

    while (tListDbgFlg[i].pName != 0)
    {
      UTIL_TERM_printf_forced_cr("\t%-10s : %s",  tListDbgFlg[i].pName, ((curFlag & tListDbgFlg[i].msk) != 0U) ? "ON" : "OFF");
      i++;
    }
    return;
  }

  if (argc >= 3)
  {
    bool set   = (strcmp(argv[1], "set")   == 0);
    bool clear = (strcmp(argv[1], "clear") == 0);

    if (set || clear)
    {
      for (const struct t_debug_flags *pItem = tListDbgFlg; pItem->pName != NULL; pItem++)
      {
        if (strcasecmp(pItem->pName, argv[2]) == 0)
        {
          /* access to the registry could crash the MCU if used by an algo, so stop */
          plugin_terminal_cmd_stop_pipe();
          plugin_terminal_set_trace_value(pItem->pName, set);
          UTIL_TERM_printf_forced_cr("trace %s %s", pItem->pName, set ? "enabled" : "disabled");
          return;
        }
      }
    }
  }
  UTIL_TERM_printf_forced_cr("Syntax error");
}


/**

* @brief print ac flags
*
*/

static void plugin_terminal_cmd_flags_help(const char *pTab)
{
  uint32_t i = 0;
  UTIL_TERM_printf_forced_cr("%sState flag named:", pTab);
  uint32_t curFlag = livetune_state_get_flags();
  const struct t_state_flags *pFlags = plugin_terminal_cmd_get_flags();
  while (pFlags[i].pName != 0)
  {
    UTIL_TERM_printf_forced_cr("%s\t%-12s:%s", pTab, pFlags[i].pName, ((curFlag & pFlags[i].msk) != 0U) ? "ON" : "OFF");
    i++;
  }
}


/**

* @brief Command ac flags (set/clear)
*
* @param argc  num args
* @param argv  args list
*/
static void plugin_terminal_cmd_ac_flags(int argc, char_t *argv[])
{
  if (argc < 3)
  {
    plugin_terminal_cmd_flags_help("");
    return;
  }

  if (argc >= 3)
  {
    bool set   = (strcmp(argv[1], "set")   == 0);
    bool clear = (strcmp(argv[1], "clear") == 0);

    if (set || clear)
    {
      for (const struct t_state_flags *pItem = plugin_terminal_cmd_get_flags(); pItem->pName != NULL; pItem++)
      {
        if (strcasecmp(pItem->pName, argv[2]) == 0)
        {
          /* access to the registry could crash the MCU if used by an algo, so stop */
          plugin_terminal_cmd_stop_pipe();
          plugin_terminal_set_state_value(pItem->pName, set);
          UTIL_TERM_printf_forced_cr("state %s %s", pItem->pName, set ? "enabled" : "disabled");
          return;
        }
      }
    }
  }

  UTIL_TERM_printf_forced_cr("Syntax error");
}


static void plugin_terminal_cmd_audio_get_config_full_name(const audio_persist_config *const pConfig, char *const pConfigFullName, size_t const configFullNameSize)
{
  size_t configNameLength = strlen(pConfig->pConfigName);
  memset(pConfigFullName, (int)' ', configFullNameSize);
  memcpy(pConfigFullName, "Mic:", 4UL);
  memcpy(&pConfigFullName[4UL], pConfig->pConfigName, configNameLength);
  pConfigFullName[4UL + configNameLength] = ':';
  snprintf(&pConfigFullName[18UL],
           configFullNameSize - 18UL,
           "Out: %2dKHz %dCh In: %2dKHz %dCh %2dbit-%dMic Duration:%dms LowLatency:%d",
           pConfig->audioOutFreq / 1000UL,
           pConfig->audioOutCh,
           pConfig->audioInFreq / 1000UL,
           pConfig->audioInCh,
           pConfig->audioInResolution,
           pConfig->audioInHwCh,
           pConfig->audioMs,
           pConfig->audioChainLowLatency);
}


static void plugin_terminal_cmd_audio_config_help(const char *pTab)
{
  UTIL_TERM_printf_forced_cr("%sNum Audio Config:", pTab);
  int32_t nbConfig = audio_persist_get_config_number();
  for (int32_t index = 0; index < nbConfig; index++)
  {
    const audio_persist_config *const pConfig = audio_persist_get_config(index);
    char configFullName[100];
    char tUniqueID[30] = "";
    audio_persist_get_config_id(pConfig, tUniqueID, sizeof(tUniqueID));
    plugin_terminal_cmd_audio_get_config_full_name(pConfig, configFullName, sizeof(configFullName));
    UTIL_TERM_printf_forced_cr("%s\t%c%02d: %-10s: %s",
                               pTab,
                               (index == audio_persist_get_config_index()) ? '>' : ' ',
                               index,
                               configFullName,
                               tUniqueID);
  }
}


static void plugin_terminal_cmd_audio_config(int argc, char_t *argv[])
{
  if (argc <= 2)
  {
    plugin_terminal_cmd_audio_config_help("");
    return;
  }
  if (argc >= 2)
  {
    int32_t index = atoi(argv[2]);
    if (index < audio_persist_get_config_number())
    {
      st_persist_sys *pSystem = st_registry_lock_sys(&livetune_get_instance()->hRegistry);
      if (pSystem)
      {
        const audio_persist_config *const pConfig = audio_persist_get_config(index);
        char configFullName[100];
        /* access to the registry could crash the MCU if used by an algo, so stop */
        plugin_terminal_cmd_stop_pipe();
        pSystem->hUser.iAudioConfig = index;
        st_registry_unlock_sys(&livetune_get_instance()->hRegistry, TRUE);
        plugin_terminal_cmd_audio_get_config_full_name(pConfig, configFullName, sizeof(configFullName));
        UTIL_TERM_printf_forced_cr("Reboot with the config %d:%s", index, configFullName);
        livetune_reset();
      }
    }
  }
  UTIL_TERM_printf_forced_cr("Syntax error");
}


static void plugin_terminal_cmd_pool_help(const char *pTab, uint32_t const index)
{
  for (audio_descriptor_key_value_t const *pDesc = tRamTypeKeyValue; pDesc->pKey != NULL; pDesc++)
  {
    UTIL_TERM_printf_forced_cr("%s\t%c%d: %s", pTab, (index == pDesc->iValue) ? '>' : ' ', pDesc->iValue, pDesc->pKey);
  }
}


static void plugin_terminal_cmd_pool(int argc, char_t *argv[], uint32_t (*get_mem_pool)(void), uint32_t (*set_mem_pool)(uint8_t mem_pool))
{
  if (argc <= 2)
  {
    plugin_terminal_cmd_pool_help("", get_mem_pool());
    return;
  }
  else
  {
    uint32_t index = (uint32_t)atoi(argv[2]);
    for (audio_descriptor_key_value_t const *pDesc = tRamTypeKeyValue; pDesc->pKey != NULL; pDesc++)
    {
      if (index == pDesc->iValue)
      {
        /* access to the registry could crash the MCU if used by an algo, so stop */
        plugin_terminal_cmd_stop_pipe();
        set_mem_pool((uint8_t)index);
        UTIL_TERM_printf_forced_cr("set %s to %s", argv[1], pDesc->pKey);
        return;
      }
    }
  }
  UTIL_TERM_printf_forced_cr("Syntax error");
}


static void plugin_terminal_cmd_chunk_pool_help(const char *pTab)
{
  plugin_terminal_cmd_pool_help(pTab, livetune_state_get_chunk_pool());
}


static void plugin_terminal_cmd_algo_pool_help(const char *pTab)
{
  plugin_terminal_cmd_pool_help(pTab, livetune_state_get_algo_pool());
}


static void plugin_terminal_cmd_chunk_pool(int argc, char_t *argv[])
{
  plugin_terminal_cmd_pool(argc, argv, livetune_state_get_chunk_pool, livetune_state_set_chunk_pool);
}


static void plugin_terminal_cmd_algo_pool(int argc, char_t *argv[])
{
  plugin_terminal_cmd_pool(argc, argv, livetune_state_get_algo_pool, livetune_state_set_algo_pool);
}


/**

* @brief Change state levels
*
* @param argc  num args
* @param argv  args list
*/

static void plugin_terminal_cmd_ac(int argc, char_t *argv[])
{
  if (argc < 2)
  {
    UTIL_TERM_printf_forced_cr("Error: Wrong Command parameters");
    return;
  }

  if (strcmp(argv[1], "help") == 0)
  {
    UTIL_TERM_printf_forced_cr("ac command name:");
    for (const plugin_terminal_cmd_ac_t *pItem = tListAcCmd; pItem->pName != NULL; pItem++)
    {
      UTIL_TERM_printf_forced_cr("\t%-10s : %s", pItem->pName, pItem->pDesc);
      if (pItem->cmdHelp != NULL)
      {
        pItem->cmdHelp("                     ");
      }
    }
    return;
  }

  if (argc >= 2)
  {
    const plugin_terminal_cmd_ac_t *pCmd = plugin_terminal_get_ac_cmd(argv[1]);
    if ((pCmd != NULL) && (pCmd->cmdCb != NULL))
    {
      pCmd->cmdCb(argc, argv);
      return;
    }
  }

  UTIL_TERM_printf_forced_cr("Syntax error");
}


/**

* @brief Generate the algo list (used or available)
*
* @param argc  num args
* @param argv  args list
*/

static void plugin_terminal_cmd_generate_algo_list(int argc, char_t *argv[])
{
  UTIL_TERM_printf_forced_cr("Generate Algo list ...");
  uint32_t          bUsedOnly    = TRUE;
  uint32_t          bDone        = FALSE;
  livetune_db_list *pElementUsed = NULL;

  if ((argc == 2) && (strcmp(argv[1], "all") == 0))
  {
    bUsedOnly = FALSE;
    bDone     = TRUE;
  }
  if (argc == 1) // no args
  {
    bUsedOnly = TRUE;
    bDone     = TRUE;
  }
  if (bDone)
  {
    UTIL_TERM_printf_forced_cr("#include \"audio_chain_factory.h\"");
    UTIL_TERM_printf_forced_cr("");
    livetune_db *pDb = &UTIL_TERM_get_instance()->hDesignerDB;

    if (bUsedOnly)
    {
      UTIL_TERM_printf_forced_cr("/* generated by the terminal command \"algo_list\" */\n");

      /* select only algo used by the graph */
      uint32_t nbInstance = livetune_db_list_nb(&pDb->pInstances);
      for (uint32_t indexInstance = 0; indexInstance < nbInstance; indexInstance++)
      {
        livetune_db_instance *pInstance = livetune_db_list_get_object(&pDb->pInstances, indexInstance);
        bool                  found     = false;
        for (livetune_db_list *pList = pElementUsed; pList != NULL; pList = pList->pNext)
        {
          if (strcmp(pList->pObject, pInstance->pElements->pName) == 0)
          {
            found = true;
            break;
          }
        }
        if (!found)
        {
          livetune_helper_builder_def *pRef = (livetune_helper_builder_def *)pInstance->pElements->pRefBuilder;

          /* check if it is an algo factory */
          if ((pRef != NULL) && (pRef->pAlgoName != NULL))
          {
            if (livetune_ac_is_factory_algo(pRef ->pAlgoName))
            {
              livetune_db_list_create(&pElementUsed, (void *)pRef->pAlgoName);
            }
          }
        }
      }
    }
    else
    {
      UTIL_TERM_printf_forced_cr("/* generated by the terminal command \"algo_list all\" */\n");
      uint32_t nbElements = livetune_db_list_nb(&pDb->pElements);
      for (uint32_t indexElement = 0; indexElement < nbElements; indexElement++)
      {
        livetune_db_element *pElement = livetune_db_list_get_object(&pDb->pElements, indexElement);
        if (pElement->pRefBuilder)
        {
          const char_t *pAlgoName = ((livetune_helper_builder_def *)pElement->pRefBuilder)->pAlgoName;
          livetune_db_list_create(&pElementUsed, (void *)pAlgoName);
        }
      }
    }
    UTIL_TERM_printf_forced_cr("#ifdef ALGO_USE_LIST");
    int32_t indexElement = 0;
    for (livetune_db_list *pList = pElementUsed; pList != NULL; pList = pList->pNext)
    {
      const char_t *pAlgoName            = pList->pObject;
      const char_t *pFactoryInstanceName = livetune_ac_factory_get_factory_instance_name(pAlgoName);
      if (pFactoryInstanceName)
      {
        UTIL_TERM_printf_forced_cr("/* %02d %s */", indexElement, pAlgoName);
        UTIL_TERM_printf_forced_cr("extern audio_algo_factory_t %s;", pFactoryInstanceName);
        UTIL_TERM_printf_forced_cr("ALGO_FACTORY_DECLARE_FORCE(%s);", pFactoryInstanceName);
      }
      indexElement++;
    }
    UTIL_TERM_printf_forced_cr("#endif");
  }
  else
  {
    UTIL_TERM_printf_forced_cr("Syntax error");
  }

  livetune_db_list_clear(&pElementUsed, FALSE);
}



TERM_CMD_DECLARE("trace",     "[help|set|clear] [flag name]",  "Manage the trace flags",                                                                plugin_terminal_cmd_trace);
TERM_CMD_DECLARE("ac",        "[help|set|clear|audio] [help]", "Manage the Audio Chain designer behaviors.",                                            plugin_terminal_cmd_ac);
TERM_CMD_DECLARE("algo_list", "[all]",                         "Generate the algo list used when ALGO_USE_LIST is defined or the list of all elements", plugin_terminal_cmd_generate_algo_list);

#endif
