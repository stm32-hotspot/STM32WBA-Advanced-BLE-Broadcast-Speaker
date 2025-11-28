/**
******************************************************************************
* @file          st_os_monitor_cpu_freertos_template.c
* @author        MCD Application Team
* @brief         Implement CPU load manager
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

#ifdef USE_FREERTOS

#include <stdbool.h>
/*cstat -MISRAC2012-Rule-10.4_a  A third-party produces a violation message we cannot fix*/
#include "FreeRTOS.h"
#include "task.h"
/*cstat +MISRAC2012-Rule-10.4_a */
#include "st_os_monitor_cpu.h"


/* Global variables ----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
#define DWT_CYCCNT    ((uint32_t*)0xE0001004UL) // common register for all cortex 
/* Private macros ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private globals ------------------------------------------------------- */
static st_os_monitor_cpu_info_t                cpuInfo;
static enum {INIT, STARTING, STARTED, RUNNING} osCPU_state                   = INIT;
static uint32_t                                osCPU_StartTime               = 0UL;
static uint32_t                                osCPU_IdleStartTime           = 0UL;
static uint32_t                                osCPU_TotalIdleTime           = 0UL;
static uint32_t                                osCPU_CalculationPeriodCycles = 0UL;
static TaskHandle_t                            xIdleHandle                   = NULL;

/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void     s_execution_idle_time_reset(void);
static uint32_t s_execution_idle_time_get(void);

/* Functions Definition ------------------------------------------------------*/
/**
  * @brief  Start Idle monitor
  * @param  None
  * @retval None
  */
void StartIdleMonitor(void);
void StartIdleMonitor(void)
{
  if (xIdleHandle == NULL)
  {
    xIdleHandle = xTaskGetIdleTaskHandle();
  }
  if (xTaskGetCurrentTaskHandle() == xIdleHandle)
  {
    osCPU_IdleStartTime = *DWT_CYCCNT;
  }
}


/**
  * @brief  Stop Idle monitor
  * @param  None
  * @retval None
  */
void EndIdleMonitor(void);
void EndIdleMonitor(void)
{
  if (xTaskGetCurrentTaskHandle() == xIdleHandle)
  {
    static uint32_t initTime    = 0UL;
    uint32_t        currentTime = *DWT_CYCCNT;

    switch (osCPU_state)
    {
      case INIT:
        initTime    = currentTime;
        osCPU_state = STARTING;
        break;

      case STARTING:
      {
        uint32_t const timeFromInit = currentTime - initTime;

        if (timeFromInit >= (uint32_t)cpuInfo.osCpu_Freq)
        {
          // wait 1 second before measuring cpu load
          // to have a more stable value
          osCPU_StartTime = currentTime;
          osCPU_state     = STARTED;
        }
        break;
      }

      case STARTED:
      case RUNNING:
      {
        /* Store the handle to the idle task. */
        uint32_t const elapsedTime   = currentTime - osCPU_StartTime;
        uint32_t const idleSpentTime = currentTime - osCPU_IdleStartTime;

        osCPU_TotalIdleTime += idleSpentTime;
        if (elapsedTime >= osCPU_CalculationPeriodCycles)
        {
          uint32_t totalCpuSpentTime = elapsedTime - s_execution_idle_time_get();

          s_execution_idle_time_reset();

          cpuInfo.osCPU_UsageRatio = (float)totalCpuSpentTime / (float)elapsedTime;
          if (cpuInfo.osCPU_UsageRatio >= cpuInfo.osCPU_PeakRatio)
          {
            cpuInfo.osCPU_PeakRatio = cpuInfo.osCPU_UsageRatio;
          }

          osCPU_StartTime = currentTime;
          osCPU_state     = RUNNING;
        }
        break;
      }

      default:
        break;
    }
  }
}


/**
  * @brief  Init cpu monitor
  * @retval 1 if need a idle task
  */

int8_t st_os_monitor_cpu_init(uint32_t freqCpu)
{
  float CalculationPeriodCyclesFloat;

  cpuInfo.osCPU_UsageRatio      = 0.0f;
  cpuInfo.osCPU_PeakRatio       = 0.0f;
  cpuInfo.osCpu_Freq            = (float)freqCpu; // HAL_GetCpuClockFreq();
  CalculationPeriodCyclesFloat  = (float)ST_OS_MONITOR_CALCULATION_PERIOD_MS / 1000.0f * cpuInfo.osCpu_Freq;
  osCPU_CalculationPeriodCycles = (uint32_t)CalculationPeriodCyclesFloat;

  return 0;
}

/**
  * @brief  Term  Idle monitor
  * @retval 1 if ok
  */
int8_t st_os_monitor_cpu_term(void)
{
  return 1;
}


/**
  * @brief  return cpu monitor info as struct
  * @param  pInfo pointer on cpu monitor info
  * @retval 1 if ok
  */
int8_t st_os_monitor_cpu_info(st_os_monitor_cpu_info_t *pInfo)
{
  int8_t ok = 0;

  if ((pInfo != NULL) && (xIdleHandle != NULL) && (osCPU_state == RUNNING))
  {
    ok     = 1;
    *pInfo = cpuInfo;
  }

  return ok;
}


/**
  * @brief  clear the reference peak cpu
  * @retval 1 if ok
  */
int8_t st_os_monitor_cpu_clear(void)
{
  cpuInfo.osCPU_PeakRatio = 0.0f;
  return 1;
}

/* Private functions ------------------------------------------------------- */

static uint32_t s_execution_idle_time_get(void)
{
  return osCPU_TotalIdleTime;
}


static void s_execution_idle_time_reset(void)
{
  osCPU_TotalIdleTime = 0UL;
}

#endif // USE_FREERTOS
