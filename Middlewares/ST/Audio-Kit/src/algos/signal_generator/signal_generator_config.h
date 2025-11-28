/**
  ******************************************************************************
  * @file    signal_generator_config.h
  * @author  MCD Application Team
  * @brief   signal_generator config
  *******************************************************************************
  * @attention
  *
  * Copyright (c) 2019(-2022) STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ********************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SIGNAL_GENERATOR_CONFIG_H
#define __SIGNAL_GENERATOR_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
typedef enum {WHITE_NOISE, PINK_NOISE, SINE, SQUARE, TRIANGLE} signalType_t;

typedef struct
{
  uint8_t  ramType;
}
signal_generator_static_config_t;

typedef struct
{
  uint8_t  signalType;
  uint32_t frequency;               // for SINE, SQUARE or TRIANGLE; not used for WHITE_NOISE, PINK_NOISE
  float    gain;
}
signal_generator_dynamic_config_t;

/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */


#ifdef __cplusplus
}
#endif

#endif /* __SIGNAL_GENERATOR_CONFIG_H */
