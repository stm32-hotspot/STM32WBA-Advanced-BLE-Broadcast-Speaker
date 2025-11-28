/**
******************************************************************************
* @file          st_os_monitor_cpu.c
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
/*cstat -MISRAC2012-Rule-10.4_a  A third-party produces a violation message we cannot fix*/
#include "cmsis_compiler.h"
/*cstat +MISRAC2012-Rule-10.4_a*/

#include "st_os_monitor_cpu.h"



__WEAK int8_t st_os_monitor_cpu_init(uint32_t freqCpu)
{
  return 1;
}

__WEAK int8_t st_os_monitor_cpu_term(void)
{
  return 1;
}
__WEAK int8_t st_os_monitor_cpu_info(st_os_monitor_cpu_info_t *pInfo)
{
  /* must be implemented */
  return 0;
}
__WEAK int8_t st_os_monitor_cpu_clear(void)
{
  return 0;
}
__WEAK void  st_os_monitor_cpu_idle(void)
{
}




/**
  * @brief  get percent
  * @param  pCpuCurrent ptr on current
  * @param  pCpuCurrent pCpuPeak on peak
  * @retval 1 if ok
  */
int8_t st_os_monitor_cpu_percent(float *pCpuCurrent, float *pCpuPeak)
{
  st_os_monitor_cpu_info_t cpuInfo;
  int8_t ok = st_os_monitor_cpu_info(&cpuInfo);

  if (ok)
  {
    if (pCpuCurrent)
    {
      *pCpuCurrent = 100.0f * cpuInfo.osCPU_UsageRatio;
    }

    if (pCpuPeak)
    {
      *pCpuPeak = 100.0f * cpuInfo.osCPU_PeakRatio;
    }
  }
  return ok;
}


/**
  * @brief  get Mhz
  * @param  pCpuCurrent ptr on current
  * @param  pCpuCurrent pCpuPeak on peak
  * @retval 1 if ok
  */
int8_t st_os_monitor_cpu_Mhz(float *pCpuCurrent, float *pCpuPeak)
{
  st_os_monitor_cpu_info_t cpuInfo;
  int8_t ok = st_os_monitor_cpu_info(&cpuInfo);

  if (ok)
  {
    if (pCpuCurrent)
    {
      *pCpuCurrent = cpuInfo.osCpu_Freq * cpuInfo.osCPU_UsageRatio * 0.000001f;
    }
    if (pCpuPeak)
    {
      *pCpuPeak = cpuInfo.osCpu_Freq * cpuInfo.osCPU_PeakRatio * 0.000001f;
    }
  }
  return ok;
}



