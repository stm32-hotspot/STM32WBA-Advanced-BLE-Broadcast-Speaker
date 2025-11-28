/**
******************************************************************************
* @file          stm32_term_acsdk_cmd.c
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

#include "main_hooks.h"
#include "st_os.h"
#include "st_os_mem.h"
#include "acSdk.h"
#include "stm32_term_cmd.h"
#include "string.h"
#include "st_os_monitor_cpu.h"
#include "audio_chain_instance.h"
#include <stdio.h>
#include <string.h>

#ifndef CPU_MONITOR__TASK_NAME
  #define CPU_MONITOR_TASK_NAME "ST:CpuMonitor"
#endif
#ifndef CPU_MONITOR_TASK_STACK
  #define CPU_MONITOR_TASK_STACK  (500)
#endif
#ifndef CPU_MONITOR_TASK_PRIO
  #define CPU_MONITOR_TASK_PRIO   (ST_Priority_Low1)
#endif

#define   FLOAT_MEM_FORMAT "%10.3f"



#if !defined(AUDIO_CHAIN_RELEASE)

static void stm32_term_acsdk_cpu_monitor_task(const void *pCookie)
{
  (void)pCookie; /* unused */
  while (1)
  {
    st_os_monitor_cpu_idle();
  }
}

/**
* @brief hook from the main to init cpuload computation
*
*/
void main_hooks_task_init(void)
{
  if (st_os_monitor_cpu_init(HAL_GetCpuClockFreq()))
  {
    /* the cpuload computation needs an idle task */
    static st_task hTask;
    if (st_os_task_create(&hTask, CPU_MONITOR_TASK_NAME, stm32_term_acsdk_cpu_monitor_task, NULL, CPU_MONITOR_TASK_STACK, CPU_MONITOR_TASK_PRIO) != ST_OS_OK)
    {
      UTIL_TERM_printf_cr("ERROR: Can't create idle task\n");
    }
  }
}
#endif


/**

* @brief print mem status
*
* @param argc  num args
* @param argv  args list
*/
#define CMD_BYTES2M(bytes)      (((double)(bytes)) / (1000.0*1000.0))
#define CMD_BYTES2KB(bytes)     (((double)(bytes)) / 1024.0)
#define CMD_PCENT(val, total)   (((total) == 0UL) ? 0.0 : (100.0 * (double)(val) / (double)(total)))

static void stm32_term_acsdk_mem0(int argc, char *argv[], bool const verbose)
{
  st_os_mem_status *pSysInfo;
  st_os_mem_info(&pSysInfo);
  uint32_t totalMem = 0UL;
  uint32_t allocMem = 0UL;
  uint32_t peakMem  = 0UL;

  /* dump algos memory usage */
  acAlgosMemDump(verbose);

  for (uint32_t a = 0; a < (uint32_t)ST_Mem_Type_MAX; a++)
  {
    st_os_mem_pool *pPool = &pSysInfo->tPools[a];
    if ((pPool->iFlags & st_os_mem_hidden) == 0UL)
    {
      char tName[100];
      char tOffset[15] = "@unknown";
      uint32_t total   = pPool->iPoolSize;
      uint32_t mfree   = pPool->iPoolFreeSize;
      uint32_t alloc   = total - mfree;
      uint32_t max     = pPool->iPoolMaxAlloc;
      uint32_t nbFrag  = pPool->iPoolFrag;
      uint32_t perf    = pPool->iPerf;

      if (pPool->iPoolOffset)
      {
        snprintf(tOffset, sizeof(tOffset), "@%08X", pPool->iPoolOffset);
      }
      snprintf(tName, sizeof(tName), "%-10s : %s%s", tOffset, pPool->pName, ((pPool->iFlags & st_os_mem_unmapped) != 0UL) ? "(unused)" : "");

      if (perf)
      {
        UTIL_TERM_printf("%-30s Total:"FLOAT_MEM_FORMAT" Kb Allocated:"FLOAT_MEM_FORMAT" Kb (%5.1f%%):(%4d) Free:"FLOAT_MEM_FORMAT" Kb (%5.1f%%) Peak : "FLOAT_MEM_FORMAT" Kb  Perf:"FLOAT_MEM_FORMAT" mega rw/s\n",
                         tName,
                         CMD_BYTES2KB(total),
                         CMD_BYTES2KB(alloc),
                         CMD_PCENT(alloc, total),
                         nbFrag,
                         CMD_BYTES2KB(mfree),
                         CMD_PCENT(mfree, total),
                         CMD_BYTES2KB(max),
                         CMD_BYTES2M(perf));
      }
      else
      {
        UTIL_TERM_printf("%-30s Total:"FLOAT_MEM_FORMAT" Kb Allocated:"FLOAT_MEM_FORMAT" Kb (%5.1f%%):(%4d) Free:"FLOAT_MEM_FORMAT" Kb (%5.1f%%) Peak : "FLOAT_MEM_FORMAT" Kb\n",
                         tName,
                         CMD_BYTES2KB(total),
                         CMD_BYTES2KB(alloc),
                         CMD_PCENT(alloc, total),
                         nbFrag,
                         CMD_BYTES2KB(mfree),
                         CMD_PCENT(mfree, total),
                         CMD_BYTES2KB(max));
      }
      totalMem += total;
      allocMem += alloc;
      peakMem  += max;
    }
  }

  uint32_t freeMem = totalMem - allocMem;
  UTIL_TERM_printf("\n%-12s TotalMem:"FLOAT_MEM_FORMAT" Kb Free:"FLOAT_MEM_FORMAT" Kb (%5.1f%%) Used:"FLOAT_MEM_FORMAT" Kb (%5.1f%%) Peak:"FLOAT_MEM_FORMAT" Kb\n",
                   "STATUS",
                   CMD_BYTES2KB(totalMem),
                   CMD_BYTES2KB(freeMem),
                   CMD_PCENT(freeMem, totalMem),
                   CMD_BYTES2KB(allocMem),
                   CMD_PCENT(allocMem, totalMem),
                   CMD_BYTES2KB(peakMem));

  UTIL_TERM_printf("\n");

  UTIL_TERM_printf("%-12s Total:   "FLOAT_MEM_FORMAT" Kb\n",
                   "BSS",
                   CMD_BYTES2KB(pSysInfo->iMemBssSpace));

  UTIL_TERM_printf("%-12s Total:   "FLOAT_MEM_FORMAT" Kb\n",
                   "DATA",
                   CMD_BYTES2KB(pSysInfo->iMemDataSpace));
  UTIL_TERM_printf("%-12s Total:   "FLOAT_MEM_FORMAT" Kb\n",
                   "FLASH",
                   CMD_BYTES2KB(pSysInfo->iMemFlashSpace));
  UTIL_TERM_printf("\n");
  for (uint32_t index = 0UL; index < (sizeof(pSysInfo->tStaticPools) / sizeof(pSysInfo->tStaticPools[0])); index++)
  {
    st_os_mem_pool *pPool = &pSysInfo->tStaticPools[index];
    if (pPool ->iPoolSize != 0U)
    {
      uint32_t total  = pPool->iPoolSize;
      uint32_t mfree  = pPool->iPoolFreeSize;
      uint32_t alloc  = total - mfree;
      uint32_t max    = pPool->iPoolMaxAlloc;
      uint32_t nbFrag = pPool->iPoolFrag;

      UTIL_TERM_printf("%-12s Total:"FLOAT_MEM_FORMAT" Kb Allocated:"FLOAT_MEM_FORMAT" Kb (%5.1f%%):(%4d) Free:"FLOAT_MEM_FORMAT" Kb (%5.1f%%) Peak : "FLOAT_MEM_FORMAT" Kb\n",
                       pPool ->pName,
                       CMD_BYTES2KB(total),
                       CMD_BYTES2KB(alloc),
                       CMD_PCENT(alloc, total),
                       nbFrag,
                       CMD_BYTES2KB(mfree),
                       CMD_PCENT(mfree, total),
                       CMD_BYTES2KB(max));
    }
  }
}

static void stm32_term_acsdk_mem(int argc, char *argv[])
{
  stm32_term_acsdk_mem0(argc, argv, false);
}

static void stm32_term_acsdk_mem2(int argc, char *argv[])
{
  stm32_term_acsdk_mem0(argc, argv, true);
}


/**

* @brief print task status
*
* @param argc  num args
* @param argv  args list
*/

static void  stm32_term_acsdk_task(int argc, char *argv[])
{
  st_os_task_info_print();
}


/**
* @brief  Break the FW  ( IAR or cube IDE )
*
* @param argc  num args
* @param argv  args list
*/

static void stm32_term_acsdk_cpu(int argc, char *argv[])
{
  float currentPercent, peakPercent;
  float currentMHz, peakMHz;
  float interruptsPcent, interruptsMHz;

  /* get monitor cpu values before calling acCyclesDump so that acCyclesDump doesn't impact these values, especially for terminal task & UART */
  st_os_monitor_cpu_percent(&currentPercent, &peakPercent);
  st_os_monitor_cpu_Mhz(&currentMHz, &peakMHz);

  acCyclesDump(true, false, &interruptsPcent, &interruptsMHz);  // display on terminal and compute interrupts and tasks cpu load

  UTIL_TERM_printf("RTOS Cpu usage (without interrupts): %5.2f%% - %6.2f MHz / Peak : %5.2f%% - %6.2f MHz\n",
                   (double)currentPercent, (double)currentMHz, (double)peakPercent, (double)peakMHz);
  UTIL_TERM_printf("Total Cpu usage (with interrupts):   %5.2f%% - %6.2f MHz\n",
                   (double)currentPercent + (double)interruptsPcent, (double)currentMHz + (double)interruptsMHz);

  /* clear monitor cpu values after calling acCyclesDump so that acCyclesDump doesn't impact peak values */
  st_os_monitor_cpu_clear();
}



///**
//* @brief  list algo
//*
//* @param argc  num args
//* @param argv  args list
//*/
//
//static void stm32_term_acsdk_algo_show(int argc, char *argv[])
//{
//  int32_t bError = 1;
//
//  if (AudioChain_isStarted(&AudioChainInstance) == 0)
//  {
//    UTIL_TERM_printf_cr("AudioChain is stopped");
//    return;
//  }
//  if (argc >= 2)
//  {
//    UTIL_TERM_printf_cr("Algo Instance name :%s", argv[1]);
//    acAlgo hAlgo = acAlgoGetInstance(&AudioChainInstance, argv[1]);
//    if (hAlgo)
//    {
//      bError = acAlgoDump(hAlgo) ;
//      if (bError != 0)
//      {
//        UTIL_TERM_printf_cr("AudioChain is stopped");
//      }
//      else
//      {
//        UTIL_TERM_printf_cr("Success");
//      }
//    }
//  }
//  else
//  {
//    UTIL_TERM_printf_cr("Syntax error");
//  }
//  if (bError == 0)
//  {
//    UTIL_TERM_printf_cr("Cmd OK");
//  }
//
//}


//
//
///**
//* @brief  list algo
//*
//* @param argc  num args
//* @param argv  args list
//*/
//
//static void stm32_term_acsdk_algo_info(int argc, char *argv[])
//{
//  int32_t bError = 1;
//
//  if (argc >= 2)
//  {
//
//    UTIL_TERM_printf_cr("Algo name:%s", argv[1]);
//    if (acAlgoTemplateDump(argv[1]) != 0)
//    {
//      UTIL_TERM_printf_cr("AudioChain is stopped");
//    }
//    else
//    {
//      UTIL_TERM_printf_cr("Success");
//    }
//
//  }
//  else
//  {
//    UTIL_TERM_printf_cr("Syntax error");
//  }
//  if (bError == 0)
//  {
//    UTIL_TERM_printf_cr("Cmd OK");
//  }
//
//}
//
//
//
///**
//* @brief  set a param
//*
//* @param argc  num args
//* @param argv  args list
//*/
//
//static void stm32_term_acsdk_algo_set(int argc, char *argv[])
//{
//  int32_t bError = 1;
//  if (AudioChain_isStarted(&AudioChainInstance) == 0)
//  {
//    UTIL_TERM_printf_cr("AudioChain is stopped");
//    return;
//  }
//
//  if (argc >= 4)
//  {
//    uint8_t bUpdate = 1;
//    if (argc >= 5)
//    {
//      bUpdate = (uint8_t)atoi(argv[4]);
//    }
//    UTIL_TERM_printf_cr("Algo Instance name : %s update:%d", argv[1], bUpdate);
//    acAlgo hAlgo = acAlgoGetInstance(&AudioChainInstance, argv[1]);
//    if (hAlgo)
//    {
//      bError = acAlgoSetConfig(hAlgo, argv[2], argv[3]);
//      if (bError == 0)
//      {
//        if (bUpdate)
//        {
//          bError = acAlgoRequestUpdate(hAlgo);
//          /* wait a delay to leave enough time for the refresh asynchrone */
//          st_os_task_delay(100);
//
//        }
//      }
//      if (bError == 0)
//      {
//        bError  = acAlgoDump(hAlgo);
//      }
//    }
//    if (bError != 0)
//    {
//      UTIL_TERM_printf_cr("AudioChain is stopped");
//    }
//    else
//    {
//      UTIL_TERM_printf_cr("Success");
//    }
//  }
//  else
//  {
//    UTIL_TERM_printf_cr("Syntax error");
//  }
//  if (bError == 0)
//  {
//    UTIL_TERM_printf_cr("Cmd OK");
//  }
//
//}
//
//
//
///**
//* @brief  apply a params
//*
//* @param argc  num args
//* @param argv  args list
//*/
//
//static void stm32_term_acsdk_algo_update(int argc, char *argv[])
//{
//  int32_t bError = 1;
//  if (AudioChain_isStarted(&AudioChainInstance) == 0)
//  {
//    UTIL_TERM_printf_cr("AudioChain is stopped");
//    return;
//  }
//
//  if (argc >= 2)
//  {
//    UTIL_TERM_printf_cr("Algo Instance name :%s", argv[1]);
//    acAlgo hAlgo = acAlgoGetInstance(&AudioChainInstance, argv[1]);
//    if (hAlgo)
//    {
//      bError = acAlgoRequestUpdate(hAlgo);
//    }
//    if (bError != 0)
//    {
//      UTIL_TERM_printf_cr("AudioChain is stopped");
//    }
//  }
//  else
//  {
//    UTIL_TERM_printf_cr("Syntax error");
//  }
//  if (bError == 0)
//  {
//    UTIL_TERM_printf_cr("Cmd OK");
//  }
//}
//
//
///**
//* @brief  show algos
//*
//* @param argc  num args
//* @param argv  args list
//*/

//static void stm32_term_acsdk_algos(int argc, char *argv[])
//{
//  int32_t bError = 1;
//
//  if (AudioChain_isStarted(&AudioChainInstance) == 0)
//  {
//    UTIL_TERM_printf_cr("AudioChain is stopped");
//    return;
//  }
//
//  if (AudioChain_getAlgosList(&AudioChainInstance) == NULL)
//  {
//    UTIL_TERM_printf_cr("AudioChain is stopped");
//  }
//  for (audio_algo_list_t *pCurrent = AudioChain_getAlgosList(&AudioChainInstance); pCurrent != NULL; pCurrent = pCurrent->next)
//  {
//    audio_algo_t *pAlgo = pCurrent->pAlgo;
//
//    UTIL_TERM_printf("%2d: %s %s\n", AudioAlgo_getIndex(pAlgo), pAlgo->pName, pAlgo->pDesc);
//  }
//  if (bError == 0)
//  {
//    UTIL_TERM_printf_cr("Cmd OK");
//  }
//
//}


TERM_CMD_DECLARE("mem", NULL, "Print the memory status with algos memory usage summary", stm32_term_acsdk_mem);
TERM_CMD_DECLARE("mem2", NULL, "Print the memory status with algos detailed memory usage", stm32_term_acsdk_mem2);
TERM_CMD_DECLARE("task", NULL, "Print the task status", stm32_term_acsdk_task);
TERM_CMD_DECLARE("cpu", NULL, "Print the cpu status", stm32_term_acsdk_cpu);
//TERM_CMD_DECLARE("algos", NULL, "Display algo list in current graph", stm32_term_acsdk_algos);
//TERM_CMD_DECLARE("algo_info", "[algo]", "Show the algo info", stm32_term_acsdk_algo_info);
//TERM_CMD_DECLARE("algo_show", "[instance]", "Show all parameters for an algo ", stm32_term_acsdk_algo_show);
//TERM_CMD_DECLARE("algo_set", "[instance] [param] [value] [apply 1/0]", "Set an algo parameter", stm32_term_acsdk_algo_set);
//TERM_CMD_DECLARE("algo_update", "[instance] ", "Apply parameters previously set", stm32_term_acsdk_algo_update);
//

