/**
******************************************************************************
* @file    BLE_Speaker_H5_bsp_com.h
* @author  MCD Application Team
* @brief
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SPEAKER_BSP_COM_H
#define __SPEAKER_BSP_COM_H

#include "main.h"
#include "BoardSetup.h"
#include "stm32h5xx_hal.h"

void MX_TIM3_Init(void);
void MX_TIM3_Deinit(void);

void MX_I2C1_Init(void);

void HAL_I2C_MspInit(I2C_HandleTypeDef* hi2c);
void HAL_I2C_MspDeInit(I2C_HandleTypeDef* hi2c);

#define I2C_SLAVE_ADDR        0x19 /* own slave addr */
#define I2C_RX_LEN            (1u)
#define I2C_TX_LEN            (1u)

#define I2C_IT_PRIO_LISTEN    4
#define I2C_IT_PRIO_STD       5

#endif /* __SPEAKER_BSP_COM_H */
