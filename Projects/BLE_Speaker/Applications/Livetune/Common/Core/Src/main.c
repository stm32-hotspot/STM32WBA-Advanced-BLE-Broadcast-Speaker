/**
******************************************************************************
* @file    main.c
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
#include "main.h"
#include "main_hooks.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private functions prototypes-----------------------------------------------*/
void cacheMonitoring(void);
/* Functions Definition ------------------------------------------------------*/



int main(void)
{
  main_hooks_pre_init();

  /* Setup processor clocks, signal routing, timers, etc. */
  main_hooks_coreInit();

  /*
    Init basic hw such as Uart for traces Leds and buttons. This must be done just after hw core inits
    This to make sure we have logs and led instrumentations immediately available for the next sequence.
  */
  main_hooks_boardPreInit();

  /*
    TraceAlyzer if RTOS or can be GPIO init for scheduling tests
     notice if we define TRC_CFG_START_TYPE=TRC_START_AWAIT_HOST
    The code will be stopped here until we press start from Tracealyzer 4
  */
  main_hooks_activateTrace();

  main_hooks_rtos_init();    /* Initialize RTOS if needed before boardInit which may create a task for audio capture */
  main_hooks_task_init();    /* allows to customize inits before the kernel start */
  main_hooks_boardInit();    /* Setup board peripherals (CODECs, external memory, etc.)*/
  main_hooks_print();        /* Display starting trace on all chosen trace output */
  main_hooks_audioInit();    /* Initialize Audio Front End*/
  main_hooks_post_init();

  /* Start while loop or OS if any */
  main_hooks_run();

  return 0;
}   /* End main*/


void main_hooks_cacheMonitor(void)
{
  cacheMonitoring();
}

/* Empty function for compilation even if Utilities/CacheCnt component is not used */
__weak void cacheMonitoring(void)
{
}