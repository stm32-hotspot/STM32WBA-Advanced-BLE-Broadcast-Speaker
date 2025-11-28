/**
  ******************************************************************************
  * @file    max9867.h
  * @author  MCD Application Team
  * @brief   basic driver for communication with the MAX9867 component
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

#ifndef _MAX9867_H_
#define _MAX9867_H_

#include <stdint.h>
#include "stm32wbaxx_hal.h"

/**
  * @brief  Probe the component
  * @param  hi2c : handle of the I2C
  * @retval HAL status
  */
uint8_t MAX9867_Init(I2C_HandleTypeDef *hi2c);

/**
  * @brief  Set component in shutdown mode
  * @retval HAL status
  */
uint8_t MAX9867_Shutdown(void);

/**
  * @brief  Set component in media mode (music FIR), mono L output only
  * @retval HAL status
  */
uint8_t MAX9867_StartMedia(void);

/**
  * @brief  Set the volume register
  * @param  volume : volume in range 0 to 100
  * @retval HAL status
  */
uint8_t MAX9867_SetVolume(uint8_t volume);

/**
  * @brief  Mute the component
  * @param  en_mute : set to 1 for mute or 0 to unmute
  * @retval HAL status
  */
uint8_t MAX9867_Mute(uint8_t en_mute);

#endif /* _MAX9867_H_ */
