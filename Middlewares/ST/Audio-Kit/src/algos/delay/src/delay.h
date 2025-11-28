/**
******************************************************************************
* @file    delay.h
* @author  MCD Application Team
* @brief   wrapper of delay algo to match usage inside audio_chain.c
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
#ifndef __DELAY_H
#define __DELAY_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
int32_t delay_init(audio_algo_t *const pAlgo, bool const delayInSeconds);
int32_t delay_deinit(audio_algo_t *const pAlgo);
int32_t delay_dataInOut(audio_algo_t *const pAlgo);

#ifdef __cplusplus
}
#endif

#endif /* __DELAY_H */
