/**
  ******************************************************************************
  * @file    BoardSetup.h
  * @author  MCD Application Team
  * @brief   Header for BoardSetup.c module
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
#ifndef __BOARDSETUP_H
#define __BOARDSETUP_H

#ifdef __cplusplus
extern "C" {
#endif
/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void BoardInit(void);
void BoardPreInit(void);
void checkButton(void);
bool isButtonPushedAtInit(void);
bool isButtonPushed(void);
bool getButtonState(void);


#ifdef __cplusplus
}
#endif

#endif /* __BOARDSETUP_H */
