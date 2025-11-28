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

#ifndef _STM32H5COM_H_
#define _STM32H5COM_H_

#include <stdint.h>
#include "stm32wbaxx_hal.h"
#include "board_mngr.h"

/* COMMANDS */
#define H5APP_SHUTDOWN            0x01
#define H5APP_RUN_AUDIKIT         0x02
#define H5APP_RESET               0x03
#define H5APP_START_LOCAL_CLK     0x04
#define H5APP_STOP_LOCAL_CLK      0x05
#define H5APP_SET_I2S_AUDIOCLK    0x06
#define H5APP_SET_I2S_PLLQ        0x07

#define H5APP_PLAY_ADV            0x10
#define H5APP_PLAY_CON            0x11
#define H5APP_PLAY_STB            0x12
#define H5APP_PLAY_LOWBAT         0x13
#define H5APP_PLAY_DISCO          0x14
#define H5APP_PLAY_SYNCH          0x15
#define H5APP_PLAY_SCAN           0x16
#define H5APP_PLAY_EXE            0x17
#define H5APP_PLAY_BIP            0x18

#define H5APP_STOP_PLAY           0x20

#define H5APP_BOOT_MEDIA          0x30
#define H5APP_BOOT_AURACAST       0x31
#define H5APP_BOOT_TELEPHONY      0x32

/* STATES */
#define H5APP_STATE_IDLE          0x00
#define H5APP_STATE_CLK_ON        0x01
#define H5APP_STATE_LOCAL_PLAY    0x02
#define H5APP_STATE_WBA_PLAY      0x04
#define H5APP_STATE_BUSY          0x08 /* command pending to be executed */
#define H5APP_STATE_RUN_AUDIOCLK  0x10

/**
  * @brief  Send an command code to the STM32H5 component
  * @param cmd : one byte command
  * @retval HAL status
  */
uint8_t H5_send_cmd(uint8_t cmd);

/**
  * @brief  Read status from the STM32H5 component
  * @param status : pointer for returning the status
  * @retval HAL status
  */
uint8_t H5_read_status(uint8_t *status);

/**
  * @brief  Initialize the wakeup command pin and generate a pulse
  * @retval none
  */
void H5_wakeup();

/**
  * @brief  De-initialize the wakeup command pin
  * @retval none
  */
void H5_free_gpio(void);

#endif /* _STM32H5COM_H_ */
