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

#include "ltc3556.h"

#define LTC3556_I2C_TIMEOUT    100
#define LTC3556_I2C_ADDR       0x12u

static uint8_t ltc3556_send(uint16_t regval);
static uint8_t LTC3556_set_switch_mode(uint16_t mode);

static I2C_HandleTypeDef *pI2C;

volatile uint16_t LTC3556_state = 0x00FF;

#define DE_CHR          0x0001

#define SERVO_SW1_MAX   0x00F0
#define SERVO_SW1_MED   0x0040
#define SERVO_SW1_MIN   0x0000

#define SERVO_SW3_MAX   0x000F
#define SERVO_SW3_MED   0x0008
#define SERVO_SW3_MIN   0x0000
#define SERVO_SW3_MASK  0xFFF0

#define USB_1xMODE      0x0000
#define USB_10xMODE     0x0100
#define USB_SUSPEND     0x0200
#define USB_5xMODE      0x0300

#define EN_REG3         0x0400

#define EN_REG2         0x0800

#define EN_REG1         0x1000

#define REG_PULSE       0x0000 /* SW1 : Pulse Skip     SW3 PWM*/
#define REG_FORCED      0x2000 /* SW1 : Forced Burst   SW3 Burst */
#define REG_LDO         0x4000 /* SW1 : LDO mode       SW3 Burst */
#define REG_BURST       0x6000 /* SW1 : Burst mode     SW3 PWM */

#define DIS_BAT_CHGR    0x8000

/*
 Output voltages are depending on the hardware impelmentation. Value are given for resistor of the datasheet such
 R4 = 750k
 R5 = 243k
 R6 = 560k
 R9 = 100k

 Then in this implementation
 SW1 is used for DIGITAL domain : V = servo * (R4/R5 + 1) = servo * 4.086
 SW2 in not used
 SW3 is the boost reg used for audio amplifier : V = (servo * R6/R9) + 1 = (servo * 5.6) + 1
*/


/* for SW3 : PWM allows better current, but has a low efficiency at low current,
             Burst mode is limited to 50mA, more noisy but much greater efficiency for low loads

  for SW1 : Pulse Skip : goes to high load (500mA), but efficiency drops at low current < 1mA
            Forced :     up to 100mA, better efficiency at low loads
            Burst :      up to 500mA, some ripple a low current, meddium efficiency at low loads
            LDO :        very low quiescent current
*/


static uint8_t ltc3556_send(uint16_t regval)
{
  uint16_t reg = regval;
  return (uint8_t)HAL_I2C_Master_Transmit(pI2C, LTC3556_I2C_ADDR, (uint8_t*)&reg, 2, LTC3556_I2C_TIMEOUT);
}

static uint8_t LTC3556_set_switch_mode(uint16_t mode)
{
  uint8_t status;

  if ((LTC3556_state & REG_BURST) == mode)
  {
    /* mode already set */
    status = HAL_OK;
  }
  else
  {
    LTC3556_state &= ~REG_BURST; /* clear all bits */
    LTC3556_state |= mode;    /* set bits */

    status = ltc3556_send(LTC3556_state);
  }

	return status;
}



/**
  * @brief  Initialize the component and allows to maintain SW1 voltage after ENALL is released.
  *         SW2 and SW3 are off. Regulator uses forced mode for better efficiency
  * @param  hi2c : handle of the I2C
  * @param  voltage : voltage applied on SW1
  * @retval HAL status
  */
uint8_t LTC3556_init(I2C_HandleTypeDef* hi2c, SysVoltage_t voltage)
{
	pI2C = hi2c;

  LTC3556_state = (SERVO_SW3_MIN | USB_5xMODE | REG_FORCED | EN_REG1) &~ (EN_REG2 | EN_REG3 | DE_CHR);

  if (voltage == V_3V3)
  {
    LTC3556_state |= SERVO_SW1_MAX;
  }
  else if (voltage == V_2V2)
  {
    LTC3556_state |= SERVO_SW1_MED;
  }
  else
  {
    return HAL_ERROR;
  }
	return ltc3556_send(LTC3556_state);
}


/**
  * @brief  Enable SW3 boost output, check schematics for precise voltage
  * @retval HAL status
  */
uint8_t LTC3556_set_5V_on(void)
{
  uint8_t status;

  LTC3556_state |= EN_REG3;
  LTC3556_state &= SERVO_SW3_MASK;
  status = ltc3556_send(LTC3556_state);

  HAL_Delay(20);

  LTC3556_state |= SERVO_SW3_MAX;
  status |= ltc3556_send(LTC3556_state);

  HAL_Delay(20);

  return status;
}

/**
  * @brief  Disable SW3 boost output
  * @retval HAL status
  */
uint8_t LTC3556_set_5V_off(void)
{
  uint8_t status;

  LTC3556_state &= SERVO_SW3_MASK;
  status = ltc3556_send(LTC3556_state);

  HAL_Delay(20);

  LTC3556_state &= ~EN_REG3;
  status = ltc3556_send(LTC3556_state);

  return status;
}

/**
  * @brief  Switch regulation mode to Burst mode, more efficient when high current is required on SW3
  * @retval HAL status
  */
uint8_t LTC3556_set_5V_high_load(uint8_t en_high_load)
{
  uint8_t status;

  if (en_high_load)
  {
    status = LTC3556_set_switch_mode(REG_BURST);
  }
  else
  {
    status = LTC3556_set_switch_mode(REG_FORCED);
  }

  return status;
}

/**
  * @brief  Disable all regulator output (system shutdown)
  * @retval HAL status
  */
uint8_t LTC3556_set_all_off(void)
{
  LTC3556_state &= ~(EN_REG1 | EN_REG3);
  return ltc3556_send(LTC3556_state);
}

/**
  * @brief  Disable or enable charging function
  * @param  disable : set to 1 for disabling or 0 for enabling the charger
  * @retval HAL status
  */
uint8_t LTC3556_disable_chrg(uint8_t disable)
{
  if (disable)
    LTC3556_state |= DIS_BAT_CHGR;
  else
    LTC3556_state &= ~DIS_BAT_CHGR;

  return ltc3556_send(LTC3556_state);
}
