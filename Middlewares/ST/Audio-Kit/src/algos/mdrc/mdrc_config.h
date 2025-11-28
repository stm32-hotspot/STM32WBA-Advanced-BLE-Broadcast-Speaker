/**
  ******************************************************************************
  * @file    mdrc_config.h
  * @author  MCD Application Team
  * @brief   MDRC (multi-band dynamic range compressor) config
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
#ifndef __MDRC_CONFIG_H
#define __MDRC_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define MDRC5B_SUBBAND_MAX           5
#define MDRC5B_KNEEPOINTS_MAX        10

#define MDRC5B_POSTGAIN_MAX          45.0f        // in dB
#define MDRC5B_POSTGAIN_MIN          -45.0f       // in dB

#define MDRC5B_COM_AT_TIME_MAX       1000.0f      // in milliseconds
#define MDRC5B_COM_AT_TIME_MIN       1.0f         // in milliseconds
#define MDRC5B_COM_RE_TIME_MAX       5000.0f      // in milliseconds
#define MDRC5B_COM_RE_TIME_MIN       10.0f        // in milliseconds

#define MDRC5B_LEVEL_MIN_DB          -120.0f      // in dB
#define MDRC5B_LEVEL_MAX_DB          0.0f         // in dB

#define MDRC5B_LIMITER_AT_TIME_MAX   10.0f        // in milliseconds
#define MDRC5B_LIMITER_AT_TIME_MIN   0.1f         // in milliseconds
#define MDRC5B_LIMITER_RE_TIME_MAX   5000.0f      // in milliseconds
#define MDRC5B_LIMITER_RE_TIME_MIN   10.0f        // in milliseconds

#define MDRC5B_LIMITER_THRESH_DB_MAX -1.0f        // in dB, leave 1 dB buffer to saturation
#define MDRC5B_LIMITER_THRESH_DB_MIN -20.0f       // in dB

/* Exported types ------------------------------------------------------------*/

enum eProcessingModeMask
{
  MDRC_COMPRESSOR_MODE = 1,
  MDRC_LIMITER_MODE    = 2
};

// config structure for a subband compressor for the main channels
typedef struct
{
  float inputLevel;  // in dB
  float outputLevel; // in dB
} mdrc5b_dyn_resp_point_t;

/* the following structure will be used in param description as a structure array, thus it must follow this template:
  uint32_t            <number of structure instances>;
  <structure typedef> <structure array name>[<max number of structure instances>];
*/
typedef struct
{
  uint32_t                   kneePoints;
  mdrc5b_dyn_resp_point_t    dynamicResponsePoints[MDRC5B_KNEEPOINTS_MAX];
} mdrc5b_dyn_resp_t;

typedef struct
{
  uint8_t                    enable;
  uint32_t                   freqCutoff;        // in Hz
  float                      attackTime;        // in milliseconds
  float                      releaseTime;       // in milliseconds
  float                      postGain;          // in dB
  mdrc5b_dyn_resp_t          dynamicResponse;
} mdrc5b_band_config_t;

/* the following structure will be used in param description as a structure array, thus it must follow this template:
  uint32_t            <number of structure instances>;
  <structure typedef> <structure array name>[<max number of structure instances>];
*/
typedef struct
{
  uint32_t                   numBands;
  mdrc5b_band_config_t       bandCompressors[MDRC5B_SUBBAND_MAX];
} mdrc5b_compressor_config_t;

typedef struct
{
  float                      attackTime;        // in milliseconds
  float                      releaseTime;       // in milliseconds
  float                      thresh;            // in dB
} mdrc5b_limiter_config_t;

typedef struct
{
  uint8_t                    preset;            // not used, just for factory
  uint8_t                    mode;
  mdrc5b_compressor_config_t compressor;
  mdrc5b_limiter_config_t    limiter;
} mdrc5b_algo_config_t;

typedef mdrc5b_algo_config_t mdrcDynamicConfig_t;

typedef struct
{
  uint8_t                    ramType;
} mdrcStaticConfig_t;

/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */


#ifdef __cplusplus
}
#endif

#endif /* __MDRC_CONFIG_H */
