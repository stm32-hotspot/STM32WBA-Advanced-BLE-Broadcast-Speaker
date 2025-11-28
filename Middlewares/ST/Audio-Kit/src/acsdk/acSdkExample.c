/**
  ******************************************************************************
  * @file    acSdkExample.c
  * @author  MCD Application Team
  * @brief   audio chain SDK Example
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

#include  <stdio.h>
#include  <assert.h>
#include  "acSdk.h"
#include  "math.h"
#include  "cmsis_os.h"

/* Global variables ----------------------------------------------------------*/
/* some handles */

/*! [create-pipe] */

static acAlgo                   hGenerator1;
static acAlgo                   hSplit1;
static acPipe                   hPipe;
static acAlgo                   hRms;
static acChunk                  hSysOut1;
static acChunk                  hSysOut2;
static acChunk                  hCnx_0;
static acChunk                  hCnx_1;

/* Private function prototypes -----------------------------------------------*/
static void     s_closeCB(void  *argument);
static int32_t  s_control_Algo_RMS_1(acAlgo hAlgo);

/* Functions Definition ------------------------------------------------------*/

__weak void AudioChainInstance_initGraph(void)
{
  int32_t error = 0;

  /*! set environmental parameters [set-env] */
  acEnvDump();
  assert(acEnvSetConfig("setTrace", "debug") == 0);
  /*! [set-env] */


  /*! generate algo [auto-doc] */
  error = acAlgoListDump();
  assert(error == 0);

  error = acAlgoTemplateDump("signal_generator");
  assert(error == 0);

  error = acAlgoTemplateDump("split");
  assert(error == 0);

  error = acAlgoTemplateDump("rms");
  assert(error == 0);

  /*![auto-doc] */


  /*! Initialize the API [initialize] */
  acInitialize();

  error = acPipeCreate(&hPipe);
  assert(error  == 0);
  /*! [initialize] */


  /*! Create the pipe */

  /*! Create chunks [create-chunk] */
  acChunkCreate(hPipe, acSYSOUTCHUNK2, &hSysOut2);
  assert(hSysOut2);

  acChunkCreate(hPipe, acSYSOUTCHUNK1, &hSysOut1);
  assert(hSysOut1);

  acChunkCreate(hPipe, "hCnx_0", &hCnx_0);
  assert(hCnx_0);

  acChunkCreate(hPipe, "hCnx_1", &hCnx_1);
  assert(hCnx_1);

  /*! [create-chunk] */


  acChunkDump(hCnx_1);

  /* We change the default setup properties */

  acChunkSetConfig(hCnx_0, "fs", "16000");
  acChunkSetConfig(hCnx_0, "interleaved", "ABUFF_FORMAT_NON_INTERLEAVED");

  /* Verify the config */
  acChunkDump(hCnx_1);

  /* Create the algo generator */

  acAlgoCreate(hPipe, "signal_generator", "signal_generator-1", &hGenerator1, 0);
  assert(hGenerator1);

  /* Dump its default value */
  error = acAlgoDump(hGenerator1);
  assert(error == 0);


  /* Configure the signal-generator-1 instance */
  error = acAlgoSetConfig(hGenerator1, "signalType", "SINE");
  assert(error == 0);

  error = acAlgoSetConfig(hGenerator1, "frequency", "500");
  assert(error == 0);

  error = acAlgoSetConfig(hGenerator1, "gain", "0.500000");
  assert(error == 0);

  /* Verify its new configuration */
  error = acAlgoDump(hGenerator1);
  assert(error == 0);


  /* create  the algo split2-1 instance */
  acAlgoCreate(hPipe, "split", "split2-1", &hSplit1, 0);
  assert(hSplit1);

  /* Verify its configuration */
  acAlgoDump(hSplit1);


  /* Create  the algo RMS instance */
  /*! Create chunks [create-algo] */


  acAlgoCreate(hPipe, "rms", "Rms-1", &hRms, 0);
  assert(hRms);

  /* Configure the algo rms*/

  error = acAlgoSetConfig(hRms, "smoothingTime", "300");
  assert(error == 0);

  error = acAlgoSetConfig(hRms, "rmsWindow", "8000");
  assert(error == 0);


  /* We need a control call back, so, we use the common config to pass a user data (not used) and the callback */
  /*! [control-setup] */
  error = acAlgoSetCommonConfig(hRms, "userData", NULL);
  assert(error == 0);

  error = acAlgoSetCommonConfig(hRms, "controlCb", (void *)s_control_Algo_RMS_1);
  assert(error == 0);
  /*! [control-setup] */
  /*![create-algo] */


  /* Verify its new configuration */
  error = acAlgoDump(hRms);
  assert(error == 0);


  /*! Create Connection Generator to Split [connect]*/
  error = acPipeConnect(hPipe, hGenerator1, hCnx_0, hSplit1);
  assert(error == 0);

  /* Create Connection hSplit1 to hRms*/
  error = acPipeConnect(hPipe, hSplit1, hCnx_1, hRms);
  assert(error == 0);


  /* finlay, connect Split to sysout1 and sysout2 */

  error = acPipeConnectPinOut(hPipe, hSplit1, hSysOut2);
  assert(error == 0);

  error = acPipeConnectPinOut(hPipe, hSplit1, hSysOut1);
  assert(error == 0);
  /*! [connect]*/


  /*! the pipe is ready,  Start it [start-pipe]*/
  error =  acPipePlay(hPipe, AC_START);
  if (error != 0)
  {
    acTrace("Ac play error :( %d\n", error);
  }
  else
  {
    acTrace("Ac graph playing :)\n");
  }
  /*! [start-pipe]*/


  /* Program a delay to close the pipe and cleanup */
  osThreadNew(s_closeCB, NULL, NULL);
}

/*! [terminate] Close pipe Thread after a delay */

static void s_closeCB(void  *argument)
{
  /* Wait 5 secs */
  osDelay(5000);
  assert(acPipePlay(hPipe, AC_STOP) == 0);
  assert(acPipeDelete(hPipe) == 0);
  assert(acTerminate() == 0);
  while (1)
  {
    osDelay(1000);
    printf("The pipe is closed\n");
  }
}
/*! [terminate] */

/* RMS control function call back, process the RMS presentation */

/*! [control-callback] */
static int32_t  s_control_Algo_RMS_1(acAlgo hAlgo)
{
  float *pRmsArray;
  int32_t nbChannels = 2;
  void    *pUserData = NULL;
  int32_t error;

  error = acAlgoGetCommonConfig(hAlgo, "userData", &pUserData);
  assert(error == 0);
  error = acAlgoGetControl(hAlgo, "rms", &pRmsArray);
  assert(error == 0);
  error = acAlgoGetControl(hAlgo, "nbChannels", &nbChannels);
  assert(error == 0);

  if (pRmsArray != NULL)
  {
    if (nbChannels == 1)
    {
      acTrace("%2.2f%% = %2.1f dB\n", (100.0 * pRmsArray[0]), 20.0 * log10(pRmsArray[0]));

    }
    else
    {
      acTrace("LEFT %2.2f%% = %2.1f dB RIGHT %2.2f%% = %2.1f dB\n",
              100.0 * pRmsArray[0], 20.0 * log10(pRmsArray[0]),
              100.0 * pRmsArray[1], 20.0 * log10(pRmsArray[1]));
    }
  }
  return 1;
}


/*! [control-callback] */
/*! [create-pipe] */

