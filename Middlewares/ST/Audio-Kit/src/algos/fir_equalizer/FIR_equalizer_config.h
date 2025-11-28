/**
  ******************************************************************************
  * @file    FIR_equalizer_config.h
  * @author  MCD Application Team
  * @brief   FIR equalizer config
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
#ifndef __FIR_EQUALIZER_CONFIG_H
#define __FIR_EQUALIZER_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define FIREQ_MAX_RESPONSE_POINTS 50

/* Exported types ------------------------------------------------------------*/
typedef enum
{
  FIREQ_LINEAR_PHASE,
  FIREQ_MINIMUM_PHASE,
  FIREQ_TYPE_NB
} firEqType_t;

typedef struct
{
  float                frequency;
  float                gain_dB;
} firEqResponsePoint_t;

/* the following structure will be used in param description as a structure array, thus it must follow this template:
  uint32_t            <number of structure instances>;
  <structure typedef> <structure array name>[<max number of structure instances>];
*/
typedef struct
{
  uint32_t             nbFirResponsePoints;
  firEqResponsePoint_t firResponsePoints[FIREQ_MAX_RESPONSE_POINTS];
} firEqResponse_t;

typedef struct
{
  uint8_t              preset;                // not used, just for factory
  uint8_t              gainInterpolLinear;
  uint8_t              firType;
  uint16_t             firSize;
  firEqResponse_t      firResponse;
  uint8_t              ramType;
} fir_equalizer_staticConfig_t;

typedef struct
{
  uint32_t nbBands;
  float    samplingRate;
  float   *pResponse_dB;
  float   *pResponseAtConfigFreq_dB;
  float    modMinFrequency;
  float    modMaxFrequency;
  float    modMin_dB;
  float    modMax_dB;
  float    modAvg_dB;
  float    avgError_dB;
  float    maxError_dB;
} fir_equalizer_ctrl_t;

/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */


#ifdef __cplusplus
}
#endif

#endif /* __FIR_EQUALIZER_CONFIG_H */
