/**
  ******************************************************************************
  * @file    ltc3556.c
  * @author  MCD Application Team
  * @brief   basic driver for communication with the LTC3556 component
  *          voltages are given for an hardware implementation
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

#ifndef _LTC3556_H_
#define _LTC3556_H_

#include <stdint.h>
#include "stm32wbaxx_hal.h"


/**
  * @brief  Voltage applied on SW1 (depend on hardware implementation)
  */
typedef enum
{
  V_3V3,
  V_2V2,
} SysVoltage_t;


/**
  * @brief  Initialize the component and allows to maintain SW1 voltage after ENALL is released.
  *         SW2 and SW3 are off. Regulator uses forced mode for better efficiency
  * @param  hi2c : handle of the I2C
  * @param  voltage : voltage applied on SW1
  * @retval HAL status
  */
uint8_t LTC3556_init(I2C_HandleTypeDef*, SysVoltage_t voltage);

/**
  * @brief  Enable SW3 boost output, check schematics for precise voltage
  * @retval HAL status
  */
uint8_t LTC3556_set_5V_on();

/**
  * @brief  Disable SW3 boost output
  * @retval HAL status
  */
uint8_t LTC3556_set_5V_off();

/**
  * @brief  Switch regulation mode to Burst mode, more efficient when high current is required on SW3
  * @retval HAL status
  */
uint8_t LTC3556_set_5V_high_load(uint8_t en_high_load);

/**
  * @brief  Disable all regulator output (system shutdown)
  * @retval HAL status
  */
uint8_t LTC3556_set_all_off();

/**
  * @brief  Disable or enable charging function
  * @param  disable : set to 1 for disabling or 0 for enabling the charger
  * @retval HAL status
  */
uint8_t LTC3556_disable_chrg(uint8_t disable);

#endif /* _LTC3556_H_ */
