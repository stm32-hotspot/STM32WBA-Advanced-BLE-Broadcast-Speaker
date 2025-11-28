/**
******************************************************************************
* @file          livetune_audio_chain_hook.h
* @author        MCD Application Team
* @brief         manage the connection with AC
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



#ifndef LIVETUNE_AUDIO_CHAIN_HOOK
#define LIVETUNE_AUDIO_CHAIN_HOOK

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32_audio.h"


/* Exported constants --------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/


/* Exported macros ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void livetune_hook_hw_conf(void);
void platform_flash_ext_config(void);



#ifdef __cplusplus
};
#endif


#endif



