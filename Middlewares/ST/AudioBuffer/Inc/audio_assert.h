/**
******************************************************************************
* @file    audio_assert.h
* @author  MCD Application Team
* @brief   Header for AUDIO_ASSERT macro.
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
#ifndef __AUDIO_ASSERT_H
#define __AUDIO_ASSERT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <assert.h>

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
#define AUDIO_ASSERT(cond, pUtilsHandle, ...) AudioChainUtils_assert((pUtilsHandle), (cond), #cond, __FILE__, __LINE__, __VA_ARGS__)


#ifdef __cplusplus
}
#endif

#endif /* __AUDIO_ASSERT_H */
