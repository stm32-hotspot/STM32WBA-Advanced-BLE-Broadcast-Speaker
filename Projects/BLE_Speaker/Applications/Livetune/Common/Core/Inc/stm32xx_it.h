/**
  ******************************************************************************
  * @file    stm32xx_it.h
  * @author  MCD Application Team
  * @brief   Header for stm32xx_it.c module
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2019(-2022) STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32XX_IT_H
#define __STM32XX_IT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32xxx_voice_bsp.h"


/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void SVC_Handler(void);
void DebugMon_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);


void AUDIO_IN_DFSDM_DMA_1st_CH_IRQHandler(void);
void AUDIO_IN_DFSDM_DMA_2nd_CH_IRQHandler(void);
void AUDIO_IN_DFSDM_DMA_3rd_CH_IRQHandler(void);
void AUDIO_IN_DFSDM_DMA_4th_CH_IRQHandler(void);
void AUDIO_IN_I2S_IRQHandler(void);


#ifdef __cplusplus
}
#endif

#endif /* __STM32XX_IT_H */
