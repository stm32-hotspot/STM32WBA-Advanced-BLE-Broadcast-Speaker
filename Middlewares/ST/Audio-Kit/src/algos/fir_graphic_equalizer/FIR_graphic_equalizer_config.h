/**
  ******************************************************************************
  * @file    FIR_graphic_equalizer_config.h
  * @author  MCD Application Team
  * @brief   FIR graphic equalizer config
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
#ifndef __FIR_GRAPHIC_EQUALIZER_CONFIG_H
#define __FIR_GRAPHIC_EQUALIZER_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "FIR_equalizer/FIR_equalizer_config.h"

/* Exported constants --------------------------------------------------------*/
#define FIRGREQ_MAX_NB_BANDS 20

/* Exported types ------------------------------------------------------------*/

typedef enum
{
  FIRGREQ_MODE_AUTO,
  FIRGREQ_MODE_SEMI_AUTO,
  FIRGREQ_MODE_MANUAL,
  FIRGREQ_MODE_NB
} firGrEqMode_t;

typedef enum
{
  FIRGREQ_BAND_STEP_1DIV6_OCTAVE,
  FIRGREQ_BAND_STEP_1DIV5_OCTAVE,
  FIRGREQ_BAND_STEP_1DIV4_OCTAVE,
  FIRGREQ_BAND_STEP_1DIV3_OCTAVE,
  FIRGREQ_BAND_STEP_1DIV2_OCTAVE,
  FIRGREQ_BAND_STEP_1_OCTAVE,
  FIRGREQ_BAND_STEP_2_OCTAVES,
  FIRGREQ_BAND_STEP_3_OCTAVES,
  FIRGREQ_BAND_STEP_4_OCTAVES,
  FIRGREQ_BAND_STEP_5_OCTAVES,
  FIRGREQ_BAND_STEP_6_OCTAVES,
  FIRGREQ_BAND_STEP_NB
} firGrEqBandStep_t;

typedef struct
{
  float                centerFreq;
  float                gain_dB;
} firGrEqResponsePoint_t;

/* the following structure will be used in param description as a structure array, thus it must follow this template:
  uint32_t            <number of structure instances>;
  <structure typedef> <structure array name>[<max number of structure instances>];
*/
typedef struct
{
  uint32_t               nbBands;
  firGrEqResponsePoint_t firResponsePoints[FIRGREQ_MAX_NB_BANDS];
} firGrEqResponse_t;

typedef struct
{
  uint8_t              reorderBands;
  uint8_t              centerFrequencyMode;
  uint8_t              gainInterpolLinear;
  uint8_t              firType;
  uint8_t              bandCenterFrequencyStep;
  float                firstBandCenterFrequency;
  uint16_t             firSize;
  firGrEqResponse_t    firGrEqResponse;
  uint8_t              ramType;
} fir_graphic_equalizer_staticConfig_t;

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
} fir_graphic_equalizer_ctrl_t;

/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */


#ifdef __cplusplus
}
#endif

#endif /* __FIR_GRAPHIC_EQUALIZER_CONFIG_H */
