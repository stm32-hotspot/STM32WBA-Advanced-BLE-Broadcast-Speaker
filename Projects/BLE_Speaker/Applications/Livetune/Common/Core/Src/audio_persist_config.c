/**
******************************************************************************
* @file          audio_persist_config.h
* @author        MCD Application Team
* @brief         Manage the persistent audio configuration
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


/* Includes ------------------------------------------------------------------*/
#include "audio_persist_config.h"
#include "stm32_audio_conf.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include  "stm32xxx_voice_bsp.h" /* Needed for the define identifying the family
*                                   such as STM32N6, STM32H7, etc...
*                                   wiz cubeMx this define is removed from pre
*                                   processor define list thus we need to get it
*                                   from stm32n6xx.h, stm32h7xx.h, etc...
*                                   In our audio projects, these includes are
*                                   handled by stm32xxx_voice_bsp.h.
*/

/* Private defines -----------------------------------------------------------*/
/* globals         -----------------------------------------------------------*/

/* configuration list supported */



static int32_t gCurAudioConfig; /* current audio config */


/**
* @brief  return the current config struct
*
* @return the config struct
*/
const audio_persist_config *audio_persist_get_config_cur(void)
{
  return audio_persist_get_config(gCurAudioConfig);
}


/**
* @brief  return the config struct
*
* @param index config index
* @return the config struct
*/
__weak const audio_persist_config *audio_persist_get_config(int32_t index)
{
  return NULL;
}

/**
* @brief  return the number of config
*
* @return config number
*/

__weak int32_t audio_persist_get_config_number(void)
{
  return (int32_t)0;
}

/**
* @brief  return the current config index
*
* @return config number
*/
int32_t audio_persist_get_config_index(void)
{
  return gCurAudioConfig;
}

/**
* @brief  set  the current config index
*
* @param index config number
*/
void audio_persist_set_config_index(int32_t index)
{
  gCurAudioConfig = index;
}

/**
* @brief  generate the config unique id
*
* @param pConfig config pointer
* @param pBuffer unique id buffer string
* @param szBuffer unique id buffer size

*/

void  audio_persist_get_config_id(const audio_persist_config *pConfig, char *pBuffer, uint32_t szBuffer)
{
  snprintf(pBuffer, szBuffer, "ID_%02X%02d%02X%02d%c%c%c%02d%02X%02d%02X",
           pConfig->audioOutCh, pConfig->audioOutFreq / 1000UL,
           pConfig->audioInCh, pConfig->audioInFreq / 1000UL,
           (char)tolower((int)pConfig->pConfigName[0]), (char)tolower((int)pConfig->pConfigName[1]), (char)tolower((int)pConfig->pConfigName[2]),
           pConfig->audioMs,
           pConfig->audioChainLowLatency,
           pConfig->audioInResolution,
           0xFF); /* future ext */
}


/**
* @brief  return the index id from a unique string id
*
* @param pID  string id
*/

int32_t audio_persist_get_config_index_from_string(const char *pId)
{
  int32_t indexId = -1;
  int32_t nbId = audio_persist_get_config_number();
  int i;
  char modifiedId1[24];
  char modifiedId2[24];

  for (i = 0; (i < 23) && (pId[i] != '\0'); i++)
  {
    modifiedId1[i] = (char)tolower((int)pId[i]);
    modifiedId2[i] = modifiedId1[i];
  }
  modifiedId1[i] = '\0';
  modifiedId2[i] = '\0';

  #ifdef STM32N6
  // in case we are running on N6 platform and generated code was generated on another platform and used onboard or stval_mic008 microphones
  // we consider generated code with connected microphones
  if (((pId[11] == 'o') && (pId[12] == 'n') && (pId[13] == 'b')) || ((pId[11] == 's') && (pId[12] == 't') && (pId[13] == 'v')))
  {
    modifiedId1[11] = 'c';
    modifiedId1[12] = 'o';
    modifiedId1[13] = 'n';
  }
  #else
  // in case we are not running on N6 platform and generated code was generated N6 platform and used connected microphones
  // we consider generated code with onboard or stval_mic008 microphones
  if ((modifiedId1[11] == 'c') && (modifiedId1[12] == 'o') && (modifiedId1[13] == 'n'))
  {
    modifiedId1[11] = 'o';
    modifiedId1[12] = 'n';
    modifiedId1[13] = 'b';
    modifiedId2[11] = 's';
    modifiedId2[12] = 't';
    modifiedId2[13] = 'v';
  }
  #endif

  for (int32_t a = 0; a < nbId; a++)
  {
    const audio_persist_config *pConfig = audio_persist_get_config(a);
    if (pConfig)
    {
      char tScratch[24];

      audio_persist_get_config_id(pConfig, tScratch, sizeof(tScratch));

      // equivalent of stricmp which is not available in standard library
      for (i = 0; tScratch[i] != '\0'; i++)
      {
        tScratch[i] = (char)tolower((int)tScratch[i]);
      }
      if ((strcmp(tScratch, modifiedId1) == 0) || (strcmp(tScratch, modifiedId2) == 0))
      {
        // ok => config found
        indexId = a;
        break;
      }
    }
  }
  return indexId;
}


/**
* @brief  return the index id from a config name
*
* @param pName  name
*/
int32_t audio_persist_get_config_index_from_name(const char *pName)
{
  int32_t indexId = -1;
  int32_t nbId = audio_persist_get_config_number();

  for (int32_t a = 0; a < nbId; a++)
  {
    const audio_persist_config *pConfig = audio_persist_get_config(a);
    if (pConfig)
    {
      if (strcmp(pName, pConfig->pConfigName) == 0)
      {
        // ok => config found
        indexId = a;
        break;
      }
    }
  }
  return indexId;
}