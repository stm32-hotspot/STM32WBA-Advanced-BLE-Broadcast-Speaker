/**
  ******************************************************************************
  * @file    max9867.c
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

#include "max9867.h"

#define MAX9867_I2C_TIMEOUT       10
#define MAX9867_I2C_ADDR          0x30

/*status*/
#define MAX9867_STATUS_ADDR       0x00
#define MAX9867_IT_EN_ADDR        0x03

/*clock ctrl*/
#define MAX9867_SYSCLK_ADDR       0x05
#define MAX9867_CLK_HIGH_ADDR     0x06
#define MAX9867_CLK_LOW_ADDR      0x07

/*DAI*/
#define MAX9867_DAI1_ADDR         0x08
#define MAX9867_DAI2_ADDR         0x09

/*filtering*/
#define MAX9867_DFILTER_ADDR      0x0a

/*level control*/
#define MAX9867_SIDETONE_ADDR     0x0b
#define MAX9867_DACLVL_ADDR       0x0c
#define MAX9867_ADCLVL_ADDR       0x0d
#define MAX9867_LVOL_ADDR         0x10
#define MAX9867_RVOL_ADDR         0x11
#define MAX9867_MODE_ADDR         0x16

/*power mngnt*/
#define MAX9867_PWR_ADDR          0x17
#define MAX9867_REV_ADDR          0xff

#define MAX9867_REV_VAL           0x42


static I2C_HandleTypeDef *pI2C;


static uint8_t max9867_RegisterRead(uint16_t addr, uint8_t *data, uint8_t len)
{
  return (uint8_t)HAL_I2C_Mem_Read(pI2C, MAX9867_I2C_ADDR, addr, 1, data, len, MAX9867_I2C_TIMEOUT);
}

static uint8_t max9867_RegisterWrite(uint16_t addr, uint8_t *data, uint8_t len)
{
  return (uint8_t)HAL_I2C_Mem_Write(pI2C, MAX9867_I2C_ADDR, addr, 1, data, len, MAX9867_I2C_TIMEOUT);
}


/**
  * @brief  Probe the component
  * @param  hi2c : handle of the I2C
  * @retval HAL status
  */
uint8_t MAX9867_Init(I2C_HandleTypeDef *hi2c)
{
  uint8_t i2cdata[1];
  uint8_t status = 0;

  pI2C = hi2c;

  status = max9867_RegisterRead(MAX9867_REV_ADDR, i2cdata, 1);
  if ((status == 0) && (i2cdata[0] != MAX9867_REV_VAL))
  {
    status = 1;
  }

	return status;
}


/**
  * @brief  Set component in shutdown mode
  * @retval HAL status
  */
uint8_t MAX9867_Shutdown(void)
{
  uint8_t i2cdata[1];
  i2cdata[0] = 0x00;

  return max9867_RegisterWrite(MAX9867_PWR_ADDR, i2cdata, 1);
}


/**
  * @brief  Set component in media mode (music FIR), mono L output only
  * @retval HAL status
  */
uint8_t MAX9867_StartMedia(void)
{
  uint8_t i2cdata[1];
  uint8_t status;

  /* setup clocks : input is 12.288MHz */
  i2cdata[0] = 0x10;
  status = max9867_RegisterWrite(MAX9867_SYSCLK_ADDR, i2cdata, 1);

  uint16_t NIval = 0x6000;
  i2cdata[0] = 0x00 | (uint8_t)(NIval>>7);
  status |= max9867_RegisterWrite(MAX9867_CLK_HIGH_ADDR, i2cdata, 1);

  i2cdata[0] = (uint8_t)(NIval & 0x00ff);
  status |= max9867_RegisterWrite(MAX9867_CLK_LOW_ADDR, i2cdata, 1);

  /*Configure digital audio interface*/
  /* use DLY : I2S compatible */
  i2cdata[0] = 0x10;
  status |= max9867_RegisterWrite(MAX9867_DAI1_ADDR, i2cdata, 1);

  /* filters : mode FIR (music)*/
  i2cdata[0] = 0x80;
  status |= max9867_RegisterWrite(MAX9867_DFILTER_ADDR, i2cdata, 1);

  /* DAC level */
  i2cdata[0] = 0x0a;
  status |= max9867_RegisterWrite(MAX9867_DACLVL_ADDR, i2cdata, 1);

  /* set HP mode to mono */
  i2cdata[0] = 0x01;
  status |= max9867_RegisterWrite(MAX9867_PWR_ADDR, i2cdata, 1);

  /* Disable shutdown and enable Left DAC */
  i2cdata[0] = 0x88;
  status |= max9867_RegisterWrite(MAX9867_PWR_ADDR, i2cdata, 1);

	return status;
}


/**
  * @brief  Set the volume register
  * @param  volume : volume in range 0 to 100
  * @retval HAL status
  */
uint8_t MAX9867_SetVolume(uint8_t volume)
{
  uint8_t i2cdata[1];
  uint8_t status = 0;

  if (volume == 0)
  {
    i2cdata[0] = 0x3f;//0x40;
  }
  else if (volume >= 100)
  {
    i2cdata[0] = 0x00;
  }
  else
  {
    //i2cdata[0] = (uint8_t)(((100 - volume)*39)/100);
    float tmp = (float)(volume);
    i2cdata[0] = (uint8_t)(tmp*tmp*-0.0025f+0.011f*tmp+25.2f);
  }

  /* gain is coded on 6 bits (lsb) */
  status = max9867_RegisterWrite(MAX9867_LVOL_ADDR, i2cdata, 1);

  return status;
}


/**
  * @brief  Mute the component
  * @param  en_mute : set to 1 for mute or 0 to unmute
  * @retval HAL status
  */
uint8_t MAX9867_Mute(uint8_t en_mute)
{
  uint8_t i2cdata[1];
  uint8_t status = 0;

  status = max9867_RegisterRead(MAX9867_LVOL_ADDR, i2cdata, 1);

  if (en_mute)
  {
    i2cdata[0] |= 0x40;
  }
  else
  {
    i2cdata[0] &= ~0x40;
  }

  status |= max9867_RegisterWrite(MAX9867_LVOL_ADDR, i2cdata, 1);

	return status;
}