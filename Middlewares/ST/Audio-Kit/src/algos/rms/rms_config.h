/**
  ******************************************************************************
  * @file    rms_config.h
  * @author  MCD Application Team
  * @brief   rms algo config
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
#ifndef __RMS_CONFIG_H
#define __RMS_CONFIG_H

/* Includes ------------------------------------------------------------------*/
#include "audio_chain.h"

/* Exported types ------------------------------------------------------------*/
#define RMS_MAX_NB_CHANNELS 8U

typedef struct
{
  uint16_t smoothingTime;  // time in milliseconds
  uint16_t rmsWindow;      // for output
}
rms_dyn_config_t;

typedef struct
{
  uint8_t isDoublePrecision;
}
rms_stat_config_t;

typedef struct
{
  uint32_t  nbChannels;
  float     rms[RMS_MAX_NB_CHANNELS];     // one value per channel
} rmsCtrl_t;

/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */


#ifdef __cplusplus
}
#endif

#endif /* __RMS_CONFIG_H */
