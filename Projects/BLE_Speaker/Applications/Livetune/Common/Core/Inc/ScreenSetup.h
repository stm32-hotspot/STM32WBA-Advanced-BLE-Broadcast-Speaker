/**
  ******************************************************************************
  * @file    ScreenSetup.h
  * @author  MCD Application Team
  * @brief   Header for ScreenSetup.c module
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
#ifndef __SCREENSETUP_H
#define __SCREENSETUP_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "screen_conf.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void    LCDInit(void);
void    LCDDisplayHeader(void);
uint8_t LCDIsInitDone(void);
void    LCDInitXspi(void);

#ifdef __cplusplus
}
#endif

#endif /* __SCREENSETUP_H */