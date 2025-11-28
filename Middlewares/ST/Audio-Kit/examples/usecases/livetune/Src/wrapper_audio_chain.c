/**
******************************************************************************
* @file    wrapper_audio_chain.c
* @author  MCD Application Team
* @brief   connects audioChain to main application.
******************************************************************************
* @attention
*
* Copyright (c) 2018(-2022) STMicroelectronics.
* All rights reserved.
*
* This software is licensed under terms that can be found in the LICENSE file
* in the root directory of this software component.
* If no LICENSE file comes with this software, it is provided AS-IS.
*
******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#if defined(USBD_AUDIO_OUT_USED) || defined(USBD_AUDIO_IN_USED)
  #include "usbd_audio_if.h"
#endif
//#include "stm32_audio.h"
#include "audio_chain_instance.h"
#include "audio_chain_sysIOs_conf.h"
#include "traces.h"
#include "main_hooks.h"
#include "stm32_audio_tasks.h"
//#include "stm32_term.h"

/* Global variables ----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void s_createTerminal(void);
/* Functions Definition ------------------------------------------------------*/
extern void AudioChainInstance_initSystem(void);
void main_hooks_audioInit(void)
{
  audio_chain_instance_params_t params =
  {
    .traceEnable             = true,
    .isDataInOutSpecificTask = true,
    .isProcessSpecificTask   = true,
    .isControlSpecificTask   = true,
    .logInit                 = false,
    .logCmsisOs              = UTIL_AUDIO_LOG_TASK_QUEUE_LEVELS
  };

  if (params.isProcessSpecificTask)
  {
    /* In case low latency mode is requested, process callbacks are no longer in dedicated task but are called right after the dataInOut callbacks */
    params.isProcessSpecificTask = (AC_LOW_LATENCY == 0U);
  }

  AudioChainInstance_initSystem();

  /* Creates audio_buffer_t structure that will feed AudioChain.
  */
  AudioChainSysIOs_addIn("SysIn-BLE-LC3",     "From BLE LC3",       UTIL_AUDIO_CAPTURE_getAudioBuffer(MAIN_PATH),      (uint8_t)AC_SYSIN_BLE_LC3,    NULL);
  AudioChainSysIOs_addOut("SysOut-Codec-DAC", "Towards codec DAC",  UTIL_AUDIO_RENDER_getAudioBuffer(MAIN_PATH),       (uint8_t)AC_SYSOUT_CODEC_DAC, NULL);

  AudioChainSysIOs_addOut("SysOut-BLE-LC3",   "Towards BLE LC3",    UTIL_AUDIO_RENDER_getAudioBuffer(ALTERNATE_PATH),  (uint8_t)AC_SYSOUT_BLE_LC3,   NULL);

#ifdef USE_STM32WBA65I_DK1
  AudioChainSysIOs_addIn("SysIn-Codec-ADC",   "From codec ADC",     UTIL_AUDIO_CAPTURE_getAudioBuffer(ALTERNATE_PATH), (uint8_t)AC_SYSIN_CODEC_ADC,  NULL);
#endif

#ifdef USE_BLE_SPEAKER
  AudioChainSysIOs_addIn("SysIn-MicroPDM",    "From PDM Micro",     UTIL_AUDIO_CAPTURE_getAudioBuffer(ALTERNATE_PATH), (uint8_t)AC_SYSIN_PDM,        NULL);
  AudioChainSysIOs_addIn("SysIn-WAVFILE",     "From Flash WAV file",UTIL_AUDIO_WAVFILE_CAPTURE_getAudioBuffer(),       (uint8_t)AC_SYSIN_WAVFILE,    NULL);
#endif /* USE_BLE_SPEAKER */

  AudioChainInstance_init(&params);
  AudioChainInstance_initGraph();
  AudioChainInstance_initTuning();

#ifdef USE_BLE_SPEAKER
  //start audio now
  UTIL_AUDIO_start();
#endif /* USE_BLE_SPEAKER */

  /* In case LiveTune is used on top of AudioChain, a terminal is necessary */
  s_createTerminal();
}

extern void AudioChainInstance_reset_states(void);
void main_hooks_audioDeInit(void)
{
  /* This function is not called but provided as an example for a clean termination of audio services */
  if (AudioChainInstance_isStarted())
  {
    AudioChainInstance_deinitTuning();
    AudioChainInstance_deinitGraph();
    AudioChainInstance_deinit();

    AudioChainInstance_reset_states();
  }

  /* If AudioChain needs to be stopped: call the following after testing if (AudioChainInstance_isStarted())
  **   - trace_setAsynchronous(false); ==> will force synchronous trace to be sure to get all traces
  **   - AudioChainInstance_deinitTuning();
  **   - AudioChainInstance_deinitGraph();
  **   - AudioChainInstance_deinit();
  */

  /* If Audio HW needs to be stopped: call the following :
  **   - UTIL_AUDIO_RENDER_stop();
  **   - UTIL_AUDIO_CAPTURE_stop();
  **   - UTIL_AUDIO_USB_deinit();
  **   - UTIL_AUDIO_CAPTURE_deinit();
  **   - UTIL_AUDIO_RENDER_deinit();
  **   - AudioMallocDisplayRemainingMalloc(0); => provided that AUDIO_MEM_CONF_TRACK_MALLOC & AUDIO_MEM_CONF_TRACK_MALLOC_VERBOSE1 & AUDIO_MEM_CONF_TRACK_MALLOC_VERBOSE1 are set will allow to check that all malloced memory has propelry been freed
  */
}


void main_hooks_audioIdle(void)
{
  /* need to check periodically if we must stop audioChain graph */
  AudioChainInstance_idle();
}

void UTIL_AUDIO_feed(void)
{
  AudioChainInstance_run();
}

__WEAK void UTIL_AUDIO_triggerTask(void)
{

}

__WEAK void UTIL_AUDIO_createTask(bool const log_en)
{

}

/* Private Functions Definition ----------------------------------------------*/

static void s_createTerminal(void)
{
  #ifndef NO_DEFAULT_TERMINAL
  int32_t error = UTIL_TERM_set_uart_rx(UTIL_UART_GetHdle());
  if (AudioError_isError(error))
  {
    AudioChainInstance_error(__FILE__, __LINE__, "UTIL_TERM_set_uart_rx error");
  }

  if (AudioError_isOk(error))
  {
    error = UTIL_TERM_create();
    if (AudioError_isError(error))
    {
      AudioChainInstance_error(__FILE__, __LINE__, "UTIL_TERM_create error");
    }
  }
  #endif
}
