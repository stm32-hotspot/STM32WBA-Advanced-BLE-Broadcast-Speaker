/**
  ******************************************************************************
  ******************************************************************************
  * @file    IIR_equalizer_config.h
  * @author  MCD Application Team
  * @brief   IIR equalizer config
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
#ifndef __IIR_EQUALIZER_CONFIG_H
#define __IIR_EQUALIZER_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "common/biquad.h"

/* Exported constants --------------------------------------------------------*/
#define IIREQ_MAX_BIQUAD_CELLS 10
#if IIREQ_MAX_BIQUAD_CELLS > MAX_BIQUAD_CELLS
#error "IIREQ_MAX_BIQUAD_CELLS must be <= MAX_BIQUAD_CELLS"
#endif

/* Exported types ------------------------------------------------------------*/
/**
* @brief  Library configuration handler.
*/

/* the following structure will be used in param description as a structure array, thus it must follow this template:
  uint32_t            <number of structure instances>;
  <structure typedef> <structure array name>[<max number of structure instances>];
*/
typedef struct
{
  uint32_t           nbCells;
  biquadConfig_t     biquadConfig[IIREQ_MAX_BIQUAD_CELLS];
} iirEqStructArray_t;

typedef struct
{
  uint8_t            biquadSort;
  iirEqStructArray_t iirEqStructArray;
  uint8_t            ramType;
} iir_equalizer_staticConfig_t;

typedef struct
{
  uint32_t nbBands;
  float    samplingRate;
  float   *pResponse_dB;
  float    modMinFrequency;
  float    modMaxFrequency;
  float    modMin_dB;
  float    modMax_dB;
  float    modAvg_dB;
} iir_equalizer_ctrl_t;

/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */


#ifdef __cplusplus
}
#endif

#endif /* __IIR_EQUALIZER_CONFIG_H */
