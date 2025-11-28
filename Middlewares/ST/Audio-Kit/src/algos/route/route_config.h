/**
  ******************************************************************************
  * @file    route_config.h
  * @author  MCD Application Team
  * @brief   config of router algo
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
#ifndef __ROUTE_CONFIG_H
#define __ROUTE_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define AC_CHANNEL_MUTED                0xFFU
#define AUDIOCHAINWRP_ROUTE_NB_MAX      8   // max number of output channels

/* Exported types ------------------------------------------------------------*/
typedef struct
{
  uint8_t inputId;
  uint8_t chId;
} router_index_t;

typedef struct
{
  uint32_t       nbChOut;
  router_index_t tIndexes[AUDIOCHAINWRP_ROUTE_NB_MAX];
} routeChOutConfig_t;

typedef struct
{
  routeChOutConfig_t routeChOutConfig;
}
router_dynamic_config_t;

/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */


#ifdef __cplusplus
}
#endif

#endif /* __ROUTE_CONFIG_H */
