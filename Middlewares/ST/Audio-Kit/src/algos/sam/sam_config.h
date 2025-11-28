/**
  ******************************************************************************
  * @file    sam_config.h
  * @author  MCD Application Team
  * @brief   frequency sam algorithm config
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
#ifndef __SAM_CONFIG_H
#define __SAM_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define SAM_MODE_LOWCONSO        0U
#define SAM_MODE_HIGHPERF        1U
#define SAM_MODE_HIGHDIRECTIVITY 2U

#define SAM_BEAMSELECT_FRONT     0U
#define SAM_BEAMSELECT_REAR      1U
#define SAM_BEAMSELECT_NOCHANGE  2U

#define SAM_MIXER_DISABLE        0U
#define SAM_MIXER_ENABLE         1U
#define SAM_MIXER_NOCHANGE       2U

/* Exported types ------------------------------------------------------------*/
/**
* @brief  Library dynamic configuration handler. It contains dynamic parameters.
*/

typedef struct
{
  //  uint8_t  mode;
  uint16_t nbTaps;
  uint16_t control; /* if 0 control is not active, if n will be call every n*frame size */
  uint8_t  ramType;
}
sam_static_config_t;

typedef struct
{
  uint8_t  freeze;
  uint8_t  beamSelect;
  //  uint8_t  enable_mixer;
  //  uint16_t idBandStart;
  uint16_t attackMs;
  uint16_t releaseMs;
  float    mu;
  //  float    gain_pre;
  float    gain;
  //  float    tLowDb;
  //  float    tHighDb;
}
sam_dynamic_config_t;

typedef struct
{
  uint8_t  beamSelect;
  //  float    psdMicDb;
  //  float    mixerGainLin;
}
sam_control_t;


/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */


#ifdef __cplusplus
}
#endif

#endif /* __SAM_CONFIG_H */
