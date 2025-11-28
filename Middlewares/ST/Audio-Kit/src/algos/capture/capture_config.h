/**
  ******************************************************************************
  * @file    capture_config.h
  * @author  MCD Application Team
  * @brief   config of capture algo (capture estimation from input time samples)
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
#ifndef __CAPTURE_CONFIG_H
#define __CAPTURE_CONFIG_H

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
typedef struct
{
  uint8_t  nbChannels;
  uint32_t nbSamplesByChannels;
  uint32_t fs;
  void    *pSamples;
} captureCtrl_t;

/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */


#ifdef __cplusplus
}
#endif

#endif /* __CAPTURE_CONFIG_H */
