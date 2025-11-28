/**
******************************************************************************
* @file    cycles_measure.c
* @author  MCD Application Team
* @brief   Helper file to manage cycles measures
*          parallel audio path
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
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "cycles_measure.h"
#include "audio_chain_instance.h"
#include "main_hooks.h"
#include "stm32_term_tasks.h"

#ifdef USE_PARTNER_AFE
  #include "user_afe.h"
#endif

#ifndef AUDIO_CHAIN_RELEASE
  #include "st_message.h"
#endif

#ifdef USE_THREADX
  #include "tx_user.h"
  #if (TX_EXECUTION_PROFILE_ENABLE == 1) && defined(TX_CORTEX_M_EPK)
    #define TX_EXECUTION_ISR_PROFILE
  #endif
#endif
#ifdef TX_EXECUTION_ISR_PROFILE
  #include "tx_api.h"
  #include "tx_execution_profile.h"
  #define TX_EXECUTION_ISR_ENTER _tx_execution_isr_enter();
  #define TX_EXECUTION_ISR_EXIT  _tx_execution_isr_exit();
#else
  #define TX_EXECUTION_ISR_ENTER
  #define TX_EXECUTION_ISR_EXIT
#endif


/* Global variables ----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
// structure for background task, terminal task, audio interrupts, audio capture, etc... cycles measure
static CycleStatsTypeDef G_audioInterruptsCycles;
static CycleStatsTypeDef G_audioCaptureCycles;
static CycleStatsTypeDef G_backgroundCycles;
static CycleStatsTypeDef G_terminalCycles;
static CycleStatsTypeDef G_uartIrqCycles;
static CycleStatsTypeDef G_uartDmaStreamTxIrqCycles;
static CycleStatsTypeDef G_uartDmaStreamRxIrqCycles;
#ifndef AUDIO_CHAIN_RELEASE
  static CycleStatsTypeDef G_stMsgCycles;
#endif
#ifdef USE_ETHERNET
  static CycleStatsTypeDef G_netRxCycles;
  static CycleStatsTypeDef G_netTxCycles;
  static CycleStatsTypeDef G_netIfCycles;
  static CycleStatsTypeDef G_netLinkCycles;
#endif

/* Functions Definition ------------------------------------------------------*/
void displayDetailedCycleCount(bool const traceLog, bool const interruptsOnly, cyclesMeasure_values_t *const pCyclesMeasure_values)
{
  if (pCyclesMeasure_values != NULL)
  {
    cycles_stats_t audioInterruptsCyclesStats;
    cycles_stats_t uartIrqCyclesStats, uartDmaStreamTxIrqCyclesStats;
    cycles_stats_t uartDmaStreamRxIrqCyclesStats;
    uint32_t       uartInterruptsHz;

    memset(pCyclesMeasure_values, 0, sizeof(cyclesMeasure_values_t));

    // audio interrupts cycles
    cycleMeasure_getCyclesMgntStats(&G_audioInterruptsCycles, &audioInterruptsCyclesStats, CYCLES_LAST_MEASURE);
    pCyclesMeasure_values->audioInterrupt.pcent = audioInterruptsCyclesStats.averageCpuLoadPcent;
    pCyclesMeasure_values->audioInterrupt.MHz   = 0.000001f * (float)audioInterruptsCyclesStats.averageCpuLoadHz;

    // UART interrupts cycles
    cycleMeasure_getCyclesMgntStats(&G_uartIrqCycles,            &uartIrqCyclesStats,            CYCLES_LAST_MEASURE);
    cycleMeasure_getCyclesMgntStats(&G_uartDmaStreamTxIrqCycles, &uartDmaStreamTxIrqCyclesStats, CYCLES_LAST_MEASURE);
    cycleMeasure_getCyclesMgntStats(&G_uartDmaStreamRxIrqCycles, &uartDmaStreamRxIrqCyclesStats, CYCLES_LAST_MEASURE);
    pCyclesMeasure_values->uartInterrupt.pcent = uartIrqCyclesStats.averageCpuLoadPcent + uartDmaStreamTxIrqCyclesStats.averageCpuLoadPcent + uartDmaStreamRxIrqCyclesStats.averageCpuLoadPcent;
    uartInterruptsHz                           = uartIrqCyclesStats.averageCpuLoadHz    + uartDmaStreamTxIrqCyclesStats.averageCpuLoadHz    + uartDmaStreamRxIrqCyclesStats.averageCpuLoadHz;
    pCyclesMeasure_values->uartInterrupt.MHz   = 0.000001f * (float)uartInterruptsHz;

    if (interruptsOnly)
    {
      if (traceLog)
      {
        UTIL_TERM_printf("AudioInterrupts    : %5.2f%% - %6.2f MHz\n", (double)pCyclesMeasure_values->audioInterrupt.pcent, (double)pCyclesMeasure_values->audioInterrupt.MHz);
        UTIL_TERM_printf("UART interrupts    : %5.2f%% - %6.2f MHz\n", (double)pCyclesMeasure_values->uartInterrupt.pcent, (double)pCyclesMeasure_values->uartInterrupt.MHz);
      }
    }
    else
    {
      bool const     audioChainStarted = AudioChain_isStarted(&AudioChainInstance);
      cycles_stats_t audioCaptureCyclesStats;
      cycles_stats_t terminalCyclesStats;
      #ifndef AUDIO_CHAIN_RELEASE
      cycles_stats_t stMsgCyclesStats;
      #endif
      #ifdef USE_PARTNER_AFE
      cycles_stats_t afePartnerCyclesStats;
      #endif
      #ifdef USE_ETHERNET
      cycles_stats_t netRxCyclesStats, netTxCyclesStats, netIfCyclesStats, netLinkCyclesStats;
      #endif
      cycles_stats_t backgroundCyclesStats;
      cycles_stats_t audioChainDataInOutCyclesStats, audioChainProcessCyclesStats, audioChainProcessLowLevelCyclesStats, audioChainControlCyclesStats;
      float          audioChainTotalPcent, audioTotalPcent, totalPcent;
      float          audioChainTotalMHz,   audioTotalMHz,   totalMHz;

      // audio capture cycles
      cycleMeasure_getCyclesMgntStats(&G_audioCaptureCycles, &audioCaptureCyclesStats, CYCLES_LAST_MEASURE);
      pCyclesMeasure_values->audioCaptureTask.pcent = audioCaptureCyclesStats.averageCpuLoadPcent;
      pCyclesMeasure_values->audioCaptureTask.MHz   = 0.000001f * (float)audioCaptureCyclesStats.averageCpuLoadHz;

      // audio cycles
      audioTotalPcent = pCyclesMeasure_values->audioInterrupt.pcent + pCyclesMeasure_values->audioCaptureTask.pcent;
      audioTotalMHz   = pCyclesMeasure_values->audioInterrupt.MHz   + pCyclesMeasure_values->audioCaptureTask.MHz;

      if (audioChainStarted)
      {
        float algosTotalPcent;
        float algosTotalMHz;
        float audioDataInOutPcent;
        float audioProcessPcent;
        float audioProcessLowLevelPcent;
        float audioControlPcent;
        float audioDataInOutMHz;
        float audioProcessMHz;
        float audioProcessLowLevelMHz;
        float audioControlMHz;

        // algos cycles
        if (traceLog)
        {
          UTIL_TERM_printf("audio-chain algos:\n");
        }
        for (audio_algo_list_t *pAlgoList = AudioChain_getAlgosList(&AudioChainInstance); pAlgoList != NULL; pAlgoList = pAlgoList->next)
        {
          CycleStatsTypeDef *const pAlgoDataInOutCycleStats = AudioAlgo_getDataInOutCyclesMgntStats(pAlgoList->pAlgo);
          CycleStatsTypeDef *const pAlgoProcessCycleStats   = AudioAlgo_getProcessCyclesMgntStats(pAlgoList->pAlgo);
          CycleStatsTypeDef *const pAlgoControlCycleStats   = AudioAlgo_getControlCyclesMgntStats(pAlgoList->pAlgo);
          cycles_stats_t           algoDataInOutCyclesStats, algoProcessCyclesStats, algoControlCyclesStats;
          float                    algoDataInOutPcent, algoProcessPcent, algoControlPcent, algoTotalPcent;
          float                    algoDataInOutMHz,   algoProcessMHz,   algoControlMHz,   algoTotalMHz;
          bool                     processLowLevel = (pAlgoProcessCycleStats->task == PROCESSLOWLEVEL_TASK);

          cycleMeasure_getCyclesMgntStats(pAlgoDataInOutCycleStats, &algoDataInOutCyclesStats, CYCLES_LAST_MEASURE);
          cycleMeasure_getCyclesMgntStats(pAlgoProcessCycleStats,   &algoProcessCyclesStats,   CYCLES_LAST_MEASURE);
          cycleMeasure_getCyclesMgntStats(pAlgoControlCycleStats,   &algoControlCyclesStats,   CYCLES_LAST_MEASURE);
          algoDataInOutPcent = algoDataInOutCyclesStats.averageCpuLoadPcent;
          algoProcessPcent   = algoProcessCyclesStats.averageCpuLoadPcent;
          algoControlPcent   = algoControlCyclesStats.averageCpuLoadPcent;
          algoDataInOutMHz   = 0.000001f * (float)algoDataInOutCyclesStats.averageCpuLoadHz;
          algoProcessMHz     = 0.000001f * (float)algoProcessCyclesStats.averageCpuLoadHz;
          algoControlMHz     = 0.000001f * (float)algoControlCyclesStats.averageCpuLoadHz;
          algoTotalPcent     = algoDataInOutPcent + algoProcessPcent + algoControlPcent;
          algoTotalMHz       = algoDataInOutMHz   + algoProcessMHz   + algoControlMHz;
          if (traceLog)
          {
            UTIL_TERM_printf("  %s:\n"
                             "    DataInOut      : %5.2f%% - %6.2f MHz\n"
                             "    %s: %5.2f%% - %6.2f MHz\n"
                             "    Control        : %5.2f%% - %6.2f MHz\n"
                             "    Total          : %5.2f%% - %6.2f MHz\n",
                             AudioAlgo_getFullDescriptionString(pAlgoList->pAlgo),
                             (double)algoDataInOutPcent, (double)algoDataInOutMHz,
                             processLowLevel ? "ProcessLowLevel" : "Process        ",
                             (double)algoProcessPcent, (double)algoProcessMHz,
                             (double)algoControlPcent, (double)algoControlMHz,
                             (double)algoTotalPcent, (double)algoTotalMHz);
          }

          pCyclesMeasure_values->algosDataInOutTask.pcent += algoDataInOutPcent;
          pCyclesMeasure_values->algosDataInOutTask.MHz   += algoDataInOutMHz;
          if (processLowLevel)
          {
            pCyclesMeasure_values->algosProcessLowLevelTask.pcent += algoProcessPcent;
            pCyclesMeasure_values->algosProcessLowLevelTask.MHz   += algoProcessMHz;
          }
          else
          {
            pCyclesMeasure_values->algosProcessTask.pcent += algoProcessPcent;
            pCyclesMeasure_values->algosProcessTask.MHz   += algoProcessMHz;
          }
          pCyclesMeasure_values->algosControlTask.pcent += algoControlPcent;
          pCyclesMeasure_values->algosControlTask.MHz   += algoControlMHz;
        }
        algosTotalPcent = pCyclesMeasure_values->algosDataInOutTask.pcent + pCyclesMeasure_values->algosProcessTask.pcent + pCyclesMeasure_values->algosProcessLowLevelTask.pcent + pCyclesMeasure_values->algosControlTask.pcent;
        algosTotalMHz   = pCyclesMeasure_values->algosDataInOutTask.MHz   + pCyclesMeasure_values->algosProcessTask.MHz   + pCyclesMeasure_values->algosProcessLowLevelTask.MHz   + pCyclesMeasure_values->algosControlTask.MHz;
        if (traceLog)
        {
          UTIL_TERM_printf("total audio-chain algos:\n"
                           "    DataInOut      : %5.2f%% - %6.2f MHz\n"
                           "    Process        : %5.2f%% - %6.2f MHz\n"
                           "    ProcessLowLevel: %5.2f%% - %6.2f MHz\n"
                           "    Control        : %5.2f%% - %6.2f MHz\n"
                           "    Total          : %5.2f%% - %6.2f MHz\n",
                           (double)pCyclesMeasure_values->algosDataInOutTask.pcent, (double)pCyclesMeasure_values->algosDataInOutTask.MHz,
                           (double)pCyclesMeasure_values->algosProcessTask.pcent, (double)pCyclesMeasure_values->algosProcessTask.MHz,
                           (double)pCyclesMeasure_values->algosProcessLowLevelTask.pcent, (double)pCyclesMeasure_values->algosProcessLowLevelTask.MHz,
                           (double)pCyclesMeasure_values->algosControlTask.pcent, (double)pCyclesMeasure_values->algosControlTask.MHz,
                           (double)algosTotalPcent, (double)algosTotalMHz);
        }

        // audio chain cycles
        cycleMeasure_getCyclesMgntStats(AudioChain_getDataInOutTaskCycleMgntStats(&AudioChainInstance),       &audioChainDataInOutCyclesStats,       CYCLES_LAST_MEASURE);
        cycleMeasure_getCyclesMgntStats(AudioChain_getProcessTaskCycleMgntStats(&AudioChainInstance),         &audioChainProcessCyclesStats,         CYCLES_LAST_MEASURE);
        cycleMeasure_getCyclesMgntStats(AudioChain_getProcessLowLevelTaskCycleMgntStats(&AudioChainInstance), &audioChainProcessLowLevelCyclesStats, CYCLES_LAST_MEASURE);
        cycleMeasure_getCyclesMgntStats(AudioChain_getControlTaskCycleMgntStats(&AudioChainInstance),         &audioChainControlCyclesStats,         CYCLES_LAST_MEASURE);
        pCyclesMeasure_values->frameworkDataInOutTask.pcent       = audioChainDataInOutCyclesStats.averageCpuLoadPcent;
        pCyclesMeasure_values->frameworkProcessTask.pcent         = audioChainProcessCyclesStats.averageCpuLoadPcent;
        pCyclesMeasure_values->frameworkProcessLowLevelTask.pcent = audioChainProcessLowLevelCyclesStats.averageCpuLoadPcent;
        pCyclesMeasure_values->frameworkControlTask.pcent         = audioChainControlCyclesStats.averageCpuLoadPcent;
        pCyclesMeasure_values->frameworkDataInOutTask.MHz         = 0.000001f * (float)audioChainDataInOutCyclesStats.averageCpuLoadHz;
        pCyclesMeasure_values->frameworkProcessTask.MHz           = 0.000001f * (float)audioChainProcessCyclesStats.averageCpuLoadHz;
        pCyclesMeasure_values->frameworkProcessLowLevelTask.MHz   = 0.000001f * (float)audioChainProcessLowLevelCyclesStats.averageCpuLoadHz;
        pCyclesMeasure_values->frameworkControlTask.MHz           = 0.000001f * (float)audioChainControlCyclesStats.averageCpuLoadHz;
        audioChainTotalPcent                                      = pCyclesMeasure_values->frameworkDataInOutTask.pcent + pCyclesMeasure_values->frameworkProcessTask.pcent + pCyclesMeasure_values->frameworkProcessLowLevelTask.pcent + pCyclesMeasure_values->frameworkControlTask.pcent;
        audioChainTotalMHz                                        = pCyclesMeasure_values->frameworkDataInOutTask.MHz   + pCyclesMeasure_values->frameworkProcessTask.MHz   + pCyclesMeasure_values->frameworkProcessLowLevelTask.MHz   + pCyclesMeasure_values->frameworkControlTask.MHz;
        if (traceLog)
        {
          UTIL_TERM_printf("audio-chain tasks (audio framework):\n"
                           "    DataInOut      : %5.2f%% - %6.2f MHz\n"
                           "    Process        : %5.2f%% - %6.2f MHz\n"
                           "    ProcessLowLevel: %5.2f%% - %6.2f MHz\n"
                           "    Control        : %5.2f%% - %6.2f MHz\n"
                           "    Total          : %5.2f%% - %6.2f MHz\n",
                           (double)pCyclesMeasure_values->frameworkDataInOutTask.pcent, (double)pCyclesMeasure_values->frameworkDataInOutTask.MHz,
                           (double)pCyclesMeasure_values->frameworkProcessTask.pcent, (double)pCyclesMeasure_values->frameworkProcessTask.MHz,
                           (double)pCyclesMeasure_values->frameworkProcessLowLevelTask.pcent, (double)pCyclesMeasure_values->frameworkProcessLowLevelTask.MHz,
                           (double)pCyclesMeasure_values->frameworkControlTask.pcent, (double)pCyclesMeasure_values->frameworkControlTask.MHz,
                           (double)audioChainTotalPcent, (double)audioChainTotalMHz);
        }
        audioDataInOutPcent       = pCyclesMeasure_values->algosDataInOutTask.pcent       + pCyclesMeasure_values->frameworkDataInOutTask.pcent;
        audioProcessPcent         = pCyclesMeasure_values->algosProcessTask.pcent         + pCyclesMeasure_values->frameworkProcessTask.pcent;
        audioProcessLowLevelPcent = pCyclesMeasure_values->algosProcessLowLevelTask.pcent + pCyclesMeasure_values->frameworkProcessLowLevelTask.pcent;
        audioControlPcent         = pCyclesMeasure_values->algosControlTask.pcent         + pCyclesMeasure_values->frameworkControlTask.pcent;
        audioDataInOutMHz         = pCyclesMeasure_values->algosDataInOutTask.MHz         + pCyclesMeasure_values->frameworkDataInOutTask.MHz;
        audioProcessMHz           = pCyclesMeasure_values->algosProcessTask.MHz           + pCyclesMeasure_values->frameworkProcessTask.MHz;
        audioProcessLowLevelMHz   = pCyclesMeasure_values->algosProcessLowLevelTask.MHz   + pCyclesMeasure_values->frameworkProcessLowLevelTask.MHz;
        audioControlMHz           = pCyclesMeasure_values->algosControlTask.MHz           + pCyclesMeasure_values->frameworkControlTask.MHz;

        // PARTNER AFE messages task cycles
        #ifdef USE_PARTNER_AFE
        if (userAfe_getCyclesStat() != NULL)
        {
          cycleMeasure_getCyclesMgntStats(userAfe_getCyclesStat(), &afePartnerCyclesStats, CYCLES_LAST_MEASURE);
          pCyclesMeasure_values->afePartnerTask.pcent = afePartnerCyclesStats.averageCpuLoadPcent;
          pCyclesMeasure_values->afePartnerTask.MHz   = 0.000001f * (float)afePartnerCyclesStats.averageCpuLoadHz;
        }
        #endif

        audioTotalPcent += audioDataInOutPcent + audioProcessPcent + audioProcessLowLevelPcent + audioControlPcent + pCyclesMeasure_values->afePartnerTask.pcent;
        audioTotalMHz   += audioDataInOutMHz   + audioProcessMHz   + audioProcessLowLevelMHz   + audioControlMHz   + pCyclesMeasure_values->afePartnerTask.MHz;
        if (traceLog)
        {
          UTIL_TERM_printf("total audio:\n"
                           "    AudioInterrupts: %5.2f%% - %6.2f MHz\n"
                           "    AudioCapture   : %5.2f%% - %6.2f MHz\n"
                           "    DataInOut      : %5.2f%% - %6.2f MHz\n"
                           "    Process        : %5.2f%% - %6.2f MHz\n"
                           "    ProcessLowLevel: %5.2f%% - %6.2f MHz\n"
                           "    Control        : %5.2f%% - %6.2f MHz\n"
                           #ifdef USE_PARTNER_AFE
                           "    AFE            : %5.2f%% - %6.2f MHz\n"
                           #endif
                           "    Total          : %5.2f%% - %6.2f MHz\n\n",
                           (double)pCyclesMeasure_values->audioInterrupt.pcent, (double)pCyclesMeasure_values->audioInterrupt.MHz,
                           (double)pCyclesMeasure_values->audioCaptureTask.pcent, (double)pCyclesMeasure_values->audioCaptureTask.MHz,
                           (double)audioDataInOutPcent, (double)audioDataInOutMHz,
                           (double)audioProcessPcent, (double)audioProcessMHz,
                           (double)audioProcessLowLevelPcent, (double)audioProcessLowLevelMHz,
                           (double)audioControlPcent, (double)audioControlMHz,
                           #ifdef USE_PARTNER_AFE
                           (double)pCyclesMeasure_values->afePartnerTask.pcent, (double)pCyclesMeasure_values->afePartnerTask.MHz,
                           #endif
                           (double)audioTotalPcent, (double)audioTotalMHz);
        }
      }
      else
      {
        if (traceLog)
        {
          UTIL_TERM_printf("total audio:\n"
                           "    AudioInterrupts: %5.2f%% - %6.2f MHz\n"
                           "    AudioCapture   : %5.2f%% - %6.2f MHz\n"
                           "    Total          : %5.2f%% - %6.2f MHz\n\n",
                           (double)pCyclesMeasure_values->audioInterrupt.pcent, (double)pCyclesMeasure_values->audioInterrupt.MHz,
                           (double)pCyclesMeasure_values->audioCaptureTask.pcent, (double)pCyclesMeasure_values->audioCaptureTask.MHz,
                           (double)audioTotalPcent, (double)audioTotalMHz);
        }
      }

      // Ethernet messages task cycles
      #ifdef USE_ETHERNET
      cycleMeasure_getCyclesMgntStats(&G_netRxCycles,   &netRxCyclesStats,   CYCLES_LAST_MEASURE);
      cycleMeasure_getCyclesMgntStats(&G_netTxCycles,   &netTxCyclesStats,   CYCLES_LAST_MEASURE);
      cycleMeasure_getCyclesMgntStats(&G_netIfCycles,   &netIfCyclesStats,   CYCLES_LAST_MEASURE);
      cycleMeasure_getCyclesMgntStats(&G_netLinkCycles, &netLinkCyclesStats, CYCLES_LAST_MEASURE);
      pCyclesMeasure_values->netRx.pcent   = netRxCyclesStats.averageCpuLoadPcent;
      pCyclesMeasure_values->netRx.MHz     = 0.000001f * (float)netRxCyclesStats.averageCpuLoadHz;
      pCyclesMeasure_values->netTx.pcent   = netTxCyclesStats.averageCpuLoadPcent;
      pCyclesMeasure_values->netTx.MHz     = 0.000001f * (float)netTxCyclesStats.averageCpuLoadHz;
      pCyclesMeasure_values->netIf.pcent   = netIfCyclesStats.averageCpuLoadPcent;
      pCyclesMeasure_values->netIf.MHz     = 0.000001f * (float)netIfCyclesStats.averageCpuLoadHz;
      pCyclesMeasure_values->netLink.pcent = netLinkCyclesStats.averageCpuLoadPcent;
      pCyclesMeasure_values->netLink.MHz   = 0.000001f * (float)netLinkCyclesStats.averageCpuLoadHz;
      if (traceLog)
      {
        UTIL_TERM_printf("Ethernet tasks (TCP/IP not measured):\n"
                         "  network RX       : %5.2f%% - %6.2f MHz\n"
                         "  network TX       : %5.2f%% - %6.2f MHz\n"
                         "  network IF       : %5.2f%% - %6.2f MHz\n"
                         "  network Link     : %5.2f%% - %6.2f MHz\n\n",
                         (double)pCyclesMeasure_values->netRx.pcent, (double)pCyclesMeasure_values->netRx.MHz,
                         (double)pCyclesMeasure_values->netTx.pcent, (double)pCyclesMeasure_values->netTx.MHz,
                         (double)pCyclesMeasure_values->netIf.pcent, (double)pCyclesMeasure_values->netIf.MHz,
                         (double)pCyclesMeasure_values->netLink.pcent, (double)pCyclesMeasure_values->netLink.MHz);
      }
      #endif

      // terminal task cycles
      cycleMeasure_getCyclesMgntStats(&G_terminalCycles, &terminalCyclesStats, CYCLES_LAST_MEASURE);
      pCyclesMeasure_values->terminalTask.pcent = terminalCyclesStats.averageCpuLoadPcent;
      pCyclesMeasure_values->terminalTask.MHz   = 0.000001f * (float)terminalCyclesStats.averageCpuLoadHz;
      if (traceLog)
      {
        UTIL_TERM_printf("UART interrupts    : %5.2f%% - %6.2f MHz\n", (double)pCyclesMeasure_values->uartInterrupt.pcent, (double)pCyclesMeasure_values->uartInterrupt.MHz);
        UTIL_TERM_printf("Terminal task      : %5.2f%% - %6.2f MHz\n", (double)pCyclesMeasure_values->terminalTask.pcent, (double)pCyclesMeasure_values->terminalTask.MHz);
      }

      #ifndef AUDIO_CHAIN_RELEASE
      // ST messages task cycles
      cycleMeasure_getCyclesMgntStats(&G_stMsgCycles, &stMsgCyclesStats, CYCLES_LAST_MEASURE);
      pCyclesMeasure_values->stMessageTask.pcent = stMsgCyclesStats.averageCpuLoadPcent;
      pCyclesMeasure_values->stMessageTask.MHz   = 0.000001f * (float)stMsgCyclesStats.averageCpuLoadHz;
      if (traceLog)
      {
        UTIL_TERM_printf("ST messages task   : %5.2f%% - %6.2f MHz\n", (double)pCyclesMeasure_values->stMessageTask.pcent, (double)pCyclesMeasure_values->stMessageTask.MHz);
      }
      #endif

      // background task cycles
      cycleMeasure_getCyclesMgntStats(&G_backgroundCycles, &backgroundCyclesStats, CYCLES_LAST_MEASURE);
      pCyclesMeasure_values->backGroundTask.pcent = backgroundCyclesStats.averageCpuLoadPcent;
      pCyclesMeasure_values->backGroundTask.MHz   = 0.000001f * (float)backgroundCyclesStats.averageCpuLoadHz;
      if (traceLog)
      {
        UTIL_TERM_printf("Background task    : %5.2f%% - %6.2f MHz\n\n", (double)pCyclesMeasure_values->backGroundTask.pcent, (double)pCyclesMeasure_values->backGroundTask.MHz);
      }

      // grand total cycles
      totalPcent = audioTotalPcent + pCyclesMeasure_values->terminalTask.pcent + pCyclesMeasure_values->netRx.pcent + pCyclesMeasure_values->netTx.pcent + pCyclesMeasure_values->netIf.pcent + pCyclesMeasure_values->netLink.pcent + pCyclesMeasure_values->stMessageTask.pcent + pCyclesMeasure_values->uartInterrupt.pcent + pCyclesMeasure_values->backGroundTask.pcent;
      totalMHz   = audioTotalMHz   + pCyclesMeasure_values->terminalTask.MHz   + pCyclesMeasure_values->netRx.MHz   + pCyclesMeasure_values->netTx.MHz   + pCyclesMeasure_values->netIf.MHz   + pCyclesMeasure_values->netLink.MHz   + pCyclesMeasure_values->stMessageTask.MHz   + pCyclesMeasure_values->uartInterrupt.MHz   + pCyclesMeasure_values->backGroundTask.MHz;
      if (traceLog)
      {
        UTIL_TERM_printf("Total              : %5.2f%% - %6.2f MHz\n\n", (double)totalPcent, (double)totalMHz);
      }
    }
  }
}


// redefinition of __weak routines for cycles measure
#ifdef USE_ETHERNET
void NetRx_cycleMeasure_Init(void)
{
  cycleMeasure_Init();
  G_netRxCycles.pName = "NetRx";
  G_netRxCycles.task  = OTHER_TASK;
}


void NetRx_cycleMeasure_Reset(void)
{
  cycleMeasure_Reset(&G_netRxCycles);
}


void NetRx_cycleMeasure_Start(void)
{
  cycleMeasure_Start(&G_netRxCycles);
}


void NetRx_cycleMeasure_Stop(void)
{
  if (cycleMeasure_Stop(&G_netRxCycles, AudioChainInstance_getCycleCountMeasureTimeout()))
  {
    /* add here any stuff you want regarding G_netRxCycles */
    NetRx_cycleMeasure_Reset();
  }
}


void NetTx_cycleMeasure_Init(void)
{
  cycleMeasure_Init();
  G_netTxCycles.pName = "NetTx";
  G_netTxCycles.task  = OTHER_TASK;
}


void NetTx_cycleMeasure_Reset(void)
{
  cycleMeasure_Reset(&G_netTxCycles);
}


void NetTx_cycleMeasure_Start(void)
{
  cycleMeasure_Start(&G_netTxCycles);
}


void NetTx_cycleMeasure_Stop(void)
{
  if (cycleMeasure_Stop(&G_netTxCycles, AudioChainInstance_getCycleCountMeasureTimeout()))
  {
    /* add here any stuff you want regarding G_netTxCycles */
    NetTx_cycleMeasure_Reset();
  }
}


void NetIf_cycleMeasure_Init(void)
{
  cycleMeasure_Init();
  G_netIfCycles.pName = "NetIf";
  G_netIfCycles.task  = OTHER_TASK;
}


void NetIf_cycleMeasure_Reset(void)
{
  cycleMeasure_Reset(&G_netIfCycles);
}


void NetIf_cycleMeasure_Start(void)
{
  cycleMeasure_Start(&G_netIfCycles);
}


void NetIf_cycleMeasure_Stop(void)
{
  if (cycleMeasure_Stop(&G_netIfCycles, AudioChainInstance_getCycleCountMeasureTimeout()))
  {
    /* add here any stuff you want regarding G_netIfCycles */
    NetIf_cycleMeasure_Reset();
  }
}


void NetLink_cycleMeasure_Init(void)
{
  cycleMeasure_Init();
  G_netLinkCycles.pName = "NetLink";
  G_netLinkCycles.task  = OTHER_TASK;
}


void NetLink_cycleMeasure_Reset(void)
{
  cycleMeasure_Reset(&G_netLinkCycles);
}


void NetLink_cycleMeasure_Start(void)
{
  cycleMeasure_Start(&G_netLinkCycles);
}


void NetLink_cycleMeasure_Stop(void)
{
  if (cycleMeasure_Stop(&G_netLinkCycles, AudioChainInstance_getCycleCountMeasureTimeout()))
  {
    /* add here any stuff you want regarding G_netLinkCycles */
    NetLink_cycleMeasure_Reset();
  }
}
#endif // USE_ETHERNET


void UTIL_TERM_cycleMeasure_Init(void)
{
  cycleMeasure_Init();
  G_terminalCycles.pName = "Terminal";
  G_terminalCycles.task  = OTHER_TASK;
}


void UTIL_TERM_cycleMeasure_Reset(void)
{
  cycleMeasure_Reset(&G_terminalCycles);
}


void UTIL_TERM_cycleMeasure_Start(void)
{
  cycleMeasure_Start(&G_terminalCycles);
}


void UTIL_TERM_cycleMeasure_Stop(void)
{
  if (cycleMeasure_Stop(&G_terminalCycles, AudioChainInstance_getCycleCountMeasureTimeout()))
  {
    /* add here any stuff you want regarding G_terminalCycles */
    UTIL_TERM_cycleMeasure_Reset();
  }
}


#ifndef AUDIO_CHAIN_RELEASE
void st_message_cycleMeasure_Init(void)
{
  cycleMeasure_Init();
  G_stMsgCycles.pName = "ST messages";
  G_stMsgCycles.task  = OTHER_TASK;
}


void st_message_cycleMeasure_Reset(void)
{
  cycleMeasure_Reset(&G_stMsgCycles);
}


void st_message_cycleMeasure_Start(void)
{
  cycleMeasure_Start(&G_stMsgCycles);
}


void st_message_cycleMeasure_Stop(void)
{
  if (cycleMeasure_Stop(&G_stMsgCycles, AudioChainInstance_getCycleCountMeasureTimeout()))
  {
    /* add here any stuff you want regarding G_stMsgCycles */
    st_message_cycleMeasure_Reset();
  }
}
#endif


void main_hooks_cyclesMeasureInit(void)
{
  cycleMeasure_Init();
  G_backgroundCycles.pName = "background";
  G_backgroundCycles.task  = OTHER_TASK;
}


void main_hooks_cyclesMeasureReset(void)
{
  cycleMeasure_Reset(&G_backgroundCycles);
}


void main_hooks_cyclesMeasureStart(void)
{
  cycleMeasure_Start(&G_backgroundCycles);
}


void main_hooks_cyclesMeasureStop(void)
{
  if (cycleMeasure_Stop(&G_backgroundCycles, AudioChainInstance_getCycleCountMeasureTimeout()))
  {
    /* add here any stuff you want regarding G_backgroundCycles */
    main_hooks_cyclesMeasureReset();
  }
}


void UTIL_UART_IRQ_cycleMeasure_Init(void)
{
  cycleMeasure_Init();
  G_uartIrqCycles.pName = "UART IRQ";
  G_uartIrqCycles.task  = INTERRUPT;
}


void UTIL_UART_IRQ_cycleMeasure_Reset(void)
{
  cycleMeasure_Reset(&G_uartIrqCycles);
}


void UTIL_UART_IRQ_cycleMeasure_Start(void)
{
  TX_EXECUTION_ISR_ENTER
  cycleMeasure_Start(&G_uartIrqCycles);
}


void UTIL_UART_IRQ_cycleMeasure_Stop(void)
{
  if (cycleMeasure_Stop(&G_uartIrqCycles, AudioChainInstance_getCycleCountMeasureTimeout()))
  {
    /* add here any stuff you want regarding G_uartIrqCycles */
    UTIL_UART_IRQ_cycleMeasure_Reset();
  }
  TX_EXECUTION_ISR_EXIT
}


void UTIL_UART_DMA_STREAM_RX_IRQ_cycleMeasure_Init(void)
{
  cycleMeasure_Init();
  G_uartDmaStreamRxIrqCycles.pName = "UART DMA RX IRQ";
  G_uartDmaStreamRxIrqCycles.task  = INTERRUPT;
}


void UTIL_UART_DMA_STREAM_RX_IRQ_cycleMeasure_Reset(void)
{
  cycleMeasure_Reset(&G_uartDmaStreamRxIrqCycles);
}


void UTIL_UART_DMA_STREAM_RX_IRQ_cycleMeasure_Start(void)
{
  TX_EXECUTION_ISR_ENTER
  cycleMeasure_Start(&G_uartDmaStreamRxIrqCycles);
}


void UTIL_UART_DMA_STREAM_RX_IRQ_cycleMeasure_Stop(void)
{
  if (cycleMeasure_Stop(&G_uartDmaStreamRxIrqCycles, AudioChainInstance_getCycleCountMeasureTimeout()))
  {
    /* add here any stuff you want regarding G_uartDmaStreamRxIrqCycles */
    UTIL_UART_DMA_STREAM_RX_IRQ_cycleMeasure_Reset();
  }
  TX_EXECUTION_ISR_EXIT
}


void UTIL_UART_DMA_STREAM_TX_IRQ_cycleMeasure_Init(void)
{
  cycleMeasure_Init();
  G_uartDmaStreamTxIrqCycles.pName = "UART DMA TX IRQ";
  G_uartDmaStreamTxIrqCycles.task  = INTERRUPT;
}


void UTIL_UART_DMA_STREAM_TX_IRQ_cycleMeasure_Reset(void)
{
  cycleMeasure_Reset(&G_uartDmaStreamTxIrqCycles);
}


void UTIL_UART_DMA_STREAM_TX_IRQ_cycleMeasure_Start(void)
{
  TX_EXECUTION_ISR_ENTER
  cycleMeasure_Start(&G_uartDmaStreamTxIrqCycles);
}


void UTIL_UART_DMA_STREAM_TX_IRQ_cycleMeasure_Stop(void)
{
  if (cycleMeasure_Stop(&G_uartDmaStreamTxIrqCycles, AudioChainInstance_getCycleCountMeasureTimeout()))
  {
    /* add here any stuff you want regarding G_uartDmaStreamTxIrqCycles */
    UTIL_UART_DMA_STREAM_TX_IRQ_cycleMeasure_Reset();
  }
  TX_EXECUTION_ISR_EXIT
}


void UTIL_AUDIO_INOUT_IRQ_cycleMeasure_Init(void)
{
  cycleMeasure_Init();
  G_audioInterruptsCycles.pName = "Audio IRQ";
  G_audioInterruptsCycles.task  = INTERRUPT;
}


void UTIL_AUDIO_INOUT_IRQ_cycleMeasure_Reset(void)
{
  cycleMeasure_Reset(&G_audioInterruptsCycles);
}


void UTIL_AUDIO_INOUT_IRQ_cycleMeasure_Start(void)
{
  TX_EXECUTION_ISR_ENTER
  cycleMeasure_Start(&G_audioInterruptsCycles);
}


void UTIL_AUDIO_INOUT_IRQ_cycleMeasure_Stop(void)
{
  if (cycleMeasure_Stop(&G_audioInterruptsCycles, AudioChainInstance_getCycleCountMeasureTimeout()))
  {
    /* add here any stuff you want regarding G_audioInterruptsCycles */
    UTIL_AUDIO_INOUT_IRQ_cycleMeasure_Reset();
  }
  TX_EXECUTION_ISR_EXIT
}


void UTIL_AUDIO_cycleMeasure_Init(void)
{
  cycleMeasure_Init();
  G_audioCaptureCycles.pName = "Audio capture";
  G_audioCaptureCycles.task  = OTHER_TASK;
}


void UTIL_AUDIO_cycleMeasure_Reset(void)
{
  cycleMeasure_Reset(&G_audioCaptureCycles);
}


void UTIL_AUDIO_cycleMeasure_Start(void)
{
  cycleMeasure_Start(&G_audioCaptureCycles);
}


void UTIL_AUDIO_cycleMeasure_Stop(void)
{
  if (cycleMeasure_Stop(&G_audioCaptureCycles, AudioChainInstance_getCycleCountMeasureTimeout()))
  {
    /* add here any stuff you want regarding G_audioCaptureCycles */
    UTIL_AUDIO_cycleMeasure_Reset();
  }
}
