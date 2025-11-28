/**
******************************************************************************
* @file          livetune_audio_chain_hook.c
* @author        MCD Application Team
* @brief         Hook the ac standalone initialization
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
#include "livetune_audio_chain_hook.h"
#include "livetune.h"
#include "st_flash_storage.h"
#include "livetune_ac_wrapper.h"
#include "platform_setup_conf.h"
#include "st_flash_storage.h"

/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Global variables ----------------------------------------------------------*/
extern int32_t gAudio_Config;
extern st_flash_storage_type gCurrent_json;
/* Declaration      ----------------------------------------------------------*/


#ifdef BUTTON_WAKEUP
  #define RESCUE_BUTTON BUTTON_WAKEUP
#else
  #define RESCUE_BUTTON BUTTON_USER
#endif

__weak void livetune_hook_hw_conf(void)
{
}
__weak void platform_flash_ext_config(void)
{
}

/* Implementation init & startup */
/* Called from the init core MCU, allows to init extra HW according to the board  and use case */
void SystemHook_Init(void)
{
  /* Init the MPU partition for the flash ext and RAM ext */
  livetune_hook_hw_conf();

  /* Init the flash partition  */
  st_flash_storage_set_partition(st_flash_storage_REGISTRY, ST_FLASH_REGISTRY_BASE_ADDR, ST_FLASH_REGISTRY_BASE_SECTOR, ST_FLASH_REGISTRY_BASE_SECTOR_NB, ST_FLASH_REGISTRY_BASE_SECTOR_SIZE);
  st_flash_storage_set_partition(st_flash_storage_SAVE_JSON1, ST_FLASH_SAVE_JSON1_BASE_ADDR, ST_FLASH_SAVE_JSON1_BASE_SECTOR, ST_FLASH_SAVE_JSON1_BASE_SECTOR_NB, ST_FLASH_SAVE_JSON1_BASE_SECTOR_SIZE);
  st_flash_storage_set_partition(st_flash_storage_SAVE_JSON2, ST_FLASH_SAVE_JSON2_BASE_ADDR, ST_FLASH_SAVE_JSON2_BASE_SECTOR, ST_FLASH_SAVE_JSON2_BASE_SECTOR_NB, ST_FLASH_SAVE_JSON2_BASE_SECTOR_SIZE);
  st_flash_storage_set_partition(st_flash_storage_SAVE_JSON3, ST_FLASH_SAVE_JSON3_BASE_ADDR, ST_FLASH_SAVE_JSON3_BASE_SECTOR, ST_FLASH_SAVE_JSON3_BASE_SECTOR_NB, ST_FLASH_SAVE_JSON3_BASE_SECTOR_SIZE);

  /*
   Used different Json for each audio configuration : in/out has different sampling frequencies
  */

  /* Set the audio config before the real audio init */
  st_persist_sys *pSystem = (st_persist_sys *)ST_FLASH_REGISTRY_BASE_ADDR;
  if (pSystem->iSignature == ST_PERSIST_SIGNATURE)
  {
    audio_persist_set_config_index(gAudio_Config);

    if (gAudio_Config == audio_persist_get_config_index_from_name("media48kHz"))
    {
      gCurrent_json = st_flash_storage_SAVE_JSON1;
    }
    else if (gAudio_Config == audio_persist_get_config_index_from_name("media24kHz"))
    {
      gCurrent_json = st_flash_storage_SAVE_JSON2;
    }
    else if (gAudio_Config == audio_persist_get_config_index_from_name("telephony24kHz"))
    {
      gCurrent_json = st_flash_storage_SAVE_JSON3;
    }
    else
    {
      /* error, should not be reached  */
      while(1);
    }


  }
}

/**
  * @brief  similar to trace_print_args  Gives the opportunity to catch error coming from the system and react according to this
             mainly used in the designer to stop the pipe in case of error
  * @param  trace output = display and/or uart
  * @param  trace level
  * @param  Format string and parameters
  * @retval None
  */

bool trace_print_args_hook(traceOutput_t const output, traceLvl_t const level, const char *pFormat, va_list args)
{

  livetune_ac_check_system_error(level);
  return false;
}


static st_task taskLauncher;

/*
  Overload app reset to make sure we don't corrupt the registry
*/
void main_hooks_systemReset(void)
{
  livetune_reset();
}

/* task used only once to init the designer as soon as the kernel starts */
static void taskLauncher_cb(const void *pRefBuilder)
{
  /* Init the flash Ext */
  platform_flash_ext_config();

  ST_VERIFY(livetune_create() == ST_OK);
  st_os_task_exit(&taskLauncher);
}

/**
* @brief create the graph
*
*/
void AudioChainInstance_initGraph(void)
{
  /*
  In the context of this FW, we will use a cascaded DMA to push UART transmission without blocking. The initialization is done before the kernel start. so, the RTOS disable some interrupts when we create some RTOS objects before the kernel start. consequently, the cascade of DMA stall it is also the case for all IP using interrupts such as USB etc...
  To workaround this issue, the DMA cascade will start in a task launcher
  more info here:
  https://www.freertos.org/FreeRTOS_Support_Forum_Archive/February_2017/freertos_xTaskResumeAll_not_inside_a_task_488cec5bj.html
  */
  ST_VERIFY((ST_OS_Result)st_os_task_create(&taskLauncher, "taskLauncher", taskLauncher_cb, NULL, 2000UL, ST_Priority_Normal) == ST_OS_OK);
}


#endif