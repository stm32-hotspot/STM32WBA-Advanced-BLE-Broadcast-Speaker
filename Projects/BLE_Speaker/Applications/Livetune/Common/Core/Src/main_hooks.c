/**
******************************************************************************
* @file    main_hooks.c
* @author  MCD Application Team
* @brief   Main program body
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
#include <stdio.h>
#include "irq_utils.h"
#include "core_init.h"
#include "BoardSetup.h"
#include "stm32_audio.h"
#include "traces.h"
#include "cycles.h"
#include "main_hooks.h"


#if defined(TUD_AUDIO_OUT_USED) || defined(TUD_AUDIO_IN_USED)
  #include "tinyusb.h"
#endif

#include "stm32xxx_voice_bsp.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#ifndef IDLE_PROCESS_MAX
  #define IDLE_PROCESS_MAX 3
#endif

#if defined(USE_FREERTOS)
  #define BUILD_RTOS  "FreeRtos"
#elif defined(USE_THREADX)
  #define BUILD_RTOS  "ThreadX"
#else
  #define BUILD_RTOS  "Bare Metal"
#endif


#if defined(__ICCARM__)
  #define BUILD_TOOL_CHAIN  "IAR"
#elif defined(__GNUC__)
  #define BUILD_TOOL_CHAIN  "GCC"
#else
  #define BUILD_TOOL_CHAIN  "Unknown"
#endif

/* Private macro -------------------------------------------------------------*/
#define N_SEC ((1000UL * UTIL_AUDIO_N_MS_DIV) / UTIL_AUDIO_N_MS_PER_INTERRUPT)

/* Private variables ---------------------------------------------------------*/
/* Private functions prototypes-----------------------------------------------*/
static void s_stackMonitoring(void);
static void s_initStackMonitoring(void);


/* external functions prototypes ---------------------------------------------*/
void AudioChainInstance_initSystem(void);

/* Functions Definition ------------------------------------------------------*/

void main_hooks_pre_init(void)
{
  s_initStackMonitoring();

  /* At init (before tasks are launched and idle loop is running), TRACES MUST BE SYNCHRONOUS (displayed immediately), */
  /* else they wouldn't be displayed because in asynchronous mode they are displayed in idle loop;                     */
  /* moreover synchronous traces don't impact real-time at this stage because real-time processing is not running      */
  trace_setAsynchronous(false);

  /* hook overloadbale to set the audio config , default is the config 0*/
  AudioChainInstance_initSystem();
}


void main_hooks_post_init(void)
{
  #if 0
  /* just for test: to see available character styles and colors and chosen trace log colors depending on trace level
  WARNING: this trace blocks the system in a while(1) loop after the last trace */
  trace_test();
  #endif

  /* AFTER INIT, TRACES MUST BE ASYNCHRONOUS (displayed in background in idle loop);                                          */
  /* in asynchronous mode, traces are much less intrusive (they don't impact real time) when called from high-priority tasks, */
  /* especially UART traces which may consume lots of cycles if baud rate is low;                                             */
  /* moreover asynchronous mode insures traces order consistency (a trace from high-priority task won't interrupt a trace     */
  /* of lower priority tasks)                                                                                                 */
  /* Remark: error traces will always be displayed immediately whatever synchronous/asynchronous mode                         */
  trace_setAsynchronous(true);

  /* Start Idle cycle count if implemented by user */
  main_hooks_cyclesMeasureInit();
  main_hooks_cyclesMeasureReset();

  #ifndef STM32H5
  // Workaround : Enable DCache after Audio init & started
  SCB_EnableDCache();
  #endif
}

void main_hooks_coreInit(void)
{
  CoreInit();
  AudioChainInstance_initSystem(); /* update the audio configuration that has been defined at this point*/
}

void main_hooks_boardInit(void)
{
  BoardInit();
}

void main_hooks_boardPreInit(void)
{
  BoardPreInit();
}


void main_hooks_idle(void)
{
  static uint32_t minInterval   = 0UL;
  static uint32_t lastCycles    = 0UL;
  uint32_t        currentCycles = 0UL;
  uint32_t        interval      = 0xFFFFFFFFUL;

  /* start cycles measure for background task */
  main_hooks_cyclesMeasureStart();

  currentCycles = cycleMeasure_currentCycles();
  if (minInterval == 0UL)
  {
    minInterval = cycleMeasure_getSystemCoreClock() >> 8;  /* 1/256 second, i.e. around 4 ms */
  }
  else
  {
    interval = currentCycles - lastCycles;
  }

  if (interval > minInterval)
  {
    /* do all this stuff only only if last time is more than minInterval ago */
    lastCycles = currentCycles;

    /* stack monitoring */
    s_stackMonitoring();

    /* Cache and core events monitoring */
    main_hooks_cacheMonitor();

    /* Dump footprints */
    main_hooks_footprintDump();

    /* check AudioMalloc consistency if AUDIO_MEM_CONF_TRACK_MALLOC is defined */
    AudioMallocCheckConsistency();

    /* must be called periodically if user wants to use isButtonPushed or getButtonState */
    //checkButton();

    /* asynchronous trace to display if activated */
    main_hooks_logTrace();
  }

  /* if afe used owns an idle or tick function to be called */
  main_hooks_audioIdle();

  /* user routine to be run in idle */
  main_hooks_user_idle();

  /* stop cycles measure for background task */
  main_hooks_cyclesMeasureStop();
}


/* List of weak functions to offer plug and play services  ------------------*/

/* defined as weak - by default empty; to be defined application hook used */
__weak void main_hooks_rtos_init(void)
{
}


__weak void main_hooks_task_init(void)
{
}

__weak void main_hooks_run(void)
{
  while (1)
  {
    main_hooks_idle();
    /* Don't add any delay here , it may break USB function because of it's idle task */
  }
  /* End while*/
}



__weak void main_hooks_print(void)
{
  /* Display starting trace on all chosen trace output */
  trace_print(TRACE_OUTPUT_UART, TRACE_LVL_LOG, "\n\nBoard started :\n");
  trace_print(TRACE_OUTPUT_UART, TRACE_LVL_LOG, "\tFrequency   : %d MHz\n", HAL_GetCpuClockFreq() / 1000UL / 1000UL);
  trace_print(TRACE_OUTPUT_UART, TRACE_LVL_LOG, "\tBuild       : %s %s\n", __DATE__, __TIME__);
  trace_print(TRACE_OUTPUT_UART, TRACE_LVL_LOG, "\tTool Chain  : %s\n", BUILD_TOOL_CHAIN);
  trace_print(TRACE_OUTPUT_UART, TRACE_LVL_LOG, "\tRTOS        : %s\n", BUILD_RTOS);
  trace_print(TRACE_OUTPUT_UART, TRACE_LVL_LOG, "\tAppVersion  : v%s\n", APP_REVISION);

  #ifdef TEST_VERSION
  trace_print(TRACE_OUTPUT_UART, TRACE_LVL_WARNING, "\tTEST_VERSION : %s\n", TEST_VERSION);
  #endif

  #if defined(TUD_AUDIO_IN_USED) || defined(TUD_AUDIO_IN_USED)
  uint32_t vid, pid;
  tusb_device_get_usb_ids(&vid, &pid);
  trace_print(TRACE_OUTPUT_UART, TRACE_LVL_LOG, "\tUSB VID:PID : %d:%d\n", vid, pid);
  #endif
}


__weak void main_hooks_systemReset(void)
{
  HAL_Delay(500);
  HAL_NVIC_SystemReset();
}


__weak void main_hooks_footprintDump(void)
{
}

__weak void main_hooks_cacheMonitor(void)
{
}


__weak void main_hooks_user_idle(void)
{
}


__weak void main_hooks_activateTrace(void)
{
  /* Can init some GPIO to measure task duration for instance */
  return;
}


/* defined as weak - */
__weak void main_hooks_logTrace(void)
{
  trace_outputBuffers();
  return;
}



/* ---------------------------------------------------------------------------*/
/* First audio stream management: in general AFE  ----------------------------*/
/* ---------------------------------------------------------------------------*/

/* defined as weak - by default empty; to be defined inside application wrapper if used */
__weak void main_hooks_audioInit(void)
{
  return;
}


__weak void main_hooks_audioDeInit(void)
{
  return;
}


__weak void main_hooks_audioFeed(audio_buffer_t const *const pBuff1,
                                 audio_buffer_t const *const pBuff2)
{
  /* send some audio 2 usb (mostly for debug & investigation)
   * allows audio recording on the PC for post analysis */
  main_hooks_usbRecFeed(pBuff1, pBuff2);

  return;
}


__weak void main_hooks_audioIdle(void)
{
}

/* ---------------------------------------------------------------------------*/
/* USB feeding; defined as so it can be unplugged ----------------------------*/
/* ---------------------------------------------------------------------------*/

__weak void main_hooks_usbRecFeed(audio_buffer_t const *const pBuff1, audio_buffer_t const *const pBuff2)
{
  if ((pBuff1 != NULL) && (pBuff2 != NULL))
  {
    UTIL_AUDIO_USB_REC_mergeAndSend(pBuff1, pBuff2);
  }
  else if (pBuff1 != NULL)
  {
    if (AudioBuffer_getNbChannels(pBuff1) == 2U)
    {
      UTIL_AUDIO_USB_REC_send(pBuff1);
    }
  }
  else
  {
    static bool traceOnce = false;
    if (traceOnce == false)
    {
      /*Not supported */
      trace_print(TRACE_OUTPUT_UART, TRACE_LVL_LOG, "USB STREAMING CONF NOT SUPPORTED\n");
      traceOnce = true;
    }
  }
}


__weak void main_hooks_cyclesMeasureInit(void)
{
}


__weak void main_hooks_cyclesMeasureReset(void)
{
}


__weak void main_hooks_cyclesMeasureStart(void)
{
}


__weak void main_hooks_cyclesMeasureStop(void)
{
}


/* ---------------------------------------------------------------------------*/
/* Stack monitoring  ---------------------------------------------------------*/
/* ---------------------------------------------------------------------------*/
#ifdef STACK_MONITORING
  #if defined(__ICCARM__)
    #pragma section = "CSTACK"
    static uint32_t const *const stackBegin   = (uint32_t *)__section_end("CSTACK");
    static uint32_t const       *stackPtr     = (uint32_t *)__section_end("CSTACK");
    static uint32_t const *const stackEnd     = (uint32_t *)__section_begin("CSTACK");

  #elif defined(__GNUC__)
    extern uint8_t _sstack;
    extern uint8_t _estack;
    static uint32_t const *const stackBegin   = (uint32_t *) &_estack;
    static uint32_t const       *stackPtr     = (uint32_t *) &_estack;
    static uint32_t const *const stackEnd     = (uint32_t *) &_sstack;
  #endif
  static uint32_t          const stackPattern = 0xC0A7BE94UL;
#endif


/**
  * @brief  if stack monitoring used, fill the stack section with a known pattern, this function must be called early in the main(),
            it fill a pattern the section from the master SP already used, to the CSTACK start section
  * @retval None
  */
void s_initStackMonitoring(void)
{
  #ifdef STACK_MONITORING
  disable_irq_with_cnt();
  uint32_t *p = (uint32_t *)__get_MSP();

  stackPtr = p;
  p--;
  while (1)
  {
    *p = stackPattern;
    if (p == stackEnd)
    {
      break;
    }
    p--;
  }
  enable_irq_with_cnt();
  #endif
}


/**
  * @brief  check the master stack section, notice s_initStackMonitoring() must be called before the service starts
  * @retval None
  */
static void s_stackMonitoring(void)
{
  #ifdef STACK_MONITORING
  uint32_t const *const stackThreshold = &stackEnd[(stackBegin - stackEnd) / 8];    /* 12.5% free stack */  /*cstat !MISRAC2012-Rule-18.2 stackBegin and stackEnd address the same memory zone*/
  uint32_t const       *p              = (uint32_t const *)__get_MSP();
  #ifndef BUILD_REDUCED_LOG_MESSAGE
  uint32_t const       *lastStackPtr   = stackPtr;
  #endif

  if (p < stackEnd)
  {
    trace_print(TRACE_OUTPUT_UART, TRACE_LVL_ERROR, "main stack pointer below stack end !\n");

    /* stack level below threshold (stack max usage too big) => we block FW */
    disable_irq_with_cnt();
    while (1)
    {
    }
  }

  for (p = stackEnd; p < stackBegin; p++) /*cstat !MISRAC2012-Rule-18.3 p and stackBegin address the same memory zone*/
  {
    if (p[0] != stackPattern)
    {
      stackPtr = p;
      break;
    }
  }

  #ifndef BUILD_REDUCED_LOG_MESSAGE
  if (stackPtr < lastStackPtr)
  {
    static bool firstTime = true;
    int const   stackUsed = (int)stackBegin - (int)stackPtr;  /*cstat !MISRAC2012-Rule-18.2 stackBegin and stackEnd address the same memory zone*/
    int const   stackSize = (int)stackBegin - (int)stackEnd;  /*cstat !MISRAC2012-Rule-18.2 stackBegin and stackEnd address the same memory zone*/

    if (firstTime)
    {
      trace_print(TRACE_OUTPUT_UART, TRACE_LVL_DEBUG, "main stack from 0x%08X (excluded) to 0x%08X (included)\n", stackBegin, stackEnd);
      firstTime = false;
    }
    trace_print(TRACE_OUTPUT_UART, TRACE_LVL_DEBUG, "main stack usage: %d bytes out of %d => %d%%\n", stackUsed, stackSize, (100 * stackUsed) / stackSize);  /*cstat !MISRAC2012-Rule-1.3_h stackSize is not 0*/
  }
  #endif

  if (stackPtr < stackThreshold)  /*cstat !MISRAC2012-Rule-18.3 stackBegin and stackEnd address the same memory zone*/
  {
    trace_print(TRACE_OUTPUT_UART, TRACE_LVL_ERROR, "main stack too small (more than 7/8 were used), please increase !\n");

    /* stack level below threshold (stack max usage too big) => we block FW */
    disable_irq_with_cnt();
    while (1)
    {
    }
  }
  #endif
}


/**
  * @brief  Error Handler.
  * @retval None
  */
void main_hooks_error_handler(void)
{
  //disable_irq_with_cnt();
  while (1)
  {

  }
}


/* ---------------------------------------------------------------------------*/
/* Static  functions  --------------------------------------------------------*/
/* ---------------------------------------------------------------------------*/
