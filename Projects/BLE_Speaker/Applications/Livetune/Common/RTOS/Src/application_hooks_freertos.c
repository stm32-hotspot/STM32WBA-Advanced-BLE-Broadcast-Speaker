/**
******************************************************************************
* @file    application_hooks_freertos.c
* @author  MCD Application Team
* @brief   Connect application & FreeRTOS (idle, hooks, etc...)
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

#ifdef USE_FREERTOS

/* Includes ------------------------------------------------------------------*/
#include "st_os_hl.h"
#include "background_task.h"
/*cstat -MISRAC2012-Rule-10.4_a  A third-party produces a violation message we cannot fix*/
#include "FreeRTOS.h"
#include "task.h"
/*cstat +MISRAC2012-Rule-10.4_a */
#include "stm32xxx_voice_bsp.h"
#include "main_hooks.h"
#include "traces.h"
#include "irq_utils.h"

#ifdef USE_TRACEALYZER
  #include "trcRecorder.h"
#endif

/* Global variables ----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/

/* Allocate the memory for the heap. */
#if( configAPPLICATION_ALLOCATED_HEAP == 1 )
  #define FREERTOS_SECTION ".FreeRTOS_Section"

  /* The application writer has already defined the array used for the RTOS
  heap - probably so it can be placed in a special segment or address. */

  VAR_DECLARE_ALIGN4_AT_SECTION(uint8_t, ucHeap[configTOTAL_HEAP_SIZE], FREERTOS_SECTION);    /*cstat !MISRAC2012-Rule-1.4_a extended language features needed*/

#endif /* configAPPLICATION_ALLOCATED_HEAP */


#ifndef RTOS_BACKGROUND_TASK_ENABLE
  #if configUSE_IDLE_HOOK == 1
    /* no background task since we use FreeRTOS idle task */
    #define RTOS_BACKGROUND_TASK_ENABLE 0U

  #else
    #define RTOS_BACKGROUND_TASK_ENABLE 1U

  #endif
#endif

/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void s_backgroundFunc(uint16_t const param, ARGUMENT_TYPE argument);

/* Extern function prototypes ------------------------------------------------*/
extern void vApplicationIdleHook(void);
extern void vApplicationMallocFailedHook(void);

/* Functions Definition ------------------------------------------------------*/

/**
* @brief  Init Rtime tracing
* @param  None
* @retval None.
*/
void RtTraces_init(void);
__weak void RtTraces_init(void)
{
}

/* Main.c hooks function redefinition ----------------------------------------*/
/**
* @brief  Init
* @param  None
* @retval None.
*/
void main_hooks_rtos_init(void)
{
  st_os_hl_init();
}


/**
* @brief  Activate TraceAlyzer
* @param  None
* @retval None.
*/
void main_hooks_activateTrace(void)
{
  #ifdef USE_TRACEALYZER
  /* Trace MUST be activated before any task creation */
  #ifndef CFG_TUD_TRACEALYZER_CDC
  vTraceEnable(TRC_START);
  #else
  #if TRC_CFG_START_TYPE   == TRC_START_AWAIT_HOST
  printf("Trace alyzer, wait start from host\n");
  #endif
  vTraceEnable(TRC_CFG_START_TYPE);
  #endif

  /* Init real time traces - if compiled */
  RtTraces_init();

  #endif
}

/* Private function definition ------------------------------------------*/


/* create a background task for cpu monitor purpose: if background task was in idle task, total cycles - idle cycles wouldn't represent free cycles */
/* moreover, putting background task in a specific task is mandatory with some RTOS (no access to idle task with some RTOS)                         */

static void s_backgroundFunc(uint16_t const param, ARGUMENT_TYPE argument)
{
  main_hooks_idle();
}

/**
* @brief  Creates a debug task that prints useful information concerning the RTOS and performances,
* @retval None.
*/
static void s_createBackgroundTask(uint8_t enable)
{
  if (enable == 1U)
  {
    if (!st_os_hl_task_background_create(s_backgroundFunc))
    {
      trace_print(TRACE_OUTPUT_ALL, TRACE_LVL_ERROR_FATAL, "Background_Thread creation failed\n");
    }
  }
}



/**
* @brief  Connect RTOS kernel start to our main application
* @param  None
* @retval None.
*/
void main_hooks_run(void)
{
  s_createBackgroundTask(RTOS_BACKGROUND_TASK_ENABLE);
  st_os_hl_start(10UL);  /* 10 ms timeout for all threads started */
}



/* RTOS Hooks function definition --------------------------------------------*/

void vApplicationMallocFailedHook(void)
{
  main_hooks_error_handler();
}

#if configUSE_IDLE_HOOK == 1
#include "wba_link.h"
/* we are in FreeRTOS and we use idle hook for background task */
void vApplicationIdleHook(void)
{
  main_hooks_idle();
  app_sleep();
}
#endif

#if configCHECK_FOR_STACK_OVERFLOW == 1
/**
* @brief  While 1 to be sure that user will know that there are some memory overflow. MUST NOT be kept tin the ned application hence under define.
* @param  Task handle
* @param  Task name
* @retval None.
*/

void vApplicationStackOverflowHook(xTaskHandle xTask, char *pcTaskName);
void vApplicationStackOverflowHook(xTaskHandle xTask, char *pcTaskName)
{
  disable_irq_with_cnt();
  while (1) {}
}
#endif


#if (configSUPPORT_STATIC_ALLOCATION  == 1 && configSUPPORT_DYNAMIC_ALLOCATION == 1)
/**
 * Workaround until moving to CMSIS_V2
 */

/**
 * @brief This is to provide memory that is used by the Idle task.
 *
 * If configUSE_STATIC_ALLOCATION is set to 1, then the application must provide an
 * implementation of vApplicationGetIdleTaskMemory() in order to provide memory to
 * the Idle task.
 */
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                   StackType_t **ppxIdleTaskStackBuffer,
                                   uint32_t *pulIdleTaskStackSize);

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                   StackType_t **ppxIdleTaskStackBuffer,
                                   uint32_t *pulIdleTaskStackSize)
{
  /* If the buffers to be provided to the Idle task are declared inside this
     * function then they must be declared static - otherwise they will be allocated on
     * the stack and so not exists after this function exits. */
  static StaticTask_t xIdleTaskTCB;
  static StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE];

  /* Pass out a pointer to the StaticTask_t structure in which the Idle
     * task's state will be stored. */
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

  /* Pass out the array that will be used as the Idle task's stack. */
  *ppxIdleTaskStackBuffer = uxIdleTaskStack;

  /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
     * Note that, as the array is necessarily of type StackType_t,
     * configMINIMAL_STACK_SIZE is specified in words, not bytes. */
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

/**
 * @brief configUSE_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
 *        application must provide an implementation of vApplicationGetTimerTaskMemory()
 *        to provide the memory that is used by the Timer service task.
 *
 * @param ppxTimerTaskTCBBuffer
 * @param ppxTimerTaskStackBuffer
 * @param pulTimerTaskStackSize
 */
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
                                    StackType_t **ppxTimerTaskStackBuffer,
                                    uint32_t *pulTimerTaskStackSize);

void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
                                    StackType_t **ppxTimerTaskStackBuffer,
                                    uint32_t *pulTimerTaskStackSize)
{
  /* If the buffers to be provided to the Timer task are declared inside this
  * function then they must be declared static - otherwise they will be allocated on
  * the stack and so not exists after this function exits. */
  static StaticTask_t xTimerTaskTCB;
  static StackType_t uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH];

  /* Pass out a pointer to the StaticTask_t structure in which the Timer
     * task's state will be stored. */
  *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;

  /* Pass out the array that will be used as the Timer task's stack. */
  *ppxTimerTaskStackBuffer = uxTimerTaskStack;

  /* Pass out the size of the array pointed to by *ppxTimerTaskStackBuffer.
     * Note that, as the array is necessarily of type StackType_t,
     * configTIMER_TASK_STACK_DEPTH is specified in words, not bytes. */
  *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}
#endif
#endif  /* defined (USE_FREERTOS) */
