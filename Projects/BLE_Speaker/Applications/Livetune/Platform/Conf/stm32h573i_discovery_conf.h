/**
  ******************************************************************************
  * @file    stm32h573i_discovery_conf.h
  * @author  MCD Application Team
  * @brief   STM32H573I-DK board configuration file.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32H573I_DISCOVERY_CONF_H
#define __STM32H573I_DISCOVERY_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h5xx_hal.h"
#include <stdio.h>


#define BSP_BUTTON_USER_IT_PRIORITY         15U
#define BSP_BUTTON_WAKEUP_IT_PRIORITY       15U

#define BSP_AUDIO_OUT_IT_PRIORITY           7U
#define BSP_AUDIO_IN_IT_PRIORITY            7U
#define BSP_TS_IT_PRIORITY                  15U

#ifdef __cplusplus
}
#endif

#endif /* __STM32H573I_DISCOVERY_CONF_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
