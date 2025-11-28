/**
  ******************************************************************************
  * @file    H5com.c
  * @author  MCD Application Team
  * @brief   Driver for communication with H5 component implemented on the
  *          PCB BLE Speaker and runing the associated firmware
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

#include "H5com.h"

#define H5APP_I2C_TIMEOUT    100
#define H5APP_I2C_ADDR       0x19u

extern I2C_HandleTypeDef hi2c3;

/**
  * @brief  Send an command code to the STM32H5 component
  * @param cmd : one byte command
  * @retval HAL status
  */
uint8_t H5_send_cmd(uint8_t cmd)
{
  uint8_t i2cdata = cmd;
  return HAL_I2C_Master_Transmit(&hi2c3, H5APP_I2C_ADDR, &i2cdata, 1, H5APP_I2C_TIMEOUT);
}

/**
  * @brief  Read status from the STM32H5 component
  * @param status : pointer for returning the status
  * @retval HAL status
  */
uint8_t H5_read_status(uint8_t *status)
{
  return (uint8_t)HAL_I2C_Master_Receive(&hi2c3, H5APP_I2C_ADDR, status, 1, H5APP_I2C_TIMEOUT);
}

/**
  * @brief  Initialize the wakeup command pin and generate a pulse
  * @retval none
  */
void H5_wakeup(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  HAL_GPIO_WritePin(SPEAKER_COMP_WPUP_PORT, SPEAKER_COMP_WKUP_PIN, GPIO_PIN_RESET);

  GPIO_InitStruct.Pin = SPEAKER_COMP_WKUP_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(SPEAKER_COMP_WPUP_PORT, &GPIO_InitStruct);

  HAL_GPIO_WritePin(SPEAKER_COMP_WPUP_PORT, SPEAKER_COMP_WKUP_PIN, GPIO_PIN_SET);
  HAL_Delay(10);
  HAL_GPIO_WritePin(SPEAKER_COMP_WPUP_PORT, SPEAKER_COMP_WKUP_PIN, GPIO_PIN_RESET);
  HAL_Delay(20); /* boot first part : I2C ready */
}

/**
  * @brief  De-initialize the wakeup command pin
  * @retval none
  */
void H5_free_gpio(void)
{
  /* pin have pull down at H5 side */
  HAL_GPIO_DeInit(SPEAKER_COMP_WPUP_PORT, SPEAKER_COMP_WKUP_PIN);
}
