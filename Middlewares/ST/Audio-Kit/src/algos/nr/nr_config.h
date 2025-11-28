/**
  ******************************************************************************
  * @file    nr_config.h
  * @author  MCD Application Team
  * @brief   NR config
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
#ifndef __NR_CONFIG_H
#define __NR_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define NR_ALPHAB_MIN      0.1f
#define NR_ALPHAB_MAX      0.9f
#define NR_ALPHAG_MIN      0.5f
#define NR_ALPHAG_MAX      2.0f

#define NR_ALPHAB_DEFAULT  0.3f
#define NR_ALPHAG_DEFAULT  0.8f
#define NR_ALPHAB_FAST     0.2f
#define NR_ALPHAG_FAST     0.6f
#define NR_ALPHAB_ACCURATE 0.5f
#define NR_ALPHAG_ACCURATE 1.0f

/* Exported types ------------------------------------------------------------*/

typedef enum
{
  NR_CONVERGENCE_PROFILE_DEFAULT,
  NR_CONVERGENCE_PROFILE_FAST,
  NR_CONVERGENCE_PROFILE_ACCURATE,
  NR_CONVERGENCE_PROFILE_USER_DEFINED_WITH_ALPHA
} nr_convergence_profile_t;

typedef struct
{
  float   maxAttInNoise;
  float   maxAttInSpeech;
  float   noiseMeanRef;
  float   lowPassBandPcent;
  uint8_t convergenceProfile;
  float   alphaB;
  float   alphaG;
}
nr_dynamic_config_t;

typedef struct
{
  uint8_t ramType;
}
nr_static_config_t;

/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */


#ifdef __cplusplus
}
#endif

#endif /* __NR_CONFIG_H */
