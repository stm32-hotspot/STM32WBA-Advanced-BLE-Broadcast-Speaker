/**
******************************************************************************
* @file    cycles_measure.h
* @author  MCD Application Team
* @brief   Header for cycles_measure.c module
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


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CYCLES_MEASURE_H
#define __CYCLES_MEASURE_H

/* Includes ------------------------------------------------------------------*/
#include "cycles.h"

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported structures--------------------------------------------------------*/
typedef struct
{
  float pcent;
  float MHz;
} cyclesMeasure_info_t;

typedef struct
{
  // audio interrupt (every UTIL_AUDIO_N_MS_PER_INTERRUPT ms)
  cyclesMeasure_info_t audioInterrupt;

  // audio capture task (every UTIL_AUDIO_N_MS_PER_INTERRUPT ms)
  cyclesMeasure_info_t audioCaptureTask;

  // audio algo tasks (every AC_SYSIN_MIC_MS ms)
  cyclesMeasure_info_t algosDataInOutTask;
  cyclesMeasure_info_t algosProcessTask;
  cyclesMeasure_info_t algosProcessLowLevelTask;
  cyclesMeasure_info_t algosControlTask;
  cyclesMeasure_info_t afePartnerTask;

  // audio framework tasks (every AC_SYSIN_MIC_MS ms)
  cyclesMeasure_info_t frameworkDataInOutTask;
  cyclesMeasure_info_t frameworkProcessTask;
  cyclesMeasure_info_t frameworkProcessLowLevelTask;
  cyclesMeasure_info_t frameworkControlTask;

  // ethernet tasks
  cyclesMeasure_info_t netRx;
  cyclesMeasure_info_t netTx;
  cyclesMeasure_info_t netIf;
  cyclesMeasure_info_t netLink;

  // UART interrupt
  cyclesMeasure_info_t uartInterrupt;

  // terminal task
  cyclesMeasure_info_t terminalTask;

  // ST message task
  cyclesMeasure_info_t stMessageTask;

  // back-ground taskk
  cyclesMeasure_info_t backGroundTask;
} cyclesMeasure_values_t;

/* Exported variables ------------------------------------------------------- */
/* Exported functions ------------------------------------------------------- */

/* Cycles Count routines */
void displayDetailedCycleCount(bool const traceLog, bool const interruptsOnly, cyclesMeasure_values_t *const pCyclesMeasure_values);

#endif /* __CYCLES_MEASURE_H */

