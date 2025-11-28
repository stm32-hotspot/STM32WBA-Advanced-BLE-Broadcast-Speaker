/**
******************************************************************************
* @file    audio_algo_list.h
* @author  MCD Application Team
* @brief   chained list of algos.
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
#ifndef __AUDIO_ALGO_LIST_H
#define __AUDIO_ALGO_LIST_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

#include "audio_algo.h"

/* Exported types ------------------------------------------------------------*/
typedef struct audio_algo_list
{
  audio_algo_t           *pAlgo;
  struct audio_algo_list *next;
  struct audio_algo_list *prev;
} audio_algo_list_t;

/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */


#ifdef __cplusplus
}
#endif

#endif  /* __AUDIO_ALGO_LIST_H */
