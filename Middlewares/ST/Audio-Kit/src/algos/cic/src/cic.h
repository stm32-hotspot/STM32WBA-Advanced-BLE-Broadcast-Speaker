/**
******************************************************************************
* @file    cic.h
* @author  MCD Application Team
* @brief   wrapper of cic algo to match usage inside audio_chain.c
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
#ifndef __CIC_H
#define __CIC_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
int32_t cic_init(audio_algo_t *const pAlgo);
int32_t cic_deinit(audio_algo_t *const pAlgo);
int32_t cic_configure(audio_algo_t *const pAlgo);
int32_t cic_process(audio_algo_t *const pAlgo);

#ifdef __cplusplus
}
#endif

#endif /* __CIC_H */
