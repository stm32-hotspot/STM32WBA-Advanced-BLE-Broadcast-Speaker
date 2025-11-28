/**
******************************************************************************
* @file          st_os_monitor_cpu.h
* @author        MCD Application Team
* @brief         implement CPU load manager
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


#ifndef st_os_monitor_h
#define st_os_monitor_h
/* Includes ------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C"
{
#endif
#include <stdint.h>

/* Exported constants --------------------------------------------------------*/
#ifndef ST_OS_MONITOR_CALCULATION_PERIOD_MS
// to insure correct cycles measurement, calculation period in cycles must be < 2^32
// i.e. ST_OS_MONITOR_CALCULATION_PERIOD_MS must be < 2^32 * 1000 / osCpu_Freq
// for instance with osCpu_Freq = 520 MHz, we obtain ST_OS_MONITOR_CALCULATION_PERIOD_MS < 8259 ms
#define ST_OS_MONITOR_CALCULATION_PERIOD_MS 500UL
#endif

/* Exported types ------------------------------------------------------------*/
typedef struct st_os_monitor_cpu_info_t
{
  float osCPU_UsageRatio;
  float osCPU_PeakRatio;
  float osCpu_Freq;
} st_os_monitor_cpu_info_t;


int8_t st_os_monitor_cpu_clear(void);
int8_t st_os_monitor_cpu_percent(float *pCpuCurrent, float *pCpuPeak);
int8_t st_os_monitor_cpu_Mhz(float *pCpuCurrent, float *pCpuPeak);
int8_t st_os_monitor_cpu_info(st_os_monitor_cpu_info_t *pInfo);
int8_t st_os_monitor_cpu_init(uint32_t freqCpu);
int8_t st_os_monitor_cpu_term(void);
void   st_os_monitor_cpu_idle(void);


#ifdef __cplusplus
};
#endif
#endif /* st_monitor_h */


