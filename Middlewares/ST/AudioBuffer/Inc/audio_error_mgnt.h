/**
******************************************************************************
* @file    audio_error_mgnt.h
* @author  MCD Application Team
* @brief   Header for audio_error_mgnt.c
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
#ifndef __AUDIO_ERROR_MGNT_H
#define __AUDIO_ERROR_MGNT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define AUDIO_ERR_MGNT_NONE               (0)
#define AUDIO_ERR_MGNT_NB_MAX_PROCESS_ACC (-1)
#define AUDIO_ERR_MGNT_NB_MAX_CONTROL_ACC (-2)
#define AUDIO_ERR_MGNT_UNDERFLOW          (-3)
#define AUDIO_ERR_MGNT_OVERFLOW           (-4)
#define AUDIO_ERR_MGNT_VALUE_CLAMPED      (-5)
#define AUDIO_ERR_MGNT_CONFIG_WARNING     (-6)
#define AUDIO_ERR_MGNT_NOT_DONE           (-7)

#define AUDIO_ERR_MGNT_WARNING_LEVEL      (-20)

#define AUDIO_ERR_MGNT_NOT_FOUND          (AUDIO_ERR_MGNT_WARNING_LEVEL-2)
#define AUDIO_ERR_MGNT_NB_CH              (AUDIO_ERR_MGNT_WARNING_LEVEL-3)
#define AUDIO_ERR_MGNT_FS                 (AUDIO_ERR_MGNT_WARNING_LEVEL-4)
#define AUDIO_ERR_MGNT_TIME_FREQ          (AUDIO_ERR_MGNT_WARNING_LEVEL-5)
#define AUDIO_ERR_MGNT_TYPE               (AUDIO_ERR_MGNT_WARNING_LEVEL-6)
#define AUDIO_ERR_MGNT_INTERLEAVING       (AUDIO_ERR_MGNT_WARNING_LEVEL-7)
#define AUDIO_ERR_MGNT_NB_ELEMENTS        (AUDIO_ERR_MGNT_WARNING_LEVEL-8)
#define AUDIO_ERR_MGNT_DURATION           (AUDIO_ERR_MGNT_WARNING_LEVEL-9)
#define AUDIO_ERR_MGNT_ERROR              (AUDIO_ERR_MGNT_WARNING_LEVEL-10)
#define AUDIO_ERR_MGNT_MEMORY_LEAK        (AUDIO_ERR_MGNT_WARNING_LEVEL-11)
#define AUDIO_ERR_MGNT_PTR_NULL           (AUDIO_ERR_MGNT_WARNING_LEVEL-12)
#define AUDIO_ERR_MGNT_CMSIS              (AUDIO_ERR_MGNT_WARNING_LEVEL-13)
#define AUDIO_ERR_MGNT_CMSIS_FFT_INIT     (AUDIO_ERR_MGNT_WARNING_LEVEL-14)
#define AUDIO_ERR_MGNT_INIT               (AUDIO_ERR_MGNT_WARNING_LEVEL-15)
#define AUDIO_ERR_MGNT_PDM                (AUDIO_ERR_MGNT_WARNING_LEVEL-16)
#define AUDIO_ERR_MGNT_GRAPH_LOOP         (AUDIO_ERR_MGNT_WARNING_LEVEL-17)
#define AUDIO_ERR_MGNT_CONFIG             (AUDIO_ERR_MGNT_WARNING_LEVEL-18)
#define AUDIO_ERR_MGNT_ALLOCATION         (AUDIO_ERR_MGNT_WARNING_LEVEL-19)
#define AUDIO_ERR_MGNT_DEALLOCATION       (AUDIO_ERR_MGNT_WARNING_LEVEL-20)
#define AUDIO_ERR_MGNT_IMPLEMENTATION     (AUDIO_ERR_MGNT_WARNING_LEVEL-21)
#define AUDIO_ERR_MGNT_SYNTAX             (AUDIO_ERR_MGNT_WARNING_LEVEL-22)


/**
  * @brief  get algo's most important error between old and new error
  * @param  oldError: previous error
  * @param  newError: new error
  * @retval most important error
  */
int32_t AudioError_update(int32_t const oldError, int32_t const newError);

/**
  * @brief  is error code OK ?
  * @param  error: error
  * @retval TRUE if it is OK, else FALSE
  */
bool AudioError_isOk(int32_t const error);

/**
  * @brief  is error code NOK ?
  * @param  error: error
  * @retval TRUE if it is NOK (i.e. warning or error), else FALSE
  */
bool AudioError_isNok(int32_t const error);

/**
  * @brief  is error code a warning ?
  * @param  error: error
  * @retval TRUE if it is an warning, else FALSE
  */
bool AudioError_isWarning(int32_t const error);

/**
  * @brief  is error code an error ?
  * @param  error: error
  * @retval TRUE if it is an error, else FALSE
  */
bool AudioError_isError(int32_t const error);

/**
  * @brief  is not error code an error ?
  * @param  error: error
  * @retval TRUE if it is not an error (i.e. ok or warning), else FALSE
  */
bool AudioError_isNoError(int32_t const error);

#ifdef __cplusplus
}
#endif

#endif /* __AUDIO_BUFFER_H */
