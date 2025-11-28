/**
  ******************************************************************************
  * @file    core_init.h
  * @author  MCD Application Team
  * @brief   Header for core_init.c module
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
#ifndef _CORE_INIT_H_
#define _CORE_INIT_H_

#ifdef __cplusplus
extern "C" {
#endif
/* Includes ------------------------------------------------------------------*/
#include "stm32xxx_voice_bsp.h"
#include "wba_link.h"
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void    CoreInit(void);
void    SystemClock_Config(void);
void    MPU_Config(void);
void    CPU_CACHE_Enable(void);
void    XSPI_Init(bool enable_mem_map);
void    XSPI_DeInit(void);


#ifdef __cplusplus
}
#endif

#endif /* _CORE_INIT_H_ */
