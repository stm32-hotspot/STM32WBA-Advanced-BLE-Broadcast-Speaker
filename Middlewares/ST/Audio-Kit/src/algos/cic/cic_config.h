/**
  ******************************************************************************
  * @file    cic_config.h
  * @author  MCD Application Team
  * @brief   config of cic algo
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
#ifndef __CIC_CONFIG_H
#define __CIC_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
/* Exported types ------------------------------------------------------------*/
typedef struct
{
  uint8_t order;                      /* Filter order (0: FastSync)           */
  uint8_t ramType;
}
cic_static_config_t;

typedef struct
{
  uint8_t rbs;                        /* Right Bit Shift for gain             */
}
cic_dynamic_config_t;

/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */


#ifdef __cplusplus
}
#endif

#endif /* __CIC_CONFIG_H */
