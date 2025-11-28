/**
******************************************************************************
* @file    audio_error_mgnt.c
* @author  MCD Application Team
* @brief   audio error management
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
#include <stdio.h>
#include <stdint.h>
#include "audio_error_mgnt.h"

/* Global variables ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

/* Functions Definition ------------------------------------------------------*/



/**
  * @brief  get algo's most important error between old and new error
  * @param  oldError: previous error
  * @param  newError: new error
  * @retval most important error
  */
int32_t AudioError_update(int32_t const oldError, int32_t const newError)
{
  return (newError < oldError) ? newError : oldError;
}


/**
  * @brief  is error code OK ?
  * @param  error: error
  * @retval TRUE if it is OK, else FALSE
  */
bool AudioError_isOk(int32_t const error)
{
  return (error == AUDIO_ERR_MGNT_NONE);
}


/**
  * @brief  is error code NOK ?
  * @param  error: error
  * @retval TRUE if it is NOK (i.e. warning or error), else FALSE
  */
bool AudioError_isNok(int32_t const error)
{
  return (error != AUDIO_ERR_MGNT_NONE);
}


/**
  * @brief  is error code a warning ?
  * @param  error: error
  * @retval TRUE if it is an warning, else FALSE
  */
bool AudioError_isWarning(int32_t const error)
{
  return (error >= AUDIO_ERR_MGNT_WARNING_LEVEL) && (error != AUDIO_ERR_MGNT_NONE);
}


/**
  * @brief  is error code an error ?
  * @param  error: error
  * @retval TRUE if it is an error, else FALSE
  */
bool AudioError_isError(int32_t const error)
{
  return (error < AUDIO_ERR_MGNT_WARNING_LEVEL);
}


/**
  * @brief  is not error code an error ?
  * @param  error: error
  * @retval TRUE if it is not an error (i.e. ok or warning), else FALSE
  */
bool AudioError_isNoError(int32_t const error)
{
  return (error >= AUDIO_ERR_MGNT_WARNING_LEVEL);
}
