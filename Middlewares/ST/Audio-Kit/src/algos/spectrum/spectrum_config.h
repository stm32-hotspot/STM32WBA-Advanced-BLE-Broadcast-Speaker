/**
  ******************************************************************************
  * @file    spectrum_config.h
  * @author  MCD Application Team
  * @brief   spectrum config
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
#ifndef __SPECTRUM_CONFIG_H
#define __SPECTRUM_CONFIG_H

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
typedef struct
{
  uint32_t fftLength;
  uint8_t ramType;
}
spectrum_static_config_t;

typedef struct
{
  float   *pSquareMag;         // nbChIn*nbBands
  float    normalizationCoef;  // square magnitude normalization coefficient
  uint32_t nbAccSpectrum;      // number of accumulated spectrums (must be set to 0 by user to acknowledge reading of square magnitude for spectrum to restart accumulation)
  uint32_t fs;
  uint32_t fftLength;
  uint8_t  nbChannels;
} spectrumCtrl_t;

/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */


#ifdef __cplusplus
}
#endif

#endif /* __AUDIO_CHAIN_SPECTRUM_H */
